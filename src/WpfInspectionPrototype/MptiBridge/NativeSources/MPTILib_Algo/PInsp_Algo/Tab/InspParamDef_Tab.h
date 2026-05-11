#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct tagAlgoTab
{
	// copa 용
	int m_nLeadTipDirection;		// Lead 방향
	int m_nLeadPosition;			// Lead 위치
	int m_nSolderLength;			// leadtip 으로 부터 solder까지의 거리

	int m_nCntRect;
	RECT m_rcArrTabRect[BRIEDGE_CNT];
	BOOL m_bAutoSearchROI;

	double m_dArrTailArea;//m_dArrArea1[BRIEDGE_CNT];
	double m_dArrMarginErrArea;//m_dArrArea3[BRIEDGE_CNT];
	double m_dWidth[BRIEDGE_CNT];
	double m_dLine;

	//bin
	struct tagAlgoBlobBase m_sBlobBase_TabArea;
	struct tagAlgoBlobBase m_sBlobBase_NG;
	struct tagAlgoBlobBase m_sBlobBase_NG2;

	BOOL m_bArrUseLine[BRIEDGE_CNT];
	BOOL m_bArrUseWidth[BRIEDGE_CNT];
	BOOL m_bArrUseTab[BRIEDGE_CNT];
	BOOL m_bArrUseTail[BRIEDGE_CNT];

	int m_nHist1;
	int m_nHist2;

	BOOL m_bUseMinScarThickness;
	double m_dMinScarThickness;
	BOOL m_bUseScarAspectRatio;
	BOOL m_bUseDisableTabArea;
	double m_dScarAspectRatio;
	BOOL m_bUseCrossLineDetect;
	BOOL m_bContraction;

	BOOL m_bUseVerticalMaxLen;
	double m_dVerticalMaxLen;
	BOOL m_bUseHorizontalMaxLen;
	double m_dHorizontalMaxLen;
	BOOL m_bUseMaxNGArea;
	double m_dMaxNGArea;
	BOOL m_bUseEarseScarArea;
	int m_nStartIdx;
	int m_nCriticalArea;
	int m_nMaxCippingLeng;

	float m_fstdWidth;
	BOOL m_bUseChippingCriticalLine;
	BOOL m_bUseTieBarROpt;

	BOOL m_bUseTabAreaOther;

	enum ETypeGep
	{
		eTab,
		eTabSpace,
		eColor
	};

	tagAlgoTab()
	{

		m_nSolderLength = 10;
		m_nLeadTipDirection = 0;
		m_nLeadPosition = 0;

		m_nCntRect = 0;
		m_bAutoSearchROI = FALSE;
		m_nHist1 = 0;
		m_nHist2 = 0;

		m_bUseMinScarThickness = FALSE;
		m_dMinScarThickness = 0.0;
		m_bUseScarAspectRatio = FALSE;
		m_bUseDisableTabArea = FALSE;
		m_bUseCrossLineDetect = FALSE;

		m_bUseVerticalMaxLen = FALSE;
		m_dVerticalMaxLen = 0.0;
		m_bUseHorizontalMaxLen = FALSE;
		m_dHorizontalMaxLen = 0.0;
		m_bUseMaxNGArea = FALSE;
		m_dMaxNGArea = 0.0;
		m_bUseEarseScarArea = FALSE;
		m_nCriticalArea = 100;
		m_nMaxCippingLeng = 10;
		m_bUseChippingCriticalLine = FALSE;
		m_bUseTieBarROpt = TRUE;

		m_bUseTabAreaOther = FALSE;
	}

	BOOL IsUseMultiArea()
	{
		return TRUE;
	}
}AlgoTab;

