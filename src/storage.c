#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "storage.h"

static Entry entries[MAX_ENTRIES];
static int entry_count = 0;

static Entity entities[MAX_ENTITIES];
static int entity_count = 0;
static int current_entity_index = -1;

static Record records[MAX_RECORDS];
static int record_count = 0;
static int current_record_index = -1;
static int next_record_id = 1;

static Entry transaction_entries[MAX_ENTRIES];
static int transaction_entry_count = 0;

static Entity transaction_entities[MAX_ENTITIES];
static int transaction_entity_count = 0;

static Record transaction_records[MAX_RECORDS];
static int transaction_record_count = 0;

static HistoryEntry transaction_history[MAX_HISTORY];
static int transaction_history_count = 0;

static int transaction_current_entity_index = -1;
static int transaction_current_record_index = -1;
static int transaction_next_record_id = 1;

static int transaction_active = 0;
static HistoryEntry history[MAX_HISTORY];
static int history_count = 0;

#define MAX_SNAPSHOTS 10

typedef struct
{
    int id;
    Entity entities[MAX_ENTITIES];
    int entity_count;
    Record records[MAX_RECORDS];
    int record_count;
    Entry entries[MAX_ENTRIES];
    int entry_count;
    int current_entity_index;
    int current_record_index;
} Snapshot;

static Snapshot snapshots[MAX_SNAPSHOTS];
static int snapshot_count = 0;
static int next_snapshot_id = 1;


static ValueType detect_value_type(const char *value)
{
    if (value == NULL || value[0] == '\0') {
        return TYPE_STRING;
    }

    if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0) {
        return TYPE_BOOLEAN;
    }

    char *endptr;
    strtol(value, &endptr, 10);

    if (*endptr == '\0') {
        return TYPE_INTEGER;
    }

    strtod(value, &endptr);

    if (*endptr == '\0') {
        return TYPE_FLOAT;
    }

    return TYPE_STRING;
}


static int record_history(const char *key, const char *value)
{
    if (history_count >= MAX_HISTORY) {
        return 0;
    }

    int version = 1;

    for (int i = 0; i < history_count; i++) {
        if (history[i].entity_index == current_entity_index && history[i].record_id == records[current_record_index].id && strcmp(history[i].key, key) == 0) {
            if (history[i].version >= version) {
                version = history[i].version + 1;
            }
        }
    }

    strcpy(history[history_count].key, key);
    strcpy(history[history_count].value, value);
    history[history_count].type = detect_value_type(value);
    history[history_count].version = version;
    history[history_count].entity_index = current_entity_index;
    history[history_count].record_id = records[current_record_index].id;
    history_count++;

    return 1;
}


static int entry_matches_context(const Entry *entry)
{
    if (current_entity_index < 0 || current_record_index < 0) {
        return entry->entity_index == -1 && entry->record_id == 0;
    }

    return entry->entity_index == current_entity_index && entry->record_id == records[current_record_index].id;
}

static int current_entity_has_record(void)
{
    if (current_entity_index < 0) {
        return 0;
    }

    if (current_record_index >= 0 && records[current_record_index].entity_index == current_entity_index) {
        return 1;
    }

    return 0;
}

int storage_has_current_context(void)
{
    return current_entity_has_record();
}

int storage_set(const char *key, const char *value)
{
    if (key == NULL || value == NULL || key[0] == '\0' || strlen(key) >= MAX_KEY_LENGTH || strlen(value) >= MAX_VALUE_LENGTH) {
        return 0;
    }

    if (current_entity_index < 0 || current_record_index < 0) {
        return 0;
    }

    for (int i = 0; i < entry_count; i++) {
        if (entry_matches_context(&entries[i]) && strcmp(entries[i].key, key) == 0) {
            return 0;
        }
    }

    if (entry_count >= MAX_ENTRIES) {
        return 0;
    }

    if (!record_history(key, value)) {
        return 0;
    }

    strcpy(entries[entry_count].key, key);
    strcpy(entries[entry_count].value, value);
    entries[entry_count].type = detect_value_type(value);
    entries[entry_count].entity_index = current_entity_index;
    entries[entry_count].record_id = records[current_record_index].id;
    entry_count++;

    return 1;
}

const char *storage_get(const char *key)
{
    if (key == NULL || current_entity_index < 0 || current_record_index < 0) {
        return NULL;
    }

    for (int i = 0; i < entry_count; i++) {
        if (entry_matches_context(&entries[i]) && strcmp(entries[i].key, key) == 0) {
            return entries[i].value;
        }
    }

    return NULL;
}

