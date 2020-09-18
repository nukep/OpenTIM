// A readaptation of lzhuf.c from Haruyasu Yoshizaki.
// Readapted by Danny Spencer (2020). Decompression only.
// Original credits:
/**************************************************************
    lzhuf.c
    written by Haruyasu Yoshizaki 1988/11/20
    some minor changes 1989/04/06
    comments translated by Haruhiko Okumura 1989/04/07
    getbit and getbyte modified 1990/03/23 by Paul Edwards
      so that they would work on machines where integers are
      not necessarily 16 bits (although ANSI guarantees a
      minimum of 16).  This program has compiled and run with
      no errors under Turbo C 2.0, Power C, and SAS/C 4.5
      (running on an IBM mainframe under MVS/XA 2.2).  Could
      people please use YYYY/MM/DD date format so that everyone
      in the world can know what format the date is in?
    external storage of filesize changed 1990/04/18 by Paul Edwards to
      Intel's "little endian" rather than a machine-dependant style so
      that files produced on one machine with lzhuf can be decoded on
      any other.  "little endian" style was chosen since lzhuf
      originated on PC's, and therefore they should dictate the
      standard.
    initialization of something predicting spaces changed 1990/04/22 by
      Paul Edwards so that when the compressed file is taken somewhere
      else, it will decode properly, without changing ascii spaces to
      ebcdic spaces.  This was done by changing the ' ' (space literal)
      to 0x20 (which is the far most likely character to occur, if you
      don't know what environment it will be running on.
**************************************************************/

#include <string.h>
#include <stddef.h>
#include "lzhuf_decode.h"
#include "int.h"
#include "testing.h"

const char *lzhuf_errstr(int error_code) {
    #define X(error, code) if (error_code == code) { return #error; }
    LZHUF_ERRORS
    #undef X

    return "Unknown error code";
}

#define N           4096                    // Buffer size
#define F           60                      // Lookahead buffer size
#define THRESHOLD   2
#define NIL         BUFFER_SIZE             // Leaf of tree

#define N_CHAR      (256 - THRESHOLD + F)   // Kinds of characters (character code = 0..N_CHAR-1)
#define T           (N_CHAR*2 - 1)          // Size of table
#define R           (T - 1)                 // Position of root
#define MAX_FREQ    0x8000                  // Updates tree when the

/* table for decoding the upper 6 bits of position */

