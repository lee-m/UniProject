//------------------------------------------------------------------------------------------
// File: CArrayRefTreeNode.h
// Desc: Represents an access to an array element
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CARRAYREFTREENODE_H__
#define __CARRAYREFTREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1. Index
*/

class CArrayRefTreeNode : public CParseTreeNode
{
public:
	CArrayRefTreeNode(void) {}
	~CArrayRefTreeNode(void) {}

	//identifier name of the array we're indexing into 
	string ArrName;

	void DumpNode(TiXmlElement *XmlNode)
	{
		TiXmlElement *ArrRefNode;
		ArrRefNode = XmlNode->InsertEndChild(TiXmlElement("array_ref"))->ToElement();

		TiXmlElement *IndexNode;
		IndexNode = ArrRefNode->InsertEndChild(TiXmlElement("index"))->ToElement();

		//dump the expression used for the index
		ParseTreeChildItr itr = Children.begin();
		(*itr)->DumpNode(IndexNode);
	}
};

#endif