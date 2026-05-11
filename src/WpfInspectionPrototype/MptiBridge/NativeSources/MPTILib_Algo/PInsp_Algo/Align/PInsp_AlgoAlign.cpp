#include "PInsp_AlgoAlign.h"


CPInsp_AlgoAlign::CPInsp_AlgoAlign(void)
{
}


CPInsp_AlgoAlign::~CPInsp_AlgoAlign(void)
{
}

void CPInsp_AlgoAlign::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoAlign::GetInspAlgoData()
{
	return eSPCAlgoAlign;
}

int CPInsp_AlgoAlign::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeAlignWrong] = e_NG;
	nCurrentNgType = TypeAlignWrong;
	return nCurrentNgType;
}

bool CPInsp_AlgoAlign::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoAlign::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg )
{
	BOOL bResult = FALSE;

	if (stAlgoParam.m_bInspection)
	{
		if (stAlgoParam.m_bUSeLeadAlign == FALSE)
			bResult = InspAlign(sInspAlgo, sWndAlgoImg, *stAlgoParam.m_rcBlobBody, *sInspImageData, (RstAlgoAlign *)sRstAlgo, ucArrDstImg, stAlgoParam.m_sAlignRes, stAlgoParam.m_nAlignCnt);
		else
			bResult = InspAlign(sInspAlgo, sWndAlgoImg, *stAlgoParam.m_rcBlobBody, *sInspImageData, (RstAlgoAlign *)sRstAlgo);
	}
	else
	{
		if (stAlgoParam.m_byWndInspType == 1)
			bResult = TeachLeadAlign(sInspAlgo, sWndAlgoImg, (RstAlgoAlign *)sRstAlgo, stAlgoParam);
		else
			bResult = TeachAlign(sInspAlgo, sWndAlgoImg, *sInspImageData, ucArrDstImg);
	}

	return bResult;
}

BOOL CPInsp_AlgoAlign::InspAlign(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, CRect &rcBlobBody, InspRoiImgBuf &sInspImageData, RstAlgoAlign *sRstAlgo, UCHAR* ucArrDstImg, AlignResult * sAlignRes, int nAlignCnt)
{
	BOOL  bResult = TRUE;
	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, (sizeof(RstAlgoAlign)));
		sRstAlgo->Init();
	}
	rcBlobBody = CRect(0, 0, 0, 0);
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	AlgoAlign * pInspAlgoAlign = (AlgoAlign *)sInspAlgo.m_ptrInspAlgoParam;
	if (!m_pProcMilAlgo || sInspAlgo.m_eAlgoType != eAlgoAlign ||
		!pInspAlgoAlign->m_bInsp2D && !pInspAlgoAlign->m_bInsp3D && !pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor ||
		pucImgSrc == NULL || (pfImgSrc == NULL && sWndAlgoImg.m_nLight_index == (int)eMSCN_COAXIAL) || nImgWidth <= 0 || nImgHeight <= 0)
		return FALSE;
#if _DEBUG
	cv::Mat ImgpucImgSrc(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc);
