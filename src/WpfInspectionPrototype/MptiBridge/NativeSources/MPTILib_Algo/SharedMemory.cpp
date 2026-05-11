#include "StdAfx.h"
#include "SharedMemory.h"
#include <WinIoCtl.h>
#include <strsafe.h>
#include "ExtLog.h"
#include "ExtTools.h"

CSharedMemory::CSharedMemory(void)
	: m_hSharedMemory(INVALID_HANDLE_VALUE)
	, m_nBytes(0)
	, m_pPtr(NULL)
	, m_hFile(INVALID_HANDLE_VALUE)
	, m_dwAccessType(FILE_MAP_WRITE)
	, m_nMapSz(0)
	, m_nMapOffset(0)
	, m_nStartBlock(0)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_dwBytesInBlock = si.dwAllocationGranularity;
}

CSharedMemory::~CSharedMemory(void)
{
	Free();
}

bool CSharedMemory::AllocMemory(QWORD nBytes, CString sMemName, HANDLE hFile)
{
	Free();

	//QWORD Res = nBytes % 65536;
	//if (Res > 0)
	//	Res = 65536 - Res;
	//nBytes += Res;

	QWORD qwAlignSz = m_dwBytesInBlock;
	if ((nBytes % qwAlignSz) > 0)
		nBytes += qwAlignSz;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	m_hSharedMemory = ::CreateFileMapping(hFile, &sa, PAGE_READWRITE, (DWORD)(nBytes >> 32),  (DWORD)(nBytes & 0xFFFFFFFF), sMemName);

	if (m_hSharedMemory == INVALID_HANDLE_VALUE || m_hSharedMemory == NULL)
	{
		DWORD dwError = GetLastError();
		ext::Log::add(ext::Format(_T("m_pPtr = ::CreateFileMapping Error : Code=%d"), dwError));
		return false;
	}
	
	m_nBytes = nBytes;
	return true;
}

bool CSharedMemory::AllocFile(CString sFilePath, QWORD nBytes, CString sMemName, DWORD dwCreationDisposition, DWORD dwFlags)
{
	Free();

	DWORD dwUseCache(0);

// 	if(bUseCache==false)
// 		dwUseCache = FILE_FLAG_WRITE_THROUGH;
	
	if(sFilePath.IsEmpty()==false)
	{
		m_hFile = CreateFile(sFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, dwCreationDisposition, dwFlags, NULL);
		if(m_hFile == INVALID_HANDLE_VALUE)
			return false;

		m_sFilePath = sFilePath;
	}

	m_hSharedMemory = ::CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, (nBytes>>32),  (nBytes&0xffffffff), NULL);
	if(m_hSharedMemory == INVALID_HANDLE_VALUE)
		return false;

	m_nBytes = nBytes;
	return true;
}

void CSharedMemory::Seek(__int64 nBytes, DWORD dwMethod)
{
	LARGE_INTEGER li;
	li.QuadPart = nBytes;
	SetFilePointerEx(m_hFile, li, NULL, dwMethod);
}

bool CSharedMemory::SetZero()
{
	FILE_ZERO_DATA_INFORMATION fzdi;
	DWORD dwTemp(0);
	fzdi.FileOffset.QuadPart = 0;
	fzdi.BeyondFinalZero.QuadPart = m_nBytes;
	bool bRes = ::DeviceIoControl(m_hFile, FSCTL_SET_ZERO_DATA, &fzdi, sizeof(fzdi), NULL, 0, &dwTemp, NULL)!=FALSE;

	return bRes;
}
void CSharedMemory::SetOffSetZero()
{
	m_pPtrOffset = 0;
}

bool CSharedMemory::OpenFile(CString sFilePath, CString sMemName, DWORD dwCreationDisposition, DWORD dwFlags)
{
	Free();

	DWORD dwUseCache(0);

	// 	if(bUseCache==false)
	// 		dwUseCache = FILE_FLAG_WRITE_THROUGH;

	if(sFilePath.IsEmpty()==false)
	{
		m_hFile = CreateFile(sFilePath, GENERIC_READ, 0, NULL, dwCreationDisposition, dwFlags, NULL);
		if(m_hFile == INVALID_HANDLE_VALUE)
			return false;

		m_sFilePath = sFilePath;
	}

	ULARGE_INTEGER nSize;
	if(m_hFile!=INVALID_HANDLE_VALUE)
	{
		nSize.LowPart = ::GetFileSize(m_hFile, &nSize.HighPart);
		m_nBytes = nSize.QuadPart;
	}

	m_hSharedMemory = ::CreateFileMapping(m_hFile, NULL, PAGE_READONLY, (m_nBytes>>32),  (m_nBytes&0xffffffff), NULL);
	if(m_hSharedMemory == INVALID_HANDLE_VALUE)
		return false;

	return true;
}

