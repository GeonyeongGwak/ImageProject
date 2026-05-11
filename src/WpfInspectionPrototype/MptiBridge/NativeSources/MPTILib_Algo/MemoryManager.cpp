#include "StdAfx.h"
#include "MemoryManager.h"

using namespace std;

MemoryManager *g_pMManager = NULL;

bool cmp_list(const LPMemoryItem &a, const LPMemoryItem &b)
{
	int nResult = std::strcmp(a->pszAllocFuncName, b->pszAllocFuncName);

	if (nResult == 0)
		return a->nAllocLine < b->nAllocLine;
	else
		return nResult < 0;
}

MemoryManager::MemoryManager()
{
	m_nAlloc = 0;
	m_nRelease = 0;
	map_Leak.clear();
	list_Leak.clear();

	InitializeCriticalSection(&m_cs);
}

MemoryManager::~MemoryManager()
{
	int nLeakCnt = map_Leak.size();
#if _DEBUG
	if (nLeakCnt > 0 || list_Leak.size() > 0)
		MessageBox(NULL, _T("Memory Leak Check Plz"), _T("Memory Leak"), MB_OK | MB_ICONERROR);
#endif
#ifdef _MEMCHECK
	CustomTrace("\r\n\r\nInfo Total Alloc[%d] Release[%d] nRemainCnt[%d]", m_nAlloc, m_nRelease, nLeakCnt);

	if (nLeakCnt == 0 && list_Leak.size() == 0)
	{
		CustomTrace("\r\n�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�CLEAR�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�\r\n\r\n");
		return;
	}

	map<LPVOID, LPMemoryItem>::iterator iter_map;
	for (iter_map = map_Leak.begin(); iter_map != map_Leak.end(); ++iter_map)
		list_Leak.push_back(iter_map->second);

	if (list_Leak.size() > 0)
	{
		list_Leak.sort(cmp_list);

		list<LPMemoryItem>::iterator iter_list;
		CustomTrace("\r\n�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ� [Total:%d]\r\n", list_Leak.size());

		for (iter_list = list_Leak.begin(); iter_list != list_Leak.end(); ++iter_list)
		{
			if ((*iter_list)->bWrongDelete == true)
				CustomTrace("[WrongDelete] FN : %s, LN : %d (Addr:%08x)\r\n", (*iter_list)->pszAllocFuncName, (*iter_list)->nAllocLine, (*iter_list)->pStartAddr);
			else
				CustomTrace("FN : %s, LN : %d (Addr:%08x)\r\n", (*iter_list)->pszAllocFuncName, (*iter_list)->nAllocLine, (*iter_list)->pStartAddr);

			delete[](*iter_list)->pszAllocFuncName;
			delete (*iter_list);
		}
		CustomTrace("\r\n");
	}

	CustomTrace("\r\n");

	map_Leak.clear();
	list_Leak.clear();

#endif

	DeleteCriticalSection(&m_cs);
}

void MemoryManager::CustomTrace(const PCHAR format, ...)
{
	const int TraceBufferSize = 1024;
	CHAR buffer[TraceBufferSize];

	va_list argptr; va_start(argptr, format);
	vsprintf_s(buffer, format, argptr);
	va_end(argptr);

	OutputDebugStringA(buffer);
}

void MemoryManager::pem_free(LPVOID pMem, bool bReAlloc)
{
	DeleteMap(pMem);

	if (bReAlloc == false)
		free(pMem);
}

void MemoryManager::pem_aligned_free(LPVOID pMem, bool bReAlloc)
{
	DeleteMap(pMem);

	if (bReAlloc == false)
		_aligned_free(pMem);
}

void MemoryManager::pem_LocalFree(LPVOID pMem, bool bReAlloc)
{
	DeleteMap(pMem);

	if (bReAlloc == false)
		LocalFree(pMem);
}

void MemoryManager::pem_free(LPVOID pMem)
{
#ifndef _MEMCHECK
	free(pMem);
#else

	EnterCriticalSection(&m_cs);
	pem_free(pMem, false);
	LeaveCriticalSection(&m_cs);

#endif
}

void MemoryManager::pem_aligned_free(LPVOID pMem)
{
#ifndef _MEMCHECK
	_aligned_free(pMem);
#else

	EnterCriticalSection(&m_cs);
	pem_aligned_free(pMem, false);
	LeaveCriticalSection(&m_cs);

#endif
}

void MemoryManager::pem_LocalFree(LPVOID pMem)
{
#ifndef _MEMCHECK
	LocalFree(pMem);
#else

	EnterCriticalSection(&m_cs);
	pem_LocalFree(pMem, false);
	LeaveCriticalSection(&m_cs);

#endif
}

LPVOID MemoryManager::pem_realloc(LPVOID pMem, size_t size, PCHAR FN, UINT LN)
{
#ifndef _MEMCHECK
	return realloc(pMem, size);
#else

	EnterCriticalSection(&m_cs);
	pem_free(pMem, true);
	LPVOID p = pem_malloc(pMem, size, FN, LN, true);
	LeaveCriticalSection(&m_cs);

	return p;

#endif
}

LPVOID MemoryManager::pem_aligned_realloc(LPVOID pMem, size_t size, size_t align, PCHAR FN, UINT LN)
{
#ifndef _MEMCHECK
	return _aligned_realloc(pMem, size, align);
#else

	EnterCriticalSection(&m_cs);
	pem_aligned_free(pMem, true);
	LPVOID p = pem_aligned_malloc(pMem, size, align, FN, LN, true);
	LeaveCriticalSection(&m_cs);

	return p;

#endif
}

