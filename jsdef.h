#ifndef _JSDEF_H
#define _JSDEF_H

typedef int v8_handle;

struct js_vm_s;
typedef struct js_vm_s js_vm;
typedef js_vm *v8_state;

typedef void (*Fngo)(js_vm *vm, v8_handle coro, void *data);

typedef void (*Fnfree)(void *ptr);

typedef v8_handle (*Fnfnwrap)(js_vm *, int, v8_handle []);

typedef struct v8_ffn_s {
    int pcount;
    void *fp;
    const char *name;
    int flags;
#define V8_DLFUNC  (1<<0)
#define V8_DLCORO  (1<<1)
} v8_ffn;

typedef v8_handle v8_args[4];

#endif
