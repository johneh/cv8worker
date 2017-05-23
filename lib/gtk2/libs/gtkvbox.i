//#######################################################
// GtkVBox <- GtkBox <- GtkContainer <- GtkWidget.
//#######################################################

ctype('GtkVBox', 'GtkBox', lib.get_type());

module.exports = function(homogeneous = false, spacing = 0) {
    spacing = spacing|0;
    if (spacing < 0) spacing = 0;
    let vbox = lib.new(!!homogeneous, spacing);
    CTYPES['GtkVBox'].init(vbox);
    return vbox;
};
