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

/* Read sizeof(size_t) bytes to avoid breaking on massive wordlists */
static size_t
random_size_t(void)
{
        size_t n;

        if (fread(&n, sizeof(n), 1, urand) != 1)
                die("cannot read /dev/urandom");

        return n;
}

/* Flawless unbiased rejection sampling, now fully 64-bit safe */
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

        switch (CASE) {
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

int
main(int argc, char *argv[])
{
        FILE *fp;
        char *line = NULL;
        size_t linesize = 0;
        char **slot;
        size_t nvalid = 0;
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

        /* Safely allocate memory, die cleanly if the OS hates us */
        slot = ecalloc(COUNT, sizeof(*slot));

        /*
         * Single-pass reservoir sampling: keeps COUNT words uniformly
         * distributed without holding the whole wordlist in memory.
         */
        while (getline(&line, &linesize, fp) != -1) {
                size_t j;

                line[strcspn(line, "\n")] = '\0';

                if (!valid_word(line))
                        continue;

                apply_case(line);

                if (nvalid < COUNT) {
                        slot[nvalid] = estrdup(line);
                } else {
                        j = random_below(nvalid + 1);
                        if (j < COUNT) {
                                free(slot[j]);
                                slot[j] = estrdup(line);
                        }
                }

                nvalid++;
        }

        free(line);

        if (fp != stdin)
                fclose(fp);

        if (nvalid < COUNT)
                die("not enough words in list");

        fputs(PREFIX, stdout);

        for (i = 0; i < COUNT; i++) {
                if (i)
                        fputs(SEPARATOR, stdout);

                fputs(slot[i], stdout);
        }

        fputs(SUFFIX, stdout);
        putchar('\n');

        /* Valgrind will weep tears of joy */
        for (i = 0; i < COUNT; i++)
                free(slot[i]);

        free(slot);
        fclose(urand);

        return EXIT_SUCCESS;
}
