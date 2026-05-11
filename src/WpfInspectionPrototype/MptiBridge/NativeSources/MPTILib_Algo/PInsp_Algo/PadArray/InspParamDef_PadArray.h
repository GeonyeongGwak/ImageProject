#pragma once
#include "../InspParamDef_AlgoBase.h"

#define	MAX_PAD_CNT			6
enum m_ePadArray_by
{
	PadArray_by_Data,
	PadArray_by_Filter,
	PadArray_by_Min2D,
	PadArray_by_Max2D,
	PadArray_by_Range2D,
	PadArray_by_Range3D,
	PadArray_by_Row, //PAD
	PadArray_by_Column, //PAD
	PadArray_by_Total
};

enum m_ePadArray_Data
{
	PadArray_Data_Bin2D = 1 << 0,
	PadArray_Data_Bin3D = 1 << 1,
	PadArray_Data_Filter = 1 << 2,
	PadArray_Data_FillHoll = 1 << 3,
	PadArray_Data_UseShift = 1 << 4,
	PadArray_Data_UseShiftX = 1 << 5,
	PadArray_Data_UseShiftY = 1 << 6,
	PadArray_Data_UseWidth = 1 << 7,
	PadArray_Data_UseLength = 1 << 8,
	PadArray_Data_UseDistanceX = 1 << 9,
	PadArray_Data_UseDistanceY = 1 << 10,
	PadArray_Data_UseHeightDiffMin = 1 << 11,
	PadArray_Data_UseHeightDiffMax = 1 << 12,
};

enum m_ePadArray_F
{
	PadArray_F_Min3D,
	PadArray_F_Max3D,
	PadArray_F_ShiftX,
	PadArray_F_ShiftY,
	PadArray_F_CenterX,
	PadArray_F_CenterY,
	PadArray_F_Width,
	PadArray_F_Width_MIN,
	PadArray_F_Width_MAX,
	PadArray_F_Length,
	PadArray_F_Length_MIN,
	PadArray_F_Length_MAX,
	PadArray_F_TeachDistanceX, //PAD 우측 distance
	PadArray_F_TeachDistanceX_MIN,
	PadArray_F_TeachDistanceX_MAX,
	PadArray_F_TeachDistanceY, //PAD 아래 distance
	PadArray_F_TeachDistanceY_MIN,
	PadArray_F_TeachDistanceY_MAX,
	PadArray_F_HeightAvg,
	PadArray_F_HeightMIN,
	PadArray_F_HeightMAX,
	PadArray_F_Total
};

typedef struct stAlgoPadArray
{
	byte narrdata[PadArray_by_Total];
	float farrdata[PadArray_F_Total];
	int idata;

	RECT rcFirstROI[MAX_PAD_CNT];
	RECT rcSecondROI[MAX_PAD_CNT];

	struct tagAlgoColorBase m_sAlgoColorBase;
	int m_nCntPatternPath;

	stAlgoPadArray()
	{
		memset(narrdata, 0, PadArray_by_Total * sizeof(byte));
		memset(farrdata, 0, PadArray_F_Total * sizeof(float));
		idata = 0;			
		memset(rcFirstROI, 0, MAX_PAD_CNT * sizeof(RECT));
		memset(rcSecondROI, 0, MAX_PAD_CNT * sizeof(RECT));
	}
}AlgoPadArray;

typedef struct tagRstAlgoPadArray
{
	float dRstWidth[MAX_PAD_CNT];
	float dRstLength[MAX_PAD_CNT];
	float dRstDistanceX[MAX_PAD_CNT];
	float dRstDistanceY[MAX_PAD_CNT];

	float dRstShiftX;
	float dRstShiftY;
	float dRstCenterPointX;
	float dRstCenterPointY;

	int m_nArrRectCnt;

	// OK 판정 //
	BOOL m_bOKWidth[MAX_PAD_CNT];
	BOOL m_bOKLength[MAX_PAD_CNT];
	BOOL m_bOKDistanceX[MAX_PAD_CNT];
	BOOL m_bOKDistanceY[MAX_PAD_CNT];
	BOOL m_bOKHeightDiff[MAX_PAD_CNT];
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;

	RECT m_rcArrRect[MAX_PAD_CNT];
	POINTF m_poDrawCenter;

	//height diff
	RECT m_rcHeightROI1[MAX_PAD_CNT];
	RECT m_rcHeightROI2[MAX_PAD_CNT];
	float m_heightMean1[MAX_PAD_CNT];
	float m_heightMean2[MAX_PAD_CNT];

	tagRstAlgoPadArray()
	{
		memset(dRstWidth, 1, MAX_PAD_CNT * sizeof(float));
		memset(dRstLength, 1, MAX_PAD_CNT * sizeof(float));
		memset(dRstDistanceX, 1, MAX_PAD_CNT * sizeof(float));
		memset(dRstDistanceY, 1, MAX_PAD_CNT * sizeof(float));

		dRstShiftX = 0.;
		dRstShiftY = 0.;
		dRstCenterPointX = 0.;
		dRstCenterPointY = 0.;

		memset(m_bOKWidth, TRUE, MAX_PAD_CNT * sizeof(BOOL));
		memset(m_bOKLength, TRUE, MAX_PAD_CNT * sizeof(BOOL));
		memset(m_bOKDistanceX, TRUE, MAX_PAD_CNT * sizeof(BOOL));
		memset(m_bOKDistanceY, TRUE, MAX_PAD_CNT * sizeof(BOOL));
		memset(m_bOKHeightDiff, TRUE, MAX_PAD_CNT * sizeof(BOOL));
		m_bOKShiftX = TRUE;
		m_bOKShiftY = TRUE;

		m_nArrRectCnt = 0;
		memset(m_rcArrRect, 1, MAX_PAD_CNT * sizeof(RECT));
		m_poDrawCenter.x = 0.;
		m_poDrawCenter.y = 0.;

		memset(m_rcHeightROI1, 0, MAX_PAD_CNT * sizeof(POINTF));
		memset(m_rcHeightROI2, 0, MAX_PAD_CNT * sizeof(POINTF));
		memset(m_heightMean1, 0, MAX_PAD_CNT * sizeof(float));
		memset(m_heightMean2, 0, MAX_PAD_CNT * sizeof(float));
	}

}RstAlgoPadArray;