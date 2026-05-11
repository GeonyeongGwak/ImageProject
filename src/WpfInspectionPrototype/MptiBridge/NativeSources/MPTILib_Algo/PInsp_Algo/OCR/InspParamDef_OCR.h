#pragma once
#include "../InspParamDef_AlgoBase.h"

//LJH 2016.05.28 합칠 영상의 조건 설정
typedef struct tagAlgoMixCondition
{
	int m_nForeGround;
	int m_nThresholdValue;
	BOOL m_bUseAutoThreshold;
	BOOL m_bUseNoiseFilter;
	int m_searchThreshold;
	int m_nInspFunc;

}AlgoMixCondition;

//2016.12.13 shkim morphology param
typedef struct tagAlgoMorphParam
{
	BOOL Use;
	int filterSize;
	int filterType;
}AlgoMorphParam;

typedef struct tagTeachParamOCR
{
	//[input]	
	double wndAngle;
	double fontAngle;
	int rstImgSizeX;
	int rstImgSizeY;

	wchar_t fontPath[MAX_STRLEN];
	wchar_t targetString[MAX_STRLEN];
	int foreGroundColor;
	int rstFontColor;
	int spaceWidthMode;

	int threshVal;
	int threshMode;

	// LMJ 2013/11/06
	int removeLineCnt;
	int removeLinePos[10];
	int removeLineWid[10];

	//LJH 2016.06.08
	int m_nFilterType;      // LJH 2016.06.08 추가 (0 : 사용안함, 1 : Thin[글자가 얇은 경우], 2 : Thick[글자가 굵은 경우])
	int m_nUseImageMixType;	// LJH 2016.06.08 (0 : 사용안함[추가한 영상만큼 검사 진행], 1 : 영상 Mix 사용) 

	//LJH 2016.05.28 합칠 영상 조건 추가
	BOOL m_bUseImageMix;
	int m_nImageMixCount;
	AlgoMixCondition AlgoMixCon[MAX_MIX_COUNT];
	AlgoMorphParam AlgoErode;
	AlgoMorphParam AlgoDilate;
	BOOL m_bUseOCRNG;
	int m_SearchThreshold;
	BOOL m_bThresholdtracking;
	BOOL m_bConstraint;
	BOOL m_bStretching;

	RECT m_rtInspectArea;
	int m_nInspFunc;
	int m_nconsecutiveSpace;
}TeachParamOCR;


typedef struct tagOcrDecisionList
{
	BOOL existOK;
	BOOL stringOk;
	BOOL stringScoreOk;
	BOOL charCountOk;
	BOOL polarityOK;
	int m_searchThreshold;

}OcrDecisionList;

typedef struct tagRstInspOCR
{
	int strCount;
	int charCount;
	double stringScore;
	double charScore[MAX_STRLEN];
	int charScoreResult[MAX_STRLEN];
	double strPosX;
	double strPosY;
	double strAngle;
	wchar_t str[MAX_STRLEN];
	int inspCount;

	BOOL isReverse;
	BOOL isInsp;
	BOOL ok;  // 1: GOOD ,  0: NG
	int m_nSearchThreshold;
	double charX[MAX_STRLEN];
	double charY[MAX_STRLEN];
	double charWidth[MAX_STRLEN];
	double charHeight[MAX_STRLEN];
	double charAngle[MAX_STRLEN];
	//int wndDefectCode;
	OcrDecisionList list;
}RstInspOCR;

