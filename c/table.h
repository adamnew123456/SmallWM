#ifndef __SMALLWM_TABLE__
#define __SMALLWM_TABLE__

#include <stdlib.h>
#include <string.h>

// A node of the linked list which the elements of a table_t point to - this is
// used to resolve collisions in the table.
typedef struct table_linkednode_s {
    int key;
    void *value;
    struct table_linkednode_s *next;
} table_linkednode_t;

typedef table_linkednode_t* table_t;

// This can be modified to any smallish prime number
#define TABLE_SIZE 1021

table_t *new_table();
void add_table(table_t *table, unsigned int key, void *value);
void *get_table(table_t *table, unsigned int key);
void *del_table(table_t *table, unsigned int key);
void **to_list_table(table_t *table, int *n_elems);
#endif
