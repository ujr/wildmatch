
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* iterative wildcard matching */
/* with character classes and case folding */
/* with special logic for path names */

#include "../wildmatch.h"

static bool debug = 0;

/** scan cclass, return length or 0 if not a cclass */
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

/** return true iff sc or folded occur in cclass at pat */
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

/** return c with case (lower/upper) swapped */
static int
swapcase(int c)
{
  int lc = tolower(c);
  return lc == c ? toupper(c) : lc;
}

/** return true iff pat ends with slash-star-star or equivalent */
static bool
isglobstar0(int pc, const char *pat)
{
  if (pc != '/') return false;
again:
  if (*pat == '*') pat++; else return false;
  if (*pat == '*') pat++; else return false;
  for (; *pat; pat++) {
    if (*pat == '/') { pat++; goto again; }
    if (*pat != '*') return false;
  }
  return true;
}

/** wildcard matching; return true iff str matches pat */
static bool
imatch4(const char *pat, const char *str, int flags)
{
  const char *p, *s, *t;
  const char *pat0 = pat;
  char pc, sc, folded;
  size_t n;
  bool fold = flags & WILD_CASEFOLD;
  bool path = flags & WILD_PATHNAME;
  bool matchslash = false;

  p = s = 0;
  for (;;) {
    if (debug)
      fprintf(stderr, "s=%s\tp=%s\n", str, pat);
    pc = *pat++;
    if (pc == '*') {
      if (*pat == '*') {
        const char *before = pat-2;
        while (*pat == '*') pat++;
        if (!path) matchslash = true;
        else if ((before < pat0 || *before == '/') &&
                (*pat == 0 || *pat == '/')) {
          if (*pat == 0) return true;  /* trailing ** matches anything */
          if (pat[1]) pat++;  /* skip non-trailing slash */
          while (*str) {
            if (imatch4(pat, str, flags)) return true;
            t = strchr(str, '/');
            if (t) str = t+1; else str += strlen(str);
          }
          return false;
        }
        else matchslash = false;
      }
      else matchslash = path ? false : true;
      /* set anchor (commits previous wild star) */
      p = pat; s = str;
      continue;
    }
    sc = *str++;
    if (sc == 0)
      return pc == 0 || isglobstar0(pc, pat) ? true : false;
    if (sc == '/' && pc != sc && !matchslash)
      return false;  /* only a slash can match a slash */
    folded = fold ? swapcase(sc) : sc;
    if (pc == '[' && (n = scanbrack(pat)) > 0) {
      if (matchbrack(pat, sc, folded)) pat += n;
      else if (s && *s == '/' && !matchslash)
        return false;  /* cannot stretch across slash */
      else if (!p) return false;
      else { pat = p; str = ++s; }
      continue;
    }
    if (pc != '?' && pc != sc && pc != folded) {
      if (s && *s == '/' && !matchslash)
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
  return imatch4(pat, str, flags);
}

#ifdef STANDALONE

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CHECK(pat, str, x) do { int r = !!wildmatch(pat, str, WILD_PATHNAME); \
  if (r != !!x) { s += 1; \
    fprintf(stderr, "FAIL r=%d x=%d pat=%s str=%s\n", r, x, pat, str); } \
  } while (0)

static int
runtests(void)
{
  int s = 0;

  fprintf(stderr, "Running tests...\n");

  CHECK("*",           "f",         true );
  CHECK("*",           "d/f",       false);
  CHECK("**",          "f",         true );
  CHECK("**",          "d/f",       true );
  CHECK("**",          "d/e/f",     true );

  /* leading or trailing slash must exist (useful for dir matching) */
  CHECK("**/",         "f",         false);
  CHECK("**/",         "d/f",       false);
  CHECK("**/",         "d/e/f",     false);
  CHECK("/**",         "f",         false);
  CHECK("/**",         "d/f",       false);
  CHECK("/**",         "d/e/f",     false);

  CHECK("**/f",        "f",         true );
  CHECK("**/f",        "d/f",       true );
  CHECK("**/f",        "d/e/f",     true );

  CHECK("d/**",        "d",         true );
  CHECK("d/**",        "d/e",       true );
  CHECK("d/**",        "d/e/f",     true );

  CHECK("a/**/b/**",   "ab",        false);
  CHECK("a/**/b/**",   "a/b",       true );
  CHECK("a/**/b/**",   "a/x/b/x",   true );
  CHECK("a/**/b/**",   "a/x/y/z/b", true );

  /* nasty: two stretchables in sequence must be merged or our algo cannot handle it */
  CHECK("**/*.x",      "f.x",       true );
  CHECK("**/*.x",      "d/f.x",     true );
  CHECK("**/*.x",      "d/e/f.x",   true );
  //CHECK("a/**/**/**/", "a/",        true );
  //CHECK("a/**/**/**/", "a/b/c/d/e/f/g/", true);

  /* nastier: stretchables cannot be merged, will resort to recursion */
  CHECK("**/a*",       "a/b/ab",    true );
  CHECK("a*/**/a*",    "a/b/ab",    true );

  CHECK("a/**/*/**/b", "a/b",       false);
  CHECK("a/**/*/**/b", "a//b",      true );
  CHECK("a/**/*/**/b", "a/x/y/z/b", true );

  CHECK("a/*/*/**/b",  "a/x/b",     false);
  CHECK("a/*/*/**/b",  "a/x/y/b",   true );
  CHECK("a/*/*/**/b",  "a/x/y/z/b", true );

  CHECK("a/*/**/*/b",  "a/x/b",     false);
  CHECK("a/*/**/*/b",  "a/x/y/b",   true );
  CHECK("a/*/**/*/b",  "a/x/y/z/b", true );
  CHECK("a/**/*/*/b",  "a/x/b",     false);
  CHECK("a/**/*/*/b",  "a/x/y/b",   true );
  CHECK("a/**/*/*/b",  "a/x/y/z/b", true );

  return s;
}

int
main(int argc, char *argv[])
{
  const char *me;
  const char *pat;
  int i, opt, flags = 0;

  if (argc <= 1)
    return runtests();

  me = argv[0];
  while ((opt = getopt(argc, argv, "dfFpP")) > 0) {
    switch (opt) {
      case 'd': debug = 1; break;
      case 'f': flags |= WILD_CASEFOLD; break;
      case 'F': flags &= ~WILD_CASEFOLD; break;
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
    fprintf(stderr, "Usage: %s [-dfFpP] <pat> <str1> ...\n", me);
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
