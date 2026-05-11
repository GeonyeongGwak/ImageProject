#include "PInsp_AlgoHeightMean.h"


CPInsp_AlgoHeightMean::CPInsp_AlgoHeightMean(void)
{
}


CPInsp_AlgoHeightMean::~CPInsp_AlgoHeightMean(void)
{
}

void CPInsp_AlgoHeightMean::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoHeightMean::GetInspAlgoData()
{
	return eSPCAlgoHeight_Mean;
}

int CPInsp_AlgoHeightMean::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoHeightMean * rstAlgo = (RstAlgoHeightMean *)vRstInspAlgo;
	if (nWndType == eINSP_SOLDER)
	{
		if (!rstAlgo->m_bExcess && !rstAlgo->m_bNoSolder)
		{
			eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
			nCurrentNgType = TypeSolderFilet;
		}
		else if (rstAlgo->m_bExcess || !rstAlgo->m_bNoSolder)
		{
			eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
			nCurrentNgType = TypeSolderExcess;
		}
		else if (!rstAlgo->m_bExcess || rstAlgo->m_bNoSolder)
		{
			eWholeNgTypeTemp[TypeNoSolder] = e_NG;
			nCurrentNgType = TypeNoSolder;
		}
	}
	else
	{
		eWholeNgTypeTemp[TypeMountWorng] = e_NG;
		nCurrentNgType = TypeMountWorng;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoHeightMean::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoHeightMean::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspHeightMean(sInspAlgo, *stAlgoParam.m_sPartAlgoImg, sWndAlgoImg, *sInspImageData, stTieArea, (RstAlgoHeightMean *)sRstAlgo, ucArrDstImg, stAlgoParam.m_nStartX, stAlgoParam.m_nStartY, !stAlgoParam.m_bInspection);

	return bResult;
}

