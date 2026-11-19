#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_entire_file(const char *filename)
{
    FILE *file;
    long file_size_long;
    size_t file_size;
    size_t bytes_read;
    char *buffer;

    if (filename == NULL)
    {
        fprintf(stderr, "Error: no filename was provided.\n");
        return NULL;
    }

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Error opening JSON file");
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        perror("Error seeking in JSON file");
        fclose(file);
        return NULL;
    }

    file_size_long = ftell(file);
    if (file_size_long < 0)
    {
        perror("Error determining JSON file size");
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        perror("Error rewinding JSON file");
        fclose(file);
        return NULL;
    }

    file_size = (size_t)file_size_long;
    if ((long)file_size != file_size_long)
    {
        fprintf(stderr, "Error: JSON file is too large for this system.\n");
        fclose(file);
        return NULL;
    }

    buffer = malloc(file_size + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory for JSON file.\n");
        fclose(file);
        return NULL;
    }

    bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size || ferror(file))
    {
        fprintf(stderr, "Error: could not read the entire JSON file.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

cJSON *json_parse_file(const char *filename)
{
    char *content;
    const char *error_position = NULL;
    cJSON *root;

    content = read_entire_file(filename);
    if (content == NULL)
        return NULL;

    root = cJSON_ParseWithOpts(content, &error_position, 1);
    if (root == NULL)
    {
        if (error_position != NULL)
        {
            fprintf(stderr,
                    "Invalid JSON near byte %ld: %.40s\n",
                    (long)(error_position - content),
                    error_position);
        }
        else
        {
            fprintf(stderr, "Invalid JSON: parsing failed.\n");
        }

        free(content);
        return NULL;
    }

    free(content);
    return root;
}

cJSON *json_parse_string(const char *content){
    const char *error_position = NULL;

    if (content == NULL){
        return NULL;
    }

    cJSON *root = cJSON_ParseWithOpts(
        content,
        &error_position,
        1
    );

    if(root == NULL){
        if (error_position != NULL){
            fprintf(stderr,"Invalid JSON near: %.40s\n",error_position);
        }
        else{
            fprintf(stderr,"Invalid JSON.\n");
        }
    }

    return root;
}