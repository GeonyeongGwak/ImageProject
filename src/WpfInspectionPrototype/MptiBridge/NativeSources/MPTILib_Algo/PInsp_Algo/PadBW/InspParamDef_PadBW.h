#pragma once
#include "../InspParamDef_AlgoBase.h"
#define BLOBLABELCNT 10
#define MAX_CNT_PAD_COMPOSED_LIGHT 4

enum ePadBW_MaskType
{
	AlgoMask = 0,
	WindowMask = 1,
	CurrentShape = 2,
};
enum eAlgoPadBinOption
{
	eAlgoPadBinOption_UseHistogram2Quadrant = 0x01,
};
typedef struct tagBin
{
	// 2D
	BOOL m_bInsp2D;
	int m_nMinBinary;
	int m_nMaxBinary;
	int m_nTypeRange2D;    // in 0, out 1, upper 2, lower 3

	  // 3D
	BOOL m_bInsp3D;
	double m_dHeightRateMin;
	double m_dHeightRateMax;
	int m_nTypeRange3D;

	BOOL m_bUseHistogram;

	byte m_byHistoLimitMin;
	byte m_byHistoLimitMax;
	byte m_byHistoFreq;
	byte m_byMaskOpt;
	byte m_byFilterSize;

	BOOL m_bUseFillHole;
	int m_nHistoGridX;
	int m_nHistoGridY;
	struct tagAlgoColorBase m_sAlgoColorBase;
	tagBin()
	{
		m_bInsp2D = TRUE;
		m_nMinBinary = 125;
		m_nMaxBinary = 255;
		m_nTypeRange2D = eTypeRangeIn;

		m_bInsp3D = FALSE;
		m_dHeightRateMin = 80;
		m_dHeightRateMax = 120;
		m_nTypeRange3D = eTypeRangeIn;

		m_bUseHistogram = false;

		m_byHistoLimitMin = 0;
		m_byHistoLimitMax = 0;
		m_byHistoFreq = 0;
		m_byMaskOpt = 0;
		m_byFilterSize = 0;
		m_bUseFillHole = false;
		m_nHistoGridX = 1;
		m_nHistoGridY = 1;
	}
}Bin;

typedef struct stAlgoPadBin
{
	lightData stInspAlgoLightsMix;
	Bin stBin;   //BW condition
	int nOption;
	stAlgoPadBin()
	{
		nOption = 0;
	}

}stPadBin;

typedef struct tagBlobLabelInfo
{
	int nTotalCnt;
	int nid[BLOBLABELCNT];
	float fCx[BLOBLABELCNT];
	float fCy[BLOBLABELCNT];
	int nStx[BLOBLABELCNT];
	int nSty[BLOBLABELCNT];
	int nRoiWidth[BLOBLABELCNT];
	int nRoiLength[BLOBLABELCNT];
	int nArea[BLOBLABELCNT];

	tagBlobLabelInfo()
	{
		nTotalCnt = 0;
		memset(nid, 0, sizeof(int) * BLOBLABELCNT);
		memset(fCx, 0, sizeof(float) * BLOBLABELCNT);
		memset(fCy, 0, sizeof(float) * BLOBLABELCNT);
		memset(nStx, 0, sizeof(int) * BLOBLABELCNT);
		memset(nSty, 0, sizeof(int) * BLOBLABELCNT);
		memset(nRoiWidth, 0, sizeof(int) * BLOBLABELCNT);
		memset(nRoiLength, 0, sizeof(int) * BLOBLABELCNT);
		memset(nArea, 0, sizeof(int) * BLOBLABELCNT);
	}

}BlobLabelInfo;

