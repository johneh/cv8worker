#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <math.h>

#include "v8.h"
#include "libplatform/libplatform.h"

#include "v8binding.h"
#include "vm.h"
#include "util.h"
#include "long.h"
#include "ptr.h"
#include "store.h"

using namespace v8;

class ArrayBufferAllocator : public ArrayBuffer::Allocator {
 public:
    virtual void* Allocate(size_t length) {
        void* data = AllocateUninitialized(length);
        return data == NULL ? data : memset(data, 0, length);
    }
    virtual void* AllocateUninitialized(size_t length) {
        return malloc(length);
    }
    virtual void Free(void* data, size_t) { free(data); }
};


char *GetExceptionString(Isolate* isolate, TryCatch* try_catch) {
  std::string s;
  HandleScope handle_scope(isolate);
  String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = *exception ? *exception : "unknown error";
  v8Message message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    s.append(exception_string);
  } else {
    // Print (filename):(line number)
    String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
    v8Context context(isolate->GetCurrentContext());
    const char* filename_string = *filename ? *filename : "?";
    int linenum = message->GetLineNumber(context).FromJust();
    char linenum_string[32];

    sprintf(linenum_string, "%i:", linenum);

    s.append(filename_string);
    s.append(":");
    s.append(linenum_string);
    s.append(exception_string);
  }
  return estrdup(s.c_str(), s.length());
}


static ArrayBufferAllocator allocator;
static Isolate::CreateParams create_params;

extern "C" {
#include "jsv8.h"
#define V8_BINDING
#include "jsv8dlfn.h"

#ifdef V8TEST
#define DPRINT(format, args...) \
fprintf(stderr, format , ## args);
#else
#define DPRINT(format, args...) /* nothing */
#endif

static v8_val global_;
static v8_val null_;
static v8_val nullptr_;

static void v8_to_ctype(v8_state vm, v8Value v, v8_val *ctval);
static v8Value ctype_to_v8(v8_state vm, v8_val *ctval);
static v8_val ctype_handle(v8_handle h);
static void v8_reset(v8_state vm, v8_val val);
static v8_val v8_ctypestr(const char *stp, unsigned stlen);

static int SetFinalizer(v8_state vm, v8Object ptrObj, Fnfree free_func);

static inline void DeletePersister(v8_state vm, v8_handle h) {
    if (h > NUM_PERMANENT_HANDLES)
        vm->store_->Dispose(h);
}

/* val _should_ not be null, undef, $global or $nullptr. */
#define NewPersister(vm, val)   (vm)->store_->Set(val)

#define GetValueFromPersister(vm, h)    (vm)->store_->Get(h)

static void Panic(const char *errstr) {
    fprintf(stderr, "%s\n", errstr);
    abort();
    exit(1);
}

////////////////////////////////// Coroutine //////////////////////////////////

#define GOROUTINE_INTERNAL_FIELD_COUNT 2
enum {
    GoDone = (1 << 0),
    GoResolve = (1 << 1),
    GoReject = (1 << 2),
};

struct GoCallback {
    v8_coro g;
    v8_val result;
    int flags;
};

struct Go_s {
    v8_state vm;
    v8_ffn *fndef;
    union {
        void *fp;
        v8_handle hcb;
    };
    v8_handle hpr;  /* Promise Resolver */
    int argc;
    v8_val args[MAXARGS];
};

static v8Object WrapGo(v8_state vm, v8_ffn *fndef) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ = v8ObjectTemplate::New(isolate, vm->go_template);
    v8Object obj = templ->NewInstance(
                        isolate->GetCurrentContext()).ToLocalChecked();
    SetObjectId(obj, V8GO);
    obj->SetInternalField(1, External::New(isolate, fndef));
    return handle_scope.Escape(obj);
}

static v8Object ToGo(v8Value v) {
    if (!v.IsEmpty() && v->IsObject()) {
        v8Object cr = v8Object::Cast(v);
        if (cr->InternalFieldCount() == GOROUTINE_INTERNAL_FIELD_COUNT
            && (V8GO == (static_cast<uint16_t>(reinterpret_cast<uintptr_t>(
                    cr->GetAlignedPointerFromInternalField(0))) >> 2))
        )
        return cr;
    }
    return v8Object();  // Empty handle
}

// Start a $go() coroutine in the main thread.
coroutine static void start_go(mill_pipe inq) {
    while (true) {
        int done;
        struct Go_s *g = *((struct Go_s **) mill_piperecv(inq, &done));
        if (done)
            break;
        go(((Fngo)g->fndef->fp)(g->vm, g, g->argc, g->args));
    }
}

// Receive coroutine callbacks from the main thread.
coroutine static void recv_go(v8_state vm) {
    while (true) {
        int done;
        GoCallback *cb = *((GoCallback **) mill_piperecv(vm->outq, &done));
        if (done) {
            mill_pipefree(vm->outq);
            break;
        }
        /* Send to the channel to be processed later in WaitFor(). */
        int rc = mill_chs(vm->ch, &cb);
        assert(rc == 0);
    }
}

static void send_go(v8_state vm, Go_s *g) {
    int rc = mill_pipesend(vm->inq, (void *) &g);
    if (rc == -1) {
        fprintf(stderr, "v8binding.cc:send_go(): writing to pipe failed\n");
        Panic(strerror(errno));
    }
}

static int v8_goresolve(v8_state vm, v8_coro cr, v8_val data, int done);

// goroutine wpapper for a C-type function running in non-V8 thread
// N.B.: cannot reliably read errno set by the function.
coroutine static void do_cgo(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    FnCtype fp = (FnCtype) (((struct Go_s *) cr)->fp);
    v8_val retv = fp(vm, argc, args);
    v8_goresolve(vm, cr, retv, 1);
}

static v8_ffn cgodef = { 0, (void *) do_cgo, "__cgo__", FN_CORO };

// $go(coro [, in1 [, in2 .. in12 ]])
// $go(coro [, in1 [, in2 .. in12 ]], fn)
//   fn-> function(result, done (= true|false))
static void Go(const FunctionCallbackInfo<Value>& args) {
    Go_s *g;
    v8_state vm;
    Isolate *isolate;

    if (true) { // start V8 scope

    isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    int argc = args.Length();
    ThrowNotEnoughArgs(isolate, argc < 1);
    vm = reinterpret_cast<v8_state>(isolate->GetData(0));
    v8_ffn *fndef;

    v8Object coro = ToGo(args[0]);
    if (coro.IsEmpty()) {
        if (GetObjectId(vm, args[0]) != V8EXTFUNC)
            ThrowTypeError(isolate, "$go argument #1: coroutine expected");
        // use the do_cgo wrapper to run in the other thread.
        fndef = reinterpret_cast<v8_ffn *>(
                    v8External::Cast(
                        v8Object::Cast(args[0])->GetInternalField(1)
                    )->Value()
            );
    } else {
        fndef = reinterpret_cast<v8_ffn *>(
            v8External::Cast(coro->GetInternalField(1))->Value());
    }

    if (fndef->type == FN_COROPUSH) {
        if (argc < 2 || !args[argc-1]->IsFunction())
            ThrowTypeError(isolate,
                "$go: callback (last argument) is not a function");
        argc--;
    }

    argc--;
    if (argc > MAXARGS || argc != fndef->pcount)
        ThrowError(isolate,
                "$go: incorrect number of arguments for the goroutine");

    g = new Go_s;
    g->vm = vm;
    g->hcb = 0;
    g->argc = argc;
    for (int i = 0; i < argc; i++) {
        v8_to_ctype(vm, args[i+1], &g->args[i]);
    }

    if (fndef->type == FN_COROPUSH)
        g->hcb = NewPersister(vm, args[argc+1]);
    if (fndef->type == FN_CTYPE) {
        g->fp = fndef->fp;
        g->fndef = &cgodef;
    } else
        g->fndef = fndef;

    Local<v8::Promise::Resolver> pr = Promise::Resolver::New(
                isolate->GetCurrentContext()).ToLocalChecked();

    args.GetReturnValue().Set(pr->GetPromise());

    g->hpr = NewPersister(vm, pr);
    vm->ncoro++;

    }   // end V8 scope

    send_go(vm, g);
}

static bool FinishGo(v8_state vm, GoCallback *cb) {
    vm->ncoro--;
    Go_s *g = reinterpret_cast<Go_s *>(cb->g);
    for (int i = 0; i < g->argc; i++) {
        v8_reset(vm, g->args[i]);
    }
    if (g->fndef->type == FN_COROPUSH)
        vm->store_->Dispose(g->hcb);
    v8_reset(vm, cb->result);
    vm->store_->Dispose(g->hpr);
    delete g;
    delete cb;
    return true;
}

static inline void ResolveGo(Isolate *isolate, Go_s *g, v8Value val) {
    Local<v8::Promise::Resolver>::Cast(GetValueFromPersister(g->vm, g->hpr))
        -> Resolve(isolate->GetCurrentContext(), val).FromJust();
    // Empty the Microtask Work Queue.
    isolate->RunMicrotasks();
}

static inline void RejectGo(Isolate *isolate, Go_s *g, v8Value val) {
    Local<v8::Promise::Resolver>::Cast(GetValueFromPersister(g->vm, g->hpr))
        -> Reject(isolate->GetCurrentContext(), val).FromJust();
    // Empty the Microtask Work Queue.
    isolate->RunMicrotasks();
}

static bool RunGoCallback(v8_state vm, GoCallback *cb) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    // GetCurrentContext() is NULL if called from WaitFor().
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    Go_s *g = reinterpret_cast<Go_s *>(cb->g);
    v8_ffn *fndef = g->fndef;

    if (cb->flags & GoReject) {
        char *message = cb->result.stp;
        assert(message);
        RejectGo(isolate, g, Exception::Error(v8STR(isolate, message)));
        return FinishGo(vm, cb);
    }

    v8Value retv = ctype_to_v8(vm, &cb->result);    // V8_VOID return -> JS undefined 
    if (retv.IsEmpty()) // invalid persistent handle
        Panic("$go: received invalid resolve value");

    if (fndef->type == FN_CORO) {
        ResolveGo(isolate, g, retv);
        return FinishGo(vm, cb);
    }
    if (fndef->type == FN_COROPULL) {
        /* "Pull" stream */
        v8Object robj = Object::New(isolate);
        robj->Set(context, v8STR(isolate, "done"),
                    Boolean::New(isolate, cb->flags & GoDone));
        robj->Set(context, v8STR(isolate, "value"), retv);
        ResolveGo(isolate, g, robj);
        return FinishGo(vm, cb);
    }

    assert(g->hcb);

    /* "Push" stream */
    TryCatch try_catch(isolate);

    bool done = (cb->flags & GoDone) != 0;
    v8Value args[2];
    args[0] = retv;
    args[1] = Boolean::New(isolate, done);

    v8Value result = v8Function::Cast(GetValueFromPersister(vm, g->hcb))
                        ->Call(context->Global(), 2, args);

    if (done) {
        if (try_catch.HasCaught()) {
            RejectGo(isolate, g, Exception::Error(
                    v8STR(isolate, GetExceptionString(isolate, &try_catch))));
        } else {
            /* Using final return from the callback as the settled value */
            ResolveGo(isolate, g, result);
        }
        return FinishGo(vm, cb);
    }

    /* XXX: swallow intermediate exceptions !!! */
    v8_reset(vm, cb->result);
    delete cb;
    return false;
}

