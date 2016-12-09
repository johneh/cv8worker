TOPDIR := ../..
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) $(MILL_LIBS) -lm -lstdc++

all: libfs.so


SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

fs.o: $(TOPDIR)/jsv8dlfn.h fs.c
	$(CC) $(SO_CFLAGS) -I$(MILL_INCLUDES) fs.c

libfs.so: fs.o
	$(CC) $(SO_LDFLAGS) fs.o -o $@ $(MILL_LIBS)

clean:
	rm -f *.o
	rm -f *.so
