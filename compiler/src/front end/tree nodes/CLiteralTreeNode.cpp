//------------------------------------------------------------------------------------------
// File: CLiteralTreeNode.cpp
// Desc: A literal value representation.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CLiteralTreeNode.h"
#include "CGlobalDataStore.h"

//-------------------------------------------------------------
void CLiteralTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	TiXmlElement *LitNode;
	LitNode = XmlNode->InsertEndChild(TiXmlElement("literal_node"))->ToElement();

	//value
	TiXmlElement *ValNode;
	ValNode = LitNode->InsertEndChild(TiXmlElement("value"))->ToElement();
	ValNode->InsertEndChild(TiXmlText(LitValue.c_str()));

	//type
	TiXmlElement *TypeNode;
	TypeNode = LitNode->InsertEndChild(TiXmlElement("type"))->ToElement();
	TypeNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(LitType).c_str()));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLiteralTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	OutFile << LitValue;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CLiteralTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	if(LitType == TOKTYPE_FLOATLITERAL)
	{
		string LitRefNo = CGlobalDataStore::GetSingletonPtr()->GetFpLitRefNo(LitValue);
		CGlobalDataStore::GetSingletonPtr()->AddLiteral(LitValue, LitRefNo, 
			LT_FLOATING_POINT);

		return CTreeNodeFactory::GetSingletonPtr()->BuildLiteralRef(
			CreateCopy(), LitRefNo);
	}

	if(LitType == TOKTYPE_STRINGLITERAL)
	{
		string RefNo = CGlobalDataStore::GetSingletonPtr()->GetStrLitRefNo(LitValue);
		CGlobalDataStore::GetSingletonPtr()->AddLiteral(LitValue, RefNo, 
			LT_STRING);

		return CTreeNodeFactory::GetSingletonPtr()->BuildLiteralRef(
			CreateCopy(), RefNo);
	}

	//when rewriting into MIR form we want to turn Boolean literals
	//into 0's and 1's
	if(LitValue == "true")
		return CTreeNodeFactory::GetSingletonPtr()->BuildLiteral("1", TOKTYPE_INTEGERTYPE);
	else if(LitValue == "false")
		return CTreeNodeFactory::GetSingletonPtr()->BuildLiteral("0", TOKTYPE_INTEGERTYPE);

	//some other literal
	return CreateCopy();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CLiteralTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildLiteral(LitValue, LitType);
}
//-------------------------------------------------------------
