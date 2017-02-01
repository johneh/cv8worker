/*
Implementation follows (code copied, or translated from C++ to JS)
node-canvas  (https://github.com/Automattic/node-canvas)

Copyright (c) 2010 LearnBoost, and contributors <dev@learnboost.com>

Copyright (c) 2014 Automattic, Inc and contributors <dev@automattic.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

const pango = $loadlib('./libpango.so');
const gObject = $loadlib('../gdk/libgobject.so');
const libcairo = require('./cairo.js');
const cairo = libcairo.identifiers;

const PANGO_SCALE = 1024;   // See pango/pango-types.h

const TEXT_BASELINE = {
    alphabetic  : 0,
    top         : 1,
    bottom      : 2,
    middle      : 3,
    ideographic : 4,
    hanging     : 5
};

const TEXT_DRAW_PATHS = 0;
const TEXT_DRAW_GLYPHS = 1;

function _clone(obj) {
    // simple types, null or undefined
    if (obj === null || typeof obj !== 'object')
        return obj;

    // Array
    if (obj instanceof Array) {
        let copy = [];
        for (let i = 0, len = obj.length; i < len; i++) {
            copy[i] = _clone(obj[i]);
        }
        return copy;
    }

    // Pointer
    if ($isPointer(obj))
        return obj;

    // Object
    if (obj instanceof Object) {
        let copy = {};
        for (let attr in obj) {
            if (obj.hasOwnProperty(attr))
                copy[attr] = _clone(obj[attr]);
        }
        return copy;
    }
}

/**
 * Font RegExp helpers.
 */

const weights = 'normal|bold|bolder|lighter|[1-9]00'
  , styles = 'normal|italic|oblique'
  , units = 'px|pt|pc|in|cm|mm|%'
  , string = '\'([^\']+)\'|"([^"]+)"|[\\w-]+';

/**
 * Font parser RegExp;
 */

const fontre = new RegExp('^ *'
  + '(?:(' + weights + ') *)?'
  + '(?:(' + styles + ') *)?'
  + '([\\d\\.]+)(' + units + ') *'
  + '((?:' + string + ')( *, *(?:' + string + '))*)'
);

function _parseFont(str) {
    let font = {}, captures = fontre.exec(str);

    // Invalid
    if (!captures) return;

    // Cached
    //  if (cache[str]) return cache[str];

    // Populate font object
    font.weight = captures[1] || 'normal';
    font.style = captures[2] || 'normal';
    font.size = parseFloat(captures[3]);
    font.unit = captures[4];
    font.family = captures[5].replace(/["']/g, '').split(',').map(function (family) {
        return family.trim();
    }).join(',');

    // TODO: dpi
    // TODO: remaining unit conversion
    switch (font.unit) {
    case 'pt':
        font.size /= .75;
        break;
    case 'in':
        font.size *= 96;
        break;
    case 'mm':
        font.size *= 96.0 / 25.4;
        break;
    case 'cm':
        font.size *= 96.0 / 2.54;
        break;
    }

    // return cache[str] = font;
    return font;
}

function _setFont(layout, fontDesc, font) {
    if (font.family) {
        pango.font_description_set_family(fontDesc, font.family);
    }
    let s = pango.PANGO_STYLE_NORMAL;
    if (font.style === "italic") {
        s = pango.PANGO_STYLE_ITALIC;
    } else if (font.style === "oblique") {
        s = PANGO_STYLE_OBLIQUE;
    }
    pango.font_description_set_style(fontDesc, s);

    let w = pango.PANGO_WEIGHT_NORMAL;
    if (font.weight === "bold") {
        w = pango.PANGO_WEIGHT_BOLD;
    } else if (font.weight === "light") {
        w = pango.PANGO_WEIGHT_LIGHT;
    } // else if ... TODO ..

    pango.font_description_set_weight(fontDesc, w);

    if (font.size > 0) {
        pango.font_description_set_absolute_size(fontDesc, font.size * PANGO_SCALE);
    }
    pango.layout_set_font_description(layout, fontDesc);
}

function _fontMetrics(layout, fontDesc) {
    return pango.context_get_metrics(pango.layout_get_context(layout),
        fontDesc, $nullptr);
}

function _hasShadow(state) {
	return state.shadow.alpha
		&& (state.shadowBlur || state.shadowOffsetX || state.shadowOffsetY);
}

