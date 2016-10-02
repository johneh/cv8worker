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
#include "util.h"

using namespace v8;

extern "C" {

v8Object Int64(js_vm *vm, int64_t i64) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->i64_template);
    v8Object obj = templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked();
    obj->SetAlignedPointerInInternalField(0,
            reinterpret_cast<void*>(static_cast<uintptr_t>(V8INT64<<2)));
    obj->SetInternalField(1, External::New(isolate,
            reinterpret_cast<void*>(static_cast<intptr_t>(i64))));
    return handle_scope.Escape(obj);
}

v8Object UInt64(js_vm *vm, uint64_t ui64) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->ui64_template);
    v8Object obj = templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked();
    obj->SetAlignedPointerInInternalField(0,
             reinterpret_cast<void*>(static_cast<uintptr_t>(V8UINT64<<2)));
    obj->SetInternalField(1, External::New(isolate,
            reinterpret_cast<void*>(static_cast<uintptr_t>(ui64))));
    return handle_scope.Escape(obj);
}

bool IsInt64(v8Value v) {
    if (v->IsObject()
        && v8Object::Cast(v)->InternalFieldCount() == 2
        && (V8INT64 == static_cast<int>(reinterpret_cast<uintptr_t>(
                v8Object::Cast(v)->GetAlignedPointerFromInternalField(0)) >> 2))
    )
        return true;
    return false;
}

int64_t GetInt64(v8Value v) {
    return static_cast<int64_t>(
            reinterpret_cast<intptr_t>(v8External::Cast(
                    v8Object::Cast(v)->GetInternalField(1))->Value()
            )
    );
}

bool IsUInt64(v8Value v) {
    if (v->IsObject()
        && v8Object::Cast(v)->InternalFieldCount() == 2
        && (V8UINT64 == static_cast<int>(reinterpret_cast<uintptr_t>(
                v8Object::Cast(v)->GetAlignedPointerFromInternalField(0)) >> 2))
    )
        return true;
    return false;
}

uint64_t GetUInt64(v8Value v) {
    return static_cast<uint64_t>(
            reinterpret_cast<uintptr_t>(v8External::Cast(
                v8Object::Cast(v)->GetInternalField(1))->Value()
            )
    );
}

int LongValue(v8Value v, Long64 *val) {
    if (v->IsObject()
            && v8Object::Cast(v)->InternalFieldCount() == 2) {
        int id = static_cast<int>(reinterpret_cast<uintptr_t>(
                v8Object::Cast(v)->GetAlignedPointerFromInternalField(0)) >> 2);
        if (id == V8INT64) {
            val->i64 = static_cast<int64_t>(
                reinterpret_cast<intptr_t>(v8External::Cast(
                    v8Object::Cast(v)->GetInternalField(1))->Value()
                )
            );
            return V8INT64;
        }
        if (id == V8UINT64) {
            val->u64 = static_cast<uint64_t>(
                reinterpret_cast<uintptr_t>(v8External::Cast(
                    v8Object::Cast(v)->GetInternalField(1))->Value()
                )
            );
            return V8UINT64;
        }
    }
    return 0;
}

enum LongCmd {
    LONG_ISINT = 1,
    LONG_ISUINT,
    LONG_TOSTRING,
    LONG_TONUMBER,
    LONG_LOW32,
    LONG_HIGH32,
    LONG_LOW32U,
    LONG_HIGH32U,
    LONG_ISZERO,
    LONG_EQUAL,
};

static struct lcmd_s {
    char *name;
    enum LongCmd cmd;
} long_cmds[] = {
    { (char *) "isInt64", LONG_ISINT },
    { (char *) "isUint64", LONG_ISUINT },
    { (char *) "toString", LONG_TOSTRING },
    { (char *) "toNumber", LONG_TONUMBER },
    { (char *) "low32", LONG_LOW32 },
    { (char *) "high32", LONG_HIGH32 },
    { (char *) "low32u", LONG_LOW32U },
    { (char *) "high32u", LONG_HIGH32U },
    { (char *) "isZero", LONG_ISZERO },
    { (char *) "eq", LONG_EQUAL },
    {0}
};

