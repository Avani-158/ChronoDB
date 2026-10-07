#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "storage.h"

static Entry entries[MAX_ENTRIES];
static int entry_count = 0;

static Entry transaction_entries[MAX_ENTRIES];
static int transaction_entry_count = 0;
static int transaction_history_count = 0;
static int transaction_active = 0;

static HistoryEntry history[MAX_HISTORY];
static int history_count = 0;

#define MAX_SNAPSHOTS 10

typedef struct
{
    int id;
    Entry entries[MAX_ENTRIES];
    int entry_count;
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
        if (strcmp(history[i].key, key) == 0) {
            if (history[i].version >= version) {
                version = history[i].version + 1;
            }
        }
    }

    strcpy(history[history_count].key, key);
    strcpy(history[history_count].value, value);
    history[history_count].type = detect_value_type(value);
    history[history_count].version = version;
    history_count++;

    return 1;
}

int storage_set(const char *key, const char *value)
{
    if (key == NULL || value == NULL || key[0] == '\0' || strlen(key) >= MAX_KEY_LENGTH || strlen(value) >= MAX_VALUE_LENGTH) {
        return 0;
    }

    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            if (!record_history(key, value)) {
                return 0;
            }

            strcpy(entries[i].value, value);
	    entries[i].type=detect_value_type(value);
            return 1;
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
    entry_count++;

    return 1;
}

const char *storage_get(const char *key)
{
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].key, key) == 0) {
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

    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].key, key) == 0) {
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
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].key, key) == 0) {
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
    if (entry_count == 0) {
        printf("Database is empty.\n");
        return;
    }

    printf("Key\tValue\n");
    printf("-------------------------\n");

    for (int i = 0; i < entry_count; i++) {
        printf("%s\t%s\n", entries[i].key, entries[i].value);
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

    int found = 0;

    printf("\n");
    printf("========== Search Results ==========\n");

    for (int i = 0; i < entry_count; i++) {
        if (contains_case_insensitive(entries[i].key, term) || contains_case_insensitive(entries[i].value, term)) {
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

    if (fprintf(file, "CHRONODB 3\n") < 0 || fprintf(file, "ENTRIES %d\n", entry_count) < 0) {
        success = 0;
    }

    for (int i = 0; success && i < entry_count; i++) {
        if (fprintf(file, "%s %s %d\n", entries[i].key, entries[i].value, entries[i].type) < 0) {
            success = 0;
        }
    }

    if (success && fprintf(file, "HISTORY %d\n", history_count) < 0) {
        success = 0;
    }

    for (int i = 0; success && i < history_count; i++) {
        if (fprintf(file, "%s %s %d %d\n", history[i].key, history[i].value, history[i].type, history[i].version) < 0) {
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

    Entry loaded[MAX_ENTRIES];
    HistoryEntry loaded_history[MAX_HISTORY];

    int loaded_count = 0;
    int loaded_history_count = 0;
    int success = 1;

    char line[256];
    char header[32];
    int version;
    char extra;

    if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%31s %d %c", header, &version, &extra) != 2 || strcmp(header, "CHRONODB") != 0 || version != 3) {
        success = 0;
    }

    int expected_entries = -1;
    int expected_history = -1;

    if (success) {
        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "ENTRIES %d %c", &expected_entries, &extra) != 1 || expected_entries < 0 || expected_entries > MAX_ENTRIES) {
            success = 0;
        }
    }

    for (int i = 0; success && i < expected_entries; i++) {
        char key[MAX_KEY_LENGTH];
        char value[MAX_VALUE_LENGTH];
	int value_type;
	char extra;

        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%49s %199s %d  %c", key, value, &value_type,  &extra) != 3) {
            success = 0;
            break;
        }

	if (value_type < TYPE_STRING || value_type > TYPE_BOOLEAN) {
              success = 0;
	      break;
	}

        for (int j = 0; j < loaded_count; j++) {
            if (strcmp(loaded[j].key, key) == 0) {
                success = 0;
                break;
            }
        }

        if (!success) {
            break;
        }

        strcpy(loaded[loaded_count].key, key);
        strcpy(loaded[loaded_count].value, value);
	loaded[loaded_count].type = (ValueType)value_type;
        loaded_count++;
    }

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
	char extra;

        if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%49s %199s %d %d %c", key, value, &value_type,  &history_version, &extra) != 4 || history_version <= 0) {
            success = 0;
            break;
        }

	if (value_type < TYPE_STRING || value_type > TYPE_BOOLEAN) {
		success = 0;
		break;
	}

        for (int j = 0; j < loaded_history_count; j++) {
            if (strcmp(loaded_history[j].key, key) == 0 && loaded_history[j].version == history_version) {
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
        loaded_history_count++;
    }

    for (int i = 0; success && i < loaded_count; i++) {
        int latest_version = 0;
        const char *latest_value = NULL;
	ValueType latest_type = TYPE_STRING;

        for (int j = 0; j < loaded_history_count; j++) {
            if (strcmp(loaded[i].key, loaded_history[j].key) == 0 && loaded_history[j].version > latest_version) {
                latest_version = loaded_history[j].version;
                latest_value = loaded_history[j].value;
		latest_type = loaded_history[j].type;
            }
        }

        if (latest_value == NULL || strcmp(loaded[i].value, latest_value) != 0 || loaded[i].type != latest_type) {
            success = 0;
        }
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

    memcpy(entries, loaded, loaded_count * sizeof(Entry));
    entry_count = loaded_count;

    memcpy(history, loaded_history, loaded_history_count * sizeof(HistoryEntry));
    history_count = loaded_history_count;

    return 1;
}

void storage_history(const char *key)
{
    if (key == NULL || key[0] == '\0') {
        printf("Invalid key.\n");
        return;
    }

    int found = 0;

    printf("History for key: %s\n", key);
    printf("Version\tValue\n");
    printf("-------------------------\n");

    for (int i = 0; i < history_count; i++) {
        if (strcmp(history[i].key, key) == 0) {
            printf("%d\t%s\n", history[i].version, history[i].value);
            found = 1;
        }
    }

    if (!found) {
        printf("No history found for this key.\n");
    }
}

int storage_rollback(const char *key, int version)
{
    if (key == NULL || key[0] == '\0' || version <= 0) {
        return 0;
    }

    int current_index = -1;
    int history_index = -1;

    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            current_index = i;
            break;
        }
    }

    if (current_index == -1) {
        return 0;
    }

    for (int i = 0; i < history_count; i++) {
        if (strcmp(history[i].key, key) == 0 && history[i].version == version) {
            history_index = i;
            break;
        }
    }

    if (history_index == -1) {
        return 0;
    }

    char restored_value[MAX_VALUE_LENGTH];
    ValueType restored_type = history[history_index].type;

    strcpy(restored_value, history[history_index].value);

    if (!record_history(key, restored_value)) {
        return 0;
    }

    history[history_count - 1].type = restored_type;
    strcpy(entries[current_index].value, restored_value);
    entries[current_index].type = restored_type;

    return 1;
}


