/*
 * rwp - random word password generator
 * suckless / Unix / KISS
 *
 * Generates a passphrase of COUNT random words from a wordlist.
 * usage: rwp [wordlist|-]
 */

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

static void
usage(void)
{
	fprintf(stderr, "usage: rwp [wordlist|-]\n");
	exit(EXIT_FAILURE);
}

static uint32_t
random_u32(void)
{
	uint32_t n;

	if (fread(&n, sizeof(n), 1, urand) != 1)
		die("cannot read /dev/urandom");

	return n;
}

static size_t
random_below(size_t n)
{
	uint32_t r;
	uint32_t limit;

	if (n == 0)
		die("invalid random range");

	limit = UINT32_MAX - (UINT32_MAX % n);

	do {
		r = random_u32();
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

	slot = calloc(COUNT, sizeof(*slot));
	if (!slot)
		die("out of memory");

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
			slot[nvalid] = strdup(line);
			if (!slot[nvalid])
				die("out of memory");
		} else {
			j = random_below(nvalid + 1);
			if (j < COUNT) {
				free(slot[j]);
				slot[j] = strdup(line);
				if (!slot[j])
					die("out of memory");
			}
		}

		nvalid++;
	}

	free(line);

	if (fp != stdin)
		fclose(fp);

	if (nvalid == 0)
		die("no words matched filter");

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

	for (i = 0; i < COUNT; i++)
		free(slot[i]);

	free(slot);

	return EXIT_SUCCESS;
}
