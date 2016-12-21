TOPDIR := ..
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) $(MILL_LIBS_STATIC) -lm -lstdc++

BINS := testjs

ifdef GCTEST
EXTRA_FLAGS += -DGCTEST=1
endif

all:

# $(BINS)

words.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsdef.h words.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(INCLUDES) -c words.c

words: words.o $(TOPDIR)/libcv8.a
	$(CC) -o $@ $^ $(LIBS)

testjs.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsdef.h testjs.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(INCLUDES) -c testjs.c

testjs: testjs.o $(TOPDIR)/libcv8.a
	$(CC) -o $@ $^ $(LIBS)

testgo.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsdef.h testgo.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(INCLUDES) -c testgo.c

testgo: testgo.o $(TOPDIR)/libcv8.a
	$(CC) -o $@ $^ $(LIBS)

SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

CAIRO_DIR := /usr/include/cairo
cairojs.c: $(TOPDIR)/tools/gencode.cc
	cd $(TOPDIR)/tools; make; cd $(TOPDIR)/tests; \
	$(TOPDIR)/tools/gencode --strip cairo_ $(CAIRO_DIR)/cairo.h -- > $@

cairojs.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h cairojs.c
	$(CC) $(SO_CFLAGS) -I$(CAIRO_DIR) cairojs.c

libcairojs.so: cairojs.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ -lcairo


pointjs.c: $(TOPDIR)/tools/gencode.cc
	cd $(TOPDIR)/tools; make; cd $(TOPDIR)/tests; \
	$(TOPDIR)/tools/gencode point.h -- > $@

pointjs.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsv8dlfn.h pointjs.c
	$(CC) $(SO_CFLAGS) pointjs.c

libpointjs.so: pointjs.o
	$(CC) $(SO_LDFLAGS) $^ -o $@

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.png
	rm -f $(BINS)
