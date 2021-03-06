TOPDIR := ../..
include $(TOPDIR)/config.mk

GENCODE := $(TOPDIR)/tools/gencode
CLANG_DIR := /usr/include/clang/3.8


all: libgobject.so libgdk-pixbuf.so libgdk-pixbuf-loader.so \
libpixbuf-await.so libgdk-cairo.so

SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

gdk-pixbuf.c:
	$(GENCODE) --strip gdk_pixbuf_ -include gdk-pixbuf/gdk-pixbuf.h \
/usr/include/gdk-pixbuf-2.0/gdk-pixbuf/gdk-pixbuf-core.h -- \
`pkg-config --cflags gdk-pixbuf-2.0` -I$(CLANG_DIR)/include > $@

gdk-pixbuf-loader.c:
	$(GENCODE) --strip gdk_pixbuf_loader_ -include gdk-pixbuf/gdk-pixbuf-loader.h \
/usr/include/gdk-pixbuf-2.0/gdk-pixbuf/gdk-pixbuf-loader.h -- \
`pkg-config --cflags gdk-pixbuf-2.0` -I$(CLANG_DIR)/include > $@

gdk-cairo.c:
	$(GENCODE) --strip gdk_cairo_ -include gdk/gdkcairo.h \
/usr/include/gtk-2.0/gdk/gdkcairo.h -- \
`pkg-config --cflags gdk-2.0` -I$(CLANG_DIR)/include > $@

gdk-cairo.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h gdk-cairo.c
	$(CC) `pkg-config --cflags gdk-2.0` $(SO_CFLAGS) gdk-cairo.c

libgdk-cairo.so: gdk-cairo.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ `pkg-config --libs gdk-2.0`

gobject.c:
	$(GENCODE) --strip g_object_ -include gobject-min.h \
./gobject-min.h -- \
`pkg-config --cflags gdk-2.0` -I$(CLANG_DIR)/include > $@

gobject.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h gobject.c
	$(CC) `pkg-config --cflags gdk-2.0` $(SO_CFLAGS) gobject.c

libgobject.so: gobject.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ `pkg-config --libs gdk-2.0`

gdk-pixbuf.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h gdk-pixbuf.c
	$(CC) `pkg-config --cflags gdk-pixbuf-2.0` $(SO_CFLAGS) gdk-pixbuf.c

libgdk-pixbuf.so: gdk-pixbuf.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ `pkg-config --libs gdk-pixbuf-2.0`

gdk-pixbuf-loader.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h gdk-pixbuf-loader.c
	$(CC) `pkg-config --cflags gdk-pixbuf-2.0` $(SO_CFLAGS) gdk-pixbuf-loader.c

libgdk-pixbuf-loader.so: gdk-pixbuf-loader.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ `pkg-config --libs gdk-pixbuf-2.0`

pixbuf-await.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h pixbuf-await.c
	$(CC) `pkg-config --cflags gdk-pixbuf-2.0` -I$(MILL_INCLUDES) $(SO_CFLAGS) pixbuf-await.c

libpixbuf-await.so: pixbuf-await.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ $(MILL_LIBS) `pkg-config --libs gdk-pixbuf-2.0`

clean:
	rm -f *.o
	rm -f *.so
	rm -f gdk-*.c
