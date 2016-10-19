#include <stdint.h>
#include "jsdef.h"

typedef void *v8_val;
typedef void (*Fndlfnwrap)(v8_state, int, v8_val);

struct v8_dlfn_s {
    int (*to_int)(v8_state, int, v8_val);
    unsigned int (*to_uint)(v8_state, int, v8_val);
    int64_t (*to_long)(v8_state, int, v8_val);
    uint64_t (*to_ulong)(v8_state, int, v8_val);
    double (*to_double)(v8_state, int, v8_val);
    char *(*to_string)(v8_state, int, v8_val);
    void *(*to_pointer)(v8_state, int, v8_val);

    void (*from_int)(v8_state, int, v8_val);
    void (*from_uint)(v8_state, unsigned int, v8_val);
    void (*from_long)(v8_state, int64_t, v8_val);
    void (*from_ulong)(v8_state, uint64_t, v8_val);
    void (*from_double)(v8_state, double, v8_val);
    void (*from_pointer)(v8_state, void *, v8_val);

    const char *(*errstr)(v8_state);    /* XXX: duplicated in struct below */
};

struct v8_fn_s {
    v8_handle (*to_handle)(v8_state vm, int arg_num, v8_val argv);

    /* return only value */
    void (*from_handle)(v8_state vm, v8_handle h, v8_val argv);

    v8_handle (*number)(v8_state vm, double d);
    double (*tonumber)(v8_state vm, v8_handle h);
    v8_handle (*int32)(v8_state vm, int32_t i);
    int32_t (*toint32)(v8_state vm, v8_handle h);

    v8_handle (*string)(v8_state vm, const char *stptr, int length);
    /* Returned string should be deallocated using free(). */
    char *(*tostring)(v8_state vm, v8_handle h);

    v8_handle (*object)(v8_state vm);
    v8_handle (*get)(v8_state vm, v8_handle hobj, const char *key);
    int (*set)(v8_state vm, v8_handle hobj, const char *key, v8_handle hval);
    v8_handle (*geti)(v8_state vm, v8_handle hobj, unsigned index);
    int (*seti)(v8_state vm, v8_handle hobj, unsigned index, v8_handle hval);
    v8_handle (*array)(v8_state vm, int length);

    void (*reset)(v8_state vm, v8_handle h);

    /* pointer handle */
    int (*dispose)(v8_state vm, v8_handle h, Fnfree free_func);

    v8_handle (*global)(v8_state vm);
    v8_handle (*null)(v8_state vm);

    v8_handle (*goroutine)(v8_state, Fngo);
    int (*gosend)(v8_state, v8_handle, void *);
    int (*goerr)(v8_state, v8_handle, char *);
    int (*godone)(v8_state, v8_handle);

    const char *(*errstr)(v8_state vm);
    v8_handle (*callstr)(v8_state vm, const char *source,
            v8_handle hself, v8_args hargs);
};


#ifndef V8_BINDING   /* define in the main source before including this header */
static const struct v8_dlfn_s *v8dl;
static const struct v8_fn_s *v8;
#define JS_LOAD(vm, hlibobj) \
v8_load_(vm, hlibobj, const struct v8_dlfn_s *const dl_,\
        const struct v8_fn_s *const v8_, v8_ffn **fp_) {\
    v8dl = dl_; v8 = v8_;

#define JS_EXPORT(t_) } do {\
*fp_ = t_; return sizeof(t_)/sizeof(t_[0]);\
} while(0)

#else
/* The library must export this routine: */
#define V8_LOAD_FUNC "v8_load_"
#endif
