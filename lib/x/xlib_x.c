#include <X11/Xlib.h>
#include <X11/Xutil.h>

int queryPointer(Display *display, Window w, int *r) {
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned mask;
    if (XQueryPointer(display, w, &root, &child,
            &root_x, &root_y, &win_x, &win_y, &mask)) {
        r[0] = win_x;
        r[1] = win_y;
        r[2] = mask;
        return 1;
    }
    return 0;
}