typedef struct tagInspParamOCR
{
	//BOOL enabled;

	//[input]	
	double wndAngle;
	double fontAngle;
	int rstImgSizeX;
	int rstImgSizeY;
	int areaMargin;
	BOOL isPolarity;

	wchar_t fontPath[MAX_STRLEN];
	wchar_t targetString[MAX_STRLEN];
	int foreGroundColor;
	int rstStringColor;
	int charMinCount;
	int charMaxCount;
	BOOL mode;
	int threshMode;  //0: manual  // 1:AUTO
	int threshVal;

	//Acceptance Data
	double stdStrScore;
	BOOL useCharScore;

	//shk 20140421
	BOOL IsRemoveTmn;

	//[output]
	RstInspOCR retInspOcrResult;	  //teach test only

	BOOL m_bUsePassFont;
	int m_nPassFontCnt;

	int m_searchThreshold;
	int m_nAutoFlag;

	// [Input]
	// SHW 20160304
	int m_nOCRSimilarityScore;
	int m_nHomogeneityScore;

	//LJH 2016.06.08
	int m_nFilterType;      // LJH 2016.06.08 추가 (0 : 사용안함, 1 : Thin[글자가 얇은 경우], 2 : Thick[글자가 굵은 경우])
	int m_nUseImageMixType;	// LJH 2016.06.08 (0 : 사용안함[추가한 영상만큼 검사 진행], 1 : 영상 Mix 사용) 

	//LJH 2016.05.28 합칠 영상 조건 추가
	BOOL m_bUseImageMix;
	int m_nImageMixCount;
	AlgoMixCondition AlgoMixCon[MAX_MIX_COUNT];
	AlgoMorphParam AlgoErode;
	AlgoMorphParam AlgoDilate;
	BOOL m_bUseOCRNG;
	BOOL m_bThresholdtracking;
	BOOL m_bConstraint;
	BOOL m_bStretching;

	RECT m_rtInspectArea;
	int m_nInspFunc;
	int m_nconsecutiveSpace;
}InspParamOCR;

