//------------------------------------------------------------------------------------------
// File: CParser.h
// Desc: Class to peform syntactic and semantic checks on the input source program
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CPARSER_H__
#define __CPARSER_H__

#include "CLexicalScanner.h"
#include "CSymbolTable.h"
#include "CParseTree.h"

//forward decls
class CParseTreeNode;

//the different contexts used for expression parsing
enum ExprContext
{
	EC_INITIALISER,
	EC_ARRAY,
	EC_FUNCCALLPARM,
	EC_CONDITIONAL,
	EC_FUNCCALLPARMS
};

//forward decls
class CArrayRefTreeNode;
class CIdentifierTreeNode;
class CCallExprTreeNode;
class CBinaryExprTreeNode;

class CParser
{
public:
	CParser(void);
	~CParser(void);

	//parse a single source file, returns true if parsing 
	//completed successfully with no errors, otherwise false
	bool ParseSourceFile(const string &Identifier);

private:

	//check for a group of tokens
	bool IsType(TokenType Type);
	bool IsLiteral(TokenType Type);
	bool IsArithmeticOperator(TokenType Tok);
	bool IsBooleanOperator(TokenType Tok);

	//emit a diagnostic
	void Error(const string &Msg);
	void Warning(const string &Msg);

	//if the next token is not the expected one, issue an error
	bool ExpectNextToken(const string &Msg, const Token_t &Tok, TokenType ExpType);

	//keep consuming tokens until we find the end of the current statement
	void SkipToToken(TokenType Tok);

	//production rules
	CParseTreeNode* ParseVariableDeclaration(void);
	CParseTreeNode* ParseArrayDeclaration(CParseTreeNode *Var);
	CParseTreeNode* ParseExpression(ExprContext Context);
	CParseTreeNode* ParseStatement(void);
  CParseTreeNode* ParseIfStatement(void);
	CParseTreeNode* ParseAssignmentStatement(const string &IdentifierName);
	CParseTreeNode* ParseWhileStatement(void);
	CParseTreeNode* ParseFunctionDecl(void);
	CParseTreeNode* ParseReturnStatement(void);
	CParseTreeNode* ParseFunctionCall(const string &FuncName, bool RequireSemiColon);
	CParseTreeNode* ParseFuncCallOrAssignmentExpr(void);

	//subroutines of ParseFunctionDecl
	bool ParseFuncDeclParameterList(CParseTreeNode *ParmList, int &NumParms);
	bool ValidateFuncReturnsInfo(TokenType &RetType);

	//determine the precedence of an operator
	int OperatorPrecedence(TokenType Op);

	//reduce the expression on top of the stack
	bool ReduceExpression(stack<TokenType> &OpStack, stack<CParseTreeNode*> &ValStack);

	//destroy the expression stack if we hit an error during
	//parsing an expression
	void DestroyExpressionStack(stack<CParseTreeNode*> &ValStack);

	//helper method for ParseExpression to parse and verify an single term
	CParseTreeNode* ParseExpressionTerm(stack<TokenType> &OpStack, int &ParenLevel);

	//perform type checking on an expression to see if it's compatible
	//with the requested type
	bool TypeCheckExpression(CParseTreeNode *Expr, TokenType RequiredType);

	//see if a type can be converted to the requested 
	bool TypeConvertsTo(TokenType Orig, TokenType To);

	//see if a literal type matches a correspondnig _type
	bool MatchLiteralTypeToVarType(TokenType VarType, TokenType LiteralType);

	//return true if a given type is valid to appear either side of
	//a Boolean operator
	bool TypeValidForBooleanOperator(TokenType Type);

	//likewise for arithmetic expressions
	bool TypeValidForArithmeticOperator(TokenType Type);

	//given a tree of a given type, extract the underlying type
	TokenType ExtractTypeFromGenericExpr(CParseTreeNode *Expr);
	TokenType ExtractTypeFromArrayRef(CArrayRefTreeNode *ArrRef);
	TokenType ExtractTypeFromIdentifier(CIdentifierTreeNode *Ident);
	TokenType ExtractTypeFromCallExpr(CCallExprTreeNode *CallExpr);
	TokenType ExtractTypeFromBinaryExpr(CBinaryExprTreeNode *BinExpr);

	//are we in the middle of parsing a function decl?
	bool m_ParsingFuncDecl;

	//is a function declaration allowed to appear here?
	bool m_FuncDeclAllowed;

	//the function we're currently parsing
	CParseTreeNode *m_CurrFuncDecl;

	CLexicalScanner m_LexScanner;
	CSymbolTable m_SymTable;
	CParseTree m_ParseTree;

	//a hash table of token strings and their equivalent operator precedence
	//used for fast look up during expression parsing
	map<TokenType, int> m_OpPrecs;
};

#endif