int storage_update(const char *key, const char *value)
{
    if (key == NULL || value == NULL || key[0] == '\0' || strlen(key) >= MAX_KEY_LENGTH || strlen(value) >= MAX_VALUE_LENGTH) {
        return 0;
    }

    if (current_entity_index < 0 || current_record_index < 0) {
        return 0;
    }

    for (int i = 0; i < entry_count; i++) {
        if (entry_matches_context(&entries[i]) && strcmp(entries[i].key, key) == 0) {
            if (!record_history(key, value)) {
                return 0;
            }

            strcpy(entries[i].value, value);
            entries[i].type = detect_value_type(value);
            return 1;
        }
    }

    return 0;
}

int storage_delete(const char *key)
{
    if (key == NULL || current_entity_index < 0 || current_record_index < 0) {
        return 0;
    }

    for (int i = 0; i < entry_count; i++) {
        if (entry_matches_context(&entries[i]) && strcmp(entries[i].key, key) == 0) {
            for (int j = i; j < entry_count - 1; j++) {
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
    if (current_entity_index < 0 || current_record_index < 0) {
        printf("No entity or record selected.\n");
        return;
    }

    int found = 0;

    printf("Key\tValue\n");
    printf("-------------------------\n");

    for (int i = 0; i < entry_count; i++) {
        if (entry_matches_context(&entries[i])) {
            printf("%s\t%s\n", entries[i].key, entries[i].value);
            found = 1;
        }
    }

    if (!found) {
        printf("Record is empty.\n");
    }
}

static int contains_case_insensitive(const char *text, const char *term)
{
    size_t text_length = strlen(text);
    size_t term_length = strlen(term);

    if (term_length == 0) {
        return 1;
    }

    if (term_length > text_length) {
        return 0;
    }

    for (size_t i = 0; i <= text_length - term_length; i++) {
        size_t j = 0;

        while (j < term_length && tolower((unsigned char)text[i + j]) == tolower((unsigned char)term[j])) {
            j++;
        }

        if (j == term_length) {
            return 1;
        }
    }

    return 0;
}

int storage_search(const char *term)
{
    if (term == NULL || term[0] == '\0') {
        return 0;
    }

    if (current_entity_index < 0 || current_record_index < 0) {
        return 0;
    }

    int found = 0;

    printf("\n");
    printf("========== Search Results ==========\n");

    for (int i = 0; i < entry_count; i++) {
        if (entry_matches_context(&entries[i]) && (contains_case_insensitive(entries[i].key, term) || contains_case_insensitive(entries[i].value, term))) {
            printf("%s -> %s\n", entries[i].key, entries[i].value);
            found++;
        }
    }

    if (found == 0) {
        printf("No matching entries found.\n");
    }

    printf("====================================\n\n");

    return found;
}


int storage_save(const char *filename)
{
    if (filename == NULL) {
        return 0;
    }

    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        return 0;
    }

    int success = 1;

    if (fprintf(file, "CHRONODB 5\n") < 0) {
        success = 0;
    }

    if (success && fprintf(file, "ENTITIES %d\n", entity_count) < 0) {
        success = 0;
    }

    for (int i = 0; success && i < entity_count; i++) {
        if (fprintf(file, "%s\n", entities[i].name) < 0) {
            success = 0;
        }
    }

    if (success && fprintf(file, "RECORDS %d\n", record_count) < 0) {
        success = 0;
    }

    for (int i = 0; success && i < record_count; i++) {
        if (fprintf(file, "%d %d\n", records[i].id, records[i].entity_index) < 0) {
            success = 0;
        }
    }

    if (success && fprintf(file, "CURRENT %d %d\n", current_entity_index, current_record_index) < 0) {
        success = 0;
    }

    if (success && fprintf(file, "ENTRIES %d\n", entry_count) < 0) {
        success = 0;
    }

    for (int i = 0; success && i < entry_count; i++) {
        if (fprintf(file, "%s %s %d %d %d\n", entries[i].key, entries[i].value, entries[i].type, entries[i].entity_index, entries[i].record_id) < 0) {
            success = 0;
        }
    }

    if (success && fprintf(file, "HISTORY %d\n", history_count) < 0) {
        success = 0;
    }

    for (int i = 0; success && i < history_count; i++) {
        if (fprintf(file, "%s %s %d %d %d %d\n", history[i].key, history[i].value, history[i].type, history[i].version, history[i].entity_index, history[i].record_id) < 0) {
            success = 0;
        }
    }

    if (fclose(file) != 0) {
        success = 0;
    }

    return success;
}


int storage_load(const char *filename)
{
    if (filename == NULL) {
        return 0;
    }

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return 0;
    }

    Entity loaded_entities[MAX_ENTITIES];
    Record loaded_records[MAX_RECORDS];
    Entry loaded_entries[MAX_ENTRIES];
    HistoryEntry loaded_history[MAX_HISTORY];


    int loaded_entity_count = 0;
    int loaded_record_count = 0;
    int loaded_count = 0;
    int loaded_history_count = 0;
    int loaded_current_entity = -1;
    int loaded_current_record = -1;
    int loaded_next_record_id = 1;

    int success = 1;
    char line[256];
    char header[32];
    int version;
    char extra;

    if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%31s %d %c", header, &version, &extra) != 2 || strcmp(header, "CHRONODB") != 0 || version != 5) {
        success = 0;
    }

    int expected_entities = -1;

    if (success) {
        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "ENTITIES %d %c", &expected_entities, &extra) != 1 || expected_entities < 0 || expected_entities > MAX_ENTITIES) {
            success = 0;
        }
    }

    for (int i = 0; success && i < expected_entities; i++) {
        if (fgets(line, sizeof(line), file) == NULL) {
            success = 0;
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0' || strlen(line) >= MAX_ENTITY_NAME_LENGTH) {
            success = 0;
            break;
        }

        for (int j = 0; j < loaded_entity_count; j++) {
            if (strcmp(loaded_entities[j].name, line) == 0) {
                success = 0;
                break;
            }
        }

        if (!success) {
            break;
        }

        strcpy(loaded_entities[loaded_entity_count].name, line);
        loaded_entity_count++;
    }

    int expected_records = -1;

    if (success) {
        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "RECORDS %d %c", &expected_records, &extra) != 1 || expected_records < 0 || expected_records > MAX_RECORDS) {
            success = 0;
        }
    }

    for (int i = 0; success && i < expected_records; i++) {
        int id;
        int entity_index;

        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%d %d %c", &id, &entity_index, &extra) != 2 || id <= 0 || entity_index < 0 || entity_index >= loaded_entity_count) {
            success = 0;
            break;
        }

        for (int j = 0; j < loaded_record_count; j++) {
            if (loaded_records[j].id == id) {
                success = 0;
                break;
            }
        }

        if (!success) {
            break;
        }

        loaded_records[loaded_record_count].id = id;
        loaded_records[loaded_record_count].entity_index = entity_index;
        loaded_record_count++;

        if (id >= loaded_next_record_id) {
            loaded_next_record_id = id + 1;
        }
    }

    if (success) {
        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "CURRENT %d %d %c", &loaded_current_entity, &loaded_current_record, &extra) != 2) {
            success = 0;
        }
    }

    if (success && (loaded_current_entity < -1 || loaded_current_entity >= loaded_entity_count || loaded_current_record < -1 || loaded_current_record >= loaded_record_count)) {
        success = 0;
    }

    int expected_entries = -1;

    if (success) {
        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "ENTRIES %d %c", &expected_entries, &extra) != 1 || expected_entries < 0 || expected_entries > MAX_ENTRIES) {
            success = 0;
        }
    }

    for (int i = 0; success && i < expected_entries; i++) {
        char key[MAX_KEY_LENGTH];
        char value[MAX_VALUE_LENGTH];
        int value_type;
        int entity_index;
        int record_id;

        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%49s %199s %d %d %d %c", key, value, &value_type, &entity_index, &record_id, &extra) != 5) {
            success = 0;
            break;
        }

        if (value_type < TYPE_STRING || value_type > TYPE_BOOLEAN || entity_index < 0 || entity_index >= loaded_entity_count || record_id <= 0) {
            success = 0;
            break;
        }

        int record_exists = 0;

        for (int j = 0; j < loaded_record_count; j++) {
            if (loaded_records[j].id == record_id && loaded_records[j].entity_index == entity_index) {
                record_exists = 1;
                break;
            }
        }

        if (!record_exists) {
            success = 0;
            break;
        }

        for (int j = 0; j < loaded_count; j++) {
            if (strcmp(loaded_entries[j].key, key) == 0 && loaded_entries[j].entity_index == entity_index && loaded_entries[j].record_id == record_id) {
                success = 0;
                break;
            }
        }

        if (!success) {
            break;
        }

        strcpy(loaded_entries[loaded_count].key, key);
        strcpy(loaded_entries[loaded_count].value, value);
        loaded_entries[loaded_count].type = (ValueType)value_type;
        loaded_entries[loaded_count].entity_index = entity_index;
        loaded_entries[loaded_count].record_id = record_id;
        loaded_count++;
    }

    int expected_history = -1;

    if (success) {
        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "HISTORY %d %c", &expected_history, &extra) != 1 || expected_history < 0 || expected_history > MAX_HISTORY) {
            success = 0;
        }
    }

    for (int i = 0; success && i < expected_history; i++) {
        char key[MAX_KEY_LENGTH];
        char value[MAX_VALUE_LENGTH];
        int value_type;
        int history_version;
        int entity_index;
        int record_id;

        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%49s %199s %d %d %d %d %c", key, value, &value_type, &history_version, &entity_index, &record_id, &extra) != 6 || history_version <= 0) {
            success = 0;
            break;
        }

        if (value_type < TYPE_STRING || value_type > TYPE_BOOLEAN || entity_index < 0 || entity_index >= loaded_entity_count || record_id <= 0) {
            success = 0;
            break;
        }

        int record_exists = 0;

        for (int j = 0; j < loaded_record_count; j++) {
            if (loaded_records[j].id == record_id && loaded_records[j].entity_index == entity_index) {
                record_exists = 1;
                break;
            }
        }

        if (!record_exists) {
            success = 0;
            break;
        }

        for (int j = 0; j < loaded_history_count; j++) {
            if (strcmp(loaded_history[j].key, key) == 0 && loaded_history[j].version == history_version && loaded_history[j].entity_index == entity_index && loaded_history[j].record_id == record_id) {
                success = 0;
                break;
            }
        }

        if (!success) {
            break;
        }

        strcpy(loaded_history[loaded_history_count].key, key);
        strcpy(loaded_history[loaded_history_count].value, value);
        loaded_history[loaded_history_count].type = (ValueType)value_type;
        loaded_history[loaded_history_count].version = history_version;
        loaded_history[loaded_history_count].entity_index = entity_index;
        loaded_history[loaded_history_count].record_id = record_id;
        loaded_history_count++;
    }

    if (success && fgets(line, sizeof(line), file) != NULL) {
        success = 0;
    }

    if (ferror(file)) {
        success = 0;
    }

    fclose(file);

    if (!success) {
        return 0;
    }

    memcpy(entities, loaded_entities, loaded_entity_count * sizeof(Entity));
    entity_count = loaded_entity_count;

    memcpy(records, loaded_records, loaded_record_count * sizeof(Record));
    record_count = loaded_record_count;

    memcpy(entries, loaded_entries, loaded_count * sizeof(Entry));
    entry_count = loaded_count;

    memcpy(history, loaded_history, loaded_history_count * sizeof(HistoryEntry));
    history_count = loaded_history_count;

    current_entity_index = loaded_current_entity;
    current_record_index = loaded_current_record;
    next_record_id = loaded_next_record_id;

    return 1;
}


