# Wildcard Pattern Matching

It is a feature found in many pieces of software:
match a string against a pattern that can contain
“wildcards”, usually `?` and `*`.

- a `?` matches any *single* character
- a `*` matches any *sequence* of characters,
  including the empty sequence
- all other characters match themselves

For example, the pattern `*.txt` matches any string that
ends with `.txt`, and `?*` matches any string that consists
of at least one character.

How can wildcard matching be implemented?

## Recursive Matching

There is a simple recursive algorithm for matching
a pattern *pat* against a string *str* as follows.
Look at the first characters of both *pat* and *str*;
if both are `\0` then we successfully matched; else
if they do not match, we have a mismatch; otherwise
recurse over the tails of *pat* and *str*.

With no wildcards, this is just a recursive
implementation of string equality:

```C
bool rmatch(const char *pat, const char *str)
{
  if (*pat == 0) return *str == 0;
  if (*pat != *str) return false;
  return rmatch(pat+1, str+1);
}
```

Add support for the `?` wildcard; the `?` always
matches as long as *str* still has a character:

```C
bool rmatch(const char *pat, const char *str)
{
  if (*pat == 0) return *str == 0;
  if (*pat == '?') {
    if (*str == 0) return false;
  }
  else if (*pat != *str) return false;
  return rmatch(pat+1, str+1);
}
```

Add support for the `*` wildcard; if we are at a `*`
in the pattern, see if the remainder of the pattern
matches ever shorter suffixes (including the empty
suffix) of the remaining string.

```C
bool rmatch(const char *pat, const char *str)
{
  if (*pat == 0) return *str == 0;
  if (*pat == '*') {
    size_t i, n = strlen(str);
    for (i = 0; i <= n; i++)
      if (rmatch(pat+1, str+i)) return true;
    return false;
  }
  if (*pat == '?') {
    if (*str == 0) return false;
  }
  else if (*pat != *str) return false;
  return rmatch(pat+1, str+1);
}
```

This implementation can be found in the
[recursive.c](./recursive.c) file.

Recursion depth can get as deep as the string is long,
but no deeper. Still this is highly undesirable and
calls for an iterative implementation.

## Iterative Matching

Any `*` in the pattern ends a segment of characters that
must match 1:1, whereas the `*` can “stretch” as needed
so as to find a match for the following segment.

The initial segment (before the first `*`) must match 1:1
and if it does not, the whole pattern does not match.

If any of the following segments does not match, try
again but resume one character later in the string
(conceptually we stretch the previous `*` by one character).

```C
bool imatch(const char *pat, const char *str)
{
  for (;;) {
    pc = *pat++;
    if (pc == '*') break;
    sc = *str++;
    if (sc == 0)
      return pc == 0 ? true : false;
    if (pc != '?' && pc != sc)
      return false;
  }

  p = pat; s = str;  // set anchor just after *

  for (;;) {
    pc = *pat++;
    if (pc == '*') {
      p = pat;       // set anchor just after *
      s = str;
      continue;
    }
    sc = *str++;
    if (sc == 0)
      return pc == 0 ? true : false;
    if (pc != '?' && pc != sc) {
      pat = p;       // resume at anchor in pattern
      str = ++s;     // but one later in string
      continue;
    }
  }
}
```

First `if` in second loop: when seeing an `*` in *pat*
this commits the current segment and we set an “anchor”
just after the `*` in *pat* (`p`) and at the current
location in *str* (`s`).

Last `if` in second loop: upon a mismatch, return to
the anchor in *pat* and one after the anchor in *str*
and try again. Here is room for optimization: e.g.,
after matching `*abcX` against `abcZ`, resuming with
`*abcX` against `bcZ` is on the safe side, but if there
is a match, it must be after the `Z` in *str*.

Likely there are other algorithms, but the one above
was “stretchy” enough for my mind. An implementation
can be found in the [iterative1.c](./iterative1.c) file.

## Bells and Whistles

- character classes like `[abc]`
- option to ignore case (case folding)
- path names: wildcards do not match `/` (path separator)
- path names: new wildcard `**` that does match `/`
- hidden files: leading dot is only matched by leading dot
- utf-8: multibyte encoding, possibility for invalid encoding

### Character Classes

Pattern characters enclosed in squere brackets like `[abc]`
are known as character classes. They match any single character
from the class; all other characters to not match.
Two characters in a class separated by a dash like `0-9` denote
the range of all characters between the two. The extension depends
on the character set, but at least with ASCII, e.g. `[0-9A-Fa-f]`
denotes the set of all hex characters `0123456789ABCDEFabcdef`.
To include a literal dash in the character class, place it at
the very beginning of the very end of the class. If a character
class begins with a `!` (in some implementations also with a `^`),
then the class matches any single character that is not in the class.

