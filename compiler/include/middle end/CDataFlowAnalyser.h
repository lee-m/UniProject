//------------------------------------------------------------------------------------------
// File: CDataFlowAnalyser.h
// Desc: Iterative solver for the various data flow problems
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CDATAFLOWANALYSER_H__
#define __CDATAFLOWANALYSER_H__

//forward decls
struct BasicBlock_t;
class CFunctionDeclTreeNode;

/**
 * Implementation of a worklist based solver for the use-defs and 
 * liveness dataflow equations. The class operates a single function
 * at a time and works but continually solving the equations for each
 * block in the functions CFG until a solution is converged upon. 
 */
class CDataFlowAnalyser
{
public:
	CDataFlowAnalyser(void) {};
	~CDataFlowAnalyser(void) {};

	/** 
	 * Perform data flow analysis for a given function, solving the
	 * use-defs and liveness equations.
	 * @param FnDecl Function decl whose CFG we're analysing.
	 * @param DumpDfInfo True if we should output the computed dataflow
	 *        information to a dump file.
	 */
	void AnalyseFunction(CFunctionDeclTreeNode **FnDecl, bool DumpDfInfo);

private:

	/**
	 * Compute the sets of values which are used and defined for a basic block.
	 * @param Block Basic block we're looking for use-defs in.
	 */
	void ComputeUseDefs(BasicBlock_t *Block);

	/**
	 * Compute liveness information for the current basic blocks in
	 * the worklist which will be the contents of the CFG for the function
	 * passed to AnalyseFunction.
	 */
	void ComputeLivenessInfo(void);

	/**
	 * Subroutine of ComputeLivenessInfo, computes the set of values 
	 * which are live in exit for a basic block.
	 * @param Block Basic block we're processing.
	 * @return True if the liveout information for Block has been 
	 *         altered, false if nothing changed.
	 */
	bool ComputeLiveOutForBlock(BasicBlock_t *Block);

	/**
	 * Subroutine of ComputeLivenessInfo, computes the set of values 
	 * which are live on entry for a basic block.
	 * @param Block Basic block we're processing.
	 * @return True if the live in information has been adjusted, false
	 *         if nothing changed.
	 */
	bool ComputeLiveInForBlock(BasicBlock_t *Block);

	/**
	 * Dump the computed data flow information to file.
	 * @param Graph Control flow graph for a single function whose 
	 *        dataflow contents we're dumping to a file.
	 * @param FuncName Name of the function whose information we're outputting.
	 */
	void DumpDataFlowInformation(list<BasicBlock_t*> &Graph, string FuncName);

	/**
	 * Scan a tree node for var uses, adding any found to the "uses"
	 * set of the basic block.
	 * @param Stmt Generic stmt we're looking for uses in.
	 * @param Block Basic block we're scanning for uses.
	 */
	void ScanStmtForUses(CTreeNode *Stmt, BasicBlock_t *Block);

	/** 
	 * Subroutine of ScanStmtForUses to specifically handle identifier
	 * tree nodes as they have far greater potential for referencing 
	 * other values in the program.
	 * @param Id Identifier node we're looking at for uses of other variables.
	 * @param Block Current basic block being processed where we'll add
	 *        any uses to.
	 */
	void ScanIdentifierNodeForUses(CTreeNode *Id, BasicBlock_t *Block);

	/**
	 * Helper function to compare two sets for equality. Although the 
	 * std::set container allows a functor object to be used when creating
	 * instances, we can have the situation where there are two different
	 * identifier tree nodes which refer to the same values. In this situation
	 * using the std algorithm such as "includes" doesn't work so we have
	 * to compare the sets manually. This approach fixes a bug in the 
	 * solver where we hit an infinite loop as we kept thinking the dataflow
	 * sets had changed when in fact they had converged on a solution.
	 * @param SetOne The first set we're comparing with.
	 * @param SetTwo The set we're comparing the first one with.
	 */
	bool CompareSets(set<CIdentifierTreeNode*, CompIdentifiers> &SetOne,
									 set<CIdentifierTreeNode*, CompIdentifiers> &SetTwo);

	/**
	 * When computing definitions the current stmt we're looking at will 
	 * be stored here so it can be associated with each def as we find them
	 */
	int m_CurrStmtNo;

	/** Worklist of remaining blocks to process. */
	deque<BasicBlock_t*> m_Worklist;
};

#endif