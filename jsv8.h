#ifndef _JSV8_H
#define _JSV8_H
#include <stdint.h>
#include <stddef.h>
#include "jsdef.h"

typedef void *js_worker;

extern js_vm *js_vmopen(js_worker w);
extern void js_vmclose(js_vm *vm);

extern js_handle *js_eval(js_vm *vm, const char *src);
extern int js_run(js_vm *vm, const char *src);

typedef js_handle *js_args[4];
extern js_handle *js_call(js_vm *vm,
        js_handle *hfunc, js_handle *hself, js_args hargs);
extern js_handle *js_callstr(js_vm *vm, const char *source,
        js_handle *hself, js_args hargs);

/* if length is -1, the number of bytes is calculated using strlen(stp). */
extern js_handle *js_string(js_vm *vm, const char *stp, int length);

extern js_handle *js_number(js_vm *vm, double d);
extern js_handle *js_int32(js_vm *vm, int32_t i);
extern js_handle *js_object(js_vm *vm);
extern js_handle *js_get(js_handle *hobj, const char *key);
extern int js_set(js_handle *hobj, const char *key, js_handle *hval);
extern js_handle *js_geti(js_handle *hobj, unsigned index);
extern int js_seti(js_handle *hobj, unsigned index, js_handle *hval);
extern int js_set_string(js_handle *hobj,
        const char *name, const char *val);
extern js_handle *js_array(js_vm *vm, int length);
extern js_handle *js_arraybuffer(js_vm *vm,
        void *ptr, size_t byte_length);
extern size_t js_bytelength(js_handle *harrbufview);
extern js_handle *js_getbuffer(js_handle *harrbufview);
extern void *js_externalize(js_handle *h);

extern const js_handle *js_global(js_vm *vm);
#define JSGLOBAL(vm)    (js_handle *)js_global(vm)
extern const js_handle *js_null(js_vm *vm);
#define JSNULL(vm)    (js_handle *)js_null(vm)

extern js_handle *js_pointer(js_vm *vm, void *ptr);
extern js_handle *js_cfunc(js_vm *vm, const js_ffn *func_wrap);
extern js_handle *js_error(js_vm *vm, const char *message);

extern void js_reset(js_handle *h);
extern void js_dispose(js_handle *h, Fnfree free_func);

extern void *js_topointer(js_handle *h);
extern const char *js_tostring(js_handle *h);
extern double js_tonumber(js_handle *h);
extern int32_t js_toint32(js_handle *h);

extern int js_isnumber(js_handle *h);
extern int js_isfunction(js_handle *h);
extern int js_isobject(js_handle *h);
extern int js_isarray(js_handle *h);
extern int js_ispointer(js_handle *h);
extern int js_isstring(js_handle *h);
extern int js_isnull(js_handle *h);
extern int js_isundefined(js_handle *h);

extern js_handle *js_go(js_vm *vm, Fngo fptr);
/* callback */
extern int js_gosend(js_vm *vm, js_hndl hcr, void *data);
extern int js_goerr(js_vm *vm, js_hndl hcr, char *message);
extern int js_godone(js_vm *vm, js_hndl hcr);

extern const char *js_errstr(js_vm *vm);

extern int js_gc(js_vm *vm);

#endif
