extern "C" {

#define PTR_INTERNAL_FIELD_COUNT    3

extern void MakeCtypeProto(js_vm *vm);
extern int IsCtypeWeak(v8::Local<v8::Object> ptrObj);
extern void SetCtypeWeak(v8::Local<v8::Object> ptrObj);
extern void SetCtypeSize(v8::Local<v8::Object> ptrObj, int size);
extern int GetCtypeSize(v8::Local<v8::Object> ptrObj);
extern v8::Local<v8::Object> ToPtr(v8::Local<v8::Value> v);
extern v8::Local<v8::Object> WrapPtr(js_vm *vm, void *ptr);
extern void *UnwrapPtr(v8::Local<v8::Object> ptrObj);
extern void Dispose(const v8::FunctionCallbackInfo<v8::Value>& args);

}
