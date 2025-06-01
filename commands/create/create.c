#include "create.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"

void handle_create(char** tokens){

    char* table_name = tokens[1];
    if (!table_name)return;

    // printf("Columns:\n");
    // for (int i = 2; tokens[i] != NULL;i+=2){
    //     char* column_name = tokens[i];
    //     char* column_type = tokens[i+1];
    // }

    int count = 0;
    while (tokens[count] != NULL) {
    count++;
    }

    create_table(table_name,tokens,count);
}