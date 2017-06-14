//------------------------------------------------------------------------------------------
// File: CAssignExprTreeNode.h
// Desc: Representation of an assignment expression within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CASSIGNEXPRTREENODE_H__
#define __CASSIGNEXPRTREENODE_H__

#include "CTreeNode.h"

/**
 * Representation of an assignment expression in the parse/MIR tree.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. the identifier or array decl being assigned to <br>
 * &nbsp;&nbsp; 1. expression the identifier is being assigned.
*/
class CAssignExprTreeNode : public CTreeNode
{
private:
	CAssignExprTreeNode(void) {}
	~CAssignExprTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

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
	 * Inherited from CTreeNode. Rewrites the RHS of the expression
	 * if required to break it down into simpler statements.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
