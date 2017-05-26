ctype('GtkEventBox', 'GtkContainer', lib.get_type());

module.exports = function() {
    let eb = lib.new();
    CTYPES['GtkEventBox'].init(eb);
    return eb;
};