BOOL CPInsp_AlgoHeightMean::InspHeightMean(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, InspRoiImgBuf &sInspImgBuf, TotalInspExceptArea stTieArea, RstAlgoHeightMean *sRstAlgo, UCHAR* puImgDst, int nStartX, int nStartY, bool bTeach)
{
	BOOL bReturn = FALSE;
	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoHeightMean));
		sRstAlgo->Init();
	}
	if (!m_pProcMilAlgo || sInspAlgo.m_eAlgoType != eAlgoHeight_Mean)
		return bReturn;
	AlgoHeightMean *pInspAlgoHeightMean = (AlgoHeightMean *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoHeightMean)
		return bReturn;
	UCHAR *pucImgSrc = sWndClipAlgo.m_ucArr2D;
	float *pfImgOrg = sWndClipAlgo.m_fArr3D;
	int nImgWidth = sWndClipAlgo.m_nWidth;
	int nImgHeight = sWndClipAlgo.m_nHeight;
	int nOrgWidth = sWndAlgoImg.m_nWidth;
	int nOrgHeight = sWndAlgoImg.m_nHeight;
	if ((pfImgOrg == NULL) || (pucImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0) || (nOrgWidth <= 0) || (nOrgHeight <= 0))
		return bReturn;

	bool bHighestBW = ((pInspAlgoHeightMean->m_sBlobHighestBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse);
	bool bLowestBW  = ((pInspAlgoHeightMean->m_sBlobLowestBase .m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse);
	if (puImgDst)
	{
		if (pInspAlgoHeightMean->m_bBWOption == TRUE || bHighestBW || bLowestBW)
			memset(puImgDst, 0, nOrgWidth * nOrgHeight * sizeof(UCHAR));
		else
		{
			IppiSize iSize = { nOrgWidth, nOrgHeight };
			IppStatus sts = ippiCopy_8u_C1R(sWndAlgoImg.m_ucArr2D, nOrgWidth, puImgDst, nOrgWidth, iSize);
			m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("Origin2D.bmp"));
		}
	}
	float * pfImgSrc = NULL;
	UCHAR *ucArrOverlapImg = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pfImgSrc, nImgWidth * nImgHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrOverlapImg, nImgWidth * nImgHeight);
	memcpy(pfImgSrc, pfImgOrg, nImgWidth * nImgHeight * sizeof(float));
	memset(ucArrOverlapImg, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("ORG_3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("ORG_2D.bmp"));
	int64 nExceptCnt = 0;

	stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
	stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
	stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
	//for (int i = 0; i < MAX_MASKING_NUM; i++)
	//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
	//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
	//	stTieArea.m_rcArrMaskingROI.push_back(sInspAlgo.m_rcArrMaskingROI[i]);
	//stTieArea.m_rcArrMaskingROI.resize(sInspAlgo.m_nUsedMaskingValue);
	//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i ++)
	//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
	for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];
	UCHAR *pUcOverlapImg = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pUcOverlapImg, nImgWidth * nImgHeight);
	memset(pUcOverlapImg, 0, nImgWidth * nImgHeight);
	/*nExceptCnt = m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, (UCHAR*)pfImgSrc, pUcOverlapImg, stTieArea);*/

	nExceptCnt += m_pCPInsp_Algo->FillOutOfInspArea(nImgWidth, nImgHeight, 0, pfImgSrc, ucArrOverlapImg, stTieArea.m_nUsedWndPolygon, stTieArea.m_ptArrWndPolygon);
	nExceptCnt += m_pCPInsp_Algo->FillOutOfInspArea(nImgWidth, nImgHeight, 0, pfImgSrc, ucArrOverlapImg, sInspAlgo.m_nUsedInspPolygon, sInspAlgo.m_ptArrInspPolygon, 0, sInspAlgo.m_bConvetExceptROI);
	nExceptCnt += m_pCPInsp_Algo->FillMaskingROI(nImgWidth, nImgHeight, 0, pfImgSrc, sInspAlgo.m_nUsedMaskingValue, sInspAlgo.m_rcArrMaskingROI, 0, ucArrOverlapImg);

	Delete_1DArray(&pUcOverlapImg);
	Delete_1DArray(&ucArrOverlapImg);
	float fMean = 0.0f;
	float fHeightMinValue = pInspAlgoHeightMean->m_bUseLowest == TRUE ? FLT_MAX : 0;
	float fHeightMaxValue = 0.0f;
	float fAVG = 0.0f;
	if (pInspAlgoHeightMean->m_bHeightUse == TRUE)
	{
		if (pInspAlgoHeightMean->m_bBWOption == TRUE)
		{
			UCHAR * ucArrClipBWDst = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrClipBWDst, nImgWidth * nImgHeight);
			m_pCPInsp_Algo->GetBWImageStruct(*pInspAlgoHeightMean, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, ucArrClipBWDst);

			// KIY 2020/04/23 : 폴리곤 통합
			m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, ucArrClipBWDst, NULL, stTieArea);

			int nCnt = 0;
			float fSum = 0;
			for (int y = 0; y < nImgHeight; y++)
			{
				for (int x = 0; x < nImgWidth; x++)
				{
					int nIndex = (y * nImgWidth) + x;
					int nIndex2 = ((y + nStartY) * nOrgWidth) + (x + nStartX);
					if (ucArrClipBWDst[nIndex] == 0) continue;
					if (bHighestBW == false && puImgDst && nIndex2 < nOrgWidth * nOrgHeight)
						puImgDst[nIndex2] = 255;
					fSum += pfImgSrc[nIndex];
					nCnt++;
				}
			}
			if (nCnt > 0) fMean = fSum / nCnt;
			Delete_1DArray(&ucArrClipBWDst);
		}
		else
			fMean = m_pCPInsp_Algo->GetHeightMean(pfImgSrc, nImgWidth, nImgHeight, nExceptCnt);
	}
	if (pInspAlgoHeightMean->m_bUseHighest == TRUE)
	{
		float fHeightMinValueTmp = 0.0f;
		if (bHighestBW)
		{
			CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
			UCHAR *ucColorImgDst = NULL;
			if (pColorTeach != NULL && pInspAlgoHeightMean->m_sBlobHighestBase.m_sAlgoColorBase.m_bUseColor == TRUE)
			{
				Delete_1DArray(&ucColorImgDst);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImgBuf.nImageSizeX * sInspImgBuf.nImageSizeY);
				int nSideCameraIndex = sWndAlgoImg.m_nLight_index;
				pColorTeach->GetColorBaseBin(&pInspAlgoHeightMean->m_sBlobHighestBase.m_sAlgoColorBase, sInspImgBuf, ucColorImgDst, bTeach, 0, 0, -1, nSideCameraIndex);
				m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImgBuf.nImageSizeX, sInspImgBuf.nImageSizeY, _T("OrgColor_Clip.bmp"));
			}
			UCHAR *pucImgBin = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucImgBin, nImgWidth * nImgHeight);
			m_pCPInsp_Algo->BlobImageStruct_Base(pInspAlgoHeightMean->m_sBlobHighestBase, pucImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, stTieArea, pucImgBin);
			m_pProcMilAlgo->SaveWorkImg(pucImgBin, nImgWidth, nImgHeight, _T("Highest_Bin1.bmp"));
			m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pucImgBin, NULL, stTieArea);
			m_pProcMilAlgo->SaveWorkImg(pucImgBin, nImgWidth, nImgHeight, _T("Highest_Bin2.bmp"));
			for (int y = 0; y < nImgHeight; y++)
			{
				for (int x = 0; x < nImgWidth; x++)
				{
					int nIndex = (y * nImgWidth) + x;
					int nIndex2 = ((y + nStartY) * nOrgWidth) + (x + nStartX);
					if (pucImgBin[nIndex] == 0) continue;
					if (puImgDst && nIndex2 < nOrgWidth * nOrgHeight)
						puImgDst[nIndex2] = 255;
					if (pfImgSrc[nIndex] > fHeightMaxValue)
					{
						fHeightMaxValue = pfImgSrc[nIndex];
						fAVG = XY_Check(pfImgSrc, nImgWidth, nImgHeight, x, y, pInspAlgoHeightMean->m_nHighestX, pInspAlgoHeightMean->m_nHighestY);
					}
				}
			}
			if (puImgDst)
				m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("Highest_ORG_Bin.bmp"));
			Delete_1DArray(&pucImgBin);
			Delete_1DArray(&ucColorImgDst);

		}
		else
		{
			m_pCPInsp_Algo->GetHeightMinMax(pfImgSrc, nImgWidth, nImgHeight, &fHeightMinValueTmp, &fHeightMaxValue);
			fAVG = XY_Check(pfImgSrc, nImgWidth, nImgHeight, fHeightMaxValue, pInspAlgoHeightMean->m_nHighestX, pInspAlgoHeightMean->m_nHighestY);
		}

		if (pInspAlgoHeightMean->m_nHighestX > 0 || pInspAlgoHeightMean->m_nHighestY > 0)
			fHeightMaxValue = fAVG;
	}
	if (pInspAlgoHeightMean->m_bUseLowest == TRUE)
	{
		float fHeightMaxValueTmp = 0.0f;
		if (bLowestBW == TRUE)
		{
			CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
			UCHAR *ucColorImgDst = NULL;
			if (pColorTeach != NULL && pInspAlgoHeightMean->m_sBlobLowestBase.m_sAlgoColorBase.m_bUseColor == TRUE)
			{
				Delete_1DArray(&ucColorImgDst);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImgBuf.nImageSizeX * sInspImgBuf.nImageSizeY);
				int nSideCameraIndex = sWndAlgoImg.m_nLight_index;
				pColorTeach->GetColorBaseBin(&pInspAlgoHeightMean->m_sBlobLowestBase.m_sAlgoColorBase, sInspImgBuf, ucColorImgDst, bTeach, 0, 0, -1, nSideCameraIndex);
				m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImgBuf.nImageSizeX, sInspImgBuf.nImageSizeY, _T("OrgColor_Clip2.bmp"));
			}
			UCHAR *pucImgBin = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucImgBin, nImgWidth * nImgHeight);
			m_pCPInsp_Algo->BlobImageStruct_Base(pInspAlgoHeightMean->m_sBlobLowestBase, pucImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, stTieArea, pucImgBin);
			m_pProcMilAlgo->SaveWorkImg(pucImgBin, nImgWidth, nImgHeight, _T("Lowest_Bin1.bmp"));
			m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pucImgBin, NULL, stTieArea);
			m_pProcMilAlgo->SaveWorkImg(pucImgBin, nImgWidth, nImgHeight, _T("Lowest_Bin2.bmp"));

			for (int y = 0; y < nImgHeight; y++)
			{
				for (int x = 0; x < nImgWidth; x++)
				{
					int nIndex = (y * nImgWidth) + x;
					int nIndex2 = ((y + nStartY) * nOrgWidth) + (x + nStartX);

					if (pucImgBin[nIndex] == 0) continue;
					if (puImgDst && nIndex2 < nOrgWidth * nOrgHeight && pInspAlgoHeightMean->m_bBWOption == FALSE && bHighestBW == FALSE)
						puImgDst[nIndex2] = 255;
					if (pfImgSrc[nIndex] < fHeightMinValue)
						fHeightMinValue = pfImgSrc[nIndex];
				}
			}
			if (fHeightMinValue == FLT_MAX)
				fHeightMinValue = 0;
			if (puImgDst)
				m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("Lowest_ORG_Bin.bmp"));
			Delete_1DArray(&pucImgBin);

		}
		else
			m_pCPInsp_Algo->GetHeightMinMax(pfImgSrc, nImgWidth, nImgHeight, &fHeightMinValue, &fHeightMaxValueTmp);
	}
	Delete_1DArray(&pfImgSrc);
	bReturn = TRUE;

	if (sRstAlgo)
	{
		sRstAlgo->m_dRstHeightMean = fMean + pInspAlgoHeightMean->m_fAddHeight;
		sRstAlgo->m_dRstHeighestValue = fHeightMaxValue + pInspAlgoHeightMean->m_fAddHighest;
		sRstAlgo->m_dRstLowestValue = fHeightMinValue + pInspAlgoHeightMean->m_fAddLowest;
		sRstAlgo->m_bOKHeightMean = TRUE;
		sRstAlgo->m_bOKHeighest = TRUE;
		sRstAlgo->m_bOKLowest = TRUE;
		if (pInspAlgoHeightMean->m_bUseHighest == TRUE)
		{
			if (pInspAlgoHeightMean->m_dHighestMin > sRstAlgo->m_dRstHeighestValue || pInspAlgoHeightMean->m_dHighestMax < sRstAlgo->m_dRstHeighestValue)
			{
				bReturn = FALSE;
				sRstAlgo->m_bOKHeighest = FALSE;
			}
		}
		if (pInspAlgoHeightMean->m_bHeightUse == TRUE)
		{
			if (pInspAlgoHeightMean->m_bUseHeightMin3D == TRUE)
			{
				if (pInspAlgoHeightMean->dHeightMin3D > sRstAlgo->m_dRstHeightMean)
				{
					bReturn = FALSE;
					if (pInspAlgoHeightMean->dHeightMin3D * 0.05 > sRstAlgo->m_dRstHeightMean)
						sRstAlgo->m_bNoSolder = TRUE;
					sRstAlgo->m_bExcess = FALSE;
					sRstAlgo->m_bOKHeightMean = FALSE;
				}
			}
			if (pInspAlgoHeightMean->m_bUseHeightMax3D == TRUE)
			{
				if (pInspAlgoHeightMean->dHeightMax3D < sRstAlgo->m_dRstHeightMean)
				{
					bReturn = FALSE;
					sRstAlgo->m_bNoSolder = FALSE;
					sRstAlgo->m_bExcess = TRUE;
					sRstAlgo->m_bOKHeightMean = FALSE;
				}
			}
		}
		if (pInspAlgoHeightMean->m_bUseLowest == TRUE)
		{
			if (pInspAlgoHeightMean->m_dLowestMin > sRstAlgo->m_dRstLowestValue || pInspAlgoHeightMean->m_dLowestMax < sRstAlgo->m_dRstLowestValue)
			{
				bReturn = FALSE;
				sRstAlgo->m_bOKLowest = FALSE;
			}
		}
	}
	return bReturn;
}
BOOL CPInsp_AlgoHeightMean::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoHeightMean::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoHeightMean * pInspAlgo = (AlgoHeightMean *)sInspAlgo.m_ptrInspAlgoParam;
	if ((pInspAlgo->m_sBlobHighestBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse)
	{
		nData = pInspAlgo->m_sBlobHighestBase.m_sAlgoColorBase.GetColorData();
		nData |= COLOR_DATA_WND;
	}
	if ((pInspAlgo->m_sBlobLowestBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse)
	{
		nData = pInspAlgo->m_sBlobLowestBase.m_sAlgoColorBase.GetColorData();
		nData |= COLOR_DATA_WND;
	}

	return nData;
}
double CPInsp_AlgoHeightMean::InspSelectAreaHeightMean(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RECT &rcSelectArea, int &nLeadDir)
{
	double dResult = 0.0;

	if (!m_pProcMilAlgo)
		return dResult;

	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
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

	int nSolderX = nStartX;
	int nSolderY = nStartY;
	int nSolderWidth = nROIWidth;
	int nSolderHeight = nROIHeight;
	int nSolderScanCount = nSolderWidth * nSolderHeight;

	float *pfSolderImg = g_pMManager->pem_new<float>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);

	m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("OriginMean3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfSolderImg, nSolderWidth, nSolderHeight, _T("HeightMean3D.bmp"));

	// Height Mean 알고리즘
	dResult = m_pCPInsp_Algo->GetHeightMean(pfSolderImg, nSolderWidth, nSolderHeight);

	if (pfSolderImg)
		Delete_1DArray(&pfSolderImg);

	return dResult;

}

BOOL CPInsp_AlgoHeightMean::InspHeightMnMx(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, RECT &rcSelectArea, int nMnMx, bool bHighest)
{
	double dResult = 0.0;

	if (!m_pProcMilAlgo)
		return dResult;

	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return dResult;

	RECT rcDstRect = rcSelectArea;
	if (bHighest == false && m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, rcSelectArea, &rcDstRect) == FALSE)
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

	{
		nSolderX = nStartX;
		nSolderY = nStartY;
		nSolderWidth = nROIWidth;
		nSolderHeight = nROIHeight;
	}
	nSolderScanCount = nSolderWidth * nSolderHeight;

	//float *pfSolderImg	= new float[nSolderScanCount];
	float *pfSolderImg = g_pMManager->pem_new<float>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);

	m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("OriginMean3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfSolderImg, nSolderWidth, nSolderHeight, _T("HeightMean3D.bmp"));

	// Height Mean 알고리즘
	dResult = m_pCPInsp_Algo->GetMinMaxImg(pfSolderImg, nSolderWidth, nSolderHeight, nMnMx);

	if (pfSolderImg)
		Delete_1DArray(&pfSolderImg);

	return dResult;
}
bool CPInsp_AlgoHeightMean::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}
float CPInsp_AlgoHeightMean::XY_Check(float* pfImgSrc, int nImgWidth, int nImgHeight, int x, int y, int nHighestX, int nHighestY)
{
	if (nHighestX <= 0 && nHighestY <= 0)
		return 0.0f;

	float fSum = 0.0f;
	float fCNT = 0;
	int nXCnt = (nHighestX * 2) + 1;
	int nYCnt = (nHighestY * 2) + 1;
	for (int nY = 0; nY < nYCnt; nY++)
	{
		int nY_T = y;
		if (nY < nHighestY)
			nY_T = y - (nHighestY - nY);
		else if (nY == nHighestY)
			nY_T = y;
		else
			nY_T = y + (nY - nHighestY);
		if (nY_T < 0 || nY_T >= nImgHeight) continue;

		for (int nX = 0; nX < nXCnt; nX++)
		{
			int nX_T = x;
			if (nX < nHighestX)
				nX_T = x - (nHighestX - nX);
			else if (nX == nHighestX)
				nX_T = x;
			else
				nX_T = x + (nX - nHighestX);
			if (nX_T < 0 || nX_T >= nImgWidth) continue;

			int nIDX = (nY_T * nImgWidth) + nX_T;
			fSum += pfImgSrc[nIDX];
			fCNT += 1.0f;
		}
	}

	float fAVG = 0.0f;
	if (fCNT > 0) fAVG = fSum / fCNT;
	return fAVG;
}
float CPInsp_AlgoHeightMean::XY_Check(float* pfImgSrc, int nImgWidth, int nImgHeight, float fH, int nHighestX, int nHighestY)
{
	if (nHighestX <= 0 && nHighestY <= 0)
		return 0.0f;
	for (int y = 0; y < nImgHeight; y++)
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			int nIndex = (y * nImgWidth) + x;
			if (pfImgSrc[nIndex] == fH)
				return XY_Check(pfImgSrc, nImgWidth, nImgHeight, x, y, nHighestX, nHighestY);
		}
	}
	return 0.0f;
}