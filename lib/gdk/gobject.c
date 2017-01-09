#include "jsv8dlfn.h"
#include <gobject-light.h>

static v8_val do_ref(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) g_object_ref(p0);
return V8_PTR(r);
}
static v8_val do_unref(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
g_object_unref(p0);
return V8_VOID;
}
static v8_val do_ref_sink(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) g_object_ref_sink(p0);
return V8_PTR(r);
}
static v8_val do_is_floating(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) g_object_is_floating(p0);
return V8_INT32(r);
}

static v8_ffn fntab_[] = {
{ 1, do_ref, "ref"},
{ 1, do_unref, "unref"},
{ 1, do_ref_sink, "ref_sink"},
{ 1, do_is_floating, "is_floating"},
{0}
};
static const char source_str_[] = "(function(){\
var _tags = {}, _types = {}, _s;\
this['#tags'] = _tags;this['#types'] = _types;});";

int JS_LOAD(v8_state vm, v8_val hobj) {
v8_val rc = jsv8->callstr(vm, source_str_, hobj, 0, NULL);
if (V8_ISERROR(rc)) return -1;
JS_EXPORT(fntab_);
}
