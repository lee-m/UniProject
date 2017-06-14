//------------------------------------------------------------------------------------------
// File: CLiteralTreeNode.h
// Desc: A literal value representation.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CLITERALTREENODE_H__
#define __CLITERALTREENODE_H__

#include "CTreeNode.h"
#include "TokenTypes.h"

/**
 * Like CIdentifierTreeNode only this time we store a literal
 * value such as strings, integral, floating point of Boolean types.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; None.
 */
class CLiteralTreeNode : public CTreeNode
{
private:
	CLiteralTreeNode(void) {}
	~CLiteralTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Type of the literal */
	TokenType LitType;

	/** literal value */
	string LitValue;

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
	 * Inherited from CTreeNode. Nothing to do here so 
	 * simply returns a copy of itself.
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
