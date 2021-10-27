# define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "../mfmt.c"

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
    asprintf(&pycmd, "python -c 'print(\"{%s}\".format(%zd), end=\"\")'", format, i);

    char *output = cmdout(pycmd);

    free(pycmd);
    return output;
}

char *rustfmt(const char *format, intmax_t i) {
    char tmpdir[] = "mfmt-XXXXXX";
    char *src = "fn main() { print!(\"{%s}\", %zd); }";

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
    mfmt_specification_t *spec = mfmt_parse_specification(format + 1);

    /* dump_spec(format, spec); */
    if(spec == NULL || mfmt_print_imax(f, spec, i) == -1) {
        buf = strdup("<invalid>");
    }

    fclose(f);
    free(spec);

    return buf;
}

void compare(const char *format, intmax_t i) {
    puts("-----------------------");

    char *rout = rustfmt(format, i);
    char *pout = pythonfmt(format, i);
    char *mout = mfmt(format, i);

    if((!rout != !pout) || (rout && pout && strcmp(rout, pout) != 0)) { fputs("\033[31m", stdout); }
    printf("rust: format!(\"{%s}\", %zd) -> '%s'\n", format, i, rout ? rout : "<invalid>");
    printf("python: \"{%s}\".format(%zd) -> '%s'\n", format, i, pout ? pout : "<invalid>");
    printf("mfmt: mfmt(\"{%s}\", %zd)    -> '%s'\n", format, i, mout ? mout : "<invalid>");
    fputs("\033[0m", stdout);

    free(rout);
    free(pout);
    free(mout);
}

int main(void) {
    compare(":+5", 42);
    compare(":+05", 42);
    compare(":_>+5", 42);
    compare(":_<+5", 42);
    compare(":_>+05", 42);
    compare(":_<+05", 42);
    compare(":<+05", 42);
    compare(":>+05", 42);
    compare(":=+5", 42);
    compare(":_=+5", 42);
    compare(":=+05", 42);
    return 0;
}
