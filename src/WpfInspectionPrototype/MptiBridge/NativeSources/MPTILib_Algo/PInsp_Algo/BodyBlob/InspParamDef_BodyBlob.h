#pragma once
#include "../InspParamDef_AlgoBase.h"

#define  dfBodyBlobShapeAreaCnt		3
enum m_eAlgoBB
{
	m_eAlgoBB_Find = 0x01,
};
typedef struct stAlgoBodyBlob
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
	float m_fHeightAvg;

	int m_nTypeRange3D;	// in 0, out 1, upper 2, lower 3

	double m_dTechCenterX;
	double m_dTechCenterY;

	BOOL m_bTeachWidthUse;
	double m_dTeachWidth;
	double m_dTeachWidthRateMin;
	double m_dTeachWidthRateMax;

	BOOL m_bTeachLengthUse;
	double m_dTeachLength;
	double m_dTeachLengthRateMin;
	double m_dTeachLengthRateMax;

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
	int m_nTeachHeightAreaRate;

	struct tagAlgoColorBase m_sAlgoColorBase;
	// 	int m_nHeightDivisionLevel;

	BOOL	m_bIsModelSubImg;
	BOOL	m_bUsePattern;
	wchar_t m_sModelPath[MAX_STRLEN];

	int m_nExtractRange;
	double m_dSearchMargin;

	double m_dTeachRotate;
	double m_dStandardRotate;

	BOOL m_bUseAngle;						// Angle NG 검사 유무

	BOOL m_bUseDamage;
	double m_dDamageHeight;
	double m_dDamageArea;
	BOOL m_bShiftMaxUse; // LMJ 2017/05/22

	// Body Tip
	BOOL m_bUseBodyTip;
	BOOL m_bOnlyBodyTip;
	BOOL m_bTip2dCheck;
	byte m_nTip2dRange;
	byte m_nTip2dMinValue;
	byte m_nTip2dMaxValue;
	BOOL m_bTip3dCheck;
	byte m_nTip3dRange;
	float m_dTip3dHeightMin;
	float m_dTip3dHeightMax;
	BOOL m_bIsHorizon;    // true : 가로(극성 양옆), false : 세로(극성 위아래)

	BOOL m_bUseShape;
	float m_fTeachShapeHeightMax;
	float m_fTeachShapeArea;

	BOOL m_bUseDamage2D;
	BOOL m_bUseDamageArea;

	// Body Per
	BOOL m_b3dPerCheck;
	byte m_n3dPerRange;
	float m_f3dPerHeightMin;
	float m_f3dPerHeightMax;
	float m_f3dPerHeightAvg;

	// Pattern
// 	BOOL m_bUsePatternbody;
// 	wchar_t m_sbodyModelPath[MAX_STRLEN];

	BOOL m_bRChipWithOCR;
	float m_fSearchDefault;
	float m_fSearchBodyTip;
	BOOL m_bNGOffset;
	
	BOOL m_bUseOffsetDistance;
	float m_fOffsetDistance;

	BOOL m_bNotUseBoundaryAngle;
	double m_dFiduAngle;
	int m_nOPT;
	double m_dShiftX_SC;
	double m_dShiftY_SC;
	stAlgoBodyBlob()
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
		m_fHeightAvg = 0.0F;
		m_nTypeRange3D = eTypeRangeIn;

		m_dTechCenterX = 0.0;
		m_dTechCenterY = 0.0;

		m_bUseIPC = FALSE;

		m_bTeachWidthUse = FALSE;
		m_dTeachWidth = 0.0;
		m_dTeachWidthRateMin = 80.;
		m_dTeachWidthRateMax = 120.;
		m_bTeachLengthUse = FALSE;
		m_dTeachLength = 0.0;
		m_dTeachLengthRateMin = 80.;
		m_dTeachLengthRateMax = 120.;

		m_bUseBlobNG = FALSE;
		m_dBlobSizeWidth = 0.f;
		m_dBlobSizeLength = 0.f;

		m_bShiftXUse = TRUE;
		m_bShiftYUse = TRUE;

		m_bUseBlobNG = FALSE;
		m_dBlobSizeWidth = 0.0;
		m_dBlobSizeLength = 0.0;

		m_bUseHeight = FALSE;
		m_dTeachHeight = 0.;
		m_dTeachHeightMax = 0.;
		m_dTeachHeightMin = 0.;
		m_nTeachHeightAreaRate = 100;


		m_bIsModelSubImg = TRUE;
		m_bUsePattern = FALSE;
		// 		m_nHeightDivisionLevel = 1;

		m_nExtractRange = 20;
		m_dTechCenterX = 0.0;
		m_dTechCenterY = 0.0;

		m_dTeachRotate = 10.;
		m_dStandardRotate = 0.0;

		m_bUseAngle = TRUE;

		m_bUseDamage = FALSE;
		m_dDamageHeight = 100.0;
		m_dDamageArea = 10.0;

		m_bUseBodyTip = FALSE;
		m_bTip2dCheck = TRUE;
		m_nTip2dRange = eTypeRangeUpper;
		m_nTip2dMinValue = 0;
		m_nTip2dMaxValue = 180;
		m_bTip3dCheck = TRUE;
		m_nTip3dRange = eTypeRangeUpper;
		m_dTip3dHeightMin = 0;
		m_dTip3dHeightMax = 30;
		m_bIsHorizon = TRUE;

		m_bUseShape = FALSE;
		m_fTeachShapeHeightMax = 0;
		m_fTeachShapeArea = 0;

		m_bUseDamage2D = FALSE;
		m_bUseDamageArea = FALSE;

		m_b3dPerCheck = FALSE;
		m_n3dPerRange = eTypeRangeUpper;
		m_f3dPerHeightMin = 80.0f;
		m_f3dPerHeightMax = 0.0f;
		m_f3dPerHeightAvg = 0.0f;

		m_bUsePattern = FALSE;

		m_bRChipWithOCR = FALSE;
		m_fSearchDefault = 50;
		m_fSearchBodyTip = 50;
		m_bNGOffset = FALSE;

		m_bUseOffsetDistance = FALSE;
		m_fOffsetDistance = 0.3f;

		m_bNotUseBoundaryAngle = FALSE;
		m_dFiduAngle = 0;
		m_dShiftX_SC = 0;
		m_dShiftY_SC = 0;
	}
	bool UseData(int nType)
	{
		if ((m_nOPT & nType) == nType)
			return true;

		return false;
	}
}AlgoBodyBlob;

