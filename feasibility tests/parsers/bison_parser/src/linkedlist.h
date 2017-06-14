/*
  Implementation of a singly linked list, to create a new list
  for use call create_list(), nodes can be added to the tail
  of the list by calling insert_node_at_end(), when you're finished
  with the list call destroy_list, passing the previously created
  list to free all allocated memory
*/

#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

/* list node */
struct list_node
{
  struct list_node *next;
  void *value;
};

/* the list itself. */
struct linked_list
{
  struct list_node *head;
  struct list_node *tail;
}; 


/* create a new list ready for use. */
struct linked_list* create_list();

/* destroy a previously created list, freeing all allocated memory. */
void destroy_list(struct linked_list *list);

/* insert a new node, returns a pointer to the node in the list. */
struct list_node* add_linked_list_node(struct linked_list *list, void *val, 
				       unsigned int len);

/* remove a node from the list and destroy it. */
void remove_list_node(struct linked_list *list, struct list_node *node);

/* find the list_node whose value matches VALUE */
struct list_node* find_list_node(struct linked_list *list, void *value, unsigned int size);

#endif /* __LINKED_LIST_H__ */
