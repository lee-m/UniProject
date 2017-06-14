//------------------------------------------------------------------------------------------
// File: CGotoExprTreeNode.h
// Desc: Representation of a label which is the target of a goto expr.
//       Only used in MIR form
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CGOTOEXPRTREENODE_H__
#define __CGOTOEXPRTREENODE_H__

#include "CTreeNode.h"
#include "CTreeNodeFactory.h"

/**
 * MIR specific tree node which is generated during flattening of
 * an "if" statement in straight line code. By itself, a goto represents
 * an unconditional jump but placed inside the body of an "if" statement
 * it becomes conditional. Any attempt to use this tree node from the
 * front-end will trigger an ICE.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; None.
 */
class CGotoExprTreeNode : public CTreeNode
{
private:
	CGotoExprTreeNode(void) {}
	~CGotoExprTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Target of the jump. */
	string Target;

	/**
	 * Inherited from CTreeNode. Should never be called.
	 * @see CTreeNode.
	 */
	void DumpAsParseTreeNode(TiXmlElement *XmlNode)
	{
		//should never happen
		InternalError("attempt to dump goto expr as parse tree node");
	}

	/**
	 * Inherited from CTreeNode. Should never be called.
	 * @see CTreeNode.
	 */
	void DumpAsMIRTreeNode(ofstream &OutFile)
	{
		OutFile << "goto " << Target;
	}

	/**
	 * Inherited from CTreeNode. Should never be called.
	 * @see CTreeNode.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts)
	{
		//shouldn't happen
		InternalError("attempt to rewrite goto expr into MIR");
	}

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void)
	{
		return CTreeNodeFactory::GetSingletonPtr()->BuildGotoExpr(Target);
	}
};

#endif
