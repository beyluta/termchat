#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

constexpr uint8_t FILE_EXISTS = 0;
constexpr uint8_t FILE_NOT_EXISTS = 1;

/**
 * @brief Retrieves the path to the configuration file.
 * @param output A character array to store the resulting path.
 * @param len The length of the output buffer.
 * @return 0 on success, or a non-zero error code on failure.
 */
int get_rc_path(char *const output, const int len);

/**
 * @brief Checks if the configuration file exists.
 * @return 0 if the configuration file exists, 1 otherwise.
 */
int get_rc_exists();

/**
 * @brief Reads the contents of the specified configuration file.
 * @param filename The name of the configuration file to read.
 * @param buffer A character array to store the file's contents.
 * @param bufferLength Size of the buffer array
 * @return 0 on success, or a non-zero error code on failure.
 */
int get_rc_contents(const char *const filename, char *const buffer,
                    unsigned long bufferLength);

#endif
