TOPDIR := ..
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) $(MILL_LIBS) -lm -lstdc++

BINS := testjs

all: $(BINS)

testjs.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsdef.h testjs.c
	$(CC) $(CFLAGS) $(INCLUDES) -c testjs.c

testjs: testjs.o $(TOPDIR)/libcv8.a
	$(CC) -o $@ $^ $(LIBS)

SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

math.o: $(TOPDIR)/jsv8.h math.c
	$(CC) $(SO_CFLAGS) math.c

libmath.so: math.o
	$(CC) $(SO_LDFLAGS) math.o -o $@

clean:
	rm -f *.o
	rm -f $(BINS)
