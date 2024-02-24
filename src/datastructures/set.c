/* 
 * set.c - CS50 'set' module
 * 
 * see set.h for more information
 * 
 * Asher Vogel
 * CS 50, Fall 2022
 */

#include <stdlib.h>
#include <yalnix.h>
#include <hardware.h>
#include <ylib.h>
#include "set.h"

// Node in the set
typedef struct set_node {
    int key;
    void* item;
    struct set_node *next;
} set_node_t;

/**************** local functions ****************/

static set_node_t* set_node_new(int key, void* item) {
    if (key < 0) {
        TracePrintf(1, "Key provided was null\n");
        return NULL;
    }
 
    if (item == NULL) {
        TracePrintf(1, "Item provided for key %s was null\n", key);
        return NULL;
    }

    set_node_t* node = malloc(sizeof(set_node_t));

    if (node == NULL) {
        TracePrintf(1, "Error allocating memory for set node\n");
        return NULL;
    }

    node->key = key;
    node->item = item;
    node->next = NULL;
    return node;
}

/**************** GLOBAL FUNCTIONS ****************/

set_t* set_new(void) {
    set_t* set = malloc(sizeof(set_t));

    if (set == NULL) {
        TracePrintf(1, "Error allocating memory for set\n");
        return NULL;
    }

    set->head = NULL;
    set->node_count = 0;
    return set;
}

int set_insert(set_t* set, int key, void* item) {
    // ensure set pointer is valid
    if (set == NULL) {
        TracePrintf(1, "Error finding set\n");
        return ERROR;
    }
    // ensure key is valid
    if (key < 0) {
        TracePrintf(1, "Cannot create set node with null key\n");
        return ERROR;
    }
    // ensure item is valid
    if (item == NULL) {
        TracePrintf(1, "Cannot create set node with null item\n");
        return ERROR;
    }
    // look for key in set
    for (set_node_t *node = set->head; node != NULL; node = node->next) {
        if (key == node->key) {
            TracePrintf(1, "Key %d already exists in set\n", key);
            return ERROR;
        }
    }
    // node is valid. add it to the head of the linked list
    set_node_t *node = set_node_new(key, item);
    if (node == NULL) {
      TracePrintf(1, "Error allocating memory for new set node\n");
      return ERROR;
    }
    node->next = set->head;
    set->head = node;
    set->node_count++;
    return 0;
}

void* set_find(set_t* set, int key) {
    // ensure set pointer is valid
    if (set == NULL) {
        TracePrintf(1, "Error finding set\n");
        return NULL;
    }
    // ensure key is valid
    if (key < 0) {
        TracePrintf(1, "Cannot create set node with null key\n");
        return NULL;
    }
    // look for key in set. return pointer to item if key is found
    for (set_node_t *node = set->head; node != NULL; node = node->next) {
        if (key == node->key) {
            return node->item;
        }
    }
    // key not found, return NULL
    TracePrintf(1, "Key %s not found in set\n", key);
    return NULL;
}

void* set_pop(set_t* set, int key) {
    // ensure set pointer is valid
    if (set == NULL) {
        TracePrintf(1, "Error finding set\n");
        return NULL;
    }
    // ensure key is valid
    if (key < 0) {
        TracePrintf(1, "Cannot create set node with null key\n");
        return NULL;
    }
    // ensure set has elements
    if (set->head == NULL) {
        TracePrintf(1, "Cannot pop from empty set\n");
        return NULL;
    }


    set_node_t* prev = NULL;
    for (set_node_t *node = set->head; node != NULL; node = node->next) {
        
        // Key found
        if (key == node->key) {
            // Grab item
            void* item = node->item;

            // Handle deletion
            if (prev == NULL) {
                // removing only node
                if (node->next == NULL) { set->head = NULL; }
                // removing head node
                else { set->head = node->next; }
            }
            // removing from middle or end
            else {
                prev->next = node->next;
            }
            free(node);
            set->node_count--;
            return item;
        }
        prev = node;
    }
    // key not found, return NULL
    TracePrintf(1, "Key %s not found in set\n", key);
    return NULL;
}

void set_iterate(set_t* set, void* arg,
                 void (*itemfunc)(void* arg, int key, void* item) ) {
                     // ensure set pointer is valid
                     if (set == NULL) {
                         TracePrintf(1, "Null set pointer passed\n");
                     }
                     // ensure itemfunc pointer is valid
                     else if (itemfunc == NULL) {
                         TracePrintf(1, "Null itemfunc pointer passed\n");
                     }
                     // call itemfunc on each item with (arg, key, item)
                     else {
                         for (set_node_t* node = set->head; node != NULL; node = node->next) {
                             itemfunc(arg, node->key, node->item);
                         }
                     }
                 }

void set_delete(set_t* set, void (*itemdelete)(void* item) ) {
    // ensure set pointer is valid
    if (set == NULL) {
        TracePrintf(1, "Null set pointer passed\n");
    }
    // ensure itemdelete is valid
    else if (itemdelete == NULL) {
        TracePrintf(1, "Null itemdelete pointer passed\n");
    }
    // call itemdelete on each node and free the key string
    for (set_node_t* node = set->head; node != NULL; ) {
        set_node_t* next = node->next;
        if (node->item != NULL && itemdelete != NULL) {
            (*itemdelete)(node->item);
        }
        free(node);
        node = next;
    }
    // free pointer to the set
    free(set);
}