#pragma once
#include "../Color/InspParamDef_Color.h"
#define SAlgoGridEA	40000
typedef enum _grid_IgnoreMode
{
	eNone = 0,
	eColor,
	eGray,
	eAll,
}grid_IgnoreMode;

typedef struct tagAlgoGrid
{
	int nColumn;
	int nRow;

	int m_nIgnoreColor;
	double dStdDev;
	double m_dDetectWidth;
	double m_dDetectLength;

	BOOL m_b3dCheck;
	int m_n3dRange;
	double m_d3dHeightMin;
	double m_d3dHeightMax;
	double m_d3dAvgHeight;

	BOOL m_bIgnoreUse;
	BOOL m_bStdUse;

	AlgoColor sAlgoColor;
	BOOL m_bArrExceptArea[SAlgoGridEA];
	tagAlgoGrid()
	{
		nColumn = 1;
		nRow = 1;

		dStdDev = 0.0;

		m_dDetectWidth = 0.0;
		m_dDetectLength = 0.0;

		m_b3dCheck = false;
		m_n3dRange = 3;
		m_d3dHeightMin = 0;
		m_d3dHeightMax = 30;
		m_d3dAvgHeight = 0.0;

		m_bIgnoreUse = true;
		m_bStdUse = true;

		m_nIgnoreColor = grid_IgnoreMode::eAll;
		memset(m_bArrExceptArea, FALSE, sizeof(BOOL) * SAlgoGridEA);
	}
}AlgoGrid;

typedef struct tagRstAlgoGrid
{
	double dRstDetectArea;

	tagRstAlgoGrid()
	{
		dRstDetectArea = 0;
	}
}RstAlgoGrid;