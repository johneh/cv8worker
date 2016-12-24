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
#include "store.h"
#include "ptr.h"
#include "util.h"

using namespace v8;

extern "C" {

v8Object ToPtr(v8Value v) {
    if (v->IsObject()) {
        v8Object obj = v8Object::Cast(v);
        if (obj->InternalFieldCount() == PTR_INTERNAL_FIELD_COUNT
            && (V8EXTPTR == (static_cast<uint16_t>(reinterpret_cast<uintptr_t>(
                    obj->GetAlignedPointerFromInternalField(0))) >> 2)
            )
        )
            return obj;
    }
    return v8Object();
}

int IsCtypeWeak(v8Object obj) {
    assert(obj->InternalFieldCount() == PTR_INTERNAL_FIELD_COUNT);
    int id = static_cast<int>(reinterpret_cast<uintptr_t>(
                obj->GetAlignedPointerFromInternalField(0))) >> 1;
    return (id & 1);
}

void SetCtypeWeak(v8Object obj) {
    int id = static_cast<int>(reinterpret_cast<uintptr_t>(
                obj->GetAlignedPointerFromInternalField(0)));
    id |= (1<<1);
    obj->SetAlignedPointerInInternalField(0,
                reinterpret_cast<void*>(static_cast<uintptr_t>(id)));
}

void SetCtypeSize(v8Object obj, int size) {
    obj->SetAlignedPointerInInternalField(2,
            reinterpret_cast<void*>(static_cast<intptr_t>(size << 1)));
}

int GetCtypeSize(v8Object obj) {
    return static_cast<int>(reinterpret_cast<intptr_t>(
                obj->GetAlignedPointerFromInternalField(2))>>1);
}

static void Free(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8Object obj = args.Holder();
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    if (GetObjectId(vm, obj) != V8EXTPTR)
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
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    v8Object obj = args.Holder();
    if (GetObjectId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "notNull: not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (!ptr)
        ThrowError(isolate, "notNull: pointer is null");
    // Return the pointer to facilitate chaining:
    //  ptr = foo().notNull();
    args.GetReturnValue().Set(obj);
}

// ptr.setId(id) -- set Ctype id, returns the pointer.
static void SetId(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    v8Object obj = args.Holder();
    if (GetObjectId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "setId: not a pointer");
    int ct = 0;
    if (args.Length() > 0) {
        ct = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();
    }
    SetCid(obj, ct);
    args.GetReturnValue().Set(obj);
}

// ptr.utf8String(length = -1)
static void Utf8String(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));

    v8Object obj = args.Holder();
    void *ptr;
    int maxLength;
    if (obj->IsArrayBuffer()) {
        ArrayBuffer::Contents c = v8ArrayBuffer::Cast(obj)->GetContents();
        ptr = c.Data();
        maxLength = (int) c.ByteLength();
    } else if (GetObjectId(vm, obj) == V8EXTPTR) {
        ptr = v8External::Cast(obj
                        -> GetInternalField(1))->Value();
        if (!ptr)
            ThrowError(isolate, "utf8String: pointer is null");
        maxLength = GetCtypeSize(obj);
    } else {
        ThrowTypeError(isolate, "utf8String: not a pointer");
    }

    int byteLength = -1;
    if (maxLength < 0) {    // pointer
        if (argc == 0)
            ThrowError(isolate, "utf8String: LENGTH argument required");
        byteLength = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();
        if (byteLength < 0)
            byteLength = -1;    // assume nul-terminated
    } else if (argc > 0) {
        byteLength = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();
        if (byteLength < 0)
            byteLength = -1;
        else if (byteLength > maxLength)
            byteLength = maxLength;
    } else {
        byteLength = maxLength;
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, (char *) ptr,
                v8::NewStringType::kNormal, byteLength).ToLocalChecked());
}


