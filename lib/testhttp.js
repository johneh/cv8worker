let createServer = require('./http.js').createServer;

let BODY = `<!DOCTYPE html>
<html>
<head>
  <title>Test</title>
</head>
<body><p>This is a test.</body>
</html>`;

let HEADER = `HTTP/1.1 200 OK\r
Content-Type: text/html\r
Content-Length: ${BODY.length}\r
Connection: close\r\n\r\n`;

/*
createServer(5556).start((req, resp) => {
    req.headers()
    .then((data) => {
        // console.log(data);
    })
    .then(() => {
        resp.write(HEADER);
        return resp.end(BODY);
    })
    .then((bytesSent) => {
        // console.log(`${bytesSent} bytes sent`);
    });
});
*/ 


createServer(5556).start(async (req, resp) => {
    let h = await req.headers();
    // console.log(h);
    resp.write(HEADER);
    let bytesSent = await resp.end(BODY);
    // console.log(`${bytesSent} bytes sent`);
});
