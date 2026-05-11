#include "PInsp_AlgoGrayDiff.h"


CPInsp_AlgoGrayDiff::CPInsp_AlgoGrayDiff(void)
{
}


CPInsp_AlgoGrayDiff::~CPInsp_AlgoGrayDiff(void)
{
}

void CPInsp_AlgoGrayDiff::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoGrayDiff::GetInspAlgoData()
{
	return eSPCAlgoGray_Diff;
}

int CPInsp_AlgoGrayDiff::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoGrayDiff * rstAlgo = (RstAlgoGrayDiff *)vRstInspAlgo;
	if (nWndType == eINSP_SOLDER)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}
	else
	{
		if (rstAlgo->m_bCheckPolarity)
		{
			eWholeNgTypeTemp[TypeMountPolarity] = e_NG;
			nCurrentNgType = TypeMountPolarity;
		}
		else
		{
			eWholeNgTypeTemp[TypeMountWorng] = e_NG;
			nCurrentNgType = TypeMountWorng;
		}
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoGrayDiff::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoGrayDiff::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspGrayDiff(sInspAlgo, sWndAlgoImg, (RstAlgoGrayDiff *)sRstAlgo, stAlgoParam.m_sAlignRes, stAlgoParam.m_nAlignCnt);

	return bResult;
}

