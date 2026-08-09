# rwp

Random Word Password — Suckless-style C / UNIX / POSIX / KISS.

Reads a wordlist, filters it, and prints a passphrase of `COUNT` randomly chosen
words joined by a separator. Data goes to stdout, diagnostics to stderr; there
are no prompts, flags, or interactive behavior. All tuning happens at compile
time in `config.h`. Selection uses single-pass reservoir sampling, so memory
use is bounded regardless of wordlist size.

Compose freely with other tools:

```
$ shuf /usr/share/dict/words | head -n 5000 | rwp -
```

## Usage

```
usage: rwp [wordlist|-]
```

- `wordlist` — read words from this file (default: `WORDLIST` in `config.h`).
- `-` — read words from standard input, enabling composition.

### Examples

```
$ rwp
asap risk webs toga twat

$ rwp | xclip -selection clipboard

$ rwp /usr/share/dict/words
flaw pays nevi kelp fuck

```

Words are filtered to alphabetic characters only and case-transformed per the
`CASE` setting. Lines of any length are handled (`getline`), so there is no
fixed word-size cap.

## Exit status

- `0` — success; the passphrase was printed.
- `1` — any error: unreadable wordlist, `/dev/urandom` failure, out of memory,
  or not enough matching words.

## Build and install

```
sudo make clean install

sudo make uninstall
```

`PREFIX` defaults to `/usr/local`; override with:

```
make PREFIX=$HOME/.local install
```

Installation honors `DESTDIR` for packaging.

## Configuration

Compile-time defaults live in `config.h`:

- `WORDLIST` — default wordlist path (e.g. `/usr/share/dict/words`).
- `MIN_LEN` / `MAX_LEN` — accepted word length range.
- `COUNT` — number of words in the passphrase.
- `SEPARATOR` — string placed between words.
- `CASE_BEHAVIOR` — case transformation per word.
- `PREFIX` — string printed before the first word.
- `SUFFIX` — string printed after the last word.

### Example: JSON output

```
#define SEPARATOR "\",\""
#define PREFIX "{\"password\":\""
#define SUFFIX "\"}"
```

Output:
```
{"password":"house","tiger","dumb","leafs"}
```

## Dependencies

- POSIX system with `/dev/urandom`
- C compiler (cc/gcc/clang)
- Standard C library (`libc`)

## License

MIT/X Consortium License
