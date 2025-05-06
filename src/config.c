#include "../include/config.h"
#include "../include/globdef.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RC_FILENAME "termchatrc.json"

const int get_rc_path(char output[], const int len) {
  const char *config_dir = getenv("XDG_CONFIG_HOME");
  if (config_dir != NULL) {
    snprintf(output, len, "%s/%s", config_dir, RC_FILENAME);
    return ERR_RECOVERABLE;
  }

  const char *home_dir = getenv("HOME");
  if (home_dir == NULL) {
    fprintf(stderr, "Could not find home directory\n");
    return ERR_UNRECOVERABLE;
  }
  snprintf(output, len, "%s/.config/%s", home_dir, RC_FILENAME);
  return ERR_RECOVERABLE;
}

const int get_rc_exists() {
  char filepath[MAX_BUFF_SIZE];
  if (get_rc_path(filepath, sizeof(filepath)) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Could not find file\n");
    return FILE_NOT_EXISTS;
  }
  return access(filepath, F_OK) != 0;
}

const int get_rc_contents(const char *filename, char buffer[],
                          unsigned long bufferLength) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file for reading\n");
    return ERR_UNRECOVERABLE;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    fprintf(stderr, "Failed to read file to end\n");
    return ERR_UNRECOVERABLE;
  }

  const long length = ftell(file);
  if (length <= -1) {
    fclose(file);
    fprintf(stderr, "Failed to get current position of the stream\n");
    return ERR_UNRECOVERABLE;
  }

  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    fprintf(stderr, "Failed to set seek\n");
    return ERR_UNRECOVERABLE;
  }

  const unsigned long fileLength = fread(buffer, 1, length, file);
  if (fileLength <= 0) {
    fclose(file);
    fprintf(stderr, "Failed to read contents into memory\n");
    return ERR_UNRECOVERABLE;
  }

  if (fileLength > bufferLength) {
    fclose(file);
    fprintf(stderr, "Buffer is too small to hold file contents\n");
    return ERR_UNRECOVERABLE;
  }

  fclose(file);
  buffer[length] = '\0';
  return ERR_RECOVERABLE;
}