static void WaitFor(v8_state vm) {
    while (vm->ncoro > 0) {
        /* fprintf(stderr, [[ WaitFor: %d unfinished coroutines. ]]\n", vm->ncoro);*/
        GoCallback *cb = chr(vm->ch, GoCallback *);
        assert(cb);
        RunGoCallback(vm, cb);
    }
}

static void MakeGoTemplate(v8_state vm) {
    Isolate *isolate = vm->isolate;
    HandleScope handle_scope(isolate);
    v8ObjectTemplate templ = ObjectTemplate::New(isolate);
    templ->SetInternalFieldCount(GOROUTINE_INTERNAL_FIELD_COUNT);
    vm->go_template.Reset(isolate, templ);
}

// called from non-V8 thread.
static int v8_goresolve(v8_state vm, v8_coro cr, v8_val data, int done) {
    GoCallback *cb = new GoCallback;
    cb->g = cr;
    cb->result = data;
    cb->flags = (GoResolve|!!done);
    int rc = mill_pipesend(vm->outq, &cb);
    assert(rc == 0);
    return (rc == 0);
}

// called from non-V8 thread.
static int v8_goreject(v8_state vm, v8_coro cr, const char *message) {
    GoCallback *cb = new GoCallback;
    cb->g = cr;
    cb->result = v8_ctypestr(message, strlen(message));
    cb->flags = GoReject;
    int rc = mill_pipesend(vm->outq, &cb);
    assert(rc == 0);
    return (rc == 0);
}

///////////////////////////////////End Coroutine////////////////////////////////

static v8Object WrapFunc(v8_state vm, v8_ffn *func_item) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope scope(isolate);
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->extfunc_template);
    v8Object fnObj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    SetObjectId(fnObj, V8EXTFUNC);
    fnObj->SetInternalField(1, External::New(isolate, (void *)func_item));
    fnObj->SetPrototype(v8Value::New(isolate, vm->cfunc_proto));
    return scope.Escape(fnObj);
}

