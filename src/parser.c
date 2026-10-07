#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "storage.h"

void parser_handle_command(const char *input)
{
    char line[100];
    char *command;
    char *key;
    char *value;
    char *extra;

    if (strlen(input) >= sizeof(line)) {
        printf("ERR command too long\n");
        return;
    }

    strcpy(line, input);

    command = strtok(line, " \t");

    if (command == NULL) {
        return;
    }

    if (strcmp(command, "HELP") == 0) {
        if (strtok(NULL, " \t") != NULL) {
            printf("ERR usage: HELP\n");
            return;
        }

        printf("\n");
        printf("========== ChronoDB Commands ==========\n\n");

        printf("Data Operations:\n");
        printf("  SET <key> <value>       Create or replace a key\n");
        printf("  GET <key>               Retrieve a value\n");
        printf("  UPDATE <key> <value>   Update an existing key\n");
        printf("  DELETE <key>            Delete a key\n");
        printf("  LIST                    List all current entries\n");
        printf("  SEARCH <term>           Search keys and values\n");

	printf("  SNAPSHOT CREATE         Create a database snapshot\n");
	printf("  SNAPSHOT LIST           List all snapshots\n");
	printf("  SNAPSHOT RESTORE <id>   Restore a database snapshot\n");

        printf("\nVersion Control:\n");
        printf("  HISTORY <key>           Show key version history\n");
        printf("  ROLLBACK <key> <ver>   Restore a previous version\n");

	printf("\nTransactions:\n");
        printf("  BEGIN                   Start a transaction\n");
        printf("  COMMIT                  Commit the current transaction\n");
        printf("  ABORT                   Abort the current transaction\n");

        printf("\nPersistence:\n");
        printf("  SAVE                    Save database to disk\n");
        printf("  LOAD                    Load database from disk\n");

        printf("\nSystem:\n");
	printf("  STATS                   Show database statistics\n");
        printf("  HELP                    Show this help message\n");
        printf("  EXIT                    Exit ChronoDB\n");

        printf("\n=======================================\n\n");
    }

    else if (strcmp(command, "SET") == 0 || strcmp(command, "UPDATE") == 0) {
        key = strtok(NULL, " \t");
        value = strtok(NULL, " \t");
        extra = strtok(NULL, " \t");

        if (key == NULL || value == NULL || extra != NULL) {
            printf("ERR usage: %s <key> <value>\n", command);
            return;
        }

        if (strlen(key) >= MAX_KEY_LENGTH || strlen(value) >= MAX_VALUE_LENGTH) {
            printf("ERR key or value too long\n");
            return;
        }

        int success;

        if (strcmp(command, "SET") == 0) {
            success = storage_set(key, value);

            if (!success) {
                printf("ERR could not set key. Database may be full.\n");
                return;
            }

            printf("OK: key '%s' stored successfully\n", key);
        }

        else {
            success = storage_update(key, value);

            if (!success) {
                printf("ERR key '%s' not found\n", key);
                return;
            }

            printf("OK: key '%s' updated successfully\n", key);
        }
    }

    else if (strcmp(command, "GET") == 0 || strcmp(command, "DELETE") == 0) {
        key = strtok(NULL, " \t");
        extra = strtok(NULL, " \t");

        if (key == NULL || extra != NULL) {
            printf("ERR usage: %s <key>\n", command);
            return;
        }

        if (strlen(key) >= MAX_KEY_LENGTH) {
            printf("ERR key too long\n");
            return;
        }

        if (strcmp(command, "GET") == 0) {
            const char *result = storage_get(key);

            if (result != NULL) {
                printf("VALUE: %s\n", result);
		printf("TYPE: %s\n", storage_type_name(storage_get_type(key)));
            }

            else {
                printf("ERR key '%s' not found\n", key);
            }
        }

        else {
            if (storage_delete(key)) {
                printf("OK: key '%s' deleted\n", key);
            }

            else {
                printf("ERR key '%s' not found\n", key);
            }
        }
    }

    else if (strcmp(command, "LIST") == 0) {
        if (strtok(NULL, " \t") != NULL) {
            printf("ERR usage: LIST\n");
            return;
        }

        printf("\n");
        printf("========== Current Entries ==========\n");
        storage_list();
        printf("=====================================\n\n");
    }

    else if (strcmp(command, "SEARCH") == 0) {
        char *term = strtok(NULL, " \t");
        extra = strtok(NULL, " \t");

        if (term == NULL || extra != NULL) {
            printf("ERR usage: SEARCH <term>\n");
            return;
        }

        if (strlen(term) >= MAX_KEY_LENGTH) {
            printf("ERR search term too long\n");
            return;
        }

        storage_search(term);
    }

    else if (strcmp(command, "SNAPSHOT") == 0) {
   	 char *action = strtok(NULL, " \t");
   	 char *snapshot_id_text = strtok(NULL, " \t");
  	  extra = strtok(NULL, " \t");

   	 if (action == NULL) {
		 printf("ERR usage: SNAPSHOT CREATE | LIST | RESTORE <id>\n");
        	 return;
   	 }

    	if (strcmp(action, "CREATE") == 0) {
        	if (snapshot_id_text != NULL || extra != NULL) {
          	 	 printf("ERR usage: SNAPSHOT CREATE\n");
            		return;
       	 }

        	int snapshot_id = storage_snapshot_create();

       		if (snapshot_id == 0) {
            		printf("ERR could not create snapshot. Snapshot limit reached.\n");
            		return;
        	}

        	printf("OK: snapshot %d created\n", snapshot_id);
    	}

    	else if (strcmp(action, "LIST") == 0) {
        	if (snapshot_id_text != NULL || extra != NULL) {
            		printf("ERR usage: SNAPSHOT LIST\n");
            		return;
        	}

        	printf("\n");
        	printf("========== Snapshots ==========\n");
        	storage_snapshot_list();
        	printf("===============================\n\n");
    	}

    	else if (strcmp(action, "RESTORE") == 0) {
        	if (snapshot_id_text == NULL || extra != NULL) {
            		printf("ERR usage: SNAPSHOT RESTORE <id>\n");
            		return;
        	}

        	char *endptr;
        	long snapshot_id = strtol(snapshot_id_text, &endptr, 10);

        	if (*snapshot_id_text == '\0' || *endptr != '\0' || snapshot_id <= 0 || snapshot_id > 2147483647) {
            		printf("ERR invalid snapshot ID\n");
            		return;
        	}

        	if (storage_snapshot_restore((int)snapshot_id)) {
            		printf("OK: snapshot %ld restored successfully\n", snapshot_id);
        	}

        	else {
            		printf("ERR snapshot %ld not found\n", snapshot_id);
        	}
    	}

    	else {
        	printf("ERR unknown SNAPSHOT action '%s'\n", action);
    	 }
     }

    else if (strcmp(command, "HISTORY") == 0) {
        key = strtok(NULL, " \t");
        extra = strtok(NULL, " \t");

        if (key == NULL || extra != NULL) {
            printf("ERR usage: HISTORY <key>\n");
            return;
        }

        if (strlen(key) >= MAX_KEY_LENGTH) {
            printf("ERR key too long\n");
            return;
        }

        printf("\n");
	storage_history(key);
        printf("\n");
    }

    else if (strcmp(command, "ROLLBACK") == 0) {
        key = strtok(NULL, " \t");
        char *version_text = strtok(NULL, " \t");

        extra = strtok(NULL, " \t");

        if (key == NULL || version_text == NULL || extra != NULL) {
            printf("ERR usage: ROLLBACK <key> <version>\n");
            return;
        }

        if (strlen(key) >= MAX_KEY_LENGTH) {
            printf("ERR key too long\n");
            return;
        }

        char *endptr;
        long version = strtol(version_text, &endptr, 10);

        if (*version_text == '\0' || *endptr != '\0' || version <= 0 || version > 2147483647) {
            printf("ERR invalid version\n");
            return;
        }

        if (storage_rollback(key, (int)version)) {
            printf("OK: key '%s' rolled back to version %ld\n", key, version);
        }

        else {
            printf("ERR rollback failed: key or version not found, or history full\n");
        }
    }

    else if (strcmp(command, "SAVE") == 0) {
        if (strtok(NULL, " \t") != NULL) {
            printf("ERR usage: SAVE\n");
            return;
        }

        if (storage_save("data/chronodb.db")) {
            printf("OK: database saved to data/chronodb.db\n");
        }

        else {
            printf("ERR could not save database\n");
        }
    }

    else if (strcmp(command, "LOAD") == 0) {
        if (strtok(NULL, " \t") != NULL) {
            printf("ERR usage: LOAD\n");
            return;
        }

        if (storage_load("data/chronodb.db")) {
            printf("OK: database loaded successfully\n");
        }

        else {
            printf("ERR could not load database\n");
        }
    }

    else if (strcmp(command, "STATS") == 0) {
   	 if (strtok(NULL, " \t") != NULL) {
        	printf("ERR usage: STATS\n");
        	return;
   	 }

    	storage_stats();
    }
    else if (strcmp(command, "BEGIN") == 0) {
	    if (strtok(NULL, " \t") != NULL) {
		    printf("ERR usage: BEGIN\n");
		    return;
	    }

	    if (storage_transaction_begin()) {
		    printf("OK: transaction started\n");
	    }

	    else {
		    printf("ERR transaction already active\n");
	    }
    }
    else if (strcmp(command, "COMMIT") == 0) {
	    if (strtok(NULL, " \t") != NULL) {
		    printf("ERR usage: COMMIT\n");
		    return;
	    }

	    if (storage_transaction_commit()) {
		    printf("OK: transaction committed\n");
	    }

	    else {
		    printf("ERR no active transaction\n");
	    }
    }
    else if (strcmp(command, "ABORT") == 0) {
	    if (strtok(NULL, " \t") != NULL) {
		    printf("ERR usage: ABORT\n");
		    return;
	    }

	    if (storage_transaction_abort()) {
		    printf("OK: transaction aborted\n");
	    }

	    else {
		    printf("ERR no active transaction\n");
	    }
    }

    else if (strcmp(command, "EXIT") == 0) {
        if (strtok(NULL, " \t") != NULL) {
            printf("ERR usage: EXIT\n");
            return;
        }

        printf("Goodbye.\n");
    }

    else {
        printf("ERR unknown command '%s'. Type HELP.\n", command);
    }
}  
