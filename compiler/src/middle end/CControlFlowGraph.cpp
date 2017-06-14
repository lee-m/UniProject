//------------------------------------------------------------------------------------------
// File: CControlFlowGraph.cpp
// Desc: Partition the list of MIR statements into basic blocks which forms the CFG
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CControlFlowGraph.h"
#include "CFunctionDeclTreeNode.h"
#include "CLabelTreeNode.h"
#include "CGotoExprTreeNode.h"

//-------------------------------------------------------------
void CControlFlowGraph::BuildControlFlowGraph(CFunctionDeclTreeNode **FnDecl, bool DumpGraph)
{
	SanityCheck(FnDecl);

	//first pass: partition the fn decls statements into basic blocks
	FindBasicBlocks(FnDecl);

	//second pass: build the CFG edges
	ComputeEdges(FnDecl);

	if(DumpGraph)
		DumpControlFlowGraph(*FnDecl);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CControlFlowGraph::FindBasicBlocks(CFunctionDeclTreeNode **FnDecl)
{
	//the parser should have stripped out any empty functions
	SanityCheck((*FnDecl)->GetNumChildren() > 0);

	//go through the statements comprising the function and partition them
	//into basic blocks
	BasicBlock_t *CurrBlock = new BasicBlock_t;
	BasicBlock_t *LastBlock = NULL;

	CurrBlock->ExitBb = false;
	unsigned int BbIdNo = 0;

	//add the first statement, child 0 of a fndecl is the
	//parameter list
	CurrBlock->Stmts.push_back((*FnDecl)->GetChild(1));
	CurrBlock->BbId = BbIdNo;

	//process the remaining statements
	for(unsigned int i = 2; i < (*FnDecl)->GetNumChildren(); i++)
	{
		//if the current statement is not a leader, add it to the current block
		if(!IsLeader((*FnDecl)->GetChild(i)))
			CurrBlock->Stmts.push_back((*FnDecl)->GetChild(i));
		else
		{
			//add the current block to the list of bbs
			(*FnDecl)->FuncCFG.push_back(CurrBlock);

			//create a new block and add the current stmt to it
			LastBlock = CurrBlock;
			CurrBlock = new BasicBlock_t;

			CurrBlock->ExitBb = false;
			CurrBlock->BbId = ++BbIdNo;
			CurrBlock->Stmts.push_back((*FnDecl)->GetChild(i));

			//if the last statement in the old block is not a jump, mark it as 
			//a fall thru block and add the relevant edges
			CTreeNode *LastStmt = LastBlock->Stmts.back();
			if(LastStmt->Code != TC_GOTOEXPR)
			{
				//we have the potential to fall thru if the cond expr evaluates to false
				LastBlock->Fallthru = true;

				//add the current block as a successor of the old one
				LastBlock->Successors.insert(CurrBlock);
				CurrBlock->Predecessors.insert(LastBlock);

				//if we've got an if_stmt as the last stmt in the block,
				//add the target of the jump to the edges list for use later on
				if(LastStmt->Code == TC_IFSTMT)
				{
					CGotoExprTreeNode *CondGoto;
					CondGoto = (CGotoExprTreeNode*)LastStmt->GetChild(IF_STMT_BODY)->GetChild(0);
					m_Edges.insert(make_pair(CondGoto->Target, LastBlock));
				}
			}
			else
			{
				//we've got a jump, use the jump target as the key for this 
			  //basic block when we compute the graph's edges later on
				CGotoExprTreeNode *GotoExpr = (CGotoExprTreeNode*)LastStmt;
				m_Edges.insert(make_pair(GotoExpr->Target, LastBlock));
			}
		}
	}

	//add the last block built
	(*FnDecl)->FuncCFG.push_back(CurrBlock);

	//add an exit block along with an edge to the last block in the function
	BasicBlock_t *ExitBlock = new BasicBlock_t;
	ExitBlock->Predecessors.insert(CurrBlock);
	CurrBlock->Successors.insert(ExitBlock);

	ExitBlock->ExitBb = true;
	ExitBlock->BbId = ++BbIdNo;
	(*FnDecl)->FuncCFG.push_back(ExitBlock);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CControlFlowGraph::ComputeEdges(CFunctionDeclTreeNode **FnDecl)
{
	//go through each basic block in the CFG
	list<BasicBlock_t*>::iterator BbItr = (*FnDecl)->FuncCFG.begin();
	for(; BbItr != --(*FnDecl)->FuncCFG.end(); BbItr++)
	{
		//get the first statement
		CTreeNode *FirstStmt = *((*BbItr)->Stmts.begin());

		//if the first statement is a label we've got a jump somewhere to
		//it. Use the info we've gathered during finding the basic blocks to 
		//look it up
		if(FirstStmt->Code == TC_LABEL)
		{
			//get the name of the label for the lookup
			string LblName = ((CLabelTreeNode*)FirstStmt)->Name;

			//find out which blocks contains a jump to this label as the last stmt
			//in their respective blocks and add the relevent edges beween them
			multimap<string, BasicBlock_t*>::iterator EdgeItr;
			for(EdgeItr = m_Edges.lower_bound(LblName); 
				  EdgeItr != m_Edges.upper_bound(LblName); EdgeItr++)
			{
				(*BbItr)->Predecessors.insert(EdgeItr->second);
				EdgeItr->second->Successors.insert(*BbItr);
			}
		}

		//get the last stmt
		CTreeNode *LastStmt = *(--(*BbItr)->Stmts.end());

		//if the block terminates with a return stmt, add an edge to the exit block
		if(LastStmt->Code == TC_RETURNSTMT)
		{
			BasicBlock_t *ExitBlock = (*FnDecl)->FuncCFG.back();
			(*BbItr)->Successors.insert(ExitBlock);
			ExitBlock->Predecessors.insert(*BbItr);
		}
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CControlFlowGraph::IsLeader(CTreeNode *Stmt)
{
	//leader statements are jumps and the targets of a jump
	return Stmt->Code == TC_GOTOEXPR
		|| Stmt->Code == TC_LABEL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CControlFlowGraph::DumpControlFlowGraph(CFunctionDeclTreeNode *FnDecl)
{
	//open the outfile for creation or appending
	ofstream DumpFile;
	DumpFile.open("ControlFlowGraph.txt", ios::app);

	//dump which function the CFG is for
	DumpFile << "Control Flow Graph for function: " << FnDecl->Name << endl << endl;

	//dump the blocks
	list<BasicBlock_t*>::iterator itr = FnDecl->FuncCFG.begin();

	for(; itr != FnDecl->FuncCFG.end(); itr++)
	{
		//show which bb this is
		DumpFile << "<basic block " << (*itr)->BbId << ", ";

		if((*itr)->Fallthru)
			DumpFile << "fallthru, ";

		//add predecessors
		DumpFile << "predecessors - ";

		BasicBlockItr PredsItr = (*itr)->Predecessors.begin();
		for(; PredsItr != (*itr)->Predecessors.end(); PredsItr++)
			DumpFile << (*PredsItr)->BbId << " ";

		//add successors
		DumpFile << "; successors - ";

		BasicBlockItr SuccsItr = (*itr)->Successors.begin();
		for(; SuccsItr != (*itr)->Successors.end(); SuccsItr++)
			DumpFile << (*SuccsItr)->BbId << " ";

		DumpFile << ">" << endl;

		//dump the statements which make up this block
		BasicBlockStmtItr StmtItr = (*itr)->Stmts.begin();
		for(; StmtItr != (*itr)->Stmts.end(); StmtItr++)
		{
			DumpFile << "  ";
			(*StmtItr)->DumpAsMIRTreeNode(DumpFile);
			DumpFile << endl;
		}

		//mark the end of the basic block
		DumpFile << endl << "<end of basic block> " << endl << endl;
	}

	DumpFile.close();
}
//-------------------------------------------------------------