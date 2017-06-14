//------------------------------------------------------------------------------------------
// File: CParser.h
// Desc: Class to peform syntactic and semantic checks on the input source program
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARSER_H__
#define __CPARSER_H__

#include "CLexicalScanner.h"

//forward decls
class CTreeNode;
class CTreeNodeFactory;
class CSymbolTable;
struct Symbol_t;

/** The different contexts used for expression parsing */
enum ExprContext
{
	//variable initialisations
	EC_INITIALISER,

	//array index access
	EC_ARRAY,

	//parameter to a function call which takes a single parameter
	EC_FUNCCALLPARM,

	//if/while statement conditional
	EC_CONDITIONAL,

	//parameter to a function call which takes more than one parameter
	EC_FUNCCALLPARMS
};

//forward decls
class CTreeNode;
class CArrayRefTreeNode;
class CIdentifierTreeNode;
class CCallExprTreeNode;
class CArithExprTreeNode;
class CVarDeclTreeNode;

/**
 * A recursive descent predictive parser for the langauge. This parses
 * the entire file at once, performing the necessary syntactic and semantic
 * analysis and building the parse tree as it goes. The error recovery
 * technique employed is fairly basic in that it skips to the end of a statement
 * if it's found to be invalid.
 */
class CParser
{
public:
	CParser(void);
	~CParser(void) {};

	/**
	 * Parse a single source file.
	 * @param InputFile Filename of the file to parse.
	 * @return Parse tree for the file is successfull, NULL if any
	 *         errors occurred.
	 */
	CTreeNode* ParseSourceFile(const string &InputFile);

private:

	/**
	 * Determine if the token represents a variable type.
	 * @param Type Token type to check.
	 * @return True if the token represents a variable type, otherwise false.
	 */
	bool IsType(TokenType Type);

	/**
	 * Determine if the token represents a literal token of any 
	 * type - integer/float or string.
	 * @param Type Token type to check.
	 * @return True if the type is a literal type, otherwise false.
	 */
	bool IsLiteral(TokenType Type);

	/**
	 * Determine if the token represents an arithmetic operator.
	 * @param Tok Token type to check.
	 * @return True if the token type is an arithmetic operator, otherwise false.
	 */
	bool IsArithmeticOperator(TokenType Tok);

	/**
	 * Like IsArithmeticOperator only this time for Boolean operators.
	 * @param Tok Token type to check.
	 * @return True if the parameter is a Boolean operator, otherwise false.
	 */
	bool IsBooleanOperator(TokenType Tok);

	/**
	 * Check to see if a token represents a logical Boolean operator
	 * such as "and" or "or".
	 * @param Tok The token type to check.
	 * @return True if the token is a logical Boolean operator, otherwise false.
	 */
	bool IsLogicalOperator(TokenType Tok);

	/**
	 * Like IsLogicalOperator only this time check for the relational
	 * Boolean operators "<", "<=", ">" and ">=".
	 * @param Tok Token type to check.
	 * @return True if the token is a relational Boolean operator, otherwise false.
	 */
	bool IsRelationalOperator(TokenType Tok);

	/**
	 * Issue an error message.
	 * @param Msg Error message to emit.
	 */
	void Error(const string &Msg);

	/**
	 * Issue a warning.
	 * @param Msg Warning message to emit.
	 */
	void Warning(const string &Msg);

	/**
	 * Check that the next token scanned matches an expected type. If it
	 * doesn't an error is issued.
	 * @param Msg Human readable string for the expected token type. This is 
	 *						used in the error message if the check fails.
	 * @param Tok The next token returned from the lexical scanner to check.
	 * @param ExpType The expected type of the token Tok.
	 * @return True if the token matches the expected type, false if it doesn't.
	 */
	bool ExpectNextToken(const string &Msg, const Token_t &Tok, TokenType ExpType);

	/**
	 * Keep consuming tokens until we find the end of the current statement.
	 * This is used during error recovery to skip over the remaining tokens of
	 * an invalid statement.
	 * @param Tok The token type to stop consuming tokens on.
	 */
	void SkipToToken(TokenType Tok);

	/**
	 * Variable declaration production rule.
	 * @return A new tree node for the var decl.
	 */
	CTreeNode* ParseVariableDeclaration(void);

	/**
	 * Array declaration production rule.
	 * @param Var A VAR_DECL tree node we're wrapping in a new array decl.
	 * @return A new tree node for the array declaration.
	 */
	CTreeNode* ParseArrayDeclaration(CVarDeclTreeNode *Var);

	/**
	 * Parse a generic expression.
	 * @param Context The context in which this expression is being parsed.
	 * @return A new tree node of either arith_expr or bln_expr type containing
	 *         the expression.
	 */
	CTreeNode* ParseExpression(ExprContext Context);

	/**
	 * High level statement parsing production rule. Looks at the next 
	 * token and determines which production rule to apply next based upon it.
	 * @return A new tree node representing the statement.
	 */
	CTreeNode* ParseStatement(void);

	/**
	 * Parse an if statement including any statements which comprise the 
	 * "then" and "else" arms of it.
	 * @return A new tree node representing the new statement and it's body.
	 */
  CTreeNode* ParseIfStatement(void);

