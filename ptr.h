extern "C" {

extern void MakeCtypeProto(js_vm *vm);
extern int GetCtypeId(js_vm *vm, v8::Local<v8::Value> v);
extern int IsCtypeWeak(v8::Local<v8::Object> obj);
extern v8::Local<v8::Object> ToPtr(v8::Local<v8::Value> v);
extern v8::Local<v8::Object> WrapPtr(js_vm *vm, void *ptr);
extern void *UnwrapPtr(v8::Local<v8::Object> ptrObj);
extern void Dispose(const v8::FunctionCallbackInfo<v8::Value>& args);

}