int storage_rollback(const char *key, int version)
{
    if (key == NULL || key[0] == '\0' || version <= 0) {
        return 0;
    }

    if (current_entity_index < 0 || current_record_index < 0) {
        return 0;
    }

    int target_index = -1;

    for (int i = 0; i < history_count; i++) {
        if (history[i].entity_index == current_entity_index && history[i].record_id == records[current_record_index].id && strcmp(history[i].key, key) == 0 && history[i].version == version) {
            target_index = i;
            break;
        }
    }

    if (target_index < 0) {
        return 0;
    }

    int entry_index = -1;

    for (int i = 0; i < entry_count; i++) {
        if (entry_matches_context(&entries[i]) && strcmp(entries[i].key, key) == 0) {
            entry_index = i;
            break;
        }
    }

    if (entry_index < 0) {
        return 0;
    }

    if (!record_history(key, history[target_index].value)) {
        return 0;
    }

    strcpy(entries[entry_index].value, history[target_index].value);
    entries[entry_index].type = history[target_index].type;

    return 1;
}


int storage_snapshot_restore(int snapshot_id)
{
    if (snapshot_id <= 0) {
        return 0;
    }

    for (int i = 0; i < snapshot_count; i++) {
        if (snapshots[i].id == snapshot_id) {
            memcpy(entities, snapshots[i].entities, snapshots[i].entity_count * sizeof(Entity));
            entity_count = snapshots[i].entity_count;

            memcpy(records, snapshots[i].records, snapshots[i].record_count * sizeof(Record));
            record_count = snapshots[i].record_count;

            memcpy(entries, snapshots[i].entries, snapshots[i].entry_count * sizeof(Entry));
            entry_count = snapshots[i].entry_count;

            current_entity_index = snapshots[i].current_entity_index;
            current_record_index = snapshots[i].current_record_index;

            next_record_id = 1;

            for (int j = 0; j < record_count; j++) {
                if (records[j].id >= next_record_id) {
                    next_record_id = records[j].id + 1;
                }
            }

            return 1;
        }
    }

    return 0;
}


