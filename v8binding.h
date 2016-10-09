#ifdef __cplusplus
extern "C" {
#endif
#include "libmill.h"
#include "jsdef.h"

enum js_code {
    V8UNKNOWN = 0,
    /* begin C objects */
    V8EXTPTR,
    V8EXTFUNC,
    V8INT64,
    V8UINT64,
    V8GO,
    /* end C objects */
    V8UNDEFINED,
    V8NULL,
    V8NUMBER,
    V8STRING,
    V8OBJECT,
    V8ARRAY,
    V8FUNC,
};

enum js_cmd {
    V8COMPILERUN = 1,
    V8CALL,
    V8CALLSTR,
    V8GC,   /* Request garbage collection */
};

struct js8_arg_s {
    enum js_cmd type;
    int nargs;   /* js_call */
    js_vm *vm;

    union {
        js_handle *h1;
        char *source;
    };
    js_handle *a[4];
    /* input + output */
    volatile union {
        js_handle *h;
        int weak_counter;
    };
};

extern js_vm *js8_vmnew(mill_worker w);
extern int js8_vminit(js_vm *vm);
extern int js8_do(struct js8_arg_s *args);
extern void js8_vmclose(js_vm *vm);
#ifdef __cplusplus
}
#endif
