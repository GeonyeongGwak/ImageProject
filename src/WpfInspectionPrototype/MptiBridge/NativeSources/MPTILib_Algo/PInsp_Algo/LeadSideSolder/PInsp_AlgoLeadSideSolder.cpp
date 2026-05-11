#include "PInsp_AlgoLeadSideSolder.h"


CPInsp_AlgoLeadSideSolder::CPInsp_AlgoLeadSideSolder(void)
{
}


CPInsp_AlgoLeadSideSolder::~CPInsp_AlgoLeadSideSolder(void)
{
}

void CPInsp_AlgoLeadSideSolder::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoLeadSideSolder::GetInspAlgoData()
{
	return eSPCAlgoLeadSideSolder;
}

int CPInsp_AlgoLeadSideSolder::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeLeadSolder] = e_NG;
	eWholeNgType[TypeLeadSolder] = e_NG;
	nCurrentNgType = TypeLeadSolder;
	return nCurrentNgType;
}

bool CPInsp_AlgoLeadSideSolder::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return true;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoLeadSideSolder::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspSideSolder(sInspAlgo, sWndAlgoImg, *sInspImageData, ucArrDstImg, (RstAlgoLeadSideSolder *)sRstAlgo, stAlgoParam.m_nLeadTipPos, stAlgoParam.m_ucArrDstImgAC);

	if (stAlgoParam.m_bInspection)
	{
		RstAlgoLeadSideSolder * sRst = (RstAlgoLeadSideSolder *)sRstAlgo;
		for (int nIdx = 0; nIdx < 3; nIdx++)
		{
			sRst->m_rcRect_I[nIdx].left += stAlgoParam.m_dx;
			sRst->m_rcRect_I[nIdx].right += stAlgoParam.m_dx;
			sRst->m_rcRect_I[nIdx].top += stAlgoParam.m_dy;
			sRst->m_rcRect_I[nIdx].bottom += stAlgoParam.m_dy;
		}
	}
	return bResult;
}

