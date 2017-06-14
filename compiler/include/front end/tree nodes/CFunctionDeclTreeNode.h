//------------------------------------------------------------------------------------------
// File: CFunctionDefTreeNode.h
// Desc: Representation of a function within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CFUNCTIONDECLTREENODE_H__
#define __CFUNCTIONDECLTREENODE_H__

#include "CTreeNode.h"
#include "TokenTypes.h"
#include "CControlFlowGraph.h"

//forward decls
struct BasicBlock_t;

/**
 * Declaration of a function within the IR. Includes the statements
 * which make up the body and any declared parameters. This 
 * representation is used throughout the compiler so has properties
 * which aren't set upon creation. The size of the stack frame is initially
 * calculated during MIR lowering which is set big enough to hold all 
 * the parameters, this is then later revised after local register allocation
 * to take into account any spilled local variables. The CFG is not set
 * until the CFG pass run right after MIR lowering, this will also contain
 * any dataflow information attached to each block in the graph.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; 0. Parameter list <br>
 * &nbsp;&nbsp; 1 -> N. Statements making up the function body.
 */
class CFunctionDeclTreeNode : public CTreeNode
{
private:
	CFunctionDeclTreeNode(void);
	~CFunctionDeclTreeNode(void) {}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

public:

	/** Return type of the function. */
	TokenType RetType;

	/** Number of declared parameters. */
	int NumParameters;

	/** Name of the function. */
	string Name;

	/** CFG for this function. */
	list<BasicBlock_t*> FuncCFG;

	/** Stack frame size for parameters and spilled local variables. */
	int StackFrameSize;

	/**
	 * We need to know where the parameters end and the spilled
	 * vars begin in the stack frame when we're loading a 
	 * spilled value so we store the offset in bytes to the start
	 * of the spilled variables.
	 */
	int SpillVarsOffset;

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	void DumpAsParseTreeNode(TiXmlElement *XmlNode);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	void DumpAsMIRTreeNode(ofstream &OutFile);

	/**
	 * Inherited from CTreeNode. Overriden here so we can clean
	 * up the CFG before destroying ourselves.
	 * @see CTreeNode.
	 */
	void DestroyNode();

	/**
	 * Process each statement in the function to convert them to 
	 * MIR form, adding the new statements to the function body
	 * before returning the new function in MIR form.
	 * @see CTreeNode.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Create a deep copy of this node, including all parameters
	 * and statements. Not something which should be done if 
	 * possible as it will be expensive if the function is large.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
