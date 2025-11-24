#include "md4c/md4c-html.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MD4C_USE_UTF8

#define BUILD_DIR ".build"

typedef struct Context {
  FILE *file;
} Context;

void process_output(const MD_CHAR *text, MD_SIZE size, void *userdata) {
  // printf("%s", text);
  fprintf(((Context *)userdata)->file, "%s", text);
}

int generate(char *path, char *out) {
  printf("%s: ", path);
  struct stat sb;

  if (lstat(path, &sb) == -1) {
    perror("lstat");
    return 1;
  }

  switch (sb.st_mode & S_IFMT) {
  case S_IFDIR:
    printf("directory\n");
    struct dirent *dp;
    DIR *dir = opendir(path);
    if (dir == NULL) {
      fprintf(stderr, "Couldn't open directory: %s", path);
      closedir(dir);
      return 1;
    }
    while ((dp = readdir(dir)) != NULL) {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
        continue;
      char newpath[sizeof(path) + sizeof(dp->d_name)];
      snprintf(newpath, sizeof(newpath), "%s/%s", path, dp->d_name);
      if (generate(newpath, out) != 0) {
        closedir(dir);
        return 1;
      }
    }
    closedir(dir);
    break;
  case S_IFREG:
    if (strncmp(path + strlen(path) - 3, ".md", 3) != 0) {
      printf("skipping non-markdown file\n");
      break;
    }
    printf("regular file\n");

    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *contents = malloc(fsize + 1);
    fread(contents, fsize, 1, f);
    fclose(f);

    contents[fsize] = 0;

    // TODO:
    FILE *file = fopen(path + ".html", "a");

    Context c = {file};
    md_html(contents, fsize + 1, process_output, (void *)&c, MD_HTML_FLAG_DEBUG,
            MD_FLAG_TABLES | MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH |
                MD_FLAG_LATEXMATHSPANS | MD_FLAG_WIKILINKS | MD_FLAG_UNDERLINE);

    fclose(file);
    free(contents);
    break;
  case S_IFBLK:
    printf("block device\n");
    break;
  case S_IFCHR:
    printf("character device\n");
    break;
  case S_IFIFO:
    printf("FIFO/pipe\n");
    break;
  case S_IFLNK:
    printf("symlink\n");
    break;
  case S_IFSOCK:
    printf("socket\n");
    break;
  default:
    printf("unknown?\n");
    break;
  }

  return 0;
}

int rmdir_rec(const char *path) {
  struct dirent *entry;
  DIR *dp = opendir(path);

  if (dp == NULL) {
    printf("Error opening directory '%s'.\n", path);
    closedir(dp);
    return 1;
  }

  while ((entry = readdir(dp)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
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

int create_out(char *out) {
  struct stat st;
  if (stat(out, &st) == 0 && (st.st_mode & S_IFDIR))
    if (rmdir_rec(out) != 0)
      return 1;

  if (mkdir(out, 0700) != 0)
    return 1;

  return 0;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input_path> <output_path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (create_out(argv[2]) != 0) {
    fprintf(stderr, "Failed to create %s", argv[2]);
    exit(EXIT_FAILURE);
  }

  if (generate(argv[1], argv[2]) != 0) {
    fprintf(stderr, "Failed to generate %s", argv[1]);
    exit(EXIT_FAILURE);
  }

  return 0;
}