function rgba_from_str(s) {
    let re1 = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/gi;
    let re2 = /\s*rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\)/g;
    let re3 = /\s*rgba\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,(.*)\)/g;
    let rgba = null;

    s.replace(re1, function (p0, p1, p2, p3) {
        rgba = { red: parseInt(p1, 16) / 255,
                 green: parseInt(p2, 16) / 255,
                 blue: parseInt(p3, 16) / 255,
                 alpha: 1 };
    });
    if (rgba === null) {
        s.replace(re2, function (p0, p1, p2, p3) {
            rgba = { red: Math.min(p1 / 255, 1),
                     green: Math.min(p2 / 255, 1),
                     blue: Math.min(p3 / 255, 1),
                     alpha: 1 };
        });
    }
    if (rgba === null) {
        s.replace(re3, function (p0, p1, p2, p3, p4) {
            rgba = { red: Math.min(p1 / 255, 1),
                     green: Math.min(p2 / 255, 1),
                     blue: Math.min(p3 / 255, 1),
                     alpha: Math.min(parseFloat(p4), 1) };
        });
    }
    return rgba;
}

function rgba_to_str(rgba) {
    if (1 === rgba.alpha) {
        return '#' + Math.round(rgba.red * 255).toString(16)
                + Math.round(rgba.green * 255).toString(16)
                + Math.round(rgba.blue * 255).toString(16);
    } else {
        return 'rgba(' + Math.round(rgba.red * 255) + ', '
                + Math.round(rgba.green * 255) + ', '
                + Math.round(rgba.blue * 255) + ', '
                + Math.round(rgba.alpha * 100) / 100 + ')';
    }
}

function _setSourceRGBA(ctx, color, globalAlpha) {
    cairo.set_source_rgba(ctx,
        color.red,
        color.green,
        color.blue,
        color.alpha * globalAlpha);
}

// TODO: check if image surface
function _shadow(ctx, state, fn) {
    let path = cairo.copy_path_flat(ctx);
    cairo.save(ctx);

    // shadowOffset is unaffected by current transform
    let path_matrix = new ArrayBuffer(libcairo.sizeOf('cairo_matrix_t'));
    cairo.get_matrix(ctx, path_matrix);
    cairo.identity_matrix(ctx);

    // Apply shadow
    cairo.push_group(ctx);

    // No need to invoke blur if shadowBlur is 0
    if (state.shadowBlur) {
        // find out extent of path
        let dx1 = new Float64Array(1);
        let dy1 = new Float64Array(1);
        let dx2 = new Float64Array(1);
        let dy2 = new Float64Array(1);
        if (fn === cairo.fill || fn === cairo.fill_preserve) {
            cairo.fill_extents(ctx, dx1, dy1, dx2, dy2);
        } else {
            cairo.stroke_extents(ctx, dx1, dy1, dx2, dy2);
        }
        let x1 = dx1[0], y1 = dy1[0], x2 = dx2[0], y2 = dy2[0];
        // create new image surface that size + padding for blurring
        let dx = dx1, dy = dy1;
        dx[0] = x2-x1;
        dy[0] = y2-y1;
        cairo.user_to_device_distance(ctx, dx, dy);
        let pad = state.shadowBlur * 2;
        let shadow_surface = cairo.image_surface_create(
                cairo.CAIRO_FORMAT_ARGB32,
                dx[0] + 2 * pad,
                dy[0] + 2 * pad);
        let shadow_context = cairo.create(shadow_surface);

        // transform path to the right place
        cairo.translate(shadow_context, pad-x1, pad-y1);
        cairo.transform(shadow_context, path_matrix);

        // draw the path and blur
        cairo.set_line_width(shadow_context, cairo.get_line_width(ctx));
        cairo.new_path(shadow_context);
        cairo.append_path(shadow_context, path);
        _setSourceRGBA(shadow_context, state.shadow, state.globalAlpha);
        fn(shadow_context);
        cairo.blur(shadow_surface, state.shadowBlur);

        // paint to original context
        cairo.set_source_surface(ctx, shadow_surface,
                x1 - pad + state.shadowOffsetX + 1,
                y1 - pad + state.shadowOffsetY + 1);
        cairo.paint(ctx);
        cairo.destroy(shadow_context);
        cairo.surface_destroy(shadow_surface);
    } else {
        // Offset first, then apply path's transform
        cairo.translate(ctx,
            state.shadowOffsetX, state.shadowOffsetY);
        cairo.transform(ctx, path_matrix);

        // Apply shadow
        cairo.new_path(ctx);
        cairo.append_path(ctx, path);
        _setSourceRGBA(ctx, state.shadow, state.globalAlpha);
        fn(ctx);
    }

    // Paint the shadow
    cairo.pop_group_to_source(ctx);
    cairo.paint(ctx);

    // Restore state
    cairo.restore(ctx);
    cairo.new_path(ctx);
    cairo.append_path(ctx, path);
    fn(ctx);

    cairo.path_destroy(path);
}

function _savePath(ctx) {
    let path = cairo.copy_path_flat(ctx);
    cairo.new_path(ctx);
    return path;
}

function _restorePath(ctx, path) {
    cairo.new_path(ctx);
    cairo.append_path(ctx, path);
    cairo.path_destroy(path);
}

