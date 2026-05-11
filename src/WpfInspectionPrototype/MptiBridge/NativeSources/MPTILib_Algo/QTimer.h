//---------------------------------------------------------------------------

#pragma once
//---------------------------------------------------------------------------
#include "Windows.h"

class CQTimer
{
public:

	LARGE_INTEGER ticksPerSecond;
  	LARGE_INTEGER Startticks,ElpasedTick,Endticks;

	bool m_bUse;
    CQTimer();
    ~CQTimer();
    void StartTick();
    double EndTick();     //sec

	void WaitSomeSec(double dSec);
};

