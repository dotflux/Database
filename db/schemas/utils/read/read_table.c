#include "../../schema.h"
#include "../../bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void print_border(int* colw, int column_count) {
    putchar('+');
    for (int c = 0; c < column_count; c++) {
        for (int i = 0; i < colw[c] + 2; i++) putchar('-');
        putchar('+');
    }
    putchar('\n');
}

void read_table(const char* table_name, const WhereClause* where) {
    char schemafile[128], filename[128];
    snprintf(schemafile, sizeof(schemafile), "db/schemas/%s.schema", table_name);
    snprintf(filename,   sizeof(filename),   "db/tables/%s.table",  table_name);

    FILE* sfp = fopen(schemafile, "rb");
    if (!sfp) { perror("Failed to open schema file"); return; }
    TableSchema schema;
    if (fread(&schema, sizeof(schema), 1, sfp) != 1) {
        perror("Failed to read schema"); fclose(sfp); return;
    }
    fclose(sfp);

    FILE* fp = fopen(filename, "rb");
    if (!fp) { perror("Failed to open table file"); return; }

    clock_t start = clock();

    // figure out total rows
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    long row_size   = (long)schema.column_count * MAX_VALUE_LEN;
    int  total_rows = file_size / row_size;

    // find which rows match
    IndexMap index_map = {0};
    int*      matched_rows = malloc(sizeof *matched_rows * total_rows);
    int       matched_count;
    if (where && where->count > 0) {
        build_indexes(fp, &schema, where, &index_map);
        filter_rows(&index_map, where, matched_rows, &matched_count, total_rows);
    } else {
        matched_count = total_rows;
        for (int i = 0; i < total_rows; i++) matched_rows[i] = i;
    }

    if (matched_count == 0) {
        printf("No matched rows found\n");
        free(matched_rows);
        fclose(fp);
        return;
    }

    // read all matching rows into a string buffer
    char*** rows = malloc(sizeof(char**) * matched_count);
    for (int r = 0; r < matched_count; r++) {
        rows[r] = malloc(sizeof(char*) * schema.column_count);
        for (int c = 0; c < schema.column_count; c++) {
            rows[r][c] = malloc(MAX_VALUE_LEN);
        }

        fseek(fp, (long)matched_rows[r] * row_size, SEEK_SET);
        for (int c = 0; c < schema.column_count; c++) {
            fread(rows[r][c], 1, MAX_VALUE_LEN, fp);
            rows[r][c][MAX_VALUE_LEN-1] = '\0';
        }
    }

    // compute column widths
    int* colw = malloc(sizeof *colw * schema.column_count);
    for (int c = 0; c < schema.column_count; c++) {
        int w = strlen(schema.columns[c].name);
        for (int r = 0; r < matched_count; r++) {
            int l = (int)strlen(rows[r][c]);
            if (l > w) w = l;
        }
        colw[c] = w;
    }

    // helper to print horizontal border

    // header
    print_border(colw, schema.column_count);
    putchar('|');
    for (int c = 0; c < schema.column_count; c++) {
        printf(" %-*s |", colw[c], schema.columns[c].name);
    }
    putchar('\n');
    print_border(colw, schema.column_count);

    // rows
    for (int r = 0; r < matched_count; r++) {
        putchar('|');
        for (int c = 0; c < schema.column_count; c++) {
            printf(" %-*s |", colw[c], rows[r][c]);
        }
        putchar('\n');
    }
    print_border(colw, schema.column_count);

    // cleanup
    for (int r = 0; r < matched_count; r++) {
        for (int c = 0; c < schema.column_count; c++) {
            free(rows[r][c]);
        }
        free(rows[r]);
    }
    free(rows);
    free(colw);
    free(matched_rows);

    clock_t end = clock();
    double t = (double)(end - start)/CLOCKS_PER_SEC;
    printf("[Query executed in %.6f seconds]\n", t);
    printf("Matched count: %d\n",matched_count);

    fclose(fp);
}