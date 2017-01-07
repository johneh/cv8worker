//#include <X11/Xlib.h>

typedef void Display;
typedef void Visual;
typedef unsigned long Window;
typedef unsigned long Colormap;
typedef void *Region;
typedef void *GC;
typedef int Bool;

typedef struct {
	short x, y;
	unsigned short width, height;
} XRectangle;

extern int DefaultScreen(Display *display);
Visual *DefaultVisual(Display *display, int screen_number);
//Window DefaultRootWindow(Display *display);
Window RootWindow(Display *display, int screen_number);

extern  unsigned long BlackPixel(Display *display, int screen_number);

extern  unsigned long WhitePixel(Display *display, int screen_number);

extern int ConnectionNumber(Display *display);

extern Colormap DefaultColormap(Display *display, int screen_number);

// This is in Xutil.h!!!
extern int XLookupString(/*XKeyEvent*/ void *event_struct,
         char *buffer_return, int bytes_buffer,
        /*KeySym*/void *keysym_return, /*XComposeStatus*/ void *status_in_out);

Region XCreateRegion(void);
int XSetRegion(Display *display, GC gc, Region r);
int XDestroyRegion(Region r);
int XIntersectRegion(Region sra, Region srb, Region dr_return);
int XUnionRegion(Region sra, Region srb, Region dr_return);
int XUnionRectWithRegion(XRectangle *rectangle, Region src_region,
              Region dest_region_return);
int XSubtractRegion(Region sra, Region srb, Region dr_return);
int XXorRegion(Region sra, Region srb, Region dr_return);
int XOffsetRegion(Region r, int dx, int dy);
int XShrinkRegion(Region r, int dx, int dy);
Bool XEmptyRegion(Region r);
Bool XEqualRegion(Region r1, Region r2);
Bool XPointInRegion(Region r, int x, int y);
int XRectInRegion(Region r, int x, int y, unsigned int width, unsigned
              int height);
