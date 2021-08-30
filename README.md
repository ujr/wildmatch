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
a pattern *pat* against a string *str* as follows:

1. if *pat* is empty but *str* is not, report mismatch
2. if first of *pat* is `*`, match remainder of *pat*
   against ever shorter suffixes of *str* until a
   match is found; if none is found, report mismatch
3. if first of *pat* is `?` but *str* is empty, report mismatch
4. if first of *pat* differs from first of *str*, report mismatch
5. match remainder of *pat* against remainder of *str*

The file [recursive.c](./recursive.c) has an implementation.

The recursion can get as deep as the string is long,
which is not very pleasing.

## Iterative Matching

TODO

## Bells and Whistles

- character classes like `[abc]`
- path names: wildcards do not match `/` (path separator)
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
