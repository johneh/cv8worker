ctype('GtkDrawingArea', 'GtkWidget', lib.get_type());

module.exports = function() {
    let da = lib.new();
    CTYPES['GtkDrawingArea'].init(da);
    return da;
};
