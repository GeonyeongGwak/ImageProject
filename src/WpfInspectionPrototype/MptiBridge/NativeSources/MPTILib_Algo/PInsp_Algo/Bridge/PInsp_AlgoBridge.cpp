#include "PInsp_AlgoBridge.h"

CPInsp_AlgoBridge::CPInsp_AlgoBridge(void)
{
}


CPInsp_AlgoBridge::~CPInsp_AlgoBridge(void)
{
}

void CPInsp_AlgoBridge::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoBridge::GetInspAlgoData()
{
	return eSPCAlgoBridge;
}

int CPInsp_AlgoBridge::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	if (nWndType == eINSP_TAB)
	{
		eWholeNgTypeTemp[TypeTab] = e_NG;
		nCurrentNgType = TypeTab;
	}
	else if (nWndType == eINSP_SOLDER)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}
	else
	{
		eWholeNgTypeTemp[TypeLeadBridge] = e_NG;
		nCurrentNgType = TypeLeadBridge;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoBridge::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoBridge::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	if (stAlgoParam.m_bInspection == TRUE)
	{
		AlgoBridge* pAlgoBridge = (AlgoBridge *)sInspAlgo.m_ptrInspAlgoParam;
		if (pAlgoBridge && pAlgoBridge->m_bAutoSearchROI)
		{
			int nCntBridgeROI = stAlgoParam.m_rcArrTabBridge->GetCount();
			if (nCntBridgeROI > 0)
			{
				for (int i = 0; i < pAlgoBridge->nGapCnt; ++i)
				{
					CRect rcTab(pAlgoBridge->ptrrcGapRect[i]);

					CPoint xyCenOrg = rcTab.CenterPoint();
					int nIdxNear = -1;
					int nDistanceMin = 0;
					for (int nROI = 0; nROI < nCntBridgeROI; ++nROI)
					{
						CPoint xyOffset = stAlgoParam.m_rcArrTabBridge->GetAt(nROI).CenterPoint();
						xyOffset -= xyCenOrg;
						int nDistance = 0;//pow((double)xyOffset.x, 2) + pow((double)xyOffset.y, 2);
						if ((pAlgoBridge->nLeadTipDirection == e_TOP) || (pAlgoBridge->nLeadTipDirection == e_BOTTOM))
							nDistance = pow((double)xyOffset.x, 2);
						else
							nDistance = pow((double)xyOffset.y, 2);

						if ((nIdxNear == -1)
							|| (nDistanceMin > nDistance))
						{
							nIdxNear = nROI;
							nDistanceMin = nDistance;
						}
					}

					if (nIdxNear < 0)
						continue;

					CPoint xyOffset = stAlgoParam.m_rcArrTabBridge->GetAt(nIdxNear).CenterPoint() - xyCenOrg;
					rcTab.OffsetRect(xyOffset);
					pAlgoBridge->ptrrcGapRect[i] = rcTab;
				}
			}
		}
	}

	bResult = InspBridge(sInspAlgo, sWndAlgoImg, *sInspImageData, (RstAlgoBridge *)sRstAlgo, &stAlgoParam, ucArrDstImg);

	if (stAlgoParam.m_bInspection == TRUE)
	{

		RstAlgoBridge * rst = (RstAlgoBridge *)sRstAlgo;
		for (int n = 0; n < rst->m_nGapCnt; n++)
		{
			//if (rst->m_bLeadBridgeRst[n] == false) //shkim 2025.06.20 NG/OK상관없이 윈도우 좌표보정
			{
				rst->m_pofNGRectCenter[n].x += stAlgoParam.m_dx;
				rst->m_pofNGRectCenter[n].y += stAlgoParam.m_dy;
			}
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoBridge::InspBridge(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImgBuf, RstAlgoBridge * sRstAlgo, InspAlgoParam* stAlgoParam, UCHAR* pucRstMode2)
{
	BOOL bResult = FALSE;
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	int* pnResult = stAlgoParam->m_nReviewMin;
	double *pdResultHeight = stAlgoParam->m_pdResultHeight;
	AlignResult * arrAlignRes = stAlgoParam->m_sAlignRes;
	int nImgType = stAlgoParam->m_nTypeSelectBlob;
	if (sRstAlgo)
	{
		sRstAlgo->Init();
		sRstAlgo->m_nGapCnt = 0;
		memset(sRstAlgo->m_bLeadBridgeRst, 0, sRstAlgo->m_nGapCnt);
		memset(sRstAlgo->m_ucBridgeRectValue, 0, sRstAlgo->m_nGapCnt);
		memset(sRstAlgo->m_fBridgeRectValue, 0, sRstAlgo->m_nGapCnt);
	}
	if (pnResult)
		*pnResult = 0;
	if (pdResultHeight)
		*pdResultHeight = 0;

	if (sInspAlgo.m_eAlgoType != eAlgoBridge)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	AlgoBridge *pInspAlgoBridge = (AlgoBridge *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoBridge)
		return bResult;

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	int nGapRectCnt = pInspAlgoBridge->nGapCnt;
	float f3DSetValue = pInspAlgoBridge->m_dHeightReferenceValue;
	if (f3DSetValue < 0 || pInspAlgoBridge->bUse3D == FALSE)
		f3DSetValue = 0;

	BOOL bUseInsp2D = pInspAlgoBridge->m_bUseInsp2D;
	if (pInspAlgoBridge->m_nTypeBridge == AlgoBridge::eTab)
	{
		//bUseInsp2D = TRUE;
		//pInspAlgoBridge->m_bUseInsp3D = FALSE;
	}
	bool bType2 = ((pInspAlgoBridge->m_nOption & m_eData_Bridge_Use_Type2) == m_eData_Bridge_Use_Type2);
	bool bType2_SB = (bType2 == false) ? false : pInspAlgoBridge->m_bUseSolderBall;
	bool bType2_A = (bType2 == false) ? false : ((pInspAlgoBridge->m_nOption & m_eData_Bridge_Area) == m_eData_Bridge_Area);
	bool bType2_W = (bType2 == false) ? false : ((pInspAlgoBridge->m_nOption & m_eData_Bridge_Width) == m_eData_Bridge_Width);
	bool bType2_L = (bType2 == false) ? false : ((pInspAlgoBridge->m_nOption & m_eData_Bridge_Length) == m_eData_Bridge_Length);

	if (bUseInsp2D == FALSE && pInspAlgoBridge->m_bUseInsp3D == FALSE && bType2 == false &&
		bType2_SB == false && bType2_A == false && bType2_W == false && bType2_L == false)
		return bResult;

	if (nGapRectCnt <= 0)
	{
		if (sRstAlgo)
		{
			sRstAlgo->m_nGapCnt = nGapRectCnt;
			sRstAlgo->m_bLeadBridgeRst[0] = TRUE;
		}
		return bResult = TRUE;
	}

	UCHAR *ucColorImgSrc = NULL;
	if (bType2 && pColorTeach != NULL && pInspAlgoBridge->m_sBlobBase.m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgSrc, nImgWidth * nImgHeight);
		int nSideCameraIndex = sWndAlgoImg.m_nLight_index;
		pColorTeach->GetColorBaseBin(&pInspAlgoBridge->m_sBlobBase.m_sAlgoColorBase, sInspImgBuf, ucColorImgSrc, false, 0, 0, -1, nSideCameraIndex);
		m_pProcMilAlgo->SaveWorkImg(ucColorImgSrc, nImgWidth, nImgHeight, _T("OrgColor_Clip.bmp"));
	}

	m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("BridgeOrigin2D.bmp"));
	bool b3DUse = pInspAlgoBridge->bUse3D;
	/*int * ptrnResultBridge_2D = new int[nGapRectCnt];
	float * ptrfResultBridge_3D = new float[nGapRectCnt];
	double * ptrdResultBridge_3D = new double[nGapRectCnt];
	float * ptrfResultAreaPer = new float[nGapRectCnt];
	bool * ptrbResultMode2 = new bool[nGapRectCnt];
	bool * ptrbResultMode3 = new bool[nGapRectCnt];
	float * ptrfResultArea = new float[nGapRectCnt];
	float * ptrfResultWidth = new float[nGapRectCnt];
	float * ptrfResultLength = new float[nGapRectCnt];*/
	int * ptrnResultBridge_2D = g_pMManager->pem_new<int>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	float * ptrfResultBridge_3D = g_pMManager->pem_new<float>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	double * ptrdResultBridge_3D = g_pMManager->pem_new<double>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	float * ptrfResultAreaPer = g_pMManager->pem_new<float>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	bool * ptrbResultMode2 = g_pMManager->pem_new<bool>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	bool * ptrbResultMode3 = g_pMManager->pem_new<bool>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	float * ptrfResultArea = g_pMManager->pem_new<float>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	float * ptrfResultWidth = g_pMManager->pem_new<float>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	float * ptrfResultLength = g_pMManager->pem_new<float>(true, nGapRectCnt, (PCHAR)__FUNCTION__, __LINE__);
	bResult = TRUE;
	memset(ptrnResultBridge_2D, 0, nGapRectCnt * sizeof(int));
	memset(ptrdResultBridge_3D, 0, nGapRectCnt * sizeof(double));
	memset(ptrfResultBridge_3D, 0, nGapRectCnt * sizeof(float));
	memset(ptrfResultAreaPer, 0, nGapRectCnt * sizeof(float));
	memset(ptrbResultMode2, false, nGapRectCnt * sizeof(bool));
	memset(ptrbResultMode3, false, nGapRectCnt * sizeof(bool));
	memset(ptrfResultArea, 0, nGapRectCnt * sizeof(float));
	memset(ptrfResultWidth, 0, nGapRectCnt * sizeof(float));
	memset(ptrfResultLength, 0, nGapRectCnt * sizeof(float));
	if (pucRstMode2 != NULL)
	{
		memcpy(pucRstMode2, pucImgSrc, nImgWidth * nImgHeight * sizeof(UCHAR));
		m_pProcMilAlgo->SaveWorkImg(pucRstMode2, nImgWidth, nImgHeight, _T("Bridge Mode2 Rst.bmp"));
	}
	RECT rcGapRect;
	int nBridge2DValue = pInspAlgoBridge->nGrayDiff;
	float fBridge3DValue = pInspAlgoBridge->m_fHeightDiff;
	int nThinBridge = pInspAlgoBridge->m_fThinBridge / m_resolX;
	if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
		nThinBridge = pInspAlgoBridge->m_fThinBridge / m_resolY;
	bool bMode2 = pInspAlgoBridge->m_bUseMode2;
	if (bMode2 && pInspAlgoBridge->m_bInsp2DUpper)
		bUseInsp2D = FALSE;
	else
		bMode2 = false;
	for (int i = 0; i < nGapRectCnt; i++)
	{
		m_pCPInsp_Algo->AngleRectChange(sWndAlgoImg.dAngle, nImgWidth, nImgHeight, pInspAlgoBridge->ptrrcGapRect[i], &rcGapRect);
		if (rcGapRect.left < 0)
			rcGapRect.left = 0;
		if (rcGapRect.top < 0)
			rcGapRect.top = 0;
		if (rcGapRect.right < 0)
			rcGapRect.right = 0;
		if (rcGapRect.bottom < 0)
			rcGapRect.bottom = 0;
		if (arrAlignRes != NULL && arrAlignRes->nWindowID >= 0)
		{
			int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
			double dCorr_x = 0, dCorr_y = 0;
			double dROIW = rcGapRect.right - rcGapRect.left;
			double dROIH = rcGapRect.bottom - rcGapRect.top;
			double dROICX = rcGapRect.left + (dROIW / 2.0);
			double dROICY = rcGapRect.top + (dROIH / 2.0);
			int *ptrnWndID = NULL;
			if (nSize > 0)
			{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrnWndID, nSize);
				memset(ptrnWndID, 0, nSize);
			}
			for (int nIdx = 0; nIdx < nSize; nIdx++)
			{
				if (arrAlignRes[nIdx].nWindowID == -1)
					continue;
				if (nIdx == 0)
					ptrnWndID[nIdx] = arrAlignRes[nIdx].nWindowID;
				else
				{
					bool bInput = true;
					for (int nIdx2 = 0; nIdx2 < nIdx; nIdx2++)
					{
						if (ptrnWndID[nIdx2] == arrAlignRes[nIdx].nWindowID)
						{
							bInput = false;
							break;
						}
					}
					if (bInput == false)
						continue;
				}
				double dCx = ((arrAlignRes[nIdx].centerX + arrAlignRes[nIdx].offsetX) / m_resolX) + (sWndAlgoImg.m_nWidth3D / 2.0);
				double dCy = (sWndAlgoImg.m_nHeight3D / 2.0) - ((arrAlignRes[nIdx].centerY + arrAlignRes[nIdx].offsetY) / m_resolY);
				double dTheta = arrAlignRes[nIdx].theta;
				int nRstCor = m_proc3d.CorrectCoordinate(dROICX, dROICY, dCx, dCy, (dTheta * -1), 0, 0, &dCorr_x, &dCorr_y);
				if (nRstCor == 1)
				{
					int nGap = 0;
					if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
					{
						rcGapRect.left = dCorr_x - (dROIW / 2.0);
						if (rcGapRect.left < 0)
						{
							nGap = rcGapRect.left;
							rcGapRect.left = 0;
						}
						rcGapRect.right = rcGapRect.left + dROIW + nGap;
						if (rcGapRect.right - rcGapRect.left > nImgWidth)
							rcGapRect.right = nImgWidth - rcGapRect.left - 1;
					}
					else
					{
						rcGapRect.top = dCorr_y - (dROIH / 2.0);
						if (rcGapRect.top < 0)
						{
							nGap = rcGapRect.top;
							rcGapRect.top = 0;
						}
						rcGapRect.bottom = rcGapRect.top + dROIH + nGap;
						if (rcGapRect.bottom - rcGapRect.top > nImgHeight)
							rcGapRect.bottom = nImgHeight - rcGapRect.top - 1;
					}
				}
			}
			Delete_1DArray(&ptrnWndID);
		}
		int nBridgeWidth = 0;
		int nBridgeHeight = 0;
		int nBridgeEnd = 0;
		int nSearchBridgeArea = 0;
		int nCenter = 0;
		float fCenterX = 0;
		float fCenterY = 0;
		float fWidth = 0;
		float fLength = 0;
		if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
		{
			nBridgeWidth = abs(rcGapRect.right - rcGapRect.left);
			nBridgeHeight = 1;
			nBridgeEnd = abs(rcGapRect.bottom - rcGapRect.top);
			nSearchBridgeArea = nBridgeWidth;
			nCenter = rcGapRect.top;

			fWidth = abs(rcGapRect.right - rcGapRect.left);
			fLength = abs(rcGapRect.bottom - rcGapRect.top);
			fCenterX = rcGapRect.left + (fWidth / 2.0f);
			fCenterY = rcGapRect.top + (fLength / 2.0f);
		}
		else
		{
			nBridgeWidth = 1;
			nBridgeHeight = abs(rcGapRect.bottom - rcGapRect.top);
			nBridgeEnd = abs(rcGapRect.right - rcGapRect.left);
			nSearchBridgeArea = nBridgeHeight;
			nCenter = rcGapRect.left;

			fWidth = abs(rcGapRect.bottom - rcGapRect.top);
			fLength = abs(rcGapRect.right - rcGapRect.left);
			fCenterX = rcGapRect.top + (fWidth / 2.0f);
			fCenterY = rcGapRect.left + (fLength / 2.0f);
		}
		if ((nBridgeWidth <= 0) || (nBridgeHeight <= 0))
		{
			if (ptrnResultBridge_2D)
				Delete_1DArray(&ptrnResultBridge_2D);
			if (ptrdResultBridge_3D)
				Delete_1DArray(&ptrdResultBridge_3D);
			if (ptrfResultBridge_3D)
				Delete_1DArray(&ptrfResultBridge_3D);
			if (ptrfResultAreaPer)
				Delete_1DArray(&ptrfResultAreaPer);
			if (ptrbResultMode2)
				Delete_1DArray(&ptrbResultMode2);
			if (ptrbResultMode3)
				Delete_1DArray(&ptrbResultMode3);
			if (ptrfResultArea)
				Delete_1DArray(&ptrfResultArea);
			if (ptrfResultWidth)
				Delete_1DArray(&ptrfResultWidth);
			if (ptrfResultLength)
				Delete_1DArray(&ptrfResultLength);
			if (ucColorImgSrc)
				Delete_1DArray(&ucColorImgSrc);
			return bResult;
		}
		int nBridgeWidthback = nBridgeWidth;
		int nBridgeHeightback = nBridgeHeight;

		if (sRstAlgo)
		{
			if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
			{
				sRstAlgo->m_pofNGRectCenter[i].x = fCenterX;
				sRstAlgo->m_pofNGRectCenter[i].y = fCenterY;
				sRstAlgo->m_pofNGRectSize[i].x = fWidth;
				sRstAlgo->m_pofNGRectSize[i].y = fLength;
			}
			else
			{
				sRstAlgo->m_pofNGRectCenter[i].x = fCenterY;
				sRstAlgo->m_pofNGRectCenter[i].y = fCenterX;
				sRstAlgo->m_pofNGRectSize[i].x = fLength;
				sRstAlgo->m_pofNGRectSize[i].y = fWidth;
			}
			sRstAlgo->m_rcArrROI[i] = rcGapRect;
			sRstAlgo->m_rcArrGapRect[i] = pInspAlgoBridge->ptrrcGapRect[i];
		}

		UCHAR *pucImgDst = NULL;
		//pucImgDst = new UCHAR[nBridgeWidth * nBridgeHeight];
		pucImgDst = g_pMManager->pem_new<UCHAR>(true, nBridgeWidth * nBridgeHeight, (PCHAR)__FUNCTION__, __LINE__);
		float *pfImgDst_3D = NULL;
		//pfImgDst_3D = new float[nBridgeWidth * nBridgeHeight];
		pfImgDst_3D = g_pMManager->pem_new<float>(true, nBridgeWidth * nBridgeHeight, (PCHAR)__FUNCTION__, __LINE__);
		int nGrayMaxValue = (nBridge2DValue < defBridgeGrayMaxValue) ? defBridgeGrayMaxValue : nBridge2DValue;
		//if(nGrayMaxValue < defBridgeGrayMaxValue)
		//	nGrayMaxValue = defBridgeGrayMaxValue;		

		float fDiffValue = 0.0F;
		int nCntValuable = 0;
		for (int j = 0; j < nBridgeEnd; j++)
		{
			int nStartX = 0;
			int nStartY = 0;

			if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
			{
				//nStartX = rcGapRect.left + nBridgeWidth/2;
				nStartX = rcGapRect.left;
				nStartY = j + nCenter;
			}
			else
			{
				nStartX = j + nCenter;
				//nStartY = rcGapRect.top + nBridgeHeight/2;
				nStartY = rcGapRect.top;
			}
			if (nStartX < 0) nStartX = 0;
			if (nStartY < 0) nStartY = 0;
			if (nStartX + nBridgeWidth > nImgWidth)
				nBridgeWidth = nImgWidth - nStartX;
			if (nStartY + nBridgeHeight > nImgHeight)
				nBridgeHeight = nImgHeight - nStartY;
			if (nBridgeWidth < 1 || nBridgeHeight < 1)
				continue;

			if (bUseInsp2D == TRUE)
			{
#if _DEBUG
				cv::Mat ImgSrc(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc);
				cv::Mat ImgDst(nBridgeHeight, nBridgeWidth, CV_8UC1, pucImgDst);
#endif
				UCHAR ucGrayMinValue = 0.0F;
				UCHAR ucGrayMaxValue = 0.0F;
				memset(pucImgDst, 0, nBridgeWidth * nBridgeHeight);
				m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucImgDst, nStartX, nStartY, nBridgeWidth, nBridgeHeight);
				ippsMinMax_8u(pucImgDst, nBridgeWidth * nBridgeHeight, &ucGrayMinValue, &ucGrayMaxValue);
				//m_pProcMilAlgo->SaveWorkImg(pucImgDst, nBridgeWidth, nBridgeHeight, _T("Bridge Dst2D.bmp"));

				fDiffValue = ucGrayMaxValue - ucGrayMinValue;
				if (pInspAlgoBridge->m_bInsp2DUpper == FALSE)
					fDiffValue = ucGrayMinValue;
				else
				{
					if (ucGrayMaxValue == ucGrayMinValue || (ucGrayMaxValue >= nBridge2DValue && ucGrayMinValue >= nBridge2DValue))
						fDiffValue = (ucGrayMaxValue + ucGrayMinValue) / 2.0f;
				}
				if (nThinBridge > 0
					&& ((pInspAlgoBridge->m_bInsp2DUpper == TRUE && fDiffValue > nBridge2DValue)
						|| (pInspAlgoBridge->m_bInsp2DUpper == FALSE && fDiffValue < nBridge2DValue)))
				{
					bool bChange = true;
					ucGrayMinValue = 0.0F;
					int nThinWidth = nBridgeHeight;
					if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
						nThinWidth = nBridgeWidth;
					if (nThinBridge < nThinWidth)
					{
						for (int nX = 0; nX < nThinWidth; nX++)
						{
							if (nX + nThinBridge + 1 >= nThinWidth)
								break;

							for (int nIdx = nX; nIdx < nX + nThinBridge + 1; nIdx++)
							{
								if (nX == 0 && nIdx == 0)
									ucGrayMinValue = pucImgDst[nX];
								else
								{
									if (ucGrayMinValue > pucImgDst[nX])
										ucGrayMinValue = pucImgDst[nX];
								}
							}
							if ((pInspAlgoBridge->m_bInsp2DUpper == TRUE && ucGrayMinValue > nBridge2DValue)
								|| (pInspAlgoBridge->m_bInsp2DUpper == FALSE && ucGrayMinValue < nBridge2DValue))
							{
								bChange = false;
								break;
							}
						}
						if (bChange == true)
							fDiffValue = ucGrayMinValue;
					}
				}
				if (j == 0)
					ptrnResultBridge_2D[i] = fDiffValue;
				if (pInspAlgoBridge->m_bInsp2DUpper == TRUE)
				{
					if (ptrnResultBridge_2D[i] > fDiffValue)
						ptrnResultBridge_2D[i] = fDiffValue;
				}
				else
				{
					if (ptrnResultBridge_2D[i] < fDiffValue)
						ptrnResultBridge_2D[i] = fDiffValue;
				}

				if ((pInspAlgoBridge->m_nTypeBridge == AlgoBridge::eTab) && (fDiffValue <= nBridge2DValue))
					++nCntValuable;
			}
			if (pInspAlgoBridge->m_bUseInsp3D == TRUE)
			{
				float fHeightMinValue = 0.0F;
				float fHeightMaxValue = 0.0F;
				memset(pfImgDst_3D, 0, nBridgeWidth * nBridgeHeight);
				m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst_3D, nImgWidth, nImgHeight, nStartX, nStartY, nBridgeWidth, nBridgeHeight);
				ippsMinMax_32f(pfImgDst_3D, nBridgeWidth * nBridgeHeight, &fHeightMinValue, &fHeightMaxValue);

				if (fHeightMinValue >= fBridge3DValue && fHeightMaxValue >= fBridge3DValue)
					fDiffValue = (fHeightMinValue + fHeightMaxValue) / 2.0f;
				else
					fDiffValue = fHeightMaxValue - fHeightMinValue;
				if (nThinBridge > 0 && fDiffValue > fBridge3DValue)
				{
					bool bChange = true;
					fHeightMinValue = 0.0F;
					int nThinWidth = nBridgeHeight;
					if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
						nThinWidth = nBridgeWidth;
					if (nThinBridge < nThinWidth)
					{
						for (int nX = 0; nX < nThinWidth; nX++)
						{
							if (nX + nThinBridge + 1 >= nThinWidth)
								break;

							for (int nIdx = nX; nIdx < nX + nThinBridge + 1; nIdx++)
							{
								if (nX == 0 && nIdx == 0)
									fHeightMinValue = pfImgDst_3D[nX];
								else
								{
									if (fHeightMinValue > pfImgDst_3D[nX])
										fHeightMinValue = pfImgDst_3D[nX];
								}
							}
							if (fHeightMinValue > fBridge3DValue)
							{
								bChange = false;
								break;
							}
						}
						if (bChange == true)
							fDiffValue = fHeightMinValue;
					}
				}
				if (j == 0)
					ptrfResultBridge_3D[i] = fDiffValue;

				if (ptrfResultBridge_3D[i] > fDiffValue)
					ptrfResultBridge_3D[i] = fDiffValue;
			}
		}
		if (pucImgDst)
			Delete_1DArray(&pucImgDst);
		if (pfImgDst_3D)
			Delete_1DArray(&pfImgDst_3D);

		if (bUseInsp2D == TRUE && pfImgSrc != NULL && b3DUse == true &&
			((pInspAlgoBridge->m_bInsp2DUpper == TRUE && ptrnResultBridge_2D[i] > nBridge2DValue) ||
			(pInspAlgoBridge->m_bInsp2DUpper == FALSE && ptrnResultBridge_2D[i] < nBridge2DValue)))
		{

			nBridgeWidth = nBridgeWidthback;
			nBridgeHeight = nBridgeHeightback;
			UCHAR *pucImgDst_Search = NULL;
			int nStartX = nCenter;
			int nStartY = nCenter;
			int n2DW = 1;
			int n2DH = 1;
			int nSearchGrayAreaStartX = -1;
			int nSearchGrayAreaStartY = -1;
			int nSearchGrayAreaEndX = -1;
			int nSearchGrayAreaEndY = -1;
			int nSearchWidth = -1;
			int nSearchHeight = -1;
			int nResultGray = 0;
			if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))	// 가로로 긴 방향
				n2DH = nBridgeEnd = abs(rcGapRect.bottom - rcGapRect.top);
			else
				n2DW = nBridgeEnd = abs(rcGapRect.right - rcGapRect.left);
			//pucImgDst_Search = new UCHAR[nBridgeEnd];
			pucImgDst_Search = g_pMManager->pem_new<UCHAR>(true, nBridgeEnd, (PCHAR)__FUNCTION__, __LINE__);
			for (int j = 0; j < nSearchBridgeArea; j++)
			{
				if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))	// 가로로 긴 방향
					nStartX = rcGapRect.left + j;
				else
					nStartY = rcGapRect.top + j;
				memset(pucImgDst_Search, 0, nBridgeEnd);
				m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucImgDst_Search, nStartX, nStartY, n2DW, n2DH);
				//m_pProcMilAlgo->SaveWorkImg(pucImgDst, n2DW, n2DH, _T("Bridge Area2D.bmp"));

				IppiSize roiSize;
				roiSize.width = nBridgeEnd;
				roiSize.height = 1;
				Ipp64f mean;
				ippiMean_8u_C1R(pucImgDst_Search, nImgWidth, roiSize, &mean);

				if ((pInspAlgoBridge->m_bInsp2DUpper == TRUE && mean > nBridge2DValue)
					|| (pInspAlgoBridge->m_bInsp2DUpper == FALSE && mean < nBridge2DValue))
				{
					if (nSearchGrayAreaStartX == -1 && nSearchGrayAreaStartY == -1)
					{
						nSearchGrayAreaStartX = nStartX;
						nSearchGrayAreaStartY = nStartY;
					}
					else
					{
						nSearchGrayAreaEndX = nStartX;
						nSearchGrayAreaEndY = nStartY;
					}
				}
			}
			if (pucImgDst_Search)
				Delete_1DArray(&pucImgDst_Search);

			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("BridgeOrigin3D.bmp"));
			UCHAR *pucImgDst_2D = NULL;
			fDiffValue = 0.0F;
			int n3DSizeBuf = 3;
			int nBridge3DGrayDefault = nBridge2DValue * 1.2;
			if (pInspAlgoBridge->m_bInsp2DUpper == FALSE)
				nBridge3DGrayDefault = nBridge2DValue * 0.8;
			if (nBridge3DGrayDefault > 255)
				nBridge3DGrayDefault = nBridge2DValue;
			for (int j = 0; j < nBridgeEnd; j++)
			{
				int nStartX = 0;
				int nStartY = 0;

				if ((pInspAlgoBridge->nLeadTipDirection == 0) || (pInspAlgoBridge->nLeadTipDirection == 1))
				{
					nStartX = nSearchGrayAreaStartX - n3DSizeBuf;
					if (nStartX < 0)
						nStartX = 0;
					int nStartY_Buf = nSearchGrayAreaStartY - n3DSizeBuf;
					if (nStartY_Buf < 0)
						nStartY_Buf = 0;
					if (nBridgeEnd > 6)
					{
						if (j == nBridgeEnd - 2)
							break;
						else
							nStartY = (j + 1) + nStartY_Buf;
					}
					else
						nStartY = j + nStartY_Buf;
					int nBridgeWidth_Buf = (nSearchGrayAreaEndX + n3DSizeBuf) - (nSearchGrayAreaStartX - n3DSizeBuf);
					if (nBridgeWidth_Buf > nBridgeWidth)
						nBridgeWidth_Buf = nBridgeWidth;
					if (0 > nBridgeWidth_Buf)
						nBridgeWidth_Buf = nBridgeWidth;
					nBridgeWidth = nBridgeWidth_Buf;
				}
				else
				{
					int nStartX_Buf = nSearchGrayAreaStartX - n3DSizeBuf;
					if (nStartX_Buf < 0)
						nStartX_Buf = 0;

					if (nBridgeEnd > 6)
					{
						if (j == nBridgeEnd - 2)
							break;
						else
							nStartX = (j + 1) + nStartX_Buf;
					}
					else
						nStartX = j + nStartX_Buf;
					nStartY = nSearchGrayAreaStartY - n3DSizeBuf;
					if (nStartY < 0)
						nStartY = 0;
					int nBridgeHeight_Buf = (nSearchGrayAreaEndY + n3DSizeBuf) - (nSearchGrayAreaStartY - n3DSizeBuf);
					if (nBridgeHeight_Buf > nBridgeHeight)
						nBridgeHeight_Buf = nBridgeHeight;
					if (0 > nBridgeHeight_Buf)
						nBridgeHeight_Buf = nBridgeHeight;
					nBridgeHeight = nBridgeHeight_Buf;
				}

				if (0 >= nBridgeWidth || 0 >= nBridgeHeight)
					continue;

				float fHeightMinValue = -1;
				float fHeightMaxValue = -1;

				/*pucImgDst_2D = new UCHAR[nBridgeWidth * nBridgeHeight];
				pfImgDst_3D = new float[nBridgeWidth * nBridgeHeight];*/
				pucImgDst_2D = g_pMManager->pem_new<UCHAR>(true, nBridgeWidth * nBridgeHeight, (PCHAR)__FUNCTION__, __LINE__);
				pfImgDst_3D = g_pMManager->pem_new<float>(true, nBridgeWidth * nBridgeHeight, (PCHAR)__FUNCTION__, __LINE__);
				memset(pfImgDst_3D, 0, nBridgeWidth * nBridgeHeight);
				memset(pucImgDst_2D, 0, nBridgeWidth * nBridgeHeight);
				m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst_3D, nImgWidth, nImgHeight, nStartX, nStartY, nBridgeWidth, nBridgeHeight);
				m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucImgDst_2D, nStartX, nStartY, nBridgeWidth, nBridgeHeight);
				for (int nY = 0; nY < nBridgeHeight; nY++)
				{
					for (int nX = 0; nX < nBridgeWidth; nX++)
					{
						int nGray = pucImgDst_2D[nX + (nY * nBridgeWidth)];
						if ((pInspAlgoBridge->m_bInsp2DUpper == TRUE && nGray > nBridge3DGrayDefault)
							|| (pInspAlgoBridge->m_bInsp2DUpper == FALSE && nGray < nBridge3DGrayDefault))
						{
							float f3DValue = pfImgDst_3D[nX + (nY * nBridgeWidth)];
							if (f3DValue < 0)
								f3DValue = 0;
							if (fHeightMinValue == -1 && fHeightMaxValue == -1)
							{
								fHeightMinValue = f3DValue;
								fHeightMaxValue = f3DValue;
							}
							else
							{
								if (f3DValue < fHeightMinValue)
									fHeightMinValue = f3DValue;
								if (f3DValue > fHeightMaxValue)
									fHeightMaxValue = f3DValue;
							}
						}
					}
				}
				//m_pProcMilAlgo->SaveWorkImg_float(pfImgDst, nBridgeWidth, nBridgeHeight, _T("BridgeClip3D.bmp"));
				//ippsMinMax_32f(pfImgDst_3D, nBridgeWidth * nBridgeHeight, &fHeightMinValue, &fHeightMaxValue);
				//double mean = m_pCPInsp_Algo->GetHeightMean(pfImgDst, nBridgeWidth, nBridgeHeight);

				if (fHeightMaxValue >= f3DSetValue && fHeightMinValue >= f3DSetValue)
					fDiffValue = (fHeightMaxValue + fHeightMinValue) / 2.0f;
				else
					fDiffValue = fHeightMaxValue - fHeightMinValue;

				if (j == 0)
					ptrdResultBridge_3D[i] = fDiffValue;

				if (ptrdResultBridge_3D[i] > fDiffValue)
					ptrdResultBridge_3D[i] = fDiffValue;

				if (pfImgDst_3D)
					Delete_1DArray(&pfImgDst_3D);
				if (pucImgDst_2D)
					Delete_1DArray(&pucImgDst_2D);
			}
			if (pfImgDst_3D)
				Delete_1DArray(&pfImgDst_3D);
			if (pucImgDst_2D)
				Delete_1DArray(&pucImgDst_2D);
			if (pucImgDst_Search)
				Delete_1DArray(&pucImgDst_Search);
		}
