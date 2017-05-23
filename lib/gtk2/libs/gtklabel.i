//########################################
// GtkLabel <- GtkMisc <- GtkWidget.
//########################################

ctype('GtkLabel', 'GtkMisc', lib.get_type());

module.exports = function(text) {
    let w;
    if (typeof text === 'string') {
        w = lib.new(text);
    } else {
        w = lib.new($nullptr);
    }
    CTYPES['GtkLabel'].init(w);
    return w;
};
