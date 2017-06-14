//------------------------------------------------------------------------------------------
// File: CParameterDeclTreeNode.h
// Desc: Representation of a parameter declaration
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARAMETERDECLTREENODE_H__
#define __CPARAMETERDECLTREENODE_H__

#include "CParseTreeNode.h"
#include "TokenTypes.h"

class CParameterDeclTreeNode : public CParseTreeNode
{
public:
	CParameterDeclTreeNode(void) {}
	~CParameterDeclTreeNode(void) {}

	//name of the parameter
	string Name;

	//parameter type
	TokenType Type;

	//the parameter const qualified?
	bool IsConst;

	void DumpNode(TiXmlElement *XmlNode)
	{
		TiXmlElement *ParmNode;
	  ParmNode = XmlNode->InsertEndChild(TiXmlElement("parm_decl"))->ToElement();
	  
		//name
		TiXmlElement *NameNode;
	  NameNode = ParmNode->InsertEndChild(TiXmlElement("name"))->ToElement();
	  NameNode->InsertEndChild(TiXmlText(Name.c_str()));

		//type
		TiXmlElement *TypeNode;
	  TypeNode = ParmNode->InsertEndChild(TiXmlElement("type"))->ToElement();
	  TypeNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(Type).c_str()));

		//is const or not
		TiXmlElement *ConstNode;
		ConstNode = ParmNode->InsertEndChild(TiXmlElement("is_const"))->ToElement();
		ConstNode->InsertEndChild(TiXmlText(IsConst ? "true" : "false"));
	}
};

#endif