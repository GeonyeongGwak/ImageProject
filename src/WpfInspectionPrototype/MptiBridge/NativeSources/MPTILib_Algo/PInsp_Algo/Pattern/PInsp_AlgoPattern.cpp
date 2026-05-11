#include "PInsp_AlgoPattern.h"


CPInsp_AlgoPattern::CPInsp_AlgoPattern(void)
{
}


CPInsp_AlgoPattern::~CPInsp_AlgoPattern(void)
{
}

void CPInsp_AlgoPattern::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoPattern::GetInspAlgoData()
{
	return eSPCAlgoPattern;
}

int CPInsp_AlgoPattern::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoPattern * rstAlgo = (RstAlgoPattern *)vRstInspAlgo;
	if (!rstAlgo->m_bOKOffsetX || !rstAlgo->m_bOKOffsetY)
	{
		eWholeNgTypeTemp[TypeMountShift] = e_NG;
		nCurrentNgType = TypeMountShift;
	}
	if (!rstAlgo->m_bOKAngle)
	{
		eWholeNgTypeTemp[TypeMountAngle] = e_NG;
		nCurrentNgType = TypeMountAngle;
	}
	if (!rstAlgo->m_bOKPolarity)
	{
		eWholeNgTypeTemp[TypeMountPolarity] = e_NG;
		nCurrentNgType = TypeMountPolarity;
	}
	if (!rstAlgo->m_bOKFind || !rstAlgo->m_bOKScore)
	{
		eWholeNgTypeTemp[TypeMountWorng] = e_NG;
		nCurrentNgType = TypeMountWorng;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoPattern::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoPattern::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	CPInsp_Pattern *pPatternInsp = g_pInspMng->GetPatternInsp();
	if (pPatternInsp == nullptr)
		return bResult;

	pPatternInsp->SetInspParam(sInspAlgo, sWndAlgoImg, *stAlgoParam.m_coordinateAlgo, stAlgoParam.m_dInspW, stAlgoParam.m_dInspH, stTieArea);
	// 3 들어가는거 공유메모리에 Model Load 하면 바꿔야함
	bResult = pPatternInsp->InspProc_Pattern(0, stAlgoParam.m_nOffX_pix, stAlgoParam.m_nOffY_pix);

	pPatternInsp->GetInspRst((RstAlgoPattern *)sRstAlgo);

	return bResult;
}

BOOL CPInsp_AlgoPattern::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	RstAlgoPattern * rst = (RstAlgoPattern *)sRstAlgo;
	pAlignRes->offsetX = -rst->offsetX;
	pAlignRes->offsetY = rst->offsetY;
	pAlignRes->theta = rst->angle;		//  만약 isReverse가 true 이면??
	bRet = TRUE;
	
	return bRet;
}
int CPInsp_AlgoPattern::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoPattern::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}