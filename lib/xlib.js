const Long = require('./long.js').Long;
const Buffer = require('./buffer.js').Buffer;

const libX11 = new DLL('./x/libxlibjs.so', __dirname);
const xlib = libX11.identifiers;

let WmDelWindowAtom;
let WmProtocolsAtom;
let SCREEN;
let VISUAL;
let ROOTWINDOW;
let BLACKPIXEL;
let WHITEPIXEL;
let COLORMAP;

const WINLIST = {};
const RECTS = [];   // "dirty" areas to be Expose-d

// open Display once.
let DISPLAY = (function () {
    const dpy = xlib.XOpenDisplay($nullptr);
    if (! dpy)
        throw new Error("unable to open X display");
    WmDelWindowAtom = xlib.XInternAtom(dpy, "WM_DELETE_WINDOW", false);
    WmProtocolsAtom = xlib.XInternAtom(dpy, "WM_PROTOCOLS", false);

    // int
    SCREEN = xlib.DefaultScreen(dpy);

    // pointer
    VISUAL = xlib.DefaultVisual(dpy, SCREEN);

    // unsigned long
    ROOTWINDOW = xlib.RootWindow(dpy, SCREEN);

    // unsigned long
    BLACKPIXEL = xlib.BlackPixel(dpy, SCREEN);
    WHITEPIXEL = xlib.WhitePixel(dpy, SCREEN);

    // XID -- unsigned long
    COLORMAP = xlib.DefaultColormap(dpy, SCREEN);

    return dpy;
})();


const eventTypes = [ null, null,
'KeyPress', 'KeyRelease', 'ButtonPress', 'ButtonRelease',
'MotionNotify', 'EnterNotify', 'LeaveNotify',
'FocusIn', 'FocusOut', 'KeymapNotify', 'Expose',
'GraphicsExpose', 'NoExpose', 'VisibilityNotify',
'CreateNotify', 'DestroyNotify', 'UnmapNotify',
'MapNotify', 'MapRequest', 'ReparentNotify',
'ConfigureNotify', 'ConfigureRequest', 'GravityNotify',
'ResizeRequest', 'CirculateNotify', 'CirculateRequest',
'PropertyNotify', 'SelectionClear', 'SelectionRequest',
'SelectionNotify', 'ColqormapNotify', 'ClientMessage', 'MappingNotify' ];

const eventStructs = [ null, null,
'XKeyEvent', 'XKeyEvent', 'XButtonEvent', 'XButtonEvent',
'XMotionEvent', 'XCrossingEvent', 'XCrossingEvent',
'XFocusChangeEvent', 'XFocusChangeEvent', 'XKeymapEvent', 'XExposeEvent',
'XGraphicsExposeEvent', 'XNoExposeEvent', 'XVisibilityEvent',
'XCreateWindowEvent', 'XDestroyWindowEvent', 'XUnmapEvent',
'XMapEvent', 'XMapRequestEvent', 'XReparentEvent',
'XConfigureEvent', 'XConfigureRequestEvent', 'XGravityEvent',
'XResizeRequestEvent', 'XCirculateEvent', 'XCirculateRequestEvent',
'XPropertyEvent', 'XSelectionClearEvent', 'XSelectionRequestEvent',
'XSelectionEvent', 'XColormapEvent', 'XClientMessageEvent', 'XMappingEvent' ];

const NoEventMask = 0;
const NonMaskableMask = 0x40000000;

const eventMasks = [ null, null,
(1<<0), (1<<1), (1<<2), (1<<3),
(1<<6)|(1<<13), (1<<4), (1<<5),
(1<<21), (1<<21), (1<<14), (1<<15),
NonMaskableMask, NonMaskableMask, (1<<16),
(1<<19), (1<<17), (1<<17),
(1<<17), (1<<20), (1<<17),
(1<<17), (1<<20), (1<<17),
(1<<18), (1<<17), (1<<20),
(1<<22), NonMaskableMask, NonMaskableMask,
NonMaskableMask, (1<<23), NonMaskableMask, NonMaskableMask ];


class XEvent extends Buffer {
    constructor() {
        const sz = libX11.sizeOf('XEvent');
        super(sz);
        // $print(JSON.stringify(libX11.structDesc('XAnyEvent')));
        this._windowType = libX11.typeOf('XAnyEvent', 'window');
        this._windowOffset = libX11.offsetOf('XAnyEvent', 'window');
        this._displayOffset = libX11.offsetOf('XAnyEvent', 'display');
        this._keybuf = null;
        this._keysym = null;
    }

