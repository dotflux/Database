#include "../../schema.h"
#include "../../bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void delete_data(const char* table_name, const WhereClause* where) {
    char schemafile[128], filename[128];
    snprintf(schemafile, sizeof(schemafile), "db/schemas/%s.schema", table_name);
    snprintf(filename,   sizeof(filename),   "db/tables/%s.table",  table_name);

    // 1) Open and read the schema
    FILE* sfp = fopen(schemafile, "rb");
    if (!sfp) {
        perror("delete_data: Failed to open schema file");
        return;
    }
    TableSchema schema;
    if (fread(&schema, sizeof(schema), 1, sfp) != 1) {
        perror("delete_data: Failed to read schema");
        fclose(sfp);
        return;
    }
    fclose(sfp);

    // 2) Open the table file for reading
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("delete_data: Failed to open table file");
        return;
    }

    // 3) Determine how many rows are in the file
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    long row_size   = (long)schema.column_count * MAX_VALUE_LEN;
    int  total_rows = (int)(file_size / row_size);

    // 4) Build a list of “row indices to delete”—either via WHERE or everything
    int* matched_rows = calloc(total_rows, sizeof(int));
    int  matched_count = 0;
    if (where && where->count > 0) {
        IndexMap index_map = {0};
        build_indexes(fp, &schema, where, &index_map);
        filter_rows(&index_map, where, matched_rows, &matched_count, total_rows);
    } else {
        // no WHERE ⇒ delete every row
        matched_count = total_rows;
        for (int i = 0; i < total_rows; i++) {
            matched_rows[i] = i;
        }
    }

    if (matched_count == 0) {
        printf("No rows to delete\n");
        free(matched_rows);
        fclose(fp);
        return;
    }

    // 5) Turn that list into a boolean “delete_mask” of length total_rows
    char* delete_mask = calloc(total_rows, sizeof(char));
    for (int i = 0; i < matched_count; i++) {
        int idx = matched_rows[i];
        if (idx >= 0 && idx < total_rows) {
            delete_mask[idx] = 1;
        }
    }

    // 6) Open a temporary file next to the original (same folder)
    char tmp_filename[144];
    snprintf(tmp_filename, sizeof(tmp_filename), "db/tables/%s.tmp", table_name);
    FILE* tfp = fopen(tmp_filename, "wb");
    if (!tfp) {
        perror("delete_data: Failed to open temporary file for writing");
        free(delete_mask);
        free(matched_rows);
        fclose(fp);
        return;
    }

    // 7) For each row‐index r ∈ [0..total_rows), seek to its offset:
    //      if delete_mask[r]==0 ⇒ fread(row_size) & fwrite(row_size).
    //      if delete_mask[r]==1 ⇒ skip it entirely.
    char* buffer = malloc(row_size);
    for (int r = 0; r < total_rows; r++) {
        if (delete_mask[r]) {
            // skip reading this row
            continue;
        }
        long offset = (long)r * row_size;
        if (fseek(fp, offset, SEEK_SET) != 0) {
            perror("delete_data: Failed to seek to row");
            break;
        }
        if (fread(buffer, 1, row_size, fp) != (size_t)row_size) {
            perror("delete_data: Failed to read row during copy");
            break;
        }
        if (fwrite(buffer, 1, row_size, tfp) != (size_t)row_size) {
            perror("delete_data: Failed to write row into temp file");
            break;
        }
    }

    free(buffer);
    fclose(fp);
    fclose(tfp);

    // 8) Replace original file with the temp file
    if (remove(filename) != 0) {
        perror("delete_data: Could not remove original table file");
    }
    if (rename(tmp_filename, filename) != 0) {
        perror("delete_data: Could not rename temp file to original");
    } else {
        printf("Deleted %d row(s)\n", matched_count);
    }

    free(delete_mask);
    free(matched_rows);
}