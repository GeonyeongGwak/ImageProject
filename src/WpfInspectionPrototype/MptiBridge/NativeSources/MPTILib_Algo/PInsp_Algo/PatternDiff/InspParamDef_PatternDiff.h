#pragma once
#include "../InspParamDef_AlgoBase.h"

//★★ 코드 작성 필요
enum m_ePatternDiffData
{
	m_ePatternDiffData_Use2D = 0x01,
	m_ePatternDiffData_Use3D = 0x02,
	m_ePatternDiffData_Shift = 0x04,
	m_ePatternDiffData_Angle = 0x08,
	m_ePatternDiffData_Total = 0x0F,
};
enum m_ePatternDiffN
{
	m_ePatternDiffN_Data,
	m_ePatternDiffN_Range2D,
	m_ePatternDiffN_Range3D,
	m_ePatternDiffN_Min2D,
	m_ePatternDiffN_Max2D,
	m_ePatternDiffN_Total,
};
enum m_ePatternDiffF
{
	m_ePatternDiffF_Min3D,
	m_ePatternDiffF_Max3D,
	m_ePatternDiffF_Angle,
	m_ePatternDiffF_ShiftX,
	m_ePatternDiffF_ShiftY,
	m_ePatternDiffF_Total,
};
enum m_ePatternDiffPO
{
	m_ePatternDiffPO_Teach1,
	m_ePatternDiffPO_Teach2,
	m_ePatternDiffPO_Size,
	m_ePatternDiffPO_Total,
};

typedef struct tagAlgoPatternDiff
{
	int m_nLayerCnt;
	struct tagLayerPolygon m_stPolygon[MAX_LAYERCNT];
	int m_nArrLayerType[MAX_LAYERCNT];
	struct tagAlgoBlobBase m_stBlobBase[MAX_LAYERCNT];
	double m_dTeachLength[MAX_LAYERCNT];
	double m_dTeachWidth[MAX_LAYERCNT];
	double m_dTeachArea[MAX_LAYERCNT];
	BOOL m_bUsePattern;
	RECT m_rcMatchingArea;
	int m_nModelAddCnt;
	wchar_t m_sModelPath[MAX_STRLEN];
	double m_dTheta;
	BOOL m_bUseDarkForeign;
	BOOL m_bUseFilter[MAX_LAYERCNT];
	BOOL m_bNGBlobMode[MAX_LAYERCNT];
	struct tagHistoParamBase m_stHistoBase[MAX_LAYERCNT];
	wchar_t m_sModelPath1[MAX_STRLEN];
	wchar_t m_sModelPath2[MAX_STRLEN];
	wchar_t m_sModelPath3[MAX_STRLEN];
	wchar_t m_sModelPath4[MAX_STRLEN];
	wchar_t m_sModelPath5[MAX_STRLEN];
	wchar_t m_sModelPath6[MAX_STRLEN];
	wchar_t m_sModelPath7[MAX_STRLEN];
	wchar_t m_sModelPath8[MAX_STRLEN];
	wchar_t m_sModelPath9[MAX_STRLEN];
	wchar_t m_sModelPath10[MAX_STRLEN];
	wchar_t m_sModelPath11[MAX_STRLEN];
	wchar_t m_sModelPath12[MAX_STRLEN];
	wchar_t m_sModelPath13[MAX_STRLEN];
	wchar_t m_sModelPath14[MAX_STRLEN];
	wchar_t m_sModelPath15[MAX_STRLEN];
	wchar_t m_sModelPath16[MAX_STRLEN];

	int nSelectedModel;					//OK model index Before Auto InspSeq. If Teaching Seq, This Param must 0.
	BOOL m_bUseSizeAnd[MAX_LAYERCNT];

	//ExceptPattern for Chipping
	BOOL m_bUseExcPattern;
	int m_nExcModelAddCnt;
	int m_nExcPatternScore;

	tagModelPath m_stExcPath;
	lightData m_stArrLightData[MAX_LAYERCNT];
	BOOL m_bArrUseMergeLayer[MAX_LAYERCNT];
	BOOL m_bUseEdgeFilter[MAX_LAYERCNT];
	int m_nFilterSize[MAX_LAYERCNT];
	int m_nEdgeFilterLevel[MAX_LAYERCNT];

	RECT m_rcArrAlignArea[MAX_ALIGNCNT];
	BOOL m_bUseArrAlignArea;
	int m_nAcceptAlignScore;
	BOOL m_bPatternMatching; //매칭검사여부결정
	POINTF ptMatchingCenter;


	BOOL m_bUseMaskBW[MAX_LAYERCNT];
	struct tagAlgoBlobBase m_stBlobBase_Mask;
	BOOL m_UseFilter_Mask;
	int m_nFilterSize_Mask;

	tagAlgoPatternDiff()
	{
		m_nLayerCnt = 0;
		m_bUsePattern = FALSE;
		m_rcMatchingArea = { 0, 0, 0, 0 };
		m_nModelAddCnt = 0;
		m_dTheta = 0.0;
		m_bUseDarkForeign = FALSE;

		m_bUseExcPattern = FALSE;
		m_nExcModelAddCnt = 0;
		m_nExcPatternScore = 0;

		m_bUseArrAlignArea = FALSE;
		nSelectedModel = 0;
		m_nAcceptAlignScore = 80;

		memset(m_stPolygon, 0, MAX_LAYERCNT * sizeof(tagLayerPolygon));
		memset(m_nArrLayerType, 0, MAX_LAYERCNT * sizeof(int));
		memset(m_stBlobBase, 0, MAX_LAYERCNT * sizeof(tagAlgoBlobBase));
		memset(m_dTeachLength, 0, MAX_LAYERCNT * sizeof(double));
		memset(m_dTeachWidth, 0, MAX_LAYERCNT * sizeof(double));
		memset(m_dTeachArea, 0, MAX_LAYERCNT * sizeof(double));
		memset(m_bUseFilter, 0, MAX_LAYERCNT * sizeof(BOOL));
		memset(m_bNGBlobMode, 0, MAX_LAYERCNT * sizeof(BOOL));
		memset(m_stHistoBase, 0, MAX_LAYERCNT * sizeof(tagHistoParamBase));
		memset(m_bUseSizeAnd, 0, MAX_LAYERCNT * sizeof(BOOL));
		memset(&m_stArrLightData, 0, MAX_LAYERCNT * sizeof(lightData));
		memset(m_bArrUseMergeLayer, 0, MAX_LAYERCNT * sizeof(BOOL));
		memset(m_bUseEdgeFilter, 0, MAX_LAYERCNT * sizeof(BOOL));
		memset(m_nFilterSize, 0, MAX_LAYERCNT * sizeof(int));
		memset(m_nEdgeFilterLevel, 0, MAX_LAYERCNT * sizeof(int));
		memset(m_rcArrAlignArea, 0, MAX_ALIGNCNT * sizeof(RECT));

		m_bPatternMatching = true;
		ptMatchingCenter.x = 0.0f;
		ptMatchingCenter.y = 0.0f;
		memset(m_bUseMaskBW, 0, MAX_LAYERCNT * sizeof(BOOL));

		m_UseFilter_Mask = FALSE;
		m_nFilterSize_Mask = 0;
		memset(&m_stBlobBase_Mask, 0, sizeof(tagAlgoBlobBase));
	}
	CString GetModelPath(int nModelIdx)
	{
		CString str = _T("");

		if (nModelIdx >= this->m_nModelAddCnt || nModelIdx >= MAX_MODELCNT)
			return str;

		switch (nModelIdx)
		{
		case 0: str.Format(_T("%s"),m_sModelPath1); break;
		case 1: str.Format(_T("%s"),m_sModelPath2); break;
		case 2: str.Format(_T("%s"),m_sModelPath3); break;
		case 3: str.Format(_T("%s"),m_sModelPath4); break;
		case 4: str.Format(_T("%s"),m_sModelPath5); break;
		case 5: str.Format(_T("%s"),m_sModelPath6); break;
		case 6: str.Format(_T("%s"),m_sModelPath7); break;
		case 7: str.Format(_T("%s"),m_sModelPath8); break;
		case 8: str.Format(_T("%s"),m_sModelPath9); break;
		case 9: str.Format(_T("%s"),m_sModelPath10); break;
		case 10: str.Format(_T("%s"),m_sModelPath11); break;
		case 11: str.Format(_T("%s"),m_sModelPath12); break;
		case 12: str.Format(_T("%s"),m_sModelPath13); break;
		case 13: str.Format(_T("%s"),m_sModelPath14); break;
		case 14: str.Format(_T("%s"),m_sModelPath15); break;
		case 15: str.Format(_T("%s"),m_sModelPath16); break;

		}
		return str;
	}

}AlgoPatternDiff;

