#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <math.h>
#include <signal.h>

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
        void *data = malloc((length+8)&~7);
        if (data != NULL)
            *((char *)data + length) = '\0';
        return data;
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
    s.append("\n");
    Local<Value> stack_trace_string;
    if (try_catch->StackTrace(isolate->GetCurrentContext())
            .ToLocal(&stack_trace_string) &&
            stack_trace_string->IsString()) {
        String::Utf8Value stack_trace(Local<String>::Cast(stack_trace_string));
        if (*stack_trace)
            s.append(*stack_trace);
        s.append("\n");
    }
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

#define ISOLATE_SCOPE(args_, isolate_) \
    Isolate *isolate_ = args_.GetIsolate(); \
    HandleScope handle_scope(isolate_);

/*
static void Panic(const char *errstr) {
    fprintf(stderr, "%s\n", errstr);
    abort();
    exit(1);
}
*/

static void panic(Isolate *isolate,
            const char *errstr, const char *prefix) {
    fprintf(stderr, "fatal error: ");
    if(prefix)
        fprintf(stderr, "%s:", prefix);
    fprintf(stderr, "%s\n", errstr);
    Local<StackTrace> stack_trace = StackTrace::CurrentStackTrace(isolate, 5);
    int n = stack_trace->GetFrameCount();
    for (int i = 0; i < n; i++) {
        Local<StackFrame> stack_frame = stack_trace->GetFrame(i);
        String::Utf8Value script_name(stack_frame->GetScriptName());
        String::Utf8Value func_name(stack_frame->GetFunctionName());
        fprintf(stderr, "\tat %s (%.*s:%d)\n",
            func_name.length() ? *func_name : "<anonymous>",
            script_name.length(), *script_name,
            stack_frame->GetLineNumber());
    }
    abort();
    exit(1);
}

////////////////////////////////// Coroutine //////////////////////////////////

static void promise_reject_callback(PromiseRejectMessage message) {
    Local<Promise> promise = message.GetPromise();
    Isolate* isolate = promise->GetIsolate();
    HandleScope handle_scope(isolate);

    v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
    // v8Context context = v8Context::New(isolate, vm->context);
    // Context::Scope context_scope(context);

    v8Context context = isolate->GetCurrentContext();

    v8Value exception = message.GetValue();

/*
    String::Utf8Value s(exception);
    const char* exception_string = *s ? *s : "unknown error";
    fprintf(stderr, "%s\n", exception_string);
    // v8Message msg = v8::Exception::CreateMessage(isolate, exception);
*/

    Local<Integer> event = Integer::New(isolate, message.GetEvent());

    v8Function callback = v8Function::Cast(
            GetValueFromPersister(vm, vm->on_promise_reject));
    if (exception.IsEmpty())
        exception = Undefined(isolate);
    v8Value args[] = { event, promise, exception };

    TryCatch try_catch(isolate);
    callback->Call(context->Global(), 3, args);
    if(try_catch.HasCaught()) { // XXX: ???
        fprintf(stderr, "bailing out ..\n");
        exit(1);
    }
}

  /** v8.h:
   * Set callback to notify about promise reject with no handler, or
   * revocation of such a previous notification once the handler is added.
   */

static void OnPromiseReject(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
    int argc = args.Length();
    if (argc > 0 && args[0]->IsFunction() && !vm->on_promise_reject) {
        isolate->SetPromiseRejectCallback(promise_reject_callback);
        vm->on_promise_reject = NewPersister(vm, args[0]);
    }
}


#define GOROUTINE_INTERNAL_FIELD_COUNT 2
enum {
    GoDone = (1 << 0),
    GoResolve = (1 << 1),
    GoReject = (1 << 2),
};

/* promise resolver */
struct GoCallback_s {
    int type;
    int flags;
    v8_coro g;
    v8_val result;
};

/*
enum {
    CbDone = (1 << 0),
    CbValue = (1 << 1),
    CbError = (1 << 2),
};
*/

