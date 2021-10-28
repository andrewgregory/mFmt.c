# define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "../mfmt.c"

#define TAP   1
#define TABLE 2
#define DIFF  3

int output = TAP;

char *cmdout(const char *cmd) {
    char *output = NULL;
    size_t olen = 0;

    FILE *out = open_memstream(&output, &olen);
    FILE *p = popen(cmd, "r");
    if(p == NULL) {
        perror("popen");
    }

    char buf[128];
    size_t blen;
    while((blen = fread(buf, 1, 128, p))) {
        fwrite(buf, blen, 1, out);
    }
    fclose(out);
    fclose(p);

    return output;
}

char *pythonfmt(const char *format, intmax_t i) {
    char *pycmd;
    asprintf(&pycmd, "python -c 'print(\"%s\".format(%zd), end=\"\")'", format, i);

    char *output = cmdout(pycmd);

    free(pycmd);
    return output;
}

char *rustfmt(const char *format, intmax_t i) {
    char tmpdir[] = "mfmt-XXXXXX";
    char *src = "fn main() { print!(\"%s\", %zd); }";

    if(!mkdtemp(tmpdir)) { return NULL; }

    char *srcfile, *prgfile, *rscmd;
    asprintf(&srcfile, "%s/prog.rs", tmpdir);
    asprintf(&prgfile, "%s/prog", tmpdir);
    asprintf(&rscmd, "rustc '%s' -o '%s' &>/dev/null", srcfile, prgfile);

    FILE *f = fopen(srcfile, "w");
    fprintf(f, src, format, i);
    fclose(f);


    char *output = NULL;
    if(system(rscmd) == 0) { output = cmdout(prgfile); }

    unlink(srcfile);
    unlink(prgfile);
    rmdir(tmpdir);

    free(srcfile);
    free(prgfile);
    free(rscmd);

    return output;
}

void dump_spec(const char *fmt, mfmt_specification_t *spec) {
    printf("spec: %s\n", fmt);
    if(spec) {
        printf("{\n"
               "    .fill = %c\n"
               "    .align = %c\n"
               "    .sign = %c\n"
               "    .grouping = %c\n"
               "    .alternate = %d\n"
               "    .zero = %d\n"
               "    .width = %zd\n"
               "    .precision = %zd\n"
               "    .type = %s\n"
               "    .set = %d\n"
               "}\n",
               spec->fill, spec->align, spec->sign, spec->grouping,
               spec->alternate, spec->zero, spec->width, spec->precision,
               spec->type, spec->set
          );
    } else {
        puts("<invalid>");
    }
}

char *mfmt(const char *format, intmax_t i) {
    char *buf = NULL;
    size_t buflen = 0;
    FILE *f = open_memstream(&buf, &buflen);

    mfmt_print(f, format, MFMT_NVAL(NULL, i));
    mfmt_print(f, "{:5}", MFMT_NVAL(NULL, "foo"));

    /* mfmt_specification_t *spec = mfmt_parse_specification(format + 1); */

    /* /1* dump_spec(format, spec); *1/ */
    /* if(spec == NULL || mfmt_print_imax(f, spec, i) == -1) { */
    /*     buf = strdup("<invalid>"); */
    /* } */

    /* free(spec); */

    fclose(f);
    return buf;
}

void compare(const char *format, intmax_t i) {

    char *rout = rustfmt(format, i);
    char *pout = pythonfmt(format, i);
    char *mout = mfmt(format, i);

    int rdiff = !mout != !rout || (mout && strcmp(mout, rout) != 0);
    int pdiff = !mout != !pout || (mout && strcmp(mout, pout) != 0);

    /* printf("mfmt: mfmt(\"{%s}\", %zd)    -> '%s'\n", format, i, mout ? mout : "<invalid>"); */
    /* printf("\033[%dm", !mout != !rout || (mout && strcmp(mout, rout) != 0) ? 31 : 0); */
    /* printf("rust: format!(\"{%s}\", %zd) -> '%s'\n", format, i, rout ? rout : "<invalid>"); */
    /* printf("\033[%dm", !mout != !pout || (mout && strcmp(mout, pout) != 0) ? 31 : 0); */
    /* printf("python: \"{%s}\".format(%zd) -> '%s'\n", format, i, pout ? pout : "<invalid>"); */
    /* fputs("\033[0m", stdout); */

    printf("+------------+------------+------------+------------+\n");
    printf("| %10s | %10s | %10s | %10s |%s\n", format,
            mout ? mout : "<invalid>",
            pout ? pout : "<invalid>",
            rout ? rout : "<invalid>",
            rdiff || pdiff ? "*" : "");

    free(rout);
    free(pout);
    free(mout);
}

int main(void) {
    printf("+------------+------------+------------+------------+\n");
    printf("|   Format   |   mFmt.c   |   Python   |    Rust    |\n");
    compare("{:+05}", 42);
    compare("{:_>+5}", 42);
    compare("{:0>+5}", 42);
    compare("{:_<+5}", 42);
    compare("{:_>+05}", 42);
    compare("{:_<+05}", 42);
    compare("{:<+05}", 42);
    compare("{:>+05}", 42);
    compare("{:=+5}", 42);
    compare("{:_=+5}", 42);
    compare("{:=+05}", 42);
    printf("+------------+------------+------------+------------+\n");
    return 0;
}
