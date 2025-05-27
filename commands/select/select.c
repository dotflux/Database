#include "select.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"
#include "../../db/schemas/schema.h"

// Parses tokens[start..end] as a sequence of condition tokens:
//  column=“value” or age>10, joined by && or ||.
void parse_conditions(char** tokens, int start, int end, WhereClause* where) {
    where->count = 0;
    for (int i = start; i <= end && where->count < MAX_CONDITIONS; ) {
        Condition* cond = &where->conditions[where->count];
        char* t = tokens[i++];

        // 1) Split t into column/op/value
        const char* ops[] = {">=", "<=", "!=", ">", "<", "="};
        int op_i;
        for (op_i = 0; op_i < 6; op_i++) {
            char* p = strstr(t, ops[op_i]);
            if (p) {
                size_t col_len = p - t;
                strncpy(cond->column, t, col_len);
                cond->column[col_len] = '\0';
                strcpy(cond->op, ops[op_i]);
                strcpy(cond->value, p + strlen(ops[op_i]));
                break;
            }
        }
        if (op_i == 6) {
            printf("Invalid condition token: %s\n", t);
            continue;
        }

        // strip surrounding quotes from value
        char* v = cond->value;
        size_t vlen = strlen(v);
        if (vlen >= 2 && ((v[0] == '"' && v[vlen-1] == '"') ||
                          (v[0] == '\'' && v[vlen-1] == '\''))) {
            memmove(v, v+1, vlen-2);
            v[vlen-2] = '\0';
        }

        // 2) Check for a logical operator after this token
        char lop = 'A';  // default AND
        if (i <= end && (strcmp(tokens[i], "&&") == 0 || strcmp(tokens[i], "||") == 0)) {
            lop = (tokens[i][0] == '&' ? 'A' : 'O');
            i++;
        }
        where->logic_ops[where->count] = lop;

        where->count++;
    }
}

void handle_select(char** tokens) {
    // 1) table name = tokens[1]
    const char* table_name = tokens[1];
    if (!table_name) return;

    // 2) if no tokens[2], no conditions
    if (!tokens[2]) {
        read_table(table_name, NULL);
        return;
    }

    // 3) find last token index
    int end = 2;
    while (tokens[end]) end++;
    end--;  // now tokens[end] is last condition or logic token

    // 4) parse conditions from index 2..end
    WhereClause where;
    parse_conditions(tokens, 2, end, &where);

    // 5) call read_table with the built WhereClause
    read_table(table_name, &where);
}
