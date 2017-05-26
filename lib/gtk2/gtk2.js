'use strict';
module.__path = module.__path + ':./libs';

const { CTYPES, ctype, ctype_to_js, isa } = require('./gtkb.js');

const gtkb = require('gtkb.so');
const gtkenums = require('gtkenums.so');
const gtkwidget = require('gtkwidget.so');
//const gtkstyle = require('gtkstyle.so');

const $P = new WeakMap();   // private properties
const CALLBACKS = [];
const G_TYPE_OBJECT = gtkb.g_type_from_name('GObject');

let __done__ = false;

// "own" properties
exports.properties = function(gtype_name) {
    if (typeof gtype_name === 'string') {
        const gtype = gtkb.g_type_from_name(gtype_name);
        if (gtype)
            return gtkb.class_props(gtype);
    }
};


// return value is the X connection (file descriptor).
const __xfd__ = gtkb.init(
    $$persist.set(ctype_to_js),
    $$persist.set(function (val) {
            return GObject.isPrototypeOf(val);
    })
);

function dispatcher(sigid, n_params) {
    const c = CALLBACKS[sigid];
    if (c) {
        const obj = c.gobject;
        if (!$P.has(obj))
            return;
        // param at index 0 is the object itself.
        if (n_params <= 1)
            return c.fn.call(obj, ...c.data);
        const params = [];
        for (let i = 1; i < n_params; i++)
            params.push(gtkb._get_param(i));
        return c.fn.call(obj, ...params, ...c.data);
    }
}

function waitXEvent() {
    if (__done__)
        return Promise.resolve(true);
    gtkb.xflush();
    return $co($fdevent, __xfd__)
    .then(() => {
        return false;
    })
    .catch((err) => {
        console.log('Xlib: IO error');
        return true;
    });
};


async function loop() {
    __done__ = false;
    while (true) {
        let done = await waitXEvent();
        if (done)
            break;
        gtkb.do_pending_events();
    }
}

exports.loop = loop;
// N.B.: call XFlush() if updating windows outside loop().
exports.xflush = gtkb.xflush;


function boxedProperty(proto, name, offset, type) {
    Object.defineProperty(proto, name, {
        get: function() {
            return $unpack(this, offset, type)[0];
        },
        set: function(val) {
            $pack(this, offset, 'i', val|0);
        }
    });
}


//############## GdkWindow #################
ctype('GdkWindow', Object.create(Object.getPrototypeOf($nullptr)));
// N.B.:  GDK_WINDOW_XID(win) - Returns the X window belonging to a GdkWindow.

//############## GtkAllocation & GdkRectangle ############

const GBoxed = Object.create(Object.getPrototypeOf($nullptr));
ctype('GBoxed', GBoxed);

const GtkAllocation = Object.create(GBoxed);

ctype('GtkAllocation', GtkAllocation);
ctype('GdkRectangle', GtkAllocation);

function _GtkAllocation() {
    if (this instanceof _GtkAllocation)
        throw new Error('new: operation not permitted');
    const ptr = $malloc(4*4);
    Object.setPrototypeOf(ptr, GtkAllocation);
    return ptr;
}

boxedProperty(GtkAllocation, 'x', 0, 'i');
boxedProperty(GtkAllocation, 'y', 4, 'i');
boxedProperty(GtkAllocation, 'width', 8, 'i');
boxedProperty(GtkAllocation, 'height', 12, 'i');

exports.GtkAllocation = _GtkAllocation;
exports.GdkRectangle = _GtkAllocation;

//############## GdkColor ############

const gdkcolorlib = require('gdkcolor.so', '-dll');
const gdkcolor = gdkcolorlib.identifiers;

const GdkColor = Object.create(GBoxed);
ctype('GdkColor', GdkColor);

function _GdkColor() {
    if (this instanceof _GdkColor)
        throw new Error('new: operation not permitted');
    const ptr = $malloc(gdkcolorlib.sizeOf('_GdkColor'));
    Object.setPrototypeOf(ptr, GdkColor);
    return ptr;
}

//standard names, '#rgb', '#rrggbb', '#rrrgggbbb' or '#rrrrggggbbbb'
_GdkColor.color_parse = function(spec) {
    if (typeof spec === 'string' && spec.length > 0) {
        let color = _GdkColor();
        if (gdkcolor.color_parse(spec, color)) {
            return color;
        }
        throw new Error('Invalid color name');
    }
    throw new Error('non-empty string expected');
};

