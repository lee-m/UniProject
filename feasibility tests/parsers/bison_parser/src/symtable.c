#include "symtable.h"
#include "hashtable.h"
#include <stdlib.h>

/* create a new symbol table */
struct symbol_table* create_symbol_table()
{
  struct symbol_table *new_table = malloc(sizeof(struct symbol_table));

  if(!new_table)
    abort();

  new_table->global_scope = malloc(sizeof(struct scope_node));
  new_table->current_scope = new_table->global_scope;

  new_table->global_scope->symbols = create_hash_table(100, 0.9f);
  new_table->global_scope->child_scopes = create_list();
  new_table->global_scope->parent_node = NULL;

  return new_table;
}

/* release all memory associated with a single scope */
static void destroy_scope(struct scope_node *node)
{
  struct list_node *tree_nodes = NULL;

  /* destroy the symbols at the current scope */
  destroy_hash_table(node->symbols);
  node->symbols = NULL;

  /* recurse through all child scopes, freeing each one */
  tree_nodes = node->child_scopes->head;

  while(tree_nodes)
    {
      destroy_scope((struct scope_node*)tree_nodes->value);
      tree_nodes = tree_nodes->next;
    }

  destroy_list(node->child_scopes);
  node->child_scopes = 0;
}
 
/* release all memory allocated by the symbol table SYMTAB */
void destroy_symbol_table(struct symbol_table *symtab)
{
  if(!symtab)
    return;

  destroy_hash_table(symtab->current_scope->symbols);
  destroy_list(symtab->current_scope->child_scopes);

  /* free the top level data structure objects */
  free(symtab->current_scope);
  symtab->current_scope = NULL;

  free(symtab);
  symtab = NULL;
}

/* add a new level to the scope tree */
void push_new_symbol_scope(struct symbol_table *symtab)
{
  struct scope_node *new_scope = (struct scope_node*) malloc(sizeof(struct scope_node));

  if(!new_scope)
    abort();

  new_scope->symbols = create_hash_table(100, 0.9f);
  new_scope->child_scopes = create_list();
  new_scope->parent_node = symtab->current_scope;

  add_linked_list_node(symtab->current_scope->child_scopes, new_scope, sizeof(struct scope_node));
  symtab->current_scope = new_scope;
}

/* move the current symbol scope up one level in the tree */
void pop_symbol_scope(struct symbol_table *symtab)
{
  struct scope_node *temp_node = symtab->current_scope->parent_node;

  /* we should never be popping the global scope */
  if(!temp_node)
    abort();

  /* remove the scope from the tree */
  struct list_node *dead_node = find_list_node(temp_node->child_scopes, 
					       symtab->current_scope, 
					       sizeof(symtab->current_scope));

  remove_list_node(temp_node->child_scopes, dead_node);

  /* destroy all the resources used by the current scope */
  destroy_scope(symtab->current_scope);
  free(symtab->current_scope);
  symtab->current_scope = temp_node;
}

/* 
   add a symbol called IDENTIFIER declared at LINE_NO to the table 
   SYMTAB at the current scope 
*/
int add_symbol(struct symbol_table *symtab, char *identifier, int line_no)
{
  /* see if this identifier is visible from the current scope */
  if(lookup_identifier(symtab, identifier))
    return 0;

  insert_hashtab_entry(symtab->current_scope->symbols, identifier, line_no);
  return 1;
}

/* try to find a declaration for IDENTIFIER visible from the current scope. */
int lookup_identifier(struct symbol_table *symtab, char *identifier)
{
  struct scope_node *parent_scope = NULL;

  /* check the current scope */
  if(find_hashtab_entry(symtab->current_scope->symbols, identifier) != -1)
    return 1;

  /* not found in the current scope, so walk the scope tree back to the
     root, looking at in enclosing scope as we go. */
  parent_scope = symtab->current_scope->parent_node;

  while(parent_scope)
    {
      if(find_hashtab_entry(parent_scope->symbols, identifier) != -1)
	return 1;

      parent_scope = parent_scope->parent_node;
    }

  /* symbol not found anywhere */
  return 0;
}