//$lcntl(Int64/UInt64, LongCmd)
void LongCntl(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    union integer64_u ival;
    bool issigned = false;
    enum LongCmd cmd = LONG_TOSTRING;
    if (argc > 1) {
        cmd = static_cast<enum LongCmd>(args[1]->ToInt32(
                    isolate->GetCurrentContext()).ToLocalChecked()->Value());
    }

    if (IsInt64(args[0])) {
        issigned = true;
        ival.i64 = GetInt64(args[0]);
    } else if (IsUInt64(args[0])) {
        ival.u64 = GetUInt64(args[0]);
    } else {
        if (argc > 1 && (cmd == LONG_ISINT || cmd == LONG_ISUINT)) {
            args.GetReturnValue().Set(v8::False(isolate));
            return;
        }
        ThrowTypeError(isolate, "$lcntl: not a 64-bit integer");
    }

    switch (cmd) {
    case LONG_ISINT:
        args.GetReturnValue().Set(Boolean::New(isolate, issigned));
        break;
    case LONG_ISUINT:
        args.GetReturnValue().Set(Boolean::New(isolate, !issigned));
        break;
    case LONG_TOSTRING: {
        char buf[32];
        if (issigned)
            sprintf(buf, "%" PRId64, ival.i64);
        else
            sprintf(buf, "%" PRIu64, ival.u64);
        args.GetReturnValue().Set(v8_str(isolate, buf));
    }
        break;
    case LONG_TONUMBER: {
        double d = issigned ? (double)ival.i64 : (double)ival.u64;
        args.GetReturnValue().Set(Number::New(isolate, d));
    }
        break;
    case LONG_LOW32:
        args.GetReturnValue().Set(Integer::New(isolate, ival.low));
        break;
    case LONG_HIGH32:
        args.GetReturnValue().Set(Integer::New(isolate, ival.high));
        break;
    case LONG_LOW32U:
        args.GetReturnValue().Set(
                        Integer::NewFromUnsigned(isolate, ival.ulow));
        break;
    case LONG_HIGH32U:
        args.GetReturnValue().Set(
                        Integer::NewFromUnsigned(isolate, ival.uhigh));
        break;
    case LONG_ISZERO:
        args.GetReturnValue().Set(
                    Boolean::New(isolate, !ival.i64));
        break;
    case LONG_EQUAL: {
        union integer64_u ival2;
        bool issigned2 = false;
        if (argc < 3)
            ThrowNotEnoughArgs(isolate, true);
        if (IsInt64(args[2])) {
            issigned2 = true;
            ival2.i64 = GetInt64(args[2]);
        } else if (IsUInt64(args[2])) {
            ival2.u64 = GetUInt64(args[2]);
        } else
            ThrowTypeError(isolate,
                    "$lcntl: 'long' argument (#3) expected");
        bool isequal;
        if (issigned != issigned2 && (ival.uhigh >> 31) && (ival2.uhigh >> 31))
            isequal = false;
        else
            isequal = (ival.high == ival2.high) && (ival.low == ival2.low); 
        args.GetReturnValue().Set(
                    Boolean::New(isolate, isequal));
    }
        break;
    default:
        ThrowError(isolate, "$lcntl: 'cmd' argument is invalid");
    }
}

// $toLong("string")
// $toLong(number)
// $toLong([signed 32-bit low, high])
static void ToLong(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8Context context = isolate->GetCurrentContext();
    bool issigned = true;
    if (argc > 1)
        issigned = !args[1]->BooleanValue(context).FromJust();

    union integer64_u ival;
    ival.low = ival.high = 0;
    if (args[0]->IsObject()) {  // likely array
        v8Object obj = v8Object::Cast(args[0]);
        ival.low = obj->Get(context, 0).ToLocalChecked()
                        ->Int32Value(context).FromJust();
        ival.high = obj->Get(context, 1).ToLocalChecked()
                        ->Int32Value(context).FromJust();
    } else if (args[0]->IsNumber()) {
        double d = args[0]->NumberValue(context).FromJust();
        if (isfinite(d)) {
            if (issigned)
                ival.i64 = d;
            else
                ival.u64 = d;
        }
    } else {
        v8String s = args[0]->ToString(context).ToLocalChecked();
        String::Utf8Value stval(s);
        if (*stval) {
            errno = 0;
            if (issigned) {
                ival.i64 = (int64_t) strtoll(*stval, nullptr, 0);
                if (errno != 0)
                    ival.i64 = 0;
            } else {
                ival.u64 = (uint64_t) strtoull(*stval, nullptr, 0);
                if (errno != 0)
                    ival.u64 = 0;
            }
        }
    }
    if (issigned)
        args.GetReturnValue().Set(Int64(vm, ival.i64));
    else
        args.GetReturnValue().Set(UInt64(vm, ival.u64));
}

Local<FunctionTemplate> LongTemplate(js_vm *vm) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    Local<FunctionTemplate> long_templ = FunctionTemplate::New(isolate, ToLong);
    for (unsigned i = 0; long_cmds[i].name; i++) {
        // Only primitive values are allowed!
        long_templ->Set(v8_str(isolate, long_cmds[i].name),
                Integer::New(isolate, long_cmds[i].cmd));
    }
    return handle_scope.Escape(long_templ);
}

}

