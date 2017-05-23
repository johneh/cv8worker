ctype('GtkEntryCompletion', 'GObject', lib.get_type());

module.exports = function() {
    const ec = lib.new();
    CTYPES['GtkEntryCompletion'].init(ec);
    return ec;
};
