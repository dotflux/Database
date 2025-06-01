#include "../../schema.h"
#include "../../bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void update_data(const char* table_name, const WhereClause* where, const UpdatePair* updates, int update_count){
    // --- load schema ---
    char schemafile[128], tablefile[128];
    snprintf(schemafile,sizeof(schemafile),"db/schemas/%s.schema",table_name);
    snprintf(tablefile, sizeof(tablefile),"db/tables/%s.table", table_name);

    FILE* sfp = fopen(schemafile,"rb");
    if (!sfp){ perror("opening schema"); return; }
    TableSchema schema;
    fread(&schema,sizeof(schema),1,sfp);
    fclose(sfp);

    // --- find matching rows ---
    FILE* tfp = fopen(tablefile,"rb+");
    if (!tfp){ perror("opening table"); return; }

    clock_t start = clock();

    // build & apply indexes
    IndexMap idxmap = {0};
    fseek(tfp,0,SEEK_SET);
    build_indexes(tfp,&schema,where,&idxmap);

    // compute total rows
    fseek(tfp,0,SEEK_END);
    long file_size = ftell(tfp);
    long row_size  = schema.column_count * MAX_VALUE_LEN;
    int total_rows = file_size / row_size;

    int *matched = malloc(sizeof *matched * total_rows);
    int matched_count = 0;
    filter_rows(&idxmap,where,matched,&matched_count,total_rows);

    // --- apply updates in-place ---
    char buf[MAX_VALUE_LEN];
    for (int u = 0; u < update_count; u++) {
        // find column index
        int col_idx = -1;
        for (int c = 0; c < schema.column_count; c++) {
            if (strcmp(schema.columns[c].name, updates[u].column)==0) {
                col_idx = c; break;
            }
        }
        if (col_idx<0) {
            fprintf(stderr,"Unknown column '%s'\n",updates[u].column);
            continue;
        }
        // for each matched row, seek and overwrite
        for (int i = 0; i < matched_count; i++) {
            long pos = (long)matched[i] * row_size
                     + (long)col_idx * MAX_VALUE_LEN;
            fseek(tfp, pos, SEEK_SET);
            // pad or truncate value to MAX_VALUE_LEN
            memset(buf,0,MAX_VALUE_LEN);
            strncpy(buf, updates[u].value, MAX_VALUE_LEN-1);
            fwrite(buf,1,MAX_VALUE_LEN,tfp);
        }
    }

    free(matched);
    fclose(tfp);
    printf("Updated %d row(s)\n", matched_count);
    clock_t end = clock();
    double t = (double)(end - start)/CLOCKS_PER_SEC;
    printf("[Query executed in %.6f seconds]\n", t);
}