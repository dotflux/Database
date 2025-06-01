#include "delete.h"
#include "../select/select.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"
#include "../../db/schemas/schema.h"

void handle_delete(char** tokens) {
    // 1) tokens[1] is the table name
    const char* table_name = tokens[1];
    if (!table_name) return;

    // 2) If no further tokens, syntax error
    if (!tokens[2]) {
        printf("Syntax error: delete <tableName> (<whereClause>)\n");
        return;
    }

    // 3) Find the last token index
    int end = 2;
    while (tokens[end]) end++;
    end--;  // Now tokens[end] is the last condition or logic‐operator

    // 4) Parse WHERE clause from tokens[2..end]
    WhereClause where;
    parse_conditions(tokens, 2, end, &where);

    // 5) Call delete_data() with the parsed WHERE
    delete_table(table_name, &where);
}