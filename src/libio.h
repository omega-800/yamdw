#ifndef YAMDW_LIBIO_H
#define YAMDW_LIBIO_H

int eprintf(const char *msg);
int rmdir_rec(const char *path);
int cp(const char *from, const char *to);
char *closest_dir(const char *path);
char *read_file(const char *path);
void create_parent_dirs(const char *path);

#endif