A practical consideration is how to deal with unclosed character
classes like `[abc`. While an error is a possible solution, it is
probably more useful to treat such constructs as ordinary pattern
characters. File [iterative2.c](./iterative2.c) has an implementation.

### Ignoring Case

Ignoring case is sometimes useful, but not always, so this is an
option, `CASEFOLD`. It is implemented by folding upper case to
lower case (or vice versa) before comparison.
An implementation can be found in [iterative3.c](./iterative3.c).

Two strategies: either, fold both pattern and string to, say,
lower case; or, compare original pattern against both upper'ed
and lower'ed string characters (one or the other must match).

### Path Name Specials

Wildcard pattern matching is most frequently used with file path
names, and in this case, it is more useful if the wildcards do
not match the directory separator (usually `/`): only a literal
`/` in the pattern can match a `/` in the string. For example,
we expect `*.txt` to match `file.txt`, but **not** to match
`../other/file.txt`. As with case folding, we make this an
option, `PATHNAME`, so the matching remains useful for other
applications.

Two strategies: either, modify the matching logic to look at
directory separators; or, match each path part separately.
The implementation in [iterative4.c](./iterative4.c) follows
the first strategy.

```text
str:   abcxy       abc/xy    abc/xy         the tilde ~
pat:   *~~xy       *~~xy     *~~/xy         indicates the
                                            stretching of
      stretch     but cannot stretch        the asterisk
      to match    across dirsep (pat
                  fails at '/' vs 'x')
```

With the `PATHNAME` option, we can now only match a fixed
number of directory parts in a path, e.g. `foo/*/*/bar`
would match `foo/x/y/bar` but not `foo/x/y/z/bar`. It is
therefore useful to have a new wildcard `**` that matches
any number (including zero) of directory parts in a path.
Because `**` shall match entire directory parts, this
wildcard can only occur as `**/`, `/**/`, and `/**`.
Without the `PATHNAME` option, two (or indeed any number
of) consecutive asterisks are equivalent to just one.

This complicates the code quite a bit because of details
such as `x/**/y` matching `x/y` (only one slash in string
but two in pattern) and sensible handling of more than two
stars (treat `***` as `**`).
The `goto` is a matter of taste but somehow fits in well.

### Hidden Files

By convention, files with names starting with a period
(aka dot files) are hidden on Unix systems, that is, the
`ls` command does not list them unless they are explicitly
named (or a special option is specified).
It is useful for a wildcard match to have an option `PERIOD`
that makes it exhibit this same behaviour: an initial period
(at start of pattern or immediately following a slash) can
only be matched by a literal dot in the pattern.
File [iterative5.c](./iterative5.c) contains an implementation.

### UTF-8

The wildcard matcher so far compares bytes, not characters.
For single-byte character sets like ASCII this is no problem.
But nowadays, Unicode, encoded with UTF-8, is prevalent.
[UTF-8][utf8] encodes each Unicode character in one or more
bytes. For example, `A` is encoded as `41` hex (one byte),
and `€` is encoded as `E2 82 AC` hex (three bytes).

For matching literal characters, no changes are needed: UTF-8
just works. However, wildcards are supposed to match characters,
not bytes, so must *decode* byte(s) to a character.

This means we replace assignments like `pc = *pat++` by calls
to a UTF-8 decoding routine, and also the `str = ++s` assignments
must must be changed to consider the number of bytes of the
character at `s`. The back references `str[-2]` look suspicious,
but indeed can stay unchanged, because we know the two previous
characters have been encoded in one byte. Not any sequence of
bytes is a valid UTF-8 encoding, so the decoding routine may
signal an error; in this case it is probably best to skip
forward to the next byte in range 0..127, that is, the next
valid starting byte in the UTF-8 sequence; alternatively,
we could return an error flag (instead of true/false).

The decoder could have a signature similar to

```c
int decode(void *buf, int *pc);
```

writing the character to `*pc` and returning the number
of bytes decoded, or 0 on end of input, or -1 on error.

An implementation can be found in [iterative.c](./iterative.c).

[utf8]: https://en.wikipedia.org/wiki/UTF-8

## Comparison to Regular Expressions

Wildcard pattern matching is different from and simpler
than regular expression (regex) matching. Wildcard patterns
can be expressed as regex patterns (but not the other way):

- the wildcard `?` corresponds to the regex `.`
- the wildcard `*` corresponds to the regex `.*`
- the wildcard `[...]` corresponds to the regex `[...]`
- the wildcard `[!...]` corresponds to the regex `[^...]`

## License

Dedicated to the public domain
(see the [UNLICENSE](./UNLICENSE) file).
