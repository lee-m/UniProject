//------------------------------------------------------------------------------------------
// File: CTreeNode.h
// Desc: Encapsulation of a tree node used as the IR for the compiler.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CTREENODE_H__
#define __CTREENODE_H__

#include <vector>
using namespace std;

//forward decls
class TiXmlElement;

/** The different types of trees that can be built by CTreeNodeFactory. */
enum TreeCode
{
	TC_IDENTIFIER = 0,
	TC_VARDECL,
	TC_ARRAYDECL,
	TC_LITERAL,
	TC_ARITH_EXPR,
	TC_BOOLEAN_EXPR,
	TC_UNARY_EXPR,
	TC_ARRAY_REF,
	TC_IFSTMT,
	TC_WHILESTMT,
	TC_ASSIGN_EXPR,
	TC_PARAMETER_LIST,
	TC_FUNCTIONDECL,
	TC_PARMDECL,
	TC_RETURNSTMT,
	TC_CALLEXPR,
	TC_ROOT,
	TC_LABEL,
	TC_GOTOEXPR,
	TC_LITERALREF
};

/**
 * The core representation of the program used by the compiler from the 
 * front-end down through the back-end. Every tree node created has a 
 * "type" attribute so although every tree node can be treated in a generic
 * fashion through CTReeNode*, it's possible to determine exactly what flavour
 * of tree is being worked with.
 * <br><br>
 * Creation of new tree nodes is controlled through the CTreeNode factory,
 * for more information about why this is done see the documentation for 
 * that class.
 * <br><br>
 * There are two possible states that a tree node can be in depending on
 * what phase of compilation is in progress. During parsing all trees will
 * be in a form which closely matches the declared source code to allow syntax
 * and semantic checks to be performed. After parsing these tree are then 
 * lowered into a form known as "MIR" - middle-end intermediate representation
 * which includes additional tree types not found during parsing. The same tree
 * structures are used as they provide much of the functionality required for
 * manipulating and storing various information for each language construct.
 * <br><br>
 * The process of lowering a parse tree into it's equivalent form is specific
 * to type of node being lowering so each specialisation of this class is
 * required to implement the necessary functionality to handle it.
 */
class CTreeNode
{
public:
	CTreeNode(void) {}
	virtual ~CTreeNode(void) {}

	/** What type of tree this is. */
	TreeCode Code;

	/**
	 * Output the contents of this tree node to an XML document to 
	 * show the heirarchal structure of the parse tree. The contents of
	 * the file doesn't include any new information about the input
	 * program, it's merely intented to show the programmer how the 
	 * compiler views the program.
	 * @param XmlNode A parent XML document node to which this tree node
	 *        will output it's contents to.
	 */
	virtual void DumpAsParseTreeNode(TiXmlElement *XmlNode) = 0;

	/**
	 * Similiar to DumpAsParseTreeNode but instead this time the
	 * output file shows the program after the MIR lowering has 
	 * taken place. This is used to show the results of the various 
	 * transformations the compiler has applied such as control flow
	 * flattening and complex expression decomposition.
	 * @param OutFile File handle this node outputs it's contents to.
	 */
	virtual void DumpAsMIRTreeNode(ofstream &OutFile) = 0;

	/** Destroy this node and any children. */
	virtual void DestroyNode(void);

	/**
	 * Add a new tree node as a child of this one, optionally 
	 * specifying the position it should have.
	 * @param NewChild The new tree node to add, should never be NULL.
	 * @param Pos Position to insert the new child, if the value is
	 *            -1 then it's added as the last child.
	 */
	void AddChild(CTreeNode *NewChild, unsigned int Pos = -1);

	/**
	 * Find out the number of children this tree node has.
	 * @return The number of children added.
	 */
	unsigned int GetNumChildren(void);

	/**
	 * Get a required child node, this doesn't return a l-value so 
	 * can't be used for modifying the underlying child node, for that
   * use GetChildPtr.
	 * @param Id The child node to return, will cause an ICE if
	 *           Id > GetNumChildren().
	 * @return A pointer to the tree node at the requested position.
	 */
	CTreeNode *GetChild(unsigned int Id);

	/**
	 * Identical to GetChild only this function returns an l-value
	 * so the contents can be changed if reguired.
	 * @see GetChild
	 * @return A pointer to the requested child which can be used as an l-value.
	 */
	CTreeNode **GetChildPtr(unsigned int Id);

	/**
	 * Rewrite this node into MIR form, adding any newly created
	 * statements to the provided tree.
	 * @param Stmts The current MIR tree that is being built.
	 * @return The last statement created from this function so it
	 *         can be used in any callers if required.
	 */
	virtual CTreeNode* RewriteIntoMIRForm(CTreeNode *Stmts) = 0;

	/**
	 * In some cases we need to copy around tree nodes, we
	 * can't simply do a = b in that case because we'll crash when
	 * destroying the node since there are two pointers to the same
	 * piece of memory. Instead, use this function so the above
	 * snippet is now: a = b->CreateCopy(). Depending on the type of node
	 * being copied this could result is quite a lot of extra memory being
	 * allocated so should be used with caution.
	 */
	virtual CTreeNode *CreateCopy(void) = 0;

	/**
	 * Due to the complexities with accessing an array element in the back-end, 
	 * we replace all instances of array references with references to a 
	 * temporarary variable initialised to the array element being access.
	 * @param Node The node to (optionally) replace.
	 * @param Stmts Global MIR tree being built.
	 */
	CTreeNode* ReplaceArrayRefs(CTreeNode *Node, CTreeNode *Stmts);

	/**
	 * Replaces any uses of a call expr in the expression by creating
	 * a new temporary variable which is initialised to the return
	 * value of the function. The input node is then replaced with
	 * an identifier node which refers to the newly created variable.
	 * @param Node Expression term to replace if required.
	 * @param Stmts Global MIR tree to add the new statements to.
	 * @return If Node is not a call_expr, returns Node. Otherwise returns
	 *         an identifier node referring to a new temporary variable.
	 */
	CTreeNode* ReplaceCallExpr(CTreeNode *Node, CTreeNode *Stmts);
protected:

	/** Any child nodes. */
	vector<CTreeNode*> Children;
};

#endif
