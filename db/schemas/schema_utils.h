#ifndef SCHEMA_UTILS_H
#define SCHEMA_UTILS_H
#include "schema.h"

void write_to_schema(const char* table_name, char** tokens, int token_count);
void write_to_table(const char* table_name, char** tokens, int token_count);
void read_table(const char* table_name,const WhereClause* where);

#endif