//------------------------------------------------------------------------------------------
// File: CCodeGenerator.h
// Desc: Generate x86 assembly code from a series of MIR statements
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CCODEGENERATOR_H__
#define __CCODEGENERATOR_H__

//forward decls
class CTreeNode;

//pretty much every instruction we generate doesn't allow
//both of their operands to be in memory so we use this structure
//to record either their register name, or their memory location 
//and also whether they are to be loaded from memory or not
struct InsnOp_t
{
	string Loc;
	bool InMemory;
};

class CCodeGenerator
{
public:
	CCodeGenerator(void) : m_CurrStmtNo(0) {};
	~CCodeGenerator(void) {};

	//take a MIR tree for a function and generate the equilavent assembly 
	//machine instructions, ouputting them to the specified file
	void AssembleMIRTree(const string &OutputFile, CTreeNode *MirTree);

private:

	//output the data sections .data and .bss
	void OutputDataSections(void);

	//look at the statement Stmt and delegate the code generation to 
	//the necessary function that can handle it
	void GenerateCodeForStatement(CTreeNode *Stmt);

	//specialist functions to handle the code gen for a specific stmt type
	void GenerateCodeForAssignExpr(CTreeNode *AssignExpr);
	void GenerateCodeForArithExpr(CTreeNode *Expr);
	void GenerateCodeForIfStmt(CTreeNode *Stmt);
	void GenerateCodeForCallExpr(CTreeNode *Expr);
	void GenerateCodeForNegExpr(CTreeNode *Expr);
	void GenerateCodeForReturnStmt(CTreeNode *Stmt);

	//subroutines of GenerateCodeForArithExpr
	void GenerateCodeForImulInsn(InsnOp_t &LhsOp, InsnOp_t &RhsOp, CTreeNode *Lhs);
	void GenerateCodeForIntAddOrSubInsn(InsnOp_t &LhsOp, InsnOp_t &RhsOp, 
																			string InsnName, CTreeNode *Rhs);
	void GenerateCodeForIdivInsn(InsnOp_t &LhsOp, InsnOp_t &RhsOp);

	//subroutine of GenerateCodeForAssignExpr
	void GenerateCodeForFPUAssignExpr(CTreeNode *Lhs, CTreeNode *Rhs);

	//process the LHS/RHS of an expression "a op b", determining the location 
	//of the value being assigned to and whether it's in memory or not
	void ProcessExprOperand(CTreeNode *Expr, InsnOp_t &Op);

	//subroutine of the above function to handle var_decls since they require
	//more processing than other types of node
	void ProcessVarDeclExprOperand(CTreeNode *Decl, InsnOp_t &Op);

	//get the location to load from of a var_decl
	void GetDeclLocation(CTreeNode *Decl, InsnOp_t &Op);

	//return the name of the instruction to use for a given arithmetic operator
	string GetArithInsnName(TokenType Op, bool FpuCalc);

	//returns the name of a jxx jump instruction as determined by the operator used
	string GetJumpInsnForBooleanOp(TokenType Op);

	//output a function's prologue/epilogue code
	void OutputFunctionPrologue(int StackFrameSize);
	void OutputFunctionEpilogue(void);

	//return the name of an available scratch register
	string GetScratchReg(CTreeNode *Decl);

	//get a memory address in the stack frame for a value
	string CalcStackFrameAddress(bool ParmAddr, int Slot);

	//subroutine of GenerateCodeForAssignExpr to handle the truncation
	//of a floating point value to an integral one
	void TruncateFloatToInteger(CTreeNode *Lhs, CTreeNode *Rhs);

	//store the value of an array element to the top of the stack
	void CalcArrayElementAddress(CTreeNode *ArrayRef, bool SaveEax);

	//handle to the file we're outputting to
	ofstream m_OutputFile;

	//the current stmt number we're processing. Used for detecting
	//when to load a spilled value from the stack frame rather than
	//it's allocated register
	int m_CurrStmtNo;

  //TODO: document
	void ProcessArrToArrAssignment(CTreeNode *Lhs, CTreeNode *Rhs);
	void ProcessLhsArrAssignment(CTreeNode *Lhs, CTreeNode *Rhs);
	void ProcessRhsArrAssignment(CTreeNode *Lhs, CTreeNode *Rhs);
	void GenerateCodeForArrayAssignExpr(CTreeNode *Lhs, CTreeNode *Rhs);
	void GenerateCodeForFPUComparison(CTreeNode *CondExpr, string &JumpInsn);
	void LoadValToFpu(CTreeNode *Val);
};

#endif