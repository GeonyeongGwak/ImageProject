#include "PInsp_AlgoNewAlgo1.h"


//★★ 코드 작성 필요
CPInsp_AlgoNewAlgo1::CPInsp_AlgoNewAlgo1(void)
{
}


CPInsp_AlgoNewAlgo1::~CPInsp_AlgoNewAlgo1(void)
{
}

void CPInsp_AlgoNewAlgo1::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoNewAlgo1::GetInspAlgoData()
{
	unsigned long long ret = -1;
	//shw Delete Xret = eSPCAlgoNewAlgo1;
	return ret;
}

int CPInsp_AlgoNewAlgo1::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	//★★ 코드 수정 필요
	int nCurrentNgType = 0;
	eWholeNgTypeTemp[TypeMountWorng] = e_NG;
	nCurrentNgType = TypeMountWorng;
	return nCurrentNgType;
}

bool CPInsp_AlgoNewAlgo1::InspWindowArea(int nType)
{
	//★★ 코드 수정 필요
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoNewAlgo1::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	//★★ 코드 수정 필요
	BOOL bResult = FALSE;

	RstAlgoNewAlgo1 * rst = (RstAlgoNewAlgo1 *)sRstAlgo;
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if (!m_pProcMilAlgo || rst == NULL || pucImgSrc == NULL || pfImgSrc == NULL || nImgWidth <= 1 || nImgHeight <= 1)
		return bResult;
	//shw Delete Xif (sInspAlgo.m_eAlgoType != eAlgoNewAlgo1)#@return bResult;
	memset(rst, 0, (sizeof(RstAlgoNewAlgo1)));
	rst->Init();
	for (int a = 0; a < 2; a++)
	{
		rst->m_poXY[a].x = -1;
		rst->m_poXY[a].y = -1;
	}
	AlgoNewAlgo1 * pInsp = (AlgoNewAlgo1 *)sInspAlgo.m_ptrInspAlgoParam;
	bool bUse2D = ((pInsp->m_ArrDataN[m_eNewAlgo1N_Data] & m_eNewAlgo1Data_Use2D) == m_eNewAlgo1Data_Use2D);
	bool bUse3D = ((pInsp->m_ArrDataN[m_eNewAlgo1N_Data] & m_eNewAlgo1Data_Use3D) == m_eNewAlgo1Data_Use3D);
	if (bUse2D == false && bUse3D == false)
		return bResult;
	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	bool bDelete = false;
	if (ucArrDstImg == NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrDstImg, nImgWidth * nImgHeight);
		memset(ucArrDstImg, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		bDelete = true;
	}
	if (bUse2D)
	{
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Origin2D.bmp"));
		m_pCPInsp_Algo->Binarize(pucImgSrc, nImgWidth, nImgHeight, pInsp->m_ArrDataN[m_eNewAlgo1N_Range2D],
			pInsp->m_ArrDataN[m_eNewAlgo1N_Min2D], pInsp->m_ArrDataN[m_eNewAlgo1N_Max2D], false, pUcImgDst2D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst2D, nImgWidth, nImgHeight, _T("Bin2D_Org.bmp"));
		if (bUse3D == false) memcpy(ucArrDstImg, pUcImgDst2D, sizeof(UCHAR)*nImgWidth * nImgHeight);
	}
	if (bUse3D)
	{
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Origin3D.bmp"));
		m_pCPInsp_Algo->Binarize(pfImgSrc, nImgWidth, nImgHeight, pInsp->m_ArrDataN[m_eNewAlgo1N_Range3D],
			pInsp->m_ArrDataF[m_eNewAlgo1F_Min3D], pInsp->m_ArrDataF[m_eNewAlgo1F_Max3D], false, pUcImgDst3D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst3D, nImgWidth, nImgHeight, _T("Bin3D_Org.bmp"));
		if (bUse2D == false) memcpy(ucArrDstImg, pUcImgDst3D, sizeof(UCHAR)*nImgWidth * nImgHeight);
	}
	if (bUse2D && bUse3D && pUcImgDst2D != NULL && pUcImgDst3D != NULL)
	{
		for (int a = 0; a < nImgWidth * nImgHeight; a++)
			ucArrDstImg[a] = ((pUcImgDst2D[a] == 255) && (pUcImgDst3D[a] == 255)) ? 255 : 0;
	}
	m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("Bin2D3D.bmp"));
	Delete_1DArray(&pUcImgDst2D);
	Delete_1DArray(&pUcImgDst3D);
	int nSizeW = pInsp->m_ArrDataPO[m_eNewAlgo1PO_Size].x;
	int nSizeH = pInsp->m_ArrDataPO[m_eNewAlgo1PO_Size].y;
	if (sWndAlgoImg.dAngle == 90 || sWndAlgoImg.dAngle == 270)
	{
		nSizeW = pInsp->m_ArrDataPO[m_eNewAlgo1PO_Size].y;
		nSizeH = pInsp->m_ArrDataPO[m_eNewAlgo1PO_Size].x;
	}
	int nMargin = 10;
	int nSearchSizeX = nSizeW + nMargin;
	int nSearchSizeY = nSizeH + nMargin;
	if (nSearchSizeX > nImgWidth) nSearchSizeX = nImgWidth;
	if (nSearchSizeY > nImgHeight) nSearchSizeY = nImgHeight;
	POINTF poSrcPoint[2];
	memset(poSrcPoint, 0, sizeof(POINTF) * 2);
	memset(rst->m_poXY, 0, sizeof(POINTF) * 2);
	rst->m_bOKShift = TRUE;
	rst->m_bOKA = TRUE;
	rst->m_fRstShiftX = 0;
	rst->m_fRstShiftY = 0;
	rst->m_fRstA = 0;
	BOOL m_bOKSearch = TRUE;
	UCHAR * ucArrSearch = NULL;
	UCHAR * ucArrBlobDst = NULL;
	for (int a = 0; a < 2; a++)
	{
		POINTF poSrc;
		poSrc.x = pInsp->m_ArrDataPO[a].x / m_resolX;
		poSrc.y = pInsp->m_ArrDataPO[a].y / m_resolY;
		if (m_pCPInsp_Algo->AnglePointChange(nImgWidth, nImgHeight, poSrc, &poSrcPoint[a]) == FALSE)
		{
			if (bDelete) Delete_1DArray(&ucArrDstImg);
			return bResult;
		}
		int nL = poSrcPoint[a].x - (nSearchSizeX / 2);
		int nT = poSrcPoint[a].y - (nSearchSizeY / 2);
		if (stAlgoParam.m_nAlignCnt > 0 && stAlgoParam.m_sAlignRes != NULL)
		{
			nL = nL + (nSearchSizeX / 2.0) - ((double)nImgWidth / 2.0);
			nT = ((double)nImgHeight / 2.0) - (nT + (nSearchSizeY / 2.0));
			double corr_x = 0, corr_y = 0;
			for (int n = 0; n < stAlgoParam.m_nAlignCnt; n++)
			{
				double corr_x_Buf = 0, corr_y_Buf = 0;
				int nCorrectCoordinate = m_proc3d.CorrectCoordinate(nL, nT, stAlgoParam.m_sAlignRes[n].centerX / m_resolX, stAlgoParam.m_sAlignRes[n].centerY / m_resolY, stAlgoParam.m_sAlignRes[n].theta, stAlgoParam.m_sAlignRes[n].offsetX / m_resolX, stAlgoParam.m_sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
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
			nL = RounD(corr_x + ((double)nImgWidth / 2.0) - (nSearchSizeX / 2.0));
			nT = RounD(((double)nImgHeight / 2.0) - (corr_y + (nSearchSizeY / 2.0)));
			if (nL + RounD(nSearchSizeX) > nImgWidth)		// 2014/12/15	: 예외처리
				nL = nImgWidth - nSearchSizeX;
			if (nT + RounD(nSearchSizeY) > nImgHeight)
				nT = nImgHeight - nSearchSizeY;
		}
		int nR = nL + nSearchSizeX;
		int nB = nT + nSearchSizeY;
		if (nL >= nImgWidth) nL = nImgWidth - 1;
		if (nT >= nImgHeight) nT = nImgHeight - 1;
		if (nR > nImgWidth) nR = nImgWidth;
		if (nB > nImgHeight) nB = nImgHeight;
		if (nL < 0) nL = 0;
		if (nT < 0) nT = 0;
		if (nR < 0) nR = 0;
		if (nB < 0) nB = 0;
		int nW = nR - nL;
		int nH = nB - nT;
		if (nW <= 1 || nH <= 1)
		{
			if (bDelete) Delete_1DArray(&ucArrDstImg);
			return bResult;
		}
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearch, nW * nH);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobDst, nW * nH);
		m_pProcMilAlgo->GetClipImage_LT(ucArrDstImg, nImgWidth, nImgHeight, ucArrSearch, nL, nT, nW, nH);
		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrSearch, ucArrBlobDst, nW, nH, 4, FALSE, FALSE, 0, eSelectMix);
		if (nCntBlob == 0)
		{
			rst->m_poXY[a].x = nL + nSearchSizeX / 2;
			rst->m_poXY[a].y = nT + nSearchSizeY / 2;
			rst->m_poXY[2].x = nSizeW;
			rst->m_poXY[2].y = nSizeH;
			rst->m_bOKShift = FALSE;
			rst->m_bOKA = FALSE;
			if (a == 0)
			{
				m_bOKSearch = FALSE;
				continue;
			}
			if (bDelete) Delete_1DArray(&ucArrDstImg);
			Delete_1DArray(&ucArrSearch);
			Delete_1DArray(&ucArrBlobDst);
			return bResult;
		}
		double *dArrX = NULL;
		double *dArrY = NULL;
		CRect *cArrR = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrX, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrY, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &cArrR, nCntBlob);
		m_pProcMilAlgo->GetForeignBlobResult(NULL, dArrX, dArrY, cArrR);
		double dAreaMin = 0.4;
		double dAreaMax = 1.6;
		int nBlobSizeWMin = ((double)nSizeW * dAreaMin);
		int nBlobSizeWMax = ((double)nSizeW * dAreaMax);
		int nBlobSizeHMin = ((double)nSizeH * dAreaMin);
		int nBlobSizeHMax = ((double)nSizeH * dAreaMax);
		int nGoodNum = -1;
		double dMin = nW * nH;
		for (int c = 0; c < nCntBlob; c++)
		{
			int nSearchW = cArrR[c].Width();
			int nSearchH = cArrR[c].Height();
			if (stAlgoParam.m_bTeach)
			{
				double dX = dArrX[c] - (nW / 2);
				double dY = dArrY[c] - (nH / 2);
				double dDist = sqrt(pow(dX, 2) + pow(dY, 2));
				if (dMin > dDist)
					nGoodNum = c;
			}
			else
			{
				if (nBlobSizeWMin < nSearchW && nBlobSizeWMax > nSearchW &&
					nBlobSizeHMin < nSearchH && nBlobSizeHMax > nSearchH)
				{
					nGoodNum = c;
					break;
				}
			}
		}
		if (nGoodNum == -1)
		{
			dMin = nW * nH;
			for (int c = 0; c < nCntBlob; c++)
			{
				int nSearchW = cArrR[c].Width();
				int nSearchH = cArrR[c].Height();
				double dX = dArrX[c] - (nW / 2);
				double dY = dArrY[c] - (nH / 2);
				double dDist = sqrt(pow(dX, 2) + pow(dY, 2));
				if (dMin > dDist)
					nGoodNum = c;
			}
		}
		if (nGoodNum > -1)
		{
			rst->m_poXY[a].x = nL + dArrX[nGoodNum];
			rst->m_poXY[a].y = nT + dArrY[nGoodNum];
			rst->m_poXY[2].x += cArrR[nGoodNum].Width();
			rst->m_poXY[2].y += cArrR[nGoodNum].Height();
		}
		Delete_1DArray(&ucArrSearch);
		Delete_1DArray(&ucArrBlobDst);
		Delete_1DArray(&dArrX);
		Delete_1DArray(&dArrY);
		Delete_1DArray(&cArrR);
		if (nGoodNum == -1)
		{
			if (bDelete) Delete_1DArray(&ucArrDstImg);
			return bResult;
		}
	}
	if (bDelete) Delete_1DArray(&ucArrDstImg);
	if (((pInsp->m_ArrDataN[m_eNewAlgo1N_Data] & m_eNewAlgo1Data_Shift) == m_eNewAlgo1Data_Shift) && m_bOKSearch)
	{
		float fX_R = (((poSrcPoint[0].x - rst->m_poXY[0].x) + (poSrcPoint[1].x - rst->m_poXY[1].x)) / 2.0f) * m_resolX;
		float fY_R = (((poSrcPoint[0].y - rst->m_poXY[0].y) + (poSrcPoint[1].y - rst->m_poXY[1].y)) / 2.0f) * m_resolY;
		float fX_T = pInsp->m_ArrDataF[m_eNewAlgo1F_ShiftX];
		float fY_T = pInsp->m_ArrDataF[m_eNewAlgo1F_ShiftY];
		if (fX_R < 0) fX_R *= -1.0f;
		if (fY_R < 0) fY_R *= -1.0f;
		if (fX_T < 0) fX_T *= -1.0f;
		if (fY_T < 0) fY_T *= -1.0f;
		rst->m_fRstShiftX = fX_R;
		rst->m_fRstShiftY = fY_R;
		rst->m_bOKShift = (fX_T > fX_R && fY_T > fY_R);
	}
	if (((pInsp->m_ArrDataN[m_eNewAlgo1N_Data] & m_eNewAlgo1Data_Angle) == m_eNewAlgo1Data_Angle) && m_bOKSearch)
	{
		rst->m_fRstA = 0.0;
		double dDeltaX1 = poSrcPoint[1].x - poSrcPoint[0].x;
		double dDeltaY1 = poSrcPoint[0].y - poSrcPoint[1].y;
		double dDeltaX2 = rst->m_poXY[1].x - rst->m_poXY[0].x;
		double dDeltaY2 = rst->m_poXY[0].y - rst->m_poXY[1].y;
		double dRad1 = (double)atan2(dDeltaY1, dDeltaX1);     //티칭된 각도(radian)
		double dRad2 = (double)atan2(dDeltaY2, dDeltaX2);    //검사한 각도(radian)
		double temp = dRad2 - dRad1;
		if (temp >= -PI)
		{
			while (temp >= PI)
				temp -= PI * 2.0;
		}
		else
		{
			while (temp < -PI)
				temp += PI * 2.0;
		}
		rst->m_fRstA = (temp * 180) / PI;
		float fA_T = pInsp->m_ArrDataF[m_eNewAlgo1F_Angle];
		float fA_R = rst->m_fRstA;
		if (fA_T < 0) fA_T *= -1.0f;
		if (fA_R < 0) fA_R *= -1.0f;
		rst->m_bOKA = (fA_T > fA_R);
	}
	rst->m_poXY[2].x /= 2.0f;
	rst->m_poXY[2].y /= 2.0f;
	bResult = (rst->m_bOKA && rst->m_bOKShift && m_bOKSearch);



	return bResult;
}

BOOL CPInsp_AlgoNewAlgo1::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	//★★ 코드 수정 필요
	BOOL bRet = FALSE;

	RstAlgoNewAlgo1 * rst = (RstAlgoNewAlgo1 *)sRstAlgo;
	pAlignRes->offsetX = (double)-rst->m_fRstShiftX;
	pAlignRes->offsetY = (double)rst->m_fRstShiftY;
	pAlignRes->theta = (double)rst->m_fRstA;
	bRet = TRUE;

	return bRet;
}
int CPInsp_AlgoNewAlgo1::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoNewAlgo1::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}