static int packint(v8Value val,
            int width, int issigned, void *ptr, Isolate *isolate) {
    double d = val->NumberValue(
                isolate->GetCurrentContext()).FromJust();
    if (width == 1) {
        if (issigned)
            *((int8_t *) ptr) = (int8_t) d;
        else
            *((uint8_t *) ptr) = (uint8_t) d;
        return 1;
    } else if (width == 2) {
        if (issigned)
            *((int16_t *) ptr) = (int16_t) d;
        else
            *((uint16_t *) ptr) = (uint16_t) d;
        return 2;
    } else if (width == 4) {
        if (issigned)
            *((int32_t *) ptr) = (int32_t) d;
        else
            *((uint32_t *) ptr) = (uint32_t) d;
        return 4;
    } else {
        /* width == 8 */
        if (issigned)
            *((int64_t *) ptr) = (int64_t) d;
        else
            *((uint64_t *) ptr) = (uint64_t) d;
        return 8;
    }
}

enum {
    BadFmtSpec = -10,
    NotEnoughArgs,
    InvalidValue
};

// FIXME -- use pure JS implementation. 
static int32_t packsize(int fmt, Isolate *isolate, v8Value val) {
    switch (fmt) {
    case '_':   /* null */
        return 0;
    case 'c':
    case 'b':
    case 'B':
    case 'x':
        return 1;
    case 'h':   /* short */
    case 'H':   /* unsigned short */
        return 2;
    case 'i':   /* int32_t (native int) */
    case 'I':   /* uint32_t (native unsigned int) */
        return 4;
    case 'l':   /* native long */
        return sizeof (long);
    case 'L':   /* native unsigned long */
        return sizeof (unsigned long);
    case 'j':   /* int64_t */
    case 'J':   /* uint64_t */
    case 'd':
        return 8;
    case 's': {
        /* nul-terminated string */
        v8String s = val->ToString(
                    isolate->GetCurrentContext()).ToLocalChecked();
        return s->Utf8Length() + 1;
    }
    case 'a': {
        size_t byte_length;
        /* arraybuffer, typedarray or dataview .. */
        if (val->IsArrayBufferView()) {
            v8ArrayBufferView av = v8ArrayBufferView::Cast(val);
            byte_length = av->ByteLength();
            goto x;
        }
        if (val->IsArrayBuffer()) {
            ArrayBuffer::Contents c = v8ArrayBuffer::Cast(val)->GetContents();
            byte_length = c.ByteLength();
            goto x;
        }
        if (val->IsString()) {
            v8String s = val->ToString(
                    isolate->GetCurrentContext()).ToLocalChecked();
            byte_length = s->Utf8Length();
            goto x;
        } else {
            int size;
            v8Object ptrObj = ToPtr(val);
            if (! ptrObj.IsEmpty() && (size = GetCtypeSize(ptrObj)) >= 0) {
                byte_length = size;
                goto x;
            }
            return InvalidValue;
        }
x:
        if (byte_length > INT32_MAX - 4)
            return InvalidValue;
        return 4 + byte_length;
    }
    case 'p':
        return sizeof (void *);
    default:
        break;
    }
    return BadFmtSpec;
}

static int32_t packsize_n(char *fmt,
            const FunctionCallbackInfo<Value>& args) {
    int c, nexti = 1;
    int argc = args.Length();
    int32_t sz, total = 0;
    while ((c = *fmt)) {
        if (c >= '0' && c <= '9') {
            /* " .. COUNTx .." */
            int count = c - '0';
            while ((c = *++fmt) && c >= '0' && c <= '9') {
                count = count * 10 + c - '0';
            }
            if (c != 'x')
                return BadFmtSpec;
            total += count;
            fmt++;
            continue;
        }
        if (c == 'x') {  /* same as "1x" */
            total++;
            fmt++;
            continue;
        }
        if (argc < (nexti + 1))
            return NotEnoughArgs;
        sz = packsize(*fmt, args.GetIsolate(), args[nexti]);
        if (sz < 0)
            return sz;
        total += sz;
        fmt++;
        nexti++;
    }
    return total;
}

