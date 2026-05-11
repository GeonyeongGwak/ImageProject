#include "stdafx.h"
#include "extLog.h"
#include "MPTI.h"

namespace ext {
	void Log::add(CString sLog, int nLogLv)
	{
#ifdef _EXTMONITOR
#else
		g_pMPTI->AddLog_Dev(sLog);
#endif 
	}
}