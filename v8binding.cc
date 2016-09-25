#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <dlfcn.h>

#include "v8.h"
#include "libplatform/libplatform.h"

#include "v8binding.h"
#include "vm.h"
#include "util.h"

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
  Local<Message> message = try_catch->Message();

  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    s.append(exception_string);
  } else {
    // Print (filename):(line number)
    String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
    Local<Context> context(isolate->GetCurrentContext());
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
#include "jsv8dlfn.h"

#ifdef V8TEST
#define DPRINT(format, args...) \
fprintf(stderr, format , ## args);
#else
#define DPRINT(format, args...) /* nothing */
#endif


extern js_coro *choose_coro(chan ch, int64_t ddline);
static js_handle *init_handle(js_vm *vm,
            js_handle *h, Local<Value> value);
static inline js_handle *make_handle(js_vm *vm,
            Local<Value> value, enum js_code type);
static void *ExternalPtrValue(js_vm *vm, Local <Value> v);
static void MakeCtypeProto(js_vm *vm);
static void WeakPtrCallback(
        const v8::WeakCallbackInfo<js_handle> &data);

static void js_set_errstr(js_vm *vm, const char *str) {
    if (vm->errstr) {
        free((void *) vm->errstr);
        vm->errstr = nullptr;
    }
    if (str)
        vm->errstr = estrdup(str, strlen(str));
}

const char *js_errstr(js_vm *vm) {
    return vm->errstr;
}

static void SetError(js_vm *vm, TryCatch *try_catch) {
    js_set_errstr(vm, GetExceptionString(vm->isolate, try_catch));
}

static void Panic(Isolate *isolate, TryCatch *try_catch) {
    char *errstr = GetExceptionString(isolate, try_catch);
    fprintf(stderr, "%s\n", errstr);
    exit(1);
}

static int GetCtypeId(js_vm *vm, Local<Value> v) {
    if (!v->IsObject())
        return 0;
    HandleScope handle_scope(vm->isolate);
    Local<Object> obj = Local<Object>::Cast(v);
    if (obj->InternalFieldCount() != 2)
        return 0;
    Local<Value> proto = obj->GetPrototype();

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

static int IsCtypeWeak(Local<Object> obj) {
    assert(obj->InternalFieldCount() == 2);
    int id = static_cast<int>(reinterpret_cast<uintptr_t>(
                obj->GetAlignedPointerFromInternalField(0)) >> 1);
    return (id & 1);
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
        Local<String> s;
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
    {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.Length() > 0)
        n = args[0]->IntegerValue(
                    isolate->GetCurrentContext()).FromJust();
    }
    mill_sleep(now()+n);
}

// $go(coro, inval, callback)
static void Go(const FunctionCallbackInfo<Value>& args) {
    js_coro *cr;
    Fncoro fn;
    js_vm *vm;

    {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    int argc = args.Length();
    ThrowNotEnoughArgs(isolate, argc < 3);
    vm = static_cast<js_vm*>(isolate->GetData(0));
    fn = (Fncoro) ExternalPtrValue(vm, args[0]);
    if (!fn)
        ThrowTypeError(isolate, "$go argument #1: coroutine expected");
    if (!args[2]->IsFunction())
        ThrowTypeError(isolate, "$go argument #3: function expected");
    cr = new js_coro;
    cr->coro = nullptr; /* XXX: not used? */
    cr->vm = vm;
    cr->callback = make_handle(vm, args[2], V8FUNC);
    cr->inval = make_handle(vm, args[1], V8UNKNOWN);
    cr->err = 0;
    cr->outval = nullptr;
    vm->ncoro++;
    }

    go(fn(vm, cr, cr->inval));
}

// callback(err, data)
static void RunCoroCallback(js_vm *vm, js_coro *cr) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    vm->ncoro--;
    TryCatch try_catch(isolate);

    // GetCurrentContext() is empty if called from WaitFor().
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Context::Scope context_scope(context);

    Local<Function> cb = Local<Function>::Cast(
                Local<Value>::New(isolate, cr->callback->handle));
    js_reset(cr->callback);

    assert(!cb.IsEmpty());
    Local<Value> args[2];
    assert(cr->outval);
    args[1] = Local<Value>::New(isolate, cr->outval->handle);
    args[0] = v8::Null(isolate);
    if (cr->err) {
        args[0] = args[1]->ToString(context).ToLocalChecked();
        args[1] = v8::Null(isolate);
    }
    js_reset(cr->inval);

    /* XXX: _must_ not be disposed in the C code.
     * Should be ref. counting persistent handles?? */
    if(cr->outval != cr->inval)
        js_reset(cr->outval);
    delete cr;

    assert(!try_catch.HasCaught());
    cb->Call(context->Global(), 2, args);
    if (try_catch.HasCaught()) {
        Panic(isolate, &try_catch);
    }
}

static void send_coro(js_vm *vm, js_coro *cr) {
    int rc = mill_pipesend(vm->inq, (void *) &cr);
    if (rc == -1) {
        char serr[] = "$send: send to a closed pipe";
        cr->outval = js_string(vm, serr, strlen(serr));
        cr->err = 1;
        cr->coro = nullptr;
        int rc = mill_chs(cr->vm->ch, &cr);
        assert(rc == 0);
    }
}

// $send(coro, inval, callback)
static void Send(const FunctionCallbackInfo<Value>& args) {
    js_vm *vm;
    js_coro *cr;

    {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    int argc = args.Length();
    ThrowNotEnoughArgs(isolate, argc < 3);
    vm = static_cast<js_vm*>(isolate->GetData(0));

    void *fn = ExternalPtrValue(vm, args[0]);
    if (!fn)
        ThrowTypeError(isolate, "$send argument #1: coroutine expected");
    if (!args[2]->IsFunction())
        ThrowTypeError(isolate, "$send argument #3: function expected");
    cr = new js_coro;
    cr->vm = vm;
    cr->coro = fn;
    cr->callback = make_handle(vm, args[2], V8FUNC);
    cr->inval = make_handle(vm, args[1], V8UNKNOWN);
    cr->err = 0;
    cr->outval = nullptr;
    vm->ncoro++;
    }
    send_coro(vm, cr);
}

static void Close(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
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
    Local<Context> context = isolate->GetCurrentContext();
    TryCatch try_catch(isolate);
    Local<String> source = Local<String>::Cast(args[0]);
    Local<String> name;
    if (!args[1]->ToString(context).ToLocal(&name)) {
        Panic(isolate, &try_catch);    
    }

    ScriptOrigin origin(name);
    Local<Script> script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        Panic(isolate, &try_catch);
    }
    Local<Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        Panic(isolate, &try_catch);
    }
    assert(!result.IsEmpty());
    args.GetReturnValue().Set(result);
}

