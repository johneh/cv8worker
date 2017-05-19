'use strict';
module.__path = module.__path + ':./libs';

const { GTYPES, register_gtype, gtype_to_js } = require('./gtkb.js');

const gtkb = require('gtkb.so');
const gtkenums = require('gtkenums.so');
const gtkwidget = require('gtkwidget.so');
const gtkstyle = require('gtkstyle.so');

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
    $$persist.set(gtype_to_js),
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


//############## GtkAllocation & GdkRectangle ############

const GBoxed = Object.create(Object.getPrototypeOf($nullptr));
register_gtype('GBoxed', GBoxed);

const GtkAllocation = Object.create(GBoxed);

register_gtype('GtkAllocation', GtkAllocation);
register_gtype('GdkRectangle', GtkAllocation);

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
register_gtype('GdkColor', GdkColor);

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

register_gtype('GtkBorder', GtkBorder);

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

register_gtype('GtkTextIter', GtkTextIter);

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

register_gtype('GObject', GObject, G_TYPE_OBJECT, initGObject); 


//#####################################
// GtkWidget <- GObject
//#####################################

const GtkWidget = Object.create(GObject);

function initGtkWidget(wptr) {
    if (!GtkWidget.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, GtkWidget);
    initGObject(wptr);

    let p = $P.get(wptr);
    p.is_toplevel = gtkwidget.is_toplevel(wptr);
    p.destroyed = false;

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
        if (p.is_toplevel) {
            __done__ = true;    /* TODO: multiple top-level window ? */
           // N.B.: gtk_main_quit() call valid only when using gtk_main()!
        }

        p.destroyed = true;
    });
}

register_gtype('GtkWidget', GtkWidget, gtkwidget.get_type(), initGtkWidget);
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

exports.GtkTextView = require('_gtktextview.js');

const GtkTextMark = Object.create(GObject);
register_gtype('GtkTextMark', GtkTextMark);

exports.GtkTextBuffer = require('_gtktextbuffer.js');
exports.GtkEntryBuffer = require('_gtkentrybuffer.js');
exports.GtkEntryCompletion = require('_gtkentrycompletion.js');
exports.GtkEntry = require('_gtkentry.js');

//for (let p1 of Object.getOwnPropertyNames(GTYPES['GtkEntry'].proto)) $print(p1);

//for (let i in GTYPES) { $print(i); }

