//------------------------------------------------------------------------------------------
// File: CTreeNode.cpp
// Desc: Encapsulation of tree node used as the IR by the compiler
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CTreeNode.h"
#include "CSymbolTable.h"
#include "CCallExprTreeNode.h"
#include "CFunctionDeclTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CArrayDeclTreeNode.h"
#include "CArrayRefTreeNode.h"

//-------------------------------------------------------------
void CTreeNode::DestroyNode(void)
{
	vector<CTreeNode*>::iterator itr = Children.begin();

	for(; itr != Children.end(); itr++)
	{
		if(*itr)
		{
			(*itr)->DestroyNode();
			(*itr) = NULL;
		}
	}

	delete this;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CTreeNode::AddChild(CTreeNode *NewChild, unsigned int Pos)
{
	SanityCheck(NewChild);

	if(Pos == -1)
		Children.push_back(NewChild);
	else
		Children.insert(Children.begin() + Pos, NewChild);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
unsigned int CTreeNode::GetNumChildren(void)
{
	return (unsigned int)Children.size();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNode::GetChild(unsigned int Id)
{
	SanityCheck(Id < Children.size());
	return Children[Id];
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode** CTreeNode::GetChildPtr(unsigned int Id)
{
	SanityCheck(Id < Children.size());
	return &Children[Id];
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNode::ReplaceCallExpr(CTreeNode *Node, CTreeNode *Stmts)
{
	if(Node->Code == TC_CALLEXPR)
	{
		CTreeNodeFactory *TreeFactory = CTreeNodeFactory::GetSingletonPtr();
		CSymbolTable *SymTable = CSymbolTable::GetSingletonPtr();

		//get the return type of the function so we create a temp var with
		//the correct type
		CCallExprTreeNode *CallExpr = (CCallExprTreeNode*)Node;
		Symbol_t *FuncSym = SymTable->LookupIdentifier(CallExpr->FuncName, false);
		SanityCheck(FuncSym);

		CFunctionDeclTreeNode *FnDecl = (CFunctionDeclTreeNode*)FuncSym->IdPtr;
		CVarDeclTreeNode *TempVar = (CVarDeclTreeNode*)TreeFactory->BuildVarDecl(
			CUtilFuncs::GenCompilerTempName(), false, FnDecl->RetType);

		//assign the value computed from the simplification of LHS of this expression
		//to this temporary variable, adding it to the MIR tree
		CTreeNode *AssignExpr = TreeFactory->BuildAssignmentExpr(
			TempVar->VarId->CreateCopy(), Node);

		Stmts->AddChild(TempVar);
		Stmts->AddChild(AssignExpr);

		CSymbolTable::GetSingletonPtr()->AddSymbol(TempVar->VarId->IdName,
			TempVar, 0, TempVar->VarType);
		
		return TempVar->VarId->CreateCopy();
	}

	//nothing to do
	return Node;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNode::ReplaceArrayRefs(CTreeNode *Node, CTreeNode *Stmts)
{
	if(Node->Code == TC_ARRAY_REF)
	{
		CTreeNodeFactory *TreeFactory = CTreeNodeFactory::GetSingletonPtr();
		CSymbolTable *SymTable = CSymbolTable::GetSingletonPtr();

		//get the return type of the function so we create a temp var with
		//the correct type
		CArrayRefTreeNode *ArrRef = (CArrayRefTreeNode*)Node;
		Symbol_t *Sym = SymTable->LookupIdentifier(ArrRef->ArrName->IdName, false);
		SanityCheck(Sym);

		CArrayDeclTreeNode *ArrDecl = (CArrayDeclTreeNode*)Sym->IdPtr;
		CVarDeclTreeNode *ArrVarDecl = (CVarDeclTreeNode*)ArrDecl->GetChild(ARR_DECL_VAR);

		CVarDeclTreeNode *TempVar = (CVarDeclTreeNode*)TreeFactory->BuildVarDecl(
			CUtilFuncs::GenCompilerTempName(), false, ArrVarDecl->VarType);

		//assign the value computed from the simplification of LHS of this expression
		//to this temporary variable, adding it to the MIR tree
		CTreeNode *AssignExpr = TreeFactory->BuildAssignmentExpr(
			TempVar->VarId->CreateCopy(), Node);

		Stmts->AddChild(TempVar);
		Stmts->AddChild(AssignExpr);

		CSymbolTable::GetSingletonPtr()->AddSymbol(TempVar->VarId->IdName,
			TempVar, 0, TempVar->VarType);
		
		return TempVar->VarId->CreateCopy();
	}

	//nothing to do
	return Node;
}
//-------------------------------------------------------------
