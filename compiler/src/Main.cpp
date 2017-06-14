//------------------------------------------------------------------------------------------
// File: Main.cpp
// Desc: Entry point for the application
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CCompilerDriver.h"

int main(int argc, char *argv[])
{
	CCompilerDriver Driver;
	Driver.CompileFile(argc, argv);

	system("pause");

#ifdef _DEBUG
	m_dumpMemoryReport("memreport.log", true);
#endif

	return 0;
}

