#ifndef _JSV8_H
#define _JSV8_H
#include <stdint.h>
#include <stddef.h>
#include "jsdef.h"

typedef void *js_worker;

extern struct v8_fn_s *jsv8;

extern v8_state js_vmopen(js_worker w);
extern void js_vmclose(v8_state vm);

extern v8_val v8_eval(v8_state vm, const char *src);
extern int v8_run(v8_state vm, const char *src);

extern v8_val v8_call(v8_state vm,
        v8_val func, v8_val self, int nargs, v8_val *args);
extern v8_val v8_callstr(v8_state vm, const char *source,
        v8_val self, int nargs, v8_val *args);

extern int v8_gc(v8_state vm);

extern v8_val v8_getbuffer(v8_state vm, v8_val val); /* argument: TypedArray */

extern v8_val v8_cfunc(v8_state vm, const v8_ffn *func_item);

#endif
