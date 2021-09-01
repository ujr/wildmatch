/* Simple Unit Testing */

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <time.h>

#include "testing.h"

#define ANSIBLACK  "\033[30m"
#define ANSIRED    "\033[31m"
#define ANSIGREEN  "\033[32m"
#define ANSIYELLOW "\033[33m"
#define ANSIBLUE   "\033[34m"
#define ANSICYAN   "\033[36m"
#define ANSIWHITE  "\033[37m"
#define ANSIDEFLT  "\033[39m"  /* default foreground color */
#define ANSIBOLD   "\033[1m"
#define ANSIRESET  "\033[0m"

struct testing {
  int use_color;
  int abort_on_fail;
  const char *cur_test_name;
  int cur_test_failed;
  int num_tests;
  int num_failed;
  int num_ignored;
  jmp_buf jumpout;
};

static struct testing T;

void
test_begin(int use_color, int abort_on_fail)
{
  T.use_color = use_color;
  T.abort_on_fail = abort_on_fail;
  T.cur_test_name = 0;
  T.cur_test_failed = 0;
  T.num_tests = T.num_failed = T.num_ignored = 0;
}

int
test_end(void)
{
  int num_pass = T.num_tests - T.num_failed - T.num_ignored;
  if (T.use_color) {
    printf("%s%s", ANSIBOLD, ANSIDEFLT);
    if (T.num_failed > 0) {
      printf("%sOops:%s %d pass, %s%d fail%s, ",
        ANSIRED, ANSIDEFLT, num_pass, ANSIRED, T.num_failed, ANSIDEFLT);
    }
    else {
      printf("%sOK: %d pass%s, %d fail, ",
        ANSIGREEN, num_pass, ANSIDEFLT, T.num_failed);
    }
    printf("%s%d ignored", T.num_ignored > 0 ? ANSIYELLOW : "", T.num_ignored);
    printf("%s\n", ANSIRESET);
  }
  else printf("%s: %d pass, %d fail, %d ignored\n",
    T.num_failed > 0 ? "Oops" : "OK", num_pass, T.num_failed, T.num_ignored);
  return T.num_failed > 0 ? 1 : 0;
}

void
test_heading(const char *msg, const char *file, int line)
{
  if (!msg || !*msg) msg = "(heading)";
  if (T.use_color)
    printf("%s%s%s (%s:%d)\n", ANSIBOLD, msg, ANSIRESET, file, line);
  else
    printf("## %s (%s:%d)\n", msg, file, line);
  fflush(stdout);
}

void
test_run(test_fun fun, const char *name, const char *file, int line)
{
  const char *fmt;
  clock_t t0, t1;
  assert(fun);
  T.num_tests += 1;
  T.cur_test_name = name;
  T.cur_test_failed = 0;
  t0 = clock();
  if (setjmp(T.jumpout) == 0)
    fun();
  t1 = clock();
  if (T.cur_test_failed) {
    T.num_failed += 1;
    fmt = T.use_color ? "%s " ANSIRED "FAIL" ANSIRESET " (%ld ms) (%s:%d)\n" : "%s FAIL (%ld ms) (%s:%d)\n";
    printf(fmt, name, (long)(t1-t0), file, line);
  }
  else {
    fmt = T.use_color ? "%s " ANSIGREEN "PASS" ANSIRESET " (%ld ms) (%s:%d)\n" : "%s PASS (%ld ms) (%s:%d)\n";
    printf(fmt, name, (long)(t1-t0), file, line);
  }
  fflush(stdout);
}

void
test_ignore(const char *name, const char *file, int line)
{
  const char *fmt;
  T.num_tests += 1;
  T.num_ignored += 1;
  T.cur_test_name = name;
  fmt = T.use_color ? "%s " ANSIYELLOW "IGNORED" ANSIRESET " (%s:%d)\n" : "%s IGNORED (%s:%d)\n";
  printf(fmt, name, file, line);
  fflush(stdout);
}

void
test_fail(const char *msg, const char *file, int line)
{
  T.cur_test_failed = 1;
  printf("- %s failed (%s:%d)\n", msg, file, line);
  fflush(stdout);
  if (T.abort_on_fail)
    test_abort(0, file, line);
}

void
test_abort(const char *msg, const char *file, int line)
{
  T.cur_test_failed = 1; /* abort implies fail */
  if (msg) {
    printf("- %s (%s:%d)\n", msg, file, line);
    fflush(stdout);
  }
  longjmp(T.jumpout, 1);
}

void
test_info(const char *msg, const char *file, int line)
{
  const char *fmt;
  if (!msg) return;
  fmt = T.use_color ? ANSIBLUE "%s" ANSIRESET " (%s:%d)\n" : "%s (%s:%d)\n";
  printf(fmt, msg, file, line);
  fflush(stdout);
}
