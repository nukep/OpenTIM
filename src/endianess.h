#ifndef ENDIANESS_H
#define ENDIANESS_H

#include <string.h>
#include <endian.h>
#include <stddef.h>
#include "int.h"

static inline void swap_endianess(char *buf, size_t n) {
    if (n % 2 != 0) {
        return;
    }

    for (size_t i = 0; i < n/2; i++) {
        char tmp = buf[i];
        buf[i] = buf[n-1-i];
        buf[n-1-i] = tmp;
    }
}

#if BYTE_ORDER == BIG_ENDIAN
static inline void adjust_le_to_native(char *buf, size_t n) {
    swap_endianess(buf, n);
}

// if platform is big-endian, do nothing
static inline void adjust_be_to_native(char *buf, size_t n) {
}
#else
// if platform is little-endian, do nothing
static inline void adjust_le_to_native(char *buf, size_t n) {
}

static inline void adjust_be_to_native(char *buf, size_t n) {
    swap_endianess(buf, n);
}
#endif

static inline void cast_16_impl(const void *in, void *buf, bool le) {
    memcpy(buf, in, 2);
    if (le) {
        adjust_le_to_native(buf, 2);
    } else {
        adjust_be_to_native(buf, 2);
    }
}

static inline void cast_32_impl(const void *in, void *buf, bool le) {
    memcpy(buf, in, 4);
    if (le) {
        adjust_le_to_native(buf, 4);
    } else {
        adjust_be_to_native(buf, 4);
    }
}

#define IMPL(type, castf) static inline type cast_##type##_le(const char *in) { \
    type val; \
    castf(in, &val, 1); \
    return val; \
} \
static inline type cast_##type##_be(const char *in) { \
    type val; \
    castf(in, &val, 0); \
    return val; \
}

IMPL(s16, cast_16_impl)
IMPL(u16, cast_16_impl)
IMPL(s32, cast_32_impl)
IMPL(u32, cast_32_impl)

#undef F

#endif