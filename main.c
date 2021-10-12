
#define _POSIX_C_SOURCE 2L  /* for getopt and fileno */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wildmatch.h"

int
main(int argc, char *argv[])
{
  const char *me;
  const char *pat;
  int i, opt, flags = 0;

  me = argv[0];
  while ((opt = getopt(argc, argv, "fFhHpP")) > 0) {
    switch (opt) {
      case 'f': flags |= WILD_CASEFOLD; break;
      case 'F': flags &= ~WILD_CASEFOLD; break;
      case 'h': flags |= WILD_PERIOD; break;
      case 'H': flags &= ~WILD_PERIOD; break;
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
    fprintf(stderr, "Usage: %s [-dfFhHpP] <pat> <str1> ...\n", me);
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
