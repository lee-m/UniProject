//------------------------------------------------------------------------------------------
// File: CUtilFuncs.h
// Desc: Various utility functions
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CUTILFUNCS_H__
#define __CUTILFUNCS_H__

//our includes
#include "TokenTypes.h"

enum ExprContext;

class CUtilFuncs
{
public:
	CUtilFuncs(void) {};
	~CUtilFuncs(void) {};

	//convert a token type to string
	static string TokenTypeToStr(TokenType Tok);

	//helper function to convert an expression parsing context into 
	//a token type to terminate parsing the expression on
	static TokenType ConvertExprContextToTokenType(ExprContext Context);
};

#endif