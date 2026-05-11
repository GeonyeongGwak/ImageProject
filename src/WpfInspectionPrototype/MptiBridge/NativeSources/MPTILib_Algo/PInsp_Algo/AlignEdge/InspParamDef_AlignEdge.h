#pragma once
#include "../Edge/InspParamDef_Edge.h"

enum m_eAE
{
	m_eAE_DistMin = 0x01,
	m_eAE_DistMax = 0x02,
	m_eAE_DistXMin = 0x04,
	m_eAE_DistXMax = 0x08,

	m_eAE_DistYMin = 0x10,
	m_eAE_DistYMax = 0x20,
	m_eAE_MissingOK = 0x40,
	m_eAE_DistXReal = 0x80,

	m_eAE_DistYReal = 0x100,
	m_eAE_1_CG = 0x200,
	m_eAE_1_ROI = 0x400,
	m_eAE_2_CG = 0x800,

	m_eAE_2_ROI = 0x1000,
	m_eAE_3_CG = 0x2000,
	m_eAE_3_ROI = 0x4000,
	m_eAE_Total = 0x3F,
};

typedef struct tagAlgoAlignEdge
{
	struct stAlgoEdge sArrAlgoEdge[ALIGNEDGE_AREA_CNTS];
	POINTF m_sArrSearchPoint[ALIGNEDGE_AREA_CNTS];	// Search 위치 좌표 (Part 기준 pixel 좌표)
	SIZE m_sArrSearchSize[ALIGNEDGE_AREA_CNTS];		// Search 영역 사이즈 (pixel 단위)
	int m_nAreaCnt;
	BOOL m_bCicleOpt;
	int m_nOPT;
	BOOL m_bUseAnchor; // align window와 검출 center의 좌표 거리 반환 옵션
	BOOL m_bUseTwoAnchor;
	wchar_t m_sAlignWindow[MAX_STRLEN];
	double m_dDistOffset;
	double m_dDistXOffset;
	double m_dDistYOffset;

	tagAlgoAlignEdge()
	{
		for (int a = 0; a < ALIGNEDGE_AREA_CNTS; a++)
		{
			sArrAlgoEdge[a].m_bInvertCheck = FALSE;
			sArrAlgoEdge[a].m_dAreaMin = 80.;
			sArrAlgoEdge[a].m_dAreaMax = 120.;
			sArrAlgoEdge[a].m_nTypeSelectBlob = eSelectCenter;
			sArrAlgoEdge[a].m_bFillHole = FALSE;
			sArrAlgoEdge[a].m_nFilterStepNarrow = 4;

			sArrAlgoEdge[a].m_bInsp2D = TRUE;
			sArrAlgoEdge[a].m_nMinBinary = 125;
			sArrAlgoEdge[a].m_nMaxBinary = 255;
			sArrAlgoEdge[a].m_nTypeRange2D = eTypeRangeIn;

			sArrAlgoEdge[a].m_bInsp3D = FALSE;
			sArrAlgoEdge[a].m_dHeightRateMin = 80;
			sArrAlgoEdge[a].m_dHeightRateMax = 120;
			sArrAlgoEdge[a].m_fHeightAvg = 0.0F;
			sArrAlgoEdge[a].m_nTypeRange3D = eTypeRangeIn;

			sArrAlgoEdge[a].m_sAlgoColorBase.m_bUseColor = FALSE;
			sArrAlgoEdge[a].m_sAlgoColorBase.m_nPolygonCnt = 1;
			sArrAlgoEdge[a].m_sAlgoColorBase.m_bInvert = FALSE;
			sArrAlgoEdge[a].m_sAlgoColorBase.m_byColorLightType = 0;
			for (int n = 0; n < COLORALGO_POLYGON_CNTS; n++)
			{
				for (int k = 0; k < POLYGON_POINT_CNTS; k++)
				{
					sArrAlgoEdge[a].m_sAlgoColorBase.m_ptArrPolygon[n][k].x = -1;
					sArrAlgoEdge[a].m_sAlgoColorBase.m_ptArrPolygon[n][k].y = -1;
				}
			}

			sArrAlgoEdge[a].m_dTechCenterX = 0.0;
			sArrAlgoEdge[a].m_dTechCenterY = 0.0;

			sArrAlgoEdge[a].m_bUseIPC = FALSE;

			sArrAlgoEdge[a].m_bTeachWidthUse = FALSE;
			sArrAlgoEdge[a].m_dTeachWidth = 0.0;
			sArrAlgoEdge[a].m_dTeachWidthRateMin = 80.;
			sArrAlgoEdge[a].m_dTeachWidthRateMax = 120.;
			sArrAlgoEdge[a].m_bTeachLengthUse = FALSE;
			sArrAlgoEdge[a].m_dTeachLength = 0.0;
			sArrAlgoEdge[a].m_dTeachLengthRateMin = 80.;
			sArrAlgoEdge[a].m_dTeachLengthRateMax = 120.;

			sArrAlgoEdge[a].m_bUseBlobNG = FALSE;
			sArrAlgoEdge[a].m_dBlobSizeWidth = 0.f;
			sArrAlgoEdge[a].m_dBlobSizeLength = 0.f;

			sArrAlgoEdge[a].m_bShiftXUse = TRUE;
			sArrAlgoEdge[a].m_bShiftYUse = TRUE;

			sArrAlgoEdge[a].m_bUseBlobNG = FALSE;
			sArrAlgoEdge[a].m_dBlobSizeWidth = 0.0;
			sArrAlgoEdge[a].m_dBlobSizeLength = 0.0;

			sArrAlgoEdge[a].m_bUseHeight = FALSE;
			sArrAlgoEdge[a].m_dTeachHeight = 0.;
			sArrAlgoEdge[a].m_dTeachHeightMax = 0.;
			sArrAlgoEdge[a].m_dTeachHeightMin = 0.;

			sArrAlgoEdge[a].m_nSetLineCnt = 0;
			sArrAlgoEdge[a].m_bGroup = TRUE;

			sArrAlgoEdge[a].m_bUseAngle = TRUE;
			sArrAlgoEdge[a].m_dTeachRotate = 10.;

			memset(sArrAlgoEdge[a].m_bArrIsHorizon, FALSE, sizeof(BOOL) * EdgeLineTotalCnt);
			memset(sArrAlgoEdge[a].m_nArrMeasureDirection, 0, sizeof(int) * EdgeLineTotalCnt);
			memset(sArrAlgoEdge[a].m_nArrSetInspCondition, 0, sizeof(int) * EdgeInspCnt);
			memset(sArrAlgoEdge[a].m_dArrTeachLength, 0, sizeof(double) * EdgeLineTotalCnt);
			memset(sArrAlgoEdge[a].m_poArrSetTeachCenter, 0, sizeof(POINTF) * EdgeLineTotalCnt);

			sArrAlgoEdge[a].m_nLineFindType = 0;
			sArrAlgoEdge[a].m_dLineFindRate = 100;
			sArrAlgoEdge[a].m_nInspOption = 0;
		}

		memset(m_sArrSearchPoint, 0, sizeof(POINTF) * ALIGNEDGE_AREA_CNTS);
		memset(m_sArrSearchSize, 0, sizeof(SIZE) * ALIGNEDGE_AREA_CNTS);
		m_nAreaCnt = 2;
		m_bCicleOpt = FALSE;
		m_nOPT = m_eAE_Total;
		m_bUseAnchor = FALSE;
		m_bUseTwoAnchor = FALSE;
		m_dDistOffset = 0.0;
		m_dDistXOffset = 0.0;
		m_dDistYOffset = 0.0;
	}
}AlgoAlignEdge;