static void Print(const FunctionCallbackInfo<Value>& args) {
    bool first = true;
    int errcount = 0;
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);

    assert(!isolate->GetCurrentContext().IsEmpty());

    for (int i = 0; i < args.Length() && errcount == 0; i++) {
        if (first) {
            first = false;
        } else {
            printf(" ");
        }
        v8String s;
        if (!args[i]->ToString(isolate->GetCurrentContext()).ToLocal(&s)) {
            errcount++;
        } else {
            String::Utf8Value str(s);
            int n = static_cast<int>(
                        fwrite(*str, sizeof(**str), str.length(), stdout));
            if (n != str.length())
                errcount++;
        }
    }
    if (!errcount) {
        printf("\n");
        fflush(stdout);
    } else
        ThrowError(isolate, "error writing to stdout");
}

static void Now(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    int64_t n = now();
    args.GetReturnValue().Set(Number::New(isolate, n));
}

static void Close(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    mill_pipeclose(vm->inq);
}

// $eval(string, origin) -- Must compile and run.
static void EvalString(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);

    int argc = args.Length();
    if (argc < 2 || !args[0]->IsString()) {
        Panic("$eval: invalid argument(s)");
    }
    v8Context context = isolate->GetCurrentContext();
    TryCatch try_catch(isolate);
    v8String source = v8String::Cast(args[0]);
    v8String name;
    if (!args[1]->ToString(context).ToLocal(&name)) {
        Panic(GetExceptionString(isolate, &try_catch));
    }

    ScriptOrigin origin(name);
    v8Script script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        // XXX: don't panic
        fprintf(stderr, "%s\n", GetExceptionString(isolate, &try_catch));
        exit(1);
    }
    v8Value result;
    if (!script->Run(context).ToLocal(&result)) {
        fprintf(stderr, "%s\n", GetExceptionString(isolate, &try_catch));
        exit(1);
    }
    assert(!result.IsEmpty());
    args.GetReturnValue().Set(result);
}

// malloc(size [, zerofill] )
// TODO: adjust GC allocation amount.
static void Malloc(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8Context context = isolate->GetCurrentContext();
    int size = args[0]->Int32Value(context).FromJust();
    if (size <= 0)
        ThrowError(isolate, "$malloc: invalid size argument");
    void *ptr = emalloc(size);
    if (argc > 1 && args[1]->BooleanValue(context).FromJust())
        memset(ptr, '\0', size);
    v8Object ptrObj = WrapPtr(
            reinterpret_cast<js_vm*>(isolate->GetData(0)), ptr);
    SetCtypeSize(ptrObj, size);
    args.GetReturnValue().Set(ptrObj);
}

//
// N.B.: Equals() corresponds to == in JS, StrictEquals() to ===
// and SameValue() to Object.is in ES6.
//

// $isPointer(v [, typeid])
static void IsPointer(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    bool r = (GetObjectId(vm, args[0]) == V8EXTPTR);
    if (r && argc > 1) {
        int ct = args[1]->Int32Value(
                    isolate->GetCurrentContext()).FromJust();
        if (GetCid(v8Object::Cast(args[0])) != ct)
            r = false;
    }
    args.GetReturnValue().Set(Boolean::New(isolate, r));
}

static void StrError(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8Context context = isolate->GetCurrentContext();
    int errnum;
    if (args.Length() > 0)
        errnum = args[0]->Int32Value(context).FromJust();
    else
        errnum = errno;
    args.GetReturnValue().Set(v8STR(isolate, strerror(errnum)));
}

static void ByteLength(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, args.Length() < 1);
    int len = 0;    // maybe < 0 (for Opaque pointer or overflow otherwise).

    v8Object ptrObj = ToPtr(args[0]);
    if (! ptrObj.IsEmpty()) {
        len = GetCtypeSize(ptrObj);
    } else if (args[0]->IsString()) {
        len = v8String::Cast(args[0])->Utf8Length();
    } else if (args[0]->IsArrayBufferView()) {
        /* ArrayBufferView is implemented by all typed arrays and DataView */
        len = v8ArrayBufferView::Cast(args[0])->ByteLength();
    } else if (args[0]->IsArrayBuffer()) {
        len = v8ArrayBuffer::Cast(args[0])->ByteLength();
    } else {
        /* ThrowTypeError(isolate, "$length(): invalid argument"); */

        /* XXX: $length(x) !== null && $length(x) >= 0 */
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }
    args.GetReturnValue().Set(Integer::New(isolate, len));
}

static void CallForeignFunc(
        const v8::FunctionCallbackInfo<v8::Value>& args) {

    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));

    v8Object obj = args.Holder();
    assert(obj->InternalFieldCount() == 2);
    v8_ffn *func_wrap = reinterpret_cast<v8_ffn *>(
                v8External::Cast(obj->GetInternalField(1))->Value());
    int argc = args.Length();
    if (argc > MAXARGS || argc != func_wrap->pcount)
        ThrowError(isolate, "C-type function: incorrect number of arguments");

    assert(func_wrap->type == FN_CTYPE);

    v8_val ctvals[MAXARGS];
    for (int i = 0; i < argc; i++) {
        v8_to_ctype(vm, args[i], &ctvals[i]);
    }

    v8_val retval;
    {
        isolate->Exit();
        v8::Unlocker unlocker(isolate);
        retval = ((FnCtype) func_wrap->fp)(vm, argc, ctvals);
    }
    isolate->Enter();
    for (int i = 0; i < argc; i++) {
        v8_reset(vm, ctvals[i]);
    }
    v8Value retv = ctype_to_v8(vm, &retval);
    if (retv.IsEmpty()) // invalid persistent handle
        Panic("received invalid return value from C-function");
    args.GetReturnValue().Set(retv);
    v8_reset(vm, retval);
}

// ArrayBuffer.transfer()?
// $transfer(oldBuffer, newByteLength)
static void Transfer(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 2);

    if (!args[0]->IsArrayBuffer()
            || !v8ArrayBuffer::Cast(args[0])->IsNeuterable()
    ) {
        ThrowTypeError(isolate, "argument #1 is not (neuterable) ArrayBuffer");
    }
    v8ArrayBuffer ab = v8ArrayBuffer::Cast(args[0]);
    uint32_t byteLength = args[1]->Uint32Value(
                        isolate->GetCurrentContext()).FromJust();
    ArrayBuffer::Contents c = ab->Externalize();
  /**
   * Neuters this ArrayBuffer and all its views (typed arrays).
   * Neutering sets the byte length of the buffer and all typed arrays to zero,
   * preventing JavaScript from ever accessing underlying backing store.
   * ArrayBuffer should have been externalized and must be neuterable.
   */
    ab->Neuter();
    args.GetReturnValue().Set(ArrayBuffer::New(isolate,
                    erealloc(c.Data(), byteLength),
                    byteLength,
                    v8::ArrayBufferCreationMode::kInternalized));
}

