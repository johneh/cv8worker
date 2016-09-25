#define JS_DLL 1
#include "jsv8dlfn.h"

static void do_add(js_vm *vm, int argc, js_val argv) {
    int i = js_dl->to_int(vm, 1, argv);
    int j = js_dl->to_int(vm, 2, argv);
    if (js_dl->errstr(vm))
        return;
    int r = i + j;
    js_dl->from_int(vm, r, argv);
}

static void do_sub(js_vm *vm, int argc, js_val argv) {
    int i = js_dl->to_int(vm, 1, argv);
    int j = js_dl->to_int(vm, 2, argv);
    if (js_dl->errstr(vm))
        return;
    int r = i - j;
    js_dl->from_int(vm, r, argv);
}

static js_ffn ff_table[] = {
    { 2, do_add, "add" },
    { 2, do_sub, "sub" },
    {0},
};

int JS_LOAD(js_vm *vm, js_val libobj) {
    js_dl = dl_;
    int rc = js_dl->call_str(vm,
"(function(){ this.PI = 3.1416;});",
            libobj);
    if (!rc) return -1;
    JS_EXPORT(ff_table);
}

