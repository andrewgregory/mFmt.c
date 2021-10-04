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

#ifndef MFMT_C
#define MFMT_C

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "mfmt.h"

char *mfmt_strechrnul(const char *haystack, int needle) {
    const char *c;
    for(c = haystack; *c; c++) {
        if(*c == needle) {
            if(*(c + 1) != needle) { break; }
            else { c++; }
        }
    }
    return (char *)c;
}

void mfmt_dedup_char(char *haystack, char needle) {
    char *c = haystack, *end = haystack + strlen(haystack);
    while((c = strchr(c, needle))) {
        if(*(c + 1) == needle) {
            memmove(c, c + 1, end - c);
        }
        c++;
    }
}

void mfmt_dedup_braces(char *str) {
    mfmt_dedup_char(str, mfmt_open);
    mfmt_dedup_char(str, mfmt_close);
}

mfmt_t *mfmt_new(void) {
    return calloc(sizeof(mfmt_t), 1);
}

void mfmt_free(mfmt_t *mfmt) {
    free(mfmt);
}

mfmt_t *mfmt_parse_tokens(const char *tmpl) {
    mfmt_t *mfmt;

    if((mfmt = mfmt_new()) == NULL) { return NULL; }

    for(const char *c = tmpl; c && *c; ) {
        mfmt->token_count++;
        if(*c == mfmt_open && *(c + 1) != mfmt_open) {
            size_t depth = 1;
            c++;
            while(c && *c && depth) {
                if(*c == mfmt_open && *(c + 1) != mfmt_open) {
                    depth++; // internal brace open
                } else if(*c == mfmt_close && *(c + 1) != mfmt_close) {
                    depth--; // brace close
                } else if(*c == mfmt_open || *c == mfmt_close) {
                    c++; // skip escaped character
                }
                c++;
            }
            if(depth) {
                // unclosed brace
                free(mfmt);
                errno = EINVAL;
                return NULL;
            }
        } else {
            c = mfmt_strechrnul(c, mfmt_open);
        }
    }

    if((mfmt->tokens = calloc(sizeof(mfmt_token_t), mfmt->token_count)) == NULL) {
        free(mfmt);
        return NULL;
    }

    const char *c;
    size_t i;
    for(c = tmpl, i = 0; c && *c; i++) {
        if(*c == mfmt_open && *(c + 1) != mfmt_open) {
            /* substitution */
            mfmt_token_substitution_t *t = &mfmt->tokens[i].substitution;
            size_t depth = 1;
            const char *end = ++c;
            t->type = MFMT_TOKEN_SUBSTITUTION;

            while(end && *end && depth) {
                if(*end == mfmt_open && *(end + 1) != mfmt_open) {
                    depth++; // internal brace open
                } else if(*end == mfmt_close && *(end + 1) != mfmt_close) {
                    depth--; // brace close
                } else if(*end == mfmt_open || *end == mfmt_close) {
                    end++; // skip escaped character
                }
                end++;
            }

            if((t->string = strndup(c, end - c - 1)) == NULL) {
                free(mfmt);
                return NULL;
            }
            mfmt_dedup_braces(t->string);
            c = end;
        } else {
            /* literal */
            const char *end = mfmt_strechrnul(c, mfmt_open);
            mfmt_token_literal_t *t = &mfmt->tokens[i].literal;
            t->type = MFMT_TOKEN_LITERAL;
            if((t->string = strndup(c, end - c)) == NULL) {
                free(mfmt);
                return NULL;
            }
            mfmt_dedup_braces(t->string);
            c = end;
        }
    }
    if(i != mfmt->token_count) {
        mfmt_free(mfmt);
        errno = EINVAL;
        return NULL;
    }

    return mfmt;
}

#endif /* MFMT_C */