// 		if (sRstAlgo && (pInspAlgoBridge->m_nTypeBridge == AlgoBridge::eTab)
// 			&& (pInspAlgoBridge->m_bUseInsp2D == TRUE || pInspAlgoBridge->m_bUseInsp3D == TRUE))
// 		{
// 			if (nCntValuable >= (nBridgeEnd * pInspAlgoBridge->m_dPercentOK / 100))
// 				sRstAlgo->m_bLeadBridgeRst[i] = TRUE;
// 			else
// 				sRstAlgo->m_bLeadBridgeRst[i] = FALSE;
// 
// 			if (sRstAlgo->m_bLeadBridgeRst[i] == FALSE)
// 				bResult = FALSE;
// 
// 			sRstAlgo->m_ucBridgeRectValue[i] = /*ptrnResultBridge_2D[i] =*/ (double)nCntValuable / nBridgeEnd * 100;
// 		}
		if (bMode2)
			ptrbResultMode2[i] = InspBridgeMode2(pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, rcGapRect, pInspAlgoBridge->nLeadTipDirection, nBridge2DValue, f3DSetValue, nThinBridge, &ptrnResultBridge_2D[i], pucRstMode2);

		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;

		if (bType2)
		{
			int nMargin = 0;
			int nStartX_Area = rcGapRect.left + nMargin;
			int nStartY_Area = rcGapRect.top + nMargin;
			int nWidth_Area = rcGapRect.right - rcGapRect.left - (nMargin * 2);
			int nHeight_Area = rcGapRect.bottom - rcGapRect.top - (nMargin * 2);
			if (nStartX_Area < 0 || nStartY_Area < 0 || nWidth_Area < 0 || nHeight_Area < 0)
			{
				nMargin = 0;
				nStartX_Area = rcGapRect.left + nMargin;
				nStartY_Area = rcGapRect.top + nMargin;
				nWidth_Area = rcGapRect.right - rcGapRect.left - (nMargin * 2);
				nHeight_Area = rcGapRect.bottom - rcGapRect.top - (nMargin * 2);
			}
			if (nWidth_Area == 0)
			{
				nStartX_Area = rcGapRect.left;
				nWidth_Area = rcGapRect.right - rcGapRect.left;
			}
			if (nHeight_Area == 0)
			{
				nStartY_Area = rcGapRect.top;
				nHeight_Area = rcGapRect.bottom - rcGapRect.top;
			}
			if (nWidth_Area <= 0 || nHeight_Area <= 0)
				continue;
			if (nStartX_Area + nWidth_Area > nImgWidth)
				nWidth_Area = nImgWidth - nStartX_Area - 1;
			if (nStartY_Area + nHeight_Area > nImgHeight)
				nHeight_Area = nImgHeight - nStartY_Area - 1;
			if (nWidth_Area <= 0 || nHeight_Area <= 0)
				continue;
			UCHAR *pucImgDst_Area = NULL;
			float *pfImgDst_3D_Area = NULL;
			UCHAR * ucArrBlobDst = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucImgDst_Area, nWidth_Area * nHeight_Area);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pfImgDst_3D_Area, nWidth_Area * nHeight_Area);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobDst, nWidth_Area * nHeight_Area);
			memset(pucImgDst_Area, 0, nWidth_Area * nHeight_Area);
			memset(pfImgDst_3D_Area, 0, nWidth_Area * nHeight_Area);
			memset(ucArrBlobDst, 0, nWidth_Area * nHeight_Area);
			m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucImgDst_Area, nStartX_Area, nStartY_Area, nWidth_Area, nHeight_Area);
			m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst_3D_Area, nImgWidth, nImgHeight, nStartX_Area, nStartY_Area, nWidth_Area, nHeight_Area);
			m_pProcMilAlgo->SaveWorkImg(pucImgDst_Area, nWidth_Area, nHeight_Area, _T("Bridge Area2D.bmp"));
			m_pProcMilAlgo->SaveWorkImg_float(pfImgDst_3D_Area, nWidth_Area, nHeight_Area, _T("BridgeClip3D.bmp"));
			double dArea = 0, cx = 0, cy = 0;
			CRect cBlobRect;
			UCHAR *ucColorImgDst = NULL;
			if (ucColorImgSrc != NULL && pInspAlgoBridge->m_sBlobBase.m_sAlgoColorBase.m_bUseColor == TRUE)
			{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, nWidth_Area * nHeight_Area);
				m_pProcMilAlgo->GetClipImage_LT(ucColorImgSrc, nImgWidth, nImgHeight, ucColorImgDst, nStartX_Area, nStartY_Area, nWidth_Area, nHeight_Area);
				m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nWidth_Area, nHeight_Area, _T("Bridge Area Color.bmp"));
			}
			int nBlobNum = m_pCPInsp_Algo->BlobImageStruct_Base(pInspAlgoBridge->m_sBlobBase, pucImgDst_Area, pfImgDst_3D_Area, ucColorImgDst, nWidth_Area, nHeight_Area, stTieAreaNULL, ucArrBlobDst);
			if (nBlobNum > 0 && stAlgoParam->m_pvInspRstPoly)
			{
				
				int nWindID = g_pInspMng->GetWindowID(stAlgoParam->m_nWndIndex);
				int nAlgoID = sInspAlgo.m_nAlgoId; 
				vector<POINTF> vPolyCenter;
				vector<vector<POINTF>> vPolygons;
				int nStartX = nStartX_Area + stAlgoParam->m_dx; // Part 이미지 좌표로 바꾸기 위한 포인트
				int nStartY = nStartY_Area + stAlgoParam->m_dy; // Part 이미지 좌표로 바꾸기 위한 포인트
				m_pCPInsp_Algo->GetBlobPolygon(ucArrBlobDst, nWidth_Area, nHeight_Area, nStartX, nStartY, &vPolygons, &vPolyCenter);
				auto arrPolyCenter = vPolyCenter.data();
				auto arrPolygon = vPolygons.data();
				int nPolySize = vPolygons.size();
				for (int idxPloy = 0; idxPloy < nPolySize; idxPloy++)
				{
					InspRstPolyAlgo vInspPoly(stAlgoParam->m_dx, stAlgoParam->m_dy, nWindID, nAlgoID, i, nPolySize, arrPolyCenter[idxPloy], arrPolygon[idxPloy]);
					stAlgoParam->m_pvInspRstPoly->push_back(vInspPoly);
				}
			}
			m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nWidth_Area, nHeight_Area, _T("Bridge Rst.bmp"));
			if (pucRstMode2 != NULL && nImgType == 1)
			{
				for (int nX = 0; nX < nWidth_Area; nX++)
				{
					for (int nY = 0; nY < nHeight_Area; nY++)
					{
						int nIndex = (nY * nWidth_Area) + nX;
						int nIndex2 = ((nY + nStartY_Area) * nImgWidth) + (nX + nStartX_Area);
						pucRstMode2[nIndex2] = ucArrBlobDst[nIndex];
					}
				}
				m_pProcMilAlgo->SaveWorkImg(pucRstMode2, nImgWidth, nImgHeight, _T("Bridge Mode2 Full Rst Bin.bmp"));
				cv::Mat img(nImgHeight, nImgWidth, CV_8UC1, pucRstMode2);
			}

			ptrbResultMode3[i] = (nBlobNum > 0);
			if (nBlobNum > 0)
			{
				std::vector<CRect> vBlobRects;
				m_pProcMilAlgo->GetBlobResult_RectsV(vBlobRects);
				double diameter = pInspAlgoBridge->m_fSolderBallArea / m_resolX;
				// 원형 찾기
				cv::Mat model(nHeight_Area, nWidth_Area, CV_8UC1, ucArrBlobDst);
				float fResult(0.0);
				int nMaxIndex(0);
				m_pProcMilAlgo->InspSolderBall(model, vBlobRects, &nMaxIndex, &fResult);
				if (vBlobRects.size() > nMaxIndex)
				{
					int nBlobW = vBlobRects[nMaxIndex].Width();
					int nBlobH = vBlobRects[nMaxIndex].Height();
					if (nBlobW > diameter && nBlobH > diameter)
						ptrfResultAreaPer[i] = fResult * 100.0f;
				}
				double *dArrA = NULL;
				CRect *cArrRect = NULL;
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrA, nBlobNum);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &cArrRect, nBlobNum);
				m_pProcMilAlgo->GetBlobResult_ALL(dArrA, NULL, NULL, cArrRect);
				for (int j = 0; j < nBlobNum; j++)
				{
					double dTempArea = dArrA[j] * m_resolX * m_resolY;
					double dTempW = cArrRect[j].Width() * m_resolX;
					double dTempH = cArrRect[j].Height() * m_resolY;
					if (dTempArea > ptrfResultArea[i])
					{
						ptrfResultArea[i] = dTempArea;
						ptrfResultWidth[i] = dTempW;
						ptrfResultLength[i] = dTempH;
					}
				}
				Delete_1DArray(&dArrA);
				Delete_1DArray(&cArrRect);
			}
			Delete_1DArray(&pucImgDst_Area);
			Delete_1DArray(&pfImgDst_3D_Area);
			Delete_1DArray(&ucArrBlobDst);
			Delete_1DArray(&ucColorImgDst);
		}
	}// End of for (int i = 0; i < nGapRectCnt; i++)

	if (sRstAlgo)
	{
		sRstAlgo->m_nGapCnt = pInspAlgoBridge->nGapCnt;

// 		bool bTab = true;
// 		if (pInspAlgoBridge->m_nTypeBridge == AlgoBridge::eTab)
// 		{
// 			if (pInspAlgoBridge->m_bUseInsp2D == TRUE || pInspAlgoBridge->m_bUseInsp3D == TRUE)
// 			{
// 				for (int n = 0; n < nGapRectCnt; n++)
// 				{
// 					if (sRstAlgo->m_bLeadBridgeRst[n] == FALSE)
// 					{
// 						bTab = false;
// 						break;
// 					}
// 				}
// 			}
// 		}

		//if(pInspAlgoBridge->m_nTypeBridge != AlgoBridge::eTab)
		{
			for (int n = 0; n < nGapRectCnt; n++)
			{
				bool b2DInspOK = false;
				bool b3DInspOK = false;
				bool bSBInspOK = false;
				bool bAInspOK = false;
				bool bWInspOK = false;
				bool bLInspOK = false;
// 				if (bTab == false)
// 					break;

				sRstAlgo->m_bLeadBridgeRst[n] = FALSE;
				if (bUseInsp2D == TRUE)
				{
					if (pInspAlgoBridge->m_bInsp2DUpper == TRUE)
					{
						if (nBridge2DValue >= ptrnResultBridge_2D[n] && ptrdResultBridge_3D[n] == 0)
							b2DInspOK = true;
						else if (b3DUse == true && pInspAlgoBridge->m_dHeightReferenceValue >= ptrdResultBridge_3D[n])
							b2DInspOK = true;
					}
					else
					{
						if (nBridge2DValue <= ptrnResultBridge_2D[n] && ptrdResultBridge_3D[n] == 0)
							b2DInspOK = true;
						else if (b3DUse == true && pInspAlgoBridge->m_dHeightReferenceValue >= ptrdResultBridge_3D[n])
							b2DInspOK = true;
					}
					sRstAlgo->m_ucBridgeRectValue[n] = ptrnResultBridge_2D[n];
					sRstAlgo->m_fBridgeRectValue2D3D[n] = ptrdResultBridge_3D[n];
				}
				else
				{
					if (bMode2)
					{
						sRstAlgo->m_ucBridgeRectValue[n] = ptrnResultBridge_2D[n];
						if (ptrbResultMode2[n])
							b2DInspOK = true;
					}
					else
						b2DInspOK = true;
				}
				if (pInspAlgoBridge->m_bUseInsp3D == TRUE)
				{
					if (fBridge3DValue >= ptrfResultBridge_3D[n])
						b3DInspOK = true;

					sRstAlgo->m_fBridgeRectValue[n] = ptrfResultBridge_3D[n];
				}
				else
					b3DInspOK = true;

				float fRstType2 = 0;
				sRstAlgo->m_nLeadBridgeRst[n] = 0;
				if (bType2)
				{
					if (bType2_SB && ptrbResultMode3[n])
					{
						if (pInspAlgoBridge->m_fSolderBallAreaPer >= ptrfResultAreaPer[n])
							bSBInspOK = true;
						else
						{
							sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_SB;
							fRstType2 = ptrfResultAreaPer[n] - pInspAlgoBridge->m_fSolderBallAreaPer;
						}
						sRstAlgo->m_fSolderBallRectValuePer[n] = ptrfResultAreaPer[n];
					}
					else
						bSBInspOK = true;
					if (bType2_A && ptrbResultMode3[n])
					{
						if (pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Area] > ptrfResultArea[n])
							bAInspOK = true;
						else
						{
							float fRstTemp = ptrfResultArea[n] - pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Area];
							if (fRstTemp > fRstType2)
							{
								sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_A;
								fRstType2 = fRstTemp;
							}
						}
						sRstAlgo->m_fAreaRectValuePer[n] = ptrfResultArea[n];
					}
					else
						bAInspOK = true;
					if (bType2_W && bType2_L && ptrbResultMode3[n])	// W, L 둘다 사용하는 경우 and조건으로 검사
					{
						if (pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Width] > ptrfResultWidth[n] 
							|| pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Length] > ptrfResultLength[n])
							bWInspOK = bLInspOK = true;
						else
						{
							sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_W;
							sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_L;
							float fRstTemp = ptrfResultWidth[n] - pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Width];
							if (fRstTemp > fRstType2)
							{
								fRstType2 = fRstTemp;
							}
							else
							{
								fRstTemp = ptrfResultLength[n] - pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Length];
								if (fRstTemp > fRstType2)
								{
									fRstType2 = fRstTemp;
								}
							}
						}
						sRstAlgo->m_fWidthRectValuePer[n] = ptrfResultWidth[n];
						sRstAlgo->m_fLengthRectValuePer[n] = ptrfResultLength[n];
					}
					else
					{
						if (bType2_W && ptrbResultMode3[n])
						{
							if (pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Width] > ptrfResultWidth[n])
								bWInspOK = true;
							else
							{
								float fRstTemp = ptrfResultWidth[n] - pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Width];
								if (fRstTemp > fRstType2)
								{
									sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_W;
									fRstType2 = fRstTemp;
								}
							}
							sRstAlgo->m_fWidthRectValuePer[n] = ptrfResultWidth[n];
						}
						else
							bWInspOK = true;
						if (bType2_L && ptrbResultMode3[n])
						{
							if (pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Length] > ptrfResultLength[n])
								bLInspOK = true;
							else
							{
								float fRstTemp = ptrfResultLength[n] - pInspAlgoBridge->m_fArrValue[m_eOPT_Bridge_Length];
								if (fRstTemp > fRstType2)
								{
									sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_L;
									fRstType2 = fRstTemp;
								}
							}
							sRstAlgo->m_fLengthRectValuePer[n] = ptrfResultLength[n];
						}
						else
							bLInspOK = true;
					}
				}
				else
				{
					bSBInspOK = true;
					bAInspOK = true;
					bWInspOK = true;
					bLInspOK = true;
				}

				if (b2DInspOK == true && b3DInspOK == true && bSBInspOK == true && bAInspOK == true && bWInspOK == true && bLInspOK == true/* && bTab == true*/)
				{
					sRstAlgo->m_bLeadBridgeRst[n] = TRUE;
					sRstAlgo->m_nLeadBridgeRst[n] = 0;
				}
				else
				{
					if (b2DInspOK == false)
						sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_2D;
					else if (b3DInspOK == false)
						sRstAlgo->m_nLeadBridgeRst[n] |= m_eBridge_Defect_3D;

					bResult = FALSE;
				}
			}
		}
	}
	if (pnResult)
	{
		int nResult = 0;
		for (int n = 0; n < nGapRectCnt; n++)
		{
			if (nResult < ptrnResultBridge_2D[n])
				nResult = ptrnResultBridge_2D[n];
		}
		*pnResult = nResult;
	}
	if (pdResultHeight)
	{
		double dResult = 0;
		for (int n = 0; n < nGapRectCnt; n++)
		{
			if (bUseInsp2D == TRUE && pInspAlgoBridge->bUse3D == TRUE)
			{
				if (dResult < ptrdResultBridge_3D[n])
					dResult = ptrdResultBridge_3D[n];
			}
			else if (pInspAlgoBridge->m_bUseInsp3D == TRUE)
			{
				if (dResult < ptrfResultBridge_3D[n])
					dResult = ptrfResultBridge_3D[n];
			}
		}
		*pdResultHeight = dResult;
	}
	if (ucColorImgSrc)
		Delete_1DArray(&ucColorImgSrc);
	if (ptrnResultBridge_2D)
		Delete_1DArray(&ptrnResultBridge_2D);
	if (ptrdResultBridge_3D)
		Delete_1DArray(&ptrdResultBridge_3D);
	if (ptrfResultBridge_3D)
		Delete_1DArray(&ptrfResultBridge_3D);
	if (ptrfResultAreaPer)
		Delete_1DArray(&ptrfResultAreaPer);
	if (ptrbResultMode2)
		Delete_1DArray(&ptrbResultMode2);
	if (ptrbResultMode3)
		Delete_1DArray(&ptrbResultMode3);
	if (ptrfResultArea)
		Delete_1DArray(&ptrfResultArea);
	if (ptrfResultWidth)
		Delete_1DArray(&ptrfResultWidth);
	if (ptrfResultLength)
		Delete_1DArray(&ptrfResultLength);
	if (pucRstMode2)
		m_pProcMilAlgo->SaveWorkImg(pucRstMode2, nImgWidth, nImgHeight, _T("Bridge Mode2 Full Rst Bin2.bmp"));
	return bResult;
}
bool CPInsp_AlgoBridge::InspBridgeMode2(UCHAR *pucImgSrc, float *pfImgSrc, int nImgWidth, int nImgHeight, RECT rcGapRect, int nDir, int nBridge2DValue, float f3DSetValue, int nThinBridge, int *nRstSize, UCHAR* pucRstMode2)
{
	bool bResult = FALSE;
	*nRstSize = 0;
	int nROIW = rcGapRect.right - rcGapRect.left;
	int nROIH = rcGapRect.bottom - rcGapRect.top;
	if (nROIW <= 10 || nROIH <= 10 || rcGapRect.left >= nImgWidth || rcGapRect.right > nImgWidth ||
		rcGapRect.top >= nImgHeight || rcGapRect.bottom > nImgHeight)
		return bResult;
	/*UCHAR *pucImgDst_Bin = new UCHAR[nROIW * nROIH];
	UCHAR *pucImgDst = new UCHAR[nROIW * nROIH];
	float *pfImgDst_3D = new float[nROIW * nROIH];*/
	UCHAR *pucImgDst_Bin = g_pMManager->pem_new<UCHAR>(true, nROIW * nROIH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR *pucImgDst = g_pMManager->pem_new<UCHAR>(true, nROIW * nROIH, (PCHAR)__FUNCTION__, __LINE__);
	float *pfImgDst_3D = g_pMManager->pem_new<float>(true, nROIW * nROIH, (PCHAR)__FUNCTION__, __LINE__);
	memset(pucImgDst_Bin, 0, nROIW * nROIH * sizeof(UCHAR));
	memset(pucImgDst, 0, nROIW * nROIH * sizeof(UCHAR));
	memset(pfImgDst_3D, 0, nROIW * nROIH * sizeof(UCHAR));
	m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucImgDst, rcGapRect.left, rcGapRect.top, nROIW, nROIH);
	m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("Bridge Mode2 Src2D.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucImgDst, nROIW, nROIH, _T("Bridge Mode2 Dst2D.bmp"));
	m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst_3D, nImgWidth, nImgHeight, rcGapRect.left, rcGapRect.top, nROIW, nROIH);
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Bridge Mode2 Src3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfImgDst_3D, nROIW, nROIH, _T("Bridge Mode2 Dst3D.bmp"));
	// Body Remove
	int nGrayAdd = 0;
	float fHeightAdd = 0.0f;
	int nCnt = 0;
	int nLineCnt = 0;
	int nRemoveBodyGray = nBridge2DValue * 1.2;
	if (nRemoveBodyGray > 255) nRemoveBodyGray = nBridge2DValue;
	double dGap = 0.8;
	int nLineMinCnt = 2;
	double dBodyMinCnt = 0.08;
	double dBodyOut = 0.3;
	int nRemoveBody = 2;
	float fHeightMeanMin = 0.5f;
	if (nDir == 0 || nDir == 1)
		nRemoveBody = nROIW * 0.15;
	else
		nRemoveBody = nROIH * 0.15;
	double dGrayMeanMin = 0.8;
	if (nDir == 0)
	{
		for (int nX = nROIW - 1; nX > (nROIW - 1) * dGap; nX--)
		{
			int nCnt2 = 0;
			for (int nY = 0; nY < nROIH; nY++)
			{
				int nIndex = (nY * nROIW) + nX;
				if (pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] > f3DSetValue)
				{
					nGrayAdd += pucImgDst[nIndex];
					fHeightAdd += pfImgDst_3D[nIndex];
					nCnt++;
					nCnt2++;
				}
			}
			if (nCnt2 > nROIH - nLineMinCnt)
				nLineCnt++;
		}
		if (nLineCnt >= (int)(nROIW * dBodyMinCnt) && nCnt > 0)
		{
			int nGrayMean = nGrayAdd / nCnt;
			float fHeightMean = fHeightAdd / (float)nCnt * fHeightMeanMin;
			for (int nX = nROIW - 1; nX >= 0; nX--)
			{
				nCnt = 0;
				for (int nY = 0; nY < nROIH; nY++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (nX + 1 < nROIW && pucImgDst[(nY * nROIW) + (nX + 1)] > 0)
						continue;
					if (pucImgDst[nIndex] >= nGrayMean && pfImgDst_3D[nIndex] > fHeightMean)
					{
						pucImgDst[nIndex] = 0;
						nCnt++;
					}
				}
				if (nCnt <= nROIH * dBodyOut)
					break;
			}
			m_pProcMilAlgo->SaveWorkImg(pucImgDst, nROIW, nROIH, _T("Bridge Mode2 Dst2D 1.bmp"));
			for (int nY = 0; nY < nROIH; nY++)
			{
				for (int nX = nROIW - 1; nX >= 0; nX--)
				{
					int nIndex = (nY * nROIW) + nX;
					if (pucImgDst[nIndex] == 0)
						continue;
					for (int nL = 0; nL < nRemoveBody; nL++)
					{
						if (nX - nL <= 0) break;
						nIndex = (nY * nROIW) + (nX - 1);
						if (pucImgDst[nIndex] >= (nGrayMean * dGrayMeanMin) ||
							(pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] >= fHeightMean))
							pucImgDst[nIndex] = 0;
						else
						{
							pucImgDst[nIndex] = 0;
							if (nX - nL - 1 >= 0)
							{
								nIndex = (nY * nROIW) + (nX - nL - 1);
								pucImgDst[nIndex] = 0;
							}
							break;
						}
					}
					break;
				}
			}
		}
	}
	else if (nDir == 1)
	{
		for (int nX = 0; nX < nROIW * (1.0 - dGap); nX++)
		{
			int nCnt2 = 0;
			for (int nY = 0; nY < nROIH; nY++)
			{
				int nIndex = (nY * nROIW) + nX;
				if (pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] > f3DSetValue)
				{
					nGrayAdd += pucImgDst[nIndex];
					fHeightAdd += pfImgDst_3D[nIndex];
					nCnt++;
					nCnt2++;
				}
			}
			if (nCnt2 > nROIH - nLineMinCnt)
				nLineCnt++;
		}
		if (nLineCnt >= (int)(nROIW * dBodyMinCnt) && nCnt > 0)
		{
			int nGrayMean = nGrayAdd / nCnt;
			float fHeightMean = fHeightAdd / (float)nCnt * fHeightMeanMin;
			for (int nX = 0; nX < nROIW; nX++)
			{
				nCnt = 0;
				for (int nY = 0; nY < nROIH; nY++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (nX - 1 > 0 && pucImgDst[(nY * nROIW) + (nX - 1)] > 0)
						continue;
					if (pucImgDst[nIndex] >= nGrayMean && pfImgDst_3D[nIndex] > fHeightMean)
					{
						pucImgDst[nIndex] = 0;
						nCnt++;
					}
				}
				if (nCnt <= nROIH * dBodyOut)
					break;
			}
			m_pProcMilAlgo->SaveWorkImg(pucImgDst, nROIW, nROIH, _T("Bridge Mode2 Dst2D 1.bmp"));
			for (int nY = 0; nY < nROIH; nY++)
			{
				for (int nX = 0; nX < nROIW; nX++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (pucImgDst[nIndex] == 0)
						continue;
					for (int nL = 0; nL < nRemoveBody; nL++)
					{
						if (nX + nL >= nROIW) break;
						nIndex = (nY * nROIW) + (nX + nL);
						if (pucImgDst[nIndex] >= (nGrayMean * dGrayMeanMin) ||
							(pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] >= fHeightMean))
							pucImgDst[nIndex] = 0;
						else
						{
							pucImgDst[nIndex] = 0;
							if (nX + nL + 1 < nROIW)
							{
								nIndex = (nY * nROIW) + (nX + nL + 1);
								pucImgDst[nIndex] = 0;
							}
							break;
						}
					}
					break;
				}
			}
		}
	}
	else if (nDir == 2)
	{
		for (int nY = nROIH - 1; nY > (nROIH - 1) * dGap; nY--)
		{
			int nCnt2 = 0;
			for (int nX = 0; nX < nROIW; nX++)
			{
				int nIndex = (nY * nROIW) + nX;
				if (pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] > f3DSetValue)
				{
					nGrayAdd += pucImgDst[nIndex];
					fHeightAdd += pfImgDst_3D[nIndex];
					nCnt++;
					nCnt2++;
				}
			}
			if (nCnt2 > nROIW - nLineMinCnt)
				nLineCnt++;
		}
		if (nLineCnt >= (int)(nROIH * dBodyMinCnt) && nCnt > 0)
		{
			int nGrayMean = nGrayAdd / nCnt;
			float fHeightMean = fHeightAdd / (float)nCnt * fHeightMeanMin;
			for (int nY = nROIH - 1; nY >= 0; nY--)
			{
				nCnt = 0;
				for (int nX = 0; nX < nROIW; nX++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (nY + 1 < nROIH && pucImgDst[((nY + 1) * nROIW) + nX] > 0)
						continue;
					if (pucImgDst[nIndex] >= nGrayMean && pfImgDst_3D[nIndex] > fHeightMean)
					{
						pucImgDst[nIndex] = 0;
						nCnt++;
					}
				}
				if (nCnt <= nROIW * dBodyOut)
					break;
			}
			m_pProcMilAlgo->SaveWorkImg(pucImgDst, nROIW, nROIH, _T("Bridge Mode2 Dst2D 1.bmp"));
			for (int nX = 0; nX < nROIW; nX++)
			{
				for (int nY = nROIH - 1; nY >= 0; nY--)
				{
					int nIndex = (nY * nROIW) + nX;
					if (pucImgDst[nIndex] == 0)
						continue;
					for (int nL = 0; nL < nRemoveBody; nL++)
					{
						if (nY - nL < 0) break;
						nIndex = ((nY - nL) * nROIW) + nX;
						if (pucImgDst[nIndex] >= (nGrayMean * dGrayMeanMin) ||
							(pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] >= fHeightMean))
							pucImgDst[nIndex] = 0;
						else
						{
							pucImgDst[nIndex] = 0;
							if (nY - nL - 1 >= 0)
							{
								nIndex = ((nY - nL - 1) * nROIW) + nX;
								pucImgDst[nIndex] = 0;
							}
							break;
						}
					}
					break;
				}
			}
		}
	}
	else if (nDir == 3)
	{
		for (int nY = 0; nY < nROIH * (1.0 - dGap); nY++)
		{
			int nCnt2 = 0;
			for (int nX = 0; nX < nROIW; nX++)
			{
				int nIndex = (nY * nROIW) + nX;
				if (pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] > f3DSetValue)
				{
					nGrayAdd += pucImgDst[nIndex];
					fHeightAdd += pfImgDst_3D[nIndex];
					nCnt++;
					nCnt2++;
				}
			}
			if (nCnt2 > nROIW - nLineMinCnt)
				nLineCnt++;
		}
		if (nLineCnt >= (int)(nROIH * dBodyMinCnt) && nCnt > 0)
		{
			int nGrayMean = nGrayAdd / nCnt;
			float fHeightMean = fHeightAdd / (float)nCnt * fHeightMeanMin;
			for (int nY = 0; nY < nROIH; nY++)
			{
				nCnt = 0;
				for (int nX = 0; nX < nROIW; nX++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (nY - 1 > 0 && pucImgDst[((nY - 1) * nROIW) + nX] > 0)
						continue;
					if (pucImgDst[nIndex] >= nGrayMean && pfImgDst_3D[nIndex] > fHeightMean)
					{
						pucImgDst[nIndex] = 0;
						nCnt++;
					}
				}
				if (nCnt <= nROIW * dBodyOut)
					break;
			}
			m_pProcMilAlgo->SaveWorkImg(pucImgDst, nROIW, nROIH, _T("Bridge Mode2 Dst2D 1.bmp"));
			for (int nX = 0; nX < nROIW; nX++)
			{
				for (int nY = 0; nY < nROIH; nY++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (pucImgDst[nIndex] == 0)
						continue;
					for (int nL = 0; nL < nRemoveBody; nL++)
					{
						if (nY + nL >= nROIH) break;
						nIndex = ((nY + nL) * nROIW) + nX;
						if (pucImgDst[nIndex] >= (nGrayMean * dGrayMeanMin) ||
							(pucImgDst[nIndex] >= nRemoveBodyGray && pfImgDst_3D[nIndex] >= fHeightMean))
							pucImgDst[nIndex] = 0;
						else
						{
							pucImgDst[nIndex] = 0;
							if (nY + nL + 1 < nROIH)
							{
								nIndex = ((nY + nL + 1) * nROIW) + nX;
								pucImgDst[nIndex] = 0;
							}
							break;
						}
					}
					break;
				}
			}
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pucImgDst, nROIW, nROIH, _T("Bridge Mode2 Dst2D 2.bmp"));
	// Create Bin
	for (int nX = 0; nX < nROIW; nX++)
	{
		for (int nY = 0; nY < nROIH; nY++)
		{
			int nIndex = (nY * nROIW) + nX;
			if (pucImgDst[nIndex] > nBridge2DValue && pfImgDst_3D[nIndex] > f3DSetValue)
				pucImgDst_Bin[nIndex] = 255;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pucImgDst_Bin, nROIW, nROIH, _T("Bridge Mode2 Bin.bmp"));
	// Thin Remove
	if (nThinBridge > 0)
	{
		if (nDir == 0 || nDir == 1)
		{
			for (int nY = 0; nY < nROIH; nY++)
			{
				for (int nX = 0; nX < nROIW; nX++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (pucImgDst_Bin[nIndex] != 255)
						continue;
					// Prev
					int nBuf = nX - 1;
					if (nBuf >= 0)
					{
						nIndex = (nY * nROIW) + nBuf;
						if (pucImgDst_Bin[nIndex] == 255)
							continue;
					}
					//Next
					nBuf = nX + nThinBridge;
					if (nBuf < nROIH)
					{
						nIndex = (nY * nROIW) + nBuf;
						if (pucImgDst_Bin[nIndex] == 255)
							continue;
					}
					//Thin
					for (int nT = 0; nT < nThinBridge; nT++)
					{
						nBuf = nX + nT;
						if (nBuf < nROIH)
						{
							nIndex = (nY * nROIW) + nBuf;
							pucImgDst_Bin[nIndex] = 0;
						}
					}
				}
			}
		}
		else
		{
			for (int nX = 0; nX < nROIW; nX++)
			{
				for (int nY = 0; nY < nROIH; nY++)
				{
					int nIndex = (nY * nROIW) + nX;
					if (pucImgDst_Bin[nIndex] != 255)
						continue;
					// Prev
					int nBuf = nY - 1;
					if (nBuf >= 0)
					{
						nIndex = (nBuf * nROIW) + nX;
						if (pucImgDst_Bin[nIndex] == 255)
							continue;
					}
					//Next
					nBuf = nY + nThinBridge;
					if (nBuf < nROIH)
					{
						nIndex = (nBuf * nROIW) + nX;
						if (pucImgDst_Bin[nIndex] == 255)
							continue;
					}
					//Thin
					for (int nT = 0; nT < nThinBridge; nT++)
					{
						nBuf = nY + nT;
						if (nBuf < nROIH)
						{
							nIndex = (nBuf * nROIW) + nX;
							pucImgDst_Bin[nIndex] = 0;
						}
					}
				}
			}
		}
	}
	/*double *dRstArea = new double[3];
	double *dRstCx = new double[3];
	double *dRstCy = new double[3];
	CRect *cBlobRect = new CRect[3];*/
	double *dRstArea = g_pMManager->pem_new<double>(true, 3, (PCHAR)__FUNCTION__, __LINE__);
	double *dRstCx = g_pMManager->pem_new<double>(true, 3, (PCHAR)__FUNCTION__, __LINE__);
	double *dRstCy = g_pMManager->pem_new<double>(true, 3, (PCHAR)__FUNCTION__, __LINE__);
	CRect *cBlobRect = g_pMManager->pem_new<CRect>(true, 3, (PCHAR)__FUNCTION__, __LINE__);
	m_pProcMilAlgo->SaveWorkImg(pucImgDst_Bin, nROIW, nROIH, _T("Bridge Mode2 Rst Bin.bmp"));
	if (pucRstMode2 != NULL)
	{
		for (int nX = 0; nX < nROIW; nX++)
		{
			for (int nY = 0; nY < nROIH; nY++)
			{
				int nIndex = (nY * nROIW) + nX;
				int nIndex2 = ((nY + rcGapRect.top) * nImgWidth) + (nX + rcGapRect.left);
				pucRstMode2[nIndex2] = pucImgDst_Bin[nIndex];
			}
		}
		m_pProcMilAlgo->SaveWorkImg(pucRstMode2, nImgWidth, nImgHeight, _T("Bridge Mode2 Full Rst Bin.bmp"));
	}
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pucImgDst_Bin, NULL, nROIW, nROIH, 5, FALSE, FALSE, 0, eSelectMix);
	if (nCntBlob > 0)
	{
		if (nCntBlob > 3) nCntBlob = 3;
		m_pProcMilAlgo->GetBlobResult(dRstArea, dRstCx, dRstCy, cBlobRect, TRUE);
		for (int nMode2Idx = 0; nMode2Idx < nCntBlob; nMode2Idx++)
		{
			if ((nDir == 0) || (nDir == 1))
			{
				if (cBlobRect[nMode2Idx].Height() == nROIH || (cBlobRect[nMode2Idx].Height() + 1) == nROIH)
				{
					*nRstSize = cBlobRect[nMode2Idx].Height();
					bResult = false;
					break;
				}
				else
					bResult = true;
			}
			else
			{
				if (cBlobRect[nMode2Idx].Width() == nROIW || (cBlobRect[nMode2Idx].Width() + 1) == nROIW)
				{
					*nRstSize = cBlobRect[nMode2Idx].Width();
					bResult = false;
					break;
				}
				else
					bResult = true;
			}
		}
	}
	else
		bResult = true;
	Delete_1DArray(&pucImgDst_Bin);
	Delete_1DArray(&dRstArea);
	Delete_1DArray(&dRstCx);
	Delete_1DArray(&dRstCy);
	Delete_1DArray(&cBlobRect);
	Delete_1DArray(&pucImgDst);
	Delete_1DArray(&pfImgDst_3D);

	return bResult;
}

BOOL CPInsp_AlgoBridge::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoBridge::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoBridge * pInspAlgo = (AlgoBridge *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->m_sBlobBase.m_sAlgoColorBase.GetColorData();
	nData |= COLOR_DATA_WND;

	return nData;
}
bool CPInsp_AlgoBridge::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}