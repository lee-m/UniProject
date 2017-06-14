//------------------------------------------------------------------------------------------
// File: CParseTreeNode.h
// Desc: Encapsulation of the parse tree built by the parser
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARSETREENODE_H__
#define __CPARSETREENODE_H__

#include "tinyxml/tinyxml.h"
#include <vector>
using namespace std;

//the different types of tree
enum TreeCode
{
	TC_IDENTIFIER = 0,
	TC_VARDECL,
	TC_ARRAYDECL,
	TC_LITERAL,
	TC_BINARY_EXPR,
	TC_UNARY_EXPR,
	TC_ARRAY_REF,
	TC_IFSTMT,
	TC_WHILESTMT,
	TC_ASSIGN_EXPR,
	TC_PARAMETER_LIST,
	TC_FUNCTIONDECL,
	TC_PARMDECL,
	TC_RETURNSTMT,
	TC_CALLEXPR,
	TC_ROOT
};

class CParseTreeNode
{
public:
	CParseTreeNode(void) {}
	virtual ~CParseTreeNode(void) {}

	//what type of tree this is
	TreeCode Code;

	//any child nodes
	vector<CParseTreeNode*> Children;

	//save this node to file
	virtual void DumpNode(TiXmlElement *XmlNode) = 0;

	//destroy this node and any children
	virtual void DestroyNode(void)
	{
		vector<CParseTreeNode*>::iterator itr = Children.begin();

		for(; itr != Children.end(); itr++)
		{
			if(*itr)
			{
				(*itr)->DestroyNode();
				(*itr) = NULL;
			}
		}

		delete this;
	}
};

//helper typedef just in case the underlying data structure to store
//the children changes
typedef vector<CParseTreeNode*>::iterator ParseTreeChildItr;

#endif