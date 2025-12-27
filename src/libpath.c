#include "arena.h"
#include <stdlib.h>
#include <string.h>

extern Arena arena;

int has_ext(const char *path, const char *ext) {
  int ext_len = strlen(ext);
  return strncmp(path + strlen(path) - ext_len, ext, ext_len) == 0;
}

int is_rel_dot(const char *path) {
  return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}

char *join_path(const char *pre, const char *post) {
  size_t pre_len = strlen(pre);
  int ends_slsh = pre && *pre && pre[pre_len - 1] == '/';
  char *result = arena_alloc(&arena, pre_len + strlen(post) + 1 + !ends_slsh);
  strcpy(result, pre);
  if (!ends_slsh)
    strcat(result, "/");
  strcat(result, post);
  return result;
}

char *parent(const char *path) {
  char *last_slash = strrchr(path, '/');
  if (last_slash == NULL)
    return arena_strdup(&arena, path);
  return arena_strndup(&arena, path, strlen(path) - strlen(last_slash));
}

char *base(const char *path) {
  char *last_slash = strrchr(path, '/');
  if (last_slash == NULL)
    return arena_strdup(&arena, path);
  return arena_strdup(&arena, last_slash + 1);
}

char *ext(const char *filename) {
  char *dot = strrchr(filename, '.');
  if (dot == NULL)
    return arena_strdup(&arena, "");
  return arena_strdup(&arena, dot);
}

char *trim_suffix(const char *path, const char *suffix) {
  size_t path_len = strlen(path);
  size_t suffix_len = strlen(suffix);
  if (path_len >= suffix_len &&
      strcmp(path + path_len - suffix_len, suffix) == 0) {

    return arena_strndup(&arena, path,
                         path_len - suffix_len -
                             (path[path_len - suffix_len - 1] == '/'));
  }
  return arena_strdup(&arena, path);
}

char *trim_prefix(const char *path, const char *prefix) {
  size_t path_len = strlen(path);
  size_t prefix_len = strlen(prefix);
  if (path_len >= prefix_len && strncmp(path, prefix, prefix_len) == 0)
    return arena_strdup(&arena,
                        path + prefix_len + (path[prefix_len - 1] != '/'));
  return arena_strdup(&arena, path);
}

char *change_ext(const char *path, const char *new_ext) {
  char *no_suf = trim_suffix(path, ext(path));
  char *result = arena_alloc(&arena, strlen(no_suf) + strlen(new_ext));
  strcpy(result, no_suf);
  strcat(result, new_ext);
  return result;
}
