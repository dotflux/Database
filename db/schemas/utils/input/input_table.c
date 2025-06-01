#include "../../schema.h"
#include "../../bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

    // // Seed for randomness
    // srand((unsigned int)time(NULL));

    // // Generate and write 100,000 random rows
    // for (int r = 0; r < 100000; r++) {
    //     for (int i = 0; i < schema.column_count; i++) {
    //         if (strcmp(schema.columns[i].type, "int") == 0) {
    //             int age = rand() % 100 + 1;
    //             snprintf(row_data[i], MAX_VALUE_LEN, "%d", age);
    //         } else {
    //             int len = rand() % 5 + 3; // name length 3–7
    //             for (int j = 0; j < len; j++) {
    //                 row_data[i][j] = 'A' + rand() % 26;
    //             }
    //             row_data[i][len] = '\0';
    //         }
    //     }

    //     for (int i = 0; i < schema.column_count; i++) {
    //         fwrite(row_data[i], sizeof(char), MAX_VALUE_LEN, tfp);
    //     }
    // }

    fclose(tfp);
    printf("Row inserted successfully.\n");


}
