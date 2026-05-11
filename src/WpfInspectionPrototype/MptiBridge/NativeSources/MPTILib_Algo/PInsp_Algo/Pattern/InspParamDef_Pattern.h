#pragma once
#include "../InspParamDef_AlgoBase.h"

//pattern
#define CNT_PATTERN_PATH		16
#define CNT_PATTERN_DIVISION_C	11
#define CNT_PATTERN_DIVISION_R	11
#define CNT_PATTERN_SCORE		/*(CNT_PATTERN_DIVISION_R + 1) * */(CNT_PATTERN_DIVISION_C + 1)
#define CNT_PATTERN_SIMILAR		5


typedef struct tagPatDecisionList
{
	BOOL findOK;
	BOOL scoreOk;
	BOOL angleOk;
	BOOL offsetXOk;
	BOOL offsetYOk;
	BOOL polarityOK;

}PatDecisionList;

typedef struct tagRstInspPattern
{
	double score;
	double angle;
	double cogX;
	double cogY;
	double offsetX;
	double offsetY;

	BOOL isReverse;
	BOOL isInsp;
	BOOL ok;	// 1: GOOD ,  0: NG

	int m_nModelNum;
	int m_nDivisionNum;
	POINTF m_ptModelPos;
	double m_dDiviScore[CNT_PATTERN_SCORE];

	wchar_t m_SimilarPartCode[MAX_STRLEN];
	double m_dRstSimilarScore;

	//int wndDefectCode;
	PatDecisionList list;

	int ModelWidth;
	int ModelHeight;
	tagRstInspPattern()
	{
		score = 0.;
		angle = 0.;
		cogX = 0;
		cogY = 0;
		offsetX = 0;
		offsetY = 0;

		isReverse = FALSE;
		isInsp = FALSE;
		ok = FALSE;

		memset(m_dDiviScore, 0, sizeof(double) * CNT_PATTERN_SCORE);

		_tcscpy(m_SimilarPartCode, _T(""));
		m_dRstSimilarScore = 0.0;
		ModelWidth = 0;
		ModelHeight = 0;
	}

}RstInspPattern;

typedef struct tagTeachParamPattern
{
	double wndAngle;
	wchar_t modelPath[MAX_STRLEN];

	double searchAngleRange_Max;
	double searchAngleRange_Min;

	//[output]
	double retScore;
	double retAngle;
	double retPosX;
	double retPosY;

	double retCogX;
	double retCogY;

	int SamplingAngle;
	BOOL m_bUseNGOpt;


}TeachParamPattern;

typedef struct tagInspParamPattern
{
	//BOOL enabled;
	double wndAngle;
	wchar_t modelPath[MAX_STRLEN];

	double stdCogX[CNT_PATTERN_PATH];
	double stdCogY[CNT_PATTERN_PATH];
	double stdScore;

	double offsetRange_x;
	double offsetRange_y;
	double angleRange;
	BOOL isPolarity;
	BOOL bUseOffset;

	double searchAngleRange_Max;
	double searchAngleRange_Min;

	int DiviLnR[CNT_PATTERN_PATH];
	int DiviLnC[CNT_PATTERN_PATH];

	double GapLnR[CNT_PATTERN_PATH][CNT_PATTERN_DIVISION_R];
	double GapLnC[CNT_PATTERN_PATH][CNT_PATTERN_DIVISION_C];

	double DiviScore[CNT_PATTERN_PATH][CNT_PATTERN_SCORE];

	BOOL UseCharacter;

	BOOL DetailSearch[CNT_PATTERN_PATH];
	RECT ModelRect[CNT_PATTERN_PATH];

	RstInspPattern retInspPatternResult;

	CString similarParts[CNT_PATTERN_SIMILAR];

	double m_factor_blue;
	double m_factor_green;
	double m_factor_red;
}InspParamPattern;

