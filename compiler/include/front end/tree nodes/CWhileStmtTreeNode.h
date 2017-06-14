//------------------------------------------------------------------------------------------
// File: CWhileStmtTreeNode.h
// Desc: Representation of a WHILE statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CWHILESTMTTREENODE_H__
#define __CWHILESTMTTREENODE_H__

#include "CTreeNode.h"

/**
 * Representation of a "while" statement within the IR including the
 * conditional statement controlling the iteration and any statement
 * that make up it's body.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Conditional expr. <br>
 * &nbsp;&nbsp; 1 -> N. Body of the while statement.
 */
class CWhileStmtTreeNode : public CTreeNode
{
private:
	CWhileStmtTreeNode(void) {}
	~CWhileStmtTreeNode(void) {}

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
	 * Inherited from CTreeNode. Calls this function on each
	 * statement in the body.
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
