//########################################
// GtkContainer <- GtkWidget.
//########################################

const GtkContainer = Object.create(GTYPES['GtkWidget'].proto);

function initGtkContainer(wptr) {
    if (!GtkContainer.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkContainer);
    GTYPES['GtkWidget'].init(wptr);
}

register_gtype('GtkContainer', GtkContainer,
        gtklib.get_type(), initGtkContainer);