BOOL CPInsp_AlgoLeadSideSolder::InspSideSolder(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImgBuf, UCHAR* ucArrDstImg, RstAlgoLeadSideSolder * sRstAlgo, int nLeadTipPos, UCHAR* ucArrDstImgAC)
{
	BOOL bResult = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoLeadSideSolder));
		sRstAlgo->Init();
		sRstAlgo->m_sInspAC.m_nInspAC = 0;
	}

	if (sInspAlgo.m_eAlgoType != eAlgoLead_SideSolder)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D_part;
	if (pfImgSrc == NULL)
		pfImgSrc = sWndAlgoImg.m_fArr3D;

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	AlgoLeadSideSolder *pInspAlgo = (AlgoLeadSideSolder *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgo)
		return bResult;

	// Auto 검사 시, Lead Tip 검사 결과에 따른 값을 적용 (nLeadTipPos)
	// Auto 검사 시 Lead Tip Algorithm이 없는 경우, pInspAlgoLeadLift->nLeadPosition 값을 적용 (nLeadTipPos = -1)
	// Teaching 시 Lead pInspAlgoLeadSolder->nLeadPosition 값을 적용	(nLeadTipPos = -1)
	if (nLeadTipPos < 0)
		nLeadTipPos = pInspAlgo->m_nLeadPosition;

	int nLeadDir = pInspAlgo->m_nLeadTipDirection;
	double dLeadLiftSetValue = pInspAlgo->m_dLeadLiftSetValue;

	int nInspAC = pInspAlgo->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data];
	bool bInspAC = ((nInspAC & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
	bool bInspAC_Ess = (bInspAC && ((nInspAC & eDefaultAC_Data_Essential) == eDefaultAC_Data_Essential));
	bool bInspAC_WA = (bInspAC && ((nInspAC & eDefaultAC_Data_WA) == eDefaultAC_Data_WA));
	int nUseColor = pInspAlgo->sAlgoBW.m_sAlgoColorBase.m_bUseColor ? 0 : 255;
	cv::Mat imgAC(nImgHeight, nImgWidth, CV_8UC3, cv::Scalar(0));
	cv::Mat imgColor(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
	if (ucArrDstImg)
		memset(ucArrDstImg, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
	if (ucArrDstImgAC)
		memset(ucArrDstImgAC, 0, nImgWidth * nImgHeight * 3 * sizeof(UCHAR));

	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (pColorTeach)
	{
		bool bTeach = (ucArrDstImg != NULL);
		if (pInspAlgo->sAlgoBW.m_sAlgoColorBase.m_bUseColor)
			pColorTeach->GetColorBaseBin(&pInspAlgo->sAlgoBW.m_sAlgoColorBase, sInspImgBuf, imgColor.data, bTeach);
		if (bInspAC)
		{
			AlgoColorBase sColorAlgo;
			sColorAlgo.m_bUseColor = FALSE;
			pColorTeach->GetColorBaseBin(&sColorAlgo, sInspImgBuf, NULL, bTeach, nInspAC, imgAC.data);
		}
	}

	RECT rcInspAreaRect;
	int nWhitePixelCnt = 0;
	int nInspAreaCnt = 0;
	int nInspTotalCnt = _nSideSolderRelCnt;
	if (pInspAlgo->m_bUseTeachArea == true)
		nInspTotalCnt = 1;
	for (int i = 0; i < nInspTotalCnt; i++)
	{
		if (pInspAlgo->m_bUseTeachArea == false && pInspAlgo->m_byInspArea > 0 && pInspAlgo->m_byInspArea <= 2)
		{
			byte byIdx = (pInspAlgo->m_byInspArea == 1) ? 1 : 0;
			if (i == byIdx)
				continue;
		}
		if (pInspAlgo->m_nRectWidth[i] <= 0 || pInspAlgo->m_nRectHeight[i] <= 0)
			continue;
		nInspAreaCnt += pInspAlgo->m_nRectWidth[i] * pInspAlgo->m_nRectHeight[i];
		int nStartX = pInspAlgo->m_ptStart[i].x;
		int nStartY = pInspAlgo->m_ptStart[i].y;
		int nWndStartX = 0;
		int nWndStartY = 0;
		int nWndW = 0;
		int nWndH = 0;
		int nInspAreaW = pInspAlgo->m_nRectWidth[i];
		int nInspAreaH = pInspAlgo->m_nRectHeight[i];

		if (nLeadTipPos <= 0)
			nLeadTipPos = 0;
		int nRealTipPos = pInspAlgo->m_nLeadPosition - nLeadTipPos;
		if (nLeadDir == 0)			// L
			nStartX += nRealTipPos;
		else if (nLeadDir == 1)		// R
			nStartX -= nRealTipPos;
		else if (nLeadDir == 2)		// T
			nStartY += nRealTipPos;
		else if (nLeadDir == 3)		// B
			nStartY -= nRealTipPos;

		if (nStartX < 0)
			nStartX = 0;
		if (nStartY < 0)
			nStartY = 0;

		UCHAR *pucSolderImg = NULL;
		float *pfSolderImg = NULL;
		UCHAR* pUcArrDstImg = NULL;

		if (nInspAreaW <= 0)
			nInspAreaW = nImgWidth;
		if (nInspAreaH <= 0)
			nInspAreaH = nImgHeight;
		if (nStartX < 0)
			nStartX = 0;
		if (nStartY < 0)
			nStartY = 0;
		if (nStartX + nInspAreaW > nImgWidth)
			nStartX = nImgWidth - nInspAreaW;
		if (nStartY + nInspAreaH > nImgHeight)
			nStartY = nImgHeight - nInspAreaH;

		if (pInspAlgo->m_bUseTeachArea == true)
		{
			if (nLeadDir == 0)			// L	
			{
				nWndStartX = pInspAlgo->m_nRectWidth[2];
				nWndStartY = pInspAlgo->m_nRectHeight[1];
				nWndW = pInspAlgo->m_nRectWidth[1];
				nWndH = nInspAreaH - (pInspAlgo->m_nRectHeight[1] * 2);
			}
			else if (nLeadDir == 1)		// R
			{
				nWndStartX = 0;
				nWndStartY = pInspAlgo->m_nRectHeight[1];
				nWndW = pInspAlgo->m_nRectWidth[1];
				nWndH = nInspAreaH - (pInspAlgo->m_nRectHeight[1] * 2);
			}
			else if (nLeadDir == 2)		// T
			{
				nWndStartX = pInspAlgo->m_nRectWidth[1];
				nWndStartY = pInspAlgo->m_nRectHeight[2];
				nWndW = nInspAreaW - (pInspAlgo->m_nRectWidth[1] * 2);
				nWndH = pInspAlgo->m_nRectHeight[1];
			}
			else if (nLeadDir == 3)		// B
			{
				nWndStartX = pInspAlgo->m_nRectWidth[1];
				nWndStartY = 0;
				nWndW = nInspAreaW - (pInspAlgo->m_nRectWidth[1] * 2);
				nWndH = pInspAlgo->m_nRectHeight[1];
			}
		}
		if (sRstAlgo)
		{
			sRstAlgo->m_rcRect_I[i].left = nStartX;
			sRstAlgo->m_rcRect_I[i].top = nStartY;
			sRstAlgo->m_rcRect_I[i].right = nStartX + nInspAreaW;
			sRstAlgo->m_rcRect_I[i].bottom = nStartY + nInspAreaH;
		}

		//pfSolderImg	= new float[nInspAreaW * nInspAreaH]; 
		pfSolderImg = g_pMManager->pem_new<float>(true, nInspAreaW * nInspAreaH, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nStartX, nStartY, nInspAreaW, nInspAreaH);
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("LeadSideSolder3D_ORG.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfSolderImg, nInspAreaW, nInspAreaH, _T("LeadSideSolder3D.bmp"));

		//pucSolderImg	= new UCHAR[nInspAreaW * nInspAreaH]; 
		pucSolderImg = g_pMManager->pem_new<UCHAR>(true, nInspAreaW * nInspAreaH, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucSolderImg, nStartX, nStartY, nInspAreaW, nInspAreaH);
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("LeadSideSolder2D_ORG.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucSolderImg, nInspAreaW, nInspAreaH, _T("LeadSideSolder2D.bmp"));

		cv::Mat imgColorTemp(nInspAreaH, nInspAreaW, CV_8UC1, cv::Scalar(nUseColor));
		if (pInspAlgo->sAlgoBW.m_sAlgoColorBase.m_bUseColor)
			m_pProcMilAlgo->GetClipImage_LT(imgColor.data, nImgWidth, nImgHeight, imgColorTemp.data, nStartX, nStartY, nInspAreaW, nInspAreaH);

		if (pInspAlgo->m_bUseTeachArea == true)
		{
			if (nWndStartX >= 0 && nWndW > 0 && nWndStartX < nInspAreaW && nWndStartX + nWndW < nInspAreaW &&
				nWndStartY >= 0 && nWndH > 0 && nWndStartY < nInspAreaH && nWndStartY + nWndH < nInspAreaH)
				imgColorTemp(cv::Rect(nWndStartX, nWndStartY, nWndW, nWndH)).setTo(0);
		}

		AlgoBlackWhite algoBw = pInspAlgo->sAlgoBW;
		//pUcArrDstImg = new UCHAR[nInspAreaW * nInspAreaH];
		pUcArrDstImg = g_pMManager->pem_new<UCHAR>(true, nInspAreaW * nInspAreaH, (PCHAR)__FUNCTION__, __LINE__);
		double dRetRate = m_pCPInsp_Algo->GetBWImageStruct(algoBw, pucSolderImg, pfSolderImg, nInspAreaW, nInspAreaH, pUcArrDstImg, true, imgColorTemp.data);
		if (pInspAlgo->m_bUseTeachArea == false || (pInspAlgo->m_bUseTeachArea == true && i == 0))
			nWhitePixelCnt += dRetRate / 100.0 * nInspAreaW * nInspAreaH;
		else
			nWhitePixelCnt -= dRetRate / 100.0 * nInspAreaW * nInspAreaH;
		m_pProcMilAlgo->SaveWorkImg(pUcArrDstImg, nInspAreaW, nInspAreaH, _T("LeadSideSolder.bmp"));

#if _DEBUG
		cv::Mat imgArrDstImg;
		cv::Mat imgArrDstImgAC;
		if (pUcArrDstImg)
			imgArrDstImg = cv::Mat(nInspAreaH, nInspAreaW, CV_8UC1, pUcArrDstImg);
		if (ucArrDstImgAC)
			imgArrDstImgAC = cv::Mat(nImgHeight, nImgWidth, CV_8UC3, ucArrDstImgAC);
#endif

		if (bInspAC || ucArrDstImg)
		{
			cv::Mat imgAC_Temp(nInspAreaH, nInspAreaW, CV_8UC3, cv::Scalar(0));
			for (int nY = nStartY; nY < nStartY + nInspAreaH; nY++)
			{
				if (nY < 0 || nY >= nImgHeight)
					continue;

				for (int nX = nStartX; nX < nStartX + nInspAreaW; nX++)
				{
					if (nX < 0 || nX >= nImgWidth)
						continue;

					if (pInspAlgo->m_bUseTeachArea == true)
					{
						if (nY >= (nStartY + nWndStartY) && nY < (nStartY + nWndStartY) + nWndH)
						{
							if (nX >= (nStartX + nWndStartX) && nX < (nStartX + nWndStartX) + nWndW)
								continue;
						}
					}

					int nX_Temp = nX - nStartX;
					int nY_Temp = nY - nStartY;
					int nIdx_Temp = (nY_Temp * nInspAreaW) + nX_Temp;
					int nIdx = (nY * nImgWidth) + nX;

					if (ucArrDstImg)
						ucArrDstImg[nIdx] = pUcArrDstImg[nIdx_Temp];

					if (bInspAC == false)
						continue;

					if (bInspAC_WA)
					{
						if (pUcArrDstImg[nIdx_Temp] != 255)
							continue;
					}

					imgAC_Temp.data[nIdx_Temp * 3] = imgAC.data[nIdx * 3];
					imgAC_Temp.data[(nIdx_Temp * 3) + 1] = imgAC.data[(nIdx * 3) + 1];
					imgAC_Temp.data[(nIdx_Temp * 3) + 2] = imgAC.data[(nIdx * 3) + 2];
					if (ucArrDstImgAC)
					{
						ucArrDstImgAC[nIdx * 3] = imgAC.data[nIdx * 3];
						ucArrDstImgAC[(nIdx * 3) + 1] = imgAC.data[(nIdx * 3) + 1];
						ucArrDstImgAC[(nIdx * 3) + 2] = imgAC.data[(nIdx * 3) + 2];
					}

					if (sRstAlgo)
						InspAC(imgAC_Temp.data, nIdx_Temp, sRstAlgo->m_sInspAC.m_fRstInspAC);
				}
			}
			if (sRstAlgo && bInspAC)
				sRstAlgo->m_sInspAC.m_nInspAC = InspAC(nInspAC, sRstAlgo->m_sInspAC.m_fRstInspAC, true);
		}

		if (pucSolderImg)
		{
			//delete [] pucSolderImg;
			g_pMManager->pem_delete(pucSolderImg, true);
			pucSolderImg = NULL;
		}
		if (pfSolderImg)
		{
			//delete [] pfSolderImg;
			g_pMManager->pem_delete(pfSolderImg, true);
			pfSolderImg = NULL;
		}
		imgColorTemp.release();
		if (pUcArrDstImg)
		{
			//delete [] pUcArrDstImg;
			g_pMManager->pem_delete(pUcArrDstImg, true);
			pUcArrDstImg = NULL;
		}
	}

	if (sRstAlgo)
	{
		double dPercentOK = 0.0;
		double dRstValue = 0.0;
		if (pInspAlgo->m_bUseTeachArea == false)
		{
			dPercentOK = pInspAlgo->sAlgoBW.m_dPercentOK;
			dRstValue = (nWhitePixelCnt / (double)nInspAreaCnt) * 100.0;
		}
		else
		{
			dPercentOK = pInspAlgo->m_fTeachArea * (pInspAlgo->sAlgoBW.m_dPercentOK / 100.0);
			dRstValue = nWhitePixelCnt * m_resolX * m_resolY;
		}
		if (dPercentOK > dRstValue)
			sRstAlgo->m_bOKBW = FALSE;
		else
			sRstAlgo->m_bOKBW = TRUE;
		sRstAlgo->m_dRstBWPercent = dRstValue;

		sRstAlgo->m_sInspAC.SetRstData(pInspAlgo->m_sAngleColorBase);

		bResult = sRstAlgo->m_bOKBW && sRstAlgo->m_sInspAC.m_nOKInspAC == 0;
		if (bInspAC && bInspAC_Ess)
		{
			bResult = TRUE;
			if (sRstAlgo->m_sInspAC.m_nOKInspAC > 0)
				bResult = FALSE;
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoLeadSideSolder::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoLeadSideSolder::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoLeadSideSolder * pInspAlgo = (AlgoLeadSideSolder *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->sAlgoBW.m_sAlgoColorBase.GetColorData();
	if (pInspAlgo->m_sAngleColorBase.UseAC())
		nData |= COLOR_DATA_AC;
	nData |= COLOR_DATA_WND;

	return nData;
}
bool CPInsp_AlgoLeadSideSolder::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}