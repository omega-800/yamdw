#include "arena.h"
#include <stddef.h>
#include <string.h>

extern Arena arena;

int ends_with(const char *str, const char *end) {
  size_t len_str = strlen(str);
  size_t len_end = strlen(end);

  return strncmp(str + len_str - len_end, end, len_end) == 0; 
}

char *str_replace(char *orig, char *rep, char *with) {
  char *result;
  char *ins;
  char *tmp;
  int len_rep;
  int len_with;
  int len_front;
  int count;

  if (!orig || !rep)
    return NULL;
  len_rep = strlen(rep);
  if (len_rep == 0)
    return NULL;
  if (!with)
    with = "";
  len_with = strlen(with);

  ins = orig;
  for (count = 0; (tmp = strstr(ins, rep)); ++count)
    ins = tmp + len_rep;

  tmp = result =
      arena_alloc(&arena, strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;

  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep;
  }
  strcpy(tmp, orig);
  return result;
}

char *concat(int n, ...) {
  va_list args;
  size_t len_full = 0;

  va_start(args, n);
  for (int i = 0; i < n; i++)
    len_full += strlen(va_arg(args, char *));
  va_end(args);

  char *full = arena_alloc(&arena, len_full);

  size_t len_cur = 0;
  va_start(args, n);
  for (int i = 0; i < n; i++) {
    char *part = va_arg(args, char *);

    size_t len_part = strlen(part);
    arena_memcpy(full + len_cur, part, len_part);
    len_cur += len_part;
  }
  va_end(args);

  full[len_full] = '\0';
  return full;
}

char *until(char *str, int n) {
  char *new = arena_alloc(&arena, n); 
  strncpy(new, str, n);
  return new;
}
