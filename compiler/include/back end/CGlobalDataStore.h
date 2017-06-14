//------------------------------------------------------------------------------------------
// File: CGlobalDataStore.h
// Desc: Store global variable decls and string literals which are output to the 
//       .data (str literals) and .bss (global vars) sections
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CGLOBALDATASTORE_H__
#define __CGLOBALDATASTORE_H__

#include "ISingleton.h"
#include "CLiteralRefTreeNode.h"

//forward decls
class CTreeNode;
class CVarDeclTreeNode;
class CArrayDeclTreeNode;

/** The two different types of literal we deal with. */
enum LiteralType
{
	LT_STRING = 0,
	LT_FLOATING_POINT
};

/**
 * During code generatin we'd like to know what information needs to be
 * emitted in the global data section for objects such as globally defined
 * variables and string literals which is that this class handles. During 
 * parsing any globally declared variables will be added whereas string 
 * literals aren't added until MIR lowering time when their useage is
 * replaced with references.
 * <br><br>
 * The information added during parsing and MIR lowering is output
 * at the beginning of code generation. Rather than provide methods
 * to retrieve the info, we allow the provision of a file handle which
 * we then use to output the .data and .bss sections ourselves.
 */
class CGlobalDataStore : public ISingleton<CGlobalDataStore>
{
public:
	CGlobalDataStore(void) : m_NextStrRef(0), m_NextFpRef(0) {};
	~CGlobalDataStore(void);

	/**
	 * Add a new literal to the store.
	 * @param Str Value of the string literal.
	 * @param RefName Reference name used to refer to this literal.
	 * @param LitType What type of literal is being added.
	 */
	void AddLiteral(const string &Str, const string &RefName, LiteralType LitType);

	/**
	 * Like AddStringLiteral but for global variables instead.
	 * @param VarDecl Globally defined variable declaration.
	 */
	void AddGlobalVar(CTreeNode *VarDecl);

	/**
	 * Mark the use of a standard library function.
	 * @param Name Name of the builtin function to mark used.
	 */
	void MarkBuiltinFunctionUsed(const string &Name);

	/**
	 * Determine if the literal Lit already exists to perform a simple
	 * form of string pooling by returning a reference to a previously
	 * added literal if the new literal matches any.
	 * @param Lit String literal value to check against those already added.
	 * @return Reference to a previously added string literal if a match
	 *         is already present or a new reference name if there isn't one.
	 */
	string GetStrLitRefNo(const string &Lit);

	/**
	 * Get the next reference number for referring to a floating point
	 * literal.
	 * @param Lit The value of the literal.
	 * @return A reference name to the literal.
	 */
	string GetFpLitRefNo(const string &Lit);

	/** 
	 * Dump all literals to the .data section of the assembly file.
	 * @param FileHandle The file we're outputting assembly code to.
	 */
	void OutputLiterals(ofstream &FileHandle);

	/**
	 * Dump global var decls to the .bss section of the assembly file.
	 * @param FileHandle The file we're outputting assembly code to.
	 */
	void OutputGlobalVariables(ofstream &FileHandle);

	/** 
	 * Output any "extern" declarations for std library functions.
	 * @param FileHandle The file we're outputting assembly code to.
	 */
	void OutputExternDeclarations(ofstream &FileHandle);

private:

	/**
	 * Subroutine of OutputGlobalVariables to handle the outputting 
	 * of a variable declaration.
	 * @param VarDecl Variable declaration to output.
	 * @param FileHandle File handle to the assembly output file.
	 */
	void OutputVarDecl(CVarDeclTreeNode *VarDecl, ofstream &FileHandle);

	/**
	 * Like OutputVarDecl but handles array declarations instead.
	 * @param ArrDecl Array declaration to output.
	 * @param FileHandle Handle to the assembly output file.
	 */
	void OutputArrayDecl(CArrayDeclTreeNode *ArrDecl, ofstream &FileHandle);

	/**
	 * All string literals we're holding onto, the key is the reference
	 * number used in the IR and the value is the literal itself.
	 */
	map<string, string> m_StrLiterals;

	/** 
	 * List of floating point literals in the program. 
	 * @see m_StrLiterals
	 */
	map<string, string> m_FpLiterals;

	/** The next string literal reference id number. */
	int m_NextStrRef;

	/** The next floating point literal reference id number. */
	int m_NextFpRef;

	/** Any globally defined variables. */
	list<CTreeNode*> m_VarDecls;

	/** Names of builtin functions used by the program. */
	vector<string> m_BuiltinFuncs;
};

#endif