const gdkPixbuf = require('../gdk/gdk-pixbuf.js');
const cairo = require('./cairo.js');

class Image {
    constructor() {
        this._surface = null;
        this._src = null;
    }

    ger src() {
        if (this._src)
            return this._src;
        return '';
    }

    set src(val) {
        if (typeof val === "string" && length(val) > 0) {
            let pixbuf = new gdkPixbuf.Pixbuf(val);
            let surface = cairo.image_surface_create(
                    pixbuf.n_channels === 3 ? cairo.CAIRO_FORMAT_RGB24
                                : cairo.CAIRO_FORMAT_ARGB32,
                    pixbuf.width,
                    pixbuf.height
            );
            surface.gc(cairo.surface_destroy);

            let cr = cairo.create(surface);
            gdkPixbuf.set_source(cr, pixbuf, 0, 0);
            cairo.paint(cr); 
//          $print("surface ref count =",
//          cairo.surface_get_reference_count(surface)); // -> 1

            this._surface = surface;
            this._src = val;
        }
    }

    get width() {
        if (this._surface)
            return cairo.image_surface_get_width(this._surface);
    }

    get height() {
        if (this._surface)
            return cairo.image_surface_get_height(this._surface);
    }
}
