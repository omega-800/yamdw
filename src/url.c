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

char *convert_uri(char *uri, char *rel) {
  int off = (uri[0] == '.' || uri[0] == '/') + uri[1] == '/';
  // TODO:
  char *full = arena_strdup(&arena, rel);
  strncat(full, uri + off, strlen(uri) - off + strlen(rel) + 1);

  char resolved_path[65536]; // TODO:

  struct stat st;
  if (stat(uri, &st) == 0) {
    if (realpath(uri, resolved_path) != NULL) {
      // OK
    } else {
      // TODO:
    }
  } else {
    char *filename = base(uri);
    char *parent_dir = parent(uri);
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
