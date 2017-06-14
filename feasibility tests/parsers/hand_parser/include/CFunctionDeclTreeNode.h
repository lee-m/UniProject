//------------------------------------------------------------------------------------------
// File: CFunctionDefTreeNode.h
// Desc: Representation of a function within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CFUNCTIONDECLTREENODE_H__
#define __CFUNCTIONDECLTREENODE_H__

#include "CParseTreeNode.h"
#include "TokenTypes.h"
#include "CUtilFuncs.h"

/*
  Children:
	  1. Parameter list.
		2..n Function body statements
*/

class CFunctionDeclTreeNode : public CParseTreeNode
{
public:
	CFunctionDeclTreeNode(void) {}
	~CFunctionDeclTreeNode(void) {}

	//return type of the function
	TokenType RetType;

	//number of declared parameters
	int NumParameters;

	//name of the function
	string Name;

	void DumpNode(TiXmlElement *XmlNode)
	{
		TiXmlElement *FnDeclNode;
		FnDeclNode = XmlNode->InsertEndChild(TiXmlElement("function_decl"))->ToElement();

		//name of the function
		TiXmlElement *NameNode;
	  NameNode = FnDeclNode->InsertEndChild(TiXmlElement("name"))->ToElement();
	  NameNode->InsertEndChild(TiXmlText(Name.c_str()));

		//return type
		TiXmlElement *RetTypeNode;
	  RetTypeNode = FnDeclNode->InsertEndChild(TiXmlElement("return_type"))->ToElement();
		RetTypeNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(RetType).c_str()));

		//parameter list
		TiXmlElement *ParmListNode;
		ParmListNode = FnDeclNode->InsertEndChild(TiXmlElement("parameter_list"))->ToElement();
		Children[0]->DumpNode(ParmListNode);

		//function body
		TiXmlElement *StmtBody;
		StmtBody = FnDeclNode->InsertEndChild(TiXmlElement("function_decl_body"))->ToElement();
		
		for(unsigned int i = 1; i < Children.size(); i++)
			Children[i]->DumpNode(StmtBody);
	}

};

#endif