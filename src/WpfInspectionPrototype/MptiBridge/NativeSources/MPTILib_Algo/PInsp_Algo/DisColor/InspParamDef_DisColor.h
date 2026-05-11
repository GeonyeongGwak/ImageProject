#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct tagAlgoDisColor
{
	int nDiscolorEnable;

	AlgoColorBase m_sAlgoColorBase;

	int m_nSizeXCIE;
	int m_nSizeYCIE;

	// Teaching
	float m_fArea;
	int m_nDisColorAllowance;
	int m_nLength;
	double m_dDisColorDiameter;
	double m_dShapeLength;

	ColorXYInfoForeign m_ColorXYInfo[COLORALGO_POLYGON_CNTS];

	int m_nSaveDebugData;

	BOOL m_bUseClustering;
	int m_nPixelClusteringCnt;

	tagAlgoDisColor()
	{
		nDiscolorEnable = 0;

		//struct tagAlgoColorBase m_sAlgoColorBase;	

		m_nSizeXCIE = 0;
		m_nSizeYCIE = 0;

		// Teaching
		m_fArea = 0;
		m_nDisColorAllowance = 0;
		m_nLength = 0;
		m_dDisColorDiameter = 0;
		m_dShapeLength = 0;

		//ColorXYInfoForeign m_ColorXYInfo[COLORALGO_POLYGON_CNTS];

		m_nSaveDebugData = 0;

		m_bUseClustering = FALSE;
		m_nPixelClusteringCnt = 0;
	}

}AlgoDisColor;