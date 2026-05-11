#pragma once
#include "../InspParamDef_AlgoBase.h"

#define  FILLET_DIVISION_COUNT	10
#define FILLET_LINE_COUNT 100		//NYJ 2020/04/13

typedef struct tagAlgoFillet
{
	BOOL m_bUseFilletInsp;
	BOOL m_bUseInspAngleRangeH;
	BOOL m_bChipTracking;

	double m_dGap;
	double m_dInterval;
	int m_nDivisionCounts;
	int m_nTipDirection;

	float m_fArrHeightOptionValue[FILLET_DIVISION_COUNT][eMMD_Total];
	float m_fArrAngleOptionValue[FILLET_DIVISION_COUNT - 1][eMMD_Total];

	BOOL m_bArrUseAngleRangeHInsp[FILLET_DIVISION_COUNT];
	BOOL m_bArrUseHighestOption[FILLET_DIVISION_COUNT];

	float m_fminAngle;
	float m_fmaxAngle;
	float m_fminAngle_Spec;
	float m_fmaxAngle_Spec;

	tagAlgoFillet()
	{
		m_bUseFilletInsp = false;
		m_bUseInspAngleRangeH = false;
		m_bChipTracking = false;
		m_dGap = 0;
		m_dInterval = 0;
		m_nDivisionCounts = 1;
		m_nTipDirection = 0;

		for (int a = 0; a < FILLET_DIVISION_COUNT; a++)
		{
			for (int b = 0; b < eMMD_Total; b++)
			{
				m_fArrHeightOptionValue[a][b] = 0;
			}
		}

		for (int a = 0; a < FILLET_DIVISION_COUNT - 1; a++)
		{
			for (int b = 0; b < eMMD_Total; b++)
			{
				m_fArrAngleOptionValue[a][b] = 0;
			}
		}

		for (int a = 0; a < FILLET_DIVISION_COUNT; a++)
		{
			m_bArrUseAngleRangeHInsp[a] = false;
			m_bArrUseHighestOption[a] = false;
		}

		m_fminAngle = 0;
		m_fmaxAngle = 0;
		m_fminAngle_Spec = 0;
		m_fmaxAngle_Spec = 0;
	}

}AlgoFillet;
typedef struct tagRstAlgoFillet
{
	BOOL m_bOK;
	float m_fArrRstHeight[FILLET_DIVISION_COUNT];
	float m_fArrRstAngle[FILLET_DIVISION_COUNT - 1];

	BOOL m_bArrOKHeight[FILLET_DIVISION_COUNT];
	BOOL m_bArrOKAngle[FILLET_DIVISION_COUNT - 1];

	int m_nFilletLineCnt;
	int m_nFilletLineSamplingCnt;

	POINTF m_fArrLineHeightAvg[FILLET_LINE_COUNT];
	POINTF m_fArrLineHeightHighest[FILLET_LINE_COUNT];
	RECT m_rcRect_I;		//Insp ROI

	int m_nDir;		// (0: Left, 1: Right, 2: Top, 3:Bottom)
	
	float m_fRstAngleRangeH;
	bool m_bOKAngleRangeH;

	tagRstAlgoFillet()
	{
		m_bOK = FALSE;

		memset(m_fArrRstHeight, 0, FILLET_DIVISION_COUNT * sizeof(float));
		memset(m_fArrRstAngle, 0, (FILLET_DIVISION_COUNT - 1) * sizeof(float));

		memset(m_bArrOKHeight, 0, FILLET_DIVISION_COUNT * sizeof(BOOL));
		memset(m_bArrOKAngle, 0, (FILLET_DIVISION_COUNT - 1) * sizeof(BOOL));

		m_nFilletLineCnt = 0;
		m_nFilletLineSamplingCnt = 0;

		memset(m_fArrLineHeightAvg, 0, FILLET_LINE_COUNT * sizeof(POINTF));
		memset(m_fArrLineHeightHighest, 0, FILLET_LINE_COUNT * sizeof(POINTF));
		m_nDir = 0;     //Inspection Dir (0: Left, 1: Right, 2: Top, 3:Bottom)
		m_fRstAngleRangeH = 0;
		m_bOKAngleRangeH = FALSE;
	}
	void Init()
	{
		m_bOK = FALSE;

		memset(m_fArrRstHeight, 0, FILLET_DIVISION_COUNT * sizeof(float));
		memset(m_fArrRstAngle, 0, (FILLET_DIVISION_COUNT - 1) * sizeof(float));

		memset(m_bArrOKHeight, 0, FILLET_DIVISION_COUNT * sizeof(BOOL));
		memset(m_bArrOKAngle, 0, (FILLET_DIVISION_COUNT - 1) * sizeof(BOOL));

		m_nFilletLineCnt = 0;
		m_nFilletLineSamplingCnt = 0;

		memset(m_fArrLineHeightAvg, 0, FILLET_LINE_COUNT * sizeof(POINTF));
		memset(m_fArrLineHeightHighest, 0, FILLET_LINE_COUNT * sizeof(POINTF));
		m_nDir = 0;     //Inspection Dir (0: Left, 1: Right, 2: Top, 3:Bottom)
	}
}RstAlgoFillet;