#pragma once
#include "../InspParamDef_AlgoBase.h"

enum m_eAIOPT
{
	m_eAIOPT_MissingSkip = 0x01,
	m_eAIOPT_OCR_Backup = 0x02,
	m_eAIOPT_AIOK_Delete = 0x04,
	m_eAIOPT_SimilarFontTargetFont = 0x08,

	m_eAIOPT_SOLDER_AutoInsp = 0x10,
	m_eAIOPT_Foot_AutoInsp = 0x20,
	m_eAIOPT_SimilarFontOKPass = 0x40,
	m_eAIOPT_SOLDER_AutoInsp_AC = 0x80,

	m_eAIOPT_OCR_AutoInsp = 0x100,
	m_eAIOPT_OCR_AutoInsp_AC = 0x200,
	m_eAIOPT_InspOneModel = 0x400,
	m_eAIOPT_OCR_NGTimeout = 0x800,

	m_eAIOPT_Search_AI_ROI_Not = 0x80000,
};

typedef struct tagAlgoPOCR
{
	double m_dWndAngle;
	double m_dFontAngle;

	wchar_t m_sWindName[MAX_STRLEN];
	wchar_t m_sFontPath[MAX_STRLEN];
	wchar_t m_sTargetFont[MAX_STRLEN];		//targetString[MAX_STRLEN];
	int m_nClrForeGround;					//nForeGroundColor;
	int m_nClrResultFont;					//nRstStringColor;

	int m_nThreshVal;
	int m_nThreshMode;						//0: manual  // 1:AUTO

	//************************
	//검사
	BOOL m_bUsePolarity;

	int m_nCharMaxCount;

	//Acceptance Data
	double m_dStdCharScore[MAX_STRLEN];				//dStdStrScore


// 	//LJH 2016.05.28
// 	int m_nImageMixCount;
// 	AlgoMixCondition AlgoMixCon[MAX_MIX_COUNT];

	BOOL m_bUsePOCRNG;

	BOOL m_bIsVariable;
	int m_nVariableCnt;
	wchar_t m_sSaveParam[MAX_STRLEN];

	BOOL m_bIAllMatch;

	BOOL m_bUseContrast;
	BOOL m_bUseUP;
	BOOL m_bUseOP;
	BOOL m_bUseBlobSize;
	double m_dContrast;
	double m_dUP;
	double m_dOP;
	int m_nBlobSizeMaxCnt;
	double m_dBlobSize;
	int m_nPixelFilter;

	BOOL m_bImageLog;

	BOOL m_bUseLib;
	BOOL m_bUseLowerSpecial;
	BOOL m_bUseCharSpace;
	BOOL m_bUseWndNameInWindow;
	BOOL m_bUseRemoveWndNameToArrChar;
	
	BOOL m_bUseFontSpec;
	int m_nFontSpec;

	BOOL m_bUseOCV;
	double m_dOCVScore;
	wchar_t m_sWndName[MAX_STRLEN];

	BOOL m_bUseFontAngle;
	float m_fStdFontAngle;

	tagAlgoPOCR()
	{
		m_bUsePolarity = FALSE;

		m_dWndAngle = 0.0;
		m_dFontAngle = 0.0;

		m_nClrForeGround = 0;
		m_nClrResultFont = 0;
		m_nCharMaxCount = 0;
		m_nThreshMode = 0;
		m_nThreshVal = 0;

		//Acceptance Data
		//m_dStdCharScore = 0.0;
		memset(m_dStdCharScore, 0, sizeof(double)*MAX_STRLEN);
		//LJH 2016.05.28
// 		m_nImageMixCount = 0;
// 
// 		for(int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
// 		{
// 			AlgoMixCon[iLoopCount].m_nForeGround = 1;
// 			AlgoMixCon[iLoopCount].m_nThresholdValue = 128;
// 			AlgoMixCon[iLoopCount].m_bUseAutoThreshold = false;
// 			AlgoMixCon[iLoopCount].m_bUseNoiseFilter = false;
// 		}

		m_bUsePOCRNG = FALSE;
		m_bIsVariable = false;
		m_nVariableCnt = 0;
		m_bIAllMatch = FALSE;

		m_bUseContrast = false;
		m_bUseUP = false;
		m_bUseOP = false;
		m_bUseBlobSize = false;
		m_dContrast = 10.0f;
		m_dUP = 80.0f;
		m_dOP = 80.0f;
		m_nBlobSizeMaxCnt = 0;
		m_dBlobSize = 0;
		m_nPixelFilter = 3;

		m_bImageLog = FALSE;

		m_bUseLib = FALSE;
		m_bUseLowerSpecial = FALSE;
		m_bUseCharSpace = FALSE;
		m_bUseWndNameInWindow = FALSE;
		m_bUseOCV = FALSE;
		m_dOCVScore = 70.0;
		
		m_bUseFontSpec = FALSE;
		m_nFontSpec = 0;

		m_bUseFontAngle = 0;
		m_fStdFontAngle = 6.0f;
	}

}AlgoPOCR;

