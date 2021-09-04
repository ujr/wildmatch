
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* iterative wildcard matching */

static bool debug = 0;

bool
imatch(const char *pat, const char *str)
{
  const char *p, *s;
  char pc, sc;

  /* match up to first * in pat */

  for (;;) {
    pc = *pat++;
    if (pc == '*')
      break;
    sc = *str++;
    if (sc == 0)
      return pc == 0 ? true : false;
    if (pc != '?' && pc != sc)
      return false;
  }

  assert(pc == '*');

  /* match remaining segments:
     the * is an anchor where we return on mismatch */

  p = pat; s = str;

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
    if (pc != '?' && pc != sc) {
      pat = p;
      str = ++s;
      continue;
    }
  }

  assert(0); /* not reached */
}

#ifdef STANDALONE

bool
wildmatch(const char *pat, const char *str)
{
  if (!pat || !str) return false;
  return imatch(pat, str);
}

#include <stdio.h>

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
