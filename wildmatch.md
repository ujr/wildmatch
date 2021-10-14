# Wildmatch Manual

The files [wildmatch.h](./wildmatch.h) and [wildmatch.c](./wildmatch.c)
implement wildcard matching. The interface consists of a single
function, `wildmatch(pat,str,flags)` returning true (non-zero)
if `pat` matches `str` and false (zero) otherwise.

All characters in the pattern match themselves, with the exception
of these wildcards:

- `*` match any sequence of zero or more characters
- `?` match any one character
- `[`...`]` match one character from the given class of characters
- `**` match zero or more directory names (only with PATHNAME option)

A character class can contain ranges like `0-9` as an
abbreviation for all characters with ordinal values between
the two boundaries (both inclusive).
If a character class begins with `!` or `^` it matches any
character *not* in the class. To include a closing bracket
in a character class, make it the first character in the
class like `[]abc]` or `[!]abc]`. An opening bracket may
occur at any place in the character class.

The *flags* parameter is either `0` or an additive combination
of these options:

- CASEFOLD to ignore case (only for letters in the ASCII range)
- PATHNAME to stop wildcards from matching `/` characters
- PERIOD to stop wildcards from matching leading `.` characters

The PATHNAME option is useful when matching file paths:
a pattern like `*.txt` will then match a filename, not an
an entire file path. With the PATHNAME option, the `**`
wildcard matches zero or more directory names (bounded
by `/` characters); it does not match partial directory
names.

The PERIOD option is used to *not* match files or directories
whose name begins with a period. Such files are also called
dot files and are “hidden” (not listed) on Unix-like systems.

Note that the backslash `\` is *not* an escape character
(as it is with fnmatch(3) by default); to turn off a
character's special meaning, put it in a character class.