float CPInsp_AlgoGrayDiff::GetMeanImg(UCHAR *pUcImgSrc, int nImgWidth, int nImgHeight)
{
	float fReturnRate(0);

	IppiSize roiSize;
	roiSize.width = nImgWidth;
	roiSize.height = nImgHeight;
	Ipp64f mean;
	ippiMean_8u_C1R(pUcImgSrc, nImgWidth, roiSize, &mean);

	fReturnRate = mean;

	return fReturnRate;
}
float CPInsp_AlgoGrayDiff::GetMinMaxImg(UCHAR *pUcImgSrc, int nImgWidth, int nImgHeight, int Min)
{
	float fReturnRate(0);

	IppiSize roiSize;
	roiSize.width = nImgWidth;
	roiSize.height = nImgHeight;
	Ipp64f mean;
	Ipp8u rst;

	switch (Min)
	{
	case 1:
		ippiMin_8u_C1R(pUcImgSrc, nImgWidth, roiSize, &rst);
		fReturnRate = rst;
		break;
	case 2:
		ippiMax_8u_C1R(pUcImgSrc, nImgWidth, roiSize, &rst);
		fReturnRate = rst;
		break;
	case 0:
	default:
		ippiMean_8u_C1R(pUcImgSrc, nImgWidth, roiSize, &mean);
		fReturnRate = mean;
		break;
	}

	return fReturnRate;
}
BOOL CPInsp_AlgoGrayDiff::InspGrayDiff(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoGrayDiff *sRstAlgo, AlignResult * sAlignRes, int nAlignResCnt)
{
	BOOL bReturn = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoGrayDiff));
		sRstAlgo->Init();
	}

	if (sInspAlgo.m_eAlgoType != eAlgoGray_Diff)
		return bReturn;
	if (!m_pProcMilAlgo)
		return bReturn;

	AlgoGrayDiff *pInspAlgoGrayDiff = (AlgoGrayDiff *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoGrayDiff)
		return bReturn;
	// 	if((pInspAlgoGrayDiff->rcFirstROI.left < 0) || (pInspAlgoGrayDiff->rcFirstROI.top < 0) ||
	// 		(pInspAlgoGrayDiff->rcFirstROI.right <= 0) || (pInspAlgoGrayDiff->rcFirstROI.bottom <=0))
	// 		return bReturn;
	// 	if((pInspAlgoGrayDiff->rcSeccondROI.left < 0) || (pInspAlgoGrayDiff->rcSeccondROI.top < 0) ||
	// 		(pInspAlgoGrayDiff->rcSeccondROI.right <= 0) || (pInspAlgoGrayDiff->rcSeccondROI.bottom <=0))
	// 		return bReturn;

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bReturn;

	RECT rcFstDstRect;
	if (m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoGrayDiff->rcFirstROI, &rcFstDstRect) == FALSE)
		return bReturn;

	RECT rcSecDstRect;
	if (m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoGrayDiff->rcSeccondROI, &rcSecDstRect) == FALSE)
		return bReturn;

	UCHAR *pucArrFirstRoiImg = NULL;
	UCHAR *pucArrSeccondRoiImg = NULL;

	double dFirstWidth = rcFstDstRect.right - rcFstDstRect.left;
	double dFirstHeight = rcFstDstRect.bottom - rcFstDstRect.top;
	// 	double dFirstX = rcFstDstRect.left + (dFirstWidth / 2.0);
	// 	double dFirstY = rcFstDstRect.top + (dFirstHeight / 2.0);
	double dFirstX = rcFstDstRect.left;
	double dFirstY = rcFstDstRect.top;
	if (nAlignResCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
	{
		dFirstX = dFirstX + (dFirstWidth / 2.0) - (double)nImgWidth / 2.0;
		dFirstY = (double)nImgHeight / 2.0 - (dFirstY + (dFirstHeight / 2.0));

		double corr_x = 0, corr_y = 0;
		for (int n = 0; n < nAlignResCnt; n++)
		{
			double corr_x_Buf = 0, corr_y_Buf = 0;
			//m_proc3d.CorrectCoordinate(dFirstX, dFirstY, 0, 0, sAlignRes->theta, sAlignRes->offsetX / m_resolX, -sAlignRes->offsetY / m_resolY, &corr_x, &corr_y);
			int nCorrectCoordinate = m_proc3d.CorrectCoordinate(dFirstX, dFirstY, sAlignRes[n].centerX / m_resolX, sAlignRes[n].centerY / m_resolY, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
			if (n == 0)
			{
				corr_x = corr_x_Buf;
				corr_y = corr_y_Buf;
			}
			else
			{
				if (nCorrectCoordinate == 1)
				{
					corr_x += corr_x_Buf;
					corr_y += corr_y_Buf;
				}
			}
		}
		dFirstX = RounD(corr_x + ((double)nImgWidth / 2.0) - (dFirstWidth / 2.0));
		dFirstY = RounD(((double)nImgHeight / 2.0) - (corr_y + (dFirstHeight / 2.0)));

		if (dFirstX + RounD(dFirstWidth) > nImgWidth)		// 2014/12/15	: 예외처리
			dFirstX = nImgWidth - dFirstWidth;
		if (dFirstY + RounD(dFirstHeight) > nImgHeight)
			dFirstY = nImgHeight - dFirstHeight;
	}

	double dSeccondWidth = rcSecDstRect.right - rcSecDstRect.left;
	double dSeccondHeight = rcSecDstRect.bottom - rcSecDstRect.top;
	// 	double dSeccondX = rcSecDstRect.left + (dSeccondWidth / 2.0);
	// 	double dSeccondY = rcSecDstRect.top + (dSeccondHeight / 2.0);
	double dSeccondX = rcSecDstRect.left;
	double dSeccondY = rcSecDstRect.top;
	if (nAlignResCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
	{
		dSeccondX += (dSeccondWidth / 2.0);
		dSeccondX -= (double)nImgWidth / 2.0;

		dSeccondY += (dSeccondHeight / 2.0);
		dSeccondY -= (double)nImgHeight / 2.0;

		double corr_x = 0, corr_y = 0;
		for (int n = 0; n < nAlignResCnt; n++)
		{
			double corr_x_Buf = 0, corr_y_Buf = 0;
			//m_proc3d.CorrectCoordinate(dFirstX, dFirstY, 0, 0, sAlignRes->theta, sAlignRes->offsetX / m_resolX, -sAlignRes->offsetY / m_resolY, &corr_x, &corr_y);
			int nCorrectCoordinate = m_proc3d.CorrectCoordinate(dSeccondX, dSeccondY, 0, 0, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, -sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
			if (n == 0)
			{
				corr_x = corr_x_Buf;
				corr_y = corr_y_Buf;
			}
			else
			{
				if (nCorrectCoordinate == 1)
				{
					corr_x += corr_x_Buf;
					corr_y += corr_y_Buf;
				}
			}
		}
		dSeccondX = RounD(corr_x + ((double)nImgWidth / 2.0) - (dSeccondWidth / 2.0));
		dSeccondY = RounD(corr_y + ((double)nImgHeight / 2.0) - (dSeccondHeight / 2.0));

		if (dSeccondX + RounD(dSeccondWidth) > nImgWidth)		// 2014/12/15	: 예외처리
			dSeccondX = nImgWidth - dSeccondWidth;
		if (dSeccondY + RounD(dSeccondHeight) > nImgHeight)
			dSeccondY = nImgHeight - dSeccondHeight;
	}

	int nFirstImgSize = dFirstWidth * dFirstHeight;
	//pucArrFirstRoiImg = new UCHAR[nFirstImgSize];
	pucArrFirstRoiImg = g_pMManager->pem_new<UCHAR>(true, nFirstImgSize, (PCHAR)__FUNCTION__, __LINE__);

	int nSeccondImgSize = dSeccondWidth * dSeccondHeight;
	//pucArrSeccondRoiImg = new UCHAR[nSeccondImgSize];
	pucArrSeccondRoiImg = g_pMManager->pem_new<UCHAR>(true, nSeccondImgSize, (PCHAR)__FUNCTION__, __LINE__);

	m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight,
		pucArrFirstRoiImg, dFirstX, dFirstY, dFirstWidth, dFirstHeight);
	m_pProcMilAlgo->SaveWorkImg(pucArrFirstRoiImg, dFirstWidth, dFirstHeight, _T("FirstROIImage.bmp"));

	m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight,
		pucArrSeccondRoiImg, dSeccondX, dSeccondY, dSeccondWidth, dSeccondHeight);
	m_pProcMilAlgo->SaveWorkImg(pucArrSeccondRoiImg, dSeccondWidth, dSeccondHeight, _T("SeccondROIImage.bmp"));
	int nMinMaxflag = pInspAlgoGrayDiff->m_nMinMaxflag;
	int nArrType[2];
	nArrType[0] = nMinMaxflag / 3;
	nArrType[1] = nMinMaxflag % 3;

	//float* pfArrROI = g_pMManager->pem_new<float>(true, nArrW[nIdx] * nArrH[nIdx], (PCHAR)__FUNCTION__, __LINE__);

	float fFristROIMinMax = GetMinMaxImg(pucArrFirstRoiImg, dFirstWidth, dFirstHeight, nArrType[0]);
	float fSeccondROIMinMax = GetMinMaxImg(pucArrSeccondRoiImg, dSeccondWidth, dSeccondHeight, nArrType[1]);

	
	bReturn = TRUE;
	 
	if (sRstAlgo)
	{
		sRstAlgo->m_rcRect_I[0].left = dFirstX;
		sRstAlgo->m_rcRect_I[0].right = dFirstX + dFirstWidth;
		sRstAlgo->m_rcRect_I[0].top = dFirstY;
		sRstAlgo->m_rcRect_I[0].bottom = dFirstY + dFirstHeight;

		sRstAlgo->m_rcRect_I[1].left = dSeccondX;
		sRstAlgo->m_rcRect_I[1].right = dSeccondX + dSeccondWidth;
		sRstAlgo->m_rcRect_I[1].top = dSeccondY;
		sRstAlgo->m_rcRect_I[1].bottom = dSeccondY + dSeccondHeight;

		sRstAlgo->m_bCheckPolarity = pInspAlgoGrayDiff->m_bPolarity;
		sRstAlgo->m_dRstF = fFristROIMinMax;
		sRstAlgo->m_dRstS = fSeccondROIMinMax;
		sRstAlgo->m_nRstGrayDiff = (sRstAlgo->m_dRstF - sRstAlgo->m_dRstS);

		if (pInspAlgoGrayDiff->nGrayDiff != 0 && pInspAlgoGrayDiff->m_bSignInversion == TRUE)
		{
			BOOL bPlusRst = sRstAlgo->m_nRstGrayDiff >= 0;
			BOOL bPlusParam = pInspAlgoGrayDiff->nGrayDiff >= 0;
			if (bPlusRst != bPlusParam)		// 차이값의 부호가 다르면 NG 로 처리
				bReturn = FALSE;
		}
		if (bReturn)
		{
			if (pInspAlgoGrayDiff->m_bDiffUpper)
				bReturn = sRstAlgo->m_nRstGrayDiff < pInspAlgoGrayDiff->nGrayDiff;
			else
				bReturn = sRstAlgo->m_nRstGrayDiff > pInspAlgoGrayDiff->nGrayDiff;
		}
	}

	if (pucArrFirstRoiImg)
		Delete_1DArray(&pucArrFirstRoiImg);
	if (pucArrSeccondRoiImg)
		Delete_1DArray(&pucArrSeccondRoiImg);

	return bReturn;
}

BOOL CPInsp_AlgoGrayDiff::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoGrayDiff::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoGrayDiff::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}