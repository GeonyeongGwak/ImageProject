#include "PInsp_AlgoGrayMean.h"


CPInsp_AlgoGrayMean::CPInsp_AlgoGrayMean(void)
{
}


CPInsp_AlgoGrayMean::~CPInsp_AlgoGrayMean(void)
{
}

void CPInsp_AlgoGrayMean::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoGrayMean::GetInspAlgoData()
{
	return eSPCAlgoGray_Mean;
}

int CPInsp_AlgoGrayMean::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoGrayMean * rstAlgo = (RstAlgoGrayMean *)vRstInspAlgo;
	if (nWndType == eINSP_SOLDER)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}
	else
	{
		if (rstAlgo->m_bUseUV == TRUE)
		{
			if (rstAlgo->m_nStdGrayMin > rstAlgo->m_dRstGrayMean)		  // Low Coating
			{
				eWholeNgTypeTemp[TypeLowCoating] = e_NG;
				nCurrentNgType = TypeLowCoating;
			}
			else if (rstAlgo->m_nStdGrayMax < rstAlgo->m_dRstGrayMean) // High Coating
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
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoGrayMean::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoGrayMean::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	UCHAR * puImgDst = NULL;
	int nWidth = sWndAlgoImg.m_nWidth;
	int nHeight = sWndAlgoImg.m_nHeight;
	if (stAlgoParam.m_bInspection == FALSE)
	{
		if (ucArrDstImg)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &puImgDst, nWidth * nHeight);
			memset(puImgDst, 0, sizeof(UCHAR) * nWidth * nHeight);
		}
	}

	bResult = InspGrayMean(sInspAlgo, sWndAlgoImg, *sInspImageData, (RstAlgoGrayMean *)sRstAlgo, stTieArea, stAlgoParam.m_byWndInspType, stAlgoParam.m_nReviewMin, stAlgoParam.m_nReviewMax, puImgDst, stAlgoParam.m_bTeach);

	if (stAlgoParam.m_bInspection == FALSE)
	{
		if (puImgDst)
		{
			UCHAR * puImgDstAll = NULL;
			int nX = stAlgoParam.m_nStartX;
			int nY = stAlgoParam.m_nStartY;
			int nW = stAlgoParam.m_sPartAlgoImg->m_nWidth3D;
			int nH = stAlgoParam.m_sPartAlgoImg->m_nHeight3D;
			if (sWndAlgoImg.m_nLight_index != eMSCN_COAXIAL)
			{
				nW = stAlgoParam.m_sPartAlgoImg->m_nWidth;
				nH = stAlgoParam.m_sPartAlgoImg->m_nHeight;
			}
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &puImgDstAll, nW * nH);
			memset(puImgDstAll, 0, sizeof(UCHAR) * nW * nH);
			for (int y = nY; y < nY + nHeight; y++)
			{
				for (int x = nX; x < nX + nWidth; x++)
				{
					int nIndex2 = ((y - nY) * nWidth) + (x - nX);
					int nIndex = (y * nW) + x;
					if (nIndex < 0 || nIndex >= nW * nH)
						continue;
					if (nIndex2 < 0 || nIndex2 >= nWidth * nHeight)
						continue;
					if (stAlgoParam.m_bBin)
						puImgDstAll[nIndex] = puImgDst[nIndex2];
					else
					{
						if (puImgDst[nIndex2] == 255)
							puImgDstAll[nIndex] = sWndAlgoImg.m_ucArr2D[nIndex2];
					}
				}
			}
			g_pInspMng->m_procMil->SaveWorkImg(puImgDstAll, nW, nH, _T("GrayMean_Dst.bmp"));
			m_pCPInsp_Algo->Update_Image_TOCV_Color(nW, nH, puImgDstAll, ucArrDstImg);
		}
	}
	if (puImgDst)
		Delete_1DArray(&puImgDst);

	return bResult;
}

