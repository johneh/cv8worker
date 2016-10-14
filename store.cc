#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "v8.h"
#include "v8binding.h"
#include "vm.h"
#include "store.h"
#include "util.h"

using namespace v8;

#define BITSIZE sizeof(uint32_t) 
#define BITWIDTH (BITSIZE * 8)
#define MarkSlot(a, k) (a[k/BITWIDTH] |= (1 << (k%BITWIDTH)))
#define IsSlotMarked(a, sz, k) (k > 0 && k < sz && (a[k/BITWIDTH] & (1 << (k%BITWIDTH))))
#define ClearSlot(a, k)    (a[k/BITWIDTH] &= ~(1 << (k%BITWIDTH)))


PersistentStore::PersistentStore(Isolate *isolate, unsigned init_size)
             : isolate_(isolate) {
    size_ = (init_size / BITWIDTH) * BITWIDTH;
    if (size_ == 0)
        size_ = BITWIDTH;
    container_.Reset(isolate_, MakeContainer(size_));
    na_ = size_ / BITWIDTH;
    a_ = (uint32_t *) emalloc(na_ * BITSIZE);
    memset((void *) a_, '\0', na_ * BITSIZE); 
    Set(v8::Undefined(isolate));   // slot 0 is invalid (error return)
}

PersistentStore::~PersistentStore() {
    container_.Reset();
    free(a_);
    size_ = 0;
}

unsigned PersistentStore::Set(v8Value value) {
    HandleScope handle_scope(isolate_);
    unsigned slot = FindSlot();

    js_vm *vm = reinterpret_cast<js_vm*>(isolate_->GetData(0));
    v8Context context = v8Context::New(isolate_, vm->context);
    Context::Scope context_scope(context);

    if (slot == size_) {
        //  Performance Tips for JavaScript in V8
        //  https://www.html5rocks.com/en/tutorials/speed/v8/
        //      Use contiguous keys starting at 0 for Arrays.
        //      Don't pre-allocate large Arrays (e.g. > 64K elements)
        //       to their maximum size, instead grow as you go.
        //      Don't delete elements in arrays, especially numeric arrays.
        //      Don't load uninitialized or deleted elements.

        size_++;
    }

    bool success = v8Object::New(isolate_, container_)
                            -> Set(context, slot, value).FromJust();
    assert(success);    // It is writable. Right?
    return slot;
}

v8Value PersistentStore::Get(unsigned slot) {
    if (!IsSlotMarked(a_, size_, slot))
        return v8Value();
    EscapableHandleScope handle_scope(isolate_);
    js_vm *vm = reinterpret_cast<js_vm*>(isolate_->GetData(0));
    v8Context context = v8Context::New(isolate_, vm->context);
    Context::Scope context_scope(context);
    v8Value v = v8Object::New(isolate_, container_)
                    ->Get(context, slot).ToLocalChecked();
    return handle_scope.Escape(v);
}

void PersistentStore::Dispose(unsigned slot) {
    if (IsSlotMarked(a_, size_, slot)) {
        HandleScope handle_scope(isolate_);
        js_vm *vm = reinterpret_cast<js_vm*>(isolate_->GetData(0));
        v8Context context = v8Context::New(isolate_, vm->context);
        Context::Scope context_scope(context);
        v8Object::New(isolate_, container_)->Set(context,
                        slot, v8::Undefined(isolate_)).FromJust();
        ClearSlot(a_, slot);
    }
}

int PersistentStore::FindSlot() {
    unsigned i, n = na_;
    for (i = 0; i < n; i++) {
        uint32_t k = ~a_[i];
        if (k != 0) {
            unsigned j = __builtin_ffs(k); // find first set (one)
            j += (i * BITWIDTH - 1);
            MarkSlot(a_, j);
            return j;
        }
    }

    // enlarge the bit-array
    na_ *= 2;
    a_ = (uint32_t *) erealloc(a_, na_ * BITSIZE);
    memset((void*)&a_[n], '\0', n * BITSIZE);
    unsigned j = n * BITWIDTH;
    MarkSlot(a_, j);
    return j;
}

v8Object PersistentStore::MakeContainer(unsigned size) {
    EscapableHandleScope handle_scope(isolate_);
    v8Array arr = Array::New(isolate_, size);
    return handle_scope.Escape(arr);
}
