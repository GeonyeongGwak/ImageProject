#include "PInsp_AlgoBW.h"


CPInsp_AlgoBW::CPInsp_AlgoBW(void)
{
}


CPInsp_AlgoBW::~CPInsp_AlgoBW(void)
{
}

void CPInsp_AlgoBW::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoBW::GetInspAlgoData()
{
	return eSPCAlgoBW;
}

int CPInsp_AlgoBW::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoBlackWhite * rstAlgo = (RstAlgoBlackWhite *)vRstInspAlgo;
	if (nWndType == eINSP_SOLDER)
	{
		if (rstAlgo->m_bNoSolder)
		{
			eWholeNgTypeTemp[TypeNoSolder] = e_NG;
			nCurrentNgType = TypeNoSolder;
		}
		else
		{
			eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
			nCurrentNgType = TypeSolderFilet;
		}
		if (rstAlgo->m_bOKMax == FALSE)
		{
			eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
			nCurrentNgType = TypeSolderExcess;
		}
		if (rstAlgo->m_bOKHeightMean == FALSE || rstAlgo->m_sInspAC.m_nOKInspAC > 0)
		{
			eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
			nCurrentNgType = TypeSolderFilet;
		}
	}
	else
	{
		eWholeNgTypeTemp[TypeMountWorng] = e_NG;
		nCurrentNgType = TypeMountWorng;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoBW::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return true;
	return true;
}

BOOL CPInsp_AlgoBW::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	if (stAlgoParam.m_bInspection == TRUE)
	{
		bResult = InspBlackAndWhite(sInspAlgo, sWndAlgoImg, *sInspImageData, (RstAlgoBlackWhite *)sRstAlgo, stTieArea, stAlgoParam.m_poCenter, ucArrDstImg);

		if (stAlgoParam.m_bInspection == TRUE)
		{
			RstAlgoBlackWhite * rst = (RstAlgoBlackWhite *)sRstAlgo;
			rst->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].left += stAlgoParam.m_dx;
			rst->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].right += stAlgoParam.m_dx;
			rst->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].top += stAlgoParam.m_dy;
			rst->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].bottom += stAlgoParam.m_dy;
		}
	}
	else
	{
		bResult = TeachBlackAndWhite(sInspAlgo, *stAlgoParam.m_sPartAlgoImg, sWndAlgoImg, *stAlgoParam.m_sWndInfo, (RstAlgoBlackWhite *)sRstAlgo, ucArrDstImg, stAlgoParam, sInspImageData, stAlgoParam.m_sInspImageData, stTieArea);
	}

	return bResult;
}

BOOL CPInsp_AlgoBW::InspBlackAndWhite(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImgBuf, RstAlgoBlackWhite *sRstAlgo, TotalInspExceptArea stTieArea, POINTF poCenter, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	int nSideCameraIndex = -1;

	nSideCameraIndex = sWndAlgoImg.m_nLight_index;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, (sizeof(RstAlgoBlackWhite)));
		sRstAlgo->Init();
	}

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	if (sInspAlgo.m_eAlgoType != eAlgoBW)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	AlgoBW *pInspAlgoBW = (AlgoBW *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoBW)
		return bResult;
	if (!pInspAlgoBW->m_b2dCheck && !pInspAlgoBW->m_b3dCheck && !pInspAlgoBW->m_sAlgoColorBase.m_bUseColor)
		return bResult;

	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	UCHAR *ucColorImgDst = NULL;
	if (pColorTeach != NULL && pInspAlgoBW->m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		Delete_1DArray(&ucColorImgDst);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImgBuf.nImageSizeX * sInspImgBuf.nImageSizeY);
		pColorTeach->GetColorBaseBin(&pInspAlgoBW->m_sAlgoColorBase, sInspImgBuf, ucColorImgDst, false, 0, 0, -1, nSideCameraIndex);
		m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImgBuf.nImageSizeX, sInspImgBuf.nImageSizeY, _T("OrgColor_Clip.bmp"));
	}
	bool bInspHeightMean = false;
	if (ucArrDstImg == NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrDstImg, nImgWidth * nImgHeight);
		bInspHeightMean = true;
	}
#if _DEBUG
	cv::Mat imgDst(nImgHeight, nImgWidth, CV_8UC1, ucArrDstImg);
