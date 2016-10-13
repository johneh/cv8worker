#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include "libmill.h"
#include "jsv8.h"

void js_panic(js_vm *vm) {
    fprintf(stderr, "%s\n", js_errstr(vm));
    exit(1);
}
#define CHECK(rc, vm) if(!rc) js_panic(vm)


void testcall(js_vm *vm) {
    js_handle *list_props = js_eval(vm,
"(function (o){\n\
    let objToInspect;\n\
    let res = [];\n\
    for(objToInspect = o; objToInspect !== null;\n\
        objToInspect = Object.getPrototypeOf(objToInspect)){\n\
            res = res.concat(Object.getOwnPropertyNames(objToInspect));\n\
    }\n\
    res.sort();\n\
    for (let i = 0; i < res.length; i++)\n\
        $print(res[i]);\n\
    }\n\
)");

    CHECK(list_props, vm);
    assert(js_isfunction(list_props));
    js_handle *h1 = js_call(vm, list_props, NULL,
                        (js_args) { JSGLOBAL(vm) });
    CHECK(h1, vm);
    js_reset(h1);
    js_reset(list_props);
}

coroutine void do_task1(js_vm *vm, handle_t hcr, void *data) {
    const char *s1 = data;
    fprintf(stderr, "<- %s\n", s1);
    int k = random() % 50;
    mill_sleep(now() + k);
    char *tmp = malloc(100);
    sprintf(tmp, "%s -> Task done in %d millsecs ...", s1, k);
    js_gosend(vm, hcr, tmp);
    js_godone(vm, hcr);
}


/* Coroutine in the main thread (concurrency & parallelism) */
void testgo(js_vm *vm) {
    js_handle *cr = js_go(vm, do_task1);
    /* cr is a js object, can set properties on it */
    int r = js_set_string(cr, "name", "task1");
    assert(r);

    js_handle *f1 = js_callstr(vm, "(function(co) {\
        $print('co name =', co.name); \
        return function(s, callback) {\
            $go(co, s, callback);\
        };\
    });", NULL, (js_args) { cr } );
    assert(f1);

    /* Global.task1 = f1; */
    int rc = js_set(JSGLOBAL(vm), "task1", f1);
    assert(rc);
    js_reset(f1);
    js_reset(cr);

    // XXX: 300 go routines to test resizing the persistent store
    rc = js_run(vm,
"function foo(k) { var s=0; for (var i=0;i <k;i++){s+=i;}return s;}"
"for(var i=1; i<=5;i++) {\n\
    var s1=$malloc(10); s1.pack(0, 's', 'go' + i);s1.dispose();\n\
    task1(s1, function (err, data) {\n\
            if (err == null) $print(data.unpack(0, 's')[0]);data.dispose();\n\
    });\n\
}\n"
"$msleep(35);\n"
"foo(1000000);$print(foo(2000000));"
"for(var i=6; i<=300;i++) {\n\
    var s1=$malloc(10); s1.pack(0, 's', 'go' + i);s1.dispose();\n\
    task1(s1, function (err, data) {\n\
        if (err == null) $print(data.unpack(0, 's')[0]);data.dispose();\n\
    });\n\
}\n"
"$msleep(25);\n"
"/*foo(1000000);$print(foo(2000000));*/"
    );
    CHECK(rc, vm);
#if 0
    int weak_counter = js_gc(vm);
    printf("weak counter = %d\n", weak_counter);
#endif
}

static char *readfile(const char *filename, size_t *len);

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

js_handle *ff_strerror(js_vm *vm, int argc, js_handle *argv[]) {
    int errnum = (int) js_tonumber(argv[0]);
    char *s = strerror(errnum);
    js_handle *ret = js_string(vm, s, -1);
    return ret;
}

