#ifndef WILDMATCH_H
#define WILDMATCH_H

#define WILD_CASEFOLD 1
#define WILD_PATHNAME 2

int wildmatch(const char *pat, const char *str, int flags);

#endif
