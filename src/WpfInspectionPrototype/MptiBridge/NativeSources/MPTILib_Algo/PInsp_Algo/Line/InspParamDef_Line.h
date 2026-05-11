#pragma once
#include "../InspParamDef_AlgoBase.h"

enum m_eLineData
{
	m_eLineData_Use = 0x01,
	m_eLineData_Min = 0x02,
	m_eLineData_Max = 0x04,
};

typedef struct stAlgoLine
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

	struct tagAlgoColorBase m_sAlgoColorBase;
	// 	int m_nHeightDivisionLevel;

	BOOL	m_bIsModelSubImg;
	BOOL	m_bUsePattern;
	wchar_t m_sModelPath[MAX_STRLEN];

	BOOL m_bIsHorizon;
	int m_nMeasureDirection;

	BOOL m_bUseAngle;						// Angle NG 검사 유무
	double m_dTeachRotate;

	BOOL m_bUseFix;
	int m_nCrossOpt;
	byte m_byLineFindType[2];
	BOOL m_bUseEndPos;
	int m_nLineData;
	float m_fArrPerpendicular[eMMD_Total];
	BOOL m_bUseInspROI; // Line 찾을 때 지정된 ROI내에서만 찾는 옵션
	RECT m_rcInspROI;
	stAlgoLine()
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


		m_bIsModelSubImg = TRUE;
		m_bUsePattern = FALSE;
		// 		m_nHeightDivisionLevel = 1;

		m_bIsHorizon = FALSE;
		m_nMeasureDirection = 0;

		m_bUseAngle = TRUE;
		m_dTeachRotate = 10.;
		m_bUseFix = FALSE;
		m_nCrossOpt = 0;
		m_byLineFindType[0] = 0;
		m_byLineFindType[1] = 100;
		m_bUseEndPos = FALSE;
		m_nLineData = 0;
		memset(m_fArrPerpendicular, 0, sizeof(float) * eMMD_Total);
		m_bUseInspROI = FALSE;
		memset(&m_rcInspROI, 0, sizeof(RECT));
	}
}AlgoLine;

typedef struct tagRstAlgoLine // Line 검사 결과
{
	double m_dRstShiftX;
	double m_dRstShiftY;
	double m_dRstAngle;
	double m_dRstLength;
	POINTF m_poDrawLine[2];

	// OK 판정 //
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;
	BOOL m_bOKAngle;
	BOOL m_bMissing;
	BOOL m_bOKLength;

	POINTF m_poDrawLine_T[2];
	POINTF m_poDrawCenter;

	BOOL m_bOKWidth;
	byte m_byWidthCnt;
	double m_dRstCuttinhW[50];
	double m_dRstCuttinhL[50];
	RECT m_rcDrawWidth[50];

	BOOL m_bOKPerpendicular;
	float m_fRstPerpendicular;
	POINTF m_poDrawLine_2[2];
	tagRstAlgoLine()
	{
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstAngle = 0.;
		m_dRstLength = 0.;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bMissing = FALSE;
		m_bOKLength = FALSE;

		m_bOKWidth = FALSE;
		m_byWidthCnt = 0;
		memset(m_dRstCuttinhW, 0, sizeof(double) * 50);
		memset(m_dRstCuttinhL, 0, sizeof(double) * 50);
		memset(m_rcDrawWidth, 0, sizeof(RECT) * 50);
		m_bOKPerpendicular = FALSE;
		m_fRstPerpendicular = 0.0f;
		memset(m_poDrawLine_2, 0, sizeof(POINTF) * 2);
	}
	void Init()
	{
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstAngle = 0.;
		m_dRstLength = 0.;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bMissing = FALSE;
		m_bOKLength = FALSE;

		m_bOKWidth = FALSE;
		m_byWidthCnt = 0;
		m_bOKPerpendicular = FALSE;
		m_fRstPerpendicular = 0.0f;
		//memset(m_dRstCuttinhW, 0, sizeof(double) * 50);
		//memset(m_dRstCuttinhL, 0, sizeof(double) * 50);
		//memset(m_rcDrawWidth, 0, sizeof(RECT) * 50);
		//memset(m_poDrawLine_2, 0, sizeof(POINTF) * 2);
	}
}RstAlgoLine;