/* callback function */
struct Callback_s {
    int type;
    int flags;  /* XXX: not used */
    v8_val func;
    v8_val result;
};

union AsyncCallback_s {
    int type;
    struct GoCallback_s go_callback;
    struct Callback_s callback;
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
        AsyncCallback_s *cb = *((AsyncCallback_s **) mill_piperecv(vm->outq, &done));
        if (done) {
            mill_pipefree(vm->outq);
            break;
        }
        /* Send to the channel to be processed later in WaitFor(). */
        int rc = mill_chs(vm->ch, &cb);
        if (rc != 0) {
            // exiting?
            mill_pipefree(vm->outq);
            break;
        }
    }
}

static void send_go(Go_s *g) {
    int rc = mill_pipesend(g->vm->inq, (void *) &g);
    if (rc == -1) {
        fprintf(stderr, "v8binding.cc:send_go(): writing to pipe failed\n");
        fprintf(stderr, "reason: %s\n", strerror(errno));
        abort();
    }
}

static int v8_goresolve(v8_state vm, v8_coro cr, v8_val data, int done);

// goroutine wpapper for a C-type function.
// N.B.: cannot reliably read errno set by the function.
coroutine static void do_cgo(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    FnCtype fp = (FnCtype) (((struct Go_s *) cr)->fp);
    v8_val retv = fp(vm, argc, args);
    v8_goresolve(vm, cr, retv, 1);
}

static v8_ffn cgodef = { 0, (void *) do_cgo, "__cgo__", FN_CORO };

static Go_s *coro_s(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));

    if (vm->exiting)
        panic(isolate, "goroutine is not allowed", NULL);

    int argc = args.Length();
    if (argc < 1)
        panic(isolate, "invalid argument", NULL);

    v8_ffn *fndef;
    v8Object coro = ToGo(args[0]);
    if (coro.IsEmpty()) {
        if (GetObjectId(args[0]) != V8EXTFUNC)
            panic(isolate, "invalid argument", NULL);
        // use the do_cgo wrapper.
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
            panic(isolate, "callback is not a function", fndef->name);
        argc--;
    }

    argc--;
    if (argc != fndef->pcount)
        panic(isolate,
            "called with incorrect number of arguments", fndef->name);

    Go_s *g = new Go_s;
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
    return g;
}


// $go(coro [, in1 [, in2 .. in12 ]])
// $go(coro [, in1 [, in2 .. in12 ]], fn)
//   fn-> function(result, done (= true|false))
static void Go(const FunctionCallbackInfo<Value>& args) {
    Go_s *g = coro_s(args);
    assert(g);
    send_go(g);
}

// $co() goroutine in the V8 thread.
coroutine static void co_start(Go_s *g) {
    mill_yield();
    go(((Fngo)g->fndef->fp)(g->vm, g, g->argc, g->args));
}

static void co_sched(Go_s *g) {
    go(co_start(g));
}

// $co(coro [, in1 [, in2 .. in12 ]])
// $co(coro [, in1 [, in2 .. in12 ]], fn)
//   fn-> function(result, done (= true|false))
static void Co(const FunctionCallbackInfo<Value>& args) {
    Go_s *g = coro_s(args);
    assert(g);
    co_sched(g);
}

