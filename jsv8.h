#ifndef _JSV8_H
#define _JSV8_H
#include <stdint.h>
#include <stddef.h>
#include "jsdef.h"

typedef void *js_worker;

extern v8_state js_vmopen(js_worker w);
extern void js_vmclose(v8_state vm);

extern v8_handle v8_eval(v8_state vm, const char *src);
extern int v8_run(v8_state vm, const char *src);

extern v8_handle v8_call(v8_state vm,
        v8_handle hfunc, v8_handle hself, v8_args hargs);
extern v8_handle v8_callstr(v8_state vm, const char *source,
        v8_handle hself, v8_args hargs);

#if 0
extern int js_set_string(js_handle *hobj,
        const char *name, const char *val);

extern int js_isnumber(js_handle *h);
extern int js_isfunction(js_handle *h);
extern int js_isobject(js_handle *h);
extern int js_isarray(js_handle *h);
extern int js_ispointer(js_handle *h);
extern int js_isstring(js_handle *h);
extern int js_isnull(js_handle *h);
extern int js_isundefined(js_handle *h);
#endif

extern v8_handle v8_go(v8_state vm, Fngo fptr);

extern int v8_gosend(v8_state vm, v8_handle hcr, void *data, int length);
extern int v8_goerr(v8_state vm, v8_handle hcr, char *message);
extern int v8_godone(v8_state vm, v8_handle hcr);
extern int v8_goresolve(v8_state vm, v8_handle hcr, void *data, int length);
extern int v8_goreject(v8_state vm, v8_handle hcr, char *message);
extern void v8_set_errstr(v8_state vm, const char *str);
extern const char *v8_errstr(v8_state vm);

extern int v8_gc(v8_state vm);

extern v8_handle v8_number(v8_state vm, double d);
extern double v8_tonumber(v8_state vm, v8_handle h);
extern v8_handle v8_int32(v8_state vm, int32_t i);
extern int32_t v8_toint32(v8_state vm, v8_handle h);

/* if length is -1, the number of bytes is calculated using strlen(stp). */
extern v8_handle v8_string(v8_state vm, const char *stptr, int length);

/* Returned string should be deallocated using free(). */
extern char *v8_tostring(v8_state vm, v8_handle h);

extern v8_handle v8_object(v8_state vm);
extern v8_handle v8_get(v8_state vm, v8_handle hobj, const char *key);
extern int v8_set(v8_state vm, v8_handle hobj, const char *key, v8_handle hval);
extern v8_handle v8_geti(v8_state vm, v8_handle hobj, unsigned index);
extern int v8_seti(v8_state vm, v8_handle hobj, unsigned index, v8_handle hval);
extern v8_handle v8_array(v8_state vm, int length);

/* V8 owns the Buffer memory. If ptr is not NULL, it must be
 * compatible with ArrayBuffer::Allocator::Free. */
extern v8_handle v8_arraybuffer(v8_state vm, void *ptr, size_t byte_length);
extern size_t v8_bytelength(v8_state vm, v8_handle h);  /* ArrayBuffer(View) */
extern size_t v8_byteoffset(v8_state vm, v8_handle harrbufview);
extern v8_handle v8_getbuffer(v8_state vm, v8_handle htypedarray);
extern void *v8_externalize(v8_state vm, v8_handle harraybuffer);

extern v8_handle v8_pointer(v8_state vm, void *ptr);
extern void *v8_topointer(v8_state vm, v8_handle h);

extern void v8_reset(v8_state vm, v8_handle h);

/* pointer */
extern int v8_dispose(v8_state vm, v8_handle h, Fnfree free_func);

extern v8_handle v8_global(v8_state vm);
extern v8_handle v8_null(v8_state vm);

extern v8_handle v8_cfunc(v8_state vm, const v8_ffn *func_item);

#endif
