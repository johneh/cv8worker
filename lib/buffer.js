const Long = require('./long.js').Long;

class Buffer extends Uint8Array {
    constructor(len) {
        let b;
        if (len && ArrayBuffer.prototype.isPrototypeOf(len))
            b = len;
        else
            b = new ArrayBuffer(len);
        super(b);
        this._buffer = b;
        this._littleEndian = true;
        this._view = new DataView(b);
    }

    setInt8(byteOffset, value) {
        this._view.setInt8(byteOffset, value);
    }

    getInt8(byteOffset) {
        return this._view.getInt8(byteOffset);
    }

    setUint8(byteOffset, value) {
        this._view.setUint8(byteOffset, value);
    }

    getUint8(byteOffset) {
        return this._view.getUint8(byteOffset);
    }

    setInt16(byteOffset, value) {
        this._view.setInt8(byteOffset, value, this._littleEndian);
    }

    getInt16(byteOffset) {
        return this._view.getInt16(byteOffset, this._littleEndian);
    }

    setUint16(byteOffset, value) {
        this._view.setUint16(byteOffset, value, this._littleEndian);
    }

    getUint16(byteOffset) {
        return this._view.getUint16(byteOffset, this._littleEndian);
    }

    setInt32(byteOffset, value) {
        this._view.setInt32(byteOffset, value, this._littleEndian);
    }

    getInt32(byteOffset) {
        return this._view.getInt32(byteOffset, this._littleEndian);
    }

    setUint32(byteOffset, value) {
        this._view.setUint32(byteOffset, value, this._littleEndian);
    }

    getUint32(byteOffset) {
        return this._view.getUint32(byteOffset, this._littleEndian);
    }

    setInt64(byteOffset, value) {
        this._view.setInt32(byteOffset, value.low32, this._littleEndian);
        this._view.setInt32(byteOffset + 4, value.high32, this._littleEndian);
    }

    getInt64(byteOffset) {
        let low = this._view.getInt32(byteOffset, this._littleEndian);
        let high = this._view.getInt32(byteOffset + 4, this._littleEndian);
        return Long(low, high, true);
    }

    setUint64(byteOffset, value) {
        this._view.setInt32(byteOffset, value.low32, this._littleEndian);
        this._view.setInt32(byteOffset + 4, value.high32, this._littleEndian);
    }

    getUint64(byteOffset) {
        let low = this._view.getInt32(byteOffset, this._littleEndian);
        let high = this._view.getInt32(byteOffset + 4, this._littleEndian);
        return Long(low, high, false);
    }

    setFloat64(byteOffset, value) {
        this._view.setFloat64(byteOffset, value, this._littleEndian);
    }

    getFloat64(byteOffset) {
        return this._view.getFloat64(byteOffset, this._littleEndian);
    }

    // byte length of old buffer and all its views set to zero
    static realloc(oldBuffer, newByteLength) {
        let oldb;
        if (oldBuffer === null)
            return new Buffer(newByteLength);
        if (oldBuffer && ArrayBuffer.prototype.isPrototypeOf(oldBuffer))
            oldb = oldBuffer;
        else if (oldBuffer && Buffer.prototype.isPrototypeOf(oldBuffer)
            //       && ArrayBuffer.prototype.isPrototypeOf(oldBuffer._buffer)
        )
            oldb = oldBuffer._buffer;
        else
            throw new TypeError('Buffer.realloc: invalid argument #1');

        // FIXME: oldb _must_ not be an argument to a pending $go().

        return new Buffer($transfer(oldb, newByteLength));
    }

    static _utf8String(b, bLen) {
        return $utf8String(b, bLen);
    }

    // FIXME byteOffset argument
    readUtf8(byteLen) {
        if (typeof byteLen === 'undefined') {
            // return string without any embedded null
            if ((this.length !== 0 && this[this.length - 1] === 0)
                || this.lastIndexOf(0) !== -1
            )
                return Buffer._utf8String(this._buffer, -1);
            return Buffer._utf8String(this._buffer, this.length);
        }
        // returned string may contain embedded nulls
        if (byteLen > this.length)
            byteLen = this.length;
        else if (byteLen < 0)
            byteLen = 0;
        return Buffer._utf8String(this._buffer, byteLen);
    }

    // write a (null-terminated) UTF8 encoded string
    writeUtf8(string, byteOffset, nullTerminate = false) {
        if (typeof string !== 'string')
            string = string + '';
        byteOffset = ~~byteOffset;
        if (byteOffset < 0)
            byteOffset = 0;
        let strByteLen = $length(string);
        let endByte = !!nullTerminate;
        if (byteOffset + strByteLen + endByte > this.length)
            throw new RangeError('Buffer.writeUtf8: string is too large');

        // return number of bytes written (>=0). This includes the terminating null,
        // when nullTerminate is true.
        return this.pack(byteOffset, endByte ? 's' : 'S', string);
    }

    // TODO copy(byteOffset, src, srcByteOffest, srcByteCount)
    copy(byteOffset, src) {
        let arr = src;
        if (src && ArrayBuffer.isView(src) && !(src instanceof Uint8Array)) {
            // copy bytes instead of values
            arr = new Uint8Array(src.buffer,
                        src.byteOffset, src.byteLength)
        }
        super.set(arr, byteOffset);
    }

    static _packSize(format, ...args) {
        return $nullptr.packSize(fromat, ...args);
    }

    static _unpack(b, offset, format) {
        return $nullptr.unpack.call(b, offset, format);
    }

    // unsafe
    unpack(offset, format) {
        // FIXME '..J ..' format in general or allow single item and use switch statement
        // (also include 'i', 'I' etc.) 
        if (format === 'J')
            return [ this.getUint64(offset), 8 ];
        if (format === 'j')
            return [ this.getInt64(offset), 8 ];
        if (format === 'p') {
            offset |= 0;
            if (offset < 0 || (offset + Buffer._packSize('p', $nullptr)) > this.byteLength)
                throw new RangeError('Offset is outside the bounds of the Buffer');
            return Buffer._unpack(this._buffer, offset, 'p');
        }

        return Buffer._unpack(this._buffer, offset, format);
    }

    // unsafe
    pack(offset, format) {
        if (format === 'J') {
            this.setUint64(offset, arguments[2]);
            return 8;
        }
        if (format === 'j') {
            this.setInt64(offset, arguments[2]);
            return 8;
        }
        return $nullptr.pack.apply(this._buffer, arguments);
    }
}

exports.Buffer = Buffer;
