//------------------------------------------------------------------------------------------
// File: CIdentifierTreeNode.cpp
// Desc: Representation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CIdentifierTreeNode.h"

//-------------------------------------------------------------
void CIdentifierTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	TiXmlElement *IdNode;
	IdNode = XmlNode->InsertEndChild(TiXmlElement("identifier"))->ToElement();
	IdNode->InsertEndChild(TiXmlText(IdName.c_str()));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CIdentifierTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	OutFile << IdName;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CIdentifierTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	return CreateCopy();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CIdentifierTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildIdentifier(IdName);
}
//-------------------------------------------------------------
