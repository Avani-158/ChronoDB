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

    fprintf(file, "CHRONODB 2 extra\n");
    fprintf(file, "ENTRIES 0\n");
    fprintf(file, "HISTORY 0\n");

    fclose(file);

    assert(storage_load("/tmp/chronodb_malformed.db") == 0);
    assert(strcmp(storage_get("safe"), "value") == 0);
    printf("PASS: Malformed header rejection\n");

    /* Test 9: Inconsistent history rejection */
    file = fopen("/tmp/chronodb_inconsistent.db", "w");
    assert(file != NULL);

    fprintf(file, "CHRONODB 2\n");
    fprintf(file, "ENTRIES 1\n");
    fprintf(file, "name ananya\n");
    fprintf(file, "HISTORY 2\n");
    fprintf(file, "name avani 1\n");
    fprintf(file, "name riya 2\n");

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

    printf("\nAll storage tests passed!\n");

    return 0;
}
