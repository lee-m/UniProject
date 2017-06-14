//------------------------------------------------------------------------------------------
// File: CArrayDeclTreeNode.h
// Desc: Representation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CARRAYDECLTREENODE_H__
#define __CARRAYDECLTREENODE_H__ 

#include "CTreeNode.h"

/**
 * Representation of an array declaration within the program. Implemented
 * as a wrapper class around CVarDeclTreeNode which stores the majority of
 * the nodes attributes.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Var decl <br>
 * &nbsp;&nbsp; 1. Bounds
*/
class CArrayDeclTreeNode : public CTreeNode
{
private:
	CArrayDeclTreeNode(void) {}
	~CArrayDeclTreeNode(void) {}

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
	 * Nothing much to do here so simply creates and returns a copy
	 * of the original node.
	 * @see CTreeNode::RewriteIntoMIRForm
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