void storage_stats(void)
{
    printf("\n");
    printf("========== Database Statistics ==========\n");
    printf("Entities        : %d / %d\n", entity_count, MAX_ENTITIES);
    printf("Records         : %d / %d\n", record_count, MAX_RECORDS);
    printf("Current entries : %d / %d\n", entry_count, MAX_ENTRIES);
    printf("History records : %d / %d\n", history_count, MAX_HISTORY);
    printf("Snapshots       : %d / %d\n", snapshot_count, MAX_SNAPSHOTS);
    printf("Next record ID  : %d\n", next_record_id);
    printf("Next snapshot ID: %d\n", next_snapshot_id);
    printf("=========================================\n\n");
}


const char *storage_type_name(ValueType type)
{
    switch (type) {
        case TYPE_STRING:
            return "STRING";
        case TYPE_INTEGER:
            return "INTEGER";
        case TYPE_FLOAT:
            return "FLOAT";
        case TYPE_BOOLEAN:
            return "BOOLEAN";
        default:
            return "UNKNOWN";
    }
}


ValueType storage_get_type(const char *key)
{
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            return entries[i].type;
        }
    }

    return TYPE_STRING;
}


int storage_transaction_begin(void)
{
    if (transaction_active) {
        return 0;
    }

    memcpy(transaction_entries, entries, entry_count * sizeof(Entry));
    transaction_entry_count = entry_count;

    memcpy(transaction_entities, entities, entity_count * sizeof(Entity));
    transaction_entity_count = entity_count;

    memcpy(transaction_records, records, record_count * sizeof(Record));
    transaction_record_count = record_count;

    memcpy(transaction_history, history, history_count * sizeof(HistoryEntry));
    transaction_history_count = history_count;

    transaction_current_entity_index = current_entity_index;
    transaction_current_record_index = current_record_index;
    transaction_next_record_id = next_record_id;

    transaction_active = 1;

    return 1;
}

