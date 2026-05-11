#include "PInsp_AlgoAlignEdge.h"

#define AlignEdgeDebug 0
CPInsp_AlgoAlignEdge::CPInsp_AlgoAlignEdge(void)
{
}


CPInsp_AlgoAlignEdge::~CPInsp_AlgoAlignEdge(void)
{
}

void CPInsp_AlgoAlignEdge::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoAlignEdge::GetInspAlgoData()
{
	return eSPCAlgoAlignEdge;
}

int CPInsp_AlgoAlignEdge::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoAlignEdge * rstAlgo = (RstAlgoAlignEdge *)vRstInspAlgo;
	if (rstAlgo->m_bMissing_1 || rstAlgo->m_bMissing_2 || rstAlgo->m_bMissing_3)
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
	if (!rstAlgo->m_bOKDistance || !rstAlgo->m_bOKDistanceX || !rstAlgo->m_bOKDistanceY)
	{
		eWholeNgTypeTemp[TypeLength] = e_NG;
		nCurrentNgType = TypeLength;
	}
	if (!rstAlgo->m_bOK3PtDis1 || !rstAlgo->m_bOK3PtDis3)
	{
		eWholeNgTypeTemp[TypeAlignWrong] = e_NG;
		nCurrentNgType = TypeAlignWrong;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoAlignEdge::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoAlignEdge::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	if (stAlgoParam.m_bUSeLeadAlign == FALSE)
	{
		bResult = InspAlignEdge(sInspAlgo, sWndAlgoImg, sInspImageData, (RstAlgoAlignEdge *)sRstAlgo, stAlgoParam.m_nSelectArea, ucArrDstImg, stAlgoParam.m_sAlignRes, stAlgoParam.m_nAlignCnt, stAlgoParam.m_bTeach);

		if (stAlgoParam.m_bInspection)
		{
			AlgoAlignEdge *pAlignEdge = (AlgoAlignEdge *)sInspAlgo.m_ptrInspAlgoParam;
			RstAlgoAlignEdge * rst = (RstAlgoAlignEdge *)sRstAlgo;
			if (pAlignEdge->m_bUseAnchor && stAlgoParam.m_sAlignRes != NULL)
			{
				double CvtBoradX = 0;
				double CvtBoradY = 0;

				CvtBoradX = ((rst->m_poDrawCenter.x - (float)(sWndAlgoImg.m_nWidth / 2.)) * m_resolX) - rst->m_dRstShiftX;
				CvtBoradY = (((float)(sWndAlgoImg.m_nHeight / 2.) - (rst->m_poDrawCenter.y)) * m_resolY) - rst->m_dRstShiftY;

				if (stAlgoParam.m_sAlignRes != NULL)
				{
					rst->m_dRstShiftX = CvtBoradX - stAlgoParam.m_sAlignRes->centerX;
					rst->m_dRstShiftY = CvtBoradY - stAlgoParam.m_sAlignRes->centerY;
				}
			}
			else if (pAlignEdge->m_bUseTwoAnchor && stAlgoParam.m_sAlignRes != NULL)
			{
				//
				CString csWindowID;
				csWindowID.Format(_T("%s"), pAlignEdge->m_sAlignWindow);
				if (csWindowID.IsEmpty() != TRUE)
				{
					AlignResult * ptrAlignRes2 = NULL;
					int nSize = _msize(stAlgoParam.m_arrAlignRes) / sizeof(stAlgoParam.m_arrAlignRes[0]);
					ptrAlignRes2 = g_pMManager->pem_new<AlignResult>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
					int WindowId = 0;

					if (csWindowID.GetLength() == 7)
						WindowId = _ttoi(csWindowID.Right(1));
					else
						WindowId = _ttoi(csWindowID.Right(2));

					g_pInspMng->FindAlignResultData(stAlgoParam.m_nWndIndex, stAlgoParam.m_arrAlignRes, ptrAlignRes2, &stAlgoParam.m_nAlignCnt, WindowId);

					double CvtBoradX = 0;
					double CvtBoradY = 0;
					double CenterX = 0;
					double CenterY = 0;

					CvtBoradX = ((rst->m_poDrawCenter.x - (float)(sWndAlgoImg.m_nWidth / 2.)) * m_resolX) - rst->m_dRstShiftX;
					CvtBoradY = (((float)(sWndAlgoImg.m_nHeight / 2.) - (rst->m_poDrawCenter.y)) * m_resolY) - rst->m_dRstShiftY;

					if (stAlgoParam.m_sAlignRes != NULL && ptrAlignRes2 != NULL)
					{
						POINTF poTarget;
						POINTF poAnchorX;
						POINTF poAnchorY;

						poTarget.x = CvtBoradX;
						poTarget.y = CvtBoradY;
						poAnchorX.x = stAlgoParam.m_sAlignRes->centerX;
						poAnchorX.y = stAlgoParam.m_sAlignRes->centerY;
						poAnchorY.x = ptrAlignRes2->centerX;
						poAnchorY.y = ptrAlignRes2->centerY;

						double CosValue = (poAnchorY.y - poAnchorX.y) / sqrt(pow(poAnchorY.x - poAnchorX.x, 2) + pow(poAnchorY.y - poAnchorX.y, 2));
						double SinValue = (poAnchorY.x - poAnchorX.x) / sqrt(pow(poAnchorY.x - poAnchorX.x, 2) + pow(poAnchorY.y - poAnchorX.y, 2));

						CenterX = poTarget.x * CosValue - poTarget.y * SinValue - poAnchorX.x;
						CenterY = poTarget.x * SinValue + poTarget.y * CosValue - poAnchorX.y;

						rst->m_dRstShiftX = CenterX;
						rst->m_dRstShiftY = CenterY;
					}
					Delete_1DArray(&ptrAlignRes2);
				}
			}
		}
	}
	else
		bResult = InspAlignEdge(sInspAlgo, sWndAlgoImg, sInspImageData, (RstAlgoAlignEdge *)sRstAlgo);
	return bResult;
}

