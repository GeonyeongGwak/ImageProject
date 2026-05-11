#include "PInsp_AlgoSolderCone.h"
#define SQR(x) ((x)*(x))

CPInsp_AlgoSolderCone::CPInsp_AlgoSolderCone(void)
{
}


CPInsp_AlgoSolderCone::~CPInsp_AlgoSolderCone(void)
{
}

void CPInsp_AlgoSolderCone::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoSolderCone::GetInspAlgoData()
{
	return eSPCAlgoSolderCone;
}

int CPInsp_AlgoSolderCone::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeSolderCone] = e_NG;
	nCurrentNgType = TypeSolderCone;
	return nCurrentNgType;
}

bool CPInsp_AlgoSolderCone::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoSolderCone::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	if (stAlgoParam.m_bInspection)
		bResult = InspSolderCone(sInspAlgo, sWndAlgoImg, (RstAlgoSolderCone *)sRstAlgo, stTieArea);
	else
	{
		if ((stAlgoParam.m_nSelectArea < eLevel1) /*|| (eLevelCnt <= stAlgoParam.m_nSelectArea)*/)
			return bResult;

		if ((eVolDifLevelCnt <= stAlgoParam.m_nSelectArea))
			return bResult;

		if (eLevelCnt <= stAlgoParam.m_nSelectArea)	//Case : volumDiff 
			bResult = TeachSolderCone(sInspAlgo, *stAlgoParam.m_sPartAlgoImg, sWndAlgoImg, stAlgoParam.m_nSelectArea, (RstAlgoSolderCone *)sRstAlgo, ucArrDstImg, stAlgoParam.m_nStartX, stAlgoParam.m_nStartY, stTieArea);
		else
			bResult = TeachSolderCone(sInspAlgo, *stAlgoParam.m_sPartAlgoImg, sWndAlgoImg, stAlgoParam.m_nSelectArea, (RstAlgoSolderCone *)sRstAlgo, ucArrDstImg, stAlgoParam.m_nStartX, stAlgoParam.m_nStartY, stTieArea);
	}

	if (stAlgoParam.m_bInspection)
	{
		//Set Insp ROI 
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;

		RstAlgoSolderCone * rstSolderVolume = (RstAlgoSolderCone *)sRstAlgo;
		rstSolderVolume->m_rcRect_I.left += stAlgoParam.m_dx;
		rstSolderVolume->m_rcRect_I.right += stAlgoParam.m_dx + nImgWidth;
		rstSolderVolume->m_rcRect_I.top += stAlgoParam.m_dy;
		rstSolderVolume->m_rcRect_I.bottom += stAlgoParam.m_dy + nImgHeight;
		rstSolderVolume->m_rcRect_I_NGArea.left		= rstSolderVolume->m_rcRect_I.left  + rstSolderVolume->m_rcRect_I_NGArea.left;		//여기에 실제 NG 영역 반영
		rstSolderVolume->m_rcRect_I_NGArea.right	= rstSolderVolume->m_rcRect_I.left + rstSolderVolume->m_rcRect_I_NGArea.right;
		rstSolderVolume->m_rcRect_I_NGArea.top		= rstSolderVolume->m_rcRect_I.top   + rstSolderVolume->m_rcRect_I_NGArea.top;
		rstSolderVolume->m_rcRect_I_NGArea.bottom	= rstSolderVolume->m_rcRect_I.top + rstSolderVolume->m_rcRect_I_NGArea.bottom;

		RECT mWindowImageRect = stAlgoParam.m_ptrAlgoColorOpt->m_rcImageRect;
		int wndWidth = mWindowImageRect.right - mWindowImageRect.left;
		int wndHeight = mWindowImageRect.bottom - mWindowImageRect.top;

		int nImageStartXInWindowLT = mWindowImageRect.left;
		int nImageStartYInWindowLT = mWindowImageRect.top;

		if (rstSolderVolume->m_sPoint.x != 0 || rstSolderVolume->m_sPoint.y != 0)
		{
			rstSolderVolume->m_sPoint.x = rstSolderVolume->m_sPoint.x + nImageStartXInWindowLT;
			rstSolderVolume->m_sPoint.y = rstSolderVolume->m_sPoint.y + nImageStartYInWindowLT;
		}

	}

	return bResult;
}

BOOL CPInsp_AlgoSolderCone::InspSolderCone(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoSolderCone *sRstAlgo, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, (sizeof(RstAlgoSolderCone)));
		sRstAlgo->Init();
	}

	if (sInspAlgo.m_eAlgoType != eAlgoSolderCone)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	AlgoSolderCone *pInspAlgoSolderCone = (AlgoSolderCone *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoSolderCone)
		return bResult;

	//UCHAR *pucImgSrc	= sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nWidth = sWndAlgoImg.m_nWidth;
	int nHeight = sWndAlgoImg.m_nHeight;
	if ((pfImgSrc == NULL) || (nWidth <= 0) || (nHeight <= 0))
		return bResult;

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

	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Image3D.bmp"));
	UCHAR* pUcArrTemp = NULL;//new UCHAR[nWidth * nHeight];

	if (!pInspAlgoSolderCone->m_bUseInspVolumeDiff && !pInspAlgoSolderCone->m_bUseInspVolumeMinLength && !pInspAlgoSolderCone->m_bUseInspVolumePie)
	{
		for (int i = 0; i < eLevelCnt; ++i)
			CalcSolderCone(pInspAlgoSolderCone, i, pfImgSrc, nWidth, nHeight, pUcArrTemp, sRstAlgo, stTieArea);

		bResult = ((sRstAlgo->m_dRstAreaLv1 > sRstAlgo->m_dRstAreaLv2) && (sRstAlgo->m_dRstAreaLv2 > sRstAlgo->m_dRstAreaLv3));
	}

	//um단위로 받음
	double dGapPosH = 0;
	int nVolumeDivCountSpec = 0;
	float fRstVolumeDiff = 0;
	if ((pInspAlgoSolderCone->m_bUseInspVolumeDiff || pInspAlgoSolderCone->m_bUseInspVolumeMinLength || pInspAlgoSolderCone->m_bUseInspVolumePie))		//if Use Volume diff Inspection
	{
		float *pfImgVolumeDiffSrc = sWndAlgoImg.m_fArr3D;
		if ((pfImgVolumeDiffSrc == NULL))
			return bResult;

		//UCHAR* pUcArrTemp1 = NULL;
		UCHAR* puImgBuf = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
		memset(puImgBuf, 0, sizeof(UCHAR) * nWidth * nHeight);


		bool bFlagHVolumeDiffAllOK = true;
		bool bFlagHVolumeMinLengthAllOK = true;
		bool bFlagHVolumePieHAllOK = true;

		float pfRstDiff = 0;

		if (pInspAlgoSolderCone->m_bUseInspVolumeDiff || pInspAlgoSolderCone->m_bUseInspVolumeMinLength)
		{
			//dGapPosH = pInspAlgoSolderCone->m_dGapH;
			nVolumeDivCountSpec = pInspAlgoSolderCone->m_nLevelCounts;

			for (int i = 3; i < nVolumeDivCountSpec + 3; i++)
			{
				bool bRst = CalcSolderVolumeDiff(pInspAlgoSolderCone, i, &pfRstDiff, pfImgSrc, nWidth, nHeight, puImgBuf, sRstAlgo, stTieArea);
			}

			//Level 별로 VolumeDiff 검사 결과가 하나라도 Spec Over하면 NG 판정
			if (pInspAlgoSolderCone->m_bUseInspVolumeDiff)
			{
				for (int nIdx = 0; nIdx < nVolumeDivCountSpec; nIdx++)
				{
					if (sRstAlgo->m_bArrOKVolumeDiff[nIdx] == FALSE && pInspAlgoSolderCone->m_bArrUseVolumeDiffInsp[nIdx] == true)
						bFlagHVolumeDiffAllOK = false;
				}
			}

			if (pInspAlgoSolderCone->m_bUseInspVolumeMinLength)
			{
				for (int nIdx = 0; nIdx < nVolumeDivCountSpec; nIdx++)
				{
					if (sRstAlgo->m_bArrOKVolumeMinLength[nIdx] == FALSE && pInspAlgoSolderCone->m_bArrUseVolumeDiffInsp[nIdx] == true)
						bFlagHVolumeMinLengthAllOK = false;
				}
			}

		}
		else
		{
			//Volume Pie 높이차이 검사하는 경우
			bool bRst = CalcSolderVolumeDiff(pInspAlgoSolderCone, i, &pfRstDiff, pfImgSrc, nWidth, nHeight, puImgBuf, sRstAlgo, stTieArea);

			if (sRstAlgo->m_bOKVolumePie == FALSE && pInspAlgoSolderCone->m_bUseInspVolumePie == true)
				bFlagHVolumeMinLengthAllOK = false;

		}

		if (bFlagHVolumeDiffAllOK == true && bFlagHVolumeMinLengthAllOK == true && bFlagHVolumePieHAllOK == true)
		{
			bResult = TRUE;
		}
		else
			bResult = FALSE;

		CRect rcInspRect(0, 0, 0, 0);
		{
			sRstAlgo->m_rcRect_I.left = rcInspRect.left;
			sRstAlgo->m_rcRect_I.right = rcInspRect.right;
			sRstAlgo->m_rcRect_I.top = rcInspRect.top;
			sRstAlgo->m_rcRect_I.bottom = rcInspRect.bottom;
		}

		Delete_1DArray(&puImgBuf);

	}

	if (pUcArrTemp)
	{
		//delete pUcArrTemp;
		g_pMManager->pem_delete(pUcArrTemp, true);
		pUcArrTemp = NULL;
	}

	return bResult;
}

BOOL CPInsp_AlgoSolderCone::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoSolderCone::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}

