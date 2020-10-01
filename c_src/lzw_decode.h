#ifndef LZW_DECODE_H
#define LZW_DECODE_H

#include <stddef.h>
#include "int.h"
#include "decode_errors.h"

#define WARN_UNUSED __attribute__ ((warn_unused_result))

struct LzwDecodeBuffers;

size_t lzw_decode_buffers_size();
WARN_UNUSED int lzw_decode(struct LzwDecodeBuffers *buffers, const char *in, size_t in_size, char *out, size_t out_size, size_t *decoded_size);

#endif