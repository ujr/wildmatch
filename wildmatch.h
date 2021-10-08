#ifndef WILDMATCH_H
#define WILDMATCH_H

#define WILD_CASEFOLD  1
#define WILD_PATHNAME  2
#define WILD_PERIOD    4

/** wildcard matching, supporting * ** ? [] */
int wildmatch(const char *pat, const char *str, int flags);

#endif
