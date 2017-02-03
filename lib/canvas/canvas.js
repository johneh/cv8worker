const cairo = require('./cairo.js').identifiers;
const Context2d = require('./context2d.js');

function _resurface(canvas) {
    if (canvas._type !== cairo.CAIRO_SURFACE_TYPE_IMAGE)
        throw new Error("not an image surface");
    let s = cairo.image_surface_create(
                cairo.CAIRO_FORMAT_ARGB32, canvas._width, canvas._height);
    if (cairo.surface_status(s) != 0)
        throw new Error("cannot create cairo surface");
    s.gc(cairo.surface_destroy);
    canvas._surface = s;
    // Reset context
    if (canvas._context2d !== null)
        canvas._context2d._resetContext();
}

class Canvas {
    constructor(w, h) {
        w = +w || 300;
        if (w < 0)
            w = 300;
        this._width = w;
        h = +h || 300;
        if (h < 0)
            h = 300;
        this._height = h;
        this._surface = cairo.image_surface_create(
            cairo.CAIRO_FORMAT_ARGB32, this._width, this._height);
        if (cairo.surface_status(this._surface) != 0)
            throw new Error("cannot create cairo surface");
        this._type = cairo.CAIRO_SURFACE_TYPE_IMAGE;
        this._surface.gc(cairo.surface_destroy);
        this._context2d = null;
    }

    getContext(contextId) {
        if ('2d' === contextId) {
            if (this._context2d === null)
                this._context2d = new Context2d(this);
            return this._context2d;
        }
    }

    writePNG(filename) {
        if (filename && typeof filename === 'string')
            cairo.surface_write_to_png(this._surface, filename);
    }

    get width() {
        return this._width;
    }

    set width(val) {
        val = +val || 0;
        if (val >= 0) {
            this._width = val;
            _resurface(this);
        }
    }

    get height() {
        return this._height;
    }

    set height(val) {
        val = +val || 0;
        if (val >= 0) {
            this._height = val;
            _resurface(this);
        }
    }

    get surface() {
        return this._surface;
    }
}

exports = module.exports = Canvas;
