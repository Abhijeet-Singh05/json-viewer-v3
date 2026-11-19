#include "validator.h"
#include "parser.h"

#include <stdio.h>

int json_validate_file(const char *filename)
{
    cJSON *root = json_parse_file(filename);

    if (root == NULL)
        return 1;

    printf("Valid JSON file: %s\n", filename);
    cJSON_Delete(root);
    return 0;
}
