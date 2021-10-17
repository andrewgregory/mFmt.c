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

/**
 * Locate a non-escaped delimiting character in a string
 *
 * Delimiter characters can be escaped by doubling them.
 *
 * @param haystack string to search
 * @param needle character to search for
 *
 * @return pointer to first character matching needle or NUL byte
 *
 * @code
 * char *c = mfmt_find_delim("foo $$ bar $", '$');
 * //    c points here ------------------^
 * @endcode
 */
char *mfmt_find_delim(const char *haystack, int needle) {
    const char *c;
    for(c = haystack; *c; c++) {
        if(*c == needle) {
            if(*(c + 1) != needle) { break; }
            else { c++; }
        }
    }
    return (char *)c;
}

char *mfmt_find_token_open(const char *haystack, int topen) {
    char *c = mfmt_find_delim(haystack, topen);
    return *c ? c : NULL;
}

/**
 * Search a string for a paired closing delimiting character.
 *
 * @param haystack string to search
 * @param topen character indicating the beginning of an internal pair
 * @param tclose character indicating the close of a pair
 *
 * @return pointer to first character matching needle or NUL byte
 *
 * @code
 * char *c = mfmt_find_token_close("foo {bar} baz}", '{', '}');
 * //    c points here --------------------------^
 * @endcode
 *
 * @note
 * \a haystack should not include the opening character.  An initial opening
 * character will be treated as an internal pair.
 */
char *mfmt_find_token_close(const char *haystack, int topen, int tclose) {
    const char *c = haystack;
    size_t depth = 1;
    while(*c && depth) {
        if(*c == tclose && *(c + 1) != tclose) {
            depth--; // token close
        } else if(*c == topen && *(c + 1) != topen) {
            depth++; // nested token open
        } else if(*c == topen || *c == tclose) {
            c++; // skip escaped character
        }
        c++;
    }
    return *c ? (char *)c : NULL;
}

void mfmt_unescape_delim(char *string, int delim) {
    char *c = string, *end = string + strlen(string);
    while((c = strchr(c, delim))) {
        if(*(c + 1) == delim) {
            memmove(c, c + 1, end - c);
        }
        c++;
    }
}

void mfmt_unescape_braces(char *str) {
    mfmt_unescape_delim(str, mfmt_open);
    mfmt_unescape_delim(str, mfmt_close);
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
            c = mfmt_find_delim(c, mfmt_open);
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
            mfmt_unescape_braces(t->string);
            c = end;
        } else {
            /* literal */
            const char *end = mfmt_find_delim(c, mfmt_open);
            mfmt_token_literal_t *t = &mfmt->tokens[i].literal;
            t->type = MFMT_TOKEN_LITERAL;
            if((t->string = strndup(c, end - c)) == NULL) {
                free(mfmt);
                return NULL;
            }
            mfmt_unescape_braces(t->string);
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

mfmt_specification_t *mfmt_parse_specification(const char *string) {
    return NULL;
}

size_t mfmt_print_char(FILE *f, int c, size_t count) {
    while(count--) { fputc(c, f); }
    return count;
}

size_t mfmt_print_string(FILE *f, mfmt_specification_t *spec, const char *string) {
    size_t len;

    if(spec->set & ~(MFMT_SPEC_FIELD_FILL | MFMT_SPEC_FIELD_ALIGN
                | MFMT_SPEC_FIELD_ZERO | MFMT_SPEC_FIELD_WIDTH
                | MFMT_SPEC_FIELD_PRECISION | MFMT_SPEC_FIELD_TYPE)) {
        errno = EINVAL;
        return 0;
    }

    len = strlen(string);
    if(spec->width < len) {
        int fill = spec->fill == '\0' ? ' ' : spec->fill;
        size_t fill_len = spec->width - len;
        switch(spec->align) {
            case MFMT_ALIGN_RIGHT: 
                return mfmt_print_char(f, fill, fill_len) + fputs(string, f);
                break;
            case MFMT_ALIGN_CENTER: 
                return mfmt_print_char(f, fill, fill_len / 2)
                    + fputs(string, f)
                    + mfmt_print_char(f, fill, fill_len - fill_len / 2);
                break;
            case MFMT_ALIGN_LEFT: 
            default:
                return fputs(string, f) + mfmt_print_char(f, fill, fill_len);
                break;
        }
    } else {
        return fputs(string, f);
    }
}

#endif /* MFMT_C */
