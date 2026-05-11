#pragma once
#include "MPTIDef.h"
#include "PInsp_Algo/InspParamDef_AlgoBase.h"
#include "InspParamDef_FR_Bin.h"
#include "InspParamDef_FR_Bubble.h"
#include "InspParamDef_FR_WP.h"
#include "InspParamDef_FR_Find.h"
#include "InspParamDef_FR_Sticker.h"

#define ZMPOLYPOINT_CNTS		20
#define ZMPOLYPOINT_ULTRA_CNTS	60
#define INSP_WARPAGE			0x0002
#define StickerPo				4
#define StickerCnt				2

enum Foreign_Matching_Type
{
	eNotExistSolderExcept = 0,
	eSolderExceptLower,
	eSolderExceptUpper,
};

enum m_eForeignTab
{
	m_eForeignTab_Foreign = 0,
	m_eForeignTab_Foreign2D,
	m_eForeignTab_Gray,
	m_eForeignTab_Copper,

	m_eForeignTab_Bubble,
	m_eForeignTab_Warpage,
	m_eForeignTab_Option,
	m_eForeignTab_Find,
	m_eForeignTab_Sticker,
	m_eForeignTab_Total,
	m_eForeignTab_None,
};
enum FR_DATA
{
	FR_DATA_DebugData = 0x01,
	FR_DATA_RemoveArea_Hole = 0x02,
	FR_DATA_ForeignExceptHeight = 0x04,
	FR_DATA_UseClustering = 0x08,

	FR_DATA_SolderArea = 0x10,
	FR_DATA_SkipPartInspection = 0x20,
	FR_DATA_SkipAreaMount = 0x40,
	FR_DATA_SkipAreaAlign = 0x80,

	FR_DATA_SkipAreaOCR = 0x100,
	FR_DATA_SkipAreaLead = 0x200,
	FR_DATA_SkipAreaSolder = 0x400,
	FR_DATA_SkipAreaSB = 0x800,

	FR_DATA_SkipAreaTab = 0x1000,
	FR_DATA_SkipAreaGerber = 0x2000,
	FR_DATA_SkipAreaBodyBlob = 0x4000,
	FR_DATA_SkipAreaETC = 0x8000,

	FR_DATA_SkipGapPart = 0x10000,
	FR_DATA_SkipFidu = 0x20000,
	FR_DATA_SkipBar = 0x40000,
	FR_DATA_SkipBad = 0x80000,

	FR_DATA_DebugData2 = 0x100000,
	FR_DATA_FindBody_Q = 0x200000,
	FR_DATA_FindBody_Wnd = 0x400000,
	FR_DATA_SAVE_NG_RAWDATA = 0x800000,

	FR_DATA_FindBody_NotSize = 0x1000000,
	FR_DATA_Manual_EXT = 0x2000000,
	FR_DATA_Calc_WP = 0x4000000,
	FR_DATA_NGCount = 0x8000000,

	FR_DATA_SkipAreaGerberShape = 0x10000000,
};
enum FR_DATA2
{
	FR_DATA2_ExtraPixelM_Per = 0x01,
	FR_DATA2_ExtraPixelS_Per = 0x02,
	FR_DATA2_ExtraPixelL_Per = 0x04,
	FR_DATA2_ExtraPixelBr_Per = 0x08,