BOOL CPInsp_AlgoSolderCone::TeachSolderCone(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, int nIdxSelectLv, RstAlgoSolderCone *pRstAlgo, UCHAR* puImgDst, int nStartX, int nStartY, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;

	if (pRstAlgo)
	{
		memset(pRstAlgo, 0, (sizeof(RstAlgoSolderCone)));
	}

	if (sInspAlgo.m_eAlgoType != eAlgoSolderCone)
		return bResult;
	if ((nIdxSelectLv < eLevel1)/* || (eLevelCnt <= nIdxSelectLv)*/)
		return bResult;
	if ((eVolDifLevelCnt <= nIdxSelectLv))
		return bResult;

	if (!m_pProcMilAlgo)
		return bResult;

	AlgoSolderCone *pInspAlgoSolderCone = (AlgoSolderCone *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoSolderCone)
		return bResult;

	//UCHAR *pucImgSrc	= sWndClipAlgo.m_ucArr2D;
	float *pfImgSrc = sWndClipAlgo.m_fArr3D;
	int nWidth = sWndClipAlgo.m_nWidth;
	int nHeight = sWndClipAlgo.m_nHeight;
	if ((pfImgSrc == NULL) || (nWidth <= 0) || (nHeight <= 0))
		return bResult;

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

	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("ScWin3D.bmp"));
	//UCHAR* puImgBuf = new UCHAR[nWidth * nHeight];
	UCHAR* puImgBuf = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(puImgBuf, 0, sizeof(UCHAR) * nWidth * nHeight);


	//bResult = CalcSolderCone(pInspAlgoSolderCone, nIdxSelectLv, pfImgSrc, nWidth, nHeight, puImgBuf, pRstAlgo, stTieArea);
	if ((pInspAlgoSolderCone->m_bUseInspVolumeDiff || pInspAlgoSolderCone->m_bUseInspVolumeMinLength) && (eLevelCnt <= nIdxSelectLv))
	{
		float pfRstDiff = 0;
		for (int i = 3; i < pInspAlgoSolderCone->m_nLevelCounts + 3; i++)
		{
			bResult = CalcSolderVolumeDiff(pInspAlgoSolderCone, i, &pfRstDiff, pfImgSrc, nWidth, nHeight, puImgBuf, pRstAlgo, stTieArea);
		}

		//Level 별로 VolumeDiff 검사 결과가 하나라도 Spec Over하면 NG 판정
		bool bFlagHVolumeDiffAllOK = true;
		if (pInspAlgoSolderCone->m_bUseInspVolumeDiff)
		{
			for (int nIdx = 0; nIdx < eLevelCnt; nIdx++)
			{
				if (pRstAlgo->m_bArrOKVolumeDiff[nIdx] == FALSE)
					bFlagHVolumeDiffAllOK = false;
			}
		}

		bool bFlagHVolumeMinLengthAllOK = true;
		if (pInspAlgoSolderCone->m_bUseInspVolumeMinLength)
		{
			for (int nIdx = 0; nIdx < eLevelCnt; nIdx++)
			{
				if (pRstAlgo->m_bArrOKVolumeMinLength[nIdx] == FALSE)
					bFlagHVolumeMinLengthAllOK = false;
			}
		}

		if (bFlagHVolumeDiffAllOK == true && bFlagHVolumeMinLengthAllOK == true)
		{
			bResult = TRUE;
		}
		else
			bResult = FALSE;

		//선택한 인덱스 Level 에 해당하는 이미지 세팅을 위해
		bool bTmp = CalcSolderVolumeDiff(pInspAlgoSolderCone, nIdxSelectLv, &pfRstDiff, pfImgSrc, nWidth, nHeight, puImgBuf, pRstAlgo, stTieArea);

	}
	else if (pInspAlgoSolderCone->m_bUseInspVolumePie && (!pInspAlgoSolderCone->m_bUseInspVolumeDiff && !pInspAlgoSolderCone->m_bUseInspVolumeMinLength) && (eLevelCnt <= nIdxSelectLv))
	{
		float pfRstDiff = 0;
		bool bTmp = CalcSolderVolumeDiff(pInspAlgoSolderCone, nIdxSelectLv, &pfRstDiff, pfImgSrc, nWidth, nHeight, puImgBuf, pRstAlgo, stTieArea);

		//VolumePie 검사 결과가 Spec Over 면 NG 판정
		bool bFlagHVolumePieAllOK = true;

		if (pRstAlgo->m_bOKVolumePie == FALSE)
			bFlagHVolumePieAllOK = false;

		if (bFlagHVolumePieAllOK == true)
		{
			bResult = TRUE;
		}
		else
			bResult = FALSE;

	}
	else
		bResult = CalcSolderCone(pInspAlgoSolderCone, nIdxSelectLv, pfImgSrc, nWidth, nHeight, puImgBuf, pRstAlgo, stTieArea);


	m_pProcMilAlgo->SaveWorkImg(puImgBuf, nWidth, nHeight, _T("DstBufBlob.bmp"));
	if (puImgDst)
	{
		int nPartWidth = sWndAlgoImg.m_nWidth;
		int nPartHeight = sWndAlgoImg.m_nHeight;
		if ((nWidth > 0) && (nHeight > 0))
		{
			memset(puImgDst, 0, sizeof(UCHAR) * nPartWidth * nPartHeight);
			int nIndex = 0;
			for (int y = nStartY; y < nStartY + nHeight; y++)
			{
				for (int x = nStartX; x < nStartX + nWidth; x++)
				{
					int nPos = (y * nPartWidth) + x;
					puImgDst[nPos] = puImgBuf[nIndex];
					nIndex++;
				}
			}
			m_pProcMilAlgo->SaveWorkImg(puImgDst, nPartWidth, nPartHeight, _T("DstBlob.bmp"));
		}
	}
	Delete_1DArray(&puImgBuf);
	return bResult;
}

int CPInsp_AlgoSolderCone::CalcSolderCone(float *pfImgSrc, int nImgWidth, int nImgHeight, double fMaxHeight, UCHAR* pUcArrTemp, double &dRateArea, TotalInspExceptArea stTieArea)
{
	int nCntBlob = 0;

	double fMinHeight = 0;
	/*m_pCPInsp_Algo->BlobImageStruct*/
	UCHAR *pUcImgBinary = NULL;
	m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeUpper, fMinHeight, fMaxHeight, false, pUcImgBinary);

	m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nImgWidth, nImgHeight, _T("Bin3D.bmp"));
	// KIY 2020/04/23 : 폴리곤 통합
	m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pUcImgBinary, NULL, stTieArea);

	int nFilter = 0;
	int nMinBlobArea = 4;
	BOOL bApplyFillHole = TRUE;
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pUcImgBinary, pUcArrTemp, nImgWidth, nImgHeight, nMinBlobArea, FALSE, bApplyFillHole, nFilter, eSelectBigger);

	if (pUcArrTemp)
		m_pProcMilAlgo->SaveWorkImg(pUcArrTemp, nImgWidth, nImgHeight, _T("Bin3D_Blob.bmp"));
	if (nCntBlob > 0)
	{
		/*
		int nWhiteCnt = 0;
		double dHeight = 0.0;

		for(int i=0; i < nImgWidth * nImgHeight; ++i)
		{
		if(pUcImgBinary[i] == 255)
		{
		dHeight += pfImgSrc[i];
		nWhiteCnt++;
		}
		}
		dRstHeight = (dHeight / 1000.0) / nWhiteCnt;
		*/

		double dArea = 0.0, dCx = 0.0, dCy = 0.0;
		CRect rcBlob;
		if (M_SUCCESS == m_pProcMilAlgo->GetBlobResult(&dArea, &dCx, &dCy, &rcBlob, FALSE))
		{
			dRateArea = dArea / (nImgWidth * nImgHeight) * 100;
		}
	}
	else
		dRateArea = 0.0;

	if (pUcImgBinary != NULL)
	{
		//delete pUcImgBinary;
		g_pMManager->pem_delete(pUcImgBinary, true);
		pUcImgBinary = NULL;
	}

	return nCntBlob;
}

BOOL CPInsp_AlgoSolderCone::CalcSolderCone(AlgoSolderCone * pInspAlgoSolderCone, int nIdxLevel, float * pfImgSrc, int nImgWidth, int nImgHeight, UCHAR* pUcArrTemp, RstAlgoSolderCone * sRstAlgo, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;
	if (!pInspAlgoSolderCone)
		return bResult;
	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	float fMaxHeight = pInspAlgoSolderCone->m_dHeightLv1;
	if (nIdxLevel == eLevel2)
		fMaxHeight = pInspAlgoSolderCone->m_dHeightLv2;
	else if (nIdxLevel == eLevel3)
		fMaxHeight = pInspAlgoSolderCone->m_dHeightLv3;

	double dRateArea = 0.0;
	int nCntBlob = CalcSolderCone(pfImgSrc, nImgWidth, nImgHeight, fMaxHeight, pUcArrTemp, dRateArea, stTieArea);

	bResult = TRUE;
	if (sRstAlgo)
	{
		if (nIdxLevel == eLevel2)
			sRstAlgo->m_dRstAreaLv2 = dRateArea;
		else if (nIdxLevel == eLevel3)
			sRstAlgo->m_dRstAreaLv3 = dRateArea;
		else
			sRstAlgo->m_dRstAreaLv1 = dRateArea;
	}

	bResult = TRUE;
	return bResult;
}


bool CPInsp_AlgoSolderCone::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}

BOOL CPInsp_AlgoSolderCone::CalcSolderVolumeDiff(AlgoSolderCone * pInspAlgoSolderCone, int nIdxLevel, float * pfRstDiff, float * pfImgSrc, int nImgWidth, int nImgHeight, UCHAR* pUcArrTemp, RstAlgoSolderCone * sRstAlgo, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;
	if (!pInspAlgoSolderCone)
		return bResult;
	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	float fMaxHeight = 0;
	int nLevelCnt = pInspAlgoSolderCone->m_nLevelCounts;
	for (int nIdx = 0; nIdx < nLevelCnt; nIdx++)
	{
		if (nIdx == nIdxLevel - 3)
		{
			float fVolDiffMax = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdx][eMMD_Max];			//최대 스펙 값
			float fVolDiffMin = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdx][eMMD_Min];			//최대 스펙 값
			float fVolDiffDefault = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdx][eMMD_Default];	//높이 값 시작 범위

			fMaxHeight = fVolDiffDefault;
		}

	}

	cv::Mat imgSrc(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);

	double dHighestH = pInspAlgoSolderCone->m_dHighestHeight;
	double dgapH = pInspAlgoSolderCone->m_dGapH;

	double dMinDiff = 0.0;
	double dMaxDiff = 0.0;
	cv::Point VolumeCenterPo(0, 0);
	POINTF maxPoints[2], minPoints[2];

	RECT rcNGArea;
	rcNGArea.left = 0, rcNGArea.top = 0, rcNGArea.right = 0, rcNGArea.bottom = 0;
	int nCntBlob2 = CalcSolderVolumeDiff(pfImgSrc, &VolumeCenterPo, nImgWidth, nImgHeight, dHighestH, dgapH, fMaxHeight, pUcArrTemp, dMinDiff, dMaxDiff, minPoints, maxPoints, stTieArea, pInspAlgoSolderCone->m_bUseSolderHighestVolumeDiff, pInspAlgoSolderCone->m_bUseInspVolumePie, &rcNGArea);
	cv::Mat imgDst2(nImgHeight, nImgWidth, CV_8UC1, pUcArrTemp);

	float fRstMinLength = 0;
	if (pInspAlgoSolderCone->m_bUseInspVolumeDiff || pInspAlgoSolderCone->m_bUseInspVolumeMinLength)
	{
		*pfRstDiff = (float)(dMaxDiff - dMinDiff);
		fRstMinLength = (float)(dMinDiff);
	}
	else if (pInspAlgoSolderCone->m_bUseInspVolumePie)
	{
		*pfRstDiff = (float)(dMaxDiff - dMinDiff);
	}


	bResult = TRUE;
	if (sRstAlgo)
	{

		sRstAlgo->m_fRstMaxDiffHeightInPie = pInspAlgoSolderCone->m_fPieHeightMaxDiff;

		if (pInspAlgoSolderCone->m_bUseInspVolumeDiff || pInspAlgoSolderCone->m_bUseInspVolumeMinLength)
		{
			for (int nIdx = 0; nIdx < nLevelCnt; nIdx++)
			{
				if (nIdx == nIdxLevel - 3)
				{
					float fVolDiffMax = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdx][eMMD_Max];			//최대 스펙 값
					float fVolDiffMin = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdx][eMMD_Min];			//최소 스펙 값

					float fVolDiffDefault = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdx][eMMD_Default];	//높이 값 시작 범위

					sRstAlgo->m_fArrRstVolumeDiff[nIdx] = *pfRstDiff * m_resolX;
					sRstAlgo->m_fArrRstVolumeMinLength[nIdx] = fRstMinLength * m_resolX;

					float fRstVolumeDiff = sRstAlgo->m_fArrRstVolumeDiff[nIdx];
					float fRstVolumeMinLength = sRstAlgo->m_fArrRstVolumeMinLength[nIdx];

					if (pInspAlgoSolderCone->m_bUseInspVolumeDiff)
					{
						if (fVolDiffMax > fRstVolumeDiff)
							sRstAlgo->m_bArrOKVolumeDiff[nIdx] = TRUE;
						else
							sRstAlgo->m_bArrOKVolumeDiff[nIdx] = FALSE;
					}

					if (pInspAlgoSolderCone->m_bUseInspVolumeMinLength)
					{
						//VolumeDiff 최소길이 검사
						if (fVolDiffMin < fRstVolumeMinLength)
							sRstAlgo->m_bArrOKVolumeMinLength[nIdx] = TRUE;
						else
							sRstAlgo->m_bArrOKVolumeMinLength[nIdx] = FALSE;
					}

					sRstAlgo->reLongAxisPoinSt[nIdx].x = (int)maxPoints[0].x;
					sRstAlgo->reLongAxisPoinSt[nIdx].y = (int)maxPoints[0].y;
					sRstAlgo->reLongAxisPoinEd[nIdx].x = (int)maxPoints[1].x;
					sRstAlgo->reLongAxisPoinEd[nIdx].y = (int)maxPoints[1].y;

					sRstAlgo->reShortAxisPoinSt[nIdx].x = (int)minPoints[0].x;
					sRstAlgo->reShortAxisPoinSt[nIdx].y = (int)minPoints[0].y;
					sRstAlgo->reShortAxisPoinEd[nIdx].x = (int)minPoints[1].x;
					sRstAlgo->reShortAxisPoinEd[nIdx].y = (int)minPoints[1].y;
				}
			}
		}
		else if (pInspAlgoSolderCone->m_bUseInspVolumePie)
		{
			float fTeachVolPieHDiffMax = pInspAlgoSolderCone->m_fPieHeightMaxDiff;			//최대 스펙 값

			sRstAlgo->m_fRstMaxDiffHeightInPie = *pfRstDiff;	// *m_resolX;
			float fRstVolumePieH = sRstAlgo->m_fRstMaxDiffHeightInPie;

			if (fRstVolumePieH < fTeachVolPieHDiffMax)
				sRstAlgo->m_bOKVolumePie = TRUE;
			else
				sRstAlgo->m_bOKVolumePie = FALSE;

			sRstAlgo->reLongAxisPoinSt[0].x = (int)maxPoints[0].x;
			sRstAlgo->reLongAxisPoinSt[0].y = (int)maxPoints[0].y;
			sRstAlgo->reLongAxisPoinEd[0].x = (int)maxPoints[1].x;
			sRstAlgo->reLongAxisPoinEd[0].y = (int)maxPoints[1].y;

			sRstAlgo->reShortAxisPoinSt[0].x = (int)maxPoints[0].x;
			sRstAlgo->reShortAxisPoinSt[0].y = (int)maxPoints[0].y;
			sRstAlgo->reShortAxisPoinEd[0].x = (int)maxPoints[1].x;
			sRstAlgo->reShortAxisPoinEd[0].y = (int)maxPoints[1].y;

			sRstAlgo->m_rcRect_I_NGArea.left = rcNGArea.left;
			sRstAlgo->m_rcRect_I_NGArea.top = rcNGArea.top;
			sRstAlgo->m_rcRect_I_NGArea.right = rcNGArea.right;
			sRstAlgo->m_rcRect_I_NGArea.bottom = rcNGArea.bottom;
		}

		sRstAlgo->m_sPoint.x = (float)VolumeCenterPo.x;
		sRstAlgo->m_sPoint.y = (float)VolumeCenterPo.y;

		//Level 별로 VolumeDiff 검사 결과가 하나라도 Spec Over하면 NG 판정