typedef struct tagRstAlgoAlignEdge
{
	double m_dCenterX_T;
	double m_dCenterY_T;
	double m_dCenterX_R;
	double m_dCenterY_R;

	double m_dRstShiftX;
	double m_dRstShiftY;
	double m_dTheta;
	double m_dDistance;
	double m_dDistanceX;
	double m_dDistanceY;
	double m_d3PtDis1;
	double m_d3PtDis3;
	double m_dArea[EdgeInspCnt];
	// OK 판정 //
	BOOL m_bOKShiftX;
	BOOL m_bOKShiftY;
	BOOL m_bOKAngle;
	BOOL m_bOKDistance;
	BOOL m_bOKDistanceX;
	BOOL m_bOKDistanceY;
	BOOL m_bOK3PtDis1;
	BOOL m_bOK3PtDis3;

	BOOL m_bMissing_1;
	BOOL m_bMissing_2;
	BOOL m_bMissing_3;

	POINTF m_poDrawLine[EdgeLineTotalCnt];
	POINTF m_poDrawLine_Sec[EdgeLineTotalCnt];
	POINTF m_poDrawLine_1[EdgeLineTotalCnt];
	POINTF m_poDrawLine_Sec_1[EdgeLineTotalCnt];

	POINTF m_poDrawLine_2[EdgeLineTotalCnt];
	POINTF m_poDrawLine_Sec_2[EdgeLineTotalCnt];

	//RECT m_rcRect_I[2];
	RECT m_rcRect_I[3];
	POINTF m_poDrawLine_T[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T2[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T21[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T22[EdgeLineTotalCnt];

	POINTF m_poDrawLine_T31[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T32[EdgeLineTotalCnt];
	POINTF m_poDrawCenter;

	//for teaching display
	RECT m_rcFindCircle[3];
	POINTF m_pHPoint[4];
	POINTF m_poRstCenter[EdgeLineTotalCnt];
	tagRstAlgoAlignEdge()
	{
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dTheta = 0.;
		m_dDistance = 0.;
		m_d3PtDis1 = 0.;
		m_d3PtDis3 = 0.;

		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bOKDistance = FALSE;
		m_bOK3PtDis1 = FALSE;
		m_bOK3PtDis3 = FALSE;

		m_bMissing_1 = FALSE;
		m_bMissing_2 = FALSE;
		m_bMissing_3 = FALSE;
	}
	void Init()
	{
		m_dRstShiftX = 0.;
		m_dRstShiftY = 0.;
		m_dTheta = 0.;
		m_dDistance = 0.;
		m_d3PtDis1 = 0.;
		m_d3PtDis3 = 0.;

		m_bOKShiftX = FALSE;
		m_bOKShiftY = FALSE;
		m_bOKAngle = FALSE;
		m_bOKDistance = FALSE;
		m_bOK3PtDis1 = FALSE;
		m_bOK3PtDis3 = FALSE;

		m_bMissing_1 = FALSE;
		m_bMissing_2 = FALSE;
		m_bMissing_3 = FALSE;
	}
}RstAlgoAlignEdge;