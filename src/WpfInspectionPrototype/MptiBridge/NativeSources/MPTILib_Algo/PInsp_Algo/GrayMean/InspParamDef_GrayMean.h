#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct stAlgoGrayMean
{
	// 2D
	int m_nGrayRateMin;	// 최소 %
	int m_nGrayRateMax;	// 최대 %
	float m_fGrayAvg;		// 기준 값
	struct tagAlgoBlobBase m_sBlobBase;

	stAlgoGrayMean()
	{
		m_nGrayRateMin = 80;
		m_nGrayRateMax = 120;
		m_fGrayAvg = 0.0;
	}
}AlgoGrayMean;

typedef struct tagRstAlgoGrayMean	// Gray Mean 검사 결과
{
	double m_dRstGrayMean;
	BOOL m_bRstStdChanged;
	int m_nRstStdGrayMin;
	int m_nRstStdGrayMax;
	BOOL m_bUseUV;
	int m_nStdGrayMin; // 티칭한 Min 값
	int m_nStdGrayMax; // 티칭한 Max 값
	tagRstAlgoGrayMean()
	{
		m_dRstGrayMean = 0.;
		m_bRstStdChanged = false;
		m_nRstStdGrayMin = 0;
		m_nRstStdGrayMax = 0;
		m_nStdGrayMin = 0;
		m_nStdGrayMax = 0;
		m_bUseUV = false;
	}
	void Init()
	{
		m_dRstGrayMean = 0.;
		m_bRstStdChanged = false;
		m_nRstStdGrayMin = 0;
		m_nRstStdGrayMax = 0;
		m_nStdGrayMin = 0;
		m_nStdGrayMax = 0;
		m_bUseUV = false;
	}
}RstAlgoGrayMean;