typedef struct tagAlgoPadBW
{
	Bin sDefaultPad;

	stPadBin sArrInspPad[MAX_CNT_PAD_COMPOSED_LIGHT];  //Composed Light 4
	int m_nTotLightCnt;

	BOOL bTeachAreaUse;
	double dTeachArea;
	double dTeachAreaRateMin;
	double dTeachAreaRateMax;

	BOOL bUseShift;
	double dTeachShiftX;
	double dTeachShiftY;

	//CString strGBMaskName;  //gerber mask ID
	wchar_t strGBMaskName[MAX_STRLEN];

	BOOL bUseBlobWidth;
	double dBlobSizeWidth;
	BOOL bUseBlobLength;
	double dBlobSizeLength;
	BOOL bUseBlobArea;
	double dBlobArea;

	int nMaskShape;
	int nFilterLevel;
	int nOrder;

	BOOL UseOption3DMinMax;
	double Option3DMin;
	double Option3DMax;
	double Option3DThickMin;

	BOOL UseOption3DRange;
	float fOption3DRange;
	int nSelectBlobType;
	int nSelectBlobNum;

	BOOL UseOptionRelativeHeight;
	float fRelativeHeight_Min;
	float fRelativeHeight_Max;

	BOOL UseShadeFix;
	BOOL bUseBlobAnd;
	BOOL bUseContrastGV;
	int nContrastGV;

	BOOL bUseNGGrouping;
	float fNGGroupingMaxSize;
	float fNGGroupingDistance;

	BOOL buseDIrection;
	int nDirection;
	double dDIrectionLength;

	BOOL bUseHoleAlign;
	BOOL bUseWarningWidth;
	double dWarningWidth;
	BOOL bUseWarningLength;
	double dWarningLength;
	BOOL bUseWarningArea;
	double dWarningArea;
	int nWarningCount;
	BOOL bWarningAnd;

	BOOL bUseImgAnd;
	BOOL bUseAI_Segmentation;
	int nMaskCurrentFilterSize;
	int nPrevMasktype;
	int nPrevFilterSize;
	int nForeignImgType;
	int nPrevDefaultAlgoID;

	BOOL m_bUseBlobSizeWidthNCritical;
	double m_dBlobSizeWidthNCritical;
	BOOL m_bUseBlobSizeLengthNCritical;
	double m_dBlobSizeLengthNCritical;
	BOOL m_bUseBlobSizeAreaNCritical;
	double m_dBlobSizeAreaNCritical;
	tagBlobLabelInfo stBlobLabelInfo;

	tagAlgoPadBW()
	{
		m_nTotLightCnt = 1;

		bTeachAreaUse = FALSE;
		dTeachArea = 0.0;
		dTeachAreaRateMin = 80.;
		dTeachAreaRateMax = 120.;

		bUseShift = TRUE;
		dTeachShiftX = 0.0;
		dTeachShiftY = 0.0;

		bUseBlobWidth = TRUE;
		dBlobSizeWidth = 0.0;
		bUseBlobLength = TRUE;
		dBlobSizeLength = 0.0;
		bUseBlobArea = TRUE;
		dBlobArea = 0.0;

		nMaskShape = 0;
		nSelectBlobType = 0;
		nSelectBlobNum = 0;

		UseOptionRelativeHeight = TRUE;
		fRelativeHeight_Min = 0.0;
		fRelativeHeight_Max = 0.0;

		UseShadeFix = FALSE;
		bUseBlobAnd = FALSE;
		bUseContrastGV = FALSE;
		nContrastGV = 10;

		bUseNGGrouping = FALSE;
		fNGGroupingMaxSize = 0.0;
		fNGGroupingDistance = 0.0;

		buseDIrection = FALSE;
		nDirection = 0;
		dDIrectionLength = 0.0;

		bUseHoleAlign = FALSE;
		bUseWarningWidth = FALSE;
		dWarningWidth = 0.0;
		bUseWarningLength = FALSE;
		dWarningLength = 0.0;
		bUseWarningArea = FALSE;
		dWarningArea = 0.0;
		nWarningCount = 0;
		bWarningAnd = FALSE;
		bUseImgAnd = FALSE;
		bUseAI_Segmentation = FALSE;
		nMaskCurrentFilterSize = 0;
		nPrevMasktype = 0;
		nPrevFilterSize = 0;
		nForeignImgType = 0;
		nPrevDefaultAlgoID = 0;
		memset(&stBlobLabelInfo, 0, sizeof(tagBlobLabelInfo));
	}

}AlgoPadBW;

