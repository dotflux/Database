#ifndef SCHEMA_H
#define SCHEMA_H

#define MAX_NAME_LEN 32
#define MAX_TYPE_LEN 16
#define MAX_COLUMNS 32
#define MAX_VALUE_LEN 256
#define MAX_CONDITIONS 64

typedef struct {
    char name[MAX_NAME_LEN];
    char type[MAX_TYPE_LEN];
} Column;

typedef struct {
    char table_name[MAX_NAME_LEN];
    int column_count;
    Column columns[MAX_COLUMNS];
} TableSchema;

typedef struct {
    char column[MAX_NAME_LEN];
    char op[3]; // ":", ">", "<", ">=", etc.
    char value[MAX_VALUE_LEN];
} Condition;

typedef struct {
    Condition conditions[MAX_CONDITIONS];
    char logic_ops[MAX_CONDITIONS]; // 'A' for AND, 'O' for OR
    int count;
} WhereClause;

typedef struct {
    char column[MAX_NAME_LEN];
    char value[MAX_VALUE_LEN];
} UpdatePair;


#endif
