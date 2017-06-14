//------------------------------------------------------------------------------------------
// File: CParser.cpp
// Desc: Class to peform syntactic and semantic checks on the input source program
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "TokenTypes.h"
#include "CParser.h"

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
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParser::~CParser(void)
{
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ParseSourceFile(const string &Identifier)
{
	m_LexScanner.SetInputFile(Identifier);

	//construct the root of the parse tree node
	CParseTreeNode *RootNode = m_ParseTree.BuildRootNode();

	while(m_LexScanner.HasMoreTokens())
	{
		//function decls can only appear at global scope
		m_FuncDeclAllowed = true;

		CParseTreeNode *Stmt = ParseStatement();

		//we're only allowed function and var decls at global scope
		if(Stmt)
		{
			if(Stmt->Code == TC_FUNCTIONDECL 
				 || Stmt->Code == TC_VARDECL
				 || Stmt->Code == TC_ARRAYDECL)
			 RootNode->Children.push_back(Stmt);
			else
			{
				Error("only variable and function declarations are allowed at global scope");
				Stmt->DestroyNode();
				continue;
			}
		}
	}
	
	//dump the parse tree
	TiXmlDocument *XmlDoc = new TiXmlDocument();
	XmlDoc->InsertEndChild(TiXmlElement("parse_tree"));
	RootNode->DumpNode(XmlDoc->RootElement());
	XmlDoc->SaveFile("testing.xml");

	//cleanup after ourselves
	RootNode->DestroyNode();
	delete XmlDoc;

	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::Error(const string &Msg)
{
	cout << "error: " << m_LexScanner.CurrentLineNumber() << " : " << Msg << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::Warning(const string &Msg)
{
	cout << "warning: " << m_LexScanner.CurrentLineNumber() << " : " << Msg << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ExpectNextToken(const string &Msg, const Token_t &InTok, TokenType ExpType)
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
void CParser::SkipToToken(TokenType Tok)
{
	Token_t TempToken = m_LexScanner.GetNextToken();

	while(TempToken.Type != Tok && m_LexScanner.HasMoreTokens())
		TempToken = m_LexScanner.GetNextToken();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseVariableDeclaration(void)
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

	if(!ExpectNextToken("identifier", VarToken, TOKTYPE_ID))
		return NULL;

	//we've now got enough information to build our tree node
	CVarDeclTreeNode *NewVarDecl;
	NewVarDecl = (CVarDeclTreeNode*)m_ParseTree.BuildVarDecl(VarToken.TokenValue, 
																											     IsConstQualified, VarType);
	assert(NewVarDecl);

	//if we've got a semi-colon we're all done
	if(!IsConstQualified && m_LexScanner.PeekNextToken().Type == TOKTYPE_SEMICOLON)
	{
		m_SymTable.AddSymbol(VarName, NewVarDecl, VarToken.LineNumber);
		m_LexScanner.ClearPeekedTokens();
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
		CParseTreeNode *InitExpr = ParseExpression(EC_INITIALISER);

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

		m_SymTable.AddSymbol(VarName, NewVarDecl, VarToken.LineNumber);
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
CParseTreeNode* CParser::ParseArrayDeclaration(CParseTreeNode *Var)
{
	//Var is the variable we're wrapping in an array declaration
	assert(Var);

	//create our new decl, parsing the array bounds in the process
	CArrayDeclTreeNode *NewArrDecl;
	CParseTreeNode *Bounds = ParseExpression(EC_ARRAY);

	if(!Bounds)
	{
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//array bounds must be of integral type
	if(!TypeCheckExpression(Bounds, TOKTYPE_INTEGERTYPE))
	{
		Error("array bounds must be of integral type");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	NewArrDecl = (CArrayDeclTreeNode*)m_ParseTree.BuildArrayDeclaration(Var, Bounds);

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

	m_SymTable.AddSymbol(((CVarDeclTreeNode*)Var)->VarId->IdName, NewArrDecl, 
											 m_LexScanner.CurrentLineNumber());
	return NewArrDecl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseExpression(ExprContext Context)
{
	stack<TokenType> ExpOpStack;
	stack<CParseTreeNode*> ExpValStack;
	int ParenNestingLevel = 0;
	CParseTreeNode *ExprTerm = NULL;
	
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
		//if the terminating token is a ')' we dont want to 
		//break out early on expressions such as ((6 + 4) * 2 + 1)
		if(Token.Type == TOKTYPE_CLOSEPAREN && EndTokenType == TOKTYPE_CLOSEPAREN)
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
			while(ExpOpStack.top() != TOKTYPE_OPENPAREN)
			{
				if(!ReduceExpression(ExpOpStack, ExpValStack))
					return NULL;

				if(ExpOpStack.empty())
				{
					Error("mismatched ')'");
					DestroyExpressionStack(ExpValStack);
					return NULL;
				}
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
															 stack<CParseTreeNode*> &ValStack)
{
	assert(ValStack.size() >= 2);
	assert(OpStack.size() >= 1);

	//pop the RHS and LHS of the expression
	CParseTreeNode *Rhs = ValStack.top();
	ValStack.pop();

	CParseTreeNode *Lhs = ValStack.top();
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
	ValStack.push(m_ParseTree.BuildBinaryExpression(Lhs, Rhs, Op));
	return true;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
int CParser::OperatorPrecedence(TokenType Op)
{
	map<TokenType, int>::iterator itr = m_OpPrecs.find(Op);
  assert(itr != m_OpPrecs.end());
	
	return itr->second;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CParser::DestroyExpressionStack(stack<CParseTreeNode*> &ValStack)
{
	//clean up values
	deque<CParseTreeNode*>::iterator itr = ValStack.c.begin();
	for(; itr != ValStack.c.end(); itr++)
		(*itr)->DestroyNode();
		
	ValStack.c.clear();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseExpressionTerm(stack<TokenType> &OpStack, int &ParenLevel)
{
	Token_t Token = m_LexScanner.GetNextToken();

	if(Token.Type == TOKTYPE_ID)
	{
		Symbol_t *IdSym = m_SymTable.LookupIdentifier(Token.TokenValue);

		//check that a declaration for this identifier exists
		if(!IdSym)
		{
			Error("undeclared identifier '" + Token.TokenValue + "'");
			return NULL;
		}

		switch(IdSym->IdPtr->Code)
		{
		case TC_PARMDECL:
		case TC_VARDECL:
				return m_ParseTree.BuildIdentifier(Token.TokenValue);
		
		case TC_ARRAYDECL:
			{
				if(!ExpectNextToken("'['", m_LexScanner.GetNextToken(), TOKTYPE_OPENSQRPAREN))
					return NULL;

				//get the index we want
				CParseTreeNode *ArrRef = ParseExpression(EC_ARRAY);

				if(!ArrRef)
					return false;

				if(!ExpectNextToken("']'", m_LexScanner.GetNextToken(), TOKTYPE_CLOSESQRPAREN))
				{
					ArrRef->DestroyNode();
					return NULL;
				}

				 return m_ParseTree.BuildArrayRef(ArrRef, Token.TokenValue);
			}

		case TC_FUNCTIONDECL:
			return ParseFunctionCall(IdSym->Name, false);

		default:
			assert(false);
		}
	}
	else if(IsLiteral(Token.Type))
		return m_ParseTree.BuildLiteral(Token.TokenValue, Token.Type);
	else if(Token.Type == TOKTYPE_OPENPAREN)
	{
		//evaluate the entire expression in place before we continue
		CParseTreeNode *Expr = ParseExpression(EC_CONDITIONAL);

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
	else if(Token.Type == TOKTYPE_MINUS)
	{
		CParseTreeNode *NegExpr = NULL;

		//see if the next token is '(' in which case we're applying
		//the operator to an entire expression rather than just a term
		Token_t ParenTok = m_LexScanner.PeekNextToken();

		if(ParenTok.Type == TOKTYPE_OPENPAREN)
		{
			m_LexScanner.ClearPeekedTokens();
			NegExpr = ParseExpression(EC_CONDITIONAL);

			//get the closing parenthesis
			if(!ExpectNextToken("')'", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
			{
				NegExpr->DestroyNode();
				return NULL;
			}
		}
		else
		{
			//we've actually got a unary expression of the form "neg expr" so 
			//parse the term that's being negated here
			NegExpr = ParseExpressionTerm(OpStack, ParenLevel);
		}

		if(!NegExpr)
			return NULL;
		
		return m_ParseTree.BuildUnaryExpression(NegExpr, TOKTYPE_MINUS);
	}

	return NULL;
}
//-------------------------------------------------------------


//-------------------------------------------------------------
CParseTreeNode* CParser::ParseStatement(void)
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
CParseTreeNode* CParser::ParseIfStatement()
{
	//ParseStatement will have checked that the current token if 'if'
	assert(m_LexScanner.GetNextToken().Type == TOKTYPE_IF);
	Token_t IfToken = m_LexScanner.GetNextToken();

	//can't declare functions inside if statements
	m_FuncDeclAllowed = false;

	//parse the head of the statement
	if(!ExpectNextToken("(", IfToken, TOKTYPE_OPENPAREN))
	{
		SkipToToken(TOKTYPE_ENDIF);
		return NULL;
	}

	CParseTreeNode *CondExpr = ParseExpression(EC_CONDITIONAL);

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

	//emit a warning for cases the conditional expression is a literal value
	//such as if(56) or if(true) since this is always true or false
	if(CondExpr->Code == TC_LITERAL)
		Warning("conditional expression is of constant value");

	CParseTreeNode *IfStmt = m_ParseTree.BuildIfStatement(CondExpr);

	if(!ExpectNextToken(")", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
	{
		SkipToToken(TOKTYPE_ENDIF);
		return NULL;
	}

	//handle cases where there's no body to the if statement
	Token_t EndIfTok = m_LexScanner.GetNextToken();

	//true if we're adding statements to the 'else' body, otherwise
	//we're adding to the 'if' part
	bool AddingToElse = false;

	if(EndIfTok.Type != TOKTYPE_ENDIF)
	{
		m_SymTable.PushNewSymbolScope();
		m_LexScanner.PushbackToken(EndIfTok);

		//parse the body
		while(true)
		{
			CParseTreeNode *Stmt = ParseStatement();

			if(Stmt)
			{
				if(AddingToElse)
					IfStmt->Children[2]->Children.push_back(Stmt);
				else
					IfStmt->Children[1]->Children.push_back(Stmt);
			}

			Token_t EndTok = m_LexScanner.GetNextToken();
			if(EndTok.Type == TOKTYPE_ENDIF)
				break;

			//see if we've hit an else clause, if so then we switch to
			//start adding statements to the else body instead
			if(EndTok.Type == TOKTYPE_ELSE)
			{
				AddingToElse = true;
				continue;
			}

			if(!m_LexScanner.HasMoreTokens())
			{
				Error("unexpected end of file found before 'end_if' was found");
				Stmt->DestroyNode();
				IfStmt->DestroyNode();
				return NULL;
			}

			//the token we've just peeked at is the beginning of another 
			//statement so push it back to enable parsing to continue
			m_LexScanner.PushbackToken(EndTok);
		}

		m_SymTable.PopSymbolScope();
	}
	else
		Warning("empty 'if' statement found");
	
	return IfStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseAssignmentStatement(const string &IdentifierName)
{
	//lookup the identifier to make sure a declaration is available for it
	Symbol_t *IdSym = m_SymTable.LookupIdentifier(IdentifierName);

	if(!IdSym)
	{
		Error("undeclared identifier '" + IdentifierName + "'");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//term appearing on the LHS and RHS of the assignment expr
	CParseTreeNode *AssignExprLhs = NULL;
	CParseTreeNode *AssignExprRhs = NULL;

	//if we've got an array identifier parse the index
	if(IdSym->IdPtr->Code == TC_ARRAYDECL)
	{
		if(!ExpectNextToken("'['", m_LexScanner.GetNextToken(), TOKTYPE_OPENSQRPAREN))
		{
			SkipToToken(TOKTYPE_SEMICOLON);
			return NULL;
		}

		CParseTreeNode *ArrIdx = ParseExpression(EC_ARRAY);

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

		AssignExprLhs = m_ParseTree.BuildArrayRef(ArrIdx, IdSym->Name);
	}
	else if(IdSym->IdPtr->Code == TC_FUNCTIONDECL)
	{
		Error("attempted to assign variable to function '" + IdSym->Name + "'");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}
	else if(IdSym->IdPtr->Code == TC_VARDECL)
	{
		//we only have a plain variable, check to see if it was declared 'const' in which
		//case we can't re-assign a value to it
		if(((CVarDeclTreeNode*)IdSym->IdPtr)->IsConst)
		{
			Error("attempted to assign value to const identifier '" + IdSym->Name + "'");
			SkipToToken(TOKTYPE_SEMICOLON);
			return NULL;
		}

		AssignExprLhs = m_ParseTree.BuildIdentifier(IdSym->Name);
	}
	else if(IdSym->IdPtr->Code == TC_PARMDECL)
	{
		//create a copy of the node for the lhs
		CParameterDeclTreeNode *OrigDecl = (CParameterDeclTreeNode*)IdSym->IdPtr;
		AssignExprLhs = m_ParseTree.BuildParameterDecl(OrigDecl->Name, OrigDecl->Type, OrigDecl->IsConst);
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

	return m_ParseTree.BuildAssignmentExpr(AssignExprLhs, AssignExprRhs);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseWhileStatement()
{
	//ParseStatement will have checked that the current token if 'if'
	assert(m_LexScanner.GetNextToken().Type == TOKTYPE_WHILE);
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
	CParseTreeNode *CondExpr = ParseExpression(EC_CONDITIONAL);

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

	//build the if stmt tree node
	CParseTreeNode *WhileStmt = m_ParseTree.BuildWhileStatement(CondExpr);

	if(!ExpectNextToken(")", m_LexScanner.GetNextToken(), TOKTYPE_CLOSEPAREN))
	{
		SkipToToken(TOKTYPE_END_WHILE);
		return NULL;
	}

	//handle cases where there's no body to the statement
	Token_t EndWhileTok = m_LexScanner.GetNextToken();

	if(EndWhileTok.Type != TOKTYPE_END_WHILE)
	{
		m_SymTable.PushNewSymbolScope();
		m_LexScanner.PushbackToken(EndWhileTok);

		//parse the body
		while(true)
		{
			CParseTreeNode *Stmt = ParseStatement();
			
			if(Stmt)
				WhileStmt->Children.push_back(Stmt);

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
				return NULL;
			}

			m_LexScanner.PushbackToken(EndTok);
		}
		
		m_SymTable.PopSymbolScope();
	}
	else
		Warning("empty 'while' statement found");

	return WhileStmt;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseFunctionDecl(void)
{
	assert(m_LexScanner.GetNextToken().Type == TOKTYPE_FUNCTION);

	//are we already parsing a function?
	if(m_ParsingFuncDecl)
	{
		Error("nested functions not allowed");
		SkipToToken(TOKTYPE_END_FUNCTION);
		return NULL;
	}

	//check to see if a function decl is allowed at this point 
	if(!m_FuncDeclAllowed)
	{
		Error("function declarations are only permitted at global scope");
		SkipToToken(TOKTYPE_END_FUNCTION);
		return NULL;
	}

	m_ParsingFuncDecl = true;
	Token_t FuncToken = m_LexScanner.GetNextToken();

	if(!ExpectNextToken("identifier", FuncToken, TOKTYPE_ID))
	{
		//skip the entire function
		SkipToToken(TOKTYPE_END_FUNCTION);
		m_ParsingFuncDecl = false;
		return NULL;
	}

	//info about the function we're parsing
	string FunctionName = FuncToken.TokenValue;
	CParseTreeNode *ParmList = m_ParseTree.BuildParameterList();
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
	CParseTreeNode *FnDeclNode = m_ParseTree.BuildFunctionDecl(FunctionName, NumParms, ParmList, RetType);
	m_CurrFuncDecl = FnDeclNode;

	//add the function name to the global namespace and push a new scope
	m_SymTable.AddSymbol(FunctionName, FnDeclNode, m_LexScanner.CurrentLineNumber());
	m_SymTable.PushNewSymbolScope();

	//add the parameters to the new function's scope
	for(unsigned int i = 0; i < ParmList->Children.size(); i++)
	{
		CParameterDeclTreeNode *ParmDecl = (CParameterDeclTreeNode*)ParmList->Children[0];
		m_SymTable.AddSymbol(ParmDecl->Name, ParmDecl, 0);
	}

	//parse the function body
	bool FoundReturnStmt = false;
	while(true)
	{
		CParseTreeNode *Stmt = ParseStatement();

		if(Stmt && Stmt->Code == TC_RETURNSTMT)
		{
			//check for a function which isn't supposed to have a return value but does
			if(((CFunctionDeclTreeNode*)FnDeclNode)->RetType == TOKTYPE_VOID)
			{
				Error("function '" + FunctionName + "' declared void but returning a value");

				FnDeclNode->DestroyNode();
				Stmt->DestroyNode();

				SkipToToken(TOKTYPE_END_FUNCTION);
				m_ParsingFuncDecl = false;
				m_SymTable.PopSymbolScope();
				return NULL;
			}

			FoundReturnStmt = true;
		}

		if(Stmt)
			FnDeclNode->Children.push_back(Stmt);

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

			m_CurrFuncDecl = NULL;
			return NULL;
		}

		m_LexScanner.PushbackToken(EndTok);
	}
	
	//see if we're supposed to have a return statement but dont
	if(!FoundReturnStmt && ((CFunctionDeclTreeNode*)FnDeclNode)->RetType != TOKTYPE_VOID)
	{
		Error("function '" + FunctionName + "' missing 'return' statement");
		FnDeclNode->DestroyNode();
	}

	m_SymTable.PopSymbolScope();
	m_ParsingFuncDecl = false;
	return FnDeclNode;

}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseReturnStatement(void)
{
	assert(m_LexScanner.GetNextToken().Type == TOKTYPE_RETURN);

	//return statements are only allowed when we're parsing a function decl
	if(!m_ParsingFuncDecl)
	{
		Error("'return' found outside of a function declaration");
		SkipToToken(TOKTYPE_SEMICOLON);
		return NULL;
	}

	//parse the expression forming the return value
	CParseTreeNode *RetExpr = ParseExpression(EC_INITIALISER);

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
			RetExpr = m_ParseTree.BuildLiteral("0", TOKTYPE_INTEGERTYPE);
		}

		return m_ParseTree.BuildReturnStmt(RetExpr);
	}
	else
		SkipToToken(TOKTYPE_SEMICOLON);

	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::ParseFuncDeclParameterList(CParseTreeNode *ParmList, int &NumParms)
{
	//tokens to store the type and name of the parameters
	Token_t ParmTypeToken;
	Token_t ParmNameToken;
	Token_t ParmToken;
	bool IsConstParm = false;

	NumParms = 0;

	while(true)
	{
		//next two tokens should be a type specifier and an identifier
		ParmTypeToken = m_LexScanner.GetNextToken();

		if(ParmTypeToken.Type == TOKTYPE_CONST)
		{
			IsConstParm = true;
			ParmTypeToken = m_LexScanner.GetNextToken();
		}
		else
			IsConstParm = false;

		if(!IsType(ParmTypeToken.Type))
		{
			Error("expected type specifier before '" + ParmTypeToken.TokenValue + "'");
			return false;
		}

		ParmNameToken = m_LexScanner.GetNextToken();
		if(!ExpectNextToken("identifier", ParmNameToken, TOKTYPE_ID))
			return false;

		CParseTreeNode *ParmDecl = m_ParseTree.BuildParameterDecl(ParmNameToken.TokenValue, 
																	ParmTypeToken.Type, IsConstParm);

		ParmList->Children.push_back(ParmDecl);
		ParmToken = m_LexScanner.GetNextToken();
		++NumParms;

		//if the next token is ')' then we're all done with parsing the parameter list
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
CParseTreeNode *CParser::ParseFunctionCall(const string &FuncName, bool RequireSemiColon)
{
	//lookup the identifier to make sure it exists
	Symbol_t *IdSym = m_SymTable.LookupIdentifier(FuncName);

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
	CParseTreeNode *CallExprNode = NULL;

	//if we've got a closing parenthesis next, check to see if the function
	//we're calling was declared with parameters or not
	if(m_LexScanner.PeekNextToken().Type == TOKTYPE_CLOSEPAREN)
	{
		if(FnDecl->NumParameters != 0)
		{
			Error("missing parameters for call to function '" + FuncName + "'");
			SkipToToken(TOKTYPE_SEMICOLON);
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
			return NULL;
		}

		CallExprNode = m_ParseTree.BuildCallExpr(FuncName);

		//the type of token we want to terminate each parameter expression on
		//depends on how many parameters we're looking for, if it's > 1 then 
		//the terminating token is ',' else it's ')'
		if(FnDecl->NumParameters == 1)
		{
			CParseTreeNode *Parm = ParseExpression(EC_CONDITIONAL);

			if(!Parm
				 || !TypeCheckExpression(Parm, ExtractTypeFromGenericExpr(FnDecl->Children[1]->Children[0])))
			{
				if(Parm)
					Parm->DestroyNode();

				SkipToToken(TOKTYPE_SEMICOLON);
				CallExprNode->DestroyNode();
				return NULL;
			}
			else
				CallExprNode->Children.push_back(Parm);
		}
		else
		{
			CParseTreeNode *Parm = NULL;
			int i = 0;

			for(; i < FnDecl->NumParameters - 1; i++)
			{
				//parse the expression forming the parameter value
				Parm = ParseExpression(EC_FUNCCALLPARMS);

				if(Parm)
				{
					//multiple parameters are separated by a comma
					if(!ExpectNextToken("','", m_LexScanner.GetNextToken(), TOKTYPE_COMMA)
						 || !TypeCheckExpression(Parm, ExtractTypeFromGenericExpr(FnDecl->Children[1]->Children[i])))
					{
						SkipToToken(TOKTYPE_SEMICOLON);
						CallExprNode->DestroyNode();
						return NULL;
					}

					CallExprNode->Children.push_back(Parm);
				}
			}

			//the final parameter expression is terminated by the closing parenthesis
			//rather than a comma like the first lot
			Parm = ParseExpression(EC_CONDITIONAL);

			if(!Parm 
				 || !TypeCheckExpression(Parm, ExtractTypeFromGenericExpr(FnDecl->Children[1]->Children[i])))
			{
				SkipToToken(TOKTYPE_SEMICOLON);
				CallExprNode->DestroyNode();
				return NULL;
			}

			CallExprNode->Children.push_back(Parm);
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

	return CallExprNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CParseTreeNode* CParser::ParseFuncCallOrAssignmentExpr(void)
{
	Token_t Identifier = m_LexScanner.GetNextToken();
	assert(Identifier.Type == TOKTYPE_ID);

	//see if we have a function call or an assignment expr
	Token_t PeekedTok = m_LexScanner.PeekNextToken();

	if(PeekedTok.Type == TOKTYPE_OPENPAREN)
		return ParseFunctionCall(Identifier.TokenValue, true);
	else
		return ParseAssignmentStatement(Identifier.TokenValue);

	Error("unexpected '" + PeekedTok.TokenValue + "'");
	return NULL;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::TypeCheckExpression(CParseTreeNode *Expr, TokenType RequiredType)
{
	TokenType FromType = (TokenType)-1;

	//we need to handle literal and binary expressions separately because of
	//type conversion issues
	if(Expr->Code == TC_LITERAL)
	{
		CLiteralTreeNode *Literal = (CLiteralTreeNode*)Expr;

		//see if the types match, if not see if the type of the expression
		//can be converted to that of the requested type
		if(MatchLiteralTypeToVarType(RequiredType, Literal->LitType)
			 || TypeConvertsTo(RequiredType, Literal->LitType))
		 return true;
		else
			FromType = Literal->LitType;
	}
	else if(Expr->Code == TC_BINARY_EXPR)
	{
		//check binary expressions
		CBinaryExprTreeNode *BinExpr = (CBinaryExprTreeNode*)Expr;

		bool LhsTypeOk = false;
		bool RhsTypeOk = false;

		//if we've got a binary expression on either side of the expr, 
		//recursively typecheck each side
		if(BinExpr->Children[0]->Code == TC_BINARY_EXPR)
			LhsTypeOk = TypeCheckExpression(BinExpr->Children[0], RequiredType);

		if(BinExpr->Children[1]->Code == TC_BINARY_EXPR)
			RhsTypeOk = TypeCheckExpression(BinExpr->Children[1], RequiredType);

		//if we've got here then we must have something like "val op val", check
		//that the types of "val" match the required types for "op"
		TokenType LhsType = ExtractTypeFromGenericExpr(BinExpr->Children[0]);
		TokenType RhsType = ExtractTypeFromGenericExpr(BinExpr->Children[1]);

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

			//we've got operand types which are applicable for the type of operator 
			//being used, now handle cases like "5.4 + 4" which is a mix of real and 
			//integral types in the same expression, for this we only issue a warning 
			//about the implicit conversion of the integral operand to real type since 
			//only real or integral values are allowed this message will always be accurrate
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
	if(Orig == TOKTYPE_FLOATTYPE && To == TOKTYPE_INTLITERAL)
		return true;

	//check instances where we have TYPE = LITERAL 
	if(IsType(Orig) && IsLiteral(To))
		return MatchLiteralTypeToVarType(Orig, To);

	return false;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CParser::MatchLiteralTypeToVarType(TokenType VarType, TokenType LiteralType)
{
	switch(VarType)
	{
	case TOKTYPE_INTEGERTYPE:
		return LiteralType == TOKTYPE_INTLITERAL;

	case TOKTYPE_FLOATTYPE:
		return LiteralType == TOKTYPE_FLOATLITERAL;

	//integral/floating point constants are implicitly boolean types
	case TOKTYPE_BOOLEANTYPE:
		return LiteralType == TOKTYPE_TRUE
							|| LiteralType == TOKTYPE_FALSE
							|| LiteralType == TOKTYPE_INTLITERAL
							|| LiteralType == TOKTYPE_FLOATLITERAL;

	case TOKTYPE_STRINGTYPE:
		return LiteralType == TOKTYPE_STRINGLITERAL;
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
TokenType CParser::ExtractTypeFromGenericExpr(CParseTreeNode *Expr)
{
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

	case TC_BINARY_EXPR:
		return ExtractTypeFromBinaryExpr((CBinaryExprTreeNode*)Expr);

	case TC_UNARY_EXPR:
		return ExtractTypeFromGenericExpr(Expr->Children[0]);

	case TC_FUNCTIONDECL:
		return ((CFunctionDeclTreeNode*)Expr)->RetType;
	}

	//oops
	assert(false);
	return (TokenType)-1;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromArrayRef(CArrayRefTreeNode *ArrRef)
{
	//get the array decl
	Symbol_t *ArrSym = m_SymTable.LookupIdentifier(ArrRef->ArrName);
	assert(ArrSym);

	//return the type
	CVarDeclTreeNode *ArrDecl = (CVarDeclTreeNode*)ArrSym->IdPtr->Children[0];
	return ArrDecl->VarType;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromIdentifier(CIdentifierTreeNode *Ident)
{
	Symbol_t *IdSym = m_SymTable.LookupIdentifier(Ident->IdName);
	assert(IdSym);

	//get the underlying type of the decl
	if(IdSym->IdPtr->Code == TC_VARDECL)
		return ((CVarDeclTreeNode*)IdSym->IdPtr)->VarType;

	//we must have a parm decl
	assert(IdSym->IdPtr->Code == TC_PARMDECL);
	return ((CParameterDeclTreeNode*)IdSym->IdPtr)->Type;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromCallExpr(CCallExprTreeNode *CallExpr)
{
	Symbol_t *CallSym = m_SymTable.LookupIdentifier(CallExpr->FuncName);
	assert(CallSym);

	CFunctionDeclTreeNode *FnDecl = (CFunctionDeclTreeNode*)CallSym->IdPtr;
	return FnDecl->RetType;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
TokenType CParser::ExtractTypeFromBinaryExpr(CBinaryExprTreeNode *BinExpr)
{
	//if we've got a binary operator for this expr, the type of the
	//whole (sub)expression is Boolean
	if(IsBooleanOperator(BinExpr->Operator))
		return TOKTYPE_BOOLEANTYPE;

	//must have an arithmetic expression
	assert(IsArithmeticOperator(BinExpr->Operator));

	//since arithmetic operators are left associative the result of the
	//expression must be of a type compatible with the LHS of the operator
	return ExtractTypeFromGenericExpr(BinExpr->Children[0]);
}
//-------------------------------------------------------------