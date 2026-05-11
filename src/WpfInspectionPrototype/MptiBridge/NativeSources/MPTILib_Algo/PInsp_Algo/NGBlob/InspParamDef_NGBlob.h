#pragma once
#include "../InspParamDef_AlgoBase.h"

#define	_TiltRectCnt		4
enum m_eNGBlob_by
{
	NGBlob_by_Data = 0,
	NGBlob_by_Filter,
	NGBlob_by_Min2D,
	NGBlob_by_Max2D,
	NGBlob_by_Range2D,
	NGBlob_by_Range3D,
	NGBlob_by_inspType,
	NGBlob_by_inspArea,
	NGBlob_by_WarpageOption,
	NGBlob_by_ExceptPattern,
	NGBlob_by_ExceptPatternScore,
	NGBlob_by_BallRangeMin,  //0~100
	NGBlob_by_BallRangeMax,   //0 ~100
	NGBlob_by_HistoMin2D, //NotUSe. To Float
	NGBlob_by_HistoMax2D, //NotUSe. To Float
	NGBlob_by_HistoLimitMin,
	NGBlob_by_HistoLimitMax,
	NGBlob_by_RangeHisto,
	NGBlob_by_MinAreaFilter,
	NGBlob_by_GridX,
	NGBlob_by_GridY,
	NGBlob_by_HistogramLocalType,
	NGBlob_by_Contrast,
	NGBlob_by_ColorContrast,
	NGBlob_by_DefineNumber,
	NGBlob_by_InspTarget,  // 0: PCB(기존), 1: Pad(Bump)
	NGBlob_by_MinimumNG,
	NGBlob_by_BlobType,
	NGBlob_by_AIModelID,
	NGBlob_by_ContrastExceptRangenum,
	NGBlob_by_ContrastExceptRangeMode,
	NGBlob_by_Total,
};
enum m_eNGBlob_Data
{
	NGBlob_Bin2D = 0x01,
	NGBlob_Bin3D = 0x02,
	NGBlob_Filter = 0x04,
	NGBlob_Width = 0x08,
	NGBlob_Length = 0x10,
	NGBlob_Area = 0x20,
	NGBlob_Color = 0x40,
	NGBlob_Fillhole = 0x80,
	NGBlob_ExtraBumpPer = 0x100,
	NGBlob_ExtraBumpWidth = 0x200,
	NGBlob_Height = 0x400,
	NGBlob_Coplanarity = 0x800, //Use Ref.Height
	NGBlob_WarpageDeviation = 0x1000,
	NGBlob_WarpageDeviationX = 0x2000,
	NGBlob_WarpageDeviationY = 0x4000,
	NGBlob_Histogram = 0x8000,
	NGBlob_AreaFilter = 0x10000,
	NGBlob_Clustering = 0x20000,
	NGBlob_AND_OP = 0x40000,
	NGBlob_Contrast = 0x80000,
	NGBlob_ColorContrast = 0x100000,
	NGBlob_UnderHeight = 0x200000,
	NGBlob_ChippingType = 0x400000,
	NGBlob_BumpNGtoOK = 0x800000,
	NGBlob_ScratchView = 0x1000000,
	NGBlob_ScDistance = 0x2000000,
	NGBlob_ScThickness = 0x4000000,
	NGBlob_UseBallDamage = 0x8000000,
	NGBlob_UseBallLand = 0x10000000,
	NGBlob_WidthC = 0x20000000,
	NGBlob_LengthC = 0x40000000,
};
enum m_eNGBlob_Data2
{
	NGBlob_UnderArea = 0x01,
	NGBlob_RelativeHeight = 0x02,
	NGBlob_SideWidth = 0x04,
	NGBlob_SideLength = 0x08,
	NGBlob_SideArea = 0x10,
	NGBlob_UseAI = 0x20,
	NGBlob_RectSize = 0x40,
	NGBlob_ContrastExceptRange = 0x80,
};
enum m_eNGBlob_F
{
	NGBlob_F_Min3D = 0,
	NGBlob_F_Max3D,
	NGBlob_F_Width,
	NGBlob_F_Length,
	NGBlob_F_Area,
	NGBlob_F_ExtraBumpPer,
	NGBlob_F_ExtraBumpWidth,
	NGBlob_F_MaxHeight,
	NGBlob_F_MaxSC_Distance,
	NGBlob_F_MaxSC_Width,
	NGBlob_F_Warpage_deviation,
	NGBlob_F_Warpage_dev_X,
	NGBlob_F_Warpage_dev_Y,
	NGBlob_F_Chipping_UserTolerance_left, // 티칭 조건
	NGBlob_F_Chipping_Width,  // NG / OK 조건
	NGBlob_F_Chipping_Length,  // NG / OK 조건
	NGBlob_F_InspExceptArea_left,
	NGBlob_F_Chipping_UserTolerance_outer_left,
	NGBlob_F_InspExceptArea_outer_left,
	NGBlob_F_Chipping_UserTolerance_top,
	NGBlob_F_Chipping_UserTolerance_right,
	NGBlob_F_Chipping_UserTolerance_bottom,
	NGBlob_F_InspExceptArea_top,
	NGBlob_F_InspExceptArea_right,
	NGBlob_F_InspExceptArea_bottom,
	NGBlob_F_Chipping_UserTolerance_outer_top,
	NGBlob_F_Chipping_UserTolerance_outer_right,
	NGBlob_F_Chipping_UserTolerance_outer_bottom,
	NGBlob_F_InspExceptArea_outer_top,
	NGBlob_F_InspExceptArea_outer_right,
	NGBlob_F_InspExceptArea_outer_bottom,
	NGBlob_F_CrossLine,
	NGBlob_F_HistoMin2D,
	NGBlob_F_HistoMax2D,
	NGBlob_F_HistoLocalAreaRate,
	NGBlob_F_ClusteringPitch,
	NGBlob_F_dnumGrouping,
	NGBlob_F_Group,
	NGBlob_F_ForeignMaskWidth,
	NGBlob_F_ForeignMaskHeight,
	NGBlob_F_Warpage_deviationMin,
	NGBlob_F_WidthC,
	NGBlob_F_LengthC,
	NGBlob_F_Total,
};
enum m_eNGBlob_InspType
{
	NGBlob_InspType_Default,
	NGBlob_InspType_ExtraBump,
	NGBlob_InspType_Scratch,
	NGBlob_InspType_Chipping,
	NGBlob_InspType_Warpage,
	NGBlob_InspType_BallDamage,
	NGBlob_InspType_DetailWarpage,
	NGBlob_InspType_Total,
};
#define CNT_NGBLOB_PATTERN_PATH		16
#define	TiltRectCnt		4
typedef struct stAlgoNGBlob
{
	byte narrdata[NGBlob_by_Total];
	float farrdata[NGBlob_F_Total];
	int idata;
	int idata2;
	RECT m_rcHeightInRoi[TiltRectCnt]; // tilt roi 구조로 변경 해야함.
	struct tagAlgoColorBase m_sAlgoColorBase;

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
	wchar_t m_sTimeMoPartWndAlgoInfo[MAX_STRLEN];
	wchar_t m_sPathModelTeach[MAX_STRLEN];
	stAlgoNGBlob()
	{
		memset(narrdata, 0, NGBlob_by_Total * sizeof(byte));
		memset(farrdata, 0, NGBlob_F_Total * sizeof(float));
		memset(m_rcHeightInRoi, 0, TiltRectCnt * sizeof(RECT));
		idata = 0;
		idata2 = 0;
		memset(m_sPathModelTeach, 0, MAX_STRLEN * sizeof(wchar_t));
		memset(m_sTimeMoPartWndAlgoInfo, 0, MAX_STRLEN * sizeof(wchar_t));
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
	}
	wchar_t* GetModelPath(int nModelIdx)
	{
		wchar_t* str;
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
}AlgoNGBlob;

typedef struct tagRstAlgoNGBlob // Edge 검사 결과
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
	BOOL bIsOKChipping;    // 
	float fRstWidth[BLOB_RECT_CNTS];		// Width/단축 검사결과괎
	float fRstLength[BLOB_RECT_CNTS];		// Length/장축 검사결과괎
	float fRstArea[BLOB_RECT_CNTS];			// Area/Coining 검사결과괎
	float fRstExtraPer[BLOB_RECT_CNTS];
	float fRstExtraWidth[BLOB_RECT_CNTS];
	float fRstHeight[BLOB_RECT_CNTS];			// Area/Coining 검사결과괎
	float fRstScDistance[BLOB_RECT_CNTS];
	float fRstScWidth[BLOB_RECT_CNTS];
	float fRstChippingWid[BLOB_RECT_CNTS];
	float fRstChippingLen[BLOB_RECT_CNTS];
	int blob_count;
	RECT m_rcArrRect[BLOB_RECT_CNTS];
	float fRstWarpagedeviation;
	float fRstWarpagedev_X;
	float fRstWarpagedev_Y;
	RECT m_BodyRect;
	int OutlineChipping[8];
	int ChippingNGDir;
	BOOL bIsOKContrast;
	BOOL bIsOKColorContrast;
	BOOL bIsOKWarningCount[BLOB_RECT_CNTS];
	tagRstAlgoNGBlob()
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
		memset(fRstWidth, 0, BLOB_RECT_CNTS * sizeof(float));  //InspType 에따라서 다른값임
		memset(fRstLength, 0, BLOB_RECT_CNTS * sizeof(float)); //InspType 에따라서 다른값임
		memset(fRstArea, 0, BLOB_RECT_CNTS * sizeof(float));  //InspType 에따라서 다른값임
		memset(fRstExtraPer, 0, BLOB_RECT_CNTS * sizeof(float));
		memset(fRstExtraWidth, 0, BLOB_RECT_CNTS * sizeof(float));
		memset(fRstHeight, 0, BLOB_RECT_CNTS * sizeof(float));
		memset(fRstScDistance, 0, BLOB_RECT_CNTS * sizeof(float));
		memset(fRstScWidth, 0, BLOB_RECT_CNTS * sizeof(float));
		memset(fRstChippingWid, 0, BLOB_RECT_CNTS * sizeof(float));
		memset(fRstChippingLen, 0, BLOB_RECT_CNTS * sizeof(float));
		blob_count = 0;
		memset(m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));
		m_BodyRect = RECT();
		fRstWarpagedeviation = 0;
		fRstWarpagedev_X = 0;
		fRstWarpagedev_Y = 0;
		memset(OutlineChipping, 0, 8 * sizeof(int));
		ChippingNGDir = 0;
		bIsOKContrast = FALSE;
		bIsOKColorContrast = FALSE;
		memset(bIsOKWarningCount, 0, BLOB_RECT_CNTS * sizeof(BOOL));
	}
}RstAlgoNGBlob;