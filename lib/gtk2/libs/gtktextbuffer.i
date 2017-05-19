const GtkTextBuffer = Object.create(GTYPES['GObject'].proto);

function initGtkTextBuffer(wptr) {
    if (!GtkTextBuffer.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkTextBuffer);
    GTYPES['GObject'].init(wptr);
}

register_gtype('GtkTextBuffer', GtkTextBuffer,
        gtklib.get_type(), initGtkTextBuffer);

module.exports = function() {
    const tb = gtklib.new($nullptr);
    initGtkTextBuffer(tb);
    return tb;
};