BOOL CPInsp_AlgoAlignEdge::GetIntersectPoint2D(POINTF & x1, POINTF & x2, POINTF & x3, POINTF & x4, POINTF * pResult)
{
	float fInc1, fConst1, fSameval1;
	float fInc2, fConst2, fSameval2;

	//line1 
	if (x1.x == x2.x)
		fSameval1 = x1.x;
	else
	{
		fInc1 = (float)(x2.y - x1.y) / (x2.x - x1.x);
		fConst1 = x1.y - fInc1 * x1.x;
	}

	//line 2 
	if (x3.x == x4.x)
		fSameval2 = x3.x;
	else
	{
		fInc2 = (float)(x4.y - x3.y) / (x4.x - x3.x);
		fConst2 = x3.y - fInc2 * x3.x;
	}

	//cross Point
	if (x1.x == x2.x && x3.x == x4.x)		//평행할경우 교점 없음
		return FALSE;

	if (x1.x == x2.x)
	{
		pResult->x = fSameval1;
		pResult->y = fInc2 * fSameval1 * fConst2;
	}
	else if (x3.x == x4.x)
	{
		pResult->x = fSameval2;
		pResult->y = fInc1 * fSameval2 * fConst1;
	}
	else
	{
		pResult->x = -(fConst1 - fConst2) / (fInc1 - fInc2);
		pResult->y = fInc1 * pResult->x + fConst1;
	}

	return true;
}
BOOL CPInsp_AlgoAlignEdge::InspAlignEdge(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sArrInspImageData, RstAlgoAlignEdge *sRstAlgo, int nSelectArea, UCHAR* ucArrDstImg, AlignResult * sAlignRes, int nAlignCnt, BOOL bTeach, BOOL bSideCamera)
{
	BOOL  bResult = TRUE;
	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoAlignEdge));
		sRstAlgo->Init();
	}

	CString sLog = _T("");
	// 	sLog.Format(_T("Func_InspAlignEdge, in "));
	// 	g_pMPTI->AddLog_Dev(sLog);
	int nLine = __LINE__;
	try
	{
		double dWndAngle = sWndAlgoImg.dAngle;
		UCHAR *pucImgSrc[3];
		pucImgSrc[0] = sWndAlgoImg.m_ucArr2D;
		pucImgSrc[1] = sWndAlgoImg.m_ucArr2D;
		pucImgSrc[2] = sWndAlgoImg.m_ucArr2D;
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if ((pucImgSrc[0] == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
			return FALSE;
		if (sInspAlgo.m_eAlgoType != eAlgoAlignEdge)
			return FALSE;
		if (!m_pProcMilAlgo)
			return FALSE;
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("AlignEdge_3DORG.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc[0], nImgWidth, nImgHeight, _T("AlignEdge_2DORG.bmp"));

#if _DEBUG
		cv::Mat cvPartOrgImage(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc[0]);
		cv::imwrite(std::string("D:\\SrcPartImgGray.bmp"), cvPartOrgImage);
#endif

		CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
		nLine = __LINE__;
		AlgoAlignEdge * pInspAlgoAlignEdge = (AlgoAlignEdge *)sInspAlgo.m_ptrInspAlgoParam;
		double dRstCx[ALIGNEDGE_AREA_CNTS];
		double dRstCy[ALIGNEDGE_AREA_CNTS];
		double dRstCxBlob[ALIGNEDGE_AREA_CNTS];
		double dRstCyBlob[ALIGNEDGE_AREA_CNTS];
		double dRstCxROI[ALIGNEDGE_AREA_CNTS];
		double dRstCyROI[ALIGNEDGE_AREA_CNTS];
		double dTeachCx[ALIGNEDGE_AREA_CNTS];
		double dTeachCy[ALIGNEDGE_AREA_CNTS];
		int nArrLeft[ALIGNEDGE_AREA_CNTS];
		int nArrTop[ALIGNEDGE_AREA_CNTS];
		nLine = __LINE__;
		BOOL bArrIsHorizon[ALIGNEDGE_AREA_CNTS][EdgeLineTotalCnt];
		POINTF poArrSetTeachCenter[ALIGNEDGE_AREA_CNTS][EdgeLineTotalCnt];
		double dArrLineDAValue[ALIGNEDGE_AREA_CNTS][EdgeLineTotalCnt];
		double dArrLineDBValue[ALIGNEDGE_AREA_CNTS][EdgeLineTotalCnt];
		double dArrArea[ALIGNEDGE_AREA_CNTS];
		memset(dRstCx, 0, sizeof(double) * ALIGNEDGE_AREA_CNTS);
		memset(dRstCy, 0, sizeof(double) * ALIGNEDGE_AREA_CNTS);
		memset(dTeachCx, 0, sizeof(double) * ALIGNEDGE_AREA_CNTS);
		memset(dTeachCy, 0, sizeof(double) * ALIGNEDGE_AREA_CNTS);
		memset(nArrLeft, 0, sizeof(int) * ALIGNEDGE_AREA_CNTS);
		memset(nArrTop, 0, sizeof(int) * ALIGNEDGE_AREA_CNTS);
		memset(dArrArea, 0, sizeof(double) * ALIGNEDGE_AREA_CNTS);

		nLine = __LINE__;
#if AlignEdgeDebug
		cv::Mat Image0;
		cv::Mat Image1;
		cv::Mat Image2;
#endif
		bool ucDstImgNull = false;
		if (pInspAlgoAlignEdge->m_nAreaCnt > 2 && sWndAlgoImg.m_ucArr2D_Mix[0] != NULL && sWndAlgoImg.m_ucArr2D_Mix[1] != NULL &&
			sInspAlgo.m_nMixCount > 0)
		{
			pucImgSrc[1] = sWndAlgoImg.m_ucArr2D_Mix[0];
			pucImgSrc[2] = sWndAlgoImg.m_ucArr2D_Mix[1];
#if AlignEdgeDebug
			cv::Mat Img0(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc[0]);
			cv::Mat Img1(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc[1]);
			cv::Mat Img2(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc[2]);
			cv::imwrite("D:\\testimage\\AlignEdge_Img0.bmp", Img0);
			cv::imwrite("D:\\testimage\\AlignEdge_Img1.bmp", Img1);
			cv::imwrite("D:\\testimage\\AlignEdge_Img2.bmp", Img2);
			Image0 = Img0.clone();
			Image1 = Img1.clone();
			Image2 = Img2.clone();
			pucImgSrc[0] = Image0.data;
			pucImgSrc[1] = Image1.data;
			pucImgSrc[2] = Image2.data;
			if (ucArrDstImg == NULL)
			{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrDstImg, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
				ucDstImgNull = true;
			}
#endif
		}
		if (pInspAlgoAlignEdge->m_nAreaCnt > 1 && sWndAlgoImg.m_ucArr2D_Mix[0] != NULL && sInspAlgo.m_nMixCount > 0)
			pucImgSrc[1] = sWndAlgoImg.m_ucArr2D_Mix[0];
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc[0], nImgWidth, nImgHeight, _T("AlignEdge_2D_1.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc[1], nImgWidth, nImgHeight, _T("AlignEdge_2D_2.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc[2], nImgWidth, nImgHeight, _T("AlignEdge_2D_3.bmp"));
		nLine = __LINE__;
		cv::RotatedRect FiduRect[3];
		int nArrIDX[ALIGNEDGE_AREA_CNTS] = { 0, 1, 2 };
		if (ucArrDstImg != NULL)
		{
			if (nSelectArea == 0)
			{
				nArrIDX[0] = 1;
				nArrIDX[1] = 2;
				nArrIDX[2] = 0;
			}
			else if (nSelectArea == 1)
			{
				nArrIDX[0] = 0;
				nArrIDX[1] = 2;
				nArrIDX[2] = 1;
			}
			nLine = __LINE__;
			memset(ucArrDstImg, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
		}

		std::vector<cv::Point2f> vecEdgeLineStartPo;
		std::vector<cv::Point2f> vecEdgeLineEndPo;
		std::vector<bool> vecbIsHorizonLine;

		for (int a = 0; a < ALIGNEDGE_AREA_CNTS; a++)		//설정된 영역개수만큼 반복
		{
			int nAreaIdx = nArrIDX[a];
			nLine = __LINE__;
			POINTF rcDstPoint;
			if (m_pCPInsp_Algo->AnglePointChange(nImgWidth, nImgHeight, pInspAlgoAlignEdge->m_sArrSearchPoint[nAreaIdx], &rcDstPoint) == FALSE)
				continue;
			if (nAreaIdx + 1 > pInspAlgoAlignEdge->m_nAreaCnt)
				continue;

			int nSearchSizeX = (int)pInspAlgoAlignEdge->m_sArrSearchSize[nAreaIdx].cx;
			int nSearchSizeY = (int)pInspAlgoAlignEdge->m_sArrSearchSize[nAreaIdx].cy;
			if (dWndAngle == 90 || dWndAngle == 270)
			{
				nSearchSizeX = (int)pInspAlgoAlignEdge->m_sArrSearchSize[nAreaIdx].cy;
				nSearchSizeY = (int)pInspAlgoAlignEdge->m_sArrSearchSize[nAreaIdx].cx;
			}
			if (nSearchSizeX <= 0 || nSearchSizeY <= 0)
				continue;
			UCHAR * ucArrSearch = NULL;
			float * fArrSearch = NULL;
			UCHAR * ucArrSearchDst = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearch, nSearchSizeX * nSearchSizeY);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrSearch, nSearchSizeX * nSearchSizeY);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearchDst, nSearchSizeX * nSearchSizeY);
#if _DEBUG
			cv::Mat imgArrSearch(nSearchSizeY, nSearchSizeX, CV_8UC1, ucArrSearch);
			cv::Mat imgArrSearchDst(nSearchSizeY, nSearchSizeX, CV_8UC1, ucArrSearchDst);
#endif
			memset(ucArrSearch, 0, nSearchSizeX * nSearchSizeY * sizeof(UCHAR));
			memset(fArrSearch, 0, nSearchSizeX * nSearchSizeY * sizeof(float));
			memset(ucArrSearchDst, 0, nSearchSizeX * nSearchSizeY * sizeof(UCHAR));
			nArrLeft[nAreaIdx] = RounD(rcDstPoint.x - nSearchSizeX / 2.);
			nArrTop[nAreaIdx] = RounD(rcDstPoint.y - nSearchSizeY / 2.);
			nLine = __LINE__;

			if (nAlignCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
			{
				nArrLeft[nAreaIdx] = nArrLeft[nAreaIdx] + (nSearchSizeX / 2.0) - ((double)nImgWidth / 2.0);
				nArrTop[nAreaIdx] = ((double)nImgHeight / 2.0) - (nArrTop[nAreaIdx] + (nSearchSizeY / 2.0));

				double corr_x = 0, corr_y = 0;
				for (int n = 0; n < nAlignCnt; n++)
				{
					double corr_x_Buf = 0, corr_y_Buf = 0;
					nLine = __LINE__;
					int nCorrectCoordinate = m_proc3d.CorrectCoordinate(nArrLeft[nAreaIdx], nArrTop[nAreaIdx], sAlignRes[n].centerX / m_resolX, sAlignRes[n].centerY / m_resolY, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
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
				nArrLeft[nAreaIdx] = RounD(corr_x + ((double)nImgWidth / 2.0) - (nSearchSizeX / 2.0));
				nArrTop[nAreaIdx] = RounD(((double)nImgHeight / 2.0) - (corr_y + (nSearchSizeY / 2.0)));
			}
			if (nArrLeft[nAreaIdx] < 0)
			{
				nSearchSizeX += nArrLeft[nAreaIdx];
				nArrLeft[nAreaIdx] = 0;
			}
			if (nArrTop[nAreaIdx] < 0)
			{
				nSearchSizeY += nArrTop[nAreaIdx];
				nArrTop[nAreaIdx] = 0;
			}
			if (nArrLeft[nAreaIdx] + nSearchSizeX > nImgWidth)
				nSearchSizeX = nImgWidth - nArrLeft[nAreaIdx];
			if (nArrTop[nAreaIdx] + nSearchSizeY > nImgHeight)
				nSearchSizeY = nImgHeight - nArrTop[nAreaIdx];

			nLine = __LINE__;
			bool bContinue = false;
			if (nArrLeft[nAreaIdx] < 0 || nArrLeft[nAreaIdx] >= nImgWidth)
				bContinue = true;
			if (nArrTop[nAreaIdx] < 0 || nArrTop[nAreaIdx] >= nImgHeight)
				bContinue = true;
			if (nArrLeft[nAreaIdx] + nSearchSizeX >= nImgWidth)
			{
				nSearchSizeX = nImgWidth - nArrLeft[nAreaIdx] - 1;
				if (nSearchSizeX < 0)
					bContinue = true;
			}
			if (nArrTop[nAreaIdx] + nSearchSizeY >= nImgHeight)
			{
				nSearchSizeY = nImgHeight - nArrTop[nAreaIdx] - 1;
				if (nSearchSizeY < 0)
					bContinue = true;
			}
			if (bContinue == true)
			{
				Delete_1DArray(&ucArrSearch);
				Delete_1DArray(&fArrSearch);
				Delete_1DArray(&ucArrSearchDst);
				continue;
			}

			if(sWndAlgoImg.m_nLight_index ==(int)eMSCN_COAXIAL)
				m_proc3d.GetCropZmap_LT(pfImgSrc, fArrSearch, nImgWidth, nImgHeight, nArrLeft[nAreaIdx], nArrTop[nAreaIdx], nSearchSizeX, nSearchSizeY);
			m_pProcMilAlgo->GetClipImage_LT(pucImgSrc[nAreaIdx], nImgWidth, nImgHeight, ucArrSearch, nArrLeft[nAreaIdx], nArrTop[nAreaIdx], nSearchSizeX, nSearchSizeY);
			m_pProcMilAlgo->SaveWorkImg_float(fArrSearch, nSearchSizeX, nSearchSizeY, _T("AlignEdge_3D.bmp"));
			m_pProcMilAlgo->SaveWorkImg(ucArrSearch, nSearchSizeX, nSearchSizeY, _T("AlignEdge_2D.bmp"));
			UCHAR * ucArrSearchColor = NULL;
			bool bUseColor = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_sAlgoColorBase.m_bUseColor;
			if (pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bGroup == TRUE)
				bUseColor = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_sAlgoColorBase.m_bUseColor;
			int nImageSizeX = 0, nImageSizeY = 0;
			if (sArrInspImageData != NULL)
			{
				if (pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bGroup == TRUE)
				{
					nImageSizeX = sArrInspImageData[0].nImageSizeX;
					nImageSizeY = sArrInspImageData[0].nImageSizeY;
				}
				else
				{
					nImageSizeX = sArrInspImageData[nAreaIdx].nImageSizeX;
					nImageSizeY = sArrInspImageData[nAreaIdx].nImageSizeY;
				}
			}
			nLine = __LINE__;
			if (pColorTeach != NULL && bUseColor == TRUE && sArrInspImageData != NULL && nImageSizeX > 0 && nImageSizeY > 0)
			{
				UCHAR *ucColorImgDst = NULL;
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, nImageSizeX * nImageSizeY);
				memset(ucColorImgDst, 0, sizeof(UCHAR) * nImageSizeX * nImageSizeY);
				bool bTeach = ucArrDstImg != NULL ? true : false;
				if (pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bGroup == TRUE)
					pColorTeach->GetColorBaseBin(&pInspAlgoAlignEdge->sArrAlgoEdge[0].m_sAlgoColorBase, sArrInspImageData[0], ucColorImgDst, bTeach);
				else
					pColorTeach->GetColorBaseBin(&pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_sAlgoColorBase, sArrInspImageData[nAreaIdx], ucColorImgDst, bTeach);
				nLine = __LINE__;
				m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nImageSizeX, nImageSizeY, _T("OrgColor_Image.bmp"));
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrSearchColor, nSearchSizeX * nSearchSizeY);
				memset(ucArrSearchColor, 0, nSearchSizeX * nSearchSizeY * sizeof(UCHAR));
				m_pProcMilAlgo->GetClipImage_LT(ucColorImgDst, nImageSizeX, nImageSizeY, ucArrSearchColor, nArrLeft[nAreaIdx], nArrTop[nAreaIdx], nSearchSizeX, nSearchSizeY);
				m_pProcMilAlgo->SaveWorkImg(ucArrSearchColor, nSearchSizeX, nSearchSizeY, _T("AlignEdge_Color.bmp"));
				Delete_1DArray(&ucColorImgDst);
			}
			nLine = __LINE__;
			double dCX = 0;
			double dCY = 0;
			double dArea_pixel = 0;
			double dArea = 0;
			int nMinBlobArea = 4;
			CRect rcBlob(0, 0, 0, 0);
			bool bIsDelete = false;
			int nCntBlob = 0;
			int nArrSelectDir[EdgeLineTotalCnt];
			memset(bArrIsHorizon[nAreaIdx], 0, EdgeLineTotalCnt * sizeof(BOOL));
			memset(poArrSetTeachCenter[nAreaIdx], 0, EdgeLineTotalCnt * sizeof(POINTF));
			memset(nArrSelectDir, 0, EdgeLineTotalCnt * sizeof(int));
			int nLineTotalCnt = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nSetLineCnt;

			nLine = __LINE__;
			for (int n = 0; n < nLineTotalCnt; n++)
			{
				if (m_pCPInsp_Algo->AnglePointChange(nSearchSizeX, nSearchSizeY, pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_poArrSetTeachCenter[n], &poArrSetTeachCenter[nAreaIdx][n]) == FALSE)
					continue;

				dTeachCx[nAreaIdx] += poArrSetTeachCenter[nAreaIdx][n].x;
				dTeachCy[nAreaIdx] += poArrSetTeachCenter[nAreaIdx][n].y;
				if (dWndAngle == 90 || dWndAngle == 270)
					bArrIsHorizon[nAreaIdx][n] = !pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_bArrIsHorizon[n];
				else
					bArrIsHorizon[nAreaIdx][n] = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_bArrIsHorizon[n];
				bool bAnyAngle = m_pCPInsp_Algo->IsAnyAngle(dWndAngle);
				if (pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n] == 0 || dWndAngle == 0 || bAnyAngle == true)
					nArrSelectDir[n] = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n];
				else
				{
					if (dWndAngle == 90 || dWndAngle == 180)
					{
						if (bArrIsHorizon[nAreaIdx][n] == true || dWndAngle == 180)
						{
							if (pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n] == 1)
								nArrSelectDir[n] = 2;
							else if (pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n] == 2)
								nArrSelectDir[n] = 1;
						}
						else
							nArrSelectDir[n] = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n];
					}
					if (dWndAngle == 270 || dWndAngle == 180)
					{
						if (bArrIsHorizon[nAreaIdx][n] == false || dWndAngle == 180)
						{
							if (pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n] == 1)
								nArrSelectDir[n] = 2;
							else if (pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n] == 2)
								nArrSelectDir[n] = 1;
						}
						else
							nArrSelectDir[n] = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_nArrMeasureDirection[n];
					}
				}
			}
			nLine = __LINE__;
			TotalInspExceptArea stTieAreaNULL;
			stTieAreaNULL.m_nUsedInspPolygon = 0;
			stTieAreaNULL.m_nUsedMaskingValue = 0;
			stTieAreaNULL.m_nUsedWndPolygon = 0;
			nLine = __LINE__;
			int nBlobGroup = nAreaIdx;
			if (nAreaIdx > 0 && pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bGroup == true)
				nBlobGroup = 0;
			AlgoBlob algoBlob = m_pCPInsp_Algo->SetAlgoBlob(sInspAlgo, nBlobGroup);
			nCntBlob = m_pCPInsp_Algo->BlobImageStruct(algoBlob, ucArrSearch, fArrSearch, ucArrSearchColor, nSearchSizeX, nSearchSizeY, nMinBlobArea, &dArea_pixel, &dCX, &dCY, &rcBlob, ucArrSearchDst, stTieAreaNULL, algoBlob.m_bFillHole);
			cv::Mat cvRstBlob(nSearchSizeY, nSearchSizeX, CV_8UC1, ucArrSearchDst);
			dRstCxBlob[nAreaIdx] = dCX;
			dRstCyBlob[nAreaIdx] = dCY;
			dRstCxROI[nAreaIdx] = dCX;
			dRstCyROI[nAreaIdx] = dCY;
			if ((nAreaIdx == 0 && (pInspAlgoAlignEdge->m_nOPT & m_eAE_1_ROI) == m_eAE_1_ROI) ||
				(nAreaIdx == 1 && (pInspAlgoAlignEdge->m_nOPT & m_eAE_2_ROI) == m_eAE_2_ROI) ||
				(nAreaIdx == 2 && (pInspAlgoAlignEdge->m_nOPT & m_eAE_3_ROI) == m_eAE_3_ROI))
			{
				POINTF poArrDstPoint[PADALIGN_AREA_CNTS];
				memset(poArrDstPoint, 0, sizeof(POINTF) * PADALIGN_AREA_CNTS);
				if (m_pCPInsp_Algo->GetROICenter(algoBlob.m_nTypeSelectBlob, cvRstBlob, poArrDstPoint))
				{
					dRstCxROI[nAreaIdx] = 0;
					dRstCyROI[nAreaIdx] = 0;
					for (int n = 0; n < 4; n++)
					{
						if (poArrDstPoint[n].x < 0)	poArrDstPoint[n].x = 0;
						if (poArrDstPoint[n].y < 0)	poArrDstPoint[n].y = 0;
						if (poArrDstPoint[n].x > cvRstBlob.cols)	poArrDstPoint[n].x = cvRstBlob.cols;
						if (poArrDstPoint[n].y > cvRstBlob.rows)	poArrDstPoint[n].y = cvRstBlob.rows;
						dRstCxROI[nAreaIdx] += poArrDstPoint[n].x;
						dRstCyROI[nAreaIdx] += poArrDstPoint[n].y;
					}
					dRstCxROI[nAreaIdx] /= 4;
					dRstCyROI[nAreaIdx] /= 4;
				}
			}

			m_pProcMilAlgo->SaveWorkImg(ucArrSearchDst, nSearchSizeX, nSearchSizeY, _T("AlignEdge_Blob.bmp"));
			nLine = __LINE__;
			if (sRstAlgo)
			{
				sRstAlgo->m_rcRect_I[nAreaIdx].left = nArrLeft[nAreaIdx];
				sRstAlgo->m_rcRect_I[nAreaIdx].top = nArrTop[nAreaIdx];
				sRstAlgo->m_rcRect_I[nAreaIdx].right = sRstAlgo->m_rcRect_I[nAreaIdx].left + nSearchSizeX;
				sRstAlgo->m_rcRect_I[nAreaIdx].bottom = sRstAlgo->m_rcRect_I[nAreaIdx].top + nSearchSizeY;
			}
			nLine = __LINE__;
			if (ucArrDstImg != NULL)
			{
				int nIndex = 0;
				for (int y = nArrTop[nAreaIdx]; y < nArrTop[nAreaIdx] + nSearchSizeY; y++)
				{
					if (y < 0)
						continue;
					else if (y >= nImgHeight)
						break;
					for (int x = nArrLeft[nAreaIdx]; x < nArrLeft[nAreaIdx] + nSearchSizeX; x++)
					{
						if (x < 0)
							continue;
						else if (x >= nImgWidth)
							break;

						int nPos = (y * nImgWidth) + x;
						ucArrDstImg[nPos] = ucArrSearchDst[nIndex];
						nIndex++;
					}
				}
			}
			dArea = dArea_pixel * m_resolX * m_resolY;
			dArrArea[nAreaIdx] = dArea;
			double dMaxArea = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_dAreaCurrent * pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_dAreaMax / 100.;
			double dMinArea = pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_dAreaCurrent * pInspAlgoAlignEdge->sArrAlgoEdge[nAreaIdx].m_dAreaMin / 100.;
			if (!bTeach)
				if (pInspAlgoAlignEdge->m_nAreaCnt > 2 && (dMaxArea < dArea || dMinArea > dArea))
				{
					nCntBlob = 0;
				}
			nLine = __LINE__;
			Delete_1DArray(&ucArrSearch);
			Delete_1DArray(&fArrSearch);
			Delete_1DArray(&ucArrSearchColor);
			nLine = __LINE__;
			if (nCntBlob == 0 || (nLineTotalCnt <= 0 || nLineTotalCnt > EdgeLineTotalCnt))
			{
				nLine = __LINE__;
				Delete_1DArray(&ucArrSearchDst);
				if (nAreaIdx == 0)
					sRstAlgo->m_bMissing_1 = TRUE;
				else if (nAreaIdx == 1)
					sRstAlgo->m_bMissing_2 = TRUE;
				else if (nAreaIdx == 2)
					sRstAlgo->m_bMissing_3 = TRUE;

				continue;
			}
			nLine = __LINE__;
			POINTF ArrDrawLine[EdgeLineTotalCnt][2];
			memset(ArrDrawLine, 0, sizeof(POINTF) * EdgeLineTotalCnt * 2);
			memset(dArrLineDAValue[nAreaIdx], 0, sizeof(double) * EdgeLineTotalCnt);
			memset(dArrLineDBValue[nAreaIdx], 0, sizeof(double) * EdgeLineTotalCnt);

