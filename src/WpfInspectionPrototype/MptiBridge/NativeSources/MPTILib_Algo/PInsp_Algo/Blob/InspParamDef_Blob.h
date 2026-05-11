#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct stAlgoBlob
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
	BOOL m_bUseFPBW;
	byte m_byFPMargin;
	BOOL m_bCircleOpt;	// PYJ 2019/05/25

	BOOL m_bUseBlobSizeDistance;
	double m_dBlobSizeDistance;

	BOOL m_bInspCoil;
	BOOL m_bOKCoilRst;

	// 2D
	BOOL m_bInsp2D_pattern;
	int m_nMinBinary_pattern;
	int m_nMaxBinary_pattern;
	int m_nTypeRange2D_pattern;
	// 3D
	BOOL m_bInsp3D_pattern;
	double m_dHeightRateMin_pattern;
	double m_dHeightRateMax_pattern;

	int m_nTypeRange3D_pattern;	// in 0, out 1, upper 2, lower 3

	int m_nCoilThMin;
	int m_nCoilThMax;

	int m_nBlobSizeDistanceType;
	POINTF m_pShiftSt;
	POINTF m_pShiftEd;
	int m_nShiftDirIndex;

	int m_nCoilCntThresh;//코일검사시 파트 내 블랍 개수 기준
	byte m_byDir;
	struct tagAngleColorBase m_sAngleColorBase;
	byte m_byNGCnt;
	BOOL m_bUseCircleRate;
	double m_dCircleRate;
	BOOL m_bUseThickMax;
	BOOL m_bUseThickMin;
	float m_fThickMax;
	float m_fThickMin;
	int m_nThickDir;

	bool m_bInsp2D_ExceptArea;
	int m_nMinBinary_ExceptArea;
	int m_nMaxBinary_ExceptArea;
	int m_nTypeRange2D_ExceptArea;
	
	bool m_bInsp3D_ExceptArea;
	double m_dHeightRateMin_ExceptArea;
	double m_dHeightRateMax_ExceptArea;
	int m_nTypeRange3D_ExceptArea;
	int m_nMinBlobArea;
	int m_nExceptAreaCal;
	int m_nMinThickRemove;
	int m_nPoCnt;
	POINTF Po1;
	POINTF Po2;
	POINTF Po3;
	POINTF Po4;
	POINTF Po5;
	POINTF Po6;

	BOOL m_bUseOffsetDistance;
	float m_fOffsetDistance;

	BOOL m_bUsePinAngle;
	float m_fPinHeight;
	float m_fPinAngle;
	float m_fPinAngleRange;

	BOOL m_bUseHeightAvgMask;
	BOOL m_bUseTeachTh;
	stAlgoBlob()
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
		m_bUseFPBW = FALSE;
		m_byFPMargin = 30;

		m_bUseBlobSizeDistance = FALSE;
		m_dBlobSizeDistance = 0.0;

		m_bCircleOpt = FALSE;
		m_bInspCoil = FALSE;

		m_nCoilThMax = 0;
		m_nCoilThMin = 0;

		m_nBlobSizeDistanceType = 0;

		m_nCoilCntThresh = 1;
		m_byDir = 0;
		m_byNGCnt = 0;
		m_bUseCircleRate = FALSE;
		m_dCircleRate = 0.0;
		m_bUseThickMax = FALSE;
		m_bUseThickMin = FALSE;
		m_fThickMax = 0;
		m_fThickMin = 0;
		m_nThickDir = 0;

		m_bInsp2D_ExceptArea = 0;
		m_nMinBinary_ExceptArea = 0;
		m_nMaxBinary_ExceptArea = 0;
		m_nTypeRange2D_ExceptArea = 0;

		m_bInsp3D_ExceptArea = 0;
		m_dHeightRateMin_ExceptArea = 0;
		m_dHeightRateMax_ExceptArea = 0;
		m_nTypeRange3D_ExceptArea = 0;

		m_nMinBlobArea = 4;
		m_nExceptAreaCal = 0;

		m_bUseOffsetDistance = FALSE;
		m_fOffsetDistance = 0.3f;

		m_bUsePinAngle = FALSE;
		m_fPinAngle = 0.0f;
		m_fPinHeight = 0.0f;
		
		m_nPoCnt = 0;

		m_bUseHeightAvgMask = FALSE;
		m_bUseTeachTh = FALSE;
	}

	//bool UsethPo()
	//{
	//	bool bRes = (Po1.x != 0 || Po1.y != 0 || Po2.x != 0 || Po2.y != 0 || Po3.x != 0 || Po3.y != 0 || Po4.x != 0 || Po4.y != 0 || Po5.x != 0 || Po5.y != 0 || Po6.x != 0 || Po6.y != 0);
	//	return bRes;
	//}
}AlgoBlob;

