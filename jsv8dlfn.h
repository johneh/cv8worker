#include <stdint.h>
#include "jsdef.h"

#ifndef V8_BINDING   /* define in the main source before including this header */
static const struct v8_fn_s *jsv8;

#define JS_LOAD(vm, hlibobj) \
v8_load_(vm, hlibobj, \
        const struct v8_fn_s *const j8_, v8_ffn **fp_) {\
    jsv8 = j8_;

#define JS_EXPORT(t_) } do {\
*fp_ = t_; return sizeof(t_)/sizeof(t_[0]);\
} while(0)

#ifndef coroutine
#if defined __GNUC__ || defined __clang__
#define coroutine __attribute__((noinline))
#else
#define coroutine
#endif
#endif

#else
/* The library must export this routine: */
#define V8_LOAD_FUNC "v8_load_"
#endif
