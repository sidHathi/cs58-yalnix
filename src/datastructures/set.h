/* 
 * Source David Kotz's set datastructure last modified in 2021
 * 
 * Changes:
 * 
 * Removed set_print
 * Removed dependency on <string.h> and <stdbool.> - false is now ERROR and true is 0
 * Added set_pop function, allowing caller to remove and return a value by key
 * Struct fields are exposed to caller
 * Node key is now an integer
 * In set_free, is the delete function is NULL, proceed to free the nodes but not the data inside. Don't return error
 * 
 * Asher Vogel, February 2024
 */

#ifndef __SET_H
#define __SET_H

/**************** structs ****************/

// Node in the set
typedef struct set_node set_node_t;


// Struct for whole set
typedef struct set {
    set_node_t *head;
    int node_count;
} set_t;

/**************** functions ****************/

/**************** set_new ****************/
/* Create a new (empty) set.
 * 
 * We return:
 *   pointer to a new set, or NULL if error.
 * We guarantee:
 *   The set is initialized empty.
 * Caller is responsible for:
 *   later calling set_delete.
 */
set_t* set_new(void);

/**************** set_insert ****************/
/* Insert item, identified by a key (string), into the given set.
 *
 * Caller provides:
 *   valid set pointer, valid integer key, and pointer to item.
 * We return:
 *  ERROR if key exists, any parameter is NULL, or error;
 *  0 iff new item was inserted.
 * Caller is responsible for:
 *   later calling set_delete to free the memory used by key strings.
 * Notes:
 *   The key string is copied for use by the set; that is, the module
 *   is responsible for allocating memory for a copy of the key string, and
 *   later deallocating that memory; thus, the caller is free to re-use or 
 *   deallocate its key string after this call.  
 */
int set_insert(set_t* set, int key, void* item);

/**************** set_find ****************/
/* Return the item associated with the given key.
 *
 * Caller provides:
 *   valid set pointer, valid integer key.
 * We return:
 *   a pointer to the desired item, if found; 
 *   NULL if set is NULL, key is NULL, or key is not found.
 * Notes:
 *   The item is *not* removed from the set.
 *   Thus, the caller should *not* free the pointer that is returned.
 */
void* set_find(set_t* set, int key);

/**************** set_pop ****************/
/* Return the item associated with the given key.
 *
 * Caller provides:
 *   valid set pointer, valid integer key.
 * We return:
 *   a pointer to the desired item, if found; 
 *   NULL if set is NULL, key is NULL, or key is not found.
 * Notes:
 *   The item IS removed from the set.
 *   Thus, the caller SHOULD free the pointer that is returned.
 */
void* set_pop(set_t* set, int key);


/**************** set_iterate ****************/
/* Iterate over the set, calling a function on each item.
 * 
 * Caller provides:
 *   valid set pointer,
 *   arbitrary argument (pointer) that is passed-through to itemfunc,
 *   valid pointer to function that handles one item.
 * We do:
 *   nothing, if set==NULL or itemfunc==NULL.
 *   otherwise, call the itemfunc on each item, with (arg, key, item).
 * Notes:
 *   the order in which set items are handled is undefined.
 *   the set and its contents are not changed by this function,
 *   but the itemfunc may change the contents of the item.
 */
void set_iterate(set_t* set, void* arg,
                 void (*itemfunc)(void* arg, int key, void* item) );

/**************** set_delete ****************/
/* Delete set, calling a delete function on each item.
 *
 * Caller provides:
 *   valid set pointer,
 *   valid pointer to function that handles one item (may be NULL).
 * We do:
 *   if set==NULL, do nothing.
 *   otherwise, unless itemfunc==NULL, call the itemfunc on each item.
 *   free all the key strings, and the set itself.
 * Notes:
 *   We free the strings that represent key for each item, because 
 *   this module allocated that memory in set_insert.
 */
void set_delete(set_t* set, void (*itemdelete)(void* item) );

#endif // __SET_H