float CPInsp_AlgoGrayMean::GetMeanImg(UCHAR *pUcImgSrc, int nImgWidth, int nImgHeight, int nExceptCount)
{
	float fReturnRate(0);

	IppiSize roiSize;
	roiSize.width = nImgWidth;
	roiSize.height = nImgHeight;
	Ipp64f isum;
	ippiSum_8u_C1R(pUcImgSrc, nImgWidth, roiSize, &isum);

	fReturnRate = isum / ((nImgWidth * nImgHeight) - nExceptCount);

	return fReturnRate;
}
BOOL CPInsp_AlgoGrayMean::InspGrayMean(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImgBuf, RstAlgoGrayMean *sRstAlgo, TotalInspExceptArea stTieArea, byte byInspType, int *nReviewMin, int *nReviewMax, UCHAR *pucImgDst, bool bTeach)
{
	BOOL bResult = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoGrayMean));
		sRstAlgo->Init();
	}

	// 예외처리
	if (g_pMPTI->IsMachineTypeUV() == FALSE)
		byInspType = WndCoating;

	if (!m_pProcMilAlgo)
		return bResult;

	AlgoGrayMean *pAlgoGray = (AlgoGrayMean *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pAlgoGray)
		return bResult;

	UCHAR *pUcImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pUcImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;
	bool bDelete = false;
	int64 nExceptCnt = 0;
	if (pucImgDst == NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucImgDst, nImgWidth * nImgHeight);
		memset(pucImgDst, 0, nImgWidth * nImgHeight);
		bDelete = true;
	}
	memcpy(pucImgDst, pUcImgSrc, nImgWidth * nImgHeight * sizeof(UCHAR));
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
	if ((pAlgoGray->m_sBlobBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse)
	{
		CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
		UCHAR *ucColorImgDst = NULL;
		if (pColorTeach != NULL && pAlgoGray->m_sBlobBase.m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			Delete_1DArray(&ucColorImgDst);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImgBuf.nImageSizeX * sInspImgBuf.nImageSizeY);
			int nSideCameraIndex = sWndAlgoImg.m_nLight_index;
			pColorTeach->GetColorBaseBin(&pAlgoGray->m_sBlobBase.m_sAlgoColorBase, sInspImgBuf, ucColorImgDst, bTeach, 0, 0, -1, nSideCameraIndex);
			m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImgBuf.nImageSizeX, sInspImgBuf.nImageSizeY, _T("OrgColor_Clip.bmp"));
		}
		memset(pucImgDst, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		m_pCPInsp_Algo->BlobImageStruct_Base(pAlgoGray->m_sBlobBase, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, stTieArea, pucImgDst);
		m_pProcMilAlgo->SaveWorkImg(pucImgDst, nImgWidth, nImgHeight, _T("Bin_GrayMean.bmp"));
		for (int a = 0; a < nImgWidth * nImgHeight; a++)
		{
			if (pucImgDst[a] != 0) continue;
			pUcImgSrc[a] = 0;
			nExceptCnt++;
		}
		if (ucColorImgDst)
			Delete_1DArray(&ucColorImgDst);
	}
	else
	{
		UCHAR *pUcOverlapImg = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pUcOverlapImg, nImgWidth * nImgHeight);
		memset(pUcOverlapImg, 0, nImgWidth * nImgHeight);
		nExceptCnt = m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pUcImgSrc, pUcOverlapImg, stTieArea);
		if (pUcOverlapImg != NULL)
			//delete[] pUcOverlapImg;
			g_pMManager->pem_delete(pUcOverlapImg, true);
		pUcOverlapImg = NULL;
	}
	m_pProcMilAlgo->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Dst_Graymean.bmp"));
	double mean = 0;
	if (nImgWidth * nImgHeight > nExceptCnt)
		mean = GetMeanImg(pUcImgSrc, nImgWidth, nImgHeight, nExceptCnt);
	bResult = TRUE;
	if (sRstAlgo)
	{
		sRstAlgo->m_nStdGrayMin = pAlgoGray->m_nGrayRateMin;
		sRstAlgo->m_nStdGrayMax = pAlgoGray->m_nGrayRateMax;
		sRstAlgo->m_dRstGrayMean = mean;
		sRstAlgo->m_bUseUV = g_pMPTI->IsMachineTypeUV();
		if ((byInspType == WndNonCoating ? false : pAlgoGray->m_nGrayRateMin > mean) || (byInspType == WndHighCoating ? false : pAlgoGray->m_nGrayRateMax < mean))
			bResult = FALSE;
	}

