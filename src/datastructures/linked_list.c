#include "linked_list.h"
#include <hardware.h>
#include <ylib.h>

//type def for a function passed as a parameter to free a LL for any data type


// A utility function to create an empty LL
// Caller is responsible freeing LL memory later.
linked_list_t* linked_list_create() {
  linked_list_t* ll = (linked_list_t*) malloc(sizeof(linked_list_t));
	ll->front = ll->rear = NULL;
	ll->next_key = 0;
	return ll;
}

// The function to add a new node to the LL
void linked_list_push(linked_list_t* ll, void* data) {
  // Allocate memory for new node
	lnode_t* new_node = (lnode_t*) malloc(sizeof(lnode_t));
	new_node->data = data;
	new_node->key = ll->next_key;
	new_node->next = NULL;
  new_node->prev = NULL;


	// If LL is empty, then new node is front and rear
	if (ll->rear == NULL) {
		ll->front = ll->rear = new_node;
		return;
	}

	// Add the new node at the end of ll and change rear
	ll->rear->next = new_node;
  new_node->prev = ll->rear;
	ll->rear = new_node;


	// Increment next_key
	ll->next_key++;

}

// Function to pop data from the LL
void* linked_list_remove(linked_list_t* ll, int key) {

  lnode_t* curr = ll->front;
  while(curr->key != key) {
    if (curr == NULL)  {
      return NULL;
    }
  }
  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;

  void* data = curr->data;
  free(curr);

  return data;
}