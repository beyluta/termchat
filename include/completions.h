
#ifndef COMPLETIONS_H
#define COMPLETIONS_H

#include <stddef.h>
#include <stdint.h>

typedef enum : uint8_t {
  role_type_user,
  role_type_assistant,
  role_type_developer
} role_type_t;

/**
 * @brief Adds context based on the provided input.
 * @param input The input string to process.
 * @param role_type Role of the current message
 * @return A static constant integer representing the result of the operation.
 */
size_t add_context(const char *const input, role_type_t role_type);

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
size_t get_prompt_response(const char *const api_key, const char *const model,
                           const char *const role,
                           const char *const instruction,
                           const char *const input, char *const output);

#endif
