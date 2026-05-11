#pragma once
#include "../InspParamDef_AlgoBase.h"

#define  m_dfAlgoBarTotal		m_eAlgoBarTotal
#define  m_dfAlgoBarTotalString	65
#define  Barcode1DQualityRECT	200
#define  Barcode1DDefectRECT	50
enum m_eAlgoBar
{
	m_eAlgoBarData,
	m_eAlgoBarData2,
	m_eAlgoBarRetry,
	m_eAlgoBarEssentialWordsPos,
	m_eAlgoBarPartialFir,
	m_eAlgoBarPartialSec,
	m_eAlgoBarCharCntMin,
	m_eAlgoBarCharCntMax,
	m_eAlgoBarUseAngle,
	m_eAlgoBarAngle,
	m_eAlgoBarAngleOK,
	m_eAlgoBarKeyName,
	m_eAlgoBarKeyName2,
	m_eAlgoBarQuality1DOffsetX,
	m_eAlgoBarQuality1DOffsetY,
	m_eAlgoBarQuality1DTolerrance,
	m_eAlgoBarQuality2DGrade,
	m_eAlgoBarQuality2DContrast,
	//m_eAlgoBarQuality2DDefectArea,
	m_eAlgoBarQuality2DDefectCnt,
	m_eAlgoBarAlternate,
	m_eAlgoBarTotal,
};
enum m_eAlgoBar_Data
{
	m_eAlgoBar_Data_CODE39 = 0x01,
	m_eAlgoBar_Data_CODE93 = 0x02,
	m_eAlgoBar_Data_CODE128 = 0x04,
	m_eAlgoBar_Data_CODABAR = 0x08,
	m_eAlgoBar_Data_BC412 = 0x10,
	m_eAlgoBar_Data_EAN8 = 0x20,
	m_eAlgoBar_Data_EAN13 = 0x40,
	m_eAlgoBar_Data_INTERLEAVED25 = 0x80,
	m_eAlgoBar_Data_PHARMACODE = 0x100,
	m_eAlgoBar_Data_PLANET = 0x200,
	m_eAlgoBar_Data_POSTNET = 0x400,
	m_eAlgoBar_Data_RSSCODE = 0x800,
	m_eAlgoBar_Data_UPC_A = 0x1000,
	m_eAlgoBar_Data_UPC_E = 0x2000,
	m_eAlgoBar_Data_DATAMATRIX = 0x4000,
	m_eAlgoBar_Data_MAXICODE = 0x8000,
	m_eAlgoBar_Data_PDF417 = 0x10000,
	m_eAlgoBar_Data_MICROPDF417 = 0x20000,
	m_eAlgoBar_Data_QRCODE = 0x40000,
	m_eAlgoBar_Data_COMPOSITECODE = 0x80000,
	m_eAlgoBar_Data_GS1_128 = 0x100000,
	m_eAlgoBar_Data_ForeColor = 0x200000,
	m_eAlgoBar_Data_XFlip = 0x400000,
	m_eAlgoBar_Data_YFlip = 0x800000,
	m_eAlgoBar_Data_B_NotUseCheckSum = 0x1000000,
	m_eAlgoBar_Data_B_UseCheckSum = 0x2000000,
	m_eAlgoBar_Data_B_BOTH = 0x4000000,
	m_eAlgoBar_Data_Alaram = 0x8000000,
	m_eAlgoBar_Data_UseCrossCheck1 = 0x10000000,
	m_eAlgoBar_Data_UseCrossCheck2 = 0x20000000,
	m_eAlgoBar_Data_MICRO_QR = 0x40000000
};
enum m_eAlgoBar_Data2
{
	m_eAlgoBar_Data2_UseEssentialWords = 0x01,
	m_eAlgoBar_Data2_UseStandardWords = 0x02,
	m_eAlgoBar_Data2_UsePartialDisplay = 0x04,
	m_eAlgoBar_Data2_DTK_Barcode = 0x08,
	m_eAlgoBar_Data2_MIL_Barcode = 0x10,
	m_eAlgoBar_Data2_HALCON_Barcode = 0x20,
	m_eAlgoBar_Data2_All_Barcode = 0x40,
	m_eAlgoBar_Data2_Automatic = 0x80,
	m_eAlgoBar_Data2_UseCharacterCount = 0x100,
	m_eAlgoBar_Data2_UseBarcodeQuality = 0x200,
	m_eAlgoBar_Data2_UseTopQuietZone = 0x400,
	m_eAlgoBar_Data2_UseBottomQuietZone = 0x800,
	m_eAlgoBar_Data2_UseDefectCondition = 0x1000,
	m_eAlgoBar_Data2_Use2DQualityModule = 0x2000,
	m_eAlgoBar_Data2_UseMachinOptSaveImage = 0x4000,
	m_eAlgoBar_Data2_UseBCDPartToModule = 0x8000,
	m_eAlgoBar_Data2_UseModuleRecognize = 0x10000,
	m_eAlgoBar_Data2_UseModuleTagWord = 0x20000,
	m_eAlgoBar_Data2_UsePNValidation = 0x40000,
	m_eAlgoBar_Data2_UseSplitFailImgSave = 0x80000,
	m_eAlgoBar_Data2_UseReadFailOK = 0x100000,
};
enum m_eAlgoBar_HALCON_QCData_1D {
	m_eAlgoBar_QC1D_Overall_Quality,
	m_eAlgoBar_QC1D_Decode,
	m_eAlgoBar_QC1D_Symbol_Contrast,
	m_eAlgoBar_QC1D_Minimal_Reflectance,
	m_eAlgoBar_QC1D_Minimal_Edge_Contrast,
	m_eAlgoBar_QC1D_Modulation,
	m_eAlgoBar_QC1D_Defects,
	m_eAlgoBar_QC1D_Decodeability,
	m_eAlgoBar_QC1D_Additional_Requirements,
	m_eAlgoBar_QC1D_Total,
};
enum m_eAlgoBar_HALCON_QCData_2D {
	m_eAlgoBar_QC2D_Overall_Quality,
	m_eAlgoBar_QC2D_Contrast,
	m_eAlgoBar_QC2D_Modulation,
	m_eAlgoBar_QC2D_Fixed_Pattern_Damage,
	m_eAlgoBar_QC2D_Decode,
	m_eAlgoBar_QC2D_Axial_Nonuniformity,
	m_eAlgoBar_QC2D_Grid_Nonuniformity,
	m_eAlgoBar_QC2D_Unused_Error_Correction,
	m_eAlgoBar_QC2D_Reflectancec_Margin,
	m_eAlgoBar_QC2D_Print_Growth,
	m_eAlgoBar_QC2D_Contrast_Uniformity,
	m_eAlgoBar_QC2D_Format_Information,
	m_eAlgoBar_QC2D_Version_Information,
	m_eAlgoBar_QC2D_Aperture,
	m_eAlgoBar_QC2D_Total,	
};

