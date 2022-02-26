
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* iterative wildcard matching */
/* with character classes */

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
matchbrack(const char *pat, int sc)
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
      if (lo <= sc && sc <= hi)
        return !neg;
      pat++;
    }
    else if (*pat == sc)
      return !neg;
  }
  return neg;
}

bool
imatch2(const char *pat, const char *str)
{
  const char *p, *s;
  char pc, sc;
  size_t n;

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
    if (pc == '[' && (n = scanbrack(pat)) > 0) {
      if (matchbrack(pat, sc)) pat += n;
      else if (!p) return false;
      else { pat = p; str = ++s; }
      continue;
    }
    if (pc != '?' && pc != sc) {
      if (!p) return false;
      pat = p; str = ++s;
      continue;
    }
  }
}

#ifdef STANDALONE

bool
wildmatch(const char *pat, const char *str)
{
  if (!pat || !str) return false;
  return imatch2(pat, str);
}

#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
  const char *me;
  const char *pat;
  int i;

  me = argv[0];
  if (argc > 1 && strcmp(argv[1], "-d") == 0) {
    debug = 1;
    argc--; argv++;
  }

  if (argc < 3) {
    fprintf(stderr, "Usage: %s [-d] <pat> <str1> ...\n", me);
    return 127;
  }

  pat = argv[1];
  for (i = 2; i < argc; i++) {
    const char *str = argv[i];
    bool r = wildmatch(pat, str);
    printf("%s  %s\n", r ? "MATCH   " : "MISMATCH", str);
  }

  return 0;
}

#endif
