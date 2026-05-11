#include "stdafx.h"
#include "Proc_Foot.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif



CProc_Foot::CProc_Foot()
{
	m_className = _T("CProc_Foot");

	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;
}


CProc_Foot::~CProc_Foot()
{
}



int CProc_Foot::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if (milApp == M_NULL || milSys == M_NULL)
	{
		return 1;
	}

	CProcMil::InitMil(milApp, milSys, bUseImagePilLib);

	InitMilAlgoBlob();

	return 0;
}

int CProc_Foot::FreeMil()
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
