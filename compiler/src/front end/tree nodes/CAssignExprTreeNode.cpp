//------------------------------------------------------------------------------------------
// File: CAssignExprTreeNode.cpp
// Desc: Representation of an assignment expression within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CAssignExprTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CIdentifierTreeNode.h"

//-------------------------------------------------------------
void CAssignExprTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	TiXmlElement *ExpNode;
	ExpNode = XmlNode->InsertEndChild(TiXmlElement("assign_expr"))->ToElement();

	//add the lhs
	TiXmlElement *LhsNode;
	LhsNode = ExpNode->InsertEndChild(TiXmlElement("lhs"))->ToElement();
	GetChild(ASSIGN_EXPR_ID)->DumpAsParseTreeNode(LhsNode);

	//add the rhs
	TiXmlElement *RhsNode;
	RhsNode = ExpNode->InsertEndChild(TiXmlElement("rhs"))->ToElement();
	GetChild(ASSIGN_EXPR_RHS)->DumpAsParseTreeNode(RhsNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CAssignExprTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	//dump the variable we're assigning to
	GetChild(ASSIGN_EXPR_ID)->DumpAsMIRTreeNode(OutFile);
	OutFile << " = ";

	//dump the expression that forms the value being assigned
	GetChild(ASSIGN_EXPR_RHS)->DumpAsMIRTreeNode(OutFile);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CAssignExprTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	CTreeNode *Lhs = NULL;

	//replace "arr[a + b] = val" with "temp = a + b; arr[temp] = val"
	if(GetChild(ASSIGN_EXPR_ID)->Code == TC_ARRAY_REF)
	{
		//need to simplify the index expr used to form the array reference
		CArrayRefTreeNode *ArrRef = (CArrayRefTreeNode*)GetChild(ASSIGN_EXPR_ID);
		CTreeNode *NewIndex = ArrRef->GetChild(ARR_REF_INDEX)->RewriteIntoMIRForm(Stmts);

		//build a new arr_ref
		Lhs = CTreeNodeFactory::GetSingletonPtr()->BuildArrayRef(NewIndex, 
			ArrRef->ArrName->IdName, ArrRef->ArrType);
	}
	else
		Lhs = GetChild(ASSIGN_EXPR_ID)->CreateCopy();

	CTreeNode *NewRhs = GetChild(ASSIGN_EXPR_RHS)->RewriteIntoMIRForm(Stmts);
	CTreeNode *RetExpr = CTreeNodeFactory::GetSingletonPtr()->BuildAssignmentExpr(
																Lhs, NewRhs);
	return RetExpr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CAssignExprTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildAssignmentExpr(
		GetChild(ASSIGN_EXPR_ID)->CreateCopy(),
		GetChild(ASSIGN_EXPR_RHS)->CreateCopy());
}
//-------------------------------------------------------------
