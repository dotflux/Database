#include "update.h"
#include "../select/select.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"
#include "../../db/schemas/schema.h"

void handle_update(char** tokens){
    const char* table = tokens[1];
    if (!table) {
        printf("Syntax: update <table> (<where>) values (<col>:<val>[, ...])\n");
        return;
    }

    // find the "values" token
    int i = 2, n = 0;
    while (tokens[n]) n++;
    int values_idx = -1;
    for (i = 2; i < n; i++) {
        if (strcmp(tokens[i], "values") == 0) {
            values_idx = i;
            break;
        }
    }
    if (values_idx < 0) {
        printf("Syntax error: missing `values` keyword\n");
        return;
    }
    if (values_idx == 2) {
        printf("Syntax error: missing WHERE clause\n");
        return;
    }
    if (values_idx + 1 >= n) {
        printf("Syntax error: missing update assignments\n");
        return;
    }

    // 1) parse WHERE from tokens[2..values_idx-1]
    WhereClause where;
    parse_conditions(tokens, 2, values_idx - 1, &where);
    if (where.count == 0) {
        printf("No conditions parsed\n");
        return;
    }

    // 2) parse assignments after "values"
    UpdatePair updates[MAX_CONDITIONS];
    int upd_count = 0;
    // they come as pairs: col value col value ...
    for (i = values_idx + 1; i + 1 < n && upd_count < MAX_CONDITIONS; i += 2) {
        const char* col = tokens[i];
        const char* val = tokens[i + 1];
        if (!col || !val) break;
        // copy
        strncpy(updates[upd_count].column, col, MAX_NAME_LEN - 1);
        updates[upd_count].column[MAX_NAME_LEN - 1] = '\0';
        strncpy(updates[upd_count].value, val, MAX_VALUE_LEN - 1);
        updates[upd_count].value[MAX_VALUE_LEN - 1] = '\0';
        upd_count++;
    }
    if (upd_count == 0) {
        printf("No assignments parsed\n");
        return;
    }

    // 3) perform the update (in your schema_utils)
    update_table(table, &where, updates, upd_count);
}