#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct stAlgoBGA
{
	// IPC
	BOOL m_bUseIPC;
	byte m_byIPCClass;
	BOOL m_bInvertCheck;

	BOOL m_bFilterIsUse;
	int m_nFilterStepNarrow;

	BOOL m_bShiftIsUse;
	double m_dShiftX;
	double m_dShiftY;

	BOOL m_bAreaIsUse;
	double m_dAreaMin;
	double m_dAreaMax;
	double m_dAreaCurrent;				//PercentOK

	int m_nTypeSelectBlob;
	BOOL m_bFillHole;
	// 2D
	BOOL m_bInsp2D;
	int m_nMinBinary;
	int m_nMaxBinary;
	int m_nTypeRange2D;
	// 3D
	BOOL m_bInsp3D;
	double m_dHeightRateMin;
	double m_dHeightRateMax;
	int m_nTypeRange3D;	// in 0, out 1, upper 2, lower 3

	BOOL m_bTeachWidthUse;
	double m_dTeachWidth;
	double m_dTeachWidthRateMin;
	double m_dTeachWidthRateMax;

	BOOL m_bTeachLengthUse;
	double m_dTeachLength;
	double m_dTeachLengthRateMin;
	double m_dTeachLengthRateMax;

	double m_dTeachArea;
	double m_dTeachVolume;

	BOOL m_bShiftXUse;
	BOOL m_bShiftYUse;

	BOOL m_bUseBlobNG;
	double m_dBlobSizeWidth;
	double m_dBlobSizeLength;
	BOOL m_bUseBlobSizeWidth;
	BOOL m_bUseBlobSizeLength;

	BOOL m_bUseHeight;
	double m_dTeachHeight;
	double m_dTeachHeightMax;
	double m_dTeachHeightMin;

	struct tagAlgoColorBase m_sAlgoColorBase;

	BOOL m_bUseCircleRate;
	double m_dTeachCircleRate;

	int m_bcoiningUse;

	BOOL m_bUseNGBump;
	int m_nNGBumpType;
	BOOL m_bNGBumpFillterUse;
	int m_nNGBumpFilterCnt;
	BOOL m_bUseNGBumpFillHole;
	BOOL m_InspNGBump2D;
	int m_NGBumpMinBinary;
	int m_NGBumpMaxBinary;
	int m_TypeRangeNGBump2D;
	BOOL m_InspNGBump3D;
	double m_NGBumpHeightRateMin;
	double m_NGBumpHeightRateMax;
	int m_TypeRangeNGBump3D;
	BOOL m_bUseNGBumpWidth;
	double m_dTeachNGBumpWidth;
	BOOL m_bUseNGBumpLength;
	double m_dTeachNGBumpLength;

	BOOL m_bUseUncoining;
	BOOL m_bUncoiningFillterUse;
	int m_nUncoiningFilterCnt;
	BOOL m_bUseUncoiningFillHole;
	BOOL m_InspUncoining2D;
	int m_UncoiningMinBinary;
	int m_UncoiningMaxBinary;
	int m_TypeRangeUncoining2D;
	BOOL m_InspUncoining3D;
	double m_UncoiningHeightRateMin;
	double m_UncoiningHeightRateMax;
	int m_TypeRangeUncoining3D;
	double m_dTeachUncoiningArea;
	int m_nInspType;
	BOOL m_bUseBridge;
	BOOL m_bUseCoplanarity;
	double m_CoplanarMax;
	double m_CoplanarMin;
	int m_nCoplOption;

	int m_nBallMaskSize;

	int m_HeightAreaType;
	int m_HeightAreaType_COP;
	int m_SelectOffSetType;
	BOOL m_bUseAbsoluteHeight;
	BOOL m_bUsePitch;
	double m_dTolPitch;

	stAlgoBGA()
	{
		m_byIPCClass = 0;
		m_bInvertCheck = FALSE;
		m_dAreaMin = 80.;
		m_dAreaMax = 120.;
		m_nTypeSelectBlob = eSelectCenter;
		m_bFillHole = FALSE;
		m_nFilterStepNarrow = 4;

		m_bInsp2D = TRUE;
		m_nMinBinary = 125;
		m_nMaxBinary = 255;
		m_nTypeRange2D = eTypeRangeIn;

		m_bInsp3D = FALSE;
		m_dHeightRateMin = 80;
		m_dHeightRateMax = 120;
		m_nTypeRange3D = eTypeRangeIn;

		m_bUseIPC = FALSE;

		m_bTeachWidthUse = FALSE;
		m_dTeachWidth = 0.0;
		m_dTeachWidthRateMin = 80.;
		m_dTeachWidthRateMax = 120.;
		m_bTeachLengthUse = FALSE;
		m_dTeachLength = 0.0;
		m_dTeachLengthRateMin = 80.;
		m_dTeachLengthRateMax = 120.;

		m_dTeachArea = 0.0;
		m_dTeachVolume = 0.0;

		m_bShiftXUse = TRUE;
		m_bShiftYUse = TRUE;

		m_bUseBlobNG = FALSE;
		m_dBlobSizeWidth = 0.0;
		m_dBlobSizeLength = 0.0;
		m_bUseBlobSizeWidth = FALSE;
		m_bUseBlobSizeLength = FALSE;

		m_bUseHeight = FALSE;
		m_dTeachHeight = 0.;
		m_dTeachHeightMax = 0.;
		m_dTeachHeightMin = 0.;

		m_bUseCircleRate = FALSE;
		m_dTeachCircleRate = 0.;
		m_bcoiningUse = 0;

		m_bUseNGBump = FALSE;
		m_nNGBumpType = 0;
		m_bNGBumpFillterUse = FALSE;
		m_nNGBumpFilterCnt = 0;
		m_bUseNGBumpFillHole = FALSE;
		m_InspNGBump2D = TRUE;
		m_NGBumpMinBinary = 125;
		m_NGBumpMaxBinary = 255;
		m_TypeRangeNGBump2D = eTypeRangeIn;
		m_InspNGBump3D = FALSE;
		m_NGBumpHeightRateMin = 0.;
		m_NGBumpHeightRateMax = 80.;
		m_TypeRangeNGBump3D = eTypeRangeIn;
		m_bUseNGBumpWidth = FALSE;
		m_dTeachNGBumpWidth = 0.;
		m_bUseNGBumpLength = FALSE;
		m_dTeachNGBumpLength = 0.;

		m_bUseUncoining = FALSE;
		m_bUncoiningFillterUse = FALSE;
		m_nUncoiningFilterCnt = 0;
		m_bUseUncoiningFillHole = FALSE;
		m_InspUncoining2D = TRUE;
		m_UncoiningMinBinary = 125;
		m_UncoiningMaxBinary = 255;
		m_TypeRangeUncoining2D = eTypeRangeIn;
		m_InspUncoining3D = FALSE;
		m_UncoiningHeightRateMin = 0.;
		m_UncoiningHeightRateMax = 80.;
		m_TypeRangeUncoining3D = eTypeRangeIn;
		m_dTeachUncoiningArea = 0.;
		m_nInspType = 0;
		m_bUseBridge = FALSE;
		m_bUseCoplanarity = FALSE;
		m_CoplanarMax = 0.0f;
		m_CoplanarMin = 0.0f;
		m_nCoplOption = 0;

		m_nBallMaskSize = -1;

		m_HeightAreaType = 0;
		m_HeightAreaType_COP = 0;
		m_SelectOffSetType = 0;

		m_bUseAbsoluteHeight = FALSE;
		m_bUsePitch = FALSE;
		m_dTolPitch = 0;
	}
}AlgoBGA;

