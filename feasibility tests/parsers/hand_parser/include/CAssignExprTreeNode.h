//------------------------------------------------------------------------------------------
// File: CAssignExprTreeNode.h
// Desc: Representation of an assignment expression within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CASSIGNEXPRTREENODE_H__
#define __CASSIGNEXPRTREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1. the IDENTIFIER or ARRAY_DECL being assigned to
		2. expression the identifier is being assigned.
*/

class CAssignExprTreeNode : public CParseTreeNode
{
public:
	CAssignExprTreeNode(void) {}
	~CAssignExprTreeNode(void) {}

	void DumpNode(TiXmlElement *XmlNode)
	{
		assert(XmlNode);

		TiXmlElement *ExpNode;
		ExpNode = XmlNode->InsertEndChild(TiXmlElement("assign_expr"))->ToElement();

		//add the lhs
		ParseTreeChildItr itr = Children.begin();

		TiXmlElement *LhsNode;
		LhsNode = ExpNode->InsertEndChild(TiXmlElement("lhs"))->ToElement();
		(*itr)->DumpNode(LhsNode);

		//add the rhs
		TiXmlElement *RhsNode;
		RhsNode = ExpNode->InsertEndChild(TiXmlElement("rhs"))->ToElement();
		(*++itr)->DumpNode(RhsNode);
	}
};

#endif