//------------------------------------------------------------------------------------------
// File: CVarDeclTreeNode.cpp
// Desc: Variable declaration within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CUtilFuncs.h"

//-------------------------------------------------------------
void CVarDeclTreeNode::DumpNode(TiXmlElement *XmlNode)
{
	TiXmlElement *VarDeclNode;
	VarDeclNode = XmlNode->InsertEndChild(TiXmlElement("var_decl"))->ToElement();

	//add the const attribute
	TiXmlElement *ConstNode;
	ConstNode = VarDeclNode->InsertEndChild(TiXmlElement("is_const"))->ToElement();
	
	if(IsConst)
		ConstNode->InsertEndChild(TiXmlText("true"));
	else
		ConstNode->InsertEndChild(TiXmlText("false"));

	//add the type
	TiXmlElement *TypeNode;
	TypeNode = VarDeclNode->InsertEndChild(TiXmlElement("type"))->ToElement();
	TypeNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(VarType).c_str()));

	//add the initialisation expression (if any)
	if(InitExpression)
	{
		TiXmlElement *InitNode;
		InitNode = VarDeclNode->InsertEndChild(TiXmlElement("initialiser"))->ToElement();
		InitExpression->DumpNode(InitNode);
	}

	VarId->DumpNode(VarDeclNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CVarDeclTreeNode::DestroyNode(void)
{
	if(InitExpression)
	{
		InitExpression->DestroyNode();
		InitExpression = NULL;
	}

	VarId->DestroyNode();
	VarId = NULL;
	delete this;
}
//-------------------------------------------------------------