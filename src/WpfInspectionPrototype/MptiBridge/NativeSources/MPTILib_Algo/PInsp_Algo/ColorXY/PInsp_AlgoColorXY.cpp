#include "PInsp_AlgoColorXY.h"


CPInsp_AlgoColorXY::CPInsp_AlgoColorXY(void)
{
}


CPInsp_AlgoColorXY::~CPInsp_AlgoColorXY(void)
{
}

void CPInsp_AlgoColorXY::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoColorXY::GetInspAlgoData()
{
	return eSPCAlgoColorXY;
}

int CPInsp_AlgoColorXY::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoColorXY * rstAlgo = (RstAlgoColorXY *)vRstInspAlgo;
	float nMinX = rstAlgo->m_fStdX_Min;
	float nMaxX = rstAlgo->m_fStdX_Max;
	float nMinY = rstAlgo->m_fStdY_Min;
	float nMaxY = rstAlgo->m_fStdY_Max;
	float nAvgX = rstAlgo->m_dRstColorXY_X;
	float nAvgY = rstAlgo->m_dRstColorXY_X;

	if (rstAlgo->m_bUseUV == TRUE)
	{
		if (nMinX > nAvgX || nMinY > nAvgY)		  // Low Coating
		{
			eWholeNgTypeTemp[TypeLowCoating] = e_NG;
			nCurrentNgType = TypeLowCoating;
		}
		else if (nMaxX < nAvgX || nMaxY < nAvgY) // High Coating
		{
			eWholeNgTypeTemp[TypeHighCoating] = e_NG;
			nCurrentNgType = TypeHighCoating;
		}
	}
	else
	{
		eWholeNgTypeTemp[TypeMountWorng] = e_NG;
		nCurrentNgType = TypeMountWorng;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoColorXY::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoColorXY::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
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
	pColorTeach->SetInspParam(sInspAlgo, sWndAlgoImg, coordinateAlgo, pInspBoardInfo, stAlgoParam.m_nLeadTipPos, rcLeadInsp, sWndAlgoImg.m_nLight_index, sInspImageData);
	for (int a = 0; a < 3; a++)
	{
		pColorTeach->m_sLightData[a].m_byLightCnt = g_pInspMng->m_sLightData[a].m_byLightCnt;
		for (int b = 0; b < LIGHT_DATA_CNT; b++)
		{
			for (int c = 0; c < LIGHT_CNT; c++)
				pColorTeach->m_sLightData[a].m_byArrLightData[b][c] = g_pInspMng->m_sLightData[a].m_byArrLightData[b][c];
		}
	}

	bResult = pColorTeach->InspColorXY(sInspAlgo, coordinateAlgo, pInspBoardInfo, (RstAlgoColorXY *)sRstAlgo, stAlgoParam.m_byWndInspType);

	g_pMManager->pem_delete(rcLeadInsp, false);

	return bResult;
}

BOOL CPInsp_AlgoColorXY::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoColorXY::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoColorXY::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}