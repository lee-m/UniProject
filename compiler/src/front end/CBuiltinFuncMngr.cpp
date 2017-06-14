//------------------------------------------------------------------------------------------
// File: CBuiltinFuncMngr.h
// Desc: Manager for functions which are part of the std library
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CBuiltinFuncMngr.h"
#include "CFunctionDeclTreeNode.h"
#include "CTreeNodeFactory.h"
#include "CSymbolTable.h"
#include "CParameterListTreeNode.h"

//-------------------------------------------------------------
CBuiltinFuncMngr::~CBuiltinFuncMngr(void)
{
	DestroyBuiltins();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CBuiltinFuncMngr::InitialiseBuiltins(bool InMir)
{
	//save on typing later on
	CTreeNodeFactory *TreeNodeFactory = CTreeNodeFactory::GetSingletonPtr();
	CSymbolTable *SymTable = CSymbolTable::GetSingletonPtr();

	//names of the builtin functions
	string PrintStringName = InMir ? "_print_string" : "print_string";
	string PrintIntegerName = InMir ? "_print_integer" : "print_integer";
	string PrintFloatName = InMir ? "_print_float" : "print_float";
	string PrintBooleanName = InMir ? "_print_boolean" : "print_boolean";

	//print_string
	CParameterListTreeNode *PrintStringParms;
	PrintStringParms = (CParameterListTreeNode*)TreeNodeFactory->BuildParameterList();
	PrintStringParms->AddChild(TreeNodeFactory->BuildParameterDecl("Str", 
		TOKTYPE_STRINGTYPE, true, 0));

	CFunctionDeclTreeNode *PrintStringDecl;
	PrintStringDecl = (CFunctionDeclTreeNode*)TreeNodeFactory->BuildFunctionDecl(
		PrintStringName, 1, PrintStringParms, TOKTYPE_VOID);

	SymTable->AddSymbol(PrintStringName, PrintStringDecl, 0, TOKTYPE_VOID);
	m_BuiltinFnDecls.push_back(PrintStringDecl);

	//print_integer
	CParameterListTreeNode *PrintIntegerParms;
	PrintIntegerParms = (CParameterListTreeNode*)TreeNodeFactory->BuildParameterList();
	PrintIntegerParms->AddChild(TreeNodeFactory->BuildParameterDecl("Val", 
		TOKTYPE_INTEGERTYPE, true, 0));

	CFunctionDeclTreeNode *PrintIntegerDecl;
	PrintIntegerDecl = (CFunctionDeclTreeNode*)TreeNodeFactory->BuildFunctionDecl(
		PrintIntegerName, 1, PrintIntegerParms, TOKTYPE_VOID);

	SymTable->AddSymbol(PrintIntegerName, PrintIntegerDecl, 0, TOKTYPE_VOID);
	m_BuiltinFnDecls.push_back(PrintIntegerDecl);

	//print_float
	CParameterListTreeNode *PrintFloatParms;
	PrintFloatParms = (CParameterListTreeNode*)TreeNodeFactory->BuildParameterList();
	PrintFloatParms->AddChild(TreeNodeFactory->BuildParameterDecl("Val", 
		TOKTYPE_FLOATTYPE, true, 0));

	CFunctionDeclTreeNode *PrintFloatDecl;
	PrintFloatDecl = (CFunctionDeclTreeNode*)TreeNodeFactory->BuildFunctionDecl(
		PrintFloatName, 1, PrintFloatParms, TOKTYPE_VOID);

	SymTable->AddSymbol(PrintFloatName, PrintFloatDecl, 0, TOKTYPE_VOID);
	m_BuiltinFnDecls.push_back(PrintFloatDecl);

	//print_boolean
	CParameterListTreeNode *PrintBooleanParms;
	PrintBooleanParms = (CParameterListTreeNode*)TreeNodeFactory->BuildParameterList();
	PrintBooleanParms->AddChild(TreeNodeFactory->BuildParameterDecl("Val", 
		TOKTYPE_BOOLEANTYPE, true, 0));

	CFunctionDeclTreeNode *PrintBooleanDecl;
	PrintBooleanDecl = (CFunctionDeclTreeNode*)TreeNodeFactory->BuildFunctionDecl(
		PrintBooleanName, 1, PrintBooleanParms, TOKTYPE_VOID);

	SymTable->AddSymbol(PrintBooleanName, PrintBooleanDecl, 0, TOKTYPE_VOID);
	m_BuiltinFnDecls.push_back(PrintBooleanDecl);

		//initialise the builtin functions list
	if(!InMir)
	{
		m_BuiltinFuncs.push_back("print_string");
		m_BuiltinFuncs.push_back("print_integer");
		m_BuiltinFuncs.push_back("print_boolean");
		m_BuiltinFuncs.push_back("print_float");
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CBuiltinFuncMngr::DestroyBuiltins(void)
{
	//release the fn decls
	vector<CTreeNode*>::iterator Itr = m_BuiltinFnDecls.begin();
	for(; Itr != m_BuiltinFnDecls.end(); Itr++)
		(*Itr)->DestroyNode();

	m_BuiltinFuncs.clear();
	m_BuiltinFnDecls.clear();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CBuiltinFuncMngr::IsBuiltinFunction(const string &FuncName)
{
	vector<string>::iterator Itr = find(m_BuiltinFuncs.begin(), m_BuiltinFuncs.end(),
																		FuncName);
	return Itr != m_BuiltinFuncs.end();
}
//-------------------------------------------------------------

//singleton static member definition
template<> CBuiltinFuncMngr* ISingleton<CBuiltinFuncMngr>::ms_Singleton = 0;