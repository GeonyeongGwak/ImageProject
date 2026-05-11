

#include "stdafx.h"
#include "QTimer.h"

CQTimer::CQTimer()
{
}

CQTimer::~CQTimer()
{
}

void CQTimer::StartTick()
{
	if (!QueryPerformanceFrequency(&ticksPerSecond))
    {
    	m_bUse = false;
	    return;
    }
    else
    	m_bUse = true;

    QueryPerformanceCounter(&Startticks);
}

double CQTimer::EndTick()
{
	if(m_bUse == false) return 0;

	QueryPerformanceCounter(&Endticks);

	double seconds = ( (double)(Endticks.QuadPart-Startticks.QuadPart) / (double)ticksPerSecond.QuadPart);
    return seconds;
}

void CQTimer::WaitSomeSec(double dSec)
{
	StartTick();
	double curSec = 0;
	while(dSec)
	{
		Sleep(0);
		curSec = EndTick();
		if(curSec >= dSec)
		{
			break;
		}
	}
}