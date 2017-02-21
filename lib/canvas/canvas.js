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
    constructor(...args) {
        let w, h;
        if (typeof args[0] === 'object' &&
                args[0].constructor.name === 'Display') {
            if (args.length < 5
                    || !(typeof args[1] === 'object' &&
                args[1].constructor.name === 'Window')
                    || !(typeof args[2] === 'object' &&
                args[2].constructor.name === 'Visual')
            )
                throw new Error("invalid argument(s)");
            w = args[3]|0;
            h = args[4]|0;
            if (w <= 0 || h <= 0)
                throw new Error("invalid width (height)");
            this._type = cairo.CAIRO_SURFACE_TYPE_XLIB;
            this._surface = cairo.xlib.surface_create(args[0],
                        args[1].window, args[2], w, h);
        } else {
            w = args[0]|0;
            h = args[1]|0;
            if (w < 0) w = 300;
            if (h < 0) h = 300;
            this._type = cairo.CAIRO_SURFACE_TYPE_IMAGE;
            this._surface = cairo.image_surface_create(
                cairo.CAIRO_FORMAT_ARGB32, w, h);
        }

        if (cairo.surface_status(this._surface) != 0)
            throw new Error("cannot create cairo surface");
        this._surface.gc(cairo.surface_destroy);
        this._width = w;
        this._height = h;
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
        if (this._type === cairo.CAIRO_SURFACE_TYPE_IMAGE && val >= 0) {
            this._width = val;
            _resurface(this);
        }
    }

    get height() {
        return this._height;
    }

    set height(val) {
        val = +val || 0;
        if (this._type === cairo.CAIRO_SURFACE_TYPE_IMAGE && val >= 0) {
            this._height = val;
            _resurface(this);
        }
    }

    _resize(w, h) {
        w = w|0;
        h = h|0;
        if (w < 0 || h < 0)
            return;
        if (this._type === cairo.CAIRO_SURFACE_TYPE_XLIB) {
            cairo.xlib.surface_set_size(this._surface, w, h);
            this._width = w;
            this._height = h;
            // FIXME -- reset context ?
        } else if (this._type === cairo.CAIRO_SURFACE_TYPE_IMAGE) {
            this._width = w;
            this._height = h;
            _resurface(this);
        }
    }

    get surface() {
        return this._surface;
    }
}

exports = module.exports = Canvas;
