#include "PInsp_AlgoLeadSolder.h"


CPInsp_AlgoLeadSolder::CPInsp_AlgoLeadSolder(void)
{
}


CPInsp_AlgoLeadSolder::~CPInsp_AlgoLeadSolder(void)
{
}

void CPInsp_AlgoLeadSolder::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoLeadSolder::GetInspAlgoData()
{
	return eSPCAlgoLead_Solder;
}

int CPInsp_AlgoLeadSolder::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeLeadSolder] = e_NG;
	eWholeNgType[TypeLeadSolder] = e_NG;
	nCurrentNgType = TypeLeadSolder;
	return nCurrentNgType;
}

bool CPInsp_AlgoLeadSolder::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return true;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoLeadSolder::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	bResult = InspSolder(sInspAlgo, sWndAlgoImg, *sInspImageData, ucArrDstImg, (RstAlgoLeadSolder *)sRstAlgo, stAlgoParam.m_nLeadTipPos, stAlgoParam.m_bViewCJ, stAlgoParam.m_ucArrDstImgAC);

	RstAlgoLeadSolder * sRst = (RstAlgoLeadSolder *)sRstAlgo;
	if (stAlgoParam.m_bInspection)
	{
		for (int nIdx = 0; nIdx < 2; nIdx++)
		{
			if (sRst->m_rcRect_I[nIdx].left == 0 && sRst->m_rcRect_I[nIdx].right == 0 &&
				sRst->m_rcRect_I[nIdx].top == 0 && sRst->m_rcRect_I[nIdx].bottom == 0)
				continue;
			sRst->m_rcRect_I[nIdx].left += stAlgoParam.m_dx;
			sRst->m_rcRect_I[nIdx].right += stAlgoParam.m_dx;
			sRst->m_rcRect_I[nIdx].top += stAlgoParam.m_dy;
			sRst->m_rcRect_I[nIdx].bottom += stAlgoParam.m_dy;
		}
		for (int nIdx = 0; nIdx < InspAC_R_Rect_Total; nIdx++)
		{
			if (sRst->m_sInspAC.m_rcArrRect[nIdx].left == 0 && sRst->m_sInspAC.m_rcArrRect[nIdx].right == 0 &&
				sRst->m_sInspAC.m_rcArrRect[nIdx].top == 0 && sRst->m_sInspAC.m_rcArrRect[nIdx].bottom == 0)
				continue;
			sRst->m_sInspAC.m_rcArrRect[nIdx].left += stAlgoParam.m_dx;
			sRst->m_sInspAC.m_rcArrRect[nIdx].right += stAlgoParam.m_dx;
			sRst->m_sInspAC.m_rcArrRect[nIdx].top += stAlgoParam.m_dy;
			sRst->m_sInspAC.m_rcArrRect[nIdx].bottom += stAlgoParam.m_dy;
		}
	}
	return bResult;
}

