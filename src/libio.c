#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arena.h"
#include "libpath.h"

extern Arena arena;

int rmdir_rec(const char *path) {
  struct dirent *entry;
  DIR *dp = opendir(path);

  if (dp == NULL) {
    printf("Error opening directory '%s'.\n", path);
    return 1;
  }

  while ((entry = readdir(dp)) != NULL) {
    if (is_rel_dot(entry->d_name))
      continue;
    char full_path[sizeof(path) + sizeof(entry->d_name)];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
    struct stat st;

    if (stat(full_path, &st) == 0) {
      if (S_ISDIR(st.st_mode)) {
        rmdir_rec(full_path);
      } else if (unlink(full_path) != 0) {
        printf("Error deleting file '%s'.\n", full_path);
        closedir(dp);
        return 1;
      }
    }
  }

  closedir(dp);

  if (rmdir(path) != 0) {
    printf("Error removing directory '%s'.\n", path);
    return 1;
  }

  return 0;
}

int cp(const char *from, const char *to) {
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0)
    return 1;

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0)
    goto out_error;

  while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;

    do {
      nwritten = write(fd_to, out_ptr, nread);

      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR)
        goto out_error;
    } while (nread > 0);
  }

  if (nread == 0) {
    if (close(fd_to) < 0) {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);

    return 0;
  }

out_error:
  saved_errno = errno;

  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);

  errno = saved_errno;
  return 1;
}

char *closest_dir(const char *path) {
  struct stat st;
  // TODO: if not exists
  if (stat(path, &st) != 0 || (stat(path, &st) == 0 && (st.st_mode & S_IFDIR)))
    return arena_strdup(&arena, path);
  return parent(path);
}

char *read_file(const char *path) {
  FILE *f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *contents = arena_alloc(&arena, fsize + 1);
  long n = fread(contents, fsize, 1, f);
  fclose(f);

  if (n != 1) {
    // welp
  }

  contents[fsize] = '\0';
  return contents;
}

void create_parent_dirs(const char *path) {
  // TODO: check if exists
  char *dir_path = closest_dir(path);
  char *next_sep = strchr(path, '/');
  while (next_sep != NULL) {
    int dir_path_len = next_sep - path;
    arena_memcpy(dir_path, path, dir_path_len);
    dir_path[dir_path_len] = '\0';
    mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH);
    next_sep = strchr(next_sep + 1, '/');
  }
  // TODO: free dir_path
}

int has_file(const char *dirpath, const char *filename) {
  int res = 0;

  struct dirent *dp;
  DIR *dir = opendir(dirpath);

  if (dir == NULL)
    return res;

  while ((dp = readdir(dir)) != NULL) {
    if (strncmp(dp->d_name, filename, strlen(dp->d_name)) == 0) {
      res = 1;
      break;
    }
  }

  closedir(dir);
  return res;
}
