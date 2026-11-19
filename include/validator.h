#ifndef JSON_VIEWER_VALIDATOR_H
#define JSON_VIEWER_VALIDATOR_H

/* Parse a file and print whether it contains valid JSON.
   Returns 0 when valid, non-zero otherwise. */
int json_validate_file(const char *filename);

#endif
