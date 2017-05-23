//###############################################
// GtkTextView <- GtkContainer <- GtkWidget.
//###############################################
ctype('GtkTextView', 'GtkContainer', lib.get_type());

module.exports = function(textbuf) {
    let textview;
    if (textbuf && isa(textbuf, 'GtkTextBuffer')) {
        textview = lib.new_with_buffer(textbuf); // increments ref. count.
    } else {
        textview = lib.new();
    }
    CTYPES['GtkTextView'].init(textview);
    return textview;
};
