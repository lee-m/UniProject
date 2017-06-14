//------------------------------------------------------------------------------------------
// File: CBinaryExprTreeNode.cpp
// Desc: Parse tree node for a binary expression
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CBinaryExprTreeNode.h"
#include "CUtilFuncs.h"

//-------------------------------------------------------------
void CBinaryExprTreeNode::DumpNode(TiXmlElement *XmlNode)
{
	assert(XmlNode);

	TiXmlElement *ExpNode;
	ExpNode = XmlNode->InsertEndChild(TiXmlElement("binary_expr"))->ToElement();

	//add the operator
	TiXmlElement *OpNode;
	OpNode = ExpNode->InsertEndChild(TiXmlElement("operator"))->ToElement();
	OpNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(Operator).c_str()));

	//add the lhs
	TiXmlElement *LhsNode;
	LhsNode = ExpNode->InsertEndChild(TiXmlElement("lhs"))->ToElement();
	Children[0]->DumpNode(LhsNode);

	//finally the rhs
	TiXmlElement *RhsNode;
	RhsNode = ExpNode->InsertEndChild(TiXmlElement("rhs"))->ToElement();
	Children[1]->DumpNode(RhsNode);
}
//-------------------------------------------------------------