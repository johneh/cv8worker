#include "jsv8.h"

static js_handle *do_add(js_vm *vm, int argc, js_handle *argv[]) {
    int i = js_tonumber(argv[0]);
    int j = js_tonumber(argv[1]);
    return js_number(vm, i+j);
}

static js_handle *do_sub(js_vm *vm, int argc, js_handle *argv[]) {
    int i = js_tonumber(argv[0]);
    int j = js_tonumber(argv[1]);
    return js_number(vm, i-j);
}

static js_ffn ff_table[] = {
    { 2, do_add, "add" },
    { 2, do_sub, "sub" },
    {0},
};

int JSLOAD(js_vm *vm, js_handle *hlib) {
    js_handle *h1 = js_callstr(vm,
"(function(){ this.PI = 3.1416;});",
            hlib, (js_args) {0});
    if (!h1) return -1;
    js_reset(h1);

    JSEXPORT(ff_table);
}

