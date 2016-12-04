var httpfuncs = $loadlib("./http/libhttp.so");


/*****************************************************************************
 * HTTP Client
 *****************************************************************************/

function nextLine(h) {
    let s = h._htext;
    let i = h._offset;
    let j = s.indexOf('\n', i);
    if (j === -1)
        return '';
    h._offset = j+1;
    if (j > 0 && s.charAt(j-1) === '\r')
        j--;
    return s.substring(i, j);
}


function Headers(htext) {
    if (typeof(htext) !== 'string')
        throw new TypeError('Headers(): string argument expected');
    this._htext = htext;
    this._offset = 0;
    let line = nextLine(this);
    let mr;
    if (!line || (mr = /^HTTP\/(\d)\.(\d) (\d{3}) ?(.*)$/.exec(line)) === null)
        throw new Error('invalid response header');
    this.RESPONSE_LINE = line;
    this.STATUS = mr[3];
}

function get_header(name, s) {
    if (typeof (name) !== 'string')
        throw new TypeError('string expected for header name');
    name = name.toUpperCase();
    let nameLen = name.length;
    if (nameLen === 0)
        return -1;
    for (let j = 0;; j++) {
        j = s.indexOf(':', j);
        if (j === -1)
            return -1;
        let i = s.lastIndexOf('\n', j);
        if (i === -1)
            return -1;
        i++;
        if ((j-i) === nameLen
                && name === s.substring(i, j).toUpperCase()) {
            return j+1;
        }
    }
    return -1;
}

Headers.prototype.get = function(name) {
    let s = this._htext;
    let j = get_header(name, s);
    let i = -1;
    if (j === -1 || (i = s.indexOf('\r', j)) === -1)
        return null;
    return s.substring(j, i).trim();
};

Headers.prototype.text = function() {
    return this._htext;
};

//Object.defineProperty(Headers.prototype, "status", {
//    get: function () {
//        return this.STATUS;
//    }
//});

function Response(url) {
    this.url = url;
    this._sockp = null;
    this._headerPr = null;
    this._headers = null;
    // this._bodyPr = null;
}

// undefined if header has not been read (i.e. the header promise resolved) yet
Object.defineProperty(Response.prototype, "status", {
    get: function () {
        if (this._headers !== null)
            return this._headers.STATUS;
    }
});

