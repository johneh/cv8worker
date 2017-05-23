ctype('GtkEntryBuffer', 'GObject', lib.get_type());

module.exports = function(s) {
    let eb;
    if (typeof s === 'string')
        eb = lib.new(s, s.length);
    else
        eb = lib.new($nullptr, 0);
    CTYPES['GtkEntryBuffer'].init(eb);
    return eb;
};


