#ifndef YAMDW_LIBPATH_H
#define YAMDW_LIBPATH_H

int has_ext(const char *path, const char *ext);
int is_rel_dot(const char *path);
int is_rel(const char *path);
char *join_path(const char *pre, const char *post);
char *base(const char *path);
char *parent(const char *path);
char *ext(const char *filename);
char *trim_suffix(const char *path, const char *suffix); 
char *trim_prefix(const char *path, const char *prefix);
char *change_ext(const char *path, const char *new_ext);
int is_abs(const char *path);

#endif
