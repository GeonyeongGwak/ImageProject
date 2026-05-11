#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct tagAlgoHeightDiff// : HeightDiff
{
	RECT rcFirstROI;
	RECT rcSeccondROI;
	double dHeightDiff3D;
	double dHeightDiff3DMax;

	BOOL m_bDiffUpper;	// 높을떄 검사
	BOOL m_bDiffLower;	// 낮을때 검사

	BOOL m_bPolarity;
	BOOL m_bSignInversion;
	int m_nMinMaxflag;
	float m_fAddHeight;
	struct tagAlgoBlobBase m_sBlobBase;
	struct tagAlgoBlobBase m_sBlobBase_ROI2;
	BOOL m_bUseErodeFilter;
	int m_nErodeFilter;
	BOOL m_bUseBW_ROI2;
	int m_nSelectRoi;

	tagAlgoHeightDiff()
	{
		rcFirstROI.bottom = 0;
		rcFirstROI.left = 0;
		rcFirstROI.right = 0;
		rcFirstROI.top = 0;

		rcSeccondROI.bottom = 0;
		rcSeccondROI.left = 0;
		rcSeccondROI.right = 0;
		rcSeccondROI.top = 0;

		dHeightDiff3D = 0.0;
		dHeightDiff3DMax = 0.0;

		m_bDiffUpper = false;
		m_bDiffLower = false;

		m_bPolarity = FALSE;
		m_bSignInversion = TRUE;
		m_nMinMaxflag = 0;
		m_fAddHeight = 0.0f;
		m_bUseErodeFilter = false;
		m_nErodeFilter = 0;
		m_bUseBW_ROI2 = FALSE;
		m_nSelectRoi = -1;
	}
}AlgoHeightDiff;

typedef struct tagRstAlgoHeightDiff	// Height Diff 검사 결과
{
	double m_dRst_1;
	double m_dRst_2;
	double m_dRstHeightDiff;
	 
	BOOL m_bCheckPolarity;
	RECT m_rcRect_I[4];

	BOOL m_bOK;
	tagRstAlgoHeightDiff()
	{
		m_dRst_1 = 0.;
		m_dRst_2 = 0.;
		m_dRstHeightDiff = 0.;
		m_bCheckPolarity = FALSE;
		m_bOK = FALSE;
	}
	void Init()
	{
		m_dRst_1 = 0.;
		m_dRst_2 = 0.;
		m_dRstHeightDiff = 0.;
		m_bCheckPolarity = FALSE;
		m_bOK = FALSE;
	}
}RstAlgoHeightDiff;