//module.__path = module.__path + ':./libs';
const gtkb = require(__dirname + '/libs/gtkb.so');

const GTYPES = {};
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

function register_gtype(gtype_name, proto, gtype) {
    // this ensures GType fits in a double
    // gtkb.g_type_from_name(gtype_name); // XXX: returns 0 if not registered yet ..
    GTYPES[gtype_name] = proto;
    if (gtype && gtkb.g_type_is_a(gtype, G_TYPE_OBJECT)) {
        install_props(proto, gtype);
    }
}

function gtype_to_js(gtype_name, gtype, ptr) {
    const GObject = GTYPES['GObject'];
    if (ptr === $nullptr)
        return null;
    let gproto = null;
    if (gtype_name === 'GObject') {
        gproto = GObject;
    } else if ((gtype_name in GTYPES)
            && GObject.isPrototypeOf(GTYPES[gtype_name])) {
        gproto = GTYPES[gtype_name];
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
        Object.setPrototypeOf(ptr, gproto);
        gtkb.gobject_ref_sink(ptr);
        const id = ptr.gc(gtkb.gobject_unref, true);
        gtkb.set_id(ptr, id);
        return ptr;
    }

    if ((gtype_name in GTYPES)
            && GTYPES['GBoxed'].isPrototypeOf(GTYPES[gtype_name])) {
        // always a copy
        Object.setPrototypeOf(ptr, GTYPES[gtype_name]);
        ptr.gc();   //ptr.gc(gtkb.g_boxed_free, gtype);
        return ptr;
    } // else if(..) { just boxed proto ..}
 
    // FIXME  ..
    return ptr;
}

module.exports = { GTYPES, register_gtype, gtype_to_js };
