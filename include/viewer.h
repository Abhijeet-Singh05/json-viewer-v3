#ifndef JSON_VIEWER_VIEWER_H
#define JSON_VIEWER_VIEWER_H

#include <cjson/cJSON.h>
#include <stddef.h>

/* Enable terminal colors only when stdout is a terminal and NO_COLOR is unset. */
void json_viewer_init_colors(void);

/* Display the JSON tree recursively. */
void json_print_tree(const cJSON *item, const char *label, int depth);

/* Search object keys recursively. The caller initializes *matches to zero. */
void json_find_key(const cJSON *item,
                   const char *key,
                   const char *path,
                   int *matches);

/* Search scalar JSON values for a case-insensitive substring. */
void json_filter_values(const cJSON *item,
                        const char *query,
                        const char *path,
                        int *matches);

/* Resolve simple paths such as address.city, skills[1], or $.address.city.
   Quoted keys and full JSONPath syntax are not supported. */
const cJSON *json_get_path(const cJSON *root, const char *path);

/* JSON statistics. Root depth is zero. */
typedef struct JSONStats
{
    size_t total_nodes;
    size_t objects;
    size_t arrays;
    size_t strings;
    size_t numbers;
    size_t booleans;
    size_t nulls;
    size_t max_depth;
} JSONStats;

void json_calculate_stats(const cJSON *item, size_t depth, JSONStats *stats);
void json_print_stats(const JSONStats *stats);

#endif
