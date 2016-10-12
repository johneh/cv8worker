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
#define IsValidSlot(a, sz, k) (k > 0 && k < sz && (a[k/BITWIDTH] & (1 << (k%BITWIDTH))))
#define ClearSlot(a, k)    (a[k/BITWIDTH] &= ~(1 << (k%BITWIDTH)))


// FIXME -- use new instead of malloc .. ?

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
    if (slot >= size_) {
        unsigned new_size = 2 * size_;
        v8Object o2 = MakeContainer((int) new_size);
        v8Object o1 = v8Object::New(isolate_, container_);
        for (unsigned i = 0; i < size_; i++) {
            o2->SetInternalField(i, o1->GetInternalField(i));
        }
        size_ = new_size;
        container_.Reset(isolate_, o2);
        assert(na_ == size_/BITWIDTH);
    }
    v8Object::New(isolate_, container_)->SetInternalField(slot, value);
    return slot;
}

v8Value PersistentStore::Get(unsigned slot) {
    EscapableHandleScope handle_scope(isolate_);
    v8Value v;
    if (!IsValidSlot(a_, size_, slot))
        v = v8::Undefined(isolate_);    //XXX: v8Value() Empty handle ?
    else
        v = v8Object::New(isolate_, container_)->GetInternalField(slot);
    return handle_scope.Escape(v);
}

void PersistentStore::Dispose(unsigned slot) {
    if (!IsValidSlot(a_, size_, slot))
        return;
    HandleScope handle_scope(isolate_);
    v8Object::New(isolate_, container_)
                    -> SetInternalField(slot, v8::Undefined(isolate_));
    ClearSlot(a_, slot);
}

int PersistentStore::FindSlot() {
    unsigned i, sz = na_;
    for (i = 0; i < sz; i++) {
        uint32_t k = ~a_[i];
        if (k != 0) {
            unsigned j = __builtin_ffs(k);
            j += (i * BITWIDTH - 1);
            MarkSlot(a_, j);
            return j;
        }
    }

    // enlarge the bit-array
    na_ *= 2;
    a_ = (uint32_t *) erealloc(a_, na_ * BITSIZE);
    memset((void*)&a_[sz], '\0', sz * BITSIZE); 
    unsigned j = sz * BITWIDTH;
    MarkSlot(a_, j);
    return j;
}

v8Object PersistentStore::MakeContainer(unsigned size) {
    EscapableHandleScope handle_scope(isolate_);
    v8ObjectTemplate templ = ObjectTemplate::New(isolate_);
    templ->SetInternalFieldCount(size);
    v8Object obj = templ->NewInstance(
            isolate_->GetCurrentContext()).ToLocalChecked();
    return handle_scope.Escape(obj);
}
