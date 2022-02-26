
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* iterative wildcard matching */
/* with character classes and case folding */
/* with special logic for path names and dot files */

#include "../wildmatch.h"

static bool debug = 0;

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
  for (; *pat != ']'; pat++) {
    if (pat[0] == '-' && pat[1] != ']') {
      int lo = pat[-1], hi = pat[1];
      if ((lo <= sc && sc <= hi) ||
          (lo <= folded && folded <= hi))
        return !compl;
      pat++;
    }
    else if (*pat == sc || *pat == folded)
      return !compl;
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
imatch5(const char *pat, const char *str, int flags)
{
  const char *p, *s;
  const char *p0 = pat, *s0 = str;
  char pc, sc, folded;
  size_t n;
  bool fold = flags & WILD_CASEFOLD;
  bool path = flags & WILD_PATHNAME;
  bool hidden = flags & WILD_PERIOD;
  bool preslash, matchslash = false;

  if (hidden) {
    if (*str == '.' && *pat != '.')
      return false;
  }

  s = p = 0;

  for (;;) {
    if (debug)
      fprintf(stderr, "s=%s\tp=%s\n", str, pat);
    pc = *pat++;
    if (pc == '*') {
      matchslash = false;
      preslash = path && pat > p0+1 && pat[-2] == '/';
      while (*pat == '*') { matchslash = true; pat++; }
      if (preslash && matchslash && *pat == '/') pat++;
      /* set anchor (commits previous star) */
      p = pat;
      s = str;
      continue;
    }
    sc = *str++;
    if (sc == 0)
      return pc == 0 ? true : false;
    if (sc == '/' && sc != pc && path && !matchslash)
      return false;  /* only a slash can match a slash */
    if (sc == '.' && sc != pc && hidden && path && str > s0+1 && str[-2] == '/')
      return false;  /* only a literal dot can match a dot in initial position */
    folded = fold ? swapcase(sc) : sc;
    if (pc == '[' && (n = scanbrack(pat)) > 0) {
      if (matchbrack(pat, sc, folded)) pat += n;
      else if (s && *s == '/' && path && !matchslash)
        return false;  /* cannot stretch across slash */
      else if (!p) return false;
      else { pat = p; str = ++s; }
      continue;
    }
    if (pc != '?' && pc != sc && pc != folded) {
      if (s && *s == '/' && path && !matchslash)
        return false;  /* cannot stretch across slash */
      if (!p) return false;
      pat = p; str = ++s;
      continue;
    }
  }
}

int
wildmatch(const char *pat, const char *str, int flags)
{
  if (!pat || !str) return false;
  return imatch5(pat, str, flags);
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
  while ((opt = getopt(argc, argv, "dfFhHpP")) > 0) {
    switch (opt) {
      case 'd': debug = 1; break;
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
    fprintf(stderr, "Usage: %s [-dfFhHpP] <pat> <str1> ...\n", me);
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
