#include <string.h>
#include "int.h"

#define WARN_UNUSED __attribute__ ((warn_unused_result))
#define ALWAYS_INLINE __attribute__((always_inline))

#define BMP_SCN_ERRORS \
    X(BMP_SCN_ERROR_NO_MORE_INPUT, -1) \
    X(BMP_SCN_OK, 0)

#define X(error, code) error = code,
enum BmpScnErrors {
    BMP_SCN_ERRORS
};
#undef X

char *bmp_scn_errstr(int error_code) {
    #define X(error, code) if (error_code == code) { return #error; }
    BMP_SCN_ERRORS
    #undef X

    return "Unknown error code";
}

// We rely on aggressive inlining by the compiler. Don't let us down, compiler!

#define IN_IMPL(f) (({ _current_input = f(read_ctx); if (_current_input < 0) { return BMP_SCN_ERROR_NO_MORE_INPUT; } }), _current_input & 0xFF)
#define IN() IN_IMPL(read)
#define IN_PEEK() IN_IMPL(peek)
#define PLOT(x, y, v) plot(plot_ctx, x, y, v)

WARN_UNUSED
ALWAYS_INLINE
static inline int bmp_scn_decode_impl(void *read_ctx, void *plot_ctx, s32 (*read)(void *), s32 (*peek)(void *), void (*plot)(void*, s32 x, s32 y, u8 v)) {
    s32 _current_input;

    s32 x = 0;
    s32 y = 0;
    
    u8 first_byte = IN();

    while (1) {
        u8 input = IN();

        u8 code = input & 0xC0;

        if (code == 0x00) {
            // 00: Next line, offset negative X

            y += 1;
            x -= input & 0x3F;

            u8 tmp = IN_PEEK();

            if ((tmp & 0xC0) == 0x00 && (tmp & 0x3F) != 0) {
                // Offset negative X by more than 64
                tmp = IN() & 0x3F;
                x -= tmp * 64;
            }
        } else if (code == 0x40) {
            // 01: Offset positive X or end

            u8 off = input & 0x3F;
            x += off;

            if (off == 0) {
                // Done!
                return BMP_SCN_OK;
            }
        } else if (code == 0x80) {
            // 10: Repeated color (run-length encoded value)

            u8 count = input & 0x3F;
            u8 color = IN() + first_byte;
            for (s32 i = 0; i < count; i++) {
                PLOT(x+i, y, color);
            }
            x += count;
        } else /* if (code == 0xC0) */ {
            // 11: Packed colors

            u8 count = input & 0x3F;
            u32 i;
            for (i = 0; i < count/2*2; i += 2) {
                u8 v = IN();
                PLOT(x+i+0, y, (v >> 4) + first_byte);
                PLOT(x+i+1, y, (v & 0xF) + first_byte);
            }
            if ((count % 2) != 0) {
                u8 v = IN();
                PLOT(x+i+0, y, (v >> 4) + first_byte);
            }

            x += count;
        }
    }
}

#undef PLOT
#undef IN_PEEK
#undef IN
#undef IN_IMPL

struct BmpScnReadData {
    u32 offset;
    u32 size;
    const char *buf;
};

s32 bmp_scn_read_impl(void *_ctx) {
    struct BmpScnReadData *ctx = _ctx;

    if (ctx->offset >= ctx->size) {
        return -1;
    }

    u8 val = ctx->buf[ctx->offset];
    ctx->offset += 1;
    return val;
}

s32 bmp_scn_peek_impl(void *_ctx) {
    struct BmpScnReadData *ctx = _ctx;

    if (ctx->offset >= ctx->size) {
        return -1;
    }

    u8 val = ctx->buf[ctx->offset];
    return val;
}

struct ImagePaletteRgba8 {
    u8 pal[256][4];
};

struct BmpScnDecodeRgba8Args {
    char *buf;
    u32 stride;
    u32 max_off;
    const struct ImagePaletteRgba8 *pal;
    s32 draw_x, draw_y;
};

static void bmp_scn_decode_rgba8_plot(void *ctx, s32 x, s32 y, u8 val) {
    struct BmpScnDecodeRgba8Args *args = ctx;

    x += args->draw_x;
    y += args->draw_y;

    if (x >= 0 && y >= 0) { 
        u32 off = x*4 + y*args->stride;

        if (off >= 0 && off+4 <= args->max_off) {
            memcpy(args->buf + off, args->pal->pal + val, 4);
        }
    }
}

WARN_UNUSED
int bmp_scn_decode_rgba8(const void *in, u32 in_size, s32 x, s32 y, char *image_buf, u32 width, u32 height, const struct ImagePaletteRgba8 *pal) {
    struct BmpScnReadData read_ctx = {
        .offset = 0,
        .size = in_size,
        .buf = in
    };

    struct BmpScnDecodeRgba8Args args = {
        .buf = image_buf,
        .stride = width*4,
        .max_off = width*height*4,
        .pal = pal,
        .draw_x = x,
        .draw_y = y
    };

    return bmp_scn_decode_impl(&read_ctx, &args, bmp_scn_read_impl, bmp_scn_peek_impl, bmp_scn_decode_rgba8_plot);
}
