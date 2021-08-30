
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* recursive wildcard matching */
static bool
rmatch(const char *pat, const char *str)
{
  // end of pat only matches end of str:
  if (*pat == 0)
    return *str == 0;

  // match remainder of pat against ever shorter suffixes
  // of str, including the empty string (if i == n below):
  if (*pat == '*') {
    size_t i, n = strlen(str);
    for (i = 0; i <= n; i++)
      if (rmatch(pat+1, str+i))
        return true;
    return false;
  }

  // ? does not match the empty string:
  if (*pat == '?') {
    if (*str == 0) return false;
  }
  // compare literal characters:
  else if (*pat != *str) return false;

  // match remainder of pat against remainder of str:
  return rmatch(pat+1, str+1);
}

bool
wildmatch(const char *pat, const char *str)
{
  if (!pat || !str) return false;
  return rmatch(pat, str);
}

int
main(int argc, char *argv[])
{
  const char *pat;
  int i;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <pat> <str1> ...\n", argv[0]);
    return 127;
  }

  pat = argv[1];
  for (i = 2; i < argc; i++) {
    const char *str = argv[i];
    bool r = wildmatch(pat, str);
    printf("%s  %s\n", r ? "MATCHED " : "MISMATCH", str);
  }

  return 0;
}
