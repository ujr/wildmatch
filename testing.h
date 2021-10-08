/* Simple Unit Testing */

#ifndef TESTING_H
#define TESTING_H

typedef void (*test_fun)(void);

void test_begin(int abort_on_fail, int use_color);
int  test_end(void);  /* return 0 iff no tests failed */

void test_heading(const char *file, long line, const char *msg);
void test_run(const char *file, long line, test_fun fun, const char *name);
void test_ignore(const char *file, long line, const char *name);

void test_fail(const char *file, long line, const char *msg);
void test_abort(const char *file, long line, const char *msg);
void test_info(const char *file, long line, const char *fmt, ...);

#define TEST_BEGIN(...)  TEST_BEGIN_PRIV(__VA_ARGS__, 0, dummy)
#define TEST_BEGIN_PRIV(c, a, ...) test_begin((c), (a))
#define TEST_END()            test_end()

#define TEST_HEADING(msg)     test_heading(__FILE__, __LINE__, (msg))
#define TEST_RUN(fun)         test_run(__FILE__, __LINE__, (fun), #fun)
#define TEST_IGNORE(fun)      test_ignore(#fun, __FILE__, __LINE__)

#define TEST_ABORT(msg)       test_abort(__FILE__, __LINE__, (msg))
#define TEST_INFO(...)        test_info(__FILE__, __LINE__, __VA_ARGS__)

#define TEST_ASSERT_FAIL(msg) test_fail(__FILE__, __LINE__, (msg))
#define TEST_ASSERT_TRUE(x)   do{if(!(x)){test_fail(__FILE__,__LINE__,#x);}}while(0)
#define TEST_ASSERT_FALSE(x)  do{if( (x)){test_fail(__FILE__,__LINE__,#x);}}while(0)

#endif
