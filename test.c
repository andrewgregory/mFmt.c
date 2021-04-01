#include <mfmt.c>

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++) {
        mfmt_t *mfmt = mfmt_parse(argv[i], NULL, NULL);
        if(mfmt) {
            printf("count: %zd\n", mfmt->token_count);
            for(size_t j = 0; j < mfmt->token_count; j++) {
                mfmt_token_t *t = &mfmt->tokens[j];
                switch(t->base.type) {
                    case MFMT_TOKEN_LITERAL:
                        printf("literal: '%s'\n", t->literal.string);
                        break;
                    case MFMT_TOKEN_CALLBACK:
                        printf("callback: '%s'\n", t->callback.name);
                        break;
                }
            }
        } else {
            perror("mfmt_parse");
        }
    }
    return 0;
}
