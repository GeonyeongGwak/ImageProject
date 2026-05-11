#pragma once
#include <string.h>

#define INSP_ALGO_LIGHT_CNT		13
#define FOREIGN_POLYGON_CNTS	10
#define POLYGON_BTM_POINT_CNTS	7
#define POLYGON_POINT_CNTS		5
#define FR_RGB_Total			3

enum InspLightType
{
	Top_Light,
	Middle_Light,
	Bottom_Light,
	User_Light,
	Side1_Light,
	Side2_Light,
	Side3_Light,
	Side4_Light,
	ThreeD,
	num,
};

enum ThresholdType
{
	eUpper = 0,
	eLower,
	eInner,
	eOuter
};

enum FR_BIN_DT
{
	FR_BIN_DT_Use = 0x01,
	FR_BIN_DT_3D = 0x02,
	FR_BIN_DT_Color = 0x04,
	FR_BIN_DT_Gray = 0x08,

	FR_BIN_DT_ColorInvert = 0x10,
	FR_BIN_DT_InspSolderArea = 0x20,
	FR_BIN_DT_NotSize = 0x40,
	FR_BIN_DT_ExceptBTC = 0x80,

	FR_BIN_DT_AngleColor = 0x100,
	FR_BIN_DT_SIZE_AREA = 0x200,
	FR_BIN_DT_N_Area_MAX = 0x400,
	FR_BIN_DT_Insp_Skip_Part = 0x800,

	FR_BIN_DT_PAD_SKIP = 0x1000,
	FR_BIN_DT_PART_VIEW = 0x2000,
	FR_BIN_DT_HA_Under = 0x4000,
	FR_BIN_DT_HA_Upper = 0x8000,

	FR_BIN_DT_INSP_TRACE = 0x10000,
	FR_BIN_DT_HA_DEFECT = 0x20000,
	FR_BIN_DT_INSP_TRACE_NONE = 0x40000,
	FR_BIN_DT_CircleCheck = 0x80000,

	FR_BIN_DT_CircleDiameter = 0x100000,
	FR_BIN_DT_CirclePercent = 0x200000,
};
enum FR_BIN_N
{
	FR_BIN_N_Data = 0,
	FR_BIN_N_Defect,
	FR_BIN_N_Range3D,
	FR_BIN_N_RangeGray,
	FR_BIN_N_Min2D,
	FR_BIN_N_Max2D,
	FR_BIN_N_RoiCount,
	FR_BIN_N_Default, // 0~99 Color 100~199 Gray
	FR_BIN_N_PAD_SKIP,
	FR_BIN_N_NOT_SIZE,
	FR_BIN_N_Total,
};
enum FR_BIN_F
{
	FR_BIN_F_N_Area = 0,
	FR_BIN_F_Min3D,
	FR_BIN_F_Max3D,
	FR_BIN_F_N_Area_Max,
	FR_BIN_F_HA_Under,
	FR_BIN_F_HA_Upper,
	FR_BIN_F_HA_W,
	FR_BIN_F_HA_L,
	FR_BIN_F_HA_A,
	FR_BIN_F_CircleDiameterMin,
	FR_BIN_F_CircleDiameterMax,
	FR_BIN_F_CirclePercent,
	FR_BIN_F_Total,
};
enum Foreign_NG_Type
{
	Foreign_NG_Type_None = -1,
	Foreign_NG_Type_Foreign,
	Foreign_NG_Type_Copper,
	Foreign_NG_Type_SolderBall,
	Foreign_NG_Type_Bubble,
	Foreign_NG_Type_Warpage,
	Foreign_NG_Type_WarpageH,
	Foreign_NG_Type_HeightArea,
	Foreign_NG_Type_Foreign_A,
	Foreign_NG_Type_Foreign_B,
	Foreign_NG_Type_Foreign_C,
	Foreign_NG_Type_Total,
};
enum Foreign_NG_Type_UI
{
	Foreign_NG_Type_UI_Foreign = 0,
	Foreign_NG_Type_UI_Copper,
	Foreign_NG_Type_UI_SolderBall,
	Foreign_NG_Type_UI_Foreign_A,
	Foreign_NG_Type_UI_Foreign_B,
	Foreign_NG_Type_UI_Foreign_C,
	Foreign_NG_Type_UI_Total,
};
enum m_eForeignInsp
{
	m_eForeignInsp_Trace = -12,
	m_eForeignInsp_TraceModeNONE = -11,
	m_eForeignInsp_TraceMode = -10,
	m_eForeignInsp_SkipPart = -8,
	m_eForeignInsp_Calc3D = -7,
	m_eForeignInsp_BTM = -6,
	m_eForeignInsp_2D_Gray = -5,
	m_eForeignInsp_2D_Color = -4,
	m_eForeignInsp_SolderArea = -3,
	m_eForeignInsp_2D = -2,
	m_eForeignInsp_3D = -1,