#endif
	// 통합 검사/제외 영역에 넣어준다
	stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
	stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
	//for (int i = 0; i < MAX_MASKING_NUM; i++)
	//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
	//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
	//	stTieArea.m_rcArrMaskingROI.push_back(sInspAlgo.m_rcArrMaskingROI[i]);
	//stTieArea.m_rcArrMaskingROI.resize(sInspAlgo.m_nUsedMaskingValue);
	//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
	//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
	stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
	stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
	for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];
	stTieArea.dAngle = sWndAlgoImg.dAngle;
	int nInspectionAreaPer = pInspAlgoBW->m_byInspectionArea;
	if (nInspectionAreaPer <= 0) nInspectionAreaPer = 1;
	else if (nInspectionAreaPer > 100) nInspectionAreaPer = 100;
	int nInspectionAreaPerH = nInspectionAreaPer;
	if ((pInspAlgoBW->m_byUseHeightOpt & m_eBWHeigthMean_Area) == m_eBWHeigthMean_Area)
		nInspectionAreaPer = 100;

	AlgoBlackWhite pInspAlgoBW2;
	pInspAlgoBW2.m_bInvertCheck = pInspAlgoBW->m_bInvertCheck;
	pInspAlgoBW2.m_dPercentOK = pInspAlgoBW->m_dPercentOK;
	pInspAlgoBW2.m_bChipTracking = pInspAlgoBW->m_bChipTracking;
	pInspAlgoBW2.m_b2dCheck = pInspAlgoBW->m_b2dCheck;
	pInspAlgoBW2.m_nRange = pInspAlgoBW->m_nRange;
	pInspAlgoBW2.m_nMinValue = pInspAlgoBW->m_nMinValue;
	pInspAlgoBW2.m_nMaxValue = pInspAlgoBW->m_nMaxValue;
	pInspAlgoBW2.m_b3dCheck = pInspAlgoBW->m_b3dCheck;
	pInspAlgoBW2.m_n3dRange = pInspAlgoBW->m_n3dRange;
	pInspAlgoBW2.m_d3dHeightMin = pInspAlgoBW->m_d3dHeightMin;
	pInspAlgoBW2.m_d3dHeightMax = pInspAlgoBW->m_d3dHeightMax;
	pInspAlgoBW2.m_d3dAvgHeight = pInspAlgoBW->m_d3dAvgHeight;
	pInspAlgoBW2.m_bUseTeachingRate = pInspAlgoBW->m_bUseTeachingRate;
	pInspAlgoBW2.m_dAreaCurrent = pInspAlgoBW->m_dAreaCurrent;
	pInspAlgoBW2.m_dTeachingArea = pInspAlgoBW->m_dTeachingArea;
	pInspAlgoBW2.m_nStdOKArea = pInspAlgoBW->m_nStdOKArea;
	pInspAlgoBW2.m_sAlgoColorBase = pInspAlgoBW->m_sAlgoColorBase;
	pInspAlgoBW2.m_nChipTrackingGap = pInspAlgoBW->m_nChipTrackingGap;
	double dRate = m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoBW2, sWndAlgoImg, ucArrDstImg, stTieArea, ucColorImgDst, pInspAlgoBW->m_byDir, nInspectionAreaPer);
	float fWhiteHeigthAvg = 0.0f;
	if ((pInspAlgoBW->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) == eDefaultAC_Data_Use && sRstAlgo)
	{
		UCHAR * pucACDst = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucACDst, nImgWidth * nImgHeight * 3);
		memset(pucACDst, 0, nImgWidth * nImgHeight * 3 * sizeof(UCHAR));
		if (pInspAlgoBW->m_sAngleColorBase.m_byArrInspAC[m_eInspAC_T2_InspArea] > 100 && pInspAlgoBW->m_sAngleColorBase.m_byArrInspAC[m_eInspAC_T2_InspArea] > nInspectionAreaPer)
		{
			UCHAR * ucArrClipBWDstAC = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrClipBWDstAC, nImgWidth * nImgHeight);
			memset(ucArrClipBWDstAC, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
			m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoBW2, sWndAlgoImg, ucArrClipBWDstAC, stTieArea, ucColorImgDst, pInspAlgoBW->m_byDir, pInspAlgoBW->m_sAngleColorBase.m_byArrInspAC[m_eInspAC_T2_InspArea]);
			sRstAlgo->m_sInspAC.m_nInspAC = InspAC(pInspAlgoBW->m_sAngleColorBase, sInspImgBuf, ucArrClipBWDstAC, sWndAlgoImg, pInspAlgoBW->m_byDir, pucACDst, sRstAlgo->m_sInspAC.m_fRstInspAC, &sRstAlgo->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ], false, poCenter);
			Delete_1DArray(&ucArrClipBWDstAC);
		}
		else
			sRstAlgo->m_sInspAC.m_nInspAC = InspAC(pInspAlgoBW->m_sAngleColorBase, sInspImgBuf, ucArrDstImg, sWndAlgoImg, pInspAlgoBW->m_byDir, pucACDst, sRstAlgo->m_sInspAC.m_fRstInspAC, &sRstAlgo->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ], false, poCenter);
		Delete_1DArray(&pucACDst);
	}

	if (ucArrDstImg != NULL && g_pMPTI->m_bUseAIModule && ((g_pMPTI->m_nAIOption & m_eAIOPT_SOLDER_AutoInsp) == m_eAIOPT_SOLDER_AutoInsp))
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &g_pInspMng->m_pucWndBinImg, nImgWidth * nImgHeight);
		memset(g_pInspMng->m_pucWndBinImg, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
		memcpy(g_pInspMng->m_pucWndBinImg, ucArrDstImg, sizeof(UCHAR) * nImgWidth * nImgHeight);
	}

	if (pInspAlgoBW->m_bUseHeightMean == TRUE && ucArrDstImg != NULL)
	{
		int nDir = pInspAlgoBW->m_byDir;
		int nW = nImgWidth;
		int nH = nImgHeight;
		int nX = 0;
		int nY = 0;
		if (nDir == 0)
		{
			int nTemp = nImgWidth * (nInspectionAreaPerH / 100.0);
			nX = nImgWidth - nTemp;
		}
		else if (nDir == 1)
			nW = nImgWidth * (nInspectionAreaPerH / 100.0);

		if (nDir == 2)
		{
			int nTemp = nImgHeight * (nInspectionAreaPerH / 100.0);
			nY = nImgHeight - nTemp;
		}
		else if (nDir == 3)
			nH = nImgHeight * (nInspectionAreaPerH / 100.0);
		float fSumH = 0.0f;
		int nSumCnt = 0;
		for (int x = nX; x < nW; x++)
		{
			for (int y = nY; y < nH; y++)
			{
				int nIndex = (y * nImgWidth) + x;
				if (pInspAlgoBW->m_bInspBWArea == FALSE)
				{
					fSumH += pfImgSrc[nIndex];
					nSumCnt++;
				}
				else if (pInspAlgoBW->m_bInspBWArea == TRUE && ucArrDstImg[nIndex] == 255)
				{
					fSumH += pfImgSrc[nIndex];
					nSumCnt++;
				}
				else if ((pInspAlgoBW->m_byUseHeightOpt & m_eBWHeigthMeanIncludeBlackArea) == m_eBWHeigthMeanIncludeBlackArea)
					nSumCnt++;
			}
		}
		if (fSumH != 0 && nSumCnt > 0)
			fWhiteHeigthAvg = fSumH / nSumCnt;
	}

	bResult = TRUE;
	g_pInspMng->m_bSetAIData = false;

	double dResolX = m_resolX;
	double dResolY = m_resolY;

	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}

	Delete_1DArray(&ucColorImgDst);
	if (bInspHeightMean == true)
		Delete_1DArray(&ucArrDstImg);
	if (sRstAlgo)
	{
		sRstAlgo->m_bOKBW = TRUE;
		double dPercentOK = 0.0;
		double dMaxPercentOK = 0.0;
		if (pInspAlgoBW->m_bUseTeachingRate == true)
		{
			dMaxPercentOK = pInspAlgoBW->m_dTeachingArea * (pInspAlgoBW->m_nMaxOKPer / 100.0);
			dPercentOK = pInspAlgoBW->m_dTeachingArea * (pInspAlgoBW->m_dPercentOK / 100.0);
			dRate = pInspAlgoBW2.m_dAreaCurrent * /*m_resolX*/dResolX * /*m_resolY*/dResolY;
		}
		else
		{
			dMaxPercentOK = pInspAlgoBW->m_nMaxOKPer;
			dPercentOK = pInspAlgoBW->m_dPercentOK;
			double dStdOK = pInspAlgoBW->m_nStdOKArea;
			if (dStdOK < 0 || dStdOK > 100) dStdOK = 1;
			dRate *= ((double)100 / dStdOK);
		}
		sRstAlgo->m_bOKMax = TRUE;
		if (pInspAlgoBW->m_bUseMaxOK == TRUE && dMaxPercentOK < dRate)
		{
			bResult = FALSE;
			sRstAlgo->m_bOKBW = FALSE;
			sRstAlgo->m_bOKMax = FALSE;
		}
		double dNoSolderStandardValue = 5;
		CPInsp_Algo* InsAlgo = g_pInspMng->GetPtrInspAlgo();
		if (InsAlgo)
			dNoSolderStandardValue = InsAlgo->m_dNO_SOLDER;
		double dTeachAreaPer = dRate;
		if (pInspAlgoBW->m_bUseTeachingRate)
		{
			if (dRate == 0 || pInspAlgoBW->m_dTeachingArea == 0)
				dTeachAreaPer = 0;
			else
				dTeachAreaPer = dRate / pInspAlgoBW->m_dTeachingArea * 100.0;
		}
		if (pInspAlgoBW->m_bStdZeroValue)
		{
			if (dPercentOK > dRate || (pInspAlgoBW->m_bUseTeachingRate == true && pInspAlgoBW->m_dTeachingArea == 0))	// LMJ : TeachArea가 0이면 NG 처리
			{
				bResult = FALSE;
				sRstAlgo->m_bOKBW = FALSE;
				if (dTeachAreaPer < dNoSolderStandardValue)
					sRstAlgo->m_bNoSolder = TRUE;
				else
					sRstAlgo->m_bNoSolder = FALSE;
			}
		}
		else
		{
			if (dPercentOK > dRate || dRate == 0 || (pInspAlgoBW->m_bUseTeachingRate == true && pInspAlgoBW->m_dTeachingArea == 0))	// LMJ : TeachArea가 0이면 NG 처리
			{
				bResult = FALSE;
				sRstAlgo->m_bOKBW = FALSE;
				if (dTeachAreaPer < dNoSolderStandardValue)
					sRstAlgo->m_bNoSolder = TRUE;
				else
					sRstAlgo->m_bNoSolder = FALSE;
			}
		}

		if (pInspAlgoBW->m_bUseHeightMean == TRUE)
		{
			sRstAlgo->m_dRstHeightMean = fWhiteHeigthAvg;
			sRstAlgo->m_bOKHeightMean = TRUE;
			if ((pInspAlgoBW->m_byUseHeightOpt & m_eBWHeigthMeanMin) == m_eBWHeigthMeanMin)
			{
				if (fWhiteHeigthAvg < pInspAlgoBW->m_fHeightMeanMin)
				{
					bResult = FALSE;
					sRstAlgo->m_bOKHeightMean = FALSE;
				}
			}
			if ((pInspAlgoBW->m_byUseHeightOpt & m_eBWHeigthMeanMax) == m_eBWHeigthMeanMax)
			{
				if (fWhiteHeigthAvg > pInspAlgoBW->m_fHeightMeanMax)
				{
					bResult = FALSE;
					sRstAlgo->m_bOKHeightMean = FALSE;
				}
			}
		}
		else
		{
			sRstAlgo->m_bOKHeightMean = TRUE;
			sRstAlgo->m_dRstHeightMean = 0;
		}
		sRstAlgo->m_dRstPercent = dRate;
		sRstAlgo->m_sInspAC.SetRstData(pInspAlgoBW->m_sAngleColorBase);
		if (sRstAlgo->m_sInspAC.m_nOKInspAC > 0)
			bResult = FALSE;

		if (sRstAlgo->m_bOKBW == FALSE && sRstAlgo->m_bOKHeightMean == TRUE && sRstAlgo->m_sInspAC.m_nOKInspAC == 0)
			g_pInspMng->m_bSetAIData = true;

		int nInspAC = pInspAlgoBW->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data];
		bool bInspAC = ((nInspAC & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
		bool bInspAC_Ess = (bInspAC && ((nInspAC & eDefaultAC_Data_Essential) == eDefaultAC_Data_Essential));
		if (bInspAC_Ess)
		{
			g_pInspMng->m_bSetAIData = false;
			bResult = TRUE;
			if (sRstAlgo->m_sInspAC.m_nOKInspAC > 0)
				bResult = FALSE;
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoBW::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoBW::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoBW * pInspAlgo = (AlgoBW *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->m_sAlgoColorBase.GetColorData();
	nData |= COLOR_DATA_WND;
	if (pInspAlgo->m_sAngleColorBase.UseAC())
		nData |= COLOR_DATA_AC;

	return nData;
}

BOOL CPInsp_AlgoBW::TeachBlackAndWhite(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, WndInfo sWndInfo, RstAlgoBlackWhite *sRstAlgo, UCHAR* puImgDst, InspAlgoParam stAlgoParam, InspRoiImgBuf* sInspImageData, InspRoiImgBuf* sClipImgBuf, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;

	int nLine = __LINE__;
	try
	{
		if (sRstAlgo)
		{
			memset(sRstAlgo, 0, (sizeof(RstAlgoBlackWhite)));
			sRstAlgo->Init();
		}

		if (sInspAlgo.m_eAlgoType != eAlgoBW || !m_pProcMilAlgo)
			return bResult;

		AlgoBW *pInspAlgoBW = (AlgoBW *)sInspAlgo.m_ptrInspAlgoParam;
		if (!pInspAlgoBW || ((!pInspAlgoBW->m_b2dCheck && !pInspAlgoBW->m_b3dCheck && !pInspAlgoBW->m_sAlgoColorBase.m_bUseColor)))
			return bResult;
		nLine = __LINE__;

		double dResolX = m_resolX;
		double dResolY = m_resolY;
		if (g_pMPTI->m_bSideOriginalSize == true)
		{
			dResolX = g_pMPTI->m_dBtmSideResX;
			dResolY = g_pMPTI->m_dBtmSideResY;
		}
		nLine = __LINE__;
		// LMJ 2021/08/08 : GetWndClip_3() 함수에서와 좌표 계산 동일하게 맞춰줌
		int nClipWidth = RounD((float)(sWndInfo.dWidth / dResolX));
		int nClipHeight = RounD((float)(sWndInfo.dLength / dResolY));

		float fClipPartWidth = RounD(sWndInfo.m_dPartWidth / dResolX);
		float fClipPartHeight = RounD(sWndInfo.m_dPartHeight / dResolY);

		int nClipStartX = 0, nClipStartY = 0;
		if (nClipWidth < sWndAlgoImg.m_nWidth && nClipHeight < sWndAlgoImg.m_nHeight)
		{
			float dClipX = (fClipPartWidth / 2.) + (sWndInfo.dCenterX / dResolX) - ((float)nClipWidth / 2.);
			float dClipY = (fClipPartHeight / 2.) - (sWndInfo.dCenterY / dResolY) - ((float)nClipHeight / 2.);

			if (dClipX < 0)
				dClipX = 0;
			if (dClipY < 0)
				dClipY = 0;

			//Get Part clip Rounding-Err
			float fPartClipErrX = sWndAlgoImg.m_fPartRoundingErrX;
			float fPartClipErrY = sWndAlgoImg.m_fPartRoundingErrY;
			dClipX -= fPartClipErrX;
			dClipY -= fPartClipErrY;

			nClipStartX = RounD(dClipX);
			nClipStartY = RounD(dClipY);
		}
		nLine = __LINE__;

		// BW 를 Blackandwhite algo.  이외에 UI 표시나 teach 용으로 사용하는 경우에 표시 할 수도 안 할수도 있다
		// UI상에서 그려야 한다고 하는 경우, 좌표를 보정한다. part 좌표를 wnd 좌표 중심으로 변경함.
		RECT rcArrPARTMaskingROI[MAX_MASKING_NUM];
		int nUsedPARTMaskingValue = sInspAlgo.m_nUsedMaskingValue;

		POINTF ptArrPARTPolygonROI[MAX_INSP_AREA_COUNT];
		int nUsedPARInspPolygon = sInspAlgo.m_nUsedInspPolygon;

		// 통합 검사/제외 영역에 넣어준다
		stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
		stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
		//for (int i = 0; i < MAX_MASKING_NUM; i++)
		//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
		//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
		//	stTieArea.m_rcArrMaskingROI.push_back(sInspAlgo.m_rcArrMaskingROI[i]);
		//stTieArea.m_rcArrMaskingROI.resize(sInspAlgo.m_nUsedMaskingValue);
		//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
		//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
		stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
		stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];
		nLine = __LINE__;

		// 원래 이미지 2진화
		UCHAR *pUcOrgSrc = sWndAlgoImg.m_ucArr2D;
		float *pfOrgSrc = sWndAlgoImg.m_fArr3D;
		int nOrgWidth = sWndAlgoImg.m_nWidth;
		int nOrgHeight = sWndAlgoImg.m_nHeight;
		if ((pUcOrgSrc == NULL) || (nOrgWidth <= 0) || (nOrgHeight <= 0))
			return bResult;
		nLine = __LINE__;

		CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
		if (puImgDst)
		{
			nLine = __LINE__;
			IppStatus sts;
			IppiSize iSize = { nOrgWidth, nOrgHeight };
			sts = ippiCopy_8u_C1R(pUcOrgSrc, nOrgWidth, puImgDst, nOrgWidth, iSize);
			m_pProcMilAlgo->SaveWorkImg(pUcOrgSrc, nOrgWidth, nOrgHeight, _T("Origin2D_Part.bmp"));
			m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("Dst2D_Part1.bmp"));
			nLine = __LINE__;
		}

		BOOL bUseBlobOption = stAlgoParam.m_bUseBlobOption;
		BOOL bFillHole = stAlgoParam.m_bFillHole;
		int nMinBlobArea = stAlgoParam.m_nBlobMinArea;
		int nTypeSelectBlob = stAlgoParam.m_nTypeSelectBlob;

		int nInspectionAreaPer = pInspAlgoBW->m_byInspectionArea;
		if (nInspectionAreaPer <= 0) nInspectionAreaPer = 1;
		else if (nInspectionAreaPer > 100) nInspectionAreaPer = 100;
		int nInspectionAreaPerH = nInspectionAreaPer;
		bool bHM_A = ((pInspAlgoBW->m_byUseHeightOpt & m_eBWHeigthMean_Area) == m_eBWHeigthMean_Area);
		if (bHM_A)
			nInspectionAreaPer = 100;

		double dCX = 0;
		double dCY = 0;
		double dArea = 0;
		CRect rcBlob(0, 0, 0, 0);
		nLine = __LINE__;
		UCHAR *ucColorImgDst = NULL;
		if (pColorTeach != NULL && pInspAlgoBW->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			nLine = __LINE__;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData->nImageSizeX * sInspImageData->nImageSizeY);
			pColorTeach->GetColorBaseBin(&pInspAlgoBW->m_sAlgoColorBase, *sInspImageData, ucColorImgDst, true, 0, 0, -1, sWndAlgoImg.m_nLight_index);
			m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImageData->nImageSizeX, sInspImageData->nImageSizeY, _T("OrgColor_Part.bmp"));
			nLine = __LINE__;
		}
		AlgoBlackWhite pInspAlgoBW2;
		pInspAlgoBW2.m_bInvertCheck = pInspAlgoBW->m_bInvertCheck;
		pInspAlgoBW2.m_dPercentOK = pInspAlgoBW->m_dPercentOK;
		pInspAlgoBW2.m_bChipTracking = pInspAlgoBW->m_bChipTracking;
		pInspAlgoBW2.m_b2dCheck = pInspAlgoBW->m_b2dCheck;
		pInspAlgoBW2.m_nRange = pInspAlgoBW->m_nRange;
		pInspAlgoBW2.m_nMinValue = pInspAlgoBW->m_nMinValue;
		pInspAlgoBW2.m_nMaxValue = pInspAlgoBW->m_nMaxValue;
		pInspAlgoBW2.m_b3dCheck = pInspAlgoBW->m_b3dCheck;
		pInspAlgoBW2.m_n3dRange = pInspAlgoBW->m_n3dRange;
		pInspAlgoBW2.m_d3dHeightMin = pInspAlgoBW->m_d3dHeightMin;
		pInspAlgoBW2.m_d3dHeightMax = pInspAlgoBW->m_d3dHeightMax;
		pInspAlgoBW2.m_d3dAvgHeight = pInspAlgoBW->m_d3dAvgHeight;
		pInspAlgoBW2.m_bUseTeachingRate = pInspAlgoBW->m_bUseTeachingRate;
		pInspAlgoBW2.m_dAreaCurrent = pInspAlgoBW->m_dAreaCurrent;
		pInspAlgoBW2.m_dTeachingArea = pInspAlgoBW->m_dTeachingArea;
		pInspAlgoBW2.m_nStdOKArea = pInspAlgoBW->m_nStdOKArea;
		pInspAlgoBW2.m_sAlgoColorBase = pInspAlgoBW->m_sAlgoColorBase;
		pInspAlgoBW2.m_nChipTrackingGap = pInspAlgoBW->m_nChipTrackingGap;
		// 화면에 그리기 위해 만듬. (full part 기준)
		TotalInspExceptArea stTieArea2;
		stTieArea2.m_nUsedInspPolygon = 0;
		stTieArea2.m_nUsedMaskingValue = 0;
		stTieArea2.m_nUsedWndPolygon = 0;
		memset(puImgDst, 0, sizeof(UCHAR) * nOrgWidth * nOrgHeight);
		if (bUseBlobOption == FALSE)
		{
			if (nTypeSelectBlob == eSelectMix || bFillHole)
			{
				m_pCPInsp_Algo->BlobImageStruct_BW(pInspAlgoBW2, sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_fArr3D, nOrgWidth, nOrgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, puImgDst,
					nTypeSelectBlob, FALSE, 0, stTieArea2, bFillHole, 0, ucColorImgDst);
			}
			else
				m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoBW2, sWndAlgoImg, puImgDst, stTieArea2, ucColorImgDst);
		}
		else if (bUseBlobOption == TRUE && nTypeSelectBlob == eSelectMix)
		{
			m_pCPInsp_Algo->BlobImageStruct_BW(pInspAlgoBW2, sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_fArr3D, nOrgWidth, nOrgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, puImgDst,
				nTypeSelectBlob, FALSE, 0, stTieArea2, bFillHole, 0, ucColorImgDst);
		}
		nLine = __LINE__;
		m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("OriginBW.bmp"));

		UCHAR *pUcImgSrc = sWndClipAlgo.m_ucArr2D;
		float *pfImgSrc = sWndClipAlgo.m_fArr3D;
		int nImgWidth = sWndClipAlgo.m_nWidth;
		int nImgHeight = sWndClipAlgo.m_nHeight;
		if ((nImgWidth <= 0) || (nImgHeight <= 0))
			return bResult;

		UCHAR * ucArrClipBWDst = NULL;
		if (puImgDst)
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrClipBWDst, nImgWidth * nImgHeight);

		if (pColorTeach != NULL && pInspAlgoBW->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			nLine = __LINE__;
			Delete_1DArray(&ucColorImgDst);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sClipImgBuf->nImageSizeX * sClipImgBuf->nImageSizeY);
			pColorTeach->GetColorBaseBin(&pInspAlgoBW->m_sAlgoColorBase, *sClipImgBuf, ucColorImgDst, true, 0, 0, -1, sWndAlgoImg.m_nLight_index);
			m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sClipImgBuf->nImageSizeX, sClipImgBuf->nImageSizeY, _T("OrgColor_Clip.bmp"));
			nLine = __LINE__;
		}
		nLine = __LINE__;

		UCHAR* pucAI = NULL;
		int nL = nClipStartX;
		int nT = nClipStartY;
		int nR = nClipStartX + nClipWidth;
		int nB = nClipStartY + nClipHeight;
		if (nL < 0) nL = 0;
		if (nR < 0) nR = 0;
		if (nT < 0) nT = 0;
		if (nB < 0) nB = 0;
		if (nL > sWndAlgoImg.m_nWidth) nL = sWndAlgoImg.m_nWidth;
		if (nR > sWndAlgoImg.m_nWidth) nR = sWndAlgoImg.m_nWidth;
		if (nT > sWndAlgoImg.m_nHeight) nT = sWndAlgoImg.m_nHeight;
		if (nB > sWndAlgoImg.m_nHeight) nB = sWndAlgoImg.m_nHeight;
		// 실제 teach 하기 위한 부분, (clip image 기준)
		double dReturnRate = 0.0;
		if (bUseBlobOption == FALSE)
		{
			nLine = __LINE__;
			if (nTypeSelectBlob == eSelectMix || bFillHole)
			{
				m_pCPInsp_Algo->BlobImageStruct_BW(pInspAlgoBW2, sWndClipAlgo.m_ucArr2D, sWndClipAlgo.m_fArr3D, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucArrClipBWDst,
					nTypeSelectBlob, FALSE, 0, stTieArea, bFillHole, 0, ucColorImgDst);
			}
			else
				dReturnRate = m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoBW2, sWndClipAlgo, ucArrClipBWDst, stTieArea, ucColorImgDst, pInspAlgoBW->m_byDir, nInspectionAreaPer, pucAI);
			sRstAlgo->m_dRstTeachArea = pInspAlgoBW2.m_dAreaCurrent;
			nLine = __LINE__;
		}
		else
		{
			nLine = __LINE__;
			dReturnRate = m_pCPInsp_Algo->BlobImageStruct_BW(pInspAlgoBW2, sWndClipAlgo.m_ucArr2D, sWndClipAlgo.m_fArr3D, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucArrClipBWDst,
				nTypeSelectBlob, FALSE, 0, stTieArea, bFillHole, 0, ucColorImgDst);
			sRstAlgo->m_dRstTeachArea = dArea;
		}
		if (pucAI)
			Delete_1DArray(&pucAI);
		m_pProcMilAlgo->SaveWorkImg(puImgDst, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("PartBlob.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ucArrClipBWDst, sWndClipAlgo.m_nWidth, sWndClipAlgo.m_nHeight, _T("WndBlob.bmp"));

		for (int y = nT; y < nB; y++)
		{
			for (int x = nL; x < nR; x++)
			{
				int nIndex = (y * sWndAlgoImg.m_nWidth) + x;

				int nX = (x - nL);
				int nY = (y - nT);
				if (nX < 0 || nX >= sWndClipAlgo.m_nWidth)
					continue;
				if (nY < 0 || nY >= sWndClipAlgo.m_nHeight)
					continue;

				int nWndIDX = nY * sWndClipAlgo.m_nWidth + nX;
				puImgDst[nIndex] = ucArrClipBWDst[nWndIDX];
			}
		}
		m_pProcMilAlgo->SaveWorkImg(puImgDst, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("PartBlob + WndBlob.bmp"));
		if ((pInspAlgoBW->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) == eDefaultAC_Data_Use && sRstAlgo)
		{
			POINTF poCenter;
			poCenter.x = nImgWidth / 2;
			poCenter.y = nImgHeight / 2;
			UCHAR * pucACDst = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucACDst, nImgWidth * nImgHeight * 3);
			memset(pucACDst, 0, nImgWidth * nImgHeight * 3 * sizeof(UCHAR));
			if (pInspAlgoBW->m_sAngleColorBase.m_byArrInspAC[m_eInspAC_T2_InspArea] > 100 && pInspAlgoBW->m_sAngleColorBase.m_byArrInspAC[m_eInspAC_T2_InspArea] > nInspectionAreaPer)
			{
				UCHAR * ucArrClipBWDstAC = NULL;
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrClipBWDstAC, nImgWidth * nImgHeight);
				memset(ucArrClipBWDstAC, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
				m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoBW2, sWndClipAlgo, ucArrClipBWDstAC, stTieArea, ucColorImgDst, pInspAlgoBW->m_byDir, pInspAlgoBW->m_sAngleColorBase.m_byArrInspAC[m_eInspAC_T2_InspArea]);
				sRstAlgo->m_sInspAC.m_nInspAC = InspAC(pInspAlgoBW->m_sAngleColorBase, *sClipImgBuf, ucArrClipBWDstAC, sWndClipAlgo, pInspAlgoBW->m_byDir, pucACDst, sRstAlgo->m_sInspAC.m_fRstInspAC, &sRstAlgo->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ], true, poCenter);
				Delete_1DArray(&ucArrClipBWDstAC);
			}
			else
				sRstAlgo->m_sInspAC.m_nInspAC = InspAC(pInspAlgoBW->m_sAngleColorBase, *sClipImgBuf, ucArrClipBWDst, sWndClipAlgo, pInspAlgoBW->m_byDir, pucACDst, sRstAlgo->m_sInspAC.m_fRstInspAC, &sRstAlgo->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ], true, poCenter);
			if (stAlgoParam.m_ucArrDstImgAC)
			{
				int nWndIDXAC = 0;
				int widthStep = nOrgWidth;
				//if ((widthStep % 4) != 0) widthStep += 4 - (widthStep % 4);
				widthStep = g_pMPTI->nCalcWidthStep(false, nOrgWidth);

				memset(stAlgoParam.m_ucArrDstImgAC, 0, widthStep * nOrgHeight * 3 * sizeof(UCHAR));
				for (int y = 0; y < nOrgHeight; y++)
				{
					for (int x = 0; x < nOrgWidth; x++)
					{
						int nIndex = (y * nOrgWidth) + x;
						int nIndex1 = (y * widthStep) + x;
						if (x >= nClipStartX && x < nClipStartX + nClipWidth &&
							y >= nClipStartY && y < nClipStartY + nClipHeight)
						{
							stAlgoParam.m_ucArrDstImgAC[(nIndex1 * 3)] = pucACDst[nWndIDXAC++];
							stAlgoParam.m_ucArrDstImgAC[(nIndex1 * 3) + 1] = pucACDst[nWndIDXAC++];
							stAlgoParam.m_ucArrDstImgAC[(nIndex1 * 3) + 2] = pucACDst[nWndIDXAC++];
						}
						else
						{
							stAlgoParam.m_ucArrDstImgAC[(nIndex1 * 3)] = puImgDst[nIndex];
							stAlgoParam.m_ucArrDstImgAC[(nIndex1 * 3) + 1] = puImgDst[nIndex];
							stAlgoParam.m_ucArrDstImgAC[(nIndex1 * 3) + 2] = puImgDst[nIndex];
						}
					}
				}
				m_pProcMilAlgo->SaveWorkImg(stAlgoParam.m_ucArrDstImgAC, widthStep, nOrgHeight, _T("AngleColor_Part_Image.bmp"), 3);
			}
			Delete_1DArray(&pucACDst);
		}
		nLine = __LINE__;
		Delete_1DArray(&ucColorImgDst);
		m_pProcMilAlgo->SaveWorkImg(ucArrClipBWDst, nImgWidth, nImgHeight, _T("ClipBW.bmp"));
