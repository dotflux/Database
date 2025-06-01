#include "../../schema.h"
#include "../../bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void write_to_schema(const char* table_name, char** tokens, int token_count){

    TableSchema schema;
    memset(&schema,0,sizeof(TableSchema));
    
    strncpy(schema.table_name,table_name,MAX_NAME_LEN);
    schema.column_count = (token_count - 2)/2;

    for (int i = 0, j = 2; i < schema.column_count && j + 1 < token_count; i++, j += 2) {
        strncpy(schema.columns[i].name, tokens[j], MAX_NAME_LEN);
        strncpy(schema.columns[i].type, tokens[j + 1], MAX_TYPE_LEN);
    }

    char filename[128];
    snprintf(filename, sizeof(filename), "db/schemas/%s.schema", table_name);

    FILE* exists = fopen(filename,"rb");
    if (exists != NULL){
        perror("Table already exists");
        return;
    }

    FILE* fp = fopen(filename, "wb");
    if (!fp){
        perror("Failed to create table schema");
        return;
    }

    fwrite(&schema, sizeof(TableSchema), 1, fp);
    if (fwrite(&schema, sizeof(TableSchema), 1, fp) != 1){
        perror("Error creating table schema");
        fclose(fp);
        return;
    }
    fclose(fp);

}
