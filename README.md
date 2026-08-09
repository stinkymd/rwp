rwp - random word password
============================
rwp reads a wordlist and prints COUNT random words joined by SEPARATOR,
suitable for use as a password. It has no dependencies beyond libc and
/dev/urandom.

Words are selected by single-pass reservoir sampling, so memory use stays
constant regardless of the wordlist size.

Requirements
------------
In order to build rwp you need a C99 compiler and a POSIX system with
/dev/urandom.

Installation
------------
Edit config.h to match your local setup and afterwards enter the following
command to build and install rwp (if necessary as root):

    sudo make clean install

To uninstall:

    sudo make uninstall

PREFIX defaults to /usr/local and may be overridden; DESTDIR is honored
for packaging.

Running
-------
    usage: rwp [wordlist|-]

    wordlist    read words from this file (default: WORDLIST in config.h)
    -           read words from standard input

Examples:

    $ rwp
    asap risk webs toga twat

    $ rwp /usr/share/dict/words
    flaw pays nevi kelp fuck

    $ shuf /usr/share/dict/words | head -n 5000 | rwp -
    $ rwp | xclip -selection clipboard

Words shorter than MIN_LEN or longer than MAX_LEN are skipped; when
ONLY_ALPHA is set, only alphabetic words are accepted.

Configuration
-------------
rwp is configured at compile time by editing config.h. Options:

    WORDLIST    default wordlist path
    MIN_LEN     minimum accepted word length
    MAX_LEN     maximum accepted word length
    ONLY_ALPHA  1 accepts only alphabetic words, 0 accepts any word
    COUNT       number of words in the passphrase
    SEPARATOR   string placed between words
    CASE        case transformation per word (KEEP, LOWER, UPPER)
    PREFIX      string printed before the first word
    SUFFIX      string printed after the last word

Exit status
-----------
0 - success, the passphrase was printed.
1 - any error: unreadable wordlist, /dev/urandom failure, out of memory,
    or not enough matching words.
