#ifndef YAMDW_LIBSTR_H
#define YAMDW_LIBSTR_H

int ends_with(const char *str, const char *end);
char *str_replace(char *orig, char *rep, char *with);
char *concat(int n, ...);
char *until(char *str, int n);

#endif
