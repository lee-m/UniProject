%{
#include "stdio.h"
#include "symtable.h"

/* in scanner.c */
extern int yylineno;
extern char *yytext;
int yylex(void);

/* error reporting */
int yyerror(char *s);

void DecIfOrWhileNestLevel();
void IncIfOrWhileNestLevel();
void IsFuncDefAllowed();
void LookupIdentifier();
void AddNewIdentifier();

/* is a function definition allowed at this point? */
int FuncDefAllowed = 0;

/* keep track of the number of nested if/while statements */
int NumNestedIfOrWhile = 0;

/* are we inside a function definition? */
int InsideFuncDefinition = 0; 

/* 
   does the current function return void? if so we're not
   allowed to have 'return' statements inside it.
*/
int CurrentFunctionReturnsVoid = 0;

/*
  Keep track of whether we've seen a 'return' value for
  the current function.
*/
int FoundReturnStmt = 0;

/* the global symbol table */
struct symbol_table *symtab;

#define YYERROR_VERBOSE 1
#define YYSTYPE int
%}

/* 
  TODO: 
  - type checking
  - keep track of whether 'call' is required or not when calling a function
*/

/***********************************************
		     TOKENS
************************************************/
%start	stmts

/* Types and modifiers*/
%token STRING FLOAT INTEGER BOOLEAN CONST TRUE FALSE STR_LITERAL VOID
%token IDENTIFIER
%token SEMICOLON EQUALS COMMA
%token OPENSQR_BRACKET CLOSESQR_BRACKET OPENRND_BRACKET CLOSERND_BRACKET
%token INT_DIGIT FLT_DIGIT

/* Control and looping statements. */
%token IF END_IF WHILE END_WHILE

/* arithmetic operators */
%token MULTIPLY DIVIDE

/* Boolean operators */
%token LESS_THAN LESS_THAN_EQUAL GREATER_THAN GREATER_THAN_EQUAL
%token LOGICAL_NOT
%token NOT_EQUAL 
%token LOGICAL_AND LOGICAL_OR

/* function tokens */
%token CALL RETURN FUNCTION RETURNS END_FUNCTION

/* associativity of the operators */
%left LESS_THAN LESS_THAN_EQUAL GREATER_THAN GREATER_THAN_EQUAL
%left LOGICAL_AND LOGICAL_OR
%left NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE
%left NEG
%left LOGICAL_NOT

%%

/***********************************************
	    	  MISC RULES
************************************************/

/* start point for the source file. */
first_stmt: var_decl_expr | if_stmt | while_stmt | assign_expr |
	    function_call | func_definition | return_stmt

/* sequence of statements */
stmts:  /* empty */
	| first_stmt stmts

id: IDENTIFIER                                  { LookupIdentifier(); }
		
/***********************************************
	      VARIABLE DECLARATIONS
************************************************/

type_specifier: STRING | FLOAT | BOOLEAN | INTEGER
type_id: type_specifier IDENTIFIER                         { AddNewIdentifier(); }
array_suffix: OPENSQR_BRACKET arith_expr CLOSESQR_BRACKET 

var_decl: type_id SEMICOLON
	  | type_id EQUALS assign_rhs SEMICOLON
const_var_decl: CONST type_id EQUALS bln_expr SEMICOLON
array_var_decl: type_id array_suffix SEMICOLON

var_decl_expr: var_decl
	       | const_var_decl
	       | array_var_decl

/***********************************************
	      ARITHMETIC EXPRESSIONS
************************************************/

arith_digit: INT_DIGIT | FLT_DIGIT | 
             IDENTIFIER                                      { LookupIdentifier(); }

arith_expr: arith_digit
	    | arith_expr PLUS arith_expr
	    | arith_expr MINUS arith_expr
	    | arith_expr MULTIPLY arith_expr
	    | arith_expr DIVIDE arith_expr
	    | bracketed_arith_expr
	    | MINUS arith_expr %prec NEG

bracketed_arith_expr: OPENRND_BRACKET arith_expr CLOSERND_BRACKET

/***********************************************
	      BOOLEAN EXPRESSIONS
************************************************/

bln_val: TRUE | FALSE
bracketed_bln_expr: OPENRND_BRACKET bln_expr CLOSERND_BRACKET