class Context2d {
    constructor(canvas) {
    	this.canvas = canvas;

        let cr = cairo.create(canvas._surface).notNull();
        cr.gc(cairo.destroy);
        this._context = cr;

        cairo.set_line_width(cr, 1);

        this._layout = pango.cairo_create_layout(cr).notNull();
        this._layout.gc(gObject.unref);

        let fontDescription = pango.font_description_from_string("sans serif");
        fontDescription.gc(pango.font_description_free);
        pango.font_description_set_absolute_size(fontDescription, 10 * PANGO_SCALE);
        pango.layout_set_font_description(this._layout, fontDescription);

    	this._default_state = {
            shadowBlur          : 0,
            shadowOffsetX       : 0,
            shadowOffsetY       : 0,
            globalAlpha         : 1.0,
            textAlignment       : -1,
            textBaseline        : TEXT_BASELINE["alphabetic"],
            fillPattern         : null,
            strokePattern       : null,
            fillGradient        : null,
            strokeGradient      : null,
            fill                : { red: 0, green: 0, blue: 0, alpha: 1 },	// transparent
            stroke              : { red: 0, green: 0, blue: 0, alpha: 1 },	// transparent
            shadow              : { red: 0, green: 0, blue: 0, alpha: 0 },	// transparent_black
            patternQuality      : cairo.CAIRO_FILTER_GOOD,
            textDrawingMode     : TEXT_DRAW_PATHS,
            fontDescription     : fontDescription,
        };

        this.state = _clone(this._default_state);
        this.states = [];
    }

    _resetContext() {
        let cr = cairo.create(this.canvas._surface).notNull();
        cr.gc(cairo.destroy);
        this._context = cr;
        this.state = _clone(this._default_state);
        this.states = [];
        // FIXME: reset pango layout?
    }

    save() {
        cairo.save(this._context);
        let state = _clone(this.state);
        state.fontDescription = pango.font_description_copy(this.state.fontDescription);
        state.fontDescription.gc(pango.font_description_free);
        this.states.push(state);
    }
    
    restore() {
        if (this.states.length > 0) {
            cairo.restore(this._context);
            this.state = this.states.pop();
            pango.layout_set_font_description(_layout, this.state.fontDescription);
        }
    }

    beginPath() {
        cairo.new_path(this._context);
    }

    closePath() {
        cairo.close_path(this._context);
    }

    clip() {
        cairo.clip_preserve(this._context);
    }

    isPointInPath(x, y) {
        if ('number' === typeof x && 'number' === typeof y)
            return cairo.in_fill(this._context, x, y)
                || cairo.in_stroke(this._context, x, y);
        return false;
    }

    scale(scale_width, scale_height) {
        scale_width = +scale_width || 0;
        scale_height = +scale_height || 0;
        if (scale_width > 0 && scale_height > 0)
            cairo.scale(this._context, scale_width, scale_height);
    }

    rotate(angle) {
    	cairo.rotate(this._context, +angle || 0);
    }

    translate(x, y) {
        cairo.translate(this._context, +x || 0, +y || 0);
    }

    transform(a, b, c, d, e, f) {
    	let matrix = new Buffer(libcairo.sizeOf('cairo_matrix_t'));
        cairo.matrix_init(matrix, +a||0, +b||0, +c||0, +d||0, +e||0, +f||0);
        cairo.transform(this._context, matrix);
    }

    setTransform() {
        cairo.identity_matrix(this._context);
        this.transform.apply(this, arguments);
    }

    fill(preserve) {
        let ctx = this._context;
        let state = this.state;
        if (state.fillPattern) {
            cairo.set_source(ctx, state.fillPattern._pattern);
            cairo.pattern_set_extend(cairo.get_source(ctx), cairo.CAIRO_EXTEND_REPEAT);
            // TODO repeat/repeat-x/repeat-y
        } else if (state.fillGradient) {
            cairo.pattern_set_filter(state.fillGradient._pattern, state.patternQuality);
            cairo.set_source(ctx, state.fillGradient._pattern);
        } else {
            _setSourceRGBA(ctx, state.fill, state.globalAlpha);
        }

        if (preserve) {
            if (_hasShadow(state))
                _shadow(ctx, state, cairo.fill_preserve);
            else
                cairo.fill_preserve(ctx);
        } else {
            if (_hasShadow(state))
                _shadow(ctx, state, cairo.fill);
            else
                cairo.fill(ctx);
        }
    }

    stroke(preserve) {
        let ctx = this._context;
        let state = this.state;
        if (state.strokePattern) {
            cairo.set_source(ctx, state.strokePattern._pattern);
            cairo.pattern_set_extend(cairo.get_source(ctx), cairo.CAIRO_EXTEND_REPEAT);
        } else if (state.strokeGradient) {
            cairo.pattern_set_filter(state.strokeGradient._pattern, state.patternQuality);
            cairo.set_source(ctx, state.strokeGradient._pattern);
        } else {
            _setSourceRGBA(ctx, state.stroke, state.globalAlpha);
        }
        if (preserve) {
            if (_hasShadow(state))
                _shadow(ctx, state, cairo.stroke_preserve);
            else
                cairo.stroke_preserve(ctx);
        } else {
            if (_hasShadow(state))
                _shadow(ctx, state, cairo.stroke);
            else
                cairo.stroke(ctx);
        }
    }

