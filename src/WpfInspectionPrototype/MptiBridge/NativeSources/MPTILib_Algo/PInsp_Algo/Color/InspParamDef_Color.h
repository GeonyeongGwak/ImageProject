#pragma once
#include "../InspParamDef_AlgoBase.h"

#define LIGHT_CNT				10
#define LIGHT_DATA_CNT			6

typedef struct tagColorDecisionList
{
	BOOL colorRateOk;
	BOOL polarityOK;

}ColorDecisionList;

// typedef struct tagLightData
// {
// 	byte m_byLightCnt;
// 	byte m_byArrLightData[LIGHT_DATA_CNT][LIGHT_CNT];
// }lightData;

typedef struct tagRstInspColor
{
	int colorInspType;

	double rstRate;
	double rstTotalRate;

	BOOL isInsp;
	BOOL ok;	// 1: GOOD ,  0: NG
	//int wndDefectCode;
	ColorDecisionList list;

}RstInspColor;

typedef struct tagInspParamColor
{
	double wndAngle;

	int colorInspType;

	//BOOL enabled;
	POINT polygonPt[COLORALGO_POLYGON_CNTS][POLYGON_POINT_CNTS];
	int m_nPolygonCnt;
	int pointCnts;
	BOOL isPolarity;

	float redFator;
	float greenFator;
	float blueFator;
	float redFactorBtm;
	float blueFactorBtm;

	int sizeX_CIE;
	int sizeY_CIE;

	//histo add
	int rangeMode; // 0: greater, 1:less, 2:in range, 3:out range
	int maxRange;
	int minRange;
	int stdHisroCnt;


	//allow data
	double stdRate;

	//return
	RstInspColor retInspColorResult;  //teach test only

	CAtlArray <CRect> m_rcArrROI;
	bool m_bUseColorMap2;
	BOOL m_bUseRangeBar;
	BOOL m_bUseRGB[COLORALGO_POLYGON_CNTS];
	byte m_byRange[COLORALGO_POLYGON_CNTS];
	byte m_byMin[COLORALGO_POLYGON_CNTS];
	byte m_byMax[COLORALGO_POLYGON_CNTS];
	BOOL m_bInvert;
	byte m_byColorLightType;
}InspParamColor;

typedef struct tagTeachParamColor
{
	double wndAngle;

	int colorInspType;
	//teach
	POINT polygonPt[COLORALGO_POLYGON_CNTS][POLYGON_POINT_CNTS];
	int pointCnts;

	float redFator;
	float greenFator;
	float blueFator;
	float redFactorBtm;
	float blueFactorBtm;

	int sizeX_CIE;
	int sizeY_CIE;

	//histo add
	int rangeMode; // 0:in range, 1:out range, 2: upper, 3:lower, 
	int maxRange;
	int minRange;


	//colorXY view
	int viewColor;

	//return
	double rstRate;
	int rstHisroCnt;

	CAtlArray <CRect> m_rcArrROI;
	CAtlArray <double> m_dArrArea1;
	CAtlArray <double> m_dArrArea3;
	CAtlArray <double> m_dWidth;
	bool m_bUseColorMap2;
	BOOL m_bUseRangeBar;
	BOOL m_bUseRGB[COLORALGO_POLYGON_CNTS];
	byte m_byRange[COLORALGO_POLYGON_CNTS];
	byte m_byMin[COLORALGO_POLYGON_CNTS];
	byte m_byMax[COLORALGO_POLYGON_CNTS];
	BOOL m_bInvert;
	byte m_byColorLightType;
}TeachParamColor;

