//------------------------------------------------------------------------------------------
// File: CWhileStmtTreeNode.h
// Desc: Representation of a WHILE statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CWHILESTMTTREENODE_H__
#define __CWHILESTMTTREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1. Conditional expr
	  2. Body of the while statement
*/
class CWhileStmtTreeNode : public CParseTreeNode
{
public:
	CWhileStmtTreeNode(void) {}
	~CWhileStmtTreeNode(void) {}

	void DumpNode(TiXmlElement *XmlNode) 
	{
		TiXmlElement *IfStmtNode;
		IfStmtNode = XmlNode->InsertEndChild(TiXmlElement("while_stmt"))->ToElement();

		//add the conditional expr
		TiXmlElement *CondExprNode;
		CondExprNode = IfStmtNode->InsertEndChild(TiXmlElement("cond_expr"))->ToElement();
		(*Children.begin())->DumpNode(CondExprNode);

		//add the body
		TiXmlElement *StmtBody;
		StmtBody = IfStmtNode->InsertEndChild(TiXmlElement("while_stmt_body"))->ToElement();

		for(unsigned int i = 1; i < Children.size(); i++)
			Children[i]->DumpNode(StmtBody);
	}
};

#endif