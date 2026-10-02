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

        size_t len = strlen(input);

        if (len > 0 && input[len - 1] == '\n')
        {
            input[len - 1] = '\0';
        }
        else if (!feof(stdin))
        {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }

            printf("ERR command too long. Maximum length is 98 characters.\n");
            continue;
        }

        if (input[0] == '\0')
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