typedef struct tagRstAlgoTab	// Tab 검사 결과
{
	// OK 판정 //
	int m_nCntRect;					// Gap Rect  개수
	BOOL m_bTabRst[BRIEDGE_CNT];	// 각 Gap Rect  Bridge 결과

	POINTF m_pofNGRectCenter[BRIEDGE_CNT];
	POINTF m_pofNGRectSize[BRIEDGE_CNT];
	RECT m_rcArrGapRect[BRIEDGE_CNT];
	double m_dArrTailArea[BRIEDGE_CNT];
	double m_dArrMarginErrArea[BRIEDGE_CNT];
	double m_dWidth[BRIEDGE_CNT];
	double m_dLine[BRIEDGE_CNT];
	int m_nArrMaxFreqValue1[BRIEDGE_CNT];
	int m_nArrMaxFreqValue2[BRIEDGE_CNT];

	int m_nNGAreaRoiCnt;
	RECT m_rcArrNGArrRect[BRIEDGE_CNT];

	BOOL m_bUseWidth[BRIEDGE_CNT];
	BOOL m_bUseErrArea[BRIEDGE_CNT];
	BOOL m_bUseLine[BRIEDGE_CNT];
	BOOL m_bUseTail[BRIEDGE_CNT];

	BOOL m_bRstWidth[BRIEDGE_CNT];
	BOOL m_bRstErrArea[BRIEDGE_CNT];
	BOOL m_bRstLine[BRIEDGE_CNT];
	BOOL m_bRstTail[BRIEDGE_CNT];

	int m_rcArrNGArrRectCnt[BRIEDGE_CNT];
	RECT m_rcArrCalcRect[BRIEDGE_CNT];
	tagRstAlgoTab()
	{
		m_nCntRect = 0;

		memset(m_bTabRst, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseWidth, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseErrArea, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseLine, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseTail, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstWidth, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstErrArea, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstLine, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstTail, 0, sizeof(BOOL) * BRIEDGE_CNT);

		memset(m_pofNGRectCenter, 0, sizeof(POINTF) * BRIEDGE_CNT);
		memset(m_pofNGRectSize, 0, sizeof(POINTF) * BRIEDGE_CNT);
		memset(m_rcArrGapRect, 0, sizeof(RECT) * BRIEDGE_CNT);
		memset(m_dArrTailArea, 0, sizeof(double) * BRIEDGE_CNT);
		memset(m_dArrMarginErrArea, 0, sizeof(double) * BRIEDGE_CNT);
		memset(m_dWidth, 0, sizeof(double) * BRIEDGE_CNT);
		memset(m_dLine, 0, sizeof(double) * BRIEDGE_CNT);

		memset(m_nArrMaxFreqValue1, 0, sizeof(int) * BRIEDGE_CNT);

		memset(m_nArrMaxFreqValue2, 0, sizeof(int) * BRIEDGE_CNT);
		memset(m_rcArrNGArrRect, 0, sizeof(RECT) * BRIEDGE_CNT);

		memset(m_rcArrNGArrRectCnt, 0, sizeof(int) * BRIEDGE_CNT);
		memset(m_rcArrCalcRect, 0, sizeof(RECT) * BRIEDGE_CNT);
	}
}RstAlgoTab;

