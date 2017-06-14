//------------------------------------------------------------------------------------------
// File: CReturnStmtTreeNode.h
// Desc: Representation of a return statement
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CRETURNSTMTTREENODE_H__
#define __CRETURNSTMTTREENODE_H__

#include "CTreeNode.h"

/**
 * Simple tree node which is used to represent a expression
 * forming the return value of a function.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Expression forming the value being returned.
 */

class CReturnStmtTreeNode : public CTreeNode
{
private:
	CReturnStmtTreeNode(void) {}
	~CReturnStmtTreeNode(void) {}

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
	 * Inherited from CTreeNode. Performs the necessary simplification
	 * of the return expression, returns either the original expression
	 * or the last statement created from the simplification.
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
