#ifndef SELECT_H
#define SELECT_H
#include "../../db/schemas/schema.h"

void parse_conditions(char** tokens, int start, int end, WhereClause* where);
void handle_select(char** tokens);

#endif