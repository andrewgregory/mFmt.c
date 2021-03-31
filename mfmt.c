#include <errno.h>

#include <mfmt.h>

mfmt_t mfmt_parse(const char *tmpl, mfmt_callback_t *cb, void *ctx) {
    mfmt_t *mfmt = calloc(sizeof(mft_t), 1);
    if(mfmt == NULL) { return NULL; }
    char *c;
    for(c = tmpl; *c; c++) {
        mfmt->token_count++;
        if(*c == '{' && *(c + 1) != '{') {
            /* replacement */
        } else {
            /* literal */
            while(*c) {
                while(*c && *c != '{') { c++; }
                if(*c == '{' && *(c + 1) == '{') { c += 2; }
            }
        }
    }

    if((mfmt->tokens = calloc(sizeof(mfmt_token_t), mfmt->token_count))) {
    }

    return mfmt;
}

size_t mfmt_printf(mfmt_t *mfmt, void *args, FILE *f) {
    size_t len = 0;
    size_t i;
    for(i = 0; i < mfmt->token_count; i++) {
        mfmt_token_t *t = mfmt->tokens[i];
        switch(t->type) {
            case MFMT_TOKEN_LITERAL:
                len += fputs(((mfmt_token_literal)t)->string, f);
                break;
            case MFMT_TOKEN_CALLBACK:
                len += mfmt->cb((const mfmt_token_callback_t)t, args, f, mfmt->context);
                break;
            default:
                errno = EINVAL;
                return -1;
        }
    }
    return len;
}

static size_t _mfmt_printf_close(mfmt_t *mfmt, void *args, FILE *f) {
    if(f) {
        size_t len = mfmt_printf(mfmt, args, f);
        fclose(f);
        return len;
    }
    return -1;
}

size_t mfmt_printd(mfmt_t *mfmt, void *args, int fd) {
    return _mfmt_printf_close(mfmt, args, fdopen(fd, "w"));
}

size_t mfmt_printb(mfmt_t *mfmt, void *args, char *buf, size_t buflen) {
    return _mfmt_printf_close(mfmt, args, fmemopen(buf, buflen, "w"));
}

size_t mfmt_prints(mfmt_t *mfmt, void *args, char **buf, size_t buflen) {
    return _mfmt_printf_close(mfmt, args, open_memstream(buf, buflen, "w"));
}

size_t mfmt_formatf(const char *tmple, mfmt_callback_t *cb, void ctx, void *args, FILE *f) {
    mfmt_t mfmt = mfmt_parse(tmple, cb, ctx);
    size_t len = mfmt_printf(mfmt, args, f);
    mfmt_free(mfmt);
    return len;
}
