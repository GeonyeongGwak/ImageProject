#pragma once
#ifdef _AITester
#include "../AIWrapper/iSession.h"
#else
#include "iSession.h"
#endif


isession * create_pytorch(CString work, CString type, CString path, int nGpuID);
isession * create_csml(CString work, CString path, int nGpuID);