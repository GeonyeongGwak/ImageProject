#include "PInsp_AlgoHeightDiff.h"


CPInsp_AlgoHeightDiff::CPInsp_AlgoHeightDiff(void)
{
}


CPInsp_AlgoHeightDiff::~CPInsp_AlgoHeightDiff(void)
{
}

void CPInsp_AlgoHeightDiff::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoHeightDiff::GetInspAlgoData()
{
	return eSPCAlgoHeight_Diff;
}

int CPInsp_AlgoHeightDiff::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoHeightDiff * rstAlgo = (RstAlgoHeightDiff *)vRstInspAlgo;
	if (nWndType == eINSP_SOLDER)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}
	else
	{
		if (rstAlgo->m_bCheckPolarity)
		{
			eWholeNgTypeTemp[TypeMountPolarity] = e_NG;
			nCurrentNgType = TypeMountPolarity;
		}
		else
		{
			eWholeNgTypeTemp[TypeMountWorng] = e_NG;
			nCurrentNgType = TypeMountWorng;
		}
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoHeightDiff::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoHeightDiff::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspHeightDiff(sInspAlgo, sWndAlgoImg, *sInspImageData, (RstAlgoHeightDiff *)sRstAlgo, stAlgoParam.m_sAlignRes, stAlgoParam.m_nAlignCnt, ucArrDstImg);

	return bResult;
}

