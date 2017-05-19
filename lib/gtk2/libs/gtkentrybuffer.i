const GtkEntryBuffer = Object.create(GTYPES['GObject'].proto);

function initGtkEntryBuffer(wptr) {
    if ($GtkEntryBuffer.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkEntryBuffer);
    GTYPES['GObject'].init(wptr);
}

register_gtype('GtkEntryBuffer', GtkEntryBuffer,
        gtklib.get_type(), initGtkEntryBuffer);

module.exports = function(s) {
    let eb;
    if (typeof s === 'string')
        eb = gtklib.new(s, s.length);
    else
        eb = gtklib.new($nullptr, 0);
    initGtkEntryBuffer(eb);
    return eb;
};


