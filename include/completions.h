
#ifndef COMPLETIONS_H
#define COMPLETIONS_H

#include <stddef.h>

/**
 * @brief Adds context based on the provided input.
 * @param input The input string to process.
 * @param is_user A boolean indicating if the context is user-specific.
 * @return A static constant integer representing the result of the operation.
 */
size_t add_context(const char input[], bool is_user);

/**
 * @brief Gets a specific value from a JSON object
 * @param input JSON string
 * @param key key of the desired value
 * @param output pointer to the array of chars
 * @return Whether the function was successful
 */
bool get_json_value(const char input[], const char key[], char output[]);

/**
 * @brief Calls the OpenAI Completions API with the user input
 * @param api_key OpenAI generated api key
 * @param model GPT model to use
 * @param role Role of the LLM: 'system' | 'user' | 'developer' | 'tool' |
 * 'assistant'
 * @param instruction instruction on what the LLM should do
 * @param input user input
 * @param output output buffer to be written to
 * @param outputSize size of the buffer
 * @return Whether the function was successful
 */
size_t get_prompt_response(const char api_key[], const char model[],
                           const char role[], const char instruction[],
                           const char input[], char output[]);

#endif
