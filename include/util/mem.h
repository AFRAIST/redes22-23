#pragma once
char *StrNSplitSpaceNext(char *s, size_t bounds);
Result BufNotContainsInvalidNull(char *b, size_t sz);
Result BufNotContainsNull(char *b, size_t sz);
Result BufTokenizeOpts(char *b, char **opts, size_t sz);
char *BufTokenizeOpt(char *b, const char *delim, char **next);
bool all_digits(const char *s);
void str_replace(char *s, char old, char new);

/* Checks for a ssize_t actually. Is called unsigned for accuracy.
   Things over LONG_MAX will error. */
Result strtoul_check(ssize_t *out, const char *s);
