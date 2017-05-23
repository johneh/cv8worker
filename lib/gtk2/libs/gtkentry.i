//###############################################################
// GtkEntry <- GtkWidget.
//###############################################################
ctype('GtkEntry', 'GtkWidget', lib.get_type());

module.exports = function(b) {
    let e;
    if (isa(b, 'GtkEntryBuffer')) {
        e = lib.new_with_buffer(b);
    } else {
        e = lib.new();
    }
    CTYPES['GtkEntry'].init(e);
    return e;
};

OVERRIDES.get_layout_offsets = function() {
    let b = $malloc(8);
    lib.get_layout_offsets(this,b,b.offsetAt(4));
    let [x, y] = $unpack(b, 0, 'ii');
    return [x, y];
};

