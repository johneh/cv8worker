const sqlite3lib = $loadlib('./sqlite3/libsql3.so');

function Statement(stmt) {
    this._stmt = stmt;
}

Statement.prototype.finalize = function () {
    if (this._stmt === null) {
        // return Promise.reject(new Error('statement already finalized'));
        return Promise.resolve(true);
    }
    let stmt = this._stmt;
    this._stmt = null;
    return $go(sqlite3lib.finalize, stmt)
    .then((data) => {
        return true; // Promise.resolve(true);
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
            format += 's';
            buflen += params[i].length;
        } else  // FIXME 'a'
            return Promise.reject(new TypeError('unsupported bind param'));
    }

    let bufp = $malloc($nullptr.packSize('p', $nullptr)+format.length+1+buflen).gc();
    let offset = bufp.pack(0, 'ps', this._stmt, format);
    for(let i = 0; i < format.length; i++) {
        offset += bufp.pack(offset, format[i], params[i]);
    }
    return $go(sqlite3lib.bind, bufp);
};

// step + fetch
Statement.prototype.next = function () {
    return $go(sqlite3lib.next, this._stmt)
    .then((data) => {
        if (data === $nullptr)  // done
            return Promise.resolve(null);
        const [ colTypes, count ] = data.unpack(0, 's');
        //if (colTypes.length == 0)   // empty result set
        //    return Promise.resolve(undefined); // Likely SQLITE_DONE

        // unpack returns null if format == ''!
        const row = data.unpack(count, colTypes);
        row.pop();
        return Promise.resolve(row);
    })
    .catch ((err) => {
        return Promise.reject(err);
    });
};

Statement.prototype.each = function (callback, maxRows) {
    if (typeof callback !== 'function')
        Promise.reject(new TypeError('CALLBACK is not a function'));
    if (typeof maxRows === 'undefined') {
        maxRows = 4294967295;   // UINT32_MAX: Math.pow(2, 32) - 1
    } else {
        maxRows = ~~maxRows;
        if (maxRows < 0)
            maxRows = 0;
    }

    let batchRows = 50; /* yield after every 50 rows */
    const ptr = $malloc($nullptr.packSize('pII', $nullptr, 0, 0)).gc();
    ptr.pack(0, 'pII', this._stmt, maxRows, batchRows);
    return $go(sqlite3lib.each, ptr, (data, done) => {
                if (! done) {
                    const [ colTypes, count ] = data.unpack(0, 's');
                    const row = data.unpack(count, colTypes);
                    row.pop();
                    callback(row);
                }
        }
    )
//    .then((finalResult) => {
//        return finalResult;
//    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

function Database(db) {
    this._db = db;
}

Database.prototype.prepare = function (query) {
    if (typeof query !== 'string')
        return Promise.reject(new TypeError('QUERY must be a string'));
    const ptr = $malloc($nullptr.packSize('pa', $nullptr, query)).gc();
    ptr.pack(0, 'pa', this._db, query);
    return $go(sqlite3lib.prepare, ptr)
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
        return Promise.resolve(true);
    }

    let db = this._db;
    this._db = null;
    return $go(sqlite3lib.close, db)
    .then((data) => {
        return true; // Promise.resolve(true);
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

exports.open = function (filename) {
    if (typeof filename !== 'string')
        return Promise.reject(new TypeError('FILNAME must be a string'));
    const ptr = $malloc(filename.length+1).gc();
    ptr.pack(0, 's', filename);
    return $go(sqlite3lib.open, ptr)
    .then((data) => {
        return new Database(data);
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};
