//------------------------------------------------------------------------------------------
// File: CCompilerDriver.h
// Desc: Class to handle the different phases of compilation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CCOMPILERDRIVER_H__
#define __CCOMPILERDRIVER_H__

//forward decls
class CMIRTreeRewriter;
class CTreeNode;

/**
 * The main workhorse of the compiler. This is where each pass is executed
 * in turn to ensure correct results although the later passes are only 
 * run if parsing completes successfully. Once all the passes have completed
 * we run the assembler and linker to produce the final executable.
 */
class CCompilerDriver
{
public:
	CCompilerDriver(void);
	~CCompilerDriver(void) {};

	/**
	 * Compile a single input file.
	 * @param ArgCount Number of command line arguments.
	 * @param Arguments An array of C-style strings for the arguments.
	 */
	void CompileFile(int ArgCount, char *Arguments[]);

private:

	/**
	 * Parse the command line arguments, extracting the relevant values
	 * into the member variables of this class for the relevent dump
	 * file options.
	 * @param ArgCount Number of command line arguments.
	 * @param Arguments The values of each command line parameter.
	 */
	void DecodeCommandLineArgs(int ArgCount, char *Arguments[]);

	/** Cleanup any dump files from a previous run. */
	void DeleteOldDumpFiles(void);

	/**
	 * First pass: parsing and semantic analysis.
	 * @return Parse tree representation of the source code.
	 */
	CTreeNode* RunParser(void);

	/**
	 * Second pass: rewrite the parse tree into MIR form.
	 * @param ParseTree The parse tree contructed during the first pass.
	 * @return A MIR representation of the input parse tree.
	 */
	CTreeNode *RewriteIntoMIRForm(CTreeNode *ParseTree);

	/**
	 * Third pass: construction of CFG for data flow analysis.
	 * @param MirTree The MIR representation of the program.
	 */
	void ConstructControlFlowGraph(CTreeNode **MirTree);

	/**
	 * Fourth pass: run the data flow analyser on each function 
	 * to compute liveness informaion.
	 * @param MirTree The MIR representation of the program.
	 */
	void RunDataFlowAnalyser(CTreeNode *MirTree);

	/**
	 * Fifth pass: register allocation.
	 * @param MirTree The MIR representation of the program.
	 */
	void RunRegisterAllocator(CTreeNode *MirTree);

	/**
	 * Sixth and final pass: code generation.
	 * @param MirTree The MIR representation of the program.
	 */
	void RunCodeGenerator(CTreeNode *MirTree);

	/** Run the assembler and linker to produce the final executable. */
	void RunAssemblerAndLinker(void);

	/** Allocate the global singleton objects. */
	void ConstructSingletons(void);

	/** Release the global singleton objects. */
	void DeleteSingletons(void);

	/**
	 * Execute a program (either the assembler or linker) and
	 * check it's exit code. Returns true if the program exited 
	 * with a code of 0 otherwise false.
	 * @param Prog The name of the program to execute.
	 * @return True if the program executed successfully, otherwise false.
	 */
	bool ExecuteProgram(const string &Prog);

	/** True to dump the parse tree representation of the program. */
	bool m_DumpParseTree;

	/** True to dump the MIR representation of the program. */
	bool m_DumpMirTree;

	/** True to dump the CFG for each function during contruction. */
	bool m_DumpCfg;

	/** True to dump the information computed during DFA. */
	bool m_DumpDfInfo;

	/** 
	 * True to dump the computed live intervals for each function 
	 * in the program. 
	 */
	bool m_DumpLiveIntervals;

	/** True to dump the registers/stack slots for each value in the program. */
	bool m_DumpFinalRegAllocs;

	/** True if we've been asked to save the generated assembly file. */
	bool m_SaveAsmFile;

	/** Input file we're compiling. */
	string m_InputFile;

	/** Assembly file we've generated. */
	string m_AsmFile;
};

#endif