// Use $nullptr as the object:
//  $nullptr.packSize(format, value1, ... )
static void PackSize(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    //  v8Object obj = args.Holder(); - Ignore
    int32_t sz = -1;
    String::Utf8Value fmt_str(args[0]);
    if (fmt_str.length() > 0)
        sz = packsize_n(*fmt_str, args);
    if (sz < 0)
        ThrowError(isolate,
            "packsize: invalid format or not enough arguments");
    args.GetReturnValue().Set(Integer::New(isolate, sz));
}

static int32_t pack(void *ptr, int fmt, Isolate *isolate, v8Value val) {
    switch (fmt) {
    case '_':   /* null */
        return 0;
    case 'c': {
        v8String s = val->ToString(
                isolate->GetCurrentContext()).ToLocalChecked();
        int l = 0;
        if (s->Utf8Length() > 0)
            l = s->WriteUtf8((char *) ptr, 1);
        if (l == 0)
            return InvalidValue;
    }
        return 1;
    case 'b':   /* char */
        return packint(val, 1, true, ptr, isolate);
    case 'B':   /* unsigned char */
        return packint(val, 1, false, ptr, isolate);
    case 'h':   /* short */
        return packint(val, 2, true, ptr, isolate);
    case 'H':   /* unsigned short */
        return packint(val, 2, false, ptr, isolate);
    case 'i':   /* int32_t (native int) */
        return packint(val, 4, true, ptr, isolate);
    case 'I':   /* uint32_t (native unsigned int) */
        return packint(val, 4, false, ptr, isolate);
    case 'l':   /* native long */
        if (sizeof (long) == 4)
            return packint(val, 4, true, ptr, isolate);
        return packint(val, 8, true, ptr, isolate);
    case 'L':   /* native unsigned long */
        if (sizeof (unsigned long) == 4)
            return packint(val, 4, false, ptr, isolate);
        return packint(val, 8, false, ptr, isolate);
    case 'j':   /* int64_t */
        return packint(val, 8, true, ptr, isolate);
    case 'J':   /* uint64_t */
        return packint(val, 8, false, ptr, isolate);
    case 'd':
        *((double *) ptr) = val->NumberValue(
                isolate->GetCurrentContext()).FromJust();
        return 8;
    case 'x':   /* padding */
        return 1;
    case 's': {
        /* nul-terminated string */
        v8String s = val->ToString(
                    isolate->GetCurrentContext()).ToLocalChecked();
        int utf8len = s->Utf8Length();      /* >= 0 */
        int l = s->WriteUtf8((char *)ptr, utf8len);
        *((char *) ptr + l) = '\0';
        return l+1;
    }
    case 'a':   /* arraybuffer, typedarray or dataview, string or ptr */
        if (val->IsArrayBufferView()) {
            v8ArrayBufferView av = v8ArrayBufferView::Cast(val);
            size_t byte_length = av->ByteLength();
            if (byte_length > INT32_MAX - 4)
                return InvalidValue;
            *((int32_t *) ptr) = (int32_t) byte_length;
            size_t l = av->CopyContents((char *) ptr + 4, byte_length);
            assert(l == byte_length);
            return 4 + byte_length;
        }
        if (val->IsArrayBuffer()) {
            ArrayBuffer::Contents c = v8ArrayBuffer::Cast(val)->GetContents();
            size_t byte_length = c.ByteLength();
            if (byte_length > INT32_MAX - 4)
                return InvalidValue;
            *((int32_t *) ptr) = (int32_t) byte_length;
            memcpy((char *)ptr + 4, c.Data(), byte_length);
            return 4 + byte_length;
        }
        if (val->IsString()) {
            v8String s = val->ToString(
                    isolate->GetCurrentContext()).ToLocalChecked();
            int utf8len = s->Utf8Length();      /* >= 0 */
            *((int32_t *) ptr) = (int32_t) utf8len;
            if (utf8len <= INT32_MAX - 4
                    && utf8len == s->WriteUtf8((char *)ptr+4, utf8len))
                return 4 + utf8len;
        } else {
            int size;
            v8Object ptrObj = ToPtr(val);
            if (! ptrObj.IsEmpty() && (size = GetCtypeSize(ptrObj)) >= 0
                    && size <= INT32_MAX - 4
            ) {
                *((int32_t *) ptr) = size;
                memcpy((char *)ptr+4, UnwrapPtr(ptrObj), size);
                return 4 + size;
            }
        }
        return InvalidValue;
    case 'p': {
        v8Object ptrObj = ToPtr(val);
        if (! ptrObj.IsEmpty()) {
            *((void **) ptr) = UnwrapPtr(ptrObj);
            return sizeof (void *);
        }
    }
        return InvalidValue;
    default:
        break;
    }
    return BadFmtSpec;
}

