#include <stddef.h>
#include "int.h"
#include "endianess.h"
#include "decode_errors.h"
#include "lzw_decode.h"
#include "lzhuf_decode.h"

const char *decode_errstr(int error_code) {
    #define X(error, code) if (error_code == code) { return #error; }
    DECODE_ERRORS
    #undef X

    return "Unknown error code";
}

#define MAX(a, b) ((a > b) ? (a) : (b))
size_t generic_decode_buffers_size() {
    // It's just the maximum of any of the possible decoder's buffers, because we only use one at a time.
    size_t a = lzw_decode_buffers_size();
    size_t b = lzhuf_decode_buffers_size();

    return MAX(a, b);
}

static int noop_decode(const char *in, size_t in_size, char *out, size_t max_out_size, size_t *decoded_size) {
    if (max_out_size < in_size) {
        return DECODE_NOOP_ERROR_INSUFFICIENT_OUTPUT;
    }

    memcpy(out, in, in_size);
    *decoded_size = in_size;
    return DECODE_OK;
}

int generic_decode(void *buffers, const char *in, size_t in_size, char *out, size_t max_out_size, size_t *decoded_size) {
    if (in_size < 5) {
        return DECODE_ERROR_NO_HEADER;
    }

    u8 compression_type = in[0];
    u32 uncompressed_size = cast_u32_le(in+1);

    if (uncompressed_size > max_out_size) {
        return DECODE_ERROR_OUTPUT_NOT_LARGE_ENOUGH;
    }

    in += 5;
    in_size -= 5;

    switch (compression_type) {
        case 0:     // No compression
        return noop_decode(in, in_size, out, uncompressed_size, decoded_size);
        break;

        case 1:     // RLE
        return DECODE_RLE_ERROR_UNIMPLEMENTED;
        break;

        case 2:     // LZW
        return lzw_decode(buffers, in, in_size, out, uncompressed_size, decoded_size);
        break;

        case 3:     // lzhuf
        return lzhuf_decode(buffers, in, in_size, out, uncompressed_size, decoded_size);
        break;

        default:
        return DECODE_ERROR_UNKNOWN_TYPE;
    }
}