Response.prototype.headers = function () {
    let self = this;
    if (self._headerPr === null) {
        self._headerPr = $go(httpfuncs.header, self._sockp);
        self._headerPr.then(function (data) {
            self._headers = new Headers(data.utf8String());
        });
    }

    return Promise.resolve(self._headerPr)
    .then((data) => {
        return self._headers;
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

//
// async function(url) {
//    var resp = await fetch(url);
//    var h = await resp.headers();
//    var count = 0;
//    var bytesReceived = await resp.onBody(function(data, done) {
//        count += data.sizeOf();
//        if (done)
//            return count;
//    });
//    console.log(`Content-Length: ${resp.url}:`, bytesReceived);
// }
//

Response.prototype.onBody = function (callback) {
    let self = this;
    if (typeof callback !== 'function')
        throw new TypeError('callback is not a function');
    // make sure headers promise resolved
    return self.headers()
    .then((data) => {
        let p = $go(httpfuncs.readp, self._sockp, callback);
        return p.then((finalResult) => {
            // console.log('[==== Done: closing fd ====]');
            httpfuncs.closefd(self._sockp);
            return finalResult;
        })
        .catch((err) => {
            return Promise.reject(err);
        });
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};


Response.prototype.body = function () {
    let self = this;
    // make sure headers promise resolved
    return self.headers()
    .then((data) => {
        let p = $go(httpfuncs.reada, self._sockp);
        return p.then((data) => {
            // console.log('[==== Done: closing fd ====]');
            httpfuncs.closefd(self._sockp);
            return data;
        })
        .catch((err) => {
            return Promise.reject(err);
        });    
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

//  readBodyData()
//  --------------
//  returns a promise that resolves in a objects containing two properties:
//      done  - true if no more data available.
//      value - actual data (pointer); maybe $nullptr if done is true.
//
//  Usage:
//      let bytesReceived = 0;
//      resp.readBodyData()
//      .catch(function perror(err) {
//          console.log(err);
//      })
//      .then(function process(result) {
//          bytesReceived += $length(result.value);
//          if (! result.done) {
//              return resp.readBodyData().then(process).catch(perror);
//          }
//      });
//

Response.prototype.readBodyData = function () {
    let self = this;
    return self.headers()
    .then((data) => {
        let p = $go(httpfuncs.readb, self._sockp);
        return p.then((result) => {
            if (result.done) {
                // console.log('[==== Done: closing fd ====]');
                httpfuncs.closefd(self._sockp);
            }
            return Promise.resolve(p);
        })
        .catch((err) => {
            return Promise.reject(err);
        });
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

function fetch(req) {
    let host, port;
    let path;
    let schema = 'http';

    if (req === null || typeof req !== 'object')
        req = {};
    port = ~~req.port;
    host = req.host || 'localhost';
    host += '';
    path = req.path || '/';
    path += '';
    if (req.schema === 'https') {
        schema = 'https';
        port = port || 443;
    } else
        port = port || 80;

    let sockp = httpfuncs.create(host, port, -1,
                        schema === 'https').notNull();
    sockp.gc(httpfuncs.free);

    return $go(httpfuncs.connect, sockp)
    .then(function (data) {
        let s = `GET ${path} HTTP/1.1\r\nHost: ${host}\r\nConnection: close\r\n\r\n`;
        httpfuncs.setwbuf(sockp, s, $length(s));
        return $go(httpfuncs.send, sockp);
    })
    .then((data) => {
        let r = new Response(`${schema}://${host}${path}`);
        r._sockp = sockp;
        return Promise.resolve(r);
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

exports.fetch = fetch;


/*****************************************************************************
 * HTTP Server
 *****************************************************************************/

function httpRequest(c) {
    this._sockp = c;
    this._headers = null;
    this._headerPr = null;
}

httpRequest.prototype.headers = function () {
    let self = this;
    if (self._headerPr === null) {
        self._headerPr = $go(httpfuncs.header, self._sockp);
        self._headerPr.then(function (data) {
            self._headers = data.utf8String();
        });
    }
    return Promise.resolve(self._headerPr)
    .then((data) => {
        return self._headers;
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};

function httpResponse(c) {
    this._sockp = c;
    this._lastChunk = null;
    this._chunks = [];
    this._endResolve = null;
    this._endReject = null;
    this._endPr = null;
    this._err = null;   /* write error */
    this.bytesSent = 0;
}

function _writeChunk(self) {
    if (self._err !== null) {
        // XXX: likely unnecessary
        if (self._endPr !== null)
            self._endReject(self._err);
        return;
    }
    if (self._lastChunk !== null)
        return;
    if (self._chunks.length > 0) {
        let chunk = self._chunks.shift();
        httpfuncs.setwbuf(self._sockp, chunk, $length(chunk));
        self._lastChunk = chunk;
        $go(httpfuncs.send, self._sockp)
        .then(function (data) {
            self.bytesSent += $length(chunk);
            self._lastChunk = null;
            _writeChunk(self);
        })
        .catch(function (err) {
            // self._lastChunk = null;
            self._err = err;
            console.log(err);
            if (self._endPr !== null)
                self._endReject(err);
        });
    } else if (self._endPr !== null) {
        /* self._chunks.length == 0 */
        httpfuncs.closefd(self._sockp);
        self._endResolve(self.bytesSent);
    }
}

httpResponse.prototype.end = function (chunk) {
    let self = this;

    if (self._err !== null)
        return Promise.reject(self._err);

    if (typeof chunk === 'undefined') {
        if (self._lastChunk === null && self._chunks.length === 0) {
            httpfuncs.closefd(resp._sockp);
            return Promise.resolve(self.bytesSent);
        }
    } else if ($length(chunk) === null || $length(chunk) < 0) {
        return Promise.reject(new TypeError('invalid argument'));
    } else {
        // string, pointer, or arraybuffer(view)
        self._chunks.push(chunk);
    }
    _writeChunk(self);
    if (self._endPr !== null)
        return Promise.resolve(self._endPr);
    return (self._endPr = new Promise(resolve => {
                self._endResolve = resolve;
            }, reject => {
                self._endReject = reject;
            })
        );
};

httpResponse.prototype.write = function (chunk) {
    if ($length(chunk) === null || $length(chunk) < 0)
        throw new TypeError('invalid argument');
    if (this._err !== null)
        throw this._err;
    this._chunks.push(chunk);
    _writeChunk(this);
};

function httpServer(port, hostname) {
    this.port = ~~port;
    if (typeof hostname !== 'string')
        throw new TypeError('hostname is not a string');
    this.hostname = hostname;   // '' -> bind to all interfaces.
    this.running = false;
}

httpServer.prototype.start = function (callback) {
    let self = this;
    if (self.running)
        throw new Error('server already running');
    let p = $malloc($nullptr.packSize('iiiis', 0,0,0,0,'')).gc();
    p.pack(0, 'iiiis', self.port, 0, 32, 0, self.hostname);
    self.running = true;
    $go(httpfuncs.listen_and_accept, p, function(client, done) {
        client.gc(httpfuncs.free);
        callback(new httpRequest(client), new httpResponse(client));
    })
    .catch((err) => {
        console.log(err);
    });
};

function createServer(port, hostname = '') {
    return new httpServer(port, hostname);
}

exports.createServer = createServer;

