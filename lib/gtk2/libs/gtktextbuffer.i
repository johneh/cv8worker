ctype('GtkTextBuffer', 'GObject', lib.get_type());

module.exports = function() {
    const tb = lib.new($nullptr);
    CTYPES['GtkTextBuffer'].init(tb);
    return tb;
};
