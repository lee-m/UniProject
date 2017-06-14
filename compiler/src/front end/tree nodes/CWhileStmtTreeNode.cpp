//------------------------------------------------------------------------------------------
// File: CWhileStmtTreeNode.cpp
// Desc: Representation of a WHILE statement within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CWhileStmtTreeNode.h"
#include "CLabelTreeNode.h"
#include "CGotoExprTreeNode.h"

//-------------------------------------------------------------
void CWhileStmtTreeNode::DumpAsParseTreeNode(TiXmlElement *XmlNode)
{
	TiXmlElement *IfStmtNode;
	IfStmtNode = XmlNode->InsertEndChild(TiXmlElement("while_stmt"))->ToElement();

	//add the conditional expr
	TiXmlElement *CondExprNode;
	CondExprNode = IfStmtNode->InsertEndChild(TiXmlElement("cond_expr"))->ToElement();
	GetChild(WHILE_STMT_COND_EXPR)->DumpAsParseTreeNode(CondExprNode);

	//add the body
	TiXmlElement *StmtBody;
	StmtBody = IfStmtNode->InsertEndChild(TiXmlElement("while_stmt_body"))->ToElement();

	for(unsigned int i = 1; i < GetNumChildren(); i++)
		GetChild(i)->DumpAsParseTreeNode(StmtBody);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CWhileStmtTreeNode::DumpAsMIRTreeNode(ofstream &OutFile)
{
	InternalError("unimplemented");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CWhileStmtTreeNode::RewriteIntoMIRForm(CTreeNode *Stmts)
{
	//when lowering a while statement we turn
	//  while(a > 0)
	//    <stmts>
	//  end_while
	//into:
	//  L1: if(a > 0)
	//        goto L2;
	//      goto L3;
	//  L2: <stmts>
	//      goto L1;
	//  L3:

	//add the header label
	CTreeNode *HeaderLbl;
	HeaderLbl = CTreeNodeFactory::GetSingletonPtr()->BuildLabel(CUtilFuncs::GenNewLabel());
	Stmts->AddChild(HeaderLbl);

	//rewrite the loop condition into an if statement
	CTreeNode *LoopCondExpr = GetChild(WHILE_STMT_COND_EXPR)->RewriteIntoMIRForm(Stmts);

	//build the if statement which tests the above cond_expr and a label (L1 in
	//the above example) for it so we can jump back to the start of the loop after
	//each iteration
	CTreeNode *LoopIfStmt;
  LoopIfStmt = CTreeNodeFactory::GetSingletonPtr()->BuildIfStatement(LoopCondExpr);
  
	//add the label and if_stmt to the IR
	Stmts->AddChild(LoopIfStmt);

	//the following jumps are to the start of the loop body, and to the
	//end of <stmts> (L2 and L3 in the above example respectively)
	CTreeNode *StmtStartJmp = CTreeNodeFactory::GetSingletonPtr()->BuildGotoExpr(
															CUtilFuncs::GenNewLabel());
	CTreeNode *StmtEndJmp = CTreeNodeFactory::GetSingletonPtr()->BuildGotoExpr(
															CUtilFuncs::GenNewLabel());

	//add the jump to the loop body to the if stmt, add the jump
	//to the end of the loop to the IR after the if_stmt
	LoopIfStmt->GetChild(IF_STMT_BODY)->AddChild(StmtStartJmp);
	Stmts->AddChild(StmtEndJmp);

	//create and add a label marking the start of <stmts>
	Stmts->AddChild(CTreeNodeFactory::GetSingletonPtr()->BuildLabel(
		((CGotoExprTreeNode*)StmtStartJmp)->Target));

	//add the statements forming the loop body
	for(unsigned int i = 1; i < GetNumChildren(); i++)
	{
		CTreeNode *Temp = GetChild(i)->RewriteIntoMIRForm(Stmts);

		//cope with nested while/if statements which return NULL 
		//from MIR lowering
		if(Temp)
			Stmts->AddChild(Temp);
	}

	//add a jump back to the loop header to retest the condition
	Stmts->AddChild(CTreeNodeFactory::GetSingletonPtr()->BuildGotoExpr(
		((CLabelTreeNode*)HeaderLbl)->Name));

	//add the label marking the end of the loop which we jump to when
	//the condition evaluates to false
	Stmts->AddChild(CTreeNodeFactory::GetSingletonPtr()->BuildLabel(
		((CGotoExprTreeNode*)StmtEndJmp)->Target));

  //nothing to return
	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CWhileStmtTreeNode::CreateCopy(void)
{
	//create the initial while_stmt
	CTreeNode *NewWhileStmt;
	NewWhileStmt = CTreeNodeFactory::GetSingletonPtr()->BuildWhileStatement(
										GetChild(WHILE_STMT_COND_EXPR)->CreateCopy());

	//add the body
	for(unsigned int i = 1; i < GetNumChildren(); i++)
		NewWhileStmt->AddChild(GetChild(i)->CreateCopy());

	return NewWhileStmt;
}
//-------------------------------------------------------------
