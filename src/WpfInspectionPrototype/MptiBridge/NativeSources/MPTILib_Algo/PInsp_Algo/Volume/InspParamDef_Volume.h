#pragma once
#include "../InspParamDef_AlgoBase.h"

//Volume Area
const int g_nFullCnt = 10000;
#define VOLUME_ROI_CNTS		500
enum m_eVolumeData
{
	m_eVolumeData_Shift = 0x01,
	m_eVolumeData_ShiftX = 0x02,
	m_eVolumeData_ShiftY = 0x04,
	m_eVolumeData_ShiftPer = 0x08,

	m_eVolumeData_PAD_PAT = 0x10,
	m_eVolumeData_PAD_PAT_Check = 0x20,
	m_eVolumeData_VolumeRangeRef = 0x40,
	m_eVolumeData_UnderHeightMinArea = 0x80,

	m_eVolumeData_UpperHeightMaxArea = 0x100,
	m_eVolumeData_CircleCheck = 0x200,
	m_eVolumeData_CircleDiameter = 0x400,
	m_eVolumeData_CirclePercent = 0x800,

	m_eVolumeData_Width = 0x1000,
	m_eVolumeData_Length = 0x2000,
	m_eVolumeData_InspectionArea = 0x4000,
};
enum m_eVolumeN
{
	m_eVolumeN_Data = 0,
	m_eVolumeN_PatternDivide,
	m_eVolumeN_InspectionArea,
	m_eVolumeN_Total,
};
enum m_eVolumeF
{
	m_eVolumeF_CenterX = 0,
	m_eVolumeF_CenterY,
	m_eVolumeF_ShiftX,
	m_eVolumeF_ShiftY,
	m_eVolumeF_NotUse,
	m_eVolumeF_VolumeRangeRef,
	m_eVolumeF_UnderHeight,
	m_eVolumeF_UnderHeightWidthMax,
	m_eVolumeF_UnderHeightLengthMax,
	m_eVolumeF_UnderHeightAreaMax,
	m_eVolumeF_UpperHeight,
	m_eVolumeF_UpperHeightWidthMax,
	m_eVolumeF_UpperHeightLengthMax,
	m_eVolumeF_UpperHeightAreaMax,
	m_eVolumeF_CircleDiameterMin,
	m_eVolumeF_CircleDiameterMax,
	m_eVolumeF_CirclePercent,
	m_eVolumeF_HeightMin,
	m_eVolumeF_HeightMax,
	m_eVolumeF_WidthMinPer,
	m_eVolumeF_WidthMaxPer,
	m_eVolumeF_LengthMinPer,
	m_eVolumeF_LengthMaxPer,
	m_eVolumeF_ShiftXPer,
	m_eVolumeF_ShiftYPer,
	m_eVolumeF_Total,
};
enum m_eVolumeRData
{
	m_eVolumeRData_ShiftX = 0x01,
	m_eVolumeRData_ShiftY = 0x02,
	m_eVolumeRData_UnderHeightMin = 0x04,
	m_eVolumeRData_UpperHeightMax = 0x08,

	m_eVolumeRData_UnderHeightMinWidth = 0x10,
	m_eVolumeRData_UnderHeightMinLength = 0x20,
	m_eVolumeRData_UnderHeightMinArea = 0x40,
	m_eVolumeRData_UpperHeightMaxWidth = 0x80,

	m_eVolumeRData_UpperHeightMaxLength = 0x100,
	m_eVolumeRData_UpperHeightMaxArea = 0x200,
	m_eVolumeRData_CircleDiameter = 0x400,
	m_eVolumeRData_CirclePercent = 0x800,

	m_eVolumeRData_Width = 0x1000,
	m_eVolumeRData_Length = 0x2000,
};
enum m_eVolumeR
{
	m_eVolumeR_ShiftX = 0,
	m_eVolumeR_ShiftY,
	m_eVolumeR_Teach_X,
	m_eVolumeR_Teach_Y,
	m_eVolumeR_Rst_X,
	m_eVolumeR_Rst_Y,
	m_eVolumeR_Rst_UnderHeightWidth,
	m_eVolumeR_Rst_UnderHeightLength,
	m_eVolumeR_Rst_UnderHeightArea,
	m_eVolumeR_Rst_UpperHeightWidth,
	m_eVolumeR_Rst_UpperHeightLength,
	m_eVolumeR_Rst_UpperHeightArea,
	m_eVolumeR_Rst_CircleDiameter,
	m_eVolumeR_Rst_CirclePercent,
	m_eVolumeR_Rst_TH_MIN,
	m_eVolumeR_Rst_TH_MAX,
	m_eVolumeR_Rst_PAT_X,
	m_eVolumeR_Rst_PAT_Y,
	m_eVolumeR_Total,
};
enum m_eViewMode_Volume
{
	m_eViewMode_Volume_View_Volume,
	m_eViewMode_Volume_View_Under,
	m_eViewMode_Volume_View_Upper,
	m_eViewMode_Volume_View_PatternCheck,
	m_eViewMode_Volume_View_PAD,
	m_eViewMode_Volume_Num,
};
enum m_eVOL_TH_FAIL
{
	m_eVOL_TH_FAIL_MISSING = 0x01,
	m_eVOL_TH_FAIL_MIN = 0x02,
	m_eVOL_TH_FAIL_MAX = 0x04,
};

