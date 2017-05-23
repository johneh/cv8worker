//###############################################################
// GtkAlignment <- GtkContainer.
//###############################################################
ctype('GtkAlignment', 'GtkContainer', lib.get_type());

module.exports = function(xalign, yalign, xscale, yscale) {
    let a = lib.new(xalign+0.0, yalign+0.0, xscale+0.0, yscale+0.0);
    CTYPES['GtkAlignment'].init(a);
    return a;
};
