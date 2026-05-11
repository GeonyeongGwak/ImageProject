#pragma once
#include "../InspParamDef_AlgoBase.h"

typedef struct tagAlgoLeadSearch
{
	//[input]
	int nDisplayPannelSizeX;
	int nDisplayPannelSizeY;

	int nLeadPosition;  //0:left, 1:right, 2:top, 3:bottom
	int nSolderStartPos;
	BOOL bIsContainer;

	BOOL bUse3D; //lead 추출할때 3D 사용할지 여부
	int nThresholdValue;  //2d

	BOOL bInspHeightFlag;

	BOOL bInspCoplanrity;

	//solder
	//[output]
	int nSolderCnt;

	BOOL bUse2D; //lead 추출할때 2D 사용할지 여부

	// Manual
	double m_dLeadWidth;
	double m_dLeadPitch;
	POINT m_poStartLead;
	POINT m_poEndLead;
	BOOL m_bIsManualSearch;
	BOOL m_bIsManualType;
	int m_nLeadStartLeft;
	double m_dLeadStartLeft_AlignCenterDstX;

	BOOL m_bSameWidthPitch;

	BOOL m_bUseGerber;
	int m_nPadCnt;
	RECT m_rcArrPadRect[BRIEDGE_CNT];

	BOOL m_bWideSize;
	tagAlgoLeadSearch()
	{
		nDisplayPannelSizeX = 0;
		nDisplayPannelSizeY = 0;

		nLeadPosition = 0;
		nSolderStartPos = 0;
		bIsContainer = TRUE;

		bUse3D = TRUE;
		nThresholdValue = 20;

		bInspHeightFlag = TRUE;

		bInspCoplanrity = FALSE;

		nSolderCnt = 0;

		bUse2D = TRUE;

		m_dLeadWidth = 0.0;
		m_dLeadPitch = 0.0;
		m_poStartLead.x = 0;
		m_poStartLead.y = 0;
		m_poEndLead.x = 0;
		m_poEndLead.y = 0;

		m_bIsManualSearch = FALSE;
		m_bIsManualType = FALSE;
		m_nLeadStartLeft = 0;
		m_dLeadStartLeft_AlignCenterDstX = 0.0;

		m_bSameWidthPitch = FALSE;

		m_bUseGerber = FALSE;
		m_nPadCnt = 0;
		memset(m_rcArrPadRect, 0, sizeof(RECT) * BRIEDGE_CNT);

		m_bWideSize = FALSE;
	}
}AlgoLeadSearch;

typedef struct tagRstAlgoLeadSearch		// Lead Search 검사 결과
{

}RstAlgoLeadSearch;