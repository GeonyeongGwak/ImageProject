#pragma once
#include "../BW/InspParamDef_BW.h"

//Pad Aligne
#define PADALIGN_AREA_CNTS		4
typedef struct tagAlgoPadAlign
{
	struct tagAlgoBlackWhite m_sAlgoBW;
	BOOL m_bFilterIsUse;
	int m_nFilterStepNarrow;
	int m_nTypeSelectBlob;
	BOOL m_bFillHole;
	POINTF m_sSearchPoint; // LT
	float m_fTeachWidth;
	float m_fTeachHeight;

	BOOL m_bUseShift;
	float m_fShiftX;
	float m_fShiftY;

	BOOL m_bUseAngle;
	float m_fAngle;

	BOOL m_bUseWidth;
	float m_fMin_W;
	float m_fMax_W;

	BOOL m_bUseHeight;
	float m_fMin_H;
	float m_fMax_H;

	tagAlgoPadAlign()
	{
		m_bFilterIsUse = TRUE;
		m_nFilterStepNarrow = 0;
		m_nTypeSelectBlob = 0;
		m_bFillHole = FALSE;
		m_sSearchPoint.x = 0.0f;
		m_sSearchPoint.y = 0.0f;
		m_fTeachWidth = 0.0f;
		m_fTeachHeight = 0.0f;

		m_bUseShift = TRUE;
		m_fShiftX = 0.0f;
		m_fShiftY = 0.0f;
		m_bUseAngle = TRUE;
		m_fAngle = 10.0;

		m_bUseWidth = FALSE;
		m_fMin_W = 80;
		m_fMax_W = 120;

		m_bUseHeight = FALSE;
		m_fMin_H = 80;
		m_fMax_H = 120;
	}
}AlgoPadAlign;

typedef struct tagRstAlgoPadAlign
{
	// OK ÆÇÁ¤ //
	BOOL m_bMissing;
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;
	BOOL m_bOKAngle;
	BOOL m_bOKWidth;
	BOOL m_bOKHeight;

	double m_dRstShiftX;
	double m_dRstShiftY;
	double m_dRstAngle;
	double m_dRstWidth;
	double m_dRstHeight;

	RECT m_rcTeachRect;
	POINTF m_sArrDstPoint[PADALIGN_AREA_CNTS]; // LT | RT | LB | RB
	POINTF m_poDrawCenter;
	tagRstAlgoPadAlign()
	{
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bOKWidth = FALSE;
		m_bOKHeight = FALSE;

		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstAngle = 0.;
		m_dRstWidth = 0.;
		m_dRstHeight = 0.;
		memset(m_sArrDstPoint, 0, sizeof(POINTF) * PADALIGN_AREA_CNTS);
	}
	void Init()
	{
		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bOKWidth = FALSE;
		m_bOKHeight = FALSE;

		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dRstAngle = 0.;
		m_dRstWidth = 0.;
		m_dRstHeight = 0.;
	}
}RstAlgoPadAlign;