#include "config.h"
#include "globdef.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

constexpr unsigned char RC_FILENAME[] = "termchatrc.json";

/**
 * @brief Gets the path where the configuration file for the program lives. The
 * default path should always be `/home/__USER__/.config/termchatrc.json`
 *
 * @param output A character array to store the resulting path.
 * @param len The length of the output buffer.
 * @return 0 on success, or a non-zero error code on failure.
 */
int get_rc_path(char *const output, const int len) {
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

/**
 * @brief Searches if the `termchatrc` file exists under
 * `/home/__USER__/.config`
 *
 * @param output A character array to store the resulting path.
 * @param len The length of the output buffer.
 * @return 0 on success, or a non-zero error code on failure.
 */
int get_rc_exists() {
  char filepath[MAX_BUFF_SIZE];
  if (get_rc_path(filepath, sizeof(filepath)) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Could not find file\n");
    return FILE_NOT_EXISTS;
  }
  return access(filepath, F_OK) != 0;
}

/**
 * @brief Opens and reads the entire content of the `termchatrc` file into a
 * buffer.
 *
 * @param output A character array to store the resulting path.
 * @param len The length of the output buffer.
 * @return 0 on success, or a non-zero error code on failure.
 */
int get_rc_contents(const char *const filename, char *const buffer,
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
