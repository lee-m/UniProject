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
#include <sstream>
#include <map>
#include <set>
#include <queue>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdarg>
using namespace std;

#include "tinyxml.h"

//must come in between std includes and our own
#include "mmgr.h"

//all files need access to this
#include "CInternalErrorException.h"

//stable headers which aren't likely to change very often (if at all)
#include "CUtilFuncs.h"
#include "CTreeNodeFactory.h"
#include "TreeChildIndicies.h"