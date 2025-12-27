#ifndef YAMDW_LIBFS_H
#define YAMDW_LIBFS_H

int rmdir_rec(const char *path);
int cp(const char *from, const char *to);
const char *closest_dir(const char *path);
char *read_file(const char *path);

#endif
