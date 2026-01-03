#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libio.h"
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

// char curpath[PATH_MAX] = "";
char inpath[PATH_MAX] = "";
char outpath[PATH_MAX] = "";

// char *html_page = "";
// int len_page = 0;
char *html_page_begin = "<html>\n<body>\n";
char *html_page_end = "</body>\n</html>\n";

typedef struct {
  FILE *file;
} Context;

void process_output(const MD_CHAR *text, MD_SIZE size, void *userdata) {
  fprintf(((Context *)userdata)->file, "%*.*s", size, size, text);
}

char *to_out_path(const char *path, const int to_html) {
  // TODO: better memory management
  char *new_path = trim_prefix(path, inpath);
  if (to_html)
    new_path = change_ext(new_path, ".html");
  return join_path(outpath, new_path);
}

int generate(const char *path) {
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
      if (generate(join_path(path, dp->d_name)) == 0)
        continue;
      closedir(dir);
      return 1;
    }
    closedir(dir);
    break;
  case S_IFREG:
    char *html_path = to_out_path(path, 1);
    create_parent_dirs(html_path);

    if (!has_ext(path, ".md")) {
      char *new_path = to_out_path(path, 0);
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

int create_out(const char *path) {
  struct stat st;
  if (stat(path, &st) == 0 && (st.st_mode & S_IFDIR))
    return 0;
  // TODO: rm file if present
  // if (rmdir_rec(out) != 0)
  //   return 1;

  if (mkdir(path, 0700) != 0)
    return 1;

  return 0;
}

int main(int argc, char **argv) {
  // TODO: getopt
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input_path> <output_path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // if (getcwd(curpath, MAX_BUF) == NULL) {
  //   perror("getcwd");
  //   exit(EXIT_FAILURE);
  // }

  const char *in = closest_dir(argv[1]);

  if (realpath(in, inpath) == NULL) {
    fprintf(stderr, "Failed to get real input path '%s'", argv[1]);
    exit(EXIT_FAILURE);
  }

  const char *out = closest_dir(argv[2]);

  if (create_out(out) != 0) {
    fprintf(stderr, "Failed to create '%s'", outpath);
    exit(EXIT_FAILURE);
  }

  if (realpath(out, outpath) == NULL) {
    fprintf(stderr, "Failed to get real output path '%s'", argv[2]);
    exit(EXIT_FAILURE);
  }

  printf("test/in/asdf: %s\n", convert_uri("asdf", ""));
  printf("test/in/asdf: %s\n", convert_uri("/asdf", ""));
  printf("test/in/asdf: %s\n", convert_uri("./asdf", ""));

  printf("test/in/asdf: %s\n", convert_uri("asdf/asdf.md", ""));
  printf("test/in/asdf: %s\n", convert_uri("asdf.md", "asdf"));
  printf("test/in/asdf: %s\n", convert_uri("asdf.md", "asdf/")); // breaks
  printf("test/in/asdf: %s\n", convert_uri("/asdf.md", "asdf"));
  printf("test/in/asdf: %s\n", convert_uri("./asdf.md", "asdf"));
  printf("test/in/index.md: %s\n", convert_uri("index.md", ""));
  printf("test/in/asdf/img.png: %s\n", convert_uri("asdf/img.png", ""));
  printf("/etc/passwd: %s\n", convert_uri("/etc/passwd", ""));
  printf("./bonkers: %s\n", convert_uri("./bonkers", ""));
  printf("test/in/bonkers: %s\n", convert_uri("test/in/bonkers", ""));
  printf("test/in/../../README.md: %s\n", convert_uri("../../README.md", ""));
  printf("./README.md: %s\n", convert_uri("./README.md", ""));
  printf("asdf@asdf.asdf: %s\n", convert_uri("asdf@asdf.asdf", ""));
  printf("https://a.a: %s\n", convert_uri("https://a.a", ""));
  exit(0);

  // html_page = read_file("./assets/page.html");
  // len_page = strlen(html_page);

  if (generate(argv[1]) != 0) {
    fprintf(stderr, "Failed to generate '%s'", inpath);
    exit(EXIT_FAILURE);
  }
  arena_free(&arena);

  return 0;
}