// FIXME use this in js_pointer()
static Local<Object> WrapPtr(js_vm *vm, void *ptr) {
    Isolate *isolate = vm->isolate;
    assert(Locker::IsLocked(isolate));
    // Local scope for temporary handles.
    EscapableHandleScope handle_scope(isolate);
    Local<ObjectTemplate> templ =
        Local<ObjectTemplate>::New(isolate, vm->extptr_template);
    Local<Object> obj = templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked();
    int oid = (V8EXTPTR<<2);
    obj->SetAlignedPointerInInternalField(0,
             reinterpret_cast<void*>(static_cast<uintptr_t>(oid)));
    obj->SetInternalField(1, External::New(isolate, ptr));
    obj->SetPrototype(Local<Value>::New(isolate, vm->cptr_proto));
    return handle_scope.Escape(obj);
}

// malloc(size [, zerofill])
// TODO: adjust GC allocation amount.
static void Malloc(const v8::FunctionCallbackInfo<v8::Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    ThrowNotEnoughArgs(isolate, argc < 1);
    Local<Context> context = isolate->GetCurrentContext();
    int size = args[0]->Int32Value(context).FromJust();
    if (size <= 0)
        ThrowError(isolate, "$malloc: invalid size argument");
    void *ptr = emalloc(size);
    if (argc > 1 && args[1]->BooleanValue(context).FromJust())
        memset(ptr, '\0', size);
    Local<Object> obj = WrapPtr(
            static_cast<js_vm*>(isolate->GetData(0)), ptr);
    args.GetReturnValue().Set(obj);
}

static void CallForeignFunc(
        const v8::FunctionCallbackInfo<v8::Value>& args) {

    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));

    Local<Object> obj = args.Holder();
    assert(obj->InternalFieldCount() == 2);
    cffn_s *func_wrap = static_cast<cffn_s *>(
                Local<External>::Cast(obj->GetInternalField(1))->Value());
    int argc = args.Length();
    if (argc > MAXARGS || argc != func_wrap->pcount)
        ThrowError(isolate, "C-function called with incorrect # of arguments");

    if (func_wrap->isdlfunc) {
        assert(vm->dlstr_idx == 0);
        Local<Value> argv[MAXARGS+1];
        // N.B.: argv[0] is for the return value
        for (int i = 0; i < argc; i++)
            argv[i + 1] = args[i];
        argv[0] = v8::Undefined(isolate);   // default return type is void

        js_set_errstr(vm, nullptr);
        ((Fndlfnwrap) func_wrap->fp)(vm, argc, reinterpret_cast<js_val>(argv));

        while (vm->dlstr_idx > 0) {
            free(vm->dlstr[--vm->dlstr_idx]);
        }
        if (vm->errstr)
            ThrowError(isolate, vm->errstr);
        args.GetReturnValue().Set(argv[0]);
    } else {
        for (int i = 0; i < argc; i++)
            (void) init_handle(vm, vm->args[i], args[i]);
        int err = 0;
        js_handle *hret = ((Fnfnwrap) func_wrap->fp)(vm, argc, vm->args);
        Local<Value> retv = Local<Value>();
        if (!hret) {
            retv = v8::Undefined(isolate);
        } else {
            assert(!(hret->flags & ARG_HANDLE)); // XXX: should bail out.
            retv = Local<Value>::New(isolate, hret->handle);
            if (hret->type == V8ERROR)  // From js_error().
                err = 1;
            js_reset(hret);
        }
        for (int i = 0; i < argc; i++) {
            js_handle *h = vm->args[i];
            assert(! (h->flags & PERM_HANDLE));
            h->handle.Reset();
            if (h->flags & STR_HANDLE)
                free(h->stp);
            h->flags = ARG_HANDLE;
        }
        if (err)
            isolate->ThrowException(retv);
        else
            args.GetReturnValue().Set(retv);
    }
}

static void WaitFor(js_vm *vm) {
    while (vm->ncoro > 0) {
        DPRINT("[[ WaitFor: %d unfinished coroutines. ]]\n", vm->ncoro);
        js_coro *cr = chr(vm->ch, js_coro *);
        assert(cr);
        RunCoroCallback(vm, cr);
    }
}

static void v8Microtask(void *data) {
    js_vm *vm = (js_vm *) data;
    int64_t ddline = now() + 1;
    int k = 0;
    while (vm->ncoro > 0) {
        js_coro *cr = choose_coro(vm->ch, ddline);
        if (cr) {
            k++;
            RunCoroCallback(vm, cr);
        } else
            break;
    }
    if (k > 0)
        DPRINT("[[ v8Microtask: processed %d coroutines. ]]\n", k);
    if (vm->ncoro > 0)
        yield();
}

// Callback triggered after microtasks are run. Callback will trigger even
// if microtasks were attempted to run, but the microtasks queue was empty
// and no single microtask was actually executed.