    get type() {
        return super.getInt32(0);
    }

    // TODO: get serial, send_event

    get display() {
        return super.unpack(this._displayOffset, 'p')[0];
    }

    get window() {
        if (this._windowType === 'J')
            return super.getUint64(this._windowOffset);
        return super.unpack(this._windowOffset, this._windowType)[0];
    }

    get(field) {
        const tp = this.type;
        let evs = eventStructs[tp];
        if (!evs)
            throw new Error('invalid event type');
        let fieldType = libX11.typeOf(evs, field);
        let fieldOffset = libX11.offsetOf(evs, field);
        if (fieldType === null)
            return undefined;
        return super.unpack(fieldOffset, fieldType)[0];
    }

    // Window object instance
    get win() {
        // undefined if not in list
        return WINLIST[this.window.toString()];
    }

    set type(value) {
        super.setInt32(0, value);
    }

    set window(w) {
        if (typeof WINLIST[w.toString()] === 'undefined')
            throw new Error('invalid window');
        if (this._windowType === 'J')
            super.setUint64(this._windowOffset, w);
        else
            super.pack(this._windowOffset, this._windowType, w);
    }

    get x() {
        switch (this.type) {
        case XEvent.Expose:
        case XEvent.KeyPress:
        case XEvent.KeyRelease:
        case XEvent.ButtonPress:
        case XEvent.ButtonRelease:
        case XEvent.MotionNotify:
        case XEvent.EnterNotify:
        case XEvent.LeaveNotify:
        case XEvent.ConfigureNotify:
        case XEvent.ConfigureRequest:
        case XEvent.CreateNotify:
        case XEvent.ReparentNotify:
        case XEvent.GraphicsExpose:
        case XEvent.GravityNotify:
            return this.get('x');
        default:
            break;
        }
    }

    get y() {
        switch (this.type) {
        case XEvent.Expose:
        case XEvent.KeyPress:
        case XEvent.KeyRelease:
        case XEvent.ButtonPress:
        case XEvent.ButtonRelease:
        case XEvent.MotionNotify:
        case XEvent.EnterNotify:
        case XEvent.LeaveNotify:
        case XEvent.ConfigureNotify:
        case XEvent.ConfigureRequest:
        case XEvent.CreateNotify:
        case XEvent.ReparentNotify:
        case XEvent.GraphicsExpose:
        case XEvent.GravityNotify:
            return this.get('y');
        default:
            break;
        }
    }

    get width() {
        switch (this.type) {
        case XEvent.Expose:
        case XEvent.ConfigureNotify:
        case XEvent.ConfigureRequest:
        case XEvent.CreateNotify:
        case XEvent.GraphicsExpose:
        case XEvent.ResizeRequest:
            return this.get('width');
        default:
            break;
        }
    }

    get height() {
        switch (this.type) {
        case XEvent.Expose:
        case XEvent.ConfigureNotify:
        case XEvent.ConfigureRequest:
        case XEvent.CreateNotify:
        case XEvent.GraphicsExpose:
        case XEvent.ResizeRequest:
            return this.get('height');
        default:
            break;
        }
    }

    get count() {
        switch (this.type) {
        case XEvent.Expose:
        case XEvent.GraphicsExpose:
            return this.get('count');
        default:
            break;
        }
    }

    set(field, value) {
        const tp = this.type;
        let evs = eventStructs[tp];
        if (!evs)
            throw new Error('invalid event type');
        let fieldType = libX11.typeOf(evs, field);
        let fieldOffset = libX11.offsetOf(evs, field);
        if (fieldType === undefined) // struct etc.
            throw new Error('cannot set XEvent field ' + field);
        super.pack(fieldOffset, fieldType, value);
    }

    lookupChar() {
        if (this.type !== XEvent.KeyPress && this.type !== XEvent.KeyRelease)
            throw new Error('not a KeyEvent');
        if (this._keybuf === null) {
            this._keybuf = new Buffer(16);
            this._keysym = new Buffer(8);   // unsigned long *
        }
        const count = xlib.XLookupString(this, this._keybuf, 16,
                                                    this._keysym, $nullptr);
        if (count !== 0)
            return String.fromCharCode(this._keybuf.getInt8(0));
        return null;
    }

