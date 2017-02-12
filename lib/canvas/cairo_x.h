#include <cairo/cairo.h>
#include <stdint.h>
extern void blur(cairo_surface_t *surface, int radius);
extern void get_image_surface_data(cairo_surface_t *surface,
		int sx, int sy, int sw, int sh, uint8_t *dst);
extern void put_image_surface_data(cairo_surface_t *surface,
        uint8_t *src, int width, int height,
		int sx, int sy,
		int dx, int dy, int rows, int cols);

