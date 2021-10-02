/* Unit Tests */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "wildmatch.h"
#include "testing.h"

bool rmatch(const char *pat, const char *str);
bool imatch(const char *pat, const char *str, int flags);

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

void
test_imatch(void)
{
  TEST_ASSERT_TRUE(imatch("abc", "abc", 0));
  TEST_ASSERT_FALSE(imatch("abc", "abz", 0));

  TEST_ASSERT_TRUE(imatch("*.txt", "file.txt", 0));
  TEST_ASSERT_FALSE(imatch("*.txt", "file.doc", 0));
  TEST_ASSERT_TRUE(imatch("file-?.dat", "file-a.dat", 0));
  TEST_ASSERT_FALSE(imatch("file-?.dat", "file-zz.dat", 0));

  TEST_ASSERT_TRUE(imatch("", "", 0));
  TEST_ASSERT_TRUE(imatch("*", "", 0));
  TEST_ASSERT_TRUE(imatch("**", "", 0));
  TEST_ASSERT_FALSE(imatch("?", "", 0));

  TEST_ASSERT_FALSE(imatch("*?", "", 0));
  TEST_ASSERT_TRUE(imatch("*?", "x", 0));
  TEST_ASSERT_TRUE(imatch("*?", "xxx", 0));

  TEST_ASSERT_FALSE(imatch("?*", "", 0));
  TEST_ASSERT_TRUE(imatch("?*", "x", 0));
  TEST_ASSERT_TRUE(imatch("?*", "xxx", 0));

  TEST_ASSERT_TRUE(imatch("x**x", "xx", 0));
  TEST_ASSERT_TRUE(imatch("x**x", "xAx", 0));
  TEST_ASSERT_TRUE(imatch("x**x", "xAAx", 0));
  TEST_ASSERT_FALSE(imatch("x**x", "xAAx.", 0));

  TEST_ASSERT_FALSE(imatch("*x*", "", 0));
  TEST_ASSERT_TRUE(imatch("*x*", "x", 0));
  TEST_ASSERT_TRUE(imatch("*x*", "xx", 0));
  TEST_ASSERT_TRUE(imatch("*x*", "Zxx", 0));
  TEST_ASSERT_TRUE(imatch("*x*", "xZx", 0));
  TEST_ASSERT_TRUE(imatch("*x*", "xxZ", 0));
  TEST_ASSERT_FALSE(imatch("*x*", "ZZ", 0));

  TEST_ASSERT_FALSE(imatch("a*x*b", "ab", 0));
  TEST_ASSERT_TRUE(imatch("a*x*b", "abxbab", 0));
  TEST_ASSERT_TRUE(imatch("s*no*", "salentino", 0));
}

void
test_imatch_brack(void)
{
  TEST_ASSERT_TRUE(imatch("[abc]", "a", 0));
  TEST_ASSERT_TRUE(imatch("x[abc]", "xb", 0));
  TEST_ASSERT_TRUE(imatch("x[abc]z", "xcz", 0));
  TEST_ASSERT_TRUE(imatch("?[!]-]*", "-x-", 0));
  TEST_ASSERT_TRUE(imatch("?[!]-]*", "-!-", 0));
  TEST_ASSERT_FALSE(imatch("?[!]-]*", "---", 0));
  TEST_ASSERT_FALSE(imatch("?[!]-]*", "-]-", 0));
  TEST_ASSERT_TRUE(imatch("[aA][bB][cC]", "AbC", 0));
  TEST_ASSERT_FALSE(imatch("a[!b].c", "ab.c", 0));
  TEST_ASSERT_TRUE(imatch("[*]/b", "*/b", 0));
  TEST_ASSERT_FALSE(imatch("[*]/b", "a/b", 0));
  TEST_ASSERT_TRUE(imatch("[?]/b", "?/b", 0));
  TEST_ASSERT_FALSE(imatch("[?]/b", "a/b", 0));
  TEST_ASSERT_TRUE(imatch("a[b", "a[b", 0)); /* unclosed cc: literal */
  TEST_ASSERT_TRUE(imatch("-O[0123]", "-O3", 0));
  TEST_ASSERT_FALSE(imatch("-O[0123]", "-O4", 0));
}

static struct {
  const char *pat;
  const char *str;
  int flags, expected;
} tests[] = {
  { "abc",       "aBc",     WILD_CASEFOLD, true },
  { "a[xy]b",    "aXb",     0,             false},
  { "a[xy]b",    "aXb",     WILD_CASEFOLD, true },
  { "*X*[yY]?*", "xyz",     0,             false},
  { "*X*[yY]?*", "xyz",     WILD_CASEFOLD, true },
  { "*X*[yY]?*", "-x-Y-z-", WILD_CASEFOLD, true },
};

void
test_imatch_fold(void)
{
  size_t numfail = 0;
  size_t i, n = sizeof(tests) / sizeof(tests[0]);
  for (i = 0; i < n; i++) {
    int r = imatch(tests[i].pat, tests[i].str, tests[i].flags);
    int x = tests[i].expected;
    if (r != x) {
      printf("match pat(%s) str=(%s) flags=%d -- r=%d x=%d FAILED\n",
        tests[i].pat, tests[i].str, tests[i].flags, r, x);
      numfail += 1;
    }
  }
  if (numfail > 0)
    TEST_ASSERT_FAIL("test failed");
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

  TEST_HEADING("Testing iterative wildcard match with brackets");
  TEST_RUN(test_imatch_brack);

  TEST_HEADING("Testing iterative matching with case folding");
  TEST_RUN(test_imatch_fold);

  return TEST_END();
}
