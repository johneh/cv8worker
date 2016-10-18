#include "jsv8dlfn.h"
#include <stdio.h>
#include <assert.h>

static void do_add(v8_state vm, int argc, v8_val argv) {
    int i = v8dl->to_int(vm, 1, argv);
    int j = v8dl->to_int(vm, 2, argv);
    if (v8dl->errstr(vm))
        return;
    int r = i + j;
    v8dl->from_int(vm, r, argv);
}

static void do_sub(v8_state vm, int argc, v8_val argv) {
    int i = v8dl->to_int(vm, 1, argv);
    int j = v8dl->to_int(vm, 2, argv);
    if (v8dl->errstr(vm))
        return;
    int r = i - j;
    v8dl->from_int(vm, r, argv);
}

/*
static void do_mult(v8_state vm, int argc, v8_val argv) {
    assert(argc >= 2);
    v8_handle h1 = v8->to_handle(vm, 1, argv);
    v8_handle h2 = v8->to_handle(vm, 2, argv);
    if (v8->errstr(vm))
        return;
    v8_handle h3 = v8->callstr(vm, "(function(i,j) { return i*j;});",
            0, (v8_args) { h1, h2 });
    v8->from_handle(vm, h3, argv);
    v8->reset(vm, h1); v8->reset(vm, h2); v8->reset(vm, h3);
}
*/

static v8_ffn ff_table[] = {
    { 2, do_add, "add", 0},
    { 2, do_sub, "sub", 0},
/*    { 2, do_mult, "mult", 0}, */
    {0},
};


int JS_LOAD(v8_state vm, v8_handle hobj) {
    int rc = v8->callstr(vm,
"(function(){ this.PI = 3.1416;});",
            hobj, (v8_args){0});
    if (!rc) return -1;
    JS_EXPORT(ff_table);
}

