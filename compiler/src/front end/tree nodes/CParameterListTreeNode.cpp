//------------------------------------------------------------------------------------------
// File: CParameterListTreeNode.cpp
// Desc: Representation of a function's declared parameters within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CParameterListTreeNode.h"

//-------------------------------------------------------------
void CParameterListTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	for(unsigned int i = 0; i < GetNumChildren(); i++)
		GetChild(i)->DumpAsParseTreeNode(XmlNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParameterListTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	for(unsigned int i = 0; i < GetNumChildren(); i++)
	{
		GetChild(i)->DumpAsMIRTreeNode(OutFile);

		if(i != GetNumChildren() - 1)
			OutFile << ", ";
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParameterListTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//we don't need to rewrite parameter decls into MIR form, we
	//can simply create a copy of them
	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParameterListTreeNode::CreateCopy(void)
{
	//create the plain list node
	CTreeNode *NewParmList;
	NewParmList = CTreeNodeFactory::GetSingletonPtr()->BuildParameterList();

	//copy across the parameters
	for(unsigned int i = 0; i < GetNumChildren(); i++)
		NewParmList->AddChild(GetChild(i)->CreateCopy());

	return NewParmList;
}
//-------------------------------------------------------------
