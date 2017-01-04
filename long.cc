///////////////////////// 64 bit integers /////////////////////
// FIXME: 64 bit platforms only. For 32 bit platforms, store low and high 32bits
// in seperate internal fields.

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <inttypes.h>
#include <math.h>

#include "v8.h"
#include "v8binding.h"
#include "vm.h"
#include "long.h"
#include "ptr.h"
#include "util.h"

using namespace v8;

extern "C" {

v8Object Int64(v8_state vm, int64_t i64) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->i64_template);
    v8Object obj = templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked();
    SetObjectId(obj, V8INT64);
    obj->SetInternalField(1, External::New(isolate,
            reinterpret_cast<void*>(static_cast<intptr_t>(i64))));
    obj->SetPrototype(v8Value::New(isolate, vm->long_proto));
    return handle_scope.Escape(obj);
}

v8Object UInt64(v8_state vm, uint64_t ui64) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->ui64_template);
    v8Object obj = templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked();
    SetObjectId(obj, V8UINT64);
    obj->SetInternalField(1, External::New(isolate,
            reinterpret_cast<void*>(static_cast<uintptr_t>(ui64))));
    obj->SetPrototype(v8Value::New(isolate, vm->long_proto));
    return handle_scope.Escape(obj);
}

static inline void SetUInt64(v8Value v, uint64_t ui64) {
    v8Object obj = v8Object::Cast(v);
    obj->SetInternalField(1,
            External::New(obj->CreationContext()->GetIsolate(),
                reinterpret_cast<void*>(static_cast<uintptr_t>(ui64))));
}

int LongValue(v8Value v, long64 *l) {
    v8Object obj;
    if (v->IsObject()
            && (obj = v8Object::Cast(v))->InternalFieldCount() == 2) {
        int id = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(
                obj->GetAlignedPointerFromInternalField(0))) >> 2;
        if (id == V8INT64) {
            l->val.i64 = static_cast<int64_t>(
                reinterpret_cast<intptr_t>(v8External::Cast(
                    obj->GetInternalField(1))->Value()
                )
            );
            l->issigned = 1;
            return 1;
        }
        if (id == V8UINT64) {
            l->val.u64 = static_cast<uint64_t>(
                reinterpret_cast<uintptr_t>(v8External::Cast(
                    obj->GetInternalField(1))->Value()
                )
            );
            l->issigned = 0;
            return 1;
        }
    }
    return 0;
}

enum LongCmd {
    LONG_ISLONG = 1,
    LONG_ISUINT,
    LONG_TOSTRING,
    LONG_TONUMBER,
    LONG_LOW32,
    LONG_HIGH32,
    LONG_LOW32U,
    LONG_HIGH32U,
    LONG_NOT,
    LONG_COMPL,
    LONG_LSHIFT,
    LONG_RSHIFT,
    LONG_LRSHIFT,   // logical (unsigned) right shift
    LONG_PACK,
    LONG_UNPACK,
    LONG_EQUAL,
    LONG_ADD,
    LONG_SUB,
    LONG_OR,
    LONG_AND,
};

static struct lcmd_s {
    char *name;
    enum LongCmd cmd;
} long_cmds[] = {
    { (char *) "isLong", LONG_ISLONG },
    { (char *) "isUnsigned", LONG_ISUINT },
    { (char *) "toString", LONG_TOSTRING },
    { (char *) "toNumber", LONG_TONUMBER },
    { (char *) "low32", LONG_LOW32 },
    { (char *) "high32", LONG_HIGH32 },
    { (char *) "low32u", LONG_LOW32U },
    { (char *) "high32u", LONG_HIGH32U },
    { (char *) "not", LONG_NOT },
    { (char *) "compl", LONG_COMPL },
    { (char *) "lshift", LONG_LSHIFT },
    { (char *) "rshift", LONG_RSHIFT },
    { (char *) "lrshift", LONG_LRSHIFT },
    { (char *) "pack", LONG_PACK },
    { (char *) "unpack", LONG_UNPACK },
    { (char *) "eq", LONG_EQUAL },
    { (char *) "add", LONG_ADD },
    { (char *) "sub", LONG_SUB },
    { (char *) "or", LONG_OR },
    { (char *) "and", LONG_AND },
    {0}
};

