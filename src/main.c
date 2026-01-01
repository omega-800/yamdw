#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libfs.h"
#include "libpath.h"
#include "libstr.h"
#include "md4c/md4c-html.h"
#include "url.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"

#define MD4C_USE_UTF8

#define BUILD_DIR ".build"
#define MAX_BUF 256

// TODO: MD_PARSER enter_span(MD_SPAN_A)

Arena arena = {0};
char curpath[MAX_BUF] = {0};
// char *html_page = "";
// int len_page = 0;
char *html_page_begin = "<html>\n<body>\n";
char *html_page_end = "</body>\n</html>\n";

typedef struct Context {
  FILE *file;
} Context;

void process_output(const MD_CHAR *text, MD_SIZE size, void *userdata) {
  fprintf(((Context *)userdata)->file, "%*.*s", size, size, text);
}

char *to_out_path(const char *path, const char *root, const char *out,
                  const int to_html) {
  // TODO: better memory management
  char *new_path = trim_prefix(path, root);
  if (to_html)
    new_path = change_ext(new_path, ".html");
  return join_path(out, new_path);
}

int generate(const char *path, const char *root, const char *out) {
  struct stat sb;

  if (lstat(path, &sb) == -1) {
    perror("lstat");
    return 1;
  }

  switch (sb.st_mode & S_IFMT) {
  case S_IFDIR:
    struct dirent *dp;
    DIR *dir = opendir(path);
    if (dir == NULL) {
      fprintf(stderr, "Couldn't open directory: %s", path);
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
    char *html_path = to_out_path(path, root, out, 1);
    create_parent_dirs(html_path);

    if (!has_ext(path, ".md")) {
      char *new_path = to_out_path(path, root, out, 0);
      cp(path, new_path);
      break;
    }

    char *contents = read_file(path);

    fclose(fopen(html_path, "w"));
    FILE *file = fopen(html_path, "a");

    Context c = {file};
    fprintf(file, "%s", html_page_begin);
    md_html(contents, strlen(contents), process_output, (void *)&c,
            MD_FLAG_TABLES | MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH |
                MD_FLAG_LATEXMATHSPANS | MD_FLAG_WIKILINKS | MD_FLAG_UNDERLINE,
            0);
    fprintf(file, "%s", html_page_end);

    // TODO:
    // free contents
    fclose(file);
    break;
  case S_IFLNK:
    // TODO:
    printf("TODO: Symlink\n");
    break;
  default:
    printf("Unsupported \"file\" encountered: %s\n", path);
    break;
  }

  return 0;
}

int create_out(const char *out) {
  struct stat st;
  if (stat(out, &st) == 0 && (st.st_mode & S_IFDIR))
    return 0;
  // TODO: rm file if present
  // if (rmdir_rec(out) != 0)
  //   return 1;

  if (mkdir(out, 0700) != 0)
    return 1;

  return 0;
}

int main(int argc, char **argv) {
  if (getcwd(curpath, MAX_BUF) == NULL) {
    perror("getcwd");
    exit(EXIT_FAILURE);
  }

  printf("asdfasdfasdfasdfasdfasdfasdf: %s", curpath);
  printf("test/in/asdf: %s\n", convert_uri("test/in/asdf", "test/in"));
  printf("test/in/asdf/asdf.md: %s\n", convert_uri("test/in/asdf/asdf.md", "test/in"));
  printf("test/in/index.md: %s\n", convert_uri("test/in/index.md", "test/in"));
  printf("test/in/asdf/img.png: %s\n", convert_uri("test/in/asdf/img.png", "test/in"));
  printf("/etc/passwd: %s\n", convert_uri("/etc/passwd", "test/in"));
  printf("./bonkers: %s\n", convert_uri("./bonkers", "test/in"));
  printf("test/in/bonkers: %s\n", convert_uri("test/in/bonkers", "test/in"));
  printf("test/in/../../README.md: %s\n", convert_uri("test/in/../../README.md", "test/in"));
  printf("./README.md: %s\n", convert_uri("./README.md", "test/in"));
  printf("asdf@asdf.asdf: %s\n", convert_uri("asdf@asdf.asdf", "test/in"));
  printf("https://a.a: %s\n", convert_uri("https://a.a", "test/in"));
  printf(": %s\n", convert_uri("", "test/in"));
  exit(0);

  // TODO: getopt
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input_path> <output_path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *in = closest_dir(argv[1]);
  const char *out = closest_dir(argv[2]);

  if (create_out(out) != 0) {
    fprintf(stderr, "Failed to create %s", out);
    exit(EXIT_FAILURE);
  }

  // html_page = read_file("./assets/page.html");
  // len_page = strlen(html_page);

  if (generate(argv[1], in, out) != 0) {
    fprintf(stderr, "Failed to generate %s", in);
    exit(EXIT_FAILURE);
  }
  arena_free(&arena);

  return 0;
}
