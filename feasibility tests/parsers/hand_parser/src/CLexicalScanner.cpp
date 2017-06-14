//------------------------------------------------------------------------------------------
// File: CLexicalScanner.cpp
// Desc: Implementation of the hand written scanner
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

//our headers
#include "StdHeader.h"
#include "CLexicalScanner.h"

//-------------------------------------------------------------
CLexicalScanner::CLexicalScanner(void) : m_InputBufferSize(128)
{
	m_InputBuffer = NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CLexicalScanner::~CLexicalScanner(void)
{
	delete [] m_InputBuffer;
	m_InputBuffer = NULL;

	m_FileHandle.close();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::SetInputFile(const string &File)
{
	//try and open it
	m_FileHandle.open(File.c_str());

	if(!m_FileHandle.is_open())
		throw exception("unable to open input file for tokenising");

	//allocate the buffers and read in the first lot of characters
	m_InputBuffer = new char[m_InputBufferSize + 2];
	
	if(!m_InputBuffer)
		throw exception("unable to allocate token buffer");

	RefreshInputBuffer();
	m_CurrLineNumber = 1;

	//clear out the current token
	m_CurrTokenStr = "";
	m_PeekingAtToken = false;

	//build the reserved id's list
	InitialiseKeywordsList();
	EatWhitespaceAndComments();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::InitialiseKeywordsList(void)
{
	InitialiseTypeTokens();
	InitialiseControlStmtTokens();
	InitialiseFunctionTokens();
	InitialiseMiscTokens();
	
	//Boolean literals
	m_Keywords.insert(make_pair("true", TOKTYPE_TRUE));
	m_Keywords.insert(make_pair("false", TOKTYPE_FALSE));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::InitialiseTypeTokens(void)
{
	m_Keywords.insert(make_pair("integer", TOKTYPE_INTEGERTYPE));
	m_Keywords.insert(make_pair("float", TOKTYPE_FLOATTYPE));
	m_Keywords.insert(make_pair("boolean", TOKTYPE_BOOLEANTYPE));
	m_Keywords.insert(make_pair("string", TOKTYPE_STRINGTYPE));
	m_Keywords.insert(make_pair("const", TOKTYPE_CONST));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::InitialiseControlStmtTokens(void)
{
	m_Keywords.insert(make_pair("if", TOKTYPE_IF));
	m_Keywords.insert(make_pair("end_if", TOKTYPE_ENDIF));
	m_Keywords.insert(make_pair("else", TOKTYPE_ELSE));
	m_Keywords.insert(make_pair("while", TOKTYPE_WHILE));
	m_Keywords.insert(make_pair("end_while", TOKTYPE_END_WHILE));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::InitialiseFunctionTokens(void)
{
	m_Keywords.insert(make_pair("function", TOKTYPE_FUNCTION));
	m_Keywords.insert(make_pair("returns", TOKTYPE_RETURNS));
	m_Keywords.insert(make_pair("end_function", TOKTYPE_END_FUNCTION));
	m_Keywords.insert(make_pair("return", TOKTYPE_RETURN));
	m_Keywords.insert(make_pair("void", TOKTYPE_VOID));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::InitialiseMiscTokens(void)
{
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
	m_MiscTokens.insert(make_pair("[", TOKTYPE_OPENSQRPAREN));
	m_MiscTokens.insert(make_pair("]", TOKTYPE_CLOSESQRPAREN));
	m_MiscTokens.insert(make_pair(",", TOKTYPE_COMMA));
	m_MiscTokens.insert(make_pair("const", TOKTYPE_CONST));

	m_MiscTokens.insert(make_pair("-", TOKTYPE_MINUS));
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
	//have we got any peeked tokens?
	if(!m_PeekedTokens.empty())
	{
		Token_t Temp = *m_PeekedTokens.begin();
		m_PeekedTokens.pop_front();
		return Temp;
	}

	//take a look at the input token and see what we're pointing at,
	//we can then base a decision about which token to scan based upon that.
	//Each one the decisions below represents a starting state in the token
	//reconisition state transition diagrams.

	EatWhitespaceAndComments();

	//blank out the old token
	m_CurrTokenStr = "";

	//find out what we have next
	if(isalpha((int)*m_ForwardPtr) || *m_ForwardPtr == '_')
		ScanIdentifierToken();
	else if(isdigit((int)*m_ForwardPtr))
		ScanDigitToken();
	else if(*m_ForwardPtr == '\"')
		ScanStringToken();
	else
	{
		//for each of the four tokens in the condition, the next character can be '=' so 
		//handle them all in one go
		m_CurrTokenStr.append(1, IncForwardPtr());

		if(*m_ForwardPtr == '=' || *m_ForwardPtr == '>' || *m_ForwardPtr == '<' || *m_ForwardPtr == '!')
		{
			if(*m_ForwardPtr == '=')
				m_CurrTokenStr.append(1, IncForwardPtr());
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
	RetToken.TokenValue = m_CurrTokenStr;
	RetToken.Type = GetTokenType();
	return RetToken;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
Token_t CLexicalScanner::PeekNextToken(void)
{
	m_PeekingAtToken = true;
	Token_t Temp = GetNextToken();
	m_PeekedTokens.push_back(Temp);
	m_PeekingAtToken = false;
	return Temp;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::PushbackToken(Token_t Tok)
{
	m_PeekedTokens.push_front(Tok);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLexicalScanner::ClearPeekedTokens(void)
{
	m_PeekedTokens.clear();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CLexicalScanner::GetTokenType(void)
{
	if(IsKeyword(m_CurrTokenStr))
		return m_Keywords[m_CurrTokenStr];
	else
	{
		//see if it's one of the misc tokens
		TokenMap::iterator itr = m_MiscTokens.find(m_CurrTokenStr);

		if(itr == m_MiscTokens.end())
		{
			//not one of the misc tokens, must be an either a digit
			if(isdigit((int)m_CurrTokenStr[0]))
			{
				//if a decimal point is present, it an FP number
				string Temp(m_CurrTokenStr);
				if(Temp.find(".") != string::npos)
					return TOKTYPE_FLOATLITERAL;
				else
					return TOKTYPE_INTLITERAL;
			}
			else
			{
				//if the first character is a double quote we have a string, otherwise a nam
				if(m_CurrTokenStr[0] == '\"')
					return TOKTYPE_STRINGLITERAL;
				else
					return TOKTYPE_ID;
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
		m_CurrTokenStr.append(1, IncForwardPtr());
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
		m_CurrTokenStr.append(1, IncForwardPtr());

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
	m_CurrTokenStr.append(1, IncForwardPtr());

	//copy the body of the string
	while(*m_ForwardPtr != '\"')
		m_CurrTokenStr.append(1, IncForwardPtr());

	//get the closing quote
	m_CurrTokenStr.append(1, IncForwardPtr());
}
//-------------------------------------------------------------

//-------------------------------------------------------------	
void CLexicalScanner::ScanLogicalOr(void)
{
	if(*m_ForwardPtr == '|')
		m_CurrTokenStr.append(1, IncForwardPtr());
}
//-------------------------------------------------------------	

//-------------------------------------------------------------	
void CLexicalScanner::ScanLogicalAnd(void)
{
	if(*m_ForwardPtr == '&')
		m_CurrTokenStr.append(1, IncForwardPtr());
}
//-------------------------------------------------------------	

//-------------------------------------------------------------	
long CLexicalScanner::CurrentLineNumber(void)
{
	return m_CurrLineNumber;
}
//-------------------------------------------------------------	