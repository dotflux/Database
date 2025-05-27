#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "schema.h"
#include "bplustree.h"
#include <ctype.h>  // for isdigit

static bool is_number(const char* s) {
    if (!s || !*s) return false;
    for (int i = 0; s[i]; i++) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

int key_compare(const char* a, const char* b) {
    if (is_number(a) && is_number(b)) {
        int na = atoi(a), nb = atoi(b);
        return (na < nb ? -1 : (na > nb ? 1 : 0));
    }
    return strcmp(a, b);
}

// Helper function: check if array contains value
bool contains(int* arr, int count, int value) {
    for (int i = 0; i < count; i++)
        if (arr[i] == value) return true;
    return false;
}

// Intersection of two arrays a and b, result in out
void intersect(int* a, int a_count, int* b, int b_count, int* out, int* out_count) {
    *out_count = 0;
    for (int i = 0; i < a_count; i++)
        if (contains(b, b_count, a[i]))
            out[(*out_count)++] = a[i];
}

// Union of two arrays a and b, result in out
void unite(int* a, int a_count, int* b, int b_count, int* out, int* out_count) {
    memcpy(out, a, sizeof(int) * a_count);
    int i = a_count;
    for (int j = 0; j < b_count; j++)
        if (!contains(a, a_count, b[j]))
            out[i++] = b[j];
    *out_count = i;
}

// Fetch B+ tree root for column from index map
BPlusNode* get_tree(IndexMap* map, const char* column) {
    for (int i = 0; i < map->count; i++)
        if (strcmp(map->trees[i].column_name, column) == 0)
            return map->trees[i].root;
    return NULL;
}

// Build B+ tree indexes for all columns in WHERE clause
void build_indexes(FILE* fp, const TableSchema* schema, const WhereClause* where, IndexMap* index_map) {
    fseek(fp, 0, SEEK_SET);
    char row[MAX_COLUMNS][MAX_VALUE_LEN];
    int row_num = 0;

    for (int i = 0; i < where->count; i++) {
        index_map->trees[i].column_name = where->conditions[i].column;
        index_map->trees[i].root = NULL;
    }
    index_map->count = where->count;

    while (fread(row, MAX_VALUE_LEN, schema->column_count, fp) == (size_t)schema->column_count) {
        for (int i = 0; i < index_map->count; i++) {
            const char* col = index_map->trees[i].column_name;

            for (int j = 0; j < schema->column_count; j++) {
                if (strcmp(schema->columns[j].name, col) == 0) {
                    const char* val = row[j];
                    int len = strlen(val);

                    // Normalize by stripping surrounding quotes if any
                    char cleaned[256];
                    if (len >= 2 && ((val[0] == '"' && val[len - 1] == '"') || (val[0] == '\'' && val[len - 1] == '\''))) {
                        strncpy(cleaned, val + 1, len - 2);
                        cleaned[len - 2] = '\0';
                        val = cleaned;
                    }

                    bpt_insert(&index_map->trees[i].root, val, row_num);
                    break;
                }

            }
        }
        row_num++;
    }
}

// Filter rows based on WHERE clause using index_map, output matching row numbers in final_rows
void filter_rows(IndexMap* index_map, const WhereClause* where, int* final_rows, int* final_count) {
    int temp1[1024], temp2[1024];
    int count1 = 0, count2 = 0;

    if (where->count == 0) {
        *final_count = 0;
        return;
    }

    for (int i = 0; i < where->count; i++) {
        Condition cond = where->conditions[i];
        BPlusNode* tree = get_tree(index_map, cond.column);
        if (!tree) {
            // No index for column, so no matches
            *final_count = 0;
            return;
        }

        // Search rows matching this condition in B+ tree
        bpt_search(tree, cond.op, cond.value, (i == 0 ? temp1 : temp2), (i == 0 ? &count1 : &count2));

        if (i == 0) continue;

        if (where->logic_ops[i - 1] == 'A') { // AND
            intersect(temp1, count1, temp2, count2, temp1, &count1);
        } else if (where->logic_ops[i - 1] == 'O') { // OR
            unite(temp1, count1, temp2, count2, temp1, &count1);
        }
    }

    memcpy(final_rows, temp1, sizeof(int) * count1);
    *final_count = count1;
}

// Create a new B+ tree node (leaf or internal)
BPlusNode* create_node(bool is_leaf) {
    BPlusNode* node = calloc(1, sizeof(BPlusNode));
    node->is_leaf = is_leaf;
    return node;
}

// Split a leaf node, move second half keys/values to new_leaf, update linked list pointers
void split_leaf(BPlusNode* leaf, BPlusNode** new_leaf, char* up_key) {
    int mid = leaf->key_count / 2;
    *new_leaf = create_node(true);

    for (int i = mid, j = 0; i < leaf->key_count; i++, j++) {
        strcpy((*new_leaf)->keys[j], leaf->keys[i]);
        (*new_leaf)->values[j] = leaf->values[i];
        (*new_leaf)->key_count++;
    }

    leaf->key_count = mid;

    strcpy(up_key, (*new_leaf)->keys[0]);

    (*new_leaf)->next = leaf->next;
    leaf->next = *new_leaf;
}

// Split an internal node, move second half keys/children to new_node, promote middle key in up_key
void split_internal(BPlusNode* node, BPlusNode** new_node, char* up_key) {
    int mid = node->key_count / 2;
    *new_node = create_node(false);

    strcpy(up_key, node->keys[mid]); // Promote middle key

    int j = 0;
    for (int i = mid + 1; i < node->key_count; i++, j++) {
        strcpy((*new_node)->keys[j], node->keys[i]);
        (*new_node)->children[j] = node->children[i];
        (*new_node)->key_count++;
    }
    (*new_node)->children[j] = node->children[node->key_count];

    node->key_count = mid;
}


// Insert key,row_num into a non-full B+ node (leaf or internal)
void bpt_insert_nonfull(BPlusNode* node, const char* key, int row_num) {
    int i = node->key_count - 1;

    if (node->is_leaf) {
        while (i >= 0 && key_compare(key, node->keys[i]) < 0) {
            strcpy(node->keys[i + 1], node->keys[i]);
            node->values[i + 1] = node->values[i];
            i--;
        }
        strcpy(node->keys[i + 1], key);
        node->values[i + 1] = row_num;
        node->key_count++;
    } else {
        while (i >= 0 && key_compare(key, node->keys[i]) < 0) i--;
        i++;

        if (node->children[i]->key_count == MAX_KEYS) {
            BPlusNode* new_child;
            char up_key[64];
            if (node->children[i]->is_leaf)
                split_leaf(node->children[i], &new_child, up_key);
            else
                split_internal(node->children[i], &new_child, up_key);

            for (int j = node->key_count; j > i; j--) {
                node->children[j + 1] = node->children[j];
                strcpy(node->keys[j], node->keys[j - 1]);
            }

            node->children[i + 1] = new_child;
            strcpy(node->keys[i], up_key);
            node->key_count++;

            if (key_compare(key, up_key) > 0) i++;
        }

        bpt_insert_nonfull(node->children[i], key, row_num);
    }
}

// Insert key,row_num into B+ tree root, split root if full
void bpt_insert(BPlusNode** root_ptr, const char* key, int row_num) {
    BPlusNode* root = *root_ptr;
    if (!root) {
        root = create_node(true);
        strcpy(root->keys[0], key);
        root->values[0] = row_num;
        root->key_count = 1;
        *root_ptr = root;
        return;
    }

    if (root->key_count == MAX_KEYS) {
        BPlusNode* new_root = create_node(false);
        new_root->children[0] = root;

        BPlusNode* new_node;
        char up_key[64];

        if (root->is_leaf)
            split_leaf(root, &new_node, up_key);
        else
            split_internal(root, &new_node, up_key);

        new_root->children[1] = new_node;
        strcpy(new_root->keys[0], up_key);
        new_root->key_count = 1;
        *root_ptr = new_root;

        bpt_insert_nonfull(new_root, key, row_num);
    } else {
        bpt_insert_nonfull(root, key, row_num);
    }
}

// Binary search for keys matching condition in leaf nodes and collect matching row nums
void bpt_search(BPlusNode* root, const char* op, const char* val, int* out, int* out_count) {
    *out_count = 0;
    if (!root) return;

    // 1) Descend to the first leaf
    BPlusNode* node = root;
    while (!node->is_leaf) {
        int i = 0;
        // find the child pointer to follow
        while (i < node->key_count && key_compare(val, node->keys[i]) >= 0) i++;
        node = node->children[i];
    }

    // 2) Now scan through the leaf chain
    for (; node; node = node->next) {
        for (int i = 0; i < node->key_count; i++) {
            int cmp = key_compare(node->keys[i], val);
            bool match = false;

            if      (strcmp(op, "=")  == 0) match = (cmp == 0);
            else if (strcmp(op, "!=") == 0) match = (cmp != 0);
            else if (strcmp(op, "<")  == 0) match = (cmp <  0);
            else if (strcmp(op, "<=") == 0) match = (cmp <= 0);
            else if (strcmp(op, ">")  == 0) match = (cmp >  0);
            else if (strcmp(op, ">=") == 0) match = (cmp >= 0);

            if (match) {
                out[(*out_count)++] = node->values[i];
            }
        }
    }
}


// Free B+ tree nodes recursively
void bpt_free(BPlusNode* node) {
    if (!node) return;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            bpt_free(node->children[i]);
        }
    }
    free(node);
}
