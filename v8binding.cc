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

static v8_handle MakeHandle(js_vm *vm, v8Value value);
static void DeleteHandle(js_vm *vm, v8_handle h);
#define CreateHandle(vm, val)   (vm)->store_->Set(val)
#define GetValueFromHandle(vm, h)    (vm)->store_->Get(h)

void v8_set_errstr(js_vm *vm, const char *str) {
    if (vm->errstr) {
        free((void *) vm->errstr);
        vm->errstr = nullptr;
    }
    if (str)
        vm->errstr = estrdup(str, strlen(str));
}

static void SetError(js_vm *vm, TryCatch *try_catch) {
    v8_set_errstr(vm, GetExceptionString(vm->isolate, try_catch));
}

static void Panic(Isolate *isolate, TryCatch *try_catch) {
    char *errstr = GetExceptionString(isolate, try_catch);
    fprintf(stderr, "%s\n", errstr);
    exit(1);
}

////////////////////////////////// Coroutine //////////////////////////////////
#define GOROUTINE_INTERNAL_FIELD_COUNT 4
enum {
    GoError = (1 << 0),
    GoDone = (1 << 1),
};

struct GoCallback {
    v8_handle h;
    void *data;
    int flags;
};

struct Go_s {
    js_vm *vm;
    Fngo fp;
    void *inp;
    v8_handle h;
};

static v8Object NewGo(js_vm *vm, void *fptr) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ = v8ObjectTemplate::New(isolate, vm->go_template);
    v8Object obj = templ->NewInstance(
                        isolate->GetCurrentContext()).ToLocalChecked();
    obj->SetAlignedPointerInInternalField(0,
             reinterpret_cast<void*>(static_cast<uintptr_t>(V8GO<<2)));
    obj->SetInternalField(1, External::New(isolate, fptr));
    // The rest of the internal fields are "Undefined".
    return handle_scope.Escape(obj);
}

static v8Object ToGo(v8Value v) {
    if (!v.IsEmpty() && v->IsObject()) {
        v8Object cr = v8Object::Cast(v);
        if (cr->InternalFieldCount() == GOROUTINE_INTERNAL_FIELD_COUNT
            && (V8GO == static_cast<int>(reinterpret_cast<uintptr_t>(
                    cr->GetAlignedPointerFromInternalField(0)) >> 2))
        )
        return cr;
    }
    return v8Object();  // Empty handle
}

static v8Object CloneGo(js_vm *vm, v8Object cr1) {
    Isolate *isolate = vm->isolate;
    v8Context context = isolate->GetCurrentContext();
    EscapableHandleScope handle_scope(isolate);
    v8Object cr2 = NewGo(vm,
                    v8External::Cast(cr1->GetInternalField(1))->Value());
    v8Array props1 = cr1->GetOwnPropertyNames(context).ToLocalChecked();
    unsigned num_props1 = props1->Length();
    for (unsigned i = 0; i < num_props1; i++) {
        v8Value property;
        if (props1->Get(context, i).ToLocal(&property)) {
            cr2->Set(context, property,
                    cr1->Get(context, property).ToLocalChecked()).FromJust();
        }
    }
    return handle_scope.Escape(cr2);
}

// Start a $go() coroutine in the main thread.
coroutine static void start_go(mill_pipe inq) {
    while (true) {
        int done;
        Go_s *g = *((Go_s **) mill_piperecv(inq, &done));
        if (done)
            break;
        go(g->fp(g->vm, g->h, g->inp));
    }
}

// Receive coroutine callbacks from the main thread.
coroutine static void recv_go(js_vm *vm) {
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

static void send_go(js_vm *vm, Go_s *g) {
    int rc = mill_pipesend(vm->inq, (void *) &g);
    if (rc == -1) {
        // FIXME
    }
}

// $go(coro, input [, callback])
static void Go(const FunctionCallbackInfo<Value>& args) {
    Go_s *g;
    js_vm *vm;

    if (true) { // start V8 scope

    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    int argc = args.Length();
    ThrowNotEnoughArgs(isolate, argc < 3);
    vm = reinterpret_cast<js_vm*>(isolate->GetData(0));

    if ((GetCtypeId(vm, args[1]) != V8EXTPTR) && !args[1]->IsArrayBuffer())
        ThrowTypeError(isolate,
                "$go argument #2: pointer or ArrayBuffer expected");
    if (!args[2]->IsFunction())
        ThrowTypeError(isolate, "$go argument #3: function expected");
    v8Object cr0 = ToGo(args[0]);
    if (cr0.IsEmpty())
        ThrowTypeError(isolate, "$go argument #1: coroutine expected");

    v8Object cr = CloneGo(vm, cr0);
    g = new Go_s;
    g->vm = vm;
    vm->ncoro++;

    if (args[1]->IsArrayBuffer())
        g->inp = v8ArrayBuffer::Cast(args[1])->GetContents().Data();
    else
        g->inp = UnwrapPtr(v8Object::Cast(args[1]));

    cr->SetInternalField(2, args[2]);   // callback

    cr->SetInternalField(3, args[1]);   // input

    g->fp = reinterpret_cast<Fngo>(
                    v8External::Cast(cr->GetInternalField(1))->Value());

    cr->SetAlignedPointerInInternalField(1, g); // overwrite field #1

    g->h = vm->store_->Set(cr);

    assert(g->h);

    /* Unlocker unlocker(isolate); */

    }   // end V8 scope

    send_go(vm, g);
}

// Execute callback(err, data). Panic if any exception thrown.
static void RunGoCallback(js_vm *vm, GoCallback *cb) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)

    v8Object cr = ToGo(vm->store_->Get(cb->h));
    if (cr.IsEmpty()) {
        fprintf(stderr, "go callback: invalid coroutine handle\n");
        exit(1);
    }

    if ((cb->flags & GoDone) != 0) {
        /* From godone() */
        vm->ncoro--;
        Go_s *g = reinterpret_cast<Go_s *>(
                        cr->GetAlignedPointerFromInternalField(1));
        vm->store_->Dispose(cb->h);
        delete g;
        delete cb;
        return;
    }

    TryCatch try_catch(isolate);

    // GetCurrentContext() is empty if called from WaitFor().
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);

    v8Function func = v8Function::Cast(cr->GetInternalField(2));
    v8Value args[2];
    if (cb->flags & GoError) {
        char *message = reinterpret_cast<char *>(cb->data);
        args[0] = Exception::Error(V8_STR(isolate, message));
        args[1] = v8::Null(isolate);
    } else {
        args[0] = v8::Null(isolate);
        args[1] = WrapPtr(vm, cb->data);
    }
    delete cb;

    assert(!try_catch.HasCaught());
    func->Call(context->Global(), 2, args);
    if (try_catch.HasCaught()) {
        Panic(isolate, &try_catch);
#if 0
        char *errstr = GetExceptionString(isolate, &try_catch);
        fprintf(stderr, "%s\n", errstr);
#endif
    }
}

