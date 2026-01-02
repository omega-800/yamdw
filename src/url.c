#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "arena.h"
#include "libpath.h"
// rfc 1738
// rfc 3986

extern char *curpath;
extern Arena arena;

char *convert_uri(char *rel, char *uri) {
  int off = (uri[0] == '.' || uri[0] == '/') + uri[1] == '/';

  // TODO:
  size_t len_uri = strlen(uri);
  size_t len_rel = strlen(rel);
  size_t len_full = len_uri - off + len_rel + 1;
  char *full = arena_alloc(&arena, len_full);
  arena_memcpy(full, rel, len_rel);
  arena_memcpy(full + len_rel, uri + off, len_uri - off);
  full[len_full] = '\0';

  char resolved_path[65536]; // TODO:

  struct stat st;
  if (stat(full, &st) == 0) {
    if (realpath(full, resolved_path) != NULL) {
      if(!ends_with(resolved_path, )) 
      // OK
    } else {
      // TODO:
    }
  } else {
    char *filename = base(uri);
    char *parent_dir = parent(full);
    struct dirent *dp;
    DIR *dir = opendir(parent_dir);
    if (dir != NULL) {
      while ((dp = readdir(dir)) != NULL) {
        if (is_rel_dot(dp->d_name))
          continue;
        if (strncmp(dp->d_name, filename, strlen(dp->d_name)) != 0)
          continue;
        // TODO:
      }
      closedir(dir);
    } else {
      // URI
    }
  }

  // TODO:
  // check if inside cur dir
  // check if exists
  // handle link to external url vs dir vs md/html/img vs bin/exe
  // convert file without ending to full filename (vimwiki)
  // convert path to point from root
  return uri;
}

// check if uri starts with file:// -> strip
// check if uri exists on fs (glob.h <file>\.*)?
// yes              no -> check if exists without file ending
// check            =normal a
// if points to markdown -> change to .html

// file://
// ./
// /
// asdf
// asdf/asdf
// asdf.d/asdf