static js_ffn ff_table[] = {
    { 1, ff_readfile, "readfile" },
    { 1, ff_strerror, "strerror" },
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

void testexports(js_vm *vm) {
    js_handle *eh = exports(vm);
    js_set(JSGLOBAL(vm), "c", eh);
    js_reset(eh);
    int rc = js_run(vm, "(function(filename) {\n\
var s = c.readfile(filename);\n\
if (s !== null) $print(filename + ' size = ' + s.length);\n\
else throw new Error(c.strerror($errno));})('./testjs.c');"
    );
    CHECK(rc, vm);
}

void testarraybuffer(js_vm *vm) {
    void *p1 = malloc(16);  // Memory must come from malloc().
    assert(p1);
    js_handle *h2 = js_arraybuffer(vm, p1, 16);
    CHECK(h2, vm);
    printf("byteLength = %d\n", (int) js_bytelength(h2));

    js_handle *h3 = js_callstr(vm, "(function(ab){\n\
var ia = new Int32Array(ab);\n\
ia[0] = 11; ia[1] = 22; ia[2] = 33; ia[3] = 44;\n\
$print(ia); return ia;});\n", NULL, (js_args) { h2 });
    CHECK(h3, vm);
    assert(js_isobject(h3));
    printf("byteLength = %d\n", (int) js_bytelength(h3));
    js_handle *h4 = js_getbuffer(h3);
    CHECK(h4, vm);
    printf("byteLength = %d\n", (int) js_bytelength(h4));
    void *p2 = js_topointer(h2);
    assert(p1 == p2);
    p2 = js_externalize(h2);    // Living dangerously.
    assert(p1 == p2);
    js_reset(h2);
    js_reset(h3);
    js_reset(h4);
    js_gc(vm);
    free(p1);   // no references to the array buffer left(?) or needed.
}

static int free_count;
js_handle *ff_efree(js_vm *vm, int argc, js_handle *argv[]) {
    void *ptr = js_topointer(argv[0]);
    free(ptr);
    free_count++;
    return NULL;
}

void efree(void *ptr) {
    free(ptr);
    free_count++;
}

static js_ffn fe = {1, ff_efree };

/* make GCTEST=1 */
void testdispose(js_vm *vm) {
    js_handle *h1 = js_cfunc(vm, &fe);
    int rc = js_set(JSGLOBAL(vm), "efree", h1);
    js_reset(h1);
    assert(rc);
    js_handle *p0 = js_pointer(vm, malloc(1));
    js_handle *h2 = js_callstr(vm, "(function(p0) {\n\
var p1 = $malloc(4);\n\
p1.free();p1=$malloc(8);p1.dispose(efree);});",
        NULL, (js_args) {p0});
    CHECK(h2, vm);
    js_reset(h2);
    js_dispose(p0, efree);
    js_gc(vm);
    assert(free_count == 2);
}

/* make GCTEST=1 */
void testdll(js_vm *vm) {
    int rc = js_run(vm, "(function() {\
var cairo = $load('./libcairojs.so');\
var surface = cairo.image_surface_create(\
                    cairo.CAIRO_FORMAT_ARGB32, 120, 100).notNull();\
var cr = cairo.create(surface).notNull();\
surface.dispose(cairo.surface_destroy);\
cr.dispose(cairo.destroy);\
cairo.set_source_rgb(cr, 1, 0, 0);\
cairo.rectangle(cr, 10, 10, 100, 80);\
cairo.fill(cr);\
cairo.surface_write_to_png(surface, 'crect.png');\
/*cairo.destroy(cr);*/\
/*cairo.surface_destroy(surface);*/})();"
    );
    CHECK(rc, vm);
    int weak_counter = js_gc(vm);
    assert(weak_counter == 2);
}


void testpack(js_vm *vm) {
    void *ptr = malloc(256);
    assert(ptr);
    js_handle *p1 = js_pointer(vm, ptr);
    CHECK(p1, vm);
    js_handle *r1 = js_callstr(vm, "var w = new WeakMap();\n\
    (function(p) {\n\
        var a = new Int32Array([-1,-2,-3,-4]);\n\
        p.pack(0, 'idsia', 10, 3.1416, 'apple banana', 1, a);\n\
        var x, k, off=0;\n\
        [x, k] = p.unpack(off, 'i'); $print(x); off+=k;\n\
        [x, k] = p.unpack(off, 'd'); $print(x); off+=k;\n\
        [x, k] = p.unpack(off, 's'); $print(x); off+=k;\n\
        [x, k] = p.unpack(off, 'i'); $print(x); off+=k;\n\
        var i1, d, s1, i2;\n\
        [i1, d, s1, i2, k] = p.unpack(0, 'idsi');\n\
        $print(i1); $print(s1); $print(off == k);\n\
        var ab;\n\
        [ab] = p.unpack(k, 'a');\n\
        $print(ab instanceof ArrayBuffer);\n\
        a = new Int32Array(ab); $print(a[0], a[1], a[2], a[3]);\n\
        [ab] = p.unpack(k, 'A');\n\
        /* With 'A', need to keep a reference to the pointer P. */\n\
        w.set(ab, p); /* keeps P alive as long as AB is not garbage collected */\n\
        return ab;\n\
    });", NULL, (js_args) { p1 } );
    CHECK(r1, vm);
    assert(js_isobject(r1));
    js_dispose(p1, free);
    int weak_counter = js_gc(vm);
    /* p1 still alive and r1 (ArrayBuffer) is valid. */
    assert(weak_counter == 0);

    js_handle *r2 = js_callstr(vm, "(function(ab) {\n\
        var a = new Int32Array(ab);\n\
        $print(a[0], a[1], a[2], a[3]);\n\
    });", NULL, (js_args) { r1 });
    CHECK(r2, vm);
    js_reset(r2);
    js_reset(r1);
    /* p1 can be garbage collected. */
    weak_counter = js_gc(vm);
    assert(weak_counter == 1);

    r1 = js_eval(vm, "$nullptr.packSize('idxs', 1, 1.0, 'apple');");
    CHECK(r1, vm);
    int pack_size = js_toint32(r1);
    assert(pack_size == 19);
    js_reset(r1);
}


int main(int argc, char *argv[]) {
    mill_init(-1, 0);
    mill_worker w = mill_worker_create();
    js_vm *vm = js_vmopen(w);

    testcall(vm);
    testgo(vm);
    testexports(vm);
    testarraybuffer(vm);
#ifdef GCTEST
    testpack(vm);
#endif
    /* testdispose(vm); */ /* make GCTEST=1 */
    /* testdll(vm); */    /* make GCTEST=1 */

    js_vmclose(vm);
    mill_worker_delete(w);
    mill_fini();
    return 0;
}


static char *readfile(const char *filename, size_t *len) {
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

