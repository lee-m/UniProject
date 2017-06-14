//------------------------------------------------------------------------------------------
// File: CCallExprTreeNode.h
// Desc: Representation of a WHILE statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CCALLEXPRTREENODE_H__
#define __CCALLEXPRTREENODE_H__

#include "CTreeNode.h"

/** 
 * Representation of a function call in the IR
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0 -> N. Parameter decl tree nodes
 */
class CCallExprTreeNode : public CTreeNode
{
private:
	CCallExprTreeNode(void) {}
	~CCallExprTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Name of the function we're calling. */
	string FuncName;

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
	 * If the function being called takes any parameters, the expressions
	 * forming the values of those parameters are simplified here if
	 * required and replaced with compiler generated temporary variables.
	 * If none of the parameters are complex or there are no parameters then
	 * the tree returned is essentially an exact copy of the original.
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