    /*
    // upcast to appropriate XEvent union member
    // let ev = new XEvent();
    // let kev = ev.cast();
    // $print(kev.x, kev.y);
    // $print(kev._XEvent === ev);
    cast() {
        return new Proxy(this, {
            get : function (target, name) {
                if (name === '_XEvent')  // downcast to XAnyEvent
                    return target;
                return target.get(name);
            }
        });
    }
    */
}

for (let i=2; i < eventTypes.length;i++) {
    XEvent[eventTypes[i]] = i;
}


// GC mask bits
const GCFunction            = (1<<0);
const GCPlaneMask           = (1<<1);
const GCForeground          = (1<<2);
const GCBackground          = (1<<3);
const GCLineWidth           = (1<<4);
const GCLineStyle           = (1<<5);
const GCCapStyle            = (1<<6);
const GCJoinStyle           = (1<<7);
const GCFillStyle           = (1<<8);
const GCFillRule            = (1<<9);
const GCTile                = (1<<10);
const GCStipple             = (1<<11);
const GCTileStipXOrigin     = (1<<12);
const GCTileStipYOrigin     = (1<<13);
const GCFont                = (1<<14);
const GCSubwindowMode       = (1<<15);
const GCGraphicsExposures   = (1<<16);
const GCClipXOrigin         = (1<<17);
const GCClipYOrigin         = (1<<18);
const GCClipMask            = (1<<19);
const GCDashOffset          = (1<<20);
const GCDashList            = (1<<21);
const GCArcMode             = (1<<22);


// button names
//Button1 = 1;
//Button2 = 2;
//Button3 = 3;
//Button4 = 4;
//Button5 = 5;

// XXX: GC mask is passed as unsigned long to functions but fits in int!!!
const XGCInfo = {
    function : { mask: GCFunction,
        values: [ "GXclear", "GXand", "GXandReverse", "GXcopy",
        "GXandInverted", "GXnoop", "GXxor", "GXor",
        "GXnor", "GXequiv", "GXinvert", "GXorReverse",
        "GXcopyInverted", "GXorInverted", "GXnand", "GXset" ] },
    plane_mask : { mask: GCPlaneMask, values: null },
    foreground : { mask: GCForeground, values: null },
    background : { mask: GCBackground, values: null },
    line_width : { mask: GCLineWidth, values: null },
    line_style : { mask: GCLineStyle,
                 values: [ "LineSolid", "LineOnOffDash", "LineDoubleDash" ] },
    cap_style : { mask: GCCapStyle,
                values: [ "CapNotLast", "CapButt", "CapRound", "CapProjecting" ] },
    join_style : { mask: GCJoinStyle,
                values: [ "JoinMiter", "JoinRound", "JoinBevel"] },
    fill_style : { mask: GCFillStyle, values: ["EvenOddRule", "WindingRule"] },
    arc_mode : { mask: GCArcMode, values: [ "ArcChord", "ArcPieSlice"] },
    tile : { mask: GCTile, values: null },     // Pixmap
    stipple : { mask: GCStipple, values: null },   // Pixmap
    ts_x_origin : { mask: GCTileStipXOrigin, values: null },
    ts_y_origin : { mask: GCTileStipYOrigin, values: null },
    font : { mask: GCFont, values: null }, // Font is XID (unsigned long)
    subwindow_mode : { mask: GCSubwindowMode, values: null },
    graphics_exposures : { mask: GCGraphicsExposures, values: null },
    clip_x_origin : { mask: GCClipXOrigin, values: null },
    clip_y_origin : { mask: GCClipYOrigin, values: null },
    clip_mask : { mask: GCClipMask, values: null }, // Pixmap
    dash_offset : { mask: GCDashOffset,  values: null },
    dashes : { mask: GCDashList, values: null },
};


class XGCValues extends Buffer {
    constructor() {
        super(libX11.sizeOf('XGCValues'));
    }

    get(field) {
        const type = libX11.typeOf('XGCValues', name);
        const offset = libX11.offsetOf('XGCValues', name);
        if (type !== null)
            return super.unpack(offset, type)[0];
    }

    set(field, value) {
        const type = libX11.typeOf('XGCValues', field);
        const offset = libX11.offsetOf('XGCValues', field);
        if (type !== null)
            super.pack(offset, type, value);
    }
}

