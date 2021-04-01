#define _GNU_SOURCE

#include <time.h>

#include <mfmt.c>

const int limit = 10000000;

void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }

    return;
}

void print_time(const char *label, struct timespec *start, struct timespec *end) {
    struct timespec diff;
    timespec_diff(start, end, &diff);
    fprintf(stderr, "%15s: %ld.%.09ld\n", label, diff.tv_sec, diff.tv_nsec);
}

#define MFMT_VAL(v) { .name=NULL, .string=v }
#define MFMT_NVAL(n, v) { .name=n, .string=v }

void time_mfmt(void) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    mfmt_t *mfmt = mfmt_parse("foo {} baz {}\n", NULL, NULL);
    mfmt_val_t args[] = {
        MFMT_NVAL("quux", "xuuq"),
        MFMT_NVAL("bar", "rab"),
        NULL
    };
    for(int i = 0; i < limit; i++) {
        mfmt_mfmt(mfmt, args, stdout);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    print_time("time_mfmt", &start, &end);
}

void time_pfmt(void) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < limit; i++) {
        mfmt_fmt("foo {bar} baz {quux}\n",
            (mfmt_val_t[]) {
                { .name="quux", .string="xuuq" },
                { .name="bar", .string="rab" },
                NULL
            },
            stdout
        );
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    print_time("time_pfmt", &start, &end);
}

void time_nfmt(void) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < limit; i++) {
        mfmt_fmt("foo {} baz {}\n",
            (mfmt_val_t[]) {
                { .name="quux", .string="xuuq" },
                { .name="bar", .string="rab" },
                NULL
            },
            stdout
        );
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    print_time("time_nfmt", &start, &end);
}

void time_printf(void) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < limit; i++) {
        printf("foo %s baz %s\n", "rab", "xuuq");
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    print_time("time_printf", &start, &end);
}

int main(int argc, char *argv[]) {
    time_mfmt();
    /* time_nfmt(); */
    /* time_pfmt(); */
    time_printf();
    return 0;
}
