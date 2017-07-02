const dll = require('./libcairo.so', '-dll');
const cairo_xlib = $loadlib('./libcairoxlib.so');
const cairo = dll.identifiers;

const _create = cairo.create;
const _destroy = cairo.destroy;

delete cairo.destroy;

// typed pointer
function cairo_t() {
}

cairo_t.prototype = Object.create($nullptr.__proto__);   // inheritance
cairo_t.prototype.constructor = cairo_t; // XXX: constructor.name === 'Object' without it 

cairo.create = function(surface) {
    let cr = _create(surface);
    Object.setPrototypeOf(cr, cairo_t.prototype);
    cr.gc(_destroy);
    // console.log(cr.constructor.name); // -> cairo_t
    return cr;
};

cairo._init = function (cr) {
    Object.setPrototypeOf(cr, cairo_t.prototype);
    cr.gc(_destroy);
    // console.log(cr.constructor.name); // -> cairo_t
};

cairo.xlib = cairo_xlib;

exports = module.exports = cairo;

