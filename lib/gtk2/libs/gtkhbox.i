//#######################################################
// GtkHBox <- GtkBox <- GtkContainer <- GtkWidget.
//#######################################################

const GtkHBox = Object.create(GTYPES['GtkBox'].proto);

function initGtkHBox(wptr) {
    if (!GtkHBox.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkHBox);
    GTYPES['GtkBox'].init(wptr);
}

register_gtype('GtkHBox', GtkHBox, gtklib.get_type(), initGtkHBox);

module.exports = function(homogeneous = false, spacing = 0) {
    spacing = spacing|0;
    if (spacing < 0) spacing = 0;
    let hbox = gtklib.new(!!homogeneous, spacing);
    initGtkHBox(hbox);
    return hbox;
};
