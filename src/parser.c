#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "storage.h"

void parser_handle_command(const char *input)
{
    char line[100];
    char *command;
    char *key;
    char *value;
    char *extra;

    if (strlen(input) >= sizeof(line))
    {
        printf("ERR command too long\n");
        return;
    }

    strcpy(line, input);

    command = strtok(line, " \t");

    if (command == NULL)
    {
        return;
    }

    if (strcmp(command, "HELP") == 0)
    {
        if (strtok(NULL, " \t") != NULL)
        {
            printf("ERR usage: HELP\n");
            return;
        }

        printf("Available commands:\n");
        printf("  HELP\n");
        printf("  SET <key> <value>\n");
        printf("  GET <key>\n");
        printf("  UPDATE <key> <value>\n");
        printf("  DELETE <key>\n");
        printf("  LIST\n");
        printf("  SAVE\n");
        printf("  LOAD\n");
        printf("  EXIT\n");
    }
    else if (strcmp(command, "SET") == 0 ||
             strcmp(command, "UPDATE") == 0)
    {
        key = strtok(NULL, " \t");
        value = strtok(NULL, " \t");
        extra = strtok(NULL, " \t");

        if (key == NULL || value == NULL || extra != NULL)
        {
            printf("ERR usage: %s <key> <value>\n", command);
            return;
        }

        if (strlen(key) >= MAX_KEY_LENGTH ||
            strlen(value) >= MAX_VALUE_LENGTH)
        {
            printf("ERR key or value too long\n");
            return;
        }

        int success;

        if (strcmp(command, "SET") == 0)
        {
            success = storage_set(key, value);

            if (!success)
            {
                printf("ERR storage is full\n");
                return;
            }
        }
        else
        {
            success = storage_update(key, value);

            if (!success)
            {
                printf("ERR key not found\n");
                return;
            }
        }

        printf("OK\n");
    }
    else if (strcmp(command, "GET") == 0 ||
             strcmp(command, "DELETE") == 0)
    {
        key = strtok(NULL, " \t");
        extra = strtok(NULL, " \t");

        if (key == NULL || extra != NULL)
        {
            printf("ERR usage: %s <key>\n", command);
            return;
        }

        if (strlen(key) >= MAX_KEY_LENGTH)
        {
            printf("ERR key too long\n");
            return;
        }

        if (strcmp(command, "GET") == 0)
        {
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
        else
        {
            if (storage_delete(key))
            {
                printf("OK\n");
            }
            else
            {
                printf("ERR key not found\n");
            }
        }
    }
    else if (strcmp(command, "LIST") == 0)
    {
        if (strtok(NULL, " \t") != NULL)
        {
            printf("ERR usage: LIST\n");
            return;
        }

        storage_list();
    }
    else if (strcmp(command, "SAVE") == 0)
    {
         if (strtok(NULL, " \t") != NULL)
         {
                 printf("ERR usage: SAVE\n");
                 return;
         }

         if (storage_save("data/chronodb.db"))
         {
                 printf("OK: database saved\n");
         }
         else
         {
                 printf("ERR could not save database\n");
         }
    }
    else if (strcmp(command, "LOAD") == 0)
    {
        if (strtok(NULL, " \t") != NULL)
         {
                printf("ERR usage: LOAD\n");
                return;
         }

         if (storage_load("data/chronodb.db"))
         {
                printf("OK: database loaded\n");
         }
         else
        {
                printf("ERR could not load database\n");
         }
    }
    else if (strcmp(command, "EXIT") == 0)
    {
        if (strtok(NULL, " \t") != NULL)
        {
            printf("ERR usage: EXIT\n");
            return;
        }

        printf("Goodbye.\n");
    }
    else
    {
        printf("ERR unknown command. Type HELP.\n");
    }
}
