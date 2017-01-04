const Buffer = require('./buffer.js').Buffer;
const sqlite3lib = $loadlib('./sqlite3/libsql3.so');

const _unpack = function(b, offset, format) {
    return $unpack(b, offset, format);
};

function Statement(stmt) {
    this._stmt = stmt;
}

Statement.prototype.finalize = function () {
    if (this._stmt === null) {
        // return Promise.reject(new Error('statement already finalized'));
        return Promise.resolve(false);
    }
    let stmt = this._stmt;
    this._stmt = null;
    return $go(sqlite3lib.finalize, stmt)
    .then(() => {
        return true;
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

Statement.prototype.bind = function (...params) {
    let format = '';
    let buflen = 0;
    for (let i = 0; i < params.length; i++) {
        if (params[i] === null) {
            format += '_';
        } else if (typeof params[i] === 'number') {
            if ((params[i]|0) === params[i]) {
                format += 'i';
                buflen += 4;
            } else {
                format += 'd';
                buflen += 8;
            }
        } else if (typeof params[i] === 'string') {
            // FIXME pass size + bytes 
            format += 's';
            buflen += (params[i].length + 1);
        } else {
            const bLen = $length(params[i]);
            if (bLen !== null && bLen >= 0) {
                format += 'a';  // blob
                buflen += (bLen + 4);
            } else
                return Promise.reject(new TypeError('unsupported bind parameter'));
        }
    }

    let offset = 0;
    let buf = new ArrayBuffer(buflen);
    for(let i = 0; i < format.length; i++) {
        offset += $pack(buf, offset, format[i], params[i]);
    }
    return $go(sqlite3lib.bind, this._stmt, format, buf);
};

// step + fetch
Statement.prototype.next = function () {
    return $go(sqlite3lib.next, this._stmt)
    .then((data) => {
        if (data === null)  // done
            return Promise.resolve(null);
        // data is ArrayBuffer
        const [ colTypes, count ] = _unpack(data, 0, 's');
        //if (colTypes.length == 0)   // empty result set
        //    return Promise.resolve(undefined); // Likely SQLITE_DONE

        // unpack returns null if format is empty string ('')!
        const row = _unpack(data, count, colTypes);
        row.pop();
        return Promise.resolve(row);
    })
    .catch ((err) => {
        return Promise.reject(err);
    });
};

const MAX_ROWS = 4294967295;   /* UINT32_MAX: Math.pow(2, 32) - 1 */
const BATCH_SIZE = 50;  /* yield after every BATCH_SIZE rows */

Statement.prototype.each = function (callback, maxRows) {
    if (typeof callback !== 'function')
        Promise.reject(new TypeError('CALLBACK is not a function'));
    if (typeof maxRows === 'undefined') {
        maxRows = MAX_ROWS;
    } else {
        maxRows = ~~maxRows;
        if (maxRows < 0)
            maxRows = 0;
    }

    let batchSize = BATCH_SIZE;
    let numRows = 0;
    return $go(sqlite3lib.each, this._stmt, maxRows, batchSize,
            (data, done) => {
                if (! done) {
                    const [ colTypes, count ] = _unpack(data, 0, 's');
                    const row = _unpack(data, count, colTypes);
                    row.pop();
                    numRows++;
                    callback(row);
                } else
                    return numRows;
        }
    );
};

function Database(db) {
    this._db = db;
}

Database.prototype.prepare = function (query) {
    if (this._db === null)
        return Promise.reject(new Error('Database is not open'));
    if (typeof query !== 'string')
        return Promise.reject(new TypeError('QUERY must be a string'));
    return $go(sqlite3lib.prepare, this._db, query, $length(query))
    .then((data) => {
        return new Statement(data);
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};
 
Database.prototype.close = function () {
    if (this._db === null) {
        // return Promise.reject(new Error('database already closed'));
        return Promise.resolve(false);
    }

    let db = this._db;
    this._db = null;
    return $go(sqlite3lib.close, db)
    .then(() => {
        return true;
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

exports.open = function (filename) {
    if (typeof filename !== 'string' || filename.length === 0)
        return Promise.reject(new TypeError('FILENAME must be a non-empty string'));
    return $go(sqlite3lib.open, filename)
    .then((data) => {
        return new Database(data);
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};