static v8Value LongOp2(v8_state vm,
            long64 *i1, long64 *i2, enum LongCmd op) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    switch (op) {
    case LONG_EQUAL: {
        bool isequal;
        if (i1->issigned != i2->issigned
                && (i1->val.uhigh >> 31) && (i2->val.uhigh >> 31))
            isequal = false;
        else
            isequal = (i1->val.high == i2->val.high)
                    && (i1->val.low == i2->val.low);
        return handle_scope.Escape(Boolean::New(isolate, isequal));
    }
    case LONG_ADD:
        if (i1->issigned) {
            if (i2->issigned)
                i1->val.i64 += i2->val.i64;
            else
                i1->val.i64 += i2->val.u64;
            return handle_scope.Escape(Int64(vm, i1->val.i64));
        }
        if (i2->issigned)
            i1->val.u64 += i2->val.i64;
        else
            i1->val.u64 += i2->val.u64;
        return handle_scope.Escape(UInt64(vm, i1->val.u64));
    case LONG_SUB:
        if (i1->issigned) {
            if (i2->issigned)
                i1->val.i64 -= i2->val.i64;
            else
                i1->val.i64 -= i2->val.u64;
            return handle_scope.Escape(Int64(vm, i1->val.i64));
        }
        if (i2->issigned)
            i1->val.u64 -= i2->val.i64;
        else
            i1->val.u64 -= i2->val.u64;
        return handle_scope.Escape(UInt64(vm, i1->val.u64));
    case LONG_OR:
        if (i1->issigned) {
            if (i2->issigned)
                i1->val.i64 |= i2->val.i64;
            else
                i1->val.i64 |= i2->val.u64;
            return handle_scope.Escape(Int64(vm, i1->val.i64));
        }
        if (i2->issigned)
            i1->val.u64 |= i2->val.i64;
        else
            i1->val.u64 |= i2->val.u64;
        return handle_scope.Escape(UInt64(vm, i1->val.u64));
    case LONG_AND:
        if (i1->issigned) {
            if (i2->issigned)
                i1->val.i64 &= i2->val.i64;
            else
                i1->val.i64 &= i2->val.u64;
            return handle_scope.Escape(Int64(vm, i1->val.i64));
        }
        if (i2->issigned)
            i1->val.u64 &= i2->val.i64;
        else
            i1->val.u64 &= i2->val.u64;
        return handle_scope.Escape(UInt64(vm, i1->val.u64));
    default:
        break;
    }
    return handle_scope.Escape(v8::Undefined(isolate));
}

//$lcntl(Int64/UInt64, LongCmd [, arg2])
void LongCntl(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    enum LongCmd cmd = LONG_TOSTRING;
    if (argc > 1) {
        cmd = static_cast<enum LongCmd>(args[1]->ToInt32(
                    isolate->GetCurrentContext()).ToLocalChecked()->Value());
    }
    long64 i1;
    bool isLong = LongValue(args[0], &i1);
    if (!isLong && cmd != LONG_ISLONG) {
        if (/*argc > 1 &&*/ cmd == LONG_ISUINT) {
            args.GetReturnValue().Set(v8::False(isolate));
            return;
        }
        ThrowTypeError(isolate, "$lcntl: not a 64-bit integer");
    }

    switch (cmd) {
    case LONG_ISLONG:
        args.GetReturnValue().Set(Boolean::New(isolate, isLong));
        break;
    case LONG_ISUINT:
        args.GetReturnValue().Set(Boolean::New(isolate, !i1.issigned));
        break;
    case LONG_TOSTRING: {
        char buf[32];
        if (i1.issigned)
            sprintf(buf, "%" PRId64, i1.val.i64);
        else
            sprintf(buf, "%" PRIu64, i1.val.u64);
        args.GetReturnValue().Set(v8STR(isolate, buf));
    }
        break;
    case LONG_TONUMBER: {
        double d = i1.issigned ? (double)i1.val.i64 : (double)i1.val.u64;
        args.GetReturnValue().Set(Number::New(isolate, d));
    }
        break;
    case LONG_LOW32:
        args.GetReturnValue().Set(Integer::New(isolate, i1.val.low));
        break;
    case LONG_HIGH32:
        args.GetReturnValue().Set(Integer::New(isolate, i1.val.high));
        break;
    case LONG_LOW32U:
        args.GetReturnValue().Set(
                        Integer::NewFromUnsigned(isolate, i1.val.ulow));
        break;
    case LONG_HIGH32U:
        args.GetReturnValue().Set(
                        Integer::NewFromUnsigned(isolate, i1.val.uhigh));
        break;
    case LONG_NOT:  // == 0 ?
        args.GetReturnValue().Set(
                    Boolean::New(isolate, !i1.val.i64));
        break;
    case LONG_COMPL:
        if (i1.issigned)
            args.GetReturnValue().Set(Int64(vm, ~i1.val.i64));
        else
            args.GetReturnValue().Set(UInt64(vm, ~i1.val.u64));
        break;
    case LONG_LSHIFT:
    case LONG_RSHIFT:
    case LONG_LRSHIFT: {
        uint32_t n;
        if (argc < 3)
            ThrowNotEnoughArgs(isolate, true);
        n = args[2]->ToUint32(
                isolate->GetCurrentContext()).ToLocalChecked()->Value();
        n &= 63;
        if (n == 0) {
            args.GetReturnValue().Set(args[0]);
        } else if (cmd == LONG_LRSHIFT) {
            uint64_t result = (i1.val.u64 >> n);
            if (i1.issigned)
                args.GetReturnValue().Set(Int64(vm, (int64_t)result));
            else
                args.GetReturnValue().Set(UInt64(vm, result));
        } else if (i1.issigned) {
            int64_t result = (cmd == LONG_LSHIFT) ? (i1.val.i64 << n)
                    : (i1.val.i64 >> n);
            args.GetReturnValue().Set(Int64(vm, result));
        } else {
            uint64_t result = (cmd == LONG_LSHIFT) ? (i1.val.u64 << n)
                    : (i1.val.u64 >> n);
            args.GetReturnValue().Set(UInt64(vm, result));
        }
    }
        break;
    case LONG_EQUAL:
    case LONG_OR:
    case LONG_AND:
    case LONG_SUB:
    case LONG_ADD: {
        if (argc < 3)
            ThrowNotEnoughArgs(isolate, true);
        long64 i2;
        if (!LongValue(args[2], &i2))
            ThrowTypeError(isolate,
                    "$lcntl: 'long' argument (#3) expected");
        args.GetReturnValue().Set(LongOp2(vm, &i1, &i2, cmd));
    }
        break;
    case LONG_PACK:
    case LONG_UNPACK: /* {
        FIXME -- arraybuffer ..
        if (argc < 3)
            ThrowNotEnoughArgs(isolate, true);
        v8Object obj = ToPtr(args[2]);
        if (obj.IsEmpty())
            ThrowTypeError(isolate,
                    "$lcntl: pointer argument (#3) expected");
        void *ptr = UnwrapPtr(obj);
        if (!ptr)
            ThrowError(isolate, "$lcntl: pointer argument is null");
        size_t off = 0;
        if (argc > 3)
            off = args[3]->ToUint32(
                    isolate->GetCurrentContext()).ToLocalChecked()->Value();
        ptr = (char *) ptr + off;
        if (cmd == LONG_PACK) {
            *(reinterpret_cast<uint64_t *>(ptr)) = i1.val.u64;
        } else {
            i1.val.u64 = *(reinterpret_cast<uint64_t *>(ptr));
            SetUInt64(args[0], i1.val.u64);
        }
        args.GetReturnValue().Set(Integer::New(isolate, 8));
    } */
        break;
    default:
        ThrowError(isolate, "$lcntl: 'cmd' argument is invalid");
    }
}

