/* Unit Tests */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "wildmatch.h"
#include "testing.h"

bool rmatch(const char *pat, const char *str);

void
test_rmatch(void)
{
  TEST_ASSERT_TRUE(rmatch("abc", "abc"));
  TEST_ASSERT_FALSE(rmatch("abc", "abz"));

  TEST_ASSERT_TRUE(rmatch("*.txt", "file.txt"));
  TEST_ASSERT_FALSE(rmatch("*.txt", "file.doc"));
  TEST_ASSERT_TRUE(rmatch("file-?.dat", "file-a.dat"));
  TEST_ASSERT_FALSE(rmatch("file-?.dat", "file-zz.dat"));

  TEST_ASSERT_TRUE(rmatch("", ""));
  TEST_ASSERT_TRUE(rmatch("*", ""));
  TEST_ASSERT_TRUE(rmatch("**", ""));
  TEST_ASSERT_FALSE(rmatch("?", ""));

  TEST_ASSERT_FALSE(rmatch("*?", ""));
  TEST_ASSERT_TRUE(rmatch("*?", "x"));
  TEST_ASSERT_TRUE(rmatch("*?", "xxx"));

  TEST_ASSERT_FALSE(rmatch("?*", ""));
  TEST_ASSERT_TRUE(rmatch("?*", "x"));
  TEST_ASSERT_TRUE(rmatch("?*", "xxx"));

  TEST_ASSERT_TRUE(rmatch("x**x", "xx"));
  TEST_ASSERT_TRUE(rmatch("x**x", "xAx"));
  TEST_ASSERT_TRUE(rmatch("x**x", "xAAx"));
  TEST_ASSERT_FALSE(rmatch("x**x", "xAAx."));

  TEST_ASSERT_FALSE(rmatch("a*x*b", "ab"));
  TEST_ASSERT_TRUE(rmatch("a*x*b", "abxbab"));
}

struct tests {
  const char *pat;
  const char *str;
  int flags, expected;
};

static void
tabletests(struct tests *tests)
{
  char buf[256];
  int i;
  for (i = 0; tests[i].pat; i++) {
    int r = wildmatch(tests[i].pat, tests[i].str, tests[i].flags);
    int x = tests[i].expected;
    if (r != x) {
      snprintf(buf, sizeof buf,
        "match pat=(%s), str=(%s), flags=%d -- r=%d x=%d",
        tests[i].pat, tests[i].str, tests[i].flags, r, x);
      test_fail(__FILE__, __LINE__, buf);
    }
  }
}

struct tests itests[] = {
  { "abc", "abc", 0, 1 },
  { "abc", "abz", 0, 0 },

  { "*.txt",      "file.txt",    0, 1 },
  { "*.txt",      "file.doc",    0, 0 },
  { "file-?.dat", "file-a.dat",  0, 1 },
  { "file-?.dat", "file-zz.dat", 0, 0 },

  { "",     "",      0,  true  },
  { "*",    "",      0,  true  },
  { "**",   "",      0,  true  },
  { "?",    "",      0,  false },

  { "?",    "x",     0,  true  },
  { "?",    "xx",    0,  false },
  { "*",    "x",     0,  true  },
  { "*",    "xx",    0,  true  },

  { "*?",   "",      0,  false },
  { "*?",   "x",     0,  true  },
  { "*?",   "xx",    0,  true  },
  { "*?",   "xxx",   0,  true  },

  { "?*",   "",      0,  false },
  { "?*",   "x",     0,  true  },
  { "?*",   "xxx",   0,  true  },

  { "x**x", "xx",    0,  true  },
  { "x**x", "xAx",   0,  true  },
  { "x**x", "xAAx",  0,  true  },
  { "x**x", "xAAx.", 0,  false },

  { "*x*",  "",      0,  false },
  { "*x*",  "x",     0,  true  },
  { "*x*",  "xx",    0,  true  },
  { "*x*",  "Zxx",   0,  true  },
  { "*x*",  "xZx",   0,  true  },
  { "*x*",  "xxZ",   0,  true  },
  { "*x*",  "ZZ",    0,  false },

  { "a*x*b", "ab",        0,  false },
  { "a*x*b", "abxbab",    0,  true  },
  { "s*no*", "salentino", 0,  true  },
  { "*sip*", "mississippi", 0, true  },
  { "-*-*-*-", "-foo-bar-baz-", 0, true },

  { 0, 0, 0, 0 }
};

void
test_imatch(void)
{
  tabletests(itests);
}

