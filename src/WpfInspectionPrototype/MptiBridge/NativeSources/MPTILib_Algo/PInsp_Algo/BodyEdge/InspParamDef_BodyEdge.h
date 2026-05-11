#pragma once
#include "../InspParamDef_AlgoBase.h"

//★★ 코드 작성 필요
enum m_eBodyEdge_N
{
	BodyEdge_N_Data,
	BodyEdge_N_Filter,
	BodyEdge_N_Min2D,
	BodyEdge_N_Max2D,
	BodyEdge_N_Range2D,
	BodyEdge_N_Range3D,
	BodyEdge_N_AreaNum,
	BodyEdge_N_SelectBlob,
	BodyEdge_N_SubLineMin2D,
	BodyEdge_N_SubLineMax2D,
	BodyEdge_N_SubLineRange2D,
	BodyEdge_N_SubLineRange3D,
	BodyEdge_N_Total,
};

enum m_eBodyEdge_Data
{
	BodyEdge_Data_Bin2D = 1 << 0,
	BodyEdge_Data_Bin3D = 1 << 1,
	BodyEdge_Data_Filter = 1 << 2,
	BodyEdge_Data_FillHoll = 1 << 3,
	BodyEdge_Data_UseShift = 1 << 4,
	BodyEdge_Data_UseShiftX = 1 << 5,
	BodyEdge_Data_UseShiftY = 1 << 6,
	BodyEdge_Data_UseAngle = 1 << 7,
	BodyEdge_Data_UseWidth = 1 << 8,
	BodyEdge_Data_UseLength = 1 << 9,
	BodyEdge_Data_UseDistance = 1 << 10,
	BodyEdge_Data_UseSubLine = 1 << 11,
	BodyEdge_Data_UseSubLineBin2D = 1 << 12,
	BodyEdge_Data_UseSubLineBin3D = 1 << 13,
};

enum m_eBodyEdge_F
{
	BodyEdge_F_Min3D,
	BodyEdge_F_Max3D,
	BodyEdge_F_TechCenterX,
	BodyEdge_F_TechCenterY,
	BodyEdge_F_ShiftX,
	BodyEdge_F_ShiftY,
	BodyEdge_F_Angle,
	BodyEdge_F_StandardAngle,
	BodyEdge_F_Width,
	BodyEdge_F_WidthRateMin,
	BodyEdge_F_WidthRateMax,
	BodyEdge_F_Length,
	BodyEdge_F_LengthRateMin,
	BodyEdge_F_LengthRateMax,
	BodyEdge_F_TeachDistanceX,
	BodyEdge_F_TeachDistanceY,
	BodyEdge_F_TeachToleranceX,
	BodyEdge_F_TeachToleranceY,
	BodyEdge_F_SubLineMin3D,
	BodyEdge_F_SubLineMax3D,
	BodyEdge_F_Total,
};
enum m_eAnchorMode
{
	Anchor,
	TwoAnchor,
	TwoAnchorCenter,
	OtherEdge,
	NoneAnchorMode,
};

const int BODY_EDGE_RECT_CNTS = 16; //BoyEdge Algo Area


typedef struct tagAlgoBodyEdge
{
	int narrdata[BodyEdge_N_Total];
	float farrdata[BodyEdge_F_Total];
	RECT m_rcRoi[BODY_EDGE_RECT_CNTS]; // tilt roi 구조로 변경 해야함.
	int m_nArrDirection[BODY_EDGE_RECT_CNTS];
	POINTF m_sArrTeachPoint[4];

	int m_nSelectIdxAnchorMode;
	int m_nSelectIdxWindow;

	tagAlgoBodyEdge()
	{
		memset(narrdata, 0, BodyEdge_N_Total * sizeof(int));
		memset(farrdata, 0, BodyEdge_F_Total * sizeof(float));
		memset(m_rcRoi, 0, BODY_EDGE_RECT_CNTS * sizeof(RECT));
		memset(m_nArrDirection, 0, BODY_EDGE_RECT_CNTS * sizeof(int));
		memset(m_sArrTeachPoint, 0, 4 * sizeof(POINTF));

		m_nSelectIdxAnchorMode = -1;
		m_nSelectIdxWindow = -1;
	}
}AlgoBodyEdge;



typedef struct tagRstAlgoBodyEdge
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
	RECT m_rcArea_T[BODY_EDGE_RECT_CNTS];
	RECT m_rcArea_I[BODY_EDGE_RECT_CNTS];
	//	BOOL m_bArea_OK[16];

	RECT m_rcBodyRect;
	POINTF m_ptInspEdge[4]; //좌상, 우상, 우하, 좌하 Part Pixel 좌표
	POINTF m_poDrawCenter;

	tagRstAlgoBodyEdge()
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
	}
}RstAlgoBodyEdge;