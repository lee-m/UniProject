//------------------------------------------------------------------------------------------
// File: CVarDeclTreeNode.h
// Desc: Variable declaration within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CVARDECLTREENODE_H__
#define __CVARDECLTREENODE_H__

#include "CParseTreeNode.h"
#include "CIdentifierTreeNode.h"
#include "TokenTypes.h"

class CVarDeclTreeNode : public CParseTreeNode
{
public:
	CVarDeclTreeNode(void) 
	{
		VarId = NULL;
		InitExpression = NULL;
	}

	~CVarDeclTreeNode(void)	{}

	//identifier for this declaration
	CIdentifierTreeNode *VarId;

	//the expression which this variable is initialised with, if the variable
	//isn't initialised at the point of declaration this field will be NULL
	CParseTreeNode *InitExpression;

	//tree if it's const qualified
	bool IsConst;

	//the underlying type of the variable
	TokenType VarType;

	//dump this node to an XML file
	void DumpNode(TiXmlElement *XmlNode);

	//destroy this node and any children
	void DestroyNode(void);
};

#endif