#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "storage.h"

int main(void)
{
    printf("Running ChronoDB storage tests...\n");

    /* Test 1: SET and GET */
    assert(storage_set("name", "avani") == 1);
    assert(strcmp(storage_get("name"), "avani") == 0);
    printf("PASS: SET and GET\n");

    /* Test 2: UPDATE */
    assert(storage_update("name", "ananya") == 1);
    assert(strcmp(storage_get("name"), "ananya") == 0);
    printf("PASS: UPDATE\n");

    /* Test 3: Invalid update */
    assert(storage_update("missing", "value") == 0);
    printf("PASS: Invalid UPDATE\n");

    /* Test 4: ROLLBACK */
    assert(storage_rollback("name", 1) == 1);
    assert(strcmp(storage_get("name"), "avani") == 0);
    printf("PASS: ROLLBACK\n");

    /* Test 5: SAVE and LOAD */
    assert(storage_save("/tmp/chronodb_test.db") == 1);

    assert(storage_update("name", "riya") == 1);
    assert(strcmp(storage_get("name"), "riya") == 0);

    assert(storage_load("/tmp/chronodb_test.db") == 1);
    assert(strcmp(storage_get("name"), "avani") == 0);
    printf("PASS: SAVE and LOAD\n");

    /* Test 6: DELETE */
    assert(storage_delete("name") == 1);
    assert(storage_get("name") == NULL);
    printf("PASS: DELETE\n");

    /* Test 7: Invalid database file */
    assert(storage_set("safe", "value") == 1);

    FILE *file = fopen("/tmp/chronodb_invalid.db", "w");
    assert(file != NULL);

    fprintf(file, "INVALID DATABASE\n");
    fclose(file);

    assert(storage_load("/tmp/chronodb_invalid.db") == 0);
    assert(strcmp(storage_get("safe"), "value") == 0);
    printf("PASS: Invalid database rejection\n");

    /* Test 8: Malformed database header */
    file = fopen("/tmp/chronodb_malformed.db", "w");
    assert(file != NULL);

    fprintf(file, "CHRONODB 3 extra\n");
    fprintf(file, "ENTRIES 0\n");
    fprintf(file, "HISTORY 0\n");

    fclose(file);

    assert(storage_load("/tmp/chronodb_malformed.db") == 0);
    assert(strcmp(storage_get("safe"), "value") == 0);
    printf("PASS: Malformed header rejection\n");

    /* Test 9: Inconsistent history rejection */
    file = fopen("/tmp/chronodb_inconsistent.db", "w");
    assert(file != NULL);

    fprintf(file, "CHRONODB 3\n");
    fprintf(file, "ENTRIES 1\n");
    fprintf(file, "name ananya 0\n");
    fprintf(file, "HISTORY 2\n");
    fprintf(file, "name avani 0 1\n");
    fprintf(file, "name riya 0 2\n");

    fclose(file);

    assert(storage_load("/tmp/chronodb_inconsistent.db") == 0);
    assert(strcmp(storage_get("safe"), "value") == 0);
    printf("PASS: Inconsistent history rejection\n");

    /* Test 10: Deleted key history survives loading */
    assert(storage_set("deleted", "first") == 1);
    assert(storage_update("deleted", "second") == 1);
    assert(storage_delete("deleted") == 1);
    assert(storage_get("deleted") == NULL);

    assert(storage_save("/tmp/chronodb_deleted.db") == 1);

    assert(storage_set("temporary", "value") == 1);

    assert(storage_load("/tmp/chronodb_deleted.db") == 1);
    assert(storage_get("deleted") == NULL);

    printf("PASS: Deleted key history survives loading\n");

    /* Test 11: SEARCH by key */
    assert(storage_set("username", "avani") == 1);
    assert(storage_set("city", "Chandigarh") == 1);

    assert(storage_search("user") == 1);
    printf("PASS: SEARCH by key\n");

    /* Test 12: SEARCH by value */
    assert(storage_search("chand") == 1);
    printf("PASS: SEARCH by value\n");

    /* Test 13: SEARCH is case-insensitive */
    assert(storage_search("CHANDIGARH") == 1);
    printf("PASS: Case-insensitive SEARCH\n");

    /* Test 14: SEARCH with no match */
    assert(storage_search("xyz") == 0);
    printf("PASS: SEARCH no-match handling\n");

    /* Test 15: SNAPSHOT create and restore */
    assert(storage_snapshot_create() == 1);

    assert(storage_update("username", "updated") == 1);
    assert(storage_delete("city") == 1);

    assert(storage_get("username") != NULL);
    assert(strcmp(storage_get("username"), "updated") == 0);
    assert(storage_get("city") == NULL);

    assert(storage_snapshot_restore(1) == 1);

    assert(strcmp(storage_get("username"), "avani") == 0);
    assert(strcmp(storage_get("city"), "Chandigarh") == 0);

    printf("PASS: SNAPSHOT create and restore\n");

    /* Test 16: Invalid SNAPSHOT restore */
    assert(storage_snapshot_restore(999) == 0);
    printf("PASS: Invalid SNAPSHOT restore\n");

    /* Test 17: Database statistics */
    storage_stats();
    printf("PASS: Database statistics\n");

    remove("/tmp/chronodb_deleted.db");
    remove("/tmp/chronodb_inconsistent.db");
    remove("/tmp/chronodb_malformed.db");
    remove("/tmp/chronodb_test.db");
    remove("/tmp/chronodb_invalid.db");

    /* Test 18: Typed value detection */
    printf("Test 18: Typed value detection...\n");

    assert(storage_set("typed_integer", "42") == 1);
    assert(storage_get_type("typed_integer") == TYPE_INTEGER);

    assert(storage_set("typed_float", "42.5") == 1);
    assert(storage_get_type("typed_float") == TYPE_FLOAT);

    assert(storage_set("typed_boolean", "true") == 1);
    assert(storage_get_type("typed_boolean") == TYPE_BOOLEAN);

    assert(storage_set("typed_string", "hello") == 1);
    assert(storage_get_type("typed_string") == TYPE_STRING);

    printf("Test 18 passed.\n");

    /* Test 19: UPDATE changes value type */
    printf("Test 19: UPDATE changes value type...\n");

    assert(storage_set("changing_type", "100") == 1);
    assert(storage_get_type("changing_type") == TYPE_INTEGER);

    assert(storage_update("changing_type", "100.5") == 1);
    assert(storage_get_type("changing_type") == TYPE_FLOAT);

    assert(storage_update("changing_type", "true") == 1);
    assert(storage_get_type("changing_type") == TYPE_BOOLEAN);

    assert(storage_update("changing_type", "hello") == 1);
    assert(storage_get_type("changing_type") == TYPE_STRING);

    printf("Test 19 passed.\n");

    /* Test 20: SAVE and LOAD preserve value types */
    printf("Test 20: SAVE and LOAD preserve value types...\n");

    assert(storage_set("persist_int", "123") == 1);
    assert(storage_set("persist_float", "123.45") == 1);
    assert(storage_set("persist_bool", "false") == 1);
    assert(storage_set("persist_string", "ChronoDB") == 1);

    assert(storage_update("username", "avani") == 1);

    assert(storage_save("data/test_typed.db") == 1);
    assert(storage_load("data/test_typed.db") == 1);

    assert(storage_get_type("persist_int") == TYPE_INTEGER);
    assert(storage_get_type("persist_float") == TYPE_FLOAT);
    assert(storage_get_type("persist_bool") == TYPE_BOOLEAN);
    assert(storage_get_type("persist_string") == TYPE_STRING);

    printf("Test 20 passed.\n");

    /* Test 21: ROLLBACK restores original value type */
    printf("Test 21: ROLLBACK restores original value type...\n");

    assert(storage_set("rollback_type", "50") == 1);
    assert(storage_update("rollback_type", "50.5") == 1);
    assert(storage_get_type("rollback_type") == TYPE_FLOAT);

    assert(storage_rollback("rollback_type", 1) == 1);

    assert(strcmp(storage_get("rollback_type"), "50") == 0);
    assert(storage_get_type("rollback_type") == TYPE_INTEGER);

    printf("Test 21 passed.\n");

    /* Test 22: BOOLEAN value detection */
    printf("Test 22: BOOLEAN value detection...\n");

    assert(storage_set("bool_true", "true") == 1);
    assert(storage_set("bool_false", "false") == 1);

    assert(storage_get_type("bool_true") == TYPE_BOOLEAN);
    assert(storage_get_type("bool_false") == TYPE_BOOLEAN);

    assert(strcmp(storage_get("bool_true"), "true") == 0);
    assert(strcmp(storage_get("bool_false"), "false") == 0);

    printf("Test 22 passed.\n");

    /* Test 23: Negative and decimal numeric values */
    printf("Test 23: Negative and decimal numeric values...\n");

    assert(storage_set("negative_int", "-25") == 1);
    assert(storage_set("negative_float", "-25.75") == 1);

    assert(storage_get_type("negative_int") == TYPE_INTEGER);
    assert(storage_get_type("negative_float") == TYPE_FLOAT);

    printf("Test 23 passed.\n");

    /* Test 24: String value detection */
    printf("Test 24: String value detection...\n");

    assert(storage_set("normal_text", "hello_world") == 1);
    assert(storage_set("numeric_text", "123abc") == 1);
    assert(storage_set("decimal_text", "12.34.56") == 1);

    assert(storage_get_type("normal_text") == TYPE_STRING);
    assert(storage_get_type("numeric_text") == TYPE_STRING);
    assert(storage_get_type("decimal_text") == TYPE_STRING);

    printf("Test 24 passed.\n");

        /* Test 25: BEGIN and transaction state */
    printf("Test 25: BEGIN transaction...\n");

    assert(storage_transaction_commit() == 0);
    assert(storage_transaction_abort() == 0);
    assert(storage_transaction_begin() == 1);
    assert(storage_transaction_active() == 1);
    assert(storage_transaction_begin() == 0);
    assert(storage_transaction_abort() == 1);

    printf("Test 25 passed.\n");

    /* Test 26: COMMIT keeps transaction changes */
    printf("Test 26: COMMIT keeps transaction changes...\n");

    assert(storage_set("transaction_commit", "100") == 1);
    assert(storage_transaction_begin() == 1);
    assert(storage_update("transaction_commit", "200") == 1);
    assert(storage_transaction_commit() == 1);

    assert(strcmp(storage_get("transaction_commit"), "200") == 0);
    assert(storage_get_type("transaction_commit") == TYPE_INTEGER);
    assert(storage_transaction_active() == 0);

    printf("Test 26 passed.\n");

    /* Test 27: ABORT restores previous value */
    printf("Test 27: ABORT restores previous value...\n");

    assert(storage_set("transaction_abort", "original") == 1);

    assert(storage_transaction_begin() == 1);
    assert(storage_update("transaction_abort", "changed") == 1);
    assert(strcmp(storage_get("transaction_abort"), "changed") == 0);

    assert(storage_transaction_abort() == 1);

    assert(strcmp(storage_get("transaction_abort"), "original") == 0);
    assert(storage_get_type("transaction_abort") == TYPE_STRING);
    assert(storage_transaction_active() == 0);

    printf("Test 27 passed.\n");

    /* Test 28: ABORT restores original value type */
    printf("Test 28: ABORT restores original value type...\n");

    assert(storage_set("transaction_type", "50") == 1);
    assert(storage_get_type("transaction_type") == TYPE_INTEGER);

    assert(storage_transaction_begin() == 1);
    assert(storage_update("transaction_type", "50.5") == 1);
    assert(storage_get_type("transaction_type") == TYPE_FLOAT);

    assert(storage_transaction_abort() == 1);

    assert(strcmp(storage_get("transaction_type"), "50") == 0);
    assert(storage_get_type("transaction_type") == TYPE_INTEGER);

    printf("Test 28 passed.\n");

    /* Test 29: ABORT restores history state */
    printf("Test 29: ABORT restores history state...\n");

    assert(storage_set("transaction_history", "one") == 1);

    assert(storage_transaction_begin() == 1);
    assert(storage_update("transaction_history", "two") == 1);
    assert(storage_transaction_abort() == 1);

    assert(strcmp(storage_get("transaction_history"), "one") == 0);

    printf("Test 29 passed.\n");

    /* Test 30: Multiple changes can be committed */
    printf("Test 30: COMMIT multiple changes...\n");

    assert(storage_transaction_begin() == 1);

    assert(storage_set("transaction_a", "10") == 1);
    assert(storage_set("transaction_b", "20") == 1);
    assert(storage_set("transaction_c", "30") == 1);

    assert(storage_transaction_commit() == 1);

    assert(strcmp(storage_get("transaction_a"), "10") == 0);
    assert(strcmp(storage_get("transaction_b"), "20") == 0);
    assert(strcmp(storage_get("transaction_c"), "30") == 0);

    printf("Test 30 passed.\n");

    /* Test 31: ABORT multiple changes */
    printf("Test 31: ABORT multiple changes...\n");

    assert(storage_set("transaction_x", "old") == 1);
    assert(storage_set("transaction_y", "old") == 1);

    assert(storage_transaction_begin() == 1);

    assert(storage_update("transaction_x", "new") == 1);
    assert(storage_update("transaction_y", "new") == 1);
    assert(storage_set("transaction_z", "temporary") == 1);

    assert(storage_transaction_abort() == 1);

    assert(strcmp(storage_get("transaction_x"), "old") == 0);
    assert(strcmp(storage_get("transaction_y"), "old") == 0);
    assert(storage_get("transaction_z") == NULL);

    printf("Test 31 passed.\n");

    remove("data/test_typed.db");

    printf("\nAll storage tests passed!\n");

    return 0;
}
