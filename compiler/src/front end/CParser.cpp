//------------------------------------------------------------------------------------------
// File: CParser.cpp
// Desc: Class to peform syntactic and semantic checks on the input source program
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CParser.h"
#include "CTreeNodeFactory.h"
#include "CSymbolTable.h"
#include "CVarDeclTreeNode.h"
#include "CRootTreeNode.h"
#include "CArrayDeclTreeNode.h"
#include "CIdentifierTreeNode.h"
#include "CBooleanExprTreeNode.h"
#include "CParameterDeclTreeNode.h"
#include "CFunctionDeclTreeNode.h"
#include "CLiteralTreeNode.h"
#include "CArithExprTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CCallExprTreeNode.h"
#include "CGlobalDataStore.h"
#include "CBuiltinFuncMngr.h"

//-------------------------------------------------------------
CParser::CParser(void)
{
	//initialise the precedence table
	m_OpPrecs.insert(make_pair(TOKTYPE_NOT, 7));
	m_OpPrecs.insert(make_pair(TOKTYPE_DIVIDE, 6));
	m_OpPrecs.insert(make_pair(TOKTYPE_MULTIPLY, 6));
	m_OpPrecs.insert(make_pair(TOKTYPE_PLUS, 5));
	m_OpPrecs.insert(make_pair(TOKTYPE_MINUS, 5));
	m_OpPrecs.insert(make_pair(TOKTYPE_LESSTHAN, 4));
	m_OpPrecs.insert(make_pair(TOKTYPE_LESSTHANEQ, 4));
	m_OpPrecs.insert(make_pair(TOKTYPE_GREATTHAN, 4));
	m_OpPrecs.insert(make_pair(TOKTYPE_GREATTHANEQ, 4));
	m_OpPrecs.insert(make_pair(TOKTYPE_EQUALITY, 3));
	m_OpPrecs.insert(make_pair(TOKTYPE_NOTEQ, 3));
	m_OpPrecs.insert(make_pair(TOKTYPE_AND, 2));
	m_OpPrecs.insert(make_pair(TOKTYPE_OR, 1));

	m_ParsingFuncDecl = false;
	m_FuncDeclAllowed = true;
	m_ErrorCount = 0;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseSourceFile(const string &InputFile)
{
	//the scanner will report any errors about not being able to open the file
	if(!m_LexScanner.SetInputFile(InputFile))
		return NULL;

	m_ParseTreeBuilder = CTreeNodeFactory::GetSingletonPtr();
	m_SymTable = CSymbolTable::GetSingletonPtr();

	//create our root parse tree node
	CRootTreeNode *Root;
	Root = (CRootTreeNode*)m_ParseTreeBuilder->BuildRootNode();

	while(m_LexScanner.HasMoreTokens())
	{
		//function decls can only appear at global scope
		m_FuncDeclAllowed = true;

		CTreeNode *Stmt = ParseStatement();

		//we're only allowed function and var decls at global scope
		if(Stmt)
		{
			if(Stmt->Code == TC_FUNCTIONDECL
				 || Stmt->Code == TC_VARDECL
				 || Stmt->Code == TC_ARRAYDECL)
			 Root->AddChild(Stmt);
			else
			{
				Error("only variable and function declarations are allowed at global scope");
				Stmt->DestroyNode();
				continue;
			}
		}
	}

	//don't return the parse tree if something went wrong
	if(m_ErrorCount > 0)
	{
		Root->DestroyNode();
		return NULL;
	}

	return Root;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::Error(const string &Msg)
{
	cout << "error: " << m_LexScanner.CurrentLineNumber() << " : " << Msg << endl;
	++m_ErrorCount;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::Warning(const string &Msg)
{
	cout << "warning: " << m_LexScanner.CurrentLineNumber() << " : " << Msg << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ExpectNextToken(const string &Msg, const Token_t &InTok,
															TokenType ExpType)
{
	//if the next token isn't ExpType, emit an error
	if(InTok.Type != ExpType)
	{
		Error("expected " + Msg + " before '" + InTok.TokenValue + "'");
		return false;
	}

	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::IsType(TokenType Type)
{
	return Type == TOKTYPE_STRINGTYPE
					|| Type == TOKTYPE_INTEGERTYPE
					|| Type == TOKTYPE_BOOLEANTYPE
					|| Type == TOKTYPE_FLOATTYPE;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::IsLiteral(TokenType Type)
{
	return Type == TOKTYPE_STRINGLITERAL
					|| Type == TOKTYPE_INTLITERAL
					|| Type == TOKTYPE_FLOATLITERAL
					|| Type == TOKTYPE_TRUE
					|| Type == TOKTYPE_FALSE;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::IsArithmeticOperator(TokenType Tok)
{
	return Tok == TOKTYPE_PLUS
					|| Tok == TOKTYPE_MINUS
					|| Tok == TOKTYPE_DIVIDE
					|| Tok == TOKTYPE_MULTIPLY
					|| Tok == TOKTYPE_OPENPAREN
					|| Tok == TOKTYPE_CLOSEPAREN;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::IsBooleanOperator(TokenType Tok)
{
	return Tok == TOKTYPE_LESSTHAN
					|| Tok == TOKTYPE_LESSTHANEQ
					|| Tok == TOKTYPE_GREATTHAN
					|| Tok == TOKTYPE_GREATTHANEQ
					|| Tok == TOKTYPE_NOT
					|| Tok == TOKTYPE_NOTEQ
					|| Tok == TOKTYPE_EQUALITY
					|| Tok == TOKTYPE_OR
					|| Tok == TOKTYPE_AND;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::IsLogicalOperator(TokenType Tok)
{
	return Tok == TOKTYPE_AND
				 || Tok == TOKTYPE_OR
				 || Tok == TOKTYPE_NOT;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::IsRelationalOperator(TokenType Tok)
{
	return Tok == TOKTYPE_LESSTHAN
				 || Tok == TOKTYPE_LESSTHANEQ
				 || Tok == TOKTYPE_GREATTHAN
				 || Tok == TOKTYPE_GREATTHANEQ
				 //equality and not equal isn't really a relational operator but it 
				 //has the same constraints with regards to it's operands
				 || Tok == TOKTYPE_EQUALITY
				 || Tok == TOKTYPE_NOTEQ;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::SkipToToken(TokenType Tok)
{
	Token_t TempToken = m_LexScanner.GetNextToken();

	while(TempToken.Type != Tok && m_LexScanner.HasMoreTokens())
		TempToken = m_LexScanner.GetNextToken();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseVariableDeclaration(void)
{
	Token_t VarToken = m_LexScanner.GetNextToken();

	//the name of the variable and whether it's const qualifed or not
	bool IsConstQualified = false;
	string VarName;
	TokenType VarType;

	//first token must either be the type for the variable or the const qualifier
	if(VarToken.Type != TOKTYPE_CONST
		 && !IsType(VarToken.Type))
	{
		Error("expected type specfier or 'const' before '" + VarToken.TokenValue + "'");
		return NULL;
	}

	//if the variable is const qualified, get the type otherwise we already have
	//it in VarToken
	if(VarToken.Type == TOKTYPE_CONST)
	{
		IsConstQualified = true;
		VarToken = m_LexScanner.GetNextToken();

		if(!IsType(VarToken.Type))
		{
			Error("expected type specifier before '" + VarToken.TokenValue + "'");
			return NULL;
		}
	}

	VarType = VarToken.Type;

	//get the name for the variable
	VarToken = m_LexScanner.GetNextToken();
	VarName = VarToken.TokenValue;

	//see if the declaration of the variable conflicts with one previously declared
	Symbol_t *VarSym = m_SymTable->LookupIdentifier(VarName, false);

	if(VarSym)
	{
		SkipToToken(TOKTYPE_SEMICOLON);
		AmbiguousDeclarationError(VarSym);
		return NULL;
	}

	if(!ExpectNextToken("identifier", VarToken, TOKTYPE_ID))
		return NULL;

	//we've now got enough information to build our tree node
	CVarDeclTreeNode *NewVarDecl;
	NewVarDecl = (CVarDeclTreeNode*)m_ParseTreeBuilder->BuildVarDecl(
		VarToken.TokenValue, IsConstQualified, VarType);

	//determine if it's global or not
	NewVarDecl->GlobalVar = m_SymTable->AtGlobalScope();

	//if we've got a semi-colon we're all done
	if(!IsConstQualified && m_LexScanner.PeekNextToken().Type == TOKTYPE_SEMICOLON)
	{
		m_LexScanner.ClearPeekedTokens();
		m_SymTable->AddSymbol(VarName, NewVarDecl, VarToken.LineNumber,
			NewVarDecl->VarType);

		if(m_SymTable->AtGlobalScope())
			CGlobalDataStore::GetSingletonPtr()->AddGlobalVar(NewVarDecl);

		return NewVarDecl;
	}

	//we've either got an array declaration or assignment expression,
	//determine which one
	VarToken = m_LexScanner.GetNextToken();

	//a const declared variable must be initialised when declared
	if(IsConstQualified && VarToken.Type != TOKTYPE_ASSIGNMENT)
	{
		Error("const variable " + VarName + " declared 'const' but not initialised");
		NewVarDecl->DestroyNode();
		return NULL;
	}

	if(VarToken.Type == TOKTYPE_ASSIGNMENT)
	{
		//parse the initialisation expression
		CTreeNode *InitExpr = ParseExpression(EC_INITIALISER);

		//global vars can't be initialised at the point of declaration
		if(NewVarDecl->GlobalVar 
			 && (InitExpr->Code != TC_IDENTIFIER || InitExpr->Code != TC_LITERAL))
		{
			Error("illegal initialisation of global variable '" +
				NewVarDecl->VarId->IdName + "'");

			NewVarDecl->DestroyNode();
			InitExpr->DestroyNode();
			return NULL;
		}

		//make sure the initialisation expr type is compatible with
		//the type of the variable
		if(InitExpr)
		{
			if(!TypeCheckExpression(InitExpr, VarType))
			{
				NewVarDecl->DestroyNode();
				InitExpr->DestroyNode();
				SkipToToken(TOKTYPE_SEMICOLON);
				return NULL;
			}

			NewVarDecl->InitExpression = InitExpr;
		}
		else
		{
			NewVarDecl->DestroyNode();
			SkipToToken(TOKTYPE_SEMICOLON);
			return NULL;
		}

		Token_t r = m_LexScanner.GetNextToken();
		if(!ExpectNextToken("';'", r, TOKTYPE_SEMICOLON))
		{
			NewVarDecl->DestroyNode();
			SkipToToken(TOKTYPE_SEMICOLON);
			return NULL;
		}

		m_SymTable->AddSymbol(VarName, NewVarDecl, VarToken.LineNumber,
			NewVarDecl->VarType);

		if(m_SymTable->AtGlobalScope())
			CGlobalDataStore::GetSingletonPtr()->AddGlobalVar(NewVarDecl);

		return NewVarDecl;
	}
	else if(VarToken.Type == TOKTYPE_OPENSQRPAREN)
		return ParseArrayDeclaration(NewVarDecl);

	//don't know what we could have here so bail out
	Error("expected '=' or '[' before '" + VarToken.TokenValue + "'");

	NewVarDecl->DestroyNode();
	SkipToToken(TOKTYPE_SEMICOLON);
	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseArrayDeclaration(CVarDeclTreeNode *Var)
{
	//Var is the variable we're wrapping in an array declaration
	SanityCheck(Var);

	//the decl node which is going to store this array
	CArrayDeclTreeNode *NewArrDecl = NULL;
	CTreeNode *Bounds = NULL;

	//handle the bounds, the expr used must be an integer literal
	Token_t BoundsTok = m_LexScanner.GetNextToken();

	if(BoundsTok.Type != TOKTYPE_INTLITERAL)
	{
		Error("illegal array bounds expression for array '" + Var->VarId->IdName + "'");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	Bounds = m_ParseTreeBuilder->BuildLiteral(BoundsTok.TokenValue, BoundsTok.Type);
	NewArrDecl = (CArrayDeclTreeNode*)m_ParseTreeBuilder->BuildArrayDeclaration(
		Var, Bounds);

	//check for, and consume the closing ']' and then the final semi-colon
	Token_t NextToken = m_LexScanner.GetNextToken();
	if(!ExpectNextToken("']'", NextToken, TOKTYPE_CLOSESQRPAREN))
	{
		Var->DestroyNode();
		return NULL;
	}

	NextToken = m_LexScanner.GetNextToken();
	if(!ExpectNextToken("semi-colon", NextToken, TOKTYPE_SEMICOLON))
	{
		Var->DestroyNode();
		return NULL;
	}

	m_SymTable->AddSymbol(Var->VarId->IdName, NewArrDecl,
		m_LexScanner.CurrentLineNumber(), Var->VarType);

	if(m_SymTable->AtGlobalScope())
		CGlobalDataStore::GetSingletonPtr()->AddGlobalVar(NewArrDecl);

	return NewArrDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseExpression(ExprContext Context)
{
	stack<TokenType> ExpOpStack;
	stack<CTreeNode*> ExpValStack;
	int ParenNestingLevel = 0;
	CTreeNode *ExprTerm = NULL;

	ExprTerm = ParseExpressionTerm(ExpOpStack, ParenNestingLevel);
	if(!ExprTerm)
	{
		DestroyExpressionStack(ExpValStack);
		return NULL;
	}
	else
		ExpValStack.push(ExprTerm);

	//get an operator
	Token_t Token = m_LexScanner.PeekNextToken();

	//determine which token to terminate based upon the context we're in
	TokenType EndTokenType = CUtilFuncs::ConvertExprContextToTokenType(Context);

	while(true)
	{
		//if the terminating token if a ')' we dont want to break out
		//early on expressions such as ((6 + 4) * 2 + 1)
		if(Token.Type == TOKTYPE_CLOSEPAREN
		   && EndTokenType == TOKTYPE_CLOSEPAREN)
		{
			if(ParenNestingLevel == 0)
				break;
		}
		else if(Token.Type == EndTokenType)
			break;

		if(!IsArithmeticOperator(Token.Type)
		   && !IsBooleanOperator(Token.Type))
		{
			Error("unexpected '" + Token.TokenValue + "'");
			DestroyExpressionStack(ExpValStack);
			return NULL;
		}

		//if we've hit a closing parenthesis, evaluate the expression
		//contained within it
		if(Token.Type == TOKTYPE_CLOSEPAREN)
		{
			bool Mismatched = false;

			//handle cases like a = 5 * 3);
			if(!ParenNestingLevel)
				Mismatched = true;
			else
			{
				while(ExpOpStack.top() != TOKTYPE_OPENPAREN)
				{
					if(!ReduceExpression(ExpOpStack, ExpValStack))
						return NULL;

					if(ExpOpStack.empty())
						Mismatched = true;
				}
			}

			if(Mismatched)
			{
				Error("mismatched ')'");
				DestroyExpressionStack(ExpValStack);
				return NULL;
			}

			//pop the opening parenthesis
			ExpOpStack.pop();
			--ParenNestingLevel;
			Token = m_LexScanner.GetNextToken();
			continue;
		}

		//if the operator stack is empty, shift it, if it's not empty
		//check the precedence level of the parsed operator and the one on top
		//of the stack
		if(ExpOpStack.empty()
			 || Token.Type == TOKTYPE_OPENPAREN
			 || (OperatorPrecedence(ExpOpStack.top()) < OperatorPrecedence(Token.Type)))
		{
			if(Token.Type == TOKTYPE_OPENPAREN)
				++ParenNestingLevel;

			ExpOpStack.push(Token.Type);
		}
		else
		{
			//keep reducing the expression until we find an operator of
			//lesser precedence that the current operator or we've
			//exhausted the stack
			while(OperatorPrecedence(ExpOpStack.top()) >= OperatorPrecedence(Token.Type))
			{
				if(!ReduceExpression(ExpOpStack, ExpValStack))
					return NULL;

				if(ExpOpStack.empty())
					break;
			}

			//shift the operator now
			ExpOpStack.push(Token.Type);
		}

		m_LexScanner.ClearPeekedTokens();

		//parse the RHS of the expression
		ExprTerm = ParseExpressionTerm(ExpOpStack, ParenNestingLevel);
		if(!ExprTerm)
		{
			DestroyExpressionStack(ExpValStack);
			return NULL;
		}
		else
			ExpValStack.push(ExprTerm);

		Token = m_LexScanner.PeekNextToken();
	}

	//keep reducing the parsed expression until we reach the end
	while(ExpValStack.size() != 1)
	{
		if(!ReduceExpression(ExpOpStack, ExpValStack))
			return NULL;
	}

	//detect cases like "(5 * 3"
	if(ParenNestingLevel)
	{
		Error("mismatched '('");
		DestroyExpressionStack(ExpValStack);
		return NULL;
	}

	return ExpValStack.top();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ReduceExpression(stack<TokenType> &OpStack,
															 stack<CTreeNode*> &ValStack)
{
	//sanity check: make sure we have 2 operands and an operator at least
	SanityCheck(ValStack.size() >= 2);
	SanityCheck(OpStack.size() >= 1);

	//pop the RHS and LHS of the expression
	CTreeNode *Rhs = ValStack.top();
	ValStack.pop();

	CTreeNode *Lhs = ValStack.top();
	ValStack.pop();

	//pop the operator
	TokenType Op = OpStack.top();
	OpStack.pop();

	//we should never be seeing an opening parenthesis
	if(Op == TOKTYPE_OPENPAREN)
	{
		Error("mismatched opening parenthesis");
		DestroyExpressionStack(ValStack);
		return false;
	}

	//push the new node represting this expression back onto the stack
	if(IsArithmeticOperator(Op))
		ValStack.push(m_ParseTreeBuilder->BuildArithExpression(Lhs, Rhs, Op));
	else
		ValStack.push(m_ParseTreeBuilder->BuildBooleanExpression(Lhs, Rhs, Op));
	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
int CParser::OperatorPrecedence(TokenType Op)
{
	map<TokenType, int>::iterator itr = m_OpPrecs.find(Op);

	if(itr == m_OpPrecs.end())
		InternalError("unknown operator precedence");

	return itr->second;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::DestroyExpressionStack(stack<CTreeNode*> &ValStack)
{
	//clean up values
	while(!ValStack.empty())
	{
	  ValStack.top()->DestroyNode();
	  ValStack.pop();
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseExpressionTerm(stack<TokenType> &OpStack, int &ParenLevel)
{
	Token_t Token = m_LexScanner.GetNextToken();

	if(Token.Type == TOKTYPE_ID)
	{
		Symbol_t *IdSym = m_SymTable->LookupIdentifier(Token.TokenValue, false);

		//check that a declaration for this identifier exists
		if(!IdSym)
		{
			Error("undeclared identifier '" + Token.TokenValue + "'");
			return NULL;
		}

		//find out what sort of term we have
		switch(IdSym->IdPtr->Code)
		{
		case TC_PARMDECL:
		case TC_VARDECL:
				return m_ParseTreeBuilder->BuildIdentifier(Token.TokenValue);

		case TC_ARRAYDECL:
			{
				if(!ExpectNextToken("'['", m_LexScanner.GetNextToken(), TOKTYPE_OPENSQRPAREN))
					return NULL;

				//get the index we want
				CTreeNode *ArrRef = ParseExpression(EC_ARRAY);

				if(!ArrRef)
					return false;

				if(!ExpectNextToken("']'", m_LexScanner.GetNextToken(), TOKTYPE_CLOSESQRPAREN))
				{
					ArrRef->DestroyNode();
					return NULL;
				}

				return m_ParseTreeBuilder->BuildArrayRef(ArrRef, Token.TokenValue, IdSym->SymType);
			}

		case TC_FUNCTIONDECL:
			return ParseFunctionCall(IdSym->Name, false);

		default:
			InternalError("unknown expression term tree node");
		}
	}
	else if(IsLiteral(Token.Type))
		return m_ParseTreeBuilder->BuildLiteral(Token.TokenValue, Token.Type);
	else if(Token.Type == TOKTYPE_OPENPAREN)
	{
		//evaluate the entire expression in place before we continue
		CTreeNode *Expr = ParseExpression(EC_CONDITIONAL);

		if(Expr)
		{
			if(!ExpectNextToken("')'", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
			{
				Expr->DestroyNode();
				return NULL;
			}
		}

		return Expr;
	}
	else if(Token.Type == TOKTYPE_MINUS
					|| Token.Type == TOKTYPE_NOT)
	{
		CTreeNode *Expr = NULL;

		//see if the next token is '(' in which case we're applying
		//the operator to an entire expression rather than just a term
		Token_t ParenTok = m_LexScanner.PeekNextToken();

		if(ParenTok.Type == TOKTYPE_OPENPAREN)
		{
			m_LexScanner.ClearPeekedTokens();
			Expr = ParseExpression(EC_CONDITIONAL);

			//get the closing parenthesis
			if(!ExpectNextToken("')'", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
			{
				Expr->DestroyNode();
				return NULL;
			}
		}
		else
		{
			//we've actually got a unary expression of the form "op expr" so
			//parse the expression term  here
			Expr = ParseExpressionTerm(OpStack, ParenLevel);
		}

		if(!Expr)
			return NULL;

		return m_ParseTreeBuilder->BuildUnaryExpression(Expr, Token.Type);
	}
	else
		Error("syntax error '" + Token.TokenValue + "'");

	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseStatement(void)
{
	//what type of statement have we got?
	Token_t StmtToken = m_LexScanner.PeekNextToken();

	switch(StmtToken.Type)
	{
	case TOKTYPE_IF:
		return ParseIfStatement();

	case TOKTYPE_STRINGTYPE:
	case TOKTYPE_INTEGERTYPE:
	case TOKTYPE_BOOLEANTYPE:
	case TOKTYPE_FLOATTYPE:
	case TOKTYPE_CONST:
		return ParseVariableDeclaration();

	case TOKTYPE_WHILE:
		return ParseWhileStatement();

	case TOKTYPE_ID:
		return ParseFuncCallOrAssignmentExpr();

	case TOKTYPE_FUNCTION:
		return ParseFunctionDecl();

	case TOKTYPE_RETURN:
		{
			if(!m_ParsingFuncDecl)
			{
				Error("'return' found outside of function declaration");
				SkipToToken(TOKTYPE_SEMICOLON);
				return NULL;
			}

			return ParseReturnStatement();
		}
	default:
		{
			Error("unexpectd '" + StmtToken.TokenValue + "'");
			m_LexScanner.ClearPeekedTokens();
		}
	}

	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseIfStatement(void)
{
	//ParseStatement will have checked that the current token is 'if'
	SanityCheck(m_LexScanner.GetNextToken().Type == TOKTYPE_IF);
	Token_t IfToken = m_LexScanner.GetNextToken();

	//can't declare functions inside if statements
	m_FuncDeclAllowed = false;

	//parse the head of the statement
	if(!ExpectNextToken("(", IfToken, TOKTYPE_OPENPAREN))
	{
		SkipToToken(TOKTYPE_ENDIF);
		return NULL;
	}
  
	CBooleanExprTreeNode *CondExpr;
	CondExpr = (CBooleanExprTreeNode*)ParseExpression(EC_CONDITIONAL);

	if(!CondExpr)
	{
		SkipToToken(TOKTYPE_ENDIF);
		return NULL;
	}

	//conditional expressions must evalaute to a Boolean value
	if(!TypeCheckExpression(CondExpr, TOKTYPE_BOOLEANTYPE))
	{
		SkipToToken(TOKTYPE_ENDIF);
		return NULL;
	}

	//not all Boolean operators take the same operands, check for invalid
	//combinations not before we start parsing the if stmt body
	if(CondExpr->Code == TC_BOOLEAN_EXPR)
	{
		if(!ValidateBooleanExprOperands(CondExpr))
		{
			SkipToToken(TOKTYPE_ENDIF);
			return NULL;
		}
	}

	//emit a warning for cases the conditional expression is a literal value
	//such as if(56) or if(true) since this is always true or false
	if(CondExpr->Code == TC_LITERAL)
		Warning("conditional expression is of constant value");

	CTreeNode *IfStmt = m_ParseTreeBuilder->BuildIfStatement(CondExpr);

	if(!ExpectNextToken(")", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
	{
		SkipToToken(TOKTYPE_ENDIF);
		return NULL;
	}

	//handle cases where there's no body to the if statement
	Token_t EndIfTok = m_LexScanner.GetNextToken();

	if(EndIfTok.Type != TOKTYPE_ENDIF)
	{
		m_SymTable->PushNewSymbolScope();
		m_LexScanner.PushbackToken(EndIfTok);

		//parse the body, to begin with we're adding statements to
		//the 'if' part until we see an 'else' clause, at which point we
		//start adding statements to the 'else' sub-tree instead
		bool AddingToElse = false;

		while(true)
		{
			CTreeNode *Stmt = ParseStatement();

			if(Stmt)
			{
				if(AddingToElse)
					IfStmt->GetChild(IF_STMT_ELSE_BODY)->AddChild(Stmt);
				else
					IfStmt->GetChild(IF_STMT_BODY)->AddChild(Stmt);
			}

			//see if we've reached the end of the statement
			Token_t EndTok = m_LexScanner.GetNextToken();
			if(EndTok.Type == TOKTYPE_ENDIF)
				break;

			//check for an else clause, if we find one then start
			//adding statements to the else portion of the body
			if(EndTok.Type == TOKTYPE_ELSE)
			{
				AddingToElse = true;

				//stop adding symbols to the 'then' part and start adding them to a new
				//scope within the else block
				m_SymTable->PopSymbolScope();
				m_SymTable->PushNewSymbolScope();
				continue;
			}

			if(!m_LexScanner.HasMoreTokens())
			{
				Error("unexpected end of file found before 'end_if' was found");

				if(Stmt)
					Stmt->DestroyNode();

				IfStmt->DestroyNode();
				m_SymTable->PopSymbolScope();
				return NULL;
			}

			//the token we've just peeked at is the beginning of another
			//statement so push it back to enable parsing to continue
			m_LexScanner.PushbackToken(EndTok);
		}

		m_SymTable->PopSymbolScope();
	}
	else
	{
		//we've got:
		// if(<cond>)
		// end_if
		//
		// no point in keeping this around since we can safely delete it
		Warning("empty 'if' statement found");

		//destroy the if stmt node so it doesn't get added to the IR
		IfStmt->DestroyNode();
		m_SymTable->PopSymbolScope();
		return NULL;
	}

	return IfStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseAssignmentStatement(const string &IdentifierName)
{
	//lookup the identifier to make sure a declaration is available for it
	Symbol_t *IdSym = m_SymTable->LookupIdentifier(IdentifierName, false);

	if(!IdSym)
	{
		Error("undeclared identifier '" + IdentifierName + "'");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//term appearing on the LHS and RHS of the assignment expr
	CTreeNode *AssignExprLhs = NULL;
	CTreeNode *AssignExprRhs = NULL;
	bool ErrOccurred = false;

	//if we've got an array identifier parse the index
	if(IdSym->IdPtr->Code == TC_ARRAYDECL)
	{
		if(!ExpectNextToken("'['", m_LexScanner.GetNextToken(), TOKTYPE_OPENSQRPAREN))
			ErrOccurred = true;
		else
		{
			CTreeNode *ArrIdx = ParseExpression(EC_ARRAY);

			if(!ExpectNextToken("']'", m_LexScanner.GetNextToken(), TOKTYPE_CLOSESQRPAREN))
			{
				SkipToToken(TOKTYPE_SEMICOLON);
				ArrIdx->DestroyNode();
				return NULL;
			}

			if(!ArrIdx)
			{
				SkipToToken(TOKTYPE_SEMICOLON);
				return NULL;
			}

			AssignExprLhs = m_ParseTreeBuilder->BuildArrayRef(ArrIdx, IdSym->Name, 
				IdSym->SymType);
		}
	}
	else if(IdSym->IdPtr->Code == TC_FUNCTIONDECL)
	{
		Error("illegal assignment of value to function '" + IdSym->Name + "'");
		ErrOccurred = true;
	}
	else if(IdSym->IdPtr->Code == TC_VARDECL)
	{
		//we only have a plain variable, check to see if it was declared 'const'
		//in which case we can't re-assign a value to it
		if(((CVarDeclTreeNode*)IdSym->IdPtr)->IsConst)
		{
			Error("illegal assignment to const identifier '" + IdSym->Name + "'");
			ErrOccurred = true;
		}

		AssignExprLhs = m_ParseTreeBuilder->BuildIdentifier(IdSym->Name);
	}
	else if(IdSym->IdPtr->Code == TC_PARMDECL)
	{
		if(((CParameterDeclTreeNode*)IdSym->IdPtr)->IsConst)
		{
			Error("illegal assignment to const parameter '" + IdSym->Name + "'");
			ErrOccurred = true;
		}
		else
		{
			//create a copy of the node for the lhs
			AssignExprLhs = IdSym->IdPtr->CreateCopy();
		}
	}

	//if an error occurred, skip to the end of the statement
	if(ErrOccurred)
	{
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//consume the '=' operator
	if(!ExpectNextToken("'='", m_LexScanner.GetNextToken(), TOKTYPE_ASSIGNMENT))
	{
		AssignExprLhs->DestroyNode();
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//get the RHS
	AssignExprRhs = ParseExpression(EC_INITIALISER);

	if(!AssignExprRhs)
	{
		AssignExprLhs->DestroyNode();
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//consume the terminating semi-colon
	if(!ExpectNextToken("';'", m_LexScanner.GetNextToken(), TOKTYPE_SEMICOLON))
	{
		SkipToToken(TOKTYPE_SEMICOLON);
		AssignExprLhs->DestroyNode();
		AssignExprRhs->DestroyNode();

		return NULL;
	}

	//make sure the type of value we're trying to assign matches
	//the variable type we're assigning to
	if(!TypeCheckExpression(AssignExprRhs, ExtractTypeFromGenericExpr(AssignExprLhs)))
	{
		AssignExprLhs->DestroyNode();
		AssignExprRhs->DestroyNode();
		return NULL;
	}

	//see if
	/*if(IdSym->IdPtr->Code == TC_VARDECL
		 && ((CVarDeclTreeNode*)IdSym->IdPtr)->VarType == TOKTYPE_STRINGTYPE)
	{*/

	return m_ParseTreeBuilder->BuildAssignmentExpr(AssignExprLhs, AssignExprRhs);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseWhileStatement(void)
{
	//ParseStatement will have checked that the current token is 'while'
	SanityCheck(m_LexScanner.GetNextToken().Type == TOKTYPE_WHILE);
	Token_t IfToken = m_LexScanner.GetNextToken();

	//can't declare functions inside while statements
	m_FuncDeclAllowed = false;

	//parse the head of the statement
	if(!ExpectNextToken("(", IfToken, TOKTYPE_OPENPAREN))
	{
		SkipToToken(TOKTYPE_END_WHILE);
		return NULL;
	}

	//parse the expression forming the conditional part of the statement
	CTreeNode *CondExpr = ParseExpression(EC_CONDITIONAL);

	//conditional expressions must evalaute to a Boolean value
	if(!TypeCheckExpression(CondExpr, TOKTYPE_BOOLEANTYPE))
	{
		SkipToToken(TOKTYPE_ENDIF);
		return NULL;
	}

	//emit a warning for cases the conditional expression is a literal value
	//such as while(56) or while(true) since this is always true or false
	if(CondExpr->Code == TC_LITERAL)
		Warning("conditional expression is of constant value");

	if(!CondExpr)
	{
		SkipToToken(TOKTYPE_END_WHILE);
		return NULL;
	}

	//build the while stmt tree node
	CTreeNode *WhileStmt = m_ParseTreeBuilder->BuildWhileStatement(CondExpr);

	if(!ExpectNextToken(")", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
	{
		SkipToToken(TOKTYPE_END_WHILE);
		return NULL;
	}

	//handle cases where there's no body to the statement
	Token_t EndWhileTok = m_LexScanner.GetNextToken();

	if(EndWhileTok.Type != TOKTYPE_END_WHILE)
	{
		m_SymTable->PushNewSymbolScope();
		m_LexScanner.PushbackToken(EndWhileTok);

		//parse the body
		while(true)
		{
			CTreeNode *Stmt = ParseStatement();

			if(Stmt)
				WhileStmt->AddChild(Stmt);

			Token_t EndTok = m_LexScanner.GetNextToken();
			if(EndTok.Type == TOKTYPE_END_WHILE)
			{

				m_LexScanner.ClearPeekedTokens();
				break;
			}

			if(!m_LexScanner.HasMoreTokens())
			{
				Error("unexpected end of file found before 'end_while' was found");
				WhileStmt->DestroyNode();
				Stmt->DestroyNode();
				m_SymTable->PopSymbolScope();
				return NULL;
			}

			m_LexScanner.PushbackToken(EndTok);
		}

		m_SymTable->PopSymbolScope();
	}
	else
	{
		//we've got:
		// while(<cond>)
		// end_while
		//
		// no point in keeping this around since we can safely delete it
		Warning("empty 'while' statement found");

		//destroy the while stmt node so it doesn't get added to the IR
		WhileStmt->DestroyNode();
		return NULL;
	}

	return WhileStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseFunctionDecl(void)
{
	SanityCheck(m_LexScanner.GetNextToken().Type == TOKTYPE_FUNCTION);

	//see if a function declaration is allowed at this point, if it's not
	//then IsFuncDeclAllowed will handle issuing any error messages and skips
	//any tokens until the "end_function" is found
	if(!IsFuncDeclAllowed())
		return NULL;

	m_ParsingFuncDecl = true;
	Token_t FuncToken = m_LexScanner.GetNextToken();

	if(!ExpectNextToken("identifier", FuncToken, TOKTYPE_ID))
	{
		//skip the entire function
		SkipToToken(TOKTYPE_END_FUNCTION);
		m_ParsingFuncDecl = false;
		return NULL;
	}

	//check to see if the name given to the function conflicts with
	//anything previously declared
	Symbol_t *FuncSym = m_SymTable->LookupIdentifier(FuncToken.TokenValue, false);

	if(FuncSym)
	{
		AmbiguousDeclarationError(FuncSym);
		SkipToToken(TOKTYPE_END_FUNCTION);
		return NULL;
	}

	//info about the function we're parsing
	string FunctionName = FuncToken.TokenValue;
	CTreeNode *ParmList = m_ParseTreeBuilder->BuildParameterList();
	int NumParms = 0;;
	TokenType RetType;

	//parse the parameter list
	if(!ExpectNextToken("'('", m_LexScanner.GetNextToken(), TOKTYPE_OPENPAREN))
	{
		SkipToToken(TOKTYPE_END_FUNCTION);
		m_ParsingFuncDecl = false;
		return NULL;
	}

	FuncToken = m_LexScanner.PeekNextToken();

	//see if we have any declared parameters
	if(FuncToken.Type != TOKTYPE_CLOSEPAREN)
	{
		if(!ParseFuncDeclParameterList(ParmList, NumParms))
		{
			SkipToToken(TOKTYPE_END_FUNCTION);
			m_ParsingFuncDecl = false;
			return NULL;
		}
	}
	else
		m_LexScanner.ClearPeekedTokens();

	//check the type this function returns
	if(!ValidateFuncReturnsInfo(RetType))
	{
		SkipToToken(TOKTYPE_END_FUNCTION);
		ParmList->DestroyNode();
		m_ParsingFuncDecl = false;
		return NULL;
	}

	//build the function decl
	CTreeNode *FnDeclNode = m_ParseTreeBuilder->BuildFunctionDecl(FunctionName,
																	NumParms, ParmList, RetType);
	m_CurrFuncDecl = FnDeclNode;

	//add the function name to the global namespace and push a new scope
	m_SymTable->AddSymbol(FunctionName, FnDeclNode,
		m_LexScanner.CurrentLineNumber(), RetType);
	m_SymTable->PushNewSymbolScope();

	//add the parameters to the new function's scope, checking for any ambiguities
	//along the way
	if(!AddFunctionParmsToSymbolTable(ParmList))
	{
		FnDeclNode->DestroyNode();
		return NULL;
	}

	//handle cases where the function is empty
	Token_t EndTok = m_LexScanner.GetNextToken();
	bool FoundGlobalRetStmt = false;
	m_FoundNestedRetStmt = false;

	if(EndTok.Type != TOKTYPE_END_FUNCTION)
	{
		m_LexScanner.PushbackToken(EndTok);

		//parse the function body
		while(true)
		{
			CTreeNode *Stmt = ParseStatement();

			if(Stmt)
				FnDeclNode->AddChild(Stmt);

			if(Stmt && Stmt->Code == TC_RETURNSTMT)
			{
				//check for a function which isn't supposed to have a
				//return value but does
				if(((CFunctionDeclTreeNode*)FnDeclNode)->RetType == TOKTYPE_VOID)
				{
					Error("function '" + FunctionName + "' declared void but returning a value");

					FnDeclNode->DestroyNode();
					Stmt->DestroyNode();

					SkipToToken(TOKTYPE_END_FUNCTION);
					m_ParsingFuncDecl = false;
					m_SymTable->PopSymbolScope();
					return NULL;
				}

				FoundGlobalRetStmt = true;
			}

			Token_t EndTok = m_LexScanner.GetNextToken();
			if(EndTok.Type == TOKTYPE_END_FUNCTION)
			{
				m_LexScanner.ClearPeekedTokens();
				break;
			}

			if(!m_LexScanner.HasMoreTokens())
			{
				Error("unexpected end of file found before 'end_function' was found");
				FnDeclNode->DestroyNode();

				if(Stmt)
					Stmt->DestroyNode();

				m_SymTable->PopSymbolScope();
				m_CurrFuncDecl = NULL;
				return NULL;
			}

			m_LexScanner.PushbackToken(EndTok);
		}

		//see if we're supposed to have a return statement but dont
		if(!FoundGlobalRetStmt && !m_FoundNestedRetStmt
			 && ((CFunctionDeclTreeNode*)FnDeclNode)->RetType != TOKTYPE_VOID)
			Error("function '" + FunctionName + "' missing 'return' statement");
	}
	else
		Error("function '" + FunctionName + "' with no body");

	m_SymTable->PopSymbolScope();
	m_ParsingFuncDecl = false;
	return FnDeclNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseReturnStatement(void)
{
	SanityCheck(m_LexScanner.GetNextToken().Type == TOKTYPE_RETURN);

	//return statements are only allowed when we're parsing a function decl
	if(!m_ParsingFuncDecl)
	{
		Error("'return' found outside of a function declaration");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//parse the expression forming the return value
	CTreeNode *RetExpr = ParseExpression(EC_INITIALISER);
	
	//set this to true regardless of whether the return expr is invalid as otherwise
	//we get bogus error messages about a missing return stmt
	m_FoundNestedRetStmt = true;

	if(RetExpr)
	{
		//check for the terminating semi-colon
		if(!ExpectNextToken("';'", m_LexScanner.GetNextToken(), TOKTYPE_SEMICOLON))
		{
			RetExpr->DestroyNode();
			return NULL;
		}

		if(!TypeCheckExpression(RetExpr, ExtractTypeFromGenericExpr(m_CurrFuncDecl)))
		{
			//replace the erroneous return expression with a constant to suppress
			//bogus errors later on about the function not returning a value when in
			//reality it does
			RetExpr->DestroyNode();
			RetExpr = m_ParseTreeBuilder->BuildLiteral("0", TOKTYPE_INTEGERTYPE);
		}

		return m_ParseTreeBuilder->BuildReturnStmt(RetExpr);
	}
	else
		SkipToToken(TOKTYPE_SEMICOLON);

	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ParseFuncDeclParameterList(CTreeNode *ParmList, int &NumParms)
{
	SanityCheck(ParmList);

	//tokens to store the type and name of the parameters
	Token_t ParmTypeToken;
	Token_t ParmNameToken;
	Token_t ParmToken;
	bool IsConstParm = false;
	int ParmNum = 0;

	NumParms = 0;

	while(true)
	{
		//next two tokens should be an (optional) type specifier and an identifier
		ParmTypeToken = m_LexScanner.GetNextToken();

		if(ParmTypeToken.Type == TOKTYPE_CONST)
		{
			IsConstParm = true;
			ParmTypeToken = m_LexScanner.GetNextToken();
		}
		else
			IsConstParm = false;

		//now we must have a type token
		if(!IsType(ParmTypeToken.Type))
		{
			Error("expected type specifier before '" + ParmTypeToken.TokenValue + "'");
			return false;
		}

		//followed by the name for it
		ParmNameToken = m_LexScanner.GetNextToken();
		if(!ExpectNextToken("identifier", ParmNameToken, TOKTYPE_ID))
			return false;

		//build the new parm decl and add it to the list
		CTreeNode *ParmDecl = m_ParseTreeBuilder->BuildParameterDecl(
														ParmNameToken.TokenValue, ParmTypeToken.Type,
														IsConstParm, ParmNum++);

		ParmList->AddChild(ParmDecl);
		ParmToken = m_LexScanner.GetNextToken();
		++NumParms;

		//if the next token is ')' then we're all done with
		//parsing the parameter list
		if(ParmToken.Type == TOKTYPE_CLOSEPAREN)
			break;
		else
		{
			if(!ExpectNextToken("','", ParmToken, TOKTYPE_COMMA))
				return false;
		}
	}

	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ValidateFuncReturnsInfo(TokenType &RetType)
{
	Token_t RetToken;

	if(!ExpectNextToken("'returns'", m_LexScanner.GetNextToken(), TOKTYPE_RETURNS))
		return false;

	RetToken = m_LexScanner.GetNextToken();

	if(!IsType(RetToken.Type) && RetToken.Type != TOKTYPE_VOID)
	{
		Error("expected type qualifier or 'void' before '" + RetToken.TokenValue + "'");
		return false;
	}

	RetType = RetToken.Type;
	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode *CParser::ParseFunctionCall(const string &FuncName,
                                      bool RequireSemiColon)
{
	//lookup the identifier to make sure it exists
	Symbol_t *IdSym = m_SymTable->LookupIdentifier(FuncName, false);

	if(!IdSym)
	{
		Error("undeclared identifier '" + FuncName + "'");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//make sure the identifier is actually a function
	if(IdSym->IdPtr->Code != TC_FUNCTIONDECL)
	{
		Error("identifier '" + IdSym->Name + "' is not a function");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//parse the parameters this function is being called with
	if(!ExpectNextToken("'(", m_LexScanner.GetNextToken(), TOKTYPE_OPENPAREN))
	{
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//build our call_expr node
	CFunctionDeclTreeNode *FnDecl = (CFunctionDeclTreeNode*)IdSym->IdPtr;
	CTreeNode *CallExprNode = m_ParseTreeBuilder->BuildCallExpr(FuncName);

	//if we've got a closing parenthesis next, check to see if the function
	//we're calling was declared with parameters or not
	if(m_LexScanner.PeekNextToken().Type == TOKTYPE_CLOSEPAREN)
	{
		if(FnDecl->NumParameters != 0)
		{
			Error("missing parameters for call to function '" + FuncName + "'");
			SkipToToken(TOKTYPE_SEMICOLON);
			CallExprNode->DestroyNode();
			return NULL;
		}
	}
	else
	{
		//was the function declared with no parameters?
		if(FnDecl->NumParameters == 0)
		{
			Error("invalid number of parameters for call to function '" + FuncName + "'");
			SkipToToken(TOKTYPE_SEMICOLON);
			CallExprNode->DestroyNode();
			return NULL;
		}

		//an individual parameter in the call expr
		CTreeNode *Parm = NULL;

		//the type of token we want to terminate each parameter expression on
		//depends on how many parameters we're looking for, if it's > 1 then
		//the terminating token is ',' else it's ')'
		if(FnDecl->NumParameters == 1)
		{
			Parm = ParseExpression(EC_CONDITIONAL);

			if(!Parm)
			{
				SkipToToken(TOKTYPE_SEMICOLON);
				CallExprNode->DestroyNode();
				return NULL;
			}

			//TypeCheckParameter will handle the cleanup of the nodes if it fails
			if(!TypeCheckParameter(Parm, FnDecl, CallExprNode, 0))
				return NULL;

			CallExprNode->AddChild(Parm);
		}
		else
		{
			int ParmId = 0;

			for(; ParmId < FnDecl->NumParameters - 1; ParmId++)
			{
				//parse the expression forming the parameter value
				Parm = ParseExpression(EC_FUNCCALLPARMS);

				if(Parm)
				{
					//multiple parameters are separated by a comma
					if(!ExpectNextToken("','", m_LexScanner.GetNextToken(), TOKTYPE_COMMA))
					{
						SkipToToken(TOKTYPE_SEMICOLON);
						CallExprNode->DestroyNode();
						return NULL;
					}

					//TypeCheckParameter will handle the cleanup of the nodes if it fails
					if(!TypeCheckParameter(Parm, FnDecl, CallExprNode, ParmId))
						return NULL;

					CallExprNode->AddChild(Parm);
				}
				else
				{
					Error("too few parameters for call to function '" + FnDecl->Name + "'");
					CallExprNode->DestroyNode();
					SkipToToken(TOKTYPE_SEMICOLON);
					return NULL;
				}
			}

			//the final parameter expression is terminated by the closing parenthesis
			//rather than a comma like the first lot
			Parm = ParseExpression(EC_CONDITIONAL);

			if(!Parm)
			{
				SkipToToken(TOKTYPE_SEMICOLON);
				CallExprNode->DestroyNode();
				return NULL;
			}

			//TypeCheckParameter will handle the cleanup of the nodes if it fails
			if(!TypeCheckParameter(Parm, FnDecl, CallExprNode, ParmId))
				return NULL;

			CallExprNode->AddChild(Parm);
		}
	}

	//consume the closing parenthesis and semi-colon
	if(!ExpectNextToken("')'", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
	{
		SkipToToken(TOKTYPE_SEMICOLON);
		CallExprNode->DestroyNode();
		return NULL;
	}

	if(RequireSemiColon)
	{
		//if we're missing a semi-colon, try and continue as normal rather than
		//skipping chunks of the code
		Token_t SemiColTok = m_LexScanner.GetNextToken();

		if(!ExpectNextToken("';'", SemiColTok, TOKTYPE_SEMICOLON))
		{
			CallExprNode->DestroyNode();
			m_LexScanner.PushbackToken(SemiColTok);
			return NULL;
		}
	}

	//if it's a call to a builtin function, mark it used
	if(CBuiltinFuncMngr::GetSingletonPtr()->IsBuiltinFunction(FuncName))
		CGlobalDataStore::GetSingletonPtr()->MarkBuiltinFunctionUsed(FuncName);

	return CallExprNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CParser::ParseFuncCallOrAssignmentExpr(void)
{
	Token_t Identifier = m_LexScanner.GetNextToken();
	SanityCheck(Identifier.Type == TOKTYPE_ID);

	//see if we have a function call or an assignment expr
	Token_t PeekedTok = m_LexScanner.PeekNextToken();

	if(PeekedTok.Type == TOKTYPE_OPENPAREN)
		return ParseFunctionCall(Identifier.TokenValue, true);
	
	return ParseAssignmentStatement(Identifier.TokenValue);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::TypeCheckExpression(CTreeNode *Expr, TokenType RequiredType)
{
	SanityCheck(Expr);
	TokenType FromType = (TokenType)-1;

	//we need to handle literal and binary expressions separately because of
	//type conversion issues
	if(Expr->Code == TC_LITERAL)
	{
		CLiteralTreeNode *Literal = (CLiteralTreeNode*)Expr;

		//see if the types match, if not see if the type of the expression
		//can be converted to that of the requested type
		if(TypesCompatible(RequiredType, Literal->LitType)
			 || TypeConvertsTo(RequiredType, Literal->LitType))
		 return true;
		else
			FromType = Literal->LitType;
	}
	else if(Expr->Code == TC_ARRAY_REF)
	{
		//get the type of the array
		CArrayRefTreeNode *Ref = (CArrayRefTreeNode*)Expr;
		Symbol_t *Sym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(
			Ref->ArrName->IdName, false);

		SanityCheck(Sym);
		CArrayDeclTreeNode *ArrDecl = (CArrayDeclTreeNode*)Sym->IdPtr;
		CVarDeclTreeNode *Var = (CVarDeclTreeNode*)ArrDecl->GetChild(ARR_DECL_VAR);

		//check to see if it matches
		if(TypesCompatible(RequiredType, Var->VarType)
			 || TypeConvertsTo(RequiredType, Var->VarType))
		 return true;
		else
			FromType = Var->VarType;
	}
	else if(Expr->Code == TC_ARITH_EXPR)
	{
		//check binary expressions
		CArithExprTreeNode *BinExpr = (CArithExprTreeNode*)Expr;

		bool LhsTypeOk = false;
		bool RhsTypeOk = false;

		//if we've got a binary expression on either side of the expr,
		//recursively typecheck each side
		if(BinExpr->GetChild(ARITH_EXPR_LHS)->Code == TC_ARITH_EXPR)
			LhsTypeOk = TypeCheckExpression(BinExpr->GetChild(ARITH_EXPR_LHS), RequiredType);

		if(BinExpr->GetChild(ARITH_EXPR_RHS)->Code == TC_ARITH_EXPR)
			RhsTypeOk = TypeCheckExpression(BinExpr->GetChild(ARITH_EXPR_RHS), RequiredType);

		//if we've got here then we must have something like "val op val", check
		//that the types of "val" match the required types for "op"
		TokenType LhsType = ExtractTypeFromGenericExpr(BinExpr->GetChild(ARITH_EXPR_LHS));
		TokenType RhsType = ExtractTypeFromGenericExpr(BinExpr->GetChild(ARITH_EXPR_RHS));

		if(IsBooleanOperator(BinExpr->Operator))
		{
			//only integral and real types are allowed on either side of
			//Boolean operations
			if(!TypeValidForBooleanOperator(LhsType)
				 || !TypeValidForBooleanOperator(RhsType))
			{
				Error("invalid operand to Boolean operator '" +
					CUtilFuncs::TokenTypeToStr(BinExpr->Operator) + "'");
				return false;
			}

			//the result of a Boolean expression is always going to be of a Boolean type
			FromType = TOKTYPE_BOOLEANTYPE;
		}
		else
		{
			//must have an arithmetic operator
			if(!TypeValidForArithmeticOperator(LhsType)
				 || !TypeValidForArithmeticOperator(RhsType))
			{
				Error("invalid operand to arithmetic operator '" +
					CUtilFuncs::TokenTypeToStr(BinExpr->Operator) + "'");
				return false;
			}

			//check for matching literal/value types in the same expression. This is
			//so that if we have TOKTYPE_INTEGERTYPE on the LHS and TOKTYPE_INTLITERAL
			//on the RHS, the expression is perfectly valid from a semantic POV as the
			//values are of the same type
			if(TypesCompatible(RequiredType, LhsType)
				 || TypesCompatible(RequiredType, RhsType))
			 return true;

			//we've got operand types which are applicable for the type of operator being
			//used, now handle cases like "5.4 + 4" which is a mix of real and integral
			//types in the same expression, for this we only issue a warning about the
			//implicit conversion of the integral operand to real type since only real
			//or integral values are allowed this message will always be accurrate
			if(LhsType != RhsType)
			{
				Warning("conversion from 'float' to 'integer' in arithmetic expression");

				//force the result of the expression to a real type rather than integral
				FromType = TOKTYPE_FLOATTYPE;
			}
			else
				FromType = TOKTYPE_INTEGERTYPE;
		}
	}
	else
		FromType = ExtractTypeFromGenericExpr(Expr);

	//perform the type checking
	if(FromType == RequiredType || TypeConvertsTo(RequiredType, FromType))
		return true;

	Error("invalid conversion from '" + CUtilFuncs::TokenTypeToStr(FromType) +
				"' to '" + CUtilFuncs::TokenTypeToStr(RequiredType) + "'");
	return false;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::TypeConvertsTo(TokenType Orig, TokenType To)
{
	//handle then obvious case first
	if(Orig == To)
		return true;

	//conversions between real -> integral are ok but give
	//a warning to let the programmer know
	if(Orig == TOKTYPE_INTEGERTYPE
		 && (To == TOKTYPE_FLOATLITERAL || To == TOKTYPE_FLOATTYPE))
	{
		Warning("conversion from 'float' to 'integer' may cause loss of precision");
		return true;
	}

	//conversions from integral -> floating point are ok since any floating
	//point number can be represented without loss of information
	if((Orig == TOKTYPE_FLOATTYPE || Orig == TOKTYPE_FLOATLITERAL)
		 && (To == TOKTYPE_INTLITERAL || To == TOKTYPE_INTEGERTYPE))
		return true;

	//check instances where we have TYPE = LITERAL
	if(IsType(Orig) && IsLiteral(To))
		return TypesCompatible(Orig, To);

	return false;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::TypesCompatible(TokenType TypeOne, TokenType TypeTwo)
{
	switch(TypeOne)
	{
	case TOKTYPE_INTEGERTYPE:
		return TypeTwo == TOKTYPE_INTLITERAL;

	//it's perfectly safe to allow integer literals in an floating point expr
	case TOKTYPE_FLOATTYPE:
		return TypeTwo == TOKTYPE_FLOATLITERAL
							|| TypeTwo == TOKTYPE_INTLITERAL;

	//integral/floating point constants are implicitly boolean types
	case TOKTYPE_BOOLEANTYPE:
		return TypeTwo == TOKTYPE_TRUE
							|| TypeTwo == TOKTYPE_FALSE
							|| TypeTwo == TOKTYPE_INTLITERAL
							|| TypeTwo == TOKTYPE_INTEGERTYPE
							|| TypeTwo == TOKTYPE_FLOATLITERAL
							|| TypeTwo == TOKTYPE_FLOATTYPE;

	case TOKTYPE_STRINGTYPE:
		return TypeTwo == TOKTYPE_STRINGLITERAL;
	}

	return false;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::TypeValidForBooleanOperator(TokenType Type)
{
	return Type == TOKTYPE_INTEGERTYPE
				 || Type == TOKTYPE_FLOATTYPE
				 || Type == TOKTYPE_BOOLEANTYPE
				 || Type == TOKTYPE_TRUE
				 || Type == TOKTYPE_FALSE
				 || Type == TOKTYPE_INTLITERAL
				 || Type == TOKTYPE_FLOATLITERAL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::TypeValidForArithmeticOperator(TokenType Type)
{
	return Type == TOKTYPE_INTEGERTYPE
				 || Type == TOKTYPE_FLOATTYPE
				 || Type == TOKTYPE_INTLITERAL
				 || Type == TOKTYPE_FLOATLITERAL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromGenericExpr(CTreeNode *Expr)
{
	SanityCheck(Expr);

	//we've got an unknown type of tree forming an expression,
	//find out what type it is and return the underlying type for it
	switch(Expr->Code)
	{
	case TC_IDENTIFIER:
		return ExtractTypeFromIdentifier((CIdentifierTreeNode*)Expr);

	case TC_ARRAY_REF:
		return ExtractTypeFromArrayRef((CArrayRefTreeNode*)Expr);

	case TC_CALLEXPR:
		return ExtractTypeFromCallExpr((CCallExprTreeNode*)Expr);

	case TC_LITERAL:
		return ((CLiteralTreeNode*)Expr)->LitType;

	case TC_ARITH_EXPR:
		return ExtractTypeFromArithExpr((CArithExprTreeNode*)Expr);

	case TC_UNARY_EXPR:
		return ExtractTypeFromGenericExpr(Expr->GetChild(UNARY_EXPR_EXPR));

	case TC_FUNCTIONDECL:
		return ((CFunctionDeclTreeNode*)Expr)->RetType;

	case TC_PARMDECL:
		return ((CParameterDeclTreeNode*)Expr)->Type;

	case TC_BOOLEAN_EXPR:
		return TOKTYPE_BOOLEANTYPE;

	case TC_VARDECL:
		return ((CVarDeclTreeNode*)Expr)->VarType;
	}

	//oops
	InternalError("unknown expression type");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromArrayRef(CArrayRefTreeNode *ArrRef)
{
	//get the array decl
	Symbol_t *ArrSym = m_SymTable->LookupIdentifier(ArrRef->ArrName->IdName, false);
	SanityCheck(ArrSym);

	//return the type
	CVarDeclTreeNode *ArrDecl = (CVarDeclTreeNode*)ArrSym->IdPtr->GetChild(
		ARR_DECL_VAR);
	return ArrDecl->VarType;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromIdentifier(CIdentifierTreeNode *Ident)
{
	Symbol_t *IdSym = m_SymTable->LookupIdentifier(Ident->IdName, false);
	SanityCheck(IdSym);

	//get the underlying type of the decl
	if(IdSym->IdPtr->Code == TC_VARDECL)
		return ((CVarDeclTreeNode*)IdSym->IdPtr)->VarType;

	//we must have a parm decl
	SanityCheck(IdSym->IdPtr->Code == TC_PARMDECL);
	return ((CParameterDeclTreeNode*)IdSym->IdPtr)->Type;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromCallExpr(CCallExprTreeNode *CallExpr)
{
	//the type of a call_expr node is actually the return type
	//of the function being called so lookup the function name to
	//get a pointer to the fn decl
	Symbol_t *CallSym = m_SymTable->LookupIdentifier(CallExpr->FuncName, false);
	SanityCheck(CallSym);

	//extract the function's return type
	CFunctionDeclTreeNode *FnDecl = (CFunctionDeclTreeNode*)CallSym->IdPtr;
	return FnDecl->RetType;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromArithExpr(CArithExprTreeNode *Expr)
{
	//if we've got a binary operator for this expr, the type of the
	//whole (sub)expression is Boolean
	if(IsBooleanOperator(Expr->Operator))
		return TOKTYPE_BOOLEANTYPE;

	//must have an arithmetic expression
	SanityCheck(IsArithmeticOperator(Expr->Operator));

	//since arithmetic operators are left associative, the result of the
	//expression must be of a type compatible with the LHS of the operator
	return ExtractTypeFromGenericExpr(Expr->GetChild(ARITH_EXPR_LHS));
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::TypeCheckParameter(CTreeNode *Parm, CTreeNode *FnDecl,
																 CTreeNode *CallExpr, unsigned int ParmId)
{
	SanityCheck(Parm && FnDecl && CallExpr);
	if(!TypeCheckExpression(Parm, ExtractTypeFromGenericExpr(
													FnDecl->GetChild(FUNC_DECL_PARM_LIST)->GetChild(ParmId))))
	{
		Parm->DestroyNode();
		CallExpr->DestroyNode();
		SkipToToken(TOKTYPE_SEMICOLON);
		return false;
	}

	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::AmbiguousDeclarationError(Symbol_t *Sym)
{
	SanityCheck(Sym);

	//convert the int line number to a string for outputting
	stringstream LineStr;
	LineStr << Sym->LineNo;

	//let the programmer know at which line the existing declaration is on
	Error("ambiguous declaration of identifier '" + Sym->Name + "'");
	Error("'" + Sym->Name + "' previously declared on line: " + LineStr.str());
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ValidateBooleanExprOperands(CTreeNode *Expr)
{
	CBooleanExprTreeNode *BlnExpr = (CBooleanExprTreeNode*)Expr;

	//extract the LHS and RHS expressions
	CBooleanExprTreeNode *ExprLhs;
  ExprLhs = (CBooleanExprTreeNode*)BlnExpr->GetChild(BLN_EXPR_LHS);

	CBooleanExprTreeNode *ExprRhs;
	ExprRhs = (CBooleanExprTreeNode*)BlnExpr->GetChild(BLN_EXPR_RHS);

	//determine what class of boolean operator we have, performing
	//the relevant checks required
	if(IsLogicalOperator(BlnExpr->Operator))
	{
		if(!ValidateLogicalBlnExprOperands(ExprLhs)
			 || !ValidateLogicalBlnExprOperands(ExprRhs))
		{
			Error("invalid operand(s) for logical operator '" +
				CUtilFuncs::TokenTypeToStr(BlnExpr->Operator) + "'");
			return false;
		}
	}
	else
	{
		SanityCheck(IsRelationalOperator(BlnExpr->Operator));
		if(!ValidateRelationalBlnExprOperands(ExprLhs)
			 || !ValidateRelationalBlnExprOperands(ExprRhs))
		{
			Error("invalid operand(s) for relational operator '" +
				CUtilFuncs::TokenTypeToStr(BlnExpr->Operator) + "'");
			return false;
		}
	}

	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ValidateLogicalBlnExprOperands(CTreeNode *Expr)
{
	SanityCheck(Expr);

	//recurse if necessary
	if(Expr->Code == TC_BOOLEAN_EXPR)
		return ValidateBooleanExprOperands(Expr);
	else
	{
		//valid operands are identifiers to vars, arrays, params
		//or calls to functions which boolean values or the 'true' and
		//'false' literal types
		switch(Expr->Code)
		{
		case TC_IDENTIFIER:
			{
				//look it up
				CIdentifierTreeNode *IdNode;
				IdNode = (CIdentifierTreeNode*)Expr;

				Symbol_t *IdSym = m_SymTable->LookupIdentifier(IdNode->IdName, false);
				SanityCheck(IdSym);

				//get the type and check it's ok
				TokenType IdType = ExtractTypeFromGenericExpr(IdSym->IdPtr);
				return IdType == TOKTYPE_BOOLEANTYPE;
			}

		case TC_LITERAL:
			{
				CLiteralTreeNode *LitNode = (CLiteralTreeNode*)Expr;

				return LitNode->LitType == TOKTYPE_TRUE
					     || LitNode->LitType == TOKTYPE_FALSE;
			}

		case TC_UNARY_EXPR:
			{
				if(Expr->GetChild(UNARY_EXPR_EXPR)->Code == TC_BOOLEAN_EXPR)
					return ValidateBooleanExprOperands(Expr->GetChild(UNARY_EXPR_EXPR));

				return TypeCheckExpression(Expr, TOKTYPE_BOOLEANTYPE);
			}
		}
	}

	return false;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ValidateRelationalBlnExprOperands(CTreeNode *Expr)
{
	//recurse if necessary
	if(Expr->Code == TC_BOOLEAN_EXPR)
		return ValidateBooleanExprOperands(Expr);
	else
	{
		//valid operands are identifiers to vars arrays, params
		//or calls to functions which return integral or floating point values
		switch(Expr->Code)
		{
		case TC_IDENTIFIER:
			{
				//look it up
				CIdentifierTreeNode *IdNode;
				IdNode = (CIdentifierTreeNode*)Expr;

				Symbol_t *IdSym = m_SymTable->LookupIdentifier(IdNode->IdName, false);
				SanityCheck(IdSym);

				//get the type and check it's ok
				TokenType IdType = ExtractTypeFromGenericExpr(IdSym->IdPtr);
				return IdType == TOKTYPE_INTEGERTYPE
							 || IdType == TOKTYPE_INTLITERAL
							 || IdType == TOKTYPE_FLOATTYPE
							 || IdType == TOKTYPE_FLOATLITERAL;
			}

		case TC_LITERAL:
			{
				CLiteralTreeNode *LitNode = (CLiteralTreeNode*)Expr;

				return LitNode->LitType == TOKTYPE_INTEGERTYPE
					     || LitNode->LitType == TOKTYPE_INTLITERAL
							 || LitNode->LitType == TOKTYPE_FLOATTYPE
							 || LitNode->LitType == TOKTYPE_FLOATLITERAL;
			}

		case TC_ARITH_EXPR:
			return true;

		case TC_ARRAY_REF:
			{
				//look up the array
				Symbol_t *ArrSym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(
					((CArrayRefTreeNode*)Expr)->ArrName->IdName, false);
				SanityCheck(ArrSym);

				//get the underlying var decl
				CArrayDeclTreeNode *ArrDecl = (CArrayDeclTreeNode*)ArrSym->IdPtr;
				CVarDeclTreeNode *Var = (CVarDeclTreeNode*)ArrDecl->GetChild(ARR_DECL_VAR);

				return Var->VarType == TOKTYPE_INTEGERTYPE
					     || Var->VarType == TOKTYPE_INTLITERAL
							 || Var->VarType == TOKTYPE_FLOATTYPE
							 || Var->VarType == TOKTYPE_FLOATLITERAL;
			}
		}
	}

	return false;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::AddFunctionParmsToSymbolTable(CTreeNode *ParmList)
{
	for(unsigned int i = 0; i < ParmList->GetNumChildren(); i++)
	{
		CParameterDeclTreeNode *ParmDecl;
		CIdentifierTreeNode *ParmId;

		ParmDecl = (CParameterDeclTreeNode*)ParmList->GetChild(i);
		ParmId = (CIdentifierTreeNode*)ParmDecl->GetChild(PARM_DECL_ID);

		//see if this parameter is ambiguous
		Symbol_t *ParmSym = m_SymTable->LookupIdentifier(ParmId->IdName, true);

		if(ParmSym)
		{
			AmbiguousDeclarationError(ParmSym);
			SkipToToken(TOKTYPE_END_FUNCTION);
			m_ParsingFuncDecl = false;
			m_SymTable->PopSymbolScope();
			return false;
		}
		else
			m_SymTable->AddSymbol(ParmId->IdName, ParmDecl,
				m_LexScanner.CurrentLineNumber(), ParmDecl->Type);
	}

	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::IsFuncDeclAllowed(void)
{
	//are we already parsing a function?
	if(m_ParsingFuncDecl)
	{
		Error("nested functions not allowed");
		SkipToToken(TOKTYPE_END_FUNCTION);
		return false;
	}

	//check to see if a function decl is allowed at this point
	if(!m_FuncDeclAllowed)
	{
		Error("function declarations are only permitted at global scope");
		SkipToToken(TOKTYPE_END_FUNCTION);
		return false;
	}

	return true;
}
//-------------------------------------------------------------