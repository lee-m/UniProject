//------------------------------------------------------------------------------------------
// File: CLexicalScanner.h
// Desc: Class declararation for the hand written scanner.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CLEXICAL_SCANNER_H__
#define __CLEXICAL_SCANNER_H__

#include "TokenTypes.h"

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
	CLexicalScanner(void);
	~CLexicalScanner(void);

	//set the file we're reading tokens from
	void SetInputFile(const string &File);

	//returns true if there's more tokens to come
	bool HasMoreTokens(void);

	//get the next token from the input stream
	Token_t GetNextToken(void);

	//take a look at the next token without actually consuming it
	Token_t PeekNextToken(void);

	//push a token back into the input stream, this will be the next token
	//returned on the next call to GetNextToken or PeekNextToken
	void PushbackToken(Token_t Tok);

	//discard all peeked tokens 
	void ClearPeekedTokens(void);

	//which line number are we on?
	long CurrentLineNumber(void);

private:

	//tokenise the entire input file
	void TokeniseFile();

	//initialise the tokens
	void InitialiseKeywordsList(void);
	void InitialiseTypeTokens(void);
	void InitialiseControlStmtTokens(void);
	void InitialiseFunctionTokens(void);
	void InitialiseMiscTokens(void);

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
	string m_CurrTokenStr;

	//the forward pointer is the one we advance to scan for
	//multiple characters tokens such as ">=" or "==" for example,
	//the string of characters between the two pointers in the 
	//buffer is the token we're scanning
	char *m_ForwardPtr;
	
	//current line number we're scanning
	long m_CurrLineNumber;

	//if we've peeked at any token, they're stored here until
	//we call GetNextToken again in which case we'll pop the top
	//token and return that instead of reading from the file
	list<Token_t> m_PeekedTokens;

	//are we peeking at a token when we call GetNextToken?
	bool m_PeekingAtToken;
};

#endif