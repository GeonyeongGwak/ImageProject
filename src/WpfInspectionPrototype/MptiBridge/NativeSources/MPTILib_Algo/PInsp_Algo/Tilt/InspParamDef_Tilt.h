#pragma once
#include "../InspParamDef_AlgoBase.h"

#define	TiltRectCnt		4
#define	RstTiltAngleCnt	2
#define	nTiltAngleCnt	8

enum m_eAlgoTilt_Data
{
	m_eHighest = 0x01,
	m_eOneMore = 0x02,
};

typedef struct tagAlgoTilt
{
	double dAllowDiff;
	int nSelectValue;
	RECT rcArrSelectROI[TiltRectCnt];
	BOOL m_bUseAreaDiff;
	double m_dHeightDiffMin;
	double m_dHeightDiffMax;
	BOOL m_bUsePart;
	double m_dHeightAvg;
	double m_dHeightMin;
	double m_dHeightMax;
	BOOL m_bUseAngle;
	int m_nInspAreaType; // 영역의 높이 Mean, Max 선택
	double m_dZAngle;
	double m_dAngleMaxDiff;
	RECT rcAngleStdROI[nTiltAngleCnt];
	int nSelectAngleValue;
	double m_dAngleBinMin;
	double m_dAngleBinMax;
	BOOL m_bUseColorRange;
	float m_fColorRangeMax;
	float m_fColorRangeMin;
	tagAlgoTilt()
	{
		dAllowDiff = 0.0;
		nSelectValue = 2;
		m_bUseAreaDiff = true;
		m_dHeightDiffMin = 0;
		m_dHeightDiffMax = 0;
		m_bUsePart = false;
		m_dHeightAvg = 0;
		m_dHeightMin = 80;
		m_dHeightMax = 120;
		m_bUseAngle = false;
		m_nInspAreaType = 0;
		m_dZAngle = 0.0;
		m_dAngleMaxDiff = 10;
		nSelectAngleValue = 0;
		m_dHeightDiffMin = -5000;
		m_dAngleBinMax = 5000;
		m_bUseColorRange = false;
		m_fColorRangeMax = 0;
		m_fColorRangeMin = 0;

	}
}AlgoTilt;

typedef struct tagRstAlgoTilt	// Tilt 검사 결과
{
	double m_dRstHeightDiff;

	double m_dArrRectHeight[TiltRectCnt];
	double m_dHeightDiffMin;
	double m_dHeightDiffMax;
	BOOL m_bArrOKHeightAvg[TiltRectCnt];

	BOOL m_bOKArea;
	BOOL m_bOKAvg;
	BOOL m_bAngle;
	RECT m_rcRect_I[TiltRectCnt];
	BOOL m_bArrAngleOK[TiltRectCnt];
	double m_dArrAngle[TiltRectCnt];
	double m_dRstStdHeightMax;
	double m_dRstAngleMax;
	double m_dAngleX[nTiltAngleCnt];
	double m_dAngleY[nTiltAngleCnt];
	double m_dAngleZ[nTiltAngleCnt];
	tagRstAlgoTilt()
	{
		m_dRstHeightDiff = 0.;

		for (int n = 0; n < TiltRectCnt; n++)
		{
			m_dArrRectHeight[n] = 0.0;
			m_dArrAngle[n] = 0.0;
			m_bArrAngleOK[n] = TRUE;
		}
		m_dHeightDiffMin = 0.0;
		m_dHeightDiffMax = 0.0;

		m_bOKArea = FALSE;
		m_bOKAvg = FALSE;
		m_bAngle = FALSE;

		m_dRstStdHeightMax = 0.0;
		m_dRstAngleMax = 0.0;
		m_dAngleX[0] = 0;
		m_dAngleY[0] = 0;
		m_dAngleZ[0] = 0;
		m_dAngleX[1] = 0;
		m_dAngleY[1] = 0;
		m_dAngleZ[1] = 0;
	}
	void Init()
	{
		m_dRstHeightDiff = 0.;

		for (int n = 0; n < TiltRectCnt; n++)
		{
			m_dArrRectHeight[n] = 0.0;
			m_dArrAngle[n] = 0.0;
			m_bArrAngleOK[n] = TRUE;
		}
		m_dHeightDiffMin = 0.0;
		m_dHeightDiffMax = 0.0;

		m_bOKArea = FALSE;
		m_bOKAvg = FALSE;
		m_bAngle = FALSE;

		m_dRstStdHeightMax = 0.0;
		m_dRstAngleMax = 0.0;
		m_dAngleX[0] = 0;
		m_dAngleY[0] = 0;
		m_dAngleZ[0] = 0;
		m_dAngleX[1] = 0;
		m_dAngleY[1] = 0;
		m_dAngleZ[1] = 0;
	}
}RstAlgoTilt;