// The GC can be used with any destination drawable having the same root
// and depth as the specified drawable.
class GC {
    constructor(drawable, option) {
        this._gcv = new XGCValues();
        // TODO: option instanceof XGCValues?
        const valuemask = GCForeground|GCBackground|GCLineWidth;
        let foreground = BLACKPIXEL;
        let background = WHITEPIXEL;
        let line_width = 1;
        if (option && typeof option === 'object') {
            if (typeof option.foreground !== 'undefined')
                foreground = pixel(option.foreground);
            if (typeof option.background !== 'undefined')
                background = pixel(option.background);
            if (typeof option.line_width !== 'undefined') {
                line_width = option.line_width|0;
                if (line_width <= 0)
                    line_width = 1;
            }
        }

        this._gcv.set('foreground', foreground);
        this._gcv.set('background', background);
        this._gcv.set('line_width', line_width);

        this._gc = xlib.XCreateGC(DISPLAY, drawable,
                    Long(valuemask, false), this._gcv).notNull();      
    }

    set(field, newval) {
        field = field + '';
        if (!XGCInfo.hasOwnProperty(field))
            throw new Error('invalid XGCValues field');
        if (field === 'foreground' || field === 'background')
            newval = Window.pixel(newval);

        // gc.clip_mask = 0 to set mask to "None" (None is 0L, see /usr/include/X11/X.h)
        const gci = XGCInfo[field];
        let rval = newval;
        if (gci.values && (rval = gci.values.indexOf(newval)) == -1)
            throw new RangeError("illegal GC value");
        const gcv = this._gcv;
        gcv.set(field, rval);
        xlib.XChangeGC(DISPLAY, this._gc, Long(gci.mask, false), gcv);
    }

    get(field) {
        // XXX: foreground and background are pixel values.
        //  See XQueryColor for conversion to name
        //  XGCValues: ".. the clip-mask and dash-list cannot be requested .."
        field = field + '';
        if (!XGCInfo.hasOwnProperty(field))
            throw new Error('invalid XGCValues field');
        if (field === 'clip_mask' || field === 'dashes')
            throw new Error("cannot get value for attribute");
        const gci = XGCInfo[field];
        const gcv = this._gcv;
        xlib.XGetGCValues(DISPLAY, this._gc, Long(gci.mask, false), gcv);
        const val = gcv.get(field);
        return gci.values ? gci.values[val] : val;
    }

    free() {
        if (this._gc !== null) {
            xlib.XFreeGC(DISPLAY, this._gc);
            this._gc = null;
        }
    }

    static isValid(gc) {
        return ((gc instanceof GC) && gc._gc !== null);
    }
}

class Region {
    constructor() {
        this._region = xlib.XCreateRegion();
        this._region.gc(xlib.XDestroyRegion);
    }

    add(x, y, width, height) {
        // XRectangle : struct { short x, y; unsigned short width, height; }
        const rect = new Int16Array(arguments, v => v|0);
        xlib.XUnionRectWithRegion(rect, this._region, this._region);
    }

    isEmpty() {
        return !!xlib.XEmptyRegion(this._region);
    }

    contains(x, y) {
        return !!xlib.XPointInRegion(this._region, x|0, y|0);
    }

    clip(gc) {
        if (!GC.isValid(gc))
            throw new Error('invalid argument');
        xlib.XSetRegion(DISPLAY, gc._gc, this._region);
    }
}

class Drawable {
    constructor() {
        this._d = null;
    }

    _setDrawable(d) {
        this._d = d;
    }

    // TODO drawText()

    _checkDrawable(gc) {
        if (this._d === null)
            throw new Error("invalid Drawable");
        if (!(gc instanceof GC) || gc._gc === null)
            throw new Error("invalid GC");
    }

    drawArc(gc, x, y, width, height, angle1, angle2) {
        this._checkDrawable(gc);
        xlib.XDrawArc(DISPLAY, this._d, gc._gc, x|0, y|0,
                            width|0, height|0, angle1|0, angle2|0);
    }

    drawLine(gc, x1, y1, x2, y2) {
        this._checkDrawable(gc);
        xlib.XDrawLine(DISPLAY, this._d, gc._gc, x1|0, y1|0, x2|0, y2|0);
    }

    drawPoint(gc, x, y) {
        this._checkDrawable(gc);
        xlib.XDrawPoint(DISPLAY, this._d, gc._gc, x|0, y|0);
    }

