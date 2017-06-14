//------------------------------------------------------------------------------------------
// File: CArrayDeclTreeNode.h
// Desc: Representation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CARRAYDECLTREENODE_H__
#define __CARRAYDECLTREENODE_H__ 

//our includes
#include "CParseTreeNode.h"

/*
  Children:
		1. Var decl.
		2. Array bounds.
*/

class CArrayDeclTreeNode : public CParseTreeNode
{
public:
	CArrayDeclTreeNode(void) {}
	~CArrayDeclTreeNode(void) {}

	void DumpNode(TiXmlElement *XmlNode)
	{
		ParseTreeChildItr itr = Children.begin();

		//dump variable decl
		TiXmlElement *ArrDeclNode;
		ArrDeclNode = XmlNode->InsertEndChild(TiXmlElement("arr_decl"))->ToElement();
		(*itr++)->DumpNode(ArrDeclNode);

		//dump bounds
		TiXmlElement *BoundsNode;
		BoundsNode = ArrDeclNode->InsertEndChild(TiXmlElement("bounds"))->ToElement();
		(*itr)->DumpNode(BoundsNode);
	}
};

#endif