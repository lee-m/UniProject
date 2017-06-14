//------------------------------------------------------------------------------------------
// File: CIdentifierTreeNode.h
// Desc: Representation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CIDENTIFIERTREENODE_H__
#define __CIDENTIFIERTREENODE_H__ 

#include "CTreeNode.h"

/**
 * A simple tree node which simply stores the name
 * of something (function, variable etc) so it can be stored
 * as a child of another tree node.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; None.
 */
class CIdentifierTreeNode : public CTreeNode
{
private:
	CIdentifierTreeNode(void) {}
	~CIdentifierTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Name of the identifier. */
	string IdName;

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
	 * Inherited from CTreeNode. Doesn't actually do anything
	 * and simply returns a copy of itself.
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