typedef struct tagAlgoOCR
{
	double m_dWndAngle;
	double m_dFontAngle;
	int m_nRstImgSizeX;
	int m_nRstImgSizeY;

	wchar_t m_sFontPath[MAX_STRLEN];
	wchar_t m_sTargetFont[MAX_STRLEN];		//targetString[MAX_STRLEN];
	int m_nClrForeGround;					//nForeGroundColor;
	int m_nClrResultFont;					//nRstStringColor;
	int m_nModeSpaceWidth;					//spaceWidthMode;

	int m_nThreshVal;
	int m_nThreshMode;						//0: manual  // 1:AUTO

	// LMJ 2013/11/06
	int m_nCntRemoveLine;					//removeLineCnt;
	int m_nArrRemoveLinePos[10];			//removeLinePos[10];
	int m_nArrRemoveLineWid[10];			//removeLineWid[10];

	//************************
	//검사
	BOOL m_bInvertOCR;

	BOOL m_bUsePolarity;
	BOOL m_bRemoveTermination;

	int nAreaMargin;
	int m_nCharMinCount;
	int m_nCharMaxCount;
	BOOL m_bModeFind;					// mode

	//Acceptance Data
	double m_dStdCharScore;				//dStdStrScore
	BOOL m_bUseCharScore;

	BOOL m_bUsePassFont;
	int m_nPassFontCnt;


	//LJH 2016.06.08
	int m_nFilterType;
	int m_nUseImageMixType;

	//LJH 2016.05.28
	BOOL m_bUseImageMix;
	int m_nImageMixCount;
	AlgoMixCondition AlgoMixCon[MAX_MIX_COUNT];
	AlgoMorphParam AlgoErode;
	AlgoMorphParam AlgoDilate;

	BOOL m_bUseOCRNG;
	int m_searchThreshold;
	int m_nAutoFlag;
	BOOL m_bThresholdtracking;
	BOOL m_bConstraint;
	BOOL m_bStretching;

	RECT m_rtInspectArea;

	int m_nInspFunc;
	int m_nconsecutiveSpace;

	tagAlgoOCR()
	{
		m_bUsePolarity = FALSE;
		m_bUseCharScore = FALSE;
		m_bRemoveTermination = FALSE;

		m_dWndAngle = 0.0;
		m_dFontAngle = 0.0;
		m_bInvertOCR = FALSE;

		m_nRstImgSizeX = 0;
		m_nRstImgSizeY = 0;
		nAreaMargin = 0;
		m_bModeFind = TRUE;

		m_nClrForeGround = 0;
		m_nClrResultFont = 0;
		m_nCharMinCount = 0;
		m_nCharMaxCount = 0;
		m_nThreshMode = 0;
		m_nThreshVal = 0;

		//Acceptance Data
		m_dStdCharScore = 0.0;
		m_bUseCharScore = 0;

		m_bUsePassFont = FALSE;
		m_nPassFontCnt = 0;

		//LJH 2016.06.08
		m_nFilterType = 0;
		m_nUseImageMixType = 0;

		//LJH 2016.05.28
		m_bUseImageMix = FALSE;
		m_nImageMixCount = 0;

		m_searchThreshold = 130;
		m_nAutoFlag = 0;
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		{
			AlgoMixCon[iLoopCount].m_nForeGround = 1;
			AlgoMixCon[iLoopCount].m_nThresholdValue = 128;
			AlgoMixCon[iLoopCount].m_bUseAutoThreshold = false;
			AlgoMixCon[iLoopCount].m_bUseNoiseFilter = false;
		}
		m_bUseOCRNG = FALSE;
		AlgoErode.Use = false;
		AlgoErode.filterSize = 3;
		AlgoErode.filterType = 0;
		AlgoDilate.Use = false;
		AlgoDilate.filterSize = 3;
		AlgoDilate.filterType = 0;

		m_bThresholdtracking = FALSE;
		m_bConstraint = FALSE;
		m_bStretching = FALSE;
		m_nInspFunc = 0;
		m_nconsecutiveSpace = 0;
	}

	void GetParam(TeachParamOCR *pParamTeach)
	{
		if (!pParamTeach)
			return;
		pParamTeach->wndAngle = m_dWndAngle;
		pParamTeach->fontAngle = m_dFontAngle;
		pParamTeach->rstImgSizeX = m_nRstImgSizeX;
		pParamTeach->rstImgSizeY = m_nRstImgSizeY;

		_tcscpy_s(pParamTeach->fontPath, m_sFontPath);
		_tcscpy_s(pParamTeach->targetString, m_sTargetFont);
		pParamTeach->foreGroundColor = m_nClrForeGround;
		pParamTeach->rstFontColor = m_nClrResultFont;
		pParamTeach->spaceWidthMode = m_nModeSpaceWidth;

		pParamTeach->threshVal = m_nThreshVal;
		pParamTeach->threshMode = m_nThreshMode;

		// LMJ 2013/11/06
		pParamTeach->removeLineCnt = m_nCntRemoveLine;
		memcpy(pParamTeach->removeLinePos, m_nArrRemoveLinePos, sizeof(int) * 10);
		memcpy(pParamTeach->removeLineWid, m_nArrRemoveLineWid, sizeof(int) * 10);

		//LJH 2016.06.08
		pParamTeach->m_nFilterType = m_nFilterType;
		pParamTeach->m_nUseImageMixType = m_nUseImageMixType;

		//LJH 2016.05.28
		pParamTeach->m_bUseImageMix = m_bUseImageMix;
		pParamTeach->m_nImageMixCount = m_nImageMixCount;
		memcpy(pParamTeach->AlgoMixCon, AlgoMixCon, sizeof(AlgoMixCondition) * 2);
		memcpy(&pParamTeach->AlgoErode, &AlgoErode, sizeof(AlgoMorphParam));
		memcpy(&pParamTeach->AlgoDilate, &AlgoDilate, sizeof(AlgoMorphParam));
		pParamTeach->m_bUseOCRNG = m_bUseOCRNG;
		pParamTeach->m_bThresholdtracking = m_bThresholdtracking;
		pParamTeach->m_bConstraint = m_bConstraint;
		pParamTeach->m_bStretching = m_bStretching;

		pParamTeach->m_rtInspectArea.left = m_rtInspectArea.left;
		pParamTeach->m_rtInspectArea.right = m_rtInspectArea.right;
		pParamTeach->m_rtInspectArea.top = m_rtInspectArea.top;
		pParamTeach->m_rtInspectArea.bottom = m_rtInspectArea.bottom;

		pParamTeach->m_nInspFunc = m_nInspFunc;
		pParamTeach->m_nconsecutiveSpace = m_nconsecutiveSpace;
	}

	void GetParam(InspParamOCR *pParamInsp)
	{
		if (!pParamInsp)
			return;
		pParamInsp->wndAngle = m_dWndAngle;
		pParamInsp->fontAngle = m_dFontAngle;
		pParamInsp->rstImgSizeX = m_nRstImgSizeX;
		pParamInsp->rstImgSizeY = m_nRstImgSizeY;
		pParamInsp->areaMargin = nAreaMargin;
		pParamInsp->isPolarity = m_bUsePolarity;
		_tcscpy_s(pParamInsp->fontPath, m_sFontPath);
		_tcscpy_s(pParamInsp->targetString, m_sTargetFont);
		pParamInsp->foreGroundColor = m_nClrForeGround;
		pParamInsp->rstStringColor = m_nClrResultFont;
		pParamInsp->charMinCount = m_nCharMinCount;
		pParamInsp->charMaxCount = m_nCharMaxCount;
		pParamInsp->mode = m_bModeFind;
		pParamInsp->threshMode = m_nThreshMode;
		pParamInsp->threshVal = m_nThreshVal;
		pParamInsp->stdStrScore = m_dStdCharScore;
		pParamInsp->useCharScore = m_bUseCharScore;
		pParamInsp->IsRemoveTmn = m_bRemoveTermination;

		pParamInsp->m_bUsePassFont = m_bUsePassFont;
		pParamInsp->m_nPassFontCnt = m_nPassFontCnt;

		pParamInsp->m_searchThreshold = m_searchThreshold;
		pParamInsp->m_nAutoFlag = m_nAutoFlag;
		//LJH 2016.06.08
		pParamInsp->m_nFilterType = m_nFilterType;
		pParamInsp->m_nUseImageMixType = m_nUseImageMixType;

		//LJH 
		pParamInsp->m_bUseImageMix = m_bUseImageMix;
		pParamInsp->m_nImageMixCount = m_nImageMixCount;
		memcpy(pParamInsp->AlgoMixCon, AlgoMixCon, sizeof(AlgoMixCondition) * 2);
		memcpy(&pParamInsp->AlgoErode, &AlgoErode, sizeof(AlgoMorphParam));
		memcpy(&pParamInsp->AlgoDilate, &AlgoDilate, sizeof(AlgoMorphParam));
		pParamInsp->m_bUseOCRNG = m_bUseOCRNG;
		pParamInsp->m_bThresholdtracking = m_bThresholdtracking;
		pParamInsp->m_bConstraint = m_bConstraint;
		pParamInsp->m_bStretching = m_bStretching;

		pParamInsp->m_rtInspectArea.left = m_rtInspectArea.left;
		pParamInsp->m_rtInspectArea.right = m_rtInspectArea.right;
		pParamInsp->m_rtInspectArea.top = m_rtInspectArea.top;
		pParamInsp->m_rtInspectArea.bottom = m_rtInspectArea.bottom;
		pParamInsp->m_nInspFunc = m_nInspFunc;
		pParamInsp->m_nconsecutiveSpace = m_nconsecutiveSpace;
	}
}AlgoOCR;

