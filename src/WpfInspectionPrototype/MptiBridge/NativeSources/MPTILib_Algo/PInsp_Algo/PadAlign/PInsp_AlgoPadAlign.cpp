#include "PInsp_AlgoPadAlign.h"

CPInsp_AlgoPadAlign::CPInsp_AlgoPadAlign(void)
{
}


CPInsp_AlgoPadAlign::~CPInsp_AlgoPadAlign(void)
{
}

void CPInsp_AlgoPadAlign::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoPadAlign::GetInspAlgoData()
{
	return eSPCAlgoPadAlign;
}

int CPInsp_AlgoPadAlign::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoPadAlign * rstAlgo = (RstAlgoPadAlign *)vRstInspAlgo;
	if (rstAlgo->m_bMissing)
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
	if (!rstAlgo->m_bOKWidth || !rstAlgo->m_bOKHeight)
	{
		eWholeNgTypeTemp[TypeAlignWrong] = e_NG;
		nCurrentNgType = TypeAlignWrong;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoPadAlign::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoPadAlign::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	int nWndL = stAlgoParam.m_rcWND.left;
	int nWndT = stAlgoParam.m_rcWND.top;
	int nWndW = stAlgoParam.m_rcWND.right - nWndL;
	int nWndH = stAlgoParam.m_rcWND.bottom - nWndT;
	cv::Rect wndROI = cv::Rect(nWndL, nWndT, nWndW, nWndH);
	bResult = InspPadAlign(sInspAlgo, sWndAlgoImg, *sInspImageData, (RstAlgoPadAlign*)sRstAlgo, ucArrDstImg, stAlgoParam.m_bTeach, stAlgoParam.m_nAlignCnt, stAlgoParam.m_sAlignRes, wndROI);

	return bResult;
}

