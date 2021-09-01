/* Unit Tests */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

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

void
test_imatch(void)
{
  TEST_ASSERT_FAIL("not yet implemented");
}

int
main(void)
{
  int use_color = isatty(fileno(stdout));

  TEST_BEGIN(use_color);

  TEST_HEADING("Testing recursive wildcard match");
  TEST_RUN(test_rmatch);

  TEST_IGNORE(test_imatch);

  return TEST_END();
}
