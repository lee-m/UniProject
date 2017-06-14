/*
  Simple implementation of a hash table
  for storing strings.
*/

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include "linkedlist.h"

/* entry inside the table */
struct hashtab_entry
{
  char *key;
  int value;
};

/* the hash table itself */
struct hash_table
{
  struct linked_list **buckets;
  int num_buckets;
  int val_count;
  float resize_factor;
};

/* 
   Create a new hash table, INITIAL_SIZE is the initial number
   of buckets used, RESIZE_FACTOR determines how full the table 
   needs to be before it's resized. Should be in the range 
   0 < resize_factor < 1
 */
struct hash_table* create_hash_table(unsigned int initial_size, float resize_factor);

/*
  Destoy a previously created hash table, freeing all allocated memory.
*/
void destroy_hash_table(struct hash_table *table);

/* Insert a new entry into the hash table */
void insert_hashtab_entry(struct hash_table *table, char *key, int value);

/* find a value associated with the specified key. */
int find_hashtab_entry(struct hash_table *table, char *key);

#endif /* __HASHTABLE_H__ */
