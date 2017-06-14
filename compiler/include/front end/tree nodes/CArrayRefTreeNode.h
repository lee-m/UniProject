//------------------------------------------------------------------------------------------
// File: CArrayRefTreeNode.h
// Desc: Represents an access to an array element access
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CARRAYREFTREENODE_H__
#define __CARRAYREFTREENODE_H__

#include "CTreeNode.h"

/**
 * Representation of an array element access.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Index expression
*/
class CArrayRefTreeNode : public CTreeNode
{
private:
	CArrayRefTreeNode(void) {}
	~CArrayRefTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Identifier name of the array we're indexing into */
	CIdentifierTreeNode *ArrName;

	/** Type of the array */
	TokenType ArrType;

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
	 * Inherited from CTreeNode, overridden here so the identifier
	 * node can be destroyed as well.
	 */
	void DestroyNode(void);

	/**
	 * Inherited from CTreeNode. Not a great deal to do for this 
	 * particular node as all that's required is to simplify the
	 * index expression if need be.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
