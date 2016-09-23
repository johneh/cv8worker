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

clean:
	rm -f *.o
	rm -f $(BINS)
