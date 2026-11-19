#include "parser.h"
#include "validator.h"
#include "viewer.h"
#include "fetcher.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show_help(const char *program)
{
    printf("\nJSON VIEWER V3\n\n");
    printf("Usage:\n");
    printf("  %s <file.json>                  Display JSON as a tree\n", program);
    printf("  %s --url <URL>  Fetch and display JSON from a URL\n", program);
    printf("  %s --validate <file.json>       Validate JSON syntax\n", program);
    printf("  %s --stats <file.json>          Show JSON statistics\n", program);
    printf("  %s --find <key> <file.json>     Find an object key\n", program);
    printf("  %s --filter <text> <file.json>  Search scalar values\n", program);
    printf("  %s --get <path> <file.json>     Display a value by path\n", program);
    printf("  %s --help                       Show this help\n\n", program);
    printf("Path examples: address.city, skills[1], $.education.year\n");
}

int main(int argc, char *argv[])
{
    enum Command
    {
        COMMAND_VIEW,
        COMMAND_VALIDATE,
        COMMAND_STATS,
        COMMAND_FIND,
        COMMAND_FILTER,
        COMMAND_GET
    } command = COMMAND_VIEW;

    const char *argument = NULL;
    const char *filename = NULL;
    cJSON *root;

    json_viewer_init_colors();

    if (argc == 3 && strcmp(argv[1], "--url") == 0){
        char *json_text = fetch_json_from_url(argv[2]);

        if (json_text == NULL) {
            return EXIT_FAILURE;
        }

        root = json_parse_string(json_text);

        free(json_text);

        if(root == NULL){
            return EXIT_FAILURE;
        }

        json_print_tree(root,"ROOT",0);

        cJSON_Delete(root);

        return EXIT_SUCCESS;
    }

    if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        show_help(argv[0]);
        return EXIT_SUCCESS;
    }
    else if (argc == 2)
    {
        command = COMMAND_VIEW;
        filename = argv[1];
    }
    else if (argc == 3 && strcmp(argv[1], "--validate") == 0)
    {
        return json_validate_file(argv[2]) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else if (argc == 3 && strcmp(argv[1], "--stats") == 0)
    {
        command = COMMAND_STATS;
        filename = argv[2];
    }
    else if (argc == 4 && strcmp(argv[1], "--find") == 0)
    {
        command = COMMAND_FIND;
        argument = argv[2];
        filename = argv[3];
    }
    else if (argc == 4 && strcmp(argv[1], "--filter") == 0)
    {
        command = COMMAND_FILTER;
        argument = argv[2];
        filename = argv[3];
    }
    else if (argc == 4 && strcmp(argv[1], "--get") == 0)
    {
        command = COMMAND_GET;
        argument = argv[2];
        filename = argv[3];
    }
    else
    {
        show_help(argv[0]);
        return EXIT_FAILURE;
    }

    root = json_parse_file(filename);
    if (root == NULL)
        return EXIT_FAILURE;

    switch (command)
    {
        case COMMAND_VIEW:
            printf("\n========== JSON VIEWER V3 ==========\n\n");
            json_print_tree(root, "ROOT", 0);
            break;

        case COMMAND_STATS:
        {
            JSONStats stats = {0};
            json_calculate_stats(root, 0, &stats);
            json_print_stats(&stats);
            break;
        }

        case COMMAND_FIND:
        {
            int matches = 0;
            json_find_key(root, argument, "$", &matches);
            printf("\nTotal matches: %d\n", matches);
            if (matches == 0)
                printf("Key not found.\n");
            break;
        }

        case COMMAND_FILTER:
        {
            int matches = 0;
            json_filter_values(root, argument, "$", &matches);
            printf("\nTotal matches: %d\n", matches);
            if (matches == 0)
                printf("No matching values.\n");
            break;
        }

        case COMMAND_GET:
        {
            const cJSON *result = json_get_path(root, argument);
            if (result == NULL)
            {
                fprintf(stderr, "Path not found or path syntax is unsupported: %s\n", argument);
                cJSON_Delete(root);
                return EXIT_FAILURE;
            }

            json_print_tree(result, "RESULT", 0);
            break;
        }

        case COMMAND_VALIDATE:
            /* Validation exits earlier, so this case is not normally reached. */
            break;
    }

    cJSON_Delete(root);
    return EXIT_SUCCESS;
}