bool CSharedMemory::Open(QWORD nBytes, CString sMemName, DWORD dwAccessType)
{
	Free();
	m_dwAccessType = dwAccessType;

	QWORD qwAlignSz = m_dwBytesInBlock;
	if ((nBytes % qwAlignSz) > 0)
		nBytes += qwAlignSz;

	m_hSharedMemory = ::OpenFileMapping(dwAccessType, TRUE, sMemName);
	if(m_hSharedMemory == INVALID_HANDLE_VALUE || m_hSharedMemory == NULL)
		return false;
	m_nBytes = nBytes;
	return true;
}

bool CSharedMemory::MapToRead(QWORD nOffset, SIZE_T szMapSize)
{
	return MapView(nOffset, szMapSize, FILE_MAP_READ);
}

bool CSharedMemory::MapToWrite(QWORD nOffset, SIZE_T szMapSize)
{
	return MapView(nOffset, szMapSize, FILE_MAP_WRITE);
}

void CSharedMemory::UnmapView(bool bFlush)
{
	if(m_pPtr != NULL)
	{
		if(bFlush)
			Flush();
		::UnmapViewOfFile(m_pPtr);
	}
	m_pPtr = NULL;
}

void CSharedMemory::Free()
{
	FreeMemory();

	if(m_hFile)
		::CloseHandle(m_hFile);
	m_hFile = NULL;
}

void CSharedMemory::FreeMemory()
{
	if(m_pPtr != NULL)
		::UnmapViewOfFile(m_pPtr);
	m_pPtr = NULL;

	if(m_hSharedMemory != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hSharedMemory);
	m_hSharedMemory = INVALID_HANDLE_VALUE;
}

bool CSharedMemory::Flush(void * ptr, size_t sz)
{
	return FlushViewOfFile(ptr, sz)!=FALSE;
}

bool CSharedMemory::Flush()
{
	return FlushViewOfFile(m_pPtr, m_nMapSz)!=FALSE;
}

QWORD CSharedMemory::MapToAllocGran(QWORD & nPos, SIZE_T & nSz)
{
	if(nPos > 0)
	{
		QWORD dwBytesInBlock = m_dwBytesInBlock;
		QWORD nDivBlockSt = (nPos / dwBytesInBlock) * dwBytesInBlock;		
		QWORD nDivBlockOffset = nPos-nDivBlockSt;

		QWORD nDivBlockSz = ((nSz+nDivBlockOffset) / dwBytesInBlock) * dwBytesInBlock;
		if(nDivBlockSz!=nSz)
			nDivBlockSz += dwBytesInBlock;

		nPos = nDivBlockSt;
		nSz = nDivBlockSz;
		return nDivBlockOffset;
	}
	
	return 0;
}

void CSharedMemory::ReportError(CString sName)
{
	LPVOID lpMsgBuf;
	CString lpDisplayBuf;

	DWORD dw = GetLastError(); 

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
				NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	lpDisplayBuf.Format(_T("%s failed with error [%d: %s]"), sName, dw, lpMsgBuf);
	AfxMessageBox(lpDisplayBuf, MB_ICONWARNING); 

	LocalFree(lpMsgBuf);
}
bool CSharedMemory::MapViewAll(DWORD dwDesiredAccess)
{
	return MapView(0, 0, dwDesiredAccess);
}
bool CSharedMemory::MapView(QWORD nOffset, SIZE_T szMapSize, DWORD dwDesiredAccess)
{
// 	DWORD dwErrorCode(0);
// 	if(m_pPtr != NULL)
// 		::UnmapViewOfFile(m_pPtr);
// 	m_pPtr = NULL;

	int nMapOffset = MapToAllocGran(nOffset, szMapSize);
	if(m_pPtr!=NULL && m_nStartBlock <= nOffset && (m_nStartBlock+m_nMapSz)>=(nOffset+szMapSize))
	{
		__int64 nDiff = nOffset - m_nStartBlock;
		m_nMapOffset = nMapOffset;
		if(m_pPtr)
			m_pPtrOffset = &m_pPtr[nDiff+m_nMapOffset];
		return true;
	}
	else
	{
		DWORD dwErrorCode(0);
		if(m_pPtr)
			::UnmapViewOfFile(m_pPtr);
		m_pPtr = NULL;
	}
	m_nStartBlock = nOffset;
	m_nMapOffset = nMapOffset;

	m_nMapSz = szMapSize>0? szMapSize:m_nBytes;
	m_pPtr = (BYTE *)::MapViewOfFile(m_hSharedMemory, dwDesiredAccess, (DWORD)(nOffset >> 32),  (DWORD)(nOffset & 0xFFFFFFFF), szMapSize);
	if (m_pPtr)
		m_pPtrOffset = &m_pPtr[m_nMapOffset];
	else
	{
		DWORD dwError = GetLastError();
		CString str; 
		str.Format(_T("m_pPtr = ::MapViewOfFile is nullptr : Code=%d"), dwError);
		ext::Log::add(str);
	}
	return m_pPtr != NULL;
}