enum m_ePAT_ALGO_N_DATA
{
	m_ePAT_ALGO_N_DATA_SHIFT = 0x01,
	m_ePAT_ALGO_N_DATA_AREA_OUT_NOT = 0x02,
	m_ePAT_ALGO_N_DATA_FILL = 0x04,
};
enum m_ePAT_ALGO_N
{
	m_ePAT_ALGO_N_Data = 0,
	m_ePAT_ALGO_N_Total,
};
enum m_ePAT_ALGO
{
	m_ePAT_ALGO_TH_D_MIN = 0,
	m_ePAT_ALGO_TH_Min,
	m_ePAT_ALGO_TH_Max,
	m_ePAT_ALGO_TH_D_MAX,
	m_ePAT_ALGO_AREA_MOR,
	m_ePAT_ALGO_AREA_W_MAX,
	m_ePAT_ALGO_AREA_A_MAX,
	m_ePAT_ALGO_NO_SOLDER,
	m_ePAT_ALGO_Total,
};
typedef struct tagAlgoVolume : AlgoBaseBW
{
	double m_dStdBodyHeight;    // Body Blob의 Height값
	double m_dStdWidth;         // Window Width
	double m_dStdLength;        // Pad Length

	double m_dLimitUpper;		// 과납 기준%
	double m_dStdRateValid;		// Standard Volume의 유효값

	BOOL m_bUseBWOption;
	BOOL m_bUseStdMax;

	int m_nTypeSelectBlob;
	double m_dStdVolume;

	BOOL m_bUseHeight;
	BOOL m_bHeightPer;
	double m_dMinHeight;
	double m_dMaxHeight;

	double m_dStdArea;
	BOOL m_bUseArea;
	double m_dMinArea;
	double m_dMaxArea;

	BOOL m_bUseColdJoint;
	double m_dArea_CJ;
	double m_dAreaPer_CJ;

	BOOL m_bUseInclination;
	POINTF m_poArrInclinationROI[VOLUME_INSP_CNTS];
	float m_fInclination;
	float m_fInclinationMin;
	float m_fInclinationMax;
	float m_fRemovalHeight;
	BOOL m_bBlackHeight;
	POINTF m_poArrRemovalHeightROI[VOLUME_INSP_CNTS];
	BOOL m_bFillHole;

	double m_dAddRefVolume;
	int m_nChipTrackingGap;		// ChipTracking 수치 px

	BOOL m_bFilterIsUse;		
	int m_nFilterStepNarrow;


	int m_nArr[m_eVolumeN_Total];
	float m_fArr[m_eVolumeF_Total];

	int m_nArr_M[m_ePAT_ALGO_N_Total];
	float m_fArr_M[m_ePAT_ALGO_Total];

	INSP_PAD_PAT_RST m_rstPadPatRst;

	tagAlgoVolume() : AlgoBaseBW()
	{
		m_dStdBodyHeight = 0.0;	// Body Blob의 Height값
		m_dStdWidth = 0.0;	// Window Width
		m_dStdLength = 0.0;	// Pad Length

		m_dLimitUpper = 500;	// 과납 기준%
		m_dStdRateValid = 0.3;

		m_bUseBWOption = TRUE;
		m_bUseStdMax = FALSE;

		m_nTypeSelectBlob = 0;
		m_dStdVolume = 0.0;
		m_dAddRefVolume = 0.0;

		m_bUseHeight = FALSE;
		m_bHeightPer = FALSE;
		m_dMinHeight = 70;
		m_dMaxHeight = 130;

		m_dStdArea = 0.0;
		m_bUseArea = FALSE;
		m_dMinArea = 70;
		m_dMaxArea = 130;

		m_bUseColdJoint = FALSE;
		m_dArea_CJ = 0.2;
		m_dAreaPer_CJ = 50;

		m_bUseInclination = FALSE;
		m_fInclination = 0.0f;
		m_fInclinationMin = 0.0f;
		m_fInclinationMax = 0.0f;
		m_fRemovalHeight = 0.0f;
		m_bBlackHeight = FALSE;
		m_bFillHole = FALSE;
		m_nChipTrackingGap = 0;

		m_nFilterStepNarrow = 4;

		m_rstPadPatRst.Init();
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[m_eVolumeN_Data] & nType) == nType);
		return bRet;
	}
	float GetData(int nType)
	{
		if (nType < 0 || nType >= m_eVolumeF_Total)
			return 0;
		return m_fArr[nType];
	}
	int GetDataN(int nType)
	{
		if (nType < 0 || nType >= m_eVolumeN_Total)
			return 0;
		return m_nArr[nType];
	}

	bool UseData_M(int nType)
	{
		int nData = GetDataN_M(m_ePAT_ALGO_N_Data);
		bool bRet = ((nData & nType) == nType);
		return bRet;
	}
	float GetData_M(int nType)
	{
		if (nType < 0 || nType >= m_ePAT_ALGO_Total)
			return 0;
		return m_fArr_M[nType];
	}
	int GetDataN_M(int nType)
	{
		if (nType < 0 || nType >= m_ePAT_ALGO_N_Total)
			return 0;
		return m_nArr_M[nType];
	}
}AlgoVolume;

