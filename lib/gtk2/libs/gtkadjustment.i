ctype('GtkAdjustment', 'GObject', lib.get_type());

module.exports = function (val, lower, upper,
        step_increment, page_increment, page_size) {
    let a = lib.new(val+0.0, lower+0.0, upper+0.0,
                step_increment+0.0, page_increment+0.0, page_size+0.0);
    CTYPES['GtkAdjustment'].init(a);
    return a;
}
