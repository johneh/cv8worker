ctype('GtkStatusbar', 'GtkHBox', lib.get_type());

module.exports = function() {
    let sb = lib.new();
    CTYPES['GtkStatusbar'].init(sb);
    return sb;
};
