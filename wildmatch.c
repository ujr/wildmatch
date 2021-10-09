
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
 *   First       Cont'n     Value
 *   Byte        Bytes      Range
 *   0xxx xxxx     0          0..127
 *   10xx xxxx   error                    (1)
 *   110x xxxx     1        128..2047
 *   1110 xxxx     2       2048..65535    (2)
 *   1111 0xxx     3      65536..1114111  (3)
 *   1111 10xx   error
 *
 * (1) continuation bytes are 10xx xxxx (6 bits payload)
 * (2) values 55296..57343 (UTF-16 surrogate pairs) are not allowed
 * (3) 1114111 = 10FFFF hex is the maximum value
 *
 * For details see RFC 3629 and consult Wikipedia.
 *
 * The decoder below is straightforward, but probably inefficient,
 * does not check continuation bytes, and does not reject overlong
 * sequences; look for better solutions, but not today.
 */

/** return nbytes, 0 on end, -1 on error */
static int
decode(const void *p, int *pc)
{
  const int replacement = 0xFFFD;
  const unsigned char *s = p;
  if (s[0] < 0x80) {
    *pc = s[0];
    return *pc ? 1 : 0;
  }
  if ((s[0] & 0xE0) == 0xC0) {
    *pc = (int)(s[0] & 0x1F) << 6
        | (int)(s[1] & 0x3F);
    return 2;
  }
  if ((s[0] & 0xF0) == 0xE0) {
    *pc = (int)(s[0] & 0x0F) << 12
        | (int)(s[1] & 0x3F) << 6
        | (int)(s[2] & 0x3F);
    /* surrogate pairs not allowed in UTF8 */
    if (0xD800 <= *pc && *pc <= 0xDFFF)
      *pc = replacement;
    return 3;
  }
  if ((s[0] & 0xF8) == 0xF0 && (s[0] <= 0xF4)) {
    /* 2nd cond: not greater than 0x10FFFF */
    *pc = (int)(s[0] & 0x07) << 18
        | (int)(s[1] & 0x3F) << 12
        | (int)(s[2] & 0x3F) << 6
        | (int)(s[3] & 0x3F);
    return 4;
  }
  *pc = replacement;
  /*errno = EILSEQ;*/
  return -1;
}

static size_t
scanbrack(const char *pat)
{
  /* assume opening bracket at pat[-1] */
  size_t n = 0;
  if (pat[n] == '!') n++; /* complement of class */
  if (pat[n] == ']') n++; /* ordinary at start of class */
  while (pat[n] && pat[n] != ']') n++; /* scan for end */
  return pat[n] ? n+1 : 0; /* return length if found, 0 if not */
}

static bool
matchbrack(const char *pat, int sc, int folded)
{
  int pc;
  bool compl = false;
  if (*pat == '!') {
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
  for (pc = pat[-1]; *pat != ']'; pat++) {
    if (pat[0] == '-' && pat[1] != ']') {
      int lo=pc, hi, n=decode(pat+1, &hi);
      if (n < 0) return false;
      pat += n;
      if ((lo <= sc && sc <= hi) ||
          (lo <= folded && folded <= hi))
        return !compl;
    }
    else {
      int n=decode(pat, &pc);
      if (n < 0) return false;
      pat += n-1;
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
  int pc, sc, folded, prev = 0, l;
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

  for (;;) {
    l = decode(pat, &pc);
    if (l < 0) return false;
    pat += l;
    if (pc == '*')
      goto entry;
    prev = sc;
    l = decode(str, &sc);
    if (l < 0) return false;
    str += l;
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
    l = decode(pat, &pc);
    if (l < 0) return false;
    pat += l;
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
    l = decode(str, &sc);
    if (l < 0) return false;
    str += l;
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
        str = s += decode(s, &pc);
        prev = 0;
      }
      else pat += n;
      continue;
    }
    if (pc != '?' && pc != sc && pc != folded) {
      if (*s == '/' && path && !matchslash)
        return false; /* cannot stretch across slash */
      pat = p;
      str = s += decode(s, &pc);
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

#ifdef STANDALONE

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
  const char *me;
  const char *pat;
  int i, opt, flags = 0;

  me = argv[0];
  while ((opt = getopt(argc, argv, "fFhHpP")) > 0) {
    switch (opt) {
      case 'f': flags |= WILD_CASEFOLD; break;
      case 'F': flags &= ~WILD_CASEFOLD; break;
      case 'h': flags |= WILD_PERIOD; break;
      case 'H': flags &= ~WILD_PERIOD; break;
      case 'p': flags |= WILD_PATHNAME; break;
      case 'P': flags &= ~WILD_PATHNAME; break;
      default:
        fprintf(stderr, "%s: invalid option: -%c\n", me, optopt);
        return 127;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s [-dfF] <pat> <str1> ...\n", me);
    return 127;
  }

  printf("Flags: %d\n", flags);

  pat = argv[0];
  for (i = 1; i < argc; i++) {
    const char *str = argv[i];
    bool r = wildmatch(pat, str, flags);
    printf("%s  %s\n", r ? "MATCH   " : "MISMATCH", str);
  }

  return 0;
}

#endif