	/**
	 * Assignment statement production rule.
	 * @param IdentifierName The name of the variable appearing on the LHS
	 *        of the assignment.
	 * @return A new tree node representing the assignment statement.
	 */
	CTreeNode* ParseAssignmentStatement(const string &IdentifierName);

	/**
	 * While statement production rule. Also parses the statements which
	 * make up the body and adds them to the new node returned.
	 * @return A new tree node for the while statement.
	 */
	CTreeNode* ParseWhileStatement(void);

	/**
	 * Parse a function declaration, including the statements which make
	 * up it's body.
	 * @return A new tree node for the function decl.
	 */
	CTreeNode* ParseFunctionDecl(void);

	/**
	 * Return statement production rule.
	 * @return A new tree node for the statement.
	 */
	CTreeNode* ParseReturnStatement(void);

	/**
	 * Parse a call to a function.
	 * @param FuncName The name of the function that is being called.
	 * @param RequireSemiColon True if we expect a semi-colon at the end
	 *        which marks a call to a void function outside of any expression, 
	 *        otherwise we have something like "if(foo())" where we don't require 
	 *        a semi-colon after the function call.
	 * @return A new tree node for the function call.
	 */
	CTreeNode* ParseFunctionCall(const string &FuncName, bool RequireSemiColon);

	/** 
	 * Parse either a call to a function or an assignent expression. Once
	 * it's been determined what we have pass control over to the necessary
	 * production to handle the actual parsing.
	 * @return A new tree node for the function call or assignment expression.
	 */
	CTreeNode* ParseFuncCallOrAssignmentExpr(void);

	/**
	 * Subroutine of ParseFunctionDecl to handle the parsing of a function's
	 * declared parameters.
	 * @param ParmList A parameter list tree node to add each parsed parameter to.
	 * @param NumParms After this function returns, this will be set to the number
	 *        of parsed parameters.
	 * @return True if parsing completed successfully, false on error.
	 */
	bool ParseFuncDeclParameterList(CTreeNode *ParmList, int &NumParms);

	/**
	 * Subroutine of ParseFunctionDecl to handle the parsing of the
	 * "returns" clause of a function declaration.
	 * @param RetType After this function returns, this will be set to the type
	 *        that the function has been declared to return.
	 * @return True if parsing went ok, false on error.
	 */
	bool ValidateFuncReturnsInfo(TokenType &RetType);

	/**
	 * Determine the precedence of an operator. The actual precedences
	 * of each operator are initialised in the constructor.
	 * @param Op The operator to get the precedence for.
	 * @return An integer value representing an operators precedence.
	 */
	int OperatorPrecedence(TokenType Op);

	/**
	 * Reduce the expression on top of the expression stacks.
	 * @param OpStack Stack of arithmetic/Boolean operators.
	 * @param ValStack Stack of expression terms representing the values.
	 * @return True if reduction succeeded, false on error.
	 */
	bool ReduceExpression(stack<TokenType> &OpStack, stack<CTreeNode*> &ValStack);

	/**
	 * Destroy the expression stack if we hit an error during
	 * parsing an expression.
	 * @param ValStack Value stack to destroy.
	 */
	void DestroyExpressionStack(stack<CTreeNode*> &ValStack);

	/**
	 * Helper method for ParseExpression to parse and verify an single term.
	 * @param OpStack Operator stack to add the next operator to.
	 * @param ParenLevel The current nesting level of parenthesis which we need
	 *        to keep up to date when parsing a new expression term.
	 * @return A new tree node for the term.
	 */
	CTreeNode* ParseExpressionTerm(stack<TokenType> &OpStack, int &ParenLevel);

	/**
	 * Perform type checking on an expression to see if it's compatible
	 * with the requested type.
	 * @param Expr The expression to check.
	 * @param RequiredType The required type the expression must be 
	 *        compatible with.
	 */
	bool TypeCheckExpression(CTreeNode *Expr, TokenType RequiredType);

	/**
	 * See if a type can be converted to the requested type, this is 
	 * only possible for implicit conversions between integral -> floating
	 * point values as not information is lost in the process.
	 * @param Orig The original type we're checking.
	 * @param To The type we're checking that Orig can be converted to.
	 * @return True if Orig can be converted to To, otherwise false.
	 */
	bool TypeConvertsTo(TokenType Orig, TokenType To);

	/**
	 * Check if two types are compatible with each other.
	 * @param TypeOne A _type value that a variable was declared with.
	 * @param TypeTwo A _literal or _type we're checking matches the declared type.
	 */
	bool TypesCompatible(TokenType TypeOne, TokenType TypeTwo);

	/**
	 * Return true if a given type is valid to appear either on side of
	 * a Boolean operator.
	 * @param Type The token type to check.
	 * @return True if the token can appear with a Boolean expression, 
	 *         otherwise false.
	 */
	bool TypeValidForBooleanOperator(TokenType Type);

