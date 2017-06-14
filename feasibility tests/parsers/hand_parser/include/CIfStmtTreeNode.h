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
		3. Body of the else statement attached to this if
*/
class CIfStmtTreeNode : public CParseTreeNode
{
public:
	CIfStmtTreeNode(void) {}
	~CIfStmtTreeNode(void) {}

	//inherited from CParseTreeNode
	void DumpNode(TiXmlElement *XmlNode) 
	{
		TiXmlElement *IfStmtNode;
		IfStmtNode = XmlNode->InsertEndChild(TiXmlElement("if_stmt"))->ToElement();

		//add the conditional expr
		TiXmlElement *CondExprNode;
		CondExprNode = IfStmtNode->InsertEndChild(TiXmlElement("cond_expr"))->ToElement();
		Children[0]->DumpNode(CondExprNode);

		//add the if stmt body
		TiXmlElement *IfStmtBody;
		IfStmtBody = IfStmtNode->InsertEndChild(TiXmlElement("if_stmt_body"))->ToElement();

		for(unsigned int i = 0; i < Children[1]->Children.size(); i++)
			Children[1]->Children[i]->DumpNode(IfStmtBody);

		//add the else stmt body
		TiXmlElement *ElseStmtBody;
		ElseStmtBody = IfStmtNode->InsertEndChild(TiXmlElement("else_stmt_body"))->ToElement();

		for(unsigned int i = 0; i < Children[2]->Children.size(); i++)
			Children[2]->Children[i]->DumpNode(ElseStmtBody);
	}

};

#endif