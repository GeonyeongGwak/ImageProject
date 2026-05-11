#pragma once
#include "../InspParamDef_AlgoBase.h"

enum m_eFoot
{
	m_eFoot_DeviationX = 0,
	m_eFoot_DeviationY,
	m_eFoot_Deviation,
	m_eFoot_Height,
	m_eFoot_Damage,
	m_eFoot_Width_1,
	m_eFoot_Width_1_H,
	//	m_eFoot_Width_2,
	//	m_eFoot_Width_2_H,
	m_eFoot_Length_1,
	//	m_eFoot_Length_1_H,
	//	m_eFoot_Length_2,
	//	m_eFoot_Length_2_H,
	m_eFoot_Tail_L,
	m_eFoot_Tail_A,
	m_eFoot_Wedge_W,
	m_eFoot_Wedge_L,
	m_eFoot_Foreign,
	m_eFoot_HeelCrack,
	m_eFoot_WingArea,
	m_eFoot_Bonding1,
	m_eFoot_Bonding2,
	//	m_eFoot_BondingPosition,
	m_eFoot_Total,
};
enum m_eFootBin
{
	m_eFootBin_Pad = 0,
	m_eFootBin_Foot,
	m_eFootBin_Wedge,
	m_eFootBin_Wing,
	m_eFootBin_FindDBC,
	m_eFootBin_Total,
};
enum m_eFOOT_Data
{
	m_eFOOT_Data_ShiftX = 0x01,
	m_eFOOT_Data_ShiftY = 0x02,
	m_eFOOT_Data_Shift = 0x04,
	m_eFOOT_Data_Height = 0x08,
	m_eFOOT_Data_Damage = 0x10,
	m_eFOOT_Data_Width_1 = 0x20,
	m_eFOOT_Data_Width_1_H = 0x40,
	m_eFOOT_Data_Length_1 = 0x80,
	m_eFOOT_Data_Tail_L = 0x100,
	m_eFOOT_Data_Tail_A = 0x200,
	m_eFOOT_Data_Wedge_W = 0x400,
	m_eFOOT_Data_Wedge_L = 0x800,
	m_eFOOT_Data_Foreign = 0x1000,
	m_eFOOT_Data_HeelCrack = 0x2000,
	m_eFOOT_Data_WingArea = 0x4000,
	// = 0x8000,
	// = 0x10000,
	// = 0x20000,
	// = 0x40000,
	// = 0x80000,
	m_eFOOT_Data_Total = 0xFFFF//F,
};
enum m_eFOOT_Data2
{
	m_eFOOT_Data2_RefArea = 0x01,
	m_eFOOT_Data2_Find = 0x02,
	m_eFOOT_Data2_Find_Bin3D = 0x04,
	m_eFOOT_Data2_Find_RefA = 0x08,
	m_eFOOT_Data2_Dir_Up = 0x10,
	m_eFOOT_Data2_DBC = 0x20,
	m_eFOOT_Data2_Align = 0x40,
	m_eFOOT_Data2_Total = 0xFF,
};
enum m_eFoot_FindOption
{
	Auto = 0,
	Wing,
	Bonding,
	Wing3D,
	Slope3D,
	WingGap,
	MeanSlope,
	StdDevSlope,
	total
};
enum m_eFoot_Type
{
	Wedge_bonding = 0,
	Ribbon_bonding,
	total_bonding
};

typedef struct tagAlgoBin
{
	BOOL m_bIsSet;
	byte m_byArrValue[m_eBin_num];
	float m_fArrH[eMMD_Total];
	byte m_byArrLightData[eLightData_Total][LIGHT_CNT];
	BOOL Moph;
	tagAlgoBin()
	{
		m_bIsSet = false;
		Moph = false;
		for (int a = 0; a < (int)m_eBin_num; a++)
		{
			m_byArrValue[a] = 0;
			if (a < eMMD_Total)
				m_fArrH[a] = 0;
			if (a < eLightData_Total)
			{
				for (int b = 0; b < LIGHT_CNT; b++)
					m_byArrLightData[a][b] = 0;
			}
		}
	}
}AlgoBin;

