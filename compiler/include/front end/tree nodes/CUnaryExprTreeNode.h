//------------------------------------------------------------------------------------------
// File: CUnaryExprTreeNode.h
// Desc: Unary expression of the form op Exp where Exp can itself be another expr
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CUNARYEXPRTREENODE_H__
#define __CUNARYEXPRTREENODE_H__

#include "CTreeNode.h"

/**
 * Tree node to represent the use an a unary operator in the
 * IR. Currently there is only a single unary operator supported
 * so there is no need to store the operator but we do so anyway
 * in the name of extensibility.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Expression the operator acts upon. 
 */
class CUnaryExprTreeNode : public CTreeNode
{
private:
	CUnaryExprTreeNode(void) {}
	~CUnaryExprTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Operator for this expression. */
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
	 * Inherited from CTreeNode. Converts the expression
	 * to MIR form if it's not already.
	 * @see CTreeNode.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