int storage_snapshot_create(void)
{
    if (snapshot_count >= MAX_SNAPSHOTS) {
        return 0;
    }

    snapshots[snapshot_count].id = next_snapshot_id;
    snapshots[snapshot_count].entry_count = entry_count;

    memcpy(snapshots[snapshot_count].entries, entries, entry_count * sizeof(Entry));

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

    printf("Snapshot ID\tEntries\n");
    printf("-------------------------\n");

    for (int i = 0; i < snapshot_count; i++) {
        printf("%d\t\t%d\n", snapshots[i].id, snapshots[i].entry_count);
    }
}

int storage_snapshot_restore(int snapshot_id)
{
    if (snapshot_id <= 0) {
        return 0;
    }

    for (int i = 0; i < snapshot_count; i++) {
        if (snapshots[i].id == snapshot_id) {
            memcpy(entries, snapshots[i].entries, snapshots[i].entry_count * sizeof(Entry));
            entry_count = snapshots[i].entry_count;
            return 1;
        }
    }

    return 0;
}


void storage_stats(void)
{
    printf("\n");
    printf("========== Database Statistics ==========\n");
    printf("Current entries : %d / %d\n", entry_count, MAX_ENTRIES);
    printf("History records : %d / %d\n", history_count, MAX_HISTORY);
    printf("Snapshots       : %d / %d\n", snapshot_count, MAX_SNAPSHOTS);
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
    transaction_history_count = history_count;
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

    return 1;
}


int storage_transaction_abort(void)
{
    if (!transaction_active) {
        return 0;
    }

    memcpy(entries, transaction_entries, transaction_entry_count * sizeof(Entry));
    entry_count = transaction_entry_count;
    history_count = transaction_history_count;
    transaction_active = 0;
    transaction_entry_count = 0;
    transaction_history_count = 0;

    return 1;
}


int storage_transaction_active(void)
{
    return transaction_active;
}
