#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct tagAlgoLength : AlgoBaseBW
{
	double m_dStdLength;        // 100% 기준 Solder 길이
	int m_nTipDirection;		// 시작 위치로부터의 방향

	double m_dYieldPos;			// 산출 위치 단위 %
	int m_nLeadPosition;		//Lead 위치
	int m_nChipTrackingGap;		// ChipTracking 수치 px
}AlgoLength;


typedef struct tagRstAlgoLength
{
	double m_dRstLength;
	double m_dRstPercent;
	POINTF m_poDrawLine_I[2];
	tagRstAlgoLength()
	{
		m_dRstLength = 0;
		m_dRstPercent = 0.;
	}
	void Init()
	{
		m_dRstLength = 0;
		m_dRstPercent = 0.;
	}
}RstAlgoLength;