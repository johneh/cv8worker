//########################################
// GtkButton <- GtkContainer <- GtkWidget.
//########################################

ctype('GtkButton', 'GtkContainer', lib.get_type());

module.exports = function(label) {
    if (typeof label !== 'string' || label.length === 0)
        label = "?";
    let button = lib.new_with_label(label);
    CTYPES['GtkButton'].init(button);
    return button;
};