int CPInsp_AlgoLeadSolder::InspACLead(AngleColorBase sAlgo, InspRoiImgBuf &sInspImg, WndAlgoImg &sWndAlgo, int nDir, int nLeadTipPos, UCHAR *pucRstAC, RstInspAC *sRstInspAC, bool bTeach)
{
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	int nInspAC_T = sAlgo.m_nArrInspAC[m_eInspAC_T3_Data];
	int nInspAC_T2 = sAlgo.m_nArrInspAC[m_eInspAC_T3_Data2];
	int nInspAC_T3 = sAlgo.m_nArrInspAC[m_eInspAC_T3_Data3];
	if ((nInspAC_T & eDefaultAC_Data_Use) != eDefaultAC_Data_Use ||
		sRstInspAC == NULL || pColorTeach == NULL)
		return 0;
	float* pfImgSrc = sWndAlgo.m_fArr3D;
	int nWidth = sWndAlgo.m_nWidth;
	int nHeight = sWndAlgo.m_nHeight;
	UCHAR *pucAC = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucAC, nWidth * nHeight * 3);
	memset(pucAC, 0, nWidth * nHeight * 3 * sizeof(UCHAR));
	sRstInspAC->m_nOKInspAC = 0;
	sRstInspAC->m_nInspAC = 0;
	memset(sRstInspAC->m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
	memset(sRstInspAC->m_rcArrRect, 0, InspAC_R_Rect_Total * sizeof(RECT));
	AlgoColorBase sColorAlgo;
	sColorAlgo.m_bUseColor = FALSE;
	pColorTeach->GetColorBaseBin(&sColorAlgo, sInspImg, NULL, bTeach, nInspAC_T, pucAC, -1, sWndAlgo.m_nLight_index);
	m_pProcMilAlgo->SaveWorkImg(pucAC, nWidth, nHeight, _T("InspAC_1.bmp"), 3);
	int nCnt = sAlgo.m_byArrInspAC[m_eInspAC_T2_Cnt];
	int nPix = sAlgo.m_byArrInspAC[m_eInspAC_T2_Pix];
	if (nCnt <= 0) nCnt = 1;
	if (nCnt > 3) nCnt = 3;
	if (nPix < 0) nPix = 0;
	if (nPix > 5) nPix = 5;
	int nLeadAC = nLeadTipPos;
	int nArrInspAC[3];
	int nX = nWidth;
	int nY = nHeight;
	bool horizontality = nDir == 0 || nDir == 1;
	if (horizontality)
	{
		int nGap = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Gap] / m_resolX);
		nArrInspAC[0] = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Range1] / m_resolX);
		nArrInspAC[1] = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Range2] / m_resolX);
		nArrInspAC[2] = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Range3] / m_resolX);
		if (nDir == 0)
			nLeadAC = (nWidth - nLeadTipPos) - nGap;
		else
			nLeadAC = nLeadTipPos + nGap;
	}
	else
	{
		nX = nHeight;
		nY = nWidth;
		int nGap = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Gap] / m_resolY);
		nArrInspAC[0] = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Range1] / m_resolY);
		nArrInspAC[1] = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Range2] / m_resolY);
		nArrInspAC[2] = (int)(sAlgo.m_fArrInspAC[m_eInspAC_T_Range3] / m_resolY);
		if (nDir == 2)
			nLeadAC = (nHeight - nLeadTipPos) - nGap;
		else
			nLeadAC = nLeadTipPos + nGap;
	}
	for (int a = 0; a < nCnt; a++)
	{
		int nS = nLeadAC;
		int nE = nArrInspAC[a];
		if (nDir == 0 || nDir == 2)
			nS = nLeadAC - nE;
		for (int b = 0; b < a; b++)
		{
			if (nDir == 0 || nDir == 2)
				nS -= nArrInspAC[b];
			else
				nS += nArrInspAC[b];
		}

		nS += nPix;
		nE -= (nPix * 2);
		if (nS >= nX) nS = nX - 1;
		if (nS + nE > nX) nE = nX - nS - 1;
		if (nS < 0) nS = 0;
		if (nE < 0) nE = 0;
		if (horizontality) 
		{
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].left = nS;
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].right = nS + nE;
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].top = 0;
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].bottom = nY;

		}
		else 
		{
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].left = 0;
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].right = nY;
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].top = nS;
			sRstInspAC->m_rcArrRect[m_eInspAC_R_Rect_Range1 + a].bottom = nS + nE;
		}

		float fArrRstRGB[4];
		memset(fArrRstRGB, 0, 4 * sizeof(float));
		unsigned int fixCnt = 0;
		float heightSum = 0.0f;
		for (int x = nS; x < nS + nE; x++)
		{
			for (int y = 0; y < nY; y++)
			{
				int nIndexBase = (horizontality ? (y * nWidth) + x : (x * nWidth) + y);
				fixCnt++;
				heightSum += pfImgSrc[nIndexBase];
				
				int nIndex = nIndexBase * 3;
				int r = pucAC[nIndex];
				int g = pucAC[nIndex + 1];
				int b = pucAC[nIndex + 2];
				
				if (r == 255 && g == 0 && b == 0)
					fArrRstRGB[2]++; // Red
				else if (r == 0 && g == 255 && b == 0) 
					fArrRstRGB[1]++; // Green
				else if (r == 0 && g == 0 && b == 255) 
					fArrRstRGB[0]++; // Blue
				else if (r > 0 || g > 0 || b > 0) 
					fArrRstRGB[3]++; // Other colors
			}
		}	
		int nInspAC = 0;
		float fMaxInspAC = 0;
		if ((a == 0 && (nInspAC_T & eDefaultAC_Data_Auto) == eDefaultAC_Data_Auto) ||
			(a == 1 && (nInspAC_T & eDefaultAC_Data_Auto2) == eDefaultAC_Data_Auto2) ||
			(a == 2 && (nInspAC_T & eDefaultAC_Data_Auto3) == eDefaultAC_Data_Auto3))
		{
			nInspAC = eDefaultAC_Data_Gray;
			fMaxInspAC = fArrRstRGB[m_eInspAC_R_Gr];
			for (int b = 0; b < m_eInspAC_R_Gr; b++)
			{
				if (fArrRstRGB[b] > fMaxInspAC)
				{
					fMaxInspAC = fArrRstRGB[b];
					if (b == m_eInspAC_R_R) nInspAC = eDefaultAC_Data_R;
					if (b == m_eInspAC_R_G) nInspAC = eDefaultAC_Data_G;
					if (b == m_eInspAC_R_B) nInspAC = eDefaultAC_Data_B;
				}
			}
		}
		else
		{
			if ((a == 0 && ((nInspAC_T & eDefaultAC_Data_R) == eDefaultAC_Data_R || (nInspAC_T2 & m_eDefaultAC_Data2_R_Sub) == m_eDefaultAC_Data2_R_Sub)) ||
				(a == 1 && ((nInspAC_T & eDefaultAC_Data_R2) == eDefaultAC_Data_R2 || (nInspAC_T2 & m_eDefaultAC_Data2_R2_Sub) == m_eDefaultAC_Data2_R2_Sub)) ||
				(a == 2 && ((nInspAC_T & eDefaultAC_Data_R3) == eDefaultAC_Data_R3 || (nInspAC_T2 & m_eDefaultAC_Data2_R3_Sub) == m_eDefaultAC_Data2_R3_Sub)))
			{
				nInspAC |= eDefaultAC_Data_R;
				fMaxInspAC += fArrRstRGB[m_eInspAC_R_R];
			}
			if ((a == 0 && ((nInspAC_T & eDefaultAC_Data_G) == eDefaultAC_Data_G || (nInspAC_T2 & m_eDefaultAC_Data2_G_Sub) == m_eDefaultAC_Data2_G_Sub)) ||
				(a == 1 && ((nInspAC_T & eDefaultAC_Data_G2) == eDefaultAC_Data_G2 || (nInspAC_T2 & m_eDefaultAC_Data2_G2_Sub) == m_eDefaultAC_Data2_G2_Sub)) ||
				(a == 2 && ((nInspAC_T & eDefaultAC_Data_G3) == eDefaultAC_Data_G3 || (nInspAC_T2 & m_eDefaultAC_Data2_G3_Sub) == m_eDefaultAC_Data2_G3_Sub)))
			{
				nInspAC |= eDefaultAC_Data_G;
				fMaxInspAC += fArrRstRGB[m_eInspAC_R_G];
			}
			if ((a == 0 && ((nInspAC_T & eDefaultAC_Data_B) == eDefaultAC_Data_B || (nInspAC_T2 & m_eDefaultAC_Data2_B_Sub) == m_eDefaultAC_Data2_B_Sub)) ||
				(a == 1 && ((nInspAC_T & eDefaultAC_Data_B2) == eDefaultAC_Data_B2 || (nInspAC_T2 & m_eDefaultAC_Data2_B2_Sub) == m_eDefaultAC_Data2_B2_Sub)) ||
				(a == 2 && ((nInspAC_T & eDefaultAC_Data_B3) == eDefaultAC_Data_B3 || (nInspAC_T2 & m_eDefaultAC_Data2_B3_Sub) == m_eDefaultAC_Data2_B3_Sub)))
			{
				nInspAC |= eDefaultAC_Data_B;
				fMaxInspAC += fArrRstRGB[m_eInspAC_R_B];
			}
			if ((a == 0 && ((nInspAC_T & eDefaultAC_Data_Gray) == eDefaultAC_Data_Gray || (nInspAC_T2 & m_eDefaultAC_Data2_Gray_Sub) == m_eDefaultAC_Data2_Gray_Sub)) ||
				(a == 1 && ((nInspAC_T & eDefaultAC_Data_Gray2) == eDefaultAC_Data_Gray2 || (nInspAC_T2 & m_eDefaultAC_Data2_Gray2_Sub) == m_eDefaultAC_Data2_Gray2_Sub)) ||
				(a == 2 && ((nInspAC_T & eDefaultAC_Data_Gray3) == eDefaultAC_Data_Gray3 || (nInspAC_T2 & m_eDefaultAC_Data2_Gray3_Sub) == m_eDefaultAC_Data2_Gray3_Sub)))
			{
				nInspAC |= eDefaultAC_Data_Gray;
				fMaxInspAC += fArrRstRGB[m_eInspAC_R_Gr];
			}
		}
		if (a == 0)
		{
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA] = fMaxInspAC * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_R] = fArrRstRGB[m_eInspAC_R_R] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_G] = fArrRstRGB[m_eInspAC_R_G] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_B] = fArrRstRGB[m_eInspAC_R_B] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_Gr] = fArrRstRGB[m_eInspAC_R_Gr] * m_resolX * m_resolY;
			if ((nInspAC & eDefaultAC_Data_R) == eDefaultAC_Data_R) sRstInspAC->m_nInspAC |= eDefaultAC_Data_R;
			if ((nInspAC & eDefaultAC_Data_G) == eDefaultAC_Data_G) sRstInspAC->m_nInspAC |= eDefaultAC_Data_G;
			if ((nInspAC & eDefaultAC_Data_B) == eDefaultAC_Data_B) sRstInspAC->m_nInspAC |= eDefaultAC_Data_B;
			if ((nInspAC & eDefaultAC_Data_Gray) == eDefaultAC_Data_Gray) sRstInspAC->m_nInspAC |= eDefaultAC_Data_Gray;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM1] = heightSum / fixCnt;
		}
		else if (a == 1)
		{
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA2] = fMaxInspAC * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_R2] = fArrRstRGB[m_eInspAC_R_R] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_G2] = fArrRstRGB[m_eInspAC_R_G] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_B2] = fArrRstRGB[m_eInspAC_R_B] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_Gr2] = fArrRstRGB[m_eInspAC_R_Gr] * m_resolX * m_resolY;
			if ((nInspAC & eDefaultAC_Data_R) == eDefaultAC_Data_R) sRstInspAC->m_nInspAC |= eDefaultAC_Data_R2;
			if ((nInspAC & eDefaultAC_Data_G) == eDefaultAC_Data_G) sRstInspAC->m_nInspAC |= eDefaultAC_Data_G2;
			if ((nInspAC & eDefaultAC_Data_B) == eDefaultAC_Data_B) sRstInspAC->m_nInspAC |= eDefaultAC_Data_B2;
			if ((nInspAC & eDefaultAC_Data_Gray) == eDefaultAC_Data_Gray) sRstInspAC->m_nInspAC |= eDefaultAC_Data_Gray2;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM2] = heightSum / fixCnt;
		}
		else if (a == 2)
		{
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA3] = fMaxInspAC * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_R3] = fArrRstRGB[m_eInspAC_R_R] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_G3] = fArrRstRGB[m_eInspAC_R_G] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_B3] = fArrRstRGB[m_eInspAC_R_B] * m_resolX * m_resolY;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_Gr3] = fArrRstRGB[m_eInspAC_R_Gr] * m_resolX * m_resolY;
			if ((nInspAC & eDefaultAC_Data_R) == eDefaultAC_Data_R) sRstInspAC->m_nInspAC |= eDefaultAC_Data_R3;
			if ((nInspAC & eDefaultAC_Data_G) == eDefaultAC_Data_G) sRstInspAC->m_nInspAC |= eDefaultAC_Data_G3;
			if ((nInspAC & eDefaultAC_Data_B) == eDefaultAC_Data_B) sRstInspAC->m_nInspAC |= eDefaultAC_Data_B3;
			if ((nInspAC & eDefaultAC_Data_Gray) == eDefaultAC_Data_Gray) sRstInspAC->m_nInspAC |= eDefaultAC_Data_Gray3;
			sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM3] = heightSum / fixCnt;
		}
		fMaxInspAC = 0;
		if ((nInspAC & eDefaultAC_Data_R) != eDefaultAC_Data_R &&
			((a == 0 && (nInspAC_T & eDefaultAC_Data_CJ_R) == eDefaultAC_Data_CJ_R) ||
			(a == 1 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ2_R) == m_eDefaultAC_Data2_CJ2_R) ||
				(a == 2 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ3_R) == m_eDefaultAC_Data2_CJ3_R)))
		{
			fMaxInspAC += fArrRstRGB[m_eInspAC_R_R];
			if (a == 0) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJR] = fArrRstRGB[m_eInspAC_R_R] * m_resolX * m_resolY;
			if (a == 1) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJR2] = fArrRstRGB[m_eInspAC_R_R] * m_resolX * m_resolY;
			if (a == 2) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJR3] = fArrRstRGB[m_eInspAC_R_R] * m_resolX * m_resolY;
		}
		if ((nInspAC & eDefaultAC_Data_G) != eDefaultAC_Data_G &&
			((a == 0 && (nInspAC_T & eDefaultAC_Data_CJ_G) == eDefaultAC_Data_CJ_G) ||
			(a == 1 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ2_G) == m_eDefaultAC_Data2_CJ2_G) ||
				(a == 2 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ3_G) == m_eDefaultAC_Data2_CJ3_G)))
		{
			fMaxInspAC += fArrRstRGB[m_eInspAC_R_G];
			if (a == 0) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJG] = fArrRstRGB[m_eInspAC_R_G] * m_resolX * m_resolY;
			if (a == 1) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJG2] = fArrRstRGB[m_eInspAC_R_G] * m_resolX * m_resolY;
			if (a == 2) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJG3] = fArrRstRGB[m_eInspAC_R_G] * m_resolX * m_resolY;
		}
		if ((nInspAC & eDefaultAC_Data_B) != eDefaultAC_Data_B &&
			((a == 0 && (nInspAC_T & eDefaultAC_Data_CJ_B) == eDefaultAC_Data_CJ_B) ||
			(a == 1 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ2_B) == m_eDefaultAC_Data2_CJ2_B) ||
				(a == 2 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ3_B) == m_eDefaultAC_Data2_CJ3_B)))
		{
			fMaxInspAC += fArrRstRGB[m_eInspAC_R_B];
			if (a == 0) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJB] = fArrRstRGB[m_eInspAC_R_B] * m_resolX * m_resolY;
			if (a == 1) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJB2] = fArrRstRGB[m_eInspAC_R_B] * m_resolX * m_resolY;
			if (a == 2) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJB3] = fArrRstRGB[m_eInspAC_R_B] * m_resolX * m_resolY;
		}
		if ((nInspAC & eDefaultAC_Data_Gray) != eDefaultAC_Data_Gray &&
			((a == 0 && (nInspAC_T & eDefaultAC_Data_CJ_White) == eDefaultAC_Data_CJ_White) ||
			(a == 1 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ2_Gr) == m_eDefaultAC_Data2_CJ2_Gr) ||
				(a == 2 && (nInspAC_T2 & m_eDefaultAC_Data2_CJ3_Gr) == m_eDefaultAC_Data2_CJ3_Gr)))
		{
			fMaxInspAC += fArrRstRGB[m_eInspAC_R_Gr];
			if (a == 0) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJW] = fArrRstRGB[m_eInspAC_R_Gr] * m_resolX * m_resolY;
			if (a == 1) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJW2] = fArrRstRGB[m_eInspAC_R_Gr] * m_resolX * m_resolY;
			if (a == 2) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJW3] = fArrRstRGB[m_eInspAC_R_Gr] * m_resolX * m_resolY;
		}
		if (a == 0) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJ] = fMaxInspAC * m_resolX * m_resolY;
		if (a == 1) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJ2] = fMaxInspAC * m_resolX * m_resolY;
		if (a == 2) sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJ3] = fMaxInspAC * m_resolX * m_resolY;
		
		if ((nInspAC_T & eDefaultAC_Data_TA) == eDefaultAC_Data_TA)
		{
			if (a == 0)
			{
				if (sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA] < sAlgo.m_fArrInspAC[m_eInspAC_T_TAMin])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA1;
				if (((nInspAC_T & eDefaultAC_Data_TAMax) == eDefaultAC_Data_TAMax) &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA] > sAlgo.m_fArrInspAC[m_eInspAC_T_TAMax])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA1;

				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_R) != eDefaultAC_Data_R && (nInspAC_T & eDefaultAC_Data_TA_R) == eDefaultAC_Data_TA_R &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_R] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA_R;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_G) != eDefaultAC_Data_G && (nInspAC_T & eDefaultAC_Data_TA_G) == eDefaultAC_Data_TA_G &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_G] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA_G;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_B) != eDefaultAC_Data_B && (nInspAC_T & eDefaultAC_Data_TA_B) == eDefaultAC_Data_TA_B &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_B] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA_B;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_Gray) != eDefaultAC_Data_Gray && (nInspAC_T & eDefaultAC_Data_TA_Gr) == eDefaultAC_Data_TA_Gr &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_Gr] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA_BW;
			}
			else if (a == 1)
			{
				if (sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA2] < sAlgo.m_fArrInspAC[m_eInspAC_T_TAMin2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA2;
				if (((nInspAC_T & eDefaultAC_Data_TAMax) == eDefaultAC_Data_TAMax) &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA2] > sAlgo.m_fArrInspAC[m_eInspAC_T_TAMax2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA2;

				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_R2) != eDefaultAC_Data_R2 && (nInspAC_T2 & m_eDefaultAC_Data2_TA2_R) == m_eDefaultAC_Data2_TA2_R &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_R2] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA2_R;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_G2) != eDefaultAC_Data_G2 && (nInspAC_T2 & m_eDefaultAC_Data2_TA2_G) == m_eDefaultAC_Data2_TA2_G &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_G2] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA2_G;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_B2) != eDefaultAC_Data_B2 && (nInspAC_T2 & m_eDefaultAC_Data2_TA2_B) == m_eDefaultAC_Data2_TA2_B &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_B2] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA2_B;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_Gray2) != eDefaultAC_Data_Gray2 && (nInspAC_T2 & m_eDefaultAC_Data2_TA2_Gr) == m_eDefaultAC_Data2_TA2_Gr &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_Gr2] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA2_BW;
			}
			else if (a == 2)
			{
				if (sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA3] < sAlgo.m_fArrInspAC[m_eInspAC_T_TAMin3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA3;
				if (((nInspAC_T & eDefaultAC_Data_TAMax) == eDefaultAC_Data_TAMax) &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_TA3] > sAlgo.m_fArrInspAC[m_eInspAC_T_TAMax3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA3;

				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_R3) != eDefaultAC_Data_R3 && (nInspAC_T2 & m_eDefaultAC_Data2_TA3_R) == m_eDefaultAC_Data2_TA3_R &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_R3] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA3_R;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_G3) != eDefaultAC_Data_G3 && (nInspAC_T2 & m_eDefaultAC_Data2_TA3_G) == m_eDefaultAC_Data2_TA3_G &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_G3] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA3_G;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_B3) != eDefaultAC_Data_B3 && (nInspAC_T2 & m_eDefaultAC_Data2_TA3_B) == m_eDefaultAC_Data2_TA3_B &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_B3] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA3_B;
				if ((sRstInspAC->m_nInspAC & eDefaultAC_Data_Gray3) != eDefaultAC_Data_Gray3 && (nInspAC_T2 & m_eDefaultAC_Data2_TA3_Gr) == m_eDefaultAC_Data2_TA3_Gr &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_Gr3] > sAlgo.m_fArrInspAC[m_eInspAC_T_TA3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA3_BW;
			}
			if (((sRstInspAC->m_nOKInspAC & m_eInspAC_R_Data_TA1) == m_eInspAC_R_Data_TA1) ||
				((sRstInspAC->m_nOKInspAC & m_eInspAC_R_Data_TA2) == m_eInspAC_R_Data_TA2) ||
				((sRstInspAC->m_nOKInspAC & m_eInspAC_R_Data_TA3) == m_eInspAC_R_Data_TA3))
				sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_TA;
		}
		if ((nInspAC_T & eDefaultAC_Data_CJ) == eDefaultAC_Data_CJ)
		{
			if (a == 0 && sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJ] >= sAlgo.m_fArrInspAC[m_eInspAC_T_CJ])
				sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_CJ1;
			if (a == 1 && sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJ2] >= sAlgo.m_fArrInspAC[m_eInspAC_T_CJ_X])
				sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_CJ2;
			if (a == 2 && sRstInspAC->m_fRstInspAC[m_eInspAC_R_CJ3] >= sAlgo.m_fArrInspAC[m_eInspAC_T_CJ_Y])
				sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_CJ3;
			if (((sRstInspAC->m_nOKInspAC & m_eInspAC_R_Data_CJ1) == m_eInspAC_R_Data_CJ1) ||
				((sRstInspAC->m_nOKInspAC & m_eInspAC_R_Data_CJ2) == m_eInspAC_R_Data_CJ2) ||
				((sRstInspAC->m_nOKInspAC & m_eInspAC_R_Data_CJ3) == m_eInspAC_R_Data_CJ3))
				sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_CJ;
		}
		if ((nInspAC_T3 & m_eDefaultAC_Data3_HM) == m_eDefaultAC_Data3_HM)
		{
			switch (a)
			{
			case 0:
				if (sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM1] < sAlgo.m_fArrInspAC[m_eInspAC_T_HMMin1])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_HM1;
				if (((nInspAC_T3 & m_eDefaultAC_Data3_HMMax) == m_eDefaultAC_Data3_HMMax) &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM1] > sAlgo.m_fArrInspAC[m_eInspAC_T_HMMax1])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_HM1;
				break;
			case 1:
				if (sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM2] < sAlgo.m_fArrInspAC[m_eInspAC_T_HMMin2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_HM2;
				if (((nInspAC_T3 & m_eDefaultAC_Data3_HMMax) == m_eDefaultAC_Data3_HMMax) &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM2] > sAlgo.m_fArrInspAC[m_eInspAC_T_HMMax2])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_HM2;
				break;
			case 2:
				if (sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM3] < sAlgo.m_fArrInspAC[m_eInspAC_T_HMMin3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_HM3;
				if (((nInspAC_T3 & m_eDefaultAC_Data3_HMMax) == m_eDefaultAC_Data3_HMMax) &&
					sRstInspAC->m_fRstInspAC[m_eInspAC_R_HM3] > sAlgo.m_fArrInspAC[m_eInspAC_T_HMMax3])
					sRstInspAC->m_nOKInspAC |= m_eInspAC_R_Data_HM3;
				break;
			default:
				break;
			}
		}
	}
	if (pucRstAC)
	{
		int nWidthStep = nWidth;
		//if ((nWidthStep % 4) != 0) nWidthStep += 4 - (nWidthStep % 4);
		nWidthStep = g_pMPTI->nCalcWidthStep(false, nWidth);

		memset(pucRstAC, 0, nWidthStep * nHeight * 3 * sizeof(UCHAR));
		int nWndIDXAC = 0;
		for (int y = 0; y < nHeight; y++)
		{
			for (int x = 0; x < nWidth; x++)
			{
				int nIndex = (y * nWidthStep) + x;
				pucRstAC[(nIndex * 3)] = pucAC[nWndIDXAC++];
				pucRstAC[(nIndex * 3) + 1] = pucAC[nWndIDXAC++];
				pucRstAC[(nIndex * 3) + 2] = pucAC[nWndIDXAC++];
			}
		}
		m_pProcMilAlgo->SaveWorkImg(pucRstAC, nWidthStep, nHeight, _T("AngleColor_UI_Image.bmp"), 3);
	}
	Delete_1DArray(&pucAC);
	return 0;
}
BOOL CPInsp_AlgoLeadSolder::InspSolder(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImgBuf, UCHAR* ucArrDstImg, RstAlgoLeadSolder * sRstAlgo, int nLeadTipPos, byte bViewCJ, UCHAR* ucArrDstImgAC)
{
	BOOL bResult = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoLeadSolder));
		sRstAlgo->Init();
	}

	if (sInspAlgo.m_eAlgoType != eAlgoLead_Solder)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;

	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;
