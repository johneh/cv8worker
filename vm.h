#ifndef _VM_H
#define _VM_H

#define MAXARGS 13

class PersistentStore;

struct v8_fn_s;

// V8 state (Isolate)

struct js_vm_s {
    mill_worker w;  // must be the first
    v8::Isolate *isolate;

    /* Remote coroutines */
    mill_pipe inq;
    mill_pipe outq;

    chan ch;
    int ncoro;

    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::ObjectTemplate> extptr_template;
    v8::Persistent<v8::ObjectTemplate> extfunc_template;
    v8::Persistent<v8::ObjectTemplate> i64_template;
    v8::Persistent<v8::ObjectTemplate> ui64_template;
    v8::Persistent<v8::ObjectTemplate> go_template;

    v8::Persistent<v8::Value> long_proto;
    v8::Persistent<v8::Value> cptr_proto;
    v8::Persistent<v8::Value> cfunc_proto;

    PersistentStore *store_;

#ifdef V8TEST
    int weak_counter;
#endif

};

#define NUM_PERMANENT_HANDLES   3

#define GLOBAL_HANDLE   1
#define NULL_HANDLE 2
#define NULLPTR_HANDLE  3

typedef struct js_vm_s js_vm;

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

static inline v8::Local<v8::String> v8STR(v8::Isolate* isolate,
            const char* x) {
  return v8::String::NewFromUtf8(isolate, x, v8::NewStringType::kNormal)
      .ToLocalChecked();
}

static inline int GetObjectId(v8::Local<v8::Value> v) {
    v8::Local<v8::Object> obj;
    if (v->IsObject()
            && (obj = v8::Local<v8::Object>::Cast(v))->InternalFieldCount() >= 2)
        return (static_cast<uint16_t>(reinterpret_cast<uintptr_t>(
                    obj->GetAlignedPointerFromInternalField(0))))>>2;
    return 0;
}

static inline void SetObjectId(v8::Local<v8::Object> obj, unsigned oid) {
    obj->SetAlignedPointerInInternalField(0,
            reinterpret_cast<void*>(static_cast<uintptr_t>(oid<<2)));
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
