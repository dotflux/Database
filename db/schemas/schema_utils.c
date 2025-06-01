#include "schema.h"
#include "bplustree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "utils/create/create_table.h"
#include "utils/input/input_table.h"
#include "utils/read/read_table.h"
#include "utils/update/update_data.h"
#include "utils/delete/delete_data.h"
#include "utils/insert/insert_columns.h"
#include "utils/drop/drop_table.h"


void create_table(const char* table_name, char** tokens, int token_count){
    write_to_schema(table_name,tokens,token_count);
}

void input_table(const char* table_name, char** tokens, int token_count){
    write_to_table(table_name,tokens,token_count);
}

void select_table(const char* table_name,const WhereClause* where){
    read_table(table_name,where);
}

void update_table(const char* table_name, const WhereClause* where, const UpdatePair* updates, int update_count){
    update_data(table_name,where,updates,update_count);
}

void delete_table(const char* table_name,const WhereClause* where){
    delete_data(table_name,where);
}

void insert_table(const char* table_name,char** tokens, int token_count){
    insert_to_schema(table_name,tokens,token_count);
}

void drop_a_table(const char* table_name){
    drop_table(table_name);
}