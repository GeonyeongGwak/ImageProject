#include "PInsp_AlgoBGA.h"

CPInsp_AlgoBGA::CPInsp_AlgoBGA(void)
{
}


CPInsp_AlgoBGA::~CPInsp_AlgoBGA(void)
{
}

void CPInsp_AlgoBGA::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoBGA::GetInspAlgoData()
{
	return eSPCAlgoBGA;
}

int CPInsp_AlgoBGA::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoBGA * rstAlgo = (RstAlgoBGA *)vRstInspAlgo;

	if (!rstAlgo->m_bCoplanarityOK)
	{
		eWholeNgTypeTemp[TypeUpsidedown] = e_NG;
		nCurrentNgType = TypeUpsidedown;
	}

	return nCurrentNgType;
}

bool CPInsp_AlgoBGA::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoBGA::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	return bResult;
}

BOOL CPInsp_AlgoBGA::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoBGA::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoBGA::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}