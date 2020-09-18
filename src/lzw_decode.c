#include "int.h"
#include "testing.h"
#include <stdlib.h>
#include "lzw_decode.h"

// Adapted from ScummVM (licensed GPL v2 or greater):
// https://github.com/scummvm/scummvm/blob/d11c61db1466d79ef6253814a86b04950b499ec3/engines/sci/decompressor.cpp
//
// This LZW decompression algorithm is subtlely different from other Sierra games.
// 1) The length 2+ dictionary starts at 0x101 instead of 0x102.
// 2) The reset command aligns the read stream to a 16-byte boundary.

#define IN(n) read_bits(read_ctx, n); subbit = (subbit+n);
#define OUT(v) if (out_off >= out_size) { return LZW_ERROR_INSUFFICIENT_OUTPUT; } out[out_off] = v; out_off++;

#define LZW_MAX_BITS 12

// Pass in buffers so the caller can control memory allocation (we prefer this style of coding in general).
struct LzwDecodeBuffers {
    char *tokenlist[1<<LZW_MAX_BITS];
    u16 tokenlengthlist[1<<LZW_MAX_BITS];
};

static inline int lzw_decode_impl(struct LzwDecodeBuffers *buffers, void *read_ctx, s32 (*read_bits)(void *, u32), int (*eof)(void *), char *out, size_t out_size, size_t *decoded_size) {
    u32 n_bits = 9;
    s32 curtoken = 0x101;
    s32 endtoken = 0x1ff;

    u32 subbit = 0;
    size_t out_off = 0;

    char **tokenlist     = buffers->tokenlist;
    u16 *tokenlengthlist = buffers->tokenlengthlist;

    u16 tokenlastlength = 0;

    while (!eof(read_ctx) && out_off < out_size) {
        s32 token = IN(n_bits);
        if (token < 0) {
            return LZW_ERROR_NO_MORE_INPUT;
        }

        if (token == 0x100) {
            // Reset the dictionary

            // Seek forward to a 16-byte boundary (e.g. 0x11C -> 0x120, 0x33A1 -> 0x33B0)
            u32 sb = subbit % (8*16);
            if (sb > 0) {
                u32 r = (8*16) - sb;
                IN(r%8);
                for (u32 i = 0; i < r/8; i++) {
                    IN(8);
                }
            }

            n_bits = 9;
            curtoken = 0x101;
            endtoken = 0x1ff;
        } else {
            if (token >= 0x100) {
                // Use token in dictionary

                if (token >= curtoken) {
                    return LZW_ERROR_BAD_TOKEN;
                }
                tokenlastlength = tokenlengthlist[token] + 1;

                for (int i = 0; i < tokenlastlength; i++) {
                    OUT(tokenlist[token][i]);
                }
            } else {
                tokenlastlength = 1;
                OUT(token & 0xFF);
            }

            if (curtoken > endtoken) {
                if (n_bits < LZW_MAX_BITS) {
                    n_bits += 1;
                    endtoken = (endtoken << 1) + 1;     // 511 -> 1023 -> 2047 -> 4095
                }
            }
            if (curtoken <= endtoken) {
                tokenlist[curtoken]       = out + (out_off - tokenlastlength);
                tokenlengthlist[curtoken] = tokenlastlength;
                curtoken += 1;
            }
        }
    }

    *decoded_size = out_off;
    return LZW_OK;
}
#undef IN
#undef OUT

struct BitReaderContext {
    u32 bit_off;
    const char *buf;
    u32 buf_size;
};

static s32 bit_reader(void *_read_ctx, u32 bits) {
    // Reads the least-significant bits first ("right to left")

    struct BitReaderContext *ctx = _read_ctx;

    u32 val = 0;

    for (u32 i = 0; i < bits; i++) {
        u32 byte_off = ctx->bit_off / 8;
        u32 bit = ctx->bit_off % 8;

        if (byte_off >= ctx->buf_size) {
            // can't read any more bytes
            return -1;
        }

        if ((ctx->buf[byte_off] & (1 << bit)) != 0) {
            val |= (1 << i);
        }

        ctx->bit_off += 1;
    }

    return val;
}

static int bit_reader_is_eof(void *_read_ctx) {
    struct BitReaderContext *ctx = _read_ctx;

    return ctx->bit_off/8 >= ctx->buf_size;
}

size_t lzw_decode_buffers_size() {
    return sizeof(struct LzwDecodeBuffers);
}

int lzw_decode(struct LzwDecodeBuffers *buffers, const char *in, size_t in_size, char *out, size_t out_size, size_t *decoded_size) {
    struct BitReaderContext ctx = { .bit_off = 0, .buf = in, .buf_size = in_size };
    return lzw_decode_impl(buffers, &ctx, bit_reader, bit_reader_is_eof, out, out_size, decoded_size);
}

#if ENABLE_TEST_SUITE

#define CHECK_EXPECTEDS(buf, expecteds, bits) { \
    struct BitReaderContext ctx = { .bit_off = 0, .buf = buf, .buf_size = sizeof(buf) }; \
    for (u32 i = 0; i < sizeof(expecteds)/sizeof(s32); i++) { ASSERT_EQ(bit_reader(&ctx, bits), expecteds[i]); } \
}

TEST_SUITE(lzw_decode) {
    TEST("bit_reader") {
        char buf[] = { 0xCA, 0xDD };
        s32 expecteds_1[] = { 0, 1, 0, 1, 0, 0, 1, 1,    1, 0, 1, 1, 1, 0, 1, 1,    -1, -1 };
        s32 expecteds_3[] = { 2, 1, 7, 6, 5, -1, -1, -1 };
        s32 expecteds_4[] = { 0xA, 0xC, 0xD, 0xD, -1 };
        s32 expecteds_8[] = { 0xCA, 0xDD, -1, -1 };
        s32 expecteds_9[] = { 0x1CA, -1 };
        s32 expecteds_16[] = { 0xDDCA, -1 };

        CHECK_EXPECTEDS(buf, expecteds_1, 1);
        CHECK_EXPECTEDS(buf, expecteds_3, 3);
        CHECK_EXPECTEDS(buf, expecteds_4, 4);
        CHECK_EXPECTEDS(buf, expecteds_8, 8);
        CHECK_EXPECTEDS(buf, expecteds_9, 9);
        CHECK_EXPECTEDS(buf, expecteds_16, 16);
    }

    // TODO - add some lzw_decode tests. We don't have an encoder to create sample inputs, so we either have to write an encoder or craft inputs by hand.
}

#endif