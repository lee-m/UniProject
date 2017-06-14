//------------------------------------------------------------------------------------------
// File: CCompilerDriver.cpp
// Desc: Class to handle the different phases of compilation
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "StdHeader.h"
#include "CTreeNode.h"
#include "CCompilerDriver.h"
#include "CSymbolTable.h"
#include "CParser.h"
#include "CControlFlowGraph.h"
#include "CFunctionDeclTreeNode.h"
#include "CDataFlowAnalyser.h"
#include "CLinearScanAllocator.h"
#include "CCodeGenerator.h"
#include "CGlobalDataStore.h"
#include "CBuiltinFuncMngr.h"

#include <windows.h>

//-------------------------------------------------------------
CCompilerDriver::CCompilerDriver(void)
{
	m_DumpParseTree = false;
	m_DumpMirTree = false;
	m_DumpCfg = false;
	m_DumpDfInfo = false;
	m_DumpLiveIntervals = false;
	m_DumpFinalRegAllocs = false;

	m_InputFile = "";
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::CompileFile(int ArgCount, char *Arguments[])
{
	//we must have at least one argument (the name of the file being compiled)
	//in order to do anything, anything else it's an error
	if(ArgCount < 2)
	{
		cout << "usage: <input-file>         [REQUIRED] input file to compile" << endl;
		cout << "       --dump-parse-tree    [OPTIONAL] dump the parse tree" << endl;
		cout << "       --dump-mir-tree      [OPTIONAL] dump the MIR tree" << endl;
		cout << "       --dump-cfg           [OPTIONAL] dump the CFG for each function" << endl;
		cout << "       --dump-df-info       [OPTIONAL] dump the df info for each function" << endl;
		cout << "       --dump-live-int      [OPTIONAL] dump live intervals from RA" << endl;
		cout << "       --dump-reg-allocs    [OPTIONAL] dump final register allocations" << endl;
		cout << "       --save-assembly      [OPTIONAL] save the generated assembly code" << endl;
		return;
	}

	DecodeCommandLineArgs(ArgCount, Arguments);
	DeleteOldDumpFiles();

	//make sure we've got an input file to parse
	if(m_InputFile.length() == 0)
	{
		cout << "no input file specified" << endl;
		return;
	}

	try
	{
		ConstructSingletons();

		//pass 1: semantic and syntactic analysis
		CBuiltinFuncMngr::GetSingletonPtr()->InitialiseBuiltins(false);
		CTreeNode *ParseTree = RunParser();

		if(!ParseTree)
		{
			DeleteSingletons();
			return;
		}

		//up to this point we've used the symbol for semantic checking 
		//but now as we enter the middle/back-end it takes on a different
		//purpose, we no longer care about scopes and instead use the
		//information contained within it during the various passes. During 
		//MIR lowering we rebuild the symbol table for variables only, unlike
		//during parsing where we also store functions. FIXME: dont do this
		CSymbolTable::GetSingletonPtr()->Reset();
		CBuiltinFuncMngr::GetSingletonPtr()->DestroyBuiltins();
		CBuiltinFuncMngr::GetSingletonPtr()->InitialiseBuiltins(true);

		//pass 2: convert the parse tree to MIR form, after this stage
		//we're done with the parse tree so destroy it
		CTreeNode *MirTree = RewriteIntoMIRForm(ParseTree);
		ParseTree->DestroyNode();
		SanityCheck(MirTree);

		//pass 3: build the CFG
		ConstructControlFlowGraph(&MirTree);

		//pass 4: perform data flow analysis for each function
		RunDataFlowAnalyser(MirTree);

		//pass 5: perform local register allocation for each function,
		//any global variables will be stored in memory 
		RunRegisterAllocator(MirTree);

		//pass 6: code generation
		RunCodeGenerator(MirTree);
		MirTree->DestroyNode();

		//run the assembler and linker to generate the final executable
		RunAssemblerAndLinker();

		//we're all done with the singleton objects so destroy them
		DeleteSingletons();
	}
	catch(CInternalErrorException &Err)
	{
		cout << "an internal compiler error has occurred, details: " << endl;
		cout << "  function: " << Err.GetFunc() << endl;
		cout << "  line: " << Err.GetLine() << endl;
		cout << "  message: " << Err.GetMsg() << endl;
		return;
	}
	catch(...)
	{
		cout << "an unknown exception has been thrown: " << endl;
		DeleteSingletons();
		return;
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::DecodeCommandLineArgs(int ArgCount, char *Arguments[])
{
	//first arg is the .exe name
	for(int i = 1; i < ArgCount; i++)
	{
		//if the value doesn't being with a "-", it's the input file
		if(Arguments[i][0] != '-')
		{
			m_InputFile = Arguments[i];
			continue;
		}

		//we've got one of the other options, decide which
		if(!strcmp("--dump-parse-tree", Arguments[i]))
		{
			m_DumpParseTree = true;
			continue;
		}

		if(!strcmp("--dump-mir-tree", Arguments[i]))
		{
			m_DumpMirTree = true;
			continue;
		}

		if(!strcmp("--dump-cfg", Arguments[i]))
		{
			m_DumpCfg = true;
			continue;
		}

		if(!strcmp("--dump-df-info", Arguments[i]))
		{
			m_DumpDfInfo = true;
			continue;
		}

		if(!strcmp("--dump-live-int", Arguments[i]))
		{
			m_DumpLiveIntervals = true;
			continue;
		}

		if(!strcmp("--dump-reg-allocs", Arguments[i]))
		{
			m_DumpFinalRegAllocs = true;
			continue;
		}

		if(!strcmp("--save-assembly", Arguments[i]))
		{
			m_SaveAsmFile = true;
			continue;
		}

		cout << "error: unrecognised command line option '" << Arguments[i] << "'" << endl;
		system("pause");
		exit(1);
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::DeleteOldDumpFiles(void)
{
	remove("ParseTree.xml");
	remove("MirTreeDump.txt");
	remove("ControlFlowGraph.txt");
	remove("DataFlow.txt");
	remove("LiveIntervals.txt");
	remove("RegisterAllocations.txt");
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CCompilerDriver::RunParser(void)
{
	CParser Parser;
	CTreeNode *ParseTree = Parser.ParseSourceFile(m_InputFile);

	//see if parsing completed without errors
	if(!ParseTree)
		return NULL;

	//dump the parse tree if required
	if(m_DumpParseTree)
	{
		//for some reason allocating ParseTreeDoc on the stack triggers an exception
		//in the mem mngr, might be a bug inside TinyXML???
		TiXmlDocument *ParseTreeDoc = new TiXmlDocument();

		//create the root node, then pass it to the parse tree to dump to
		ParseTreeDoc->InsertEndChild(TiXmlElement("parse_tree"));
		ParseTree->DumpAsParseTreeNode(ParseTreeDoc->RootElement());

		//save the XML document representing the tree and free the doc object
		ParseTreeDoc->SaveFile("ParseTree.xml");
		delete ParseTreeDoc;
		ParseTreeDoc = NULL;
	}

	return ParseTree;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
CTreeNode* CCompilerDriver::RewriteIntoMIRForm(CTreeNode *ParseTree)
{
	//create our root node for the tree
	CTreeNode *RootMirNode = CTreeNodeFactory::GetSingletonPtr()->BuildRootNode();

	//process each global statement in the tree, these will themselves
	//recursively process each sub-tree contained within them
	for(unsigned int i = 0; i < ParseTree->GetNumChildren(); i++)
	{
		CTreeNode *Temp = ParseTree->GetChild(i)->RewriteIntoMIRForm(RootMirNode);
		RootMirNode->AddChild(Temp);
	}

	//dump the tree if required
	if(m_DumpMirTree)
	{
		ofstream MirTreeOut("MirTreeDump.txt");

		if(!MirTreeOut.is_open())
			InternalError("unable to open MIR dump tree file");

    RootMirNode->DumpAsMIRTreeNode(MirTreeOut);
		MirTreeOut.close();
	}

	return RootMirNode;
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::ConstructControlFlowGraph(CTreeNode **MirTree)
{
	CControlFlowGraph CfgBuilder;

	//find individual function decls within the tree and build the cfg for them. 
	//Note: we use GetChildPtr not GetChild since we need to modify the tree nodes
	//when building the CFG for them
	for(unsigned int i = 0; i < (*MirTree)->GetNumChildren(); i++)
	{
		if((*MirTree)->GetChild(i)->Code == TC_FUNCTIONDECL)
		{
			//grab a pointer to the decl so we can build the CFG for it
			CFunctionDeclTreeNode **FnDecl;
			FnDecl = (CFunctionDeclTreeNode**)(*MirTree)->GetChildPtr(i);

			CfgBuilder.BuildControlFlowGraph(FnDecl, m_DumpCfg);
		}
	}
}
//-------------------------------------------------------------
void CCompilerDriver::RunDataFlowAnalyser(CTreeNode *MirTree)
{
	CDataFlowAnalyser Df;

	for(unsigned int i = 0; i < MirTree->GetNumChildren(); i++)
	{
		if(MirTree->GetChild(i)->Code == TC_FUNCTIONDECL)
		{
			//get a pointer to the function
			CFunctionDeclTreeNode **FnDecl;
			FnDecl =(CFunctionDeclTreeNode**)MirTree->GetChildPtr(i);

			//solve the data flow problems for it
			Df.AnalyseFunction(FnDecl, m_DumpDfInfo);
		}
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::RunRegisterAllocator(CTreeNode *MirTree)
{
	CLinearScanAllocator Allocator;
	
	for(unsigned int i = 0; i < MirTree->GetNumChildren(); i++)
	{
		if(MirTree->GetChild(i)->Code == TC_FUNCTIONDECL)
		{
			//get a pointer to the function
			CFunctionDeclTreeNode *FnDecl;
			FnDecl =(CFunctionDeclTreeNode*)MirTree->GetChild(i);

			//run local register allocation
			Allocator.PerformAllocation(FnDecl->FuncCFG, FnDecl->StackFrameSize,
				m_DumpLiveIntervals, m_DumpFinalRegAllocs);
		}
	}
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::RunCodeGenerator(CTreeNode *MirTree)
{
	CCodeGenerator CodeGen;

	//take the name of the input file (minus extension) and
	//append .asm on to the end of it
	m_AsmFile = m_InputFile.substr(0, m_InputFile.find_last_of('.')) + ".asm";

	//now do the code generation
		CodeGen.AssembleMIRTree(m_AsmFile, MirTree);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::RunAssemblerAndLinker(void)
{
	//the name of the .obj file we ask the assembler to output, we 
	//then pass this onto the linker
	string ObjFile = m_AsmFile.substr(0, m_AsmFile.find_last_of('.')) + ".obj";

	//build up the assembler command line arguments
	string NasmProg = ".\\tools\\nasmw.exe -f win32 -o ";
	NasmProg += ObjFile + " " + m_AsmFile;

	//run the assembler
	if(!ExecuteProgram(NasmProg))
		InternalError("error executing assembler");

	//run the linker
	string Linker = ".\\tools\\link.exe /entry:main /subsystem:console " + ObjFile;
	Linker += " .\\tools\\libs\\msvcrt.lib .\\tools\\libs\\stdlib.lib";

	if(!ExecuteProgram(Linker))
		InternalError("error executing linker");

	//cleanup the temp files
	remove(ObjFile.c_str());

	if(!m_SaveAsmFile)
		remove(m_AsmFile.c_str());
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::ConstructSingletons(void)
{
	CTreeNodeFactory *TempTreeNodeFactory = new CTreeNodeFactory();
	CSymbolTable *TempSymTable = new CSymbolTable();
	CGlobalDataStore *TempGlobStore = new CGlobalDataStore();
	CBuiltinFuncMngr *TempBuiltinMngr = new CBuiltinFuncMngr();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
void CCompilerDriver::DeleteSingletons(void)
{
	delete CTreeNodeFactory::GetSingletonPtr();
	delete CSymbolTable::GetSingletonPtr();
	delete CGlobalDataStore::GetSingletonPtr();
	delete CBuiltinFuncMngr::GetSingletonPtr();
}
//-------------------------------------------------------------

//-------------------------------------------------------------
bool CCompilerDriver::ExecuteProgram(const string &Prog)
{
	STARTUPINFO StartInfo = { sizeof(STARTUPINFO), 0 };
  PROCESS_INFORMATION ProcInfo = { 0 };
	DWORD ExitCode = 0;
	bool Ret = false;

	//spawn the process
  if(CreateProcess(NULL, (LPSTR)Prog.c_str(), NULL, NULL, FALSE, 0, 
									 NULL, NULL, &StartInfo, &ProcInfo))
  {
		//wait for it to finish then grab it's exit code
    WaitForSingleObject(ProcInfo.hProcess, INFINITE);
    GetExitCodeProcess(ProcInfo.hProcess, &ExitCode);  
    
		//cleanup after ourselves
		CloseHandle(ProcInfo.hProcess);
    CloseHandle(ProcInfo.hThread);

		//just because the process spawned ok it doesn't mean that it
		//completed successfully so check that here to return back to 
		//our caller
		Ret = (ExitCode == 0);
  }  
	else
		Ret = false;

	return Ret;
}
//-------------------------------------------------------------