typedef struct tagAlgoColor
{
	double m_dWndAngle;
	int m_nInspTypeColor;						// 0 : color, 1 : Gray
	int m_nViewColor;							//colorXY view

	POINT m_ptArrPolygon[COLORALGO_POLYGON_CNTS][POLYGON_POINT_CNTS];
	int m_nPolygonCnt;
	int m_nCntPoint;
	BOOL m_bUsePolarity;

	float m_fFatorRed;
	float m_fFatorGreen;
	float m_fFatorBlue;
	float m_fRebFactorBtm;
	float m_fBlueFactorBtm;

	int m_nSizeXCIE;
	int m_nSizeYCIE;

	//histo add
	int m_nRangeMode;							// 0: greater, 1:less, 2:in range, 3:out range
	int m_nRangeMax;
	int m_nRangeMin;

	//Inspect
	int m_nCntHistoStd;
	double m_dRateStd;							//allow data
	//RstInspColor	retInspColorResult;			//return

	//Teach Result
	int m_nCntHistoResult;
	double m_dRateResult;

	// copa 용
	int m_nLeadTipDirection;		// Lead 방향
	int m_nLeadPosition;			// Lead 위치
	int m_nSolderLength;			// leadtip 으로 부터 solder까지의 거리

	int m_nTypeTab;
	int m_nCntRect;
	RECT m_rcArrTabRect[BRIEDGE_CNT];
	BOOL m_bAutoSearchROI;
	BOOL m_bUseColorMap2;
	BOOL m_bUseRangeBar;
	BOOL m_bUseRGB[COLORALGO_POLYGON_CNTS];
	byte m_byRange[COLORALGO_POLYGON_CNTS];
	byte m_byMin[COLORALGO_POLYGON_CNTS];
	byte m_byMax[COLORALGO_POLYGON_CNTS];
	BOOL m_bInvert;
	byte m_byColorLightType;
	BOOL m_bUseIntensity;
	int m_nRangeType;
	int m_nRangeMinIntens;
	int m_nRangeMaxIntens;
	enum ETypeGep
	{
		eTab,
		eTabSpace,
		eColor
	};

	tagAlgoColor()
	{
		m_dWndAngle = 0.0;
		m_nInspTypeColor = 0;

		m_nCntPoint = 0;
		m_bUsePolarity = FALSE;

		m_fFatorRed = 0.0;
		m_fFatorGreen = 0.0;
		m_fFatorBlue = 0.0;

		m_nSizeXCIE = 0;
		m_nSizeYCIE = 0;

		m_nRangeMode = 0; // 0: greater, 1:less, 2:in range, 3:out range
		m_nRangeMax = 0;
		m_nRangeMin = 0;
		m_nCntHistoStd = 0;

		m_dRateStd = 0.0;

		m_nSolderLength = 10;
		m_nLeadTipDirection = 0;
		m_nLeadPosition = 0;

		m_nTypeTab = eColor;
		m_nCntRect = 0;
		m_bAutoSearchROI = FALSE;
		m_bUseColorMap2 = FALSE;
		m_bUseRangeBar = FALSE;
		m_bInvert = FALSE;
		m_byColorLightType = 0;
		m_bUseIntensity = FALSE;
		m_nRangeType = eTypeRangeIn;
		m_nRangeMinIntens = 0;
		m_nRangeMaxIntens = 80;
		for (int n = 0; n < COLORALGO_POLYGON_CNTS; n++)
		{
			m_bUseRGB[n] = FALSE;
			m_byRange[n] = 0;
			m_byMin[n] = 0;
			m_byMax[n] = 0;
		}
	}

	void GetParam(InspParamColor *pParamInsp)
	{
		if (!pParamInsp)
			return;

		pParamInsp->wndAngle = m_dWndAngle;
		pParamInsp->colorInspType = m_nInspTypeColor;
		pParamInsp->m_nPolygonCnt = m_nPolygonCnt;
		memcpy(pParamInsp->polygonPt, m_ptArrPolygon, sizeof(POINT) * COLORALGO_POLYGON_CNTS * POLYGON_POINT_CNTS);
		pParamInsp->pointCnts = m_nCntPoint;
		pParamInsp->isPolarity = m_bUsePolarity;
		pParamInsp->redFator = m_fFatorRed;
		pParamInsp->greenFator = m_fFatorGreen;
		pParamInsp->blueFator = m_fFatorBlue;
		pParamInsp->redFactorBtm = m_fRebFactorBtm;
		pParamInsp->blueFactorBtm = m_fBlueFactorBtm;
		pParamInsp->sizeX_CIE = m_nSizeXCIE;
		pParamInsp->sizeY_CIE = m_nSizeYCIE;
		pParamInsp->rangeMode = m_nRangeMode; // 0: greater, 1:less, 2:in range, 3:out range
		pParamInsp->maxRange = m_nRangeMax;
		pParamInsp->minRange = m_nRangeMin;
		pParamInsp->stdHisroCnt = m_nCntHistoStd;
		pParamInsp->stdRate = m_dRateStd;
		pParamInsp->m_bUseColorMap2 = m_bUseColorMap2;
		pParamInsp->m_rcArrROI.RemoveAll();

		for (int i = 0; i < m_nCntRect; ++i)
		{
			CRect rcROI(m_rcArrTabRect[i]);
			pParamInsp->m_rcArrROI.Add(rcROI);
		}
		pParamInsp->m_bUseRangeBar = m_bUseRangeBar;
		pParamInsp->m_bInvert = m_bInvert;
		pParamInsp->m_byColorLightType = m_byColorLightType;
		for (int n = 0; n < COLORALGO_POLYGON_CNTS; n++)
		{
			pParamInsp->m_bUseRGB[n] = m_bUseRGB[n];
			pParamInsp->m_byRange[n] = m_byRange[n];
			pParamInsp->m_byMin[n] = m_byMin[n];
			pParamInsp->m_byMax[n] = m_byMax[n];
		}
	}

	void GetParam(TeachParamColor *pParamTeach)
	{
		if (!pParamTeach)
			return;

		pParamTeach->wndAngle = m_dWndAngle;
		pParamTeach->colorInspType = m_nInspTypeColor;
		memcpy(pParamTeach->polygonPt, m_ptArrPolygon, sizeof(POINT) * COLORALGO_POLYGON_CNTS * POLYGON_POINT_CNTS);
		pParamTeach->pointCnts = m_nCntPoint;
		pParamTeach->redFator = m_fFatorRed;
		pParamTeach->greenFator = m_fFatorGreen;
		pParamTeach->blueFator = m_fFatorBlue;
		pParamTeach->redFactorBtm = m_fRebFactorBtm;
		pParamTeach->blueFactorBtm = m_fBlueFactorBtm;
		pParamTeach->sizeX_CIE = m_nSizeXCIE;
		pParamTeach->sizeY_CIE = m_nSizeYCIE;
		pParamTeach->rangeMode = m_nRangeMode;
		pParamTeach->minRange = m_nRangeMin;
		pParamTeach->maxRange = m_nRangeMax;
		pParamTeach->viewColor = m_nViewColor;
		pParamTeach->rstRate = m_dRateResult;
		pParamTeach->rstHisroCnt = m_nCntHistoResult;
		pParamTeach->m_bUseColorMap2 = m_bUseColorMap2;
		pParamTeach->m_rcArrROI.RemoveAll();
		pParamTeach->m_dArrArea1.RemoveAll();
		pParamTeach->m_dArrArea3.RemoveAll();
		pParamTeach->m_dWidth.RemoveAll();
		for (int i = 0; i < m_nCntRect; ++i)
		{
			CRect rcROI(m_rcArrTabRect[i]);
			pParamTeach->m_rcArrROI.Add(rcROI);
		}
		pParamTeach->m_bUseRangeBar = m_bUseRangeBar;
		pParamTeach->m_bInvert = m_bInvert;
		pParamTeach->m_byColorLightType = m_byColorLightType;
		for (int n = 0; n < COLORALGO_POLYGON_CNTS; n++)
		{
			pParamTeach->m_bUseRGB[n] = m_bUseRGB[n];
			pParamTeach->m_byRange[n] = m_byRange[n];
			pParamTeach->m_byMin[n] = m_byMin[n];
			pParamTeach->m_byMax[n] = m_byMax[n];
		}
	}

	BOOL IsUseMultiArea()
	{
		//if (m_nCntRect <= 0)				// Lead_Color에서 nTypeTab가 eTab으로 인식되는 문제가 있어서 여기서 먼저 체크
		//	return FALSE;
		if ((m_nTypeTab == eTab)
			|| (m_nTypeTab == eTabSpace))
			return TRUE;
		if (m_nCntRect > 0)				// nTypeTab먼저 체크하고 하자
			return TRUE;

		return FALSE;
	}
}AlgoColor;


typedef struct tagRstAlgoColor	// Color 검사 결과
{
	double m_dRstRate;

	// OK 판정 //
	BOOL m_bOKPolarity;
	BOOL m_bRateOK;
	RECT m_rcRect_I;
	tagRstAlgoColor()
	{
		m_dRstRate = 0.;

		m_bOKPolarity = FALSE;
		m_bRateOK = FALSE;
	}
}RstAlgoColor;
