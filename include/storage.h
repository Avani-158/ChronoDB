#ifndef STORAGE_H
#define STORAGE_H

#define MAX_ENTRIES 100
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 200

typedef struct
{
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} Entry;

#define MAX_HISTORY 500

typedef struct
{
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    int version;
} HistoryEntry;

int storage_set(const char *key, const char *value);

const char *storage_get(const char *key);

int storage_update(const char *key, const char *value);

int storage_delete(const char *key);

void storage_list(void);

int storage_save(const char *filename);

int storage_load(const char *filename);

void storage_history(const char *key);

#endif
