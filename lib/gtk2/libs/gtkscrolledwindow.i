//###############################################################
// GtkScrolledWindow <- GtkContainer <- GtkWidget.
//###############################################################

ctype('GtkScrolledWindow', 'GtkContainer', lib.get_type());

module.exports = function() {
    let sw = lib.new($nullptr, $nullptr);
    CTYPES['GtkScrolledWindow'].init(sw);
    return sw;
};