	FR_DATA2_ExtraPixelE_Per = 0x10,
};
enum FR_N
{
	FR_N_Data = 0,
	FR_N_PixelClusteringCnt,
	FR_N_ExtraPixelM,
	FR_N_ExtraPixelS,
	FR_N_ExtraPixelL,
	FR_N_ExtraPixelBr,
	FR_N_ExtraPixelETC,
	FR_N_ExtraPixelETC2,
	FR_N_PCBL,
	FR_N_PCBR,
	FR_N_PCBT,
	FR_N_PCBB,
	FR_N_ForeignStep,
	FR_N_ExceptArea,
	FR_N_FR_WideX,
	FR_N_FR_WideY,
	FR_N_FR_NGCnt_Fr,
	FR_N_FR_NGCnt_Co,
	FR_N_ForeignParamROI_Total,
	FR_N_FR_NGCnt_Sb,
	FR_N_FR_Remove_ROI,
	FR_N_Data2,
	FR_N_ExtraPixelPAD,
	FR_N_ExtraPixelPAD_W,
	FR_N_Total,
};
enum FR_F
{
	FR_F_Job_Version = 0,
	FR_F_SearchRange,
	FR_F_ForeignExceptHeightMin,
	FR_F_ForeignExceptHeightMax,
	FR_F_BoardOffset,
	FR_F_CalcRange_XY,
	FR_F_ExceptModuleAreaX,
	FR_F_ExceptModuleAreaY,
	FR_F_FactorR,
	FR_F_FactorG,
	FR_F_FactorB,
	FR_F_FactorBR,
	FR_F_FactorBB,
	FR_F_BoardW,
	FR_F_BoardH,
	FR_F_Find,
	FR_F_ExtraPixelM_Per,
	FR_F_ExtraPixelS_Per,
	FR_F_ExtraPixelL_Per,
	FR_F_ExtraPixelBr_Per,
	FR_F_ExtraPixelETC_Per,
	FR_F_ExtraPixelETC2_Per,
	FR_F_Total,
};
enum ForeignParamROI_Data
{
	ForeignParamROI_Data_Foreign = 0,
	ForeignParamROI_Data_Bubble,
	ForeignParamROI_Data_InspA,
	ForeignParamROI_Data_Module,
	ForeignParamROI_Data_SkipPart,
	ForeignParamROI_Data_PAD,
	ForeignParamROI_Data_Solder,
	ForeignParamROI_Data_Total,
};
enum FR_SUB
{
	FR_SUB_EXCP = 0x01,
	FR_SUB_PART = 0x02,
	FR_SUB_BOTM = 0x04,
	FR_SUB_AMIM = 0x08,
	FR_SUB_HOLE = 0x10,
	FR_SUB_Sticker = 0x20,
	FR_SUB_MODU = 0x010000,
};
enum FR_IMG
{
	FR_IMG_HOLE = 0,
	FR_IMG_BOTM,
	FR_IMG_AM,
	FR_IMG_IM,
	FR_IMG_HEXT,
	FR_IMG_FIND,
	FR_IMG_SOLDER,
	FR_IMG_BIN,
	FR_IMG_BLOB,
	FR_IMG_EXT,
	FR_IMG_SOLDER_INV,
	FR_IMG_SKIP_PART,
	FR_IMG_TRACE_EXT,
	FR_IMG_TRACE,
	FR_IMG_TRACE_NONE,
	FR_IMG_FLASH,
	FR_IMG_Total,
};
enum m_eInspFR_Save
{
	m_eInspFR_Save_Data = 0,
	m_eInspFR_Save_Divide,
	m_eInspFR_Save_GapX,
	m_eInspFR_Save_GapY,
	m_eInspFR_Save_GrabMode,
	m_eInspFR_Save_Quality,
	m_eInspFR_Save_ColorImgW,
	m_eInspFR_Save_ColorImgH,
	m_eInspFR_Save_Fov2DW,
	m_eInspFR_Save_Fov2DH,
	m_eInspFR_Save_Total,
};
enum m_eInspFR_Save_Data
{
	m_eInspFR_Save_Data_e3D = 0x01,
	m_eInspFR_Save_Data_e2D = 0x02,
	m_eInspFR_Save_Data_MES = 0x04,
	m_eInspFR_Save_Data_PNG = 0x08,

