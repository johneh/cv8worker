'use strict';
module.__path = module.__path + ':./libs';

const { GTYPES, register_gtype, gtype_to_js } = require('./gtkb.js');

const gtkb = require('gtkb.so');
const gtkenums = require('gtkenums.so');
const gtkwidget = require('gtkwidget.so');
const gtkwindow = require('gtkwindow.so');
const gtkcontainer = require('gtkcontainer.so');
const gtkmisc = require('gtkmisc.so');
const gtkbutton = require('gtkbutton.so');
const gtklabel = require('gtklabel.so');
const gtkbox = require('gtkbox.so');
const gtkhbox = require('gtkhbox.so');
const gtkvbox = require('gtkvbox.so');
const gtkscrolledwindow = require('gtkscrolledwindow.so');
const gtktextbuffer = require('gtktextbuffer.so');
const gtktextview = require('gtktextview.so');
const gtkentrybuffer = require('gtkentrybuffer.so');
const gtkentrycompletion = require('gtkentrycompletion.so');
const gtkentry = require('gtkentry.so');

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
            return $GObject.isPrototypeOf(val);
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


//===================================================================//

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

const $GBoxed = Object.create(Object.getPrototypeOf($nullptr));
register_gtype('GBoxed', $GBoxed);

const $GtkAllocation = Object.create($GBoxed);

register_gtype('GtkAllocation', $GtkAllocation);
register_gtype('GdkRectangle', $GtkAllocation);

const GtkAllocation = function() {
    if (this instanceof GtkAllocation)
        throw new Error('operation not permitted');
    const ptr = $malloc(4*4);
    ptr.gc(); // XXX: _not_ using g_boxed_free
    Object.setPrototypeOf(ptr, $GtkAllocation);
    return ptr;
};

boxedProperty($GtkAllocation, 'x', 0, 'i');
boxedProperty($GtkAllocation, 'y', 4, 'i');
boxedProperty($GtkAllocation, 'width', 8, 'i');
boxedProperty($GtkAllocation, 'height', 12, 'i');

exports.GtkAllocation = GtkAllocation;
exports.GdkRectangle = GtkAllocation;

//############## GdkColor ############

const gdkcolorlib = require('gdkcolor.so', '-dll');
const gdkcolor = gdkcolorlib.identifiers;

const $GdkColor = Object.create($GBoxed);

register_gtype('GdkColor', $GdkColor);

const GdkColor = function() {
    if (this instanceof GdkColor)
        throw new Error('operation not permitted');
    const ptr = $malloc(gdkcolorlib.sizeOf('_GdkColor'));
    ptr.gc();
    Object.setPrototypeOf(ptr, $GdkColor);
    return ptr;
};

//standard names, '#rgb', '#rrggbb', '#rrrgggbbb' or '#rrrrggggbbbb'
GdkColor.color_parse = function(spec) {
    if (typeof spec === 'string' && spec.length > 0) {
        let color = GdkColor();
        if (gdkcolor.color_parse(spec, color)) {
            return color;
        }
        throw new Error('Invalid color name');
    }
    throw new Error('non-empty string expected');
};

$GdkColor.color_to_string = function() {
    let s = gdkcolor.color_to_string(this);
    if (s === $nullptr)
        return null;
    return $utf8String(s, -1);
};

exports.GdkColor = GdkColor;

// $print(GdkColor.color_parse('red').color_to_string()); //-> #ffff00000000

//############## GtkBorder ##############
const $GtkBorder = Object.create($GBoxed);

register_gtype('GtkBorder', $GtkBorder);

const GtkBorder = function() {
    if (this instanceof GtkBorder)
        throw new Error('operation not permitted');
    const ptr = $malloc(4*4);
    ptr.gc();
    Object.setPrototypeOf(ptr, $GtkBorder);
    return ptr;
};

boxedProperty($GtkBorder, 'left', 0, 'i');
boxedProperty($GtkBorder, 'right', 4, 'i');
boxedProperty($GtkBorder, 'top', 8, 'i');
boxedProperty($GtkBorder, 'bottom', 12, 'i');

