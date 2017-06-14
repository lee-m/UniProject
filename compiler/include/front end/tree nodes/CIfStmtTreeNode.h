//------------------------------------------------------------------------------------------
// File: CIfStmtTreeNode.h
// Desc: Representation of an IF statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CIFSTMTTREENODE_H__
#define __CIFSTMTTREENODE_H__

#include "CTreeNode.h"

/**
 * Tree node for the if..then..else construct from the language. The
 * contents of the node is broken down into three parts with one child
 * node for each - the conditional expression, the "then" body and finally
 * the "else" body. Both the "then" and "else" child nodes are of the type
 * CRootTreeNode which contains the statements making up the respective bodies.
 * If the statement was written without using "else", there will still be 
 * a child node to represent it, but it will not have any children.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Condition expression <br>
 * &nbsp;&nbsp; 1. Body of the if statement. <br>
 * &nbsp;&nbsp; 2. Body of the else statement.
 */
class CIfStmtTreeNode : public CTreeNode
{
private:
	CIfStmtTreeNode(void) {}
	~CIfStmtTreeNode(void) {}

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
	 * Perform the simplification of all children - the conditional, if body
	 * and else body. When processing the two bodies it's a recursive process
	 * to handle each statement in turn.
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
