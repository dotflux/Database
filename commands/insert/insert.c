#include "insert.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"

void handle_insert(char** tokens){
    const char* table_name = tokens[1];
    if (!table_name) return;

    int count = 0;
    while (tokens[count] != NULL) count++;

    insert_table(table_name, tokens, count);
}