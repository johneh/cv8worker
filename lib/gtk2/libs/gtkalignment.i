//###############################################################
// GtkAlignment <- GtkContainer.
//###############################################################

const GtkAlignment = Object.create(GTYPES['GtkContainer'].proto);

function initGtkAlignment(wptr) {
    if (!GtkAlignment.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkAlignment);
    GTYPES['GtkContainer'].init(wptr);
}

register_gtype('GtkAlignment', GtkAlignment,
        gtklib.get_type(), initGtkAlignment);

module.exports = function(xalign, yalign, xscale, yscale) {
    let a = gtklib.new(xalign+0.0, yalign+0.0, xscale+0.0, yscale+0.0);
    initGtkAlignment(a);
    return a;
};
