//------------------------------------------------------------------------------------------
// File: CRootTreeNode.h
// Desc: Root node of the tree.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CROOTTREENODE_H__
#define __CROOTTREENODE_H__

#include "CTreeNode.h"

/**
 * This type of tree node does not represent any source of middle-end 
 * language construct. Instead it's used as a container for other tree
 * nodes. Uses of this class are for the "then" and "else" branches of an
 * if statement and also for the main parse tree - the type of tree returned
 * from the parser and MIR lowering is of this type with each global declaration
 * or statement being a child of this class.
 */

class CRootTreeNode : public CTreeNode
{
private:
	CRootTreeNode(void){}
	~CRootTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/**
	 * Inherited from CTreeNode. Calls this function on each
	 * of it's children which does the actual work.
	 * @see CTreeNode.
	 */
	void DumpAsParseTreeNode(TiXmlElement *XmlNode);

	/**
	 * Inherited from CTreeNode. Calls this function on each
	 * of it's children which does the actual work.
	 * @see CTreeNode.
	 */
	void DumpAsMIRTreeNode(ofstream &OutFile);

	/**
	 * Inherited from CTreeNode. Asks each child node to rewrite
	 * itself into MIR form, passing the Stmts parameter down 
	 * through the tree.
	 * @see CTreeNode.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode. Create a deep copy of each
	 * node added.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