int storage_transaction_commit(void)
{
    if (!transaction_active) {
        return 0;
    }

    transaction_active = 0;
    transaction_entry_count = 0;
    transaction_entity_count = 0;
    transaction_record_count = 0;
    transaction_history_count = 0;

    return 1;
}

int storage_transaction_abort(void)
{
    if (!transaction_active) {
        return 0;
    }

    memcpy(entries, transaction_entries, transaction_entry_count * sizeof(Entry));
    entry_count = transaction_entry_count;

    memcpy(entities, transaction_entities, transaction_entity_count * sizeof(Entity));
    entity_count = transaction_entity_count;

    memcpy(records, transaction_records, transaction_record_count * sizeof(Record));
    record_count = transaction_record_count;

    memcpy(history, transaction_history, transaction_history_count * sizeof(HistoryEntry));
    history_count = transaction_history_count;

    current_entity_index = transaction_current_entity_index;
    current_record_index = transaction_current_record_index;
    next_record_id = transaction_next_record_id;

    transaction_active = 0;
    transaction_entry_count = 0;
    transaction_entity_count = 0;
    transaction_record_count = 0;
    transaction_history_count = 0;

    return 1;
}

int storage_transaction_active(void)
{
    return transaction_active;
}

int storage_create_entity(const char *name)
{
    if (name == NULL || name[0] == '\0' || strlen(name) >= MAX_ENTITY_NAME_LENGTH) {
        return 0;
    }

    for (int i = 0; i < entity_count; i++) {
        if (strcmp(entities[i].name, name) == 0) {
            return 0;
        }
    }

    if (entity_count >= MAX_ENTITIES || record_count >= MAX_RECORDS) {
        return 0;
    }

    strcpy(entities[entity_count].name, name);

    records[record_count].id = next_record_id;
    records[record_count].entity_index = entity_count;

    current_entity_index = entity_count;
    current_record_index = record_count;

    entity_count++;
    record_count++;
    next_record_id++;

    return 1;
}


