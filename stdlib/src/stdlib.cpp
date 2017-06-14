//------------------------------------------------------------------------------------------
// File: stdlib.cpp
// Desc: Implementation of standard library functions.
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include <cstdio>
#include "stdlib.h"

//-------------------------------------------------------------
STDLIB_API void print_string(char *str)
{
	puts(str); 
}
//-------------------------------------------------------------

//-------------------------------------------------------------
STDLIB_API void print_integer(int Val)
{
	printf("%d\n", Val);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
STDLIB_API void print_boolean(int Val)
{
	printf("%s\n", Val ? "true" : "false");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
STDLIB_API void print_float(float Val)
{
	printf("%f\n", Val);
}
//-------------------------------------------------------------