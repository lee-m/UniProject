//------------------------------------------------------------------------------------------
// File: CSymbolTable.h
// Desc: Maintains a list of symbols which are accessable from the current
//       point in the program.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CSYMBOLTABLE_H__
#define __CSYMBOLTABLE_H__

#include "TokenTypes.h"
#include "CTreeNode.h"
#include "ISingleton.h"

/** Information about a single symbol in the table. */
struct Symbol_t
{
	/** Line number it appears on in the source code. */
	int LineNo;

	/** The name as it appears in the code. */
	string Name;

	/** Pointer to the tree node representing the decl being declared. */
	CTreeNode *IdPtr;

	/** The type of the decl pointed to by IdPtr */
	TokenType SymType;
};

//make it easier to change the symbol table data structure
typedef map<string, Symbol_t> SymbolTableMap;

/** Info about a single scope. */
struct Scope_t
{
	/** Pointer to the previous scope. */
	Scope_t *ParentScope;

	/** All the symbols declared within this scope. */
	SymbolTableMap Symbols;
};

/**
 * Store information about each defined symbol in the program with
 * efficient methods for looking them up by name. Each variable which
 * is declared is stored within a particular scope with only variables
 * declared inside the current scope, or any directly accessible from it
 * being visible. Once a scope exits, all symbols within it are lost. 
 * Internally the scopes are represented as a singly linked list with pointers
 * to the previous scope stored at each point with the exception of the 
 * global scope which will always have a NULL parent. 
 * <br><br>
 * After the initial parsing phase, the contents of the symbol table are
 * discarded and rebuilt during the subsequent MIR lowering. This is to take
 * into account any new temporaries created during this pass.
 */
class CSymbolTable : public ISingleton<CSymbolTable>
{
public:
	CSymbolTable(void);
	~CSymbolTable(void);

	/** Open a new scope into which we'll be inserting symbols. */
	void PushNewSymbolScope(void);

	/**
	 * Move back up one level in the scope list, destroying
	 * the current scope in the process. Any attempt to pop the
	 * global scope will trigger an ICE.
	 */
	void PopSymbolScope(void);

	/**
	 * Insert the identifier into the current scope, it's the caller's
	 * responsibility to make sure no conflicts arise from this symbol
	 * before insertion.
	 * @param Name The name of the symbol being inserted.
	 * @param Ptr Pointer to the tree node constructed for this symbol.
	 * @param LineNo Line number the symbol was declared on.
	 * @param Type The declared type of the symbol being added.
	 */
	void AddSymbol(const string &Name, CTreeNode *Ptr, int LineNo, TokenType Type);

	/**
	 * Check if the declaration for the identifier is
	 * visible from the current scope. If LocalScopeOnly is set to
	 * true then only symbols in the current scope are checked, any
	 * parent scopes are ignored.
	 * @param Identifier The name of the symbol to look up.
	 * @param LocalScopeOnly Only look for the identifier in the current scope.
	 * @return A pointer to the symbol object for the identifier if found, 
	 *         otherwise NULL.
	 */
	Symbol_t* LookupIdentifier(const string &Identifier, bool LocalScopeOnly);

	/**
	 * Reset the table back to it's original state, destroying any
	 * scopes we've got.
	 */
	void Reset(void);

	/**
	 * See if we're at global scope.
	 * @return True if we're at global scope, otherwise false.
	 */
	bool AtGlobalScope(void);

private:

	/** Current scope we're adding symbols to (and any enclosing scopes). */
	Scope_t *m_CurrentScope;
};

#endif
