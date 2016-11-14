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

void js_panic(v8_state vm) {
    fprintf(stderr, "%s\n", v8_errstr(vm));
    exit(1);
}
#define CHECK(rc, vm) if(!rc) js_panic(vm)


void testcall(v8_state vm) {
    printf("testcall .....\n");
    v8_handle list_props = v8_eval(vm,
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
    v8_handle h1 = v8_call(vm, list_props, 0,
                        (v8_args) { v8_global(vm) });
    CHECK(h1, vm);
    v8_reset(vm, h1);
    v8_reset(vm, list_props);
}

#if 0
coroutine void do_task1(v8_state vm, v8_handle hcr, void *data) {
    const char *s1 = data;
    fprintf(stderr, "<- %s\n", s1);
    int k = random() % 50;
    mill_sleep(now() + k);
    char *tmp = malloc(100);
    sprintf(tmp, "%s -> Task done in %d millsecs ...", s1, k);
    v8_gosend(vm, hcr, tmp, strlen(tmp));
    v8_godone(vm, hcr);
}

void testgo(v8_state vm) {
    printf("testgo .....\n");
    v8_handle cr = v8_go(vm, do_task1);
    /* cr is a js object, can set properties on it */
    v8_handle t1 = v8_string(vm, "task1", -1);
    int r = v8_set(vm, cr, "name", t1);
    assert(r);
    v8_reset(vm, t1);

    v8_handle f1 = v8_callstr(vm, "(function(co) {\
        $print('co name =', co.name); \
        return function(s, callback) {\
            $go(co, s, callback);\
        };\
    });", 0, (v8_args) { cr } );
    assert(f1);

    /* Global.task1 = f1; */
    int rc = v8_set(vm, v8_global(vm), "task1", f1);
    assert(rc);
    v8_reset(vm, f1);
    v8_reset(vm, cr);

    // XXX: 300 go routines to test resizing of the persistent store
    rc = v8_run(vm,
"function foo(k) { var s=0; for (var i=0;i <k;i++){s+=i;}return s;}"
"for(var i=1; i<=5;i++) {\n\
    var s1=$malloc(10); s1.pack(0, 's', 'go' + i);s1.gc();\n\
    task1(s1, function (err, data) {\n\
            if (err == null) $print(data.unpack(0, 's')[0]);\n\
    });\n\
}\n"
"$msleep(35);\n"
"foo(1000000);$print(foo(2000000));"
"for(var i=6; i<=300;i++) {\n\
    var s1=$malloc(10); s1.pack(0, 's', 'go' + i);s1.gc();\n\
    task1(s1, function (err, data) {\n\
        if (err == null) $print(data.unpack(0, 's')[0]);\n\
    });\n\
}\n"
"$msleep(25);\n"
"/*foo(1000000);$print(foo(2000000));*/"
    );
    CHECK(rc, vm);
#if 0
    int weak_counter = v8_gc(vm);
    printf("weak counter = %d\n", weak_counter);
#endif
}
#endif

coroutine void do_task2(v8_state vm, v8_handle hcr, void *data) {
    const char *s1 = data;
    fprintf(stderr, "<- %s\n", s1);
    int k = random() % 50;
    mill_sleep(now() + k);
    char *tmp = malloc(100);
    sprintf(tmp, "%s -> Task done in %d millsecs ...", s1, k);
    v8_goresolve(vm, hcr, tmp, strlen(tmp), 1);
}

void testgo2(v8_state vm) {
    printf("test go with promises.....\n");
    v8_handle cr = v8_go(vm, do_task2);
    /* Global.task2 = cr; */
    int rc = v8_set(vm, v8_global(vm), "task2", cr);
    assert(rc);
    v8_reset(vm, cr);

    rc = v8_run(vm,
"var a=[]; for(var i=1; i<=5;i++) {\n\
    var s1=$malloc(10); s1.pack(0, 's', 'co' + i);s1.gc();\n\
    var p1 = $go(task2, s1);\n\
    p1.then(function(data) {\n\
        $print(data.unpack(0, 's')[0]);\n\
        throw new Error('Ignored exception');/* use a .catch(..) to see this error */\n\
    });\n\
    a.push(p1);\n\
}\n"
"$msleep(35);\n"
    );

    CHECK(rc, vm);

    // chaining
    rc = v8_run(vm,
"var s1=$malloc(10); s1.pack(0, 's', 'coro1');s1.gc();\n\
var async_task1 = task2; var async_task2 = task2;\n\
$go(async_task1, s1)\n\
.then(function(data) {\n\
    $print(data.unpack(0, 's')[0]);\n\
    var s2 = $malloc(10); s2.pack(0, 's', 'coro2');s2.gc();\n\
    return $go(async_task2, s2);\n\
})\n\
.then(function(data) {\n\
    $print(data.unpack(0, 's')[0]);\n\
});\n\
$print('Await-ing coro1 and coro2 .....');\n\
");

    CHECK(rc, vm);
}

static char *readfile(const char *filename, size_t *len);

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

v8_handle ff_strerror(v8_state vm, int argc, v8_handle argv[]) {
    int errnum = (int) v8_tonumber(vm, argv[1]);
    char *s = strerror(errnum);
    return v8_string(vm, s, -1);
}

static v8_ffn ff_table[] = {
    { 1, ff_readfile, "readfile" },
    { 1, ff_strerror, "strerror" },
};

/* Create an object with the exported C functions */
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

void testexports(v8_state vm) {
    printf("testexports .....\n");
    v8_handle eh = exports(vm);
    v8_set(vm, v8_global(vm), "c", eh);
    v8_reset(vm, eh);
    int rc = v8_run(vm, "(function(filename) {\n\
var s = c.readfile(filename);\n\
if (s !== null) $print(filename + ' size = ' + s.length);\n\
else throw new Error(c.strerror($errno));})('./testjs.c');"
    );
    CHECK(rc, vm);
}

void testarraybuffer(v8_state vm) {
    printf("testarraybuffer .....\n");
    void *p1 = malloc(16);  // Memory must come from malloc().
    assert(p1);
    v8_handle h2 = v8_arraybuffer(vm, p1, 16);
    CHECK(h2, vm);
    printf("byteLength = %d\n", (int) v8_bytelength(vm, h2));

    v8_handle h3 = v8_callstr(vm, "(function(ab){\n\
var ia = new Int32Array(ab);\n\
ia[0] = 11; ia[1] = 22; ia[2] = 33; ia[3] = 44;\n\
$print(ia); return ia;});\n", 0, (v8_args) { h2 });
    CHECK(h3, vm);
    printf("byteLength = %d\n", (int) v8_bytelength(vm, h3));
    v8_handle h4 = v8_getbuffer(vm, h3);
    CHECK(h4, vm);
    printf("byteLength = %d\n", (int) v8_bytelength(vm, h4));
    void *p2 = v8_topointer(vm, h2);
    assert(p1 == p2);
    p2 = v8_externalize(vm, h2);    // Living dangerously.
    assert(p1 == p2);
    v8_reset(vm, h2);
    v8_reset(vm, h3);
    v8_reset(vm, h4);
    v8_gc(vm);
    free(p1);   // no references to the array buffer left(?) or needed.
}

#if 0
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
p1.free();p1=$malloc(8);p1.gc(efree);});",
        NULL, (js_args) {p0});
    CHECK(h2, vm);
    js_reset(h2);
    js_dispose(p0, efree);
    js_gc(vm);
    assert(free_count == 2);
}
#endif

