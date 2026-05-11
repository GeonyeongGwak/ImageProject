#pragma once
#include "ProcMil.h"



class CProc_Foot :
	public CProcMil
{
public:
	CProc_Foot();
	~CProc_Foot();

public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int FreeMil();


private:
	
};