static void v8MicrotasksCompleted(Isolate *isolate) {
    // Reinstall the Microtask.
    isolate->EnqueueMicrotask(v8Microtask, isolate->GetData(0));
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

// Start a $send() coroutine in the main thread.
coroutine static void start_coro(mill_pipe p) {
    while (true) {
        int done;
        js_coro *cr = *((js_coro **) mill_piperecv(p, &done));
        if (done)
            break;
        assert(cr->coro);
        Fncoro co = (Fncoro) cr->coro;
        go(co(cr->vm, cr, cr->inval));
    }
}

// Invoked by the main thread.
js_vm *js8_vmnew(mill_worker w) {
    js_vm *vm = new js_vm;
    vm->w = w;
    vm->inq = mill_pipemake(sizeof (void *));
    assert(vm->inq);
    vm->outq = mill_pipemake(sizeof (void *));
    assert(vm->outq);
    vm->ch = mill_chmake(sizeof (js_coro *), 5); // XXX: How to select a bufsize??
    assert(vm->ch);
    vm->ncoro = 0;
    vm->errstr = nullptr;
    go(start_coro(vm->inq));
    return vm;
}

static void GlobalGet(Local<Name> name,
        const PropertyCallbackInfo<Value>& info) {
    Isolate *isolate = info.GetIsolate();
    HandleScope handle_scope(isolate);
    String::Utf8Value str(name);
    if (strcmp(*str, "$errno") == 0)
        info.GetReturnValue().Set(Integer::New(isolate, errno));
}

static void GlobalSet(Local<Name> name, Local<Value> val,
        const PropertyCallbackInfo<void>& info) {
    Isolate *isolate = info.GetIsolate();
    HandleScope handle_scope(isolate);
    String::Utf8Value str(name);
    if (strcmp(*str, "$errno") == 0)
        errno = val->ToInt32(
                isolate->GetCurrentContext()).ToLocalChecked()->Value();
}

// Receive coroutine with result from the main thread.
coroutine static void recv_coro(js_vm *vm) {
    while (true) {
        int done;
        js_coro *cr = *((js_coro **) mill_piperecv(vm->outq, &done));
        if (done) {
            mill_pipefree(vm->outq);
            break;
        }
        /* Send to the channel to be processed by the V8 microtask. */
        int rc = mill_chs(vm->ch, &cr);
        assert(rc == 0);
    }
}

// Invoked by the main thread.
void js8_vmclose(js_vm *vm) {
    //
    // vm->inq already closed (See js_vmclose()).
    // Close vm->outq; This causes the receiving coroutine (recv_coro)
    // to exit the loop and free vm->outq.
    //
    Isolate *isolate = vm->isolate;

    {
    Locker locker(isolate);

    mill_pipeclose(vm->outq);
    mill_pipefree(vm->inq);

    if (vm->errstr)
        free(vm->errstr);
    delete [] vm->args[0];
    }
    isolate->Dispose();
    delete vm;
}

const js_handle *js_global(js_vm *vm) {
    return vm->global_handle;
}

const js_handle *js_null(js_vm *vm) {
    return vm->null_handle;
}

static js_handle *init_handle(js_vm *vm,
            js_handle *h, Local<Value> value) {
    if (value->IsFunction())
        h->type = V8FUNC;
    else if (value->IsString())
        h->type = V8STRING;
    else if (value->IsNumber())
        h->type = V8NUMBER;
    else if (value->IsArray())
        h->type = V8ARRAY;
    else if (value->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(value);
        assert(obj == value);
        int id = GetCtypeId(vm, obj);
        if (id > 0) {
            if (IsCtypeWeak(obj)) {
                /* Bailing out for now. */
                fprintf(stderr, "error: (weak) ctype object cannot be exported\n");
                exit(1);
            }
            h->type = (enum js_code) id;
        } else
            h->type = V8OBJECT;
    } else if (value->IsNull())
        h->type = V8NULL;
    else if (value->IsUndefined())
        h->type = V8UNDEFINED;
    else
        h->type = V8UNKNOWN;
    h->handle.Reset(vm->isolate, value);
    return h;
}

static js_handle *make_handle(js_vm *vm,
            Local<Value> value, enum js_code type) {
    js_handle *h;
    if (value->IsNull())
        return vm->null_handle;
    if (value->IsUndefined())
        return vm->undef_handle;
    h = new js_handle_s;
    vm->isolate->AdjustAmountOfExternalAllocatedMemory(
                static_cast<int64_t>(sizeof(js_handle)));
    h->flags = 0;
    h->vm = vm;
    if (type) {
        h->type = type;
        h->handle.Reset(vm->isolate, value);
        return h;
    }
    return init_handle(vm, h, value);
}

int js_isnumber(js_handle *h) {
    return (h->type == V8NUMBER);
}

int js_isfunction(js_handle *h) {
    return (h->type == V8FUNC);
}

int js_isobject(js_handle *h) {
    if (h->type == V8OBJECT)
        return 1;
    Isolate *isolate = h->vm->isolate;
    LOCK_SCOPE(isolate);
    Local<Value> v1 = Local<Value>::New(isolate, h->handle);
    return !!v1->IsObject();
}

int js_isarray(js_handle *h) {
    return (h->type == V8ARRAY);
}

int js_ispointer(js_handle *h) {
    return (h->type == V8EXTPTR);
}

int js_isstring(js_handle *h) {
    return (h->type == V8STRING);
}

int js_isnull(js_handle *h) {
    return (h->type == V8NULL);
}

int js_isundefined(js_handle *h) {
    return (h->type == V8UNDEFINED);
}

static js_handle *CompileRun(js_vm *vm, const char *src) {
    Isolate *isolate = vm->isolate;

    LOCK_SCOPE(isolate)

    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Context::Scope context_scope(context);

    TryCatch try_catch(isolate);
    const char *script_name = "<string>";   // TODO: optional argument
    Local<String> name(String::NewFromUtf8(isolate, script_name,
                NewStringType::kNormal).ToLocalChecked());
    Local<String> source(String::NewFromUtf8(isolate, src,
                NewStringType::kNormal).ToLocalChecked());

    ScriptOrigin origin(name);
    Local<Script> script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        SetError(vm, &try_catch);
        return NULL;
    }

    Handle<Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        SetError(vm, &try_catch);
        return NULL;
    }
    assert(!result.IsEmpty());
    return make_handle(vm, result, V8UNKNOWN);
}

js_handle *js_string(js_vm *vm, const char *stp, int length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    assert(stp);
    if (length < 0)
        length = strlen(stp);
    return make_handle(vm, String::NewFromUtf8(isolate, stp,
                            v8::String::kNormalString, length),
                V8STRING);
}

js_handle *js_number(js_vm *vm, double d) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    js_handle *h = make_handle(vm, Number::New(isolate, d), V8NUMBER);
    h->flags |= DBL_HANDLE;
    h->d = d;
    return h;
}

js_handle *js_int32(js_vm *vm, int32_t i) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    js_handle *h = make_handle(vm, Integer::New(isolate, i), V8NUMBER);
    h->flags |= INT32_HANDLE;
    h->i = i;
    return h;
}

js_handle *js_object(js_vm *vm) {
    Isolate *isolate = vm->isolate;
    js_handle *h;
    {
        LOCK_SCOPE(isolate)
        /* XXX: need a context in this case unlike String or Number!!! */
        Local<Context> context = Local<Context>::New(isolate, vm->context);
        Context::Scope context_scope(context);
        h = make_handle(vm, Object::New(isolate), V8OBJECT);
    } // Locker Destructor called here (locker unlocked)
    return h;
}

