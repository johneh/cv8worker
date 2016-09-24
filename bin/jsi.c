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

#include "libmill.h"
#include "jsv8.h"

static char *readfile(const char *filename, size_t *len);

void js_panic(js_vm *vm) {
    fprintf(stderr, "%s\n", js_errstr(vm));
    exit(1);
}

js_handle *ff_readfile(js_vm *vm, int argc, js_handle *argv[]) {
    const char *filename = js_tostring(argv[0]);
    size_t sz;
    char *buf = readfile(filename, & sz);
    js_handle *ret;
    if (buf) {
        ret = js_string(vm, buf, sz);
        free(buf);
    } else
        ret = JSNULL(vm);
    return ret;
}

js_handle *ff_realpath(js_vm *vm, int argc, js_handle *argv[]) {
    const char *path = js_tostring(argv[0]);
    char *resolved_path = realpath(path, NULL);
    js_handle *ret;
    if (resolved_path) {
        ret = js_string(vm, resolved_path, strlen(resolved_path));
        free(resolved_path);
    } else
        ret = JSNULL(vm);
    return ret;
}

js_handle *ff_isfile(js_vm *vm, int argc, js_handle *argv[]) {
    const char *path = js_tostring(argv[0]);
    struct stat sbuf;
    int ret = 0;
    if (path && stat(path, & sbuf) == 0) {
        if (S_ISREG(sbuf.st_mode))
            ret = 1;
    }
    return js_number(vm, ret);
}

static js_ffn ff_table[] = {
    { 1, ff_readfile, "readFile" },
    { 1, ff_realpath, "realPath" },
    { 1, ff_isfile, "isRegularFile" },
};

/* Create an object with the exported C functions */
js_handle *exports(js_vm *vm) {
    int i;
    int n = sizeof (ff_table) / sizeof (ff_table[0]);
    js_handle *h1 = js_object(vm);
    for (i = 0; i < n; i++) {
        js_handle *f1 = js_cfunc(vm, &ff_table[i]);
        if (! js_set(h1, ff_table[i].name, f1))
            js_panic(vm);
        js_reset(f1);
    }
    return h1;
}

js_handle *parse_args(js_vm *vm, int argc, char **argv, char **path) {
    js_handle *hargs = js_array(vm, argc);
    js_handle *hs;
    int i;
    *path = NULL;
    for (i = 1; i < argc; i++) {
        if (!argv[i])
            break;
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            *path = argv[++i];
            continue;
        }
        hs = js_string(vm, argv[i], strlen(argv[i]));
        js_seti(hargs, i-1, hs);
        js_reset(hs);
    }
    return hargs;
}

#ifndef MODULEPATH
#define MODULEPATH "."
#endif

static char fmt_src[] = "(function (loader, argv) {"
"var file = '%s/%s'; var s1 = loader.readFile(file);"
"if(s1 === null) throw new Error(file+': file not found');"
"$eval(s1, file).call(this, loader, argv);})";

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [-p <libpath>] -f <source>\n\
       %s <source>\n", argv[0], argv[0]);
        exit(1);
    }

    mill_init(-1, 0);
    mill_worker w = mill_worker_create();
    assert(w);
    js_vm *vm = js_vmopen(w);

    char *libpath = NULL;
    js_handle *hargs = parse_args(vm, argc, argv, &libpath);
    js_handle *hldr = exports(vm);
    if (!libpath)   /* TODO: path from an enviroment variable */
        libpath = MODULEPATH;
    js_set_string(hldr, "path", libpath);

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

    js_handle *hret = js_callstr(vm, s, JSGLOBAL(vm),
            (js_args) { hldr, hargs });
    if (!hret)
        js_panic(vm);
    js_reset(hret);
    js_reset(hldr);
    js_reset(hargs);

    js_vmclose(vm);
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
