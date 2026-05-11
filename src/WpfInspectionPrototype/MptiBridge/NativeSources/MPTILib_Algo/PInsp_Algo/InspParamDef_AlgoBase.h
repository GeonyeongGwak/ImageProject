#pragma once

#define LIGHT_CNT				10
#define LIGHT_DATA_CNT			6

//max string length
#define MAX_STRLEN   256

//LJH 2016.05.28 영상 합치는 갯수 최대 숫자
#define MAX_MIX_COUNT	2
// Color
#define COLORALGO_POLYGON_CNTS	3
#define POLYGON_POINT_CNTS		5
#define LIGHT_CNT				10

// AlignEdge
#define ALIGNEDGE_AREA_CNTS		3

// Edge
#define  EdgeLineTotalCnt	4
#define  EdgeInspCnt		3

#define InspAC_T_Total	m_eInspAC_T_Total
#define InspAC_T2_Total	m_eInspAC_T2_Total
#define InspAC_T3_Total	m_eInspAC_T3_Total
#define InspAC_R_Total	m_eInspAC_R_Total
#define InspAC_R_Rect_Total	m_eInspAC_R_Rect_Total

#define MAX_MASKING_NUM 4
#define MAX_INSP_AREA_COUNT 20
//PatternDiff
#define PST_SIDE_NUM 4
#define MAX_BIG_INFO 10

//Foreign Algorithm Max DefectCnt

#define	TiltRectCnt		4
// Bridge
#define	BRIEDGE_CNT		500
//const int BRIEDGE_CNT = 500;
// Blob
const int BLOB_RECT_CNTS = 200;
#define VOLUME_INSP_CNTS		2
const int VOLUME_DIVIDE_CNTS = 20;
const int VOLUME_DIVIDE_AREA_CNTS = VOLUME_DIVIDE_CNTS + 1;
#define MAX_POLYCNT 40
#define MAX_LAYERCNT 10
#define MAX_MODELCNT 10
#define MAX_ALIGNCNT 8
#define MAX_DEFECT_CNT 100
#define MAX_SHAPEDEFECT_CNT 30

enum kindofWholeNGType
{
	TypeMountMissing = 0,
	TypeS_Ball,
	TypeBillboarding,	//모로섬
	TypeTombstone,
	TypeMountWorng,
	TypeOcrWrong,
	TypeMountPolarity,
	TypeUpsidedown,
	TypeLength,
	TypeMountShift,
	TypeAlignWrong,
	TypeMountAngle,
	TypeMountTilt,
	TypeLeadBridge,
	TypeMountLift,
	TypeLeadSolder,
	TypeNoSolder,
	TypeSolderFilet,	//소납
	TypeSolderExcess,	//과납
	TypeLeadLift,
	TypeLeadTip,		//lead
	TypeLeadShift,
	TypeForeign,		//이물
	TypeTab,
	TypeTabSpace,
	TypeCOPPER,			//동판
	TypeDamage,
	TypeColdJoint,
	TypeSolderCone,
	TypePinShift,
	TypeVoid,
	TypeThickness,
	TypeUserDefine,
	TypeLowCoating,
	TypeHighCoating,
	TypeBubble,		//버블		//NYJ 2018/07/18
	TypeWarpage,
	TypeBGA_NG,
	TypeCOVERAGE,
	TypeBODYWIDTH,
	TypeBODYLENGTH,
	TypeBODYAREA,
	TypeCOLOR,
	TypeForeign_A,		//이물
	TypeForeign_B,		//이물
	TypeForeign_C,		//이물
	kindofWholeNGTypeTotalNum,
};

