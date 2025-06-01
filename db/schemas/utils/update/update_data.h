#ifndef UPDATE_DATA_H
#define UPDATE_DATA_H
#include "../../schema.h"

void update_data(const char* table_name, const WhereClause* where, const UpdatePair* updates, int update_count);

#endif