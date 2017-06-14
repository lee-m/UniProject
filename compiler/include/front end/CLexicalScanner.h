//------------------------------------------------------------------------------------------
// File: CLexicalScanner.h
// Desc: Class declararation for the hand written scanner.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CLEXICAL_SCANNER_H__
#define __CLEXICAL_SCANNER_H__

#include "TokenTypes.h"

typedef map<string, TokenType> TokenMap;

/**
 * Definition of a token scanned from the input file. This is the 
 * representation used throughout the rest of the front-end for dealing
 * with tokens.
 */
struct Token_t
{
	/** The value as appeared in the source code. */
	string TokenValue;

	/** Line number the token appeared on. */
	long LineNumber;

	/** What type of token this is. */
	TokenType Type;
};

/**
 * A simple deterministic based automaton to handle the recognition of the 
 * various tokens that the language defines. To make recognition easier we 
 * store the string of each token alongside it's type in a std::map so once
 * a token has been scanned, we can determine it's type in constant time. To
 * add a new token to the scanner add a new enumeration value for it in 
 * TokenTypes.h and add code to one of the InitialiseXXX functions to add it
 * to the token map so it will be recognised. 
 * <br><br>
 * Internally the scanner buffers it's input to minimise the amount of file
 * operations required. The exact size of the buffer is controlled by
 * m_InputBufferSize and defaults to 4K. Refreshing of the buffer is handled
 * automatically once it's detected that we're at the end of it, using a larger
 * buffer could improve performance at the expense of more memory used whilst a 
 * smaller buffer lessens memory requirements but impacts performance as it will
 * need to be refreshed more often.
 */
class CLexicalScanner
{
public:
	CLexicalScanner(void);
	~CLexicalScanner(void);

	/**
	 * Set the file we're reading tokens from.
	 * @param File Name of the file we're scanning.
	 * @return True if the file was opened ok, false on error.
	 */
	bool SetInputFile(const string &File);

	/** 
	 * Determine if there are any more tokens to come.
	 * @return True if there are more tokens, false if there isn't.
	 */
	bool HasMoreTokens(void);

	/**
	 * Get the next token from the input stream.
	 * @return A new Token_t object for the next token.
	 */
	Token_t GetNextToken(void);

	/**
	 * Take a look at the next token without actually consuming it. This
	 * will be the next token returned by GetNextToken. This only works for
	 * a single token lookahead, if more is required then the necessary tokens
	 * can be reinserted using PushbackToken.
	 * @return A new Token_t object for the peeked token.
	 */
	Token_t PeekNextToken(void);

	/**
	 * Push a token back into the input stream, this will be the next token
	 * returned on the next call to GetNextToken or PeekNextToken.
	 * @param Tok Token to push back into the input stream.
	 */
	void PushbackToken(Token_t Tok);

	/** Discard all peeked tokens. */
	void ClearPeekedTokens(void);

	/**
	 * Query which line number we're processing.
	 * @return The line number we're processing.
	 */
	long CurrentLineNumber(void);

private:

	/** Add any keyword tokens to the token map. */
	void InitialiseKeywordsList(void);

	/** Add the variable type tokens to the token map. */
	void InitialiseTypeTokens(void);

	/** Add the control statement tokens - if/while etc  to the token map */
	void InitialiseControlStmtTokens(void);

	/** Add those tokens which are related to functions to the token map. */
	void InitialiseFunctionTokens(void);

	/** Add any tokens not covered above to the token map. */
	void InitialiseMiscTokens(void);

	/** 
	 * See if a token value is a keyword. 
	 * @param TokStr The value of the token being checked.
	 * @return True if the token's value is a keyword, false if it's not.
	 */
	bool IsKeyword(const string &TokStr);

	/** 
	 * See if this token is covered by the misc tokens map.
	 * @param TokStr The token value to check.
	 * @return True if the token is a misc token, false if it's not.
	 */
	bool IsMiscToken(const string &TokStr);

	/** Eat whitespace and comments which are ignored. */
	void EatWhitespaceAndComments(void);

	/** Refresh the input buffer and reset m_ForwardPtr. */
	void RefreshInputBuffer(void);

	/**
	 * Increment the forward pointer and check to see if we've reached
	 * the end of the input buffer. 
	 * @return The character at the current position.
	 */
	char IncForwardPtr(void);

	/** 
	 * Keep consuming characters from the input file until we reach a
	 * character which isn't part of an identifier. The value of the identifier
	 * after this function returns is in m_CurrTokenStr.
	 */
	void ScanIdentifierToken(void);

	/**
	 * Like ScanIdentifierToken, only this time we're scanning a integral
	 * or floating point literal value. The literal value will be in 
	 * m_CurrTokenStr after this function returns.
	 */
	void ScanDigitToken(void);

	/**
	 * Almost identifical to ScanIdentifierToken only this time we're 
	 * scanning a string literal which is surrounded by double quotes. 
	 * The scanned string will be in m_CurrTokenStr after this function
	 * returns.
	 */
	void ScanStringToken(void);

	/**
	 * Determine the type of the token we have in m_CurrTokenStr.
	 * @return The exact type of the token just scanned.
	 */
	TokenType GetTokenType(void);

	/** Handle to the file we're reading tokens from. */
	ifstream m_FileHandle;

	/** Token maps for keywords. */
	TokenMap m_Keywords;

	/** Token map for misc tokens not covered by m_Keywords. */
	TokenMap m_MiscTokens;

	/**
	 * Buffer of characters read from the input file in one
	 * read operation to cut down on file IO and it's size.
	 */
	char *m_InputBuffer;

	/** The size of the input buffer being used. */
	const int m_InputBufferSize;

	/**
	 * We copy characters from the input buffer to this one whilst are 
	 * in the process of determining the current token.
	 */
	string m_CurrTokenStr;

	/**
	 * The forward pointer is the one we advance to scan for
	 * multiple characters tokens such as ">=" or "==" for example,
	 * the string of characters between the two pointers in the
	 * buffer is the token we're scanning.
	 */
	char *m_ForwardPtr;

	/** Current line number we're scanning. */
	long m_CurrLineNumber;

	/**
	 * If we've peeked at any token, they're stored here until
	 * we call GetNextToken again in which case we'll pop the top
	 * token and return that instead of reading from the file.
	 */
	list<Token_t> m_PeekedTokens;

	/** Are we peeking at a token when we call GetNextToken? */
	bool m_PeekingAtToken;
};

#endif
