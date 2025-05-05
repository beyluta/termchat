#ifndef GLOBDEF_H
#define GLOBDEF_H

#define ERR_UNRECOVERABLE 1
#define ERR_RECOVERABLE 0
#define MAX_CHAT_BUFF_CONTEXT 65536
#define MAX_BUFF_SIZE 4096

/**
 * @brief Retrieves a token from the input based on the specified index.
 * @param tokens A pointer to the array of tokens
 * @param input The input string containing the JSON data.
 * @param index The index of the token to retrieve.
 * @param output The buffer where the extracted token will be stored.
 * @return Whether the function was successful
 */
static const int get_token_index(void *tokens, const char input[],
                                 const int index, char output[]);

/**
 * @brief Get the value of a JSON object via its key
 * @param input JSON object in string format
 * @param key key to retrieve the value from
 * @param output The buffer where the extracted token will be stored.
 * @return Whether the function was successful
 */
const int get_json_value(const char input[], const char key[], char output[]);

#endif
