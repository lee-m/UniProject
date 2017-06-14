//------------------------------------------------------------------------------------------
// File: CLexicalScanner.cpp
// Desc: Implementation of the hand written scanner
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

//our headers
#include "CLexicalScanner.h"

//std headers
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cassert>

//-------------------------------------------------------------
CLexicalScanner::CLexicalScanner(const string &InputFile) : m_InputBufferSize(4)
{
	//try and open it
	m_FileHandle.open(InputFile.c_str());

	if(!m_FileHandle.is_open())
		throw exception("unable to open input file for tokenising");

	//allocate the buffers and read in the first lot of characters
	m_InputBuffer = new char[m_InputBufferSize + 2];
	assert(m_InputBuffer);

	RefreshInputBuffer();
	m_CurrLineNumber = 1;

	//clear out the current token
	m_CurrToken = "";

	//build the reserved id's list
	InitialiseKeywordsList();
	EatWhitespaceAndComments();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CLexicalScanner::~CLexicalScanner(void)
{
	delete m_InputBuffer;
	m_InputBuffer = NULL;

	m_FileHandle.close();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::InitialiseKeywordsList(void)
{
	//program keywords
	m_Keywords.insert(make_pair("program", TOKTYPE_PROGRAM));
	m_Keywords.insert(make_pair("end_program", TOKTYPE_ENDPROGRAM));

	//types
	m_Keywords.insert(make_pair("integer", TOKTYPE_INTEGERTYPE));
	m_Keywords.insert(make_pair("float", TOKTYPE_FLOATTYPE));
	m_Keywords.insert(make_pair("boolean", TOKTYPE_BOOLEANTYPE));
	m_Keywords.insert(make_pair("string", TOKTYPE_STRINGTYPE));

	//control statements
	m_Keywords.insert(make_pair("if", TOKTYPE_IF));
	m_Keywords.insert(make_pair("end_if", TOKTYPE_ENDIF));
	m_Keywords.insert(make_pair("while", TOKTYPE_WHILE));
	m_Keywords.insert(make_pair("end_while", TOKTYPE_ENDWHILE));

	//functions/procedures
	m_Keywords.insert(make_pair("function", TOKTYPE_FUNCTION));
	m_Keywords.insert(make_pair("returns", TOKTYPE_RETURNS));
	m_Keywords.insert(make_pair("return", TOKTYPE_RETURN));
	m_Keywords.insert(make_pair("end_function", TOKTYPE_ENDFUNC));
	
	//Boolean types
	m_Keywords.insert(make_pair("true", TOKTYPE_TRUE));
	m_Keywords.insert(make_pair("false", TOKTYPE_FALSE));

	//misc tokens 
	m_MiscTokens.insert(make_pair("=", TOKTYPE_ASSIGNMENT));
	m_MiscTokens.insert(make_pair("(", TOKTYPE_OPENPAREN));
	m_MiscTokens.insert(make_pair(")", TOKTYPE_CLOSEPAREN));
	m_MiscTokens.insert(make_pair(";", TOKTYPE_SEMICOLON));
	m_MiscTokens.insert(make_pair("<", TOKTYPE_LESSTHAN));
	m_MiscTokens.insert(make_pair("<=", TOKTYPE_LESSTHANEQ));
	m_MiscTokens.insert(make_pair(">", TOKTYPE_GREATTHAN));
	m_MiscTokens.insert(make_pair(">=", TOKTYPE_GREATTHANEQ));
	m_MiscTokens.insert(make_pair("!", TOKTYPE_NOT));
	m_MiscTokens.insert(make_pair("!=", TOKTYPE_NOTEQ));
	m_MiscTokens.insert(make_pair("==", TOKTYPE_EQUALITY));
	m_MiscTokens.insert(make_pair("||", TOKTYPE_OR));
	m_MiscTokens.insert(make_pair("&&", TOKTYPE_AND));
	m_MiscTokens.insert(make_pair("const", TOKTYPE_CONST));
	m_MiscTokens.insert(make_pair("]", TOKTYPE_OPENSQRPAREN));
	m_MiscTokens.insert(make_pair("[", TOKTYPE_CLOSESQRPAREN));
	m_MiscTokens.insert(make_pair(",", TOKTYPE_COMMA));

	//arithmetic operators
	m_MiscTokens.insert(make_pair("-", TOKTYPE_SUBTRACT));
	m_MiscTokens.insert(make_pair("+", TOKTYPE_PLUS));
	m_MiscTokens.insert(make_pair("/", TOKTYPE_DIVIDE));
	m_MiscTokens.insert(make_pair("*", TOKTYPE_MULTIPLY));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::EatWhitespaceAndComments(void)
{
	bool EndOfWs = false;

	while(!EndOfWs)
	{
		//check for a comment
		if(*m_ForwardPtr == '/')
		{
			IncForwardPtr();

			if(*m_ForwardPtr == '*')
			{
				//we have a comment, keep going until we hit the next non-whitespace character
				while(true)
				{
					++m_ForwardPtr;

					if(*m_ForwardPtr == '\n')
						++m_CurrLineNumber;

					if(*m_ForwardPtr == 0)
						RefreshInputBuffer();

					if(*m_ForwardPtr == '*')
					{
						IncForwardPtr();

						if(*m_ForwardPtr == '/')
						{
							//found the end of comment, move past the 
							//comment terminator character
							IncForwardPtr();
							break;
						}
					}
				}
			}
			else
			{
				--m_ForwardPtr; //we dont have the start of a comment but the division op
				EndOfWs = true;
			}
		}

		//check for whitespace
		else if(*m_ForwardPtr == ' ' || *m_ForwardPtr == '\t' || *m_ForwardPtr == '\n')
		{
			//skip it
			do
			{
				if(*m_ForwardPtr == '\n')
					++m_CurrLineNumber;

				IncForwardPtr();
			}
			while(*m_ForwardPtr == ' ' || *m_ForwardPtr == '\t' || *m_ForwardPtr == '\n');
		}
		else
			EndOfWs = true;
	}

	if(*m_ForwardPtr == 0)
		RefreshInputBuffer();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CLexicalScanner::IsKeyword(const string &TokStr)
{
	TokenMap::iterator itr = m_Keywords.find(TokStr);
	return !(itr == m_Keywords.end());
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::RefreshInputBuffer(void)
{
	//if we're not at the end of the file, read the next lot in
	if(!m_FileHandle.eof())
	{
		memset((void*)m_InputBuffer, 0, m_InputBufferSize + 1);
		m_FileHandle.read(m_InputBuffer, m_InputBufferSize);
		m_InputBuffer[m_InputBufferSize + 1] = '\0';
		m_ForwardPtr = &m_InputBuffer[0];
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
char CLexicalScanner::IncForwardPtr(void)
{
	char c = *m_ForwardPtr++;

	if(*m_ForwardPtr == 0)
		RefreshInputBuffer();

	return c;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CLexicalScanner::HasMoreTokens(void)
{
	if(m_FileHandle.eof())
	{
		if(*m_ForwardPtr == 0)
			return false;
	}
	
	//in some situations we might not be at the end of the file but 
	//have no more tokens left if there's a lot of trailing whitespace
	//or a large comment block so skip over that here
	EatWhitespaceAndComments();

	if(m_FileHandle.eof() && *m_ForwardPtr == 0)
		return false;
	else	
		return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
Token_t CLexicalScanner::GetNextToken(void)
{
	//take a look at the input token and see what we're pointing at,
	//we can then base a decision about which token to scan based upon that.
	//Each one the decisions below represents a starting state in the token
	//reconisition state transition diagrams.

	EatWhitespaceAndComments();

	//blank out the old token
	m_CurrToken = "";

	//find out what we have next
	if(isalpha((int)*m_ForwardPtr) || *m_ForwardPtr == '_')
		ScanIdentifierToken();
	else if(isdigit((int)*m_ForwardPtr))
		ScanDigitToken();
	else if(*m_ForwardPtr == '\"')
		ScanStringToken();
	else
	{
		//for each of the four tokens in the condition, the next character can be '=' so handle them
		//all in one go
		m_CurrToken.append(1, IncForwardPtr());

		if(*m_ForwardPtr == '=' || *m_ForwardPtr == '>' || *m_ForwardPtr == '<' || *m_ForwardPtr == '!')
		{
			if(*m_ForwardPtr == '=')
				m_CurrToken.append(1, IncForwardPtr());
		}
		else if(*m_ForwardPtr == '|')
			ScanLogicalOr();
		else if(*m_ForwardPtr == '&')
			ScanLogicalAnd();
	}

	//the m_CurrToken buffer is already filled with zero's so the 
	//name of the token will always be NULL terminated.
	Token_t RetToken;
	RetToken.LineNumber = m_CurrLineNumber;
	RetToken.TokenValue = m_CurrToken;
	RetToken.Type = GetTokenType();
	return RetToken;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CLexicalScanner::GetTokenType(void)
{
	if(IsKeyword(m_CurrToken))
		return m_Keywords[m_CurrToken];
	else
	{
		//see if it's one of the misc tokens
		TokenMap::iterator itr = m_MiscTokens.find(m_CurrToken);

		if(itr == m_MiscTokens.end())
		{
			//not one of the misc tokens, must be an either a digit
			if(isdigit((int)m_CurrToken[0]))
			{
				//if a decimal point is present, it an FP number
				string Temp(m_CurrToken);
				if(Temp.find(".") != string::npos)
					return TOKTYPE_FLOATDIGIT;
				else
					return TOKTYPE_INTDIGIT;
			}
			else
			{
				//if the first character is a double quote we have a string, otherwise a nam
				if(m_CurrToken[0] == '\"')
					return TOKTYPE_STRING;
				else
					return TOKTYPE_NAME;
			}
		}
		else
			return itr->second;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::ScanIdentifierToken(void)
{
	while(isalpha((int)*m_ForwardPtr) || isdigit((int)*m_ForwardPtr) || *m_ForwardPtr == '_')
	{
		//copy the character across to the token buffer
		m_CurrToken.append(1, IncForwardPtr());
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::ScanDigitToken(void)
{
	bool FoundDecimalPoint = false;

	while(isdigit((int)*m_ForwardPtr) || *m_ForwardPtr == '.')
	{
		//copy the character across to the token buffer
		m_CurrToken.append(1, IncForwardPtr());

		if(*m_ForwardPtr == '.' && FoundDecimalPoint)
			break;
		else
			FoundDecimalPoint = true;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::ScanStringToken(void)
{
	//copy the opening quote
	m_CurrToken.append(1, IncForwardPtr());

	//copy the body of the string
	while(*m_ForwardPtr != '\"')
		m_CurrToken.append(1, IncForwardPtr());

	//get the closing quote
	m_CurrToken.append(1, IncForwardPtr());
}
//-------------------------------------------------------------

//-------------------------------------------------------------	
void CLexicalScanner::ScanLogicalOr(void)
{
	if(*m_ForwardPtr == '|')
		m_CurrToken.append(1, IncForwardPtr());
}
//-------------------------------------------------------------	

//-------------------------------------------------------------	
void CLexicalScanner::ScanLogicalAnd(void)
{
	if(*m_ForwardPtr == '&')
		m_CurrToken.append(1, IncForwardPtr());
}
//-------------------------------------------------------------	