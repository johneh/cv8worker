const GtkEntryCompletion = Object.create(GTYPES['GObject'].proto);

function initGtkEntryCompletion(wptr) {
    if (!GtkEntryCompletion.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkEntryCompletion);
    GTYPES['GObject'].init(wptr);
}

register_gtype('GtkEntryCompletion', GtkEntryCompletion,
        gtklib.get_type(), initGtkEntryCompletion);

module.exports = function() {
    const ec = gtklib.new();
    initGtkEntryCompletion(ec);
    return ec;
};