static void WaitFor(js_vm *vm) {
    while (vm->ncoro > 0) {
        DPRINT("[[ WaitFor: %d unfinished coroutines. ]]\n", vm->ncoro);
        GoCallback *cb = chr(vm->ch, GoCallback *);
        assert(cb);
        RunGoCallback(vm, cb);
    }
}

static void MakeGoTemplate(js_vm *vm) {
    Isolate *isolate = vm->isolate;
    HandleScope handle_scope(isolate);
    v8ObjectTemplate templ = ObjectTemplate::New(isolate);
    templ->SetInternalFieldCount(GOROUTINE_INTERNAL_FIELD_COUNT);
    vm->go_template.Reset(isolate, templ);
}

// XXX: not needed for DLL
v8_handle v8_go(js_vm *vm, Fngo fptr) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    v8Value gObj = NewGo(vm, reinterpret_cast<void *>(fptr));
    return MakeHandle(vm, gObj);
}

static GoCallback *gosend_(js_vm *vm, v8_handle hcr, void *data) {
    GoCallback *cb = new GoCallback;
    cb->h = hcr;
    cb->data = data;
    cb->flags = 0;
    // Coroutine is running in a different thread.
    int rc = mill_pipesend(vm->outq, &cb);
    assert(rc == 0);
    return cb;
}

// gosend(vm, coro, data)
//  Defer verifying handle to avoid lock contention.

int v8_gosend(js_vm *vm, v8_handle hcr, void *data) {
    return (gosend_(vm, hcr, data) != nullptr);
}

int v8_goerr(js_vm *vm, v8_handle hcr, char *message) {
    GoCallback *cb = gosend_(vm, hcr, reinterpret_cast<void *>(message));
    if (cb)
        cb->flags = GoError;
    return (cb != nullptr);
}

int v8_godone(js_vm *vm, v8_handle hcr) {
    GoCallback *cb = gosend_(vm, hcr, nullptr);
    if (cb)
        cb->flags = GoDone;
    return (cb != nullptr);
}

///////////////////////////////////End Coroutine////////////////////////////////

static v8Object WrapFunc(js_vm *vm, v8_ffn *func_item) {
    Isolate *isolate = vm->isolate;
    EscapableHandleScope scope(isolate);
    v8ObjectTemplate templ =
        v8ObjectTemplate::New(isolate, vm->extfunc_template);
    v8Object fnObj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    fnObj->SetAlignedPointerInInternalField(0,
             reinterpret_cast<void*>(static_cast<uintptr_t>(V8EXTFUNC<<2)));
    fnObj->SetInternalField(1, External::New(isolate, (void *)func_item));
    fnObj->SetPrototype(v8Value::New(isolate, vm->ctype_proto));
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

// $msleep(milliseconds_to_sleep).
static void MSleep(const FunctionCallbackInfo<Value>& args) {
    int64_t n = 0;
    if (true) {
        Isolate *isolate = args.GetIsolate();
        HandleScope handle_scope(isolate);
        if (args.Length() > 0)
            n = args[0]->IntegerValue(
                    isolate->GetCurrentContext()).FromJust();
    }
    mill_sleep(now()+n);
}

static void Close(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    mill_pipeclose(vm->inq);
}

// $eval(string, origin) -- Must compile and run.
static void EvalString(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);

    int argc = args.Length();
    if (argc < 2 || !args[0]->IsString()) {
        fprintf(stderr, "$eval: invalid argument(s)\n");
        exit(1);
    }
    v8Context context = isolate->GetCurrentContext();
    TryCatch try_catch(isolate);
    v8String source = v8String::Cast(args[0]);
    v8String name;
    if (!args[1]->ToString(context).ToLocal(&name)) {
        Panic(isolate, &try_catch);    
    }

    ScriptOrigin origin(name);
    v8Script script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        Panic(isolate, &try_catch);
    }
    v8Value result;
    if (!script->Run(context).ToLocal(&result)) {
        Panic(isolate, &try_catch);
    }
    assert(!result.IsEmpty());
    args.GetReturnValue().Set(result);
}