// 		bool bFlagHVolumeDiffAllOK = true;
// 		for (int nIdx = 0; nIdx < nLevelCnt; nIdx++)
// 		{
// 			if (sRstAlgo->m_bArrOKVolumeDiff[nIdx] == FALSE)
// 				bFlagHVolumeDiffAllOK = false;
// 		}
// 
// 		if (bFlagHVolumeDiffAllOK == true)
// 		{
// 			bResult = TRUE;
// 		}
// 		else
// 			bResult = FALSE;

	}

	//bResult = TRUE;
	return bResult;
}

BOOL CPInsp_AlgoSolderCone::CalcSolderVolumeDiff_Inspection(AlgoSolderCone * pInspAlgoSolderCone, int nIdxLevel, float * pfRstDiff, float * pfImgSrc, int nImgWidth, int nImgHeight, UCHAR* pUcArrTemp, RstAlgoSolderCone * sRstAlgo, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;
	if (!pInspAlgoSolderCone)
		return bResult;
	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	cv::Mat imgSrc(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);

	float fMaxHeight = 0;
	double dHighestH = pInspAlgoSolderCone->m_dHighestHeight;
	double dgapH = pInspAlgoSolderCone->m_dGapH;
	cv::Point VolumeCenterPo(0, 0);

	//UCHAR* puImgBuf = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
	//memset(puImgBuf, 0, sizeof(UCHAR) * nWidth * nHeight);

	int nLevelCnt = pInspAlgoSolderCone->m_nLevelCounts;
	//for (int nIdx = 0; nIdx < nLevelCnt; nIdx++)
	{
		float fVolDiffMax = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdxLevel][eMMD_Max];			//최대 스펙 값
		float fVolDiffMin = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdxLevel][eMMD_Min];			//최소 스펙 값

		float fVolDiffDefault = pInspAlgoSolderCone->m_fArrVolumeDiff[nIdxLevel][eMMD_Default];	//높이 값 시작 범위

		fMaxHeight = fVolDiffDefault;

		double dMinDiff = 0.0;
		double dMaxDiff = 0.0;
		POINTF maxPoints[2], minPoints[2];

		RECT rcNGArea;
		rcNGArea.left = 0, rcNGArea.top = 0, rcNGArea.right = 0, rcNGArea.bottom = 0;
		int nCntBlob2 = CalcSolderVolumeDiff(pfImgSrc, &VolumeCenterPo, nImgWidth, nImgHeight, dHighestH, dgapH, fMaxHeight, pUcArrTemp, dMinDiff, dMaxDiff, minPoints, maxPoints, stTieArea, pInspAlgoSolderCone->m_bUseSolderHighestVolumeDiff, pInspAlgoSolderCone->m_bUseInspVolumePie, &rcNGArea);
		cv::Mat imgDst2(nImgHeight, nImgWidth, CV_8UC1, pUcArrTemp);
		*pfRstDiff = (float)(dMaxDiff - dMinDiff);
		float fRstMinLength = (float)(dMinDiff);

		sRstAlgo->m_fArrRstVolumeDiff[nIdxLevel] = *pfRstDiff * m_resolX;

		sRstAlgo->m_fArrRstVolumeMinLength[nIdxLevel] = fRstMinLength * m_resolX;

		float fRstVolumeDiff = sRstAlgo->m_fArrRstVolumeDiff[nIdxLevel];
		float fRstVolumeMinLength = sRstAlgo->m_fArrRstVolumeMinLength[nIdxLevel];
		float fRstVolumePie = sRstAlgo->m_fRstMaxDiffHeightInPie;

		if (fVolDiffMax > fRstVolumeDiff)
			sRstAlgo->m_bArrOKVolumeDiff[nIdxLevel] = TRUE;
		else
			sRstAlgo->m_bArrOKVolumeDiff[nIdxLevel] = FALSE;

		//VolumeDiff 최소길이 검사
		if (fVolDiffMin < fRstVolumeMinLength)
			sRstAlgo->m_bArrOKVolumeMinLength[nIdxLevel] = TRUE;
		else
			sRstAlgo->m_bArrOKVolumeMinLength[nIdxLevel] = FALSE;

		sRstAlgo->reLongAxisPoinSt[nIdxLevel].x = (int)maxPoints[0].x;
		sRstAlgo->reLongAxisPoinSt[nIdxLevel].y = (int)maxPoints[0].y;
		sRstAlgo->reLongAxisPoinEd[nIdxLevel].x = (int)maxPoints[1].x;
		sRstAlgo->reLongAxisPoinEd[nIdxLevel].y = (int)maxPoints[1].y;

		sRstAlgo->reShortAxisPoinSt[nIdxLevel].x = (int)minPoints[0].x;
		sRstAlgo->reShortAxisPoinSt[nIdxLevel].y = (int)minPoints[0].y;
		sRstAlgo->reShortAxisPoinEd[nIdxLevel].x = (int)minPoints[1].x;
		sRstAlgo->reShortAxisPoinEd[nIdxLevel].y = (int)minPoints[1].y;

	}

	bResult = TRUE;
	if (sRstAlgo)
	{
		sRstAlgo->m_sPoint.x = (float)VolumeCenterPo.x;
		sRstAlgo->m_sPoint.y = (float)VolumeCenterPo.y;

		//Level 별로 VolumeDiff 검사 결과가 하나라도 Spec Over하면 NG 판정
		bool bFlagHVolumeDiffAllOK = true;
		if (pInspAlgoSolderCone->m_bUseInspVolumeDiff)
		{
			for (int nIdx = 0; nIdx < nLevelCnt; nIdx++)
			{
				if (sRstAlgo->m_bArrOKVolumeDiff[nIdx] == FALSE && pInspAlgoSolderCone->m_bArrUseVolumeDiffInsp[nIdx] == true)
					bFlagHVolumeDiffAllOK = false;
			}
		}

		bool bFlagHVolumeMinLengthAllOK = true;
		if (pInspAlgoSolderCone->m_bUseInspVolumeMinLength)
		{
			for (int nIdx = 0; nIdx < nLevelCnt; nIdx++)
			{
				if (sRstAlgo->m_bArrOKVolumeMinLength[nIdx] == FALSE && pInspAlgoSolderCone->m_bArrUseVolumeDiffInsp[nIdx] == true)
					bFlagHVolumeMinLengthAllOK = false;
			}
		}

		bool bFlagVolumePieOK = true;
		if (pInspAlgoSolderCone->m_bUseInspVolumePie)
		{
			//for (int nIdx = 0; nIdx < nLevelCnt; nIdx++)
			{
				if (sRstAlgo->m_bOKVolumePie == FALSE && pInspAlgoSolderCone->m_bUseInspVolumePie == true)
					bFlagVolumePieOK = false;
			}
		}

		if (bFlagHVolumeDiffAllOK == true && bFlagHVolumeMinLengthAllOK == true && bFlagVolumePieOK == true)
		{
			bResult = TRUE;
		}
		else
			bResult = FALSE;




	}

	//bResult = TRUE;
	return bResult;
}




