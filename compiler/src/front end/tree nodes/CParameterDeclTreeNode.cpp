//------------------------------------------------------------------------------------------
// File: CParameterDeclTreeNode.cpp
// Desc: Representation of a parameter declaration
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CParameterDeclTreeNode.h"
#include "CIdentifierTreeNode.h"

//-------------------------------------------------------------
void CParameterDeclTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	TiXmlElement *ParmNode;
	ParmNode = XmlNode->InsertEndChild(TiXmlElement("parm_decl"))->ToElement();

	//name
	TiXmlElement *NameNode;
	CIdentifierTreeNode *ParmName;
	ParmName = (CIdentifierTreeNode*)GetChild(PARM_DECL_ID);
	NameNode = ParmNode->InsertEndChild(TiXmlElement("name"))->ToElement();
	NameNode->InsertEndChild(TiXmlText(ParmName->IdName.c_str()));

	//type
	TiXmlElement *TypeNode;
	TypeNode = ParmNode->InsertEndChild(TiXmlElement("type"))->ToElement();
	TypeNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(Type).c_str()));

	//is const or not
	TiXmlElement *ConstNode;
	ConstNode = ParmNode->InsertEndChild(TiXmlElement("is_const"))->ToElement();
	ConstNode->InsertEndChild(TiXmlText(IsConst ? "true" : "false"));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParameterDeclTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	CIdentifierTreeNode *Name = (CIdentifierTreeNode*)GetChild(PARM_DECL_ID);
	OutFile << CUtilFuncs::TokenTypeToStr(Type) << " " << Name->IdName;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParameterDeclTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//we don't need to rewrite parameter decls into MIR form, we
	//can simply create a copy of it
	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParameterDeclTreeNode::CreateCopy(void)
{
	CIdentifierTreeNode *Name = (CIdentifierTreeNode*)GetChild(PARM_DECL_ID);
	return CTreeNodeFactory::GetSingletonPtr()->BuildParameterDecl(
		Name->IdName, Type, IsConst, Position);
}
//-------------------------------------------------------------