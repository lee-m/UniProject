//------------------------------------------------------------------------------------------
// File: CControlFlowGraph.h
// Desc: Partition the list of MIR statements for a function into basic blocks which 
//       forms the CFG
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CCONTROLFLOWGRAPH_H__
#define __CCONTROLFLOWGRAPH_H__

#include "CIdentifierTreeNode.h"
#include "CUtilFuncs.h"

//forward decls
class CFunctionDeclTreeNode;

/**
 * Representation of a single basic block within the CFG. The edges 
 * between nodes are represented by Preds and Succs, together these 
 * give the flow of control for a function.
 */
struct BasicBlock_t
{
	BasicBlock_t() : ExitBb(false), BbId(0), Fallthru(false) {}

	/** Sequence of statements contained by this block. */
	list<CTreeNode*> Stmts;

	/** Successors for this block. */
	set<BasicBlock_t*, CompBasicBlocks> Successors;

	/** Predecessors for this block. */
	set<BasicBlock_t*, CompBasicBlocks> Predecessors;

	/** Is this block an artificial exit block? */
	bool ExitBb;

	/** ID of this basic block. */
	int BbId;

	/** 
	 * Can this block fallthru to the next in the chain? This can 
	 * happen with if statements which fallthru is the cond_expr is false.
	 */
	bool Fallthru;

	/** Values which are defined in this block. */
	set<CIdentifierTreeNode*, CompIdentifiers> DefinedVars;

	/** Uses of values within this block. */
	set<CIdentifierTreeNode*, CompIdentifiers> UsedVars;

	/** Values live on entry to this block. */
	set<CIdentifierTreeNode*, CompIdentifiers> ValsLiveOnEntry;

	/** Values live on exit from this block. */
	set<CIdentifierTreeNode*, CompIdentifiers> ValsLiveOnExit;
};

//iterator typedefs to make changing the containers easier
typedef set<BasicBlock_t*, CompBasicBlocks>::iterator BasicBlockItr;
typedef list<CTreeNode*>::iterator BasicBlockStmtItr;

/**
 * Control flow graph construction pass of the compiler to be run
 * right after MIR lowering.
 */
class CControlFlowGraph
{
public:
	CControlFlowGraph(void) {}
	~CControlFlowGraph(void) {}

	/**
	 * Build the CFG for a function. This occurrs in two passes
	 * over the function's statements. The first partitions the 
	 * statements into basic blocks with the second pass computing
	 * the edges between them.
	 * @param FnDecl The function declaration we want to calculate the CFG for.
	 * @param DumpGraph If this is true, dump the computed graph to a file.
	 */
	void BuildControlFlowGraph(CFunctionDeclTreeNode **FnDecl, bool DumpGraph);

private:

	/**
	 * Partition the input statements into a series of basic blocks.
	 * @param FnDecl The function declaration we're processing.
	 */
	void FindBasicBlocks(CFunctionDeclTreeNode **FnDecl);

	/**
	 * Build the edges between the basic blocks using the 
	 * edge information stored during basic block partitioning.
	 * @param FnDecl Function declaration whose CFG we compute the
	 *        edges for.
	 */
	void ComputeEdges(CFunctionDeclTreeNode **FnDecl);

	/**
	 * Determine if the statement passed is classified as a leader
	 * statement that marks the beginning of a new basic block.
	 * @param Stmt Statement to check.
	 * @return True if Stmt is a leader otherwise false.
	 */
	bool IsLeader(CTreeNode *Stmt);

	/**
	 * Output the CFG for a function to a file for debugging.
	 * @param FnDecl Function declaration whose CFG we're outputting.
	 */
	void DumpControlFlowGraph(CFunctionDeclTreeNode *FnDecl);

	/**
	 * When we're building the edges of the CFG, we don't want to do a linear
	 * search over all the basic blocks to find predecessors and successors as this
	 * would require 3 passes to complete - 1 to find the blocks and 1 each for 
	 * preds/succs. To speed things up when we find the basic blocks, we store their 
	 * entry label and exit jump location (if present) in the following multimap along 
	 * with the corresponding basic block. Now when we go to build the edges for say 
	 * successors, we can use the standard algorithm equal_range to find the basic 
	 * block whose entry label matches the current block's exit jump. Similarly for 
	 * predecessors we can use the same equal_range function, only this time the 
	 * returned results will be all the blocks which have an exit jump to the current 
	 * blocks entry label. The multimap search algorithms are logarithmic in nature so
	 * we reduce the complexity by an order of magnitude.
	 */
	multimap<string, BasicBlock_t*> m_Edges;
};

#endif
