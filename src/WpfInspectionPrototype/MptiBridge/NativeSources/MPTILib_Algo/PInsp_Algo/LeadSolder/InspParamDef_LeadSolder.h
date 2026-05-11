#pragma once
#include "../BW/InspParamDef_BW.h"

enum m_eEssential_LS
{
	m_eEssential_LS_Insp3D = 0,
	m_eEssential_LS_Insp2D,
	m_eEssential_LS_HM,
	m_eEssential_LS_CJ,
	m_eEssential_LS_HD,
	m_eEssential_LS_Total,
};

typedef struct tagAlgoLeadSolder
{
	int nSolderLength;	// leadtip 으로 부터 solder까지의 거리

	// BW
	BOOL bBWInspectionUse;
	struct tagAlgoBlackWhite sAlgoBW;

	// Height Mean
	BOOL bHeightMeanUse;
	BOOL bHeightContrastUse;
	int nHeightCalcMethod;
	int nHeightContrastInterval;
	int nHeightContrastIntervalY;
	RECT rcHeightContrast;
	double dTargetHeightContrast;
	double dLeadHeight;
	int dLeadHeight_Magin;// height margin pixel
	struct tagAlgoHeightMean sAlgoHeightMean;
	double m_nCurrentValueBW;

	// 높이 차

	BOOL bHeightDiffUse;
	int nROIWidth;			//검사 영역 width
	int nROIHeight;			//검사 영역 Height
	int nLeadInterval;	// (높이차) leadtip으로 부터 offset
	int nSolderInterval;	// (높이차) leadtip으로 부터 offset
	int nLeadTipDirection;	// Lead 방향
	int nLeadPosition;			//Lead 위치
	double dAvgHeight3D;	// 검사 영역의 기준 높이값
	double dToleranceBand3D; // 3d 높이 허용 범위

	double m_OKStandard3DRate;
	BOOL m_bEssentialInsp[m_eEssential_LS_Total];

	// Cold Joint
	BOOL m_bUseColdJoint;
	float m_fCJInterval;
	float m_fCJWidth;
	float m_fCJHeight;
	struct tagAlgoBlackWhite m_sAlgoBW_CJ;

	BOOL m_bUseTeachArea3D;
	float m_fTeachArea3D;
	float m_fGapWidth;
	float m_fSolderLength2;
	struct tagAngleColorBase m_sAngleColorBase;
	BOOL m_bHeightMeanExcept;

	tagAlgoLeadSolder()
	{
		nSolderLength = 10;

		bBWInspectionUse = TRUE;

		bHeightMeanUse = TRUE;
		m_nCurrentValueBW = 0.0;
		bHeightContrastUse = FALSE;
		nHeightCalcMethod = 0;
		nHeightContrastInterval = 0;
		nHeightContrastIntervalY = 0;
		dTargetHeightContrast = 0.0;
		dLeadHeight = 0.0;
		dLeadHeight_Magin = 12;
		m_nCurrentValueBW = 0.0;
		bHeightDiffUse = TRUE;
		nROIWidth = 10;
		nROIHeight = 10;
		nLeadInterval = 10;
		nSolderInterval = 10;
		nLeadTipDirection = 0;
		nLeadPosition = 0;
		dAvgHeight3D = 0.0;
		dToleranceBand3D = 0.0;

		m_OKStandard3DRate = 0.0;
		memset(m_bEssentialInsp, FALSE, sizeof(BOOL) * m_eEssential_LS_Total);

		m_bUseColdJoint = FALSE;
		m_fCJInterval = 0.03;
		m_fCJWidth = 0.01;
		m_fCJHeight = 0.01;

		m_bUseTeachArea3D = FALSE;
		m_fTeachArea3D = 0.0f;
		m_fGapWidth = 0.0f;
		m_fSolderLength2 = 0.0f;
		m_bHeightMeanExcept = false;
	}
}AlgoLeadSolder;

typedef struct tagRstAlgoLeadSolder	// Lead Solder 검사 결과
{
	double m_dRstBWPercent;
	double m_dRstHeightMean;
	double m_dRstHeightDiff;
	double m_dRstHeightContrast;
	double m_dRstTargetContrast;

	double m_dRstLeadHeightDiff;
	double m_dRstSolderHeightDiff;

	// OK 판정 //
	BOOL m_bOKBW;
	BOOL m_bOKHeightMean;
	BOOL m_bOKHeightDiff;
	BOOL m_bOKHeightContrast;

	double m_dRst3DPercent;
	BOOL m_bOKBW3D;

	// Cold Joint
	BOOL m_bOKCJ;
	double m_dRstCJArea;
	double m_dRstCJAreaPer;

	RECT m_rcRect_I[2];
	struct tagRstInspAC m_sInspAC;

	double m_dRstHeightContrastSoldH;
	double m_dRstHeightContrastLeadH;
	tagRstAlgoLeadSolder()
	{
		m_dRstBWPercent = 0.;
		m_dRst3DPercent = 0.;
		m_dRstHeightMean = 0.;
		m_dRstHeightDiff = 0.;
		m_dRstHeightContrast = 0.;
		m_dRstTargetContrast = 0.;

		m_bOKBW = FALSE;
		m_bOKBW3D = FALSE;
		m_bOKHeightMean = FALSE;
		m_bOKHeightContrast = FALSE;

		m_bOKCJ = FALSE;
		m_dRstCJArea = 0.;
		m_dRstCJAreaPer = 0.;

		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
	}
	void Init()
	{
		m_dRstBWPercent = 0.;
		m_dRst3DPercent = 0.;
		m_dRstHeightMean = 0.;
		m_dRstHeightDiff = 0.;

		m_bOKBW = FALSE;
		m_bOKBW3D = FALSE;
		m_bOKHeightMean = FALSE;

		m_bOKCJ = FALSE;
		m_dRstCJArea = 0.;
		m_dRstCJAreaPer = 0.;

		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		//memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
	}
}RstAlgoLeadSolder;