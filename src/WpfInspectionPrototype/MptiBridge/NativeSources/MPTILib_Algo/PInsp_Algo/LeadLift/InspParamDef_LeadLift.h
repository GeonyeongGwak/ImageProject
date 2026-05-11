#pragma once
#include "../InspParamDef_AlgoBase.h"

#define LEADLIFT_ROI_CNTS		2
enum m_eLeadLiftData
{
	m_eLeadLiftData_IPC = 0x01,
	m_eLeadLiftData_FilletHeight = 0x02,
};
enum m_eLeadLiftN
{
	m_eLeadLiftN_Data = 0,
	m_eLeadLiftN_FilletHeightFindType,
	m_eLeadLiftN_Total,
};
enum m_eLeadLiftF
{
	m_eLeadLiftF_FilletHeightROIInterval = 0,
	m_eLeadLiftF_FilletHeightROIWidth,
	m_eLeadLiftF_FilletHeightROIHeight,
	m_eLeadLiftF_CurrentFilletHeight,
	m_eLeadLiftF_FilletHeightMax,
	m_eLeadLiftF_FilletHeightMin,
	m_eLeadLiftF_CurrentFilletHeightPercent,
	m_eLeadLiftF_FilletHeightPercentMin,
	m_eLeadLiftF_Total,
};
enum m_eLeadLiftRData
{
	m_eLeadLiftRData_FilletHeight = 0x01,
	m_eLeadLiftRData_FilletHeightPercent = 0x02,
};
enum m_eLeadLiftR
{
	m_eLeadLiftR_FilletHeight = 0,
	m_eLeadLiftR_FilletHeightPercent,
	m_eLeadLiftR_Total,
};
enum m_eFilletHeightType_LeadLift
{
	m_eFilletHeightType_LeadLift_AVG,
	m_eFilletHeightType_LeadLift_MAX,
	m_eFilletHeightType_LeadLift_Num,
};
typedef struct tagAlgoLeadLift
{
	byte m_byIPCClass;

	int nROIWidth;
	int nROIHeight;
	int nLeadTipInterval;		// 간격
	int nLeadTipDirection;	// Lead 방향
	int nLeadPosition;			//Lead 위치

	double dAvgHeight3D;	// 검사 영역의 기준 높이값
	double dToleranceBand3D; // 3d 높이 허용 범위
	double m_dMinValue;		// Lead Lift Minimum

	BOOL m_bUseLeadLift;
	BOOL m_bUseCoplanarity;
	double m_dHeightDiif;
	BOOL m_bUseGradient;
	double m_dGradient;

	BOOL m_bUseColorRange;
	float m_fColorRangeMax;
	float m_fColorRangeMin;

	int m_nArr[m_eLeadLiftN_Total];
	float m_fArr[m_eLeadLiftF_Total];

	tagAlgoLeadLift()
	{
		m_byIPCClass = 0;

		nROIWidth = 10;
		nROIHeight = 10;
		nLeadTipInterval = 10;
		nLeadTipDirection = 0;
		nLeadPosition = 10;

		dAvgHeight3D = 0.0;
		dToleranceBand3D = 0.0;
		m_dMinValue = 0.0;

		m_bUseLeadLift = TRUE;
		m_bUseCoplanarity = FALSE;
		m_dHeightDiif = 0;
		m_bUseGradient = FALSE;
		m_dGradient = 0;

		m_bUseColorRange = FALSE;
		m_fColorRangeMax = 0;
		m_fColorRangeMin = 0;
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[m_eLeadLiftN_Data] & nType) == nType);
		return bRet;
	}
	float GetData(int nType)
	{
		if (nType < 0 || nType >= m_eLeadLiftF_Total)
			return 0;
		return m_fArr[nType];
	}
	int GetDataN(int nType)
	{
		if (nType < 0 || nType >= m_eLeadLiftN_Total)
			return 0;
		return m_nArr[nType];
	}
}AlgoLeadLift;

typedef struct tagRstAlgoLeadLift	// Lead Lift 검사 결과
{
	double m_dRstHeight;
	BOOL m_bInspCoplanarity;	// Lead Search 의  Coplanarity 옵션
	double m_dRstHeight_cop;	// Coplanarity가 TRUE인 경우 계산된 높이값

	// OK 판정 //
	BOOL m_bOKHeight;
	BOOL m_bOKHeightCop;
	BOOL m_bOKCoplanarity;
	BOOL m_bOKGradient;

	int m_nLeadCoplanarityCnt;
	int m_nArrLeadCoplanarityRst[BRIEDGE_CNT];
	double m_dRstGradient;

	int m_nArrLeadCopIDList[BRIEDGE_CNT];
	double m_dArrLeadCopValue[BRIEDGE_CNT];
	double m_dArrLeadCopLeadLiftVal[BRIEDGE_CNT];

	int m_nRect_I;
	RECT m_rcRect_I[LEADLIFT_ROI_CNTS];

	int m_nNG;
	float m_fArr[m_eLeadLiftR_Total];
	tagRstAlgoLeadLift()
	{
		Init();
	}
	void Init()
	{
		m_dRstHeight = 0.;
		m_bInspCoplanarity = FALSE;
		m_dRstHeight_cop = 0.;

		m_bOKHeight = FALSE;
		m_bOKHeightCop = FALSE;
		m_bOKCoplanarity = FALSE;
		m_bOKGradient = FALSE;

		for (int n = 0; n < BRIEDGE_CNT; n++)
		{
			m_nArrLeadCoplanarityRst[n] = -1;
			m_nArrLeadCopIDList[n] = -1;
			m_dArrLeadCopValue[n] = -1;
			m_dArrLeadCopLeadLiftVal[n] = -1;
		}
		m_dRstGradient = 0;


		m_nRect_I = 0;
		for (int a = 0; a < LEADLIFT_ROI_CNTS; a++)
		{
			m_rcRect_I[a].left = 0;
			m_rcRect_I[a].right = 0;
			m_rcRect_I[a].top = 0;
			m_rcRect_I[a].bottom = 0;
		}

		m_nNG = 0;
		memset(m_fArr, 0, m_eLeadLiftR_Total * sizeof(float));
	}
	void AddRect(RECT roi)
	{
		if (m_nRect_I < 0) m_nRect_I = 0;
		if (m_nRect_I >= LEADLIFT_ROI_CNTS)
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
}RstAlgoLeadLift;