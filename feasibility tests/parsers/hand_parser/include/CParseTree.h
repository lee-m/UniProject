//------------------------------------------------------------------------------------------
// File: CParseTree.h
// Desc: Encapsulation of the parse tree built by the parser
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARSETREE_H__
#define __CPARSETREE_H__

#include "TokenTypes.h"

//forward decls
class CParseTreeNode;

class CParseTree
{
public:
	CParseTree(void);
	~CParseTree(void);

	//build a tree of different types
	CParseTreeNode* BuildIdentifier(const string &Name);
	CParseTreeNode* BuildVarDecl(const string &Name, bool IsConst, TokenType VarType);
	CParseTreeNode* BuildArrayDeclaration(CParseTreeNode *VarDecl, CParseTreeNode *Bounds);
	CParseTreeNode* BuildLiteral(const string &Value, TokenType LitType);
	CParseTreeNode* BuildBinaryExpression(CParseTreeNode *Lhs, CParseTreeNode *Rhs, TokenType Op);
	CParseTreeNode* BuildUnaryExpression(CParseTreeNode *Expr);
	CParseTreeNode* BuildArrayRef(CParseTreeNode *Index, const string &ArrName);
	CParseTreeNode* BuildIfStatement(CParseTreeNode *CondExpr);
	CParseTreeNode* BuildWhileStatement(CParseTreeNode *CondExpr);
	CParseTreeNode* BuildAssignmentExpr(CParseTreeNode *Lhs, CParseTreeNode *Rhs);
	CParseTreeNode* BuildParameterList(void);
	CParseTreeNode* BuildFunctionDecl(const string &Name, int NumParms, 
																		CParseTreeNode *ParmList, TokenType RetType);
	CParseTreeNode* BuildParameterDecl(const string &Name, TokenType Type, bool IsConst);
	CParseTreeNode* BuildReturnStmt(CParseTreeNode *Expr);
	CParseTreeNode* BuildCallExpr(const string &FuncName);
	CParseTreeNode* BuildUnaryExpression(CParseTreeNode *Expr, TokenType Op);
	CParseTreeNode* BuildRootNode(void);
};

#endif