//------------------------------------------------------------------------------------------
// File: CUtilFuncs.cpp
// Desc: Various utility functions
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CUtilFuncs.h"
#include "CParser.h"
#include "CControlFlowGraph.h"
#include "CIdentifierTreeNode.h"
#include "CLinearScanAllocator.h"
#include "CSymbolTable.h"
#include "CVarDeclTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CParameterDeclTreeNode.h"
#include "CLiteralRefTreeNode.h"
#include "CCallExprTreeNode.h"
#include "CFunctionDeclTreeNode.h"

//-------------------------------------------------------------
string CUtilFuncs::TokenTypeToStr(TokenType Tok)
{
	switch(Tok)
	{
	case TOKTYPE_INTLITERAL:
	case TOKTYPE_INTEGERTYPE:
		return "integer";

	case TOKTYPE_FLOATLITERAL:
	case TOKTYPE_FLOATTYPE:
		return "float";

	case TOKTYPE_BOOLEANTYPE:
		return "boolean";

	case TOKTYPE_STRINGLITERAL:
	case TOKTYPE_STRINGTYPE:
		return "string";

	case TOKTYPE_CONST:
		return "const";

	case TOKTYPE_VOID:
		return "void";

	case TOKTYPE_IF:
		return "if";

	case TOKTYPE_ENDIF:
		return "end_if";

	case TOKTYPE_WHILE:
		return "while";

	case TOKTYPE_END_WHILE:
		return "end while";

	case TOKTYPE_FUNCTION:
		return "function";

	case TOKTYPE_RETURNS:
		return "returns";

	case TOKTYPE_RETURN:
		return "return";

	case TOKTYPE_END_FUNCTION:
		return "end_function";

	case TOKTYPE_TRUE:
		return "true";

	case TOKTYPE_FALSE:
		return "false";

	case TOKTYPE_ASSIGNMENT:
		return "assignment";

	case TOKTYPE_OPENPAREN:
		return "(";

	case TOKTYPE_CLOSEPAREN:
		return ")";

	case TOKTYPE_OPENSQRPAREN:
		return "[";

	case TOKTYPE_CLOSESQRPAREN:
		return "]";

	case TOKTYPE_SEMICOLON:
		return ";";

	case TOKTYPE_COMMA:
		return ",";

	case TOKTYPE_LESSTHAN:
		return "<";

	case TOKTYPE_LESSTHANEQ:
		return "<=";

	case TOKTYPE_GREATTHAN:
		return ">";

	case TOKTYPE_GREATTHANEQ:
		return ">=";

	case TOKTYPE_NOT:
		return "!";

	case TOKTYPE_NOTEQ:
		return "!=";

	case TOKTYPE_EQUALITY:
		return "==";

	case TOKTYPE_OR:
		return "or";

	case TOKTYPE_AND:
		return "and";

	case TOKTYPE_ID:
		return "identifier";

	case TOKTYPE_PLUS:
		return "+";

	case TOKTYPE_MINUS:
		return "-";

	case TOKTYPE_MULTIPLY:
		return "*";

	case TOKTYPE_DIVIDE:
		return "/";

	default:
		InternalError("unknown token type");
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CUtilFuncs::ConvertExprContextToTokenType(ExprContext Context)
{
	switch(Context)
	{
	case EC_INITIALISER:
		return TOKTYPE_SEMICOLON;

	case EC_ARRAY:
		return TOKTYPE_CLOSESQRPAREN;

	case EC_FUNCCALLPARM:
		return TOKTYPE_COMMA;

	case EC_CONDITIONAL:
		return TOKTYPE_CLOSEPAREN;

	case EC_FUNCCALLPARMS:
		return TOKTYPE_COMMA;
	}

	InternalError("unknown expr context type");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CUtilFuncs::GenCompilerTempName(void)
{
	static int TempNum = 0;
	stringstream RetStr;

	RetStr << "CompTemp." << TempNum++;
	return RetStr.str();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CUtilFuncs::GenNewLabel(void)
{
	static int LabelNum = 0;
	stringstream RetStr;

	RetStr << "L" << LabelNum++;
	return RetStr.str();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CUtilFuncs::HardRegisterToStr(HardRegister Reg)
{
	switch(Reg)
	{
	case REG_EAX:
		return  "eax";
			
	case REG_EBX:
		return "ebx";

	case REG_ECX:
		return "ecx";
			
	case REG_EDX:
		return "edx";

	default:
		InternalError("unreachable reg no");
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CUtilFuncs::GetDeclFromNode(CTreeNode *Node)
{
	switch(Node->Code)
	{
	case TC_ARRAY_REF:
		{
			//we don't return an array_decl for array references as this
			//looses all the information we need to compute the memory address
			//needed later on. To make matters easier we just return the 
			//array reference passed into us
			return Node;
		}
	case TC_IDENTIFIER:
		{
			//look the identifier up in the symbol table
			Symbol_t *Sym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(
				((CIdentifierTreeNode*)Node)->IdName, false);
			
			SanityCheck(Sym);
			return Sym->IdPtr;
		}

	case TC_CALLEXPR:
		{
			CCallExprTreeNode *CallExpr = (CCallExprTreeNode*)Node;

			Symbol_t *FuncSym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(
				CallExpr->FuncName, false);
			SanityCheck(FuncSym);

			return FuncSym->IdPtr;
		}

	case TC_PARMDECL:
			return GetDeclFromNode(Node->GetChild(PARM_DECL_ID));

	case TC_LITERAL:
		return Node;
	}

	InternalError("unhandled decl type");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CUtilFuncs::ExprRefersToFloatingPointType(CTreeNode *Expr)
{
	switch(Expr->Code)
	{
	case TC_BOOLEAN_EXPR:
		{
			return ExprRefersToFloatingPointType(Expr->GetChild(BLN_EXPR_LHS))
				|| ExprRefersToFloatingPointType(Expr->GetChild(BLN_EXPR_RHS));
		}
	case TC_VARDECL:
		{
			CVarDeclTreeNode *Var = (CVarDeclTreeNode*)Expr;
			return Var->VarType == TOKTYPE_FLOATTYPE;
		}

	case TC_ARRAYDECL:
		return ExprRefersToFloatingPointType(Expr->GetChild(ARR_DECL_VAR));

	case TC_ARRAY_REF:
		{
			CArrayRefTreeNode *Ref = (CArrayRefTreeNode*)Expr;
			return ExprRefersToFloatingPointType(GetDeclFromNode(Ref->ArrName));
		}

	case TC_ARITH_EXPR:
		{
			return ExprRefersToFloatingPointType(Expr->GetChild(ASSIGN_EXPR_ID))
				|| ExprRefersToFloatingPointType(Expr->GetChild(ASSIGN_EXPR_RHS));
		}

	case TC_IDENTIFIER:
		return ExprRefersToFloatingPointType(GetDeclFromNode(Expr));

	case TC_LITERAL:
		{
			CLiteralTreeNode *Lit = (CLiteralTreeNode*)Expr;
			return Lit->LitType == TOKTYPE_FLOATLITERAL;
		}

	case TC_PARMDECL:
		{
			CParameterDeclTreeNode *Parm = (CParameterDeclTreeNode*)Expr;
			return Parm->Type == TOKTYPE_FLOATTYPE;
		}

	case TC_LITERALREF:
		{
			CLiteralRefTreeNode *LitRef = (CLiteralRefTreeNode*)Expr;
			return ExprRefersToFloatingPointType(LitRef->GetChild(LITREF_LITERAL));
		}

	case TC_CALLEXPR:
		{
			CCallExprTreeNode *CallExpr = (CCallExprTreeNode*)Expr;
			
			//lookup the function
			Symbol_t *Sym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(
				CallExpr->FuncName, false);
			SanityCheck(Sym);

			//check the return type of the function
			CFunctionDeclTreeNode *FnDecl = (CFunctionDeclTreeNode*)Sym->IdPtr;
			return FnDecl->RetType == TOKTYPE_FLOATTYPE;
		}
	}

	InternalError("unhandled expr type");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CompBasicBlocks::operator () (const BasicBlock_t *BbOne, 
																	 const BasicBlock_t *BbTwo) const
{
	return BbOne->BbId < BbTwo->BbId;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CompIdentifiers::operator () (const CIdentifierTreeNode *IdOne, 
																	 const CIdentifierTreeNode *IdTwo) const
{
	return strcmp(IdOne->IdName.c_str(), IdTwo->IdName.c_str()) < 0;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CompLiveIntervals::operator ()(const LiveInterval_t &LiOne, 
																		const LiveInterval_t &LiTwo) const
{
	return LiOne.EndPoint < LiTwo.EndPoint;
}
//-------------------------------------------------------------