enum m_eMMD
{
	eMMD_Default = 0,
	eMMD_Min,
	eMMD_Max,
	eMMD_Total,
};
enum m_eMMA
{
	eMMA_Average = 0,
	eMMA_Min,
	eMMA_Max,
	eMMA_Total,
};
enum m_eTypeBGA
{
	eBump = 0,       // Round Ball
	eCoining,    // Flat Ball
	eBlob,       // Midget Bump, etc..
};
enum m_eBinData
{
	m_eBinData_Use = 0x01,
	m_eBinData_Range_In = 0x02,
	m_eBinData_Range_Out = 0x04,
	m_eBinData_Range_Up = 0x08,
	m_eBinData_Range_Lo = 0x10,
};
enum m_eBin
{
	m_eBin_Data3D,
	m_eBin_Data2D,
	m_eBin_Min2D,
	m_eBin_Max2D,
	m_eBin_L_Cnt,
	m_eBin_L_Type,
	m_eBin_num,
};
enum m_eLightData
{
	eLightData_Position = 0,
	eLightData_Calculation,
	eLightData_Red,
	eLightData_Blue,
	eLightData_Green,
	eLightData_White,
	eLightData_Total,
};
enum  ETypeBlob {
	eSelectBigger,		// 가장 큰 Area Blob 1개
	eSelectMix,			// All Blob
	eSelectCenter,		// Center에 가까운 Blob 1개
	eSelectPosition,	// 지정된 좌표에 가까운 Blob 1개
	eSimilarArea,		// 면적이 가까운
	eSelectInner,
	eSelectHiddenArea,	// 가려진 영역이 있는 All Blob (Max 수치를 넘어가는 Blob은 제외)
};
enum  ETypeInspRange {
	eTypeRangeIn,
	eTypeRangeOut,
	eTypeRangeUpper,
	eTypeRangeLower
};
enum m_enBlobBase_Data
{
	m_enBlobBase_Data_eUse = 0x01,
	m_enBlobBase_Data_e2D = 0x02,
	m_enBlobBase_Data_e2DRangeUP = 0x04,
	m_enBlobBase_Data_e2DRangeLo = 0x08,
	m_enBlobBase_Data_e2DRangeIN = 0x10,
	m_enBlobBase_Data_e2DInvert = 0x20,
	m_enBlobBase_Data_e3D = 0x40,
	m_enBlobBase_Data_e3DRangeUP = 0x80,
	m_enBlobBase_Data_e3DRangeLo = 0x100,
	m_enBlobBase_Data_e3DRangeIN = 0x200,
	m_enBlobBase_Data_eInvert = 0x400,
	m_enBlobBase_Data_eFillHole = 0x800,
	m_enBlobBase_Data_eFilter = 0x1000,
	m_enBlobBase_Data_eMaxBlob = 0x2000,
};
enum m_enBlobBase
{
	m_enBlobBase_Dt,
	m_enBlobBase_Min2D,
	m_enBlobBase_Max2D,
	m_enBlobBase_Filter,
	m_enBlobBase_MinArea,
	m_enBlobBase_Total,
};
enum m_efBlobBase
{
	m_efBlobBase_Min3D,
	m_efBlobBase_Max3D,
	m_efBlobBase_Total,
};
// AngleColor
enum m_eInspAC_T
{
	m_eInspAC_T_TAMin = 0,
	m_eInspAC_T_TAMax,
	m_eInspAC_T_TA,
	m_eInspAC_T_CJ,
	m_eInspAC_T_CJ_X,
	m_eInspAC_T_CJ_Y,
	m_eInspAC_T_Range1,
	m_eInspAC_T_Range2,
	m_eInspAC_T_Range3,
	m_eInspAC_T_Gap,
	m_eInspAC_T_TAMin2,
	m_eInspAC_T_TAMin3,
	m_eInspAC_T_TAMax2,
	m_eInspAC_T_TAMax3,
	m_eInspAC_T_TA2,
	m_eInspAC_T_TA3,
	m_eInspAC_T_HMMin1,
	m_eInspAC_T_HMMax1,
	m_eInspAC_T_HMMin2,
	m_eInspAC_T_HMMax2,
	m_eInspAC_T_HMMin3,
	m_eInspAC_T_HMMax3,
	m_eInspAC_T_Total,
};
enum m_eInspAC_T2
{
	m_eInspAC_T2_InspArea = 0,
	m_eInspAC_T2_CJ_Gap,
	m_eInspAC_T2_Cnt,
	m_eInspAC_T2_Pix,
	m_eInspAC_T2_Total,
};
enum m_eInspAC_T3
{
	m_eInspAC_T3_Data = 0,
	m_eInspAC_T3_Data2,
	m_eInspAC_T3_Data3,
	m_eInspAC_T3_Total,
};
enum m_eInspAC_R_Data
{
	m_eInspAC_R_Data_TA = 0x01,
	m_eInspAC_R_Data_CJ = 0x02,
	m_eInspAC_R_Data_TA_R = 0x04,
	m_eInspAC_R_Data_TA_G = 0x08,
	m_eInspAC_R_Data_TA_B = 0x10,
	m_eInspAC_R_Data_TA_BW = 0x20,
	m_eInspAC_R_Data_TA1 = 0x40,
	m_eInspAC_R_Data_TA2 = 0x80,
	m_eInspAC_R_Data_TA3 = 0x100,
	m_eInspAC_R_Data_TA2_R = 0x200,
	m_eInspAC_R_Data_TA2_G = 0x400,
	m_eInspAC_R_Data_TA2_B = 0x800,
	m_eInspAC_R_Data_TA2_BW = 0x1000,
	m_eInspAC_R_Data_TA3_R = 0x2000,
	m_eInspAC_R_Data_TA3_G = 0x4000,
	m_eInspAC_R_Data_TA3_B = 0x8000,
	m_eInspAC_R_Data_TA3_BW = 0x10000,
	m_eInspAC_R_Data_CJ1 = 0x20000,
	m_eInspAC_R_Data_CJ2 = 0x40000,
	m_eInspAC_R_Data_CJ3 = 0x80000,
	m_eInspAC_R_Data_HM1 = 0x100000,
	m_eInspAC_R_Data_HM2 = 0x200000,
	m_eInspAC_R_Data_HM3 = 0x400000,
};
enum m_eInspAC_R
{
	m_eInspAC_R_R = 0,
	m_eInspAC_R_G,
	m_eInspAC_R_B,
	m_eInspAC_R_Gr,
	m_eInspAC_R_CJ,
	m_eInspAC_R_TA,
	m_eInspAC_R_CJR,
	m_eInspAC_R_CJG,
	m_eInspAC_R_CJB,
	m_eInspAC_R_CJW,
	m_eInspAC_R_CJBL,
	m_eInspAC_R_TA2,
	m_eInspAC_R_TA3,
	m_eInspAC_R_CJ2,
	m_eInspAC_R_CJ3,
	m_eInspAC_R_R2,
	m_eInspAC_R_G2,
	m_eInspAC_R_B2,
	m_eInspAC_R_Gr2,
	m_eInspAC_R_R3,
	m_eInspAC_R_G3,
	m_eInspAC_R_B3,
	m_eInspAC_R_Gr3,
	m_eInspAC_R_CJR2,
	m_eInspAC_R_CJG2,
	m_eInspAC_R_CJB2,
	m_eInspAC_R_CJW2,
	m_eInspAC_R_CJR3,
	m_eInspAC_R_CJG3,
	m_eInspAC_R_CJB3,
	m_eInspAC_R_CJW3,
	m_eInspAC_R_HM1,
	m_eInspAC_R_HM2,
	m_eInspAC_R_HM3,
	m_eInspAC_R_Total,
};
enum m_eInspAC_R_Rect
{
	m_eInspAC_R_Rect_CJ = 0,
	m_eInspAC_R_Rect_Range1,
	m_eInspAC_R_Rect_Range2,
	m_eInspAC_R_Rect_Range3,
	m_eInspAC_R_Rect_Total,
};