js_handle *js_get(js_handle *hobj, const char *key) {
    Isolate *isolate = hobj->vm->isolate;
    LOCK_SCOPE(isolate)
    js_vm *vm = hobj->vm;
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    // Context::Scope context_scope(context);
    Local<Value> v1 = Local<Value>::New(isolate, hobj->handle);
    if (!v1->IsObject()) {
        js_set_errstr(vm, "js_get: object argument expected");
        return NULL;
    }
    Local<Object> obj = Local<Object>::Cast(v1);
    /* undefined for non-existent key */
    Local<Value> v2 = obj->Get(context,
                String::NewFromUtf8(isolate, key)).ToLocalChecked();
    return make_handle(vm, v2, V8UNKNOWN);
}

int js_set(js_handle *hobj, const char *key, js_handle *hval) {
    Isolate *isolate = hobj->vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Context> context = Local<Context>::New(isolate, hobj->vm->context);
    // Context::Scope context_scope(context);
    Local<Value> v1 =  Local<Value>::New(isolate, hobj->handle);
    if (!v1->IsObject()) {
        js_set_errstr(hobj->vm, "js_set: object argument expected");
        return 0;
    }
    Local<Object> obj = Local<Object>::Cast(v1);
    return obj->Set(context, String::NewFromUtf8(isolate, key),
                Local<Value>::New(isolate, hval->handle)).FromJust();
}

int js_set_string(js_handle *hobj,
            const char *name, const char *val) {
    Isolate *isolate = hobj->vm->isolate;
    LOCK_SCOPE(isolate)
    // Local<Context> context = Local<Context>::New(isolate, hobj->vm->context);
    Local<Value> v1 = Local<Value>::New(isolate, hobj->handle);
    if (!v1->IsObject()) {
        js_set_errstr(hobj->vm, "js_set_string: object argument expected");
        return 0;
    }
    Local<Object> obj = Local<Object>::Cast(v1);
    return obj->Set(String::NewFromUtf8(isolate, name),
                        String::NewFromUtf8(isolate, val));
}

js_handle *js_array(js_vm *vm, int length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Context::Scope context_scope(context);
    return make_handle(vm, Array::New(isolate, length), V8ARRAY);
}

js_handle *js_geti(js_handle *hobj, unsigned index) {
    Isolate *isolate = hobj->vm->isolate;
    LOCK_SCOPE(isolate)
    js_vm *vm = hobj->vm;
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    // Context::Scope context_scope(context);
    Local<Value> v1 = Local<Value>::New(isolate, hobj->handle);
    if (!v1->IsObject()) {
        js_set_errstr(vm, "js_geti: object argument expected");
        return NULL;
    }
    Local<Object> obj = Local<Object>::Cast(v1);
    /* undefined for non-existent index */
    Local<Value> v2 = obj->Get(context, index).ToLocalChecked();
    return make_handle(vm, v2, V8UNKNOWN);
}

int js_seti(js_handle *hobj, unsigned index, js_handle *hval) {
    Isolate *isolate = hobj->vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Context> context = Local<Context>::New(isolate, hobj->vm->context);
    // Context::Scope context_scope(context);
    Local<Value> v1 = Local<Value>::New(isolate, hobj->handle);
    if (!v1->IsObject()) {
        js_set_errstr(hobj->vm, "js_seti: object argument expected");
        return 0;
    }
    Local<Object> obj = Local<Object>::Cast(v1);
    return obj->Set(context, index,
                Local<Value>::New(isolate, hval->handle)).FromJust();
}

/* N.B.: V8 owns the Buffer memory. If ptr is not NULL, it must be
 * compatible with ArrayBuffer::Allocator::Free. */
js_handle *js_arraybuffer(js_vm *vm,
        void *ptr, size_t byte_length) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Context::Scope context_scope(context);
    if (ptr) {
        return make_handle(vm,
                ArrayBuffer::New(isolate, ptr, byte_length,
                    v8::ArrayBufferCreationMode::kInternalized), V8OBJECT);
    }
    return make_handle(vm,
            ArrayBuffer::New(isolate, byte_length), V8OBJECT);
}

size_t js_bytelength(js_handle *hab) {
    Isolate *isolate = hab->vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Value> v1 = Local<Value>::New(isolate, hab->handle);
    size_t len = 0;
    if (v1->IsArrayBufferView()) {
        /* ArrayBufferView is implemented by all typed arrays and DataView */
        len = Local<ArrayBufferView>::Cast(v1)->ByteLength();
    } else if (v1->IsArrayBuffer()) {
        len = Local<ArrayBuffer>::Cast(v1)->ByteLength();
    } /* else
        len = 0; */
    return len;
}

size_t js_byteoffset(js_handle *habv) {
    Isolate *isolate = habv->vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Value> v1 = Local<Value>::New(isolate, habv->handle);
    size_t off = 0;
    if (v1->IsArrayBufferView()) {
        /* ArrayBufferView is implemented by all typed arrays and DataView */
        off = Local<ArrayBufferView>::Cast(v1)->ByteOffset();
    } /* else
        off = 0; */
    return off;
}

js_handle *js_getbuffer(js_handle *habv) {
    js_vm *vm = habv->vm;
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Value> v1 = Local<Value>::New(isolate, habv->handle);
    if (!v1->IsTypedArray()) {
        js_set_errstr(vm, "js_getbuffer: ArrayBufferView argument expected");
        return nullptr;
    }
    Local<Value> ab = Local<TypedArray>::Cast(v1)->Buffer();
    return make_handle(vm, ab, V8OBJECT);
}

void *js_externalize(js_handle *h) {
    js_vm *vm = h->vm;
    Isolate *isolate = vm->isolate;
    void *ptr;
    LOCK_SCOPE(isolate);
    // Clear error.
    js_set_errstr(vm, nullptr);
    Local<Value> v1 = Local<Value>::New(isolate, h->handle);
    if (v1->IsArrayBuffer()) {
        ptr = Local<ArrayBuffer>::Cast(v1)->Externalize().Data();
    } else {
        js_set_errstr(vm,
            "js_externalize: ArrayBuffer argument expected");
        ptr = nullptr;
    }
    return ptr;
}

// JS exception object.
js_handle *js_error(js_vm *vm, const char *message) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    return make_handle(vm, Exception::Error(
                    String::NewFromUtf8(isolate, message)), V8ERROR);
}

