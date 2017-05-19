//########################################
// GtkButton <- GtkContainer <- GtkWidget.
//########################################

const GtkButton = Object.create(GTYPES['GtkContainer'].proto);

function initGtkButton(wptr) {
    if (!GtkButton.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkButton);
    GTYPES['GtkContainer'].init(wptr);
}

register_gtype('GtkButton', GtkButton, gtklib.get_type(), initGtkButton);

module.exports = function(label) {
    if (typeof label !== 'string' || label.length === 0)
        label = "?";
    let button = gtklib.new_with_label(label);
    initGtkButton(button);
    return button;
};
