/* Routines for managing tables of values, indexed by integers. */
#include "table.h"

// Creates a new table with the default size
table_t *new_table()
{
    table_t *table = malloc(sizeof(table_linkednode_t*) * TABLE_SIZE);
    memset(table, 0, sizeof(table_linkednode_t*) * TABLE_SIZE);

    return table;
}

// Gets the final element of a table node
table_linkednode_t *final_node_table(table_t *table, int key)
{
    int idx = key % TABLE_SIZE;
    table_linkednode_t *node = table[idx];

    if (!node)
        return NULL;

    while (node->next)
        node = node->next;
    return node;
}

// Adds an element to the table
void add_table(table_t *table, unsigned int key, void *value)
{
    unsigned int idx = key % TABLE_SIZE;

    table_linkednode_t *node = malloc(sizeof(table_linkednode_t));
    node->key = key;
    node->value = value;
    node->next = NULL;

    // There is no element, so start the linked list
    if (!table[idx])
        table[idx] = node;
    else
    {
        table_linkednode_t *last = final_node_table(table, key);
        last->next = node;
    }
}

// Gets an element from the table
void *get_table(table_t *table, unsigned int key)
{
    unsigned int idx = key % TABLE_SIZE;

    table_linkednode_t *node = table[idx];
    while (node && node->key != key)
        node = node->next;

    if (!node)
        return NULL;
    else
        return node->value;
}

// Removes an element from the table and returns it
void *del_table(table_t *table, unsigned int key)
{
    unsigned int idx = key % TABLE_SIZE;

    table_linkednode_t *previous = NULL;
    table_linkednode_t *node = table[idx];

    while (node && node->key != key)
    {
        previous = node;
        node = node->next;
    }

    // The node fell off the end of the list and wasn't found
    if (!node)
        return NULL;

    // This is the first element of the table - the head must be changed
    if (!previous)
    {
        table[idx] = node->next;
        return node;
    }
    else
    {
        previous->next = node->next;
        return node;
    }
}

// Converts a table to a flat array
void **to_list_table(table_t *table, int *n_elems)
{
    // This requires two passes - the first gets the number of elements, and the 
    // second pass gets all the elements themselves
    *n_elems = 0;
    int idx;
    for (idx = 0; idx < TABLE_SIZE; idx++)
    {
        table_linkednode_t *node = table[idx];
        while (node)
        {
            node = node->next;
            *n_elems = *n_elems + 1;
        }
    }

    void **list_elems = malloc(sizeof(void*) * *n_elems);
    int list_idx = 0;

    for (idx = 0; idx < TABLE_SIZE; idx++)
    {
        table_linkednode_t *node = table[idx];
        while (node)
        {
            list_elems[list_idx++] = node->value;
            node = node->next;
        }
    }

    return list_elems;
}
