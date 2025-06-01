#include "../../schema.h"
#include "../../bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

void drop_table(const char* table_name){
    char schema_path[128];
    char table_path[128];

    // Build the two paths
    snprintf(schema_path, sizeof(schema_path), "db/schemas/%s.schema", table_name);
    snprintf(table_path,   sizeof(table_path),   "db/tables/%s.table", table_name);

    // Try to remove each file.  Keep track of which calls succeeded.
    int schema_ret = remove(schema_path);
    int table_ret  = remove(table_path);

    // Both calls failed
    if (schema_ret != 0 && table_ret != 0) {
        // Check if it was “file not found” for both
        if (errno == ENOENT) {
            printf("No such table: %s\n", table_name);
            return;
        }
        if (schema_ret != 0 && table_ret != 0) {
            // Could inspect errno, but for now:
            printf("drop_table: Failed to remove files for table '%s'.\n", table_name);
            return;
        }
    }

    // If we reach here, at least one file was deleted (or one was missing but the other existed).
    printf("Dropped table '%s'.\n", table_name);
}