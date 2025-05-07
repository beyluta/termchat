#ifndef GLOBDEF_H
#define GLOBDEF_H

#define ERR_UNRECOVERABLE 1
#define ERR_RECOVERABLE 0
#define MAX_CHAT_BUFF_CONTEXT 65536
#define MAX_BUFF_SIZE 4096
#define BOOLEAN unsigned char
#define TRUE 1
#define FALSE 0

/**
 * @brief Get the value of a JSON object via its key
 * @param input JSON object in string format
 * @param key key to retrieve the value from
 * @param output The buffer where the extracted token will be stored.
 * @return The length of the json value
 */
const int get_json_value(const char input[], const char key[], char output[]);

#endif