enum eForeign3DExceptMode
{
	e3DInspOriginalExcept = 0,
	e3DInspSolderExcept,
	e3DInspPolygonOriginExcept,
	e3DInspPolygonSoldExcept,
};

enum eLineMeasureDir
{
	Center_Line = 0,
	Left_Line,
	Right_Line,
	TL_Line,
	TR_Line,
	BR_Line,
	BL_Line,
};
enum m_eDefaultAC_Default
{
	m_eDefaultAC_Default_GR = 0x01,
	m_eDefaultAC_Default_BR = 0x02,
	m_eDefaultAC_Default_BG = 0x04,
};
enum m_eDefaultAC_Data
{
	eDefaultAC_Data_Use = 0x01,
	eDefaultAC_Data_WA = 0x02,
	eDefaultAC_Data_TA = 0x04,
	eDefaultAC_Data_CJ = 0x08,

	eDefaultAC_Data_TAMax = 0x10,
	eDefaultAC_Data_Auto = 0x20,
	eDefaultAC_Data_R = 0x40,
	eDefaultAC_Data_G = 0x80,

	eDefaultAC_Data_B = 0x100,
	eDefaultAC_Data_Gray = 0x200,
	eDefaultAC_Data_Auto2 = 0x400,
	eDefaultAC_Data_R2 = 0x800,

	eDefaultAC_Data_G2 = 0x1000,
	eDefaultAC_Data_B2 = 0x2000,
	eDefaultAC_Data_Gray2 = 0x4000,
	eDefaultAC_Data_Auto3 = 0x8000,

	eDefaultAC_Data_R3 = 0x10000,
	eDefaultAC_Data_G3 = 0x20000,
	eDefaultAC_Data_B3 = 0x40000,
	eDefaultAC_Data_Gray3 = 0x80000,

	eDefaultAC_Data_CJ_R = 0x100000,
	eDefaultAC_Data_CJ_G = 0x200000,
	eDefaultAC_Data_CJ_B = 0x400000,
	eDefaultAC_Data_CJ_White = 0x800000,

	eDefaultAC_Data_CJ_Black = 0x1000000,
	eDefaultAC_Data_TA_R = 0x2000000,
	eDefaultAC_Data_TA_G = 0x4000000,
	eDefaultAC_Data_TA_B = 0x8000000,

	eDefaultAC_Data_TA_Gr = 0x10000000,
	eDefaultAC_Data_Essential = 0x20000000,
	eDefaultAC_Data_CJ_Body = 0x40000000,
};
enum m_eDefaultAC_Data2
{
	m_eDefaultAC_Data2_R_Sub = 0x01,
	m_eDefaultAC_Data2_G_Sub = 0x02,
	m_eDefaultAC_Data2_B_Sub = 0x04,
	m_eDefaultAC_Data2_Gray_Sub = 0x08,

	m_eDefaultAC_Data2_R2_Sub = 0x10,
	m_eDefaultAC_Data2_G2_Sub = 0x20,
	m_eDefaultAC_Data2_B2_Sub = 0x40,
	m_eDefaultAC_Data2_Gray2_Sub = 0x80,

	m_eDefaultAC_Data2_R3_Sub = 0x100,
	m_eDefaultAC_Data2_G3_Sub = 0x200,
	m_eDefaultAC_Data2_B3_Sub = 0x400,
	m_eDefaultAC_Data2_Gray3_Sub = 0x800,

	m_eDefaultAC_Data2_TA2_R = 0x1000,
	m_eDefaultAC_Data2_TA2_G = 0x2000,
	m_eDefaultAC_Data2_TA2_B = 0x4000,
	m_eDefaultAC_Data2_TA2_Gr = 0x8000,

