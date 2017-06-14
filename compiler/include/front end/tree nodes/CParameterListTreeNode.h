//------------------------------------------------------------------------------------------
// File: CParameterListTreeNode.h
// Desc: Representation of a function's declared parameters within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARAMETERLISTTREENODE_H__
#define __CPARAMETERLISTTREENODE_H__

#include "CTreeNode.h"

/**
 * Container class for CParameterDeclTreeNodes with each individual
 * parameter being stored as a child of this one.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0 -> N. Identifier for this decl.
 */
class CParameterListTreeNode : public CTreeNode
{
private:
	CParameterListTreeNode(void) {}
	~CParameterListTreeNode(void) {}

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
	 * Inherited from CTreeNode. Nothing to do for this
	 * node so always returns NULL.
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
