ctype('GtkMenuBar', 'GtkMenuShell', lib.get_type());

module.exports = function() {
    let mb = lib.new();
    CTYPES['GtkMenuBar'].init(mb);
    return mb;
};
