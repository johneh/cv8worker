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
/*
static void Free(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8Object obj = args.Holder();
    if (GetObjectId(obj) != V8EXTPTR)
        ThrowTypeError(isolate, "free: not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (ptr && !IsCtypeWeak(obj)) {
        free(ptr);
        obj->SetInternalField(1, External::New(isolate, nullptr));
    }
}
*/

// ptr.notNull() -- ensure pointer is not NULL.
static void NotNull(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8Object obj = args.Holder();
    if (GetObjectId(obj) != V8EXTPTR)
        ThrowTypeError(isolate, "notNull: not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (!ptr)
        ThrowError(isolate, "notNull: pointer is null");
    // Return the pointer to facilitate chaining:
    //  ptr = foo().notNull();
    args.GetReturnValue().Set(obj);
}

enum {
    BadFmtSpec = -10,
    NotEnoughArgs,
    InvalidValue,
    RangeError,
};

struct pack_s {
    char *ptr;
    char *end;
    int errcode;
};

#define CHECK(_ps, _w) if (_ps->ptr + (_w) > _ps->end) return RangeError

static int packint(v8Value val,
            int width, int issigned, pack_s *ps, Isolate *isolate) {
    CHECK(ps, width);
    char *ptr = ps->ptr;
    double d = val->NumberValue(
                isolate->GetCurrentContext()).FromJust();
    switch (width) {
    case 1:
        if (issigned)
            *((int8_t *) ptr) = (int8_t) d;
        else
            *((uint8_t *) ptr) = (uint8_t) d;
        ps->ptr++;
        break;
    case 2:
        if (issigned)
            *((int16_t *) ptr) = (int16_t) d;
        else
            *((uint16_t *) ptr) = (uint16_t) d;
        ps->ptr += 2;
        break;
    case 4:
        if (issigned)
            *((int32_t *) ptr) = (int32_t) d;
        else
            *((uint32_t *) ptr) = (uint32_t) d;
        ps->ptr += 4;
        break;
    default:
        /* width == 8 */
        if (issigned)
            *((int64_t *) ptr) = (int64_t) d;
        else
            *((uint64_t *) ptr) = (uint64_t) d;
        ps->ptr += 8;
        break;
    }
    return 0;
}


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
    case 'S':
    case 's': {
        /* nul-terminated string */
        v8String s = val->ToString(
                    isolate->GetCurrentContext()).ToLocalChecked();
        return (fmt == 's' ? (s->Utf8Length() + 1) : s->Utf8Length());
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

static int pack(pack_s *ps, int fmt, Isolate *isolate, v8Value val) {
    switch (fmt) {
    case '_':   /* null */
        return 0;
    case 'c': {
        v8String s = val->ToString(
                isolate->GetCurrentContext()).ToLocalChecked();
        CHECK(ps, 1);
        int l = 0;
        if (s->Utf8Length() > 0)
            l = s->WriteUtf8((char *) ps->ptr, 1);
        if (l == 0)
            return InvalidValue;
        ps->ptr++;
    }
        return 0;
    case 'b':   /* char */
        return packint(val, 1, true, ps, isolate);
    case 'B':   /* unsigned char */
        return packint(val, 1, false, ps, isolate);
    case 'h':   /* short */
        return packint(val, 2, true, ps, isolate);
    case 'H':   /* unsigned short */
        return packint(val, 2, false, ps, isolate);
    case 'i':   /* int32_t (native int) */
        return packint(val, 4, true, ps, isolate);
    case 'I':   /* uint32_t (native unsigned int) */
        return packint(val, 4, false, ps, isolate);
    case 'l':   /* native long */
        if (sizeof (long) == 4)
            return packint(val, 4, true, ps, isolate);
        return packint(val, 8, true, ps, isolate);
    case 'L':   /* native unsigned long */
        if (sizeof (unsigned long) == 4)
            return packint(val, 4, false, ps, isolate);
        return packint(val, 8, false, ps, isolate);
    case 'j':   /* int64_t */
        return packint(val, 8, true, ps, isolate);
    case 'J':   /* uint64_t */
        return packint(val, 8, false, ps, isolate);
    case 'd':
        CHECK(ps, 8);
        *((double *) ps->ptr) = val->NumberValue(
                isolate->GetCurrentContext()).FromJust();
        ps->ptr += 8;
        return 0;
    case 'x':   /* padding */
        CHECK(ps, 1);
        ps->ptr++;     
        return 0;
    case 'z': {
        v8String s = val->ToString(
                    isolate->GetCurrentContext()).ToLocalChecked();
        unsigned utf8len = s->Utf8Length();      /* >= 0 */
        CHECK(ps, utf8len + 4);
        int l = s->WriteUtf8((char *) ps->ptr + 4, (int) utf8len);
        *((int32_t *) ps->ptr) = l;
        ps->ptr += (l + 4);
        return 0;
    }
    case 'S':
    case 's': {
        /* nul-terminated string */
        int nulls = (fmt == 's');
        v8String s = val->ToString(
                    isolate->GetCurrentContext()).ToLocalChecked();
        unsigned utf8len = s->Utf8Length();      /* >= 0 */
        CHECK(ps, utf8len + nulls);
        int l = s->WriteUtf8(ps->ptr, nulls ? -1 : (int) utf8len);
        if (nulls) {
            *((char *) ps->ptr + l) = '\0';
            ps->ptr += (l+1);
        } else
            ps->ptr += l;
        return 0;
    }
    case 'a':   /* arraybuffer, typedarray or dataview, string or ptr */
        if (val->IsArrayBufferView()) {
            v8ArrayBufferView av = v8ArrayBufferView::Cast(val);
            size_t byte_length = av->ByteLength();
            if (byte_length > INT32_MAX)
                return InvalidValue;
            CHECK(ps, byte_length + 4);
            *((int32_t *) ps->ptr) = (int32_t) byte_length;
            size_t l = av->CopyContents(ps->ptr + 4, byte_length);
            assert(l == byte_length);
            ps->ptr += (l + 4);
            return 0;
        }
        if (val->IsArrayBuffer()) {
            ArrayBuffer::Contents c = v8ArrayBuffer::Cast(val)->GetContents();
            size_t byte_length = c.ByteLength();
            if (byte_length > INT32_MAX)
                return InvalidValue;
            CHECK(ps, byte_length + 4);
            *((int32_t *) ps->ptr) = (int32_t) byte_length;
            memcpy((char *) ps->ptr + 4, c.Data(), byte_length);
            ps->ptr += (4 + byte_length);
            return 0;
        }
        return InvalidValue;
    case 'p': {
        v8Object ptrObj = ToPtr(val);
        if (! ptrObj.IsEmpty()) {
            CHECK(ps, sizeof (void *));
            *((void **) ps->ptr) = UnwrapPtr(ptrObj);
            ps->ptr += sizeof (void *);
            return 0;
        }
    }
        return InvalidValue;
    default:
        break;
    }
    return BadFmtSpec;
}

static int pack_n(pack_s *ps, char *fmt,
            const FunctionCallbackInfo<Value>& args) {
    int c, nexti = 3, ret;
    int argc = args.Length();
    while ((c = *fmt)) {
        if (c >= '0' && c <= '9') {
            /* " .. COUNTx .." */
            int count = c - '0';
            while ((c = *++fmt) && c >= '0' && c <= '9') {
                count = count * 10 + c - '0';
            }
            if (c != 'x')
                return BadFmtSpec;
            CHECK(ps, count);
            ps->ptr += count;
            fmt++;
            continue;
        }
        if (c == 'x') {  /* same as "1x" */
            CHECK(ps, 1);
            ps->ptr++;
            fmt++;
            continue;
        }
        if (argc < (nexti + 1))
            return NotEnoughArgs;
        ret = pack(ps, *fmt, args.GetIsolate(), args[nexti]);
        if (ret < 0)
            return ret;
        fmt++;
        nexti++;
    }
    return 0;
}

#define CHK(_ps, _w) if (_ps->ptr + (_w) > _ps->end) { _ps->errcode = RangeError; break; }
static v8Value unpack(pack_s *ps, char fmtc, Isolate *isolate) {
    EscapableHandleScope handle_scope(isolate);
    v8Value v = v8Value();
    switch (fmtc) {
    case '_':
        v = v8::Null(isolate);
        break;
    case 'c':
        CHK(ps, 1)
        v = String::NewFromUtf8(isolate, ps->ptr,
                        NewStringType::kNormal, 1).ToLocalChecked();
        ps->ptr++;
        break;
    case 'b':
        CHK(ps, 1)
        v = Integer::New(isolate, (int32_t) *((int8_t *) ps->ptr));
        ps->ptr++;
        break;
    case 'i':
        CHK(ps, 4)
        v = Integer::New(isolate, *((int32_t *) ps->ptr));
        ps->ptr += 4;
        break;
    case 'h':
        CHK(ps, 2)
        v = Integer::New(isolate, (int32_t) *((int16_t *) ps->ptr));
        ps->ptr += 2;
        break;
    case 'H':
        CHK(ps, 2)
        v = Integer::New(isolate, (int32_t) *((uint16_t *) ps->ptr));
        ps->ptr += 2;
        break;
    case 'B':
        CHK(ps, 1)
        v = Integer::New(isolate, (int32_t) *((uint8_t *) ps->ptr));
        ps->ptr++;
        break;
    case 'd':
        CHK(ps, 8)
        v = Number::New(isolate, *((double *) ps->ptr));
        ps->ptr += 8;
        break;
    case 'I':
        CHK(ps, 4)
        v = Integer::NewFromUnsigned(isolate, *((uint32_t *) ps->ptr));
        ps->ptr += 4;
        break;
    case 'l':
        if (sizeof (long) == 4) {
            CHK(ps, 4)
            v = Integer::New(isolate, (int32_t) *((long *) ps->ptr));
            ps->ptr += 4;
        } else {
            CHK(ps, 8)
            v = Number::New(isolate, *((int64_t *) ps->ptr));
            ps->ptr += 8;
        }
        break;
    case 'L':
        if (sizeof (unsigned long) == 4) {
            CHK(ps, 4)
            v = Integer::NewFromUnsigned(isolate,
                            (uint32_t) *((unsigned long *) ps->ptr));
            ps->ptr += 4;
        } else {
            CHK(ps, 8)
            v = Number::New(isolate, *((uint64_t *) ps->ptr));
            ps->ptr += 8;
        }
        break;
    case 'j':
        CHK(ps, 8)
        v = Number::New(isolate, *((int64_t *) ps->ptr));
        ps->ptr += 8;
        break;
    case 'J':
        CHK(ps, 8)
        v = Number::New(isolate, *((uint64_t *) ps->ptr));
        ps->ptr += 8;
        break;
    case 's': {
        // nul-terminated string
        v = String::NewFromUtf8(isolate, ps->ptr,
                       v8::NewStringType::kNormal, -1).ToLocalChecked();
        int l = v8String::Cast(v)->Utf8Length();
        CHK(ps, l+1);
        ps->ptr += (l+1);
    }
        break;
    case 'p':
        CHK(ps, sizeof (void *))
        v = WrapPtr((v8_state ) isolate->GetData(0), *((void **) ps->ptr));
        ps->ptr += sizeof (void *);
        break;
    case 'z': {
        CHK(ps, 4)
        int32_t byte_length = *((int32_t *) ps->ptr);
        ps->ptr += 4;
        CHK(ps, byte_length)
        v = String::NewFromUtf8(isolate, ps->ptr,
                        v8::NewStringType::kNormal, byte_length).ToLocalChecked();
        ps->ptr += byte_length;
    }
        break;
    case 'a': {
        CHK(ps, 4)
        int32_t byte_length = *((int32_t *) ps->ptr);
        assert(byte_length >= 0);
        ps->ptr += 4;
        CHK(ps, byte_length)
        void *data = emalloc(byte_length);
        memcpy(data, ps->ptr, byte_length);
        v = ArrayBuffer::New(isolate, data, byte_length,
                             ArrayBufferCreationMode::kInternalized);
        ps->ptr += byte_length;
    }
        break;
    default:
        ps->errcode = BadFmtSpec;
        break;
    }
    return handle_scope.Escape(v);
}

static v8Array unpack_n(pack_s *ps, char *fmt, Isolate *isolate) {
    EscapableHandleScope handle_scope(isolate);
    int c, i = 0;
    char *p0 = ps->ptr;
    v8Array arr = Array::New(isolate);

    while ((c = *fmt)) {
        if (c >= '0' && c <= '9') {
            /* " .. COUNTx .." */
            int count = c - '0';
            while ((c = *++fmt) && c >= '0' && c <= '9') {
                count = count * 10 + c - '0';
            }
            if (c != 'x') {
                ps->errcode = BadFmtSpec;
                break;
            }
            CHK(ps, count);
            ps->ptr += count;
            fmt++;
            continue;
        }
        if (c == 'x') {  /* same as "1x" */
            CHK(ps, 1);
            ps->ptr++;
            fmt++;
            continue;
        }
        v8Value v = unpack(ps, *fmt, isolate);
        if (v.IsEmpty())    /* ps->errcode set */
            break;
        arr->Set(i, v);
        i++;
        fmt++;
    }
    arr->Set(i, Number::New(isolate, (ps->ptr - p0)));
    return handle_scope.Escape(arr);
}

// pack(object, offset, format, ...)
void Pack(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 2);
    if (!args[0]->IsObject())
        ThrowTypeError(isolate, "invalid argument");
    v8Object obj = args[0]->ToObject(
                isolate->GetCurrentContext()).ToLocalChecked();
    struct pack_s ps = {0};
    ps.end = (char *) UINTPTR_MAX;
    char *ptr = nullptr;
    if (obj->IsArrayBuffer()) {
        ArrayBuffer::Contents c = v8ArrayBuffer::Cast(obj)->GetContents();
        ptr = (char *) c.Data();
        if (ptr)
            ps.end = ptr + c.ByteLength();
    } else if (GetObjectId(obj) == V8EXTPTR) {
        ptr = (char *) v8External::Cast(obj->GetInternalField(1))->Value();
    }
    if (!ptr) {
        ThrowError(isolate, "empty buffer argument");
    }

    size_t off = (size_t) args[1]->Uint32Value(
                            isolate->GetCurrentContext()).FromJust();
    ps.ptr = ptr = ptr + off;

    size_t sz = 0;
    if (argc > 2) {
        String::Utf8Value fmt_str(args[2]);
        if (fmt_str.length() > 0) {
            int r = pack_n(&ps, *fmt_str, args);
            if (r < 0)
                ThrowError(isolate, "invalid format or argument(s)");   /* FIXME separate messages */
            sz = ps.ptr - (char *) ptr;
        }
    }
    args.GetReturnValue().Set(Number::New(isolate, sz));
}

// $unpack(object, offset, format) -> [item1, ... itemN, bytes_consumed]
// Returns null if the format is invalid or empty string.
void Unpack(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 2);
    if (!args[0]->IsObject())
        ThrowTypeError(isolate, "invalid argument");
    v8Object obj = args[0]->ToObject(
                isolate->GetCurrentContext()).ToLocalChecked();
    struct pack_s ps = {0};
    ps.end = (char *) UINTPTR_MAX;
    char *ptr = nullptr;
    if (obj->IsArrayBuffer()) {
        ArrayBuffer::Contents c = v8ArrayBuffer::Cast(obj)->GetContents();
        ptr = (char *) c.Data();
        if (ptr)
            ps.end = ptr + c.ByteLength();
    } else if (GetObjectId(obj) == V8EXTPTR) {
        ptr = (char *)v8External::Cast(obj->GetInternalField(1))->Value();
    }
    if (!ptr) {
        ThrowError(isolate, "invalid ArrayBuffer argument");
    }

    size_t off = (size_t) args[1]->Uint32Value(
                            isolate->GetCurrentContext()).FromJust();
    ps.ptr = ptr = ptr + off;

    if (argc > 2) {
        String::Utf8Value fmt_str(args[2]);
        if (fmt_str.length() > 0) {
            v8Array arr = unpack_n(&ps, *fmt_str, isolate);
            if (ps.errcode < 0)
                ThrowError(isolate, "invalid format or argument(s)");   /* FIXME separate messages */
            args.GetReturnValue().Set(arr);
            return;
        }
    } else {
        args.GetReturnValue().Set(v8::Null(isolate));
    }
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
    obj->SetPrototype(v8Value::New(isolate, vm->cptr_proto));
    return handle_scope.Escape(obj);
}