typedef struct tagColdJoint
{
	int m_nWndDir;
	int m_nMinBlobArea;

	double m_dArea_T;
	double m_dAreaPer_T;

	tagColdJoint()
	{
		m_nWndDir = 0;
		m_nMinBlobArea = 4;
		m_dArea_T = 0.2;
		m_dAreaPer_T = 50;
	}
}ColdJoint;

typedef struct tagInclination
{
	int m_nWndDir;
	POINTF m_ptrROI[VOLUME_INSP_CNTS];

	tagInclination()
	{
		m_nWndDir = 0;
	}
}sInclination;

typedef struct tagRemoveHight
{
	BOOL m_bBlackHeight;
	float m_fRemovalHeight;
	POINTF m_ptrROI[VOLUME_INSP_CNTS];

	tagRemoveHight()
	{
		m_bBlackHeight = FALSE;
		m_fRemovalHeight = 0.0f;
	}
}RemoveHight;

typedef struct tagRstColdJoint
{
	BOOL m_nOKColdJoint;

	double dRstArea_R;
	double dRstAreaPer_R;
	RECT m_rcRect_I;

	tagRstColdJoint()
	{
		m_nOKColdJoint = FALSE;
		dRstArea_R = 0.2;
		dRstAreaPer_R = 0;
	}
}RstColdJoint;

