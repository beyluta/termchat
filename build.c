#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

constexpr char COMPILER[] = "gcc";
constexpr char BUILDDIR[] = "build";
constexpr char OUTBIN[] = "build/out";
constexpr char SRC[][BUFSIZ] = {
    "src/main.c",    "src/window.c",      "src/config.c",
    "src/globdef.c", "src/completions.c", "minimal-c-json-parser/src/json.c",
};
constexpr char CFLAGS[][BUFSIZ] = {"-Wall",    "-Werror", "-Wextra",
                                   "-std=c23", "-O2",     "-lcurl"};
constexpr char INCL[][BUFSIZ] = {"-Iminimal-c-json-parser/include"};
constexpr size_t SRCCOUNT = sizeof(SRC) / sizeof(SRC[0]);
constexpr size_t CFLAGSCOUNT = sizeof(CFLAGS) / sizeof(CFLAGS[0]);
constexpr size_t INCLCOUNT = sizeof(INCL) / sizeof(INCL[0]);
constexpr size_t ARGSLEN =
    (SRCCOUNT * BUFSIZ) + (CFLAGSCOUNT * BUFSIZ) + (INCLCOUNT * BUFSIZ);

typedef enum : unsigned char {
  term_color_green,
  term_color_red,
} term_color_t;

static void printc(const char *src, term_color_t color) {
  switch (color) {
  default:
  case term_color_green:
    printf("\033[32m%s\033[0m\n", src);
    break;
  case term_color_red:
    fprintf(stderr, "\033[31m%s\033[0m\n", src);
    break;
  }
}

static bool ensure_dir(const char *src) {
  struct stat st = {};
  if (stat(src, &st) == -1) {
    if (mkdir(src, 0700) != 0) {
      return false;
    }
  }
  return true;
}

int main() {
  if (!ensure_dir(BUILDDIR)) {
    printc("Build directory could not be created", term_color_red);
    return 1;
  }
  printc("Build directory created", term_color_green);

  char command[ARGSLEN];
  auto total = snprintf(command, ARGSLEN, "%s -o %s", COMPILER, OUTBIN);
  if (total < 0) {
    printc("Compiler and output were unable to be set", term_color_red);
    return 1;
  }

  for (size_t i = 0; i < SRCCOUNT; i++) {
    if (snprintf(&command[total], ARGSLEN, " %s", SRC[i]) < 0) {
      printc("Source files were unable to be set", term_color_red);
      return 1;
    }
    total += strlen(SRC[i]) + 1;
  }

  for (size_t i = 0; i < INCLCOUNT; i++) {
    if (snprintf(&command[total], ARGSLEN, " %s", INCL[i]) < 0) {
      printc("Include directories were unable to be set", term_color_red);
      return 1;
    }
    total += strlen(INCL[i]) + 1;
  }

  for (size_t i = 0; i < CFLAGSCOUNT; i++) {
    if (snprintf(&command[total], ARGSLEN, " %s", CFLAGS[i]) < 0) {
      printc("CFLAGS were unable to be set", term_color_red);
      return 1;
    }
    total += strlen(CFLAGS[i]) + 1;
  }

  system(command);
  printc("Executable file created", term_color_green);

  return 0;
}