exports.GtkBorder = GtkBorder;

//############## GtkTextIter ############

const gtktextiter = require('gtktextiter.so');

const $GtkTextIter = Object.create($GBoxed);

register_gtype('GtkTextIter', $GtkTextIter);

const GtkTextIter = function() {
    if (this instanceof GtkTextIter)
        throw new Error('operation not permitted');
    const ptr = $malloc(80);    // XXX: lib.sizeOf('_GtkTextIter') === 80
    ptr.gc();
    Object.setPrototypeOf(ptr, $GtkTextIter);
    return ptr;
};

require('_gtktextiter.js');

exports.GtkTextIter = GtkTextIter;

//#####################################
// GObject
//#####################################

const $GObject = Object.create(Object.getPrototypeOf($nullptr));

function initGObject(objPtr) {
    if (!$isPointer(objPtr))
        throw new Error("not a pointer");
    if (!$GObject.isPrototypeOf(objPtr))
        Object.setPrototypeOf(objPtr, $GObject);
    $P.set(objPtr, { handlers: [] });

    gtkb.gobject_ref_sink(objPtr);
    const id = objPtr.gc(gtkb.gobject_unref, true);
    // $print('Weak Id =', id,  gtkb.g_type_name(gtkb.gobject_type(objPtr)));
    // $print(Object.is($findWeak(id), objPtr)),

    gtkb.set_id(objPtr, id);
}

const DISPATCHER = $$persist.set(dispatcher);

