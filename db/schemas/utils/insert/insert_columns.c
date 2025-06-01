#include "../../schema.h"
#include "../../bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void insert_to_schema(const char* table_name,char** tokens, int token_count){
    // tokens[2..token_count-1] come in <name>,<type> pairs.
    int num_new = (token_count - 2) / 2;  // e.g. if token_count==8, num_new==3

    // 1) Load existing schema
    char schema_path[128];
    snprintf(schema_path, sizeof(schema_path), "db/schemas/%s.schema", table_name);
    FILE* sfp = fopen(schema_path, "rb");
    if (!sfp) {
        perror("insert_to_schema: Failed to open schema file");
        return;
    }

    TableSchema schema;
    if (fread(&schema, sizeof(schema), 1, sfp) != 1) {
        perror("insert_to_schema: Failed to read schema");
        fclose(sfp);
        return;
    }
    fclose(sfp);

    int old_col_count = schema.column_count;

    // 2) Verify we won’t exceed MAX_COLUMNS
    if (old_col_count + num_new > MAX_COLUMNS) {
        fprintf(stderr,
                "insert_to_schema: Cannot add %d columns; would exceed MAX_COLUMNS (%d).\n",
                num_new, MAX_COLUMNS);
        return;
    }

    // 3) Check for duplicate names among the new columns themselves,
    //    and also make sure none already exist in the old schema.
    for (int i = 0; i < num_new; i++) {
        const char* new_name = tokens[2 + 2*i];
        // Check against existing schema
        for (int c = 0; c < old_col_count; c++) {
            if (strcmp(schema.columns[c].name, new_name) == 0) {
                printf("Column '%s' already exists in '%s'.\n", new_name, table_name);
                return;
            }
        }
        // Check among the new ones (i.e. two new columns with same name)
        for (int j = i+1; j < num_new; j++) {
            const char* other_new = tokens[2 + 2*j];
            if (strcmp(new_name, other_new) == 0) {
                printf("Duplicate column name '%s' in INSERT list.\n", new_name);
                return;
            }
        }
    }

    // 4) Append each new column into schema in memory
    for (int i = 0; i < num_new; i++) {
        const char* col_name = tokens[2 + 2*i];
        const char* col_type = tokens[2 + 2*i + 1];

        int idx = old_col_count + i;
        // copy name
        strncpy(schema.columns[idx].name, col_name, MAX_NAME_LEN - 1);
        schema.columns[idx].name[MAX_NAME_LEN - 1] = '\0';
        // copy type
        strncpy(schema.columns[idx].type, col_type, MAX_TYPE_LEN - 1);
        schema.columns[idx].type[MAX_TYPE_LEN - 1] = '\0';
    }
    schema.column_count = old_col_count + num_new;

    // 5) Overwrite the .schema file on disk
    sfp = fopen(schema_path, "wb");
    if (!sfp) {
        perror("insert_to_schema: Failed to reopen schema file for writing");
        return;
    }
    if (fwrite(&schema, sizeof(schema), 1, sfp) != 1) {
        perror("insert_to_schema: Failed to write updated schema");
        fclose(sfp);
        return;
    }
    fclose(sfp);

    // 6) Now expand every existing row in the .table file by num_new × 256 bytes.
    //    Open old data file and a temporary file side by side.
    char table_path[128];
    snprintf(table_path, sizeof(table_path), "db/tables/%s.table", table_name);

    FILE* tfp_read = fopen(table_path, "rb");
    if (!tfp_read) {
        // If table file does not yet exist, there's nothing to expand.
        // Future inserts will create it with the correct width.
        return;
    }

    char tmp_path[128];
    snprintf(tmp_path, sizeof(tmp_path), "db/tables/%s.tmp", table_name);
    FILE* tfp_write = fopen(tmp_path, "wb");
    if (!tfp_write) {
        perror("insert_to_schema: Failed to open temp file for writing");
        fclose(tfp_read);
        return;
    }

    // How many rows?  Old row size = old_col_count × 256 bytes
    long old_row_size = (long)old_col_count * MAX_VALUE_LEN;
    fseek(tfp_read, 0, SEEK_END);
    long old_file_size = ftell(tfp_read);
    rewind(tfp_read);

    int old_total_rows = 0;
    if (old_row_size > 0) {
        old_total_rows = (int)(old_file_size / old_row_size);
    }

    // Buffers:
    char* row_buffer = malloc(old_row_size);
    // we’ll write 'num_new' blocks of 256 zero‐bytes for each row
    char* extra_cell = calloc(1, MAX_VALUE_LEN);

    // Copy each old row, then append num_new × 256 zeros
    for (int r = 0; r < old_total_rows; r++) {
        if (fread(row_buffer, 1, old_row_size, tfp_read) != (size_t)old_row_size) {
            perror("insert_to_schema: Failed to read existing row");
            break;
        }
        // write old data:
        if (fwrite(row_buffer, 1, old_row_size, tfp_write) != (size_t)old_row_size) {
            perror("insert_to_schema: Failed to write row into temp");
            break;
        }
        // write num_new empty cells:
        for (int k = 0; k < num_new; k++) {
            if (fwrite(extra_cell, 1, MAX_VALUE_LEN, tfp_write) != (size_t)MAX_VALUE_LEN) {
                perror("insert_to_schema: Failed to write extra column default");
                break;
            }
        }
    }

    free(row_buffer);
    free(extra_cell);
    fclose(tfp_read);
    fclose(tfp_write);

    // 7) Replace original .table with the expanded .tmp
    if (remove(table_path) != 0) {
        perror("insert_to_schema: Could not remove original table file");
    }
    if (rename(tmp_path, table_path) != 0) {
        perror("insert_to_schema: Could not rename temp file to table file");
    }

    printf("Added %d column(s) to table '%s'.\n", num_new, table_name);
}