BOOL CPInsp_AlgoHeightDiff::InspHeightDiff(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImgBuf, RstAlgoHeightDiff *sRstAlgo, AlignResult * sAlignRes, int nAlignResCnt, UCHAR* puImgDst)
{
	BOOL bReturn = FALSE;
	int nLine = __LINE__;	
	InspPartInfo *pInspPartInfo = nullptr;
	POINTF *pPoHiddenAreaOfPartImgPx = nullptr;
	BOOL bUseHiddenArea = FALSE;
	try
	{
		if (sRstAlgo)
		{
			memset(sRstAlgo, 0, sizeof(RstAlgoHeightDiff));
			sRstAlgo->Init();
			for (int a = 0; a < 4; a++)
			{
				sRstAlgo->m_rcRect_I[a].left = 0;
				sRstAlgo->m_rcRect_I[a].right = 0;
				sRstAlgo->m_rcRect_I[a].top = 0;
				sRstAlgo->m_rcRect_I[a].bottom = 0;
			}
		}

		if (sInspAlgo.m_eAlgoType != eAlgoHeight_Diff)
			return bReturn;
		if (!m_pProcMilAlgo)
			return bReturn;

		AlgoHeightDiff *pInspAlgoHeightDiff = (AlgoHeightDiff *)sInspAlgo.m_ptrInspAlgoParam;
		if (!pInspAlgoHeightDiff)
			return bReturn;

		UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
		UCHAR *pucImgSrc_ROI2 = sWndAlgoImg.m_ucArr2D_Mix[0] != NULL ? sWndAlgoImg.m_ucArr2D_Mix[0] : sWndAlgoImg.m_ucArr2D;
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if ((pucImgSrc == NULL) && (pfImgSrc == NULL) && (nImgWidth <= 0) && (nImgHeight <= 0))
			return bReturn;

		bool bUseBW = ((pInspAlgoHeightDiff->m_sBlobBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse);
		//bUseBW = true;		//무조건 참으로 임시 처리_테스트를 위해

		bool bUseBW_ROI2 = false;
		if (pInspAlgoHeightDiff->m_bUseBW_ROI2 && bUseBW)
		{
			bUseBW_ROI2 = ((pInspAlgoHeightDiff->m_sBlobBase_ROI2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse);
		}
#if _DEBUG
		cv::Mat src_ROI1 = cv::Mat(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_MAKETYPE(CV_8U, 1), pucImgSrc);
		cv::Mat src_ROI2 = cv::Mat(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_MAKETYPE(CV_8U, 1), pucImgSrc_ROI2);
		cv::Mat src_3D = cv::Mat(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_32FC1, pfImgSrc);
#endif // _DEBUG

		if (puImgDst)
		{
			if (bUseBW)
				memset(puImgDst, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
			else
			{
				IppiSize iSize = { nImgWidth, nImgHeight };
				IppStatus sts = ippiCopy_8u_C1R(pucImgSrc, nImgWidth, puImgDst, nImgWidth, iSize);
				m_pProcMilAlgo->SaveWorkImg(puImgDst, nImgWidth, nImgHeight, _T("Origin2D.bmp"));
			}
		}

		cv::Mat imgColor(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(255));
		if (bUseBW)
		{
			CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
			if (!bUseBW_ROI2)
			{
				if (pColorTeach != NULL && pInspAlgoHeightDiff->m_sBlobBase.m_sAlgoColorBase.m_bUseColor == TRUE)
				{
					bool bTeach = (puImgDst != NULL);
					int nSideCameraIndex = sWndAlgoImg.m_nLight_index;
					pColorTeach->GetColorBaseBin(&pInspAlgoHeightDiff->m_sBlobBase.m_sAlgoColorBase, sInspImgBuf, imgColor.data, bTeach, 0, 0, -1, nSideCameraIndex);
				}
			}
			else
			{
				if (pColorTeach != NULL && pInspAlgoHeightDiff->m_sBlobBase_ROI2.m_sAlgoColorBase.m_bUseColor == TRUE)
				{
					bool bTeach = (puImgDst != NULL);
					int nSideCameraIndex = sWndAlgoImg.m_nLight_index;
					pColorTeach->GetColorBaseBin(&pInspAlgoHeightDiff->m_sBlobBase_ROI2.m_sAlgoColorBase, sInspImgBuf, imgColor.data, bTeach, 0, 0, -1, nSideCameraIndex);
				}
			}
		}

		RECT rcTeach[2];
		if (m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoHeightDiff->rcFirstROI, &rcTeach[0]) == FALSE ||
			m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoHeightDiff->rcSeccondROI, &rcTeach[1]) == FALSE)
			return bReturn;

		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("3DImage_1.bmp"));
		int nMinMaxflag = pInspAlgoHeightDiff->m_nMinMaxflag;
		int nArrL[2], nArrT[2], nArrW[2], nArrH[2], nArrType[2], nArrRstX[2], nArrRstY[2];
		float fArrRst[2], fArrMean[2], fArrMin[2], fArrMax[2];
		int nArrMinX[2], nArrMinY[2], nArrMaxX[2], nArrMaxY[2];

		//HiddenArea
		cv::Mat imgBinHidden(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(255));
		cv::Mat imgBlobHidden;
		pInspPartInfo = g_pInspMng->GetInspPartInfo();
		if (pInspPartInfo != nullptr)
		{
			if (pInspPartInfo->bStickerInsp == TRUE)
			{
				pPoHiddenAreaOfPartImgPx = pInspPartInfo->pPoHiddenArea;
				bUseHiddenArea = TRUE;
				if (pPoHiddenAreaOfPartImgPx != nullptr)
				{
					for (int nStcIdx = 0; nStcIdx < StickerCnt; nStcIdx++)
						g_pInspMng->m_FR.FillPolygon(imgBinHidden, pPoHiddenAreaOfPartImgPx + (nStcIdx * StickerPo), StickerPo, cv::Scalar(0, 0, 0));
				}
			}
		}

		//2개의 ROI에서  Blob 반복 수행
		for (int nIdx = 0; nIdx < 2; nIdx++)
		{
			if (nIdx == 0)			//첫번 째 ROI 
				nArrType[nIdx] = nMinMaxflag / 3;
			else if (nIdx == 1)		//두번 째 ROI
				nArrType[nIdx] = nMinMaxflag % 3;

			nArrL[nIdx] = rcTeach[nIdx].left;
			nArrT[nIdx] = rcTeach[nIdx].top;
			nArrW[nIdx] = rcTeach[nIdx].right - nArrL[nIdx];
			nArrH[nIdx] = rcTeach[nIdx].bottom - nArrT[nIdx];

			imgBlobHidden = cv::Mat(nArrH[nIdx], nArrW[nIdx], CV_8UC1, cv::Scalar(0));
			m_pProcMilAlgo->GetClipImage_LT(imgBinHidden.data, nImgWidth, nImgHeight, imgBlobHidden.data, nArrL[nIdx], nArrT[nIdx], nArrW[nIdx], nArrH[nIdx]);

			if (nAlignResCnt > 0 && sAlignRes != NULL && sAlignRes->nWindowID >= 0)
			{
				double dL = (double)nArrL[nIdx] + ((double)nArrW[nIdx] / 2.0) - ((double)nImgWidth / 2.0);
				double dT = ((double)nImgHeight / 2.0) - ((double)nArrT[nIdx] + ((double)nArrH[nIdx] / 2.0));

				double corr_x = 0, corr_y = 0;
				for (int n = 0; n < nAlignResCnt; n++)
				{
					double corr_x_Buf = 0, corr_y_Buf = 0;
					int nCorrectCoordinate = m_proc3d.CorrectCoordinate(dL, dT, sAlignRes[n].centerX / m_resolX, sAlignRes[n].centerY / m_resolY, sAlignRes[n].theta, sAlignRes[n].offsetX / m_resolX, sAlignRes[n].offsetY / m_resolY, &corr_x_Buf, &corr_y_Buf);
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

				double dRstX = RounD(corr_x + ((double)nImgWidth / 2.0) - ((double)nArrW[nIdx] / 2.0));
				double dRstY = RounD(((double)nImgHeight / 2.0) - (corr_y + ((double)nArrH[nIdx] / 2.0)));

				if (dRstX + nArrW[nIdx] > nImgWidth)		// 2014/12/15	: 예외처리
					dRstX = nImgWidth - nArrW[nIdx];
				if (dRstY + nArrH[nIdx] > nImgHeight)
					dRstY = nImgHeight - nArrH[nIdx];

				nArrL[nIdx] = dRstX;
				nArrT[nIdx] = dRstY;
			}

			//ROI의 중심 위치 계산
			nArrRstX[nIdx] = nArrL[nIdx] + (nArrW[nIdx] / 2);
			nArrRstY[nIdx] = nArrT[nIdx] + (nArrH[nIdx] / 2);

			//초기화 및 검사ROI 영역 저장
			fArrRst[nIdx] = 0;
			fArrMean[nIdx] = 0;
			fArrMin[nIdx] = 0;
			fArrMax[nIdx] = 0;
			if (sRstAlgo)
			{
				sRstAlgo->m_rcRect_I[nIdx].left = nArrL[nIdx];
				sRstAlgo->m_rcRect_I[nIdx].right = nArrL[nIdx] + nArrW[nIdx];
				sRstAlgo->m_rcRect_I[nIdx].top = nArrT[nIdx];
				sRstAlgo->m_rcRect_I[nIdx].bottom = nArrT[nIdx] + nArrH[nIdx];
			}

			//BLob 수행구문
			if (bUseBW)
			{
				if (nIdx == 1)		//두번 째 ROI
				{
					if (nArrL[nIdx] == nArrL[0] && nArrT[nIdx] == nArrT[0] &&
						nArrW[nIdx] == nArrW[0] && nArrH[nIdx] == nArrH[0])
					{
						if (nArrType[nIdx] == 1)
						{
							fArrRst[nIdx] = fArrMin[0];
							nArrRstX[nIdx] = nArrMinX[0];
							nArrRstY[nIdx] = nArrMinY[0];
						}
						else if (nArrType[nIdx] == 2)
						{
							fArrRst[nIdx] = fArrMax[0];
							nArrRstX[nIdx] = nArrMaxX[0];
							nArrRstY[nIdx] = nArrMaxY[0];
						}
						else
							fArrRst[nIdx] = fArrMean[0];
						break;
					}
				}

				//실제 Blob
				cv::Mat imgBlob(nArrH[nIdx], nArrW[nIdx], CV_8UC1, cv::Scalar(0));
				cv::Mat image3D(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);

				if (bUseBW_ROI2 && nIdx == 1)		//두번 째 ROI BW 설정값으로 BLob 검사
				{
					GetBlobImg(sInspAlgo, imgBlob, nArrL[nIdx], nArrT[nIdx], pfImgSrc, pucImgSrc_ROI2, imgColor, nImgWidth, nImgHeight, pInspAlgoHeightDiff->m_sBlobBase_ROI2);
				}
				else
				{
					//Blob 검사 관련 변수, 검사영역, 결과영상 세팅
					GetBlobImg(sInspAlgo, imgBlob, nArrL[nIdx], nArrT[nIdx], pfImgSrc, pucImgSrc, imgColor, nImgWidth, nImgHeight, pInspAlgoHeightDiff->m_sBlobBase);
				}

				//HiddenArea
				if (bUseHiddenArea == TRUE)
					cv::bitwise_and(imgBlob, imgBlobHidden, imgBlob);

				if (pInspAlgoHeightDiff->m_bUseErodeFilter)
				{
					int nMaskSize = pInspAlgoHeightDiff->m_nErodeFilter;
					if(nMaskSize > 0)
					cv::erode(imgBlob, imgBlob, cv::Mat(nMaskSize, nMaskSize, CV_8UC1, cv::Scalar(1)));
					if ((pInspAlgoHeightDiff->m_sBlobBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eMaxBlob) == m_enBlobBase_Data_eMaxBlob)
						m_pProcMilAlgo->CalcBlob_Select(imgBlob.data, imgBlob.data, imgBlob.cols, imgBlob.rows, 4, FALSE, FALSE, 0, eSelectBigger);
					nLine = __LINE__;
					//if (g_pMPTI->IsMachineTypeMars()) //멀티경우는 Mars 만 사용하므로 막지 않음.
					if (g_pMPTI->IsMachineTypeMars()) // Mars 경우에만 Separate Rect 기능 사용 
					{
					BlobSeparateRect(imgBlob, image3D, fArrMax, nIdx, nArrMaxX, nArrMaxY, nArrType, fArrRst, nArrRstX, nArrRstY, fArrMin, nArrMinX, nArrMinY, nArrT, nArrL);

					if (puImgDst != NULL)
					{
						cv::Mat dstImage(nImgHeight, nImgWidth, CV_8UC1, puImgDst);
						ippiCopy_8u_C1R(imgBlob.data, imgBlob.cols, puImgDst + (nArrT[nIdx] * nImgWidth + nArrL[nIdx]), nImgWidth, { imgBlob.cols, imgBlob.rows });
					}
					if (nArrType[nIdx] == 1 || nArrType[nIdx] ==2 )  // Erode Filter 사용 + Max Min 일때만 데이터 사용 Mean 은 기존 방식 그대로 
					continue;
					}
				}

				//Morph erode	

				nLine = __LINE__;
				int nCnt = 0;
				float fAdd = 0;
				float fMax = -HUGE_VAL;
				float fMin = HUGE_VAL;
				for (int nY = 0; nY < nArrH[nIdx]; nY++)
				{
					int y = nArrT[nIdx] + nY;
					if (y < 0 || y >= nImgHeight)
						continue;
					for (int nX = 0; nX < nArrW[nIdx]; nX++)
					{
						int x = nArrL[nIdx] + nX;
						if (x < 0 || x >= nImgWidth)
							continue;

						int nIndex = (y * nImgWidth) + x;
						int nIndexB = (nY * nArrW[nIdx]) + nX;

						//if (puImgDst)
						//	puImgDst[nIndex] = imgBlob.data[nIndexB];
						if (puImgDst)
						{
							if (pInspAlgoHeightDiff->m_nSelectRoi != -1)
							{
								if (pInspAlgoHeightDiff->m_bUseBW_ROI2)
								{
									if (nIdx == pInspAlgoHeightDiff->m_nSelectRoi)
										puImgDst[nIndex] = imgBlob.data[nIndexB];
								}
								else
									puImgDst[nIndex] = imgBlob.data[nIndexB];
							}
						}
						if (imgBlob.data[nIndexB] != 255)
							continue;

						if (fMax < pfImgSrc[nIndex])
						{
							fMax = pfImgSrc[nIndex];
							fArrMax[nIdx] = fMax;
							nArrMaxX[nIdx] = x;
							nArrMaxY[nIdx] = y;
							if (nArrType[nIdx] == 2)
							{
								fArrRst[nIdx] = fMax;
								nArrRstX[nIdx] = x;
								nArrRstY[nIdx] = y;
							}
						}
						if (fMin > pfImgSrc[nIndex])
						{
							fMin = pfImgSrc[nIndex];
							fArrMin[nIdx] = fMin;
							nArrMinX[nIdx] = x;
							nArrMinY[nIdx] = y;
							if (nArrType[nIdx] == 1)
							{
								fArrRst[nIdx] = fMin;
								nArrRstX[nIdx] = x;
								nArrRstY[nIdx] = y;
							}
						}
						fAdd += pfImgSrc[nIndex];
						nCnt++;
						fArrMean[nIdx] = (fAdd / (float)nCnt);
						if (nArrType[nIdx] != 1 && nArrType[nIdx] != 2)
							fArrRst[nIdx] = (fAdd / (float)nCnt);
					}
				}
			}
			else
			{
				cv::Mat imgBlob(nArrH[nIdx], nArrW[nIdx], CV_8UC1, cv::Scalar(0));

				TotalInspExceptArea stTieArea;
				stTieArea.m_nUsedMaskingValue = 0;
				stTieArea.m_nUsedWndPolygon = 0;

				stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
				stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
				for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
					stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];

				cv::Mat imgBin(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(255));
				m_pCPInsp_Algo->FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, imgBin.data, NULL, stTieArea);
				m_pProcMilAlgo->GetClipImage_LT(imgBin.data, nImgWidth, nImgHeight, imgBlob.data, nArrL[nIdx], nArrT[nIdx], nArrW[nIdx], nArrH[nIdx]);
				//HiddenArea
				if (bUseHiddenArea == TRUE)
					cv::bitwise_and(imgBlob, imgBlobHidden, imgBlob);

				int nCnt = 0;
				float fAdd = 0;
				float fMax = -HUGE_VAL;
				float fMin = HUGE_VAL;
				for (int nY = 0; nY < nArrH[nIdx]; nY++)
				{
					int y = nArrT[nIdx] + nY;
					if (y < 0 || y >= nImgHeight)
						continue;
					for (int nX = 0; nX < nArrW[nIdx]; nX++)
					{
						int x = nArrL[nIdx] + nX;
						if (x < 0 || x >= nImgWidth)
							continue;
						int nIndex = (y * nImgWidth) + x;
						int nIndexB = (nY * nArrW[nIdx]) + nX;
						if (puImgDst)
							puImgDst[nIndex] = imgBlob.data[nIndexB];
						if (imgBlob.data[nIndexB] != 255)
							continue;
						if (fMax < pfImgSrc[nIndex])
						{
							fMax = pfImgSrc[nIndex];
							fArrMax[nIdx] = fMax;
							nArrMaxX[nIdx] = x;
							nArrMaxY[nIdx] = y;
							if (nArrType[nIdx] == 2)
							{
								fArrRst[nIdx] = fMax;
								nArrRstX[nIdx] = x;
								nArrRstY[nIdx] = y;
							}
						}
						if (fMin > pfImgSrc[nIndex])
						{
							fMin = pfImgSrc[nIndex];
							fArrMin[nIdx] = fMin;
							nArrMinX[nIdx] = x;
							nArrMinY[nIdx] = y;
							if (nArrType[nIdx] == 1)
							{
								fArrRst[nIdx] = fMin;
								nArrRstX[nIdx] = x;
								nArrRstY[nIdx] = y;
							}
						}
						fAdd += pfImgSrc[nIndex];
						nCnt++;
						fArrMean[nIdx] = (fAdd / (float)nCnt);
						if (nArrType[nIdx] != 1 && nArrType[nIdx] != 2)
							fArrRst[nIdx] = (fAdd / (float)nCnt);
					}
				}
			}
		}

		bReturn = TRUE;
		if (sRstAlgo)
		{
			sRstAlgo->m_bCheckPolarity = pInspAlgoHeightDiff->m_bPolarity;
			sRstAlgo->m_dRst_1 = fArrRst[0];
			sRstAlgo->m_dRst_2 = fArrRst[1];
			sRstAlgo->m_dRstHeightDiff = (sRstAlgo->m_dRst_1 - sRstAlgo->m_dRst_2) + pInspAlgoHeightDiff->m_fAddHeight;

			if (pInspAlgoHeightDiff->dHeightDiff3D != 0 && pInspAlgoHeightDiff->m_bSignInversion == TRUE)
			{
				BOOL bPlusRst = sRstAlgo->m_dRstHeightDiff >= 0;
				BOOL bPlusParam = pInspAlgoHeightDiff->dHeightDiff3D >= 0;
				if (bPlusRst != bPlusParam)		// 차이값의 부호가 다르면 NG 로 처리
					bReturn = FALSE;
			}
			if (pInspAlgoHeightDiff->m_bDiffUpper && bReturn)
			{
				bReturn = sRstAlgo->m_dRstHeightDiff < pInspAlgoHeightDiff->dHeightDiff3DMax;
			}
			if (pInspAlgoHeightDiff->m_bDiffLower && bReturn)
			{
				bReturn = sRstAlgo->m_dRstHeightDiff > pInspAlgoHeightDiff->dHeightDiff3D;
			}

			//if ((nArrType[0] == 1 && nArrType[1] == 2) || (nArrType[0] == 2 && nArrType[1] == 1))
			//{
			//	double dMaxW = (nArrW[0] > nArrW[1]) ? nArrW[0] : nArrW[1];
			//	double dMaxH = (nArrH[0] > nArrH[1]) ? nArrH[0] : nArrH[1];
			//	double dMin = (dMaxW > dMaxH) ? dMaxH : dMaxW;
			//	int nMax = (dMin / 2 < 7) ? dMin / 2 : 7;

			//	double dFL = (nArrType[0] == 2) ? nArrRstX[0] : nArrRstX[1];
			//	double dFT = (nArrType[0] == 2) ? nArrRstY[0] : nArrRstY[1];
			//	double dEL = (nArrType[0] == 2) ? nArrRstX[1] : nArrRstX[0];
			//	double dET = (nArrType[0] == 2) ? nArrRstY[1] : nArrRstY[0];
			//	sRstAlgo->m_rcRect_I[2].left = dFL - nMax;
			//	sRstAlgo->m_rcRect_I[2].right = dFL + nMax;
			//	sRstAlgo->m_rcRect_I[2].top = dFT - nMax;
			//	sRstAlgo->m_rcRect_I[2].bottom = dFT + nMax;

			//	sRstAlgo->m_rcRect_I[3].left = dEL - nMax;
			//	sRstAlgo->m_rcRect_I[3].right = dEL + nMax;
			//	sRstAlgo->m_rcRect_I[3].top = dET - nMax;
			//	sRstAlgo->m_rcRect_I[3].bottom = dET + nMax;
			//}
			double dMaxW = (nArrW[0] > nArrW[1]) ? nArrW[0] : nArrW[1];
			double dMaxH = (nArrH[0] > nArrH[1]) ? nArrH[0] : nArrH[1];
			double dMin = (dMaxW > dMaxH) ? dMaxH : dMaxW;
			int nMax = (dMin / 2 < 7) ? dMin / 2 : 7;
			const int TYPE_MAX = 2;
			const int TYPE_MIN = 1;
			for (int idx = 0; idx < 2; idx++) {
				if ((nArrType[idx] == TYPE_MIN || nArrType[idx] == TYPE_MAX))
				{
					sRstAlgo->m_rcRect_I[2 + idx].left = nArrRstX[idx] - nMax;
					sRstAlgo->m_rcRect_I[2 + idx].right = nArrRstX[idx] + nMax;
					sRstAlgo->m_rcRect_I[2 + idx].top = nArrRstY[idx] - nMax;
					sRstAlgo->m_rcRect_I[2 + idx].bottom = nArrRstY[idx] + nMax;
				}
			}

			sRstAlgo->m_bOK = bReturn;
#if 0
			//old ver
			std::vector<float> vFirst;
			std::vector<float> vSecond;
			for (int i = 0; i < dFirstWidth*dFirstHeight; i++)
				vFirst.emplace_back(pfArrFirstRoiImg[i]);

			for (int i = 0; i < dSeccondWidth*dSeccondHeight; i++)
				vSecond.emplace_back(pfArrSeccondRoiImg[i]);

			if (pInspAlgoHeightDiff->dHeightDiff3D != 0 && pInspAlgoHeightDiff->m_bSignInversion == TRUE)
			{
				BOOL bPlusRst = sRstAlgo->m_dRstHeightDiff >= 0;
				BOOL bPlusParam = pInspAlgoHeightDiff->dHeightDiff3D >= 0;
				if (bPlusRst != bPlusParam)		// 차이값의 부호가 다르면 NG 로 처리
					bReturn = FALSE;
			}
			if (pInspAlgoHeightDiff->m_bDiffUpper && bReturn)
			{
				bReturn = sRstAlgo->m_dRstHeightDiff < pInspAlgoHeightDiff->dHeightDiff3DMax;

				if (bReturn != TRUE)
				{
					double dMaxFirst = *max_element(vFirst.begin(), vFirst.end());
					double dMaxSecond = *max_element(vSecond.begin(), vSecond.end());

					int max_index = 0, nOffsetX = 0, nOffsetY = 0, nWidth;

					if (dMaxFirst >= dMaxSecond)
					{
						max_index = std::max_element(vFirst.begin(), vFirst.end()) - vFirst.begin();
						nOffsetX = dFirstX;
						nOffsetY = dFirstY;
						nWidth = (int)dFirstWidth;
					}
					else
					{
						max_index = std::max_element(vSecond.begin(), vSecond.end()) - vSecond.begin();
						nOffsetX = dSeccondX;
						nOffsetY = dSeccondY;
						nWidth = (int)dSeccondWidth;
					}

					double dX_Temp = max_index % nWidth;
					double dY_Temp = max_index / nWidth;

					sRstAlgo->m_rcRect_I[nCnt].left = nOffsetX + dX_Temp;
					sRstAlgo->m_rcRect_I[nCnt].right = nOffsetX + dX_Temp + 1;
					sRstAlgo->m_rcRect_I[nCnt].top = nOffsetY + dY_Temp;
					sRstAlgo->m_rcRect_I[nCnt].bottom = nOffsetY + dY_Temp + 1;
					nCnt += 1;
				}
			}

			if (pInspAlgoHeightDiff->m_bDiffLower && bReturn)
			{
				bReturn = sRstAlgo->m_dRstHeightDiff > pInspAlgoHeightDiff->dHeightDiff3D;

				if (bReturn != TRUE)
				{
					double dMinFirst = *min_element(vFirst.begin(), vFirst.end());
					double dMinSecond = *min_element(vSecond.begin(), vSecond.end());

					int max_index = 0, nOffsetX = 0, nOffsetY = 0, nWidth;

					if (dMinFirst >= dMinSecond)
					{
						max_index = std::min_element(vFirst.begin(), vFirst.end()) - vFirst.begin();
						nOffsetX = dFirstX;
						nOffsetY = dFirstY;
						nWidth = (int)dFirstWidth;
					}
					else
					{
						max_index = std::min_element(vSecond.begin(), vSecond.end()) - vSecond.begin();
						nOffsetX = dSeccondX;
						nOffsetY = dSeccondY;
						nWidth = (int)dSeccondWidth;
					}

					double dX_Temp = max_index % nWidth;
					double dY_Temp = max_index / nWidth;
					sRstAlgo->m_rcRect_I[nCnt].left = nOffsetX + dX_Temp;
					sRstAlgo->m_rcRect_I[nCnt].right = nOffsetX + dX_Temp + 1;
					sRstAlgo->m_rcRect_I[nCnt].top = nOffsetY + dY_Temp;
					sRstAlgo->m_rcRect_I[nCnt].bottom = nOffsetY + dY_Temp + 1;
					nCnt += 1;
				}

			}

#endif
						}
					}

	catch (...)
	{
		CString msg;
		msg.Format(_T("PInsp_AlgoHeightDiff : %d"), nLine);
		g_pMPTI->AddLog(msg);
	}

	return bReturn;
}

