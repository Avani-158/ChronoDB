#ifndef STORAGE_H
#define STORAGE_H

#define MAX_ENTRIES 100
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 200

typedef enum
{
    TYPE_STRING,
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_BOOLEAN
} ValueType;

typedef struct
{
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    ValueType type;
} Entry;

#define MAX_HISTORY 500

typedef struct
{
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    ValueType type;
    int version;
} HistoryEntry;

int storage_set(const char *key, const char *value);

const char *storage_get(const char *key);

int storage_update(const char *key, const char *value);

int storage_delete(const char *key);

void storage_list(void);

int storage_search(const char *term);

int storage_save(const char *filename);

int storage_load(const char *filename);

void storage_history(const char *key);

int storage_rollback(const char *key, int version);

int storage_snapshot_create(void);

void storage_snapshot_list(void);

int storage_snapshot_restore(int snapshot_id);

void storage_stats(void);

const char *storage_type_name(ValueType type);

ValueType storage_get_type(const char *key);

int storage_transaction_begin(void);

int storage_transaction_commit(void);

int storage_transaction_abort(void);

int storage_transaction_active(void);

#endif
