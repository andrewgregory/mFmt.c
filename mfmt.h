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
#include <stdio.h>

const char mfmt_open  = '{';
const char mfmt_close = '}';

typedef enum mfmt_token_type_t {
    MFMT_TOKEN_LITERAL,
    MFMT_TOKEN_SUBSTITUTION,
} mfmt_token_type_t;

typedef enum mfmt_align_t {
    MFMT_ALIGN_LEFT     = '<',
    MFMT_ALIGN_RIGHT    = '>',
    MFMT_ALIGN_CENTER   = '^',
    MFMT_ALIGN_INTERNAL = '=',
} mfmt_align_t;

typedef enum mfmt_sign_t {
    MFMT_SIGN_POSITIVE = '+',
    MFMT_SIGN_NEGATIVE = '-',
    MFMT_SIGN_SPACE    = ' ',
} mfmt_sign_t;

typedef enum mfmt_grouping_t {
    MFMT_GROUPING_COMMA      = ',',
    MFMT_GROUPING_UNDERSCORE = '_',
} mfmt_grouping_t;

typedef enum mfmt_specification_field_t {
    MFMT_SPEC_FIELD_FILL      = (1<<0),
    MFMT_SPEC_FIELD_ALIGN     = (1<<1),
    MFMT_SPEC_FIELD_SIGN      = (1<<2),
    MFMT_SPEC_FIELD_ALTERNATE = (1<<3),
    MFMT_SPEC_FIELD_ZERO      = (1<<4),
    MFMT_SPEC_FIELD_WIDTH     = (1<<5),
    MFMT_SPEC_FIELD_GROUPING  = (1<<6),
    MFMT_SPEC_FIELD_PRECISION = (1<<7),
    MFMT_SPEC_FIELD_TYPE      = (1<<8),
} mfmt_specification_field_t;

typedef struct mfmt_specification_t {
    char fill;
    mfmt_align_t align;
    mfmt_sign_t sign;
    mfmt_grouping_t grouping;
    int alternate;
    int zero;
    size_t width;
    size_t precision;
    char *type;

    int set;
} mfmt_specification_t;

typedef struct mfmt_token_t {
    mfmt_token_type_t type;
    char *string;
    void *ctx;
} mfmt_token_t;

typedef struct mfmt_t {
    size_t token_count;
    mfmt_token_t *tokens;
} mfmt_t;

ssize_t mfmt_print_string(FILE *f, mfmt_specification_t *spec, const char *string);

#endif /* MFMT_H */
