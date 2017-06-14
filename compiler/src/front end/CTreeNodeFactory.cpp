//------------------------------------------------------------------------------------------
// File: CTreeNodeFactory.cpp
// Desc: Construct tree nodes of various types
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CIdentifierTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CArrayDeclTreeNode.h"
#include "CLIteralTreeNode.h"
#include "CArithExprTreeNode.h"
#include "CBooleanExprTreeNode.h"
#include "CUnaryExprTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CIfStmtTreeNode.h"
#include "CWhileStmtTreeNode.h"
#include "CReturnStmtTreeNode.h"
#include "CAssignExprTreeNode.h"
#include "CParameterListTreeNode.h"
#include "CFunctionDeclTreeNode.h"
#include "CParameterDeclTreeNode.h"
#include "CCallExprTreeNode.h"
#include "CRootTreeNode.h"
#include "CLabelTreeNode.h"
#include "CGotoExprTreeNode.h"
#include "CLiteralRefTreeNode.h"


//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildIdentifier(const string &Name)
{
	CIdentifierTreeNode *NewIdNode = new CIdentifierTreeNode();

	NewIdNode->Code = TC_IDENTIFIER;
	NewIdNode->IdName = Name;

	return NewIdNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildVarDecl(const string &Name,
																					bool IsConst, TokenType VarType)
{
	CVarDeclTreeNode *VarDeclNode = new CVarDeclTreeNode();

	VarDeclNode->Code = TC_VARDECL;
	VarDeclNode->VarId = (CIdentifierTreeNode*)BuildIdentifier(Name);
	VarDeclNode->IsConst = IsConst;
	VarDeclNode->VarType = VarType;

	return VarDeclNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildArrayDeclaration(CTreeNode *VarDecl,
																									 CTreeNode *Bounds)
{
	SanityCheck(VarDecl && Bounds);
	CArrayDeclTreeNode *NewArrayDecl = new CArrayDeclTreeNode();

	NewArrayDecl->Code = TC_ARRAYDECL;
	NewArrayDecl->AddChild(VarDecl);
	NewArrayDecl->AddChild(Bounds);

	return NewArrayDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildLiteral(const string &Value, TokenType LitType)
{
	CLiteralTreeNode *NewLiteralNode = new CLiteralTreeNode();

	NewLiteralNode->Code = TC_LITERAL;
	NewLiteralNode->LitValue = Value;
	NewLiteralNode->LitType = LitType;

	return NewLiteralNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildArithExpression(CTreeNode *Lhs, CTreeNode *Rhs,
																									TokenType Op)
{
	SanityCheck(Lhs && Rhs);
	CArithExprTreeNode *NewArithExp = new CArithExprTreeNode();

	NewArithExp->Code = TC_ARITH_EXPR;
	NewArithExp->Operator = Op;
	NewArithExp->AddChild(Lhs);
	NewArithExp->AddChild(Rhs);

	return NewArithExp;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildBooleanExpression(CTreeNode *Lhs,
																										CTreeNode *Rhs, TokenType Op)
{
	SanityCheck(Lhs && Rhs);
	CBooleanExprTreeNode *NewBlnExp = new CBooleanExprTreeNode();

	NewBlnExp->Code = TC_BOOLEAN_EXPR;
	NewBlnExp->Operator = Op;
	NewBlnExp->AddChild(Lhs);
	NewBlnExp->AddChild(Rhs);

	return NewBlnExp;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildArrayRef(CTreeNode *Index,
																					const string &ArrName, TokenType Type)
{
	SanityCheck(Index);
	CArrayRefTreeNode *NewArrRef = new CArrayRefTreeNode();

	NewArrRef->Code = TC_ARRAY_REF;
	NewArrRef->ArrName = (CIdentifierTreeNode*)BuildIdentifier(ArrName);
	NewArrRef->ArrType = Type;
	NewArrRef->AddChild(Index);

	return NewArrRef;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildIfStatement(CTreeNode *CondExpr)
{
	SanityCheck(CondExpr);
	CIfStmtTreeNode *NewIfStmt = new CIfStmtTreeNode();

	NewIfStmt->Code = TC_IFSTMT;
	NewIfStmt->AddChild(CondExpr);

	//add two sub-tree nodes for the 'if' and 'else' bodies
	NewIfStmt->AddChild(BuildRootNode());
	NewIfStmt->AddChild(BuildRootNode());

	return NewIfStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildWhileStatement(CTreeNode *CondExpr)
{
	SanityCheck(CondExpr);
	CWhileStmtTreeNode *NewWhileStmt = new CWhileStmtTreeNode();

	NewWhileStmt->Code = TC_WHILESTMT;
	NewWhileStmt->AddChild(CondExpr);

	return NewWhileStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildAssignmentExpr(CTreeNode *Lhs,
																								CTreeNode *Rhs)
{
	SanityCheck(Lhs && Rhs);
	CAssignExprTreeNode *NewAssignExpr = new CAssignExprTreeNode();

	NewAssignExpr->Code = TC_ASSIGN_EXPR;
	NewAssignExpr->AddChild(Lhs);
	NewAssignExpr->AddChild(Rhs);

	return NewAssignExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildParameterList(void)
{
	CParameterListTreeNode *NewParmList = new CParameterListTreeNode();

	NewParmList->Code = TC_PARAMETER_LIST;
	return NewParmList;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildFunctionDecl(const string &Name, int NumParms,
																							 CTreeNode *ParmList,
																							 TokenType RetType)
{
	SanityCheck(ParmList);
	CFunctionDeclTreeNode *NewFnDecl = new CFunctionDeclTreeNode();

	NewFnDecl->Code = TC_FUNCTIONDECL;
	NewFnDecl->Name = Name;
	NewFnDecl->NumParameters = NumParms;
	NewFnDecl->RetType = RetType;
	NewFnDecl->AddChild(ParmList);

	return NewFnDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildParameterDecl(const string &Name, TokenType Type,
                                                bool IsConst, int Position)
{
	CParameterDeclTreeNode *NewParmDecl = new CParameterDeclTreeNode();

	NewParmDecl->Code = TC_PARMDECL;
	NewParmDecl->AddChild(BuildIdentifier(Name));
	NewParmDecl->Type = Type;
	NewParmDecl->IsConst = IsConst;
	NewParmDecl->Position = Position;

	return NewParmDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildReturnStmt(CTreeNode *Expr)
{
	SanityCheck(Expr);
	CReturnStmtTreeNode *NewRetStmt = new CReturnStmtTreeNode();

	NewRetStmt->Code = TC_RETURNSTMT;
	NewRetStmt->AddChild(Expr);

	return NewRetStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildCallExpr(const string &FuncName)
{
	CCallExprTreeNode *NewCallExpr = new CCallExprTreeNode();

	NewCallExpr->Code = TC_CALLEXPR;
	NewCallExpr->FuncName = FuncName;

	return NewCallExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildUnaryExpression(CTreeNode *Expr, TokenType Op)
{
	SanityCheck(Expr);
	CUnaryExprTreeNode *UnExpr = new CUnaryExprTreeNode();

	UnExpr->Code = TC_UNARY_EXPR;
	UnExpr->Operator = Op;
	UnExpr->AddChild(Expr);

	return UnExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildRootNode(void)
{
	CRootTreeNode *NewRoot = new CRootTreeNode();

	NewRoot->Code = TC_ROOT;
	return NewRoot;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildLabel(const string &Name)
{
	CLabelTreeNode *NewLabel = new CLabelTreeNode();

	NewLabel->Name = Name;
	NewLabel->Code = TC_LABEL;

	return NewLabel;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildGotoExpr(const string &Target)
{
	CGotoExprTreeNode *GotoExpr = new CGotoExprTreeNode();

	GotoExpr->Code = TC_GOTOEXPR;
	GotoExpr->Target = Target;

	return GotoExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CTreeNodeFactory::BuildLiteralRef(CTreeNode *Lit, 
																						 const string &RefId)
{
	SanityCheck(Lit && Lit->Code == TC_LITERAL);
	CLiteralRefTreeNode *NewLitRef = new CLiteralRefTreeNode();

	NewLitRef->Code = TC_LITERALREF;
	NewLitRef->RefID = RefId;
	NewLitRef->AddChild(Lit);
	return NewLitRef;
}
//-------------------------------------------------------------

//singleton static member definition
template<> CTreeNodeFactory* ISingleton<CTreeNodeFactory>::ms_Singleton = 0;