typedef struct tagRstAlgoBodyBlob	// BodyBlob 검사 결과
{
	double m_dRstWidth;
	double m_dRstLength;
	double m_dRstShiftX;
	double m_dRstShiftY;
	float m_fRstShiftMaxX;
	float m_fRstShiftMaxY;
	double m_dRstCenterX;
	double m_dRstCenterY;
	double m_dRstAngle;
	double m_dRstArea;
	double m_dRstAreaRate;
	double m_dRstHeightMean;
	double m_dRstDamageRate;
	double m_dRstDamageConditionRate;
	double m_dRstDamageRate_Per;
	int m_nShapeAreaCnt;
	double m_dRstShapeArea[dfBodyBlobShapeAreaCnt];
	double m_dRstShapeArea_Per[dfBodyBlobShapeAreaCnt];
	double m_dRstHeightMean_Per;
	double m_dWNDA;

	// OK 판정 //
	BOOL m_bOKWidth;
	BOOL m_bOKLength;
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;
	BOOL m_bOKShiftMaxX;
	BOOL m_bOKShiftMaxY;
	BOOL m_bOKAngle;
	BOOL m_bOKArea;
	BOOL m_bOKHeight;
	BOOL m_bBillboarding;
	BOOL m_bTombstone;
	BOOL m_bOKDamage;
	BOOL m_bOKShape;
	BOOL m_bShowRect;

	RECT m_rcBodyRect;
	RECT m_rcInspBodyRect;
	RECT m_rcBodyDamageRect;
	RECT m_rcRect_T;
	RECT m_rcBodyShapeRect[dfBodyBlobShapeAreaCnt];
	POINTF m_poDrawCenter;

	BOOL m_bOKOffsetDistance;
	float m_fOffsetDistance;

	tagRstAlgoBodyBlob()
	{
		m_dRstWidth = 0.;
		m_dRstLength = 0.;
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstCenterX = 0.;
		m_dRstCenterY = 0.;
		m_dRstAngle = 0.;
		m_dRstHeightMean = 0.;
		m_dRstAreaRate = 0.;
		m_dRstDamageRate = 0.;
		m_dRstDamageConditionRate = 0.;
		m_bOKWidth = FALSE;
		m_bOKLength = FALSE;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKShiftMaxX = FALSE;
		m_bOKShiftMaxY = FALSE;
		m_bOKAngle = FALSE;
		m_bOKHeight = FALSE;
		m_bBillboarding = FALSE;
		m_bTombstone = FALSE;
		m_bOKDamage = FALSE;
		m_bOKShape = FALSE;
		memset(m_dRstShapeArea, 0, dfBodyBlobShapeAreaCnt * sizeof(double));
		memset(m_rcBodyShapeRect, 0, dfBodyBlobShapeAreaCnt * sizeof(RECT));

		m_bOKOffsetDistance = FALSE;
		m_fOffsetDistance = 0.0f;
	}
	void Init()
	{
		m_dRstWidth = 0.;
		m_dRstLength = 0.;
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstCenterX = 0.;
		m_dRstCenterY = 0.;
		m_dRstAngle = 0.;
		m_dRstHeightMean = 0.;
		m_dRstAreaRate = 0.;
		m_dRstDamageRate = 0.;
		m_dRstDamageConditionRate = 0.;
		m_bOKWidth = FALSE;
		m_bOKLength = FALSE;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKShiftMaxX = FALSE;
		m_bOKShiftMaxY = FALSE;
		m_bOKAngle = FALSE;
		m_bOKHeight = FALSE;
		m_bBillboarding = FALSE;
		m_bTombstone = FALSE;
		m_bOKDamage = FALSE;
		m_bOKShape = FALSE;
		m_bOKOffsetDistance = FALSE;
		m_fOffsetDistance = 0.0f;
	}
	void CalcAreaRate(double dWidthWnd, double dHeightWnd, double dSearchMargin)
	{
		if (m_dRstArea <= 0)
		{
			m_dRstAreaRate = 0.0;
		}
		else
		{
			m_dRstAreaRate = m_dRstArea /
				((dWidthWnd - dSearchMargin) *
				(dHeightWnd - dSearchMargin)) * 100;
		}
	}
	BOOL ShiftChk(double dMin, double dMax, bool bX, bool bMax)
	{
		double dRst = bX ? m_dRstShiftX * -1.0 : m_dRstShiftY;
		if (bMax)
			dRst = bX ? m_fRstShiftMaxX : m_fRstShiftMaxY;

		if (dRst >= dMax)
			return false;

		if (dMin < 0 && dRst < 0)
		{
			if (abs(dRst) >= abs(dMin))
				return false;
		}
		else
		{
			if (dRst <= dMin)
				return false;
		}
		return true;
	}
}RstAlgoBodyBlob;