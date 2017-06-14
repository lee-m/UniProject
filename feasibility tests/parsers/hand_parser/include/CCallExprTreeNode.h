//------------------------------------------------------------------------------------------
// File: CCallExprTreeNode.h
// Desc: Representation of a WHILE statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CCALLEXPRTREENODE_H__
#define __CCALLEXPRTREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1..n Expressions for each parameter
*/
class CCallExprTreeNode : public CParseTreeNode
{
public:
	CCallExprTreeNode(void) {}
	~CCallExprTreeNode(void) {}

	//name of the function we're calling
	string FuncName;

	void DumpNode(TiXmlElement *XmlNode)
	{
		TiXmlElement *CallExprNode;
		CallExprNode = XmlNode->InsertEndChild(TiXmlElement("call_expr"))->ToElement();

		//add the parameters
		TiXmlElement *ParmNode = NULL;
		
		for(unsigned int i = 0; i < Children.size(); i++)
		{
			ParmNode = CallExprNode->InsertEndChild(TiXmlElement("call_expr_parm"))->ToElement();
			Children[i]->DumpNode(ParmNode);
		}
	}
};

#endif