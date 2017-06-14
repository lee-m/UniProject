//------------------------------------------------------------------------------------------
// File: CIfStmtTreeNode.cpp
// Desc: Representation of an IF statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CIfStmtTreeNode.h"
#include "CLabelTreeNode.h"

//-------------------------------------------------------------
void CIfStmtTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	TiXmlElement *IfStmtNode;
	IfStmtNode = XmlNode->InsertEndChild(TiXmlElement("if_stmt"))->ToElement();

	//add the conditional expr
	TiXmlElement *CondExprNode;
	CondExprNode = IfStmtNode->InsertEndChild(TiXmlElement("cond_expr"))->ToElement();
	GetChild(IF_STMT_COND_EXPR)->DumpAsParseTreeNode(CondExprNode);

	//add the if stmt body
	TiXmlElement *IfStmtBody;
	IfStmtBody = IfStmtNode->InsertEndChild(TiXmlElement("if_stmt_body"))->ToElement();

	for(unsigned int i = 0; i < GetChild(IF_STMT_BODY)->GetNumChildren(); i++)
		GetChild(IF_STMT_BODY)->GetChild(i)->DumpAsParseTreeNode(IfStmtBody);

	//add the else stmt body
	TiXmlElement *ElseStmtBody;
	ElseStmtBody = IfStmtNode->InsertEndChild(TiXmlElement("else_stmt_body"))->ToElement();

	for(unsigned int i = 0; i < GetChild(IF_STMT_ELSE_BODY)->GetNumChildren(); i++)
		GetChild(IF_STMT_ELSE_BODY)->GetChild(i)->DumpAsParseTreeNode(ElseStmtBody);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CIfStmtTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	//dump the cond expr
	OutFile << "if(";
	GetChild(IF_STMT_COND_EXPR)->DumpAsMIRTreeNode(OutFile);
	OutFile << ")" << endl;

	//dump the 'if' body
	for(unsigned int i = 0; i < GetChild(IF_STMT_BODY)->GetNumChildren(); i++)
	{
		OutFile << "\t";
		GetChild(IF_STMT_BODY)->GetChild(i)->DumpAsMIRTreeNode(OutFile);
	}

  //dump the 'else' body
	for(unsigned int i = 0; i < GetChild(IF_STMT_ELSE_BODY)->GetNumChildren(); i++)
	{
		OutFile << "\t";
		GetChild(IF_STMT_ELSE_BODY)->GetChild(i)->DumpAsMIRTreeNode(OutFile);
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CIfStmtTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//here we turn a sequence of statements like
	//  if(a > b)
	//    <then_stmts>
	//  else
	//    <else_stmts>
	//  end_if
	//into:
	//  if(a > b>
	//    goto L1;
	//  goto L2;
	//  L1: <then_stmts>
	//  L2: <else_stmts>

	//generate a new if stmt using the simplified condition expr, adding it
	//to the IR
	CTreeNode *NewCondExpr = NULL;

	//simplify the conditional expr to build the new if stmt with
	NewCondExpr = GetChild(IF_STMT_COND_EXPR)->RewriteIntoMIRForm(Stmts);

	//get rid of array references in code like "if(arr[0])"
	if(NewCondExpr->Code == TC_ARRAY_REF)
		NewCondExpr = ReplaceArrayRefs(NewCondExpr, Stmts);

	//build a new if statement using the simplified conditional expr
	CTreeNode *NewIfStmt = CTreeNodeFactory::GetSingletonPtr()->BuildIfStatement(NewCondExpr);
	Stmts->AddChild(NewIfStmt);

	//label names for the <then_stmts> and <else_stmts> jumps
	string ThenLbl = CUtilFuncs::GenNewLabel();
	string ElseLbl;
	string EndOfElseLabel;

	NewIfStmt->GetChild(IF_STMT_BODY)->AddChild(
    CTreeNodeFactory::GetSingletonPtr()->BuildGotoExpr(ThenLbl));

	//label/goto expr tp the end of the if statement block or alternatively the
	//else stmt block if one is present
	CTreeNode *EndLabel = CTreeNodeFactory::GetSingletonPtr()->BuildLabel(
                          CUtilFuncs::GenNewLabel());
	CTreeNode *EndJump = CTreeNodeFactory::GetSingletonPtr()->BuildGotoExpr(
                          ((CLabelTreeNode*)EndLabel)->Name);

	//generate label for <else_stmts> if required
	if(GetChild(IF_STMT_ELSE_BODY)->GetNumChildren() > 0)
	{
		//add a jump to the start of the 'else' block which will be executed if the
		//cond_expr evaluated to false
		ElseLbl = CUtilFuncs::GenNewLabel();
		Stmts->AddChild(CTreeNodeFactory::GetSingletonPtr()->BuildGotoExpr(ElseLbl));
	}
	else
	{
		//add a jump before the beginning of the then stmts, this will be executed
		//if the cond_expr evaluated to false, causing us not to execute the body
		//of the if statement
		Stmts->AddChild(EndJump);
	}

	//process <then_stmts>
	Stmts->AddChild(CTreeNodeFactory::GetSingletonPtr()->BuildLabel(ThenLbl));
	for(unsigned int i = 0; i < GetChild(IF_STMT_BODY)->GetNumChildren(); i++)
	{
		CTreeNode *Temp = GetChild(IF_STMT_BODY)->GetChild(i)->RewriteIntoMIRForm(Stmts);
		
		//handle cases where we have a nested if which returns NULL from
		//the MIR lowering
		if(Temp)
			Stmts->AddChild(Temp);
	}

	//process <else_stmts>
	if(ElseLbl.length() > 0)
	{
		//we'll reach the start of the 'else' block when we finish executing the 'then'
		//block, causing us to execute statements in both the 'then' and 'else' blocks which
		//is obviously not what we want so add a jump to the end of this block
		Stmts->AddChild(EndJump);

		//the following label marks the start of the 'else block of statements
		Stmts->AddChild(CTreeNodeFactory::GetSingletonPtr()->BuildLabel(ElseLbl));

		//dump the 'else' statements
		for(unsigned int i = 0; i < GetChild(IF_STMT_ELSE_BODY)->GetNumChildren(); i++)
		{
			CTreeNode *Temp;
			Temp = GetChild(IF_STMT_ELSE_BODY)->GetChild(i)->RewriteIntoMIRForm(Stmts);
			
			//if we've lowered a nested if stmt it will have returned NULL
			//so make sure we dont ICE
			if(Temp)
				Stmts->AddChild(Temp);
		}
	}

	//label marking the end of the entire 'if' statement which is used
	//a) as the target for the jump executed if the cond_expr evaluated to false
	//   and we dont have an else block, or
	//b) as the target for the jump at the end of the 'then' block to skip over
	//   the 'else' block
	Stmts->AddChild(EndLabel);

	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CIfStmtTreeNode::CreateCopy(void)
{
	//create the initial if_stmt
	CTreeNode *NewIfStmt;
	NewIfStmt = CTreeNodeFactory::GetSingletonPtr()->BuildIfStatement(
								GetChild(IF_STMT_COND_EXPR)->CreateCopy());

	//copy the if body
	for(unsigned int i = 0; i < GetChild(IF_STMT_BODY)->GetNumChildren(); i++)
		NewIfStmt->GetChild(IF_STMT_BODY)->AddChild(GetChild(IF_STMT_BODY)->GetChild(i));

	//copy the else body
	for(unsigned int i = 0; i < GetChild(IF_STMT_ELSE_BODY)->GetNumChildren(); i++)
		NewIfStmt->GetChild(IF_STMT_ELSE_BODY)->AddChild(
      GetChild(IF_STMT_ELSE_BODY)->GetChild(i));

	return NewIfStmt;
}
//-------------------------------------------------------------
