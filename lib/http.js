'use strict';

const httpfuncs = $loadlib("./http/libhttp.so");

let LOGERR = console.log;

function HTTPMessage(c) {
    this._sockp = c;
    this._headerPr = null;
    this._headers = null;
}

HTTPMessage.prototype.setDeadline = function (ddline) {
    httpfuncs.set_deadline(this._sockp, ddline);
};

HTTPMessage.prototype.headers = function () {
    const self = this;
    if (self._headerPr === null) {
        self._headerPr = $go(httpfuncs.header, self._sockp);
        self._headerPr.then((data) => {
            self._headers = data;   // string
        });
    }

    return Promise.resolve(self._headerPr)
    .then((data) => {
        return self._headers;
    })
    .catch((err) => {
        LOGERR(err);
        httpfuncs.closefd(self._sockp);
        return Promise.reject(err);
    });
};


//
// async function(url) {
//    var resp = await fetch(url);
//    var h = await resp.headers();
//    var count = 0;
//    var bytesReceived = await resp.onBody(function(data, done) {
//        count += $length(data);
//        if (done)
//            return count;
//    });
//    console.log(`Content-Length: ${resp.url}:`, bytesReceived);
// }
//

HTTPMessage.prototype.onBody = function (callback) {
    const self = this;
    if (typeof callback !== 'function')
        throw new TypeError('CALLBACK is not a function');
    // make sure headers promise resolved
    return self.headers()
    .then((data) => {
        return $go(httpfuncs.readp, self._sockp, callback)
        .then((finalResult) => {
            httpfuncs.closefd(self._sockp);
            return finalResult;
        })
        .catch((err) => {
            LOGERR(err);
            httpfuncs.closefd(self._sockp);
            return Promise.reject(err);
        });
    })
    .catch((err) => {
        // XXX: already closed ?
        return Promise.reject(err);
    });
};