static int32_t pack_n(char *ptr, char *fmt,
            const FunctionCallbackInfo<Value>& args) {
    int c, nexti = 2;
    int argc = args.Length();
    char *p0 = ptr;
    int32_t sz;
    while ((c = *fmt)) {
        if (c >= '0' && c <= '9') {
            /* " .. COUNTx .." */
            int count = c - '0';
            while ((c = *++fmt) && c >= '0' && c <= '9') {
                count = count * 10 + c - '0';
            }
            if (c != 'x')
                return BadFmtSpec;
            ptr += count;
            fmt++;
            continue;
        }
        if (c == 'x') {  /* same as "1x" */
            ptr++;
            fmt++;
            continue;
        }
        if (argc < (nexti + 1))
            return NotEnoughArgs;
        sz = pack(ptr, *fmt, args.GetIsolate(), args[nexti]);
        if (sz < 0)
            return sz;
        ptr += sz;
        fmt++;
        nexti++;
    }
    return (int32_t) (ptr - p0);
}

// ptr.pack(offset, format, value1, ... )
static void Pack(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8Object obj = args.Holder();
    void *ptr;
    if (obj->IsArrayBuffer()) {
        ptr = v8ArrayBuffer::Cast(obj)->GetContents().Data();
    } else if (GetObjectId((v8_state) isolate->GetData(0), obj) == V8EXTPTR) {
        ptr = v8External::Cast(obj
                        -> GetInternalField(1))->Value();
        if (!ptr)
            ThrowError(isolate, "pack: pointer is null");
    } else {
        ThrowTypeError(isolate, "pack: not a pointer");
    }

    if (!args[0]->IsUint32())
        ThrowError(isolate, "pack: offset is not unsigned integer");
    size_t off = args[0]->Uint32Value(isolate->GetCurrentContext()).FromJust();
    ptr = (char *) ptr + off;
    int32_t sz = 0;

    if (argc > 1) {
        sz = -1;
        String::Utf8Value fmt_str(args[1]);
        if (fmt_str.length() > 0)
            sz = pack_n((char *)ptr, *fmt_str, args);
        if (sz < 0)
            ThrowError(isolate,
                "pack: invalid format or not enough arguments"); // FIXME seperate error messages
    }
    args.GetReturnValue().Set(Integer::New(isolate, sz));
}

