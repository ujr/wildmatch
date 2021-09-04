/* Unit Tests */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "testing.h"

bool rmatch(const char *pat, const char *str);
bool imatch(const char *pat, const char *str);

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

  TEST_ASSERT_FALSE(imatch("a*x*b", "ab"));
  TEST_ASSERT_TRUE(imatch("a*x*b", "abxbab"));
  TEST_ASSERT_TRUE(imatch("s*no*", "salentino"));
}

void
test_imatch(void)
{
  TEST_ASSERT_TRUE(imatch("abc", "abc"));
  TEST_ASSERT_FALSE(imatch("abc", "abz"));

  TEST_ASSERT_TRUE(imatch("*.txt", "file.txt"));
  TEST_ASSERT_FALSE(imatch("*.txt", "file.doc"));
  TEST_ASSERT_TRUE(imatch("file-?.dat", "file-a.dat"));
  TEST_ASSERT_FALSE(imatch("file-?.dat", "file-zz.dat"));

  TEST_ASSERT_TRUE(imatch("", ""));
  TEST_ASSERT_TRUE(imatch("*", ""));
  TEST_ASSERT_TRUE(imatch("**", ""));
  TEST_ASSERT_FALSE(imatch("?", ""));

  TEST_ASSERT_FALSE(imatch("*?", ""));
  TEST_ASSERT_TRUE(imatch("*?", "x"));
  TEST_ASSERT_TRUE(imatch("*?", "xxx"));

  TEST_ASSERT_FALSE(imatch("?*", ""));
  TEST_ASSERT_TRUE(imatch("?*", "x"));
  TEST_ASSERT_TRUE(imatch("?*", "xxx"));

  TEST_ASSERT_TRUE(imatch("x**x", "xx"));
  TEST_ASSERT_TRUE(imatch("x**x", "xAx"));
  TEST_ASSERT_TRUE(imatch("x**x", "xAAx"));
  TEST_ASSERT_FALSE(imatch("x**x", "xAAx."));

  TEST_ASSERT_FALSE(imatch("*x*", ""));
  TEST_ASSERT_TRUE(imatch("*x*", "x"));
  TEST_ASSERT_TRUE(imatch("*x*", "xx"));
  TEST_ASSERT_TRUE(imatch("*x*", "Zxx"));
  TEST_ASSERT_TRUE(imatch("*x*", "xZx"));
  TEST_ASSERT_TRUE(imatch("*x*", "xxZ"));
  TEST_ASSERT_FALSE(imatch("*x*", "ZZ"));

  TEST_ASSERT_FALSE(imatch("a*x*b", "ab"));
  TEST_ASSERT_TRUE(imatch("a*x*b", "abxbab"));
  TEST_ASSERT_TRUE(imatch("s*no*", "salentino"));
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

  return TEST_END();
}
