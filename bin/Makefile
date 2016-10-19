TOPDIR := ..
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) -lm -lstdc++ $(MILL_LIBS)

BINS := jsi

all: $(BINS)

jsi.o: $(TOPDIR)/jsv8.h $(TOPDIR)/jsdef.h jsi.c
	$(CC) $(CFLAGS) $(INCLUDES) -c jsi.c

jsi: jsi.o $(TOPDIR)/libcv8.a
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f *.o
	rm -f $(BINS)
