#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "jsv8dlfn.h"

// JS callbacks
static int toJsObject;
static int isJsGObject;

static GQuark js_gid_quark;
static v8_val gvalue_to_val(v8_state vm, const GValue *value, gboolean copy_boxed);

static v8_val
do_init(v8_state vm, int argc, v8_val argv[]) {
    Display *display;
    int fd, n = 0;

    toJsObject = V8_TOINT32(argv[0]);
    isJsGObject = V8_TOINT32(argv[1]);

    gtk_init(& n, NULL);

    js_gid_quark = g_quark_from_string("__gobject_id__");
    assert(js_gid_quark > 0);

    display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
    fd = ConnectionNumber(display);
    return V8_INT32(fd);
}

typedef struct {
    GClosure closure;
    gulong id;
    int hcb;
    v8_state vm;
} JsGClosure;


static GValue *_param_values;
static guint _n_param_values;
static v8_val
do_get_param(v8_state vm, int argc, v8_val argv[]) {
    guint i = V8_TOUINT32(argv[0]);
    if (i > 0 && i < _n_param_values) {
        if (i == 1) {
            GType gtype = G_VALUE_TYPE(&_param_values[i]);
            if (G_TYPE_FUNDAMENTAL(gtype) == G_TYPE_BOXED
                    && strcmp(g_type_name(gtype), "GdkEvent") == 0) {
                // Use gtk_get_current_event() .. returned value _need_ not be copied!!
                return V8_NULL;
            }
        }
        return gvalue_to_val(vm, &_param_values[i], TRUE /* copy_boxed */);
    }
    return V8_VOID;
}

static void
js_g_closure_marshal(GClosure *closure,
                    GValue *return_value,
                    guint n_param_values,
                    const GValue *param_values,
                    gpointer invocation_hint,
                    gpointer marshal_data) {

    JsGClosure *jc = (JsGClosure *)closure;
    v8_val a[2];
    a[0] = V8_UINT32(jc->id);
    a[1] = V8_UINT32(n_param_values);
    _n_param_values = n_param_values;
    _param_values = (GValue *) param_values;

    // param 0 is the widget (self)
    GType gtype = G_VALUE_TYPE(&param_values[0]);
    assert(G_TYPE_FUNDAMENTAL(gtype) == G_TYPE_OBJECT);

    v8_val ret = jsv8->call(jc->vm, V8_HANDLE(jc->hcb), V8_GLOBAL, 2, a);


    if (V8_ISERROR(ret)) {
        fprintf(stderr, "GObject (gtk2) callback: %s\n", V8_ERRSTR(ret));
    } else if (return_value) {
        // convert v8_val to a GValue and store in return_value.
        // XXX: always a gboolean?
        if (G_TYPE_BOOLEAN  == G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(return_value))
                && V8_ISNUMBER(ret)) {
            g_value_set_boolean(return_value, (V8_TODOUBLE(ret) != 0.0));
        }
    }
    jsv8->reset(jc->vm, ret);
}

static void
js_g_closure_invalidate(gpointer data, GClosure *closure) {
    /* fprintf(stderr, "jsg_closure_invalidate() ....\n"); */
}

static void
js_g_closure_finalize(gpointer data, GClosure *closure) {
    /* fprintf(stderr, "jsg_closure_finalize() ....\n"); */
}


// N.B.: the invalidate and then the finalize callbacks are called auto-magically
// when the widget (GObject self) is destroyed.
// most events are RUN_LAST meaning one cannot use gtk_signal_connect_after


static JsGClosure *
js_gobject_connect(gpointer obj, gchar *name, gboolean after) {
    guint handlerid, sigid;
    GQuark detail = 0;
    GClosure *closure;

    if (!g_signal_parse_name(name, G_OBJECT_TYPE(obj),
			     &sigid, &detail, TRUE)) {
	    fprintf(stderr, "%s: unknown signal name: %s",
		     g_type_name(G_OBJECT_TYPE(obj)), name);
	    return NULL;
    }

    closure = g_closure_new_simple(sizeof(JsGClosure), NULL);

    g_closure_add_invalidate_notifier(closure, NULL, js_g_closure_invalidate);
    g_closure_add_finalize_notifier(closure, NULL, js_g_closure_finalize);
    g_closure_set_marshal(closure, js_g_closure_marshal);

    handlerid = g_signal_connect_closure_by_id(obj, sigid, detail,
					       closure, after);
    assert(handlerid > 0);
    ((JsGClosure *) closure)->id = handlerid;
    return (JsGClosure *) closure;
}

