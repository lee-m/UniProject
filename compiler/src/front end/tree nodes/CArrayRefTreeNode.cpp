//------------------------------------------------------------------------------------------
// File: CArrayRefTreeNode.cpp
// Desc: Represents an access to an array element access
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CArrayRefTreeNode.h"
#include "CSymbolTable.h"
#include "CIdentifierTreeNode.h"
#include "CAssignExprTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CSymbolTable.h"

//-------------------------------------------------------------
void CArrayRefTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	TiXmlElement *ArrRefNode;
	ArrRefNode = XmlNode->InsertEndChild(TiXmlElement("array_ref"))->ToElement();

	TiXmlElement *IndexNode;
	IndexNode = ArrRefNode->InsertEndChild(TiXmlElement("index"))->ToElement();

	//dump the expression used for the index
	GetChild(ARR_REF_INDEX)->DumpAsParseTreeNode(IndexNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CArrayRefTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	OutFile << ArrName->IdName << "[";
	GetChild(ARR_REF_INDEX)->DumpAsMIRTreeNode(OutFile);
	OutFile << "]";
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CArrayRefTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//handle the simple case where we're using an identifier or literal
	//to access an element of the array
	CTreeNode *IndexExpr = GetChild(ARR_REF_INDEX);

	if(IndexExpr->Code == TC_LITERAL
			|| IndexExpr->Code == TC_IDENTIFIER)
	{
		CTreeNode *Copy = CreateCopy();
		CSymbolTable::GetSingletonPtr()->AddSymbol(ArrName->IdName, Copy, 0, ArrType);
		return Copy;
	}

	//saves on typing
	CTreeNodeFactory *TreeNodeFactory = CTreeNodeFactory::GetSingletonPtr();

	//decompose nested array access of the form a[b[2]]
	if(IndexExpr->Code == TC_ARRAY_REF)
	{
		//simplify the inner array access
		CTreeNode *NewIndex = IndexExpr->RewriteIntoMIRForm(Stmts);

		//create a new temp variable to hold the result of the inner array access
		string TempVarName = CUtilFuncs::GenCompilerTempName();
		CVarDeclTreeNode *TempVar = (CVarDeclTreeNode*)TreeNodeFactory->BuildVarDecl(
			TempVarName, false, TOKTYPE_INTEGERTYPE);

		//initialise the temp var with the inner array access
		CAssignExprTreeNode *TempVarAssign;
		TempVarAssign = (CAssignExprTreeNode*)TreeNodeFactory->BuildAssignmentExpr(
			TempVar->VarId->CreateCopy(), NewIndex);

		//add the new var decl to the symbol table
		CSymbolTable::GetSingletonPtr()->AddSymbol(TempVarName, TempVar, 0, 
			TOKTYPE_INTEGERTYPE);

		//add the intermediate stmts
		Stmts->AddChild(TempVar);
		Stmts->AddChild(TempVarAssign);
		return TreeNodeFactory->BuildArrayRef(TempVar->VarId->CreateCopy(), 
			ArrName->IdName, ArrType);
	}

	if(IndexExpr->Code == TC_ARITH_EXPR)
	{
		//create a temp var initialised to the result of the arith expr
		string TempVarName = CUtilFuncs::GenCompilerTempName();
		CVarDeclTreeNode *TempVar = (CVarDeclTreeNode*)TreeNodeFactory->BuildVarDecl(
			TempVarName, false, TOKTYPE_INTEGERTYPE);

		//simplify the index expression
		CTreeNode *NewIndex = IndexExpr->RewriteIntoMIRForm(Stmts);

		//initialise the temp var with the expr result
		CAssignExprTreeNode *TempVarAssign;
		TempVarAssign = (CAssignExprTreeNode*)TreeNodeFactory->BuildAssignmentExpr(
			TempVar->VarId->CreateCopy(), NewIndex);

		//add the new var decl to the symbol table
		CSymbolTable::GetSingletonPtr()->AddSymbol(TempVarName, TempVar, 0, 
			TOKTYPE_INTEGERTYPE);

		//add the intermediate stmts
		Stmts->AddChild(TempVar);
		Stmts->AddChild(TempVarAssign);

		return TreeNodeFactory->BuildArrayRef(TempVar->VarId->CreateCopy(), 
			ArrName->IdName, ArrType);
	}

	InternalError("unexpected array ref index expr type");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CArrayRefTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildArrayRef(
						GetChild(ARR_REF_INDEX)->CreateCopy(), ArrName->IdName, ArrType);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CArrayRefTreeNode::DestroyNode(void)
{
	ArrName->DestroyNode();
	GetChild(ARR_REF_INDEX)->DestroyNode();
	ArrName = NULL;
	delete this;
}
//-------------------------------------------------------------