    get font() {
        return this._lastFontString || '10px sans-serif';
    }

    set font(val) {
        if (val && typeof val === 'string') {
            let font = _parseFont(val);
            if (font) {
                this._lastFontString = val;
                _setFont(this._layout, this.state.fontDescription, font);
            }
        }
    }

    /*
     * struct PangoRectangle
     * {
     *  int x;
     *  int y;
     *  int width;
     *  int height;
     * };
     */

    measureText(text) {
        if (typeof text !== 'string')
            throw new TypeError('text is not a string');
        let ink_rect = new Int32Array(4);   // PangoRectangle
        let logical_rect = new Int32Array(4);   // PangoRectangle
        const layout = this._layout;

        pango.layout_set_text(layout, text, -1);
        pango.cairo_update_layout(this._context, layout);
        pango.layout_get_pixel_extents(layout, ink_rect, logical_rect);
        const metrics = _fontMetrics(layout, this.state.fontDescription);

        let x_offset;
        switch (this.state.textAlignment) {
        case 0: // center
            x_offset = logical_rect[2] / 2;
            break;
        case 1: // right
            x_offset = logical_rect[2];
            break;
        default: // left
            x_offset = 0.0;
        }

        let y_offset;
        switch (this.state.textBaseline) {
        case TEXT_BASELINE.alphabetic:
            y_offset = -pango.font_metrics_get_ascent(metrics) / PANGO_SCALE;
            break;
        case TEXT_BASELINE.middle:
            y_offset = -(pango.font_metrics_get_ascent(metrics)
                + pango.font_metrics_get_descent(metrics))/(2.0 * PANGO_SCALE);
            break;
        case TEXT_BASELINE.bottom:
            y_offset = -(pango.font_metrics_get_ascent(metrics)
                 + pango.font_metrics_get_descent(metrics)) / PANGO_SCALE;
            break;
        default:
            y_offset = 0.0;
        }

        // pango/pango-types.h:
        // #define PANGO_ASCENT(rect) (-(rect).y)
        // #define PANGO_DESCENT(rect) ((rect).y + (rect).height)
        // #define PANGO_LBEARING(rect) ((rect).x)
        // #define PANGO_RBEARING(rect) ((rect).x + (rect).width)
        //

        const PANGO_ASCENT = (r) => (-r[1]);
        const PANGO_DESCENT = (r) => (r[1] + r[3]);
        const PANGO_LBEARING = (r) => (r[0]);
        const PANGO_RBEARING = (r) => (r[0] + r[2]);

        const obj = {
            width: logical_rect[2],
            actualBoundingBoxLeft: x_offset - PANGO_LBEARING(logical_rect),
            actualBoundingBoxRight: x_offset + PANGO_RBEARING(logical_rect),
            actualBoundingBoxAscent: -(y_offset + ink_rect[1]),
            actualBoundingBoxDescent: PANGO_DESCENT(ink_rect) + y_offset,
            emHeightAscent: PANGO_ASCENT(logical_rect) - y_offset,
            emHeightDescent: PANGO_DESCENT(logical_rect) + y_offset,
            alphabeticBaseline: pango.font_metrics_get_ascent(metrics) / PANGO_SCALE
                                    + y_offset
        };

        pango.font_metrics_unref(metrics);
        return obj;
    }

    _setTextPath(str, x, y) {
        let ink_rect = new Int32Array(4);   // PangoRectangle
        let logical_rect = new Int32Array(4);   // PangoRectangle
        let metrics = null; // PangoFontMetrics
        const layout = this._layout;

        pango.layout_set_text(layout, str, -1);
        pango.cairo_update_layout(this._context, layout);

        switch (this.state.textAlignment) {
        // center
        case 0:
            pango.layout_get_pixel_extents(layout, ink_rect, logical_rect);
            x -= logical_rect[2] / 2;
            break;
        // right
        case 1:
            pango.layout_get_pixel_extents(layout, ink_rect, logical_rect);
            x -= logical_rect[2];
            break;
        }

        switch (this.state.textBaseline) {
        case TEXT_BASELINE.alphabetic:
            metrics = _fontMetrics(layout, this.state.fontDescription);
            y -= pango.font_metrics_get_ascent(metrics) / PANGO_SCALE;
            break;
        case TEXT_BASELINE.middle:
            metrics = _fontMetrics(layout, this.state.fontDescription);
            y -= (pango.font_metrics_get_ascent(metrics)
                    + pango.font_metrics_get_descent(metrics))/(2.0 * PANGO_SCALE);
            break;
        case TEXT_BASELINE.bottom:
            metrics = _fontMetrics(layout, this.state.fontDescription);
            y -= (pango_font_metrics_get_ascent(metrics)
                    + pango_font_metrics_get_descent(metrics)) / PANGO_SCALE;
            break;
        }
        if (metrics !== null)
            pango.font_metrics_unref(metrics);

        cairo.move_to(this._context, x, y);
        if (this.state.textDrawingMode === TEXT_DRAW_PATHS) {
            pango.cairo_layout_path(this._context, layout);
        } else if (this.state.textDrawingMode === TEXT_DRAW_GLYPHS) {
            pango.cairo_show_layout(this._context, layout);
        }
    }

