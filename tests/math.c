#include "jsv8dlfn.h"
//#include "jsv8.h"
#include <stdio.h>
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
    { 2, do_add, "add", 0},
    { 2, do_sub, "sub", 0},
    {0},
};

int JS_LOAD(js_vm *vm, js_val libobj) {
    int rc = js_dl->call_str(vm,
"(function(){ this.PI = 3.1416;});",
            libobj);
    if (!rc) return -1;
#if 0
    /////////////////////////////////////////////////
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

//    CHECK(list_props, vm);
//    assert(js_isfunction(list_props));
    js_handle *h1 = js_call(vm, list_props, NULL,
                        (js_args) { JSGLOBAL(vm) });
    /////////////////////////////////////////////////
#endif
    JS_EXPORT(ff_table);
}

