#pragma once

// YJS 2014/01/14 

class CSharedMemory
{
public:
	CSharedMemory(void);
	virtual ~CSharedMemory(void);

public:
	bool AllocFile(CString sFilePath, QWORD nBytes, CString sMemName, DWORD dwCreationDisposition=CREATE_ALWAYS, DWORD dwFlags=FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED);
	bool OpenFile(CString sFilePath, CString sMemName, DWORD dwCreationDisposition=OPEN_EXISTING, DWORD dwFlags=FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED);
	bool Open(QWORD nBytes, CString sMemName, DWORD dwAccessType=FILE_MAP_ALL_ACCESS);
	void Free();

	bool AllocMemory(QWORD nBytes, CString sMemName, HANDLE hFile=INVALID_HANDLE_VALUE);
	void FreeMemory();

	QWORD MapToAllocGran(QWORD & nPos, SIZE_T & nSz);
	bool MapView(QWORD nOffset, SIZE_T szMapSize, DWORD dwDesiredAccess);
	bool MapViewAll(DWORD dwDesiredAccess);
	bool MapToRead(QWORD nOffset, SIZE_T szMapSize);
	bool MapToWrite(QWORD nOffset, SIZE_T szMapSize);
	void UnmapView(bool bFlush=false);
	bool Flush(void * ptr, size_t sz);
	bool Flush();

	void Seek(__int64 nBytes, DWORD dwMethod=FILE_BEGIN);

	bool SetZero();
	void SetOffSetZero();

	template<typename T>
	T * Ptr(int pos)
	{
		T * p = (T *)m_pPtrOffset;
		return &T[pos];
	}
	template<typename T>
	T * CopyOf(int pos, T * lst)
	{
		T * p = (T *)m_pPtrOffset;
		memcpy(&p[pos], lst, sizeof(T));
		return &T[pos];
	}
	
	BYTE * Byte(int nOffset=0) { return &m_pPtrOffset[nOffset]; }
	float * Float() { return (float *)m_pPtrOffset; }
	UINT * UInt() { return (UINT *)m_pPtrOffset; }
	short * Short() { return (short *)m_pPtrOffset; }
	QWORD GetMemSize() { return m_nBytes;  }

	DWORD m_dwAccessType;
	QWORD m_nStartBlock;
	QWORD m_nMapOffset;
	HANDLE m_hFile;
	QWORD m_dwBytesInBlock;
	QWORD m_nBytes;
protected:
	void ReportError(CString sName=_T("CSharedMemory"));

	CString m_sFilePath;
	
	
	HANDLE m_hSharedMemory;


	BYTE * m_pPtr;
	BYTE * m_pPtrOffset;
	QWORD m_nMapSz;
};