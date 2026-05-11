#include "PInsp_AlgoLeadSearch.h"


CPInsp_AlgoLeadSearch::CPInsp_AlgoLeadSearch(void)
{
}


CPInsp_AlgoLeadSearch::~CPInsp_AlgoLeadSearch(void)
{
}

void CPInsp_AlgoLeadSearch::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoLeadSearch::GetInspAlgoData()
{
	return -1;
}

int CPInsp_AlgoLeadSearch::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	return nCurrentNgType;
}

bool CPInsp_AlgoLeadSearch::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoLeadSearch::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = TRUE;

	AlgoLeadSearch *pInspAlgoSearch = (AlgoLeadSearch *)sInspAlgo.m_ptrInspAlgoParam;
	if (!m_pProcMilAlgo || !pInspAlgoSearch ||
		(sInspAlgo.m_eAlgoType != eAlgoLead_Search && sInspAlgo.m_eAlgoType != eAlgoTab_Search && sInspAlgo.m_eAlgoType != eAlgoColorBand_Search))
		return bResult;

	if (stAlgoParam.m_bInspection == FALSE)
	{
		LeadInfo *ptrResultLeadIngo = (LeadInfo *)sRstAlgo;
		if (!ptrResultLeadIngo)
			return bResult;

		double dStartPos = 0.0;
		if (pInspAlgoSearch->m_bIsManualType)
		{
			if (pInspAlgoSearch->nLeadPosition == 0)
				dStartPos = (stAlgoParam.m_sWndInfo->dLength / 2) - (stAlgoParam.m_sAlignWndInfo->dCenterY - stAlgoParam.m_sWndInfo->dCenterY) - pInspAlgoSearch->m_dLeadStartLeft_AlignCenterDstX - (pInspAlgoSearch->m_dLeadWidth / 2);
			else if (pInspAlgoSearch->nLeadPosition == 1)
				dStartPos = (stAlgoParam.m_sWndInfo->dLength / 2) + (stAlgoParam.m_sAlignWndInfo->dCenterY - stAlgoParam.m_sWndInfo->dCenterY) - pInspAlgoSearch->m_dLeadStartLeft_AlignCenterDstX - (pInspAlgoSearch->m_dLeadWidth / 2);
			else if (pInspAlgoSearch->nLeadPosition == 2)
				dStartPos = (stAlgoParam.m_sWndInfo->dWidth / 2) - (stAlgoParam.m_sAlignWndInfo->dCenterX - stAlgoParam.m_sWndInfo->dCenterX) - pInspAlgoSearch->m_dLeadStartLeft_AlignCenterDstX - (pInspAlgoSearch->m_dLeadWidth / 2);
			else if (pInspAlgoSearch->nLeadPosition == 3)
				dStartPos = (stAlgoParam.m_sWndInfo->dWidth / 2) + (stAlgoParam.m_sAlignWndInfo->dCenterX - stAlgoParam.m_sWndInfo->dCenterX) - pInspAlgoSearch->m_dLeadStartLeft_AlignCenterDstX - (pInspAlgoSearch->m_dLeadWidth / 2);
		}

		bResult = TeachSearch(sInspAlgo, sWndAlgoImg, ucArrDstImg, ptrResultLeadIngo, *stAlgoParam.m_nReviewMin, ptrResultLeadIngo->m_rcArrLead, stAlgoParam.m_bTeach, dStartPos);
	}

	return bResult;
}

BOOL CPInsp_AlgoLeadSearch::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoLeadSearch::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}

BOOL CPInsp_AlgoLeadSearch::TeachSearch(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* pUcArrRetResultImg, LeadInfo* ptrResultLeadIngo, int &nLeadCnt, CRect* pArrLeadRect, BOOL bTabSearch, double dLeadStartPos)
{
	BOOL bResult = FALSE;

	AlgoLeadSearch *pInspAlgoSearch = (AlgoLeadSearch *)sInspAlgo.m_ptrInspAlgoParam;
	bResult = g_pInspMng->GetPtrInspAlgo()->m_pInspLeadSolder.LeadSolderTeach(pInspAlgoSearch, sWndAlgoImg, pUcArrRetResultImg, ptrResultLeadIngo, nLeadCnt, pArrLeadRect, bTabSearch, dLeadStartPos);

	return bResult;
}
bool CPInsp_AlgoLeadSearch::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}