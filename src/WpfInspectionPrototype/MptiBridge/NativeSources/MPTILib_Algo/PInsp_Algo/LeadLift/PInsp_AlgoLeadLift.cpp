#include "PInsp_AlgoLeadLift.h"


CPInsp_AlgoLeadLift::CPInsp_AlgoLeadLift(void)
{
}


CPInsp_AlgoLeadLift::~CPInsp_AlgoLeadLift(void)
{
}

void CPInsp_AlgoLeadLift::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoLeadLift::GetInspAlgoData()
{
	return eSPCAlgoLead_Lift;
}

int CPInsp_AlgoLeadLift::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeLeadLift] = e_NG;
	eWholeNgType[TypeLeadLift] = e_NG;
	nCurrentNgType = TypeLeadLift;
	return nCurrentNgType;
}

bool CPInsp_AlgoLeadLift::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return true;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoLeadLift::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspLift(sInspAlgo, sWndAlgoImg, (RstAlgoLeadLift *)sRstAlgo, stAlgoParam.m_nLeadTipPos, stAlgoParam.m_sInspAlgoTempResult);

	RstAlgoLeadLift * sRst = (RstAlgoLeadLift *)sRstAlgo;
	if (stAlgoParam.m_bInspection)
	{
		if (stAlgoParam.m_sInspAlgoTempResult != NULL)
		{
			if (stAlgoParam.m_sInspAlgoTempResult->m_bParentWnd == false)
			{
				stAlgoParam.m_sInspAlgoTempResult->m_ptrdLeadLiftResult[stAlgoParam.m_nLeadTipPosIndex] = sRst->m_dRstHeight;
				stAlgoParam.m_sInspAlgoTempResult->m_nLeadLiftGroupCnt = stAlgoParam.m_nLeadTipPosIndex + 1;
			}
			for (int nIdx = 0; nIdx < LEADLIFT_ROI_CNTS; nIdx++)
			{
				sRst->m_rcRect_I[nIdx].left += stAlgoParam.m_dx;
				sRst->m_rcRect_I[nIdx].right += stAlgoParam.m_dx;
				sRst->m_rcRect_I[nIdx].top += stAlgoParam.m_dy;
				sRst->m_rcRect_I[nIdx].bottom += stAlgoParam.m_dy;
			}
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoLeadLift::InspLift(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoLeadLift * sRstAlgo, int nLeadTipPos, InspAlgoTempResult *sInspAlgoTempResult)
{
	BOOL bResult = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoLeadLift));
		sRstAlgo->Init();
	}

	if (sInspAlgo.m_eAlgoType != eAlgoLead_Lift)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	double dAngle = sWndAlgoImg.dAngle;
	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	AlgoLeadLift *pInspAlgoLeadLift = (AlgoLeadLift *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoLeadLift)
		return bResult;
	if ((pInspAlgoLeadLift->nROIWidth <= 0) || (pInspAlgoLeadLift->nROIHeight <= 0)
		|| (pInspAlgoLeadLift->nLeadTipInterval < 0))
		return bResult;
	if ((pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIWidth) <= 0) || (pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIHeight) <= 0)
		|| (pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIInterval) < 0))
		return bResult;

	// Auto 검사 시, Lead Tip 검사 결과에 따른 값을 적용 (nLeadTipPos)
	// Auto 검사 시 Lead Tip Algorithm이 없는 경우, pInspAlgoLeadLift->nLeadPosition 값을 적용 (nLeadTipPos = -1)
	// Teaching 시 Lead pInspAlgoLeadLift->nLeadPosition 값을 적용	(nLeadTipPos = -1)
	if (nLeadTipPos < 0)
		nLeadTipPos = pInspAlgoLeadLift->nLeadPosition;

	int nROIWidth = pInspAlgoLeadLift->nROIWidth;
	int nROIHeight = pInspAlgoLeadLift->nROIHeight;
	int nROIWidth2 = RounD(pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIWidth) / m_resolX);
	int nROIHeight2 = RounD(pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIHeight) / m_resolY);
	if (dAngle == 90 || dAngle == 270)
	{
		nROIWidth = pInspAlgoLeadLift->nROIHeight;
		nROIHeight = pInspAlgoLeadLift->nROIWidth;
		nROIWidth2 = RounD(pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIHeight) / m_resolY);
		nROIHeight2 = RounD(pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIWidth) / m_resolX);
	}

	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("LiftOrigin3D.bmp"));

	if (nROIWidth > nImgWidth)
		nROIWidth = nImgWidth;
	if (nROIWidth2 > nImgWidth)
		nROIWidth2 = nImgWidth;

	if (nROIHeight > nImgHeight)
		nROIHeight = nImgHeight;
	if (nROIHeight2 > nImgHeight)
		nROIHeight2 = nImgHeight;

	int nScanCount = nROIWidth * nROIHeight;
	int nScanCount2 = nROIWidth2 * nROIHeight2;
	//float *pfImgDst = new float[nScanCount];
	float *pfImgDst = g_pMManager->pem_new<float>(true, nScanCount, (PCHAR)__FUNCTION__, __LINE__);
	float *pfImgDst2 = g_pMManager->pem_new<float>(true, nScanCount2, (PCHAR)__FUNCTION__, __LINE__);

	int nROIStartX = 0;
	int nROIStartY = 0;
	int nInterval = nLeadTipPos - pInspAlgoLeadLift->nLeadTipInterval;
	int nROIStartX2 = 0;
	int nROIStartY2 = 0;
	int nInterval2 = nLeadTipPos + RounD(pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightROIInterval) / m_resolX);
	if ((pInspAlgoLeadLift->nLeadTipDirection == 0) || (pInspAlgoLeadLift->nLeadTipDirection == 1))
	{
		if (pInspAlgoLeadLift->nLeadTipDirection == 0)
		{
			nROIStartX = nImgWidth - nInterval;		// LMJ 2015/02/05
			nROIStartX2 = nImgWidth - nInterval2 - nROIWidth2;
		}
		else
		{
			nROIStartX = nInterval - nROIWidth;
			nROIStartX2 = nInterval2;
		}

		nROIStartY = RounD(((double)nImgHeight / 2.0) - ((double)nROIHeight / 2.0));
		nROIStartY2 = RounD(((double)nImgHeight / 2.0) - ((double)nROIHeight2 / 2.0));
	}
	else
	{
		nROIStartX = RounD(((double)nImgWidth / 2.0) - ((double)nROIWidth / 2.0));
		nROIStartX2 = RounD(((double)nImgWidth / 2.0) - ((double)nROIWidth2 / 2.0));

		if (pInspAlgoLeadLift->nLeadTipDirection == 2)
		{
			nROIStartY = nImgHeight - nInterval;		// LMJ 2015/02/05
			nROIStartY2 = nImgHeight - nInterval2 - nROIHeight2;
		}
		else
		{
			nROIStartY = nInterval - nROIHeight;
			nROIStartY2 = nInterval2;
		}
	}
	if (nROIStartX < 0)
		nROIStartX = 0;
	if (nROIStartY < 0)
		nROIStartY = 0;
	if (nROIStartX + nROIWidth > nImgWidth)
		nROIWidth = nImgWidth - nROIStartX;
	if (nROIStartY + nROIHeight > nImgHeight)
		nROIHeight = nImgHeight - nROIStartY;
	if (nROIWidth <= 0)
	{
		nROIStartX -= 1;
		nROIWidth = 1;
	}
	if (nROIHeight <= 0)
	{
		nROIStartY -= 1;
		nROIHeight = 1;
	}
	if (nROIWidth > nImgWidth)
		nROIWidth = nImgWidth;
	if (nROIHeight > nImgHeight)
		nROIHeight = nImgHeight;

	if (nROIStartX2 < 0)
		nROIStartX2 = 0;
	if (nROIStartY2 < 0)
		nROIStartY2 = 0;
	if (nROIStartX2 + nROIWidth2 > nImgWidth)
		nROIWidth2 = nImgWidth - nROIStartX2;
	if (nROIStartY2 + nROIHeight2 > nImgHeight)
		nROIHeight2 = nImgHeight - nROIStartY2;
	if (nROIWidth2 <= 0)
	{
		nROIStartX2 -= 1;
		nROIWidth2 = 1;
	}
	if (nROIHeight2 <= 0)
	{
		nROIStartY2 -= 1;
		nROIHeight2 = 1;
	}
	if (nROIWidth2 > nImgWidth)
		nROIWidth2 = nImgWidth;
	if (nROIHeight2 > nImgHeight)
		nROIHeight2 = nImgHeight;

	m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst, nImgWidth, nImgHeight, nROIStartX, nROIStartY, nROIWidth, nROIHeight);
	m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst2, nImgWidth, nImgHeight, nROIStartX2, nROIStartY2, nROIWidth2, nROIHeight2);

	m_pProcMilAlgo->SaveWorkImg_float(pfImgDst, nROIWidth, nROIHeight, _T("Lift3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfImgDst2, nROIWidth2, nROIHeight2, _T("Lift3D_2.bmp"));

	double dResultMean = m_pCPInsp_Algo->GetHeightMean(pfImgDst, nROIWidth, nROIHeight);
	double dResulFilletHeight = 0.0;
	int nType = pInspAlgoLeadLift->GetDataN(m_eLeadLiftN_FilletHeightFindType);
	if (nType == m_eFilletHeightType_LeadLift_AVG)
	{
		dResulFilletHeight = m_pCPInsp_Algo->GetHeightMean(pfImgDst2, nROIWidth2, nROIHeight2);
	}
	else if (nType == m_eFilletHeightType_LeadLift_MAX)
	{
		float fHeightMinValue = 0.0f;
		float fHeightMaxValue = 0.0f;
		m_pCPInsp_Algo->GetHeightMinMax(pfImgDst2, nROIWidth2, nROIHeight2, &fHeightMinValue, &fHeightMaxValue);

		dResulFilletHeight = fHeightMaxValue;
	}
	double dResulFilletHeightPercent = dResultMean == 0.0 ? 0 : dResulFilletHeight / dResultMean * 100.0;

	double dGradient = 0;
	BOOL* bArrLeadCoplanarityRst = NULL;
	int nLeadLiftGroupCnt = 0;
	if (pInspAlgoLeadLift->m_bUseCoplanarity == TRUE && sInspAlgoTempResult != NULL)
	{
		if (sInspAlgoTempResult->m_bParentWnd == true)
		{
			nLeadLiftGroupCnt = sInspAlgoTempResult->m_nLeadLiftGroupCnt;
			int nBuf = 10;
			double dHeightDiif = pInspAlgoLeadLift->m_dHeightDiif;
			/*double* dArrX = new double[nLeadLiftGroupCnt * nBuf];
			double* dArrY = new double[nLeadLiftGroupCnt * nBuf];*/
			double* dArrX = g_pMManager->pem_new<double>(true, nLeadLiftGroupCnt * nBuf, (PCHAR)__FUNCTION__, __LINE__);
			double* dArrY = g_pMManager->pem_new<double>(true, nLeadLiftGroupCnt * nBuf, (PCHAR)__FUNCTION__, __LINE__);
			int nIndex = 0;
			int nBufIndex = 0;
			for (int n = 0; n < nLeadLiftGroupCnt * nBuf; n++)
			{
				dArrX[n] = n;
				if (nBufIndex < nBuf)
				{
					dArrY[n] = sInspAlgoTempResult->m_ptrdLeadLiftResult[nIndex];
					nBufIndex++;
				}
				else
				{
					nIndex++;
					dArrY[n] = sInspAlgoTempResult->m_ptrdLeadLiftResult[nIndex];
					nBufIndex = 1;
				}
			}
			//bArrLeadCoplanarityRst = new BOOL[nLeadLiftGroupCnt * nBuf];
			bArrLeadCoplanarityRst = g_pMManager->pem_new<BOOL>(true, nLeadLiftGroupCnt * nBuf, (PCHAR)__FUNCTION__, __LINE__);
			double dA = 0, dB = 0;
			bool bHorizon = true;
			dGradient = m_pCPInsp_Algo->GetGradient(dArrX, dArrY, (double)nLeadLiftGroupCnt * nBuf, dA, dB, bHorizon);
			for (int n = 0; n < nLeadLiftGroupCnt; n++)
			{
				int nCopIDX = (n * nBuf) + (nBuf / 2);
				bArrLeadCoplanarityRst[n] = TRUE;
				double dRstLeadLift = sInspAlgoTempResult->m_ptrdLeadLiftResult[n];
				double dGradientHeight = (dA * nCopIDX) + dB;
				if (sRstAlgo)
				{
					sRstAlgo->m_dArrLeadCopValue[n] = dGradientHeight;
					sRstAlgo->m_dArrLeadCopLeadLiftVal[n] = dRstLeadLift;
				}
				if (dRstLeadLift > dGradientHeight + dHeightDiif)
					bArrLeadCoplanarityRst[n] = FALSE;
			}

			Delete_1DArray(&dArrX);
			Delete_1DArray(&dArrY);
		}
	}

	bResult = TRUE;
	if (sRstAlgo)
	{
		RECT roi;
		roi.left = nROIStartX;
		roi.right = nROIStartX + nROIWidth;
		roi.top = nROIStartY;
		roi.bottom = nROIStartY + nROIHeight;
		sRstAlgo->AddRect(roi);

		sRstAlgo->m_dRstHeight = dResultMean;
		// 2016/06/03	: Min Value가 0보다 클 때만 체크 하도록 변경함
		if ((pInspAlgoLeadLift->dAvgHeight3D >= dResultMean) && (pInspAlgoLeadLift->m_dMinValue == 0 || pInspAlgoLeadLift->m_dMinValue <= dResultMean))
		{
			sRstAlgo->m_bOKHeight = TRUE;
		}

		if (pInspAlgoLeadLift->UseData(m_eLeadLiftData_FilletHeight) == true)
		{
			RECT roi2;
			roi2.left = nROIStartX2;
			roi2.right = nROIStartX2 + nROIWidth2;
			roi2.top = nROIStartY2;
			roi2.bottom = nROIStartY2 + nROIHeight2;
			sRstAlgo->AddRect(roi2);

			sRstAlgo->m_fArr[m_eLeadLiftR_FilletHeight] = dResulFilletHeight;
			if (pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightMax) < dResulFilletHeight || pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightMin) > dResulFilletHeight)
			{
				sRstAlgo->m_nNG |= m_eLeadLiftRData_FilletHeight;
			}

			sRstAlgo->m_fArr[m_eLeadLiftR_FilletHeightPercent] = dResulFilletHeightPercent;
			if (pInspAlgoLeadLift->GetData(m_eLeadLiftF_FilletHeightPercentMin) > dResulFilletHeightPercent)
			{
				sRstAlgo->m_nNG |= m_eLeadLiftRData_FilletHeightPercent;
			}
		}

		if (pInspAlgoLeadLift->m_bUseLeadLift == TRUE)
		{
			if (sRstAlgo->m_bOKHeight == FALSE)
				bResult = FALSE;
		}

		if (sRstAlgo->m_nNG > 0)
			bResult = FALSE;

		sRstAlgo->m_bOKCoplanarity = TRUE;
		sRstAlgo->m_bOKGradient = TRUE;
		sRstAlgo->m_nLeadCoplanarityCnt = nLeadLiftGroupCnt;
		if (pInspAlgoLeadLift->m_bUseCoplanarity == TRUE && bArrLeadCoplanarityRst != NULL)
		{
			if (sInspAlgoTempResult->m_bParentWnd == true)
			{
				for (int n = 0; n < nLeadLiftGroupCnt; n++)
				{
					sRstAlgo->m_nArrLeadCoplanarityRst[n] = -1;
					sRstAlgo->m_nArrLeadCopIDList[n] = sInspAlgoTempResult->m_ptrnLeadWndID[n];
					if (bArrLeadCoplanarityRst[n] == FALSE)
					{
						sRstAlgo->m_nArrLeadCoplanarityRst[n] = sInspAlgoTempResult->m_ptrnLeadWndID[n];
						sRstAlgo->m_bOKCoplanarity = FALSE;
						bResult = sRstAlgo->m_bOKCoplanarity;
					}
				}
				if (pInspAlgoLeadLift->m_bUseGradient == TRUE)
				{
					sRstAlgo->m_dRstGradient = dGradient;
					if (dGradient < 0)
						dGradient *= -1;
					if (dGradient > pInspAlgoLeadLift->m_dGradient)
					{
						sRstAlgo->m_bOKGradient = FALSE;
						bResult = sRstAlgo->m_bOKGradient;
					}
				}
			}
		}
	}

	Delete_1DArray(&pfImgDst);
	Delete_1DArray(&pfImgDst2);
	Delete_1DArray(&bArrLeadCoplanarityRst);
	return bResult;
}

BOOL CPInsp_AlgoLeadLift::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoLeadLift::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoLeadLift::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}