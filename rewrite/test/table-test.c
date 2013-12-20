#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../table.h"

// Adds a new element where the value is an int_t of the key
void add(table_t *table, int key)
{
    int *i = malloc(sizeof(int));
    *i = key;
    add_table(table, key, i);
}

int main()
{
    // First, create and populate a table
    table_t *table = new_table();

    int idx;
    /* Looping this far means a couple of things:
     *  (a) That all the indicies between [0, TABLE_SIZE) get filled up
     *  (b) That there are key collisions
    */
    for (idx = 0; idx < (int)(TABLE_SIZE * 1.5); idx++)
    {
        add(table, idx);
    }

    // Test all inserted keys for existance
    for (idx = 0; idx < (int)(TABLE_SIZE * 1.5); idx++)
    {
        int *val = get_table(table, idx);
        assert(*val == idx);
    }

    // This is a key not in the table
    assert(get_table(table, TABLE_SIZE * 2) == NULL);

    // Try removing some things to make sure the removal process works
    for (idx = 0; idx < (int)(TABLE_SIZE * 1.5); idx += 3)
    {
        int *x = del_table(table, idx);
        assert(*x == idx);
        free(x);
    }

    // Ensure that turning the table into a list works
    int n_elems;
    void **elements = to_list_table(table, &n_elems);

    // Print out each element, at least to try and make sure they are all ther
    for (idx = 0; idx < n_elems; idx++)
    {
        int *i = elements[idx];
        printf("elements[%d] = %d\n", idx, *i);
    }

    free(elements);

    // Delete everything
    for (idx = 0; idx < (int)(TABLE_SIZE * 1.5); idx++)
    {
        int *x = del_table(table, idx);
        assert(*x == idx);
        free(x);
    }

    // Ensure that nothing was saved
    for (idx = 0; idx < (int)(TABLE_SIZE * 1.5); idx++)
    {
        assert(get_table(table, idx) == NULL);
    }
}