static bool FinishGo(v8_state vm, GoCallback_s *cb) {
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

static bool RunGoCallback(v8_state vm, GoCallback_s *cb) {
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
        panic(isolate, "received invalid value from goroutine", fndef->name);

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


    // log intermediate exceptions with PromiseRejectCallback.
    // FIXME -- should just bail out!
    if (try_catch.HasCaught() && try_catch.CanContinue()
                && vm->on_promise_reject) {
        Local<v8::Promise::Resolver> pr =
            Local<v8::Promise::Resolver>::Cast(GetValueFromPersister(vm, g->hpr));
        v8Value exception = try_catch.Exception();
        // See PromiseRejectMessage in v8.h. Using PromiseRejectEvent
        // kPromiseRejectWithNoHandler. XXX: create our own event ?
        Local<Integer> event = Integer::New(isolate, kPromiseRejectWithNoHandler);
        v8Function callback = v8Function::Cast(
            GetValueFromPersister(vm, vm->on_promise_reject));
        if (exception.IsEmpty())
            exception = Undefined(isolate);
        v8Value args[] = { event, pr->GetPromise(), exception };
        callback->Call(context->Global(), 3, args);
    }

    v8_reset(vm, cb->result);
    delete cb;
    return false;
}

static bool RunCallback(v8_state vm, Callback_s *cb) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);

    v8Value v1 = ctype_to_v8(vm, &cb->func);
    if (!v1->IsFunction())
        panic(isolate, "callback is not a function", NULL);
    v8Value args[2];
    args[1] = ctype_to_v8(vm, &cb->result);
    args[0] = v8::Null(isolate);
    TryCatch try_catch(isolate);
    v8Function::Cast(v1)->Call(context->Global(), 2, args);
    if (try_catch.HasCaught()) {
        fprintf(stderr, "uncaught error: %s\n",
                    GetExceptionString(isolate, &try_catch));
        exit(1);
    }
    delete cb;
    return true;
}

static AsyncCallback_s *chselect(chan ch) {
    char clauses[1 * 128];  /* XXX: c++ compilers find MILL_CLAUSELEN define unpalatable */
    mill_choose_init();
    mill_choose_in(&clauses[0], ch, 0);
    mill_choose_otherwise();
    int si = mill_choose_wait();
    if(si == -1)   /* otherwise */
        return NULL;
    assert(si == 0);
    return reinterpret_cast<AsyncCallback_s *>(
            *((void **)mill_choose_val(sizeof(void *))));
}

static void WaitFor(v8_state vm) {
    AsyncCallback_s *cb;
    while (!vm->exiting && vm->ncoro > 0) {
        /* fprintf(stderr, [[ WaitFor: %d unfinished coroutines. ]]\n", vm->ncoro);*/
        cb = chr(vm->ch, AsyncCallback_s *);
        if (!cb)
            break;
        if (cb->type == 0)
            RunGoCallback(vm, reinterpret_cast<GoCallback_s *>(cb));
        else
            RunCallback(vm, reinterpret_cast<Callback_s *>(cb));
    }
    while ((cb = chselect(vm->ch))) {
        assert(cb->type != 0);
        RunCallback(vm, reinterpret_cast<Callback_s *>(cb));
    }
}

static void MakeGoTemplate(v8_state vm) {
    Isolate *isolate = vm->isolate;
    HandleScope handle_scope(isolate);
    v8ObjectTemplate templ = ObjectTemplate::New(isolate);
    templ->SetInternalFieldCount(GOROUTINE_INTERNAL_FIELD_COUNT);
    vm->go_template.Reset(isolate, templ);
}

static int v8_goresolve(v8_state vm, v8_coro cr, v8_val data, int done) {
    GoCallback_s *cb = new GoCallback_s;
    cb->type = 0;
    cb->g = cr;
    cb->result = data;
    cb->flags = (GoResolve|!!done);
    int rc = mill_pipesend(vm->outq, &cb);
    assert(rc == 0);
    return (rc == 0);
}

static int v8_goreject(v8_state vm, v8_coro cr, const char *message) {
    GoCallback_s *cb = new GoCallback_s;
    cb->type = 0;
    cb->g = cr;
    cb->result = v8_ctypestr(message, strlen(message));
    cb->flags = GoReject;
    int rc = mill_pipesend(vm->outq, &cb);
    assert(rc == 0);
    return (rc == 0);
}

static int v8_task(v8_state vm, v8_val func, v8_val data) {
    Callback_s *cb = new Callback_s;
    cb->type = 1;
    cb->func = func;
    cb->result = data;
    /*cb->flags = 0;*/
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
    ISOLATE_SCOPE(args, isolate)
    bool first = true;
    int errcount = 0;

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
    ISOLATE_SCOPE(args, isolate)
    int64_t n = now();
    args.GetReturnValue().Set(Number::New(isolate, n));
}

