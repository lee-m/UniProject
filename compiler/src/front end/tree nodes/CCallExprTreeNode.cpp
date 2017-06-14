//------------------------------------------------------------------------------------------
// File: CCallExprTreeNode.cpp
// Desc: Representation of a WHILE statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CCallExprTreeNode.h"
#include "CSymbolTable.h"
#include "CFunctionDeclTreeNode.h"
#include "CParameterListTreeNode.h"
#include "CAssignExprTreeNode.h"
#include "CVarDeclTreeNode.h"

//-------------------------------------------------------------
void CCallExprTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	TiXmlElement *CallExprNode;
	CallExprNode = XmlNode->InsertEndChild(TiXmlElement("call_expr"))->ToElement();

	//add the parameters
	TiXmlElement *ParmNode = NULL;

	for(unsigned int i = 0; i < GetNumChildren(); i++)
	{
		ParmNode = CallExprNode->InsertEndChild(TiXmlElement("call_expr_parm"))->ToElement();
		GetChild(i)->DumpAsParseTreeNode(ParmNode);
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCallExprTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	OutFile << FuncName << "(";

	for(unsigned int i = 0; i < GetNumChildren(); i++)
	{
		GetChild(i)->DumpAsMIRTreeNode(OutFile);

		if(i != GetNumChildren() - 1)
			OutFile << ", ";
	}

	OutFile << ")";
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CCallExprTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	CTreeNodeFactory *TreeNodeFactory = CTreeNodeFactory::GetSingletonPtr();
	CTreeNode *NewFuncCall = TreeNodeFactory->BuildCallExpr("_" + FuncName);

	//get the parameter list of the function being called	
	CFunctionDeclTreeNode *FnDecl;
	CParameterListTreeNode *ParmList;

	FnDecl = (CFunctionDeclTreeNode*)CUtilFuncs::GetDeclFromNode(NewFuncCall);
	ParmList = (CParameterListTreeNode*)FnDecl->GetChild(FUNC_DECL_PARM_LIST);

	//simplify the parameters passed for the call
	for(unsigned int i = 0; i < GetNumChildren(); i++)
	{
		CTreeNode *NewParm = GetChild(i)->RewriteIntoMIRForm(Stmts);
		NewParm = ReplaceArrayRefs(ReplaceCallExpr(NewParm, Stmts), Stmts);

		//see if we're passing a floating point expression to an
		//integer type parameter
		if(CUtilFuncs::ExprRefersToFloatingPointType(NewParm)
			&& !CUtilFuncs::ExprRefersToFloatingPointType(ParmList->GetChild(i)))
		{
			CTreeNodeFactory *TreeNodeFactory = CTreeNodeFactory::GetSingletonPtr();

			//create a new temporary integer type variable which is initialised 
			//to the floating point expression, this will ensure the necessary
			//truncation will take place before passing it as a parameter
			string TempVarName = CUtilFuncs::GenCompilerTempName();
			CVarDeclTreeNode *TempVar;
			TempVar = (CVarDeclTreeNode*)TreeNodeFactory->BuildVarDecl(TempVarName, false, 
				TOKTYPE_INTEGERTYPE);

			CAssignExprTreeNode *TempVarAssign;
			TempVarAssign = (CAssignExprTreeNode*)TreeNodeFactory->BuildAssignmentExpr(
				TempVar->VarId->CreateCopy(), NewParm);

			//add the new statements
			Stmts->AddChild(TempVar);
			Stmts->AddChild(TempVarAssign);

			//add the new var to the symbol table
			CSymbolTable::GetSingletonPtr()->AddSymbol(TempVarName, TempVar, 0, 
				TOKTYPE_INTEGERTYPE);

			//replace the original parameter with a reference to the temp var
			NewFuncCall->AddChild(TempVar->VarId->CreateCopy());
		}
		else
			NewFuncCall->AddChild(NewParm);
	}

	return NewFuncCall;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CCallExprTreeNode::CreateCopy(void)
{
	//create a new call_expr node to hold the arguments
	CTreeNode *Copy;
	Copy = CTreeNodeFactory::GetSingletonPtr()->BuildCallExpr(FuncName);

	//add each parameter
	for(unsigned int i = 0; i < GetNumChildren(); i++)
		Copy->AddChild(GetChild(i)->CreateCopy());

	return Copy;
}
//-------------------------------------------------------------
