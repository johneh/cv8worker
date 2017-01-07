#include "jsv8dlfn.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // XLookupString

static v8_val do__Xmblen(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) _Xmblen(p0,p1);
return V8_INT32(r);
}
static v8_val do_XLoadQueryFont(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
void *r = (void *) XLoadQueryFont(p0,p1);
return V8_PTR(r);
}
static v8_val do_XQueryFont(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *r = (void *) XQueryFont(p0,p1);
return V8_PTR(r);
}
static v8_val do_XGetMotionEvents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *r = (void *) XGetMotionEvents(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_XDeleteModifiermapEntry(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *r = (void *) XDeleteModifiermapEntry(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XGetModifierMapping(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XGetModifierMapping(p0);
return V8_PTR(r);
}
static v8_val do_XInsertModifiermapEntry(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *r = (void *) XInsertModifiermapEntry(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XNewModifiermap(v8_state vm, int argc, v8_val argv[]) {
int p0 = V8_TOINT32(argv[0]);
void *r = (void *) XNewModifiermap(p0);
return V8_PTR(r);
}
static v8_val do_XCreateImage(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
char *p5 = V8_TOSTR(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
unsigned int p7 = V8_TOUINT32(argv[7]);
int p8 = V8_TOINT32(argv[8]);
int p9 = V8_TOINT32(argv[9]);
void *r = (void *) XCreateImage(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9);
return V8_PTR(r);
}
static v8_val do_XInitImage(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XInitImage(p0);
return V8_INT32(r);
}
static v8_val do_XGetImage(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
uint64_t p6 = V8_TOULONG(argv[6]);
int p7 = V8_TOINT32(argv[7]);
void *r = (void *) XGetImage(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_PTR(r);
}
static v8_val do_XGetSubImage(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
uint64_t p6 = V8_TOULONG(argv[6]);
int p7 = V8_TOINT32(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
int p9 = V8_TOINT32(argv[9]);
int p10 = V8_TOINT32(argv[10]);
void *r = (void *) XGetSubImage(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
return V8_PTR(r);
}
static v8_val do_XOpenDisplay(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
void *r = (void *) XOpenDisplay(p0);
return V8_PTR(r);
}
static v8_val do_XrmInitialize(v8_state vm, int argc, v8_val argv[]) {
XrmInitialize();
return V8_VOID;
}
static v8_val do_XFetchBytes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) XFetchBytes(p0,p1);
return V8_PTR(r);
}
static v8_val do_XFetchBuffer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *r = (void *) XFetchBuffer(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XGetAtomName(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *r = (void *) XGetAtomName(p0,p1);
return V8_PTR(r);
}
static v8_val do_XGetAtomNames(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XGetAtomNames(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XGetDefault(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
void *r = (void *) XGetDefault(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XDisplayName(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
void *r = (void *) XDisplayName(p0);
return V8_PTR(r);
}
static v8_val do_XKeysymToString(v8_state vm, int argc, v8_val argv[]) {
uint64_t p0 = V8_TOULONG(argv[0]);
void *r = (void *) XKeysymToString(p0);
return V8_PTR(r);
}
static v8_val do_XSynchronize(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *r = (void *) XSynchronize(p0,p1);
return V8_PTR(r);
}
static v8_val do_XSetAfterFunction(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) XSetAfterFunction(p0,p1);
return V8_PTR(r);
}
static v8_val do_XInternAtom(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
uint64_t r = (uint64_t) XInternAtom(p0,p1,p2);
return V8_ULONG(r);
}
static v8_val do_XInternAtoms(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XInternAtoms(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XCopyColormapAndFree(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t r = (uint64_t) XCopyColormapAndFree(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XCreateColormap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
uint64_t r = (uint64_t) XCreateColormap(p0,p1,p2,p3);
return V8_ULONG(r);
}
static v8_val do_XCreatePixmapCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
uint64_t r = (uint64_t) XCreatePixmapCursor(p0,p1,p2,p3,p4,p5,p6);
return V8_ULONG(r);
}
static v8_val do_XCreateGlyphCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
uint64_t r = (uint64_t) XCreateGlyphCursor(p0,p1,p2,p3,p4,p5,p6);
return V8_ULONG(r);
}
static v8_val do_XCreateFontCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
unsigned int p1 = V8_TOUINT32(argv[1]);
uint64_t r = (uint64_t) XCreateFontCursor(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XLoadFont(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
uint64_t r = (uint64_t) XLoadFont(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XCreateGC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *r = (void *) XCreateGC(p0,p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_XGContextFromGC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XGContextFromGC(p0);
return V8_ULONG(r);
}
static v8_val do_XFlushGC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
XFlushGC(p0,p1);
return V8_VOID;
}
static v8_val do_XCreatePixmap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
uint64_t r = (uint64_t) XCreatePixmap(p0,p1,p2,p3,p4);
return V8_ULONG(r);
}
static v8_val do_XCreateBitmapFromData(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
uint64_t r = (uint64_t) XCreateBitmapFromData(p0,p1,p2,p3,p4);
return V8_ULONG(r);
}
static v8_val do_XCreatePixmapFromBitmapData(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
uint64_t p5 = V8_TOULONG(argv[5]);
uint64_t p6 = V8_TOULONG(argv[6]);
unsigned int p7 = V8_TOUINT32(argv[7]);
uint64_t r = (uint64_t) XCreatePixmapFromBitmapData(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_ULONG(r);
}
static v8_val do_XCreateSimpleWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
uint64_t p7 = V8_TOULONG(argv[7]);
uint64_t p8 = V8_TOULONG(argv[8]);
uint64_t r = (uint64_t) XCreateSimpleWindow(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_ULONG(r);
}
static v8_val do_XGetSelectionOwner(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t r = (uint64_t) XGetSelectionOwner(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XCreateWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
unsigned int p8 = V8_TOUINT32(argv[8]);
void *p9 = V8_TOPTR(argv[9]);
uint64_t p10 = V8_TOULONG(argv[10]);
void *p11 = V8_TOPTR(argv[11]);
uint64_t r = (uint64_t) XCreateWindow(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11);
return V8_ULONG(r);
}
static v8_val do_XListInstalledColormaps(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *r = (void *) XListInstalledColormaps(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XListFonts(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *r = (void *) XListFonts(p0,p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_XListFontsWithInfo(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *r = (void *) XListFontsWithInfo(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_XGetFontPath(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) XGetFontPath(p0,p1);
return V8_PTR(r);
}
static v8_val do_XListExtensions(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) XListExtensions(p0,p1);
return V8_PTR(r);
}
static v8_val do_XListProperties(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *r = (void *) XListProperties(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XListHosts(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *r = (void *) XListHosts(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XLookupKeysym(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) XLookupKeysym(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XGetKeyboardMapping(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *r = (void *) XGetKeyboardMapping(p0,p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_XStringToKeysym(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
uint64_t r = (uint64_t) XStringToKeysym(p0);
return V8_ULONG(r);
}
static v8_val do_XMaxRequestSize(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int64_t r = (int64_t) XMaxRequestSize(p0);
return V8_LONG(r);
}
static v8_val do_XExtendedMaxRequestSize(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int64_t r = (int64_t) XExtendedMaxRequestSize(p0);
return V8_LONG(r);
}
static v8_val do_XResourceManagerString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XResourceManagerString(p0);
return V8_PTR(r);
}
static v8_val do_XScreenResourceString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XScreenResourceString(p0);
return V8_PTR(r);
}
static v8_val do_XDisplayMotionBufferSize(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XDisplayMotionBufferSize(p0);
return V8_ULONG(r);
}
static v8_val do_XVisualIDFromVisual(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XVisualIDFromVisual(p0);
return V8_ULONG(r);
}
static v8_val do_XInitThreads(v8_state vm, int argc, v8_val argv[]) {
int r = (int) XInitThreads();
return V8_INT32(r);
}
static v8_val do_XLockDisplay(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
XLockDisplay(p0);
return V8_VOID;
}
static v8_val do_XUnlockDisplay(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
XUnlockDisplay(p0);
return V8_VOID;
}
static v8_val do_XInitExtension(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
void *r = (void *) XInitExtension(p0,p1);
return V8_PTR(r);
}
static v8_val do_XAddExtension(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XAddExtension(p0);
return V8_PTR(r);
}
static v8_val do_XFindOnExtensionList(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *r = (void *) XFindOnExtensionList(p0,p1);
return V8_PTR(r);
}
static v8_val do_XRootWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) XRootWindow(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XDefaultRootWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XDefaultRootWindow(p0);
return V8_ULONG(r);
}
static v8_val do_XRootWindowOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XRootWindowOfScreen(p0);
return V8_ULONG(r);
}
static v8_val do_XDefaultVisual(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *r = (void *) XDefaultVisual(p0,p1);
return V8_PTR(r);
}
static v8_val do_XDefaultVisualOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XDefaultVisualOfScreen(p0);
return V8_PTR(r);
}
static v8_val do_XDefaultGC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *r = (void *) XDefaultGC(p0,p1);
return V8_PTR(r);
}
static v8_val do_XDefaultGCOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XDefaultGCOfScreen(p0);
return V8_PTR(r);
}
static v8_val do_XBlackPixel(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) XBlackPixel(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XWhitePixel(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) XWhitePixel(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XAllPlanes(v8_state vm, int argc, v8_val argv[]) {
uint64_t r = (uint64_t) XAllPlanes();
return V8_ULONG(r);
}
static v8_val do_XBlackPixelOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XBlackPixelOfScreen(p0);
return V8_ULONG(r);
}
static v8_val do_XWhitePixelOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XWhitePixelOfScreen(p0);
return V8_ULONG(r);
}
static v8_val do_XNextRequest(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XNextRequest(p0);
return V8_ULONG(r);
}
static v8_val do_XLastKnownRequestProcessed(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XLastKnownRequestProcessed(p0);
return V8_ULONG(r);
}
static v8_val do_XServerVendor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XServerVendor(p0);
return V8_PTR(r);
}
static v8_val do_XDisplayString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XDisplayString(p0);
return V8_PTR(r);
}
static v8_val do_XDefaultColormap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t r = (uint64_t) XDefaultColormap(p0,p1);
return V8_ULONG(r);
}
static v8_val do_XDefaultColormapOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t r = (uint64_t) XDefaultColormapOfScreen(p0);
return V8_ULONG(r);
}
static v8_val do_XDisplayOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XDisplayOfScreen(p0);
return V8_PTR(r);
}
static v8_val do_XScreenOfDisplay(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *r = (void *) XScreenOfDisplay(p0,p1);
return V8_PTR(r);
}
static v8_val do_XDefaultScreenOfDisplay(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XDefaultScreenOfDisplay(p0);
return V8_PTR(r);
}
static v8_val do_XEventMaskOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int64_t r = (int64_t) XEventMaskOfScreen(p0);
return V8_LONG(r);
}
static v8_val do_XScreenNumberOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XScreenNumberOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XSetErrorHandler(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XSetErrorHandler(p0);
return V8_PTR(r);
}
static v8_val do_XSetIOErrorHandler(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XSetIOErrorHandler(p0);
return V8_PTR(r);
}
static v8_val do_XListPixmapFormats(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *r = (void *) XListPixmapFormats(p0,p1);
return V8_PTR(r);
}
static v8_val do_XListDepths(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *r = (void *) XListDepths(p0,p1,p2);
return V8_PTR(r);
}
static v8_val do_XReconfigureWMWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XReconfigureWMWindow(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XGetWMProtocols(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XGetWMProtocols(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XSetWMProtocols(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XSetWMProtocols(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XIconifyWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XIconifyWindow(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XWithdrawWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XWithdrawWindow(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XGetCommand(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XGetCommand(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XGetWMColormapWindows(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XGetWMColormapWindows(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XSetWMColormapWindows(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XSetWMColormapWindows(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XFreeStringList(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
XFreeStringList(p0);
return V8_VOID;
}
static v8_val do_XSetTransientForHint(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetTransientForHint(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XActivateScreenSaver(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XActivateScreenSaver(p0);
return V8_INT32(r);
}
static v8_val do_XAddHost(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XAddHost(p0,p1);
return V8_INT32(r);
}
static v8_val do_XAddHosts(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XAddHosts(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XAddToExtensionList(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XAddToExtensionList(p0,p1);
return V8_INT32(r);
}
static v8_val do_XAddToSaveSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XAddToSaveSet(p0,p1);
return V8_INT32(r);
}
static v8_val do_XAllocColor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XAllocColor(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XAllocColorCells(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int r = (int) XAllocColorCells(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XAllocColorPlanes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
void *p9 = V8_TOPTR(argv[9]);
void *p10 = V8_TOPTR(argv[10]);
int r = (int) XAllocColorPlanes(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
return V8_INT32(r);
}
static v8_val do_XAllocNamedColor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XAllocNamedColor(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XAllowEvents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XAllowEvents(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XAutoRepeatOff(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XAutoRepeatOff(p0);
return V8_INT32(r);
}
static v8_val do_XAutoRepeatOn(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XAutoRepeatOn(p0);
return V8_INT32(r);
}
static v8_val do_XBell(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XBell(p0,p1);
return V8_INT32(r);
}
static v8_val do_XBitmapBitOrder(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XBitmapBitOrder(p0);
return V8_INT32(r);
}
static v8_val do_XBitmapPad(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XBitmapPad(p0);
return V8_INT32(r);
}
static v8_val do_XBitmapUnit(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XBitmapUnit(p0);
return V8_INT32(r);
}
static v8_val do_XCellsOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XCellsOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XChangeActivePointerGrab(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
unsigned int p1 = V8_TOUINT32(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int r = (int) XChangeActivePointerGrab(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XChangeGC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XChangeGC(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XChangeKeyboardControl(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XChangeKeyboardControl(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XChangeKeyboardMapping(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XChangeKeyboardMapping(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XChangePointerControl(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int r = (int) XChangePointerControl(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XChangeProperty(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int p7 = V8_TOINT32(argv[7]);
int r = (int) XChangeProperty(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_INT32(r);
}
static v8_val do_XChangeSaveSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XChangeSaveSet(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XChangeWindowAttributes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XChangeWindowAttributes(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XCheckIfEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
int r = (int) XCheckIfEvent(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XCheckMaskEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int64_t p1 = V8_TOLONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XCheckMaskEvent(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XCheckTypedEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XCheckTypedEvent(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XCheckTypedWindowEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XCheckTypedWindowEvent(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XCheckWindowEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int64_t p2 = V8_TOLONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XCheckWindowEvent(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XCirculateSubwindows(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XCirculateSubwindows(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XCirculateSubwindowsDown(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XCirculateSubwindowsDown(p0,p1);
return V8_INT32(r);
}
static v8_val do_XCirculateSubwindowsUp(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XCirculateSubwindowsUp(p0,p1);
return V8_INT32(r);
}
static v8_val do_XClearArea(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XClearArea(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XClearWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XClearWindow(p0,p1);
return V8_INT32(r);
}
static v8_val do_XCloseDisplay(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XCloseDisplay(p0);
return V8_INT32(r);
}
static v8_val do_XConfigureWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XConfigureWindow(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XConnectionNumber(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XConnectionNumber(p0);
return V8_INT32(r);
}
static v8_val do_XConvertSelection(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
uint64_t p4 = V8_TOULONG(argv[4]);
uint64_t p5 = V8_TOULONG(argv[5]);
int r = (int) XConvertSelection(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XCopyArea(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
unsigned int p7 = V8_TOUINT32(argv[7]);
int p8 = V8_TOINT32(argv[8]);
int p9 = V8_TOINT32(argv[9]);
int r = (int) XCopyArea(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9);
return V8_INT32(r);
}
static v8_val do_XCopyGC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XCopyGC(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XCopyPlane(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
unsigned int p7 = V8_TOUINT32(argv[7]);
int p8 = V8_TOINT32(argv[8]);
int p9 = V8_TOINT32(argv[9]);
uint64_t p10 = V8_TOULONG(argv[10]);
int r = (int) XCopyPlane(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
return V8_INT32(r);
}
static v8_val do_XDefaultDepth(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XDefaultDepth(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDefaultDepthOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XDefaultDepthOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XDefaultScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XDefaultScreen(p0);
return V8_INT32(r);
}
static v8_val do_XDefineCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XDefineCursor(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XDeleteProperty(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XDeleteProperty(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XDestroyWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XDestroyWindow(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDestroySubwindows(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XDestroySubwindows(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDoesBackingStore(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XDoesBackingStore(p0);
return V8_INT32(r);
}
static v8_val do_XDoesSaveUnders(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XDoesSaveUnders(p0);
return V8_INT32(r);
}
static v8_val do_XDisableAccessControl(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XDisableAccessControl(p0);
return V8_INT32(r);
}
static v8_val do_XDisplayCells(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XDisplayCells(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDisplayHeight(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XDisplayHeight(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDisplayHeightMM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XDisplayHeightMM(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDisplayKeycodes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XDisplayKeycodes(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XDisplayPlanes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XDisplayPlanes(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDisplayWidth(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XDisplayWidth(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDisplayWidthMM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XDisplayWidthMM(p0,p1);
return V8_INT32(r);
}
static v8_val do_XDrawArc(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
int p8 = V8_TOINT32(argv[8]);
int r = (int) XDrawArc(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XDrawArcs(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XDrawArcs(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XDrawImageString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
char *p5 = V8_TOSTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XDrawImageString(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XDrawImageString16(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XDrawImageString16(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XDrawLine(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XDrawLine(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XDrawLines(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int r = (int) XDrawLines(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XDrawPoint(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XDrawPoint(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XDrawPoints(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int r = (int) XDrawPoints(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XDrawRectangle(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int r = (int) XDrawRectangle(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XDrawRectangles(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XDrawRectangles(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XDrawSegments(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XDrawSegments(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XDrawString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
char *p5 = V8_TOSTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XDrawString(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XDrawString16(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XDrawString16(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XDrawText(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XDrawText(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XDrawText16(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XDrawText16(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XEnableAccessControl(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XEnableAccessControl(p0);
return V8_INT32(r);
}
static v8_val do_XEventsQueued(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XEventsQueued(p0,p1);
return V8_INT32(r);
}
static v8_val do_XFetchName(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XFetchName(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XFillArc(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
int p8 = V8_TOINT32(argv[8]);
int r = (int) XFillArc(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XFillArcs(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XFillArcs(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XFillPolygon(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XFillPolygon(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XFillRectangle(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int r = (int) XFillRectangle(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XFillRectangles(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XFillRectangles(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XFlush(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XFlush(p0);
return V8_INT32(r);
}
static v8_val do_XForceScreenSaver(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XForceScreenSaver(p0,p1);
return V8_INT32(r);
}
static v8_val do_XFree(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XFree(p0);
return V8_INT32(r);
}
static v8_val do_XFreeColormap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XFreeColormap(p0,p1);
return V8_INT32(r);
}
static v8_val do_XFreeColors(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
uint64_t p4 = V8_TOULONG(argv[4]);
int r = (int) XFreeColors(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XFreeCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XFreeCursor(p0,p1);
return V8_INT32(r);
}
static v8_val do_XFreeExtensionList(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XFreeExtensionList(p0);
return V8_INT32(r);
}
static v8_val do_XFreeFont(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XFreeFont(p0,p1);
return V8_INT32(r);
}
static v8_val do_XFreeFontInfo(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XFreeFontInfo(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XFreeFontNames(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XFreeFontNames(p0);
return V8_INT32(r);
}
static v8_val do_XFreeFontPath(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XFreeFontPath(p0);
return V8_INT32(r);
}
static v8_val do_XFreeGC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XFreeGC(p0,p1);
return V8_INT32(r);
}
static v8_val do_XFreeModifiermap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XFreeModifiermap(p0);
return V8_INT32(r);
}
static v8_val do_XFreePixmap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XFreePixmap(p0,p1);
return V8_INT32(r);
}
static v8_val do_XGeometry(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
int p8 = V8_TOINT32(argv[8]);
void *p9 = V8_TOPTR(argv[9]);
void *p10 = V8_TOPTR(argv[10]);
void *p11 = V8_TOPTR(argv[11]);
void *p12 = V8_TOPTR(argv[12]);
int r = (int) XGeometry(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12);
return V8_INT32(r);
}
static v8_val do_XGetErrorDatabaseText(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
char *p4 = V8_TOSTR(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int r = (int) XGetErrorDatabaseText(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XGetErrorText(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XGetErrorText(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XGetFontProperty(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XGetFontProperty(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XGetGCValues(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XGetGCValues(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XGetGeometry(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
int r = (int) XGetGeometry(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XGetIconName(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XGetIconName(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XGetInputFocus(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XGetInputFocus(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XGetKeyboardControl(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XGetKeyboardControl(p0,p1);
return V8_INT32(r);
}
static v8_val do_XGetPointerControl(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XGetPointerControl(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XGetPointerMapping(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XGetPointerMapping(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XGetScreenSaver(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XGetScreenSaver(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XGetTransientForHint(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XGetTransientForHint(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XGetWindowProperty(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int64_t p3 = V8_TOLONG(argv[3]);
int64_t p4 = V8_TOLONG(argv[4]);
int p5 = V8_TOINT32(argv[5]);
uint64_t p6 = V8_TOULONG(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
void *p9 = V8_TOPTR(argv[9]);
void *p10 = V8_TOPTR(argv[10]);
void *p11 = V8_TOPTR(argv[11]);
int r = (int) XGetWindowProperty(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11);
return V8_INT32(r);
}
static v8_val do_XGetWindowAttributes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XGetWindowAttributes(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XGrabButton(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
unsigned int p1 = V8_TOUINT32(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int p4 = V8_TOINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
uint64_t p8 = V8_TOULONG(argv[8]);
uint64_t p9 = V8_TOULONG(argv[9]);
int r = (int) XGrabButton(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9);
return V8_INT32(r);
}
static v8_val do_XGrabKey(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XGrabKey(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XGrabKeyboard(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
uint64_t p5 = V8_TOULONG(argv[5]);
int r = (int) XGrabKeyboard(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XGrabPointer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
uint64_t p6 = V8_TOULONG(argv[6]);
uint64_t p7 = V8_TOULONG(argv[7]);
uint64_t p8 = V8_TOULONG(argv[8]);
int r = (int) XGrabPointer(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XGrabServer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XGrabServer(p0);
return V8_INT32(r);
}
static v8_val do_XHeightMMOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XHeightMMOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XHeightOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XHeightOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XIfEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
int r = (int) XIfEvent(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XImageByteOrder(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XImageByteOrder(p0);
return V8_INT32(r);
}
static v8_val do_XInstallColormap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XInstallColormap(p0,p1);
return V8_INT32(r);
}
static v8_val do_XKeysymToKeycode(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XKeysymToKeycode(p0,p1);
return V8_INT32(r);
}
static v8_val do_XKillClient(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XKillClient(p0,p1);
return V8_INT32(r);
}
static v8_val do_XLookupColor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XLookupColor(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XLowerWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XLowerWindow(p0,p1);
return V8_INT32(r);
}
static v8_val do_XMapRaised(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XMapRaised(p0,p1);
return V8_INT32(r);
}
static v8_val do_XMapSubwindows(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XMapSubwindows(p0,p1);
return V8_INT32(r);
}
static v8_val do_XMapWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XMapWindow(p0,p1);
return V8_INT32(r);
}
static v8_val do_XMaskEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int64_t p1 = V8_TOLONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XMaskEvent(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XMaxCmapsOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XMaxCmapsOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XMinCmapsOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XMinCmapsOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XMoveResizeWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
int r = (int) XMoveResizeWindow(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XMoveWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XMoveWindow(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XNextEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XNextEvent(p0,p1);
return V8_INT32(r);
}
static v8_val do_XNoOp(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XNoOp(p0);
return V8_INT32(r);
}
static v8_val do_XParseColor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XParseColor(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XParseGeometry(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XParseGeometry(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XPeekEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XPeekEvent(p0,p1);
return V8_INT32(r);
}
static v8_val do_XPeekIfEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
int r = (int) XPeekIfEvent(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XPending(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XPending(p0);
return V8_INT32(r);
}
static v8_val do_XPlanesOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XPlanesOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XProtocolRevision(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XProtocolRevision(p0);
return V8_INT32(r);
}
static v8_val do_XProtocolVersion(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XProtocolVersion(p0);
return V8_INT32(r);
}
static v8_val do_XPutBackEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XPutBackEvent(p0,p1);
return V8_INT32(r);
}
static v8_val do_XPutImage(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
unsigned int p8 = V8_TOUINT32(argv[8]);
unsigned int p9 = V8_TOUINT32(argv[9]);
int r = (int) XPutImage(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9);
return V8_INT32(r);
}
static v8_val do_XQLength(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XQLength(p0);
return V8_INT32(r);
}
static v8_val do_XQueryBestCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) XQueryBestCursor(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XQueryBestSize(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int r = (int) XQueryBestSize(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XQueryBestStipple(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) XQueryBestStipple(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XQueryBestTile(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) XQueryBestTile(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XQueryColor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XQueryColor(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XQueryColors(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XQueryColors(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XQueryExtension(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XQueryExtension(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XQueryPointer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
int r = (int) XQueryPointer(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XQueryTextExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
int r = (int) XQueryTextExtents(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_INT32(r);
}
static v8_val do_XQueryTextExtents16(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
int r = (int) XQueryTextExtents16(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_INT32(r);
}
static v8_val do_XQueryTree(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) XQueryTree(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XRaiseWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XRaiseWindow(p0,p1);
return V8_INT32(r);
}
static v8_val do_XReadBitmapFile(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
int r = (int) XReadBitmapFile(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_INT32(r);
}
static v8_val do_XReadBitmapFileData(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) XReadBitmapFileData(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XRebindKeysym(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int r = (int) XRebindKeysym(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XRecolorCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XRecolorCursor(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XRefreshKeyboardMapping(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XRefreshKeyboardMapping(p0);
return V8_INT32(r);
}
static v8_val do_XRemoveFromSaveSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XRemoveFromSaveSet(p0,p1);
return V8_INT32(r);
}
static v8_val do_XRemoveHost(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XRemoveHost(p0,p1);
return V8_INT32(r);
}
static v8_val do_XRemoveHosts(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XRemoveHosts(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XReparentWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XReparentWindow(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XResetScreenSaver(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XResetScreenSaver(p0);
return V8_INT32(r);
}
static v8_val do_XResizeWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
int r = (int) XResizeWindow(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XRestackWindows(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XRestackWindows(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XRotateBuffers(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XRotateBuffers(p0,p1);
return V8_INT32(r);
}
static v8_val do_XRotateWindowProperties(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XRotateWindowProperties(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XScreenCount(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XScreenCount(p0);
return V8_INT32(r);
}
static v8_val do_XSelectInput(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int64_t p2 = V8_TOLONG(argv[2]);
int r = (int) XSelectInput(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSendEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int64_t p3 = V8_TOLONG(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XSendEvent(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XSetAccessControl(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XSetAccessControl(p0,p1);
return V8_INT32(r);
}
static v8_val do_XSetArcMode(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetArcMode(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetBackground(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetBackground(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetClipMask(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetClipMask(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetClipOrigin(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XSetClipOrigin(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XSetClipRectangles(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XSetClipRectangles(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XSetCloseDownMode(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XSetCloseDownMode(p0,p1);
return V8_INT32(r);
}
static v8_val do_XSetCommand(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XSetCommand(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XSetDashes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XSetDashes(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XSetFillRule(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetFillRule(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetFillStyle(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetFillStyle(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetFont(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetFont(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetFontPath(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetFontPath(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetForeground(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetForeground(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetFunction(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetFunction(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetGraphicsExposures(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetGraphicsExposures(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetIconName(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int r = (int) XSetIconName(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetInputFocus(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int p2 = V8_TOINT32(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int r = (int) XSetInputFocus(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XSetLineAttributes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int r = (int) XSetLineAttributes(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XSetModifierMapping(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XSetModifierMapping(p0,p1);
return V8_INT32(r);
}
static v8_val do_XSetPlaneMask(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetPlaneMask(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetPointerMapping(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetPointerMapping(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetScreenSaver(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XSetScreenSaver(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XSetSelectionOwner(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int r = (int) XSetSelectionOwner(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XSetState(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int p4 = V8_TOINT32(argv[4]);
uint64_t p5 = V8_TOULONG(argv[5]);
int r = (int) XSetState(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XSetStipple(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetStipple(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetSubwindowMode(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XSetSubwindowMode(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetTSOrigin(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XSetTSOrigin(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XSetTile(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetTile(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetWindowBackground(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetWindowBackground(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetWindowBackgroundPixmap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetWindowBackgroundPixmap(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetWindowBorder(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetWindowBorder(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetWindowBorderPixmap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetWindowBorderPixmap(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetWindowBorderWidth(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
int r = (int) XSetWindowBorderWidth(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XSetWindowColormap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int r = (int) XSetWindowColormap(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XStoreBuffer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XStoreBuffer(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XStoreBytes(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XStoreBytes(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XStoreColor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XStoreColor(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XStoreColors(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int r = (int) XStoreColors(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XStoreName(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int r = (int) XStoreName(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XStoreNamedColor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int r = (int) XStoreNamedColor(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XSync(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) XSync(p0,p1);
return V8_INT32(r);
}
static v8_val do_XTextExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int r = (int) XTextExtents(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XTextExtents16(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int r = (int) XTextExtents16(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XTextWidth(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XTextWidth(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XTextWidth16(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XTextWidth16(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XTranslateCoordinates(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
int r = (int) XTranslateCoordinates(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_INT32(r);
}
static v8_val do_XUndefineCursor(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XUndefineCursor(p0,p1);
return V8_INT32(r);
}
static v8_val do_XUngrabButton(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
unsigned int p1 = V8_TOUINT32(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int r = (int) XUngrabButton(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XUngrabKey(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
unsigned int p2 = V8_TOUINT32(argv[2]);
uint64_t p3 = V8_TOULONG(argv[3]);
int r = (int) XUngrabKey(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XUngrabKeyboard(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XUngrabKeyboard(p0,p1);
return V8_INT32(r);
}
static v8_val do_XUngrabPointer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XUngrabPointer(p0,p1);
return V8_INT32(r);
}
static v8_val do_XUngrabServer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XUngrabServer(p0);
return V8_INT32(r);
}
static v8_val do_XUninstallColormap(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XUninstallColormap(p0,p1);
return V8_INT32(r);
}
static v8_val do_XUnloadFont(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XUnloadFont(p0,p1);
return V8_INT32(r);
}
static v8_val do_XUnmapSubwindows(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XUnmapSubwindows(p0,p1);
return V8_INT32(r);
}
static v8_val do_XUnmapWindow(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XUnmapWindow(p0,p1);
return V8_INT32(r);
}
static v8_val do_XVendorRelease(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XVendorRelease(p0);
return V8_INT32(r);
}
static v8_val do_XWarpPointer(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
unsigned int p5 = V8_TOUINT32(argv[5]);
unsigned int p6 = V8_TOUINT32(argv[6]);
int p7 = V8_TOINT32(argv[7]);
int p8 = V8_TOINT32(argv[8]);
int r = (int) XWarpPointer(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XWidthMMOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XWidthMMOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XWidthOfScreen(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XWidthOfScreen(p0);
return V8_INT32(r);
}
static v8_val do_XWindowEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int64_t p2 = V8_TOLONG(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int r = (int) XWindowEvent(p0,p1,p2,p3);
return V8_INT32(r);
}
static v8_val do_XWriteBitmapFile(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
uint64_t p2 = V8_TOULONG(argv[2]);
unsigned int p3 = V8_TOUINT32(argv[3]);
unsigned int p4 = V8_TOUINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
int p6 = V8_TOINT32(argv[6]);
int r = (int) XWriteBitmapFile(p0,p1,p2,p3,p4,p5,p6);
return V8_INT32(r);
}
static v8_val do_XSupportsLocale(v8_state vm, int argc, v8_val argv[]) {
int r = (int) XSupportsLocale();
return V8_INT32(r);
}
static v8_val do_XSetLocaleModifiers(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
void *r = (void *) XSetLocaleModifiers(p0);
return V8_PTR(r);
}
static v8_val do_XOpenOM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
void *r = (void *) XOpenOM(p0,p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_XCloseOM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XCloseOM(p0);
return V8_INT32(r);
}
static v8_val do_XDisplayOfOM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XDisplayOfOM(p0);
return V8_PTR(r);
}
static v8_val do_XLocaleOfOM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XLocaleOfOM(p0);
return V8_PTR(r);
}
static v8_val do_XDestroyOC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
XDestroyOC(p0);
return V8_VOID;
}
static v8_val do_XOMOfOC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XOMOfOC(p0);
return V8_PTR(r);
}
static v8_val do_XCreateFontSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *r = (void *) XCreateFontSet(p0,p1,p2,p3,p4);
return V8_PTR(r);
}
static v8_val do_XFreeFontSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
XFreeFontSet(p0,p1);
return V8_VOID;
}
static v8_val do_XFontsOfFontSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XFontsOfFontSet(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XBaseFontNameListOfFontSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XBaseFontNameListOfFontSet(p0);
return V8_PTR(r);
}
static v8_val do_XLocaleOfFontSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XLocaleOfFontSet(p0);
return V8_PTR(r);
}
static v8_val do_XContextDependentDrawing(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XContextDependentDrawing(p0);
return V8_INT32(r);
}
static v8_val do_XDirectionalDependentDrawing(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XDirectionalDependentDrawing(p0);
return V8_INT32(r);
}
static v8_val do_XContextualDrawing(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XContextualDrawing(p0);
return V8_INT32(r);
}
static v8_val do_XExtentsOfFontSet(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XExtentsOfFontSet(p0);
return V8_PTR(r);
}
static v8_val do_XmbTextEscapement(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XmbTextEscapement(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XwcTextEscapement(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) XwcTextEscapement(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_Xutf8TextEscapement(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) Xutf8TextEscapement(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XmbTextExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XmbTextExtents(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XwcTextExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) XwcTextExtents(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_Xutf8TextExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int r = (int) Xutf8TextExtents(p0,p1,p2,p3,p4);
return V8_INT32(r);
}
static v8_val do_XmbTextPerCharExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int p5 = V8_TOINT32(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
int r = (int) XmbTextPerCharExtents(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XwcTextPerCharExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int p5 = V8_TOINT32(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
int r = (int) XwcTextPerCharExtents(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_Xutf8TextPerCharExtents(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
int p5 = V8_TOINT32(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
void *p7 = V8_TOPTR(argv[7]);
void *p8 = V8_TOPTR(argv[8]);
int r = (int) Xutf8TextPerCharExtents(p0,p1,p2,p3,p4,p5,p6,p7,p8);
return V8_INT32(r);
}
static v8_val do_XmbDrawText(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
XmbDrawText(p0,p1,p2,p3,p4,p5,p6);
return V8_VOID;
}
static v8_val do_XwcDrawText(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
XwcDrawText(p0,p1,p2,p3,p4,p5,p6);
return V8_VOID;
}
static v8_val do_Xutf8DrawText(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
int p4 = V8_TOINT32(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int p6 = V8_TOINT32(argv[6]);
Xutf8DrawText(p0,p1,p2,p3,p4,p5,p6);
return V8_VOID;
}
static v8_val do_XmbDrawString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
char *p6 = V8_TOSTR(argv[6]);
int p7 = V8_TOINT32(argv[7]);
XmbDrawString(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_XwcDrawString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int p7 = V8_TOINT32(argv[7]);
XwcDrawString(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_Xutf8DrawString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
char *p6 = V8_TOSTR(argv[6]);
int p7 = V8_TOINT32(argv[7]);
Xutf8DrawString(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_XmbDrawImageString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
char *p6 = V8_TOSTR(argv[6]);
int p7 = V8_TOINT32(argv[7]);
XmbDrawImageString(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_XwcDrawImageString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
void *p6 = V8_TOPTR(argv[6]);
int p7 = V8_TOINT32(argv[7]);
XwcDrawImageString(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_Xutf8DrawImageString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
void *p3 = V8_TOPTR(argv[3]);
int p4 = V8_TOINT32(argv[4]);
int p5 = V8_TOINT32(argv[5]);
char *p6 = V8_TOSTR(argv[6]);
int p7 = V8_TOINT32(argv[7]);
Xutf8DrawImageString(p0,p1,p2,p3,p4,p5,p6,p7);
return V8_VOID;
}
static v8_val do_XOpenIM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
void *r = (void *) XOpenIM(p0,p1,p2,p3);
return V8_PTR(r);
}
static v8_val do_XCloseIM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int r = (int) XCloseIM(p0);
return V8_INT32(r);
}
static v8_val do_XDisplayOfIM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XDisplayOfIM(p0);
return V8_PTR(r);
}
static v8_val do_XLocaleOfIM(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XLocaleOfIM(p0);
return V8_PTR(r);
}
static v8_val do_XDestroyIC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
XDestroyIC(p0);
return V8_VOID;
}
static v8_val do_XSetICFocus(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
XSetICFocus(p0);
return V8_VOID;
}
static v8_val do_XUnsetICFocus(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
XUnsetICFocus(p0);
return V8_VOID;
}
static v8_val do_XwcResetIC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XwcResetIC(p0);
return V8_PTR(r);
}
static v8_val do_XmbResetIC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XmbResetIC(p0);
return V8_PTR(r);
}
static v8_val do_Xutf8ResetIC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) Xutf8ResetIC(p0);
return V8_PTR(r);
}
static v8_val do_XIMOfIC(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *r = (void *) XIMOfIC(p0);
return V8_PTR(r);
}
static v8_val do_XFilterEvent(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
uint64_t p1 = V8_TOULONG(argv[1]);
int r = (int) XFilterEvent(p0,p1);
return V8_INT32(r);
}
static v8_val do_XmbLookupString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) XmbLookupString(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XwcLookupString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) XwcLookupString(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_Xutf8LookupString(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
void *p5 = V8_TOPTR(argv[5]);
int r = (int) Xutf8LookupString(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XRegisterIMInstantiateCallback(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
char *p5 = V8_TOSTR(argv[5]);
int r = (int) XRegisterIMInstantiateCallback(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XUnregisterIMInstantiateCallback(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
char *p3 = V8_TOSTR(argv[3]);
void *p4 = V8_TOPTR(argv[4]);
char *p5 = V8_TOSTR(argv[5]);
int r = (int) XUnregisterIMInstantiateCallback(p0,p1,p2,p3,p4,p5);
return V8_INT32(r);
}
static v8_val do_XInternalConnectionNumbers(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
void *p2 = V8_TOPTR(argv[2]);
int r = (int) XInternalConnectionNumbers(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XProcessInternalConnection(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
XProcessInternalConnection(p0,p1);
return V8_VOID;
}
static v8_val do_XAddConnectionWatch(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int r = (int) XAddConnectionWatch(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do_XRemoveConnectionWatch(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
XRemoveConnectionWatch(p0,p1,p2);
return V8_VOID;
}
static v8_val do_XSetAuthorization(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
char *p2 = V8_TOSTR(argv[2]);
int p3 = V8_TOINT32(argv[3]);
XSetAuthorization(p0,p1,p2,p3);
return V8_VOID;
}
static v8_val do__Xmbtowc(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
char *p1 = V8_TOSTR(argv[1]);
int p2 = V8_TOINT32(argv[2]);
int r = (int) _Xmbtowc(p0,p1,p2);
return V8_INT32(r);
}
static v8_val do__Xwctomb(v8_state vm, int argc, v8_val argv[]) {
char *p0 = V8_TOSTR(argv[0]);
int p1 = V8_TOINT32(argv[1]);
int r = (int) _Xwctomb(p0,p1);
return V8_INT32(r);
}
static v8_val do_XGetEventData(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
int r = (int) XGetEventData(p0,p1);
return V8_INT32(r);
}
static v8_val do_XFreeEventData(v8_state vm, int argc, v8_val argv[]) {
void *p0 = V8_TOPTR(argv[0]);
void *p1 = V8_TOPTR(argv[1]);
XFreeEventData(p0,p1);
return V8_VOID;
}

// macro wrappers
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

// actually in Xutil.h
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
{ 2, do__Xmblen, "_Xmblen"},
{ 2, do_XLoadQueryFont, "XLoadQueryFont"},
{ 2, do_XQueryFont, "XQueryFont"},
{ 5, do_XGetMotionEvents, "XGetMotionEvents"},
{ 3, do_XDeleteModifiermapEntry, "XDeleteModifiermapEntry"},
{ 1, do_XGetModifierMapping, "XGetModifierMapping"},
{ 3, do_XInsertModifiermapEntry, "XInsertModifiermapEntry"},
{ 1, do_XNewModifiermap, "XNewModifiermap"},
{ 10, do_XCreateImage, "XCreateImage"},
{ 1, do_XInitImage, "XInitImage"},
{ 8, do_XGetImage, "XGetImage"},
{ 11, do_XGetSubImage, "XGetSubImage"},
{ 1, do_XOpenDisplay, "XOpenDisplay"},
{ 0, do_XrmInitialize, "XrmInitialize"},
{ 2, do_XFetchBytes, "XFetchBytes"},
{ 3, do_XFetchBuffer, "XFetchBuffer"},
{ 2, do_XGetAtomName, "XGetAtomName"},
{ 4, do_XGetAtomNames, "XGetAtomNames"},
{ 3, do_XGetDefault, "XGetDefault"},
{ 1, do_XDisplayName, "XDisplayName"},
{ 1, do_XKeysymToString, "XKeysymToString"},
{ 2, do_XSynchronize, "XSynchronize"},
{ 2, do_XSetAfterFunction, "XSetAfterFunction"},
{ 3, do_XInternAtom, "XInternAtom"},
{ 5, do_XInternAtoms, "XInternAtoms"},
{ 2, do_XCopyColormapAndFree, "XCopyColormapAndFree"},
{ 4, do_XCreateColormap, "XCreateColormap"},
{ 7, do_XCreatePixmapCursor, "XCreatePixmapCursor"},
{ 7, do_XCreateGlyphCursor, "XCreateGlyphCursor"},
{ 2, do_XCreateFontCursor, "XCreateFontCursor"},
{ 2, do_XLoadFont, "XLoadFont"},
{ 4, do_XCreateGC, "XCreateGC"},
{ 1, do_XGContextFromGC, "XGContextFromGC"},
{ 2, do_XFlushGC, "XFlushGC"},
{ 5, do_XCreatePixmap, "XCreatePixmap"},
{ 5, do_XCreateBitmapFromData, "XCreateBitmapFromData"},
{ 8, do_XCreatePixmapFromBitmapData, "XCreatePixmapFromBitmapData"},
{ 9, do_XCreateSimpleWindow, "XCreateSimpleWindow"},
{ 2, do_XGetSelectionOwner, "XGetSelectionOwner"},
{ 12, do_XCreateWindow, "XCreateWindow"},
{ 3, do_XListInstalledColormaps, "XListInstalledColormaps"},
{ 4, do_XListFonts, "XListFonts"},
{ 5, do_XListFontsWithInfo, "XListFontsWithInfo"},
{ 2, do_XGetFontPath, "XGetFontPath"},
{ 2, do_XListExtensions, "XListExtensions"},
{ 3, do_XListProperties, "XListProperties"},
{ 3, do_XListHosts, "XListHosts"},
{ 2, do_XLookupKeysym, "XLookupKeysym"},
{ 4, do_XGetKeyboardMapping, "XGetKeyboardMapping"},
{ 1, do_XStringToKeysym, "XStringToKeysym"},
{ 1, do_XMaxRequestSize, "XMaxRequestSize"},
{ 1, do_XExtendedMaxRequestSize, "XExtendedMaxRequestSize"},
{ 1, do_XResourceManagerString, "XResourceManagerString"},
{ 1, do_XScreenResourceString, "XScreenResourceString"},
{ 1, do_XDisplayMotionBufferSize, "XDisplayMotionBufferSize"},
{ 1, do_XVisualIDFromVisual, "XVisualIDFromVisual"},
{ 0, do_XInitThreads, "XInitThreads"},
{ 1, do_XLockDisplay, "XLockDisplay"},
{ 1, do_XUnlockDisplay, "XUnlockDisplay"},
{ 2, do_XInitExtension, "XInitExtension"},
{ 1, do_XAddExtension, "XAddExtension"},
{ 2, do_XFindOnExtensionList, "XFindOnExtensionList"},
{ 2, do_XRootWindow, "XRootWindow"},
{ 1, do_XDefaultRootWindow, "XDefaultRootWindow"},
{ 1, do_XRootWindowOfScreen, "XRootWindowOfScreen"},
{ 2, do_XDefaultVisual, "XDefaultVisual"},
{ 1, do_XDefaultVisualOfScreen, "XDefaultVisualOfScreen"},
{ 2, do_XDefaultGC, "XDefaultGC"},
{ 1, do_XDefaultGCOfScreen, "XDefaultGCOfScreen"},
{ 2, do_XBlackPixel, "XBlackPixel"},
{ 2, do_XWhitePixel, "XWhitePixel"},
{ 0, do_XAllPlanes, "XAllPlanes"},
{ 1, do_XBlackPixelOfScreen, "XBlackPixelOfScreen"},
{ 1, do_XWhitePixelOfScreen, "XWhitePixelOfScreen"},
{ 1, do_XNextRequest, "XNextRequest"},
{ 1, do_XLastKnownRequestProcessed, "XLastKnownRequestProcessed"},
{ 1, do_XServerVendor, "XServerVendor"},
{ 1, do_XDisplayString, "XDisplayString"},
{ 2, do_XDefaultColormap, "XDefaultColormap"},
{ 1, do_XDefaultColormapOfScreen, "XDefaultColormapOfScreen"},
{ 1, do_XDisplayOfScreen, "XDisplayOfScreen"},
{ 2, do_XScreenOfDisplay, "XScreenOfDisplay"},
{ 1, do_XDefaultScreenOfDisplay, "XDefaultScreenOfDisplay"},
{ 1, do_XEventMaskOfScreen, "XEventMaskOfScreen"},
{ 1, do_XScreenNumberOfScreen, "XScreenNumberOfScreen"},
{ 1, do_XSetErrorHandler, "XSetErrorHandler"},
{ 1, do_XSetIOErrorHandler, "XSetIOErrorHandler"},
{ 2, do_XListPixmapFormats, "XListPixmapFormats"},
{ 3, do_XListDepths, "XListDepths"},
{ 5, do_XReconfigureWMWindow, "XReconfigureWMWindow"},
{ 4, do_XGetWMProtocols, "XGetWMProtocols"},
{ 4, do_XSetWMProtocols, "XSetWMProtocols"},
{ 3, do_XIconifyWindow, "XIconifyWindow"},
{ 3, do_XWithdrawWindow, "XWithdrawWindow"},
{ 4, do_XGetCommand, "XGetCommand"},
{ 4, do_XGetWMColormapWindows, "XGetWMColormapWindows"},
{ 4, do_XSetWMColormapWindows, "XSetWMColormapWindows"},
{ 1, do_XFreeStringList, "XFreeStringList"},
{ 3, do_XSetTransientForHint, "XSetTransientForHint"},
{ 1, do_XActivateScreenSaver, "XActivateScreenSaver"},
{ 2, do_XAddHost, "XAddHost"},
{ 3, do_XAddHosts, "XAddHosts"},
{ 2, do_XAddToExtensionList, "XAddToExtensionList"},
{ 2, do_XAddToSaveSet, "XAddToSaveSet"},
{ 3, do_XAllocColor, "XAllocColor"},
{ 7, do_XAllocColorCells, "XAllocColorCells"},
{ 11, do_XAllocColorPlanes, "XAllocColorPlanes"},
{ 5, do_XAllocNamedColor, "XAllocNamedColor"},
{ 3, do_XAllowEvents, "XAllowEvents"},
{ 1, do_XAutoRepeatOff, "XAutoRepeatOff"},
{ 1, do_XAutoRepeatOn, "XAutoRepeatOn"},
{ 2, do_XBell, "XBell"},
{ 1, do_XBitmapBitOrder, "XBitmapBitOrder"},
{ 1, do_XBitmapPad, "XBitmapPad"},
{ 1, do_XBitmapUnit, "XBitmapUnit"},
{ 1, do_XCellsOfScreen, "XCellsOfScreen"},
{ 4, do_XChangeActivePointerGrab, "XChangeActivePointerGrab"},
{ 4, do_XChangeGC, "XChangeGC"},
{ 3, do_XChangeKeyboardControl, "XChangeKeyboardControl"},
{ 5, do_XChangeKeyboardMapping, "XChangeKeyboardMapping"},
{ 6, do_XChangePointerControl, "XChangePointerControl"},
{ 8, do_XChangeProperty, "XChangeProperty"},
{ 3, do_XChangeSaveSet, "XChangeSaveSet"},
{ 4, do_XChangeWindowAttributes, "XChangeWindowAttributes"},
{ 4, do_XCheckIfEvent, "XCheckIfEvent"},
{ 3, do_XCheckMaskEvent, "XCheckMaskEvent"},
{ 3, do_XCheckTypedEvent, "XCheckTypedEvent"},
{ 4, do_XCheckTypedWindowEvent, "XCheckTypedWindowEvent"},
{ 4, do_XCheckWindowEvent, "XCheckWindowEvent"},
{ 3, do_XCirculateSubwindows, "XCirculateSubwindows"},
{ 2, do_XCirculateSubwindowsDown, "XCirculateSubwindowsDown"},
{ 2, do_XCirculateSubwindowsUp, "XCirculateSubwindowsUp"},
{ 7, do_XClearArea, "XClearArea"},
{ 2, do_XClearWindow, "XClearWindow"},
{ 1, do_XCloseDisplay, "XCloseDisplay"},
{ 4, do_XConfigureWindow, "XConfigureWindow"},
{ 1, do_XConnectionNumber, "XConnectionNumber"},
{ 6, do_XConvertSelection, "XConvertSelection"},
{ 10, do_XCopyArea, "XCopyArea"},
{ 4, do_XCopyGC, "XCopyGC"},
{ 11, do_XCopyPlane, "XCopyPlane"},
{ 2, do_XDefaultDepth, "XDefaultDepth"},
{ 1, do_XDefaultDepthOfScreen, "XDefaultDepthOfScreen"},
{ 1, do_XDefaultScreen, "XDefaultScreen"},
{ 3, do_XDefineCursor, "XDefineCursor"},
{ 3, do_XDeleteProperty, "XDeleteProperty"},
{ 2, do_XDestroyWindow, "XDestroyWindow"},
{ 2, do_XDestroySubwindows, "XDestroySubwindows"},
{ 1, do_XDoesBackingStore, "XDoesBackingStore"},
{ 1, do_XDoesSaveUnders, "XDoesSaveUnders"},
{ 1, do_XDisableAccessControl, "XDisableAccessControl"},
{ 2, do_XDisplayCells, "XDisplayCells"},
{ 2, do_XDisplayHeight, "XDisplayHeight"},
{ 2, do_XDisplayHeightMM, "XDisplayHeightMM"},
{ 3, do_XDisplayKeycodes, "XDisplayKeycodes"},
{ 2, do_XDisplayPlanes, "XDisplayPlanes"},
{ 2, do_XDisplayWidth, "XDisplayWidth"},
{ 2, do_XDisplayWidthMM, "XDisplayWidthMM"},
{ 9, do_XDrawArc, "XDrawArc"},
{ 5, do_XDrawArcs, "XDrawArcs"},
{ 7, do_XDrawImageString, "XDrawImageString"},
{ 7, do_XDrawImageString16, "XDrawImageString16"},
{ 7, do_XDrawLine, "XDrawLine"},
{ 6, do_XDrawLines, "XDrawLines"},
{ 5, do_XDrawPoint, "XDrawPoint"},
{ 6, do_XDrawPoints, "XDrawPoints"},
{ 7, do_XDrawRectangle, "XDrawRectangle"},
{ 5, do_XDrawRectangles, "XDrawRectangles"},
{ 5, do_XDrawSegments, "XDrawSegments"},
{ 7, do_XDrawString, "XDrawString"},
{ 7, do_XDrawString16, "XDrawString16"},
{ 7, do_XDrawText, "XDrawText"},
{ 7, do_XDrawText16, "XDrawText16"},
{ 1, do_XEnableAccessControl, "XEnableAccessControl"},
{ 2, do_XEventsQueued, "XEventsQueued"},
{ 3, do_XFetchName, "XFetchName"},
{ 9, do_XFillArc, "XFillArc"},
{ 5, do_XFillArcs, "XFillArcs"},
{ 7, do_XFillPolygon, "XFillPolygon"},
{ 7, do_XFillRectangle, "XFillRectangle"},
{ 5, do_XFillRectangles, "XFillRectangles"},
{ 1, do_XFlush, "XFlush"},
{ 2, do_XForceScreenSaver, "XForceScreenSaver"},
{ 1, do_XFree, "XFree"},
{ 2, do_XFreeColormap, "XFreeColormap"},
{ 5, do_XFreeColors, "XFreeColors"},
{ 2, do_XFreeCursor, "XFreeCursor"},
{ 1, do_XFreeExtensionList, "XFreeExtensionList"},
{ 2, do_XFreeFont, "XFreeFont"},
{ 3, do_XFreeFontInfo, "XFreeFontInfo"},
{ 1, do_XFreeFontNames, "XFreeFontNames"},
{ 1, do_XFreeFontPath, "XFreeFontPath"},
{ 2, do_XFreeGC, "XFreeGC"},
{ 1, do_XFreeModifiermap, "XFreeModifiermap"},
{ 2, do_XFreePixmap, "XFreePixmap"},
{ 13, do_XGeometry, "XGeometry"},
{ 6, do_XGetErrorDatabaseText, "XGetErrorDatabaseText"},
{ 4, do_XGetErrorText, "XGetErrorText"},
{ 3, do_XGetFontProperty, "XGetFontProperty"},
{ 4, do_XGetGCValues, "XGetGCValues"},
{ 9, do_XGetGeometry, "XGetGeometry"},
{ 3, do_XGetIconName, "XGetIconName"},
{ 3, do_XGetInputFocus, "XGetInputFocus"},
{ 2, do_XGetKeyboardControl, "XGetKeyboardControl"},
{ 4, do_XGetPointerControl, "XGetPointerControl"},
{ 3, do_XGetPointerMapping, "XGetPointerMapping"},
{ 5, do_XGetScreenSaver, "XGetScreenSaver"},
{ 3, do_XGetTransientForHint, "XGetTransientForHint"},
{ 12, do_XGetWindowProperty, "XGetWindowProperty"},
{ 3, do_XGetWindowAttributes, "XGetWindowAttributes"},
{ 10, do_XGrabButton, "XGrabButton"},
{ 7, do_XGrabKey, "XGrabKey"},
{ 6, do_XGrabKeyboard, "XGrabKeyboard"},
{ 9, do_XGrabPointer, "XGrabPointer"},
{ 1, do_XGrabServer, "XGrabServer"},
{ 1, do_XHeightMMOfScreen, "XHeightMMOfScreen"},
{ 1, do_XHeightOfScreen, "XHeightOfScreen"},
{ 4, do_XIfEvent, "XIfEvent"},
{ 1, do_XImageByteOrder, "XImageByteOrder"},
{ 2, do_XInstallColormap, "XInstallColormap"},
{ 2, do_XKeysymToKeycode, "XKeysymToKeycode"},
{ 2, do_XKillClient, "XKillClient"},
{ 5, do_XLookupColor, "XLookupColor"},
{ 2, do_XLowerWindow, "XLowerWindow"},
{ 2, do_XMapRaised, "XMapRaised"},
{ 2, do_XMapSubwindows, "XMapSubwindows"},
{ 2, do_XMapWindow, "XMapWindow"},
{ 3, do_XMaskEvent, "XMaskEvent"},
{ 1, do_XMaxCmapsOfScreen, "XMaxCmapsOfScreen"},
{ 1, do_XMinCmapsOfScreen, "XMinCmapsOfScreen"},
{ 6, do_XMoveResizeWindow, "XMoveResizeWindow"},
{ 4, do_XMoveWindow, "XMoveWindow"},
{ 2, do_XNextEvent, "XNextEvent"},
{ 1, do_XNoOp, "XNoOp"},
{ 4, do_XParseColor, "XParseColor"},
{ 5, do_XParseGeometry, "XParseGeometry"},
{ 2, do_XPeekEvent, "XPeekEvent"},
{ 4, do_XPeekIfEvent, "XPeekIfEvent"},
{ 1, do_XPending, "XPending"},
{ 1, do_XPlanesOfScreen, "XPlanesOfScreen"},
{ 1, do_XProtocolRevision, "XProtocolRevision"},
{ 1, do_XProtocolVersion, "XProtocolVersion"},
{ 2, do_XPutBackEvent, "XPutBackEvent"},
{ 10, do_XPutImage, "XPutImage"},
{ 1, do_XQLength, "XQLength"},
{ 6, do_XQueryBestCursor, "XQueryBestCursor"},
{ 7, do_XQueryBestSize, "XQueryBestSize"},
{ 6, do_XQueryBestStipple, "XQueryBestStipple"},
{ 6, do_XQueryBestTile, "XQueryBestTile"},
{ 3, do_XQueryColor, "XQueryColor"},
{ 4, do_XQueryColors, "XQueryColors"},
{ 5, do_XQueryExtension, "XQueryExtension"},
{ 9, do_XQueryPointer, "XQueryPointer"},
{ 8, do_XQueryTextExtents, "XQueryTextExtents"},
{ 8, do_XQueryTextExtents16, "XQueryTextExtents16"},
{ 6, do_XQueryTree, "XQueryTree"},
{ 2, do_XRaiseWindow, "XRaiseWindow"},
{ 8, do_XReadBitmapFile, "XReadBitmapFile"},
{ 6, do_XReadBitmapFileData, "XReadBitmapFileData"},
{ 6, do_XRebindKeysym, "XRebindKeysym"},
{ 4, do_XRecolorCursor, "XRecolorCursor"},
{ 1, do_XRefreshKeyboardMapping, "XRefreshKeyboardMapping"},
{ 2, do_XRemoveFromSaveSet, "XRemoveFromSaveSet"},
{ 2, do_XRemoveHost, "XRemoveHost"},
{ 3, do_XRemoveHosts, "XRemoveHosts"},
{ 5, do_XReparentWindow, "XReparentWindow"},
{ 1, do_XResetScreenSaver, "XResetScreenSaver"},
{ 4, do_XResizeWindow, "XResizeWindow"},
{ 3, do_XRestackWindows, "XRestackWindows"},
{ 2, do_XRotateBuffers, "XRotateBuffers"},
{ 5, do_XRotateWindowProperties, "XRotateWindowProperties"},
{ 1, do_XScreenCount, "XScreenCount"},
{ 3, do_XSelectInput, "XSelectInput"},
{ 5, do_XSendEvent, "XSendEvent"},
{ 2, do_XSetAccessControl, "XSetAccessControl"},
{ 3, do_XSetArcMode, "XSetArcMode"},
{ 3, do_XSetBackground, "XSetBackground"},
{ 3, do_XSetClipMask, "XSetClipMask"},
{ 4, do_XSetClipOrigin, "XSetClipOrigin"},
{ 7, do_XSetClipRectangles, "XSetClipRectangles"},
{ 2, do_XSetCloseDownMode, "XSetCloseDownMode"},
{ 4, do_XSetCommand, "XSetCommand"},
{ 5, do_XSetDashes, "XSetDashes"},
{ 3, do_XSetFillRule, "XSetFillRule"},
{ 3, do_XSetFillStyle, "XSetFillStyle"},
{ 3, do_XSetFont, "XSetFont"},
{ 3, do_XSetFontPath, "XSetFontPath"},
{ 3, do_XSetForeground, "XSetForeground"},
{ 3, do_XSetFunction, "XSetFunction"},
{ 3, do_XSetGraphicsExposures, "XSetGraphicsExposures"},
{ 3, do_XSetIconName, "XSetIconName"},
{ 4, do_XSetInputFocus, "XSetInputFocus"},
{ 6, do_XSetLineAttributes, "XSetLineAttributes"},
{ 2, do_XSetModifierMapping, "XSetModifierMapping"},
{ 3, do_XSetPlaneMask, "XSetPlaneMask"},
{ 3, do_XSetPointerMapping, "XSetPointerMapping"},
{ 5, do_XSetScreenSaver, "XSetScreenSaver"},
{ 4, do_XSetSelectionOwner, "XSetSelectionOwner"},
{ 6, do_XSetState, "XSetState"},
{ 3, do_XSetStipple, "XSetStipple"},
{ 3, do_XSetSubwindowMode, "XSetSubwindowMode"},
{ 4, do_XSetTSOrigin, "XSetTSOrigin"},
{ 3, do_XSetTile, "XSetTile"},
{ 3, do_XSetWindowBackground, "XSetWindowBackground"},
{ 3, do_XSetWindowBackgroundPixmap, "XSetWindowBackgroundPixmap"},
{ 3, do_XSetWindowBorder, "XSetWindowBorder"},
{ 3, do_XSetWindowBorderPixmap, "XSetWindowBorderPixmap"},
{ 3, do_XSetWindowBorderWidth, "XSetWindowBorderWidth"},
{ 3, do_XSetWindowColormap, "XSetWindowColormap"},
{ 4, do_XStoreBuffer, "XStoreBuffer"},
{ 3, do_XStoreBytes, "XStoreBytes"},
{ 3, do_XStoreColor, "XStoreColor"},
{ 4, do_XStoreColors, "XStoreColors"},
{ 3, do_XStoreName, "XStoreName"},
{ 5, do_XStoreNamedColor, "XStoreNamedColor"},
{ 2, do_XSync, "XSync"},
{ 7, do_XTextExtents, "XTextExtents"},
{ 7, do_XTextExtents16, "XTextExtents16"},
{ 3, do_XTextWidth, "XTextWidth"},
{ 3, do_XTextWidth16, "XTextWidth16"},
{ 8, do_XTranslateCoordinates, "XTranslateCoordinates"},
{ 2, do_XUndefineCursor, "XUndefineCursor"},
{ 4, do_XUngrabButton, "XUngrabButton"},
{ 4, do_XUngrabKey, "XUngrabKey"},
{ 2, do_XUngrabKeyboard, "XUngrabKeyboard"},
{ 2, do_XUngrabPointer, "XUngrabPointer"},
{ 1, do_XUngrabServer, "XUngrabServer"},
{ 2, do_XUninstallColormap, "XUninstallColormap"},
{ 2, do_XUnloadFont, "XUnloadFont"},
{ 2, do_XUnmapSubwindows, "XUnmapSubwindows"},
{ 2, do_XUnmapWindow, "XUnmapWindow"},
{ 1, do_XVendorRelease, "XVendorRelease"},
{ 9, do_XWarpPointer, "XWarpPointer"},
{ 1, do_XWidthMMOfScreen, "XWidthMMOfScreen"},
{ 1, do_XWidthOfScreen, "XWidthOfScreen"},
{ 4, do_XWindowEvent, "XWindowEvent"},
{ 7, do_XWriteBitmapFile, "XWriteBitmapFile"},
{ 0, do_XSupportsLocale, "XSupportsLocale"},
{ 1, do_XSetLocaleModifiers, "XSetLocaleModifiers"},
{ 4, do_XOpenOM, "XOpenOM"},
{ 1, do_XCloseOM, "XCloseOM"},
{ 1, do_XDisplayOfOM, "XDisplayOfOM"},
{ 1, do_XLocaleOfOM, "XLocaleOfOM"},
{ 1, do_XDestroyOC, "XDestroyOC"},
{ 1, do_XOMOfOC, "XOMOfOC"},
{ 5, do_XCreateFontSet, "XCreateFontSet"},
{ 2, do_XFreeFontSet, "XFreeFontSet"},
{ 3, do_XFontsOfFontSet, "XFontsOfFontSet"},
{ 1, do_XBaseFontNameListOfFontSet, "XBaseFontNameListOfFontSet"},
{ 1, do_XLocaleOfFontSet, "XLocaleOfFontSet"},
{ 1, do_XContextDependentDrawing, "XContextDependentDrawing"},
{ 1, do_XDirectionalDependentDrawing, "XDirectionalDependentDrawing"},
{ 1, do_XContextualDrawing, "XContextualDrawing"},
{ 1, do_XExtentsOfFontSet, "XExtentsOfFontSet"},
{ 3, do_XmbTextEscapement, "XmbTextEscapement"},
{ 3, do_XwcTextEscapement, "XwcTextEscapement"},
{ 3, do_Xutf8TextEscapement, "Xutf8TextEscapement"},
{ 5, do_XmbTextExtents, "XmbTextExtents"},
{ 5, do_XwcTextExtents, "XwcTextExtents"},
{ 5, do_Xutf8TextExtents, "Xutf8TextExtents"},
{ 9, do_XmbTextPerCharExtents, "XmbTextPerCharExtents"},
{ 9, do_XwcTextPerCharExtents, "XwcTextPerCharExtents"},
{ 9, do_Xutf8TextPerCharExtents, "Xutf8TextPerCharExtents"},
{ 7, do_XmbDrawText, "XmbDrawText"},
{ 7, do_XwcDrawText, "XwcDrawText"},
{ 7, do_Xutf8DrawText, "Xutf8DrawText"},
{ 8, do_XmbDrawString, "XmbDrawString"},
{ 8, do_XwcDrawString, "XwcDrawString"},
{ 8, do_Xutf8DrawString, "Xutf8DrawString"},
{ 8, do_XmbDrawImageString, "XmbDrawImageString"},
{ 8, do_XwcDrawImageString, "XwcDrawImageString"},
{ 8, do_Xutf8DrawImageString, "Xutf8DrawImageString"},
{ 4, do_XOpenIM, "XOpenIM"},
{ 1, do_XCloseIM, "XCloseIM"},
{ 1, do_XDisplayOfIM, "XDisplayOfIM"},
{ 1, do_XLocaleOfIM, "XLocaleOfIM"},
{ 1, do_XDestroyIC, "XDestroyIC"},
{ 1, do_XSetICFocus, "XSetICFocus"},
{ 1, do_XUnsetICFocus, "XUnsetICFocus"},
{ 1, do_XwcResetIC, "XwcResetIC"},
{ 1, do_XmbResetIC, "XmbResetIC"},
{ 1, do_Xutf8ResetIC, "Xutf8ResetIC"},
{ 1, do_XIMOfIC, "XIMOfIC"},
{ 2, do_XFilterEvent, "XFilterEvent"},
{ 6, do_XmbLookupString, "XmbLookupString"},
{ 6, do_XwcLookupString, "XwcLookupString"},
{ 6, do_Xutf8LookupString, "Xutf8LookupString"},
{ 6, do_XRegisterIMInstantiateCallback, "XRegisterIMInstantiateCallback"},
{ 6, do_XUnregisterIMInstantiateCallback, "XUnregisterIMInstantiateCallback"},
{ 3, do_XInternalConnectionNumbers, "XInternalConnectionNumbers"},
{ 2, do_XProcessInternalConnection, "XProcessInternalConnection"},
{ 3, do_XAddConnectionWatch, "XAddConnectionWatch"},
{ 3, do_XRemoveConnectionWatch, "XRemoveConnectionWatch"},
{ 4, do_XSetAuthorization, "XSetAuthorization"},
{ 3, do__Xmbtowc, "_Xmbtowc"},
{ 2, do__Xwctomb, "_Xwctomb"},
{ 2, do_XGetEventData, "XGetEventData"},
{ 2, do_XFreeEventData, "XFreeEventData"},
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
this.XOMOrientation_LTR_TTB = 0;\
this.XOMOrientation_RTL_TTB = 1;\
this.XOMOrientation_TTB_LTR = 2;\
this.XOMOrientation_TTB_RTL = 3;\
this.XOMOrientation_Context = 4;\
this.XIMForwardChar = 0;\
this.XIMBackwardChar = 1;\
this.XIMForwardWord = 2;\
this.XIMBackwardWord = 3;\
this.XIMCaretUp = 4;\
this.XIMCaretDown = 5;\
this.XIMNextLine = 6;\
this.XIMPreviousLine = 7;\
this.XIMLineStart = 8;\
this.XIMLineEnd = 9;\
this.XIMAbsolutePosition = 10;\
this.XIMDontChange = 11;\
this.XIMIsInvisible = 0;\
this.XIMIsPrimary = 1;\
this.XIMIsSecondary = 2;\
this.XIMTextType = 0;\
this.XIMBitmapType = 1;\
_s = { number : 6881280, next : 7340040, free_private : 7340048, private_data : 7340056 }; _s['#size'] = 32;\
_tags['_XExtData'] = _s;\
_types['XExtData'] = _s;\
_s = { extension : 6881280, major_opcode : 6881284, first_event : 6881288, first_error : 6881292 }; _s['#size'] = 16;\
_types['XExtCodes'] = _s;\
_s = { depth : 6881280, bits_per_pixel : 6881284, scanline_pad : 6881288 }; _s['#size'] = 12;\
_types['XPixmapFormatValues'] = _s;\
_s = { function : 6881280, plane_mask : 4849672, foreground : 4849680, background : 4849688, line_width : 6881312, line_style : 6881316, cap_style : 6881320, join_style : 6881324, fill_style : 6881328, fill_rule : 6881332, arc_mode : 6881336, tile : 4849728, stipple : 4849736, ts_x_origin : 6881360, ts_y_origin : 6881364, font : 4849752, subwindow_mode : 6881376, graphics_exposures : 6881380, clip_x_origin : 6881384, clip_y_origin : 6881388, clip_mask : 4849776, dash_offset : 6881400, dashes : 6422652 }; _s['#size'] = 128;\
_types['XGCValues'] = _s;\
_s = { ext_data : 7340032, visualid : 4849672, class : 6881296, red_mask : 4849688, green_mask : 4849696, blue_mask : 4849704, bits_per_rgb : 6881328, map_entries : 6881332 }; _s['#size'] = 56;\
_types['Visual'] = _s;\
_s = { depth : 6881280, nvisuals : 6881284, visuals : 7340040 }; _s['#size'] = 16;\
_types['Depth'] = _s;\
_s = { ext_data : 7340032, display : 7340040, root : 4849680, width : 6881304, height : 6881308, mwidth : 6881312, mheight : 6881316, ndepths : 6881320, depths : 7340080, root_depth : 6881336, root_visual : 7340096, default_gc : 7340104, cmap : 4849744, white_pixel : 4849752, black_pixel : 4849760, max_maps : 6881384, min_maps : 6881388, backing_store : 6881392, save_unders : 6881396, root_input_mask : 6946936 }; _s['#size'] = 128;\
_types['Screen'] = _s;\
_s = { ext_data : 7340032, depth : 6881288, bits_per_pixel : 6881292, scanline_pad : 6881296 }; _s['#size'] = 24;\
_types['ScreenFormat'] = _s;\
_s = { background_pixmap : 4849664, background_pixel : 4849672, border_pixmap : 4849680, border_pixel : 4849688, bit_gravity : 6881312, win_gravity : 6881316, backing_store : 6881320, backing_planes : 4849712, backing_pixel : 4849720, save_under : 6881344, event_mask : 6946888, do_not_propagate_mask : 6946896, override_redirect : 6881368, colormap : 4849760, cursor : 4849768 }; _s['#size'] = 112;\
_types['XSetWindowAttributes'] = _s;\
_s = { x : 6881280, y : 6881284, width : 6881288, height : 6881292, border_width : 6881296, depth : 6881300, visual : 7340056, root : 4849696, class : 6881320, bit_gravity : 6881324, win_gravity : 6881328, backing_store : 6881332, backing_planes : 4849720, backing_pixel : 4849728, save_under : 6881352, colormap : 4849744, map_installed : 6881368, map_state : 6881372, all_event_masks : 6946912, your_event_mask : 6946920, do_not_propagate_mask : 6946928, override_redirect : 6881400, screen : 7340160 }; _s['#size'] = 136;\
_types['XWindowAttributes'] = _s;\
_s = { family : 6881280, length : 6881284, address : 7340040 }; _s['#size'] = 16;\
_types['XHostAddress'] = _s;\
_s = { typelength : 6881280, valuelength : 6881284, type : 7340040, value : 7340048 }; _s['#size'] = 24;\
_types['XServerInterpretedAddress'] = _s;\
_s = { width : 6881280, height : 6881284, xoffset : 6881288, format : 6881292, data : 7340048, byte_order : 6881304, bitmap_unit : 6881308, bitmap_bit_order : 6881312, bitmap_pad : 6881316, depth : 6881320, bytes_per_line : 6881324, bits_per_pixel : 6881328, red_mask : 4849720, green_mask : 4849728, blue_mask : 4849736, obdata : 7340112, f : 88 }; _s['#size'] = 136;\
_tags['_XImage'] = _s;\
_types['XImage'] = _s;\
_s = { create_image : 7340032, destroy_image : 7340040, get_pixel : 7340048, put_pixel : 7340056, sub_image : 7340064, add_pixel : 7340072 }; _s['#size'] = 48;\
_tags['funcs'] = _s;\
_s = { x : 6881280, y : 6881284, width : 6881288, height : 6881292, border_width : 6881296, sibling : 4849688, stack_mode : 6881312 }; _s['#size'] = 40;\
_types['XWindowChanges'] = _s;\
_s = { pixel : 4849664, red : 4718600, green : 4718602, blue : 4718604, flags : 6422542, pad : 6422543 }; _s['#size'] = 16;\
_types['XColor'] = _s;\
_s = { x1 : 6815744, y1 : 6815746, x2 : 6815748, y2 : 6815750 }; _s['#size'] = 8;\
_types['XSegment'] = _s;\
_s = { x : 6815744, y : 6815746 }; _s['#size'] = 4;\
_types['XPoint'] = _s;\
_s = { x : 6815744, y : 6815746, width : 4718596, height : 4718598 }; _s['#size'] = 8;\
_types['XRectangle'] = _s;\
_s = { x : 6815744, y : 6815746, width : 4718596, height : 4718598, angle1 : 6815752, angle2 : 6815754 }; _s['#size'] = 12;\
_types['XArc'] = _s;\
_s = { key_click_percent : 6881280, bell_percent : 6881284, bell_pitch : 6881288, bell_duration : 6881292, led : 6881296, led_mode : 6881300, key : 6881304, auto_repeat_mode : 6881308 }; _s['#size'] = 32;\
_types['XKeyboardControl'] = _s;\
_s = { key_click_percent : 6881280, bell_percent : 6881284, bell_pitch : 4784136, bell_duration : 4784140, led_mask : 4849680, global_auto_repeat : 6881304, auto_repeats : 28 }; _s['#size'] = 64;\
_types['XKeyboardState'] = _s;\
_s = { time : 4849664, x : 6815752, y : 6815754 }; _s['#size'] = 16;\
_types['XTimeCoord'] = _s;\
_s = { max_keypermod : 6881280, modifiermap : 7340040 }; _s['#size'] = 16;\
_types['XModifierKeymap'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, keycode : 4784212, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XKeyEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, keycode : 4784212, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XKeyPressedEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, keycode : 4784212, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XKeyReleasedEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, button : 4784212, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XButtonEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, button : 4784212, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XButtonPressedEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, button : 4784212, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XButtonReleasedEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, is_hint : 6422612, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XMotionEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, state : 4784208, is_hint : 6422612, same_screen : 6881368 }; _s['#size'] = 96;\
_types['XPointerMovedEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, mode : 6881360, detail : 6881364, same_screen : 6881368, focus : 6881372, state : 4784224 }; _s['#size'] = 104;\
_types['XCrossingEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, mode : 6881360, detail : 6881364, same_screen : 6881368, focus : 6881372, state : 4784224 }; _s['#size'] = 104;\
_types['XEnterWindowEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, root : 4849704, subwindow : 4849712, time : 4849720, x : 6881344, y : 6881348, x_root : 6881352, y_root : 6881356, mode : 6881360, detail : 6881364, same_screen : 6881368, focus : 6881372, state : 4784224 }; _s['#size'] = 104;\
_types['XLeaveWindowEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, mode : 6881320, detail : 6881324 }; _s['#size'] = 48;\
_types['XFocusChangeEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, mode : 6881320, detail : 6881324 }; _s['#size'] = 48;\
_types['XFocusInEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, mode : 6881320, detail : 6881324 }; _s['#size'] = 48;\
_types['XFocusOutEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, key_vector : 40 }; _s['#size'] = 72;\
_types['XKeymapEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, x : 6881320, y : 6881324, width : 6881328, height : 6881332, count : 6881336 }; _s['#size'] = 64;\
_types['XExposeEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, drawable : 4849696, x : 6881320, y : 6881324, width : 6881328, height : 6881332, count : 6881336, major_code : 6881340, minor_code : 6881344 }; _s['#size'] = 72;\
_types['XGraphicsExposeEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, drawable : 4849696, major_code : 6881320, minor_code : 6881324 }; _s['#size'] = 48;\
_types['XNoExposeEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, state : 6881320 }; _s['#size'] = 48;\
_types['XVisibilityEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, parent : 4849696, window : 4849704, x : 6881328, y : 6881332, width : 6881336, height : 6881340, border_width : 6881344, override_redirect : 6881348 }; _s['#size'] = 72;\
_types['XCreateWindowEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, event : 4849696, window : 4849704 }; _s['#size'] = 48;\
_types['XDestroyWindowEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, event : 4849696, window : 4849704, from_configure : 6881328 }; _s['#size'] = 56;\
_types['XUnmapEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, event : 4849696, window : 4849704, override_redirect : 6881328 }; _s['#size'] = 56;\
_types['XMapEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, parent : 4849696, window : 4849704 }; _s['#size'] = 48;\
_types['XMapRequestEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, event : 4849696, window : 4849704, parent : 4849712, x : 6881336, y : 6881340, override_redirect : 6881344 }; _s['#size'] = 72;\
_types['XReparentEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, event : 4849696, window : 4849704, x : 6881328, y : 6881332, width : 6881336, height : 6881340, border_width : 6881344, above : 4849736, override_redirect : 6881360 }; _s['#size'] = 88;\
_types['XConfigureEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, event : 4849696, window : 4849704, x : 6881328, y : 6881332 }; _s['#size'] = 56;\
_types['XGravityEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, width : 6881320, height : 6881324 }; _s['#size'] = 48;\
_types['XResizeRequestEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, parent : 4849696, window : 4849704, x : 6881328, y : 6881332, width : 6881336, height : 6881340, border_width : 6881344, above : 4849736, detail : 6881360, value_mask : 4849752 }; _s['#size'] = 96;\
_types['XConfigureRequestEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, event : 4849696, window : 4849704, place : 6881328 }; _s['#size'] = 56;\
_types['XCirculateEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, parent : 4849696, window : 4849704, place : 6881328 }; _s['#size'] = 56;\
_types['XCirculateRequestEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, atom : 4849704, time : 4849712, state : 6881336 }; _s['#size'] = 64;\
_types['XPropertyEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, selection : 4849704, time : 4849712 }; _s['#size'] = 56;\
_types['XSelectionClearEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, owner : 4849696, requestor : 4849704, selection : 4849712, target : 4849720, property : 4849728, time : 4849736 }; _s['#size'] = 80;\
_types['XSelectionRequestEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, requestor : 4849696, selection : 4849704, target : 4849712, property : 4849720, time : 4849728 }; _s['#size'] = 72;\
_types['XSelectionEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, colormap : 4849704, new : 6881328, state : 6881332 }; _s['#size'] = 56;\
_types['XColormapEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, message_type : 4849704, format : 6881328, data : 56 }; _s['#size'] = 96;\
_types['XClientMessageEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696, request : 6881320, first_keycode : 6881324, count : 6881328 }; _s['#size'] = 56;\
_types['XMappingEvent'] = _s;\
_s = { type : 6881280, display : 7340040, resourceid : 4849680, serial : 4849688, error_code : 4325408, request_code : 4325409, minor_code : 4325410 }; _s['#size'] = 40;\
_types['XErrorEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, window : 4849696 }; _s['#size'] = 40;\
_types['XAnyEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, extension : 6881312, evtype : 6881316 }; _s['#size'] = 40;\
_types['XGenericEvent'] = _s;\
_s = { type : 6881280, serial : 4849672, send_event : 6881296, display : 7340056, extension : 6881312, evtype : 6881316, cookie : 4784168, data : 7340080 }; _s['#size'] = 56;\
_types['XGenericEventCookie'] = _s;\
_s = { type : 6881280, xany : 0, xkey : 0, xbutton : 0, xmotion : 0, xcrossing : 0, xfocus : 0, xexpose : 0, xgraphicsexpose : 0, xnoexpose : 0, xvisibility : 0, xcreatewindow : 0, xdestroywindow : 0, xunmap : 0, xmap : 0, xmaprequest : 0, xreparent : 0, xconfigure : 0, xgravity : 0, xresizerequest : 0, xconfigurerequest : 0, xcirculate : 0, xcirculaterequest : 0, xproperty : 0, xselectionclear : 0, xselectionrequest : 0, xselection : 0, xcolormap : 0, xclient : 0, xmapping : 0, xerror : 0, xkeymap : 0, xgeneric : 0, xcookie : 0, pad : 0 }; _s['#size'] = 192;\
_tags['_XEvent'] = _s;\
_types['XEvent'] = _s;\
_s = { lbearing : 6815744, rbearing : 6815746, width : 6815748, ascent : 6815750, descent : 6815752, attributes : 4718602 }; _s['#size'] = 12;\
_types['XCharStruct'] = _s;\
_s = { name : 4849664, card32 : 4849672 }; _s['#size'] = 16;\
_types['XFontProp'] = _s;\
_s = { ext_data : 7340032, fid : 4849672, direction : 4784144, min_char_or_byte2 : 4784148, max_char_or_byte2 : 4784152, min_byte1 : 4784156, max_byte1 : 4784160, all_chars_exist : 6881316, default_char : 4784168, n_properties : 6881324, properties : 7340080, min_bounds : 56, max_bounds : 68, per_char : 7340112, ascent : 6881368, descent : 6881372 }; _s['#size'] = 96;\
_types['XFontStruct'] = _s;\
_s = { chars : 7340032, nchars : 6881288, delta : 6881292, font : 4849680 }; _s['#size'] = 24;\
_types['XTextItem'] = _s;\
_s = { byte1 : 4325376, byte2 : 4325377 }; _s['#size'] = 2;\
_types['XChar2b'] = _s;\
_s = { chars : 7340032, nchars : 6881288, delta : 6881292, font : 4849680 }; _s['#size'] = 24;\
_types['XTextItem16'] = _s;\
_s = { display : 7340032, gc : 7340032, visual : 7340032, screen : 7340032, pixmap_format : 7340032, font : 7340032 }; _s['#size'] = 8;\
_types['XEDataObject'] = _s;\
_s = { max_ink_extent : 0, max_logical_extent : 8 }; _s['#size'] = 16;\
_types['XFontSetExtents'] = _s;\
_s = { chars : 7340032, nchars : 6881288, delta : 6881292, font_set : 7340048 }; _s['#size'] = 24;\
_types['XmbTextItem'] = _s;\
_s = { chars : 7340032, nchars : 6881288, delta : 6881292, font_set : 7340048 }; _s['#size'] = 24;\
_types['XwcTextItem'] = _s;\
_s = { charset_count : 6881280, charset_list : 7340040 }; _s['#size'] = 16;\
_types['XOMCharSetList'] = _s;\
_s = { num_orientation : 6881280, orientation : 7340040 }; _s['#size'] = 16;\
_types['XOMOrientation'] = _s;\
_s = { num_font : 6881280, font_struct_list : 7340040, font_name_list : 7340048 }; _s['#size'] = 24;\
_types['XOMFontInfo'] = _s;\
_s = { count_styles : 4718592, supported_styles : 7340040 }; _s['#size'] = 16;\
_types['XIMStyles'] = _s;\
_s = { client_data : 7340032, callback : 7340040 }; _s['#size'] = 16;\
_types['XIMCallback'] = _s;\
_s = { client_data : 7340032, callback : 7340040 }; _s['#size'] = 16;\
_types['XICCallback'] = _s;\
_s = { length : 4718592, feedback : 7340040, encoding_is_wchar : 6881296, string : 24 }; _s['#size'] = 32;\
_tags['_XIMText'] = _s;\
_types['XIMText'] = _s;\
_s = { state : 4849664 }; _s['#size'] = 8;\
_tags['_XIMPreeditStateNotifyCallbackStruct'] = _s;\
_types['XIMPreeditStateNotifyCallbackStruct'] = _s;\
_s = { length : 4718592, feedback : 7340040, encoding_is_wchar : 6881296, string : 24 }; _s['#size'] = 32;\
_tags['_XIMStringConversionText'] = _s;\
_types['XIMStringConversionText'] = _s;\
_s = { position : 4718592, direction : 109, operation : 4718600, factor : 4718602, text : 7340048 }; _s['#size'] = 24;\
_tags['_XIMStringConversionCallbackStruct'] = _s;\
_types['XIMStringConversionCallbackStruct'] = _s;\
_s = { caret : 6881280, chg_first : 6881284, chg_length : 6881288, text : 7340048 }; _s['#size'] = 24;\
_tags['_XIMPreeditDrawCallbackStruct'] = _s;\
_types['XIMPreeditDrawCallbackStruct'] = _s;\
_s = { position : 6881280, direction : 109, style : 105 }; _s['#size'] = 12;\
_tags['_XIMPreeditCaretCallbackStruct'] = _s;\
_types['XIMPreeditCaretCallbackStruct'] = _s;\
_s = { type : 105, data : 8 }; _s['#size'] = 16;\
_tags['_XIMStatusDrawCallbackStruct'] = _s;\
_types['XIMStatusDrawCallbackStruct'] = _s;\
_s = { keysym : 4849664, modifier : 6881288, modifier_mask : 6881292 }; _s['#size'] = 16;\
_tags['_XIMHotKeyTrigger'] = _s;\
_types['XIMHotKeyTrigger'] = _s;\
_s = { num_hot_key : 6881280, key : 7340040 }; _s['#size'] = 16;\
_tags['_XIMHotKeyTriggers'] = _s;\
_types['XIMHotKeyTriggers'] = _s;\
_s = { count_values : 4718592, supported_values : 7340040 }; _s['#size'] = 16;\
_types['XIMValuesList'] = _s;\
this['#tags'] = _tags;this['#types'] = _types;});";

int JS_LOAD(v8_state vm, v8_val hobj) {
v8_val rc = jsv8->callstr(vm, source_str_, hobj, 0, NULL);
if (V8_ISERROR(rc)) return -1;
JS_EXPORT(fntab_);
}
