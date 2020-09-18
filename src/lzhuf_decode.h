#ifndef LZHUF_DECODE_H
#define LZHUF_DECODE_H

#include <stddef.h>
#include "int.h"

#define WARN_UNUSED __attribute__ ((warn_unused_result))

#define LZHUF_ERRORS \
    X(LZHUF_ERROR_INSUFFICIENT_OUTPUT, -1) \
    X(LZHUF_OK, 0)

#define X(error, code) error = code,
enum LzhufErrors {
    LZHUF_ERRORS
};
#undef X

struct LzhufDecodeBuffers;

size_t lzhuf_decode_buffers_size();
const char *lzhuf_errstr(int error_code);
WARN_UNUSED int lzhuf_decode(struct LzhufDecodeBuffers *buffers, const char *in, size_t in_size, char *out, size_t out_size, size_t *decoded_size);

#endif