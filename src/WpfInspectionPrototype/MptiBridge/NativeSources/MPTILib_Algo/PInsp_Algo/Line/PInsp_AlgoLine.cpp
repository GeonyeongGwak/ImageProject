#include "PInsp_AlgoLine.h"


CPInsp_AlgoLine::CPInsp_AlgoLine(void)
{
}


CPInsp_AlgoLine::~CPInsp_AlgoLine(void)
{
}

void CPInsp_AlgoLine::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoLine::GetInspAlgoData()
{
	return eSPCAlgoLine;
}

int CPInsp_AlgoLine::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoLine * rstAlgo = (RstAlgoLine *)vRstInspAlgo;
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
	if (!rstAlgo->m_bOKWidth)
	{
		eWholeNgTypeTemp[TypeLength] = e_NG;
		nCurrentNgType = TypeLength;
	}
	if (!rstAlgo->m_bOKPerpendicular)
	{
		eWholeNgTypeTemp[TypeMountWorng] = e_NG;
		nCurrentNgType = TypeMountWorng;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoLine::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoLine::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspLine(sInspAlgo, *stAlgoParam.m_sPartAlgoImg, sWndAlgoImg, *sInspImageData, (RstAlgoLine *)sRstAlgo, stTieArea, ucArrDstImg, stAlgoParam.m_nStartX, stAlgoParam.m_nStartY);

	RstAlgoLine * rst = (RstAlgoLine *)sRstAlgo;
	if (stAlgoParam.m_bInspection == TRUE)
	{
		rst->m_poDrawCenter.x += stAlgoParam.m_dx;
		rst->m_poDrawCenter.y = stAlgoParam.m_dInspH - (rst->m_poDrawCenter.y + stAlgoParam.m_dy);
		for (int nIdx = 0; nIdx < 2; nIdx++)
		{
			rst->m_poDrawLine[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine[nIdx].y += stAlgoParam.m_dy;
			rst->m_poDrawLine_T[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine_T[nIdx].y += stAlgoParam.m_dy;
			rst->m_poDrawLine_2[nIdx].x += stAlgoParam.m_dx;
			rst->m_poDrawLine_2[nIdx].y += stAlgoParam.m_dy;
		}
		for (int nIdx = 0; nIdx < 50; nIdx++)
		{
			rst->m_rcDrawWidth[nIdx].left += stAlgoParam.m_dx;
			rst->m_rcDrawWidth[nIdx].right += stAlgoParam.m_dx;
			rst->m_rcDrawWidth[nIdx].top += stAlgoParam.m_dy;
			rst->m_rcDrawWidth[nIdx].bottom += stAlgoParam.m_dy;
		}
	}
	else
	{
		for (int a = 0; a < 50; a++)
		{
			if (a < rst->m_byWidthCnt)
			{
				rst->m_rcDrawWidth[a].left += stAlgoParam.m_nStartX;
				rst->m_rcDrawWidth[a].right += stAlgoParam.m_nStartX;
				rst->m_rcDrawWidth[a].top += stAlgoParam.m_nStartY;
				rst->m_rcDrawWidth[a].bottom += stAlgoParam.m_nStartY;
			}
			else
			{
				rst->m_rcDrawWidth[a].left = 0;
				rst->m_rcDrawWidth[a].right = 0;
				rst->m_rcDrawWidth[a].top = 0;
				rst->m_rcDrawWidth[a].bottom = 0;
			}
		}
	}
	return bResult;
}

double CPInsp_AlgoLine::InspectionCrossLine(UCHAR *ucArrSrcImg, int nWndW, int nWndH, int nMeasureDirection, bool bUseFixPos, int nCrossOpt, POINTF* poDrawLine, double* dRstAngle)
{
	double dReturn = 0;
	if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
		return dReturn;
	if (nWndW <= 0 || nWndH <= 0 || ucArrSrcImg == NULL)
		return dReturn;

	int nStartX = 0;
	int nStartY = 0;
	int nEndX = 0;
	int nEndY = 0;
	double dLengthBuf = nWndW * nWndH;
	/*double* dptrArrLength = new double[nWndW];
	int* nptrArrYList = new int[nWndW];*/
	double* dptrArrLength = g_pMManager->pem_new<double>(true, nWndW, (PCHAR)__FUNCTION__, __LINE__);
	int* nptrArrYList = g_pMManager->pem_new<int>(true, nWndW, (PCHAR)__FUNCTION__, __LINE__);
	memset(dptrArrLength, 0, sizeof(double) * nWndW);
	memset(nptrArrYList, 0, sizeof(int) * nWndW);
	if (nMeasureDirection == TL_Line || nMeasureDirection == TR_Line)
	{
		nStartX = 0;
		nStartY = 0;
		if (nMeasureDirection == TR_Line)
		{
			nStartX = nWndW;
			nStartY = 0;
		}
		if (bUseFixPos == false)
		{
			int nStartXBuf = nStartX;
			int nStartYBuf = nStartY;
			for (int nX = 0; nX < nWndW; nX++)
			{
				for (int nY = 0; nY < nWndH; nY++)
				{
					if (ucArrSrcImg[(nY * nWndW) + nX] == 255)
					{
						double dLength = sqrt(pow((double)(nX - nStartXBuf), 2) + pow((double)(nY - nStartYBuf), 2));
						if (dLengthBuf > dLength)
						{
							dLengthBuf = dLength;
							nStartX = nX;
							nStartY = nY;
						}
						break;
					}
				}
			}
		}
		for (int nX = 0; nX < nWndW; nX++)
		{
			for (int nY = nWndH - 1; nY > 0; nY--)
			{
				if (ucArrSrcImg[(nY * nWndW) + nX] == 255)
				{
					nptrArrYList[nX] = nY;
					dptrArrLength[nX] = sqrt(pow((double)(nX - nStartX), 2) + pow((double)(nY - nStartY), 2));
					break;
				}
			}
		}
	}
	else if (nMeasureDirection == BL_Line || nMeasureDirection == BR_Line)
	{
		nStartX = 0;
		nStartY = nWndH;
		if (nMeasureDirection == BR_Line)
		{
			nStartX = nWndW;
			nStartY = nWndH;
		}
		if (bUseFixPos == false)
		{
			int nStartXBuf = nStartX;
			int nStartYBuf = nStartY;
			for (int nX = 0; nX < nWndW; nX++)
			{
				for (int nY = nWndH; nY > 0; nY--)
				{
					if (ucArrSrcImg[(nY * nWndW) + nX] == 255)
					{
						double dLength = sqrt(pow((double)(nX - nStartXBuf), 2) + pow((double)(nY - nStartYBuf), 2));
						if (dLengthBuf > dLength)
						{
							dLengthBuf = dLength;
							nStartX = nX;
							nStartY = nY;
						}
						break;
					}
				}
			}
		}
		for (int nX = 0; nX < nWndW; nX++)
		{
			for (int nY = 0; nY < nWndH; nY++)
			{
				if (ucArrSrcImg[(nY * nWndW) + nX] == 255)
				{
					nptrArrYList[nX] = nY;
					dptrArrLength[nX] = sqrt(pow((double)(nX - nStartX), 2) + pow((double)(nY - nStartY), 2));
					break;
				}
			}
		}
	}

	// nCrossOpt : 0 Min 1 Center 2 Max 3 Avg
	if (nCrossOpt == 0)
		dReturn = nWndW * nWndH;
	else if (nCrossOpt == 1 || nCrossOpt == 3)
	{
		nEndX = nWndW / 2;
		nEndY = nptrArrYList[nEndX];
	}
	for (int n = 0; n < nWndW; n++)
	{
		if (dptrArrLength[n] <= 0)
			continue;
		if (nCrossOpt == 0)
		{
			if (dReturn > dptrArrLength[n])
			{
				dReturn = dptrArrLength[n];
				nEndX = n;
				nEndY = nptrArrYList[n];
			}
		}
		else if (nCrossOpt == 1)
		{
			dReturn = dptrArrLength[nEndX];
			break;
		}
		else if (nCrossOpt == 2)
		{
			if (dptrArrLength[n] > dReturn)
			{
				dReturn = dptrArrLength[n];
				nEndX = n;
				nEndY = nptrArrYList[n];
			}
		}
		else if (nCrossOpt == 3)
		{
			dReturn += dptrArrLength[n];
			if (dReturn > dptrArrLength[n])
				dReturn /= 2.0;
		}
	}
	if (dReturn < 0)	dReturn = 0;
	if (nStartX < 0)	nStartX = 0;
	if (nStartY < 0)	nStartY = 0;
	if (nEndX < 0)	nEndX = 0;
	if (nEndY < 0)	nEndY = 0;
	if (nStartX > nWndW)	nStartX = nWndW - 1;
	if (nStartY > nWndH)	nStartY = nWndH - 1;
	if (nEndX > nWndW)	nEndX = nWndW - 1;
	if (nEndY > nWndH)	nEndY = nWndH - 1;

	double dXDist = sqrt(pow((double)(nEndX - nStartX), 2));
	double dYDist = sqrt(pow((double)(nEndY - nStartY), 2));
	*dRstAngle = 0;
	if (dXDist > 0 && dYDist > 0)
		*dRstAngle = atan(dYDist / dXDist) * 180 / PI;
	poDrawLine[0].x = nStartX;
	poDrawLine[0].y = nStartY;
	poDrawLine[1].x = nEndX;
	poDrawLine[1].y = nEndY;
	Delete_1DArray(&dptrArrLength);
	Delete_1DArray(&nptrArrYList);
	return dReturn;
}
void CPInsp_AlgoLine::CuttingPlaneLine(AlgoLine algoLine, UCHAR *ucArrDst, int nImgWidth, int nImgHeight, bool bIsHorizon, int nDir, RstAlgoLine * sRstAlgo)
{
	if (sRstAlgo == NULL) return;
	sRstAlgo->m_bOKWidth = TRUE;
	sRstAlgo->m_byWidthCnt = 0;
	sRstAlgo->m_dRstAngle = 0;
	memset(sRstAlgo->m_poDrawLine, 0, sizeof(POINTF) * 2);
	memset(sRstAlgo->m_dRstCuttinhW, 0, sizeof(double) * 50);
	memset(sRstAlgo->m_dRstCuttinhL, 0, sizeof(double) * 50);
	memset(sRstAlgo->m_rcDrawWidth, 0, sizeof(RECT) * 50);
	if (algoLine.m_bTeachWidthUse == FALSE)
		return;
	if ((ucArrDst == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0)) return;
	m_pProcMilAlgo->SaveWorkImg(ucArrDst, nImgWidth, nImgHeight, _T("ORG_CuttingPlaneLine.bmp"));
	int nCPW = RounD((algoLine.m_dTeachWidth / 1000.0) / m_resolY);
	int nCPL = RounD((algoLine.m_dTeachWidthRateMin / 1000.0) / m_resolX);
	if (bIsHorizon == false)
	{
		nCPW = ((algoLine.m_dTeachWidth / 1000.0) / m_resolX);
		nCPL = ((algoLine.m_dTeachWidthRateMin / 1000.0) / m_resolY);
	}
	int nW = (bIsHorizon) ? nImgWidth : nImgHeight;
	int nH = (bIsHorizon) ? nImgHeight : nImgWidth;
	int nStX = 0;
	int nStY = 0;
	bool bUseROI = false;
	if (algoLine.m_bUseInspROI && algoLine.m_rcInspROI.right - algoLine.m_rcInspROI.left > 0)
	{
		bUseROI = true;
		RECT rcInspROI = algoLine.m_rcInspROI;
		//m_pCPInsp_Algo->AngleRectChange(0, nImgWidth, nImgHeight, algoLine.m_rcInspROI, &rcInspROI);
		if (bIsHorizon)
		{
			nStX = rcInspROI.left;
			nStY = rcInspROI.top;
			nW = rcInspROI.right - rcInspROI.left;
			nH = rcInspROI.bottom - rcInspROI.top;
		}
		else
		{
			nStX = rcInspROI.top;
			nStY = rcInspROI.left;
			nW = rcInspROI.bottom - rcInspROI.top;
			nH = rcInspROI.right - rcInspROI.left;
		}
	}

	int nCnt = 0;
	double *dArrX = NULL;
	double *dArrY = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrX, nW);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrY, nW);
	memset(dArrX, 0, sizeof(double) * nW);
	memset(dArrY, 0, sizeof(double) * nW);
	bool bBothEnds = algoLine.m_bUseEndPos;
	if (nW < 100) bBothEnds = false;
	for (int x = nStX; x < nStX + nW; x++)
	{
		if (nDir == Right_Line)
		{
			for (int y = (nStY + nH) - 1; y >= nStY; y--)
			{
				int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
					continue;
				if (ucArrDst[nIndex] != 255) continue;
				dArrX[nCnt] = (bIsHorizon) ? x : y;
				dArrY[nCnt] = (bIsHorizon) ? y : x;
				nCnt++;
				break;
			}
		}
		else
		{
			for (int y = nStY; y < nStY + nH; y++)
			{
				int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
					continue;
				if (ucArrDst[nIndex] != 255) continue;
				dArrX[nCnt] = (bIsHorizon) ? x : y;
				dArrY[nCnt] = (bIsHorizon) ? y : x;
				nCnt++;
				break;
			}
		}
		if (bBothEnds && nCnt >= 5)
			break;
	}
	if (bBothEnds)
	{
		for (int x = nStX + nW - 1; x >= nStX; x--)
		{
			if (nDir == Right_Line)
			{
				for (int y = nStY + nH - 1; y >= nStY; y--)
				{
					int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
						continue;
					if (ucArrDst[nIndex] != 255) continue;
					dArrX[10 - (nCnt % 5 + 1)] = (bIsHorizon) ? x : y;
					dArrY[10 - (nCnt % 5 + 1)] = (bIsHorizon) ? y : x;
					nCnt++;
					break;
				}
			}
			else
			{
				for (int y = nStY; y < nStY + nH; y++)
				{
					int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
						continue;
					if (ucArrDst[nIndex] != 255) continue;
					dArrX[10 - (nCnt % 5 + 1)] = (bIsHorizon) ? x : y;
					dArrY[10 - (nCnt % 5 + 1)] = (bIsHorizon) ? y : x;
					nCnt++;
					break;
				}
			}
			if (nCnt >= 10)
				break;
		}
	}
	if (nCnt <= 1)
	{
		Delete_1DArray(&dArrX);
		Delete_1DArray(&dArrY);
		return;
	}

	nW = (bIsHorizon) ? nImgWidth : nImgHeight;
	nH = (bIsHorizon) ? nImgHeight : nImgWidth;
	double dA = 0, dB = 0;
	double x1 = dArrX[0], y1 = dArrY[0], x2 = dArrX[nCnt - 1], y2 = dArrY[nCnt - 1];
	if (bUseROI)
	{		// ROI 사용 시 기울기 0으로 처리 (위치는 중간위치로)
		if (bIsHorizon)
		{
			x1 = dA = 0;
			x2 = nImgWidth;
			y1 = y2 = dB = (y1 + y2) / 2.0;	
		}
		else
		{
			y1 = dA = 0;
			y2 = nImgHeight;
			x1 = x2 = dB = (x1 + x2) / 2.0;
		}
	}
	else
		sRstAlgo->m_dRstAngle = m_pCPInsp_Algo->GetGradient(dArrX, dArrY, nCnt, dA, dB, bIsHorizon);
	sRstAlgo->m_poDrawLine[0].x = (bIsHorizon) ? x1 : (dA * y1) + dB;
	sRstAlgo->m_poDrawLine[0].y = (bIsHorizon) ? (dA * x1) + dB : y1;
	sRstAlgo->m_poDrawLine[1].x = (bIsHorizon) ? x2 : (dA * y2) + dB;
	sRstAlgo->m_poDrawLine[1].y = (bIsHorizon) ? (dA * x2) + dB : y2;
	Delete_1DArray(&dArrX);
	Delete_1DArray(&dArrY);
	UCHAR *pucCutting = NULL;
	UCHAR *pucCuttingBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucCutting, nImgWidth * nImgHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucCuttingBlob, nImgWidth * nImgHeight);
	memset(pucCutting, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
	memset(pucCuttingBlob, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
	for (int x = 0; x < nW; x++)
	{
		int nS = -1;
		int nY = (dA * x) + dB;
		if (nDir == Right_Line)
		{
			for (int y = nH - 1; y >= 0; y--)
			{
				int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
					continue;
				if (ucArrDst[nIndex] == 0) continue;
				nS = y;
				break;
			}
		}
		else
		{
			for (int y = 0; y < nH; y++)
			{
				int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
					continue;
				if (ucArrDst[nIndex] == 0) continue;
				nS = y;
				break;
			}
		}
		if (nS == -1)
		{
			if (nDir == Right_Line)
			{
				for (int y = 0; y <= nY; y++)
				{
					int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
						continue;
					pucCutting[nIndex] = 255;
				}
			}
			else
			{
				for (int y = nH - 1; y >= nY; y--)
				{
					int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
						continue;
					pucCutting[nIndex] = 255;
				}
			}
			continue;
		}
		int nWidth = nY - nS;
		if (nWidth < 0) nWidth *= -1;
		if (nWidth >= nCPW)
		{
			if (nY > nS)
			{
				for (int y = nS; y <= nY; y++)
				{
					int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
						continue;
					pucCutting[nIndex] = 255;
				}
			}
			else
			{
				for (int y = nY; y <= nS; y++)
				{
					int nIndex = (bIsHorizon) ? (y * nImgWidth) + x : (x * nImgWidth) + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
						continue;
					pucCutting[nIndex] = 255;
				}
			}
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pucCutting, nImgWidth, nImgHeight, _T("CuttingPlaneLine.bmp"));
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pucCutting, pucCuttingBlob, nImgWidth, nImgHeight, 4, FALSE, TRUE, 1, eSelectMix);
	m_pProcMilAlgo->SaveWorkImg(pucCuttingBlob, nImgWidth, nImgHeight, _T("CuttingPlaneLineBlob.bmp"));
	if (nCntBlob > 0)
	{
		CRect *rcArea = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &rcArea, nCntBlob);
		memset(rcArea, 0, sizeof(CRect) * nCntBlob);
		m_pProcMilAlgo->GetForeignBlobResult(NULL, NULL, NULL, rcArea);
		for (int a = 0; a < nCntBlob; a++)
		{
			int nWidth = 0;
			if (bIsHorizon)
				nWidth = rcArea[a].right - rcArea[a].left;
			else
				nWidth = rcArea[a].bottom - rcArea[a].top;
			if (nWidth >= nCPL)
			{
				sRstAlgo->m_rcDrawWidth[sRstAlgo->m_byWidthCnt].left = rcArea[a].left;
				sRstAlgo->m_rcDrawWidth[sRstAlgo->m_byWidthCnt].right = rcArea[a].right;
				sRstAlgo->m_rcDrawWidth[sRstAlgo->m_byWidthCnt].top = rcArea[a].top;
				sRstAlgo->m_rcDrawWidth[sRstAlgo->m_byWidthCnt].bottom = rcArea[a].bottom;
				sRstAlgo->m_byWidthCnt++;
			}
			if (sRstAlgo->m_byWidthCnt >= 50)
				break;
		}
		Delete_1DArray(&rcArea);
	}
	Delete_1DArray(&pucCutting);
	Delete_1DArray(&pucCuttingBlob);
	for (int a = 0; a < sRstAlgo->m_byWidthCnt; a++)
	{
		int nRstW = sRstAlgo->m_rcDrawWidth[a].right - sRstAlgo->m_rcDrawWidth[a].left;
		int nRstL = sRstAlgo->m_rcDrawWidth[a].bottom - sRstAlgo->m_rcDrawWidth[a].top;
		if (bIsHorizon)
		{
			sRstAlgo->m_dRstCuttinhW[a] = nRstL * m_resolY * 1000.0;
			sRstAlgo->m_dRstCuttinhL[a] = nRstW * m_resolX * 1000.0;
		}
		else
		{
			sRstAlgo->m_dRstCuttinhW[a] = nRstW * m_resolX * 1000.0;
			sRstAlgo->m_dRstCuttinhL[a] = nRstL * m_resolY * 1000.0;
		}
	}
	sRstAlgo->m_bOKWidth = (sRstAlgo->m_byWidthCnt == 0);
}
double CPInsp_AlgoLine::PerpendicularLine(UCHAR *ucImage, int nImgWidth, int nImgHeight, int nType, bool bHorizon, double dA, double dB, POINTF* poDrawLine)
{
	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	double dRstDist = 0;
	if ((nType & m_eLineData_Min) == m_eLineData_Min)
		dRstDist = nImgWidth * nImgHeight;
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("PerpendicularLine.bmp"));
	poDrawLine[0].x = 0;
	poDrawLine[0].y = 0;
	poDrawLine[1].x = 0;
	poDrawLine[1].y = 0;
	int nCnt = 0;
	for (int a = 0; a < nW; a++)
	{
		double dY = dA * a + dB;
		int nY = (int)RounD(dY);
		if (nY < 0 || nY > nH) continue;
		int nIndex = (bHorizon) ? nY * nImgWidth + a : a * nImgWidth + nY;
		if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
		if (ucImage[nIndex] == 0) continue;
		double dA2 = 0;
		if (dA != 0) dA2 = -1.0 / dA;
		double dB2 = dY - (dA2 * a);
		int nMinBX = -1, nMinBY = -1, nMaxBX = -1, nMaxBY = -1;
		int nGapDist = 0, nBlobSize = 0;
		for (int a2 = 0; a2 < nH; a2++)
		{
			int nYTotal = 1;
			int nM = 1;
			if (dA2 == 0)
			{
				nYTotal = nH;
				nY = a;
			}
			else
			{
				dY = (a2 - dB2) / dA2;
				nY = (int)RounD(dY);
				if (nY < 0 || nY >= nW) continue;
				double dY2 = ((a2 + 1) - dB2) / dA2;
				int nY2 = (int)RounD(dY2);
				if (nY2 >= 0 && nY2 < nW)
				{
					nYTotal = nY2 - nY;
					if (nYTotal < 0)
					{
						nM = -1;
						nYTotal *= nM;
					}
				}
			}
			if (nYTotal == 0) nYTotal = 1;
			for (int a3 = 0; a3 < nYTotal; a3++)
			{
				nIndex = (bHorizon) ? (a2 + (a3 * nM)) * nImgWidth + nY : nY * nImgWidth + (a2 + (a3 * nM));
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nIndex] == 0)
				{
					if (nMinBX != -1)
						nGapDist += 1;
					continue;
				}
				if (nMinBX == -1 || (nGapDist >= 5 && nBlobSize >= 5))
				{
					nBlobSize = 0;
					nMinBX = (bHorizon) ? nY : (a2 + (a3 * nM));
					nMinBY = (bHorizon) ? (a2 + (a3 * nM)) : nY;
				}
				nGapDist = 0;
				nBlobSize += 1;
				nMaxBX = (bHorizon) ? nY : (a2 + (a3 * nM));
				nMaxBY = (bHorizon) ? (a2 + (a3 * nM)) : nY;
			}
		}
		if (nMinBX < 0 || nMaxBX < 0 || nMinBY < 0 || nMaxBY < 0)
			continue;
		double dDist = sqrt(pow((nMaxBX - nMinBX) * m_resolX, 2) + pow((nMaxBY - nMinBY) * m_resolY, 2));
		if ((nType & m_eLineData_Min) == m_eLineData_Min)
		{
			if (dRstDist > dDist)
			{
				dRstDist = dDist;
				poDrawLine[0].x = nMinBX;
				poDrawLine[0].y = nMinBY;
				poDrawLine[1].x = nMaxBX;
				poDrawLine[1].y = nMaxBY;
				nCnt = 1;
			}
		}
		else if ((nType & m_eLineData_Max) == m_eLineData_Max)
		{
			if (dRstDist < dDist)
			{
				dRstDist = dDist;
				poDrawLine[0].x = nMinBX;
				poDrawLine[0].y = nMinBY;
				poDrawLine[1].x = nMaxBX;
				poDrawLine[1].y = nMaxBY;
				nCnt = 1;
			}
		}
		else
		{
			dRstDist += dDist;
			poDrawLine[0].x += nMinBX;
			poDrawLine[0].y += nMinBY;
			poDrawLine[1].x += nMaxBX;
			poDrawLine[1].y += nMaxBY;
			nCnt++;
		}
	}
	if (nCnt == 0)
	{
		poDrawLine[0].x = 0;
		poDrawLine[0].y = 0;
		poDrawLine[1].x = 0;
		poDrawLine[1].y = 0;
		return 0;
	}
	dRstDist /= (double)nCnt;
	poDrawLine[0].x /= nCnt;
	poDrawLine[0].y /= nCnt;
	poDrawLine[1].x /= nCnt;
	poDrawLine[1].y /= nCnt;
	return dRstDist;
}
BOOL CPInsp_AlgoLine::InspLine(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipImg, InspRoiImgBuf &sInspImageData, RstAlgoLine * sRstAlgo, TotalInspExceptArea stTieArea, UCHAR *ucArrDstImg, int nStartX, int nStartY)
{
	BOOL bReturn = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoLine));
		sRstAlgo->Init();
	}

	if (sInspAlgo.m_eAlgoType != eAlgoLine)
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

	AlgoLine *pInspAlgoLine = (AlgoLine *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoLine)
		return bReturn;
	AlgoBlob algoBlob = m_pCPInsp_Algo->SetAlgoBlob(sInspAlgo);
	double dCX = 0;
	double dCY = 0;
	double dArea = 0;
	int nMinBlobArea = 4;
	CRect rcBlob(0, 0, 0, 0);
	bool bIsDelete = false;
	int nCntBlob = 0;
	bool bIsHorizon = pInspAlgoLine->m_bIsHorizon;
	int nMeasureDirection = pInspAlgoLine->m_nMeasureDirection;
	bool bShiftXUse = pInspAlgoLine->m_bShiftXUse;
	bool bShiftYUse = pInspAlgoLine->m_bShiftYUse;
	double dShiftX = pInspAlgoLine->m_dShiftX;
	double dShiftY = pInspAlgoLine->m_dShiftY;
	double dTechCenterX = pInspAlgoLine->m_dTechCenterX;
	double dTechCenterY = pInspAlgoLine->m_dTechCenterY;
	if (dWndAngle == 90 || dWndAngle == 270)
	{
		bIsHorizon = !pInspAlgoLine->m_bIsHorizon;
		bShiftXUse = pInspAlgoLine->m_bShiftYUse;
		bShiftYUse = pInspAlgoLine->m_bShiftXUse;
		dShiftX = pInspAlgoLine->m_dShiftY;
		dShiftY = pInspAlgoLine->m_dShiftX;
		dTechCenterX = pInspAlgoLine->m_dTechCenterY;
		dTechCenterY = pInspAlgoLine->m_dTechCenterX;
	}
	if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
	{
		bool bAnyAngle = m_pCPInsp_Algo->IsAnyAngle(dWndAngle);
		if (pInspAlgoLine->m_nMeasureDirection == Center_Line || dWndAngle == 0 || bAnyAngle == true)
			nMeasureDirection = pInspAlgoLine->m_nMeasureDirection;
		else
		{
			if (dWndAngle == 90 || dWndAngle == 180)
			{
				if (bIsHorizon == true || dWndAngle == 180)
				{
					if (pInspAlgoLine->m_nMeasureDirection == Left_Line)
						nMeasureDirection = Right_Line;
					else if (pInspAlgoLine->m_nMeasureDirection == Right_Line)
						nMeasureDirection = Left_Line;
				}
				else
					nMeasureDirection = pInspAlgoLine->m_nMeasureDirection;
			}
			if (dWndAngle == 270 || dWndAngle == 180)
			{
				if (bIsHorizon == false || dWndAngle == 180)
				{
					if (pInspAlgoLine->m_nMeasureDirection == Left_Line)
						nMeasureDirection = Right_Line;
					else if (pInspAlgoLine->m_nMeasureDirection == Right_Line)
						nMeasureDirection = Left_Line;
				}
				else
					nMeasureDirection = pInspAlgoLine->m_nMeasureDirection;
			}
		}
	}
	else
	{
		int nChange = 0;
		if (dWndAngle == 90)
			nChange = 3;
		else if (dWndAngle == 180)
			nChange = 2;
		else if (dWndAngle == 270)
			nChange = 1;
		nMeasureDirection = pInspAlgoLine->m_nMeasureDirection + nChange;
		nChange = 0;
		if (nMeasureDirection > BL_Line)
		{
			nChange = nMeasureDirection - BL_Line;
			nMeasureDirection = nChange + Right_Line;
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
	if (pColorTeach != NULL && pInspAlgoLine->m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		bool bTeach = ucArrDstImg != NULL ? true : false;
		pColorTeach->GetColorBaseBin(&pInspAlgoLine->m_sAlgoColorBase, sInspImageData, ucColorImgDst, bTeach, 0, 0, -1, sWndAlgoImg.m_nLight_index);
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
		m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("Line_All.bmp"));
		Delete_1DArray(&ucColorImgDst);
		if (pColorTeach != NULL && pInspAlgoLine->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			UCHAR *ucColorImgBuf = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgBuf, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, nWndWidth * nWndHeight);
			memset(ucColorImgBuf, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			memset(ucColorImgDst, 0, sizeof(UCHAR) * nWndWidth * nWndHeight);
			bool bTeach = ucArrDstImg != NULL ? true : false;
			pColorTeach->GetColorBaseBin(&pInspAlgoLine->m_sAlgoColorBase, sInspImageData, ucColorImgBuf, true, 0, 0, -1, sWndAlgoImg.m_nLight_index);
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
	m_pProcMilAlgo->SaveWorkImg(ucArrDstWnd, nWndWidth, nWndHeight, _T("Line_Blob.bmp"));
	Delete_1DArray(&ucColorImgDst);
	if (ucArrDstImg != NULL)
	{
		int nIndex = 0;
		for (int y = nStartY; y < nStartY + nWndHeight; y++)
		{
			for (int x = nStartX; x < nStartX + nWndWidth; x++)
			{
				int nPos = (y * nImgWidth) + x;
				ucArrDstImg[nPos] = ucArrDstWnd[nIndex];
				nIndex++;
			}
		}
	}
	if (sRstAlgo)
	{
		sRstAlgo->m_poDrawLine[0].x = 0;
		sRstAlgo->m_poDrawLine[0].y = 0;
		sRstAlgo->m_poDrawLine[1].x = 0;
		sRstAlgo->m_poDrawLine[1].y = 0;
		sRstAlgo->m_poDrawLine_T[0].x = dTechCenterX;
		sRstAlgo->m_poDrawLine_T[0].y = 0;
		sRstAlgo->m_poDrawLine_T[1].x = dTechCenterX;
		sRstAlgo->m_poDrawLine_T[1].y = nWndHeight;
		if (bIsHorizon == true)
		{
			sRstAlgo->m_poDrawLine_T[0].x = 0;
			sRstAlgo->m_poDrawLine_T[0].y = dTechCenterY;
			sRstAlgo->m_poDrawLine_T[1].x = nWndWidth;
			sRstAlgo->m_poDrawLine_T[1].y = dTechCenterY;
		}
	}
	if (nCntBlob == 0)
	{
		Delete_1DArray(&ucArrDstWnd);
		sRstAlgo->m_bMissing = TRUE;
		return bReturn;
	}

	double dAngle = 0.0;
	double dA = 0.0;
	double dB = 0.0;
	int nLineLength = 0;
	POINTF poDrawLine[2];
	POINTF poDrawLine2[2];
	POINTF poDrawWidth[2];
	memset(poDrawLine, 0, sizeof(POINTF) * 2);
	memset(poDrawLine2, 0, sizeof(POINTF) * 2);
	memset(poDrawWidth, 0, sizeof(POINTF) * 2);
	int nInspOption = m_eEdgeData_UseExceptAngle;
	if (pInspAlgoLine->m_bTeachWidthUse && sRstAlgo)
	{
		CuttingPlaneLine(*pInspAlgoLine, ucArrDstWnd, nWndWidth, nWndHeight, bIsHorizon, nMeasureDirection, sRstAlgo);
		dAngle = sRstAlgo->m_dRstAngle;
		poDrawLine[0].x = sRstAlgo->m_poDrawLine[0].x;
		poDrawLine[0].y = sRstAlgo->m_poDrawLine[0].y;
		poDrawLine[1].x = sRstAlgo->m_poDrawLine[1].x;
		poDrawLine[1].y = sRstAlgo->m_poDrawLine[1].y;
		double dArrX[2] = { poDrawLine[0].x, poDrawLine[1].x };
		double dArrY[2] = { poDrawLine[0].y, poDrawLine[1].y };
		dAngle = m_pCPInsp_Algo->GetGradient(dArrX, dArrY, 2, dA, dB, bIsHorizon);
	}
	else if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
		bReturn = m_pCPInsp_Algo->InspectionLine(nWndWidth, nWndHeight, ucArrDstWnd, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB, pInspAlgoLine->m_byLineFindType[0], pInspAlgoLine->m_byLineFindType[1], nInspOption);
	else
	{
		double dLength = InspectionCrossLine(ucArrDstWnd, nWndWidth, nWndHeight, nMeasureDirection, pInspAlgoLine->m_bUseFix, pInspAlgoLine->m_nCrossOpt, poDrawLine, &dAngle);
		nLineLength = RounD(dLength);
		double dArrX[2] = { poDrawLine[0].x, poDrawLine[1].x };
		double dArrY[2] = { poDrawLine[0].y, poDrawLine[1].y };
		dAngle = m_pCPInsp_Algo->GetGradient(dArrX, dArrY, 2, dA, dB, true);
	}
	if ((pInspAlgoLine->m_nLineData & m_eLineData_Use) == m_eLineData_Use)
		PerpendicularLine(ucArrDstWnd, nWndWidth, nWndHeight, pInspAlgoLine->m_nLineData, bIsHorizon, dA, dB, poDrawLine2);
	bReturn = TRUE;
	if (sRstAlgo)
	{
		sRstAlgo->m_dRstAngle = dAngle;
		sRstAlgo->m_bMissing = FALSE;
		for (int a = 0; a < 2; a++)
		{
			if (poDrawLine[a].x < 0) poDrawLine[a].x = 0;
			if (poDrawLine[a].y < 0) poDrawLine[a].y = 0;
			if (poDrawLine[a].x > nWndWidth) poDrawLine[a].x = nWndWidth;
			if (poDrawLine[a].y > nWndHeight) poDrawLine[a].y = nWndHeight;
			if (poDrawLine2[a].x < 0) poDrawLine2[a].x = 0;
			if (poDrawLine2[a].y < 0) poDrawLine2[a].y = 0;
			if (poDrawLine2[a].x > nWndWidth) poDrawLine2[a].x = nWndWidth;
			if (poDrawLine2[a].y > nWndHeight) poDrawLine2[a].y = nWndHeight;

			sRstAlgo->m_poDrawLine[a].x = poDrawLine[a].x;
			sRstAlgo->m_poDrawLine[a].y = poDrawLine[a].y;
			sRstAlgo->m_poDrawLine_2[a].x = poDrawLine2[a].x;
			sRstAlgo->m_poDrawLine_2[a].y = poDrawLine2[a].y;
		}
		sRstAlgo->m_dRstLength = sqrt(pow((poDrawLine[0].x - poDrawLine[1].x) * m_resolX, 2) + pow((poDrawLine[0].y - poDrawLine[1].y) * m_resolY, 2));
		sRstAlgo->m_fRstPerpendicular = sqrt(pow((poDrawLine2[0].x - poDrawLine2[1].x) * m_resolX, 2) + pow((poDrawLine2[0].y - poDrawLine2[1].y) * m_resolY, 2));
		float fInspCenterX = (sRstAlgo->m_poDrawLine[0].x + sRstAlgo->m_poDrawLine[1].x) / 2.0f;
		float fInspCenterY = (sRstAlgo->m_poDrawLine[0].y + sRstAlgo->m_poDrawLine[1].y) / 2.0f;
		sRstAlgo->m_dRstShiftX = (dTechCenterX - fInspCenterX) * m_resolX;
		sRstAlgo->m_dRstShiftY = (dTechCenterY - fInspCenterY) * m_resolY;
		sRstAlgo->m_poDrawCenter.x = fInspCenterX;
		sRstAlgo->m_poDrawCenter.y = fInspCenterY;
		if (pInspAlgoLine->m_bUseAngle == TRUE)
		{
			sRstAlgo->m_bOKAngle = TRUE;
			if (sRstAlgo->m_dRstAngle > pInspAlgoLine->m_dTeachRotate || sRstAlgo->m_dRstAngle < (pInspAlgoLine->m_dTeachRotate * -1))
			{
				bReturn = FALSE;
				sRstAlgo->m_bOKAngle = bReturn;
			}
		}
		else
		{
			sRstAlgo->m_bOKAngle = TRUE;
			sRstAlgo->m_dRstAngle = 0;
		}

		if (pInspAlgoLine->m_bShiftIsUse == TRUE)
		{
			sRstAlgo->m_bOKShiftX = TRUE;
			sRstAlgo->m_bOKShiftY = TRUE;
			if (bShiftXUse == TRUE)
			{
				if (sRstAlgo->m_dRstShiftX > dShiftX || sRstAlgo->m_dRstShiftX < (dShiftX * -1))
				{
					bReturn = FALSE;
					sRstAlgo->m_bOKShiftX = bReturn;
				}
			}
			else
				sRstAlgo->m_dRstShiftX = 0;

			if (bShiftYUse == TRUE)
			{
				if (sRstAlgo->m_dRstShiftY > dShiftY || sRstAlgo->m_dRstShiftY < (dShiftY * -1))
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
			sRstAlgo->m_bOKShiftX = TRUE;
			sRstAlgo->m_bOKShiftY = TRUE;
			sRstAlgo->m_dRstShiftX = 0;
			sRstAlgo->m_dRstShiftY = 0;
		}
		if (pInspAlgoLine->m_bTeachLengthUse == TRUE)
		{
			sRstAlgo->m_bOKLength = TRUE;
			double dLengthMin = pInspAlgoLine->m_dTeachLength * (pInspAlgoLine->m_dTeachLengthRateMin / 100.0);
			double dLengthMax = pInspAlgoLine->m_dTeachLength * (pInspAlgoLine->m_dTeachLengthRateMax / 100.0);
			if (sRstAlgo->m_dRstLength < dLengthMin || sRstAlgo->m_dRstLength > dLengthMax)
			{
				bReturn = FALSE;
				sRstAlgo->m_bOKLength = bReturn;
			}
		}
		else
			sRstAlgo->m_bOKLength = TRUE;
		if ((pInspAlgoLine->m_nLineData & m_eLineData_Use) == m_eLineData_Use)
		{
			sRstAlgo->m_bOKPerpendicular = TRUE;
			if (sRstAlgo->m_fRstPerpendicular < pInspAlgoLine->m_fArrPerpendicular[eMMD_Min] ||
				sRstAlgo->m_fRstPerpendicular > pInspAlgoLine->m_fArrPerpendicular[eMMD_Max])
			{
				bReturn = FALSE;
				sRstAlgo->m_bOKPerpendicular = bReturn;
			}
		}
		else
			sRstAlgo->m_bOKPerpendicular = TRUE;
		if (pInspAlgoLine->m_bTeachWidthUse)
		{
			if (sRstAlgo->m_bOKWidth == FALSE)
				bReturn = FALSE;
		}
		else
			sRstAlgo->m_bOKWidth = TRUE;
	}
	Delete_1DArray(&ucArrDstWnd);
	return bReturn;
}

BOOL CPInsp_AlgoLine::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	if ((inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
	{
		RstAlgoLine * rst = (RstAlgoLine *)sRstAlgo;
		pAlignRes->offsetX = -rst->m_dRstShiftX;
		pAlignRes->offsetY = rst->m_dRstShiftY;
		pAlignRes->theta = rst->m_dRstAngle;
		bRet = TRUE;
	}

	return bRet;
}
int CPInsp_AlgoLine::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoLine * pInspAlgo = (AlgoLine *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->m_sAlgoColorBase.GetColorData();
	nData |= COLOR_DATA_WND;

	return nData;
}
bool CPInsp_AlgoLine::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}