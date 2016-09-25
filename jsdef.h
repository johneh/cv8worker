#ifndef _JSDEF_H
#define _JSDEF_H

struct js_vm_s;
typedef struct js_vm_s js_vm;

struct js_handle_s;
typedef struct js_handle_s js_handle;

struct js_coro_s;
typedef struct js_coro_s js_coro;
typedef void (*Fncoro)(js_vm *vm, js_coro *, js_handle *);
typedef void (*Fnfree)(void *ptr);
typedef js_handle *(*Fnfnwrap)(js_vm *, int, js_handle *[]);

struct cffn_s {
    int pcount;
    void *fp;
    const char *name;
    int isdlfunc;
};

typedef struct cffn_s js_ffn;
#endif
