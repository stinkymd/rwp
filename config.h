#ifndef CONFIG_H
#define CONFIG_H

enum {
	CASE_KEEP,
	CASE_LOWER,
	CASE_UPPER,
};

#define WORDLIST "/usr/share/dict/words"
#define MIN_LEN 4
#define MAX_LEN 4
#define ONLY_ALPHA 1 /* 1 = accept only alphabetic words, 0 = accept any word */
#define COUNT 5
#define SEPARATOR " "
#define CASE CASE_LOWER
#define PREFIX ""
#define SUFFIX ""

#endif
