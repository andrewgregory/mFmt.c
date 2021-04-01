#include <stdarg.h>
#include <stdlib.h>

#include "../ext/tap.c/tap.c"

#include "../mfmt.c"

#define IS(g, ...) is(g, #g, __LINE__, __VA_ARGS__)

int main(void) {
    mfmt_t *mfmt;

    tap_plan(4);

    

    return tap_finish();
}
