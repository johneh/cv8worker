#include <gdk-pixbuf/gdk-pixbuf.h>
#include "jsv8dlfn.h"

static v8_val do_error_quark(v8_state vm, int argc, v8_val argv[]) {
unsigned int r = (unsigned int) gdk_pixbuf_error_quark();
return V8_UINT32(r);
}
static v8_val do_get_type(v8_state vm, int argc, v8_val argv[]) {
double r = (double) gdk_pixbuf_get_type();
return V8_DOUBLE(r);
}
static v8_val do_get_colorspace(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_colorspace(p1);
return V8_INT32(r);
}
static v8_val do_get_n_channels(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_n_channels(p1);
return V8_INT32(r);
}
static v8_val do_get_has_alpha(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_has_alpha(p1);
return V8_INT32(r);
}
static v8_val do_get_bits_per_sample(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_bits_per_sample(p1);
return V8_INT32(r);
}
static v8_val do_get_pixels(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_get_pixels(p1);
return V8_PTR(r);
}
static v8_val do_get_width(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_width(p1);
return V8_INT32(r);
}
static v8_val do_get_height(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_height(p1);
return V8_INT32(r);
}
static v8_val do_get_rowstride(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_rowstride(p1);
return V8_INT32(r);
}
static v8_val do_get_byte_length(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
double r = (double) gdk_pixbuf_get_byte_length(p1);
return V8_DOUBLE(r);
}
static v8_val do_get_pixels_with_length(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_get_pixels_with_length(p1,p2);
return V8_PTR(r);
}
static v8_val do_new(v8_state vm, int argc, v8_val argv[]) {
int p1 = V8_TOINT32(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
int p5 = V8_TOINT32(argv[4]);
void *r = (void *) gdk_pixbuf_new(p1,p2,p3,p4,p5);
return V8_PTR(r);
}
static v8_val do_copy(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_copy(p1);
return V8_PTR(r);
}
static v8_val do_new_subpixbuf(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
int p5 = V8_TOINT32(argv[4]);
void *r = (void *) gdk_pixbuf_new_subpixbuf(p1,p2,p3,p4,p5);
return V8_PTR(r);
}
static v8_val do_new_from_file(v8_state vm, int argc, v8_val argv[]) {
char *p1 = V8_TOSTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_new_from_file(p1,p2);
return V8_PTR(r);
}
static v8_val do_new_from_file_at_size(v8_state vm, int argc, v8_val argv[]) {
char *p1 = V8_TOSTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
void *p4 = V8_TOPTR(argv[3]);
void *r = (void *) gdk_pixbuf_new_from_file_at_size(p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_new_from_file_at_scale(v8_state vm, int argc, v8_val argv[]) {
char *p1 = V8_TOSTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
void *p5 = V8_TOPTR(argv[4]);
void *r = (void *) gdk_pixbuf_new_from_file_at_scale(p1,p2,p3,p4,p5);
return V8_PTR(r);
}
static v8_val do_new_from_resource(v8_state vm, int argc, v8_val argv[]) {
char *p1 = V8_TOSTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_new_from_resource(p1,p2);
return V8_PTR(r);
}
static v8_val do_new_from_resource_at_scale(v8_state vm, int argc, v8_val argv[]) {
char *p1 = V8_TOSTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
void *p5 = V8_TOPTR(argv[4]);
void *r = (void *) gdk_pixbuf_new_from_resource_at_scale(p1,p2,p3,p4,p5);
return V8_PTR(r);
}
static v8_val do_new_from_data(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
int p5 = V8_TOINT32(argv[4]);
int p6 = V8_TOINT32(argv[5]);
int p7 = V8_TOINT32(argv[6]);
void *p8 = V8_TOPTR(argv[7]);
void *p9 = V8_TOPTR(argv[8]);
void *r = (void *) gdk_pixbuf_new_from_data(p1,p2,p3,p4,p5,p6,p7,p8,p9);
return V8_PTR(r);
}
static v8_val do_new_from_xpm_data(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_new_from_xpm_data(p1);
return V8_PTR(r);
}
static v8_val do_new_from_inline(v8_state vm, int argc, v8_val argv[]) {
int p1 = V8_TOINT32(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
int p3 = V8_TOINT32(argv[2]);
void *p4 = V8_TOPTR(argv[3]);
void *r = (void *) gdk_pixbuf_new_from_inline(p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_fill(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
unsigned int p2 = V8_TOUINT32(argv[1]);
gdk_pixbuf_fill(p1,p2);
return V8_VOID;
}
static v8_val do_savev(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
char *p2 = V8_TOSTR(argv[1]);
char *p3 = V8_TOSTR(argv[2]);
void *p4 = V8_TOPTR(argv[3]);
void *p5 = V8_TOPTR(argv[4]);
void *p6 = V8_TOPTR(argv[5]);
int r = (int) gdk_pixbuf_savev(p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_save_to_callbackv(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *p3 = V8_TOPTR(argv[2]);
char *p4 = V8_TOSTR(argv[3]);
void *p5 = V8_TOPTR(argv[4]);
void *p6 = V8_TOPTR(argv[5]);
void *p7 = V8_TOPTR(argv[6]);
int r = (int) gdk_pixbuf_save_to_callbackv(p1,p2,p3,p4,p5,p6,p7);
return V8_INT32(r);
}
static v8_val do_save_to_bufferv(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *p3 = V8_TOPTR(argv[2]);
char *p4 = V8_TOSTR(argv[3]);
void *p5 = V8_TOPTR(argv[4]);
void *p6 = V8_TOPTR(argv[5]);
void *p7 = V8_TOPTR(argv[6]);
int r = (int) gdk_pixbuf_save_to_bufferv(p1,p2,p3,p4,p5,p6,p7);
return V8_INT32(r);
}
static v8_val do_new_from_stream(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *p3 = V8_TOPTR(argv[2]);
void *r = (void *) gdk_pixbuf_new_from_stream(p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_new_from_stream_async(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *p3 = V8_TOPTR(argv[2]);
void *p4 = V8_TOPTR(argv[3]);
gdk_pixbuf_new_from_stream_async(p1,p2,p3,p4);
return V8_VOID;
}
static v8_val do_new_from_stream_finish(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_new_from_stream_finish(p1,p2);
return V8_PTR(r);
}
static v8_val do_new_from_stream_at_scale(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
void *p5 = V8_TOPTR(argv[4]);
void *p6 = V8_TOPTR(argv[5]);
void *r = (void *) gdk_pixbuf_new_from_stream_at_scale(p1,p2,p3,p4,p5,p6);
return V8_PTR(r);
}
static v8_val do_new_from_stream_at_scale_async(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
void *p5 = V8_TOPTR(argv[4]);
void *p6 = V8_TOPTR(argv[5]);
void *p7 = V8_TOPTR(argv[6]);
gdk_pixbuf_new_from_stream_at_scale_async(p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_save_to_stream_finish(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
int r = (int) gdk_pixbuf_save_to_stream_finish(p1,p2);
return V8_INT32(r);
}
static v8_val do_add_alpha(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
int p5 = V8_TOINT32(argv[4]);
void *r = (void *) gdk_pixbuf_add_alpha(p1,p2,p3,p4,p5);
return V8_PTR(r);
}
static v8_val do_copy_area(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
int p4 = V8_TOINT32(argv[3]);
int p5 = V8_TOINT32(argv[4]);
void *p6 = V8_TOPTR(argv[5]);
int p7 = V8_TOINT32(argv[6]);
int p8 = V8_TOINT32(argv[7]);
gdk_pixbuf_copy_area(p1,p2,p3,p4,p5,p6,p7,p8);
return V8_VOID;
}
static v8_val do_saturate_and_pixelate(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
double p3 = V8_TODOUBLE(argv[2]);
int p4 = V8_TOINT32(argv[3]);
gdk_pixbuf_saturate_and_pixelate(p1,p2,p3,p4);
return V8_VOID;
}
static v8_val do_apply_embedded_orientation(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_apply_embedded_orientation(p1);
return V8_PTR(r);
}
static v8_val do_get_option(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
char *p2 = V8_TOSTR(argv[1]);
void *r = (void *) gdk_pixbuf_get_option(p1,p2);
return V8_PTR(r);
}
static v8_ffn fntab_[] = {
{ 0, do_error_quark, "error_quark"},
{ 0, do_get_type, "get_type"},
{ 1, do_get_colorspace, "get_colorspace"},
{ 1, do_get_n_channels, "get_n_channels"},
{ 1, do_get_has_alpha, "get_has_alpha"},
{ 1, do_get_bits_per_sample, "get_bits_per_sample"},
{ 1, do_get_pixels, "get_pixels"},
{ 1, do_get_width, "get_width"},
{ 1, do_get_height, "get_height"},
{ 1, do_get_rowstride, "get_rowstride"},
{ 1, do_get_byte_length, "get_byte_length"},
{ 2, do_get_pixels_with_length, "get_pixels_with_length"},
{ 5, do_new, "new"},
{ 1, do_copy, "copy"},
{ 5, do_new_subpixbuf, "new_subpixbuf"},
{ 2, do_new_from_file, "new_from_file"},
{ 4, do_new_from_file_at_size, "new_from_file_at_size"},
{ 5, do_new_from_file_at_scale, "new_from_file_at_scale"},
{ 2, do_new_from_resource, "new_from_resource"},
{ 5, do_new_from_resource_at_scale, "new_from_resource_at_scale"},
{ 9, do_new_from_data, "new_from_data"},
{ 1, do_new_from_xpm_data, "new_from_xpm_data"},
{ 4, do_new_from_inline, "new_from_inline"},
{ 2, do_fill, "fill"},
{ 6, do_savev, "savev"},
{ 7, do_save_to_callbackv, "save_to_callbackv"},
{ 7, do_save_to_bufferv, "save_to_bufferv"},
{ 3, do_new_from_stream, "new_from_stream"},
{ 4, do_new_from_stream_async, "new_from_stream_async"},
{ 2, do_new_from_stream_finish, "new_from_stream_finish"},
{ 6, do_new_from_stream_at_scale, "new_from_stream_at_scale"},
{ 7, do_new_from_stream_at_scale_async, "new_from_stream_at_scale_async"},
{ 2, do_save_to_stream_finish, "save_to_stream_finish"},
{ 5, do_add_alpha, "add_alpha"},
{ 8, do_copy_area, "copy_area"},
{ 4, do_saturate_and_pixelate, "saturate_and_pixelate"},
{ 1, do_apply_embedded_orientation, "apply_embedded_orientation"},
{ 2, do_get_option, "get_option"},
{0}
};
static const char source_str_[] = "(function(){\
var _tags = {}, _types = {}, _s;\
this.GDK_PIXBUF_ALPHA_BILEVEL = 0;\
this.GDK_PIXBUF_ALPHA_FULL = 1;\
this.GDK_COLORSPACE_RGB = 0;\
this.GDK_PIXBUF_ERROR_CORRUPT_IMAGE = 0;\
this.GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY = 1;\
this.GDK_PIXBUF_ERROR_BAD_OPTION = 2;\
this.GDK_PIXBUF_ERROR_UNKNOWN_TYPE = 3;\
this.GDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION = 4;\
this.GDK_PIXBUF_ERROR_FAILED = 5;\
this['#funcs'] = [\
{name: 'error_quark', params: [\
{ type: 3, vname: ''},\
]},\
{name: 'get_type', params: [\
{ type: 6, vname: ''},\
]},\
{name: 'get_colorspace', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_n_channels', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_has_alpha', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_bits_per_sample', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_pixels', params: [\
{ type: 11, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_width', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_height', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_rowstride', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_byte_length', params: [\
{ type: 6, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_pixels_with_length', params: [\
{ type: 11, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
{ type: 11, vname: 'length'},\
]},\
{name: 'new', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 2, vname: 'colorspace'},\
{ type: 2, vname: 'has_alpha'},\
{ type: 2, vname: 'bits_per_sample'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
]},\
{name: 'copy', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
]},\
{name: 'new_subpixbuf', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'src_pixbuf', tagname: 'GdkPixbuf'},\
{ type: 2, vname: 'src_x'},\
{ type: 2, vname: 'src_y'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
]},\
{name: 'new_from_file', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 7, vname: 'filename'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_file_at_size', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 7, vname: 'filename'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_file_at_scale', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 7, vname: 'filename'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
{ type: 2, vname: 'preserve_aspect_ratio'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_resource', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 7, vname: 'resource_path'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_resource_at_scale', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 7, vname: 'resource_path'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
{ type: 2, vname: 'preserve_aspect_ratio'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_data', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 11, vname: 'data'},\
{ type: 2, vname: 'colorspace'},\
{ type: 2, vname: 'has_alpha'},\
{ type: 2, vname: 'bits_per_sample'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
{ type: 2, vname: 'rowstride'},\
{ type: 11, vname: 'destroy_fn'},\
{ type: 8, vname: 'destroy_fn_data'},\
]},\
{name: 'new_from_xpm_data', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 11, vname: 'data'},\
]},\
{name: 'new_from_inline', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 2, vname: 'data_length'},\
{ type: 11, vname: 'data'},\
{ type: 2, vname: 'copy_pixels'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'fill', params: [\
{ type: 1, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
{ type: 3, vname: 'pixel'},\
]},\
{name: 'savev', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
{ type: 7, vname: 'filename'},\
{ type: 7, vname: 'type'},\
{ type: 11, vname: 'option_keys'},\
{ type: 11, vname: 'option_values'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'save_to_callbackv', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
{ type: 11, vname: 'save_func'},\
{ type: 8, vname: 'user_data'},\
{ type: 7, vname: 'type'},\
{ type: 11, vname: 'option_keys'},\
{ type: 11, vname: 'option_values'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'save_to_bufferv', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
{ type: 11, vname: 'buffer'},\
{ type: 11, vname: 'buffer_size'},\
{ type: 7, vname: 'type'},\
{ type: 11, vname: 'option_keys'},\
{ type: 11, vname: 'option_values'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_stream', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'stream', tagname: 'GInputStream'},\
{ type: 10, vname: 'cancellable', tagname: 'GCancellable'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_stream_async', params: [\
{ type: 1, vname: ''},\
{ type: 10, vname: 'stream', tagname: 'GInputStream'},\
{ type: 10, vname: 'cancellable', tagname: 'GCancellable'},\
{ type: 11, vname: 'callback'},\
{ type: 8, vname: 'user_data'},\
]},\
{name: 'new_from_stream_finish', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'async_result', tagname: 'GAsyncResult'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_stream_at_scale', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'stream', tagname: 'GInputStream'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
{ type: 2, vname: 'preserve_aspect_ratio'},\
{ type: 10, vname: 'cancellable', tagname: 'GCancellable'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_from_stream_at_scale_async', params: [\
{ type: 1, vname: ''},\
{ type: 10, vname: 'stream', tagname: 'GInputStream'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
{ type: 2, vname: 'preserve_aspect_ratio'},\
{ type: 10, vname: 'cancellable', tagname: 'GCancellable'},\
{ type: 11, vname: 'callback'},\
{ type: 8, vname: 'user_data'},\
]},\
{name: 'save_to_stream_finish', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'async_result', tagname: 'GAsyncResult'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'add_alpha', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
{ type: 2, vname: 'substitute_color'},\
{ type: 2, vname: 'r'},\
{ type: 2, vname: 'g'},\
{ type: 2, vname: 'b'},\
]},\
{name: 'copy_area', params: [\
{ type: 1, vname: ''},\
{ type: 10, vname: 'src_pixbuf', tagname: 'GdkPixbuf'},\
{ type: 2, vname: 'src_x'},\
{ type: 2, vname: 'src_y'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
{ type: 10, vname: 'dest_pixbuf', tagname: 'GdkPixbuf'},\
{ type: 2, vname: 'dest_x'},\
{ type: 2, vname: 'dest_y'},\
]},\
{name: 'saturate_and_pixelate', params: [\
{ type: 1, vname: ''},\
{ type: 10, vname: 'src', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'dest', tagname: 'GdkPixbuf'},\
{ type: 6, vname: 'saturation'},\
{ type: 2, vname: 'pixelate'},\
]},\
{name: 'apply_embedded_orientation', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'src', tagname: 'GdkPixbuf'},\
]},\
{name: 'get_option', params: [\
{ type: 7, vname: ''},\
{ type: 10, vname: 'pixbuf', tagname: 'GdkPixbuf'},\
{ type: 7, vname: 'key'},\
]},\
];\
this.VOID = 1;\
this.INT = 2;\
this.UINT = 3;\
this.LONG = 4;\
this.ULONG = 5;\
this.DOUBLE = 6;\
this.STRING = 7;\
this.VOIDPTR = 8;\
this.RECORDPTR = 9;\
this.TYPEDEFPTR = 10;\
this.PTR = 11;\
this['#tags'] = _tags;this['#types'] = _types;});";

int JS_LOAD(v8_state vm, v8_val hobj) {
v8_val rc = jsv8->callstr(vm, source_str_, hobj, 0, NULL);
if (V8_ISERROR(rc)) return -1;
JS_EXPORT(fntab_);
}
