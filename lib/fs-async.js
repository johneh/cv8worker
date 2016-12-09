const fs = $loadlib('./fs/libfs.so');

function readFile(filename) {
    if (typeof filename !== 'string')
        return Promise.reject(new TypeError('FILENAME must be a string'));
    let p1 = $malloc(filename.length+1).gc();
    p1.pack(0, 's', filename);
    return $go(fs.readfile_a, p1)
    .then((data) => {
        return data.utf8String();
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

//
// Example:
// readFile('./testfs.js')
// .then((data) => {
//     $print(data);
// })
// .catch((err) => {
//     $print(err);
// });
//


function open(filename, flags) {
    if (typeof filename !== 'string')
        return Promise.reject(new TypeError('FILENAME is not a string')); 
    if (typeof flags !== 'string')
        return Promise.reject(new TypeError('FLAGS is not a string'));
    flags = fs.str2flag(flags);
    let mode = 0666;
    let p1 = $malloc($nullptr.packSize('iiis', 0, 0, 0, filename)).gc();
    p1.pack(0, 'iiis', -1, mode, flags, filename);
    return $go(fs.open_a, p1)
    .then((data) => {
        return p1.unpack(0, 'i')[0];  // file descriptor
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

function pread(fd, offset, count) {
    if ((fd|0) !== fd || fd < 0)
        return Promise.reject(new TypeError('FD is not a positive integer'));
    if ((offset|0) !== offset || offset < 0)
        return Promise.reject(new TypeError('OFFSET is not a positive integer'));
    if ((count|0) !== count || count < 0)
        return Promise.reject(new TypeError('COUNT is not a positive integer'));
    let p1 = $malloc($nullptr.packSize('iII', 0, 0, 0)).gc();
    p1.pack(0, 'iII', fd, offset, count);
    return $go(fs.pread_a, p1)
    .then((data) => {
        return data;    // pointer (== $nullptr if bytes_read == 0)
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

function pwrite(fd, offset, data) {
    if ((fd|0) !== fd || fd < 0)
        return Promise.reject(new TypeError('FD is not a positive integer'));
    if ((offset|0) !== offset || offset < 0)
        return Promise.reject(new TypeError('OFFSET is not a positive integer'));
    if ($length(data) === null || $length(data) < 0)
        return Promise.reject(new TypeError('DATA type is incompatible'));
    let p1 = $malloc($nullptr.packSize('iIa', 0, 0, data)).gc();
    p1.pack(0, 'iIa', fd, offset, data);
    return $go(fs.pwrite_a, p1)
    .then((x) => {
        return p1.unpack(0, 'i')[0];    // bytes written >= 0
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

function close(fd) {
    if ((fd|0) !== fd || fd < 0)
        return Promise.reject(new TypeError('FD is not a positive integer'));
    let p1 = $malloc($nullptr.packSize('i', 0)).gc();
    p1.pack(0, 'i', fd);
    return $go(fs.close_a, p1)
    .then((data) => {
        return 0;
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

function unlink(filename) {
    if (typeof filename !== 'string')
        return Promise.reject(new TypeError('FILENAME is not a string'));
    let p1 = $malloc($nullptr.packSize('s', filename)).gc();
    p1.pack(0, 's', filename);
    return $go(fs.unlink_a, p1)
    .then((data) => {
        return 0;
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}


exports.readFile = readFile;
exports.open = open;
exports.pread = pread;
exports.pwrite = pwrite;
exports.close = close;
exports.unlink = unlink;

