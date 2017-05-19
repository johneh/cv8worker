//#######################################################
// GtkVBox <- GtkBox <- GtkContainer <- GtkWidget.
//#######################################################

const GtkVBox = Object.create(GTYPES['GtkBox'].proto);

function initGtkVBox(wptr) {
    if (!GtkVBox.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkVBox);
    GTYPES['GtkBox'].init(wptr);
}

register_gtype('GtkVBox', GtkVBox, gtklib.get_type(), initGtkVBox);

module.exports = function(homogeneous = false, spacing = 0) {
    spacing = spacing|0;
    if (spacing < 0) spacing = 0;
    let vbox = gtklib.new(!!homogeneous, spacing);
    initGtkVBox(vbox);
    return vbox;
};