int CPInsp_AlgoSolderCone::CalcSolderVolumeDiff(float *pfImgSrc, cv::Point* CenterPo, int nImgWidth, int nImgHeight, double dHighestH, double dGapH, double fMaxHeight, UCHAR* pUcArrTemp, double &dMinDistance, double &dMaxDistance, POINTF *minPoints, POINTF *maxPoints, TotalInspExceptArea stTieArea, bool bUseSolderHighestVolumeDiff, bool bUseInspVolumePieHDiff, RECT *rcNGArea)
{
	int nCntBlob = 0;

	UCHAR *pUcInputImg = NULL;
	if (dHighestH < 0.0)
		dHighestH = 0.0;

	if (dGapH <= 0.0)	//gap 초기값 최소 10um 지정(최고 높이에서는 구간 BW 생성 불가)
		dGapH = 10.0;

	double fMinH_forCalcCtr = 0;
	fMinH_forCalcCtr = dHighestH - dGapH;

	if (fMinH_forCalcCtr < 0)
		fMinH_forCalcCtr = 0;

	bool bUseHighestH = bUseSolderHighestVolumeDiff;

	//두 영상(바깥쪽 Blob, 안쪽 blob)을 담을 matrix 선언
	cv::Mat cvBWInnerBlob(nImgHeight, nImgWidth, CV_8UC1);
	cvBWInnerBlob.setTo(0);

	//3D 영상 확인
	cv::Mat cvSolderConeImg3D(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);


	cv::Mat cvBWOuterBlob(nImgHeight, nImgWidth, CV_8UC1);
	cvBWOuterBlob.setTo(0);

	//1. 안쪽 원(pin)의 3D BW 영상 생성 [max:(Highest H), min:(Highest H - gap H)]
	if (!bUseSolderHighestVolumeDiff)
		m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeIn, (int)dHighestH - dGapH, (int)dHighestH, false, pUcInputImg);
	else
		m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeIn, (int)dHighestH - 20, (int)dHighestH, false, pUcInputImg);

	m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pUcInputImg, NULL, stTieArea);	// KIY 2020/04/23 : 폴리곤 통합

	int nFilter = 0;
	int nMinBlobArea = 20;

	if (bUseSolderHighestVolumeDiff)
		nMinBlobArea = 1;

	BOOL bApplyFillHole = TRUE;
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pUcInputImg, pUcArrTemp, nImgWidth, nImgHeight, nMinBlobArea, FALSE, bApplyFillHole, nFilter, eSelectBigger);

	if (nCntBlob <= 0)
	{
		//return 0;
		pUcInputImg = NULL;

		if (!bUseSolderHighestVolumeDiff)
		{
			dGapH = dGapH + (dHighestH*0.1);
			m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeIn, (int)dHighestH - dGapH, (int)dHighestH, false, pUcInputImg);
		}
		else
		{
			m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeIn, (int)dHighestH - (dHighestH*0.1), (int)dHighestH, false, pUcInputImg);
		}

		m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pUcInputImg, NULL, stTieArea);	// KIY 2020/04/23 : 폴리곤 통합
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pUcInputImg, pUcArrTemp, nImgWidth, nImgHeight, nMinBlobArea, FALSE, bApplyFillHole, nFilter, eSelectBigger);

		if (nCntBlob <= 0)
			return 0;
	}

	cv::Mat cvBWImg_HighestH(nImgHeight, nImgWidth, CV_8UC1, pUcArrTemp);
	cvBWInnerBlob = cvBWImg_HighestH.clone();


	//1-1. 안쪽(pin) 원 BW 영상의 외곽선 추출
	std::vector<std::vector<cv::Point>> contours_outLine;
	std::vector<cv::Vec4i> hierarchy_outLine;
	double dMxArea = 0;
	dMxArea = 100;		//외곽선 포인트 최소 개수
	int idx = 0;

	contours_outLine.clear();
	hierarchy_outLine.clear();
	cv::findContours(cvBWInnerBlob, contours_outLine, hierarchy_outLine, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

	cv::Mat cvBlobContour2D_outLine(nImgHeight, nImgWidth, CV_8UC1);
	cvBlobContour2D_outLine.setTo(0);
	idx = 0;
	for (contours_outLine.begin(); idx < contours_outLine.size(); idx++)
	{
		//drawContours(cvBlobContour2D_outLine, contours_outLine, idx, cv::Scalar(255), cv::FILLED, 8, hierarchy_outLine);
		drawContours(cvBlobContour2D_outLine, contours_outLine, idx, cv::Scalar(255), 1, 8, hierarchy_outLine);
	}

	std::vector<POINTF> m_vCircleContour_outLine;
	m_vCircleContour_outLine.clear();
	if (contours_outLine.size() > 0)
	{
		std::vector<cv::Point> selectedCon;
		for (int cCnt = 0; cCnt < contours_outLine.size(); cCnt++)
		{
			if (contours_outLine[cCnt].size() < 6 && !bUseSolderHighestVolumeDiff)
				continue;

			if (contours_outLine[cCnt].size() < selectedCon.size())
				continue;

			selectedCon = contours_outLine[cCnt];

			int cnt = contours_outLine[cCnt].size();
			m_vCircleContour_outLine.clear();
			m_vCircleContour_outLine.reserve(cnt);
			for (int i = 0; i < cnt; i++)
			{
				POINTF pt;
				pt.x = contours_outLine[cCnt][i].x;
				pt.y = contours_outLine[cCnt][i].y;

				m_vCircleContour_outLine.emplace_back(pt);
			}

			//poTargetContour = m_vCircleContour_outLine[0];
		}

	}
	/////////////////////////////////////////////////////////////////

	cv::Mat cvInspAreaCir(nImgHeight, nImgWidth, CV_8UC1);
	cvInspAreaCir.setTo(0);
	int nIdxMaxDiff = -1, nIdxMinDiff = -1;		//pie 영역 8개 중 높이값이 가장 작은 index 저장
	std::vector<std::vector<cv::Point>> fDevidedPiePosList;
	std::vector<cv::Point> vPosTemp1L, vPosTemp1R, vPosTemp2L, vPosTemp2R, vPosTemp3L, vPosTemp3R, vPosTemp4L, vPosTemp4R;
	fDevidedPiePosList.clear();

	vPosTemp1L.clear(); vPosTemp1R.clear(); vPosTemp2L.clear(); vPosTemp2R.clear();
	vPosTemp3L.clear(); vPosTemp3R.clear(); vPosTemp4L.clear(); vPosTemp4R.clear();

	if (bUseInspVolumePieHDiff)		//Pie 높이차이 검사 시에만 진입
	{
		//3D 영상으로 pin 부분의 높이평균 계산
		float fPinSumH = 0;
		int nPinArea = 0;
		float fPinAvgH = 0;
		cv::Mat Foot3DImg = cvSolderConeImg3D.clone();
		size_t floatstep_3DCorrect = Foot3DImg.step / sizeof(float);

		for (int r = 0; r < Foot3DImg.rows; r++)
		{
			UCHAR* Ptr_pin2D = cvBWImg_HighestH.data;
			float* Ptr_3D = (float*)Foot3DImg.data;

			for (int c = 0; c < Foot3DImg.cols; c++)
			{
				//pin 2D Blob 부분만 높이 값 취함 
				if (Ptr_pin2D[r*cvBWImg_HighestH.step + c] == 255)
				{
					fPinSumH += Ptr_3D[r * floatstep_3DCorrect + c];
					nPinArea++;
				}
			}
		}

		if (fPinSumH > 0 && nPinArea > 0)
		{
			fPinAvgH = fPinSumH / nPinArea;
		}

		//3D 영상으로 pin 주변 부분만 취함
		std::vector<cv::Point> targetContour;
		for (int a = 0; a < contours_outLine.size(); a++)
		{
			//cv::Rect contourRect = cv::boundingRect(contours[a]);
			//if (contourRect.width == rt.width && contourRect.height == rt.height)	// Blob Rect와 동일한 크기인 Contour 선택
			{
				targetContour = contours_outLine[a];
				break;
			}
		}

		cv::Mat cvBlobContour_pin(nImgHeight, nImgWidth, CV_8UC1);
		cvBlobContour_pin.setTo(0);
		idx = 0;
		for (contours_outLine.begin(); idx < contours_outLine.size(); idx++)
		{
			drawContours(cvBlobContour_pin, contours_outLine, idx, cv::Scalar(255), cv::FILLED, 8, hierarchy_outLine);
		}

		//pin 검출부 Circle fitting
		double centX, centY;
		double err = 0.0, radius = 0;
		if (targetContour.size() > 0)
			CircleFitting(targetContour, &centX, &centY, &radius, &err);

		cv::Mat sCircle(nImgHeight, nImgWidth, CV_8UC1);
		sCircle.setTo(0);
		cv::circle(sCircle, cv::Point(centX, centY), radius, cv::Scalar(255), 2);

		//높이 단차 검사부 Circle 영역 만듦
		cv::Mat sCircleMargin(nImgHeight, nImgWidth, CV_8UC1);
		sCircleMargin.setTo(0);
		cv::circle(sCircleMargin, cv::Point(centX, centY), radius + 10, cv::Scalar(255), -1);

		//vCircleErr[i] = 1.0 - err; // 원형율 score 저장. score가 높으면 이물일 가능성이 높으니까 다른 Blob들과 직선성분으로 묶지 않음.

		sCircleMargin = sCircleMargin - cvBlobContour_pin;	//원형 검사 영역에서 Pin 영역 제외하기
		/////////////////////////////////////////////////////////////

		//3D 영상으로 pin 높이랑 가장 많이 높이가 차이나는 부분의 좌표 계산 
		float fMaxHeightDiff = 0;
		cv::Point po;
		po.x = 0; po.y = 0;

		//#원형 검사영역을 케이크처럼 분할해서 각 영역의 평균 높이 확인
		//1.원을 8개로 분할
		cv::Mat cvRoundSection_1(nImgHeight, nImgWidth, CV_8UC1);
		cv::Mat cvRoundSection_2(nImgHeight, nImgWidth, CV_8UC1);
		cv::Mat cvRoundSection_3(nImgHeight, nImgWidth, CV_8UC1);
		cv::Mat cvRoundSection_4(nImgHeight, nImgWidth, CV_8UC1);

		cvRoundSection_1.setTo(0);
		cvRoundSection_2.setTo(0);
		cvRoundSection_3.setTo(0);
		cvRoundSection_4.setTo(0);


		cvInspAreaCir = sCircleMargin.clone();

		for (int i = 0; i < 8; i++)
		{
			double angle = i * (360.0 / 8); // 각도 계산

			cv::Point startPoint(centX + (radius + 11) * std::cos(angle * CV_PI / 180),
				centY + (radius + 11) * std::sin(angle * CV_PI / 180));

			cv::Point endPoint(centX + (radius + 11) * std::cos((angle + 180) * CV_PI / 180),
				centY + (radius + 11) * std::sin((angle + 180) * CV_PI / 180));

			cv::line(cvInspAreaCir, startPoint, endPoint, cv::Scalar(0, 0, 0), 2);	// 원을 8개의 조각으로 분할한 선 그리기

			double dStartAngle = i * (360.0 / 8);
			double dEndAngle = (i + 1) * (360.0 / 8);

			if (i == 0)
				cv::ellipse(cvRoundSection_1, cv::Point(centX, centY), cv::Size(radius + 11, radius + 11), angle, dStartAngle, dEndAngle, cv::Scalar(255, 255, 255), -1);
			if (i == 1)
				cv::ellipse(cvRoundSection_2, cv::Point(centX, centY), cv::Size(radius + 11, radius + 11), angle, dStartAngle, dEndAngle, cv::Scalar(255, 255, 255), -1);
			if (i == 2)
				cv::ellipse(cvRoundSection_3, cv::Point(centX, centY), cv::Size(radius + 11, radius + 11), angle, dStartAngle, dEndAngle, cv::Scalar(255, 255, 255), -1);
			if (i == 3)
				cv::ellipse(cvRoundSection_4, cv::Point(centX, centY), cv::Size(radius + 11, radius + 11), angle, dStartAngle, dEndAngle, cv::Scalar(255, 255, 255), -1);

		}

		//2.분할한 조각을 실제 검사영역과 and 연산으로 오직 검사대상만 추출
		cvRoundSection_1 = cvRoundSection_1 & cvInspAreaCir;
		cvRoundSection_2 = cvRoundSection_2 & cvInspAreaCir;
		cvRoundSection_3 = cvRoundSection_3 & cvInspAreaCir;
		cvRoundSection_4 = cvRoundSection_4 & cvInspAreaCir;

		float fSumH_1R = 0, fSumH_2R = 0, fSumH_3R = 0, fSumH_4R = 0;
		float fSumH_1L = 0, fSumH_2L = 0, fSumH_3L = 0, fSumH_4L = 0;

		int nCntArea_1R = 0, nCntArea_2R = 0, nCntArea_3R = 0, nCntArea_4R = 0;
		int nCntArea_1L = 0, nCntArea_2L = 0, nCntArea_3L = 0, nCntArea_4L = 0;

		float fAvgH_1R = 0, fAvgH_2R = 0, fAvgH_3R = 0, fAvgH_4R = 0;
		float fAvgH_1L = 0, fAvgH_2L = 0, fAvgH_3L = 0, fAvgH_4L = 0;			

		//검사 영역 여러개로 쪼개서 평균 높이 확인
		{
			//우선은 4개영역으로 쪼개기
			for (int r = 0; r < Foot3DImg.rows; r++)
			{
				UCHAR* Ptr_pin2DMarginROI = sCircleMargin.data;

				UCHAR* Ptr_pin2DMarginROI_1R = cvRoundSection_4.data;
				UCHAR* Ptr_pin2DMarginROI_2R = cvRoundSection_3.data;
				UCHAR* Ptr_pin2DMarginROI_3R = cvRoundSection_2.data;
				UCHAR* Ptr_pin2DMarginROI_4R = cvRoundSection_1.data;

				float* Ptr_3D = (float*)Foot3DImg.data;

				cv::Point vPTemp;
				vPTemp.x = 0; vPTemp.y = 0;
				for (int c = 0; c < Foot3DImg.cols; c++)
				{
					if (Ptr_pin2DMarginROI[r * sCircleMargin.step + c] == 255 && Ptr_3D[r * floatstep_3DCorrect + c] > 0)
					{
						if (centX < c  && centY > r)	//1사분면 확인
						{
							if (Ptr_pin2DMarginROI_1R[r * sCircleMargin.step + c] == 255)
							{
								fSumH_1R = fSumH_1R + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_1R++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp1R.push_back(vPTemp);
							}
							else
							{
								fSumH_1L = fSumH_1L + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_1L++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp1L.push_back(vPTemp);
							}
						}
						else if (centX > c  && centY > r)		//2사분면
						{
							if (Ptr_pin2DMarginROI_2R[r * sCircleMargin.step + c] == 255)
							{
								fSumH_2R = fSumH_2R + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_2R++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp2R.push_back(vPTemp);
							}
							else
							{
								fSumH_2L = fSumH_2L + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_2L++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp2L.push_back(vPTemp);
							}
						}
						else if (centX > c  && centY < r)		//3사분면
						{
							if (Ptr_pin2DMarginROI_3R[r * sCircleMargin.step + c] == 255)
							{
								fSumH_3R = fSumH_3R + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_3R++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp3R.push_back(vPTemp);
							}
							else
							{
								fSumH_3L = fSumH_3L + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_3L++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp3L.push_back(vPTemp);
							}
						}
						else if (centX < c  && centY < r)		//4사분면
						{
							if (Ptr_pin2DMarginROI_4R[r * sCircleMargin.step + c] == 255)
							{
								fSumH_4R = fSumH_4R + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_4R++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp4R.push_back(vPTemp);
							}
							else
							{
								fSumH_4L = fSumH_4L + Ptr_3D[r * floatstep_3DCorrect + c];
								nCntArea_4L++;
								vPTemp.x = c; vPTemp.y = r;
								vPosTemp4L.push_back(vPTemp);
							}
						}
					}
				}
			}

			if (fSumH_1L > 0 && nCntArea_1L > 0)
			{
				fAvgH_1L = fSumH_1L / nCntArea_1L;
			}
			if (fSumH_2L > 0 && nCntArea_2L > 0)
			{
				fAvgH_2L = fSumH_2L / nCntArea_2L;
			}
			if (fSumH_3L > 0 && nCntArea_3L > 0)
			{
				fAvgH_3L = fSumH_3L / nCntArea_3L;
			}
			if (fSumH_4L > 0 && nCntArea_4L > 0)
			{
				fAvgH_4L = fSumH_4L / nCntArea_4L;
			}

			if (fSumH_1R > 0 && nCntArea_1R > 0)
			{
				fAvgH_1R = fSumH_1R / nCntArea_1R;
			}
			if (fSumH_2R > 0 && nCntArea_2R > 0)
			{
				fAvgH_2R = fSumH_2R / nCntArea_2R;
			}
			if (fSumH_3R > 0 && nCntArea_3R > 0)
			{
				fAvgH_3R = fSumH_3R / nCntArea_3R;
			}
			if (fSumH_4R > 0 && nCntArea_4R > 0)
			{
				fAvgH_4R = fSumH_4R / nCntArea_4R;
			}

		}

		fDevidedPiePosList.emplace_back(vPosTemp1L);	fDevidedPiePosList.emplace_back(vPosTemp1R);
		fDevidedPiePosList.emplace_back(vPosTemp2L);	fDevidedPiePosList.emplace_back(vPosTemp2R);
		fDevidedPiePosList.emplace_back(vPosTemp3L);	fDevidedPiePosList.emplace_back(vPosTemp3R);
		fDevidedPiePosList.emplace_back(vPosTemp4L);	fDevidedPiePosList.emplace_back(vPosTemp4R);




		std::vector<float> fPieHeightList;
		fPieHeightList.clear();
		fPieHeightList.emplace_back(fAvgH_1L);		fPieHeightList.emplace_back(fAvgH_1R);
		fPieHeightList.emplace_back(fAvgH_2L);		fPieHeightList.emplace_back(fAvgH_2R);
		fPieHeightList.emplace_back(fAvgH_3L);		fPieHeightList.emplace_back(fAvgH_3R);
		fPieHeightList.emplace_back(fAvgH_4L);		fPieHeightList.emplace_back(fAvgH_4R);


		float fPieMaxHeight = 0;
		float fPieMinHeight = 1000;


		for (int i = 0; i < 8; i++)
		{
			if (fPieMaxHeight < fPieHeightList[i])
			{
				fPieMaxHeight = fPieHeightList[i];
				nIdxMaxDiff = i;
			}
			if (fPieMinHeight > fPieHeightList[i])
			{
				fPieMinHeight = fPieHeightList[i];
				nIdxMinDiff = i;
			}
		}

		if (fPieMaxHeight != 0 && fPieMinHeight != 1000)
		{
			//결과 값 세팅
			dMinDistance = fPieMinHeight;
			dMaxDistance = fPieMaxHeight;

			for (int i = 0; i < 8; i++)
			{
				if (i == nIdxMinDiff)
				{
					int nCntPieHeight = fDevidedPiePosList[i].size();
					int nPoX = 0, nPoY = 0;

					for (int j = 0; j < nCntPieHeight; j++)
					{
						nPoX = fDevidedPiePosList[i][j].x;
						nPoY = fDevidedPiePosList[i][j].y;

						maxPoints[0].x = nPoX; maxPoints[0].y = nPoY;
						maxPoints[1].x = nPoX; maxPoints[1].y = nPoY;

						break;
					}

					cv::Mat cvMaxHDiffPosition(nImgHeight, nImgWidth, CV_8UC1);
					cvMaxHDiffPosition.setTo(0);

					//해당 좌표 확인을 위한 점 찍기
					if (nPoX != 0 && nPoY != 0)
					{
						cv::Point cvPos;
						cvPos.x = nPoX; cvPos.y = nPoY;
						cv::Scalar color = cv::Scalar(255, 255, 255);
						cv::line(cvMaxHDiffPosition, cvPos, cvPos, color, 3);

						cv::Mat cvSolderVol2D(nImgHeight, nImgWidth, CV_8UC1, pUcArrTemp);
						cv::cvtColor(cvSolderVol2D, cvSolderVol2D, cv::COLOR_GRAY2RGB);
						{
							cv::Scalar color = cv::Scalar(255, 255, 255);
							cv::line(cvSolderVol2D, cvPos, cvPos, cv::Scalar(0, 0, 255), 3);
						}
					}
				}
			}

			//maxPoints[0].x = cvMaxDist_st.x; maxPoints[0].y = cvMaxDist_st.y;
			//maxPoints[1].x = cvMaxDist_ed.x; maxPoints[1].y = cvMaxDist_ed.y;

			minPoints[0].x = maxPoints[0].x; minPoints[0].y = maxPoints[0].y;
			minPoints[1].x = maxPoints[1].x; maxPoints[1].y = maxPoints[1].y;

		}

	}

	//if (nCntBlob <= 0)
	//	return 0;

	//2. 안쪽 원 BW의 중심위치 검출
	int nHighestVolRectMinX = cvBWImg_HighestH.cols;
	int nHighestVolRectMaxX = 0;
	int nHighestVolRectMinY = cvBWImg_HighestH.rows;
	int nHighestVolRectMaxY = 0;

	for (int r = 0; r < cvBWImg_HighestH.rows; r++)
	{
		UCHAR* Ptr = cvBWImg_HighestH.data;
		for (int c = 0; c < cvBWImg_HighestH.cols; c++)
		{
			if (Ptr[r * cvBWImg_HighestH.step + c] == 255)
			{
				if (r < nHighestVolRectMinY)	nHighestVolRectMinY = r;
				if (r > nHighestVolRectMaxY)	nHighestVolRectMaxY = r;
				if (c < nHighestVolRectMinX)	nHighestVolRectMinX = c;
				if (c > nHighestVolRectMaxX)	nHighestVolRectMaxX = c;

			}
		}
	}

	cv::Point poHighestVolCenter; poHighestVolCenter.x = 0; poHighestVolCenter.y = 0;
	if (nHighestVolRectMinX != cvBWImg_HighestH.cols && nHighestVolRectMinY != cvBWImg_HighestH.cols &&
		nHighestVolRectMaxX != 0 && nHighestVolRectMaxY != 0)
	{
		poHighestVolCenter.x = nHighestVolRectMinX + (nHighestVolRectMaxX - nHighestVolRectMinX + 1) / 2;
		poHighestVolCenter.y = nHighestVolRectMinY + (nHighestVolRectMaxY - nHighestVolRectMinY + 1) / 2;
	}

	CenterPo->x = poHighestVolCenter.x;
	CenterPo->y = poHighestVolCenter.y;

	if (bUseInspVolumePieHDiff && nIdxMinDiff !=-1)
	{
		int nPosX = 0, nPosY = 0;
		int nMaxPosX = 0, nMaxPosY = 0, nMinPosX = 10000, nMinPosY = 10000;
		for (int k = 0; k < cvInspAreaCir.rows; k++)
		{
			UCHAR* Ptr = cvInspAreaCir.data;
			for (int l = 0; l < cvInspAreaCir.cols; l++)
			{
				if (Ptr[k* cvInspAreaCir.step + l] != 255)
					continue;

				for (int i = 0; i < fDevidedPiePosList[nIdxMinDiff].size(); i++)
				{
					nPosX = fDevidedPiePosList[nIdxMinDiff][i].x;
					nPosY = fDevidedPiePosList[nIdxMinDiff][i].y;

					if (k == nPosY && l == nPosX)
					{
						Ptr[k* cvInspAreaCir.step + l] = 100;
						if (nMaxPosX < l) nMaxPosX = l;
						if (nMaxPosY < k) nMaxPosY = k;
						if (nMinPosX > l) nMinPosX = l;
						if (nMinPosY > k) nMinPosY = k;
					}
					else
						continue;
				}

			}
		}

		cv::Mat cvBWTeachImg(nImgHeight, nImgWidth, CV_8UC1); cvBWTeachImg.setTo(0);
		cvBWTeachImg = cvInspAreaCir.clone();
		if (fDevidedPiePosList[nIdxMinDiff].size() == 0)
		{
			nMaxPosX = 0;
			nMaxPosY = 0;
			nMinPosX = 0;
			nMinPosY = 0;
		}
		rcNGArea->left = nMinPosX;
		rcNGArea->top = nMinPosY;
		rcNGArea->right = nMaxPosX;
		rcNGArea->bottom = nMaxPosY;

		//pUcArrTemp = cvBWTeachImg.data;
		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIdx = y * nImgWidth + x;

				UCHAR ucG = cvBWTeachImg.data[y * nImgWidth + x];
				bool b2DIn = false;
				bool b2DNGArea = false;

				//binarization
				if (ucG > 0 && ucG != 100)
				{
					b2DIn = true;
				}
				else if(ucG > 0 && ucG == 100)
					b2DNGArea = true;

				if (b2DIn)
					pUcArrTemp[nIdx] = 255;
				else if(b2DNGArea)
					pUcArrTemp[nIdx] = 100;
				else 
					pUcArrTemp[nIdx] = 0;

			}
		}

		//함수 종료
		//Center좌표까지 세팅하고 난 후
		if (pUcInputImg != NULL)
		{
			//delete pUcInputImg;
			g_pMManager->pem_delete(pUcInputImg, false);
			pUcInputImg = NULL;
		}

		return nCntBlob;
	}


	//3. 바깥 쪽 원의 3D BW 영상 생성 [max:(Highest H - gap H), min:Teach H]
	//BW_not fillhole & find 2 contour
	UCHAR *pUcImgBinary = NULL;
	double fMinHeight = fMaxHeight;

	if (!bUseSolderHighestVolumeDiff)
		m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeIn, (int)fMinHeight, (int)dHighestH - dGapH, false, pUcImgBinary);
	else
		m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeIn, (int)fMinHeight, (int)dHighestH, false, pUcImgBinary);

	m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pUcImgBinary, NULL, stTieArea);// KIY 2020/04/23 : 폴리곤 통합

	bApplyFillHole = FALSE;
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pUcImgBinary, pUcArrTemp, nImgWidth, nImgHeight, nMinBlobArea, FALSE, bApplyFillHole, nFilter, eSelectBigger);

	cv::Mat cvBlobImg2D(nImgHeight, nImgWidth, CV_8UC1, pUcArrTemp);
	cvBWOuterBlob = cvBlobImg2D.clone();

	cvBWOuterBlob = cvBWOuterBlob | cvBWInnerBlob;

	//3-1. 바깥 쪽 원 BW 영상의 외곽선 추출
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	dMxArea = 0;
	idx = 0;

	contours.clear();
	hierarchy.clear();
	cv::findContours(cvBWOuterBlob, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

	cv::Mat cvBlobContour2D(nImgHeight, nImgWidth, CV_8UC1); cvBlobContour2D.setTo(0);
// 	idx = 0;
// 	for (contours.begin(); idx < contours.size(); idx++)
// 	{
// 		//drawContours(cvBlobContour2D, contours, idx, cv::Scalar(255), cv::FILLED, 8, hierarchy);
// 		drawContours(cvBlobContour2D, contours, idx, cv::Scalar(255), 1, 8, hierarchy);
// 	}

	std::vector<POINTF> m_vCircleContour;
	m_vCircleContour.clear();
	POINTF* poAnchorContour = NULL;
	if (contours.size() > 0)
	{
		for (int cCnt = 0; cCnt < contours.size(); cCnt++)
		{
			if (contours[cCnt].size() < 6 && (!bUseSolderHighestVolumeDiff))
				continue;

			int cnt = contours[cCnt].size();

			for (int i = 0; i < cnt; i++)
			{
				POINTF pt;
				pt.x = contours[cCnt][i].x;
				pt.y = contours[cCnt][i].y;

				m_vCircleContour.emplace_back(pt);
			}

			drawContours(cvBlobContour2D, contours, cCnt, cv::Scalar(255), 1, 8, hierarchy);

			//poAnchorContour = m_vCircleContour[0];
		}
	}

	//4. 안쪽/바깥쪽 두 원의 외곽선 두께 검출 (한 원이 바깥 원 안에 포함되는 경우)
	double m_dTeachAngle = 0;

	double stdGap = 5.0;		//5도 간격으로 평균 거리 측정
	double dMin = nImgHeight;

	double maxDist(0), minDist(dMin);

	//POINTF maxPoints[2], minPoints[2], poTarget = { dCircle_x_outLine, dCircle_y_outLine };
	POINTF poTarget = { poHighestVolCenter.x, poHighestVolCenter.y };

	cv::Mat cvInput(nImgHeight, nImgWidth, CV_8UC1);
	cvInput.setTo(0);

	cvInput = cvBlobContour2D_outLine | cvBlobContour2D;

	std::vector<POINT> m_vDistancePosList;
	std::vector<int> m_vDistancePosDirList;
	m_vDistancePosDirList.clear();
	m_vDistancePosList.clear();

	//안쪽 원을 가지고 직선의 방정식 구하기
	for (int i = 0; i < m_vCircleContour_outLine.size(); i++)
	{
		POINTF p1 = { 0,0 };
		p1.x = m_vCircleContour_outLine[i].x;
		p1.y = m_vCircleContour_outLine[i].y;

		POINTF p2 = poTarget;

		double slope, yIntercept;
		//calculateLineEquation(p1, p2, slope, yIntercept);
		slope = (p2.y - p1.y) / (p2.x - p1.x);

		// Calculate the y-intercept
		yIntercept = p1.y - slope * p1.x;

		//if (m_vCircleContour_outLine.size() < m_vCircleContour.size())
		{
			//직선의 방정식을 만족하는 점 찾기 y=slope *x + yIntercept
			for (int j = 0; j < m_vCircleContour.size(); j++)
			{
				double yAxisVal = (m_vCircleContour[j].x * slope) + yIntercept;

				if ((int)round(yAxisVal) == (int)m_vCircleContour[j].y || int(yAxisVal) == (int)m_vCircleContour[j].y)
				{
					POINT poPositionIdx = { 0,0 };
					poPositionIdx.x = i;
					poPositionIdx.y = j;

					if (poTarget.y - m_vCircleContour_outLine[i].y > 0)
					{
						if (poTarget.y - m_vCircleContour[j].y > 0)
						{
							cv::Mat cvRstLine(nImgHeight, nImgWidth, CV_8UC1);
							cvRstLine.setTo(0); //= cvInput.clone();

							cv::Point cvPoMax_st, cvPoMax_ed;
							cvPoMax_st.x = m_vCircleContour[j].x; cvPoMax_st.y = m_vCircleContour[j].y;
							cvPoMax_ed.x = m_vCircleContour_outLine[i].x; cvPoMax_ed.y = m_vCircleContour_outLine[i].y;

							cv::line(cvRstLine, cvPoMax_st, cvPoMax_ed, cv::Scalar(255));

							cv::Mat cvInterSectPointCnt(nImgHeight, nImgWidth, CV_8UC1);
							cvInterSectPointCnt.setTo(0);

							cvInterSectPointCnt = cvRstLine & cvInput;

							//만족하는 점이 2개 이상일 시, 가장 가까운 거리로 선택
							if (cv::countNonZero(cvInterSectPointCnt) > 2)
							{
								double dMinCandidates = cvInterSectPointCnt.rows;
								POINT poMinCandi = { 0,0 };
								//안쪽 외곽선 좌표로부터 만족하는 점들간 거리 계산
								//m_vCircleContour_outLine[i].y
								for (int k = 0; k < cvInterSectPointCnt.rows; k++)
								{
									UCHAR* Ptr = cvInterSectPointCnt.data;
									for (int l = 0; l < cvInterSectPointCnt.cols; l++)
									{
										if (Ptr[k*cvInterSectPointCnt.step + l] == 255 &&
											k != m_vCircleContour_outLine[i].y && l != m_vCircleContour_outLine[i].x)
										{
											//거리 계산
											double dPox = m_vCircleContour_outLine[i].x - l;
											double dPoy = m_vCircleContour_outLine[i].y - k;
											double dDistance = std::sqrt(dPox * dPox + dPoy * dPoy);

											if (dMinCandidates > dDistance)
											{
												dMinCandidates = dDistance;
												poMinCandi.x = l;
												poMinCandi.y = k;
											}

										}
									}
								}

								if (dMinCandidates != cvInterSectPointCnt.rows && poMinCandi.x != 0 && poMinCandi.y != 0)
								{
									for (int m = 0; m < m_vCircleContour.size(); m++)
									{
										if (m_vCircleContour[m].x == poMinCandi.x && m_vCircleContour[m].y == poMinCandi.y && poTarget.y - m_vCircleContour[m].y > 0)
										{
											poPositionIdx.x = i;
											poPositionIdx.y = m;

											m_vDistancePosList.emplace_back(poPositionIdx);
											m_vDistancePosDirList.emplace_back(1);
										}

									}
								}
								

							}
							else
							{
								m_vDistancePosList.emplace_back(poPositionIdx);
								m_vDistancePosDirList.emplace_back(1);
							}
						}
					}
					else
					{
						if (poTarget.y - m_vCircleContour[j].y < 0)
						{
							cv::Mat cvRstLine(nImgHeight, nImgWidth, CV_8UC1);
							cvRstLine.setTo(0);

							cv::Point cvPoMax_st, cvPoMax_ed;
							cvPoMax_st.x = m_vCircleContour[j].x; cvPoMax_st.y = m_vCircleContour[j].y;
							cvPoMax_ed.x = m_vCircleContour_outLine[i].x; cvPoMax_ed.y = m_vCircleContour_outLine[i].y;

							cv::line(cvRstLine, cvPoMax_st, cvPoMax_ed, cv::Scalar(255));

							cv::Mat cvInterSectPointCnt(nImgHeight, nImgWidth, CV_8UC1);
							cvInterSectPointCnt.setTo(0);

							cvInterSectPointCnt = cvRstLine & cvInput;

							if (cv::countNonZero(cvInterSectPointCnt) > 2)
							{
								double dMinCandidates = cvInterSectPointCnt.rows;
								POINT poMinCandi = { 0,0 };
								//안쪽 외곽선 좌표로부터 만족하는 점들간 거리 계산
								//m_vCircleContour_outLine[i].y
								for (int k = 0; k < cvInterSectPointCnt.rows; k++)
								{
									UCHAR* Ptr = cvInterSectPointCnt.data;
									for (int l = 0; l < cvInterSectPointCnt.cols; l++)
									{
										if (Ptr[k*cvInterSectPointCnt.step + l] == 255 &&
											k != m_vCircleContour_outLine[i].y && l != m_vCircleContour_outLine[i].x)
										{
											//거리 계산
											double dPox = m_vCircleContour_outLine[i].x - l;
											double dPoy = m_vCircleContour_outLine[i].y - k;
											double dDistance = std::sqrt(dPox * dPox + dPoy * dPoy);

											if (dMinCandidates > dDistance)
											{
												dMinCandidates = dDistance;
												poMinCandi.x = l;
												poMinCandi.y = k;
											}

										}
									}
								}

								if (dMinCandidates != cvInterSectPointCnt.rows && poMinCandi.x != 0 && poMinCandi.y != 0)
								{
									for (int m = 0; m < m_vCircleContour.size(); m++)
									{
										if (m_vCircleContour[m].x == poMinCandi.x && m_vCircleContour[m].y == poMinCandi.y && poTarget.y - m_vCircleContour[m].y < 0)
										{
											poPositionIdx.x = i;
											poPositionIdx.y = m;

											m_vDistancePosList.emplace_back(poPositionIdx);
											m_vDistancePosDirList.emplace_back(2);
										}

									}
								}

								
							}
							else
							{
								m_vDistancePosList.emplace_back(poPositionIdx);
								m_vDistancePosDirList.emplace_back(2);
							}
						}
					}



				}
			}
		}

	}

	double dMinDist = cvInput.rows;
	double dMaxDist = 0;

	int nMaxDistIdx = 0;
	int nMinDistIdx = 0;
	int nPreIdxInnerDistNum = 0;

	cv::Point cvMaxDist_st, cvMaxDist_ed;
	cvMaxDist_st.x = 0, cvMaxDist_st.y = 0, cvMaxDist_ed.x = 0, cvMaxDist_ed.y = 0;
	cv::Point cvMinDist_st, cvMinDist_ed;
	cvMinDist_st.x = 0, cvMinDist_st.y = 0, cvMinDist_ed.x = 0, cvMinDist_ed.y = 0;

	if (m_vDistancePosList.size() > 0)
	{

		for (int j = 0; j < m_vDistancePosList.size(); j++)
		{

			POINT nPosInnerCon;

// 			if (nPreIdxInnerDistNum == m_vDistancePosList[j].x)
// 			{
// 				if (m_vDistancePosDirList[j] == 2)
// 					continue;
// 			}

			nPosInnerCon.x = (int)m_vCircleContour_outLine[m_vDistancePosList[j].x].x;
			nPosInnerCon.y = (int)m_vCircleContour_outLine[m_vDistancePosList[j].x].y;

			POINT nPosOutCon;
			nPosOutCon.x = (int)m_vCircleContour[m_vDistancePosList[j].y].x;
			nPosOutCon.y = (int)m_vCircleContour[m_vDistancePosList[j].y].y;

			double dx = nPosOutCon.x - nPosInnerCon.x;
			double dy = nPosOutCon.y - nPosInnerCon.y;
			double dDistance = std::sqrt(dx * dx + dy * dy);

			if (dMaxDist < dDistance)
			{
				dMaxDist = dDistance;
				nMaxDistIdx = j;
			}

			if (dMinDist > dDistance)
			{
				dMinDist = dDistance;
				nMinDistIdx = j;
			}

			nPreIdxInnerDistNum = m_vDistancePosList[j].x;
		}

		cv::Mat cvRstView(nImgHeight, nImgWidth, CV_8UC1);
		cvRstView = cvInput.clone();

		//cv::Point cvMaxDist_st, cvMaxDist_ed;
		cvMaxDist_st.x = (int)m_vCircleContour_outLine[(int)m_vDistancePosList[nMaxDistIdx].x].x;
		cvMaxDist_st.y = (int)m_vCircleContour_outLine[(int)m_vDistancePosList[nMaxDistIdx].x].y;
		cvMaxDist_ed.x = (int)m_vCircleContour[(int)m_vDistancePosList[nMaxDistIdx].y].x;
		cvMaxDist_ed.y = (int)m_vCircleContour[(int)m_vDistancePosList[nMaxDistIdx].y].y;

		//cv::Point cvMinDist_st, cvMinDist_ed;
		cvMinDist_st.x = (int)m_vCircleContour_outLine[(int)m_vDistancePosList[nMinDistIdx].x].x;
		cvMinDist_st.y = (int)m_vCircleContour_outLine[(int)m_vDistancePosList[nMinDistIdx].x].y;
		cvMinDist_ed.x = (int)m_vCircleContour[(int)m_vDistancePosList[nMinDistIdx].y].x;
		cvMinDist_ed.y = (int)m_vCircleContour[(int)m_vDistancePosList[nMinDistIdx].y].y;

		cv::line(cvRstView, cvMaxDist_st, cvMaxDist_ed, cv::Scalar(255));
		cv::line(cvRstView, cvMinDist_st, cvMinDist_ed, cv::Scalar(255));
	}

	maxPoints[0].x = cvMaxDist_st.x; maxPoints[0].y = cvMaxDist_st.y;
	maxPoints[1].x = cvMaxDist_ed.x; maxPoints[1].y = cvMaxDist_ed.y;

	minPoints[0].x = cvMinDist_st.x; minPoints[0].y = cvMinDist_st.y;
	minPoints[1].x = cvMinDist_ed.x; minPoints[1].y = cvMinDist_ed.y;

	double dx = cvMinDist_ed.x - cvMinDist_st.x;
	double dy = cvMinDist_ed.y - cvMinDist_st.y;
	minDist = std::sqrt(dx * dx + dy * dy);

	dx = cvMaxDist_ed.x - cvMaxDist_st.x;
	dy = cvMaxDist_ed.y - cvMaxDist_st.y;
	maxDist = std::sqrt(dx * dx + dy * dy);


	/*
		//원의 특징을 활용해 두께 구하기
		// 	GetThetaAreaDist(cvInput, stdGap, dRadius_outLine, dRadius_1, m_vCircleContour_outLine.size(), m_vCircleContour.size(),
		// 		poTarget, poTarget, m_vCircleContour_outLine, m_vCircleContour, maxPoints, minPoints, maxDist, minDist);

		GetThetaAreaDist(cvInput, stdGap, dRadius_1, dRadius_outLine, m_vCircleContour.size(), m_vCircleContour_outLine.size(),
			poTarget, poTarget, m_vCircleContour, m_vCircleContour_outLine, maxPoints, minPoints, maxDist, minDist);

		cv::Mat cvRstLine(nImgHeight, nImgWidth, CV_8UC1);
		cvRstLine = cvInput.clone();

		cv::Point cvPoMaxDiff_st, cvPoMaxDiff_ed;
		if (maxDist == 0)
		{
			cvPoMaxDiff_st.x = 0; cvPoMaxDiff_st.y = 0;
			cvPoMaxDiff_ed.x = 0; cvPoMaxDiff_ed.y = 0;
		}
		else
		{
			cvPoMaxDiff_st.x = maxPoints[0].x; cvPoMaxDiff_st.y = maxPoints[0].y;
			cvPoMaxDiff_ed.x = maxPoints[1].x; cvPoMaxDiff_ed.y = maxPoints[1].y;
		}

		cv::Point cvPoMinDiff_st, cvPoMinDiff_ed;
		if (minDist == 0)
		{
			cvPoMinDiff_st.x = 0; cvPoMinDiff_st.y = 0;
			cvPoMinDiff_ed.x = 0; cvPoMinDiff_ed.y = 0;
		}
		else
		{
			cvPoMinDiff_st.x = minPoints[0].x; cvPoMinDiff_st.y = minPoints[0].y;
			cvPoMinDiff_ed.x = minPoints[1].x; cvPoMinDiff_ed.y = minPoints[1].y;
		}

		cv::line(cvRstLine, cvPoMaxDiff_st, cvPoMaxDiff_ed, cv::Scalar(255));
		cv::line(cvRstLine, cvPoMinDiff_st, cvPoMinDiff_ed, cv::Scalar(255));

		*/

	if (minDist == dMin || maxDist == 0)
	{
		//둘다 검출 안되는 경우에는 결과를 0으로
		if (minDist == dMin && maxDist == 0)
		{
			dMinDistance = 0.0;
			dMaxDistance = 0.0;
		}
		else
		{
			//둘 중 하나만 검출 안되면 하나 결과만 0으로
			if (minDist == dMin)
			{
				dMinDistance = 0.0;
			}
			else
			{
				if (cvMinDist_st.x > nImgWidth || cvMinDist_st.y > nImgHeight ||
					cvMinDist_ed.x > nImgWidth || cvMinDist_ed.y > nImgHeight)
				{
					dMinDistance = 0.0;
				}
			}

			if (maxDist == 0)
			{
				dMaxDistance = 0.0;
			}
			else
			{
				if (cvMaxDist_st.x > nImgWidth || cvMaxDist_st.y > nImgHeight ||
					cvMaxDist_ed.x > nImgWidth || cvMaxDist_ed.y > nImgHeight)
				{
					dMaxDistance = 0.0;
				}
			}
		}
	}
	else
	{
		dMinDistance = minDist;
		dMaxDistance = maxDist;

	}


	//티칭창 2D BW 이미지 세팅용
// 	fMinHeight = fMaxHeight;
// 	m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, eTypeRangeIn, (int)fMinHeight, (int)dHighestH - dGapH, false, pUcImgBinary);
// 	m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, pUcImgBinary, NULL, stTieArea);
// 	bApplyFillHole = FALSE;
// 	int nCntB = m_pProcMilAlgo->CalcBlob_Select(pUcImgBinary, pUcArrTemp, nImgWidth, nImgHeight, nMinBlobArea, FALSE, bApplyFillHole, nFilter, eSelectBigger);

	cv::Mat cvBWTeachImg(nImgHeight, nImgWidth, CV_8UC1); cvBWTeachImg.setTo(0);
	cvBWTeachImg = cvBWOuterBlob - cvBWInnerBlob;

	//pUcArrTemp = cvBWTeachImg.data;
	for (int y = 0; y < nImgHeight; y++)
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			int nIdx = y * nImgWidth + x;

			UCHAR ucG = cvBWTeachImg.data[y * nImgWidth + x];
			bool b2DIn = false;

			//binarization
			if (ucG > 0)
			{
				b2DIn = true;
			}

			if (b2DIn)
				pUcArrTemp[nIdx] = 255;
			else
				pUcArrTemp[nIdx] = 0;

		}
	}

	if (pUcImgBinary != NULL)
	{
		//delete pUcImgBinary;
		g_pMManager->pem_delete(pUcImgBinary, false);
		pUcImgBinary = NULL;
	}

	if (pUcInputImg != NULL)
	{
		//delete pUcInputImg;
		g_pMManager->pem_delete(pUcInputImg, false);
		pUcInputImg = NULL;
	}

	return nCntBlob;
}