BOOL CPInsp_AlgoHeightDiff::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoHeightDiff::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoHeightDiff * pInspAlgo = (AlgoHeightDiff *)sInspAlgo.m_ptrInspAlgoParam;
	if ((pInspAlgo->m_sBlobBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse)
	{
		nData = pInspAlgo->m_sBlobBase.m_sAlgoColorBase.GetColorData();
		nData |= COLOR_DATA_WND;
	}
	if ((pInspAlgo->m_sBlobBase_ROI2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eUse) == m_enBlobBase_Data_eUse)
	{
		nData = pInspAlgo->m_sBlobBase_ROI2.m_sAlgoColorBase.GetColorData();
		nData |= COLOR_DATA_WND;
	}

	return nData;
}
bool CPInsp_AlgoHeightDiff::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}
void CPInsp_AlgoHeightDiff::GetBlobImg(const InspAlgo &sInspAlgo, cv::Mat imgBlob, int nL, int nT, float* pfImgSrc, UCHAR *pucImgSrc, cv::Mat imgColor, int nImgWidth, int nImgHeight, tagAlgoBlobBase sBlobBase)
{
	int nW = imgBlob.cols;
	int nH = imgBlob.rows;
	cv::Mat img2D;
	cv::Mat img3D;
	cv::Mat imgCo;
	cv::Mat imgRst(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));

	bool b2D = ((sBlobBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2D) == m_enBlobBase_Data_e2D);
	bool b3D = ((sBlobBase.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3D) == m_enBlobBase_Data_e3D);

	if (b3D)
	{
		img3D = cv::Mat(nImgHeight, nImgWidth, CV_32FC1, cv::Scalar(0));
		m_proc3d.GetCropZmap_LT(pfImgSrc, img3D.ptr<float>(), nImgWidth, nImgHeight, 0, 0, nImgWidth, nImgHeight);
	}

	if (b2D)
	{
		img2D = cv::Mat(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, img2D.data, 0, 0, nImgWidth, nImgHeight);
	}

	if (sBlobBase.m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		imgCo = cv::Mat(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		m_pProcMilAlgo->GetClipImage_LT(imgColor.data, nImgWidth, nImgHeight, imgCo.data, 0, 0, nImgWidth, nImgHeight);
	}

	TotalInspExceptArea stTieArea;
	stTieArea.m_nUsedMaskingValue = 0;
	stTieArea.m_nUsedWndPolygon = 0;
	stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
	stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
	for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];

	m_pCPInsp_Algo->BlobImageStruct_Base(sBlobBase, img2D, img3D, imgCo, nImgWidth, nImgHeight, stTieArea, imgRst, true);
	m_pProcMilAlgo->GetClipImage_LT(imgRst.data, nImgWidth, nImgHeight, imgBlob.data, nL, nT, nW, nH);
	img2D.release();
	img3D.release();
	imgCo.release();
	imgRst.release();
}

