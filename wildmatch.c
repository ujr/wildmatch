
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

/* iterative wildcard matching */
/* with character classes and case folding */
/* with special logic for path names and dot files */

#include "wildmatch.h"

/* About UTF-8
 *
 * Value Range     First Byte Continuation Bytes
 *     0..127      0xxx xxxx
 *   128..2047     110x xxxx  10xx xxxx                        (1)
 *  2048..65535    1110 xxxx  10xx xxxx  10xx xxxx             (2)
 * 65536..1114111  1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx  (3)
 *
 * (1) continuation bytes are 10xx xxxx (6 bits payload)
 * (2) values 55296..57343 (UTF-16 surrogate pairs) are not allowed
 * (3) 1114111 = 10FFFF hex is the maximum value allowed
 *
 * For details see RFC 3629 and consult Wikipedia.
 *
 * The decoder below uses a table to get the payload from the
 * first byte, instead of switching on the first few bits
 * (this idea is from SQLite). Then it reads all continuation
 * bytes that follow, even if there are more than the first
 * byte mandates. Overlong encodings of 7bit characters are
 * recognised and replaced by U+FFFD (replacement character),
 * as are surrogate pairs 0xD800..0xDFFF, which are not allowed
 * in UTF-8. However, overlong encodings of larger values are
 * not detected and bytes 0x80..0xBF are returned as-is, even
 * though they are not valid UTF-8.
 */

/* Payload of 1st byte & 0x3F given the two hi bits are 11 */
static const unsigned char utf8tab[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  /* 110x xxxx */
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 110x xxxx */
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,  /* 110x xxxx */
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,  /* 110x xxxx */
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  /* 1110 xxxx */
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 1110 xxxx */
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  /* 1111 0xxx */
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,  /* 1111 10xx, 110x, 1110, 1111 */
};

static int
utf8get(const char **pp)
{
  const int replacement = 0xFFFD;
  const char *s = *pp;
  int c = (unsigned char) *s++;
  if (c >= 0xC0) {
    /* get payload from low 6 bits of first byte */
    c = utf8tab[c & 0x3F];
    /* ingest continuation bytes (10xx xxxx) */
    while ((*s & 0xC0) == 0x80) {
      c = (c << 6) + ((unsigned char) *s++ & 0x3F);
    }
    /* replace overlong 7bit encodings and surrogate pairs */
    if (c < 0x80 || (0xD800 <= c && c <= 0xDFFF)) {
      c = replacement;
    }
  }
  *pp = s;
  return c;
}

static size_t
scanbrack(const char *pat)
{
  /* assume opening bracket at pat[-1] */
  size_t n = 0;
  if (pat[n] == '!' || pat[n] == '^') n++; /* complement of class */
  if (pat[n] == ']') n++; /* ordinary at start of class */
  while (pat[n] && pat[n] != ']') n++; /* scan for end */
  return pat[n] ? n+1 : 0; /* return length if found, 0 if not */
}

static bool
matchbrack(const char *pat, int sc, int folded)
{
  int pc;
  bool compl = false;
  if (*pat == '!' || *pat == '^') {
    compl = true;
    pat++;
  }
  if (*pat == ']') {
    if (sc == ']')
      return !compl;
    pat++;
  }
  else if (*pat == '-') {
    if (sc == '-')
      return !compl;
    pat++;
  }
  for (pc = pat[-1]; *pat != ']'; ) {
    if (pat[0] == '-' && pat[1] != ']') {
      int lo, hi;
      pat++; /* skip the dash */
      lo=pc, hi=utf8get(&pat);
      if ((lo <= sc && sc <= hi) ||
          (lo <= folded && folded <= hi))
        return !compl;
    }
    else {
      pc = utf8get(&pat);
      if (pc == sc || pc == folded)
        return !compl;
    }
  }
  return compl;
}

static int
swapcase(int c)
{
  int lc = tolower(c);
  return lc == c ? toupper(c) : lc;
}

static bool
imatch(const char *pat, const char *str, int flags)
{
  const char *p, *s;
  const char *p0 = pat;
  int pc, sc, folded, prev;
  size_t n;
  bool fold = flags & WILD_CASEFOLD;
  bool path = flags & WILD_PATHNAME;
  bool hidden = flags & WILD_PERIOD;
  bool matchslash, preslash;

  if (hidden) {
    if (*str == '.' && *pat != '.')
      return false;
  }

  /* match up to first * in pat */

  pc = sc = prev = 0;

  for (;;) {
    pc = utf8get(&pat);
    if (pc == '*')
      goto entry;
    prev = sc;
    sc = utf8get(&str);
    if (sc == 0)
      return pc == 0 ? true : false;
    if (sc == '/' && sc != pc && path)
      return false;
    if (sc == '.' && sc != pc && hidden && path && prev == '/')
      return false;
    folded = fold ? swapcase(sc) : sc;
    if (pc == '[' && (n = scanbrack(pat)) > 0) {
      if (!matchbrack(pat, sc, folded))
        return false;
      pat += n;
    }
    else if (pc != '?' && pc != sc && pc != folded)
      return false;
  }

  assert(0); /* not reached */

  /* match remaining segments:
     the * is an anchor where we return on mismatch */

  for (;;) {
    pc = utf8get(&pat);
    if (pc == '*') {
entry:
      matchslash = false;
      preslash = path && pat > p0+1 && pat[-2] == '/';
      while (*pat == '*') { matchslash = true; pat++; }
      if (preslash && matchslash && *pat == '/') pat++;
      /* set anchor (commits previous star) */
      p = pat;
      s = str;
      continue;
    }
    prev = sc;
    sc = utf8get(&str);
    if (sc == 0)
      return pc == 0 ? true : false;
    if (sc == '/' && sc != pc && path && !matchslash)
      return false; /* only a slash can match a slash */
    if (sc == '.' && sc != pc && hidden && path && prev == '/')
      return false; /* only a literal dot can match an initial dot */
    folded = fold ? swapcase(sc) : sc;
    if (pc == '[' && (n = scanbrack(pat)) > 0) {
      if (!matchbrack(pat, sc, folded)) {
        if (*s == '/' && path && !matchslash)
          return false; /* cannot stretch across slash */
        pat = p;
        (void) utf8get(&s);
        str = s;
        prev = 0;
      }
      else pat += n;
      continue;
    }
    if (pc != '?' && pc != sc && pc != folded) {
      if (*s == '/' && path && !matchslash)
        return false; /* cannot stretch across slash */
      pat = p;
      (void) utf8get(&s);
      str = s;
      prev = 0;
      continue;
    }
  }

  assert(0); /* not reached */
}

int
wildmatch(const char *pat, const char *str, int flags)
{
  if (!pat || !str) return false;
  return imatch(pat, str, flags);
}