    fillText(text, x, y) {
    	text = text + '';
        x = +x || 0;
        y = +y || 0;
        let path = _savePath(this._context);
        if (this.state.textDrawingMode === TEXT_DRAW_GLYPHS) {
            this.fill(false);
            this._setTextPath(text, x, y);
        } else if (this.state.textDrawingMode === TEXT_DRAW_PATHS) {
            this._setTextPath(text, x, y);
            this.fill(false);
        }
        _restorePath(this._context, path);
    }

    strokeText(text, x, y) {
        text = text + '';
        x = +x || 0;
        y = +y || 0;
        let path = _savePath(this._context);
        if (this.state.textDrawingMode === TEXT_DRAW_GLYPHS) {
            this.stroke(false);
            this._setTextPath(text, x, y);
        } else if (this.state.textDrawingMode === TEXT_DRAW_PATHS) {
            this._setTextPath(text, x, y);
            this.stroke(false);
        }
        _restorePath(this._context, path);
    }

    moveTo(x, y) {
        cairo.move_to(this._context, +x || 0, +y || 0);
    }

    lineTo(x, y) {
        cairo.line_to(this._context, +x || 0, +y || 0);
    }

    rect(x, y, width, height) {
        x = +x || 0;
        y = +y || 0;
        width = +width || 0;
        height = +height || 0;
        if (width == 0) {
		    cairo.move_to(this._context, x, y);
		    cairo.line_to(this._context, x, y + height);
	    } else if (height == 0) {
		    cairo.move_to(this._context, x, y);
		    cairo.line_to(this._context, x + width, y);
	    } else {
		    cairo.rectangle(this._context, x, y, width, height);
	    }
    }

    fillRect(x, y, width, height) {
        x = +x || 0;
        y = +y || 0;
        width = +width || 0;
        height = +height || 0;
        if (width > 0 && height > 0) {
            let path = _savePath(this._context);
            cairo.rectangle(this._context, x, y, width, height);
            this.fill(false);
            _restorePath(this._context, path);
        }
    }

    strokeRect(x, y, width, height) {
        x = +x || 0;
        y = +y || 0;
        width = +width || 0;
        height = +height || 0;
        if (width > 0 && height > 0) {
            let path = _savePath(this._context);
            cairo.rectangle(this._context, x, y, width, height);
            this.stroke(false);
            _restorePath(this._context, path);
        }
    }

    clearRect(x, y, width, height) {
    	x = +x || 0;
    	y = +y || 0;
    	width = +width || 0;
    	height = +height || 0;
    	if (width > 0 && height > 0) {
            let ctx = this._context;
            cairo.save(ctx);
            let path = _savePath(ctx);
            cairo.rectangle(ctx, x, y, width, height);
            cairo.set_operator(ctx, cairo.CAIRO_OPERATOR_CLEAR);
            this.fill(false);
            _restorePath(ctx, path);
            cairo.restore(ctx);
        }
    }

    arc(xc, yc, radius, angle1, angle2, anticlockwise) {
    	angle1 = +angle1 || 0;
    	angle2 = +angle2 || 0;
    	xc = +xc || 0;
    	yc = +yc || 0;
    	radius = +radius || 0;
        if (anticlockwise && Math.PI * 2 != angle2) {
            cairo.arc_negative(this._context,
                    xc, yc, radius, angle1, angle2);
        } else {
            cairo.arc(this._context,
                    xc, yc, radius, angle1, angle2);
        }
    }