LPVOID MemoryManager::pem_LocalReAlloc(LPVOID pMem, UINT nFlags, SIZE_T uBytes, PCHAR FN, UINT LN)
{
#ifndef _MEMCHECK
	return LocalReAlloc(pMem, uBytes, nFlags);
#else

	EnterCriticalSection(&m_cs);
	pem_LocalFree(pMem, true);
	LPVOID p = pem_LocalAlloc(pMem, nFlags, uBytes, FN, LN, true);
	LeaveCriticalSection(&m_cs);

	return p;

#endif
}

LPVOID MemoryManager::pem_malloc(LPVOID pMem, size_t size, PCHAR FN, UINT LN, bool bReAlloc)
{
	LPVOID pAlloc = NULL;

	if (bReAlloc == false)
		pAlloc = malloc(size);
	else
		pAlloc = realloc(pMem, size);


	InsertMap(pAlloc, size, FN, LN);
	return pAlloc;
}

LPVOID MemoryManager::pem_aligned_malloc(LPVOID pMem, size_t size, size_t align, PCHAR FN, UINT LN, bool bReAlloc)
{
	LPVOID pAlloc = NULL;

	if (bReAlloc == false)
		pAlloc = _aligned_malloc(size, align);
	else
		pAlloc = _aligned_realloc(pMem, size, align);


	InsertMap(pAlloc, false, FN, LN);
	return pAlloc;
}

LPVOID MemoryManager::pem_LocalAlloc(LPVOID pMem, UINT nFlags, SIZE_T uBytes, PCHAR FN, UINT LN, bool bReAlloc)
{
	LPVOID pAlloc = NULL;

	if (bReAlloc == false)
		pAlloc = LocalAlloc(nFlags, uBytes);
	else
		pAlloc = LocalReAlloc(pMem, uBytes, nFlags);


	InsertMap(pAlloc, false, FN, LN);
	return pAlloc;
}

void MemoryManager::InsertMap(LPVOID pAlloc, bool bArray, PCHAR FN, UINT LN)
{
	LPMemoryItem pItem = new MemoryItem();
	pItem->pStartAddr = pAlloc;
	pItem->pszAllocFuncName = new CHAR[FUNC_NAME];
	ZeroMemory(pItem->pszAllocFuncName, FUNC_NAME);
	strcpy_s(pItem->pszAllocFuncName, FUNC_NAME, FN);
	pItem->nAllocLine = LN;
	pItem->bArray = bArray;

	pair<map<LPVOID, LPMemoryItem>::iterator, bool> ret;

	ret = map_Leak.insert(pair<LPVOID, LPMemoryItem>(pAlloc, pItem));

	if (ret.second == false)
	{
		delete[] pItem->pszAllocFuncName;
		delete pItem;

		return;
	}

	++m_nAlloc;
}

void MemoryManager::DeleteMap(LPVOID pMem)
{
	map<LPVOID, LPMemoryItem>::iterator iter_Release_map;
	iter_Release_map = map_Leak.find(pMem);

	if (iter_Release_map == map_Leak.end())
	{
		if (pMem == NULL)
			return;

		// No Alloc, Only Free, Parent/Child Create
		//ASSERT(FALSE);
	}
	else
	{
		delete[] iter_Release_map->second->pszAllocFuncName;
		delete iter_Release_map->second;
		map_Leak.erase(iter_Release_map);

		++m_nRelease;
	}
}

LPVOID MemoryManager::pem_malloc(size_t size, PCHAR FN, UINT LN)
{
#ifndef _MEMCHECK
	return malloc(size);
#else

	EnterCriticalSection(&m_cs);
	LPVOID p = pem_malloc(NULL, size, FN, LN, false);
	LeaveCriticalSection(&m_cs);

	return p;

#endif
}

LPVOID MemoryManager::pem_aligned_malloc(size_t size, size_t align, PCHAR FN, UINT LN)
{
#ifndef _MEMCHECK
	return _aligned_malloc(size, align);
#else

	EnterCriticalSection(&m_cs);
	LPVOID p = pem_aligned_malloc(NULL, size, align, FN, LN, false);
	LeaveCriticalSection(&m_cs);

	return p;

#endif
}

LPVOID MemoryManager::pem_LocalAlloc(UINT nFlags, SIZE_T uBytes, PCHAR FN, UINT LN)
{
#ifndef _MEMCHECK
	return LocalAlloc(nFlags, uBytes);
#else

	EnterCriticalSection(&m_cs);
	LPVOID p = pem_LocalAlloc(NULL, nFlags, uBytes, FN, LN, false);
	LeaveCriticalSection(&m_cs);

	return p;

#endif
}

void MemoryManager::pem_new_check(LPVOID pMem, PCHAR FN, UINT LN)
{
#ifdef _MEMCHECK

	EnterCriticalSection(&m_cs);
	InsertMap(pMem, false, FN, LN);
	LeaveCriticalSection(&m_cs);

#endif
}

void MemoryManager::pem_delete_check(LPVOID pMem)
{
#ifdef _MEMCHECK

	EnterCriticalSection(&m_cs);
	DeleteMap(pMem);
	LeaveCriticalSection(&m_cs);

#endif
}
