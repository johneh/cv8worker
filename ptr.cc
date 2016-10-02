#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <inttypes.h>

#include "v8.h"
#include "v8binding.h"
#include "vm.h"
#include "ptr.h"
#include "util.h"

using namespace v8;

extern "C" {

int GetCtypeId(js_vm *vm, v8Value v) {
    if (!v->IsObject())
        return 0;
    HandleScope handle_scope(vm->isolate);
    v8Object obj = v8Object::Cast(v);
    if (obj->InternalFieldCount() != 2)
        return 0;
    v8Value proto = obj->GetPrototype();

    /* Walking the proto chain */
    while (!proto->IsNull()) {
        if (proto == vm->ctype_proto) {
            return static_cast<int>(reinterpret_cast<uintptr_t>(
                    obj->GetAlignedPointerFromInternalField(0)) >> 2);
        }
        proto = proto->ToObject()->GetPrototype();
    }
    return 0;
}

int IsCtypeWeak(v8Object obj) {
    assert(obj->InternalFieldCount() == 2);
    int id = static_cast<int>(reinterpret_cast<uintptr_t>(
                obj->GetAlignedPointerFromInternalField(0)) >> 1);
    return (id & 1);
}

// C pointer and function objects method
static void Ctype(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    v8Object obj = args.This();
    assert(args.Holder() == args.This()); // N.B.: not true in accessor callback.!
    int id = GetCtypeId(vm, obj);
    if (id == V8EXTPTR)
        args.GetReturnValue().Set(v8_str(isolate, "C-pointer"));
    else if (id == V8EXTFUNC)
        args.GetReturnValue().Set(v8_str(isolate, "C-function"));
}

static void Free(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8Object obj = args.Holder();
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "free: not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (ptr && !IsCtypeWeak(obj)) {
        free(ptr);
        obj->SetInternalField(1, External::New(isolate, nullptr));
    }
}

// ptr.notNull() -- ensure pointer is not NULL.
static void NotNull(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    v8Object obj = args.Holder();
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "notNull: not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (!ptr)
        ThrowError(isolate, "notNull: pointer is null");
    // Return the pointer to facilitate chaining:
    //  ptr = foo().notNull();
    args.GetReturnValue().Set(obj);
}

// ptr.utf8String(length = -1)
static void Utf8String(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    v8Object obj = args.Holder();
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "utf8String: not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (!ptr)
        ThrowError(isolate, "utf8String: pointer is null");
    int length = -1;
    if (args.Length() > 0) {
        length = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();
        if (length < 0)
            length = -1;
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate,
                (char *) ptr, v8::String::kNormalString, length));
}

v8Object WrapPtr(js_vm *vm, void *ptr) {
    Isolate *isolate = vm->isolate;
    assert(Locker::IsLocked(isolate));
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->extptr_template);
    v8Object obj = templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked();
    int oid = (V8EXTPTR<<2);
    obj->SetAlignedPointerInInternalField(0,
             reinterpret_cast<void*>(static_cast<uintptr_t>(oid)));
    obj->SetInternalField(1, External::New(isolate, ptr));
    obj->SetPrototype(v8Value::New(isolate, vm->cptr_proto));
    return handle_scope.Escape(obj);
}

// Construct the prototype object for C pointers and functions.
void MakeCtypeProto(js_vm *vm) {
    Isolate *isolate = vm->isolate;
    HandleScope handle_scope(isolate);
    v8ObjectTemplate cp_templ = ObjectTemplate::New(isolate);
    cp_templ->Set(v8_str(isolate, "ctype"),
                FunctionTemplate::New(isolate, Ctype));
    vm->ctype_proto.Reset(isolate, cp_templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked());

    // Create the proto object for the C-type pointer.
    //  cptr_object.__proto__ = ptr_proto;
    //  ptr_proto.__proto__ = vm->ctype_proto;
    v8ObjectTemplate ptr_templ = ObjectTemplate::New(isolate);
    ptr_templ->Set(v8_str(isolate, "dispose"),
                FunctionTemplate::New(isolate, Dispose));
    ptr_templ->Set(v8_str(isolate, "free"),
                FunctionTemplate::New(isolate, Free));
    ptr_templ->Set(v8_str(isolate, "notNull"),
                FunctionTemplate::New(isolate, NotNull));
    ptr_templ->Set(v8_str(isolate, "utf8String"),
                FunctionTemplate::New(isolate, Utf8String));

    // Create the one and only proto instance.
    v8Object ptr_proto = ptr_templ
                -> NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    // Setup the proto chain.
    ptr_proto->SetPrototype(v8Value::New(isolate, vm->ctype_proto));
    // Make the proto object persistent.
    vm->cptr_proto.Reset(isolate, ptr_proto);
}

}