typedef struct tagRstAlgoBGA	// BGA 검사 결과
{
	BOOL m_bCoplanarityOK;
	BOOL m_bOKGridOffsetX;
	BOOL m_bOKGridOffsetY;
	BOOL m_bOKTwist;

	float m_fCoplanarity;
	float m_fRstGridOffsetX;
	float m_fRstGridOffsetY;
	float m_fRstTwist;

	RECT m_rcRect_I;
	RECT m_rcMinCoplanarity;
	RECT m_rcMaxCoplanarity;

	float m_fArrRstCoplan[eMMA_Total];
	float m_fArrRstHeight[eMMA_Total];
	float m_fArrRstWidth[eMMA_Total];

	tagRstAlgoBGA()
	{
		m_bCoplanarityOK = FALSE;
		m_bOKGridOffsetX = FALSE;
		m_bOKGridOffsetY = FALSE;
		m_bOKTwist = FALSE;
		m_fCoplanarity = 0;
		m_fRstGridOffsetX = 0;
		m_fRstGridOffsetY = 0;
		m_fRstTwist = 0;

		for (int b = 0; b < eMMA_Total; b++)
		{
			m_fArrRstCoplan[b] = 0.0f;
			m_fArrRstHeight[b] = 0.0f;
			m_fArrRstWidth[b] = 0.0f;
		}
	}
}RstAlgoBGA;