void CPInsp_AlgoSolderCone::CircleFitting(std::vector<cv::Point> vPoList, double *centX, double *centY, double *radius, double *err)
{
	double theta;
	double s_cos = 0, s_sin = 0;
	double xy = 0, x = 0, y = 0;

	double err2 = 0;

	double ds_Xn2 = 0, ds_Yn2 = 0, ds_XnYn = 0, ds_Xn = 0, ds_Yn = 0;
	double dsub1 = 0, dsub2 = 0, dsub3 = 0;
	for (int i = 0; i < vPoList.size(); i++)
	{
		double x, y;
		x = vPoList[i].x;
		y = vPoList[i].y;

		ds_Xn2 += x * x;
		ds_Yn2 += y * y;
		ds_XnYn += x * y;
		ds_Xn += x;
		ds_Yn += y;

		dsub1 += x * (x*x + y * y);
		dsub2 += y * (x*x + y * y);
		dsub3 += x * x + y * y;
	}


	int matrixSize = 3;
	int matrixSize2 = matrixSize * matrixSize;

	double org_m[9];
	double sub_m[9];

	org_m[0] = ds_Xn2; org_m[1] = ds_XnYn; org_m[2] = ds_Xn;
	org_m[3] = ds_XnYn; org_m[4] = ds_Yn2; org_m[5] = ds_Yn;
	org_m[6] = ds_Xn; org_m[7] = ds_Yn; org_m[8] = vPoList.size();

	sub_m[0] = dsub1; sub_m[1] = dsub2; sub_m[2] = dsub3;

	//IppStatus ans=ippmInvert_ma_64f(org_m, (int)36*2, (int)12*2, (int)4*2, pBuff, inv_m,
		//(int)36*2, (int)12*2, (int)4*2,(int)3, (int)1);

	//ans=ippmMul_mav_64f(inv_m, 36*2, 12*2, 4*2, 3,3, sub_m, 4*2, 3, org_m, 12*2,4*2,1);

	/**centX = (float)org_m[0]/2;
	*centY = (float)org_m[1]/2;
	*radius = (float)((*centX**centX) + (*centY**centY) + org_m[2]);
	*radius = sqrt(*radius);*/

	cv::Mat org(3, 3, CV_64FC1, org_m);
	cv::Mat sub(3, 1, CV_64FC1, sub_m);
	cv::Mat dst = org.inv() * sub;

	double cX = (float)dst.at<double>(0, 0) / 2;
	double cY = (float)dst.at<double>(1, 0) / 2;
	double rad = sqrt(float((cX*cX) + (cY*cY) + dst.at<double>(2, 0)));
	if (rad < 0) rad = -rad;

	for (int i = 0; i < vPoList.size(); i++)
	{
		CPoint pt(vPoList[i].x, vPoList[i].y);

		float fDistance = sqrtf(std::powf(pt.y - cY, 2) + std::powf(pt.x - cX, 2));
		float fDiff = abs(rad - fDistance);
		float fErr = fDiff / rad;

		if (fErr > 0.15)
			err2 += 1;
	}

	if (centX)	*centX = cX;
	if (centY)	*centY = cY;
	if (radius)	*radius = rad;
	if (err)	*err = err2 / vPoList.size();
}

