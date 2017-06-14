//------------------------------------------------------------------------------------------
// File: CUtilFuncs.cpp
// Desc: Various utility functions
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CUtilFuncs.h"
#include "CParser.h"

//-------------------------------------------------------------
string CUtilFuncs::TokenTypeToStr(TokenType Tok)
{
	switch(Tok)
	{
	case TOKTYPE_INTLITERAL:
	case TOKTYPE_INTEGERTYPE:
		return "integer";

	case TOKTYPE_FLOATLITERAL:
	case TOKTYPE_FLOATTYPE:
		return "float";

	case TOKTYPE_BOOLEANTYPE:
		return "boolean";

	case TOKTYPE_STRINGLITERAL:
	case TOKTYPE_STRINGTYPE:
		return "string";

	case TOKTYPE_CONST:
		return "const";

	case TOKTYPE_VOID:
		return "void";

	case TOKTYPE_IF:
		return "if";

	case TOKTYPE_ENDIF:
		return "end_if";
			
	case TOKTYPE_WHILE:
		return "while";

	case TOKTYPE_END_WHILE:
		return "end while";

	case TOKTYPE_FUNCTION:
		return "function";

	case TOKTYPE_RETURNS:
		return "returns";

	case TOKTYPE_RETURN:
		return "return";

	case TOKTYPE_END_FUNCTION:
		return "end_function";

	case TOKTYPE_TRUE:
		return "true";

	case TOKTYPE_FALSE:
		return "false";

	case TOKTYPE_ASSIGNMENT:
		return "assignment";

	case TOKTYPE_OPENPAREN:
		return "(";

	case TOKTYPE_CLOSEPAREN:
		return ")";

	case TOKTYPE_OPENSQRPAREN:
		return "[";

	case TOKTYPE_CLOSESQRPAREN:
		return "]";

	case TOKTYPE_SEMICOLON:
		return ";";

	case TOKTYPE_COMMA:
		return ",";

	case TOKTYPE_LESSTHAN:
		return "<";

	case TOKTYPE_LESSTHANEQ:
		return "<=";

	case TOKTYPE_GREATTHAN:
		return ">";

	case TOKTYPE_GREATTHANEQ:
		return ">=";

	case TOKTYPE_NOT:
		return "!";

	case TOKTYPE_NOTEQ:
		return "!=";

	case TOKTYPE_EQUALITY:
		return "==";

	case TOKTYPE_OR:
		return "||";

	case TOKTYPE_AND:
		return "&&";

	case TOKTYPE_ID:
		return "identifier";

	case TOKTYPE_PLUS:
		return "+";

	case TOKTYPE_MINUS:
		return "-";

	case TOKTYPE_MULTIPLY:
		return "*";

	case TOKTYPE_DIVIDE:
		return "/";

	default:
		return "UNKNOWN";
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CUtilFuncs::ConvertExprContextToTokenType(ExprContext Context)
{
	switch(Context)
	{
	case EC_INITIALISER:
		return TOKTYPE_SEMICOLON;

	case EC_ARRAY:
		return TOKTYPE_CLOSESQRPAREN;
		
	case EC_FUNCCALLPARM:
		return TOKTYPE_COMMA;

	case EC_CONDITIONAL:
		return TOKTYPE_CLOSEPAREN;

	case EC_FUNCCALLPARMS:
		return TOKTYPE_COMMA;
	}

	assert(false);

	//useless, but stops the compiler complaining
	return (TokenType)-1;
}
//-------------------------------------------------------------