typedef struct tagRstInspTab
{
	int colorInspType;

	BOOL isInsp;
	BOOL ok;	// 1: GOOD ,  0: NG

	int m_nCntRect;					// Gap Rect  개수
	BOOL m_bTabRst[BRIEDGE_CNT];	// 각 Gap Rect  Bridge 결과

	POINTF m_pofNGRectCenter[BRIEDGE_CNT];
	POINTF m_pofNGRectSize[BRIEDGE_CNT];
	RECT m_rcArrGapRect[BRIEDGE_CNT];
	double m_dArrTailArea[BRIEDGE_CNT];
	double m_dArrMarginErrArea[BRIEDGE_CNT];
	double m_dWidth[BRIEDGE_CNT];
	double m_dLine[BRIEDGE_CNT];
	int m_nArrMaxFreqValue1[BRIEDGE_CNT];
	int m_nArrMaxFreqValue2[BRIEDGE_CNT];
	//UCHAR * m_ucResultImg;

	BOOL m_bUseWidth[BRIEDGE_CNT];
	BOOL m_bUseErrArea[BRIEDGE_CNT];
	BOOL m_bUseLine[BRIEDGE_CNT];
	BOOL m_bUseTail[BRIEDGE_CNT];

	BOOL m_bRstWidth[BRIEDGE_CNT];
	BOOL m_bRstErrArea[BRIEDGE_CNT];
	BOOL m_bRstLine[BRIEDGE_CNT];
	BOOL m_bRstTail[BRIEDGE_CNT];

	int m_nNGAreaRoiCnt;
	RECT m_rcArrNGArrRect[BRIEDGE_CNT];

	int m_rcArrNGArrRectCnt[BRIEDGE_CNT];
	RECT m_rcArrCalcRect[BRIEDGE_CNT];
	struct tagRstInspTab& operator=(const struct tagRstInspTab& result)
	{
		colorInspType = result.colorInspType;
		isInsp = result.isInsp;
		ok = result.ok;
		m_nCntRect = result.m_nCntRect;

		for (int i = 0; i < BRIEDGE_CNT; ++i)
		{
			m_bTabRst[i] = result.m_bTabRst[i];
			m_pofNGRectCenter[i].x = result.m_pofNGRectCenter[i].x;
			m_pofNGRectCenter[i].y = result.m_pofNGRectCenter[i].y;
			m_pofNGRectSize[i].x = result.m_pofNGRectSize[i].x;
			m_pofNGRectSize[i].y = result.m_pofNGRectSize[i].y;
			m_dArrTailArea[i] = result.m_dArrTailArea[i];
			m_dArrMarginErrArea[i] = result.m_dArrMarginErrArea[i];
			m_dWidth[i] = result.m_dWidth[i];
			m_dLine[i] = result.m_dLine[i];
			m_nArrMaxFreqValue1[i] = result.m_nArrMaxFreqValue1[i];
			m_nArrMaxFreqValue2[i] = result.m_nArrMaxFreqValue2[i];

			m_bUseWidth[i] = result.m_bUseWidth[i];
			m_bUseErrArea[i] = result.m_bUseErrArea[i];
			m_bUseLine[i] = result.m_bUseLine[i];
			m_bUseTail[i] = result.m_bUseTail[i];

			m_bRstWidth[i] = result.m_bRstWidth[i];
			m_bRstErrArea[i] = result.m_bRstErrArea[i];
			m_bRstLine[i] = result.m_bRstLine[i];
			m_bRstTail[i] = result.m_bRstTail[i];

			m_rcArrNGArrRect[i] = result.m_rcArrNGArrRect[i];
			m_rcArrNGArrRectCnt[i] = result.m_rcArrNGArrRectCnt[i];
			m_rcArrCalcRect[i] = result.m_rcArrCalcRect[i];
		}
		//m_ucResultImg = result.m_ucResultImg;
		m_nNGAreaRoiCnt = result.m_nNGAreaRoiCnt;
		return *this;
	}
	tagRstInspTab()
	{
		m_nCntRect = 0;

		memset(m_bTabRst, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseWidth, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseErrArea, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseLine, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bUseTail, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstWidth, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstErrArea, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstLine, 0, sizeof(BOOL) * BRIEDGE_CNT);
		memset(m_bRstTail, 0, sizeof(BOOL) * BRIEDGE_CNT);

		memset(m_pofNGRectCenter, 0, sizeof(POINTF) * BRIEDGE_CNT);
		memset(m_pofNGRectSize, 0, sizeof(POINTF) * BRIEDGE_CNT);
		memset(m_rcArrGapRect, 0, sizeof(RECT) * BRIEDGE_CNT);
		memset(m_dArrTailArea, 0, sizeof(double) * BRIEDGE_CNT);
		memset(m_dArrMarginErrArea, 0, sizeof(double) * BRIEDGE_CNT);
		memset(m_dWidth, 0, sizeof(double) * BRIEDGE_CNT);
		memset(m_dLine, 0, sizeof(double) * BRIEDGE_CNT);

		memset(m_nArrMaxFreqValue1, 0, sizeof(int) * BRIEDGE_CNT);

		memset(m_nArrMaxFreqValue2, 0, sizeof(int) * BRIEDGE_CNT);
		memset(m_rcArrNGArrRect, 0, sizeof(RECT) * BRIEDGE_CNT);

		memset(m_rcArrNGArrRectCnt, 0, sizeof(int) * BRIEDGE_CNT);
		memset(m_rcArrCalcRect, 0, sizeof(RECT) * BRIEDGE_CNT);
	}
}RstInspTab;