//------------------------------------------------------------------------------------------
// File: CBooleanExprTreeNode.h
// Desc: Binary expression of the form Exp op Exp using boolean values and sub-expressions
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CBOOLEANEXPRTREENODE_H__
#define __CBOOLEANEXPRTREENODE_H__

#include "CTreeNode.h"
#include "TokenTypes.h"

//forward decls
class CIdentifierTreeNode;

/**
 * At the semantic level both this class and CArithExprTreeNode are the same,
 * and can be used interchangably, the distinction comes when we reach the lower
 * levels of representation in which this difference does become important. This 
 * why there are two separate classes for arithmetic and boolean expression 
 * tree nodes.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. LHS of the expression <br>
 * &nbsp;&nbsp; 1. RHS of the expression.
 */
class CBooleanExprTreeNode : public CTreeNode
{
private:
	CBooleanExprTreeNode(void) {}
	~CBooleanExprTreeNode(void){}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

	/**
	 * Rewrite the Boolean expr into an arithmetic form if it's
	 * of logical type. Operations against true and false are replaced
	 * with the literal values 1 and 0 respectively.
	 * @param RhsNode The simplified RHS of the original expression
	 * @param TempVar The temporary variable which will hold the rewritten
	 *        result of the expression.
	 * @param Stmts Global MIR tree passed in the compiler driver.
	 */
	CTreeNode *RewriteExprIntoArithmeticForm(CTreeNode *RhsNode, 
		CVarDeclTreeNode *TempVar, CTreeNode *Stmts);

	/** 
	 * Determine if we have a relational or logical operator.
	 * @return True if we have a relation operator, false if we
	 *         have a logical one.
	 */
	bool HasRelationalOperator(void);

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
	 * For logical Boolean expressions, rewrite it into a series
	 * or arithmetic equivalents using the literal values 0 and 1 
	 * instead of "true" and "false". For relation expressions these
	 * transformations aren't applied, only the normal simplifications.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
