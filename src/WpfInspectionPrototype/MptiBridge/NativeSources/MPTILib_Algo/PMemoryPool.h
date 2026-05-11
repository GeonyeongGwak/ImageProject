#pragma once
#include <vector>

class PMemoryPool
{

public:
	PMemoryPool();
	~PMemoryPool();

	void SetSize(long long totlaSize);
	UCHAR* RequstBuffer(long long size);

	void Clear();


private:
	std::vector<UCHAR> m_Buffer;
	long long m_Current_Pos;
	long long m_Max_Length;

};

