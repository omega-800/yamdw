#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "arena.h"
#include "libio.h"
#include "libpath.h"
#include "libstr.h"
// rfc 1738
// rfc 3986

extern char inpath[PATH_MAX];
extern char outpath[PATH_MAX];
extern Arena arena;

const char *handle_existing_path(const char *path) {
  size_t len_in = strlen(inpath);
  char *resolved_path = arena_alloc(&arena, PATH_MAX);
  if (realpath(path, resolved_path) == NULL) {
    fprintf(stderr, "Failed to resolve '%s'\n", path);
    perror("realpath");
    return path;
  }

  if (strncmp(resolved_path, inpath, len_in) != 0) {
    fprintf(stderr, "Trying to access '%s' which is outside of root '%s'\n",
            resolved_path, inpath);
    return NULL;
  }

  char *rel_path = resolved_path + len_in;
  if (ends_with(rel_path, ".md"))
    return change_ext(rel_path, ".html");
  else
    return rel_path;
}

int offset(char *path) {
  return strlen(path) > 1 && path[0] == '.' && path[1] == '/' ? 2 : 0;
}

const char *convert_uri(const char *uri, const char *rel) {
  const char *full;
  size_t len_rel = strlen(rel);

  // TODO: check first if file or other protocol

  if (len_rel == 0 && is_abs(uri)) {
    full = uri;
  } else if (is_abs(rel)) {
    full = concat(3, rel, "/", uri);
  } else {
    full = concat(5, inpath, "/", rel, "/", uri);
  }

  struct stat st;
  if (stat(full, &st) == 0) {
    if (st.st_mode & S_IFDIR && has_file(full, "index.md")) {
      return handle_existing_path(concat(2, full, "/index.md"));
    } else {
      return handle_existing_path(full);
    }
  } else {
    char *filename = base(uri);
    char *parent_dir = parent(full);

    struct dirent *dp;
    DIR *dir = opendir(parent_dir);
    if (dir == NULL)
      return uri;
    while ((dp = readdir(dir)) != NULL) {
      // TODO: further checks
      if (strncmp(dp->d_name, filename, strlen(dp->d_name)) == 0) {
        const char *res =
            handle_existing_path(concat(3, parent_dir, "/", dp->d_name));
        closedir(dir);
        return res;
      }
    }
    closedir(dir);
  }
  // TODO: if above checks fail and path is abs
  return uri;
}
