//------------------------------------------------------------------------------------------
// File: TokenTypes.h
// Desc: Different tokens returned from the lexical scanner.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __TOKENTYPES_H__
#define __TOKENTYPES_H__

/** 
 * The different types of tokens created by the lexical scanner.
 */
enum TokenType
{
	//reserved keywords
	TOKTYPE_INTEGERTYPE,
	TOKTYPE_FLOATTYPE,
	TOKTYPE_BOOLEANTYPE,
	TOKTYPE_STRINGTYPE,
	TOKTYPE_CONST,
	TOKTYPE_VOID,

	TOKTYPE_IF,
	TOKTYPE_ELSE,
	TOKTYPE_ENDIF,
	TOKTYPE_WHILE,
	TOKTYPE_END_WHILE,

	TOKTYPE_FUNCTION,
	TOKTYPE_RETURNS,
	TOKTYPE_END_FUNCTION,
	TOKTYPE_RETURN,

	TOKTYPE_TRUE,
	TOKTYPE_FALSE,

	//misc
	TOKTYPE_ID,
	TOKTYPE_STRINGLITERAL,
	TOKTYPE_INTLITERAL,
	TOKTYPE_FLOATLITERAL,
	TOKTYPE_ASSIGNMENT,

	TOKTYPE_OPENPAREN,
	TOKTYPE_CLOSEPAREN,
	TOKTYPE_OPENSQRPAREN,
	TOKTYPE_CLOSESQRPAREN,

	TOKTYPE_SEMICOLON,
	TOKTYPE_COMMA,

	//arithmetic operators
	TOKTYPE_PLUS,
	TOKTYPE_MINUS,
	TOKTYPE_DIVIDE,
	TOKTYPE_MULTIPLY,

	//relational operators
	TOKTYPE_LESSTHAN,
	TOKTYPE_LESSTHANEQ,
	TOKTYPE_GREATTHAN,
	TOKTYPE_GREATTHANEQ,

	//logical operators
	TOKTYPE_NOT,
	TOKTYPE_NOTEQ,
	TOKTYPE_EQUALITY,
	TOKTYPE_OR,
	TOKTYPE_AND
};

#endif
