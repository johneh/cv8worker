TOPDIR := ../../..
include $(TOPDIR)/config.mk

GENCODE := $(TOPDIR)/tools/gencode
CLANG_DIR := /usr/include/clang/3.8
JSI = ../../../tests/jsi.sh
CG=../codegen.js

#INCLUDES = -I. -I$(TOPDIR) -I$(MILL_INCLUDES) $(V8_INCLUDES)
#LIBS = $(V8_LIBS) $(MILL_LIBS_STATIC) -lm -lstdc++

all: libs js

libs: gtkwidget.so gtkwindow.so gtkcontainer.so gtkalignment.so \
gtkfixed.so gtklabel.so gtkbutton.so gtkhscale.so \
gtkmisc.so gtkbox.so gtkhbox.so gtkvbox.so gtkframe.so \
gtkentry.so gtkentrybuffer.so gtkentrycompletion.so gtkeditable.so \
gtktextbuffer.so gtktextview.so gtktextiter.so \
gtktexttag.so gtktexttagtable.so gtkadjustment.so \
gtkscrolledwindow.so gtkstyle.so gtkenums.so gtkb.so \
gtkdrawingarea.so gtkdialog.so gtkeventbox.so \
gtkmenushell.so gtkmenu.so gtkmenuitem.so gtkmenubar.so \
gtkstatusbar.so gtktoolshell.so gtktoolitem.so gtktoolbutton.so gtktoolbar.so \
gdkevents.so gdkcolor.so

X=.
js: $(X)/_gtkwidget.js $(X)/_gtkwindow.js \
$(X)/_gtkcontainer.js $(X)/_gtkalignment.js $(X)/_gtkfixed.js $(X)/_gtklabel.js \
$(X)/_gtkbutton.js $(X)/_gtkhscale.js $(X)/_gtkmisc.js \
$(X)/_gtkbox.js $(X)/_gtkhbox.js $(X)/_gtkvbox.js $(X)/_gtkframe.js \
$(X)/_gtkentry.js $(X)/_gtkentrybuffer.js $(X)/_gtkentrycompletion.js $(X)/_gtkeditable.js \
$(X)/_gtktextbuffer.js $(X)/_gtktextview.js $(X)/_gtktextiter.js \
$(X)/_gtkscrolledwindow.js $(X)/_gtkadjustment.js $(X)/_gtkdrawingarea.js \
$(X)/_gtkdialog.js $(X)/_gtkeventbox.js \
$(X)/_gtkmenushell.js $(X)/_gtkmenu.js $(X)/_gtkmenuitem.js $(X)/_gtkmenubar.js \
$(X)/_gtkstatusbar.js $(X)/_gtktoolshell.js $(X)/_gtktoolitem.js $(X)/_gtktoolbutton.js \
$(X)/_gtktoolbar.js


$(X)/_gtkwidget.js: gtkwidget.so
	$(JSI) -f $(CG) $^ GtkWidget > $@
$(X)/_gtkwindow.js: gtkwindow.so
	$(JSI) -f $(CG) $^ GtkWindow > $@
$(X)/_gtkcontainer.js: gtkcontainer.so
	$(JSI) -f $(CG) $^ GtkContainer > $@
$(X)/_gtkalignment.js: gtkalignment.so
	$(JSI) -f $(CG) $^ GtkAlignment > $@
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
$(X)/_gtkeditable.js: gtkeditable.so
	$(JSI) -f $(CG) $^ GtkEditable > $@
$(X)/_gtktextbuffer.js: gtktextbuffer.so
	$(JSI) -f $(CG) $^ GtkTextBuffer > $@
$(X)/_gtktextview.js: gtktextview.so
	$(JSI) -f $(CG) $^ GtkTextView > $@
$(X)/_gtktextiter.js: gtktextiter.so
	$(JSI) -f $(CG) $^ GtkTextIter > $@
$(X)/_gtkscrolledwindow.js: gtkscrolledwindow.so
	$(JSI) -f $(CG) $^ GtkScrolledWindow > $@
$(X)/_gtkadjustment.js: gtkadjustment.so
	$(JSI) -f $(CG) $^ GtkAdjustment > $@
$(X)/_gtkdrawingarea.js: gtkdrawingarea.so
	$(JSI) -f $(CG) $^ GtkDrawingArea > $@
$(X)/_gtkdialog.js: gtkdialog.so
	$(JSI) -f $(CG) $^ GtkDialog > $@
$(X)/_gtkmenushell.js: gtkmenushell.so
	$(JSI) -f $(CG) $^ GtkMenuShell > $@
$(X)/_gtkmenu.js: gtkmenu.so
	$(JSI) -f $(CG) $^ GtkMenu > $@
$(X)/_gtkmenuitem.js: gtkmenuitem.so
	$(JSI) -f $(CG) $^ GtkMenuItem > $@
