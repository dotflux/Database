#include "schema.h"
#include "bplustree.h"
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

void write_to_table(const char* table_name, char** tokens, int token_count){

    char schema_filename[128];
    snprintf(schema_filename, sizeof(schema_filename), "db/schemas/%s.schema", table_name);

    FILE* sfp = fopen(schema_filename, "rb");
    if (!sfp) {
        perror("Schema file not found");
        return;
    }

    TableSchema schema;
    if (fread(&schema, sizeof(TableSchema), 1, sfp) != 1) {
        perror("Failed to read schema");
        fclose(sfp);
        return;
    }
    fclose(sfp);

    int expected_tokens = schema.column_count * 2; // key + value per column
    if (token_count - 2 != expected_tokens) {
        fprintf(stderr, "Column count mismatch: expected %d columns\n", schema.column_count);
        return;
    }

    for (int i = 0, j = 2; i < schema.column_count && j + 1 < token_count; i++, j += 2) {
        if (strcmp(schema.columns[i].name, tokens[j]) != 0) {
            fprintf(stderr, "Column name mismatch: expected '%s', got '%s'\n", schema.columns[i].name, tokens[j]);
            return;
        }
        // Optional: add type validation here, e.g. check if tokens[j+1] matches schema.columns[i].type
    }

    char table_filename[128];
    snprintf(table_filename, sizeof(table_filename), "db/tables/%s.table", table_name);

    FILE* tfp = fopen(table_filename, "ab");
    if (!tfp) {
        perror("Failed to open table file");
        return;
    }

    char row_data[schema.column_count][MAX_VALUE_LEN];
    for (int i = 0, j = 2; i < schema.column_count && j + 1 < token_count; i++, j += 2) {

        strncpy(row_data[i], tokens[j + 1], MAX_VALUE_LEN - 1);
        
        row_data[i][MAX_VALUE_LEN - 1] = '\0';
    }
    
    for (int i = 0; i < schema.column_count; i++) {
        fwrite(row_data[i], sizeof(char), MAX_VALUE_LEN, tfp);
    }

    fclose(tfp);
    printf("Row inserted successfully.\n");


}

void read_table(const char* table_name, const WhereClause* where) {
    char schemafile[128];
    char filename[128];
    snprintf(schemafile, sizeof(schemafile), "db/schemas/%s.schema", table_name);
    snprintf(filename, sizeof(filename), "db/tables/%s.table", table_name);

    FILE* sfp = fopen(schemafile, "rb");
    if (!sfp) {
        perror("Failed to open schema file");
        return;
    }

    TableSchema schema;
    if (fread(&schema, sizeof(TableSchema), 1, sfp) != 1) {
        perror("Failed to read schema");
        fclose(sfp);
        return;
    }
    fclose(sfp);

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open table file");
        return;
    }

    clock_t start = clock();

    // Build indexes on needed columns based on WHERE clause
    IndexMap index_map = {0};
    
    int matched_rows[1024];
    int matched_count = 0;

    if (where && where->count > 0) {
        build_indexes(fp, &schema, where, &index_map);
        filter_rows(&index_map, where, matched_rows, &matched_count);
    } else {
        // No WHERE clause, select all rows
        // We'll read all rows sequentially by row number
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        long row_size = schema.column_count * MAX_VALUE_LEN;
        matched_count = (int)(file_size / row_size);
        for (int i = 0; i < matched_count; i++) {
            matched_rows[i] = i;
        }
    }

    char row_data[MAX_COLUMNS][MAX_VALUE_LEN];

    for (int i = 0; i < matched_count; i++) {
        int row_num = matched_rows[i];
        // Seek to the row's position in the file
        fseek(fp, row_num * schema.column_count * MAX_VALUE_LEN, SEEK_SET);
        if (fread(row_data, MAX_VALUE_LEN, schema.column_count, fp) != (size_t)schema.column_count) {
            fprintf(stderr, "Failed to read row %d\n", row_num);
            continue;
        }

        printf("Row %d:\n", row_num + 1);
        for (int j = 0; j < schema.column_count; j++) {
            printf("  %s: %s\n", schema.columns[j].name, row_data[j]);
        }
        printf("\n");
    }

    if (matched_count == 0){
        printf("No matched row founds");
    }

    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("[Query executed in %.6f seconds]\n", time_taken);

    fclose(fp);
}