void *UnwrapPtr(v8Object ptrObj) {
    return v8External::Cast(ptrObj->GetInternalField(1))->Value();
}

// ptr.offsetAt(offset).
// Usage:
// struct foo { int x; struct bar; };
// p1 = (struct foo *) ..;
// p2 = p1.offsetAt(4); // -> (struct bar *).
// p2.offsetAt(-4).equal(p1); // -> true
// XXX: may need to keep a reference to p1
// (e.g. as a property in p2 object) in JS.

static void OffsetAt(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);

    v8Object obj = args.Holder();
    if (GetObjectId(obj) != V8EXTPTR)
        ThrowTypeError(isolate, "not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (!ptr)
        ThrowError(isolate, "pointer is null");
    int off = args[0]->Int32Value(
                            isolate->GetCurrentContext()).FromJust();
    args.GetReturnValue().Set(
                WrapPtr((v8_state ) isolate->GetData(0), (char *) ptr + off));
}

static void Equal(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);

    v8Object obj1 = args.Holder();
    if (GetObjectId(obj1) != V8EXTPTR)
        ThrowTypeError(isolate, "not a pointer");
    void *ptr1 = v8External::Cast(obj1->GetInternalField(1))->Value();

    if (GetObjectId(args[0]) != V8EXTPTR)
        ThrowTypeError(isolate, "invalid argument");
    v8Object obj2 = args[0]->ToObject(
                isolate->GetCurrentContext()).ToLocalChecked();
    void *ptr2 = v8External::Cast(obj2->GetInternalField(1))->Value();

    args.GetReturnValue().Set(Boolean::New(isolate, (ptr1 == ptr2)));
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
    /* ptr_templ->Set(v8STR(isolate, "free"),
                FunctionTemplate::New(isolate, Free)); */
    ptr_templ->Set(v8STR(isolate, "notNull"),
                FunctionTemplate::New(isolate, NotNull));
    ptr_templ->Set(v8STR(isolate, "offsetAt"),
                FunctionTemplate::New(isolate, OffsetAt));
    ptr_templ->Set(v8STR(isolate, "equal"),
                FunctionTemplate::New(isolate, Equal));
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

