#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <stdbool.h>
#include <stdio.h>       // Needed for FILE*
#include "schema.h"

#define MAX_KEYS 4

typedef struct BPlusNode {
    bool is_leaf;
    int key_count;
    char keys[MAX_KEYS][64];
    int values[MAX_KEYS];
    struct BPlusNode* children[MAX_KEYS + 1];
    struct BPlusNode* next;
} BPlusNode;

typedef struct {
    const char* column_name;
    BPlusNode* root;
} BPlusTree;

typedef struct {
    BPlusTree trees[8];
    int count;
} IndexMap;

// --- Prototypes ---
BPlusNode* create_node(bool is_leaf);
void bpt_insert(BPlusNode** root, const char* key, int row_num);
void bpt_search_int(BPlusNode* root,
                    const char* op,    // must be "="
                    int          val,  // integer literal
                    int*         out,
                    int*         out_count);
                    void bpt_search_range(BPlusNode* root,
                      int        low,      bool low_incl,
                      int        high,     bool high_incl,
                      int*       out,
                      int*       out_count);
void bpt_search(BPlusNode* root, const char* op, const char* value, int* matches, int* match_count);
void build_indexes(FILE* fp, const TableSchema* schema, const WhereClause* where, IndexMap* index_map);
BPlusNode* get_tree(IndexMap* map, const char* column);

bool contains(int* arr, int count, int value);
void intersect(int* a, int a_count, int* b, int b_count, int* out, int* out_count);
void unite(int* a, int a_count, int* b, int b_count, int* out, int* out_count);
void filter_rows(IndexMap* index_map, const WhereClause* where, int* final_rows, int* final_count,int total_rows);

#endif