    arcTo(x1, y1, x2, y2, radius) {
    	x1 = +x1 || 0;
    	y1 = +y1 || 0;
    	x2 = +x2 || 0;
    	y2 = +y2 || 0;
    	radius = +radius || 0;

    	let Point = (x0, y0) => ({ x: x0, y: y0 });
    	let ctx = this._context;
    	// Current path point
    	let px_ = new Float64Array(1);
    	let py_ = new Float64Array(1);
    	cairo.get_current_point(ctx, px_, py_);

        let p0 = Point(px_[0], py_[0]);
        let p1 = Point(x1, y1);
        let p2 = Point(x2, y2);
        if ((p1.x === p0.x && p1.y === p0.y) || (p1.x === p2.x && p1.y === p2.y)
                || radius === 0) {
            cairo.line_to(ctx, p1.x, p1.y);
            return;
        }

        let p1p0 = Point((p0.x - p1.x), (p0.y - p1.y));
        let p1p2 = Point((p2.x - p1.x), (p2.y - p1.y));
        let p1p0_length = Math.sqrt(p1p0.x * p1p0.x + p1p0.y * p1p0.y);
        let p1p2_length = Math.sqrt(p1p2.x * p1p2.x + p1p2.y * p1p2.y);

        let cos_phi = (p1p0.x * p1p2.x + p1p0.y * p1p2.y) / (p1p0_length * p1p2_length);
        // all points on a line logic
        if (-1 === cos_phi) {
            cairo.line_to(ctx, p1.x, p1.y);
            return;
        }

        if (1 === cos_phi) {
            // add infinite far away point
            let max_length = 65535;
            let factor_max = max_length / p1p0_length;
            let ep = Point((p0.x + factor_max * p1p0.x), (p0.y + factor_max * p1p0.y));
            cairo.line_to(ctx, ep.x, ep.y);
            return;
        }

        let tangent = radius / Math.tan(Math.acos(cos_phi) / 2);
        let factor_p1p0 = tangent / p1p0_length;
        let t_p1p0 = Point((p1.x + factor_p1p0 * p1p0.x), (p1.y + factor_p1p0 * p1p0.y));
        let orth_p1p0 = Point(p1p0.y, -p1p0.x);
        let orth_p1p0_length = Math.sqrt(orth_p1p0.x * orth_p1p0.x
                                    + orth_p1p0.y * orth_p1p0.y);
        let factor_ra = radius / orth_p1p0_length;

        let cos_alpha = (orth_p1p0.x * p1p2.x + orth_p1p0.y * p1p2.y)
					/ (orth_p1p0_length * p1p2_length);
        if (cos_alpha < 0)
            orth_p1p0 = Point(-orth_p1p0.x, -orth_p1p0.y);

        let p = Point((t_p1p0.x + factor_ra * orth_p1p0.x),
				    (t_p1p0.y + factor_ra * orth_p1p0.y));

        orth_p1p0 = Point(-orth_p1p0.x, -orth_p1p0.y);
        let sa = Math.acos(orth_p1p0.x / orth_p1p0_length);
        if (orth_p1p0.y < 0)
            sa = 2 * Math.PI - sa;

        let anticlockwise = false;
        let factor_p1p2 = tangent / p1p2_length;
        let t_p1p2 = Point((p1.x + factor_p1p2 * p1p2.x),
    					(p1.y + factor_p1p2 * p1p2.y));
        let orth_p1p2 = Point((t_p1p2.x - p.x), (t_p1p2.y - p.y));
        let orth_p1p2_length = Math.sqrt(orth_p1p2.x * orth_p1p2.x
                                + orth_p1p2.y * orth_p1p2.y);
        let ea = Math.acos(orth_p1p2.x / orth_p1p2_length);

        if (orth_p1p2.y < 0) ea = 2 * Math.PI - ea;
        if ((sa > ea) && ((sa - ea) < Math.PI)) anticlockwise = true;
        if ((sa < ea) && ((ea - sa) > Math.PI)) anticlockwise = true;

        cairo.line_to(ctx, t_p1p0.x, t_p1p0.y);

        if (anticlockwise && Math.PI * 2 != radius) {
            cairo.arc_negative(ctx, p.x, p.y, radius, sa, ea);
        } else {
            cairo.arc(ctx, p.x, p.y, radius, sa, ea);
        }
    }

    bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y) {
    	cairo.curve_to(this._context,
		    +cp1x||0, +cp1y||0, +cp2x||0, +cp2y||0, +x||0, +y||0);
    }

    quadraticCurveTo(x1, y1, x2, y2) {
    	x1 = +x1 || 0;
    	y1 = +y1 || 0;
    	x2 = +x2 || 0;
    	y2 = +y2 || 0;
    	let px = new Float64Array(1);
        let py = new Float64Array(1);
        cairo.get_current_point(this._context, px, py);
    	let x = px[0];
        let y = py[0];
        if (0 === x && 0 === y) {
            x = x1;
            y = y1;
        }
        cairo.curve_to(this._context,
            x  + 2.0 / 3.0 * (x1 - x),  y  + 2.0 / 3.0 * (y1 - y),
            x2 + 2.0 / 3.0 * (x1 - x2), y2 + 2.0 / 3.0 * (y1 - y2),
            x2,
            y2
        );
    }

    createLinearGradient(x0, y0, x1, y1) {
	    return new CanvasGradient(
		    cairo.pattern_create_linear(+x0||0, +y0||0, +x1||0, +y1||0));
    }

    createRadialGradient(x0, y0, r0, x1, y1, r1) {
	    return new CanvasGradient(
		    cairo.pattern_create_radial(+x0||0, +y0||0, +r0||0,
                +x1||0, +y1||0, +r1||0));
    }

    /*
     * createPattern(image,"repeat|repeat-x|repeat-y|no-repeat");
     *	repeats the image/canvas element in the specified direction.
     * "repeat" 	Default. The pattern repeats both horizontally and vertically
     */

    createPattern(image) {
	    if (! ('object' === typeof image
			    && (image instanceof Image || image instanceof Canvas))
	    )
		    throw new TypeError('not a Canvas or Image');
    	return new CanvasPattern(image._surface);
    }

    get lineCap() {
        let cap = cairo.get_line_cap(this._context);
        if (cap === cairo.CAIRO_LINE_CAP_ROUND) {
            return 'round';
        }
        if (cap === cairo.CAIRO_LINE_CAP_SQUARE) {
            return 'square';
        }
        return 'butt';
    }

    set lineCap(val) {
        let cap = cairo.CAIRO_LINE_CAP_BUTT;
        val = val + '';
        if (val === 'round') {
            cap = cairo.CAIRO_LINE_CAP_ROUND;
        } else if (val === 'square') {
            cap = cairo.CAIRO_LINE_CAP_SQUARE;
        }
        cairo.set_line_cap(this._context, cap);
    }

    get lineJoin() {
        let j = cairo.get_line_join(this._context);
        if (j === cairo.CAIRO_LINE_JOIN_ROUND) {
            return 'round';
        }
        if (j === cairo.CAIRO_LINE_JOIN_BEVEL) {
            return 'bevel';
        }
        return 'miter';
    }

    set lineJoin(val) {
        let j = cairo.CAIRO_LINE_JOIN_MITER;
        val = val + '';
        if (val === 'round') {
            j = cairo.CAIRO_LINE_JOIN_ROUND;
        } else if (val === 'bevel') {
            j = cairo.CAIRO_LINE_JOIN_BEVEL;
        }
        cairo.set_line_join(this._context, j);
    }

    get lineWidth() {
        return cairo.get_line_width(this._context);
    }

    set lineWidth(val) {
        val = +val || 0;
        if (val > 0)
            cairo.set_line_width(this._context, val);
    }

    get miterLimit() {
        return cairo.get_miter_limit(this._context);
    }

    set miterLimit(val) {
        val = +val || 0;
        if (val > 0)
            cairo.set_miter_limit(this._context, val);
    }

    get globalAlpha() {
        return this.state.globalAlpha;
    }

    set globalAlpha(val) {
        val = +val || 0;
        if (val >= 0 && val <= 1.0)
            this.state.globalAlpha = val;
    }

    get fillStyle() {
        return this.state.fillGradient
            || this.state.fillPattern
            || rgba_to_str(this.state.fill);
    }

    set fillStyle(val) {
        if (typeof val === 'object'
                && val instanceof CanvasGradient) {
            this.state.fillGradient = val;
            this.state.fillPattern = null;
        } else if (typeof val === 'object'
                && val instanceof CanvasPattern) {
            this.state.fillPattern = val;
            this.state.fillGradient = null;
        } else {
            let color = rgba_from_str(val + '');
            if (color !== null)
                this.state.fill = color;
        }
    }

    get strokeStyle() {
        return this.state.strokeGradient
            || this.state.strokePattern
            || rgba_to_str(this.state.stroke);
    }

    set strokeStyle(val) {
        if ('object' === typeof val
                && val instanceof CanvasGradient) {
            this.state.strokeGradient = val;
            this.state.strokePattern = null;
        } else if ('object' === typeof val
                && val instanceof CanvasPattern) {
            this.state.strokePattern = val;
            this.state.strokeGradient = null;
        } else {
            let color = rgba_from_str(val + '');
            if (color !== null)
                this.state.stroke = color;
        }
    }

    get textBaseline() {
        for (let val in TEXT_BASELINE) {
            if (TEXT_BASELINE[val] === this.state.textBaseline)
                return val;
        }
    }

    set textBaseline(val) {
        if (val && typeof val === 'string'
                && TEXT_BASELINE.hasOwnProperty(val)) {
            this.state.textBaseline = TEXT_BASELINE[val];
        }
    }

    get textAlign() {
        if (this.state.textAlignment === -1) {
            return 'start';
        }
        if (this.state.textAlignment === 1) {
            return 'end';
        }
        return 'center';
    }

    set textAlign(val) {
        val = val + '';
        switch (val) {
        case 'center':
            this.state.textAlignment = 0;
            break;
        case 'left':
        case 'start':
            this.state.textAlignment = -1;
            break;
        case 'right':
        case 'end':
            this.state.textAlignment = 1;
            break;
        }
    }

    get shadowBlur() {
        return this.state.shadowBlur;
    }

    set shadowBlur(val) {
        val = +val || 0;
        if (val >= 0)
            this.state.shadowBlur = val;
    }

    get shadowColor() {
        return rgba_to_str(this.state.shadow);
    }

    set shadowColor(val) {
        if ('string' === typeof val) {
            let color = rgba_from_str(val);
            if (color !== null)
                this.state.shadow = color;
        }
    }

