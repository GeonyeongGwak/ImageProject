#include "stdafx.h"
#include "InspParamDef.h"
#include "AlgoFactory.h"
#include "ExtInspRoot.h"

// void tagInspParamTemp::Destroy()
// {
// 
// 	int nCntAlgo = nAlgorithmCnt;
// 	for (int nAlgo = 0; nAlgo < nCntAlgo; nAlgo++)
// 	{
// 		InspAlgo &inspAlgoCopy = vArrAlgoParam[nAlgo];
// 		void *&ptrInspAlgoCopy = inspAlgoCopy.m_ptrInspAlgoParam;
// 		if (!ptrInspAlgoCopy)
// 			return;
// 
// 		delete ptrInspAlgoCopy;
// 		ptrInspAlgoCopy = NULL;
// 	}
// 
// 	delete[] vArrAlgoParam;
// 	vArrAlgoParam = NULL;
// 
// }
	//if (ext::env::nSystemType == EXT_INSP_SERVER)


void InspAlgo::Clone(InspAlgo &inspAlgo)
{
	inspAlgo.m_nAlgoId = m_nAlgoId;
	inspAlgo.m_eAlgoType = m_eAlgoType;
	inspAlgo.m_bAlgoEnable = m_bAlgoEnable;
	inspAlgo.m_bAlgoGroup = m_bAlgoGroup;
	inspAlgo.m_bIsRequired = m_bIsRequired;
	inspAlgo.m_eLightType = m_eLightType;
	inspAlgo.m_nRedValue = m_nRedValue;
	inspAlgo.m_nGreenValue = m_nGreenValue;
	inspAlgo.m_nBlueValue = m_nBlueValue;
	inspAlgo.m_nWhiteValue = m_nWhiteValue;

	inspAlgo.m_bUsingManualDefectCode = m_bUsingManualDefectCode;
	inspAlgo.m_nManualDefectCode = m_nManualDefectCode;
	inspAlgo.m_nManualSubDefectCode = m_nManualSubDefectCode;
	inspAlgo.m_ptrInspAlgoParam = m_ptrInspAlgoParam;

	if (!m_ptrInspAlgoParam)
		return;

	inspAlgo.m_nLightCnt = m_nLightCnt;
	inspAlgo.m_nArrRedValue = m_nArrRedValue;
	inspAlgo.m_nArrGreenValue = m_nArrGreenValue;
	inspAlgo.m_nArrBlueValue = m_nArrBlueValue;
	inspAlgo.m_nArrWhiteValue = m_nArrWhiteValue;
	inspAlgo.m_nArrCalculation = m_nArrCalculation;
	inspAlgo.m_nArrLightPosition = m_nArrLightPosition;
	inspAlgo.m_bUseAI = m_bUseAI;
	inspAlgo.m_AIModelID = m_AIModelID;
	if (m_nLightCnt != 0)
	{
		if (!m_nArrRedValue)
			return;
		if (!m_nArrGreenValue)
			return;
		if (!m_nArrBlueValue)
			return;
		if (!m_nArrWhiteValue)
			return;
		if (!m_nArrCalculation)
			return;
		if (!m_nArrLightPosition)
			return;
	}

	// for masking ROI
	inspAlgo.m_nUsedMaskingValue = m_nUsedMaskingValue;
	//for (int i = 0; i < MAX_MASKING_NUM; i++)
	//	inspAlgo.m_rcArrMaskingROI[i] = m_rcArrMaskingROI[i];
	inspAlgo.m_rcArrMaskingROI = m_rcArrMaskingROI;

	// for Insp Area Polygon
	inspAlgo.m_nUsedInspPolygon = m_nUsedInspPolygon;
	inspAlgo.m_bConvetExceptROI = m_bConvetExceptROI;
	for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		inspAlgo.m_ptArrInspPolygon[i] = m_ptArrInspPolygon[i];

	inspAlgo.m_nMixCount = m_nMixCount;
	for (int i = 0; i < m_nMixCount; i++)
		InspAlgoLightsMix[i].Clone(inspAlgo.InspAlgoLightsMix[i]);

	CAlgoFactory cloneFactory;
	cloneFactory.CreateCAlgoClone(m_eAlgoType, m_ptrInspAlgoParam, inspAlgo.m_ptrInspAlgoParam);
}