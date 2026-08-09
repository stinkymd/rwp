/* See LICENSE for license details. */
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

static FILE *urand;

static void
die(const char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        fprintf(stderr, "rwp: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fputc('\n', stderr);
        exit(EXIT_FAILURE);
}

/* Suckless-style memory allocation wrappers */
static void *
ecalloc(size_t nmemb, size_t size)
{
        void *p;

        p = calloc(nmemb, size);
        if (!p)
                die("out of memory");

        return p;
}

static char *
estrdup(const char *s)
{
        char *p;

        p = strdup(s);
        if (!p)
                die("out of memory");

        return p;
}

static void
usage(void)
{
        fprintf(stderr, "usage: rwp [wordlist|-]\n");
        exit(EXIT_FAILURE);
}

/* Draw the full size_t width so rejection sampling stays unbiased for any n */
static size_t
random_size_t(void)
{
        size_t n;

        if (fread(&n, sizeof(n), 1, urand) != 1)
                die("cannot read /dev/urandom");

        return n;
}

/* Reject until r is below the largest multiple of n that fits in size_t */
static size_t
random_below(size_t n)
{
        size_t r;
        size_t limit;

        limit = SIZE_MAX - (SIZE_MAX % n);

        do {
                r = random_size_t();
        } while (r >= limit);

        return r % n;
}

static int
valid_word(const char *word)
{
        size_t len;
        const unsigned char *p;

        len = strlen(word);

        if (len < MIN_LEN || len > MAX_LEN)
                return 0;

#if ONLY_ALPHA
        for (p = (const unsigned char *)word; *p; p++)
                if (!isalpha(*p))
                        return 0;
#endif

        return 1;
}

static void
apply_case(char *word)
{
        unsigned char *p;

        switch (CASE_MODE) {
        case CASE_LOWER:
                for (p = (unsigned char *)word; *p; p++)
                        *p = (unsigned char)tolower(*p);
                break;
        case CASE_UPPER:
                for (p = (unsigned char *)word; *p; p++)
                        *p = (unsigned char)toupper(*p);
                break;
        case CASE_KEEP:
        default:
                break;
        }
}

/*
 * Single-pass reservoir sampling: keeps COUNT words uniformly
 * distributed without holding the whole wordlist in memory.
 * Returns the number of valid words seen.
 */
static size_t
fill_reservoir(FILE *fp, char **reservoir)
{
        char *line = NULL;
        size_t linesize = 0;
        size_t nvalid = 0;

        while (getline(&line, &linesize, fp) != -1) {
                size_t j;
                size_t nl;

                nl = strcspn(line, "\n");
                line[nl] = '\0';
                if (nl > 0 && line[nl - 1] == '\r')
                        line[nl - 1] = '\0';

                if (!valid_word(line))
                        continue;

                apply_case(line);

                if (nvalid < COUNT) {
                        reservoir[nvalid] = estrdup(line);
                } else {
                        j = random_below(nvalid + 1);
                        if (j < COUNT) {
                                free(reservoir[j]);
                                reservoir[j] = estrdup(line);
                        }
                }

                nvalid++;
        }

        if (ferror(fp))
                die("read error on input");

        free(line);

        return nvalid;
}

int
main(int argc, char *argv[])
{
        FILE *fp;
        char **reservoir;
        size_t nvalid;
        size_t i;

        if (argc > 2)
                usage();

        if (argc == 2 && strcmp(argv[1], "-") == 0) {
                fp = stdin;
        } else {
                const char *src = argc == 2 ? argv[1] : WORDLIST;

                fp = fopen(src, "r");
                if (!fp)
                        die("cannot open %s", src);
        }

        urand = fopen("/dev/urandom", "rb");
        if (!urand)
                die("cannot open /dev/urandom");

        /* Allocation failure is fatal; there is no recovery path */
        reservoir = ecalloc(COUNT, sizeof(*reservoir));

        nvalid = fill_reservoir(fp, reservoir);

        if (fp != stdin)
                fclose(fp);

        if (nvalid < COUNT)
                die("not enough words in list");

        fputs(PREFIX, stdout);

        for (i = 0; i < COUNT; i++) {
                if (i)
                        fputs(SEPARATOR, stdout);

                fputs(reservoir[i], stdout);
        }

        fputs(SUFFIX, stdout);
        putchar('\n');

        if (fflush(stdout) != 0 || ferror(stdout))
                die("write error on stdout");

        for (i = 0; i < COUNT; i++)
                free(reservoir[i]);

        free(reservoir);
        fclose(urand);

        return EXIT_SUCCESS;
}