// 	if (g_pMPTI->IsMachineTypeUV())
// 	{
// 		AlgoDebuggingMode vAlgoDebuggingMode = g_pInspMng->GetAlgoDebuggingMode(g_pMPTI->GetLaneNum());
// 
// 		if (vAlgoDebuggingMode.bAlgoDebuggingReady && vAlgoDebuggingMode.nAlgoDebuggingRemainCnt > 0 || vAlgoDebuggingMode.bReviewMode)
// 		{
// 			float fMax = 0;
// 			float fMin = 0;
// 			int nCurrMax_Coat = 0;
// 			int nCurrMin_Coat = 0;
// 			int nCurrMax_Non = 0;
// 			int nCurrMin_Non = 0;
// 			int nCurrMax_High = 0;
// 			int nCurrMin_High = 0;
// 
// 			int nCurrMax_Compare = 0;
// 			int nCurrMin_Compare = 0;
// 
// 			DefaultGrayMeanStd vDefaultGrayMeanStd = g_pInspMng->GetDefaultGrayMeanStd();
// 
// 			if (vAlgoDebuggingMode.bType == 0) // Value
// 			{
// 				nCurrMax_Coat = sRstAlgo->m_dRstGrayMean + vDefaultGrayMeanStd.nStdGrayValueMax_Coat;
// 				nCurrMin_Coat = sRstAlgo->m_dRstGrayMean - vDefaultGrayMeanStd.nStdGrayValueMin_Coat;
// 
// 				nCurrMax_Non = sRstAlgo->m_dRstGrayMean + vDefaultGrayMeanStd.nStdGrayValueMax_Non;
// 				nCurrMin_Non = sRstAlgo->m_dRstGrayMean - vDefaultGrayMeanStd.nStdGrayValueMin_Non;
// 
// 				nCurrMax_High = sRstAlgo->m_dRstGrayMean + vDefaultGrayMeanStd.nStdGrayValueMax_High;
// 				nCurrMin_High = sRstAlgo->m_dRstGrayMean - vDefaultGrayMeanStd.nStdGrayValueMin_High;
// 			}
// 			else if (vAlgoDebuggingMode.bType == 1) // Percentage
// 			{
// 				fMax = vDefaultGrayMeanStd.nStdGrayMax_Coat != 0 ? vDefaultGrayMeanStd.nStdGrayMax_Coat : 120;
// 				fMin = vDefaultGrayMeanStd.nStdGrayMin_Coat != 0 ? vDefaultGrayMeanStd.nStdGrayMin_Coat : 80;
// 				nCurrMax_Coat = (int)(mean * (RounD(fMax) / 100.0));
// 				nCurrMin_Coat = (int)(mean * (RounD(fMin) / 100.0));
// 
// 				fMax = vDefaultGrayMeanStd.nStdGrayMax_Non != 0 ? vDefaultGrayMeanStd.nStdGrayMax_Non : 120;
// 				fMin = vDefaultGrayMeanStd.nStdGrayMin_Non != 0 ? vDefaultGrayMeanStd.nStdGrayMin_Non : 80;
// 				nCurrMax_Non = (int)(mean * (RounD(fMax) / 100.0));
// 				nCurrMin_Non = (int)(mean * (RounD(fMin) / 100.0));
// 
// 				fMax = vDefaultGrayMeanStd.nStdGrayMax_High != 0 ? vDefaultGrayMeanStd.nStdGrayMax_High : 120;
// 				fMin = vDefaultGrayMeanStd.nStdGrayMin_High != 0 ? vDefaultGrayMeanStd.nStdGrayMin_High : 80;
// 				nCurrMax_High = (int)(mean * (RounD(fMax) / 100.0));
// 				nCurrMin_High = (int)(mean * (RounD(fMin) / 100.0));
// 			}
// 
// 			// 최대값 최소값 에러 문제
// 			if (nCurrMin_Coat < 0)
// 				nCurrMin_Coat = 0;
// 			if (nCurrMax_Coat > 255)
// 				nCurrMax_Coat = 255;
// 			if (nCurrMin_Non < 0)
// 				nCurrMin_Non = 0;
// 			if (nCurrMax_Non > 255)
// 				nCurrMax_Non = 255;
// 			if (nCurrMin_High < 0)
// 				nCurrMin_High = 0;
// 			if (nCurrMax_High > 255)
// 				nCurrMax_High = 255;
// 
// 			// Coating/non coating/high coating 각각의 조건값을 분리
// 			nCurrMin_Compare = nCurrMin_Coat;
// 			nCurrMax_Compare = nCurrMax_Coat;
// 
// 			if (byInspType == WndNonCoating && vDefaultGrayMeanStd.bUseStdGray_Non)
// 			{
// 				nCurrMin_Compare = nCurrMin_Non;
// 				nCurrMax_Compare = nCurrMax_Non;
// 			}
// 			else if (byInspType == WndHighCoating && vDefaultGrayMeanStd.bUseStdGray_High)
// 			{
// 				nCurrMin_Compare = nCurrMin_High;
// 				nCurrMax_Compare = nCurrMax_High;
// 			}
// 
// 
// 
// 			// ReviewMode인경우에는 선택된것만 적용함 아닌경우 패스.
// 			if (vAlgoDebuggingMode.bReviewMode)
// 			{
// 				*nReviewMin = nCurrMin_Compare;
// 				*nReviewMax = nCurrMax_Compare;
// 			}
// 
// 			bool bIsIncludeNG = vAlgoDebuggingMode.bIncludeAlgoNG == true ? true : bResult;	// NG도 포함하는 모드이면 무조건 하고 아니면 OK 일때만 한다.
// 
// 			// is Result good?
// 			if (bIsIncludeNG)
// 			{
// 				bool bChanged = false;
// 				if (nCurrMax_Compare > pAlgoGray->m_nGrayRateMax)
// 				{
// 					sRstAlgo->m_nRstStdGrayMax = nCurrMax_Compare;
// 					bChanged = true;
// 				}
// 				else
// 					sRstAlgo->m_nRstStdGrayMax = -1;
// 
// 				if (nCurrMin_Compare < pAlgoGray->m_nGrayRateMin)
// 				{
// 					sRstAlgo->m_nRstStdGrayMin = nCurrMin_Compare;
// 					bChanged = true;
// 				}
// 				else
// 					sRstAlgo->m_nRstStdGrayMin = -1;
// 
// 				sRstAlgo->m_bRstStdChanged = bChanged;
// 			}
// 			else
// 				sRstAlgo->m_bRstStdChanged = false;
// 		}
// 		else
// 		{
// 			sRstAlgo->m_bRstStdChanged = false;
// 		}
// 	}

	m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, nImgWidth, nImgHeight, _T("Origin2D.bmp"));
	if (bDelete)
		Delete_1DArray(&pucImgDst);
	return bResult;
}

