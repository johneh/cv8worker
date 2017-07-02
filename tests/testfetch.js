var fetch = require('../lib/http.js').fetch;
var parse = require('../lib/http.js').parse;

fetch({host: 'www.google.com'})
.then((resp) => {
    resp.headers()
    .then((h) => {
//        $print(h);
        return resp.body();
    })
    .then((b) => {
        $print(`Content-Length: ${resp.url}:`, $length(b));
    });
});


fetch({ host: 'news.google.com', path: '/news/?ned=us&hl=en', schema: 'https'} )
.then((resp) => {
    resp.headers()
    .then((h) => {
        // $print(h);
        return resp.body();
    })
    .then((b) => {
        // $print(b.utf8String());
        $print(`Content-Length: ${resp.url}:`, $length(b));
    });
});


async function chunky(url) {
    try {
        let resp = await fetch(url);
        let msg_headers = await resp.headers();
        //$print(msg_headers);
        console.log(`transfer-encoding: ${url.host} ->`,
                parse(msg_headers).get('transfer-encoding'));
        var b = await resp.body();
        $print(`Content-Length: ${resp.url}:`, $length(b));
    } catch (e) {
        $print(url.host, ':', e);
    }
}

chunky({ host: 'thehill.com'});
chunky({ host: 'www.aol.com', schema: 'https' });

(async function (url) {
    try {
        var resp = await fetch(url);
        var msg_headers = await resp.headers();
        var h = parse(msg_headers);
        console.log('STATUS =', h.statusCode);    // -> 200 or ..
        console.log('CONTENT-TYPE =', h.get('Content-Type'));

        var n = 0;
        var total = await resp.onBody((data, done) => {
            // XXX: should use try-catch
			n += $length(data);
			if (done)
				return n;
		});
        $print(`Content-Length: ${resp.url}:`, total);
    } catch (e) {
        $print(url.host, ':', e);
    }
})({host: 'www.google.com'});


// like onBody() but slower
fetch({ host:'www.google.com'})
.then((resp) => {
    let bytesReceived = 0;
    let perror = (err) => {
            console.log(err.stack);
    };

    // headers() is optional (always received first anyway see fetch.js)
    resp.headers()
    .then((h) => {
        // $print(h);
        resp.readBodyData()
        .catch(perror)
        .then(function process(result) {
            bytesReceived += $length(result.value);
            // console.log('Received', bytesReceived, 'bytes of data so far');
            if (result.done) {
                console.log(`Content-Length: ${resp.url}:`, bytesReceived);
                return;
            }
            // Read some more ..
            return resp.readBodyData().then(process).catch(perror);
        });
    });
});