typedef struct tagAlgoBarcode
{
	int m_nArrData[m_dfAlgoBarTotal];
	wchar_t m_sEss[MAX_STRLEN];
	wchar_t m_sStd[MAX_STRLEN];
	wchar_t m_sMix[MAX_STRLEN];
	wchar_t m_sSourceFilter[MAX_STRLEN];
	wchar_t m_sTargetFilter[MAX_STRLEN];
	wchar_t m_sSourceFilter2[MAX_STRLEN];
	wchar_t m_sTargetFilter2[MAX_STRLEN];
	wchar_t m_sCrossTarget[MAX_STRLEN];
	wchar_t m_sCrossTarget2[MAX_STRLEN];
	BOOL m_bUseBarcodeAlign;
	BOOL m_bUseTeachBarcodeAlign;
	float m_fTeachAngle;
	POINTF m_ptTeachCen;
	wchar_t m_sWndName[MAX_STRLEN];
	float m_fDefWidth;
	float m_fDefLength;
	int m_nDefCount;
	float m_f2DDefArea;
	wchar_t m_sRecogWord[MAX_STRLEN];
	wchar_t m_sRecogLength[MAX_STRLEN];

	tagAlgoBarcode()
	{
		memset(m_nArrData, 0, sizeof(int) * m_dfAlgoBarTotal);
		memset(m_sEss, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sStd, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sMix, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sSourceFilter, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sTargetFilter, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossTarget, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sSourceFilter2, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sTargetFilter2, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossTarget2, 0, sizeof(wchar_t) * MAX_STRLEN);
		m_bUseBarcodeAlign = FALSE;
		m_bUseTeachBarcodeAlign = FALSE;
		m_fTeachAngle = 0.0;
		m_ptTeachCen = POINTF();
		memset(m_sWndName, 0, sizeof(wchar_t) * MAX_STRLEN);
		m_fDefWidth = 0;
		m_fDefLength = 0;
		m_nDefCount = 0;
		m_f2DDefArea = 0.0;
		memset(m_sRecogWord, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sRecogLength, 0, sizeof(wchar_t) * MAX_STRLEN);
	}
}AlgoBarcode;
typedef struct tagRstAlgoBarcode
{
	// OK 판정 //
	BOOL m_bOK;
	wchar_t m_sBarcode[MAX_STRLEN];
	wchar_t m_sPartialBarcode[MAX_STRLEN];
	float fAngle;
	BOOL bIsOrientationOK;
	POINTF ptCen;
	wchar_t m_sCrossSource[MAX_STRLEN];
	wchar_t m_sCrossTarget[MAX_STRLEN];
	wchar_t m_sCrossSource2[MAX_STRLEN];
	wchar_t m_sCrossTarget2[MAX_STRLEN];
	BOOL IsCrossCheckOK;
	BOOL IsCrossCheckOK2;
	BOOL m_bUseAlign;
	POINTF m_ptWndPos;	// Part 내에서 Window Position
	RECT m_rcROI;		// 바코드를 포함하는 Rect

	float m_fArrRst_QC1D[m_eAlgoBar_QC1D_Total];
	float m_fArrRst_QC2D[m_eAlgoBar_QC2D_Total];
	BOOL IsDecodeQualityOK;
	BOOL IsDefectQualityOK;
	POINTF m_ptRst_Coner[4];	// 돌아간 이미지에서 실제 바코드의 영역(우상 ->> 좌상 ->> 좌하 ->> 우하)
	int m_nBarType;		// 현재 바코드 타입 (1 or 2)
	float m_fLengthTopQZ;
	float m_fLengthBtmQZ;
	BOOL IsTopQuietZoneOK;
	BOOL IsBtmQuietZoneOK;
	POINTF m_fNGArrTopQZ[4];
	POINTF m_fNGArrBtmQZ[4];
	BOOL IsTopQZLengthOK;
	BOOL IsBtmQZLengthOK;
	int nInspRectCnt;								// Quality Inspection Rect Cnt 
	RECT InspBarRect[Barcode1DQualityRECT];		// Quality Inspection 위치 Rect정보 
	int nDetectRectCnt;
	RECT DetectBarRect[Barcode1DQualityRECT];		// 1D Barcode Black Bar 위치 Rect정보 
	int nDefectRectCnt;								// Quality Inspection Defect Cnt 
	RECT InspDefectRect[Barcode1DDefectRECT];	// Quality Defect 위치 Rect정보 
	int nDefectRectIndex[Barcode1DDefectRECT]; // Defect Rect Index 
	int nDefectGrade;
	wchar_t m_sPNValidation[MAX_STRLEN];
	BOOL IsPNValidationOK;
	tagRstAlgoBarcode()
	{
		m_bOK = FALSE;
		memset(m_sBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sPartialBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
		fAngle = 0;
		bIsOrientationOK = TRUE;
		ptCen = POINTF();
		memset(m_sCrossSource, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossTarget, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossSource2, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossTarget2, 0, sizeof(wchar_t) * MAX_STRLEN);
		IsCrossCheckOK = TRUE;
		IsCrossCheckOK2 = TRUE;
		m_bUseAlign = FALSE;
		m_ptWndPos = POINTF();
		m_rcROI = RECT();

		//memset(m_fArrRst_QC1D, 0.0, sizeof(float) * m_eAlgoBar_QC1D_Total);
		//memset(m_fArrRst_QC2D, 0.0, sizeof(float) * m_eAlgoBar_QC2D_Total);
		for (int i = 0; i < m_eAlgoBar_QC1D_Total; i++)
			m_fArrRst_QC1D[i] = -1;
		for (int i = 0; i < m_eAlgoBar_QC2D_Total; i++)
			m_fArrRst_QC2D[i] = -1;
		IsDecodeQualityOK = TRUE;
		IsDefectQualityOK = TRUE;
		memset(m_ptRst_Coner, 0.0, sizeof(POINTF) * 4);
		m_nBarType = 0;
		m_fLengthTopQZ = 0.0;
		m_fLengthBtmQZ = 0.0;
		IsTopQuietZoneOK = TRUE;
		IsBtmQuietZoneOK = TRUE;
		memset(m_fNGArrTopQZ, 0.0, sizeof(POINTF) * 4);
		memset(m_fNGArrBtmQZ, 0.0, sizeof(POINTF) * 4);
		IsTopQZLengthOK = TRUE;
		IsBtmQZLengthOK = TRUE;
		nInspRectCnt = 0;
		memset(InspBarRect, 0, sizeof(RECT) * Barcode1DQualityRECT);
		nDetectRectCnt = 0;
		memset(DetectBarRect, 0, sizeof(RECT) * Barcode1DQualityRECT);
		nDefectRectCnt = 0;
		memset(InspDefectRect, 0, sizeof(RECT) * Barcode1DDefectRECT);
			for (int i = 0; i < Barcode1DDefectRECT; i++)
				nDefectRectIndex[i] = -1;
		nDefectGrade = -1;
		memset(m_sPNValidation, 0, sizeof(wchar_t) * MAX_STRLEN);
		IsPNValidationOK = TRUE;
	}
	void Init()
	{
		m_bOK = FALSE;
		memset(m_sBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sPartialBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
		fAngle = 0;
		bIsOrientationOK = TRUE;
		ptCen = POINTF();
		memset(m_sCrossSource, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossTarget, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossSource2, 0, sizeof(wchar_t) * MAX_STRLEN);
		memset(m_sCrossTarget2, 0, sizeof(wchar_t) * MAX_STRLEN);
		IsCrossCheckOK = TRUE;
		IsCrossCheckOK2 = TRUE;
		m_bUseAlign = FALSE;
		m_ptWndPos = POINTF();
		m_rcROI = RECT();

		//memset(m_fArrRst_QC1D, 0.0, sizeof(float) * m_eAlgoBar_QC1D_Total);
		//memset(m_fArrRst_QC2D, 0.0, sizeof(float) * m_eAlgoBar_QC2D_Total);
		for (int i = 0; i < m_eAlgoBar_QC1D_Total; i++)
			m_fArrRst_QC1D[i] = -1;
		for (int i = 0; i < m_eAlgoBar_QC2D_Total; i++)
			m_fArrRst_QC2D[i] = -1;
		IsDecodeQualityOK = TRUE;
		IsDefectQualityOK = TRUE;
		memset(m_ptRst_Coner, 0.0, sizeof(POINTF) * 4);
		m_nBarType = 0;
		m_fLengthTopQZ = 0.0;
		m_fLengthBtmQZ = 0.0;
		IsTopQuietZoneOK = TRUE;
		IsBtmQuietZoneOK = TRUE;
		memset(m_fNGArrTopQZ, 0.0, sizeof(POINTF) * 4);
		memset(m_fNGArrBtmQZ, 0.0, sizeof(POINTF) * 4);
		IsTopQZLengthOK = TRUE;
		IsBtmQZLengthOK = TRUE;
		nInspRectCnt = 0;
		memset(InspBarRect, 0, sizeof(RECT) * Barcode1DQualityRECT);
		nDetectRectCnt = 0;
		memset(DetectBarRect, 0, sizeof(RECT) * Barcode1DQualityRECT);
		nDefectRectCnt = 0;
		memset(InspDefectRect, 0, sizeof(RECT) * Barcode1DDefectRECT);
		for (int i = 0; i < Barcode1DDefectRECT; i++)
			nDefectRectIndex[i] = -1;
		nDefectGrade = -1;
		memset(m_sPNValidation, 0, sizeof(wchar_t) * MAX_STRLEN);
		IsPNValidationOK = TRUE;
	}
}RstAlgoBarcode;