static v8Value unpack(void *ptr, char fmtc,
            Isolate *isolate, int32_t *pwidth) {
    EscapableHandleScope handle_scope(isolate);
    v8Value v = v8Value();
    switch (fmtc) {
    case '_':
        *pwidth = 0;
        v = v8::Null(isolate);
        break;
    case 'c':
        *pwidth = 1;
        v = String::NewFromUtf8(isolate, (char *) ptr,
                        NewStringType::kNormal, 1).ToLocalChecked();
        break;
    case 'b':
        *pwidth = 1;
        v = Integer::New(isolate, (int32_t) *((int8_t *) ptr));
        break;
    case 'i':
        *pwidth = 4;
        v = Integer::New(isolate, *((int32_t *) ptr));
        break;
    case 'h':
        *pwidth = 2;
        v = Integer::New(isolate, (int32_t) *((int16_t *) ptr));
        break;
    case 'H':
        *pwidth = 2;
        v = Integer::New(isolate, (int32_t) *((uint16_t *) ptr));
        break;
    case 'B':
        *pwidth = 1;
        v = Integer::New(isolate, (int32_t) *((uint8_t *) ptr));
        break;
    case 'd':
        *pwidth = 8;
        v = Number::New(isolate, *((double *) ptr));
        break;
    case 'I':
        *pwidth = 4;
        v = Integer::NewFromUnsigned(isolate, *((uint32_t *) ptr));
        break;
    case 'l':
        if (sizeof (long) == 4) {
            v = Integer::New(isolate, (int32_t) *((long *) ptr));
            *pwidth = 4;
        } else {
            v = Number::New(isolate, *((int64_t *) ptr));
            *pwidth = 8;
        }
        break;
    case 'L':
        if (sizeof (unsigned long) == 4) {
            v = Integer::NewFromUnsigned(isolate,
                            (uint32_t) *((unsigned long *) ptr));
            *pwidth = 4;
        } else {
            v = Number::New(isolate, *((uint64_t *) ptr));
            *pwidth = 8;
        }
        break;
    case 'j':
        v = Number::New(isolate, *((int64_t *) ptr));
        *pwidth = 8;
        break;
    case 'J':
        v = Number::New(isolate, *((uint64_t *) ptr));
        *pwidth = 8;
        break;
    case 's':   // nul-terminated string
        v = String::NewFromUtf8(isolate, (char *) ptr);
        *pwidth = strlen((char *) ptr) + 1;
        break;
    case 'p':
        v = WrapPtr((v8_state ) isolate->GetData(0), *((void **) ptr));
        *pwidth = sizeof (void *);
        break;
    case 'a': {
        int32_t byte_length = *((int32_t *) ptr);
        if (byte_length < 0) {
            *pwidth = InvalidValue;
        } else {
            void *data = emalloc(byte_length);
            memcpy(data, (char *) ptr + 4, byte_length);
            v = ArrayBuffer::New(isolate, data, byte_length,
                             ArrayBufferCreationMode::kInternalized);
            *pwidth = 4 + byte_length;
        }
    }
        break;
    case 'A': {
        // Externalized arraybuffer; Will need to keep a reference to the
        // pointer. (Hint: A global weak map with arraybuffer as key and
        // the pointer as value.)
        int32_t byte_length = *((int32_t *) ptr);
        if (byte_length < 0) {
            *pwidth = InvalidValue;
        } else {
            v = ArrayBuffer::New(isolate, (char *) ptr + 4, byte_length);
            *pwidth = 4 + byte_length;
        }
    }
        break;
    default:
        *pwidth = BadFmtSpec;
    }
    return handle_scope.Escape(v);
}


static int32_t unpack_n(char *ptr, char *fmt, Isolate *isolate, v8Array arr) {
    int c, i = 0;
    char *p0 = ptr;
    int32_t sz;
    while ((c = *fmt)) {
        if (c >= '0' && c <= '9') {
            /* " .. COUNTx .." */
            int count = c - '0';
            while ((c = *++fmt) && c >= '0' && c <= '9') {
                count = count * 10 + c - '0';
            }
            if (c != 'x')
                return BadFmtSpec;
            ptr += count;
            fmt++;
            continue;
        }
        if (c == 'x') {  /* same as "1x" */
            ptr++;
            fmt++;
            continue;
        }
        v8Value v = unpack(ptr, *fmt, isolate, & sz);
        if (sz < 0)
            return sz;
        arr->Set(i, v);
        i++;
        ptr += sz;
        fmt++;
    }
    sz = (int32_t) (ptr - p0);
    arr->Set(i, Integer::New(isolate, sz));
    return sz;
}

