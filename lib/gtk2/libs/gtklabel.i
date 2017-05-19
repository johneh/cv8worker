//########################################
// GtkLabel <- GtkMisc <- GtkWidget.
//########################################

const GtkLabel = Object.create(GTYPES['GtkMisc'].proto);

function initGtkLabel(wptr) {
    if (!GtkLabel.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkLabel);
    GTYPES['GtkMisc'].init(wptr);
}

register_gtype('GtkLabel', GtkLabel, gtklib.get_type(), initGtkLabel);

module.exports = function(text) {
    let w;
    if (typeof text === 'string') {
        w = gtklib.new(text);
    } else {
        w = gtklib.new($nullptr);
    }
    initGtkLabel(w);
    return w;
};
