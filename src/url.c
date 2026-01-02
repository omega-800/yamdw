#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "arena.h"
#include "libpath.h"
#include "libstr.h"
// rfc 1738
// rfc 3986

extern char inpath[PATH_MAX];
extern char outpath[PATH_MAX];
extern Arena arena;

char *handle_existing_path(char *path) {
  char resolved_path[PATH_MAX]; // TODO:
  printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
  printf("in: %s, out: %s \n", inpath, outpath);

  if (realpath(path, resolved_path) != NULL) {
    if (!ends_with(resolved_path, inpath)) {
      fprintf(stderr, "Trying to access '%s' which is outside of root '%s'",
              resolved_path, inpath);
      return NULL;
    }
    // OK
    // TODO:
    if (ends_with(path, ".md"))
      return change_ext(path, ".html");
    else 
      return path;
  } else {
    fprintf(stderr, "Failed to get real path '%s'", path);
  }
  return NULL;
}

char *concat(char *fst, char *snd) {
  size_t len_fst = strlen(fst);
  size_t len_snd = strlen(snd);
  size_t len_full = len_fst + len_snd;
  char *full = arena_alloc(&arena, len_full);
  arena_memcpy(full, fst, len_fst);
  arena_memcpy(full + len_fst, snd, len_snd);
  full[len_full] = '\0';
  return full;
}

char *convert_uri(char *uri, char *rel) {
  int off = (uri[0] == '.' || uri[0] == '/') + uri[1] == '/';
  char *full = concat(rel, uri + off);

  struct stat st;
  if (stat(full, &st) == 0) {
    return handle_existing_path(full);
  } else {
    char *filename = base(uri);
    char *parent_dir = parent(full);
    struct dirent *dp;
    DIR *dir = opendir(parent_dir);
    if (dir != NULL) {
      while ((dp = readdir(dir)) != NULL) {
        // if (is_rel_dot(dp->d_name))
        // continue;
        // TODO: further checks
        // TODO: dir/index.md
        if (strncmp(dp->d_name, filename, strlen(dp->d_name)) == 0)
          return handle_existing_path(concat(parent_dir, dp->d_name));
      }
      closedir(dir);
    }
  }

  return uri;
}