    drawRect(gc, x, y, width, height) {
        this._checkDrawable(gc);
        xlib.XDrawRectangle(DISPLAY, this._d, gc._gc,
                                    x|0, y|0, width|0, height|0);
    }

    fillArc(gc, x, y, width, height, angle1, angle2) {
        this._checkDrawable(gc);
        xlib.XFillArc(DISPLAY, this._d, gc._gc, x|0, y|0,
                            width|0, height|0, angle1|0, angle2|0);
    }

    fillRect(gc, x, y, width, height) {
        xlib.XFillRectangle(DISPLAY, this._d, gc._gc,
                                    x|0, y|0, width|0, height|0);
    }

    // XCopyArea and XCopyPlane can generate GraphicsExpose/NoExpose events 
    copyArea(dest, gc, sx, sy, width, height, dx, dy) {
        if (! (dest || dest instanceof Drawable))
            throw new TypeError('DEST is not a Drawable');
        this._checkDrawable(gc);
        xlib.XCopyArea(DISPLAY, this._d, dest._d, gc._gc,
                        sx|0, sy|0, width|0, height|0, dx|0, dy|0);
    }

    createGC() {
        if (this._d === null)
            throw new Error('invalid Drawable');
        // FIXME option argument ..
        return new GC(this._d);
    }
}


class Window extends Drawable {
    constructor() {
        super();
        this._init();
    }

    _init() {
        this._w = null;
        this._borderPixel = BLACKPIXEL;  // Long(0, false);
        this._backgroundPixel = null; // use setter first
        this._eventMask = 0;
        this._callbacks = [];
        this._onWMDelete = null;

        this.isTopLevel = true;
        this.borderWidth = 0;
        super._setDrawable(null);
    }

    create(parent, x, y, width, height) {
        if (this._w !== null)
            throw new Error("window exists!");
        let parentWindow = null;
        if (! parent) {
            parentWindow = ROOTWINDOW;
        } else if (parent instanceof Window && parent._w != null) {
            parentWindow = parent._w;
            this.isTopLevel = false;
        } else
            throw new Error("invalid parent window");

//        w = xlib.XCreateSimpleWindow(DISPLAY, parentWindow,
//                x, y, width, height,
//                this.borderWidth,
//                this.borderPixel,
//                this.backgroundPixel
//        );

        // Use XCreateWindow for flicker-free cairo drawing.
        // N.B.: Can set background using w.background = ... after window creation
        const w = xlib.XCreateWindow(DISPLAY, parentWindow,
                        x|0, y|0, width|0, height|0,
                        this.borderWidth,
                        0, // depth - CopyFromParent
                        0, // class - CopyFromParent,
                        $nullptr, // visual - CopyFromParent,
                        Long(0, false), // valuemask - unsigned long
                        $nullptr
                    );
        this._w = w;
        super._setDrawable(w);
        xlib.XSetWindowBorder(DISPLAY, w, this._borderPixel);
        // Do not set background pixel for flicker-free painting with cairo
        // xlib.XSetWindowBackground(DISPLAY, w, this._backgroundPixel);

        // Always run default "destroy-notify" handler
        this._eventMask |= eventMasks[XEvent.DestroyNotify];
        xlib.XSelectInput(DISPLAY, w,
                Long(this._eventMask & ~NonMaskableMask, true));

        if (this.isTopLevel) {
            let atom = new Buffer(8);
            atom.setUint64(0, WmDelWindowAtom); // Atom - unsigned long
            xlib.XSetWMProtocols(DISPLAY, w, atom, 1);
        }
        WINLIST[w.toString()] = this;
    }

    _checkWindow() {
        if (this._w === null)
            throw new Error("unrealized window");
    }

    map() {
        this._checkWindow();
        xlib.XMapWindow(DISPLAY, this._w);
    }

    unmap() {
        this._checkWindow();
        xlib.XUnmapWindow(DISPLAY, this._w);
    }

    resize(width, height) {
        this._checkWindow();
        xlib.XResizeWindow(DISPLAY, this._w, width|0, height|0);
    }

    moveResize(x, y, width, height) {
        this._checkWindow();
        xlib.XMoveResizeWindow(DISPLAY, this._w, x|0, y|0, width|0, height|0);
    }

    clear() {
        // same as xlib.XClearArea(DISPLAY, this._w, 0, 0, 0, 0, false)
        this._checkWindow();
        xlib.XClearWindow(DISPLAY, this._w);
    }