// ptr.unpack(offset, format)
// Returns null if the format argument is invalid.
static void Unpack(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8Object obj = args.Holder();
    void *ptr;
    if (obj->IsArrayBuffer()) {
        ptr = v8ArrayBuffer::Cast(obj)->GetContents().Data();
    } else if (GetObjectId((v8_state ) isolate->GetData(0), obj) == V8EXTPTR) {
        ptr = v8External::Cast(obj
                        -> GetInternalField(1))->Value();
        if (!ptr)
            ThrowError(isolate, "pack: pointer is null");
    } else {
        ThrowTypeError(isolate, "pack: not a pointer");
    }

    if (!args[0]->IsUint32())
        ThrowError(isolate, "unpack: offset is not unsigned integer");
    size_t off = args[0]->Uint32Value(isolate->GetCurrentContext()).FromJust();
    ptr = (char *) ptr + off;
    if (argc > 1) {
        String::Utf8Value fmt_str(args[1]);
        if (fmt_str.length() > 0) {
            v8Array arr = Array::New(isolate, fmt_str.length()+1);
            if (unpack_n((char *)ptr, *fmt_str, isolate, arr) > 0) {
                args.GetReturnValue().Set(arr);
                return;
            }
        }
    }
    args.GetReturnValue().Set(v8::Null(isolate));
}

v8Object WrapPtr(v8_state vm, void *ptr) {
    Isolate *isolate = vm->isolate;
    assert(Locker::IsLocked(isolate));
    EscapableHandleScope handle_scope(isolate);
    if (!ptr) {
        return handle_scope.Escape(v8Object::Cast(
                vm->store_->Get(NULLPTR_HANDLE)));
    }
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->extptr_template);
    v8Object obj = templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked();
    SetObjectId(obj, V8EXTPTR);
    obj->SetInternalField(1, External::New(isolate, ptr));
    SetCtypeSize(obj, -1);  // opaque
    obj->SetPrototype(v8Value::New(isolate, vm->cptr_proto));
    return handle_scope.Escape(obj);
}

void *UnwrapPtr(v8Object ptrObj) {
    return v8External::Cast(ptrObj->GetInternalField(1))->Value();
}

// Construct the prototype object for C pointers and functions.
void MakeCtypeProto(v8_state vm) {
    Isolate *isolate = vm->isolate;
    HandleScope handle_scope(isolate);

    // Create the proto object for the C-type pointer.
    //  cptr_object.__proto__ = ptr_proto;
    v8ObjectTemplate ptr_templ = ObjectTemplate::New(isolate);
    ptr_templ->Set(v8STR(isolate, "gc"),
                FunctionTemplate::New(isolate, Gc));
    ptr_templ->Set(v8STR(isolate, "free"),
                FunctionTemplate::New(isolate, Free));
    ptr_templ->Set(v8STR(isolate, "notNull"),
                FunctionTemplate::New(isolate, NotNull));
    ptr_templ->Set(v8STR(isolate, "setId"),
                FunctionTemplate::New(isolate, SetId));
    ptr_templ->Set(v8STR(isolate, "utf8String"),
                FunctionTemplate::New(isolate, Utf8String));
    ptr_templ->Set(v8STR(isolate, "pack"),
                FunctionTemplate::New(isolate, Pack));
    ptr_templ->Set(v8STR(isolate, "unpack"),
                FunctionTemplate::New(isolate, Unpack));
    ptr_templ->Set(v8STR(isolate, "packSize"),
                FunctionTemplate::New(isolate, PackSize));

    // Create the one and only proto instance.
    v8Object ptr_proto = ptr_templ
                -> NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    // Make the proto object persistent.
    vm->cptr_proto.Reset(isolate, ptr_proto);
}

}

/*

// Contrast with plain $malloc(..)
function Pointer(size) {
    // throw "illegal operation";
    // OR ..
	size = ~~size;
	if (size <= 0)
		return $nullptr;
	return $malloc(size).gc();	// XXX: with finalizer free() by default.
}

Pointer.prototype = $nullptr.__proto__;
Pointer.prototype.constructor = Pointer;    // or $nullptr.__proto__.constructor = Pointer;

// additional proto methods .. 
Pointer.prototype.foo = function() {
	$print('.... foo ...');
	// this.free(); etc.
};

var p2 = $malloc(8);
$print(p2 instanceof Pointer); // -> true
var p3 = new Pointer(16);
$print(p3 instanceof Pointer); // -> true
$print($nullptr instanceof Pointer); // -> true

*/

