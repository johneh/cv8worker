TOPDIR := ../..
include $(TOPDIR)/config.mk

GENCODE := $(TOPDIR)/tools/gencode
CLANG_DIR := /usr/include/clang/3.8

CAIRO_DIR := /usr/include/cairo
PANGO_DIR := /usr/include/pango-1.0/pango

#INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
#LIBS = $(V8_LIBS) $(MILL_LIBS_STATIC) -lm -lstdc++

all: libpango.so libcairo.so libcairoxlib.so

SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

pango_bindings.c:
	$(GENCODE) --strip pango_ -include pango/pango.h:pango/pangocairo.h \
$(PANGO_DIR)/pango-context.h $(PANGO_DIR)/pango-font.h \
$(PANGO_DIR)/pango-layout.h $(PANGO_DIR)/pangocairo.h -- \
`pkg-config --cflags pango cairo` -I$(CLANG_DIR)/include > $@

cairo_bindings.c: cairo_x.h
	$(GENCODE) --strip cairo_ -include cairo/cairo.h:cairo_x.h \
$(CAIRO_DIR)/cairo.h cairo_x.h -- \
`pkg-config --cflags cairo` -I$(CLANG_DIR)/include > $@

pango_bindings.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h pango_bindings.c
	$(CC) `pkg-config --cflags pangocairo` $(SO_CFLAGS) pango_bindings.c

libpango.so: pango_bindings.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ `pkg-config --libs pangocairo`

cairo_bindings.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h cairo_bindings.c
	$(CC) `pkg-config --cflags cairo` $(SO_CFLAGS) cairo_bindings.c

cairo_x.o: cairo_x.c
	$(CC) `pkg-config --cflags cairo` $(SO_CFLAGS) cairo_x.c

libcairo.so: cairo_bindings.o cairo_x.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ `pkg-config --libs cairo`

cairo_xlib_bindings.c:
	$(GENCODE) --strip cairo_xlib_ -include cairo/cairo-xlib.h \
-long $(CAIRO_DIR)/cairo-xlib.h -- \
`pkg-config --cflags cairo` -I$(CLANG_DIR)/include > $@

cairo_xlib_bindings.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h cairo_xlib_bindings.c
	$(CC) `pkg-config --cflags cairo` $(SO_CFLAGS) cairo_xlib_bindings.c

libcairoxlib.so: cairo_xlib_bindings.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ `pkg-config --libs cairo`

clean:
	rm -f *.o
	rm -f *.so
	rm -f cairo_bindings.c cairo_xlib_bindings.c pango_bindings.c 