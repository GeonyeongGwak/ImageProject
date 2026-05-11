#pragma once
#include "../InspParamDef_AlgoBase.h"
#define  SOLDERVOL_LEVEL_COUNT	10

typedef struct tagAlgoSolderCone
{
	double m_dHeightLv1;
	double m_dHeightLv2;
	double m_dHeightLv3;
	double m_dTeachHeight;

	BOOL m_bUseSolderHighestVolumeDiff;		
	BOOL m_bUseInspVolumeDiff;		//VolumeDiff 검사 진행 여부
	BOOL m_bUseInspVolumeMinLength;		//VolumeDiff 검사 시, Min 길이 검사 유무
	BOOL m_bUseInspVolumePie;		//VolumePie 검사 진행 여부

	double m_dHighestHeight;		//Volume 최고 높이
	double m_dGapH;					//Volume 최고/최저 높이로 부터 gap 만큼의 높이 (50um)
	int m_nLevelCounts;				//VolumeDiff 검사 구간 개수

	float m_fPieHeightMaxDiff;

	float m_fArrVolumeDiff[SOLDERVOL_LEVEL_COUNT][eMMD_Total];
	BOOL m_bArrUseVolumeDiffInsp[SOLDERVOL_LEVEL_COUNT];
	BOOL m_bArrUseVolumeMinLengthInsp[SOLDERVOL_LEVEL_COUNT];

	tagAlgoSolderCone()
	{
		m_dHeightLv1 = 0.0;
		m_dHeightLv2 = 0.0;
		m_dHeightLv3 = 0.0;
		m_dTeachHeight = 0.0;

		m_bUseSolderHighestVolumeDiff = false;
		m_bUseInspVolumeDiff = false;
		m_bUseInspVolumeMinLength = false;
		m_bUseInspVolumePie = false;

		m_dHighestHeight = 0.0;
		m_dGapH = 50.0;
		m_nLevelCounts = 1;

		m_fPieHeightMaxDiff = 0.0f;

		for (int a = 0; a < SOLDERVOL_LEVEL_COUNT; a++)
		{
			for (int b = 0; b < eMMD_Total; b++)
			{
				m_fArrVolumeDiff[a][b] = 0;
			}
		}

		for (int a = 0; a < SOLDERVOL_LEVEL_COUNT; a++)
		{
			m_bArrUseVolumeDiffInsp[a] = false;
			
		}

		for (int a = 0; a < SOLDERVOL_LEVEL_COUNT; a++)
		{
			m_bArrUseVolumeMinLengthInsp[a] = false;

		}
	}
}AlgoSolderCone;

typedef struct tagRstAlgoSolderCone
{
	double m_dRstAreaLv1;
	double m_dRstAreaLv2;
	double m_dRstAreaLv3;

	float m_fArrRstVolumeDiff[SOLDERVOL_LEVEL_COUNT];	//VolumeDiff 검사 결과 값
	float m_fArrRstVolumeMinLength[SOLDERVOL_LEVEL_COUNT];	//VolumeMinLength 검사 결과 값
	//float m_fRstVolumeH;	//VolumeMinLength 검사 결과 값

	float m_fRstMaxDiffHeightInPie;        //Pie 영역 높이차이 검사결과
	
	BOOL m_bOKVolumePie;		//VolumePie 검사 판정 값

	BOOL m_bArrOKVolumeDiff[SOLDERVOL_LEVEL_COUNT];		//VolumeDiff 검사 판정 값
	BOOL m_bArrOKVolumeMinLength[SOLDERVOL_LEVEL_COUNT];		//VolumeMinLength 검사 판정 값

	POINTF  m_sPoint;									//VolumeDiff 최상단-gap 중심 좌표
	POINT reLongAxisPoinSt[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 장축의 시작좌표
	POINT reLongAxisPoinEd[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 장축의 끝좌표
	POINT reShortAxisPoinSt[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 단축의 시작좌표
	POINT reShortAxisPoinEd[SOLDERVOL_LEVEL_COUNT];		//Volume 높이 구간에서의 단축의 끝좌표

	RECT m_rcRect_I;		//Insp ROI
	RECT m_rcRect_I_NGArea;		//Insp NG ROI

	tagRstAlgoSolderCone()
	{
		m_dRstAreaLv1 = 0.0;
		m_dRstAreaLv2 = 0.0;
		m_dRstAreaLv3 = 0.0;

		memset(m_fArrRstVolumeDiff, 0, sizeof(float)*		SOLDERVOL_LEVEL_COUNT);
		memset(m_fArrRstVolumeMinLength, 0, sizeof(float)*		SOLDERVOL_LEVEL_COUNT);

		//m_fRstVolumeH = 0.0f;
		m_fRstMaxDiffHeightInPie = 0.0f;

		m_bOKVolumePie = 0;
		memset(m_bArrOKVolumeDiff, 0, sizeof(BOOL) *		SOLDERVOL_LEVEL_COUNT);
		memset(m_bArrOKVolumeMinLength, 0, sizeof(BOOL) *		SOLDERVOL_LEVEL_COUNT);

		m_sPoint.x = 0.0f;
		m_sPoint.y = 0.0f;
		memset(reLongAxisPoinSt, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);
		memset(reLongAxisPoinEd, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);
		memset(reShortAxisPoinSt, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);
		memset(reShortAxisPoinEd, 0, sizeof(POINT)*		SOLDERVOL_LEVEL_COUNT);
		
	}
	void Init()
	{
		m_dRstAreaLv1 = 0.0;
		m_dRstAreaLv2 = 0.0;
		m_dRstAreaLv3 = 0.0;
	}
}RstAlgoSolderCone;