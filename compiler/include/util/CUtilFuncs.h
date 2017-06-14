//------------------------------------------------------------------------------------------
// File: CUtilFuncs.h
// Desc: Various utility functions
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CUTILFUNCS_H__
#define __CUTILFUNCS_H__

//our includes
#include "TokenTypes.h"
#include "CLinearScanAllocator.h"
#include "CParser.h"
#include "CLiteralTreeNode.h"

//forward decls
class CIdentifierTreeNode;
struct BasicBlock_t;
struct DefVar_t;
struct LiveInterval_t;

/**
 * A collection of functions which aren't really tied to any particular
 * class in the compiler, but are used in a few places.
 */
class CUtilFuncs
{
public:
	CUtilFuncs(void) {};
	~CUtilFuncs(void) {};

	/**
	 * Convert a literal tree node into a value of the
	 * correct type. It's the callers responsibility to provide
	 * the correct type as the template parameter T.
	 * @param LitNode The literal node to extract the value from.
	 * @return Value stored inside the literal node.
	 */
	template<typename T>
	static T GetLiteralValue(CTreeNode *LitNode)
	{
		SanityCheck(LitNode->Code == TC_LITERAL);

		CLiteralTreeNode *Lit = (CLiteralTreeNode*)LitNode;
		stringstream ss;

		ss << Lit->LitValue;

		T RetVal;
		ss >> RetVal;

		return RetVal;
	}

	/**
	 * Convert a token type to string.
	 * @param Tok The token type to convert.
	 * @return A human understandable string describing the token.
	 */
	static string TokenTypeToStr(TokenType Tok);

	/**
	 * Helper function to convert an expression parsing context into
	 * a token type to terminate parsing the expression on.
	 * @param Context The expression context to get the terminating token for.
	 * @return A token type to terminate parsing an expression on given the 
	 *         context passed in.
	 */
	static TokenType ConvertExprContextToTokenType(ExprContext Context);

	/**
	 * Create a new name for a compiler generated temporary variable.
	 * @return A new name of the form "CompTemp.XX" where XX is a unique number. 
	 */
	static string GenCompilerTempName(void);

	/**
	 * Create a new name for a label.
	 * @return The name of the new label of the form "LXX" where XX 
	 *         is a unique number.
	 */
	static string GenNewLabel(void);

	/**
	 * Convert a hard-register to it's equilavent name such as "eax" etc.
	 * @param Reg The enumeration we'd like to get the name for.
	 * @return The hard registers name.
	 */
	static string HardRegisterToStr(HardRegister Reg);

	/**
	 * Checks to see if the given expression efers to a floating point type.
	 * @param Expr Expression/statement to check.
	 * @return True if a floating point type is used in Expr, false if it doesn't.
	 */
	static bool ExprRefersToFloatingPointType(CTreeNode *Expr);

	/**
	 * Helper function to get a decl from an arbritrary node, recurses down through
	 * the tree until we hit the root for that particular node type.
	 * @param Node The node we want the _decl for.
	 * @return A _decl tree node for Node.
	 */
	static CTreeNode *GetDeclFromNode(CTreeNode *Node);
};

/**
 * During CFG edge construction we dont want to allow the same block
 * to appear multiple times in either the successors or predecessors so this
 * function is used to compare two blocks IDs for equality.
 */
struct CompBasicBlocks
{
	bool operator()(const BasicBlock_t *BbOne, const BasicBlock_t *BbTwo) const;
};

/**
 * During data flow analysis we want to compare two CIdentifierTreeNode's
 * for equality to determine if we've already added a node to a given set.
 */
struct CompIdentifiers
{
	bool operator()(const CIdentifierTreeNode *IdOne, const CIdentifierTreeNode *IdTwo) const;
};

/**
 * Compare two live intervals for the register allocator by their end points.
 */
struct CompLiveIntervals
{
	bool operator()(const LiveInterval_t &LiOne, const LiveInterval_t &LiTwo) const;
};

#endif
