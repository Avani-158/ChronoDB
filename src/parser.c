#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "storage.h"

void parser_handle_command(const char *input)
{
    char command[20];
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];

    if (sscanf(input, "%19s", command) != 1)
    {
        return;
    }

    if (strcmp(command, "HELP") == 0)
    {
        printf("Available commands:\n");
        printf("  HELP\n");
        printf("  SET <key> <value>\n");
        printf("  GET <key>\n");
        printf("  EXIT\n");
    }
    else if (strcmp(command, "SET") == 0)
    {
        if (sscanf(input, "%*s %49s %199s", key, value) != 2)
        {
            printf("ERR usage: SET <key> <value>\n");
            return;
        }

        if (storage_set(key, value))
        {
            printf("OK\n");
        }
        else
        {
            printf("ERR storage is full\n");
        }
    }
    else if (strcmp(command, "GET") == 0)
    {
        if (sscanf(input, "%*s %49s", key) != 1)
        {
            printf("ERR usage: GET <key>\n");
            return;
        }

        const char *result = storage_get(key);

        if (result != NULL)
        {
            printf("%s\n", result);
        }
        else
        {
            printf("ERR key not found\n");
        }
    }
    else if (strcmp(command, "EXIT") == 0)
    {
        printf("Goodbye.\n");
    }
    else
    {
        printf("ERR unknown command. Type HELP.\n");
    }
}
