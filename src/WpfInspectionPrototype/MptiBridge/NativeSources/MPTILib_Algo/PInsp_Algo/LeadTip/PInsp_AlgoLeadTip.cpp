#include "PInsp_AlgoLeadTip.h"


CPInsp_AlgoLeadTip::CPInsp_AlgoLeadTip(void)
{
}


CPInsp_AlgoLeadTip::~CPInsp_AlgoLeadTip(void)
{
}

void CPInsp_AlgoLeadTip::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoLeadTip::GetInspAlgoData()
{
	return eSPCAlgoLead_Tip;
}

int CPInsp_AlgoLeadTip::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoLeadTip *rstLeadTip = (RstAlgoLeadTip *)vRstInspAlgo;
	bTipFaultNG = rstLeadTip->m_bTipFaultNG;

	if (!rstLeadTip->m_bSideTipOK || !rstLeadTip->m_bSideTipShiftXOK || !rstLeadTip->m_bSideTipShiftYOK)
	{
		eWholeNgTypeTemp[TypeLeadShift] = e_NG;
		eWholeNgType[TypeLeadShift] = e_NG;
		nCurrentNgType = TypeLeadShift;
	}

	if (!rstLeadTip->m_bRstFindOK)
	{
		eWholeNgTypeTemp[TypeLeadTip] = e_NG;
		eWholeNgType[TypeLeadTip] = e_NG;
		nCurrentNgType = TypeLeadTip;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoLeadTip::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoLeadTip::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	if (stAlgoParam.m_bInspection == TRUE)
	{
		float fHeightMean = -100.0f;
		stAlgoParam.m_pfHeightMean = &fHeightMean;
	}
	bResult = TeachTip(sInspAlgo, *stAlgoParam.m_sWndInfo, sWndAlgoImg, stAlgoParam.m_dx, stAlgoParam.m_dy, (RstAlgoLeadTip *)sRstAlgo, stAlgoParam.m_bTeach, *stAlgoParam.m_pfHeightMean);

	if (stAlgoParam.m_bInspection == TRUE)
	{
		RstAlgoLeadTip * rst = (RstAlgoLeadTip *)sRstAlgo;
		for (int nIdx = 0; nIdx < 2; nIdx++)
		{
			rst->m_poDrawLine_T[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine_T[nIdx].y += stAlgoParam.m_dy;
			rst->m_poDrawLine_I[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine_I[nIdx].y += stAlgoParam.m_dy;
		}
		// bTipFaultNG 가 true이면 윈도우 결과 NG로 처리
		AlgoLeadTip *pInspAlgoLeadTip = (AlgoLeadTip *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgoLeadTip->bTipFaultNG && bResult == false)
			stAlgoParam.m_pnRet = (int)e_NG;

		stAlgoParam.m_sInspAlgoTempResult->m_ptrnLeadTipPosResult[stAlgoParam.m_nLeadTipPosIndex] = rst->m_nRstLeadTipPos;
		if (rst->m_bRstFindOK == false)
		{
			int nLeadTipPosBuf = stAlgoParam.m_sInspAlgoTempResult->m_ptrnLeadTipPosResult[stAlgoParam.m_nLeadTipPosIndex];
			if (pInspAlgoLeadTip->m_nNGTipOption == 1)
			{
				if (stAlgoParam.m_nWndIndex != 0)
				{
					if (stAlgoParam.m_sInspAlgoTempResult->m_ptrnLeadTipPosResult[stAlgoParam.m_nLeadTipPosIndex - 1] > 0)
					{
						nLeadTipPosBuf = stAlgoParam.m_sInspAlgoTempResult->m_ptrnLeadTipPosResult[stAlgoParam.m_nLeadTipPosIndex - 1];
						stAlgoParam.m_sInspAlgoTempResult->m_ptrnLeadTipPosResult[stAlgoParam.m_nLeadTipPosIndex] = nLeadTipPosBuf;
					}
				}
			}
			else if (pInspAlgoLeadTip->m_nNGTipOption == 2)
			{
				for (int n = 0; n < stAlgoParam.m_nLeadTipPosIndex; n++)
				{
					if (stAlgoParam.m_sInspAlgoTempResult->m_ptrnLeadTipPosResult[n] <= 0)
						continue;
					nLeadTipPosBuf += stAlgoParam.m_sInspAlgoTempResult->m_ptrnLeadTipPosResult[n];
					nLeadTipPosBuf /= 2;
				}
			}
			rst->m_nRstLeadTipPos = nLeadTipPosBuf;
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoLeadTip::SearchSideTipPosition(AlgoLeadTip *pInspAlgoLeadTip, LeadTipSideTip sSideTip, RstLeadTipSideTip *sRst)
{
	BOOL bResult = FALSE;
	int nSideTipPos = pInspAlgoLeadTip->m_fSideTipPos / m_resolX;
	if (pInspAlgoLeadTip->m_bSideTip_TipFind == TRUE)
	{
		if (SearchSideTipFind(pInspAlgoLeadTip, sSideTip, sRst) == TRUE)
			nSideTipPos = sRst->m_nSideTip;
	}
	int nSideTipWidth = 0;
	int nSideTipHeight = 0;
	int nSearchSize = 0;
	int nSideTipStartX = 0;
	int nSideTipStartY = 0;
	int nLeadTipPos = 0;
	int nSideTipDir[2] = { 2,3 };
	double dSideCenter = 0;
	int nSideTipGap = pInspAlgoLeadTip->m_fSideTipGap / m_resolX;
	if (nSideTipPos <= 0)	nSideTipPos = pInspAlgoLeadTip->nLeadTipPos;
	if (nSideTipGap <= 0)	nSideTipGap = pInspAlgoLeadTip->nSearchRange;
	float fGapWidth = pInspAlgoLeadTip->m_fSideTipGapWidth;
	int nGapWidth = fGapWidth / m_resolX * 2;
	if (nGapWidth < 0) nGapWidth = 0;
	int nOffset = 0;
	if (pInspAlgoLeadTip->nLeadDirection == 0 || pInspAlgoLeadTip->nLeadDirection == 1)	// 0 = left, 1 = right
	{
		if (nGapWidth > sSideTip.nImgHeight) nGapWidth = sSideTip.nImgHeight;
		nSearchSize = sSideTip.nImgHeight * (pInspAlgoLeadTip->m_dSideTipSearchArea / 100.0);
		nSideTipWidth = nSideTipGap;
		nSideTipHeight = nSearchSize * 2 + (sSideTip.nImgHeight - nGapWidth);
		if (pInspAlgoLeadTip->nLeadDirection == 0)
		{
			nLeadTipPos = sRst->nStartX + (sSideTip.nImgWidth - nSideTipPos);
			nSideTipStartX = nLeadTipPos;
		}
		else
		{
			nLeadTipPos = sRst->nStartX + nSideTipPos;
			nSideTipStartX = nLeadTipPos - nSideTipWidth;
		}
		nSideTipStartY = sRst->nStartY + (nGapWidth / 2);// - nSearchSize;
		if (nSideTipStartX < 0)
			nSideTipStartX = 0;
		if (nSideTipStartY < 0)
			nSideTipStartY = 0;
		if (nSideTipStartX + nSideTipWidth > sSideTip.nPartImgWidth)
			nSideTipWidth = sSideTip.nPartImgWidth - nSideTipStartX;
		if (nSideTipStartY + nSideTipHeight > sSideTip.nPartImgHeight)
			nSideTipHeight = sSideTip.nPartImgHeight - nSideTipStartY;
		if (nSideTipWidth < 0)
			nSideTipWidth = 0;
		if (nSideTipHeight < 0)
			nSideTipHeight = 0;
		if (nSideTipWidth > sSideTip.nPartImgWidth)
			nSideTipWidth = sSideTip.nPartImgWidth - 1;
		if (nSideTipHeight > sSideTip.nPartImgHeight)
			nSideTipHeight = sSideTip.nPartImgHeight - 1;
		dSideCenter = nSideTipHeight / 2.0;
		nOffset = (sSideTip.nPartImgHeight - nSideTipHeight) / 2.0;
	}
	else
	{
		if (nGapWidth > sSideTip.nImgWidth) nGapWidth = sSideTip.nImgWidth;
		nSideTipDir[0] = 0;
		nSideTipDir[1] = 1;
		nSearchSize = sSideTip.nImgWidth * (pInspAlgoLeadTip->m_dSideTipSearchArea / 100.0);
		nSideTipWidth = nSearchSize * 2 + (sSideTip.nImgWidth - nGapWidth);
		nSideTipHeight = nSideTipGap;
		nSideTipStartX = sRst->nStartX + (nGapWidth / 2);// - nSearchSize;
		if (pInspAlgoLeadTip->nLeadDirection == 2)
		{
			nLeadTipPos = sRst->nStartY + (sSideTip.nImgHeight - nSideTipPos);
			nSideTipStartY = nLeadTipPos;
		}
		else
		{
			nLeadTipPos = sRst->nStartY + nSideTipPos;
			nSideTipStartY = nLeadTipPos - nSideTipHeight;
		}
		if (nSideTipStartX < 0)
			nSideTipStartX = 0;
		if (nSideTipStartY < 0)
			nSideTipStartY = 0;
		if (nSideTipStartX + nSideTipWidth > sSideTip.nPartImgWidth)
			nSideTipWidth = sSideTip.nPartImgWidth - nSideTipStartX;
		if (nSideTipStartY + nSideTipHeight > sSideTip.nPartImgHeight)
			nSideTipHeight = sSideTip.nPartImgHeight - nSideTipStartY;
		if (nSideTipWidth < 0)
			nSideTipWidth = 0;
		if (nSideTipHeight < 0)
			nSideTipHeight = 0;
		if (nSideTipWidth > sSideTip.nPartImgWidth)
			nSideTipWidth = sSideTip.nPartImgWidth - 1;
		if (nSideTipHeight > sSideTip.nPartImgHeight)
			nSideTipHeight = sSideTip.nPartImgHeight - 1;
		dSideCenter = nSideTipWidth / 2.0;
		nOffset = (sSideTip.nPartImgWidth - nSideTipWidth) / 2.0;
	}
	if (nOffset < 0) nOffset = 0;

	UCHAR *pucSideTip = NULL;
	float *pfSideTip = NULL;
	if (nSideTipWidth > 0 && nSideTipHeight > 0 && nSideTipStartX >= 0 && nSideTipStartY >= 0)
	{
		/*pucSideTip = new UCHAR[nSideTipWidth * nSideTipHeight];
		pfSideTip  = new float[nSideTipWidth * nSideTipHeight];*/
		pucSideTip = g_pMManager->pem_new<UCHAR>(true, nSideTipWidth * nSideTipHeight, (PCHAR)__FUNCTION__, __LINE__);
		pfSideTip = g_pMManager->pem_new<float>(true, nSideTipWidth * nSideTipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sSideTip.pucPartImgSrc, sSideTip.nPartImgWidth, sSideTip.nPartImgHeight, pucSideTip, nSideTipStartX, nSideTipStartY, nSideTipWidth, nSideTipHeight);
		m_proc3d.GetCropZmap_LT(sSideTip.pfPartImgSrc, pfSideTip, sSideTip.nPartImgWidth, sSideTip.nPartImgHeight, nSideTipStartX, nSideTipStartY, nSideTipWidth, nSideTipHeight);

		m_pProcMilAlgo->SaveWorkImg(pucSideTip, nSideTipWidth, nSideTipHeight, _T("Clip_SideTip_2DImage.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfSideTip, nSideTipWidth, nSideTipHeight, _T("Clip_SideTip_3DImage.bmp"));
		int nArrSideTipPos[2] = { 0,0 };
		BOOL bArrSideTipResult[2] = { FALSE, FALSE };
		BOOL bUseSide2D = pInspAlgoLeadTip->m_b2dCheck;
		BOOL bUseSide3D = pInspAlgoLeadTip->b3dCheck;
		int nSideTipType = 1;
		if (pInspAlgoLeadTip->m_bUseSideBin == TRUE)
		{
			bUseSide2D = FALSE;
			bUseSide3D = TRUE;
			nSideTipType = 2;
		}
		for (int n = 0; n < 2; n++)
		{
			if (bUseSide2D == TRUE)
			{
				bArrSideTipResult[n] = SearchTipPosition(pInspAlgoLeadTip, nSideTipType
					, dSideCenter, dSideCenter, nSideTipDir[n], pInspAlgoLeadTip->nPixelPercentValue
					, pucSideTip, pfSideTip, nSideTipWidth, nSideTipHeight, bUseSide2D, FALSE
					, nArrSideTipPos[n], sRst->fHeightMeanBuf);
			}
			if (bArrSideTipResult[n] == FALSE)
			{
				if (bUseSide3D == TRUE)
				{
					BOOL bUse2D = pInspAlgoLeadTip->m_b3DIn2dCheck;
					BOOL bUse3D = pInspAlgoLeadTip->b3dCheck;
					if (pInspAlgoLeadTip->m_bUseSideBin == TRUE)
					{
						bUse2D = pInspAlgoLeadTip->m_b2dCheck_Side;
						bUse3D = pInspAlgoLeadTip->m_b3dCheck_Side;
					}
					bArrSideTipResult[n] = SearchTipPosition(pInspAlgoLeadTip, nSideTipType
						, dSideCenter, dSideCenter, nSideTipDir[n], pInspAlgoLeadTip->nPixelPercentValue
						, pucSideTip, pfSideTip, nSideTipWidth, nSideTipHeight, bUse2D, bUse3D
						, nArrSideTipPos[n], sRst->fHeightMeanBuf);
				}
			}
			if (bArrSideTipResult[n] == TRUE)
				sRst->m_bSideTipResult = TRUE;
		}
		if (nArrSideTipPos[0] < nArrSideTipPos[1])
		{
			double dRstSideTipCenter = (nArrSideTipPos[0] + nArrSideTipPos[1]) / 2.0;
			if (pInspAlgoLeadTip->nLeadDirection == 0 || pInspAlgoLeadTip->nLeadDirection == 1)
			{
				sRst->m_dRstShiftY = (dSideCenter - dRstSideTipCenter) * m_resolY;
				sRst->m_dRstShiftYPer = sRst->m_dRstShiftY / (sSideTip.nImgHeight * m_resolY) * 100;
				if (bArrSideTipResult[0] == TRUE)
					sRst->m_dRstSideStartPos = (nSideTipStartY + nArrSideTipPos[0]) * m_resolY;
				else if (bArrSideTipResult[1] == TRUE)
					sRst->m_dRstSideStartPos = (nSideTipStartY + nArrSideTipPos[1]) * m_resolY;
			}
			else
			{
				sRst->m_dRstShiftX = (dSideCenter - dRstSideTipCenter) * m_resolX;
				sRst->m_dRstShiftXPer = sRst->m_dRstShiftX / (sSideTip.nImgWidth * m_resolX) * 100;
				if (bArrSideTipResult[0] == TRUE)
					sRst->m_dRstSideStartPos = (nSideTipStartX + nArrSideTipPos[0]) * m_resolX;
				else if (bArrSideTipResult[1] == TRUE)
					sRst->m_dRstSideStartPos = (nSideTipStartX + nArrSideTipPos[1]) * m_resolX;
			}
			sRst->nSideTipLength = nArrSideTipPos[1] - nArrSideTipPos[0];
			if (sRst->nSideTipLength < 0)
				sRst->nSideTipLength = sRst->nSideTipLength * -1;
			sRst->m_dRstSideWidth = sqrt(pow(((nArrSideTipPos[0] * m_resolX) - (nArrSideTipPos[1] * m_resolX)), 2));

			if (pInspAlgoLeadTip->nLeadDirection == 0 || pInspAlgoLeadTip->nLeadDirection == 1)	// 0 = left, 1 = right
			{
				sRst->nStartY = nArrSideTipPos[0] + nSideTipStartY;
				sRst->nStartY3D = nArrSideTipPos[0] + nSideTipStartY;
				sRst->m_poDrawLine_Side[0].x = sSideTip.dWndX + (nSideTipStartX + nSideTipWidth / 2.0);
				sRst->m_poDrawLine_Side[0].y = sSideTip.dWndY + nArrSideTipPos[0] + nOffset;
				sRst->m_poDrawLine_Side[1].x = sSideTip.dWndX + (nSideTipStartX + nSideTipWidth / 2.0);
				sRst->m_poDrawLine_Side[1].y = sSideTip.dWndY + nArrSideTipPos[1] + nOffset;
				if (pInspAlgoLeadTip->m_bSideTip_TipFind == TRUE)
				{
					sRst->m_poDrawLine_Side[0].x = sSideTip.dWndX + nSideTipStartX;
					sRst->m_poDrawLine_Side[1].x = sSideTip.dWndX + nSideTipStartX;
					if (pInspAlgoLeadTip->nLeadDirection == 1)
					{
						sRst->m_poDrawLine_Side[0].x += nSideTipWidth;
						sRst->m_poDrawLine_Side[1].x += nSideTipWidth;
					}
				}
			}
			else
			{
				sRst->nStartX = nArrSideTipPos[0] + nSideTipStartX;
				sRst->nStartX3D = nArrSideTipPos[0] + nSideTipStartX;
				sRst->m_poDrawLine_Side[0].x = sSideTip.dWndX + nArrSideTipPos[0] + nOffset;
				sRst->m_poDrawLine_Side[0].y = sSideTip.dWndY + (nSideTipStartY + nSideTipHeight / 2.0);
				sRst->m_poDrawLine_Side[1].x = sSideTip.dWndX + nArrSideTipPos[1] + nOffset;
				sRst->m_poDrawLine_Side[1].y = sSideTip.dWndY + (nSideTipStartY + nSideTipHeight / 2.0);
				if (pInspAlgoLeadTip->m_bSideTip_TipFind == TRUE)
				{
					sRst->m_poDrawLine_Side[0].y = sSideTip.dWndY + nSideTipStartY;
					sRst->m_poDrawLine_Side[1].y = sSideTip.dWndY + nSideTipStartY;
					if (pInspAlgoLeadTip->nLeadDirection == 3)
					{
						sRst->m_poDrawLine_Side[0].y += nSideTipHeight;
						sRst->m_poDrawLine_Side[1].y += nSideTipHeight;
					}
				}
			}
		}
		else
		{
			sRst->m_bSideTipResult = FALSE;
			sRst->m_dRstShiftX = 0;
			sRst->m_dRstShiftY = 0;
			sRst->m_dRstSideWidth = 0;
			sRst->m_dRstSideStartPos = 0;
			sRst->nStartX = 0;
			sRst->nStartY = 0;
			sRst->m_dRstShiftXPer = 0;
			sRst->m_dRstShiftYPer = 0;
			if (pInspAlgoLeadTip->nLeadDirection == 0 || pInspAlgoLeadTip->nLeadDirection == 1)	// 0 = left, 1 = right
			{
				sRst->nStartY = (sSideTip.nPartImgHeight / 2) - (sSideTip.nImgHeight / 2);
				if (sRst->nStartY < 0) sRst->nStartY = 0;
				sRst->nStartY3D = sRst->nStartY;
			}
			else
			{
				sRst->nStartX = (sSideTip.nPartImgWidth / 2) - (sSideTip.nImgWidth / 2);
				if (sRst->nStartX < 0) sRst->nStartX = 0;
				sRst->nStartX3D = sRst->nStartX;
			}
		}
		Delete_1DArray(&pucSideTip);
		Delete_1DArray(&pfSideTip);
	}
	bResult = TRUE;
	return bResult;
}
BOOL CPInsp_AlgoLeadTip::SearchSideTipFind(AlgoLeadTip *pInspAlgoLeadTip, LeadTipSideTip sSideTip, RstLeadTipSideTip *sRst)
{
	BOOL bResult = FALSE;
	sRst->m_nSideTip = 0;
	int nResultPos = 0;
	int nSideTipWidth = 0;
	int nSideTipHeight = 0;
	int nSideTipStartX = 0;
	int nSideTipStartY = 0;

	int nSideTipPos = pInspAlgoLeadTip->m_fSideTipPos / m_resolX;
	int nSideTipGap = pInspAlgoLeadTip->m_fSideTipGap / m_resolX;
	int nGapWidth = pInspAlgoLeadTip->m_fSideTipGapWidth / m_resolX * 2.0;
	int nLeadTipPos = 0;
	int nSearchSize = 0;
	if (nSideTipPos <= 0)	nSideTipPos = pInspAlgoLeadTip->nLeadTipPos;
	if (nSideTipGap <= 0)	nSideTipGap = pInspAlgoLeadTip->nSearchRange;
	if (nGapWidth < 0) nGapWidth = 0;
	if (pInspAlgoLeadTip->nLeadDirection == 0 || pInspAlgoLeadTip->nLeadDirection == 1)	// 0 = left, 1 = right
	{
		if (nGapWidth > sSideTip.nImgHeight) nGapWidth = sSideTip.nImgHeight;
		nSearchSize = sSideTip.nImgHeight * (pInspAlgoLeadTip->m_dSideTipSearchArea / 100.0);
		nSideTipWidth = nSideTipGap * 2;
		nSideTipHeight = sSideTip.nImgHeight - nGapWidth;
		if (pInspAlgoLeadTip->nLeadDirection == 0)
		{
			nLeadTipPos = sRst->nStartX + (sSideTip.nImgWidth - nSideTipPos);
			nSideTipStartX = nLeadTipPos - nSideTipGap;
		}
		else
		{
			nLeadTipPos = sRst->nStartX + nSideTipPos;
			nSideTipStartX = nLeadTipPos - nSideTipGap;
		}
		nSideTipStartY = sRst->nStartY + (nGapWidth / 2) + nSearchSize;
	}
	else
	{
		if (nGapWidth > sSideTip.nImgWidth) nGapWidth = sSideTip.nImgWidth;
		nSearchSize = sSideTip.nImgWidth * (pInspAlgoLeadTip->m_dSideTipSearchArea / 100.0);
		nSideTipWidth = sSideTip.nImgWidth - nGapWidth;
		nSideTipHeight = nSideTipGap * 2;
		nSideTipStartX = sRst->nStartX + (nGapWidth / 2) + nSearchSize;
		if (pInspAlgoLeadTip->nLeadDirection == 2)
		{
			nLeadTipPos = sRst->nStartY + (sSideTip.nImgHeight - nSideTipPos);
			nSideTipStartY = nLeadTipPos - nSideTipGap;
		}
		else
		{
			nLeadTipPos = sRst->nStartY + nSideTipPos;
			nSideTipStartY = nLeadTipPos - nSideTipGap;
		}
	}
	if (nSideTipStartX < 0)
		nSideTipStartX = 0;
	if (nSideTipStartY < 0)
		nSideTipStartY = 0;
	if (nSideTipStartX + nSideTipWidth > sSideTip.nPartImgWidth)
		nSideTipWidth = sSideTip.nPartImgWidth - nSideTipStartX;
	if (nSideTipStartY + nSideTipHeight > sSideTip.nPartImgHeight)
		nSideTipHeight = sSideTip.nPartImgHeight - nSideTipStartY;
	if (nSideTipWidth < 0)
		nSideTipWidth = 0;
	if (nSideTipHeight < 0)
		nSideTipHeight = 0;
	if (nSideTipWidth > sSideTip.nPartImgWidth)
		nSideTipWidth = sSideTip.nPartImgWidth - 1;
	if (nSideTipHeight > sSideTip.nPartImgHeight)
		nSideTipHeight = sSideTip.nPartImgHeight - 1;

	if (nSideTipWidth <= 0 || nSideTipHeight < 0 || nSideTipStartX < 0 || nSideTipStartY < 0)
		return bResult;

	/*UCHAR *pucSideTip = new UCHAR[nSideTipWidth * nSideTipHeight];
	float *pfSideTip  = new float[nSideTipWidth * nSideTipHeight];*/
	UCHAR *pucSideTip = g_pMManager->pem_new<UCHAR>(true, nSideTipWidth * nSideTipHeight, (PCHAR)__FUNCTION__, __LINE__);
	float *pfSideTip = g_pMManager->pem_new<float>(true, nSideTipWidth * nSideTipHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(pucSideTip, 0, nSideTipWidth * nSideTipHeight * sizeof(UCHAR));
	memset(pfSideTip, 0, nSideTipWidth * nSideTipHeight * sizeof(float));
	m_pProcMilAlgo->SaveWorkImg(sSideTip.pucPartImgSrc, sSideTip.nPartImgWidth, sSideTip.nPartImgHeight, _T("Clip_SideFindTip_2DImage_ORG.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(sSideTip.pfPartImgSrc, sSideTip.nPartImgWidth, sSideTip.nPartImgHeight, _T("Clip_SideFindTip_3DImage_ORG.bmp"));
	m_pProcMilAlgo->GetClipImage_LT(sSideTip.pucPartImgSrc, sSideTip.nPartImgWidth, sSideTip.nPartImgHeight, pucSideTip, nSideTipStartX, nSideTipStartY, nSideTipWidth, nSideTipHeight);
	m_proc3d.GetCropZmap_LT(sSideTip.pfPartImgSrc, pfSideTip, sSideTip.nPartImgWidth, sSideTip.nPartImgHeight, nSideTipStartX, nSideTipStartY, nSideTipWidth, nSideTipHeight);
	m_pProcMilAlgo->SaveWorkImg(pucSideTip, nSideTipWidth, nSideTipHeight, _T("Clip_SideFindTip_2DImage.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfSideTip, nSideTipWidth, nSideTipHeight, _T("Clip_SideFindTip_3DImage.bmp"));

	BOOL bUseSide2D = pInspAlgoLeadTip->m_b2dCheck;
	BOOL bUseSide3D = pInspAlgoLeadTip->b3dCheck;
	if (pInspAlgoLeadTip->m_bUseSideBin == TRUE)
	{
		bUseSide2D = FALSE;
		bUseSide3D = TRUE;
	}
	if (bUseSide2D == TRUE)
	{
		bResult = SearchTipPosition(pInspAlgoLeadTip, 3
			, nSideTipGap, nSideTipGap, pInspAlgoLeadTip->nLeadDirection, pInspAlgoLeadTip->nPixelPercentValue
			, pucSideTip, pfSideTip, nSideTipWidth, nSideTipHeight, bUseSide2D, FALSE
			, nResultPos, sRst->fHeightMeanBuf);
	}
	if (bResult == FALSE)
	{
		if (bUseSide3D == TRUE)
		{
			BOOL bUse2D = pInspAlgoLeadTip->m_b3DIn2dCheck;
			BOOL bUse3D = pInspAlgoLeadTip->b3dCheck;
			bResult = SearchTipPosition(pInspAlgoLeadTip, 0
				, nSideTipGap, nSideTipGap, pInspAlgoLeadTip->nLeadDirection, pInspAlgoLeadTip->nPixelPercentValue
				, pucSideTip, pfSideTip, nSideTipWidth, nSideTipHeight, bUse2D, bUse3D
				, nResultPos, sRst->fHeightMeanBuf);
		}
	}
	if (bResult == TRUE)
	{
		int nGap = nSideTipGap - nResultPos;
		sRst->m_nSideTip = nSideTipPos - nGap;
	}
	Delete_1DArray(&pucSideTip);
	Delete_1DArray(&pfSideTip);
	return bResult;
}
BOOL CPInsp_AlgoLeadTip::SearchTipPosition(AlgoLeadTip *pInspAlgoLeadTip, int nSearchType
	, int nSearchRange, int nLeadTipPos, int nLeadDirection, int nPixelPercentValue
	, UCHAR *pucImgSrc, float *pfImgSrc, int nImgWidth, int nImgHeight, BOOL bUse2D, BOOL bUse3D
	, int &nResultPos, float &fHeigntMean, BOOL bDirInvert)
{
	BOOL bResult = FALSE;
	int nBodySeachDir = pInspAlgoLeadTip->m_bSearchDirBody;
	nBodySeachDir = 2;
	if (nSearchRange <= 0)
		nSearchRange = 1;
	if (nLeadTipPos <= 0)
		nLeadTipPos = nSearchRange + 1;

	double dPixPerValue1 = nPixelPercentValue;
	double dPixPerValue2 = nPixelPercentValue;
	int nType = 0;
	if (nSearchType == 1 || nSearchType == 2)
		nType = 1;
	if (nType == 1)
		dPixPerValue2 = 90;
	if (dPixPerValue1 >= 100)
		dPixPerValue1 = 99.9;
	if (dPixPerValue2 >= 100)
		dPixPerValue2 = 99.9;

	int nArrTipStartX[2] = { 0,0 };
	int nArrTipStartY[2] = { 0,0 };
	int nArrSearchDirBody[2] = { 0,0 };
	int nArrSearchValue[2] = { 0,0 };

	int nRsultYValue = 0;
	int nTipWidth = 0;
	int nTipHeight = 0;
	int nTipY = 0;
	int nTipX = 0;

	//Bin
	int n2DRange = pInspAlgoLeadTip->n2dRange;
	int n2DMin = pInspAlgoLeadTip->n2dBinaryMin;
	int n2DMax = pInspAlgoLeadTip->n2dBinaryMax;
	int n3DRange = pInspAlgoLeadTip->n3dRange;
	float f3DMin = (float)pInspAlgoLeadTip->n3dHeightMin;
	float f3DMax = (float)pInspAlgoLeadTip->n3dHeightMax;
	if (bUse3D == TRUE && pInspAlgoLeadTip->m_b3DIn2dCheck == true)
	{
		n2DRange = pInspAlgoLeadTip->m_n3DIn2DRange;
		n2DMin = pInspAlgoLeadTip->m_n3DIn2DMinValue;
		n2DMax = pInspAlgoLeadTip->m_n3DIn2DMaxValue;
	}
	if (nSearchType == 2 || nSearchType == 3)
	{
		n2DRange = pInspAlgoLeadTip->m_n2dRange_Side;
		n2DMin = pInspAlgoLeadTip->m_n2dBinaryMin_Side;
		n2DMax = pInspAlgoLeadTip->m_n2dBinaryMax_Side;
		n3DRange = pInspAlgoLeadTip->m_n3dRange_Side;
		f3DMin = pInspAlgoLeadTip->m_f3dHeightMin_Side;
		f3DMax = pInspAlgoLeadTip->m_f3dHeightMax_Side;
	}
	if (_isnan(f3DMin) == true)
		f3DMin = 0;
	if (_isnan(f3DMax) == true)
		f3DMax = 0;
	if ((nLeadDirection == 0) || (nLeadDirection == 1))	// 0 = left, 1 = right
	{
		if (nLeadTipPos >= nImgWidth)
			nLeadTipPos = nImgWidth / 2;

		nArrTipStartY[0] = 0;

		if (nType == 0)
		{
			if ((nLeadDirection == 0))
				nArrTipStartX[0] = nImgWidth - nLeadTipPos - 1;
			else
				nArrTipStartX[0] = nLeadTipPos + 1;

			nArrTipStartX[0] = nArrTipStartX[0] - nSearchRange;
		}
		else if (nType == 1)
			nArrTipStartX[0] = 0;

		if (nArrTipStartX[0] < 0)
			nArrTipStartX[0] = 0;
		nTipWidth = nSearchRange * 2;
		if (nTipWidth > nImgWidth)
			nTipWidth = nImgWidth;
		if (nArrTipStartX[0] + nTipWidth >= nImgWidth)
			nTipWidth = nTipWidth - (nArrTipStartX[0] + nTipWidth - nImgWidth);
		nTipHeight = nImgHeight;
		nTipY = nTipWidth;
		nTipX = nTipHeight;
		nArrSearchDirBody[0] = 1;

		if (nBodySeachDir == 2)
		{
			nArrTipStartY[1] = 0;

			if (nType == 0)
			{
				if ((nLeadDirection == 0))
					nArrTipStartX[1] = nImgWidth - nLeadTipPos - 1;
				else
					nArrTipStartX[1] = nLeadTipPos + 1;
				nArrTipStartX[1] = nArrTipStartX[1] - 5;
			}
			else
				nArrTipStartX[1] = 0;

			if (nArrTipStartX[1] < 0)
				nArrTipStartX[1] = 0;
			if (nArrTipStartX[1] > nImgWidth)
				nArrTipStartX[1] = nImgWidth;
			if (nArrTipStartX[1] + nTipWidth >= nImgWidth)
				nArrTipStartX[1] = nImgWidth - nTipWidth;
			nTipY = nTipWidth;

			if (nLeadDirection == 0)
			{
				nArrSearchDirBody[0] = (bDirInvert) ? 1 : 0;
				nArrSearchValue[0] = 0;

				nArrSearchDirBody[1] = (bDirInvert) ? 1 : 0;
				nArrSearchValue[1] = 0;
			}
			else if (nLeadDirection == 1)
			{
				nArrSearchDirBody[0] = (bDirInvert) ? 0 : 1;
				nArrSearchValue[0] = 0;

				nArrSearchDirBody[1] = (bDirInvert) ? 0 : 1;
				nArrSearchValue[1] = 0;
			}
		}

		if (nArrTipStartX[0] + nTipWidth > nImgWidth)
			nArrTipStartX[0] = nImgWidth - nTipWidth;
	}
	else	// 2 = top, 3 = bottom
	{
		if (nLeadTipPos >= nImgHeight)
			nLeadTipPos = nImgHeight / 2;

		nArrTipStartX[0] = 0;
		nTipWidth = nImgWidth;
		if (nType == 0)
		{
			if (nLeadDirection == 2)
				nArrTipStartY[0] = nImgHeight - nLeadTipPos;
			else
				nArrTipStartY[0] = nLeadTipPos;
			nArrTipStartY[0] = nArrTipStartY[0] - nSearchRange;
		}
		else if (nType == 1)
			nArrTipStartY[0] = 0;

		nTipHeight = nSearchRange * 2;
		if (nArrTipStartY[0] < 0)
			nArrTipStartY[0] = 0;
		if (nTipHeight > nImgHeight)
			nTipHeight = nImgHeight;
		if (nArrTipStartY[0] + nTipHeight >= nImgHeight)
			nTipHeight = nTipHeight - (nArrTipStartY[0] + nTipHeight - nImgHeight);
		nTipY = nTipHeight;
		nTipX = nTipWidth;
		nArrSearchDirBody[0] = 0;

		if (nBodySeachDir == 2)
		{
			nArrTipStartX[1] = 0;
			if (nType == 0)
			{
				if (nLeadDirection == 2)
					nArrTipStartY[1] = nImgHeight - nLeadTipPos;
				else
					nArrTipStartY[1] = nLeadTipPos;
				nArrTipStartY[1] = nArrTipStartY[1] - 5;
			}
			else if (nType == 1)
				nArrTipStartY[1] = 0;

			if (nArrTipStartY[1] < 0)
				nArrTipStartY[1] = 0;
			if (nArrTipStartY[1] > nImgHeight)
				nArrTipStartY[1] = nImgHeight;
			if (nArrTipStartY[1] + nTipHeight >= nImgHeight)
				nArrTipStartY[1] = nImgHeight - nTipHeight;
			nTipY = nTipHeight;

			if (nLeadDirection == 2)
			{
				nArrSearchDirBody[0] = (bDirInvert) ? 1 : 0;
				nArrSearchValue[0] = 0;
				nArrSearchDirBody[1] = (bDirInvert) ? 1 : 0;
				nArrSearchValue[1] = 0;
			}
			else if (nLeadDirection == 3)
			{
				nArrSearchDirBody[0] = (bDirInvert) ? 0 : 1;
				nArrSearchValue[0] = 0;
				nArrSearchDirBody[1] = (bDirInvert) ? 0 : 1;
				nArrSearchValue[1] = 0;
			}
		}

		if (nArrTipStartY[0] + nTipHeight > nImgHeight)
			nArrTipStartY[0] = nImgHeight - nTipHeight;
	}

	if (nBodySeachDir == 0)
	{
		if (nLeadDirection == 0)
			nArrSearchDirBody[0] = 0;
		else if (nLeadDirection == 3)
			nArrSearchDirBody[0] = 1;
		nArrSearchValue[0] = 0;
	}
	else if (nBodySeachDir == 1)
	{
		if (nLeadDirection == 1)
			nArrSearchDirBody[0] = 0;
		else if (nLeadDirection == 2)
			nArrSearchDirBody[0] = 1;
		nArrSearchValue[0] = 255;
	}

	if ((nArrTipStartX[0] < 0) || (nArrTipStartY[0] < 0)
		|| (nArrTipStartX[1] < 0) || (nArrTipStartY[1] < 0)
		|| (nTipWidth <= 0) || (nTipHeight <= 0))
		return bResult;

	int nIndex = 0;
	for (int n = 0; n < 2; n++)
	{
		bool bLeadFind = false;
		int nTopLeft = nArrSearchDirBody[n];
		int nStartX = nTipX;
		int nStartY = nTipY;

		UCHAR *pucTip2DImg = NULL;
		UCHAR *pucTip2DData = NULL;
		float *pucTip3DImg = NULL;
		UCHAR *pucTip3DData = NULL;
		if (bUse2D == TRUE)
		{
			if (pucImgSrc == NULL)
				return bResult;

			//pucTip2DImg = new UCHAR[nTipWidth * nTipHeight];
			pucTip2DImg = g_pMManager->pem_new<UCHAR>(true, nTipWidth * nTipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucTip2DImg, nArrTipStartX[n], nArrTipStartY[n], nTipWidth, nTipHeight);
			m_pCPInsp_Algo->Binarize(pucTip2DImg, nTipWidth, nTipHeight, n2DRange, n2DMin, n2DMax, pInspAlgoLeadTip->bInvertCheck, pucTip2DData);

			m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Origin2D.bmp"));
			m_pProcMilAlgo->SaveWorkImg(pucTip2DImg, nTipWidth, nTipHeight, _T("Clip2D.bmp"));
			m_pProcMilAlgo->SaveWorkImg(pucTip2DData, nTipWidth, nTipHeight, _T("Binary2D.bmp"));
		}
		if (bUse3D == TRUE)
		{
			if (pfImgSrc == NULL)
				return bResult;

			//pucTip3DImg = new float[nTipWidth * nTipHeight];
			pucTip3DImg = g_pMManager->pem_new<float>(true, nTipWidth * nTipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_proc3d.GetCropZmap_LT(pfImgSrc, pucTip3DImg, nImgWidth, nImgHeight, nArrTipStartX[n], nArrTipStartY[n], nTipWidth, nTipHeight);
			m_pCPInsp_Algo->Binarize(pucTip3DImg, nTipWidth, nTipHeight, n3DRange, f3DMin, f3DMax, pInspAlgoLeadTip->bInvertCheck, pucTip3DData);

			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Origin3D.bmp"));
			m_pProcMilAlgo->SaveWorkImg_float(pucTip3DImg, nTipWidth, nTipHeight, _T("Clip3D.bmp"));
			m_pProcMilAlgo->SaveWorkImg(pucTip3DData, nTipWidth, nTipHeight, _T("Bin3D.bmp"));
			if (fHeigntMean != -100.0f)
				fHeigntMean = m_pCPInsp_Algo->GetHeightMean(pucTip3DImg, nTipWidth, nTipHeight);
		}
		UCHAR *pucTipData = NULL;
		UCHAR *pucTipBlobData = NULL;
		/*pucTipData = new UCHAR[nTipWidth * nTipHeight];
		pucTipBlobData = new UCHAR[nTipWidth * nTipHeight];*/
		pucTipData = g_pMManager->pem_new<UCHAR>(true, nTipWidth * nTipHeight, (PCHAR)__FUNCTION__, __LINE__);
		pucTipBlobData = g_pMManager->pem_new<UCHAR>(true, nTipWidth * nTipHeight, (PCHAR)__FUNCTION__, __LINE__);
		if (bUse2D == TRUE && bUse3D == TRUE)
		{
			if (pucTip2DData && pucTip3DData)
			{
				for (int y = 0; y < nTipHeight; y++)
				{
					for (int x = 0; x < nTipWidth; x++)
					{
						int index = y * nTipWidth + x;

						pucTipData[index] = ((pucTip2DData[index] == 255) && (pucTip3DData[index] == 255)) ? 255 : 0;
					}
				}
			}
		}
		else
		{
			for (int y = 0; y < nTipHeight; y++)
			{
				for (int x = 0; x < nTipWidth; x++)
				{
					int index = y * nTipWidth + x;
					if (bUse2D == TRUE)
						pucTipData[index] = pucTip2DData[index];
					if (bUse3D == TRUE)
						pucTipData[index] = pucTip3DData[index];
				}
			}
		}
		m_pProcMilAlgo->SaveWorkImg(pucTipData, nTipWidth, nTipHeight, _T("MixBin_LeadTip.bmp"));
		int nCntBlob = 0;
		if (pInspAlgoLeadTip->m_nMinBlobArea > 0)
		{
			int nMinBlobArea = pInspAlgoLeadTip->m_nMinBlobArea;
			nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pucTipData, pucTipBlobData, nTipWidth, nTipHeight, nMinBlobArea, FALSE, FALSE, -1, eSelectMix);
		}
		if (pInspAlgoLeadTip->m_nMinBlobArea == 0 || nCntBlob == 0)
		{
			for (int y = 0; y < nTipHeight; y++)
			{
				for (int x = 0; x < nTipWidth; x++)
				{
					int index = y * nTipWidth + x;
					pucTipBlobData[index] = pucTipData[index];
				}
			}
		}
		m_pProcMilAlgo->SaveWorkImg(pucTipBlobData, nTipWidth, nTipHeight, _T("MixBlob_LeadTip.bmp"));
		if (nTopLeft == 1)
		{
			for (int y = nStartY - 1; y > 0; y--)
			{
				int nWhiteCnt = 0;
				int nResult = 0;
				for (int x = 0; x < nStartX; x++)
				{
					if ((nLeadDirection == 0) || (nLeadDirection == 1))
					{
						if (pucTipBlobData[y + (x * nTipWidth)] == nArrSearchValue[n])
							nWhiteCnt += 1;
					}
					else if ((nLeadDirection == 2) || (nLeadDirection == 3))
					{
						if (pucTipBlobData[x + (y * nTipWidth)] == nArrSearchValue[n])
							nWhiteCnt += 1;
					}
				}
				// ROI내에 흰색값이 설정값보다 아래일 경우 tip이라 간주
				double dBlackRate = (nWhiteCnt * 100.0) / (double)nStartX;
				//20141016 SHW : 이전줄이 흰색이 50%이상이고 다음줄이 50%이하일 경우 팁이라 간주
				//0번째 줄은 아니게 된다.
				if (bLeadFind == true)
				{
					if (dPixPerValue1 >= dBlackRate)
					{
						nRsultYValue = y;
						y = 0;
						bResult = TRUE;
					}
				}
				if (dPixPerValue2 <= dBlackRate)
					bLeadFind = true;
				else
				{
					if (nType == 0)
						bLeadFind = false;
				}
			}
		}
		else if (nTopLeft == 0)
		{
			for (int y = 0; y < nStartY; y++)
			{
				int nWhiteCnt = 0;
				int nResult = 0;
				for (int x = 0; x < nStartX; x++)
				{
					if ((nLeadDirection == 0) || (nLeadDirection == 1))
					{
						if (pucTipBlobData[y + (x * nTipWidth)] == nArrSearchValue[n])
							nWhiteCnt += 1;
					}
					else if ((nLeadDirection == 2) || (nLeadDirection == 3))
					{
						if (pucTipBlobData[x + (y * nTipWidth)] == nArrSearchValue[n])
							nWhiteCnt += 1;
					}
				}

				double dBlackRate = (nWhiteCnt * 100.0) / (double)nStartX;
				// ROI내에 흰색값이 설정값보다 아래일 경우 tip이라 간주
				if (bLeadFind == true)
				{
					if (dPixPerValue1 >= dBlackRate)
					{
						nRsultYValue = y;
						y = nStartY;
						bResult = TRUE;
					}
				}
				if (dPixPerValue2 <= dBlackRate)
					bLeadFind = true;
				else
				{
					if (nType == 0)
						bLeadFind = false;
				}
			}
		}
		if (pucTip2DImg)
			Delete_1DArray(&pucTip2DImg);
		if (pucTip2DData)
			Delete_1DArray(&pucTip2DData);
		if (pucTip3DImg)
			Delete_1DArray(&pucTip3DImg);
		if (pucTip3DData)
			Delete_1DArray(&pucTip3DData);
		if (pucTipData)
			Delete_1DArray(&pucTipData);
		if (pucTipBlobData)
			Delete_1DArray(&pucTipBlobData);

		if (bResult == TRUE)
		{
			nIndex = n;
			break;
		}
	}

	if (bResult)
	{
		if ((nLeadDirection == 0))			// 0 = left
		{
			if (nType == 0)
				nResultPos = ((nImgWidth)-(nArrTipStartX[nIndex] + nRsultYValue));// + 1;
			else
				nResultPos = nArrTipStartX[nIndex] + nRsultYValue;// - 1;
		}
		else if (nLeadDirection == 1)		// 1 = right
		{
			nResultPos = nArrTipStartX[nIndex] + nRsultYValue;// - 1;
		}
		else if (nLeadDirection == 2)		// 2 = top
		{
			if (nType == 0)
				nResultPos = ((nImgHeight)-(nArrTipStartY[nIndex] + nRsultYValue));// + 1;
			else
				nResultPos = nArrTipStartY[nIndex] + nRsultYValue;// - 1;
		}
		else if (nLeadDirection == 3)		// 3 = bottom
		{
			nResultPos = nArrTipStartY[nIndex] + nRsultYValue;// - 1;
		}
	}
	else
		nResultPos = nLeadTipPos;

	return bResult;
}
int CPInsp_AlgoLeadTip::SearchTipPosition2(UCHAR *pucImgSrc, float *pfImgSrc, int nImgWidth, int nImgHeight, AlgoLeadTip *pInspAlgoLeadTip)
{
	int nResultPos = -1;
	int nFindTipPos = -1;
	if (pucImgSrc == NULL || pfImgSrc == NULL || nImgWidth <= 0 || nImgHeight <= 0)
		return nResultPos;
	BOOL bUse2D = pInspAlgoLeadTip->m_b3DIn2dCheck;
	BOOL bUse3D = pInspAlgoLeadTip->b3dCheck;
	double d3DValueOrg = pInspAlgoLeadTip->n3dHeightMax + pInspAlgoLeadTip->m_fTipCapGap;
	double d3DValue = pInspAlgoLeadTip->n3dHeightMax + pInspAlgoLeadTip->m_fTipCapGap;
	double d2DValue = pInspAlgoLeadTip->m_n3DIn2DMaxValue;
	if (_isnan(d3DValue) == true)
		d3DValue = 0;
	if (_isnan(d2DValue) == true)
		d2DValue = 0;
	if (bUse3D == FALSE)
		d3DValue = 100;
	if (bUse2D == FALSE)
		d2DValue = 140;
	double d3DValueMax = d3DValue * 2;
	if (d3DValueMax < 200)
		d3DValueMax = 200;
	int nLeadDirection = pInspAlgoLeadTip->nLeadDirection;
	int nLeadTipPos = pInspAlgoLeadTip->nLeadTipPos;
	int nEndPos = 0;
	double* dArrAvg2D = NULL;
	double* dArrAvg3D = NULL;
	if (nLeadDirection == 0 || nLeadDirection == 1)
	{
		nEndPos = nImgWidth;
		/*dArrAvg2D = new double[nImgWidth];
		dArrAvg3D = new double[nImgWidth];*/
		dArrAvg2D = g_pMManager->pem_new<double>(true, nImgWidth, (PCHAR)__FUNCTION__, __LINE__);
		dArrAvg3D = g_pMManager->pem_new<double>(true, nImgWidth, (PCHAR)__FUNCTION__, __LINE__);
		memset(dArrAvg2D, 0, sizeof(double) * nImgWidth);
		memset(dArrAvg3D, 0, sizeof(double) * nImgWidth);
	}
	else
	{
		nEndPos = nImgHeight;
		/*dArrAvg2D = new double[nImgHeight];
		dArrAvg3D = new double[nImgHeight];*/
		dArrAvg2D = g_pMManager->pem_new<double>(true, nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		dArrAvg3D = g_pMManager->pem_new<double>(true, nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		memset(dArrAvg2D, 0, sizeof(double) * nImgHeight);
		memset(dArrAvg3D, 0, sizeof(double) * nImgHeight);
	}
	double dMinValue = 0.8;
	if (nLeadDirection == 0)	// 0 = Left
	{
		for (int x = nImgWidth - 1; x >= 0; x--)
		{
			for (int y = 0; y < nImgHeight; y++)
			{
				int nIndex = (y * nImgWidth) + x;
				if (dArrAvg2D[x] == 0)
					dArrAvg2D[x] += pucImgSrc[nIndex];
				else
				{
					if (pucImgSrc[nIndex] >= dArrAvg2D[x] * dMinValue)
						dArrAvg2D[x] = (dArrAvg2D[x] + pucImgSrc[nIndex]) / 2.0;
				}
				if (dArrAvg3D[x] == 0)
					dArrAvg3D[x] += pfImgSrc[nIndex];
				else
				{
					if (pfImgSrc[nIndex] >= dArrAvg3D[x] * dMinValue)
						dArrAvg3D[x] = (dArrAvg3D[x] + pfImgSrc[nIndex]) / 2.0;
				}
			}
		}
	}
	else if (nLeadDirection == 1)	// 1 = Right
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			for (int y = 0; y < nImgHeight; y++)
			{
				int nIndex = (y * nImgWidth) + x;
				if (dArrAvg2D[x] == 0)
					dArrAvg2D[x] += pucImgSrc[nIndex];
				else
				{
					if (pucImgSrc[nIndex] >= dArrAvg2D[x] * dMinValue)
						dArrAvg2D[x] = (dArrAvg2D[x] + pucImgSrc[nIndex]) / 2.0;
				}
				if (dArrAvg3D[x] == 0)
					dArrAvg3D[x] += pfImgSrc[nIndex];
				else
				{
					if (pfImgSrc[nIndex] >= dArrAvg3D[x] * dMinValue)
						dArrAvg3D[x] = (dArrAvg3D[x] + pfImgSrc[nIndex]) / 2.0;
				}
			}
		}
	}
	else if (nLeadDirection == 2)	// 2 = Top
	{
		for (int y = nImgHeight - 1; y >= 0; y--)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIndex = (y * nImgWidth) + x;
				if (dArrAvg2D[y] == 0)
					dArrAvg2D[y] += pucImgSrc[nIndex];
				else
				{
					if (pucImgSrc[nIndex] >= dArrAvg2D[y] * dMinValue)
						dArrAvg2D[y] = (dArrAvg2D[y] + pucImgSrc[nIndex]) / 2.0;
				}
				if (dArrAvg3D[y] == 0)
					dArrAvg3D[y] += pfImgSrc[nIndex];
				else
				{
					if (pfImgSrc[nIndex] >= dArrAvg3D[y] * dMinValue)
						dArrAvg3D[y] = (dArrAvg3D[y] + pfImgSrc[nIndex]) / 2.0;
				}
			}
		}
	}
	else if (nLeadDirection == 3)	// 3 = Bottom
	{
		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIndex = (y * nImgWidth) + x;
				if (dArrAvg2D[y] == 0)
					dArrAvg2D[y] += pucImgSrc[nIndex];
				else
				{
					if (pucImgSrc[nIndex] >= dArrAvg2D[y] * dMinValue)
						dArrAvg2D[y] = (dArrAvg2D[y] + pucImgSrc[nIndex]) / 2.0;
				}
				if (dArrAvg3D[y] == 0)
					dArrAvg3D[y] += pfImgSrc[nIndex];
				else
				{
					if (pfImgSrc[nIndex] >= dArrAvg3D[y] * dMinValue)
						dArrAvg3D[y] = (dArrAvg3D[y] + pfImgSrc[nIndex]) / 2.0;
				}
			}
		}
	}
	int nGap = 10;
	int nGap2 = nGap * 3;
	if (nEndPos < nGap)
		nGap = nEndPos / 3;
	if (nEndPos < nGap2)
		nGap2 = nEndPos / 2;
	int nStartPos = -1;
	double d3DAvg = 0;
	double d2DAvg = 0;
	double d3DMax = 0;
	int nFindTipPos3D = -1;
	int nFindTipEndPos = -1;
	int nFindTipMax = -1;
	int nFindTipMax2 = -1;
	int nPosDiff = 0;
	for (int n = 0; n < nGap * 4; n++)
	{
		if (n % nGap == 0)
		{
			d2DValue = pInspAlgoLeadTip->m_n3DIn2DMaxValue;
			d3DValue = d3DValueOrg;
		}
		else if (n % 4 == 0)
		{
			if (n < nGap || (n > nGap && n < nGap * 2))
				d2DValue -= nGap;
			else
				d2DValue += nGap;
		}
		else
		{
			if (n < nGap || n > nGap * 2)
				d3DValue -= nGap;
			else
				d3DValue += nGap;
		}
		d3DValueMax = d3DValue * 2;
		if (d2DValue < 0)
			d2DValue = pInspAlgoLeadTip->m_n3DIn2DMaxValue;
		if (d3DValue < 0)
			d3DValue = d3DValueOrg;
		int nStartPosBuf = -1;
		double d3DAvgBuf = -1;
		d2DAvg = -1;
		d3DMax = -1;
		int nPosDiffBuf = 0;
		if (nLeadDirection == 1 || nLeadDirection == 3)
		{
			for (int a = 0; a < nEndPos; a++)
			{
				if (dArrAvg3D[a] > d3DValue && dArrAvg3D[a] < d3DValueMax && dArrAvg2D[a] >= (int)d2DValue)
				{
					if (nStartPosBuf == -1)
						nStartPosBuf = a;
					if (d3DAvgBuf == 0)
						d3DAvgBuf += dArrAvg3D[a];
					else
						d3DAvgBuf = (d3DAvgBuf + dArrAvg3D[a]) / 2.0;
					if (d2DAvg == 0)
						d2DAvg += dArrAvg2D[a];
					else
						d2DAvg = (d2DAvg + dArrAvg2D[a]) / 2.0;
					if (dArrAvg3D[a] > d3DMax)
					{
						d3DMax = dArrAvg3D[a];
						nFindTipMax = a;
					}
					nFindTipEndPos = a;
				}
			}
			nPosDiffBuf = nFindTipEndPos - nStartPosBuf;
		}
		else
		{
			for (int a = nEndPos - 1; a >= 0; a--)
			{
				if (dArrAvg3D[a] > d3DValue && dArrAvg3D[a] < d3DValueMax && dArrAvg2D[a] >= (int)d2DValue)
				{
					if (nStartPosBuf == -1)
						nStartPosBuf = a;
					if (d3DAvgBuf == 0)
						d3DAvgBuf += dArrAvg3D[a];
					else
						d3DAvgBuf = (d3DAvgBuf + dArrAvg3D[a]) / 2.0;
					if (d2DAvg == 0)
						d2DAvg += dArrAvg2D[a];
					else
						d2DAvg = (d2DAvg + dArrAvg2D[a]) / 2.0;
					if (dArrAvg3D[a] > d3DMax)
					{
						d3DMax = dArrAvg3D[a];
						nFindTipMax = a;
					}
					nFindTipEndPos = a;
				}
			}
			nPosDiffBuf = nStartPosBuf - nFindTipEndPos;
		}
		if (nStartPosBuf == -1 || d3DAvgBuf == 0)
			continue;
		if (nPosDiffBuf <= nGap2 && n < nGap - 1)
		{
			if (nPosDiffBuf > nPosDiff)
			{
				nPosDiff = nPosDiffBuf;
				nStartPos = nStartPosBuf;
				d3DAvg = d3DAvgBuf;
			}
			continue;
		}
		if (nStartPosBuf > -1 || d3DAvgBuf > 0)
		{
			nStartPos = nStartPosBuf;
			d3DAvg = d3DAvgBuf;
			break;
		}
	}
	if (nStartPos == -1 || d3DAvg <= 0)
	{
		if (dArrAvg2D)
			Delete_1DArray(&dArrAvg2D);
		if (dArrAvg3D)
			Delete_1DArray(&dArrAvg3D);
		return nResultPos;
	}
	d3DMax -= nGap;
	if (d3DMax < 0)
		d3DMax += nGap;
	int nDownValue = 5;
	if (nLeadDirection == 1 || nLeadDirection == 3)
	{
		int nStart = nStartPos + nGap;
		if (nStart > nEndPos)
			nStart = nStartPos;
		for (int a = nStart; a < nEndPos; a++)
		{
			if (a - nDownValue < nStart)
				continue;
			if (d2DAvg > dArrAvg2D[a] && d3DAvg > dArrAvg3D[a])
			{
				bool bInput = true;
				for (int b = 1; b <= nDownValue; b++)
				{
					if ((int)dArrAvg2D[a - b] >= (int)dArrAvg2D[a] && dArrAvg3D[a - b] > dArrAvg3D[a])
						bInput = true;
					else
					{
						bInput = false;
						break;
					}
				}
				if (nFindTipPos == -1 && bInput == true)
					nFindTipPos = a;
			}
			if (d3DAvg > dArrAvg3D[a] && dArrAvg3D[a - 1] > dArrAvg3D[a] && nFindTipPos3D == -1)
				nFindTipPos3D = a;
			if (a > nFindTipMax && d3DMax > dArrAvg3D[a] && nFindTipMax2 == -1)
				nFindTipMax2 = a;
			if (nFindTipPos > -1 && nFindTipPos3D > -1 && nFindTipMax2 > -1)
				break;
		}
	}
	else
	{
		int nStart = nStartPos - nGap;
		if (nStart < 0)
			nStart = nStartPos;
		for (int a = nStart; a >= 0; a--)
		{
			if (a + nDownValue > nStart)
				continue;
			if (d2DAvg > dArrAvg2D[a] && d3DAvg > dArrAvg3D[a])
			{
				bool bInput = true;
				for (int b = 1; b <= nDownValue; b++)
				{
					if ((int)dArrAvg2D[a + b] >= (int)dArrAvg2D[a] && dArrAvg3D[a + b] > dArrAvg3D[a])
						bInput = true;
					else
					{
						bInput = false;
						break;
					}
				}
				if (nFindTipPos == -1 && bInput == true)
					nFindTipPos = a;
			}
			if (d3DAvg > dArrAvg3D[a] && dArrAvg3D[a + 1] > dArrAvg3D[a] && nFindTipPos3D == -1)
				nFindTipPos3D = a;
			if (a < nFindTipMax && d3DMax > dArrAvg3D[a] && nFindTipMax2 == -1)
				nFindTipMax2 = a;
			if (nFindTipPos > -1 && nFindTipPos3D > -1 && nFindTipMax2 > -1)
				break;
		}
	}

	int nDiff = nFindTipPos3D - nFindTipPos;
	int nDiff2 = nFindTipMax2 - nFindTipPos;
	if (nDiff < 0)
		nDiff *= -1;
	if (nDiff2 < 0)
		nDiff2 *= -1;
	if (nFindTipPos == -1 || nDiff > 20)
		nFindTipPos = nFindTipPos3D;
	else if (nFindTipPos == -1 || nDiff2 > 30)
		nFindTipPos = nFindTipMax2;

	if ((nLeadDirection == 0))			// 0 = left
		nResultPos = nImgWidth - nFindTipPos;
	else if (nLeadDirection == 1)		// 1 = right
		nResultPos = nFindTipPos;
	else if (nLeadDirection == 2)		// 2 = top
		nResultPos = nImgHeight - nFindTipPos;
	else if (nLeadDirection == 3)		// 3 = bottom
		nResultPos = nFindTipPos;

	if (dArrAvg2D)
		Delete_1DArray(&dArrAvg2D);
	if (dArrAvg3D)
		Delete_1DArray(&dArrAvg3D);
	return nResultPos;
}
BOOL CPInsp_AlgoLeadTip::TeachTip(const InspAlgo &sInspAlgo, WndInfo &sWndInfo, WndAlgoImg &sWndAlgoImg, double dWndX, double dWndY, RstAlgoLeadTip * sRstAlgo, BOOL bTeach, float &fHeightMean)
{
	BOOL bResult = FALSE;
	BOOL bSideTipResult = FALSE;
	if (sRstAlgo)
	{
		sRstAlgo->m_bRstFindOK = FALSE;
	}

	if (sInspAlgo.m_eAlgoType != eAlgoLead_Tip)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	UCHAR *pucPartImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfPartImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	int nPartImgWidth = sWndAlgoImg.m_nWidth;		// Window Size + Search Area
	int nPartImgHeight = sWndAlgoImg.m_nHeight;
	int nPartImgWidth3D = sWndAlgoImg.m_nWidth3D;
	int nPartImgHeight3D = sWndAlgoImg.m_nHeight3D;
	if ((pucPartImgSrc == NULL) || (pfPartImgSrc == NULL) || (nPartImgWidth <= 0) || (nPartImgHeight <= 0))
		return bResult;

	int nStartX = 0;
	int nStartY = 0;
	int nStartX3D = 0;
	int nStartY3D = 0;
	float fHeightMeanBuf = -100.0f;
	if (bTeach == TRUE)
	{
		nStartX = 0;
		nStartY = 0;

		double dClipWidth = (float)(nImgWidth / m_resolX);
		double dClipHeight = (float)(nImgHeight / m_resolY);
		if (dClipWidth > nImgWidth)
			dClipWidth = nImgWidth;
		if (dClipHeight > nImgHeight)
			dClipHeight = nImgHeight;

		if ((dClipWidth <= 0) || (dClipHeight <= 0))
			return bResult;
		double dClipX3D = (double)nPartImgWidth3D / 2. + (sWndInfo.dCenterX / m_resolX);
		double dClipY3D = (double)nPartImgHeight3D / 2. - (sWndInfo.dCenterY / m_resolY);

		if (dClipX3D <= dClipWidth / 2.)
			dClipX3D = dClipWidth / 2.;
		if (dClipY3D <= dClipHeight / 2.)
			dClipY3D = dClipHeight / 2.;

		nStartX3D = RounD(dClipX3D - (dClipWidth / 2.0));
		nStartY3D = RounD(dClipY3D - (dClipHeight / 2.0));
	}
	else
	{
		nPartImgWidth = sWndAlgoImg.m_nWidth;
		nPartImgHeight = sWndAlgoImg.m_nHeight;
		nPartImgWidth3D = sWndAlgoImg.m_nWidth;
		nPartImgHeight3D = sWndAlgoImg.m_nHeight;
		nImgWidth = sWndInfo.dWidth;			// window Width
		nImgHeight = sWndInfo.dLength;			// Width Height
		nStartX = 0;//RounD(nPartImgWidth / 2.0 - (sWndInfo.dWidth / 2.0));
		nStartY = 0;//RounD(nPartImgHeight / 2.0 - (sWndInfo.dLength / 2.0));
		nStartX3D = nStartX;
		nStartY3D = nStartY;
	}

	AlgoLeadTip *pInspAlgoLeadTip = (AlgoLeadTip *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoLeadTip)
		return bResult;

	double dRstShiftX = 0;
	double dRstShiftY = 0;
	double dRstSideWidth = 0;
	double dRstSideStartPos = 0;
	double dRstShiftXPer = 0;
	double dRstShiftYPer = 0;
	if (pInspAlgoLeadTip->m_bUseSideTip && bTeach == FALSE)
	{
		LeadTipSideTip sSideTip;
		//RstLeadTipSideTip *sRst = new RstLeadTipSideTip();
		RstLeadTipSideTip *sRst = g_pMManager->pem_new<RstLeadTipSideTip>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		sSideTip.pucPartImgSrc = pucPartImgSrc;
		sSideTip.pfPartImgSrc = pfPartImgSrc;
		sSideTip.nImgWidth = nImgWidth;
		sSideTip.nImgHeight = nImgHeight;
		sSideTip.nPartImgWidth = nPartImgWidth;
		sSideTip.nPartImgHeight = nPartImgHeight;
		sSideTip.dWndX = dWndX;
		sSideTip.dWndY = dWndY;
		sRst->nStartX = nStartX;
		sRst->nStartY = nStartY;
		sRst->nStartX3D = nStartX3D;
		sRst->nStartY3D = nStartY3D;
		sRst->fHeightMeanBuf = fHeightMeanBuf;
		SearchSideTipPosition(pInspAlgoLeadTip, sSideTip, sRst);
		bSideTipResult = sRst->m_bSideTipResult;
		dRstShiftX = sRst->m_dRstShiftX;
		dRstShiftY = sRst->m_dRstShiftY;
		dRstSideWidth = sRst->m_dRstSideWidth;
		dRstSideStartPos = sRst->m_dRstSideStartPos;
		nStartX = sRst->nStartX;
		nStartY = sRst->nStartY;
		nStartX3D = sRst->nStartX3D;
		nStartY3D = sRst->nStartY3D;
		dRstShiftXPer = sRst->m_dRstShiftXPer;
		dRstShiftYPer = sRst->m_dRstShiftYPer;
		if (pInspAlgoLeadTip->nLeadDirection == 0 || pInspAlgoLeadTip->nLeadDirection == 1)	// 0 = left, 1 = right
		{
			if (pInspAlgoLeadTip->m_bUseLeadWindowSizeChange == TRUE && sRst->nSideTipLength > 0)
				nImgHeight = sRst->nSideTipLength;
		}
		else
		{
			if (pInspAlgoLeadTip->m_bUseLeadWindowSizeChange == TRUE && sRst->nSideTipLength > 0)
				nImgWidth = sRst->nSideTipLength;
		}
		if (sRstAlgo)
		{
			sRstAlgo->m_poDrawLine_Side[0].x = sRst->m_poDrawLine_Side[0].x;
			sRstAlgo->m_poDrawLine_Side[0].y = sRst->m_poDrawLine_Side[0].y;
			sRstAlgo->m_poDrawLine_Side[1].x = sRst->m_poDrawLine_Side[1].x;
			sRstAlgo->m_poDrawLine_Side[1].y = sRst->m_poDrawLine_Side[1].y;
		}

		// MemoryLeak Fix -wjlee
		//Delete_1DArray(&sRst);
		g_pMManager->pem_delete(sRst, false);
	}

	UCHAR *pucImgSrc = NULL;
	float *pfImgSrc = NULL;
	bool bDel2D = false;
	bool bDel3D = false;
	if (bTeach == TRUE)
	{
		pucImgSrc = pucPartImgSrc;
		pfImgSrc = pfPartImgSrc;
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Window_2DImage.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Window_3DImage.bmp"));
	}
	else if (nImgWidth == nPartImgWidth && nImgHeight == nPartImgHeight)
	{
		bDel3D = true;
		pucImgSrc = pucPartImgSrc;
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Clip_Window_2DImage.bmp"));
		//pfImgSrc  = new float[nImgWidth * nImgHeight];
		pfImgSrc = g_pMManager->pem_new<float>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap_LT(pfPartImgSrc, pfImgSrc, nPartImgWidth3D, nPartImgHeight3D, nStartX3D, nStartY3D, nImgWidth, nImgHeight);
		m_pProcMilAlgo->SaveWorkImg_float(pfPartImgSrc, nPartImgWidth3D, nPartImgHeight3D, _T("Origin_Part_3DImage.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Clip_Window_3DImage.bmp"));
	}
	else
	{
		bDel2D = true;
		bDel3D = true;
		/*pucImgSrc = new UCHAR[nImgWidth * nImgHeight];
		pfImgSrc  = new float[nImgWidth * nImgHeight];*/
		pucImgSrc = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		pfImgSrc = g_pMManager->pem_new<float>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(pucPartImgSrc, nPartImgWidth, nPartImgHeight, pucImgSrc, nStartX, nStartY, nImgWidth, nImgHeight);
		m_pProcMilAlgo->SaveWorkImg(pucPartImgSrc, nPartImgWidth, nPartImgHeight, _T("Origin_Part_2DImage.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Clip_Window_2DImage.bmp"));
		m_proc3d.GetCropZmap_LT(pfPartImgSrc, pfImgSrc, nPartImgWidth3D, nPartImgHeight3D, nStartX3D, nStartY3D, nImgWidth, nImgHeight);
		m_pProcMilAlgo->SaveWorkImg_float(pfPartImgSrc, nPartImgWidth3D, nPartImgHeight3D, _T("Origin_Part_3DImage.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Clip_Window_3DImage.bmp"));
	}
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	int nResultPos = 0;
	bResult = FALSE;
	if (pInspAlgoLeadTip->m_nTipFindOpt == eLeadTip_Tip_Cap || pInspAlgoLeadTip->m_nTipFindOpt == eLeadTip_Tip_All)
	{
		nResultPos = SearchTipPosition2(pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, pInspAlgoLeadTip);
		if (nResultPos > -1)
			bResult = TRUE;
	}
	if (bResult == FALSE)
	{
		if (pInspAlgoLeadTip->m_b2dCheck == TRUE)
		{
			bResult = SearchTipPosition(pInspAlgoLeadTip, 0
				, pInspAlgoLeadTip->nSearchRange, pInspAlgoLeadTip->nLeadTipPos, pInspAlgoLeadTip->nLeadDirection, pInspAlgoLeadTip->nPixelPercentValue
				, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, pInspAlgoLeadTip->m_b2dCheck, FALSE
				, nResultPos, fHeightMeanBuf, pInspAlgoLeadTip->m_bDirInvert);
		}
		if (bResult == FALSE)
		{
			if (pInspAlgoLeadTip->b3dCheck == TRUE)
			{
				BOOL bUse2D = pInspAlgoLeadTip->m_b3DIn2dCheck;
				BOOL bUse3D = pInspAlgoLeadTip->b3dCheck;
				nResultPos = 0;
				bResult = SearchTipPosition(pInspAlgoLeadTip, 0
					, pInspAlgoLeadTip->nSearchRange, pInspAlgoLeadTip->nLeadTipPos, pInspAlgoLeadTip->nLeadDirection, pInspAlgoLeadTip->nPixelPercentValue
					, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, bUse2D, bUse3D
					, nResultPos, fHeightMean, pInspAlgoLeadTip->m_bDirInvert);
			}
		}
	}

	if (sRstAlgo)
	{
		int nRealOffset = pInspAlgoLeadTip->m_nRealOffset;
		if (nRealOffset < -5) nRealOffset = -5;
		if (nRealOffset > 5) nRealOffset = 5;
		nResultPos += nRealOffset;
		if ((pInspAlgoLeadTip->nLeadDirection == 0) || (pInspAlgoLeadTip->nLeadDirection == 1))	// 0 = left, 1 = right
		{
			if (nResultPos >= nImgWidth)
			{
				bResult = FALSE;
				nResultPos = nImgWidth - 1;
			}
		}
		else		// Top, Bottom
		{
			if (nResultPos >= nImgHeight)
			{
				bResult = FALSE;
				nResultPos = nImgHeight - 1;
			}
		}
		if (nResultPos < 0)
		{
			bResult = FALSE;
			nResultPos = 0;
		}
		if (bResult == TRUE && (pInspAlgoLeadTip->m_nTipFindOpt == eLeadTip_Tip_All || pInspAlgoLeadTip->m_nTipFindOpt == eLeadTip_Tip_Length))
		{
			float fTipLength = pInspAlgoLeadTip->m_fTipLength;
			sRstAlgo->m_nRstLeadTipPos_Length = nResultPos;
			if ((pInspAlgoLeadTip->nLeadDirection == 0) || (pInspAlgoLeadTip->nLeadDirection == 1))
				nResultPos += (fTipLength / m_resolX);
			else if ((pInspAlgoLeadTip->nLeadDirection == 2) || (pInspAlgoLeadTip->nLeadDirection == 3))
				nResultPos += (fTipLength / m_resolY);
			if ((pInspAlgoLeadTip->nLeadDirection == 0) || (pInspAlgoLeadTip->nLeadDirection == 1))	// 0 = left, 1 = right
			{
				if (nResultPos >= nImgWidth)
				{
					bResult = FALSE;
					nResultPos = nImgWidth - 1;
				}
			}
			else		// Top, Bottom
			{
				if (nResultPos >= nImgHeight)
				{
					bResult = FALSE;
					nResultPos = nImgHeight - 1;
				}
			}
			if (nResultPos < 0)
			{
				bResult = FALSE;
				nResultPos = 0;
			}
		}
		else
		{
			sRstAlgo->m_nRstLeadTipPos_Length = nResultPos;
			if (pInspAlgoLeadTip->m_nTipFindOpt == eLeadTip_Tip_Length)
			{
				float fTipLength = pInspAlgoLeadTip->m_fTipLength;
				sRstAlgo->m_nRstLeadTipPos_Length = nResultPos;
				if ((pInspAlgoLeadTip->nLeadDirection == 0) || (pInspAlgoLeadTip->nLeadDirection == 1))
					nResultPos += (fTipLength / m_resolX);
				else if ((pInspAlgoLeadTip->nLeadDirection == 2) || (pInspAlgoLeadTip->nLeadDirection == 3))
					nResultPos += (fTipLength / m_resolY);
			}
		}

		if ((pInspAlgoLeadTip->nLeadDirection == 0) || (pInspAlgoLeadTip->nLeadDirection == 1))
		{
			sRstAlgo->m_poDrawLine_T[0].x = nStartX + pInspAlgoLeadTip->nLeadTipPos;
			sRstAlgo->m_poDrawLine_T[0].y = nStartY;
			sRstAlgo->m_poDrawLine_T[1].x = nStartX + pInspAlgoLeadTip->nLeadTipPos;
			sRstAlgo->m_poDrawLine_T[1].y = nStartY + nImgHeight;

			sRstAlgo->m_poDrawLine_I[0].x = nStartX + nResultPos;
			sRstAlgo->m_poDrawLine_I[0].y = nStartY;
			sRstAlgo->m_poDrawLine_I[1].x = nStartX + nResultPos;
			sRstAlgo->m_poDrawLine_I[1].y = nStartY + nImgHeight;
		}
		else
		{
			sRstAlgo->m_poDrawLine_T[0].x = nStartX;
			sRstAlgo->m_poDrawLine_T[0].y = nStartY + pInspAlgoLeadTip->nLeadTipPos;
			sRstAlgo->m_poDrawLine_T[1].x = nStartX + nImgWidth;
			sRstAlgo->m_poDrawLine_T[1].y = nStartY + pInspAlgoLeadTip->nLeadTipPos;

			sRstAlgo->m_poDrawLine_I[0].x = nStartX;
			sRstAlgo->m_poDrawLine_I[0].y = nStartY + nResultPos;
			sRstAlgo->m_poDrawLine_I[1].x = nStartX + nImgWidth;
			sRstAlgo->m_poDrawLine_I[1].y = nStartY + nResultPos;
		}

		sRstAlgo->m_bRstFindOK = bResult;
		sRstAlgo->m_nRstLeadTipPos = nResultPos;
		sRstAlgo->m_bTipFaultNG = pInspAlgoLeadTip->bTipFaultNG;
		sRstAlgo->m_bLeadTipMaster = FALSE;
		if (pInspAlgoLeadTip->m_bUseSideTip == TRUE && bTeach == FALSE)
		{
			sRstAlgo->m_bSideTipOK = bSideTipResult;
			bResult = bResult && bSideTipResult;
			sRstAlgo->m_dRstSideTipPos = dRstSideStartPos;
			sRstAlgo->m_bSideTipShiftXOK = TRUE;
			sRstAlgo->m_bSideTipShiftYOK = TRUE;
			sRstAlgo->m_dRstSideTipShiftX = dRstShiftX;
			sRstAlgo->m_dRstSideTipShiftY = dRstShiftY;
			sRstAlgo->m_dRstSideTipShiftXPer = dRstShiftXPer;
			sRstAlgo->m_dRstSideTipShiftYPer = dRstShiftYPer;
			if (pInspAlgoLeadTip->m_bUseSideTipShift == TRUE)
			{
				double dShiftX = dRstShiftX;
				double dShiftY = dRstShiftY;
				if (dShiftX < 0)
					dShiftX *= -1;
				if (dShiftY < 0)
					dShiftY *= -1;
				if (pInspAlgoLeadTip->m_dSideTipShiftX < dShiftX)
				{
					sRstAlgo->m_bSideTipShiftXOK = FALSE;
					bResult = FALSE;
				}
				if (pInspAlgoLeadTip->m_dSideTipShiftY < dShiftY)
				{
					sRstAlgo->m_bSideTipShiftYOK = FALSE;
					bResult = FALSE;
				}
			}
			if (pInspAlgoLeadTip->m_bUseLeadWindowSizeChange == TRUE)
				sRstAlgo->m_dRstSideTipSize = dRstSideWidth;
			else
				sRstAlgo->m_dRstSideTipSize = -1;
		}
	}

	if (bDel2D)
		Delete_1DArray(&pucImgSrc);
	if (bDel3D)
		Delete_1DArray(&pfImgSrc);

	return bResult;
}

BOOL CPInsp_AlgoLeadTip::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoLeadTip::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}

int CPInsp_AlgoLeadTip::TipMaster(const InspAlgo &sInspAlgo, int *ptrnArrLeadTipRst, int nLeadTipTotalCnt, RstAlgoLeadTip * sRstAlgo)
{
	int nRstTipPos = -1;

	if (sInspAlgo.m_eAlgoType != eAlgoLead_Tip || !m_pProcMilAlgo
		|| ptrnArrLeadTipRst == NULL || nLeadTipTotalCnt <= 0)
		return nRstTipPos;

	AlgoLeadTip *pInspAlgoLeadTip = (AlgoLeadTip *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoLeadTip || pInspAlgoLeadTip->m_bTipMaster == false)
		return nRstTipPos;
	float fLength = pInspAlgoLeadTip->m_fTipLength;
	float fMinLength = pInspAlgoLeadTip->m_fTipLength - pInspAlgoLeadTip->m_fTMAllowRange;
	float fMaxLength = pInspAlgoLeadTip->m_fTipLength + pInspAlgoLeadTip->m_fTMAllowRange;
	int nLength = 0;
	int nMinLength = 0;
	int nMaxLength = 0;
	if ((pInspAlgoLeadTip->nLeadDirection == 0) || (pInspAlgoLeadTip->nLeadDirection == 1))
	{
		nLength = RounD(fLength / m_resolX);
		nMinLength = RounD(fMinLength / m_resolX);
		nMaxLength = RounD(fMaxLength / m_resolX);
	}
	else if ((pInspAlgoLeadTip->nLeadDirection == 2) || (pInspAlgoLeadTip->nLeadDirection == 3))
	{
		nLength = RounD(fLength / m_resolY);
		nMinLength = RounD(fMinLength / m_resolY);
		nMaxLength = RounD(fMaxLength / m_resolY);
	}
	int nTipLength = pInspAlgoLeadTip->nLeadTipPos + nLength;
	int nTipMinLength = pInspAlgoLeadTip->nLeadTipPos + nMinLength;
	int nTipMaxLength = pInspAlgoLeadTip->nLeadTipPos + nMaxLength;

	int nGap = 0;
	bool bStartInput = false;
	for (int nIdx = 0; nIdx < nLeadTipTotalCnt; nIdx++)
	{
		int nTipPos = ptrnArrLeadTipRst[nIdx];
		if (nTipMinLength <= nTipPos && nTipMaxLength >= nTipPos)
		{
			if (bStartInput == false)
			{
				nRstTipPos = nTipPos;
				nGap = nTipPos - nTipMaxLength;
				if (nGap < 0) nGap *= -1;
				bStartInput = true;
			}
			else
			{
				int nGapBuf = nTipPos - nTipMaxLength;
				if (nGapBuf < 0) nGapBuf *= -1;
				if (nGap > nGapBuf)
				{
					nRstTipPos = nTipPos;
					nGap = nGapBuf;
				}
			}
		}
	}
	if (sRstAlgo)
	{
		sRstAlgo->m_bLeadTipMaster = TRUE;
		sRstAlgo->m_nOrgTipMasterPos = nTipLength;
		sRstAlgo->m_nOrgRstLeadTipPos = sRstAlgo->m_nRstLeadTipPos;
		if (nRstTipPos != -1)
			sRstAlgo->m_nRstLeadTipPos = nRstTipPos;
		else
		{
			nRstTipPos = nTipLength;
			sRstAlgo->m_nRstLeadTipPos = nTipLength;
		}
	}
	return nRstTipPos;
}
bool CPInsp_AlgoLeadTip::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}