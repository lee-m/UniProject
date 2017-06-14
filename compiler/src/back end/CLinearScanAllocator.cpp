//------------------------------------------------------------------------------------------
// File: CLinearScanAllocator.cpp
// Desc: Implementation of the linear scan register allocation algorithm
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CLinearScanAllocator.h"
#include "CControlFlowGraph.h"
#include "CSymbolTable.h"
#include "CVarDeclTreeNode.h"
#include "CArrayDeclTreeNode.h"
#include "CLiteralTreeNode.h"

//-------------------------------------------------------------
void CLinearScanAllocator::CalculateLiveIntervals(list<BasicBlock_t*> &FuncCFG)
{
	//make a pass through the control flow graph, looking for variable
	//definitions which we use for computing the start point of that
	//values live interval
	list<BasicBlock_t*>::iterator BbItr;
	int StmtNo = 0;

	for(BbItr = FuncCFG.begin(); BbItr != FuncCFG.end(); BbItr++)
	{
		AddNewLiveIntervals(*BbItr, StmtNo);
		StmtNo += (int)(*BbItr)->Stmts.size();
		CalculateEndPoints(*BbItr, StmtNo);
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::AddNewLiveIntervals(BasicBlock_t *Bb, int StartStmtNo)
{
	//does this block have any values live on entry?
	if(!Bb->ValsLiveOnEntry.empty())
	{
		//go through each value live on entry and record their
		//start point as the beginning of this basic block
		set<CIdentifierTreeNode*, CompIdentifiers>::iterator ValItr;
		for(ValItr = Bb->ValsLiveOnEntry.begin(); ValItr != Bb->ValsLiveOnEntry.end(); ValItr++)
		{
			LiveInterval_t *LiveInt = FindLiveInterval(*ValItr);

			if(!LiveInt)
			{
				LiveInterval_t NewLiveInterval;

				//get the decl for this identifier so we can determine it's start point
				Symbol_t *Sym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(
													(*ValItr)->IdName, false);
				SanityCheck(Sym);

				if(Sym->IdPtr->Code == TC_VARDECL)
				{
					CVarDeclTreeNode *TempVar = (CVarDeclTreeNode*)Sym->IdPtr;
					NewLiveInterval.StartPoint = TempVar->DeclPoint;
				}
				else if(Sym->IdPtr->Code == TC_PARMDECL)
				{
					//we dont assign registers to parameters
					return;
				}

				//initialise the end point as the end of this basic block, this
				//will be updated later on if needs be but it serves as a useful
				//initial end point
				NewLiveInterval.EndPoint = StartStmtNo + (int)Bb->Stmts.size();
				NewLiveInterval.Id = *ValItr;
				NewLiveInterval.Active = true;

				//add it
				m_LiveIntervals.push_back(NewLiveInterval);
			}
			else
			{
				//see if it's end point needs to be updated
				if(LiveInt->EndPoint < StartStmtNo + (int)Bb->Stmts.size())
					LiveInt->EndPoint = StartStmtNo + (int)Bb->Stmts.size();
			}
		}
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
LiveInterval_t* CLinearScanAllocator::FindLiveInterval(CIdentifierTreeNode *Id)
{
	list<LiveInterval_t>::iterator LiItr;
	for(LiItr = m_LiveIntervals.begin(); LiItr != m_LiveIntervals.end(); LiItr++)
	{
		if((*LiItr).Id->IdName == Id->IdName)
			return &(*LiItr);
	}

	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::CalculateEndPoints(BasicBlock_t *Bb, int EndStmtNo)
{
	//look at each value that's live on exit and compre it against
	//the current end point stored for it
	if(!Bb->ValsLiveOnExit.empty())
	{
		set<CIdentifierTreeNode*, CompIdentifiers>::iterator ValItr;
		LiveInterval_t *Interval = NULL;

		for(ValItr = Bb->ValsLiveOnExit.begin(); ValItr != Bb->ValsLiveOnExit.end(); ValItr++)
		{
			Interval = FindLiveInterval(*ValItr);

			//compare the end point against this blocks end point
			if(Interval)
			{
				if(Interval->EndPoint < EndStmtNo)
					Interval->EndPoint = EndStmtNo;
			}
		}
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::PerformAllocation(list<BasicBlock_t*> &FunctionCfg, 
																						 int &StackSize, bool DumpIntervals, 
																						 bool DumpAllocations)
{
	ResetAllocator();
	CalculateLiveIntervals(FunctionCfg);

	if(DumpIntervals)
		DumpLiveIntervals();

	//go through each of the live ranges and try to allocate it
	//to a free register, spilling if necessary
	list<LiveInterval_t>::iterator Itr;
	for(Itr = m_LiveIntervals.begin(); Itr != m_LiveIntervals.end(); Itr++)
	{
		LiveInterval_t *NewInterval = &(*Itr);

		//clear out any dead intervals
		PurgeDeadIntervals(NewInterval);

		//see if we can allocate this interval to a register
		if(!AllocateInterval(NewInterval))
			SpillInterval(NewInterval);
	}

	if(DumpAllocations)
		DumpFinalRegisterAllocations();

	//increase the size of the stack frame size to accomodate the spilled live 
	//ranges, this size is in bytes. We need to increase it by four to get the 
	//correct value as we decrease it by 4 each time we spill a value
	if(m_NextStackFrameAddr != 0)
		StackSize += m_NextStackFrameAddr - 4;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::DumpLiveIntervals(void)
{
	//open the file for creation or appending
	ofstream DumpFile;
	DumpFile.open("LiveIntervals.txt", ios::app);

	list<LiveInterval_t>::iterator LiveIntItr;
	for(LiveIntItr = m_LiveIntervals.begin(); LiveIntItr != m_LiveIntervals.end(); LiveIntItr++)
	{
		DumpFile << "Value: " << (*LiveIntItr).Id->IdName << "  " ;
		DumpFile << "Start Point: " << (*LiveIntItr).StartPoint << "  ";
		DumpFile << "End Point: " << (*LiveIntItr).EndPoint << "  " << endl;
	}

	DumpFile.close();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CLinearScanAllocator::AllocateInterval(LiveInterval_t *Interval)
{
	//if the value being allocated is of floating point we automatically
	//spill it to the stack since it's going to be transferred between the 
	//ALU and FPU units when it's used
	CTreeNode *Decl = GetDeclForInterval(Interval);

	if(Decl->Code == TC_VARDECL 
		 && ((CVarDeclTreeNode*)Decl)->VarType == TOKTYPE_FLOATTYPE)
	{
		//return true since we dont want to spill any other
		//values which can reside in GPR's
		AllocateFloatingPointVar(Decl, Interval);
		return true;
	}
	else if(Decl->Code == TC_ARRAYDECL)
	{
		//return true to mimic allocating this interval
		AllocateArrayDecl(Decl, Interval);
		return true;
	}

	//if it's a global variable there's nothing for us to do
	SanityCheck(Decl->Code == TC_VARDECL);
	
	if(((CVarDeclTreeNode*)Decl)->GlobalVar)
		return true;

	//see if we've got any free regs to allocate
	if(m_FreeRegs.empty())
		return false;

	//allocate the first free reg to this interval
	Interval->Reg = m_FreeRegs.front();
	m_FreeRegs.pop_front();
	m_AllocatedValues.push_back(*Interval);
	m_AllocatedValues.sort(CompLiveIntervals());

	//update the var decl that we've just allocated
	CVarDeclTreeNode *VarDecl = (CVarDeclTreeNode*)Decl;
	VarDecl->AllocatedReg = Interval->Reg;
	VarDecl->Spilled = false;

	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::AllocateFloatingPointVar(CTreeNode *Decl, 
																										LiveInterval_t *Interval)
{
	CVarDeclTreeNode *VarDecl = (CVarDeclTreeNode*)Decl;

	VarDecl->Spilled = true;
	VarDecl->SpillPoint = Interval->StartPoint - 1;
	VarDecl->StackFrameAddr = m_NextStackFrameAddr;

	//update for the next spilled var
	m_NextStackFrameAddr += 4;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::AllocateArrayDecl(CTreeNode *Decl, 
																						 LiveInterval_t *Interval)
{
	//get the var decl this array ref is wrapping
	CVarDeclTreeNode *ArrDeclVar = (CVarDeclTreeNode*)Decl->GetChild(ARR_DECL_VAR);

	//if the array has been declared at global scope we dont need to do
	//anything else update it
	if(!ArrDeclVar->GlobalVar)
	{
		ArrDeclVar->Spilled = true;
		ArrDeclVar->SpillPoint = Interval->StartPoint - 1;
		ArrDeclVar->StackFrameAddr = m_NextStackFrameAddr;

		//get the array decl
		CArrayDeclTreeNode *ArrDecl = (CArrayDeclTreeNode*)Decl;
		SanityCheck(ArrDecl->GetChild(ARR_DECL_BOUNDS)->Code == TC_LITERAL);

		//get the bounds value, since this is a tree we need to convert it to 
		//an integer value we can work with
		CLiteralTreeNode *Bounds = (CLiteralTreeNode*)ArrDecl->GetChild(ARR_DECL_BOUNDS);
		
		//update the stack frame amount
		m_NextStackFrameAddr += CUtilFuncs::GetLiteralValue<int>(Bounds) * 4;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::PurgeDeadIntervals(LiveInterval_t *Interval)
{
	//because the active intervals are sorted by increasing start
	//point we can terminate as soon as we reach one which overlaps
	//the interval passed into this function
	list<LiveInterval_t>::iterator Itr;
	for(Itr = m_LiveIntervals.begin(); Itr != m_LiveIntervals.end(); Itr++)
	{
		//if this one is dead skip it
		if(!(*Itr).Active)
			continue;

		if((*Itr).EndPoint >= Interval->StartPoint)
			return;

		//free the allocated register
		m_FreeRegs.push_back((*Itr).Reg);
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::ResetAllocator(void)
{
	//clear out any live intervals which are remaining
	m_LiveIntervals.clear();

	//add in the registers ready for allocation
	m_FreeRegs.clear();
	m_FreeRegs.push_back(REG_EAX);
	m_FreeRegs.push_back(REG_EBX);
	m_FreeRegs.push_back(REG_ECX);
	m_FreeRegs.push_back(REG_EDX);

	m_NextStackFrameAddr = 4;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::SpillInterval(LiveInterval_t *Interval)
{
	//spill the interval which ends the furthest away
	LiveInterval_t *SpillInterval = &m_AllocatedValues.back();
	
	//free the register and reallocate it
	m_FreeRegs.push_back(SpillInterval->Reg);
	SanityCheck(AllocateInterval(Interval));

  //update the variable which has been spilled to indicate this
	CVarDeclTreeNode *SpilledVar = (CVarDeclTreeNode*)GetDeclForInterval(Interval);
	SpilledVar->Spilled = true;
	SpilledVar->SpillPoint = Interval->StartPoint;
	SpilledVar->StackFrameAddr = m_NextStackFrameAddr;

	//update the stack addr for the next spilled var
	m_NextStackFrameAddr += 4;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CLinearScanAllocator::GetDeclForInterval(LiveInterval_t *Interval)
{
	Symbol_t *VarSym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(
		Interval->Id->IdName, false);

	//make sure the symbol was found
	SanityCheck(VarSym);
	return VarSym->IdPtr;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CLinearScanAllocator::DumpFinalRegisterAllocations(void)
{
	ofstream DumpFile;
	DumpFile.open("RegisterAllocations.txt", ios::app);

	list<LiveInterval_t>::iterator Itr = m_LiveIntervals.begin();
	CSymbolTable *SymTable = CSymbolTable::GetSingletonPtr();
	CVarDeclTreeNode *VarDecl = NULL;

	for(; Itr != m_LiveIntervals.end(); Itr++)
	{
		Symbol_t *VarSym = SymTable->LookupIdentifier((*Itr).Id->IdName, false);
		SanityCheck(VarSym);
		
		switch(VarSym->IdPtr->Code)
		{
		case TC_VARDECL:
			{
				VarDecl = (CVarDeclTreeNode*)VarSym->IdPtr;
				break;
			}

		case TC_ARRAYDECL:
			{
				VarDecl = (CVarDeclTreeNode*)VarSym->IdPtr->GetChild(ARR_DECL_VAR);
				break;
			}
		default:
			InternalError("unhandled var type");
		}

		DumpFile << "Value: " << (*Itr).Id->IdName << "  ";

		if(VarDecl->Spilled)
		{
			DumpFile << "Spilled At Location: " << VarDecl->SpillPoint << "  ";
			DumpFile << "Stack Frame Offset: " << -VarDecl->StackFrameAddr << endl;
		}
		else if(VarDecl->GlobalVar)
			DumpFile << "Global Variable, no register required" << endl;
		else
			DumpFile << "Register: " << CUtilFuncs::HardRegisterToStr((*Itr).Reg) << endl;
		
		DumpFile << endl;
	}

	DumpFile.close();
}
//-------------------------------------------------------------