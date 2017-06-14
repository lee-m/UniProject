//------------------------------------------------------------------------------------------
// File: StdHeader.h
// Desc: Standard library header file includes
// Auth: Lee Millward
//------------------------------------------------------------------------------------------

#include "nommgr.h"

#include <string>
#include <fstream>
#include <map>
#include <list>
#include <stack>
#include <iostream>
#include <algorithm>
#include <utility>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdarg>
using namespace std;

#include "mmgr.h"

//tree nodes
#include "CRootParseTreeNode.h"
#include "CParseTreeNode.h"
#include "CIdentifierTreeNode.h"
#include "CVarDeclTreeNode.h"
#include "CArrayDeclTreeNode.h"
#include "CLiteralTreeNode.h"
#include "CBinaryExprTreeNode.h"
#include "CUnaryExprTreeNode.h"
#include "CArrayRefTreeNode.h"
#include "CIfStmtTreeNode.h"
#include "CWhileStmtTreeNode.h"
#include "CAssignExprTreeNode.h"
#include "CParameterListTreeNode.h"
#include "CFunctionDeclTreeNode.h"
#include "CParameterDeclTreeNode.h"
#include "CReturnStmtTreeNode.h"
#include "CCallExprTreeNode.h"
#include "CUnaryExprTreeNode.h"