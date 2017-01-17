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

v8_state js_vmopen(js_worker jw) {
    mill_worker w = (mill_worker) jw;
    if (!w)
        mill_panic("js_vmopen: mill_worker expected");
    v8_state vm = js8_vmnew(w);   /* This runs in the main thread. */
    assert(*((mill_worker *) vm) == WORKER(vm));

    /* This 2nd part of the initialization process actually creates the V8 "isolate".
     * Runs js8_vminit() in the worker(V8) thread. */
    int rc = task_run(w, (void *) js8_vminit, vm, -1);
    assert(rc == 0);
    return vm;
}

void js_vmclose(v8_state vm, const char *onexit, v8_val onexit_arg) {
    struct js8_cmd_s c;
    c.type = V8SHUTDOWN;
    c.vm = vm;
    c.h1.stp = (char *) onexit;
    c.a[0] = onexit_arg;
    c.nargs = 1;
    c.h2 = V8_GLOBAL;
    task_run(WORKER(vm), (void *) js8_do, &c, -1);
    js8_vmclose(vm);
}


static int v8_sched(struct js8_cmd_s *cmd) {
    if (mill_isself(WORKER(cmd->vm)))
        return js8_do(cmd);
    return task_run(WORKER(cmd->vm), (void *) js8_do, cmd, -1);
}

/* Returned value should be checked for JS exception. */
v8_val v8_eval(v8_state vm, const char *src) {
    struct js8_cmd_s c;
    c.type = V8COMPILERUN;
    c.vm = vm;
    c.h1.stp = (char *) src;
    v8_sched(&c);
    return c.h2;
}

int v8_run(v8_state vm, const char *src) {
    v8_val r = v8_eval(vm, src);
    if (!r.type)
        return 0;
    jsv8->reset(vm, r);
    return 1;
}

//
// v8_call(vm, func, jsv8->global(vm), (v8_args){0})
// -> this === Global and 0 args
//
/* Returned value should be checked for JS exception. */
v8_val v8_call(v8_state vm, v8_val hfunc,
            v8_val hself, int nargs, v8_val *args) {
    struct js8_cmd_s c;
    c.type = V8CALL;
    c.vm = vm;
    c.h1 = hfunc;
    // FIXME panic if nargs > 4
    memcpy(&c.a, args, nargs * sizeof (v8_val));
    c.nargs = nargs;
    c.h2 = hself;
    v8_sched(&c);
    return c.h2;
}

/* SOURCE is a function expression.
 Returned value should be checked for JS exception. */
v8_val v8_callstr(v8_state vm, const char *source,
            v8_val hself, int nargs, v8_val *args) {
    struct js8_cmd_s c;
    c.type = V8CALLSTR;
    assert(source);
    c.vm = vm;
    c.h1.stp = (char *)source;
    memcpy(&c.a, args, nargs * sizeof (v8_val));
    c.nargs = nargs;
    c.h2 = hself;
    v8_sched(&c);
    return c.h2;
}

int v8_gc(v8_state vm) {
    struct js8_cmd_s c;
    c.type = V8GC;
    c.vm = vm;
    v8_sched(&c);
    return V8_TOINT32(c.h2);
}

