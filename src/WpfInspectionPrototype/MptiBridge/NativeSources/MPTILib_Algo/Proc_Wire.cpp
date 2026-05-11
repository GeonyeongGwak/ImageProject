#include "stdafx.h"
#include "Proc_Wire.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif




CProc_Wire::CProc_Wire()
{
	m_className = _T("CProc_Wire");

	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;
}


CProc_Wire::~CProc_Wire()
{
}



int CProc_Wire::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if (milApp == M_NULL || milSys == M_NULL)
	{
		return 1;
	}

	CProcMil::InitMil(milApp, milSys, bUseImagePilLib);

	InitMilAlgoBlob();

	return 0;
}

int CProc_Wire::FreeMil()
{
#ifndef _DISABLE_MIL	
	if (m_milBlobFeature != M_NULL)
	{
		Im::Blob::blobFree(m_milBlobFeature);
		m_milBlobFeature = M_NULL;
	}

	if (m_milBlobResult != M_NULL)
	{
		Im::Blob::blobFree(m_milBlobResult);
		m_milBlobResult = M_NULL;
	}

	FreeMilAlgoBlob();

	CProcMil::FreeMil();
#endif

	return 0;
}
