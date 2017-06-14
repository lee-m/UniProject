//------------------------------------------------------------------------------------------
// File: CLexicalScanner.h
// Desc: Class declararation for the hand written scanner.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CLEXICAL_SCANNER_H__
#define __CLEXICAL_SCANNER_H__

#include <string>
#include <fstream>
#include <map>
using namespace std;

//different types of tokens
enum TokenType
{
	//reserved keywords
	TOKTYPE_PROGRAM = 0,
	TOKTYPE_ENDPROGRAM,
	TOKTYPE_INTEGERTYPE,
	TOKTYPE_FLOATTYPE,
	TOKTYPE_BOOLEANTYPE,
	TOKTYPE_STRINGTYPE,
	TOKTYPE_IF,
	TOKTYPE_ENDIF,
	TOKTYPE_WHILE,
	TOKTYPE_ENDWHILE,
	TOKTYPE_FUNCTION,
	TOKTYPE_RETURNS,
	TOKTYPE_RETURN,
	TOKTYPE_ENDFUNC,
	TOKTYPE_TRUE,
	TOKTYPE_FALSE,
	TOKTYPE_CONST,

	//misc
	TOKTYPE_NAME,
	TOKTYPE_STRING,			//TOKTYPE_NAME is names of vars/funcs etc, TOKTYPE_STRING is "foo" for example
	TOKTYPE_INTDIGIT,
	TOKTYPE_FLOATDIGIT,
	TOKTYPE_ASSIGNMENT,
	TOKTYPE_OPENPAREN,
	TOKTYPE_CLOSEPAREN,
	TOKTYPE_SEMICOLON,
	TOKTYPE_OPENSQRPAREN,
	TOKTYPE_CLOSESQRPAREN,
	TOKTYPE_COMMA,

	//arithmetic operators
	TOKTYPE_PLUS,
	TOKTYPE_DIVIDE,
	TOKTYPE_MULTIPLY,
	TOKTYPE_SUBTRACT,

	//logical operators
	TOKTYPE_LESSTHAN,
	TOKTYPE_LESSTHANEQ,
	TOKTYPE_GREATTHAN,
	TOKTYPE_GREATTHANEQ,
	TOKTYPE_NOT,
	TOKTYPE_NOTEQ,
	TOKTYPE_EQUALITY,
	TOKTYPE_OR,
	TOKTYPE_AND
};

typedef map<string, TokenType> TokenMap;

//definition of a token scanned from the input file
struct Token_t
{
	//the value as appeared in the source code.
	string TokenValue;

	//line number the token appeared on
	long LineNumber;

	//what type of token this is
	TokenType Type;
};

class CLexicalScanner
{
public:
	CLexicalScanner(const string &InputFile);
	~CLexicalScanner(void);

	//returns true if there's more tokens to come
	bool HasMoreTokens(void);

	//get the next token from the input stream
	Token_t GetNextToken(void);

private:

	//initialise the reserved keywords list
	void InitialiseKeywordsList();

	//returns true if the scanned identifier is of a given type
	bool IsKeyword(const string &TokStr);
	bool IsMiscToken(const string &TokStr);

	//eat whitespace and comments which are ignored
	void EatWhitespaceAndComments(void);

	//refresh the input buffer and reset m_ForwardPtr
	void RefreshInputBuffer(void);

	//increment the forward pointer and check to see if we've reached 
	//the end of the input buffer. Returns the character at the current position
	char IncForwardPtr(void);

	//return the different types of possible token, subroutine 
	//of GetNextToken().
	void ScanIdentifierToken(void);
	void ScanDigitToken(void);
	void ScanStringToken(void);
	void ScanLogicalOr(void);
	void ScanLogicalAnd(void);

	//get the type of the token we've just scanned
	TokenType GetTokenType(void);

	//handle to the file we're reading tokens from
	ifstream m_FileHandle;

	//token maps for the different types
	TokenMap m_Keywords;
	TokenMap m_MiscTokens;

	//buffer of characters read from the input file in one
	//read operation to cut down on file IO and it's size.
	char *m_InputBuffer;
	const int m_InputBufferSize;

	//we copy characters from the input buffer to this one whilst
	//we are in the process of determining the current token
	string m_CurrToken;

	//the forward pointer is the one we advance to scan for
	//multiple characters tokens such as ">=" or "==" for example,
	//the string of characters between the two pointers in the 
	//buffer is the token we're scanning
	char *m_ForwardPtr;
	
	//current line number we're scanning
	long m_CurrLineNumber;
};

#endif