# define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

char *pythonfmt(const char *format, const char *args) {
    char *pycmd;
    asprintf(&pycmd, "python -c 'print(\"%s\".format(%s), end=\"\")'", format, args);

    char *output = cmdout(pycmd);
    free(pycmd);
    return output;
}

char *rustfmt(const char *format, const char *args) {
    char tmpdir[] = "mfmt-XXXXXX";
    char *src = "fn main() { print!(\"%s\", %s); }";

    if(!mkdtemp(tmpdir)) { return NULL; }

    puts("allocating paths");
    char *srcfile, *prgfile, *rscmd;
    asprintf(&srcfile, "%s/prog.rs", tmpdir);
    asprintf(&prgfile, "%s/prog", tmpdir);
    asprintf(&rscmd, "rustc '%s' -o '%s'", srcfile, prgfile);

    printf("writing source file to '%s'\n", srcfile);
    FILE *f = fopen(srcfile, "w");
    fprintf(f, src, format, args);
    fclose(f);

    printf("compiling '%s'\n", rscmd);
    system(rscmd);

    printf("running '%s'\n", prgfile);
    char *output = cmdout(prgfile);

    puts("cleaning up");
    unlink(srcfile);
    unlink(prgfile);
    rmdir(tmpdir);

    free(srcfile);
    free(prgfile);
    free(rscmd);

    return output;
}

int main(void) {
    printf("rust:   '%s'\n", rustfmt("{} - {}", "\"foo\", \"bar\""));
    printf("python: '%s'\n", pythonfmt("{} - {}", "\"foo\", \"bar\""));
    return 0;
}
