/*
 * N.B.:
 * GTK_DIALOG_MODAL -> use gtk_window_set_modal().
 * GTK_DIALOG_DESTROY_WITH_PARENT -> use gtk_window_set_destroy_with_parent().
 */

ctype('GtkDialog', 'GtkWindow', lib.get_type());

module.exports = function() {
    let dlg = lib.new();
    CTYPES['GtkDialog'].init(dlg);
    return dlg;
};