BOOL CPInsp_AlgoPadAlign::GetROILRTB(UCHAR* ucArrImg, int nImgW, int nImgH, POINTF* ptrpoPoint)
{
	int nMeasureDirection = 1;
	BOOL bArrIsHorizon[2] = { FALSE, TRUE };
	int nLineFindType = 1;
	double dLineFindRate = 120;

	double dArrLineDAValue[PADALIGN_AREA_CNTS];
	double dArrLineDBValue[PADALIGN_AREA_CNTS];
	POINTF ArrDrawLine[PADALIGN_AREA_CNTS][2];
	int nInspOption = m_eEdgeData_UseExceptAngle;
	for (int n = 0; n < PADALIGN_AREA_CNTS; n++)
	{
		if (nMeasureDirection == 3)
			nMeasureDirection = 1;
		bool bIsHorizon = false;
		if (n > 1)
			bIsHorizon = true;
		double dAngle = 0;
		int nLineLength = 0;
		bool bReturn = m_pCPInsp_Algo->InspectionLine(nImgW, nImgH, ucArrImg, bIsHorizon, nMeasureDirection,
			&dAngle, ArrDrawLine[n], &nLineLength, &dArrLineDAValue[n], &dArrLineDBValue[n],
			nLineFindType, dLineFindRate, nInspOption);
		nMeasureDirection++;
	}
	// LT
	double dArrA[2];
	double dArrB[2];
	dArrA[0] = dArrLineDAValue[0];
	dArrB[0] = dArrLineDBValue[0];
	dArrA[1] = dArrLineDAValue[2];
	dArrB[1] = dArrLineDBValue[2];
	ptrpoPoint[0] = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrA, dArrB, bArrIsHorizon);
	// RT
	dArrA[0] = dArrLineDAValue[1];
	dArrB[0] = dArrLineDBValue[1];
	dArrA[1] = dArrLineDAValue[2];
	dArrB[1] = dArrLineDBValue[2];
	ptrpoPoint[1] = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrA, dArrB, bArrIsHorizon);
	// LB
	dArrA[0] = dArrLineDAValue[0];
	dArrB[0] = dArrLineDBValue[0];
	dArrA[1] = dArrLineDAValue[3];
	dArrB[1] = dArrLineDBValue[3];
	ptrpoPoint[2] = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrA, dArrB, bArrIsHorizon);
	// RB
	dArrA[0] = dArrLineDAValue[1];
	dArrB[0] = dArrLineDBValue[1];
	dArrA[1] = dArrLineDAValue[3];
	dArrB[1] = dArrLineDBValue[3];
	ptrpoPoint[3] = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrA, dArrB, bArrIsHorizon);
	for (int n = 0; n < 4; n++)
	{
		if (ptrpoPoint[n].x < 0)	ptrpoPoint[n].x = 0;
		if (ptrpoPoint[n].y < 0)	ptrpoPoint[n].y = 0;
		if (ptrpoPoint[n].x > nImgW)	ptrpoPoint[n].x = nImgW;
		if (ptrpoPoint[n].y > nImgH)	ptrpoPoint[n].y = nImgH;
	}
	return true;
}
BOOL CPInsp_AlgoPadAlign::GetBWImageSize(UCHAR* ucArrImg, int nImgW, int nImgH, POINTF poMinMax, POINTF* poTL, POINTF* poSize)
{
	BOOL bReturn = FALSE;
	if (ucArrImg == NULL || nImgW <= 0 || nImgH <= 0)
		return bReturn;

	// Width 평균 찾기
	double dWidth = 0;
	double dHeight = 0;
	double dXPos = 0;
	double dYPos = 0;
	for (int y = 0; y < nImgH; y++)
	{
		bool bStart = false;
		double dStart = 0;
		double dEnd = 0;
		for (int x = 0; x < nImgW; x++)
		{
			int nIndex = (y * nImgW) + x;
			if (ucArrImg[nIndex] == 255)
			{
				if (bStart == false)
				{
					dStart = x;
					dEnd = x;
					bStart = true;
				}
				else
					dEnd = x;
			}
		}
		double dBuf = dEnd - dStart;
		if (dBuf == 0)
			continue;
		if (dBuf > dWidth)
		{
			dXPos = dStart;
			dWidth = dBuf;
		}
	}
	for (int x = dXPos; x < nImgW; x++)
	{
		bool bStart = false;
		double dStart = 0;
		double dEnd = 0;
		for (int y = 0; y < nImgH; y++)
		{
			int nIndex = (y * nImgW) + x;
			if (ucArrImg[nIndex] == 255)
			{
				if (bStart == false)
				{
					dStart = y;
					dEnd = y;
					bStart = true;
				}
				else
					dEnd = y;
			}
		}
		double dBuf = dEnd - dStart;
		if (dBuf == 0)
			continue;
		bool bStartBuf = false;
		double dStartBuf = 0;
		double dEndBuf = 0;
		for (int a = 0; a < nImgW; a++)
		{
			int nIndexBuf = (dStart * nImgW) + a;
			if (ucArrImg[nIndexBuf] == 255)
			{
				if (bStartBuf == false)
				{
					dStartBuf = a;
					dEndBuf = a;
					bStartBuf = true;
				}
				else
					dEndBuf = a;
			}
		}
		double dBuf2 = dEndBuf - dStartBuf;
		if (dBuf2 > (dWidth * poMinMax.x) && dBuf2 < (dWidth * poMinMax.y))
		{
			bStartBuf = false;
			dStartBuf = 0;
			dEndBuf = 0;
			for (int a = 0; a < nImgW; a++)
			{
				int nIndexBuf = (dEnd * nImgW) + a;
				if (ucArrImg[nIndexBuf] == 255)
				{
					if (bStartBuf == false)
					{
						dStartBuf = a;
						dEndBuf = a;
						bStartBuf = true;
					}
					else
						dEndBuf = a;
				}
			}
			dBuf2 = dEndBuf - dStartBuf;
			if (dBuf2 > (dWidth * poMinMax.x) && dBuf2 < (dWidth * poMinMax.y))
			{
				if (dBuf > dHeight)
				{
					dHeight = dBuf;
					dYPos = dStart;
				}
			}
		}
	}
	poTL->x = (float)dXPos;
	poTL->y = (float)dYPos;
	poSize->x = (float)dWidth;
	poSize->y = (float)dHeight;
	return bReturn;
}
BOOL CPInsp_AlgoPadAlign::InspPadAlign(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, RstAlgoPadAlign *sRstAlgo, UCHAR* ucArrDstImg, bool bTeachData, int nAlignCnt, AlignResult * sAlignRes, cv::Rect wndROI)
{
	BOOL  bResult = FALSE;
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	CPInsp_Algo* InsAlgo = g_pInspMng->GetPtrInspAlgo();
	if (!m_pProcMilAlgo || sInspAlgo.m_eAlgoType != eAlgoPadAlign || pColorTeach == NULL)
		return bResult;
	if (sWndAlgoImg.m_ucArr2D == NULL || sWndAlgoImg.m_fArr3D == NULL || sWndAlgoImg.m_nWidth <= 0 || sWndAlgoImg.m_nHeight <= 0)
		return bResult;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoPadAlign));
		sRstAlgo->Init();
	}

	cv::Mat img2D(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D);
	cv::Mat img3D(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_32FC1, sWndAlgoImg.m_fArr3D);
	float fImgWidth = sWndAlgoImg.m_nWidth * m_resolX;
	float fImgHeight = sWndAlgoImg.m_nHeight * m_resolY;
	double dAngle = sWndAlgoImg.dAngle;

	AlgoPadAlign * pInspAlgoAlign = (AlgoPadAlign *)sInspAlgo.m_ptrInspAlgoParam;
	if (pInspAlgoAlign == NULL || pInspAlgoAlign == nullptr)
		return bResult;

	if (!pInspAlgoAlign->m_sAlgoBW.m_b2dCheck && !pInspAlgoAlign->m_sAlgoBW.m_b3dCheck && !pInspAlgoAlign->m_sAlgoBW.m_sAlgoColorBase.m_bUseColor)
		return bResult;

	POINTF poSrcPoint;
	if (m_pCPInsp_Algo->AnglePointChange_mm(fImgWidth, fImgHeight, pInspAlgoAlign->m_sSearchPoint, &poSrcPoint) == FALSE)
		return bResult;

	bool bTeach = (ucArrDstImg != NULL);
	cv::Mat imgDst(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBin(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgColor(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, cv::Scalar(0));

	if (pInspAlgoAlign->m_sAlgoBW.m_sAlgoColorBase.m_bUseColor == TRUE)
		pColorTeach->GetColorBaseBin(&pInspAlgoAlign->m_sAlgoBW.m_sAlgoColorBase, sInspImageData, imgColor.data, bTeach, 0, 0, -1, sWndAlgoImg.m_nLight_index);

	// 통합 검사/제외 영역 사용 안함
	TotalInspExceptArea stTieArea;
	stTieArea.m_nUsedMaskingValue = 0;
	stTieArea.m_nUsedInspPolygon = 0;
	stTieArea.m_nUsedWndPolygon = 0;

	m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoAlign->m_sAlgoBW, sWndAlgoImg, imgBin.data, stTieArea, imgColor.data);
	if (InsAlgo != NULL && (InsAlgo->m_nAlgoOPT & m_eAlgoOPT_PadAlign_WND) == m_eAlgoOPT_PadAlign_WND)
	{
		if (wndROI.tl().x >= 0 && wndROI.tl().x < sWndAlgoImg.m_nWidth &&
			wndROI.tl().y >= 0 && wndROI.tl().y < sWndAlgoImg.m_nHeight &&
			wndROI.br().x > 0 && wndROI.br().x < sWndAlgoImg.m_nWidth &&
			wndROI.br().y > 0 && wndROI.br().y < sWndAlgoImg.m_nHeight &&
			wndROI.size().width > 0 && wndROI.size().width < sWndAlgoImg.m_nWidth &&
			wndROI.size().height > 0 && wndROI.size().height < sWndAlgoImg.m_nHeight)
		{
			cv::Mat imgWND(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, cv::Scalar(0));
			imgWND(wndROI).setTo(255);
			cv::bitwise_and(imgWND, imgBin, imgBin);
		}
	}

	bool bAngle90 = (dAngle == 90 || dAngle == 270);
	float fTeachW = pInspAlgoAlign->m_fTeachWidth;
	float fTeachH = pInspAlgoAlign->m_fTeachHeight;
	float fShiftX = pInspAlgoAlign->m_fShiftX;
	float fShiftY = pInspAlgoAlign->m_fShiftY;
	int nTeachImgW = pInspAlgoAlign->m_fTeachWidth / m_resolX;
	int nTeachImgH = pInspAlgoAlign->m_fTeachHeight / m_resolY;

	bool bUseWidth = pInspAlgoAlign->m_bUseWidth;
	float fMin_W = pInspAlgoAlign->m_fMin_W;
	float fMax_W = pInspAlgoAlign->m_fMax_W;
	bool bUseHeight = pInspAlgoAlign->m_bUseHeight;
	float fMin_H = pInspAlgoAlign->m_fMin_H;
	float fMax_H = pInspAlgoAlign->m_fMax_H;
	if (dAngle == 90 || dAngle == 270)
	{
		fTeachW = pInspAlgoAlign->m_fTeachHeight;
		fTeachH = pInspAlgoAlign->m_fTeachWidth;
		fShiftX = pInspAlgoAlign->m_fShiftY;
		fShiftY = pInspAlgoAlign->m_fShiftX;
		nTeachImgW = pInspAlgoAlign->m_fTeachHeight / m_resolY;
		nTeachImgH = pInspAlgoAlign->m_fTeachWidth / m_resolX;

		bUseWidth = pInspAlgoAlign->m_bUseHeight;
		fMin_W = pInspAlgoAlign->m_fMin_H;
		fMax_W = pInspAlgoAlign->m_fMax_H;
		bUseHeight = pInspAlgoAlign->m_bUseWidth;
		fMin_H = pInspAlgoAlign->m_fMin_W;
		fMax_H = pInspAlgoAlign->m_fMax_W;
	}

	double cx = 0, cy = 0;
	int nMinA = 10;
	int nMinBlobArea = (int)((nTeachImgW * nTeachImgH) / 5.0f);
	if (nMinBlobArea < nMinA) nMinBlobArea = nMinA;
	int nSelectBlob = pInspAlgoAlign->m_nTypeSelectBlob;
	int nFilter = (pInspAlgoAlign->m_bFilterIsUse == TRUE) ? pInspAlgoAlign->m_nFilterStepNarrow : 0;
	BOOL bFillHole = pInspAlgoAlign->m_bFillHole;
	// blob
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imgBin.data, ucArrDstImg, imgBin.cols, imgBin.rows, nMinBlobArea, FALSE, bFillHole, nFilter, nSelectBlob);
	if (nCntBlob == 0)
	{
		if (bTeach)
			m_pProcMilAlgo->CalcBlob_Select(imgBin.data, ucArrDstImg, imgBin.cols, imgBin.rows, nMinA, FALSE, bFillHole, nFilter, nSelectBlob);

		if (sRstAlgo)
			sRstAlgo->m_bMissing = TRUE;

		return bResult;
	}

	int nBlobType = (nTeachImgW > 0 && nTeachImgH > 0) ? 1 : -1;
	m_pProcMilAlgo->CalcBlob_Select(imgBin.data, imgDst.data, imgBin.cols, imgBin.rows, nMinBlobArea, FALSE, bFillHole, nFilter, nSelectBlob, nTeachImgW, nTeachImgH, nBlobType);

	POINTF poTL;
	POINTF poSize;
	POINTF poHeight;
	poHeight.x = 0.95;
	poHeight.y = 1.05;
	if (bUseHeight)
	{
		poHeight.x = fMin_H / 100.0f;
		poHeight.y = fMax_H / 100.0f;
	}
	if (bTeachData == true)
	{
		GetBWImageSize(imgDst.data, imgDst.cols, imgDst.rows, poHeight, &poTL, &poSize);
		nTeachImgW = (int)poSize.x;
		nTeachImgH = (int)poSize.y;
	}
	poSize.x = nTeachImgW;
	poSize.y = nTeachImgH;

	POINTF poArrDstPoint[PADALIGN_AREA_CNTS];
	memset(poArrDstPoint, 0, sizeof(POINTF) * PADALIGN_AREA_CNTS);

	bool bFindLRTB = true;
	if (InsAlgo != NULL && (InsAlgo->m_nAlgoOPT & m_eAlgoOPT_PadAlign_ROI) == m_eAlgoOPT_PadAlign_ROI)
	{
		if (m_pCPInsp_Algo->GetROICenter(nSelectBlob, imgDst, poArrDstPoint))
			bFindLRTB = false;
	}
	if (bFindLRTB)
	{
		// LT | RT | LB | RB
		GetROILRTB(imgDst.data, imgDst.cols, imgDst.rows, poArrDstPoint);
	}
	for (int n = 0; n < 4; n++)
	{
		if (poArrDstPoint[n].x < 0)	poArrDstPoint[n].x = 0;
		if (poArrDstPoint[n].y < 0)	poArrDstPoint[n].y = 0;
		if (poArrDstPoint[n].x > imgDst.cols)	poArrDstPoint[n].x = imgDst.cols;
		if (poArrDstPoint[n].y > imgDst.rows)	poArrDstPoint[n].y = imgDst.rows;
	}

	if (nAlignCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
	{
		double dWBoard = (double)imgDst.cols * m_resolX;
		double dHBoard = (double)imgDst.rows * m_resolY;
		double fSrcX = poSrcPoint.x - (dWBoard / 2.0);
		double fSrcY = (dHBoard / 2.0) - poSrcPoint.y;
		double corr_x = 0, corr_y = 0;
		for (int n = 0; n < nAlignCnt; n++)
		{
			double corr_x_Buf = 0, corr_y_Buf = 0;
			int nCorrectCoordinate = m_proc3d.CorrectCoordinate(fSrcX, fSrcY, sAlignRes[n].centerX, sAlignRes[n].centerY, sAlignRes[n].theta, sAlignRes[n].offsetX, sAlignRes[n].offsetY, &corr_x_Buf, &corr_y_Buf);
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
		poSrcPoint.x = corr_x + (dWBoard / 2.0);
		poSrcPoint.y = (dHBoard / 2.0) - corr_y;
	}

	float fTeachCenterX = poSrcPoint.x / m_resolX;
	float fTeachCenterY = poSrcPoint.y / m_resolY;
	float fCrossDistence = sqrt(pow((float)nTeachImgW, 2) + pow((float)nTeachImgH, 2));
	float fRstDist_1 = sqrt(pow((poArrDstPoint[3].x - poArrDstPoint[0].x), 2) + pow((poArrDstPoint[3].y - poArrDstPoint[0].y), 2));
	float fRstDist_2 = sqrt(pow((poArrDstPoint[1].x - poArrDstPoint[2].x), 2) + pow((poArrDstPoint[2].y - poArrDstPoint[1].y), 2));
	float fRstW_1 = poArrDstPoint[3].x - poArrDstPoint[0].x;
	float fRstH_1 = poArrDstPoint[3].y - poArrDstPoint[0].y;
	float fRstW_2 = poArrDstPoint[1].x - poArrDstPoint[2].x;
	float fRstH_2 = poArrDstPoint[2].y - poArrDstPoint[1].y;
	float fRstCX_1 = poArrDstPoint[0].x + (fRstW_1 / 2.0f);
	float fRstCY_1 = poArrDstPoint[0].y + (fRstH_1 / 2.0f);
	float fRstCX_2 = poArrDstPoint[2].x + (fRstW_2 / 2.0f);
	float fRstCY_2 = poArrDstPoint[1].y + (fRstH_2 / 2.0f);

	bool bLT_RB = false;
	bool bRT_LB = false;
	float fShift = sqrt(pow((fShiftX / m_resolX), 2) + pow((fShiftY / m_resolY), 2));
	float fGap_1 = fCrossDistence - fRstDist_1;
	if (fGap_1 < 0)
		fGap_1 *= -1;
	if (fGap_1 <= fShift)
		bLT_RB = true;
	float fGap_2 = fCrossDistence - fRstDist_2;
	if (fGap_2 < 0)
		fGap_2 *= -1;
	if (fGap_2 <= fShift)
		bRT_LB = true;
	if (bLT_RB == false && bRT_LB == false)
	{
		if (fGap_1 > fGap_2)
			bRT_LB = true;
		else
			bLT_RB = true;
	}
	float fRstShiftX = 0;
	float fRstShiftY = 0;
	float fRstAngle = 0;
	float fRstWidth = 0;
	float fRstHeight = 0;
	if (bLT_RB == true && bRT_LB == true)
	{
		float fRstCenterX = ((fRstCX_1 + fRstCX_2) / 2.0f) + (sWndAlgoImg.m_fPartRoundingErrX * m_resolX);
		float fRstCenterY = ((fRstCY_1 + fRstCY_2) / 2.0f) + (sWndAlgoImg.m_fPartRoundingErrY * m_resolY);
		fRstShiftX = fTeachCenterX - fRstCenterX;
		fRstShiftY = fTeachCenterY - fRstCenterY;

		fRstAngle = atan2(fRstH_1, fRstW_1);
		fRstAngle += atan2(fRstH_2, fRstW_2);
		fRstAngle = fRstAngle / 2.;

		fRstWidth = (fRstW_1 + fRstW_2) / 2.0f;
		fRstHeight = (fRstH_1 + fRstH_2) / 2.0f;
		if (sRstAlgo)
		{
			sRstAlgo->m_poDrawCenter.x = fRstCenterX;
			sRstAlgo->m_poDrawCenter.y = fRstCenterY;
		}
	}
	else if (bLT_RB == true)
	{
		float fRstCenterX = fRstCX_1 + (sWndAlgoImg.m_fPartRoundingErrX * m_resolX);
		float fRstCenterY = fRstCY_1 + (sWndAlgoImg.m_fPartRoundingErrY * m_resolY);
		fRstShiftX = fTeachCenterX - fRstCenterX;
		fRstShiftY = fTeachCenterY - fRstCenterY;

		fRstAngle = atan2(fRstH_1, fRstW_1);

		fRstWidth = fRstW_1;
		fRstHeight = fRstH_1;
		if (sRstAlgo)
		{
			sRstAlgo->m_poDrawCenter.x = fRstCenterX;
			sRstAlgo->m_poDrawCenter.y = fRstCenterY;
		}
	}
	else if (bRT_LB == true)
	{
		float fRstCenterX = fRstCX_2 + (sWndAlgoImg.m_fPartRoundingErrX * m_resolX);
		float fRstCenterY = fRstCY_2 + (sWndAlgoImg.m_fPartRoundingErrY * m_resolY);
		fRstShiftX = fTeachCenterX - fRstCenterX;
		fRstShiftY = fTeachCenterY - fRstCenterY;

		fRstAngle = atan2(fRstH_2, fRstW_2);

		fRstWidth = fRstW_2;
		fRstHeight = fRstH_2;
		if (sRstAlgo)
		{
			sRstAlgo->m_poDrawCenter.x = fRstCenterX;
			sRstAlgo->m_poDrawCenter.y = fRstCenterY;
		}
	}
	if (sRstAlgo)
	{
		sRstAlgo->m_bMissing = FALSE;
		sRstAlgo->m_dRstShiftX = fRstShiftX * m_resolX;
		sRstAlgo->m_dRstShiftY = fRstShiftY * m_resolY;
		sRstAlgo->m_dRstAngle = fRstAngle;
		sRstAlgo->m_dRstWidth = fRstWidth * m_resolX;
		sRstAlgo->m_dRstHeight = fRstHeight * m_resolY;
		bResult = TRUE;
		if (pInspAlgoAlign->m_bUseShift == TRUE)
		{
			float fRstValue = sRstAlgo->m_dRstShiftX;
			if (fRstValue < 0)
				fRstValue *= -1;
			if (fRstValue > fShiftX)
			{
				sRstAlgo->m_bOKShiftX = FALSE;
				bResult = FALSE;
			}
			else
				sRstAlgo->m_bOKShiftX = TRUE;
			fRstValue = sRstAlgo->m_dRstShiftY;
			if (fRstValue < 0)
				fRstValue *= -1;
			if (fRstValue > fShiftY)
			{
				sRstAlgo->m_bOKShiftY = FALSE;
				bResult = FALSE;
			}
			else
				sRstAlgo->m_bOKShiftY = TRUE;
		}
		else
		{
			sRstAlgo->m_dRstShiftX = 0;
			sRstAlgo->m_dRstShiftY = 0;
			sRstAlgo->m_bOKShiftX = TRUE;
			sRstAlgo->m_bOKShiftY = TRUE;
		}
		if (pInspAlgoAlign->m_bUseAngle == TRUE)
		{
			if (fRstAngle < 0)
				fRstAngle *= -1;
			if (fRstAngle > pInspAlgoAlign->m_fAngle)
			{
				sRstAlgo->m_bOKAngle = FALSE;
				bResult = FALSE;
			}
			else
				sRstAlgo->m_bOKAngle = TRUE;
		}
		else
		{
			sRstAlgo->m_dRstAngle = 0;
			sRstAlgo->m_bOKAngle = TRUE;
		}

		if (bUseWidth == TRUE)
		{
			float fMinW = fTeachW * (fMin_W / 100.0f);
			float fMaxW = fTeachW * (fMax_W / 100.0f);
			if (sRstAlgo->m_dRstWidth > fMinW && sRstAlgo->m_dRstWidth < fMaxW)
				sRstAlgo->m_bOKWidth = TRUE;
			else
			{
				sRstAlgo->m_bOKWidth = FALSE;
				bResult = FALSE;
			}
		}
		else
		{
			sRstAlgo->m_dRstWidth = 0;
			sRstAlgo->m_bOKWidth = TRUE;
		}

		if (bUseHeight == TRUE)
		{
			float fMinH = fTeachH * (fMin_H / 100.0f);
			float fMaxH = fTeachH * (fMax_H / 100.0f);
			if (sRstAlgo->m_dRstHeight > fMinH && sRstAlgo->m_dRstHeight < fMaxH)
				sRstAlgo->m_bOKHeight = TRUE;
			else
			{
				sRstAlgo->m_bOKHeight = FALSE;
				bResult = FALSE;
			}
		}
		else
		{
			sRstAlgo->m_dRstHeight = 0;
			sRstAlgo->m_bOKHeight = TRUE;
		}

		sRstAlgo->m_rcTeachRect.left = fTeachCenterX - (nTeachImgW / 2.0f);
		sRstAlgo->m_rcTeachRect.top = fTeachCenterY - (nTeachImgH / 2.0f);
		if (bTeachData == true)
		{
			sRstAlgo->m_rcTeachRect.left = poTL.x;
			sRstAlgo->m_rcTeachRect.top = poTL.y;
		}
		sRstAlgo->m_rcTeachRect.right = sRstAlgo->m_rcTeachRect.left + nTeachImgW;
		sRstAlgo->m_rcTeachRect.bottom = sRstAlgo->m_rcTeachRect.top + nTeachImgH;
		for (int n = 0; n < PADALIGN_AREA_CNTS; n++)
		{
			sRstAlgo->m_sArrDstPoint[n].x = poArrDstPoint[n].x;
			sRstAlgo->m_sArrDstPoint[n].y = poArrDstPoint[n].y;
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoPadAlign::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;
	
	RstAlgoPadAlign * rst = (RstAlgoPadAlign *)sRstAlgo;
	pAlignRes->offsetX = -rst->m_dRstShiftX;
	pAlignRes->offsetY = rst->m_dRstShiftY;
	pAlignRes->theta = rst->m_dRstAngle;
	bRet = TRUE;

	return bRet;
}
int CPInsp_AlgoPadAlign::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoPadAlign * pInspAlgo = (AlgoPadAlign *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->m_sAlgoBW.m_sAlgoColorBase.GetColorData();

	return nData;
}
bool CPInsp_AlgoPadAlign::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}