    clearArea(x, y, width, height, exposures) {
        this._checkWindow();
        xlib.XClearArea(DISPLAY, this._w,
                        x|0, y|0, width|0, height|0, !!exposures);
    }

    // creates Expose event
    redraw() {
        this._checkWindow(); 
        xlib.XClearArea(DISPLAY, this._w, 0, 0, 0, 0, true);
    }

    addEventListener(type, cb, ...args) {
        // cb.call(this = Window, XEvent, ...args)

        if (!eventTypes[type])
            throw new Error('invalid event type');
        if (cb === null) {
            // remove listener
            if (this._callbacks[type]) {
                this._eventMask &= eventMasks[type];
                this._callbacks[type] = null;
            }
        } else if (typeof cb === 'function') {
            this._eventMask |= eventMasks[type];
            this._callbacks[type] = { callback: cb, data: args };
        } else {
            throw new Error('invalid argument');
        }

        if (this._w !== null) {
            xlib.XSelectInput(DISPLAY, this._w,
                    Long(this._eventMask & ~NonMaskableMask, true)); // mask is long here!
        }
    }

    // window manager ("WM_DELETE_WINDOW")
    onWMDelete(cb, ...args) {
        // cb.call(this = Window, ...args)
        if (typeof cb === 'function' && this.isTopLevel)
            this._onWMDelete = { callback: cb, data: args };
    }

    destroy() {
        if (this._w === null)
            return;
        xlib.XDestroyWindow(DISPLAY, this._w);
        xlib.XFlush(DISPLAY);
    }

    setTitle(title) {
        this._checkWindow();
        xlib.XStoreName(DISPLAY, this._w, title + '');
    }

    // Add rectangle to the update area ("dirty" regions) for window.
    // After the current batch of events has been processed in the event loop,
    // the window will receive expose event for each rectangle.
    // N.B.: unlike XClearArea, background isn't repainted.
    invalidateRect(x, y, width, height) {
        this._checkWindow();
        RECTS.push({ window: this._w, x: x|0, y: y|0,
                width: width|0, height: height|0 });
    }

    set background(value) {
        // value -- '#hhhhhh', color name or unsigned Long
        this._backgroundPixel = Window.pixel(value);
        if (this._w !== null) {
            xlib.XSetWindowBackground(DISPLAY, this._w, this._backgroundPixel);
            // need to repaint if changing background_pixel
            this.redraw();
        }
    }

    get background() {
        // value -- '#hhhhhh', color name or unsigned Long
        return this._backgroundPixel;
    }

    set border(value) {
        this._borderPixel = Window.pixel(value);
        if (this._w !== null)
            xlib.XSetWindowBorder(DISPLAY, this._w, this._borderPixel);
    }

    get border() {
        return this._borderPixel;
    }

    // TODO:    setClass(name, className)

    // unsigned long
    static colorPixel(colorName) {
        const c1 = new Buffer(libX11.sizeOf('XColor'));
        if (xlib.XParseColor(DISPLAY, COLORMAP, colorName, c1)
                &&  xlib.XAllocColor(DISPLAY, COLORMAP, c1)) {
            const type = libX11.typeOf('XColor', 'pixel');
            const offset = libX11.offsetOf('XColor', 'pixel');
            return c1.unpack(offset, type)[0];
        }
        console.log('Xlib: failed to allocate color:', colorName);
        return BLACKPIXEL;
    }

    // name -- color in /usr/share/X11/rgb.txt or "#ffffff" 
    static pixel(name) {
        if (typeof name === 'string') { 
            let rgb = name;
            name.replace(/^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/gi,
                function (p0, p1, p2, p3) {
                    rgb = 'rgb:' + p1 + '/' + p2 + '/' + p3;
                }
            );
            return Window.colorPixel(rgb);
        }
        return Long(name, false);
    }

    static exists(win) {
        if (win && win instanceof Window) {
           return (win._w !== null);
        }
        throw new TypeError('not a Window argument');
    }
}

Object.defineProperty(Window, 'BLACKPIXEL', {
    value: BLACKPIXEL,
    writable: false,
    enumerable: true
});

Object.defineProperty(Window, 'WHITEPIXEL', {
    value: WHITEPIXEL,
    writable: false,
    enumerable: true
});