typedef struct tagAlgoPattern
{
	BOOL m_bUsePolarity;
	double m_dAcceptScore;

	BOOL m_bShiftIsUse;
	double m_dShiftX;
	double m_dShiftY;
	double m_dRangeAngle;

	double m_dWndAngle;
	double m_dSearchAngleRange_Max;
	double m_dSearchAngleRange_Min;

	int m_nCntPatternPath;
	wchar_t m_sArrPathModelInspect1[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect2[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect3[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect4[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect5[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect6[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect7[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect8[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect9[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect10[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect11[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect12[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect13[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect14[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect15[MAX_STRLEN];
	wchar_t m_sArrPathModelInspect16[MAX_STRLEN];
	wchar_t m_sPathModelTeach[MAX_STRLEN];

	//[Teaching_Output]
	/*double m_dRetScore;*/
	/*double m_dRetAngle;*/
	double m_dRetPosX;
	double m_dRetPosY;
	double m_dRetCogX;
	double m_dRetCogY;

	//[Inspect_Output]
	double m_dStdCogX[CNT_PATTERN_PATH];
	double m_dStdCogY[CNT_PATTERN_PATH];
	double m_dStdScore;

	int m_SamplingAngle;
	BOOL m_bUseNGOpt;

	int m_nDiviLnR[CNT_PATTERN_PATH];
	int m_nDiviLnC[CNT_PATTERN_PATH];

	double m_dGapLnR[CNT_PATTERN_PATH][CNT_PATTERN_DIVISION_R];
	double m_dGapLnC[CNT_PATTERN_PATH][CNT_PATTERN_DIVISION_C];

	double m_dDiviScore[CNT_PATTERN_PATH][CNT_PATTERN_SCORE];

	BOOL m_bUseCharacter;

	BOOL m_bDetailSearch[CNT_PATTERN_PATH];
	RECT m_rtModelRect[CNT_PATTERN_PATH];
	wchar_t m_sSimilarPartList1[MAX_STRLEN];
	wchar_t m_sSimilarPartList2[MAX_STRLEN];
	wchar_t m_sSimilarPartList3[MAX_STRLEN];
	wchar_t m_sSimilarPartList4[MAX_STRLEN];
	wchar_t m_sSimilarPartList5[MAX_STRLEN];

	double m_factor_blue;
	double m_factor_green;
	double m_factor_red;

	int m_ExtraImageLightIndex;

	int m_nModelFilter;

	wchar_t m_sWndName[MAX_STRLEN];
	tagAlgoPattern()
	{
		m_bUsePolarity = FALSE;
		m_dAcceptScore = 0.0;

		m_dWndAngle = 0.0;

		memset(m_dStdCogX, 0, sizeof(double) * CNT_PATTERN_PATH);
		memset(m_dStdCogY, 0, sizeof(double) * CNT_PATTERN_PATH);
		m_dStdScore = 0.0;

		m_bShiftIsUse = TRUE;
		m_dShiftX = 1.0;
		m_dShiftY = 1.0;
		m_dRangeAngle = 10.0;

		m_dSearchAngleRange_Max = 0.0;
		m_dSearchAngleRange_Min = 0.0;
		m_nCntPatternPath = 0;

		memset(m_nDiviLnR, 0, sizeof(int) * CNT_PATTERN_PATH);
		memset(m_nDiviLnC, 0, sizeof(int) * CNT_PATTERN_PATH);

		memset(m_dGapLnR, 0, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_DIVISION_R);
		memset(m_dGapLnC, 0, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_DIVISION_C);

		memset(m_dDiviScore, 0, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_SCORE);

		m_bUseCharacter = FALSE;

		memset(m_bDetailSearch, 0, sizeof(BOOL) * CNT_PATTERN_PATH);
		memset(m_rtModelRect, 0, sizeof(RECT) * CNT_PATTERN_PATH);
	}

	void GetParam(TeachParamPattern *pParamTeach)
	{
		if (!pParamTeach)
			return;

		pParamTeach->wndAngle = m_dWndAngle;
		pParamTeach->searchAngleRange_Max = m_dSearchAngleRange_Max;
		pParamTeach->searchAngleRange_Min = m_dSearchAngleRange_Min;
		_tcscpy_s(pParamTeach->modelPath, m_sPathModelTeach);
		pParamTeach->SamplingAngle = m_SamplingAngle;
		pParamTeach->m_bUseNGOpt = m_bUseNGOpt;
	}

	void SetParam(const TeachParamPattern *pParamTeach)
	{
		m_dWndAngle = pParamTeach->retAngle;
		m_dStdScore = pParamTeach->retScore;

		m_dRetPosX = pParamTeach->retPosX;
		m_dRetPosY = pParamTeach->retPosY;
		//m_dRetCogX = m_dStdCogX = pParamTeach->retCogX;
		//m_dRetCogY = m_dStdCogY = pParamTeach->retCogY;
	}

	void GetParam(InspParamPattern *pParamInsp)
	{
		pParamInsp->wndAngle = m_dWndAngle;
		memcpy(pParamInsp->stdCogX, m_dStdCogX, sizeof(double) * CNT_PATTERN_PATH);
		memcpy(pParamInsp->stdCogY, m_dStdCogY, sizeof(double) * CNT_PATTERN_PATH);
		pParamInsp->stdScore = m_dAcceptScore;
		pParamInsp->offsetRange_x = m_dShiftX;
		pParamInsp->offsetRange_y = m_dShiftY;
		pParamInsp->angleRange = m_dRangeAngle;
		pParamInsp->isPolarity = m_bUsePolarity;
		pParamInsp->bUseOffset = m_bShiftIsUse;
		pParamInsp->searchAngleRange_Max = m_dSearchAngleRange_Max;
		pParamInsp->searchAngleRange_Min = m_dSearchAngleRange_Min;

		memcpy(pParamInsp->DiviLnR, m_nDiviLnR, sizeof(int) * CNT_PATTERN_PATH);
		memcpy(pParamInsp->DiviLnC, m_nDiviLnC, sizeof(int) * CNT_PATTERN_PATH);

		memcpy(pParamInsp->GapLnR, m_dGapLnR, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_DIVISION_R);
		memcpy(pParamInsp->GapLnC, m_dGapLnC, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_DIVISION_C);

		memcpy(pParamInsp->DiviScore, m_dDiviScore, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_SCORE);

		pParamInsp->UseCharacter = m_bUseCharacter;

		memcpy(pParamInsp->DetailSearch, m_bDetailSearch, sizeof(BOOL) * CNT_PATTERN_PATH);
		memcpy(pParamInsp->ModelRect, m_rtModelRect, sizeof(RECT) * CNT_PATTERN_PATH);

		for (int i = 0; i < CNT_PATTERN_SIMILAR; i++)
		{
			switch (i)
			{
			case 0:
				pParamInsp->similarParts[i].Format(_T("%s"), m_sSimilarPartList1);
				break;
			case 1:
				pParamInsp->similarParts[i].Format(_T("%s"), m_sSimilarPartList2);
				break;
			case 2:
				pParamInsp->similarParts[i].Format(_T("%s"), m_sSimilarPartList3);
				break;
			case 3:
				pParamInsp->similarParts[i].Format(_T("%s"), m_sSimilarPartList4);
				break;
			case 4:
				pParamInsp->similarParts[i].Format(_T("%s"), m_sSimilarPartList5);
				break;
			}

			pParamInsp->m_factor_blue = m_factor_blue;
			pParamInsp->m_factor_green = m_factor_green;
			pParamInsp->m_factor_red = m_factor_red;
		}
	}
	CString GetModelPath(int nModelIdx)
	{
		CString str;
		str.Format(_T(""));
		switch (nModelIdx)
		{
		case 0: str.Format(_T("%s"),m_sArrPathModelInspect1); break;
		case 1: str.Format(_T("%s"),m_sArrPathModelInspect2); break;
		case 2: str.Format(_T("%s"),m_sArrPathModelInspect3); break;
		case 3: str.Format(_T("%s"),m_sArrPathModelInspect4); break;
		case 4: str.Format(_T("%s"),m_sArrPathModelInspect5); break;
		case 5: str.Format(_T("%s"),m_sArrPathModelInspect6); break;
		case 6: str.Format(_T("%s"),m_sArrPathModelInspect7); break;
		case 7: str.Format(_T("%s"),m_sArrPathModelInspect8); break;
		case 8: str.Format(_T("%s"),m_sArrPathModelInspect9); break;
		case 9: str.Format(_T("%s"),m_sArrPathModelInspect10); break;
		case 10: str.Format(_T("%s"),m_sArrPathModelInspect11); break;
		case 11: str.Format(_T("%s"),m_sArrPathModelInspect12); break;
		case 12: str.Format(_T("%s"),m_sArrPathModelInspect13); break;
		case 13: str.Format(_T("%s"),m_sArrPathModelInspect14); break;
		case 14: str.Format(_T("%s"),m_sArrPathModelInspect15); break;
		case 15: str.Format(_T("%s"),m_sArrPathModelInspect16); break;
		}
		return str;
	}
	CString GetSimilarModelPath(int nModelIdx)
	{
		CString str;
		str.Format(_T(""));
		switch (nModelIdx)
		{
		case 0: str.Format(_T("%s"),m_sSimilarPartList1); break;
		case 1: str.Format(_T("%s"),m_sSimilarPartList2); break;
		case 2: str.Format(_T("%s"),m_sSimilarPartList3); break;
		case 3: str.Format(_T("%s"),m_sSimilarPartList4); break;
		case 4: str.Format(_T("%s"),m_sSimilarPartList5); break;
		}
		return str;
	}
}AlgoPattern;

typedef struct tagRstAlgoPattern		// Pattern 검사 결과
{
	double score;
	double angle;
	double cogX;
	double cogY;
	double offsetX;
	double offsetY;
	BOOL isReverse;

	// OK 판정 //
	BOOL m_bOKFind;
	BOOL m_bOKScore;
	BOOL m_bOKAngle;
	BOOL m_bOKOffsetX;
	BOOL m_bOKOffsetY;
	BOOL m_bOKPolarity;

	int m_nModelNum;
	int m_nDivisionNum;
	POINTF m_ptModelPos;
	double m_dDisiScore[CNT_PATTERN_SCORE];

	wchar_t m_SimilarPartCode[MAX_STRLEN];
	double m_dRstSimilarScore;
	int ModelWidth;
	int ModelHeight;
	tagRstAlgoPattern()
	{
		score = 0.;
		angle = 0.;
		cogX = 0;
		cogY = 0;
		offsetX = 0;
		offsetY = 0;

		isReverse = FALSE;

		m_bOKFind = FALSE;
		m_bOKScore = FALSE;
		m_bOKAngle = FALSE;
		m_bOKOffsetX = FALSE;
		m_bOKOffsetY = FALSE;
		m_bOKPolarity = FALSE;

		m_nModelNum = 0;
		m_nDivisionNum = 0;
		m_ptModelPos = { 0.f, 0.f };

		memset(m_dDisiScore, 0, sizeof(double) * CNT_PATTERN_SCORE);
		_tcscpy(m_SimilarPartCode, _T(""));
		m_dRstSimilarScore = 0.0;
		ModelWidth = 0;
		ModelHeight = 0;
	}
}RstAlgoPattern;