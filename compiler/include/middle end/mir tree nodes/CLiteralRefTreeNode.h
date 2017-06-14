//------------------------------------------------------------------------------------------
// File: CLiteralRefTreeNode.h
// Desc: An indirect reference to a string or floating point literal
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CLITERALREFTREENODE_H__
#define __CLITERALREFTREENODE_H__

#include "CTreeNode.h"
#include "CTreeNodeFactory.h"

/**
 * MIR specific tree node to represent a string or floating point iteral. 
 * We use this in the middle/back ends as these particular types of data are
 * not capable of being stored in normal GP registers, while FP literals
 * can be in the FP registers, they have to go through memory to get there.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Literal being referred to.
 */
class CLiteralRefTreeNode : public CTreeNode
{
private:

	CLiteralRefTreeNode(void) {}
	~CLiteralRefTreeNode(void) {}

	//only let the tree node factory class create instances of this class
	friend class CTreeNodeFactory;

public:

	/** Unique reference id. */
	string RefID;

	/**
	 * Inherited from CTreeNode. 
	 * @see CTreeNode.
	 */
	void DumpAsMIRTreeNode(ofstream &OutFile)
	{
		OutFile << "<" + RefID + ">";
	}

	/**
	 * Inherited from CTreeNode. Should never be called.
	 * @see CTreeNode.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts)
	{
		InternalError("attempt to rewrite literal ref node into MIR form");
	}

	/**
	 * Inherited from CTreeNode. Should never be called.
	 * @see CTreeNode.
	 */
	void DumpAsParseTreeNode(TiXmlElement *XmlNode)
	{
		InternalError("attempt to dump literal ref as parse tree node");
	}

	/**
	 * Inherited from CTreeNode. Should never be called.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void)
	{
		return CTreeNodeFactory::GetSingletonPtr()->BuildLiteralRef(
			GetChild(LITREF_LITERAL), RefID);
	}
};

#endif