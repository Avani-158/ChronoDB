#include <stdio.h>
#include <string.h>

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

        if (strcmp(input, "EXIT") == 0)
        {
            printf("Goodbye.\n");
            break;
        }

        if (strcmp(input, "HELP") == 0)
        {
            printf("Available commands:\n");
            printf("  HELP\n");
            printf("  EXIT\n");
            continue;
        }

        if (strlen(input) == 0)
        {
            continue;
        }

        printf("ERR unknown command. Type HELP.\n");
    }

    return 0;
}
