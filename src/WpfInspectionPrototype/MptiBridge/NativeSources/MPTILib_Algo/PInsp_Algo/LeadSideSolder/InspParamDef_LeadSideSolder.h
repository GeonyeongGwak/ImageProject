#pragma once
#include "../BW/InspParamDef_BW.h"

const int _nSideSolderCnt = 5;
const int _nSideSolderRelCnt = 3;
typedef struct tagAlgoLeadSideSolder
{
	struct tagAlgoBlackWhite sAlgoBW;	// 검사용 BW

	int m_nLeadTipDirection;	// Lead 방향
	int m_nLeadPosition;			//Lead 위치
	//검사영역
//	RECT m_ptrrcInspArea[_nSideSolderCnt];
	POINT m_ptStart[_nSideSolderCnt];
	int m_nRectWidth[_nSideSolderCnt];
	int m_nRectHeight[_nSideSolderCnt];

	double m_dLeadLiftSetValue;	// LeadLift 설정 높이
	BOOL m_bUseTeachArea;
	float m_fTeachArea;
	byte m_byInspArea;
	struct tagAngleColorBase m_sAngleColorBase;
	tagAlgoLeadSideSolder()
	{
		m_nLeadTipDirection = 0;
		m_nLeadPosition = 0;
		m_dLeadLiftSetValue = 0.0;
		m_bUseTeachArea = 0;
		m_byInspArea = 0;
		for (int n = 0; n < _nSideSolderCnt; n++)
		{
			m_nRectWidth[n] = 0;
			m_nRectHeight[n] = 0;
		}
	}
}AlgoLeadSideSolder;

typedef struct tagRstAlgoLeadSideSolder	// Lead SideSolder 검사 결과
{
	double m_dRstBWPercent;

	// OK 판정 //
	BOOL m_bOKBW;

	RECT m_rcRect_I[3];
	struct tagRstInspAC m_sInspAC;
	tagRstAlgoLeadSideSolder()
	{
		m_dRstBWPercent = 0.;
		m_bOKBW = FALSE;

		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
	}
	void Init()
	{
		m_dRstBWPercent = 0.;
		m_bOKBW = FALSE;

		m_sInspAC.m_nOKInspAC = 1;
		m_sInspAC.m_nInspAC = 0;
		//memset(m_sInspAC.m_fRstInspAC, 0, InspAC_R_Total * sizeof(float));
	}
}RstAlgoLeadSideSolder;