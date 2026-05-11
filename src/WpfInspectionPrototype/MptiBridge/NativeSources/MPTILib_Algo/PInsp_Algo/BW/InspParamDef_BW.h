#pragma once
#include "../InspParamDef_AlgoBase.h"

enum m_eBWHeigthMean
{
	m_eBWHeigthMeanMin = 0x01,
	m_eBWHeigthMeanMax = 0x02,
	m_eBWHeigthMeanIncludeBlackArea = 0x04,
	m_eBWHeigthMean_Area = 0x08,
};

typedef struct tagAlgoBlackWhite : AlgoBaseBW
{
	BOOL m_bUseTeachingRate;
	double m_dAreaCurrent;				//PercentOK
	double m_dTeachingArea;				//PercentOK
	int m_nStdOKArea;
	struct tagAlgoColorBase m_sAlgoColorBase;
	int m_nChipTrackingGap;
}AlgoBlackWhite;

typedef struct tagAlgoBW : AlgoBaseBW
{
	BOOL m_bUseTeachingRate;
	double m_dAreaCurrent;				//PercentOK
	double m_dTeachingArea;				//PercentOK
	int m_nStdOKArea;
	struct tagAlgoColorBase m_sAlgoColorBase;
	int m_nChipTrackingGap;

	BOOL m_bUseHeightMean;
	byte m_byUseHeightOpt;
	BOOL m_bInspBWArea;
	float m_fHeightMeanMin;
	float m_fHeightMeanMax;
	byte m_byDir;
	byte m_byInspectionArea;
	BOOL m_bUseMaxOK;
	int m_nMaxOKPer;
	BOOL m_bStdZeroValue;
	struct tagAngleColorBase m_sAngleColorBase;
}AlgoBW;

typedef struct tagRstAlgoBlackWhite	// BW 검사 결과
{
	double m_dRstPercent;
	double m_dRstHeightMean;

	// OK 판정 //
	BOOL m_bOKBW;
	BOOL m_bNoSolder;
	BOOL m_bOKHeightMean;
	BOOL m_bOKMax;
	BOOL m_bAIOK;
	BOOL m_bTeach;

	struct tagRstInspAC m_sInspAC;

	double m_dRstTeachArea;

	double m_dRstPer_AI;
	double m_dRstArea_AI;

	tagRstAlgoBlackWhite()
	{
		m_dRstPercent = 0.;
		m_dRstHeightMean = 0.;
		m_bOKBW = TRUE;
		m_bNoSolder = FALSE;
		m_bOKHeightMean = TRUE;
		m_bOKMax = TRUE;
		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
		m_dRstTeachArea = 0;
		m_bAIOK = FALSE;
		m_bTeach = FALSE;
		m_dRstPer_AI = 0.;
		m_dRstArea_AI = 0.;
	}
	void Init()
	{
		m_dRstPercent = 0.;
		m_dRstHeightMean = 0.;
		m_bOKBW = TRUE;
		m_bNoSolder = FALSE;
		m_bOKHeightMean = TRUE;
		m_bOKMax = TRUE;
		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
		m_dRstTeachArea = 0;
		m_bAIOK = FALSE;
		m_bTeach = FALSE;
		m_dRstPer_AI = 0.;
		m_dRstArea_AI = 0.;
	}
}RstAlgoBlackWhite;