#include <stdio.h>
#include <string.h>
#include "parser.h"

int main(void)
{
    char input[100];

    printf("ChronoDB 1.0\n");
    printf("Type HELP for commands.\n");

    while (1)
    {
        printf("chronodb> ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\nGoodbye.\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
        {
            continue;
        }

        parser_handle_command(input);

        if (strcmp(input, "EXIT") == 0)
        {
            break;
        }
    }

    return 0;
}