void storage_list_entities(void)
{
    if (entity_count == 0) {
        printf("No entities created.\n");
        return;
    }

    printf("Entities:\n");

    for (int i = 0; i < entity_count; i++) {
        printf("  %s\n", entities[i].name);
    }
}


int storage_use_entity(const char *name)
{
    if (name == NULL) {
        return 0;
    }

    for (int i = 0; i < entity_count; i++) {
        if (strcmp(entities[i].name, name) == 0) {
            current_entity_index = i;
            current_record_index = -1;

            for (int j = 0; j < record_count; j++) {
                if (records[j].entity_index == i) {
                    current_record_index = j;
                    break;
                }
            }

            return current_record_index >= 0;
        }
    }

    return 0;
}

const char *storage_current_entity(void)
{
    if (current_entity_index < 0) {
        return NULL;
    }

    return entities[current_entity_index].name;
}

int storage_create_record(void)
{
    if (current_entity_index < 0) {
        return 0;
    }

    if (record_count >= MAX_RECORDS) {
        return 0;
    }

    records[record_count].id = next_record_id;
    records[record_count].entity_index = current_entity_index;
    current_record_index = record_count;
    record_count++;
    next_record_id++;

    return 1;
}

void storage_list_records(void)
{
    if (current_entity_index < 0) {
        printf("ERR no entity selected\n");
        return;
    }

    int found = 0;

    printf("Records for entity '%s':\n", entities[current_entity_index].name);

    for (int i = 0; i < record_count; i++) {
        if (records[i].entity_index == current_entity_index) {
            printf("  Record %d", records[i].id);

            if (i == current_record_index) {
                printf("  [current]");
            }

            printf("\n");
            found = 1;
        }
    }

    if (!found) {
        printf("  No records.\n");
    }
}

int storage_use_record(int id)
{
    if (current_entity_index < 0) {
        return 0;
    }

    for (int i = 0; i < record_count; i++) {
        if (records[i].id == id && records[i].entity_index == current_entity_index) {
            current_record_index = i;
            return 1;
        }
    }

    return 0;
}

int storage_current_record(void)
{
    if (current_record_index < 0) {
        return -1;
    }

    return records[current_record_index].id;
}

void storage_history(const char *key)
{
    if (key == NULL || key[0] == '\0') {
        printf("Invalid key.\n");
        return;
    }

    if (current_entity_index < 0 || current_record_index < 0) {
        printf("ERR no record selected\n");
        return;
    }

    int found = 0;

    printf("History for key: %s\n", key);
    printf("Version\tValue\n");
    printf("-------------------------\n");

    for (int i = 0; i < history_count; i++) {
        if (history[i].entity_index == current_entity_index && history[i].record_id == records[current_record_index].id && strcmp(history[i].key, key) == 0) {
            printf("%d\t%s\n", history[i].version, history[i].value);
            found = 1;
        }
    }

    if (!found) {
        printf("No history found for this key.\n");
    }

}


int storage_snapshot_create(void)
{
    if (snapshot_count >= MAX_SNAPSHOTS) {
        return 0;
    }

    snapshots[snapshot_count].id = next_snapshot_id;

    snapshots[snapshot_count].entity_count = entity_count;
    memcpy(snapshots[snapshot_count].entities, entities, entity_count * sizeof(Entity));

    snapshots[snapshot_count].record_count = record_count;
    memcpy(snapshots[snapshot_count].records, records, record_count * sizeof(Record));

    snapshots[snapshot_count].entry_count = entry_count;
    memcpy(snapshots[snapshot_count].entries, entries, entry_count * sizeof(Entry));

    snapshots[snapshot_count].current_entity_index = current_entity_index;
    snapshots[snapshot_count].current_record_index = current_record_index;

    snapshot_count++;
    next_snapshot_id++;

    return snapshots[snapshot_count - 1].id;
}

void storage_snapshot_list(void)
{
    if (snapshot_count == 0) {
        printf("No snapshots available.\n");
        return;
    }

    printf("Snapshot ID\tEntities\tRecords\t\tEntries\n");
    printf("------------------------------------------------\n");

    for (int i = 0; i < snapshot_count; i++) {
        printf("%d\t\t%d\t\t%d\t\t%d\n", snapshots[i].id, snapshots[i].entity_count, snapshots[i].record_count, snapshots[i].entry_count);
    }
}