static v8_val
do_signal_connect(v8_state vm, int argc, v8_val argv[]) {
    guint handlerid = 0;
    JsGClosure *jc = js_gobject_connect(
                V8_TOPTR(argv[0]),  /* GObject */
                V8_TOSTR(argv[1]),  /* signal name */
                !!V8_TOINT32(argv[2])   /* after */
    );
    if (jc) {
        jc->vm = vm;
        jc->hcb = V8_TOINT32(argv[3]);  /* js callback */
        handlerid = jc->id;
    }
    return V8_UINT32(handlerid);
}


//typedef struct {
//  GTypeInstance  g_type_instance;

//  gchar         *name;
//  GParamFlags    flags;
//  GType		 value_type;
//  GType		 owner_type; /* class or interface using(introducing) this property */
//} GParamSpec;

static v8_val
class_props(v8_state vm, GType type) {
    GParamSpec **props;
    guint nprops = 0, i, k;
    char *name;
    v8_val props_list, v1;
    GObjectClass *class;

    class = g_type_class_ref(type);
    props = g_object_class_list_properties(class, & nprops);
    g_type_class_unref(class);
    props_list = jsv8->array(vm, 0);

    for (i = 0, k = 0; i < nprops; i++) {
        if (props[i]->owner_type != type)   // filter own properties
            continue;
        name = (char *) g_param_spec_get_name(props[i]);
        v1 = V8_STR(name, strlen(name));
        jsv8->seti(vm, props_list, k, v1);
        jsv8->reset(vm, v1);
        k++;
    }
    if (props)
        g_free(props);
    return props_list;
}

static v8_val
do_class_props(v8_state vm, int argc, v8_val argv[]) {
    return class_props(vm, V8_TODOUBLE(argv[0]));
}

static v8_val
do_xflush(v8_state vm, int argc, v8_val argv[]) {
    XFlush(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()));
    return V8_VOID;
}

// called when X connection becomes readable
static v8_val
do_pending_events(v8_state vm, int argc, v8_val argv[]) {
#if 0
    GdkEvent *event;
    while ((event = gdk_event_get()) != NULL) {
        gtk_main_do_event(event);
        gdk_event_free(event);
    }
#else
    while (gtk_events_pending())
        gtk_main_iteration();
#endif
    return V8_VOID;
}

static v8_val
do_gobject_ref_count(v8_state vm, int argc, v8_val argv[]) {
    void *ptr = V8_TOPTR(argv[0]);
    // N.B: there is no public api to get the current reference count
    int refcount = G_OBJECT(ptr)->ref_count;
    return V8_INT32(refcount);
}

// Debugging
static v8_val
do_gobject_unref(v8_state vm, int argc, v8_val argv[]) {
    void *ptr = V8_TOPTR(argv[0]);
    /*
    int refcount = G_OBJECT(ptr)->ref_count;
    fprintf(stderr, "ref_count before unref = %d [%s]\n", refcount,
            g_type_name(G_OBJECT_TYPE(G_OBJECT(ptr)))); */
    g_object_unref(ptr);
    return V8_VOID;
}

// check if js val is a GObject
static int
is_js_gobject(v8_state vm, v8_val val) {
    v8_val a[1];
    v8_val retv;
    a[0] = val;
    if (!V8_ISPOINTER(val))
        return 0;
    retv = jsv8->call(vm, V8_HANDLE(isJsGObject), V8_GLOBAL, 1, a);
    assert(V8_ISNUMBER(retv));
    return V8_TOINT32(retv);
}