typedef struct tagRstAlgoBlob	// Blob 검사 결과
{
	double m_dRstArea;
	double m_dRstShiftX;
	double m_dRstShiftY;
	double m_dRstAreaRate;
	double m_dRstHeightMean;

	// OK 판정 //
	BOOL m_bOKArea;
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;
	BOOL m_bOKHeight;

	double m_dRstWidth;
	double m_dRstLength;
	BOOL m_bOKWidth;
	BOOL m_bOKLength;

	BOOL m_bOKArea_Reverse;	// solder ball insp
	double m_dRstArea_Reverse;
	double m_dTeachArea_Reverse;

	RECT m_rcRect_I;

	BOOL m_bUseSizeWidth;
	BOOL m_bUseSizeLength;
	POINTF m_poDrawCenter;
	POINTF m_poDrawCenterPix;

	int m_nArrRectCnt;
	RECT m_rcRectT;
	RECT m_rcArrRect[BLOB_RECT_CNTS];

	BOOL m_bOKDistance;
	double m_dRstDistance;

	BOOL m_bOKCoilRst;

	POINTF m_pShiftSt;
	POINTF m_pShiftEd;
	int m_nShiftDirIndex;
	POINTF m_pFPMatchPos[4];
	struct tagRstInspAC m_sInspAC;
	byte m_nRstCnt;
	float m_fArrRstA[BLOB_RECT_CNTS];
	float m_fArrRstW[BLOB_RECT_CNTS];
	float m_fArrRstL[BLOB_RECT_CNTS];
	BOOL m_bOKCircleRate;
	double m_dRstCircleRate;

	BOOL m_bOKThickMax;
	float m_fRstThickMax;
	BOOL m_bOKThickMin;
	float m_fRstThickMin;
	//POINTF m_pThicknessPoint[255];
	POINTF Po1;// Top;
	POINTF Po2;//Bot;
	POINTF Po3;//Left;
	POINTF Po4;//Right;
	POINTF Po5;//DiagLeft;
	POINTF Po6;//DiagRight;

	BOOL m_bOKOffsetDistance;
	float m_fOffsetDistance;

	BOOL m_bOKPinAngle;
	float m_fRstPinAngle;
	float m_fRstPinHeight;
	tagRstAlgoBlob()
	{
		m_dRstArea = 0.;
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstAreaRate = 0.;
		m_dRstHeightMean = 0.;
		m_bOKArea = FALSE;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKHeight = FALSE;
		m_bOKCoilRst = FALSE;

		m_dRstWidth = 0.;
		m_dRstLength = 0.;
		m_dRstDistance = 0.;
		m_bOKWidth = FALSE;
		m_bOKLength = FALSE;
		m_bOKDistance = FALSE;
		m_bOKArea_Reverse = TRUE;	// always Start TRUE
		m_dRstArea_Reverse = 0.;
		m_dTeachArea_Reverse = 0.;

		m_bUseSizeWidth = FALSE;
		m_bUseSizeLength = FALSE;

		m_bOKThickMax = FALSE;
		m_bOKThickMin = FALSE;

		m_nArrRectCnt = 0;
		memset(m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));

		m_nShiftDirIndex = 0;
		memset(m_pFPMatchPos, 0, 4 * sizeof(POINTF));
		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
		memset(m_fArrRstA, 0.0, BLOB_RECT_CNTS * sizeof(float));
		memset(m_fArrRstW, 0.0, BLOB_RECT_CNTS * sizeof(float));
		memset(m_fArrRstL, 0.0, BLOB_RECT_CNTS * sizeof(float));
		m_bOKCircleRate = FALSE;
		m_dRstCircleRate = 0.;
		m_fRstThickMax = 0.;
		m_fRstThickMin = 0.;

		m_bOKOffsetDistance = FALSE;
		m_fOffsetDistance = 0.0f;

		m_bOKPinAngle = FALSE;
		m_fRstPinAngle = 0.0f;
		m_fRstPinHeight = 0.0f;
	}
	void Init()
	{
		m_dRstArea = 0.;
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstAreaRate = 0.;
		m_dRstHeightMean = 0.;
		m_bOKArea = FALSE;
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKHeight = FALSE;
		m_bOKCoilRst = FALSE;

		m_dRstWidth = 0.;
		m_dRstLength = 0.;
		m_dRstDistance = 0.;
		m_bOKWidth = FALSE;
		m_bOKLength = FALSE;
		m_bOKDistance = FALSE;
		m_bOKArea_Reverse = TRUE;	// always Start TRUE
		m_dRstArea_Reverse = 0.;
		m_dTeachArea_Reverse = 0.;

		m_bUseSizeWidth = FALSE;
		m_bUseSizeLength = FALSE;

		m_nArrRectCnt = 0;

		m_nShiftDirIndex = 0;
		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		m_bOKCircleRate = FALSE;
		m_dRstCircleRate = 0.;

		m_bOKOffsetDistance = FALSE;
		m_fOffsetDistance = 0.0f;

		//memset(m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));
		//memset(m_pFPMatchPos, 0, 4 * sizeof(POINTF));
		//memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
		//memset(m_fArrRstA, 0.0, BLOB_RECT_CNTS * sizeof(float));
		//memset(m_fArrRstW, 0.0, BLOB_RECT_CNTS * sizeof(float));
		//memset(m_fArrRstL, 0.0, BLOB_RECT_CNTS * sizeof(float));
	}
}RstAlgoBlob;