js_handle *js_pointer(js_vm *vm, void *ptr) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate);
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Context::Scope context_scope(context);
    if (!ptr)
        return vm->nullptr_handle;
    Local<ObjectTemplate> templ =
            Local<ObjectTemplate>::New(isolate, vm->extptr_template);
    Local<Object> obj = templ->NewInstance(context).ToLocalChecked();
    void *ptr1 = reinterpret_cast<void*>(static_cast<uintptr_t>(V8EXTPTR<<2));
    obj->SetAlignedPointerInInternalField(0, ptr1);
    obj->SetInternalField(1, External::New(isolate, ptr));
    obj->SetPrototype(Local<Value>::New(isolate, vm->ctype_proto));
    js_handle *h = make_handle(vm, obj, V8EXTPTR);
    h->ptr = ptr;
    h->flags |= PTR_HANDLE;
    h->free_func = (Fnfree) nullptr;
    return h;
}

// XXX: returns NULL if not V8EXTPTR!
static void *ExternalPtrValue(js_vm *vm, Local <Value> v) {
    if (!v->IsObject())
        return nullptr;
    Local <Object> obj = Local<Object>::Cast(v);
    if (GetCtypeId(vm, obj) == V8EXTPTR)
        return Local<External>::Cast(obj->GetInternalField(1))->Value();
    return nullptr;
}

js_handle *js_cfunc(js_vm *vm, const struct cffn_s *func_wrap) {
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate);
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Context::Scope context_scope(context);
    ((struct cffn_s *) func_wrap)->isdlfunc = 0;
    Local<ObjectTemplate> templ =
        Local<ObjectTemplate>::New(isolate, vm->extfunc_template);
    Local<Object> fnObj = templ->NewInstance(context).ToLocalChecked();
    fnObj->SetAlignedPointerInInternalField(0,
             reinterpret_cast<void*>(static_cast<uintptr_t>(V8EXTFUNC<<2)));
    fnObj->SetInternalField(1, External::New(isolate, (void *)func_wrap));
    fnObj->SetPrototype(Local<Value>::New(isolate, vm->ctype_proto));
    return make_handle(vm, fnObj, V8EXTFUNC);
}

// Send from C coroutine to V8
void js_send(js_coro *cr, js_handle *oh, int err) {
    if (!oh)
        oh = cr->vm->null_handle;
    cr->outval = oh;
    cr->err = err;
    if (!cr->coro) {
        // Coroutine is in the V8 thread.
        int rc = mill_chs(cr->vm->ch, &cr);
        assert(rc == 0);
    } else {
        // Coroutine is running in the main thread.
        int rc = mill_pipesend(cr->vm->outq, &cr);
        assert(rc == 0);
    }
}

const char *js_tostring(js_handle *h) {
    js_vm *vm = h->vm;
    Isolate *isolate = vm->isolate;
    Locker locker(isolate);
    if (h->type == V8STRING && (h->flags & STR_HANDLE) != 0) {
        return h->stp;
    }
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    if ((h->flags & STR_HANDLE) != 0)
        free(h->stp);
    h->flags &= ~VALUE_MASK;
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Local<String> s = Local<Value>::New(isolate, h->handle)
                    -> ToString(context).ToLocalChecked();
    String::Utf8Value stval(s);
#if 0
    if (!*stval)    // conversion error
        return NULL;
#endif
    /* return empty string if there was an error during conversion. */
    h->stp = estrdup(*stval, stval.length());
    h->flags |= STR_HANDLE;
    return h->stp;
}

double js_tonumber(js_handle *h) {
    Isolate *isolate = h->vm->isolate;
    Locker locker(isolate);
    if (h->type == V8NUMBER && (h->flags & DBL_HANDLE) != 0) {
        return h->d;
    }
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Local<Context>::New(isolate, h->vm->context);
    double d = Local<Value>::New(isolate, h->handle)
                    -> NumberValue(context).FromJust();
    if (h->type == V8NUMBER) {
        if (h->flags & STR_HANDLE)
            free(h->stp);
        h->flags &= ~VALUE_MASK;
        h->flags |= DBL_HANDLE;
        h->d = d;
    }
    return d;
}

int32_t js_toint32(js_handle *h) {
    Isolate *isolate = h->vm->isolate;
    Locker locker(isolate);
    if (h->type == V8NUMBER && (h->flags & INT32_HANDLE) != 0) {
        return h->i;
    }
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Local<Context>::New(isolate, h->vm->context);
    int32_t i = Local<Value>::New(isolate, h->handle)
                        -> Int32Value(context).FromJust();
    if (h->type == V8NUMBER) {
        if (h->flags & STR_HANDLE)
            free(h->stp);
        h->flags &= ~VALUE_MASK;
        h->flags |= INT32_HANDLE;
        h->i = i;
    }
    return i;
}

void *js_topointer(js_handle *h) {
    js_vm *vm = h->vm;
    Isolate *isolate = vm->isolate;
    Locker locker(isolate);
    // Clear error
    js_set_errstr(vm, nullptr);
    if (h->type == V8EXTPTR && (h->flags & PTR_HANDLE))
        return h->ptr;

    void *ptr;
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    Local<Value> v1 = Local<Value>::New(isolate, h->handle);
    if (h->type == V8EXTPTR) {
        ptr = Local<External>::Cast(
                    Local<Object>::Cast(v1)->GetInternalField(1)
                )->Value();
        h->flags |= PTR_HANDLE;
    } else if (v1->IsArrayBuffer()) {
        // The pointer will be invalid if the ArrayBuffer gets
        // garbage collected!.
        ptr = Local<ArrayBuffer>::Cast(v1)->GetContents().Data();
    } else {
        js_set_errstr(vm, "js_topointer: pointer argument expected");
        ptr = nullptr;
    }
    return ptr;
}

void js_reset(js_handle *h) {
    Isolate *isolate = h->vm->isolate;
    Locker locker(isolate);
    if ((h->flags & (PERM_HANDLE|ARG_HANDLE|WEAK_HANDLE)) == 0) {
        Isolate::Scope isolate_scope(isolate); // more than one isolate in worker ???
        h->handle.Reset();
        if (h->flags & STR_HANDLE)
            free(h->stp);
        delete h;
        isolate->AdjustAmountOfExternalAllocatedMemory(
                - static_cast<int64_t>(sizeof(js_handle)));
    }
}

