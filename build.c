#include "include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

constexpr char COMPILER[] = "gcc";
constexpr char BUILDDIR[] = "build";
constexpr char OUTBIN[] = "build/out";
constexpr char UPDATESUBMODULES[] =
    "git submodule update --init --recursive --remote";
constexpr char SRC[][BUFSIZ] = {
    "src/main.c",
    "src/config.c",
    "src/globdef.c",
    "src/completions.c",
    "minimal-c-json-parser/src/json.c",
};
constexpr char CFLAGS[][BUFSIZ] = {"-Wall",      "-Werror", "-Wextra",
                                   "-std=gnu23", "-O2",     "-lcurl"};
constexpr char INCL[][BUFSIZ] = {"-Iinclude",
                                 "-Iminimal-c-json-parser/include"};
constexpr size_t SRCCOUNT = sizeof(SRC) / sizeof(SRC[0]);
constexpr size_t CFLAGSCOUNT = sizeof(CFLAGS) / sizeof(CFLAGS[0]);
constexpr size_t INCLCOUNT = sizeof(INCL) / sizeof(INCL[0]);
constexpr size_t ARGSLEN =
    (SRCCOUNT * BUFSIZ) + (CFLAGSCOUNT * BUFSIZ) + (INCLCOUNT * BUFSIZ);

static bool ensure_dir(const char *const src) {
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
    term_print_color("Build directory could not be created", term_color_red);
    return 1;
  }
  term_print_color("Build directory created", term_color_green);

  char command[ARGSLEN];
  auto total = snprintf(command, ARGSLEN, "%s && %s -o %s", UPDATESUBMODULES,
                        COMPILER, OUTBIN);
  if (total < 0) {
    term_print_color("Compiler and output were unable to be set",
                     term_color_red);
    return 1;
  }

  for (size_t i = 0; i < SRCCOUNT; i++) {
    if (snprintf(&command[total], ARGSLEN, " %s", SRC[i]) < 0) {
      term_print_color("Source files were unable to be set", term_color_red);
      return 1;
    }
    total += strlen(SRC[i]) + 1;
  }

  for (size_t i = 0; i < INCLCOUNT; i++) {
    if (snprintf(&command[total], ARGSLEN, " %s", INCL[i]) < 0) {
      term_print_color("Include directories were unable to be set",
                       term_color_red);
      return 1;
    }
    total += strlen(INCL[i]) + 1;
  }

  for (size_t i = 0; i < CFLAGSCOUNT; i++) {
    if (snprintf(&command[total], ARGSLEN, " %s", CFLAGS[i]) < 0) {
      term_print_color("CFLAGS were unable to be set", term_color_red);
      return 1;
    }
    total += strlen(CFLAGS[i]) + 1;
  }

  system(command);
  term_print_color("Executable file created", term_color_green);

  return 0;
}