BOOL CPInsp_AlgoGrayMean::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoGrayMean::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoGrayMean * pInspAlgo = (AlgoGrayMean *)sInspAlgo.m_ptrInspAlgoParam;
	if ((pInspAlgo->m_sBlobBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse)
	{
		nData = pInspAlgo->m_sBlobBase.m_sAlgoColorBase.GetColorData();
		nData |= COLOR_DATA_WND;
	}

	return nData;
}

double CPInsp_AlgoGrayMean::InspSelectAreaGrayMean(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RECT &rcSelectArea, int &nLeadDir)
{
	double dResult = 0.0;

	if (!m_pProcMilAlgo)
		return dResult;

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return dResult;

	m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("GrayMeanOrgImage.bmp"));
	int nROIWidth = 0;
	int nROIHeight = 0;

	RECT rcDstArea;
	if (m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, rcSelectArea, &rcDstArea) == FALSE)
		return dResult;

	int nStartX = rcDstArea.left;
	int nStartY = rcDstArea.top;

	if (rcDstArea.right > rcDstArea.left)
		nROIWidth = rcDstArea.right - rcDstArea.left;
	else
		nROIWidth = rcDstArea.left - rcDstArea.right;

	if (rcDstArea.bottom > rcDstArea.top)
		nROIHeight = rcDstArea.bottom - rcDstArea.top;
	else
		nROIHeight = rcDstArea.top - rcDstArea.bottom;

	if ((nROIWidth <= 0) || (nROIHeight <= 0))
		return dResult;

	int nSelectScanCount = 0;
	nSelectScanCount = nROIWidth * nROIHeight;

	int nSelectX = nStartX;
	int nSelectY = nStartY;

	//UCHAR *pucSelectImg = new UCHAR[nSelectScanCount];
	UCHAR *pucSelectImg = g_pMManager->pem_new<UCHAR>(true, nSelectScanCount, (PCHAR)__FUNCTION__, __LINE__);

	m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
		pucSelectImg, nSelectX, nSelectY, nROIWidth, nROIHeight);
	m_pProcMilAlgo->SaveWorkImg(pucSelectImg, nROIWidth, nROIHeight, _T("ClipGrayMeanImage.bmp"));

	// Height Mean 알고리즘
	IppiSize roiSize;
	roiSize.width = nROIWidth;
	roiSize.height = nROIHeight;
	Ipp64f mean;
	ippiMean_8u_C1R(pucSelectImg, nROIWidth, roiSize, &mean);
	dResult = mean;

	if (pucSelectImg)
		Delete_1DArray(&pucSelectImg);

	return dResult;

}
bool CPInsp_AlgoGrayMean::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}