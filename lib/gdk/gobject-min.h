#include <glib-object.h>

/*
./gencode ../lib/gdk/gobject-light.h -- `pkg-config --cflags gdk-2.0` -I/usr/include/clang/3.8/include
*/

gpointer g_object_ref (gpointer object);
/*
Increases the reference count of object .
*/

void g_object_unref (gpointer object);
/*
Decreases the reference count of object . When its reference count drops to 0, the object is finalized (i.e. its memory is freed).
If the pointer to the GObject may be reused in future (for example, if it is an instance variable of another object), it is recommended to clear the pointer to NULL rather than retain a dangling pointer to a potentially invalid GObject instance. Use g_clear_object() for this.
*/

gpointer g_object_ref_sink (gpointer object);
/*
Increase the reference count of object , and possibly remove the floating reference, if object has a floating reference.
In other words, if the object is floating, then this call "assumes ownership" of the floating reference, converting it to a normal reference by clearing the floating flag while leaving the reference count unchanged. If the object is not floating, then this call adds a new normal reference increasing the reference count by one.
*/

// void g_clear_object (volatile GObject **object_ptr); MACRO
/*
Clears a reference to a GObject.
object_ptr must not be NULL.
If the reference is NULL then this function does nothing. Otherwise, the reference count of the object is decreased and the pointer is set to NULL.
A macro is also included that allows this function to be used without pointer casts.
*/

gboolean g_object_is_floating (gpointer object);
/*
Checks whether object has a floating reference.
*/

