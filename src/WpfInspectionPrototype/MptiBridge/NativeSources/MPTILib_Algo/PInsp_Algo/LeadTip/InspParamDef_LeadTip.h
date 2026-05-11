#pragma once
#include "../InspParamDef_AlgoBase.h"

enum LeadTipOpt
{
	eLeadTip_Default,
	eLeadTip_Tip_Length,
	eLeadTip_Tip_Cap,
	eLeadTip_Tip_All,
};

typedef struct tagAlgoLeadTip
{
	int nLeadTipPos;
	int nSearchRange;
	int nLeadDirection;

	BOOL bInvertCheck;
	int nPixelPercentValue;

	int n2dRange;
	int n2dBinaryMin;
	int n2dBinaryMax;

	BOOL b3dCheck;
	int n3dRange;
	double n3dHeightMin;
	double n3dHeightMax;
	double d3dAvgHeight;

	BOOL bTipFaultNG;
	BOOL m_b2dCheck;
	int m_nLeadPercentValue;
	int m_bSearchDirBody;
	int m_nNGTipOption;

	BOOL m_b3DIn2dCheck;			// 2D 적용 여부
	int m_n3DIn2DRange;				// 2D Range
	int m_n3DIn2DMinValue;			// 2D Threshold Min.
	int m_n3DIn2DMaxValue;			// 2D Threshold Max.

	BOOL m_bUseSideTip;
	BOOL m_bUseLeadWindowSizeChange;
	BOOL m_bUseSideTipShift;
	double m_dSideTipSearchArea;
	double m_dSideTipShiftX;
	double m_dSideTipShiftY;

	int m_nTipFindOpt;
	float m_fTipLength;
	float m_fTipCapGap;

	float m_fSideTipPos;
	float m_fSideTipGap;

	// Side Tip Bin
	BOOL m_bUseSideBin;
	BOOL m_b2dCheck_Side;
	byte m_n2dRange_Side;
	byte m_n2dBinaryMin_Side;
	byte m_n2dBinaryMax_Side;
	BOOL m_b3dCheck_Side;
	byte m_n3dRange_Side;
	float m_f3dHeightMin_Side;
	float m_f3dHeightMax_Side;

	int m_nMinBlobArea;
	float m_fSideTipGapWidth;

	BOOL m_bTipMaster;
	float m_fTMAllowRange;
	int m_nRealOffset;
	BOOL m_bSideTip_TipFind;
	BOOL m_bDirInvert;
	double m_dSideTipShiftXPer;
	double m_dSideTipShiftYPer;
	BOOL m_bUseSideTipShiftUnitPer;
	tagAlgoLeadTip()
	{
		nLeadTipPos = 0;
		nSearchRange = 10;
		nLeadDirection = 0;
		bInvertCheck = FALSE;
		nPixelPercentValue = 50;

		n2dRange = 0;
		n2dBinaryMin = 125;
		n2dBinaryMax = 255;

		b3dCheck = FALSE;
		n3dRange = 0;
		n3dHeightMin = 80;
		n3dHeightMax = 120;
		d3dAvgHeight = 0.0;

		bTipFaultNG = FALSE;
		m_b2dCheck = TRUE;
		m_nLeadPercentValue = 50;
		m_nNGTipOption = 0;

		m_b3DIn2dCheck = FALSE;
		m_n3DIn2DRange = 0;
		m_n3DIn2DMinValue = 125;
		m_n3DIn2DMaxValue = 255;

		m_bSearchDirBody = 0;

		m_bUseSideTip = FALSE;
		m_bUseLeadWindowSizeChange = FALSE;
		m_bUseSideTipShift = FALSE;
		m_dSideTipSearchArea = 120;
		m_dSideTipShiftX = 0.3;
		m_dSideTipShiftY = 0.3;

		m_nTipFindOpt = 0;
		m_fTipLength = 0.0f;
		m_fTipCapGap = 50.0f;

		m_fSideTipPos = -10;
		m_fSideTipGap = -10;

		m_bUseSideBin = FALSE;
		m_b2dCheck_Side = FALSE;
		m_n2dRange_Side = 0;
		m_n2dBinaryMin_Side = 0;
		m_n2dBinaryMax_Side = 0;
		m_b3dCheck_Side = FALSE;
		m_n3dRange_Side = 0;
		m_f3dHeightMin_Side = 0.0f;
		m_f3dHeightMax_Side = 0.0f;

		m_nMinBlobArea = 0;
		m_fSideTipGapWidth = 0.0f;

		m_bTipMaster = FALSE;
		m_nRealOffset = 0;
		m_fTMAllowRange = 0;
		m_bSideTip_TipFind = FALSE;
		m_bDirInvert = FALSE;
		m_dSideTipShiftXPer = 0.0;
		m_dSideTipShiftYPer = 0.0;
		m_bUseSideTipShiftUnitPer = FALSE;
	}
}AlgoLeadTip;

