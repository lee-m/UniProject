//------------------------------------------------------------------------------------------
// File: CIfStmtTreeNode.h
// Desc: Representation of an IF statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CIFSTMTTREENODE_H__
#define __CIFSTMTTREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1. Conditional expr
	  2. Body of the if statement
*/
class CIfStmtTreeNode : public CParseTreeNode
{
public:
	CIfStmtTreeNode(void) {}
	~CIfStmtTreeNode(void) {}

	void DumpNode(TiXmlElement *XmlNode) 
	{
		TiXmlElement *IfStmtNode;
		IfStmtNode = XmlNode->InsertEndChild(TiXmlElement("if_stmt"))->ToElement();

		//add the conditional expr
		TiXmlElement *CondExprNode;
		CondExprNode = IfStmtNode->InsertEndChild(TiXmlElement("cond_expr"))->ToElement();
		Children[0]->DumpNode(CondExprNode);

		//add the body
		TiXmlElement *StmtBody;
		StmtBody = IfStmtNode->InsertEndChild(TiXmlElement("if_stmt_body"))->ToElement();

		for(unsigned int i = 1; i < Children.size(); i++)
			Children[i]->DumpNode(StmtBody);
	}
};

#endif