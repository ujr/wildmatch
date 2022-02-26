
#include <stdbool.h>
#include <stdio.h>

/* iterative wildcard matching */

static bool debug = 0;

bool
imatch1(const char *pat, const char *str)
{
  const char *p, *s;
  char pc, sc;

  p = s = 0;           /* anchor initially not set */

  for (;;) {
    if (debug)
      fprintf(stderr, "s=%s\tp=%s\n", str, pat);
    pc = *pat++;
    if (pc == '*') {
      p = pat;         /* set anchor just after wild star */
      s = str;
      continue;
    }
    sc = *str++;
    if (sc == 0)
      return pc == 0 ? true : false;
    if (pc != '?' && pc != sc) {
      if (!p)
        return false;
      pat = p;         /* resume at anchor in pattern */
      str = ++s;       /* but one later in string */
      continue;
    }
  }
}

#ifdef STANDALONE

bool
wildmatch(const char *pat, const char *str)
{
  if (!pat || !str) return false;
  return imatch1(pat, str);
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
