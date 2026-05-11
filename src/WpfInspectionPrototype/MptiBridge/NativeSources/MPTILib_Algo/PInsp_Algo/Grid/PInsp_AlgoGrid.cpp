#include "PInsp_AlgoGrid.h"


CPInsp_AlgoGrid::CPInsp_AlgoGrid(void)
{
}


CPInsp_AlgoGrid::~CPInsp_AlgoGrid(void)
{
}

void CPInsp_AlgoGrid::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoGrid::GetInspAlgoData()
{
	return eSPCAlgoGrid;
}

int CPInsp_AlgoGrid::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	eWholeNgTypeTemp[TypeForeign] = e_NG;
	return TypeForeign;
}

bool CPInsp_AlgoGrid::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoGrid::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	CPInsp_Grid *pGridInsp = g_pInspMng->GetGridInsp();
	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	if (pGridInsp ==  nullptr || pInspBoardInfo == nullptr)
		return bResult;

	pGridInsp->SetInspParam(sInspAlgo, sWndAlgoImg, *stAlgoParam.m_coordinateAlgo, pInspBoardInfo);

	pGridInsp->InspProc();

	bResult = pGridInsp->GetInspRst((RstAlgoGrid *)sRstAlgo);

	return bResult;
}

BOOL CPInsp_AlgoGrid::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoGrid::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoGrid::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}