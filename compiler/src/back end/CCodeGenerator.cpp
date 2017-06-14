//------------------------------------------------------------------------------------------
// File: CCodeGenerator.h
// Desc: Generate x86 assembly code from a series of MIR statements
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CCodeGenerator.h"
#include "CGlobalDataStore.h"
#include "CSymbolTable.h"
#include "CTreeNode.h"
#include "CIdentifierTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CLiteralRefTreeNode.h"
#include "CFunctionDeclTreeNode.h"
#include "CLiteralTreeNode.h"
#include "CArithExprTreeNode.h"
#include "CLabelTreeNode.h"
#include "CBooleanExprTreeNode.h"
#include "CGotoExprTreeNode.h"
#include "CParameterDeclTreeNode.h"
#include "CCallExprTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CArrayDeclTreeNode.h"
#include "CUtilFuncs.h"

//-------------------------------------------------------------
void CCodeGenerator::AssembleMIRTree(const string &OutputFile, CTreeNode *MirTree)
{
	m_OutputFile.open(OutputFile.c_str());
	SanityCheck(m_OutputFile.is_open());

	OutputDataSections();

	m_OutputFile << "[section .text]" << endl << endl;
	m_OutputFile << "global _main" << endl << endl;
	CGlobalDataStore::GetSingletonPtr()->OutputExternDeclarations(m_OutputFile);

	//take each statement in turn and generate the assembly code for it
	for(unsigned int i = 0; i < MirTree->GetNumChildren(); i++, m_CurrStmtNo++)
	{
		//filter out anything which isn't a function
		if(MirTree->GetChild(i)->Code != TC_FUNCTIONDECL)
			continue;

		CFunctionDeclTreeNode *FnDecl;
		FnDecl = (CFunctionDeclTreeNode*)MirTree->GetChild(i);

		//output the name of the function followed by the prologue
		m_OutputFile << endl << FnDecl->Name << ":" << endl;
		OutputFunctionPrologue(FnDecl->StackFrameSize);

		//go through the function body and generate the code for each stmt in it
		for(unsigned int j = 1; j < FnDecl->GetNumChildren(); j++)
			GenerateCodeForStatement(FnDecl->GetChild(j));

		//if the function returns void we won't have output the epilogue
		//so do that now
		if(FnDecl->RetType == TOKTYPE_VOID)
			OutputFunctionEpilogue();
	}

	m_OutputFile.close();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::OutputDataSections(void)
{
	m_OutputFile << "[section .data]" << endl;
	CGlobalDataStore::GetSingletonPtr()->OutputLiterals(m_OutputFile);
	
	m_OutputFile << endl << "[section .bss]" << endl;
	CGlobalDataStore::GetSingletonPtr()->OutputGlobalVariables(m_OutputFile);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForStatement(CTreeNode *Stmt)
{
	switch(Stmt->Code)
	{
	case TC_ARRAYDECL:
	case TC_VARDECL:
		{
			//var/array decls are ignored
			break;
		}

	case TC_ASSIGN_EXPR:
		{
			GenerateCodeForAssignExpr(Stmt);
			break;
		}

	case TC_ARITH_EXPR:
		{
			GenerateCodeForArithExpr(Stmt);
			break;
		}

	case TC_IFSTMT:
		{
			GenerateCodeForIfStmt(Stmt);
			break;
		}

	case TC_LABEL:
		{
			m_OutputFile << ((CLabelTreeNode*)Stmt)->Name << ":" << endl;
			break;
		}

	case TC_GOTOEXPR:
		{
			//a goto on it's own is an unconditional jump
			m_OutputFile << "\tjmp\t" << ((CGotoExprTreeNode*)Stmt)->Target << endl;
			break;
		}

	case TC_CALLEXPR:
		{
			GenerateCodeForCallExpr(Stmt);
			break;
		}

	case TC_UNARY_EXPR:
		{
			GenerateCodeForNegExpr(Stmt);
			break;
		}

	case TC_RETURNSTMT:
		{
			GenerateCodeForReturnStmt(Stmt);
			break;
		}

	default:
		{
			InternalError("unhandled stmt type");
			break;
		}
	}

	++m_CurrStmtNo;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::ProcessExprOperand(CTreeNode *ExprOp, InsnOp_t &Op)
{
	switch(ExprOp->Code)
	{
	case TC_VARDECL:
		{
			ProcessVarDeclExprOperand(ExprOp, Op);
			return;
		}

	case TC_ARRAY_REF:
		{
			//get the underlying var_decl
			CArrayRefTreeNode *Ref = (CArrayRefTreeNode*)ExprOp;
			CArrayDeclTreeNode *ArrDecl = (CArrayDeclTreeNode*)CUtilFuncs::GetDeclFromNode(
																			Ref->ArrName);
			CVarDeclTreeNode *ArrVar = (CVarDeclTreeNode*)ArrDecl->GetChild(ARR_DECL_VAR);
			CTreeNode *Index = Ref->GetChild(ARR_REF_INDEX);

			//get the base address of the array in the stack frame
			int BaseAddr = ArrVar->StackFrameAddr;

			if(Index->Code == TC_LITERAL)
			{
				//calculate the offset address in the array for the
				//element being accessed
				int ElementOffset = CUtilFuncs::GetLiteralValue<int>(
						(CLiteralTreeNode*)Index) * 4;

				Op.Loc = CalcStackFrameAddress(false, ElementOffset + BaseAddr);
			}
			
			Op.InMemory = true;
			return;
		}

	case TC_LITERAL:
		{
			Op.InMemory = false;
			Op.Loc = ((CLiteralTreeNode*)ExprOp)->LitValue;
			return;
		}

	case TC_IDENTIFIER:
		{
			ProcessExprOperand(CUtilFuncs::GetDeclFromNode(ExprOp), Op);
			return;
		}

	case TC_PARMDECL:
		{
			Op.InMemory = true;
			Op.Loc = CalcStackFrameAddress(true, ((CParameterDeclTreeNode*)ExprOp)->Position);
			return;
		}	

	case TC_UNARY_EXPR:
		{
			SanityCheck(ExprOp->GetChild(UNARY_EXPR_EXPR)->Code != TC_IDENTIFIER);
			ProcessExprOperand(ExprOp->GetChild(UNARY_EXPR_EXPR), Op);
			return;
		}

	case TC_LITERALREF:
		{
			GetDeclLocation(ExprOp, Op);
			return;
		}
	}
	
	InternalError("unknown LHS type");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::ProcessVarDeclExprOperand(CTreeNode *Decl, InsnOp_t &Op)
{
	SanityCheck(Decl->Code == TC_VARDECL);
	CVarDeclTreeNode *VarLhs = (CVarDeclTreeNode*)Decl;

	//if this var has been spilled and we're past the location at which
	//it was spilled we need to load it from the stack frame. 
	if((VarLhs->Spilled && m_CurrStmtNo >= VarLhs->SpillPoint) || VarLhs->GlobalVar)
		GetDeclLocation(Decl, Op);
	else if(VarLhs->VarType == TOKTYPE_STRINGTYPE)
	{
		//strings and global variables are also loaded from memory
		Op.InMemory = true;
		Op.Loc = VarLhs->VarId->IdName;
	}
	else
	{
		Op.Loc = CUtilFuncs::HardRegisterToStr(VarLhs->AllocatedReg);
		Op.InMemory = false;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GetDeclLocation(CTreeNode *Decl, InsnOp_t &Op)
{
	switch(Decl->Code)
	{
	case TC_VARDECL:
		{
			CVarDeclTreeNode *VarDecl = (CVarDeclTreeNode*)Decl;

			if(VarDecl->Spilled && m_CurrStmtNo >= VarDecl->SpillPoint 
				 || VarDecl->GlobalVar)
			{
				Op.InMemory = true;

				if(!VarDecl->GlobalVar)
					Op.Loc = CalcStackFrameAddress(false, VarDecl->StackFrameAddr);
				else
					Op.Loc = "dword [" + VarDecl->VarId->IdName + "]";
			}
			else
			{
				Op.Loc = CUtilFuncs::HardRegisterToStr(VarDecl->AllocatedReg);
				Op.InMemory = false;
			}

			return;
		}

	case TC_PARMDECL:
		{
			CParameterDeclTreeNode *ParmDecl = (CParameterDeclTreeNode*)Decl;

			Op.InMemory = true;
			Op.Loc = CalcStackFrameAddress(true, ParmDecl->Position);
 			return;
		}

	case TC_IDENTIFIER:
		{
			GetDeclLocation(CUtilFuncs::GetDeclFromNode(Decl), Op);
			return;
		}

	case TC_LITERAL:
		{
			Op.InMemory = false;
			Op.Loc = ((CLiteralTreeNode*)Decl)->LitValue;
			return;
		}

	case TC_UNARY_EXPR:
		{
			GetDeclLocation(Decl->GetChild(UNARY_EXPR_EXPR), Op);
			return;
		}

	case TC_LITERALREF:
		{
			CLiteralRefTreeNode *RefNode = (CLiteralRefTreeNode*)Decl;
			CLiteralTreeNode *RefLit = (CLiteralTreeNode*)RefNode->GetChild(LITREF_LITERAL);
			Op.InMemory = true;

			//if we're referencing a string then we don't want the square
			//brackets as we only work with the *address* of the literal, not
			//the actual value like we do for floating point values
			if(RefLit->LitType == TOKTYPE_STRINGLITERAL)
				Op.Loc = "dword " + RefNode->RefID;
			else
				Op.Loc = "dword [" + RefNode->RefID + "]";

			return;
		}
	}

	InternalError("unhandled decl type");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForAssignExpr(CTreeNode *AssignExpr)
{
	SanityCheck(AssignExpr->Code == TC_ASSIGN_EXPR);

	//get the LHS/RHS
	CTreeNode *AssignExprLhs;
	AssignExprLhs = CUtilFuncs::GetDeclFromNode(AssignExpr->GetChild(ASSIGN_EXPR_ID));

	CTreeNode *AssignExprRhs = AssignExpr->GetChild(ASSIGN_EXPR_RHS);

	//see if we're assigning a value into a floating point value
	if(CUtilFuncs::ExprRefersToFloatingPointType(AssignExprLhs))
	{
		GenerateCodeForFPUAssignExpr(AssignExprLhs, AssignExprRhs);
		return;
	}
	else
	{
		//see if we're truncating a floating point value
		if(!CUtilFuncs::ExprRefersToFloatingPointType(AssignExprLhs)
			 && CUtilFuncs::ExprRefersToFloatingPointType(AssignExprRhs))
		{
			TruncateFloatToInteger(AssignExprLhs, AssignExprRhs);
			return;
		}
	}

	//if the RHS of the expression is not a literal value or a reference
	//to some variable then process that 
	if(AssignExprRhs->Code != TC_IDENTIFIER 
		 && AssignExprRhs->Code != TC_LITERAL
		 && AssignExprRhs->Code != TC_LITERALREF
		 && AssignExprRhs->Code != TC_ARRAY_REF)
	{
		//literal values can also be hiding in unary_exprs
		if(AssignExprRhs->Code != TC_UNARY_EXPR
			 || (AssignExprRhs->Code == TC_UNARY_EXPR
			     && AssignExprRhs->GetChild(UNARY_EXPR_EXPR)->Code != TC_LITERAL))
		{
			GenerateCodeForStatement(AssignExprRhs);
			return;
		}
	}

	//the instructions operands
	InsnOp_t LhsOp;
	InsnOp_t RhsOp;

	//see if either side references an array as these need to be handled separately
	if(AssignExprLhs->Code == TC_ARRAY_REF
		 || AssignExprRhs->Code == TC_ARRAY_REF)
	{
		GenerateCodeForArrayAssignExpr(AssignExprLhs, AssignExprRhs);
		return;
	}

	//process the lhs and rhs to determine their locations and whether they
	//are located in memory or not
	ProcessExprOperand(AssignExprLhs, LhsOp);
	ProcessExprOperand(AssignExprRhs, RhsOp);

	//the mov instruction doesn't support having both operands refer
	//to a memory location so in this instance we push the rhs onto
	//the stack and then pop it into the lhs
	if(LhsOp.InMemory && RhsOp.InMemory)
	{
		//push the RHS onto the stack so we can pop it off info the LHS
		m_OutputFile << "\tpush\t" << RhsOp.Loc << endl;
		m_OutputFile << "\tpop\t" << LhsOp.Loc << endl;
	}
	else
	{
		//check for something like "integer a = 5.667" where we need to 
		//truncate the RHS before storing it to the LHS
		if(CUtilFuncs::ExprRefersToFloatingPointType(AssignExprRhs))
			TruncateFloatToInteger(AssignExprLhs, AssignExprRhs);
		else
		{
			//generate the instruction to assign the RHS to the LHS
			m_OutputFile << "\tmov\t" << LhsOp.Loc << "," << RhsOp.Loc << endl;
		}
	}

	//if the RHS was a unary expression we need to negate the value
	//stored in the LHS
	if(AssignExprRhs->Code == TC_UNARY_EXPR)
		m_OutputFile << "\tneg\t" << LhsOp.Loc << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForFPUAssignExpr(CTreeNode *Lhs, CTreeNode *Rhs)
{
	//process the RHS if required
	if(Rhs->Code == TC_ARITH_EXPR)
		GenerateCodeForStatement(Rhs);

	//process the LHS
	InsnOp_t LhsOp;
	ProcessExprOperand(Lhs, LhsOp);
	
	//if the RHS is not a floating point type then the result
	//will already have been stored as part of the processing otherwise
	//we need to pop the result of the FPU stack into the LHS
	if(Rhs->Code == TC_ARITH_EXPR)
	{
		m_OutputFile << "\tfstp\t" << LhsOp.Loc << endl;
		return;
	}

	//process the RHS
	InsnOp_t RhsOp;
	ProcessExprOperand(Rhs, RhsOp);

	//since we're moving a value to a stack slot, we may need to load the 
	//RHS into a scratch reg before moving into the LHS, use eax as the scratch 
	//reg since we know neither of the operands are in a register
	m_OutputFile << "\tpush\teax" << endl;
	m_OutputFile << "\tmov\teax," << RhsOp.Loc << endl;
	m_OutputFile << "\tmov\t" << LhsOp.Loc << ",eax" << endl;
	m_OutputFile << "\tpop\teax" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForArithExpr(CTreeNode *Expr)
{
	SanityCheck(Expr->Code == TC_ARITH_EXPR);
	CArithExprTreeNode *ArithExpr = (CArithExprTreeNode*)Expr;

	//we know from the MIR spec that the value appearing on the LHS
	//of the expression is also the one we're assigning into
	CIdentifierTreeNode *Lhs;
	CTreeNode *Rhs;

	Lhs = (CIdentifierTreeNode*)ArithExpr->GetChild(ARITH_EXPR_LHS);
	Rhs = ArithExpr->GetChild(ARITH_EXPR_RHS);

	//instruction operands
	InsnOp_t LhsOp;
	InsnOp_t RhsOp;

	//determine where to load the LHS and RHS from
	GetDeclLocation(CUtilFuncs::GetDeclFromNode(Lhs), LhsOp);
	GetDeclLocation(Rhs, RhsOp);

	//if either of the types are floating point types, do the computation
	//on the FPU. We can always truncate the result later if it's being stored
	//into a integer type location
	bool DoCalcOnFpu = false;

	if(CUtilFuncs::ExprRefersToFloatingPointType(CUtilFuncs::GetDeclFromNode(Lhs))
		|| CUtilFuncs::ExprRefersToFloatingPointType(CUtilFuncs::GetDeclFromNode(Rhs)))
		DoCalcOnFpu = true;

	//determine which instruction to use based on the operator
	string InsnName = GetArithInsnName(ArithExpr->Operator, DoCalcOnFpu);

	//due to the quirks of the x86 ISA, the arithmetic instructions don't share a 
	//common format - imul can take 1, 2, or 3 operands but doesn't support having 
	//it's dest operand referring to a memory location like the add and sub 
	//instructions do and the idiv instruction only takes one operand which is the 
	//value to divide by and requires the value to be divided reside in edx:eax where 
	//the result is written to. Because of this we need to check which instruction is 
	//being used and handle it appropiately so we generate correct code 
	if(!DoCalcOnFpu)
	{
		if(InsnName == "add" || InsnName == "sub")
			GenerateCodeForIntAddOrSubInsn(LhsOp, RhsOp, InsnName, 
					ArithExpr->GetChild(ARITH_EXPR_RHS));
		else if(InsnName == "imul")
			GenerateCodeForImulInsn(LhsOp, RhsOp, Lhs);
		else if(InsnName == "idiv")
			GenerateCodeForIdivInsn(LhsOp, RhsOp);
	}
	else
	{
		//the FPU instruction's aren't as awkward as the integer instructions
		//so we don't need to special case their handling. The result of the
		//computation will be in st0
		m_OutputFile << "\tfld\t" << RhsOp.Loc << endl;
		m_OutputFile << "\tfld\t" << LhsOp.Loc << endl;
		m_OutputFile << "\t" << InsnName << "\tst1" << endl;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForIfStmt(CTreeNode *Stmt)
{
	SanityCheck(Stmt->Code == TC_IFSTMT); 
	string JumpInsn;

	//extract the cond_expr
	CBooleanExprTreeNode *CondExpr;
	CondExpr = (CBooleanExprTreeNode*)Stmt->GetChild(IF_STMT_COND_EXPR);

	if(CUtilFuncs::ExprRefersToFloatingPointType(CondExpr))
		GenerateCodeForFPUComparison(CondExpr, JumpInsn);
	else if(CondExpr->Code == TC_BOOLEAN_EXPR)
	{
		//determine which version of the jump instruction we need to use
		JumpInsn = GetJumpInsnForBooleanOp(CondExpr->Operator);

		//instruction operands
		InsnOp_t LhsOp;
		InsnOp_t RhsOp;

		ProcessExprOperand(CondExpr->GetChild(BLN_EXPR_LHS), LhsOp);
		ProcessExprOperand(CondExpr->GetChild(BLN_EXPR_RHS), RhsOp);

		//the cmp instruction doesn't allow both it's operands to be in memory 
		//so if that happens we need to temporarily load one into a scratch register 
		//to carry out the operation
		if(LhsOp.InMemory && RhsOp.InMemory)
		{
			string ScratchReg = GetScratchReg(CondExpr->GetChild(BLN_EXPR_RHS));

			m_OutputFile << "\tpush\t" << ScratchReg << endl;
			m_OutputFile << "\tmov\t" << ScratchReg << "," << RhsOp.Loc << endl;
			m_OutputFile << "\tcmp\t" << LhsOp.Loc << "," << ScratchReg << endl;
			m_OutputFile << "\tpop\t" << ScratchReg << endl;
		}
		else
			m_OutputFile << "\tcmp\t" << LhsOp.Loc << "," << RhsOp.Loc << endl;
	}
	else
	{
		//we've got something like if(literal) or if(var)
		InsnOp_t CondOp;
		ProcessExprOperand(Stmt->GetChild(IF_STMT_COND_EXPR), CondOp);

		//we need a scratch register to load the value into 
		string ScratchReg;

		if(Stmt->GetChild(IF_STMT_COND_EXPR)->Code == TC_IDENTIFIER)
			ScratchReg = GetScratchReg(CUtilFuncs::GetDeclFromNode(
				Stmt->GetChild(IF_STMT_COND_EXPR)));
		else
		{
			//eax is as good as any so use that
			ScratchReg = "eax";
		}

		//move the value of the var or lit into a temp register
		//to perform the comparison. This can be optimised to remove the 
		//push/pop since this is only needed if the cond_expr is a variable
		//but quality of the final code isn't the main concern here
		m_OutputFile << "\tpush\t" << ScratchReg << endl;
		m_OutputFile << "\tmov\t" << ScratchReg << "," << CondOp.Loc << endl;
		m_OutputFile << "\tcmp\t" << ScratchReg << ",0" << endl;
		m_OutputFile << "\tpop\t" << ScratchReg << endl;
		JumpInsn = "jnz";
	}

	//get the target of the jump if the comparison succeeds
	CGotoExprTreeNode *SuccJmp;
	SuccJmp = (CGotoExprTreeNode*)Stmt->GetChild(IF_STMT_BODY)->GetChild(0);
	m_OutputFile << "\t" << JumpInsn << " \t" <<  SuccJmp->Target << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CCodeGenerator::GetArithInsnName(TokenType Op, bool FpuCalc)
{
	switch(Op)
	{
	case TOKTYPE_PLUS:
		return FpuCalc ? "fadd" : "add";
			
	case TOKTYPE_MULTIPLY:
		return FpuCalc ? "fmul" : "imul";
			
	case TOKTYPE_DIVIDE:
		return FpuCalc ? "fdiv" : "idiv";
			
	case TOKTYPE_MINUS:
		return FpuCalc ? "fsub" : "sub";
	}

	InternalError("unrecognisable arithmetic operator");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CCodeGenerator::GetJumpInsnForBooleanOp(TokenType Op)
{
	switch(Op)
	{
	case TOKTYPE_LESSTHAN:
		return "jl";

	case TOKTYPE_LESSTHANEQ:
		return "jle";

	case TOKTYPE_GREATTHAN:
		return "jg";

	case TOKTYPE_GREATTHANEQ:
		return "jge";

	case TOKTYPE_NOTEQ:
		return "jne";

	case TOKTYPE_EQUALITY:
		return "je";
	}

	InternalError("unknown Boolean operator");
}		
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::OutputFunctionPrologue(int StackFrameSize)
{
	//although all 3 instructions below can be replaced with a single
	//'enter' instruction, it's actually faster this way
	m_OutputFile << "\tpush\tebp" << endl;
	m_OutputFile << "\tmov\tebp,esp" << endl;

	if(StackFrameSize != 0)
		m_OutputFile << "\tsub\tesp," << StackFrameSize << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::OutputFunctionEpilogue(void)
{
	m_OutputFile << "\tmov\tesp,ebp" << endl;
	m_OutputFile << "\tpop\tebp" << endl;
	m_OutputFile << "\tret" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CCodeGenerator::GetScratchReg(CTreeNode *Decl)
{
	//Decl is referring to something which could potentially reside
	//in a register so first thing we need to do is determine whether 
	//it is or not
	if(Decl->Code != TC_VARDECL)
	{
		//we can use any register we like so pick eax
		return "eax";
	}

	//Decl is referring to a variable so now determine if it's
	//in memory or a register
	CVarDeclTreeNode *VarDecl = (CVarDeclTreeNode*)Decl;

	if(VarDecl->GlobalVar || (VarDecl->Spilled && VarDecl->SpillPoint > m_CurrStmtNo))
	{
		//it's in memory so we can use anything we like again
		return "eax";
	}

	//here we know for certain it's in a register so pick one which 
	//it's not stored in
	switch(VarDecl->AllocatedReg)
	{
	case REG_EAX:
		return "ebx";

	case REG_EBX:
	case REG_ECX:
	case REG_EDX:
		return "eax";
	}

	//whoops
	InternalError("unable to determine scratch register");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
string CCodeGenerator::CalcStackFrameAddress(bool ParmAddr, int Slot)
{
	//compute the offset in bytes to load from
	int Offset = Slot;
	string Op;

	//the first parameter is stored at [ebp+8]
	if(ParmAddr)
	{
		//the "slot" parameter is actually the parameters position so 
		//we need to multiply it by 4 to get the correct offset
		Offset *= 4;
		Offset += 8;
		Op = " + ";
	}
	else
	{
		//first local var slot is at [ebp-4]
		Op = " - ";
	}

	stringstream ss;
	ss << "dword [ebp" << Op << Offset << "]";

  return ss.str();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForCallExpr(CTreeNode *Expr)
{
	SanityCheck(Expr->Code == TC_CALLEXPR);

	//the child nodes of a call_expr are arameters to the function, so go
	//through each one and push it onto the stack in the reverse order that they
	//was declared in to ensure the first parameter is at address [ebp+8]
	InsnOp_t ParmOp;

	//save the contents of the registers so they don't get clobbered by
	//the function call
	m_OutputFile << "\tpushad" << endl;

	for(int i = Expr->GetNumChildren() - 1; i >= 0; i--)
	{
		ProcessExprOperand(Expr->GetChild(i), ParmOp);
		m_OutputFile << "\tpush\t" << ParmOp.Loc << endl;
	}

	//call the function
	m_OutputFile << "\tcall\t" << ((CCallExprTreeNode*)Expr)->FuncName << endl;

	//remove the arguments from the stack and restore the registers
	m_OutputFile << "\tadd\tesp," << Expr->GetNumChildren() * 4 << endl;
	m_OutputFile << "\tpopad" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForNegExpr(CTreeNode *Expr)
{
	//we shouldn't be called with a literal expression
	SanityCheck(Expr->GetChild(UNARY_EXPR_EXPR)->Code != TC_LITERAL);

	//process the expression to determine it's location
	InsnOp_t NegExprOp;
	ProcessExprOperand(Expr->GetChild(UNARY_EXPR_EXPR), NegExprOp);

	//it doesn't matter where the value being negated resides as the
	//neg instruction will happily process both register and memory operands
	m_OutputFile << "\tneg\t" << NegExprOp.Loc << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForReturnStmt(CTreeNode *Stmt)
{
	//process the operand
	InsnOp_t RetStmtOp;
	ProcessExprOperand(Stmt->GetChild(RET_STMT_RET_EXPR), RetStmtOp);

	//emit the code, the return value from a function is saved in EAX
	m_OutputFile << "\tmov\teax," << RetStmtOp.Loc << endl;
	OutputFunctionEpilogue();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForImulInsn(InsnOp_t &LhsOp, InsnOp_t &RhsOp,
																						 CTreeNode *Lhs)
{
	//the imul instruction is different from the other instructions in that
	//it can take 3 operands rather than 2 like the others at the expense of not
	//allowing the destination operand to be a memory location. Detect that
	//here and jiggle the values about to get the data in the right place for the
	//instruction to work with

	//if the LHS is in a register then we're ok to proceed
	if(!LhsOp.InMemory)
		m_OutputFile << "\timul\t" << LhsOp.Loc << "," << RhsOp.Loc << endl;
	else
	{
		//get a scratch register to load the LHS into 
		string ScratchReg = GetScratchReg(CUtilFuncs::GetDeclFromNode(Lhs));

		//load the LHS into the scratch reg, perform the calculation
		//and then store the result back into the lhs
		m_OutputFile << "\tpush\t" << ScratchReg << endl;
		m_OutputFile << "\tmov\t" << ScratchReg << "," << LhsOp.Loc << endl;
		m_OutputFile << "\timul\t" << ScratchReg << "," << RhsOp.Loc << endl;
		
		//store the result back into the LHS and restore the scratch reg
		m_OutputFile << "\tmov\t" << LhsOp.Loc << "," << ScratchReg << endl;
		m_OutputFile << "\tpop\t" << ScratchReg << endl;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForIntAddOrSubInsn(InsnOp_t &LhsOp, InsnOp_t &RhsOp, 
																										string InsnName, CTreeNode *Rhs)
{
	//the add/sub instructions doesn't support having both it's operands
	//in memory so if this is the case we need to temporarily load the
	//RHS into a scratch register and use that instead
	if(LhsOp.InMemory && RhsOp.InMemory)
	{
		//get a scratch register to load the RHS into 
		string ScratchReg = GetScratchReg(Rhs);
		m_OutputFile << "\tpush\t" << ScratchReg << endl;
			
		//load the RHS into the scratch reg, perform the calculation and 
		//then restore it
		m_OutputFile << "\tmov\t" << ScratchReg << "," << RhsOp.Loc << endl;
		m_OutputFile << "\t" << InsnName << "\t" << LhsOp.Loc << "," << ScratchReg << endl;
		m_OutputFile << "\tpop\t" << ScratchReg << endl;
	}
	else
		m_OutputFile << "\t" << InsnName << "\t" << LhsOp.Loc << "," << RhsOp.Loc << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForIdivInsn(InsnOp_t &LhsOp, InsnOp_t &RhsOp)
{
	//because we need all four GPR's to perform an idiv instruction, save the
	//values in eax/ebx and load in the LHS and RHS. If the LHS is already
	//allocated to eax it saves a few instructions as we don't need to 
	//save and restore it
	if(LhsOp.Loc != "eax")
	{
		m_OutputFile << "\tpush\teax" << endl;
		m_OutputFile << "\tpush\tebx" << endl;
		m_OutputFile << "\tmov\teax," << LhsOp.Loc << endl;
		m_OutputFile << "\tmov\tebx," << RhsOp.Loc << endl;
	}
	else
	{
		if(LhsOp.Loc != "ebx")
			m_OutputFile << "\tpush\tebx" << endl;

		m_OutputFile << "\tmov\tebx," << RhsOp.Loc << endl;
	}

	//idiv uses edx for the upper 32-bits of the calculation so clear
	//it out since we're only using eax, if we don't this the CPU could
	//trap on signed overflow because of the potentially junk value in edx
	m_OutputFile << "\tpush\tedx" << endl;
	m_OutputFile << "\txor\tedx,edx" << endl;

	//perform the division, this stores the result in eax
	//so move it back to the LHS afterwards
	m_OutputFile << "\tidiv\tebx" << endl;

	if(LhsOp.Loc != "eax")
		m_OutputFile << "\tmov\t" << LhsOp.Loc << ",eax" << endl;
	
	//restore the registers
	m_OutputFile << "\tpop\tedx" << endl;

	//if we didn't save eax or ebx, there's no need to restore them
	if(LhsOp.Loc != "ebx")
		m_OutputFile << "\tpop\tebx" << endl;
	
	if(LhsOp.Loc != "eax")
		m_OutputFile << "\tpop\teax" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::TruncateFloatToInteger(CTreeNode *Lhs, CTreeNode *Rhs)
{
	InsnOp_t LhsOp;

	if(Lhs->Code != TC_ARRAY_REF)
	{
		//we use eax to store/modify the FPU control word so save it first
		//to avoid trashing any data which might be using it afterwards unless
		//the LHS is using eax
		ProcessExprOperand(Lhs, LhsOp);

		if(LhsOp.Loc != "eax")
			m_OutputFile << "\tpush\teax" << endl;
	}
	else
	{
		//this leaves 8 bytes on top of the stack which needs to be popped
		CalcArrayElementAddress(Lhs, true);
	}

	//load the value being truncated
	LoadValToFpu(Rhs);

	//create space on the stack for the existing control word
	//to save it to, making sure the operation completes before continuing
	m_OutputFile << "\tsub\tesp, 4" << endl;
	m_OutputFile << "\tmov\tdword [esp],0" << endl;
	m_OutputFile << "\tfstcw\t[esp]" << endl;
	m_OutputFile << "\tfwait" << endl;
	
	//load the original control word into eax so we can toggle the 
	//rounding mode to truncation. We then push the new control word
	//onto the stack so we can load it from the FPU 
	m_OutputFile << "\tmov\teax,[esp]" << endl;
	m_OutputFile << "\tor\tax,0C00h" << endl;
	m_OutputFile << "\tpush\teax" << endl;
	m_OutputFile << "\tfldcw\t[esp]" << endl;
	m_OutputFile << "\tfwait" << endl;
	
	//create space on the stack for the truncated result to be stored to, we
	//then truncate the value on top of the FPU stack to an integer and pop it
	//into the slot on top of the stack
	m_OutputFile << "\tsub\tesp,4" << endl;
	m_OutputFile << "\tfistp\tdword [esp]" << endl;

	//store the truncated result into the LHS
	if(Lhs->Code != TC_ARRAY_REF)
		m_OutputFile << "\tpop\t" << LhsOp.Loc << endl;
	else
	{
		//load the address of the array element we're assigning to into ecx
		//the +16 offset is for: the two control words and the saved EAX and ECX
		m_OutputFile << "\tpush\tecx" << endl;
		m_OutputFile << "\tmov\tecx,dword [esp + 16]" << endl;

		//load the truncated result into eax. Since we already have it saved we
		//dont need to bother again here. The +4 is to skip over the saved ECX
		m_OutputFile << "\tmov\teax,[esp + 4]" << endl;

		//store the truncated value
		m_OutputFile << "\tmov\tdword [ecx],eax" << endl;
		m_OutputFile << "\tpop\tecx" << endl;

		//remove the truncated result
		m_OutputFile << "\tadd\tesp,4" << endl;
	}

	//remove the modified control word
	m_OutputFile << "\tadd\tesp,4" << endl;
	
	//restore the old control word, removing it from the stack afterwards
	m_OutputFile << "\tfldcw\tword [esp]" << endl;
	m_OutputFile << "\tfwait" << endl;
	m_OutputFile << "\tadd\tesp, 4" << endl;
	
	//restore the original value of eax if needed
	if(Lhs->Code != TC_ARRAY_REF)
	{
		if(LhsOp.Loc != "eax")
			m_OutputFile << "\tpop\teax" << endl;
	}
	else
	{
		//remove the array element address and restore eax
		m_OutputFile << "\tadd\tesp,4" << endl;
		m_OutputFile << "\tpop\teax" << endl;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForArrayAssignExpr(CTreeNode *Lhs, CTreeNode *Rhs)
{
	if(Lhs->Code == TC_ARRAY_REF)
	{
		//store the address of the array element to the top of the stack
		CalcArrayElementAddress(Lhs, true);

		//check to see if we have something like arr[foo] = arr[bar]
		if(Rhs->Code == TC_ARRAY_REF)
		{
			ProcessArrToArrAssignment(Lhs, Rhs);
			return;
		}
		else
		{
			//we have arr[foo] = val
			ProcessLhsArrAssignment(Lhs, Rhs);
			return;
		}
	}
	else if(Rhs->Code == TC_ARRAY_REF)
	{
		ProcessRhsArrAssignment(Lhs, Rhs);
		return;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::CalcArrayElementAddress(CTreeNode *ArrRef, bool SaveEax)
{
	InsnOp_t IndexOp;
	ProcessExprOperand(ArrRef->GetChild(ARR_REF_INDEX), IndexOp);

	//get the array declaration being references
	CArrayRefTreeNode *Ref = (CArrayRefTreeNode*)ArrRef;
	Symbol_t *Sym = CSymbolTable::GetSingletonPtr()->LookupIdentifier(Ref->ArrName->IdName, false);
	CArrayDeclTreeNode *ArrDecl = (CArrayDeclTreeNode*)Sym->IdPtr;
	
	//we need access to the underlying var_decl for details about whether it's
	//been globally defined and it's stack frame offset
	CVarDeclTreeNode *Var = (CVarDeclTreeNode*)ArrDecl->GetChild(ARR_DECL_VAR);

	//save eax if needed
	if(SaveEax)
		m_OutputFile << "\tpush\teax" << endl;

	//if the array has been globally declared it doesn't exist within the stack
	//frame so we need to compute it's address differently
	if(!Var->GlobalVar)
	{
		int Offset = -Var->StackFrameAddr;

		//load the index of the element we want
		m_OutputFile << "\tmov\teax," << IndexOp.Loc << endl;

		//because the size of each element is 4 bytes, and the stack grows
		//downwards we need to multiply the index value by -4 to get an offset
		//relative to ebp for reading from
		m_OutputFile << "\timul\teax,-4" << endl;

		//adjust for the offset of the first element 
		m_OutputFile << "\tadd\teax," << Offset << endl;

		//add the computed offset to ebp which gives us the address to read from
		m_OutputFile << "\tadd\teax,ebp" << endl;
	}
	else
	{
		//load the index required into eax and multiply it by 4, this 
		//gives the byte offset from the start of the array of the value
		//we're after
		m_OutputFile << "\tmov\teax," << IndexOp.Loc << endl;
		m_OutputFile << "\timul\teax,4" << endl;

		//add the offset to the address of the array
		m_OutputFile << "\tadd\teax," << Var->VarId->IdName << endl;

		m_OutputFile << "nop" << endl;
	}

	//push the address to the stack
	m_OutputFile << "\tpush\teax" << endl;

	if(SaveEax)
		m_OutputFile << "\tmov\teax,[esp + 4]" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::ProcessArrToArrAssignment(CTreeNode *Lhs, CTreeNode *Rhs)
{
	//check for:
	//    float fp_arr[...]
	//    integer int_arr[...]
	//    int_arr[foo] = fp_arr[bar]
	//where we need to truncate the value in fp_arr[bar]
	if(CUtilFuncs::ExprRefersToFloatingPointType(Rhs)
		 && !CUtilFuncs::ExprRefersToFloatingPointType(Lhs))
	{
		TruncateFloatToInteger(Lhs, Rhs);
		return;
	}

	/*
	 After the call to CalcArrayElementAddress below, the stack will
	 look like this:
		| saved eax   | [esp + 20]
		| lhs address | [esp + 16]
		| saved eax   | [esp + 12]
		| rhs address | [esp + 8]
		| saved ecx   | [esp + 4]
		| saved edx   | [esp]
		*/
	CalcArrayElementAddress(Rhs, true);

	//save ECX and EDX
	m_OutputFile << "\tpush\tecx" << endl;
	m_OutputFile << "\tpush\tedx" << endl;

	//load the address where the RHS value is into ecx
	m_OutputFile << "\tmov\tecx,[esp+8]" << endl;

	//load the actual value of the RHS into edx
	m_OutputFile << "\tmov\tedx,[ecx]" << endl;

	//load the address of the LHS into ecx
	m_OutputFile << "\tmov\tecx,[esp+16]" << endl;

	//copy the value of the RHS into the LHS
	m_OutputFile << "\tmov\t[ecx],edx" << endl;

	//restore ecx and edx
	m_OutputFile << "\tpop\tedx" << endl;
	m_OutputFile << "\tpop\tecx" << endl;

	//theres now 12 bytes of junk on the stack we need to remove to get at the
	//saved value of eax from the first call to CalcArrayElementAddress
	m_OutputFile << "\tadd\tesp,12" << endl;
	m_OutputFile << "\tpop\teax" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::ProcessLhsArrAssignment(CTreeNode *Lhs, CTreeNode *Rhs)
{
	if(CUtilFuncs::ExprRefersToFloatingPointType(Rhs)
		 && !CUtilFuncs::ExprRefersToFloatingPointType(Lhs))
	{
		TruncateFloatToInteger(Lhs, Rhs);
		return;
	}

	InsnOp_t RhsOp;
	ProcessExprOperand(Rhs, RhsOp);
			
	//move the address of the array element we're assigning to 
	//into a temporary scratch reg
	string ScratchReg = GetScratchReg(CUtilFuncs::GetDeclFromNode(Rhs));
	m_OutputFile << "\tpush\t" << ScratchReg << endl;
	m_OutputFile << "\tmov\t" << ScratchReg << ",dword [esp + 4]" << endl;

	//store the value to the address in the scratch reg
	if(RhsOp.InMemory)
	{
		m_OutputFile << "\tpush\t" << RhsOp.Loc << endl;
		m_OutputFile << "\tpop\tdword [" << ScratchReg << "]" << endl;
	}
	else
		m_OutputFile << "\tmov\tdword [" << ScratchReg << "]," << RhsOp.Loc << endl;

	//restore scratch reg
	m_OutputFile << "\tpop\t" << ScratchReg << endl;

	//CalcArrayElementAddress has left 8 bytes on the stack which needs cleaning
	//up, the first 4 are the address of the LHS and the second is the saved
	//eax so restore it now
	m_OutputFile << "\tadd\tesp, 4" << endl;
	m_OutputFile << "\tpop\teax" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::ProcessRhsArrAssignment(CTreeNode *Lhs, CTreeNode *Rhs)
{
	//validate our inputs
	SanityCheck(Lhs && Lhs->Code != TC_ARRAY_REF);
	SanityCheck(Rhs && Rhs->Code == TC_ARRAY_REF);

	//check for:
	//    float arr[...]
	//    integer foo = arr[0]
	//where we need to truncate the value in arr[0]
	if(CUtilFuncs::ExprRefersToFloatingPointType(Rhs)
		 && !CUtilFuncs::ExprRefersToFloatingPointType(Lhs))
	{
		TruncateFloatToInteger(Lhs, Rhs);
		return;
	}

	//get the location for the LHS of the assignment
	InsnOp_t LhsOp;
	ProcessExprOperand(Lhs, LhsOp);

	//stack offset to load array element address from
	string StackOffset = "+4";

	//stick the address of the array element we're assigning into the 
	//LHS on top of the stack
	CalcArrayElementAddress(Rhs, LhsOp.Loc == "eax" ? false : true);

	//save ecx and edx if needed
	if(LhsOp.Loc != "ecx")
	{
		m_OutputFile << "\tpush\tecx" << endl;
		StackOffset = "+8";
	}

	if(LhsOp.Loc != "edx")
		m_OutputFile << "\tpush\tedx" << endl;
	else
		StackOffset = "+4";

	//load the address of the array element into ecx, then load the
	//value being assigned to the LHS into edx
	m_OutputFile << "\tmov\tecx,[esp" + StackOffset + "]" << endl;
	m_OutputFile << "\tmov\tedx,[ecx]" << endl;

	//move the RHS into the LHS
	m_OutputFile << "\tmov\t" << LhsOp.Loc << ",edx" << endl;

	//restore ecd and edx
	if(LhsOp.Loc != "edx")
		m_OutputFile << "\tpop\tedx" << endl;

	if(LhsOp.Loc != "ecx")
		m_OutputFile << "\tpop\tecx" << endl;

	//CalcArrayElementAddress has left 8 bytes on the stack which needs cleaning
	//up, the first 4 are the address of the LHS and the second is the saved
	//eax so restore it now
	m_OutputFile << "\tadd\tesp, 4" << endl;

	if(LhsOp.Loc != "eax")
		m_OutputFile << "\tpop\teax" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::GenerateCodeForFPUComparison(CTreeNode *Expr, 
																									string &JumpInsn)
{
	if(Expr->Code == TC_BOOLEAN_EXPR)
	{
		CBooleanExprTreeNode *BlnExpr = (CBooleanExprTreeNode*)Expr;

		//extract both operands
		CTreeNode *CondExprLhs = Expr->GetChild(BLN_EXPR_LHS);
		CTreeNode *CondExprRhs = Expr->GetChild(BLN_EXPR_RHS);

		//load them both the FPU registers
		LoadValToFpu(CondExprLhs);
		LoadValToFpu(CondExprRhs);

		//determine which version of the jump instruction we need to use
		JumpInsn = GetJumpInsnForBooleanOp(BlnExpr->Operator);

		//do the comparison, popping both operands afterwards
		m_OutputFile << "\tfcompp" << endl;
	}
	else
	{
		//load the operand into the top two FP registers
		LoadValToFpu(Expr);
		JumpInsn = "jnz";

		//examine the contents of st(0)
		m_OutputFile << "\tfxam" << endl;
	}

	//load the FPU status word into ax, then into the flags register
	m_OutputFile << "\tpush\teax" << endl;
	m_OutputFile << "\tfstsw\tax" << endl;
	m_OutputFile << "\tfwait" << endl;
	m_OutputFile << "\tsahf" << endl;
	m_OutputFile << "\tpop\teax" << endl;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCodeGenerator::LoadValToFpu(CTreeNode *Val)
{
	if(Val->Code != TC_ARRAY_REF)
	{
		//process the value to see where it's stored
		InsnOp_t ValOp;
		ProcessExprOperand(Val, ValOp);

		//if it's in memory we can load it directly to the FPU
		if(ValOp.InMemory)
		{
			m_OutputFile << "\tfld\t" << ValOp.Loc << endl;
			return;
		}

		//push the val to the stack and load it from there
		m_OutputFile << "\tpush\t" << ValOp.Loc << endl;
		m_OutputFile << "\tfld\tdword [esp]" << endl;
		m_OutputFile << "\tadd\tesp,4" << endl;
	}
	else
	{
		//calculate the array element to use, after this call we will 
		//have 8 bytes on top of the stack (saved EAX and address) that
		//will need to be removed before we exit this function
		CalcArrayElementAddress(Val, true);

		//load the address into eax, since we have already saved it through the
		//call to CalcArrayElementAddress we dont need to do so again
		m_OutputFile << "\tmov\teax,[esp]" << endl;

		//load the value to the stack
		m_OutputFile << "\tfld\tdword [eax]" << endl;

		//cleanup after the call to CalcArrayElementAddress
		m_OutputFile << "\tadd\tesp,4" << endl;
		m_OutputFile << "\tpop\teax" << endl;
	}
}
//-------------------------------------------------------------
