#pragma once
char *StrNSplitSpaceNext(char *s, size_t bounds);
Result BufNotContainsInvalidNull(char *b, size_t sz);
Result BufNotContainsNull(char *b, size_t sz);
Result BufTokenizeOpts(char *b, char **opts, size_t sz);
bool all_digits(const char *s);
Result strtoul_check(ssize_t *out, const char *s);
