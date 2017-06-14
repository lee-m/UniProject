//------------------------------------------------------------------------------------------
// File: CTreeNodeFactory.h
// Desc: Build tree nodes of various types
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARSETREEBUILDER_H__
#define __CPARSETREEBUILDER_H__

#include "TokenTypes.h"
#include "ISingleton.h"

//forward decls
class CTreeNode;
class CLiteralTreeNode;

/**
 * This is the only class which can construct the different types
 * of tree nodes so we can ensure that all required attributes are 
 * set before the node is used.
 */
class CTreeNodeFactory : public ISingleton<CTreeNodeFactory>
{
public:
	CTreeNodeFactory(void) {};
	~CTreeNodeFactory(void) {};

	/**
	 * Build a new identifier tree node.
	 * @param Name The name fof the identifier.
	 */
	CTreeNode* BuildIdentifier(const string &Name);

	/**
	 * Build a variable declaration
	 * @param Name The variable's name.
	 * @param IsConst Was the variable declared constant?
	 * @param VarType The declared type of the variable.
	 */
	CTreeNode* BuildVarDecl(const string &Name, bool IsConst, TokenType VarType);

	/**
	 * Build a new array declaration tree node.
	 * @param VarDecl The variable declaration we're wrapping with this
	 *        new node.
	 * @param Bounds The declared bounds of the array.
	 */
	CTreeNode* BuildArrayDeclaration(CTreeNode *VarDecl, CTreeNode *Bounds);

	/**
	 * Build a new literal tree node.
	 * @param Value The value of the literal.
	 * @param LitType The type of the literal - integral, floating point or string.
	 */
	CTreeNode* BuildLiteral(const string &Value, TokenType LitType);

	/**
	 * Build a new arithmetic expression tree node. Both the LHS and RHS
	 * can be sub-expressions represented by arith_expr or bln_expr nodes.
	 * @param Lhs The term on the LHS of the expression.
	 * @param Rhs The term on the RHS of the expression.
	 * @param Op The arithmetic operator to apply.
	 */
	CTreeNode* BuildArithExpression(CTreeNode *Lhs, CTreeNode *Rhs, TokenType Op);

	/**
	 * Build a new Boolean expression tree node. Like arithmetic expression
	 * nodes both the LHS and RHS can be sub-expressions.
	 * @param Lhs Term on the LHS of the expression.
	 * @param Rhs Term on the RHS of the expression.
	 * @param Op Boolean operator to apply.
	 */
	CTreeNode* BuildBooleanExpression(CTreeNode *Lhs, CTreeNode *Rhs, TokenType Op);

	/**
	 * Build a new array element access tree node.
	 * @param Index Expression which forms the index of the element to access.
	 * @param ArrName Name of the array variable being access.
	 * @param Type Type of the array being accessed.
	 */
	CTreeNode* BuildArrayRef(CTreeNode *Index, const string &ArrName, TokenType Type);

	/**
	 * Build a new if statement tree node.
	 * @param CondExpr Expression forming the condition to test.
	 */
	CTreeNode* BuildIfStatement(CTreeNode *CondExpr);

	/**
	 * Build a new while statement tree node.
	 * @param CondExpr Expression forming the conditional which controls the 
	 *        loops iteration.
	 */
	CTreeNode* BuildWhileStatement(CTreeNode *CondExpr);

	/**
	 * Build a new assignment expression.
	 * @param Lhs The variable which appears on the LHS of the "="
	 * @param Rhs Expression or value being assigned to the LHS.
	 */
	CTreeNode* BuildAssignmentExpr(CTreeNode *Lhs, CTreeNode *Rhs);

	/** Construct a new, empty parameter list. */
	CTreeNode* BuildParameterList(void);

	/**
	 * Build a new function declaration tree node.
	 * @param Name Name of the function.
	 * @param NumParms Number of parameters the function takes.
	 * @param ParmList List of parameter declaration tree nodes.
	 * @param RetType Declared return type of the function.
	 */
	CTreeNode* BuildFunctionDecl(const string &Name, int NumParms,
															 CTreeNode *ParmList, TokenType RetType);

	/**
	 * Build a new parameter decl tree node.
	 * @param Name Name of the parameter.
	 * @param Type Declared type of the parameter.
	 * @param IsConst Specifies whether the parameter is "const" qualified.
	 * @param Position The position it was specified in the parameter list.
	 *        Indexed from 0, not 1.
	 */
	CTreeNode* BuildParameterDecl(const string &Name, TokenType Type, 
		                            bool IsConst, int Position);

	/** 
	 * Build a new return statement tree node.
	 * @param Expr The expression forming the value being returned.
	 */
	CTreeNode* BuildReturnStmt(CTreeNode *Expr);

	/** 
	 * Build a new function call expression tree node.
	 * @param FuncName Name of the function being called.
	 */
	CTreeNode* BuildCallExpr(const string &FuncName);

	/**
	 * Build a new unary expression tree node.
	 * @param Expr The expression the operator applies to.
	 * @param Op The operator to apply.
	 */
	CTreeNode* BuildUnaryExpression(CTreeNode *Expr, TokenType Op);

	/** Build a new root tree node. */
	CTreeNode* BuildRootNode(void);

	/**
	 * Build a new label tree node. Only used by the middle-end
	 * @param Name Name of the label of the form "LXXX".
	 */
	CTreeNode* BuildLabel(const string &Name);

	/**
	 * Build a goto expression tree node. Only used by the middle-end.
	 * @param Target Name of the label which this goto targets.
	 */
	CTreeNode* BuildGotoExpr(const string &Target);

	/**
	 * Build a new literal reference tree node.
	 * @param Lit The literal being referenced.
	 * @param RefId Unique reference name to refer to the literal.
	 */
	CTreeNode* BuildLiteralRef(CTreeNode *Lit, const string &RefId);
};

#endif
