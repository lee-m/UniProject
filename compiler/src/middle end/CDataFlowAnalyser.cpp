//------------------------------------------------------------------------------------------
// File: CDataFlowAnalyser.cpp
// Desc: Iterative solver for the various data flow problems
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CDataFlowAnalyser.h"
#include "CTreeNode.h"
#include "CControlFlowGraph.h"
#include "CFunctionDeclTreeNode.h"
#include "CIdentifierTreeNode.h"
#include "CSymbolTable.h"
#include "CCallExprTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CParameterDeclTreeNode.h"

//-------------------------------------------------------------
void CDataFlowAnalyser::AnalyseFunction(CFunctionDeclTreeNode **FnDecl, 
																				bool DumpDfInfo)
{
	SanityCheck(FnDecl);

	//first step: initialise the work list for the algorithm to work with then
	//reverse it since liveness analysis is a backwards problem
	list<BasicBlock_t*>::iterator BbIter;
	for(BbIter = (*FnDecl)->FuncCFG.begin(); BbIter != (*FnDecl)->FuncCFG.end(); BbIter++)
		m_Worklist.push_back(*BbIter);
	reverse(m_Worklist.begin(), m_Worklist.end());

	//scan the statements inside each block to compute the use-def information, 
	//we need this for the liveness calculation later on
	m_CurrStmtNo = 0;

	for(BbIter = (*FnDecl)->FuncCFG.begin(); BbIter != (*FnDecl)->FuncCFG.end(); BbIter++)
		ComputeUseDefs(*BbIter);

	ComputeLivenessInfo();

	//dump the computed info if required
	if(DumpDfInfo)
		DumpDataFlowInformation((*FnDecl)->FuncCFG, (*FnDecl)->Name);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CDataFlowAnalyser::ComputeUseDefs(BasicBlock_t *Block)
{
	SanityCheck(Block);

	//scan each statement looking for definitions
	list<CTreeNode*>::iterator StmtItr;
	for(StmtItr = Block->Stmts.begin(); StmtItr != Block->Stmts.end(); StmtItr++)
	{
		//grab the stmt
		CTreeNode *Stmt = *StmtItr;
		++m_CurrStmtNo;

		//filter out useless statements we're not interested in
		if(Stmt->Code == TC_LABEL || Stmt->Code == TC_GOTOEXPR)
			continue;

		switch(Stmt->Code)
		{
		case TC_VARDECL:
			{
				CVarDeclTreeNode *VarDecl = (CVarDeclTreeNode*)Stmt;
				VarDecl->DeclPoint = m_CurrStmtNo;
				Block->DefinedVars.insert(VarDecl->VarId);
				break;
			}

		case TC_ASSIGN_EXPR:
			{
				//find any uses on the RHS
				ScanStmtForUses(Stmt->GetChild(ASSIGN_EXPR_RHS), Block);

				//add the var being assigned to to the list of def vars, valid
				//lvalues which could appear are identifiers and array references
				if(Stmt->GetChild(ASSIGN_EXPR_ID)->Code == TC_IDENTIFIER)
				{
					Block->DefinedVars.insert((CIdentifierTreeNode*)Stmt->GetChild(ASSIGN_EXPR_ID));
				}
				else if(Stmt->GetChild(ASSIGN_EXPR_ID)->Code == TC_ARRAY_REF)
					ScanStmtForUses(Stmt->GetChild(ASSIGN_EXPR_ID), Block);
			
				break;
			}

		case TC_RETURNSTMT:
			{
				ScanStmtForUses(Stmt->GetChild(RET_STMT_RET_EXPR), Block);
				break;
			}

		case TC_IFSTMT:
			{
				ScanStmtForUses(Stmt->GetChild(IF_STMT_COND_EXPR), Block);
				break;
			}

		case TC_CALLEXPR:
			{
				//scan each parameter expr
				for(unsigned int i = 0; i < Stmt->GetNumChildren(); i++)
					ScanStmtForUses(Stmt->GetChild(i), Block);
				break;
			}

		case TC_ARRAYDECL:
			{
				//array decls live in the stack frame, not hard registers
				//so dont mark it as defined
				break;
			}

		case TC_BOOLEAN_EXPR:
			{
				ScanStmtForUses(Stmt->GetChild(BLN_EXPR_LHS), Block);
				ScanStmtForUses(Stmt->GetChild(BLN_EXPR_RHS), Block);
				break;
			}

		default:
			InternalError("unhandled stmt type");
		}
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CDataFlowAnalyser::ScanStmtForUses(CTreeNode *Stmt, BasicBlock_t *Block)
{
	SanityCheck(Stmt && Block);

	//discard useless statements
	if(Stmt->Code == TC_LITERAL || Stmt->Code == TC_LITERALREF)
		return;

	switch(Stmt->Code)
	{
	case TC_IDENTIFIER:
		{
			ScanIdentifierNodeForUses(Stmt, Block);
			break;
		}

	case TC_UNARY_EXPR:
		{
			ScanStmtForUses(Stmt->GetChild(UNARY_EXPR_EXPR), Block);
			break;
		}

	case TC_ARITH_EXPR:
	case TC_BOOLEAN_EXPR:
		{
			//scan both sides of the expr for uses
			ScanStmtForUses(Stmt->GetChild(ARITH_EXPR_LHS), Block);
			ScanStmtForUses(Stmt->GetChild(ARITH_EXPR_RHS), Block);
			break;
		}
	case TC_CALLEXPR:
		{
			//scan each parameter expr
			for(unsigned int i = 0; i < Stmt->GetNumChildren(); i++)
				ScanStmtForUses(Stmt->GetChild(i), Block);
			break;
		}

	case TC_ARRAY_REF:
		{
			CArrayRefTreeNode *ArrRef = (CArrayRefTreeNode*)Stmt;
			Block->UsedVars.insert(ArrRef->ArrName);
			break;
		}

	default:
		InternalError("unhandled expr type");
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CDataFlowAnalyser::ScanIdentifierNodeForUses(CTreeNode *Id, BasicBlock_t *Block)
{
	SanityCheck(Id && Block);
	CIdentifierTreeNode *IdNode = (CIdentifierTreeNode*)Id;
	Symbol_t *Sym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(IdNode->IdName, false);
	SanityCheck(Sym);

	switch(Sym->IdPtr->Code)
	{
	case TC_VARDECL:
	case TC_ARRAY_REF:
		{
			//nothing special here, just add the name to the list of used vars
			Block->UsedVars.insert(IdNode);
			return;
		}

	case TC_CALLEXPR:
		{
			//iterate through any parameters, checking them as we go
			CCallExprTreeNode *CallExpr = (CCallExprTreeNode*)Sym->IdPtr;

			for(unsigned int i = 0; i < CallExpr->GetNumChildren(); i++)
				ScanStmtForUses(CallExpr->GetChild(i), Block);
			return;
		}

	case TC_PARMDECL:
		{
			//parameter decls live in the stack frame so dont mark them as
			//defined to prevent them from being allocated a hard register since
			//they can be loaded directly from the stack
			break;
		}
	default:
			InternalError("unknown stmt type");
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CDataFlowAnalyser::DumpDataFlowInformation(list<BasicBlock_t*> &Graph,
																								string FuncName)
{
  //open the outfile for creation or appending
	ofstream DumpFile;
	DumpFile.open("DataFlow.txt", ios::app);

	//dump which file the CFG is for
	DumpFile << "Data flow information for function: " << FuncName << endl << endl;

	list<BasicBlock_t*>::iterator GraphItr;
	for(GraphItr = Graph.begin(); GraphItr != Graph.end(); GraphItr++)
	{
		//show which block we're dumping the info for
		DumpFile << "Basic Block: " << (*GraphItr)->BbId << endl;

		//show the definitions if we've got any
		if(!(*GraphItr)->DefinedVars.empty())
		{
			DumpFile << "Definitions:";

			set<CIdentifierTreeNode*, CompIdentifiers>::iterator DefItr;
			for(DefItr = (*GraphItr)->DefinedVars.begin(); DefItr != (*GraphItr)->DefinedVars.end(); DefItr++)
				DumpFile << " " << (*DefItr)->IdName << " ";

			DumpFile << endl;
		}

		//uses
		if(!(*GraphItr)->UsedVars.empty())
		{
			DumpFile << "Uses:";

			set<CIdentifierTreeNode*, CompIdentifiers>::iterator UseItr;
			for(UseItr = (*GraphItr)->UsedVars.begin(); UseItr != (*GraphItr)->UsedVars.end(); UseItr++)
				DumpFile << " " << (*UseItr)->IdName << " ";

			DumpFile << endl;
		}

		//live on exit
		if(!(*GraphItr)->ValsLiveOnExit.empty())
		{
			DumpFile << "Live On Exit:";

			set<CIdentifierTreeNode*, CompIdentifiers>::iterator LiveOnExitItr;
			for(LiveOnExitItr = (*GraphItr)->ValsLiveOnExit.begin();
				LiveOnExitItr != (*GraphItr)->ValsLiveOnExit.end(); LiveOnExitItr++)
			{
				DumpFile << " " << (*LiveOnExitItr)->IdName << " ";
			}

			DumpFile << endl;
		}

		//live on entry
		if(!(*GraphItr)->ValsLiveOnEntry.empty())
		{
			DumpFile << "Live On Entry:";

			set<CIdentifierTreeNode*, CompIdentifiers>::iterator LiveOnEntryItr;
			for(LiveOnEntryItr = (*GraphItr)->ValsLiveOnEntry.begin();
				LiveOnEntryItr != (*GraphItr)->ValsLiveOnEntry.end(); LiveOnEntryItr++)
			{
				DumpFile << " " << (*LiveOnEntryItr)->IdName << " ";
			}

			DumpFile << endl;
		}

		DumpFile << endl << endl;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CDataFlowAnalyser::ComputeLivenessInfo(void)
{
	//keep processing each block in the worklist until it's empty. If 
	//the liveness information for any block is changed along the way we
	//add it's successors to the worklist for re-processing if they haven't
	//already been added
	while(!m_Worklist.empty())
	{
		//grab the top bb
		BasicBlock_t *Bb = m_Worklist.front();
		m_Worklist.pop_front();

		//compute the liveness info for the current block
		bool LiveInChanged = ComputeLiveInForBlock(Bb);
		bool LiveOutChanged = ComputeLiveOutForBlock(Bb);

		if(LiveOutChanged || LiveInChanged)
		{
			BasicBlockItr SuccItr;
			for(SuccItr = Bb->Successors.begin();  SuccItr != Bb->Successors.end(); SuccItr++)
				m_Worklist.push_back(*SuccItr);

			//we need to re-process this block as information changes in live in or
			//live out could have an impact on the alternative problem
			m_Worklist.push_back(Bb);
		}
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CDataFlowAnalyser::ComputeLiveOutForBlock(BasicBlock_t *Block)
{
	bool RetVal = false;

	//the dataflow equation we solve here is:
	// out[n] = U in[s] for each bb s in succs[n] U (def[n] - in[n])
	//any variable which is live on entry to a block must also be live
	//on the edge coming from the successor it was defined in to this one.
	//Similiarly, any value which is live on entry to this block which
	//isn't killed within it (defined) must also be live on the exit edge
	//to any successor blocks
	list<BasicBlock_t*>::iterator SuccsItr;

	//set to hold the union of all successor's live on entry vals for this block
	set<CIdentifierTreeNode*, CompIdentifiers> LiveOnExitResults;

	//if there isn't any successors there's nothing to do 
	//for this particular equation
	if(!Block->Successors.empty()) 
	{
		//if the block only has a single successor, add it's contents
		//to the live out set if they're not already in there
		if(Block->Successors.size() == 1)
		{
			BasicBlock_t *Succ = *(Block->Successors.begin());

			if(Block->ValsLiveOnExit.empty()
				 || !includes(Succ->ValsLiveOnEntry.begin(), Succ->ValsLiveOnEntry.end(),
							Block->ValsLiveOnExit.begin(), Block->ValsLiveOnExit.end()))
			{
				//copy those values which are live on entry to the successor 
				//block to the live on exit of this block
				if(!Succ->ValsLiveOnEntry.empty())
				{
					insert_iterator < set<CIdentifierTreeNode*, CompIdentifiers> > InsItr(
						LiveOnExitResults, LiveOnExitResults.begin());

					copy(Succ->ValsLiveOnEntry.begin(), Succ->ValsLiveOnEntry.end(), InsItr);
				}
			}
		}
		else
		{
			//the basic blocks we use for the union when iterating through
			BasicBlock_t *LastBlock = *(Block->Successors.begin());
			BasicBlock_t *CurrBlock = *(++Block->Successors.begin());

			BasicBlockItr SuccItr = ++Block->Successors.begin();

			while(true)
			{
				//compute the union of the two sets and store the 
				//resulting values in LiveOnExitResults
				insert_iterator < set<CIdentifierTreeNode*, CompIdentifiers> > InsItr(
					LiveOnExitResults, LiveOnExitResults.begin());

				set_union(LastBlock->ValsLiveOnEntry.begin(), LastBlock->ValsLiveOnEntry.end(),
					CurrBlock->ValsLiveOnEntry.begin(), CurrBlock->ValsLiveOnEntry.end(),
					InsItr);

				swap(CurrBlock, LastBlock);
				BasicBlockItr Temp = ++SuccItr;

				if(Temp == Block->Successors.end())
					break;

				CurrBlock = *Temp;
			}
		}
	}

	//any values live on the incoming edge which aren't killed in this block need
	//to be marked live on the exit edge to any successors
	insert_iterator< set<CIdentifierTreeNode*, CompIdentifiers> > InMinDefInsItr(
			LiveOnExitResults, LiveOnExitResults.begin());

	if(!Block->DefinedVars.empty())
	{
		//add any variables which appear in the live on entry but dont also appear
		//in the live on exit to the current results set
		set_difference(Block->ValsLiveOnEntry.begin(), Block->ValsLiveOnEntry.end(),
			Block->DefinedVars.begin(), Block->DefinedVars.end(), InMinDefInsItr);
	}
	else
	{
		//HACK!!! This has got to be the biggest bodge I've ever written. For 
		//our sets using op== doesn't return true even when the sets do actually
		//contain the same value because it's presumably using pointer equality and we
		//can end up with the same identifier value in the sets but allocated to 
		//different pointers, even when using a comparison function so we 
		//end up in an infinite loop since we think we're constantly making changes 
		//to the computed sets. To fix this we go through each member of the sets 
		//and compare them ourselves 
		if(!CompareSets(Block->ValsLiveOnEntry, Block->ValsLiveOnExit))
		{
			copy(Block->ValsLiveOnEntry.begin(), Block->ValsLiveOnEntry.end(), 
				InMinDefInsItr);
		}
	}

	//copy the results to this blocks live on exit set if they're
	//not in there already, we need this check otherwise we'll end
	//up in an infinite loop for any block with more than one successor		
	if(!CompareSets(Block->ValsLiveOnExit, LiveOnExitResults)
		 && !LiveOnExitResults.empty())
	{
		insert_iterator< set<CIdentifierTreeNode*, CompIdentifiers> > ResInsItr(
			Block->ValsLiveOnExit, Block->ValsLiveOnExit.begin());

		copy(LiveOnExitResults.begin(), LiveOnExitResults.end(), ResInsItr);
		RetVal = true;
	}

	return RetVal;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CDataFlowAnalyser::ComputeLiveInForBlock(BasicBlock_t *Block)
{
	//solve in[n] = use[n] U (out[n] - def[n])

	//compute out[n] - def[n]
	set<CIdentifierTreeNode*> OutMinDefSet;
	insert_iterator< set<CIdentifierTreeNode*> > OutMinInsItr(OutMinDefSet,
		OutMinDefSet.begin());

	set_difference(Block->DefinedVars.begin(), Block->DefinedVars.end(),
		Block->ValsLiveOnExit.begin(), Block->ValsLiveOnExit.end(), OutMinInsItr);

	//compute use[n] U (out[n] - def[n]) where the results of
	//def[n] - out[n] are stored in OutMinDefSet. Store the results
	//into LiveInResults.
	set<CIdentifierTreeNode*, CompIdentifiers> LiveInResults;

	//if out[n] - def[n] is empty theres nothing to union with so copy the
	//used vars into the results
	if(!OutMinDefSet.empty())
	{
		insert_iterator< set<CIdentifierTreeNode*, CompIdentifiers> > ResInsItr(
			LiveInResults, LiveInResults.begin());

		set_union(Block->UsedVars.begin(), Block->UsedVars.end(),
			OutMinDefSet.begin(), OutMinDefSet.end(), ResInsItr);
	}
	else
	{
		insert_iterator< set<CIdentifierTreeNode*, CompIdentifiers> > InsItr(
			LiveInResults, LiveInResults.begin());

		copy(Block->UsedVars.begin(), Block->UsedVars.end(), InsItr);
	}

  //if the computed results aren't contained in this blocks live
	//out set, copy them in
	if(!CompareSets(Block->ValsLiveOnEntry, LiveInResults))
	{
		insert_iterator< set<CIdentifierTreeNode*, CompIdentifiers> > InsItr(
			Block->ValsLiveOnEntry, Block->ValsLiveOnEntry.begin());

		copy(LiveInResults.begin(), LiveInResults.end(), 
			InsItr);
		return true;
	}

	return false;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CDataFlowAnalyser::CompareSets(set<CIdentifierTreeNode*, CompIdentifiers> &SetOne,
																	  set<CIdentifierTreeNode*, CompIdentifiers> &SetTwo)
{
	bool RetValue = true;

	//if the sets dont have the same size they can't match
	if(SetOne.size() == SetTwo.size())
	{
		//get each value from the two sets and compare them
		set<CIdentifierTreeNode*, CompIdentifiers>::iterator SetOneItr;
		set<CIdentifierTreeNode*, CompIdentifiers>::iterator SetTwoItr;

		SetOneItr = SetOne.begin();
		SetTwoItr = SetTwo.begin();

		for(; SetOneItr != SetOne.end(); SetOneItr++, SetTwoItr++)
		{
			CIdentifierTreeNode *IdOne = *SetOneItr;
			CIdentifierTreeNode *IdTwo = *SetTwoItr;
		
			if(IdOne->IdName != IdTwo->IdName)
				RetValue = false;
		}
	}
	else
		RetValue = false;

	return RetValue;
}
//-------------------------------------------------------------