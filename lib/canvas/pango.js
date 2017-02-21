const pango = $loadlib('./libpango.so');
pango.PANGO_SCALE = 1024;   // See pango/pango-types.h

exports = module.exports = pango;
