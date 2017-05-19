const GtkAdjustment = Object.create(GTYPES['GObject'].proto);

function initGtkAdjustment(wptr) {
    if (!GtkAdjustment.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkAdjustment);
    GTYPES['GObject'].init(wptr);
}

register_gtype('GtkAdjustment', GtkAdjustment,
            gtklib.get_type(), initGtkAdjustment);

module.exports = function (val, lower, upper,
        step_increment, page_increment, page_size) {
    let a = gtklib.new(val+0.0, lower+0.0, upper+0.0,
                step_increment+0.0, page_increment+0.0, page_size+0.0);
    initGtkAdjustment(a);
    return a;
}
