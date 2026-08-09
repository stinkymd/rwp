rwp - random word password
====================================
rwp is a suckless-style, C99/POSIX program that reads a wordlist and prints
a passphrase of `COUNT` random words joined by a separator, suitable for use
as a password. It has no dependencies beyond libc and `/dev/urandom`.

Data goes to stdout, diagnostics to stderr; there are no prompts, flags, or
interactive behavior. All tuning happens at compile time in `config.h`.
Selection uses single-pass reservoir sampling, so memory use is bounded
regardless of wordlist size. Compose freely with other tools:

    $ shuf /usr/share/dict/words | head -n 5000 | rwp -

    $ rwp | xclip -selection clipboard


Requirements
------------
In order to build rwp you need a C99 compiler (cc, gcc or clang) and a
POSIX system with `/dev/urandom`.


Installation
------------
Edit `config.h` to match your local setup and afterwards enter the
following command to build and install rwp (if necessary as root):

    sudo make clean install

To uninstall:

    sudo make uninstall

`PREFIX` defaults to `/usr/local` and can be overridden; installation
honors `DESTDIR` for packaging:

    make PREFIX=$HOME/.local install


Running
-------
usage: rwp [wordlist|-]

    wordlist    read words from this file (default: `WORDLIST` in config.h)
    -           read words from standard input, enabling composition

Examples:

    $ rwp
    asap risk webs toga twat

    $ rwp /usr/share/dict/words
    flaw pays nevi kelp fuck

Words are accepted per the config.h settings: the length range
(`MIN_LEN`/`MAX_LEN`) and, when `ONLY_ALPHA` is enabled, alphabetic
characters only. Each word is then case-transformed per the `CASE`
setting. Lines of any length are handled (`getline`), so there is no
fixed word-size cap.


Configuration
-------------
The configuration of rwp is done by creating a custom `config.h` and
(re)compiling the source code, which keeps it fast, secure and simple.

    WORDLIST    default wordlist path (e.g. /usr/share/dict/words)
    MIN_LEN     minimum accepted word length
    MAX_LEN     maximum accepted word length
    ONLY_ALPHA  1 accepts only alphabetic words, 0 accepts any word
    COUNT       number of words in the passphrase
    SEPARATOR   string placed between words
    CASE        case transformation per word
    PREFIX      string printed before the first word
    SUFFIX      string printed after the last word

For example, to emit a JSON document:

    #define SEPARATOR "\",\""
    #define PREFIX "{\"password\":\""
    #define SUFFIX "\"}"

yields:

    {"password":"house","tiger","dumb","leafs"}


Exit status
-----------
0 - success; the passphrase was printed.
1 - any error: unreadable wordlist, `/dev/urandom` failure, or not enough
    matching words.