HTTPMessage.prototype.body = function () {
    const self = this;
    // make sure headers promise resolved
    return self.headers()
    .then((data) => {
        const p = $go(httpfuncs.reada, self._sockp);
        return p.then((data) => {
            httpfuncs.closefd(self._sockp);
            return data;
        })
        .catch((err) => {
            LOGERR(err);
            httpfuncs.closefd(self._sockp);
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

HTTPMessage.prototype.readBodyData = function () {
    const self = this;
    return self.headers()
    .then((data) => {
        const p = $go(httpfuncs.readb, self._sockp);
        return p.then((result) => {
            if (result.done) {
                httpfuncs.closefd(self._sockp);
            }
            return Promise.resolve(p);
        })
        .catch((err) => {
            LOGERR(err);
            httpfuncs.closefd(self._sockp);
            return Promise.reject(err);
        });
    })
    .catch((err) => {
        return Promise.reject(err);
    });
};


const _nextLine = function (h) {
    const s = h._htext;
    const i = h._offset;
    let j = s.indexOf('\n', i);
    if (j === -1)
        return '';
    h._offset = j+1;
    if (j > 0 && s.charAt(j-1) === '\r')
        j--;
    return s.substring(i, j);
};

const METHODS = [
  'DELETE',
  'GET',
  'HEAD',
  'POST',
  'PUT',
  'CONNECT',
  'OPTIONS',
  'TRACE'
];


HTTPParser.prototype.REQUEST_LINE = function (line) {
    let mr;
    if (!line || (mr = /^([A-Z-]+) ([^ ]+) HTTP\/(\d)\.(\d)$/.exec(line)) === null)
        throw new Error('invalid Request-Line');
    if (METHODS.indexOf(mr[1]) === -1)
        throw new Error('invalid request method');
    this.method = mr[1];
    this.uri = mr[2];
    this.version = mr[3] + '.' + mr[4];
};

HTTPParser.prototype.STATUS_LINE = function (line) {
    let mr;
    if (!line || (mr = /^HTTP\/(\d)\.(\d) (\d{3}) ?(.*)$/.exec(line)) === null)
        throw new Error('invalid Status-Line');
    this.statusCode = mr[3];
    this.version = mr[1] + '.' + mr[2];
    this.reasonPhrase = mr[4];
};

function HTTPParser(htext) {
    if (typeof htext !== 'string')
        throw new TypeError('string argument expected');
    this._htext = htext;
    this._offset = 0;
    const line = _nextLine(this);
    try {
        this.STATUS_LINE(line);
    } catch (e) {
        this.REQUEST_LINE(line);
    }
}

const _findHeader = function (name, s) {
    if (typeof name !== 'string')
        throw new TypeError('string expected for header NAME');
    name = name.toUpperCase();
    const nameLen = name.length;
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
};

HTTPParser.prototype.get = function(name) {
    const s = this._htext;
    const j = _findHeader(name, s);
    let i = -1;
    if (j === -1 || (i = s.indexOf('\r', j)) === -1)
        return null;
    return s.substring(j, i).trim();
};

HTTPParser.prototype.text = function() {
    // FIXME -- strip request(status) line (and trailing NEWLINE ?)
    return this._htext;
};


exports.parse = function (message_header) {
    return new HTTPParser(message_header);
};

/*****************************************************************************
 * HTTP Client
 *****************************************************************************/

function FetchResponse(c, url) {
    this.url = url;
    HTTPMessage.call(this, c);
}

FetchResponse.prototype = Object.create(HTTPMessage.prototype);

const CONNECT_TIMEOUT = 120000; /* 2 minutes default for dns lookup and connect */
// FIXME -- currently timeout also active for write

// TODO req.connectTimeout ..
const fetch = function (req) {
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

    const sockp = httpfuncs.create(host, port, $now() + CONNECT_TIMEOUT,
                        schema === 'https').notNull();
    sockp.gc(httpfuncs.free);

    return $go(httpfuncs.connect, sockp)
    .then(() => {
        const s = `GET ${path} HTTP/1.1\r\nHost: ${host}\r\nConnection: close\r\n\r\n`;
        return $go(httpfuncs.send, sockp, s, $length(s));
    })
    .then((data) => {
        let m = new FetchResponse(sockp, `${schema}://${host}${path}`);
        m.setDeadline(-1);
        return Promise.resolve(m);
    })
    .catch((err) => {
        LOGERR(err);
        httpfuncs.closefd(sockp);
        return Promise.reject(err);
    });
};

exports.fetch = fetch;


/*****************************************************************************
 * HTTP Server
 *****************************************************************************/

function HTTPResponse(c) {
    this._sockp = c;
    this._lastChunk = null;
    this._chunks = [];
    this._endResolve = null;
    this._endReject = null;
    this._endPr = null;
    this._err = null;   /* write error */
    this.bytesSent = 0;
}

const _writeChunk = function (self) {
    if (self._err !== null) {
        // XXX: likely unnecessary
        if (self._endPr !== null)
            self._endReject(self._err);
        return;
    }
    if (self._lastChunk !== null)
        return;
    if (self._chunks.length > 0) {
        const chunk = self._chunks.shift();
        self._lastChunk = chunk;
        const chunkLen = $length(chunk);
        $go(httpfuncs.send, self._sockp, chunk, chunkLen)
        .then(() => {
            self.bytesSent += chunkLen;
            self._lastChunk = null;
            _writeChunk(self);
        })
        .catch((err) => {
            // self._lastChunk = null;
            self._err = err;
            LOGERR(err);
            httpfuncs.closefd(self._sockp);
            if (self._endPr !== null)
                self._endReject(err);
        });
    } else if (self._endPr !== null) {
        /* self._chunks.length == 0 */
        httpfuncs.closefd(self._sockp);
        self._endResolve(self.bytesSent);
    }
};

HTTPResponse.prototype.end = function (data) {
    const self = this;
    if (self._err !== null)
        return Promise.reject(self._err);

    if (typeof data === 'undefined') {
        if (self._lastChunk === null && self._chunks.length === 0) {
            httpfuncs.closefd(self._sockp);
            return Promise.resolve(self.bytesSent);
        }
    } else if ($length(data) === null || $length(data) < 0) {
        return Promise.reject(new TypeError('incompatible type for DATA'));
    } else {
        // string, pointer, or arraybuffer(view)
        self._chunks.push(data);
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

HTTPResponse.prototype.write = function (data) {
    if ($length(data) === null || $length(data) < 0)
        throw new TypeError('incompatible type for DATA');
    if (this._err !== null)
        throw this._err;
    this._chunks.push(data);
    _writeChunk(this);
};

HTTPResponse.prototype.setDeadline = function (ddline) {
    httpfuncs.set_deadline(this._sockp, ddline);
};

function HTTPServer(port, hostname) {
    this.port = ~~port;
    if (typeof hostname !== 'string')
        throw new TypeError('HOSTNAME is not a string');
    this.hostname = hostname;   // '' -> bind to all interfaces.
    this.running = false;
    this.TIMEOUT = 120 * 1000;  /* default in millisecs */
}

HTTPServer.prototype.start = function (callback) {
    if (typeof callback !== 'function')
        throw new TypeError('CALLBACK is not a function');
    const self = this;
    if (self.running)
        throw new Error('server already running');
    self.running = true;
    $go(httpfuncs.listen_and_accept, self.port, 0, 32, 0, self.hostname,
        function(csock, done) {
            csock.gc(httpfuncs.free);
            httpfuncs.set_deadline(csock, $now() + self.TIMEOUT);
            callback(new HTTPMessage(csock), new HTTPResponse(csock));
    })
    .catch((err) => {
        LOGERR(err);
    });
};

exports.createServer = function (port, hostname = '') {
    return new HTTPServer(port, hostname);
};
