#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct tagAlgoGrayDiff// : GrayDiff
{
	RECT rcFirstROI;
	RECT rcSeccondROI;
	int nGrayDiff;

	BOOL m_bDiffUpper;	// 높을때 검사
	BOOL m_bDiffLower;	// 낮을때 검사

	BOOL m_bPolarity;	// SHW 20150302
	BOOL m_bSignInversion;
	int m_nMinMaxflag;
	tagAlgoGrayDiff()
	{
		rcFirstROI.bottom = 0;
		rcFirstROI.left = 0;
		rcFirstROI.right = 0;
		rcFirstROI.top = 0;

		rcSeccondROI.bottom = 0;
		rcSeccondROI.left = 0;
		rcSeccondROI.right = 0;
		rcSeccondROI.top = 0;

		nGrayDiff = 0.0;

		m_bDiffUpper = false;
		m_bDiffLower = false;

		m_bPolarity = FALSE;
		m_bSignInversion = TRUE;
		m_nMinMaxflag = 0;
	}
}AlgoGrayDiff;

typedef struct tagRstAlgoGrayDiff	// Gray diff 검사 결과
{
	int m_nRstGrayDiff;

	double m_dRstF;
	double m_dRstS;

	BOOL m_bCheckPolarity;
	RECT m_rcRect_I[2];
	tagRstAlgoGrayDiff()
	{
		m_nRstGrayDiff = 0;
		m_dRstF = 0;
		m_dRstS = 0;
		m_bCheckPolarity = FALSE;
	}
	void Init()
	{
		m_nRstGrayDiff = 0;
		m_dRstF = 0;
		m_dRstS = 0;
		m_bCheckPolarity = FALSE;
	}
}RstAlgoGrayDiff;