	m_eInspFR_Save_Data_TIF = 0x10,
	m_eInspFR_Save_Data_ONLINE = 0x20,
	m_eInspFR_Save_Data_RUN = 0x40,
	m_eInspFR_Save_Data_FOV = 0x80,
};
enum WP_TACT
{
	WP_TACT_SET = 0,
	WP_TACT_CLIP,
	WP_TACT_RGB,
	WP_TACT_RST_SET,
	WP_TACT_INSP_WP,
	WP_TACT_AUTO,
	WP_TACT_RST,
	WP_TACT_END,
	WP_TACT_TOTAL,
	WP_TACT_CNT,
};
enum CalcSeqZmap_DATA
{
	CalcSeqZmap_DATA_FR_3D_CHK = 0x01,
	CalcSeqZmap_DATA_FR = 0x02,
	CalcSeqZmap_DATA_FR_3D = 0x04,
	CalcSeqZmap_DATA_Z_OFFSET = 0x08,
};
enum pad_format
{
	pad_format_Flash = 1,
	pad_format_Trace,
};
enum m_eForeignData_DATA
{
	m_eForeignData_DATA_GND = 0x01,
};
enum FR_HIT_AREA_DATA
{
	FR_HIT_AREA_DATA_GRAY = 0x01,
	FR_HIT_AREA_DATA_BTM_GRAY = 0x02,
	FR_HIT_AREA_DATA_BTM = 0x04,
	FR_HIT_AREA_DATA_BTM_GRAY_R = 0x08,
	FR_HIT_AREA_DATA_BTM_GRAY_G = 0x10,
	FR_HIT_AREA_DATA_BTM_GRAY_B = 0x20,
};
typedef struct tagInspForeignInfo
{
	int m_nFOVOption_3D;
	int m_nArrData[FR_N_Total];
	float m_fArrData[FR_F_Total];
	FR_Bin sBin[m_eForeignInsp_GrayBub];
	FR_Bubble sBubble;
	FR_WP sWP;
	FR_Sticker sSticker;
	tagInspForeignInfo()
	{
		m_nFOVOption_3D = 0;
		memset(m_nArrData, 0, sizeof(int) * FR_N_Total);
		memset(m_fArrData, 0, sizeof(float) * FR_F_Total);
		for (int a = 0; a < m_eForeignInsp_GrayBub; a++)
			sBin[a].Init();
		sBubble.Init();
		sWP.Init();
		sSticker.Init();
	}
	void Clone(tagInspForeignInfo &inspAlgo)
	{
		inspAlgo.m_nFOVOption_3D = m_nFOVOption_3D;
		memcpy(inspAlgo.m_nArrData, m_nArrData, sizeof(int) * FR_N_Total);
		memcpy(inspAlgo.m_fArrData, m_fArrData, sizeof(float) * FR_F_Total);
		for (int a = 0; a < m_eForeignInsp_GrayBub; a++)
			sBin[a].Clone(inspAlgo.sBin[a]);
		sBubble.Clone(inspAlgo.sBubble);
		sWP.Clone(inspAlgo.sWP);
		sSticker.Clone(inspAlgo.sSticker);
	}
	int GetRemove(int nW, int nH)
	{
		int nDivide = GetDivide();
		int nExceptArea = m_nArrData[FR_N_ExceptArea] / nDivide;
		if (nExceptArea < 0) nExceptArea = 0;
		if (nExceptArea > ((nW / 2) - 2)) nExceptArea = (nW / 2) - 2;
		if (nExceptArea > ((nH / 2) - 2)) nExceptArea = (nH / 2) - 2;

		return nExceptArea;
	}
	bool UseData(int nType, bool bFOV = false)
	{
		int nData = m_nArrData[FR_N_Data];
		if (bFOV)
			nData = m_nFOVOption_3D;

		bool bRet = ((nData & nType) == nType);
		return bRet;
	}
	bool UseData2(int nType)
	{
		int nData = m_nArrData[FR_N_Data2];
		bool bRet = ((nData & nType) == nType);
		return bRet;
	}
	int GetDivide()
	{
		int nDivide = (m_nArrData[FR_N_ForeignStep] <= 2) ? 2 : 4;
		return nDivide;
	}
	bool IsForeignEnable(int nType, int nCheck = m_eForeignInsp_Total)
	{
		bool bRet = FALSE;
		if (nType == m_eForeignInsp_GrayBub)
			bRet = sBubble.UseData(FR_BUBBLE_DT_USE);
		else if (nType == m_eForeignInsp_Warpage)
		{
			bRet = UseData(INSP_WARPAGE, true);
			if (bRet)
			{
				switch (nCheck)
				{
					case m_eForeignInsp_2D_Gray:
					case m_eForeignInsp_2D_Color:
					case m_eForeignInsp_SolderArea:
					case m_eForeignInsp_2D:
						bRet = FALSE;
						break;
					case m_eForeignInsp_BTM:
						bRet = sWP.UseData(FR_WP_DT_BTC);
						break;
				}
			}
			else
				bRet = FALSE;
		}
		else if (nType == m_eForeignInsp_Foreign && nCheck == m_eForeignInsp_Trace)
		{
			for (int a = 0; a < m_eForeignInsp_GrayBub; a++)
			{
				if (sBin[a].UseData(FR_BIN_DT_Use))
				{
					if (sBin[a].UseData(FR_BIN_DT_INSP_TRACE))
						return TRUE;
					else if (sBin[a].UseData(FR_BIN_DT_INSP_TRACE_NONE))
						return TRUE;
				}
			}
			return FALSE;
		}
		else if (nType == m_eForeignInsp_Foreign && (nCheck == m_eForeignInsp_TraceMode || nCheck == m_eForeignInsp_TraceModeNONE))
		{
			for (int a = 0; a < m_eForeignInsp_GrayBub; a++)
			{
				if (sBin[a].UseData(FR_BIN_DT_Use) == false)
					continue;
				if (nCheck == m_eForeignInsp_TraceMode)
				{
					if (sBin[a].UseData(FR_BIN_DT_INSP_TRACE))
						return TRUE;
				}
				else if (nCheck == m_eForeignInsp_TraceModeNONE)
				{
					if (sBin[a].UseData(FR_BIN_DT_INSP_TRACE_NONE))
						return TRUE;
				}
			}
			return FALSE;
		}
		else if (m_eForeignInsp_GrayBub > nType)
		{
			bRet = sBin[nType].UseData(FR_BIN_DT_Use);
			if (bRet)
			{
				switch (nCheck)
				{
				case m_eForeignInsp_3D:
					bRet = sBin[nType].UseData(FR_BIN_DT_3D);
					if (sBin[nType].UseData(FR_BIN_DT_HA_Under) || sBin[nType].UseData(FR_BIN_DT_HA_Upper))
						bRet = true;
					break;
				case m_eForeignInsp_2D:
					bRet = sBin[nType].UseData(FR_BIN_DT_Color) || sBin[nType].UseData(FR_BIN_DT_Gray);
					break;
				case m_eForeignInsp_SolderArea:
					bRet = sBin[nType].UseData(FR_BIN_DT_InspSolderArea);
					break;
				case m_eForeignInsp_2D_Color:
					bRet = sBin[nType].UseData(FR_BIN_DT_Color);
					break;
				case m_eForeignInsp_2D_Gray:
					bRet = sBin[nType].UseData(FR_BIN_DT_Gray);
					break;
				case m_eForeignInsp_BTM:
					bRet = sBin[nType].UseData(FR_BIN_DT_ExceptBTC);
					break;
				case m_eForeignInsp_SkipPart:
					bRet = sBin[nType].UseData(FR_BIN_DT_Insp_Skip_Part);
					break;
				default:
					break;
				}
			}
		}
		return bRet;
	}
}InspForeignInfo;
typedef struct tagForeignParamROI
{
	byte m_nType;
	int m_nModuleID;
	int m_nInspFR;
	int m_nPolyCNT;
	BOOL m_bSKIP;
	BOOL m_bPolygon;
	POINTF m_pPos;
	POINTF m_pSize;
	POINTF m_pROI[ZMPOLYPOINT_CNTS];
	POINTF m_pROIHit[ZMPOLYPOINT_ULTRA_CNTS];
	void Clone(tagForeignParamROI &inspAlgo)
	{
		inspAlgo.m_nType = m_nType;
		inspAlgo.m_bSKIP = m_bSKIP;
		inspAlgo.m_nModuleID = m_nModuleID;
		inspAlgo.m_nInspFR = m_nInspFR;
		inspAlgo.m_nPolyCNT = m_nPolyCNT;
		inspAlgo.m_bPolygon = m_bPolygon;
		inspAlgo.m_pPos.x = m_pPos.x;
		inspAlgo.m_pPos.y = m_pPos.y;
		inspAlgo.m_pSize.x = m_pSize.x;
		inspAlgo.m_pSize.y = m_pSize.y;
		memcpy(inspAlgo.m_pROI, m_pROI, sizeof(POINTF) * ZMPOLYPOINT_CNTS);
		memcpy(inspAlgo.m_pROIHit, m_pROIHit, sizeof(POINTF) * ZMPOLYPOINT_ULTRA_CNTS);
	}
}ForeignParamROI;
typedef struct tagForeignData
{
	BOOL m_bDEBUG;
	MPTI_InspectionMode m_vInspectionMode;
	int m_nProjectionmode;
	int m_nFovID;
	int m_nMode;
	int m_nBin;
	int m_nData;

	int m_nTotalCnt;
	int m_nContainModuleID;
	int m_nModule;

	int m_nWP_W;
	int m_nWP_H;
	int m_nFRROI;
	RECT m_rcFOV;
	RECT m_rcFOVMargin;

	int* m_pXList; // m_nTotalCnt
	int* m_pYList; // m_nTotalCnt
	int* m_pWList; // m_nTotalCnt
	int* m_pHList; // m_nTotalCnt
	int* m_pInspModule; 

	UCHAR* m_pDesImg;   //m_nWP_W * m_nWP_H
	UCHAR* m_pDesImgBin; // X
	UCHAR* m_pimgTopR; //m_nWP_W * m_nWP_H
	UCHAR* m_pimgTopG; //m_nWP_W * m_nWP_H
	UCHAR* m_pimgTopB; //m_nWP_W * m_nWP_H
	UCHAR* m_pimgWPInspA; //m_nWP_W *m_nWP_H

	float* m_p3D;  //m_nWP_W * m_nWP_H * sizeof(float)
	float* m_pimgFR;  //m_nWP_W * m_nWP_H * sizeof(float)
	float* m_pimgWP;  //m_nWP_W * m_nWP_H * sizeof(float)
	int* m_pimgSUB;  //m_nWP_W * m_nWP_H * sizeof(int)

	POINT vStickerArea[StickerCnt * StickerPo];
	bool UseData(int nType)
	{
		int nData = m_nData;
		return UseData(nData, nType);
	}
	bool UseData(int nData, int nType)
	{
		if ((nData & nType) == nType)
			return true;
		return false;
	}
	void SUB_DATA(long long nDst, int nSUB, int nModuleTotal, int* nArrModuleID, bool bSticker)
	{
		if (USE_SUB_IDX(nDst) == false)
			return;

		int nDstVal = GET_SUB(nDst);
		bool bEXCP = UseData(nSUB, FR_SUB_EXCP) || UseData(nDstVal, FR_SUB_EXCP);
		bool bPART = UseData(nSUB, FR_SUB_PART) || UseData(nDstVal, FR_SUB_PART);
		bool bBOTM = UseData(nSUB, FR_SUB_BOTM) || UseData(nDstVal, FR_SUB_BOTM);
		bool bAMIM = UseData(nSUB, FR_SUB_AMIM) || UseData(nDstVal, FR_SUB_AMIM);
		bool bHOLE = UseData(nSUB, FR_SUB_HOLE) || UseData(nDstVal, FR_SUB_HOLE);
		bool bStic = UseData(nSUB, FR_SUB_Sticker) || UseData(nDstVal, FR_SUB_Sticker) || bSticker;
		SET_SUB(nDst, FR_SUB_EXCP, bEXCP);
		SET_SUB(nDst, FR_SUB_PART, bPART);
		SET_SUB(nDst, FR_SUB_BOTM, bBOTM);
		SET_SUB(nDst, FR_SUB_AMIM, bAMIM);
		SET_SUB(nDst, FR_SUB_HOLE, bHOLE);
		SET_SUB(nDst, FR_SUB_Sticker, bStic);

		for (int nModuleIDX = nModuleTotal - 1; nModuleIDX >= 0; nModuleIDX--)
		{
			int nModuleID = nArrModuleID[nModuleIDX];
			if (nModuleIDX > 0 && nModuleID <= 0)
				continue;

			int nType = FR_SUB_MODU * (nModuleID + 1);
			bool bModule = UseData(nSUB, nType) || UseData(nDstVal, nType);
			SET_SUB(nDst, nType, bModule);
		}
	}
	int GET_SUB(long long nDst)
	{
		if (USE_SUB_IDX(nDst) == false)
			return 0;

		return m_pimgSUB[nDst];
	}
	int GET_SUB(int nX, int nY)
	{
		int nWP_W = m_nWP_W;
		int nWP_H = m_nWP_H;
		if (nX < 0 || nX >= nWP_W)
			return 0;
		if (nY < 0 || nY >= nWP_H)
			return 0;
		long long nDst = (nY * nWP_W) + nX;
		return GET_SUB(nDst);
	}
	void SET_SUB(long long nDst, int nType, bool bUSE)
	{
		if (USE_SUB_IDX(nDst) == false)
			return;

		if (bUSE)
			m_pimgSUB[nDst] |= nType;
		else
		{
			if (UseData(m_pimgSUB[nDst], nType))
				m_pimgSUB[nDst] -= nType;
		}
	}
	bool USE_SUB_IDX(long long nDst)
	{
		int nWP_W = m_nWP_W;
		int nWP_H = m_nWP_H;
		if (m_pimgSUB == NULL || m_pimgSUB == nullptr)
			return false;
		else if (nDst < 0 || nDst >= nWP_W * nWP_H)
			return false;

		return true;
	}
}ForeignData;

