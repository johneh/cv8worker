const libcairo = new DLL('./libcairo.so', __dirname);
const cairo_xlib = $loadlib('./libcairoxlib.so');

const _create = libcairo.identifiers.create;
const _destroy = libcairo.identifiers.destroy;

delete libcairo.identifiers.destroy;

// typed pointer
function cairo_t() {
}

cairo_t.prototype = Object.create($nullptr.__proto__);   // inheritance
cairo_t.prototype.constructor = cairo_t;

libcairo.identifiers.create = function(surface) {
    let cr = _create(surface);
    Object.setPrototypeOf(cr, cairo_t.prototype);
    cr.gc(_destroy);
    // console.log(cr.constructor.name); //      -> cairo_t
    return cr;
}

libcairo.identifiers.xlib = cairo_xlib;

exports = module.exports = libcairo;