typedef struct tagRstInclination
{
	float m_fRstInclination;
	int m_nLineTotalCnt;
	float m_fArrLineHeightAvg[g_nFullCnt];
	RECT m_rcRect_I;
	tagRstInclination()
	{
		m_fRstInclination = 0.0f;
		m_nLineTotalCnt = 0;
		memset(m_fArrLineHeightAvg, 0, g_nFullCnt * sizeof(UCHAR));
	}
}RstInclination;
typedef struct tagRstPAT_DIVIDE
{
	float m_fROIH;
	BOOL m_bOK;
	RECT m_rcRect;
	tagRstPAT_DIVIDE()
	{
		Init();
	}
	void Init()
	{
		m_fROIH = 0;
		m_bOK = FALSE;
		m_rcRect.left = 0;
		m_rcRect.right = 0;
		m_rcRect.top = 0;
		m_rcRect.bottom = 0;
	}
	void AddRST(tagRstPAT_DIVIDE sRst)
	{
		m_fROIH = sRst.m_fROIH;
		m_bOK = sRst.m_bOK;
		m_rcRect.left = sRst.m_rcRect.left;
		m_rcRect.right = sRst.m_rcRect.right;
		m_rcRect.top = sRst.m_rcRect.top;
		m_rcRect.bottom = sRst.m_rcRect.bottom;
	}
}RstPAT_DIVIDE;
typedef struct tagRstAlgoVolume
{
	double m_dRstVolume;
	double m_dRstPercent;
	double m_dRstArea;
	double m_dRstHeight;
	double m_dRstArea_CJ;
	double m_dRstAreaPer_CJ;
	double m_dRstInclination;
	double m_dRstRemoveH;
	double m_dRstWidth;
	double m_dRstLength;
	double m_dRstNotOverlapArea;
	BOOL m_bVolume;
	BOOL m_bNoSolder;
	BOOL m_bExcess;
	BOOL m_bArea;
	BOOL m_bHeight;
	BOOL m_bOKColdJoint;
	BOOL m_bOKInclination;
	BOOL m_bPadPatternAlgo;

	int m_nOKPadPatternCheck;
	int m_nPadPatternCheck;
	int m_nRect_I;
	RECT m_rcRect_I[VOLUME_ROI_CNTS];
	RECT m_rcRect_TH[VOLUME_INSP_CNTS];

	int m_nLineTotalCnt;
	float m_fArrLineHeightAvg[g_nFullCnt];

	int m_nNG;
	int m_nNGSub;
	float m_fArr[m_eVolumeR_Total];

	int m_nRstPAD_CHK;
	int m_nRstPAD_DIVIDE;
	RstPAT_CHK m_sRstPAD_CHK[VOLUME_DIVIDE_AREA_CNTS];
	RstPAT_DIVIDE m_sRstPAD_DIVIDE[VOLUME_DIVIDE_AREA_CNTS];
	tagRstAlgoVolume()
	{
		Init();
	}
	void Init()
	{
		m_dRstVolume = 0;
		m_dRstPercent = 0;
		m_dRstArea = 0;
		m_dRstHeight = 0;
		m_dRstArea_CJ = 0;
		m_dRstAreaPer_CJ = 0;
		m_dRstInclination = 0;
		m_dRstRemoveH = 0;
		m_dRstWidth = 0;
		m_dRstLength = 0;
		m_dRstNotOverlapArea = 0;

		m_bVolume = FALSE;
		m_bNoSolder = FALSE;
		m_bExcess = FALSE;
		m_bArea = FALSE;
		m_bHeight = FALSE;
		m_bOKColdJoint = FALSE;
		m_bOKInclination = FALSE;
		m_bPadPatternAlgo = FALSE;

		m_nOKPadPatternCheck = 0;
		m_nPadPatternCheck = 0;
		m_nRect_I = 0;
		for (int a = 0; a < VOLUME_ROI_CNTS; a++)
		{
			m_rcRect_I[a].left = 0;
			m_rcRect_I[a].right = 0;
			m_rcRect_I[a].top = 0;
			m_rcRect_I[a].bottom = 0;
		}
		for (int a = 0; a < VOLUME_INSP_CNTS; a++)
		{
			m_rcRect_TH[a].left = 0;
			m_rcRect_TH[a].right = 0;
			m_rcRect_TH[a].top = 0;
			m_rcRect_TH[a].bottom = 0;
		}

		m_nLineTotalCnt = 0;
		memset(m_fArrLineHeightAvg, 0, g_nFullCnt * sizeof(float));

		m_nNG = 0;
		m_nNGSub = 0;
		memset(m_fArr, 0, m_eVolumeR_Total * sizeof(float));

		m_nRstPAD_CHK = 0;
		m_nRstPAD_DIVIDE = 0;
		for (int a = 0; a < VOLUME_DIVIDE_AREA_CNTS; a++)
		{
			m_sRstPAD_CHK[a].Init();
			m_sRstPAD_DIVIDE[a].Init();
		}
	}
	void AddRect(RECT roi)
	{
		if (m_nRect_I < 0) m_nRect_I = 0;
		if (m_nRect_I >= VOLUME_ROI_CNTS)
			return;

		m_rcRect_I[m_nRect_I].left = roi.left;
		m_rcRect_I[m_nRect_I].right = roi.right;
		m_rcRect_I[m_nRect_I].top = roi.top;
		m_rcRect_I[m_nRect_I].bottom = roi.bottom;
		m_nRect_I++;
	}
	bool OKData(int nType)
	{
		bool bRet = ((m_nNG & nType) == nType);
		return !bRet;
	}
	bool OKSubData(int nType)
	{
		bool bRet = ((m_nNGSub & nType) == nType);
		return !bRet;
	}
	void AddPAT_CHK(RstPAT_CHK sRst)
	{
		if (m_nRstPAD_CHK < 0)
		{
			m_nRstPAD_CHK = 0;
			for (int a = 0; a < VOLUME_DIVIDE_AREA_CNTS; a++)
				m_sRstPAD_CHK[a].Init();
		}
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
	void AddPAT_DIVIDE(RstPAT_DIVIDE sRst)
	{
		if (m_nRstPAD_DIVIDE < 0) m_nRstPAD_DIVIDE = 0;
		int nID = m_nRstPAD_DIVIDE;
		if (nID < VOLUME_DIVIDE_AREA_CNTS)
		{
			m_sRstPAD_DIVIDE[nID].AddRST(sRst);
			m_nRstPAD_DIVIDE++;
		}
	}
}RstAlgoVolume;
