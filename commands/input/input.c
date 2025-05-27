#include "input.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"

void handle_input(char** tokens){

    char* table_name = tokens[1];
    if (!table_name) return;
    char * command = tokens[0];

    int count = 0;
    while (tokens[count] != NULL) {
    count++;
    }

    write_to_table(table_name,tokens,count);
}