typedef struct tagFR_FIND_HIT
{
	int m_nData;
	int m_nGray;
	int m_nW;
	int m_nH;
	int m_nPer;
	RECT m_rcFOV;
	ColorXYInfoForeign m_vCol;

	UCHAR* m_pDesImg;
	POINTF m_pPo[POLYGON_BTM_POINT_CNTS];
	bool UseData(int nType)
	{
		if ((m_nData & nType) == nType)
			return true;
		return false;
	}
}FR_FIND_HIT;
typedef struct tagGB_PatternImg
{
	int m_nPixels_x;		// X pixel 개수
	int m_nPixels_y;		// Y pixel 개수
	UCHAR* vImg_pat;	    // Image (Gray Value)

}GB_PatternImg;

// 한 이물 결과
enum m_eFR_RST_N
{
	m_eFR_RST_N_OPT,
	m_eFR_RST_N_Mode,
	m_eFR_RST_N_ModuleID,
	m_eFR_RST_N_Range3D,
	m_eFR_RST_N_Defect,
	m_eFR_RST_N_UVColorSelect,
	m_eFR_RST_N_Total,
};
enum m_eFR_RST_F
{
	m_eFR_RST_F_X,
	m_eFR_RST_F_Y,
	m_eFR_RST_F_W,
	m_eFR_RST_F_L,
	m_eFR_RST_F_A,
	m_eFR_RST_F_H,
	m_eFR_RST_F_Total,
};
enum m_eFR_RST_D
{
	m_eFR_RST_D_HA = 0x01,
};
typedef struct tagAForeignResult
{
	int m_nArr[m_eFR_RST_N_Total];
	float m_fArr[m_eFR_RST_F_Total];

	RECT m_rcRect;		// FOV 내에서의 이물 좌표(Pixel)

	void Clone(tagAForeignResult &InspForeignRes)
	{
		memcpy(InspForeignRes.m_nArr, m_nArr, sizeof(int) * m_eFR_RST_N_Total);
		memcpy(InspForeignRes.m_fArr, m_fArr, sizeof(float) * m_eFR_RST_F_Total);
		InspForeignRes.m_rcRect = m_rcRect;
	}
	int GetN(int nType, bool bSet = false, int nSet = 0)
	{
		if (nType < 0 || nType >= m_eFR_RST_N_Total)
			return 0;
		if (bSet)
			m_nArr[nType] = nSet;
		return m_nArr[nType];
	}
	float GetF(int nType, bool bSet = false, float fSet = 0)
	{
		if (nType < 0 || nType >= m_eFR_RST_N_Total)
			return 0;
		if (bSet)
			m_fArr[nType] = fSet;
		return m_fArr[nType];
	}
	bool UseData(int nType)
	{
		int nData = m_nArr[m_eFR_RST_N_OPT];
		if ((nData & nType) == nType)
			return true;
		return false;
	}
	void SetData(int nType, bool bUSE)
	{
		int nData = m_nArr[m_eFR_RST_N_OPT];
		if (bUSE)
			nData |= nType;
		else
		{
			if (UseData(nType))
				nData -= nType;
		}
		m_nArr[m_eFR_RST_N_OPT] = nData;
	}
}AForeignResult;
typedef struct tagAForeignResultWP
{
	int nID;
	byte m_byWP;
	float m_fHeightMin;
	float m_fHeightMax;
	float m_fCXMin;
	float m_fCYMin;
	float m_fCXMax;
	float m_fCYMax;

	byte m_byAM;
	float m_fHeightMinAM;
	float m_fHeightMaxAM;
	float m_fCXMinAM;
	float m_fCYMinAM;
	float m_fCXMaxAM;
	float m_fCYMaxAM;

	void Clone(tagAForeignResultWP &InspForeignRes)
	{
		InspForeignRes.nID = nID;
		InspForeignRes.m_byWP = m_byWP;
		InspForeignRes.m_fHeightMin = m_fHeightMin;
		InspForeignRes.m_fHeightMax = m_fHeightMax;
		InspForeignRes.m_fCXMin = m_fCXMin;
		InspForeignRes.m_fCYMin = m_fCYMin;
		InspForeignRes.m_fCXMax = m_fCXMax;
		InspForeignRes.m_fCYMax = m_fCYMax;

		InspForeignRes.m_byAM = m_byAM;
		InspForeignRes.m_fHeightMinAM = m_fHeightMinAM;
		InspForeignRes.m_fHeightMaxAM = m_fHeightMaxAM;
		InspForeignRes.m_fCXMinAM = m_fCXMinAM;
		InspForeignRes.m_fCYMinAM = m_fCYMinAM;
		InspForeignRes.m_fCXMaxAM = m_fCXMaxAM;
		InspForeignRes.m_fCYMaxAM = m_fCYMaxAM;
	}
}AForeignResultWP;
// 한 FOV 내의 이물 결과
typedef struct tagInspFovForeignResult
{
	BOOL m_bOK;
	int m_nCountDefect;					// 이물 개수
	AForeignResult * m_stForeign;		// 이물 결과
	int m_nCountDefectWP;					// Warpage 개수
	AForeignResultWP * m_stForeignWP;		// Warpage 결과
	tagInspFovForeignResult()
	{
		m_bOK = FALSE;
		m_nCountDefect = 0;
		m_stForeign = NULL;
		m_nCountDefectWP = 0;
		m_stForeignWP = NULL;
	}

	~tagInspFovForeignResult()
	{
		if (m_stForeign)
			//delete [] m_stForeign;
			g_pMManager->pem_delete(m_stForeign, true);
		m_stForeign = NULL;
		if (m_stForeignWP)
			g_pMManager->pem_delete(m_stForeignWP, true);
		m_stForeignWP = NULL;
	}

	void Clone(tagInspFovForeignResult &InspForeignRes)
	{
		InspForeignRes.m_bOK = m_bOK;
		InspForeignRes.m_nCountDefect = m_nCountDefect;
		InspForeignRes.m_nCountDefectWP = m_nCountDefectWP;
		InspForeignRes.m_stForeign = g_pMManager->pem_new<AForeignResult>(true, m_nCountDefect, (PCHAR)__FUNCTION__, __LINE__);
		InspForeignRes.m_stForeignWP = g_pMManager->pem_new<AForeignResultWP>(true, m_nCountDefectWP, (PCHAR)__FUNCTION__, __LINE__);
		for (int i = 0; i < m_nCountDefect; i++)
		{
			m_stForeign[i].Clone(InspForeignRes.m_stForeign[i]);
		}
		for (int i = 0; i < m_nCountDefectWP; i++)
		{
			m_stForeignWP[i].Clone(InspForeignRes.m_stForeignWP[i]);
		}
	}

}InspFovForeignResult;

