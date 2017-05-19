//###############################################################
// GtkEntry <- GtkWidget.
//###############################################################

const GtkEntry = Object.create(GTYPES['GtkWidget'].proto);

function initGtkEntry(wptr) {
    if (!GtkEntry.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkEntry);
    GTYPES['GtkWidget'].init(wptr);
}

register_gtype('GtkEntry', GtkEntry, gtklib.get_type(), initGtkEntry);

module.exports = function(b) {
    let e;
    if (GTYPES['GtkEntryBuffer'].proto.isPrototypeOf(b)) {
        e = gtklib.new_with_buffer(b);
    } else {
        e = gtklib.new();
    }
    initGtkEntry(e);
    return e;
};

OVERRIDES.get_layout_offsets = function() {
    let b = $malloc(8);
    gtklib.get_layout_offsets(this,b,b.offsetAt(4));
    let [x, y] = $unpack(b, 0, 'ii');
    return [x, y];
};

