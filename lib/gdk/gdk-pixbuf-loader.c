#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include "jsv8dlfn.h"

static v8_val do_get_type(v8_state vm, int argc, v8_val argv[]) {
double r = (double) gdk_pixbuf_loader_get_type();
return V8_DOUBLE(r);
}
static v8_val do_new(v8_state vm, int argc, v8_val argv[]) {
void *r = (void *) gdk_pixbuf_loader_new();
return V8_PTR(r);
}
static v8_val do_new_with_type(v8_state vm, int argc, v8_val argv[]) {
char *p1 = V8_TOSTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_loader_new_with_type(p1,p2);
return V8_PTR(r);
}
static v8_val do_new_with_mime_type(v8_state vm, int argc, v8_val argv[]) {
char *p1 = V8_TOSTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *r = (void *) gdk_pixbuf_loader_new_with_mime_type(p1,p2);
return V8_PTR(r);
}
static v8_val do_set_size(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
int p2 = V8_TOINT32(argv[1]);
int p3 = V8_TOINT32(argv[2]);
gdk_pixbuf_loader_set_size(p1,p2,p3);
return V8_VOID;
}
static v8_val do_write(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
double p3 = V8_TODOUBLE(argv[2]);
void *p4 = V8_TOPTR(argv[3]);
int r = (int) gdk_pixbuf_loader_write(p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_write_bytes(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
void *p3 = V8_TOPTR(argv[2]);
int r = (int) gdk_pixbuf_loader_write_bytes(p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_get_pixbuf(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_loader_get_pixbuf(p1);
return V8_PTR(r);
}
static v8_val do_get_animation(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_loader_get_animation(p1);
return V8_PTR(r);
}
static v8_val do_close(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *p2 = V8_TOPTR(argv[1]);
int r = (int) gdk_pixbuf_loader_close(p1,p2);
return V8_INT32(r);
}
static v8_val do_get_format(v8_state vm, int argc, v8_val argv[]) {
void *p1 = V8_TOPTR(argv[0]);
void *r = (void *) gdk_pixbuf_loader_get_format(p1);
return V8_PTR(r);
}
static v8_ffn fntab_[] = {
{ 0, do_get_type, "get_type"},
{ 0, do_new, "new"},
{ 2, do_new_with_type, "new_with_type"},
{ 2, do_new_with_mime_type, "new_with_mime_type"},
{ 3, do_set_size, "set_size"},
{ 4, do_write, "write"},
{ 3, do_write_bytes, "write_bytes"},
{ 1, do_get_pixbuf, "get_pixbuf"},
{ 1, do_get_animation, "get_animation"},
{ 2, do_close, "close"},
{ 1, do_get_format, "get_format"},
{0}
};
static const char source_str_[] = "(function(){\
var _tags = {}, _types = {}, _s;\
_s = { parent_instance : 0, priv : 7340056 }; _s['#size'] = 32;\
_tags['_GdkPixbufLoader'] = _s;\
_types['GdkPixbufLoader'] = _s;\
_s = { parent_class : 0, size_prepared : 7340168, area_prepared : 7340176, area_updated : 7340184, closed : 7340192 }; _s['#size'] = 168;\
_tags['_GdkPixbufLoaderClass'] = _s;\
_types['GdkPixbufLoaderClass'] = _s;\
this['#funcs'] = [\
{name: 'get_type', params: [\
{ type: 6, vname: ''},\
]},\
{name: 'new', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbufLoader'},\
]},\
{name: 'new_with_type', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbufLoader'},\
{ type: 7, vname: 'image_type'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'new_with_mime_type', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbufLoader'},\
{ type: 7, vname: 'mime_type'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'set_size', params: [\
{ type: 1, vname: ''},\
{ type: 10, vname: 'loader', tagname: 'GdkPixbufLoader'},\
{ type: 2, vname: 'width'},\
{ type: 2, vname: 'height'},\
]},\
{name: 'write', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'loader', tagname: 'GdkPixbufLoader'},\
{ type: 11, vname: 'buf'},\
{ type: 6, vname: 'count'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'write_bytes', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'loader', tagname: 'GdkPixbufLoader'},\
{ type: 10, vname: 'buffer', tagname: 'GBytes'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'get_pixbuf', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbuf'},\
{ type: 10, vname: 'loader', tagname: 'GdkPixbufLoader'},\
]},\
{name: 'get_animation', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbufAnimation'},\
{ type: 10, vname: 'loader', tagname: 'GdkPixbufLoader'},\
]},\
{name: 'close', params: [\
{ type: 2, vname: ''},\
{ type: 10, vname: 'loader', tagname: 'GdkPixbufLoader'},\
{ type: 11, vname: 'error'},\
]},\
{name: 'get_format', params: [\
{ type: 10, vname: '', tagname: 'GdkPixbufFormat'},\
{ type: 10, vname: 'loader', tagname: 'GdkPixbufLoader'},\
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
