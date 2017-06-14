//------------------------------------------------------------------------------------------
// File: CReturnStmtTreeNode.h
// Desc: Representation of a return statement
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CRETURNSTMTTREENODE_H__
#define __CRETURNSTMTTREENODE_H__

#include "CParseTreeNode.h"

/*
  Children:
	  1. Expression forming the value being returned
*/
class CReturnStmtTreeNode : public CParseTreeNode
{
public:
	CReturnStmtTreeNode(void) {}
	~CReturnStmtTreeNode(void) {}

	void DumpNode(TiXmlElement *XmlNode)
	{
	}
};

#endif