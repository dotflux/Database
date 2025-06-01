#ifndef SCHEMA_UTILS_H
#define SCHEMA_UTILS_H
#include "schema.h"

void create_table(const char* table_name, char** tokens, int token_count);
void input_table(const char* table_name, char** tokens, int token_count);
void select_table(const char* table_name,const WhereClause* where);
void update_table(const char* table_name, const WhereClause* where, const UpdatePair* updates, int update_count);
void delete_table(const char* table_name,const WhereClause* where);

#endif