typedef struct tagInspFR_Save
{
	int m_nArrData[m_eInspFR_Save_Total];
	tagAForeignResult m_sforeign;

	wchar_t m_s3DPath[MAX_STRLEN];
	wchar_t m_s2DPath[MAX_STRLEN];
	wchar_t m_sName[MAX_STRLEN];

	wchar_t m_sMESPath[MAX_STRLEN];
	wchar_t m_sMESName[MAX_STRLEN];

	float* m_pfOff3D;

	UCHAR* m_pucTR;
	UCHAR* m_pucTG;
	UCHAR* m_pucTB;
	UCHAR* m_pucBR;
	UCHAR* m_pucBB;

	int m_nSave_F_MES;
	wchar_t m_sMES_F_Path[MAX_STRLEN];;

	bool UseData(int nData)
	{
		if ((m_nArrData[(int)m_eInspFR_Save_Data] & nData) == nData)
			return true;
		return false;
	}
}InspFR_Save;
typedef struct tagFR2DData
{
	int m_nCalc;
	int m_nFov;
	int m_nW;
	int m_nH;

	UCHAR* m_pFr1;
	UCHAR* m_pFr2;
	UCHAR* m_pFr3;
	UCHAR* m_pFr4;

	UCHAR* m_pucTR;
	UCHAR* m_pucTG;
	UCHAR* m_pucTB;
	UCHAR* m_pucBR;
	UCHAR* m_pucBB;
}FR2DData;
typedef struct tagFR_EXT_SRC
{
	int m_nPAD;
	int m_nFovW;
	int m_nFovH;
	int m_nProjectionmode;
	float* m_p3D;
	UCHAR* m_pDesImgBin;
}FR_EXT_SRC;
typedef struct tagFR_EXT_POLY
{
	int m_nPoly;
	RECT m_rcROI;
	POINT m_pROI[ZMPOLYPOINT_CNTS];
}FR_EXT_POLY;
typedef struct tagFR_EXT_RST
{
	FR_EXT_POLY m_pROI[FR_EXT_RST_MAX];
}FR_EXT_RST;