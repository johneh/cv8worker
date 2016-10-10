extern "C" {

typedef struct {
    union {
        int64_t i64;
        uint64_t u64;
#if __BYTE_ORDER == __BIG_ENDIAN
        struct { int32_t high; int32_t low; };
        struct { uint32_t uhigh; uint32_t ulow; };
#else
        struct { int32_t low; int32_t high; };
        struct { uint32_t ulow; uint32_t uhigh; };
#endif
    } val;
    int issigned;
} long64;

extern v8::Local<v8::Object> Int64(js_vm *vm, int64_t i64);
extern v8::Local<v8::Object> UInt64(js_vm *vm, uint64_t ui64);
int LongValue(v8::Local<v8::Value> v, long64 *val);
extern void LongCntl(const v8::FunctionCallbackInfo<v8::Value>& args);
extern v8::Local<v8::FunctionTemplate> LongTemplate(js_vm *vm);

}