// malloc(size [, zerofill])
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
    args.GetReturnValue().Set(ptrObj);
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
    args.GetReturnValue().Set(V8_STR(isolate, strerror(errnum)));
}

static void CallForeignFunc(
        const v8::FunctionCallbackInfo<v8::Value>& args) {

    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = reinterpret_cast<js_vm*>(isolate->GetData(0));

    v8Object obj = args.Holder();
    assert(obj->InternalFieldCount() == 2);
    v8_ffn *func_wrap = reinterpret_cast<v8_ffn *>(
                v8External::Cast(obj->GetInternalField(1))->Value());
    int argc = args.Length();
    if (argc > MAXARGS || argc != func_wrap->pcount)
        ThrowError(isolate, "C-function called with incorrect # of arguments");

    if ((func_wrap->flags & V8_DLFUNC)) {
        assert(vm->dlstr_idx == 0);
        v8Value argv[MAXARGS+1];
        // N.B.: argv[0] is for the return value
        for (int i = 0; i < argc; i++)
            argv[i + 1] = args[i];
        argv[0] = v8::Undefined(isolate);   // Default return type is void.

        v8_set_errstr(vm, nullptr);
        ((Fndlfnwrap) func_wrap->fp)(vm, argc, reinterpret_cast<v8_val>(argv));

        while (vm->dlstr_idx > 0) {
            free(vm->dlstr[--vm->dlstr_idx]);
        }
        if (vm->errstr)
            ThrowError(isolate, vm->errstr);
        args.GetReturnValue().Set(argv[0]);
    } else {
        v8_handle argh[MAXARGS+1];
        for (int i = 0; i < argc; i++)
            argh[i+1] = MakeHandle(vm, args[i]);
        argh[0] = vm->undef_handle;
        // XXX: use v8_set_errstr() and return 0 in case of error.
        argh[0] = ((Fnfnwrap) func_wrap->fp)(vm, argc, argh);
        v8Value retv = v8Value();
        if (argh[0]) {
            retv = GetValueFromHandle(vm, argh[0]);
        }
        for (int i = 0; i <= argc; i++)
            DeleteHandle(vm, argh[i]);
        if (retv.IsEmpty())
            ThrowError(isolate, (vm->errstr ? vm->errstr : "unknown error"));
        args.GetReturnValue().Set(retv);
    }
}

// TODO: use pthread_once
static int v8initialized = 0;

static void v8init(void) {
    if (v8initialized)
        return;
    // N.B.: V8BINDIR must have a trailing slash!
    V8::InitializeExternalStartupData(V8BINDIR);

#ifdef V8TEST
    // Enable garbage collection
    const char* flags = "--expose_gc";
    V8::SetFlagsFromString(flags, strlen(flags));
#endif

    Platform* platform = platform::CreateDefaultPlatform();
    V8::InitializePlatform(platform);
    V8::Initialize();
    create_params.array_buffer_allocator = &allocator;
    v8initialized = 1;
}

// Invoked by the main thread.
js_vm *js8_vmnew(mill_worker w) {
    js_vm *vm = new js_vm;
    vm->w = w;
    vm->inq = mill_pipemake(sizeof (void *));
    assert(vm->inq);
    vm->outq = mill_pipemake(sizeof (void *));
    assert(vm->outq);
    vm->ch = mill_chmake(sizeof (GoCallback *), 64); // XXX: How to select a bufsize??
    assert(vm->ch);
    vm->ncoro = 0;
    vm->errstr = nullptr;
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
void js8_vmclose(js_vm *vm) {
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

        if (vm->errstr)
            free(vm->errstr);
    }

    isolate->Dispose();
    delete vm;
}

static v8_handle MakeHandle(js_vm *vm, v8Value val) {
    if (val->IsUndefined())
        return vm->undef_handle;
    if (val->IsNull())
        return vm->null_handle;
    return CreateHandle(vm, val);
}

static void DeleteHandle(js_vm *vm, v8_handle h) {
    if (h > NUM_PERMANENT_HANDLES)
        vm->store_->Dispose(h);
}

static v8_handle CompileRun(js_vm *vm, const char *src) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)

    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);

    TryCatch try_catch(isolate);
    const char *script_name = "<string>";   // TODO: optional argument
    v8String name(V8_STR(isolate, script_name));
    v8String source(V8_STR(isolate, src));

    ScriptOrigin origin(name);
    v8Script script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        SetError(vm, &try_catch);
        return 0;
    }

    Handle<Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        SetError(vm, &try_catch);
        return 0;
    }
    assert(!result.IsEmpty());
    return MakeHandle(vm, result);
}

