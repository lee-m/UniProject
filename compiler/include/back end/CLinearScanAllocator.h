//------------------------------------------------------------------------------------------
// File: CLinearScanAllocator.h
// Desc: Implementation of the linear scan register allocation algorithm
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CLINEARSCANALLOCATOR_H__
#define __CLINEARSCANALLOCATOR_H__

//forward decls
class CIdentifierTreeNode;
class CTreeNode;
struct BasicBlock_t;
struct LiveInterval_t;

//available general purpose hard registers we an allocate
enum HardRegister
{
	REG_NONE = -1,

	//general purpose regs
	REG_EAX = 0,
	REG_EBX,
	REG_ECX,
	REG_EDX,
};

struct LiveInterval_t
{
	LiveInterval_t(void) : StartPoint(0), EndPoint(0), Reg(REG_NONE), Id(NULL) {}

	//start and end statements for this interval
	int StartPoint;
	int EndPoint;

	//the register allocated to this interval, NULL if none
	HardRegister Reg;

	//value this range is for
	CIdentifierTreeNode *Id;

	//is this interval active?
	bool Active;
};

class CLinearScanAllocator
{
public:
	CLinearScanAllocator(void) {};
	~CLinearScanAllocator(void) {};

	//perform local allocation for a single function
	void PerformAllocation(list<BasicBlock_t*> &FunctionCfg, int &StackSize, 
												 bool DumpIntervals, bool DumpFinalAllocations);

private:

	//reset the allocator - delete all live intervals, unallocate
	//all registers etc ready for the next round of allocation. This
	//gets called automatically each time RunAllocation is invoked
  void ResetAllocator(void);

	//given a control flow graph, compute the set of live intervals 
	//which are sorted in order of increasing start point
	void CalculateLiveIntervals(list<BasicBlock_t*> &FuncCfg);

	//subroutine of CalculateLiveIntervals which adds any live intervals
	//for any values live on entry for a basic block which aren't already 
	//contained in our list 
	void AddNewLiveIntervals(BasicBlock_t *Bb, int StartStmtNo);

	//subroutine of CalculateLiveIntervals to update the end point
	//of any live intervals which contain values that are live on exit
	void CalculateEndPoints(BasicBlock_t *Bb, int EndStmtNo);

	//purge any intervals whose endpoint is less than the current
	//intervals start point, releasing their allocated registers back
	//into the free pool for re-allocation
	void PurgeDeadIntervals(LiveInterval_t *CurrInterval);

	//determine if we've already added a live interval for the value ID
	LiveInterval_t* FindLiveInterval(CIdentifierTreeNode *Id);

	//dump the computed live intervals out to a text file for debugging
	void DumpLiveIntervals(void);

	//attempt to allocate an interval to a hard register, returns true if an 
	//appropiate register was found otherwise it returns false to indicate 
	//an existing live interval needs to be spilled to make way for it
	bool AllocateInterval(LiveInterval_t *Interval);

	//we've been unable to allocate a hard register for a variable so
	//spill it to the stack frame
	void SpillInterval(LiveInterval_t *Interval);

	//get a _decl tree node for an associated live interval
	CTreeNode *GetDeclForInterval(LiveInterval_t *Interval);

	//subroutines of AllocateInterval to handle array's and floating
	//point vars which need special handling
	void AllocateFloatingPointVar(CTreeNode *Decl, LiveInterval_t *Interval);
	void AllocateArrayDecl(CTreeNode *Decl, LiveInterval_t *Interval);

	//output the registers allocated to each interval or their
	//stack frame address to a dump file
	void DumpFinalRegisterAllocations(void);

	//live intervals sorted in increasing start point
	list<LiveInterval_t> m_LiveIntervals;

	//those live intervals which are still live, sorted by
	//increasing start point
	list<LiveInterval_t> m_AllocatedValues;

	//pool of free registers
	list<HardRegister> m_FreeRegs;

	//variable to keep track of the current stack frame address 
	//we assign to the next variable that's spilled
	int m_NextStackFrameAddr;
};

#endif