struct tests btests[] = {
  { "[abc]",        "a",    0, true  },
  { "x[abc]",       "xb",   0, true  },
  { "x[abc]z",      "xcz",  0, true  },
  { "?[!]-]*",      "-x-",  0, true  },
  { "?[!]-]*",      "-!-",  0, true  },
  { "?[!]-]*",      "---",  0, false },
  { "?[!]-]*",      "-]-",  0, false },
  { "[aA][bB][cC]", "AbC",  0, true  },
  { "a[!b].c",      "ab.c", 0, false },
  { "[*]/b",        "*/b",  0, true  },
  { "[*]/b",        "a/b",  0, false },
  { "[?]/b",        "?/b",  0, true  },
  { "[?]/b",        "a/b",  0, false },
  { "a[b",          "a[b",  0, true  }, /* unclosed cc: literal */
  { "-O[0123]",     "-O3",  0, true  },
  { "-O[0123]",     "-O4",  0, false },
  { 0, 0, 0, 0 }
};

void
test_imatch_brackets(void)
{
  tabletests(btests);
}

struct tests ftests[] = {
  { "abc",       "aBc",     WILD_CASEFOLD, true  },
  { "a[xy]b",    "aXb",     0,             false },
  { "a[xy]b",    "aXb",     WILD_CASEFOLD, true  },
  { "*X*[yY]?*", "xyz",     0,             false },
  { "*X*[yY]?*", "xyz",     WILD_CASEFOLD, true  },
  { "*X*[yY]?*", "-x-Y-z-", WILD_CASEFOLD, true  },
  { 0, 0, 0, 0 }
};

void
test_imatch_casefold(void)
{
  tabletests(ftests);
}

struct tests ptests[] = {
  { "foo/bar",  "foo/bar",   0,             true  },
  { "foo/bar",  "foo/bar",   WILD_PATHNAME, true  },
  { "*/*",      "foo/bar",   WILD_PATHNAME, true  },
  { "*/bar",    "/bar",      WILD_PATHNAME, true  },
  { "foo/*",    "foo/",      WILD_PATHNAME, true  },
  { "*",        "foo/bar",   WILD_PATHNAME, false },
  { "/f/bar/x", "/f/baz/x",  WILD_PATHNAME, false },

  { "a?b",      "a/b",       0,             true  },
  { "a?b",      "a/b",       WILD_PATHNAME, false },
  { "a*b",      "a/b",       0,             true  },
  { "a*b",      "a/b",       WILD_PATHNAME, false },
  { "a[/]b",    "a/b",       0,             true  },
  { "a[/]b",    "a/b",       WILD_PATHNAME, false },
  { "*[/]b",    "a/b",       WILD_PATHNAME, false },
  { "*[b]",     "a/b",       WILD_PATHNAME, false },
  { "???",      "a/b",       0,             true  },
  { "???",      "a/b",       WILD_PATHNAME, false },

  { "a[b/c]*",  "a/z",       0,             true  },
  { "a[b/c]*",  "a/z",       WILD_PATHNAME, false },
  { "foo/*.c",  "foo/bar.c", WILD_PATHNAME, true  },
  { "foo*.c",   "foo/bar.c", WILD_PATHNAME, false },

  { "/a/b/c/",  "/a/b/c/",   WILD_PATHNAME, true  },
  { "/*/*/*/",  "/a/b/c/",   WILD_PATHNAME, true  },
  { "/?/?/?/",  "/a/b/c/",   WILD_PATHNAME, true  },
  { "/*/*/*/",  "////",      WILD_PATHNAME, true  },
  { "/*/*/*/",  "////",      0,             true  },
  { "//***//",  "////",      WILD_PATHNAME, true  },

  { "**/foo",   "/foo",      0,             true  },
  { "**/foo",   "a/foo",     WILD_PATHNAME, true  },
  { "**/foo",   "a/b/c/foo", WILD_PATHNAME, true  },
  { "*/foo",    "a/b/c/foo", WILD_PATHNAME, false },
  { "*/foo",    "a/b/c/foo", 0,             true  },
  { "foo/**",   "foo/",      WILD_PATHNAME, true  },
  { "foo/**",   "foo/a",     WILD_PATHNAME, true  },
  { "foo/**",   "foo/a/b/c", WILD_PATHNAME, true  },
  { "foo/*",    "foo/a/b/c", WILD_PATHNAME, false },
  { "foo/*",    "foo/a/b/c", 0,             true  },
  { "a/**/b",   "a/b",       0,             false },
  { "a/**/b",   "a/b",       WILD_PATHNAME, true  },
  { "a/**/b",   "a/x/b",     WILD_PATHNAME, true  },
  { "a/**/b",   "a/x/y/z/b", WILD_PATHNAME, true  },
  { "a/*/b",    "a/x/y/z/b", WILD_PATHNAME, false },
  { "a/*/b",    "a/x/z/y/b", 0,             true  },
  { "**/a*",    "a/b/ab",    WILD_PATHNAME, true  },
  { "**/a*",    "a/b/a/b",   WILD_PATHNAME, false },