typedef struct tagRstAlgoPatternDiff // PatternDiff 검사 결과
{

	BOOL m_bOK;
	int m_nRectCnt;

	POINTF m_ptArrRstPtr[BLOB_RECT_CNTS][4];
	BOOL m_bArrRstLayer[MAX_LAYERCNT];
	double m_dRstLength[BLOB_RECT_CNTS];
	double m_dRstWidth[BLOB_RECT_CNTS];
	double m_dRstArea[BLOB_RECT_CNTS];
	int m_nArrDefectLayerNum[BLOB_RECT_CNTS];
	double m_dTheta;
	int m_nSelectedModelIdx;
	BOOL m_bRstChipping;
	double m_dRstPatternScore[BLOB_RECT_CNTS];
	POINTF m_ptRstShiftXY;
	POINTF m_ptRstMatchingCenter;
	double m_dAlignScore;
	double m_dMatchingScore[MAX_ALIGNCNT];
	BOOL m_bMatchingOK;
	tagRstAlgoPatternDiff()
	{
		m_bOK = FALSE;
		m_nRectCnt = 0;
		m_dTheta = 0.0;
		m_ptRstShiftXY.x = 0;
		m_ptRstShiftXY.y = 0;

		memset(m_ptArrRstPtr, 0, BLOB_RECT_CNTS * 4 * sizeof(POINTF));
		memset(m_bArrRstLayer, 0, MAX_LAYERCNT * sizeof(BOOL));
		memset(m_dRstLength, 0, BLOB_RECT_CNTS * sizeof(double));
		memset(m_dRstWidth, 0, BLOB_RECT_CNTS * sizeof(double));
		memset(m_dRstArea, 0, BLOB_RECT_CNTS * sizeof(double));
		memset(m_nArrDefectLayerNum, 0, BLOB_RECT_CNTS * sizeof(int));
		memset(m_dRstPatternScore, 0, BLOB_RECT_CNTS * sizeof(double));

		m_nSelectedModelIdx = 0;
		m_bRstChipping = FALSE;
		m_ptRstMatchingCenter.x = 0;
		m_ptRstMatchingCenter.y = 0;
		m_dAlignScore = 0.;
		memset(m_dMatchingScore, 0, MAX_ALIGNCNT * sizeof(double));
		m_bMatchingOK = FALSE;
	}
}RstAlgoPatternDiff;