typedef struct tagRstAlgoOCR	// OCR 검사 결과
{
	int m_nStrCount;
	int m_nCharCount;
	double m_dStringScore;
	double m_dArrCharScore[MAX_STRLEN];
	int m_nCharScoreResult[MAX_STRLEN];
	double m_dStrPosX;
	double m_dStrPosY;
	double m_dStrAngle;
	wchar_t m_cArrStr[MAX_STRLEN];
	int m_nInspCount;
	BOOL m_bIsReverse;

	int m_nSearchThreshold;
	int m_nAutoFlag;
	// OK 판정 //
	BOOL m_bOKString;
	BOOL m_bOKScore;
	BOOL m_bOKPolarity;

	double charX[MAX_STRLEN];
	double charY[MAX_STRLEN];
	double charWidth[MAX_STRLEN];
	double charHeight[MAX_STRLEN];
	double charAngle[MAX_STRLEN];

	BOOL m_bRotate;
	int m_nconsecutiveSpace;

	tagRstAlgoOCR()
	{
		m_nStrCount = 0;
		m_nCharCount = 0;
		m_dStringScore = 0.;
		m_dStrPosX = 0.;
		m_dStrPosY = 0.;
		m_dStrAngle = 0.;
		m_nInspCount = 0;
		m_bIsReverse = FALSE;
		m_nSearchThreshold = 130;
		m_nAutoFlag = 0;
		m_bRotate = FALSE;
		m_nconsecutiveSpace = 0;
		memset(m_cArrStr, 0, sizeof(wchar_t)*MAX_STRLEN);
	}
}RstAlgoOCR;