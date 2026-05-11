#include "PInsp_AlgoTabSearch.h"


CPInsp_AlgoTabSearch::CPInsp_AlgoTabSearch(void)
{
}


CPInsp_AlgoTabSearch::~CPInsp_AlgoTabSearch(void)
{
}

void CPInsp_AlgoTabSearch::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoTabSearch::GetInspAlgoData()
{
	return -1;
}

int CPInsp_AlgoTabSearch::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	return nCurrentNgType;
}

bool CPInsp_AlgoTabSearch::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoTabSearch::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	stAlgoParam.m_rcArrTab->RemoveAll();
	stAlgoParam.m_rcArrTabBridge->RemoveAll();
	if (stAlgoParam.m_bTeach)
	{
		int nLeadCnt = 0;
		int dLeasStartPos = 0.0;
		CRect rcArrLeadRect[1000];
		LeadInfo infoResultLead;

		int nDir = 0;
		AlgoLeadSearch *pInspAlgoSearch = (AlgoLeadSearch *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgoSearch)
			nDir = pInspAlgoSearch->nLeadPosition;

		if (nDir == e_LEFT)
			pInspAlgoSearch->nLeadPosition = e_RIGHT;
		else if (nDir == e_TOP)
			pInspAlgoSearch->nLeadPosition = e_BOTTOM;

		TeachSearch(sInspAlgo, sWndAlgoImg, NULL, &infoResultLead, nLeadCnt, rcArrLeadRect, TRUE, dLeasStartPos);

		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;

		if ((nDir == e_LEFT) || (nDir == e_RIGHT))
		{
			swap(nImgWidth, nImgHeight);
		}

		for (int nROI = 0; nROI < nLeadCnt; ++nROI)
		{
			CRect rcROI1 = rcArrLeadRect[nROI];
			rcROI1.OffsetRect(-nImgWidth / 2, -nImgHeight / 2);

			CRect rcROI2 = rcArrLeadRect[nROI + 1];
			rcROI2.OffsetRect(-nImgWidth / 2, -nImgHeight / 2);

			if ((abs(rcROI1.left - rcROI2.left) < 2)
				&& (abs(rcROI1.right - rcROI2.right) < 2))
			{
				rcROI1.UnionRect(rcROI1, rcROI2);
				++nROI;
				rcROI2 = rcArrLeadRect[nROI + 1];
				rcROI2.OffsetRect(-nImgWidth / 2, -nImgHeight / 2);
			}

			if (nROI < nLeadCnt - 1)
			{
				CRect rcBrige(rcROI1.right + 4, rcROI1.top/*MAX(rcROI1.top, rcROI2.top)*/,
					rcROI2.left - 4, rcROI1.bottom/*MIN(rcROI1.bottom, rcROI2.bottom)*/);
				rcBrige = CalcRect_byDirection(rcBrige, nDir);
				stAlgoParam.m_rcArrTabBridge->Add(rcBrige);
			}

			rcROI1 = CalcRect_byDirection(rcROI1, nDir);
			stAlgoParam.m_rcArrTab->Add(rcROI1);
		}
	}
	//
	bResult = TRUE;

	return bResult;
}

BOOL CPInsp_AlgoTabSearch::TeachSearch(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* pUcArrRetResultImg, LeadInfo* ptrResultLeadIngo, int &nLeadCnt, CRect* pArrLeadRect, BOOL bTabSearch, double dLeadStartPos)
{
	BOOL bResult = FALSE;

	if ((sInspAlgo.m_eAlgoType != eAlgoLead_Search)
		&& (sInspAlgo.m_eAlgoType != eAlgoTab_Search)
		&& (sInspAlgo.m_eAlgoType != eAlgoColorBand_Search))
		return bResult;

	if ((!m_pProcMilAlgo)/* || (pUcArrRetResultImg == NULL)*/)
		return bResult;

	AlgoLeadSearch *pInspAlgoSearch = (AlgoLeadSearch *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoSearch)
		return bResult;

	bResult = g_pInspMng->GetPtrInspAlgo()->m_pInspLeadSolder.LeadSolderTeach(pInspAlgoSearch, sWndAlgoImg, pUcArrRetResultImg, ptrResultLeadIngo, nLeadCnt, pArrLeadRect, bTabSearch, dLeadStartPos);

	return bResult;
}

CRect CPInsp_AlgoTabSearch::CalcRect_byDirection(CRect rcROI, int nDir)
{
	CRect rcTmpROI = rcROI;
	switch (nDir)
	{
	case e_TOP:
		rcROI.left = -rcTmpROI.right;
		rcROI.top = -rcTmpROI.bottom;
		rcROI.right = -rcTmpROI.left;
		rcROI.bottom = -rcTmpROI.top;
		break;

	case e_LEFT:
		rcROI.left = -rcTmpROI.bottom;
		rcROI.top = rcTmpROI.left;
		rcROI.right = -rcTmpROI.top;
		rcROI.bottom = rcTmpROI.right;
		break;

	case e_BOTTOM:
		rcROI.left = rcTmpROI.left;
		rcROI.top = rcTmpROI.top;
		rcROI.right = rcTmpROI.right;
		rcROI.bottom = rcTmpROI.bottom;
		break;

	case e_RIGHT:
		rcROI.left = rcTmpROI.top;
		rcROI.top = -rcTmpROI.right;
		rcROI.right = rcTmpROI.bottom;
		rcROI.bottom = -rcTmpROI.left;
		break;
	}

	return rcROI;
}

BOOL CPInsp_AlgoTabSearch::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoTabSearch::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoTabSearch::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}