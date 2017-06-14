/*
  The symbol table.
*/

/* a single node within the scope tree */
#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

/* an individual node within the scope tree */
struct scope_node
{
  struct hash_table *symbols;
  struct linked_list *child_scopes;
  struct scope_node *parent_node;
};

/* the symbol table */
struct symbol_table
{
  /* global scope */
  struct scope_node *global_scope;

  /* current child scope we're inserting symbols/other child scopes into */
  struct scope_node *current_scope;
};

/* create a new symbol table reasy for use */
struct symbol_table* create_symbol_table();

/* destroy a previously created symbol table, releasing all allocated memory */
void destroy_symbol_table(struct symbol_table *symtab);

/* add a child scope to the currently active scope */
void push_new_symbol_scope(struct symbol_table *symtab);

/* remove a child scope from the currently active scope */
void pop_symbol_scope(struct symbol_table *symtab);

/* 
  add a new symbol IDENTIFIER defined on LINE_NO to the currently 
  active scope. Returns 1 if the symbol was actually added or 0 on error.
*/
int add_symbol(struct symbol_table *symtab, char *identifier, int line_no);

/* 
  lookup an identifier in the symbol at the current scope in the symbol
  table SYMTAB, searching any enclosing scopes. Returns 1 if the a declaration 
  for IDENTIFIER can be seen from the current scope, otherwise 0.
*/
int lookup_identifier(struct symbol_table *symtab, char *identifier); 

#endif /* __SYMTABLE_H__ */
