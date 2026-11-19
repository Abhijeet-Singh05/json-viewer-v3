#include "viewer.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *COLOR_RESET = "";
static const char *COLOR_CYAN = "";
static const char *COLOR_GREEN = "";
static const char *COLOR_YELLOW = "";
static const char *COLOR_MAGENTA = "";
static const char *COLOR_RED = "";

void json_viewer_init_colors(void)
{
    if (isatty(STDOUT_FILENO) && getenv("NO_COLOR") == NULL)
    {
        COLOR_RESET = "\033[0m";
        COLOR_CYAN = "\033[36m";
        COLOR_GREEN = "\033[32m";
        COLOR_YELLOW = "\033[33m";
        COLOR_MAGENTA = "\033[35m";
        COLOR_RED = "\033[31m";
    }
}

static void print_indent(int depth)
{
    int i;
    for (i = 0; i < depth; i++)
        printf("    ");
}

static void print_scalar_value(const cJSON *item)
{
    char *value;
    const char *color = COLOR_RESET;

    value = cJSON_PrintUnformatted(item);
    if (value == NULL)
    {
        printf("(unable to display value)");
        return;
    }

    if (cJSON_IsString(item))
        color = COLOR_GREEN;
    else if (cJSON_IsNumber(item))
        color = COLOR_YELLOW;
    else if (cJSON_IsBool(item))
        color = COLOR_MAGENTA;
    else if (cJSON_IsNull(item))
        color = COLOR_RED;

    printf("%s%s%s", color, value, COLOR_RESET);
    cJSON_free(value);
}

void json_print_tree(const cJSON *item, const char *label, int depth)
{
    const cJSON *child;
    int index;

    if (item == NULL || label == NULL || depth < 0)
        return;

    print_indent(depth);

    if (cJSON_IsObject(item))
    {
        printf("%s%s%s (Object)\n", COLOR_CYAN, label, COLOR_RESET);
        child = item->child;

        if (child == NULL)
        {
            print_indent(depth + 1);
            printf("(empty)\n");
        }

        while (child != NULL)
        {
            json_print_tree(child,
                            child->string != NULL ? child->string : "(unnamed)",
                            depth + 1);
            child = child->next;
        }
    }
    else if (cJSON_IsArray(item))
    {
        printf("%s%s%s (Array)\n", COLOR_CYAN, label, COLOR_RESET);
        child = item->child;
        index = 0;

        if (child == NULL)
        {
            print_indent(depth + 1);
            printf("(empty)\n");
        }

        while (child != NULL)
        {
            char index_label[32];
            snprintf(index_label, sizeof(index_label), "[%d]", index);
            json_print_tree(child, index_label, depth + 1);
            child = child->next;
            index++;
        }
    }
    else
    {
        printf("%s%s%s: ", COLOR_CYAN, label, COLOR_RESET);
        print_scalar_value(item);
        putchar('\n');
    }
}

static char *make_path(const char *parent, const char *part, int is_array)
{
    size_t parent_length;
    size_t part_length;
    size_t required;
    char *path;

    if (parent == NULL || part == NULL)
        return NULL;

    parent_length = strlen(parent);
    part_length = strlen(part);
    required = parent_length + part_length + (is_array ? 3U : 2U);

    path = malloc(required);
    if (path == NULL)
        return NULL;

    if (is_array)
        snprintf(path, required, "%s[%s]", parent, part);
    else
        snprintf(path, required, "%s.%s", parent, part);

    return path;
}

void json_find_key(const cJSON *item,
                   const char *key,
                   const char *path,
                   int *matches)
{
    const cJSON *child;

    if (item == NULL || key == NULL || path == NULL || matches == NULL)
        return;

    if (cJSON_IsObject(item))
    {
        child = item->child;
        while (child != NULL)
        {
            const char *name = child->string != NULL ? child->string : "(unnamed)";
            char *child_path = make_path(path, name, 0);

            if (child_path == NULL)
            {
                fprintf(stderr, "Warning: search stopped because memory allocation failed.\n");
                return;
            }

            if (strcmp(name, key) == 0)
            {
                printf("\nFound at: %s\n", child_path);
                json_print_tree(child, "VALUE", 0);
                (*matches)++;
            }

            json_find_key(child, key, child_path, matches);
            free(child_path);
            child = child->next;
        }
    }
    else if (cJSON_IsArray(item))
    {
        int index = 0;
        child = item->child;

        while (child != NULL)
        {
            char index_text[32];
            char *child_path;

            snprintf(index_text, sizeof(index_text), "%d", index);
            child_path = make_path(path, index_text, 1);
            if (child_path == NULL)
            {
                fprintf(stderr, "Warning: search stopped because memory allocation failed.\n");
                return;
            }

            json_find_key(child, key, child_path, matches);
            free(child_path);
            child = child->next;
            index++;
        }
    }
}

