//------------------------------------------------------------------------------------------
// File: CParseTree.cpp
// Desc: Encapsulation of the parse tree built by the parser
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CParseTree.h"

//-------------------------------------------------------------
CParseTree::CParseTree(void)
{
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTree::~CParseTree(void)
{
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildIdentifier(const string &Name)
{
	CIdentifierTreeNode *NewIdNode = new CIdentifierTreeNode();

	NewIdNode->Code = TC_IDENTIFIER;
	NewIdNode->IdName = Name;
	
	return NewIdNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildVarDecl(const string &Name, bool IsConst, TokenType VarType)
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
CParseTreeNode* CParseTree::BuildArrayDeclaration(CParseTreeNode *VarDecl,
																									CParseTreeNode *Bounds)
{
	assert(VarDecl && Bounds);
	CArrayDeclTreeNode *NewArrayDecl = new CArrayDeclTreeNode();

	NewArrayDecl->Code = TC_ARRAYDECL;
	NewArrayDecl->Children.push_back(VarDecl);
	NewArrayDecl->Children.push_back(Bounds);

	return NewArrayDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildLiteral(const string &Value, TokenType LitType)
{
	CLiteralTreeNode *NewLiteralNode = new CLiteralTreeNode();

	NewLiteralNode->Code = TC_LITERAL;
	NewLiteralNode->LitValue = Value;
	NewLiteralNode->LitType = LitType;

	return NewLiteralNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildBinaryExpression(CParseTreeNode *Lhs,
																									CParseTreeNode *Rhs, TokenType Op)
{
	assert(Lhs && Rhs);
	CBinaryExprTreeNode *NewBinExp = new CBinaryExprTreeNode();

	NewBinExp->Code = TC_BINARY_EXPR;
	NewBinExp->Operator = Op;
	NewBinExp->Children.push_back(Lhs);
	NewBinExp->Children.push_back(Rhs);

	return NewBinExp;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildUnaryExpression(CParseTreeNode *Expr)
{
	CUnaryExprTreeNode *NewUnaryExpr = new CUnaryExprTreeNode();

	NewUnaryExpr->Code = TC_UNARY_EXPR;
  NewUnaryExpr->Children.push_back(Expr);

	return NewUnaryExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildArrayRef(CParseTreeNode *Index, const string &ArrName)
{
	assert(Index);
	CArrayRefTreeNode *NewArrRef = new CArrayRefTreeNode();

	NewArrRef->Code = TC_ARRAY_REF;
	NewArrRef->ArrName = ArrName;
	NewArrRef->Children.push_back(Index);

	return NewArrRef;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildIfStatement(CParseTreeNode *CondExpr)
{
	assert(CondExpr);
	CIfStmtTreeNode *NewIfStmt = new CIfStmtTreeNode();

	NewIfStmt->Code = TC_IFSTMT;
	NewIfStmt->Children.push_back(CondExpr);

	//add two sub-tree nodes for the 'if' and 'else' bodies
	NewIfStmt->Children.push_back(BuildRootNode());
	NewIfStmt->Children.push_back(BuildRootNode());

	return NewIfStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildWhileStatement(CParseTreeNode *CondExpr)
{
	assert(CondExpr);
	CWhileStmtTreeNode *NewWhileStmt = new CWhileStmtTreeNode();

	NewWhileStmt->Code = TC_WHILESTMT;
	NewWhileStmt->Children.push_back(CondExpr);

	return NewWhileStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildAssignmentExpr(CParseTreeNode *Lhs, CParseTreeNode *Rhs)
{
	assert(Lhs && Rhs);
	CAssignExprTreeNode *NewAssignExpr = new CAssignExprTreeNode();

	NewAssignExpr->Code = TC_ASSIGN_EXPR;
	NewAssignExpr->Children.push_back(Lhs);
	NewAssignExpr->Children.push_back(Rhs);

	return NewAssignExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildParameterList(void)
{
	CParameterListTreeNode *NewParmList = new CParameterListTreeNode();
	NewParmList->Code = TC_PARAMETER_LIST;
	return NewParmList;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildFunctionDecl(const string &Name, int NumParms, 
																							CParseTreeNode *ParmList, TokenType RetType)
{
	CFunctionDeclTreeNode *NewFnDecl = new CFunctionDeclTreeNode();

	NewFnDecl->Code = TC_FUNCTIONDECL;
	NewFnDecl->Name = Name;
	NewFnDecl->NumParameters = NumParms;
	NewFnDecl->RetType = RetType;
	NewFnDecl->Children.push_back(ParmList);

	return NewFnDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildParameterDecl(const string &Name, TokenType Type, bool IsConst)
{
	CParameterDeclTreeNode *NewParmDecl = new CParameterDeclTreeNode();

	NewParmDecl->Code = TC_PARMDECL;
	NewParmDecl->Name = Name;
	NewParmDecl->Type = Type;
	NewParmDecl->IsConst = IsConst;

	return NewParmDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildReturnStmt(CParseTreeNode *Expr)
{
	CReturnStmtTreeNode *NewRetStmt = new CReturnStmtTreeNode();

	NewRetStmt->Code = TC_RETURNSTMT;
	NewRetStmt->Children.push_back(Expr);

	return NewRetStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildCallExpr(const string &FuncName)
{
	CCallExprTreeNode *NewCallExpr = new CCallExprTreeNode();

	NewCallExpr->Code = TC_CALLEXPR;
	NewCallExpr->FuncName = FuncName;

	return NewCallExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildUnaryExpression(CParseTreeNode *Expr, TokenType Op)
{
	CUnaryExprTreeNode *UnExpr = new CUnaryExprTreeNode();

	UnExpr->Code = TC_UNARY_EXPR;
	UnExpr->Operator = Op;
	UnExpr->Children.push_back(Expr);

	return UnExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParseTree::BuildRootNode(void)
{
	CRootParseTreeNode *NewRoot = new CRootParseTreeNode();

	NewRoot->Code = TC_ROOT;
	return NewRoot;
}
//-------------------------------------------------------------
