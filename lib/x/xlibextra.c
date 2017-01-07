#include "jsv8dlfn.h"
#include <xlibmacros.h>

static v8_val do_DefaultScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) DefaultScreen(p0);
return V8_INT32(r);
}
static v8_val do_DefaultVisual(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *r = (void *) DefaultVisual(p0,p1);
return V8_PTR(r);
}
static v8_val do_RootWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) RootWindow(p0,p1);
return V8_ULONG(r);
}
static v8_val do_BlackPixel(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) BlackPixel(p0,p1);
return V8_ULONG(r);
}
static v8_val do_WhitePixel(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) WhitePixel(p0,p1);
return V8_ULONG(r);
}
static v8_val do_ConnectionNumber(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) ConnectionNumber(p0);
return V8_INT32(r);
}
static v8_val do_DefaultColormap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) DefaultColormap(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XLookupString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XLookupString(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XCreateRegion(v8_state vm, int argc, v8_val argv[]) {
void *r = (void *) XCreateRegion();
return V8_PTR(r);
}
static v8_val do_XSetRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XSetRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XDestroyRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XDestroyRegion(p0);
return V8_INT32(r);
}
static v8_val do_XIntersectRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XIntersectRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XUnionRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XUnionRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XUnionRectWithRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XUnionRectWithRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSubtractRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XSubtractRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XXorRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XXorRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XOffsetRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XOffsetRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XShrinkRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XShrinkRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XEmptyRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XEmptyRegion(p0);
return V8_INT32(r);
}
static v8_val do_XEqualRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XEqualRegion(p0,p1);
return V8_INT32(r);
}
static v8_val do_XPointInRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XPointInRegion(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XRectInRegion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
int r = (int) XRectInRegion(p0,p1,p2,p3,p4);
return V8_INT32(r);
}

static v8_ffn fntab_[] = {
{ 1, do_DefaultScreen, "DefaultScreen"},
{ 2, do_DefaultVisual, "DefaultVisual"},
{ 2, do_RootWindow, "RootWindow"},
{ 2, do_BlackPixel, "BlackPixel"},
{ 2, do_WhitePixel, "WhitePixel"},
{ 1, do_ConnectionNumber, "ConnectionNumber"},
{ 2, do_DefaultColormap, "DefaultColormap"},
{ 5, do_XLookupString, "XLookupString"},
{ 0, do_XCreateRegion, "XCreateRegion"},
{ 3, do_XSetRegion, "XSetRegion"},
{ 1, do_XDestroyRegion, "XDestroyRegion"},
{ 3, do_XIntersectRegion, "XIntersectRegion"},
{ 3, do_XUnionRegion, "XUnionRegion"},
{ 3, do_XUnionRectWithRegion, "XUnionRectWithRegion"},
{ 3, do_XSubtractRegion, "XSubtractRegion"},
{ 3, do_XXorRegion, "XXorRegion"},
{ 3, do_XOffsetRegion, "XOffsetRegion"},
{ 3, do_XShrinkRegion, "XShrinkRegion"},
{ 1, do_XEmptyRegion, "XEmptyRegion"},
{ 2, do_XEqualRegion, "XEqualRegion"},
{ 3, do_XPointInRegion, "XPointInRegion"},
{ 5, do_XRectInRegion, "XRectInRegion"},
{0}
};
static const char source_str_[] = "(function(){\
var _tags = {}, _types = {}, _s;\
_s = { x : 6815744, y : 6815746, width : 4718596, height : 4718598 }; _s['#size'] = 8;\
_types['XRectangle'] = _s;\
this['#tags'] = _tags;this['#types'] = _types;});";

int JS_LOAD(v8_state vm, v8_val hobj) {
v8_val rc = jsv8->callstr(vm, source_str_, hobj, 0, NULL);
if (V8_ISERROR(rc)) return -1;
JS_EXPORT(fntab_);
}
