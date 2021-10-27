#include <stdarg.h>
#include <stdlib.h>

#include "../ext/tap.c/tap.c"

#include "../mfmt.c"

#define IS(spec, imax, expected) _is(__FILE__, __LINE__, spec, imax, expected)
#define SPEC &(mfmt_specification_t)

void _is(const char *file, int line, mfmt_specification_t *spec, intmax_t i, const char *expected) {
    char *buf = NULL;
    size_t buflen = 0;
    FILE *f = open_memstream(&buf, &buflen);
    mfmt_print_imax(f, spec, i);
    fclose(f);
    _tap_is_str(file, line, buf, expected, "%s", expected);
    free(buf);
}

int main(void) {

    tap_plan(8);

    {
        mfmt_specification_t spec = {0};
        IS(&spec, 42, "42");
    }
    {
        mfmt_specification_t spec = {
            .set = MFMT_SPEC_FIELD_SIGN,
            .sign = MFMT_SIGN_POSITIVE,
        };
        IS(&spec, 42, "+42");
    }
    {
        mfmt_specification_t spec = {
            .set = MFMT_SPEC_FIELD_WIDTH,
            .width = 5,
        };
        IS(&spec, 42, "   42");
    }
    {
        mfmt_specification_t spec = {
            .set = MFMT_SPEC_FIELD_WIDTH | MFMT_SPEC_FIELD_ZERO,
            .zero = 1,
            .width = 5,
        };
        IS(&spec, 42, "00042");
    }
    {
        mfmt_specification_t spec = {
            .set = MFMT_SPEC_FIELD_TYPE,
            .type = "o",
        };
        IS(&spec, 42, "52");
    }
    {
        mfmt_specification_t spec = {
            .set = MFMT_SPEC_FIELD_TYPE,
            .type = "b",
        };
        IS(&spec, 42, "101010");
    }
    {
        mfmt_specification_t spec = {
            .set = MFMT_SPEC_FIELD_TYPE,
            .type = "X",
        };
        IS(&spec, 42, "2A");
    }
    {
        mfmt_specification_t spec = {
            .set = MFMT_SPEC_FIELD_TYPE,
            .type = "x",
        };
        IS(&spec, 42, "2a");
    }

    return tap_finish();
}