void CPInsp_AlgoSolderCone::GetThetaAreaDist(cv::Mat cvInputImg, double gapDegree, double andRadi, double tarRadi, int ancSize, int tarSize, POINTF Acenter, POINTF Tcenter, std::vector<POINTF> AnchorCont, std::vector<POINTF> TargetCont, POINTF *maxPoints, POINTF *minPoints, double &maxDist, double &minDist)
{
	if (gapDegree <= 0)
		return;

	int regionCnt = int(360.0 / gapDegree + 0.5);
	double gapTheta = gapDegree * PI / 180.0;

	double dist(0);
	int targetSize = tarSize;
	int anchorSize = ancSize;

	//POINTF *targetContour = TargetCont;
	//POINTF *anchorContour = AnchorCont;

	double targetR = tarRadi;
	double anchorR = andRadi;
	POINTF vecBetweenCenters = { Tcenter.x - Acenter.x, Tcenter.y - Acenter.y };

	double default_dValue = 0;
	int default_IValue = 0;
	std::vector<double> vTargetDist(regionCnt, default_dValue), vAnchorDist(regionCnt, default_dValue);
	std::vector<int> vTargetDistCnt(regionCnt, default_IValue), vAnchorDistCnt(regionCnt, default_IValue);

	//if (bUseCircleContour)
	{
		double targetDist(0), targetCnt(0);
		int targetHalfLen = 0;
		//targetHalfLen = int(targetR * gapTheta / 2.0);
		targetHalfLen = int((targetR / ((1000.0 * m_resolX) * gapTheta)) / 2.0);

		for (int i = 0; i < targetSize; i++)
		{
			double xDiff = TargetCont[i].x - Acenter.x;
			double yDiff = TargetCont[i].y - Acenter.y;

			double theta = atan2(yDiff, xDiff);
			if (theta < 0)
				theta += CV_2PI;

			int region = int(theta / gapTheta);

			vTargetDist[region] += sqrt(xDiff * xDiff + yDiff * yDiff);
			vTargetDistCnt[region]++;
		}

		double anchorDist(0), anchorCnt(0);
		int anchorHalfLen = 0;
		//anchorHalfLen = int(anchorR * gapTheta / 2.0);
		anchorHalfLen = int((anchorR / ((1000.0 * m_resolX) * gapTheta)) / 2.0);

		for (int i = 0; i < anchorSize; i++)
		{
			double xDiff = AnchorCont[i].x - Acenter.x;
			double yDiff = AnchorCont[i].y - Acenter.y;

			double theta = atan2(yDiff, xDiff);
			if (theta < 0)
				theta += CV_2PI;

			int region = int(theta / gapTheta);

			vAnchorDist[region] += sqrt(xDiff * xDiff + yDiff * yDiff);
			vAnchorDistCnt[region]++;
		}

		for (int i = 0; i < regionCnt; i++)
		{
			if (vTargetDistCnt[i] > targetHalfLen)
			{
				vTargetDist[i] /= vTargetDistCnt[i];
			}
			else
			{
				double stDist = GetDist_ACenterToTCircle(vecBetweenCenters, i*gapTheta, targetR);
				double edDist = GetDist_ACenterToTCircle(vecBetweenCenters, (i + 1)*gapTheta, targetR);

				double tmpDist = (stDist + edDist) / 2.0;
				double len = min(stDist, edDist) * gapTheta / 2;

				for (int a = 0; a < len; a++)
				{
					vTargetDist[i] += tmpDist;
				}
				vTargetDist[i] /= len + vTargetDistCnt[i];
			}

			if (vAnchorDistCnt[i] > anchorHalfLen)
			{
				vAnchorDist[i] /= vAnchorDistCnt[i];
			}
			else
			{
				for (int a = 0; a < anchorHalfLen; a++)
				{
					vAnchorDist[i] += anchorR;
				}
				vAnchorDist[i] /= anchorHalfLen + vAnchorDistCnt[i];
			}
		}

	}
	// 	else 원일때
// 	{
// 		for (int i = 0; i < regionCnt; i++)
// 		{
// 			double stDist = GetDist_ACenterToTCircle(vecBetweenCenters, i*gapTheta, targetR);
// 			double edDist = GetDist_ACenterToTCircle(vecBetweenCenters, (i + 1)*gapTheta, targetR);
// 
// 			vTargetDist[i] = (stDist + edDist) / 2.0;
// 			vAnchorDist[i] += anchorR;
// 		}
// 	}

	bool isAnchorBig = anchorR > targetR;
	minDist = isAnchorBig ? anchorR : targetR;
	maxDist = 0;

	int minRegion(0), maxRegion(0);

	for (int i = 0; i < regionCnt; i++)
	{
		double dist = 0.0;
		//dist = isAnchorBig ? vAnchorDist[i] - vTargetDist[i] : vTargetDist[i] - vAnchorDist[i];
		if (_isnan(vAnchorDist[i]) == true || _isnan(vTargetDist[i]) == true)
		{
			dist = 0;
		}
		else
		{
			if (isAnchorBig)
				dist = vAnchorDist[i] - vTargetDist[i];
			else
				dist = vTargetDist[i] - vAnchorDist[i];
		}

		if (dist < minDist)
		{
			minDist = dist;
			minRegion = i;
		}
		if (dist > maxDist)
		{
			maxDist = dist;
			maxRegion = i;
		}
	}

	maxPoints[0].x = Acenter.x + cos((2 * maxRegion + 1) / 2.0 * gapTheta) * vTargetDist[maxRegion];
	maxPoints[0].y = Acenter.y + sin((2 * maxRegion + 1) / 2.0 * gapTheta) * vTargetDist[maxRegion];
	maxPoints[1].x = Acenter.x + cos((2 * maxRegion + 1) / 2.0 * gapTheta) * vAnchorDist[maxRegion];
	maxPoints[1].y = Acenter.y + sin((2 * maxRegion + 1) / 2.0 * gapTheta) * vAnchorDist[maxRegion];

	minPoints[0].x = Acenter.x + cos((2 * minRegion + 1) / 2.0 * gapTheta) * vTargetDist[minRegion];
	minPoints[0].y = Acenter.y + sin((2 * minRegion + 1) / 2.0 * gapTheta) * vTargetDist[minRegion];
	minPoints[1].x = Acenter.x + cos((2 * minRegion + 1) / 2.0 * gapTheta) * vAnchorDist[minRegion];
	minPoints[1].y = Acenter.y + sin((2 * minRegion + 1) / 2.0 * gapTheta) * vAnchorDist[minRegion];
}