static void WeakPtrCallback(
        const v8::WeakCallbackInfo<js_handle> &data) {
    js_handle *h = data.GetParameter();
    Isolate *isolate = h->vm->isolate;
    if (h->flags & FREE_WRAP) {
        js_handle *ah[1] = {h};
        js_handle *hret = h->free_wrap(h->vm, 1, ah);
        if (hret) // should be NULL
            js_reset(hret);
    } else if (h->free_func)
        h->free_func(h->ptr);
    h->handle.Reset();
    delete h;
    isolate->AdjustAmountOfExternalAllocatedMemory(
                - static_cast<int64_t>(sizeof(js_handle)));
}

void js_dispose(js_handle *h, Fnfree free_func) {
    Isolate *isolate = h->vm->isolate;
    Locker locker(isolate);
    if (free_func && h->type == V8EXTPTR
            && (h->flags & (PERM_HANDLE|ARG_HANDLE|WEAK_HANDLE)) == 0
    ) {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        Local<Object> obj = Local<Object>::Cast(
                    Local<Value>::New(isolate, h->handle));
        int oid = (V8EXTPTR<<2)|(1<<1);
        obj->SetAlignedPointerInInternalField(0,
                reinterpret_cast<void*>(static_cast<uintptr_t>(oid)));
        assert(IsCtypeWeak(obj));
        h->free_func = free_func;
        h->handle.SetWeak(h, WeakPtrCallback, WeakCallbackType::kParameter);
        h->handle.MarkIndependent();
        h->flags |= WEAK_HANDLE;
    }
}

static js_handle *CallFunc(struct js8_arg_s *args) {
    js_vm *vm = args->vm;
    Isolate *isolate = vm->isolate;
    LOCK_SCOPE(isolate)
    Local<Context> context = Local<Context>::New(isolate, vm->context);
    Context::Scope context_scope(context);

    Local<Value> v1;
    if (args->type == V8CALLSTR) {
        // function expression
        assert(args->source);
        js_handle *result = CompileRun(vm, args->source);
        if (!result)
            return NULL;
        v1 = Local<Value>::New(isolate, result->handle);
        js_reset(result);
    } else
        v1 = Local<Value>::New(isolate, args->h1->handle);

    if (!v1->IsFunction()) {
        js_set_errstr(vm, "js_call: function argument #1 expected");
        return NULL;
    }
    Local<Function> func = Local<Function>::Cast(v1);

    int argc = args->nargs;
    assert(argc <= 4);

    TryCatch try_catch(isolate);
    Local<Object> self = context->Global();
    if (args->h)
        self = Local<Value>::New(isolate, args->h->handle)
                -> ToObject(context).ToLocalChecked();

    Local<Value> argv[4];
    int i;
    for (i = 0; i < argc; i++) {
        argv[i] = Local<Value>::New(isolate, args->a[i]->handle);
    }

    Local<Value> result = func->Call(self, argc, argv);
    if (try_catch.HasCaught()) {
        SetError(vm, &try_catch);
        return NULL;
    }
    return make_handle(vm, result, V8UNKNOWN);
}

int js8_do(struct js8_arg_s *args) {
    switch (args->type) {
    case V8COMPILERUN:
        assert(args->vm->ncoro == 0);
        args->h = CompileRun(args->vm, args->source);
        WaitFor(args->vm);
        break;
    case V8CALL:
    case V8CALLSTR:
        args->h = CallFunc(args);
        WaitFor(args->vm);
        break;
    case V8GC:
#ifdef V8TEST
    {
        Isolate *isolate = args->vm->isolate;
        Locker locker(isolate);
        Isolate::Scope isolate_scope(isolate);
        isolate->RequestGarbageCollectionForTesting(
                            Isolate::kFullGarbageCollection);
    }
#endif
        break;
    default:
        fprintf(stderr, "error: js8_do(): received unexpected code");
        exit(1);
    }
    return 1;
}

///////////////////////// C-type proto /////////////////////////

// C pointer and function objects method
static void Ctype(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    Local<Object> obj = args.This();
    assert(args.Holder() == args.This()); // unlike in accessor callback, true here!
    int id = GetCtypeId(vm, obj);
    if (id == V8EXTPTR)
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, "C-pointer"));
    else if (id == V8EXTFUNC)
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, "C-function"));
}

/*
 *  ptr.dispose() => use free() as the finalizer.
 *  ptr.dispose(finalizer_function).
 */
static void Dispose(const FunctionCallbackInfo<Value>& args) {
    int argc = args.Length();
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    Local <Object> obj = args.Holder();
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "dispose: not a pointer");
    void *ptr = Local<External>::Cast(obj->GetInternalField(1))->Value();
    if (!ptr || IsCtypeWeak(obj))
        return;

    js_handle *h = make_handle(vm, obj, V8EXTPTR);
    h->ptr = ptr;
    h->flags |= PTR_HANDLE;
    if (argc == 0) {
        h->free_func = free;
        h->flags |= WEAK_HANDLE;
    } else if (GetCtypeId(vm, args[0]) == V8EXTFUNC) {
        js_ffn *func_wrap = static_cast<js_ffn *>(
                Local<External>::Cast(
                        Local<Object>::Cast(args[0])->GetInternalField(1)
                    )->Value()
            );
        if (!func_wrap->isdlfunc && func_wrap->pcount == 1) {
            h->free_wrap = (Fnfnwrap) func_wrap->fp;
            h->flags |= (FREE_WRAP|WEAK_HANDLE);
        }
    }
    if (!(h->flags & WEAK_HANDLE)) {
        js_reset(h);
        ThrowError(isolate, "dispose: invalid argument");
    }
    int oid = (V8EXTPTR<<2)|(1<<1);
    obj->SetAlignedPointerInInternalField(0,
                reinterpret_cast<void*>(static_cast<uintptr_t>(oid)));
    h->handle.SetWeak(h, WeakPtrCallback, WeakCallbackType::kParameter);
    h->handle.MarkIndependent();
}

static void Free(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    Local <Object> obj = args.Holder();
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "free: not a pointer");
    void *ptr = Local<External>::Cast(obj->GetInternalField(1))->Value();
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
    Local<Object> obj = args.Holder();
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "notNull: not a pointer");
    void *ptr = Local<External>::Cast(obj->GetInternalField(1))->Value();
    if (!ptr)
        ThrowError(isolate, "notNull: pointer is null");
    // return the pointer to facilitate chaining:
    //  ptr = foo().notNull();
    args.GetReturnValue().Set(obj);
}

// ptr.utf8String(length = -1)
static void Utf8String(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));
    Local<Object> obj = args.Holder();
    if (GetCtypeId(vm, obj) != V8EXTPTR)
        ThrowTypeError(isolate, "utf8String: not a pointer");
    void *ptr = Local<External>::Cast(obj->GetInternalField(1))->Value();
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