typedef struct tagRstAlgoPadBW
{
	// Pad default Insp OK decision
	BOOL m_bOKShapeArea;
	BOOL m_bOKShapeShiftX;
	BOOL m_bOKShapeShiftY;

	float m_fArrRstShapeArea[MAX_SHAPEDEFECT_CNT];
	float m_fArrRstShapeAreaRate[MAX_SHAPEDEFECT_CNT];
	float m_fArrRstShiftX[MAX_SHAPEDEFECT_CNT];
	float m_fArrRstShiftY[MAX_SHAPEDEFECT_CNT];
	RECT m_rcArrShapeRect_I[MAX_SHAPEDEFECT_CNT];
	BOOL m_bArrShapeOK[MAX_SHAPEDEFECT_CNT];
	int m_nArrShapeRectCnt;

	float m_fArrRstArea[MAX_DEFECT_CNT];
	float m_fArrRstWidth[MAX_DEFECT_CNT];
	float m_fArrRstLength[MAX_DEFECT_CNT];

	// Pad foreign Insp OK decision
	BOOL m_bOKWidth;
	BOOL m_bOKLength;

	RECT m_rcArrRect_I[MAX_DEFECT_CNT];
	int m_nArrRectCnt;

	BOOL m_bOKArea;
	BOOL m_bMaskLoadSuccess;
	BOOL Option3DRangeOK;
	double m_dRstOption3DRange;
	int m_nArrOption3DRangeIdx;

	BOOL Option3DMinMaxOK;
	float m_fRsOption3DMin;
	float m_fRsOption3DMax;
	int m_nArrHeightMinMaxIdx;
	float m_nArrRstHeightMin[MAX_DEFECT_CNT];
	float m_nArrRstHeightMax[MAX_DEFECT_CNT];
	double m_dAlignResultTheta;

	float m_fArrRstRelativeHeightMin[MAX_DEFECT_CNT];
	float m_fArrRstRelativeHeightMax[MAX_DEFECT_CNT];
	float m_fArrRstContrast[MAX_DEFECT_CNT];

	BOOL bIsAIOK;    //AI OK
	BOOL ArrAIOK[MAX_DEFECT_CNT];   //AI DefectROI OK
	float stdAIScore;   //AI OK Score standard
	float ArrAIScore[MAX_DEFECT_CNT];   //AI OK Score
	tagRstAlgoPadBW()
	{
		m_bOKShapeArea = FALSE;
		m_bOKShapeShiftX = FALSE;
		m_bOKShapeShiftY = FALSE;

		memset(m_fArrRstShapeArea, 0.0, MAX_SHAPEDEFECT_CNT * sizeof(float));
		memset(m_fArrRstShapeAreaRate, 0.0, MAX_SHAPEDEFECT_CNT * sizeof(float));
		memset(m_fArrRstShiftX, 0.0, MAX_SHAPEDEFECT_CNT * sizeof(float));
		memset(m_fArrRstShiftY, 0.0, MAX_SHAPEDEFECT_CNT * sizeof(float));

		m_nArrShapeRectCnt = 0;
		memset(m_rcArrShapeRect_I, 0, MAX_SHAPEDEFECT_CNT * sizeof(RECT));
		memset(m_bArrShapeOK, FALSE, MAX_SHAPEDEFECT_CNT * sizeof(BOOL));

		memset(m_fArrRstArea, 0.0, MAX_DEFECT_CNT * sizeof(float));
		memset(m_fArrRstWidth, 0.0, MAX_DEFECT_CNT * sizeof(float));
		memset(m_fArrRstLength, 0.0, MAX_DEFECT_CNT * sizeof(float));

		m_bOKWidth = FALSE;
		m_bOKLength = FALSE;

		m_nArrRectCnt = 0;
		memset(m_rcArrRect_I, 0, MAX_DEFECT_CNT * sizeof(RECT));

		m_bOKArea = FALSE;
		m_bMaskLoadSuccess = TRUE;
		Option3DRangeOK = TRUE;
		m_dRstOption3DRange = 0.0f;
		m_nArrOption3DRangeIdx = 0;

		Option3DMinMaxOK = TRUE;
		m_fRsOption3DMin = 0.0f;
		m_fRsOption3DMax = 0.0f;
		m_nArrHeightMinMaxIdx = 0;
		memset(&m_nArrRstHeightMin, 0, sizeof(float)*MAX_DEFECT_CNT);
		memset(&m_nArrRstHeightMax, 0, sizeof(float)*MAX_DEFECT_CNT);
		m_dAlignResultTheta = 0.0f;
		memset(&m_fArrRstRelativeHeightMin, 0, sizeof(float) * MAX_DEFECT_CNT);
		memset(&m_fArrRstRelativeHeightMax, 0, sizeof(float) * MAX_DEFECT_CNT);
		memset(&m_fArrRstContrast, 0, sizeof(float) * MAX_DEFECT_CNT);

		bIsAIOK = FALSE;
		stdAIScore = 0;
		memset(&ArrAIOK, FALSE, sizeof(BOOL) * MAX_DEFECT_CNT);
		memset(&ArrAIScore, 0, sizeof(float) * MAX_DEFECT_CNT);
	}

}RstAlgoPadBW;
