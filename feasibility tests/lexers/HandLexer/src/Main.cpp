// HandLexer.cpp : Defines the entry point for the console application.
//

#include "CLexicalScanner.h"
#include <iostream>
#include <string>


//helper function to convert a token type to a string
//for printing to the console
string TokenTypeToString(TokenType Type)
{
	switch(Type)
	{
	case TOKTYPE_PROGRAM:
		return "begin program";

	case TOKTYPE_ENDPROGRAM:
		return "end program";

	case TOKTYPE_INTEGERTYPE:
		return "integer type";

	case TOKTYPE_FLOATTYPE:
		return "float type";

	case TOKTYPE_BOOLEANTYPE:
		return "boolean type";

	case TOKTYPE_STRINGTYPE:
		return "string type";

	case TOKTYPE_STRING:
		return "string";

	case TOKTYPE_IF:
		return "if";

	case TOKTYPE_ENDIF:
		return "end_if";
			
	case TOKTYPE_WHILE:
		return "while";

	case TOKTYPE_ENDWHILE:
		return "end while";

	case TOKTYPE_FUNCTION:
		return "function";

	case TOKTYPE_RETURNS:
		return "returns";

	case TOKTYPE_ENDFUNC:
		return "end_function";

	case TOKTYPE_TRUE:
		return "true";

	case TOKTYPE_FALSE:
		return "false";

	case TOKTYPE_INTDIGIT:
		return "integer digit";

	case TOKTYPE_FLOATDIGIT:
		return "float digit";

	case TOKTYPE_ASSIGNMENT:
		return "assignment";

	case TOKTYPE_OPENPAREN:
		return "open paren";

	case TOKTYPE_CLOSEPAREN:
		return "close paren";

	case TOKTYPE_SEMICOLON:
		return "semi-colon";

	case TOKTYPE_LESSTHAN:
		return "less than";

	case TOKTYPE_LESSTHANEQ:
		return "less than or equal";

	case TOKTYPE_GREATTHAN:
		return "greater than";

	case TOKTYPE_GREATTHANEQ:
		return "greater than or equal";

	case TOKTYPE_NOT:
		return "not";

	case TOKTYPE_NOTEQ:
		return "not equal";

	case TOKTYPE_EQUALITY:
		return "equality";

	case TOKTYPE_OR:
		return "or";

	case TOKTYPE_AND:
		return "and";

	case TOKTYPE_NAME:
		return "name";

	case TOKTYPE_PLUS:
		return "addition op";

	case TOKTYPE_SUBTRACT:
		return "subtract op";

	case TOKTYPE_MULTIPLY:
		return "multiply op";

	case TOKTYPE_DIVIDE:
		return "divide op";
	}

	throw exception("unknown token type");
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		cout << "no input file specified" << endl;
		system("pause");
		return 1;
	}

	CLexicalScanner Scanner(argv[1]);

	while(Scanner.HasMoreTokens())
	{
		Token_t CurrToken = Scanner.GetNextToken();

		//print out some info about the token we've just scanned
		cout << "token type: " << TokenTypeToString(CurrToken.Type) << endl;
		cout << "token value: " << CurrToken.TokenValue << endl;
		cout << endl;

	}

	system("pause");
	return 0;
}