typedef struct tagRstAlgoPOCR	// POCR 검사 결과
{
	int m_nCharCount;
	double m_dStringScore;
	double m_dArrCharScore[MAX_STRLEN];
	int m_nCharScoreResult[MAX_STRLEN];
	wchar_t m_cArrStr[MAX_STRLEN];

	// 	int m_nSearchThreshold;
	// 	int m_nAutoFlag;
		// OK 판정 //
	BOOL m_bOKString;
	BOOL m_bOKScore;
	BOOL m_bOKPolarity;

	double ModelX;
	double ModelY;
	double ModelWidth;
	double ModelHeight;

	double charX[MAX_STRLEN];
	double charY[MAX_STRLEN];
	double charWidth[MAX_STRLEN];
	double charHeight[MAX_STRLEN];
	double charAngle[MAX_STRLEN];

	int Insp_Ro;
	int m_nCharScoreResult_Ro[MAX_STRLEN];
	double ModelX_Ro;
	double ModelY_Ro;
	double ModelWidth_Ro;
	double ModelHeight_Ro;

	double m_dArrCharScore_Ro[MAX_STRLEN];
	double charX_Ro[MAX_STRLEN];
	double charY_Ro[MAX_STRLEN];
	double charWidth_Ro[MAX_STRLEN];
	double charHeight_Ro[MAX_STRLEN];
	double charAngle_Ro[MAX_STRLEN];

	BOOL bAIOK;
	int nAIFontOK[MAX_STRLEN];
	wchar_t sFont_AI[MAX_STRLEN];
	wchar_t sFont_AI_Sec[MAX_STRLEN];
	double Score_AI[MAX_STRLEN];
	double Score_AI_Sec[MAX_STRLEN];

	BOOL bRstShiftX;
	double dRstShiftX;
	BOOL bRstShiftY;
	double dRstShiftY;

	BOOL bContrastOK[MAX_STRLEN];
	BOOL bUPOK[MAX_STRLEN];
	BOOL bOPOK[MAX_STRLEN];
	BOOL bBlobSizeOK[MAX_STRLEN];

	double dContrastScore[MAX_STRLEN];
	double dUPScore[MAX_STRLEN];
	double dOPScore[MAX_STRLEN];
	double dBlobSizeValue[MAX_STRLEN];

	RECT m_rcROI[MAX_STRLEN];
	RECT m_rcROI_Ro[MAX_STRLEN];
	int RstRotate;//0: 0도에서 score가 높음, 1: 180도 검사가 score가 높음

	BOOL bFontAngleOK[MAX_STRLEN];
	wchar_t m_cArrTeachStr[MAX_STRLEN];
	tagRstAlgoPOCR()
	{
		reset();
	}

	void reset()
	{
		m_nCharCount = 0;
		m_dStringScore = 100;
		memset(m_dArrCharScore, 0, sizeof(double)*MAX_STRLEN);
		memset(m_nCharScoreResult, 0, sizeof(int)*MAX_STRLEN);
		memset(m_cArrStr, 0, sizeof(wchar_t)*MAX_STRLEN);

		// 	int m_nSearchThreshold;
		// 	int m_nAutoFlag;
		// OK 판정 //
		m_bOKString = true;
		m_bOKScore = true;
		m_bOKPolarity = true;

		memset(charX, 0, sizeof(double)*MAX_STRLEN);
		memset(charY, 0, sizeof(double)*MAX_STRLEN);
		memset(charWidth, 0, sizeof(double)*MAX_STRLEN);
		memset(charHeight, 0, sizeof(double)*MAX_STRLEN);
		memset(charAngle, 0, sizeof(double)*MAX_STRLEN);

		Insp_Ro = 0;
		bAIOK = FALSE;
		memset(m_nCharScoreResult_Ro, 0, sizeof(int)*MAX_STRLEN);
		memset(m_dArrCharScore_Ro, 0, sizeof(double)*MAX_STRLEN);
		memset(charX_Ro, 0, sizeof(double)*MAX_STRLEN);
		memset(charY_Ro, 0, sizeof(double)*MAX_STRLEN);
		memset(charWidth_Ro, 0, sizeof(double)*MAX_STRLEN);
		memset(charHeight_Ro, 0, sizeof(double)*MAX_STRLEN);
		memset(charAngle_Ro, 0, sizeof(double)*MAX_STRLEN);
		memset(nAIFontOK, 0, sizeof(int)*MAX_STRLEN);
		memset(sFont_AI, 0, sizeof(wchar_t)*MAX_STRLEN);
		memset(sFont_AI_Sec, 0, sizeof(double)*MAX_STRLEN);
		memset(Score_AI, 0, sizeof(double)*MAX_STRLEN);
		memset(Score_AI_Sec, 0, sizeof(double)*MAX_STRLEN);

		bRstShiftX = true;
		dRstShiftX = 0;
		bRstShiftY = true;
		dRstShiftY = 0;

		memset(bContrastOK, 0, sizeof(BOOL)*MAX_STRLEN);
		memset(bUPOK, 0, sizeof(BOOL)*MAX_STRLEN);
		memset(bOPOK, 0, sizeof(BOOL)*MAX_STRLEN);
		memset(bBlobSizeOK, 0, sizeof(BOOL)*MAX_STRLEN);

		memset(dContrastScore, 0, sizeof(double)*MAX_STRLEN);
		memset(dUPScore, 0, sizeof(double)*MAX_STRLEN);
		memset(dOPScore, 0, sizeof(double)*MAX_STRLEN);
		memset(dBlobSizeValue, 0, sizeof(double)*MAX_STRLEN);

		memset(m_rcROI, 0, sizeof(RECT)*MAX_STRLEN);
		memset(m_rcROI_Ro, 0, sizeof(RECT)*MAX_STRLEN);

		RstRotate = 0;
		memset(m_cArrTeachStr, 0, sizeof(wchar_t) * MAX_STRLEN);
	}
}RstAlgoPOCR;