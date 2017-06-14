//------------------------------------------------------------------------------------------
// File: CBinaryExprTreeNode.h
// Desc: Binary expression of the form Exp op Exp where Exp can itself be another expr
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CBINARYEXPRTREENODE_H__
#define __CBINARYEXPRTREENODE_H__

#include "CParseTreeNode.h"
#include "TokenTypes.h"

/*
  Children:
	  1. LHS of the expression
		2. RHS of the expression
*/

class CBinaryExprTreeNode : public CParseTreeNode
{
public:
	CBinaryExprTreeNode(void) {}
	~CBinaryExprTreeNode(void) {}

	//operator for this expression
	TokenType Operator;

	//inherited from CParseTreeNode
	void DumpNode(TiXmlElement *XmlNode);
};

#endif