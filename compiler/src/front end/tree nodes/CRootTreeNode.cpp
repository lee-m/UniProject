//------------------------------------------------------------------------------------------
// File: CRootTreeNode.cpp
// Desc: Root node of the tree.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CRootTreeNode.h"

//-------------------------------------------------------------
void CRootTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	//dump the tree
	for(unsigned int i = 0; i < GetNumChildren(); i++)
		GetChild(i)->DumpAsParseTreeNode(XmlNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CRootTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	for(unsigned int i = 0; i < GetNumChildren(); i++)
	{
		GetChild(i)->DumpAsMIRTreeNode(OutFile);

		//we dont need a semi-colon if we've just dumped a function
		if(GetChild(i)->Code != TC_FUNCTIONDECL)
			OutFile << ";" << endl;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CRootTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	for(unsigned int i = 0; i < GetNumChildren(); i++)
	{
		CTreeNode *NewStmt = GetChild(i)->RewriteIntoMIRForm(Stmts);
		Stmts->AddChild(NewStmt);
	}

	return Stmts;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CRootTreeNode::CreateCopy(void)
{
	CRootTreeNode *NewRoot;
	NewRoot = (CRootTreeNode*)CTreeNodeFactory::GetSingletonPtr()->BuildRootNode();

	//add in each statement
	for(unsigned int i = 0; i < GetNumChildren(); i++)
		NewRoot->AddChild(GetChild(i)->CreateCopy());

	return NewRoot;
}
//-------------------------------------------------------------