typedef struct tagRstAlgoLeadTip	// Lead Tip 검사 결과
{
	BOOL m_bRstFindOK;			// Lead Tip 찾기 성공 여부
	int m_nRstLeadTipPos;			// 찾은 Lead Tip 위치
	int m_nRstLeadTipPos_Length;			// 찾은 Lead Tip 위치
	BOOL m_bTipFaultNG;

	BOOL m_bSideTipOK;
	BOOL m_bSideTipShiftXOK;
	BOOL m_bSideTipShiftYOK;
	double m_dRstSideTipShiftX;
	double m_dRstSideTipShiftY;
	double m_dRstSideTipSize;
	double m_dRstSideTipPos;

	POINTF m_poDrawLine_T[2];
	POINTF m_poDrawLine_I[2];
	POINTF m_poDrawLine_Side[2];
	BOOL m_bLeadTipMaster;
	int m_nOrgTipMasterPos;
	int m_nOrgRstLeadTipPos;
	double m_dRstSideTipShiftXPer;
	double m_dRstSideTipShiftYPer;
	tagRstAlgoLeadTip()
	{
		m_bRstFindOK = FALSE;
		m_nRstLeadTipPos = 0;
		m_nRstLeadTipPos_Length = 0;
		m_bTipFaultNG = FALSE;

		m_bSideTipOK = FALSE;
		m_bSideTipShiftXOK = FALSE;
		m_bSideTipShiftYOK = FALSE;
		m_dRstSideTipShiftX = 0;
		m_dRstSideTipShiftY = 0;
		m_dRstSideTipSize = 0;
		m_dRstSideTipPos = 0;
		m_bLeadTipMaster = FALSE;
		m_nOrgTipMasterPos = 0;
		m_nOrgRstLeadTipPos = 0;
		m_dRstSideTipShiftXPer = 0;
		m_dRstSideTipShiftYPer = 0;
	}
}RstAlgoLeadTip;


typedef struct tagLeadTipSideTip
{
	UCHAR *pucPartImgSrc;
	float *pfPartImgSrc;
	int nImgWidth;
	int nImgHeight;
	int nPartImgWidth;
	int nPartImgHeight;
	double dWndX;
	double dWndY;
	tagLeadTipSideTip()
	{
		nImgWidth = 0;
		nImgHeight = 0;
		nPartImgWidth = 0;
		nPartImgHeight = 0;
		dWndX = 0;
		dWndY = 0;
	}
}LeadTipSideTip;

typedef struct tagRstLeadTipSideTip
{
	BOOL m_bSideTipResult;
	double m_dRstShiftX;
	double m_dRstShiftY;
	double m_dRstSideWidth;
	double m_dRstSideStartPos;
	float fHeightMeanBuf;
	POINTF m_poDrawLine_Side[2];
	int nStartX;
	int nStartY;
	int nStartX3D;
	int nStartY3D;
	int nSideTipLength;
	int m_nSideTip;
	double m_dRstShiftXPer;
	double m_dRstShiftYPer;
	tagRstLeadTipSideTip()
	{
		m_bSideTipResult = FALSE;
		m_dRstShiftX = 0;
		m_dRstShiftY = 0;
		m_dRstSideWidth = 0;
		m_dRstSideStartPos = 0;
		fHeightMeanBuf = 0;
		nStartX = 0;
		nStartY = 0;
		nStartX3D = 0;
		nStartY3D = 0;
		nSideTipLength = 0;
		m_nSideTip = 0;
		m_dRstShiftXPer = 0;
		m_dRstShiftYPer = 0;
	}
}RstLeadTipSideTip;
