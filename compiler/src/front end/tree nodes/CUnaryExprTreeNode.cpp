//------------------------------------------------------------------------------------------
// File: CUnaryExprTreeNode.cpp
// Desc: Unary expression of the form op Exp where Exp can itself be another expr
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CunaryExprTreeNode.h"

//-------------------------------------------------------------
void CUnaryExprTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	TiXmlElement *ExpNode;
	ExpNode = XmlNode->InsertEndChild(TiXmlElement("unary_expr"))->ToElement();

	//add the operator
	TiXmlElement *OpNode;
	OpNode = ExpNode->InsertEndChild(TiXmlElement("operator"))->ToElement();
	OpNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(Operator).c_str()));

	//add the expr we're negating
	TiXmlElement *LhsNode;
	LhsNode = ExpNode->InsertEndChild(TiXmlElement("expr"))->ToElement();
	GetChild(UNARY_EXPR_EXPR)->DumpAsParseTreeNode(LhsNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CUnaryExprTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	OutFile << CUtilFuncs::TokenTypeToStr(Operator);
	GetChild(UNARY_EXPR_EXPR)->DumpAsMIRTreeNode(OutFile);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CUnaryExprTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//turn statements of the form
	// <op> <term>
	//into
	// tempvar = <term>
	// tempvar = <op> tempvar
	return CTreeNodeFactory::GetSingletonPtr()->BuildUnaryExpression(
		GetChild(UNARY_EXPR_EXPR)->RewriteIntoMIRForm(Stmts), Operator);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CUnaryExprTreeNode::CreateCopy(void)
{
	//create the new unary_expr node
	CTreeNode *NewUnExpr;
	NewUnExpr = CTreeNodeFactory::GetSingletonPtr()->BuildUnaryExpression(
									GetChild(UNARY_EXPR_EXPR)->CreateCopy(), Operator);

	return NewUnExpr;
}
//-------------------------------------------------------------