static v8_handle CallFunc(struct js8_cmd_s *cmd) {
    js_vm *vm = cmd->vm;
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);

    v8Value v1;
    if (cmd->type == V8CALLSTR) {
        // function expression
        assert(cmd->source);
        v8_handle result = CompileRun(vm, cmd->source);
        if (!result)
            return 0;
        v1 = GetValueFromHandle(vm, result);
        DeleteHandle(vm, result);
    } else
        v1 = GetValueFromHandle(vm, cmd->h1);

    if (v1.IsEmpty() || !v1->IsFunction()) {
        v8_set_errstr(vm, "js_call: function argument #1 expected");
        return 0;
    }
    v8Function func = v8Function::Cast(v1);

    int argc = cmd->nargs;
    assert(argc <= 4);

    TryCatch try_catch(isolate);
    v8Object self = context->Global();
    if (cmd->h) {
        v8Value v = GetValueFromHandle(vm, cmd->h);
        if (v.IsEmpty()) {
            v8_set_errstr(vm, "js_call: invalid handle");
            return 0;
        }
        self = v->ToObject(context).ToLocalChecked();
    }

    v8Value argv[4];
    int i;
    for (i = 0; i < argc; i++) {
        argv[i] = GetValueFromHandle(vm, cmd->a[i]);
        // FIXME -- bail out
        assert(!argv[i].IsEmpty());
    }

    v8Value result = func->Call(
            context, self, argc, argv).FromMaybe(v8Value());
    if (try_catch.HasCaught()) {
        SetError(vm, &try_catch);
        return 0;
    }
    return MakeHandle(vm, result);
}

int js8_do(struct js8_cmd_s *cmd) {
    switch (cmd->type) {
    case V8COMPILERUN:
        assert(cmd->vm->ncoro == 0);
        cmd->h = CompileRun(cmd->vm, cmd->source);
        WaitFor(cmd->vm);
        break;
    case V8CALL:
    case V8CALLSTR:
        cmd->h = CallFunc(cmd);
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
        cmd->weak_counter = cmd->vm->weak_counter;
    }
#endif
        break;
    default:
        fprintf(stderr, "error: js8_do(): received unexpected code");
        abort();
    }
    return 1;
}


////////////////////////////// DLL ///////////////////////////
#define ARGV reinterpret_cast<v8Value *>(argv)[arg_num]
#define ISOLATE(vm)   (vm->isolate)
#define CURR_CONTEXT(vm)   ISOLATE(vm)->GetCurrentContext()
#define CHECK_NUMBER(vm, v) \
    v8Value v = ARGV; \
    if (! v->IsNumber() && ! v->IsBoolean()) {\
        v8_set_errstr(vm, "C-type argument is not a number");\
        return 0;\
    }

// N.B.: There is no type coercion in any of these JS to C
// conversion routines.
static int to_int(js_vm *vm, int arg_num, v8_val argv) {
    CHECK_NUMBER(vm, v)
    return v->Int32Value(CURR_CONTEXT(vm)).FromJust();
}

static unsigned int to_uint(js_vm *vm, int arg_num, v8_val argv) {
    CHECK_NUMBER(vm, v)
    return v->Uint32Value(CURR_CONTEXT(vm)).FromJust();
}

static int64_t to_long(js_vm *vm, int arg_num, v8_val argv) {
    v8Value v = ARGV;
    long64 i;
    if (LongValue(v, &i))
        return i.val.i64;
    if (! v->IsNumber() && ! v->IsBoolean()) {
        v8_set_errstr(vm, "C-type argument is not a number");
        return 0;
    }
    return v->IntegerValue(CURR_CONTEXT(vm)).FromJust();
}

static uint64_t to_ulong(js_vm *vm, int arg_num, v8_val argv) {
    v8Value v = ARGV;
    long64 i;
    if (LongValue(v, &i))
        return i.val.u64;
    if (! v->IsNumber() && ! v->IsBoolean()) {
        v8_set_errstr(vm, "C-type argument is not a number");
        return 0;
    }
    double d = v->NumberValue(CURR_CONTEXT(vm)).FromJust();
    if (isfinite(d))
        return (uint64_t) d;
    return 0;
}

static double to_double(js_vm *vm, int arg_num, v8_val argv) {
    CHECK_NUMBER(vm, v)
    return v->NumberValue(CURR_CONTEXT(vm)).FromJust();
}

static char *to_string(js_vm *vm, int arg_num, v8_val argv) {
    v8Value v = ARGV;
    if (!v->IsString()) {
        if (GetCtypeId(vm, v) == V8EXTPTR) {
            return (char *) v8External::Cast(
                    v8Object::Cast(v)->GetInternalField(1))->Value();
        }
        v8_set_errstr(vm, "C-type argument is not a string");
        return nullptr;
    }
    v8String s = v->ToString(CURR_CONTEXT(vm)).ToLocalChecked();
    int utf8len = s->Utf8Length();	/* >= 0 */
    char *p = (char *) emalloc(utf8len + 1);
    int l = s->WriteUtf8(p, utf8len);
    p[l] = '\0';
    /* Keep track of memory so can free upon return; See CallForeignFunc() */
    vm->dlstr[vm->dlstr_idx++] = p;
    return p;
}

