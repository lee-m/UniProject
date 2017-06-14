//------------------------------------------------------------------------------------------
// File: stdlib.h
// Desc: Declaration of standard library functions.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#ifdef STDLIB_EXPORTS
#define STDLIB_API __declspec(dllexport)
#else
#define STDLIB_API __declspec(dllimport)
#endif

extern "C"
{
	STDLIB_API void print_string(char *Str);
	STDLIB_API void print_integer(int Val);
	STDLIB_API void print_boolean(int Val);
	STDLIB_API void print_float(float Val);
}
