#pragma once
#include "../InspParamDef_AlgoBase.h"

//★★ 코드 작성 필요

#define DISTANCE_ANCHOR_LIMIT 10

enum m_eDistanceData
{
	m_eDistanceData_UseX = 0x01,
	m_eDistanceData_UseY = 0x02,
	m_eDistanceData_UseDist = 0x04,
	m_eDistanceData_UseAngle = 0x08,
	m_eDistanceData_UseTargetAxes = 0x10,
	m_eDistanceData_CoordinateLog = 0x20,
	m_eDistanceData_UseTargetPnt = 0x40,
	m_eDistanceData_Total = 0xFF,
};
enum m_eDistanceN
{
	m_eDistanceN_Data,
	m_eDistanceN_AnchorMode,
	m_eDistanceN_AnchorCnt,
	m_eDistanceN_Total,
};
enum m_eDistanceF
{
	m_eDistanceF_TeachX,
	m_eDistanceF_TeachY,
	m_eDistanceF_TeachDist,
	m_eDistanceF_TeachTolerX,
	m_eDistanceF_TeachTolerY,
	m_eDistanceF_TeachTolerDist,
	m_eDistanceF_TeachAngle,
	m_eDistanceF_TeachOffsetX,
	m_eDistanceF_TeachOffsetY,
	m_eDistanceF_TeachOffsetDist,
	m_eDistanceF_TeachTolerAngle,
	m_eDistanceF_Total,
};
enum m_eDistanceAnchorMode
{
	m_eDistanceAnchorMode_Anchor,
	m_eDistanceAnchorMode_TwoAnchor,
	m_eDistanceAnchorMode_TwoAnchorCenter,
	m_eDistanceAnchorMode_RectAnchor,
	m_eDistanceAnchorMode_LineTheta,
	m_eDistanceAnchorMode_LineDistance,
	m_eDistanceAnchorMode_NoneAnchorMode,
};

typedef struct tagAlgoDistance
{
	//★★ 코드 수정 필요
	int m_ArrDataN[m_eDistanceN_Total];
	float m_ArrDataF[m_eDistanceF_Total];

	int m_nTargetWindowID;
	int m_nTargetAlgoID;
	int m_nTargetAlgoType;

	int m_arrAnchorWindowID[DISTANCE_ANCHOR_LIMIT];
	int m_arrAnchorAlgoID[DISTANCE_ANCHOR_LIMIT];
	int m_arrAnchorAlgoType[DISTANCE_ANCHOR_LIMIT];
	POINTF m_arrAnchorPos[DISTANCE_ANCHOR_LIMIT];

	POINTF m_ptTargetPnt;

	tagAlgoDistance()
	{
		memset(m_ArrDataN, 0, m_eDistanceN_Total * sizeof(int));
		memset(m_ArrDataF, 0, m_eDistanceF_Total * sizeof(float));

		m_nTargetWindowID = 0;
		m_nTargetAlgoID = 0;
		m_nTargetAlgoType = 0;

		memset(m_arrAnchorWindowID, 0, DISTANCE_ANCHOR_LIMIT * sizeof(int));
		memset(m_arrAnchorAlgoID, 0, DISTANCE_ANCHOR_LIMIT * sizeof(int));
		memset(m_arrAnchorAlgoType, 0, DISTANCE_ANCHOR_LIMIT * sizeof(int));
		memset(m_arrAnchorPos, 0, DISTANCE_ANCHOR_LIMIT * sizeof(POINTF));
	}
}AlgoDistance;

typedef struct tagRstAlgoDistance // Distance 검사 결과
{
	//★★ 코드 수정 필요
	BOOL m_bOKX;
	BOOL m_bOKY;
	BOOL m_bOKDist;
	BOOL m_bOKAngle;

	BOOL m_bOKTarget;
	BOOL m_bArrOKAnchor[DISTANCE_ANCHOR_LIMIT];

	double m_dRstX;
	double m_dRstY;
	double m_dRstDist;
	double m_dRstAngle;

	POINTF m_pTarget;
	POINTF m_pArrAnchor[DISTANCE_ANCHOR_LIMIT];

	POINTF m_pTargetRotated;
	POINTF m_pArrAnchorRotated[DISTANCE_ANCHOR_LIMIT];

	POINTF m_dRstRectCenter[2];
	double m_dRstRectWidth;
	double m_dRstRectHeight;
	double m_dRstRectTheta;

	int m_nRstBodyX;
	int m_nRstBodyY;
	int m_nRstBodyWidth;
	int m_nRstBodyLength;

	tagRstAlgoDistance()
	{
		m_bOKX = FALSE;
		m_bOKY = FALSE;
		m_bOKDist = FALSE;
		m_bOKAngle = FALSE;

		m_bOKTarget = FALSE;
		memset(&m_bArrOKAnchor, 0, sizeof(BOOL) * DISTANCE_ANCHOR_LIMIT);

		m_dRstX = 0.0;
		m_dRstY = 0.0;
		m_dRstDist = 0.0;
		m_dRstAngle = 0.0;

		memset(m_pArrAnchor, 0, sizeof(POINTF) * DISTANCE_ANCHOR_LIMIT);
		memset(&m_pTarget, 0, sizeof(POINTF));

		memset(m_pArrAnchorRotated, 0, sizeof(POINTF) * DISTANCE_ANCHOR_LIMIT);
		memset(&m_pTargetRotated, 0, sizeof(POINTF));

		memset(m_dRstRectCenter, 0, sizeof(POINTF) * 2);
		m_dRstRectWidth = 0.0;
		m_dRstRectHeight = 0.0;
		m_dRstRectTheta = 0.0;

		m_nRstBodyX = 0;
		m_nRstBodyY = 0;
		m_nRstBodyWidth = 0;
		m_nRstBodyLength = 0;
	}
}RstAlgoDistance;

typedef struct tagAlgoDistanceParam
{
	int nWindowIdx;
	int nAlgoIdx;
	void *wndResult;
	int nAlignCnt;
	void * ptrAlignRes;

	tagAlgoDistanceParam(int wndIdx, int algoIdx, void *result, int alignCnt, void * ptrAlign)
	{
		nWindowIdx = wndIdx;
		nAlgoIdx = algoIdx;
		wndResult = result;
		nAlignCnt = alignCnt;
		ptrAlignRes = ptrAlign;
	}
}AlgoDistanceParam;