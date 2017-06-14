//------------------------------------------------------------------------------------------
// File: CUnaryExprTreeNode.h
// Desc: Unary expression of the form op Exp where Exp can itself be another expr
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CUNARYEXPRTREENODE_H__
#define __CUNARYEXPRTREENODE_H__

#include "CParseTreeNode.h"
#include "TokenTypes.h"

/*
  Children:
	  1. Expression the operator acts upon
*/

class CUnaryExprTreeNode : public CParseTreeNode
{
public:
	CUnaryExprTreeNode(void) {}
	~CUnaryExprTreeNode(void) {}

	//operator for this expression
	TokenType Operator;

	void DumpNode(TiXmlElement *XmlNode) 
	{
		TiXmlElement *ExpNode;
		ExpNode = XmlNode->InsertEndChild(TiXmlElement("unary_expr"))->ToElement();

		//add the operator
		TiXmlElement *OpNode;
		OpNode = ExpNode->InsertEndChild(TiXmlElement("operator"))->ToElement();
		OpNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(Operator).c_str()));

		//add the expr we're negating
		TiXmlElement *LhsNode;
		LhsNode = ExpNode->InsertEndChild(TiXmlElement("expr"))->ToElement();
		Children[0]->DumpNode(LhsNode);
	}
};

#endif