#include "PInsp_AlgoTilt.h"


#define CalcTiltHeight 1
CPInsp_AlgoTilt::CPInsp_AlgoTilt(void)
{
}


CPInsp_AlgoTilt::~CPInsp_AlgoTilt(void)
{
}

void CPInsp_AlgoTilt::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoTilt::GetInspAlgoData()
{
	return eSPCAlgoTilt;
}

int CPInsp_AlgoTilt::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeMountTilt] = e_NG;
	nCurrentNgType = TypeMountTilt;
	return nCurrentNgType;
}

bool CPInsp_AlgoTilt::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoTilt::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspTilt(sInspAlgo, sWndAlgoImg, (RstAlgoTilt *)sRstAlgo, stAlgoParam.m_sAlignRes, stAlgoParam.m_nAlignCnt);

	return bResult;
}

BOOL CPInsp_AlgoTilt::InspTilt(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoTilt *sRstAlgo, AlignResult * sAlignRes, int nAlignResCnt)
{
	BOOL bReturn = FALSE;
	int nLine = __LINE__;
	try
	{
		if (sRstAlgo)
		{
			memset(sRstAlgo, 0, sizeof(RstAlgoTilt));
			sRstAlgo->Init();
		}

		if (sInspAlgo.m_eAlgoType != eAlgoTilt)
			return bReturn;
		if (!m_pProcMilAlgo)
			return bReturn;

		AlgoTilt *pInspAlgoTilt = (AlgoTilt *)sInspAlgo.m_ptrInspAlgoParam;
		if (!pInspAlgoTilt)
			return bReturn;
		if ((pInspAlgoTilt->nSelectValue <= 1) || (pInspAlgoTilt->nSelectValue > 4))
			return bReturn;
		if (pInspAlgoTilt->rcArrSelectROI == NULL)
			return bReturn;
		if (pInspAlgoTilt->m_bUseAreaDiff == FALSE && pInspAlgoTilt->m_bUsePart == FALSE && pInspAlgoTilt->m_bUseAngle == FALSE)
			pInspAlgoTilt->m_bUseAreaDiff = TRUE;
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if (pfImgSrc == NULL)
			pfImgSrc = sWndAlgoImg.m_fArr3D_part;

		if ((pfImgSrc == NULL) && (nImgWidth <= 0) && (nImgHeight <= 0))
			return bReturn;

		float fMinValue = 0.0;//std::numeric_limits<float>::max();
		float fMaxValue = 0.0;
		double dHeightDiffMin = pInspAlgoTilt->m_dHeightDiffMin;
		double dHeightDiffMax = pInspAlgoTilt->m_dHeightDiffMax;
		double dArrRectHeightValue[TiltRectCnt];
		memset(dArrRectHeightValue, 0, TiltRectCnt * sizeof(double));

		//shkim 2018.03.26 tilt angle param
		double distance;
		double *A, *B, *C;
		//		distance = new double[pInspAlgoTilt->nSelectValue];
				/*A = new double[2];
				B = new double[2];
				C = new double[2];*/
		A = g_pMManager->pem_new<double>(true, 2, (PCHAR)__FUNCTION__, __LINE__);
		B = g_pMManager->pem_new<double>(true, 2, (PCHAR)__FUNCTION__, __LINE__);
		C = g_pMManager->pem_new<double>(true, 2, (PCHAR)__FUNCTION__, __LINE__);
		////////////////////////////////////////////////
		bool bInspHighest = ((pInspAlgoTilt->m_nInspAreaType & (int)m_eHighest) == (int)m_eHighest);
		bool bInspOneMore = ((pInspAlgoTilt->m_nInspAreaType & (int)m_eOneMore) == (int)m_eOneMore);

		for (int i = 0; i < pInspAlgoTilt->nSelectValue; i++)
		{

			nLine = __LINE__;

			RECT rcDstRect;
			if (m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoTilt->rcArrSelectROI[i], &rcDstRect) == FALSE)
			{
				dArrRectHeightValue[i] = 0;
				continue;
			}

			nLine = __LINE__;

			double dROIWidth = rcDstRect.right - rcDstRect.left;
			double dROIHeight = rcDstRect.bottom - rcDstRect.top;
			// 		double dROIX = rcDstRect.left + (dROIWidth / 2);
			// 		double dROIY = rcDstRect.top + (dROIHeight / 2);
			double dROIX = rcDstRect.left;
			double dROIY = rcDstRect.top;
			if (nAlignResCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
			{
				dROIX = dROIX + (dROIWidth / 2.0) - ((double)nImgWidth / 2.0);
				dROIY = ((double)nImgHeight / 2.0) - (dROIY + (dROIHeight / 2.0));

				double corr_x = 0, corr_y = 0;
				for (int n = 0; n < nAlignResCnt; n++)
				{
					double corr_x_Buf = 0, corr_y_Buf = 0;
					//m_proc3d.CorrectCoordinate(dFirstX, dFirstY, 0, 0, sAlignRes->theta, sAlignRes->offsetX / m_resolX, -sAlignRes->offsetY / m_resolY, &corr_x, &corr_y);
					int nCorrectCoordinate = m_proc3d.CorrectCoordinate(dROIX, dROIY, sAlignRes[n].centerX / m_resolX, sAlignRes[n].centerY / m_resolY, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
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
				dROIX = RounD(corr_x + ((double)nImgWidth / 2.0) - (dROIWidth / 2.0));
				dROIY = RounD(((double)nImgHeight / 2.0) - (corr_y + (dROIHeight / 2.0)));

				if (dROIX + RounD(dROIWidth) > nImgWidth)		// 2014/12/15	: 예외처리
					dROIX = nImgWidth - dROIWidth;
				if (dROIY + RounD(dROIHeight) > nImgHeight)
					dROIY = nImgHeight - dROIHeight;
			}

			nLine = __LINE__;

			//float *pfArrImgDst = new float[(int)(dROIWidth * dROIHeight)];
			float *pfArrImgDst = g_pMManager->pem_new<float>(true, (int)(dROIWidth * dROIHeight), (PCHAR)__FUNCTION__, __LINE__);
			memset(pfArrImgDst, 0, dROIWidth * dROIHeight * sizeof(float));
			try
			{
				if (dROIX < 0 || dROIX + dROIWidth > nImgWidth || dROIY < 0 || dROIY + dROIHeight > nImgHeight)
				{
					g_pMManager->pem_delete(A, true);
					g_pMManager->pem_delete(B, true);
					g_pMManager->pem_delete(C, true);
					g_pMManager->pem_delete(pfArrImgDst, true);
					return false;
				}
			}
			catch (CMemoryException* e)
			{
				CString sLog = _T("");
				sLog.Format(_T("CPInsp_Algo::InspTilt(), Line : %d Pass!!!"), dROIX, dROIY);
				g_pMPTI->AddLog_Dev(sLog);

			}

			m_proc3d.GetCropZmap_LT(pfImgSrc, pfArrImgDst, nImgWidth, nImgHeight, dROIX, dROIY, dROIWidth, dROIHeight);
			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("3DImage.bmp"));
			m_pProcMilAlgo->SaveWorkImg_float(pfArrImgDst, dROIWidth, dROIHeight, _T("Clip3DImage.bmp"));

			nLine = __LINE__;
			float fSelectROIMean = 0;
			if (pInspAlgoTilt->m_bUseAngle == true)
				fSelectROIMean = m_pCPInsp_Algo->GetHeightMaxHist(pfArrImgDst, dROIWidth, dROIHeight);
			else
			{
				if (bInspHighest)
					fSelectROIMean = m_pCPInsp_Algo->GetMinMaxImg(pfArrImgDst, dROIWidth, dROIHeight, 2);
				else
					fSelectROIMean = m_pCPInsp_Algo->GetHeightMean(pfArrImgDst, dROIWidth, dROIHeight, 0, bInspOneMore);
			}

			dArrRectHeightValue[i] = fSelectROIMean;

			if (sRstAlgo)
			{
				sRstAlgo->m_rcRect_I[i].left = dROIX;
				sRstAlgo->m_rcRect_I[i].top = dROIY;
				sRstAlgo->m_rcRect_I[i].right = dROIX + dROIWidth;
				sRstAlgo->m_rcRect_I[i].bottom = dROIY + dROIHeight;
			}
			nLine = __LINE__;

			if (dHeightDiffMin > 0 || dHeightDiffMax > 0)
			{
				int nClipSize = 1;
				int nMeanCnt = 0;
				float fExceptMinValue = fSelectROIMean - dHeightDiffMin;
				float fExceptMaxValue = fSelectROIMean + dHeightDiffMax;
				if (fExceptMinValue > 0)
				{
					//float *pfArrImgClip = new float[nClipSize];
					float *pfArrImgClip = g_pMManager->pem_new<float>(true, nClipSize, (PCHAR)__FUNCTION__, __LINE__);
					memset(pfArrImgClip, 0, nClipSize * sizeof(float));
					float fSumROIMean = 0.0f;
					for (int y = 0; y < dROIHeight; y++)
					{
						for (int x = 0; x < dROIWidth; x++)
						{
							nLine = __LINE__;

							m_proc3d.GetCropZmap_LT(pfImgSrc, pfArrImgClip, nImgWidth, nImgHeight, dROIX + x, dROIY + y, nClipSize, nClipSize);

							nLine = __LINE__;

							float fClipROIMean = m_pCPInsp_Algo->GetHeightMean(pfArrImgClip, nClipSize, nClipSize, 0, bInspOneMore);
							//float fClipROIMean =  m_pCPInsp_Algo->GetHeightMaxHist(pfArrImgClip, nClipSize, nClipSize);

							nLine = __LINE__;

							bool bContinue = false;
							if (fClipROIMean > fExceptMinValue && fClipROIMean < fExceptMaxValue)
							{
								fSumROIMean += fClipROIMean;
								bContinue = true;
								nMeanCnt++;
							}
							if (bContinue == false)
							{
								if (dHeightDiffMin == 0)
								{
									if (fClipROIMean < fExceptMinValue)
									{
										fSumROIMean += fClipROIMean;
										nMeanCnt++;
									}
								}
								if (dHeightDiffMax == 0)
								{
									if (fClipROIMean > fExceptMaxValue)
									{
										fSumROIMean += fClipROIMean;
										nMeanCnt++;
									}
								}
							}
						}
					}

					nLine = __LINE__;
					if (nMeanCnt > 0)
						fSelectROIMean = fSumROIMean / nMeanCnt;
					nLine = __LINE__;

					if (pfArrImgClip)
						Delete_1DArray(&pfArrImgClip);
					dArrRectHeightValue[i] = fSelectROIMean;

					nLine = __LINE__;

				}
			}


			if (i == 0)
			{
				fMinValue = fSelectROIMean;
				fMaxValue = fSelectROIMean;
			}
			else
			{
				if (fMinValue > fSelectROIMean)
					fMinValue = fSelectROIMean;

				if (fMaxValue < fSelectROIMean)
					fMaxValue = fSelectROIMean;
			}

			fSelectROIMean = 0.0;

			nLine = __LINE__;



			if (pfArrImgDst)
				Delete_1DArray(&pfArrImgDst);

			nLine = __LINE__;
		}

		bReturn = TRUE;

		if (sRstAlgo)
		{
			for (int n = 0; n < pInspAlgoTilt->nSelectValue; n++)
				sRstAlgo->m_dArrRectHeight[n] = dArrRectHeightValue[n];
			sRstAlgo->m_bOKArea = TRUE;
			if (pInspAlgoTilt->m_bUseAreaDiff == true)
			{
				sRstAlgo->m_dRstHeightDiff = (fMaxValue - fMinValue);
				sRstAlgo->m_dHeightDiffMin = fMinValue;
				sRstAlgo->m_dHeightDiffMax = fMaxValue;
				if (sRstAlgo->m_dRstHeightDiff > pInspAlgoTilt->dAllowDiff)
					sRstAlgo->m_bOKArea = FALSE;
			}
			else
			{
				sRstAlgo->m_dRstHeightDiff = 0;
				sRstAlgo->m_dHeightDiffMin = 0;
				sRstAlgo->m_dHeightDiffMax = 0;
			}

			sRstAlgo->m_bOKAvg = TRUE;
			if (pInspAlgoTilt->m_bUsePart == true)
			{
				double dHeightAvg = pInspAlgoTilt->m_dHeightAvg;
				double dHeightMin = dHeightAvg * (pInspAlgoTilt->m_dHeightMin / 100.0);
				double dHeightMax = dHeightAvg * (pInspAlgoTilt->m_dHeightMax / 100.0);
				for (int n = 0; n < pInspAlgoTilt->nSelectValue; n++)
				{
					if (dArrRectHeightValue[n] > dHeightMin && dArrRectHeightValue[n] < dHeightMax)
						sRstAlgo->m_bArrOKHeightAvg[n] = TRUE;
					else
					{
						sRstAlgo->m_bOKAvg = FALSE;
						sRstAlgo->m_bArrOKHeightAvg[n] = FALSE;
					}
				}
			}
			else
			{
				for (int n = 0; n < pInspAlgoTilt->nSelectValue; n++)
					sRstAlgo->m_bArrOKHeightAvg[n] = TRUE;
			}
			// 			double* distance;
			// 			float *A,*B,*C;
			// 			distance = new double[pInspAlgoTilt->nSelectValue];
			// 			A = new float[pInspAlgoTilt->nSelectValue];
			// 			B = new float[pInspAlgoTilt->nSelectValue];
			// 			C = new float[pInspAlgoTilt->nSelectValue];

			sRstAlgo->m_bAngle = TRUE;
			//degree inspection
			if (pInspAlgoTilt->m_bUseAngle == true &&
				pInspAlgoTilt->rcAngleStdROI[0].right - pInspAlgoTilt->rcAngleStdROI[0].left > 0 &&
				pInspAlgoTilt->rcAngleStdROI[0].bottom - pInspAlgoTilt->rcAngleStdROI[0].top > 0)
			{
#pragma region Tile ROI angle
				//calc///////////////////////////////////////////////////////////////////////////////////////////////////////////

				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				/*float *height = new float[pInspAlgoTilt->nSelectValue];
				POINTF* pint = new POINTF[pInspAlgoTilt->nSelectValue];*/
				float *height = g_pMManager->pem_new<float>(true, pInspAlgoTilt->nSelectValue, (PCHAR)__FUNCTION__, __LINE__);
				POINTF* pint = g_pMManager->pem_new<POINTF>(true, pInspAlgoTilt->nSelectValue, (PCHAR)__FUNCTION__, __LINE__);

				for (int RoiIdx = 0; RoiIdx < pInspAlgoTilt->nSelectValue; RoiIdx++)
				{
					height[RoiIdx] = (float)dArrRectHeightValue[RoiIdx];
					float x = (sRstAlgo->m_rcRect_I[RoiIdx].right - sRstAlgo->m_rcRect_I[RoiIdx].left) / 2 + sRstAlgo->m_rcRect_I[RoiIdx].left;
					float y = (sRstAlgo->m_rcRect_I[RoiIdx].bottom - sRstAlgo->m_rcRect_I[RoiIdx].top) / 2 + sRstAlgo->m_rcRect_I[RoiIdx].top;

					pint[RoiIdx].x = x;
					pint[RoiIdx].y = y;
				}
				float avg = m_pCPInsp_Algo->LeastSquarePlane(pInspAlgoTilt->nSelectValue, height, pint, &A[1], &B[1], &C[1]);
#if CalcTiltHeight
				//float* CalcHight = new float[nImgHeight*nImgWidth];
				float* CalcHight = g_pMManager->pem_new<float>(true, nImgHeight*nImgWidth, (PCHAR)__FUNCTION__, __LINE__);

				for (int r = 0; r < nImgHeight; r++)
				{
					for (int c = 0; c < nImgWidth; c++)
					{
						//						double tempHight= A[1]*c+B[1]*r+avg;
						CalcHight[r * nImgWidth + c] = pfImgSrc[r * nImgWidth + c] - avg;//tempHight;
					}
				}
#else

#endif

				/*RECT* rcDstRect = new RECT[nTiltAngleCnt];
				float *	he = new float[pInspAlgoTilt->nSelectAngleValue];
				POINTF* pnt = new POINTF[pInspAlgoTilt->nSelectAngleValue];

				float **pfArrImgDst = new float*[pInspAlgoTilt->nSelectAngleValue];
				double* dROIWidth = new double[pInspAlgoTilt->nSelectAngleValue];
				double* dROIHeight = new double[pInspAlgoTilt->nSelectAngleValue];*/
				RECT* rcDstRect = g_pMManager->pem_new<RECT>(true, nTiltAngleCnt, (PCHAR)__FUNCTION__, __LINE__);
				float *	he = g_pMManager->pem_new<float>(true, pInspAlgoTilt->nSelectAngleValue, (PCHAR)__FUNCTION__, __LINE__);
				POINTF* pnt = g_pMManager->pem_new<POINTF>(true, pInspAlgoTilt->nSelectAngleValue, (PCHAR)__FUNCTION__, __LINE__);

				float **pfArrImgDst = g_pMManager->pem_new<float*>(true, pInspAlgoTilt->nSelectAngleValue, (PCHAR)__FUNCTION__, __LINE__);
				double* dROIWidth = g_pMManager->pem_new<double>(true, pInspAlgoTilt->nSelectAngleValue, (PCHAR)__FUNCTION__, __LINE__);
				double* dROIHeight = g_pMManager->pem_new<double>(true, pInspAlgoTilt->nSelectAngleValue, (PCHAR)__FUNCTION__, __LINE__);


				int ImgH = nImgHeight;
				int ImgW = nImgWidth;
				AlgoBlob AlBlob;
				AlBlob.m_bInsp2D = false;
				AlBlob.m_bInsp3D = true;
				AlBlob.m_bFillHole = false;
				AlBlob.m_nTypeRange3D = (int)ETypeInspRange::eTypeRangeIn;//0:in
				AlBlob.m_dHeightRateMin = pInspAlgoTilt->m_dAngleBinMin;//3250.0;
				AlBlob.m_dHeightRateMax = pInspAlgoTilt->m_dAngleBinMax;//3350.0;
				AlBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;//ETypeBlob::eSelectBigger;
				/*UCHAR* pUcImgSrc = new UCHAR[1];
				UCHAR* pUcImgBlob = new UCHAR[nImgHeight*nImgWidth];*/
				UCHAR* pUcImgSrc = g_pMManager->pem_new<UCHAR>(true, 1, (PCHAR)__FUNCTION__, __LINE__);
				UCHAR* pUcImgBlob = g_pMManager->pem_new<UCHAR>(true, nImgHeight*nImgWidth, (PCHAR)__FUNCTION__, __LINE__);

				double dArea;

				TotalInspExceptArea stTieArea;
				stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
				CRect rcBlob;
				double dCX, dCY;
				UCHAR * ucColorImgDst = NULL;
#if CalcTiltHeight
				int nCntBlob = m_pCPInsp_Algo->BlobImageStruct(AlBlob, pUcImgSrc, CalcHight, ucColorImgDst, nImgWidth, nImgHeight, 1, &dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, false);
#else
				int nCntBlob = m_pCPInsp_Algo->BlobImageStruct(AlBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, 1, &dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, false);
#endif
				cv::Mat ucImgBlobMat(nImgHeight, nImgWidth, CV_8UC1, pUcImgBlob);

				//ExceptPointed(&ucImgBlobMat,CalcHight);
#if _DEBUG
				cv::imwrite("D:\\Eagle3D_data\\Mat.bmp", ucImgBlobMat);
#endif
				for (int nAglIdx = 0; nAglIdx < pInspAlgoTilt->nSelectAngleValue; nAglIdx++)
				{
					m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoTilt->rcAngleStdROI[nAglIdx], &rcDstRect[nAglIdx]);

					/*double*/ dROIWidth[nAglIdx] = rcDstRect[nAglIdx].right - rcDstRect[nAglIdx].left;
					/*double*/ dROIHeight[nAglIdx] = rcDstRect[nAglIdx].bottom - rcDstRect[nAglIdx].top;
					// 		double dROIX = rcDstRect.left + (dROIWidth / 2);
					// 		double dROIY = rcDstRect.top + (dROIHeight / 2);
					double dROIX = rcDstRect[nAglIdx].left;
					double dROIY = rcDstRect[nAglIdx].top;
					if (nAlignResCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
					{
						dROIX = dROIX + (dROIWidth[nAglIdx] / 2.0) - ((double)nImgWidth / 2.0);
						dROIY = ((double)nImgHeight / 2.0) - (dROIY + (dROIHeight[nAglIdx] / 2.0));

						double corr_x = 0, corr_y = 0;
						for (int n = 0; n < nAlignResCnt; n++)
						{
							double corr_x_Buf = 0, corr_y_Buf = 0;
							//m_proc3d.CorrectCoordinate(dFirstX, dFirstY, 0, 0, sAlignRes->theta, sAlignRes->offsetX / m_resolX, -sAlignRes->offsetY / m_resolY, &corr_x, &corr_y);
							int nCorrectCoordinate = m_proc3d.CorrectCoordinate(dROIX, dROIY, sAlignRes[n].centerX / m_resolX, sAlignRes[n].centerY / m_resolY, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
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
						dROIX = RounD(corr_x + ((double)nImgWidth / 2.0) - (dROIWidth[nAglIdx] / 2.0));
						dROIY = RounD(((double)nImgHeight / 2.0) - (corr_y + (dROIHeight[nAglIdx] / 2.0)));

						if (dROIX + RounD(dROIWidth[nAglIdx]) > nImgWidth)		// 2014/12/15	: 예외처리
							dROIX = nImgWidth - dROIWidth[nAglIdx];
						if (dROIY + RounD(dROIHeight[nAglIdx]) > nImgHeight)
							dROIY = nImgHeight - dROIHeight[nAglIdx];
					}

					nLine = __LINE__;

					///*float **/pfArrImgDst[nAglIdx] = new float[(int)(dROIWidth[nAglIdx] * dROIHeight[nAglIdx])];
					/*float **/pfArrImgDst[nAglIdx] = g_pMManager->pem_new<float>(true, (int)(dROIWidth[nAglIdx] * dROIHeight[nAglIdx]), (PCHAR)__FUNCTION__, __LINE__);
					memset(pfArrImgDst[nAglIdx], 0, dROIWidth[nAglIdx] * dROIHeight[nAglIdx] * sizeof(float));

#if CalcTiltHeight
					m_proc3d.GetCropZmap_LT(CalcHight, pfArrImgDst[nAglIdx], nImgWidth, nImgHeight, dROIX, dROIY, dROIWidth[nAglIdx], dROIHeight[nAglIdx]);
#else
					m_proc3d.GetCropZmap_LT(pfImgSrc, pfArrImgDst[nAglIdx], nImgWidth, nImgHeight, dROIX, dROIY, dROIWidth[nAglIdx], dROIHeight[nAglIdx]);
#endif
					m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("3DImage.bmp"));
					m_pProcMilAlgo->SaveWorkImg_float(pfArrImgDst[nAglIdx], dROIWidth[nAglIdx], dROIHeight[nAglIdx], _T("Clip3DImage.bmp"));

					nLine = __LINE__;
					cv::Rect cvRe(dROIX, dROIY, dROIWidth[nAglIdx], dROIHeight[nAglIdx]);
					cv::Mat MatCropImg = ucImgBlobMat(cvRe).clone();
#if _DEBUG
					cv::imwrite("D:\\Eagle3D_data\\Matcrpo.bmp", MatCropImg);
#endif
					//float fSelectROIMean =  m_pCPInsp_Algo->GetHeightMean(pfArrImgDst[nAglIdx], dROIWidth[nAglIdx], dROIHeight[nAglIdx]);
					float fSelectROIMean = 0;
					if (pInspAlgoTilt->m_dAngleBinMin == -5000 && pInspAlgoTilt->m_dAngleBinMax == 5000)
					{
						fSelectROIMean = m_pCPInsp_Algo->GetHeightMaxHist(pfArrImgDst[nAglIdx], dROIWidth[nAglIdx], dROIHeight[nAglIdx]);
						sRstAlgo->m_dRstStdHeightMax += fSelectROIMean;
					}
					else
					{
						fSelectROIMean = m_pCPInsp_Algo->GetHeightAvgMask(pfArrImgDst[nAglIdx], dROIWidth[nAglIdx], dROIHeight[nAglIdx], MatCropImg.ptr(0));
						if (-HUGE_VAL != fSelectROIMean)
							sRstAlgo->m_dRstStdHeightMax += fSelectROIMean;
					}

					he[nAglIdx] = fSelectROIMean;
					float x = dROIX + dROIWidth[nAglIdx] / 2;
					float y = dROIY + dROIHeight[nAglIdx] / 2;

					pnt[nAglIdx].x = x;
					pnt[nAglIdx].y = y;
					// 					int nROIWidth =dROIWidth[nAglIdx];
					// 					CString sFileName;
					// 					sFileName.Format(_T("D:\\Eagle3D_data\\TiltAngle_%d.txt"),nAglIdx);
					// 					CString sLog;
					// 					CStdioFile file;
					// 					if(file.Open(sFileName, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate)==TRUE)
					// 					{
					// 						file.SeekToEnd();
					// 
					// 						for (int rtmp = 0; rtmp < dROIHeight[nAglIdx]; rtmp++)
					// 						{
					// 							for (int ctmp = 0; ctmp < dROIWidth[nAglIdx]; ctmp++)
					// 							{
					// 								sLog.Format(_T("%.3f,"), pfArrImgDst[nAglIdx][rtmp*nROIWidth+ctmp]);
					// 								file.WriteString(sLog);
					// 							}
					// 							sLog.Format(_T("\n"));
					// 							file.WriteString(sLog);
					// 						}
					// 						sLog.Format(_T("================\n"));
					// 						file.WriteString(sLog);
					// 						file.Close();
					// 					}

				}
				sRstAlgo->m_dRstStdHeightMax = sRstAlgo->m_dRstStdHeightMax / pInspAlgoTilt->nSelectAngleValue;


				if (pInspAlgoTilt->nSelectAngleValue < 2)
					m_pCPInsp_Algo->CalcTiltAngle(pfArrImgDst[0], dROIWidth[0], dROIHeight[0], &A[0], &B[0], &C[0]);
				else
				{
					int nHidx = 0;
					for (int selval = 0; selval < pInspAlgoTilt->nSelectAngleValue; selval++)
					{
						if (he[selval] != -HUGE_VAL)
						{
							he[nHidx] = he[selval];
							pnt[nHidx++] = pnt[selval];
						}
					}
					if (nHidx < 2)
						m_pCPInsp_Algo->CalcTiltAngle(pfArrImgDst[0], dROIWidth[0], dROIHeight[0], &A[0], &B[0], &C[0]);
					else
						m_pCPInsp_Algo->LeastSquarePlane(nHidx, he, pnt, &A[0], &B[0], &C[0]);
				}

				if (pfArrImgDst)
				{
					for (int ArrIdx = 0; ArrIdx < pInspAlgoTilt->nSelectAngleValue; ArrIdx++)
						Delete_1DArray(&pfArrImgDst[ArrIdx]);

					//delete [] pfArrImgDst;
					g_pMManager->pem_delete(pfArrImgDst, true);
				}

				/*delete [] pUcImgSrc;
				delete [] pUcImgBlob;
				delete [] he;
				delete [] pnt;*/
				g_pMManager->pem_delete(pUcImgSrc, true);
				g_pMManager->pem_delete(pUcImgBlob, true);
				g_pMManager->pem_delete(he, true);
				g_pMManager->pem_delete(pnt, true);
#if CalcTiltHeight
				//delete [] CalcHight;
				g_pMManager->pem_delete(CalcHight, true);
#endif

				double maxAngle(0.0f);
				double RadianTemp = acos(C[0] / ((double)(sqrt(A[0] * A[0] + B[0] * B[0] + C[0] * C[0]))));
				if (RadianTemp < 0)
					RadianTemp = -RadianTemp;
				maxAngle = abs(RadianTemp * 180 / PI);
				sRstAlgo->m_dRstAngleMax = sRstAlgo->m_dArrAngle[0] = maxAngle;

				maxAngle = 0.0;
				RadianTemp = 0.0;
				RadianTemp = acos(C[1] / sqrt((double)(A[1] * A[1] + B[1] * B[1] + C[1] * C[1])));
				if (RadianTemp < 0)
					RadianTemp = -RadianTemp;
				maxAngle = abs(RadianTemp * 180 / PI);
				sRstAlgo->m_dArrAngle[1] = maxAngle;

				maxAngle = 0.0;
				RadianTemp = 0.0;
				double tmp3 = (double)(abs(A[0] * A[1] + B[0] * B[1] + C[0] * C[1])) / (double)(sqrt((double)(A[1] * A[1] + B[1] * B[1] + C[1] * C[1]))*sqrt((double)(A[0] * A[0] + B[0] * B[0] + C[0] * C[0])));
				if (tmp3 > 1)
					tmp3 = 1;
				else if (tmp3 < -1)
					tmp3 = -1;
				RadianTemp = acos(tmp3);


				if (RadianTemp < 0)
					RadianTemp = -RadianTemp;
				maxAngle = abs(RadianTemp * 180 / PI);
				sRstAlgo->m_dRstAngleMax = maxAngle;

				if (sRstAlgo->m_dRstAngleMax < 0)
				{
					sRstAlgo->m_dRstAngleMax = -sRstAlgo->m_dRstAngleMax;
				}

				double currentAngle = abs(pInspAlgoTilt->m_dZAngle - sRstAlgo->m_dRstAngleMax);

				if (currentAngle > pInspAlgoTilt->m_dAngleMaxDiff)
				{
					sRstAlgo->m_bAngle = FALSE;
				}
				else
				{
					sRstAlgo->m_bAngle = TRUE;
				}

				/*delete [] height;
				delete [] pint;
				delete [] rcDstRect;
				delete [] dROIWidth ;
				delete [] dROIHeight;*/
				g_pMManager->pem_delete(height, true);
				g_pMManager->pem_delete(pint, true);
				g_pMManager->pem_delete(rcDstRect, true);
				g_pMManager->pem_delete(dROIWidth, true);
				g_pMManager->pem_delete(dROIHeight, true);
				sRstAlgo->m_dAngleX[0] = A[0];
				sRstAlgo->m_dAngleY[0] = B[0];
				sRstAlgo->m_dAngleZ[0] = C[0];
				sRstAlgo->m_dAngleX[1] = A[1];
				sRstAlgo->m_dAngleY[1] = B[1];
				sRstAlgo->m_dAngleZ[1] = C[1];

#pragma endregion Tile ROI angle

#pragma region Tile angle Average
#if 0
				RECT rcRstRect;
				RECT rcDstRect;

				if (m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoTilt->rcAngleStdROI, &rcDstRect) == FALSE)
				{
					sRstAlgo->m_dRstAngleMax = 0;
				}
				else
				{
					nLine = __LINE__;

					double dROIWidth = rcDstRect.right - rcDstRect.left;
					double dROIHeight = rcDstRect.bottom - rcDstRect.top;
					// 		double dROIX = rcDstRect.left + (dROIWidth / 2);
					// 		double dROIY = rcDstRect.top + (dROIHeight / 2);
					double dROIX = rcDstRect.left;
					double dROIY = rcDstRect.top;
					if (nAlignResCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
					{
						dROIX = dROIX + (dROIWidth / 2.0) - ((double)nImgWidth / 2.0);
						dROIY = ((double)nImgHeight / 2.0) - (dROIY + (dROIHeight / 2.0));

						double corr_x = 0, corr_y = 0;
						for (int n = 0; n < nAlignResCnt; n++)
						{
							double corr_x_Buf = 0, corr_y_Buf = 0;
							//m_proc3d.CorrectCoordinate(dFirstX, dFirstY, 0, 0, sAlignRes->theta, sAlignRes->offsetX / m_resolX, -sAlignRes->offsetY / m_resolY, &corr_x, &corr_y);
							int nCorrectCoordinate = m_proc3d.CorrectCoordinate(dROIX, dROIY, sAlignRes[n].centerX / m_resolX, sAlignRes[n].centerY / m_resolY, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
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
						dROIX = RounD(corr_x + ((double)nImgWidth / 2.0) - (dROIWidth / 2.0));
						dROIY = RounD(((double)nImgHeight / 2.0) - (corr_y + (dROIHeight / 2.0)));

						if (dROIX + RounD(dROIWidth) > nImgWidth)		// 2014/12/15	: 예외처리
							dROIX = nImgWidth - dROIWidth;
						if (dROIY + RounD(dROIHeight) > nImgHeight)
							dROIY = nImgHeight - dROIHeight;
					}

					nLine = __LINE__;

					float *pfArrImgDst = new float[(int)(dROIWidth * dROIHeight)];
					memset(pfArrImgDst, 0, dROIWidth * dROIHeight * sizeof(float));

					m_proc3d.GetCropZmap_LT(pfImgSrc, pfArrImgDst, nImgWidth, nImgHeight, dROIX, dROIY, dROIWidth, dROIHeight);
					m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("3DImage.bmp"));
					m_pProcMilAlgo->SaveWorkImg_float(pfArrImgDst, dROIWidth, dROIHeight, _T("Clip3DImage.bmp"));

					nLine = __LINE__;
					float fSelectROIMean = m_pCPInsp_Algo->GetHeightMean(pfArrImgDst, dROIWidth, dROIHeight);
					//float fSelectROIMean =  m_pCPInsp_Algo->GetHeightMaxHist(pfArrImgDst, dROIWidth, dROIHeight);
					sRstAlgo->m_dRstStdHeightMax = fSelectROIMean;


					if (pInspAlgoTilt->m_bUseAngle == true)
						m_pCPInsp_Algo->CalcTiltAngle(pfArrImgDst, dROIWidth, dROIHeight, &A[i], &B[i], &C[i]);

					if (sRstAlgo)
					{
						rcRstRect.left = dROIX;
						rcRstRect.top = dROIY;
						rcRstRect.right = dROIX + dROIWidth;
						rcRstRect.bottom = dROIY + dROIHeight;
					}

					if (pfArrImgDst)
						Delete_1DArray(&pfArrImgDst);
				}

				float stdx = (rcRstRect.right - rcRstRect.left) / 2 + rcRstRect.left;
				float stdy = (rcRstRect.bottom - rcRstRect.top) / 2 + rcRstRect.top;
				double stdz = sRstAlgo->m_dRstStdHeightMax;

				double dAngleAvg(0);

				for (int i = 0; i < pInspAlgoTilt->nSelectValue; i++)
				{
					float x = (sRstAlgo->m_rcRect_I[i].right - sRstAlgo->m_rcRect_I[i].left) / 2 + sRstAlgo->m_rcRect_I[i].left;
					float y = (sRstAlgo->m_rcRect_I[i].bottom - sRstAlgo->m_rcRect_I[i].top) / 2 + sRstAlgo->m_rcRect_I[i].top;

					double dx = (stdx - x)* m_resolX * 1000;// *1000=> 'mm->um'
					double dy = (stdy - y)* m_resolY * 1000;// *1000=> 'mm->um'

					distance[i] = sqrt(pow(dx, 2) + pow(dy, 2));

					double radiantmp = atan((stdz - sRstAlgo->m_dArrRectHeight[i]) / distance[i]);
					sRstAlgo->m_dArrAngle[i] = radiantmp * 180 / PI;
					dAngleAvg += abs(sRstAlgo->m_dArrAngle[i]);

					double currentAngle = abs(pInspAlgoTilt->m_dZAngle - sRstAlgo->m_dArrAngle[i]);
					if (currentAngle > pInspAlgoTilt->m_dAngleMaxDiff)
					{
						sRstAlgo->m_bArrAngleOK[i] = FALSE;
					}
					else
					{
						sRstAlgo->m_bArrAngleOK[i] = TRUE;
					}
				}

				dAngleAvg = dAngleAvg / (pInspAlgoTilt->nSelectValue);	//shkim 2018.02.13 컨셉변경
				sRstAlgo->m_dRstAngleMax = dAngleAvg;

				double currentAngle = abs(pInspAlgoTilt->m_dZAngle - dAngleAvg);

				if (currentAngle > pInspAlgoTilt->m_dAngleMaxDiff)
				{
					sRstAlgo->m_bAngle = FALSE;
				}
				else
				{
					sRstAlgo->m_bAngle = TRUE;
				}
				delete[] rcDstRect;
#endif
#pragma endregion Tile angle Average
				// 				float A(0.0),B(0.0),C(0.0);
				// 				m_pCPInsp_Algo->CalcTiltAngle(pfArrImgClip, nClipSize, nClipSize,&A,&B,&C);
			}
			else
			{
				for (int n = 0; n < pInspAlgoTilt->nSelectValue; n++)
					sRstAlgo->m_bArrAngleOK[n] = TRUE;
			}
			bReturn = (sRstAlgo->m_bOKArea == TRUE && sRstAlgo->m_bOKAvg == TRUE && sRstAlgo->m_bAngle == TRUE) ? TRUE : FALSE;
			/*delete [] A;
			delete [] B;
			delete [] C;*/
			g_pMManager->pem_delete(A, true);
			g_pMManager->pem_delete(B, true);
			g_pMManager->pem_delete(C, true);
		}

	}
	catch (...)
	{

		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Algo::InspTilt(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return bReturn;
}

BOOL CPInsp_AlgoTilt::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoTilt::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoTilt::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}