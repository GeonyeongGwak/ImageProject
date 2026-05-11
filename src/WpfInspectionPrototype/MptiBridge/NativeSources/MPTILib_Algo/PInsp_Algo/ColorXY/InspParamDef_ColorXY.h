#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct stAlgoColorXY
{
	// 2D
	float m_fColorXYMinX;	// 최소 %
	float m_fColorXYMaxX;	// 최대 %
	float m_fColorAvgX;	// 기준 값

	float m_fColorXYMinY;	// 최소 %
	float m_fColorXYMaxY;	// 최대 %
	float m_fColorAvgY;	// 기준 값

	float m_fFatorRed;
	float m_fFatorGreen;
	float m_fFatorBlue;

	stAlgoColorXY()
	{
		m_fColorXYMinY = 30.f;	// 최소 
		m_fColorXYMaxY = 30.f;	// 최대 
		m_fColorAvgY = 0.f;	// 기준 값

		m_fColorXYMinY = 30.f;	// 최소 
		m_fColorXYMaxY = 30.f;	// 최대 
		m_fColorAvgY = 0.f;	// 기준 값

		m_fFatorRed = 0.0;
		m_fFatorGreen = 0.0;
		m_fFatorBlue = 0.0;
	}
}AlgoColorXY;

typedef struct tagRstAlgoColorXY	// ColorXY 검사 결과
{
	double m_dRstColorXY_X;
	double m_dRstColorXY_Y;

	BOOL m_bRstStdChanged;

	float m_fRstColorXYMinX;
	float m_fRstColorXYMaxX;

	float m_fRstColorXYMinY;
	float m_fRstColorXYMaxY;

	BOOL m_bColorXYOK_X;
	BOOL m_bColorXYOK_Y;

	BOOL m_bUseUV;
	float m_fStdX_Min;
	float m_fStdX_Max;
	float m_fStdY_Min;
	float m_fStdY_Max;

	tagRstAlgoColorXY()
	{
		m_dRstColorXY_X = 0.f;
		m_dRstColorXY_Y = 0.f;

		m_bRstStdChanged = false;

		m_fRstColorXYMinX = 0.f;
		m_fRstColorXYMaxX = 0.f;

		m_fRstColorXYMinY = 0.f;
		m_fRstColorXYMaxY = 0.f;

		m_bColorXYOK_X = TRUE;
		m_bColorXYOK_Y = TRUE;

		m_bUseUV = TRUE;

		m_fStdX_Min = 0.f;
		m_fStdX_Max = 0.f;
		m_fStdY_Min = 0.f;
		m_fStdY_Max = 0.f;
	}
	void Init()
	{
		m_dRstColorXY_X = 0.f;
		m_dRstColorXY_Y = 0.f;

		m_bRstStdChanged = false;

		m_fRstColorXYMinX = 0.f;
		m_fRstColorXYMaxX = 0.f;

		m_fRstColorXYMinY = 0.f;
		m_fRstColorXYMaxY = 0.f;

		m_bColorXYOK_X = TRUE;
		m_bColorXYOK_Y = TRUE;

		m_bUseUV = TRUE;

		m_fStdX_Min = 0.f;
		m_fStdX_Max = 0.f;
		m_fStdY_Min = 0.f;
		m_fStdY_Max = 0.f;
	}
}RstAlgoColorXY;