#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct tagAlgoHeightMean : AlgoBaseBW// : InspAlgo
{
	BOOL m_bHeightUse;
	double dHeightMin3D;			// 최소 높이 um
	double dHeightMax3D;			// 최대 높이 um
	double dAvgHeight3D;			// 티칭 시 높이 um

	BOOL m_bUseHighest;
	double m_dHighestMin;
	double m_dHighestMax;
	double m_dHighestValue;

	BOOL m_bUseLowest;
	double m_dLowestMin;
	double m_dLowestMax;
	double m_dLowestValue;

	BOOL m_bBWOption;

	BOOL m_bUseHeightMin3D;
	BOOL m_bUseHeightMax3D;

	BOOL m_bUseSolderHighest;
	float m_fAddHeight;
	float m_fAddHighest;
	float m_fAddLowest;
	struct tagAlgoBlobBase m_sBlobHighestBase;
	struct tagAlgoBlobBase m_sBlobLowestBase;

	int m_nHighestX;
	int m_nHighestY;
	tagAlgoHeightMean() : AlgoBaseBW()
	{
		m_bHeightUse = TRUE;
		dHeightMin3D = 80;
		dHeightMax3D = 120;
		dAvgHeight3D = 0.0;

		m_bUseHighest = FALSE;
		m_dHighestMin = 80;
		m_dHighestMax = 120;
		m_dHighestValue = 0.0;

		m_bUseLowest = FALSE;
		m_dLowestMin = 80;
		m_dLowestMax = 120;
		m_dLowestValue = 0.0;

		m_bBWOption = FALSE;

		m_bUseHeightMin3D = FALSE;
		m_bUseHeightMax3D = FALSE;

		m_bUseSolderHighest = FALSE;
		m_fAddHeight = 0.0f;
		m_fAddHighest = 0.0f;
		m_fAddLowest = 0.0f;

		m_nHighestX = 0;
		m_nHighestY = 0;
	}
}AlgoHeightMean;

typedef struct tagRstAlgoHeightMean	// Height Mean 검사 결과
{
	double m_dRstHeightMean;
	double m_dRstHeighestValue;
	double m_dRstLowestValue;

	BOOL m_bOKHeightMean;
	BOOL m_bOKHeighest;
	BOOL m_bOKLowest;

	BOOL m_bNoSolder;
	BOOL m_bExcess;

	tagRstAlgoHeightMean()
	{
		m_dRstHeightMean = 0.;
		m_dRstHeighestValue = 0.;
		m_dRstLowestValue = 0.;

		m_bOKHeightMean = FALSE;
		m_bOKHeighest = FALSE;
		m_bOKLowest = FALSE;

		m_bNoSolder = FALSE;
		m_bExcess = FALSE;
	}
	void Init()
	{
		m_dRstHeightMean = 0.;
		m_dRstHeighestValue = 0.;

		m_bOKHeightMean = FALSE;
		m_bOKHeighest = FALSE;

		m_bNoSolder = FALSE;
		m_bExcess = FALSE;
	}
}RstAlgoHeightMean;