/**
 * set or return the horizontal distance of the shadow from the shape.
 * shadowOffsetX=0 indicates that the shadow is right behind the shape.
 * shadowOffsetX=20 indicates that the shadow starts 20 pixels to the right
 *	(from the shape's left position).
 * shadowOffsetX=-20 indicates that the shadow starts 20 pixels to the left
 *	(from the shape's left position).
 */

    get shadowOffsetX() {
        return this.state.shadowOffsetX;
    }

    set shadowOffsetX(val) {
        if ('number' === typeof val)
            this.state.shadowOffsetX = val;
    }

/**
 * set or return the vertical distance of the shadow from the shape.
 * shadowOffsety=0 indicates that the shadow is right behind the shape.
 * shadowOffsetY=20 indicates that the shadow starts 20 pixels
 *	below the shape's top position.
 * shadowOffsetY=-20 indicates that the shadow starts 20 pixels
 *	above the shape's top position.
 */

    get shadowOffsetY() {
        return this.state.shadowOffsetY;
    }

    set shadowOffsetY(val) {
        if ('number' === typeof val)
            this.state.shadowOffsetY = val;
    }

    get antiAlias() {
        const aa = cairo.get_antialias(this._context);
        switch (aa) {
        case cairo.CAIRO_ANTIALIAS_NONE:
            return "none";
        case cairo.CAIRO_ANTIALIAS_GRAY:
            return "gray";
        case cairo.CAIRO_ANTIALIAS_SUBPIXEL:
            return "subpixel";
        defualt:
            break;
        }
        return "default";
    }

    set antiAlias(val) {
        val = val+'';
        let aa = cairo.CAIRO_ANTIALIAS_DEFAULT;
        switch (val) {
        case 'none':
            aa = cairo.CAIRO_ANTIALIAS_NONE;
            break;
        case 'gray':
            aa = cairo.CAIRO_ANTIALIAS_GRAY;
            break;
        case 'subpixel':
            aa = cairo.CAIRO_ANTIALIAS_SUBPIXEL;
            break;
        }
        cairo.set_antialias(this._context, aa);
    }

    setLineDash(segments) {
        if (!Array.isArray(segments))
            return;
        let num_dashes = segments.length;
        if (num_dashes&1)
            num_dashes *= 2;
        let dashes = new Float64Array(num_dashes);
        for(let i = 0; i < num_dashes; i++) {
            let d = segments[i%segments.length];
            if (typeof d !== 'number' || !isFinite(d))
                return;
            dashes[i] = d;
        }
        let offset = new Float64Array(1);
        cairo.get_dash(this._context, $nullptr, offset);
        cairo.set_dash(this._context, dashes, num_dashes, offset[0]);
    }

    getLineDash() {
        let num_dashes = cairo.get_dash_count(this._context);
        if (num_dashes === 0)
            return [];
        let dashes = new Float64Array(num_dashes);
        cairo.get_dash(this._context, dashes, $nullptr);
        return Array.from(dashes);
    }

    set lineDashOffset(val) {
        let offset = +val||0;
        let num_dashes = cairo.get_dash_count(this._context);
        num_dashes++;
        let dashes = new Float64Array(num_dashes);
        cairo.get_dash(this._context, dashes, $nullptr);
        cairo.set_dash(this._context, dashes, num_dashes - 1, offset);
    }

    get lineDashOffset() {
        let offset = new Float64Array(1);
        cairo.get_dash(this._context, $nullptr, offset);
        return offset[0];
    }
}


/**********************************************
 *  CanvasGradient
 **********************************************/

class CanvasGradient{
    constructor(pattern) {
        this._pattern = pattern;
        pattern.gc(cairo.pattern_destroy);
    }

    /**
     * Specifies the colors and position in a gradient object.
     *	stop    A value between 0.0 and 1.0 that represents the position
     *	        between start and end in a gradient
     *	color   A CSS color value to display at the stop position
     */

    addColorStop(position, cval) {
        let color = null;
        position = +position || 0;
        if (position < 0)
            position = 0;
        else if (position > 1.0)
            position = 1.0;
        if (typeof cval === 'string') {
            color = rgba_from_str(cval);
            if (color !== null) {
                cairo.pattern_add_color_stop_rgba(
                    this._pattern, position,
                    color.red, color.green, color.blue, color.alpha
                );
            }
        }
        if (color === null)
            throw new TypeError('invalid color format');
    }
}2

/**********************************************
 *  CanvasPattern
 **********************************************/

class CanvasPattern {
    constructor(surface) {
        this._pattern = cairo.pattern_create_for_surface(surface);
        this._pattern.gc(cairo.pattern_destroy);
    }	
}


exports = module.exports = Context2d;
