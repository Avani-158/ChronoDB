#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "storage.h"

#define TEST_DB_FILE "/tmp/chronodb_test.db"

static void test_entity_and_values(void)
{
    assert(storage_create_entity("TEST"));
    assert(storage_has_current_context());

    assert(storage_set("name", "Avani"));
    assert(strcmp(storage_get("name"), "Avani") == 0);

    assert(storage_set("age", "21"));
    assert(strcmp(storage_get("age"), "21") == 0);
    assert(storage_get_type("age") == TYPE_INTEGER);

    assert(storage_set("score", "98.5"));
    assert(strcmp(storage_get("score"), "98.5") == 0);
    assert(storage_get_type("score") == TYPE_FLOAT);

    assert(storage_set("active", "true"));
    assert(strcmp(storage_get("active"), "true") == 0);
    assert(storage_get_type("active") == TYPE_BOOLEAN);

    assert(storage_set("name", "Duplicate") == 0);
    assert(storage_update("missing", "value") == 0);
    assert(storage_delete("active"));
    assert(storage_get("active") == NULL);
    assert(storage_delete("active") == 0);
}

static void test_history_and_rollback(void)
{
    assert(storage_update("name", "Deeya"));
    assert(storage_update("name", "Rahul"));
    assert(storage_rollback("name", 1));
    assert(strcmp(storage_get("name"), "Avani") == 0);
}

static void test_records(void)
{
    assert(storage_create_record());
    assert(storage_current_record() == 2);

    assert(storage_set("name", "SecondRecord"));
    assert(strcmp(storage_get("name"), "SecondRecord") == 0);

    assert(storage_use_record(1));
    assert(strcmp(storage_get("name"), "Avani") == 0);

    assert(storage_use_record(2));
    assert(strcmp(storage_get("name"), "SecondRecord") == 0);
}

static void test_transactions(void)
{
    assert(storage_use_record(2));

    assert(storage_transaction_begin());
    assert(storage_transaction_active());
    assert(storage_update("name", "AbortedValue"));
    assert(strcmp(storage_get("name"), "AbortedValue") == 0);
    assert(storage_transaction_abort());
    assert(!storage_transaction_active());
    assert(strcmp(storage_get("name"), "SecondRecord") == 0);

    assert(storage_transaction_begin());
    assert(storage_update("name", "CommittedValue"));
    assert(storage_transaction_commit());
    assert(!storage_transaction_active());
    assert(strcmp(storage_get("name"), "CommittedValue") == 0);
}

static void test_snapshot(void)
{
    assert(storage_snapshot_create());
    assert(storage_update("name", "AfterSnapshot"));
    assert(strcmp(storage_get("name"), "AfterSnapshot") == 0);

    assert(storage_snapshot_restore(1));
    assert(strcmp(storage_get("name"), "CommittedValue") == 0);
}

static void test_persistence(void)
{
    assert(storage_save(TEST_DB_FILE));

    assert(storage_update("name", "BeforeReload"));
    assert(strcmp(storage_get("name"), "BeforeReload") == 0);

    assert(storage_load(TEST_DB_FILE));
    assert(strcmp(storage_get("name"), "CommittedValue") == 0);

    assert(storage_use_record(1));
    assert(strcmp(storage_get("name"), "Avani") == 0);

    assert(storage_use_record(2));
    assert(strcmp(storage_get("name"), "CommittedValue") == 0);
}

int main(void)
{
    remove(TEST_DB_FILE);

    test_entity_and_values();
    test_history_and_rollback();
    test_records();
    test_transactions();
    test_snapshot();
    test_persistence();

    remove(TEST_DB_FILE);

    printf("All ChronoDB storage tests passed.\n");
    return 0;
}
