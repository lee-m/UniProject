//------------------------------------------------------------------------------------------
// File: CFunctionDefTreeNode.cpp
// Desc: Representation of a function within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CFunctionDeclTreeNode.h"
#include "CControlFlowGraph.h"
#include "CSymbolTable.h"
#include "CParameterDeclTreeNode.h"
#include "CIdentifierTreeNode.h"

//-------------------------------------------------------------
CFunctionDeclTreeNode::CFunctionDeclTreeNode(void)
{
	RetType = TOKTYPE_VOID;

	NumParameters = 0;
	StackFrameSize = 0;

	Name = "";
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CFunctionDeclTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	TiXmlElement *FnDeclNode;
	FnDeclNode = XmlNode->InsertEndChild(TiXmlElement("function_decl"))->ToElement();

	//name of the function
	TiXmlElement *NameNode;
	NameNode = FnDeclNode->InsertEndChild(TiXmlElement("name"))->ToElement();
	NameNode->InsertEndChild(TiXmlText(Name.c_str()));

	//return type
	TiXmlElement *RetTypeNode;
	RetTypeNode = FnDeclNode->InsertEndChild(TiXmlElement("return_type"))->ToElement();
	RetTypeNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(RetType).c_str()));

	//parameter list
	TiXmlElement *ParmListNode;
	ParmListNode = FnDeclNode->InsertEndChild(TiXmlElement("parameter_list"))->ToElement();
	GetChild(FUNC_DECL_PARM_LIST)->DumpAsParseTreeNode(ParmListNode);

	//function body
	TiXmlElement *StmtBody;
	StmtBody = FnDeclNode->InsertEndChild(TiXmlElement("function_decl_body"))->ToElement();

	for(unsigned int i = 1; i < GetNumChildren(); i++)
		GetChild(i)->DumpAsParseTreeNode(StmtBody);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CFunctionDeclTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//construct a new function decl. We append an underscore to the 
	//function name to avoid any clashes with functions that share a 
	//name with assembly instructions such as "test"
	CTreeNodeFactory *ParseTreeBuilder = CTreeNodeFactory::GetSingletonPtr();
	CTreeNode *NewMirFuncDecl = ParseTreeBuilder->BuildFunctionDecl("_" + Name, 
		NumParameters, GetChild(FUNC_DECL_PARM_LIST)->CreateCopy(), RetType);

	//add an entry for this function in the symbol table
	CSymbolTable::GetSingletonPtr()->AddSymbol("_" + Name, NewMirFuncDecl, 0, RetType);

	//add each parameter to the symbol table
	for(unsigned int i = 0; i < GetChild(FUNC_DECL_PARM_LIST)->GetNumChildren(); i++)
	{
		CParameterDeclTreeNode *Parm;
		CIdentifierTreeNode *ParmName;

		Parm = (CParameterDeclTreeNode*)NewMirFuncDecl->GetChild(FUNC_DECL_PARM_LIST)->GetChild(i);
		ParmName = (CIdentifierTreeNode*)Parm->GetChild(PARM_DECL_ID);

		CSymbolTable::GetSingletonPtr()->AddSymbol(ParmName->IdName, Parm, 0, Parm->Type);
	}

	//update the stack frame size according to the number of 
	//parameters this function has
	StackFrameSize += 4 * GetChild(FUNC_DECL_PARM_LIST)->GetNumChildren();
	SpillVarsOffset = StackFrameSize;

	//go through each statement in the function and convert it, using the newly
	//created function decl as the parent for adding the statements to
	for(unsigned int i = 1; i < GetNumChildren(); i++)
	{
		CTreeNode *FuncBodyStmt = GetChild(i)->RewriteIntoMIRForm(NewMirFuncDecl);

		if(FuncBodyStmt)
			NewMirFuncDecl->AddChild(FuncBodyStmt);
	}

	return NewMirFuncDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CFunctionDeclTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	//dump the function signature
	OutFile << "function " << Name << "(";
	GetChild(FUNC_DECL_PARM_LIST)->DumpAsMIRTreeNode(OutFile);
	OutFile << ") returns " << CUtilFuncs::TokenTypeToStr(RetType) << endl;

	//dump the body
	for(unsigned int i = 1; i < GetNumChildren(); i++)
	{
		OutFile << "  ";
		GetChild(i)->DumpAsMIRTreeNode(OutFile);

		//we dont need a semi-colon if we've just dumped a label
		if(GetChild(i)->Code != TC_LABEL)
			OutFile << ";" << endl;
	}

	OutFile << "end_function" << endl << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CFunctionDeclTreeNode::CreateCopy(void)
{
	//create the bare bones function decl
	CTreeNode *NewFnDecl;
	NewFnDecl = CTreeNodeFactory::GetSingletonPtr()->BuildFunctionDecl(
		Name, NumParameters, GetChild(FUNC_DECL_PARM_LIST)->CreateCopy(), RetType);

	//copy the statements
	for(unsigned int i = 1; i < GetNumChildren(); i++)
		NewFnDecl->AddChild(GetChild(i)->CreateCopy());

	return NewFnDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CFunctionDeclTreeNode::DestroyNode()
{
	//clean up the CFG
	list<BasicBlock_t*>::iterator itr = FuncCFG.begin();
	for(; itr != FuncCFG.end(); itr++)
		delete (*itr);

	//clean up our children
	CTreeNode::DestroyNode();
}
//-------------------------------------------------------------
