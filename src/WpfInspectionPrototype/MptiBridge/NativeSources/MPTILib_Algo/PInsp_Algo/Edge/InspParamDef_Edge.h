#pragma once
#include "../InspParamDef_AlgoBase.h"

enum m_eEdgeData
{
	m_eEdgeData_UseExceptAngle = 0x01,
	m_eEdgeData_AngleBetweenLines = 0x02,
};

typedef struct stAlgoEdge
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

	int m_nSetLineCnt;
	BOOL m_bGroup;

	BOOL m_bArrIsHorizon[EdgeLineTotalCnt];
	int m_nArrMeasureDirection[EdgeLineTotalCnt];
	int m_nArrSetInspCondition[EdgeInspCnt];

	POINTF m_poArrSetTeachCenter[EdgeLineTotalCnt];
	double m_dArrTeachLength[EdgeLineTotalCnt];

	BOOL m_bUseAngle;						// Angle NG 검사 유무
	double m_dTeachRotate;

	int m_nLineFindType;
	double m_dLineFindRate;

	BOOL m_bDistanceX;
	double m_dTeachDistanceX;
	double m_dTeachDistanceXRateMin;
	double m_dTeachDistanceXRateMax;
	BOOL m_bDistanceY;
	double m_dTeachDistanceY;
	double m_dTeachDistanceYRateMin;
	double m_dTeachDistanceYRateMax;
	BOOL m_bFindCenter;
	BOOL m_bFillHole;

	double m_dTeach1_3PtDis;
	double m_dTeach1_3PtDisMin;
	double m_dTeach1_3PtDisMax;
	double m_dTeach1_3PtDisOff;
	double m_dTeach3_3PtDis;
	double m_dTeach3_3PtDisMin;
	double m_dTeach3_3PtDisMax;
	double m_dTeach3_3PtDisOff;

	BOOL m_bUseCrossCenter;
	double m_dStdTeachRotate;
	int m_nInspOption;
	stAlgoEdge()
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

		m_nSetLineCnt = 0;
		m_bGroup = TRUE;

		m_bUseAngle = TRUE;
		m_dTeachRotate = 10.;

		memset(m_bArrIsHorizon, FALSE, sizeof(BOOL) * EdgeLineTotalCnt);
		memset(m_nArrMeasureDirection, 0, sizeof(int) * EdgeLineTotalCnt);
		memset(m_nArrSetInspCondition, 0, sizeof(int) * EdgeInspCnt);
		memset(m_dArrTeachLength, 0, sizeof(double) * EdgeLineTotalCnt);
		memset(m_poArrSetTeachCenter, 0, sizeof(POINTF) * EdgeLineTotalCnt);

		m_nLineFindType = 0;
		m_dLineFindRate = 100;

		m_bDistanceX = FALSE;
		m_dTeachDistanceX = 0;
		m_dTeachDistanceXRateMin = 0;
		m_dTeachDistanceXRateMax = 0;
		m_bDistanceY = FALSE;
		m_dTeachDistanceY = 0;
		m_dTeachDistanceYRateMin = 0;
		m_dTeachDistanceYRateMax = 0;
		m_bFindCenter = FALSE;

		m_bUseCrossCenter = FALSE;
		m_dStdTeachRotate = 0.0;
		m_nInspOption = 0;
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nInspOption & nType) == nType);
		return bRet;
	}	
}AlgoEdge;

typedef struct tagRstAlgoEdge // Edge 검사 결과
{
	double m_dRstShiftX;
	double m_dRstShiftY;
	double m_dRstRealAngle;
	double m_dRstAngle;
	double m_dRstLength[EdgeLineTotalCnt];
	double m_dRstDistance;
	double m_dRstDistanceX;
	double m_dRstDistanceY;
	POINTF m_poDrawLine[EdgeLineTotalCnt];
	POINTF m_poDrawLine_Sec[EdgeLineTotalCnt];
	// OK 판정 //
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;
	BOOL m_bOKAngle;
	BOOL m_bMissing;
	BOOL m_bOKLength;
	BOOL m_bArrOKLength[EdgeLineTotalCnt];
	BOOL m_bDistance;
	BOOL m_bDistanceX;
	BOOL m_bDistanceY;
	POINTF m_poDrawLine_T[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T2[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T3[2];
	POINTF m_poDrawCenter;
	tagRstAlgoEdge()
	{
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstRealAngle = 0.;
		m_dRstAngle = 0.;
		memset(m_dRstLength, 0, sizeof(double) * EdgeLineTotalCnt);
		m_dRstDistance = 0.;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bMissing = FALSE;
		m_bDistance = FALSE;
		m_bDistanceX = FALSE;
		m_bDistanceY = FALSE;
		memset(m_bArrOKLength, FALSE, sizeof(BOOL) * EdgeLineTotalCnt);
		memset(m_poDrawLine_T, 0, sizeof(POINTF) * EdgeLineTotalCnt);
		memset(m_poDrawLine_T2, 0, sizeof(POINTF) * EdgeLineTotalCnt);
		memset(m_poDrawLine_T3, 0, sizeof(POINTF) * 2);
	}
	void Init()
	{
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstRealAngle = 0.;
		m_dRstAngle = 0.;
		m_dRstDistance = 0.;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bMissing = FALSE;
		m_bDistance = FALSE;
		m_bDistanceX = FALSE;
		m_bDistanceY = FALSE;
		//memset(m_dRstLength, 0, sizeof(double) * EdgeLineTotalCnt);
		//memset(m_bArrOKLength, FALSE, sizeof(BOOL) * EdgeLineTotalCnt);
		//memset(m_poDrawLine_T, 0, sizeof(POINTF) * EdgeLineTotalCnt);
		//memset(m_poDrawLine_T2, 0, sizeof(POINTF) * EdgeLineTotalCnt);
		//memset(m_poDrawLine_T3, 0, sizeof(POINTF) * 2);
	}
}RstAlgoEdge;