	m_eDefaultAC_Data2_TA3_R = 0x10000,
	m_eDefaultAC_Data2_TA3_G = 0x20000,
	m_eDefaultAC_Data2_TA3_B = 0x40000,
	m_eDefaultAC_Data2_TA3_Gr = 0x80000,

	m_eDefaultAC_Data2_CJ2_R = 0x100000,
	m_eDefaultAC_Data2_CJ2_G = 0x200000,
	m_eDefaultAC_Data2_CJ2_B = 0x400000,
	m_eDefaultAC_Data2_CJ2_Gr = 0x800000,

	m_eDefaultAC_Data2_CJ3_R = 0x1000000,
	m_eDefaultAC_Data2_CJ3_G = 0x2000000,
	m_eDefaultAC_Data2_CJ3_B = 0x4000000,
	m_eDefaultAC_Data2_CJ3_Gr = 0x8000000,
};
enum m_eDefaultAC_Data3
{
	m_eDefaultAC_Data3_HM = 0x01,
	m_eDefaultAC_Data3_HMMax = 0x02,
};
enum m_enHistoBase
{
	eHistoUse,
	eHistoRangeType,
	eHistoMax2D,
	eHistoMin2D,
	eHistoMinExcept,
	eHistoMaxExcept,
	eHistoTotal,
};
enum m_eVOL_TACT
{
	m_eVOL_TACT_Blob = 0,
	m_eVOL_TACT_SET_POLY,
	m_eVOL_TACT_FILL_POLY,
	m_eVOL_TACT_POLY_IMG_CHECK,
	m_eVOL_TACT_POLY_CEN,
	m_eVOL_TACT_POLY_ROI,
	m_eVOL_TACT_INSP_SHIFT_AREA,
	m_eVOL_TACT_InspRemoveHeight,
	m_eVOL_TACT_PAT_Divide,
	m_eVOL_TACT_HeightArea,
	m_eVOL_TACT_CircleCheck,
	m_eVOL_TACT_ColdJoint,
	m_eVOL_TACT_Inclination,
	m_eVOL_TACT_PADPatternCheck,
	m_eVOL_TACT_PADPatternCheck_CHK_INSP_OK,
	m_eVOL_TACT_PADPatternCheck_CHK_INSP_FAIL,
	m_eVOL_TACT_PADPatternCheck_CHK_INSP_AERA,
	m_eVOL_TACT_PADPatternCheck_CHK_INSP_TH_IN,
	m_eVOL_TACT_PADPatternCheck_CHK_INSP_TH_OUT,
	m_eVOL_TACT_PADPatternCheck_CHK_INSP_PAT_AREA,
	m_eVOL_TACT_PADPatternCheck_CHK_INSP_OK_FAIL,
	m_eVOL_TACT_PAD_OUT_1,
	m_eVOL_TACT_PAD_OUT_2,
	m_eVOL_TACT_PAD_OUT_3,
	m_eVOL_TACT_PAD_OUT_4,
	m_eVOL_TACT_PAD_OUT_5,
	m_eVOL_TACT_PAD_OUT_6,
	m_eVOL_TACT_Result,
	m_eVOL_TACT_PAT_SET,
	m_eVOL_TACT_PAT_FIND_OFFSET,
	m_eVOL_TACT_PAT_MIX,
	m_eVOL_TACT_PAT_VIA,
	m_eVOL_TACT_PAT_OverlapArea,
	m_eVOL_TACT_PAT_Thickness,
	m_eVOL_TACT_PAT_TH_SET,
	m_eVOL_TACT_PAT_TH_1,
	m_eVOL_TACT_PAT_TH_2,
	m_eVOL_TACT_PAT_TH_AB,
	m_eVOL_TACT_VOLUME_INSP_OK,
	m_eVOL_TACT_VOLUME_INSP_FAIL,
	m_eVOL_TACT_VOLUME_INSP_FAIL2,

	m_eVOL_TACT_Total,
};
enum m_eVOL_PAT_CHK
{
	m_eVOL_PAT_CHK_AERA = 0x01,
	m_eVOL_PAT_CHK_TH_IN = 0x02,
	m_eVOL_PAT_CHK_TH_OUT = 0x04,
};
enum m_eVOL_PAT_RST
{
	m_eVOL_PAT_RST_OK = 0,
	m_eVOL_PAT_RST_AERA = 0x01,
	m_eVOL_PAT_RST_TH_IN = 0x02,
	m_eVOL_PAT_RST_TH_OUT = 0x04,
	m_eVOL_PAT_RST_PAT_AREA = 0x08,

	m_eVOL_PAT_RST_OK_FAIL = 0x10,
	m_eVOL_PAT_RST_PAT_AREA_N = 0x20,
	m_eVOL_PAT_RST_OK_FAIL_N = 0x40,
	m_eVOL_PAT_RST_PAD_N = 0x80,
};

#define  COLOR_DATA_Non		0x00
#define  COLOR_DATA_USE		0x01
#define  COLOR_DATA_AC		0x02
#define  COLOR_DATA_MID		0x04
#define  COLOR_DATA_AI		0x08
#define  COLOR_DATA_WND		0x10
#define  COLOR_DATA_BIN_AC	0x20