static void *to_pointer(js_vm *vm, int arg_num, v8_val argv) {
    v8Value v = ARGV;
    if (GetCtypeId(vm, v) != V8EXTPTR) {
        v8_set_errstr(vm, "C-type argument is not a pointer");
        return nullptr;
    }
    return v8External::Cast(
                v8Object::Cast(v)->GetInternalField(1))->Value();
}

static v8_handle to_handle(js_vm *vm, int arg_num, v8_val argv) {
    return MakeHandle(vm, ARGV);
}

#define RETVAL reinterpret_cast<v8Value *>(argv)[0]

static void from_int(js_vm *vm, int i, v8_val argv) {
    RETVAL = Integer::New(ISOLATE(vm), i);
}

static void from_uint(js_vm *vm, unsigned ui, v8_val argv) {
    RETVAL = Integer::NewFromUnsigned(ISOLATE(vm), ui);
}

static void from_long(js_vm *vm, int64_t i, v8_val argv) {
    RETVAL = Int64(vm, i);
}

static void from_ulong(js_vm *vm, uint64_t ui, v8_val argv) {
    RETVAL = UInt64(vm, ui);
}

static void from_double(js_vm *vm, double d, v8_val argv) {
    RETVAL = Number::New(ISOLATE(vm), d);
}

static void from_pointer(js_vm *vm, void *ptr, v8_val argv) {
    if (!ptr)
        RETVAL = vm->store_->Get(vm->nullptr_handle);
    else
        RETVAL = WrapPtr(vm, ptr);
}

static void from_handle(js_vm *vm, v8_handle h, v8_val argv) {
    v8Value v = GetValueFromHandle(vm, h);
    if (!v.IsEmpty())
        RETVAL = v;
    else
        v8_set_errstr(vm, "invalid handle");
}

static struct v8_dlfn_s v8dlfns = {
    to_int,
    to_uint,
    to_long,
    to_ulong,
    to_double,
    to_string,
    to_pointer,
    from_int,
    from_uint,
    from_long,
    from_ulong,
    from_double,
    from_pointer,
    v8_errstr,
};

static struct v8_fn_s v8fns = {
    to_handle,
    from_handle,
    v8_number,
    v8_tonumber,
    v8_int32,
    v8_toint32,
    v8_string,
    v8_tostring,
    v8_object,
    v8_get,
    v8_set,
    v8_geti,
    v8_seti,
    v8_array,
    v8_reset,
    v8_dispose,
    v8_global,
    v8_null,
    v8_go,
    v8_gosend,
    v8_goerr,
    v8_godone,
    v8_errstr,
    v8_callstr,
};

typedef int (*Fnload)(js_vm *, v8_handle, v8_dlfn_s *const, v8_fn_s *const, v8_ffn **);

// $load - load a dynamic library.
// The filename must contain a slash. Any path search should be
// done in the JS.

static void Load(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = reinterpret_cast<js_vm*>(isolate->GetData(0));

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
    v8_handle h1 = CreateHandle(vm, o1);
    v8_set_errstr(vm, nullptr);
    int nfunc = load_func(vm, h1, &v8dlfns, &v8fns, &functab);
    DeleteHandle(vm, h1);

    if (nfunc < 0) {
        dlclose(dl);
        ThrowError(isolate, vm->errstr ? vm->errstr : "unknown error");
    }
    for (int i = 0; i < nfunc && functab[i].name; i++) {
        v8Object fnObj;
        if (functab[i].flags & V8_DLCORO) {
            fnObj = NewGo(vm, functab[i].fp);
        } else {
            functab[i].flags |= V8_DLFUNC;
            fnObj = WrapFunc(vm, &functab[i]);
        }
        o1->Set(context,
                String::NewFromUtf8(isolate, functab[i].name),
                fnObj).FromJust();
    }
    args.GetReturnValue().Set(o1);
}

///////////////////////////////////////////////////////////////
static void PersistentSet(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    js_vm *vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    unsigned k = MakeHandle(vm, args[0]);
    args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, k));
}

static void PersistentGet(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    unsigned k = args[0]->Uint32Value(isolate->GetCurrentContext()).FromJust();
    js_vm *vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    v8Value v = vm->store_->Get(k);
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
    js_vm *vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    DeleteHandle(vm, k);
}

static v8Value MakePersistent(Isolate *isolate) {
    EscapableHandleScope handle_scope(isolate);
    v8ObjectTemplate templ = ObjectTemplate::New(isolate);
    templ->Set(V8_STR(isolate, "set"),
                FunctionTemplate::New(isolate, PersistentSet));
    templ->Set(V8_STR(isolate, "get"),
                FunctionTemplate::New(isolate, PersistentGet));
    templ->Set(V8_STR(isolate, "delete"),
                FunctionTemplate::New(isolate, PersistentDelete));
    // Create the one and only instance.
    v8Object p = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    return handle_scope.Escape(p);
}

