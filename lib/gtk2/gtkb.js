//module.__path = module.__path + ':./libs';
const gtkb = require(__dirname + '/libs/gtkb.so');

const CTYPES = {};
const G_TYPE_OBJECT = gtkb.g_type_from_name('GObject');

function install_props(proto, gtype) {
    gtkb.class_props(gtype).forEach(function (name) {
        Object.defineProperty(proto, name, {
            get: function () {
                    return gtkb.get_property(this, name);
                },
            set: function (val) {
                    return gtkb.set_property(this, name, val);
                }
            });
        }
    );
}

function ctype(gtype_name, proto, gtype, init) {
    if (typeof proto === 'string' && (proto in CTYPES)) {
        let base = proto;
        proto = Object.create(CTYPES[base].proto, { __ctype__: { value: gtype_name }});
        if (typeof init === 'undefined') {
            init = function(ptr) {
                if (!isa(ptr, gtype_name))
                    Object.setPrototypeOf(ptr, CTYPES[gtype_name].proto);
                if (typeof CTYPES[base].init === 'function')
                    CTYPES[base].init(ptr);
            };
        }
    } else {
        if (!({}).hasOwnProperty.call(proto, '__ctype__')) {
            Object.defineProperty(proto, '__ctype__', { value: gtype_name });
        }
    }

    CTYPES[gtype_name] = { proto, init };
    if (gtype && gtkb.g_type_is_a(gtype, G_TYPE_OBJECT)) {
        install_props(proto, gtype);
    }
    return proto;
}

function ctype_to_js(gtype_name, gtype, ptr) {
    const GObject = CTYPES['GObject'].proto;
    if (ptr === $nullptr)
        return null;
    let gproto = null;
    if (gtype_name === 'GObject') {
        gproto = GObject;
    } else if ((gtype_name in CTYPES)
            && GObject.isPrototypeOf(CTYPES[gtype_name].proto)) {
        gproto = CTYPES[gtype_name].proto;
    } else {
        if (!gtype) {
            gtype = gtkb.g_type_from_name(gtype_name);
            // $print('gtype_to_js(): gtype from name =', gtype);
        }
        if (gtype && gtkb.g_type_is_a(gtype, G_TYPE_OBJECT)) {
            gproto = GObject;
        }
    }
    if (gproto !== null) {
        const obj = $findWeak(gtkb.get_id(ptr));
        if (obj)
            return obj;

        /*$print('ctype_to_js:', gtype_name,
                        gtkb.g_type_name(gtkb.gobject_type(ptr))); */
        // find actual gtype and g_type_name. gtype is likely 0.
        const typ = gtkb.gobject_type(ptr);
        if (typ !== 0) {
            const typname = gtkb.g_type_name(typ);
            if (typname && (typname in CTYPES)) {
                CTYPES[typname].init(ptr);
                return ptr;
            }
        }

        CTYPES[gproto.__ctype__].init(ptr);
        return ptr;
    }

    if ((gtype_name in CTYPES)
            && isa(CTYPES[gtype_name].proto, 'GBoxed')) {
        // always a copy
        Object.setPrototypeOf(ptr, CTYPES[gtype_name].proto);
        ptr.gc();   //ptr.gc(gtkb.g_boxed_free, gtype);
        return ptr;
    } // else if(..) { just boxed proto ..}
 
    // FIXME  ..
    return ptr;
}

function isa(obj, type_name) {
    return ((type_name in CTYPES) &&
        CTYPES[type_name].proto.isPrototypeOf(obj));
}

module.exports = { CTYPES, ctype, ctype_to_js, isa };
