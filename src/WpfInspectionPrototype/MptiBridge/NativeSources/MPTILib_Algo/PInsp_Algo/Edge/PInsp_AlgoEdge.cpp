#include "PInsp_AlgoEdge.h"


CPInsp_AlgoEdge::CPInsp_AlgoEdge(void)
{
}


CPInsp_AlgoEdge::~CPInsp_AlgoEdge(void)
{
}

void CPInsp_AlgoEdge::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoEdge::GetInspAlgoData()
{
	return eSPCAlgoEdge;
}

int CPInsp_AlgoEdge::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoEdge * rstAlgo = (RstAlgoEdge *)vRstInspAlgo;
	if (rstAlgo->m_bMissing == TRUE || !rstAlgo->m_bOKLength)
	{
		eWholeNgTypeTemp[TypeMountMissing] = e_NG;
		nCurrentNgType = TypeMountMissing;
	}

	if (!rstAlgo->m_bOKAngle)
	{
		eWholeNgTypeTemp[TypeMountAngle] = e_NG;
		nCurrentNgType = TypeMountAngle;
	}

	if (!rstAlgo->m_bOKShiftX || !rstAlgo->m_bOKShiftY)
	{
		eWholeNgTypeTemp[TypeMountShift] = e_NG;
		nCurrentNgType = TypeMountShift;
	}

	if (!rstAlgo->m_bDistance || !rstAlgo->m_bDistanceX || !rstAlgo->m_bDistanceY)
	{
		eWholeNgTypeTemp[TypeMountWorng] = e_NG;
		nCurrentNgType = TypeMountWorng;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoEdge::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoEdge::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspEdge(sInspAlgo, *stAlgoParam.m_sPartAlgoImg, sWndAlgoImg, *sInspImageData, (RstAlgoEdge *)sRstAlgo, stTieArea, ucArrDstImg, stAlgoParam.m_nStartX, stAlgoParam.m_nStartY);

	RstAlgoEdge * rst = (RstAlgoEdge *)sRstAlgo;
	if (stAlgoParam.m_bInspection == TRUE)
	{
		rst->m_poDrawCenter.x += stAlgoParam.m_dx;
		rst->m_poDrawCenter.y = stAlgoParam.m_dInspH - (rst->m_poDrawCenter.y + stAlgoParam.m_dy);
		for (int nIdx = 0; nIdx < EdgeLineTotalCnt; nIdx++)
		{
			rst->m_poDrawLine[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine[nIdx].y += stAlgoParam.m_dy;
			rst->m_poDrawLine_Sec[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine_Sec[nIdx].y += stAlgoParam.m_dy;
			rst->m_poDrawLine_T[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine_T[nIdx].y += stAlgoParam.m_dy;
			rst->m_poDrawLine_T2[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine_T2[nIdx].y += stAlgoParam.m_dy;
		}
	}
	else
	{
		rst->m_poDrawCenter.x += stAlgoParam.m_nStartX;
		rst->m_poDrawCenter.y += stAlgoParam.m_nStartY;
	}
	return bResult;
}

void CPInsp_AlgoEdge::FindCenterPos_Line(UCHAR *ucArrWnd, int nWndWidth, int nWndHeight, BOOL bArrIsHorizon[EdgeLineTotalCnt], int nArrSelectDir[EdgeLineTotalCnt], RstAlgoEdge sRstAlgo, POINTF *poRstCenter)
{
	if (ucArrWnd == NULL || nWndWidth <= 0 || nWndHeight <= 0 || nArrSelectDir == NULL || bArrIsHorizon == NULL)
		return;
	if (nArrSelectDir[0] == Center_Line || nArrSelectDir[1] == Center_Line || bArrIsHorizon[0] == bArrIsHorizon[1])
		return;
	int nDiv = 6;
	int nDiv2 = 3;
	int nLim = 10;
	float fDrawX = sRstAlgo.m_poDrawLine[0].x - sRstAlgo.m_poDrawLine_Sec[0].x;
	float fDrawY = sRstAlgo.m_poDrawLine[0].y - sRstAlgo.m_poDrawLine_Sec[0].y;
	float fDrawX_2 = sRstAlgo.m_poDrawLine[1].x - sRstAlgo.m_poDrawLine_Sec[1].x;
	float fDrawY_2 = sRstAlgo.m_poDrawLine[1].y - sRstAlgo.m_poDrawLine_Sec[1].y;
	if (fDrawX < 0) fDrawX *= -1;
	if (fDrawY < 0) fDrawY *= -1;
	if (fDrawX_2 < 0) fDrawX_2 *= -1;
	if (fDrawY_2 < 0) fDrawY_2 *= -1;
	int nFindW = fDrawX;
	int nFindH = fDrawY;
	if (fDrawX_2 > fDrawX) nFindW = fDrawX_2;
	if (fDrawY_2 > fDrawY) nFindH = fDrawY_2;
	nFindW /= nDiv;
	nFindH /= nDiv;
	if (nFindW < nLim) nFindW = nLim;
	if (nFindH < nLim) nFindH = nLim;
	int nStartX = poRstCenter->x - (nFindW / nDiv2 * (nDiv2 - 1));
	int nStartY = poRstCenter->y - (nFindH / nDiv2);
	bool bVL = false;
	bool bHL = true;
	if ((bArrIsHorizon[0] == FALSE && nArrSelectDir[0] == Left_Line) ||
		(bArrIsHorizon[1] == FALSE && nArrSelectDir[1] == Left_Line))
	{
		nStartX = poRstCenter->x - (nFindW / nDiv2);
		bVL = true;
	}
	if ((bArrIsHorizon[0] == TRUE && nArrSelectDir[0] == Right_Line) ||
		(bArrIsHorizon[1] == TRUE && nArrSelectDir[1] == Right_Line))
	{
		nStartY = poRstCenter->y - (nFindH / nDiv2 * (nDiv2 - 1));
		bHL = false;
	}
	if (nStartX < 0) nStartX = 0;
	if (nStartY < 0) nStartY = 0;
	if (nStartX + nFindW > nWndWidth) nFindW = nWndWidth - nStartX;
	if (nStartY + nFindH > nWndHeight) nFindH = nWndHeight - nStartY;
	UCHAR *ucArrFindArea = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrFindArea, nFindW * nFindH);
	memset(ucArrFindArea, 0, sizeof(UCHAR) * nFindW * nFindH);
	m_pProcMilAlgo->GetClipImage_LT(ucArrWnd, nWndWidth, nWndHeight,
		ucArrFindArea, nStartX, nStartY, nFindW, nFindH);

	m_pProcMilAlgo->SaveWorkImg(ucArrWnd, nWndWidth, nWndHeight, _T("Org_Image_FindCT.bmp"));
	m_pProcMilAlgo->SaveWorkImg(ucArrFindArea, nFindW, nFindH, _T("Dst_Image_FindCT.bmp"));

	int nMin = nWndWidth * nWndHeight;
	int nMax = -1;
	for (int y = 0; y < nFindH; y++)
	{
		if (bVL == true)
		{
			for (int x = 0; x < nFindW; x++)
			{
				int nIndex = x + (y * nFindW);
				if (ucArrFindArea[nIndex] == 255)
				{
					if (nMin > x) nMin = x;
					if (nMax < x) nMax = x;
					break;
				}
			}
		}
		else
		{
			for (int x = nFindW - 1; x > 0; x--)
			{
				int nIndex = x + (y * nFindW);
				if (ucArrFindArea[nIndex] == 255)
				{
					if (nMin > x) nMin = x;
					if (nMax < x) nMax = x;
					break;
				}
			}
		}
	}
	if (nMin == nWndWidth * nWndHeight && nMax == -1)
	{
		Delete_1DArray(&ucArrFindArea);
		return;
	}
	int nCenterPosX = (nMax + nMin) / 2;
	nMin = nWndWidth * nWndHeight;
	nMax = -1;
	if (bHL == true)
	{
		for (int y = 0; y < nFindH; y++)
		{
			int nIndex = nCenterPosX + (y * nFindW);
			if (ucArrFindArea[nIndex] == 255)
			{
				if (nMin > y) nMin = y;
				if (nMax < y) nMax = y;
				break;
			}
		}
	}
	else
	{
		for (int y = nFindH - 1; y > 0; y--)
		{
			int nIndex = nCenterPosX + (y * nFindW);
			if (ucArrFindArea[nIndex] == 255)
			{
				if (nMin > y) nMin = y;
				if (nMax < y) nMax = y;
				break;
			}
		}
	}
	if (nMin == nWndWidth * nWndHeight && nMax == -1)
	{
		Delete_1DArray(&ucArrFindArea);
		return;
	}
	int nCenterPosY = (nMax + nMin) / 2;
	poRstCenter->x = nCenterPosX + nStartX;
	poRstCenter->y = nCenterPosY + nStartY;
	Delete_1DArray(&ucArrFindArea);
}
BOOL CPInsp_AlgoEdge::InspEdge(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipImg, InspRoiImgBuf &sInspImageData, RstAlgoEdge * sRstAlgo, TotalInspExceptArea stTieArea, UCHAR *ucArrDstImg, int nStartX, int nStartY)
{
	BOOL bReturn = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoEdge));
		sRstAlgo->Init();
	}

	if (sInspAlgo.m_eAlgoType != eAlgoEdge)
		return bReturn;
	if (!m_pProcMilAlgo)
		return bReturn;

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL && sWndAlgoImg.m_nLight_index == (int)eMSCN_COAXIAL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bReturn;
	double dWndAngle = sWndAlgoImg.dAngle;
	UCHAR *pucWndSrc = sWndClipImg.m_ucArr2D;
	float *pfWndSrc = sWndClipImg.m_fArr3D;
	int nWndWidth = sWndClipImg.m_nWidth;
	int nWndHeight = sWndClipImg.m_nHeight;
	if ((pucWndSrc == NULL) || (pfWndSrc == NULL && sWndClipImg.m_nLight_index == (int)eMSCN_COAXIAL) || (nWndWidth <= 0) || (nWndHeight <= 0))
		return bReturn;
	if (ucArrDstImg == NULL)
	{
		nImgWidth = sWndAlgoImg.m_nWidth3D;
		nImgHeight = sWndAlgoImg.m_nHeight3D;
	}
	AlgoEdge *pInspAlgoEdge = (AlgoEdge *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoEdge)
		return bReturn;
	AlgoBlob algoBlob = m_pCPInsp_Algo->SetAlgoBlob(sInspAlgo);
	double dCX = 0;
	double dCY = 0;
	double dArea = 0;
	int nMinBlobArea = 4;
	CRect rcBlob(0, 0, 0, 0);
	bool bIsDelete = false;
	int nCntBlob = 0;
	bool bShiftXUse = pInspAlgoEdge->m_bShiftXUse;
	bool bShiftYUse = pInspAlgoEdge->m_bShiftYUse;
	double dShiftX = pInspAlgoEdge->m_dShiftX;
	double dShiftY = pInspAlgoEdge->m_dShiftY;
	double dTechCenterX = pInspAlgoEdge->m_dTechCenterX;
	double dTechCenterY = pInspAlgoEdge->m_dTechCenterY;

	BOOL bArrIsHorizon[EdgeLineTotalCnt];
	POINTF poArrSetTeachCenter[EdgeLineTotalCnt];
	int nArrSelectDir[EdgeLineTotalCnt];
	int nLineTotalCnt = pInspAlgoEdge->m_nSetLineCnt;
	if (dWndAngle == 90 || dWndAngle == 270)
	{
		dTechCenterX = pInspAlgoEdge->m_dTechCenterY;
		dTechCenterY = pInspAlgoEdge->m_dTechCenterX;
		bShiftXUse = pInspAlgoEdge->m_bShiftYUse;
		bShiftYUse = pInspAlgoEdge->m_bShiftXUse;
		dShiftX = pInspAlgoEdge->m_dShiftY;
		dShiftY = pInspAlgoEdge->m_dShiftX;
	}
	for (int n = 0; n < nLineTotalCnt; n++)
	{
		if (dWndAngle == 90 || dWndAngle == 270)
			bArrIsHorizon[n] = !pInspAlgoEdge->m_bArrIsHorizon[n];
		else
			bArrIsHorizon[n] = pInspAlgoEdge->m_bArrIsHorizon[n];
		bool bAnyAngle = m_pCPInsp_Algo->IsAnyAngle(dWndAngle);
		if (pInspAlgoEdge->m_nArrMeasureDirection[n] == 0 || dWndAngle == 0 || bAnyAngle == true)
			nArrSelectDir[n] = pInspAlgoEdge->m_nArrMeasureDirection[n];
		else
		{
			if (dWndAngle == 90 || dWndAngle == 180)
			{
				if (bArrIsHorizon[n] == true || dWndAngle == 180)
				{
					if (pInspAlgoEdge->m_nArrMeasureDirection[n] == 1)
						nArrSelectDir[n] = 2;
					else if (pInspAlgoEdge->m_nArrMeasureDirection[n] == 2)
						nArrSelectDir[n] = 1;
				}
				else
					nArrSelectDir[n] = pInspAlgoEdge->m_nArrMeasureDirection[n];
			}
			if (dWndAngle == 270 || dWndAngle == 180)
			{
				if (bArrIsHorizon[n] == false || dWndAngle == 180)
				{
					if (pInspAlgoEdge->m_nArrMeasureDirection[n] == 1)
						nArrSelectDir[n] = 2;
					else if (pInspAlgoEdge->m_nArrMeasureDirection[n] == 2)
						nArrSelectDir[n] = 1;
				}
				else
					nArrSelectDir[n] = pInspAlgoEdge->m_nArrMeasureDirection[n];
			}
		}
		if (m_pCPInsp_Algo->AnglePosChange(dWndAngle, nWndWidth, nWndHeight, pInspAlgoEdge->m_poArrSetTeachCenter[n], &poArrSetTeachCenter[n]) == FALSE)
			return bReturn;
		if (sRstAlgo)
		{
			sRstAlgo->m_poDrawLine_T[n].x = poArrSetTeachCenter[n].x;
			sRstAlgo->m_poDrawLine_T[n].y = 0;
			sRstAlgo->m_poDrawLine_T2[n].x = poArrSetTeachCenter[n].x;
			sRstAlgo->m_poDrawLine_T2[n].y = nWndHeight;
			if (bArrIsHorizon[n] == true)
			{
				sRstAlgo->m_poDrawLine_T[n].x = 0;
				sRstAlgo->m_poDrawLine_T[n].y = poArrSetTeachCenter[n].y;
				sRstAlgo->m_poDrawLine_T2[n].x = nWndWidth;
				sRstAlgo->m_poDrawLine_T2[n].y = poArrSetTeachCenter[n].y;
			}
		}
	}

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

	UCHAR *ucColorImgDst = NULL;
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (pColorTeach != NULL && pInspAlgoEdge->m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		bool bTeach = ucArrDstImg != NULL ? true : false;
		pColorTeach->GetColorBaseBin(&pInspAlgoEdge->m_sAlgoColorBase, sInspImageData, ucColorImgDst, bTeach, 0, 0, -1, sWndAlgoImg.m_nLight_index);
		m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImageData.nImageSizeX, sInspImageData.nImageSizeY, _T("OrgColor_Image.bmp"));
	}
	if (ucArrDstImg != NULL)
	{
		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;

		// 전체 이미지는 마스킹을 하지않는다.
		nCntBlob = m_pCPInsp_Algo->BlobImageStruct(algoBlob, pucImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucArrDstImg, stTieAreaNULL, algoBlob.m_bFillHole);
		m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("Edge_All.bmp"));
		Delete_1DArray(&ucColorImgDst);
		if (pColorTeach != NULL && pInspAlgoEdge->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			UCHAR *ucColorImgBuf = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgBuf, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, nWndWidth * nWndHeight);
			memset(ucColorImgBuf, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			memset(ucColorImgDst, 0, sizeof(UCHAR) * nWndWidth * nWndHeight);
			bool bTeach = ucArrDstImg != NULL ? true : false;
			pColorTeach->GetColorBaseBin(&pInspAlgoEdge->m_sAlgoColorBase, sInspImageData, ucColorImgBuf, bTeach, 0, 0, -1, sWndAlgoImg.m_nLight_index);
			m_pProcMilAlgo->SaveWorkImg(ucColorImgBuf, sInspImageData.nImageSizeX, sInspImageData.nImageSizeY, _T("OrgColor_Image.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(ucColorImgBuf, sInspImageData.nImageSizeX, sInspImageData.nImageSizeY,
				ucColorImgDst, nStartX, nStartY, nWndWidth, nWndHeight);
			m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nWndWidth, nWndHeight, _T("OrgColor_WndImage.bmp"));
			Delete_1DArray(&ucColorImgBuf);
		}
	}

	UCHAR *ucArrDstWnd = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrDstWnd, (nWndWidth * nWndHeight) * sizeof(UCHAR));
	nCntBlob = m_pCPInsp_Algo->BlobImageStruct(algoBlob, pucWndSrc, pfWndSrc, ucColorImgDst, nWndWidth, nWndHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucArrDstWnd, stTieArea, algoBlob.m_bFillHole);
	m_pProcMilAlgo->SaveWorkImg(ucArrDstWnd, nWndWidth, nWndHeight, _T("Edge_Blob.bmp"));
	Delete_1DArray(&ucColorImgDst);
	if (ucArrDstImg != NULL)
	{
		memset(ucArrDstImg, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		for (int y = nStartY; y < nStartY + nWndHeight; y++)
		{
			for (int x = nStartX; x < nStartX + nWndWidth; x++)
			{
				int nIndex = ((y - nStartY) * nWndWidth) + (x - nStartX);
				int nPos = (y * nImgWidth) + x;
				if (nIndex < 0 || nIndex >= nWndWidth * nWndHeight)
					continue;
				if (nPos < 0 || nPos >= nImgWidth * nImgHeight)
					continue;

				ucArrDstImg[nPos] = ucArrDstWnd[nIndex];
			}
		}
	}
	if (nCntBlob == 0 || (nLineTotalCnt <= 0 || nLineTotalCnt > EdgeLineTotalCnt))
	{
		Delete_1DArray(&ucArrDstWnd);
		sRstAlgo->m_bMissing = TRUE;
		return bReturn;
	}

	double dArrAngle[EdgeLineTotalCnt];
	double dRstAngle = 0.0;
	int nLineLength;
	POINTF ArrDrawLine[EdgeLineTotalCnt][2];
	float fArrInspCenterX[EdgeLineTotalCnt];
	float fArrInspCenterY[EdgeLineTotalCnt];
	double dArrLineDAValue[EdgeLineTotalCnt];
	double dArrLineDBValue[EdgeLineTotalCnt];
	memset(dArrAngle, 0, sizeof(double) * EdgeLineTotalCnt);
	memset(ArrDrawLine, 0, sizeof(POINTF) * EdgeLineTotalCnt * 2);
	memset(fArrInspCenterX, 0, sizeof(float) * EdgeLineTotalCnt);
	memset(fArrInspCenterY, 0, sizeof(float) * EdgeLineTotalCnt);
	memset(dArrLineDAValue, 0, sizeof(double) * EdgeLineTotalCnt);
	memset(dArrLineDBValue, 0, sizeof(double) * EdgeLineTotalCnt);

	float fInspCenterX = 0.0f;
	float fInspCenterY = 0.0f;
	bool bGroup = pInspAlgoEdge->m_bGroup;
	double dRstDistance = 0.0;
	for (int n = 0; n < nLineTotalCnt; n++)
	{
		int nMeasureDirection = nArrSelectDir[n];
		bool bIsHorizon = bArrIsHorizon[n];
		dArrAngle[n] = 0.0;
		nLineLength = 0;
		bReturn = m_pCPInsp_Algo->InspectionLine(nWndWidth, nWndHeight, ucArrDstWnd,
			bIsHorizon, nMeasureDirection, &dArrAngle[n], ArrDrawLine[n], &nLineLength,
			&dArrLineDAValue[n], &dArrLineDBValue[n],
			pInspAlgoEdge->m_nLineFindType, pInspAlgoEdge->m_dLineFindRate, pInspAlgoEdge->m_nInspOption);
		double dAngleBuf = dArrAngle[n];
		if (dAngleBuf < 0)
			dAngleBuf = dAngleBuf * -1;
		if (dAngleBuf > dRstAngle)
			dRstAngle = dArrAngle[n];
		if (sRstAlgo)
		{
			sRstAlgo->m_poDrawLine[n] = ArrDrawLine[n][0];
			sRstAlgo->m_poDrawLine_Sec[n] = ArrDrawLine[n][1];
			fArrInspCenterX[n] = (sRstAlgo->m_poDrawLine[n].x + sRstAlgo->m_poDrawLine_Sec[n].x) / 2.0f;
			fArrInspCenterY[n] = (sRstAlgo->m_poDrawLine[n].y + sRstAlgo->m_poDrawLine_Sec[n].y) / 2.0f;
			fInspCenterX += fArrInspCenterX[n];
			fInspCenterY += fArrInspCenterY[n];
			if (bGroup == false)
			{
				float dArrTechCenterX = poArrSetTeachCenter[n].x;
				float dArrTechCenterY = poArrSetTeachCenter[n].y;
				if (pInspAlgoEdge->m_nArrSetInspCondition[0] == n + 1)	//ShiftX
				{
					sRstAlgo->m_dRstShiftX = (dArrTechCenterX - fArrInspCenterX[n]) * m_resolX;
					sRstAlgo->m_poDrawCenter.x = fArrInspCenterX[n];
				}
				if (pInspAlgoEdge->m_nArrSetInspCondition[1] == n + 1)	//ShiftY
				{
					sRstAlgo->m_dRstShiftY = (dArrTechCenterY - fArrInspCenterY[n]) * m_resolX;
					sRstAlgo->m_poDrawCenter.y = fArrInspCenterY[n];
				}
				if (pInspAlgoEdge->m_nArrSetInspCondition[2] == n + 1)	//Angle
				{
					sRstAlgo->m_dRstAngle = dArrAngle[n];
					sRstAlgo->m_dRstRealAngle = dArrAngle[n];
				}
			}
			if (pInspAlgoEdge->m_bTeachLengthUse == TRUE)
			{
				sRstAlgo->m_bArrOKLength[n] = TRUE;
				if (bIsHorizon == FALSE)
					sRstAlgo->m_dRstLength[n] = nLineLength * m_resolY;
				else
					sRstAlgo->m_dRstLength[n] = nLineLength * m_resolX;
				double dLengthMin = pInspAlgoEdge->m_dArrTeachLength[n] * (pInspAlgoEdge->m_dTeachLengthRateMin / 100.0);
				double dLengthMax = pInspAlgoEdge->m_dArrTeachLength[n] * (pInspAlgoEdge->m_dTeachLengthRateMax / 100.0);
				if (sRstAlgo->m_dRstLength[n] < dLengthMin || sRstAlgo->m_dRstLength[n] > dLengthMax)
				{
					bReturn = FALSE;
					sRstAlgo->m_bArrOKLength[n] = bReturn;
				}
			}
			else
			{
				if (ucArrDstImg != NULL)
					sRstAlgo->m_dRstLength[n] = nLineLength * m_resolY;
				else
					sRstAlgo->m_dRstLength[n] = 0;
			}
		}
	}

	if (sRstAlgo)
	{
		bReturn = TRUE;
		sRstAlgo->m_bMissing = FALSE;
		memset(sRstAlgo->m_poDrawLine_T3, 0, sizeof(POINTF) * 2);
		if (pInspAlgoEdge->UseData(m_eEdgeData_AngleBetweenLines))
		{
			if (nLineTotalCnt == 2)
			{
				POINTF poA1 = sRstAlgo->m_poDrawLine[0];
				POINTF poA2 = sRstAlgo->m_poDrawLine_Sec[0];
				POINTF poB1 = sRstAlgo->m_poDrawLine[1];
				POINTF poB2 = sRstAlgo->m_poDrawLine_Sec[1];
				POINTF poC = m_pCPInsp_Algo->CPInsp::GetIntersection(poA1, poA2, poB1, poB2);
				if (poC.x >= 0 && poC.y >= 0)
					dRstAngle = m_pCPInsp_Algo->CPInsp::GetAngleBetweenLines(poA1, poB1, poC);
			}
		}
		if (bGroup == true)
		{
			fInspCenterX /= nLineTotalCnt;
			fInspCenterY /= nLineTotalCnt;

			if (nLineTotalCnt == 2)
			{
				if (bArrIsHorizon[0] != bArrIsHorizon[1])
				{
					POINTF poCrossLinePoint = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrLineDAValue, dArrLineDBValue, bArrIsHorizon);
					if (pInspAlgoEdge->m_bFindCenter == TRUE)
						FindCenterPos_Line(ucArrDstWnd, nWndWidth, nWndHeight, bArrIsHorizon, nArrSelectDir, *sRstAlgo, &poCrossLinePoint);
					fInspCenterX = poCrossLinePoint.x;
					fInspCenterY = poCrossLinePoint.y;
					if (bArrIsHorizon[0] == true)
					{
						dTechCenterX = poArrSetTeachCenter[1].x;
						dTechCenterY = poArrSetTeachCenter[0].y;
					}
					else
					{
						dTechCenterX = poArrSetTeachCenter[0].x;
						dTechCenterY = poArrSetTeachCenter[1].y;
					}
				}
			}
			else if (nLineTotalCnt == 4 && pInspAlgoEdge->m_bUseCrossCenter)
			{
				double dArrA[2];
				double dArrB[2];
				BOOL bArrH[2];
				int nArrS[2];
				dArrA[0] = dArrLineDAValue[0]; dArrA[1] = dArrLineDAValue[1];
				dArrB[0] = dArrLineDBValue[0]; dArrB[1] = dArrLineDBValue[1];
				bArrH[0] = bArrIsHorizon[0]; bArrH[1] = bArrIsHorizon[1];
				nArrS[0] = nArrSelectDir[0]; nArrS[1] = nArrSelectDir[1];
				if (bArrIsHorizon[0] != bArrIsHorizon[1])
					sRstAlgo->m_poDrawLine_T3[0] = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrA, dArrB, bArrH);
				else
				{
					sRstAlgo->m_poDrawLine_T3[0].x = (fArrInspCenterX[0] + fArrInspCenterX[1]) / 2.0f;
					sRstAlgo->m_poDrawLine_T3[0].y = (fArrInspCenterY[0] + fArrInspCenterY[1]) / 2.0f;
				}
				dArrA[0] = dArrLineDAValue[2]; dArrA[1] = dArrLineDAValue[3];
				dArrB[0] = dArrLineDBValue[2]; dArrB[1] = dArrLineDBValue[3];
				bArrH[0] = bArrIsHorizon[2]; bArrH[1] = bArrIsHorizon[3];
				if (bArrIsHorizon[2] != bArrIsHorizon[3])
					sRstAlgo->m_poDrawLine_T3[1] = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrA, dArrB, bArrH);
				else
				{
					sRstAlgo->m_poDrawLine_T3[1].x = (fArrInspCenterX[2] + fArrInspCenterX[3]) / 2.0f;
					sRstAlgo->m_poDrawLine_T3[1].y = (fArrInspCenterY[2] + fArrInspCenterY[3]) / 2.0f;
				}
				dArrA[0] = sRstAlgo->m_poDrawLine_T3[0].x;
				dArrA[1] = sRstAlgo->m_poDrawLine_T3[1].x;
				dArrB[0] = sRstAlgo->m_poDrawLine_T3[0].y;
				dArrB[1] = sRstAlgo->m_poDrawLine_T3[1].y;
				fInspCenterX = (sRstAlgo->m_poDrawLine_T3[0].x + sRstAlgo->m_poDrawLine_T3[1].x) / 2.0f;
				fInspCenterY = (sRstAlgo->m_poDrawLine_T3[0].y + sRstAlgo->m_poDrawLine_T3[1].y) / 2.0f;
				double dAV = 0, dBV = 0;
				bool bHorizon = true;
				if (dWndAngle == 90 || dWndAngle == 270)
					bHorizon = false;
				dRstAngle = m_pCPInsp_Algo->GetGradient(dArrA, dArrB, 2, dAV, dBV, bHorizon);
			}
			sRstAlgo->m_dRstShiftX = (dTechCenterX - fInspCenterX) * m_resolX;
			sRstAlgo->m_dRstShiftY = (dTechCenterY - fInspCenterY) * m_resolY;
			sRstAlgo->m_dRstAngle = dRstAngle;
			sRstAlgo->m_dRstRealAngle = dRstAngle;
			sRstAlgo->m_poDrawCenter.x = fInspCenterX;
			sRstAlgo->m_poDrawCenter.y = fInspCenterY;
		}
		sRstAlgo->m_bOKLength = TRUE;
		if (pInspAlgoEdge->m_bTeachLengthUse == TRUE)
		{
			for (int n = 0; n < nLineTotalCnt; n++)
			{
				if (sRstAlgo->m_bArrOKLength[n] == FALSE)
				{
					bReturn = FALSE;
					sRstAlgo->m_bOKLength = FALSE;
					break;
				}
			}
		}
		sRstAlgo->m_bOKAngle = TRUE;
		if (pInspAlgoEdge->m_bUseAngle == TRUE)
		{
			double dStdAngle = pInspAlgoEdge->m_dStdTeachRotate;
			if (dStdAngle < 0) dStdAngle *= -1.0;
			if (sRstAlgo->m_dRstAngle < 0)
				dRstAngle = sRstAlgo->m_dRstAngle + dStdAngle;
			else
				dRstAngle = sRstAlgo->m_dRstAngle - dStdAngle;
			sRstAlgo->m_dRstAngle = dRstAngle;
			if (dRstAngle < 0) dRstAngle *= -1.0;
			if (dRstAngle > pInspAlgoEdge->m_dTeachRotate)
			{
				bReturn = FALSE;
				sRstAlgo->m_bOKAngle = bReturn;
			}
		}
		else
		{
			sRstAlgo->m_dRstRealAngle = 0;
			sRstAlgo->m_dRstAngle = 0;
		}
		sRstAlgo->m_bOKShiftX = TRUE;
		sRstAlgo->m_bOKShiftY = TRUE;
		if (pInspAlgoEdge->m_bShiftIsUse == TRUE)
		{
			if (bShiftXUse == TRUE)
			{
				double dShiftXBuf = sRstAlgo->m_dRstShiftX;
				if (dShiftXBuf < 0)
					dShiftXBuf = sRstAlgo->m_dRstShiftX * -1;
				if (dShiftXBuf > dShiftX)
				{
					bReturn = FALSE;
					sRstAlgo->m_bOKShiftX = bReturn;
				}
			}
			else
				sRstAlgo->m_dRstShiftX = 0;

			if (bShiftYUse == TRUE)
			{
				double dShiftYBuf = sRstAlgo->m_dRstShiftY;
				if (dShiftYBuf < 0)
					dShiftYBuf = sRstAlgo->m_dRstShiftY * -1;
				if (dShiftYBuf > dShiftY)
				{
					bReturn = FALSE;
					sRstAlgo->m_bOKShiftY = bReturn;
				}
			}
			else
				sRstAlgo->m_dRstShiftY = 0;
		}
		else
		{
			sRstAlgo->m_dRstShiftX = 0;
			sRstAlgo->m_dRstShiftY = 0;
		}
		BOOL bDistance = pInspAlgoEdge->m_bTeachWidthUse;
		double dDistanceMin = pInspAlgoEdge->m_dTeachWidthRateMin >= 50 ? pInspAlgoEdge->m_dTeachWidth*0.8 : pInspAlgoEdge->m_dTeachWidthRateMin;
		double dDistanceMax = pInspAlgoEdge->m_dTeachWidthRateMax >= 100 ? pInspAlgoEdge->m_dTeachWidth*1.2 : pInspAlgoEdge->m_dTeachWidthRateMax;

		BOOL bDistanceX = pInspAlgoEdge->m_bDistanceX;
		double dDistanceXMin = pInspAlgoEdge->m_dTeachDistanceXRateMin;
		double dDistanceXMax = pInspAlgoEdge->m_dTeachDistanceXRateMax;

		BOOL bDistanceY = pInspAlgoEdge->m_bDistanceY;
		double dDistanceYMin = pInspAlgoEdge->m_dTeachDistanceYRateMin;
		double dDistanceYMax = pInspAlgoEdge->m_dTeachDistanceYRateMax;
		if (dWndAngle == 90 || dWndAngle == 270)
		{
			bDistanceX = pInspAlgoEdge->m_bDistanceY;
			dDistanceXMin = pInspAlgoEdge->m_dTeachDistanceYRateMin;
			dDistanceXMax = pInspAlgoEdge->m_dTeachDistanceYRateMax;

			bDistanceY = pInspAlgoEdge->m_bDistanceX;
			dDistanceYMin = pInspAlgoEdge->m_dTeachDistanceXRateMin;
			dDistanceYMax = pInspAlgoEdge->m_dTeachDistanceXRateMax;
		}
		double dRstDistance = 0.0;
		double dRstDistanceX = 0.0;
		double dRstDistanceY = 0.0;
		for (int n = 0; n < nLineTotalCnt - 1; n++)
		{
			dRstDistanceX += ((fArrInspCenterX[n + 1] * m_resolX) - (fArrInspCenterX[n] * m_resolX));
			dRstDistanceY += ((fArrInspCenterY[n + 1] * m_resolY) - (fArrInspCenterY[n] * m_resolY));
			dRstDistance += sqrt(pow(((fArrInspCenterX[n + 1] * m_resolX) - (fArrInspCenterX[n] * m_resolX)), 2) + pow(((fArrInspCenterY[n + 1] * m_resolY) - (fArrInspCenterY[n] * m_resolY)), 2));
		}

		sRstAlgo->m_bDistance = TRUE;
		sRstAlgo->m_bDistanceX = TRUE;
		sRstAlgo->m_bDistanceY = TRUE;
		sRstAlgo->m_dRstDistance = dRstDistance;
		sRstAlgo->m_dRstDistanceX = abs(dRstDistanceX);
		sRstAlgo->m_dRstDistanceY = abs(dRstDistanceY);

		if (bDistance == TRUE || bDistanceX == TRUE || bDistanceY == TRUE)
		{
			if (bDistance == TRUE)	//Distance
			{
				if (sRstAlgo->m_dRstDistance < dDistanceMin || sRstAlgo->m_dRstDistance > dDistanceMax)
				{
					bReturn = FALSE;
					sRstAlgo->m_bDistance = bReturn;
				}
			}
			if (bDistanceX == TRUE)	//DistanceX
			{
				if (sRstAlgo->m_dRstDistanceX < dDistanceXMin || sRstAlgo->m_dRstDistanceX > dDistanceXMax)
				{
					bReturn = FALSE;
					sRstAlgo->m_bDistanceX = bReturn;
				}
			}
			if (bDistanceY == TRUE)	//DistanceY
			{
				if (sRstAlgo->m_dRstDistanceY < dDistanceYMin || sRstAlgo->m_dRstDistanceY > dDistanceYMax)
				{
					bReturn = FALSE;
					sRstAlgo->m_bDistanceY = bReturn;
				}
			}
		}
		else
		{
			if (ucArrDstImg != NULL)
			{
				sRstAlgo->m_dRstDistanceX = abs(dRstDistanceX);
				sRstAlgo->m_dRstDistanceY = abs(dRstDistanceY);
				sRstAlgo->m_dRstDistance = dRstDistance;
			}
			else
			{
				sRstAlgo->m_dRstDistanceX = 0;
				sRstAlgo->m_dRstDistanceY = 0;
				sRstAlgo->m_dRstDistance = 0;
			}
		}
	}

	Delete_1DArray(&ucArrDstWnd);
	return bReturn;
}

BOOL CPInsp_AlgoEdge::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	if ((inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
	{
		RstAlgoEdge * rst = (RstAlgoEdge *)sRstAlgo;
		pAlignRes->offsetX = -rst->m_dRstShiftX;
		pAlignRes->offsetY = rst->m_dRstShiftY;
		pAlignRes->theta = rst->m_dRstAngle;
		bRet = TRUE;
	}

	return bRet;
}
int CPInsp_AlgoEdge::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoEdge * pInspAlgo = (AlgoEdge *)sInspAlgo.m_ptrInspAlgoParam;

	nData = pInspAlgo->m_sAlgoColorBase.GetColorData();
	nData |= COLOR_DATA_WND;

	return nData;
}
bool CPInsp_AlgoEdge::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}