var cairo = $load('./libcairojs.so');
var surface = cairo.image_surface_create(
                    cairo.CAIRO_FORMAT_ARGB32, 120, 100).notNull();
var cr = cairo.create(surface).notNull();
cairo.set_source_rgb(cr, 1, 0, 0);
cairo.rectangle(cr, 10, 10, 100, 80);
cairo.fill(cr); 
cairo.surface_write_to_png(surface, "rect.png");
cairo.destroy(cr);
cairo.surface_destroy(surface);
