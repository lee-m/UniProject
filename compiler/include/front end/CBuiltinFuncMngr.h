//------------------------------------------------------------------------------------------
// File: CBuiltinFuncMngr.h
// Desc: Manager for functions which are part of the std library
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CBUILTINFUNCMNGR__
#define __CBUILTINFUNCMNGR__

#include "ISingleton.h"

//forward decls
class CFunctionDeclTreeNode;

/**
 * Class to handle the management of the builtin functions which are part
 * of the standard library. In order to allow other parts of the front-end
 * to work with them without needing to special case their usage we generate
 * function declarations to add to the symbol table, destroying them at
 * the end of the program.
 */
class CBuiltinFuncMngr : public ISingleton<CBuiltinFuncMngr>
{
public:
	CBuiltinFuncMngr(void) {} ;
	~CBuiltinFuncMngr(void);

	/**
	 * Create the builtin function decls for use later on if required.
	 * @param InMir True if the builtins are being created for use in the MIR
	 *        representation where all function names are prepended with a 
	 *        leading underscore.
	 */
	void InitialiseBuiltins(bool InMir);

	/** Destroy any builtin functions created by InitialiseBuiltins. */
	void DestroyBuiltins(void);

	/**
	 * Check whether an identifer refers to a built in function.
	 * @param FuncName Name of the function to check.
	 * @return True if FuncName refers to a built in function otherwise false.
	 */
	bool IsBuiltinFunction(const string &FuncName);

private:

	/**
	 * List of builtin functions keyed by their name. The value
	 * part of the map is the parameter type the function expects
	 * to enable type checking to take place.
	 */
	vector<string> m_BuiltinFuncs;

	/**
	 * Function decls for each builtin function, keyed
	 * by their name.
	 */
	vector<CTreeNode*> m_BuiltinFnDecls;
};

#endif
