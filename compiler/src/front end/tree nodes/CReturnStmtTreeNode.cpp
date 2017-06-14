//------------------------------------------------------------------------------------------
// File: CReturnStmtTreeNode.cpp
// Desc: Representation of a return statement
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CReturnStmtTreeNode.h"

//-------------------------------------------------------------
void CReturnStmtTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);
	TiXmlElement *RetStmtNode;
	RetStmtNode = XmlNode->InsertEndChild(TiXmlElement("return_stmt"))->ToElement();
	GetChild(RET_STMT_RET_EXPR)->DumpAsParseTreeNode(RetStmtNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CReturnStmtTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	OutFile << "return ";
	GetChild(RET_STMT_RET_EXPR)->DumpAsMIRTreeNode(OutFile);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CReturnStmtTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//simplify the return expr, storing the last stmt it created
	CTreeNode *SimpRetExpr;
	CTreeNode *NewRetExpr;

	SimpRetExpr = GetChild(RET_STMT_RET_EXPR)->RewriteIntoMIRForm(Stmts);
	NewRetExpr = CTreeNodeFactory::GetSingletonPtr()->BuildReturnStmt(SimpRetExpr);

	//add the new stmt and return it
	//Stmts->AddChild(NewRetExpr);
	return NewRetExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CReturnStmtTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildReturnStmt(
						GetChild(RET_STMT_RET_EXPR)->CreateCopy());
}
//-------------------------------------------------------------
