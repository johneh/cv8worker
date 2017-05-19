//###############################################################
// GtkScrolledWindow <- GtkContainer <- GtkWidget.
//###############################################################

const GtkScrolledWindow = Object.create(GTYPES['GtkContainer'].proto);

function initGtkScrolledWindow(wptr) {
    if (!GtkScrolledWindow.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkScrolledWindow);
    GTYPES['GtkContainer'].init(wptr);
}

register_gtype('GtkScrolledWindow', GtkScrolledWindow,
        gtklib.get_type(), initGtkScrolledWindow);

module.exports = function() {
    let sw = gtklib.new($nullptr, $nullptr);
    initGtkScrolledWindow(sw);
    return sw;
};