#pragma region circleopt
			if (pInspAlgoAlignEdge->m_bCicleOpt && pInspAlgoAlignEdge->m_nAreaCnt > 2 && nAreaIdx < 2)
			{
				// 			sLog.Format(_T("Func_InspAlignEdge, in cirlce Pass!!! Idx: %d"),nAreaIdx);
				// 			g_pMPTI->AddLog_Dev(sLog);
				nLine = __LINE__;
				cv::Mat testImage(nSearchSizeY, nSearchSizeX, CV_8UC1, ucArrSearchDst);
				cv::Mat Ecircle = testImage.clone();

				std::vector<std::vector<cv::Point>> contours;
				std::vector<cv::Vec4i> hierarchy;

				// 			sLog.Format(_T("Func_InspAlignEdge, findContours Idx: %d"),nAreaIdx);
				// 			g_pMPTI->AddLog_Dev(sLog);
				cv::Mat cannybuf;
				cv::Canny(Ecircle, cannybuf, 100, 255);
				cv::findContours(cannybuf, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
				nLine = __LINE__;
				if (contours.size() > 0)
				{
					std::vector<cv::Point> vPTmp;// = contours[0];
					cv::Mat Points;
					cv::Mat conTemp;// = cv::Mat(vPTmp).clone();
					cv::Point pCenterPoint(0, 0);
					cv::Size2f cicleSize(0, 0);

					std::vector<cv::Point> selectedCon;
					for (int cCnt = 0; cCnt < contours.size(); cCnt++)
					{
						if (contours[cCnt].size() < 6)
							continue;

						if (contours[cCnt].size() < selectedCon.size())
							continue;
						selectedCon = contours[cCnt];
					}

					conTemp = cv::Mat(selectedCon);
					conTemp.convertTo(Points, CV_32F);
					nLine = __LINE__;
					try
					{
						if (selectedCon.size() > 6)
							FiduRect[nAreaIdx] = cv::fitEllipse(cv::Mat(Points));
					}
					catch (CMemoryException* e)
					{
						sLog.Format(_T("Func_InspAlignEdge, fitEllipse_CMemoryException_Points:%d_Points:%d"), Points.cols, Points.rows);
						g_pMPTI->AddLog_Dev(sLog);

					}
					catch (CFileException* e)
					{
						sLog.Format(_T("Func_InspAlignEdge, fitEllipse_CFileException_Points:%d_Points:%d"), Points.cols, Points.rows);
						g_pMPTI->AddLog_Dev(sLog);
					}
					catch (CException* e)
					{
						sLog.Format(_T("Func_InspAlignEdge, fitEllipse_CException_Points:%d_Points:%d"), Points.cols, Points.rows);
						g_pMPTI->AddLog_Dev(sLog);
					}
					nLine = __LINE__;
					pCenterPoint.x = FiduRect[nAreaIdx].center.x;
					pCenterPoint.y = FiduRect[nAreaIdx].center.y;
					cicleSize.width = FiduRect[nAreaIdx].size.width;
					cicleSize.height = FiduRect[nAreaIdx].size.height;
#if _DEBUG
					CString msg;
					msg.Format(_T("D:\\AlignEdge%d.bmp"), nAreaIdx);
					cv::imwrite(std::string(CT2A(msg)), testImage);

					cv::Mat Drawcircle = testImage.clone();
					cv::ellipse(Drawcircle, FiduRect[nAreaIdx], cv::Scalar(126));
					msg.Format(_T("D:\\AlignEdge%d_EllipseCircle.bmp"), nAreaIdx);
					cv::imwrite(std::string(CT2A(msg)), Drawcircle);

#endif
					nLine = __LINE__;
					if (nAreaIdx == 0)
					{
						sRstAlgo->m_poDrawLine[0].x = pCenterPoint.x - (cicleSize.width / 2);
						sRstAlgo->m_poDrawLine[0].y = pCenterPoint.y;
						sRstAlgo->m_poDrawLine[1].x = pCenterPoint.x;
						sRstAlgo->m_poDrawLine[1].y = pCenterPoint.y - (cicleSize.height / 2);
						sRstAlgo->m_poDrawLine_Sec[0].x = pCenterPoint.x + (cicleSize.width / 2);
						sRstAlgo->m_poDrawLine_Sec[0].y = pCenterPoint.y;
						sRstAlgo->m_poDrawLine_Sec[1].x = pCenterPoint.x;
						sRstAlgo->m_poDrawLine_Sec[1].y = pCenterPoint.y + (cicleSize.height / 2);
					}
					else if (nAreaIdx == 1)
					{
						sRstAlgo->m_poDrawLine_1[0].x = pCenterPoint.x - (cicleSize.width / 2);
						sRstAlgo->m_poDrawLine_1[0].y = pCenterPoint.y;
						sRstAlgo->m_poDrawLine_1[1].x = pCenterPoint.x;
						sRstAlgo->m_poDrawLine_1[1].y = pCenterPoint.y - (cicleSize.height / 2);
						sRstAlgo->m_poDrawLine_Sec_1[0].x = pCenterPoint.x + (cicleSize.width / 2);
						sRstAlgo->m_poDrawLine_Sec_1[0].y = pCenterPoint.y;
						sRstAlgo->m_poDrawLine_Sec_1[1].x = pCenterPoint.x;
						sRstAlgo->m_poDrawLine_Sec_1[1].y = pCenterPoint.y + (cicleSize.height / 2);
					}
					else if (nAreaIdx == 2)
					{
						sRstAlgo->m_poDrawLine_2[0].x = pCenterPoint.x - (cicleSize.width / 2);
						sRstAlgo->m_poDrawLine_2[0].y = pCenterPoint.y;
						sRstAlgo->m_poDrawLine_2[1].x = pCenterPoint.x;
						sRstAlgo->m_poDrawLine_2[1].y = pCenterPoint.y - (cicleSize.height / 2);
						sRstAlgo->m_poDrawLine_Sec_2[0].x = pCenterPoint.x + (cicleSize.width / 2);
						sRstAlgo->m_poDrawLine_Sec_2[0].y = pCenterPoint.y;
						sRstAlgo->m_poDrawLine_Sec_2[1].x = pCenterPoint.x;
						sRstAlgo->m_poDrawLine_Sec_2[1].y = pCenterPoint.y + (cicleSize.height / 2);
					}
					nLine = __LINE__;
					dRstCx[nAreaIdx] = (pCenterPoint.x*nLineTotalCnt);//*m_resolX*1000;
					dRstCy[nAreaIdx] = (pCenterPoint.y*nLineTotalCnt);//*m_resolY*1000;

					Delete_1DArray(&ucArrSearchDst);
					continue;
				}
			}
#pragma endregion circleopt
			nLine = __LINE__;

			int nInspOption = pInspAlgoAlignEdge->sArrAlgoEdge[nBlobGroup].m_nInspOption;
			nInspOption |= m_eEdgeData_UseExceptAngle;

			for (int n = 0; n < nLineTotalCnt; n++)
			{
				int nMeasureDirection = nArrSelectDir[n];
				bool bIsHorizon = bArrIsHorizon[nAreaIdx][n];
				double dAngle = 0;
				int nLineLength = 0;
				nLine = __LINE__;
				bool bReturn = m_pCPInsp_Algo->InspectionLine(nSearchSizeX, nSearchSizeY, ucArrSearchDst,
					bIsHorizon, nMeasureDirection, &dAngle, ArrDrawLine[n], &nLineLength,
					&dArrLineDAValue[nAreaIdx][n], &dArrLineDBValue[nAreaIdx][n],
					pInspAlgoAlignEdge->sArrAlgoEdge[nBlobGroup].m_nLineFindType, pInspAlgoAlignEdge->sArrAlgoEdge[nBlobGroup].m_dLineFindRate, nInspOption);
				for (int j = 0; j < 2; j++)
				{
					ArrDrawLine[n][j].x += sWndAlgoImg.m_fPartRoundingErrX;
					ArrDrawLine[n][j].y += sWndAlgoImg.m_fPartRoundingErrY;
				}
				if (pInspAlgoAlignEdge->m_bUseAnchor || pInspAlgoAlignEdge->m_bUseTwoAnchor)
				{

#if _DEBUG
					cv::Mat cvRstSearch(nSearchSizeY, nSearchSizeX, CV_8UC1, ucArrSearchDst);
					cv::Mat colorImg;
					cv::cvtColor(cvRstSearch, colorImg, cv::COLOR_GRAY2RGB);
#endif				
					cv::Point2f DrawLinePo1(ArrDrawLine[n][0].x, ArrDrawLine[n][0].y);
					cv::Point2f DrawLinePo2(ArrDrawLine[n][1].x, ArrDrawLine[n][1].y);

#if _DEBUG
					cv::line(colorImg, DrawLinePo1, DrawLinePo2, cv::Scalar(0, 0, 255), 15);

					cv::imwrite(std::string("D:\\DstImgGray.bmp"), colorImg);
					cv::imwrite(std::string("D:\\DstImgGray_org.bmp"), cvRstSearch);
#endif

					DrawLinePo1.x = (float)DrawLinePo1.x + (float)nArrLeft[nAreaIdx];
					DrawLinePo1.y = (float)DrawLinePo1.y + (float)nArrTop[nAreaIdx];

					DrawLinePo2.x = (float)DrawLinePo2.x + (float)nArrLeft[nAreaIdx];
					DrawLinePo2.y = (float)DrawLinePo2.y + (float)nArrTop[nAreaIdx];

					vecEdgeLineStartPo.push_back(DrawLinePo1);
					vecEdgeLineEndPo.push_back(DrawLinePo2);
					vecbIsHorizonLine.push_back(bIsHorizon);

				}

				nLine = __LINE__;
				if (sRstAlgo)
				{
					if (nAreaIdx == 0)
					{
						sRstAlgo->m_poDrawLine[n] = ArrDrawLine[n][0];
						sRstAlgo->m_poDrawLine_Sec[n] = ArrDrawLine[n][1];

						sRstAlgo->m_poDrawLine_T[n].x = poArrSetTeachCenter[nAreaIdx][n].x;
						sRstAlgo->m_poDrawLine_T[n].y = 0;
						sRstAlgo->m_poDrawLine_T2[n].x = poArrSetTeachCenter[nAreaIdx][n].x;
						sRstAlgo->m_poDrawLine_T2[n].y = nSearchSizeY;
						if (bIsHorizon == true)
						{
							sRstAlgo->m_poDrawLine_T[n].x = 0;
							sRstAlgo->m_poDrawLine_T[n].y = poArrSetTeachCenter[nAreaIdx][n].y;
							sRstAlgo->m_poDrawLine_T2[n].x = nSearchSizeX;
							sRstAlgo->m_poDrawLine_T2[n].y = poArrSetTeachCenter[nAreaIdx][n].y;
						}
					}
					else if (nAreaIdx == 1)
					{
						sRstAlgo->m_poDrawLine_1[n] = ArrDrawLine[n][0];
						sRstAlgo->m_poDrawLine_Sec_1[n] = ArrDrawLine[n][1];
						sRstAlgo->m_poDrawLine_T21[n].x = poArrSetTeachCenter[nAreaIdx][n].x;
						sRstAlgo->m_poDrawLine_T21[n].y = 0;
						sRstAlgo->m_poDrawLine_T22[n].x = poArrSetTeachCenter[nAreaIdx][n].x;
						sRstAlgo->m_poDrawLine_T22[n].y = nSearchSizeY;
						if (bIsHorizon == true)
						{
							sRstAlgo->m_poDrawLine_T21[n].x = 0;
							sRstAlgo->m_poDrawLine_T21[n].y = poArrSetTeachCenter[nAreaIdx][n].y;
							sRstAlgo->m_poDrawLine_T22[n].x = nSearchSizeX;
							sRstAlgo->m_poDrawLine_T22[n].y = poArrSetTeachCenter[nAreaIdx][n].y;
						}
					}
					else if (nAreaIdx == 2)
					{
						sRstAlgo->m_poDrawLine_2[n] = ArrDrawLine[n][0];
						sRstAlgo->m_poDrawLine_Sec_2[n] = ArrDrawLine[n][1];
						sRstAlgo->m_poDrawLine_T31[n].x = poArrSetTeachCenter[nAreaIdx][n].x;
						sRstAlgo->m_poDrawLine_T31[n].y = 0;
						sRstAlgo->m_poDrawLine_T32[n].x = poArrSetTeachCenter[nAreaIdx][n].x;
						sRstAlgo->m_poDrawLine_T32[n].y = nSearchSizeY;
						if (bIsHorizon == true)
						{
							sRstAlgo->m_poDrawLine_T31[n].x = 0;
							sRstAlgo->m_poDrawLine_T31[n].y = poArrSetTeachCenter[nAreaIdx][n].y;
							sRstAlgo->m_poDrawLine_T32[n].x = nSearchSizeX;
							sRstAlgo->m_poDrawLine_T32[n].y = poArrSetTeachCenter[nAreaIdx][n].y;
						}
					}
				}
				dRstCx[nAreaIdx] += (ArrDrawLine[n][0].x + ArrDrawLine[n][1].x) / 2.0f;
				dRstCy[nAreaIdx] += (ArrDrawLine[n][0].y + ArrDrawLine[n][1].y) / 2.0f;
			}
			Delete_1DArray(&ucArrSearchDst);
		}
		nLine = __LINE__;


		std::vector<POINTF> vecRstcrossLinePo;
		if (pInspAlgoAlignEdge->m_bUseAnchor || pInspAlgoAlignEdge->m_bUseTwoAnchor)
		{
			//SCC 부품 외곽의 무게중심 검사 관련 NYJ 21/04/12
			POINTF crossLinePo;

			POINTF poLine1St;
			POINTF poLine1End;

			POINTF poLine2St;
			POINTF poLine2End;

			if (vecEdgeLineStartPo.size() == 4 && vecEdgeLineEndPo.size() == 4 && vecbIsHorizonLine.size() == 4)
			{
				for (int j = 0; j < pInspAlgoAlignEdge->m_nAreaCnt; j++)
				{
					crossLinePo.x = 0;
					crossLinePo.y = 0;

					poLine1St.x = vecEdgeLineStartPo[j].x;
					poLine1St.y = vecEdgeLineStartPo[j].y;
					poLine1End.x = vecEdgeLineEndPo[j].x;
					poLine1End.y = vecEdgeLineEndPo[j].y;

					bool bIsLine1Horizon = vecbIsHorizonLine[j];

					POINTF cvCrossPo;

					for (int i = j; i < (pInspAlgoAlignEdge->sArrAlgoEdge[j].m_nSetLineCnt *pInspAlgoAlignEdge->m_nAreaCnt) - 1; i++)
					{
						poLine2St.x = vecEdgeLineStartPo[i + 1].x;
						poLine2St.y = vecEdgeLineStartPo[i + 1].y;

						poLine2End.x = vecEdgeLineEndPo[i + 1].x;
						poLine2End.y = vecEdgeLineEndPo[i + 1].y;

						bool bIsLine2Horizon = vecbIsHorizonLine[i + 1];

						bool bGetIntersectPo = GetIntersectPoint2D(poLine1St, poLine1End,
							poLine2St, poLine2End, &crossLinePo);

						bool bFindCrossPo = false;
						if (bGetIntersectPo)
						{
							if (nImgHeight > crossLinePo.y && nImgWidth > crossLinePo.x && crossLinePo.y > 0 && crossLinePo.x > 0)
							{
								cvCrossPo.x = crossLinePo.x;
								cvCrossPo.y = crossLinePo.y;

								bFindCrossPo = true;

							}
						}


						if (bIsLine1Horizon && bIsLine2Horizon || (!bIsLine1Horizon && !bIsLine2Horizon))	//둘다 수평이나 수직일 경우 처리하지 않음
						{
							continue;
						}
						else
						{		//수평, 수직일 경우 교차점 존재
							if (bIsLine1Horizon && !bIsLine2Horizon)
							{
								if (bFindCrossPo || !bGetIntersectPo || (bGetIntersectPo && !bFindCrossPo))
								{
									cvCrossPo.x = poLine2St.x;
									cvCrossPo.y = poLine1St.y;

									vecRstcrossLinePo.push_back(cvCrossPo);
									//cv::line(cvPartOrgImage, cvCrossPo, cvCrossPo, cv::Scalar(0, 0, 255), 20);
								}
							}
							else if (!bIsLine1Horizon && bIsLine2Horizon)
							{
								if (bFindCrossPo || !bGetIntersectPo || (bGetIntersectPo && !bFindCrossPo))
								{
									cvCrossPo.x = poLine1St.x;
									cvCrossPo.y = poLine2St.y;

									vecRstcrossLinePo.push_back(cvCrossPo);
									//cv::line(cvPartOrgImage, cvCrossPo, cvCrossPo, cv::Scalar(0, 0, 255), 20);
								}
							}

						}

					}
				}
			}
		}

		if (sRstAlgo)
		{
			double dLineTotalCnt = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_nSetLineCnt;
			double dRstCX_1 = dRstCx[0] / (double)pInspAlgoAlignEdge->sArrAlgoEdge[0].m_nSetLineCnt;
			double dRstCY_1 = dRstCy[0] / (double)pInspAlgoAlignEdge->sArrAlgoEdge[0].m_nSetLineCnt;
			double dTeachCX_1 = dTeachCx[0] / (double)pInspAlgoAlignEdge->sArrAlgoEdge[0].m_nSetLineCnt;
			double dTeachCY_1 = dTeachCy[0] / (double)pInspAlgoAlignEdge->sArrAlgoEdge[0].m_nSetLineCnt;
			nLine = __LINE__;
			if (dLineTotalCnt == 2)
			{
				if (bArrIsHorizon[0][0] != bArrIsHorizon[0][1])
				{
					if (!(pInspAlgoAlignEdge->m_bCicleOpt && pInspAlgoAlignEdge->m_nAreaCnt > 1))
					{
						nLine = __LINE__;
						POINTF poCrossLinePoint = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrLineDAValue[0], dArrLineDBValue[0], bArrIsHorizon[0]);
						dRstCX_1 = poCrossLinePoint.x;
						dRstCY_1 = poCrossLinePoint.y;
						sRstAlgo->m_poRstCenter[0].x = dRstCX_1;
						sRstAlgo->m_poRstCenter[0].y = dRstCY_1;
					}
					if (bArrIsHorizon[0][0] == true)
					{
						dTeachCX_1 = poArrSetTeachCenter[0][1].x;
						dTeachCY_1 = poArrSetTeachCenter[0][0].y;
					}
					else
					{
						dTeachCX_1 = poArrSetTeachCenter[0][0].x;
						dTeachCY_1 = poArrSetTeachCenter[0][1].y;
					}
				}
			}
			if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_1_CG) == m_eAE_1_CG)
			{
				dRstCX_1 = dRstCxBlob[0];
				dRstCY_1 = dRstCyBlob[0];
			}
			else if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_1_ROI) == m_eAE_1_ROI)
			{
				dRstCX_1 = dRstCxROI[0];
				dRstCY_1 = dRstCyROI[0];
			}
			sRstAlgo->m_poRstCenter[0].x = dRstCX_1;
			sRstAlgo->m_poRstCenter[0].y = dRstCY_1;
			sRstAlgo->m_pHPoint[0].x = dRstCX_1 + nArrLeft[0];
			sRstAlgo->m_pHPoint[0].y = dRstCY_1 + nArrTop[0];
			nLine = __LINE__;
			dLineTotalCnt = pInspAlgoAlignEdge->sArrAlgoEdge[1].m_nSetLineCnt;
			double dRstCX_2 = dRstCx[1] / dLineTotalCnt;
			double dRstCY_2 = dRstCy[1] / dLineTotalCnt;
			double dTeachCX_2 = dTeachCx[1] / dLineTotalCnt;
			double dTeachCY_2 = dTeachCy[1] / dLineTotalCnt;
			if (dLineTotalCnt == 2)
			{
				if (bArrIsHorizon[1][0] != bArrIsHorizon[1][1])
				{
					if (!(pInspAlgoAlignEdge->m_bCicleOpt && pInspAlgoAlignEdge->m_nAreaCnt > 1))
					{
						nLine = __LINE__;
						POINTF poCrossLinePoint = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrLineDAValue[1], dArrLineDBValue[1], bArrIsHorizon[1]);
						dRstCX_2 = poCrossLinePoint.x;
						dRstCY_2 = poCrossLinePoint.y;
					}
					if (bArrIsHorizon[1][0] == true)
					{
						dTeachCX_2 = poArrSetTeachCenter[1][1].x;
						dTeachCY_2 = poArrSetTeachCenter[1][0].y;
					}
					else
					{
						dTeachCX_2 = poArrSetTeachCenter[1][0].x;
						dTeachCY_2 = poArrSetTeachCenter[1][1].y;
					}
				}
			}
			if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_2_CG) == m_eAE_2_CG)
			{
				dRstCX_2 = dRstCxBlob[1];
				dRstCY_2 = dRstCyBlob[1];
			}
			else if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_2_ROI) == m_eAE_2_ROI)
			{
				dRstCX_2 = dRstCxROI[1];
				dRstCY_2 = dRstCyROI[1];
			}
			sRstAlgo->m_poRstCenter[1].x = dRstCX_2;
			sRstAlgo->m_poRstCenter[1].y = dRstCY_2;
			sRstAlgo->m_pHPoint[1].x = dRstCX_2 + nArrLeft[1];
			sRstAlgo->m_pHPoint[1].y = dRstCY_2 + nArrTop[1];
			nLine = __LINE__;
			// 3
			dLineTotalCnt = pInspAlgoAlignEdge->sArrAlgoEdge[2].m_nSetLineCnt;
			double dRstCX_3 = dRstCx[2] / dLineTotalCnt;
			double dRstCY_3 = dRstCy[2] / dLineTotalCnt;
			double dTeachCX_3 = dTeachCx[2] / dLineTotalCnt;
			double dTeachCY_3 = dTeachCy[2] / dLineTotalCnt;
			if (dLineTotalCnt == 2)
			{
				if (bArrIsHorizon[2][0] != bArrIsHorizon[2][1])
				{
					nLine = __LINE__;
					POINTF poCrossLinePoint = m_pCPInsp_Algo->GetIntersectionOfTwoStraightLines(dArrLineDAValue[2], dArrLineDBValue[2], bArrIsHorizon[2]);
					dRstCX_3 = poCrossLinePoint.x;
					dRstCY_3 = poCrossLinePoint.y;
					if (bArrIsHorizon[2][0] == true)
					{
						dTeachCX_3 = poArrSetTeachCenter[2][1].x;
						dTeachCY_3 = poArrSetTeachCenter[2][0].y;
					}
					else
					{
						dTeachCX_3 = poArrSetTeachCenter[2][0].x;
						dTeachCY_3 = poArrSetTeachCenter[2][1].y;
					}
				}
			}
			if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_3_CG) == m_eAE_3_CG)
			{
				dRstCX_3 = dRstCxBlob[2];
				dRstCY_3 = dRstCyBlob[2];
			}
			else if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_3_ROI) == m_eAE_3_ROI)
			{
				dRstCX_3 = dRstCxROI[2];
				dRstCY_3 = dRstCyROI[2];
			}
			sRstAlgo->m_poRstCenter[2].x = dRstCX_3;
			sRstAlgo->m_poRstCenter[2].y = dRstCY_3;
			sRstAlgo->m_pHPoint[2].x = dRstCX_3 + nArrLeft[2];
			sRstAlgo->m_pHPoint[2].y = dRstCY_3 + nArrTop[2];
			nLine = __LINE__;
			if (sRstAlgo->m_bMissing_1 == TRUE || sRstAlgo->m_bMissing_2 == TRUE || sRstAlgo->m_bMissing_3 == TRUE)
			{

				// 			sLog.Format(_T("Func_InspAlignEdge, return m_bMissing_1: %d, m_bMissing_2: %d"),sRstAlgo->m_bMissing_1,sRstAlgo->m_bMissing_2);
				// 			g_pMPTI->AddLog_Dev(sLog);
				sRstAlgo->m_bOKShiftX = FALSE;
				sRstAlgo->m_bOKShiftY = FALSE;
				sRstAlgo->m_bOKAngle = FALSE;
				sRstAlgo->m_bOKDistance = FALSE;
				sRstAlgo->m_bOKDistanceX = FALSE;
				sRstAlgo->m_bOKDistanceY = FALSE;
				sRstAlgo->m_dRstShiftX = 0;
				sRstAlgo->m_dRstShiftY = 0;
				sRstAlgo->m_dTheta = 0;
				sRstAlgo->m_dDistance = 0;
				sRstAlgo->m_d3PtDis1 = 88888;
				sRstAlgo->m_d3PtDis3 = 88888;
				bResult = FALSE;
				sRstAlgo->m_dArea[0] = dArrArea[0];
				sRstAlgo->m_dArea[1] = dArrArea[1];
				sRstAlgo->m_dArea[2] = dArrArea[2];
				if (sRstAlgo->m_bMissing_1 == TRUE && ((pInspAlgoAlignEdge->m_nOPT & m_eAE_MissingOK) == m_eAE_MissingOK))
				{
					sRstAlgo->m_dDistance = 0.01;
					sRstAlgo->m_dDistanceX = 0.01;
					sRstAlgo->m_dDistanceY = 0.01;
					bResult = true;
				}
				return bResult;
			}
			nLine = __LINE__;
			sRstAlgo->m_bOKShiftX = TRUE;
			sRstAlgo->m_bOKShiftY = TRUE;
			sRstAlgo->m_bOKAngle = TRUE;
			sRstAlgo->m_bOKDistance = TRUE;
			sRstAlgo->m_bOKDistanceX = TRUE;
			sRstAlgo->m_bOKDistanceY = TRUE;
			sRstAlgo->m_bMissing_1 = FALSE;
			sRstAlgo->m_bMissing_2 = FALSE;
			bool bUseShift = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bShiftIsUse;
			sRstAlgo->m_poDrawCenter.x = (nArrLeft[0] + dRstCX_1 + nArrLeft[1] + dRstCX_2) / 2.0;
			sRstAlgo->m_poDrawCenter.y = (nArrTop[0] + dRstCY_1 + nArrTop[1] + dRstCY_2) / 2.0;

			if (pInspAlgoAlignEdge->m_bUseAnchor || pInspAlgoAlignEdge->m_bUseTwoAnchor)
			{
				if (vecRstcrossLinePo.size() == 4)
				{
					float fCenterX = 0.0f;
					float fCenterY = 0.0f;

					//대각선 교차점으로 중점 계산
					POINTF poLine1, poLine2, poLine3, poLine4;
					poLine1.x = vecRstcrossLinePo[0].x;  poLine1.y = vecRstcrossLinePo[0].y;
					poLine2.x = vecRstcrossLinePo[1].x;  poLine2.y = vecRstcrossLinePo[1].y;
					poLine3.x = vecRstcrossLinePo[2].x;  poLine3.y = vecRstcrossLinePo[2].y;
					poLine4.x = vecRstcrossLinePo[3].x;  poLine4.y = vecRstcrossLinePo[3].y;

					POINTF pRstcenterPo;
					bool bGetIntersectPoCenter = GetIntersectPoint2D(poLine1, poLine4,
						poLine2, poLine3, &pRstcenterPo);

					if (bGetIntersectPoCenter)
					{
						fCenterX = pRstcenterPo.x;
						fCenterY = pRstcenterPo.y;
					}
					else
					{
						fCenterX = (vecRstcrossLinePo[0].x + vecRstcrossLinePo[1].x + vecRstcrossLinePo[2].x + vecRstcrossLinePo[3].x) / 4.0f;
						fCenterY = (vecRstcrossLinePo[0].y + vecRstcrossLinePo[1].y + vecRstcrossLinePo[2].y + vecRstcrossLinePo[3].y) / 4.0f;
					}

					if (fCenterX > 0 && fCenterX < nImgWidth && fCenterY > 0 && fCenterY < nImgHeight)
					{
						sRstAlgo->m_poDrawCenter.x = fCenterX;
						sRstAlgo->m_poDrawCenter.y = fCenterY;
					}
					else
					{
						sRstAlgo->m_poDrawCenter.x = (nArrLeft[0] + dRstCX_1 + nArrLeft[1] + dRstCX_2) / 2.0;
						sRstAlgo->m_poDrawCenter.y = (nArrTop[0] + dRstCY_1 + nArrTop[1] + dRstCY_2) / 2.0;
					}
				}
			}

			nLine = __LINE__;
			if (bUseShift == true)
			{
				bool bUseShiftX = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bShiftXUse;
				bool bUseShiftY = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bShiftYUse;
				double dTechCenterX = (dTeachCX_1 + dTeachCX_2) / 2.0;
				double dTechCenterY = (dTeachCY_1 + dTeachCY_2) / 2.0;
				double dInspCenterX = (dRstCX_1 + dRstCX_2) / 2.0;
				double dInspCenterY = (dRstCY_1 + dRstCY_2) / 2.0;
				double dShiftX = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dShiftX;
				double dShiftY = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dShiftY;
				if (dWndAngle == 90 || dWndAngle == 270)
				{
					dShiftX = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dShiftY;
					dShiftY = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dShiftX;
				}
				sRstAlgo->m_dCenterX_T = ((nArrLeft[0] + dTeachCX_1) + (nArrLeft[1] + dTeachCX_2)) / 2.0 * m_resolX;
				sRstAlgo->m_dCenterY_T = ((nArrTop[0] + dTeachCY_1) + (nArrTop[1] + dTeachCY_2)) / 2.0 * m_resolY;
				sRstAlgo->m_dCenterX_R = ((nArrLeft[0] + dRstCX_1) + (nArrLeft[1] + dRstCX_2)) / 2.0 * m_resolX;
				sRstAlgo->m_dCenterY_R = ((nArrTop[0] + dRstCY_1) + (nArrTop[1] + dRstCY_2)) / 2.0 * m_resolY;
				sRstAlgo->m_dRstShiftX = (dTechCenterX - dInspCenterX) * m_resolX;
				sRstAlgo->m_dRstShiftY = (dTechCenterY - dInspCenterY) * m_resolY;
				if (bUseShiftX == TRUE)
				{
					double dRstShiftX = sRstAlgo->m_dRstShiftX;
					if (dRstShiftX < 0)
						dRstShiftX = sRstAlgo->m_dRstShiftX * -1;
					if (dRstShiftX > dShiftX)
					{
						bResult = FALSE;
						sRstAlgo->m_bOKShiftX = bResult;
					}
				}
				else
					sRstAlgo->m_dRstShiftX = 0;

				if (bUseShiftY == TRUE)
				{
					double dRstShiftY = sRstAlgo->m_dRstShiftY;
					if (dRstShiftY < 0)
						dRstShiftY = sRstAlgo->m_dRstShiftY * -1;
					if (dRstShiftY > dShiftY)
					{
						bResult = FALSE;
						sRstAlgo->m_bOKShiftY = bResult;
					}
				}
				else
					sRstAlgo->m_dRstShiftY = 0;
			}
			nLine = __LINE__;
			bool bUseAngle = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bUseAngle;
			if (bUseAngle == true)
			{
				BOOL bChangeRotateCenter = g_pInspMng->m_bChangeRotateCenter;
				double dAngle = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachRotate;
				if (nSelectArea != -1) dAngle = 0;
				double dRstX = (dRstCX_2 + nArrLeft[1]) - (dRstCX_1 + nArrLeft[0]);
				double dRstY = ((dRstCY_2 + nArrTop[1]) - (dRstCY_1 + nArrTop[0])) * -1;
				double dA = 0, dB = 0;
				double ptrdLineX[2] = { dRstCX_1 + nArrLeft[0], dRstCX_2 + nArrLeft[1] };
				double ptrdLineY[2] = { dRstCY_1 + nArrTop[0], dRstCY_2 + nArrTop[1] };
				double dA_T = 0, dB_T = 0;
				double ptrdLineX_T[2] = { dTeachCX_1 + nArrLeft[0], dTeachCX_2 + nArrLeft[1] };
				double ptrdLineY_T[2] = { dTeachCY_1 + nArrTop[0], dTeachCY_2 + nArrTop[1] };
				bool bHorizon = true;
				int nX = nArrLeft[0] - nArrLeft[1];
				int nY = nArrTop[0] - nArrTop[1];
				if (nX < 0) nX = nArrLeft[1] - nArrLeft[0];
				if (nY < 0) nY = nArrTop[1] - nArrTop[0];
				if (nY > nX) bHorizon = false;
				double dAngleRst = m_pCPInsp_Algo->GetGradient(ptrdLineX, ptrdLineY, 2, dA, dB, bHorizon);
				double dAngleTeach = m_pCPInsp_Algo->GetGradient(ptrdLineX_T, ptrdLineY_T, 2, dA_T, dB_T, bHorizon);
				if (dWndAngle == 90 || dWndAngle == 270)
				{
					dAngle *= -1.0;
					dAngleTeach *= -1.0;
				}
				if (bChangeRotateCenter == TRUE)
				{
					sRstAlgo->m_dTheta = dAngleRst - dAngleTeach;
					if ((bHorizon == false && dWndAngle != 90 && dWndAngle != 270 && bTeach == FALSE) == FALSE)
						sRstAlgo->m_dTheta *= -1.0;
				}
				else
				{
					sRstAlgo->m_dTheta = dAngleRst - dAngle;
					if (dWndAngle != 90 && dWndAngle != 270 && bTeach == FALSE)
						sRstAlgo->m_dTheta *= -1.0;
				}

				if (pInspAlgoAlignEdge->sArrAlgoEdge[0].UseData(m_eEdgeData_AngleBetweenLines))
				{
					if (pInspAlgoAlignEdge->m_nAreaCnt == 2)
					{
						if (pInspAlgoAlignEdge->sArrAlgoEdge[0].m_nSetLineCnt == 1 &&
							pInspAlgoAlignEdge->sArrAlgoEdge[1].m_nSetLineCnt == 1)
						{
							POINTF poA1;
							POINTF poA2;
							POINTF poB1;
							POINTF poB2;
							poA1.x = sRstAlgo->m_poDrawLine[0].x + nArrLeft[0];
							poA1.y = sRstAlgo->m_poDrawLine[0].y + nArrTop[0];
							poA2.x = sRstAlgo->m_poDrawLine_Sec[0].x + nArrLeft[0];
							poA2.y = sRstAlgo->m_poDrawLine_Sec[0].y + nArrTop[0];

							poB1.x = sRstAlgo->m_poDrawLine_1[0].x + nArrLeft[1];
							poB1.y = sRstAlgo->m_poDrawLine_1[0].y + nArrTop[1];
							poB2.x = sRstAlgo->m_poDrawLine_Sec_1[0].x + nArrLeft[1];
							poB2.y = sRstAlgo->m_poDrawLine_Sec_1[0].y + nArrTop[1];
							POINTF poC = m_pCPInsp_Algo->CPInsp::GetIntersection(poA1, poA2, poB1, poB2);
							if (poC.x >= 0 && poC.y >= 0)
								sRstAlgo->m_dTheta = m_pCPInsp_Algo->CPInsp::GetAngleBetweenLines(poA1, poB1, poC);
						}
					}
				}

				double dRstAngle = sRstAlgo->m_dTheta;
				if (dRstAngle < 0) dRstAngle *= -1;
				double dAngleCondition = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachLengthRateMin;
				if (dRstAngle > dAngleCondition)
				{
					bResult = FALSE;
					sRstAlgo->m_bOKAngle = bResult;
				}
			}
			else
				sRstAlgo->m_dTheta = 0;
			nLine = __LINE__;
			bool bUseDistance = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bTeachWidthUse | pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bDistanceX | pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bDistanceY;
			if (bUseDistance == true)
			{
				double dSizeX = ((dRstCX_1 + nArrLeft[0]) * m_resolX) - ((dRstCX_2 + nArrLeft[1]) * m_resolX);
				double dSizeY = ((dRstCY_1 + nArrTop[0]) * m_resolY) - ((dRstCY_2 + nArrTop[1]) * m_resolY);
				sRstAlgo->m_dDistance = sqrt(pow(dSizeX, 2) + pow(dSizeY, 2)) + pInspAlgoAlignEdge->m_dDistOffset;
				if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistXReal) == m_eAE_DistXReal)
					sRstAlgo->m_dDistanceX = dSizeX + pInspAlgoAlignEdge->m_dDistXOffset;
				else
					sRstAlgo->m_dDistanceX = abs(dSizeX) + pInspAlgoAlignEdge->m_dDistXOffset;
				if ((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistYReal) == m_eAE_DistYReal)
					sRstAlgo->m_dDistanceY = dSizeY + pInspAlgoAlignEdge->m_dDistYOffset;
				else
					sRstAlgo->m_dDistanceY = abs(dSizeY) + pInspAlgoAlignEdge->m_dDistYOffset;
				double dDistance = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachWidth;
				double dDistanceRateMin = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachWidthRateMin >= 50 ? dDistance * 0.8 : pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachWidthRateMin;
				double dDistanceRateMax = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachWidthRateMax >= 100 ? dDistance * 1.2 : pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachWidthRateMax;
				double dDistanceMin = dDistanceRateMin;
				double dDistanceMax = dDistanceRateMax;

				BOOL bDisX = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bDistanceX;
				double dDistanceX = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceX;
				double dDistanceXMin = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceXRateMin;
				double dDistanceXMax = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceXRateMax;
				BOOL bDisY = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bDistanceY;
				double dDistanceY = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceY;
				double dDistanceYMin = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceYRateMin;
				double dDistanceYMax = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceYRateMax;

				if (dWndAngle == 90 || dWndAngle == 270)
				{
					bDisX = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bDistanceY;
					dDistanceX = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceY;
					dDistanceXMin = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceYRateMin;
					dDistanceXMax = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceYRateMax;

					bDisY = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bDistanceX;
					dDistanceY = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceX;
					dDistanceYMin = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceXRateMin;
					dDistanceYMax = pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeachDistanceXRateMax;
				}

				if (pInspAlgoAlignEdge->sArrAlgoEdge[0].m_bTeachWidthUse)
				{
					if (((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistMin) == m_eAE_DistMin && sRstAlgo->m_dDistance < dDistanceMin) ||
						((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistMax) == m_eAE_DistMax && sRstAlgo->m_dDistance > dDistanceMax))
					{
						bResult = FALSE;
						sRstAlgo->m_bOKDistance = bResult;
					}
				}

				if (bDisX)
				{
					if (((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistXMin) == m_eAE_DistXMin && sRstAlgo->m_dDistanceX < dDistanceXMin) ||
						((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistXMax) == m_eAE_DistXMax && sRstAlgo->m_dDistanceX > dDistanceXMax))
					{
						bResult = FALSE;
						sRstAlgo->m_bOKDistanceX = bResult;
					}
				}

				if (bDisY)
				{
					if (((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistYMin) == m_eAE_DistYMin && sRstAlgo->m_dDistanceY < dDistanceYMin) ||
						((pInspAlgoAlignEdge->m_nOPT & m_eAE_DistYMax) == m_eAE_DistYMax && sRstAlgo->m_dDistanceY > dDistanceYMax))
					{
						bResult = FALSE;
						sRstAlgo->m_bOKDistanceY = bResult;
					}
				}
			}
			else
				sRstAlgo->m_dDistance = 0;
			nLine = __LINE__;
			if (pInspAlgoAlignEdge->m_nAreaCnt > 2)
			{
				int nAx = (dRstCX_1 + nArrLeft[0]), nAy = (dRstCY_1 + nArrTop[0]);
				int nBx = (dRstCX_2 + nArrLeft[1]), nBy = (dRstCY_2 + nArrTop[1]);
				int nCx = (dRstCX_3 + nArrLeft[2]), nCy = (dRstCY_3 + nArrTop[2]);
				double dAx = (nAx * m_resolX), dAy = (nAy * m_resolY);
				double dBx = (nBx * m_resolX), dBy = (nBy * m_resolY);
				double dCx = (nCx * m_resolX), dCy = (nCy * m_resolY);

				double dHx = 0.0, dHy = 0.0;	// rst

				double m = 0., n = 0.;// 기울기
				if (dBx - dAx == 0)
				{
					dHx = dAx;
					dHy = dCy;
				}
				else if ((dBy - dAy) == 0)
				{
					dHx = dCx;
					dHy = dAy;
					n = dAy;
				}
				else
				{
					m = (dBy - dAy) / (dBx - dAx);
					n = dAy - m * dAx;// y 절편
					if (m == 0)
						m = 0.00001;
					double l = dCy + m * dCx;
					dHx = (m * dCy + dCx - m * n) / (m * m + 1);
					dHy = (dCx + m * dCy - dHx) / m;
				}



				// 방향 (- , +)
				double vactor = 1;
				if (dBx - dAx > 0 && m * dCx + n < dCy)
					vactor = -1;
				else if (dBx - dAx < 0 && m * dCx + n > dCy)
					vactor = -1;
				else if (dBx - dAx == 0)
				{
					if (dBy - dAy > 0 && dCx > dAx)
						vactor = -1;
					else if (dBy - dAy < 0 && dCx < dAx)
						vactor = -1;
				}
				nLine = __LINE__;
				//double dDis1 = 0.;
				//if(abs(dHx - dAx) > 0.01 && abs(dHy - dAy) > 0.01)
				//{
				//	dDis1 = (sqrt(pow((dHx - dAx), 2) + pow((dHy - dAy), 2)));
				//}
				//else if(abs(dHx - dAx) > 0.01)	dDis1 = dHx - dAx;
				//else if(abs(dHy - dAy) > 0.01)	dDis1 = dHy - dAy;

				//double dDis2 = 0.;
				//if(abs(dHx - dCx) > 0.01 && abs(dHy - dCy) > 0.01)	dDis2 = sqrt(pow((dHx - dCx), 2) + pow((dHy - dCy), 2));
				//else if(abs(dHx - dCx) > 0.01)	dDis2 = dHx - dCx;
				//else if(abs(dHy - dCy) > 0.01)	dDis2 = dHy - dCy;

				//sRstAlgo->m_d3PtDis1 = dDis1;
				//sRstAlgo->m_d3PtDis3 = dDis2 *vactor;

				double dDis1 = floor(sqrt(pow((dHx - dAx), 2) + pow((dHy - dAy), 2)) * 1000) / 1000;
				double dDis2 = floor(sqrt(pow((dHx - dCx), 2) + pow((dHy - dCy), 2)) * 1000) / 1000;
				sRstAlgo->m_d3PtDis1 = dDis1 + pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeach1_3PtDisOff;
				sRstAlgo->m_d3PtDis3 = dDis2 * vactor + pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeach3_3PtDisOff;

				sRstAlgo->m_dArea[0] = dArrArea[0];
				sRstAlgo->m_dArea[1] = dArrArea[1];
				sRstAlgo->m_dArea[2] = dArrArea[2];
				int nHx = (dHx / m_resolX), nHy = (dHy / m_resolX);
				sRstAlgo->m_pHPoint[0].x = nAx;
				sRstAlgo->m_pHPoint[0].y = nAy;
				sRstAlgo->m_pHPoint[1].x = nBx;
				sRstAlgo->m_pHPoint[1].y = nBy;
				sRstAlgo->m_pHPoint[2].x = nCx;
				sRstAlgo->m_pHPoint[2].y = nCy;
				sRstAlgo->m_pHPoint[3].x = nHx;
				sRstAlgo->m_pHPoint[3].y = nHy;
				//sRstAlgo->m_d3PtDis1 = sqrt(pow((dHx - dAx) ,2) + pow((dHy - dAy),2));
				//sRstAlgo->m_d3PtDis3 = sqrt(pow((dHx - dCx) ,2) + pow((dHy - dCy),2)) * vactor;
#if AlignEdgeDebug
				bool saveFlag = false;
#endif
				if (sRstAlgo->m_d3PtDis1 >= pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeach1_3PtDisMin && sRstAlgo->m_d3PtDis1 <= pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeach1_3PtDisMax)
					sRstAlgo->m_bOK3PtDis1 = TRUE;
				else
				{
					bResult = FALSE;
					sRstAlgo->m_bOK3PtDis1 = FALSE;
#if AlignEdgeDebug
					saveFlag = true;
#endif
				}
				if (sRstAlgo->m_d3PtDis3 >= pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeach3_3PtDisMin && sRstAlgo->m_d3PtDis3 <= pInspAlgoAlignEdge->sArrAlgoEdge[0].m_dTeach3_3PtDisMax)
					sRstAlgo->m_bOK3PtDis3 = TRUE;
				else
				{
					bResult = FALSE;
					sRstAlgo->m_bOK3PtDis3 = FALSE;
#if AlignEdgeDebug
					saveFlag = true;
#endif
				}
#if AlignEdgeDebug
				if (saveFlag && (sWndAlgoImg.m_ucArr2D_Mix[0] != NULL && sWndAlgoImg.m_ucArr2D_Mix[1] != NULL))
				{
					CString sImageFileName;
					sImageFileName.Format(_T("D:\\Eagle3D_data\\AlignEdge\\ResultImage_%d_%.2f_%.2f.bmp"), m_nAlignEdgeCnt, sRstAlgo->m_d3PtDis1, sRstAlgo->m_d3PtDis3);

					int nAx = (dAx / m_resolX), nAy = (dAy / m_resolY);
					int nBx = (dBx / m_resolX), nBy = (dBy / m_resolY);
					int nCx = (dCx / m_resolX), nCy = (dCy / m_resolY);
					int nHx = dHx / m_resolX;
					int nHy = dHy / m_resolX;

					cv::Mat vSaveImage;
					cv::Mat vTmpImage(nImgHeight, nImgWidth, CV_8UC1, ucArrDstImg);
					cv::Mat* vBinSaveImage = &vTmpImage.clone();
					cv::cvtColor(Image0, vSaveImage, cv::COLOR_GRAY2RGB);

					cv::line(vSaveImage, cv::Point(nAx, nAy), cv::Point(nBx, nBy), cv::Scalar(0, 255, 0), 2);
					cv::line(vSaveImage, cv::Point(nHx, nHy), cv::Point(nCx, nCy), cv::Scalar(255, 0, 0), 2);

					cv::line(vSaveImage, cv::Point(nAx, nAy), cv::Point(nAx, nAy), cv::Scalar(0, 0, 255), 2);
					cv::line(vSaveImage, cv::Point(nBx, nBy), cv::Point(nBx, nBy), cv::Scalar(0, 0, 255), 2);
					cv::line(vSaveImage, cv::Point(nCx, nCy), cv::Point(nCx, nCy), cv::Scalar(0, 0, 255), 2);

					FiduRect[0].center.x = nAx;
					FiduRect[0].center.y = nAy;
					FiduRect[1].center.x = nBx;
					FiduRect[1].center.y = nBy;

					cv::ellipse(vSaveImage, FiduRect[0], cv::Scalar(126, 126, 126));
					cv::ellipse(vSaveImage, FiduRect[1], cv::Scalar(126, 126, 126));

					cv::imwrite(std::string(CT2A(sImageFileName)), vSaveImage);


					cv::cvtColor(vTmpImage, *vBinSaveImage, cv::COLOR_GRAY2RGB);

					cv::line(*vBinSaveImage, cv::Point(nAx, nAy), cv::Point(nBx, nBy), cv::Scalar(0, 255, 0), 2);
					cv::line(*vBinSaveImage, cv::Point(nHx, nHy), cv::Point(nCx, nCy), cv::Scalar(255, 0, 0), 2);

					cv::line(*vBinSaveImage, cv::Point(nAx, nAy), cv::Point(nAx, nAy), cv::Scalar(0, 0, 255), 2);
					cv::line(*vBinSaveImage, cv::Point(nBx, nBy), cv::Point(nBx, nBy), cv::Scalar(0, 0, 255), 2);
					cv::line(*vBinSaveImage, cv::Point(nCx, nCy), cv::Point(nCx, nCy), cv::Scalar(0, 0, 255), 2);

					cv::ellipse(*vBinSaveImage, FiduRect[0], cv::Scalar(126, 126, 126));
					cv::ellipse(*vBinSaveImage, FiduRect[1], cv::Scalar(126, 126, 126));

					sImageFileName.Format(_T("D:\\Eagle3D_data\\AlignEdge\\ResultImage_%d_%.2f_%.2f_Bin.bmp"), m_nAlignEdgeCnt, sRstAlgo->m_d3PtDis1, sRstAlgo->m_d3PtDis3);
					cv::imwrite(std::string(CT2A(sImageFileName)), *vBinSaveImage);

					g_pMPTI->AddLog_Dev(sImageFileName);
					sImageFileName.Format(_T("A: %d,%d / B: %d,%d / C: %d,%d / H: %d,%d"), nAx, nAy, nBx, nBy, nCx, nCy, nHx, nHy);
					g_pMPTI->AddLog_Dev(sImageFileName);
					m_nAlignEdgeCnt++;

					if (ucDstImgNull)
						Delete_1DArray(&ucArrDstImg);
				}
#endif
				if (ucArrDstImg != NULL)
				{
					int nAx = (dAx / m_resolX), nAy = (dAy / m_resolY);
					int nBx = (dBx / m_resolX), nBy = (dBy / m_resolY);
					int nCx = (dCx / m_resolX), nCy = (dCy / m_resolY);
					int nHx = dHx / m_resolX;
					int nHy = dHy / m_resolX;
					FiduRect[0].center.x = nAx;
					FiduRect[0].center.y = nAy;
					FiduRect[1].center.x = nBx;
					FiduRect[1].center.y = nBy;
					FiduRect[2].center.x = nCx;
					FiduRect[2].center.y = nCy;

					sRstAlgo->m_rcFindCircle[0].left = FiduRect[0].center.x - FiduRect[0].size.width / 2;
					sRstAlgo->m_rcFindCircle[0].top = FiduRect[0].center.y - FiduRect[0].size.height / 2;
					sRstAlgo->m_rcFindCircle[0].right = FiduRect[0].center.x + FiduRect[0].size.width / 2;
					sRstAlgo->m_rcFindCircle[0].bottom = FiduRect[0].center.y + FiduRect[0].size.height / 2;

					sRstAlgo->m_rcFindCircle[1].left = FiduRect[1].center.x - FiduRect[1].size.width / 2;
					sRstAlgo->m_rcFindCircle[1].top = FiduRect[1].center.y - FiduRect[1].size.height / 2;
					sRstAlgo->m_rcFindCircle[1].right = FiduRect[1].center.x + FiduRect[1].size.width / 2;
					sRstAlgo->m_rcFindCircle[1].bottom = FiduRect[1].center.y + FiduRect[1].size.height / 2;

					float fRe3W = abs(sRstAlgo->m_poDrawLine_2[0].x - sRstAlgo->m_poDrawLine_Sec_2[0].x);
					float fRe3H = abs(sRstAlgo->m_poDrawLine_2[1].y - sRstAlgo->m_poDrawLine_Sec_2[1].y);
					if (fRe3W < abs(sRstAlgo->m_poDrawLine_2[1].x - sRstAlgo->m_poDrawLine_Sec_2[1].x))
					{
						fRe3W = abs(sRstAlgo->m_poDrawLine_2[1].x - sRstAlgo->m_poDrawLine_Sec_2[1].x) + 0.5;
						fRe3H = abs(sRstAlgo->m_poDrawLine_2[0].y - sRstAlgo->m_poDrawLine_Sec_2[0].y) + 0.5;
					}

					sRstAlgo->m_rcFindCircle[2].left = FiduRect[2].center.x - fRe3W / 2;
					sRstAlgo->m_rcFindCircle[2].top = FiduRect[2].center.y - fRe3H / 2;
					sRstAlgo->m_rcFindCircle[2].right = FiduRect[2].center.x + fRe3W / 2;
					sRstAlgo->m_rcFindCircle[2].bottom = FiduRect[2].center.y + fRe3H / 2;
				}
			}

		}
		nLine = __LINE__;
		// 	sLog.Format(_T("Func_InspAlignEdge, end "));
		// 	g_pMPTI->AddLog_Dev(sLog);

	}
	catch (CMemoryException* e)
	{
		sLog.Format(_T("Func_InspAlignEdge, CMemoryException* e: %d "), nLine);
		g_pMPTI->AddLog_Dev(sLog);

	}
	catch (CFileException* e)
	{
		sLog.Format(_T("Func_InspAlignEdge, CFileException* e: %d "), nLine);
		g_pMPTI->AddLog_Dev(sLog);
	}
	catch (CException* e)
	{
		sLog.Format(_T("Func_InspAlignEdge, CException* e: %d "), nLine);
		g_pMPTI->AddLog_Dev(sLog);
	}
	catch (...)
	{
		sLog.Format(_T("Func_InspAlignEdge, ...: %d "), nLine);
		g_pMPTI->AddLog_Dev(sLog);
	}

	return bResult;
}

