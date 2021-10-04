/*
 * Copyright 2021 Andrew Gregory <andrew.gregory.8@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Project URL: http://github.com/andrewgregory/mFmt.c
 */

#ifndef MFMT_H
#define MFMT_H

#include <stddef.h>

const char mfmt_open  = '{';
const char mfmt_close = '}';

typedef enum mfmt_token_type_t {
    MFMT_TOKEN_LITERAL,
    MFMT_TOKEN_SUBSTITUTION,
} mfmt_token_type_t;

typedef struct mfmt_token_base_t {
    mfmt_token_type_t type;
} mfmt_token_base_t;

typedef struct mfmt_token_literal_t {
    mfmt_token_type_t type;
    char *string;
} mfmt_token_literal_t;

typedef struct mfmt_token_substitution_t {
    mfmt_token_type_t type;
    char *string;
    void *ctx;
} mfmt_token_substitution_t;

typedef union mfmt_token_t {
    mfmt_token_base_t base;
    mfmt_token_literal_t literal;
    mfmt_token_substitution_t substitution;
} mfmt_token_t;

typedef struct mfmt_t {
    size_t token_count;
    mfmt_token_t *tokens;
} mfmt_t;

#endif /* MFMT_H */
