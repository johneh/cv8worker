#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "libpill.h"
#include "jsv8dlfn.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <glib-object.h>    // g_signal_connect ...

enum {
    SIZE_PREPARED = 1,
    AREA_PREPARED,
    AREA_UPDATED,
    LOADER_CLOSED,
    PIXBUF_OFFSET = 6,
    DATA_COUNT = 8,
};


static int *
make_data(int type, int x, int y, int height, int width) {
    int *data = malloc(DATA_COUNT * sizeof (int)); /* extra required for area-prepared */
    if (!data)
        jsv8->panic_("Out of memory", __func__);
    data[0] = type;
    data[1] = 0;
    data[2] = 0;
    data[3] = width;
    data[4] = height;
    return data;
}


#define send_data(ch, data) chs(ch, int *, data)

static void
on_area_updated(GdkPixbufLoader *loader, gint x, gint y,
            gint width, gint height, gpointer user_data)
{
    chan ch = user_data;
    int *data = make_data(AREA_UPDATED, x, y, width, height);
    send_data(ch, data);
}

static void
on_size_preped(GdkPixbufLoader *loader,
            gint width, gint height, gpointer user_data)
{
    chan ch = user_data;
    int *data = make_data(SIZE_PREPARED, 0, 0, width, height);
    send_data(ch, data);
}

static void
on_area_preped(GdkPixbufLoader *loader, gpointer user_data)
{
    chan ch = user_data;
    int *data = make_data(AREA_PREPARED, 0, 0, 0, 0);
    if (data) {
        void **ptr = (void **) &data[PIXBUF_OFFSET];
        *ptr = gdk_pixbuf_loader_get_pixbuf(loader);
    }
    send_data(ch, data);
}

static void
on_closed(GdkPixbufLoader *loader, gpointer user_data)
{
    chan ch = user_data;
    int *data = make_data(LOADER_CLOSED, 0, 0, 0, 0);
    send_data(ch, data);
}

coroutine static void
do_pixbuf_loader_await(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    GdkPixbufLoader *loader = V8_TOPTR(args[0]);
    chan ch = chmake(int *, 1);
    if (!ch)
        jsv8->panic_("Out of memory", __func__);
    g_signal_connect(loader, "area_prepared", (GCallback) on_area_preped, ch);
    g_signal_connect(loader, "area_updated", (GCallback) on_area_updated, ch);
    g_signal_connect(loader, "size-prepared", (GCallback) on_size_preped, ch);
    g_signal_connect(loader, "closed", (GCallback) on_closed, ch);

    // READY signal
    int *data = make_data(0, 0, 0, 0, 0);
    jsv8->goresolve(vm, cr, V8_BUFFER(data, DATA_COUNT * 4), 0);

    while (1) {
        int *ptr = chr(ch, int *);
        if (ptr[0] == LOADER_CLOSED) {
            g_signal_handlers_disconnect_by_data(loader, ch);
            chclose(ch);
            jsv8->goresolve(vm, cr, V8_BUFFER(ptr, DATA_COUNT * 4), 1);
            return;
        }
        jsv8->goresolve(vm, cr, V8_BUFFER(ptr, DATA_COUNT * 4), 0);
    }
}

static v8_ffn ff_table[] = {
    {1, do_pixbuf_loader_await, "await", FN_COROPUSH },
    {0}
};

int JS_LOAD(v8_state vm, v8_val hlib) {
    JS_EXPORT(ff_table);
}

