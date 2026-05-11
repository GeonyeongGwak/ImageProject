//////////////////////////////////////////////////////////////////////////
//
//	CSML은 GPU 코드이기 때문에 pem_ 함수로 수정 x
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <list>
#include <algorithm>

using namespace std;

#define FUNC_NAME 1024

typedef struct tagMemoryItem
{
	LPVOID pStartAddr;
	UINT nAllocLine;
	PCHAR pszAllocFuncName;
	bool bArray;
	bool bWrongDelete;

}MemoryItem, *LPMemoryItem;

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();

private:

	std::map<LPVOID, LPMemoryItem> map_Leak;
	std::list<LPMemoryItem> list_Leak;
	CRITICAL_SECTION m_cs;

	void CustomTrace(const PCHAR format, ...);

	LPVOID pem_malloc(LPVOID pMem, size_t size, PCHAR FN, UINT LN, bool bReAlloc);
	LPVOID pem_aligned_malloc(LPVOID pMem, size_t size, size_t align, PCHAR FN, UINT LN, bool bReAlloc);
	LPVOID pem_LocalAlloc(LPVOID pMem, UINT nFlags, SIZE_T uBytes, PCHAR FN, UINT LN, bool bReAlloc);

	void pem_free(LPVOID pMem, bool bReAlloc);
	void pem_aligned_free(LPVOID pMem, bool bReAlloc);
	void pem_LocalFree(LPVOID pMem, bool bReAlloc);

	void InsertMap(LPVOID pAlloc, bool bArray, PCHAR FN, UINT LN);
	void DeleteMap(LPVOID pMem);

	template <typename T> T* pem_new_memory(bool bArray, UINT nArrayCnt, bool bZeroInit);
	template <typename T> void pem_delete_memory(T* pMem, bool bArray);

	int m_nAlloc;
	int m_nRelease;
public:
	LPVOID pem_malloc(size_t size, PCHAR FN, UINT LN);
	LPVOID pem_realloc(LPVOID pMem, size_t size, PCHAR FN, UINT LN);
	void pem_free(LPVOID pMem);

	LPVOID pem_aligned_malloc(size_t size, size_t align, PCHAR FN, UINT LN);
	LPVOID pem_aligned_realloc(LPVOID pMem, size_t size, size_t align, PCHAR FN, UINT LN);
	void pem_aligned_free(LPVOID pMem);

	LPVOID pem_LocalAlloc(UINT nFlags, SIZE_T uBytes, PCHAR FN, UINT LN);
	LPVOID pem_LocalReAlloc(LPVOID pMem, UINT nFlags, SIZE_T uBytes, PCHAR FN, UINT LN);
	void pem_LocalFree(LPVOID pMem);

	// bZeroInit = true -> new Class();, false -> new Class;
	template <typename T> T* pem_new(bool bArray, UINT nArrayCnt, PCHAR FN, UINT LN, bool bZeroInit = false);
	template <typename T> void pem_delete(T* pMem, bool bArray);


	// 메모리 확인 용 (메모리 생성 대리 안함) [param이 있는 생성자, 또는 클래스 생성자에서 사용]
	void pem_new_check(LPVOID pMem, PCHAR FN, UINT LN);

	// 클래스 소멸자 위치에서만 사용 할 것.
	void pem_delete_check(LPVOID pMem);
};

extern MemoryManager *g_pMManager;

template <typename T>
T* MemoryManager::pem_new_memory(bool bArray, UINT nArrayCnt, bool bZeroInit)
{
	T* pAlloc = NULL;

	if (bArray == false)
	{
		if (bZeroInit == true)
			pAlloc = new T();
		else
			pAlloc = new T;
	}
	else
	{
		pAlloc = new T[nArrayCnt];
	}

	return pAlloc;
}

template <typename T>
T* MemoryManager::pem_new(bool bArray, UINT nArrayCnt, PCHAR FN, UINT LN, bool bZeroInit)
{
#ifndef _MEMCHECK
	return pem_new_memory<T>(bArray, nArrayCnt, bZeroInit);
#else

	EnterCriticalSection(&m_cs);
	T* pAlloc = pem_new_memory<T>(bArray, nArrayCnt, bZeroInit);
	InsertMap(pAlloc, bArray, FN, LN);
	LeaveCriticalSection(&m_cs);

	return pAlloc;

#endif
}

template <typename T>
void MemoryManager::pem_delete_memory(T* pMem, bool bArray)
{
	if (bArray == true)
		delete[] pMem;
	else
		delete pMem;
}

template <typename T>
void MemoryManager::pem_delete(T* pMem, bool bArray)
{
#ifndef _MEMCHECK
	pem_delete_memory<T>(pMem, bArray);
#else

	EnterCriticalSection(&m_cs);

	map<LPVOID, LPMemoryItem>::iterator iter_Release_map;
	iter_Release_map = map_Leak.find(pMem);

	if (iter_Release_map == map_Leak.end())
	{
		if (pMem == NULL)
		{
			LeaveCriticalSection(&m_cs);
			return;
		}

		// No Alloc, Only Free, Check & Modify Alloc Position
		ASSERT(FALSE);
	}
	else
	{
		if (bArray != iter_Release_map->second->bArray)
		{
			iter_Release_map->second->bWrongDelete = true;
			list_Leak.push_back(iter_Release_map->second);
			map_Leak.erase(iter_Release_map);
		}
		else
		{
			delete[] iter_Release_map->second->pszAllocFuncName;
			delete iter_Release_map->second;
			map_Leak.erase(iter_Release_map);
		}

		++m_nRelease;
	}

	pem_delete_memory<T>(pMem, bArray);
	LeaveCriticalSection(&m_cs);

#endif
}

