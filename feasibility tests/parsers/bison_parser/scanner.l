%{
#include "parser.h"
#include "symtable.h"
int yyerror(char *s);
int yylineno = 1;

extern struct symbol_table *symtab;
%}

symbol		!|£|$|%|\^|&|\*|\(|\)|-|\+|=|\{|\}|\[|\]|:|;|@|'|~|#|\||\\|<|.|,|>|\?|_|"/"
underscore	"_"
letter		[a-zA-Z]
id		{underscore}*{letter}+({underscore}|{int_digit}|{letter})*
delim		[ \t]
whitespace	{delim}*
int_digit	[0-9]+
flt_digit	[0-9]+(.[0-9]+)?
strings		\"({symbol}|{letter}|{int_digit})*\"

%%
{whitespace}
"\n"		{ ++yylineno; }
"+"		{ return PLUS; }
"-"		{ return MINUS; }
"*"		{ return MULTIPLY; }
"/"		{ return DIVIDE; }
","		{ return COMMA; }
"["		{ return OPENSQR_BRACKET; }
"]"		{ return CLOSESQR_BRACKET; }
"("		{ return OPENRND_BRACKET; }
")"		{ return CLOSERND_BRACKET; }
"<"		{ return LESS_THAN; }
"<="		{ return LESS_THAN_EQUAL; }
">"		{ return GREATER_THAN; }
">="		{ return GREATER_THAN_EQUAL; }
"!"		{ return LOGICAL_NOT; }
"!="		{ return NOT_EQUAL; }
"&&"		{ return LOGICAL_AND; }
"||"		{ return LOGICAL_OR; }
true		{ return TRUE; }
false		{ return FALSE; }
"="		{ return EQUALS; }
{int_digit}	{ return INT_DIGIT; }
{flt_digit}	{ return FLT_DIGIT; }
;		{ return SEMICOLON; }
integer		{ return INTEGER; }
boolean 	{ return BOOLEAN; }
string  	{ return STRING; }
float   	{ return FLOAT; }
void 		{ return VOID; }
if		{ return IF; }
end_if		{ return END_IF; }
while		{ return WHILE; }
end_while	{ return END_WHILE; }
const		{ return CONST; }
call		{ return CALL; }
returns		{ return RETURNS; }
function	{ return FUNCTION; }
end_function 	{ return END_FUNCTION; }
return		{ return RETURN; }
{strings}	{ return STR_LITERAL; }
{id}		{ return IDENTIFIER; }