$(X)/_gtkmenubar.js: gtkmenubar.so
	$(JSI) -f $(CG) $^ GtkMenuBar > $@
$(X)/_gtkeventbox.js: gtkeventbox.so
	$(JSI) -f $(CG) $^ GtkEventBox > $@
$(X)/_gtkstatusbar.js: gtkstatusbar.so
	$(JSI) -f $(CG) $^ GtkStatusbar > $@
$(X)/_gtktoolshell.js: gtktoolshell.so
	$(JSI) -f $(CG) $^ GtkToolShell > $@
$(X)/_gtktoolitem.js: gtktoolitem.so
	$(JSI) -f $(CG) $^ GtkToolItem > $@
$(X)/_gtktoolbutton.js: gtktoolbutton.so
	$(JSI) -f $(CG) $^ GtkToolButton > $@
$(X)/_gtktoolbar.js: gtktoolbar.so
	$(JSI) -f $(CG) $^ GtkToolbar > $@


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

gtkalignment.c:
	$(GENCODE) --strip gtk_alignment_ -include gtk/gtkalignment.h \
/usr/include/gtk-2.0/gtk/gtkalignment.h -- \
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

gtkeditable.c:
	$(GENCODE) --strip gtk_editable_ -include gtk/gtkeditable.h \
/usr/include/gtk-2.0/gtk/gtkeditable.h -- \
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

gtkdrawingarea.c:
	$(GENCODE) --strip gtk_drawing_area_ -include gtk/gtkdrawingarea.h \
/usr/include/gtk-2.0/gtk/gtkdrawingarea.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkdialog.c:
	$(GENCODE) --strip gtk_dialog_ -include gtk/gtkdialog.h \
/usr/include/gtk-2.0/gtk/gtkdialog.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkmenushell.c:
	$(GENCODE) --strip gtk_menu_shell_ -include gtk/gtkmenushell.h \
/usr/include/gtk-2.0/gtk/gtkmenushell.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkmenu.c:
	$(GENCODE) --strip gtk_menu_ -include gtk/gtkmenu.h \
/usr/include/gtk-2.0/gtk/gtkmenu.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkmenuitem.c:
	$(GENCODE) --strip gtk_menu_item_ -include gtk/gtkmenuitem.h \
/usr/include/gtk-2.0/gtk/gtkmenuitem.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkmenubar.c:
	$(GENCODE) --strip gtk_menu_bar_ -include gtk/gtkmenubar.h \
/usr/include/gtk-2.0/gtk/gtkmenubar.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkeventbox.c:
	$(GENCODE) --strip gtk_event_box_ -include gtk/gtkeventbox.h \
/usr/include/gtk-2.0/gtk/gtkeventbox.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkstatusbar.c:
	$(GENCODE) --strip gtk_statusbar_ -include gtk/gtkstatusbar.h \
/usr/include/gtk-2.0/gtk/gtkstatusbar.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtktoolshell.c:
	$(GENCODE) --strip gtk_tool_shell_ -include gtk/gtk.h \
/usr/include/gtk-2.0/gtk/gtktoolshell.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include -D__GTK_H_INSIDE__=1 -DGTK_COMPILATION=1 > $@

gtktoolitem.c:
	$(GENCODE) --strip gtk_tool_item_ -include gtk/gtktoolitem.h \
/usr/include/gtk-2.0/gtk/gtktoolitem.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include  > $@

gtktoolbutton.c:
	$(GENCODE) --strip gtk_tool_button_ -include gtk/gtktoolbutton.h \
/usr/include/gtk-2.0/gtk/gtktoolbutton.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include  > $@

gtktoolbar.c:
	$(GENCODE) --strip gtk_toolbar_ -include gtk/gtktoolbar.h \
/usr/include/gtk-2.0/gtk/gtktoolbar.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include  > $@

#gtkliststore.c:
#	$(GENCODE) --strip gtk_list_store_ -include gtk/gtkliststore.h \
#-nowrap gtk_list_store_set_valist /usr/include/gtk-2.0/gtk/gtkliststore.h -- \
#$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gtkadjustment.c:
	$(GENCODE) --strip gtk_adjustment_ -include gtk/gtkadjustment.h \
/usr/include/gtk-2.0/gtk/gtkadjustment.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

# have gtk_style_ and gtk_paint_; strip gtk_ only
gtkstyle.c:
	$(GENCODE) --strip gtk_ -include gtk/gtkstyle.h \
-nowrap gtk_style_get_valist /usr/include/gtk-2.0/gtk/gtkstyle.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gdkcolor.c:
	$(GENCODE) --strip gdk_ -include gdk/gdk.h \
/usr/include/gtk-2.0/gdk/gdkcolor.h -- \
$(GTK_CFLAGS) -I$(CLANG_DIR)/include > $@

gdkevents.c:
	$(GENCODE) --strip gdk_event_ -include gdk/gdk.h \
/usr/include/gtk-2.0/gdk/gdkevents.h -- \
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