$GObject.connect = function (name, callback, ...args) {
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
Object.defineProperty($GObject, 'gtype', {
    get: function() {
        return gtkb.gobject_type(this);
    }
});

register_gtype('GObject', $GObject, G_TYPE_OBJECT); 

//#####################################
// GtkAdjustment <- GObject
//#####################################

const $GtkAdjustment = Object.create($GObject);
register_gtype('GtkAdjustment', $GtkAdjustment /*, gtkadjustment.get_type() */);
// TODO: gtkadjustment.so, _gtkadjustment.js ... 


//#####################################
// GtkWidget <- GObject
//#####################################

const $GtkWidget = Object.create($GObject);

function initGtkWidget(wptr) {
    if (!$GtkWidget.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkWidget);
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

register_gtype('GtkWidget', $GtkWidget, gtkwidget.get_type());
require('_gtkwidget.js');

// Override
$GtkWidget.style_get_property = function(property_name) {
    if (typeof property_name !== 'string')
        throw new TypeError('property_name is not a string');
    return gtkb.style_get_property(this,property_name);
};

//########################################
// GtkMisc <- GtkWidget.
//########################################

const $GtkMisc = Object.create($GtkWidget);

function initGtkMisc(wptr) {
    if (!$GtkMisc.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkMisc);
    initGtkWidget(wptr);
}

register_gtype('GtkMisc', $GtkMisc, gtkmisc.get_type());

//########################################
// GtkLabel <- GtkMisc <- GtkWidget.
//########################################

const $GtkLabel = Object.create($GtkMisc);

function initGtkLabel(wptr) {
    if (!$GtkLabel.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkLabel);
    initGtkMisc(wptr);
}

exports.GtkLabel = function(text) {
    let w;
    if (typeof text === 'string') {
        w = gtklabel.new(text);
    } else {
        w = gtklabel.new($nullptr);
    }
    initGtkLabel(w);
    return w;
};

register_gtype('GtkLabel', $GtkLabel, gtklabel.get_type());

//########################################
// GtkContainer <- GtkWidget.
//########################################

const $GtkContainer = Object.create($GtkWidget);

function initGtkContainer(wptr) {
    if (!$GtkContainer.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkContainer);
    initGtkWidget(wptr);
}

register_gtype('GtkContainer', $GtkContainer, gtkcontainer.get_type());
require('_gtkcontainer.js');

//#####################################################
// GtkWindow (<- GtkBin) <- GtkContainer <- GtkWidget.
//#####################################################

exports.WINDOW_TOPLEVEL = gtkenums.GTK_WINDOW_TOPLEVEL;
exports.WINDOW_POPUP = gtkenums.GTK_WINDOW_POPUP;

const $GtkWindow = Object.create($GtkContainer);

function initGtkWindow(wptr) {
    if (!$GtkWindow.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkWindow);
    initGtkContainer(wptr);
}

$GtkWindow.set_title = function(title) {
    if (typeof title === 'string' && title.length > 0)
        gtkwindow.set_title(this, title);
};

$GtkWindow.set_default_size = function(width, height) {
        width = width|0;
        height = height|0;
        gtkwindow.set_default_size(this,
                width >= 0 ? width : -1,
                height >= 0 ? height : -1);
};

exports.GtkWindow = function(wintype = gtkenums.GTK_WINDOW_TOPLEVEL) {
    let w = gtkwindow.new(wintype);
    initGtkWindow(w);
    return w;
};

register_gtype('GtkWindow', $GtkWindow, gtkwindow.get_type());


//########################################
// GtkButton <- GtkContainer <- GtkWidget.
//########################################

const $GtkButton = Object.create($GtkContainer);

function initGtkButton(wptr) {
    if (!$GtkButton.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkButton);
    initGtkContainer(wptr);
}

exports.GtkButton = function(label) {
    if (typeof label !== 'string' || label.length === 0)
        label = "?";
    let button = gtkbutton.new_with_label(label);
    initGtkButton(button);
    return button;
};

register_gtype('GtkButton', $GtkButton, gtkbutton.get_type());
require('_gtkbutton.js');

//###############################################
// GtkBox <- GtkContainer <- GtkWidget.
//###############################################
exports.PACK_START = gtkenums.GTK_PACK_START;
exports.PACK_END = gtkenums.GTK_PACK_END;

const $GtkBox = Object.create($GtkContainer);

function initGtkBox(wptr) {
    if (!$GtkBox.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkBox);
    initGtkContainer(wptr);
}


$GtkBox.packStart = function(child, expand = true, fill = true, padding = 0) {
    if (! $GtkWidget.isPrototypeOf(child))
        throw new TypeError('child not a GtkWidget');
    padding = padding|0;
    if (padding < 0) padding = 0;
    gtkbox.pack_start(this, child, !!expand, !!fill, padding);
};

register_gtype('GtkBox', $GtkBox, gtkbox.get_type());

//#######################################################
// GtkHBox <- GtkBox <- GtkContainer <- GtkWidget.
//#######################################################

const $GtkHBox = Object.create($GtkBox);

function initGtkHBox(wptr) {
    if (!$GtkHBox.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkHBox);
    initGtkBox(wptr);
}

exports.GtkHBox = function(homogeneous = false, spacing = 0) {
    spacing = spacing|0;
    if (spacing < 0) spacing = 0;
    let hbox = gtkhbox.new(!!homogeneous, spacing);
    initGtkHBox(hbox);
    return hbox;
};

register_gtype('GtkHBox', $GtkHBox, gtkhbox.get_type());

//#######################################################
// GtkVBox <- GtkBox <- GtkContainer <- GtkWidget.
//#######################################################

const $GtkVBox = Object.create($GtkBox);

function initGtkVBox(wptr) {
    if (!$GtkVBox.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkVBox);
    initGtkBox(wptr);
}

exports.GtkVBox = function(homogeneous = false, spacing = 0) {
    spacing = spacing|0;
    if (spacing < 0) spacing = 0;
    let vbox = gtkvbox.new(!!homogeneous, spacing);
    initGtkVBox(vbox);
    return vbox;
};

register_gtype('GtkVBox', $GtkVBox, gtkvbox.get_type());

//###############################################################
// GtkScrolledWindow <- GtkContainer <- GtkWidget.
//###############################################################
exports.POLICY_ALWAYS = gtkenums.GTK_POLICY_ALWAYS;
exports.POLICY_AUTOMATIC = gtkenums.GTK_POLICY_AUTOMATIC;
exports.POLICY_NEVER = gtkenums.GTK_POLICY_NEVER;

const $GtkScrolledWindow = Object.create($GtkContainer);

function initGtkScrolledWindow(wptr) {
    if (!$GtkScrolledWindow.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkScrolledWindow);
    initGtkContainer(wptr);
}

exports.GtkScrolledWindow = function() {
    let sw = gtkscrolledwindow.new($nullptr, $nullptr);
    initGtkScrolledWindow(sw);
    return sw;
};

register_gtype('GtkScrolledWindow', $GtkScrolledWindow, gtkscrolledwindow.get_type());
require('_gtkscrolledwindow.js');

//###############################################
// GtkTextView <- GtkContainer <- GtkWidget.
//###############################################

const $GtkTextView = Object.create($GtkContainer);

function initGtkTextView(wptr) {
    if (!$GtkTextView.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkTextView);
    initGtkContainer(wptr);
}

exports.GtkTextView = function(textbuf) {
    let textview;
    if (textbuf && $GtkTextBuffer.isPrototypeOf(textbuf)) {
        textview = gtktextview.new_with_buffer(textbuf); // increments ref. count.
    } else {
        textview = gtktextview.new();
    }
    initGtkTextView(textview);
    return textview;
};

register_gtype('GtkTextView', $GtkTextView, gtktextview.get_type());
require('_gtktextview.js');


//###############################################################
// GtkTextMark <- GObject.
//###############################################################

const $GtkTextMark = Object.create($GObject);
register_gtype('GtkTextMark', $GtkTextMark);

//###############################################################
// GtkTextBuffer <- GObject.
//###############################################################

const $GtkTextBuffer = Object.create($GObject);

function initGtkTextBuffer(wptr) {
    if (!$GtkTextBuffer.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkTextBuffer);
    initGObject(wptr);
}

register_gtype('GtkTextBuffer', $GtkTextBuffer, gtktextbuffer.get_type());
require('_gtktextbuffer.js');

exports.GtkTextBuffer = function() {
    const tb = gtktextbuffer.new($nullptr);
    initGtkTextBuffer(tb);
    return tb;
};

//###############################################################
// GtkEntryBuffer <- GObject.
//###############################################################

const $GtkEntryBuffer = Object.create($GObject);

function initGtkEntryBuffer(wptr) {
    if (!$GtkEntryBuffer.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkEntryBuffer);
    initGObject(wptr);
}

register_gtype('GtkEntryBuffer', $GtkEntryBuffer, gtkentrybuffer.get_type());
require('_gtkentrybuffer.js');

exports.GtkEntryBuffer = function(s) {
    let eb;
    if (typeof s === 'string')
        eb = gtkentrybuffer.new(s, s.length);
    else
        eb = gtkentrybuffer.new($nullptr, 0);
    initGtkEntryBuffer(eb);
    return eb;
};

//###############################################################
// GtkEntryCompletion <- GObject.
//###############################################################

const $GtkEntryCompletion = Object.create($GObject);

function initGtkEntryCompletion(wptr) {
    if (!$GtkEntryCompletion.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkEntryCompletion);
    initGObject(wptr);
}

register_gtype('GtkEntryCompletion', $GtkEntryCompletion, gtkentrycompletion.get_type());
require('_gtkentrycompletion.js');

exports.GtkEntryCompletion = function() {
    const ec = gtkentrycompletion.new();
    initGtkEntryCompletion(ec);
    return ec;
};

//###############################################################
// GtkEntry <- GtkWidget.
//###############################################################

const $GtkEntry = Object.create($GtkWidget);

function initGtkEntry(wptr) {
    if (!$GtkEntry.isPrototypeOf(wptr))
        Object.setPrototypeOf(wptr, $GtkEntry);
    initGtkWidget(wptr);
}

register_gtype('GtkEntry', $GtkEntry, gtkentry.get_type());
require('_gtkentry.js');

exports.GtkEntry = function(b) {
    let e;
    if ($GtkEntryBuffer.isPrototypeOf(b)) {
        e = gtkentry.new_with_buffer(b);
    } else {
        e = gtkentry.new();
    }
    initGtkEntry(e);
    return e;
};


//for (let i in GTYPES) { $print(i); }

