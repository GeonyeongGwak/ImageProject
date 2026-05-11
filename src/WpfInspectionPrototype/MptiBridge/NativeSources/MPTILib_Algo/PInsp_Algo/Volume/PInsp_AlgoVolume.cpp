#include "PInsp_AlgoVolume.h"


CPInsp_AlgoVolume::CPInsp_AlgoVolume(void)
{
}


CPInsp_AlgoVolume::~CPInsp_AlgoVolume(void)
{
}

void CPInsp_AlgoVolume::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoVolume::GetInspAlgoData()
{
	return eSPCAlgoVolume;
}

int CPInsp_AlgoVolume::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoVolume * rstAlgo = (RstAlgoVolume *)vRstInspAlgo;
	if (rstAlgo->m_bExcess && !rstAlgo->m_bNoSolder)
	{
		eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
		nCurrentNgType = TypeSolderExcess;
	}
	else if (!rstAlgo->m_bExcess && rstAlgo->m_bNoSolder)
	{
		eWholeNgTypeTemp[TypeNoSolder] = e_NG;
		nCurrentNgType = TypeNoSolder;
	}
	else if (rstAlgo->m_bVolume)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}

	if (rstAlgo->m_bOKColdJoint == FALSE)
	{
		eWholeNgTypeTemp[TypeColdJoint] = e_NG;
		nCurrentNgType = TypeColdJoint;
	}

	if (nWndType == eINSP_SOLDER)
	{
		if (rstAlgo->m_bArea == TRUE || rstAlgo->m_bHeight == TRUE)
		{
			eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
			nCurrentNgType = TypeSolderFilet;
		}
		if (rstAlgo->m_bOKInclination == FALSE || rstAlgo->m_nNG > 0)
		{
			eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
			nCurrentNgType = TypeSolderFilet;
		}
	}
	else if (nWndType == eINSP_MOUNT)
	{
		if (rstAlgo->m_bArea == TRUE)
		{
			eWholeNgTypeTemp[TypeBODYAREA] = e_NG;
			nCurrentNgType = TypeBODYAREA;
		}
		if (rstAlgo->m_bHeight == TRUE)
		{
			eWholeNgTypeTemp[TypeMountLift] = e_NG;
			nCurrentNgType = TypeMountLift;
		}
		if (rstAlgo->m_bOKInclination == FALSE)
		{
			eWholeNgTypeTemp[TypeMountWorng] = e_NG;
			nCurrentNgType = TypeMountWorng;
		}
		if (!rstAlgo->OKData(m_eVolumeRData_ShiftX) || !rstAlgo->OKData(m_eVolumeRData_ShiftY))
		{
			eWholeNgTypeTemp[TypeMountShift] = e_NG;
			nCurrentNgType = TypeMountShift;
		}
		if (!rstAlgo->OKData(m_eVolumeRData_CircleDiameter) || !rstAlgo->OKData(m_eVolumeRData_CirclePercent))
		{
			eWholeNgTypeTemp[TypeS_Ball] = e_NG;
			nCurrentNgType = TypeS_Ball;
		}
		if (!rstAlgo->OKData(m_eVolumeRData_UnderHeightMin) || !rstAlgo->OKData(m_eVolumeRData_UpperHeightMax) || !rstAlgo->OKData(m_eVolumeRData_Width))
		{
			eWholeNgTypeTemp[TypeBODYWIDTH] = e_NG;
			nCurrentNgType = TypeBODYWIDTH;
		}
		if (rstAlgo->m_nOKPadPatternCheck > 0 || !rstAlgo->OKData(m_eVolumeRData_Length))
		{
			eWholeNgTypeTemp[TypeBODYLENGTH] = e_NG;
			nCurrentNgType = TypeBODYLENGTH;
		}
	}

	return nCurrentNgType;
}

bool CPInsp_AlgoVolume::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return true;
	return true;
}

BOOL CPInsp_AlgoVolume::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	UCHAR * puImgDst = NULL;
	int nWidth = sWndAlgoImg.m_nWidth;
	int nHeight = sWndAlgoImg.m_nHeight;
	if (stAlgoParam.m_bInspection == false)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &puImgDst, nWidth * nHeight);
		memset(puImgDst, 0, sizeof(UCHAR)* nWidth * nHeight);
	}
	bResult = InspVolume(sInspAlgo, sWndAlgoImg, stAlgoParam, (RstAlgoVolume *)sRstAlgo, stTieArea, puImgDst);

	RstAlgoVolume * rstVolume = (RstAlgoVolume *)sRstAlgo;
	if (stAlgoParam.m_bInspection)
	{
		for (int nIdx = 0; nIdx < rstVolume->m_nRect_I; nIdx++)
		{
			rstVolume->m_rcRect_I[nIdx].left += stAlgoParam.m_dx;
			rstVolume->m_rcRect_I[nIdx].right += stAlgoParam.m_dx;
			rstVolume->m_rcRect_I[nIdx].top += stAlgoParam.m_dy;
			rstVolume->m_rcRect_I[nIdx].bottom += stAlgoParam.m_dy;
		}
		rstVolume->m_fArr[m_eVolumeR_Teach_X] += stAlgoParam.m_dx;
		rstVolume->m_fArr[m_eVolumeR_Teach_Y] += stAlgoParam.m_dy;
		rstVolume->m_fArr[m_eVolumeR_Rst_X] += stAlgoParam.m_dx;
		rstVolume->m_fArr[m_eVolumeR_Rst_Y] += stAlgoParam.m_dy;
		for (int nIdx = 0; nIdx < VOLUME_INSP_CNTS; nIdx++)
		{
			rstVolume->m_rcRect_TH[nIdx].left += stAlgoParam.m_dx;
			rstVolume->m_rcRect_TH[nIdx].right += stAlgoParam.m_dx;
			rstVolume->m_rcRect_TH[nIdx].top += stAlgoParam.m_dy;
			rstVolume->m_rcRect_TH[nIdx].bottom += stAlgoParam.m_dy;
		}
		for (int nIdx = 0; nIdx < rstVolume->m_nRstPAD_CHK; nIdx++)
		{
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.left += stAlgoParam.m_dx;
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.right += stAlgoParam.m_dx;
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.top += stAlgoParam.m_dy;
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.bottom += stAlgoParam.m_dy;
		}
		for (int nIdx = 0; nIdx < rstVolume->m_nRstPAD_DIVIDE; nIdx++)
		{
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.left += stAlgoParam.m_dx;
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.right += stAlgoParam.m_dx;
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.top += stAlgoParam.m_dy;
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.bottom += stAlgoParam.m_dy;
		}
	}
	else
	{
		int nStartX = stAlgoParam.m_nStartX;
		int nStartY = stAlgoParam.m_nStartY;
		int nPartW = stAlgoParam.m_sPartAlgoImg->m_nWidth;
		int nPartH = stAlgoParam.m_sPartAlgoImg->m_nHeight;
		for (int y = nStartY; y < nStartY + nHeight; y++)
		{
			for (int x = nStartX; x < nStartX + nWidth; x++)
			{
				int nIndex = (y * nPartW) + x;
				int nIndex2 = ((y - nStartY) * nWidth) + (x - nStartX);
				if (nIndex < 0 || nIndex > nPartW * nPartH)
					continue;
				if (nIndex2 < 0 || nIndex2 > nWidth * nHeight)
					continue;
				ucArrDstImg[nIndex] = puImgDst[nIndex2];
			}
		}
		for (int nIdx = 0; nIdx < rstVolume->m_nRect_I; nIdx++)
		{
			rstVolume->m_rcRect_I[nIdx].left += nStartX;
			rstVolume->m_rcRect_I[nIdx].right += nStartX;
			rstVolume->m_rcRect_I[nIdx].top += nStartY;
			rstVolume->m_rcRect_I[nIdx].bottom += nStartY;
		}
		rstVolume->m_fArr[m_eVolumeR_Teach_X] += nStartX;
		rstVolume->m_fArr[m_eVolumeR_Teach_Y] += nStartY;
		rstVolume->m_fArr[m_eVolumeR_Rst_X] += nStartX;
		rstVolume->m_fArr[m_eVolumeR_Rst_Y] += nStartY;
		for (int nIdx = 0; nIdx < VOLUME_INSP_CNTS; nIdx++)
		{
			rstVolume->m_rcRect_TH[nIdx].left += nStartX;
			rstVolume->m_rcRect_TH[nIdx].right += nStartX;
			rstVolume->m_rcRect_TH[nIdx].top += nStartY;
			rstVolume->m_rcRect_TH[nIdx].bottom += nStartY;
		}
		for (int nIdx = 0; nIdx < rstVolume->m_nRstPAD_CHK; nIdx++)
		{
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.left += nStartX;
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.right += nStartX;
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.top += nStartY;
			rstVolume->m_sRstPAD_CHK[nIdx].m_rcRect.bottom += nStartY;
		}
		for (int nIdx = 0; nIdx < rstVolume->m_nRstPAD_DIVIDE; nIdx++)
		{
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.left += nStartX;
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.right += nStartX;
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.top += nStartY;
			rstVolume->m_sRstPAD_DIVIDE[nIdx].m_rcRect.bottom += nStartY;
		}
		Delete_1DArray(&puImgDst);
	}
	return bResult;
}

