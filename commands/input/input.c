#include "input.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"

void handle_input(char** tokens){

    char* table_name = tokens[1];
    if (!table_name) return;

    int count = 0;
    while (tokens[count] != NULL) {
    count++;
    }

    input_table(table_name,tokens,count);
}