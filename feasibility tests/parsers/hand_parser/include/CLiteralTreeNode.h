//------------------------------------------------------------------------------------------
// File: CLiteralTreeNode.h
// Desc: A literal value representation.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CLITERALTREENODE_H__
#define __CLITERALTREENODE_H__

#include "CParseTreeNode.h"
#include "TokenTypes.h"
#include "CUtilFuncs.h"

/*
  Children:
	  None.
*/

class CLiteralTreeNode : public CParseTreeNode
{
public:
	CLiteralTreeNode(void) {}
	~CLiteralTreeNode(void) {}

	//type of the literal
	TokenType LitType;

	//literal value
	string LitValue;

	void DumpNode(TiXmlElement *XmlNode)
	{
		TiXmlElement *LitNode;
	  LitNode = XmlNode->InsertEndChild(TiXmlElement("literal_node"))->ToElement();
	  
		//value
		TiXmlElement *ValNode;
	  ValNode = LitNode->InsertEndChild(TiXmlElement("value"))->ToElement();
	  ValNode->InsertEndChild(TiXmlText(LitValue.c_str()));

		//type
		TiXmlElement *TypeNode;
	  TypeNode = LitNode->InsertEndChild(TiXmlElement("type"))->ToElement();
	  TypeNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(LitType).c_str()));
	}
};

#endif