/* for decoding */
static const u8 d_code[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
    0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
    0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
    0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
    0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
    0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
    0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
    0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
    0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
    0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

static const u8 d_len[256] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

struct LzhufDecodeBuffers {
    u8 text_buf[N + F - 1];

    u16 freq[T + 1];            // Frequency table

    u16 prnt[T + N_CHAR];       // Pointers to parent nodes, except for the
                                // elements [T..T + N_CHAR - 1] which are used to get
                                // the positions of leaves corresponding to the codes.

    u16 son[T];                 // Pointers to child nodes (son[], son[] + 1)
};

static void lzhuf_init(struct LzhufDecodeBuffers *b) {
    for (u16 i = 0; i < N_CHAR; i++) {
        b->freq[i] = 1;
        b->son[i] = i + T;
        b->prnt[i + T] = i;
    }

    u16 i = 0;
    for (u16 j = N_CHAR; j <= R; j++) {
        b->freq[j] = b->freq[i] + b->freq[i + 1];
        b->son[j] = i;
        b->prnt[i] = b->prnt[i + 1] = j;
        i += 2;
    }
    b->freq[T] = 0xFFFF;
    b->prnt[R] = 0;
}

// Reconstruction of tree
static void reconst(struct LzhufDecodeBuffers *b) {
    // Collect leaf nodes in the first half of the table
    // and replace the freq by (freq + 1) / 2
    u16 j = 0;
    for (u16 i = 0; i < T; i++) {
        if (b->son[i] >= T) {
            b->freq[j] = (b->freq[i] + 1)/2;
            b->son[j] = b->son[i];
            j += 1;
        }
    }

    // Begin constructing tree by connecting sons
    for (u16 i = 0, j = N_CHAR; j < T; i += 2, j++) {
        u16 k = i + 1;
        u16 f = b->freq[j] = b->freq[i] + b->freq[k];
        
        for (k = j-1; f < b->freq[k]; k--);

        k += 1;

        u16 l = (j - k)*2;
        memmove(b->freq + k + 1, b->freq + k, l);
        b->freq[k] = f;
        memmove(b->son + k + 1, b->son + k, l);
        b->son[k] = i;
    }

    // Connect prnt
    for (u16 i = 0; i < T; i++) {
        u16 k = b->son[i];
        if (k >= T) {
            b->prnt[k] = i;
        } else {
            b->prnt[k] = b->prnt[k + 1] = i;
        }
    }
}

// Increment frequency of given code by one, and update tree
static void update(struct LzhufDecodeBuffers *b, u16 c) {
    if (b->freq[R] == MAX_FREQ) {
        reconst(b);
    }
    c = b->prnt[c + T];

    do {
        b->freq[c] += 1;
        u16 k = b->freq[c];

        // If the order is disturbed, exchange nodes
        u16 l = c + 1;
        if (k > b->freq[l]) {
            while (k > b->freq[++l]);

            l -= 1;
            b->freq[c] = b->freq[l];
            b->freq[l] = k;

            u16 i = b->son[c];
            b->prnt[i] = l;
            if (i < T) {
                b->prnt[i + 1] = l;
            }

            u16 j = b->son[l];
            b->son[l] = i;

            b->prnt[j] = c;
            if (j < T) {
                b->prnt[j + 1] = c;
            }
            b->son[c] = j;

            c = l;
        }

        c = b->prnt[c];
    } while (c != 0);       // repeat up to root
}

#define IN(n) read_bits(read_ctx, n)
#define OUT(v) if (out_off >= out_size) { return LZHUF_ERROR_INSUFFICIENT_OUTPUT; } out[out_off] = v; out_off++;
static inline int lzhuf_decode_impl(struct LzhufDecodeBuffers *b, void *read_ctx, s32 (*read_bits)(void *, u32), char *out, size_t out_size, size_t *decoded_size) {
    size_t out_off = 0;

    lzhuf_init(b);

    for (int i = 0; i < N-F; i++) {
        b->text_buf[i] = 0x20;
    }

    u16 r = N - F;
    size_t count = 0;
    while (count < out_size) {
        // Decode char
        u16 c = b->son[R];
        while (c < T) {
            c += IN(1);
            c = b->son[c];
        }
        c -= T;
        update(b, c);
        // End decode char

        if (c < 256) {
            OUT(c);
            b->text_buf[r] = c;
            r += 1;
            r &= N-1;
            count += 1;
        } else {
            // Decode position
            u16 pos;
            {
                // Recover upper bits from table
                u16 i = IN(8);
                u16 c = d_code[i] << 6;
                u8 j = d_len[i];

                // Read lower 6 bits verbatim
                j -= 2;
                while (j > 0) {
                    j -= 1;
                    i = (i << 1) + IN(1);
                }

                pos = c | (i & 0x3F);
            }
            // End decode position

            int i = (r - pos - 1) & (N-1);
            int j = c - 255 + THRESHOLD;
            for (int k = 0; k < j; k++) {
                c = b->text_buf[(i + k) & (N - 1)];
                OUT(c);
                b->text_buf[r] = c;
                r += 1;
                r &= (N - 1);
                count += 1;
            }
        }
    }

    *decoded_size = out_off;
    return LZHUF_OK;
}

struct BitReaderContext {
    u32 bit_off;
    const char *buf;
    u32 buf_size;
};

static s32 bit_reader(void *_read_ctx, u32 bits) {
    // Reads the most-significant bits first ("left to right")

    struct BitReaderContext *ctx = _read_ctx;

    u32 val = 0;

    for (u32 i = 0; i < bits; i++) {
        u32 byte_off = ctx->bit_off / 8;
        u32 bit = ctx->bit_off % 8;

        if (byte_off >= ctx->buf_size) {
            // can't read any more bytes
            // pretend it's 0
            val = val << 1;
        } else {
            val = val << 1;
            if ((ctx->buf[byte_off] & (1 << (7 - bit))) != 0) {
                val |= 1;
            }
        }

        ctx->bit_off += 1;
    }

    return val;
}

size_t lzhuf_decode_buffers_size() {
    return sizeof(struct LzhufDecodeBuffers);
}

WARN_UNUSED
int lzhuf_decode(struct LzhufDecodeBuffers *buffers, const char *in, size_t in_size, char *out, size_t out_size, size_t *decoded_size) {
    struct BitReaderContext ctx = { .bit_off = 0, .buf = in, .buf_size = in_size };
    return lzhuf_decode_impl(buffers, &ctx, bit_reader, out, out_size, decoded_size);
}

#if ENABLE_TEST_SUITE

#define CHECK_EXPECTEDS(buf, expecteds, bits) { \
    struct BitReaderContext ctx = { .bit_off = 0, .buf = buf, .buf_size = sizeof(buf) }; \
    for (u32 i = 0; i < sizeof(expecteds)/sizeof(s32); i++) { ASSERT_EQ(bit_reader(&ctx, bits), expecteds[i]); } \
}

TEST_SUITE(lzhuf_decode) {
    TEST("bit_reader") {
        char buf[] = { 0xCA, 0xDD };
        s32 expecteds_1[] = { 1, 1, 0, 0, 1, 0, 1, 0,    1, 1, 0, 1, 1, 1, 0, 1,    0, 0 };
        s32 expecteds_3[] = { 6, 2, 5, 5, 6, 4,  0 };
        s32 expecteds_4[] = { 0xC, 0xA, 0xD, 0xD, 0 };
        s32 expecteds_8[] = { 0xCA, 0xDD, 0, 0};
        s32 expecteds_9[] = { 0x195, 0x174, 0 };
        s32 expecteds_16[] = { 0xCADD, 0 };

        CHECK_EXPECTEDS(buf, expecteds_1, 1);
        CHECK_EXPECTEDS(buf, expecteds_3, 3);
        CHECK_EXPECTEDS(buf, expecteds_4, 4);
        CHECK_EXPECTEDS(buf, expecteds_8, 8);
        CHECK_EXPECTEDS(buf, expecteds_9, 9);
        CHECK_EXPECTEDS(buf, expecteds_16, 16);
    }

    // TODO - add some lzhuf_decode tests
}

#endif