typedef struct tagLightData
{
	byte m_byLightCnt;
	byte m_byArrLightData[LIGHT_DATA_CNT][LIGHT_CNT];
	void Clone(tagLightData &inspAlgo)
	{
		inspAlgo.m_byLightCnt = m_byLightCnt;
		for (int a = 0; a < LIGHT_DATA_CNT; a++)
			for (int b = 0; b < LIGHT_CNT; b++)
				inspAlgo.m_byArrLightData[a][b] = m_byArrLightData[a][b];
	}
}lightData;

typedef struct tagAlgoColorBase
{
	BOOL m_bUseColor;
	BOOL m_bUseAngleColor;
	int m_nPolygonCnt;
	POINT m_ptArrPolygon[COLORALGO_POLYGON_CNTS][POLYGON_POINT_CNTS];
	byte m_byType;
	BOOL m_bUseRGB[COLORALGO_POLYGON_CNTS];
	byte m_byRange[COLORALGO_POLYGON_CNTS];
	byte m_byMin[COLORALGO_POLYGON_CNTS];
	byte m_byMax[COLORALGO_POLYGON_CNTS];
	BOOL m_bInvert;
	byte m_byColorLightType;

	BOOL m_bUseIntensity;
	int m_nRangeType;
	int m_nRangeMin;
	int m_nRangeMax;

	tagAlgoColorBase()
	{
		m_bUseColor = FALSE;
		m_bUseAngleColor = FALSE;
		m_nPolygonCnt = 1;
		m_byType = 0;
		for (int n = 0; n < COLORALGO_POLYGON_CNTS; n++)
		{
			for (int k = 0; k < POLYGON_POINT_CNTS; k++)
			{
				m_ptArrPolygon[n][k].x = -1;
				m_ptArrPolygon[n][k].y = -1;
			}
			m_bUseRGB[n] = FALSE;
			m_byRange[n] = 0;
			m_byMin[n] = 0;
			m_byMax[n] = 0;
		}
		m_bInvert = FALSE;
		m_byColorLightType = 0;

		m_bUseIntensity = FALSE;
		m_nRangeType = eTypeRangeIn;
		m_nRangeMin = 0;
		m_nRangeMax = 80;
	}
	int GetColorData()
	{
		int nData = COLOR_DATA_Non;

		if (m_bUseColor)
		{
			nData = COLOR_DATA_USE;

			if (m_bUseAngleColor)
				nData |= COLOR_DATA_BIN_AC;
			if (m_byColorLightType == 1)
				nData |= COLOR_DATA_MID;
		}

		return nData;
	}
}AlgoColorBase;

typedef struct tagAlgoBlobBase
{
	int m_nArrValue[m_enBlobBase_Total];
	float m_fArrValue[m_efBlobBase_Total];
	struct tagAlgoColorBase m_sAlgoColorBase;
	tagAlgoBlobBase()
	{
		memset(m_nArrValue, 0, m_enBlobBase_Total * sizeof(int));
		memset(m_fArrValue, 0, m_efBlobBase_Total * sizeof(float));
	}
}AlgoBlobBase;

typedef struct tagAlgoBaseBW
{
	BOOL m_bInvertCheck;		// 반전 여부
	double m_dPercentOK;		// white pixel 비율 OK 조건

	BOOL m_bChipTracking;		// Bodyblob을 이용한 솔더 위치 찾기

	BOOL m_b2dCheck;			// 2D 적용 여부
	int m_nRange;				// 2D Range
	int m_nMinValue;			// 2D Threshold Min.
	int m_nMaxValue;			// 2D Threshold Max.

	BOOL m_b3dCheck;			// 3D 적용 여부
	int m_n3dRange;				// 3D Range
	double m_d3dHeightMin;	// 3D Threshold Min. (um단위)
	double m_d3dHeightMax;	// 3D Threshold Max. (um단위)
	double m_d3dAvgHeight;	// 3D Teahing Height Value
}AlgoBaseBW;

typedef struct tagAngleColorBase
{
	int m_nArrInspAC[InspAC_T3_Total];
	byte m_byArrInspAC[InspAC_T2_Total];
	float m_fArrInspAC[InspAC_T_Total];
	tagAngleColorBase()
	{
		memset(m_nArrInspAC, 0, InspAC_T3_Total * sizeof(int));
		memset(m_byArrInspAC, 0, InspAC_T2_Total * sizeof(byte));
		memset(m_fArrInspAC, 0, InspAC_T_Total * sizeof(float));
	}
	bool UseAC()
	{
		if ((m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) == eDefaultAC_Data_Use)
			return true;

		else false;
	}

}AngleColorBase;

