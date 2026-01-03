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
  printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
  printf("in: %s, out: %s \n", inpath, outpath);

  // TODO: resolve path if ..
  if (strncmp(path, inpath, strlen(inpath)) != 0) {
    fprintf(stderr, "Trying to access '%s' which is outside of root '%s'",
            path, inpath);
    return NULL;
  }
  // OK
  // TODO:
  if (ends_with(path, ".md"))
    return change_ext(path, ".html");
  else 
    return path;
  return NULL;
}

char *concat(int n, ...) {
  printf("NRARGS:(%i)\n", n);
  va_list args;
  size_t len_full = 1;

  va_start(args, n);  
  for (int i = 0; i < n; i++) 
    len_full += strlen(va_arg(args, char*));
  va_end(args);

  char *full = arena_alloc(&arena, len_full);

  size_t len_cur = 0;
  va_start(args, n);  
  for (int i = 0; i < n; i++) {
    char *part = va_arg(args, char*);

    printf("PART(%i): ", i);
    printf("%s\n", part);

    size_t len_part = strlen(part);
    arena_memcpy(full + len_cur, part, len_part);
    len_cur += len_part;
  }
  va_end(args);

  full[len_full] = '\0';
  return full;
}

int offset(char *path) {
  size_t len = strlen(path);
  return (len > 0 && (path[0] == '.' || path[0] == '/')) + (len > 1 && path[1] == '/');
}

char *convert_uri(char *uri, char *rel) {
  int off_uri = offset(uri);
  int off_rel = offset(rel);
  printf("IN: %s REL: %s URI: %s\n", inpath, rel + off_rel, uri + off_uri);
  char *full = strlen(rel) - off_rel > 0 
    ? concat(5, inpath, "/", rel + off_rel, "/", uri + off_uri) 
    : concat(3, inpath, "/", uri + off_uri);
  printf("\n[D] '%s': ", full);

  struct stat st;
  if (stat(full, &st) == 0) {
    printf("exists, ");
    return handle_existing_path(full);
  } else {
    perror("stat");
    printf("notexists, ");
    char *filename = base(uri);
    char *parent_dir = parent(full);
    struct dirent *dp;
    DIR *dir = opendir(parent_dir);
    if (dir != NULL) {
      printf("dir exists, ");
      while ((dp = readdir(dir)) != NULL) {
        // if (is_rel_dot(dp->d_name))
        // continue;
        // TODO: further checks
        // TODO: dir/index.md
        if (strncmp(dp->d_name, filename, strlen(dp->d_name)) == 0) {
          printf("file exists, ");
          char *res = handle_existing_path(concat(3, parent_dir, "/", dp->d_name));
          closedir(dir);
          return res;
        }
      }
      printf("file notexists, ");
      closedir(dir);
    }
  }

  printf("default, ");
  return uri;
}
