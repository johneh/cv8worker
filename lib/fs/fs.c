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
do_readfile(v8_state vm, v8_handle hcr, void *ptr) {
    char *filename = ptr;
    size_t sz;
    char *data = readfile(filename, &sz);
    if (!data)
        v8->goreject(vm, hcr, strerror(errno));
    else
        v8->goresolve(vm, hcr, data, sz, 1);
}

static coroutine void
do_open(v8_state vm, v8_handle hcr, void *ptr) {
    struct open_s {
        int fd;
        int mode;
        int flags;
        char filename[1];
    } __attribute__((packed));
    struct open_s *args = ptr;  /* input and output */
    args->fd = open_a(args->filename, args->flags, args->mode);
    if (args->fd < 0)
        v8->goreject(vm, hcr, strerror(errno));
    else
        v8->goresolve(vm, hcr, (void *) args, -1, 1);
}

static coroutine void
do_pwrite(v8_state vm, v8_handle hcr, void *ptr) {
    struct pwrite_s {
        int fd; /* input and output(bytes written) */
        unsigned offset;
        int count;
        char buf[1];
    } __attribute__((packed));
    struct pwrite_s *args = ptr;
    ssize_t sz = pwrite_a(args->fd, args->buf, args->count, args->offset);
    if (sz < 0)
        v8->goreject(vm, hcr, strerror(errno));
    else {
        args->fd = (int) sz;
        v8->goresolve(vm, hcr, (void *) args, -1, 1);
    }
}

static coroutine void
do_pread(v8_state vm, v8_handle hcr, void *ptr) {
    struct pread_s {
        int fd;
        unsigned offset;
        unsigned count;
    } __attribute__((packed));
    struct pread_s *args = ptr;
    void *buf = malloc(args->count+1);
    if (!buf) {
        v8->goreject(vm, hcr, strerror(ENOMEM));
        return;
    }
    ssize_t sz = pread_a(args->fd, buf, args->count, args->offset);
    if (sz < 0) {
        free(buf);
        v8->goreject(vm, hcr, strerror(errno));
    } else {
        if (sz == 0) {
            free(buf);
            buf = NULL;
        }
        v8->goresolve(vm, hcr, buf, (int) sz, 1);
    }
}

static coroutine void
do_close(v8_state vm, v8_handle hcr, void *ptr) {
    struct close_s {
        int fd;
    } __attribute__((packed));
    struct close_s *args = ptr;
    int rc = close_a(args->fd);
    if (rc < 0)
        v8->goreject(vm, hcr, strerror(errno));
    else
        v8->goresolve(vm, hcr, NULL, -1, 1);
}

static coroutine void
do_unlink(v8_state vm, v8_handle hcr, void *ptr) {
    struct unlink_s {
        char filename[1];
    } __attribute__((packed));
    struct unlink_s *args = ptr;  /* input and output */
    int rc = unlink_a(args->filename);
    if (rc < 0)
        v8->goreject(vm, hcr, strerror(errno));
    else
        v8->goresolve(vm, hcr, NULL, -1, 1);
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

static void
do_str2flag(v8_state vm, int argc, v8_val argv) {
    char *mode = v8dl->to_string(vm, 1, argv);
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
    v8dl->from_int(vm, ret, argv);
}


static v8_ffn ff_table[] = {
    {1, do_str2flag, "str2flag", 0},
    {0, do_readfile, "readfile_a", V8_DLCORO},
    {0, do_open, "open_a", V8_DLCORO},
    {0, do_pread, "pread_a", V8_DLCORO},
    {0, do_pwrite, "pwrite_a", V8_DLCORO},
    {0, do_close, "close_a", V8_DLCORO},
    {0, do_unlink, "unlink_a", V8_DLCORO},
    {0},
};

int JS_LOAD(v8_state vm, v8_handle hlib) {
    JS_EXPORT(ff_table);
}

