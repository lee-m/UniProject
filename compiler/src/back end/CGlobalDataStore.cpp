//------------------------------------------------------------------------------------------
// File: CGlobalDataStore.h
// Desc: Store global variable decls and string literals which are output to the 
//       .data (str literals) and .bss (global vars) sections
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CGlobalDataStore.h"
#include "CVarDeclTreeNode.h"
#include "CArrayDeclTreeNode.h"
#include "CIdentifierTreeNode.h"
#include "CLiteralTreeNode.h"

//-------------------------------------------------------------
CGlobalDataStore::~CGlobalDataStore(void)
{
	//cleanup our temp global var decls
	list<CTreeNode*>::iterator Itr = m_VarDecls.begin();
	for(; Itr != m_VarDecls.end(); Itr++)
		(*Itr)->DestroyNode();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::AddLiteral(const string &Str, const string &RefName,
																	LiteralType LitType)
{
	if(LitType == LT_STRING)
	{
		if(m_StrLiterals.find(Str) == m_StrLiterals.end())
			m_StrLiterals.insert(make_pair(Str, RefName));
	}
	else
	{
		if(m_FpLiterals.find(Str) == m_FpLiterals.end())
			m_FpLiterals.insert(make_pair(Str, RefName));
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::AddGlobalVar(CTreeNode *VarDecl)
{
	SanityCheck(VarDecl && 
		(VarDecl->Code == TC_VARDECL || VarDecl->Code == TC_ARRAYDECL));

	//HACK: something is clobbering the var decl pointers between adding
	//the decl to the list and actually outputting them so we create a copy
	//to get around this for now.
	m_VarDecls.push_back(VarDecl->CreateCopy());
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::MarkBuiltinFunctionUsed(const string &Name)
{
	if(find(m_BuiltinFuncs.begin(), m_BuiltinFuncs.end(), Name) == m_BuiltinFuncs.end())
		m_BuiltinFuncs.push_back(Name);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CGlobalDataStore::GetStrLitRefNo(const string &Lit)
{
	//see if we've already got this literal value stored
	map<string, string>::iterator LitItr = m_StrLiterals.find(Lit);

	if(LitItr != m_StrLiterals.end())
		return LitItr->second;

	//need to create a new one
	stringstream StrStream;
	StrStream << "str_ref_" << m_NextStrRef++;

	return StrStream.str();
}
//-------------------------------------------------------------
string CGlobalDataStore::GetFpLitRefNo(const string &Lit)
{
	map<string, string>::iterator LitItr = m_FpLiterals.find(Lit);

	if(LitItr != m_FpLiterals.end())
		return LitItr->second;

	stringstream StrStream;
	StrStream << "fp_ref_" << m_NextFpRef++;

	return StrStream.str();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::OutputGlobalVariables(ofstream &FileHandle)
{
	list<CTreeNode*>::iterator Itr = m_VarDecls.begin();
	for(; Itr != m_VarDecls.end(); Itr++)
	{
		if((*Itr)->Code == TC_VARDECL)
			OutputVarDecl((CVarDeclTreeNode*)(*Itr), FileHandle);
		else
			OutputArrayDecl((CArrayDeclTreeNode*)(*Itr), FileHandle);

		FileHandle << endl;
	}

	FileHandle << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::OutputExternDeclarations(ofstream &FileHandle)
{
	for(unsigned int i = 0; i < m_BuiltinFuncs.size(); i++)
		FileHandle << "extern _" << m_BuiltinFuncs[i] << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::OutputLiterals(ofstream &FileHandle)
{
	//do the string literals first
	map<string, string>::iterator LitItr;

	for(LitItr = m_StrLiterals.begin(); LitItr != m_StrLiterals.end(); LitItr++)
	{
		FileHandle << "\t" << LitItr->second << "\tdb ";
		FileHandle << LitItr->first << ",0" << endl;
	}

	//now the fp ones
	for(LitItr = m_FpLiterals.begin(); LitItr != m_FpLiterals.end(); LitItr++)
	{
		FileHandle << "\t" << LitItr->second << "\tdd ";
		FileHandle << LitItr->first << endl;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::OutputVarDecl(CVarDeclTreeNode *VarDecl, ofstream &FileHandle)
{
	//all variables are 4 bytes in size regardless of their type, 
	//even string types since we convert them to a pointer to the string
	//in the .data section when using them
	FileHandle << "\t" << VarDecl->VarId->IdName << "\t resd 1";
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CGlobalDataStore::OutputArrayDecl(CArrayDeclTreeNode *ArrDecl, 
																			 ofstream &FileHandle)
{
	//get the var decl for the array so we can get it's name
	CVarDeclTreeNode *ArrayVar;
	ArrayVar = (CVarDeclTreeNode*)ArrDecl->GetChild(ARR_DECL_VAR);

	//get the index value so we can compute how many bytes are required
	CLiteralTreeNode *IndexVal;
	IndexVal = (CLiteralTreeNode*)ArrDecl->GetChild(ARR_DECL_BOUNDS);

	FileHandle << "\t" << ArrayVar->VarId->IdName << " resd ";
	FileHandle << atoi(IndexVal->LitValue.c_str()) * 4;
}
//-------------------------------------------------------------

//singleton static member definition
template<> CGlobalDataStore* ISingleton<CGlobalDataStore>::ms_Singleton = 0;