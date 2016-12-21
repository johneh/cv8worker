#ifdef __cplusplus
extern "C" {
#endif
#include "libpill.h"
#include "jsdef.h"

enum {
    V8EXTPTR = 1,
    V8EXTFUNC,
    V8INT64,
    V8UINT64,
    V8GO,
};

enum js_cmd {
    V8COMPILERUN = 1,
    V8CALL,
    V8CALLSTR,
    V8GC,   /* Request garbage collection */
};

struct js8_cmd_s {
    enum js_cmd type;
    int nargs;   /* call, callstr */
    js_vm *vm;

    v8_val h1;
    v8_val a[4];   /* strictly for arguments */

    /* input + output */
    v8_val h2;
};

extern js_vm *js8_vmnew(mill_worker w);
extern int js8_vminit(js_vm *vm);
extern int js8_do(struct js8_cmd_s *args);
extern void js8_vmclose(js_vm *vm);
#ifdef __cplusplus
}
#endif
