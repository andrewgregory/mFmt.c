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
#include <langinfo.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "mfmt.h"

/**
 * Locate a non-escaped delimiting character in a string.
 *
 * Delimiter characters can be escaped by doubling them.
 *
 * @param haystack string to search
 * @param needle character to search for
 *
 * @return pointer to first character matching needle or NUL byte
 *
 * @code
 *
 * char *c = mfmt_find_delim_nul("foo $$ bar $", '$');
 * // c points here -------------------------^
 *
 * @endcode
 */
char *mfmt_find_delim_nul(const char *haystack, int needle) {
    const char *c;
    for(c = haystack; *c; c++) {
        if(*c == needle) {
            if(*(c + 1) != needle) { break; }
            else { c++; }
        }
    }
    return (char *)c;
}

/**
 * Search a string for an opening delimiting character.
 *
 * @param haystack string to search
 * @param topen delimiter to search for
 *
 * @return pointer to first delimiter matching needle or NULL
 *
 * @code
 *
 * char *c = mfmt_find_token_close("{{foo {bar} baz}", '{');
 * // c points here ----------------------^
 *
 * @endcode
 */
char *mfmt_find_token_open(const char *haystack, int topen) {
    char *c = mfmt_find_delim_nul(haystack, topen);
    return *c ? c : NULL;
}

/**
 * Search a string for a paired closing delimiting character.
 *
 * @param haystack string to search
 * @param topen character indicating the beginning of an internal pair
 * @param tclose character indicating the close of a pair
 *
 * @return pointer to first delimiter matching needle or NULL
 *
 * @code
 *
 * char *c = mfmt_find_token_close("foo {bar} baz}", '{', '}');
 * // c points here -----------------------------^
 *
 * @endcode
 *
 * @note
 *
 * \a haystack should not include the opening character.  An initial opening
 * character will be treated as an internal pair.
 */
