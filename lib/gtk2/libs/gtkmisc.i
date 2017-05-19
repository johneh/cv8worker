//########################################
// GtkMisc <- GtkWidget.
//########################################

const GtkMisc = Object.create(GTYPES['GtkWidget'].proto);

function initGtkMisc(wptr) {
    if (!GtkMisc.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkMisc);
    GTYPES['GtkWidget'].init(wptr);
}

register_gtype('GtkMisc', GtkMisc, gtklib.get_type(), initGtkMisc);