// Construct the prototype object for C pointers and functions.
static void MakeCtypeProto(js_vm *vm) {
    Isolate *isolate = vm->isolate;
    HandleScope handle_scope(isolate);
    Local<ObjectTemplate> cp_templ = ObjectTemplate::New(isolate);
    cp_templ->Set(String::NewFromUtf8(isolate, "ctype"),
                FunctionTemplate::New(isolate, Ctype));
    vm->ctype_proto.Reset(isolate, cp_templ->NewInstance(
                    isolate->GetCurrentContext()).ToLocalChecked());

    // Create the proto object for the C-type pointer.
    //  cptr_object.__proto__ = ptr_proto;
    //  ptr_proto.__proto__ = vm->ctype_proto;
    Local<ObjectTemplate> ptr_templ = ObjectTemplate::New(isolate);
    ptr_templ->Set(String::NewFromUtf8(isolate, "dispose"),
                FunctionTemplate::New(isolate, Dispose));
    ptr_templ->Set(String::NewFromUtf8(isolate, "free"),
                FunctionTemplate::New(isolate, Free));
    ptr_templ->Set(String::NewFromUtf8(isolate, "notNull"),
                FunctionTemplate::New(isolate, NotNull));
    ptr_templ->Set(String::NewFromUtf8(isolate, "utf8String"),
                FunctionTemplate::New(isolate, Utf8String));

    // Create the one and only proto instance.
    Local<Object> ptr_proto = ptr_templ
                -> NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    // Setup the proto chain.
    ptr_proto->SetPrototype(Local<Value>::New(isolate, vm->ctype_proto));
    // Make the proto object persistent.
    vm->cptr_proto.Reset(isolate, ptr_proto);
}

////////////////////////////// DLL ///////////////////////////
#define ARGV reinterpret_cast<Local<Value> *>(argv)[arg_num]
#define ISOLATE(vm)   (vm->isolate)
#define CURR_CONTEXT(vm)   ISOLATE(vm)->GetCurrentContext()
#define CHECK_NUMBER(vm, v) \
    Local<Value> v = ARGV; \
    if (! v->IsNumber() && ! v->IsBoolean()) {\
        js_set_errstr(vm, "C-type argument is not a number");\
        return 0;\
    }

// N.B.: unlike js_toint32 etc.  there is no coercion in
// any of these conversion routines.
static int to_int(js_vm *vm, int arg_num, js_val argv) {
    CHECK_NUMBER(vm, v)
    return v->Int32Value(CURR_CONTEXT(vm)).FromJust();
}

static unsigned int to_uint(js_vm *vm, int arg_num, js_val argv) {
    CHECK_NUMBER(vm, v)
    return v->Uint32Value(CURR_CONTEXT(vm)).FromJust();
}

static double to_double(js_vm *vm, int arg_num, js_val argv) {
    CHECK_NUMBER(vm, v)
    return v->NumberValue(CURR_CONTEXT(vm)).FromJust();
}

static char *to_string(js_vm *vm, int arg_num, js_val argv) {
    Local<Value> v = ARGV;
    if (!v->IsString() /*&& !v->IsStringObject()*/) {
        js_set_errstr(vm, "C-type argument is not a string");
        return nullptr;
    }
    Local<String> s = v->ToString(CURR_CONTEXT(vm)).ToLocalChecked();
    int utf8len = s->Utf8Length();	/* >= 0 */
    char *p = (char *) emalloc(utf8len + 1);
    int l = s->WriteUtf8(p, utf8len);
    p[l] = '\0';
    /* Keep track of memory so can free upon return; See CallDllFunc() */
    vm->dlstr[vm->dlstr_idx++] = p;
    return p;
}

static void *to_pointer(js_vm *vm, int arg_num, js_val argv) {
    Local<Value> v = ARGV;
    if (GetCtypeId(vm, v) != V8EXTPTR) {
        js_set_errstr(vm, "C-type argument is not a pointer");
        return nullptr;
    }
    return Local<External>::Cast(
                Local<Object>::Cast(v)->GetInternalField(1))->Value();
}

#define RETVAL reinterpret_cast<Local<Value> *>(argv)[0]

static void from_int(js_vm *vm, int i, js_val argv) {
    RETVAL = Integer::New(ISOLATE(vm), i);
}

static void from_uint(js_vm *vm, unsigned ui, js_val argv) {
    RETVAL = Integer::NewFromUnsigned(ISOLATE(vm), ui);
}

static void from_double(js_vm *vm, double d, js_val argv) {
    RETVAL = Number::New(ISOLATE(vm), d);
}

static void from_pointer(js_vm *vm, void *ptr, js_val argv) {
    if (!ptr)
        RETVAL = Local<Value>::New(ISOLATE(vm),
                    vm->nullptr_handle->handle);
    else
        RETVAL = WrapPtr(vm, ptr);
}

static int call_str(js_vm *vm, const char *source, js_val argv) {
    struct js8_arg_s args;
    args.type = V8CALLSTR;
    assert(source);
    args.vm = vm;
    args.source = (char *)source;
    args.nargs = 0;
    args.h = nullptr;   // this == Global
    if (argv) {
        // this
        args.h = make_handle(vm,
                    reinterpret_cast<Local<Value> *>(argv)[0], V8UNKNOWN);
    }
    js_handle *r = CallFunc(& args);
    // WaitFor(vm) ???
    if (args.h)
        js_reset(args.h);
    if (r) {
        js_reset(r);
        return true; // Success
    }
    return false;   // Error
}

static struct js_dlfn_s dlfns = {
    to_int,
    to_uint,
    to_double,
    to_string,
    to_pointer,
    from_int,
    from_uint,
    from_double,
    from_pointer,
    call_str,
    js_errstr
};


typedef int (*Fnload)(js_vm *, js_val, js_dlfn_s *, js_ffn **);

// $load - load a dynamic library.
// The filename must contain a slash. Any path search should be
// done in the JS.

