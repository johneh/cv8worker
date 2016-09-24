TOPDIR := ..
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) $(MILL_LIBS) -lm -lstdc++

BINS := jsi

all: $(BINS)

jsi.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsdef.h jsi.c
	$(CC) $(CFLAGS) $(INCLUDES) -c jsi.c

jsi: jsi.o $(TOPDIR)/libcv8.a
	$(CC) -o $@ $^ -rdynamic $(LIBS)

clean:
	rm -f *.o
	rm -f $(BINS)