static int contains_text_case_insensitive(const char *text, const char *query)
{
    const char *start;

    if (text == NULL || query == NULL)
        return 0;

    if (*query == '\0')
        return 1;

    for (start = text; *start != '\0'; start++)
    {
        const char *left = start;
        const char *right = query;

        while (*left != '\0' && *right != '\0' &&
               tolower((unsigned char)*left) == tolower((unsigned char)*right))
        {
            left++;
            right++;
        }

        if (*right == '\0')
            return 1;
    }

    return 0;
}

void json_filter_values(const cJSON *item,
                        const char *query,
                        const char *path,
                        int *matches)
{
    const cJSON *child;
    int index;

    if (item == NULL || query == NULL || path == NULL || matches == NULL)
        return;

    if (cJSON_IsObject(item) || cJSON_IsArray(item))
    {
        child = item->child;
        index = 0;

        while (child != NULL)
        {
            char index_text[32];
            char *child_path;

            if (cJSON_IsArray(item))
            {
                snprintf(index_text, sizeof(index_text), "%d", index);
                child_path = make_path(path, index_text, 1);
            }
            else
            {
                const char *name = child->string != NULL ? child->string : "(unnamed)";
                child_path = make_path(path, name, 0);
            }

            if (child_path == NULL)
            {
                fprintf(stderr, "Warning: filtering stopped because memory allocation failed.\n");
                return;
            }

            json_filter_values(child, query, child_path, matches);
            free(child_path);
            child = child->next;
            index++;
        }
    }
    else
    {
        char *value = cJSON_PrintUnformatted(item);
        if (value == NULL)
            return;

        if (contains_text_case_insensitive(value, query))
        {
            printf("%s = ", path);
            print_scalar_value(item);
            putchar('\n');
            (*matches)++;
        }

        cJSON_free(value);
    }
}

const cJSON *json_get_path(const cJSON *root, const char *path)
{
    const cJSON *current = root;
    const char *cursor = path;

    if (root == NULL || path == NULL || *path == '\0')
        return NULL;

    if (*cursor == '$')
    {
        cursor++;
        if (*cursor != '\0' && *cursor != '.' && *cursor != '[')
            return NULL;
    }

    while (*cursor != '\0')
    {
        if (*cursor == '.')
            cursor++;

        if (*cursor == '\0' || *cursor == '.')
            return NULL;

        if (*cursor == '[')
        {
            char *end;
            long index;

            cursor++;
            if (!isdigit((unsigned char)*cursor))
                return NULL;

            index = strtol(cursor, &end, 10);
            if (end == cursor || *end != ']' || index < 0 || index > INT_MAX)
                return NULL;

            cursor = end + 1;
            if (!cJSON_IsArray(current))
                return NULL;

            current = cJSON_GetArrayItem(current, (int)index);
        }
        else
        {
            char key[256];
            size_t length = 0;

            while (*cursor != '\0' && *cursor != '.' && *cursor != '[')
            {
                if (length >= sizeof(key) - 1)
                    return NULL;
                key[length++] = *cursor++;
            }

            if (length == 0 || !cJSON_IsObject(current))
                return NULL;

            key[length] = '\0';
            current = cJSON_GetObjectItemCaseSensitive(current, key);
        }

        if (current == NULL)
            return NULL;
    }

    return current;
}

void json_calculate_stats(const cJSON *item, size_t depth, JSONStats *stats)
{
    const cJSON *child;

    if (item == NULL || stats == NULL)
        return;

    stats->total_nodes++;
    if (depth > stats->max_depth)
        stats->max_depth = depth;

    if (cJSON_IsObject(item))
        stats->objects++;
    else if (cJSON_IsArray(item))
        stats->arrays++;
    else if (cJSON_IsString(item))
        stats->strings++;
    else if (cJSON_IsNumber(item))
        stats->numbers++;
    else if (cJSON_IsBool(item))
        stats->booleans++;
    else if (cJSON_IsNull(item))
        stats->nulls++;

    if (cJSON_IsObject(item) || cJSON_IsArray(item))
    {
        child = item->child;
        while (child != NULL)
        {
            json_calculate_stats(child, depth + 1, stats);
            child = child->next;
        }
    }
}

void json_print_stats(const JSONStats *stats)
{
    if (stats == NULL)
        return;

    printf("\n========== JSON STATISTICS ==========\n");
    printf("Total nodes  : %zu\n", stats->total_nodes);
    printf("Objects      : %zu\n", stats->objects);
    printf("Arrays       : %zu\n", stats->arrays);
    printf("Strings      : %zu\n", stats->strings);
    printf("Numbers      : %zu\n", stats->numbers);
    printf("Booleans     : %zu\n", stats->booleans);
    printf("Null values  : %zu\n", stats->nulls);
    printf("Max depth    : %zu\n", stats->max_depth);
    printf("=====================================\n");
}
