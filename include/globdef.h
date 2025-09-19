#ifndef GLOBDEF_H
#define GLOBDEF_H

#include <stddef.h>

constexpr unsigned char ERR_UNRECOVERABLE = 1;
constexpr unsigned char ERR_RECOVERABLE = 0;
constexpr unsigned short MAX_BUFF_SIZE = 65535;
constexpr short MAX_USR_SIZE = 32767;

/**
 * @brief Get the value of a JSON object via its key
 * @param input JSON object in string format
 * @param key key to retrieve the value from
 * @param output The buffer where the extracted token will be stored.
 * @return The length of the json value
 */
bool get_json_value(const char input[], const char key[], char output[]);

#endif
