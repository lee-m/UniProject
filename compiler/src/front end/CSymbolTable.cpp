//------------------------------------------------------------------------------------------
// File: CSymbolTable.cpp
// Desc: Class declararation for the hand written scanner.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CSymbolTable.h"

//-------------------------------------------------------------
CSymbolTable::CSymbolTable(void)
{
	m_CurrentScope = new Scope_t();
	m_CurrentScope->ParentScope = NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CSymbolTable::~CSymbolTable(void)
{
	delete m_CurrentScope;
	m_CurrentScope = NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CSymbolTable::PushNewSymbolScope(void)
{
	Scope_t *NewScope = new Scope_t();
	NewScope->ParentScope = m_CurrentScope;
	m_CurrentScope = NewScope;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CSymbolTable::PopSymbolScope(void)
{
	//create a copy of the current scope pointer so we
	//can delete it then move back up one level
	Scope_t *Temp = m_CurrentScope;
	m_CurrentScope = m_CurrentScope->ParentScope;

	//we should never be trying to pop the global scope
	SanityCheck(m_CurrentScope);

	delete Temp;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CSymbolTable::AddSymbol(const string &Name, CTreeNode *Ptr,
														 int LineNo, TokenType Type)
{
	Symbol_t NewSym;
	NewSym.IdPtr = Ptr;
	NewSym.LineNo = LineNo;
	NewSym.SymType = Type;
	NewSym.Name = Name;

	//insert the symbol into the current scope
	m_CurrentScope->Symbols.insert(make_pair(Name, NewSym));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
Symbol_t* CSymbolTable::LookupIdentifier(const string &Identifier, 
																				 bool LocalScopeOnly)
{
	//lookup the symbol in the current scope, if it's not there search
  //the parent scopes for it back up the global scope
	SymbolTableMap::iterator itr = m_CurrentScope->Symbols.find(Identifier);

	if(itr == m_CurrentScope->Symbols.end())
	{
		//see if we're allowed to search the parent scopes for the symbol
		if(LocalScopeOnly)
			return NULL;

		//create a copy of the current scope so we don't affect
		//where we are in the scope list
		Scope_t *TempScope = m_CurrentScope->ParentScope;

		while(TempScope)
		{
			//look for the symbol in this scope
			itr = TempScope->Symbols.find(Identifier);

			if(itr != TempScope->Symbols.end())
				return &itr->second;

			TempScope = TempScope->ParentScope;
		}
	}
	else
		return &itr->second;

	//not found in any scope
	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CSymbolTable::Reset(void)
{
	//destroy any nested scopes until we reach the global scope
	while(m_CurrentScope->ParentScope)
		PopSymbolScope();

	//clear out any global symbols
	m_CurrentScope->Symbols.clear();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CSymbolTable::AtGlobalScope(void)
{
	return m_CurrentScope->ParentScope == NULL;
}
//-------------------------------------------------------------

//singleton static member definition
template<> CSymbolTable* ISingleton<CSymbolTable>::ms_Singleton = 0;
