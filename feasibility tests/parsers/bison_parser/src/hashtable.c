#include "hashtable.h"
#include "linkedlist.h"
#include <stdlib.h>
#include <string.h>

/* 
  Table of prime numbers sorted into ascending order 
  used to lookup the next largest prime number for a 
  given capacity. 
*/

static unsigned int prime_number_table[] =
  {
     7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 
     8191, 16381, 32749, 65521, 131071, 262139, 524287, 
     1048573, 2097143, 4194301, 8388593, 16777213, 
     33554393, 67108859, 134217689, 268435399, 536870909, 
     1073741789, 2147483647
  };

/*
  Find the next prime number which is greater than 
  a given value NUM using a simple linear scan of the
  prime number table.
*/
static unsigned int find_nearest_prime(unsigned int num)
{
  int num_entries = sizeof(prime_number_table) / sizeof(int);
  int i = 0;

  for(i = 0; i < num_entries - 1; i++)
  {
    if(prime_number_table[i] > num && num < prime_number_table[i + 1])
      return prime_number_table[i];
  }
  
  return -1;
}

/*
  Given an input string STR, compute the 32-bit hash value
  for it which will be the bucket index to store the 
  associated value into. Hash algorithm by P.J Weinberg and
  taken from the "dragon book" page 434.
*/
static unsigned int compute_hash(char *str)
{
  char *ptr = NULL;
  unsigned int hash = 0;
  unsigned int temp = 0;

  for(ptr = str; *ptr != '\0'; ptr++)
    {
      hash = (hash << 4) + (*ptr);
      temp = hash & 0xF0000000;

      if(temp)
	{
	  hash = hash ^ (temp >> 24);
	  hash = hash ^ temp;
	}
    }

  return hash;
}

/*
  Resize a hash table to (roughly) twice it's current size by
  creating a new hash table with the increased number of buckets
  then copying the values over from the old table to the new one.
*/
void resize_hash_table(struct hash_table *table)
{
  if(!table)
    return;
}

/*
  Create a new hash table with INITIAL_SIZE buckets (rounded up to the
  nearest prime). When there are (num_values / bucket_count) > resize_factor
  values in the table, it's capacity will automatically double in size.
*/
struct hash_table* create_hash_table(unsigned int initial_size, float resize_factor)
{
  unsigned int bucket_count = find_nearest_prime(initial_size);
  struct hash_table *ret_table = NULL;
  unsigned int i = 0;

  /* allocate the table and all the buckets */
  ret_table = (struct hash_table*) malloc(sizeof(struct hash_table));
  if(!ret_table)
    abort();

  ret_table->buckets = (struct linked_list**) malloc(sizeof(struct linked_list) * bucket_count);
  if(!ret_table->buckets)
    abort();

  for(i = 0; i < bucket_count; i++)
    ret_table->buckets[i] = create_list();

  /* if the given resize factor is outside the range 0 < resize_factor < 1, default to 1 */
  if(resize_factor > 1 || resize_factor < 0)
    resize_factor = 1;

  /* store resize factor and current count */
  ret_table->resize_factor = resize_factor;
  ret_table->num_buckets = bucket_count;
  ret_table->val_count = 0;

  return ret_table;
}

/*
  Free all allocated memory associated with a hash_table 
*/
void destroy_hash_table(struct hash_table *table)
{
  int i = 0;
  struct list_node *node;
  struct hashtab_entry *entry;

  if(!table)
    return;

  /* free all the contents of the buckets */
  for(i = 0; i < table->num_buckets; i++)
    {
      /* need to free all the values stored in the hash entries */
      node = table->buckets[i]->head;

      while(node)
        {
          entry = (struct hashtab_entry*)node->value;
          free(entry->key);
          node = node->next;
        }

      /* destroy the list itself */
      destroy_list(table->buckets[i]);
      table->buckets[i] = NULL;
    }

  /* deallocate the buckets and finally, the table itself*/
  free(table->buckets);
  table->buckets = NULL;

  free(table);
  table = NULL;
}

/*
  Insert a new key/value denoted by KEY and VALUE into the hash table TABLE, SIZE
  is the length of the data being inserted.
*/
void insert_hashtab_entry(struct hash_table *table, char *key, int value)
{
  /* first, compute the hash value for the key */
  unsigned key_hash = compute_hash(key);

  /* allocate a new hash table entry */
  struct hashtab_entry new_entry;
  new_entry.key = strdup(key);
  new_entry.value = value;

  /* store the value in the table */
  add_linked_list_node(table->buckets[key_hash % table->num_buckets],
		       &new_entry, sizeof(struct hashtab_entry));
  table->val_count++;

  /* do we need to resize the table? */
  if(table->val_count > (table->resize_factor * table->num_buckets))
    resize_hash_table(table);
}

/*
  Look for the value associated with KEY in the hash table TABLE. Returns the
  associated value if found, otherwise -1.
*/
int find_hashtab_entry(struct hash_table *table, char *key)
{
  struct linked_list *bucket_list = NULL;
  struct list_node *node = NULL;
  struct hashtab_entry *table_entry = NULL;

  if(!table)
    abort();

  /* hash the key to find the bucket the value might be in. */
  bucket_list = table->buckets[compute_hash(key) % table->num_buckets];
  
  /* search the bucket for the value */
  node = bucket_list->head;

  while(node)
    {
      table_entry = (struct hashtab_entry*)node->value;

      if(strcmp(table_entry->key, key) == 0)
	return table_entry->value;
      
      node = node->next;
    }

  /* value not in the table. */
  return -1;
}