#if _DEBUG
		cv::Mat imgArrClipBWDst(nImgHeight, nImgWidth, CV_8UC1, ucArrClipBWDst);
		cv::Mat ImgDst(nOrgHeight, nOrgWidth, CV_8UC1, puImgDst);
#endif
		float fWhiteHeigthAvg = 0.0f;
		if (pInspAlgoBW->m_bUseHeightMean == TRUE)
		{
			int nWhiteCnt = 0;
			double dResolutionXY = dResolX * dResolY;//GetResolX() * GetResolY();
			int nDir = pInspAlgoBW->m_byDir;
			int nW = nImgWidth;
			int nH = nImgHeight;
			int nX = 0;
			int nY = 0;
			if (nDir == 0)
			{
				int nTemp = nImgWidth * (nInspectionAreaPerH / 100.0);
				nX = nImgWidth - nTemp;
			}
			else if (nDir == 1)
				nW = nImgWidth * (nInspectionAreaPerH / 100.0);

			if (nDir == 2)
			{
				int nTemp = nImgHeight * (nInspectionAreaPerH / 100.0);
				nY = nImgHeight - nTemp;
			}
			else if (nDir == 3)
				nH = nImgHeight * (nInspectionAreaPerH / 100.0);

			int cnt = 0;
			float fsum = 0;
			for (int x = nX; x < nW; x++)
			{
				for (int y = nY; y < nH; y++)
				{
					int nIndex = (y * nImgWidth) + x;
					if (pInspAlgoBW->m_bInspBWArea == FALSE)
					{
						fsum += pfImgSrc[nIndex];
						cnt++;
					}
					else if (pInspAlgoBW->m_bInspBWArea == TRUE && ucArrClipBWDst[nIndex] == 255)
					{
						fsum += pfImgSrc[nIndex];
						cnt++;
					}
					else if ((pInspAlgoBW->m_byUseHeightOpt & m_eBWHeigthMeanIncludeBlackArea) == m_eBWHeigthMeanIncludeBlackArea)
					{
						cnt++;
					}
					if (bHM_A)
					{
						int nX2 = x + nL;
						int nY2 = y + nT;
						if (nX2 >= 0 && nX2 < nOrgWidth)
						{
							if (nY2 >= 0 && nY2 < nOrgHeight)
							{
								int nIDX = nY2 * nOrgWidth + nX2;
								if (ucArrClipBWDst[nIndex] == 255 && puImgDst[nIDX] == 255)
									puImgDst[nIDX] = 192;
								else if (ucArrClipBWDst[nIndex] == 255 && puImgDst[nIDX] == 0)
									puImgDst[nIDX] = 128;
								else if (ucArrClipBWDst[nIndex] == 0 && puImgDst[nIDX] == 255)
									puImgDst[nIDX] = 64;
							}
						}
					}
				}
			}
			if (fsum != 0 && cnt > 0)
				fWhiteHeigthAvg = fsum / (float)cnt;
		}
		nLine = __LINE__;
		if (sRstAlgo)
		{
			if (pInspAlgoBW->m_bUseTeachingRate == false)
			{
				double dStdOK = pInspAlgoBW->m_nStdOKArea;
				if (dStdOK < 0 || dStdOK > 100) dStdOK = 1;
				dReturnRate *= ((double)100 / dStdOK);
			}
			sRstAlgo->m_dRstPercent = dReturnRate;
			if (pInspAlgoBW->m_bUseTeachingRate == TRUE)
				sRstAlgo->m_dRstTeachArea *= /*m_resolX*/dResolX * dResolY/*m_resolY*/;
			if (pInspAlgoBW->m_bUseHeightMean == TRUE)
			{
				sRstAlgo->m_dRstHeightMean = fWhiteHeigthAvg;
				if (fWhiteHeigthAvg > pInspAlgoBW->m_fHeightMeanMin && fWhiteHeigthAvg < pInspAlgoBW->m_fHeightMeanMax)
					sRstAlgo->m_bOKHeightMean = TRUE;
				else
					sRstAlgo->m_bOKHeightMean = FALSE;
			}
			else
			{
				sRstAlgo->m_bOKHeightMean = TRUE;
				sRstAlgo->m_dRstHeightMean = 0;
			}
		}
		nLine = __LINE__;
		if (puImgDst)
			bResult = TRUE;

		if (ucArrClipBWDst)
			Delete_1DArray(&ucArrClipBWDst);
		nLine = __LINE__;
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Algo::TeachBlackAndWhite(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return bResult;
}

