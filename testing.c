/* Simple Unit Testing */

#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "testing.h"

#define ANSIOFF     "\033[0m"
#define ANSIBOLD    "\033[1m"
#define ANSIDIM     "\033[90m"

#define ANSIBLACK   "\033[30m"
#define ANSIRED     "\033[31m"
#define ANSIGREEN   "\033[32m"
#define ANSIYELLOW  "\033[33m"
#define ANSIBLUE    "\033[34m"
#define ANSIMAGENTA "\033[35m"
#define ANSICYAN    "\033[36m"
#define ANSIWHITE   "\033[37m"
#define ANSIDEFLT   "\033[39m"  /* default foreground color */

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

static void
print_loc_ln(const char *file, long line)
{
  if (T.use_color) printf("%s", ANSIDIM);
  printf("(%s:%ld)", file, line);
  if (T.use_color) printf("%s", ANSIOFF);
  printf("\n");
}

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
    printf("%s\n", ANSIOFF);
  }
  else printf("%s: %d pass, %d fail, %d ignored\n",
    T.num_failed > 0 ? "Oops" : "OK", num_pass, T.num_failed, T.num_ignored);
  return T.num_failed > 0 ? 1 : 0;
}

void
test_heading(const char *file, long line, const char *msg)
{
  if (!msg || !*msg) msg = "(heading)";
  if (T.use_color)
    printf("%s%s%s ", ANSIBOLD, msg, ANSIOFF);
  else
    printf("## %s ", msg);
  print_loc_ln(file, line);
  fflush(stdout);
}

void
test_run(const char *file, long line, test_fun fun, const char *name)
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
    fmt = T.use_color ? "%s " ANSIRED "FAIL" ANSIOFF " (%ld ms) " : "%s FAIL (%ld ms) ";
    printf(fmt, name, (long)(t1-t0));
    print_loc_ln(file, line);
  }
  else {
    fmt = T.use_color ? "%s " ANSIGREEN "PASS" ANSIOFF " (%ld ms) " : "%s PASS (%ld ms) ";
    printf(fmt, name, (long)(t1-t0));
    print_loc_ln(file, line);
  }
  fflush(stdout);
}

void
test_ignore(const char *file, long line, const char *name)
{
  const char *fmt;
  T.num_tests += 1;
  T.num_ignored += 1;
  T.cur_test_name = name;
  fmt = T.use_color ? "%s " ANSIYELLOW "IGNORED" ANSIOFF " " : "%s IGNORED ";
  printf(fmt, name);
  print_loc_ln(file, line);
  fflush(stdout);
}

void
test_fail(const char *file, long line, const char *msg)
{
  T.cur_test_failed = 1;
  printf("- %s failed ", msg);
  print_loc_ln(file, line);
  fflush(stdout);
  if (T.abort_on_fail)
    test_abort(file, line, 0);
}

void
test_abort(const char *file, long line, const char *msg)
{
  T.cur_test_failed = 1; /* abort implies fail */
  if (msg) {
    printf("- %s ", msg);
    print_loc_ln(file, line);
    fflush(stdout);
  }
  longjmp(T.jumpout, 1);
}

void
test_info(const char *file, long line, const char *fmt, ...)
{
  va_list ap;
  if (!fmt) return;
  if (T.use_color) printf("%s", ANSIBLUE);
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  if (T.use_color) printf(" %s", ANSIOFF);
  print_loc_ln(file, line);
  fflush(stdout);
}
