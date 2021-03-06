
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* iterative wildcard matching */
/* with character classes and case folding */

#include "../wildmatch.h"

static bool debug = 0;

static size_t
scanbrack(const char *pat)
{
  /* assume opening bracket at pat[-1] */
  size_t n = 0;
  if (pat[n] == '!') n++; /* complement of character class */
  if (pat[n] == ']') n++; /* ordinary at start of class */
  while (pat[n] && pat[n] != ']') n++; /* scan for closing bracket */
  return pat[n] ? n+1 : 0; /* return length if found, 0 if not */
}

static int
matchbrack(const char *pat, int sc, int folded)
{
  int neg = 0;
  if (*pat == '!') {
    neg = 1;
    pat++;
  }
  if (*pat == ']') {
    if (sc == ']')
      return !neg;
    pat++;
  }
  else if (*pat == '-') {
    if (sc == '-')
      return !neg;
    pat++;
  }
  for (; *pat != ']'; pat++) {
    if (pat[0] == '-' && pat[1] != ']') {
      int lo = pat[-1], hi = pat[1];
      if ((lo <= sc && sc <= hi) ||
          (lo <= folded && folded <= hi))
        return !neg;
      pat++;
    }
    else if (*pat == sc || *pat == folded)
      return !neg;
  }
  return neg;
}

static int
swapcase(int c)
{
  int l = tolower(c);
  return l == c ? toupper(c) : l;
}

static bool
imatch3(const char *pat, const char *str, int flags)
{
  const char *p, *s;
  char pc, sc, folded;
  size_t n;
  bool fold = flags & WILD_CASEFOLD;

  p = s = 0;

  for (;;) {
    if (debug)
      fprintf(stderr, "s=%s\tp=%s\n", str, pat);
    pc = *pat++;
    if (pc == '*') {
      p = pat;
      s = str;
      continue;
    }
    sc = *str++;
    if (sc == 0)
      return pc == 0 ? true : false;
    folded = fold ? swapcase(sc) : sc;
    if (pc == '[' && (n = scanbrack(pat)) > 0) {
      if (matchbrack(pat, sc, folded)) pat += n;
      else if (!p) return false;
      else { pat = p; str = ++s; }
      continue;
    }
    if (pc != '?' && pc != sc && pc != folded) {
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
  return imatch3(pat, str, flags);
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
  while ((opt = getopt(argc, argv, "dfF")) > 0) {
    switch (opt) {
      case 'd': debug = 1; break;
      case 'f': flags |= WILD_CASEFOLD; break;
      case 'F': flags &= ~WILD_CASEFOLD; break;
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
