
#ifndef COMPLETIONS_H
#define COMPLETIONS_H

/**
 * @brief Writes the response string to the output string
 * @param ptr pointer to the string
 * @param size size of the string
 * @param nmemb size of the memory in bytes
 * @param output buffer to be written to
 * @return Total size of the buffer
 */
static int write_func(void *ptr, int size, int nmemb, char output[]);

/**
 * @brief Gets a specific value from a JSON object
 * @param input JSON string
 * @param key key of the desired value
 * @param output pointer to the array of chars
 * @return Whether the function was successful
 */
const int get_json_value(const char input[], const char key[], char output[]);

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
const int get_prompt_response(const char api_key[], const char model[],
                              const char role[], const char instruction[],
                              const char input[], char *output,
                              const int outputSize);

#endif
