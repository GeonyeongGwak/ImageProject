#pragma once
#include "../InspParamDef_AlgoBase.h"

const int BUMP_RECT_CNTS = 10;
enum m_eBump_by // int
{
	Bump_by_Data = 0,
	Bump_by_Filter,
	Bump_by_Min2D,
	Bump_by_Max2D,
	Bump_by_Range2D,
	Bump_by_Range3D,
	Bump_by_InspType,
	Bump_by_HeightAreaType,
	Bump_by_HeightAreaType_COP,
	Bump_by_Total,
};
enum m_eBump_Data // bool
{
	Bump_Data_Bin2D = 0x0001,
	Bump_Data_Bin3D = 0x0002,
	Bump_Data_Filter = 0x0004,
	Bump_Data_UseFillHole = 0x0008,
	Bump_Data_UseNGBumpW = 0x0010,
	Bump_Data_UseNGBumpL = 0x0020,
	Bump_Data_UseUnCoining = 0x0040,
	Bump_Data_UseShiftX = 0x0080,
	//        UseCoining = 0x0100, 필요없음
	Bump_Data_UseArea = 0x0100,
	Bump_Data_UseWidth = 0x0200,
	Bump_Data_UseLength = 0x0400,
	Bump_Data_UseHeight = 0x0800,
	Bump_Data_UseCirclerate = 0x1000,
	Bump_Data_UseAnchor = 0x2000,
	Bump_Data_UseTwoAnchor = 0x4000,
	Bump_Data_UseBridge = 0x8000,
	Bump_Data_UsePitch = 0x10000,
	Bump_Data_UseShiftR = 0x20000,
	Bump_Data_UseContrast = 0x40000,
	Bump_Data_UseHeightDev = 0x80000,
	Bump_Data_UseConplanarity = 0x100000,
	Bump_Data_UseVolume = 0x200000,
	Bump_Data_UseUmPer = 0x400000,
	Bump_Data_UseWLPer = 0x800000,
	Bump_Data_UseAbsoluteHeight = 0x1000000,
};
enum m_eBump_F // float
{
	Bump_F_Min3D = 0,
	Bump_F_Max3D,
	Bump_F_Area,
	Bump_F_NGBumpW,
	Bump_F_NGBumpL,
	Bump_F_UncoiningValue,
	Bump_F_TeachWidth,
	Bump_F_TeachLength,
	Bump_F_TeachShiftX,
	Bump_F_TeachShiftY,
	Bump_F_AreaMin,
	Bump_F_AreaMax,
	Bump_F_LengthRateMax,
	Bump_F_LengthRateMin,
	Bump_F_WidthRateMax,
	Bump_F_WidthRateMin,
	Bump_F_HeightRateMin,
	Bump_F_HeightRateMax,
	Bump_F_HeightMin,
	Bump_F_HeightMax,
	Bump_F_CircleRate,
	Bump_F_TeachHeight,
	Bump_F_TechCenterX,
	Bump_F_TechCenterY,
	Bump_F_TolPitch,
	Bump_F_TolShiftR,
	Bump_F_TolContrast,
	Bump_F_TeachVolume,
	Bump_F_VolumeRateMin,
	Bump_F_VolumeRateMax,
	Bump_F_HeightDev,
	Bump_F_Total,
};

typedef struct stAlgoBump
{
	int narrdata[Bump_by_Total];
	float farrdata[Bump_F_Total];
	int idata;
	wchar_t m_sAlignWindow[20];
	wchar_t m_sBallLabelID[20];

	stAlgoBump()
	{
		memset(narrdata, 0, Bump_by_Total * sizeof(int));
		memset(farrdata, 0, Bump_F_Total * sizeof(float));
		idata = 0;
		memset(m_sAlignWindow, 0, 20 * sizeof(wchar_t));
		memset(m_sBallLabelID, 0, 20 * sizeof(wchar_t));
	}
}AlgoBump;

typedef struct tagRstAlgoBump // Edge 검사 결과
{
	//memset(m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));
	BOOL m_bOKShiftX;    // Shift 검사결과
	BOOL m_bOKShiftY;    // Shift 검사결과
	BOOL m_bOKShiftR;    // Shift 검사결과
	BOOL bIsOKArea;      // Area,Coining 검사결과
	BOOL bIsOKHeight;    // Height 검사결과
	BOOL bIsOKNGBumpW;    // NGBumpWidth 검사결과
	BOOL bIsOKNGBumpL;    // NGBumpLength 검사결과
	BOOL bIsOKUnCoining;    // Uncoining 검사결과
	BOOL bIsOKWidth;    // Width/단축 검사결과
	BOOL bIsOKLength;    // Length/장축 검사결과
	BOOL bIsOKCircleRate;    // CircleRate 검사결과
	BOOL bIsOKBridge;    // Bridge 검사결과
	BOOL bIsOKPitch;
	BOOL bIsOKContrast;
	BOOL bIsOKHeightDev;
	float fRstShiftX;    // ShiftX 검사결과값
	float fRstShiftY;    // ShiftY 검사결과값
	float fRstShiftR;
	float fRstNGBumpW;    // NGBumpWidth 검사결과값
	float fRstNGBumpL;    // NGBumpLength 검사결과값
	float fRstUncoining;    // Uncoining 검사결과값
	float fRstWidth;		// Width/단축 검사결과괎
	float fRstLength;		// Length/장축 검사결과괎
	float fRstArea;			// Area/Coining 검사결과괎
	float fRstHeight;    // Height 검사결과값
	float fRstCicleRate;    // CircleRate 검사결과값
	float fRstPitch;
	float fRstContrast;
	float fRstHeightDev;
	float fRstCoplanarity;
	RECT m_rcArrRectF[BUMP_RECT_CNTS]; // 이물영역
	int m_iFRectCnt; // 이물개수
	RECT m_rcRectI; // 찾은 Bump 영역
	RECT m_rcRectT; // Search Bump 영역
	BOOL bIsOKVolume;
	float fRstVolume;
	float fRstBottomHeight;
	tagRstAlgoBump()
	{
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKShiftR = FALSE;
		bIsOKArea = FALSE;
		bIsOKHeight = FALSE;
		bIsOKNGBumpW = FALSE;
		bIsOKNGBumpL = FALSE;
		bIsOKUnCoining = FALSE;
		bIsOKWidth = FALSE;
		bIsOKLength = FALSE;
		bIsOKCircleRate = FALSE;
		bIsOKBridge = FALSE;
		bIsOKPitch = FALSE;
		bIsOKContrast = FALSE;
		bIsOKHeightDev = FALSE;
		fRstShiftX = 0;
		fRstShiftY = 0;
		fRstShiftR = 0;
		fRstNGBumpW = 0;
		fRstNGBumpL = 0;
		fRstUncoining = 0;
		fRstWidth = 0;  //InspType 에따라서 다른값임
		fRstLength = 0; //InspType 에따라서 다른값임
		fRstArea = 0;  //InspType 에따라서 다른값임
		fRstHeight = 0;
		fRstCicleRate = 0;
		fRstPitch = 0;
		fRstContrast = 0;
		fRstHeightDev = 0;
		fRstCoplanarity = 0;
		memset(m_rcArrRectF, 0, BUMP_RECT_CNTS * sizeof(RECT));
		m_iFRectCnt = 0;
		bIsOKVolume = FALSE;
		fRstVolume = 0;
		fRstBottomHeight = 0;
	}
}RstAlgoBump;