GdkColor.color_to_string = function() {
    let s = gdkcolor.color_to_string(this);
    if (s === $nullptr)
        return null;
    return $utf8String(s, -1);
};

exports.GdkColor = _GdkColor;

// $print(_GdkColor.color_parse('red').color_to_string()); //-> #ffff00000000

//############## GtkBorder ##############
const GtkBorder = Object.create(GBoxed);

ctype('GtkBorder', GtkBorder);

function _GtkBorder() {
    if (this instanceof _GtkBorder)
        throw new Error('new: operation not permitted');
    const ptr = $malloc(4*4);
    Object.setPrototypeOf(ptr, GtkBorder);
    return ptr;
}

boxedProperty(GtkBorder, 'left', 0, 'i');
boxedProperty(GtkBorder, 'right', 4, 'i');
boxedProperty(GtkBorder, 'top', 8, 'i');
boxedProperty(GtkBorder, 'bottom', 12, 'i');

exports.GtkBorder = _GtkBorder;

//############## GtkTextIter ############

const gtktextiter = require('gtktextiter.so');

const GtkTextIter = Object.create(GBoxed);

ctype('GtkTextIter', GtkTextIter);

function _GtkTextIter() {
    if (this instanceof _GtkTextIter)
        throw new Error('new: operation not permitted');
    const ptr = $malloc(80);    // XXX: lib.sizeOf('_GtkTextIter') === 80
    Object.setPrototypeOf(ptr, GtkTextIter);
    return ptr;
}

require('_gtktextiter.js');

exports.GtkTextIter = _GtkTextIter;

//#####################################
// GObject
//#####################################

const GObject = Object.create(Object.getPrototypeOf($nullptr));

function initGObject(objPtr) {
    if (!$isPointer(objPtr))
        throw new Error("not a pointer");
    if (!GObject.isPrototypeOf(objPtr))
        Object.setPrototypeOf(objPtr, GObject);
    $P.set(objPtr, { handlers: [] });

    gtkb.gobject_ref_sink(objPtr);
    const id = objPtr.gc(gtkb.gobject_unref, true);
    // $print('Weak Id =', id,  gtkb.g_type_name(gtkb.gobject_type(objPtr)));
    // $print(Object.is($findWeak(id), objPtr)),

    gtkb.set_id(objPtr, id);
}

const DISPATCHER = $$persist.set(dispatcher);

GObject.connect = function (name, callback, ...args) {
    if (typeof callback !== 'function')
        throw new TypeError('not a function');
    if (typeof name !== 'string')
        throw new TypeError('not a string');
    const obj = this;
    let id = gtkb.signal_connect(obj, name, false, DISPATCHER);
    if (id === 0)
        throw new Error('unknown signal name');
    CALLBACKS[id] = { gobject: obj, fn: callback, data: args };
    $P.get(obj).handlers.push(id);
    return id;
};

// GType
Object.defineProperty(GObject, 'gtype', {
    get: function() {
        return gtkb.gobject_type(this);
    }
});

ctype('GObject', GObject, G_TYPE_OBJECT, initGObject);


//#####################################
// GtkWidget <- GObject
//#####################################

let N_TOPLEVEL = 0;

function initGtkWidget(wptr) {
    if (!isa(wptr, 'GtkWidget'))
        Object.setPrototypeOf(wptr, CTYPES['GtkWidget'].proto);
    initGObject(wptr);

    let p = $P.get(wptr);
    p.is_toplevel = gtkwidget.is_toplevel(wptr);
    p.destroyed = false;

    if (p.is_toplevel)
        N_TOPLEVEL++;

    // All non top-level widgets have floating references

    // The reference count should be 2 now for a top-level widget.
    // When a (non top-level) widget is added to a container, the reference
    // count will be incremented (2).
    // After the top-level window is closed, the window and all its
    // children are unref-ed (reference count decremented). In the js 'destroy'
    // signal callback, expect the ref. count to be 1 for all widgets.


    wptr.connect('destroy', function() {
        // FIXME: user destroy callbacks will never be called.
        let p = $P.get(this);

        // Expect the reference count to be 1.
        // $print('destroy handler ... widget ref_count =',
        //          gtkb.gobject_ref_count(p.widget));

        let id;
        while ((id = p.handlers.pop())) {
            delete CALLBACKS[id];
        }
        if (p.is_toplevel && --N_TOPLEVEL === 0) {
            __done__ = true;
           // N.B.: gtk_main_quit() call valid only when using gtk_main()!
        }

        p.destroyed = true;
    });
}