// $eval(string, origin) -- Must compile and run.
static void EvalString(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    int argc = args.Length();
    if (argc < 2 || !args[0]->IsString())
        panic(isolate, "$eval: invalid argument(s)", NULL);
    v8Context context = isolate->GetCurrentContext();
    TryCatch try_catch(isolate);
    v8String source = v8String::Cast(args[0]);
    v8String name;
    if (!args[1]->ToString(context).ToLocal(&name)) {
        fprintf(stderr, "fatal error: %s\n", GetExceptionString(isolate, &try_catch));
        exit(1);
    }

    ScriptOrigin origin(name);
    v8Script script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        // XXX: don't panic
        fprintf(stderr, "fatal error: %s\n", GetExceptionString(isolate, &try_catch));
        exit(1);
    }
    v8Value result;
    if (!script->Run(context).ToLocal(&result)) {
        fprintf(stderr, "fatal error: %s\n", GetExceptionString(isolate, &try_catch));
        exit(1);
    }
    assert(!result.IsEmpty());
    args.GetReturnValue().Set(result);
}

// malloc(size [, zerofill] )
// TODO: adjust GC allocation amount.
static void Malloc(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    int argc = args.Length();
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
    args.GetReturnValue().Set(ptrObj);
}

// $isPointer(v [, isNotNull = false])
static void IsPointer(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    int argc = args.Length();
    bool r = false;
    if (argc > 0) {
        r = (GetObjectId(args[0]) == V8EXTPTR);
        if (r && argc > 1 && args[1]->BooleanValue(
                    isolate->GetCurrentContext()).FromJust()) {
            void *ptr = v8External::Cast(v8Object::Cast(args[0])
                                -> GetInternalField(1))->Value();
            if (!ptr)
                r = false;
        }
    }
    args.GetReturnValue().Set(Boolean::New(isolate, r));
}

static void StrError(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    int errnum;
    if (args.Length() > 0)
        errnum = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();
    else
        errnum = errno;
    args.GetReturnValue().Set(v8STR(isolate, strerror(errnum)));
}

static void ByteLength(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    ThrowNotEnoughArgs(isolate, args.Length() < 1);
    unsigned len = 0;    // maybe < 0 (for Opaque pointer or overflow otherwise).

    if (args[0]->IsString()) {
        len = v8String::Cast(args[0])->Utf8Length();
    } else if (args[0]->IsArrayBufferView()) {
        /* ArrayBufferView is implemented by all typed arrays and DataView */
        len = v8ArrayBufferView::Cast(args[0])->ByteLength();
    } else if (args[0]->IsArrayBuffer()) {
        len = v8ArrayBuffer::Cast(args[0])->ByteLength();
    } else {
        /* ThrowTypeError(isolate, "$length(): invalid argument"); */

        /* XXX: $length(x) !== null */
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }
    args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, len));
}

