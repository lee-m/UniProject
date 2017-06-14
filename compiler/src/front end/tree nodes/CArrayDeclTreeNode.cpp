//------------------------------------------------------------------------------------------
// File: CArrayDeclTreeNode.cpp
// Desc: Representation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CArrayDeclTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CIdentifierTreeNode.h"
#include "CSymbolTable.h"

//-------------------------------------------------------------
void CArrayDeclTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	//dump variable decl
	TiXmlElement *ArrDeclNode;
	ArrDeclNode = XmlNode->InsertEndChild(TiXmlElement("arr_decl"))->ToElement();
	GetChild(ARR_DECL_VAR)->DumpAsParseTreeNode(ArrDeclNode);

	//dump bounds
	TiXmlElement *BoundsNode;
	BoundsNode = ArrDeclNode->InsertEndChild(TiXmlElement("bounds"))->ToElement();
	GetChild(ARR_DECL_BOUNDS)->DumpAsParseTreeNode(BoundsNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CArrayDeclTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	CVarDeclTreeNode *ArrVarDecl = (CVarDeclTreeNode*)GetChild(ARR_DECL_VAR);

	//type and name
	OutFile << CUtilFuncs::TokenTypeToStr(ArrVarDecl->VarType) << " ";
	ArrVarDecl->VarId->DumpAsMIRTreeNode(OutFile);

	//bounds
	OutFile << "[";
	GetChild(ARR_DECL_BOUNDS)->DumpAsMIRTreeNode(OutFile);
	OutFile << "]";
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CArrayDeclTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//add ourselves to the symbol table
	CArrayDeclTreeNode *Copy = (CArrayDeclTreeNode*)CreateCopy();
	CVarDeclTreeNode *ArrVar = (CVarDeclTreeNode*)Copy->GetChild(ARR_DECL_VAR);
	CSymbolTable::GetSingletonPtr()->AddSymbol(ArrVar->VarId->IdName, Copy, 0, ArrVar->VarType);

	return Copy;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CArrayDeclTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildArrayDeclaration(
						GetChild(ARR_DECL_VAR)->CreateCopy(),
						GetChild(ARR_DECL_BOUNDS)->CreateCopy());
}
//-------------------------------------------------------------
