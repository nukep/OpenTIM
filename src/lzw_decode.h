#ifndef LZW_DECODE_H
#define LZW_DECODE_H

#include <stddef.h>
#include "int.h"

#define WARN_UNUSED __attribute__ ((warn_unused_result))

#define LZW_ERRORS \
    X(LZW_ERROR_DICTIONARY_FULL, -4) \
    X(LZW_ERROR_INSUFFICIENT_OUTPUT, -3) \
    X(LZW_ERROR_NO_MORE_INPUT, -2) \
    X(LZW_ERROR_BAD_TOKEN, -1) \
    X(LZW_OK, 0)

#define X(error, code) error = code,
enum LzwErrors {
    LZW_ERRORS
};
#undef X

struct LzwDecodeBuffers;

size_t lzw_decode_buffers_size();
char *lzw_errstr(int error_code);
WARN_UNUSED int lzw_decode(struct LzwDecodeBuffers *buffers, const char *in, size_t in_size, char *out, size_t out_size, size_t *decoded_size);

#endif