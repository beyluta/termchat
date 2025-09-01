#include "../include/globdef.h"
#include <json.h>

bool get_json_value(const char input[], const char key[], char output[]) {
  StringJSON json;
  StatusJSON status;
  if ((status = StrToJSON(input, &json) != FUNC_SUCCESS)) {
    return false;
  }

  if ((status = GetProperty(json, &json, key) != FUNC_SUCCESS)) {
    return false;
  }

  if ((status = JSONToStr(json, output) != FUNC_SUCCESS)) {
    return false;
  }

  return true;
}
