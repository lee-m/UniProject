//------------------------------------------------------------------------------------------
// File: CParameterListTreeNode.h
// Desc: Representation of a function's declared parameters within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARAMETERLISTTREENODE_H__
#define __CPARAMETERLISTTREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1..n. Parameter decl nodes for each declared parameter
*/

class CParameterListTreeNode : public CParseTreeNode
{
public:
	CParameterListTreeNode(void) {}
	~CParameterListTreeNode(void) {}

	void DumpNode(TiXmlElement *XmlNode)
	{
		for(unsigned int i = 0; i < Children.size(); i++)
			Children[i]->DumpNode(XmlNode);
	}
};

#endif