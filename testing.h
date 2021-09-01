/* Simple Unit Testing */

#ifndef TESTING_H
#define TESTING_H

typedef void (*test_fun)(void);

void test_begin(int abort_on_fail, int use_color);
int  test_end(void); // return 0 iff no tests failed

void test_heading(const char *msg, const char *file, int line);
void test_run(test_fun fun, const char *name, const char *file, int line);
void test_ignore(const char *name, const char *file, int line);

void test_fail(const char *msg, const char *file, int line);
void test_abort(const char *msg, const char *file, int line);
void test_info(const char *msg, const char *file, int line);

#define TEST_BEGIN(...) TEST_BEGIN_PRIV(__VA_ARGS__, 0, dummy)
#define TEST_BEGIN_PRIV(c, a, ...) test_begin((c), (a))
#define TEST_END()         test_end()

#define TEST_HEADING(msg)     test_heading((msg), __FILE__, __LINE__)
#define TEST_RUN(fun)         test_run((fun), #fun, __FILE__, __LINE__)
#define TEST_IGNORE(fun)      test_ignore(#fun, __FILE__, __LINE__)

#define TEST_ABORT(msg)       test_abort((msg), __FILE__, __LINE__)
#define TEST_INFO(msg)        test_info((msg), __FILE__, __LINE__)

#define TEST_ASSERT_FAIL(msg) test_fail((msg), __FILE__, __LINE__)
#define TEST_ASSERT_TRUE(x)   do{if(!(x)){test_fail(#x,__FILE__,__LINE__);}}while(0)
#define TEST_ASSERT_FALSE(x)  do{if( (x)){test_fail(#x,__FILE__,__LINE__);}}while(0)

#endif
