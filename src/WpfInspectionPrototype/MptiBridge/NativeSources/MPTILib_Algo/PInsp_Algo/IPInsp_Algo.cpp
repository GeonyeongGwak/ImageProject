#include "IPInsp_Algo.h"


IPINSP_ALGO::IPINSP_ALGO(void)
{
}

IPINSP_ALGO::~IPINSP_ALGO(void)
{
}

#pragma region  Common Func

int IPINSP_ALGO::InspAC(AngleColorBase sAlgo, InspRoiImgBuf &sInspImg, UCHAR *pucBWImage, WndAlgoImg &sWndAlgo, int nDir, UCHAR *pucRstAC, float* fArrRstRGB, RECT *rcArea, bool bTeach, POINTF poCenter)
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	int nInspAC_T = sAlgo.m_nArrInspAC[m_eInspAC_T3_Data];
	if ((nInspAC_T & eDefaultAC_Data_Use) != eDefaultAC_Data_Use ||
		pucRstAC == NULL || fArrRstRGB == NULL || pColorTeach == NULL || rcArea == NULL)
		return 0;
	int nWidth = sWndAlgo.m_nWidth;
	int nHeight = sWndAlgo.m_nHeight;
	memset(fArrRstRGB, 0, m_eInspAC_R_Total * sizeof(float));
	int nWidthStep = nWidth;

	//if ((nWidthStep % 4) != 0) nWidthStep += 4 - (nWidthStep % 4);
	nWidthStep = g_pMPTI->nCalcWidthStep(false, nWidth);

	AlgoColorBase sColorAlgo;
	sColorAlgo.m_bUseColor = FALSE;
	pColorTeach->GetColorBaseBin(&sColorAlgo, sInspImg, NULL, bTeach, nInspAC_T, pucRstAC, -1, sWndAlgo.m_nLight_index);
	m_pProcMilAlgo->SaveWorkImg(pucRstAC, nWidth, nHeight, _T("InspAC_1.bmp"), 3);
	if (sAlgo.m_byArrInspAC[m_eInspAC_T2_InspArea] < 100)
		m_pCPInsp_Algo->GetInspectArea(pucRstAC, nWidth, nHeight, nDir, sAlgo.m_byArrInspAC[m_eInspAC_T2_InspArea], 3);
	m_pProcMilAlgo->SaveWorkImg(pucRstAC, nWidth, nHeight, _T("InspAC_2.bmp"), 3);

	for (int y = 0; y < nHeight; y++)
	{
		for (int x = 0; x < nWidth; x++)
		{
			int nIndex = (y * nWidth) + x;

			if ((nInspAC_T & eDefaultAC_Data_WA) == eDefaultAC_Data_WA && pucBWImage)
			{
				if (pucBWImage[nIndex] != 255)
				{
					pucRstAC[(nIndex * 3)] = 0;
					pucRstAC[(nIndex * 3) + 1] = 0;
					pucRstAC[(nIndex * 3) + 2] = 0;
					continue;
				}
			}

			InspAC(pucRstAC, nIndex, fArrRstRGB);
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pucRstAC, nWidth, nHeight, _T("InspAC_3.bmp"), 3);

	int nInspAC = InspAC(nInspAC_T, fArrRstRGB);
	fArrRstRGB[m_eInspAC_R_CJ] = GetInspACCJ(sAlgo, pucRstAC, sWndAlgo, nInspAC, rcArea, fArrRstRGB, nDir, poCenter);
	for (int a = 0; a < m_eInspAC_R_Total; a++)
		fArrRstRGB[a] = fArrRstRGB[a] * m_resolX * m_resolY;
	return nInspAC;
}
float IPINSP_ALGO::GetInspACCJ(AngleColorBase sAlgo, UCHAR *pucRstAC, WndAlgoImg &sWndAlgo, int nAC, RECT *rcArea, float* fArrRstRGB, int nDir, POINTF poCenter)
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
	float fResult = 0.0f;
	int nWidth = sWndAlgo.m_nWidth;
	int nHeight = sWndAlgo.m_nHeight;
	int nInspAC = sAlgo.m_nArrInspAC[m_eInspAC_T3_Data];
	if (pucRstAC == NULL || nWidth <= 0 || nHeight <= 0 || rcArea == NULL ||
		((nInspAC & eDefaultAC_Data_CJ) != eDefaultAC_Data_CJ))
		return fResult;
	rcArea->left = 0;
	rcArea->right = nWidth;
	rcArea->top = 0;
	rcArea->bottom = nHeight;
	if (sAlgo.m_fArrInspAC[m_eInspAC_T_CJ_X] != 0 || sAlgo.m_fArrInspAC[m_eInspAC_T_CJ_Y] != 0 ||
		sAlgo.m_fArrInspAC[m_eInspAC_T_Range1] > 0 || sAlgo.m_fArrInspAC[m_eInspAC_T_Range2] > 0)
	{
		float fW = RounD(sAlgo.m_fArrInspAC[m_eInspAC_T_Range1] / m_resolX);
		float fH = RounD(sAlgo.m_fArrInspAC[m_eInspAC_T_Range2] / m_resolY);
		if (sWndAlgo.dAngle == 90.0 || sWndAlgo.dAngle == 270.0)
		{
			fW = RounD(sAlgo.m_fArrInspAC[m_eInspAC_T_Range2] / m_resolX);
			fH = RounD(sAlgo.m_fArrInspAC[m_eInspAC_T_Range1] / m_resolY);
		}
		POINTF poDstPos;
		if ((nInspAC & eDefaultAC_Data_CJ_Body) == eDefaultAC_Data_CJ_Body)
		{
			byte byGap = sAlgo.m_byArrInspAC[m_eInspAC_T2_CJ_Gap];
			if (byGap < 0)	byGap = 0;
			if (byGap > 20)	byGap = 20;
			poDstPos.x = poCenter.x;
			poDstPos.y = poCenter.y;
			if (nDir == 0) // L
				poDstPos.x = (nWidth - fW - byGap) + (fW / 2.0f);
			else if (nDir == 1) // R
				poDstPos.x = byGap + (fW / 2.0f);
			else if (nDir == 2) // T
				poDstPos.y = (nHeight - fH - byGap) + (fH / 2.0f);
			else if (nDir == 3) // B
				poDstPos.y = byGap + (fH / 2.0f);
		}
		else
		{
			POINTF poSrcPos;
			poSrcPos.x = sAlgo.m_fArrInspAC[m_eInspAC_T_CJ_X] / m_resolX;
			poSrcPos.y = sAlgo.m_fArrInspAC[m_eInspAC_T_CJ_Y] / m_resolY;
			m_pCPInsp_Algo->ROIAnglePointChange(sWndAlgo.dAngle, poSrcPos, &poDstPos);
			poDstPos.x += (nWidth / 2);
			poDstPos.y += (nHeight / 2);
		}
		rcArea->left = RounD(poDstPos.x - (fW / 2.0f));
		rcArea->right = RounD(rcArea->left + fW);
		rcArea->top = RounD(poDstPos.y - (fH / 2.0f));
		rcArea->bottom = RounD(rcArea->top + fH);
	}
	if (rcArea->left < 0) rcArea->left = 0;
	if (rcArea->right < 0) rcArea->right = 0;
	if (rcArea->top < 0) rcArea->top = 0;
	if (rcArea->bottom < 0) rcArea->bottom = 0;
	if (rcArea->left > nWidth) rcArea->left = nWidth;
	if (rcArea->right > nWidth) rcArea->right = nWidth;
	if (rcArea->top > nHeight) rcArea->top = nHeight;
	if (rcArea->bottom > nHeight) rcArea->bottom = nHeight;
	for (int y = rcArea->top; y < rcArea->bottom; y++)
	{
		for (int x = rcArea->left; x < rcArea->right; x++)
		{
			int nIndex = (y * nWidth) + x;
			if (pucRstAC[(nIndex * 3)] == 0 && pucRstAC[(nIndex * 3) + 1] == 0 && pucRstAC[(nIndex * 3) + 2] == 0)
				continue;
			else if (pucRstAC[(nIndex * 3)] == 255 && pucRstAC[(nIndex * 3) + 1] == 0 && pucRstAC[(nIndex * 3) + 2] == 0)
			{
				if ((nAC & eDefaultAC_Data_B) == eDefaultAC_Data_B)
					continue;
				if ((nInspAC & eDefaultAC_Data_CJ_B) == eDefaultAC_Data_CJ_B)
				{
					fResult++;
					fArrRstRGB[m_eInspAC_R_CJB]++;
				}
			}
			else if (pucRstAC[(nIndex * 3)] == 0 && pucRstAC[(nIndex * 3) + 1] == 255 && pucRstAC[(nIndex * 3) + 2] == 0)
			{
				if ((nAC & eDefaultAC_Data_G) == eDefaultAC_Data_G)
					continue;
				if ((nInspAC & eDefaultAC_Data_CJ_G) == eDefaultAC_Data_CJ_G)
				{
					fResult++;
					fArrRstRGB[m_eInspAC_R_CJG]++;
				}
			}
			else if (pucRstAC[(nIndex * 3)] == 0 && pucRstAC[(nIndex * 3) + 1] == 0 && pucRstAC[(nIndex * 3) + 2] == 255)
			{
				if ((nAC & eDefaultAC_Data_R) == eDefaultAC_Data_R)
					continue;
				if ((nInspAC & eDefaultAC_Data_CJ_R) == eDefaultAC_Data_CJ_R)
				{
					fResult++;
					fArrRstRGB[m_eInspAC_R_CJR]++;
				}
			}
			else if (pucRstAC[(nIndex * 3)] > 125 && pucRstAC[(nIndex * 3) + 1] > 125 && pucRstAC[(nIndex * 3) + 2] > 125)
			{
				if ((nAC & eDefaultAC_Data_Gray) == eDefaultAC_Data_Gray)
					continue;
				if ((nInspAC & eDefaultAC_Data_CJ_White) == eDefaultAC_Data_CJ_White)
				{
					fResult++;
					fArrRstRGB[m_eInspAC_R_CJW]++;
				}
			}
			else
			{
				if ((nAC & eDefaultAC_Data_Gray) == eDefaultAC_Data_Gray)
					continue;
				if ((nInspAC & eDefaultAC_Data_CJ_Black) == eDefaultAC_Data_CJ_Black)
				{
					fResult++;
					fArrRstRGB[m_eInspAC_R_CJBL]++;
				}
			}
		}
	}
	return fResult;
}
int IPINSP_ALGO::InspAC(int nInspAC_T, float* fArrRstRGB, bool bRsx)
{
	int nInspAC = 0;
	if ((nInspAC_T & eDefaultAC_Data_Use) != eDefaultAC_Data_Use || fArrRstRGB == NULL)
		return nInspAC;

	if ((nInspAC_T & eDefaultAC_Data_Auto) == eDefaultAC_Data_Auto)
	{
		nInspAC = eDefaultAC_Data_Gray;
		float fMaxInspAC = fArrRstRGB[m_eInspAC_R_Gr];
		for (int a = 0; a < m_eInspAC_R_Gr; a++)
		{
			if (fArrRstRGB[a] > fMaxInspAC)
			{
				fMaxInspAC = fArrRstRGB[a];
				if (a == m_eInspAC_R_R) nInspAC = eDefaultAC_Data_R;
				if (a == m_eInspAC_R_G) nInspAC = eDefaultAC_Data_G;
				if (a == m_eInspAC_R_B) nInspAC = eDefaultAC_Data_B;
			}
		}
		fArrRstRGB[m_eInspAC_R_TA] = fMaxInspAC;
	}
	else
	{
		if ((nInspAC_T & eDefaultAC_Data_R) == eDefaultAC_Data_R || (nInspAC_T & eDefaultAC_Data_R2) == eDefaultAC_Data_R2)
		{
			nInspAC |= eDefaultAC_Data_R;
			fArrRstRGB[m_eInspAC_R_TA] += fArrRstRGB[m_eInspAC_R_R];
		}
		if ((nInspAC_T & eDefaultAC_Data_G) == eDefaultAC_Data_G || (nInspAC_T & eDefaultAC_Data_G2) == eDefaultAC_Data_G2)
		{
			nInspAC |= eDefaultAC_Data_G;
			fArrRstRGB[m_eInspAC_R_TA] += fArrRstRGB[m_eInspAC_R_G];
		}
		if ((nInspAC_T & eDefaultAC_Data_B) == eDefaultAC_Data_B || (nInspAC_T & eDefaultAC_Data_B2) == eDefaultAC_Data_B2)
		{
			nInspAC |= eDefaultAC_Data_B;
			fArrRstRGB[m_eInspAC_R_TA] += fArrRstRGB[m_eInspAC_R_B];
		}
		if ((nInspAC_T & eDefaultAC_Data_Gray) == eDefaultAC_Data_Gray || (nInspAC_T & eDefaultAC_Data_Gray2) == eDefaultAC_Data_Gray2)
		{
			nInspAC |= eDefaultAC_Data_Gray;
			fArrRstRGB[m_eInspAC_R_TA] += fArrRstRGB[m_eInspAC_R_Gr];
		}
	}

	if (bRsx)
	{
		for (int a = 0; a < m_eInspAC_R_Total; a++)
			fArrRstRGB[a] = fArrRstRGB[a] * m_resolX * m_resolY;
	}

	return nInspAC;
}
void IPINSP_ALGO::InspAC(UCHAR *pucRstAC, int nIndex, float* fArrRstRGB)
{
	if (pucRstAC == NULL || nIndex < 0 || fArrRstRGB == NULL)
		return;

	if (pucRstAC[(nIndex * 3)] == 255 && pucRstAC[(nIndex * 3) + 1] == 0 && pucRstAC[(nIndex * 3) + 2] == 0)
		fArrRstRGB[m_eInspAC_R_B]++;
	else if (pucRstAC[(nIndex * 3)] == 0 && pucRstAC[(nIndex * 3) + 1] == 255 && pucRstAC[(nIndex * 3) + 2] == 0)
		fArrRstRGB[m_eInspAC_R_G]++;
	else if (pucRstAC[(nIndex * 3)] < 100 && pucRstAC[(nIndex * 3) + 1] < 100 && pucRstAC[(nIndex * 3) + 2] == 255)	// Red는 255,92,92 로 처리되고 있음
		fArrRstRGB[m_eInspAC_R_R]++;
	else if (pucRstAC[(nIndex * 3)] == 0 && pucRstAC[(nIndex * 3) + 1] == 0 && pucRstAC[(nIndex * 3) + 2] == 0)
	{

	}
	else
		fArrRstRGB[m_eInspAC_R_Gr]++;
}

#pragma endregion