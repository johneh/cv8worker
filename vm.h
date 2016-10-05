#ifndef _VM_H
#define _VM_H

#define MAXARGS 12

// V8 state (Isolate)

struct js_vm_s {
    mill_worker w;  // must be the first
    v8::Isolate *isolate;
    char *errstr;

    /* Remote coroutines */
    mill_pipe inq;
    mill_pipe outq;

    chan ch;
    int ncoro;

    struct js_handle_s *global_handle;
    struct js_handle_s *null_handle;
    struct js_handle_s *undef_handle;
    struct js_handle_s *nullptr_handle;

    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::ObjectTemplate> extptr_template;
    v8::Persistent<v8::ObjectTemplate> extfunc_template;
    v8::Persistent<v8::ObjectTemplate> i64_template;
    v8::Persistent<v8::ObjectTemplate> ui64_template;
    v8::Persistent<v8::ObjectTemplate> go_template;

    v8::Persistent<v8::Value> ctype_proto;
    v8::Persistent<v8::Value> cptr_proto;

    char *dlstr[MAXARGS];
    int dlstr_idx;

    js_handle *args[MAXARGS];
#ifdef V8TEST
    int weak_counter;
#endif
};

typedef struct js_vm_s js_vm;

struct js_handle_s {
    enum js_code type;
    int flags;
#define PERM_HANDLE (1 << 0)
#define DBL_HANDLE  (1 << 1)
#define STR_HANDLE  (1 << 2)
#define INT32_HANDLE    (1 << 3)
#define PTR_HANDLE  (1 << 4)
#define VALUE_MASK (DBL_HANDLE|STR_HANDLE|INT32_HANDLE|PTR_HANDLE)
#define ARG_HANDLE  (1 << 5)
#define WEAK_HANDLE (1 << 6)
#define FREE_EXTWRAP    (1 << 7)
#define FREE_DLWRAP (1 << 8)

// Coroutine flags
#define GoNoCallback    (1 << 13)
#define GoDeferReset    (1 << 14)
#define GoInString      (1 << 15)

    union {
        double d;
        char *stp;
        int32_t i;
        void *ptr;
    };
    js_vm *vm;
    union {
        Fnfree free_func;
        struct js_handle_s *(*free_extwrap)(js_vm *, int, struct js_handle_s *[]);
        void (*free_dlwrap)(js_vm *, int, void *argv);
        void *fp;
    };
    v8::Persistent<v8::Value> handle;
};


#define LOCK_SCOPE(isolate) \
Locker locker(isolate); \
Isolate::Scope isolate_scope(isolate); \
HandleScope handle_scope(isolate);

#define ThrowTypeError(isolate, m_) \
do {\
isolate->ThrowException(Exception::TypeError(\
    String::NewFromUtf8(isolate, m_))); \
return; } while(0)

#define ThrowNotEnoughArgs(isolate, t_) \
do {\
if (t_) {                      \
isolate->ThrowException(Exception::Error(\
    String::NewFromUtf8(isolate, "too few arguments"))); \
return; }} while(0)

#define ThrowError(isolate, m) \
do {\
isolate->ThrowException(Exception::Error(\
    String::NewFromUtf8(isolate, (m))));\
return; } while(0)

static inline v8::Local<v8::String> v8_str(v8::Isolate* isolate,
            const char* x) {
  return v8::String::NewFromUtf8(isolate, x, v8::NewStringType::kNormal)
      .ToLocalChecked();
}

#define v8Value Local<Value>
#define v8String Local<String>
#define v8Object Local<Object>
#define v8Array Local<Array>
#define v8Context Local<Context>
#define v8Function Local<Function>
#define v8ObjectTemplate Local<ObjectTemplate>
#define v8External Local<External>
#define v8Name Local<Name>
#define v8Script Local<Script>
#define v8Message Local<Message>
#define v8ArrayBufferView Local<ArrayBufferView>
#define v8ArrayBuffer Local<ArrayBuffer>
#define v8TypedArray Local<TypedArray>

#endif
