#pragma once
#ifdef _AITester
#include "../AIWrapper/iSession.h"
#else
#include "iSession.h"
#endif

isession * create_tensorrt(CString work, CString path, int nGpuID);