// TODO: use pthread_once
static int v8initialized = 0;

static void v8init(void) {
    if (v8initialized)
        return;
    // N.B.: V8BINDIR must have a trailing slash!
    V8::InitializeExternalStartupData(V8BINDIR);

#if defined V8TEST
    // Enable garbage collection
    const char* flags = "--expose_gc --harmony-async-await";
    V8::SetFlagsFromString(flags, strlen(flags));
#else
    const char* flags = "--harmony-async-await";
    V8::SetFlagsFromString(flags, strlen(flags));
#endif

    Platform* platform = platform::CreateDefaultPlatform();
    V8::InitializePlatform(platform);
    V8::Initialize();
    create_params.array_buffer_allocator = &allocator;
    global_ = ctype_handle(GLOBAL_HANDLE);
    null_ = ctype_handle(NULL_HANDLE);
    nullptr_.type = V8_CTYPE_PTR;
    nullptr_.hndle = NULLPTR_HANDLE;
    nullptr_.ptr = NULL;

    v8initialized = 1;
}

// Invoked by the main thread.
v8_state js8_vmnew(mill_worker w) {
    v8_state vm = new js_vm;
    vm->w = w;
    vm->inq = mill_pipemake(sizeof (void *));
    assert(vm->inq);
    vm->outq = mill_pipemake(sizeof (void *));
    assert(vm->outq);
    vm->ch = mill_chmake(sizeof (GoCallback *), 64); // XXX: How to select a bufsize??
    assert(vm->ch);
    vm->ncoro = 0;
    go(start_go(vm->inq));
    return vm;
}

static void GlobalGet(v8Name name,
        const PropertyCallbackInfo<Value>& info) {
    Isolate *isolate = info.GetIsolate();
    HandleScope handle_scope(isolate);
    String::Utf8Value str(name);
    if (strcmp(*str, "$errno") == 0)
        info.GetReturnValue().Set(Integer::New(isolate, errno));
}

static void GlobalSet(v8Name name, v8Value val,
        const PropertyCallbackInfo<void>& info) {
    Isolate *isolate = info.GetIsolate();
    HandleScope handle_scope(isolate);
    String::Utf8Value str(name);
    if (strcmp(*str, "$errno") == 0)
        errno = val->ToInt32(
                isolate->GetCurrentContext()).ToLocalChecked()->Value();
}

// Invoked by the main thread.
void js8_vmclose(v8_state vm) {
    //
    // vm->inq already closed (See js_vmclose()).
    // Close vm->outq; This causes the receiving coroutine (recv_coro)
    // to exit the loop and free vm->outq.
    //
    Isolate *isolate = vm->isolate;

    if (true) {
        Locker locker(isolate);

        mill_pipeclose(vm->outq);
        mill_pipefree(vm->inq);
    }

    isolate->Dispose();
    delete vm;
}

static v8_val ctype_set_error(Isolate *isolate, TryCatch *try_catch) {
    v8_val errval;
    errval.type = V8_CTYPE_ERR;
    errval.stp = GetExceptionString(isolate, try_catch);
    return errval;
}

static v8Value CompileRun(v8_state vm, const char *src, v8_val *errval) {
    Isolate *isolate = vm->isolate;
    /* Locker locker(isolate);
    Isolate::Scope isolate_scope(isolate); */
    EscapableHandleScope handle_scope(isolate);

    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);

    TryCatch try_catch(isolate);
    const char *script_name = "<string>";   // TODO: optional argument
    v8String name(v8STR(isolate, script_name));
    v8String source(v8STR(isolate, src));

    ScriptOrigin origin(name);
    v8Script script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        *errval = ctype_set_error(isolate, &try_catch);
        return v8Value();
    }

    Handle<Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        *errval = ctype_set_error(isolate, &try_catch);
        return v8Value();
    }
    assert(!result.IsEmpty());
    return handle_scope.Escape(result);
}

static v8_val CallFunc(struct js8_cmd_s *cmd) {
    v8_state vm = cmd->vm;
    Isolate *isolate = vm->isolate;
    Locker locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);

    v8Value v1;
    v8_val ret;
    if (cmd->type == V8CALLSTR) {
        // function expression
        v1 = CompileRun(vm, cmd->h1.stp, &ret);
        if (v1.IsEmpty())
            return ret;
    } else {
        v1 = ctype_to_v8(vm, &cmd->h1);
    }
    if (v1.IsEmpty() || !v1->IsFunction())
        Panic("v8_call: function argument #1 expected");
    v8Function func = v8Function::Cast(v1);

    int argc = cmd->nargs;
    assert(argc <= 4);

    TryCatch try_catch(isolate);
    v8Value v = ctype_to_v8(vm, &cmd->h2);
    if (v.IsEmpty())
        Panic("v8_call: invalid handle for 'this'");
    v8Object self = v->ToObject(context).ToLocalChecked();

    v8Value argv[4];
    int i;
    for (i = 0; i < argc; i++) {
        argv[i] = ctype_to_v8(vm, &cmd->a[i]);
        if (argv[i].IsEmpty())
            Panic("v8_call: invalid handle for argument");
    }

    v8Value result = func->Call(
            context, self, argc, argv).FromMaybe(v8Value());
    if (try_catch.HasCaught()) {
        return ctype_set_error(isolate, &try_catch);
    }
    v8_to_ctype(vm, result, &ret);
    return ret;
}

int js8_do(struct js8_cmd_s *cmd) {
    switch (cmd->type) {
    case V8COMPILERUN: {
        Isolate *isolate = cmd->vm->isolate;
        LOCK_SCOPE(isolate)
        v8Value retv = CompileRun(cmd->vm, cmd->h1.stp, &cmd->h2);
        if (retv.IsEmpty()) {
            break;
        }
        v8_to_ctype(cmd->vm, retv, &cmd->h2);
    } // release locker
        WaitFor(cmd->vm);
        break;
    case V8CALL:
    case V8CALLSTR:
        cmd->h2 = CallFunc(cmd);
        if (!V8_ISERROR(cmd->h2))
            WaitFor(cmd->vm);
        break;
    case V8GC:
#ifdef V8TEST
    {
        Isolate *isolate = cmd->vm->isolate;
        Locker locker(isolate);
        Isolate::Scope isolate_scope(isolate);
        cmd->vm->weak_counter = 0;
        isolate->RequestGarbageCollectionForTesting(
                            Isolate::kFullGarbageCollection);
        V8_INT32(cmd->h2, cmd->vm->weak_counter);
    }
#endif
        break;
    default:
        fprintf(stderr, "error: js8_do(): received unexpected code");
        abort();
    }
    return 1;
}


