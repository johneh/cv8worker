#include <stdint.h>
#include "jsdef.h"

typedef void *js_val;
typedef void (*Fndlfnwrap)(js_vm *, int, js_val);

struct js_dlfn_s {
    int (*to_int)(js_vm *, int, js_val);
    unsigned int (*to_uint)(js_vm *, int, js_val);
    int64_t (*to_long)(js_vm *, int, js_val);
    uint64_t (*to_ulong)(js_vm *, int, js_val);
    double (*to_double)(js_vm *, int, js_val);
    char *(*to_string)(js_vm *, int, js_val);
    void *(*to_pointer)(js_vm *, int, js_val);

    void (*from_int)(js_vm *, int, js_val);
    void (*from_uint)(js_vm *, unsigned int, js_val);
    void (*from_long)(js_vm *, int64_t, js_val);
    void (*from_ulong)(js_vm *, uint64_t, js_val);
    void (*from_double)(js_vm *, double, js_val);
    void (*from_pointer)(js_vm *, void *, js_val);

    int (*call_str)(js_vm *, const char *, js_val);
    const char *(*errstr)(js_vm *);
};

#ifdef JS_DLL   /* define in the dll source before including this header */
static struct js_dlfn_s *js_dl;
#define JS_LOAD(vm, libobj) js_load_(vm, libobj, struct js_dlfn_s *dl_, js_ffn **fp_)
#define JS_EXPORT(t_) do {\
*fp_ = t_; return sizeof(t_)/sizeof(t_[0]);\
} while(0)
#else
/* The library must export this routine: */
#define LOAD_FUNC "js_load_"
#endif
