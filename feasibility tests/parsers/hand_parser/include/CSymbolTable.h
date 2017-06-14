//------------------------------------------------------------------------------------------
// File: CSymbolTable.h
// Desc: Maintains a list of symbols which are accessable from the current
//       point in the program.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CSYMBOLTABLE_H__
#define __CSYMBOLTABLE_H__

#include "TokenTypes.h"
#include "CParseTreeNode.h"

//information about a single symbol in the table
struct Symbol_t
{
	int LineNo;
	string Name;
	CParseTreeNode *IdPtr;
};

//make it easier to change the symbol table data structure
typedef map<string, Symbol_t> SymbolTableMap;

//info about the current scope we're in
struct Scope_t
{
	Scope_t *ParentScope;
	SymbolTableMap Symbols;
};

class CSymbolTable
{
public:
	CSymbolTable(void);
	~CSymbolTable(void);

	//open a new scope into which we'll be inserting symbols
	void PushNewSymbolScope(void);

	//move back up one level in the scope list, destroying
	//the current scope in the process
	void PopSymbolScope(void);

	//insert the identifier into the current scope, it's the caller's 
	//responsibility to make sure no conflicts arise from this symbol
	//before insertion
	void AddSymbol(const string &Name, CParseTreeNode *Ptr, int LineNo);

	//returns true if the declaration for the identifier is
	//visible within the current scope
	Symbol_t* LookupIdentifier(const string &Identifier);

private:

	//current scope we're adding symbols to (and any enclosing scopes)
	Scope_t *m_CurrentScope;
};

#endif