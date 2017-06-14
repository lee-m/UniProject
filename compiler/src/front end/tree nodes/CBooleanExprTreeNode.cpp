//------------------------------------------------------------------------------------------
// File: CBooleanExprTreeNode.cpp
// Desc: Binary expression of the form Exp op Exp using boolean values and 
//       sub-expressions
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CBooleanExprTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CSymbolTable.h"
#include "CIdentifierTreeNode.h"

//-------------------------------------------------------------
void CBooleanExprTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	SanityCheck(XmlNode);

	TiXmlElement *ExpNode;
	ExpNode = XmlNode->InsertEndChild(TiXmlElement("bln_expr"))->ToElement();

	//add the operator
	TiXmlElement *OpNode;
	OpNode = ExpNode->InsertEndChild(TiXmlElement("operator"))->ToElement();
	OpNode->InsertEndChild(TiXmlText(CUtilFuncs::TokenTypeToStr(Operator).c_str()));

	//add the lhs
	TiXmlElement *LhsNode;
	LhsNode = ExpNode->InsertEndChild(TiXmlElement("lhs"))->ToElement();
	GetChild(BLN_EXPR_LHS)->DumpAsParseTreeNode(LhsNode);

	//finally the rhs
	TiXmlElement *RhsNode;
	RhsNode = ExpNode->InsertEndChild(TiXmlElement("rhs"))->ToElement();
	GetChild(BLN_EXPR_RHS)->DumpAsParseTreeNode(RhsNode);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CBooleanExprTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	GetChild(BLN_EXPR_LHS)->DumpAsMIRTreeNode(OutFile);
	OutFile << " " << CUtilFuncs::TokenTypeToStr(Operator) << " ";
	GetChild(BLN_EXPR_RHS)->DumpAsMIRTreeNode(OutFile);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CBooleanExprTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//process the operands, getting rid of any call_exprs or array references 
	//that may be lingering in the process
	CTreeNode *LhsNode = GetChild(BLN_EXPR_LHS)->RewriteIntoMIRForm(Stmts);
	LhsNode = ReplaceArrayRefs(ReplaceCallExpr(LhsNode, Stmts), Stmts);

	CTreeNode *RhsNode = GetChild(BLN_EXPR_RHS)->RewriteIntoMIRForm(Stmts);
	RhsNode = ReplaceArrayRefs(ReplaceCallExpr(RhsNode, Stmts), Stmts);

	//if either side was simplified, we will have a bln_expr in LhsNode or
	//RhsNode when we only want the identifier being assigned to which will
	//be the LHS of the expression
	if(LhsNode->Code == TC_BOOLEAN_EXPR)
	{
		CTreeNode *Temp = LhsNode->GetChild(BLN_EXPR_LHS)->CreateCopy();
		LhsNode->DestroyNode();
		LhsNode = Temp;
	}

	if(RhsNode->Code == TC_BOOLEAN_EXPR)
	{
		CTreeNode *Temp = RhsNode->GetChild(BLN_EXPR_LHS)->CreateCopy();
		RhsNode->DestroyNode();
		RhsNode = Temp;
	}

	//if we have a floating point type in the expression we need to preserve
	//that otherwise we could end up with wrong code. Consider the following:
	//   if(0.687) -> this is true
	//      ...
	//   end_if
	//however, if we did this:
	//   integer temp = 0.687;
	//   if(temp)
	//      ...
	//   end_if
	//the expression would evaluate to false because the floating point value
	//will be truncated to zero.
	TokenType TempVarType;
	if(CUtilFuncs::ExprRefersToFloatingPointType(LhsNode)
		 || CUtilFuncs::ExprRefersToFloatingPointType(RhsNode))
	  TempVarType = TOKTYPE_FLOATTYPE;
	else
		TempVarType = TOKTYPE_INTEGERTYPE;

	//generate a variable name for this temp var that will hold the
	//result of this expression
	string TempVarName = CUtilFuncs::GenCompilerTempName();
	CTreeNodeFactory *TreeNodeFactory = CTreeNodeFactory::GetSingletonPtr();

	//create a temp var_decl
	CVarDeclTreeNode *TempVar = (CVarDeclTreeNode*)TreeNodeFactory->BuildVarDecl(
		TempVarName, false, TempVarType);
	Stmts->AddChild(TempVar);

	//generate an assignment from the init value to the temp var
	Stmts->AddChild(TreeNodeFactory->BuildAssignmentExpr(TempVar->VarId->CreateCopy(),
		LhsNode));

	CSymbolTable::GetSingletonPtr()->AddSymbol(TempVar->VarId->IdName,
		TempVar, 0, TempVar->VarType);

	return RewriteExprIntoArithmeticForm(RhsNode, TempVar, Stmts);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CBooleanExprTreeNode::CreateCopy(void)
{
	return CTreeNodeFactory::GetSingletonPtr()->BuildBooleanExpression(
				GetChild(BLN_EXPR_LHS)->CreateCopy(),
				GetChild(BLN_EXPR_RHS)->CreateCopy(),
				Operator);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CBooleanExprTreeNode::RewriteExprIntoArithmeticForm(CTreeNode *RhsNode,
                                    CVarDeclTreeNode *TempVar, CTreeNode *Stmts)
{
	TokenType ArithOp;
	TokenType CmpOp;
	string LitValue;

	switch(Operator)
	{
	case TOKTYPE_GREATTHANEQ:
	case TOKTYPE_GREATTHAN:
		{
			//rewrite a > b to (a - b) > 0
			ArithOp = TOKTYPE_MINUS;
			CmpOp = Operator;
			LitValue = "0";
			break;
		}

	case TOKTYPE_LESSTHANEQ:
	case TOKTYPE_LESSTHAN:
		{
			//rewrite a < b to (a - b) < 0
			ArithOp = TOKTYPE_MINUS;
			CmpOp = Operator;
			LitValue = "0";
			break;
		}

	case TOKTYPE_AND:
		{
			//rewrite a and b to a + b >= 2
			ArithOp = TOKTYPE_PLUS;
			CmpOp = TOKTYPE_GREATTHANEQ;
			LitValue = "2";
			break;
		}

	case TOKTYPE_OR:
		{
			//rewrite a or b to a + b >= 1
			ArithOp = TOKTYPE_PLUS;
			CmpOp = TOKTYPE_GREATTHANEQ;
			LitValue = "1";
			break;
		}

	case TOKTYPE_NOTEQ:
		{
			//rewrite a != b to a - b != 0
			ArithOp = TOKTYPE_MINUS;
			CmpOp = TOKTYPE_NOTEQ;
			LitValue = "0";
			break;
		}

	case TOKTYPE_EQUALITY:
		{
			//rewrite a == b to a - b == 0
			ArithOp = TOKTYPE_MINUS;
			CmpOp = TOKTYPE_EQUALITY;
			LitValue = "0";
			break;
		}

	default:
		InternalError("unknown boolean operator");
	}

	CTreeNode *ArithExpr = CTreeNodeFactory::GetSingletonPtr()->BuildArithExpression(
		TempVar->VarId->CreateCopy(), RhsNode, ArithOp);

	CTreeNode *AssignExpr = CTreeNodeFactory::GetSingletonPtr()->BuildAssignmentExpr(
    TempVar->VarId->CreateCopy(), ArithExpr);
	Stmts->AddChild(AssignExpr);

	return CTreeNodeFactory::GetSingletonPtr()->BuildBooleanExpression(
		AssignExpr->GetChild(ASSIGN_EXPR_ID)->CreateCopy(),
		CTreeNodeFactory::GetSingletonPtr()->BuildLiteral(LitValue, TOKTYPE_INTEGERTYPE),
		CmpOp);
}
//-------------------------------------------------------------