char *mfmt_find_token_close(const char *haystack, int topen, int tclose) {
    const char *c = haystack;
    size_t depth = 1;
    while(*c) {
        if(*c == tclose && *(c + 1) != tclose) {
            depth--; // token close
            if(depth == 0) { break; }
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

mfmt_t *mfmt_new(void) {
    return calloc(sizeof(mfmt_t), 1);
}

void mfmt_free(mfmt_t *mfmt) {
    if(mfmt) {
        free(mfmt->tokens);
        free(mfmt);
    }
}

static int _mfmt_find_token_end(const char *haystack,
        const int ropen, const int rclose,
        const char **next, mfmt_token_type_t *type) {
    char *delim = mfmt_find_delim_nul(haystack, ropen);
    if(delim != haystack) {
        /* literal */
        *next = delim;
        *type = MFMT_TOKEN_LITERAL;
        return 0;
    } else if((*next = mfmt_find_token_close(delim + 1, ropen, rclose))) {
        /* properly formed replacement */
        (*next)++; /* move to character after closing delimiter */
        *type = MFMT_TOKEN_SUBSTITUTION;
        return 0;
    } else {
        /* replacement without closing delimiter */
        errno = EINVAL;
        return -1;
    }
}

mfmt_t *mfmt_parse_tokens(const char *tmpl) {
    mfmt_t *mfmt;
    size_t token_count = 0;

    for(const char *c = tmpl, *next; *c; c = next) {
        mfmt_token_type_t ttype;
        if(_mfmt_find_token_end(c, mfmt_open, mfmt_close, &next, &ttype) != 0) {
            return NULL;
        }
        token_count++;
    }

    if((mfmt = mfmt_new()) == NULL) { return NULL; }
    if((mfmt->tokens = calloc(sizeof(mfmt_token_t), token_count)) == NULL) {
        mfmt_free(mfmt);
        return NULL;
    }

    for(const char *c = tmpl, *next; *c && mfmt->token_count < token_count; c = next) {
        mfmt_token_t *t = &mfmt->tokens[mfmt->token_count];
        if(_mfmt_find_token_end(c, mfmt_open, mfmt_close, &next, &t->type) != 0) {
            mfmt_free(mfmt);
            return NULL;
        }
        t->string =
            t->type == MFMT_TOKEN_LITERAL ? strndup(c, next - c)
            :                               strndup(c + 1, next - c - 2);

        mfmt_unescape_delim(t->string, mfmt_open);
        mfmt_unescape_delim(t->string, mfmt_close);
        mfmt->token_count++;
    }

    /* double check the number of assigned tokens matches initial count */
    if(token_count != mfmt->token_count) {
        mfmt_free(mfmt);
        errno = EINVAL;
        return NULL;
    }

    return mfmt;
}

static int _mfmt_parse_count(const char *string, size_t *ret, const char **end) {
    *end = string;
    *ret = 0;
    if(!(**end >= '0' && **end <= '9')) { errno = EINVAL; return -1; }
    do {
        size_t new = (*ret * 10) + (**end - '0');
        if(new < *ret) { errno = EOVERFLOW; return -1; }
        *ret = new;
        (*end)++;
    } while(**end >= '0' && **end <= '9');
    return 0;
}

mfmt_specification_t *mfmt_parse_specification(const char *string) {
    const char *c = string;
    mfmt_specification_t *spec = calloc(sizeof(mfmt_specification_t), 1);

    if(!spec) { return NULL; }

    if(c[0] != '\0' && (c[1] == '<' || c[1] == '^' || c[1] == '>' || c[1] == '=')) {
        spec->set |= MFMT_SPEC_FIELD_FILL | MFMT_SPEC_FIELD_ALIGN;
        spec->fill = *(c++);
        spec->align = *(c++);
    } else if(*c == '<' || *c == '^' || *c == '>' || *c == '=') {
        spec->set |= MFMT_SPEC_FIELD_ALIGN;
        spec->align = *(c++);
    }

    if(*c == '+' || *c == '-' || *c == ' ') {
        spec->set |= MFMT_SPEC_FIELD_SIGN;
        spec->sign = *(c++);
    }

    if(*c == '#') {
        spec->set |= MFMT_SPEC_FIELD_ALTERNATE;
        spec->alternate = 1;
        c++;
    }

    if(*c == '0') {
        spec->set |= MFMT_SPEC_FIELD_ZERO;
        spec->zero = 1;
        c++;
    }

    if(*c >= '0' && *c <= '9') {
        spec->set |= MFMT_SPEC_FIELD_WIDTH;
        if(_mfmt_parse_count(c, &spec->width, &c) != 0) {
            free(spec);
            return NULL;
        }
    }

    if(*c == ',' || *c == '_') {
        spec->set |= MFMT_SPEC_FIELD_GROUPING;
        spec->grouping = *(c++);
    }

    if(*c == '.') {
        spec->set |= MFMT_SPEC_FIELD_PRECISION;
        if(_mfmt_parse_count(c, &spec->precision, &c) != 0) {
            free(spec);
            return NULL;
        }
    }

    if(*c) {
        spec->set |= MFMT_SPEC_FIELD_TYPE;
        if((spec->type = strdup(c)) == NULL) {
            free(spec);
            return NULL;
        }
    }

    return spec;
}

ssize_t mfmt_print_imax(FILE *f, const mfmt_specification_t *spec, const intmax_t imax) {
    mfmt_sign_t sign = spec->set & MFMT_SPEC_FIELD_SIGN ? spec->sign : MFMT_SIGN_NEGATIVE;
    size_t width = spec->set & MFMT_SPEC_FIELD_WIDTH ? spec->width : 0;
    mfmt_align_t align
        = spec->set & MFMT_SPEC_FIELD_ZERO && spec->zero ? MFMT_ALIGN_INTERNAL
        : spec->set & MFMT_SPEC_FIELD_ALIGN              ? spec->align
        :                                                  MFMT_ALIGN_RIGHT;
    char fill
        = spec->set & MFMT_SPEC_FIELD_ZERO && spec->zero ? '0'
        : spec->set & MFMT_SPEC_FIELD_FILL               ? spec->fill
        :                                                  ' ';
    char *grouping
        = !(spec->set & MFMT_SPEC_FIELD_GROUPING) ? ""
        : spec->grouping == '_'                   ? "_"
        :                                           ",";

    size_t len, nlen;
    int base = 10;
    char *lchars = "0123456789abcdef";
    char *uchars = "0123456789ABCDEF";
    char *chars = lchars;

    if(spec->set & MFMT_SPEC_FIELD_TYPE) {
        char t = spec->type[0] != '\0' && spec->type[1] == '\0' ? spec->type[0] : '\0';
        switch(t) {
            case 'b': base = 2; break;
            case 'd': base = 10; break;
            case 'n': grouping = nl_langinfo(THOUSEP); break; /* TODO: thread unsafe */
            case 'o': base = 8; break;
            case 'X': chars = uchars; /* fall-through */
            case 'x': base = 16; break;
            case 'c': /* TODO? */
            default: errno = EINVAL; return -1;
        }
    }

    /* count characters */
    nlen = 1;
    for(intmax_t i = imax / base; i; i /= base) { nlen++; }
    len = nlen;
    if(sign != MFMT_SIGN_NEGATIVE || imax < 0) { len++; }

    if(width > len && (align == MFMT_ALIGN_RIGHT || align == MFMT_ALIGN_CENTER)) {
        size_t count = width - len;
        if(align == MFMT_ALIGN_CENTER) { count = count / 2; }
        while(count--) { if(fputc(fill, f) == EOF) { return -1; } }
    }

    char buf[nlen], *c = buf + nlen;
    if(imax < 0)                        { fputc('-', f); }
    else if(sign == MFMT_SIGN_POSITIVE) { fputc('+', f); }
    else if(sign == MFMT_SIGN_SPACE)    { fputc(' ', f); }

    if(width > len && align == MFMT_ALIGN_INTERNAL) {
        size_t count = width - len;
        while(count--) { if(fputc(fill, f) == EOF) { return -1; } }
    }

    for(intmax_t i = imax; i; i /= base) { *--c = chars[i % base]; }

    size_t bytes = fwrite(buf, 1, nlen, f);
    if(bytes != nlen) { return -1; }

    if(width > len && (align == MFMT_ALIGN_LEFT || align == MFMT_ALIGN_CENTER)) {
        size_t count = width - len;
        if(align == MFMT_ALIGN_CENTER) { count = width - count / 2; }
        while(count--) { if(fputc(fill, f) == EOF) { return -1; } }
    }

    return bytes;
}

ssize_t mfmt_print_string(FILE *f, const mfmt_specification_t *spec, const char *string) {
    size_t len = strlen(string);
    size_t wlen = spec->set & MFMT_SPEC_FIELD_PRECISION && spec->precision < len ? spec->precision : len;
    mfmt_align_t align
        = spec->set & MFMT_SPEC_FIELD_ALIGN              ? spec->align
        :                                                  MFMT_ALIGN_LEFT;

    if(spec->width > wlen && (align == MFMT_ALIGN_RIGHT || align == MFMT_ALIGN_CENTER)) {
        char fill = spec->set & MFMT_SPEC_FIELD_FILL ? spec->fill : ' ';
        size_t count = spec->width - wlen;
        if(align == MFMT_ALIGN_CENTER) { count = count / 2; }
        while(count--) { if(fputc(fill, f) == EOF) { return -1; } }
    }

    size_t bytes = fwrite(string, 1, wlen, f);
    if(bytes != wlen) { return -1; }

    if(spec->width > wlen && (align == MFMT_ALIGN_LEFT || align == MFMT_ALIGN_CENTER)) {
        char fill = spec->set & MFMT_SPEC_FIELD_FILL ? spec->fill : ' ';
        size_t count = spec->width - wlen;
        if(align == MFMT_ALIGN_CENTER) { count = spec->width - count / 2; }
        while(count--) { if(fputc(fill, f) == EOF) { return -1; } }
    }

    return spec->width > wlen ? spec->width : bytes;
}

mfmt_t *mfmt_compile(const char *format) {
    mfmt_t *mfmt = mfmt_parse_tokens(format);
    if(mfmt == NULL) { return NULL; }
    for(size_t i = 0; i < mfmt->token_count; i++) {
        mfmt_token_t *t = &mfmt->tokens[i];
        if(t->type == MFMT_TOKEN_SUBSTITUTION) {
            char *s = strchr(t->string, ':');
            if(s) {
                t->ctx = mfmt_parse_specification(s + 1);
                s = '\0';
            } else {
                t->ctx = calloc(sizeof(mfmt_specification_t), 1);
            }
        }
    }
    return mfmt;
}

ssize_t mfmt_print(FILE *f, const char *format, ...) {
    mfmt_t *mfmt = mfmt_compile(format);
    size_t ret = 0;

    va_list ap;
    va_start(ap, format);

    for(size_t i = 0; i < mfmt->token_count; i++) {
        mfmt_token_t *t = &mfmt->tokens[i];
        if(t->type == MFMT_TOKEN_LITERAL) {
            fputs(t->string, f);
        } else {
            mfmt_specification_t *spec = (mfmt_specification_t*)t->ctx;
            mfmt_val_t val = va_arg(ap, mfmt_val_t);

            switch(val.type) {
                case MFMT_VAL_TYPE_UINTMAX: return -1;
                case MFMT_VAL_TYPE_INTMAX: ret += mfmt_print_imax(f, spec, val.value.intmax); break;
                case MFMT_VAL_TYPE_STRING: ret += mfmt_print_string(f, spec, val.value.string); break;
            }
        }
    }

    mfmt_free(mfmt);

    return ret > SSIZE_MAX ? SSIZE_MAX : (ssize_t)ret;
}

#endif /* MFMT_C */
