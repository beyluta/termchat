#include "../include/globdef.h"
#include <json.h>

bool get_json_value(const char input[], const char key[], char output[]) {
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
