#include <stdio.h>
#include <string.h>
#include "parser.h"

void parser_handle_command(const char *input)
{
    if (strcmp(input, "HELP") == 0)
    {
        printf("Available commands:\n");
        printf("  HELP\n");
        printf("  EXIT\n");
    }
    else if (strcmp(input, "EXIT") == 0)
    {
        printf("Goodbye.\n");
    }
    else
    {
        printf("ERR unknown command. Type HELP.\n");
    }
}
