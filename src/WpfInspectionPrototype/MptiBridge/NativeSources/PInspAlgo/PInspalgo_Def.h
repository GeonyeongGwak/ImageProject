#pragma once

#include <vector>

#if defined(_MSC_VER)

#if defined(PINSPALGO_EXPORTS) && defined(_USRDLL)
#define PINSPALGO_API __declspec(dllexport)
#else
#define PINSPALGO_API __declspec(dllimport)
#endif

#else
#define PINSPALGO_API
#endif

#define  DOWN_LEFT    0
#define  DOWN_RIGHT   1
#define  UP_LEFT      2
#define  UP_RIGHT     3

#define _NoSolderStandardValue 5
#define _WIRE_FOOT_CNT 10
#define _WIRE_MAX_CNT 10
#define _WIRE_DRAW_CNT 2
#define _WIRE_DOT_CNT 500
#define _WIRE_DOT3_CNT 20
#define _INT_DOUBLE_DP 1000
////max string length
#define _MAX_STRLEN   256
#define _FOREIGN_POLYGON_CNTS	10
#define _COLORALGO_POLYGON_CNTS	3
#define _MAX_MASKING_NUM 20
#define _MAX_INSP_AREA_COUNT 20
#define _POLYGON_POINT_CNTS		5
//AlignEdge Area
#define _ALIGNEDGE_AREA_CNTS		3
//
//pattern
#define _CNT_PATTERN_PATH		16
#define _CNT_PATTERN_DIVISION_C	11
#define _CNT_PATTERN_DIVISION_R	11
#define _CNT_PATTERN_SCORE		(_CNT_PATTERN_DIVISION_C + 1)
#define _CNT_PATTERN_SIMILAR		5

#define _LIGHT_CNT	10
#define _LIGHT_DATA_CNT	6

#define _HISTO_SIZE      256
#define _SIZE_CLRX	(1000)	// (CIE)	// 256 (RB)
#define _SIZE_CLRY	(1000)

#define _CIE_OFFSETX		235
#define _CIE_OFFSETY		205

#define _CIE_XR	(0.490f)
#define _CIE_XG	(0.310f)
#define _CIE_XB	(0.200f)
#define _CIE_YR	(0.177f)
#define _CIE_YG	(0.813f)
#define _CIE_YB	(0.011f)
#define _CIE_ZR	(0.000f)
#define _CIE_ZG	(0.010f)
#define _CIE_ZB	(0.990f)

#define _InspAC_T_Total			_m_eInspAC_T_Total
#define _InspAC_T2_Total		_m_eInspAC_T2_Total
#define _InspAC_T3_Total		_m_eInspAC_T3_Total
#define _InspAC_R_Total			_m_eInspAC_R_Total
#define _InspAC_R_Rect_Total	_m_eInspAC_R_Rect_Total

#define	_limit_value(val, _min, _max) (val < _min) ? _min : ((val > _max) ? _max : val)
#define	_limit_Max(val, _max) (val > _max) ? _max : val
//Tilt Algorithm   NYJ 2020/11/10
#define	_TiltRectCnt		4
#define	_RstTiltAngleCnt	2
#define	_nTiltAngleCnt	8

const int _BLOB_RECT_CNTS = 200;
const int _BUMP_RECT_CNTS = 10;
#define _VOID_RECT_CNTS  10
#define _MAX_CNT_PAD_COMPOSED_LIGHT 4		//NYJ 2021/04/21
#define _dfUser_LightCnt	10				// User light 합성 개수
#define LIGHT_CNT				10
#define LIGHT_DATA_CNT			6
#define _PADBW_MASK_IDX 5	//PadBW RotateMaskBuffer
#define _PADBW_SHAPEDST_IDX 6 //PadBW ShapeInsp Result buffer

const int _g_nBridgeCnt = 500;

const int _GWIRE_MAX_CNT = 256;
const int _GWIRE_MAX_PAD_CNT = 64;
const int _MAX_PAD_CNT = 6;

#define _MinBlobAreaSize 10



#pragma region X-ray
#define MAX_THUMB_COUNT 10  
#pragma endregion

#define _CNT_NGBLOB_PATTERN_PATH		16

//Pattern Diff
#define _MAX_POLYCNT 40
#define _MAX_POLYGROUPCNT 10
#define _MAX_LAYERCNT 10
#define _MAX_MODELCNT 16
#define _MAX_BLOBCNT 200
#define _MATCHING_MARGIN 15
#define _MAX_ALIGNCNT 8

//max string length
#define MAX_STRLEN   256
#define  SOLDERVOL_LEVEL_COUNT	10

//PadBW
#define _BLOBLABELCNT 10
#define _PADBWTILTER 2

//Barcode
#define _CNT_1DQUALITY_PARAMS 9		//m_eAlgoBar_QC1D_Total
#define _CNT_2DQUALITY_PARAMS 14	//m_eAlgoBar_QC2D_Total

//Foreign Algorithm Max DefectCnt
#define _MAX_DEFECT_CNT 100
#define _MAX_SHAPEDEFECT_CNT 30


enum _InspLightType
{
	_Top_Light,
	_Middle_Light,
	_Bottom_Light,
	_User_Light,
	_Side1_Light,
	_Side2_Light,
	_Side3_Light,
	_Side4_Light,
	_ThreeD,
	_num,
};

enum _m_eDefaultAC
{
	_eDefaultAC_Data = 0,
	_eDefaultAC_BW,
	_eDefaultAC_Total,
};

enum _m_eDefaultAC_Default
{
	_m_eDefaultAC_Default_GR = 0x01,
	_m_eDefaultAC_Default_BR = 0x02,
	_m_eDefaultAC_Default_BG = 0x04,
};

enum _m_eDefaultAC_Data
{
	_eDefaultAC_Data_Use = 0x01,
	_eDefaultAC_Data_WA = 0x02,
	_eDefaultAC_Data_TA = 0x04,
	_eDefaultAC_Data_CJ = 0x08,
	
	_eDefaultAC_Data_TAMax = 0x10,
	_eDefaultAC_Data_Auto = 0x20,
	_eDefaultAC_Data_R = 0x40,
	_eDefaultAC_Data_G = 0x80,
	
	_eDefaultAC_Data_B = 0x100,
	_eDefaultAC_Data_Gray = 0x200,
	_eDefaultAC_Data_Auto2 = 0x400,
	_eDefaultAC_Data_R2 = 0x800,
	
	_eDefaultAC_Data_G2 = 0x1000,
	_eDefaultAC_Data_B2 = 0x2000,
	_eDefaultAC_Data_Gray2 = 0x4000,
	_eDefaultAC_Data_Auto3 = 0x8000,
	
	_eDefaultAC_Data_R3 = 0x10000,
	_eDefaultAC_Data_G3 = 0x20000,
	_eDefaultAC_Data_B3 = 0x40000,
	_eDefaultAC_Data_Gray3 = 0x80000,
	
	_eDefaultAC_Data_CJ_R = 0x100000,
	_eDefaultAC_Data_CJ_G = 0x200000,
	_eDefaultAC_Data_CJ_B = 0x400000,
	_eDefaultAC_Data_CJ_White = 0x800000,
	
	_eDefaultAC_Data_CJ_Black = 0x1000000,
	_eDefaultAC_Data_TA_R = 0x2000000,
	_eDefaultAC_Data_TA_G = 0x4000000,
	_eDefaultAC_Data_TA_B = 0x8000000,
	
	_eDefaultAC_Data_TA_Gr = 0x10000000,
	_eDefaultAC_Data_Essential = 0x20000000,
	_eDefaultAC_Data_CJ_Body = 0x40000000,
};

enum _m_eDefaultAC_Data2
{
	_m_eDefaultAC_Data2_R_Sub = 0x01,
	_m_eDefaultAC_Data2_G_Sub = 0x02,
	_m_eDefaultAC_Data2_B_Sub = 0x04,
	_m_eDefaultAC_Data2_Gray_Sub = 0x08,
	
	_m_eDefaultAC_Data2_R2_Sub = 0x10,
	_m_eDefaultAC_Data2_G2_Sub = 0x20,
	_m_eDefaultAC_Data2_B2_Sub = 0x40,
	_m_eDefaultAC_Data2_Gray2_Sub = 0x80,
	
	_m_eDefaultAC_Data2_R3_Sub = 0x100,
	_m_eDefaultAC_Data2_G3_Sub = 0x200,
	_m_eDefaultAC_Data2_B3_Sub = 0x400,
	_m_eDefaultAC_Data2_Gray3_Sub = 0x800,
	
	_m_eDefaultAC_Data2_TA2_R = 0x1000,
	_m_eDefaultAC_Data2_TA2_G = 0x2000,
	_m_eDefaultAC_Data2_TA2_B = 0x4000,
	_m_eDefaultAC_Data2_TA2_Gr = 0x8000,
	
	_m_eDefaultAC_Data2_TA3_R = 0x10000,
	_m_eDefaultAC_Data2_TA3_G = 0x20000,
	_m_eDefaultAC_Data2_TA3_B = 0x40000,
	_m_eDefaultAC_Data2_TA3_Gr = 0x80000,
	
	_m_eDefaultAC_Data2_CJ2_R = 0x100000,
	_m_eDefaultAC_Data2_CJ2_G = 0x200000,
	_m_eDefaultAC_Data2_CJ2_B = 0x400000,
	_m_eDefaultAC_Data2_CJ2_Gr = 0x800000,
	
	_m_eDefaultAC_Data2_CJ3_R = 0x1000000,
	_m_eDefaultAC_Data2_CJ3_G = 0x2000000,
	_m_eDefaultAC_Data2_CJ3_B = 0x4000000,
	_m_eDefaultAC_Data2_CJ3_Gr = 0x8000000,
};

enum _m_eInspAC_T
{
	_m_eInspAC_T_TAMin = 0,
	_m_eInspAC_T_TAMax,
	_m_eInspAC_T_TA,
	_m_eInspAC_T_CJ,
	_m_eInspAC_T_CJ_X,
	_m_eInspAC_T_CJ_Y,
	_m_eInspAC_T_Range1,
	_m_eInspAC_T_Range2,
	_m_eInspAC_T_Range3,
	_m_eInspAC_T_Gap,
	_m_eInspAC_T_TAMin2,
	_m_eInspAC_T_TAMin3,
	_m_eInspAC_T_TAMax2,
	_m_eInspAC_T_TAMax3,
	_m_eInspAC_T_TA2,
	_m_eInspAC_T_TA3,
	_m_eInspAC_T_Total,
};

enum _m_eInspAC_T2
{
	_m_eInspAC_T2_InspArea = 0,
	_m_eInspAC_T2_CJ_Gap,
	_m_eInspAC_T2_Cnt,
	_m_eInspAC_T2_Pix,
	_m_eInspAC_T2_Total,
};

enum _m_eInspAC_T3
{
	_m_eInspAC_T3_Data = 0,
	_m_eInspAC_T3_Data2,
	_m_eInspAC_T3_Total,
};

enum _m_eInspAC_R_Data
{
	_m_eInspAC_R_Data_TA = 0x01,
	_m_eInspAC_R_Data_CJ = 0x02,
	_m_eInspAC_R_Data_TA_R = 0x04,
	_m_eInspAC_R_Data_TA_G = 0x08,
	_m_eInspAC_R_Data_TA_B = 0x10,
	_m_eInspAC_R_Data_TA_BW = 0x20,
	_m_eInspAC_R_Data_TA1 = 0x40,
	_m_eInspAC_R_Data_TA2 = 0x80,
	_m_eInspAC_R_Data_TA3 = 0x100,
	_m_eInspAC_R_Data_TA2_R = 0x200,
	_m_eInspAC_R_Data_TA2_G = 0x400,
	_m_eInspAC_R_Data_TA2_B = 0x800,
	_m_eInspAC_R_Data_TA2_BW = 0x1000,
	_m_eInspAC_R_Data_TA3_R = 0x2000,
	_m_eInspAC_R_Data_TA3_G = 0x4000,
	_m_eInspAC_R_Data_TA3_B = 0x8000,
	_m_eInspAC_R_Data_TA3_BW = 0x10000,
	_m_eInspAC_R_Data_CJ1 = 0x20000,
	_m_eInspAC_R_Data_CJ2 = 0x40000,
	_m_eInspAC_R_Data_CJ3 = 0x80000,
};

enum _m_eInspAC_R
{
	_m_eInspAC_R_R = 0,
	_m_eInspAC_R_G,
	_m_eInspAC_R_B,
	_m_eInspAC_R_Gr,
	_m_eInspAC_R_CJ,
	_m_eInspAC_R_TA,
	_m_eInspAC_R_CJR,
	_m_eInspAC_R_CJG,
	_m_eInspAC_R_CJB,
	_m_eInspAC_R_CJW,
	_m_eInspAC_R_CJBL,
	_m_eInspAC_R_TA2,
	_m_eInspAC_R_TA3,
	_m_eInspAC_R_CJ2,
	_m_eInspAC_R_CJ3,
	_m_eInspAC_R_R2,
	_m_eInspAC_R_G2,
	_m_eInspAC_R_B2,
	_m_eInspAC_R_Gr2,
	_m_eInspAC_R_R3,
	_m_eInspAC_R_G3,
	_m_eInspAC_R_B3,
	_m_eInspAC_R_Gr3,
	_m_eInspAC_R_CJR2,
	_m_eInspAC_R_CJG2,
	_m_eInspAC_R_CJB2,
	_m_eInspAC_R_CJW2,
	_m_eInspAC_R_CJR3,
	_m_eInspAC_R_CJG3,
	_m_eInspAC_R_CJB3,
	_m_eInspAC_R_CJW3,
	_m_eInspAC_R_Total,
};

enum _m_eInspAC_R_Rect
{
	_m_eInspAC_R_Rect_CJ = 0,
	_m_eInspAC_R_Rect_Range1,
	_m_eInspAC_R_Rect_Range2,
	_m_eInspAC_R_Rect_Range3,
	_m_eInspAC_R_Rect_Total,
};

enum  _ETypeBlob {
	_eSelectBigger,		// 가장 큰 Area Blob 1개
	_eSelectMix,			// All Blob
	_eSelectCenter,		// Center에 가까운 Blob 1개
	_eSelectPosition,	// 지정된 좌표에 가까운 Blob 1개
	_eSimilarArea,		// 면적이 가까운
	_eSelectInner, // Center 보다 진짜 Center
	_eSelectHiddenArea, // 가려진 영역이 있는 All Blob (Max 수치를 넘어가는 Blob은 제외)
};

enum  _ETypeInspRange {
	_eTypeRangeIn,
	_eTypeRangeOut,
	_eTypeRangeUpper,
	_eTypeRangeLower
};

enum _m_eMMD
{
	_eMMD_Default = 0,
	_eMMD_Min,
	_eMMD_Max,
	_eMMD_Total,
};

enum _m_eMMA
{
	_eMMA_Average = 0,
	_eMMA_Min,
	_eMMA_Max,
	_eMMA_Total,
};

enum _m_eWIRE
{
	_eWIRE_Warp = 0,
	_eWIRE_Thickness,
	_eWIRE_Distance,
	_eWIRE_Height,
	_eWIRE_Highest,
	_eWIRE_SteepSlope,
	_eWIRE_Interval,
	_eWIRE_Height2,
	_eWIRE_Total,
};

enum _m_eWIRE2
{
	_eWIRE2_Dot = 0,
	_eWIRE2_UseOption,
	_eWIRE2_UseOption2,
	_eWIRE2_DivCnt,
	_eWIRE2_SteepSlopeMargin,
	_eWIRE2_Total,
};

enum _m_eWIRE_Data
{
	_eWIRE_Data_Warp = 0x01,
	_eWIRE_Data_Thickness = 0x02,
	_eWIRE_Data_Distance = 0x04,
	_eWIRE_Data_Height = 0x08,
	_eWIRE_Data_Highest = 0x10,
	_eWIRE_Data_SteepSlope = 0x20,
	_eWIRE_Data_Interval = 0x40,
	_eWIRE_Data_Height2 = 0x80,
	//eWIRE_Data_Interval = 0x100,
	//eWIRE_Data_Height2 = 0x200,
};

enum _m_eWIRE_Data2
{
	_m_eWIRE_Data2_RefArea = 0x01,
	_m_eWIRE_Data2_Max = 0x02,
	_m_eWIRE_Data2_FootPos = 0x04,
	_m_eWIRE_Data2_Total = 0x0F,
};

enum _m_eWIRE_DOT
{
	_eDot_Highest = 0,
	_eDot_Warp,
	_eDot_TH,
	_eDot_TH1,
	_eDot_TH2,
	_eDot_LowH,
	_eDot_Total,
};

enum _m_eBinData
{
	_m_eBinData_Use = 0x01,
	_m_eBinData_Range_In = 0x02,
	_m_eBinData_Range_Out = 0x04,
	_m_eBinData_Range_Up = 0x08,
	_m_eBinData_Range_Lo = 0x10,
};

enum _m_eBin
{
	_m_eBin_Data3D,
	_m_eBin_Data2D,
	_m_eBin_Min2D,
	_m_eBin_Max2D,
	_m_eBin_L_Cnt,
	_m_eBin_L_Type,
	_m_eBin_num,
};

enum _m_eFoot
{
	_m_eFoot_DeviationX = 0,
	_m_eFoot_DeviationY,
	_m_eFoot_Deviation,
	_m_eFoot_Height,
	_m_eFoot_Damage,
	_m_eFoot_Width_1,
	_m_eFoot_Width_1_H,
	_m_eFoot_Length_1,
	_m_eFoot_Tail_L,
	_m_eFoot_Tail_A,
	_m_eFoot_Wedge_W,
	_m_eFoot_Wedge_L,
	_m_eFoot_Foreign,
	_m_eFoot_HeelCrack,
	_m_eFoot_WingArea,
	_m_eFoot_Bonding1,
	_m_eFoot_Bonding2,
	_m_eFoot_Total,
};
enum _m_eFootBin
{
	_m_eFootBin_Pad = 0,
	_m_eFootBin_Foot,
	_m_eFootBin_Wing,
	_m_eFootBin_Wedge,
	_m_eFootBin_FindDBC,
	_m_eFootBin_Total,
};
enum _m_eFOOT_Data
{
	_m_eFOOT_Data_ShiftX = 0x01,
	_m_eFOOT_Data_ShiftY = 0x02,
	_m_eFOOT_Data_Shift = 0x04,
	_m_eFOOT_Data_Height = 0x08,
	_m_eFOOT_Data_Damage = 0x10,
	_m_eFOOT_Data_Width_1 = 0x20,
	_m_eFOOT_Data_Width_1_H = 0x40,
	_m_eFOOT_Data_Length_1 = 0x80,
	_m_eFOOT_Data_Tail_L = 0x100,
	_m_eFOOT_Data_Tail_A = 0x200,
	_m_eFOOT_Data_Wedge_W = 0x400,
	_m_eFOOT_Data_Wedge_L = 0x800,
	_m_eFOOT_Data_Foreign = 0x1000,
	_m_eFOOT_Data_HeelCrack = 0x2000,
	_m_eFOOT_Data_WingArea = 0x4000,
	// = 0x8000,
	// = 0x10000,
	// = 0x20000,
	// = 0x40000,
	// = 0x80000,
	_m_eFOOT_Data_Total = 0xFFFF//F,
};
enum _m_eFOOT_Data2
{
	_m_eFOOT_Data2_RefArea = 0x01,
	_m_eFOOT_Data2_Find = 0x02,
	_m_eFOOT_Data2_Find_Bin3D = 0x04,
	_m_eFOOT_Data2_Find_RefA = 0x08,
	_m_eFOOT_Data2_Dir_Up = 0x10,
	_m_eFOOT_Data2_DBC = 0x20,
	_m_eFOOT_Data2_Align = 0x40,
	_m_eFOOT_Data2_Total = 0xFF,
};
enum _m_eFoot_FindOption
{
	_Auto = 0,
	_Wing,
	_Bonding,
	_Wing3D,
	_Slope3D,
	_WingGap,
	_MeanSlope,
	_StdDevSlope,
	_total
};
enum _m_eFoot_Type
{
	_Wedge_bonding = 0,
	_Ribbon_bonding,
	_total_bonding
};

enum _colorInspType
{
	_e_COLORXY = 0,
	_e_HISTOGRAM,
};

enum _color_stepID
{
	_eCStepID_ANALYSIS_COLOR = 0,
	_eCStepID_ANALYSIS_COLOR_ROTATE,
};

enum _color_HistoRangeMode
{
	_eHISTO_IN_RANGE = 0,
	_eHISTO_OUT_RANGE,
	_eHISTO_UPPER,
	_eHISTO_LOWER,
};

enum _m_eLightData
{
	_eLightData_Position = 0,
	_eLightData_Calculation,
	_eLightData_Red,
	_eLightData_Blue,
	_eLightData_Green,
	_eLightData_White,
	_eLightData_Total,
};
enum _m_enBlobBase_Data
{
	_m_enBlobBase_Data_eUse = 0x01,
	_m_enBlobBase_Data_e2D = 0x02,
	_m_enBlobBase_Data_e2DRangeUP = 0x04,
	_m_enBlobBase_Data_e2DRangeLo = 0x08,
	_m_enBlobBase_Data_e2DRangeIN = 0x10,
	_m_enBlobBase_Data_e2DInvert = 0x20,
	_m_enBlobBase_Data_e3D = 0x40,
	_m_enBlobBase_Data_e3DRangeUP = 0x80,
	_m_enBlobBase_Data_e3DRangeLo = 0x100,
	_m_enBlobBase_Data_e3DRangeIN = 0x200,
	_m_enBlobBase_Data_eInvert = 0x400,
	_m_enBlobBase_Data_eFillHole = 0x800,
	_m_enBlobBase_Data_eFilter = 0x1000,
	_m_enBlobBase_Data_eMaxBlob = 0x2000,
};
enum _m_enBlobBase
{
	_m_enBlobBase_Dt,
	_m_enBlobBase_Min2D,
	_m_enBlobBase_Max2D,
	_m_enBlobBase_Filter,
	_m_enBlobBase_MinArea,
	_m_enBlobBase_Total,
};
enum _m_efBlobBase
{
	_m_efBlobBase_Min3D,
	_m_efBlobBase_Max3D,
	_m_efBlobBase_Total,
};

enum _m_eNGBlob_by
{
	_NGBlob_by_Data = 0,
	_NGBlob_by_Filter,
	_NGBlob_by_Min2D,
	_NGBlob_by_Max2D,
	_NGBlob_by_Range2D,
	_NGBlob_by_Range3D,
	_NGBlob_by_inspType,
	_NGBlob_by_inspArea,
	_NGBlob_by_WarpageOption,
	_NGBlob_by_ExceptPattern,
	_NGBlob_by_ExceptPatternScore,
	_NGBlob_by_BallRangeMin,  //0~100
	_NGBlob_by_BallRangeMax,   //0 ~100
	_NGBlob_by_HistoMin2D, //NotUSe. To Float
	_NGBlob_by_HistoMax2D, //NotUSe. To Float
	_NGBlob_by_HistoLimitMin,
	_NGBlob_by_HistoLimitMax,
	_NGBlob_by_RangeHisto,
	_NGBlob_by_MinAreaFilter,
	_NGBlob_by_GridX,
	_NGBlob_by_GridY,
	_NGBlob_by_HistogramLocalType,
	_NGBlob_by_Contrast,
	_NGBlob_by_ColorContrast,
	_NGBlob_by_DefineNumber,
	_NGBlob_by_InspTarget,  // 0: PCB(기존), 1: Pad(Bump)
	_NGBlob_by_MinimumNG,
	_NGBlob_by_BlobType,
	_NGBlob_by_ModelID,
	_NGBlob_by_ContrastExceptRangenum,
	_NGBlob_by_ContrastExceptRangeMode,
	_NGBlob_by_KernelSize,
	_NGBlob_by_DownSample,
	_NGBlob_by_InsideThreshold,
	_NGBlob_by_Total,
};

enum _m_eNGBlob_InspType
{
	Default,
	ExtraBump,
	Scratch,
	Chipping,
	Warpage,
	BallDamage,
	DetailWarpage,
	Bubbles,
	Total,
};

enum _m_eNGBlob_InspType_Renew
{
	FM,
	FM_DARK,
	FM_WHITE,
	DENT,
	ExtraBump_Renew,
	Scratch_Renew,
	Chipping_Renew,
	Warpage_Renew,
	BallDamage_Renew,
	Total_Renew,
};

enum _m_eNGBlob_InspArea
{
	InspArea_None = 0x00,
	InspArea_Inner = 0x01,
	InspArea_Outer = 0x02,
	InspArea_InOut = 0x03,
	InspArea_Total
};

enum _m_eNGBlob_Data
{
	_NGBlob_Bin2D = 0x01,
	_NGBlob_Bin3D = 0x02,
	_NGBlob_Filter = 0x04,
	_NGBlob_Width = 0x08,
	_NGBlob_Length = 0x10,
	_NGBlob_Area = 0x20,
	_NGBlob_Color = 0x40,
	_NGBlob_Fillhole = 0x80,
	_NGBlob_ExtraBumpPer = 0x100,
	_NGBlob_ExtraBumpWidth = 0x200,
	_NGBlob_Height = 0x400,
	_NGBlob_Coplanarity = 0x800, //Use Ref.Height
	_NGBlob_WarpageDeviation = 0x1000,
	_NGBlob_WarpageDeviationX = 0x2000,
	_NGBlob_WarpageDeviationY = 0x4000,
	_NGBlob_Histogram = 0x8000,
	_NGBlob_AreaFilter = 0x10000,
	_NGBlob_Clustering = 0x20000,
	_NGBlob_AND_OP = 0x40000,
	_NGBlob_Contrast = 0x80000,
	_NGBlob_ColorContrast = 0x100000,
	_NGBlob_UnderHeight = 0x200000,
	_NGBlob_ChippingType = 0x400000,
	_NGBlob_BumpNGtoOK = 0x800000,
	_NGBlob_ScratchView = 0x1000000,
	_NGBlob_ScDistance = 0x2000000,
	_NGBlob_ScThickness = 0x4000000,
	_NGBlob_UseBallDamage = 0x8000000,
	_NGBlob_UseBallLand = 0x10000000,
	_NGBlob_WidthC = 0x20000000,
	_NGBlob_LengthC = 0x40000000,
	//_NGBlob_Total = 0x16000 - 1,
};
enum _m_eNGBlob_Data2
{
	_NGBlob_UnderArea = 0x01,
	_NGBlob_RelativeHeight = 0x02,
	_NGBlob_SideWidth = 0x04,
	_NGBlob_SideLength = 0x08,
	_NGBlob_SideArea = 0x10,
	_NGBlob_UseAI = 0x20,
	_NGBlob_RectSize = 0x40,
	_NGBlob_ContrastExceptRange = 0x80,
	_NGBlob_InsideBubbleWidth = 0x100,
	_NGBlob_InsideBubbleLength = 0x200,
	_NGBlob_InsideBubbleArea = 0x400,
};

enum _m_eNGBlob_F
{
	_NGBlob_F_Min3D = 0,
	_NGBlob_F_Max3D,
	_NGBlob_F_Width,
	_NGBlob_F_Length,
	_NGBlob_F_Area,
	_NGBlob_F_ExtraBumpPer,
	_NGBlob_F_ExtraBumpWidth,
	_NGBlob_F_MaxHeight, // NG / OK 조건
	_NGBlob_F_MaxSC_Distance, // NG / OK 조건
	_NGBlob_F_MaxSC_Width,  // NG / OK 조건
	_NGBlob_F_Warpage_deviation,
	_NGBlob_F_Warpage_dev_X,
	_NGBlob_F_Warpage_dev_Y,
	_NGBlob_F_Chipping_UserTolerance_left, // 티칭 조건
	_NGBlob_F_Chipping_Width,  // NG / OK 조건
	_NGBlob_F_Chipping_Length,  // NG / OK 조건
	_NGBlob_F_InspExceptArea_left,
	_NGBlob_F_Chipping_UserTolerance_outer_left,
	_NGBlob_F_InspExceptArea_outer_left,
	_NGBlob_F_Chipping_UserTolerance_top,
	_NGBlob_F_Chipping_UserTolerance_right,
	_NGBlob_F_Chipping_UserTolerance_bottom,
	_NGBlob_F_InspExceptArea_top,
	_NGBlob_F_InspExceptArea_right,
	_NGBlob_F_InspExceptArea_bottom,
	_NGBlob_F_Chipping_UserTolerance_outer_top,
	_NGBlob_F_Chipping_UserTolerance_outer_right,
	_NGBlob_F_Chipping_UserTolerance_outer_bottom,
	_NGBlob_F_InspExceptArea_outer_top,
	_NGBlob_F_InspExceptArea_outer_right,
	_NGBlob_F_InspExceptArea_outer_bottom,
	_NGBlob_F_CrossLine,
	_NGBlob_F_HistoMin2D,
	_NGBlob_F_HistoMax2D,
	_NGBlob_F_HistoLocalAreaRate,
	_NGBlob_F_ClusteringPitch,
	_NGBlob_F_dNumGrouping,
	_NGBlob_F_Group,
	_NGBlob_F_ForeignMaskWidth,
	_NGBlob_F_ForeignMaskHeight,
	_NGBlob_F_ForeignMaskAnglePart,
	_NGBlob_F_Warpage_deviationMin,
	_NGBlob_F_WidthC,
	_NGBlob_F_LengthC,
	_NGblob_F_SideWidthMin,
	_NGblob_F_SideWidthMax,
	_NGblob_F_SideLengthMin,
	_NGblob_F_SideLengthMax,
	_NGblob_F_SideAreaMin,
	_NGblob_F_SideAreaMax,
	_NGBlob_F_MinWidth,
	_NGBlob_F_InsideBubbleWidth,
	_NGBlob_F_InsideBubbleLength,
	_NGBlob_F_InsideBubbleArea,
	_NGBlob_F_Total,
};

enum _m_eNGBlob_Group
{
	_NGBlob_None = 0,
	_NGBlob_W = 1,
	_NGBlob_L = 2,
	_NGBlob_A = 4,
	_NGBlob_H = 8,
	_NGBlob_EC = 16,
	_NGBlob_EW = 32,
	_NGBlob_SD = 64,
	_NGBlob_ST = 128,
	_NGBlob_WP = 256,
	_NGBlob_WX = 512,
	_NGBlob_WY = 1024,
	_NGBlob_C = 2048,
	_NGBlob_CC = 4096,
};

enum _m_ePadArray_by
{
	_PadArray_by_Data,
	_PadArray_by_Filter,
	_PadArray_by_Min2D,
	_PadArray_by_Max2D,
	_PadArray_by_Range2D,
	_PadArray_by_Range3D,
	_PadArray_by_Row, //PAD
	_PadArray_by_Column, //PAD
	_PadArray_by_Total
};

