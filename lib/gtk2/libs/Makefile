TOPDIR := ../../..
include $(TOPDIR)/config.mk

GENCODE := $(TOPDIR)/tools/gencode
CLANG_DIR := /usr/include/clang/3.8
JSI = ../../../tests/jsi.sh
CG=../codegen.js

#INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
#LIBS = $(V8_LIBS) $(MILL_LIBS_STATIC) -lm -lstdc++

all: libs js

libs: gtkwidget.so gtkwindow.so \
gtkcontainer.so gtkfixed.so gtklabel.so gtkbutton.so gtkhscale.so \
gtkmisc.so gtkbox.so gtkhbox.so gtkvbox.so gtkframe.so \
gtkentry.so gtkentrybuffer.so gtkentrycompletion.so \
gtktextbuffer.so gtktextview.so gtktextiter.so \
gtktexttag.so gtktexttagtable.so \
gtkscrolledwindow.so gtkstyle.so gtkenums.so gtkb.so \
gdkcolor.so

X=.
js: $(X)/_gtkwidget.js $(X)/_gtkwindow.js \
$(X)/_gtkcontainer.js $(X)/_gtkfixed.js $(X)/_gtklabel.js \
$(X)/_gtkbutton.js $(X)/_gtkhscale.js $(X)/_gtkmisc.js \
$(X)/_gtkbox.js $(X)/_gtkhbox.js $(X)/_gtkvbox.js $(X)/_gtkframe.js \
$(X)/_gtkentry.js $(X)/_gtkentrybuffer.js $(X)/_gtkentrycompletion.js \
$(X)/_gtktextbuffer.js $(X)/_gtktextview.js $(X)/_gtktextiter.js \
$(X)/_gtkscrolledwindow.js


$(X)/_gtkwidget.js: gtkwidget.so
	$(JSI) -f $(CG) $^ GtkWidget > $@
$(X)/_gtkwindow.js: gtkwindow.so
	$(JSI) -f $(CG) $^ GtkWindow > $@
$(X)/_gtkcontainer.js: gtkcontainer.so
	$(JSI) -f $(CG) $^ GtkContainer > $@
$(X)/_gtkfixed.js: gtkfixed.so
	$(JSI) -f $(CG) $^ GtkFixed > $@
$(X)/_gtklabel.js: gtklabel.so
	$(JSI) -f $(CG) $^ GtkLabel > $@
$(X)/_gtkbutton.js: gtkbutton.so
	$(JSI) -f $(CG) $^ GtkButton > $@
$(X)/_gtkhscale.js: gtkhscale.so
	$(JSI) -f $(CG) $^ GtkHScale > $@
$(X)/_gtkmisc.js: gtkmisc.so
	$(JSI) -f $(CG) $^ GtkMisc > $@
$(X)/_gtkbox.js: gtkbox.so
	$(JSI) -f $(CG) $^ GtkBox > $@
$(X)/_gtkhbox.js: gtkhbox.so
	$(JSI) -f $(CG) $^ GtkHBox > $@
$(X)/_gtkvbox.js: gtkvbox.so
	$(JSI) -f $(CG) $^ GtkVBox > $@
$(X)/_gtkframe.js: gtkframe.so
	$(JSI) -f $(CG) $^ GtkFrame > $@
$(X)/_gtkentry.js: gtkentry.so
	$(JSI) -f $(CG) $^ GtkEntry > $@
$(X)/_gtkentrybuffer.js: gtkentrybuffer.so
	$(JSI) -f $(CG) $^ GtkEntryBuffer > $@
$(X)/_gtkentrycompletion.js: gtkentrycompletion.so
	$(JSI) -f $(CG) $^ GtkEntryCompletion > $@
$(X)/_gtktextbuffer.js: gtktextbuffer.so
	$(JSI) -f $(CG) $^ GtkTextBuffer > $@
$(X)/_gtktextview.js: gtktextview.so
	$(JSI) -f $(CG) $^ GtkTextView > $@
$(X)/_gtktextiter.js: gtktextiter.so
	$(JSI) -f $(CG) $^ GtkTextIter > $@
$(X)/_gtkscrolledwindow.js: gtkscrolledwindow.so
	$(JSI) -f $(CG) $^ GtkScrolledWindow > $@


SO_CFLAGS = -fPIC -Wall -c -g -I. -I$(TOPDIR)
SO_LDFLAGS = -shared

GTK_CFLAGS := $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS := $(shell pkg-config --libs gtk+-2.0)

gtkb.o: ../gtkb.c
	$(CC) -c $(GTK_CFLAGS) $(SO_CFLAGS) -I$(MILL_INCLUDES) -o $@ ../gtkb.c

.c.o:
	$(CC) -c $(GTK_CFLAGS) $(SO_CFLAGS) -I$(MILL_INCLUDES) -o $@ $<

#gtkmain.c:
#	$(GENCODE) --strip gtk_ -include gtk/gtkmain.h \
#/usr/include/gtk-2.0/gtk/gtkmain.h -- \
#$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkwidget.c:
	$(GENCODE) --strip gtk_widget_ -include gtk/gtkwidget.h \
