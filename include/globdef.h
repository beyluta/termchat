#ifndef GLOBDEF_H
#define GLOBDEF_H

#include <stddef.h>
#include <stdint.h>

constexpr uint8_t ERR_UNRECOVERABLE = 1;
constexpr uint8_t ERR_RECOVERABLE = 0;
constexpr uint16_t MAX_BUFF_SIZE = 65535;
constexpr int16_t MAX_USR_SIZE = 32767;

/**
 * @brief Get the value of a JSON object via its key
 * @param input JSON object in string format
 * @param key key to retrieve the value from
 * @param output The buffer where the extracted token will be stored.
 * @return The length of the json value
 */
bool get_json_value(const char *const input, const char *const key,
                    char *const output);

#endif
