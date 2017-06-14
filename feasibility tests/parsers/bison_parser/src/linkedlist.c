#include "linkedlist.h"
#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

/* helper function to find the node prior to NODE */
static struct list_node* find_previous(struct linked_list *list, struct list_node *node)
{
  return NULL;
}

/*
  Construct and return a new linked list.
*/
struct linked_list* create_list()
{
  struct linked_list *ret = (struct linked_list*)malloc (sizeof (struct linked_list));

  if(!ret)
    abort();

  ret->head = NULL;
  ret->tail = NULL;
  return ret;
}

/*
  Iterate through the list, freeing all allocated memory
  as it goes before finally freeing the list itself. */
void destroy_list(struct linked_list *list)
{
  struct list_node* node = NULL;
  struct list_node *temp = NULL;

  if (!list)
    return;

  node = list->head;

  while (node)
    {
      temp = node;
      node = node->next;

      free (temp->value);
      free (temp);
      temp = NULL;
    }

  free (list);
  list = NULL;
}

/*
  Insert a new node at the end of LIST with a node value of VAL. */
struct list_node* add_linked_list_node(struct linked_list *list, void *val, unsigned int size)
{
  struct list_node *new_node = malloc(sizeof(struct list_node));

  if (!new_node)
    return NULL;

  new_node->next = NULL;
  new_node->value = malloc(size);

  if(!new_node->value)
    abort();

  memcpy(new_node->value, val, size);

  /* have we got any nodes already? */
  if (!list->head)
    {
      list->head = new_node;
      list->tail = list->head;
    }
  else
    {
      /* insert the node at the end */
      list->tail->next = new_node;
      list->tail = list->tail->next;
    }

  return new_node;
}

/* remove a node from the list and destroy it. */
void remove_list_node(struct linked_list *list, struct list_node *node)
{
  struct list_node *dead_node = NULL;

  /* are we deleting the head node? */
  if(node = list->head)
    {
      dead_node = list->head;
      list->head = list->head->next;
    }
  else if(node = list->tail)
    {
      /* are we deleting the tail node? */
      dead_node = list->tail;
      list->tail = find_previous(list, list->tail);
    }
  else
    {
      /* deleting from in the middle */
      dead_node = find_previous(list, node);
      dead_node->next = node->next;
    }

  /* destroy the node */
  if(dead_node)
    {
      free(dead_node->value);
      dead_node->value = NULL;

      free(dead_node);
      dead_node = NULL;
    }
}

/*
 find the list node (if any) with VALUE. SIZE if the size of the 
 memort pointed to by VALUE.
 */
struct list_node* find_list_node(struct linked_list *list, void *value, 
				 unsigned int size)
{
  struct list_node *temp = list->head;

  while(temp)
    {
      if(memcmp(temp->value, value, size) == 0)
	return temp;

      temp = temp->next;
    }

  return NULL;
}
