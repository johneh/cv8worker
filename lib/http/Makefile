TOPDIR := ../..
include $(TOPDIR)/config.mk

INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
LIBS = $(V8_LIBS) $(MILL_LIBS) -lm -lstdc++
SSL_LIBS = -lssl -lcrypto

all: libhttp.so


SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

http.o: $(TOPDIR)/jsv8dlfn.h mssl.h http.c
	$(CC) $(SO_CFLAGS) -I$(MILL_INCLUDES) http.c

mssl.o: mssl.h mssl.c
	$(CC) $(SO_CFLAGS) -I$(MILL_INCLUDES) mssl.c

libhttp.so: http.o mssl.o
	$(CC) $(SO_LDFLAGS) http.o mssl.o -o $@ $(MILL_LIBS) $(SSL_LIBS)

clean:
	rm -f *.o
	rm -f *.so
