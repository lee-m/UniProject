//------------------------------------------------------------------------------------------
// File: CRootParseTreeNode.h
// Desc: Root node of the parse tree.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CROOTPARSETREENODE_H__
#define __CROOTPARSETREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1..n Global statements and declarations
*/

class CRootParseTreeNode : public CParseTreeNode
{
public:
	CRootParseTreeNode(void){}
	~CRootParseTreeNode(void) {}

	//inherited from CParseTreeNode
	void DumpNode(TiXmlElement *XmlNode)
	{
		//dump the tree
		for(unsigned int i = 0; i < Children.size(); i++)
			Children[i]->DumpNode(XmlNode);
	}
};

#endif