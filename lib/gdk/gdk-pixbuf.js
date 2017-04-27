const gLoader = require('./libgdk-pixbuf-loader.so');
const gPixbuf = require('./libgdk-pixbuf.so');
const _await = require('./libpixbuf-await.so').await;
const gObject = require('./libgobject.so');
const gCairo = require('./libgdk-cairo.so');


class Pixbuf {
    constructor(filename) {
        if (typeof filename === 'undefined')
            this._pixbuf = null;
        else if (typeof filename === 'string') {
            this._pixbuf = gPixbuf.new_from_file(filename, $nullptr);
            if (this._pixbuf === $nullptr)
                throw new Error('unable to get Pixbuf from file');
            // gObject.is_floating(this._pixbuf)  => false
            this._pixbuf.gc(gObject.unref);
        } else
            throw new TypeError('invalid argument');
    }

    _init(p) {
        if ($isPointer(p, true)) {
            gObject.ref(p); // XXX: don't use the return value!
            this._pixbuf = p;
            this._pixbuf.gc(gObject.unref);
        } else
            throw new TypeError('invalid argument');
    }

    get width() {
        return gPixbuf.get_width(this._pixbuf);
    }

    get height() {
        return gPixbuf.get_height(this._pixbuf);
    }

    get n_channels() {
        return gPixbuf.get_n_channels(this._pixbuf);
    }

    // Queries whether a pixbuf has an alpha channel (opacity information).
    get has_alpha() {
        return gPixbuf.get_has_alpha(this._pixbuf);
    }

    static set_source(cr, pixbuf, pixbuf_x, pixbuf_y) {
        // cr -- cairo_t object
        if ($isPointer(cr) && cr.constructor.name === 'cairo_t'
                && pixbuf instanceof Pixbuf && pixbuf._pixbuf) {
            gCairo.set_source_pixbuf(cr, pixbuf._pixbuf,
                pixbuf_x|0, pixbuf_y|0);
        }
    }
}

const SIZE_PREPARED = 1;
const AREA_PREPARED = 2;
const AREA_UPDATED = 3;
const CLOSED = 4;


class PixbufLoader {
    constructor(on) {
        this._loader = gLoader.new();
        this._loader.gc(gObject.unref);
        this._loaderPr = new Promise(resolve => {
            this._resolveLoader = resolve;
        });
        this._pixbuf = null;
        this._on = new Array(5);

        if (on && typeof on === 'object') {
            [ null, 'size_prepared', 'area_prepared', 'area_updated',
                'closed' ].map((x, i) => {
                    if (i > 0 && typeof on[x] === 'function')
                        this._on[i] = on[x];
                    //else this._on[i] = (function() { });
            });
        }


        $co(_await, this._loader, (data, done) => {
            const self = this;
            const on = self._on;
            const a = new Int32Array(data);
            switch (a[0]) {
            case 0:
                /* setTimeout(() => {
                    self._resolveLoader(true); }, 5000); -- test */
                self._resolveLoader(true);
                break;
            case SIZE_PREPARED: // width, height
                if (on[SIZE_PREPARED])
                    on[SIZE_PREPARED].call(self, a[3], a[4]);
                break;
            case AREA_PREPARED: {
                // pixbuf at index 6 of the int array.
                const pixbuf = $unpack(data, 24, 'p')[0];
                self._pixbuf = pixbuf.notNull();
                if (on[AREA_PREPARED])
                    on[AREA_PREPARED].call(self);
            }
                break;
            case AREA_UPDATED: // x, y, width, height
                if (on[AREA_UPDATED])
                    on[AREA_UPDATED].call(self, a[1], a[2], a[3], a[4]);
                break;
            case CLOSED:
                if (on[CLOSED])
                    on[CLOSED].call(self);
                break;
            }
        }).then(() => {
            // closed
            this._loader = null;
        });
    }

    // Returns null if enough data has not been read to determine how to create the image buffer.
    // Should be called after area-prepared signal has been emitted.
    get pixbuf() {
        if (this._pixbuf !== null) {
            const p = new Pixbuf();
            p._init(this._pixbuf);
            return p;
        }
        return null;
    }

    write(data, final = false) {
        // data - ArrayBuffer or TypedArray
        return this._loaderPr.then(() => {
            let status = gLoader.write(this._loader, data, data.byteLength, $nullptr);
            if (final && status)
                status = gLoader.close(this._loader, $nullptr);
            if (!status)
                return Promise.reject(new Error('GdkPixbufLoader error'));
            return true;
        });
    }

    // write in non V8 thread.
    pwrite(data, final = false) {
        // data - ArrayBuffer or TypedArray
        return this._loaderPr.then(() => {
            return $go(gLoader.write, this._loader, data, data.byteLength, $nullptr)
            .then((status) => {
                if (final && status)
                    status = gLoader.close(this._loader, $nullptr);
                if (!status)
                    return Promise.reject(new Error('GdkPixbufLoader error'));
                return true;
            });
        });
    }

    // frees internal structures; It is ok to call it more than once.
    close() {
        return this._loaderPr.then(() => {
            if (this._loader !== null && !gLoader.close(this._loader, $nullptr))
                return Promise.reject(new Error('GdkPixbufLoader error'));
            return true;
        });
    }

    // Causes the image to be scaled while it is loaded. The desired image size
    // can be determined relative to the original size of the image by calling
    // gdk_pixbuf_loader_set_size() from a signal handler for the
    // ::size-prepared signal.
    // Attempts to set the desired image size are ignored after the emission
    // of the ::size-prepared signal.
    setSize(width, height) {
        gLoader.set_size(this._loader, width|0, height|0);
    }
}


///*
// Test

const fs = require('../fs-async.js');

const png = './rect.png';

let pl = new PixbufLoader({
    closed: () => { $print('done !!'); },
    size_prepared: (width, height) => { $print(width, height); },
    area_updated: (x, y, w, h) => { $print(x, y, w, h); },
    area_prepared: function() {
        const p = this.pixbuf;
        $print('Pixbuf=', p.width, p.height);
    }
});

// **** must always close() ****
async function loadImage(imgfile, p) {
    try {
        let data = await fs.readFile(imgfile);
        // data is ArrayBuffer
        $print('data length=', data.byteLength);
        await p.pwrite(data, true);
    } catch (e) {
        $print(e);
    } finally {
        p.close();
    }
}

loadImage(png, pl);
//*/

