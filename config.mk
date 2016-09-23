# V8 compiled using 'make i18nsupport=off x64.release'.
# With i18nsupport on, append third_part/icu/libicu{uc,i18n,data}.a
# to V8_LIBS

CC = gcc
CPP = g++
CFLAGS = -g -O2 -Wall
CPPFLAGS = -g -O2 -Wall
#MILL_LIBS = -L$(HOME)/opt/lib -lmill
MILL_DIR = $(HOME)/t/libmill_worker
MILL_LIBS = $(MILL_DIR)/lib/libmill.a
MILL_INCLUDES = $(MILL_DIR)/include

V8_DIR = $(HOME)/src/v8
V8_TARGET = x64.release
V8_INCLUDES = -I$(V8_DIR) -I$(V8_DIR)/include
V8_OUTDIR = $(V8_DIR)/out/$(V8_TARGET)
V8_LIBDIR = $(V8_OUTDIR)/obj.target/src
V8_LIBS = -Wl,--start-group $(V8_LIBDIR)/libv8_base.a \
$(V8_LIBDIR)/libv8_libbase.a $(V8_LIBDIR)/libv8_external_snapshot.a \
$(V8_LIBDIR)/libv8_libplatform.a -Wl,--end-group -lrt -ldl -pthread
