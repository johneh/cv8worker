#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include "libpill.h"
#include "jsv8dlfn.h"

static char *readfile(const char *filename, size_t *len);

static coroutine void
do_readfile(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    char *filename = V8_TOSTR(args[0]);
    size_t sz;
    // FIXME -- limit to UINT32_MAX
    char *data = readfile(filename, &sz);
    if (!data)
        jsv8->goreject(vm, cr, strerror(errno));
    else
        jsv8->goresolve(vm, cr, V8_BUFFER(data, sz), 1);
}

static coroutine void
do_open(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    int mode = V8_TOINT32(args[2]);
    int flags = V8_TOINT32(args[1]);
    char *filename = V8_TOSTR(args[0]);
    int fd = open_a(filename, flags, mode);
    if (fd < 0)
        jsv8->goreject(vm, cr, strerror(errno));
    else
        jsv8->goresolve(vm, cr, V8_INT32(fd), 1);
}

static coroutine void
do_pwrite(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    int fd = V8_TOINT32(args[0]);
    unsigned offset = V8_TOUINT32(args[1]);
    unsigned count = V8_TOUINT32(args[3]);
    void *buf;
    if (V8_ISSTRING(args[2]))
        buf = V8_TOSTR(args[2]);
    else
        buf = V8_TOPTR(args[2]);
    ssize_t sz = pwrite_a(fd, buf, count, offset);
    if (sz < 0)
        jsv8->goreject(vm, cr, strerror(errno));
    else
        jsv8->goresolve(vm, cr, V8_UINT32((uint32_t) sz), 1);
}

static coroutine void
do_pread(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    int fd = V8_TOINT32(args[0]);
    unsigned offset = V8_TOUINT32(args[1]);
    unsigned count = V8_TOUINT32(args[2]);
    void *buf = malloc(count+1);
    if (!buf) {
        jsv8->goreject(vm, cr, strerror(ENOMEM));
        return;
    }
    ssize_t sz = pread_a(fd, buf, count, offset);
    if (sz < 0) {
        free(buf);
        jsv8->goreject(vm, cr, strerror(errno));
    } else {
        if (sz == 0) {
            free(buf);
            buf = NULL;
        }
        jsv8->goresolve(vm, cr, V8_BUFFER(buf, (int) sz), 1);
    }
}

static coroutine void
do_close(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    int fd = V8_TOINT32(args[0]);
    int rc = close_a(fd);
    if (rc < 0)
        jsv8->goreject(vm, cr, strerror(errno));
    else
        jsv8->goresolve(vm, cr, V8_VOID, 1);
}

static coroutine void
do_unlink(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    char *filename = V8_TOSTR(args[0]);
    int rc = unlink_a(filename);
    if (rc < 0)
        jsv8->goreject(vm, cr, strerror(errno));
    else
        jsv8->goresolve(vm, cr, V8_VOID, 1);
}

static char *
readfile(const char *filename, size_t *len) {
    if (!filename || !*filename) {
        errno = EINVAL;
        return NULL;
    }
    int fd = open(filename, 0, 0666);
    if (fd < 0)
        goto er;
    struct stat sbuf;
    if (fstat(fd, & sbuf) != 0)
        goto er;
    if (S_ISDIR(sbuf.st_mode)) {
        errno = EISDIR;
        goto er;
    }
    size_t size = sbuf.st_size;
    char *buf = malloc(size + 1);
    if (!buf) {
        errno = ENOMEM;
        goto er;
    }
    if (read(fd, buf, size) != size) {
        free(buf);
        goto er;
    }
    close(fd);
    buf[size] = '\0';
    *len = size;
    return buf;
er:
    if (fd >= 0)
        close(fd);
    return NULL;
}

static v8_val
do_str2flag(v8_state vm, int argc, v8_val argv[]) {
    char *mode = V8_TOSTR(argv[0]);
    int ret = -1;
    if (! mode || ! mode[0]) {
        goto finish;
    }
    char c2 = mode[1];
    switch (mode[0]) {
    case 'r':
        if (c2 == '+' || c2 == 'w')
            ret = O_RDWR;
        else
            ret = O_RDONLY;
        break;
    case 'w':
        if (c2 == '+' || c2 == 'r')
            ret = O_RDWR|O_CREAT|O_TRUNC;
        else
            ret = O_WRONLY|O_CREAT|O_TRUNC;
        break;
    case 'a':
        if (c2 == '+')
            ret = O_RDWR|O_APPEND|O_CREAT;
        else
            ret = O_WRONLY|O_APPEND|O_CREAT;
        break;
    default:
        break;
    }
finish:
    return V8_INT32(ret);
}


static v8_ffn ff_table[] = {
    {1, do_str2flag, "str2flag", FN_CTYPE},
    {1, do_readfile, "readfile_a", FN_CORO},
    {3, do_open, "open_a", FN_CORO},
    {3, do_pread, "pread_a", FN_CORO},
    {4, do_pwrite, "pwrite_a", FN_CORO},
    {1, do_close, "close_a", FN_CORO},
    {1, do_unlink, "unlink_a", FN_CORO},
    {0},
};

int JS_LOAD(v8_state vm, v8_val hlib) {
    JS_EXPORT(ff_table);
}

