
class PersistentStore {
 public:
    explicit PersistentStore(v8::Isolate *isolate, unsigned init_size);
    ~PersistentStore();
    int GetSize() { return size_; }
    unsigned Set(v8::Local<v8::Value> value);
    v8::Local<v8::Value> Get(unsigned slot);
    void Dispose(unsigned slot);
 private:
    int FindSlot();
    v8::Local<v8::Object> MakeContainer(unsigned size);

 private:
    v8::Isolate *isolate_;
    uint32_t *a_;
    unsigned na_;
    unsigned size_;
    v8::Persistent<v8::Object> container_;
};
