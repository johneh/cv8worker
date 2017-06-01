ctype('GtkToolbar', 'GtkContainer', lib.get_type());

module.exports = function() {
    let tb = lib.new();
    CTYPES['GtkToolbar'].init(tb);
    return tb;
};