////////////////////////////// C-type ///////////////////////////

static void v8_panic(const char *msg, const char *funcname) {
    fprintf(stderr, "Panic:%s:%s\n", funcname, msg);
    abort();
    exit(1);
}

static v8_val ctype_handle(v8_handle h) {
    v8_val ctval;
    ctval.type = V8_CTYPE_HANDLE;
    ctval.hndle = h;
    return ctval;
}

static v8_val v8_ctypestr(const char *stp, unsigned stlen) {
    v8_val ctval;
    ctval.type = V8_CTYPE_STR;
    if (!stp)
        stlen = 0; /* empty string ! */
    ctval.stp = (char *) emalloc(stlen+1);
    memcpy(ctval.stp, stp, stlen);
    ctval.stp[stlen] = '\0';
    return ctval;
}

static void v8_to_ctype(v8_state vm, v8Value v, v8_val *ctval) {
    long64 l;

    if (v->IsNumber()) {
        ctval->type = V8_CTYPE_DOUBLE;
        ctval->d = Local<Number>::Cast(v)->Value();
    } else if (v->IsString()) {
        ctval->type = V8_CTYPE_STR;
        v8String s = v8String::Cast(v);
        int utf8len = s->Utf8Length();	/* >= 0 */
        char *p = (char *) emalloc(utf8len + 1);
        int len = s->WriteUtf8(p, utf8len);
        p[len] = '\0';
        ctval->stp = p;
    } else if (GetObjectId(vm, v) == V8EXTPTR) {
        // XXX: optimize: check nullptr?
        ctval->type = V8_CTYPE_PTR;
        ctval->hndle = NewPersister(vm, v);
        ctval->ptr = v8External::Cast(
                v8Object::Cast(v)->GetInternalField(1))->Value();
    } else if (v->IsArrayBufferView()) {
        // TypedArray and DataView
        ctval->type = V8_CTYPE_PTR;
        ctval->hndle = NewPersister(vm, v);
        v8ArrayBufferView av = v8ArrayBufferView::Cast(v);
        ctval->ptr = (char *)av->Buffer()->GetContents().Data() + av->ByteOffset();
    } else if (v->IsArrayBuffer()) {
        ctval->type = V8_CTYPE_PTR;
        ctval->hndle = NewPersister(vm, v);
        ctval->ptr = v8ArrayBuffer::Cast(v)->GetContents().Data();
    } else if (v->IsBoolean()) {
        ctval->type = V8_CTYPE_DOUBLE;
        ctval->d = (double) Local<Boolean>::Cast(v)->Value();
    } else if (LongValue(v, &l)) {
        if (l.issigned) {
            ctval->type = V8_CTYPE_LONG;
            ctval->i64 = l.val.i64;
        } else {
            ctval->type = V8_CTYPE_ULONG;
            ctval->u64 = l.val.u64;
        }
    } else if (v->IsNull()) {
        *ctval = null_;
    } else if (!v->IsUndefined()) {
        ctval->type = V8_CTYPE_HANDLE;
        ctval->hndle = NewPersister(vm, v);
    } else {
        /* undefined */
        ctval->type = V8_CTYPE_VOID;
    }
}

static v8Value ctype_to_v8(v8_state vm, v8_val *ctval) {
    switch (ctval->type) {
    case V8_CTYPE_INT32:
        return Integer::New(vm->isolate, ctval->i32);
    case V8_CTYPE_UINT32:
        return Integer::NewFromUnsigned(vm->isolate, ctval->ui32);
    case V8_CTYPE_DOUBLE:
        return Number::New(vm->isolate, ctval->d);
    case V8_CTYPE_STR:
        return String::NewFromUtf8(vm->isolate, ctval->stp);
    case V8_CTYPE_PTR:
        if (!ctval->ptr)
            return GetValueFromPersister(vm, NULLPTR_HANDLE);
        return WrapPtr(vm, ctval->ptr);
    case V8_CTYPE_BUFFER: {
        void *ptr = ctval->ptr;
        unsigned byte_length = (unsigned) ctval->hndle;
        if (ptr) {
            return ArrayBuffer::New(vm->isolate, ptr, byte_length,
                        v8::ArrayBufferCreationMode::kInternalized);
        }
        return ArrayBuffer::New(vm->isolate, byte_length);
    }
    case V8_CTYPE_LONG:
        return Int64(vm, ctval->i64);
    case V8_CTYPE_ULONG:
        return UInt64(vm, ctval->u64);
    case V8_CTYPE_HANDLE:
        return GetValueFromPersister(vm, ctval->hndle); // possibly empty
    default:
        break;
    }
    return v8::Undefined(vm->isolate);
}


///////////////////////////////////////////////////////////////
static void PersistentSet(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    unsigned k = NewPersister(vm, args[0]);
    args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, k));
}

static void PersistentGet(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    unsigned k = args[0]->Uint32Value(isolate->GetCurrentContext()).FromJust();
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    v8Value v = GetValueFromPersister(vm, k);
    if (v.IsEmpty())
        ThrowError(isolate, "get: invalid handle argument");
    args.GetReturnValue().Set(v);
}

static void PersistentDelete(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    unsigned k = args[0]->Uint32Value(isolate->GetCurrentContext()).FromJust();
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    DeletePersister(vm, k);
}

// $store.set(), $store.get() and $store.delete()
static v8Value MakeStore(Isolate *isolate) {
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ = ObjectTemplate::New(isolate);
    templ->Set(v8STR(isolate, "set"),
                FunctionTemplate::New(isolate, PersistentSet));
    templ->Set(v8STR(isolate, "get"),
                FunctionTemplate::New(isolate, PersistentGet));
    templ->Set(v8STR(isolate, "delete"),
                FunctionTemplate::New(isolate, PersistentDelete));
    // Create the one and only instance.
    v8Object p = templ->NewInstance(
                isolate->GetCurrentContext()).ToLocalChecked();
    return handle_scope.Escape(p);
}


static void CollectGarbage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    int rc = 0;
#ifdef V8TEST
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    vm->weak_counter = 0;
    isolate->RequestGarbageCollectionForTesting(
                            Isolate::kFullGarbageCollection);
    rc = vm->weak_counter;
#endif
    args.GetReturnValue().Set(Integer::New(isolate, rc));
}

