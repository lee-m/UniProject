//------------------------------------------------------------------------------------------
// File: CVarDeclTreeNode.h
// Desc: Variable declaration within the parse tree
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CVARDECLTREENODE_H__
#define __CVARDECLTREENODE_H__

#include "CTreeNode.h"
#include "TokenTypes.h"
#include "CLinearScanAllocator.h"

//forward decls
class CIdentifierTreeNode;

/**
 * Representation of a variable declaration in the IR. Since the tree
 * structures are used throughout the compiler we store back-end specific
 * information about each variable such as it's allocated register and
 * whether it's been spilled or not. These attributes are not initially
 * set and are only valid after dataflow analysis and register allocation.
 * <br><br>
 * <b>Children:</b><br>
 * &nbsp;&nbsp; None.
 */
class CVarDeclTreeNode : public CTreeNode
{
private:
	CVarDeclTreeNode(void);
	~CVarDeclTreeNode(void)	{}

	//only CTreeNodeFactory can instantiate tree nodes
	friend class CTreeNodeFactory;

	/**
	 * Subroutine of RewriteIntoMIR form to handle the simplification
	 * of a unary_expr initialisation expression.
	 * @param NewVarDecl Temporary variable declaration to hold the result
	 *        of the simplification.
	 * @param InitExpr Original initialisation expression we're processing.
	 * @param Stmts Global MIR tree to add intermediate tree nodes to.
	 * @return Assignment expression initialising NewVarDecl with the result
	 *         of the simplification.
	 */

	CTreeNode* SimplifyUnaryInitExpr(CVarDeclTreeNode *NewVarDecl, CTreeNode *InitExpr, 
																	 CTreeNode *Stmts);

	/**
	 * Subroutine of RewriteIntoMIR for to handle the simplification
	 * of a literal initialisation expression.
	 * @param InitExpr The original initialisation expression.
	 * @param Stmts Global MIR tree being built.
	 * @return A reference to the literal in memory or an assignment expression
	 *         to initialise this variable to the literal.
	 */
	CTreeNode* SimplifyLiteralInitExpr(CTreeNode *InitExpr, CTreeNode *Stmts);

	/**
	 * Subroutine of RewriteIntoMIR form to handle the simplification
	 * of call_expr initialisation expression.
	 * @param InitExpr The original initialisation expression.
	 * @param Stmts Global MIR tree to add intermediate tree nodes to.
	 * @param VarType The type of the temporary variable used for simplification.
	 * @return Assignment expression initialising the temporary variable
	 *         with the results of simplification.
	 */
	CTreeNode* SimplifyCallExprInitExpr(CTreeNode *InitExpr, CTreeNode *Stmts,
																			TokenType VarType);

	/**
	 * Soubroutine of RewriteIntoMIR form to handle the simplification
	 * of Boolean initialisation expressions.
	 * @param InitExpr Original initialisation expression.
	 * @param Stmts Global MIR tree being built to add intermediate 
	 *        statements to.
	 * @param NewVarDecl Compiler generated temporary variable declaration
	 *        which is initialised with the results of the simplification.
	 * @return Assignment expression initialising NewVarDecl with the results.
	 */
	CTreeNode* SimplifyBooleanInitExpr(CTreeNode *InitExpr, CTreeNode *Stmts,
																		 CVarDeclTreeNode *NewVarDecl);
public:

	/** Identifier for this declaration */
	CIdentifierTreeNode *VarId;

	/**
	 * The expression which this variable is initialised with, if the variable
	 * isn't initialised at the point of declaration this field will be NULL
	 */
	CTreeNode *InitExpression;

	/** True if it's const qualified. */
	bool IsConst;

	/** The underlying type of the variable. */
	TokenType VarType;

	/**
	 * True if this value was spilled to the stack at some
	 * point during it's live range. Set during register allocation.
	 */
	bool Spilled;

	/**
	 * If we have been spilled, this is the point at which
	 * we was. Used during code generation to determine if the
	 * value needs to be reloaded/read from memory at each use 
	 * after this point in the code.
	 */
	int SpillPoint;

	/**
	 * Address in the stack frame this var was spilled to, this is
	 * where we can reload the value into a register if needed
	 */
	int StackFrameAddr;

	/**
	 * If we haven't been spilled, this is the register
	 * that we've been allocated.
	 */
	HardRegister AllocatedReg;

	/** Is this a global variable? */
	bool GlobalVar;

	/**
	 * When we lower the decl into MIR form, this is the statement number 
	 * which declares it. This is set during data flow analysis and is used
	 * during register allocation to determine it's live interval.
	 */
	int DeclPoint;

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
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	void DestroyNode(void);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts);

	/**
	 * Inherited from CTreeNode.
	 * @see CTreeNode.
	 */
	CTreeNode* CreateCopy(void);
};

#endif
