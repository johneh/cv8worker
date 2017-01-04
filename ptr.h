extern "C" {

#define PTR_INTERNAL_FIELD_COUNT    2

extern void MakeCtypeProto(js_vm *vm);
extern int IsCtypeWeak(v8::Local<v8::Object> ptrObj);
extern void SetCtypeWeak(v8::Local<v8::Object> ptrObj);
extern v8::Local<v8::Object> ToPtr(v8::Local<v8::Value> v);
extern v8::Local<v8::Object> WrapPtr(js_vm *vm, void *ptr);
extern void *UnwrapPtr(v8::Local<v8::Object> ptrObj);
extern void Gc(const v8::FunctionCallbackInfo<v8::Value>& args);

extern void Pack(const v8::FunctionCallbackInfo<v8::Value>& args);
extern void Unpack(const v8::FunctionCallbackInfo<v8::Value>& args);

}