static void Load(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    js_vm *vm = static_cast<js_vm*>(isolate->GetData(0));

    int argc = args.Length();
    ThrowNotEnoughArgs(isolate, argc < 1);
    Local<Context> context = isolate->GetCurrentContext();
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

    Fnload load_func = (Fnload) dlsym(dl, LOAD_FUNC);
    if (!load_func) {
        dlclose(dl);
        ThrowError(isolate,
            "$load: cannot find library initialization function");
    }

    js_ffn *functab;
    Local<Object> o1 = Object::New(isolate);
    Local<Value> argv[] = { o1 };
    js_set_errstr(vm, nullptr);
    int nfunc = load_func(vm, argv, &dlfns, &functab);
    if (nfunc < 0) {
        dlclose(dl);
        ThrowError(isolate, vm->errstr ? vm->errstr : "unknown error");
    }
    for (int i = 0; i < nfunc; i++) {
        if (!functab[i].name)
            break;
        functab[i].isdlfunc = 1;
        Local<ObjectTemplate> templ =
            Local<ObjectTemplate>::New(isolate, vm->extfunc_template);
        Local<Object> fnObj = templ->NewInstance(context).ToLocalChecked();
        fnObj->SetAlignedPointerInInternalField(0,
             reinterpret_cast<void*>(static_cast<uintptr_t>(V8EXTFUNC<<2)));
        fnObj->SetInternalField(1,
                    External::New(isolate, (void *)&functab[i]));
        fnObj->SetPrototype(Local<Value>::New(isolate, vm->ctype_proto));

        o1->Set(context,
                String::NewFromUtf8(isolate, functab[i].name),
                fnObj).FromJust();
    }
    args.GetReturnValue().Set(o1);
}


///////////////////////////////////////////////////////////////

// The second part of the vm initialization.
static void CreateIsolate(js_vm *vm) {
    v8init();
    Isolate* isolate = Isolate::New(create_params);
    LOCK_SCOPE(isolate)

    assert(isolate->GetCurrentContext().IsEmpty());

    isolate->AddMicrotasksCompletedCallback(v8MicrotasksCompleted);
    isolate->EnqueueMicrotask(v8Microtask, vm);

    vm->isolate = isolate;

    js_handle *hargs = new js_handle[MAXARGS];
    for (int i = 0; i < MAXARGS; i++) {
        hargs[i].vm = vm;
        hargs[i].flags = ARG_HANDLE;
        vm->args[i] = &hargs[i];
    }
    vm->dlstr_idx = 0;

    // isolate->SetCaptureStackTraceForUncaughtExceptions(true);
    isolate->SetData(0, vm);

    Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
    global->Set(String::NewFromUtf8(isolate, "$print"),
                FunctionTemplate::New(isolate, Print));
    global->Set(String::NewFromUtf8(isolate, "$go"),
                FunctionTemplate::New(isolate, Go));
    global->Set(String::NewFromUtf8(isolate, "$msleep"),
                FunctionTemplate::New(isolate, MSleep));
    global->Set(String::NewFromUtf8(isolate, "$now"),
                FunctionTemplate::New(isolate, Now));
    global->Set(String::NewFromUtf8(isolate, "$send"),
                FunctionTemplate::New(isolate, Send));
    global->Set(String::NewFromUtf8(isolate, "$close"),
                FunctionTemplate::New(isolate, Close));
    global->Set(String::NewFromUtf8(isolate, "$eval"),
                FunctionTemplate::New(isolate, EvalString));
    global->Set(String::NewFromUtf8(isolate, "$malloc"),
                FunctionTemplate::New(isolate, Malloc));
    global->Set(String::NewFromUtf8(isolate, "$load"),
                FunctionTemplate::New(isolate, Load));

    Local<Context> context = Context::New(isolate, NULL, global);
    if (context.IsEmpty()) {
        fprintf(stderr, "failed to create a V8 context\n");
        exit(1);
    }

    vm->context.Reset(isolate, context);

    Context::Scope context_scope(context);

    // Make the template for external(foreign) pointer objects.
    Local<ObjectTemplate> extptr_templ = ObjectTemplate::New(isolate);
    extptr_templ->SetInternalFieldCount(2);
    vm->extptr_template.Reset(isolate, extptr_templ);

    // Make the template for foreign function objects.
    Local<ObjectTemplate> ffunc_templ = ObjectTemplate::New(isolate);
    ffunc_templ->SetInternalFieldCount(2);
    ffunc_templ->SetCallAsFunctionHandler(CallForeignFunc);
    vm->extfunc_template.Reset(isolate, ffunc_templ);

    vm->undef_handle = new js_handle_s;
    vm->undef_handle->vm = vm;
    vm->undef_handle->type = V8UNDEFINED;
    vm->undef_handle->flags = PERM_HANDLE;
    vm->undef_handle->handle.Reset(vm->isolate, v8::Undefined(isolate));
    vm->null_handle = new js_handle_s;
    vm->null_handle->vm = vm;
    vm->null_handle->type = V8NULL;
    vm->null_handle->flags = PERM_HANDLE;
    vm->null_handle->handle.Reset(vm->isolate, v8::Null(isolate));

    // Create __proto__ for C-type objects.
    MakeCtypeProto(vm);

    Local<Object> nullptr_obj = extptr_templ->NewInstance(
                                            context).ToLocalChecked();
    nullptr_obj->SetAlignedPointerInInternalField(0,
            reinterpret_cast<void*>(static_cast<uintptr_t>(V8EXTPTR<<2)));
    nullptr_obj->SetInternalField(1, External::New(isolate, nullptr));
    nullptr_obj->SetPrototype(Local<Value>::New(isolate, vm->cptr_proto));
    vm->nullptr_handle = new js_handle_s;
    vm->nullptr_handle->vm = vm;
    vm->nullptr_handle->type = V8EXTPTR;
    vm->nullptr_handle->ptr = nullptr;
    vm->nullptr_handle->flags = (PERM_HANDLE|PTR_HANDLE);
    vm->nullptr_handle->handle.Reset(isolate, nullptr_obj);

    // Name the global object "Global".
    Local<Object> realGlobal = Local<Object>::Cast(
                        context->Global()->GetPrototype());
    realGlobal->Set(String::NewFromUtf8(isolate, "Global"), realGlobal);
    realGlobal->Set(String::NewFromUtf8(isolate, "$nullptr"), nullptr_obj);
    realGlobal->SetAccessor(context,
            String::NewFromUtf8(isolate, "$errno"),
            GlobalGet, GlobalSet).FromJust();
    vm->global_handle = make_handle(vm, realGlobal, V8OBJECT);
    vm->global_handle->flags |= PERM_HANDLE;
}

// Runs in the worker(V8) thread.
int js8_vminit(js_vm *vm) {
    CreateIsolate(vm);
    go(recv_coro(vm));
    return 0;
}



}
