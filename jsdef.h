#ifndef _JSDEF_H
#define _JSDEF_H

struct js_vm_s;
typedef struct js_vm_s js_vm;

struct js_handle_s;
typedef struct js_handle_s js_handle;

typedef void (*Fngo)(js_vm *vm, js_handle *coro, js_handle *hin);

typedef void (*Fnfree)(void *ptr);
typedef js_handle *(*Fnfnwrap)(js_vm *, int, js_handle *[]);

struct cffn_s {
    int pcount;
    void *fp;
    const char *name;
    int flags;
#define JSV8_DLFUNC  (1<<0)
#define JSV8_DLCORO  (1<<1)
};

typedef struct cffn_s js_ffn;
#endif