/* make GCTEST=1 */
void testdll(v8_state vm) {
    int rc = v8_run(vm, "(function() {\
var cairo = $load('./libcairojs.so');\
var surface = cairo.image_surface_create(\
                    cairo.CAIRO_FORMAT_ARGB32, 120, 100).notNull();\
var cr = cairo.create(surface).notNull();\
surface.gc(cairo.surface_destroy);\
cr.gc(cairo.destroy);\
cairo.set_source_rgb(cr, 1, 0, 0);\
cairo.rectangle(cr, 10, 10, 100, 80);\
cairo.fill(cr);\
cairo.surface_write_to_png(surface, 'crect.png');\
/*cairo.destroy(cr);*/\
/*cairo.surface_destroy(surface);*/})();"
    );
    CHECK(rc, vm);
    int weak_counter = v8_gc(vm);
    assert(weak_counter == 2);
}


void testpack(v8_state vm) {
    printf("testpack .....\n");
    void *ptr = malloc(256);
    assert(ptr);
    v8_handle p1 = v8_pointer(vm, ptr);
    CHECK(p1, vm);
    v8_handle r1 = v8_callstr(vm, "var w = new WeakMap();\n\
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
    });", 0, (v8_args) { p1 } );
    CHECK(r1, vm);
    v8_dispose(vm, p1, free);
    int weak_counter = v8_gc(vm);
    /* p1 still alive and r1 (ArrayBuffer) is valid. */
    assert(weak_counter == 0);

    v8_handle r2 = v8_callstr(vm, "(function(ab) {\n\
        var a = new Int32Array(ab);\n\
        $print(a[0], a[1], a[2], a[3]);\n\
    });", 0, (v8_args) { r1 });
    CHECK(r2, vm);
    v8_reset(vm, r2);
    v8_reset(vm, r1);
    /* p1 can be garbage collected. */
    weak_counter = v8_gc(vm);
    assert(weak_counter == 1);

    r1 = v8_eval(vm, "$nullptr.packSize('idxs', 1, 1.0, 'apple');");
    CHECK(r1, vm);
    int pack_size = v8_toint32(vm, r1);
    assert(pack_size == 19);
    v8_reset(vm, r1);
}


int main(int argc, char *argv[]) {
    mill_init(-1, 0);
    mill_worker w = mill_worker_create();
    v8_state vm = js_vmopen(w);

    testcall(vm);
//    testgo(vm);
    testexports(vm);
    testarraybuffer(vm);
#ifdef GCTEST
    testpack(vm);
//    testdll(vm);
#endif
    /* testdispose(vm); */ /* make GCTEST=1 */

    testgo2(vm);

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

