//###############################################
// GtkBox <- GtkContainer <- GtkWidget.
//###############################################

const GtkBox = Object.create(GTYPES['GtkContainer'].proto);

function initGtkBox(wptr) {
    if (!GtkBox.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkBox);
    GTYPES['GtkContainer'].init(wptr);
}

register_gtype('GtkBox', GtkBox, gtklib.get_type(), initGtkBox);