// The second part of the vm initialization.
static void CreateIsolate(js_vm *vm) {
    v8init();
    Isolate* isolate = Isolate::New(create_params);
    LOCK_SCOPE(isolate)

    assert(isolate->GetCurrentContext().IsEmpty());
    vm->isolate = isolate;
    vm->dlstr_idx = 0;

    // isolate->SetCaptureStackTraceForUncaughtExceptions(true);
    isolate->SetData(0, vm);

    v8ObjectTemplate global = ObjectTemplate::New(isolate);
    global->Set(V8_STR(isolate, "$print"),
                FunctionTemplate::New(isolate, Print));
    global->Set(V8_STR(isolate, "$go"),
                FunctionTemplate::New(isolate, Go));
    global->Set(V8_STR(isolate, "$msleep"),
                FunctionTemplate::New(isolate, MSleep));
    global->Set(V8_STR(isolate, "$now"),
                FunctionTemplate::New(isolate, Now));
    global->Set(V8_STR(isolate, "$close"),
                FunctionTemplate::New(isolate, Close));
    global->Set(V8_STR(isolate, "$eval"),
                FunctionTemplate::New(isolate, EvalString));
    global->Set(V8_STR(isolate, "$malloc"),
                FunctionTemplate::New(isolate, Malloc));
    global->Set(V8_STR(isolate, "$strerror"),
                FunctionTemplate::New(isolate, StrError));
    global->Set(V8_STR(isolate, "$load"),
                FunctionTemplate::New(isolate, Load));
    global->Set(V8_STR(isolate, "$lcntl"),
                FunctionTemplate::New(isolate, LongCntl));
    global->Set(V8_STR(isolate, "$long"), LongTemplate(vm));

    v8Context context = Context::New(isolate, NULL, global);
    if (context.IsEmpty()) {
        fprintf(stderr, "failed to create a V8 context\n");
        exit(1);
    }

    vm->context.Reset(isolate, context);

    Context::Scope context_scope(context);

    // Make the template for external(foreign) pointer objects.
    v8ObjectTemplate extptr_templ = ObjectTemplate::New(isolate);
    extptr_templ->SetInternalFieldCount(2);
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

    // Create __proto__ for C-type objects.
    MakeCtypeProto(vm);

    v8Object nullptr_obj = extptr_templ
                -> NewInstance(context).ToLocalChecked();
    nullptr_obj->SetAlignedPointerInInternalField(0,
            reinterpret_cast<void*>(static_cast<uintptr_t>(V8EXTPTR<<2)));
    nullptr_obj->SetInternalField(1, External::New(isolate, nullptr));
    nullptr_obj->SetPrototype(v8Value::New(isolate, vm->cptr_proto));

    // Create Go object template.
    MakeGoTemplate(vm);

    // Name the global object "Global".
    v8Object realGlobal = v8Object::Cast(
                        context->Global()->GetPrototype());
    realGlobal->Set(V8_STR(isolate, "Global"), realGlobal);
    realGlobal->Set(V8_STR(isolate, "$nullptr"), nullptr_obj);
    realGlobal->Set(V8_STR(isolate, "$store"), MakePersistent(isolate));

    realGlobal->SetAccessor(context, V8_STR(isolate, "$errno"),
                    GlobalGet, GlobalSet).FromJust();

    // Initial size should be some multiple of 32.
    vm->store_ = new PersistentStore(isolate, 1024);

    vm->undef_handle = vm->store_->Set(v8::Undefined(isolate));
    vm->null_handle = vm->store_->Set(v8::Null(isolate));
    vm->global_handle = vm->store_->Set(realGlobal);
    vm->nullptr_handle = vm->store_->Set(nullptr_obj);

    assert(vm->nullptr_handle == NUM_PERMANENT_HANDLES);
}

// Runs in the worker(V8) thread.
int js8_vminit(js_vm *vm) {
    CreateIsolate(vm);
    go(recv_go(vm));
    return 0;
}

////////////////////////////////////////////////////////////////////////
const char *v8_errstr(v8_state vm) {
    return vm->errstr;
}

v8_handle v8_global(v8_state vm) {
    return vm->global_handle;
}

v8_handle v8_null(v8_state vm) {
    return vm->null_handle;
}

v8_handle v8_number(v8_state vm, double d) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    return CreateHandle(vm, Number::New(isolate, d));
}

double v8_tonumber(v8_state vm, v8_handle h) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v = GetValueFromHandle(vm, h);
    if (v.IsEmpty()) {
        v8_set_errstr(vm, "v8_tonumber: invalid handle");
        return 0;
    }
    v8Context context = v8Context::New(isolate, vm->context);
    return v->NumberValue(context).FromJust();
}

v8_handle v8_int32(v8_state vm, int32_t i) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    return CreateHandle(vm, Integer::New(isolate, i));
}

int32_t v8_toint32(v8_state vm, v8_handle h) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v = GetValueFromHandle(vm, h);
    if (v.IsEmpty()) {
        v8_set_errstr(vm, "v8_toint32: invalid handle");
        return 0;
    }
    v8Context context = v8Context::New(isolate, vm->context);
    return v->Int32Value(context).FromJust();
}

v8_handle v8_string(v8_state vm, const char *stptr, int length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    assert(stptr);
    if (length < 0)
        length = strlen(stptr);
    return CreateHandle(vm, String::NewFromUtf8(isolate, stptr,
                            v8::String::kNormalString, length));
}

char *v8_tostring(v8_state vm, v8_handle h) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v = GetValueFromHandle(vm, h);
    if (v.IsEmpty()) {
        v8_set_errstr(vm, "v8_tostring: invalid handle");
        return nullptr;
    }
    v8Context context = v8Context::New(isolate, vm->context);
    String::Utf8Value stval(v->ToString(context).ToLocalChecked());
    /* return empty string if there was an error during conversion. */
    return estrdup(*stval, stval.length());
}

