//#####################################################
// GtkWindow (<- GtkBin) <- GtkContainer <- GtkWidget.
//#####################################################

const gtkenums = require('./gtkenums.so');

const GtkWindow = Object.create(GTYPES['GtkContainer'].proto);

function initGtkWindow(wptr) {
    if (!GtkWindow.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkWindow);
    GTYPES['GtkContainer'].init(wptr);
}


register_gtype('GtkWindow', GtkWindow, gtklib.get_type(), initGtkWindow);

module.exports = function(wintype = gtkenums.GTK_WINDOW_TOPLEVEL) {
    let w = gtklib.new(wintype);
    initGtkWindow(w);
    return w;
};