// utf8String(obj, length = -1)
static void utf8String(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    int argc = args.Length();
    v8Context context = isolate->GetCurrentContext();
    void *ptr = nullptr;
    int maxLength = -1;

    if (argc > 0 && args[0]->IsObject()) {
        v8Object obj = v8Object::Cast(args[0]);
        if (obj->IsArrayBuffer()) {
            ArrayBuffer::Contents c = v8ArrayBuffer::Cast(obj)->GetContents();
            ptr = c.Data();
            maxLength = (int) c.ByteLength();
        } else if (GetObjectId(obj) == V8EXTPTR) {
            ptr = v8External::Cast(obj->GetInternalField(1))->Value();
        }
    }
    if (!ptr)
        ThrowTypeError(isolate, "$utf8String: invalid argument");

    int byteLength = -1;
    if (maxLength < 0) {    // pointer
        if (argc == 1)
            ThrowError(isolate, "utf8String: LENGTH argument expected");
        byteLength = args[0]->Int32Value(context).FromJust();
        if (byteLength < 0)
            byteLength = -1;    // assume nul-terminated
    } else if (argc > 1) {
        byteLength = args[1]->Int32Value(context).FromJust();
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

//$toarraybuffer(string)
static void ToArrayBuffer(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    int argc = args.Length();
    if (argc < 1 || !args[0]->IsString())
        ThrowTypeError(isolate, "string argument expected");
    v8String s = v8String::Cast(args[0]);
    int len = s->Utf8Length();
    v8ArrayBuffer ab = ArrayBuffer::New(isolate, len);
    s->WriteUtf8(reinterpret_cast<char*>(ab->GetContents().Data()), len);
    args.GetReturnValue().Set(ab);
}

static void CallForeignFunc(
        const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
    v8Object obj = args.Holder();
    assert(obj->InternalFieldCount() == 2);
    v8_ffn *func_wrap = reinterpret_cast<v8_ffn *>(
                v8External::Cast(obj->GetInternalField(1))->Value());
    int argc = args.Length();
    if (argc != func_wrap->pcount)
        panic(isolate,
            "C-type function called with incorrect number of arguments",
            func_wrap->name);

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
        panic(isolate, "received invalid return value", func_wrap->name);
    args.GetReturnValue().Set(retv);
    v8_reset(vm, retval);
}

// ArrayBuffer.transfer()?
// $transfer(oldBuffer, newByteLength)
static void Transfer(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    int argc = args.Length();
    ThrowNotEnoughArgs(isolate, argc < 2);

    /* XXX: IsNeuterable() == false if SharedArrayBuffer ? */
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

    /*
     * FIXME -- should not have pending $go() call with arraybuffer as an argument!!
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
    vm->ch = mill_chmake(sizeof (void *), 64); // XXX: How to select a bufsize??
    assert(vm->ch);
    vm->ncoro = 0;
    go(start_go(vm->inq));
    return vm;
}

static void GlobalGet(v8Name name,
        const PropertyCallbackInfo<Value>& info) {
    ISOLATE_SCOPE(info, isolate)
    v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
    String::Utf8Value str(name);
    if (strcmp(*str, "$errno") == 0)
        info.GetReturnValue().Set(Integer::New(isolate, errno));
    else if (strcmp(*str, "$$ncoro") == 0)
        info.GetReturnValue().Set(Integer::New(isolate, vm->ncoro));
}

static void GlobalSet(v8Name name, v8Value val,
        const PropertyCallbackInfo<void>& info) {
    ISOLATE_SCOPE(info, isolate)
    String::Utf8Value str(name);
    if (strcmp(*str, "$errno") == 0)
        errno = val->ToInt32(
                isolate->GetCurrentContext()).ToLocalChecked()->Value();
}

// Invoked by the main thread.
void js8_vmclose(v8_state vm) {
    //
    // vm->inq already closed.
    // Close vm->outq and channel; This should cause the receiving coroutine (recv_coro)
    // to exit the loop.
    //
    Isolate *isolate = vm->isolate;

    if (true) {
        Locker locker(isolate);
        GoCallback_s *cb = nullptr;

        mill_pipeclose(vm->outq);
        mill_chdone(vm->ch, &cb);
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
    if (cmd->type == V8CALLSTR || cmd->type == V8SHUTDOWN) {
        // function expression
        v1 = CompileRun(vm, cmd->h1.stp, &ret);
        if (v1.IsEmpty())
            return ret;
    } else {
        v1 = ctype_to_v8(vm, &cmd->h1);
    }
    if (v1.IsEmpty() || !v1->IsFunction())
        panic(isolate, "C api: function argument #1 expected", NULL);
    v8Function func = v8Function::Cast(v1);

    int argc = cmd->nargs;
    assert(argc <= 4);

    v8Value v = ctype_to_v8(vm, &cmd->h2);
    if (v.IsEmpty())
        panic(isolate, "C api: invalid handle for 'this'", NULL);
    v8Object self;
    if (v->IsObject())
        self = v8Object::Cast(v);
    else
        self = context->Global();

    v8Value argv[4];
    int i;
    for (i = 0; i < argc; i++) {
        argv[i] = ctype_to_v8(vm, &cmd->a[i]);
        if (argv[i].IsEmpty())
            panic(isolate, "C api: invalid handle for argument", NULL);
    }
    TryCatch try_catch(isolate);
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
    case V8SHUTDOWN: {
        v8_state vm = cmd->vm;
        Isolate *isolate = vm->isolate;
        LOCK_SCOPE(isolate)
        vm->exiting = true;
        // Close the write-end of pipe to disallow $go() .
        mill_pipeclose(vm->inq);
        // TODO: disallow $co().
        if (cmd->h1.stp) {
            cmd->h2 = CallFunc(cmd);
            v8_reset(vm, cmd->h2);
        }
        WaitFor(vm);
    }
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
        cmd->h2.type = V8_CTYPE_INT32;
        cmd->h2.i32 = cmd->vm->weak_counter;
    }
#endif
        break;
    default:
        fprintf(stderr, "fatal error: js8_do(): received unexpected code\n");
        abort();
    }
    return 1;
}


////////////////////////////// C-type ///////////////////////////

static void v8_panic(const char *msg, const char *funcname) {
    fprintf(stderr, "fatal error:%s:%s\n", funcname, msg);
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
    } else if (GetObjectId(v) == V8EXTPTR) {
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
static void PersistSet(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (argc > 0) {
        v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
        unsigned k = NewPersister(vm, args[0]);
        args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, k));
    }
}

static void PersistGet(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (argc > 0) {
        unsigned k = args[0]->Uint32Value(isolate->GetCurrentContext()).FromJust();
        v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
        v8Value v = GetValueFromPersister(vm, k);
        if (v.IsEmpty())
            ThrowError(isolate, "invalid handle");
        args.GetReturnValue().Set(v);
    }
}

static void PersistDelete(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (argc > 0) {
        unsigned k = args[0]->Uint32Value(isolate->GetCurrentContext()).FromJust();
        v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
        DeletePersister(vm, k);
    }
}

// $$persist.set(), $$persist.get() and $$persist.delete()
static v8Value MakePersist(Isolate *isolate) {
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ = ObjectTemplate::New(isolate);
    templ->Set(v8STR(isolate, "set"),
                FunctionTemplate::New(isolate, PersistSet));
    templ->Set(v8STR(isolate, "get"),
                FunctionTemplate::New(isolate, PersistGet));
    templ->Set(v8STR(isolate, "delete"),
                FunctionTemplate::New(isolate, PersistDelete));
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
    vm->exiting = 0;

    // isolate->SetCaptureStackTraceForUncaughtExceptions(true);
    isolate->SetData(0, vm);

    v8ObjectTemplate global = ObjectTemplate::New(isolate);

    global->Set(v8STR(isolate, "$print"),
                FunctionTemplate::New(isolate, Print));
    global->Set(v8STR(isolate, "$go"),
                FunctionTemplate::New(isolate, Go));
    global->Set(v8STR(isolate, "$co"),
                FunctionTemplate::New(isolate, Co));
    global->Set(v8STR(isolate, "$now"),
                FunctionTemplate::New(isolate, Now));
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
    global->Set(v8STR(isolate, "$utf8String"),
                FunctionTemplate::New(isolate, utf8String));
    global->Set(v8STR(isolate, "$toArrayBuffer"),
                FunctionTemplate::New(isolate, ToArrayBuffer));
    global->Set(v8STR(isolate, "$pack"),
                FunctionTemplate::New(isolate, Pack));
    global->Set(v8STR(isolate, "$unpack"),
                FunctionTemplate::New(isolate, Unpack));
    global->Set(v8STR(isolate, "$$onPromiseReject"),
                FunctionTemplate::New(isolate, OnPromiseReject));

    v8Context context = Context::New(isolate, NULL, global);
    if (context.IsEmpty()) {
        fprintf(stderr, "fatal: failed to create a V8 context\n");
        exit(1);
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

    nullptr_obj->SetInternalField(1, External::New(isolate, nullptr));
    nullptr_obj->SetPrototype(v8Value::New(isolate, vm->cptr_proto));

    // Create Go object template.
    MakeGoTemplate(vm);

    // Name the global object "Global".
    v8Object realGlobal = v8Object::Cast(
                        context->Global()->GetPrototype());
    realGlobal->Set(v8STR(isolate, "Global"), realGlobal);
    realGlobal->Set(v8STR(isolate, "$nullptr"), nullptr_obj);
    realGlobal->Set(v8STR(isolate, "$$persist"), MakePersist(isolate));

    realGlobal->SetAccessor(context, v8STR(isolate, "$errno"),
                    GlobalGet, GlobalSet).FromJust();
    realGlobal->SetAccessor(context, v8STR(isolate, "$$ncoro"),
                    GlobalGet).FromJust();

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
    vm->on_promise_reject = 0;
}

// Runs in the worker(V8) thread.
int js8_vminit(v8_state vm) {
    CreateIsolate(vm);
    sigset_t mask;
    sigfillset(&mask);
    /* unblock all signals */
    if (pthread_sigmask(SIG_UNBLOCK, &mask, NULL) == -1)
        fprintf(stderr, "pthread_sigmask: %s\n", strerror(errno));
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
        panic(isolate, "get: invalid handle", NULL);
    if (!v1->IsObject())
        panic(isolate, "get: not an object", NULL);
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
        panic(isolate, "set: invalid handle", NULL);
    if (!v1->IsObject())
        panic(isolate, "set: not an object", NULL);
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
        panic(isolate, "geti: invalid handle", NULL);
    if (!v1->IsObject())
        panic(isolate, "geti: not an object", NULL);
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
        panic(isolate, "seti: invalid handle", NULL);
    if (!v1->IsObject())
        panic(isolate, "seti: not an object", NULL);
    v8Object obj = v8Object::Cast(v1);
    if (! v2->IsUndefined())
        return obj->Set(context, index, v2).FromJust();
    return 1;   // undefined -> do nothing
}

// Import a C function.
static v8_val v8_cfunc(v8_state vm, const v8_ffn *func_item) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate);
    if (func_item->pcount > MAXARGS)
        panic(isolate, "C-type function with too many parameters",
                func_item->name);
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    v8Object fnObj;
    if (func_item->type == FN_CTYPE)
        fnObj = WrapFunc(vm, (v8_ffn *) func_item);
    else
        fnObj = WrapGo(vm, (v8_ffn *) func_item);
    return ctype_handle(NewPersister(vm, fnObj));
}

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
    if (GetObjectId(obj) != V8EXTPTR)
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
    if (GetObjectId(args[0]) == V8EXTFUNC) {
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

static struct v8_api_s v8api = {
    v8_object,
    v8_get,
    v8_set,
    v8_geti,
    v8_seti,
    v8_array,
    v8_reset,
    v8_cfunc,
    v8_goresolve,
    v8_goreject,
    v8_callstr,
    v8_call,
    &global_,
    &null_,
    v8_task,
    v8_ctypestr,
    v8_panic,
    v8_ctype_errs,
};

struct v8_api_s *jsv8 = &v8api;

typedef int (*Fnload)(v8_state, v8_val, v8_api_s *const, v8_ffn **);

// $load - load a dynamic library.
// The filename must contain a slash. Any path search should be
// done in the JS.

static void Load(const FunctionCallbackInfo<Value>& args) {
    ISOLATE_SCOPE(args, isolate)
    v8_state vm = reinterpret_cast<v8_state>(isolate->GetData(0));
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
    int nfunc = load_func(vm, val1, &v8api, &functab);
    DeletePersister(vm, val1.hndle);

    if (nfunc < 0) {
        dlclose(dl);
        ThrowError(isolate, errno ? strerror(errno) : "$load: unknown error");
    }
    for (int i = 0; i < nfunc && functab[i].name; i++) {
        v8Object fnObj;
        if (functab[i].pcount > MAXARGS)
            panic(isolate, "$load: C function with too many parameters",
                functab[i].name);
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
