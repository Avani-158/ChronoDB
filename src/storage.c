#include <stdio.h>
#include <string.h>
#include "storage.h"
#include <stddef.h>

static Entry entries[MAX_ENTRIES];
static int entry_count = 0;

int storage_set(const char *key, const char *value)
{
    if (key == NULL || value == NULL || key[0] == '\0' || strlen(key) >= MAX_KEY_LENGTH || strlen(value) >= MAX_VALUE_LENGTH)
    {
        return 0;
    }

    for (int i = 0; i < entry_count; i++)
    {
        if (strcmp(entries[i].key, key) == 0)
        {
            strcpy(entries[i].value, value);
            return 1;
        }
    }

    if (entry_count >= MAX_ENTRIES)
    {
        return 0;
    }

    strcpy(entries[entry_count].key, key);
    strcpy(entries[entry_count].value, value);
    entry_count++;

    return 1;
}

const char *storage_get(const char *key)
{
    for (int i = 0; i < entry_count; i++)
    {
        if (strcmp(entries[i].key, key) == 0)
        {
            return entries[i].value;
        }
    }

    return NULL;
}


int storage_update(const char *key, const char *value)
{
    if (key == NULL || value == NULL || key[0] == '\0' || strlen(key) >= MAX_KEY_LENGTH || strlen(value) >= MAX_VALUE_LENGTH)
    {
        return 0;
    }

    for (int i = 0; i < entry_count; i++)
    {
        if (strcmp(entries[i].key, key) == 0)
        {
            strcpy(entries[i].value, value);
            return 1;
        }
    }

    return 0;
}

int storage_delete(const char *key)
{
    for (int i = 0; i < entry_count; i++)
    {
        if (strcmp(entries[i].key, key) == 0)
        {
            for (int j = i; j < entry_count - 1; j++)
            {
                entries[j] = entries[j + 1];
            }

            entry_count--;
            return 1;
        }
    }

    return 0;
}

void storage_list(void)
{
    if (entry_count == 0)
    {
        printf("Database is empty.\n");
        return;
    }

    printf("Key\tValue\n");
    printf("-------------------------\n");

    for (int i = 0; i < entry_count; i++)
    {
        printf("%s\t%s\n", entries[i].key, entries[i].value);
    }
}
