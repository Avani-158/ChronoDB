#include <stdio.h>
#include <string.h>
#include "storage.h"
#include <stddef.h>

static Entry entries[MAX_ENTRIES];
static int entry_count = 0;

static HistoryEntry history[MAX_HISTORY];
static int history_count = 0;

static int record_history(const char *key, const char *value)
{
    if (history_count >= MAX_HISTORY)
    {
        return 0;
    }

    int version = 1;

    for (int i = 0; i < history_count; i++)
    {
        if (strcmp(history[i].key, key) == 0)
        {
            if (history[i].version >= version)
            {
                version = history[i].version + 1;
            }
        }
    }

    strcpy(history[history_count].key, key);
    strcpy(history[history_count].value, value);
    history[history_count].version = version;

    history_count++;

    return 1;
}

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
		if (!record_history(key,value))
		{
			return 0;
		}

            strcpy(entries[i].value, value);
            return 1;
        }
    }

    if (entry_count >= MAX_ENTRIES)
    {
        return 0;
    }

    if (!record_history(key,value))
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
		if (!record_history(key,value))
		{
			return 0;
		}

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

int storage_save(const char *filename)
{
    if (filename == NULL)
    {
        return 0;
    }

    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        return 0;
    }

    for (int i = 0; i < entry_count; i++)
    {
        if (fprintf(file, "%s %s\n",
                    entries[i].key,
                    entries[i].value) < 0)
        {
            fclose(file);
            return 0;
        }
    }

    if (fclose(file) != 0)
    {
        return 0;
    }

    return 1;
}

int storage_load(const char *filename)
{
    if (filename == NULL)
    {
        return 0;
    }

    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        return 0;
    }

    Entry loaded[MAX_ENTRIES];
    int loaded_count = 0;
    char line[256];
    int success = 1;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        size_t len = strlen(line);

        if (len > 0 && line[len - 1] != '\n' && !feof(file))
        {
            success = 0;
            break;
        }

        char key[MAX_KEY_LENGTH];
        char value[MAX_VALUE_LENGTH];
        char extra;

        if (sscanf(line, "%49s %199s %c",
                   key, value, &extra) != 2)
        {
            success = 0;
            break;
        }

        if (loaded_count >= MAX_ENTRIES)
        {
            success = 0;
            break;
        }

        for (int i = 0; i < loaded_count; i++)
        {
            if (strcmp(loaded[i].key, key) == 0)
            {
                success = 0;
                break;
            }
        }

        if (!success)
        {
            break;
        }

        strcpy(loaded[loaded_count].key, key);
        strcpy(loaded[loaded_count].value, value);
        loaded_count++;
    }

    if (ferror(file))
    {
        success = 0;
    }

    fclose(file);

    if (!success)
    {
        return 0;
    }

    memcpy(entries, loaded, loaded_count * sizeof(Entry));
    entry_count = loaded_count;

    return 1;
}

void storage_history(const char *key)
{
    if (key == NULL || key[0] == '\0')
    {
        printf("Invalid key.\n");
        return;
    }

    int found = 0;

    printf("History for key: %s\n", key);
    printf("Version\tValue\n");
    printf("-------------------------\n");

    for (int i = 0; i < history_count; i++)
    {
        if (strcmp(history[i].key, key) == 0)
        {
            printf("%d\t%s\n", history[i].version, history[i].value);
            found = 1;
        }
    }

    if (!found)
    {
        printf("No history found for this key.\n");
    }
}


int storage_rollback(const char *key, int version)
{
    if (key == NULL || key[0] == '\0' || version <= 0)
    {
        return 0;
    }

    int current_index = -1;
    int history_index = -1;

    for (int i = 0; i < entry_count; i++)
    {
        if (strcmp(entries[i].key, key) == 0)
        {
            current_index = i;
            break;
        }
    }

    if (current_index == -1)
    {
        return 0;
    }

    for (int i = 0; i < history_count; i++)
    {
        if (strcmp(history[i].key, key) == 0 &&
            history[i].version == version)
        {
            history_index = i;
            break;
        }
    }

    if (history_index == -1)
    {
        return 0;
    }

    char restored_value[MAX_VALUE_LENGTH];
    strcpy(restored_value, history[history_index].value);

    if (!record_history(key, restored_value))
    {
        return 0;
    }

    strcpy(entries[current_index].value, restored_value);

    return 1;
}