enum _m_ePadArray_Data
{
	_PadArray_Data_Bin2D = 1 << 0,
	_PadArray_Data_Bin3D = 1 << 1,
	_PadArray_Data_Filter = 1 << 2,
	_PadArray_Data_FillHoll = 1 << 3,
	_PadArray_Data_UseShift = 1 << 4,
	_PadArray_Data_UseShiftX = 1 << 5,
	_PadArray_Data_UseShiftY = 1 << 6,
	_PadArray_Data_UseWidth = 1 << 7,
	_PadArray_Data_UseLength = 1 << 8,
	_PadArray_Data_UseDistanceX = 1 << 9,
	_PadArray_Data_UseDistanceY = 1 << 10,
	_PadArray_Data_UseHeightDiffMin = 1 << 11,
	_PadArray_Data_UseHeightDiffMax = 1 << 12,
};

enum _m_ePadArray_F
{
	_PadArray_F_Min3D,
	_PadArray_F_Max3D,
	_PadArray_F_ShiftX,
	_PadArray_F_ShiftY,
	_PadArray_F_CenterX,
	_PadArray_F_CenterY,
	_PadArray_F_Width,
	_PadArray_F_Width_MIN,
	_PadArray_F_Width_MAX,
	_PadArray_F_Length,
	_PadArray_F_Length_MIN,
	_PadArray_F_Length_MAX,
	_PadArray_F_TeachDistanceX, //PAD 우측 distance
	_PadArray_F_TeachDistanceX_MIN,
	_PadArray_F_TeachDistanceX_MAX,
	_PadArray_F_TeachDistanceY, //PAD 아래 distance
	_PadArray_F_TeachDistanceY_MIN,
	_PadArray_F_TeachDistanceY_MAX,
	_PadArray_F_HeightAvg,
	_PadArray_F_HeightMIN,
	_PadArray_F_HeightMAX,
	_PadArray_F_Total
};
enum _m_enHistoBase
{
	_eHistoUse,
	_eHistoRangeType,
	_eHistoMax2D,
	_eHistoMin2D,
	_eHistoMinExcept,
	_eHistoMaxExcept,
	_eHistoTotal,
};
enum _m_enPatternDiffLayerType
{
	eExcept = 0,
	eInspect,
	eChipping,
	eProbe,
};
enum _m_enPatternDiffPolyDir
{
	ePolyTop = 0,
	ePolyLeft,
	ePolyBot,
	ePolyRight,
	ePolyTotal,
};

enum _m_ePackageThicknessData
{
	_m_ePackageThicknessData_UseThickness = 0x01,
	_m_ePackageThicknessData_Bin2D = 0x02,
	_m_ePackageThicknessData_Bin3D = 0x04,
	_m_ePackageThicknessData_FillHole = 0x08,
	_m_ePackageThicknessData_TH = 0x10,
	_m_ePackageThicknessData_CH = 0x20,
	_m_ePackageThicknessData_Zig = 0x40,
};

enum _m_ePackageThicknessBy
{
	_m_ePackageThicknessBy_Data = 0,
	_m_ePackageThicknessBy_Min2D,
	_m_ePackageThicknessBy_Max2D,
	_m_ePackageThicknessBy_Range2D,
	_m_ePackageThicknessBy_Range3D,
	_m_ePackageThicknessBy_RoiSize,
	_m_ePackageThicknessBy_PickerID,
	_m_ePackageThicknessBy_InspType,
	_m_ePackageThicknessBy_Total,
};

enum _m_ePackageThicknessF
{
	_m_ePackageThicknessF_MinThickness,
	_m_ePackageThicknessF_MaxThickness,
	_m_ePackageThicknessF_Min3D,
	_m_ePackageThicknessF_Max3D,
	_m_ePackageThicknessF_PickerHeight,
	_m_ePackageThicknessF_PackageHeight,
	_m_ePackageThicknessF_RepeatCnt,
	_m_ePackageThicknessF_ExcpetRange,
	_m_ePackageThicknessF_MinTH,
	_m_ePackageThicknessF_MaxTH,
	_m_ePackageThicknessF_MinCH,
	_m_ePackageThicknessF_MaxCH,
	_m_ePackageThicknessF_PickerOffset,
	_m_ePackageThicknessF_Total,
};

enum _m_ePackageThickness_Type
{
	_m_ePackageThickness_Package,
	_m_ePackageThickness_Picker,
};

enum _m_eGWire_Type
{
	_m_eGWire_Wire,
	_m_eGWire_Foot,
	_m_eGWire_Ball,
	_m_eGWire_Wedge,
	_m_eGWire_Ribbon,
};

enum _m_eGWire_Direction
{
	_m_eGWire_Direction_Left,
	_m_eGWire_Direction_TL,
	_m_eGWire_Direction_Top,
	_m_eGWire_Direction_TR,
	_m_eGWire_Direction_Right,
	_m_eGWire_Direction_BR,
	_m_eGWire_Direction_Bottom,
	_m_eGWire_Direction_BL,
};

enum _m_eGWire_Bin
{
	_m_eGWire_Bin_BW,
	_m_eGWire_Bin_Die,
	_m_eGWire_Bin_Option,
	_m_eGWire_Bin_Polygon,
	_m_eGWire_Bin_Total,
};

enum _InspAlgoType
{
	_eAlgoBW,
	_eAlgoBlob,
	_eAlgoAlign,
	_eAlgoBody_Blob,
	_eAlgoTilt,
	_eAlgoOCR,
	_eAlgoPattern,
	_eAlgoColor,
	_eAlgoGray_Mean,
	_eAlgoHeight_Mean,
	_eAlgoGray_Diff,
	_eAlgoHeight_Diff,
	_eAlgoLead_Search,
	_eAlgoBridge,
	_eAlgoLead_Tip,
	_eAlgoLead_Lift,
	_eAlgoLead_Solder,
	_eAlgoLead_Color,
	_eAlgoWidth,
	_eAlgoVolume,
	_eAlgoLead_SideSolder,
	_eAlgoTab_Search,
	_eAlgoTab,
	_eAlgoColorBand_Search,
	_eAlgoGrid,
	_eAlgoLine,
	_eAlgoEdge,
	_eAlgoSolderCone,
	_eAlgoColorXY,
	_eAlgoAlignEdge,
	_eAlgoPadAlign,
	_eAlgoPOCR,
	_eAlgoDisColor,
	_eAlgoWire,
	_eAlgoFoot,
	_eAlgoBarcode,
	_eAlgoFillet,
	_eAlgoBGA,
	_eAlgoBump,
	_eAlgoNGBlob,
	_eAlgoPadBW,		//NYJ 2021/06/09
	_eAlgoForeignOCV,
	_eAlgoBodyEdge,
	_eAlgoPackageThickness,
	_eAlgoDistance,
	_eAlgoGWire,
	_eAlgoEdgePoint,
	_eAlgoPatternDiff,
	_eAlgoShapeX,
	_eAlgoPadArray,
	_eAlgoLQBGA,
	_eAlgoNum,
};

enum _m_eAIOPT
{
	_m_eAIOPT_MissingSkip = 0x01,
	_m_eAIOPT_OCR_Backup = 0x02,
	_m_eAIOPT_AIOK_Delete = 0x04,
	_m_eAIOPT_SimilarFontTargetFont = 0x08,
	_m_eAIOPT_SOLDER_AutoInsp = 0x10,
	_m_eAIOPT_Foot_AutoInsp = 0x20,
	_m_eAIOPT_SimilarFontOKPass = 0x40,
};
enum _m_eAlgoPadBinOption
{
	_m_eAlgoPadBinOption_UseHistogram2Quadrant = 0x01,
};
template <class T>
void _Make_1DArray(T** array, __int64 sizeX)
{
	if (*array != NULL)
		delete[](*array);

	if (sizeX > 0)
		*array = new T[sizeX];
}

template <class T>
void _Make_1DArray(PCHAR FN, UINT LN, T** array, __int64 sizeX)
{
	if (*array != NULL)
		delete [] (*array); 
		//g_pMManager->pem_delete(*array, true);

	if (sizeX > 0)
		*array = new T[sizeX];
		//*array = g_pMManager->pem_new<T>(true, sizeX, FN, LN);
}

template <class T>
void _Delete_1DArray(T** array)
{
	if (*array == NULL)
		return;
	delete[](*array);
	(*array) = NULL;
}

inline int _RounD(double val)
{
	if (val < 0)
	{
		return (int)(val - 0.5005);
	}
	else
	{
		return (int)(val + 0.5005);
	}
}

inline int _RounDF(float val)
{
	if (val < 0)
	{
		return (int)(val - 0.5005);
	}
	else
	{
		return (int)(val + 0.5005);
	}
}
namespace PIAL
{
	typedef struct _tagFootPos
	{
		RECT m_sArrPadRect[_WIRE_DRAW_CNT];//shkim  background Area(searchArea)
		POINTF  m_sArrFootPoint[_WIRE_DRAW_CNT];//shkim foot Center

	}_FootPos;
	typedef struct _tagWireSearch // shkim AutoSearch result
	{
		int WireCnt;
		_FootPos m_FootPos[_WIRE_MAX_CNT];
		
	}_WireSearch;
	typedef struct _tagFootColor // shkim AutoSearch result
	{
		byte nRed;
		byte nGreen;
		byte nBlue;
	}_FootColor;

	typedef struct _tagLightTypeBuf
	{
		UCHAR* m_pucTRed;
		UCHAR* m_pucTGreen;
		UCHAR* m_pucTBlue;
		UCHAR* m_pucTWhite;

		UCHAR* m_pucMRed;
		UCHAR* m_pucMGreen;
		UCHAR* m_pucMBlue;
		UCHAR* m_pucMWhite;

		UCHAR* m_pucBRed;
		UCHAR* m_pucBGreen;
		UCHAR* m_pucBBlue;
		UCHAR* m_pucBWhite;

		int m_nImgWidth;
		int m_nImgHeight;

		int m_nImgCnt;
		int* m_pnRedValue;
		int* m_pnGreenValue;
		int* m_pnBlueValue;
		int* m_pnWhiteValue;
		int* m_pnPosition;
		int* m_pnCalculation;

		int m_nROIImgWidth;
		int m_nROIImgHeight;
		double m_dROIX;
		double m_dROIY;
	}_LightTypeBuf;

	typedef struct _tagLightData
	{
		byte m_byLightCnt;
		byte m_byArrLightData[_LIGHT_DATA_CNT][_LIGHT_CNT];
	}_lightData;

	typedef struct _tagAlgoWire
	{
		byte m_byFootCnt;
		int  m_nArrData[_eWIRE2_Total];
		RECT m_sWireRect;
		POINTF  m_sArrPoint[_WIRE_DRAW_CNT];
		float m_fArrDistance;
		float m_fArrOptionValue[_eWIRE_Total][_eMMD_Total];
		int  m_nWireRefAreaOpt;	//NYJ 2021/03/09

		_tagAlgoWire()
		{
			m_byFootCnt = 0;
			m_sWireRect.left = 0;
			m_sWireRect.right = 0;
			m_sWireRect.top = 0;
			m_sWireRect.bottom = 0;
			m_fArrDistance = 0;
			memset(m_nArrData, 0, _eWIRE2_Total * sizeof(int));
			memset(m_sArrPoint, 0, _WIRE_DRAW_CNT * sizeof(POINTF));

			for (int a = 0; a < _eWIRE_Total; a++)
			{
				memset(m_fArrOptionValue[a], 0, _eMMD_Total * sizeof(float));
			}
			m_nWireRefAreaOpt = 0;
		}
	}_AlgoWire;

	typedef struct _tagRstAlgoWire
	{
		int m_nWireCnt;
		BOOL m_bOK;
		BOOL m_bArrOK[_eWIRE_Total];
		byte m_bArrNGType;
		int m_nArrCnt;
		float m_fArrRst[_eWIRE_Total];
		float m_fArrValue_H[_WIRE_DOT_CNT];
		float m_fArrValue_W[_WIRE_DOT_CNT];
		float m_fArrValue_H2[_WIRE_DOT_CNT];
		float m_fArrValue_T[_WIRE_DOT3_CNT];
		float m_fArrValue_I[_WIRE_DOT3_CNT];
		int m_nArrX[_eDot_Total];
		int m_nArrY[_eDot_Total];
		int m_nArrX_T[_WIRE_DRAW_CNT];
		int m_nArrY_T[_WIRE_DRAW_CNT];
		int m_nArrX_I[_WIRE_DRAW_CNT];
		int m_nArrY_I[_WIRE_DRAW_CNT];
		float m_fRefArea_H;

		POINTF m_poWire_Wnd;
		POINTF m_poWire_TS;
		POINTF m_poWire_TE;
		POINTF m_poWire_RS;
		POINTF m_poWire_RE;
		RECT m_rcWire;

		BOOL m_bArrHo;
		POINTF m_poArrAB;

		int m_nBinCenterCnt;
		_tagRstAlgoWire() = default;

	}_RstAlgoWire;

	typedef struct _tagBin
	{
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
		BOOL m_bInvertCheck;
		_tagBin()
		{
			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_nTypeRange3D = _eTypeRangeIn;

			m_bInvertCheck = false;
		}

	}_Bin;

	typedef struct _tagBlob :_Bin
	{
		//Mopology
		BOOL m_bFilterIsUse;
		int m_nFilterStepNarrow;

		//Fill Holl
		BOOL m_bFillHole;

		// Blob Type
		int m_nTypeSelectBlob; // Big 0, All 1, center 2, position 3

		_tagBlob()
		{
			m_bFilterIsUse = FALSE;
			m_nFilterStepNarrow = 4;
			m_bFillHole = FALSE;
			m_nTypeSelectBlob = _eSelectCenter;
		}

	}_Blob;

	typedef struct _tagAlgoLight
	{
		_InspLightType m_eLightType;
		byte m_nRedValue;
		byte m_nGreenValue;
		byte m_nBlueValue;
		byte m_nWhiteValue;

		byte m_nLightCnt;
		byte m_nArrRedValue[10];
		byte m_nArrGreenValue[10];
		byte m_nArrBlueValue[10];
		byte m_nArrWhiteValue[10];
		byte m_nArrCalculation[10];
		byte m_nArrLightPosition[10];

		void Clone(_tagAlgoLight &inspAlgoLight)
		{
			inspAlgoLight.m_eLightType = m_eLightType;
			inspAlgoLight.m_nRedValue = m_nRedValue;
			inspAlgoLight.m_nGreenValue = m_nGreenValue;
			inspAlgoLight.m_nBlueValue = m_nBlueValue;
			inspAlgoLight.m_nWhiteValue = m_nWhiteValue;

			inspAlgoLight.m_nLightCnt = m_nLightCnt;
			if (m_nLightCnt > 0)
			{
				memcpy(&inspAlgoLight.m_nArrRedValue, m_nArrRedValue, sizeof(byte)*m_nLightCnt);
				memcpy(&inspAlgoLight.m_nArrGreenValue, m_nArrGreenValue, sizeof(byte)*m_nLightCnt);
				memcpy(&inspAlgoLight.m_nArrBlueValue, m_nArrBlueValue, sizeof(byte)*m_nLightCnt);
				memcpy(&inspAlgoLight.m_nArrWhiteValue, m_nArrWhiteValue, sizeof(byte)*m_nLightCnt);
				memcpy(&inspAlgoLight.m_nArrCalculation, m_nArrCalculation, sizeof(byte)*m_nLightCnt);
				memcpy(&inspAlgoLight.m_nArrLightPosition, m_nArrLightPosition, sizeof(byte)*m_nLightCnt);
			}
			

		}
	}_AlgoLight;

	//수정필요
	typedef struct _tagAlgoBinParam : public _Bin, _AlgoLight
	{	
		bool m_bIsSet;
		bool Moph;
		_tagAlgoBinParam()
		{
			m_bIsSet = false;
			Moph = false;
		}

	}_AlgoBinParam;

	typedef struct _tagAlgoFoot
	{
		BOOL m_bOffset;		//NYJ 2019/12/18
		int m_nUseOption;
		int m_nUseOption2;
		int m_nFindOption;
		int m_nFindOption2;
		BOOL m_bUsePatternAngle;
		RECT m_sRefArea;
		
		struct _tagAlgoBinParam m_sArrBin[_m_eFootBin_Total];
		POINTF  m_sPoint;
		RECT m_rSpecRect;
		float m_fArrOptionValue[_m_eFoot_Total][_eMMD_Total] = { 0 };
		RECT m_rInspRect = { 0 };
		int m_nFindOptionOrder[_m_eFoot_FindOption::_total] = { 0 };
		int m_nFindOptionOrder2[_m_eFoot_FindOption::_total] = { 0 };
		int m_nFootType;
		int m_nFootAngle;
		float m_fPadAreaAvgHeight;
		double m_dTeachFootAngle;

		_tagAlgoFoot()
		{
			m_bOffset = FALSE;	//NYJ 2019/12/18
			m_nUseOption = _m_eFOOT_Data_Total;
			m_nUseOption2 = _m_eFOOT_Data2_Total;
			m_sRefArea.left = 0;
			m_sRefArea.right = 0;
			m_sRefArea.top = 0;
			m_sRefArea.bottom = 0;
			m_rSpecRect.left = 0;
			m_rSpecRect.right = 0;
			m_rSpecRect.top = 0;
			m_rSpecRect.bottom = 0;
			m_sPoint.x = 0;
			m_sPoint.y = 0;
			m_bUsePatternAngle = FALSE;
			m_nFootAngle = 0;

			memset(&m_sArrBin, 0, sizeof(_tagAlgoBinParam)* _m_eFootBin_Total);

			m_nFootType = 0;
			m_fPadAreaAvgHeight = 0.0f;
			m_dTeachFootAngle = 0.0f;

			m_nFindOption = 0;
			m_nFindOption2 = 0;
			
		}
	}_AlgoFoot;

	typedef struct _tagRstAlgoFoot
	{
		BOOL m_bOK;
		int m_nFindBin;
		BOOL m_bFind;
		BOOL m_bArrOK[_m_eFoot::_m_eFoot_Total];
		float m_fArrRst[_m_eFoot::_m_eFoot_Total];

		POINTF m_poWire_Wnd;
		RECT m_rcRefArea = { 0 };
		RECT m_rcSearchArea = { 0 };
		POINT rePoinSt[_m_eFoot::_m_eFoot_Total];
		POINT rePoinEd[_m_eFoot::_m_eFoot_Total];
		int m_nMatchModelNum;
		POINTF  m_sPoint;
		float m_fAreaHeight;
		float m_fAngle;
		POINTF  m_sEnd;
		float m_fArrRst_WingArea[4];
		int m_stWingFunc{};
		int m_edWingFunc{};

		//pitting rect
		POINT m_nTeachRectLT;
		POINT m_nTeachRectLB;
		POINT m_nTeachRectRB;
		POINT m_nTeachRectRT;

		//matching score
		float MatchingScore;

		POINT DispPoinSt[_m_eFoot::_m_eFoot_Total]{};
		POINT DispPoinEd[_m_eFoot::_m_eFoot_Total]{};
		_tagRstAlgoFoot()
		{
			m_bOK = TRUE;
			m_nFindBin = 0;
			m_poWire_Wnd.x = 0;
			m_poWire_Wnd.y = 0;
			m_fAreaHeight = 0.0f;
			m_bFind = false;
			memset(m_bArrOK, 0, sizeof(BOOL)*_m_eFoot::_m_eFoot_Total);
			memset(m_fArrRst, 0, sizeof(float)*_m_eFoot::_m_eFoot_Total);
			memset(rePoinSt, 0, sizeof(POINT)*_m_eFoot::_m_eFoot_Total);
			memset(rePoinEd, 0, sizeof(POINT)*_m_eFoot::_m_eFoot_Total);
			m_sPoint.x = 0.0f;
			m_sPoint.y = 0.0f;
			m_fAngle = 0.0f;
			m_sEnd.x = 0.0f;
			m_sEnd.y = 0.0f;
			memset(m_fArrRst_WingArea, 0, sizeof(float) * 4);
			for (int i = 0; i < _m_eFoot::_m_eFoot_Total; i++)
				m_bArrOK[i] = true;
			m_nMatchModelNum = 0;

			m_nTeachRectLT.x = 0;
			m_nTeachRectLB.x = 0;
			m_nTeachRectRB.x = 0;
			m_nTeachRectRT.x = 0;

			m_nTeachRectLT.y = 0;
			m_nTeachRectLB.y = 0;
			m_nTeachRectRB.y = 0;
			m_nTeachRectRT.y = 0;

			MatchingScore = 0.0f;
		}
	}_RstAlgoFoot;

	
	typedef struct _tagInspAlgoLight
	{
		int m_nImageNum;
		_InspLightType m_eLightType;
		int m_nRedValue;
		int m_nGreenValue;
		int m_nBlueValue;
		int m_nWhiteValue;

		int m_nLightCnt;
		int *m_nArrRedValue;
		int *m_nArrGreenValue;
		int *m_nArrBlueValue;
		int *m_nArrWhiteValue;
		int *m_nArrCalculation;
		int *m_nArrLightPosition;

		void Clone(_tagInspAlgoLight &inspAlgoLight)
		{
			inspAlgoLight.m_nImageNum = m_nImageNum;
			inspAlgoLight.m_eLightType = m_eLightType;
			inspAlgoLight.m_nRedValue = m_nRedValue;
			inspAlgoLight.m_nGreenValue = m_nGreenValue;
			inspAlgoLight.m_nBlueValue = m_nBlueValue;
			inspAlgoLight.m_nWhiteValue = m_nWhiteValue;

			inspAlgoLight.m_nLightCnt = m_nLightCnt;
			inspAlgoLight.m_nArrRedValue = m_nArrRedValue;
			inspAlgoLight.m_nArrGreenValue = m_nArrGreenValue;
			inspAlgoLight.m_nArrBlueValue = m_nArrBlueValue;
			inspAlgoLight.m_nArrWhiteValue = m_nArrWhiteValue;
			inspAlgoLight.m_nArrCalculation = m_nArrCalculation;
			inspAlgoLight.m_nArrLightPosition = m_nArrLightPosition;

			if (m_nLightCnt != 0)
			{
				if (!m_nArrRedValue)			return;
				if (!m_nArrGreenValue)		return;
				if (!m_nArrBlueValue)		return;
				if (!m_nArrWhiteValue)		return;
				if (!m_nArrCalculation)		return;
				if (!m_nArrLightPosition)	return;
			}
		}
	}_InspAlgoLight;

	typedef struct _tagRoiImgBuf
	{
		int nImageSizeX;
		int nImageSizeY;

		UCHAR* imgTop_R;
		UCHAR* imgTop_G;
		UCHAR* imgTop_B;
		UCHAR* imgTop_W;

		UCHAR* imgMiddle_R;
		UCHAR* imgMiddle_B;

		UCHAR* imgBottom_R;
		UCHAR* imgBottom_B;

		UCHAR* imgSide1_R;
		UCHAR* imgSide1_G;
		UCHAR* imgSide1_B;

		UCHAR* imgSide2_R;
		UCHAR* imgSide2_G;
		UCHAR* imgSide2_B;

		UCHAR* imgSide3_R;
		UCHAR* imgSide3_G;
		UCHAR* imgSide3_B;

		UCHAR* imgSide4_R;
		UCHAR* imgSide4_G;
		UCHAR* imgSide4_B;

		_tagRoiImgBuf()
		{
			imgTop_R = NULL;
			imgTop_G = NULL;
			imgTop_B = NULL;
			imgTop_W = NULL;

			imgMiddle_R = NULL;
			imgMiddle_B = NULL;

			imgBottom_R = NULL;
			imgBottom_B = NULL;

			imgSide1_R = NULL;
			imgSide1_G = NULL;
			imgSide1_B = NULL;

			imgSide2_R = NULL;
			imgSide2_G = NULL;
			imgSide2_B = NULL;

			imgSide3_R = NULL;
			imgSide3_G = NULL;
			imgSide3_B = NULL;

			imgSide4_R = NULL;
			imgSide4_G = NULL;
			imgSide4_B = NULL;

			nImageSizeX = 0;
			nImageSizeY = 0;
		}

	}_InspRoiImgBuf;

	typedef struct _tagAlgoColorBase
	{
		BOOL m_bUseColor;
		BOOL m_bUseAngleColor;
		int m_nPolygonCnt;
		POINT m_ptArrPolygon[_COLORALGO_POLYGON_CNTS][_POLYGON_POINT_CNTS];
		byte m_byType;
		BOOL m_bUseRGB[_COLORALGO_POLYGON_CNTS];
		byte m_byRange[_COLORALGO_POLYGON_CNTS];
		byte m_byMin[_COLORALGO_POLYGON_CNTS];
		byte m_byMax[_COLORALGO_POLYGON_CNTS];
		BOOL m_bInvert;
		byte m_byColorLightType;

		BOOL m_bUseIntensity;
		int m_nRangeType;
		int m_nRangeMin;
		int m_nRangeMax;

		_tagAlgoColorBase()
		{
			m_bUseColor = FALSE;
			m_bUseAngleColor = FALSE;
			m_nPolygonCnt = 1;
			m_byType = 0;
			for (int n = 0; n < _COLORALGO_POLYGON_CNTS; n++)
			{
				for (int k = 0; k < _POLYGON_POINT_CNTS; k++)
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
			m_nRangeType = _eTypeRangeIn;
			m_nRangeMin = 0;
			m_nRangeMax = 80;
		}
	}_AlgoColorBase;

	typedef struct _tagRstInspAC	// BW 검사 결과
	{
		int m_nOKInspAC;
		int m_nInspAC;
		float m_fRstInspAC[_InspAC_R_Total];
		RECT m_rcArrRect[_InspAC_R_Rect_Total];
		_tagRstInspAC()
		{
			m_nOKInspAC = 1;
			m_nInspAC = 0;
			memset(m_fRstInspAC, 0, _InspAC_R_Total * sizeof(float));
			memset(m_rcArrRect, 0, _InspAC_R_Rect_Total * sizeof(RECT));
		}
	}_RstInspAC;

	typedef struct _tagAngleColorBase
	{
		int m_nArrInspAC[_InspAC_T3_Total];
		byte m_byArrInspAC[_InspAC_T2_Total];
		float m_fArrInspAC[_InspAC_T_Total];
		_tagAngleColorBase()
		{
			memset(m_nArrInspAC, 0, _InspAC_T3_Total * sizeof(int));
			memset(m_byArrInspAC, 0, _InspAC_T2_Total * sizeof(byte));
			memset(m_fArrInspAC, 0, _InspAC_T_Total * sizeof(float));
		}
	}_AngleColorBase;

	typedef struct _tagTotalInspExceptArea
	{
		int m_nUsedMaskingValue;
		std::vector<RECT> m_rcArrMaskingROI;

		int m_nUsedInspPolygon;
		BOOL m_bConvetExceptROI;
		POINTF m_ptArrInspPolygon[_MAX_INSP_AREA_COUNT];

		int m_nUsedWndPolygon;
		POINTF m_ptArrWndPolygon[_MAX_INSP_AREA_COUNT];

		int m_nArrObjPolyInspCnt = 0;
		POINTF* m_ptArrObjPolyInsp;

		int m_nArrObjPolyExceptCnt = 0;
		POINTF* m_ptArrObjPolyExcept;
		double dAngle;

		void Shift(int nShiftX, int nShiftY)
		{
			for (int i = 0; i < m_nUsedMaskingValue; i++)
			{
				m_rcArrMaskingROI[i].left -= nShiftX;
				m_rcArrMaskingROI[i].top -= nShiftY;
				m_rcArrMaskingROI[i].right -= nShiftX;
				m_rcArrMaskingROI[i].bottom -= nShiftY;
			}

			for (int i = 0; i < m_nUsedInspPolygon; i++)
			{
				m_ptArrInspPolygon[i].x -= nShiftX;
				m_ptArrInspPolygon[i].y -= nShiftY;
			}

			for (int i = 0; i < m_nUsedWndPolygon; i++)
			{
				m_ptArrWndPolygon[i].x -= nShiftX;
				m_ptArrWndPolygon[i].y -= nShiftX;
			}
		
		}

		_tagTotalInspExceptArea()
		{
			m_nUsedMaskingValue = 0;

			//RECT rcInit;
			//rcInit.left = 0; rcInit.bottom = 0; rcInit.right = 0; rcInit.top = 0;
			//for (int i = 0; i < _MAX_MASKING_NUM; i++)
			//	m_rcArrMaskingROI[i] = rcInit;
			m_rcArrMaskingROI.clear();
			//m_rcArrMaskingROI = NULL;

			// for Insp Area Polygon
			m_nUsedInspPolygon = 0;
			m_bConvetExceptROI = FALSE;

			POINTF ptInit;
			ptInit.x = 0; ptInit.y = 0;

			for (int i = 0; i < _MAX_INSP_AREA_COUNT; i++)
				m_ptArrInspPolygon[i] = ptInit;

			m_nUsedWndPolygon = 0;
			for (int i = 0; i < _MAX_INSP_AREA_COUNT; i++)
				m_ptArrWndPolygon[i] = ptInit;

			dAngle = 0;
			m_nArrObjPolyInspCnt = 0;
			m_nArrObjPolyExceptCnt = 0;
			m_ptArrObjPolyInsp = NULL;
			m_ptArrObjPolyExcept = NULL;
		}

	}_TotalInspExceptArea;

	typedef struct _tagfileAlgoPath
	{
		CString m_sPathModelTeach;
		int type;

		//2017.03.29 shkim Divide Area
		int DiviLnR[_CNT_PATTERN_PATH];
		int DiviLnC[_CNT_PATTERN_PATH];

		double GapLnR[_CNT_PATTERN_PATH][_CNT_PATTERN_DIVISION_R];
		double GapLnC[_CNT_PATTERN_PATH][_CNT_PATTERN_DIVISION_C];

		double DiviScore[_CNT_PATTERN_PATH][_CNT_PATTERN_SCORE];

		BOOL DetailSearch[_CNT_PATTERN_PATH];
		BOOL m_bUseCharacter;
	}_fileAlgoPath;

	typedef struct _stAlgoBlob
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

		struct _tagAlgoColorBase m_sAlgoColorBase;
		// 	int m_nHeightDivisionLevel;

		BOOL	m_bIsModelSubImg;
		BOOL	m_bUsePattern;
		wchar_t m_sModelPath[_MAX_STRLEN] = { 0 };
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
		POINTF m_pShiftSt = { 0 };
		POINTF m_pShiftEd = { 0 };
		int m_nShiftDirIndex;

		BOOL m_bUseThick;
		float m_fThick;
		_stAlgoBlob()
		{
			m_byIPCClass = 0;
			m_bInvertCheck = FALSE;
			m_dAreaMin = 80.;
			m_dAreaMax = 120.;
			m_nTypeSelectBlob = _eSelectCenter;
			m_bFillHole = FALSE;
			m_nFilterStepNarrow = 4;

			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_fHeightAvg = 0.0F;
			m_nTypeRange3D = _eTypeRangeIn;

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

			m_nCoilThMax = 10;
			m_nCoilThMin = 10;

			m_nBlobSizeDistanceType = 0;

			m_bUseThick = FALSE;
			m_fThick = 0;

			m_bAreaIsUse = FALSE;
			m_bFilterIsUse = FALSE;
			m_bInsp2D_pattern = FALSE;
			m_bInsp3D_pattern = FALSE;
			m_bOKCoilRst = FALSE;
			m_bShiftIsUse = FALSE;
			m_bUseBlobSizeLength = FALSE;
			m_bUseBlobSizeWidth = FALSE;
			m_dAreaCurrent = 0.;
			m_dHeightRateMin_pattern = 0.;
			m_dHeightRateMax_pattern = 0.;
			m_dShiftX = 0.;
			m_dShiftY = 0.;
			m_nMaxBinary_pattern = 0;
			m_nMinBinary_pattern = 0;
			m_nShiftDirIndex = 0;
			m_nTypeRange2D_pattern = 0;
			m_nTypeRange3D_pattern = 0;
			
		}
	}_AlgoBlob;