BOOL CPInsp_AlgoVolume::InspColdJoint(WndAlgoImg &sWndAlgoImg, AlgoVolume *pInspAlgoVolume, UCHAR *puImgBW, ColdJoint sColdJoint, RstColdJoint *sRstColdJoint, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;

	int nLine = __LINE__;
	try
	{
		UCHAR* pucImgSrc = sWndAlgoImg.m_ucArr2D;
		float* pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if ((puImgBW == NULL) || (pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
			return bResult;

		if (sRstColdJoint)
			memset(sRstColdJoint, 0, (sizeof(RstColdJoint)));

		double dCX_Temp = 0;
		double dCY_Temp = 0;
		double dArea_Temp = 0;
		CRect rcBlob_Temp(0, 0, 0, 0);

		int nInspectionAreaPer = 100;
		if (pInspAlgoVolume->UseData(m_eVolumeData_InspectionArea))
		{
			nInspectionAreaPer = pInspAlgoVolume->GetDataN(m_eVolumeN_InspectionArea);
			if (nInspectionAreaPer <= 0) nInspectionAreaPer = 1;
			else if (nInspectionAreaPer > 100) nInspectionAreaPer = 100;
		}

		cv::Mat imgBW(nImgHeight, nImgWidth, CV_8UC1, puImgBW);
		cv::Mat imgBW_Fillhole(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		m_pCPInsp_Algo->BlobImageStruct_BW2(*pInspAlgoVolume, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, sColdJoint.m_nMinBlobArea,
			&dArea_Temp, &dCX_Temp, &dCY_Temp, &rcBlob_Temp, imgBW_Fillhole.data, pInspAlgoVolume->m_nTypeSelectBlob, pInspAlgoVolume->m_bFilterIsUse, pInspAlgoVolume->m_nFilterStepNarrow,
			stTieArea, TRUE, sColdJoint.m_nWndDir, nInspectionAreaPer);

		nLine = __LINE__;

		std::vector<std::vector<cv::Point>> allcontour;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(imgBW_Fillhole, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		int nMAX_ID = -1;
		int nMAXArea = 0;
		for (int a = 0; a < allcontour.size(); a++)
		{
			cv::Rect roiMax = cv::boundingRect(allcontour[a]);
			int nCNT = cv::contourArea(allcontour[a]);
			if (nCNT > nMAXArea)
			{
				nMAXArea = nCNT;
				nMAX_ID = a;
				dCX_Temp = roiMax.x + roiMax.width / 2.0;
				dCY_Temp = roiMax.y + roiMax.height / 2.0;
			}
		}

		nLine = __LINE__;

		if (nMAX_ID >= 0 && nMAXArea > 0)
		{
			if (dCX_Temp < 0) dCX_Temp = 0;
			if (dCY_Temp < 0) dCY_Temp = 0;
			if (dCX_Temp > nImgWidth - 1) dCX_Temp = nImgWidth - 1;
			if (dCY_Temp > nImgHeight - 1) dCY_Temp = nImgHeight - 1;

			cv::Mat fillMask = cv::Mat::zeros(nImgHeight + 2, nImgWidth + 2, CV_8UC1);
			cv::floodFill(imgBW_Fillhole, fillMask, cv::Point(dCX_Temp, dCY_Temp), cv::Scalar(255));
		}

		nLine = __LINE__;

		cv::Mat imgBW_Invert;
		cv::bitwise_not(imgBW, imgBW_Invert);
		m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, imgBW_Invert.data, NULL, stTieArea);

		nLine = __LINE__;

		cv::Mat imgCJ;
		cv::bitwise_and(imgBW_Fillhole, imgBW_Invert, imgCJ);
		allcontour.clear();
		hierarchy.clear();
		cv::findContours(imgCJ, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		nLine = __LINE__;

		int nMaxBin = 0;
		for (int a = 0; a < allcontour.size(); a++)
		{
			int nBin = cv::contourArea(allcontour[a]);
			if (nBin > nMaxBin)
			{
				nMaxBin = nBin;
				cv::Rect boundingRect = cv::boundingRect(allcontour[a]);
				RECT roi;
				roi.left = boundingRect.x;
				roi.right = boundingRect.x + boundingRect.width;
				roi.top = boundingRect.y;
				roi.bottom = boundingRect.y + boundingRect.height;

				sRstColdJoint->m_rcRect_I.left = roi.left;
				sRstColdJoint->m_rcRect_I.right = roi.right;
				sRstColdJoint->m_rcRect_I.top = roi.top;
				sRstColdJoint->m_rcRect_I.bottom = roi.bottom;
			}
		}

		nLine = __LINE__;

		int nCJCnt = cv::countNonZero(imgCJ);
		int nBWCnt = cv::countNonZero(imgBW);

		double dResolutionXY = m_pCPInsp_Algo->GetResolX() * m_pCPInsp_Algo->GetResolY();
		double dCJ_Area = nCJCnt * dResolutionXY;
		double dBW_Area = nBWCnt * dResolutionXY;

		sRstColdJoint->dRstAreaPer_R = dBW_Area > 0 ? (dCJ_Area / dBW_Area) * 100 : 0.0;
		sRstColdJoint->dRstArea_R = dCJ_Area;

		sRstColdJoint->m_nOKColdJoint = TRUE;
		if ((sColdJoint.m_dArea_T > 0.0 && sRstColdJoint->dRstArea_R >= sColdJoint.m_dArea_T) ||
			(sColdJoint.m_dAreaPer_T > 0.0 && sRstColdJoint->dRstAreaPer_R >= sColdJoint.m_dAreaPer_T))
			sRstColdJoint->m_nOKColdJoint = FALSE;
		else
		{
			sRstColdJoint->m_rcRect_I.left = 0;
			sRstColdJoint->m_rcRect_I.right = 0;
			sRstColdJoint->m_rcRect_I.top = 0;
			sRstColdJoint->m_rcRect_I.bottom = 0;
		}
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_AlgoVolume::InspColdJoint(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}

	return bResult;
}
BOOL CPInsp_AlgoVolume::InspInclination(WndAlgoImg &sWndAlgoImg, UCHAR *puImgBW, sInclination sData, RstInclination *sRst)
{
	BOOL bResult = FALSE;

	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((puImgBW == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;
	if (sRst)
		memset(sRst, 0, (sizeof(RstInclination)));
	else
		return bResult;
	double dUM = 1000.0;
	float fPosX = sData.m_ptrROI[0].x;
	float fPosY = sData.m_ptrROI[0].y;
	float fRoiW = sData.m_ptrROI[1].x;
	float fRoiH = sData.m_ptrROI[1].y;
	bool bAllWnd = false;
	if (fPosX == 0 && fPosX == 0 && fPosX == 0 && fPosX == 0)
		bAllWnd = true;
	if (sWndAlgoImg.dAngle == 90 || sWndAlgoImg.dAngle == 270)
	{
		fRoiW = sData.m_ptrROI[1].y;
		fRoiH = sData.m_ptrROI[1].x;
	}
	POINTF poSrcPos;
	POINTF poDstPos;
	poSrcPos.x = fPosX / m_resolX;
	poSrcPos.y = fPosY / m_resolY;
	if (m_pCPInsp_Algo->AnglePosChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, poSrcPos, &poDstPos) == FALSE)
		return bResult;

	int nRoiW = RounD(fRoiW / m_resolX);
	int nRoiH = RounD(fRoiH / m_resolY);
	int nRoiL = poDstPos.x - (nRoiW / 2);
	int nRoiT = poDstPos.y - (nRoiH / 2);
	if (bAllWnd == true)
	{
		nRoiW = nImgWidth;
		nRoiH = nImgHeight;
		nRoiL = 0;
		nRoiT = 0;
	}
	if (nRoiL < 0) nRoiL = 0;
	if (nRoiT < 0) nRoiT = 0;
	if (nRoiW < 0) nRoiW = 0;
	if (nRoiH < 0) nRoiH = 0;
	if (nRoiL + nRoiW > nImgWidth)
		nRoiW = nImgWidth - nRoiL;
	if (nRoiT + nRoiH > nImgHeight)
		nRoiH = nImgHeight - nRoiT;

	if (nRoiW < 0 && nRoiH < 0)
		return bResult;

	sRst->m_rcRect_I.left = nRoiL;
	sRst->m_rcRect_I.top = nRoiT;
	sRst->m_rcRect_I.right = nRoiL + nRoiW;
	sRst->m_rcRect_I.bottom = nRoiT + nRoiH;
	//float *pfInspROI = new float[nRoiW * nRoiH];
	float *pfInspROI = g_pMManager->pem_new<float>(true, nRoiW * nRoiH, (PCHAR)__FUNCTION__, __LINE__);
	memset(pfInspROI, 0, sizeof(float) * nRoiW * nRoiH);
	m_proc3d.GetCropZmap_LT(pfImgSrc, pfInspROI, nImgWidth, nImgHeight, nRoiL, nRoiT, nRoiW, nRoiH);
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("InspROI_ORG.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfInspROI, nRoiW, nRoiH, _T("InspROI.bmp"));
	int nW = nRoiW;
	int nH = nRoiH;
	bool bIsHorizon = true;
	if (sData.m_nWndDir == e_TOP || sData.m_nWndDir == e_BOTTOM)
	{
		nW = nRoiH;
		nH = nRoiW;
	}
	/*double *ptrdLineX = new double[nW];
	double *ptrdLineY = new double[nW];*/
	double *ptrdLineX = g_pMManager->pem_new<double>(true, nW, (PCHAR)__FUNCTION__, __LINE__);
	double *ptrdLineY = g_pMManager->pem_new<double>(true, nW, (PCHAR)__FUNCTION__, __LINE__);
	sRst->m_nLineTotalCnt = 0;
	for (int x = 0; x < nW; x++)
	{
		int nX = x;
		if (sData.m_nWndDir == e_TOP || sData.m_nWndDir == e_LEFT)
			nX = (nW - 1) - x;
		if (nX < 0 || nX >= nW) break;
		float fSum = 0.0f;
		int nCnt = 0;
		for (int y = 0; y < nH; y++)
		{
			int nIndex = (y * nRoiW) + nX;
			int nIndex2 = ((y + nRoiT) * nRoiW) + (nX + nRoiL);
			if (sData.m_nWndDir == e_TOP || sData.m_nWndDir == e_BOTTOM)
			{
				nIndex = (nX * nRoiW) + y;
				nIndex2 = ((nX + nRoiL) * nRoiW) + (y + nRoiT);
			}
			if (nIndex >= nRoiW * nRoiH || nIndex < 0 ||
				nIndex2 >= nImgWidth * nImgHeight || nIndex2 < 0 ||
				puImgBW[nIndex2] == 0)
				continue;
			fSum += pfInspROI[nIndex];
			nCnt++;
		}
		if (nCnt == 0) continue;
		float fAvg = fSum / (float)nCnt;
		if (fAvg < 0) fAvg = 0;
		ptrdLineX[sRst->m_nLineTotalCnt] = (nX + 1) * m_resolX * dUM;
		ptrdLineY[sRst->m_nLineTotalCnt] = fAvg;
		if (g_nFullCnt > sRst->m_nLineTotalCnt)
			sRst->m_fArrLineHeightAvg[sRst->m_nLineTotalCnt] = fAvg;
		sRst->m_nLineTotalCnt++;
	}
	double dA = 0, dB = 0;
	sRst->m_fRstInclination = (float)m_pCPInsp_Algo->GetGradient(ptrdLineX, ptrdLineY, (double)sRst->m_nLineTotalCnt, dA, dB, bIsHorizon);
	if (sRst->m_nLineTotalCnt > g_nFullCnt)
	{
		float *fArrBuf = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrBuf, sRst->m_nLineTotalCnt);
		memset(fArrBuf, 0, sRst->m_nLineTotalCnt * sizeof(float));
		int nGap = sRst->m_nLineTotalCnt % g_nFullCnt;
		int nGap2 = nGap / 2;
		if (nGap % 2 != 0) nGap2++;
		int nIdx = 0;
		for (int a = 0; a < sRst->m_nLineTotalCnt - nGap; a++)
		{
			fArrBuf[a] = ptrdLineY[nIdx];
			if (a < nGap2 || a >= sRst->m_nLineTotalCnt - nGap2)
				nIdx++;
			nIdx++;
		}
		nGap = (sRst->m_nLineTotalCnt - nGap) / g_nFullCnt;
		nIdx = 0;
		for (int a = 0; a < g_nFullCnt; a++)
		{
			sRst->m_fArrLineHeightAvg[a] = fArrBuf[nIdx];
			nIdx += nGap;
		}
		sRst->m_nLineTotalCnt = g_nFullCnt - 1;
		Delete_1DArray(&fArrBuf);
	}
	Delete_1DArray(&pfInspROI);
	Delete_1DArray(&ptrdLineX);
	Delete_1DArray(&ptrdLineY);

	return bResult;
}
double CPInsp_AlgoVolume::InspRemoveHeight(WndAlgoImg &sWndAlgoImg, UCHAR *puImgBW, RemoveHight sData, TotalInspExceptArea stTieArea)
{
	double dResult = 0;
	if (sData.m_bBlackHeight == FALSE)
		return dResult;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((puImgBW == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return dResult;
	double dBlackHeight = 0;
	double dUM = 1000.0;
	if (sData.m_bBlackHeight == TRUE)
	{
		UCHAR *puImgBWBuf = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &puImgBWBuf, nImgWidth * nImgHeight);
		memset(puImgBWBuf, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
		for (int i = 0; i < nImgWidth * nImgHeight; ++i)
			puImgBWBuf[i] = puImgBW[i];

		cv::Mat imgBW(nImgHeight, nImgWidth, CV_8UC1, puImgBW);
		cv::Mat imgBWBuf(nImgHeight, nImgWidth, CV_8UC1, puImgBWBuf);
		cv::Mat imgSrc(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);
		// KIY 2020/04/23 : 폴리곤 통합
		m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, puImgBWBuf, NULL, stTieArea);

		float fPosX = sData.m_ptrROI[0].x;
		float fPosY = sData.m_ptrROI[0].y;
		float fRoiW = sData.m_ptrROI[1].x;
		float fRoiH = sData.m_ptrROI[1].y;
		bool bAllWnd = false;
		if (fPosX == 0 && fPosX == 0 && fPosX == 0 && fPosX == 0)
			bAllWnd = true;
		if (sWndAlgoImg.dAngle == 90 || sWndAlgoImg.dAngle == 270)
		{
			fRoiW = sData.m_ptrROI[1].y;
			fRoiH = sData.m_ptrROI[1].x;
		}
		POINTF poSrcPos;
		POINTF poDstPos;
		poSrcPos.x = fPosX / m_resolX;
		poSrcPos.y = fPosY / m_resolY;
		if (m_pCPInsp_Algo->AnglePosChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, poSrcPos, &poDstPos) == TRUE)
		{
			int nRoiW = RounD(fRoiW / m_resolX);
			int nRoiH = RounD(fRoiH / m_resolY);
			int nRoiL = poDstPos.x - (nRoiW / 2);
			int nRoiT = poDstPos.y - (nRoiH / 2);
			if (bAllWnd == true)
			{
				nRoiW = nImgWidth;
				nRoiH = nImgHeight;
				nRoiL = 0;
				nRoiT = 0;
			}
			if (nRoiL < 0) nRoiL = 0;
			if (nRoiT < 0) nRoiT = 0;
			if (nRoiW < 0) nRoiW = 0;
			if (nRoiH < 0) nRoiH = 0;
			if (nRoiL + nRoiW > nImgWidth)
				nRoiW = nImgWidth - nRoiL;
			if (nRoiT + nRoiH > nImgHeight)
				nRoiH = nImgHeight - nRoiT;
			if (nRoiW < 0) nRoiW = 0;
			if (nRoiH < 0) nRoiH = 0;
			double dBlackSum = 0;
			int nBlackCnt = 0;
			for (int y = nRoiT; y < nRoiT + nRoiH; y++)
			{
				for (int x = nRoiL; x < nRoiL + nRoiW; x++)
				{
					int nIndex = (y * nImgWidth) + x;
					if (puImgBWBuf[nIndex] == 1)
						continue;
					dBlackSum += pfImgSrc[nIndex];
					nBlackCnt++;
				}
			}
			double dAvgMin = 0;
			double dAvgMax = 0;
			if (nBlackCnt > 0)
			{
				dAvgMin = (dBlackSum / nBlackCnt) * 0.8;
				dAvgMax = (dBlackSum / nBlackCnt) * 1.2;
			}
			if (dAvgMin > dAvgMax)	// 마이너스인 경우
				swap(dAvgMin, dAvgMax);

			dBlackSum = 0;
			nBlackCnt = 0;
			for (int y = nRoiT; y < nRoiT + nRoiH; y++)
			{
				for (int x = nRoiL; x < nRoiL + nRoiW; x++)
				{
					int nIndex = (y * nImgWidth) + x;
					if (puImgBWBuf[nIndex] == 1)
						continue;
					if (dAvgMin <= pfImgSrc[nIndex] && dAvgMax >= pfImgSrc[nIndex])
					{
						dBlackSum += pfImgSrc[nIndex];
						nBlackCnt++;
					}
				}
			}
			if (nBlackCnt > 0)
				dBlackHeight = (dBlackSum / dUM) / nBlackCnt;
		}
		Delete_1DArray(&puImgBWBuf);
	}
	dResult = sData.m_fRemovalHeight + dBlackHeight;
	return dResult;
}
BOOL CPInsp_AlgoVolume::InspVolume(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspAlgoParam stAlgoParam, RstAlgoVolume *sRstAlgo, TotalInspExceptArea stTieArea, UCHAR* ucArrDstImg, bool bParallel)
{
	DWORD st = GetTickCount();
	BOOL bResult = FALSE;

	int nLine = __LINE__;
	try
	{
		PAD_PAT* cPAT = g_pInspMng->GetPAD_PAT();
		if (cPAT == NULL)
			return FALSE;

		int nWndDir = stAlgoParam.m_nWndDir;
		if (sRstAlgo)
		{
			memset(sRstAlgo, 0, (sizeof(RstAlgoVolume)));
			sRstAlgo->Init();
		}
		nLine = __LINE__;

		UCHAR* pucImgSrc = sWndAlgoImg.m_ucArr2D;
		float* pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
			return bResult;

		bool bInspSave = (g_pMPTI && g_pMPTI->m_LogLevel == m_eLogLv_Blob);
		if (sInspAlgo.m_eAlgoType != eAlgoVolume)
			return bResult;
		if (!m_pProcMilAlgo)
			return bResult;

		AlgoVolume* pInspAlgoVolume = (AlgoVolume*)sInspAlgo.m_ptrInspAlgoParam;
		if (!pInspAlgoVolume)
			return bResult;
		if (!pInspAlgoVolume->m_b2dCheck && !pInspAlgoVolume->m_b3dCheck)
			return bResult;
		nLine = __LINE__;

		int nTYPE_P = stAlgoParam.m_nTYPE;
		if (nTYPE_P == m_ePAD_PARALLEL_Type4 && bParallel == false)
		{
			if (pInspAlgoVolume->m_rstPadPatRst.CheckINSP_PARALLEL() == true)
			{
				if (sRstAlgo && stAlgoParam.m_sRstAlgo_P)
				{
					g_pInspMng->m_fVolTact[m_eVOL_TACT_VOLUME_INSP_OK] += 1;

					*sRstAlgo = *(RstAlgoVolume*)stAlgoParam.m_sRstAlgo_P;
					bResult = pInspAlgoVolume->m_rstPadPatRst.m_bVolume_RET;
					return bResult;
				}
				else
					g_pInspMng->m_fVolTact[m_eVOL_TACT_VOLUME_INSP_FAIL2] += 1;
			}
			else
				g_pInspMng->m_fVolTact[m_eVOL_TACT_VOLUME_INSP_FAIL] += 1;
		}

		double dWndAngle = sWndAlgoImg.dAngle;
		double dCenTeachX = pInspAlgoVolume->GetData(m_eVolumeF_CenterX);
		double dCenTeachY = pInspAlgoVolume->GetData(m_eVolumeF_CenterY);
		double dResolX = m_resolX;
		double dResolY = m_resolY;
		if (g_pMPTI->m_bSideOriginalSize == true)
		{
			dResolX = g_pMPTI->m_dBtmSideResX;
			dResolY = g_pMPTI->m_dBtmSideResY;
		}
		if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
			m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle, 0, 0, &dCenTeachX, &dCenTeachY);
		nLine = __LINE__;

		double dCX = 0;
		double dCY = 0;
		double dArea = 0;
		int nMinBlobArea = 4;
		CRect rcBlob(0, 0, 0, 0);
		int nSizeImg = nImgWidth * nImgHeight;
		UCHAR* pUcArrDstImg = g_pMManager->pem_new<UCHAR>(true, nSizeImg, (PCHAR)__FUNCTION__, __LINE__);
		memset(pUcArrDstImg, 0, sizeof(UCHAR) * nSizeImg);
#if _DEBUG
		cv::Mat ImgSrc(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc);
		cv::Mat imgDstImg(nImgHeight, nImgWidth, CV_8UC1, pUcArrDstImg);
#endif

		// 통합 검사/제외 영역에 넣어준다
		stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
		stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
		stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
		stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];
		nLine = __LINE__;

		// Result
		double dUM = 1000.0;
		double dRstVolume = 0;
		double dRstHeight = 0.0;
		double dRstArea = 0.0;
		double dResolutionX = m_pCPInsp_Algo->GetResolX();
		double dResolutionY = m_pCPInsp_Algo->GetResolY();
		double dResolutionXY = dResolutionX * dResolutionY;
		double dBlackHeight = 0;
		double dRstWidth = 0.0;
		double dRstLength = 0.0;
		double dRstUnderHeightWidth = 0.0;
		double dRstUnderHeightLength = 0.0;
		double dRstUnderHeightArea = 0.0;
		double dRstUpperHeightWidth = 0.0;
		double dRstUpperHeightLength = 0.0;
		double dRstUpperHeightArea = 0.0;
		float fRstCircleDiameter = 0.0f;
		float fRstCirclePercent = 0.0f;
		int nPatternDivide = pInspAlgoVolume->GetDataN(m_eVolumeN_PatternDivide);
		double dPAD_PAT_POLY_X = 0;
		double dPAD_PAT_POLY_Y = 0;
		CRect rcPAD_PAT_POLY(-1, -1, -1, -1);
		int nCheckINSP = pInspAlgoVolume->m_rstPadPatRst.CheckINSP();
		if (pInspAlgoVolume->m_bUseBWOption)
		{
			nLine = __LINE__;

			cv::Mat ImgPoly(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
			bool bPOLY_IMG = false;
			if (stAlgoParam.m_bInspection == TRUE && pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT_Check))
			{
				if (nCheckINSP == m_eVOL_PAT_RST_OK)
				{
					if (stAlgoParam.m_ucPOLY != NULL &&
						stAlgoParam.m_nPOLY_W == nImgWidth &&
						stAlgoParam.m_nPOLY_H == nImgHeight)
					{
						memcpy(ImgPoly.data, stAlgoParam.m_ucPOLY, nImgWidth * nImgHeight * sizeof(UCHAR));
						bPOLY_IMG = true;
					}
				}
			}

			std::vector<std::vector<cv::Point>> vPoly(1, std::vector<cv::Point>());
			for (int i = 0; i < stAlgoParam.m_sPadPoly.m_nPolyCnt; i++)
			{
				int nPolyX = (int)std::round(stAlgoParam.m_sPadPoly.m_ptArrPoly[i].x);
				int nPolyY = (int)std::round(stAlgoParam.m_sPadPoly.m_ptArrPoly[i].y);
				cv::Point ptPoly(nPolyX, nPolyY);
				vPoly[0].push_back(ptPoly);
			}
			g_pInspMng->m_fVolTact[m_eVOL_TACT_SET_POLY] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

			int nTypeSelectBlob = pInspAlgoVolume->m_nTypeSelectBlob;
			BOOL bPadPatternBlob = FALSE;
			if (stAlgoParam.m_sPadPoly.m_nPolyCnt > 0 && vPoly[0].size() > 3)
			{
				nLine = __LINE__;
				if (bPOLY_IMG == false)
					cv::fillPoly(ImgPoly, vPoly, cv::Scalar(255));
				g_pInspMng->m_fVolTact[m_eVOL_TACT_FILL_POLY] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				cPAT->POLY_IMG_CHECK(ImgPoly);
				g_pInspMng->m_fVolTact[m_eVOL_TACT_POLY_IMG_CHECK] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				cv::Moments m = cv::moments(ImgPoly, true);
				if (m.m00 != 0 && pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT))
				{
					dPAD_PAT_POLY_X = m.m10 / m.m00;
					dPAD_PAT_POLY_Y = m.m01 / m.m00;
					double dPolyXCen = dPAD_PAT_POLY_X - (nImgWidth / 2.0);
					double dPolyYCen = (nImgHeight / 2.0) - dPAD_PAT_POLY_Y;
					dCenTeachX = dPolyXCen * dResolX;
					dCenTeachY = dPolyYCen * dResolY;
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_POLY_CEN] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				std::vector<std::vector<cv::Point>> vec2Po;
				std::vector<cv::Vec4i> vecH;
				cv::findContours(ImgPoly, vec2Po, vecH, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
				for (int a = 0; a < vec2Po.size(); a++)
				{
					for (int b = 0; b < vec2Po[a].size(); b++)
					{
						if (rcPAD_PAT_POLY.left < 0 || rcPAD_PAT_POLY.left > vec2Po[a][b].x) rcPAD_PAT_POLY.left = vec2Po[a][b].x;
						if (rcPAD_PAT_POLY.right < 0 || rcPAD_PAT_POLY.right < vec2Po[a][b].x) rcPAD_PAT_POLY.right = vec2Po[a][b].x;
						if (rcPAD_PAT_POLY.top < 0 || rcPAD_PAT_POLY.top > vec2Po[a][b].y) rcPAD_PAT_POLY.top = vec2Po[a][b].y;
						if (rcPAD_PAT_POLY.bottom < 0 || rcPAD_PAT_POLY.bottom < vec2Po[a][b].y) rcPAD_PAT_POLY.bottom = vec2Po[a][b].y;
					}
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_POLY_ROI] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				bPadPatternBlob = TRUE;
				m_pProcMilAlgo->SaveWorkImg(ImgPoly.data, nImgWidth, nImgHeight, _T("VOL_Poly.bmp"));
			}

			nLine = __LINE__;

			int nInspectionAreaPer = 100;
			if (pInspAlgoVolume->UseData(m_eVolumeData_InspectionArea))
			{
				nInspectionAreaPer = pInspAlgoVolume->GetDataN(m_eVolumeN_InspectionArea);
				if (nInspectionAreaPer <= 0) nInspectionAreaPer = 1;
				else if (nInspectionAreaPer > 100) nInspectionAreaPer = 100;
			}

			double dRate = m_pCPInsp_Algo->BlobImageStruct_BW2(*pInspAlgoVolume, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, nMinBlobArea,
				&dArea, &dCX, &dCY, &rcBlob, pUcArrDstImg, nTypeSelectBlob, pInspAlgoVolume->m_bFilterIsUse, pInspAlgoVolume->m_nFilterStepNarrow,
				stTieArea, pInspAlgoVolume->m_bFillHole, nWndDir, nInspectionAreaPer);
			m_pProcMilAlgo->SaveWorkImg(pUcArrDstImg, nImgWidth, nImgHeight, _T("VOL_ArrDstImg.bmp"));
			double dVolume = 0.0;
			g_pInspMng->m_fVolTact[m_eVOL_TACT_Blob] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();
			if (dRate > 0)
			{
				nLine = __LINE__;

				if (nTypeSelectBlob == eSelectMix && pInspAlgoVolume->UseData(m_eVolumeData_Shift))
				{
					nLine = __LINE__;
					if (bPadPatternBlob)
						cPAT->INSP_SHIFT_AREA(pInspAlgoVolume, sWndAlgoImg, ImgPoly, pUcArrDstImg, nImgWidth, nImgHeight);
					m_pProcMilAlgo->SaveWorkImg(pUcArrDstImg, nImgWidth, nImgHeight, _T("VOL_INSP_SHIFT_AREA_ArrDstImg.bmp"));
					int nFilter = pInspAlgoVolume->m_bFilterIsUse ? pInspAlgoVolume->m_nFilterStepNarrow : 0;
					cv::Mat imgBlob(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
					int nCntBlob = m_pCPInsp_Algo->CalcBlob(pUcArrDstImg, imgBlob.data, nImgWidth, nImgHeight, nMinBlobArea, pInspAlgoVolume->m_bFillHole, nFilter, nTypeSelectBlob, &dArea, &dCX, &dCY, &rcBlob);
					m_pProcMilAlgo->SaveWorkImg(imgBlob.data, nImgWidth, nImgHeight, _T("VOL_INSP_SHIFT_AREA_Blob.bmp"));
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_INSP_SHIFT_AREA] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				if (ucArrDstImg && stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_PAD)
				{
					const uchar* polyData = ImgPoly.data;
					int totalPixels = nImgHeight * nImgWidth;
					#pragma omp parallel for
					for (int i = 0; i < totalPixels; i++) {
						bool bPAD = polyData[i] == 255;
						bool bBIN = pUcArrDstImg[i] == 255;

						if (bPAD && bBIN)
							ucArrDstImg[i] = 255;
						else if (bPAD)
							ucArrDstImg[i] = 170;
						else if (bBIN)
							ucArrDstImg[i] = 85;
						else
							ucArrDstImg[i] = 0;
					}
					m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("VOL_ArrDstImg_2.bmp"));
				}

				RemoveHight sData;
				sData.m_bBlackHeight = pInspAlgoVolume->m_bBlackHeight;
				sData.m_fRemovalHeight = pInspAlgoVolume->m_fRemovalHeight;
				for (int nIdx = 0; nIdx < VOLUME_INSP_CNTS; nIdx++)
				{
					sData.m_ptrROI[nIdx].x = pInspAlgoVolume->m_poArrRemovalHeightROI[nIdx].x;
					sData.m_ptrROI[nIdx].y = pInspAlgoVolume->m_poArrRemovalHeightROI[nIdx].y;
				}
				nLine = __LINE__;

				dBlackHeight = InspRemoveHeight(sWndAlgoImg, pUcArrDstImg, sData, stTieArea);
				g_pInspMng->m_fVolTact[m_eVOL_TACT_InspRemoveHeight] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();
				int nWhiteCnt = 0;
				double dV = 0;
				double dBlackHeightUM = dBlackHeight * dUM;
				for (int i = 0; i < nImgWidth * nImgHeight; ++i)
				{
					if (pUcArrDstImg[i] != 255)
						continue;

					float fSrcHOrg = pfImgSrc[i];
					float fSrcH = fSrcHOrg / dUM;
					if (sData.m_bBlackHeight == FALSE || (sData.m_bBlackHeight == TRUE && fSrcH > dBlackHeight))	// Removal Height 높이가 설정된 경우, 해당 높이 이상의 볼륨만 측정되도록(BW - Threshold조건으로 White에 해당되더라도 제외)
					{
						dVolume += (fSrcH - dBlackHeight);
						nWhiteCnt++;
						dV += (dResolutionXY * (fSrcH - dBlackHeight));
					}
				}
				dArea = nWhiteCnt;
				dRstVolume = 0;
				if (nWhiteCnt > 0)
				{
					dRstHeight = dVolume / nWhiteCnt;
					dRstArea = dArea * dResolutionXY;
					dRstVolume = dV;
				}

				dRstWidth = rcBlob.Width() * dResolutionX;
				dRstLength = rcBlob.Height() * dResolutionY;

				if (bPadPatternBlob == TRUE && nPatternDivide > 0)
				{
					nLine = __LINE__;

					UCHAR* pImgPoly = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
					memcpy(pImgPoly, ImgPoly.data, nImgWidth * nImgHeight * sizeof(UCHAR));
					bool bDivideHorizon = (nImgWidth < nImgHeight);
					int nArrDivideWH_ST[VOLUME_DIVIDE_AREA_CNTS];
					int nArrDivideWH_ED[VOLUME_DIVIDE_AREA_CNTS];
					int nDivideWH_Offset = bDivideHorizon ? (nImgHeight / (nPatternDivide + 1)) : (nImgWidth / (nPatternDivide + 1));

					for (int nDivideIndex = 0; nDivideIndex < nPatternDivide + 1; nDivideIndex++)
					{
						if (nDivideIndex == 0)
						{
							nArrDivideWH_ST[nDivideIndex] = 0;
							nArrDivideWH_ED[nDivideIndex] = nArrDivideWH_ST[nDivideIndex] + nDivideWH_Offset;
						}
						else if (nDivideIndex == nPatternDivide)
						{
							nArrDivideWH_ST[nDivideIndex] = nArrDivideWH_ED[nDivideIndex - 1];
							nArrDivideWH_ED[nDivideIndex] = bDivideHorizon ? nImgHeight - 1 : nImgWidth - 1;
						}
						else
						{
							nArrDivideWH_ST[nDivideIndex] = nArrDivideWH_ED[nDivideIndex - 1];
							nArrDivideWH_ED[nDivideIndex] = nArrDivideWH_ST[nDivideIndex] + nDivideWH_Offset;
						}

						if (bDivideHorizon)
						{
							if (nArrDivideWH_ST[nDivideIndex] >= nImgHeight)
								nArrDivideWH_ST[nDivideIndex] = nImgHeight - 1;
							if (nArrDivideWH_ED[nDivideIndex] >= nImgHeight)
								nArrDivideWH_ED[nDivideIndex] = nImgHeight - 1;
						}
						else
						{
							if (nArrDivideWH_ST[nDivideIndex] >= nImgWidth)
								nArrDivideWH_ST[nDivideIndex] = nImgWidth - 1;
							if (nArrDivideWH_ED[nDivideIndex] >= nImgWidth)
								nArrDivideWH_ED[nDivideIndex] = nImgWidth - 1;
						}
						if (nArrDivideWH_ST[nDivideIndex] < 0)
							nArrDivideWH_ST[nDivideIndex] = 0;
						if (nArrDivideWH_ED[nDivideIndex] < 0)
							nArrDivideWH_ED[nDivideIndex] = 0;
					}

					for (int nDivideIndex = 0; nDivideIndex < nPatternDivide + 1; nDivideIndex++)
					{
						bool bAdd = false;
						if (bDivideHorizon == true)
						{
							for (int nH = nArrDivideWH_ST[nDivideIndex]; nH < nArrDivideWH_ED[nDivideIndex]; nH++)
							{
								for (int nW = 0; nW < nImgWidth; nW++)
								{
									int nIndex = (nH * nImgWidth) + nW;
									if (pImgPoly[nIndex] == 255)
									{
										bAdd = true;
										break;
									}
								}

								if (bAdd == true)
									break;
							}
						}
						else
						{
							for (int nH = 0; nH < nImgHeight; nH++)
							{
								for (int nW = nArrDivideWH_ST[nDivideIndex]; nW < nArrDivideWH_ED[nDivideIndex]; nW++)
								{
									int nIndex = (nH * nImgWidth) + nW;
									if (pImgPoly[nIndex] == 255)
									{
										bAdd = true;
										break;
									}
								}

								if (bAdd == true)
									break;
							}
						}

						if (bAdd == false)
							continue;

						double dDivideHeight = 0.0;
						dVolume = 0.0;
						nWhiteCnt = 0;
						if (bDivideHorizon == true)
						{
							for (int nH = nArrDivideWH_ST[nDivideIndex]; nH < nArrDivideWH_ED[nDivideIndex]; nH++)
							{
								for (int nW = 0; nW < nImgWidth; nW++)
								{
									int nIndex = (nH * nImgWidth) + nW;
									if (pUcArrDstImg[nIndex] == 255)
									{
										if (sData.m_bBlackHeight == FALSE ||
											sData.m_bBlackHeight == TRUE && pfImgSrc[nIndex] / dUM > dBlackHeight)
										{
											dVolume += pfImgSrc[nIndex];
											nWhiteCnt++;
										}
									}
								}
							}
						}
						else
						{
							for (int nH = 0; nH < nImgHeight; nH++)
							{
								for (int nW = nArrDivideWH_ST[nDivideIndex]; nW < nArrDivideWH_ED[nDivideIndex]; nW++)
								{
									int nIndex = (nH * nImgWidth) + nW;
									if (pUcArrDstImg[nIndex] == 255)
									{
										if (sData.m_bBlackHeight == FALSE ||
											sData.m_bBlackHeight == TRUE && pfImgSrc[nIndex] / dUM > dBlackHeight)
										{
											dVolume += pfImgSrc[nIndex];
											nWhiteCnt++;
										}
									}
								}
							}
						}

						if (sRstAlgo && bAdd == true)
						{
							if (nWhiteCnt > 0)
							{
								dDivideHeight = ((dVolume / dUM) / nWhiteCnt) - dBlackHeight;
								if (dDivideHeight < 0)	dDivideHeight = 0;
							}

							RstPAT_DIVIDE sRst;
							sRst.m_fROIH = dDivideHeight;
							if (bDivideHorizon)
							{
								sRst.m_rcRect.left = 0;
								sRst.m_rcRect.right = nImgWidth - 1;
								sRst.m_rcRect.top = nArrDivideWH_ST[nDivideIndex];
								sRst.m_rcRect.bottom = nArrDivideWH_ED[nDivideIndex];
							}
							else
							{
								sRst.m_rcRect.left = nArrDivideWH_ST[nDivideIndex];
								sRst.m_rcRect.right = nArrDivideWH_ED[nDivideIndex];
								sRst.m_rcRect.top = 0;
								sRst.m_rcRect.bottom = nImgHeight - 1;
							}
							sRstAlgo->AddPAT_DIVIDE(sRst);
						}
					}

					if (pImgPoly)
					{
						g_pMManager->pem_delete(pImgPoly, true);
						pImgPoly = NULL;
					}
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_Divide] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				if (pInspAlgoVolume->UseData(m_eVolumeData_UnderHeightMinArea) || pInspAlgoVolume->UseData(m_eVolumeData_UpperHeightMaxArea) ||
					stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_Under || stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_Upper)
				{
					nLine = __LINE__;

					double dCX_Temp = 0;
					double dCY_Temp = 0;
					double dArea_Temp = 0;
					CRect rcBlob_Temp(0, 0, 0, 0);
					cv::Mat ImgBin(nImgHeight, nImgWidth, CV_8UC1, pUcArrDstImg);

					AlgoBlackWhite algoBW;
					algoBW.m_bInvertCheck = FALSE;
					algoBW.m_b2dCheck = FALSE;
					algoBW.m_b3dCheck = TRUE;

					for (int nH_Area = 0; nH_Area < 2; nH_Area++)
					{
						bool bIMG = stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_Under;
						bool bUSE_H_Area = pInspAlgoVolume->UseData(m_eVolumeData_UnderHeightMinArea) || bIMG;
						algoBW.m_n3dRange = eTypeRangeLower;
						algoBW.m_d3dHeightMin = pInspAlgoVolume->GetData(m_eVolumeF_UnderHeight) * dUM;
						algoBW.m_d3dHeightMax = pInspAlgoVolume->GetData(m_eVolumeF_UnderHeight) * dUM;
						if (nH_Area == 1)
						{
							bIMG = stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_Upper;
							bUSE_H_Area = pInspAlgoVolume->UseData(m_eVolumeData_UpperHeightMaxArea) || bIMG;
							algoBW.m_n3dRange = eTypeRangeUpper;
							algoBW.m_d3dHeightMin = pInspAlgoVolume->GetData(m_eVolumeF_UpperHeight) * dUM;
							algoBW.m_d3dHeightMax = pInspAlgoVolume->GetData(m_eVolumeF_UpperHeight) * dUM;
						}
						if (bUSE_H_Area == false)
							continue;

						cv::Mat imgH_Area(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
						m_pCPInsp_Algo->BlobImageStruct_BW2(algoBW, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, nMinBlobArea, &dArea_Temp, &dCX_Temp, &dCY_Temp, &rcBlob_Temp, imgH_Area.data, eSelectMix, FALSE, 0, stTieArea, FALSE, nWndDir, nInspectionAreaPer);
						cv::Mat ImgAnd;
						cv::bitwise_and(ImgBin, imgH_Area, ImgAnd);

						std::vector<std::vector<cv::Point>> allcontour;
						std::vector<cv::Vec4i> hierarchy;
						cv::findContours(ImgAnd, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
						for (int a = 0; a < allcontour.size(); a++)
						{
							int nBin = cv::contourArea(allcontour[a]);
							cv::Rect boundingRect = cv::boundingRect(allcontour[a]);
							RECT roi;
							roi.left = boundingRect.x;
							roi.right = boundingRect.x + boundingRect.width;
							roi.top = boundingRect.y;
							roi.bottom = boundingRect.y + boundingRect.height;
							sRstAlgo->AddRect(roi);

							double dH_Area = nBin * dResolutionXY;
							double dH_Width = boundingRect.width * dResolutionX;
							double dH_Length = boundingRect.height * dResolutionY;
							if (nH_Area == 0)
							{
								if (a == 0 || dH_Area > dRstUnderHeightArea)
								{
									dRstUnderHeightWidth = dH_Width;
									dRstUnderHeightLength = dH_Length;
									dRstUnderHeightArea = dH_Area;
								}
							}
							else if (nH_Area == 1)
							{
								if (a == 0 || dH_Area > dRstUpperHeightArea)
								{
									dRstUpperHeightWidth = dH_Width;
									dRstUpperHeightLength = dH_Length;
									dRstUpperHeightArea = dH_Area;
								}
							}
						}
						if (ucArrDstImg && bIMG)
						{
							m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("VOL_ArrDstImg_1.bmp"));
							memcpy(ucArrDstImg, ImgAnd.data, sizeof(UCHAR) * nSizeImg);
						}
					}
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_HeightArea] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				if (pInspAlgoVolume->UseData(m_eVolumeData_CircleCheck))
				{
					nLine = __LINE__;

					double diameter = pInspAlgoVolume->GetData(m_eVolumeF_CircleDiameterMin) / m_resolX;
					// 원형 찾기
					cv::Mat ImgBin(nImgHeight, nImgWidth, CV_8UC1, pUcArrDstImg);
					std::vector<CRect> vBlobRects;
					m_pCPInsp_Algo->GetBlobResult_Rects(ImgBin, vBlobRects);
					float fResult(0.0);
					int nMaxIndex(0);
					m_pProcMilAlgo->InspSolderBall(ImgBin, vBlobRects, &nMaxIndex, &fResult);
					if (vBlobRects.size() > nMaxIndex)
					{
						fRstCircleDiameter = vBlobRects[nMaxIndex].Width();
						fRstCirclePercent = fResult * 100.0f;
					}
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_CircleCheck] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();
			}

			if (pInspAlgoVolume->m_bUseColdJoint == TRUE)
			{
				nLine = __LINE__;

				ColdJoint sColdJoint;
				sColdJoint.m_nWndDir = nWndDir;
				sColdJoint.m_nMinBlobArea = nMinBlobArea;
				sColdJoint.m_dArea_T = pInspAlgoVolume->m_dArea_CJ;
				sColdJoint.m_dAreaPer_T = pInspAlgoVolume->m_dAreaPer_CJ;
				//RstColdJoint *sRstColdJoint = new RstColdJoint();
				RstColdJoint* sRstColdJoint = g_pMManager->pem_new<RstColdJoint>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
				InspColdJoint(sWndAlgoImg, pInspAlgoVolume, pUcArrDstImg, sColdJoint, sRstColdJoint, stTieArea);
				if (sRstAlgo)
				{
					sRstAlgo->m_bOKColdJoint = sRstColdJoint->m_nOKColdJoint;
					sRstAlgo->m_dRstArea_CJ = sRstColdJoint->dRstArea_R;
					sRstAlgo->m_dRstAreaPer_CJ = sRstColdJoint->dRstAreaPer_R;
					sRstAlgo->AddRect(sRstColdJoint->m_rcRect_I);
				}

				if (sRstColdJoint)
				{
					g_pMManager->pem_delete(sRstColdJoint, false);
					sRstColdJoint = NULL;
				}
			}
			else
			{
				nLine = __LINE__;

				if (sRstAlgo)
				{
					sRstAlgo->m_bOKColdJoint = TRUE;
					sRstAlgo->m_dRstArea_CJ = 0;
					sRstAlgo->m_dRstAreaPer_CJ = 0;
				}
			}
			g_pInspMng->m_fVolTact[m_eVOL_TACT_ColdJoint] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();
		}
		else
		{
			nLine = __LINE__;

			double dVolume = 0.0;
			for (int i = 0; i < nImgWidth * nImgHeight; ++i)
				dVolume += pfImgSrc[i];
			dRstVolume = (dVolume / dUM) * dResolutionXY;
		}
		if (pInspAlgoVolume->m_bUseInclination)
		{
			nLine = __LINE__;

			sInclination sData;
			sData.m_nWndDir = nWndDir;
			for (int nIdx = 0; nIdx < VOLUME_INSP_CNTS; nIdx++)
			{
				sData.m_ptrROI[nIdx].x = pInspAlgoVolume->m_poArrInclinationROI[nIdx].x;
				sData.m_ptrROI[nIdx].y = pInspAlgoVolume->m_poArrInclinationROI[nIdx].y;
			}
			//RstInclination *sRstData = new RstInclination();
			RstInclination* sRstData = g_pMManager->pem_new<RstInclination>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			InspInclination(sWndAlgoImg, pUcArrDstImg, sData, sRstData);
			if (sRstAlgo)
			{
				float fInclinationMin = pInspAlgoVolume->m_fInclination * (pInspAlgoVolume->m_fInclinationMin / 100.0f);
				float fInclinationMax = pInspAlgoVolume->m_fInclination * (pInspAlgoVolume->m_fInclinationMax / 100.0f);
				float fRstInclination = sRstData->m_fRstInclination;
				sRstAlgo->m_dRstInclination = sRstData->m_fRstInclination;
				if (pInspAlgoVolume->m_fInclination < 0)
				{
					fInclinationMin *= -1;
					fInclinationMax *= -1;
					fRstInclination *= -1;
				}
				sRstAlgo->m_bOKInclination = FALSE;
				if (fInclinationMin < fRstInclination && fInclinationMax > fRstInclination)
					sRstAlgo->m_bOKInclination = TRUE;
				sRstAlgo->m_nLineTotalCnt = sRstData->m_nLineTotalCnt;
				for (int nIdx = 0; nIdx < sRstData->m_nLineTotalCnt; nIdx++)
					sRstAlgo->m_fArrLineHeightAvg[nIdx] = sRstData->m_fArrLineHeightAvg[nIdx];
				sRstAlgo->AddRect(sRstData->m_rcRect_I);
			}

			if (sRstData)
			{
				g_pMManager->pem_delete(sRstData, false);
				sRstData = NULL;
			}
		}
		else
		{
			nLine = __LINE__;

			if (sRstAlgo)
			{
				sRstAlgo->m_bOKInclination = TRUE;
				sRstAlgo->m_dRstInclination = 0;
			}
		}
		g_pInspMng->m_fVolTact[m_eVOL_TACT_Inclination] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

		bResult = TRUE;
		if (sRstAlgo)
		{
			nLine = __LINE__;

			double dStdVolume = pInspAlgoVolume->m_dStdVolume;
			double dAddRefVolume = pInspAlgoVolume->m_dAddRefVolume;

			// 상식적으로 솔더 부피가 100% 일수 없으므로 30%를 유효부피로 본다.
			if (dStdVolume == 0 || pInspAlgoVolume->m_bUseBWOption == FALSE)
				dStdVolume = pInspAlgoVolume->m_dStdBodyHeight * pInspAlgoVolume->m_dStdWidth * pInspAlgoVolume->m_dStdLength * pInspAlgoVolume->m_dStdRateValid;

			dRstVolume = dAddRefVolume + dRstVolume;	//Jig의 Hole 내부 부피값에서 + 측정된 부피 값을 더하기 

			double dPercentOK = pInspAlgoVolume->m_dPercentOK;
			double dLimitUpper = pInspAlgoVolume->m_dLimitUpper;
			double dNoSolderStandardValue = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_NO_SOLDER);

			sRstAlgo->m_dRstVolume = dRstVolume;
			sRstAlgo->m_dRstPercent = (dStdVolume + dAddRefVolume > 0) ? (sRstAlgo->m_dRstVolume / (dStdVolume + dAddRefVolume)) * 100 : 0.0;
			sRstAlgo->m_dRstArea = dRstArea;
			sRstAlgo->m_dRstHeight = dRstHeight;
			sRstAlgo->m_dRstRemoveH = dBlackHeight;
			sRstAlgo->m_dRstWidth = dRstWidth;
			sRstAlgo->m_dRstHeight = dRstHeight;
			sRstAlgo->m_dRstLength = dRstLength;
			sRstAlgo->m_bVolume = FALSE;
			sRstAlgo->m_bNoSolder = FALSE;
			sRstAlgo->m_bExcess = FALSE;
			sRstAlgo->m_nNG = 0;
			for (int a = 0; a < m_eVolumeR_Total; a++)
				sRstAlgo->m_fArr[a] = 0;

			if (pInspAlgoVolume->m_bHeightPer == TRUE)	// unit
			{
				nLine = __LINE__;

				double dHeightMin = pInspAlgoVolume->GetData(m_eVolumeF_HeightMin);
				double dHeightMax = pInspAlgoVolume->GetData(m_eVolumeF_HeightMax);
				double dAreaMin = (pInspAlgoVolume->m_dStdArea * pInspAlgoVolume->m_dMinArea) / 100.0;
				double dAreaMax = (pInspAlgoVolume->m_dStdArea * pInspAlgoVolume->m_dMaxArea) / 100.0;
				double dVolumeMin = dHeightMin * dAreaMin;
				double dVolumeMax = dHeightMax * dAreaMax;
				double dRstV = sRstAlgo->m_dRstVolume;
				if (dRstV < 0) dRstV *= -1.0;

				if (pInspAlgoVolume->m_bUseBWOption == TRUE && pInspAlgoVolume->UseData(m_eVolumeData_VolumeRangeRef) == TRUE)
				{
					dVolumeMin -= pInspAlgoVolume->GetData(m_eVolumeF_VolumeRangeRef);
					dVolumeMax += pInspAlgoVolume->GetData(m_eVolumeF_VolumeRangeRef);
				}

				dPercentOK = (dVolumeMin / dStdVolume) * 100;
				dLimitUpper = (dVolumeMax / dStdVolume) * 100;
				double dVolumeNoSolder = (dVolumeMin < 0.0) ? dVolumeMin * (1.0 + (dNoSolderStandardValue / 100.0)) : dVolumeMin * (dNoSolderStandardValue / 100.0);
				if (dVolumeMin > dRstV)
				{
					sRstAlgo->m_bVolume = TRUE;
					bResult = FALSE;
				}

				if (dRstV < dVolumeNoSolder)
				{
					sRstAlgo->m_bVolume = TRUE;
					sRstAlgo->m_bNoSolder = TRUE;			// 미납
					bResult = FALSE;
				}

				if (pInspAlgoVolume->m_bUseStdMax == TRUE && dVolumeMax < dRstV)
				{
					sRstAlgo->m_bExcess = TRUE;			// 과납
					bResult = FALSE;
				}
			}
			else	// %
			{
				nLine = __LINE__;

				if (pInspAlgoVolume->m_bUseBWOption == TRUE && pInspAlgoVolume->UseData(m_eVolumeData_VolumeRangeRef) == TRUE)
				{
					double dVolumeMin = (dStdVolume * dPercentOK) / 100.0;
					double dVolumeMax = (dStdVolume * dLimitUpper) / 100.0;
					double dVolumeMinAddRef = dVolumeMin - pInspAlgoVolume->GetData(m_eVolumeF_VolumeRangeRef);
					double dVolumeMaxAddRef = dVolumeMax + pInspAlgoVolume->GetData(m_eVolumeF_VolumeRangeRef);
					if (dVolumeMinAddRef < 0.0)
						dVolumeMinAddRef = 0.0;
					dPercentOK = (dVolumeMinAddRef / dStdVolume) * 100.0;
					dLimitUpper = (dVolumeMaxAddRef / dStdVolume) * 100.0;
				}

				if (dPercentOK > sRstAlgo->m_dRstPercent)
				{
					sRstAlgo->m_bVolume = TRUE;
					bResult = FALSE;
				}

				if (sRstAlgo->m_dRstPercent < dNoSolderStandardValue)
				{
					sRstAlgo->m_bVolume = TRUE;
					sRstAlgo->m_bNoSolder = TRUE;			// 미납
					bResult = FALSE;
				}

				if (pInspAlgoVolume->m_bUseStdMax == TRUE && dLimitUpper < sRstAlgo->m_dRstPercent)
				{
					sRstAlgo->m_bExcess = TRUE;			// 과납
					bResult = FALSE;
				}
			}
			if (pInspAlgoVolume->m_bUseBWOption == TRUE)
			{
				nLine = __LINE__;

				double dMinValue = 0.0;
				double dMaxValue = 0.0;
				if (pInspAlgoVolume->m_bUseArea == TRUE)
				{
					dMinValue = (pInspAlgoVolume->m_dStdArea * pInspAlgoVolume->m_dMinArea) / 100.0;
					dMaxValue = (pInspAlgoVolume->m_dStdArea * pInspAlgoVolume->m_dMaxArea) / 100.0;
					if (sRstAlgo->m_dRstArea > dMinValue && sRstAlgo->m_dRstArea < dMaxValue)
						sRstAlgo->m_bArea = FALSE;
					else
					{
						sRstAlgo->m_bArea = TRUE;
						bResult = FALSE;
					}
				}
				else
					sRstAlgo->m_bArea = FALSE;
				if (pInspAlgoVolume->m_bUseHeight == TRUE)
				{
					if (pInspAlgoVolume->m_bHeightPer == TRUE)	// unit
					{
						dMinValue = pInspAlgoVolume->GetData(m_eVolumeF_HeightMin);
						dMaxValue = pInspAlgoVolume->GetData(m_eVolumeF_HeightMax);
					}
					else	// %
					{
						dMinValue = (pInspAlgoVolume->m_dStdBodyHeight * pInspAlgoVolume->m_dMinHeight) / 100.0;
						dMaxValue = (pInspAlgoVolume->m_dStdBodyHeight * pInspAlgoVolume->m_dMaxHeight) / 100.0;

						if (pInspAlgoVolume->m_dStdBodyHeight < 0.0)
						{
							double dTemp = dMinValue;
							dMinValue = dMaxValue;
							dMaxValue = dTemp;
						}
					}

					sRstAlgo->m_bHeight = FALSE;
					if (stAlgoParam.m_bPadPattern == TRUE && nPatternDivide > 0)
					{
						for (int nIdx = 0; nIdx < sRstAlgo->m_nRstPAD_DIVIDE; nIdx++)
						{
							if (sRstAlgo->m_sRstPAD_DIVIDE[nIdx].m_fROIH > dMinValue && sRstAlgo->m_sRstPAD_DIVIDE[nIdx].m_fROIH < dMaxValue)
							{
								sRstAlgo->m_sRstPAD_DIVIDE[nIdx].m_bOK = TRUE;
							}
							else
							{
								sRstAlgo->m_bHeight = TRUE;
								sRstAlgo->m_sRstPAD_DIVIDE[nIdx].m_bOK = FALSE;
								bResult = FALSE;
							}
						}
					}
					else
					{
						if (sRstAlgo->m_dRstHeight > dMinValue && sRstAlgo->m_dRstHeight < dMaxValue)
							sRstAlgo->m_bHeight = FALSE;
						else
						{
							sRstAlgo->m_bHeight = TRUE;
							bResult = FALSE;
						}
					}
				}
				else
					sRstAlgo->m_bHeight = FALSE;

				if (sRstAlgo->m_bOKColdJoint == FALSE)
					bResult = FALSE;

				if (sRstAlgo->m_bOKInclination == FALSE)
					bResult = FALSE;

				double dRstShiftX = 0.0;
				double dRstShiftY = 0.0;
				double dRstShiftXPix = 0.0;
				double dRstShiftYPix = 0.0;
				if (pInspAlgoVolume->UseData(m_eVolumeData_Shift))
				{
					nLine = __LINE__;

					float fPartErrX = sWndAlgoImg.m_fPartRoundingErrX;
					float fPartErrY = sWndAlgoImg.m_fPartRoundingErrY;
					if (stAlgoParam.m_bInspection == FALSE)
					{
						fPartErrX = stAlgoParam.m_sPartAlgoImg->m_fPartRoundingErrX;
						fPartErrY = stAlgoParam.m_sPartAlgoImg->m_fPartRoundingErrY;
					}
					double dCenTeachXPix = dCenTeachX / dResolX;
					double dCenTeachYPix = dCenTeachY / dResolY;
					double dTeachXPix = dCenTeachXPix + (nImgWidth / 2.0);
					double dTeachYPix = (nImgHeight / 2.0) - dCenTeachYPix;
					double dTeachXPixwnd = dTeachXPix - sWndAlgoImg.m_fWndRoundingErrX;
					double dTeachYPixwnd = dTeachYPix + sWndAlgoImg.m_fWndRoundingErrY;
					double dTeachXPixpart = dTeachXPixwnd - fPartErrX;
					double dTeachYPixpart = dTeachYPixwnd + fPartErrY;
					sRstAlgo->m_fArr[m_eVolumeR_Rst_X] = dCX;
					sRstAlgo->m_fArr[m_eVolumeR_Rst_Y] = dCY;
					sRstAlgo->m_fArr[m_eVolumeR_Teach_X] = dTeachXPixpart;
					sRstAlgo->m_fArr[m_eVolumeR_Teach_Y] = dTeachYPixpart;

					bool bShiftXUse = pInspAlgoVolume->UseData(m_eVolumeData_ShiftX);
					bool bShiftYUse = pInspAlgoVolume->UseData(m_eVolumeData_ShiftY);
					double dShiftX = pInspAlgoVolume->GetData(m_eVolumeF_ShiftX);
					double dShiftY = pInspAlgoVolume->GetData(m_eVolumeF_ShiftY);
					if (dWndAngle == 90 || dWndAngle == 270)
					{
						dShiftX = pInspAlgoVolume->GetData(m_eVolumeF_ShiftY);
						dShiftY = pInspAlgoVolume->GetData(m_eVolumeF_ShiftX);
						bShiftXUse = pInspAlgoVolume->UseData(m_eVolumeData_ShiftY);
						bShiftYUse = pInspAlgoVolume->UseData(m_eVolumeData_ShiftX);
					}

					double dRstXparterr = dCX + fPartErrX;
					double dRstYparterr = dCY - fPartErrY;
					double dRstXwnderr = dRstXparterr + sWndAlgoImg.m_fWndRoundingErrX;
					double dRstYwnderr = dRstYparterr - sWndAlgoImg.m_fWndRoundingErrY;
					double dRstXCen = dRstXwnderr - (nImgWidth / 2.0);
					double dRstYCen = (nImgHeight / 2.0) - dRstYwnderr;

					double dRstX = dRstXCen * dResolX;
					double dRstY = dRstYCen * dResolY;

					dRstShiftX = (dRstX - dCenTeachX);
					dRstShiftY = (dRstY - dCenTeachY);

					bool bPlus_X = (dRstShiftX >= 0.0) ? true : false;
					bool bPlus_Y = (dRstShiftY >= 0.0) ? true : false;

					if (pInspAlgoVolume->UseData_M(m_ePAT_ALGO_N_DATA_SHIFT))
					{
						double dBinShiftX_Pix = dCX - dPAD_PAT_POLY_X;
						double dBinShiftY_Pix = dCY - dPAD_PAT_POLY_Y;
						double dBinShiftX = dBinShiftX_Pix * dResolX;
						double dBinShiftY = dBinShiftY_Pix * dResolY;
						if (fabs(dRstShiftX) > fabs(dBinShiftX))
						{
							dRstShiftX = dBinShiftX;
							sRstAlgo->m_fArr[m_eVolumeR_Teach_X] = dPAD_PAT_POLY_X;
							sRstAlgo->m_fArr[m_eVolumeR_Rst_X] = dCX;
						}
						if (fabs(dRstShiftY) > fabs(dBinShiftY))
						{
							dRstShiftY = dBinShiftY;
							sRstAlgo->m_fArr[m_eVolumeR_Teach_Y] = dPAD_PAT_POLY_Y;
							sRstAlgo->m_fArr[m_eVolumeR_Rst_Y] = dCY;
						}

						double dPolyCX = (rcPAD_PAT_POLY.left + rcPAD_PAT_POLY.right) / 2.0;
						double dPolyCY = (rcPAD_PAT_POLY.top + rcPAD_PAT_POLY.bottom) / 2.0;
						double dROICX = (rcBlob.left + rcBlob.right) / 2.0;
						double dROICY = (rcBlob.top + rcBlob.bottom) / 2.0;
						double dROIShiftXPix = dROICX - dPolyCX;
						double dROIShiftYPix = dROICY - dPolyCY;
						double dROIShiftX = dROIShiftXPix * dResolX;
						double dROIShiftY = dROIShiftYPix * dResolY;
						dRstShiftXPix = dROIShiftXPix;
						dRstShiftYPix = dROIShiftYPix;
						if (fabs(dRstShiftX) > fabs(dROIShiftX))
						{
							dRstShiftX = dROIShiftX;
							sRstAlgo->m_fArr[m_eVolumeR_Teach_X] = dPolyCX;
							sRstAlgo->m_fArr[m_eVolumeR_Rst_X] = dROICX;
						}
						if (fabs(dRstShiftY) > fabs(dROIShiftY))
						{
							dRstShiftY = dROIShiftY;
							sRstAlgo->m_fArr[m_eVolumeR_Teach_Y] = dPolyCY;
							sRstAlgo->m_fArr[m_eVolumeR_Rst_Y] = dROICY;
						}

						if (stAlgoParam.m_bInspection == TRUE && pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT_Check))
						{
							if (nCheckINSP == m_eVOL_PAT_RST_OK && pInspAlgoVolume->m_rstPadPatRst.CheckDATA())
							{
								dBinShiftX = pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.top * dResolX;
								dBinShiftY = pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.bottom * dResolY;
								if (fabs(dRstShiftX) > fabs(dBinShiftX))
								{
									dRstShiftX = dBinShiftX;
									sRstAlgo->m_fArr[m_eVolumeR_Teach_X] = dCX + pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.top;
									sRstAlgo->m_fArr[m_eVolumeR_Rst_X] = dCX;
								}
								if (fabs(dRstShiftY) > fabs(dBinShiftY))
								{
									dRstShiftY = dBinShiftY;
									sRstAlgo->m_fArr[m_eVolumeR_Teach_Y] = dCY + pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.bottom;
									sRstAlgo->m_fArr[m_eVolumeR_Rst_Y] = dCY;
								}
							}
						}

								if (bPlus_X && dRstShiftX < 0.0 ||
									bPlus_X == false && dRstShiftX >= 0.0)
								{
									dRstShiftX *= -1.0;
								}

								if (bPlus_Y && dRstShiftY < 0.0 ||
									bPlus_Y == false && dRstShiftY >= 0.0)
								{
									dRstShiftY *= -1.0;
						}
					}
					if (bShiftXUse)
					{
						sRstAlgo->m_fArr[m_eVolumeR_ShiftX] = dRstShiftX * -1;
						if (fabs(sRstAlgo->m_fArr[m_eVolumeR_ShiftX]) > dShiftX)
							sRstAlgo->m_nNG |= m_eVolumeRData_ShiftX;
					}
					if (bShiftYUse)
					{
						sRstAlgo->m_fArr[m_eVolumeR_ShiftY] = dRstShiftY;
						if (fabs(sRstAlgo->m_fArr[m_eVolumeR_ShiftY]) > dShiftY)
							sRstAlgo->m_nNG |= m_eVolumeRData_ShiftY;
					}
				}

				if (pInspAlgoVolume->UseData(m_eVolumeData_CircleCheck))
				{
					nLine = __LINE__;

					if (pInspAlgoVolume->UseData(m_eVolumeData_CircleDiameter))
					{
						double dMin = pInspAlgoVolume->GetData(m_eVolumeF_CircleDiameterMin);
						double dMax = pInspAlgoVolume->GetData(m_eVolumeF_CircleDiameterMax);
						sRstAlgo->m_fArr[m_eVolumeR_Rst_CircleDiameter] = fRstCircleDiameter;
						if (fRstCircleDiameter < dMin || fRstCircleDiameter > dMax)
							sRstAlgo->m_nNG |= m_eVolumeRData_CircleDiameter;
					}

					if (pInspAlgoVolume->UseData(m_eVolumeData_CirclePercent))
					{
						double dMin = pInspAlgoVolume->GetData(m_eVolumeF_CirclePercent);
						sRstAlgo->m_fArr[m_eVolumeR_Rst_CirclePercent] = fRstCirclePercent;
						if (fRstCirclePercent < dMin)
							sRstAlgo->m_nNG |= m_eVolumeRData_CirclePercent;
					}
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_CircleCheck] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				nLine = __LINE__;
				bool bInspPADPatternCheck = true;
				if (stAlgoParam.m_bInspection == TRUE && pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT_Check) && nTYPE_P < m_ePAD_PARALLEL_Type4 && bParallel == false)
				{
					if (nCheckINSP == m_eVOL_PAT_RST_OK)
					{
						bInspPADPatternCheck = false;
						sRstAlgo->m_nOKPadPatternCheck = pInspAlgoVolume->m_rstPadPatRst.m_nOKPadPatternCheck;
						sRstAlgo->m_nPadPatternCheck = pInspAlgoVolume->m_rstPadPatRst.m_nPadPatternCheck;
						sRstAlgo->m_dRstNotOverlapArea = pInspAlgoVolume->m_rstPadPatRst.m_fRstNotOverlapArea;
						sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN] = pInspAlgoVolume->m_rstPadPatRst.m_fRst_TH_MIN;
						sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX] = pInspAlgoVolume->m_rstPadPatRst.m_fRst_TH_MAX;
						sRstAlgo->m_nRstPAD_CHK = 0;
						for (int a = 0; a < pInspAlgoVolume->m_rstPadPatRst.m_nRstPAD_CHK; a++)
							sRstAlgo->AddPAT_CHK(pInspAlgoVolume->m_rstPadPatRst.m_sRstPAD_CHK[a]);
						sRstAlgo->m_nRstPAD_CHK = pInspAlgoVolume->m_rstPadPatRst.m_nRstPAD_CHK;
						for (int a = 0; a < VOLUME_INSP_CNTS; a++)
						{
							sRstAlgo->m_rcRect_TH[a].left = pInspAlgoVolume->m_rstPadPatRst.m_rcRect_TH[a].left;
							sRstAlgo->m_rcRect_TH[a].right = pInspAlgoVolume->m_rstPadPatRst.m_rcRect_TH[a].right;
							sRstAlgo->m_rcRect_TH[a].top = pInspAlgoVolume->m_rstPadPatRst.m_rcRect_TH[a].top;
							sRstAlgo->m_rcRect_TH[a].bottom = pInspAlgoVolume->m_rstPadPatRst.m_rcRect_TH[a].bottom;
						}
						double dFIND_RSTX = pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.top;
						double dFIND_RSTY = pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.bottom;
						sRstAlgo->m_fArr[m_eVolumeR_Rst_PAT_X] = dFIND_RSTX * m_resolX;
						sRstAlgo->m_fArr[m_eVolumeR_Rst_PAT_Y] = dFIND_RSTY * m_resolY;
						if (pInspAlgoVolume->m_rstPadPatRst.m_bPAD_RET == FALSE)
							bResult = FALSE;
						g_pInspMng->m_fVolTact[m_eVOL_TACT_PADPatternCheck_CHK_INSP_OK] += 1;
					}
					else
					{
						g_pInspMng->m_fVolTact[m_eVOL_TACT_PADPatternCheck_CHK_INSP_FAIL] += 1;
						for (int a = m_eVOL_TACT_PADPatternCheck_CHK_INSP_AERA; a < m_eVOL_TACT_PAD_OUT_1; a++)
						{
							if (pInspAlgoVolume->m_rstPadPatRst.InspT(a, nCheckINSP))
								g_pInspMng->m_fVolTact[a] += 1;
						}
					}
				}
				if (bInspPADPatternCheck)
				{
					if (cPAT->PADPatternCheck(sWndAlgoImg, pInspAlgoVolume, stAlgoParam, (RstAlgoVolume*)sRstAlgo, pUcArrDstImg, ucArrDstImg, nImgWidth, nImgHeight, dRstShiftX, dRstShiftY, dRstShiftXPix, dRstShiftYPix) == FALSE)
						bResult = FALSE;
				}
				g_pInspMng->m_fVolTact[m_eVOL_TACT_PADPatternCheck] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

				if (pInspAlgoVolume->UseData(m_eVolumeData_UnderHeightMinArea))
				{
					nLine = __LINE__;

					sRstAlgo->m_fArr[m_eVolumeR_Rst_UnderHeightWidth] = dRstUnderHeightWidth;
					sRstAlgo->m_fArr[m_eVolumeR_Rst_UnderHeightLength] = dRstUnderHeightLength;
					sRstAlgo->m_fArr[m_eVolumeR_Rst_UnderHeightArea] = dRstUnderHeightArea;
					if (sRstAlgo->m_fArr[m_eVolumeR_Rst_UnderHeightWidth] > pInspAlgoVolume->GetData(m_eVolumeF_UnderHeightWidthMax))
					{
						sRstAlgo->m_nNGSub |= m_eVolumeRData_UnderHeightMinWidth;
					}
					if (sRstAlgo->m_fArr[m_eVolumeR_Rst_UnderHeightLength] > pInspAlgoVolume->GetData(m_eVolumeF_UnderHeightLengthMax))
					{
						sRstAlgo->m_nNGSub |= m_eVolumeRData_UnderHeightMinLength;
					}

					if (pInspAlgoVolume->GetData(m_eVolumeF_UnderHeightAreaMax) > 0.0f)
					{
						if (sRstAlgo->m_fArr[m_eVolumeR_Rst_UnderHeightArea] > pInspAlgoVolume->GetData(m_eVolumeF_UnderHeightAreaMax))
						{
							sRstAlgo->m_nNGSub |= m_eVolumeRData_UnderHeightMinArea;
						}
					}

					if (!sRstAlgo->OKSubData(m_eVolumeRData_UnderHeightMinWidth) && !sRstAlgo->OKSubData(m_eVolumeRData_UnderHeightMinLength) || !sRstAlgo->OKSubData(m_eVolumeRData_UnderHeightMinArea))
					{
						sRstAlgo->m_nNG |= m_eVolumeRData_UnderHeightMin;
						bResult = FALSE;
					}
				}

				if (pInspAlgoVolume->UseData(m_eVolumeData_UpperHeightMaxArea))
				{
					nLine = __LINE__;

					sRstAlgo->m_fArr[m_eVolumeR_Rst_UpperHeightWidth] = dRstUpperHeightWidth;
					sRstAlgo->m_fArr[m_eVolumeR_Rst_UpperHeightLength] = dRstUpperHeightLength;
					sRstAlgo->m_fArr[m_eVolumeR_Rst_UpperHeightArea] = dRstUpperHeightArea;
					if (sRstAlgo->m_fArr[m_eVolumeR_Rst_UpperHeightWidth] > pInspAlgoVolume->GetData(m_eVolumeF_UpperHeightWidthMax))
					{
						sRstAlgo->m_nNGSub |= m_eVolumeRData_UpperHeightMaxWidth;
					}
					if (sRstAlgo->m_fArr[m_eVolumeR_Rst_UpperHeightLength] > pInspAlgoVolume->GetData(m_eVolumeF_UpperHeightLengthMax))
					{
						sRstAlgo->m_nNGSub |= m_eVolumeRData_UpperHeightMaxLength;
					}

					if (pInspAlgoVolume->GetData(m_eVolumeF_UpperHeightAreaMax) > 0.0f)
					{
						if (sRstAlgo->m_fArr[m_eVolumeR_Rst_UpperHeightArea] > pInspAlgoVolume->GetData(m_eVolumeF_UpperHeightAreaMax))
						{
							sRstAlgo->m_nNGSub |= m_eVolumeRData_UpperHeightMaxArea;
						}
					}

					if (!sRstAlgo->OKSubData(m_eVolumeRData_UpperHeightMaxWidth) && !sRstAlgo->OKSubData(m_eVolumeRData_UpperHeightMaxLength) || !sRstAlgo->OKSubData(m_eVolumeRData_UpperHeightMaxArea))
					{
						sRstAlgo->m_nNG |= m_eVolumeRData_UpperHeightMax;
						bResult = FALSE;
					}
				}

				if (pInspAlgoVolume->UseData(m_eVolumeData_Width))
				{
					nLine = __LINE__;

					sRstAlgo->m_dRstWidth = dRstWidth;
					double dRstWidthPer = pInspAlgoVolume->m_dStdWidth > 0 ? (dRstWidth / pInspAlgoVolume->m_dStdWidth) * 100.0 : 0.0;
					if (dRstWidthPer < pInspAlgoVolume->GetData(m_eVolumeF_WidthMinPer) || dRstWidthPer > pInspAlgoVolume->GetData(m_eVolumeF_WidthMaxPer))
					{
						sRstAlgo->m_nNG |= m_eVolumeRData_Width;
						bResult = FALSE;
					}
				}

				if (pInspAlgoVolume->UseData(m_eVolumeData_Length))
				{
					nLine = __LINE__;
					sRstAlgo->m_dRstLength = dRstLength;
					double dRstLengthPer = pInspAlgoVolume->m_dStdLength > 0 ? (dRstLength / pInspAlgoVolume->m_dStdLength) * 100.0 : 0.0;
					if (dRstLengthPer < pInspAlgoVolume->GetData(m_eVolumeF_LengthMinPer) || dRstLengthPer > pInspAlgoVolume->GetData(m_eVolumeF_LengthMaxPer))
					{
						sRstAlgo->m_nNG |= m_eVolumeRData_Length;
						bResult = FALSE;
					}
				}
			}

			if (sRstAlgo->m_nNG > 0)
				bResult = FALSE;

			sRstAlgo->m_bPadPatternAlgo = stAlgoParam.m_bPadPattern;
		}
		nLine = __LINE__;
		if (ucArrDstImg && stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_Volume)
		{
			m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("VOL_ArrDstImg_2.bmp"));
			memcpy(ucArrDstImg, pUcArrDstImg, sizeof(UCHAR) * nSizeImg);
		}
		Delete_1DArray(&pUcArrDstImg);
		g_pInspMng->m_fVolTact[m_eVOL_TACT_Result] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_AlgoVolume::InspVolume(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return bResult;
}

int CPInsp_AlgoVolume::VOLUME_PARALLEL(INSP_PAD_PAT sInspData, INSP_PAD_PAT_RST* pPAT_RST, void * sRstAlgo)
{
	int nORG_W = sInspData.m_nORG_W;
	int nORG_H = sInspData.m_nORG_H;
	if (sInspData.m_ucBIN == NULL || sInspData.m_ucGRAY == NULL || sInspData.m_f3D == NULL)
		return -1;
	else if (sInspData.m_nORG_W <= PAT_INSP_MIN_SIZE || sInspData.m_nORG_H <= PAT_INSP_MIN_SIZE)
		return -2;
	else if (pPAT_RST == NULL)
		return -3;

	pPAT_RST->Init();
	cv::Mat imgGRAY_ORG(nORG_H, nORG_W, CV_8UC1, sInspData.m_ucGRAY);
	cv::Mat imgBIN_ORG(nORG_H, nORG_W, CV_8UC1, sInspData.m_ucBIN);
	cv::Mat img3D_ORG(nORG_H, nORG_W, CV_32FC1, sInspData.m_f3D);
	bool bCalc3D = sInspData.UseData(m_eINSP_PAD_PAT_Calc3D);
	if (sInspData.m_nL < 0 || sInspData.m_nL >= nORG_W ||
		sInspData.m_nT < 0 || sInspData.m_nT >= nORG_H)
		return -4;
	else if (sInspData.m_nW <= PAT_INSP_MIN_SIZE || sInspData.m_nH <= PAT_INSP_MIN_SIZE)
		return -5;
	else if (sInspData.m_nL + sInspData.m_nW > nORG_W || sInspData.m_nT + sInspData.m_nH > nORG_H)
		return -6;
	else if (bCalc3D == false)
		return -7;

	InspAlgo sInspAlgo = sInspData.m_sInspAlgo;

	cv::Rect roiCLIP(sInspData.m_nL, sInspData.m_nT, sInspData.m_nW, sInspData.m_nH);
	int nW = roiCLIP.width;
	int nH = roiCLIP.height;

	cv::Mat imgGRAY = imgGRAY_ORG(roiCLIP).clone();
	WndAlgoImg sWndAlgoImg;
	sWndAlgoImg.m_ucArr2D = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	sWndAlgoImg.m_fArr3D = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	sWndAlgoImg.m_nWidth = nW;
	sWndAlgoImg.m_nHeight = nH;
	sWndAlgoImg.dAngle = sInspData.m_dAngle;
	memcpy(sWndAlgoImg.m_ucArr2D, imgGRAY.data, sizeof(UCHAR) * nW * nH);
	m_proc3d.GetCropZmap_LT(sInspData.m_f3D, sWndAlgoImg.m_fArr3D, nORG_W, nORG_H, roiCLIP.tl().x, roiCLIP.tl().y, nW, nH);

#if _DEBUG
	cv::Mat ImgBin = imgBIN_ORG(roiCLIP).clone();
	cv::Mat img3D(nH, nW, CV_32FC1, sWndAlgoImg.m_fArr3D);
#endif

	double dX = sInspData.m_nL;
	double dY = sInspData.m_nT;
	int nWndDir = e_LEFT;
	if (nORG_W > nORG_H)
	{
		if (nORG_W / 2 > dX + (nW / 2))
			nWndDir = e_LEFT;
		else
			nWndDir = e_RIGHT;
	}
	else
	{
		if (nORG_H / 2 > dY + (nH / 2))
			nWndDir = e_TOP;
		else
			nWndDir = e_BOTTOM;
	}

	InspAlgoParam stAlgoParam;
	//
	stAlgoParam.m_dx = dX;
	stAlgoParam.m_dy = dY;
	//
	stAlgoParam.m_bInspection = TRUE;
	//
	stAlgoParam.m_sPartAlgoImg = &sWndAlgoImg;
	//
	if (sInspData.m_bPadPattern == TRUE)
	{
		stAlgoParam.m_bPadPattern = TRUE;
		stAlgoParam.m_sPadPoly = sInspData.m_sPadPoly;
		stAlgoParam.m_nOverlapPadPolyCnt = sInspData.m_nOverlapPadPolyCnt;
		stAlgoParam.m_sArrOverlapPadPoly = sInspData.m_sArrOverlapPadPoly;
	}

	// 통합 검사/제외 영역 생성
	TotalInspExceptArea stTieArea;
	stTieArea.m_nUsedWndPolygon = sInspData.m_nUsedWndPolygon;
	for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		stTieArea.m_ptArrWndPolygon[i] = sInspData.m_ptArrWndPolygon[i];

	UCHAR* ucArrDstImg = NULL;
	BOOL bRET = InspVolume(sInspAlgo, sWndAlgoImg, stAlgoParam, (RstAlgoVolume *)sRstAlgo, stTieArea, ucArrDstImg, true);

	pPAT_RST->m_bVolume_RET = bRET;
	pPAT_RST->m_bVolume_PARALLEL = TRUE;

	sWndAlgoImg.Destroy();
	return 1;
}

BOOL CPInsp_AlgoVolume::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	if (inspType == eINSP_MOUNT || inspType == eINSP_ALIGN)
	{
		AlgoVolume *pAlgo = (AlgoVolume *)sInspAlgo.m_ptrInspAlgoParam;
		pAlignRes->TeachCenterX = pAlgo->GetData(m_eVolumeF_CenterX);
		pAlignRes->TeachCenterY = pAlgo->GetData(m_eVolumeF_CenterY);

		RstAlgoVolume * rst = (RstAlgoVolume *)sRstAlgo;
		pAlignRes->offsetX = -rst->m_fArr[m_eVolumeR_ShiftX];
		pAlignRes->offsetY = rst->m_fArr[m_eVolumeR_ShiftY];
		pAlignRes->theta = 0;
		bRet = TRUE;
	}

	return bRet;
}
int CPInsp_AlgoVolume::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoVolume::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}