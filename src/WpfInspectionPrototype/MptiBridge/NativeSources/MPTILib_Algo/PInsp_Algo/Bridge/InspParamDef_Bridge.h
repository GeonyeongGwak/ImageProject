#pragma once
#include "../InspParamDef_AlgoBase.h"
#define defBridgeGrayMaxValue 100
enum m_eOPT_Bridge
{
	m_eOPT_Bridge_Area,
	m_eOPT_Bridge_Width,
	m_eOPT_Bridge_Length,
	m_eOPT_Bridge_Total,
};
enum m_eData_Bridge
{
	m_eData_Bridge_Use_Type2 = 0x01,
	m_eData_Bridge_Area = 0x02,
	m_eData_Bridge_Width = 0x04,
	m_eData_Bridge_Length = 0x08,
};
enum m_eBridge_Defect
{
	m_eBridge_Defect_2D = 0x01,
	m_eBridge_Defect_3D = 0x02,
	m_eBridge_Defect_SB = 0x04,
	m_eBridge_Defect_A = 0x08,

	m_eBridge_Defect_W = 0x10,
	m_eBridge_Defect_L = 0x20,
};

typedef struct tagAlgoBridge
{
	int nGrayDiff;
	int nLeadTipDirection;

	BOOL bUse3D;
	double m_dHeightReferenceValue;
	int nGapCnt;
	RECT ptrrcGapRect[BRIEDGE_CNT];

	BOOL m_bOffset;

	int m_nTypeBridge;
	double m_dPercentOK;

	BOOL m_bUseInsp2D;
	BOOL m_bUseInsp3D;
	float m_fHeightDiff;
	BOOL m_bAutoSearchROI;

	BOOL m_bUseSolderBall;
	float m_fSolderBallArea;
	float m_fSolderBallAreaPer;

	BOOL m_bInsp2DUpper;
	float m_fThinBridge;
	byte m_by2D3DLine;

	struct tagAlgoBlobBase m_sBlobBase;

	BOOL m_bUseMode2;
	int m_nOption;
	float m_fArrValue[m_eOPT_Bridge_Total];
	enum ETypeBridge
	{
		eGeneral,
		eTab,
	};

	tagAlgoBridge()
	{
		nGrayDiff = 0;
		nLeadTipDirection = 0;

		nGapCnt = 0;

		bUse3D = FALSE;
		m_dHeightReferenceValue = 0;

		m_bOffset = FALSE;

		m_nTypeBridge = (int)ETypeBridge::eGeneral;
		m_dPercentOK = 60;

		m_bUseInsp2D = TRUE;
		m_bUseInsp3D = FALSE;
		m_fHeightDiff = 0.0;

		m_bUseSolderBall = FALSE;
		m_fSolderBallArea = 0.0f;
		m_fSolderBallAreaPer = 0.0f;

		m_bInsp2DUpper = true;
		m_fThinBridge = 0.03f;
		m_by2D3DLine = 90;

		m_bUseMode2 = false;
		m_nOption = 0;
		memset(m_fArrValue, 0, sizeof(float) * m_eOPT_Bridge_Total);
	}
}AlgoBridge;

typedef struct tagRstAlgoBridge	// Bridge 검사 결과
{
	int m_nGapCnt;					// Gap Rect  개수
	BOOL m_bLeadBridgeRst[BRIEDGE_CNT];	// 각 Gap Rect  Bridge 결과
	UCHAR m_ucBridgeRectValue[BRIEDGE_CNT];
	float m_fBridgeRectValue[BRIEDGE_CNT];
	float m_fBridgeRectValue2D3D[BRIEDGE_CNT];
	POINTF m_pofNGRectCenter[BRIEDGE_CNT];
	POINTF m_pofNGRectSize[BRIEDGE_CNT];
	RECT m_rcArrGapRect[BRIEDGE_CNT];
	int m_nLeadBridgeRst[BRIEDGE_CNT];
	float m_fSolderBallRectValuePer[BRIEDGE_CNT];
	RECT m_rcArrROI[BRIEDGE_CNT];
	float m_fAreaRectValuePer[BRIEDGE_CNT];
	float m_fWidthRectValuePer[BRIEDGE_CNT];
	float m_fLengthRectValuePer[BRIEDGE_CNT];
	tagRstAlgoBridge()
	{
		m_nGapCnt = 0;
		memset(m_bLeadBridgeRst, 0, BRIEDGE_CNT * sizeof(BOOL));
		memset(m_ucBridgeRectValue, 0, BRIEDGE_CNT * sizeof(UCHAR));
		memset(m_fBridgeRectValue2D3D, 0, BRIEDGE_CNT * sizeof(UCHAR));
		memset(m_fBridgeRectValue, 0, BRIEDGE_CNT * sizeof(float));
		memset(m_pofNGRectCenter, 0, BRIEDGE_CNT * sizeof(POINTF));
		memset(m_pofNGRectSize, 0, BRIEDGE_CNT * sizeof(POINTF));
		memset(m_rcArrGapRect, 0, BRIEDGE_CNT * sizeof(RECT));
		memset(m_nLeadBridgeRst, 0, BRIEDGE_CNT * sizeof(int));
		memset(m_fSolderBallRectValuePer, 0, BRIEDGE_CNT * sizeof(float));
		memset(m_rcArrROI, 0, BRIEDGE_CNT * sizeof(RECT));
		memset(m_fAreaRectValuePer, 0, BRIEDGE_CNT * sizeof(float));
		memset(m_fWidthRectValuePer, 0, BRIEDGE_CNT * sizeof(float));
		memset(m_fLengthRectValuePer, 0, BRIEDGE_CNT * sizeof(float));
	}
	void Init()
	{
		m_nGapCnt = 0;
	}
}RstAlgoBridge;