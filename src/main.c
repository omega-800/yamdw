#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libpath.h"
#include "md4c/md4c-html.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"

Arena arena = {0};

#define MD4C_USE_UTF8

#define BUILD_DIR ".build"

typedef struct Context {
  FILE *file;
} Context;

void process_output(const MD_CHAR *text, MD_SIZE size, void *userdata) {
  fprintf(((Context *)userdata)->file,  "%*.*s", size, size, text);
}

int generate(char *path, char *root, char *out) {
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
      if (is_rel_dot(dp->d_name))
        continue;
      if (generate(join_path(path, dp->d_name), root, out) == 0)
        continue;
      closedir(dir);
      return 1;
    }
    closedir(dir);
    break;
  case S_IFREG:
    if (!has_ext(path, ".md")) {
      // TODO: copy
      printf("skipping non-markdown file\n");
      break;
    }
    printf("regular file\n");

    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *contents = arena_alloc(&arena, fsize + 1);
    fread(contents, fsize, 1, f);
    fclose(f);

    contents[fsize] = 0;

    // TODO: better memory management
    char *no_pre = trim_prefix(path, root);
    char *changed_ext = change_ext(no_pre, ".html");
    char *html_path = join_path(out, changed_ext);

    FILE *file = fopen(html_path, "a");

    Context c = {file};
    md_html(contents, fsize + 1, process_output, (void *)&c, MD_HTML_FLAG_DEBUG,
            MD_FLAG_TABLES | MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH |
                MD_FLAG_LATEXMATHSPANS | MD_FLAG_WIKILINKS | MD_FLAG_UNDERLINE);

    // TODO:
    // free contents
    fclose(file);
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

int create_out(char *out) {
  // TODO: check for path type and act accordingly
  struct stat st;
  if (stat(out, &st) == 0 && (st.st_mode & S_IFDIR))
    return 1;
  // TODO:
  // if (rmdir_rec(out) != 0)
  //   return 1;

  if (mkdir(out, 0700) != 0)
    return 1;

  return 0;
}

char *to_dir(char *path) {
  struct stat st;
  if (stat(path, &st) == 0 && (st.st_mode & S_IFREG))
    return parent(path);
  return path;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input_path> <output_path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char *in = to_dir(argv[1]);
  char *out = to_dir(argv[2]);

  if (create_out(out) != 0) {
    fprintf(stderr, "Failed to create %s", out);
    exit(EXIT_FAILURE);
  }

  if (generate(argv[1], in, out) != 0) {
    fprintf(stderr, "Failed to generate %s", in);
    exit(EXIT_FAILURE);
  }
  arena_free(&arena);

  return 0;
}
