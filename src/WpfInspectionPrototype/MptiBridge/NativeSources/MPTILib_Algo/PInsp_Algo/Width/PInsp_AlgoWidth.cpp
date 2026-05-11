#include "PInsp_AlgoWidth.h"


CPInsp_AlgoWidth::CPInsp_AlgoWidth(void)
{
}


CPInsp_AlgoWidth::~CPInsp_AlgoWidth(void)
{
}

void CPInsp_AlgoWidth::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoWidth::GetInspAlgoData()
{
	return eSPCAlgoWidth;
}

int CPInsp_AlgoWidth::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	if (nWndType == eINSP_SOLDER)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}
	else
	{
		eWholeNgTypeTemp[TypeLeadSolder] = e_NG;
		eWholeNgType[TypeLeadSolder] = e_NG;
		nCurrentNgType = TypeLeadSolder;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoWidth::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return true;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return true;
	return true;
}

BOOL CPInsp_AlgoWidth::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspWidth(sInspAlgo, sWndAlgoImg, (RstAlgoLength *)sRstAlgo, stAlgoParam.m_nLeadTipPos, ucArrDstImg);

	if (stAlgoParam.m_bInspection)
	{
		RstAlgoLength * sRst = (RstAlgoLength *)sRstAlgo;
		for (int nIdx = 0; nIdx < 2; nIdx++)
		{
			sRst->m_poDrawLine_I[nIdx].x += stAlgoParam.m_dx;
			sRst->m_poDrawLine_I[nIdx].y += stAlgoParam.m_dy;
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoWidth::InspWidth(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoLength *sRstAlgo, int nLeadTipPos/*=-1*/, UCHAR* ucArrDstImg /*= NULL*/)
{
	BOOL bResult = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, (sizeof(RstAlgoLength)));
		sRstAlgo->Init();
	}

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	if (sInspAlgo.m_eAlgoType != eAlgoWidth)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	AlgoLength *pInspAlgoLength = (AlgoLength *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoLength)
		return bResult;
	if (!pInspAlgoLength->m_b2dCheck && !pInspAlgoLength->m_b3dCheck)
		return bResult;

	int nSizeImg = nImgWidth * nImgHeight;
	//UCHAR* pUcArrDstImg = new UCHAR[nSizeImg];
	UCHAR* pUcArrDstImg = g_pMManager->pem_new<UCHAR>(true, nSizeImg, (PCHAR)__FUNCTION__, __LINE__);
	memset(pUcArrDstImg, 0, sizeof(UCHAR) * nSizeImg);
	double dRate = m_pCPInsp_Algo->GetBWImageStruct(*pInspAlgoLength, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, pUcArrDstImg);

	double dWidthSolder = 0.0;
	int nTeach = 0;
	int nEndX = 0;
	int nEndY = 0;
	if (dRate > 0)
	{
		int nIdxSttX = 0;
		int nIdxEndX = nImgWidth;
		int nIdxSttY = 0;
		int nIdxEndY = nImgHeight;

		double dYieldRate = pInspAlgoLength->m_dYieldPos / 100;
		double dLeadPos = (nLeadTipPos < 0) ? pInspAlgoLength->m_nLeadPosition : nLeadTipPos;
		if (dLeadPos < 0)
			dLeadPos = 0.0;

		if (pInspAlgoLength->m_nTipDirection == m_pCPInsp_Algo->EDirection::eTop)
		{
			int y = (nImgHeight - 1 - dLeadPos) - ((nImgHeight - 1 - dLeadPos) * dYieldRate);
			nTeach = y;
			nEndX = nIdxSttX;
			nEndY = nIdxEndX;
			int nCntValid = 0;
			for (int x = nIdxSttX; x < nIdxEndX; x += 1)
			{
				if (pUcArrDstImg[(nImgWidth*y) + x] > 1)
					++nCntValid;
			}
			dWidthSolder = nCntValid * m_pCPInsp_Algo->GetResolX();
		}
		else if (pInspAlgoLength->m_nTipDirection == m_pCPInsp_Algo->EDirection::eBottom)
		{
			int y = (nImgHeight - 1 - dLeadPos) * dYieldRate + dLeadPos;
			nTeach = y;
			nEndX = nIdxSttX;
			nEndY = nIdxEndX;
			int nCntValid = 0;
			for (int x = nIdxSttX; x < nIdxEndX; x += 1)
			{
				if (pUcArrDstImg[(nImgWidth*y) + x] > 1)
					++nCntValid;
			}
			dWidthSolder = nCntValid * m_pCPInsp_Algo->GetResolX();
		}
		else if (pInspAlgoLength->m_nTipDirection == m_pCPInsp_Algo->EDirection::eLeft)
		{
			int x = (nImgWidth - 1 - dLeadPos) - ((nImgWidth - 1 - dLeadPos) * dYieldRate);
			nTeach = x;
			nEndX = nIdxSttY;
			nEndY = nIdxEndY;
			int nCntValid = 0;
			for (int y = nIdxSttY; y < nIdxEndY; y += 1)
			{
				if (pUcArrDstImg[(nImgWidth*y) + x] > 1)
					++nCntValid;
			}
			dWidthSolder = nCntValid * m_pCPInsp_Algo->GetResolY();
		}
		else if (pInspAlgoLength->m_nTipDirection == m_pCPInsp_Algo->EDirection::eRight)
		{
			int x = (nImgWidth - 1 - dLeadPos) * dYieldRate + dLeadPos;
			nTeach = x;
			nEndX = nIdxSttY;
			nEndY = nIdxEndY;
			int nCntValid = 0;
			for (int y = nIdxSttY; y < nIdxEndY; y += 1)
			{
				if (pUcArrDstImg[(nImgWidth*y) + x] > 1)
					++nCntValid;
			}
			dWidthSolder = nCntValid * m_pCPInsp_Algo->GetResolY();
		}
	}

	bResult = TRUE;
	if (sRstAlgo)
	{
		if (pInspAlgoLength->m_nTipDirection == m_pCPInsp_Algo->EDirection::eLeft || pInspAlgoLength->m_nTipDirection == m_pCPInsp_Algo->EDirection::eRight)
		{
			sRstAlgo->m_poDrawLine_I[0].x = nTeach;
			sRstAlgo->m_poDrawLine_I[0].y = nEndX;
			sRstAlgo->m_poDrawLine_I[1].x = nTeach;
			sRstAlgo->m_poDrawLine_I[1].y = nEndY;
		}
		else
		{
			sRstAlgo->m_poDrawLine_I[0].x = nEndX;
			sRstAlgo->m_poDrawLine_I[0].y = nTeach;
			sRstAlgo->m_poDrawLine_I[1].x = nEndY;
			sRstAlgo->m_poDrawLine_I[1].y = nTeach;
		}
		sRstAlgo->m_dRstLength = dWidthSolder;
		sRstAlgo->m_dRstPercent = (pInspAlgoLength->m_dStdLength > 0)
			? (sRstAlgo->m_dRstLength / pInspAlgoLength->m_dStdLength) * 100
			: 0.0;

		if ((pInspAlgoLength->m_dStdLength * pInspAlgoLength->m_dPercentOK) / 100.0 > sRstAlgo->m_dRstLength)
			bResult = FALSE;
	}

	if (!ucArrDstImg)
		Delete_1DArray(&pUcArrDstImg);
	else if (pUcArrDstImg)
	{
		Delete_1DArray(&ucArrDstImg);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrDstImg, nSizeImg);
		memcpy_s(ucArrDstImg, sizeof(UCHAR) * nSizeImg, pUcArrDstImg, sizeof(UCHAR) * nSizeImg);
	}

	return bResult;
}

BOOL CPInsp_AlgoWidth::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoWidth::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoWidth::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}