const GtkWidget = ctype('GtkWidget', 'GObject', gtkwidget.get_type(), initGtkWidget);
require('_gtkwidget.js');

// Override
GtkWidget.style_get_property = function(property_name) {
    if (typeof property_name !== 'string')
        throw new TypeError('property_name is not a string');
    return gtkb.style_get_property(this,property_name);
};


exports.GtkAdjustment = require('_gtkadjustment.js'); 

require('_gtkmisc.js');
exports.GtkLabel = require('_gtklabel.js');
require('_gtkcontainer.js');
exports.GtkAlignment = require('_gtkalignment.js');

exports.WINDOW_TOPLEVEL = gtkenums.GTK_WINDOW_TOPLEVEL;
exports.WINDOW_POPUP = gtkenums.GTK_WINDOW_POPUP;
exports.WIN_POS_NONE = gtkenums.GTK_WIN_POS_NONE;
exports.WIN_POS_CENTER = gtkenums.GTK_WIN_POS_CENTER;
exports.WIN_POS_MOUSE = gtkenums.GTK_WIN_POS_MOUSE;
exports.WIN_POS_CENTER_ALWAYS = gtkenums.GTK_WIN_POS_CENTER_ALWAYS;
exports.WIN_POS_CENTER_ON_PARENT = gtkenums.GTK_WIN_POS_CENTER_ON_PARENT;
exports.GtkWindow = require('_gtkwindow.js');

exports.GtkButton = require('_gtkbutton.js');

exports.PACK_START = gtkenums.GTK_PACK_START;
exports.PACK_END = gtkenums.GTK_PACK_END;
require('_gtkbox.js');

exports.GtkHBox = require('_gtkhbox.js');
exports.GtkVBox = require('_gtkvbox.js');

exports.POLICY_ALWAYS = gtkenums.GTK_POLICY_ALWAYS;
exports.POLICY_AUTOMATIC = gtkenums.GTK_POLICY_AUTOMATIC;
exports.POLICY_NEVER = gtkenums.GTK_POLICY_NEVER;
exports.GtkScrolledWindow = require('_gtkscrolledwindow.js');
exports.GtkDrawingArea = require('_gtkdrawingarea.js');
exports.GtkDialog = require('_gtkdialog.js');
exports.STOCK = require('./gtkstockid.js');

require('_gtkmenushell.js');
exports.GtkMenuItem = require('_gtkmenuitem.js');
exports.GtkMenu = require('_gtkmenu.js');

exports.GtkEventBox = require('_gtkeventbox.js');

exports.GtkTextView = require('_gtktextview.js');

const GtkTextMark = Object.create(GObject);
ctype('GtkTextMark', GtkTextMark);

exports.GtkTextBuffer = require('_gtktextbuffer.js');
exports.GtkEntryBuffer = require('_gtkentrybuffer.js');
exports.GtkEntryCompletion = require('_gtkentrycompletion.js');

require('_gtkeditable.js'); // GtkInterface
exports.GtkEntry = require('_gtkentry.js');

// merge(mixin) GtkEditable interface properties
Object.assign(CTYPES['GtkEntry'].proto, CTYPES['GtkEditable'].proto);

//for (let p1 of Object.getOwnPropertyNames(CTYPES['GtkEntry'].proto)) $print(p1);
//for (let i in CTYPES) { $print(i); }


/************ gdkevents **********/
const gdkevents = require('gdkevents.so', '-dll');
const _g = gdkevents.identifiers;

let GDK = {};
for(let s in _g) {
    if (s.startsWith('GDK_'))
        GDK[s.substr(4)] = _g[s];
}