#endif
	if (ucArrDstImg)
	{
		IppStatus sts;
		IppiSize iSize = { nImgWidth, nImgHeight };
		sts = ippiCopy_8u_C1R(pucImgSrc, nImgWidth, ucArrDstImg, nImgWidth, iSize);

		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Origin2D.bmp"));
	}
	UCHAR *ucColorImgDst = NULL;
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	UCHAR * ucArrSearchColor = NULL;
	if (pColorTeach != NULL && pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		pColorTeach->GetColorBaseBin(&pInspAlgoAlign->m_sAlgoColorBase, sInspImageData, ucColorImgDst, false, 0, 0, -1, sWndAlgoImg.m_nLight_index);
		m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nImgWidth, nImgHeight, _T("OrgColor_Image.bmp"));
	}
	AlgoBlob algoBlob = m_pCPInsp_Algo->SetAlgoBlob(sInspAlgo);
	TotalInspExceptArea stTieArea;
	stTieArea.m_nUsedInspPolygon = 0;
	stTieArea.m_nUsedMaskingValue = 0;
	stTieArea.m_nUsedWndPolygon = 0;
	const int nIndexMax = 3;
	const int nSearchNumMAx = 4;
	int nSearchNum = pInspAlgoAlign->m_nSearchNum;
	double sum_x = 0, sum_y = 0;
	double dRstSum_x = 0, dRstSum_y = 0;
	double insp_x[nSearchNumMAx];
	double insp_y[nSearchNumMAx];
	POINTF rcDstPoint[nSearchNumMAx];
	CRect rcBlob[nSearchNumMAx];
	CRect rcBlob_Search[nSearchNumMAx][nIndexMax];
	double cx[nSearchNumMAx][nIndexMax];
	double cy[nSearchNumMAx][nIndexMax];
	int nFirstConectIndex = 1;
	BOOL bArrOKArea[nSearchNumMAx];
	int nArrLeft[nSearchNumMAx];
	int nArrTop[nSearchNumMAx];
	int nOKAreaCnt = 0;
	BOOL bFindOpt = TRUE;
	int nSearchSizeX[nSearchNumMAx];
	int nSearchSizeY[nSearchNumMAx];

	for (int nRoiIdx = 0; nRoiIdx < nSearchNumMAx; nRoiIdx++)
	{
		nSearchSizeX[nRoiIdx] = pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cx + pInspAlgoAlign->m_nSearchMargin * 2;
		nSearchSizeY[nRoiIdx] = pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cy + pInspAlgoAlign->m_nSearchMargin * 2;
	}
	for (int n = 0; n < nSearchNum; n++)
	{
		int nMinBlobArea = (pInspAlgoAlign->m_sArrSearchSize[n].cx * pInspAlgoAlign->m_sArrSearchSize[n].cy) / 10;
		if (nMinBlobArea < pInspAlgoAlign->m_nMinBlobArea)
			nMinBlobArea = pInspAlgoAlign->m_nMinBlobArea;
		UCHAR* ucArrSearch = NULL;
		float* fArrSearch = NULL;
		UCHAR* ucArrSearchColor = NULL;
		UCHAR* ucArrSearchDst = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearch, nSearchSizeX[n] * nSearchSizeY[n]);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrSearch, nSearchSizeX[n] * nSearchSizeY[n]);
		if (pColorTeach != NULL && pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearchColor, nSearchSizeX[n] * nSearchSizeY[n]);
		}
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearchDst, nSearchSizeX[n] * nSearchSizeY[n]);

		if (m_pCPInsp_Algo->AnglePointChange(nImgWidth, nImgHeight, pInspAlgoAlign->m_sArrSearchPoint[n], &rcDstPoint[n]) == FALSE)
		{
			Delete_1DArray(&ucArrSearch);
			Delete_1DArray(&fArrSearch);
			Delete_1DArray(&ucColorImgDst);
			Delete_1DArray(&ucArrSearchColor);
			Delete_1DArray(&ucArrSearchDst);
			return FALSE;
		}
		// Rect가 4개일 경우에만 첫번째 Rect에서 가장 먼 Rect를 찾는다.
		if (nSearchNum == nSearchNumMAx && n == nSearchNumMAx - 1)
		{
			double dPointDistance = 0.0;
			for (int b = 1; b < nSearchNum; b++)
			{
				float fx = rcDstPoint[0].x - rcDstPoint[b].x;
				float fy = rcDstPoint[0].y - rcDstPoint[b].y;

				double dDistance = (double)sqrt(pow(fx, 2) + pow(fy, 2));

				if (dDistance > dPointDistance)
				{
					dPointDistance = dDistance;
					nFirstConectIndex = b;
				}
			}
		}

		int left = RounD(rcDstPoint[n].x - nSearchSizeX[n] / 2.);
		int top = RounD(rcDstPoint[n].y - nSearchSizeY[n] / 2.);
		nArrLeft[n] = left;
		nArrTop[n] = top;
		if (nAlignCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
		{
			left = left + (nSearchSizeX[n] / 2.0) - ((double)nImgWidth / 2.0);
			top = ((double)nImgHeight / 2.0) - (top + (nSearchSizeY[n] / 2.0));

			double corr_x = 0, corr_y = 0;
			for (int n = 0; n < nAlignCnt; n++)
			{
				double corr_x_Buf = 0, corr_y_Buf = 0;
				int nCorrectCoordinate = m_proc3d.CorrectCoordinate(left, top, sAlignRes[n].centerX / m_resolX, sAlignRes[n].centerY / m_resolY, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
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
			left = RounD(corr_x + ((double)nImgWidth / 2.0) - (nSearchSizeX[n] / 2.0));
			top = RounD(((double)nImgHeight / 2.0) - (corr_y + (nSearchSizeY[n] / 2.0)));
		}

		if (left + RounD(nSearchSizeX[n]) > nImgWidth)		// 2014/12/15	: 예외처리
			left = nImgWidth - nSearchSizeX[n];
		if (top + RounD(nSearchSizeY[n]) > nImgHeight)
			top = nImgHeight - nSearchSizeY[n];

		m_proc3d.GetCropZmap_LT(pfImgSrc, fArrSearch, nImgWidth, nImgHeight, left, top, nSearchSizeX[n], nSearchSizeY[n]);
		m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, ucArrSearch, left, top, nSearchSizeX[n], nSearchSizeY[n]);
		m_pProcMilAlgo->SaveWorkImg_float(fArrSearch, nSearchSizeX[n], nSearchSizeY[n], _T("Align 3D.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ucArrSearch, nSearchSizeX[n], nSearchSizeY[n], _T("Align 2D.bmp"));
		if (pColorTeach != NULL && pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			m_pProcMilAlgo->GetClipImage_LT(ucColorImgDst, sInspImageData.nImageSizeX, sInspImageData.nImageSizeY, ucArrSearchColor, left, top, nSearchSizeX[n], nSearchSizeY[n]);
			m_pProcMilAlgo->SaveWorkImg(ucArrSearchColor, nSearchSizeX[n], nSearchSizeY[n], _T("Align Color.bmp"));
		}
		int nBlobNum = m_pCPInsp_Algo->BlobImageStruct(algoBlob, ucArrSearch, fArrSearch, ucArrSearchColor, nSearchSizeX[n], nSearchSizeY[n], nMinBlobArea, NULL, cx[n], cy[n], rcBlob_Search[n], ucArrSearchDst, stTieArea, algoBlob.m_bFillHole, TRUE);
		if (nBlobNum == 0)
		{
			bResult = FALSE;
			bArrOKArea[n] = FALSE;
		}
		else if (nBlobNum > 1)
		{
			if (nBlobNum > nIndexMax)
				nBlobNum = nIndexMax;
			double dAreaMin = 0.7;
			double dAreaMax = 1.3;
			int nBlobSizeW = pInspAlgoAlign->m_sArrSearchSize[n].cx;
			int nBlobSizeH = pInspAlgoAlign->m_sArrSearchSize[n].cy;
			int nBlobSizeWMin = ((double)nBlobSizeW * dAreaMin);
			int nBlobSizeWMax = ((double)nBlobSizeW * dAreaMax);
			int nBlobSizeHMin = ((double)nBlobSizeH * dAreaMin);
			int nBlobSizeHMax = ((double)nBlobSizeH * dAreaMax);
			int nGoodNum = -1;
			for (int c = 0; c < nBlobNum; c++)
			{
				int nSearchW = rcBlob_Search[n][c].Width();
				int nSearchH = rcBlob_Search[n][c].Height();
				if (nBlobSizeWMin < nSearchW && nBlobSizeWMax > nSearchW &&
					nBlobSizeHMin < nSearchH && nBlobSizeHMax > nSearchH)
				{
					nGoodNum = c;
					break;
				}
			}
			if (nGoodNum == -1)
			{
				bResult = FALSE;
				bArrOKArea[n] = FALSE;
			}
			else
			{
				for (int c = 0; c < nBlobNum; c++)
				{
					int nSearchW = rcBlob_Search[n][c].Width();
					int nSearchH = rcBlob_Search[n][c].Height();
					if (nBlobSizeWMin > nSearchW || nBlobSizeWMax < nSearchW
						|| nBlobSizeHMin > nSearchH || nBlobSizeHMax < nSearchH)
					{
						cx[n][c] = cx[n][nGoodNum];
						cy[n][c] = cy[n][nGoodNum];
						rcBlob_Search[n][c] = rcBlob_Search[n][nGoodNum];
					}
				}
				bArrOKArea[n] = true;
				nOKAreaCnt++;
			}
		}
		else
		{
			bArrOKArea[n] = true;
			nOKAreaCnt++;
		}
		if (sRstAlgo)
		{
			sRstAlgo->m_rcRect_T[n].left = RounD(rcDstPoint[n].x - pInspAlgoAlign->m_sArrSearchSize[n].cx / 2.0);
			sRstAlgo->m_rcRect_T[n].top = RounD(rcDstPoint[n].y - pInspAlgoAlign->m_sArrSearchSize[n].cy / 2.0);
			sRstAlgo->m_rcRect_T[n].right = sRstAlgo->m_rcRect_T[n].left + pInspAlgoAlign->m_sArrSearchSize[n].cx;
			sRstAlgo->m_rcRect_T[n].bottom = sRstAlgo->m_rcRect_T[n].top + pInspAlgoAlign->m_sArrSearchSize[n].cy;

			if (nBlobNum > 0)
			{
				sRstAlgo->m_rcRect_I[n].left = left + rcBlob_Search[n][0].left;
				sRstAlgo->m_rcRect_I[n].top = top + rcBlob_Search[n][0].top;
			}
			else
			{
				sRstAlgo->m_rcRect_I[n].left = RounD(rcDstPoint[n].x - pInspAlgoAlign->m_sArrSearchSize[n].cx / 2.0);
				sRstAlgo->m_rcRect_I[n].top = RounD(rcDstPoint[n].y - pInspAlgoAlign->m_sArrSearchSize[n].cy / 2.0);
			}
			sRstAlgo->m_rcRect_I[n].right = sRstAlgo->m_rcRect_I[n].left + pInspAlgoAlign->m_sArrSearchSize[n].cx;
			sRstAlgo->m_rcRect_I[n].bottom = sRstAlgo->m_rcRect_I[n].top + pInspAlgoAlign->m_sArrSearchSize[n].cy;
		}
		Delete_1DArray(&ucArrSearch);
		Delete_1DArray(&fArrSearch);
		Delete_1DArray(&ucArrSearchDst);
		Delete_1DArray(&ucArrSearchColor);
	}
	if (bFindOpt == TRUE && nOKAreaCnt > 1 && nOKAreaCnt != nSearchNum)
	{
		bResult = TRUE;
		POINTF rcDstPointBuf[nSearchNumMAx];
		CRect rcBlob_SearchBuf[nSearchNumMAx][nIndexMax];
		double cxBuf[nSearchNumMAx][nIndexMax];
		double cyBuf[nSearchNumMAx][nIndexMax];
		int nIdxBuf = 0;
		for (int a = 0; a < nSearchNum; a++)
		{
			if (bArrOKArea[a] == true)
			{
				rcDstPointBuf[nIdxBuf].x = rcDstPoint[a].x;
				rcDstPointBuf[nIdxBuf].y = rcDstPoint[a].y;
				for (int b = 0; b < nIndexMax; b++)
				{
					rcBlob_SearchBuf[nIdxBuf][b].left = rcBlob_Search[a][b].left;
					rcBlob_SearchBuf[nIdxBuf][b].right = rcBlob_Search[a][b].right;
					rcBlob_SearchBuf[nIdxBuf][b].top = rcBlob_Search[a][b].top;
					rcBlob_SearchBuf[nIdxBuf][b].bottom = rcBlob_Search[a][b].bottom;

					cxBuf[nIdxBuf][b] = cx[a][b];
					cyBuf[nIdxBuf][b] = cy[a][b];
				}
				nIdxBuf++;
			}
		}
		nSearchNum = nIdxBuf;
		for (int a = 0; a < nSearchNum; a++)
		{
			rcDstPoint[a] = rcDstPointBuf[a];
			for (int b = 0; b < nIndexMax; b++)
			{
				rcBlob_Search[a][b] = rcBlob_SearchBuf[a][b];

				cx[a][b] = cxBuf[a][b];
				cy[a][b] = cyBuf[a][b];
			}
		}
		nFirstConectIndex = 1;
	}
	if (bResult == TRUE)
	{
		// Rect내에 찾은 Blob을 이용하여 거리를 측정한다.
		int nArrBlobIndex[nSearchNumMAx] = { 0, };
		// Rect 개수에 따라 검사 횟수가 달라짐
		// 2개면 1번만 돌면 된다.
		int nSearchCnt = 1;
		if (nSearchNum > 2)
			nSearchCnt = 2;
		int nFirIndex = 0;
		int nSecIndex = 0;
		for (int a = 0; a < nSearchCnt; a++)
		{
			if (a == 0)
			{
				nFirIndex = 0;
				nSecIndex = nFirstConectIndex;
			}
			else
			{
				if (nSearchNum == 3)
				{
					nFirIndex = 0;
					nSecIndex = nSearchNum - nFirstConectIndex;
				}
				else
				{
					if (nFirstConectIndex == 1)
					{
						nFirIndex = 2;
						nSecIndex = 3;
					}
					else if (nFirstConectIndex == 2)
					{
						nFirIndex = 1;
						nSecIndex = 3;
					}
					else
					{
						nFirIndex = 1;
						nSecIndex = 2;
					}
				}
			}
			// Rect Center 위치 값
			double dRectCenterX = rcDstPoint[nFirIndex].x - rcDstPoint[nSecIndex].x;
			double dRectCenterY = rcDstPoint[nFirIndex].y - rcDstPoint[nSecIndex].y;
			double dRectCenterDistance = sqrt(pow(dRectCenterX, 2) + pow(dRectCenterY, 2));
			// 가져온 Rect의 위치값을 이미지 좌표로 계산하기 위해 필요
			double dFirImagePosX = rcDstPoint[nFirIndex].x - nSearchSizeX[nFirIndex] / 2.;
			double dFirImagePosY = rcDstPoint[nFirIndex].y - nSearchSizeY[nFirIndex] / 2.;
			double dSecImagePosX = rcDstPoint[nSecIndex].x - nSearchSizeX[nSecIndex] / 2.;
			double dSecImagePosY = rcDstPoint[nSecIndex].y - nSearchSizeY[nSecIndex] / 2.;
			double dDiffValue = dRectCenterDistance;
			for (int a1 = 0; a1 < nIndexMax; a1++)
			{
				if (rcBlob_Search[nFirIndex][a1].left <= 0 && rcBlob_Search[nFirIndex][a1].right <= 0 && rcBlob_Search[nFirIndex][a1].top <= 0 && rcBlob_Search[nFirIndex][a1].bottom <= 0)
					continue;
				for (int b1 = 0; b1 < nIndexMax; b1++)
				{
					// 					if(rcBlob_Search[nFirIndex][b1].left <= 0 && rcBlob_Search[nFirIndex][b1].right <= 0 && rcBlob_Search[nFirIndex][b1].top <= 0 && rcBlob_Search[nFirIndex][b1].bottom <= 0)
					if (rcBlob_Search[nSecIndex][b1].left <= 0 && rcBlob_Search[nSecIndex][b1].right <= 0 && rcBlob_Search[nSecIndex][b1].top <= 0 && rcBlob_Search[nSecIndex][b1].bottom <= 0)
						continue;

					double dx = (cx[nFirIndex][a1] + dFirImagePosX) - (cx[nSecIndex][b1] + dSecImagePosX);
					double dy = (cy[nFirIndex][a1] + dFirImagePosY) - (cy[nSecIndex][b1] + dSecImagePosY);

					double dDistance = sqrt(pow(dx, 2) + pow(dy, 2));
					double dDiff = dDistance - dRectCenterDistance;
					if (dRectCenterDistance > dDistance)
						dDiff = dRectCenterDistance - dDistance;

					if (dDiffValue > dDiff)
					{
						dDiffValue = dDiff;
						nArrBlobIndex[nFirIndex] = a1;
						nArrBlobIndex[nSecIndex] = b1;
					}
				}
			}
			double dRstCXFir = (cx[nFirIndex][nArrBlobIndex[nFirIndex]] + sWndAlgoImg.m_fPartRoundingErrX);
			double dRstCYFir = (cy[nFirIndex][nArrBlobIndex[nFirIndex]] + sWndAlgoImg.m_fPartRoundingErrY);

			double dRstCXSec = (cx[nSecIndex][nArrBlobIndex[nSecIndex]] + sWndAlgoImg.m_fPartRoundingErrX);
			double dRstCYSec = (cy[nSecIndex][nArrBlobIndex[nSecIndex]] + sWndAlgoImg.m_fPartRoundingErrY);

			double dTeachCXFir = rcDstPoint[nFirIndex].x;
			double dTeachCYFir = rcDstPoint[nFirIndex].y;

			double dTeachCXSec = rcDstPoint[nSecIndex].x;
			double dTeachCYSec = rcDstPoint[nSecIndex].y;

			sum_x += dTeachCXFir - (dRstCXFir + nArrLeft[nFirIndex]);
			sum_y += dTeachCYFir - (dRstCYFir + nArrTop[nFirIndex]);

			sum_x += dTeachCXSec - (dRstCXSec + nArrLeft[nSecIndex]);
			sum_y += dTeachCYSec - (dRstCYSec + nArrTop[nSecIndex]);

			dRstSum_x += (nArrLeft[nFirIndex] + dRstCXFir);
			dRstSum_y += (nArrTop[nFirIndex] + dRstCYFir);

			dRstSum_x += (nArrLeft[nSecIndex] + dRstCXSec);
			dRstSum_y += (nArrTop[nSecIndex] + dRstCYSec);

			insp_x[nFirIndex] = dRstCXFir + rcDstPoint[nFirIndex].x - nSearchSizeX[nFirIndex] / 2.;
			insp_y[nFirIndex] = dRstCYFir + rcDstPoint[nFirIndex].y - nSearchSizeY[nFirIndex] / 2.;

			insp_x[nSecIndex] = dRstCXSec + rcDstPoint[nSecIndex].x - nSearchSizeX[nSecIndex] / 2.;
			insp_y[nSecIndex] = dRstCYSec + rcDstPoint[nSecIndex].y - nSearchSizeY[nSecIndex] / 2.;

			rcBlob[nFirIndex].left = rcBlob_Search[nFirIndex][nArrBlobIndex[nFirIndex]].left + (int)dFirImagePosX;
			rcBlob[nFirIndex].right = rcBlob_Search[nFirIndex][nArrBlobIndex[nFirIndex]].right + (int)dFirImagePosX;
			rcBlob[nFirIndex].top = rcBlob_Search[nFirIndex][nArrBlobIndex[nFirIndex]].top + (int)dFirImagePosY;
			rcBlob[nFirIndex].bottom = rcBlob_Search[nFirIndex][nArrBlobIndex[nFirIndex]].bottom + (int)dFirImagePosY;
			rcBlob[nSecIndex].left = rcBlob_Search[nSecIndex][nArrBlobIndex[nSecIndex]].left + (int)dSecImagePosX;
			rcBlob[nSecIndex].right = rcBlob_Search[nSecIndex][nArrBlobIndex[nSecIndex]].right + (int)dSecImagePosX;
			rcBlob[nSecIndex].top = rcBlob_Search[nSecIndex][nArrBlobIndex[nSecIndex]].top + (int)dSecImagePosY;
			rcBlob[nSecIndex].bottom = rcBlob_Search[nSecIndex][nArrBlobIndex[nSecIndex]].bottom + (int)dSecImagePosY;
		}
	}
	if (bResult)
	{
		if (sRstAlgo)
		{
			switch (nSearchNum)
			{
			case 2:
				sRstAlgo->m_dTheta = Calc_Theta(rcDstPoint[0].x, rcDstPoint[0].y, rcDstPoint[1].x, rcDstPoint[1].y, insp_x[0], insp_y[0], insp_x[1], insp_y[1]);
				break;
			case 3:
			{
				double theta_sum = 0;
				theta_sum = Calc_Theta(rcDstPoint[0].x, rcDstPoint[0].y, rcDstPoint[1].x, rcDstPoint[1].y, insp_x[0], insp_y[0], insp_x[1], insp_y[1]);
				theta_sum += Calc_Theta(rcDstPoint[1].x, rcDstPoint[1].y, rcDstPoint[2].x, rcDstPoint[2].y, insp_x[1], insp_y[1], insp_x[2], insp_y[2]);
				theta_sum += Calc_Theta(rcDstPoint[2].x, rcDstPoint[2].y, rcDstPoint[0].x, rcDstPoint[0].y, insp_x[2], insp_y[2], insp_x[0], insp_y[0]);
				sRstAlgo->m_dTheta = theta_sum / 3.;
			}
			break;
			case 4:
			{
				double theta_sum = 0;
				theta_sum = Calc_Theta(rcDstPoint[0].x, rcDstPoint[0].y, rcDstPoint[1].x, rcDstPoint[1].y, insp_x[0], insp_y[0], insp_x[1], insp_y[1]);
				theta_sum += Calc_Theta(rcDstPoint[2].x, rcDstPoint[2].y, rcDstPoint[3].x, rcDstPoint[3].y, insp_x[2], insp_y[2], insp_x[3], insp_y[3]);
				sRstAlgo->m_dTheta = theta_sum / 2.;
			}
			break;
			}
			int nOffsetX = RounD(sum_x / nSearchNum);
			int nOffsetY = RounD(sum_y / nSearchNum);
			sRstAlgo->m_poDrawCenter.x = dRstSum_x / nSearchNum;
			sRstAlgo->m_poDrawCenter.y = dRstSum_y / nSearchNum;
			//for (int nIdx = 0; nIdx < nSearchNum; nIdx++)
			//{
			//	sRstAlgo->m_rcRect_I[nIdx].left -= nOffsetX;
			//	sRstAlgo->m_rcRect_I[nIdx].right -= nOffsetX;
			//	sRstAlgo->m_rcRect_I[nIdx].top -= nOffsetY;
			//	sRstAlgo->m_rcRect_I[nIdx].bottom -= nOffsetY;
			//}
			if (pInspAlgoAlign->m_bUseShift)
			{
				sRstAlgo->m_dOffset_x = (sum_x / nSearchNum) * m_resolX;
				sRstAlgo->m_dOffset_y = (sum_y / nSearchNum) * m_resolY;

				double dShiftX = pInspAlgoAlign->m_dShiftX;
				double dShiftY = pInspAlgoAlign->m_dShiftY;
				if (sWndAlgoImg.dAngle == 90 || sWndAlgoImg.dAngle == 270)
				{
					dShiftX = pInspAlgoAlign->m_dShiftY;
					dShiftY = pInspAlgoAlign->m_dShiftX;
				}

				if (fabs(sRstAlgo->m_dOffset_x) <= dShiftX)
					sRstAlgo->m_bOKShiftX = TRUE;
				if (fabs(sRstAlgo->m_dOffset_y) <= dShiftY)
					sRstAlgo->m_bOKShiftY = TRUE;

				if (!sRstAlgo->m_bOKShiftX || !sRstAlgo->m_bOKShiftY)
					bResult = FALSE;
			}
			else
			{
				sRstAlgo->m_dOffset_x = 0.;
				sRstAlgo->m_dOffset_y = 0.;

				sRstAlgo->m_bOKShiftX = TRUE;
				sRstAlgo->m_bOKShiftY = TRUE;
			}

			if (pInspAlgoAlign->m_bUseAngle)
			{
				sRstAlgo->m_dTheta += pInspAlgoAlign->m_dFiduAngle;
				if (fabs(sRstAlgo->m_dTheta) <= pInspAlgoAlign->m_dAngle)
					sRstAlgo->m_bOKAngle = TRUE;
				else
					bResult = FALSE;
			}
			else
			{
				sRstAlgo->m_bOKAngle = TRUE;
				sRstAlgo->m_dTheta = 0.0;
			}

			// Body Rect 구하기
			for (int n = 0; n < nSearchNum; n++)
			{
				if (n == 0)
					rcBlobBody = rcBlob[n];
				else
				{
					if (rcBlob[n].left < rcBlobBody.left)
						rcBlobBody.left = rcBlob[n].left;
					if (rcBlob[n].right > rcBlobBody.right)
						rcBlobBody.right = rcBlob[n].right;
					if (rcBlob[n].top < rcBlobBody.top)
						rcBlobBody.top = rcBlob[n].top;
					if (rcBlob[n].bottom > rcBlobBody.bottom)
						rcBlobBody.bottom = rcBlob[n].bottom;
				}
				sRstAlgo->m_rcBodyRect = rcBlobBody;
			}

			sRstAlgo->m_nOKAreaCnt = nOKAreaCnt;

			if ((pInspAlgoAlign->m_byInspOPT & eAlign_1) == eAlign_1 && nOKAreaCnt > 0 && nOKAreaCnt != pInspAlgoAlign->m_nSearchNum)
				bResult = FALSE;
		}
	}
	Delete_1DArray(&ucColorImgDst);
	return bResult;
}

BOOL CPInsp_AlgoAlign::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	PIAL::_RstAlgoAlign pRstAlign;
	PIAL::_AlgoAlign pAlgoAlign;
	InspWrapper->ConvertAlgo((AlgoAlign*)InspAlgo.m_ptrInspAlgoParam, pAlgoAlign);

	if (algoParam.m_bInspection)
	{
		if (algoParam.m_bUSeLeadAlign == FALSE)
			bResult = InspWrapper->m_PInspAlgo->InspAlign(pAlgoAlign, *pImg_buf, *algoParam.m_rcBlobBody, &pRstAlign, vecAlignResult/*,매개가 있어야 하는지 확인..*/);
		else
		{
			vecAlignResult.clear();
			bResult = InspWrapper->m_PInspAlgo->InspAlign(pAlgoAlign, *pImg_buf, *algoParam.m_rcBlobBody, &pRstAlign, vecAlignResult);
		}
	}
	else
		bResult = InspWrapper->m_PInspAlgo->TeachAlign(pAlgoAlign, *pImg_buf/*,매개가 있어야 하는지 확인..*/);
	InspWrapper->ConvertRstAlgo(pRstAlign, (RstAlgoAlign*)sRstAlgo);

	return bResult;
}


BOOL CPInsp_AlgoAlign::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	RstAlgoAlign * rst = (RstAlgoAlign *)sRstAlgo;
	pAlignRes->offsetX = -rst->m_dOffset_x;
	pAlignRes->offsetY = rst->m_dOffset_y;
	pAlignRes->theta = rst->m_dTheta;
	bRet = TRUE;

	return bRet;
}
int CPInsp_AlgoAlign::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;
	AlgoAlign * pInspAlgo = (AlgoAlign *)sInspAlgo.m_ptrInspAlgoParam;

	nData = pInspAlgo->m_sAlgoColorBase.GetColorData();

	return nData;
}

