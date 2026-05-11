#include "stdafx.h"
#include "PMemoryPool.h"
#include <stdexcept>

PMemoryPool::PMemoryPool()
{
	m_Current_Pos =0;
	m_Max_Length =0;
}


PMemoryPool::~PMemoryPool()
{
}

void PMemoryPool::SetSize(long long totlaSize)
{
	m_Current_Pos = 0;
	m_Max_Length = totlaSize;
	m_Buffer.clear();
	m_Buffer.resize(totlaSize);
}

UCHAR* PMemoryPool::RequstBuffer(long long size)
{
	if ((m_Current_Pos + size) > m_Max_Length)
	{
		throw std::invalid_argument("Size is Over flow");
		return 0;
	}

	UCHAR* ptr = &m_Buffer[m_Current_Pos];
	m_Current_Pos += size;

	return ptr;
}

void PMemoryPool::Clear()
{
	m_Current_Pos = 0;
	m_Max_Length = 0;
	m_Buffer.clear();

	std::vector<UCHAR> deleteBuffer;
	m_Buffer.swap(deleteBuffer);
}