void CPInsp_AlgoSolderCone::GetFiduPos_LSQ(std::vector<cv::Point>&  Points, double *centX, double *centY, double *radius, double *err)
{
	int nBorderCnt = Points.size();

	double theta;
	double s_cos = 0, s_sin = 0;
	double xy = 0, x = 0, y = 0;
	int cnt = nBorderCnt;

	double err2 = 0;

	double ds_Xn2 = 0, ds_Yn2 = 0, ds_XnYn = 0, ds_Xn = 0, ds_Yn = 0;
	double dsub1 = 0, dsub2 = 0, dsub3 = 0;

	//?? ???
	for (int i = 0; i < nBorderCnt; i++)
	{
		double x, y;
		x = Points[i].x;
		y = Points[i].y;

		ds_Xn2 += x * x;
		ds_Yn2 += y * y;
		ds_XnYn += x * y;
		ds_Xn += x;
		ds_Yn += y;

		dsub1 += x * (x*x + y * y);
		dsub2 += y * (x*x + y * y);
		dsub3 += x * x + y * y;
	}

	int matrixSize = 3;
	int matrixSize2 = matrixSize * matrixSize;

	/*double* org_m = (double*)malloc(sizeof(double)*matrixSize2);
	double* inv_m = (double*)malloc(sizeof(double)*matrixSize2);
	double* sub_m = (double*)malloc(sizeof(double)*matrixSize);
	double* pBuff = (double*)malloc(sizeof(double)*matrixSize2);*/
	double* org_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* inv_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* sub_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize, (PCHAR)__FUNCTION__, __LINE__);
	double* pBuff = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);

	org_m[0] = ds_Xn2; org_m[1] = ds_XnYn; org_m[2] = ds_Xn;
	org_m[3] = ds_XnYn; org_m[4] = ds_Yn2; org_m[5] = ds_Yn;
	org_m[6] = ds_Xn; org_m[7] = ds_Yn; org_m[8] = cnt;

	sub_m[0] = dsub1; sub_m[1] = dsub2; sub_m[2] = dsub3;

	//IppStatus ans = ippmInvert_ma_64f(org_m, (int)36 * 2, (int)12 * 2, (int)4 * 2, pBuff, inv_m,
	//	(int)36 * 2, (int)12 * 2, (int)4 * 2, (int)3, (int)1);
	//ans = ippmMul_mav_64f(inv_m, 36 * 2, 12 * 2, 4 * 2, 3, 3, sub_m, 4 * 2, 3, org_m, 12 * 2, 4 * 2, 1);

	//*centX = (float)org_m[0] / 2;
	//*centY = (float)org_m[1] / 2;
	//*radius = (float)((*centX**centX) + (*centY**centY) + org_m[2]);
	//*radius = sqrt(*radius);

	cv::Mat org(3, 3, CV_64FC1, org_m);
	cv::Mat sub(3, 1, CV_64FC1, sub_m);
	cv::Mat dst = org.inv() * sub;

	*centX = (float)dst.at<double>(0, 0) / 2;
	*centY = (float)dst.at<double>(1, 0) / 2;
	*radius = (float)((*centX**centX) + (*centY**centY) + dst.at<double>(2, 0));
	*radius = sqrt(*radius);

	delete[] org_m;
	delete[] inv_m;
	delete[] sub_m;
	delete[] pBuff;

	if (*radius < 0) *radius = -*radius;


	int m_borderCnt = 0;
	for (int i = 0; i < nBorderCnt; i++)
	{
		m_borderCnt++;
		cv::Point pt = Points[i];

		double dist = sqrt(SQR(pt.x - *centX) + SQR(pt.y - *centY));
		double distdeviate = fabs(dist - *radius) / *radius;

		if (distdeviate > 0.07)
			err2++;
	}
	*err = err2 / nBorderCnt;

	return;
}

double CPInsp_AlgoSolderCone::GetDist_ACenterToTCircle(POINTF vecBetweenCenters, double theta, double targetRadius)
{
	double a = sin(theta)*vecBetweenCenters.x - cos(theta)*vecBetweenCenters.y;

	if (a >= targetRadius)
		return targetRadius;
	else
		return sqrt(targetRadius*targetRadius - a * a) + cos(theta)*vecBetweenCenters.x + sin(theta)*vecBetweenCenters.y;
}
