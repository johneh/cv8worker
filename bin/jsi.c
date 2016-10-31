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

void js_panic(v8_state vm) {
    fprintf(stderr, "%s\n", v8_errstr(vm));
    exit(1);
}

v8_handle ff_readfile(v8_state vm, int argc, v8_handle argv[]) {
    const char *filename = v8_tostring(vm, argv[1]);
    size_t sz;
    char *buf = readfile(filename, & sz);
    v8_handle ret;
    if (buf) {
        ret = v8_string(vm, buf, sz);
        free(buf);
    } else
        ret = v8_null(vm);
    return ret;
}

v8_handle ff_realpath(v8_state vm, int argc, v8_handle argv[]) {
    const char *path = v8_tostring(vm, argv[1]);
    char *resolved_path = realpath(path, NULL);
    v8_handle ret;
    if (resolved_path) {
        ret = v8_string(vm, resolved_path, strlen(resolved_path));
        free(resolved_path);
    } else
        ret = v8_null(vm);
    return ret;
}

v8_handle ff_isfile(v8_state vm, int argc, v8_handle argv[]) {
    const char *path = v8_tostring(vm, argv[1]);
    struct stat sbuf;
    int ret = 0;
    if (path && stat(path, & sbuf) == 0) {
        if (S_ISREG(sbuf.st_mode))
            ret = 1;
    }
    return v8_number(vm, ret);
}

static v8_ffn ff_table[] = {
    { 1, ff_readfile, "readFile" },
    { 1, ff_realpath, "realPath" },
    { 1, ff_isfile, "isRegularFile" },
};

/* Return an object with the exported C functions */
v8_handle exports(v8_state vm) {
    int i;
    int n = sizeof (ff_table) / sizeof (ff_table[0]);
    v8_handle h1 = v8_object(vm);
    for (i = 0; i < n; i++) {
        v8_handle f1 = v8_cfunc(vm, &ff_table[i]);
        if (! v8_set(vm, h1, ff_table[i].name, f1))
            js_panic(vm);
        v8_reset(vm, f1);
    }
    return h1;
}

v8_handle parse_args(v8_state vm, int argc, char **argv, char **path) {
    v8_handle hargs = v8_array(vm, argc);
    v8_handle hs;
    int i;
    *path = NULL;
    for (i = 1; i < argc; i++) {
        if (!argv[i])
            break;
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            *path = argv[++i];
            continue;
        }
        hs = v8_string(vm, argv[i], strlen(argv[i]));
        v8_seti(vm, hargs, i-1, hs);
        v8_reset(vm, hs);
    }
    return hargs;
}

coroutine void set_timeout(v8_state vm, v8_handle hcr, void *p1) {
    int64_t delay = *((int64_t *) p1);
    mill_sleep(now()+delay);
    v8_gosend(vm, hcr, NULL, 0);
    v8_godone(vm, hcr);
}

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

    mill_init(-1, 0);
    mill_worker w = mill_worker_create();
    assert(w);
    v8_state vm = js_vmopen(w);

    char *libpath = NULL;
    v8_handle hargs = parse_args(vm, argc, argv, &libpath);
    v8_handle loader = exports(vm);
    if (!libpath)   /* TODO: path from an enviroment variable */
        libpath = MODULEPATH;

    v8_handle htmp = v8_string(vm, libpath, -1);
    v8_set(vm, loader, "path", htmp);
    v8_reset(vm, htmp);

    htmp = v8_go(vm, set_timeout);
    v8_set(vm, loader, "msleep", htmp);
    v8_reset(vm, htmp);

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

    v8_handle hret = v8_callstr(vm, s, v8_global(vm),
                            (v8_args) { loader, hargs });
    if (!hret)
        js_panic(vm);
    v8_reset(vm, hret);
    v8_reset(vm, loader);
    v8_reset(vm, hargs);

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