v8_handle v8_object(v8_state vm) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    /* XXX: need a context in this case unlike String or Number!!! */
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    return CreateHandle(vm, Object::New(isolate));
}

v8_handle v8_get(v8_state vm, v8_handle hobj, const char *key) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = GetValueFromHandle(vm, hobj);
    if (v1.IsEmpty()) {
        v8_set_errstr(vm, "v8_get: invalid handle");
        return 0;
    }
    if (!v1->IsObject()) {
        v8_set_errstr(vm, "v8_get: object argument expected");
        return 0;
    }
    v8Object obj = v8Object::Cast(v1);
    v8Value v2 = obj->Get(v8Context::New(isolate, vm->context),
                V8_STR(isolate, key)).ToLocalChecked();
    return MakeHandle(vm, v2);
}

int v8_set(v8_state vm, v8_handle hobj, const char *key, v8_handle hval) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    if (!hval)
        hval = vm->null_handle;
    v8Value v1 = GetValueFromHandle(vm, hobj);
    v8Value v2 = GetValueFromHandle(vm, hval);
    if (v1.IsEmpty() || v2.IsEmpty()) {
        v8_set_errstr(vm, "v8_set: invalid handle");
        return 0;
    }
    if (!v1->IsObject()) {
        v8_set_errstr(vm, "v8_set: object argument expected");
        return 0;
    }
    v8Object obj = v8Object::Cast(v1);
    return obj->Set(v8Context::New(isolate, vm->context),
                    V8_STR(isolate, key), v2).FromJust();
}

v8_handle v8_array(v8_state vm, int length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    return CreateHandle(vm, Array::New(isolate, length));
}

v8_handle v8_geti(v8_state vm, v8_handle hobj, unsigned index) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = GetValueFromHandle(vm, hobj);
    if (v1.IsEmpty()) {
        v8_set_errstr(vm, "v8_geti: invalid handle");
        return 0;
    }
    if (!v1->IsObject()) {
        v8_set_errstr(vm, "v8_geti: object argument expected");
        return 0;
    }
    v8Object obj = v8Object::Cast(v1);

    // Undefined for nonexistent index.
    v8Value v2 = obj->Get(v8Context::New(isolate, vm->context),
                        index).ToLocalChecked();
    return MakeHandle(vm, v2);
}

int v8_seti(v8_state vm, v8_handle hobj, unsigned index, v8_handle hval) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    if (!hval)
        hval = vm->null_handle;
    v8Value v1 = GetValueFromHandle(vm, hobj);
    v8Value v2 = GetValueFromHandle(vm, hval);
    if (v1.IsEmpty() || v2.IsEmpty()) {
        v8_set_errstr(vm, "v8_seti: invalid handle");
        return 0;
    }
    if (!v1->IsObject()) {
        v8_set_errstr(vm, "v8_seti: object argument expected");
        return 0;
    }
    v8Object obj = v8Object::Cast(v1);
    return obj->Set(v8Context::New(isolate, vm->context),
                        index, v2).FromJust();
}

v8_handle v8_pointer(v8_state vm, void *ptr) {
    if (!ptr)
        return vm->nullptr_handle;
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    return CreateHandle(vm, WrapPtr(vm, ptr));
}

void *v8_topointer(v8_state vm, v8_handle h) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v = GetValueFromHandle(vm, h);
    if (v.IsEmpty()) {
        v8_set_errstr(vm, "v8_topointer: invalid handle");
        return nullptr;
    }
    if (v->IsArrayBuffer()) {
        // The pointer will be invalid if the ArrayBuffer gets
        // garbage collected!.
        return v8ArrayBuffer::Cast(v)->GetContents().Data();
    }

    v8Object obj = ToPtr(v);
    if (obj.IsEmpty()) {
        v8_set_errstr(vm, "v8_topointer: pointer argument expected");
        return nullptr;
    }
    // Clear error
    v8_set_errstr(vm, nullptr);
    return UnwrapPtr(obj);
}

// Import a C function.
v8_handle v8_cfunc(v8_state vm, const v8_ffn *func_item) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate);
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    ((v8_ffn *) func_item)->flags = 0;
    v8Object fnObj = WrapFunc(vm, (v8_ffn *) func_item);
    return CreateHandle(vm, fnObj);
}

/* N.B.: V8 owns the Buffer memory. If ptr is not NULL, it must be
 * compatible with ArrayBuffer::Allocator::Free. */
v8_handle v8_arraybuffer(v8_state vm, void *ptr, size_t byte_length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Context context = v8Context::New(isolate, vm->context);
    Context::Scope context_scope(context);
    if (ptr) {
        return CreateHandle(vm,
                ArrayBuffer::New(isolate, ptr, byte_length,
                    v8::ArrayBufferCreationMode::kInternalized));
    }
    return CreateHandle(vm,
            ArrayBuffer::New(isolate, byte_length));
}

size_t v8_bytelength(v8_state vm, v8_handle hab) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = GetValueFromHandle(vm, hab);
    size_t len = 0;
    if (v1.IsEmpty())
        return 0;
    if (v1->IsArrayBufferView()) {
        /* ArrayBufferView is implemented by all typed arrays and DataView */
        len = v8ArrayBufferView::Cast(v1)->ByteLength();
    } else if (v1->IsArrayBuffer()) {
        len = v8ArrayBuffer::Cast(v1)->ByteLength();
    } /* else
        len = 0; */
    return len;
}

