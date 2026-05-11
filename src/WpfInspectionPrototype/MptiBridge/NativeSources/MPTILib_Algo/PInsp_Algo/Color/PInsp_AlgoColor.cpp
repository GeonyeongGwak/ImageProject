#include "PInsp_AlgoColor.h"


CPInsp_AlgoColor::CPInsp_AlgoColor(void)
{
}


CPInsp_AlgoColor::~CPInsp_AlgoColor(void)
{
}

void CPInsp_AlgoColor::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoColor::GetInspAlgoData()
{
	return eSPCAlgoColor;
}

int CPInsp_AlgoColor::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoColor * rstAlgo = (RstAlgoColor *)vRstInspAlgo;
	if (nWndType == eINSP_SOLDER)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}
	else
	{
		if (!rstAlgo->m_bRateOK)
		{
			eWholeNgTypeTemp[TypeCOLOR] = e_NG;
			nCurrentNgType = TypeCOLOR;
		}
		if (!rstAlgo->m_bOKPolarity)
		{
			eWholeNgTypeTemp[TypeMountPolarity] = e_NG;
			nCurrentNgType = TypeMountPolarity;
		}
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoColor::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoColor::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	if (pColorTeach == nullptr || pInspBoardInfo == nullptr)
		return bResult;

	AlgoColor* pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
	RECT* rcLeadInsp = g_pMManager->pem_new<RECT>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	rcLeadInsp->left = 0;
	rcLeadInsp->right = 0;
	rcLeadInsp->top = 0;
	rcLeadInsp->bottom = 0;
	AlgoCoordinate coordinateAlgo = *stAlgoParam.m_coordinateAlgo;
	pColorTeach->SetInspParam(sInspAlgo, sWndAlgoImg, coordinateAlgo, pInspBoardInfo, stAlgoParam.m_nLeadTipPos, rcLeadInsp, sWndAlgoImg.m_nLight_index, sInspImageData, &stTieArea);
	for (int a = 0; a < 3; a++)
	{
		pColorTeach->m_sLightData[a].m_byLightCnt = g_pInspMng->m_sLightData[a].m_byLightCnt;
		for (int b = 0; b < LIGHT_DATA_CNT; b++)
		{
			for (int c = 0; c < LIGHT_CNT; c++)
				pColorTeach->m_sLightData[a].m_byArrLightData[b][c] = g_pInspMng->m_sLightData[a].m_byArrLightData[b][c];
		}
	}

	pColorTeach->InspProc();  // YJS 2016/11/04
	int nAlgo = stAlgoParam.m_nAlgo;
	rcLeadInsp->left += stAlgoParam.m_dx;
	rcLeadInsp->right += stAlgoParam.m_dx;
	rcLeadInsp->top += stAlgoParam.m_dy;
	rcLeadInsp->bottom += stAlgoParam.m_dy;
	pColorTeach->GetInspRst(stAlgoParam.m_WndResult->m_vArrRstInspAlgo[nAlgo], *rcLeadInsp);
	if (pAlgoColor && pAlgoColor->IsUseMultiArea())
		g_pInspMng->TabNGRectChangePosition(stAlgoParam.m_WndResult, nAlgo, stAlgoParam.m_dx, stAlgoParam.m_dy);
	bResult = stAlgoParam.m_WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk;

	g_pMManager->pem_delete(rcLeadInsp, false);

	return bResult;
}

BOOL CPInsp_AlgoColor::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoColor::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoColor * pInspAlgo = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
	nData |= COLOR_DATA_USE;
	nData |= COLOR_DATA_WND;
	if (pInspAlgo)
	{
		if (pInspAlgo->m_bUseColorMap2)
			nData |= COLOR_DATA_BIN_AC;
		if (pInspAlgo->m_byColorLightType == 1)
			nData |= COLOR_DATA_MID;
	}
	return nData;
}
bool CPInsp_AlgoColor::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}