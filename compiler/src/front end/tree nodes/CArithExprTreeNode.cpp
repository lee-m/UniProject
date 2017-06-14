//------------------------------------------------------------------------------------------
// File: CArithExprTreeNode.cpp
// Desc: Parse tree node for a binary arithmetic expression using integral
//       or floating point operands
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CArithExprTreeNode.h"
#include "CLiteralTreeNode.h"
#include "CSymbolTable.h"
#include "CIdentifierTreeNode.h"
#include "CCallExprTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CUnaryExprTreeNode.h"
#include "CFunctionDeclTreeNode.h"
#include "CLiteralRefTreeNode.h"
#include "CArrayDeclTreeNode.h"

//-------------------------------------------------------------
void CArithExprTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	TiXmlElement *ExpNode;
	ExpNode = XmlNode->InsertEndChild(TiXmlElement("arith_expr"))->ToElement();

	//add the operator
	TiXmlElement *OpNode;
	OpNode = ExpNode->InsertEndChild(TiXmlElement("operator"))->ToElement();
	OpNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(Operator).c_str()));

	//add the lhs
	TiXmlElement *LhsNode;
	LhsNode = ExpNode->InsertEndChild(TiXmlElement("lhs"))->ToElement();
	GetChild(ARITH_EXPR_LHS)->DumpAsParseTreeNode(LhsNode);

	//finally the rhs
	TiXmlElement *RhsNode;
	RhsNode = ExpNode->InsertEndChild(TiXmlElement("rhs"))->ToElement();
	GetChild(ARITH_EXPR_RHS)->DumpAsParseTreeNode(RhsNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CArithExprTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	GetChild(ARITH_EXPR_LHS)->DumpAsMIRTreeNode(OutFile);
	OutFile << " " << CUtilFuncs::TokenTypeToStr(Operator) << " ";
	GetChild(ARITH_EXPR_RHS)->DumpAsMIRTreeNode(OutFile);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CArithExprTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//in this function we turn statements of the form:
	//   integer a = 65 * 43;
	//into:
	//   integer a = 65;
	//   a = a * 43;
	//we do it this was it maps nicely onto the equilavent
	//x86 instruction sequence:
	//   mov eax, 65
	//   imul eax, 43

	//simplify each side of the expression
	CTreeNode *LhsNode = GetChild(ARITH_EXPR_LHS)->RewriteIntoMIRForm(Stmts);
	LhsNode = ReplaceArrayRefs(ReplaceCallExpr(LhsNode, Stmts), Stmts);

	CTreeNode *RhsNode = GetChild(ARITH_EXPR_RHS)->RewriteIntoMIRForm(Stmts);
	RhsNode = ReplaceArrayRefs(ReplaceCallExpr(RhsNode, Stmts), Stmts);

	CTreeNodeFactory *TreeFactory = CTreeNodeFactory::GetSingletonPtr();
	CSymbolTable *SymTable = CSymbolTable::GetSingletonPtr();

	//generate a variable name for this temp var that will hold the
	//result of this expression
	string TempVarName = CUtilFuncs::GenCompilerTempName();

	//the simplification of the LHS either produces an identifier node,
	//or a literal value. In either case we need to extract the type so
	//we can construct a temporary variable of the correct type
	TokenType TempVarType = DetermineTempVarType(LhsNode);
	CIdentifierTreeNode *TempLhsId = NULL;

	//if we have an identifier already on the LHS, we dont need to create a temporary
	//variable. This cuts down on the number of temporaries required during conversion
	//and reduces register pressure within the basic block the expression appears in.
	//For a sequence of statements like a + b + c it also makes code generation easier
	//since it exposes to the code generator the fact that all the operations can use 
	//the same hard register
	if(0 && LhsNode->Code == TC_IDENTIFIER)
	{
		TempLhsId = (CIdentifierTreeNode*)LhsNode->CreateCopy();
		LhsNode->DestroyNode();
	}
	else
	{
		//create the var_decl, using the same type as the variable
		CVarDeclTreeNode *TempVar = (CVarDeclTreeNode*)TreeFactory->BuildVarDecl(
			TempVarName, false, TempVarType);

		//assign the value computed from the simplification of LHS of this expression
		//to this temporary variable, adding it to the MIR tree
		CTreeNode *AssignExpr = TreeFactory->BuildAssignmentExpr(
			TempVar->VarId->CreateCopy(), LhsNode);

		Stmts->AddChild(TempVar);
		Stmts->AddChild(AssignExpr);

		SymTable->AddSymbol(TempVar->VarId->IdName, TempVar, 0, TempVar->VarType);
		TempLhsId = (CIdentifierTreeNode*)TempVar->VarId->CreateCopy();
	}

	//now we generate an assignment expression to the temporary variable created above,
	//we use the temp var on the LHS of the expression so the final statement will be 
	//of the form: a = <literal>; a = a [op] <literal>
	CTreeNode *AssignInitExpr = TreeFactory->BuildArithExpression(
		TempLhsId, RhsNode, Operator);

	CTreeNode *AssignExpr = TreeFactory->BuildAssignmentExpr(
		TempLhsId->CreateCopy(), AssignInitExpr);

	//add the assignment statement to the MIR tree
	Stmts->AddChild(AssignExpr);

	//there's little use in simply returning an assignment expression so instead
	//return the LHS which is the identifier holding the value
	return AssignExpr->GetChild(ASSIGN_EXPR_ID)->CreateCopy();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CArithExprTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildArithExpression(
						GetChild(ARITH_EXPR_LHS)->CreateCopy(),
						GetChild(ARITH_EXPR_RHS)->CreateCopy(),
						Operator);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CArithExprTreeNode::DetermineTempVarType(CTreeNode *Lhs)
{
	CSymbolTable *SymTable = CSymbolTable::GetSingletonPtr();
	Symbol_t *LhsSym = NULL;

	switch(Lhs->Code)
	{
	case TC_LITERAL:
		{
			switch(((CLiteralTreeNode*)Lhs)->LitType)
			{
			case TOKTYPE_FLOATLITERAL:
				return TOKTYPE_FLOATTYPE;

			case TOKTYPE_INTLITERAL:
				return TOKTYPE_INTEGERTYPE;

			case TOKTYPE_STRINGLITERAL:
				return TOKTYPE_STRINGTYPE;
			}
		}

	case TC_LITERALREF:
		return DetermineTempVarType(((CLiteralRefTreeNode*)Lhs)->GetChild(LITREF_LITERAL));

	case TC_IDENTIFIER:
	{
		CIdentifierTreeNode *Id = (CIdentifierTreeNode*)Lhs;
		LhsSym = SymTable->LookupIdentifier(Id->IdName, false);
		SanityCheck(LhsSym);

		return  LhsSym->SymType;
	}
	
	case TC_ARRAY_REF:
		{
			CArrayRefTreeNode *ArrRef = (CArrayRefTreeNode*)Lhs;
			LhsSym = SymTable->LookupIdentifier(ArrRef->ArrName->IdName, false);
			SanityCheck(LhsSym);

			return LhsSym->SymType;
		}

	case TC_UNARY_EXPR:
		{
			CUnaryExprTreeNode *UnExpr = (CUnaryExprTreeNode*)Lhs;

			//see if we have a literal or an identifier for the expression
			if(UnExpr->GetChild(UNARY_EXPR_EXPR)->Code == TC_LITERAL)
			{
				CLiteralTreeNode *UnExprLit;
				UnExprLit = (CLiteralTreeNode*)UnExpr->GetChild(UNARY_EXPR_EXPR);
				return UnExprLit->LitType;
			}

			//must have an identifier
			SanityCheck(UnExpr->GetChild(UNARY_EXPR_EXPR)->Code == TC_IDENTIFIER);

			CIdentifierTreeNode *UnExprId;
			UnExprId = (CIdentifierTreeNode*)UnExpr->GetChild(UNARY_EXPR_EXPR);

			//look it up
			Symbol_t *ExprSym = SymTable->LookupIdentifier(UnExprId->IdName, false);
			SanityCheck(ExprSym);

			return ExprSym->SymType;
		}
	}

	InternalError("unknown LHS node type");
}
//-------------------------------------------------------------
