#pragma once
#include "ProcMil.h"



class CProc_Wire :
	public CProcMil
{
public:
	CProc_Wire();
	~CProc_Wire();

public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int FreeMil();
};

