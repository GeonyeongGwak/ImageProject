#pragma once
#include "../InspParamDef_AlgoBase.h"

#define WIRE_FOOT_CNT 10
#define WIRE_MAX_CNT 10
#define WIRE_DRAW_CNT 2
#define WIRE_HEIGHT_CNT 2
#define WIRE_DOT_CNT 500
#define WIRE_DOT3_CNT 20
#define INT_DOUBLE_DP 1000

enum m_eWIRE_Select_Point
{
	eWire_StartPoint,
	eWire_EndPoint,
	eWire_Height_Select_1 , // 0, 1 StartPoint , 2 ,3 
	eWire_Height_Select_2,
	eWire_Select_Max
};
enum m_eWIRE
{
	eWIRE_Warp = 0,
	eWIRE_Thickness,
	eWIRE_Distance,
	eWIRE_Height,
	eWIRE_Highest,
	eWIRE_HeightSub, // 고객 요청으로 높이 위치 변경을 위함
	eWIRE_SteepSlope,
	eWIRE_Interval,
	eWIRE_Height2,

	eWIRE_Total,
};
enum m_eWIRE2
{
	eWIRE2_Dot = 0,
	eWIRE2_UseOption,
	eWIRE2_UseOption2,
	eWIRE2_DivCnt,
	eWIRE2_SteepSlopeMargin,
	eWIRE2_Total,
};
enum m_eWIRE_Data
{
	eWIRE_Data_Warp = 0x01,
	eWIRE_Data_Thickness = 0x02,
	eWIRE_Data_Distance = 0x04,
	eWIRE_Data_Height = 0x08,
	eWIRE_Data_Highest = 0x10,
	eWIRE_Data_SteepSlope = 0x20,
	eWIRE_Data_Interval = 0x40,
	eWIRE_Data_Height2 = 0x80,
	eWIRE_Data_Height_Sub = 0x100, //SPC 고려 맨아래쪽 배치 
	//eWIRE_Data_Interval = 0x100,
	//eWIRE_Data_Height2 = 0x200,
};
enum m_eWIRE_Data2
{
	m_eWIRE_Data2_RefArea = 0x01,
	m_eWIRE_Data2_Max = 0x02,
	m_eWIRE_Data2_FootPos = 0x04,
	m_eWIRE_Data2_StartPointHeightSearch = 0x08,
	m_eWIRE_Data2_Total = 0x0F,
};
enum m_eWIRE_DOT
{
	eDot_Highest = 0,
	eDot_Warp,
	eDot_TH,
	eDot_TH1,
	eDot_TH2,
	eDot_LowH,
	eDot_Height_Sub,
	eDot_Total,
};

enum m_eWire_RefAreaType
{
	DrawROI = 0,
	Foot1_pad,
	Foot2_pad,
	wireRef_total,
};

typedef struct tagAlgoWire
{
	byte m_byFootCnt;
	int  m_nArrData[eWIRE2_Total];
	RECT m_sWireRect;
	POINTF  m_sArrPoint[WIRE_DRAW_CNT + WIRE_HEIGHT_CNT];
	float m_fArrDistance;
	float m_fArrOptionValue[eWIRE_Total][eMMD_Total];
	int  m_nWireRefAreaOpt;	//NYJ 2021/03/09
	bool m_bDirectionH;
	float m_fWireRefAreaH; //Teaching 때만 사용.
	float m_fHeightSubOffset;
	tagAlgoWire()
	{
		m_bDirectionH = 0;
		m_byFootCnt = 0;
		m_sWireRect.left = 0;
		m_sWireRect.right = 0;
		m_sWireRect.top = 0;
		m_sWireRect.bottom = 0;
		m_fArrDistance = 0;
		memset(m_nArrData, 0, eWIRE2_Total * sizeof(int));
		memset(m_sArrPoint, 0, WIRE_DRAW_CNT + WIRE_HEIGHT_CNT * sizeof(POINTF));

		for (int a = 0; a < eWIRE_Total; a++)
		{
			memset(m_fArrOptionValue[a], 0, eMMD_Total * sizeof(float));
		}

		m_nWireRefAreaOpt = 0;
		m_fWireRefAreaH = 0; 
		m_fHeightSubOffset = 0;
	}
}AlgoWire;

typedef struct tagRstAlgoWire
{
	int m_nWireCnt;
	BOOL m_bOK;
	BOOL m_bArrOK[eWIRE_Total];
	byte m_bArrNGType;
	int m_nArrCnt;
	float m_fArrRst[eWIRE_Total];
	float m_fArrValue_H[WIRE_DOT_CNT];
	float m_fArrValue_W[WIRE_DOT_CNT];
	float m_fArrValue_H2[WIRE_DOT_CNT];
	float m_fArrValue_T[WIRE_DOT3_CNT];
	float m_fArrValue_I[WIRE_DOT3_CNT];
	int m_nArrX[eDot_Total];
	int m_nArrY[eDot_Total];
	int m_nArrX_T[WIRE_DRAW_CNT];
	int m_nArrY_T[WIRE_DRAW_CNT];
	int m_nArrX_I[WIRE_DRAW_CNT];
	int m_nArrY_I[WIRE_DRAW_CNT];
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
	tagRstAlgoWire()
	{

	}
	void Init()
	{
	}
}RstAlgoWire;

typedef struct tagFootPos
{
	RECT m_sArrPadRect[WIRE_DRAW_CNT];//shkim  background Area(searchArea)
	POINTF  m_sArrFootPoint[WIRE_DRAW_CNT];//shkim foot Center

}FootPos;
typedef struct tagWireSearch // shkim AutoSearch result
{
	int WireCnt;
	FootPos m_FootPos[WIRE_MAX_CNT];

}WireSearch;
typedef struct tagFootColor // shkim AutoSearch result
{
	byte nRed;
	byte nGreen;
	byte nBlue;
}FootColor;
typedef struct tagWireRst //Wire Short
{
	AlgoWire* m_Wire;
	RstAlgoWire* m_RstWire;
} WireRst;