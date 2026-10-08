#ifndef STORAGE_H
#define STORAGE_H

#define MAX_ENTRIES 100
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 200
#define MAX_ENTITIES 20
#define MAX_ENTITY_NAME_LENGTH 50

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
    int entity_index;
    int record_id;
} Entry;

typedef struct
{
    char name[MAX_ENTITY_NAME_LENGTH];
} Entity;

#define MAX_RECORDS 100

typedef struct
{
    int id;
    int entity_index;
} Record;

#define MAX_HISTORY 500

typedef struct
{
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    ValueType type;
    int version;
    int entity_index;
    int record_id;
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

int storage_create_entity(const char *name);

void storage_list_entities(void);

int storage_use_entity(const char *name);

const char *storage_current_entity(void);

int storage_create_record(void);

void storage_list_records(void);

int storage_use_record(int id);

int storage_current_record(void);

int storage_has_current_context(void);

#endif