Object.defineProperty(Window, 'DISPLAY', {
    get: function() { return DISPLAY; },
    enumerable: true
});

Object.defineProperty(Window, 'VISUAL', {
    get: function() { return VISUAL; },
    enumerable: true
});

/**********************************************
 * X Event processing
 **********************************************/
let __fd__ = xlib.ConnectionNumber(DISPLAY);
let __evp__ = new XEvent();
let __done__ = false;

function nextEvent() {
    xlib.XNextEvent(DISPLAY, __evp__);  // blocking
    return __evp__;
}

// default DestroyNotify handler
function onDestroy(win) {
    // console.log("DestroyNotify w =", win._w);
    delete WINLIST[win._w.toString()];
    win._init();
    if (Object.keys(WINLIST).length === 0)
        __done__ = true;
}

function compressMotionEvents(ev) {
    // ev.type == XEvent.MotionNotify
    const sw = ev.get('subwindow');
    const w = ev.window;
    // console.log('subwindow', sw);
    const ev1 = new XEvent();
    while (xlib.XPending(DISPLAY)) {
        xlib.XPeekEvent(DISPLAY, ev1);
        if (ev1.type === XEvent.MotionNotify
                && w.eq(ev1.window) && sw.eq(ev1.get('subwindow'))
        ) {
            // replace the current event with the next one
            ev = nextEvent();
            console.log('motion event compressed ..');
        } else
            break;
    }
    return ev;
}

function dispatch(ev) {
    const type = ev.type;
    const win = ev.win;

    if (typeof win === 'undefined') // destroyed and removed from WINLIST?
        return;
    const cb = win._callbacks[type];
    if (cb) {
        if (type === XEvent.MotionNotify)
            ev = compressMotionEvents(ev);
        cb.callback.call(win, ev, ...cb.data);
    }

    if (type === XEvent.DestroyNotify) {
        // Always run the default handler.
        onDestroy(win);
    } else if (type === XEvent.ClientMessage) {
        // ev.message_type and ev.xclient.data.l[0]
        const off = libX11.offsetOf(eventStructs[type], 'data'); // union { long[5], ..
        const data0 = ev.unpack(off, 'j')[0];

        if (WmProtocolsAtom.eq(ev.get('message_type')) && WmDelWindowAtom.eq(data0)) {
            if (win._onWMDelete) {
                const d = win._onWMDelete;
                d.callback.call(win, ...d.data);
            } else
                win.destroy();
        }
    }
}

function processUpdates() {
    if (__done__ || RECTS.length === 0)
        return;
    const ev = new XEvent();
    ev.type = XEvent.Expose;
    let r;
    while (r = RECTS.pop()) {
        // Set window, x, y, width and height
        let w = r.window;
        if (typeof WINLIST[w.toString()] === 'undefined')
            continue;
        ev.window = w;
        ev.set('x', r.x);
        ev.set('y', r.y);
        ev.set('width', r.width);
        ev.set('height', r.height);
        xlib.XSendEvent(DISPLAY, w, false, Long(NoEventMask, true), ev);
    }
    xlib.XFlush(DISPLAY);
}

// Blocking
XEvent.wait = function() {
    if (Object.keys(WINLIST).length === 0)
        return;
    __done__ = false;
    while (! __done__) {
        dispatch(nextEvent());
        processUpdates();
    }
};


function waitXEvent() {
    // xlib.XSync(DISPLAY, false);
    while (! __done__ && xlib.XPending(DISPLAY)) {
        dispatch(nextEvent());
    }
    processUpdates();
    if (__done__)
        return Promise.resolve(true);
    return $go(fdevent, __fd__)
    .then(() => {
        return false;
    })
    .catch((err) => {
        console.log('Xlib: IO error');
        return true;
    });
};

// XXX: use onExit to free resources such as (shared) GC.
async function loop(onExit, ...args) {
    let done = (Object.keys(WINLIST).length === 0);
    if (!done)
        __done__ = false;
    while (!done)
        done = await waitXEvent();
    if (typeof onExit === 'function')
        onExit(...args);
}

// Non-blocking
XEvent.await = loop;

// Primarily for Xlib calls outside the async loop.
XEvent.flush = function () {
    xlib.XFlush(DISPLAY);
};

exports.XEvent = XEvent;
exports.Window = Window;
exports.Region = Region;
// exports.XGCValues = XGCValues;
// exports.GC = GC;
// exports.Drawable = Drawable;