#ifdef V8TEST
void GarbageCollected(Isolate *isolate, GCType type, GCCallbackFlags flags) {
/*
  kGCTypeScavenge = 1 << 0,
  kGCTypeMarkSweepCompact = 1 << 1,
  kGCTypeIncrementalMarking = 1 << 2,
  kGCTypeProcessWeakCallbacks = 1 << 3
*/
    fprintf(stderr, "*************** Garbage Collected (%d)**************\n",
        type);
}
#endif


static void Load(const FunctionCallbackInfo<Value>& args);

// The second part of the vm initialization.
static void CreateIsolate(v8_state vm) {
    v8init();
    Isolate* isolate = Isolate::New(create_params);
    LOCK_SCOPE(isolate)

#ifdef V8TEST
    isolate->AddGCEpilogueCallback(GarbageCollected);
#endif

    assert(isolate->GetCurrentContext().IsEmpty());
    vm->isolate = isolate;

    // isolate->SetCaptureStackTraceForUncaughtExceptions(true);
    isolate->SetData(0, vm);

    v8ObjectTemplate global = ObjectTemplate::New(isolate);

    global->Set(v8STR(isolate, "$print"),
                FunctionTemplate::New(isolate, Print));
    global->Set(v8STR(isolate, "$go"),
                FunctionTemplate::New(isolate, Go));
    global->Set(v8STR(isolate, "$now"),
                FunctionTemplate::New(isolate, Now));
    global->Set(v8STR(isolate, "$close"),
                FunctionTemplate::New(isolate, Close));
    global->Set(v8STR(isolate, "$eval"),
                FunctionTemplate::New(isolate, EvalString));
    global->Set(v8STR(isolate, "$malloc"),
                FunctionTemplate::New(isolate, Malloc));
    global->Set(v8STR(isolate, "$isPointer"),
                FunctionTemplate::New(isolate, IsPointer));
    global->Set(v8STR(isolate, "$strerror"),
                FunctionTemplate::New(isolate, StrError));
    global->Set(v8STR(isolate, "$length"),
                FunctionTemplate::New(isolate, ByteLength));
    global->Set(v8STR(isolate, "$load"),
                FunctionTemplate::New(isolate, Load));
    global->Set(v8STR(isolate, "$collectGarbage"),
                FunctionTemplate::New(isolate, CollectGarbage));
    global->Set(v8STR(isolate, "$lcntl"),
                FunctionTemplate::New(isolate, LongCntl));
    global->Set(v8STR(isolate, "$long"), LongTemplate(vm));
    global->Set(v8STR(isolate, "$transfer"),
                FunctionTemplate::New(isolate, Transfer));

    v8Context context = Context::New(isolate, NULL, global);
    if (context.IsEmpty()) {
        Panic("failed to create a V8 context");
    }

    vm->context.Reset(isolate, context);

    Context::Scope context_scope(context);

    // Make the template for external(foreign) pointer objects.
    v8ObjectTemplate extptr_templ = ObjectTemplate::New(isolate);
    extptr_templ->SetInternalFieldCount(PTR_INTERNAL_FIELD_COUNT);
    vm->extptr_template.Reset(isolate, extptr_templ);

    // Make the template for foreign function objects.
    v8ObjectTemplate ffunc_templ = ObjectTemplate::New(isolate);
    ffunc_templ->SetInternalFieldCount(2);
    ffunc_templ->SetCallAsFunctionHandler(CallForeignFunc);
    vm->extfunc_template.Reset(isolate, ffunc_templ);

    // Make the template for Int64 objects.
    v8ObjectTemplate i64_templ = ObjectTemplate::New(isolate);
    i64_templ->SetInternalFieldCount(2);
    vm->i64_template.Reset(isolate, i64_templ);
    // Make the template for UInt64 objects.
    v8ObjectTemplate ui64_templ = ObjectTemplate::New(isolate);
    ui64_templ->SetInternalFieldCount(2);
    vm->ui64_template.Reset(isolate, ui64_templ);

    // Create __proto__ for pointer objects.
    MakeCtypeProto(vm);

    // Create the C function prototype object.
    v8ObjectTemplate templ1 = ObjectTemplate::New(isolate);
    vm->cfunc_proto.Reset(isolate,
                templ1->NewInstance(context).ToLocalChecked());

    // Create the long prototype object.
    v8ObjectTemplate templ2 = ObjectTemplate::New(isolate);
    /* templ2->Set(v8STR(isolate, "..."),
                FunctionTemplate::New(isolate, ...)); */
    vm->long_proto.Reset(isolate,
                templ2->NewInstance(context).ToLocalChecked());

    v8Object nullptr_obj = extptr_templ
                -> NewInstance(context).ToLocalChecked();
    SetObjectId(nullptr_obj, V8EXTPTR);
    SetCtypeSize(nullptr_obj, 0);

    nullptr_obj->SetInternalField(1, External::New(isolate, nullptr));
    nullptr_obj->SetPrototype(v8Value::New(isolate, vm->cptr_proto));

    // Create Go object template.
    MakeGoTemplate(vm);

    // Name the global object "Global".
    v8Object realGlobal = v8Object::Cast(
                        context->Global()->GetPrototype());
    realGlobal->Set(v8STR(isolate, "Global"), realGlobal);
    realGlobal->Set(v8STR(isolate, "$nullptr"), nullptr_obj);
    realGlobal->Set(v8STR(isolate, "$store"), MakeStore(isolate));

    realGlobal->SetAccessor(context, v8STR(isolate, "$errno"),
                    GlobalGet, GlobalSet).FromJust();

    // Initial size should be some multiple of 32.
    vm->store_ = new PersistentStore(isolate, 1024);

    /* NUM_PERMANENT_HANDLES == 3*/
    v8_handle h1;
    h1 = NewPersister(vm, realGlobal);
    assert(h1 == GLOBAL_HANDLE);
    h1 = NewPersister(vm, v8::Null(isolate));
    assert(h1 == NULL_HANDLE);
    h1 = NewPersister(vm, nullptr_obj);
    assert(h1 == NULLPTR_HANDLE);
}

// Runs in the worker(V8) thread.
int js8_vminit(v8_state vm) {
    CreateIsolate(vm);
    go(recv_go(vm));
    return 0;
}

////////////////////////////////////////////////////////////////////////

static v8_val v8_object(v8_state vm) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    /* XXX: need a context in this case unlike String or Number!!! */
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    return ctype_handle(NewPersister(vm, Object::New(isolate)));
}

