#ifndef DECODE_ERRORS_H
#define DECODE_ERRORS_H

#define DECODE_ERRORS \
    X(LZW_ERROR_DICTIONARY_FULL, -10) \
    X(LZW_ERROR_INSUFFICIENT_OUTPUT, -9) \
    X(LZW_ERROR_NO_MORE_INPUT, -8) \
    X(LZW_ERROR_BAD_TOKEN, -7) \
    X(LZHUF_ERROR_INSUFFICIENT_OUTPUT, -6) \
    X(DECODE_RLE_ERROR_UNIMPLEMENTED, -5) \
    X(DECODE_ERROR_UNKNOWN_TYPE, -4) \
    X(DECODE_ERROR_OUTPUT_NOT_LARGE_ENOUGH, -3) \
    X(DECODE_ERROR_NO_HEADER, -2) \
    X(DECODE_NOOP_ERROR_INSUFFICIENT_OUTPUT, -1) \
    X(DECODE_OK, 0) \

// The remainders are equivalent to DECODE_OK, but the error string will be "DECODE_OK"
#define LZW_OK 0
#define LZHUF_OK 0

#define X(error, code) error = code,
enum DecodeErrors {
    DECODE_ERRORS
};
#undef X

const char *decode_errstr(int error_code);

#endif