void CPInsp_AlgoHeightDiff::BlobSeparateRect(cv::Mat imgBlob, cv::Mat image3D, float * fArrMax, int nIdx, int * nArrMaxX, int * nArrMaxY, int * nArrType, float * fArrRst, int * nArrRstX, int * nArrRstY, float * fArrMin, int * nArrMinX, int * nArrMinY, int * nArrT , int * nArrL)
{
	if (nArrType[nIdx] == 0) // Mean 값은 계산되지않음. 
		return;
#if _DEBUG
	cv::Mat OutPut3Ch[] = { imgBlob ,imgBlob ,imgBlob };
	cv::Mat imgBlobDebug;
	cv::merge(OutPut3Ch, 3, imgBlobDebug);
	float fMaxData = -HUGE_VAL;
	float fMinData = HUGE_VAL;
#endif
	std::vector < std::vector < cv::Point>> contours;
	cv::findContours(imgBlob, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	// 		cv::drawContours(imgBlobDebug, contour, 0, cv::Scalar(255, 0, 255));
	// 		cv::Rect rect = cv::boundingRect(contour[0]);
	// 		cv::rectangle(imgBlobDebug, rect, cv::Scalar(255, 0, 0));

			// 2. contour의 최소 사각형 구하기
	std::vector<cv::Rect> bounding_rects(contours.size());
	for (size_t i = 0; i < contours.size(); i++) {
		bounding_rects[i] = cv::boundingRect(contours[i]);
	}

	// 3. 최소 사각형 크기를 나누기 위한 상수 정의
	const int CELL_SIZE = 5;

	// 4. 최소 사각형을 일정한 크기의 작은 사각형으로 나누기
	std::vector<cv::Rect> divided_rects;
	for (const cv::Rect& bounding_rect : bounding_rects) {
		int num_cols = bounding_rect.width / CELL_SIZE;
		int num_rows = bounding_rect.height / CELL_SIZE;

		for (int row = 0; row < num_rows; row++) {
			for (int col = 0; col < num_cols; col++) {
				cv::Rect cell_rect = bounding_rect;
				cell_rect.x += col * CELL_SIZE;
				cell_rect.y += row * CELL_SIZE;
				cell_rect.width = CELL_SIZE;
				cell_rect.height = CELL_SIZE;

				divided_rects.push_back(cell_rect);
			}
		}
	}

	// 5. 이진화 이미지에 포함된 영역이 있는 사각형들만 추려내기
	double avg_val;
	std::vector<double> vAvgValue; 
	std::vector <cv::Point > vPtCenter;
	std::vector<cv::Rect> binary_rects;
	for (const cv::Rect& divided_rect : divided_rects) {
		cv::Mat cell_roi = imgBlob(divided_rect);
		if (cv::countNonZero(cell_roi) > 12) 
		{
			cv::Mat mask = cell_roi / 255; // 5x5 Mask
			cv::Rect rc3DTemp (divided_rect.x + nArrL[nIdx], divided_rect.y + nArrT[nIdx], 5, 5) ;
			vAvgValue.emplace_back(cv::mean(image3D(rc3DTemp), mask)[0]);
			vPtCenter.emplace_back(cv::Point (rc3DTemp.x + 2, rc3DTemp.y + 2));
			binary_rects.push_back(divided_rect);
		}
	}
#if _DEBUG
	for (int i = 0; i < binary_rects.size(); i++)
		cv::rectangle(imgBlobDebug, binary_rects[i], cv::Scalar(255, 0, 0));
#endif

	if (vAvgValue.size() != 0)
	{
		int nMaxBlobIndex = max_element(vAvgValue.begin(), vAvgValue.end()) - vAvgValue.begin();
		fArrMax[nIdx] = vAvgValue[nMaxBlobIndex];
		nArrMaxX[nIdx] = vPtCenter[nMaxBlobIndex].x;
		nArrMaxY[nIdx] = vPtCenter[nMaxBlobIndex].y;

		int nMinBlobIndex = min_element(vAvgValue.begin(), vAvgValue.end()) - vAvgValue.begin();
		fArrMin[nIdx] = vAvgValue[nMinBlobIndex];
		nArrMinX[nIdx] = vPtCenter[nMinBlobIndex].x;
		nArrMinY[nIdx] = vPtCenter[nMinBlobIndex].y;


		if (nArrType[nIdx] == 2)
		{
			fArrRst[nIdx] = fArrMax[nIdx];
			nArrRstX[nIdx] = nArrMaxX[nIdx];
			nArrRstY[nIdx] = nArrMaxY[nIdx];
		}
		if (nArrType[nIdx] == 1)
		{
			fArrRst[nIdx] = fArrMin[nIdx];
			nArrRstX[nIdx] = nArrMinX[nIdx];
			nArrRstY[nIdx] = nArrMinY[nIdx];
		}
#if _DEBUG
		cv::rectangle(imgBlobDebug, binary_rects[nMaxBlobIndex], cv::Scalar(0, 0, 255),3);
		cv::rectangle(imgBlobDebug, binary_rects[nMinBlobIndex], cv::Scalar(0, 255, 255),3);
#endif
	}
	

}
