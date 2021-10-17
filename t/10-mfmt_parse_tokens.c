#define _GNU_SOURCE

#include <stdarg.h>
#include <stdlib.h>

#include "../ext/tap.c/tap.c"

#include "../mfmt.c"

//#define IS(template, ...) _is_tokens(__FILE__, __LINE__, template, __VA_ARGS__, NULL)
#define IS(template, ...) _is_tokens(template)

void _is_tokens(const char *template) {
    mfmt_t *mfmt = mfmt_parse_tokens(template);
    if(!mfmt) { perror("mfmt_parse_tokens"); return; }

    printf("template: '%s'\n", template);
    for(size_t i = 0; i < mfmt->token_count; i++) {
        mfmt_token_t *t = &mfmt->tokens[i];
        switch(t->base.type) {
            case MFMT_TOKEN_LITERAL:
                {
                    mfmt_token_literal_t *l = &t->literal;
                    printf("literal: '%s'\n", l->string);
                    break;
                }
            case MFMT_TOKEN_SUBSTITUTION:
                {
                    mfmt_token_substitution_t *s = &t->substitution;
                    printf("substitution: '%s'\n", s->string);
                    break;
                }
        }
    }
    putchar('\n');

    free(mfmt);
}

/* void _is_tokens(const char *file, int line, const char *template, ...) { */
/*     va_list ap; */
/*     va_start(ap, template); */

/*     mfmt_t *mfmt = mfmt_parse_tokens(template); */
/*     if(!tap_ok(mfmt != NULL, "parse '%s'", template)) { */
/*         int count = 0; */
/*         while(va_arg(ap, char *) != NULL) { count++; } */
/*         tap_skip(count, "parsing failed"); */
/*     } else { */
/*         size_t i = 0; */
/*         char *g, *e; */
/*         while((g = mfmt->tokens[i]) && (e = va_arg(ap, char *))) { */
            
/*             i++; */
/*         } */
/*         if(i < mfmt->token_count) { */
/*         } else if(e) { */
/*         } */
/*     } */

/*     va_end(ap); */
/* } */

int main(void) {
    mfmt_t *mfmt;

    tap_plan(7);

    // simple name-only tokens
    IS("foo", "foo");                                 // literal onlay
    IS("{{foo}}", "foo");                                 // literal onlay
    IS("{foo}", "{foo}");                             // tokan only
    IS("{foo}{bar}", "{foo}{bar}");                   // tokan only
    IS("{1}{2}", "{foo}{bar}");                   // tokan only
    IS("foo{}{}bar", "{foo}{bar}");                   // tokan only
    IS("foo {bar} baz", "foo ", "{bar}", " baz");     // leading trailing literal
    IS("{foo} bar {baz}", "{foo}", " bar ", "{baz}"); // leading/trailing token

    IS("{foo{bar}baz}", "{foo{bar}baz}"); // internal matched braced
    IS("{foo{{}", "{foo{}");              // escaped open brace
    IS("{foo}}}", "{foo}}");              // escaped close brace

    return tap_finish();
}
