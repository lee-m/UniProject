//------------------------------------------------------------------------------------------
// File: Main.cpp
// Desc: Entry point for the application
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CLexicalScanner.h"
#include "CSymbolTable.h"
#include "CParser.h"

int main(int argc, char *argv[])
{
	/*if(argc < 2)
	{
		cout << "no input file specified" << endl;
		system("pause");
		return 1;
	}*/

	/*
	CSymbolTable symtab;

	symtab.AddSymbol("test", 1);
	symtab.PushNewSymbolScope();
	
	if(symtab.LookupIdentifier("test"))
		cout << "found identifier test" << endl;

	symtab.PushNewSymbolScope();
	symtab.AddSymbol("test2", 2);
	symtab.PopSymbolScope();
	

	if(symtab.LookupIdentifier("test2"))
		cout << "found identifier test2" << endl;
*/
	signed int a = INT_MIN;
	a -= 32;

	CParser parser;
	parser.ParseSourceFile("prog.txt");

	system("pause");
/*
  CLexicalScanner Scanner;
	Scanner.SetInputFile(argv[1]);

	cout << TokenTypeToString(Scanner.PeekNextToken().Type) << endl;
	cout << TokenTypeToString(Scanner.PeekNextToken().Type) << endl;
	cout << TokenTypeToString(Scanner.PeekNextToken().Type) << endl;

	cout << endl;

	cout << TokenTypeToString(Scanner.GetNextToken().Type) << endl;
	cout << TokenTypeToString(Scanner.GetNextToken().Type) << endl;
	cout << TokenTypeToString(Scanner.GetNextToken().Type) << endl;
*/
	m_dumpMemoryReport("memreport.log", true);

	return 0;
}