BOOL CPInsp_AlgoAlignEdge::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	AlgoAlignEdge *pAlgo = (AlgoAlignEdge *)ptrInspAlgoParam;

	RstAlgoAlignEdge * rst = (RstAlgoAlignEdge *)sRstAlgo;
	pAlignRes->offsetX = -rst->m_dRstShiftX;
	pAlignRes->offsetY = rst->m_dRstShiftY;
	pAlignRes->theta = rst->m_dTheta;

	pAlignRes->m_bAlgoCenter = g_pInspMng->m_bChangeRotateCenter;
	pAlignRes->m_dCenterX_T = rst->m_dCenterX_T;
	pAlignRes->m_dCenterY_T = rst->m_dCenterY_T;
	pAlignRes->m_dCenterX_R = rst->m_dCenterX_R;
	pAlignRes->m_dCenterY_R = rst->m_dCenterY_R;

	bRet = TRUE;

	return bRet;
}
int CPInsp_AlgoAlignEdge::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;
	AlgoAlignEdge * pInspAlgoAlign = (AlgoAlignEdge *)sInspAlgo.m_ptrInspAlgoParam;
	for (int nID = 0; nID < ALIGNEDGE_AREA_CNTS; nID++)
	{
		if (nIndex >= 0 && nIndex != nID)
			continue;

		nData = pInspAlgoAlign->sArrAlgoEdge[nID].m_sAlgoColorBase.GetColorData();
		if ((nData & COLOR_DATA_USE) == COLOR_DATA_USE)
			break;
	}
	return nData;
}
bool CPInsp_AlgoAlignEdge::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}