	typedef struct _tagRstAlgoBlob	// Blob 검사 결과
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

		RECT m_rcRect_I = { 0 };

		BOOL m_bUseSizeWidth;
		BOOL m_bUseSizeLength;
		POINTF m_poDrawCenter = { 0 };

		int m_nArrRectCnt;
		RECT m_rcRectT = { 0 };
		RECT m_rcArrRect[_BLOB_RECT_CNTS] = { 0 };

		BOOL m_bOKDistance;
		double m_dRstDistance;

		bool m_bOKCoilRst;

		POINTF m_pShiftSt = { 0 };
		POINTF m_pShiftEd = { 0 };
		int m_nShiftDirIndex;
		POINTF m_pFPMatchPos[4];


		BOOL m_bOKThick;
		float m_fRstThick;

		_tagRstAlgoBlob()
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
			memset(m_rcArrRect, 0, _BLOB_RECT_CNTS * sizeof(RECT));

			m_nShiftDirIndex = 0;
			memset(m_pFPMatchPos, 0, 4 * sizeof(POINTF));

			m_fRstThick = 0.;

			m_bOKThick = FALSE;
		}
	}_RstAlgoBlob;

	//BumpDataBGA
	typedef struct _tagTeachBumpBoundary
	{
		int nBumpID;
		RECT m_rcRect_I;

		_tagTeachBumpBoundary()
		{
			nBumpID = 0;
			memset(&m_rcRect_I, 0, sizeof(RECT));
		}
	}_TeachBumpBoundary;

	enum _InspLightTypeXray
	{
		e2D,
		eSlice,
		MaxXrayType,
	};

	typedef struct tagInspLightXray
	{

		typedef struct tagLightInfoXray
		{
			int m_SliceImgIndex;
			double m_Gamma;
		}SLightInfoXray;

		int m_nCntXrayLT;
		_InspLightTypeXray m_eLightTypeXray;
		SLightInfoXray m_arrXraySliceLT[MAX_THUMB_COUNT];


	}InspLightXray;

	typedef struct _tagAlgoVoid
	{
		BOOL m_bUseInspVoid;
		float m_fTeachVoidRate;
		float m_fTeachMaxVoidRate;

		BOOL m_bUseFillHole;
		BOOL m_bUseFilter;
		int m_nFilterSize;

		int m_nTypeRange2D;
		int m_nMinBinary;
		int m_nMaxBinary;

		float m_fTeachWeight;

		float m_VoidHullCircleRate;
		int m_VoidPixelDiffValue;

		float m_fLogValue;
		float m_fGammaValue;

		int m_nMethodType;

		BOOL m_bUseSum;
		BOOL m_bUseMax;

		float m_fSliceInterval;

		int m_nBoundaryCnt;
		struct _tagTeachBumpBoundary* TeachBoundary; // 필요할수 있음.

		struct tagInspLightXray _InspLightXray;

	}_AlgoVoid;

	typedef struct _stAlgoBump
	{
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

		BOOL m_bTeachWidthUse; //Diamter
		double m_dTeachWidth;
		double m_dTeachWidthRateMin;
		double m_dTeachWidthRateMax;

		BOOL m_bTeachWidth2Use; //Width
		double m_dTeachWidth2;
		double m_dTeachWidthRateMin2;
		double m_dTeachWidthRateMax2;

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

		struct _tagAlgoColorBase m_sAlgoColorBase;
		// 	int m_nHeightDivisionLevel;

		BOOL m_bCircleOpt;	// PYJ 2019/05/25

		BOOL m_bUseCircleRate;
		double m_dCircleRate;
		//0: Default, 2: floatball, 3: blob , 4: Tiny, 5: Pad 
		int m_InspAreaType;

		BOOL m_bUseNGBump;
		BOOL   m_bUseUnCoining;
		double   m_dReverseArea;
		bool UseBridge;

		BOOL m_bUseShiftR;
		double m_dShiftR;

		BOOL m_bUsePitch;
		double m_dTolPitch;

		BOOL m_bUseContrast;
		double m_dTolContrast;

		BOOL m_bUseHeightDev;
		double m_dHeightDev;

		int m_HeightAreaType;
		int m_HeightAreaType_COP;

		BOOL m_bUseVolume;
		double m_dTeachVolume;
		double m_dTeachVolumeRateMin;
		double m_dTeachVolumeRateMax;

		BOOL m_bUseVolume_Flux;
		double m_dTeachVolumeHeightDifference;  //um

		BOOL m_bUseAbsoluteHeight;

		BOOL m_bSelftAlign;
		double m_SelftAlignCenterX;
		double m_SelftAlignCenterY;

		int m_ShapeType; // 0:Circle, 1:Rectangle
		float m_ShapeAngle;
		double PODTeachCX;
		double PODTeachCY;

		BOOL m_bUseRelHeightValue;
		double m_dRelHeightValue;
		_stAlgoBump()
		{
			m_dAreaMin = 80.;
			m_dAreaMax = 120.;
			m_nTypeSelectBlob = _eSelectCenter;
			m_bFillHole = FALSE;
			m_nFilterStepNarrow = 4;

			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_fHeightAvg = 0.0F;
			m_nTypeRange3D = _eTypeRangeIn;

			m_dTechCenterX = 0.0;
			m_dTechCenterY = 0.0;

			m_bTeachWidthUse = FALSE;
			m_dTeachWidth = 0.0;
			m_dTeachWidthRateMin = 80.;
			m_dTeachWidthRateMax = 120.;

			m_bTeachWidth2Use = FALSE;
			m_dTeachWidth2 = 0.0;
			m_dTeachWidthRateMin2 = 80.;
			m_dTeachWidthRateMax2 = 120.;

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

			m_bCircleOpt = FALSE;

			m_bUseCircleRate = FALSE;
			m_dCircleRate = 0.;
			m_InspAreaType = 0;

			m_bUseNGBump = FALSE;
			m_bUseUnCoining = FALSE;
			m_dReverseArea = 0.;
			UseBridge = false;

			m_bUseShiftR = FALSE;
			m_dShiftR = 0.;

			m_bUsePitch = FALSE;
			m_dTolPitch = 0;

			m_bUseContrast = FALSE;
			m_dTolContrast = 0;

			m_bUseHeightDev = FALSE;
			m_dHeightDev = 0;

			m_HeightAreaType = 0;
			m_HeightAreaType_COP = 3;

			m_bUseVolume = FALSE;
			m_dTeachVolume = 0.;
			m_dTeachVolumeRateMin = 0.;
			m_dTeachVolumeRateMax = 0.;

			m_bUseVolume_Flux = FALSE;
			m_dTeachVolumeHeightDifference = 0.;

			m_bUseAbsoluteHeight = FALSE;

			m_bSelftAlign = FALSE;
			m_SelftAlignCenterX = 0.;
			m_SelftAlignCenterY = 0.;

			m_ShapeType = 0; //Pad에서 사용
			m_ShapeAngle = 0;

			m_dShiftX = 0.;
			m_dShiftY = 0.;
			m_dAreaCurrent = 0.;
			m_bUseBlobSizeWidth = FALSE;
			m_bUseBlobSizeLength = FALSE;
			m_bShiftIsUse = FALSE;
			m_bFilterIsUse = FALSE;
			m_bAreaIsUse = FALSE;
			PODTeachCX = 0.;
			PODTeachCY = 0.;

			m_bUseRelHeightValue = FALSE;
			m_dRelHeightValue = .0;
		}
	}_AlgoBump;

	typedef struct _tagRstEachVoid
	{
		BOOL bResult; //VoidRatio가 티칭값보다 작으면 true(기본값 true)
		RECT rtVoid; //Void Rect.
		float fVoidRate; //Bump 대비 Void 비율(mm3)
		float fVoidArea; //Void Area (mm2)
		int nVoidptCnt;
		POINT *ptVoid;

		_tagRstEachVoid()
		{
			bResult = FALSE;
			rtVoid = RECT();
			fVoidRate = 0.0f;
			fVoidArea = 0.0f;
			nVoidptCnt = 0;
			ptVoid = nullptr;
		}
	}_RstEachVoid;

	typedef struct _tagRstPer1Img
	{
		int nTotalCnt; //bump 내 EachVoid 개수
		float fVoidRate; //EachVoid에서 나온 결과 합
		float fVoidArea; //EachVoid에서 나온 결과 합
		float fBumpArea; //Defect 찾기 전 계산한 bump Area
		int nMaxVoidID; //가장 큰 Void 값을 가지는 EachVoid Index
		BOOL bResult; //VoidRatio가 티칭값보다 작으면 true(기본값 true)

		struct _tagRstEachVoid* pEachVoid; //각 void 데이터

		_tagRstPer1Img()
		{
			nTotalCnt = 0;
			fVoidRate = .0f;
			fVoidArea = .0f;
			fBumpArea = .0f;
			nMaxVoidID = 0;
			bResult = FALSE;
			pEachVoid = nullptr;
		}
	}_RstPer1mg;

	typedef struct _tagRstAlgoVoid
	{
		int BumpID;							//input 에 사용 된 ID 그대로 적용
		float fBumpSumArea;

		float fVoidSumArea;
		float fVoidSumRate;					//RstPer1Img에서 나온 결과 합. 
		BOOL bIsOKSum;						//RstPer1Img에서 나온 결과 중 NG 가 있으면 NG

		int nMaxId;							//가장 큰 Void 값을 가지는 RstPer1Img Index
		float fVoidMaxArea;					//
		float fVoidMaxRate;					//
		BOOL bIsOKMax;						//

		int nSummaryRtCnt;					//sum에 대한 void rect 개수
		RECT *rtSummary;					//sum에 대한 void rect
		int nRst1ImgCnt;					//RstPer1Img 스트럭쳐 개수
		struct _tagRstPer1Img* RstPer1Img;	//1개의 Bump에 검사한 결과 데이터.(2D는 단수, Slice는 복수.)
		_tagRstAlgoVoid()
		{
			BumpID = 0;
			fBumpSumArea = .0f;

			fVoidSumArea = .0f;
			fVoidSumRate = .0f;
			bIsOKSum = FALSE; //*제거 할 수 있음.

			nMaxId = 0;
			fVoidMaxRate = .0f;
			fVoidMaxArea = .0f;
			bIsOKMax = FALSE;

			nSummaryRtCnt = 0;
			rtSummary = nullptr;
			nRst1ImgCnt = 0;
			RstPer1Img = nullptr;
		}
	}_RstAlgoVoid;

	typedef struct _tagBumpRstAlgoVoid
	{
		int m_numBump;
		struct _tagTempRstAlgoVoid* pBump;

		_tagBumpRstAlgoVoid()
		{
			m_numBump = 0;
			pBump = nullptr;
		}

	}_BumpRstAlgoVoid;

	typedef struct _tagRstAlgoBump	// Blob 검사 결과
	{
		double m_dRstArea;
		double m_dRstShiftX;
		double m_dRstShiftY;
		double m_dRstHeightMean;
		double m_dRstWidth;
		double m_dRstLength;
		double m_dRstArea_Reverse;
		double m_dTeachArea_Reverse;
		double m_dRstWidth_Reverse;
		double m_dRstLength_Reverse;
		double m_dRstDistance;
		double m_dRstCircleRate;
		double m_dRstShiftR;
		double m_dRstPitch;
		double m_dRstContrast;
		double m_dRstHeightDev;
		double m_dRstHeightRelative;
		double m_dRstHeightAbsolute;
		double m_dRstHeightSurface;
		double m_dRstDiameter;
		double m_dRstCoplanarity;
		double m_dRstWidth2;
		double m_dRstAreaS1;
		double m_dRstAreaS2;
		double m_dRstAreaS3;
		double m_dRstAreaS4;
		double m_dRstAreaS5;
		double m_dTeachAreaS1;
		double m_dTeachAreaS2;
		double m_dTeachAreaS3;
		double m_dTeachAreaS4;
		double m_dTeachAreaS5;

		// OK 판정 //
		BOOL m_bOKArea;
		BOOL m_bOKShiftX;
		BOOL m_bOKShiftY;
		BOOL m_bOKShiftR;
		BOOL m_bOKHeight;
		BOOL m_bOKWidth; //Radius
		BOOL m_bOKLength;
		BOOL m_bOKArea_Reverse;
		BOOL m_bOKWidth_Reverse;
		BOOL m_bOKLength_Reverse;
		BOOL m_bOKDistance;
		BOOL m_bOKCircleRate;
		BOOL m_bOKPitch;
		BOOL m_bOKContrast;
		BOOL m_bOKHeightDev;
		BOOL m_bOKWidth2; //Width
		BOOL m_bOKAreaS1;
		BOOL m_bOKAreaS2;
		BOOL m_bOKAreaS3;
		BOOL m_bOKAreaS4;
		BOOL m_bOKAreaS5;
		BOOL m_bUseZoneBasedArea;
		
	/*	BOOL m_bUseSizeWidth;
		BOOL m_bUseSizeLength;*/
		POINTF m_poDrawCenter;

		RECT m_rcRect_I;
		RECT m_rcRectT;
		int m_nArrRectCnt;
		RECT m_rcArrRect[_BUMP_RECT_CNTS];

		BOOL m_bDoubeCoining; // 아직 사용않함.

		BOOL m_bOKVolume;
		BOOL m_bBallDamage;
		double m_dRstVolume;
		double m_dRstBottomHeight;
		double m_dShapeAngle;
		BOOL m_bSwappedWL;
		BOOL m_bBumpNGToOK;

		_tagRstAlgoBump()
		{
			m_dRstArea = 0.;
			m_dRstShiftX = 0.;
			m_dRstShiftY = 0.;
			m_dRstShiftR = 0.;
			m_dRstHeightMean = 0.;
			m_dRstWidth = 0.;
			m_dRstLength = 0.;
			m_dRstDistance = 0.;
			m_dRstArea_Reverse = 0.;
			m_dTeachArea_Reverse = 0.;
			m_dRstWidth_Reverse = 0;
			m_dRstLength_Reverse = 0;
			m_dRstCircleRate = 0.;
			m_dRstPitch = 0.;
			m_dRstContrast = 0;
			m_dRstHeightDev = 0;
			m_dRstHeightRelative = 0;
			m_dRstHeightAbsolute = 0;
			m_dRstHeightSurface = 0;
			m_dRstDiameter = 0;
			m_dRstCoplanarity = 0;
			m_dRstWidth2 = 0;

			m_bOKArea = FALSE;
			m_bOKShiftX = FALSE;
			m_bOKShiftY = FALSE;
			m_bOKShiftR = FALSE;
			m_bOKHeight = FALSE;
			m_bOKWidth = FALSE;
			m_bOKLength = FALSE;
			m_bOKDistance = FALSE;
			m_bOKArea_Reverse = TRUE;	// always Start TRUE
			m_bOKWidth_Reverse = TRUE;
			m_bOKLength_Reverse = TRUE;
			m_bOKCircleRate = FALSE;
			m_bOKPitch = FALSE;
			m_bOKContrast = FALSE;
			m_bOKHeightDev = FALSE;
			m_bOKWidth2 = FALSE;
			m_bOKAreaS1 = TRUE;
			m_bOKAreaS2 = TRUE;
			m_bOKAreaS3 = TRUE;
			m_bOKAreaS4 = TRUE;
			m_bOKAreaS5 = TRUE;
			m_bUseZoneBasedArea = FALSE;
			m_dRstAreaS1 = 0.;
			m_dRstAreaS2 = 0.;
			m_dRstAreaS3 = 0.;
			m_dRstAreaS4 = 0.;
			m_dRstAreaS5 = 0.;
			m_dTeachAreaS1 = 0.;
			m_dTeachAreaS2 = 0.;
			m_dTeachAreaS3 = 0.;
			m_dTeachAreaS4 = 0.;
			m_dTeachAreaS5 = 0.;
			/*m_bUseSizeWidth = FALSE;
			m_bUseSizeLength = FALSE;*/

			m_nArrRectCnt = 0;
			memset(m_rcArrRect, 0, _BUMP_RECT_CNTS * sizeof(RECT));

			m_bDoubeCoining = FALSE;
			memset(&m_rcRect_I, 0,sizeof(RECT));
			memset(&m_rcRectT, 0, sizeof(RECT));

			m_bOKVolume = FALSE;
			m_dRstVolume = 0.;
			m_dRstBottomHeight = 0.;
			m_dShapeAngle = 0.;
			m_bBallDamage = FALSE;

			m_poDrawCenter.x = 0.f;
			m_poDrawCenter.y = 0.f;
			m_bSwappedWL = FALSE;
			m_bBumpNGToOK = FALSE;
		}
	}_RstAlgoBump;


	typedef struct tagTeachBGA
	{
		int m_numBump;
		struct tagTeachBump* pBump;

		tagTeachBGA()
		{
			m_numBump = 0;
			pBump = nullptr;
		}

	}TeachBGA;

	typedef struct tagTeachBump
	{
		double m_cx;
		double m_cy;
		double m_radius;
		double m_heightMean;
		double m_Circlerate;
		double m_width;
		double m_Length;
		double m_Area;
		RECT m_rcRect_I;

		tagTeachBump()
		{
			m_cx = 0;
			m_cy = 0;
			m_radius = 0;
			m_heightMean = 0.;
			m_width = 0;
			m_Length = 0;
			m_Area = 0;
			m_Circlerate = 0.;
			memset(&m_rcRect_I, 0, sizeof(RECT));
		}
	}TeachBump;

	typedef struct tagTeachBGA2
	{
		int m_numBump;
		struct tagTeachBump2* pBump;

		tagTeachBGA2()
		{
			m_numBump = 0;
			pBump = nullptr;
		}

	}TeachBGA2;

	typedef struct TagPadDrawShape
	{
	public:
		int m_ShapeType;
		POINTF m_ptInspCenter;
		float m_Width;
		float m_Length;
		float m_Angle;
		BOOL m_bSwapWL;
		TagPadDrawShape()
		{
			m_ShapeType =0;
			m_ptInspCenter.x=0;
			m_ptInspCenter.y = 0;
			m_Width =0;
			m_Length=0;
			m_Angle=0;
			m_bSwapWL = FALSE;
		}
	}PadDrawShape;

	typedef struct tagTeachBump2
	{
		double m_cx;
		double m_cy;
		double m_radius;
		double m_heightMean;
		double m_Circlerate;
		double m_width;
		double m_Length;
		double m_Area;
		double m_Angle;
		RECT m_rcRect_I;
		int m_nType;

		tagTeachBump2()
		{
			m_cx = 0;
			m_cy = 0;
			m_radius = 0;
			m_heightMean = 0.;
			m_width = 0;
			m_Length = 0;
			m_Area = 0;
			m_Angle = 0;
			m_nType = 0;
			m_Circlerate =0.;
			memset(&m_rcRect_I, 0, sizeof(RECT));
		}
	}TeachBump2;
	const int _BGA_RECT_CNTS = 8;
	typedef struct _stAlgoBGA
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

		struct _tagAlgoColorBase m_sAlgoColorBase;
		
	
		BOOL m_bUseVolume;		//추가
		double dVolumeRateMin;
		double dVolumeRateMax;

		BOOL m_bUseZCalib;
		BOOL m_bUseHeightOffset;
		double m_dHeightOffset;

		BOOL m_bUseGWarp;
		BOOL m_bUseRemovePadVol;
		BOOL m_bUseInspAreaHOffset;
		double m_dInspAreaHOffset;
		BOOL m_bVisualizeBridge;
		BOOL m_bUseNoFluxLimit;
		int m_nNoFluxLimit;
		double m_dRemovePadVol;
		BOOL m_bUseZoneBasedArea;
		double m_dSectorHeight;
		double m_dTeachAreaS1;
		double m_dTeachAreaS2;
		double m_dTeachAreaS3;
		double m_dTeachAreaS4;
		double m_dTeachAreaS5;
		int m_nInnerRad;
		int m_nOuterRad;
		double m_dPadHeight;
		
		BOOL m_bUseModelMatching;
		BOOL m_bUseModel;
		wchar_t m_BGAMaskName[_MAX_STRLEN];

		BOOL m_bUseCoplanarity;
		double m_CoplanarMax;
		double m_CoplanarMin;

		//0: Default, 1: Coinning
		int m_InspAreaType;
		BOOL m_bUseBridge;

		BOOL   m_bUseNGBump;
		int   m_nNGBumpType;  // 0: defalt  1: In Bump ,2 :Except Bump
		struct _tagBlob m_Blob_NGBump;

		BOOL   m_bUseUnCoining;
		float m_fUncoiningValue;
		struct _tagBlob m_Blob_uncoining;

		BOOL m_bUsePitch;
		double m_dTolPitch;

		BOOL m_bUseShiftR;
		double m_dShiftR;

		BOOL m_bUseContrast;
		double m_dTolContrast;

		BOOL m_bUseGridOffsetX;
		BOOL m_bUseGridOffsetY;
		double m_dTolGridOffsetX;
		double m_dTolGridOffsetY;

		int m_HeightAreaType;
		int m_HeightAreaType_COP;
		int m_SelectOffSetType;
		int m_nCoplOption;
		BOOL m_bUseGoldenDevice;

		//Align Bump
		int m_nIndexTarget1Bump;
		int m_nIndexTarget2Bump;
		//Coplanirity Bump
		int m_nIndexCopleBump;
		//Calc Copl MaxMinBump
		int m_nIndexMaxBump;
		int m_nIndexMinBump;

		BOOL m_UseNormalizeBumpWidth;
		int m_nBasisBall;
		int m_nOffSetBall1;
		int m_nOffSetBall2;
		int m_nBig3Ball1;
		int m_nBig3Ball2;
		int m_nBig3Ball3;
		int m_nSelectBall;
		BOOL m_bUseTwist;
		double m_dTwist;
		
		int m_nBallMaskSize;
		
		BOOL m_bFluxUse;
		double m_dTeachFlux;
		BOOL m_bUseAbsoluteHeight;
		BOOL m_bTeachWidth2Use;
		int m_SelectDirection;
		int m_SelectDirection2;
		int m_SelectAlgoID;
		int m_SelectAlgoID2;
		double m_dTolDistance;
		double m_dTolDistance2;
		BOOL UseS1;
		BOOL UseS2;
		int TypeTeachWLUnit;
		double m_dMinDistance;
		double m_dMinDistance2;

		int m_nNumArea;
		RECT m_rcRoi[_BGA_RECT_CNTS];
		BOOL m_bUseRemoveLargeBlob;
		_stAlgoBGA()
		{
			m_byIPCClass = 0;
			m_bInvertCheck = FALSE;
			m_dAreaMin = 80.;
			m_dAreaMax = 120.;
			m_nTypeSelectBlob = _eSelectCenter;
			m_bFillHole = FALSE;
			m_nFilterStepNarrow = 4;

			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_nTypeRange3D = _eTypeRangeIn;

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
			m_bUseBlobSizeWidth = FALSE;
			m_bUseBlobSizeLength = FALSE;

			m_bUseHeight = FALSE;
			m_dTeachHeight = 0.;
			m_dTeachHeightMax = 0.;
			m_dTeachHeightMin = 0.;
			m_InspAreaType = 0;

			m_bUseNGBump = FALSE;
			m_nNGBumpType = 0;
			m_Blob_NGBump.m_nTypeSelectBlob = _eSelectMix;

			m_bUseUnCoining = FALSE;
			m_fUncoiningValue = 0.0f;

            m_bUseCoplanarity = FALSE;
			m_CoplanarMax = 0.0f;
			m_CoplanarMin = 0.0f;

			m_bUseShiftR = FALSE;
			m_dShiftR = 0.0f;


			m_bUsePitch = FALSE;
			m_dTolPitch = 0.;

			m_bUseContrast = FALSE;
			m_dTolContrast = 0;

			m_bUseGridOffsetX = FALSE;
			m_bUseGridOffsetY = FALSE;
			m_dTolGridOffsetX = 0;
			m_dTolGridOffsetY = 0;

			m_HeightAreaType = 0;
			m_HeightAreaType_COP = 0;
			m_SelectOffSetType = 0;
			m_nCoplOption = 0;
			m_bUseGoldenDevice = FALSE;

			//Align Bump
			m_nIndexTarget1Bump = 15;
			m_nIndexTarget2Bump =3;
			//Coplanirity Bump
			m_nIndexCopleBump = m_nIndexTarget1Bump;

			m_UseNormalizeBumpWidth = FALSE;
			m_nBasisBall = 0;
			m_nOffSetBall1 = -1;
			m_nOffSetBall2 = -1;
			m_nBig3Ball1 = -1;
			m_nBig3Ball2 = -1;
			m_nBig3Ball3 = -1;
			m_nSelectBall = -1;
			m_bUseTwist = FALSE;
			m_dTwist = 0;

			m_nBallMaskSize = 2;

			m_bFluxUse = FALSE;
			m_dTeachFlux = 0.;

			m_bUseAbsoluteHeight = FALSE;
			m_bTeachWidth2Use = FALSE;

			m_nIndexMaxBump = -1;
			m_nIndexMinBump = -1;
			m_SelectDirection = -1;
			m_SelectDirection2 = -1;
			m_SelectAlgoID = -1;
			m_SelectAlgoID2 = -1;
			m_dTolDistance = 0;
			m_dTolDistance2 = 0;
			UseS1 = FALSE;
			UseS2 = FALSE;
			TypeTeachWLUnit = 0;

			dVolumeRateMax = 0.;
			dVolumeRateMin = 0.;
			m_bAreaIsUse = FALSE;
			m_bFilterIsUse = FALSE;
			m_bShiftIsUse = FALSE;
			m_bUseVolume = FALSE;
			m_dAreaCurrent = 0.;
			m_dShiftX = 0.;
			m_dShiftY = 0.;
			m_dTeachArea = 0.;
			m_dTeachVolume = 0.;
			m_dMinDistance = 0;
			m_dMinDistance2 = 0;
			m_nNumArea = 0;
			memset(m_rcRoi, 0, _BGA_RECT_CNTS * sizeof(RECT));

			m_bUseZCalib = FALSE;
			m_bUseHeightOffset = FALSE;
			m_dHeightOffset =0;

			m_bUseZoneBasedArea = FALSE;
			m_dSectorHeight = 0.;
			m_dTeachAreaS1=0.;
			m_dTeachAreaS2=0.;
			m_dTeachAreaS3=0.;
			m_dTeachAreaS4=0.;
			m_dTeachAreaS5=0.;
			m_nInnerRad=50;
			m_nOuterRad=150;
			m_dPadHeight=0.;

			m_bUseGWarp = FALSE;
			m_bUseInspAreaHOffset = FALSE;
			m_dInspAreaHOffset = 0;
			m_bUseRemovePadVol = FALSE;
			m_dRemovePadVol = 0;
			m_bVisualizeBridge = FALSE;
			m_bUseNoFluxLimit = FALSE;
			m_nNoFluxLimit = 0;
			m_bUseModelMatching = FALSE;
			m_bUseModel = FALSE;
			m_bUseRemoveLargeBlob = FALSE;
		}
	}_AlgoBGA;

	typedef struct _tagRstAlgoBGA	// BGA 검사 결과
	{
		BOOL m_bOKCoplanarity;
		BOOL m_bOKGridOffsetX;
		BOOL m_bOKGridOffsetY;
		BOOL m_bOKTwist;
		BOOL m_bOKNoFluxLimit;

		float m_fRstCoplanarity;
		float m_fRstGridOffsetX;
		float m_fRstGridOffsetY;
		float m_fRstTwist;
		int m_nRstNoFluxLimit;
		int m_nTeachNoFluxLimit;

		RECT m_rcRect_I = { 0 };
		RECT m_rcMinCoplanarity;
		RECT m_rcMaxCoplanarity;

		float m_fArrRstCoplan[_eMMA_Total];
		float m_fArrRstHeight[_eMMA_Total];
		float m_fArrRstWidth[_eMMA_Total];
		float m_fArrRstVolume[_eMMA_Total];
		float m_fArrRstWidth2[_eMMA_Total];
		float m_fArrRstLength[_eMMA_Total];

		BOOL m_bOKDistance;
		BOOL m_bOKDistance2;
		float m_fRstDistance;
		float m_fRstDistance2;
		POINTF m_LineS1Point[2];
		POINTF m_LineS2Point[2];
		POINTF m_CenterPoint[2];
		POINTF m_PODCenter[2];
		_tagRstAlgoBGA()
		{
			m_bOKCoplanarity = FALSE;
			m_bOKGridOffsetX = FALSE;
			m_bOKGridOffsetY = FALSE;
			m_bOKTwist = FALSE;
			m_bOKNoFluxLimit = FALSE;

			m_fRstCoplanarity = 0;
			m_fRstGridOffsetX = 0;
			m_fRstGridOffsetY = 0;
			m_fRstTwist = 0;
			m_nRstNoFluxLimit = 0;
			m_nTeachNoFluxLimit = 0;
			memset(&m_rcMinCoplanarity, 0, sizeof(RECT));
			memset(&m_rcMaxCoplanarity, 0, sizeof(RECT));

			memset(m_fArrRstCoplan, 0, sizeof(float) * _eMMA_Total);
			memset(m_fArrRstHeight, 0, sizeof(float) * _eMMA_Total);
			memset(m_fArrRstWidth, 0, sizeof(float) * _eMMA_Total);
			memset(m_fArrRstVolume, 0, sizeof(float) * _eMMA_Total);
			memset(m_fArrRstWidth2, 0, sizeof(float) * _eMMA_Total);
			memset(m_fArrRstLength, 0, sizeof(float) * _eMMA_Total);
			m_bOKDistance = FALSE;
			m_bOKDistance2 = FALSE;
			m_fRstDistance = 0;
			m_fRstDistance2 = 0;
			memset(m_LineS1Point, 0, 2 * sizeof(POINTF));
			memset(m_LineS2Point, 0, 2 * sizeof(POINTF));
			memset(m_CenterPoint, 0, 2 * sizeof(POINTF));
			memset(m_PODCenter, 0, 2 * sizeof(POINTF));
		}
	}_RstAlgoBGA;

	enum _m_eSBump_Inspection
	{
		_eSBump_Insp_Diameter = 0x01,
		_eSBump_Insp_Width = 0x02,
		_eSBump_Insp_Length = 0x04,
		_eSBump_Insp_Height = 0x08,

		_eSBump_Insp_Area = 0x10,
		_eSBump_Insp_Volumn = 0x20,
		_eSBump_Insp_ShiftX = 0x30,
		_eSBump_Insp_ShiftY = 0x40,

		_eSBump_Insp_ShiftR = 0x100,
		_eSBump_Insp_Circle = 0x200,
		_eSBump_Insp_Missing = 0x300,
	};

	typedef struct _stLQBGA_Grid
	{

		int nCondition;
		//Grid 별 검사 조건
		float fDiameter_spec;
		float fWidth_spec;
		float fLength_spec;
		float fHeight_spec;
		float fArea_spec;
		float fVolumn_spec;

		float fDiameter_min;
		float fDiameter_max;

		float fWidth_min;
		float fWidth_max;

		float fLength_min;
		float fLength_max;

		float fHeight_min;
		float fHeight_max;

		float fArea_min;
		float fArea_max;

		float fVolumn_min;
		float fVolumn_max;

		int HeightAreaType;

		bool bSelfAlign;

		_stLQBGA_Grid()
		{
			nCondition = 0;
			fDiameter_spec =0.f;
			fWidth_spec = 0.f;
			fLength_spec = 0.f;
			fHeight_spec = 0.f;
			fArea_spec = 0.f;
			fVolumn_spec =0.f;

			fDiameter_min = 0.f;
			fDiameter_max = 0.f;

			fWidth_min = 0.f;
			fWidth_max = 0.f;

			fLength_min = 0.f;
			fLength_max = 0.f;

			fHeight_min = 0.f;
			fHeight_max = 0.f;

			fArea_min = 0.f;
			fArea_max = 0.f;

			fVolumn_min =0.f;
			fVolumn_max =0.f;

			HeightAreaType = 0;

			bSelfAlign = false;
		}
		
	}_LQBGA_Grid;

	typedef struct _stLQBump
	{
		float fTeach_CenterX;
		float fTeach_CenterY;
		int nPadType;
		int nID;

		_stLQBump()
		{
			fTeach_CenterX = 0;
			fTeach_CenterY = 0;
			nPadType = 0;
			nID = 0;
		}

	}_LQBump;

	typedef struct _strstLQBump
	{
		int nID;
		int nResult;  //0이면 OK

		float fTeachPosX_Pixel;
		float fTeachPosY_Pixel;
		float fRealPosX_Pixel;
		float fRealPosY_Pixel;

		float fRstDiameter;
		float fRstWidth;
		float fRstLength;
		float fRstHeight;

		float fRstArea;
		float fRstVolumn;

		float fRstShitX;
		float fRstShitY;
		float fRstShitR;

		float fRstCopl;
		float fRstHeight_Bottom;
		float fRstHeight_Absolute;

		void Missing()
		{
			nResult = _eSBump_Insp_Missing;
		}

		bool IsMissing()
		{
			if (nResult & _eSBump_Insp_Missing)
				return true;
			
			return false;
		}

		_strstLQBump()
		{
			nID = 0;
			nResult = 0;  //0이면 OK
			fTeachPosX_Pixel = 0.f; //Pixel;
			fTeachPosY_Pixel = 0.f;
			fRealPosX_Pixel = 0.f;
			fRealPosY_Pixel = 0.f;
			fRstDiameter = 0.f;
			fRstWidth = 0.f;
			fRstLength = 0.f;
			fRstHeight = 0.f;
			fRstCopl = 0.f;

			fRstArea =0.f;
			fRstVolumn=0.f;

			fRstShitX = 0.f;
			fRstShitY = 0.f;
			fRstShitR = 0.f;
			fRstHeight_Bottom = 0.f;
			fRstHeight_Absolute = 0.f;
		}

	}_rstLQBump; //조건 전부 um

	typedef struct _stAlgoLQBGA
	{
		// 이진화
		 _Blob m_Blob;
		struct _tagAlgoColorBase m_sAlgoColorBase;

		// BGA 검사 조건 옵션	
		int m_InspAreaType;//0: Bump, 1: Flat, 2: blob, 3:pad, 4:tiny
		int m_SelectOffSetType; //Align
		int m_nCoplOption;//Copl

		// BGA 검사 조건
		BOOL m_bUseCoplanarity;
		double m_CoplanarMax;
		double m_CoplanarMin;

		// Bump 검사 조건
		_LQBGA_Grid grid;

		int nNumBump;
		_LQBump* pBumps;
		_stAlgoLQBGA()
		{
			pBumps = nullptr;
			nNumBump = 0;

			m_InspAreaType = 0;
			m_SelectOffSetType = 0;
			m_nCoplOption = 0;
			m_bUseCoplanarity = FALSE;
			m_CoplanarMax =0.;
			m_CoplanarMin =0.;
		}

		void Delete()
		{
			if (pBumps)
				delete[] pBumps;
		}

	}_AlgoLQBGA;

	typedef struct _stRstAlgoBGA//결과는 전부 미크론 um
	{
		BOOL bResult; // BGA, Bump 전체 결과

		BOOL bResult_Bump;
		BOOL bOKCoplanarity;

		// BGA 검사 결과
		float m_fRstCoplanarity; //Copl

		POINTF MaxCoplArea;
		POINTF MInCoplArea;

		float fHeight_AVG;
		float fHeight_Min;
		float fHeight_Max;
		float fHeight_STD;

		float fCopl_AVG;
		float fCopl_Min;
		float fCopl_Max;
		float fCopl_STD;
		float fCopl_CPK;

		float fDiameter_AVG;
		float fDiameter_MIn;
		float fDiameter_Max;
		float fDiameter_STD;

		//Bump 검사 결과
		int nNumBump;
		_rstLQBump* RstBumps;
		_stRstAlgoBGA()
		{
			RstBumps = nullptr;
			nNumBump = 0;
			bResult = FALSE;
			bResult_Bump = FALSE;
			bOKCoplanarity = FALSE;

			MaxCoplArea.x = 0;
			MaxCoplArea.y = 0;

			MInCoplArea.x = 0;
			MInCoplArea.y = 0;

			fHeight_AVG = 0;
			fHeight_Min = 0;
			fHeight_Max = 0;
			fHeight_STD = 0;

			fCopl_AVG=0;
			fCopl_Min=0;
			fCopl_Max=0;
			fCopl_STD=0;
			fCopl_CPK=0;

			fDiameter_AVG=0;
			fDiameter_MIn=0;
			fDiameter_Max=0;
			fDiameter_STD=0;

			m_fRstCoplanarity = 0.f;
		}

		void DeleteRsults()
		{
			if (RstBumps == nullptr)
				return;

			delete[] RstBumps;
			RstBumps = nullptr;
		}

	}_RstAlgoLQBGA;


	typedef struct tagTeachQFN
	{
		//수정필요 20220303_NYJ
		int m_numUnit;
		struct tagTeachUnit* pUnit;

		tagTeachQFN()
		{
			m_numUnit = 0;
			pUnit = nullptr;
		}

	}TeachQFN;

	typedef struct tagTeachUnit
	{
		double m_cx;
		double m_cy;

		double m_width;
		double m_Length;

		double m_Area;
		RECT m_rcRect_I;

		double m_heightMean;
		double m_Rectangularity;

		tagTeachUnit()
		{
			m_cx = 0.;
			m_cy = 0.;
			
			m_width = 0.;
			m_Length = 0.;
			m_Area = 0.;
			memset(&m_rcRect_I, 0, sizeof(RECT));

			m_heightMean = 0.;
			m_Rectangularity = 0.;
		}
	}TeachUnit;
	
	typedef struct tagTeachPad
	{
		double m_cx;
		double m_cy;

		double m_width;
		double m_Length;

		RECT m_rcRect_I;

		double m_heightMean;

		double m_distanceX; //우측
		double m_distanceY; //아래

		tagTeachPad()
		{
			m_cx = 0;
			m_cy = 0;

			m_width = 0;
			m_Length = 0;
			memset(&m_rcRect_I, 0, sizeof(RECT));

			m_heightMean = 0.;

			m_distanceX = 0.;
			m_distanceY = 0.;
		}
	}TeachPad;
	
	typedef struct tagTeachPadArray
	{
		int m_numPad;
		double m_centerPointX;
		double m_centerPointY;
		double m_width;
		double m_length;
		double m_distanceX;
		double m_distanceY;
		struct tagTeachPad* pPad;

		tagTeachPadArray()
		{
			m_numPad = 0;
			m_centerPointX =0.;
			m_centerPointY = 0.;
			m_width = 0.;
			m_length = 0.;
			m_distanceX = 0.;
			m_distanceY = 0.;
			pPad = nullptr;
		}
	}TeachPadArray;

	typedef struct _stAlgoQFN
	{
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

		int m_nTypeSelectBlob;
		BOOL m_bFilterIsUse;
		BOOL m_bFillHole;

		BOOL m_bTeachWidthUse;
		double m_dTeachWidth;
		double m_dTeachWidthRateMin;
		double m_dTeachWidthRateMax;

		BOOL m_bTeachLengthUse;
		double m_dTeachLength;
		double m_dTeachLengthRateMin;
		double m_dTeachLengthRateMax;

		BOOL m_bShiftIsUse;
		BOOL m_bShiftXUse;
		BOOL m_bShiftYUse;
		double m_dShiftX;
		double m_dShiftY;

		BOOL m_bAreaIsUse;
		double m_dAreaMin;
		double m_dAreaMax;
		double m_dAreaCurrent;				//PercentOK

		BOOL m_bUsePitch;
		double m_dTolPitch;

		BOOL m_bUseContrast;
		double m_dTolContrast;

		BOOL m_bUseHeight;
		double m_dTeachHeight;
		double m_dTeachHeightMax;
		double m_dTeachHeightMin;

		BOOL m_bUseCoplanarity;
		double m_dCoplanarMax;

		BOOL m_bUseDistanceToEdge;
		double m_dDistanceToEdge;

		BOOL m_bUsePadSpacing;
		double m_dPadSpacing;

		_stAlgoQFN()
		{
			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_nTypeRange3D = _eTypeRangeIn;

			m_nTypeSelectBlob = _eSelectCenter;
			m_bFilterIsUse = FALSE;
			m_bFillHole = FALSE;

			m_bTeachWidthUse = FALSE;
			m_dTeachWidth = 0.0;
			m_dTeachWidthRateMin = 80.;
			m_dTeachWidthRateMax = 120.;
			m_bTeachLengthUse = FALSE;
			m_dTeachLength = 0.0;
			m_dTeachLengthRateMin = 80.;
			m_dTeachLengthRateMax = 120.;

			m_bShiftIsUse = FALSE;
			m_bShiftXUse = TRUE;
			m_bShiftYUse = TRUE;

			m_dAreaMin = 80.;
			m_dAreaMax = 120.;

			m_bUsePitch = FALSE;
			m_dTolPitch = 0.;

			m_bUseContrast = FALSE;
			m_dTolContrast = 0;

			m_bUseHeight = FALSE;
			m_dTeachHeight = 0.;
			m_dTeachHeightMax = 0.;
			m_dTeachHeightMin = 0.;

			m_bUseCoplanarity = FALSE;
			m_dCoplanarMax = 0.0f;

			m_bUseDistanceToEdge = FALSE;
			m_dDistanceToEdge = 0.0f;

			m_bUsePadSpacing = FALSE;
			m_dPadSpacing = 0.0f;

			m_dAreaCurrent = 0.;
			m_dShiftX = 0.;
			m_dShiftY = 0.;

			m_bAreaIsUse = FALSE;
			
		}
	}_AlgoQFN;

	typedef struct _tagRstAlgoQFN	// QFN 검사 결과
	{
		BOOL m_bOKCoplanarity;
		
		float m_fRstCoplanarity;

		RECT m_rcRect_I;
		RECT m_rcMinCoplanarity;
		RECT m_rcMaxCoplanarity;

		_tagRstAlgoQFN()
		{
			m_bOKCoplanarity = FALSE;
			
			m_fRstCoplanarity = 0;
			
			memset(&m_rcRect_I, 0, sizeof(RECT));
			memset(&m_rcMinCoplanarity, 0, sizeof(RECT));
			memset(&m_rcMaxCoplanarity, 0, sizeof(RECT));
		}
	}_RstAlgoQFN;

	typedef struct _stAlgoNGBlob
	{
		byte narrdata[_NGBlob_by_Total];
		float farrdata[_NGBlob_F_Total];
		int idata;
		int idata2;
		RECT m_rcHeightInRoi[_TiltRectCnt]; // tilt roi 구조로 변경 해야함.
		//struct tagAlgoColorBase m_sAlgoColorBase; // 이건 차후에 하기로함.
		struct _tagAlgoColorBase m_sAlgoColorBase;

		int m_nCntPatternPath;
		wchar_t m_sArrPathExceptModel1[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel2[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel3[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel4[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel5[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel6[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel7[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel8[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel9[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel10[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel11[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel12[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel13[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel14[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel15[MAX_STRLEN];
		wchar_t m_sArrPathExceptModel16[MAX_STRLEN];
		wchar_t m_sPathModelTeach[_MAX_STRLEN];
		wchar_t m_sTimeMoPartWndAlgoInfo[_MAX_STRLEN];
		_stAlgoNGBlob()
		{
			memset(narrdata, 0, _NGBlob_by_Total * sizeof(byte));
			memset(farrdata, 0, _NGBlob_F_Total * sizeof(float));
			memset(m_rcHeightInRoi, 0, _TiltRectCnt * sizeof(RECT));
			m_nCntPatternPath = 0;
			memset(m_sArrPathExceptModel1, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel2, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel3, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel4, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel5, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel6, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel7, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel8, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel9, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel10, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel11, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel12, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel13, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel14, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel15, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sArrPathExceptModel16, 0, MAX_STRLEN * sizeof(wchar_t));

			memset(m_sPathModelTeach, 0, MAX_STRLEN * sizeof(wchar_t));
			memset(m_sTimeMoPartWndAlgoInfo, 0, MAX_STRLEN * sizeof(wchar_t));

			idata = 0;
			idata2 = 0;
		}
		wchar_t* GetModelPath(int nModelIdx)
		{
			wchar_t* str = nullptr;
			switch (nModelIdx)
			{
			case 0:  str = m_sArrPathExceptModel1; break;
			case 1:  str = m_sArrPathExceptModel2; break;
			case 2:  str = m_sArrPathExceptModel3; break;
			case 3:  str = m_sArrPathExceptModel4; break;
			case 4:  str = m_sArrPathExceptModel5; break;
			case 5:  str = m_sArrPathExceptModel6; break;
			case 6:  str = m_sArrPathExceptModel7; break;
			case 7:  str = m_sArrPathExceptModel8; break;
			case 8:  str = m_sArrPathExceptModel9; break;
			case 9:  str = m_sArrPathExceptModel10; break;
			case 10: str = m_sArrPathExceptModel11; break;
			case 11: str = m_sArrPathExceptModel12; break;
			case 12: str = m_sArrPathExceptModel13; break;
			case 13: str = m_sArrPathExceptModel14; break;
			case 14: str = m_sArrPathExceptModel15; break;
			case 15: str = m_sArrPathExceptModel16; break;

			}
			return str;
		}
		void SetModelPath(wchar_t* str, int nModelIdx)
		{
			if (str == NULL)
				return;
			switch (nModelIdx)
			{
			case 0:	memcpy(m_sArrPathExceptModel1, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 1: memcpy(m_sArrPathExceptModel2, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 2: memcpy(m_sArrPathExceptModel3, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 3: memcpy(m_sArrPathExceptModel4, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 4: memcpy(m_sArrPathExceptModel5, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 5: memcpy(m_sArrPathExceptModel6, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 6: memcpy(m_sArrPathExceptModel7, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 7: memcpy(m_sArrPathExceptModel8, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 8: memcpy(m_sArrPathExceptModel9, str, sizeof(wchar_t)*MAX_STRLEN); break;
			case 9: memcpy(m_sArrPathExceptModel10, str, sizeof(wchar_t)*MAX_STRLEN);  break;
			case 10: memcpy(m_sArrPathExceptModel11, str, sizeof(wchar_t)*MAX_STRLEN);  break;
			case 11: memcpy(m_sArrPathExceptModel12, str, sizeof(wchar_t)*MAX_STRLEN);  break;
			case 12: memcpy(m_sArrPathExceptModel13, str, sizeof(wchar_t)*MAX_STRLEN);  break;
			case 13: memcpy(m_sArrPathExceptModel14, str, sizeof(wchar_t)*MAX_STRLEN);  break;
			case 14: memcpy(m_sArrPathExceptModel15, str, sizeof(wchar_t)*MAX_STRLEN);  break;
			case 15: memcpy(m_sArrPathExceptModel16, str, sizeof(wchar_t)*MAX_STRLEN);  break;

			}
		}
	}_AlgoNGBlob;
	typedef struct _tagRstAlgoNGBlob // Edge 검사 결과
	{
		BOOL bIsOKArea;      // Area,Coining 검사결과
		BOOL bIsOKWidth;    // Width/단축 검사결과
		BOOL bIsOKLength;    // Length/장축 검사결과
		BOOL bIsOKExtraPer;    // Length/장축 검사결과
		BOOL bIsOKExtraWidth;    // Length/장축 검사결과
		BOOL bIsOKHeight;    // FM 검사결과
		BOOL bIsOKScratch;    // Scratch 검사결과
		BOOL bIsOKWarpage;    // Warpage 검사결과
		BOOL bIsOKWarpageX;
		BOOL bIsOKWarpageY;
		BOOL bIsOKChipping;
		float fRstWidth[_BLOB_RECT_CNTS];		// Width/단축 검사결과괎
		float fRstLength[_BLOB_RECT_CNTS];		// Length/장축 검사결과괎
		float fRstArea[_BLOB_RECT_CNTS];			// Area/Coining 검사결과괎
		float fRstExtraPer[_BLOB_RECT_CNTS];   //  default에서는 Contrast
		float fRstExtraWidth[_BLOB_RECT_CNTS];
		float fRstHeight[_BLOB_RECT_CNTS];
		float fRstScDistance[_BLOB_RECT_CNTS];
		float fRstScWidth[_BLOB_RECT_CNTS];
		float fRstChippingWid[_BLOB_RECT_CNTS];
		float fRstChippingLen[_BLOB_RECT_CNTS];
		int blob_count;
		RECT m_rcArrRect[_BLOB_RECT_CNTS];
		float fRstWarpagedeviation;
		float fRstWarpagedev_X;
		float fRstWarpagedev_Y;
		RECT m_BodyRect;
		int OutlineChipping[8];
		int ChippingNGDir;
		BOOL bIsOKContrast;
		BOOL bIsOKColorContrast;
		BOOL bIsOKWarningCount[_BLOB_RECT_CNTS];
		_tagRstAlgoNGBlob()
		{
			bIsOKArea = FALSE;
			bIsOKWidth = FALSE;
			bIsOKLength = FALSE;
			bIsOKExtraPer = FALSE;
			bIsOKExtraWidth = FALSE;
			bIsOKHeight = FALSE;
			bIsOKScratch = FALSE;
			bIsOKWarpage = FALSE;
			bIsOKWarpageX = FALSE;
			bIsOKWarpageY = FALSE;
			bIsOKChipping = FALSE;
			memset(fRstWidth, 0, _BLOB_RECT_CNTS * sizeof(float));  //InspType 에따라서 다른값임
			memset(fRstLength, 0, _BLOB_RECT_CNTS * sizeof(float));  //InspType 에따라서 다른값임
			memset(fRstArea, 0, _BLOB_RECT_CNTS * sizeof(float));  //InspType 에따라서 다른값임
			blob_count = 0;
			memset(fRstExtraPer, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(fRstExtraWidth, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(fRstHeight, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(fRstScDistance, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(fRstScWidth, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(fRstChippingWid, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(fRstChippingLen, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(m_rcArrRect, 0, _BLOB_RECT_CNTS * sizeof(RECT));
			m_BodyRect = RECT();
			fRstWarpagedeviation = 0;
			fRstWarpagedev_X = 0;
			fRstWarpagedev_Y = 0;
			memset(OutlineChipping, 0, 8 * sizeof(int));
			ChippingNGDir = 0;
			bIsOKContrast = FALSE;
			bIsOKColorContrast = FALSE;
			memset(bIsOKWarningCount, FALSE, _BLOB_RECT_CNTS * sizeof(BOOL));
		}
	}_RstAlgoNGBlob;

	typedef struct _stAlgoPadArray
	{
		byte narrdata[_PadArray_by_Total];
		float farrdata[_PadArray_F_Total];
		int idata;

		RECT rcFirstROI[_MAX_PAD_CNT];
		RECT rcSeccondROI[_MAX_PAD_CNT];

		_stAlgoPadArray()
		{
			memset(narrdata, 0, _PadArray_by_Total * sizeof(byte));
			memset(farrdata, 0, _PadArray_F_Total * sizeof(float));
			memset(rcFirstROI, 0, _MAX_PAD_CNT * sizeof(RECT));
			memset(rcSeccondROI, 0, _MAX_PAD_CNT * sizeof(RECT));
			idata = 0;
		}
	}_AlgoPadArray;
	
	typedef struct _tagRstAlgoPadArray
	{
		float dRstWidth[_MAX_PAD_CNT];
		float dRstLength[_MAX_PAD_CNT];
		float dRstDistanceX[_MAX_PAD_CNT];
		float dRstDistanceY[_MAX_PAD_CNT];

		float dRstShiftX;
		float dRstShiftY;
		float dRstCenterPointX;
		float dRstCenterPointY;

		int m_nArrRectCnt;

		// OK 판정 //
		BOOL m_bOKWidth[_MAX_PAD_CNT];
		BOOL m_bOKLength[_MAX_PAD_CNT];
		BOOL m_bOKDistanceX[_MAX_PAD_CNT];
		BOOL m_bOKDistanceY[_MAX_PAD_CNT];
		BOOL m_bOKHeightDiff[_MAX_PAD_CNT];
		BOOL m_bOKShiftX;
		BOOL m_bOKShiftY;

		RECT m_rcArrRect[_MAX_PAD_CNT];
		POINTF m_poDrawCenter;

		//height diff
		RECT m_rcHeightROI1[_MAX_PAD_CNT] = { 0 };
		RECT m_rcHeightROI2[_MAX_PAD_CNT] = { 0 };
		float m_heightMean1[_MAX_PAD_CNT] = { 0 };
		float m_heightMean2[_MAX_PAD_CNT] = { 0 };

		_tagRstAlgoPadArray()
		{
			memset(dRstWidth, 0, _MAX_PAD_CNT * sizeof(float));
			memset(dRstLength, 0, _MAX_PAD_CNT * sizeof(float));
			memset(dRstDistanceX, 0, _MAX_PAD_CNT * sizeof(float));
			memset(dRstDistanceY, 0, _MAX_PAD_CNT * sizeof(float));

			dRstShiftX = 0;
			dRstShiftY = 0;
			dRstCenterPointX = 0;
			dRstCenterPointY = 0;

			memset(m_bOKWidth, TRUE, _MAX_PAD_CNT * sizeof(BOOL));
			memset(m_bOKLength, TRUE, _MAX_PAD_CNT * sizeof(BOOL));
			memset(m_bOKDistanceX, TRUE, _MAX_PAD_CNT * sizeof(BOOL));
			memset(m_bOKDistanceY, TRUE, _MAX_PAD_CNT * sizeof(BOOL));
			memset(m_bOKHeightDiff, TRUE, _MAX_PAD_CNT * sizeof(BOOL));

			m_bOKShiftX = TRUE;
			m_bOKShiftY = TRUE;

			m_nArrRectCnt = 0;
			memset(m_rcArrRect, 0, _MAX_PAD_CNT * sizeof(RECT));
			m_poDrawCenter.x = 0;
			m_poDrawCenter.y = 0;
		}
	}_RstAlgoPadArray;

	enum m_eAlgoTilt_Data
	{
		m_eHighest = 0x01,
	};

	typedef struct _stAlgoTilt
	{
		double dAllowDiff;
		int nSelectValue;
		RECT rcArrSelectROI[_TiltRectCnt] = { 0 };
		RECT rcArrDiffROI[_TiltRectCnt] = { 0 };
		BOOL m_bUseAreaDiff;
		double m_dHeightDiffMin;
		double m_dHeightDiffMax;
		double m_dPtPDistanceMin;
		double m_dPtPDistanceMax;
		BOOL m_bUsePart;
		double m_dHeightAvg;
		double m_dHeightMin;
		double m_dHeightMax;
		BOOL m_bUseAngle;
		int m_nInspAreaType; // 영역의 높이 Mean, Max 선택
		double m_dZAngle;
		double m_dAngleMaxDiff;
		RECT rcAngleStdROI[_nTiltAngleCnt] = { 0 };
		int nSelectAngleValue;
		double m_dAngleBinMin;
		double m_dAngleBinMax;
		bool m_bUseColorRange;
		float m_fColorRangeMax;
		float m_fColorRangeMin;
		BOOL m_bCheckedDiffOption;

		_stAlgoTilt()
		{
			dAllowDiff = 0.0;
			nSelectValue = 2;
			m_bUseAreaDiff = true;
			m_dHeightDiffMin = 0;
			m_dHeightDiffMax = 0;
			m_dPtPDistanceMin = 0;
			m_dPtPDistanceMax = 0;
			m_bUsePart = false;
			m_dHeightAvg = 0;
			m_dHeightMin = 80;
			m_dHeightMax = 120;
			m_bUseAngle = false;
			m_nInspAreaType = 0;
			m_dZAngle = 0.0;
			m_dAngleMaxDiff = 10;
			nSelectAngleValue = 0;
			m_dHeightDiffMin = -5000;
			m_dAngleBinMax = 5000;
			m_bUseColorRange = false;
			m_fColorRangeMax = 0;
			m_fColorRangeMin = 0;
			m_bCheckedDiffOption = false;
			m_dAngleBinMin = 0.;
		}
	}_AlgoTilt;

	typedef struct _tagRstAlgoTilt	// Tilt 검사 결과
	{
		double m_dRstHeightDiff;

		double m_dArrRectHeight[_TiltRectCnt];
		double m_dArrDiffRectHeight[_TiltRectCnt];
		double m_dArrDiffResultHeight[_TiltRectCnt];
		double m_dHeightDiffMin;
		double m_dHeightDiffMax;
		double m_dResultDistance;
		double m_dPtPDistance[_TiltRectCnt];
		double m_dPtPDistanceMin;
		double m_dPtPDistanceMax;
		BOOL m_bArrOKHeightAvg[_TiltRectCnt];

		BOOL m_bOKArea;
		BOOL m_bOKDiff;
		BOOL m_bOKDistance;
		BOOL m_bOKAvg;
		BOOL m_bAngle;
		RECT m_rcRect_I[_TiltRectCnt] = { 0 };
		BOOL m_bArrAngleOK[_TiltRectCnt] = { 0 };
		double m_dArrAngle[_TiltRectCnt];
		double m_dRstStdHeightMax;
		double m_dRstAngleMax;
		double m_dAngleX[_nTiltAngleCnt];
		double m_dAngleY[_nTiltAngleCnt];
		double m_dAngleZ[_nTiltAngleCnt];
		BOOL m_bCheckedDiffOption;
		_tagRstAlgoTilt()
		{
			m_dRstHeightDiff = 0.;

			for (int n = 0; n < _TiltRectCnt; n++)
			{
				m_dArrRectHeight[n] = 0.0;
				m_dArrDiffRectHeight[n] = 0.0;
				m_dArrDiffResultHeight[n] = 0.0;
				m_dArrAngle[n] = 0.0;
				m_bArrAngleOK[n] = TRUE;
				m_dPtPDistance[n] = 0.0;
			}
			m_dHeightDiffMin = 0.0;
			m_dHeightDiffMax = 0.0;
			m_dResultDistance = 0.0;
			m_dPtPDistanceMin = 0.0;
			m_dPtPDistanceMax = 0.0;

			m_bOKArea = FALSE;
			m_bOKDiff = FALSE;
			m_bOKDistance = FALSE;
			m_bOKAvg = FALSE;
			m_bAngle = FALSE;
			m_bCheckedDiffOption = FALSE;

			m_dRstStdHeightMax = 0.0;
			m_dRstAngleMax = 0.0;
			m_dAngleX[0] = 0;
			m_dAngleY[0] = 0;
			m_dAngleZ[0] = 0;
			m_dAngleX[1] = 0;
			m_dAngleY[1] = 0;
			m_dAngleZ[1] = 0;
		}
	}_RstAlgoTilt;

	// Align Window		NYJ 2020/11/10
	// Align Window 검사 결과 구조체
	typedef struct _tagAlignResult
	{
		int nWindowID;
		double centerX;
		double centerY;
		double offsetX;
		double offsetY;
		double theta;
		double TeachCenterX;
		double TeachCenterY;

		CRect rcBodyRect;

		_tagAlignResult()
		{
			Init();
		}

		void Init()
		{
			nWindowID = -1;
			centerX = 0;
			centerY = 0;
			offsetX = 0.;
			offsetY = 0.;
			theta = 0.;

			rcBodyRect = CRect(0, 0, 0, 0);

			TeachCenterX = 0.;
			TeachCenterY = 0.;
		}

		struct _tagAlignResult& operator=(const struct _tagAlignResult& result)
		{
			nWindowID = result.nWindowID;
			centerX = result.centerX;
			centerY = result.centerY;
			offsetX = result.offsetX;
			offsetY = result.offsetY;
			theta = result.theta;

			rcBodyRect = result.rcBodyRect;

			return *this;
		}
	}_AlignResult;

	typedef struct _tagAlgoColor
	{
		double m_dWndAngle;
		int m_nInspTypeColor;						// 0 : color, 1 : Gray
		int m_nViewColor;							//colorXY view

		POINT m_ptArrPolygon[_COLORALGO_POLYGON_CNTS][_POLYGON_POINT_CNTS] = { 0 };
		int m_nPolygonCnt;
		int m_nCntPoint;
		BOOL m_bUsePolarity;

		float m_fFatorRed;
		float m_fFatorGreen;
		float m_fFatorBlue;
		float m_fRebFactorBtm;
		float m_fBlueFactorBtm;

		int m_nSizeXCIE;
		int m_nSizeYCIE;

		//histo add
		int m_nRangeMode;							// 0: greater, 1:less, 2:in range, 3:out range
		int m_nRangeMax;
		int m_nRangeMin;

		//Inspect
		int m_nCntHistoStd;
		double m_dRateStd;							//allow data
		//RstInspColor	retInspColorResult;			//return

		//Teach Result
		int m_nCntHistoResult;
		double m_dRateResult;

		// copa 용
		int m_nLeadTipDirection;		// Lead 방향
		int m_nLeadPosition;			// Lead 위치
		int m_nSolderLength;			// leadtip 으로 부터 solder까지의 거리

		int m_nTypeTab;
		int m_nCntRect;
		RECT m_rcArrTabRect[_g_nBridgeCnt] = { 0 };
		BOOL m_bAutoSearchROI;
		BOOL m_bUseColorMap2;
		BOOL m_bUseRangeBar;
		BOOL m_bUseRGB[_COLORALGO_POLYGON_CNTS];
		byte m_byRange[_COLORALGO_POLYGON_CNTS];
		byte m_byMin[_COLORALGO_POLYGON_CNTS];
		byte m_byMax[_COLORALGO_POLYGON_CNTS];
		BOOL m_bInvert;
		byte m_byColorLightType;
		enum ETypeGep
		{
			_eTab,
			_eTabSpace,
			_eColor
		};

		_tagAlgoColor()
		{
			m_dWndAngle = 0.0;
			m_nInspTypeColor = 0;

			m_nCntPoint = 0;
			m_bUsePolarity = FALSE;

			m_fFatorRed = 0.0;
			m_fFatorGreen = 0.0;
			m_fFatorBlue = 0.0;

			m_nSizeXCIE = 0;
			m_nSizeYCIE = 0;

			m_nRangeMode = 0; // 0: greater, 1:less, 2:in range, 3:out range
			m_nRangeMax = 0;
			m_nRangeMin = 0;
			m_nCntHistoStd = 0;

			m_dRateStd = 0.0;

			m_nSolderLength = 10;
			m_nLeadTipDirection = 0;
			m_nLeadPosition = 0;

			m_nTypeTab = _eColor;
			m_nCntRect = 0;
			m_bAutoSearchROI = FALSE;
			m_bUseColorMap2 = FALSE;
			m_bUseRangeBar = FALSE;
			m_bInvert = FALSE;
			m_byColorLightType = 0;
			for (int n = 0; n < _COLORALGO_POLYGON_CNTS; n++)
			{
				m_bUseRGB[n] = FALSE;
				m_byRange[n] = 0;
				m_byMin[n] = 0;
				m_byMax[n] = 0;
			}

			m_dRateResult = 0;

			m_fBlueFactorBtm = 0.f;
			m_fRebFactorBtm = 0.f;
			m_nCntHistoResult = 0;
			m_nPolygonCnt = 0;
			m_nViewColor = 0;
			
		}

		//void GetParam(InspParamColor *pParamInsp)
		//{
		//	if (!pParamInsp)
		//		return;

		//	pParamInsp->wndAngle = m_dWndAngle;
		//	pParamInsp->colorInspType = m_nInspTypeColor;
		//	pParamInsp->m_nPolygonCnt = m_nPolygonCnt;
		//	memcpy(pParamInsp->polygonPt, m_ptArrPolygon, sizeof(POINT) * _COLORALGO_POLYGON_CNTS * _POLYGON_POINT_CNTS);
		//	pParamInsp->pointCnts = m_nCntPoint;
		//	pParamInsp->isPolarity = m_bUsePolarity;
		//	pParamInsp->redFator = m_fFatorRed;
		//	pParamInsp->greenFator = m_fFatorGreen;
		//	pParamInsp->blueFator = m_fFatorBlue;
		//	pParamInsp->redFactorBtm = m_fRebFactorBtm;
		//	pParamInsp->blueFactorBtm = m_fBlueFactorBtm;
		//	pParamInsp->sizeX_CIE = m_nSizeXCIE;
		//	pParamInsp->sizeY_CIE = m_nSizeYCIE;
		//	pParamInsp->rangeMode = m_nRangeMode; // 0: greater, 1:less, 2:in range, 3:out range
		//	pParamInsp->maxRange = m_nRangeMax;
		//	pParamInsp->minRange = m_nRangeMin;
		//	pParamInsp->stdHisroCnt = m_nCntHistoStd;
		//	pParamInsp->stdRate = m_dRateStd;
		//	pParamInsp->m_bUseColorMap2 = m_bUseColorMap2;
		//	pParamInsp->m_rcArrROI.RemoveAll();

		//	for (int i = 0; i < m_nCntRect; ++i)
		//	{
		//		CRect rcROI(m_rcArrTabRect[i]);
		//		pParamInsp->m_rcArrROI.Add(rcROI);
		//	}
		//	pParamInsp->m_bUseRangeBar = m_bUseRangeBar;
		//	pParamInsp->m_bInvert = m_bInvert;
		//	for (int n = 0; n < _COLORALGO_POLYGON_CNTS; n++)
		//	{
		//		pParamInsp->m_bUseRGB[n] = m_bUseRGB[n];
		//		pParamInsp->m_byRange[n] = m_byRange[n];
		//		pParamInsp->m_byMin[n] = m_byMin[n];
		//		pParamInsp->m_byMax[n] = m_byMax[n];
		//	}
		//}

		//void GetParam(TeachParamColor *pParamTeach)
		//{
		//	if (!pParamTeach)
		//		return;

		//	pParamTeach->wndAngle = m_dWndAngle;
		//	pParamTeach->colorInspType = m_nInspTypeColor;
		//	memcpy(pParamTeach->polygonPt, m_ptArrPolygon, sizeof(POINT) * _COLORALGO_POLYGON_CNTS * _POLYGON_POINT_CNTS);
		//	pParamTeach->pointCnts = m_nCntPoint;
		//	pParamTeach->redFator = m_fFatorRed;
		//	pParamTeach->greenFator = m_fFatorGreen;
		//	pParamTeach->blueFator = m_fFatorBlue;
		//	pParamTeach->redFactorBtm = m_fRebFactorBtm;
		//	pParamTeach->blueFactorBtm = m_fBlueFactorBtm;
		//	pParamTeach->sizeX_CIE = m_nSizeXCIE;
		//	pParamTeach->sizeY_CIE = m_nSizeYCIE;
		//	pParamTeach->rangeMode = m_nRangeMode;
		//	pParamTeach->minRange = m_nRangeMin;
		//	pParamTeach->maxRange = m_nRangeMax;
		//	pParamTeach->viewColor = m_nViewColor;
		//	pParamTeach->rstRate = m_dRateResult;
		//	pParamTeach->rstHisroCnt = m_nCntHistoResult;
		//	pParamTeach->m_bUseColorMap2 = m_bUseColorMap2;
		//	pParamTeach->m_rcArrROI.RemoveAll();

		//	for (int i = 0; i < m_nCntRect; ++i)
		//	{
		//		CRect rcROI(m_rcArrTabRect[i]);
		//		pParamTeach->m_rcArrROI.Add(rcROI);
		//	}
		//	pParamTeach->m_bUseRangeBar = m_bUseRangeBar;
		//	pParamTeach->m_bInvert = m_bInvert;
		//	for (int n = 0; n < _COLORALGO_POLYGON_CNTS; n++)
		//	{
		//		pParamTeach->m_bUseRGB[n] = m_bUseRGB[n];
		//		pParamTeach->m_byRange[n] = m_byRange[n];
		//		pParamTeach->m_byMin[n] = m_byMin[n];
		//		pParamTeach->m_byMax[n] = m_byMax[n];
		//	}
		//}

		BOOL IsUseMultiArea()
		{
			//if (m_nCntRect <= 0)				// Lead_Color에서 nTypeTab가 eTab으로 인식되는 문제가 있어서 여기서 먼저 체크
			//	return FALSE;
			if ((m_nTypeTab == _eTab)
				|| (m_nTypeTab == _eTabSpace))
				return TRUE;
			if (m_nCntRect > 0)				// nTypeTab먼저 체크하고 하자
				return TRUE;

			return FALSE;
		}
	}_AlgoColor;

	typedef struct _tagRstAlgoColor	// Color 검사 결과
	{
		double m_dRstRate;

		// OK 판정 //
		BOOL m_bOKPolarity;
		BOOL m_bRateOK;
		RECT m_rcRect_I = { 0 };
		_tagRstAlgoColor()
		{
			m_dRstRate = 0.;

			m_bOKPolarity = FALSE;
			m_bRateOK = FALSE;
		}
	}_RstAlgoColor;

	typedef struct _tagAlgoTab : _tagAlgoColor
	{
		_tagAlgoTab() : _tagAlgoColor()
		{
			m_nTypeTab = _eTab;
			m_nCntRect = 0;
		}

	}_AlgoTab;

	typedef struct _tagAlgoBaseBW
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
	}_AlgoBaseBW;

	typedef struct _tagAlgoBlackWhite : _AlgoBaseBW
	{
		BOOL m_bUseTeachingRate = FALSE;
		double m_dAreaCurrent = 0.;				//PercentOK
		double m_dTeachingArea = 0.;				//PercentOK
		int m_nStdOKArea = 0;
		struct _tagAlgoColorBase m_sAlgoColorBase;
		int m_nChipTrackingGap = 0;

	}_AlgoBlackWhite;

	typedef struct _tagAlgoBW : _AlgoBaseBW
	{
		BOOL m_bUseTeachingRate;
		double m_dAreaCurrent;				//PercentOK
		double m_dTeachingArea;				//PercentOK
		int m_nStdOKArea;
		struct _tagAlgoColorBase m_sAlgoColorBase;
		int m_nChipTrackingGap;

		BOOL m_bUseHeightMean;
		byte m_byUseHeightOpt;
		BOOL m_bInspBWArea;
		float m_fHeightMeanMin;
		float m_fHeightMeanMax;
		byte m_byDir;
		byte m_byInspectionArea;
		BOOL m_bUseMaxOK;
		int m_nMaxOKPer;
		struct _tagAngleColorBase m_sAngleColorBase;
	}_AlgoBW;

	typedef struct _tagRstAlgoBW	// BW 검사 결과
	{
		double m_dRstPercent;
		double m_dRstHeightMean;

		// OK 판정 //
		BOOL m_bOKBW;
		BOOL m_bNoSolder;
		BOOL m_bOKHeightMean;
		BOOL m_bOKMax;

		struct _tagRstInspAC m_sInspAC;
		_tagRstAlgoBW()
		{
			m_dRstPercent = 0.;
			m_dRstHeightMean = 0.;
			m_bOKBW = TRUE;
			m_bNoSolder = FALSE;
			m_bOKHeightMean = TRUE;
			m_bOKMax = TRUE;
			m_sInspAC.m_nOKInspAC = 1;
			m_sInspAC.m_nInspAC = 0;
			memset(m_sInspAC.m_fRstInspAC, 0, _InspAC_R_Total * sizeof(float));
		}
	}_RstAlgoBW;
	typedef struct _tagAlgoBlobBase
	{
		int m_nArrValue[_m_enBlobBase_Total];
		float m_fArrValue[_m_efBlobBase_Total];
		struct _tagAlgoColorBase m_sAlgoColorBase;
		_tagAlgoBlobBase()
		{
			memset(m_nArrValue, 0, _m_enBlobBase_Total * sizeof(int));
			memset(m_fArrValue, 0, _m_efBlobBase_Total * sizeof(float));
		}
	}_AlgoBlobBase;

	typedef struct _stAlgoGrayMean
	{
		// 2D
		int m_nGrayRateMin;	// 최소 %
		int m_nGrayRateMax;	// 최대 %
		float m_fGrayAvg;		// 기준 값
		struct _tagAlgoBlobBase m_sBlobBase;

		_stAlgoGrayMean()
		{
			m_nGrayRateMin = 80;
			m_nGrayRateMax = 120;
			m_fGrayAvg = 0.0;
		}
	}_AlgoGrayMean;
	

	typedef struct _tagRstAlgoGrayMean	// Gray Mean 검사 결과
	{
		double m_dRstGrayMean;
		BOOL m_bRstStdChanged;
		int m_nRstStdGrayMin;
		int m_nRstStdGrayMax;
		BOOL m_bUseUV;
		int m_nStdGrayMin; // 티칭한 Min 값
		int m_nStdGrayMax; // 티칭한 Max 값
		_tagRstAlgoGrayMean()
		{
			m_dRstGrayMean = 0.;
			m_bRstStdChanged = false;
			m_nRstStdGrayMin = 0;
			m_nRstStdGrayMax = 0;
			m_nStdGrayMin = 0;
			m_nStdGrayMax = 0;
			m_bUseUV = false;
		}
	}_RstAlgoGrayMean;

	typedef struct _tagAlgoHeightMean : _AlgoBaseBW// : InspAlgo
	{
		BOOL m_bHeightUse;
		double dHeightMin3D;			// 최소 높이 um
		double dHeightMax3D;			// 최대 높이 um
		double dAvgHeight3D;			// 티칭 시 높이 um

		BOOL m_bUseHighest;
		double m_dHighestMin;
		double m_dHighestMax;
		double m_dHighestValue;

		BOOL m_bBWOption;

		BOOL m_bUseHeightMin3D;
		BOOL m_bUseHeightMax3D;

		BOOL m_bUseSolderHighest;
		float m_fAddHeight;
		float m_fAddHighest;
		struct _tagAlgoBlobBase m_sBlobBase;

		BOOL m_bUseCorrectionHeight;
		float m_fCorrectionValue;
		float m_fCorrectionUpper;
		float m_fCorrectionLower;

		_tagAlgoHeightMean() : _AlgoBaseBW()
		{
			m_bHeightUse = TRUE;
			dHeightMin3D = 80;
			dHeightMax3D = 120;
			dAvgHeight3D = 0.0;

			m_bUseHighest = FALSE;
			m_dHighestMin = 80;
			m_dHighestMax = 120;
			m_dHighestValue = 0.0;

			m_bBWOption = FALSE;

			m_bUseHeightMin3D = FALSE;
			m_bUseHeightMax3D = FALSE;

			m_bUseSolderHighest = FALSE;
			m_fAddHeight = 0.0f;
			m_fAddHighest = 0.0f;

			m_bUseCorrectionHeight = FALSE;
			m_fCorrectionValue = 0.0f;
			m_fCorrectionUpper = 0.0f;
			m_fCorrectionLower = 0.0f;
		}
	}_AlgoHeightMean;

	typedef struct _tagRstAlgoHeightMean	// Height Mean 검사 결과
	{
		double m_dRstHeightMean;
		double m_dRstHeighestValue;

		BOOL m_bOKHeightMean;
		BOOL m_bOKHeighest;

		BOOL m_bNoSolder;
		BOOL m_bExcess;

		_tagRstAlgoHeightMean()
		{
			m_dRstHeightMean = 0.;
			m_dRstHeighestValue = 0.;

			m_bOKHeightMean = FALSE;
			m_bOKHeighest = FALSE;

			m_bNoSolder = FALSE;
			m_bExcess = FALSE;
		}
	}_RstAlgoHeightMean;
	typedef struct _tagAlgoGrayDiff// : GrayDiff
	{
		RECT rcFirstROI;
		RECT rcSeccondROI;
		int nGrayDiff;

		BOOL m_bDiffUpper;	// 높을때 검사
		BOOL m_bDiffLower;	// 낮을때 검사

		BOOL m_bPolarity;	// SHW 20150302
		BOOL m_bSignInversion;
		int m_nMinMaxflag;
		_tagAlgoGrayDiff()
		{
			rcFirstROI.bottom = 0;
			rcFirstROI.left = 0;
			rcFirstROI.right = 0;
			rcFirstROI.top = 0;

			rcSeccondROI.bottom = 0;
			rcSeccondROI.left = 0;
			rcSeccondROI.right = 0;
			rcSeccondROI.top = 0;

			nGrayDiff = 0;

			m_bDiffUpper = false;
			m_bDiffLower = false;

			m_bPolarity = FALSE;
			m_bSignInversion = TRUE;
			m_nMinMaxflag = 0;
		}
	}_AlgoGrayDiff;

	typedef struct _tagRstAlgoGrayDiff	// Gray diff 검사 결과
	{
		int m_nRstGrayDiff;

		double m_dRstF;
		double m_dRstS;

		BOOL m_bCheckPolarity;
		RECT m_rcRect_I[2] = { 0 };
		_tagRstAlgoGrayDiff()
		{
			m_nRstGrayDiff = 0;
			m_dRstF = 0;
			m_dRstS = 0;
			m_bCheckPolarity = FALSE;
		}
	}_RstAlgoGrayDiff;

	typedef struct _tagAlgoHeightDiff// : HeightDiff
	{
		RECT rcFirstROI;
		RECT rcSeccondROI;
		double dHeightDiff3D;
		double dHeightDiff3DMax;

		BOOL m_bDiffUpper;	// 높을떄 검사
		BOOL m_bDiffLower;	// 낮을때 검사

		BOOL m_bPolarity;
		BOOL m_bSignInversion;
		int m_nMinMaxflag;
		float m_fAddHeight;
		struct _tagAlgoBlobBase m_sBlobBase;
		struct _tagAlgoBlobBase m_sBlobBase_ROI2;
		BOOL m_bUseErodeFilter;
		int m_nErodeFilter;
		BOOL m_bUseBW_ROI2;
		int m_nSelectRoi;

		_tagAlgoHeightDiff()
		{
			rcFirstROI.bottom = 0;
			rcFirstROI.left = 0;
			rcFirstROI.right = 0;
			rcFirstROI.top = 0;

			rcSeccondROI.bottom = 0;
			rcSeccondROI.left = 0;
			rcSeccondROI.right = 0;
			rcSeccondROI.top = 0;

			dHeightDiff3D = 0.0;
			dHeightDiff3DMax = 0.0;

			m_bDiffUpper = false;
			m_bDiffLower = false;

			m_bPolarity = FALSE;
			m_bSignInversion = TRUE;
			m_nMinMaxflag = 0;
			m_fAddHeight = 0.0f;
			m_bUseErodeFilter = false;
			m_nErodeFilter = 0;

			m_bUseBW_ROI2 = FALSE;
			m_nSelectRoi = -1;
		}
	}_AlgoHeightDiff;

	typedef struct _tagRstAlgoHeightDiff	// Height Diff 검사 결과
	{
		double m_dRst_1;
		double m_dRst_2;
		double m_dRstHeightDiff;

		BOOL m_bCheckPolarity;
		RECT m_rcRect_I[4] = { 0 };

		BOOL m_bOK;
		_tagRstAlgoHeightDiff()
		{
			m_dRst_1 = 0.;
			m_dRst_2 = 0.;
			m_dRstHeightDiff = 0.;
			m_bCheckPolarity = FALSE;
			m_bOK = FALSE;
		}
	}_RstAlgoHeightDiff;

	typedef struct _tagPadBin
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

		BOOL m_bFillHole;
		int m_nHistoGridX;
		int m_nHistoGridY;

		struct _tagAlgoColorBase m_sAlgoColorBase;
		_tagPadBin()
		{
			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_nTypeRange3D = _eTypeRangeIn;

			m_bUseHistogram = false;

			m_byHistoLimitMin = 0;
			m_byHistoLimitMax = 0;
			m_byHistoFreq = 0;
			m_byMaskOpt = 0;
			m_byFilterSize = 0;
			m_bFillHole = false;
			m_nHistoGridX = 1;
			m_nHistoGridY = 1;
		}
	}_PadBin;

	typedef struct _stAlgoPadBin
	{
		_lightData stInspAlgoLightsMix = { 0 };
		_PadBin stBin;				//BW condition
		int nOption;
		_stAlgoPadBin()
		{
			nOption = 0;
		}

	}_stPadBin;

	typedef struct _tagBlobLabelInfo
	{
		int nTotalCnt;
		int nid[_BLOBLABELCNT];
		float fCx[_BLOBLABELCNT];
		float fCy[_BLOBLABELCNT];
		int nStx[_BLOBLABELCNT];
		int nSty[_BLOBLABELCNT];
		int nRoiWidth[_BLOBLABELCNT];
		int nRoiLength[_BLOBLABELCNT];
		int nArea[_BLOBLABELCNT];

		_tagBlobLabelInfo()
		{
			nTotalCnt = 0;
			memset(nid, 0, sizeof(int) * _BLOBLABELCNT);
			memset(fCx, 0, sizeof(float) * _BLOBLABELCNT);
			memset(fCy, 0, sizeof(float) * _BLOBLABELCNT);
			memset(nStx, 0, sizeof(int) * _BLOBLABELCNT);
			memset(nSty, 0, sizeof(int) * _BLOBLABELCNT);
			memset(nRoiWidth, 0, sizeof(int) * _BLOBLABELCNT);
			memset(nRoiLength, 0, sizeof(int) * _BLOBLABELCNT);
			memset(nArea, 0, sizeof(int) * _BLOBLABELCNT);
		}

	}_BlobLabelInfo;

	typedef struct _tagAlgoPadBW
	{
		_PadBin sDefaultPad;

		_stPadBin sArrInspPad[_MAX_CNT_PAD_COMPOSED_LIGHT];		//Composed Light 4
		int m_nTotLightCnt;

		BOOL bTeachAreaUse;
		double dTeachArea;
		double dTeachAreaRateMin;
		double dTeachAreaRateMax;

		BOOL bUseShift;
		double dTeachShiftX;
		double dTeachShiftY;

		//CString strGBMaskName;		//gerber mask ID
		wchar_t strGBMaskName[_MAX_STRLEN] = { 0 };

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
		//BOOL bUseRemoveHVComponent;//수직수평 성분 제거 옵션
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
		_tagBlobLabelInfo stBlobLabelInfo;
		_tagAlgoPadBW()
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

			nFilterLevel = 2;
			nOrder = 0;

			UseOption3DMinMax = FALSE;
			Option3DMin = 0;
			Option3DMax = 0;
			Option3DThickMin = 0;

			UseOption3DRange = FALSE;
			fOption3DRange = 0;
			nSelectBlobType = 0;
			nSelectBlobNum = 0;

			UseOptionRelativeHeight = FALSE;
			fRelativeHeight_Min = 0.0f;
			fRelativeHeight_Max = 0.0f;

			UseShadeFix = FALSE;
			bUseBlobAnd = FALSE;
			bUseContrastGV = FALSE;
			nContrastGV = 10;
			
			bUseNGGrouping = FALSE;
			fNGGroupingMaxSize = 0.0f;
			fNGGroupingDistance = 0.0f;

			buseDIrection = FALSE;
			nDirection = 0;
			dDIrectionLength = 0.0;
			//bUseRemoveHVComponent = FALSE;
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

			m_bUseBlobSizeWidthNCritical = FALSE;
			m_dBlobSizeWidthNCritical = 0.0f;
			m_bUseBlobSizeLengthNCritical = FALSE;
			m_dBlobSizeLengthNCritical = 0.0f;
			m_bUseBlobSizeAreaNCritical = FALSE;
			m_dBlobSizeAreaNCritical = 0.0f;
			memset(&stBlobLabelInfo, 0, sizeof(_tagBlobLabelInfo));
		}

	}_AlgoPadBW;

	typedef struct _tagRstAlgoPadBW
	{
		// Pad default Insp OK decision
		BOOL m_bOKShapeArea;
		BOOL m_bOKShapeShiftX;
		BOOL m_bOKShapeShiftY;

		float m_fArrRstShapeArea[_MAX_SHAPEDEFECT_CNT];
		float m_fArrRstShapeAreaRate[_MAX_SHAPEDEFECT_CNT];
		float m_fArrRstShiftX[_MAX_SHAPEDEFECT_CNT];
		float m_fArrRstShiftY[_MAX_SHAPEDEFECT_CNT];
		RECT m_rcArrShapeRect_I[_MAX_SHAPEDEFECT_CNT];
		BOOL m_bArrShapeOK[_MAX_SHAPEDEFECT_CNT];
		int m_nArrShapeRectCnt;

		float m_fArrRstArea[_MAX_DEFECT_CNT];
		float m_fArrRstWidth[_MAX_DEFECT_CNT];
		float m_fArrRstLength[_MAX_DEFECT_CNT];

		// Pad foreign Insp OK decision
		BOOL m_bOKWidth;
		BOOL m_bOKLength;

		RECT m_rcArrRect_I[_MAX_DEFECT_CNT];
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

		float m_nArrRstHeightMin[_MAX_DEFECT_CNT];
		float m_nArrRstHeightMax[_MAX_DEFECT_CNT];
		double m_dAlignResultTheta;

		float m_fArrRstRelativeHeightMin[_MAX_DEFECT_CNT];
		float m_fArrRstRelativeHeightMax[_MAX_DEFECT_CNT];
		float m_fArrRstContrast[_MAX_DEFECT_CNT];
		_tagRstAlgoPadBW()
		{
			m_bOKShapeArea = FALSE;
			m_bOKShapeShiftX = FALSE;
			m_bOKShapeShiftY = FALSE;

			memset(m_fArrRstShapeArea, 0, _MAX_SHAPEDEFECT_CNT * sizeof(float));
			memset(m_fArrRstShapeAreaRate, 0, _MAX_SHAPEDEFECT_CNT * sizeof(float));
			memset(m_fArrRstShiftX, 0, _MAX_SHAPEDEFECT_CNT * sizeof(float));
			memset(m_fArrRstShiftY, 0, _MAX_SHAPEDEFECT_CNT * sizeof(float));
					 
			m_nArrShapeRectCnt = 0;
			memset(m_rcArrShapeRect_I, 0, _MAX_SHAPEDEFECT_CNT * sizeof(RECT));
			memset(m_bArrShapeOK, FALSE, _MAX_SHAPEDEFECT_CNT * sizeof(BOOL));

			memset(m_fArrRstArea, 0, _MAX_DEFECT_CNT * sizeof(float));
			memset(m_fArrRstWidth, 0, _MAX_DEFECT_CNT * sizeof(float));
			memset(m_fArrRstLength, 0, _MAX_DEFECT_CNT * sizeof(float));

			m_bOKWidth = FALSE;
			m_bOKLength = FALSE;

			m_nArrRectCnt = 0;
			memset(m_rcArrRect_I, 0, _MAX_DEFECT_CNT * sizeof(RECT));

			m_bOKArea = FALSE;
			m_bMaskLoadSuccess = TRUE;

			Option3DRangeOK = TRUE;
			m_dRstOption3DRange = 0.0f;
			m_nArrOption3DRangeIdx = 0;

			Option3DMinMaxOK = TRUE;
			m_fRsOption3DMin = 0.0f;
			m_fRsOption3DMax = 0.0f;
			m_nArrHeightMinMaxIdx = 0;
			memset(m_nArrRstHeightMin, 0, _MAX_DEFECT_CNT * sizeof(float));
			memset(m_nArrRstHeightMax, 0, _MAX_DEFECT_CNT * sizeof(float));
			m_dAlignResultTheta = 0.0f;

			memset(m_fArrRstRelativeHeightMin, 0, _MAX_DEFECT_CNT * sizeof(float));
			memset(m_fArrRstRelativeHeightMax, 0, _MAX_DEFECT_CNT * sizeof(float));
			memset(m_fArrRstContrast, 0, _MAX_DEFECT_CNT * sizeof(float));


		}

	}_RstAlgoPadBW;

#define  _dfBodyBlobShapeAreaCnt		3
	typedef struct _tagAlgoBodyBlob
	{
		BOOL Passive;
		float PassiveMargin;
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
		double m_dTeachWidthMin;
		double m_dTeachWidthMax;

		BOOL m_bTeachLengthUse;
		double m_dTeachLength;
		double m_dTeachLengthRateMin;
		double m_dTeachLengthRateMax;
		double m_dTeachLengthMin;
		double m_dTeachLengthMax;

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

		struct _tagAlgoColorBase m_sAlgoColorBase;
		// 	int m_nHeightDivisionLevel;

		BOOL	m_bIsModelSubImg;
		BOOL	m_bUsePattern;
		wchar_t m_sModelPath[_MAX_STRLEN] = { 0 };

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

		bool m_bUseShape;
		float m_fTeachShapeHeightMax;
		float m_fTeachShapeArea;

		bool m_bUseDamage2D;

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
		int m_nHeightRefValue;

		BOOL m_bUseOffsetDistance;
		float m_fOffsetDistance;

		_tagAlgoBodyBlob()
		{
			Passive = FALSE;
			PassiveMargin = 0.1f;
			m_byIPCClass = 0;
			m_bInvertCheck = FALSE;
			m_dAreaMin = 80.;
			m_dAreaMax = 120.;
			m_nTypeSelectBlob = _eSelectCenter;
			m_bFillHole = FALSE;
			m_nFilterStepNarrow = 4;

			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_fHeightAvg = 0.0F;
			m_nTypeRange3D = _eTypeRangeIn;

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
			m_nTip2dRange = _eTypeRangeUpper;
			m_nTip2dMinValue = 0;
			m_nTip2dMaxValue = 180;
			m_bTip3dCheck = TRUE;
			m_nTip3dRange = _eTypeRangeUpper;
			m_dTip3dHeightMin = 0;
			m_dTip3dHeightMax = 30;
			m_bIsHorizon = TRUE;

			m_bUseShape = FALSE;
			m_fTeachShapeHeightMax = 0;
			m_fTeachShapeArea = 0;

			m_bUseDamage2D = false;

			m_b3dPerCheck = FALSE;
			m_n3dPerRange = _eTypeRangeUpper;
			m_f3dPerHeightMin = 80.0f;
			m_f3dPerHeightMax = 0.0f;
			m_f3dPerHeightAvg = 0.0f;

			m_bUsePattern = FALSE;

			m_bRChipWithOCR = FALSE;
			m_fSearchDefault = 50;
			m_fSearchBodyTip = 50;
			m_bNGOffset = FALSE;
			m_nHeightRefValue = 0;

			m_bUseOffsetDistance = FALSE;
			m_fOffsetDistance = 0.3f;

			m_bAreaIsUse = FALSE;
			m_bFilterIsUse = FALSE;
			m_bOnlyBodyTip = FALSE;
			m_bShiftIsUse = FALSE;
			m_bShiftMaxUse = FALSE;
			m_bUseBlobSizeWidth = FALSE;
			m_bUseBlobSizeLength = FALSE;
			m_dAreaCurrent = 0.;
			m_dSearchMargin = 0.;
			m_dShiftX = 0.;
			m_dShiftY = 0.;
			m_dTeachLengthMax = 0.;
			m_dTeachLengthMin = 0.;
			m_dTeachWidthMax = 0.;
			m_dTeachWidthMin = 0.;
			
		}
	}_AlgoBodyBlob;

	typedef struct _tagRstAlgoBodyBlob	// BodyBlob 검사 결과
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
		double m_dRstShapeArea[_dfBodyBlobShapeAreaCnt];
		double m_dRstShapeArea_Per[_dfBodyBlobShapeAreaCnt] = { 0 };
		double m_dRstHeightMean_Per;

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

		RECT m_rcBodyRect = { 0 };
		RECT m_rcInspBodyRect = { 0 };
		RECT m_rcBodyDamageRect = { 0 };
		RECT m_rcRect_T = { 0 };
		RECT m_rcBodyShapeRect[_dfBodyBlobShapeAreaCnt];
		POINTF m_poDrawCenter;

		BOOL m_bOKOffsetDistance;
		float m_fOffsetDistance;

		_tagRstAlgoBodyBlob()
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
			memset(m_dRstShapeArea, 0, _dfBodyBlobShapeAreaCnt * sizeof(double));
			memset(m_rcBodyShapeRect, 0, _dfBodyBlobShapeAreaCnt * sizeof(RECT));
			m_bOKOffsetDistance = FALSE;
			m_fOffsetDistance = 0.0f;

			m_bOKArea = FALSE;
			m_bShowRect = FALSE;
			m_dRstArea = 0.0;
			m_dRstDamageRate_Per = 0.0;
			m_dRstHeightMean_Per = 0.0;
			m_fRstShiftMaxX = 0.f;
			m_fRstShiftMaxY = 0.f;
			m_nShapeAreaCnt = 0;
			m_poDrawCenter.x = 0.f;
			m_poDrawCenter.y = 0.f;

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
	}_RstAlgoBodyBlob;


#define  _EdgeLineTotalCnt	4
#define  _EdgeInspCnt		3
	typedef struct _tagAlgoEdge : _Blob
	{
		// IPC
		BOOL m_bUseIPC;
		byte m_byIPCClass;

		BOOL m_bShiftIsUse;
		double m_dShiftX;
		double m_dShiftY;

		BOOL m_bAreaIsUse;
		double m_dAreaMin;
		double m_dAreaMax;
		double m_dAreaCurrent;				//PercentOK

		float m_fHeightAvg;

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

		struct _tagAlgoColorBase m_sAlgoColorBase;
		// 	int m_nHeightDivisionLevel;

	/*	BOOL	m_bIsModelSubImg;
		BOOL	m_bUsePattern;
		wchar_t m_sModelPath[_MAX_STRLEN];*/

		int m_nSetLineCnt;
		BOOL m_bGroup;

		BOOL m_bArrIsHorizon[_EdgeLineTotalCnt];
		int m_nArrMeasureDirection[_EdgeLineTotalCnt];
		int m_nArrSetInspCondition[_EdgeInspCnt];

		POINTF m_poArrSetTeachCenter[_EdgeLineTotalCnt];
		double m_dArrTeachLength[_EdgeLineTotalCnt];

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
		_tagAlgoEdge()
		{
			m_byIPCClass = 0;
			m_bInvertCheck = FALSE;
			m_dAreaMin = 80.;
			m_dAreaMax = 120.;
			m_nTypeSelectBlob = _eSelectCenter;
			m_bFillHole = FALSE;
			m_nFilterStepNarrow = 4;

			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_fHeightAvg = 0.0F;
			m_nTypeRange3D = _eTypeRangeIn;

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


		/*	m_bIsModelSubImg = TRUE;
			m_bUsePattern = FALSE;*/
			// 		m_nHeightDivisionLevel = 1;

			m_nSetLineCnt = 0;
			m_bGroup = TRUE;

			m_bUseAngle = TRUE;
			m_dTeachRotate = 10.;

			memset(m_bArrIsHorizon, FALSE, sizeof(BOOL) * _EdgeLineTotalCnt);
			memset(m_nArrMeasureDirection, 0, sizeof(int) * _EdgeLineTotalCnt);
			memset(m_nArrSetInspCondition, 0, sizeof(int) * _EdgeInspCnt);
			memset(m_dArrTeachLength, 0, sizeof(double) * _EdgeLineTotalCnt);
			memset(m_poArrSetTeachCenter, 0, sizeof(POINTF) * _EdgeLineTotalCnt);

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

			m_bFillHole = FALSE;
			m_bAreaIsUse = FALSE;
			m_bShiftIsUse = FALSE;
			m_bUseBlobSizeLength = FALSE;
			m_bUseBlobSizeWidth = FALSE;

			m_dAreaCurrent = 0.;
			m_dShiftX = 0.;
			m_dShiftY = 0.;

			m_dTeach1_3PtDis = 0.;
			m_dTeach1_3PtDisMin = 0.;
			m_dTeach1_3PtDisMax = 0.;
			m_dTeach1_3PtDisOff = 0.;
			m_dTeach3_3PtDis = 0.;
			m_dTeach3_3PtDisMin = 0.;
			m_dTeach3_3PtDisMax = 0.;
			m_dTeach3_3PtDisOff = 0.;
		}
	}_AlgoEdge;

	typedef struct _tagRstAlgoEdge // Edge 검사 결과
	{
		double m_dRstShiftX;
		double m_dRstShiftY;
		double m_dRstRealAngle;
		double m_dRstAngle;
		double m_dRstLength[_EdgeLineTotalCnt];
		double m_dRstDistance;
		double m_dRstDistanceX;
		double m_dRstDistanceY;
		POINTF m_poDrawLine[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_Sec[_EdgeLineTotalCnt] = { 0 };
		// OK 판정 //
		BOOL m_bOKShiftX;
		BOOL m_bOKShiftY;
		BOOL m_bOKAngle;
		BOOL m_bMissing;
		BOOL m_bOKLength;
		BOOL m_bArrOKLength[_EdgeLineTotalCnt];
		BOOL m_bDistance;
		BOOL m_bDistanceX;
		BOOL m_bDistanceY;
		POINTF m_poDrawLine_T[_EdgeLineTotalCnt];
		POINTF m_poDrawLine_T2[_EdgeLineTotalCnt];
		//POINTF m_poDrawLine_T3[2];
		POINTF m_poDrawCenter;
		_tagRstAlgoEdge()
		{
			m_dRstShiftX = 0.;
			m_dRstShiftY = 0.;
			m_dRstRealAngle = 0.;
			m_dRstAngle = 0.;
			memset(m_dRstLength, 0, sizeof(double) * _EdgeLineTotalCnt);
			m_dRstDistance = 0.;
			m_bOKShiftX = FALSE;
			m_bOKShiftY = FALSE;
			m_bOKAngle = FALSE;
			m_bMissing = FALSE;
			m_bDistance = FALSE;
			m_bDistanceX = FALSE;
			m_bDistanceY = FALSE;
			memset(m_bArrOKLength, FALSE, sizeof(BOOL) * _EdgeLineTotalCnt);
			memset(m_poDrawLine_T, 0, sizeof(POINTF) * _EdgeLineTotalCnt);
			memset(m_poDrawLine_T2, 0, sizeof(POINTF) * _EdgeLineTotalCnt);
			//memset(m_poDrawLine_T3, 0, sizeof(POINTF) * 2);
			m_bOKLength = FALSE;
			m_dRstDistanceX = 0.;
			m_dRstDistanceY = 0.;
			m_poDrawCenter.x = 0.f;
			m_poDrawCenter.y = 0.f;
		}
	}_RstAlgoEdge;

	typedef struct _tagAlgoLine : _Blob
	{
		// IPC
		BOOL m_bUseIPC;
		byte m_byIPCClass;
	
		BOOL m_bShiftIsUse;
		double m_dShiftX;
		double m_dShiftY;

		BOOL m_bAreaIsUse;
		double m_dAreaMin;
		double m_dAreaMax;
		double m_dAreaCurrent;				//PercentOK

		float m_fHeightAvg;

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

		struct _tagAlgoColorBase m_sAlgoColorBase;
		// 	int m_nHeightDivisionLevel;

	/*	BOOL	m_bIsModelSubImg;
		BOOL	m_bUsePattern;
		wchar_t m_sModelPath[MAX_STRLEN];*/

		BOOL m_bIsHorizon;
		int m_nMeasureDirection;

		BOOL m_bUseAngle;						// Angle NG 검사 유무
		double m_dTeachRotate;

		BOOL m_bUseFix;
		int m_nCrossOpt;
		byte m_byLineFindType[2];
		BOOL m_bUseEndPos;
		int m_nLineData;
		float m_fArrPerpendicular[_eMMD_Total];
		BOOL m_bUseInspROI; // Line 찾을 때 지정된 ROI내에서만 찾는 옵션
		RECT m_rcInspROI;
		_tagAlgoLine()
		{
			m_byIPCClass = 0;
			m_bInvertCheck = FALSE;
			m_dAreaMin = 80.;
			m_dAreaMax = 120.;
			m_nTypeSelectBlob = _eSelectCenter;
			m_bFillHole = FALSE;
			m_nFilterStepNarrow = 4;

			m_bInsp2D = TRUE;
			m_nMinBinary = 125;
			m_nMaxBinary = 255;
			m_nTypeRange2D = _eTypeRangeIn;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_fHeightAvg = 0.0F;
			m_nTypeRange3D = _eTypeRangeIn;

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


			/*m_bIsModelSubImg = TRUE;
			m_bUsePattern = FALSE;*/
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
			memset(m_fArrPerpendicular, 0, sizeof(float) * _eMMD_Total);

			m_bFillHole = FALSE;
			m_bUseInspROI = FALSE;
			m_rcInspROI.left = 0;
			m_rcInspROI.right = 0;
			m_rcInspROI.top = 0;
			m_rcInspROI.bottom = 0;

			m_bAreaIsUse = FALSE;
			m_bShiftIsUse = FALSE;
			m_bUseBlobSizeLength = FALSE;
			m_bUseBlobSizeWidth = FALSE;
			m_dAreaCurrent = 0.;
			m_dShiftX = 0.;
			m_dShiftY = 0.;
		}
	}_AlgoLine;

	typedef struct _tagRstAlgoLine // Line 검사 결과
	{
		double m_dRstShiftX;
		double m_dRstShiftY;
		double m_dRstAngle;
		double m_dRstLength;
		POINTF m_poDrawLine[2]={0};

		// OK 판정 //
		BOOL m_bOKShiftX;
		BOOL m_bOKShiftY;
		BOOL m_bOKAngle;
		BOOL m_bMissing;
		BOOL m_bOKLength;

		POINTF m_poDrawLine_T[2] = { 0 };
		POINTF m_poDrawCenter = { 0 };

		BOOL m_bOKWidth;
		byte m_byWidthCnt;
		double m_dRstCuttinhW[50];
		double m_dRstCuttinhL[50];
		RECT m_rcDrawWidth[50];

		BOOL m_bOKPerpendicular;
		float m_fRstPerpendicular;
		POINTF m_poDrawLine_2[2];
		_tagRstAlgoLine()
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
	}_RstAlgoLine;

	typedef struct _tagAlgoAlignEdge
	{
		struct _tagAlgoEdge sArrAlgoEdge[_ALIGNEDGE_AREA_CNTS];
		POINTF m_sArrSearchPoint[_ALIGNEDGE_AREA_CNTS];	// Search 위치 좌표 (Part 기준 pixel 좌표)
		SIZE m_sArrSearchSize[_ALIGNEDGE_AREA_CNTS];		// Search 영역 사이즈 (pixel 단위)
		int m_nAreaCnt;
		BOOL m_bCicleOpt;
		int m_nOPT;
		BOOL m_bUseAnchor; // align window와 검출 center의 좌표 거리 반환 옵션
		BOOL m_bUseTwoAnchor;
		wchar_t m_sAlignWindow[MAX_STRLEN];
		double m_dDistOffset;
		double m_dDistXOffset;
		double m_dDistYOffset;

		_tagAlgoAlignEdge()
		{
			for (int a = 0; a < _ALIGNEDGE_AREA_CNTS; a++)
			{
				sArrAlgoEdge[a].m_bInvertCheck = FALSE;
				sArrAlgoEdge[a].m_dAreaMin = 80.;
				sArrAlgoEdge[a].m_dAreaMax = 120.;
				sArrAlgoEdge[a].m_nTypeSelectBlob = _eSelectCenter;
				sArrAlgoEdge[a].m_bFillHole = FALSE;
				sArrAlgoEdge[a].m_nFilterStepNarrow = 4;

				sArrAlgoEdge[a].m_bInsp2D = TRUE;
				sArrAlgoEdge[a].m_nMinBinary = 125;
				sArrAlgoEdge[a].m_nMaxBinary = 255;
				sArrAlgoEdge[a].m_nTypeRange2D = _eTypeRangeIn;

				sArrAlgoEdge[a].m_bInsp3D = FALSE;
				sArrAlgoEdge[a].m_dHeightRateMin = 80;
				sArrAlgoEdge[a].m_dHeightRateMax = 120;
				sArrAlgoEdge[a].m_fHeightAvg = 0.0F;
				sArrAlgoEdge[a].m_nTypeRange3D = _eTypeRangeIn;

				sArrAlgoEdge[a].m_sAlgoColorBase.m_bUseColor = FALSE;
				sArrAlgoEdge[a].m_sAlgoColorBase.m_nPolygonCnt = 1;
				sArrAlgoEdge[a].m_sAlgoColorBase.m_bInvert = FALSE;
				for (int n = 0; n < _COLORALGO_POLYGON_CNTS; n++)
				{
					for (int k = 0; k < _POLYGON_POINT_CNTS; k++)
					{
						sArrAlgoEdge[a].m_sAlgoColorBase.m_ptArrPolygon[n][k].x = -1;
						sArrAlgoEdge[a].m_sAlgoColorBase.m_ptArrPolygon[n][k].y = -1;
					}
				}

				sArrAlgoEdge[a].m_dTechCenterX = 0.0;
				sArrAlgoEdge[a].m_dTechCenterY = 0.0;

				sArrAlgoEdge[a].m_bUseIPC = FALSE;

				sArrAlgoEdge[a].m_bTeachWidthUse = FALSE;
				sArrAlgoEdge[a].m_dTeachWidth = 0.0;
				sArrAlgoEdge[a].m_dTeachWidthRateMin = 80.;
				sArrAlgoEdge[a].m_dTeachWidthRateMax = 120.;
				sArrAlgoEdge[a].m_bTeachLengthUse = FALSE;
				sArrAlgoEdge[a].m_dTeachLength = 0.0;
				sArrAlgoEdge[a].m_dTeachLengthRateMin = 80.;
				sArrAlgoEdge[a].m_dTeachLengthRateMax = 120.;

				sArrAlgoEdge[a].m_bUseBlobNG = FALSE;
				sArrAlgoEdge[a].m_dBlobSizeWidth = 0.f;
				sArrAlgoEdge[a].m_dBlobSizeLength = 0.f;

				sArrAlgoEdge[a].m_bShiftXUse = TRUE;
				sArrAlgoEdge[a].m_bShiftYUse = TRUE;

				sArrAlgoEdge[a].m_bUseBlobNG = FALSE;
				sArrAlgoEdge[a].m_dBlobSizeWidth = 0.0;
				sArrAlgoEdge[a].m_dBlobSizeLength = 0.0;

				sArrAlgoEdge[a].m_bUseHeight = FALSE;
				sArrAlgoEdge[a].m_dTeachHeight = 0.;
				sArrAlgoEdge[a].m_dTeachHeightMax = 0.;
				sArrAlgoEdge[a].m_dTeachHeightMin = 0.;

				sArrAlgoEdge[a].m_nSetLineCnt = 0;
				sArrAlgoEdge[a].m_bGroup = TRUE;

				sArrAlgoEdge[a].m_bUseAngle = TRUE;
				sArrAlgoEdge[a].m_dTeachRotate = 10.;

				memset(sArrAlgoEdge[a].m_bArrIsHorizon, FALSE, sizeof(BOOL) * _EdgeLineTotalCnt);
				memset(sArrAlgoEdge[a].m_nArrMeasureDirection, 0, sizeof(int) * _EdgeLineTotalCnt);
				memset(sArrAlgoEdge[a].m_nArrSetInspCondition, 0, sizeof(int) * _EdgeInspCnt);
				memset(sArrAlgoEdge[a].m_dArrTeachLength, 0, sizeof(double) * _EdgeLineTotalCnt);
				memset(sArrAlgoEdge[a].m_poArrSetTeachCenter, 0, sizeof(POINTF) * _EdgeLineTotalCnt);

				sArrAlgoEdge[a].m_nLineFindType = 0;
				sArrAlgoEdge[a].m_dLineFindRate = 100;
			}

			memset(m_sArrSearchPoint, 0, sizeof(POINTF) * _ALIGNEDGE_AREA_CNTS);
			memset(m_sArrSearchSize, 0, sizeof(SIZE) * _ALIGNEDGE_AREA_CNTS);
			m_nAreaCnt = 2;
			m_bCicleOpt = FALSE;
			m_nOPT = 0x3F;
			m_bUseAnchor = FALSE;
			m_bUseTwoAnchor = FALSE;
			//m_sAlignWindow = "";
			m_dDistOffset = 0.0;
			m_dDistXOffset = 0.0;
			m_dDistYOffset = 0.0;
		}
	}_AlgoAlignEdge;

	typedef struct _tagRstAlgoAlignEdge
	{
		double m_dCenterX_T;
		double m_dCenterY_T;
		double m_dCenterX_R;
		double m_dCenterY_R;

		double m_dRstShiftX;
		double m_dRstShiftY;
		double m_dTheta;
		double m_dDistance;
		double m_dDistanceX;
		double m_dDistanceY;
		double m_d3PtDis1;
		double m_d3PtDis3;
		double m_dArea[_EdgeInspCnt] = { 0 };
		// OK 판정 //
		BOOL m_bOKShiftX;
		BOOL m_bOKShiftY;
		BOOL m_bOKAngle;
		BOOL m_bOKDistance;
		BOOL m_bOKDistanceX;
		BOOL m_bOKDistanceY;
		BOOL m_bOK3PtDis1;
		BOOL m_bOK3PtDis3;

		BOOL m_bMissing_1;
		BOOL m_bMissing_2;
		BOOL m_bMissing_3;

		POINTF m_poDrawLine[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_Sec[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_1[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_Sec_1[_EdgeLineTotalCnt] = { 0 };

		POINTF m_poDrawLine_2[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_Sec_2[_EdgeLineTotalCnt] = { 0 };

		RECT m_rcRect_I[3] = { 0 };
		POINTF m_poDrawLine_T[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_T2[_EdgeLineTotalCnt] = { 0 };

		POINTF m_poDrawLine_T21[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_T22[_EdgeLineTotalCnt] = { 0 };

		POINTF m_poDrawLine_T31[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawLine_T32[_EdgeLineTotalCnt] = { 0 };
		POINTF m_poDrawCenter = { 0 };

		//for teaching display
		RECT m_rcFindCircle[3] = { 0 };
		POINTF m_pHPoint[4] = { 0 };
		POINTF m_poRstCenter[_EdgeLineTotalCnt] = { 0 };
		_tagRstAlgoAlignEdge()
		{
			m_dRstShiftX = 0.;
			m_dRstShiftY = 0.;
			m_dTheta = 0.;
			m_dDistance = 0.;
			m_d3PtDis1 = 0.;
			m_d3PtDis3 = 0.;

			m_bOKShiftX = FALSE;
			m_bOKShiftY = FALSE;
			m_bOKAngle = FALSE;
			m_bOKDistance = FALSE;
			m_bOK3PtDis1 = FALSE;
			m_bOK3PtDis3 = FALSE;

			m_bMissing_1 = FALSE;
			m_bMissing_2 = FALSE;
			m_bMissing_3 = FALSE;

			m_bOKDistanceX = FALSE;
			m_bOKDistanceY = FALSE;

			m_dCenterX_T = 0.;
			m_dCenterY_T = 0.;
			m_dCenterX_R =0.;
			m_dCenterY_R =0.;
			
			m_dDistanceX = 0.;
			m_dDistanceY = 0.;
		}
	}_RstAlgoAlignEdge;

	typedef struct _tagAlgoAlign : _Bin
	{
		//BOOL m_bUseIPC;
		//byte m_byIPCClass;
		double m_dHeightAvg;

		int m_nSearchNum;				// Search 영역 개수
		POINTF m_sArrSearchPoint[4] = { 0 };	// Search 위치 좌표 (Part 기준 pixel 좌표)
		SIZE m_sArrSearchSize[4] = { 0 };		// Search 영역 사이즈 (pixel 단위)
		int m_nSearchMargin;			// Search 영역 Margin +- 적용 (pixel 단위)	

		BOOL m_bUseShift;		// Shift NG 검사 유무
		double m_dShiftX;
		double m_dShiftY;
		BOOL m_bUseAngle;		// Angle NG 검사 유무
		double m_dAngle;

		int m_nMinBlobArea;
		struct _tagAlgoColorBase m_sAlgoColorBase;
		BOOL m_bFillHole;
		byte m_byInspOPT;
		BOOL m_bMaxBlobFull;
		BOOL m_bFilterIsUse;
		int m_nFilterStepNarrow;
		BOOL m_bSameSize;
		_tagAlgoAlign()
		{
			m_bInvertCheck = FALSE;
			m_bInsp2D = TRUE;
			m_nMinBinary = 100;
			m_nMaxBinary = 100;

			m_bInsp3D = FALSE;
			m_dHeightRateMin = 80;
			m_dHeightRateMax = 120;
			m_dHeightAvg = 0.0F;

			m_nSearchNum = 2;
			m_nSearchMargin = 5;

			m_bUseShift = false;
			m_dShiftX = 1;
			m_dShiftY = 1;

			m_bUseAngle = FALSE;		// Angle NG 검사 유무
			m_dAngle = 0;
			m_nMinBlobArea = 5;
			m_bFillHole = FALSE;
			m_byInspOPT = 0;
			m_bMaxBlobFull = FALSE;
			m_bFilterIsUse = FALSE;
			m_nFilterStepNarrow = 4;
			m_bSameSize = TRUE;
		}
	}_AlgoAlign;

	typedef struct _tagRstAlgoAlign	// Align 검사 결과
	{
		double m_dOffset_x;
		double m_dOffset_y;
		double m_dTheta;
		double m_dRotCenterX;
		double m_dRotCenterY;

		// OK 판정 //
		BOOL m_bOKShiftX;
		BOOL m_bOKShiftY;
		BOOL m_bOKAngle;

		RECT m_rcBodyRect = { 0 };

		RECT m_rcRect_T[4] = { 0 };
		RECT m_rcRect_I[4] = { 0 };
		POINTF m_poDrawCenter = { 0 };
		_tagRstAlgoAlign()
		{
			m_dOffset_x = 0.;
			m_dOffset_y = 0.;
			m_dTheta = 0.;
			m_dRotCenterX = 0.;
			m_dRotCenterY = 0.;

			m_bOKShiftX = FALSE;
			m_bOKShiftY = FALSE;
			m_bOKAngle = FALSE;
		}
	}_RstAlgoAlign;

	const int _BODY_EDGE_RECT_CNTS = 16; //BoyEdge Algo Area

	typedef struct _tagAlgoBodyEdge : _Blob
	{
		float m_dTechCenterX;
		float m_dTechCenterY;

		BOOL m_bUseShift;
		BOOL m_bUseShiftX;
		BOOL m_bUseShiftY;
		float m_dShiftX;
		float m_dShiftY;

		BOOL m_bUseAngle;
		float m_dAngle;
		float m_dStandardAngle; //Teaching시에 Angle

		BOOL m_bUseWidth;
		float m_dWidth;
		float m_dWidthRateMin;
		float m_dWidthRateMax;

		BOOL m_bUseLength;
		float m_dLength;
		float m_dLengthRateMin;
		float m_dLengthRateMax;

		RECT m_rcArea_T[16];
		int m_nAreaDirection[16] = { 0 };
		int m_nAreaNum;

		POINTF m_ptTeachEdge[4]={0}; //좌상, 우상, 우하, 좌하

		int m_nSelectIdxAnchorMode;
		int m_nSelectIdxWindow;

		BOOL m_bUseDistance;
		float m_dTeachDistanceX;
		float m_dTeachDistanceY;
		float m_dTeachToleranceX;
		float m_dTeachToleranceY; // AnchorMode에 따른 타 Window와의 거리 +-오차 범위 

		//subLine
		BOOL m_bUseSubLIne;
		BOOL m_bSubLineInsp2D;
		BOOL m_bSubLineInsp3D;
		int m_nSubLineMin2D;
		int m_nSubLineMax2D;
		int m_nSubLineRange2D;
		int m_nSubLineRange3D;

		float m_dSubLineMin3D;
		float m_dSubLineMax3D;

		BOOL m_bSelectBodyCenterBlob;

		BOOL m_bEdgeStraightness;
		float m_fEdgeStraightness;
		int m_nEdgeStraightness_Type; // 0: ROI 1: Body

		int m_nPreAlign_Type; // 0: None, 1: X축, 2: Y축

		float m_fSubLProp;

		BOOL m_bUseSobel_SubLine;
		BOOL m_bUseSobel_SubLineDetail;

		BOOL m_bEndLineFail;

		int m_TargetType; // 0:default 1: Circle

		_tagAlgoBodyEdge()
		{
			m_dTechCenterX = 0.0f;
			m_dTechCenterY = 0.0f;
			m_bUseShift = FALSE;
			m_bUseShiftX = FALSE;
			m_bUseShiftY = FALSE;
			m_dShiftX = 0.0f;
			m_dShiftY = 0.0f;
			m_bUseAngle = FALSE;
			m_dAngle = 0.0f;
			m_dStandardAngle = 0.0f;
			m_bUseWidth = FALSE;
			m_dWidth = 0.0f;
			m_dWidthRateMin = 0.0f;
			m_dWidthRateMax = 0.0f;

			m_bUseLength = FALSE;
			m_dLength = 0.0f;
			m_dLengthRateMin = 0.0f;
			m_dLengthRateMax = 0.0f;

			/*	RECT m_rcArea_T[16];
				int m_nAreaDirection[16];*/
			m_nAreaNum = 0;

			//POINTF m_ptTeachEdge[4]; //좌상, 우상, 우하, 좌하

			m_nSelectIdxAnchorMode = 0;
			m_nSelectIdxWindow = 0;

			m_bUseDistance = FALSE;
			m_dTeachDistanceX = 0.0f;
			m_dTeachDistanceY = 0.0f;
			m_dTeachToleranceX = 0.0f;
			m_dTeachToleranceY = 0.0f;

			//subLine
			m_bUseSubLIne = FALSE;
			m_bSubLineInsp2D = FALSE;
			m_bSubLineInsp3D = FALSE;
			m_nSubLineMin2D = 0;
			m_nSubLineMax2D = 0;
			m_nSubLineRange2D = 0;
			m_nSubLineRange3D = 0;

			m_dSubLineMin3D = 0.0f;
			m_dSubLineMax3D = 0.0f;

			m_bSelectBodyCenterBlob = FALSE;

			m_bEdgeStraightness = FALSE;
			m_fEdgeStraightness = 0.;
			m_nEdgeStraightness_Type = 0;

			m_nPreAlign_Type = 0;
			m_fSubLProp = 0.65f;

			m_bUseSobel_SubLine = FALSE;
			m_bUseSobel_SubLineDetail = FALSE;
			m_bEndLineFail = FALSE;

			m_TargetType = 0;
		}

	}_AlgoBodyEdge; //Part 이미지 기준

	typedef struct _tagRstAlgoBodyEdge
	{
		double m_dRstOffset_x;
		double m_dRstOffset_y;
		double m_dRstTheta; //Teaching시에 Angle 반영
		double m_dRstRealTheta;
		double m_dRstWidth;
		double m_dRstLength;
		double m_dRstCenterX;
		double m_dRstCenterY;
		double m_dRstDistanceX;	// AnchorMode에 따른 타 Window와의 측정 거리
		double m_dRstDistanceY;
		double m_dRstTeachDistanceX;
		double m_dRstTeachDistanceY;

		// OK 판정 //
		BOOL m_bOKShiftX;
		BOOL m_bOKShiftY;
		BOOL m_bOKAngle;
		BOOL m_bOKWidth;
		BOOL m_bOKLength;
		BOOL m_bOKDistanceX;
		BOOL m_bOKDistanceY;

		int m_nAreaNum;
		RECT m_rcArea_T[_BODY_EDGE_RECT_CNTS] = { 0 };;
		RECT m_rcArea_I[_BODY_EDGE_RECT_CNTS] = { 0 };
		//	BOOL m_bArea_OK[16];

		RECT m_rcBodyRect = { 0 };;
		POINTF m_ptInspEdge[4]; //좌상, 우상, 우하, 좌하 Part Pixel 좌표
		POINTF m_poDrawCenter;

		BOOL m_bOKEdgeStraightness;
		float m_fRstEdgeStraightness;
		float m_fRstES_Above;
		float m_fRstES_Below;
		POINT m_poES_Above;
		POINT m_poES_Below;

		BOOL m_bMissingOK;

		_tagRstAlgoBodyEdge()
		{
			m_dRstOffset_x = 0.;
			m_dRstOffset_y = 0.;
			m_dRstTheta = 0.;
			m_dRstRealTheta = 0.;
			m_dRstWidth = 0.;
			m_dRstLength = 0.;
			m_dRstCenterX = 0.;
			m_dRstCenterY = 0.;
			m_dRstDistanceX = 0.;
			m_dRstDistanceY = 0.;
			m_dRstTeachDistanceX = 0.;
			m_dRstTeachDistanceY = 0.;

			m_bOKShiftX = FALSE;
			m_bOKShiftY = FALSE;
			m_bOKAngle = FALSE;
			m_bOKWidth = FALSE;
			m_bOKLength = FALSE;
			m_bOKDistanceX = FALSE;
			m_bOKDistanceY = FALSE;

			memset(m_ptInspEdge, 0, sizeof(POINTF) * 4);
			memset(&m_poDrawCenter, 0, sizeof(POINTF));

			m_nAreaNum = 0;

			m_fRstEdgeStraightness = 0.;
			m_bOKEdgeStraightness = FALSE;
			m_fRstES_Above = 0;
			m_fRstES_Below = 0;
			memset(&m_poES_Above, 0, sizeof(POINT));
			memset(&m_poES_Below, 0, sizeof(POINT));

			m_bMissingOK = TRUE;

		}
	}_RstAlgoBodyEdge;

	typedef struct _stAlgoPackageThickness
	{
		byte m_nArrData[_m_ePackageThicknessBy_Total];
		float m_fArrData[_m_ePackageThicknessF_Total];
		int idata;
		RECT rcFirstROI;
		_stAlgoPackageThickness()
		{
			memset(m_nArrData, 0, _m_ePackageThicknessBy_Total * sizeof(byte));
			memset(m_fArrData, 0, _m_ePackageThicknessF_Total * sizeof(float));
			idata = 0;
			rcFirstROI.bottom = 0;
			rcFirstROI.left = 0;
			rcFirstROI.right = 0;
			rcFirstROI.top = 0;
		}
	}_AlgoPackageThickness;

	typedef struct _tagRstAlgoPackageThickness
	{
		BOOL m_bOKThickness;
		BOOL m_bOKThicknessTH;
		BOOL m_bOKThicknessCH;
		double m_dRstThickness;
		double m_dRstPickerHeight;
		double m_dRstPackageHeight;
		double m_dRstThicknessCH;
		RECT m_RstRect[16];
		RECT m_rcArrRect[16];
		_tagRstAlgoPackageThickness()
		{
			m_bOKThickness = FALSE;
			m_bOKThicknessTH = FALSE;
			m_bOKThicknessCH = FALSE;
			m_dRstThickness = 0;
			m_dRstPickerHeight = 0;
			m_dRstPackageHeight = 0;
			m_dRstThicknessCH = 0;
			memset(m_RstRect, 0, 16 * sizeof(RECT));
			memset(m_rcArrRect, 0, 16 * sizeof(RECT));
		}

	}_RstAlgoPackageThickness;

	typedef struct _tagPoly
	{
		int m_nPtrcnt;      //최대 20개
		int m_nPolyDir;		 //in chipping inspect, Direction of Polygon Area
		POINTF m_dArrPtr[_MAX_POLYCNT];  //최대 20개
		void stPoly()
		{
			m_nPtrcnt = 0;
			memset(m_dArrPtr, 0, sizeof(POINTF) * _MAX_POLYCNT);
		}
	}_stPoly;
	typedef struct _tagAlgoGWire: _Blob
	{
		int m_nType;
		int m_nDirection;
		int m_nWireCnt;
		int m_nCenterMargin;

		double m_dWireThickness;
		
		BOOL m_bUseCrossFilter;
		BOOL m_bUseCenterMargin;
		BOOL m_bNotUseWireCheck;

		BOOL m_bUseShift;
		double m_dShift;

		BOOL m_bUseCheckBridge;
		BOOL m_bUseWidth;
		BOOL m_bUseLength;
		BOOL m_bUseArea;
		BOOL m_bUseCircleRate;
		BOOL m_bUseAngle;
		BOOL m_bUseWireConnectedFoot;
		BOOL m_bUse3DFlatten;

		double m_dTeachArea;
		double m_dTeachMinArea;
		double m_dTeachMaxArea;
		double m_dTeachWidth;
		double m_dTeachMinWidth;
		double m_dTeachMaxWidth;
		double m_dTeachLength;
		double m_dTeachMinLength;
		double m_dTeachMaxLength;
		double m_dTeachCircleRate;

		BOOL m_bInspDie2D;
		int m_nMinDieBinary;
		int m_nMaxDieBinary;
		int m_nTypeDieRange2D;

		BOOL m_bInspDie3D;
		double m_dHeightDieRateMin;
		double m_dHeightDieRateMax;
		int m_nTypeDieRange3D;
		
		int m_nCurBin;

		BOOL m_bDieFillHole;
		BOOL m_bDieFilterIsUse;
		int m_nDieFilterStepNarrow;
		BOOL m_bMergeAllDie;

		int m_nPadCnt;
		_stPoly m_arrPolygon[_GWIRE_MAX_PAD_CNT];
		double m_dTeachDieTheta;
		POINTF m_ptTeachDieCenter;

		bool m_bUseCntInPoly;
		int m_arrCntInPoly[_GWIRE_MAX_PAD_CNT];

		BOOL m_bUseInnerCircle;
		double m_dInnerCirOKRate;
		double m_dInnerCirShiftRate;

		BOOL m_bUseBlackWireOpt;
		BOOL m_bUseMajorMinor;

		BOOL UseFootThicknessRate;
		double TeachFootThicknessRateMin;
		double TeachFootThicknessRateMax;
		_tagAlgoGWire()
		{
			m_nType = 0;
			m_nDirection = 0;
			m_nWireCnt = 0;
			m_nCenterMargin = 0;

			m_dWireThickness = 0;

			m_bUseCrossFilter = FALSE;
			m_bUseCenterMargin = FALSE;
			m_bNotUseWireCheck = FALSE;

			m_bUseShift = FALSE;

			m_dShift = 0;

			m_bUseCheckBridge = FALSE;
			m_bUseWidth = FALSE;
			m_bUseLength = FALSE;
			m_bUseArea = FALSE;
			m_bUseCircleRate = FALSE;
			m_bUseAngle = FALSE;
			m_bUseWireConnectedFoot = FALSE;
			m_bUse3DFlatten = FALSE;

			m_dTeachArea = 0;
			m_dTeachMinArea = 0;
			m_dTeachMaxArea = 0;
			m_dTeachWidth = 0;
			m_dTeachMinWidth = 0;
			m_dTeachMaxWidth = 0;
			m_dTeachLength = 0;
			m_dTeachMinLength = 0;
			m_dTeachMaxLength = 0;
			m_dTeachCircleRate = 0;

			m_bInspDie2D = TRUE;
			m_nMinDieBinary = 125;
			m_nMaxDieBinary = 255;
			m_nTypeDieRange2D = _eTypeRangeIn;

			m_bInspDie3D = FALSE;
			m_dHeightDieRateMin = 80;
			m_dHeightDieRateMax = 120;
			m_nTypeDieRange3D = _eTypeRangeIn;

			m_nCurBin = 0;

			m_bDieFillHole = FALSE;
			m_bDieFilterIsUse = FALSE;
			m_nDieFilterStepNarrow = 0;
			m_bMergeAllDie = FALSE;

			m_nPadCnt = 0;
			memset(m_arrPolygon, 0, sizeof(_stPoly)*_GWIRE_MAX_PAD_CNT);
			m_dTeachDieTheta = 0.;
			m_ptTeachDieCenter.x = m_ptTeachDieCenter.y = 0;

			m_bUseCntInPoly = FALSE;
			memset(m_arrCntInPoly, 0, sizeof(int)*_GWIRE_MAX_PAD_CNT);

			m_bUseInnerCircle = FALSE;
			m_dInnerCirShiftRate = 0.0f;

			m_bUseBlackWireOpt = FALSE;

			m_bUseMajorMinor = FALSE;
			UseFootThicknessRate = FALSE;
			TeachFootThicknessRateMin = 250.0f;
			TeachFootThicknessRateMax = 500.0f;

			m_dInnerCirOKRate = 0.;
		}
	}_AlgoGWire;

	typedef struct _tagRstAlgoGWire
	{
		BOOL m_bCntOK;
		int m_nRstCnt;

		int m_nCenterMarginX;
		int m_nCenterMarginY;
		int m_nCenterMarginW;
		int m_nCenterMarginH;

		RECT m_arrRstRect[_GWIRE_MAX_CNT];
		double m_arrShift[_GWIRE_MAX_CNT];
		BOOL m_arrShiftisOK[_GWIRE_MAX_CNT];
		BOOL m_bShiftOK;

		BOOL m_arrCheckBridgeOK[_GWIRE_MAX_CNT];
		BOOL m_arrWidthOK[_GWIRE_MAX_CNT];
		BOOL m_arrLengthOK[_GWIRE_MAX_CNT];
		BOOL m_arrAreaOK[_GWIRE_MAX_CNT];
		BOOL m_arrCircleRateOK[_GWIRE_MAX_CNT];
		BOOL m_arrAngleOK[_GWIRE_MAX_CNT];
		BOOL m_arrWireConnectedFootOK[_GWIRE_MAX_CNT];

		double m_arrWidth[_GWIRE_MAX_CNT];
		double m_arrLength[_GWIRE_MAX_CNT];
		double m_arrArea[_GWIRE_MAX_CNT];
		double m_arrCircleRate[_GWIRE_MAX_CNT];
		double m_arrAngle[_GWIRE_MAX_CNT];

		BOOL m_bCheckBridgeOK;
		BOOL m_bWidthOK;
		BOOL m_bLengthOK;
		BOOL m_bAreaOK;
		BOOL m_bCircleRateOK;
		BOOL m_bAngleOK;
		BOOL m_bWireConnectedFootOK;

		BOOL m_arrRstOK[_GWIRE_MAX_CNT];
		POINTF m_poWnd_LT = { 0 };

		POINTF m_arrRstLine[_GWIRE_MAX_CNT * 2];

		double m_dRstDieTheta;
		POINTF m_ptRstDieCenter;
		int m_arrRstCntInPoly[_GWIRE_MAX_PAD_CNT];

		BOOL m_bInnerCircleOK;
		int m_nInnerCircleCnt;
		BOOL m_arrRstInnerCircleOK[_GWIRE_MAX_CNT];
		POINT m_arrRstInnerCirclePt[_GWIRE_MAX_CNT];
		double m_arrRstInnerCircleRad[_GWIRE_MAX_CNT];
		double m_arrRstInnerCirclefRate[_GWIRE_MAX_CNT];
		BOOL m_bBlackWireOK;
		int m_nBlackWireCnt;
		BOOL m_arrRstBlackWireOK[_GWIRE_MAX_CNT];
		RECT m_arrRstBlackWire[_GWIRE_MAX_CNT];

		BOOL m_bFootThicknessRateOK;
		BOOL m_arrRstFootThicknessRateOK[_GWIRE_MAX_CNT];
		double m_arrRstFootThicknessRate[_GWIRE_MAX_CNT];//um
		_tagRstAlgoGWire()
		{
			m_bCntOK = FALSE;
			m_nRstCnt = 0;

			m_nCenterMarginX = 0;
			m_nCenterMarginY = 0;
			m_nCenterMarginW = 0;
			m_nCenterMarginH = 0;

			memset(m_arrRstRect, 0, sizeof(RECT)*_GWIRE_MAX_CNT);
			memset(m_arrShift, 0, sizeof(double)*_GWIRE_MAX_CNT);
			memset(m_arrShiftisOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);

			m_bShiftOK = FALSE;

			memset(m_arrCheckBridgeOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrWidthOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrLengthOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrAreaOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrCircleRateOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrAngleOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrWireConnectedFootOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);

			memset(m_arrWidth, 0, sizeof(double)*_GWIRE_MAX_CNT);
			memset(m_arrLength, 0, sizeof(double)*_GWIRE_MAX_CNT);
			memset(m_arrArea, 0, sizeof(double)*_GWIRE_MAX_CNT);
			memset(m_arrCircleRate, 0, sizeof(double)*_GWIRE_MAX_CNT);
			memset(m_arrAngle, 0, sizeof(double)*_GWIRE_MAX_CNT);

			m_bCheckBridgeOK = FALSE;
			m_bWidthOK = FALSE;
			m_bLengthOK = FALSE;
			m_bAreaOK = FALSE;
			m_bCircleRateOK = FALSE;
			m_bAngleOK = FALSE;
			m_bWireConnectedFootOK = FALSE;

			memset(m_arrRstOK, 0, sizeof(BOOL)*_GWIRE_MAX_CNT);

			memset(m_arrRstLine, 0, sizeof(POINTF)*_GWIRE_MAX_CNT * 2);

			m_dRstDieTheta = 0.;
			m_ptRstDieCenter.x = m_ptRstDieCenter.y = 0;
			memset(m_arrRstCntInPoly, 0, sizeof(int)*_GWIRE_MAX_PAD_CNT);

			m_bInnerCircleOK = FALSE;
			m_nInnerCircleCnt = 0;
			memset(m_arrRstInnerCircleOK, 1, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrRstInnerCirclePt, 0, sizeof(POINT)*_GWIRE_MAX_CNT);
			memset(m_arrRstInnerCircleRad, 0, sizeof(double)*_GWIRE_MAX_CNT);
			memset(m_arrRstInnerCirclefRate, 0, sizeof(double)*_GWIRE_MAX_CNT);

			m_bBlackWireOK = FALSE;
			m_nBlackWireCnt = 0;
			memset(m_arrRstBlackWireOK, 1, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrRstBlackWire, 1, sizeof(RECT)*_GWIRE_MAX_CNT);

			m_bFootThicknessRateOK = FALSE;
			memset(m_arrRstFootThicknessRateOK, 1, sizeof(BOOL)*_GWIRE_MAX_CNT);
			memset(m_arrRstFootThicknessRate, 0, sizeof(double)*_GWIRE_MAX_CNT);
		}
	}_RstAlgoGWire;


	typedef struct _tagAlgoPOCR
	{
		double m_dWndAngle;
		double m_dFontAngle;

		wchar_t m_sWindName[_MAX_STRLEN] = { 0 };
		wchar_t m_sFontPath[_MAX_STRLEN] = { 0 };
		wchar_t m_sTargetFont[_MAX_STRLEN] = { 0 };		//targetString[_MAX_STRLEN];
		int m_nClrForeGround;					//nForeGroundColor;
		int m_nClrResultFont;					//nRstStringColor;

		int m_nThreshVal;
		int m_nThreshMode;						//0: manual  // 1:AUTO

		//************************
		//검사
		BOOL m_bUsePolarity;

		int m_nCharMaxCount;

		BOOL m_bUseLib;
		//Acceptance Data
		double m_dStdCharScore[_MAX_STRLEN];				//dStdStrScore


	// 	//LJH 2016.05.28
	// 	int m_nImageMixCount;
	// 	AlgoMixCondition AlgoMixCon[MAX_MIX_COUNT];

		BOOL m_bUsePOCRNG;
		BOOL m_bUseLowerSpecial;
		BOOL m_bUseCharSpace;
		BOOL m_bUseWndNameInWindow;
		BOOL m_bUseOCV;
		double m_dOCVScore;
		_tagAlgoPOCR()
		{
			m_bUsePolarity = FALSE;

			m_dWndAngle = 0.0;
			m_dFontAngle = 0.0;

			m_nClrForeGround = 0;
			m_nClrResultFont = 0;
			m_nCharMaxCount = 0;
			m_nThreshMode = 0;
			m_nThreshVal = 0;

			//Acceptance Data
			//m_dStdCharScore = 0.0;
			memset(m_dStdCharScore, 0, sizeof(double)*_MAX_STRLEN);
			//LJH 2016.05.28
	// 		m_nImageMixCount = 0;
	// 
	// 		for(int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
	// 		{
	// 			AlgoMixCon[iLoopCount].m_nForeGround = 1;
	// 			AlgoMixCon[iLoopCount].m_nThresholdValue = 128;
	// 			AlgoMixCon[iLoopCount].m_bUseAutoThreshold = false;
	// 			AlgoMixCon[iLoopCount].m_bUseNoiseFilter = false;
	// 		}

			m_bUsePOCRNG = FALSE;

			m_bUseLib = FALSE;
			m_bUseLowerSpecial = FALSE;
			m_bUseCharSpace = FALSE;
			m_bUseWndNameInWindow = FALSE;
			m_bUseOCV = FALSE;
			m_dOCVScore = 70.0;
		}

	}_AlgoPOCR;

	typedef struct _tagRstAlgoPOCR	// POCR 검사 결과
	{
		int m_nCharCount;
		double m_dStringScore;
		double m_dArrCharScore[_MAX_STRLEN];
		int m_nCharScoreResult[_MAX_STRLEN];
		wchar_t m_cArrStr[_MAX_STRLEN];

		// 	int m_nSearchThreshold;
		// 	int m_nAutoFlag;
			// OK 판정 //
		BOOL m_bOKString;
		BOOL m_bOKScore;
		BOOL m_bOKPolarity;

		double ModelX;
		double ModelY;
		double ModelWidth;
		double ModelHeight;

		double charX[_MAX_STRLEN];
		double charY[_MAX_STRLEN];
		double charWidth[_MAX_STRLEN];
		double charHeight[_MAX_STRLEN];
		double charAngle[_MAX_STRLEN];

		int Insp_Ro;
		int m_nCharScoreResult_Ro[_MAX_STRLEN];
		double ModelX_Ro;
		double ModelY_Ro;
		double ModelWidth_Ro;
		double ModelHeight_Ro;

		double m_dArrCharScore_Ro[_MAX_STRLEN];
		double charX_Ro[_MAX_STRLEN];
		double charY_Ro[_MAX_STRLEN];
		double charWidth_Ro[_MAX_STRLEN];
		double charHeight_Ro[_MAX_STRLEN];
		double charAngle_Ro[_MAX_STRLEN];

		BOOL bAIOK;
		int nAIFontOK[_MAX_STRLEN];
		wchar_t sFont_AI[_MAX_STRLEN];
		wchar_t sFont_AI_Sec[_MAX_STRLEN];
		double Score_AI[_MAX_STRLEN];
		double Score_AI_Sec[_MAX_STRLEN];

		BOOL bRstShiftX;
		double dRstShiftX;
		BOOL bRstShiftY;
		double dRstShiftY;
		double dStringAngle;
		wchar_t m_cArrTeachStr[MAX_STRLEN];

		_tagRstAlgoPOCR()
		{
			reset();
		}

		void reset()
		{
			m_nCharCount = 0;
			m_dStringScore = 100;
			memset(m_dArrCharScore, 0, sizeof(double)*_MAX_STRLEN);
			memset(m_nCharScoreResult, 0, sizeof(int)*_MAX_STRLEN);
			memset(m_cArrStr, 0, sizeof(wchar_t)*_MAX_STRLEN);

			// 	int m_nSearchThreshold;
			// 	int m_nAutoFlag;
			// OK 판정 //
			m_bOKString = true;
			m_bOKScore = true;
			m_bOKPolarity = true;

			memset(charX, 0, sizeof(double)*_MAX_STRLEN);
			memset(charY, 0, sizeof(double)*_MAX_STRLEN);
			memset(charWidth, 0, sizeof(double)*_MAX_STRLEN);
			memset(charHeight, 0, sizeof(double)*_MAX_STRLEN);
			memset(charAngle, 0, sizeof(double)*_MAX_STRLEN);

			Insp_Ro = 0;
			bAIOK = FALSE;
			memset(m_dArrCharScore_Ro, 0, sizeof(double)*_MAX_STRLEN);
			memset(charX_Ro, 0, sizeof(double)*_MAX_STRLEN);
			memset(charY_Ro, 0, sizeof(double)*_MAX_STRLEN);
			memset(charWidth_Ro, 0, sizeof(double)*_MAX_STRLEN);
			memset(charHeight_Ro, 0, sizeof(double)*_MAX_STRLEN);
			memset(charAngle_Ro, 0, sizeof(double)*_MAX_STRLEN);
			memset(nAIFontOK, 0, sizeof(int)*_MAX_STRLEN);
			memset(sFont_AI, 0, sizeof(wchar_t)*_MAX_STRLEN);
			memset(Score_AI, 0, sizeof(double)*_MAX_STRLEN);

			bRstShiftX = true;
			dRstShiftX = 0;
			bRstShiftY = true;
			dRstShiftY = 0;
			dStringAngle = 0;
			memset(m_cArrTeachStr, 0, sizeof(wchar_t) * _MAX_STRLEN);

		}
	}_RstAlgoPOCR;
	typedef struct tagOCRFilePath//shkim 2025.04.14 HalconOCRPath
	{
		wchar_t FilePath[_MAX_STRLEN];
	}OCRFilePath;
#define _Barcode1DQualityRECT 200
#define _Barcode1DDefectRECT 50
	typedef struct _tagAlgoBarcode
	{
		UCHAR * ucSrc;
		int nImgSizeX;
		int nImgSizeY;
		POINT stPt;
		POINT edPt;
		int nType;
		int nColor;
		int nFlipX;
		int nFlipY;
		int nCheckSum;
		int nCount;
		int nRetry;
		int nThresholdType;
		int nTimeOut;
		BOOL bPPDB_DTK_twice;
		BOOL bQualityCheck;
		BOOL bQuality2DModule;
		BOOL bTopQZCheck;
		BOOL bBtmQZCheck;
		float fTopQZLength;
		float fBtmQZLength;
		int nOffsetX;  // 1D Quality 
		int nOffsetY;  // 1D Quality 
		int nTolerance; //1D Quality
		int n2DTargetGrade;	// 2D Target Quality Grade
		int n2DContrast;	// 2D Quality 
		int n2DDefectArea;  // 2D Quality 
		int n2DDefectCnt;  // 2D Quality 
		BOOL bSaveImage;

		_tagAlgoBarcode()
		{
			ucSrc = NULL;
			nImgSizeX = 0;
			nImgSizeY = 0;
			stPt = POINT();
			edPt = POINT();
			nType = 0;
			nColor = 0;
			nFlipX = 0;
			nFlipY = 0;
			nCheckSum = 0;
			nCount = 1;
			nRetry = 1;
			nThresholdType = 0;
			nTimeOut = 300;
			bPPDB_DTK_twice = FALSE;
			bQualityCheck = FALSE;
			bQuality2DModule = FALSE;
			bTopQZCheck = FALSE;
			bBtmQZCheck = FALSE;
			fTopQZLength = 0.0;
			fBtmQZLength = 0.0;
			nOffsetX = 0;
			nOffsetY = 0;
			nTolerance = 0;
			n2DTargetGrade = 0;
			n2DContrast = 0;
			n2DDefectArea = 0;
			n2DDefectCnt = 0;
			bSaveImage = FALSE;
		}
	}_AlgoBarcode;

	typedef struct _tagRstAlgoBarcode
	{
		CString sResultCode;							// 바코드 리딩 데이터
		float fAngle;									// 검사 이미지에서 바코드 회전 각도
		POINTF ptCen;									// 바코드 인식 영역의 중간 좌표
		CRect rtBoundary;								// 바코드 인식 영역의 TLBR Rect
		POINTF vCorner[5];								// 바코드 인식 영역의 네 꼭지점 좌표
		float fArrBarcodeQC[_CNT_1DQUALITY_PARAMS];		// 품질검사 결과값(1D)
		float fArrDataMatrixQC[_CNT_2DQUALITY_PARAMS];	// 품질검사 결과값(2D)
		float fLengthTopQZ;								// 1D 상단 QZ 검사 길이
		float fLengthBtmQZ;								// 1D 하단 QZ 검사 길이
		POINTF pfNGRegionTopQZ[5];						// 1D 상단 QZ 검사 NG 영역 좌표
		POINTF pfNGRegionBtmQZ[5];						// 1D 하단 QZ 검사 NG 영역 좌표
		BOOL bQuietZoneLengthNG;						// QZ 거리가 충분한지 확인
		RECT DetectBarRect[_Barcode1DQualityRECT];		// 1D Barcode Black Bar 위치 Rect정보
		int nInspRectCnt;								// Quality Inspection Rect Cnt 
		RECT InspBarRect[_Barcode1DQualityRECT];		// Quality Inspection 위치 Rect정보 
		int nDefectRectCnt;								// Quality Inspection Defect Cnt 
		RECT InspDefectRect[_Barcode1DDefectRECT];		// Quality Defect 위치 Rect정보 
		int nDefectRectIndex[_Barcode1DDefectRECT];		// Quality Defect Index 정보 
		int nDefectGrade;								// Quality Defect Grade 등급

		_tagRstAlgoBarcode()
		{
			sResultCode = _T("");
			fAngle = 0.0;
			ptCen = POINTF();
			rtBoundary = CRect();
			memset(vCorner, 0, sizeof(POINTF) * 5);
			//memset(fArrBarcodeQC, 0, sizeof(float) * _CNT_1DQUALITY_PARAMS);
			//memset(fArrDataMatrixQC, 0, sizeof(float) * _CNT_2DQUALITY_PARAMS);
			for (int i = 0; i < _CNT_1DQUALITY_PARAMS; i++)
				fArrBarcodeQC[i] = -1;
			for (int i = 0; i < _CNT_2DQUALITY_PARAMS; i++)
				fArrDataMatrixQC[i] = -1;
			fLengthTopQZ = 0.0;
			fLengthBtmQZ = 0.0;
			memset(pfNGRegionTopQZ, 0, sizeof(POINTF) * 5);
			memset(pfNGRegionBtmQZ, 0, sizeof(POINTF) * 5);
			bQuietZoneLengthNG = FALSE;
			memset(DetectBarRect, 0, sizeof(RECT) * _Barcode1DQualityRECT);
			nInspRectCnt = 0;
			memset(InspBarRect, 0, sizeof(RECT) * _Barcode1DQualityRECT);
			nDefectRectCnt = 0;
			memset(InspDefectRect, 0, sizeof(RECT) * _Barcode1DDefectRECT);
			for (int i = 0; i < _Barcode1DDefectRECT; i++)
				nDefectRectIndex[i] = -1;
			//memset(nDefectRectIndex, 0, sizeof(int) * _Barcode1DDefectRECT);
			nDefectGrade = -1;
		}
	}_RstAlgoBarcode;

	typedef struct _tagAlgoPattern
	{
		BOOL m_bUsePolarity;
		double m_dAcceptScore;

		BOOL m_bShiftIsUse;
		double m_dShiftX;
		double m_dShiftY;
		double m_dRangeAngle;

		double m_dWndAngle;
		double m_dSearchAngleRange_Max;
		double m_dSearchAngleRange_Min;

		int m_nCntPatternPath;
		CString m_sArrPathModelInspect[_CNT_PATTERN_PATH];
		wchar_t m_sPathModelTeach[_MAX_STRLEN] = { 0 };

		//[Teaching_Output]
		/*double m_dRetScore;*/
		/*double m_dRetAngle;*/
		double m_dRetPosX;
		double m_dRetPosY;
		double m_dRetCogX;
		double m_dRetCogY;

		//[Inspect_Output]
		double m_dStdCogX[_CNT_PATTERN_PATH];
		double m_dStdCogY[_CNT_PATTERN_PATH];
		double m_dStdScore;

		int m_SamplingAngle;
		BOOL m_bUseNGOpt;

		int m_nDiviLnR[_CNT_PATTERN_PATH];
		int m_nDiviLnC[_CNT_PATTERN_PATH];

		double m_dGapLnR[_CNT_PATTERN_PATH][_CNT_PATTERN_DIVISION_R];
		double m_dGapLnC[_CNT_PATTERN_PATH][_CNT_PATTERN_DIVISION_C];

		double m_dDiviScore[_CNT_PATTERN_PATH][_CNT_PATTERN_SCORE];

		BOOL m_bUseCharacter;

		BOOL m_bDetailSearch[_CNT_PATTERN_PATH];
		RECT m_rtModelRect[_CNT_PATTERN_PATH];
		CString m_sSimilarPartList[_CNT_PATTERN_SIMILAR];

		double m_factor_blue;
		double m_factor_green;
		double m_factor_red;

		int m_ExtraImageLightIndex;

		int m_nModelFilter;

		_tagAlgoPattern()
		{
			m_bUsePolarity = FALSE;
			m_dAcceptScore = 0.0;

			m_dWndAngle = 0.0;

			memset(m_dStdCogX, 0, sizeof(double) * _CNT_PATTERN_PATH);
			memset(m_dStdCogY, 0, sizeof(double) * _CNT_PATTERN_PATH);
			m_dStdScore = 0.0;

			m_bShiftIsUse = TRUE;
			m_dShiftX = 1.0;
			m_dShiftY = 1.0;
			m_dRangeAngle = 10.0;

			m_dSearchAngleRange_Max = 0.0;
			m_dSearchAngleRange_Min = 0.0;
			m_nCntPatternPath = 0;

			memset(m_nDiviLnR, 0, sizeof(int) * _CNT_PATTERN_PATH);
			memset(m_nDiviLnC, 0, sizeof(int) * _CNT_PATTERN_PATH);

			memset(m_dGapLnR, 0, sizeof(double) * _CNT_PATTERN_PATH * _CNT_PATTERN_DIVISION_R);
			memset(m_dGapLnC, 0, sizeof(double) * _CNT_PATTERN_PATH * _CNT_PATTERN_DIVISION_C);

			memset(m_dDiviScore, 0, sizeof(double) * _CNT_PATTERN_PATH * _CNT_PATTERN_SCORE);

			m_bUseCharacter = FALSE;

			memset(m_bDetailSearch, 0, sizeof(BOOL) * _CNT_PATTERN_PATH);
			memset(m_rtModelRect, 0, sizeof(RECT) * _CNT_PATTERN_PATH);
		}

// 		void GetParam(TeachParamPattern *pParamTeach)
// 		{
// 			if (!pParamTeach)
// 				return;
// 
// 			pParamTeach->wndAngle = m_dWndAngle;
// 			pParamTeach->searchAngleRange_Max = m_dSearchAngleRange_Max;
// 			pParamTeach->searchAngleRange_Min = m_dSearchAngleRange_Min;
// 			_tcscpy_s(pParamTeach->modelPath, m_sPathModelTeach);
// 			pParamTeach->SamplingAngle = m_SamplingAngle;
// 			pParamTeach->m_bUseNGOpt = m_bUseNGOpt;
// 		}
// 
// 		void SetParam(const TeachParamPattern *pParamTeach)
// 		{
// 			m_dWndAngle = pParamTeach->retAngle;
// 			m_dStdScore = pParamTeach->retScore;
// 
// 			m_dRetPosX = pParamTeach->retPosX;
// 			m_dRetPosY = pParamTeach->retPosY;
// 			//m_dRetCogX = m_dStdCogX = pParamTeach->retCogX;
// 			//m_dRetCogY = m_dStdCogY = pParamTeach->retCogY;
// 		}

// 		void GetParam(InspParamPattern *pParamInsp)
// 		{
// 			pParamInsp->wndAngle = m_dWndAngle;
// 			memcpy(pParamInsp->stdCogX, m_dStdCogX, sizeof(double) * _CNT_PATTERN_PATH);
// 			memcpy(pParamInsp->stdCogY, m_dStdCogY, sizeof(double) * _CNT_PATTERN_PATH);
// 			pParamInsp->stdScore = m_dAcceptScore;
// 			pParamInsp->offsetRange_x = m_dShiftX;
// 			pParamInsp->offsetRange_y = m_dShiftY;
// 			pParamInsp->angleRange = m_dRangeAngle;
// 			pParamInsp->isPolarity = m_bUsePolarity;
// 			pParamInsp->bUseOffset = m_bShiftIsUse;
// 			pParamInsp->searchAngleRange_Max = m_dSearchAngleRange_Max;
// 			pParamInsp->searchAngleRange_Min = m_dSearchAngleRange_Min;
// 
// 			memcpy(pParamInsp->DiviLnR, m_nDiviLnR, sizeof(int) * _CNT_PATTERN_PATH);
// 			memcpy(pParamInsp->DiviLnC, m_nDiviLnC, sizeof(int) * _CNT_PATTERN_PATH);
// 
// 			memcpy(pParamInsp->GapLnR, m_dGapLnR, sizeof(double) * _CNT_PATTERN_PATH * _CNT_PATTERN_DIVISION_R);
// 			memcpy(pParamInsp->GapLnC, m_dGapLnC, sizeof(double) * _CNT_PATTERN_PATH * _CNT_PATTERN_DIVISION_C);
// 
// 			memcpy(pParamInsp->DiviScore, m_dDiviScore, sizeof(double) * _CNT_PATTERN_PATH * CNT_PATTERN_SCORE);
// 
// 			pParamInsp->UseCharacter = m_bUseCharacter;
// 
// 			memcpy(pParamInsp->DetailSearch, m_bDetailSearch, sizeof(BOOL) * _CNT_PATTERN_PATH);
// 			memcpy(pParamInsp->ModelRect, m_rtModelRect, sizeof(RECT) * _CNT_PATTERN_PATH);
// 
// 			for (int i = 0; i < _CNT_PATTERN_SIMILAR; i++)
// 			{
// 				pParamInsp->similarParts[i].Format(_T("%s"), m_sSimilarPartList[i]);
// 
// 				pParamInsp->m_factor_blue = m_factor_blue;
// 				pParamInsp->m_factor_green = m_factor_green;
// 				pParamInsp->m_factor_red = m_factor_red;
// 			}
// 		}
	}_AlgoPattern;

	typedef struct _tagRstAlgoPattern
	{
		double score;
		double angle;
		double cogX;
		double cogY;
		double offsetX;
		double offsetY;
		BOOL isReverse;

		// OK 판정 //
		BOOL m_bOKFind;
		BOOL m_bOKScore;
		BOOL m_bOKAngle;
		BOOL m_bOKOffsetX;
		BOOL m_bOKOffsetY;
		BOOL m_bOKPolarity;

		int m_nModelNum;
		int m_nDivisionNum;
		POINTF m_ptModelPos;
		double m_dDisiScore[_CNT_PATTERN_SCORE];

		wchar_t m_SimilarPartCode[_MAX_STRLEN] = {0};
		double m_dRstSimilarScore;

		_tagRstAlgoPattern()
		{
			score = 0.;
			angle = 0.;
			cogX = 0;
			cogY = 0;
			offsetX = 0;
			offsetY = 0;

			memset(m_dDisiScore, 0, sizeof(double) * _CNT_PATTERN_SCORE);
			m_dRstSimilarScore = 0.0;
		}
	}_RstAlgoPattern;


	typedef struct _tagAlgoSolderCone
	{
		double m_dHeightLv1;
		double m_dHeightLv2;
		double m_dHeightLv3;
		double m_dTeachHeight;

		BOOL m_bUseSolderHighestVolumeDiff;
		BOOL m_bUseInspVolumeDiff;		//VolumeDiff 검사 진행 여부
		BOOL m_bUseInspVolumeMinLength;		//VolumeDiff 검사 시, Min 길이 검사 유무
		BOOL m_bUseInspVolumePie;		//VolumePie 검사 진행 여부

		double m_dHighestHeight;		//Volume 최고 높이
		double m_dGapH;					//Volume 최고/최저 높이로 부터 gap 만큼의 높이 (50um)
		int m_nLevelCounts;				//VolumeDiff 검사 구간 개수

		float m_fPieHeightMaxDiff;

		float m_fArrVolumeDiff[SOLDERVOL_LEVEL_COUNT][_eMMD_Total];
		BOOL m_bArrUseVolumeDiffInsp[SOLDERVOL_LEVEL_COUNT];
		BOOL m_bArrUseVolumeMinLengthInsp[SOLDERVOL_LEVEL_COUNT];

		_tagAlgoSolderCone()
		{
			m_dHeightLv1 = 0.0;
			m_dHeightLv2 = 0.0;
			m_dHeightLv3 = 0.0;
			m_dTeachHeight = 0.0;

			m_bUseSolderHighestVolumeDiff = false;
			m_bUseInspVolumeDiff = false;
			m_bUseInspVolumeMinLength = false;
			m_bUseInspVolumePie = false;

			m_dHighestHeight = 0.0;
			m_dGapH = 50.0;
			m_nLevelCounts = 1;

			m_fPieHeightMaxDiff = 0.0f;

			for (int a = 0; a < SOLDERVOL_LEVEL_COUNT; a++)
			{
				for (int b = 0; b < _eMMD_Total; b++)
				{
					m_fArrVolumeDiff[a][b] = 0;
				}
			}

			for (int a = 0; a < SOLDERVOL_LEVEL_COUNT; a++)
			{
				m_bArrUseVolumeDiffInsp[a] = false;

			}

			for (int a = 0; a < SOLDERVOL_LEVEL_COUNT; a++)
			{
				m_bArrUseVolumeMinLengthInsp[a] = false;

			}
		}
	}_AlgoSolderCone;

	typedef struct _tagRstAlgoSolderCone
	{
		double m_dRstAreaLv1;
		double m_dRstAreaLv2;
		double m_dRstAreaLv3;

		float m_fArrRstVolumeDiff[SOLDERVOL_LEVEL_COUNT];	//VolumeDiff 검사 결과 값
		float m_fArrRstVolumeMinLength[SOLDERVOL_LEVEL_COUNT];	//VolumeMinLength 검사 결과 값
		//float m_fRstVolumeH;	//VolumeMinLength 검사 결과 값

		float m_fRstMaxDiffHeightInPie;        //Pie 영역 높이차이 검사결과

		BOOL m_bOKVolumePie;		//VolumePie 검사 판정 값

		BOOL m_bArrOKVolumeDiff[SOLDERVOL_LEVEL_COUNT];		//VolumeDiff 검사 판정 값
		BOOL m_bArrOKVolumeMinLength[SOLDERVOL_LEVEL_COUNT];		//VolumeMinLength 검사 판정 값

		POINTF  m_sPoint;									//VolumeDiff 최상단-gap 중심 좌표
		POINT reLongAxisPoinSt[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 장축의 시작좌표
		POINT reLongAxisPoinEd[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 장축의 끝좌표
		POINT reShortAxisPoinSt[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 단축의 시작좌표
		POINT reShortAxisPoinEd[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 단축의 끝좌표

		RECT m_rcRect_I = { 0 };		//Insp ROI
		RECT m_rcRect_I_NGArea = { 0 };		//Insp NG ROI

		_tagRstAlgoSolderCone()
		{
			m_dRstAreaLv1 = 0.0;
			m_dRstAreaLv2 = 0.0;
			m_dRstAreaLv3 = 0.0;

			memset(m_fArrRstVolumeDiff, 0, sizeof(float)*		SOLDERVOL_LEVEL_COUNT);
			memset(m_fArrRstVolumeMinLength, 0, sizeof(float)*		SOLDERVOL_LEVEL_COUNT);

			//m_fRstVolumeH = 0.0f;
			m_fRstMaxDiffHeightInPie = 0.0f;

			m_bOKVolumePie = 0;
			memset(m_bArrOKVolumeDiff, 0, sizeof(BOOL) *		SOLDERVOL_LEVEL_COUNT);
			memset(m_bArrOKVolumeMinLength, 0, sizeof(BOOL) *		SOLDERVOL_LEVEL_COUNT);

			m_sPoint.x = 0.0f;
			m_sPoint.y = 0.0f;
			memset(reLongAxisPoinSt, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);
			memset(reLongAxisPoinEd, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);
			memset(reShortAxisPoinSt, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);
			memset(reShortAxisPoinEd, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);

		}
	}_RstAlgoSolderCone;

#pragma region _PatternDiff Parameter_

	typedef struct _tagLayerPolygon
	{
		int m_nAreaCnt;
		_stPoly m_nPoly[_MAX_POLYGROUPCNT];

		void LayerPolygon()
		{
			m_nAreaCnt = 0;
		}

	}_LayerPolygon;
	typedef struct _tagHistoParamBase
	{
		int m_nArrValue[(int)_eHistoTotal];

		_tagHistoParamBase()
		{
			memset(m_nArrValue, 0, (int)_eHistoTotal * sizeof(int));
		}
	}_HistoParamBase;
	typedef struct _tagModelPath
	{
		wchar_t m_sModelPath1[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath2[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath3[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath4[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath5[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath6[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath7[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath8[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath9[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath10[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath11[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath12[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath13[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath14[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath15[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath16[MAX_STRLEN] = { 0 };

		_tagModelPath()
		{
		}

		CString GetModelPath(int nModelIdx)
		{
			CString str = _T("");

			if (nModelIdx >= _MAX_MODELCNT)
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
	}_ModelPath;
	typedef struct _tagAlgoPatternDiff
	{
		int m_nLayerCnt;
		struct _tagLayerPolygon m_stPolygon[_MAX_LAYERCNT];
		int m_nArrLayerType[_MAX_LAYERCNT];
		struct _tagAlgoBlobBase m_stBlobBase[_MAX_LAYERCNT];
		double m_dTeachLength[_MAX_LAYERCNT];
		double m_dTeachWidth[_MAX_LAYERCNT];
		double m_dTeachArea[_MAX_LAYERCNT];
		BOOL m_bUsePattern;
		RECT m_rcMatchingArea;
		int m_nModelAddCnt;
		wchar_t m_sModelPath[_MAX_STRLEN];
		double m_dTheta;
		BOOL m_bUseDarkForeign;
		BOOL m_bUseFilter[_MAX_LAYERCNT];
		BOOL m_bNGBlobMode[_MAX_LAYERCNT];
		struct _tagHistoParamBase m_stHistoBase[_MAX_LAYERCNT];
		wchar_t m_sModelPath1[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath2[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath3[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath4[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath5[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath6[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath7[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath8[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath9[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath10[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath11[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath12[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath13[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath14[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath15[MAX_STRLEN] = { 0 };
		wchar_t m_sModelPath16[MAX_STRLEN] = { 0 };
		int nSelectedModel;					//OK model index Before Auto InspSeq. If Teaching Seq, This Param must 0.		
		BOOL m_bUseSizeAnd[_MAX_LAYERCNT];

		//ExceptPattern for Chipping
		BOOL m_bUseExcPattern;
		int m_nExcModelAddCnt;
		int m_nExcPatternScore;

		_tagModelPath m_stExcPath;

		_lightData m_stArrLightData[_MAX_LAYERCNT];
		BOOL m_bArrUseMergeLayer[_MAX_LAYERCNT];
		BOOL m_bUseEdgeFilter[_MAX_LAYERCNT];
		int m_nFilterSize[_MAX_LAYERCNT];
		int m_nEdgeFilterLevel[_MAX_LAYERCNT];

		RECT m_rcArrAlignArea[_MAX_ALIGNCNT];
		BOOL m_bUseArrAlignArea;
		int m_nAcceptAlignScore;

		BOOL m_bPatternMatching; //매칭검사여부결정
		POINTF ptMatchingCenter;

		BOOL m_bUseMaskBW[_MAX_LAYERCNT];
		struct _tagAlgoBlobBase m_stBlobBase_Mask;
		BOOL m_UseFilter_Mask;
		BOOL m_UseEdgeFilter_Mask;
		int m_nFilterSize_Mask;
		int m_nEdgeFilterLevel_Mask;
		double m_dTeachProbeDefectSize[_MAX_LAYERCNT];
		double m_dTeachProbeMarkSize[_MAX_LAYERCNT];
		BOOL m_bUseModelAllMatching;
		BOOL m_bUseRelativeHeight[_MAX_LAYERCNT];
		int m_nTeachRelativeHeightMin[_MAX_LAYERCNT];
		int m_nTeachRelativeHeightMax[_MAX_LAYERCNT];
		_tagAlgoPatternDiff()
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
			nSelectedModel = 0;
			m_nAcceptAlignScore = 80;

			memset(m_stPolygon, 0, _MAX_LAYERCNT * sizeof(_tagLayerPolygon));
			memset(m_nArrLayerType, 0, _MAX_LAYERCNT * sizeof(int));
			memset(m_stBlobBase, 0, _MAX_LAYERCNT * sizeof(_tagAlgoBlobBase));
			memset(m_dTeachLength, 0, _MAX_LAYERCNT * sizeof(double));
			memset(m_dTeachWidth, 0, _MAX_LAYERCNT * sizeof(double));
			memset(m_dTeachArea, 0, _MAX_LAYERCNT * sizeof(double));
			memset(m_bUseFilter, 0, _MAX_LAYERCNT * sizeof(BOOL));
			memset(m_bNGBlobMode, 0, _MAX_LAYERCNT * sizeof(BOOL));
			memset(m_stHistoBase, 0, _MAX_LAYERCNT * sizeof(_tagHistoParamBase));
			memset(m_bUseSizeAnd, 0, _MAX_LAYERCNT * sizeof(BOOL));
			memset(&m_stArrLightData, 0, _MAX_LAYERCNT * sizeof(_lightData));
			memset(&m_bArrUseMergeLayer, 0, _MAX_LAYERCNT * sizeof(BOOL));
			memset(m_bUseEdgeFilter, 0, _MAX_LAYERCNT * sizeof(BOOL));
			memset(m_nFilterSize, 0, _MAX_LAYERCNT * sizeof(int));
			memset(m_nEdgeFilterLevel, 0, _MAX_LAYERCNT * sizeof(int));
			memset(m_bUseMaskBW, 0, _MAX_LAYERCNT * sizeof(BOOL));

			m_bPatternMatching = true;
			ptMatchingCenter.x = 0.0f;
			ptMatchingCenter.y = 0.0f;

			m_UseFilter_Mask = FALSE;
			m_UseEdgeFilter_Mask = FALSE;
			m_nFilterSize_Mask = 0;
			m_nEdgeFilterLevel_Mask = 0;
			memset(&m_stBlobBase_Mask, 0, sizeof(_tagAlgoBlobBase));
			memset(m_dTeachProbeDefectSize, 0, _MAX_LAYERCNT * sizeof(double));
			memset(m_dTeachProbeMarkSize, 0, _MAX_LAYERCNT * sizeof(double));
			m_bUseModelAllMatching = FALSE;
			memset(m_bUseRelativeHeight, 0, _MAX_LAYERCNT * sizeof(BOOL));
			memset(m_nTeachRelativeHeightMin, 0, _MAX_LAYERCNT * sizeof(int));
			memset(m_nTeachRelativeHeightMax, 0, _MAX_LAYERCNT * sizeof(int));
		}

		CString GetModelPath(int nModelIdx)
		{
			CString str = _T("");

			if (nModelIdx >= this->m_nModelAddCnt || nModelIdx >= _MAX_MODELCNT)
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

	}_AlgoPatternDiff;

	typedef struct _tagRstAlgoPatternDiff // PatternDiff 검사 결과
	{
		BOOL m_bOK;
		int m_nRectCnt;

		POINTF m_ptArrRstPtr[_BLOB_RECT_CNTS][4];
		BOOL m_bArrRstLayer[_MAX_LAYERCNT];
		double m_dRstLength[_BLOB_RECT_CNTS];
		double m_dRstWidth[_BLOB_RECT_CNTS];
		double m_dRstArea[_BLOB_RECT_CNTS];
		int m_nArrDefectLayerNum[_BLOB_RECT_CNTS];
		double m_dTheta;
		int m_nSelectedModelIdx;
		BOOL m_bRstChipping;
		double m_dRstPatternScore[_BLOB_RECT_CNTS];
		POINTF m_ptRstShiftXY;
		POINTF m_ptRstMatchingCenter;
		double m_dAlignScore;
		double m_dMatchingScore[_MAX_ALIGNCNT];
		BOOL m_bMatchingOK;
		float m_fArrRstRelativeHeightInspMin[_BLOB_RECT_CNTS];
		float m_fArrRstRelativeHeightInspMax[_BLOB_RECT_CNTS];

		_tagRstAlgoPatternDiff()
		{
			m_bOK = FALSE;
			m_nRectCnt = 0;
			m_dTheta = 0.0;
			m_ptRstShiftXY.x = 0;
			m_ptRstShiftXY.y = 0;

			memset(m_ptArrRstPtr, 0, _BLOB_RECT_CNTS * 4 * sizeof(POINTF));
			memset(m_bArrRstLayer, 0, _MAX_LAYERCNT * sizeof(BOOL));
			memset(m_dRstLength, 0, _BLOB_RECT_CNTS * sizeof(double));
			memset(m_dRstWidth, 0, _BLOB_RECT_CNTS * sizeof(double));
			memset(m_dRstArea, 0, _BLOB_RECT_CNTS * sizeof(double));
			memset(m_nArrDefectLayerNum, 0, _BLOB_RECT_CNTS * sizeof(int));
			memset(m_dRstPatternScore, 0, _BLOB_RECT_CNTS * sizeof(double));

			m_nSelectedModelIdx = 0;
			m_bRstChipping = TRUE;

			m_ptRstMatchingCenter.x = 0;
			m_ptRstMatchingCenter.y = 0;

			m_dAlignScore = 0.;
			memset(m_dMatchingScore, 0, _MAX_ALIGNCNT * sizeof(double));
			m_bMatchingOK = FALSE;
			memset(m_fArrRstRelativeHeightInspMin, 0, _BLOB_RECT_CNTS * sizeof(float));
			memset(m_fArrRstRelativeHeightInspMax, 0, _BLOB_RECT_CNTS * sizeof(float));
		}
	}_RstAlgoPatternDiff;
	typedef struct _tagNGBlob_AISaveParam
	{
		BOOL bSaveAITeachImg;
		int nPartNo;
		int nPartID;
		int nModuleID;
		int nWindowID;
		int nAlgoID;
		wchar_t* sCurTime[_MAX_STRLEN];

		_tagNGBlob_AISaveParam()
		{
			bSaveAITeachImg = FALSE;
			nPartNo = 0;
			nPartID = 1;
			nModuleID = 1;
			nWindowID = 1;
			nAlgoID = 1;
			memset(sCurTime, 0, sizeof(wchar_t) * _MAX_STRLEN);
		}
	}_NGBlob_AISaveParam;
#pragma endregion _PatternDiff Parameter_

	typedef struct tagFeaturedPnt
	{
		POINTF point;	// 좌표
		double cosVal;	// cos 값
		int sinSign;	// sin 값의 부호 (-1 / 0 / 1) -> sin 절댓값이 0.1보다 작으면 0으로 한다.

		tagFeaturedPnt() : point(POINTF()), cosVal(0), sinSign(0) {}
		tagFeaturedPnt(const POINTF &pt)
			: point(pt), cosVal(0), sinSign(0) {}
		tagFeaturedPnt(const POINTF &pt, const double& cV)
			: point(pt), cosVal(cV), sinSign(0) {}
		tagFeaturedPnt(const float &x, const float& y)
			: cosVal(0), sinSign(0)
		{
			point.x = x;
			point.y = y;
		}
		tagFeaturedPnt(const float &x, const float& y, const double& cV)
			: cosVal(cV), sinSign(0)
		{
			point.x = x;
			point.y = y;
		}
	} FeaturedPoint;
#pragma region _ShapeX Parameter_

// sjb+c 250411.[1] ~PInspalgo_Def.h~
#define _nShapeXCnt 200
//#define _nShapeXCnt 600
// +csjb 250411.[1] 

#define _nShapeXModelCnt 10
	enum _eShapeXBin
	{
		ShapeXArea = 0,
		Model,
		NGImg,
		NGImg2,
		Insp,
		Result,
		Total,
	};
	enum _SHAPEX_OPTION
	{
		MinScarThick = 0x01,
		aspect_ratio = 0x02,
		CrossLine_Detect = 0x04,
		VerticalMaxLength = 0x08,
		HorizontalMaxLength = 0x10,
		MaxNGArea = 0x20,
		ChippingCriticalLine = 0x40,
		TieBarRateCheck = 0x80,
		DeleteExitArea =0x100,
		InspExitInnerArea = 0x200,
		TabLineChipping = 0x400,
		NonMatchingMode = 0x800,
		TabIncludeSide=0x1000,
		WrForeignCnt = 0x2000,
		WrForeignWidth = 0x4000,
		WrForeignLength = 0x8000,
		NGGrouping=0x10000,
		UseContrast=0x20000,
		UseSwapX_Y = 0x40000,
		UseNG1SubtNG2 = 0x80000,
		UseStripeCorrection = 0x100000,
		UseGradationFilter = 0x200000,
		UseCurrentShape = 0x400000,      // CurrentShape 모드: 마스크 기반 검사
		UseInvMatching = 0x800000,
		UseOutLineSecond = 0x1000000,    // 외곽선 검사 2회 실시 여부
		UseAngleProfiling = 0x2000000,        // 각도 프로파일링 검사 실시 여부
		UseNonCriticalExist = 0x4000000,      // Non-Critical Exist 검사 실시 여부
		UseLineChippingSSD = 0x8000000,      // Line Chipping SSD 중간부분 chipping검사 생략
		   // CurrentShape 마스크 생성 여부
	};

	// CurrentShape 16bit 마스크의 비트 정의 (CV_16UC1)
	enum _SHAPEX_MASK_BIT
	{
		MaskBit_Contact		= 0x0001, // contact
		MaskBit_Gap			= 0x0002, // black outline
		MaskBit_OutPlane	= 0x0004, // plane
		//MaskBit_Rect		= 0x0008, // rect
	};

	struct _SaveShapeX3D
	{
		bool bSaved;
		int nModelIdx;
		CString sPath;
		_SaveShapeX3D()
		{
			bSaved = false;
			int nModelIdx = 0;
			CString sPath = _T("");
		}
	};
	struct _ShapeXROI
	{
	public:
		RECT ROI = { 0 };      //윈도우 이미지 내부의 알고리즘의 검사 ROI
		POINT pCt = { 0 };     //윈도우 이미지 내부의 pad(tab)의 center좌표
		BOOL bUseShape;     //형상검사 (형상과 현재 이진화를 차연산하여 Area측정)
		BOOL bUseInner;     //내부검사
		BOOL bUseExist;     //존재유무검사 true존재영역에있는 부분 검사: false 존재영역에 있는 부분 don`t care
		BOOL bUseShift;     //shift검사
		
		float fShapeArea;       //형상검사 OK Area기준
		float fInnerArea;       //내부검사 OK Area기준
		float fExistArea;       //존재유무 OK Area기준
		float fShiftX;           //shiftX 허용량(mm단위)
		float fShiftY;           //shiftY 허용량(mm단위)		
		int nModelCnt;
		int ArrModel[_nShapeXModelCnt] = { 0 };  //사용 할 File상 Model Index
        //차후 추가 변수
		BOOL bUseExistShape;
		float fExistShapeArea;

		// sjb++ 250408.[24] ~Pemto/PInslapgo_Def.h~
		float fLocalLength;
		float fLocalLength2;
		float fDent;
		float fDentMin;
		float fDentMax;
		BOOL bUseLocalLength;
		BOOL bUseDent;

		BOOL bUseForeignWidth;
		BOOL bUseForeignLength;
		float fForeignWidth;
		float fForeignLength;

		BOOL bUseForeignWLAnd;

		_ShapeXROI()
		{
			bUseShape = false;
			bUseInner = false;
			bUseExist = false;
			bUseShift = false;
			bUseForeignWidth = false;
			bUseForeignLength = false;
			fShapeArea = 0.0f;
			fInnerArea = 0.0f;
			fExistArea = 0.0f;
			fForeignWidth = 0.025f;
			fForeignLength = 0.025f;
			fShiftX = 0.0f;
			fShiftY = 0.0f;
			nModelCnt = 0;
			bUseExistShape = false;
			fExistShapeArea = 0.0f;
			fLocalLength = 0.0f;
			fLocalLength2 = 0.0f;
			fDent = 0.0f;
			fDentMin = 0.0f;
			fDentMax = 0.0f;
			bUseLocalLength = false;
			bUseDent = false;
			bUseForeignWidth = false;
			bUseForeignLength = false;
			fForeignWidth = 0.025f;
			fForeignLength = 0.025f;
			bUseForeignWLAnd = false;
		}
		// esjb++ 250408.[24]

	};
	typedef struct _tagAlgoShapeX
	{
	public:
		double dPartangle;
		int m_nLeadTipDirection;        // Lead 방향

		// sjb++ 250403.[42] ~PInspalgo_Def.h~
		int m_nTypeSelectBlob;
		int m_nTypeSelectTarget;
		// esjb++ 250403.[42]

		short ROICnt;

		short MatchSc;
		short nHist1;
		short nHistLow1;
		short nHistUp1;
		short nHist2;
		short nHistLow2;
		short nHistUp2;
		struct _tagAlgoBlobBase m_sBlobBase_ShapeArea;
		struct _tagAlgoBlobBase m_sBlobBase_NG;
		struct _tagAlgoBlobBase m_sBlobBase_NG2;
		wchar_t ArrPathModel[_MAX_STRLEN] = { 0 };
		_ShapeXROI spROI[_nShapeXCnt];		
		int nInspOption;

		float Aspectratio;
		float MinScarThickness;
		float Verticalmaxlength;
		float Horizontalmaxlength;
		float MaxNGArea;
		float CriticalArea;
		float MaxChippingLength;
		int nStartIdx;
		float fTieBarRate;
		short WrForeignCnt;
		float WrForeignWidth;
		float WrForeignLength;

		float NGGroupingMaxSize;
		float NGGroupingDistance;
		int ContrastValue;
		float Check3DMaxVal;
		float Check3DMinVal;

		bool CheckSearchByShape;

		int nWndId;

		enum _ETypeGep
		{
			_eShapeX,
			_eShapeXSpace,
			_eColor
		};
		_tagAlgoShapeX()
		{
			m_nTypeSelectBlob = _ETypeBlob::_eSelectBigger;
			m_nTypeSelectTarget = 0;

			Aspectratio = 0.f;
			CriticalArea = 0.f;
			Horizontalmaxlength = 0.f;
			MatchSc = 0;
			MaxChippingLength = 0.f;
			MaxNGArea = 0.f;
			MinScarThickness = 0.f;
			ROICnt = 0;
			Verticalmaxlength = 0.;
			dPartangle = 0.;
			fTieBarRate = 0.f;
			WrForeignCnt = 0;
			WrForeignWidth = 0.f;
			WrForeignLength = 0.f;
			NGGroupingMaxSize = 0.f;
			NGGroupingDistance = 0.f;
			ContrastValue = 0;
			m_nLeadTipDirection = 0;
			nHist1=0;
			nHistLow1 = 0;
			nHistUp1 = 0;
			nHist2 = 0;
			nHistLow2 = 0;
			nHistUp2 = 0;
			nInspOption = 0;
			nStartIdx = 0;
			Check3DMaxVal = 0.1f; 
			Check3DMinVal = 0.0f;
			CheckSearchByShape = false;
			nWndId = 0;
		}

		BOOL IsUseMultiArea()
		{
			return TRUE;
		}
	}_AlgoShapeX;

	typedef struct _tagRstAlgoShapeX	// ShapeX 검사 결과
	{
	public:
		BOOL isinsp;//검사여부
		BOOL isOK;//Algo ok
		
		short nROICnt;      //shapePadROI개수
		short nNGAreaRoiCnt;//NG blob개수
		int nMatchingIdx[_nShapeXCnt];    //Matching Model Index;
		float nMatchingSc[_nShapeXCnt];   //Matching score
		//BOOL m_bUseShape[_nShapeXCnt];	//형상검사사용
		//BOOL m_bUseInner[_nShapeXCnt];	//내부검사사용
		//BOOL m_bUseExist[_nShapeXCnt];	//존재유무검사사용
		BOOL ArrROIOK[_nShapeXCnt];     //ROI검사결과
		BOOL ArrOKShape[_nShapeXCnt];   //형상검사결과
		BOOL ArrOKInner[_nShapeXCnt];   //내부검사결과
		BOOL ArrOKExist[_nShapeXCnt];   //존재유무검사결과
		BOOL ArrOKShift[_nShapeXCnt];   //shift검사결과
		BOOL ArrOKForeignW[_nShapeXCnt];//이물단축검사결과
		BOOL ArrOKForeignL[_nShapeXCnt];//이물장축검사결과
		float ArrShapeArea[_nShapeXCnt];//형상검사 NG Area
		float ArrInnerArea[_nShapeXCnt];//내부검사 NG Area
		float ArrExistArea[_nShapeXCnt];//존재유무 NG Area
		float ArrShiftX[_nShapeXCnt];    //shift 값
		float ArrShiftY[_nShapeXCnt];    //shift 값
		float ArrForeignW[_nShapeXCnt];  //이물단축 NG mm
		float ArrForeignL[_nShapeXCnt];  //이물장축 NG mm
		RECT rcArrRect[_nShapeXCnt];	//현재 shpaeArea의 Rect
		int m_rcArrNGArrRectCnt[_nShapeXCnt];   //ROI별 NG개수
		RECT rcArrNGRect[_nShapeXCnt];	//NG위치
		int m_nArrMaxFreqValue1[_nShapeXCnt];   //foreign1 평균
		int m_nArrMaxFreqValue2[_nShapeXCnt];   //foreign2 평균

		BOOL m_bUseExistShape[_nShapeXCnt];	//존재검사영역 형상검사사용
		BOOL ArrOKExistShape[_nShapeXCnt];   //존재검사영역 형상검사결과
		float ArrExistShapeArea[_nShapeXCnt];//존재검사영역 형상검사 NG Area

		// sjb++ 250408.[21] ~PInspalgo_Def.h~
		float ArrLocalLength[_nShapeXCnt];
		float ArrDent[_nShapeXCnt];
		float ArrDent2[_nShapeXCnt];
		BOOL ArrOKLocalLength[_nShapeXCnt];
		BOOL ArrOKDent[_nShapeXCnt];
		// esjb++

		POINTF poArrCenter[_nShapeXCnt];	//shpaeArea 내부의 Matching Center
		BOOL bMatchOK[_nShapeXCnt];   //Matching OK
		
		BOOL m_bUseShape[_nShapeXCnt];	//형상검사사용
		BOOL m_bUseInner[_nShapeXCnt];	//내부검사사용
		BOOL m_bUseExist[_nShapeXCnt];	//존재유무검사사용

		POINTF ArrStartJointP[_nShapeXCnt];
		POINTF ArrEndJointP[_nShapeXCnt];

		BOOL RstOKWrForeignCnt;             // Warning Foreign Count 결과
		BOOL ArrOKWrForeignW[_nShapeXCnt];  // Warning Foreign 단축검사결과
		BOOL ArrOKWrForeignL[_nShapeXCnt];  // Warning Foreign 장축검사결과
		short RstWrForeignCnt;              // Warning Foreign Count 값 (Total)
		short ArrWrForeignCnt[_nShapeXCnt]; // Warning Foreing Count 값 (Roi)
		float ArrWrForeignW[_nShapeXCnt];   // Warning Foreign 단축 값
		float ArrWrForeignL[_nShapeXCnt];   // Warning Foreign 장축 값

		// 추가: 형상용 배열 (Shape 전용 Rect 및 NG 개수)
		short nShapeNGCnt;//Shape NG blob개수
		RECT rcArrShapeRect[_nShapeXCnt];   // 형상검사용 Rect
		int m_rcArrShapeNGArrRectCnt[_nShapeXCnt]; // 형상검사용 ROI별 NG개수
		short StripeRectCnt;//Stripe Rect개수
		RECT rcArrStripeRect[_nShapeXCnt];	//현재 shpaeArea의 Stripe 영역 위치

		_tagRstAlgoShapeX()
		{
			isOK = RstOKWrForeignCnt =  FALSE;
			nNGAreaRoiCnt = nROICnt = RstWrForeignCnt = 0;
			memset(nMatchingIdx, -1, sizeof(int) * _nShapeXCnt);
			memset(nMatchingSc, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrROIOK, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKShape, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKInner, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKExist, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKShift, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKForeignW, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKForeignL, 0, sizeof(BOOL) * _nShapeXCnt);

			memset(ArrShapeArea, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrInnerArea, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrExistArea, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrShiftX, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrShiftY, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrForeignW, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrForeignL, 0, sizeof(float) * _nShapeXCnt);

			memset(rcArrRect, 0, sizeof(RECT) * _nShapeXCnt);
			memset(m_rcArrNGArrRectCnt, 0, sizeof(int) * _nShapeXCnt);
			memset(rcArrNGRect, 0, sizeof(RECT) * _nShapeXCnt);
			memset(m_nArrMaxFreqValue1, 0, sizeof(int) * _nShapeXCnt);
			memset(m_nArrMaxFreqValue2, 0, sizeof(int) * _nShapeXCnt);

			//추가변수
			memset(ArrOKExistShape, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(m_bUseExistShape, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrExistShapeArea, 0, sizeof(float) * _nShapeXCnt);

			// sjb++ 250408.[22] ~PInspalgo_Def.h~
			memset(ArrLocalLength, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrDent, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrDent2, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrOKLocalLength, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKDent, 0, sizeof(BOOL) * _nShapeXCnt);
			// esjb++ 250408.[22]

			memset(poArrCenter, 0, sizeof(POINTF) * _nShapeXCnt);
			memset(bMatchOK, 0, sizeof(BOOL) * _nShapeXCnt);

			isinsp = FALSE;
			memset(m_bUseExist, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(m_bUseInner, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(m_bUseShape, 0, sizeof(BOOL) * _nShapeXCnt);

			memset(ArrStartJointP, 0, sizeof(POINTF) * _nShapeXCnt);
			memset(ArrEndJointP, 0, sizeof(POINTF) * _nShapeXCnt);

			memset(ArrOKWrForeignW, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrOKWrForeignL, 0, sizeof(BOOL) * _nShapeXCnt);
			memset(ArrWrForeignCnt, 0, sizeof(short) * _nShapeXCnt);
			memset(ArrWrForeignW, 0, sizeof(float) * _nShapeXCnt);
			memset(ArrWrForeignL, 0, sizeof(float) * _nShapeXCnt);

			// 추가: 형상용 배열 초기화
			nShapeNGCnt = 0;
			memset(rcArrShapeRect, 0, sizeof(RECT) * _nShapeXCnt);
			memset(m_rcArrShapeNGArrRectCnt, 0, sizeof(int) * _nShapeXCnt);

			StripeRectCnt = 0;
			memset(rcArrStripeRect, 0, sizeof(RECT) * _nShapeXCnt);
		}
	}_RstAlgoShapeX;

	typedef struct _tagsShapeXList
	{
		wchar_t m_sFilePath[MAX_STRLEN];
	}_sShapeXList;
#pragma endregion _ShapeX Parameter_
}