	/**
	 * Like TypeValidForBooleanOperator only this time checking for
	 * arithmetic expressions.
	 * @see TypeValidForBooleanOperator
	 */
	bool TypeValidForArithmeticOperator(TokenType Type);

	/**
	 * Given a tree , extract the underlying type of it.
	 * @param Expr The expression to extract the type from.
	 * @return The type of the expression.
	 */
	TokenType ExtractTypeFromGenericExpr(CTreeNode *Expr);

	/**
	 * Subroutine of ExtractTypeFromGenericExpr to handle
	 * array references.
	 * @param ArrRef Array reference tree node to extract the type from.
	 * @return The type of the array being referenced.
	 */
	TokenType ExtractTypeFromArrayRef(CArrayRefTreeNode *ArrRef);

	/**
	 * Subroutine of ExtractTypeFromGenericExpr to handle identifiers 
	 * which could refer to functions, variables or parameters.
	 * @param Ident Identifier to get the type for.
	 * @return The type of the decl the identifier refers to.
	 */
	TokenType ExtractTypeFromIdentifier(CIdentifierTreeNode *Ident);

	/**
	 * Subroutine of ExtractTypeFromGenericExpr to handle call_exprs. The
	 * type of call_exprs is the return type of the function being called.
	 * @param CallExpr Function call expr we're getting the type of.
	 * @return The return type of the function being called.
	 */
	TokenType ExtractTypeFromCallExpr(CCallExprTreeNode *CallExpr);

	/**
	 * Subroutine of ExtractTypeFromGenericExpr to handle arithmetic expressions.
	 * If the operator used is a Boolean operator, the whole expression is 
	 * of type Boolean otherwise the type of an arithmetic expression is 
	 * defined to be of the type appearing on the LHS.
	 * @param BinExpr Expr to determine the type of.
	 * @return The type of the expression.
	 */
	TokenType ExtractTypeFromArithExpr(CArithExprTreeNode *BinExpr);

	/**
	 * Typechecking of parameters appears in three locations, the code for which
	 * is identical in both cases so it's extracted to a common method here that
	 * gets used instead. If the check fails both Parm and CallExpr will be
	 * destroyed and the token stream skipped to the closing semi-colon.
	 * @param Parm The parameter decl being checked. 
	 * @param FnDecl Function the parameter belongs to.
	 * @param CallExpr Function call expression being processed.
	 * @param ParmId The parameter's index in the function's parameter list.
	 * @return True if type checking succeeded, false on error.
	 */
	bool TypeCheckParameter(CTreeNode *Parm, CTreeNode *FnDecl,
					CTreeNode *CallExpr, unsigned int ParmId);

	/**
	 * Give an error about an ambiguous declaration.
	 * @param Sym The ambiguous symbol.
	 */
	void AmbiguousDeclarationError(Symbol_t *Sym);

	/**
	 * Perform semantic checking on the operands to a Boolean expr.
	 * @param Expr The expression being checked.
	 * @return True if the expression is semantically valid, false if it's not.
	 */
	bool ValidateBooleanExprOperands(CTreeNode *Expr);

	/**
	 * Subroutine of ValidateBooleanExprOperands.
	 * @see ValidateBooleanExprOperands
	 */
	bool ValidateLogicalBlnExprOperands(CTreeNode *Expr);

	/**
	 * Subroutine of ValidateBooleanExprOperands.
	 * @see ValidateBooleanExprOperands
	 */
	bool ValidateRelationalBlnExprOperands(CTreeNode *Expr);

	/**
	 * Subroutine of ParseFunctionDecl to add any declared parameters 
	 * to the current scope, checking for ambiguities along the way.
	 * @param ParmList List of declared parameters for the function.
	 * @return True if no anbiguities were found, otherwise false.
	 */
	bool AddFunctionParmsToSymbolTable(CTreeNode *ParmList);

	/**
	 * Determine if a function declaration is allowed at the point
	 * when this is called. If one isn't allowed, issues an error message
	 * and skips any tokens until the closing "end_function" is found.
	 * @return True if a function declaration is allowed at the current point, 
	 *         otherwise false.
	 */
	bool IsFuncDeclAllowed(void);

	/** True if we're in the middle of parsing a function decl */
	bool m_ParsingFuncDecl;

	/** True if a function declaration is allowed to appear here */
	bool m_FuncDeclAllowed;

	/**
	 * True if we've found a return stmt inside the current function,
	 * we use this because otherwise we'd miss return statements which
	 * are 'hidden' inside if/while statements.
	 */
	bool m_FoundNestedRetStmt;

	/** The function we're currently parsing */
	CTreeNode *m_CurrFuncDecl;

	/** The scanner which provides us with tokens. */
	CLexicalScanner m_LexScanner;

	/** Local copy of the symbol table used during parsing */
	CSymbolTable *m_SymTable;

	/** Tree node factory to build the various tree nodes. */
	CTreeNodeFactory *m_ParseTreeBuilder;

	/** Number of errors which occurred during parsing */
	int m_ErrorCount;

	/**
	 * A hash table of token strings and their equivalent operator precedence
	 * used for fast look up during expression parsing.
	 */
	map<TokenType, int> m_OpPrecs;
};

#endif