BOOL CPInsp_AlgoAlign::TeachAlign(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, UCHAR* ucArrDstImg/*=NULL*/)
{
	CPInsp_Algo* m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	CProc3D m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	CProcMil *m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	double m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	double m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
	BOOL  bResult = FALSE;

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	if (sInspAlgo.m_eAlgoType != eAlgoAlign)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	AlgoAlign * pInspAlgoAlign = (AlgoAlign *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoAlign->m_bInsp2D && !pInspAlgoAlign->m_bInsp3D && !pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor)
		return bResult;

	if (ucArrDstImg)
	{
		IppStatus sts;
		IppiSize iSize = { nImgWidth, nImgHeight };
		sts = ippiCopy_8u_C1R(pucImgSrc, nImgWidth, ucArrDstImg, nImgWidth, iSize);

		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Origin2D.bmp"));
	}

	int nSearchMargin = pInspAlgoAlign->m_nSearchMargin;
	int nSearchSizeX[4];
	int nSearchSizeY[4];
	for (int nRoiIdx = 0; nRoiIdx < 4; nRoiIdx++)
	{
		nSearchSizeX[nRoiIdx] = pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cx + nSearchMargin * 2;
		nSearchSizeY[nRoiIdx] = pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cy + nSearchMargin * 2;
		if (nSearchSizeX[nRoiIdx] > nImgWidth)
			nSearchSizeX[nRoiIdx] = nImgWidth - 2;
		if (nSearchSizeY[nRoiIdx] > nImgHeight)
			nSearchSizeY[nRoiIdx] = nImgHeight - 2;
	}
	UCHAR *ucColorImgDst = NULL;
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (pColorTeach != NULL && pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		pColorTeach->GetColorBaseBin(&pInspAlgoAlign->m_sAlgoColorBase, sInspImageData, ucColorImgDst, true, 0, 0, -1, sWndAlgoImg.m_nLight_index);
		m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nImgWidth, nImgHeight, _T("OrgColor_Image.bmp"));
	}
	// 	UCHAR * ucArrSearchDst = NULL;
	// 	if(ucArrDstImg)
	// 	{
	// 		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearchDst, nSearchSizeX * nSearchSizeY);
	// 	}
	double sum_x = 0, sum_y = 0;
	double * insp_x = NULL, *insp_y = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &insp_x, pInspAlgoAlign->m_nSearchNum);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &insp_y, pInspAlgoAlign->m_nSearchNum);

	POINTF rcDstPoint[4];
	int nLeft[4], nTop[4];
	for (int n = 0; n < pInspAlgoAlign->m_nSearchNum; n++)
	{
		if (m_pCPInsp_Algo->AnglePointChange(nImgWidth, nImgHeight, pInspAlgoAlign->m_sArrSearchPoint[n], &rcDstPoint[n]) == FALSE)
			continue;

		nLeft[n] = RounD(rcDstPoint[n].x - nSearchSizeX[n] / 2.);
		nTop[n] = RounD(rcDstPoint[n].y - nSearchSizeY[n] / 2.);
		if (nLeft[n] < 0)
			nLeft[n] = 1;
		if (nTop[n] < 0)
			nTop[n] = 1;
		if (nLeft[n] + nSearchSizeX[n] > nImgWidth)
			nSearchSizeX[n] = nImgWidth - nLeft[n] - 4;
		if (nTop[n] + nSearchSizeY[n] > nImgHeight)
			nSearchSizeY[n] = nImgHeight - nTop[n] - 4;
	}
	double dBlobWidth[4];
	double dBlobHeight[4];
	double dBlobCnt = 0;
	double dBlobSumWidth = 0;
	double dBlobSumHeight = 0;
	double dBlobAvgWidth = 0;
	double dBlobAvgHeight = 0;
	int nSearchSizeSumX = 0;
	int nSearchSizeSumY = 0;
	int nSearchSizeAvgX = 0;
	int nSearchSizeAvgY = 0;
	BOOL bBlobExist[4];
	memset(dBlobWidth, 0.0, sizeof(double) * 4);
	memset(dBlobHeight, 0.0, sizeof(double) * 4);
	memset(bBlobExist, FALSE, sizeof(BOOL) * 4);
	for (int n = 0; n < pInspAlgoAlign->m_nSearchNum; n++)
	{
		UCHAR* ucArrSearch = NULL;
		float* fArrSearch = NULL;
		UCHAR* ucArrSearchColor = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearch, nSearchSizeX[n] * nSearchSizeY[n]);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrSearch, nSearchSizeX[n] * nSearchSizeY[n]);
		if (pColorTeach != NULL && pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearchColor, nSearchSizeX[n] * nSearchSizeY[n]);
		}

		if (sWndAlgoImg.m_nLight_index == (int)eMSCN_COAXIAL)
			m_proc3d.GetCropZmap_LT(pfImgSrc, fArrSearch, nImgWidth, nImgHeight, nLeft[n], nTop[n], nSearchSizeX[n], nSearchSizeY[n]);
		m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, ucArrSearch, nLeft[n], nTop[n], nSearchSizeX[n], nSearchSizeY[n]);
		m_pProcMilAlgo->SaveWorkImg_float(fArrSearch, nSearchSizeX[n], nSearchSizeY[n], _T("Align 3D.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ucArrSearch, nSearchSizeX[n], nSearchSizeY[n], _T("Align 2D.bmp"));
		if (pColorTeach != NULL && pInspAlgoAlign->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			m_pProcMilAlgo->GetClipImage_LT(ucColorImgDst, sInspImageData.nImageSizeX, sInspImageData.nImageSizeY, ucArrSearchColor, nLeft[n], nTop[n], nSearchSizeX[n], nSearchSizeY[n]);
			m_pProcMilAlgo->SaveWorkImg(ucArrSearchColor, nSearchSizeX[n], nSearchSizeY[n], _T("Align Color.bmp"));
		}

		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;

		double cx = 0, cy = 0;
		AlgoBlob algoBlob = m_pCPInsp_Algo->SetAlgoBlob(sInspAlgo);
		algoBlob.m_nTypeSelectBlob = eSelectCenter;
		int nMinBlobArea = (pInspAlgoAlign->m_sArrSearchSize[n].cx * pInspAlgoAlign->m_sArrSearchSize[n].cy) / 10;
		if (nMinBlobArea < pInspAlgoAlign->m_nMinBlobArea)
			nMinBlobArea = pInspAlgoAlign->m_nMinBlobArea;
		//CRect *cBlobRect = new CRect();
		CRect *cBlobRect = g_pMManager->pem_new<CRect>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		int nBlobNum = m_pCPInsp_Algo->BlobImageStruct(algoBlob, ucArrSearch, fArrSearch, ucArrSearchColor, nSearchSizeX[n], nSearchSizeY[n], nMinBlobArea, NULL, &cx, &cy, cBlobRect, NULL, stTieAreaNULL, algoBlob.m_bFillHole);
		if (nBlobNum == 0)
		{
			bResult = FALSE;
			g_pMManager->pem_delete(cBlobRect, false);
			continue;
		}
		bResult = TRUE;

		dBlobWidth[n] = (cBlobRect->right - cBlobRect->left) + 1;
		dBlobHeight[n] = (cBlobRect->bottom - cBlobRect->top) + 1;
		dBlobCnt++;
		dBlobSumWidth += dBlobWidth[n];
		dBlobSumHeight += dBlobHeight[n];
		nSearchSizeSumX += nSearchSizeX[n];
		nSearchSizeSumY += nSearchSizeY[n];
		bBlobExist[n] = TRUE;
		pInspAlgoAlign->m_sArrSearchPoint[n].x = (cx + nLeft[n]) - (nImgWidth / 2.0) + sWndAlgoImg.m_fPartRoundingErrX;
		pInspAlgoAlign->m_sArrSearchPoint[n].y = (cy + nTop[n]) - (nImgHeight / 2.0) + sWndAlgoImg.m_fPartRoundingErrY;
		//delete cBlobRect;
		g_pMManager->pem_delete(cBlobRect, false);

		if (ucArrSearch)
			Delete_1DArray(&ucArrSearch);
		if (fArrSearch)
			Delete_1DArray(&fArrSearch);
		if (ucArrSearchColor)
			Delete_1DArray(&ucArrSearchColor);
	}
	if (bResult == TRUE)
	{
		nSearchSizeAvgX = RounD(nSearchSizeSumX / dBlobCnt);
		nSearchSizeAvgY = RounD(nSearchSizeSumY / dBlobCnt);
		dBlobAvgWidth = RounD(dBlobSumWidth / dBlobCnt);
		dBlobAvgHeight = RounD(dBlobSumHeight / dBlobCnt);
		for (int nRoiIdx = 0; nRoiIdx < 4; nRoiIdx++)
		{
			if (pInspAlgoAlign->m_bSameSize == TRUE)
			{
				pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cx = dBlobAvgWidth;
				pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cy = dBlobAvgHeight;
			}
			if (pInspAlgoAlign->m_bSameSize == FALSE && bBlobExist[nRoiIdx] == TRUE)
			{
				pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cx = RounD(dBlobWidth[nRoiIdx]);
				pInspAlgoAlign->m_sArrSearchSize[nRoiIdx].cy = RounD(dBlobHeight[nRoiIdx]);
			}
		}

		int nMarginValueX = (nSearchSizeAvgX - dBlobAvgWidth) / 2;
		int nMarginValueY = (nSearchSizeAvgY - dBlobAvgHeight) / 2;
		if (nMarginValueX < 0)
			nMarginValueX *= -1;
		if (nMarginValueY < 0)
			nMarginValueY *= -1;

		if (nMarginValueX < nMarginValueY)
			pInspAlgoAlign->m_nSearchMargin = nMarginValueX;
		else
			pInspAlgoAlign->m_nSearchMargin = nMarginValueY;
	}

	if (ucColorImgDst)
		Delete_1DArray(&ucColorImgDst);
	// 	if(ucArrSearchDst)
	// 		Delete_1DArray(&ucArrSearchDst);
	if (insp_x)
		Delete_1DArray(&insp_x);
	if (insp_y)
		Delete_1DArray(&insp_y);

	return bResult;
}

