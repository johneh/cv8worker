ctype('GtkToolItem', 'GtkContainer', lib.get_type());

module.exports = function() {
    let ti = lib.new();
    CTYPES['GtkToolItem'].init(ti);
    return ti;
};
