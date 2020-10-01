#ifndef DECODE_H
#define DECODE_H

#include <stddef.h>
#include "decode_errors.h"

size_t generic_decode_buffers_size();
int generic_decode(void *buffers, const char *in, size_t in_size, char *out, size_t max_out_size, size_t *decoded_size);

#endif