static v8_val
gtype_to_js(v8_state vm, GType gtype, gpointer vp) {
    /*
     * G_TYPE_FUNDAMENTAL(gtype) == G_TYPE_OBJECT or G_TYPE_BOXED
     */
    v8_val a[3];
    v8_val retv;
    char *gname = (char *) g_type_name(gtype);
    a[0] = V8_STR(gname, strlen(gname));
    a[1] = V8_DOUBLE(gtype); // XXX: gsize (ulong)
    a[2] = V8_PTR(vp);
    retv = jsv8->call(vm, V8_HANDLE(toJsObject), V8_GLOBAL, 3, a);
    jsv8->reset(vm, a[0]);  // free string
    /* g_object_ref_sink(vp); Now in js callback */ 
    return retv;
}


/* Convert GValue to v8_val */
static v8_val
gvalue_to_val(v8_state vm, const GValue *value, gboolean copy_boxed) {
    switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(value))) {
#if 0
    case G_TYPE_INTERFACE:
        ...
#endif

    case G_TYPE_CHAR:
        return V8_INT32((int) g_value_get_schar(value));
    case G_TYPE_UCHAR:
        return V8_INT32((int) g_value_get_uchar(value));
    case G_TYPE_BOOLEAN:
        return V8_INT32((int) g_value_get_boolean(value));
    case G_TYPE_INT:
        return V8_INT32(g_value_get_int(value));
    case G_TYPE_UINT:
        return V8_UINT32((unsigned) g_value_get_uint(value));
    case G_TYPE_LONG:
        return V8_DOUBLE(g_value_get_long(value));
    case G_TYPE_ULONG:
        return V8_DOUBLE(g_value_get_ulong(value));
    case G_TYPE_DOUBLE:
        return V8_DOUBLE(g_value_get_double(value));
    case G_TYPE_INT64:
        return V8_LONG(g_value_get_int64(value));
    case G_TYPE_UINT64:
        return V8_ULONG(g_value_get_uint64(value));
    case G_TYPE_STRING:	{   /* gchar * */
        const gchar *str = g_value_get_string(value);
        if (str)
            return V8_STR(str, strlen(str));
        return V8_NULL; /* V8_PTR(NULL); */
    }
    case G_TYPE_ENUM:
        return V8_INT32(g_value_get_enum(value));
    case G_TYPE_FLAGS: {
        guint val = g_value_get_flags(value);
        return V8_UINT32(val);
    }
    case G_TYPE_FLOAT: {
        double val = g_value_get_float(value);
        return V8_DOUBLE(val);
    }
    case G_TYPE_POINTER:
        return V8_PTR(g_value_get_pointer(value));
    case G_TYPE_BOXED: {
        gpointer ptr = g_value_get_boxed(value);
        if (!ptr)
            return V8_NULL;
        if (copy_boxed)
            ptr = g_boxed_copy(G_VALUE_TYPE(value), ptr);
        return gtype_to_js(vm, G_VALUE_TYPE(value), ptr);
    }
    case G_TYPE_OBJECT: {
        gpointer ptr = g_value_get_object(value);
        if (!ptr)
            return V8_NULL;
        return gtype_to_js(vm, G_VALUE_TYPE(value), ptr);
    }
    default:
        break;
    }
    return V8_NULL; // unsupported type
}

