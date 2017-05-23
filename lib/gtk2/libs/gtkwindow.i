//#####################################################
// GtkWindow (<- GtkBin) <- GtkContainer <- GtkWidget.
//#####################################################

const gtkenums = require('./gtkenums.so');

ctype('GtkWindow', 'GtkContainer', lib.get_type());

module.exports = function(wintype = gtkenums.GTK_WINDOW_TOPLEVEL) {
    let w = lib.new(wintype);
    CTYPES['GtkWindow'].init(w);
    return w;
};