// $long(string|number, issigned = true)
// $long(signed 32-bit low, signed 32-bit high, issigned)
// $long(object, ..) -> 0
static void Long(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8Context context = isolate->GetCurrentContext();
    int issigned = 1;
    long64 i1;
    i1.val.low = i1.val.high = 0;

    if (argc >= 3) {
        issigned = args[2]->BooleanValue(context).FromJust();
        // Int32Value(object|NAN etc.) -> 0
        // Int32Value('1') -> 1
        i1.val.low = args[0]->Int32Value(context).FromJust();
        i1.val.high = args[1]->Int32Value(context).FromJust();
    } else {
        if (argc > 1)
            issigned = args[1]->BooleanValue(context).FromJust();
        if (LongValue(args[0], &i1) && (i1.issigned == issigned)) {
            args.GetReturnValue().Set(args[0]);
            return;
        }
        if (args[0]->IsNumber()) {
            double d = args[0]->NumberValue(context).FromJust();
            if (isfinite(d)) {
                if (issigned)
                    i1.val.i64 = d;
                else
                    i1.val.u64 = d;
            }
        } else {
            v8String s = args[0]->ToString(context).ToLocalChecked();
            String::Utf8Value stval(s);
            if (*stval) {
                errno = 0;
                if (issigned) {
                    i1.val.i64 = (int64_t) strtoll(*stval, nullptr, 0);
                    if (errno != 0)
                        i1.val.i64 = 0;
                } else {
                    i1.val.u64 = (uint64_t) strtoull(*stval, nullptr, 0);
                    if (errno != 0)
                        i1.val.u64 = 0;
                }
            }
        }
    }
    if (issigned)
        args.GetReturnValue().Set(Int64(vm, i1.val.i64));
    else
        args.GetReturnValue().Set(UInt64(vm, i1.val.u64));
}

Local<FunctionTemplate> LongTemplate(v8_state vm) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);

    Local<FunctionTemplate> long_templ = FunctionTemplate::New(isolate, Long);
    for (unsigned i = 0; long_cmds[i].name; i++) {
        // Only primitive values are allowed!
        long_templ->Set(v8STR(isolate, long_cmds[i].name),
                Integer::New(isolate, long_cmds[i].cmd));
    }
    return handle_scope.Escape(long_templ);
}

}

/*

function Long(x, unsigned) {
    return $long(x, ~~unsigned);
}

Long.prototype = $long(0).__proto__;
Long.prototype.constructor = Long;

Long.prototype.add = function(x) {
    return $lcntl(this, $long.add, x);
};

$print(Long(0) instanceof Long); // -> true
$print($long("0") instanceof Long); // -> true
var i1 = Long(-1);
var i2 = $long("1");
var i3 = i1.add(i2);
$print($lcntl(i3)); // -> 0
$print(i3 instanceof Long); //-> true

*/
