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

#include "libpill.h"
#include "jsv8.h"

static char *readfile(const char *filename, size_t *len);

v8_val ff_readfile(v8_state vm, int argc, v8_val argv[]) {
    const char *filename = V8_TOSTR(argv[0]);
    size_t sz;
    char *buf = readfile(filename, & sz);
    v8_val ret;
    if (buf) {
        ret = V8_STR(buf, sz);
        free(buf);
    } else
        ret = V8_NULL;
    return ret;
}

v8_val ff_realpath(v8_state vm, int argc, v8_val argv[]) {
    const char *path = V8_TOSTR(argv[0]);
    char *resolved_path = realpath(path, NULL);
    v8_val ret;
    if (resolved_path) {
        ret = V8_STR(resolved_path, strlen(resolved_path));
        free(resolved_path);
    } else
        ret = V8_NULL;
    return ret;
}

v8_val ff_isfile(v8_state vm, int argc, v8_val argv[]) {
    const char *path = V8_TOSTR(argv[0]);
    struct stat sbuf;
    int ret = 0;
    if (path && stat(path, & sbuf) == 0) {
        if (S_ISREG(sbuf.st_mode))
            ret = 1;
    }
    return V8_INT32(ret);
}

coroutine void set_timeout(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    uint32_t delay = V8_TOUINT32(args[0]);
    mill_sleep(now()+delay);
    jsv8->goresolve(vm, cr, V8_VOID, 1);
}

// POLLIN only. FIXME
coroutine void do_fdevent(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    int fd = V8_TOINT32(args[0]);
    int ret = mill_fdevent(fd, FDW_IN, -1);
    if (ret != FDW_IN)
        jsv8->goreject(vm, cr, strerror(EIO));
    else
        jsv8->goresolve(vm, cr, V8_VOID, 1);
}

static v8_ffn ff_table[] = {
    { 1, ff_readfile, "readFile", FN_CTYPE },
    { 1, ff_realpath, "realPath", FN_CTYPE },
    { 1, ff_isfile, "isRegularFile", FN_CTYPE },
    { 1, set_timeout, "msleep", FN_CORO },
    { 1, do_fdevent, "fdevent", FN_CORO },
};

/* Return an object with the exported C functions */
v8_val exports(v8_state vm) {
    int i;
    int n = sizeof (ff_table) / sizeof (ff_table[0]);
    v8_val h1 = jsv8->object(vm);
    for (i = 0; i < n; i++) {
        v8_val f1 = jsv8->cfunc(vm, &ff_table[i]);
        jsv8->set(vm, h1, ff_table[i].name, f1);
        jsv8->reset(vm, f1);
    }
    return h1;
}

v8_val parse_args(v8_state vm, int argc, char **argv, char **path) {
    v8_val hargs = jsv8->array(vm, argc);
    v8_val hs;
    int i;
    *path = NULL;
    for (i = 1; i < argc; i++) {
        if (!argv[i])
            break;
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            *path = argv[++i];
            continue;
        }
        hs = V8_STR(argv[i], strlen(argv[i]));
        jsv8->seti(vm, hargs, i-1, hs);
        jsv8->reset(vm, hs);
    }
    return hargs;
}


#if 0
int finished = 0;

coroutine void testcall(v8_state vm, mill_wgroup wg) {
    mill_wgadd(wg);
    v8_val hglobal = jsv8->global(vm);
    int count = 0;
    while (!finished) {
        mill_sleep(now()+1);
        count++;
        jsv8->set(vm, hglobal, "counter", V8_INT32(count));
    }
    //h1 = jsv8->get(vm, jsv8->global(vm), "counter");
    //printf("counter = %d\n", jsv8->toint32(vm, h1));
    //jsv8->reset(vm, h1);
    v8_val ctval = jsv8->get(vm, hglobal, "counter");
    fprintf(stderr, "counter = %d\n", V8_TOINT32(vm, ctval));
}
#endif


#ifndef MODULEPATH
#define MODULEPATH "."
#endif

static char fmt_src[] = "(function (loader, argv) {"
"var file = '%s/%s'; var s1 = loader.readFile(file);"
"if(s1 === null) throw new Error('error reading file: ' + file);"
"$eval(s1, file).call(this, loader, argv);})";

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [-p <libpath>] -f <source>\n\
       %s <source>\n", argv[0], argv[0]);
        exit(1);
    }

    mill_init(-1, -1);
    mill_worker w = mill_worker_create();
    assert(w);
    v8_state vm = js_vmopen(w);

    char *libpath = NULL;
    v8_val hargs = parse_args(vm, argc, argv, &libpath);
    v8_val loader = exports(vm);
    if (!libpath)   /* TODO: path from an enviroment variable */
        libpath = MODULEPATH;

    v8_val htmp = V8_STR(libpath, strlen(libpath));
    jsv8->set(vm, loader, "path", htmp);
    jsv8->reset(vm, htmp);

    char *mainpath = strdup(libpath);
    assert(mainpath);
    char *p;
    /* Only search the first item for __main__.js */
    if ((p = strchr(mainpath, ':')))
        *p = '\0';

    assert(strlen(fmt_src) < 256);
    char s[PATH_MAX+256];
    snprintf(s, PATH_MAX+256, fmt_src, mainpath, "__main__.js");
    free(mainpath);

//mill_wgroup wg = mill_wgmake();
//go(testcall(vm, wg));

    v8_val args[] = { loader, hargs };
    v8_val retval = jsv8->callstr(vm, s, V8_GLOBAL, 2, args);


//    finished = 1;
//   mill_wgwait(wg, -1);

    v8_val atexit_code = jsv8->get(vm, loader, "atexit");
    v8_val atexit_data = jsv8->get(vm, loader, "atexitData");

    jsv8->reset(vm, retval);
    jsv8->reset(vm, loader);
    jsv8->reset(vm, hargs);

    js_vmclose(vm, V8_TOSTR(atexit_code), atexit_data);

    if (V8_ISERROR(retval)) {
        fprintf(stderr, "%s\n", V8_ERRSTR(retval));
        exit(1);
    }

    mill_worker_delete(w);
    mill_fini();

    return 0;
}

static char *readfile(const char *filename, size_t *len) {
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
