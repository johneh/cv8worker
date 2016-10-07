#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

// XXX: include libmill before v8_binding.h (need choose macro)
#define MILL_CHOOSE 1
#include "libmill.h"

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
    /* Run js8_vminit() in the V8 thread. */
    int rc = task_run(w, (void *) js8_vminit, vm, -1);
    assert(rc == 0);
    return vm;
}

void js_vmclose(js_vm *vm) {
    /* Close the write end of the pipe from the V8 thread. */
    (void) js_run(vm, "$close();");
    js8_vmclose(vm);
}

static int js_sched(struct js8_arg_s *args) {
    if (mill_isself(WORKER(args->vm)))
        return js8_do(args);
    return task_run(WORKER(args->vm), (void *) js8_do, args, -1);
}

/* Returns NULL if there was an error in V8. */
js_handle *js_eval(js_vm *vm, const char *src) {
    struct js8_arg_s args;
    args.type = V8COMPILERUN;
    args.vm = vm;
    args.source = (char *) src;
    js_sched(&args);
    return args.h;
}

int js_run(js_vm *vm, const char *src) {
    js_handle *h = js_eval(vm, src);
    if (!h)
        return 0;
    js_reset(h);
    return 1;
}

//
// js_call(vm, func, NULL, (js_args){0})
// -> this === Global and 0 args
//
/* Returns NULL if there was an error in V8. */
js_handle *js_call(js_vm *vm, js_handle *hfunc,
        js_handle *hself, js_args hargs) {
    struct js8_arg_s args;
    args.type = V8CALL;
    args.vm = vm;
    args.h1 = hfunc;
    int i, nargs = 0;
    for(i = 0; i < 4 && hargs[i]; i++) {
        nargs++;
        args.a[i] = hargs[i];
    }
    args.nargs = nargs;
    args.h = hself;
    js_sched(&args);
    return args.h;
}

/* SOURCE is a function expression.
 Returns NULL if there was an error in V8. */
js_handle *js_callstr(js_vm *vm, const char *source,
        js_handle *hself, js_args hargs) {
    struct js8_arg_s args;
    args.type = V8CALLSTR;
    assert(source);
    args.vm = vm;
    args.source = (char *)source;
    int i, nargs = 0;
    for(i = 0; i < 4 && hargs[i]; i++) {
        nargs++;
        args.a[i] = hargs[i];
    }
    args.nargs = nargs;
    args.h = hself;
    js_sched(&args);
    return args.h;
}

int js_gc(js_vm *vm) {
    struct js8_arg_s args;
    args.type = V8GC;
    args.vm = vm;
    js_sched(&args);
    return args.weak_counter;
}

void *choose_coro(chan ch) {
    void *t = NULL;
    choose {
    in(ch, js_handle *, t1):
        t = t1;
    otherwise:
        t = NULL;
    end
    }
    return t;
}

