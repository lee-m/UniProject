//------------------------------------------------------------------------------------------
// File: CParameterDeclTreeNode.h
// Desc: Representation of a parameter declaration
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARAMETERDECLTREENODE_H__
#define __CPARAMETERDECLTREENODE_H__

#include "CTreeNode.h"
#include "TokenTypes.h"

/**
 * Tree node to represent a single entry in the parameter list 
 * of a function declaration. This node isn't used by itself and
 * is instead wrapped in a CParameterListTreeNode which stores all
 * declared parameters for a function.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Identifier for this decl.
 */
class CParameterDeclTreeNode : public CTreeNode
{
private:
	CParameterDeclTreeNode(void) {}
	~CParameterDeclTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Parameter type. */
	TokenType Type;

	/** Is this parm decl const qualified? */
	bool IsConst;

	/** It's position in the argument list. */
	int Position;
	
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