	m_eForeignInsp_Foreign = 0,
	m_eForeignInsp_Foreign2D,
	m_eForeignInsp_Gray,
	m_eForeignInsp_Copper,

	m_eForeignInsp_GrayBub,
	m_eForeignInsp_Warpage,
	m_eForeignInsp_Total,
};
enum m_eFR_Bin_INSP
{
	m_eFR_Bin_INSP_A,
	m_eFR_Bin_INSP_W,
	m_eFR_Bin_INSP_L,
	m_eFR_Bin_INSP_Total,
};
enum m_eFR_Bin_INSP_N
{
	m_eFR_Bin_INSP_N_Data,
	m_eFR_Bin_INSP_N_Total,
};
enum m_eFR_Bin_INSP_D
{
	m_eFR_Bin_INSP_D_Min = 0x01,
	m_eFR_Bin_INSP_D_Max = 0x02,
	m_eFR_Bin_INSP_D_Ess = 0x04,
};
enum m_eFR_Bin_INSP_F
{
	m_eFR_Bin_INSP_F_Min,
	m_eFR_Bin_INSP_F_Max,
	m_eFR_Bin_INSP_F_Total,
};

typedef struct tagColorXYInfoForeign
{
	BOOL m_bUseThreshold;
	ThresholdType m_vThresholdType;
	int m_nMin;
	int m_nMax;

	tagColorXYInfoForeign()
	{
		m_bUseThreshold = FALSE;
		m_vThresholdType = eUpper;
		m_nMin = 0;
		m_nMax = 255;
	}
	void Clone(tagColorXYInfoForeign &inspAlgo)
	{
		inspAlgo.m_bUseThreshold = m_bUseThreshold;
		inspAlgo.m_vThresholdType = m_vThresholdType;
		inspAlgo.m_nMin = m_nMin;
		inspAlgo.m_nMax = m_nMax;
	}
}ColorXYInfoForeign;
typedef struct tagForignLight
{
	InspLightType m_eLightType;
	int m_nRedValue;
	int m_nGreenValue;
	int m_nBlueValue;
	int m_nWhiteValue;

	int m_nLightCnt;
	int m_nArrRedValue[INSP_ALGO_LIGHT_CNT];
	int m_nArrGreenValue[INSP_ALGO_LIGHT_CNT];
	int m_nArrBlueValue[INSP_ALGO_LIGHT_CNT];
	int m_nArrWhiteValue[INSP_ALGO_LIGHT_CNT];
	int m_nArrCalculation[INSP_ALGO_LIGHT_CNT];
	int m_nArrLightPosition[INSP_ALGO_LIGHT_CNT];

	void LightClone(tagForignLight &inspAlgo)
	{
		inspAlgo.m_eLightType = m_eLightType;
		inspAlgo.m_nRedValue = m_nRedValue;
		inspAlgo.m_nGreenValue = m_nGreenValue;
		inspAlgo.m_nBlueValue = m_nBlueValue;
		inspAlgo.m_nWhiteValue = m_nWhiteValue;

		inspAlgo.m_nLightCnt = m_nLightCnt;
		if (inspAlgo.m_nLightCnt >= INSP_ALGO_LIGHT_CNT)
			inspAlgo.m_nLightCnt = INSP_ALGO_LIGHT_CNT - 1;
		if (inspAlgo.m_nLightCnt < 0)
			inspAlgo.m_nLightCnt = 0;
		memcpy(inspAlgo.m_nArrRedValue, m_nArrRedValue, sizeof(int) * INSP_ALGO_LIGHT_CNT);
		memcpy(inspAlgo.m_nArrGreenValue, m_nArrGreenValue, sizeof(int) * INSP_ALGO_LIGHT_CNT);
		memcpy(inspAlgo.m_nArrBlueValue, m_nArrBlueValue, sizeof(int) * INSP_ALGO_LIGHT_CNT);
		memcpy(inspAlgo.m_nArrWhiteValue, m_nArrWhiteValue, sizeof(int) * INSP_ALGO_LIGHT_CNT);
		memcpy(inspAlgo.m_nArrCalculation, m_nArrCalculation, sizeof(int) * INSP_ALGO_LIGHT_CNT);
		memcpy(inspAlgo.m_nArrLightPosition, m_nArrLightPosition, sizeof(int) * INSP_ALGO_LIGHT_CNT);
	}
}ForignLight;
typedef struct tagFR_Bin_INSP
{
	int m_nArr[m_eFR_Bin_INSP_N_Total];
	float m_fArr[m_eFR_Bin_INSP_F_Total];
	void Clone(tagFR_Bin_INSP& inspAlgo)
	{
		memcpy(inspAlgo.m_nArr, m_nArr, sizeof(int) * m_eFR_Bin_INSP_N_Total);
		memcpy(inspAlgo.m_fArr, m_fArr, sizeof(float) * m_eFR_Bin_INSP_F_Total);
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[m_eFR_Bin_INSP_N_Data] & nType) == nType);
		return bRet;
	}

}FR_Bin_INSP;
typedef struct tagFR_Bin
{
	int m_nArr[FR_BIN_N_Total];
	float m_fArr[FR_BIN_F_Total];

	tagForignLight m_MixLight;
	POINT m_ptArrPolygon[FOREIGN_POLYGON_CNTS][POLYGON_POINT_CNTS];
	ColorXYInfoForeign m_ColorXYInfo[FOREIGN_POLYGON_CNTS];
	lightData m_sLightData[FR_RGB_Total];
	FR_Bin_INSP m_arrINSP[m_eFR_Bin_INSP_Total];
	void Init()
	{
		memset(m_nArr, 0, sizeof(int) * FR_BIN_N_Total);
		memset(m_fArr, 0, sizeof(float) * FR_BIN_F_Total);
		for (int a = 0; a < FOREIGN_POLYGON_CNTS; a++)
			memset(m_ptArrPolygon[a], 0, sizeof(POINT) * POLYGON_POINT_CNTS);
		memset(m_ColorXYInfo, 0, sizeof(ColorXYInfoForeign) * FOREIGN_POLYGON_CNTS);
		memset(m_sLightData, 0, sizeof(lightData) * FR_RGB_Total);
		memset(m_arrINSP, 0, sizeof(FR_Bin_INSP) * m_eFR_Bin_INSP_Total);
	}
	void Clone(tagFR_Bin &inspAlgo)
	{
		memcpy(inspAlgo.m_nArr, m_nArr, sizeof(int) * FR_BIN_N_Total);
		memcpy(inspAlgo.m_fArr, m_fArr, sizeof(float) * FR_BIN_F_Total);
		m_MixLight.LightClone(inspAlgo.m_MixLight);
		for (int a = 0; a < FOREIGN_POLYGON_CNTS; a++)
		{
			memcpy(inspAlgo.m_ptArrPolygon[a], m_ptArrPolygon[a], sizeof(POINT) * POLYGON_POINT_CNTS);
			m_ColorXYInfo[a].Clone(inspAlgo.m_ColorXYInfo[a]);
		}
		for (int a = 0; a < FR_RGB_Total; a++)
			m_sLightData[a].Clone(inspAlgo.m_sLightData[a]);
		for (int a = 0; a < m_eFR_Bin_INSP_Total; a++)
			m_arrINSP[a].Clone(inspAlgo.m_arrINSP[a]);
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[FR_BIN_N_Data] & nType) == nType);
		return bRet;
	}
	int GetDEFECT()
	{
		int nDefect = m_nArr[FR_BIN_N_Defect];
		if (nDefect == Foreign_NG_Type_UI_Copper)
			return Foreign_NG_Type_Copper;
		else if (nDefect == Foreign_NG_Type_UI_SolderBall)
			return Foreign_NG_Type_SolderBall;
		else if (nDefect == Foreign_NG_Type_UI_Foreign_A)
			return Foreign_NG_Type_Foreign_A;
		else if (nDefect == Foreign_NG_Type_UI_Foreign_B)
			return Foreign_NG_Type_Foreign_B;
		else if (nDefect == Foreign_NG_Type_UI_Foreign_C)
			return Foreign_NG_Type_Foreign_C;
		return Foreign_NG_Type_Foreign;
	}
	bool IsForeignEnable(int nCheck)
	{
		bool bRet = FALSE;
		bRet = UseData(FR_BIN_DT_Use);
		if (bRet)
		{
			switch (nCheck)
			{
			case m_eForeignInsp_3D:
				bRet = UseData(FR_BIN_DT_3D);
				if (UseData(FR_BIN_DT_HA_Under) || UseData(FR_BIN_DT_HA_Upper))
					bRet = true;
				break;
			case m_eForeignInsp_2D:
				bRet = UseData(FR_BIN_DT_Color) || UseData(FR_BIN_DT_Gray);
				break;
			case m_eForeignInsp_SolderArea:
				bRet = UseData(FR_BIN_DT_InspSolderArea);
				break;
			case m_eForeignInsp_2D_Color:
				bRet = UseData(FR_BIN_DT_Color);
				break;
			case m_eForeignInsp_2D_Gray:
				bRet = UseData(FR_BIN_DT_Gray);
				break;
			case m_eForeignInsp_BTM:
				bRet = UseData(FR_BIN_DT_ExceptBTC);
				break;
			case m_eForeignInsp_SkipPart:
				bRet = UseData(FR_BIN_DT_Insp_Skip_Part);
				break;
			default:
				break;
			}
		}
		return bRet;
	}
}FR_Bin;