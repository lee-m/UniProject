//------------------------------------------------------------------------------------------
// File: CArithExprTreeNode.h
// Desc: Binary expression of the form Exp op Exp using floating point or 
//       integral values
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CARITHEXPRTREENODE_H__
#define __CARITHEXPRTREENODE_H__

#include "CTreeNode.h"
#include "TokenTypes.h"

/**
 * Representation of a binary arithmetic expression. Used by both the front
 * and middle-end although when this node appears in the ME onwards it will
 * be in MIR form.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. LHS of the expression <br>
 * &nbsp;&nbsp; 1. RHS of the expression
*/
class CArithExprTreeNode : public CTreeNode
{
private:
	CArithExprTreeNode(void) {}
	~CArithExprTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

	/**
	 * Looks at the LHS of the expression to determine it's type.
	 * @param Lhs Left hand side of the expression.
	 * @return The type of the LHS used to create the temp variable
	 *         holding the result of this expression.
	 */
	TokenType DetermineTempVarType(CTreeNode *Lhs);

public:

	/** Operator for this expression */
	TokenType Operator;

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	void DumpAsParseTreeNode(TiXmlElement *XmlNode);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	void DumpAsMIRTreeNode(ofstream &OutFile);

	/**
	 * Inherited from CTreeNode. If either operand of the expression is
	 * itself another expression, perform the necessary simplification 
	 * by breaking it down into smaller expressions using compiler generated
	 * temporary variables.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
