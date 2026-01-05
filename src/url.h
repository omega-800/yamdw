#ifndef YAMDW_URL_H
#define YAMDW_URL_H

#include <stdio.h>

const char *convert_uri(const char *uri, const char *rel);

typedef struct {
  FILE *file;
  const char *relpath;
} Context;

#endif