-nowrap gtk_widget_style_get_valist \
/usr/include/gtk-2.0/gtk/gtkwidget.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkwindow.c:
	$(GENCODE) --strip gtk_window_ -include gtk/gtkwindow.h \
/usr/include/gtk-2.0/gtk/gtkwindow.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkcontainer.c:
	$(GENCODE) --strip gtk_container_ -include gtk/gtkcontainer.h \
-nowrap gtk_container_child_set_valist:gtk_container_child_get_valist \
/usr/include/gtk-2.0/gtk/gtkcontainer.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkfixed.c:
	$(GENCODE) --strip gtk_fixed_ -include gtk/gtkfixed.h \
/usr/include/gtk-2.0/gtk/gtkfixed.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtklabel.c:
	$(GENCODE) --strip gtk_label_ -include gtk/gtklabel.h \
/usr/include/gtk-2.0/gtk/gtklabel.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkbutton.c:
	$(GENCODE) --strip gtk_button_ -include gtk/gtkbutton.h \
/usr/include/gtk-2.0/gtk/gtkbutton.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkentry.c:
	$(GENCODE) --strip gtk_entry_ -include gtk/gtkentry.h \
/usr/include/gtk-2.0/gtk/gtkentry.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkentrybuffer.c:
	$(GENCODE) --strip gtk_entry_buffer_ -include gtk/gtkentrybuffer.h \
/usr/include/gtk-2.0/gtk/gtkentrybuffer.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkentrycompletion.c:
	$(GENCODE) --strip gtk_entry_completion_ -include gtk/gtkentrycompletion.h \
/usr/include/gtk-2.0/gtk/gtkentrycompletion.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkframe.c:
	$(GENCODE) --strip gtk_frame_ -include gtk/gtkframe.h \
/usr/include/gtk-2.0/gtk/gtkframe.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtktextiter.c:
	$(GENCODE) --strip gtk_text_iter_ -include gtk/gtktextiter.h \
/usr/include/gtk-2.0/gtk/gtktextiter.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

#gtktextmark.c:
#	$(GENCODE) --strip gtk_text_mark_ -include gtk/gtktextmark.h \
#/usr/include/gtk-2.0/gtk/gtktextmark.h -- \
#$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtktexttag.c:
	$(GENCODE) --strip gtk_text_tag_ -include gtk/gtktexttag.h \
/usr/include/gtk-2.0/gtk/gtktexttag.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtktexttagtable.c:
	$(GENCODE) --strip gtk_text_tag_table_ -include gtk/gtktexttagtable.h \
/usr/include/gtk-2.0/gtk/gtktexttagtable.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtktextbuffer.c:
	$(GENCODE) --strip gtk_text_buffer_ -include gtk/gtktextbuffer.h \
/usr/include/gtk-2.0/gtk/gtktextbuffer.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtktextview.c:
	$(GENCODE) --strip gtk_text_view_ -include gtk/gtktextview.h \
/usr/include/gtk-2.0/gtk/gtktextview.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkbox.c:
	$(GENCODE) --strip gtk_box_ -include gtk/gtkbox.h \
/usr/include/gtk-2.0/gtk/gtkbox.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkhbox.c:
	$(GENCODE) --strip gtk_hbox_ -include gtk/gtkhbox.h \
/usr/include/gtk-2.0/gtk/gtkhbox.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkvbox.c:
	$(GENCODE) --strip gtk_vbox_ -include gtk/gtkvbox.h \
/usr/include/gtk-2.0/gtk/gtkvbox.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkmisc.c:
	$(GENCODE) --strip gtk_misc_ -include gtk/gtkmisc.h \
/usr/include/gtk-2.0/gtk/gtkmisc.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkhscale.c:
	$(GENCODE) --strip gtk_hscale_ -include gtk/gtkhscale.h \
/usr/include/gtk-2.0/gtk/gtkhscale.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkscrolledwindow.c:
	$(GENCODE) --strip gtk_scrolled_window_ -include gtk/gtkscrolledwindow.h \
/usr/include/gtk-2.0/gtk/gtkscrolledwindow.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

#gtkliststore.c:
#	$(GENCODE) --strip gtk_list_store_ -include gtk/gtkliststore.h \
#-nowrap gtk_list_store_set_valist /usr/include/gtk-2.0/gtk/gtkliststore.h -- \
#$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@


# have gtk_style_ and gtk_paint_; strip gtk_ only
gtkstyle.c:
	$(GENCODE) --strip gtk_ -include gtk/gtkstyle.h \
-nowrap gtk_style_get_valist /usr/include/gtk-2.0/gtk/gtkstyle.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gdkcolor.c:
	$(GENCODE) --strip gdk_ -include gdk/gdk.h \
/usr/include/gtk-2.0/gdk/gdkcolor.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@


gtkenums.c:
	$(GENCODE) --strip gtk_ -include gtk/gtkenums.h \
/usr/include/gtk-2.0/gtk/gtkenums.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

%.so: %.o
	$(CC) $(SO_LDFLAGS) $^ -o $@ $(GTK_LIBS)

clean:
	rm -f *.o
	rm -f *.so