static gboolean
gvalue_from_val(v8_state vm, GValue *value, v8_val val) {
    GType type  = G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(value));
    switch (type) {
    case G_TYPE_STRING: /* gchar * */
        if (!V8_ISSTRING(val))
            return FALSE;
        g_value_set_string(value, V8_TOSTR(val));
        return TRUE;
    case G_TYPE_POINTER:
        if (!V8_ISPOINTER(val))
            return FALSE;
        g_value_set_pointer(value, V8_TOPTR(val));
        return TRUE;
    case G_TYPE_OBJECT: {
        gpointer ptr;
        if (V8_ISNULL(val)) {
            g_value_set_object(value, NULL);
            return TRUE;
        }
        if (!V8_ISPOINTER(val))
            return FALSE;
        ptr = V8_TOPTR(val);
        if (is_js_gobject(vm, val)
                && G_TYPE_CHECK_INSTANCE_TYPE(ptr, G_VALUE_TYPE(value))) {
            g_value_set_object(value, ptr);
            return TRUE;
        }
        return FALSE;
    }
    default:
        break;
    }

    if (!V8_ISNUMBER(val))
        return FALSE;
    double d = V8_TODOUBLE(val);

    switch (type) {
    case G_TYPE_UCHAR:
        g_value_set_uchar(value, d);
        return TRUE;
    case G_TYPE_CHAR:
        g_value_set_schar(value, d);
        return TRUE;
    case G_TYPE_BOOLEAN:
	    g_value_set_boolean(value, !!d);
	    return TRUE;
    case G_TYPE_INT:
        g_value_set_int(value, d);
        return TRUE;
    case G_TYPE_UINT:
        g_value_set_uint(value, d);
        return TRUE;
    case G_TYPE_FLAGS:
        g_value_set_flags(value, d);
        return TRUE;
    case G_TYPE_ENUM:
        g_value_set_enum(value, d);
        return TRUE;
    case G_TYPE_LONG:
        g_value_set_long(value, d);
        return TRUE;
    case G_TYPE_ULONG:
        g_value_set_ulong(value, d);
        return TRUE;
    case G_TYPE_FLOAT:
        g_value_set_float(value, d);
        return TRUE;
    case G_TYPE_DOUBLE:
        g_value_set_double(value, d);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}


static v8_val
do_get_property(v8_state vm, int argc, v8_val argv[]) {
    GObject *widget = V8_TOPTR(argv[0]);
    char *prop_name = V8_TOSTR(argv[1]);
    GValue value = { 0, };
    v8_val retv;

    GParamSpec *param_spec = g_object_class_find_property(
                    G_OBJECT_GET_CLASS(widget), prop_name);
    if (! param_spec)
        return V8_ERR("unknown object property");
    if ( (param_spec->flags & G_PARAM_READABLE) == 0 )
        return V8_ERR("property is not readable");

    g_value_init(& value, G_PARAM_SPEC_VALUE_TYPE(param_spec));
    g_object_get_property(widget, prop_name, & value);
    retv = gvalue_to_val(vm, & value, TRUE);
    g_value_unset(& value);
    return retv;
}

static v8_val
do_set_property(v8_state vm, int argc, v8_val argv[]) {
    GObject *widget = V8_TOPTR(argv[0]);
    char *prop_name = V8_TOSTR(argv[1]);
    GValue value = { 0, };
    GParamSpec *param_spec = g_object_class_find_property(
                        G_OBJECT_GET_CLASS(G_OBJECT(widget)), prop_name);
    if (! param_spec)
        return V8_ERR("unknown object property");

    if (param_spec->flags & G_PARAM_CONSTRUCT_ONLY)
        return V8_ERR("property can only be set in constructor");

    if ( (param_spec->flags & G_PARAM_WRITABLE) == 0)
        return V8_ERR("property is not writable");

    g_value_init(& value, G_PARAM_SPEC_VALUE_TYPE(param_spec));

    if (!gvalue_from_val(vm, & value, argv[2])) {
        g_value_unset(& value);
        return V8_ERR("invalid type for property value");
    }
    g_object_set_property(widget, prop_name, &value);
    g_value_unset(& value);
    return V8_VOID;
}

static v8_val
do_style_get_property(v8_state vm, int argc, v8_val argv[]) {
    GtkWidget *widget = V8_TOPTR(argv[0]);
    char *prop_name = V8_TOSTR(argv[1]);
    GValue value = { 0, };
    v8_val retv;

    GParamSpec *param_spec = gtk_widget_class_find_style_property(
                    GTK_WIDGET_GET_CLASS(widget), prop_name);
    if (! param_spec)
        return V8_ERR("unknown style property");
    if ( (param_spec->flags & G_PARAM_READABLE) == 0 )
        return V8_ERR("style property is not readable");

    g_value_init(& value, G_PARAM_SPEC_VALUE_TYPE(param_spec));
    gtk_widget_style_get_property(widget, prop_name, & value);
    retv = gvalue_to_val(vm, & value, TRUE);
    g_value_unset(& value);
    return retv;
}

// FIXME -- move to gobject.c
static v8_val
do_gobject_type(v8_state vm, int argc, v8_val argv[]) {
    void *object = V8_TOPTR(argv[0]);
    GType gtype = G_OBJECT_TYPE(object);
    // GType -> unsigned long(gsize)
    return V8_DOUBLE(gtype);
}

