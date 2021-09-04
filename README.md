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

TODO

## Bells and Whistles

- character classes like `[abc]`
- option to ignore case (case folding)
- path names: wildcards do not match `/` (path separator)
- path names: new wildcard `**` that does match `/`
- hidden files: leading dot is only matched by leading dot
- utf-8: multibyte encoding, possibility for invalid encoding

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