bln_expr: bln_val
	  | arith_expr
	  | bln_expr LESS_THAN bln_expr
	  | bln_expr LESS_THAN_EQUAL bln_expr
	  | bln_expr GREATER_THAN bln_expr
	  | bln_expr GREATER_THAN_EQUAL bln_expr
	  | bln_expr NOT_EQUAL bln_expr
	  | LOGICAL_NOT bln_expr
	  | bln_expr LOGICAL_AND bln_expr
	  | bln_expr LOGICAL_OR bln_expr

/***********************************************
	     IF/WHILE EXPRESSIONS
************************************************/
if_stmt: IF 					{ IncIfOrWhileNestLevel(); }
	 bracketed_bln_expr stmts 
         END_IF					{ DecIfOrWhileNestLevel(); }

while_stmt: WHILE 				{ IncIfOrWhileNestLevel(); }
	    bracketed_bln_expr stmts 
	    END_WHILE				{ DecIfOrWhileNestLevel(); }

/***********************************************
	    ASSIGNMENT EXPRESSIONS
************************************************/
assign_lhs: id                         
	    | id OPENSQR_BRACKET arith_expr CLOSESQR_BRACKET

assign_rhs: bln_expr | STR_LITERAL

assign_expr: assign_lhs EQUALS assign_rhs SEMICOLON

/***********************************************
	   FUNCTION DEFINITIONS/CALLS
************************************************/

returns_type_specifier: VOID			{ CurrentFunctionReturnsVoid = 1; }
			| type_specifier

function_call: CALL IDENTIFIER OPENRND_BRACKET
	       call_parameters CLOSERND_BRACKET

call_parameters: /* empty */
	    	| bln_expr
	    	| bln_expr COMMA call_parameters

parameter_list: /* empty */
		| type_id
		| type_id COMMA parameter_list

func_definition: FUNCTION			{ IsFuncDefAllowed(); }
		 IDENTIFIER OPENRND_BRACKET 
		 parameter_list CLOSERND_BRACKET
		 RETURNS returns_type_specifier
		 stmts 
		 END_FUNCTION			{ 
						  if(!FoundReturnStmt && !CurrentFunctionReturnsVoid)
						    yyerror("function does not return a value");

						  CurrentFunctionReturnsVoid = 0; 
						  FoundReturnStmt = 0;
						  InsideFuncDefinition = 0;
                                                  pop_symbol_scope(symtab);
						}

return_stmt: RETURN bln_expr SEMICOLON		{ 
						  if(!InsideFuncDefinition) 
						    yyerror("'return' not allowed outside function definition");

						  if(CurrentFunctionReturnsVoid)
						    yyerror("'return' not allowed in function returning 'void'");
						
						  FoundReturnStmt = 1;
						}

%%

/* report a parse error generated by Bison. */
int yyerror(char *s)
{
  printf("line:%d %s\n", yylineno, s);
  ++yynerrs;
}

/*
  Increment the nesting level of the current IF/WHILE statements
  so we can keep track of whether or not a function definition is
  permitted at any given point.
*/
void IncIfOrWhileNestLevel()
{
  ++NumNestedIfOrWhile; 
  FuncDefAllowed = 0; 
  push_new_symbol_scope(symtab);
}

/*
  Decrease the nesting level when we've parsed an END_IF or END_WHILE,
  if we're back at global scope then allow function definitions again.
*/
void DecIfOrWhileNestLevel()
{
  --NumNestedIfOrWhile;
						  
  if(!NumNestedIfOrWhile)
    FuncDefAllowed = 1;
  pop_symbol_scope(symtab);
}

/*
  Check to see if a function definition is allowed at this point,
  if not issue an error an continue.
*/
void IsFuncDefAllowed()
{
  if(!FuncDefAllowed)
    yyerror("function definition not allowed here");

  if(InsideFuncDefinition)
    yyerror("nested functions not allowed");

  InsideFuncDefinition = 1;
  push_new_symbol_scope(symtab);
}

void LookupIdentifier()
{
  char Msg[1024] = { "undeclared identifier " };

  if(!lookup_identifier(symtab, yytext))
    {
      strcat(Msg, yytext);
      yyerror(Msg);
    }
}

void AddNewIdentifier()
{
  if(!add_symbol(symtab, yytext, yylineno))
  {
    char Msg[1024] = { "ambiguous identifier '" };
    strcat(Msg, yytext);
    strcat(Msg, "'");
    yyerror(Msg);
  }
}