static v8_val
do_gobject_get_id(v8_state vm, int argc, v8_val argv[]) {
    void *object = V8_TOPTR(argv[0]);
    gpointer data = g_object_get_qdata(object, js_gid_quark);
    guint id = 0;
    if (data) {
        id = GPOINTER_TO_UINT(data);
    }
    return V8_UINT32(id);
}

static v8_val
do_gobject_set_id(v8_state vm, int argc, v8_val argv[]) {
    void *object = V8_TOPTR(argv[0]);
    guint id = V8_TOUINT32(argv[1]);
    if (id == 0) {
        return V8_ERR("gobject_set_id: received invalid id");
    }
    g_object_set_qdata(object, js_gid_quark, GUINT_TO_POINTER(id));
    return V8_VOID;
}


/////////////// Glib type system /////////////////
// g_type_XXX from /usr/include/glib-2.0/gobject/gtype.h
// XXX: should be in a seperate .so

// unsafe -- GType must be a valid type.
static v8_val
do_gtype_name(v8_state vm, int argc, v8_val argv[]) {
    GType gtype = V8_TODOUBLE(argv[0]);
    char *name = (char *) g_type_name(gtype);
    return V8_STR(name, strlen(name));
}

// N.B: returns 0 if no type has been registered yet.
static v8_val
do_gtype_from_name(v8_state vm, int argc, v8_val argv[]) {
    char *name = V8_TOSTR(argv[0]);
    GType gtype = g_type_from_name(name);
    if (gtype > (1UL << 53)) {
        // Gtype is a gsize (ulong).
        fprintf(stderr, "GType does not fit in double exactly\n");
        exit(1);
    }
    return V8_DOUBLE(gtype);
}

/*
 * If is_a_type is a derivable type, check whether type is a descendant
 * of is_a_type . If is_a_type is an interface, check whether type conforms to it.
 */
static v8_val
do_gtype_is_a(v8_state vm, int argc, v8_val argv[]) {
    GType gtype = V8_TODOUBLE(argv[0]);
    GType is_a_type = V8_TODOUBLE(argv[1]);
    gboolean r = g_type_is_a(gtype, is_a_type);
    return V8_INT32(r);
}

// N.B.: GObject only?
static v8_val
do_gtype_instance_size(v8_state vm, int argc, v8_val argv[]) {
    GType gtype = V8_TODOUBLE(argv[0]);
    GTypeQuery q;
    g_type_query(gtype, &q);
    if (q.type > 0)
        return V8_INT32(q.instance_size);
    return V8_VOID;
}

static v8_val
do_g_boxed_free(v8_state vm, int argc, v8_val argv[]) {
    GType gtype = V8_TODOUBLE(argv[0]);
    g_boxed_free(gtype, V8_TOPTR(argv[1]));
    return V8_VOID;
}


static v8_ffn fntab_[] = {
{ 2, do_init, "init"},
{ 2, do_gobject_set_id, "set_id" },
{ 1, do_gobject_get_id, "get_id" },
{ 4, do_signal_connect, "signal_connect"},
{ 1, do_class_props, "class_props" },
{ 2, do_get_property, "get_property" },
{ 3, do_set_property, "set_property" },
{ 2, do_style_get_property, "style_get_property" },
{ 0, do_xflush, "xflush"},
{ 0, do_pending_events, "do_pending_events" },
{ 1, do_get_param, "_get_param" },
{ 1, do_gobject_type, "gobject_type" },
{ 1, do_gtype_name, "g_type_name" },
{ 1, do_gtype_from_name, "g_type_from_name" },
{ 2, do_gtype_is_a, "g_type_is_a" },
{ 1, do_gtype_instance_size, "g_type_instance_size" },
{ 2, do_g_boxed_free, "g_boxed_free" },
{ 1, do_gobject_ref_count, "ref_count" },
{ 1, do_gobject_unref, "gobject_unref" },
{0}
};

int JS_LOAD(v8_state vm, v8_val hobj) {
JS_EXPORT(fntab_);
}