function evStruct(type) {
    switch(type) {
    case _g.GDK_NOTHING: break;
    case _g.GDK_DELETE: break;
    case _g.GDK_DESTROY: break;
    case _g.GDK_EXPOSE:    return  'GdkEventExpose';
    case _g.GDK_MOTION_NOTIFY: return 'GdkEventMotion';
    case _g.GDK_BUTTON_PRESS:      /* GdkEventButton */
    case _g.GDK_2BUTTON_PRESS:     /* GdkEventButton */
    case _g.GDK_3BUTTON_PRESS:     /* GdkEventButton */
    case _g.GDK_BUTTON_RELEASE:    return 'GdkEventButton';
    case _g.GDK_KEY_PRESS:         /* GdkEventKey */
    case _g.GDK_KEY_RELEASE:   return 'GdkEventKey';
    case _g.GDK_ENTER_NOTIFY:  /* GdkEventCrossing */
    case _g.GDK_LEAVE_NOTIFY:  return 'GdkEventCrossing';
    case _g.GDK_FOCUS_CHANGE:  return 'GdkEventFocus';
    case _g.GDK_CONFIGURE: return 'GdkEventConfigure';
    case _g.GDK_MAP: break;
    case _g.GDK_UNMAP: break;
    case _g.GDK_PROPERTY_NOTIFY:   return 'GdkEventProperty';
    case _g.GDK_SELECTION_CLEAR:   /* GdkEventSelection */
    case _g.GDK_SELECTION_REQUEST: /* GdkEventSelection */
    case _g.GDK_SELECTION_NOTIFY:  return 'GdkEventSelection';
    case _g.GDK_PROXIMITY_IN:      /* GdkEventProximity */
    case _g.GDK_PROXIMITY_OUT: return 'GdkEventProximity';
    case _g.GDK_DRAG_ENTER:        /* GdkEventDND */
    case _g.GDK_DRAG_LEAVE:        /* GdkEventDND */
    case _g.GDK_DRAG_MOTION:       /* GdkEventDND */
    case _g.GDK_DRAG_STATUS:       /* GdkEventDND */
    case _g.GDK_DROP_START:        /* GdkEventDND */
    case _g.GDK_DROP_FINISHED: return 'GdkEventDND';
    case _g.GDK_CLIENT_EVENT:  return 'GdkEventClient';
    case _g.GDK_VISIBILITY_NOTIFY: return 'GdkEventVisibility';
    case _g.GDK_NO_EXPOSE: return 'GdkEventNoExpose';
    case _g.GDK_SCROLL:    return 'GdkEventScroll';
    case _g.GDK_WINDOW_STATE:  return 'GdkEventWindowState';
    case _g.GDK_SETTING:   return 'GdkEventSetting';
    case _g.GDK_OWNER_CHANGE:
    case _g.GDK_GRAB_BROKEN:
    case _g.GDK_DAMAGE:
        break;
    default:
        return null;
    }
    return 'GdkEventAny';
}

exports.GDK = GDK;
exports.get_current_event = function() {
    const ev = gtkb.get_current_event();
    if (ev === $nullptr)
        return null;
    ev.gc(gtkb.gdk_event_free);
    return new Proxy(ev, {
        get: function(target, name) {
            const type = $unpack(target, 0, 'i')[0];
            if (name === 'type') {
                return type;
            }
            const evs = evStruct(type);
            if (evs === null)
                throw new Error('invalid event type');
            const fieldType = gdkevents.typeOf(evs, name);
            if (fieldType !== null) {
                const fieldOffset = gdkevents.offsetOf(evs, name);
                // TODO: GdkWindow .. convert pointers to js type using ctype_to_js.
                return $unpack(target, fieldOffset, fieldType)[0];
            }
        }
    });
};


// Example: GtkDialog
/*
function quick_message(message, parent) {
    let dialog = gtk.GtkDialog();
    dialog.set_default_size(200, 100);
    dialog.set_title("Message");
    dialog.set_transient_for(parent);
    dialog.set_destroy_with_parent(true);
    dialog.add_button(gtk.STOCK.OK, -1);    // GTK_STOCK_OK, GTK_RESPONSE_NONE
    dialog.add_button(gtk.STOCK.CANCEL, -6);    // GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL

    let content_area = dialog.get_content_area();
    let label = gtk.GtkLabel(message);
    dialog.connect("response", function(response_id) {
        $print('response_id =', response_id),
        this.destroy();
    });
    content_area.add(label);
    dialog.show_all();
}
*/

