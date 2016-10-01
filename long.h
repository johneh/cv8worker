extern "C" {

extern v8::Local<v8::Object> Int64(js_vm *vm, int64_t i64);
extern v8::Local<v8::Object> UInt64(js_vm *vm, uint64_t ui64);
extern int IsInt64(v8::Local<v8::Value> v);
extern int64_t GetInt64(v8::Local<v8::Value> v);
extern int IsUInt64(v8::Local<v8::Value> v);
extern uint64_t GetUInt64(v8::Local<v8::Value> v);
extern void LongCntl(const v8::FunctionCallbackInfo<v8::Value>& args);
extern v8::Local<v8::FunctionTemplate> LongTemplate(js_vm *vm);

}
