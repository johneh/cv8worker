#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "libpill.h"

#include "v8binding.h"
#include "jsv8.h"
#include "util.h"

#define WORKER(vm)  *((mill_worker *) (vm))

js_vm *js_vmopen(js_worker jw) {
    mill_worker w = (mill_worker) jw;
    if (!w)
        mill_panic("js_vmopen: mill_worker expected");
    js_vm *vm = js8_vmnew(w);   /* This runs in the main thread. */
    assert(*((mill_worker *) vm) == WORKER(vm));

    /* This 2nd part of the initialization process actually creates the V8 "isolate".
     * Runs js8_vminit() in the worker(V8) thread. */
    int rc = task_run(w, (void *) js8_vminit, vm, -1);
    assert(rc == 0);
    return vm;
}

void js_vmclose(js_vm *vm) {
    /* Close the write end of the pipe from the V8 thread. */
    (void) v8_run(vm, "$close();");
    js8_vmclose(vm);
}

static int v8_sched(struct js8_cmd_s *cmd) {
    if (mill_isself(WORKER(cmd->vm)))
        return js8_do(cmd);
    return task_run(WORKER(cmd->vm), (void *) js8_do, cmd, -1);
}

/* Returns NULL if there was an error in V8. */
v8_handle v8_eval(js_vm *vm, const char *src) {
    struct js8_cmd_s c;
    c.type = V8COMPILERUN;
    c.vm = vm;
    c.source = (char *) src;
    v8_sched(&c);
    return c.h;
}

int v8_run(js_vm *vm, const char *src) {
    v8_handle h = v8_eval(vm, src);
    if (!h)
        return 0;
    v8_reset(vm, h);
    return 1;
}

//
// v8_call(vm, func, NULL, (v8_args){0})
// -> this === Global and 0 args
//
/* Returns NULL if there was an error in V8. */
v8_handle v8_call(js_vm *vm, v8_handle hfunc,
            v8_handle hself, v8_args hargs) {
    struct js8_cmd_s c;
    c.type = V8CALL;
    c.vm = vm;
    c.h1 = hfunc;
    int i, nargs = 0;
    for (i = 0; i < 4 && hargs[i]; i++) {
        nargs++;
        c.a[i] = hargs[i];
    }
    c.nargs = nargs;
    c.h = hself;
    v8_sched(&c);
    return c.h;
}

/* SOURCE is a function expression.
 Returns NULL if there was an error in V8. */
v8_handle v8_callstr(js_vm *vm, const char *source,
            v8_handle hself, v8_args hargs) {
    struct js8_cmd_s c;
    c.type = V8CALLSTR;
    assert(source);
    c.vm = vm;
    c.source = (char *)source;
    int i, nargs = 0;
    for (i = 0; i < 4 && hargs[i]; i++) {
        nargs++;
        c.a[i] = hargs[i];
    }
    c.nargs = nargs;
    c.h = hself;
    v8_sched(&c);
    return c.h;
}

int v8_gc(js_vm *vm) {
    struct js8_cmd_s c;
    c.type = V8GC;
    c.vm = vm;
    v8_sched(&c);
    return c.weak_counter;
}

