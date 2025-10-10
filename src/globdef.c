#include "../include/globdef.h"
#include <json.h>

/**
 * @brief Get the value of a JSON object using the `minimal-c-json-parser`
 * Library. This function is a wrapper that invokes the functions from the
 * library directly.
 *
 * @param input JSON object in string format
 * @param key key to retrieve the value from
 * @param output The buffer where the extracted token will be stored.
 * @return The length of the json value
 */
bool get_json_value(const char *const input, const char *const key,
                    char *const output) {
  string_json_t json;
  status_json_t status;
  if ((status = ConvertStringToJson(input, &json) != FUNC_SUCCESS)) {
    return false;
  }

  if ((status = GetProperty(&json, key)) != FUNC_SUCCESS) {
    return false;
  }

  if ((status = ConvertJsonToString(json, output)) != FUNC_SUCCESS) {
    return false;
  }

  return true;
}
