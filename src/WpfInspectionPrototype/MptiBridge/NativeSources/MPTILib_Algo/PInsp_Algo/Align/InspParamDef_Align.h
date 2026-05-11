#pragma once
#include "../InspParamDef_AlgoBase.h"

enum m_eAlign
{
	eAlign_1 = 0x01,
};

typedef struct tagAlgoAlign
{
	BOOL m_bUseIPC;
	byte m_byIPCClass;
	BOOL m_InvertCheck;	// 반전 여부
	// 2D
	BOOL m_bInsp2D;		// 2D 적용 여부
	int m_nMinBinary;
	int m_nMaxBinary;
	int m_nTypeRange2D;
	// 3D
	BOOL m_bInsp3D;		// 3D 적용 여부
	double m_dHeightRateMin;
	double m_dHeightRateMax;
	double m_dHeightAvg;
	int m_nTypeRange3D;

	int m_nSearchNum;				// Search 영역 개수
	POINTF m_sArrSearchPoint[4];	// Search 위치 좌표 (Part 기준 pixel 좌표)
	SIZE m_sArrSearchSize[4];		// Search 영역 사이즈 (pixel 단위)
	int m_nSearchMargin;			// Search 영역 Margin +- 적용 (pixel 단위)	

	BOOL m_bUseShift;		// Shift NG 검사 유무
	double m_dShiftX;
	double m_dShiftY;
	BOOL m_bUseAngle;		// Angle NG 검사 유무
	double m_dAngle;

	int m_nMinBlobArea;
	struct tagAlgoColorBase m_sAlgoColorBase;
	BOOL m_bFillHole;
	byte m_byInspOPT;
	double m_dFiduAngle;
	BOOL m_bSameSize;
	tagAlgoAlign()
	{
		m_InvertCheck = FALSE;
		m_bInsp2D = TRUE;
		m_nMinBinary = 100;
		m_nMaxBinary = 100;

		m_bInsp3D = FALSE;
		m_dHeightRateMin = 80;
		m_dHeightRateMax = 120;
		m_dHeightAvg = 0.0F;

		m_nSearchNum = 2;
		m_nSearchMargin = 5;

		m_bUseShift = false;
		m_dShiftX = 1;
		m_dShiftY = 1;

		m_bUseAngle = FALSE;		// Angle NG 검사 유무
		m_dAngle = 0;
		m_nMinBlobArea = 5;
		m_bFillHole = FALSE;
		m_byInspOPT = 0;
		m_dFiduAngle = 0;
		m_bSameSize = TRUE;
	}
}AlgoAlign;

typedef struct tagRstAlgoAlign	// Align 검사 결과
{
	int m_nOKAreaCnt;

	double m_dOffset_x;
	double m_dOffset_y;
	double m_dTheta;
	// OK 판정 //
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;
	BOOL m_bOKAngle;

	RECT m_rcBodyRect;

	RECT m_rcRect_T[4];
	RECT m_rcRect_I[4];
	POINTF m_poDrawCenter;
	POINTF m_poCenter[4];
	tagRstAlgoAlign()
	{
		m_nOKAreaCnt = 0;

		m_dOffset_x = 0.;
		m_dOffset_y = 0.;
		m_dTheta = 0.;

		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
	}
	void Init()
	{
		m_nOKAreaCnt = 0;

		m_dOffset_x = 0.;
		m_dOffset_y = 0.;
		m_dTheta = 0.;

		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
	}
}RstAlgoAlign;