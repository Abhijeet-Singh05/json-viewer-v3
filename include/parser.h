#ifndef JSON_VIEWER_PARSER_H
#define JSON_VIEWER_PARSER_H

#include <cjson/cJSON.h>

/* Read and parse a JSON file. Returns a cJSON tree, or NULL on failure.
   The caller owns the returned tree and must call cJSON_Delete(). */
cJSON *json_parse_file(const char *filename);

#endif
