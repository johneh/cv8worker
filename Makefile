TOPDIR := .
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) $(MILL_LIBS) -lm -lstdc++

all: libcv8.a

v8binding.o: v8binding.cc v8binding.h jsdef.h vm.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -DV8BINDIR="\"$(V8_OUTDIR)/\"" -std=c++0x -c v8binding.cc

jsv8.o: v8binding.h jsdef.h jsv8.h jsv8.c
	$(CC) $(CFLAGS) $(INCLUDES) -c jsv8.c

util.o: util.h util.c
	$(CC) $(CFLAGS) $(INCLUDES) -c util.c

libcv8.a: v8binding.o jsv8.o util.o
	ar rcs $@ $^

clean:
	rm -f *.o
	rm -f libcv8.a
