#include "jsv8dlfn.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

static v8_val do_error_quark(v8_state vm, int argc, v8_val argv[]) {
unsigned int r = (unsigned int) gdk_pixbuf_error_quark();
return V8_UINT32(r);
}
static v8_val do_get_type(v8_state vm, int argc, v8_val argv[]) {
double r = (double) gdk_pixbuf_get_type();
return V8_DOUBLE(r);
}
static v8_val do_get_colorspace(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_colorspace(p0);
return V8_INT32(r);
}
static v8_val do_get_n_channels(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_n_channels(p0);
return V8_INT32(r);
}
static v8_val do_get_has_alpha(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_has_alpha(p0);
return V8_INT32(r);
}
static v8_val do_get_bits_per_sample(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_bits_per_sample(p0);
return V8_INT32(r);
}
static v8_val do_get_pixels(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_get_pixels(p0);
return V8_PTR(r);
}
static v8_val do_get_width(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_width(p0);
return V8_INT32(r);
}
static v8_val do_get_height(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_height(p0);
return V8_INT32(r);
}
static v8_val do_get_rowstride(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) gdk_pixbuf_get_rowstride(p0);
return V8_INT32(r);
}
static v8_val do_get_byte_length(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
double r = (double) gdk_pixbuf_get_byte_length(p0);
return V8_DOUBLE(r);
}
static v8_val do_get_pixels_with_length(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_get_pixels_with_length(p0,p1);
return V8_PTR(r);
}
static v8_val do_new(v8_state vm, int argc, v8_val argv[]) {
int p0 = V8_TOINT32(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *r = (void *) gdk_pixbuf_new(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_copy(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_copy(p0);
return V8_PTR(r);
}
static v8_val do_new_subpixbuf(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *r = (void *) gdk_pixbuf_new_subpixbuf(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_new_from_file(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_new_from_file(p0,p1);
return V8_PTR(r);
}
static v8_val do_new_from_file_at_size(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *r = (void *) gdk_pixbuf_new_from_file_at_size(p0,p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_new_from_file_at_scale(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *r = (void *) gdk_pixbuf_new_from_file_at_scale(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_new_from_resource(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_new_from_resource(p0,p1);
return V8_PTR(r);
}
static v8_val do_new_from_resource_at_scale(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *r = (void *) gdk_pixbuf_new_from_resource_at_scale(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_new_from_data(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
void *r = (void *) gdk_pixbuf_new_from_data(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_PTR(r);
}
static v8_val do_new_from_xpm_data(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_new_from_xpm_data(p0);
return V8_PTR(r);
}
static v8_val do_new_from_inline(v8_state vm, int argc, v8_val argv[]) {
int p0 = V8_TOINT32(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *r = (void *) gdk_pixbuf_new_from_inline(p0,p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_fill(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
unsigned int p1 = V8_TOUINT32(argv[1]);
gdk_pixbuf_fill(p0,p1);
return V8_VOID;
}
static v8_val do_savev(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) gdk_pixbuf_savev(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_save_to_callbackv(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int r = (int) gdk_pixbuf_save_to_callbackv(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_save_to_bufferv(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int r = (int) gdk_pixbuf_save_to_bufferv(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_new_from_stream(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *r = (void *) gdk_pixbuf_new_from_stream(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_new_from_stream_async(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
gdk_pixbuf_new_from_stream_async(p0,p1,p2,p3);
return V8_VOID;
}
static v8_val do_new_from_stream_finish(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_new_from_stream_finish(p0,p1);
return V8_PTR(r);
}
static v8_val do_new_from_stream_at_scale(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *r = (void *) gdk_pixbuf_new_from_stream_at_scale(p0,p1,p2,p3,p4,p5);
return V8_PTR(r);
}
static v8_val do_new_from_stream_at_scale_async(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
gdk_pixbuf_new_from_stream_at_scale_async(p0,p1,p2,p3,p4,p5,p6);
return V8_VOID;
}
static v8_val do_save_to_stream_finish(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) gdk_pixbuf_save_to_stream_finish(p0,p1);
return V8_INT32(r);
}
static v8_val do_add_alpha(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *r = (void *) gdk_pixbuf_add_alpha(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_copy_area(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
gdk_pixbuf_copy_area(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_saturate_and_pixelate(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
double p2 = V8_TODOUBLE(argv[2]);
int p3 = V8_TOINT32(argv[3]);
gdk_pixbuf_saturate_and_pixelate(p0,p1,p2,p3);
return V8_VOID;
}
static v8_val do_apply_embedded_orientation(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_apply_embedded_orientation(p0);
return V8_PTR(r);
}
static v8_val do_get_option(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_get_option(p0,p1);
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
this['#tags'] = _tags;this['#types'] = _types;});";

int JS_LOAD(v8_state vm, v8_val hobj) {
v8_val rc = jsv8->callstr(vm, source_str_, hobj, 0, NULL);
if (V8_ISERROR(rc)) return -1;
JS_EXPORT(fntab_);
}