static v8_val v8_get(v8_state vm, v8_val hobj, const char *key) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);

    v8Value v1 = ctype_to_v8(vm, &hobj);
    if (v1.IsEmpty())
        Panic("get: invalid handle");
    if (!v1->IsObject()) {
        //v1 = v1->ToObject(context).ToLocalChecked();
        Panic("get: not an object");
    }
    v8Object obj = v8Object::Cast(v1);
    v8Value v2 = obj->Get(context,
                v8STR(isolate, key)).ToLocalChecked();
    v8_val ctval;
    v8_to_ctype(vm, v2, &ctval);
    return ctval;
}

static int v8_set(v8_state vm, v8_val hobj,
        const char *key, v8_val ctval) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);  /* needed ? */

    v8Value v2 = ctype_to_v8(vm, &ctval);
    v8Value v1 = ctype_to_v8(vm, &hobj);
    if (v1.IsEmpty() || v2.IsEmpty())
        Panic("set: invalid handle");
    if (!v1->IsObject())
        Panic("set: not an object");
    if (!v2->IsUndefined()) {
        v8Object obj = v8Object::Cast(v1);
        return obj->Set(context,
                    v8STR(isolate, key), v2).FromJust();
    }
    return 1; // undefined -> do nothing
}

static v8_val v8_array(v8_state vm, int length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    return ctype_handle(NewPersister(vm, Array::New(isolate, length)));
}

static v8_val v8_geti(v8_state vm, v8_val hobj, unsigned index) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = ctype_to_v8(vm, &hobj);
    if (v1.IsEmpty())
        Panic("geti: invalid handle");
    if (!v1->IsObject())
        Panic("geti: not an object");
    v8Object obj = v8Object::Cast(v1);

    // Undefined for nonexistent index.
    v8Value v2 = obj->Get(v8Context::New(isolate, vm->context),
                        index).ToLocalChecked();
    v8_val ctval;
    v8_to_ctype(vm, v2, &ctval);
    return ctval;
}

static int v8_seti(v8_state vm, v8_val hobj,
        unsigned index, v8_val ctval) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context); /* needed ? */

    v8Value v1 = ctype_to_v8(vm, &hobj);
    v8Value v2 = ctype_to_v8(vm, &ctval);
    if (v1.IsEmpty() || v2.IsEmpty())
        Panic("seti: invalid handle");
    if (!v1->IsObject())
        Panic("seti: not an object");
    v8Object obj = v8Object::Cast(v1);
    if (! v2->IsUndefined())
        return obj->Set(context, index, v2).FromJust();
    return 1;   // undefined -> do nothing
}

// Import a C function.
static v8_val v8_cfunc(v8_state vm, const v8_ffn *func_item) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate);
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    v8Object fnObj;
    if (func_item->type == FN_CTYPE)
        fnObj = WrapFunc(vm, (v8_ffn *) func_item);
    else
        fnObj = WrapGo(vm, (v8_ffn *) func_item);
    return ctype_handle(NewPersister(vm, fnObj));
}


/* N.B.: V8 owns the Buffer memory. If ptr is not NULL, it must be
 * compatible with ArrayBuffer::Allocator::Free. */
static v8_val v8_arraybuffer(v8_state vm, void *ptr, size_t byte_length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    if (ptr) {
        return ctype_handle(NewPersister(vm,
                ArrayBuffer::New(isolate, ptr, byte_length,
                    v8::ArrayBufferCreationMode::kInternalized)));
    }
    return ctype_handle(NewPersister(vm,
            ArrayBuffer::New(isolate, byte_length)));
}

static size_t v8_bytelength(v8_state vm, v8_val val) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = ctype_to_v8(vm, &val);
    if (v1.IsEmpty())
        return 0;
    size_t len = 0;
    if (v1->IsArrayBufferView()) {
        /* ArrayBufferView is implemented by all typed arrays and DataView */
        len = v8ArrayBufferView::Cast(v1)->ByteLength();
    } else if (v1->IsArrayBuffer()) {
        len = v8ArrayBuffer::Cast(v1)->ByteLength();
    } /* else
        len = 0; */
    return len;
}

static size_t v8_byteoffset(v8_state vm, v8_val val) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = ctype_to_v8(vm, &val);
    size_t off = 0;
    if (!v1.IsEmpty() && v1->IsArrayBufferView()) {
        off = v8ArrayBufferView::Cast(v1)->ByteOffset();
    } /* else
        off = 0; */
    return off;
}

v8_val v8_getbuffer(v8_state vm, v8_val val) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = ctype_to_v8(vm, &val);
    if (v1.IsEmpty() || !v1->IsTypedArray())
        Panic("getbuffer: TypedArray argument expected");
    v8Value ab = v8TypedArray::Cast(v1)->Buffer();
    return ctype_handle(NewPersister(vm, ab));
}

void *v8_buffer(v8_state vm, v8_val abval) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v = ctype_to_v8(vm, &abval);
    if (!v.IsEmpty()) {
        if (v->IsArrayBuffer()) {
            return v8ArrayBuffer::Cast(v)->GetContents().Data();
        }
        if (v->IsArrayBufferView()) {
            v8ArrayBufferView av = v8ArrayBufferView::Cast(v);
            return (char *)av->Buffer()->GetContents().Data() +
                        av->ByteOffset();
        }
    }
    Panic("buffer: invalid handle");
    return nullptr;
}

#if 0
void *v8_externalize(v8_state vm, v8_val val) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate);
    v8Value v1 = ctype_to_v8(vm, &val);
    if (!v1.IsEmpty() && v1->IsArrayBuffer()) {
        return v8ArrayBuffer::Cast(v1)->Externalize().Data();
    }
    Panic("externalize: ArrayBuffer argument expected");
    return nullptr;
}
#endif

static void v8_reset(v8_state vm, v8_val val) {
    switch (val.type) {
    case V8_CTYPE_STR:
        val.type = 0;
        free(val.stp);
        val.stp = NULL;
        break;
    case V8_CTYPE_PTR:
    case V8_CTYPE_HANDLE:
        if (val.hndle > NUM_PERMANENT_HANDLES) {
            Isolate *isolate = vm->isolate;
            Locker locker(isolate);
            Isolate::Scope isolate_scope(isolate);
            vm->store_->Dispose(val.hndle);
            val.type = 0;
            val.hndle = 0;
        }
        break;
    }
}

struct WeakPtr {
    v8_state vm;
    union {
        Fnfree free_func;
        FnCtype free_ctfunc;
    };
    Persistent<Value> handle;
    void *ptr;
};