double CPInsp_AlgoAlign::Calc_Theta(double x1, double y1, double x2, double y2, double insp_x1, double insp_y1, double insp_x2, double insp_y2)
{
	double dTheta = 0;

	double dDeltaX1 = 0.0, dDeltaY1 = 0.0, dDeltaX2 = 0.0, dDeltaY2 = 0.0;
	double dRad1 = 0.0, dRad2 = 0.0;

	dDeltaX1 = x2 - x1;
	dDeltaY1 = y1 - y2;

	dRad1 = (double)atan2(dDeltaY1, dDeltaX1);     //티칭된 각도(radian)

	dDeltaX2 = insp_x2 - insp_x1;
	dDeltaY2 = insp_y1 - insp_y2;

	dRad2 = (double)atan2(dDeltaY2, dDeltaX2);    //검사한 각도(radian)

	double temp = dRad2 - dRad1;

	if (temp >= -PI)
	{
		while (temp >= PI)
		{
			temp -= PI * 2.0;
		}
	}
	else
	{
		while (temp < -PI)
		{
			temp += PI * 2.0;
		}
	}

	dTheta = (temp * 180) / PI;

	return dTheta;
}

bool CPInsp_AlgoAlign::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}
BOOL CPInsp_AlgoAlign::TeachLeadAlign(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoAlign *sRstAlgo, InspAlgoParam stAlgoParam)
{
	CProcMil *m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	if (!m_pProcMilAlgo || !sRstAlgo || sInspAlgo.m_eAlgoType != eAlgoAlign)
		return FALSE;
	memset(sRstAlgo, 0, (sizeof(RstAlgoAlign)));
	sRstAlgo->Init();

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nPartW = sWndAlgoImg.m_nWidth3D;
	int nPartH = sWndAlgoImg.m_nHeight3D;
	int nSrcW = sWndAlgoImg.m_nWidth;
	int nSrcH = sWndAlgoImg.m_nHeight;
	int nA = sWndAlgoImg.dAngle;
	if (nSrcW <= 0 || nSrcH <= 0)
		return FALSE;

	cv::Mat imgSrc2D(nSrcH, nSrcW, CV_8UC1, pucImgSrc);
	cv::Mat imgSrc3D(nSrcH, nSrcW, CV_32FC1, pfImgSrc);
	AlgoAlign * pAlgo = (AlgoAlign *)sInspAlgo.m_ptrInspAlgoParam;

	double dShiftX = pAlgo->m_dShiftX / m_resolX / 2;
	double dShiftY = pAlgo->m_dShiftY / m_resolY / 2;
	if (dShiftX < 5) dShiftX = 5;
	if (dShiftY < 5) dShiftY = 5;
	int nSearchMargin = pAlgo->m_nSearchMargin;
	int nSearchSizeX = pAlgo->m_sArrSearchSize[0].cx + nSearchMargin * 2;
	int nSearchSizeY = pAlgo->m_sArrSearchSize[0].cy + nSearchMargin * 2;
	if (nSearchSizeX > nSrcW) nSearchSizeX = nSrcW - 2;
	if (nSearchSizeY > nSrcH) nSearchSizeY = nSrcH - 2;
	if (nSearchSizeX < 3 || nSearchSizeY < 3)
		return FALSE;

	const int nSearchNumMAx = 4;
	int nDir = stAlgoParam.m_nWndDir; // e_LEFT e_RIGHT e_TOP e_BOTTOM
	int nSearchNum = pAlgo->m_nSearchNum;
	POINTF rcSrcPoint[nSearchNumMAx];
	for (int a = 0; a < nSearchNum; a++)
	{
		if (m_pCPInsp_Algo->AnglePosChange(nA, nPartW, nPartH, pAlgo->m_sArrSearchPoint[a], &rcSrcPoint[a]) == FALSE)
			return FALSE;
	}

	int nMinBlobArea = (pAlgo->m_sArrSearchSize[0].cx * pAlgo->m_sArrSearchSize[0].cy) / 10;
	if (nMinBlobArea < pAlgo->m_nMinBlobArea) nMinBlobArea = pAlgo->m_nMinBlobArea;
	if (nMinBlobArea < 10) nMinBlobArea = 10;

	cv::Mat imgBin(nSrcH, nSrcW, CV_8UC1, cv::Scalar(0));
	if (pAlgo->m_bInsp3D && pAlgo->m_bInsp2D)
		m_pCPInsp_Algo->CPInsp::Binarize(imgSrc2D, imgSrc3D, imgBin, pAlgo->m_nTypeRange2D, pAlgo->m_nMinBinary, pAlgo->m_nMaxBinary, pAlgo->m_nTypeRange3D, pAlgo->m_dHeightRateMin, pAlgo->m_dHeightRateMax);
	else if (pAlgo->m_bInsp2D)
		m_pCPInsp_Algo->CPInsp::Binarize(imgSrc2D, imgBin, pAlgo->m_nTypeRange2D, pAlgo->m_nMinBinary, pAlgo->m_nMaxBinary);
	else if (pAlgo->m_bInsp3D)
		m_pCPInsp_Algo->CPInsp::Binarize(imgSrc3D, imgBin, pAlgo->m_nTypeRange3D, pAlgo->m_dHeightRateMin, pAlgo->m_dHeightRateMax, TRUE);
	else
		return FALSE;

	cv::Mat imgBlob(nSrcH, nSrcW, CV_8UC1, cv::Scalar(0));
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imgBin.data, imgBlob.data, nSrcW, nSrcH, nMinBlobArea, FALSE, pAlgo->m_bFillHole, 0, eSelectMix);
	if (nCntBlob < 2)
		return FALSE;

	WndInfo* sWndInfo = stAlgoParam.m_sWndInfo;
	float dClipX2D = ((sWndInfo->m_dPartWidth / 2.) + sWndInfo->dCenterX - (sWndInfo->dWidth / 2.)) / m_resolX;
	float dClipY2D = ((sWndInfo->m_dPartHeight / 2.) - sWndInfo->dCenterY - (sWndInfo->dLength / 2.)) / m_resolY;
	int nStartX = RounD(dClipX2D);
	int nStartY = RounD(dClipY2D);
	if (nStartX < 0) nStartX = 0;
	if (nStartY < 0) nStartY = 0;

	double *dArrA = NULL;
	double *dArrX = NULL;
	double *dArrY = NULL;
	CRect *rcArrBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrA, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrX, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrY, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &rcArrBlob, nCntBlob);
	memset(dArrA, 0, sizeof(double) * nCntBlob);
	memset(dArrX, 0, sizeof(double) * nCntBlob);
	memset(dArrY, 0, sizeof(double) * nCntBlob);
	memset(rcArrBlob, 0, sizeof(CRect) * nCntBlob);
	m_pProcMilAlgo->GetBlobResult_ALL(dArrA, dArrX, dArrY, rcArrBlob);

	int nTotalCnt = 0;
	double dTotalSum = 0;
	double dTotalSumX = 0;
	double dTotalSumY = 0;
	double dTotalSumW = 0;
	double dTotalSumH = 0;
	POINTF poRstPoint[nSearchNumMAx];
	for (int a = 0; a < nSearchNum; a++)
	{
		int nMin = -1;
		float fMin = nSrcW * nSrcH;
		float fMax = 0;
		float fGapMin = nSrcW * nSrcH;
		poRstPoint[a].x = 0;
		poRstPoint[a].y = 0;
		for (int b = 0; b < nCntBlob; b++)
		{
			float fRst = dArrX[b];
			float fRst2 = dArrY[b];
			float fPos = rcSrcPoint[a].x - nStartX;
			double dShift = dShiftX;
			if (nDir == e_LEFT || nDir == e_RIGHT)
			{
				fRst = dArrY[b];
				fRst2 = dArrX[b];
				fPos = rcSrcPoint[a].y - nStartY;
				dShift = dShiftY;
			}
			if (fRst <= 0 || fRst2 <= 0 || fPos <= 0)
				continue;
			if (rcArrBlob[b].Width() <= 3 || rcArrBlob[b].Height() <= 3)
				continue;

			float fGap = fPos - fRst;
			if (fGap < 0) fGap *= -1.f;
			if (fGap > dShift)
				continue;

			if (fGapMin >= fGap || 5 > fGap)
				fGapMin = fGap;
			else
				continue;

			if (nDir == e_LEFT || nDir == e_TOP)
			{
				if (nMin == -1 || fRst2 > fMax)
				{
					fMax = fRst2;
					nMin = b;
				}
			}
			else
			{
				if (nMin == -1 || fRst2 < fMin)
				{
					fMin = fRst2;
					nMin = b;
				}
			}
		}
		if (nMin < 0)
			continue;

		float fSum = 0;
		int nCnt = 0;
		for (int y = rcArrBlob[nMin].top; y <= rcArrBlob[nMin].bottom && y < nSrcH && y >= 0; y++)
		{
			for (int x = rcArrBlob[nMin].left; x <= rcArrBlob[nMin].right && x < nSrcW && x >= 0; x++)
			{
				int nIndex = (y * nSrcW) + x;
				if (imgBlob.data[nIndex] == 0)
					continue;
				fSum += pfImgSrc[nIndex];
				nCnt++;
			}
		}
		if (nCnt == 0)
			continue;

		dTotalSum += (fSum / (float)nCnt);
		dTotalSumX += dArrX[nMin];
		dTotalSumY += dArrY[nMin];
		dTotalSumW += rcArrBlob[nMin].Width();
		dTotalSumH += rcArrBlob[nMin].Height();
		poRstPoint[a].x = dArrX[nMin];
		poRstPoint[a].y = dArrY[nMin];
		nTotalCnt++;
	}

	if (nTotalCnt == nSearchNum)
	{
		sRstAlgo->m_dOffset_x = (((dTotalSumX / (double)nTotalCnt) + nStartX) - (nPartW / 2.0)) * m_resolX;
		sRstAlgo->m_dOffset_y = (((dTotalSumY / (double)nTotalCnt) + nStartY) - (nPartH / 2.0)) * m_resolY;
		sRstAlgo->m_dTheta = dTotalSum / (double)nTotalCnt;
		sRstAlgo->m_poDrawCenter.x = (dTotalSumW / (double)nTotalCnt) * m_resolX;
		sRstAlgo->m_poDrawCenter.y = (dTotalSumH / (double)nTotalCnt) * m_resolY;
		for (int a = 0; a < nSearchNum; a++)
		{
			float fCX = ((poRstPoint[a].x + nStartX) - (nPartW / 2.0)) * m_resolX;
			float fCY = ((poRstPoint[a].y + nStartY) - (nPartH / 2.0)) * m_resolY;
			sRstAlgo->m_poCenter[a].x = fCX;
			sRstAlgo->m_poCenter[a].y = fCY;
		}
		sRstAlgo->m_bOKAngle = TRUE;
	}

	Delete_1DArray(&dArrA);
	Delete_1DArray(&dArrX);
	Delete_1DArray(&dArrY);
	Delete_1DArray(&rcArrBlob);
	return nTotalCnt == nSearchNum;
}