//###############################################
// GtkTextView <- GtkContainer <- GtkWidget.
//###############################################

const GtkTextView = Object.create(GTYPES['GtkContainer'].proto);

function initGtkTextView(wptr) {
    if (!GtkTextView.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkTextView);
    GTYPES['GtkContainer'].init(wptr);
}

register_gtype('GtkTextView', GtkTextView, gtklib.get_type(), initGtkTextView);

module.exports = function(textbuf) {
    let textview;
    if (textbuf && GTYPES['GtkTextBuffer'].proto.isPrototypeOf(textbuf)) {
        textview = gtklib.new_with_buffer(textbuf); // increments ref. count.
    } else {
        textview = gtklib.new();
    }
    initGtkTextView(textview);
    return textview;
};