  { "**/*/**",  "//",        WILD_PATHNAME, true  },
  { "**/*/**",  "a//b",      WILD_PATHNAME, true  },
  { "**/*/**",  "a/x/b",     WILD_PATHNAME, true  },
  { "**/*/**",  "a/x/y/b",   WILD_PATHNAME, true  }, /* sic: a/|x/|y/b */
  { "**/*/**",  "a/a//b/b",  WILD_PATHNAME, true  },

  { "**/a/*/b/***/c/*/d/**", "/a//b/c//d/", WILD_PATHNAME, true },
  { "**/a/*/b/***/c/*/d/**", "X/a/-/b/Y/c/-/d/Z", WILD_PATHNAME, true },
  { "**/a/*/b/***/c/*/d/**", "X/X/a/-/b/Y/Y/c/-/d/Z/Z", WILD_PATHNAME, true },

  { 0, 0, 0, 0 }
};

void
test_imatch_pathname(void)
{
  tabletests(ptests);
}

struct tests htests[] = {
  { "*.c",    ".foo.c",  0,                         true  },
  { "*.c",    "foo.c",   WILD_PERIOD,               true  },
  { "*.c",    ".foo.c",  WILD_PERIOD,               false },
  { ".*.c",   ".foo.c",  WILD_PERIOD,               true  },
  { "?foo",   ".foo",    WILD_PERIOD,               false },
  { "[.]foo", ".foo",    WILD_PERIOD,               false },
  /* wildcards match period in non-initial position */
  { "b?c",    "b.c",     WILD_PERIOD|WILD_PATHNAME, true  },
  { "b*c",    "b.c",     WILD_PERIOD|WILD_PATHNAME, true  },
  { "b[.]c",  "b.c",     WILD_PERIOD|WILD_PATHNAME, true  },
  /* but in initial position, only a literal dot matches */
  { "a/*",    "a/.b.c",  WILD_PERIOD,               true  },
  { "a/*",    "a/.b.c",  WILD_PERIOD|WILD_PATHNAME, false },
  { "a/?*",   "a/.b.c",  WILD_PERIOD,               true  },
  { "a/?*",   "a/.b.c",  WILD_PERIOD|WILD_PATHNAME, false },
  { "a/[.]*", "a/.b.c",  WILD_PERIOD,               true  },
  { "a/[.]*", "a/.b.c",  WILD_PERIOD|WILD_PATHNAME, false },
  { "*/*",    "a/.b.c",  WILD_PERIOD,               true  },
  { "*/*",    "a/.b.c",  WILD_PERIOD|WILD_PATHNAME, false },
  { "*/?*",   "a/.b.c",  WILD_PERIOD,               true  },
  { "*/?*",   "a/.b.c",  WILD_PERIOD|WILD_PATHNAME, false },
  { "*/[.]*", "a/.b.c",  WILD_PERIOD,               true  },
  { "*/[.]*", "a/.b.c",  WILD_PERIOD|WILD_PATHNAME, false },
  { "*/.?*",  "a/.b.c",  WILD_PERIOD|WILD_PATHNAME, true  },
  { 0, 0, 0, 0 }
};

void
test_imatch_period(void)
{
  tabletests(htests);
}

/* assume this file is UTF-8 encoded */
struct tests utests[] = {
  { "“ä-ö-ü-€”",  "“ä-ö-ü-€”",  0,  true },
  { "“?-?-?-?”",  "“ä-ö-ü-€”",  0,  true },
  { "?*€?",       "“ä-ö-ü-€”",  0,  true },
  { "?*[•€]?",    "“ä-ö-ü-€”",  0,  true },
  { "П*й?*?й", "Пётр Ильи́ч Чайко́вский", 0, true },
  { 0, 0, 0, 0 }
};

void
test_imatch_utf(void)
{
  tabletests(utests);
}

int
main(void)
{
  int use_color = isatty(fileno(stdout));

  TEST_BEGIN(use_color);

  TEST_HEADING("Testing recursive wildcard match");
  TEST_RUN(test_rmatch);

  TEST_HEADING("Testing iterative wildcard match");
  TEST_RUN(test_imatch);
  TEST_RUN(test_imatch_brackets);
  TEST_RUN(test_imatch_casefold);
  TEST_RUN(test_imatch_pathname);
  TEST_RUN(test_imatch_period);
  TEST_RUN(test_imatch_utf);

  return TEST_END();
}