typedef struct tagRstInspAC	// BW 검사 결과
{
	int m_nOKInspAC;
	int m_nInspAC;
	float m_fRstInspAC[InspAC_R_Total];
	RECT m_rcArrRect[InspAC_R_Rect_Total];
	tagRstInspAC()
	{
		m_nOKInspAC = 1;
		m_nInspAC = 0;
		memset(m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
		memset(m_rcArrRect, 0, InspAC_R_Rect_Total * sizeof(RECT));
	}

	void SetRstData(tagAngleColorBase sAngleColorBase)
	{
		int nInspAC = sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data];
		bool bInspAC = ((nInspAC & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
		bool bInspAC_Ess = (bInspAC && ((nInspAC & eDefaultAC_Data_Essential) == eDefaultAC_Data_Essential));
		bool bInspAC_WA = (bInspAC && ((nInspAC & eDefaultAC_Data_WA) == eDefaultAC_Data_WA));

		m_nOKInspAC = 0;
		if (bInspAC)
		{
			if ((nInspAC & eDefaultAC_Data_TA) == eDefaultAC_Data_TA)
			{
				if (m_fRstInspAC[m_eInspAC_R_TA] < sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TAMin])
					m_nOKInspAC |= m_eInspAC_R_Data_TA;
				if (((nInspAC & eDefaultAC_Data_TAMax) == eDefaultAC_Data_TAMax) &&
					m_fRstInspAC[m_eInspAC_R_TA] > sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TAMax])
					m_nOKInspAC |= m_eInspAC_R_Data_TA;

				if ((m_nInspAC & eDefaultAC_Data_R) != eDefaultAC_Data_R &&
					(nInspAC & eDefaultAC_Data_TA_R) == eDefaultAC_Data_TA_R &&
					m_fRstInspAC[m_eInspAC_R_R] > sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
					m_nOKInspAC |= m_eInspAC_R_Data_TA_R;
				if ((m_nInspAC & eDefaultAC_Data_G) != eDefaultAC_Data_G &&
					(nInspAC & eDefaultAC_Data_TA_G) == eDefaultAC_Data_TA_G &&
					m_fRstInspAC[m_eInspAC_R_G] > sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
					m_nOKInspAC |= m_eInspAC_R_Data_TA_G;
				if ((m_nInspAC & eDefaultAC_Data_B) != eDefaultAC_Data_B &&
					(nInspAC & eDefaultAC_Data_TA_B) == eDefaultAC_Data_TA_B &&
					m_fRstInspAC[m_eInspAC_R_B] > sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
					m_nOKInspAC |= m_eInspAC_R_Data_TA_B;
				if ((m_nInspAC & eDefaultAC_Data_Gray) != eDefaultAC_Data_Gray &&
					(nInspAC & eDefaultAC_Data_TA_Gr) == eDefaultAC_Data_TA_Gr &&
					m_fRstInspAC[m_eInspAC_R_Gr] > sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
					m_nOKInspAC |= m_eInspAC_R_Data_TA_BW;
			}
			if ((nInspAC & eDefaultAC_Data_CJ) == eDefaultAC_Data_CJ)
			{
				if (m_fRstInspAC[m_eInspAC_R_CJ] >= sAngleColorBase.m_fArrInspAC[m_eInspAC_T_CJ])
					m_nOKInspAC |= m_eInspAC_R_Data_CJ;
			}
		}
		else
		{
			memset(m_fRstInspAC, 0, m_eInspAC_R_Total * sizeof(float));
		}
	}

}RstInspAC;
typedef struct tagPoly
{
	int m_nPtrcnt;      //최대 20개
	int m_nPolyDir;		 //in chipping inspect, Direction of Polygon Area
	POINTF m_dArrPtr[MAX_POLYCNT];  //최대 20개
	void stPoly()
	{
		m_nPtrcnt = 0;
		memset(m_dArrPtr, 0, sizeof(POINTF) * MAX_POLYCNT);
	}
}stPoly;
typedef struct tagLayerPolygon
{
	int m_nAreaCnt;
	stPoly m_nPoly[MAX_LAYERCNT];

	void LayerPolygon()
	{
		m_nAreaCnt = 0;
	}

}LayerPolygon;
typedef struct tagHistoParamBase
{
	int m_nArrValue[(int)m_enHistoBase::eHistoTotal];

	tagHistoParamBase()
	{
		memset(m_nArrValue, 0, (int)m_enHistoBase::eHistoTotal * sizeof(int));
	}
}HistoParamBase;
typedef struct tagModelPath
{
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

	tagModelPath()
	{
	}

	CString GetModelPath(int nModelIdx)
	{
		CString str = _T("");

		if (nModelIdx >= MAX_MODELCNT)
			return str;

		switch (nModelIdx)
		{
		case 0: str.Format(m_sModelPath1); break;
		case 1: str.Format(m_sModelPath2); break;
		case 2: str.Format(m_sModelPath3); break;
		case 3: str.Format(m_sModelPath4); break;
		case 4: str.Format(m_sModelPath5); break;
		case 5: str.Format(m_sModelPath6); break;
		case 6: str.Format(m_sModelPath7); break;
		case 7: str.Format(m_sModelPath8); break;
		case 8: str.Format(m_sModelPath9); break;
		case 9: str.Format(m_sModelPath10); break;
		case 10: str.Format(m_sModelPath11); break;
		case 11: str.Format(m_sModelPath12); break;
		case 12: str.Format(m_sModelPath13); break;
		case 13: str.Format(m_sModelPath14); break;
		case 14: str.Format(m_sModelPath15); break;
		case 15: str.Format(m_sModelPath16); break;

		}
		return str;
	}
}ModelPath;

typedef struct tagRECT_F
{
	float left;
	float right;
	float top;
	float bottom;
	tagRECT_F()
	{
	}
	void Init()
	{
		left = 0;
		right = 0;
		top = 0;
		bottom = 0;
	}
	bool ContainXY(float nX, float nY)
	{
		if (left <= nX && right >= nX)
		{
			if (top <= nY && bottom >= nY)
				return true;
		}
		return false;
	}
	void SetData(tagRECT_F rc)
	{
		left = rc.left;
		right = rc.right;
		top = rc.top;
		bottom = rc.bottom;
	}
	void SetROI_GAP(float nXPix, float nYPix, float nGAPX, float nGAPY)
	{
		left = nXPix - nGAPX;
		right = nXPix + nGAPX;
		top = nYPix - nGAPY;
		bottom = nYPix + nGAPY;
	}
	void SetROI(float nL, float nR, float nT, float nB)
	{
		left = nL;
		right = nR;
		top = nT;
		bottom = nB;
	}
	float GetW(bool bABS = true)
	{
		if (bABS)
			return fabs(right - left + 1.0f);
		else
			return right - left + 1.0f;
	}
	float GetH(bool bABS = true)
	{
		if (bABS)
			return fabs(bottom - top + 1.0f);
		else
			return bottom - top + 1.0f;
	}
	float CALC_DIST(double dResX, double dResY)
	{
		float fX = GetW();
		float fY = GetH();
		fX *= dResX;
		fY *= dResY;
		float fTH = sqrt(pow(fX, 2.0f) + pow(fY, 2.0f));
		return fTH;
	}
	bool COMPARE_VALUE(bool bRET, bool bTEMP, tagRECT_F rcRET, double dResX, double dResY, float fPol_RET, float fPol)
	{
		if (bTEMP)
			return false;

		float fUM = 1000.0f;
		float fRET = rcRET.CALC_DIST(dResX, dResY);
		float fRET_GAP = fRET * fUM;
		float fRET_DIFF = fPol_RET - fRET_GAP;

		float fTEMP = CALC_DIST(dResX, dResY);
		float fTEMP_GAP = fTEMP * fUM;
		float fTEMP_DIFF = fPol - fTEMP_GAP;

		if (bRET == false && bTEMP == false)
		{
			if (fabs(fRET_DIFF) > fabs(fTEMP_DIFF))
				return true;
		}
		else if (bRET && bTEMP == false)
			return true;

		return false;
	}
}RECT_F;

enum m_eINSP_PAD_PAT_OUT_FIND
{
	m_eINSP_PAD_PAT_OUT_FIND_NOT = -4,
	m_eINSP_PAD_PAT_OUT_FIND_POLY = -3,
	m_eINSP_PAD_PAT_OUT_FIND_BIN = -2,
	m_eINSP_PAD_PAT_OUT_FIND_FAIL = -1,
	m_eINSP_PAD_PAT_OUT_FIND_FIND_OK = 1,
	m_eINSP_PAD_PAT_OUT_FIND_BIN_OK = 2,
	m_eINSP_PAD_PAT_OUT_FIND_POLY_OK = 3,
};

typedef struct tagRstPAT_CHK
{
	float m_fROIW;
	float m_fROIH;
	float m_fROIA;
	RECT m_rcRect;
	tagRstPAT_CHK()
	{
		Init();
	}
	void Init()
	{
		m_fROIW = 0;
		m_fROIH = 0;
		m_fROIA = 0;
		m_rcRect.left = 0;
		m_rcRect.right = 0;
		m_rcRect.top = 0;
		m_rcRect.bottom = 0;
	}
	void AddRST(tagRstPAT_CHK sRst)
	{
		m_fROIW = sRst.m_fROIW;
		m_fROIH = sRst.m_fROIH;
		m_fROIA = sRst.m_fROIA;
		m_rcRect.left = sRst.m_rcRect.left;
		m_rcRect.right = sRst.m_rcRect.right;
		m_rcRect.top = sRst.m_rcRect.top;
		m_rcRect.bottom = sRst.m_rcRect.bottom;
	}
}RstPAT_CHK;
typedef struct tagINSP_PAD_PAT_RST
{
	int m_nRST;
	double m_dCNT;
	double m_dRST_MAX_AREA;
	double m_dRST_MAX_MOR;

	RECT_F rcFIND_RST;

	BOOL m_bPAD_RET;
	int m_nRstPAD_CHK;
	int m_nOKPadPatternCheck;
	int m_nPadPatternCheck;
	float m_fRstNotOverlapArea;
	float m_fRst_TH_MIN;
	float m_fRst_TH_MAX;
	RstPAT_CHK m_sRstPAD_CHK[VOLUME_DIVIDE_AREA_CNTS];
	RECT m_rcRect_TH[VOLUME_INSP_CNTS];

	int m_nPOLY_W;
	int m_nPOLY_H;
	BOOL m_bVolume_RET;
	BOOL m_bVolume_PARALLEL;
	tagINSP_PAD_PAT_RST()
	{
		Init();
	}
	void Init()
	{
		m_nRST = 0;
		m_dCNT = 0;
		m_dRST_MAX_AREA = 0;
		m_dRST_MAX_MOR = 0;
		rcFIND_RST.Init();

		m_bPAD_RET = FALSE;
		m_nRstPAD_CHK = -1;
		m_nOKPadPatternCheck = -1;
		m_nPadPatternCheck = -1;
		m_fRstNotOverlapArea = -1;
		m_fRst_TH_MIN = -1;
		m_fRst_TH_MAX = -1;
		for (int a = 0; a < VOLUME_DIVIDE_AREA_CNTS; a++)
			m_sRstPAD_CHK[a].Init();
		for (int a = 0; a < VOLUME_INSP_CNTS; a++)
		{
			m_rcRect_TH[a].left = 0;
			m_rcRect_TH[a].right = 0;
			m_rcRect_TH[a].top = 0;
			m_rcRect_TH[a].bottom = 0;
		}
		m_nPOLY_W = 0;
		m_nPOLY_H = 0;
		m_bVolume_RET = FALSE;
		m_bVolume_PARALLEL = FALSE;
	}
	bool CheckDATA()
	{
		if (m_nRST == m_eINSP_PAD_PAT_OUT_FIND_FAIL)
			return false;

		if (m_dCNT > 0 && m_dRST_MAX_AREA > 0)
		{
			if (-5 <= rcFIND_RST.top && 5 >= rcFIND_RST.top)
			{
				if (-5 <= rcFIND_RST.bottom && 5 >= rcFIND_RST.bottom)
					return true;
			}
		}

		return false;
	}
	void AddPAT_CHK(RstPAT_CHK sRst)
	{
		if (m_nRstPAD_CHK < 0) m_nRstPAD_CHK = 0;
		int nID = m_nRstPAD_CHK;
		if (nID >= VOLUME_DIVIDE_AREA_CNTS)
		{
			for (int a = 0; a < m_nRstPAD_CHK; a++)
			{
				if (m_sRstPAD_CHK[a].m_fROIA >= sRst.m_fROIA)
					continue;
				m_sRstPAD_CHK[a].AddRST(sRst);
				break;
			}
		}
		else
		{
			m_sRstPAD_CHK[nID].AddRST(sRst);
			m_nRstPAD_CHK++;
		}
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nOKPadPatternCheck & nType) == nType);
		return bRet;
	}
	int CheckINSP()
	{
		int nRet = (int)m_eVOL_PAT_RST_OK;
		if (m_nRstPAD_CHK > 0) // 면적 불량 개수
			nRet += (int)m_eVOL_PAT_RST_PAT_AREA;
		else if (m_nRstPAD_CHK < 0)
			nRet += (int)m_eVOL_PAT_RST_PAT_AREA_N;

		if (m_nOKPadPatternCheck > 0) // Pattern 불량 체크
		{
			int nPre = nRet;
			if (UseData(m_eVOL_PAT_CHK_AERA))
				nRet += (int)m_eVOL_PAT_RST_AERA;
			if (UseData(m_eVOL_PAT_CHK_TH_IN))
				nRet += (int)m_eVOL_PAT_RST_TH_IN;
			if (UseData(m_eVOL_PAT_CHK_TH_OUT))
				nRet += (int)m_eVOL_PAT_RST_TH_OUT;

			if (nPre == nRet)
				nRet += (int)m_eVOL_PAT_RST_OK_FAIL;
		}
		else if (m_nOKPadPatternCheck < 0)
			nRet += (int)m_eVOL_PAT_RST_OK_FAIL_N;

		if (m_nPadPatternCheck < 0)
			nRet += (int)m_eVOL_PAT_RST_PAD_N;

		return nRet;
	}
	bool CheckINSP_PARALLEL()
	{
		if (m_bVolume_PARALLEL == TRUE)
			return true;
		else
			return false;
	}
	bool InspT(int nType, int nCheckINSP)
	{
		if (nCheckINSP == m_eVOL_PAT_RST_OK)
			return false;

		int nRST = -1;
		if (nType == m_eVOL_TACT_PADPatternCheck_CHK_INSP_AERA)
		{
			nRST = m_eVOL_PAT_RST_PAT_AREA;
		}
		else if (nType == m_eVOL_TACT_PADPatternCheck_CHK_INSP_TH_IN)
		{
			nRST = m_eVOL_PAT_RST_TH_IN;
		}
		else if (nType == m_eVOL_TACT_PADPatternCheck_CHK_INSP_TH_OUT)
		{
			nRST = m_eVOL_PAT_RST_TH_OUT;
		}
		else if (nType == m_eVOL_TACT_PADPatternCheck_CHK_INSP_PAT_AREA)
		{
			nRST = m_eVOL_PAT_RST_AERA;
		}
		else if (nType == m_eVOL_TACT_PADPatternCheck_CHK_INSP_OK_FAIL)
		{
			nRST = m_eVOL_PAT_RST_OK_FAIL;
		}
		else
			return false;

		bool bRet = ((nCheckINSP & nRST) == nRST);
		return bRet;
	}

}INSP_PAD_PAT_RST;
