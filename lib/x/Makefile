TOPDIR := ../..
include $(TOPDIR)/config.mk

GENCODE := $(TOPDIR)/tools/gencode
CLANG_DIR := /usr/include/clang/3.8

#INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
#LIBS = $(V8_LIBS) $(MILL_LIBS_STATIC) -lm -lstdc++

all: libxlibjs.so


SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

#CAIRO_DIR := /usr/include/cairo
#cairojs.c: $(TOPDIR)/tools/gencode.cc
#	cd $(TOPDIR)/tools; make; cd $(TOPDIR)/tests; \
#	$(TOPDIR)/tools/gencode --strip cairo_ $(CAIRO_DIR)/cairo.h -- > $@

xlibjs.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h xlibjs.c
	$(CC) $(SO_CFLAGS) xlibjs.c

libxlibjs.so: xlibjs.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ -lX11

xftjs.c:
	$(GENCODE) -long /usr/include/X11/Xft/Xft.h -- \
-I/usr/include/X11 `pkg-config --cflags xft` -I$(CLANG_DIR)/include > $@

xftjs.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h xftjs.c
	$(CC) $(SO_CFLAGS) `pkg-config --cflags xft` xftjs.c

libxftjs.so: xftjs.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ -lX11 -lXft

clean:
	rm -f *.o
	rm -f *.so