typedef struct tagAlgoFoot
{
	BOOL m_bOffset;		//NYJ 2019/12/18
	int m_nUseOption;
	int m_nUseOption2;
	int m_nFindOption;
	int m_nFindOption2;
	BOOL m_bUsePatternAngle;
	RECT m_sRefArea;
	struct tagAlgoBin m_sArrBin[m_eFootBin_Total];
	POINTF  m_sPoint;
	RECT m_rSpecRect;
	float m_fArrOptionValue[m_eFoot_Total][eMMD_Total];
	RECT m_rInspRect;
	int m_nFindOptionOrder[m_eFoot_FindOption::total];
	int m_nFindOptionOrder2[m_eFoot_FindOption::total];
	int m_nFootType;
	int m_nFootAngle;
	float m_fPadAreaAvgHeight;
	double m_dTeachFootAngle;
	BOOL m_bUse2Foot = FALSE;
	BOOL m_bUsePadAreaAutoTeach = TRUE;		//Foot-pad영역 자동 티칭기능 기본 값 :true

	bool bUseTeachRect;
	int nPadWidth;
	int nPadHeight;
	int nRadius;
	bool bUseRemovedPadArea;
	BOOL bUse2DImageForInspWidth;
	int nRemoveWireHeight;
	int nFootSobmin;
	int nFootSobmax;
	int nFootPadSizeX;
	int nFootPadSizeY;

	int nPadEdgeMinRatio;		//pad edge minimum white pixel ratio
	BOOL bUseDBCShapeTeaching;

	int nInspCrackHeight;
	tagAlgoFoot()
	{
		m_bOffset = FALSE;	//NYJ 2019/12/18
		m_nUseOption = m_eFOOT_Data_Total;
		m_nUseOption2 = m_eFOOT_Data2_Total;
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
		for (int a = 0; a < m_eFootBin_Total; a++)
		{
			for (int b = 0; b < eLightData_Total; b++)
			{
				m_sArrBin[a].m_byArrValue[b] = 0;
				for (int c = 0; c < LIGHT_CNT; c++)
					m_sArrBin[a].m_byArrLightData[b][c] = 0;
			}
			for (int b = 0; b < eMMD_Total; b++)
			{
				m_fArrOptionValue[a][b] = 0;
				m_sArrBin[a].m_fArrH[b] = 0.0f;
			}
		}
		m_nFootType = 0;
		m_fPadAreaAvgHeight = 0.0f;
		m_dTeachFootAngle = 0.0f;
		m_bUse2Foot = FALSE;
		m_bUsePadAreaAutoTeach = TRUE;
		bUse2DImageForInspWidth = FALSE;
		nRemoveWireHeight = 200;
		nFootSobmin = 0;
		nFootSobmax = 0;
		nFootPadSizeX = 900;
		nFootPadSizeY = 900;
		nPadEdgeMinRatio = 50;
		bUseTeachRect = FALSE;
		bUseDBCShapeTeaching = FALSE;
		nInspCrackHeight = 120;
	}
}AlgoFoot;

typedef struct tagRstAlgoFoot
{
	BOOL m_bOK;
	int m_nFindBin;
	BOOL m_bFind;
	BOOL m_bArrOK[m_eFoot::m_eFoot_Total];
	float m_fArrRst[m_eFoot::m_eFoot_Total];

	POINTF m_poWire_Wnd;
	RECT m_rcRefArea;
	RECT m_rcSearchArea;
	POINT rePoinSt[m_eFoot::m_eFoot_Total];
	POINT rePoinEd[m_eFoot::m_eFoot_Total];
	int m_nMatchModelNum;
	POINTF  m_sPoint;
	float m_fAreaHeight;
	float m_fAngle;
	POINTF  m_sEnd;
	float m_fArrRst_WingArea[4];
	int m_stWingFunc;
	int m_edWingFunc;

	//pitting rect
	POINT m_nTeachRectLT;
	POINT m_nTeachRectLB;
	POINT m_nTeachRectRB;
	POINT m_nTeachRectRT;

	//matching score
	float MatchingScore;

	POINT DispPoinSt[m_eFoot::m_eFoot_Total];
	POINT DispPoinEd[m_eFoot::m_eFoot_Total];

	int m_nPadWidth;
	int m_nPadHeight;
	int m_nRadius;
	tagRstAlgoFoot()
	{
		m_bOK = TRUE;
		m_nFindBin = 0;
		m_poWire_Wnd.x = 0;
		m_poWire_Wnd.y = 0;
		m_fAreaHeight = 0.0f;
		m_bFind = false;
		memset(m_bArrOK, 0, sizeof(BOOL)*m_eFoot::m_eFoot_Total);
		memset(m_fArrRst, 0.0, sizeof(float)*m_eFoot::m_eFoot_Total);
		memset(rePoinSt, 0, sizeof(POINT)*m_eFoot::m_eFoot_Total);
		memset(rePoinEd, 0, sizeof(POINT)*m_eFoot::m_eFoot_Total);
		m_sPoint.x = 0.0f;
		m_sPoint.y = 0.0f;
		m_fAngle = 0.0f;
		m_sEnd.x = 0.0f;
		m_sEnd.y = 0.0f;
		memset(m_fArrRst_WingArea, 0, sizeof(float) * 4);
		for (int i = 0; i < m_eFoot::m_eFoot_Total; i++)
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

		m_stWingFunc = 0;
		m_edWingFunc = 0;
	}
}RstAlgoFoot;