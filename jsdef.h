#ifndef _JSDEF_H
#define _JSDEF_H
#include <stddef.h>

enum v8_ctype {
    V8_CTYPE_VOID = 1,
    V8_CTYPE_INT32,
    V8_CTYPE_UINT32,
    V8_CTYPE_DOUBLE,
    V8_CTYPE_STR,
    V8_CTYPE_PTR,
    V8_CTYPE_LONG,  /* from Long object */
    V8_CTYPE_ULONG,    /* from Long object */
    V8_CTYPE_HANDLE,
    V8_CTYPE_ERR,
};

enum {
    V8_ERRNUM,
    V8_ERRSTR,
    V8_ERRPTR,
    V8_ERRLONG,
    V8_ERRULONG,
    V8_ERRHANDLE,
};

typedef int v8_handle;

typedef struct {
    int type;
    v8_handle hndle;
    union {
        int32_t i32;
        uint32_t ui32;
        double d;
        char *stp;
        void *ptr;
        int64_t i64;
        uint64_t u64;
    };
} v8_val;

struct js_vm_s;
typedef struct js_vm_s js_vm;
typedef js_vm *v8_state;

typedef void (*Fngo)(v8_state, v8_val, void *);

typedef void (*Fnfree)(void *ptr);

typedef v8_val (*FnCtype)(v8_state, int, v8_val []);

typedef struct v8_ffn_s {
    int pcount;
    void *fp;
    const char *name;
    int flags;

/* Fngo */
#define V8_DLCORO   (1 << 1)

/* FnCtype */
#define V8_CFUNC   (1 << 2)
} v8_ffn;


struct v8_fn_s {
    v8_val (*object)(v8_state vm);
    v8_val (*get)(v8_state vm, v8_val objval, const char *key);
    int (*set)(v8_state vm, v8_val hobj, const char *key, v8_val val);
    v8_val (*geti)(v8_state vm, v8_val objval, unsigned index);
    int (*seti)(v8_state vm, v8_val objval, unsigned index, v8_val val);
    v8_val (*array)(v8_state vm, int length);

    /* V8 owns the Buffer memory. If ptr is not NULL, it must be
     * compatible with ArrayBuffer::Allocator::Free. */
    v8_val (*arraybuffer)(v8_state vm, void *ptr, size_t byte_length);
    size_t (*bytelength)(v8_state vm, v8_val val);  /* ArrayBuffer(View) */
    size_t (*byteoffset)(v8_state vm, v8_val val); /* ArrayBufferView */
    void * (*buffer)(v8_state vm, v8_val abval); /* ArrayBuffer(View) */

    void (*reset)(v8_state vm, v8_val val);

    v8_val (*goroutine)(v8_state, Fngo);
    /* if length > 0, data memory is owned by V8 and must be
     * compatible with C free() */
    int (*goresolve)(v8_state, v8_val, volatile void *ptr,
            int length, int done);
    int (*goreject)(v8_state, v8_val, const char *);

    v8_val (*callstr)(v8_state vm, const char *source,
            v8_val hself, int nargs, v8_val *args);

    /* private */
    v8_val *global_;
    v8_val *null_;
    v8_val (*ctypestr_)(const char *, unsigned);
    void (*panic_)(const char *, const char *);
    const char **errs_;
};

#define V8_ISNUMBER(arg)    (arg.type == V8_CTYPE_DOUBLE)
#define V8_ISSTRING(arg)    (arg.type == V8_CTYPE_STR)
#define V8_ISPOINTER(arg)   (arg.type == V8_CTYPE_PTR)
#define V8_ISLONG(arg)      (arg.type == V8_CTYPE_LONG)
#define V8_ISULONG(arg)     (arg.type == V8_CTYPE_ULONG)
#define V8_ISHANDLE(arg)    (arg.type == V8_CTYPE_HANDLE)
#define V8_ISUNDEF(arg) (arg.type == V8_CTYPE_VOID)
#define V8_ISERROR(arg) (arg.type == V8_CTYPE_ERR)

#define V8_ERRSTR(arg)    (arg.type == V8_CTYPE_ERR ? arg.stp : "?")

#define V8_TOINT32(arg) \
(arg.type == V8_CTYPE_DOUBLE ? (int32_t) arg.d \
 : ({ jsv8->panic_(jsv8->errs_[V8_ERRNUM], __func__); 0;}))

#define V8_TOUINT32(arg) \
(arg.type == V8_CTYPE_DOUBLE ? (uint32_t) arg.d \
 : ({ jsv8->panic_(jsv8->errs_[V8_ERRNUM], __func__); 0;}))

#define V8_TODOUBLE(arg) \
(arg.type == V8_CTYPE_DOUBLE ? arg.d \
 : ({ jsv8->panic_(jsv8->errs_[V8_ERRNUM], __func__); 0;}))

#define V8_TOPTR(arg) \
(arg.type == V8_CTYPE_PTR ? arg.ptr \
 : ({ jsv8->panic_(jsv8->errs_[V8_ERRPTR], __func__); NULL;}))

/* Special case: _only_ accepts $nullptr for NULL */
#define V8_TOSTR(arg) \
(arg.type == V8_CTYPE_STR ? arg.stp \
 : (arg.type == V8_CTYPE_PTR && !arg.ptr) ? NULL \
 : ({ jsv8->panic_(jsv8->errs_[V8_ERRSTR], __func__); NULL;}))

#define V8_TOLONG(arg) \
(arg.type == V8_CTYPE_LONG ? arg.i64 \
: ({ jsv8->panic_(jsv8->errs_[V8_ERRLONG], __func__); 0;}))

#define V8_TOULONG(arg) \
(arg.type == V8_CTYPE_ULONG ? arg.u64 \
: ({ jsv8->panic_(jsv8->errs_[V8_ERRULONG], __func__); 0;}))

#define V8_TOHANDLE(arg) \
(arg.type == V8_CTYPE_HANDLE ? arg.hndle \
: ({ jsv8->panic_(jsv8->errs_[V8_ERRHANDLE], __func__); 0;}))


/* void return - undef in js */
#define V8_VOID (v8_val) { .type = V8_CTYPE_VOID }

#define V8_INT32(i_) (v8_val) { .i32 = i_, .type = V8_CTYPE_INT32 }
#define V8_UINT32(ui_) (v8_val) { .ui32 = ui_, .type = V8_CTYPE_UINT32 }
#define V8_DOUBLE(d_) (v8_val) { .d = d_, .type = V8_CTYPE_DOUBLE }

#define V8_LONG(l_) (v8_val) { .i64 = l_, .type = V8_CTYPE_LONG }
#define V8_ULONG(ul_) (v8_val) { .u64 = ul_, .type = V8_CTYPE_ULONG }

/* pointer _must_ not be the result of V8_TOSTR() */
#define V8_PTR(p_) (v8_val) { .ptr = p_, .type = V8_CTYPE_PTR }

/* string copied */
#define V8_STR(s_, l_)    jsv8->ctypestr_(s_, l_)

#define V8_HANDLE(h_) (v8_val) { .hndle = h_, .type = V8_CTYPE_HANDLE }

#define V8_GLOBAL   (*jsv8->global_)
#define V8_NULL     (*jsv8->null_)

#endif