size_t v8_byteoffset(v8_state vm, v8_handle habv) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = GetValueFromHandle(vm, habv);
    size_t off = 0;
    if (!v1.IsEmpty() && v1->IsArrayBufferView()) {
        off = v8ArrayBufferView::Cast(v1)->ByteOffset();
    } /* else
        off = 0; */
    return off;
}

v8_handle v8_getbuffer(v8_state vm, v8_handle h) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Value v1 = GetValueFromHandle(vm, h);
    if (v1.IsEmpty() || !v1->IsTypedArray()) {
        v8_set_errstr(vm, "v8_getbuffer: TypedArray argument expected");
        return 0;
    }
    v8Value ab = v8TypedArray::Cast(v1)->Buffer();
    return CreateHandle(vm, ab);
}

void *v8_externalize(v8_state vm, v8_handle h) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate);
    // Clear error.
    v8_set_errstr(vm, nullptr);
    v8Value v1 = GetValueFromHandle(vm, h);
    if (!v1.IsEmpty() && v1->IsArrayBuffer()) {
        return v8ArrayBuffer::Cast(v1)->Externalize().Data();
    }
    v8_set_errstr(vm,
            "v8_externalize: ArrayBuffer argument expected");
    return nullptr;
}

void v8_reset(js_vm *vm, v8_handle h) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    DeleteHandle(vm, h);
}

struct WeakPtr {
    v8_state vm;
    void *ptr;
    union {
        Fnfree free_func;
        /*v8_handle (*free_extwrap)(v8_state , int, v8_handle[]); -- FIXME */
        void (*free_dlwrap)(v8_state , int, void *argv);
    };
    Persistent<Value> handle;
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

// DLL function
static void WeakPtrCallback2(
        const v8::WeakCallbackInfo<WeakPtr> &data) {
    WeakPtr *w = data.GetParameter();
    Isolate *isolate = w->vm->isolate;
    HandleScope handle_scope(isolate);
    v8Value av[1];
    av[0] = v8Value::New(isolate, w->handle);
    w->free_dlwrap(w->vm, 1, reinterpret_cast<v8_val>(av));
    w->handle.Reset();
    isolate->AdjustAmountOfExternalAllocatedMemory(
                - static_cast<int64_t>(sizeof(WeakPtr)));

#ifdef V8TEST
    w->vm->weak_counter++;
#endif
    delete w;
}

int v8_dispose(v8_state vm, v8_handle h, Fnfree free_func) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    v8Object ptrObj = ToPtr(GetValueFromHandle(vm, h));
    if (!ptrObj.IsEmpty() && !IsCtypeWeak(ptrObj) && free_func) {
        vm->store_->Dispose(h);
        int oid = (V8EXTPTR<<2)|(1<<1);
        ptrObj->SetAlignedPointerInInternalField(0,
                reinterpret_cast<void*>(static_cast<uintptr_t>(oid)));
        WeakPtr *w = new WeakPtr;
        w->vm = vm;
        w->ptr = v8External::Cast(ptrObj->GetInternalField(1))->Value();
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

/*
 *  ptr.dispose() => use free() as the finalizer.
 *  ptr.dispose(finalizer_function).
 *  Returns the pointer: p1 = $malloc(..).dispose().
 */
void Dispose(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    v8Object obj = args.Holder();
    v8_state vm = reinterpret_cast<js_vm*>(isolate->GetData(0));
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "dispose: not a pointer");
    void *ptr = v8External::Cast(obj->GetInternalField(1))->Value();
    if (!ptr || IsCtypeWeak(obj)) {
        args.GetReturnValue().Set(obj);
        return;
    }

    WeakPtr *w = new WeakPtr;
    w->ptr = nullptr;
    w->vm = vm;
    if (argc == 0) {
        w->ptr = ptr;
        w->free_func = free;
        w->handle.Reset(isolate, obj);
        w->handle.SetWeak(w, WeakPtrCallback1, WeakCallbackType::kParameter);
    } else if (GetCtypeId(vm, args[0]) == V8EXTFUNC) {
        v8_ffn *func_item = reinterpret_cast<v8_ffn *>(
                v8External::Cast(
                        v8Object::Cast(args[0])->GetInternalField(1)
                    )->Value()
            );
        if ((func_item->flags & V8_DLFUNC) && func_item->pcount == 1) {
            w->ptr = ptr;
            w->free_dlwrap = (Fndlfnwrap) func_item->fp;
            w->handle.Reset(isolate, obj);
            w->handle.SetWeak(w, WeakPtrCallback2, WeakCallbackType::kParameter);
        }
    }
    if (!w->ptr) {
        delete w;
        ThrowError(isolate, "dispose: invalid argument");
    }
    int oid = (V8EXTPTR<<2)|(1<<1);
    obj->SetAlignedPointerInInternalField(0,
                reinterpret_cast<void*>(static_cast<uintptr_t>(oid)));
    w->handle.MarkIndependent();
    args.GetReturnValue().Set(obj);
}


} // extern "C"
