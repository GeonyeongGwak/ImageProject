#pragma once

#include <DbgHelp.h>

class PemMinidump
{
public:
	static BOOL Begin(VOID);
	static BOOL End(VOID);
};



typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)( // Callback 함수의 원형
	HANDLE hProcess, 
	DWORD dwPid, 
	HANDLE hFile, 
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

LONG WINAPI UnHandledExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo);

bool WriteDIB(LPCTSTR szFile, HANDLE hDIB);
HANDLE DDBToDIB(CBitmap &bitmap, DWORD dwCompression, CPalette *pPal);
void CaptureScreenShot();