double CPInsp_AlgoBW::InspSelectAreaBW(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RECT &rcSelectArea)
{
	double dResult = 0.0;

	if (!m_pProcMilAlgo)
		return dResult;

	AlgoLeadSolder *pInspAlgo = (AlgoLeadSolder *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgo)
		return dResult;

	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return dResult;

	RECT rcDstRect;
	if (m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, rcSelectArea, &rcDstRect) == FALSE)
		return dResult;

	if (rcDstRect.top < 0)	rcDstRect.top = 0;
	if (rcDstRect.bottom < 0)	rcDstRect.bottom = 0;
	if (rcDstRect.left < 0)	rcDstRect.left = 0;
	if (rcDstRect.right < 0)	rcDstRect.right = 0;

	if (rcDstRect.top > nImgHeight)	rcDstRect.top = nImgHeight;
	if (rcDstRect.bottom > nImgHeight)	rcDstRect.bottom = nImgHeight;
	if (rcDstRect.left > nImgWidth)	rcDstRect.left = nImgWidth;
	if (rcDstRect.right > nImgWidth)	rcDstRect.right = nImgWidth;

	int nStartX = rcDstRect.left;
	int nStartY = rcDstRect.top;
	int nROIWidth = 0;
	int nROIHeight = 0;

	if (rcDstRect.right > rcDstRect.left)
		nROIWidth = rcDstRect.right - rcDstRect.left;
	else
		nROIWidth = rcDstRect.left - rcDstRect.right;

	if (rcDstRect.bottom > rcDstRect.top)
		nROIHeight = rcDstRect.bottom - rcDstRect.top;
	else
		nROIHeight = rcDstRect.top - rcDstRect.bottom;

	if ((nROIWidth <= 0) || (nROIHeight <= 0))
		return dResult;

	int nSolderScanCount = 0;
	int nSolderX = 0;
	int nSolderY = 0;
	int nSolderWidth = 0;
	int nSolderHeight = 0;

	nSolderX = nStartX;
	nSolderY = nStartY;
	nSolderWidth = nROIWidth;
	nSolderHeight = nROIHeight;
	nSolderScanCount = nSolderWidth * nSolderHeight;

	//float *pfSolderImg	= new float[nSolderScanCount];
	float *pfSolderImg = g_pMManager->pem_new<float>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);
	m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Origin3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfSolderImg, nSolderWidth, nSolderHeight, _T("ROI3D.bmp"));

	//UCHAR *pucSolderImg	= new UCHAR[nSolderScanCount];
	UCHAR *pucSolderImg = g_pMManager->pem_new<UCHAR>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);
	m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucSolderImg, nStartX, nStartY, nSolderWidth, nSolderHeight);
	m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Origin2D.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucSolderImg, nSolderWidth, nSolderHeight, _T("ROI2D.bmp"));

	// Height Mean 알고리즘
	UCHAR* ucArrDstImg = NULL;
	dResult = m_pCPInsp_Algo->GetBWImageStruct(pInspAlgo->sAlgoBW, pucSolderImg, pfSolderImg, nSolderWidth, nSolderHeight, ucArrDstImg);

	if (pfSolderImg)
		Delete_1DArray(&pfSolderImg);
	if (pucSolderImg)
		Delete_1DArray(&pucSolderImg);
	if (ucArrDstImg)
		Delete_1DArray(&ucArrDstImg);

	return dResult;

}
bool CPInsp_AlgoBW::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	if (pInspBoardInfo->bIsChip == FALSE || inspType != eINSP_SOLDER)
		return false;

	bool bT = ((g_pMPTI->m_nAIOption & m_eAIOPT_SOLDER_AutoInsp) == m_eAIOPT_SOLDER_AutoInsp);
	bool bI = ((g_pMPTI->m_nAIOption & m_eAIOPT_SOLDER_AutoInsp_AC) == m_eAIOPT_SOLDER_AutoInsp_AC);
	if (bT == false && bI == false)
		return false;

	return true;
}