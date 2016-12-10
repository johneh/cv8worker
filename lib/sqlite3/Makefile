TOPDIR := ../..
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) $(MILL_LIBS) -lm -lstdc++
SQLITE3_LIBS = -lsqlite3

all: libsql3.so


SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

sqlite3.o: $(TOPDIR)/jsv8dlfn.h sqlite3.c
	$(CC) $(SO_CFLAGS) -I$(MILL_INCLUDES) sqlite3.c

libsql3.so: sqlite3.o
	$(CC) $(SO_LDFLAGS) sqlite3.o -o $@ $(MILL_LIBS) $(SQLITE3_LIBS)

clean:
	rm -f *.o
	rm -f *.so