static void WeakPtrCallback1(
        const v8::WeakCallbackInfo<WeakPtr> &data) {
    WeakPtr *w = data.GetParameter();
    Isolate *isolate = w->vm->isolate;
    w->free_func(w->ptr);
    w->handle.Reset();
    isolate->AdjustAmountOfExternalAllocatedMemory(
                - static_cast<int64_t>(sizeof(WeakPtr)));
#ifdef V8TEST
    w->vm->weak_counter++;
#endif
    delete w;
}

// imported C function
static void WeakPtrCallback2(
        const v8::WeakCallbackInfo<WeakPtr> &data) {
    WeakPtr *w = data.GetParameter();
    Isolate *isolate = w->vm->isolate;
    /* HandleScope handle_scope(isolate); */
    v8_val ctvals[1];
    ctvals[0].type = V8_CTYPE_PTR;
    ctvals[0].ptr = w->ptr;
    w->free_ctfunc(w->vm, 1, ctvals);   // FIXME -- check return type, should be V8_CTYPE_VOID
    w->handle.Reset();
    isolate->AdjustAmountOfExternalAllocatedMemory(
                - static_cast<int64_t>(sizeof(WeakPtr)));

#ifdef V8TEST
    w->vm->weak_counter++;
#endif
    delete w;
}

static int SetFinalizer(v8_state vm, v8Object ptrObj, Fnfree free_func) {
    Isolate *isolate = vm->isolate;
    if (!ptrObj.IsEmpty() && !IsCtypeWeak(ptrObj) && free_func) {
        void *ptr = v8External::Cast(ptrObj->GetInternalField(1))->Value();
        if (!ptr)
            return false;
        SetCtypeWeak(ptrObj);
        WeakPtr *w = new WeakPtr;
        w->vm = vm;
        w->ptr = ptr;
        w->free_func = free_func;
        w->handle.Reset(isolate, ptrObj);
        w->handle.SetWeak(w, WeakPtrCallback1, WeakCallbackType::kParameter);
        w->handle.MarkIndependent();
        isolate->AdjustAmountOfExternalAllocatedMemory(
                    static_cast<int64_t>(sizeof(WeakPtr)));
        return true;
    }
    return false;
}

#if 0
// XXX: use gc() in JS.
static int v8_dispose(v8_state vm, v8_handle h, Fnfree free_func) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Object ptrObj = ToPtr(GetValueFromPersister(vm, h));
    if (SetFinalizer(vm, ptrObj, free_func)) {
        vm->store_->Dispose(h);
        return true;
    }
    return false;
}
#endif

/*
 *  ptr.gc() => use free() as the finalizer.
 *  ptr.gc(finalizer_function).
 *  Returns the pointer: p1 = $malloc(..).gc().
 */
void Gc(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8Object obj = args.Holder();
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    if (GetObjectId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "gc: not a pointer");

    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (!ptr || IsCtypeWeak(obj)) {
        args.GetReturnValue().Set(obj);
        return;
    }
    if (argc == 0) {
        SetFinalizer(vm, obj, free);
        args.GetReturnValue().Set(obj);
        return;
    }
    if (GetObjectId(vm, args[0]) == V8EXTFUNC) {
        v8_ffn *func_item = reinterpret_cast<v8_ffn *>(
                v8External::Cast(
                        v8Object::Cast(args[0])->GetInternalField(1)
                    )->Value()
            );
        if ((func_item->type == FN_CTYPE) && func_item->pcount == 1) {
            WeakPtr *w = new WeakPtr;
            w->vm = vm;
            w->ptr = ptr;
            w->free_ctfunc = (FnCtype) func_item->fp;
            w->handle.Reset(isolate, obj);
            w->handle.SetWeak(w, WeakPtrCallback2,
                            WeakCallbackType::kParameter);
            SetCtypeWeak(obj);
            w->handle.MarkIndependent();
            isolate->AdjustAmountOfExternalAllocatedMemory(
                        static_cast<int64_t>(sizeof(WeakPtr)));
            args.GetReturnValue().Set(obj);
            return;
        }
    }
    ThrowError(isolate, "gc: invalid argument");
}

static const char *v8_ctype_errs[] = {
    "C-type argument is not a number",
    "C-type argument is not a string",
    "C-type argument is not a pointer",
    "C-type argument is not a long (object)",
    "C-type argument is not an unsigned long (object)",
    "invalid handle",
};

static struct v8_fn_s v8fns = {
    v8_object,
    v8_get,
    v8_set,
    v8_geti,
    v8_seti,
    v8_array,
    v8_arraybuffer,
    v8_bytelength,  /* ArrayBuffer(View) */
    v8_byteoffset, /* ArrayBufferView */
    v8_buffer,
    v8_reset,
    v8_cfunc,
    v8_goresolve,
    v8_goreject,
    v8_callstr,
    &global_,
    &null_,
    /* private */
    v8_ctypestr,
    v8_panic,
    v8_ctype_errs,
};

struct v8_fn_s *jsv8 = &v8fns;

typedef int (*Fnload)(v8_state, v8_val, v8_fn_s *const, v8_ffn **);

// $load - load a dynamic library.
// The filename must contain a slash. Any path search should be
// done in the JS.

static void Load(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));

    int argc = args.Length();
    ThrowNotEnoughArgs(isolate, argc < 1);
    v8Context context = isolate->GetCurrentContext();
    String::Utf8Value path(args[0]);
    if (!*path) {
        ThrowError(isolate, "$load: path is empty string");
    }
    if (!strchr(*path, '/')) {
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }
    dlerror();  /* Clear any existing error */
    void *dl = dlopen(*path, RTLD_LAZY);
    if (!dl) {
        ThrowError(isolate, "$load: cannot dlopen library"); /* FIXME: use  dlerror() */
    }

    Fnload load_func = (Fnload) dlsym(dl, V8_LOAD_FUNC);
    if (!load_func) {
        dlclose(dl);
        ThrowError(isolate,
            "$load: cannot find library initialization function");
    }

    v8_ffn *functab;
    v8Object o1 = Object::New(isolate);
    v8_val val1 = ctype_handle(NewPersister(vm, o1));
    errno = 0;
    int nfunc = load_func(vm, val1, &v8fns, &functab);
    DeletePersister(vm, val1.hndle);

    if (nfunc < 0) {
        dlclose(dl);
        ThrowError(isolate, errno ? strerror(errno) : "$load: unknown error");
    }
    for (int i = 0; i < nfunc && functab[i].name; i++) {
        v8Object fnObj;
        if (functab[i].type != FN_CTYPE) {
            fnObj = WrapGo(vm, &functab[i]);
        } else {
            fnObj = WrapFunc(vm, &functab[i]);
        }
        o1->Set(context,
                String::NewFromUtf8(isolate, functab[i].name),
                fnObj).FromJust();
    }
    args.GetReturnValue().Set(o1);
}


} // extern "C"
