TOPDIR := ../..
include $(TOPDIR)/config.mk

GENCODE := $(TOPDIR)/tools/gencode
CLANG_DIR := /usr/include/clang/3.8

#INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
#LIBS = $(V8_LIBS) $(MILL_LIBS_STATIC) -lm -lstdc++

all: libxlib.so

SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

xlib.c:
	$(GENCODE) -long -nowrap XEHeadOfExtensionList:XQueryKeymap \
-include X11/Xlib.h:X11/Xutil.h \
/usr/include/X11/Xlib.h xlibextra.h -- \
-I$(CLANG_DIR)/include > $@

xlib.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h xlib.c
	$(CC) $(SO_CFLAGS) xlib.c

libxlib.so: xlib.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ -lX11

clean:
	rm -f *.o
	rm -f *.so