#if _DEBUG
	cv::Mat ImgpucImgSrc(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc);
	cv::Mat imgSrc3D(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);
#endif
	AlgoLeadSolder *pInspAlgoLeadSolder = (AlgoLeadSolder *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoLeadSolder)
		return bResult;

	// Auto 검사 시, Lead Tip 검사 결과에 따른 값을 적용 (nLeadTipPos)
	// Auto 검사 시 Lead Tip Algorithm이 없는 경우, pInspAlgoLeadLift->nLeadPosition 값을 적용 (nLeadTipPos = -1)
	// Teaching 시 Lead pInspAlgoLeadSolder->nLeadPosition 값을 적용	(nLeadTipPos = -1)
	if (nLeadTipPos < 0)
		nLeadTipPos = pInspAlgoLeadSolder->nLeadPosition;

	int nLeadDir = pInspAlgoLeadSolder->nLeadTipDirection;
	int nSolderLengthHeight = pInspAlgoLeadSolder->nSolderLength;
	int nSolderScanCount = 0;
	int nSolderX = 0;
	int nSolderY = 0;
	int nSolderWidth = 0;
	int nSolderHeight = 0;

	int nCJX = 0;
	int nCJY = 0;
	int nCJWidth = 0;
	int nCJHeight = 0;
	float fGapWidth = pInspAlgoLeadSolder->m_fGapWidth;
	float fSolderLengthHeight = pInspAlgoLeadSolder->m_fSolderLength2;
	int nGapWidth = fGapWidth / m_resolX;
	int nSolderLengthHeight2 = fSolderLengthHeight / m_resolX;
	bool bInspAnd[m_eEssential_LS_Total];
	for (int n = 0; n < m_eEssential_LS_Total; n++)
		bInspAnd[n] = pInspAlgoLeadSolder->m_bEssentialInsp[n];
	if ((nLeadDir == 0) || (nLeadDir == 1))
	{
		if (nLeadTipPos >= nImgWidth)
			nLeadTipPos = nImgWidth - 2;
		if (nSolderLengthHeight2 > nImgHeight)	nSolderLengthHeight2 = nImgHeight - 1;
		if (nSolderLengthHeight2 < 0)	nSolderLengthHeight2 = 0;
		if (nLeadDir == 0)
			//nSolderX = (nImgWidth - nLeadTipPos) - nSolderLengthHeight/2;
			nSolderX = (nImgWidth - nLeadTipPos) - nSolderLengthHeight;
		else
			//nSolderX = nLeadTipPos + nSolderLengthHeight/2;
			nSolderX = nLeadTipPos - nSolderLengthHeight2;

		nSolderY = 0;
		if (nGapWidth > nImgHeight)	nGapWidth = nImgHeight;
		if (nGapWidth < 0)	nGapWidth = 0;
		nSolderWidth = nSolderLengthHeight + nSolderLengthHeight2;
		if (nSolderX + nSolderWidth > nImgWidth)
			nSolderWidth = nImgWidth - nSolderX;
		nSolderHeight = nImgHeight - nGapWidth;
		nSolderScanCount = nSolderWidth * nSolderHeight;
		if (nGapWidth > 0)
		{
			nSolderY = nGapWidth / 2;
			if (nSolderY < 0)	nSolderY = 0;
			if (nSolderY + nSolderHeight > nImgHeight)
				nSolderHeight = nImgHeight - nSolderY;
		}
	}
	else
	{
		if (nLeadTipPos >= nImgHeight)
			nLeadTipPos = nImgHeight - 2;
		nSolderX = 0;
		if (nGapWidth > nImgWidth)	nGapWidth = nImgWidth;
		if (nGapWidth < 0)	nGapWidth = 0;
		if (nSolderLengthHeight2 > nImgWidth)	nSolderLengthHeight2 = nImgWidth - 1;
		if (nSolderLengthHeight2 < 0)	nSolderLengthHeight2 = 0;
		if (nLeadDir == 2)
			//nSolderY =(nImgHeight - nLeadTipPos) - nSolderLengthHeight/2;
			nSolderY = (nImgHeight - nLeadTipPos) - nSolderLengthHeight;
		else
			//nSolderY = nLeadTipPos + nSolderLengthHeight/2;
			nSolderY = nLeadTipPos - nSolderLengthHeight2;

		nSolderWidth = nImgWidth - nGapWidth;
		nSolderHeight = nSolderLengthHeight + nSolderLengthHeight2;
		if (nSolderY + nSolderHeight > nImgHeight)
			nSolderHeight = nImgHeight - nSolderY;
		nSolderScanCount = nSolderWidth * nSolderHeight;
		if (nGapWidth > 0)
		{
			nSolderX = nGapWidth / 2;
			if (nSolderX < 0)	nSolderX = 0;
			if (nSolderX + nSolderWidth > nImgWidth)
				nSolderWidth = nImgWidth - nSolderX;
		}
	}
	if (pInspAlgoLeadSolder->sAlgoBW.m_b3dCheck || pInspAlgoLeadSolder->sAlgoBW.m_b2dCheck)
		pInspAlgoLeadSolder->bBWInspectionUse = TRUE;
	else
	{
		nSolderX = 0; nSolderY = 0;
		nSolderWidth = 0; nSolderHeight = 0;
	}

	if (ucArrDstImg)
	{
		if (bViewCJ == 1)
		{
			if (pInspAlgoLeadSolder->m_sAlgoBW_CJ.m_b3dCheck == TRUE || pInspAlgoLeadSolder->m_sAlgoBW_CJ.m_b2dCheck == TRUE)
				m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoLeadSolder->m_sAlgoBW_CJ, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, ucArrDstImg);
		}
		else
			m_pCPInsp_Algo->GetBWImageStruct(pInspAlgoLeadSolder->sAlgoBW, pucImgSrc, pfImgSrc, nImgWidth, nImgHeight, ucArrDstImg);
		m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("LeadSolderBWImg.bmp"));
	}
	// BW 알고리즘
	if (pInspAlgoLeadSolder->bBWInspectionUse == TRUE)
	{
		if (nSolderX < 0 || nSolderX >= nImgWidth ||
			nSolderY < 0 || nSolderY >= nImgHeight ||
			nSolderWidth <= 0 || nSolderHeight <= 0 ||
			nSolderX + nSolderWidth > nImgWidth ||
			nSolderY + nSolderHeight > nImgHeight)
			return bResult;

		UCHAR *pucSolderImg = NULL;
		float *pfSolderImg = NULL;
		double dRetRate2D = 0.0;
		double dRetRate3D = 0.0;

		UCHAR* pUcArrDstImg = NULL;
		if (pInspAlgoLeadSolder->sAlgoBW.m_b3dCheck == TRUE)
		{
			//pfSolderImg	= new float[nSolderScanCount];
			pfSolderImg = g_pMManager->pem_new<float>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);
			m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("LeadSolder3DORG.bmp"));
			m_pProcMilAlgo->SaveWorkImg_float(pfSolderImg, nSolderWidth, nSolderHeight, _T("LeadSolder3D.bmp"));

			AlgoBlackWhite algoBw = pInspAlgoLeadSolder->sAlgoBW;
			if (pInspAlgoLeadSolder->m_bUseTeachArea3D == TRUE)
			{
				algoBw.m_bUseTeachingRate = TRUE;
				//pUcArrDstImg = new UCHAR[nSolderScanCount];
				pUcArrDstImg = g_pMManager->pem_new<UCHAR>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);
			}
			algoBw.m_b2dCheck = FALSE;
			double dPercentOK = 0.0;
			dRetRate3D = m_pCPInsp_Algo->GetBWImageStruct(algoBw, pucSolderImg, pfSolderImg, nSolderWidth, nSolderHeight, pUcArrDstImg);
			if (pInspAlgoLeadSolder->m_bUseTeachArea3D == TRUE)
			{
				int nOrgWhiteValue = 0;
				for (int y = 0; y < nSolderHeight; y++)
				{
					for (int x = 0; x < nSolderWidth; x++)
					{
						if (pUcArrDstImg[y * nSolderWidth + x] == 255)
							nOrgWhiteValue += 1;
					}
				}
				dRetRate3D = (double)nOrgWhiteValue * m_resolX * m_resolY;
				dPercentOK = pInspAlgoLeadSolder->m_fTeachArea3D * (pInspAlgoLeadSolder->m_OKStandard3DRate / 100.0);
			}
			else
				dPercentOK = pInspAlgoLeadSolder->m_OKStandard3DRate;
			if (sRstAlgo)
			{
				sRstAlgo->m_dRst3DPercent = dRetRate3D;
				if (dPercentOK <= dRetRate3D && dPercentOK != 0)
					sRstAlgo->m_bOKBW3D = TRUE;
			}
			Delete_1DArray(&pUcArrDstImg);
		}

		if (pInspAlgoLeadSolder->sAlgoBW.m_b2dCheck == TRUE)
		{
			//pucSolderImg	= new UCHAR[nSolderScanCount]; 
			pucSolderImg = g_pMManager->pem_new<UCHAR>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight,
				pucSolderImg, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
			m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("LeadSolder2DORG.bmp"));
			m_pProcMilAlgo->SaveWorkImg(pucSolderImg, nSolderWidth, nSolderHeight, _T("LeadSolder2D.bmp"));

			AlgoBlackWhite algoBw = pInspAlgoLeadSolder->sAlgoBW;
			if (algoBw.m_bUseTeachingRate == TRUE)
				//pUcArrDstImg = new UCHAR[nSolderScanCount];
				pUcArrDstImg = g_pMManager->pem_new<UCHAR>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);
			algoBw.m_b3dCheck = FALSE;
			double dPercentOK = 0.0;
			dRetRate2D = m_pCPInsp_Algo->GetBWImageStruct(algoBw, pucSolderImg, pfSolderImg, nSolderWidth, nSolderHeight, pUcArrDstImg);
			if (algoBw.m_bUseTeachingRate == true)
			{
				int nOrgWhiteValue = 0;
				for (int y = 0; y < nSolderHeight; y++)
				{
					for (int x = 0; x < nSolderWidth; x++)
					{
						if (pUcArrDstImg[y * nSolderWidth + x] == 255)
							nOrgWhiteValue += 1;
					}
				}
				dRetRate2D = (double)nOrgWhiteValue * m_resolX * m_resolY;
				dPercentOK = algoBw.m_dTeachingArea * (pInspAlgoLeadSolder->sAlgoBW.m_dPercentOK / 100.0);
			}
			else
				dPercentOK = pInspAlgoLeadSolder->sAlgoBW.m_dPercentOK;
			if (sRstAlgo)
			{
				sRstAlgo->m_dRstBWPercent = dRetRate2D;
				if (dPercentOK <= dRetRate2D && dPercentOK != 0)
					sRstAlgo->m_bOKBW = TRUE;
			}
			Delete_1DArray(&pUcArrDstImg);
		}

		if (pucSolderImg)
			Delete_1DArray(&pucSolderImg);
		if (pfSolderImg)
			Delete_1DArray(&pfSolderImg);
	}

	// HeightMean 알고리즘
	if (pInspAlgoLeadSolder->bHeightMeanUse == TRUE)
	{
		if (nSolderX < 0 || nSolderX >= nImgWidth ||
			nSolderY < 0 || nSolderY >= nImgHeight ||
			nSolderWidth <= 0 || nSolderHeight <= 0 ||
			nSolderX + nSolderWidth > nImgWidth ||
			nSolderY + nSolderHeight > nImgHeight)
			return bResult;

		//float *pfSolderImg	= new float[nSolderScanCount];
		float *pfSolderImg = g_pMManager->pem_new<float>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);

		m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("OriginMean3D.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfSolderImg, nSolderWidth, nSolderHeight, _T("HeightMean3D.bmp"));

		// Height Mean 알고리즘
		double dRstHeightMean = m_pCPInsp_Algo->GetHeightMean(pfSolderImg, nSolderWidth, nSolderHeight);

		if (pInspAlgoLeadSolder->m_bHeightMeanExcept)
		{
			float fHeightMinValue = 0;
			float fHeightMaxValue = 0;
			m_pCPInsp_Algo->GetHeightMinMax(pfSolderImg, nSolderWidth, nSolderHeight, &fHeightMinValue, &fHeightMaxValue);

			cv::Mat DstBin(nSolderHeight, nSolderWidth, CV_32FC1, pfSolderImg);
			cv::threshold(DstBin, DstBin, fHeightMaxValue*0.7, fHeightMaxValue, cv::THRESH_TOZERO_INV);
			cv::threshold(DstBin, DstBin, 0, fHeightMaxValue, cv::THRESH_TOZERO);

			double dSum = cv::sum(DstBin)[0];
			int CntNonzero = cv::countNonZero(DstBin);

			dRstHeightMean = dSum / CntNonzero;
			DstBin.release();
		}

		if (sRstAlgo)
		{
			sRstAlgo->m_dRstHeightMean = dRstHeightMean;
			if (dRstHeightMean >= pInspAlgoLeadSolder->sAlgoHeightMean.dHeightMin3D && dRstHeightMean <= pInspAlgoLeadSolder->sAlgoHeightMean.dHeightMax3D)
				sRstAlgo->m_bOKHeightMean = TRUE;
		}

		// Lead 대비 높이 Percent 검사
		if (pInspAlgoLeadSolder->bHeightContrastUse == TRUE)
		{
			int nLeadWidth = 3;
			int nLeadHeight = 3;
			int nLeadX = 0;
			int nLeadY = 0;

			nSolderX = pInspAlgoLeadSolder->rcHeightContrast.left;
			nSolderY = pInspAlgoLeadSolder->rcHeightContrast.top;
			nSolderWidth = pInspAlgoLeadSolder->rcHeightContrast.right - pInspAlgoLeadSolder->rcHeightContrast.left;
			nSolderHeight = pInspAlgoLeadSolder->rcHeightContrast.bottom - pInspAlgoLeadSolder->rcHeightContrast.top;

			double dRstHeightContrast = 0.0;
			int nTipLineX = 0, nTipLineY = 0;
			switch (nLeadDir)
			{
			case 0:	//	Left
				nLeadY = nImgHeight / 2 - 1;
				nLeadX = nImgWidth - nLeadTipPos + pInspAlgoLeadSolder->dLeadHeight_Magin - 1;
				nTipLineX = nImgWidth - nLeadTipPos;
				nTipLineY = nImgHeight;
				nSolderX = nImgWidth - nLeadTipPos - pInspAlgoLeadSolder->nHeightContrastInterval - nSolderWidth;
				break;
			case 1:	//	Right
				nLeadY = nImgHeight / 2 - 1;
				nLeadX = nLeadTipPos - pInspAlgoLeadSolder->dLeadHeight_Magin - 1;
				nTipLineX = nLeadTipPos;
				nTipLineY = nImgHeight;
				nSolderX = nLeadTipPos + pInspAlgoLeadSolder->nHeightContrastInterval;
				break;
			case 2:	//	Top
				nLeadY = nImgHeight - nLeadTipPos + pInspAlgoLeadSolder->dLeadHeight_Magin - 1;
				nLeadX = nImgWidth / 2 - 1;
				nTipLineX = nImgWidth;
				nTipLineY = nImgHeight - nLeadTipPos;
				nSolderY = nImgHeight - nLeadTipPos - pInspAlgoLeadSolder->nHeightContrastIntervalY - nSolderHeight;
				break;
			case 3: //	Bottom
				nLeadY = nLeadTipPos - pInspAlgoLeadSolder->dLeadHeight_Magin - 1;
				nLeadX = nImgWidth / 2 - 1;
				nTipLineX = nImgWidth;
				nTipLineY = nLeadTipPos;
				nSolderY = nLeadTipPos + pInspAlgoLeadSolder->nHeightContrastIntervalY;
				break;
			default:
				break;
			}
			float *pfLeadImg = g_pMManager->pem_new<float>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);

			m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
			m_proc3d.GetCropZmap_LT(pfImgSrc, pfLeadImg, nImgWidth, nImgHeight, nLeadX, nLeadY, nLeadWidth, nLeadHeight);

			double dRstHeight = 0.0;
			if (pInspAlgoLeadSolder->nHeightCalcMethod == 0)
			{
				dRstHeight = m_pCPInsp_Algo->GetHeightMean(pfSolderImg, nSolderWidth, nSolderHeight);
			}
			else if (pInspAlgoLeadSolder->nHeightCalcMethod == 1)
			{
				float dRstHeightMin = 0;
				float dRstHeightMax = 0;

				m_pCPInsp_Algo->GetHeightMinMax(pfSolderImg, nSolderWidth, nSolderHeight, &dRstHeightMin, &dRstHeightMax);

				dRstHeight = dRstHeightMin;
			}
			double dRstLeadHeight = m_pCPInsp_Algo->GetHeightMean(pfLeadImg, nLeadWidth, nLeadHeight);

			//dRstHeightContrast = 100 - ((dRstLeadHeight - dRstHeight) / dRstLeadHeight * 100); // old
			//double dLeadHeight = pInspAlgoLeadSolder->dLeadHeight * 1000;
			//dRstHeightContrast = std::abs(dLeadHeight - dRstLeadHeight + dRstHeight) / dLeadHeight * 100;
			if (dRstLeadHeight != 0)
				dRstHeightContrast = 100 - ((dRstLeadHeight - dRstHeight) / dRstLeadHeight * 100); // 20240202 old version revert request
			else
				dRstHeightContrast = 0;

			sRstAlgo->m_dRstHeightContrast = dRstHeightContrast;
			sRstAlgo->m_dRstTargetContrast = pInspAlgoLeadSolder->dTargetHeightContrast;
			sRstAlgo->m_bOKHeightContrast = TRUE;
			if (dRstHeightContrast < pInspAlgoLeadSolder->dTargetHeightContrast)
				sRstAlgo->m_bOKHeightContrast = FALSE;

			sRstAlgo->m_dRstHeightContrastLeadH = dRstLeadHeight;
			sRstAlgo->m_dRstHeightContrastSoldH = dRstHeight;
#ifdef _DEBUG
			cv::Mat cvInspImg(nImgHeight, nImgWidth, CV_8UC1);
			cvInspImg.data = pucImgSrc;
			cv::Mat cvColorInspImg(nImgHeight, nImgWidth, CV_8UC3);
			cv::cvtColor(cvInspImg, cvColorInspImg, cv::COLOR_GRAY2BGR);
			cv::Rect cvLeadRect(nLeadX, nLeadY, nLeadWidth, nLeadHeight);
			cv::Rect cvSolderRect(nSolderX, nSolderY, nSolderWidth, nSolderHeight);
			cv::rectangle(cvColorInspImg, cvLeadRect, cv::Scalar(0, 0, 255));
			cv::rectangle(cvColorInspImg, cvSolderRect, cv::Scalar(255, 255, 0));
			if (nLeadDir == 0 || nLeadDir == 1)
			{
				cv::line(cvColorInspImg, cv::Point(nTipLineX, 0), cv::Point(nTipLineX, nImgHeight), cv::Scalar(0, 255, 0));
			}
			else if (nLeadDir == 2 || nLeadDir == 3)
			{
				cv::line(cvColorInspImg, cv::Point(0, nTipLineY), cv::Point(nTipLineX, nTipLineY), cv::Scalar(0, 255, 0));
			}
			cv::imwrite("D:\\testimage\\cvColorInspImg.bmp", cvColorInspImg);
#endif
		}
		if (pfSolderImg)
			Delete_1DArray(&pfSolderImg);
	}

	// 높이차
	if (pInspAlgoLeadSolder->bHeightDiffUse == TRUE)
	{
		int nROIWidth = pInspAlgoLeadSolder->nROIWidth;
		int nROIHeight = pInspAlgoLeadSolder->nROIHeight;
		int nLeadInterval = pInspAlgoLeadSolder->nLeadInterval;
		int nSolderInterval = pInspAlgoLeadSolder->nSolderInterval;

		if (nROIWidth > nImgWidth)
			nROIWidth = nImgWidth;
		if (nROIHeight > nImgHeight)
			nROIHeight = nImgHeight;

		if ((nROIWidth <= 0) || (nROIHeight <= 0) || (nLeadTipPos <= 0) || (nLeadInterval <= 0) || (nSolderInterval <= 0))
			return bResult;

		int nHeightDiffScanCount = nROIWidth * nROIHeight;
		//float *pfImgDst = new float[nHeightDiffScanCount];
		float *pfImgDst = g_pMManager->pem_new<float>(true, nHeightDiffScanCount, (PCHAR)__FUNCTION__, __LINE__);

		double dLeadHeight = 0.0;
		double dSolderHeight = 0.0;

		for (int i = 0; i < 2; i++)
		{
			memset(pfImgDst, 0, nHeightDiffScanCount);
			int nHeightX = 0;
			int nHeightY = 0;

			if (i == 0)
			{
				if ((nLeadDir == 0) || (nLeadDir == 1))
				{
					if (nLeadDir == 0)
						//nHeightX = (nImgWidth - nLeadTipPos) + nLeadInterval + (nROIWidth/2);
						nHeightX = (nImgWidth - nLeadTipPos) + nLeadInterval;
					else
						nHeightX = (nImgWidth - nLeadTipPos) - nLeadInterval;

					//nHeightY = nImgHeight / 2;
					nHeightY = RounD(((double)nImgHeight / 2.0) - ((double)nROIHeight / 2.0));
				}
				else
				{
					//nHeightX = nImgWidth / 2;
					nHeightX = RounD(((double)nImgWidth / 2.0) - ((double)nROIWidth / 2.0));

					if (nLeadDir == 2)
						nHeightY = (nImgHeight - nLeadTipPos) + nLeadInterval;
					else
						nHeightY = (nImgHeight - nLeadTipPos) - nLeadInterval;
				}

				m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst, nImgWidth, nImgHeight, nHeightX, nHeightY, nROIWidth, nROIHeight);
				m_pProcMilAlgo->SaveWorkImg_float(pfImgDst, nROIWidth, nROIHeight, _T("LeadHeightD.bmp"));

				// Lead
				dLeadHeight = m_pCPInsp_Algo->GetHeightMean(pfImgDst, nROIWidth, nROIHeight);
				if (sRstAlgo)
					sRstAlgo->m_dRstLeadHeightDiff = dLeadHeight;
			}
			else
			{
				if ((nLeadDir == 0) || (nLeadDir == 1))
				{
					if (nLeadDir == 0)
						nHeightX = (nImgWidth - nLeadTipPos) - nSolderInterval - (nROIWidth / 2);
					else
						nHeightX = (nImgWidth - nLeadTipPos) + nSolderInterval + (nROIWidth / 2);

					nHeightY = nImgHeight / 2;
				}
				else
				{
					nHeightX = nImgWidth / 2;

					if (nLeadDir == 2)
						nHeightY = (nImgHeight - nLeadTipPos) - nSolderInterval - (nROIHeight / 2);
					else
						nHeightY = (nImgHeight - nLeadTipPos) + nSolderInterval + (nROIHeight / 2);
				}

				m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst, nImgWidth, nImgHeight, nHeightX, nHeightY, nROIWidth, nROIHeight);
				m_pProcMilAlgo->SaveWorkImg_float(pfImgDst, nROIWidth, nROIHeight, _T("SolderHeightD.bmp"));

				// Solder
				dSolderHeight = m_pCPInsp_Algo->GetHeightMean(pfImgDst, nROIWidth, nROIHeight);
				if (sRstAlgo)
					sRstAlgo->m_dRstSolderHeightDiff = dSolderHeight;
			}
		}
		if (sRstAlgo)
		{
			sRstAlgo->m_dRstHeightDiff = dLeadHeight - dSolderHeight;
			if (pInspAlgoLeadSolder->dAvgHeight3D + pInspAlgoLeadSolder->dToleranceBand3D >= sRstAlgo->m_dRstHeightDiff)
				sRstAlgo->m_bOKHeightDiff = TRUE;
		}

		if (pfImgDst)
			Delete_1DArray(&pfImgDst);
	}

	// Cold Joint
	if (pInspAlgoLeadSolder->m_bUseColdJoint == TRUE)
	{
		float fGap = pInspAlgoLeadSolder->m_fCJInterval;
		int nCJGap = 0;
		nCJWidth = (pInspAlgoLeadSolder->m_fCJWidth / m_resolX);
		nCJHeight = (pInspAlgoLeadSolder->m_fCJHeight / m_resolY);
		if (sWndAlgoImg.dAngle == 90 || sWndAlgoImg.dAngle == 270)
		{
			nCJWidth = (pInspAlgoLeadSolder->m_fCJHeight / m_resolX);
			nCJHeight = (pInspAlgoLeadSolder->m_fCJWidth / m_resolY);
		}
		if ((nLeadDir == 0) || (nLeadDir == 1))
		{
			nCJGap = fGap / m_resolX;
			if (nLeadDir == 0)
				nCJX = (nImgWidth - nLeadTipPos) - nCJGap - nCJWidth;
			else
				nCJX = nLeadTipPos + nCJGap;

			nCJY = (nImgHeight - nCJHeight) / 2;
			if (nCJY < 0)
				nCJY = 0;
		}
		else
		{
			nCJGap = fGap / m_resolY;
			if (nLeadDir == 2)
				nCJY = (nImgHeight - nLeadTipPos) - nCJGap - nCJHeight;
			else
				nCJY = nLeadTipPos + nCJGap;

			nCJX = (nImgWidth - nCJWidth) / 2;
			if (nCJX < 0)
				nCJX = 0;
		}
		if (nCJX + nCJWidth > nImgWidth)
			nCJWidth = nImgWidth - nCJX;
		if (nCJY + nCJHeight > nImgHeight)
			nCJHeight = nImgHeight - nCJY;
		if ((pInspAlgoLeadSolder->m_sAlgoBW_CJ.m_b3dCheck == TRUE || pInspAlgoLeadSolder->m_sAlgoBW_CJ.m_b2dCheck == TRUE)
			&& (nCJWidth > 0 && nCJHeight > 0))
		{
			/*UCHAR* pUcArrDstImg_CJ = new UCHAR[nCJWidth * nCJHeight];
			float* pfSolderImg	= new float[nCJWidth * nCJHeight];
			UCHAR* pucSolderImg	= new UCHAR[nCJWidth * nCJHeight];*/
			UCHAR* pUcArrDstImg_CJ = g_pMManager->pem_new<UCHAR>(true, nCJWidth * nCJHeight, (PCHAR)__FUNCTION__, __LINE__);
			float* pfSolderImg = g_pMManager->pem_new<float>(true, nCJWidth * nCJHeight, (PCHAR)__FUNCTION__, __LINE__);
			UCHAR* pucSolderImg = g_pMManager->pem_new<UCHAR>(true, nCJWidth * nCJHeight, (PCHAR)__FUNCTION__, __LINE__);
			memset(pUcArrDstImg_CJ, 0, sizeof(UCHAR) * nCJWidth * nCJHeight);
			memset(pucSolderImg, 0, sizeof(UCHAR) * nCJWidth * nCJHeight);
			memset(pfSolderImg, 0, sizeof(float) * nCJWidth * nCJHeight);
			m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nCJX, nCJY, nCJWidth, nCJHeight);
			m_pProcMilAlgo->SaveWorkImg_float(pfSolderImg, nCJWidth, nCJHeight, _T("LeadSolder3D_CJ.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, pucSolderImg, nCJX, nCJY, nCJWidth, nCJHeight);
			m_pProcMilAlgo->SaveWorkImg(pucSolderImg, nCJWidth, nCJHeight, _T("LeadSolder2D_CJ.bmp"));
			AlgoBlackWhite *algoBw = &pInspAlgoLeadSolder->m_sAlgoBW_CJ;
			double dRstRate = m_pCPInsp_Algo->GetBWImageStruct(*algoBw, pucSolderImg, pfSolderImg, nCJWidth, nCJHeight, pUcArrDstImg_CJ);
			m_pProcMilAlgo->SaveWorkImg(pUcArrDstImg_CJ, nCJWidth, nCJHeight, _T("LeadSolderBW_CJ.bmp"));
			int nOrgWhiteValue = 0;
			for (int y = 0; y < nCJHeight; y++)
			{
				for (int x = 0; x < nCJWidth; x++)
				{
					if (pUcArrDstImg_CJ[y * nCJWidth + x] == 255)
						nOrgWhiteValue += 1;
				}
			}
			double dCurArea = nOrgWhiteValue * m_resolX * m_resolY;
			if (sRstAlgo)
			{
				sRstAlgo->m_bOKCJ = TRUE;
				double dTeachArea = pInspAlgoLeadSolder->m_sAlgoBW_CJ.m_dTeachingArea * (pInspAlgoLeadSolder->m_sAlgoBW_CJ.m_dPercentOK / 100.0);
				if (dCurArea > dTeachArea)
					sRstAlgo->m_bOKCJ = FALSE;

				sRstAlgo->m_dRstCJArea = dCurArea;
				sRstAlgo->m_dRstCJAreaPer = dRstRate;
			}
			if (pucSolderImg)
				Delete_1DArray(&pucSolderImg);
			if (pfSolderImg)
				Delete_1DArray(&pfSolderImg);
			if (pUcArrDstImg_CJ)
				Delete_1DArray(&pUcArrDstImg_CJ);
		}
	}
	//AngleColor
	if (ucArrDstImgAC)
	{
		int widthStep = nImgWidth;
		//if ((widthStep % 4) != 0) widthStep += 4 - (widthStep % 4);
		widthStep = g_pMPTI->nCalcWidthStep(false, nImgWidth);

		memset(ucArrDstImgAC, 0, widthStep * nImgHeight * 3 * sizeof(UCHAR));
	}
	if ((pInspAlgoLeadSolder->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) == eDefaultAC_Data_Use)
		InspACLead(pInspAlgoLeadSolder->m_sAngleColorBase, sInspImgBuf, sWndAlgoImg, nLeadDir, nLeadTipPos, ucArrDstImgAC, &sRstAlgo->m_sInspAC, (ucArrDstImg != NULL));
	else
		sRstAlgo->m_sInspAC.m_nOKInspAC = 1;
	bResult = FALSE;
	if (sRstAlgo)
	{
		sRstAlgo->m_rcRect_I[0].left = nSolderX;
		sRstAlgo->m_rcRect_I[0].top = nSolderY;
		sRstAlgo->m_rcRect_I[0].right = nSolderX + nSolderWidth;
		sRstAlgo->m_rcRect_I[0].bottom = nSolderY + nSolderHeight;
		sRstAlgo->m_rcRect_I[1].left = nCJX;
		sRstAlgo->m_rcRect_I[1].top = nCJY;
		sRstAlgo->m_rcRect_I[1].right = nCJX + nCJWidth;
		sRstAlgo->m_rcRect_I[1].bottom = nCJY + nCJHeight;
		bool bInspACAnd = ((pInspAlgoLeadSolder->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Essential) == eDefaultAC_Data_Essential);
		if ((bInspAnd[m_eEssential_LS_Insp3D] == FALSE) && (bInspAnd[m_eEssential_LS_Insp2D] == FALSE) &&
			(bInspAnd[m_eEssential_LS_HM] == FALSE) && (bInspAnd[m_eEssential_LS_CJ] == FALSE) && bInspACAnd == false)
		{
			if (sRstAlgo->m_bOKBW || sRstAlgo->m_bOKBW3D || sRstAlgo->m_bOKHeightMean || sRstAlgo->m_bOKCJ || (sRstAlgo->m_sInspAC.m_nOKInspAC == 0))
				bResult = TRUE;
		}
		else
		{
			if (bInspAnd[m_eEssential_LS_Insp3D] && pInspAlgoLeadSolder->sAlgoBW.m_b3dCheck)
			{
				if (sRstAlgo->m_bOKBW3D == FALSE)
					return bResult = FALSE;
				else
					bResult = TRUE;
			}
			else
				bResult = sRstAlgo->m_bOKBW3D;

			if (bInspAnd[m_eEssential_LS_Insp2D] && pInspAlgoLeadSolder->sAlgoBW.m_b2dCheck)
			{
				if (sRstAlgo->m_bOKBW == FALSE)
					return bResult = FALSE;
				else
					bResult = TRUE;
			}
			else
				bResult = bResult || sRstAlgo->m_bOKBW;

			if (bInspAnd[m_eEssential_LS_HM] && pInspAlgoLeadSolder->bHeightMeanUse)
			{
				if (sRstAlgo->m_bOKHeightMean == FALSE)
					return bResult = FALSE;
				else
					bResult = TRUE;
			}
			else
				bResult = bResult || sRstAlgo->m_bOKHeightMean;

			if (bInspAnd[m_eEssential_LS_CJ] && pInspAlgoLeadSolder->m_bUseColdJoint)
			{
				if (sRstAlgo->m_bOKCJ == FALSE)
					return bResult = FALSE;
				else
					bResult = TRUE;
			}
			else
				bResult = bResult || sRstAlgo->m_bOKCJ;

			if (bInspACAnd && (pInspAlgoLeadSolder->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) == eDefaultAC_Data_Use)
			{
				if (sRstAlgo->m_sInspAC.m_nOKInspAC > 0)
					return bResult = FALSE;
				else
					bResult = TRUE;
			}
			else
				bResult = bResult || (sRstAlgo->m_sInspAC.m_nOKInspAC == 0);
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoLeadSolder::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoLeadSolder::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoLeadSolder * pInspAlgo = (AlgoLeadSolder *)sInspAlgo.m_ptrInspAlgoParam;
	if (pInspAlgo->m_sAngleColorBase.UseAC())
	{
		nData |= COLOR_DATA_AC;
		nData |= COLOR_DATA_WND;
	}

	return nData;
}
bool CPInsp_AlgoLeadSolder::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}