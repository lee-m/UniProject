//------------------------------------------------------------------------------------------
// File: CIdentifierTreeNode.h
// Desc: Representation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CIDENTIFIERTREENODE_H__
#define __CIDENTIFIERTREENODE_H__ 

#include "CParseTreeNode.h"

/*
  Children:
	  None.
*/

class CIdentifierTreeNode : public CParseTreeNode
{
public:
	CIdentifierTreeNode(void) {}
	~CIdentifierTreeNode(void) {}

	//add this node to the XML document
	void DumpNode(TiXmlElement *XmlNode)
	{
		TiXmlElement *IdNode;
		IdNode = XmlNode->InsertEndChild(TiXmlElement("identifier"))->ToElement();
		IdNode->InsertEndChild(TiXmlText(IdName.c_str()));
	}

	//name of the identifier
	string IdName;
};

#endif