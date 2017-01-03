const fs = $loadlib('./fs/libfs.so');

function readFile(filename) {
    if (typeof filename !== 'string' || filename.length === 0)
        return Promise.reject(new TypeError('FILENAME must be a non-empty string'));
    return $go(fs.readfile_a, p1)
    .then((data) => {
        return data;    // ArrayBuffer
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

function open(filename, flags) {
    if (typeof filename !== 'string' || filename.length === 0)
        return Promise.reject(new TypeError('FILENAME is not a non-empty string')); 
    if (typeof flags !== 'string')
        return Promise.reject(new TypeError('FLAGS is not a string'));
    flags = fs.str2flag(flags);
    let mode = 0666;
    let p1 = $malloc($nullptr.packSize('iiis', 0, 0, 0, filename)).gc();
    return $go(fs.open_a, filename, flags, mode)
    .then((fd) => {
        return fd;
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
    return $go(fs.pread_a, fd, offset, count)
    .then((data) => {
        return data;    // ArrayBuffer
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
    const dataBytes = $length(data);
    if (dataBytes === null || dataBytes < 0)
        return Promise.reject(new TypeError('invalid argument for DATA'));
    return $go(fs.pwrite_a, fd, offset, data, dataBytes)
    .then((bytesWritten) => {
        return bytesWritten;    // bytes written >= 0
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

function close(fd) {
    if ((fd|0) !== fd || fd < 0)
        return Promise.reject(new TypeError('FD is not a positive integer'));
    return $go(fs.close_a, fd)
    .then(() => {
        return 0;
    })
    .catch((err) => {
        return Promise.reject(err);
    });
}

function unlink(filename) {
    if (typeof filename !== 'string' || filename.length === 0)
        return Promise.reject(new TypeError('FILENAME is not a non-empty string'));
    return $go(fs.unlink_a, filename)
    .then(() => {
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

