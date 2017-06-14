//------------------------------------------------------------------------------------------
// File: CInternalErrorException.h
// Desc: Used to indicate an internal consistency check failed
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifndef __CINTERNALERROREXCEPTION__
#define __CINTERNALERROREXCEPTION__

#include <string>
using std::string;

/**
 * Custom exception class to indicate something has gone wrong
 * which we can't recover from or an assertion has failed. No exceptions
 * are raised directly in the code in the form of "throw CInternalErrorException",
 * instead the macros InternalError and SanityCheck are used instead which
 * allows the transparent inclusion of the exact location the fault
 * originated at.
 */
class CInternalErrorException
{
public:
	CInternalErrorException(void) {}
	CInternalErrorException(int Line, const char *Func, const char *Msg)
		: m_Line(Line), m_Func(Func), m_Msg(Msg) {}
	~CInternalErrorException(void){}

	/**
	 * Accessor for the line number attribute.
	 * @return The line number the exception was raised on.
	 */
	int GetLine(void) { return m_Line; }

	/** 
	 * Accessor for the raising function.
	 * @return The name of the function which threw this exception.
	 */
	string GetFunc(void) { return m_Func; }

	/**
	 * Accessor for the message attribute.
	 * @return The accompanying message providing further information 
	 *         about the failure.
	 */
	string GetMsg(void) { return m_Msg; }

private:

	int m_Line;
	string m_Func;
	string m_Msg;
};

//handy macros to indicate an internal error
#define InternalError(msg) \
	throw CInternalErrorException(__LINE__, __FUNCTION__, (msg))

//similiar to an assert in that the condition shouldn't fail
#define SanityCheck(cond) \
	if(!(cond)) \
		throw CInternalErrorException(__LINE__, __FUNCTION__, "sanity check failed")

#endif
