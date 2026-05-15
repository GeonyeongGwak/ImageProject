#include "stdafx.h"

#include "AlgoFactory.h"
#include "InspManager.h"
#include "MemoryManager.h"

#include "MPTI.h"
#include "QTimer.h"
#include <fstream>
#include <iostream>
#include "ExtLog.h"
#include "PInsp_Pattern.h"
#include "PInsp_OCV.h"
//#include "PInsp_Barcode.h"
#include "PInspDecision.h"
//#ifdef _DEBUG
//
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

InspManager* g_pInspMng = NULL;
//lightData m_sLightData[3];

InspManager::InspManager()
{
	m_PInspWrapper = new PInspAlgoWrapper;

	m_NumWireAlgo = 0;
	m_puImgWireInterval = nullptr;
	m_puImgFootResult = nullptr;
	m_pInspDecision = NULL;
	m_inspItemCnts = NULL;
	m_groupIndexCnts = NULL;
	m_inspItemID = NULL;
	m_groupIndex = NULL;
	m_groupID = NULL;
	m_groupWndCnts = NULL;
	m_groupWndID = NULL;
	m_inspWndOrder = NULL;
	m_inspectionResult = nullptr;
	CopyPVImage = NULL;

	g_pInspMng = this;
	m_pPOCR = NULL;
	m_pWireBondingInsp = NULL;
	m_PInspWrapper = NULL;
	m_procMil = NULL;
	m_pMountInsp = NULL;
	m_pPatternInsp = NULL;
	m_pOcrInsp = NULL;
	m_pColorInsp = NULL;
	m_pLeadSolderInsp = NULL;
	//m_pBGAInsp = NULL;
	m_pForeignPattern = NULL;
	m_pGridInsp = NULL;

	m_pMountTeach = NULL;
	m_pPatternTeach = NULL;
	m_pOcrTeach = NULL;
	m_pColorTeach = NULL;
	m_pLeadSolderTeach = NULL;
	//m_pBGATeach = NULL;
	m_pTab = NULL;

	m_pInspBoardInfo_Foreign = NULL;
	m_pInspBoardInfo = NULL;
	m_pParamArray = NULL;

	//m_pRstBGA_Wnd = NULL;
	//m_pRstBGA_Grp = NULL;

	m_inspectionResult = NULL;
	m_inspForeignResult = NULL;
	m_ForeignParamROI = NULL;
	AllocBuff();
	m_NgManager = NULL;
	//m_NgManager = new CMNgManager();
	m_CamInterpolation = nullptr;
	//Initialize  TabAreaImg
	m_pucTabAreaTmp = nullptr;
	m_nTabAlgoIdx = -1;

	memset(m_bInspSkipGroupAlgo, 0, sizeof(BOOL) * eINSP_Total);
	memset(m_bInspSkipGroupWnd, 0, sizeof(BOOL) * eINSP_Total);
	memset(m_nInspSkipAlgo, 0, sizeof(unsigned long long) * eINSP_Total);
	//g_pInspMng = this;
	Algorithms();
}


InspManager::~InspManager()
{
	//delete m_PInspWrapper;
	DeleteBuff();

	DeleteResultVal();


	DeleteInspItem();
	DeleteTeachItem();

	DeleteProcMil();

	if (m_pFovImgBufBig)
	{
		//delete m_pFovImgBufBig;
		g_pMManager->pem_delete(m_pFovImgBufBig, false);
		m_pFovImgBufBig = NULL;
	}
	if (m_inspForeignResult)
	{
		if (m_inspForeignResult->m_stForeign)
			g_pMManager->pem_delete(m_inspForeignResult->m_stForeign, true);
		m_inspForeignResult->m_stForeign = NULL;
		if (m_inspForeignResult->m_stForeignWP)
			g_pMManager->pem_delete(m_inspForeignResult->m_stForeignWP, true);
		m_inspForeignResult->m_stForeignWP = NULL;
		g_pMManager->pem_delete(m_inspForeignResult, false);
		m_inspForeignResult = NULL;
	}

	//if (m_ForeignParamROI != NULL)
	//	g_pMManager->pem_delete(m_ForeignParamROI, true);
	//m_ForeignParamROI == NULL;

	//Delete_AI_Image();



	//if (m_CamInterpolation)	// PYJ 2019/06/26
	//{
	//	//delete m_CamInterpolation;
	//	g_pMManager->pem_delete(m_CamInterpolation, false);
	//	m_CamInterpolation = nullptr;
	//}

	Delete_Algorithms();
	if (CopyPVImage != NULL)
	{
		//delete [] CopyPVImage;
		delete[] CopyPVImage;//g_pMManager->pem_delete(CopyPVImage, true);
		CopyPVImage = NULL;
	}
	g_pInspMng = NULL;
	if (m_NgManager != NULL)
	{
		delete m_NgManager;
		m_NgManager = NULL;
	}
	
	if (m_puImgWireInterval)
	{
		delete m_puImgWireInterval;
		m_puImgWireInterval = NULL;

	}
		
	if (m_puImgFootResult)
	{
		delete m_puImgFootResult;
		m_puImgFootResult = NULL;
	}
		
}

void InspManager::AllocBuff()
{

	m_pInspDecision = new CPInspDecision();//g_pMManager->pem_new<CPInspDecision>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

	m_inspItemCnts = new int[eINSP_WND_CNT];//[eINSP_WND_CNT];
	memset(m_inspItemCnts, 0, sizeof(int) * eINSP_WND_CNT);

	m_groupIndexCnts = new int[eINSP_WND_CNT];//[eINSP_WND_CNT]; //검사 항목별 그룹 종류 개수.
	memset(m_groupIndexCnts, 0, sizeof(int) * eINSP_WND_CNT);

	m_inspItemID = new int*[eINSP_WND_CNT];//[eINSP_WND_CNT][MAX_WINDOW_CNTS];
	m_groupIndex = new int*[eINSP_WND_CNT];//[eINSP_WND_CNT][MAX_GROUP_CNTS]; //검사 항목별 가지고 있는 그룹 인덱스
	
	for (int i = 0; i < eINSP_WND_CNT; i++)
	{
		m_inspItemID[i] = new int[MAX_WINDOW_CNTS];
		memset(m_inspItemID[i], 0, sizeof(int) * MAX_WINDOW_CNTS);

		m_groupIndex[i] = new int[MAX_GROUP_CNTS];
		memset(m_groupIndex[i], 0, sizeof(int) * MAX_GROUP_CNTS);
	}


	m_groupID = new int[MAX_GROUP_CNTS];//[MAX_GROUP_CNTS];
	memset(m_groupID, 0, sizeof(int) * MAX_GROUP_CNTS);

	m_groupWndCnts = new int[MAX_GROUP_CNTS];//[MAX_GROUP_CNTS]; // 그룹에 속해있는 윈도우 개수.
	memset(m_groupWndCnts, 0, sizeof(int) * MAX_GROUP_CNTS);

	m_groupWndID = new int*[MAX_GROUP_CNTS];//[MAX_GROUP_CNTS][MAX_WINDOW_CNTS];  //그룹 인덱스 별 윈도우 번호
	for (int i = 0; i < MAX_GROUP_CNTS; i++)
	{
		m_groupWndID[i] = new int[MAX_WINDOW_CNTS];
		memset(m_groupWndID[i], 0, sizeof(int) * MAX_WINDOW_CNTS);
	}

	m_inspWndOrder = new int*[eINSP_WND_CNT]; //[eINSP_WND_CNT][MAX_WINDOW_CNTS];
	for (int i = 0; i < eINSP_WND_CNT; i++)
	{
		m_inspWndOrder[i] = new int[MAX_WINDOW_CNTS];
		memset(m_inspWndOrder[i], 0, sizeof(int) * MAX_WINDOW_CNTS);
	}

}
void InspManager::DeleteBuff()
{
 	if (m_pInspDecision != NULL)
 		delete (CPInspDecision *)m_pInspDecision;
	if (m_inspItemCnts != NULL)
	{
		delete[] m_inspItemCnts;
		m_inspItemCnts = NULL;
	}

	if (m_groupIndexCnts != NULL)
	{
		delete[] m_groupIndexCnts;
		m_groupIndexCnts = NULL;
	}


	if (m_inspItemID != NULL)
	{
		for (int i = 0; i < eINSP_WND_CNT; i++)
		{
			delete[] m_inspItemID[i];
			m_inspItemID[i] = NULL;
		}

		delete[] m_inspItemID;
		m_inspItemID = NULL;
	}

	if (m_groupID != NULL)
	{
		delete[] m_groupID;
		m_groupID = NULL;
	}


	if (m_groupWndCnts != NULL)
	{
		delete[] m_groupWndCnts;
		m_groupWndCnts = NULL;
	}


	if (m_groupIndex != NULL)
	{
		for (int i = 0; i < eINSP_WND_CNT; i++)
		{
			delete[] m_groupIndex[i];
			m_groupIndex[i] = NULL;
		}

		delete[] m_groupIndex;
	}

	if (m_groupWndID != NULL)
	{
		for (int i = 0; i < MAX_GROUP_CNTS; i++)
		{
			delete[] m_groupWndID[i];
			m_groupWndID[i] = NULL;
		}

		delete[] m_groupWndID;
	}


	if (m_inspWndOrder != NULL)
	{
		for (int i = 0; i < eINSP_WND_CNT; i++)
		{
			delete[] m_inspWndOrder[i];
			m_inspWndOrder[i] = NULL;
		}

		delete[] m_inspWndOrder;
	}
}

void InspManager::DeleteProcMil()
{
	if (m_procMil != NULL)
	{
		m_procMil->FreeMil();

		delete m_procMil;
		//g_pMManager->pem_delete(m_procMil, false);
		m_procMil = NULL;
	}
}

void InspManager::SetResolution(int fovWidth, int fovLength, double resolX, double resolY)
{
	m_bSetResolution = true;
	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;
	if(m_NgManager == NULL)
		m_NgManager = new CMNgManager();
	m_NgManager->Init();
	m_NgManager->SetResolution(fovWidth, fovLength, resolX, resolY);
	m_PInspWrapper->SetResolution( fovWidth,  fovLength,  resolX,  resolY);
	auto wrapper = [](CString sLog) -> void
	{
		return ext::Log::add(sLog);

	};
	m_PInspWrapper->m_PInspAlgo->LogFunc_Add(wrapper);
	//m_pPatternInsp->InitDevice(m_milApp, m_milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	m_milApp = g_pMPTI->m_milApp;
	m_milSys = g_pMPTI->m_milSys;

	m_procMil = g_pMManager->pem_new<CProcMil>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_procMil->InitMil(m_milApp, m_milSys, g_pMPTI->isUseImagePilLib());
	m_procMil->SetResol(resolX, resolY, fovWidth);
	m_procMil->InitMilAlgoBlob();


	m_InspAlgo.InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	//// 20140921 SHW
	//if (m_pOcrTeach)
	//	m_pOcrTeach->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	//if (m_pOcrInsp)
	//	m_pOcrInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());

	if (m_pPatternTeach)
		m_pPatternTeach->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	if (m_pPatternInsp)
		m_pPatternInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());

	if (m_pColorTeach)
		m_pColorTeach->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	if (m_pColorInsp)
		m_pColorInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());

	//m_pPOCR->InitMil(m_milApp, m_milSys, g_pMPTI->isUseImagePilLib());
	m_pPOCR->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());

	//m_pForeignPattern->InitMil(m_milApp,m_milSys, g_pMPTI->isUseImagePilLib());
	m_pForeignPattern->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	// YJS 2016/11/04
	if (m_pGridInsp)
		m_pGridInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());



	////YBJI 2019/10/01
	m_pWireBondingInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());

	////m_inspForeign.InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());

	m_pTab->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());

	g_pMPTI->SetExtInspInit(fovWidth, fovLength, resolX, resolY);
	//m_inspForeign.InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	m_PAD_PAT_VOL.InitAlgo(m_resolX, m_resolY);
	m_FR.InitAlgo(m_fovWidth, m_fovLength, m_resolX, m_resolY);
	for (int i = 0; i < InspAlgoType::eAlgoNum; i++)
	{
		if (m_dicInspAlgorithm[(InspAlgoType)i] != nullptr)
			m_dicInspAlgorithm[(InspAlgoType)i]->InitAlgo();
	}

	//DeleteProcMil();

}
void InspManager::SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB)
{
	m_nCompositeLightMode = LightMode;
	m_PInspWrapper->SetCompositeLightMode(LightMode, compoBtmR, compoBtmG, compoBtmB);
}
void InspManager::SetFootOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax)
{
	//m_PInspWrapper->m_PInspWire->SetFootOption(nFootPadTopWGrayLevel, nFootPatternMatchScore, bMatch2D, bMatch3D, nSobelmin, nSobelmax);
}

void InspManager::SetInspParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize)
{
	m_pInspBoardInfo = boardInfo;
	m_pParamArray = pParamArray;
	m_ParamArraySize = nParamArraySize;

// 	for (int i = 0; i < MANAGER_CNT; i++)
// 	{
// 		if (m_pManager[i] != NULL)
// 		{

	//EXT Define 필요 Algotool
	if (m_NgManager) m_NgManager->SetPartParam(boardInfo, pParamArray, nParamArraySize);
			SetInspItemInfo(m_inspItemCnts, m_inspItemID, m_inspWndOrder);
	if (m_NgManager) m_NgManager->SetInspItemInfo(m_inspItemCnts, m_inspItemID, m_inspWndOrder);
			SetInspGroupInfo(m_groupIndexCnts, m_groupIndex, m_groupID, m_groupWndCnts, m_groupWndID);
	if (m_NgManager) m_NgManager->SetInspGroupInfo(m_groupIndexCnts, m_groupIndex, m_groupID, m_groupWndCnts, m_groupWndID);
			SetPartParam(m_pInspBoardInfo, m_pParamArray, m_ParamArraySize);
// 		}
// 	}
}
void InspManager::SetPartParam(InspPartInfo* boardInfo, InspPartParam *pWindowArray, int nWindowCnt)
{
	m_pInspBoardInfo = boardInfo;
	m_ParamArraySize = nWindowCnt;
	m_pParamArray = pWindowArray;

	((CPInspDecision * )m_pInspDecision)->SetParamInfo(m_pParamArray);
	// ExtPrmPtr(m_pInspBoardInfo->partImgBuf) 에 Part ROI 만큼 Clip해서 넣어주자.
	//SetPartROIImage();

}
void InspManager::SetInspItemInfo(int* inspItemCnts, int** inspItemID, int** inspWndOrder)
{
	m_inspItemCnts = inspItemCnts;
	m_inspItemID = inspItemID;
	m_inspWndOrder = inspWndOrder;
}

void InspManager::SetInspGroupInfo(int* groupIndexCnts, int** groupIndex, int* groupID, int* groupWndCnts, int** groupWndID)
{
	m_groupIndexCnts = groupIndexCnts;
	m_groupIndex = groupIndex;

	m_groupID = groupID;
	m_groupWndCnts = groupWndCnts;
	m_groupWndID = groupWndID;

	((CPInspDecision *)m_pInspDecision)->SetParamInfo(m_groupIndexCnts, m_groupWndCnts, m_groupIndex, m_groupWndID);
}
void InspManager::SortingParamater(InspPartParam *pParamArray, int m_nParamArraySize)
{
	if (m_inspItemCnts == NULL || m_inspItemID == NULL || m_groupIndexCnts == NULL || m_groupIndex == NULL || m_groupID == NULL || m_groupWndCnts == NULL || m_groupWndID == NULL)
		return;

	memset(m_inspItemCnts, 0, eINSP_WND_CNT * sizeof(int));
	memset(m_groupIndexCnts, 0, eINSP_WND_CNT * sizeof(int));

	for (int i = 0; i < eINSP_WND_CNT; i++)
	{
		memset(m_inspItemID[i], -1, MAX_WINDOW_CNTS * sizeof(int));
		memset(m_groupIndex[i], -1, MAX_GROUP_CNTS * sizeof(int));
	}

	memset(m_groupWndCnts, 0, MAX_GROUP_CNTS * sizeof(int));
	memset(m_groupWndCnts, 0, MAX_GROUP_CNTS * sizeof(int));

	//////////////////////////////////////////////////////////////////////////
	int groupCnt = 0;
	for (int i = 0; i < m_nParamArraySize; i++)
	{
		if (m_pParamArray[i].groupIndex < 1)		// Skip 예외처리
			continue;

		m_inspItemCnts[m_pParamArray[i].inspType]++;
		m_inspItemID[m_pParamArray[i].inspType][m_inspItemCnts[m_pParamArray[i].inspType] - 1] = i;

		m_groupWndCnts[m_pParamArray[i].groupIndex]++;
		m_groupWndID[m_pParamArray[i].groupIndex][m_groupWndCnts[m_pParamArray[i].groupIndex] - 1] = i;

		if (m_groupWndCnts[m_pParamArray[i].groupIndex] < 2)
		{
			m_groupID[groupCnt] = m_pParamArray[i].groupIndex;
			groupCnt++;
		}
	}

	for (int i = 0; i < groupCnt; i++)
	{
		m_groupIndexCnts[m_pParamArray[m_groupWndID[m_groupID[i]][0]].inspType]++;
		m_groupIndex[m_pParamArray[m_groupWndID[m_groupID[i]][0]].inspType][m_groupIndexCnts[m_pParamArray[m_groupWndID[m_groupID[i]][0]].inspType] - 1] = m_groupID[i];
	}

	//////////////////////////////////////////////////////////////////////////
	//huj 2014/03/04
	int inspWndOrdeTemp[eINSP_WND_CNT][MAX_WINDOW_CNTS];
	for (int i = 0; i < eINSP_WND_CNT; i++)
	{
		memset(m_inspWndOrder[i], -1, MAX_WINDOW_CNTS * sizeof(int));
		memset(inspWndOrdeTemp[i], -1, MAX_GROUP_CNTS * sizeof(int));
	}

	int wndGroupCnt = 0;
	int groupIndex = 0;
	int indexTemp = 0;
	for (int wndType = 0; wndType < eINSP_WND_CNT; wndType++)
	{
		indexTemp = 0;
		wndGroupCnt = m_groupIndexCnts[wndType];
		for (int i = 0; i < wndGroupCnt; i++)
		{
			groupIndex = m_groupIndex[wndType][i];
			int wndCnt = m_groupWndCnts[groupIndex];

			for (int j = 0; j < wndCnt; j++)
			{
				inspWndOrdeTemp[wndType][indexTemp] = m_groupWndID[groupIndex][j];
				m_inspWndOrder[wndType][indexTemp] = indexTemp;
				indexTemp++;
			}
		}

		//sorting
		int temp = 0;
		int temp2 = 0;
		for (int i = 0; i < indexTemp; i++)
		{
			for (int j = i + 1; j < indexTemp; j++)
			{
				if (inspWndOrdeTemp[wndType][i] > inspWndOrdeTemp[wndType][j])
				{
					temp = inspWndOrdeTemp[wndType][i];
					inspWndOrdeTemp[wndType][i] = inspWndOrdeTemp[wndType][j];
					inspWndOrdeTemp[wndType][j] = temp;

					temp2 = m_inspWndOrder[wndType][i];
					m_inspWndOrder[wndType][i] = m_inspWndOrder[wndType][j];
					m_inspWndOrder[wndType][j] = temp2;
				}
			}
		}
	}


}

void InspManager::InitResultVal()
{
	
	DeleteResultVal();

	m_inspectionResult = new InspectionResult();
	memset(m_inspectionResult, 0, sizeof(InspectionResult));

	SetInspParamTemp();

	int wndType = eINSP_MOUNT;
	m_inspectionResult->mountArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->mountResult, wndType, m_inspectionResult->mountParamTemp);
	}

	wndType = eINSP_ALIGN;
	m_inspectionResult->alignArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->alignResult, wndType, m_inspectionResult->alignParamTemp);
	}

	wndType = eINSP_OCR;
	m_inspectionResult->ocrArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->ocrResult, wndType, m_inspectionResult->ocrParamTemp);
	}

	wndType = eINSP_LEADSOLDER;
	m_inspectionResult->leadSolderArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->leadSolderResult, wndType, m_inspectionResult->leadSolderParamTemp);
	}

	wndType = eINSP_SOLDER;
	m_inspectionResult->solderArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->solderResult, wndType, m_inspectionResult->solderParamTemp);
	}

	wndType = eINSP_TAB;
	m_inspectionResult->tabArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->tabResult, wndType, m_inspectionResult->tabParamTemp);
	}

	wndType = eINSP_S_BALL;
	m_inspectionResult->S_BallArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->S_BallResult, wndType, m_inspectionResult->S_BallParamTemp);
	}

	wndType = eINSP_PAD;
	m_inspectionResult->PadArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->PadResult, wndType, m_inspectionResult->PadParamTemp);
	}

	wndType = eINSP_BGA;
	m_inspectionResult->BGAArraySize = m_inspItemCnts[wndType];
	if (m_inspItemCnts[wndType] > 0)
	{
		CreateInspWndResult(&m_inspectionResult->BGAResult, wndType, m_inspectionResult->BGAParamTemp);
	}
}

void InspManager::CreateInspWndResult(InspWndResult ** pArrWndResult, int wndType, InspParamTemp * wndParam)
{
	*pArrWndResult = new InspWndResult[m_inspItemCnts[wndType]];
	memset(*pArrWndResult, 0, (sizeof(InspWndResult) * m_inspItemCnts[wndType]));

	for (int n = 0; n < m_inspItemCnts[wndType]; n++)
	{
		InspWndResult *pInspWndResult = &(*pArrWndResult)[n];
		if (pInspWndResult && (wndParam[n].nAlgorithmCnt > 0))
		{
			int nCntAlgorithm = wndParam[n].nAlgorithmCnt;
			(pInspWndResult)->m_nAlgorithmCnt = nCntAlgorithm;	// 할 것, 알고리즘별 결과 m_vArrRstInspAlgo 받으면 주석 풀 것
			(pInspWndResult)->m_vArrRstInspAlgo = new InspAlgoResult[nCntAlgorithm];
			memset((pInspWndResult)->m_vArrRstInspAlgo, 0, (sizeof(InspAlgoResult) * nCntAlgorithm));
		}
	}
}

void InspManager::SetInspParamTemp()
{
	int indexMount = 0;
	int indexAlign = 0;
	int indexOCR = 0;
	int indexLeadSolder = 0;
	int indexSolder = 0;
	int indexTab = 0;
	int indexS_Ball = 0;
	int indexPad = 0;
	int indexBGA = 0;
	CString wndName = _T("");

	InspParamTemp* wndParamTemp = NULL;

	if (m_inspItemCnts[eINSP_MOUNT] > 0)
	{
		m_inspectionResult->mountParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_MOUNT]];
		memset(m_inspectionResult->mountParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_MOUNT]));
	}
	if (m_inspItemCnts[eINSP_ALIGN] > 0)
	{
		m_inspectionResult->alignParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_ALIGN]];
		memset(m_inspectionResult->alignParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_ALIGN]));
	}
	if (m_inspItemCnts[eINSP_OCR] > 0)
	{
		m_inspectionResult->ocrParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_OCR]];
		memset(m_inspectionResult->ocrParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_OCR]));
	}
	if (m_inspItemCnts[eINSP_LEADSOLDER] > 0)
	{
		m_inspectionResult->leadSolderParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_LEADSOLDER]];
		memset(m_inspectionResult->leadSolderParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_LEADSOLDER]));
	}
	if (m_inspItemCnts[eINSP_SOLDER] > 0)
	{
		m_inspectionResult->solderParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_SOLDER]];
		memset(m_inspectionResult->solderParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_SOLDER]));
	}
	if (m_inspItemCnts[eINSP_TAB] > 0)
	{
		m_inspectionResult->tabParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_TAB]];
		memset(m_inspectionResult->tabParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_TAB]));
	}
	if (m_inspItemCnts[eINSP_S_BALL] > 0)
	{
		m_inspectionResult->S_BallParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_S_BALL]];
		memset(m_inspectionResult->S_BallParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_S_BALL]));
	}
	if (m_inspItemCnts[eINSP_PAD] > 0)
	{
		m_inspectionResult->PadParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_PAD]];
		memset(m_inspectionResult->PadParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_PAD]));
	}
	if (m_inspItemCnts[eINSP_BGA] > 0)
	{
		m_inspectionResult->BGAParamTemp = new InspParamTemp[m_inspItemCnts[eINSP_BGA]];
		memset(m_inspectionResult->BGAParamTemp, 0, (sizeof(InspParamTemp) * m_inspItemCnts[eINSP_BGA]));
		
	}

	int wndStartIndex, groupCnt, wndId, groupIndex;
	for (int wndType = 0; wndType < eINSP_WND_CNT; wndType++)
	{
		wndStartIndex = 0;
		groupCnt = m_groupIndexCnts[wndType];
		for (int i = 0; i < groupCnt; i++)
		{
			groupIndex = m_groupIndex[wndType][i];

			wndId = 0;
			for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
			{
				wndId = m_groupWndID[groupIndex][j];

				wndName.Format(_T("window%d"), m_pParamArray[wndId].wndIndex);

				if (m_pParamArray[wndId].groupIndex > 0)		// Skip 예외처리
				{
					switch (m_pParamArray[wndId].inspType)
					{
					case eINSP_MOUNT:
					{
						wndParamTemp = &m_inspectionResult->mountParamTemp[indexMount];
						indexMount++;
					}
					break;
					case eINSP_ALIGN:
					{
						wndParamTemp = &m_inspectionResult->alignParamTemp[indexAlign];
						indexAlign++;
					}
					break;
					case eINSP_OCR:
					{
						wndParamTemp = &m_inspectionResult->ocrParamTemp[indexOCR];
						indexOCR++;
					}
					break;
					case eINSP_LEADSOLDER:
					{
						wndParamTemp = &m_inspectionResult->leadSolderParamTemp[indexLeadSolder];
						indexLeadSolder++;
					}
					break;
					case eINSP_SOLDER:
					{
						wndParamTemp = &m_inspectionResult->solderParamTemp[indexSolder];
						indexSolder++;
					}
					break;
					case eINSP_TAB:
					{
						wndParamTemp = &m_inspectionResult->tabParamTemp[indexTab];
						indexTab++;
					}
					break;
					case eINSP_S_BALL:
					{
						wndParamTemp = &m_inspectionResult->S_BallParamTemp[indexS_Ball];
						indexS_Ball++;
					}
					break;
					case eINSP_PAD:
					{
						wndParamTemp = &m_inspectionResult->PadParamTemp[indexPad];
						indexPad++;
					}
					break;
					case eINSP_BGA:
					{
						wndParamTemp = &m_inspectionResult->BGAParamTemp[indexBGA];
						indexBGA++;
					}
					break;
					default:
					{
						wndParamTemp = NULL;
						break;
					}
					}
				}
				else
					wndParamTemp = NULL;

				if (wndParamTemp)
				{
					wndParamTemp->inspType = m_pParamArray[wndId].inspType;
					wndParamTemp->cx = m_pParamArray[wndId].cx;
					wndParamTemp->cy = m_pParamArray[wndId].cy;
					wndParamTemp->width = m_pParamArray[wndId].width;
					wndParamTemp->length = m_pParamArray[wndId].length;
					wndParamTemp->bdrCx = m_pInspBoardInfo->partCx;
					wndParamTemp->bdrCy = m_pInspBoardInfo->partCy;
					wndParamTemp->bdrWidth = m_pInspBoardInfo->partWidth;
					wndParamTemp->bdrLength = m_pInspBoardInfo->partHeight;
					wndParamTemp->partAngle = m_pInspBoardInfo->angle;
					wndParamTemp->groupIndex = m_pParamArray[wndId].groupIndex;
					wndParamTemp->nAlgorithmCnt = m_pParamArray[wndId].nAlgorithmCnt;
					wndParamTemp->inspParam = NULL;

					int nCntAlgo = wndParamTemp->nAlgorithmCnt;
					if (nCntAlgo > 0)
					{
						wndParamTemp->vArrAlgoParam = new InspAlgo[nCntAlgo];		// 해제 확인
						for (int nAlgo = 0; nAlgo < nCntAlgo; nAlgo++)
						{
							m_pParamArray[wndId].vArrAlgoParam[nAlgo].Clone(wndParamTemp->vArrAlgoParam[nAlgo]);
						}
					}

					wsprintf(wndParamTemp->wndName, wndName);
				}
			}
		}
	}
}

void InspManager::DeleteResultVal()
{
	DeleteResultVal(m_inspectionResult);
}

void InspManager::DeleteResultVal(InspectionResult*& pInspectionResult)
{
	if (!pInspectionResult)
		return;

	int n = 0;
	if (pInspectionResult->mountParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->mountArraySize; n++)
			pInspectionResult->mountParamTemp[n].Destroy();
		delete pInspectionResult->mountParamTemp;

		pInspectionResult->mountParamTemp = NULL;
	}

	if (pInspectionResult->alignParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->alignArraySize; n++)
			pInspectionResult->alignParamTemp[n].Destroy();
		delete pInspectionResult->alignParamTemp;
		
		pInspectionResult->alignParamTemp = NULL;
	}

	if (pInspectionResult->ocrParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->ocrArraySize; n++)
			pInspectionResult->ocrParamTemp[n].Destroy();
		delete[] pInspectionResult->ocrParamTemp;
		pInspectionResult->ocrParamTemp = NULL;
	}

	if (pInspectionResult->leadSolderParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->leadSolderArraySize; n++)
			pInspectionResult->leadSolderParamTemp[n].Destroy();
		delete pInspectionResult->leadSolderParamTemp;
		pInspectionResult->leadSolderParamTemp = NULL;
	}

	if (pInspectionResult->solderParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->solderArraySize; n++)
			pInspectionResult->solderParamTemp[n].Destroy();
		delete pInspectionResult->solderParamTemp;

		
		pInspectionResult->solderParamTemp = NULL;
	}

	if (pInspectionResult->tabParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->tabArraySize; n++)
			pInspectionResult->tabParamTemp[n].Destroy();
		delete pInspectionResult->tabParamTemp;
		
		pInspectionResult->tabParamTemp = NULL;
	}

	if (pInspectionResult->S_BallParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->S_BallArraySize; n++)
			pInspectionResult->S_BallParamTemp[n].Destroy();
		delete pInspectionResult->S_BallParamTemp;
		
		pInspectionResult->S_BallParamTemp = NULL;
	}

	if (pInspectionResult->PadParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->PadArraySize; n++)
			pInspectionResult->PadParamTemp[n].Destroy();
		delete pInspectionResult->PadParamTemp;
		
		pInspectionResult->PadParamTemp = NULL;
	}

	if (pInspectionResult->BGAParamTemp != NULL)
	{
		for (n = 0; n < pInspectionResult->BGAArraySize; n++)
			pInspectionResult->BGAParamTemp[n].Destroy();
		delete[] pInspectionResult->BGAParamTemp;
		pInspectionResult->BGAParamTemp = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	if (pInspectionResult != NULL)
	{
		pInspectionResult->Destroy();

		delete pInspectionResult;
		pInspectionResult = NULL;
	}

// 	if (!m_pParamArray)
// 		return;
// 	m_pParamArray->Destroy();
// 	delete [] m_pParamArray;

}

void InspManager::Algorithms()
{
	m_PInspWrapper = g_pMManager->pem_new<PInspAlgoWrapper>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pPatternTeach = g_pMManager->pem_new<CPInsp_Pattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pPatternInsp = g_pMManager->pem_new<CPInsp_Pattern>(true, 1, (PCHAR)__FUNCTION__, __LINE__);
	m_pColorTeach = g_pMManager->pem_new<CPInsp_Color>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pColorInsp = g_pMManager->pem_new<CPInsp_Color>(true, 1, (PCHAR)__FUNCTION__, __LINE__);
	m_pGridInsp = g_pMManager->pem_new<CPInsp_Grid>(true, 1, (PCHAR)__FUNCTION__, __LINE__);
	m_pFovImgBufBig = g_pMManager->pem_new<InspImgBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	m_pPOCR = g_pMManager->pem_new<CPInsp_POCR>(false, 1, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pForeignPattern = g_pMManager->pem_new<CPInsp_ForeignPattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pWireBondingInsp = g_pMManager->pem_new<CPInsp_WireBonding>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pTab = g_pMManager->pem_new<CPInsp_Tab>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);


	CAlgoFactory algoFac;
	for (int i = 0; i < InspAlgoType::eAlgoNum; i++)
		m_dicInspAlgorithm[(InspAlgoType)i] = algoFac.CreatePInspAlgo((InspAlgoType)i);

}

int InspManager::InspProc()
{
//	g_pMPTI->SharedInit(1);
	int ret = InspNormal_Ver2(true);
// 	ext::ClientCtrl::get()->AddResultData(g_pMPTI->m_InspMng->m_inspectionResult,1);
// 	InspectionResult* m_pTest; 
// 	ext::ClientCtrl::get()->GetResultData(m_pTest, 1);
//	SaveInspPartImage();

	return ret;
}
//int InspManager::InspNormal(bool bUseDecision )
//{
//
//	int ret = 100;
//	
//	// Align에 Lead는 추후 새로 개발해야 하므로 제외 함
//	int nAlignWndCnt = m_inspItemCnts[eINSP_MOUNT] + m_inspItemCnts[eINSP_ALIGN]/* + m_groupIndexCnts[eINSP_LEADSOLDER]*/;
//	AlignResult* arrAlignRes = NULL;
//	PIAL::PInspDataSet* InspDataSet = nullptr; // AVI 장비이거나 아니면, 특정옵션을 켰을때 동작되게 해야할듯..?
//	//if (m_pInspBoardInfo->SemiConductor == 1)
//		InspDataSet = new PIAL::PInspDataSet();
//
//	if (nAlignWndCnt > 0)
//		arrAlignRes = new AlignResult[nAlignWndCnt];
//	int wndStartIndex = 0;
//	int groupCnt = 0;
//	int wndType = 0;
//	int groupIndex = 0;
//	int nRefIndex = 0;
//	int** inspDefectTemp = NULL;
//	inspDefectTemp = new int*[eINSP_WND_CNT];
//	for (int i = 0; i < eINSP_WND_CNT; i++)
//	{
//		if (m_inspItemCnts[i] > 0)
//		{
//			inspDefectTemp[i] = new int[m_inspItemCnts[i]];
//			
//			for (int j = 0; j < m_inspItemCnts[i]; j++)
//				inspDefectTemp[i][j] = e_SKIP;
//		}
//		else
//			inspDefectTemp[i] = nullptr;
//	}
//	
//	InitResultVal();
//	BOOL bAutoInsp = TRUE;
//	wndType = eINSP_ALIGN;
//	groupCnt = m_groupIndexCnts[wndType];
//	for (int i = 0; i < groupCnt; i++)
//	{
//		groupIndex = m_groupIndex[wndType][i];
//
//		int wndId = 0;
//		BOOL bOKFlag = FALSE;
//		m_inspectionResult->isInspAlign = TRUE;
//		for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//		{
//			wndId = m_groupWndID[groupIndex][j];
//			if (m_pParamArray[wndId].m_nAlignPartWnd == 1)
//			{
//				if (bAutoInsp == TRUE)
//				{
//					inspDefectTemp[wndType][j + wndStartIndex] = e_OK;
//					m_inspectionResult->alignResult[j + wndStartIndex].m_bOk = TRUE;
//				}
//				continue;
//			}
//			if (m_pParamArray[wndId].nAlignWndID > 0)
//				continue;
//			//inspDefectTemp[wndType][j + wndStartIndex] = InspWindowAlgo(wndId, arrAlignRes, nRefIndex++, &m_inspectionResult->alignResult[j + wndStartIndex], InspDataSet);	// 할 것, Result 구조체 리턴 받기
//			if (bOKFlag == FALSE)
//			{
//				if (inspDefectTemp[wndType][j + wndStartIndex] == e_OK)
//					bOKFlag = TRUE;
//			}
//		}
//		if (bOKFlag)
//		{
//			for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//			{
//				wndId = m_groupWndID[groupIndex][j];
//				if (m_pParamArray[wndId].m_nAlignPartWnd == 1)
//				{
//					if (bAutoInsp == TRUE)
//					{
//						inspDefectTemp[wndType][j + wndStartIndex] = e_OK;
//						m_inspectionResult->alignResult[j + wndStartIndex].m_bOk = TRUE;
//					}
//					continue;
//				}
//				if (m_pParamArray[wndId].nAlignWndID > 0)
//					continue;
//				m_inspectionResult->alignResult[j + wndStartIndex].m_bOk = TRUE;
//				inspDefectTemp[wndType][j + wndStartIndex] = e_OK;
//			}
//		}
//		wndStartIndex += m_groupWndCnts[groupIndex];
//	}
//
//	PreviousInspWireShort();
//
//	wndType = eINSP_MOUNT;
//	wndStartIndex = 0;
//	groupCnt = m_groupIndexCnts[wndType];
//	for (int i = 0; i < groupCnt; i++)
//	{
//		groupIndex = m_groupIndex[wndType][i];
//
//		int wndId = 0;
//		BOOL bOKFlag = FALSE;
//		m_inspectionResult->isInspMount = TRUE;
//		for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//		{
//			if (bOKFlag == FALSE)
//			{
//				wndId = m_groupWndID[groupIndex][j];
//
//				if (m_pParamArray[wndId].InspWire > 0)
//					inspDefectTemp[wndType][j + wndStartIndex] = InspWindowAlgo2(wndType, wndId, arrAlignRes, nRefIndex++, &m_inspectionResult->mountResult[j + wndStartIndex], nullptr, false);
//				/*else
//					inspDefectTemp[wndType][j + wndStartIndex] = InspWindowAlgo(wndId, arrAlignRes, nRefIndex++, &m_inspectionResult->mountResult[j + wndStartIndex], InspDataSet);*/
//
//				if (inspDefectTemp[wndType][j + wndStartIndex] == e_OK)
//				{
//					bOKFlag = TRUE;
//				}
//			}
//			else
//			{
//				m_inspectionResult->mountResult[j + wndStartIndex].m_bOk = TRUE;
//				inspDefectTemp[wndType][j + wndStartIndex] = e_SKIP;
//			}
//		}
//		wndStartIndex += m_groupWndCnts[groupIndex];
//	}
//	wndType = eINSP_OCR;
//	wndStartIndex = 0;
//	groupCnt = m_groupIndexCnts[wndType];
//	for (int i = 0; i < groupCnt; i++)
//	{
//		groupIndex = m_groupIndex[wndType][i];
//
//		int wndId = 0;
//		BOOL bOKFlag = FALSE;
//		m_inspectionResult->isInspOcr = TRUE;
//		for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//		{
//			if (bOKFlag == FALSE)
//			{
//				wndId = m_groupWndID[groupIndex][j];
//
//				//inspDefectTemp[wndType][j + wndStartIndex] = InspWindowAlgo(wndId, arrAlignRes, nRefIndex++, &m_inspectionResult->ocrResult[j + wndStartIndex], InspDataSet);
//
//				if (inspDefectTemp[wndType][j + wndStartIndex] == e_OK)
//				{
//					bOKFlag = TRUE;
//				}
//			}
//			else
//			{
//				m_inspectionResult->mountResult[j + wndStartIndex].m_bOk = TRUE;
//				inspDefectTemp[wndType][j + wndStartIndex] = e_SKIP;
//			}
//		}
//		wndStartIndex += m_groupWndCnts[groupIndex];
//	}
//
//	if (m_NumWireAlgo > 1)
//	{
//		wndStartIndex = 0;
//		InspWireShort();
//		//재판정
//		for (int i = 0; i < groupCnt; i++)
//		{
//			int groupIndex = m_groupIndex[eINSP_MOUNT][i];
//			int wndId = 0;
//			for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//			{
//				wndId = m_groupWndID[groupIndex][j];
//				InspWndResult* WndResult = &m_inspectionResult->mountResult[j + wndStartIndex];
//				for (int nAlgo = 0; nAlgo < m_pParamArray[wndId].nAlgorithmCnt; nAlgo++)
//				{
//					InspAlgo sInspAlgo = m_pParamArray[wndId].vArrAlgoParam[nAlgo];
//
//
//					if (sInspAlgo.m_eAlgoType == eAlgoWire)
//					{
//						AlgoWire *pAlgo = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
//						RstAlgoWire *pRst = (RstAlgoWire *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//						if (pRst->m_bOK == FALSE)
//						{
//							WndResult->m_bOk = FALSE;
//							WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = FALSE;
//						}
//					}
//
//				}
//			}
//			wndStartIndex += m_groupWndCnts[groupIndex];
//		}
//	}
//	
//	wndType = eINSP_PAD;
//	wndStartIndex = 0;
//	groupCnt = m_groupIndexCnts[wndType];
//	for (int i = 0; i < groupCnt; i++)
//	{
//		groupIndex = m_groupIndex[wndType][i];
//
//		int wndId = 0;
//		BOOL bOKFlag = FALSE;
//		m_inspectionResult->isInspPad = TRUE;
//		for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//		{
//			if (bOKFlag == FALSE)
//			{
//				wndId = m_groupWndID[groupIndex][j];
//				//inspDefectTemp[wndType][j + wndStartIndex] = InspWindowAlgo( wndId, nullptr, 0, &m_inspectionResult->PadResult[j + wndStartIndex], InspDataSet);
//				if (inspDefectTemp[wndType][j + wndStartIndex] == e_OK)
//				{
//					bOKFlag = TRUE;
//				}
//			}
//			else
//			{
//				m_inspectionResult->PadResult[j + wndStartIndex].m_bOk = TRUE;
//				inspDefectTemp[wndType][j + wndStartIndex] = e_SKIP;
//			}
//		}
//		wndStartIndex += m_groupWndCnts[groupIndex];
//	}
//
//	wndType = eINSP_BGA;
//	wndStartIndex = 0;
//	groupCnt = m_groupIndexCnts[wndType];
//	for (int i = 0; i < groupCnt; i++)
//	{
//		groupIndex = m_groupIndex[wndType][i];
//
//		int wndId = 0;
//		BOOL bOKFlag = FALSE;
//		m_inspectionResult->isInspBGA = TRUE;
//		for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//		{
//			if (bOKFlag == FALSE)
//			{
//				wndId = m_groupWndID[groupIndex][j];
//				inspDefectTemp[wndType][j + wndStartIndex] = InspBGAWindowAlgo(wndId, arrAlignRes, nAlignWndCnt, &m_inspectionResult->BGAResult[j + wndStartIndex], InspDataSet,pPol);
//				if (inspDefectTemp[wndType][j + wndStartIndex] == e_OK)
//				{
//					bOKFlag = TRUE;
//				}
//			}
//			else
//			{
//				m_inspectionResult->BGAResult[j + wndStartIndex].m_bOk = TRUE;
//				inspDefectTemp[wndType][j + wndStartIndex] = e_SKIP;
//			}
//		}
//		wndStartIndex += m_groupWndCnts[groupIndex];
//	}
//	
//	wndType = eINSP_SOLDER;
//	wndStartIndex = 0;
//	groupCnt = m_groupIndexCnts[wndType];
//	for (int i = 0; i < groupCnt; i++)
//	{
//		groupIndex = m_groupIndex[wndType][i];
//
//		int wndId = 0;
//		BOOL bOKFlag = FALSE;
//		m_inspectionResult->isInspSolder = TRUE;
//		for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
//		{
//			if (bOKFlag == FALSE)
//			{
//				wndId = m_groupWndID[groupIndex][j];
//				//inspDefectTemp[wndType][j + wndStartIndex] = InspWindowAlgo(wndId, arrAlignRes, nAlignWndCnt, &m_inspectionResult->solderResult[j + wndStartIndex], InspDataSet);
//				if (inspDefectTemp[wndType][j + wndStartIndex] == e_OK)
//				{
//					bOKFlag = TRUE;
//				}
//			}
//			else
//			{
//				m_inspectionResult->solderResult[j + wndStartIndex].m_bOk = TRUE;
//				inspDefectTemp[wndType][j + wndStartIndex] = e_SKIP;
//			}
//		}
//		wndStartIndex += m_groupWndCnts[groupIndex];
//	}
//	int nSub = 0; 
//	//ret = m_pInspDecision->InspDecision(m_inspectionResult, inspDefectTemp, nSub);
//	//////////////////////////////////////////////////////////////////////////
//	//	MakeInspResult();
//	if (bUseDecision)
//		ret = ((CPInspDecision *)m_pInspDecision)->InspDecision(m_inspectionResult, inspDefectTemp, nSub);
//	/////////////////////////////////////////////////////////////////////////
//
//	for (int i = 0; i < eINSP_WND_CNT; i++)
//	{
//		if (inspDefectTemp[i] != NULL)
//			delete[] inspDefectTemp[i];
//	}
//
//	delete[] inspDefectTemp;
//	inspDefectTemp = NULL;
//
//	if(arrAlignRes)
//		delete[] arrAlignRes;
//
//	if (InspDataSet)
//		delete InspDataSet;
//
//	return ret;
//}

// Null-guarded read of nUseNGPolygon. ext::irc::get() is lazy-init so always non-null,
// but in minimal harness flows _CtrlServer._stream is uninitialized and First() returns
// nullptr, causing 0xC0000005 on the subsequent ->prod access. Default to FALSE on null.
static BOOL TryReadUseNGPolygonFlag()
{
	auto irc = ext::irc::get();
	if (!irc) return FALSE;
	auto* ctrl = irc->_CtrlServer.First();
	if (ctrl == nullptr) return FALSE;
	return ctrl->prod.nUseNGPolygon;
}

int InspManager::InspNormal_Ver2(bool bUseDecision)
{
	int _nOCRAlgoCnt = 0;// 할때마다 초기화 
	int m_nLeadTipPosIndex = 0; // 할때마다 초기화 

	wchar_t sDSIFullName[256] = {0 };// 
	CString strDSI;
	CString str(m_pInspBoardInfo->s3DImagePath);
	
	// SEH-guarded read: in minimal flows where ext::CopyParamToExt did not fully
	// initialize _CtrlServer, the original access raises 0xC0000005 outside the
	// function's try block - killing InspProc before InitResultVal runs. Default
	// to FALSE on any access failure. The guard MUST live in a separate function
	// because __try/__except cannot coexist with C++ object unwinding.
	if (TryReadUseNGPolygonFlag())
	{
		if (str.IsEmpty())
		{
			ext::Log::add(_T("[DSI]3D Path Null - DSI Make Fail"));
		}
		else
		{
			int dotPos = str.ReverseFind('.');
			if (dotPos != -1)
				strDSI = str.Left(dotPos); // 확장자는 당연히 없지만 혹시나 오면 바꿔주라고..
			else
				strDSI = str;
			// 새로운 확장자 추가
			strDSI += _T(".dsi");
			wcscpy_s(sDSIFullName, 256, strDSI.GetString());
		}
	}
	
	
	//wmemset(sDSIFullName, 0, 255); // 공정정보에서 받을 정보로 보여짐.. 받아야하면 공정정보에서..
	BOOL bAutoInsp = TRUE;
	int ret = dftCODE_OK;
	int nLine = __LINE__;
	try
	{
		//m_bNewInspection = false;
		DWORD start = GetTickCount();
		DWORD start2 = GetTickCount();
		if (m_pParamArray && m_pParamArray[0].vArrAlgoParam)
		{
			for (int n = 0; n < m_ParamArraySize; n++)
			{
				for (int a = 0; a < m_pParamArray[n].nAlgorithmCnt; a++)
				{
					InspAlgo sInspAlgo = m_pParamArray[n].vArrAlgoParam[a];
					if (sInspAlgo.m_nWhiteValue < 0 || sInspAlgo.m_nWhiteValue > 1000 || sInspAlgo.m_nGreenValue < 0 || sInspAlgo.m_nGreenValue > 1000 || !sInspAlgo.m_ptrInspAlgoParam)
						return -1;
				}
			}
		}
		nLine = __LINE__;

		int** inspDefectTemp = g_pMManager->pem_new<int*>(true, eINSP_WND_CNT, (PCHAR)__FUNCTION__, __LINE__);
		for (int i = 0; i < eINSP_WND_CNT; i++)
		{
			if (m_inspItemCnts[i] > 0)
			{
				inspDefectTemp[i] = g_pMManager->pem_new<int>(true, m_inspItemCnts[i], (PCHAR)__FUNCTION__, __LINE__);
				for (int j = 0; j < m_inspItemCnts[i]; j++)
					inspDefectTemp[i][j] = e_SKIP;
			}
			else
				inspDefectTemp[i] = NULL;
		}
		nLine = __LINE__;

		InitResultVal();		// Insp Param Set, Result 생성 및 초기화
		nLine = __LINE__;

		//Wire Short 검사 관련 [Refact] MPTILib - PInsp_Algo 구조 개선 4.0.0.05
		PreviousInspWireShort();
		nLine = __LINE__;

		//////////////////////////////////////////////////////////////////////////
		int wndStartIndex = 0;
		int groupCnt = 0;
		int wndType = 0;
		int groupIndex = 0;
		int nRefIndex = 0;
		int nAlignWndCnt = m_inspItemCnts[eINSP_MOUNT] + m_inspItemCnts[eINSP_ALIGN] + m_groupIndexCnts[eINSP_LEADSOLDER] + m_inspItemCnts[eINSP_TAB];
		AlignResult * arrAlignRes = NULL;	// Align window 결과 offset
		PIAL::PInspDataSet* InspDataSet = nullptr;
		if (m_bNewInspection)
		{
			InspDataSet = g_pMManager->pem_new<PIAL::PInspDataSet>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
			// 			InspDataSet->SetHeightOffset(nResultHeightOffset); // 아폴론에서 사용 데이터 Mars 추가 X 
			// 			InspDataSet->SetPickerOffset(nPickerOffset); // 아폴론에서 사용 데이터
		}

		if (nAlignWndCnt > 0)
			arrAlignRes = g_pMManager->pem_new<AlignResult>(true, nAlignWndCnt, (PCHAR)__FUNCTION__, __LINE__);
		nLine = __LINE__;
		m_fInspTact[m_eInspTact_WndInspInit] += ((GetTickCount() - start2) / 1000.0f);
		start2 = GetTickCount();

		//////////////////////////////////////////////////////////////////////////	
		wndType = eINSP_ALIGN;
		groupCnt = m_groupIndexCnts[wndType];
		// SHW 20170822 : Align Part
		if (bAutoInsp == FALSE) // MultiProcess 는 무조건 AutoInsp 
		{
			for (int i = 0; i < groupCnt; i++)
			{
				groupIndex = m_groupIndex[wndType][i];
				bool bInsp = false;
				for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
				{
					int wndId = m_groupWndID[groupIndex][j];
					if (m_pParamArray[wndId].m_nAlignPartWnd == 0)
						continue;
					AlignResult sPartAlignRes;
					if (m_inspectionResult->alignArraySize > j + wndStartIndex)
						int nRet = InspWindowAlgo3(wndType, wndId, &sPartAlignRes, 0, &m_inspectionResult->alignResult[j + wndStartIndex], InspDataSet);	// 할 것, Result 구조체 리턴 받기;
					bInsp = true;
					break;
				}
				if (bInsp == true)
					break;
			}
		}
		nLine = __LINE__;

		m_fInspTact[m_eInspTact_WndAlignPart] += ((GetTickCount() - start2) / 1000.0f);
		start2 = GetTickCount();
		nLine = __LINE__;
		//g_pInspMng->initMaskData();
		vector<InspRstPolyAlgo>* pInspRstPoly;
		if (sDSIFullName == NULL)
			pInspRstPoly = NULL;
		else
			pInspRstPoly = g_pMManager->pem_new<vector<InspRstPolyAlgo>>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		for (int nWndType = -1; nWndType < eINSP_WND_CNT; nWndType++)
		{
			if (nWndType == eINSP_USER)
				continue;
			else if (nWndType == -1)
				wndType = eINSP_ALIGN;
			else
				wndType = nWndType;

			wndStartIndex = 0;
			groupCnt = m_groupIndexCnts[wndType];
			InspAlgoTempResult * ptrInspAlgoTempResult = NULL;
			int nTotalCnt = groupCnt;
			int nParentIdBuf = 0;
			if (wndType == eINSP_LEADSOLDER)
			{
				for (int i = 0; i < groupCnt; i++)
					nTotalCnt += (m_groupIndex[wndType][i] + groupIndex);
				if (nTotalCnt <= 0) nTotalCnt = 1;
				ptrInspAlgoTempResult = new InspAlgoTempResult(nTotalCnt);
				g_pMManager->pem_new_check(ptrInspAlgoTempResult, (PCHAR)__FUNCTION__, __LINE__);
			}
			nLine = __LINE__;

			for (int i = 0; i < groupCnt; i++)
			{
				if (wndType == eINSP_OCR && i == 0)
					_nOCRAlgoCnt = 0;
				groupIndex = m_groupIndex[wndType][i];

				int wndId = 0;
				BOOL bOKFlag = FALSE;
				m_inspectionResult->SetInsp(wndType);
				for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
				{
					InspWndResult* WndResult = m_inspectionResult->GetWndResult(wndType);
					if (bOKFlag == FALSE || wndType == eINSP_ALIGN || (bOKFlag == TRUE && m_bInspSkipGroupWnd[wndType] == FALSE))
					{
						wndId = m_groupWndID[groupIndex][j];
						int nAlignCnt = nAlignWndCnt;
						if (wndType == eINSP_ALIGN)
						{
							if (m_pParamArray[wndId].m_nAlignPartWnd == 1)
							{
								if (bAutoInsp == TRUE)
								{
									inspDefectTemp[wndType][j + wndStartIndex] = e_OK;
									WndResult[j + wndStartIndex].m_bOk = TRUE;
								}
								continue;
							}
							if (nWndType == -1)
							{
								if (m_pParamArray[wndId].nAlignWndID > 0)
									continue;
							}
							else
							{
								if (m_pParamArray[wndId].nAlignWndID == 0)
									continue;
							}
						}
						else if (wndType == eINSP_LEADSOLDER)
						{
							ptrInspAlgoTempResult->m_bParentWnd = false;
							if (nParentIdBuf == 0)
							{
								if (m_pParamArray[wndId].nParentWndID != 0)
									nParentIdBuf = m_pParamArray[wndId].nParentWndID;
								m_nLeadTipPosIndex = 0;
								int nGroupIndex = 0;
								for (int a = 0; a < groupCnt; a++)
								{
									nGroupIndex = m_groupIndex[wndType][a];
									int nParentWndId = 0;
									for (int b = 0; b < m_groupWndCnts[nGroupIndex]; b++)
									{
										nParentWndId = m_groupWndID[nGroupIndex][b];
										if (nParentIdBuf == m_pParamArray[nParentWndId].wndIndex)
										{
											for (int nAlgo = 0; nAlgo < m_pParamArray[nParentWndId].nAlgorithmCnt; nAlgo++)
											{
												if (!m_pParamArray[nParentWndId].vArrAlgoParam)
													continue;
												InspAlgo sInspAlgo = m_pParamArray[nParentWndId].vArrAlgoParam[nAlgo];
												if (sInspAlgo.m_eAlgoType == eAlgoAlign || sInspAlgo.m_eAlgoType == eAlgoAlignEdge)
												{
													InspWndResult WndResult;
													WndResult.m_nAlgorithmCnt = m_pParamArray[nParentWndId].nAlgorithmCnt;
													InspWindowAlgo3(wndType, nParentWndId, arrAlignRes, nRefIndex, &WndResult, InspDataSet);
													nRefIndex++;
													break;
												}
											}
										}
									}
								}
							}
							if (nParentIdBuf == m_pParamArray[wndId].wndIndex)
								ptrInspAlgoTempResult->m_bParentWnd = true;
							ptrInspAlgoTempResult->m_ptrnLeadWndID[m_nLeadTipPosIndex] = m_pParamArray[wndId].wndIndex;

							if (nParentIdBuf != m_pParamArray[wndId].nParentWndID && ptrInspAlgoTempResult->m_bParentWnd == false)
							{
								memset(ptrInspAlgoTempResult->m_ptrnLeadTipPosResult, 0, sizeof(int)*nTotalCnt);
								memset(ptrInspAlgoTempResult->m_ptrdLeadLiftResult, 0, sizeof(double)*nTotalCnt);
								memset(ptrInspAlgoTempResult->m_ptrnLeadWndID, 0, sizeof(int)*nTotalCnt);
								m_nLeadTipPosIndex = 0;
								nParentIdBuf = m_pParamArray[wndId].nParentWndID;
								int nGroupIndex = 0;
								for (int a = 0; a < groupCnt; a++)
								{
									nGroupIndex = m_groupIndex[wndType][a];
									int nParentWndId = 0;
									for (int b = 0; b < m_groupWndCnts[nGroupIndex]; b++)
									{
										nParentWndId = m_groupWndID[nGroupIndex][b];
										if (nParentIdBuf == m_pParamArray[nParentWndId].wndIndex)
										{
											for (int nAlgo = 0; nAlgo < m_pParamArray[nParentWndId].nAlgorithmCnt; nAlgo++)
											{
												if (!m_pParamArray[nParentWndId].vArrAlgoParam)
													continue;
												InspAlgo sInspAlgo = m_pParamArray[nParentWndId].vArrAlgoParam[nAlgo];
												if (sInspAlgo.m_eAlgoType == eAlgoAlign || sInspAlgo.m_eAlgoType == eAlgoAlignEdge)
												{
													if (m_inspectionResult->leadSolderArraySize > b + wndStartIndex)
														inspDefectTemp[wndType][b + wndStartIndex] = InspWindowAlgo3(wndType, nParentWndId, arrAlignRes, nRefIndex++, &m_inspectionResult->leadSolderResult[b + wndStartIndex], InspDataSet);
													break;
												}
											}
										}
									}
								}
							}
						}

						if (wndType == eINSP_ALIGN || wndType == eINSP_MOUNT || wndType == eINSP_LEADSOLDER || wndType == eINSP_TAB)
							nAlignCnt = nRefIndex;
						nLine = __LINE__;

						bool bLeadInsp = (wndType == eINSP_LEADSOLDER);
						if (wndType == eINSP_BGA)
							inspDefectTemp[wndType][j + wndStartIndex] = InspBGAWindowAlgo(wndId, arrAlignRes, nAlignCnt, &WndResult[j + wndStartIndex], InspDataSet, pInspRstPoly);
						else
							inspDefectTemp[wndType][j + wndStartIndex] = InspWindowAlgo3(wndType, wndId, arrAlignRes, nAlignCnt, &WndResult[j + wndStartIndex], InspDataSet, ptrInspAlgoTempResult, bLeadInsp, pInspRstPoly);
						nLine = __LINE__;

						if (wndType == eINSP_ALIGN || wndType == eINSP_MOUNT || wndType == eINSP_TAB)
							nRefIndex++;
						else if (wndType == eINSP_LEADSOLDER)
							m_nLeadTipPosIndex++;

						if (inspDefectTemp[wndType][j + wndStartIndex] == e_OK)
							bOKFlag = TRUE;
					}
					else
					{
					CAlgoFactory cloneFactory;
					void * vRstInspAlgoTemp = nullptr;
					wndId = m_groupWndID[groupIndex][j];
					BOOL bUSeLeadAlign = FALSE;
					bool IsUseMultiArea = false;
					for (int nAlgo = 0; nAlgo < m_pParamArray[wndId].nAlgorithmCnt; nAlgo++)
					{
						DWORD start = GetTickCount();
						InspAlgo sInspAlgo = m_pParamArray[wndId].vArrAlgoParam[nAlgo];
						InspAlgoType eInspAlgoType = sInspAlgo.m_eAlgoType;
						if (eInspAlgoType == eINSP_LEADSOLDER && ptrInspAlgoTempResult == NULL)
						{
							if (eInspAlgoType != eAlgoAlign && eInspAlgoType != eAlgoAlignEdge)
								continue;
							else
								bUSeLeadAlign = TRUE;
						}

						if (eInspAlgoType == eAlgoTab || eInspAlgoType == eAlgoColor || eInspAlgoType == eAlgoLead_Color)
						{
							AlgoColor* pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
							if (pAlgoColor && pAlgoColor->IsUseMultiArea())
								IsUseMultiArea = true;
						}

						if (bUSeLeadAlign == TRUE && (eInspAlgoType == eAlgoAlign || eInspAlgoType == eAlgoAlignEdge))
							cloneFactory.CreateRstAlgoClone(eInspAlgoType, vRstInspAlgoTemp, IsUseMultiArea);
						else
						{
							cloneFactory.CreateRstAlgoClone(eInspAlgoType, WndResult[j + wndStartIndex].m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, IsUseMultiArea);
							//cloneFactory.CreateRstAlgoClone(eInspAlgoType, WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, IsUseMultiArea);
							vRstInspAlgoTemp = WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
						}
					}
					

						WndResult[j + wndStartIndex].m_bOk = TRUE;
						inspDefectTemp[wndType][j + wndStartIndex] = e_SKIP;
					}
				}
				if (nWndType == -1 && bOKFlag)
				{
					for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
					{
						wndId = m_groupWndID[groupIndex][j];
						if (m_pParamArray[wndId].m_nAlignPartWnd == 1)
						{
							if (bAutoInsp == TRUE)
							{
								inspDefectTemp[wndType][j + wndStartIndex] = e_OK;
								if (m_inspectionResult->alignArraySize > j + wndStartIndex)
									m_inspectionResult->alignResult[j + wndStartIndex].m_bOk = TRUE;
							}
							continue;
						}
						if (m_pParamArray[wndId].nAlignWndID > 0)
							continue;
						if (m_inspectionResult->alignArraySize > j + wndStartIndex)
							m_inspectionResult->alignResult[j + wndStartIndex].m_bOk = TRUE;
						inspDefectTemp[wndType][j + wndStartIndex] = e_OK;
					}
				}
				wndStartIndex += m_groupWndCnts[groupIndex];
			}
			if (wndType == eINSP_MOUNT)
			{
				//Insp Wire Short
				if (m_NumWireAlgo > 1)
				{
					wndStartIndex = 0;
					InspWireShort();
					for (int i = 0; i < groupCnt; i++)
					{
						int groupIndex = m_groupIndex[eINSP_MOUNT][i];
						int wndId = 0;
						for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
						{
							wndId = m_groupWndID[groupIndex][j];
							if (m_inspectionResult->mountArraySize <= j + wndStartIndex)
								break;
							InspWndResult* WndResult = &m_inspectionResult->mountResult[j + wndStartIndex];
							for (int nAlgo = 0; nAlgo < m_pParamArray[wndId].nAlgorithmCnt; nAlgo++)
							{
								InspAlgo sInspAlgo = m_pParamArray[wndId].vArrAlgoParam[nAlgo];
								if (sInspAlgo.m_eAlgoType == eAlgoWire)
								{
									AlgoWire *pAlgo = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
									RstAlgoWire *pRst = (RstAlgoWire *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
									if (pRst->m_bOK == FALSE)
									{
										WndResult->m_bOk = FALSE;
										WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = FALSE;
									}
								}

							}
						}
						wndStartIndex += m_groupWndCnts[groupIndex];
					}
				}
				nLine = __LINE__;
			}
			else if (wndType == eINSP_LEADSOLDER)
			{
				ptrInspAlgoTempResult->Destroy();

				// MemoryLeak Fix -wjlee
				g_pMManager->pem_delete(ptrInspAlgoTempResult, false);
				ptrInspAlgoTempResult = NULL;
				nLine = __LINE__;

				// Algo Insp Result Change
				groupCnt = m_groupIndexCnts[wndType];
				wndStartIndex = 0;
				InspAlgoTempResult * ptrInspAlgoChangeResult = NULL;
				if (groupCnt > 0)
				{
					ptrInspAlgoChangeResult = new InspAlgoTempResult(groupCnt);
					g_pMManager->pem_new_check(ptrInspAlgoChangeResult, (PCHAR)__FUNCTION__, __LINE__);
				}

				nParentIdBuf = 0;
				bool bCheckParent = false;
				for (int i = 0; i < groupCnt; i++)
				{
					groupIndex = m_groupIndex[wndType][i];

					int wndId = 0;
					for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
					{
						wndId = m_groupWndID[groupIndex][j];
						if (nParentIdBuf == 0)
						{
							if (m_pParamArray[wndId].nParentWndID != 0)
								nParentIdBuf = m_pParamArray[wndId].nParentWndID;
							m_nLeadTipPosIndex = 0;
						}
						if (nParentIdBuf != m_pParamArray[wndId].nParentWndID && ptrInspAlgoChangeResult->m_bParentWnd == false)
						{
							memset(ptrInspAlgoChangeResult->m_ptrnLeadTipPosResult, 0, sizeof(int)*groupCnt);
							memset(ptrInspAlgoChangeResult->m_ptrdLeadLiftResult, 0, sizeof(double)*groupCnt);
							memset(ptrInspAlgoChangeResult->m_ptrnLeadWndID, 0, sizeof(int)*groupCnt);
							m_nLeadTipPosIndex = 0;
							nParentIdBuf = m_pParamArray[wndId].nParentWndID;
						}
						if (nParentIdBuf != 0 && bCheckParent == false)
						{
							int nWndStartIndex = 0;
							for (int a = 0; a < groupCnt; a++)
							{
								int nGroupIndex = m_groupIndex[wndType][a];
								int nWndId = 0;
								for (int b = 0; b < m_groupWndCnts[nGroupIndex]; b++)
								{
									nWndId = m_groupWndID[nGroupIndex][b];
									if (nParentIdBuf == m_pParamArray[nWndId].wndIndex)
									{
										ptrInspAlgoChangeResult->m_bParentWnd = true;
										if (m_inspectionResult->leadSolderArraySize <= b + nWndStartIndex)
											break;
										ChangeInspAlgoResult(nWndId, &m_inspectionResult->leadSolderResult[b + nWndStartIndex], ptrInspAlgoChangeResult);
										bCheckParent = true;
										ptrInspAlgoChangeResult->m_bParentWnd = false;
										break;
									}
								}
								nWndStartIndex += m_groupWndCnts[nGroupIndex];
								if (bCheckParent == true)
									break;
							}
						}
						if (nParentIdBuf == m_pParamArray[wndId].wndIndex || nParentIdBuf == 0)
						{
							bCheckParent = false;
							continue;
						}
						if (m_inspectionResult->leadSolderArraySize > j + wndStartIndex)
							int nResult = ChangeInspAlgoResult(wndId, &m_inspectionResult->leadSolderResult[j + wndStartIndex], ptrInspAlgoChangeResult);
						m_nLeadTipPosIndex++;
					}
					wndStartIndex += m_groupWndCnts[groupIndex];
				}
				nLine = __LINE__;

				if (ptrInspAlgoChangeResult)
				{
					ptrInspAlgoChangeResult->Destroy();
					g_pMManager->pem_delete(ptrInspAlgoChangeResult, false);
					ptrInspAlgoChangeResult = NULL;
				}
				nLine = __LINE__;
			}

			if (ptrInspAlgoTempResult)
			{
				ptrInspAlgoTempResult->Destroy();
				g_pMManager->pem_delete(ptrInspAlgoTempResult, false);
				ptrInspAlgoTempResult = NULL;
			}
			m_fInspTact[m_eInspTact_WndM + (wndType - eINSP_MOUNT)] += ((GetTickCount() - start2) / 1000.0f);
			start2 = GetTickCount();
			nLine = __LINE__;
		}

		// 		if (m_vDistAlgoList.size() > 0) //Distance 알고리즘은 현재 미구현.
		// 		{
		// 			InspAlgoParam tmpStAlgoParam;
		// 			tmpStAlgoParam.m_arrAlignRes = arrAlignRes;
		// 
		// 			WndAlgoImg tmpImg;
		// 			TotalInspExceptArea tmpArea;
		// 
		// 			CAlgoFactory cloneFactory;
		// 			void * vRstInspAlgoTemp = nullptr;
		// 
		// 			nLine = __LINE__;
		// 
		// 			int nSize = m_vDistAlgoList.size();
		// 			for (int i = 0; i < nSize; i++)
		// 			{
		// 				InspWndResult *WndResult = (InspWndResult*)m_vDistAlgoList[i].wndResult;
		// 				int nAlgo = m_vDistAlgoList[i].nAlgoIdx;
		// 
		// 				nLine = __LINE__;
		// 
		// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = FALSE;
		// 
		// 				cloneFactory.CreateRstAlgoClone(eAlgoDistance, WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, false);
		// 				vRstInspAlgoTemp = WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
		// 
		// 				nLine = __LINE__;
		// 
		// 				InspAlgo algo = GetDistanceInspAlgo(i);
		// 
		// 				bool bAlgoOK = m_dicInspAlgorithm[eAlgoDistance]->InspAlgorithm(algo, tmpImg, NULL, vRstInspAlgoTemp, tmpArea, tmpStAlgoParam, NULL);
		// 
		// 				if (m_bNewInspection && InspDataSet != NULL)
		// 				{
		// 					std::vector<PIAL::PInspData*> pDataSet;
		// 					if (InspDataSet) pDataSet = InspDataSet->GetInspDataSet();
		// 					void* rstDist = vRstInspAlgoTemp;
		// 					RstAlgoDistance *rstAlgo = (RstAlgoDistance *)rstDist;
		// 					std::vector<PIAL::_AlignResult> vecAlignResult;
		// 					m_PInspWrapper->ConvertAlignResult(arrAlignRes, nAlignWndCnt, vecAlignResult);
		// 					PIAL::PInspData* pdata = PIAL::PInspDataSet::GetCurrentBodyData(pDataSet, vecAlignResult);
		// 
		// 					if (pdata != NULL && pdata->GetBodyData())
		// 					{
		// 						PIAL::BodyInfo* body = pdata->GetBodyData();
		// 
		// 						if (body->BodyStartPos)
		// 						{
		// 							rstAlgo->m_nRstBodyX = body->BodyStartPos->x;
		// 							rstAlgo->m_nRstBodyY = body->BodyStartPos->y;
		// 						}
		// 
		// 						if (body->EntireBodyEdge.size() >= 3)
		// 						{
		// 							rstAlgo->m_nRstBodyWidth = body->EntireBodyEdge[1].x - body->EntireBodyEdge[0].x;
		// 							rstAlgo->m_nRstBodyLength = body->EntireBodyEdge[2].y - (body->EntireBodyEdge[1].y);
		// 						}
		// 					}
		// 
		// 				}
		// 
		// 				nLine = __LINE__;
		// 
		// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;
		// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = bAlgoOK;
		// 
		// 				WndResult->m_bIsInsp = TRUE;
		// 				WndResult->m_bOk = (WndResult->m_bOk == e_OK) && bAlgoOK ? e_OK : e_NG;
		// 
		// 				// 교체용 디펙트 코드
		// 				if (!bAlgoOK && algo.m_bUsingManualDefectCode)
		// 				{
		// 					WndResult->m_vArrRstInspAlgo[nAlgo].m_bUsingManualDefectCode = algo.m_bUsingManualDefectCode;
		// 					WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualDefectCode = algo.m_nManualDefectCode;
		// 					WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualSubDefectCode = algo.m_nManualSubDefectCode;
		// 				}
		// 
		// 				nLine = __LINE__;
		// 			}
		// 
		// 			m_vDistAlgoList.clear();
		// 
		// 			nLine = __LINE__;
		// 		}
				//////////////////////////////////////////////////////////////////////////
		int nSub = 0;
		start2 = GetTickCount();
		ret = ((CPInspDecision *)m_pInspDecision)->InspDecision(m_inspectionResult, inspDefectTemp, nSub);
		if (pInspRstPoly)
		{
			if (ret != dftCODE_OK)
			{
				int nPartID = m_pInspBoardInfo->nPartIDOrg;
				int nImageWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
				int nImageHeight = m_pInspBoardInfo->partImgBuf.nImageSizeY;
				CString sFullName(sDSIFullName);
				CString sFileName = sFullName.Mid(sFullName.ReverseFind('\\') + 1);
				bool bInspInTeach = sFileName.Find(_T("@T")) > 0;
				DSI dsi;
				if (m_bNewInspection)
					dsi.SetDSIData_new(nPartID, nImageWidth, nImageHeight, pInspRstPoly, bInspInTeach);
				else
					dsi.SetDSIData(nPartID, nImageWidth, nImageHeight, pInspRstPoly, bInspInTeach);

				if (dsi.m_disHeader.m_nDSNum > 0)
					dsi.WriteDSIFile(sFullName);
			}
			g_pMManager->pem_delete(pInspRstPoly, false);
		}
		m_fInspTact[m_eInspTact_WndInspDecision] += ((GetTickCount() - start2) / 1000.0f);
		start2 = GetTickCount();
		nLine = __LINE__;

		///////////////////////////////////////////////////////////////////////////
		for (int i = 0; i < eINSP_WND_CNT; i++)
		{
			if (inspDefectTemp[i] != NULL)
				g_pMManager->pem_delete(inspDefectTemp[i], true);
		}
		g_pMManager->pem_delete(inspDefectTemp, true);
		inspDefectTemp = NULL;
		nLine = __LINE__;

		// 		m_resultAlign.Init();
		// 		if (arrAlignRes)
		// 		{
		// 			for (int i = nAlignWndCnt - 1; i >= 0; --i)
		// 			{
		// 				if (arrAlignRes[i].nWindowID == -1)
		// 					continue;
		// 				m_resultAlign = arrAlignRes[i];
		// 				break;
		// 			}
		// 		}
		// 		nLine = __LINE__;
		m_fInspTact[m_eInspTact_WndDelete1] += ((GetTickCount() - start2) / 1000.0f);
		start2 = GetTickCount();

		if (arrAlignRes)
		{
			g_pMManager->pem_delete(arrAlignRes, true);
			arrAlignRes = NULL;
		}

		if (InspDataSet)
		{
			g_pMManager->pem_delete(InspDataSet, false);
			InspDataSet = nullptr;
		}

		m_fInspTact[m_eInspTact_WndDelete2] += ((GetTickCount() - start2) / 1000.0f);
		m_fInspTact[m_eInspTact_WndInspEnd] += ((GetTickCount() - start) / 1000.0f);
		nLine = __LINE__;
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("InspManager::Normal_Ver2(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return ret;

}
//int InspManager::InspWindowAlgo(int nWndIndex, AlignResult* arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, PIAL::PInspDataSet* InspDataSet, InspAlgoTempResult * ptrInspAlgoTempResult)
//{
//	int ret = e_OK;
//	int inspType = 0;
//	int nLine = __LINE__; 
//
//	BOOL bUSeLeadAlign = FALSE; // 현재는 사용할수 없음.
//	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
//	BOOL bAlignResOK = FALSE;
//	AlignResult * ptrAlignRes = NULL;
//	try
//	{
//		inspType = m_pParamArray[nWndIndex].inspType;
//		PIAL::PInspData* InspData = nullptr; // 삭제하지 말 것..(소멸자에서 삭제함)
//		if (InspDataSet != nullptr) // null이 아닌경우는 Semiconductor가 True인 경우임.
//		{
//			InspData = new PIAL::PInspData;
//			InspData->WindowID = m_pParamArray[nWndIndex].wndIndex;
//			InspDataSet->SetInspData(InspData);
//		}
//		
//		int nAlignCnt = 0;
//
//		m_PInspWrapper->SetPartImage(*m_pInspBoardInfo, pImg_buf);
//		FindAlignResult(ptrAlignRes, arrAlignRes, nWndIndex, m_pParamArray[nWndIndex].nAlignWndID, nAlignWndIdx, &nAlignCnt);
//
//		int nCurAlignID = 0;
//		for (int n = 0; n < nAlignCnt; n++)
//		{
//			if (ptrAlignRes[n].nWindowID == m_pParamArray[nWndIndex].nAlignWndID ||
//				ptrAlignRes[n].nWindowID == m_pParamArray[nWndIndex].wndIndex)
//			{
//				nCurAlignID = n;
//				break;
//			}
//		}
//
//
//		if (nAlignCnt < 1)
//		{
//			nAlignCnt = 1;
//			if (ptrAlignRes == NULL)
//			{
//				ptrAlignRes = new AlignResult[1];
//				ptrAlignRes[0].nWindowID = -1;
//			}
//		}
//
//		CAtlArray<CRect> rcArrTab;
//		AlgoCoordinate coordinateAlgo;
//		WndInfo sWndInfo;
//		WndAlgoImg wndImg;
//		int nOffX_pix = 0;
//		int nOffY_pix = 0;
//
//		CString dOCRPath = _T("");
//		CString dOCRCopyPath = _T("");
//
//		int nLeadTipPos = -1;
//		BOOL bUseLeadTipSideOption = FALSE;
//		double dLeadTipShiftX = -1;
//		double dLeadTipShiftY = -1;
//		double dLeadTipSize = -1;
//		int nLeadTipDir = -1;
//		double OCRNGScore = 0.0;
//		double OCROKScore = 0.0;
//		BOOL USENGOCR = FALSE;
//		int nWndDir = e_LEFT;
//		BOOL bUSeLeadAlign = FALSE;
//		int *ptrnArrLeadTipRst = NULL;
//		int nLeadTipTotalCnt = 0;
//		int nInspLeadTipCnt = 0;
//
//		bool bAlgoOK = true;
//
//		int wnd_w = 0;
//		int wnd_h = 0;
//		double dX = 0.0;
//		double dY = 0.0;
//		int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
//		int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;
//
//// 		int nOffX_pix = 0;
//// 		int nOffY_pix = 0;
//		bool bUseDiameter = false;
//		POINTF poBodyCenter;
//		poBodyCenter.x = -1;
//		poBodyCenter.y = -1;
//
//		std::vector<PIAL::_AlignResult> vecAlignResult;
//		m_PInspWrapper->ConvertAlignResult(ptrAlignRes, nAlignCnt, vecAlignResult);
//
//
//		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
//		{
//			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
//			if (sInspAlgo.m_eAlgoType == eAlgoLead_Tip)
//				nLeadTipTotalCnt++;
//		}
//		if (nLeadTipTotalCnt > 0)
//		{
//			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrnArrLeadTipRst, nLeadTipTotalCnt);
//			memset(ptrnArrLeadTipRst, 0, sizeof(int) * nLeadTipTotalCnt);
//		}
//
//
//		//Pattern검사용
//		//WndAlgoImg wndImg;
//
//		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
//		{	
//
//			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
//			InspAlgoType eInspAlgoType = sInspAlgo.m_eAlgoType;
//			DWORD start = GetTickCount();
//			AlgoCoordinate coordinateAlgo;
//			
//
//			TotalInspExceptArea _stTieArea;
//			if (inspType == eINSP_LEADSOLDER && ptrInspAlgoTempResult == NULL)
//			{
//				if (sInspAlgo.m_eAlgoType != eAlgoAlign && sInspAlgo.m_eAlgoType != eAlgoAlignEdge)
//					continue;
//				else
//					bUSeLeadAlign = TRUE;
//			}
//// 			if (eInspAlgoType == eAlgoDistance)
//// 			{
//// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = eAlgoDistance;
//// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;
//// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = TRUE;
//// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;
//// 				AddDistanceAlgoList(nWndIndex, nAlgo, WndResult);
//// 				continue;
//// 			}
//			if (bUSeLeadAlign == FALSE && (!m_pParamArray[nWndIndex].vArrAlgoParam || !WndResult->m_vArrRstInspAlgo))
//				continue;
//			if (sInspAlgo.m_eAlgoType == eAlgoOCR) //OCR 은 못타게 
//				continue;
//
//			if (bUSeLeadAlign == FALSE)
//			{
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = sInspAlgo.m_eAlgoType;
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;
//			}
//
//			if (sInspAlgo.m_eAlgoType == (int)eAlgoPattern || sInspAlgo.m_eAlgoType == (int)eAlgoPOCR)
//			{
//				wndImg.Destroy();
//
//				_stTieArea.m_nUsedWndPolygon = m_pParamArray[nWndIndex].m_nUsedWndPolygon;
//				for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
//					_stTieArea.m_ptArrWndPolygon[i] = m_pParamArray[nWndIndex].m_ptArrWndPolygon[i];
//
//				//WndAlgoImg 구조체 생성
//				WndSizeChange(sInspAlgo.m_eAlgoType, coordinateAlgo, wndImg,
//					nWndIndex, wnd_w, wnd_h, dX, dY, ptrAlignRes, inspType, sInspAlgo,
//					FALSE, -1, -1, -1, -1, nAlignCnt, nOffX_pix, nOffY_pix);
//			}
//			else
//			{
//				//Align
//				if (nAlignCnt < 2)
//				{
//					m_PInspWrapper->WndSizeChange(pImg_buf, sInspAlgo, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY,
//						ptrAlignRes, nOffX_pix, nOffY_pix);
//				}
//				else
//				{
//					m_PInspWrapper->WndSizeChange_ArrAlign(pImg_buf, sInspAlgo.m_eAlgoType, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY
//						, ptrAlignRes, inspType, sInspAlgo, nAlignCnt);
//				}
//			}
//			
//
//			
//#pragma region TestImage
//			// PIAL::PI_Buff* buf2D = pImg_buf->m_p2D->Clone();
//			// PIAL::PI_Buff* buf3D_wnd = pImg_buf->inspWndImage->m_p3D->Clone();
//			// cv::Mat m2D = buf2D->Mat();
//			// cv::Mat m3D_Wnd = buf3D_wnd->Mat();
//			// cv::Mat org3D = cv::Mat(height, width, CV_32FC1, m_pInspBoardInfo->partZmapData.data);
//			// cv::Mat org3DBuf = cv::Mat(height, width, CV_32FC1, (float*)pImg_buf->inspPartImage->m_p3D->m_pData);
//			// delete buf2D;
//			// delete buf3D_wnd;
//#pragma endregion
//			if (bUSeLeadAlign == FALSE)
//				
//			if (bUSeLeadAlign == FALSE)
//			{
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = sInspAlgo.m_eAlgoType;
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;
//			}
//			int nSideCameraIndex = m_pParamArray[nWndIndex].nInspCameraType;
//			// Input 2D Image 합성
//			RECT rtWnd;
//			rtWnd.left = dX;
//			rtWnd.top = dY;
//			rtWnd.right = dX + wnd_w - 1;
//			rtWnd.bottom = dY + wnd_h - 1;
//			
//			pImg_buf->inspWndImage->nImgSizeX = wnd_w;
//			pImg_buf->inspWndImage->nImgSizeY = wnd_h;
//
//			int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
//			int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;
//			if (width <= 0 || height <= 0)
//				return e_NG;
//			double dAlignAngle = 0;
//			UCHAR * ImgR = NULL, *ImgG = NULL, *ImgB = NULL, *ImgW = NULL;
//			LightTypeBuf sLightBuf;
//			ImgW = m_pInspBoardInfo->partImgBuf.imgTop_W;
//			int nInspCameraType = m_pParamArray[nWndIndex].nInspCameraType;
//			wndImg.m_nLight_index = nInspCameraType;
//
//			if (sInspAlgo.m_eAlgoType != (int)eAlgoPattern && sInspAlgo.m_eAlgoType != (int)eAlgoPOCR)
//			{
//				m_PInspWrapper->NormalImageCompose(m_pInspBoardInfo, sInspAlgo.m_eAlgoType, sInspAlgo, pImg_buf, pImg_buf->m_p2D, width, height, wnd_w, wnd_h, dX, dY, nSideCameraIndex, ptrAlignRes);
//				//cvResultImage = ((PIAL::PI_Buff *)pImg_buf->m_p2D)->Mat().clone();
//				m_PInspWrapper->SetWindowImage(m_pInspBoardInfo, pImg_buf, rtWnd);
//				m_PInspWrapper->ColorDataInput(sInspAlgo, pImg_buf, width, height, rtWnd, 0, nSideCameraIndex);
//				m_PInspWrapper->GetAlgoImage(&sInspAlgo, m_pInspBoardInfo, pImg_buf, rtWnd, nSideCameraIndex);
//			}
//			else
//			{
//
//				switch (sInspAlgo.m_eLightType)
//				{
//				case Top_Light:
//					ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;
//					ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;
//					ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;
//					break;
//				case Middle_Light:
//					ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
//					ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
//					break;
//				case Bottom_Light:
//					// 				if (g_pMPTI->m_nCameraType == Basler_Color)
//					// 				{
//					// 					ImgR = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_R;
//					// 					ImgB = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_G;
//					// 				}
//									/*else*/
//				{
//					ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
//					ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
//				}
//				break;
//				case Side1_Light:
//				case Side2_Light:
//				case Side3_Light:
//				case Side4_Light:
//					if (nSideCameraIndex == eMSCN_SIDECAM1)
//					{
//						ImgR = m_pInspBoardInfo->partImgBuf.imgSide1_R;
//						ImgG = m_pInspBoardInfo->partImgBuf.imgSide1_G;
//						ImgB = m_pInspBoardInfo->partImgBuf.imgSide1_B;
//					}
//					else if (nSideCameraIndex == eMSCN_SIDECAM2)
//					{
//						ImgR = m_pInspBoardInfo->partImgBuf.imgSide2_R;
//						ImgG = m_pInspBoardInfo->partImgBuf.imgSide2_G;
//						ImgB = m_pInspBoardInfo->partImgBuf.imgSide2_B;
//					}
//					else if (nSideCameraIndex == eMSCN_SIDECAM3)
//					{
//						ImgR = m_pInspBoardInfo->partImgBuf.imgSide3_R;
//						ImgG = m_pInspBoardInfo->partImgBuf.imgSide3_G;
//						ImgB = m_pInspBoardInfo->partImgBuf.imgSide3_B;
//					}
//					else if (nSideCameraIndex == eMSCN_SIDECAM4)
//					{
//						ImgR = m_pInspBoardInfo->partImgBuf.imgSide4_R;
//						ImgG = m_pInspBoardInfo->partImgBuf.imgSide4_G;
//						ImgB = m_pInspBoardInfo->partImgBuf.imgSide4_B;
//					}
//					else
//					{
//						ImgR = NULL;
//						ImgG = NULL;
//						ImgB = NULL;
//					}
//					break;
//				case  User_Light:
//					// 				if (g_pMPTI->m_nCameraType == Basler_Color)
//					// 				{
//					// 					sLightBuf.m_pucTRed = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_W;
//					// 					sLightBuf.m_pucTGreen = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgMiddle_R;
//					// 					sLightBuf.m_pucTBlue = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgMiddle_B;
//					// 					sLightBuf.m_pucTWhite = NULL;
//					// 					sLightBuf.m_pucMRed = NULL;
//					// 					sLightBuf.m_pucMGreen = NULL;
//					// 					sLightBuf.m_pucMBlue = NULL;
//					// 					sLightBuf.m_pucMWhite = NULL;
//					// 					sLightBuf.m_pucBRed = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_R;
//					// 					sLightBuf.m_pucBGreen = NULL;
//					// 					sLightBuf.m_pucBBlue = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_G;
//					// 					sLightBuf.m_pucBWhite = NULL;
//					// 				}
//					// 				else
//				{
//					sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
//					sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
//					sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
//					sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;
//
//					sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
//					sLightBuf.m_pucMGreen = NULL;
//					sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
//					sLightBuf.m_pucMWhite = NULL;
//
//					sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
//					sLightBuf.m_pucBGreen = NULL;
//					sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
//					sLightBuf.m_pucBWhite = NULL;
//				}
//
//				sLightBuf.m_nImgWidth = width;
//				sLightBuf.m_nImgHeight = height;
//				sLightBuf.m_nROIImgWidth = wnd_w;
//				sLightBuf.m_nROIImgHeight = wnd_h;
//				sLightBuf.m_dROIX = dX;
//				sLightBuf.m_dROIY = dY;
//				sLightBuf.m_nImgCnt = sInspAlgo.m_nLightCnt;
//				sLightBuf.m_pnRedValue = sInspAlgo.m_nArrRedValue;
//				sLightBuf.m_pnGreenValue = sInspAlgo.m_nArrGreenValue;
//				sLightBuf.m_pnBlueValue = sInspAlgo.m_nArrBlueValue;
//				sLightBuf.m_pnWhiteValue = sInspAlgo.m_nArrWhiteValue;
//				sLightBuf.m_pnPosition = sInspAlgo.m_nArrLightPosition;
//				sLightBuf.m_pnCalculation = sInspAlgo.m_nArrCalculation;
//				}
//
//				if (m_pInspBoardInfo->byPCBPosInspLight == 1)	// 끝단찾기 격자 합성 조명은 Top Red 에 있고 해당 이미지만 100%로 윈도우 검사 이미지 wndImg.m_ucArr2D 를 사용하기 위해서
//				{
//					sInspAlgo.m_nRedValue = 100;
//					sInspAlgo.m_nGreenValue = 0;
//					sInspAlgo.m_nBlueValue = 0;
//					sInspAlgo.m_nWhiteValue = 0;
//				}
//
//				bool bUserLight = false;
//				//double dAlignAngle = 0;
//				if (ptrAlignRes != NULL && eInspAlgoType == eAlgoBlob)
//					dAlignAngle = ptrAlignRes[0].theta;
//
//				if ((sInspAlgo.m_eLightType == Top_Light) || (sInspAlgo.m_eLightType == Middle_Light) || (sInspAlgo.m_eLightType == Bottom_Light))
//				{
//					if (WindowRotate() == false)
//					{
//						dAlignAngle = 0;
//					}
//					m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h,
//						sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, sInspAlgo.m_nWhiteValue,
//						wndImg.m_ucArr2D, dAlignAngle);
//				}
//				else if ((sInspAlgo.m_eLightType == Side1_Light) || (sInspAlgo.m_eLightType == Side2_Light) || (sInspAlgo.m_eLightType == Side3_Light) || (sInspAlgo.m_eLightType == Side4_Light))
//				{
//					if (WindowRotate() == false)
//					{
//						dAlignAngle = 0;
//					}
//					m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h,
//						sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, 0,
//						wndImg.m_ucArr2D, dAlignAngle);
//				}
//				else
//				{
//					bUserLight = true;
//					if (WindowRotate() == false)
//					{
//						dAlignAngle = 0;
//					}
//					m_InspAlgo.ROIImageClaculCompose(sLightBuf, wndImg.m_ucArr2D, dAlignAngle);
//				}
//			}
//			//InspRoiImgBuf *sInspImgBuf = new InspRoiImgBuf;
//			InspRoiImgBuf *sInspImgBuf = g_pMManager->pem_new<InspRoiImgBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//			InspRoiImgBuf* sArrInspImgBuf = NULL;
//			sInspImgBuf->imgTop_R = NULL;
//			sInspImgBuf->imgTop_G = NULL;
//			sInspImgBuf->imgTop_B = NULL;
//			sInspImgBuf->imgTop_W = NULL;
//			sInspImgBuf->imgMiddle_R = NULL;
//			sInspImgBuf->imgMiddle_B = NULL;
//			sInspImgBuf->imgBottom_R = NULL;
//			sInspImgBuf->imgBottom_B = NULL;
//
//			//if (bNewInspAlgo == false)
//				ColorDataInput(sInspAlgo, width, height, dX, dY, wnd_w, wnd_h, sInspImgBuf, 0, nInspCameraType);
//			if (eInspAlgoType == eAlgoAlignEdge)
//			{
//				//sArrInspImgBuf = new InspRoiImgBuf[ALIGNEDGE_AREA_CNTS];
//				sArrInspImgBuf = g_pMManager->pem_new<InspRoiImgBuf>(true, ALIGNEDGE_AREA_CNTS, (PCHAR)__FUNCTION__, __LINE__);
//				for (int a = 0; a < ALIGNEDGE_AREA_CNTS; a++)
//				{
//					sArrInspImgBuf[a].imgTop_R = NULL;
//					sArrInspImgBuf[a].imgTop_G = NULL;
//					sArrInspImgBuf[a].imgTop_B = NULL;
//					sArrInspImgBuf[a].imgTop_W = NULL;
//					sArrInspImgBuf[a].imgMiddle_R = NULL;
//					sArrInspImgBuf[a].imgMiddle_B = NULL;
//					sArrInspImgBuf[a].imgBottom_R = NULL;
//					sArrInspImgBuf[a].imgBottom_B = NULL;
//					ColorDataInput(sInspAlgo, width, height, dX, dY, wnd_w, wnd_h, &sArrInspImgBuf[a], a);
//				}
//			}
//			// 통합 검사/제외 영역 생성
//			PIAL::_TotalInspExceptArea stTieArea;
//			stTieArea.m_nUsedWndPolygon = m_pParamArray[nWndIndex].m_nUsedWndPolygon;
//			for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
//				stTieArea.m_ptArrWndPolygon[i] = m_pParamArray[nWndIndex].m_ptArrWndPolygon[i];
//
//			
//			if ((eInspAlgoType == eAlgoPattern))
//			{
//				CPInsp_Pattern *pPatternInsp = (CPInsp_Pattern *)GetPatternInsp();
//
//				UCHAR* ImgR_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_R;
//				UCHAR* ImgG_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_G;
//				UCHAR* ImgB_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_B;
//				pPatternInsp->SetInspParam(sInspAlgo, wndImg, coordinateAlgo, width, height, _stTieArea);
//
//				//cv::Mat src(height,width, CV_MAKETYPE(CV_8U,3));
//				if (dX < 0) dX = 0;
//				if (dY < 0) dY = 0;
//
//				for (int y = dY; y < dY + wnd_h; y++)
//				{
//					for (int x = dX; x < dX + wnd_w; x++)
//					{
//						int sizeY = y - dY;
//						int sizeX = x - dX;
//						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 0] = ImgB_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_blue < 255 ? ImgB_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_blue : 255;
//						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 1] = ImgG_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_green < 255 ? ImgG_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_green : 255;
//						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 2] = ImgR_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_red < 255 ? ImgR_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_red : 255;
//
//					}
//				}
//				//cv::Mat cripsrc;
//
//				//cv::Rect rect(dX, dY, wnd_w, wnd_h);
//				/*cripsrc = src(rect);
//
//				for(int y=0; y< wnd_h ; y++)
//				{
//					memcpy(&wndImg.m_ucArrCV[y*(wnd_w * 3)], &cripsrc.data[y*cripsrc.step], wnd_w * 3);
//				}*/
//			}
//			//LJH 2016.05.26 일단 ocr의 경우 최대 3장 (기존 한장 + 두장)의 이미지를 취합 할 수 있도록 설정
//			if (eInspAlgoType == eAlgoOCR || eInspAlgoType == eAlgoPOCR || eInspAlgoType == eAlgoAlignEdge || eInspAlgoType == eAlgoTab || eInspAlgoType == eAlgoHeight_Diff)
//			{
//				//qtm.StartTick();
//				for (int iLoopCount = 0; iLoopCount < sInspAlgo.m_nMixCount; ++iLoopCount)
//				{
//					switch (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType)
//					{
//					case Top_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;		ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;		ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;	ImgW = m_pInspBoardInfo->partImgBuf.imgTop_W;	break;	}
//					case Middle_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;	break;	}
//					case Bottom_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;	break;	}
//					case Side1_Light:
//					case Side2_Light:
//					case Side3_Light:
//					case Side4_Light:
//						if (nInspCameraType == eMSCN_SIDECAM1)
//						{
//							ImgR = m_pInspBoardInfo->partImgBuf.imgSide1_R;
//							ImgG = m_pInspBoardInfo->partImgBuf.imgSide1_G;
//							ImgB = m_pInspBoardInfo->partImgBuf.imgSide1_B;
//						}
//						else if (nInspCameraType == eMSCN_SIDECAM2)
//						{
//							ImgR = m_pInspBoardInfo->partImgBuf.imgSide2_R;
//							ImgG = m_pInspBoardInfo->partImgBuf.imgSide2_G;
//							ImgB = m_pInspBoardInfo->partImgBuf.imgSide2_B;
//						}
//						else if (nInspCameraType == eMSCN_SIDECAM3)
//						{
//							ImgR = m_pInspBoardInfo->partImgBuf.imgSide3_R;
//							ImgG = m_pInspBoardInfo->partImgBuf.imgSide3_G;
//							ImgB = m_pInspBoardInfo->partImgBuf.imgSide3_B;
//						}
//						else if (nInspCameraType == eMSCN_SIDECAM4)
//						{
//							ImgR = m_pInspBoardInfo->partImgBuf.imgSide4_R;
//							ImgG = m_pInspBoardInfo->partImgBuf.imgSide4_G;
//							ImgB = m_pInspBoardInfo->partImgBuf.imgSide4_B;
//						}
//
//						break;
//					case  User_Light:
//						sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
//						sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
//						sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
//						sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;
//
//						sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
//						sLightBuf.m_pucMGreen = NULL;
//						sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
//						sLightBuf.m_pucMWhite = NULL;
//
//						sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
//						sLightBuf.m_pucBGreen = NULL;
//						sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
//						sLightBuf.m_pucBWhite = NULL;
//
//						sLightBuf.m_nImgWidth = width;
//						sLightBuf.m_nImgHeight = height;
//						sLightBuf.m_nROIImgWidth = wnd_w;
//						sLightBuf.m_nROIImgHeight = wnd_h;
//						sLightBuf.m_dROIX = dX;
//						sLightBuf.m_dROIY = dY;
//						sLightBuf.m_nImgCnt = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nLightCnt;
//						sLightBuf.m_pnRedValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrRedValue;
//						sLightBuf.m_pnGreenValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrGreenValue;
//						sLightBuf.m_pnBlueValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrBlueValue;
//						sLightBuf.m_pnWhiteValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrWhiteValue;
//						sLightBuf.m_pnPosition = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrLightPosition;
//						sLightBuf.m_pnCalculation = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrCalculation;
//					}
//
//					if ((sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Top_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Middle_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Bottom_Light))
//					{
//						m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nRedValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nGreenValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nBlueValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nWhiteValue, wndImg.m_ucArr2D_Mix[iLoopCount]);
//						m_procMil->SaveWorkImg(ImgR, width, height, _T("Single2D Red_Mix.bmp"));
//						m_procMil->SaveWorkImg(ImgW, width, height, _T("Single2D White_Mix.bmp"));
//						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Compose_Mix.bmp"));
//					}
//					else if ((sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side1_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side2_Light) ||
//						(sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side3_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side4_Light))
//					{
//						m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nRedValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nGreenValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nBlueValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nWhiteValue, wndImg.m_ucArr2D_Mix[iLoopCount]);
//						m_procMil->SaveWorkImg(ImgR, width, height, _T("Side_2D Red_Mix.bmp"));
//						m_procMil->SaveWorkImg(ImgW, width, height, _T("Side_2D White_Mix.bmp"));
//						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Side_Compose_Mix.bmp"));
//					}
//					else
//					{
//						m_InspAlgo.ROIImageClaculCompose(sLightBuf, wndImg.m_ucArr2D_Mix[iLoopCount]);
//						m_procMil->SaveWorkImg(sLightBuf.m_pucTRed, width, height, _T("Top Red_Mix.bmp"));
//						m_procMil->SaveWorkImg(sLightBuf.m_pucMRed, width, height, _T("Mid Red_Mix.bmp"));
//						m_procMil->SaveWorkImg(sLightBuf.m_pucBRed, width, height, _T("Bot Red_Mix.bmp"));
//						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Compose_User_Mix.bmp"));
//					}
//				}
//
//				//m_AlgoDt[_T("OCR_Compose")] = (qtm.EndTick() * 1000.0) + m_AlgoDt[_T("OCR_Compose")];
//			}
//
//			if (eInspAlgoType == eAlgoGrid || eInspAlgoType == eAlgoBlob)
//			{
//				int iLoopCount = 0;
//
//				switch (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType)
//				{
//				case Top_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;		ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;		ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;	ImgW = m_pInspBoardInfo->partImgBuf.imgTop_W;	break;	}
//				case Middle_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;	break;	}
//				case Bottom_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;	break;	}
//				case  User_Light:
//					sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
//					sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
//					sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
//					sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;
//
//					sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
//					sLightBuf.m_pucMGreen = NULL;
//					sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
//					sLightBuf.m_pucMWhite = NULL;
//
//					sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
//					sLightBuf.m_pucBGreen = NULL;
//					sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
//					sLightBuf.m_pucBWhite = NULL;
//
//					sLightBuf.m_nImgWidth = width;
//					sLightBuf.m_nImgHeight = height;
//					sLightBuf.m_nROIImgWidth = wnd_w;
//					sLightBuf.m_nROIImgHeight = wnd_h;
//					sLightBuf.m_dROIX = dX;
//					sLightBuf.m_dROIY = dY;
//					sLightBuf.m_nImgCnt = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nLightCnt;
//					sLightBuf.m_pnRedValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrRedValue;
//					sLightBuf.m_pnGreenValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrGreenValue;
//					sLightBuf.m_pnBlueValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrBlueValue;
//					sLightBuf.m_pnWhiteValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrWhiteValue;
//					sLightBuf.m_pnPosition = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrLightPosition;
//					sLightBuf.m_pnCalculation = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrCalculation;
//				}
//
//				if ((sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Top_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Middle_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Bottom_Light))
//				{
//					if (WindowRotate() == false)
//					{
//						dAlignAngle = 0;
//					}
//					m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nRedValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nGreenValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nBlueValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nWhiteValue, wndImg.m_ucArr2D_Mix[iLoopCount], dAlignAngle);
//					m_procMil->SaveWorkImg(ImgR, width, height, _T("Grid Single2D Red_Mix.bmp"));
//					m_procMil->SaveWorkImg(ImgW, width, height, _T("Grid Single2D White_Mix.bmp"));
//					m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Grid Compose_Mix.bmp"));
//				}
//				else
//				{
//					if (WindowRotate() == false)
//					{
//						dAlignAngle = 0;
//					}
//					m_InspAlgo.ROIImageClaculCompose(sLightBuf, wndImg.m_ucArr2D_Mix[iLoopCount], dAlignAngle);
//					m_procMil->SaveWorkImg(sLightBuf.m_pucTRed, width, height, _T("Grid Top Red_Mix.bmp"));
//					m_procMil->SaveWorkImg(sLightBuf.m_pucMRed, width, height, _T("Grid Mid Red_Mix.bmp"));
//					m_procMil->SaveWorkImg(sLightBuf.m_pucBRed, width, height, _T("Bot Red_Mix.bmp"));
//					m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Grid Compose_User_Mix.bmp"));
//				}
//			}
//			AlgoColorOpt ptrAlgoColorOpt;
//			if (eInspAlgoType == eAlgoFoot || eInspAlgoType == eAlgoWire)
//			{
//				ptrAlgoColorOpt.m_sFovImg.imgTop_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_R;
//				ptrAlgoColorOpt.m_sFovImg.imgTop_G = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_G;
//				ptrAlgoColorOpt.m_sFovImg.imgTop_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_B;
//				ptrAlgoColorOpt.m_sFovImg.imgTop_W = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_W;
//
//				ptrAlgoColorOpt.m_sFovImg.imgMiddle_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgMiddle_R;
//				ptrAlgoColorOpt.m_sFovImg.imgMiddle_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgMiddle_B;
//
//				ptrAlgoColorOpt.m_sFovImg.imgBottom_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgBottom_R;
//				ptrAlgoColorOpt.m_sFovImg.imgBottom_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgBottom_B;
//
//				ptrAlgoColorOpt.m_sImgBuf.nImageSizeX = 0;
//				ptrAlgoColorOpt.m_sImgBuf.nImageSizeY = 0;
//
//				ptrAlgoColorOpt.m_rcImageRect.left = dX;
//				ptrAlgoColorOpt.m_rcImageRect.top = dY;
//				ptrAlgoColorOpt.m_rcImageRect.right = dX + wnd_w;
//				ptrAlgoColorOpt.m_rcImageRect.bottom = dY + wnd_h;
//				ptrAlgoColorOpt.m_sFovImg.nImageSizeX = width;
//				ptrAlgoColorOpt.m_sFovImg.nImageSizeY = height;
//			}
//
//			nLine = __LINE__;
//			BOOL bAlgoOK = FALSE;
//			if (bUSeLeadAlign == FALSE)
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;
//			if (CheckAlgoGroup(inspType, nWndIndex, nAlgo, WndResult) == false)
//				eInspAlgoType = eAlgoNum;
//
//			nLine = __LINE__;
//			bool IsUseMultiArea = false;
//			if (eInspAlgoType == eAlgoTab || eInspAlgoType == eAlgoColor || eInspAlgoType == eAlgoLead_Color)
//			{
//				AlgoColor* pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
//				if (pAlgoColor && pAlgoColor->IsUseMultiArea())
//				{
//					IsUseMultiArea = true;
//					int nCntTabROI = rcArrTab.GetCount();
//					if (pAlgoColor->m_bAutoSearchROI && nCntTabROI > 0)
//					{
//						for (int i = 0; i < pAlgoColor->m_nCntRect; ++i)
//						{
//							CRect rcTab(pAlgoColor->m_rcArrTabRect[i]);
//							CPoint xyCenOrg = rcTab.CenterPoint();
//							int nIdxNear = -1;
//							int nDistanceMin = 0;
//							for (int nROI = 0; nROI < nCntTabROI; ++nROI)
//							{
//								CPoint xyOffset = rcArrTab[nROI].CenterPoint();
//								xyOffset -= xyCenOrg;
//								int nDistance = pow((double)xyOffset.y, 2);
//								if ((pAlgoColor->m_nLeadTipDirection == e_TOP) || (pAlgoColor->m_nLeadTipDirection == e_BOTTOM))
//									nDistance = pow((double)xyOffset.x, 2);
//								if ((nIdxNear == -1) || (nDistanceMin > nDistance))
//								{
//									nIdxNear = nROI;
//									nDistanceMin = nDistance;
//								}
//							}
//							if (nIdxNear < 0) continue;
//							CPoint xyOffset = rcArrTab[nIdxNear].CenterPoint() - xyCenOrg;
//							rcTab.OffsetRect(xyOffset);
//							pAlgoColor->m_rcArrTabRect[i] = rcTab;
//						}
//					}
//				}
//			}
//
//			CAlgoFactory cloneFactory;
//			void * vRstInspAlgoTemp = nullptr;
//			if (bUSeLeadAlign == TRUE && (eInspAlgoType == eAlgoAlign || eInspAlgoType == eAlgoAlignEdge))
//				cloneFactory.CreateRstAlgoClone(eInspAlgoType, vRstInspAlgoTemp, FALSE);
//			else
//			{
//				cloneFactory.CreateRstAlgoClone(eInspAlgoType, WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, FALSE);
//			}
//// 			if (CheckAlgoGroup(inspType, nWndIndex, nAlgo, WndResult) == false)
//// 				eInspAlgoType = eAlgoNum;
//
//			
//
//			switch (eInspAlgoType)
//			{
//				case eAlgoBW:
//				{
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBlackWhite;
//
//					PIAL::_RstAlgoBW* rstBW = new PIAL::_RstAlgoBW();
//					PIAL::_AlgoBW algoBW;
//					int nWidth = pImg_buf->inspWndImage->nImgSizeX;
//					int nHeight = pImg_buf->inspWndImage->nImgSizeY;
//					float nAngle = pImg_buf->inspPartImage->m_Angle;
//
//					POINTF ptfCenter;
//					AlgoBW* pAlgoBW = (AlgoBW *)sInspAlgo.m_ptrInspAlgoParam;
// 					m_PInspWrapper->ConvertAlgo(pAlgoBW, algoBW);
// 
// 					ptfCenter.x = (poBodyCenter.x >= 0) ? poBodyCenter.x : nWidth / 2;
// 					ptfCenter.y = (poBodyCenter.y >= 0) ? poBodyCenter.y : nHeight / 2;
// 					
// 					m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, nAngle);					
// 
// 					PIAL::_AlgoBlackWhite pInspAlgoBW2;
// 					m_PInspWrapper->ConvertAlgo(pAlgoBW, &pInspAlgoBW2);
// 
//					PIAL::PI_Buff* Clipped3D = pImg_buf->inspWndImage->m_p3D->Clone();
// 					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBW(&algoBW, pInspAlgoBW2, pImg_buf->m_p2D, Clipped3D, stTieArea, rstBW, ptfCenter, pImg_buf);
//					rstBW->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].left += dX;
//					rstBW->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].right += dX;
//					rstBW->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].top += dY;
//					rstBW->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ].bottom += dY;
//
//					m_PInspWrapper->ConvertRstAlgo(*rstBW, (RstAlgoBlackWhite*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//
//					delete rstBW;
//					delete Clipped3D;
//				}
//				break;
//				case eAlgoBlob:  //이전 결과 사용
//				{
//					nLine = __LINE__;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBlob;
//					AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
//
//					//Color
//				
//					PIAL::_TotalInspExceptArea stTieArea2;
//
//					//Solder ball
//					PIAL::_AlgoBlob algo2;
//					PIAL::_RstAlgoBlob rstAlgo2;
//					m_PInspWrapper->ConvertAlgo(pAlgoBlob, algo2);
//
//					PIAL::Insp_ROIImg sInspImageData;
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBlob(algo2, *pImg_buf, sInspImageData, &rstAlgo2, stTieArea2 ,nullptr);
//
//					rstAlgo2.m_rcRect_I.left += dX;
//					rstAlgo2.m_rcRect_I.right += dX;
//					rstAlgo2.m_rcRect_I.top += dY;
//					rstAlgo2.m_rcRect_I.bottom += dY;
//					rstAlgo2.m_poDrawCenter.x += dX;
//					rstAlgo2.m_poDrawCenter.y += dY;
//
//					if (rstAlgo2.m_nArrRectCnt > 200)
//						rstAlgo2.m_nArrRectCnt = 200;
//
//					for (int i = 0; i < rstAlgo2.m_nArrRectCnt; ++i)
//					{
//						rstAlgo2.m_rcArrRect[i].left += dX;
//						rstAlgo2.m_rcArrRect[i].right += dX;
//						rstAlgo2.m_rcArrRect[i].top += dY;
//						rstAlgo2.m_rcArrRect[i].bottom += dY;
//					}
//					m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoBlob*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					
//					if (bAlgoOK && !bAlignResOK&&(inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
//					{
//						RstAlgoBlob * rst = (RstAlgoBlob *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
//						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
//						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
//						arrAlignRes[nAlignWndIdx].offsetX = -rst->m_dRstShiftX;
//						arrAlignRes[nAlignWndIdx].offsetY = rst->m_dRstShiftY;
//						arrAlignRes[nAlignWndIdx].theta = 0;
//						ptrAlignRes[nCurAlignID] = arrAlignRes[nAlignWndIdx];
//						bAlignResOK = TRUE;
//					}
//				}
//				break;
//				case eAlgoAlign:
//				{
//					AlgoAlign *pAlgo = (AlgoAlign *)sInspAlgo.m_ptrInspAlgoParam;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoAlign;
//
//					PIAL::_AlgoAlign _algo;
//					PIAL::_RstAlgoAlign _rst;
//					m_PInspWrapper->ConvertAlgo(pAlgo, _algo);
//					m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
//
//					CRect rcBlobBody;
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspAlign(_algo, *pImg_buf, rcBlobBody, &_rst, vecAlignResult);
//
//					m_PInspWrapper->ConvertRstAlgo(_rst, (RstAlgoAlign*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					if (bAlgoOK && !bAlignResOK)
//					{
//						RstAlgoAlign * rst = (RstAlgoAlign *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//						if (arrAlignRes[nAlignWndIdx].nWindowID != -1)
//						{
//							int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
//							for (int a = 0; a < nSize; a++)
//							{
//								if (ptrAlignRes[a].nWindowID == -1);
//								{
//									nAlignWndIdx = a;
//									break;
//								}
//							}
//						}
//						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
//						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;//coordinateAlgo.dROICenterX;
//						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;//coordinateAlgo.dROICenterY;
//						arrAlignRes[nAlignWndIdx].offsetX = -rst->m_dOffset_x;
//						arrAlignRes[nAlignWndIdx].offsetY = rst->m_dOffset_y;
//						arrAlignRes[nAlignWndIdx].theta = rst->m_dTheta;
//						arrAlignRes[nAlignWndIdx].rcBodyRect = rcBlobBody;
//						ptrAlignRes[nCurAlignID] = arrAlignRes[nAlignWndIdx];
//						bAlignResOK = TRUE;
//					}
//				}
//				break;
//				case eAlgoBody_Blob:
//				{
//					nLine = __LINE__;
//					AlgoBodyBlob *pAlgoBodyBlob = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
//					//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBodyBlob;
//
//					PIAL::_AlgoBodyBlob algoBodyBlob;
//					PIAL::_RstAlgoBodyBlob rstBodyBlob;
//
//					m_PInspWrapper->ConvertAlgo(pAlgoBodyBlob, algoBodyBlob);
//					m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
//					m_PInspWrapper->ConvertExceptROI(&stTieArea, m_pInspBoardInfo, m_pParamArray[nWndIndex], InspData);
//
//					CRect rcBlobBody;
//					POINTF ptWndCenter;
//					ptWndCenter.x = m_pParamArray[nWndIndex].cx;
//					ptWndCenter.y = m_pParamArray[nWndIndex].cy;
//
//					PIAL::BodyInfo* pBody = nullptr;
//					if (InspData)
//						pBody = new PIAL::BodyInfo;
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBodyBlob(algoBodyBlob, *pImg_buf, rcBlobBody, &rstBodyBlob, ptWndCenter, stTieArea, vecAlignResult, nullptr, pBody);
//
//					if (pBody)
//					{	
//						pBody->BodyStartPos = new POINT();
//						pBody->BodySize = new SIZE();
//						pBody->BodySize->cx = rcBlobBody.Width() + (algoBodyBlob.PassiveMargin / PIAL::PInspAlgo_Lib::m_resolX * 2);
//						pBody->BodySize->cy = rcBlobBody.Height() + (algoBodyBlob.PassiveMargin / PIAL::PInspAlgo_Lib::m_resolY * 2);
//						pBody->BodyStartPos->x = rcBlobBody.left - algoBodyBlob.PassiveMargin / PIAL::PInspAlgo_Lib::m_resolX;
//						pBody->BodyStartPos->y = rcBlobBody.top - algoBodyBlob.PassiveMargin / PIAL::PInspAlgo_Lib::m_resolY;
//
//						if (algoBodyBlob.Passive)
//						{
//							pBody->Passive = true;
//							pBody->BodyStartPos->x += dX;
//							pBody->BodyStartPos->y += dY;
//
//						}
//					}
//
//					if (algoBodyBlob.Passive)
//					{
//						rstBodyBlob.m_rcBodyRect.left += dX;
//						rstBodyBlob.m_rcBodyRect.right += dX;
//						rstBodyBlob.m_rcBodyRect.top += dY;
//						rstBodyBlob.m_rcBodyRect.bottom += dY;
//
//						rstBodyBlob.m_rcInspBodyRect.right += dX;
//						rstBodyBlob.m_rcInspBodyRect.left += dX;
//						rstBodyBlob.m_rcInspBodyRect.top += dY;
//						rstBodyBlob.m_rcInspBodyRect.bottom += dY;
//						// pRstAlgo.m_rcRect_T
//						// pRstAlgo.m_rcBodyDamageRect
//					}
//
//					m_PInspWrapper->ConvertRstAlgo(rstBodyBlob, (RstAlgoBodyBlob*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//
//					if (InspData && pBody)
//						InspData->SetBodyData(pBody);
//					
//					if (((bAlgoOK && pAlgoBodyBlob->m_bNGOffset == FALSE) || pAlgoBodyBlob->m_bNGOffset == TRUE) && !bAlignResOK && inspType == eINSP_MOUNT)
//					{
//						RstAlgoBodyBlob * rst = (RstAlgoBodyBlob *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
//						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
//						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
//						arrAlignRes[nAlignWndIdx].offsetX = -rst->m_dRstShiftX;
//						arrAlignRes[nAlignWndIdx].offsetY = rst->m_dRstShiftY;
//						arrAlignRes[nAlignWndIdx].theta = rst->m_dRstAngle - pAlgoBodyBlob->m_dStandardRotate;
//						arrAlignRes[nAlignWndIdx].rcBodyRect = rcBlobBody;
//						ptrAlignRes[nCurAlignID] = arrAlignRes[nAlignWndIdx];		
//						bAlignResOK = TRUE;
//					}
//					/*if (pBody)
//						delete pBody;*/
//				}
//				break;
//				case eAlgoColor:
//				{
//					nLine = __LINE__;
//					AlgoColor* pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
//
//					//Color
//					PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg; // ÀÓ½Ã
//					PIAL::_AlgoColor AlgoColor;
//					
//					// 조명 가져오기.
//
//					// PIAL::_RstAlgoColorXY ResultColorXY; // if구문
//					PIAL::_RstAlgoColor ResultColor;        // else if구문
//					// PIAL::_RstAlgoTab ResultTab;         // else구문
//					
//					// m_PInspWrapper->AlgoImageCompose(m_pInspBoardInfo, sInspAlgo.m_eAlgoType, sInspAlgo, pWnd_buf->m_P2D->m_pData, width, height, wnd_w, wnd_h, dX, dY);
//
//
//					m_PInspWrapper->ConvertAlgo(pAlgoColor, AlgoColor);
//					if (sInspAlgo.m_eAlgoType == eAlgoColorXY)
//					{
//						TRACE(_T("sInspAlgo.m_eAlgoType == eAlgoColorXY"));
//
//						// WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoColorXY>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//					}
//					else if (pAlgoColor && !pAlgoColor->IsUseMultiArea())
//					{
//						TRACE(_T("pAlgoColor && !pAlgoColor->IsUseMultiArea()"));
//
//						// WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoColor>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//					}
//					else
//					{
//						if (pAlgoColor && pAlgoColor->m_bAutoSearchROI)
//						{
//							int nCntTabROI = rcArrTab.GetCount();
//							if (nCntTabROI > 0)
//							{
//								for (int i = 0; i < pAlgoColor->m_nCntRect; ++i)
//								{
//									CRect rcTab(pAlgoColor->m_rcArrTabRect[i]);
//
//									CPoint xyCenOrg = rcTab.CenterPoint();
//									int nIdxNear = -1;
//									int nDistanceMin = 0;
//									for (int nROI = 0; nROI < nCntTabROI; ++nROI)
//									{
//										CPoint xyOffset = rcArrTab[nROI].CenterPoint();
//										xyOffset -= xyCenOrg;
//										int nDistance = 0;
//										if ((pAlgoColor->m_nLeadTipDirection == e_TOP) || (pAlgoColor->m_nLeadTipDirection == e_BOTTOM))
//											nDistance = pow((double)xyOffset.x, 2);
//										else
//											nDistance = pow((double)xyOffset.y, 2);
//
//										if ((nIdxNear == -1)
//											|| (nDistanceMin > nDistance))
//										{
//											nIdxNear = nROI;
//											nDistanceMin = nDistance;
//										}
//									}
//
//									if (nIdxNear < 0)
//										continue;
//
//									CPoint xyOffset = rcArrTab[nIdxNear].CenterPoint() - xyCenOrg;
//									rcTab.OffsetRect(xyOffset);
//									pAlgoColor->m_rcArrTabRect[i] = rcTab;
//								}
//							}
//						}
//						// ydh
//						// WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoTab>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//						//
//					}
//
//					// ydh
//					// RECT* rcLeadInsp = g_pMManager->pem_new<RECT>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
//					// 
//					RECT* rcLeadInsp;
//					rcLeadInsp = new RECT();
//					rcLeadInsp->left = 0;
//					rcLeadInsp->right = 0;
//					rcLeadInsp->top = 0;
//					rcLeadInsp->bottom = 0;
//					
//					// R,G,B 조명 각각 가져와야함.
//					PIAL::PI_Buff* bufTR = nullptr;
//					PIAL::PI_Buff* bufTG = nullptr;
//					PIAL::PI_Buff* bufTB = nullptr;
//					PIAL::PI_Buff* bufBR = nullptr;
//					PIAL::PI_Buff* bufBB = nullptr;
//
//					if (pAlgoColor->m_bUseColorMap2)
//					{
//						bufTR = pImg_buf->m_AngleColorImageBuffer->GetBuff(PIAL::Top_R);
//						bufTG = pImg_buf->m_AngleColorImageBuffer->GetBuff(PIAL::Top_G);;
//						bufTB = pImg_buf->m_AngleColorImageBuffer->GetBuff(PIAL::Top_B);
//					}
//					else
//					{
//						bufTR = pImg_buf->inspPartImage->m_ImageBuffer->GetBuff(PIAL::Top_R);
//						bufTG = pImg_buf->inspPartImage->m_ImageBuffer->GetBuff(PIAL::Top_G);
//						bufTB = pImg_buf->inspPartImage->m_ImageBuffer->GetBuff(PIAL::Top_B);
//						if (m_nCompositeLightMode == 0)
//						{
//							bufBR = pImg_buf->inspPartImage->m_ImageBuffer->GetBuff(PIAL::Bottom_R);
//							bufBB = pImg_buf->inspPartImage->m_ImageBuffer->GetBuff(PIAL::Bottom_B);
//						}
//					}
//
//					// ydh
//					// 임시막음
//					// m_pColorInsp->SetInspParam(sInspAlgo, wndImg, coordinateAlgo, m_pInspBoardInfo, nLeadTipPos, rcLeadInsp, nInspCameraType, sInspImgBuf);
//					// 
//					// 
//					// for (int a = 0; a < 3; a++)
//					// {
//					// 	m_pColorInsp->m_sLightData[a].m_byLightCnt = m_sLightData[a].m_byLightCnt;
//					// 	for (int b = 0; b < LIGHT_DATA_CNT; b++)
//					// 	{
//					// 		for (int c = 0; c < LIGHT_CNT; c++)
//					// 			m_pColorInsp->m_sLightData[a].m_byArrLightData[b][c] = m_sLightData[a].m_byArrLightData[b][c];
//					// 	}
//					// }
//
//					nLine = __LINE__;
//					if (sInspAlgo.m_eAlgoType == eAlgoColorXY)
//					{
//						// ydh 임시막음.
//						// bAlgoOK = m_pColorInsp->InspColorXY(sInspAlgo, coordinateAlgo, m_pInspBoardInfo, (RstAlgoColorXY *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, m_pParamArray[nWndIndex].WndInspType);
//					}
//					else
//					{
//
//						// 신규작업
//						// m_PInspWrapper->m_PInspAlgo->InspProc();
//						// Arguments  =>   *pWnd_buf, &rstAlgo2, pPreImgDst
//
//						m_PInspWrapper->m_PInspAlgo->InspColorStep(AlgoColor, *pImg_buf, &ResultColor, bufTR, bufTG, bufTB, bufBR, bufBB, 0);  // YJS 2016/11/04
//
//						nLine = __LINE__;
//						rcLeadInsp->left += dX;
//						rcLeadInsp->right += dX;
//						rcLeadInsp->top += dY;
//						rcLeadInsp->bottom += dY;
//						// m_pColorInsp->GetInspRst(WndResult->m_vArrRstInspAlgo[nAlgo], *rcLeadInsp);
//
//						// ydh 임시막음.
//						// if (pAlgoColor && pAlgoColor->IsUseMultiArea())
//						//		TabNGRectChangePosition(WndResult, nAlgo, dX, dY);
//
//
//						// 제거예정
//						// bAlgoOK = WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk;
//					}
//
//					delete rcLeadInsp;
//					delete pwnd_buf_ROI;
//					nLine = __LINE__;
//					break;
//				}
//				case eAlgoFoot: 
//				{
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoFoot;
//
//					nLine = __LINE__;
//					AlgoFoot *pAlgoFoot = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;
//
//					//Color
//					PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
//
//					PIAL::_TotalInspExceptArea stTieArea2;
//
//					//Solder ball
//					PIAL::_AlgoFoot algo2;
//					PIAL::_RstAlgoFoot rstAlgo2;
//					m_PInspWrapper->ConvertAlgo(pAlgoFoot, algo2);
//
//					//////////////////////
//					m_PInspWrapper->MakeROIImg(&m_pInspBoardInfo->partImgBuf, pwnd_buf_ROI);
//
//					RECT m_rcImageRect;
//					m_rcImageRect.left = dX;
//					m_rcImageRect.top = dY;
//					m_rcImageRect.right = dX + wnd_w;
//					m_rcImageRect.bottom = dY + wnd_h;
//
//					pwnd_buf_ROI->m_rcSubRect = m_rcImageRect;	//알고리즘 사이즈
//					PIAL::PInspAlgo_WireBonding* PInspWireBonding = m_PInspWrapper->m_PInspWire;
//
//					int nPartWidth = pImg_buf->inspPartImage->nImgSizeX;
//					int nPartHeight = pImg_buf->inspPartImage->nImgSizeY;
//
//					PIAL::Foot_Model* pFoot = m_PInspWrapper->m_PInspWire->MakeFoot(&algo2, *pImg_buf, dX, dY, *pwnd_buf_ROI);
//					if (pFoot == NULL)
//					{
//						bAlgoOK = FALSE;
//						break;
//					}
//
//					//m_PInspWrapper->m_PInspWire->InspFoot(pFoot, &rstAlgo2, stTieArea, *pwnd_buf_ROI, -1, NULL, /*ptrAlignRes,*/ false);
//					m_PInspWrapper->m_PInspWire->InspFoot(pFoot, (PIAL::_RstAlgoFoot*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, stTieArea, *pwnd_buf_ROI, -1, NULL, /*ptrAlignRes,*/ false);
//
//					delete pwnd_buf_ROI;
//					/////////////////////
//
//					
//					if (pFoot)
//					{
//						delete  pFoot;
//						//g_pMManager->pem_delete(pFoot, false);
//						pFoot = NULL;
//					}
//				}
//
//				break;
//				case eAlgoWire:
//				{
//					nLine = __LINE__;
//					AlgoWire*pAlgoWire = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
//
//					//Color
//					PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
//
//					PIAL::_AlgoWire algo2;
//					PIAL::_RstAlgoWire rstAlgo2;
//					m_PInspWrapper->ConvertAlgo(pAlgoWire, algo2);
//
//					//////////////////////
//					m_PInspWrapper->MakeROIImg(&m_pInspBoardInfo->partImgBuf, pwnd_buf_ROI);
//
//					RECT m_rcImageRect;
//					m_rcImageRect.left = dX;
//					m_rcImageRect.top = dY;
//					m_rcImageRect.right = dX + wnd_w;
//					m_rcImageRect.bottom = dY + wnd_h;
//
//					pwnd_buf_ROI->m_rcSubRect = m_rcImageRect;	//알고리즘 사이즈
//					PIAL::PInspAlgo_WireBonding* PInspWireBonding = m_PInspWrapper->m_PInspWire;
//
//					int nPartWidth = pImg_buf->inspPartImage->nImgSizeX;
//					int nPartHeight = pImg_buf->inspPartImage->nImgSizeY;
//
//					PIAL::Wire_Model* pModel = m_PInspWrapper->m_PInspWire->MakeWire(algo2, *pImg_buf, dX, dY, *pwnd_buf_ROI);
//					if (pModel == NULL)
//					{
//						bAlgoOK = FALSE;
//						break;
//					}
//
//					PIAL::PI_Buff partimage(m_puImgWireInterval, nPartWidth, nPartHeight, nPartWidth);
//
//					POINTF poArrInspFoot[2];
//					AlgoWire *pAlgo = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
//					poArrInspFoot[0].x = pAlgo->m_sArrPoint[0].x + (pImg_buf->inspWndImage->nImgSizeX / 2.);
//					poArrInspFoot[0].y = pAlgo->m_sArrPoint[0].y + (pImg_buf->inspWndImage->nImgSizeY / 2.);
//					poArrInspFoot[1].x = pAlgo->m_sArrPoint[1].x + (pImg_buf->inspWndImage->nImgSizeX / 2.);;
//					poArrInspFoot[1].y = pAlgo->m_sArrPoint[1].y + (pImg_buf->inspWndImage->nImgSizeY / 2.);
//
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoWire;
//
//					m_PInspWrapper->m_PInspWire->InspWire(pModel, &rstAlgo2, poArrInspFoot,-1,&partimage);
//
//					m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoWire *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//
//					delete pwnd_buf_ROI;
//
//					if (pModel)
//					{
//						delete  pModel;
//						pModel = NULL;
//					}
//
//				}
//				break;
//				case eAlgoTilt:
//				{
//					PIAL::_RstAlgoTilt rstTilt;
//					PIAL::_AlgoTilt algoTilt = PIAL::_AlgoTilt();
//					//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoTilt;
//					AlgoTilt* pAlgoTilt = (AlgoTilt*)sInspAlgo.m_ptrInspAlgoParam;					
//					m_PInspWrapper->ConvertAlgo(pAlgoTilt, algoTilt);
//
//					//std::vector< PIAL::_AlignResult> vecAlign;
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspTilt(algoTilt, *pImg_buf, &rstTilt, vecAlignResult, stTieArea);
//					
//					bool ast = m_PInspWrapper->ConvertRstAlgo(rstTilt, ((RstAlgoTilt*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo));
//
//				}
//				break;
//				case eAlgoGray_Mean:
//				{
//					nLine = __LINE__;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoGrayMean;
//					AlgoGrayMean *pAlgoGrayMean = (AlgoGrayMean *)sInspAlgo.m_ptrInspAlgoParam;
//					PIAL::_RstAlgoGrayMean rstGrayMean;
//					PIAL::_AlgoGrayMean algoGrayMean;
//					
//					m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
//					m_PInspWrapper->ConvertAlgo(pAlgoGrayMean, algoGrayMean);
//					
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspGrayMean(algoGrayMean, *pImg_buf, &rstGrayMean, stTieArea, m_pParamArray[nWndIndex].WndInspType);
//
//					m_PInspWrapper->ConvertRstAlgo(rstGrayMean, (RstAlgoGrayMean*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//				}
//				break;
//				case eAlgoHeight_Mean:
//				{
//					nLine = __LINE__;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoHeightMean;
//					AlgoHeightMean *pAlgoHeightMean = (AlgoHeightMean *)sInspAlgo.m_ptrInspAlgoParam;
//					PIAL::_RstAlgoHeightMean rstHeightMean;
//					PIAL::_AlgoHeightMean algoHeightMean;
//				
//					m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
//					m_PInspWrapper->ConvertAlgo(pAlgoHeightMean, algoHeightMean);
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspHeightMean(algoHeightMean, *pImg_buf, *pImg_buf, stTieArea, &rstHeightMean);
//					m_PInspWrapper->ConvertRstAlgo(rstHeightMean, (RstAlgoHeightMean*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//				}
//				break;
//				case eAlgoGray_Diff:
//				{
//					nLine = __LINE__;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoGrayDiff;
//					AlgoGrayDiff *pAlgoGrayDiff = (AlgoGrayDiff *)sInspAlgo.m_ptrInspAlgoParam;
//					PIAL::_RstAlgoGrayDiff rstGrayDiff;
//					PIAL::_AlgoGrayDiff algoGrayDiff;
//					PIAL::_AlignResult *rstAlign = new PIAL::_AlignResult();
//					if (nAlignCnt < 1)
//						nAlignCnt = 1;
//					m_PInspWrapper->ConvertAlgo(pAlgoGrayDiff, algoGrayDiff);
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspGrayDiff(algoGrayDiff, *pImg_buf, &rstGrayDiff, rstAlign, nAlignCnt);
//					m_PInspWrapper->ConvertRstAlgo(rstGrayDiff, (RstAlgoGrayDiff*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					if (rstAlign)
//						delete rstAlign;
//				}
//				break;
//				case eAlgoHeight_Diff:
//				{
//					nLine = __LINE__;
//					//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoHeightDiff;
//					AlgoHeightDiff *pAlgoHeightDiff = (AlgoHeightDiff *)sInspAlgo.m_ptrInspAlgoParam;
//					PIAL::_RstAlgoHeightDiff rstHeightDiff;
//					PIAL::_AlgoHeightDiff algoHeightDiff;
//					PIAL::_AlignResult *rstAlign = new PIAL::_AlignResult();
//					if (nAlignCnt < 1)
//						nAlignCnt = 1;
//					m_PInspWrapper->ConvertAlgo(pAlgoHeightDiff, algoHeightDiff);
//
//					if(ptrAlignRes != NULL)
//						m_PInspWrapper->ConvertRstAlgo(ptrAlignRes, rstAlign);
//
//					bool bUseBW = ((pAlgoHeightDiff->m_sBlobBase.m_nArrValue[_m_enBlobBase_Dt] & _m_enBlobBase_Data_eUse) == _m_enBlobBase_Data_eUse);
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspHeightDiff(algoHeightDiff, *pImg_buf, &rstHeightDiff, rstAlign, nAlignCnt);
//
//					m_PInspWrapper->ConvertRstAlgo(rstHeightDiff, (RstAlgoHeightDiff*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					if (rstAlign)
//						delete rstAlign;
//				}
//				break;
//				case eAlgoLine:
//				{
//					AlgoLine *pAlgo = (AlgoLine *)sInspAlgo.m_ptrInspAlgoParam;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoLine;
//
//					PIAL::_AlgoLine _algo;
//					PIAL::_RstAlgoLine _rst;
//
//					m_PInspWrapper->ConvertAlgo(pAlgo, _algo);
//					m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspLine(_algo, *pImg_buf, &_rst, stTieArea, nullptr);
//					m_PInspWrapper->ConvertRstAlgo(_rst, (RstAlgoLine*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					RstAlgoLine * rst = (RstAlgoLine *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//
//					rst->m_poDrawCenter.x += dX;
//					rst->m_poDrawCenter.y = height - (rst->m_poDrawCenter.y + dY);
//					for (int nIdx = 0; nIdx < 2; nIdx++)
//					{
//						rst->m_poDrawLine[nIdx].x += dX;
//						rst->m_poDrawLine[nIdx].y += dY;
//						rst->m_poDrawLine_T[nIdx].x += dX;
//						rst->m_poDrawLine_T[nIdx].y += dY;
//						rst->m_poDrawLine_2[nIdx].x += dX;
//						rst->m_poDrawLine_2[nIdx].y += dY;
//					}
//					for (int nIdx = 0; nIdx < 50; nIdx++)
//					{
//						rst->m_rcDrawWidth[nIdx].left += dX;
//						rst->m_rcDrawWidth[nIdx].right += dX;
//						rst->m_rcDrawWidth[nIdx].top += dY;
//						rst->m_rcDrawWidth[nIdx].bottom += dY;
//					}
//					if (bAlgoOK && !bAlignResOK && (inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
//					{
//						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
//						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
//						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
//						arrAlignRes[nAlignWndIdx].offsetX = -rst->m_dRstShiftX;
//						arrAlignRes[nAlignWndIdx].offsetY = rst->m_dRstShiftY;
//						arrAlignRes[nAlignWndIdx].theta = -rst->m_dRstAngle;
//						bAlignResOK = TRUE;
//					}
//				}
//				break;
//				case eAlgoEdge:
//				{
//					AlgoEdge *pAlgo = (AlgoEdge *)sInspAlgo.m_ptrInspAlgoParam;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoEdge;
//
//					PIAL::_AlgoEdge _algo;
//					PIAL::_RstAlgoEdge _rst;
//
//					m_PInspWrapper->ConvertAlgo(pAlgo, _algo);
//					m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
//
//					/*PIAL::BodyInfo* pBody = nullptr;
//					if (InspData)
//						pBody = new PIAL::BodyInfo;*/
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspEdge(_algo, *pImg_buf, &_rst, stTieArea, nullptr);
//					_rst.m_poDrawCenter.x += dX;
//					_rst.m_poDrawCenter.y = height - (_rst.m_poDrawCenter.y + dY);
//					for (int nIdx = 0; nIdx < EdgeLineTotalCnt; nIdx++)
//					{
//						_rst.m_poDrawLine[nIdx].x += dX;
//						_rst.m_poDrawLine[nIdx].y += dY;
//						_rst.m_poDrawLine_Sec[nIdx].x += dX;
//						_rst.m_poDrawLine_Sec[nIdx].y += dY;
//						_rst.m_poDrawLine_T[nIdx].x += dX;
//						_rst.m_poDrawLine_T[nIdx].y += dY;
//						_rst.m_poDrawLine_T2[nIdx].x += dX;
//						_rst.m_poDrawLine_T2[nIdx].y += dY;
//					}
//					m_PInspWrapper->ConvertRstAlgo(_rst, (RstAlgoEdge*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					if (bAlgoOK && !bAlignResOK && (inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
//					{
//						nLine = __LINE__;
//
//						RstAlgoEdge * rst = (RstAlgoEdge *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
//						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
//						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
//						arrAlignRes[nAlignWndIdx].offsetX = -rst->m_dRstShiftX;
//						arrAlignRes[nAlignWndIdx].offsetY = rst->m_dRstShiftY;
//						arrAlignRes[nAlignWndIdx].theta = rst->m_dRstAngle;
//						bAlignResOK = TRUE;
//
//						nLine = __LINE__;
//					}
//				
//
//				}
//				break;
//				case eAlgoPadBW:
//				{
//					nLine = __LINE__;
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoPadBW;
//					AlgoPadBW *pAlgoPadBW = (AlgoPadBW *)sInspAlgo.m_ptrInspAlgoParam;
//
//					//Color
//					PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;		//위로 올림
//
//					//PIAL::_TotalInspExceptArea stTieArea2;
//					PIAL::_AlgoPadBW algo2;
//					PIAL::_RstAlgoPadBW rstAlgo2;
//
//					//m_PInspWrapper->ConvertAlgo(pAlgoPadBW, algo2);
//
//					//이 부분에 할당된 버퍼 및 FOV Size 변수 정의ㄴㄴㄴ
//					UCHAR** m_FOVImgComposed_buf = m_ucPadRstBuffer;
//					UCHAR** m_ImgCalc_buf = m_ucPadCalcBuffer;
//
//					int nFOVBufferSizeW = m_fovWidth;
//					int nFOVBufferSizeH = m_fovLength;
//
//					m_PInspWrapper->MakeROIImg(&m_pInspBoardInfo->partImgBuf, pwnd_buf_ROI);
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspPadBW(algo2, *pImg_buf, *pwnd_buf_ROI, &rstAlgo2, nFOVBufferSizeW, nFOVBufferSizeH, m_ImgCalc_buf, m_FOVImgComposed_buf);
//					//bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspPadBW(algo2, *pWnd_buf, *pwnd_buf_ROI, &rstAlgo2, nFOVBufferSizeW, nFOVBufferSizeH, m_ImgCalc_buf, m_FOVImgComposed_buf/*, stTieArea2, pPreImgDst*/);
//
//					//m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoPadBW*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//
//					delete pwnd_buf_ROI;
//
//				}
//				break;
//				case eAlgoNGBlob:  //이전 결과 사용
//				{
//					nLine = __LINE__;
//					AlgoNGBlob *pAlgoNGBlob = (AlgoNGBlob *)sInspAlgo.m_ptrInspAlgoParam;
//
//					PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
//
//					//stTieArea2 요거도 추가해야함.
//					PIAL::_TotalInspExceptArea stTieArea2;
//
//					PIAL::_AlgoNGBlob algo2;
//					PIAL::_RstAlgoNGBlob rstAlgo2;
//					//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoNGBlob;
//
//					m_PInspWrapper->ConvertAlgo(pAlgoNGBlob, algo2);
//					std::vector<PIAL::PInspData*> pDataSet = InspDataSet->GetInspDataSet();
//					SIZE partCenter;
//					partCenter.cx = m_pInspBoardInfo->partCx;
//					partCenter.cy = m_pInspBoardInfo->partCy;
//
//					cv::Mat wndimg2D = pImg_buf->m_p2D->Mat();
//
//					PIAL::PI_Buff* pMaskImg = nullptr; //Mask 이미지
//					pMaskImg = new PIAL::PI_Buff(pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY);
//					
//					/*if (InspData)
//					    PIAL::PInspDataSet::SetMaskToImage(pImg_buf, InspDataSet, InspData->GetMaskData(), false, dX, dY).copyTo(pMaskImg->Mat());
//					PIAL::PI_Buff::SaveImage(pMaskImg, L"D:\\PInspAlgo\\BodyEdgeCorner\\WindowMask1.bmp");*/
//					
//					if (InspDataSet)
//					{
//						InspDataSet->SetMaskToImage(pMaskImg, vecAlignResult, InspData->GetMaskData(), false, dX, dY);
//					}
//					//bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob_WIR(algo2, *pImg_buf, &rstAlgo2, stTieArea2, vecAlignResult, pDataSet, pMaskImg , false);
//					delete pMaskImg;
//					
//					m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoNGBlob*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					
//					RstAlgoNGBlob * rst = (RstAlgoNGBlob *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//					
//					if (rst->blob_count > 200)
//						rst->blob_count = 200;
//					for (int i = 0; i < rst->blob_count; ++i)
//					{
//						rst->m_rcArrRect[i].left += dX;
//						rst->m_rcArrRect[i].right += dX;
//						rst->m_rcArrRect[i].top += dY;
//						rst->m_rcArrRect[i].bottom += dY;
//					}
//
//					delete pwnd_buf_ROI; pwnd_buf_ROI = nullptr;
//				}
//				break;
//				case eAlgoBodyEdge:
//				{
//					//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBodyEdge;
//					
//					PIAL::_RstAlgoBodyEdge RstAlgo2;
//					PIAL::_AlgoBodyEdge Algo2;
//					m_PInspWrapper->ConvertAlgo((AlgoBodyEdge*)sInspAlgo.m_ptrInspAlgoParam, Algo2);
//
//					PIAL::BodyInfo* pBody = nullptr;
//					if (InspData)
//						pBody = new PIAL::BodyInfo;
//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->PInspAlgo::InspBodyEdge(Algo2, *pImg_buf, &RstAlgo2, vecAlignResult, pBody);
//					m_PInspWrapper->ConvertRstAlgo(RstAlgo2,(RstAlgoBodyEdge*) WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					
//					if (InspData && pBody)
//						InspData->SetBodyData(pBody);
//
//					if (bAlgoOK)
//					{
//						RstAlgoBodyEdge * rst = (RstAlgoBodyEdge *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
//						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
//						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
//						arrAlignRes[nAlignWndIdx].offsetX = -rst->m_dRstOffset_x;
//						arrAlignRes[nAlignWndIdx].offsetY = rst->m_dRstOffset_y;
//						arrAlignRes[nAlignWndIdx].theta = rst->m_dRstTheta;
//						ptrAlignRes[nCurAlignID] = arrAlignRes[nAlignWndIdx];
//						bAlignResOK = TRUE;
//					}
//				}
//				break;
//				case eAlgoPattern:
//				{
//					CPInsp_Pattern *pPatternInsp = (CPInsp_Pattern *)GetPatternInsp();
//					if (pPatternInsp == nullptr)
//						return bAlgoOK;
//
//					//pPatternInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
//
//					//임시
//					SetExtMachinePatternParam(sInspAlgo);
//
//					pPatternInsp->SetInspParam(sInspAlgo, wndImg, coordinateAlgo, m_pInspBoardInfo->partImgBuf.nImageSizeX, m_pInspBoardInfo->partImgBuf.nImageSizeY, _stTieArea);
//					int a, b;
//					bAlgoOK = pPatternInsp->InspProc_Pattern(3, nOffX_pix, nOffY_pix);
//					//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoPattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//					pPatternInsp->GetInspRst((RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//
//					if (bAlgoOK && !bAlignResOK && (inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
//					{
//						RstAlgoPattern * rst = (RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
//						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
//						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
//						arrAlignRes[nAlignWndIdx].offsetX = -rst->offsetX;
//						arrAlignRes[nAlignWndIdx].offsetY = rst->offsetY;
//						arrAlignRes[nAlignWndIdx].theta = 0;
//						//ptrAlignRes[nCurAlignID] = arrAlignRes[nAlignWndIdx];
//						bAlignResOK = TRUE;
//					}
//
//				}
//				break;
//				case eAlgoPOCR:
//				{
//					CPInsp_POCR *pPOCR = (CPInsp_POCR *)g_pInspMng->GetPocrTeach();
//					InspPartInfo* pInspBoardInfo = m_pInspBoardInfo;
//
//					//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoPOCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//
//					cv::Mat testimg(wndImg.m_nHeight, wndImg.m_nWidth, CV_8UC1, wndImg.m_ucArr2D);
//
//					pPOCR->SetInspParam(sInspAlgo, pInspBoardInfo->angle);
//					bAlgoOK = pPOCR->InspProc_POCR(wndImg.m_ucArr2D, wndImg.m_nWidth, wndImg.m_nHeight, 0);
//					pPOCR->GetInspRst((RstAlgoPOCR *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//					
//				}
//				break;
//
//				case eAlgoBarcode:
//				{
//					//CPInsp_Barcode *pBarocde = (CPInsp_Barcode *)g_pInspMng->GetBarcodeInsp();
//
//					//bAlgoOK = pBarocde->InspBarcode(sInspAlgo, wndImg, (RstAlgoBarcode *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//				}
//				break;
//			}
//			if (eAlgoNum > (int)eInspAlgoType)
//				m_dAlgoListTack[(int)eInspAlgoType] += ((GetTickCount() - start) / 1000.0f);
//
//
//
//			if (WndResult->m_vArrRstInspAlgo)
//			{
//				if (bUSeLeadAlign == FALSE)
//					WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = bAlgoOK;
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsRequired = sInspAlgo.m_bIsRequired;
//			}
//
//			nLine = __LINE__;
//			pImg_buf->ClearWindowData();
//		}
//
//		
//		BOOL bIsInsp = TRUE;
//
//		if (ret == e_OK && bUSeLeadAlign == FALSE)
//		{
//
//			nLine = __LINE__;
//
//			int nResGroup = e_NG;		// Group이 아닌 알고리즘들의 결과
//			int nResNotGroup = e_NG;		// Group인 알고리즘들의 결과
//			BOOL bIsEssentialInGroup = FALSE;	// Group에 포함된 알고리즘 중 Essential check된 것이 있는지 여부
//			bIsInsp = FALSE;
//			if (m_pParamArray[nWndIndex].nAlgorithmCnt > 0)
//				ret = e_NG;		// 일단 NG로,
//			for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
//			{
//				if (!WndResult->m_vArrRstInspAlgo || WndResult->m_nAlgorithmCnt <= nAlgo)
//					continue;
//
//				nLine = __LINE__;
//
//				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Search || WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Tip
//					|| WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoTab_Search)
//				{
//					if (m_pParamArray[nWndIndex].nAlgorithmCnt == 1)
//						ret = e_OK;	// LeadSearch
//					continue;
//				}
//				// 알고리즘 간 기본 OR 처리 (여기 들어왔다는 것은 필수 검사는 모두 OK 인 경우이므로 고려하지 않아도 됨)
//				// 2015/08/10 : 알고리즘 Group flag 추가되어서 필수 검사 NG 여부도 여기서 체크해야함
//
//				nLine = __LINE__;
//
//				InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
//				if (sInspAlgo.m_bAlgoGroup && sInspAlgo.m_bIsRequired)
//					bIsEssentialInGroup = TRUE;
//
//				nLine = __LINE__;
//
//				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk == TRUE)
//				{
//					if (!sInspAlgo.m_bAlgoGroup)
//						nResNotGroup = e_OK;		// Group Enable = false인 알고리즘 결과가 하나라도 OK라면 OK
//					else
//						nResGroup = e_OK;		// Group Enable = true인 알고리즘 결과가 하나라도 OK라면 OK
//				}
//
//				bIsInsp = TRUE;
//			}
//
//			nLine = __LINE__;
//
//			if (nResGroup == e_NG && bIsEssentialInGroup)		// Group의 결과가 NG이고 Group에 포함된 알고리즘 중 Essential check가 있으면 무조건 NG
//				ret = e_NG;
//			else if (nResNotGroup == e_OK || nResGroup == e_OK)	// 그렇지 않고, Group이 아닌 알고리즘 중 OK가 있거나, Group인 알고리즘 중 OK가 있으면 OK
//				ret = e_OK;
//		}
//
//		if (bUSeLeadAlign == FALSE)
//		{
//			WndResult->m_bOk = ret;
//			WndResult->m_bIsInsp = bIsInsp;
//		}
//		
//		if (ptrAlignRes)
//			delete[] ptrAlignRes;
//		if (pImg_buf)
//			delete pImg_buf;
//
//		if (wndImg.m_ucArr2D)
//		{
//			//Delete_1DArray(&wndImg.m_ucArr2D);
//			g_pMManager->pem_delete(wndImg.m_ucArr2D, true);
//			wndImg.m_ucArr2D = NULL;
//		}
//		if (wndImg.m_ucArrCV)
//		{
//			//Delete_1DArray(&wndImg.m_ucArrCV);
//			g_pMManager->pem_delete(wndImg.m_ucArrCV, true);
//			wndImg.m_ucArrCV = NULL;
//		}
//			
//		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
//		{
//			if (wndImg.m_ucArr2D_Mix[iLoopCount] != NULL)
//			{
//				//Delete_1DArray(&wndImg.m_ucArr2D_Mix[iLoopCount]);
//				g_pMManager->pem_delete(wndImg.m_ucArr2D_Mix[iLoopCount], true);
//				wndImg.m_ucArr2D_Mix[iLoopCount] = NULL;
//			}
//		}
//		if (wndImg.m_fArr3D)
//		{
//			//Delete_1DArray(&wndImg.m_fArr3D);
//			g_pMManager->pem_delete(wndImg.m_fArr3D, true);
//			wndImg.m_fArr3D = NULL;
//		}
//			
//
//		nLine = __LINE__;
//
//		if (wndImg.m_fArr3D)
//			Delete_1DArray(&wndImg.m_fArr3D);
//	}
//	catch (...)
//	{
//		if (ptrAlignRes)
//			delete[] ptrAlignRes;
//		if (pImg_buf)
//			delete pImg_buf;
//	}
//
//	return ret;
//}

//int InspManager::InspBGAWindowAlgo(int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, PIAL::PInspDataSet* InspDataSet)
//{
//	int ret = e_OK;
//
//	int inspType = 0;
//
//	int nLine = __LINE__;
//	try
//	{
//		inspType = m_pParamArray[nWndIndex].inspType;
//		PIAL::PInspData* InspData = nullptr; // 삭제하지 말 것..(소멸자에서 삭제함)
//		if (InspDataSet != nullptr)
//		{
//			InspData = new PIAL::PInspData;
//			InspData->WindowID = m_pParamArray[nWndIndex].wndIndex;
//			InspDataSet->SetInspData(InspData);
//		}
//
//		int nAlignCnt = 0;
//		AlignResult * ptrAlignRes = NULL;
//		try
//		{
//			if ((m_pParamArray[nWndIndex].nAlignWndID != 0) && arrAlignRes != NULL && nAlignWndIdx > 0)
//			{
//				int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
//				ptrAlignRes = new AlignResult[nSize];
//				for (int a = 0; a < nSize; a++)
//					ptrAlignRes[a].nWindowID = -1;
//
//				if (FindAlignResultData(nWndIndex, arrAlignRes, ptrAlignRes, &nAlignCnt) == false)
//					Delete_1DArray(&ptrAlignRes);
//			}
//		}
//		catch (...)
//		{
//			Delete_1DArray(&ptrAlignRes);
//		}
//
//		AlgoCoordinate coordinateAlgo;
//
//		//Interface때문에 둠...
//		int nOffX_pix = 0;
//		int nOffY_pix = 0;
//
//		bool bAlgoOK = FALSE;
//
//		PIAL::PI_Buff* pPreImgDst = nullptr; //결과 이미지
//		PIAL::PI_Buff* pForngBlobMaskImg = nullptr; // NGBlob 에 쓰일 Mask 이미지
//		PIAL::PI_Buff* pForMakingmaskimg = nullptr; // NGBlob 에 쓰일 Mask 이미지를 생성하기 위한 이미지
//		PIAL::PI_Buff* pBuffBallDamage = nullptr; // NGBlob 에 쓰일 Mask 이미지를 생성하기 위한 이미지
//		PIAL::Insp_Image* pImg_buf = nullptr; //메인 검사이미지(BGA)
//
//		int wnd_w = 0;
//		int wnd_h = 0;
//		double dX = 0.0;
//		double dY = 0.0;
//		int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
//		int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;
//		int InspAreaType = 0;
//		bool bUseNGBump = false;
//		bool bUseUnCoining = false;
//
//		PIAL::AlgoSetBGA* pBGASet = nullptr;
//		std::vector<PIAL::AlgoSetBump> vecBumpSet;
//		vecBumpSet.reserve(m_pParamArray[nWndIndex].nAlgorithmCnt);
//
//		std::vector<PIAL::_AlignResult> vecAlignResult;
//		m_PInspWrapper->ConvertAlignResult(ptrAlignRes, nAlignCnt, vecAlignResult);
//
//		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
//		{
//			DWORD start = GetTickCount();
//			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
//
//			nLine = __LINE__;
//
//			if (!m_pParamArray[nWndIndex].vArrAlgoParam/* || !WndResult->m_vArrRstInspAlgo*/)
//				continue;
//
//			nLine = __LINE__;
//
//			/*WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = sInspAlgo.m_eAlgoType;
//			WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;*/
//
//			//BGA 알고리즘만 이진화함
//			if (sInspAlgo.m_eAlgoType == eAlgoBGA || ((sInspAlgo.m_eAlgoType == eAlgoNGBlob) && (((AlgoNGBlob*)sInspAlgo.m_ptrInspAlgoParam)->narrdata[_NGBlob_by_inspType] != _m_eNGBlob_InspType::ExtraBump)))
//			{
//				if (pImg_buf)
//					delete pImg_buf;
//
//				pImg_buf = new PIAL::Insp_Image;
//
//				//Align
//				if (nAlignCnt < 2)
//				{
//					m_PInspWrapper->WndSizeChange(pImg_buf, sInspAlgo, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY,
//						ptrAlignRes, nOffX_pix, nOffY_pix);
//				}
//				else
//				{
//					m_PInspWrapper->WndSizeChange_ArrAlign(pImg_buf, sInspAlgo.m_eAlgoType, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY
//						, ptrAlignRes, inspType, sInspAlgo, nAlignCnt);
//				}
//
//				// Input 2D Image 합성
//				int nSideCameraIndex = m_pParamArray[nWndIndex].nInspCameraType;
//				// Input 2D Image ÇÕ¼º
//				RECT rtWnd;
//				rtWnd.left = dX;
//				rtWnd.top = dY;
//				rtWnd.right = dX + wnd_w - 1;
//				rtWnd.bottom = dY + wnd_h - 1;
//				pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeX = wnd_w;
//				pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeY = wnd_h;
//
//				// 여기에서 Mix조명 합성해야 할 수도 있다.
//				m_PInspWrapper->NormalImageCompose(m_pInspBoardInfo, sInspAlgo.m_eAlgoType, sInspAlgo, pImg_buf, pImg_buf->m_p2D, width, height, wnd_w, wnd_h, dX, dY, nSideCameraIndex, ptrAlignRes, nAlignCnt);
//				m_PInspWrapper->SetWindowImage(m_pInspBoardInfo, pImg_buf, rtWnd);
//				//NG Bump 합성 필요
//// 				AlgoBGA *pAlgoBlob = (AlgoBGA*)sInspAlgo.m_ptrInspAlgoParam;
//// 				if (sInspAlgo.m_eAlgoType == eAlgoBGA)
//// 				{
//// 					AlgoBGA *pAlgoBlob = (AlgoBGA*)sInspAlgo.m_ptrInspAlgoParam;
//// 
//// 					if (pAlgoBlob->m_bUseContrast == TRUE)
//// 					{
//// 						m_PInspWrapper->SetWindowImage(m_pInspBoardInfo, pImg_buf, rtWnd);
//// 					}
//// 
//// 					if (pAlgoBlob->m_bUseNGBump && sInspAlgo.m_nMixCount > 0)
//// 					{
//// 						pImg_buf->m_p2D_Mix[0] = new PIAL::PI_Buff(wnd_w, wnd_h);
//// 						m_PInspWrapper->AlgoImageMixCompose(m_pInspBoardInfo, sInspAlgo.InspAlgoLightsMix[0], pImg_buf->m_p2D_Mix[0]->m_pData, width, height, wnd_w, wnd_h, dX, dY);
//// 					}
//// 					if (pAlgoBlob->m_bUseUncoining && sInspAlgo.m_nMixCount > 1)
//// 					{
//// 						pImg_buf->m_p2D_Mix[1] = new PIAL::PI_Buff(wnd_w, wnd_h);
//// 						m_PInspWrapper->AlgoImageMixCompose(m_pInspBoardInfo, sInspAlgo.InspAlgoLightsMix[1], pImg_buf->m_p2D_Mix[1]->m_pData, width, height, wnd_w, wnd_h, dX, dY);
//// 					}
//// 				}
//
//			}
//			if (pImg_buf)
//			{
//				pImg_buf->inspWndImage->nAlgoIndex = nAlgo;
//			}
//			//WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;
//
//
//			switch (sInspAlgo.m_eAlgoType)
//			{
//			case eAlgoBGA:
//			{
//				nLine = __LINE__;
//				AlgoBGA *pAlgoBlob = (AlgoBGA*)sInspAlgo.m_ptrInspAlgoParam;
//
//				//Color
//				PIAL::_TotalInspExceptArea stTieArea2;
//				PIAL::_AlgoBGA algo2;
//				PIAL::_RstAlgoBGA rstAlgo2;
//				m_PInspWrapper->ConvertAlgo(pAlgoBlob, algo2);
//				InspAreaType = algo2.m_InspAreaType;
//				bUseNGBump = algo2.m_bUseNGBump;
//				bUseUnCoining = algo2.m_bUseUnCoining;
//
//				pPreImgDst = new PIAL::PI_Buff(pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY);
//
//				pForMakingmaskimg = pPreImgDst->Clone();
//				// algo2.m_InspAreaType = 1;
//				// algo2.m_bUseNGBump = true;
//				// algo2.m_bUseUnCoining = true;
//
//				if (algo2.m_InspAreaType == 1)
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBGA_Coining(algo2, *pImg_buf, &rstAlgo2, stTieArea2, pPreImgDst, pForMakingmaskimg);
//				else
//					m_PInspWrapper->m_PInspAlgo->InspBGA(algo2, *pImg_buf, &rstAlgo2, stTieArea2, pPreImgDst, pForMakingmaskimg);
//			
//
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBGA;
//				m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoBGA*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//
//				if (pBGASet) delete pBGASet;
//				pBGASet = new PIAL::AlgoSetBGA(nAlgo, algo2, rstAlgo2);
//			}
//			break;
//			case eAlgoBump:
//			{
//
//				nLine = __LINE__;
//				AlgoBump *pAlgoBump = (AlgoBump *)sInspAlgo.m_ptrInspAlgoParam;
//				PIAL::_AlgoBump algo2;
//				PIAL::_RstAlgoBump rstAlgo2;
//				m_PInspWrapper->ConvertAlgo(pAlgoBump, algo2);
//				algo2.m_InspAreaType = InspAreaType;
//				algo2.m_bUseNGBump = bUseNGBump;
//				if (algo2.m_InspAreaType == 1)
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBump_Coining(algo2, *pImg_buf, &rstAlgo2, nOffX_pix, nOffY_pix, vecAlignResult, pPreImgDst);
//				else
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBump(algo2, *pImg_buf, &rstAlgo2, nOffX_pix, nOffY_pix, vecAlignResult, pPreImgDst, true);
//
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBump;
//				m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//				RstAlgoBump* rst = (RstAlgoBump *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//
//				if (InspDataSet && rst->fRstHeight > 0)
//					InspDataSet->SetThickestPosition(rst->fRstHeight, &rst->m_rcRectI);
//
//				rst->m_rcRectI.left += dX;
//				rst->m_rcRectI.right += dX;
//				rst->m_rcRectI.top += dY;
//				rst->m_rcRectI.bottom += dY;
//				//rst->m_poDrawCenter.x += dX;
//				//rst->m_poDrawCenter.y += dY; LSJ 이건 Align Window 에 관한것 같아서 일단 뺌..
//				if (rst->m_iFRectCnt > BUMP_RECT_CNTS)
//					rst->m_iFRectCnt = BUMP_RECT_CNTS;
//				for (int i = 0; i < rst->m_iFRectCnt; ++i)
//				{
//					rst->m_rcArrRectF[i].left += dX;
//					rst->m_rcArrRectF[i].right += dX;
//					rst->m_rcArrRectF[i].top += dY;
//					rst->m_rcArrRectF[i].bottom += dY;
//				}
//
//				vecBumpSet.push_back(PIAL::AlgoSetBump(nAlgo, algo2, rstAlgo2));
//
//				if (pAlgoBump->idata & Bump_Data_UseAnchor)
//				{
//					double CvtBoradX = 0;
//					double CvtBoradY = 0;
//					CvtBoradX = pAlgoBump->farrdata[Bump_F_TechCenterX] - rst->fRstShiftX;
//					CvtBoradY = pAlgoBump->farrdata[Bump_F_TechCenterY] + rst->fRstShiftY;
//					if (ptrAlignRes != NULL)
//					{
//						rst->fRstShiftX = CvtBoradX - ptrAlignRes->TeachCenterX;
//						rst->fRstShiftY = CvtBoradY - ptrAlignRes->TeachCenterY;
//					}
//				}
//				else if (pAlgoBump->idata & Bump_Data_UseTwoAnchor)
//				{
//					//
//					if (pAlgoBump->m_sAlignWindow != _T("") && arrAlignRes != NULL)
//					{
//						AlignResult * ptrAlignRes2 = NULL;
//						int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
//						ptrAlignRes2 = new AlignResult[nSize];
//						int WindowId = 0;
//
//						CString csWindowID;
//						csWindowID.Format(_T("%s"), pAlgoBump->m_sAlignWindow);
//						if (csWindowID.GetLength() == 7)
//							WindowId = _ttoi(csWindowID.Right(1));
//						else
//							WindowId = _ttoi(csWindowID.Right(2));
//
//						FindAlignResultData(nWndIndex, arrAlignRes, ptrAlignRes2, &nAlignCnt, WindowId);
//
//						double CvtBoradX = 0;
//						double CvtBoradY = 0;
//						double CenterX = 0;
//						double CenterY = 0;
//						CvtBoradX = pAlgoBump->farrdata[Bump_F_TechCenterX] - rst->fRstShiftX;
//						CvtBoradY = pAlgoBump->farrdata[Bump_F_TechCenterY] + rst->fRstShiftY;
//
//						if (ptrAlignRes != NULL && ptrAlignRes2 != NULL)
//						{
//							CenterX = (ptrAlignRes->TeachCenterX + ptrAlignRes2->TeachCenterX) / 2;
//							CenterY = (ptrAlignRes->TeachCenterY + ptrAlignRes2->TeachCenterY) / 2;
//							rst->fRstShiftX = CvtBoradX - CenterX;
//							rst->fRstShiftY = CvtBoradY - CenterY;
//						}
//					}
//				}
//			}
//			break;
//			case eAlgoBlob: // 기존 범프
//			{
//				nLine = __LINE__;
//				AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
//
//				if (pAlgoBlob->m_bUseBlobNG)
//				{
//					break;
//				}
//				PIAL::_AlgoBump algo2;
//				PIAL::_RstAlgoBump rstAlgo2;
//				m_PInspWrapper->ConvertAlgo(pAlgoBlob, algo2);
//				algo2.m_InspAreaType = InspAreaType;
//				algo2.m_bUseNGBump = bUseNGBump;
//				if (algo2.m_InspAreaType == 1)
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBump_Coining(algo2, *pImg_buf, &rstAlgo2, nOffX_pix, nOffY_pix, vecAlignResult, pPreImgDst);
//				else
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBump(algo2, *pImg_buf, &rstAlgo2, nOffX_pix, nOffY_pix, vecAlignResult, pPreImgDst, true);
//
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBump;
//				m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//				RstAlgoBump* rst = (RstAlgoBump *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//
//				vecBumpSet.push_back(PIAL::AlgoSetBump(nAlgo, algo2, rstAlgo2));
//
//				rst->m_rcRectI.left += dX;
//				rst->m_rcRectI.right += dX;
//				rst->m_rcRectI.top += dY;
//				rst->m_rcRectI.bottom += dY;
//
//				if (rst->m_iFRectCnt > BUMP_RECT_CNTS)
//					rst->m_iFRectCnt = BUMP_RECT_CNTS;
//				for (int i = 0; i < rst->m_iFRectCnt; ++i)
//				{
//					rst->m_rcArrRectF[i].left += dX;
//					rst->m_rcArrRectF[i].right += dX;
//					rst->m_rcArrRectF[i].top += dY;
//					rst->m_rcArrRectF[i].bottom += dY;
//				}
//			}			
//			break;
//			case eAlgoNGBlob:
//			{
//				AlgoNGBlob *pAlgoNGBlob = (AlgoNGBlob*)sInspAlgo.m_ptrInspAlgoParam;
//				PIAL::_AlgoNGBlob algo2;
//				PIAL::_RstAlgoNGBlob rstAlgo2;
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoNGBlob;
//				m_PInspWrapper->ConvertAlgo(pAlgoNGBlob, algo2);
//				bool bNeedMask = true;
//				if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::BallDamage)
//					bNeedMask = false;
//
//				if (pForngBlobMaskImg == nullptr && pPreImgDst != nullptr)
//				{
//					pForngBlobMaskImg = new PIAL::PI_Buff(pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY);
//					
//					if (InspAreaType == 0)
//						cv::bitwise_xor(pForMakingmaskimg->Mat(), pPreImgDst->Mat(), pForngBlobMaskImg->Mat());
//					cv::bitwise_not(pForngBlobMaskImg->Mat(), pForngBlobMaskImg->Mat());
//
//					PIAL::PAlgo::MorErode(pForngBlobMaskImg, pForngBlobMaskImg,2,17);
//
//					if (InspDataSet)
//					{
//						cv::Mat Body_mask = PIAL::PInspDataSet::SetMaskToImage(pForngBlobMaskImg->Mat(), InspDataSet, InspDataSet->GetMaskData(), false, dX, dY);
//						cv::bitwise_and(pForngBlobMaskImg->Mat(), Body_mask, pForngBlobMaskImg->Mat());
//					}
//				}
//
//				PIAL::_TotalInspExceptArea stTieArea;
//				m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
//
//				if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::Warpage
//					&& pAlgoNGBlob->narrdata[_NGBlob_by_WarpageOption] == 2) //Ball Warpage
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob_BallWarpage(algo2, *pImg_buf, &rstAlgo2, vecBumpSet);
//				else if(pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::Default && InspAreaType == 1)//defulte
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob(algo2, *pImg_buf, &rstAlgo2, pForngBlobMaskImg);
//				else if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::BallDamage)//
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob_BallDamage(algo2, *pImg_buf, &rstAlgo2, vecBumpSet, pBuffBallDamage);
//				else
//				{
//					std::vector<PIAL::PInspData*> pDataSet = InspDataSet->GetInspDataSet();
//					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob(algo2, *pImg_buf, &rstAlgo2, stTieArea, vecAlignResult, pDataSet, pForngBlobMaskImg);
//				}
//
//				m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoNGBlob*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//				RstAlgoNGBlob * rst = (RstAlgoNGBlob *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
//				if (rst->blob_count > 200)
//					rst->blob_count = 200;
//				for (int i = 0; i < rst->blob_count; ++i)
//				{
//					rst->m_rcArrRect[i].left += dX;
//					rst->m_rcArrRect[i].right += dX;
//					rst->m_rcArrRect[i].top += dY;
//					rst->m_rcArrRect[i].bottom += dY;
//				}
//			}
//			break;
//// 			case eAlgoPackageThickness:
//// 			{
//// 				nLine = __LINE__;
//// 				AlgoPackageThickness* pAlgoPackageThickness = (AlgoPackageThickness *)sInspAlgo.m_ptrInspAlgoParam;
//// 				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoPackageThickness;
//// 				PIAL::_RstAlgoPackageThickness rstPackageThickness;
//// 				PIAL::_AlgoPackageThickness packageThickness;
//// 				m_PInspWrapper->ConvertAlgo(pAlgoPackageThickness, packageThickness);
//// 				bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspPackageThickness(&packageThickness, &rstPackageThickness, InspDataSet);
//// 				m_PInspWrapper->ConvertRstAlgo(rstPackageThickness, (RstAlgoPackageThickness *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
//// 			}
//// 			break;
//			default:
//				//WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = FALSE;
//				break;
//			}
//
//			//WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = bAlgoOK;
//
//			if (sInspAlgo.m_bIsRequired && !sInspAlgo.m_bAlgoGroup && !bAlgoOK)	// 필수 검사 알고리즘이  NG이면 Window  결과  NG (단, Group이 체크되어있으면 일단 보류, 아래에서 판정)
//			{
//				ret = e_NG;
//			}
//
//			// 교체용 디펙트 코드
//			/*if (!bAlgoOK && sInspAlgo.m_bUsingManualDefectCode)
//			{
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_bUsingManualDefectCode = sInspAlgo.m_bUsingManualDefectCode;
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualDefectCode = sInspAlgo.m_nManualDefectCode;
//				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualSubDefectCode = sInspAlgo.m_nManualSubDefectCode;
//			}*/
//		}
//
//		//Bridge Bump
//		if (pImg_buf)
//		{
//			std::set<int>::iterator iter;
//			for (iter = pImg_buf->inspWndImage->m_BridgealgoIndex.begin(); iter != pImg_buf->inspWndImage->m_BridgealgoIndex.end(); ++iter)
//			{
//				int nAlgoIndex = (*iter);
//				InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgoIndex];
//				if (sInspAlgo.m_eAlgoType == eAlgoBlob)
//				{
//					//// 해당 Blob 알고리즘 결과 Bridge 처리
//					//RstAlgoBlob* resultBlob = (RstAlgoBlob*)WndResult->m_vArrRstInspAlgo[nAlgoIndex].m_vRstInspAlgo;
//					//if (resultBlob) resultBlob->m_bOKDistance = FALSE;
//				}
//			}
//		}
//		BOOL bIsInsp = TRUE;
//
//		if (InspAreaType == 1 && pBGASet) //Coining
//		{
//			bool bChanged = m_PInspWrapper->m_PInspAlgo->InspCoiningSecond(*pImg_buf, pBGASet, vecBumpSet);
//			if (bChanged)
//			{
//				bool bPartOk = true;
//				int nNumBump = vecBumpSet.size();
//				//결과 변경
//				for (size_t i = 0; i < nNumBump; i++)
//				{
//					PIAL::AlgoSetBump& bump = vecBumpSet[i];
//
//					if (bump.bChanged){}//변경
//				}
//
//			}
//		}
//		else if (pBGASet&&(pBGASet->m_Algo.m_bUseCoplanarity|| pBGASet->m_Algo.m_bUsePitch || 
//			pBGASet->m_Algo.m_bUseGridOffsetX || pBGASet->m_Algo.m_bUseGridOffsetY))
//		{
//			if (m_PInspWrapper->m_PInspAlgo->InspBGASecond(*pImg_buf, pBGASet, vecBumpSet))
//			{
//				//BGA 결과 변경
//				//BGA
//				RstAlgoBGA* resultBGA= (RstAlgoBGA*)WndResult->m_vArrRstInspAlgo[pBGASet->nAlgoindex].m_vRstInspAlgo;			
//				resultBGA->m_bCoplanarityOK = pBGASet->m_Rst.m_bOKCoplanarity;
//				resultBGA->m_fCoplanarity = pBGASet->m_Rst.m_fRstCoplanarity;
//
//			}
//		}
//
//
//		if (ret == e_OK)
//		{
//
//			//nLine = __LINE__;
//
//			//int nResGroup = e_NG;		// Group이 아닌 알고리즘들의 결과
//			//int nResNotGroup = e_NG;		// Group인 알고리즘들의 결과
//			//BOOL bIsEssentialInGroup = FALSE;	// Group에 포함된 알고리즘 중 Essential check된 것이 있는지 여부
//			//bIsInsp = FALSE;
//			//if (m_pParamArray[nWndIndex].nAlgorithmCnt > 0)
//			//	ret = e_NG;		// 일단 NG로,
//			//for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
//			//{
//			//	if (!WndResult->m_vArrRstInspAlgo)
//			//		continue;
//
//			//	nLine = __LINE__;
//
//			//	if (WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Search || WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Tip
//			//		|| WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoTab_Search)
//			//	{
//			//		if (m_pParamArray[nWndIndex].nAlgorithmCnt == 1)
//			//			ret = e_OK;	// LeadSearch
//			//		continue;
//			//	}
//			//	// 알고리즘 간 기본 OR 처리 (여기 들어왔다는 것은 필수 검사는 모두 OK 인 경우이므로 고려하지 않아도 됨)
//			//	// 2015/08/10 : 알고리즘 Group flag 추가되어서 필수 검사 NG 여부도 여기서 체크해야함
//
//			//	nLine = __LINE__;
//
//			//	InspAlgo sInspAlgo = ExtPrmPtr(m_pParamArray[nWndIndex].vArrAlgoParam)[nAlgo];
//			//	if (sInspAlgo.m_bAlgoGroup && sInspAlgo.m_bIsRequired)
//			//		bIsEssentialInGroup = TRUE;
//
//			//	nLine = __LINE__;
//
//			//	if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk == TRUE)
//			//	{
//			//		if (!sInspAlgo.m_bAlgoGroup)
//			//			nResNotGroup = e_OK;		// Group Enable = false인 알고리즘 결과가 하나라도 OK라면 OK
//			//		else
//			//			nResGroup = e_OK;		// Group Enable = true인 알고리즘 결과가 하나라도 OK라면 OK
//			//	}
//
//			//	bIsInsp = TRUE;
//			//}
//
//			//nLine = __LINE__;
//
//			//if (nResGroup == e_NG && bIsEssentialInGroup)		// Group의 결과가 NG이고 Group에 포함된 알고리즘 중 Essential check가 있으면 무조건 NG
//			//	ret = e_NG;
//			//else if (nResNotGroup == e_OK || nResGroup == e_OK)	// 그렇지 않고, Group이 아닌 알고리즘 중 OK가 있거나, Group인 알고리즘 중 OK가 있으면 OK
//			//	ret = e_OK;
//		}
//
//
//		//WndResult->m_bOk = ret == true ? TRUE : FALSE;
//		//WndResult->m_bIsInsp = bIsInsp;
//
//		//PIAL::PAlgo::SaveWorkImg(pPreImgDst, L"D:\\PInspAlgo\\BGA\\Mask.bmp", true);
//
//		if (pForngBlobMaskImg != nullptr) delete pForngBlobMaskImg;
//		if (pForMakingmaskimg != nullptr) delete pForMakingmaskimg;
//		if (pPreImgDst != nullptr) delete pPreImgDst;
//		if (pImg_buf != nullptr) delete pImg_buf;
//		if (pBGASet) delete pBGASet;
//		Delete_1DArray(&ptrAlignRes);
//	}
//	catch (...)
//	{
//		/*if (g_pMPTI)
//		{
//			CString sLog = _T("");
//			sLog.Format(_T("CMInspManager::InspBGAWindowAlgo(), Line : %d Pass!!!"), nLine);
//			g_pMPTI->AddLog_Dev(sLog);
//
//			throw nLine;
//		}*/
//	}
//
//	return ret;
//
//
//}
int InspManager::InspBGAWindowAlgo(int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, PIAL::PInspDataSet* InspDataSet, std::vector<InspRstPolyAlgo>* vpInspRstPoly)
{
	int ret = e_OK;

	int inspType = 0;
	int nAlgoType = 0, nAlgoID = 0;

	int nLine = __LINE__;
	PIAL::PInspData* InspData = nullptr;
	try
	{
		inspType = m_pParamArray[nWndIndex].inspType;
		if (m_bNewInspection)
		{
			InspData = new PIAL::PInspData;
			//g_pMManager->pem_new_check(InspData, (PCHAR)__FUNCTION__, __LINE__);

			InspData->WindowID = m_pParamArray[nWndIndex].wndIndex;
			InspDataSet->SetInspData(InspData);
		}
		nLine = __LINE__;

		int nAlignCnt = 0;
		AlignResult * ptrAlignRes = NULL;
		try
		{
			if ((m_pParamArray[nWndIndex].nAlignWndID != 0) && arrAlignRes != NULL && nAlignWndIdx > 0)
			{
				int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
				ptrAlignRes = g_pMManager->pem_new<AlignResult>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
				for (int a = 0; a < nSize; a++)
					ptrAlignRes[a].nWindowID = -1;

				if (FindAlignResultData(nWndIndex, arrAlignRes, ptrAlignRes, &nAlignCnt) == false)
					Delete_1DArray(&ptrAlignRes);
			}
		}
		catch (...)
		{
			Delete_1DArray(&ptrAlignRes);
		}

		AlgoCoordinate coordinateAlgo;

		//Interface때문에 둠...
		int nOffX_pix = 0;
		int nOffY_pix = 0;

		bool bAlgoOK = FALSE;

		PIAL::PI_Buff* pPreImgDst = nullptr; //결과 이미지
		PIAL::PI_Buff* pForngBlobMaskImg = nullptr; // NGBlob 에 쓰일 Mask 이미지
		PIAL::PI_Buff* pForMakingmaskimg = nullptr; // NGBlob 에 쓰일 Mask 이미지를 생성하기 위한 이미지
		PIAL::Insp_Image* pImg_buf = nullptr;

		int wnd_w = 0;
		int wnd_h = 0;
		double dX = 0.0;
		double dY = 0.0;
		int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
		int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;
		int InspAreaType = 0;
		bool bUseNGBump = false;
		bool bUseUnCoining = false;
		bool IsOKNGBlob = true;
		bool IsThicknessOK = true;
		int nBallMaskSize = -1;

		PIAL::AlgoSetBGA* pBGASet = nullptr; //BGA 전체 검사
		std::vector<PIAL::AlgoSetBump> vecBumpSet; //BGA 전체 검사
		vecBumpSet.reserve(m_pParamArray[nWndIndex].nAlgorithmCnt);

		std::vector<PIAL::_AlignResult> vecAlignResult;
		m_PInspWrapper->ConvertAlignResult(ptrAlignRes, nAlignCnt, vecAlignResult);


		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
		{
			DWORD start = GetTickCount();
			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];

			nAlgoID = nAlgo;
			nAlgoType = (int)sInspAlgo.m_eAlgoType;

			nLine = __LINE__;

			if (!m_pParamArray[nWndIndex].vArrAlgoParam || !WndResult->m_vArrRstInspAlgo)
				continue;

			nLine = __LINE__;

			WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = sInspAlgo.m_eAlgoType;
			WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;

			//BGA 알고리즘만 이진화함
			if (sInspAlgo.m_eAlgoType == eAlgoBGA || ((sInspAlgo.m_eAlgoType == eAlgoNGBlob) && (((AlgoNGBlob*)sInspAlgo.m_ptrInspAlgoParam)->narrdata[NGBlob_by_inspType] != 1)))
			{
				if (pImg_buf)
					//delete pImg_buf;
					g_pMManager->pem_delete(pImg_buf, false);

				pImg_buf = new PIAL::Insp_Image;
				g_pMManager->pem_new_check(pImg_buf, (PCHAR)__FUNCTION__, __LINE__);
				//DSI
				if (vpInspRstPoly)
				{
					pImg_buf->bUseDSI = true;
					pImg_buf->nWindowID = g_pInspMng->GetWindowID(nWndIndex);
					pImg_buf->nAlgoID = sInspAlgo.m_nAlgoId;
				}
				//Align
				if (nAlignCnt < 2)
				{
					m_PInspWrapper->WndSizeChange(pImg_buf, sInspAlgo, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY,
						&ptrAlignRes[0], nOffX_pix, nOffY_pix);
				}
				else
				{
					m_PInspWrapper->WndSizeChange_ArrAlign(pImg_buf, sInspAlgo.m_eAlgoType, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY
						, ptrAlignRes, inspType, sInspAlgo, nAlignCnt);
				}

				nLine = __LINE__;
				// Input 2D Image 합성
				int nSideCameraIndex = m_pParamArray[nWndIndex].nInspCameraType;
				RECT rtWnd;
				rtWnd.left = dX;
				rtWnd.top = dY;
				rtWnd.right = dX + wnd_w - 1;
				rtWnd.bottom = dY + wnd_h - 1;
				pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeX = wnd_w;
				pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeY = wnd_h;

				// m_PInspWrapper->AlgoImageCompose(m_pInspBoardInfo, sInspAlgo.m_eAlgoType, sInspAlgo, pImg_buf->m_p2D->m_pData, width, height, wnd_w, wnd_h, dX, dY);
				m_PInspWrapper->NormalImageCompose(m_pInspBoardInfo, sInspAlgo.m_eAlgoType, sInspAlgo, pImg_buf, pImg_buf->m_p2D, width, height, wnd_w, wnd_h, dX, dY, nSideCameraIndex, ptrAlignRes, nAlignCnt);

				nLine = __LINE__;
				//NG Bump 합성 필요
				if (sInspAlgo.m_eAlgoType == eAlgoBGA)
				{
					AlgoBGA *pAlgoBlob = (AlgoBGA*)sInspAlgo.m_ptrInspAlgoParam;

					// 					if (pAlgoBlob->m_bUseContrast == TRUE)
					// 					{
					// 						m_PInspWrapper->SetWindowImage(m_pInspBoardInfo, pImg_buf, rtWnd);
					// 					}

					if (pAlgoBlob->m_bUseNGBump && sInspAlgo.m_nMixCount > 0)
					{
						pImg_buf->m_p2D_Mix[0] = new PIAL::PI_Buff(wnd_w, wnd_h);
						// 해당구문은 pImg_buf 소멸자(PInsp_Algo.dll)에서 해제하기 때문에 pem_new, pem_new_check를 사용하면 안됌.
						// g_pMManager->pem_new_check(pWnd_buf->m_P2D_Mix[0], (PCHAR)__FUNCTION__, __LINE__);						
						m_PInspWrapper->AlgoImageMixCompose(m_pInspBoardInfo, sInspAlgo.InspAlgoLightsMix[0], pImg_buf->m_p2D_Mix[0]->m_pData, width, height, wnd_w, wnd_h, dX, dY);
					}
					nLine = __LINE__;
					if (pAlgoBlob->m_bUseUncoining && sInspAlgo.m_nMixCount > 1)
					{
						pImg_buf->m_p2D_Mix[1] = new PIAL::PI_Buff(wnd_w, wnd_h);
						// 해당구문은 pImg_buf 소멸자(PInsp_Algo.dll)에서 해제하기 때문에 pem_new, pem_new_check를 사용하면 안됌.
						// g_pMManager->pem_new_check(pWnd_buf->m_P2D_Mix[1], (PCHAR)__FUNCTION__, __LINE__);
						m_PInspWrapper->AlgoImageMixCompose(m_pInspBoardInfo, sInspAlgo.InspAlgoLightsMix[1], pImg_buf->m_p2D_Mix[1]->m_pData, width, height, wnd_w, wnd_h, dX, dY);
					}
					m_PInspWrapper->SetWindowImage(m_pInspBoardInfo, pImg_buf, rtWnd);
					nLine = __LINE__;
				}
				else if (sInspAlgo.m_eAlgoType == eAlgoColor || sInspAlgo.m_eAlgoType == eAlgoNGBlob)
				{
					nLine = __LINE__;
					//color buffer set
					m_PInspWrapper->SetWindowImage(m_pInspBoardInfo, pImg_buf, rtWnd);
					m_PInspWrapper->ColorDataInput(sInspAlgo, pImg_buf, width, height, rtWnd, 0, nSideCameraIndex);
					m_PInspWrapper->GetAlgoImage(&sInspAlgo, m_pInspBoardInfo, pImg_buf, rtWnd, nSideCameraIndex);
				}
			}
			if (pImg_buf)
				pImg_buf->inspWndImage->nAlgoIndex = nAlgo;
			WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;
			WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsRequired = sInspAlgo.m_bIsRequired;
			switch (sInspAlgo.m_eAlgoType)
			{
			case eAlgoBGA:
			{
				nLine = __LINE__;
				// WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBGA;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoBGA>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
				AlgoBGA *pAlgoBlob = (AlgoBGA*)sInspAlgo.m_ptrInspAlgoParam;
				//Color
				PIAL::_TotalInspExceptArea stTieArea2;
				PIAL::_AlgoBGA algo2;
				PIAL::_RstAlgoBGA rstAlgo2;

				m_PInspWrapper->ConvertAlgo(pAlgoBlob, algo2);
				InspAreaType = algo2.m_InspAreaType; // Area/Coining algo2 의 검사단 안쪽에서 m_InspAreaType 변수가 어디서 쓰이는지 봐야함.
				bUseNGBump = algo2.m_bUseNGBump;	 // NGBump
				bUseUnCoining = algo2.m_bUseUnCoining;
				nBallMaskSize = pAlgoBlob->m_nBallMaskSize; // Ball Mask Size

				pPreImgDst = new PIAL::PI_Buff(pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY);
				g_pMManager->pem_new_check(pPreImgDst, (PCHAR)__FUNCTION__, __LINE__);

				pForMakingmaskimg = pPreImgDst->Clone();
				g_pMManager->pem_new_check(pForMakingmaskimg, (PCHAR)__FUNCTION__, __LINE__);

				if (algo2.m_InspAreaType == (int)m_eTypeBGA::eCoining)		//1
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBGA_FlatBall(algo2, *pImg_buf, &rstAlgo2, stTieArea2, pPreImgDst, pForMakingmaskimg);
				else if (algo2.m_InspAreaType == (int)m_eTypeBGA::eBlob)		//2
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBGA_Blob(algo2, *pImg_buf, &rstAlgo2, stTieArea2, pPreImgDst, pForMakingmaskimg);
				else
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBGA(algo2, *pImg_buf, &rstAlgo2, stTieArea2, pPreImgDst, pForMakingmaskimg);

				m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoBGA*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);

				if (pBGASet)
					// delete pBGASet;
					g_pMManager->pem_delete(pBGASet, false);

				pBGASet = new PIAL::AlgoSetBGA(nAlgo, algo2, rstAlgo2);
				g_pMManager->pem_new_check(pBGASet, (PCHAR)__FUNCTION__, __LINE__);
			}
			break;
			case eAlgoBump:
			{
				nLine = __LINE__;
				//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoBump;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoBump>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

				AlgoBump *pAlgoBlob = (AlgoBump *)sInspAlgo.m_ptrInspAlgoParam;
				PIAL::_AlgoBump algo2;
				PIAL::_RstAlgoBump rstAlgo2;
				m_PInspWrapper->ConvertAlgo(pAlgoBlob, algo2);
				algo2.m_InspAreaType = InspAreaType;
				algo2.m_bUseNGBump = bUseNGBump;

				//if (algo2.m_InspAreaType == 0)
				//	algo2.m_HeightAreaType = 4; //높이추출 방식 :  GetMaxHeightPixel()

				if (algo2.m_bAreaIsUse || algo2.m_bShiftIsUse || algo2.m_bTeachLengthUse || algo2.m_bTeachWidthUse || algo2.m_bUseBlobNG || algo2.m_bUseBlobSizeLength ||
					algo2.m_bUseBlobSizeWidth || algo2.m_bUseCircleRate || algo2.m_bUseNGBump || algo2.m_bUseHeight || algo2.m_bUseUnCoining || algo2.m_bUseHeightDev || algo2.m_bUseVolume)
				{
					if (algo2.m_InspAreaType == (int)m_eTypeBGA::eCoining)		//2
						bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBump_FlatBall(algo2, *pImg_buf, &rstAlgo2, nOffX_pix, nOffY_pix, vecAlignResult, pPreImgDst);
					else if (algo2.m_InspAreaType == (int)m_eTypeBGA::eBlob)		//1
						bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBump_Blob(algo2, *pImg_buf, &rstAlgo2, nOffX_pix, nOffY_pix, vecAlignResult, pPreImgDst);
					else
						bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspBump(algo2, *pImg_buf, &rstAlgo2, nOffX_pix, nOffY_pix, vecAlignResult, pPreImgDst, pForMakingmaskimg);

				}
				else
				{
					rstAlgo2.m_bOKArea = rstAlgo2.m_bOKArea_Reverse = TRUE;
					rstAlgo2.m_bOKCircleRate = rstAlgo2.m_bOKDistance = rstAlgo2.m_bOKHeight = TRUE;
					rstAlgo2.m_bOKLength = rstAlgo2.m_bOKLength_Reverse = TRUE;
					rstAlgo2.m_bOKShiftX = rstAlgo2.m_bOKShiftY = TRUE;
					rstAlgo2.m_bOKWidth = rstAlgo2.m_bOKWidth_Reverse = TRUE;
				}


				rstAlgo2.m_poDrawCenter.x += dX;
				rstAlgo2.m_poDrawCenter.y += dY;
				rstAlgo2.m_rcRect_I.left += dX;
				rstAlgo2.m_rcRect_I.right += dX;
				rstAlgo2.m_rcRect_I.top += dY;
				rstAlgo2.m_rcRect_I.bottom += dY;
				rstAlgo2.m_rcRectT.left += dX;
				rstAlgo2.m_rcRectT.right += dX;
				rstAlgo2.m_rcRectT.top += dY;
				rstAlgo2.m_rcRectT.bottom += dY;

				vecBumpSet.push_back(PIAL::AlgoSetBump(nAlgo, algo2, rstAlgo2));

				m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
				RstAlgoBump * rst = (RstAlgoBump *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;

				if (InspDataSet && rst->fRstHeight > 0)
					InspDataSet->SetThickestPosition(rst->fRstHeight, &rst->m_rcRectI);

				/*rst->m_rcRectI.left += dX;
				rst->m_rcRectI.right += dX;
				rst->m_rcRectI.top += dY;
				rst->m_rcRectI.bottom += dY;*/
				/*rst->m_poDrawCenter.x += dX;
				rst->m_poDrawCenter.y += dY; */
				if (rst->m_iFRectCnt > BUMP_RECT_CNTS)
					rst->m_iFRectCnt = BUMP_RECT_CNTS;
				for (int i = 0; i < rst->m_iFRectCnt; ++i)
				{
					rst->m_rcArrRectF[i].left += dX;
					rst->m_rcArrRectF[i].right += dX;
					rst->m_rcArrRectF[i].top += dY;
					rst->m_rcArrRectF[i].bottom += dY;
				}

				if (pAlgoBlob->idata & Bump_Data_UseAnchor)
				{
					double CvtBoradX = 0;
					double CvtBoradY = 0;
					CvtBoradX = pAlgoBlob->farrdata[Bump_F_TechCenterX] - rst->fRstShiftX;
					CvtBoradY = pAlgoBlob->farrdata[Bump_F_TechCenterY] + rst->fRstShiftY;
					if (ptrAlignRes != NULL)
					{
						rst->fRstShiftX = CvtBoradX - ptrAlignRes->TeachCenterX;
						rst->fRstShiftY = CvtBoradY - ptrAlignRes->TeachCenterY;
					}
				}
				else if (pAlgoBlob->idata & Bump_Data_UseTwoAnchor)
				{
					//
					if (pAlgoBlob->m_sAlignWindow != _T("") && arrAlignRes != NULL)
					{
						AlignResult * ptrAlignRes2 = NULL;
						int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
						ptrAlignRes2 = g_pMManager->pem_new<AlignResult>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
						int WindowId = 0;

						CString csWindowID;
						csWindowID.Format(_T("%s"), pAlgoBlob->m_sAlignWindow);
						if (csWindowID.GetLength() == 7)
							WindowId = _ttoi(csWindowID.Right(1));
						else
							WindowId = _ttoi(csWindowID.Right(2));

						FindAlignResultData(nWndIndex, arrAlignRes, ptrAlignRes2, &nAlignCnt, WindowId);

						double CvtBoradX = 0;
						double CvtBoradY = 0;
						double CenterX = 0;
						double CenterY = 0;
						CvtBoradX = pAlgoBlob->farrdata[Bump_F_TechCenterX] - rst->fRstShiftX;
						CvtBoradY = pAlgoBlob->farrdata[Bump_F_TechCenterY] + rst->fRstShiftY;

						if (ptrAlignRes != NULL && ptrAlignRes2 != NULL)
						{
							CenterX = (ptrAlignRes->TeachCenterX + ptrAlignRes2->TeachCenterX) / 2;
							CenterY = (ptrAlignRes->TeachCenterY + ptrAlignRes2->TeachCenterY) / 2;
							rst->fRstShiftX = CvtBoradX - CenterX;
							rst->fRstShiftY = CvtBoradY - CenterY;
						}

						Delete_1DArray(&ptrAlignRes2);
					}
				}

			}
			break;
			case eAlgoBlob: //NGblob 은 무조건 여기로
			{

			}
			break;
			case eAlgoNGBlob:
			{
				nLine = __LINE__;
				AlgoNGBlob *pAlgoNGBlob = (AlgoNGBlob*)sInspAlgo.m_ptrInspAlgoParam;
				PIAL::_AlgoNGBlob algo2;
				PIAL::_RstAlgoNGBlob rstAlgo2;
				// WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoNGBlob;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoNGBlob>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
				m_PInspWrapper->ConvertAlgo(pAlgoNGBlob, algo2);
				if (pForngBlobMaskImg == nullptr)
				{
					pForngBlobMaskImg = new PIAL::PI_Buff(pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY);
					g_pMManager->pem_new_check(pForngBlobMaskImg, (PCHAR)__FUNCTION__, __LINE__);
#if _DEBUG
					cv::Mat Temp = pForngBlobMaskImg->Mat();
					cv::Mat Temp1 = pPreImgDst->Mat();
					cv::Mat Temp2 = pForMakingmaskimg->Mat();
#endif
					if (InspAreaType == 0 && pPreImgDst != nullptr)
						cv::bitwise_xor(pForMakingmaskimg->Mat(), pPreImgDst->Mat(), pForngBlobMaskImg->Mat());
					cv::bitwise_not(pForngBlobMaskImg->Mat(), pForngBlobMaskImg->Mat());

					if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::DetailWarpage)
						nBallMaskSize = -1; // Use Kovis 

					if (nBallMaskSize == -1) //default
					{
						PIAL::PAlgo::MorErode(pForngBlobMaskImg, pForngBlobMaskImg, 2, 17);
					}
					else if (nBallMaskSize == 0) {} // Pass
					else //User Size
					{
						int nksize = nBallMaskSize * 2 + 1;
						if (nksize > 35) nksize = 35;
						PIAL::PAlgo::MorErode(pForngBlobMaskImg, pForngBlobMaskImg, 2, nksize);
					}
					if (InspDataSet != nullptr)
					{
						PIAL::PI_Buff MaskImg(pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY);
						InspDataSet->SetMaskToImage(&MaskImg, vecAlignResult, InspData->GetMaskData(), false, dX, dY);
						cv::Mat Body_mask = MaskImg.Mat();
						cv::Mat mTemp = pForngBlobMaskImg->Mat();
						//cv::Mat Body_mask = PIAL::PInspDataSet::SetMaskToImage(pForngBlobMaskImg->Mat(), InspDataSet, InspDataSet->GetMaskData(), false, dX, dY);
						cv::bitwise_and(pForngBlobMaskImg->Mat(), Body_mask, pForngBlobMaskImg->Mat());
					}
					/*int ksize = 17;
					cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(ksize, ksize));
					cv::erode(pForngBlobMaskImg->Mat(), pForngBlobMaskImg->Mat(), kernel);*/
				}

				PIAL::_TotalInspExceptArea stTieArea;
				m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &stTieArea, 0);
				m_PInspWrapper->ConvertExceptROI(&stTieArea, m_pInspBoardInfo, m_pParamArray[nWndIndex], InspData);
				//pForngBlobMaskImg 은 pForngpattern 추가 후 작업.

				if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::Warpage
					&& pAlgoNGBlob->narrdata[_NGBlob_by_WarpageOption] == 2) //Ball Warpage
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob_BallWarpage(algo2, *pImg_buf, &rstAlgo2, vecBumpSet);
				else if (InspAreaType == 1 && pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == 0) //Coning 검사
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob(algo2, *pImg_buf, &rstAlgo2, pForngBlobMaskImg);
				else if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == 1)//solderball
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspExtraBump(algo2, *pImg_buf, &rstAlgo2, pPreImgDst);
				else if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::BallDamage)//
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob_BallDamage(algo2, *pImg_buf, &rstAlgo2, vecBumpSet, nullptr);
				else if (pAlgoNGBlob->narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::DetailWarpage)//
				{
					std::vector<PIAL::PInspData*> pDataSet = InspDataSet->GetInspDataSet();
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob_KOVIS(algo2, *pImg_buf, &rstAlgo2, stTieArea, vecAlignResult, pDataSet, pForngBlobMaskImg);
				}
				else if (InspDataSet != nullptr)
				{
					std::vector<PIAL::PInspData*> pDataSet = InspDataSet->GetInspDataSet();
					bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspNGBlob(algo2, *pImg_buf, &rstAlgo2, stTieArea, vecAlignResult, pDataSet, pForngBlobMaskImg, false);

				}
				if (bAlgoOK == false)
					IsOKNGBlob = false;

				m_PInspWrapper->ConvertRstAlgo(rstAlgo2, (RstAlgoNGBlob*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
				RstAlgoNGBlob * rst = (RstAlgoNGBlob *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
				if (rst->blob_count > 200)
					rst->blob_count = 200;
				for (int i = 0; i < rst->blob_count; ++i)
				{
					rst->m_rcArrRect[i].left += dX;
					rst->m_rcArrRect[i].right += dX;
					rst->m_rcArrRect[i].top += dY;
					rst->m_rcArrRect[i].bottom += dY;
				}


				if (rst->OutlineChipping[7] != 0)
				{
					rst->OutlineChipping[0] += dX;
					rst->OutlineChipping[2] += dX;
					rst->OutlineChipping[4] += dX;
					rst->OutlineChipping[6] += dX;

					rst->OutlineChipping[1] += dY;
					rst->OutlineChipping[3] += dY;
					rst->OutlineChipping[5] += dY;
					rst->OutlineChipping[7] += dY;
				}
			}
			break;
			default:
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = FALSE;
				break;
			}

			WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = bAlgoOK;

			if (sInspAlgo.m_bIsRequired && !sInspAlgo.m_bAlgoGroup && !bAlgoOK)	// 필수 검사 알고리즘이  NG이면 Window  결과  NG (단, Group이 체크되어있으면 일단 보류, 아래에서 판정)
			{
				ret = e_NG;
			}

			// 교체용 디펙트 코드
			if (!bAlgoOK && sInspAlgo.m_bUsingManualDefectCode)
			{
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bUsingManualDefectCode = sInspAlgo.m_bUsingManualDefectCode;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualDefectCode = sInspAlgo.m_nManualDefectCode;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualSubDefectCode = sInspAlgo.m_nManualSubDefectCode;
			}
			if (vpInspRstPoly)
			{
				for (size_t i = 0; i < pImg_buf->vecDSI.size(); i++)
				{
					InspRstPolyAlgo poly;
					m_PInspWrapper->ConvertAlgo(&pImg_buf->vecDSI[i], &poly);
					vpInspRstPoly->push_back(poly);
				}
			}
			if (eAlgoNum > (int)sInspAlgo.m_eAlgoType)
				m_fInspAlgoTact[(int)sInspAlgo.m_eAlgoType] += ((GetTickCount() - start) / 1000.0f);
		}

		//Bridge Bump
		if (pImg_buf)
		{
			std::set<int>::iterator iter;
			for (iter = pImg_buf->inspWndImage->m_BridgealgoIndex.begin(); iter != pImg_buf->inspWndImage->m_BridgealgoIndex.end(); ++iter)
			{
				int nAlgoIndex = (*iter);
				InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgoIndex];
				if (sInspAlgo.m_eAlgoType == eAlgoBump)
				{
					// 해당 Blob 알고리즘 결과 Bridge 처리
					RstAlgoBump* resultBlob = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlgoIndex].m_vRstInspAlgo;
					if (resultBlob) resultBlob->bIsOKBridge = FALSE;
				}
			}
		}

		/*
		//BGA Second Insp
		if (pBGASet&&InspAreaType == 1)
		{
			//Coining Second Insp
			bool bChanged = m_PInspWrapper->m_PInspAlgo->InspCoiningSecond(*pImg_buf, pBGASet, vecBumpSet);

			if (bChanged)
			{
				bool bPartOk = true;
				int nNumBump = vecBumpSet.size();

				for (size_t i = 0; i < nNumBump; i++) //Bump 결과 변경
				{
					PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
					int nAlogoIndex = bumpSet.nAlgoindex;
					if (bumpSet.bChanged)
					{
						RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;
						WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_bOk = bumpSet.bRst;
						rstBump->bIsOKUnCoining = TRUE;
						rstBump->fRstUncoining = 0;

						if (bumpSet.bRst == false) bPartOk = false;
					}
					else if (bPartOk)
					{
						if (WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_bOk == FALSE)
							bPartOk = false;
					}
				}

				if (bPartOk&&IsOKNGBlob)
					ret = e_OK;
			}
		}
		*/

		nLine = __LINE__;
		//BGA Second Insp
		if (pBGASet && (pBGASet->m_Algo.m_bUseCoplanarity
			|| pBGASet->m_Algo.m_bUsePitch || pBGASet->m_Algo.m_bUseGridOffsetX || pBGASet->m_Algo.m_bUseGridOffsetY
			|| pBGASet->m_Algo.m_bUseShiftR || pBGASet->m_Algo.m_bShiftIsUse || pBGASet->m_Algo.m_bShiftXUse || pBGASet->m_Algo.m_bShiftYUse || pBGASet->m_Algo.m_UseNormalizeBumpWidth))
			//else //coplanarity 검사 추가 
			//if (pBGASet &&  (pBGASet->m_Algo.m_bUseCoplanarity || pBGASet->m_Algo.m_SelectOffSetType > 0) )
		{
			if (pBGASet->m_Algo.m_nCoplOption == 1)
				pBGASet->m_Algo.m_nCoplOption = 2;	//Coplanarity 검사 유형 2번으로 사용 // Max Height 3 Ball..
			else
				pBGASet->m_Algo.m_nCoplOption = 4;		//Coplanarity 검사유형 4번으로 고정

			if (m_PInspWrapper->m_PInspAlgo->InspBGASecond(*pImg_buf, pBGASet, vecBumpSet))
			{
				//BGA 결과 변경
				//BGA
				RstAlgoBGA* resultBGA = (RstAlgoBGA*)WndResult->m_vArrRstInspAlgo[pBGASet->nAlgoindex].m_vRstInspAlgo;

				if (pBGASet->m_Algo.m_bUseCoplanarity == TRUE)
				{
					resultBGA->m_bCoplanarityOK = pBGASet->m_Rst.m_bOKCoplanarity;
					resultBGA->m_fCoplanarity = pBGASet->m_Rst.m_fRstCoplanarity;
					resultBGA->m_rcMinCoplanarity = pBGASet->m_Rst.m_rcMinCoplanarity;
					resultBGA->m_rcMaxCoplanarity = pBGASet->m_Rst.m_rcMaxCoplanarity;

					int nNumBump = vecBumpSet.size();
					for (size_t i = 0; i < nNumBump; i++) //Bump 결과 변경
					{
						PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
						int nAlogoIndex = bumpSet.nAlgoindex;
						RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;
						rstBump->fRstCoplanarity = bumpSet.m_Rst.m_dRstCoplanarity;
					}
				}

				if (pBGASet->m_Rst.m_bOKCoplanarity == FALSE || pBGASet->m_Rst.m_bOKGridOffsetX == FALSE || pBGASet->m_Rst.m_bOKGridOffsetY == FALSE)
				{
					ret = e_NG;
					WndResult->m_vArrRstInspAlgo[pBGASet->nAlgoindex].m_bOk = FALSE;
				}
				bool bPartOk = true;
				if (WndResult->m_vArrRstInspAlgo[pBGASet->nAlgoindex].m_bOk == FALSE)
					bPartOk = false;
				if (pBGASet->m_Algo.m_bUsePitch == TRUE)
				{
					int nNumBump = vecBumpSet.size();
					for (size_t i = 0; i < nNumBump; i++) //Bump 결과 변경
					{
						PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
						int nAlogoIndex = bumpSet.nAlgoindex;
						RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;
						rstBump->bIsOKPitch = bumpSet.m_Rst.m_bOKPitch;
						rstBump->fRstPitch = bumpSet.m_Rst.m_dRstPitch;

						if (rstBump->bIsOKPitch == FALSE)
						{
							WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_bOk = FALSE;
							bPartOk = false;
						}

					}

				}

				//Shift 변경
				if (pBGASet->m_Algo.m_bUseShiftR || pBGASet->m_Algo.m_bShiftIsUse || pBGASet->m_Algo.m_bShiftXUse || pBGASet->m_Algo.m_bShiftYUse || pBGASet->m_Algo.m_UseNormalizeBumpWidth)
				{
					int nNumBump = vecBumpSet.size();
					for (size_t i = 0; i < nNumBump; i++) //Bump 결과 변경
					{
						PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
						int nAlogoIndex = bumpSet.nAlgoindex;
						RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;
						rstBump->fRstShiftX = bumpSet.m_Rst.m_dRstShiftX;
						rstBump->fRstShiftY = bumpSet.m_Rst.m_dRstShiftY;
						rstBump->fRstShiftR = bumpSet.m_Rst.m_dRstShiftR;
						rstBump->fRstWidth = bumpSet.m_Rst.m_dRstWidth;
						rstBump->fRstLength = bumpSet.m_Rst.m_dRstLength;
						rstBump->m_bOKShiftX = bumpSet.m_Rst.m_bOKShiftX;
						rstBump->m_bOKShiftY = bumpSet.m_Rst.m_bOKShiftY;
						rstBump->m_bOKShiftR = bumpSet.m_Rst.m_bOKShiftR;

						rstBump->bIsOKWidth = bumpSet.m_Rst.m_bOKWidth;
						rstBump->bIsOKLength = bumpSet.m_Rst.m_bOKLength;
						rstBump->bIsOKPitch = bumpSet.m_Rst.m_bOKPitch;
						rstBump->m_rcRectT = bumpSet.m_Rst.m_rcRectT;
						if (rstBump->m_bOKShiftX == FALSE || rstBump->m_bOKShiftY == FALSE || rstBump->m_bOKShiftR == FALSE || rstBump->bIsOKArea == FALSE ||
							rstBump->bIsOKHeight == FALSE || rstBump->bIsOKNGBumpW == FALSE || rstBump->bIsOKNGBumpL == FALSE || rstBump->bIsOKUnCoining == FALSE ||
							rstBump->bIsOKWidth == FALSE || rstBump->bIsOKLength == FALSE || rstBump->bIsOKCircleRate == FALSE || rstBump->bIsOKBridge == FALSE ||
							rstBump->bIsOKPitch == FALSE || rstBump->bIsOKContrast == FALSE || rstBump->bIsOKHeightDev == FALSE)
						{
							WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_bOk = FALSE;
							bPartOk = false;
						}
						else
							WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_bOk = TRUE;

					}
				}
				else
				{
					int nNumBump = vecBumpSet.size();
					for (size_t i = 0; i < nNumBump; i++) //Bump 결과 변경
					{
						PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
						int nAlogoIndex = bumpSet.nAlgoindex;
						RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;

						if (rstBump->m_bOKShiftX == FALSE || rstBump->m_bOKShiftY == FALSE || rstBump->m_bOKShiftR == FALSE || rstBump->bIsOKArea == FALSE ||
							rstBump->bIsOKHeight == FALSE || rstBump->bIsOKNGBumpW == FALSE || rstBump->bIsOKNGBumpL == FALSE || rstBump->bIsOKUnCoining == FALSE ||
							rstBump->bIsOKWidth == FALSE || rstBump->bIsOKLength == FALSE || rstBump->bIsOKCircleRate == FALSE || rstBump->bIsOKBridge == FALSE ||
							rstBump->bIsOKPitch == FALSE || rstBump->bIsOKContrast == FALSE || rstBump->bIsOKHeightDev == FALSE)
						{
							WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_bOk = FALSE;
							bPartOk = false;
						}
						else
							WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_bOk = TRUE;

					}
				}

				if (bPartOk == false)
					ret = e_NG;
				else if (bPartOk && IsOKNGBlob && IsThicknessOK)
					ret = e_OK;

			}
		}

		nLine = __LINE__;
		if (pBGASet && pBGASet->m_Algo.m_bUseHeight)
		{
			float fMinBumpHeight = 1000, fMaxBumpHeight = 0;
			float fAvgBumpHeight = 0, fSumBumpHeight = 0;

			RstAlgoBGA* resultAlgo_BGA = (RstAlgoBGA*)WndResult->m_vArrRstInspAlgo[pBGASet->nAlgoindex].m_vRstInspAlgo;

			int nCntBumpAlgo = vecBumpSet.size();
			for (size_t i = 0; i < nCntBumpAlgo; i++) //Bump 결과 변경
			{
				PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
				int nAlogoIndex = bumpSet.nAlgoindex;
				RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;

				if (fMinBumpHeight > bumpSet.m_Rst.m_dRstHeightMean)
					fMinBumpHeight = bumpSet.m_Rst.m_dRstHeightMean;

				if (fMaxBumpHeight < bumpSet.m_Rst.m_dRstHeightMean)
					fMaxBumpHeight = bumpSet.m_Rst.m_dRstHeightMean;

				fSumBumpHeight = fSumBumpHeight + bumpSet.m_Rst.m_dRstHeightMean;

			}

			if (nCntBumpAlgo != 0 && fSumBumpHeight != 0)
				fAvgBumpHeight = fSumBumpHeight / nCntBumpAlgo;

			if (fMinBumpHeight == 1000)
				fMinBumpHeight = 0;

			resultAlgo_BGA->m_fArrRstHeight[(int)m_eMMA::eMMA_Average] = fAvgBumpHeight;
			resultAlgo_BGA->m_fArrRstHeight[(int)m_eMMA::eMMA_Min] = fMinBumpHeight;
			resultAlgo_BGA->m_fArrRstHeight[(int)m_eMMA::eMMA_Max] = fMaxBumpHeight;

			if (pBGASet->m_Algo.m_bUseCoplanarity)
			{
				float fMinCoplan = 1000, fMaxCoplan = 0;
				float fAvgCoplan = 0, fSumCoplan = 0;

				int nCntBumpAlgo = vecBumpSet.size();
				for (size_t i = 0; i < nCntBumpAlgo; i++) //Bump 결과 변경
				{
					PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
					int nAlogoIndex = bumpSet.nAlgoindex;
					RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;

					if (fMinCoplan > bumpSet.m_Rst.m_dRstCoplanarity)
						fMinCoplan = bumpSet.m_Rst.m_dRstCoplanarity;

					if (fMaxCoplan < bumpSet.m_Rst.m_dRstCoplanarity)
						fMaxCoplan = bumpSet.m_Rst.m_dRstCoplanarity;

					fSumCoplan = fSumCoplan + bumpSet.m_Rst.m_dRstCoplanarity;
				}

				if (fMinCoplan == 1000)
					fMinCoplan = 0;

				if (nCntBumpAlgo != 0 && fSumCoplan != 0)
					fAvgCoplan = fSumCoplan / nCntBumpAlgo;

				//Coplanarity 통계값 저장
				resultAlgo_BGA->m_fArrRstCoplan[(int)m_eMMA::eMMA_Average] = fAvgCoplan;
				resultAlgo_BGA->m_fArrRstCoplan[(int)m_eMMA::eMMA_Min] = fMinCoplan;
				resultAlgo_BGA->m_fArrRstCoplan[(int)m_eMMA::eMMA_Max] = fMaxCoplan;
			}
		}

		nLine = __LINE__;
		if (pBGASet && pBGASet->m_Algo.m_bTeachWidthUse)
		{
			RstAlgoBGA* resultAlgo_BGA = (RstAlgoBGA*)WndResult->m_vArrRstInspAlgo[pBGASet->nAlgoindex].m_vRstInspAlgo;

			float fMinWidth = 1000, fMaxWidth = 0;
			float fAvgWidth = 0, fSumWidth = 0;

			int nCntBumpAlgo = vecBumpSet.size();
			for (size_t i = 0; i < nCntBumpAlgo; i++) //Bump 결과 변경
			{
				PIAL::AlgoSetBump& bumpSet = vecBumpSet[i];
				int nAlogoIndex = bumpSet.nAlgoindex;
				RstAlgoBump* rstBump = (RstAlgoBump*)WndResult->m_vArrRstInspAlgo[nAlogoIndex].m_vRstInspAlgo;

				if (fMinWidth > bumpSet.m_Rst.m_dRstWidth)
					fMinWidth = bumpSet.m_Rst.m_dRstWidth;

				if (fMaxWidth < bumpSet.m_Rst.m_dRstWidth)
					fMaxWidth = bumpSet.m_Rst.m_dRstWidth;

				fSumWidth = fSumWidth + bumpSet.m_Rst.m_dRstWidth;
			}

			if (fMinWidth == 1000)
				fMinWidth = 0;

			if (nCntBumpAlgo != 0 && fSumWidth != 0)
				fAvgWidth = fSumWidth / nCntBumpAlgo;

			resultAlgo_BGA->m_fArrRstWidth[(int)m_eMMA::eMMA_Average] = fAvgWidth;
			resultAlgo_BGA->m_fArrRstWidth[(int)m_eMMA::eMMA_Min] = fMinWidth;
			resultAlgo_BGA->m_fArrRstWidth[(int)m_eMMA::eMMA_Max] = fMaxWidth;
		}

		BOOL bIsInsp = TRUE;
		if (ret == e_OK)
		{

			nLine = __LINE__;

			int nResGroup = e_NG;		// Group이 아닌 알고리즘들의 결과
			int nResNotGroup = e_NG;		// Group인 알고리즘들의 결과
			BOOL bIsEssentialInGroup = FALSE;	// Group에 포함된 알고리즘 중 Essential check된 것이 있는지 여부
			bIsInsp = FALSE;
			if (m_pParamArray[nWndIndex].nAlgorithmCnt > 0)
				ret = e_NG;		// 일단 NG로,
			for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
			{
				if (!WndResult->m_vArrRstInspAlgo)
					continue;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Search || WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Tip
					|| WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoTab_Search)
				{
					if (m_pParamArray[nWndIndex].nAlgorithmCnt == 1)
						ret = e_OK;	// LeadSearch
					continue;
				}
				// 알고리즘 간 기본 OR 처리 (여기 들어왔다는 것은 필수 검사는 모두 OK 인 경우이므로 고려하지 않아도 됨)
				// 2015/08/10 : 알고리즘 Group flag 추가되어서 필수 검사 NG 여부도 여기서 체크해야함

				nLine = __LINE__;

				InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
				if (sInspAlgo.m_bAlgoGroup && sInspAlgo.m_bIsRequired)
					bIsEssentialInGroup = TRUE;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk == TRUE)
				{
					if (!sInspAlgo.m_bAlgoGroup)
						nResNotGroup = e_OK;		// Group Enable = false인 알고리즘 결과가 하나라도 OK라면 OK
					else
						nResGroup = e_OK;		// Group Enable = true인 알고리즘 결과가 하나라도 OK라면 OK
				}

				bIsInsp = TRUE;
			}

			nLine = __LINE__;

			if (nResGroup == e_NG && bIsEssentialInGroup)		// Group의 결과가 NG이고 Group에 포함된 알고리즘 중 Essential check가 있으면 무조건 NG
				ret = e_NG;
			else if (nResNotGroup == e_OK || nResGroup == e_OK)	// 그렇지 않고, Group이 아닌 알고리즘 중 OK가 있거나, Group인 알고리즘 중 OK가 있으면 OK
				ret = e_OK;
		}

		nLine = __LINE__;
		WndResult->m_bOk = ret == true ? TRUE : FALSE;
		WndResult->m_bIsInsp = bIsInsp;
		if (pBGASet != nullptr) g_pMManager->pem_delete(pBGASet, false);
		if (pForngBlobMaskImg != nullptr) /*delete pForngBlobMaskImg*/ g_pMManager->pem_delete(pForngBlobMaskImg, false);
		if (pForMakingmaskimg != nullptr) /*delete pForMakingmaskimg*/g_pMManager->pem_delete(pForMakingmaskimg, false);
		if (pPreImgDst != nullptr) /*delete pPreImgDst*/g_pMManager->pem_delete(pPreImgDst, false);
		if (pImg_buf != nullptr) /*delete pImg_buf*/g_pMManager->pem_delete(pImg_buf, false);
		Delete_1DArray(&ptrAlignRes);
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CMInspManager::InspBGAWindowAlgo(), Line : %d Pass!!! WndType : %d, WinID : %d, AlgoType : %d, AlgoID : %d"), nLine, inspType, nWndIndex, nAlgoType, nAlgoID);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}

	return ret;
}
int InspManager::GetInspectionResult(InspectionResult* retResult)
{
	if (m_inspectionResult == NULL /*|| m_isSingleInsp == TRUE*/)
		return eIMSG_FAIL;

	*retResult = *m_inspectionResult;

	return eIMSG_SUCCESE;
}

void InspManager::FindAlignResult(AlignResult*& dst, AlignResult* org, int nWndIndex, int nAlignWndID, int nAlignWndIdx, int* nAlignCnt)
{
	if ((nAlignWndID != 0) && org != NULL && nAlignWndIdx > 0)
	{
		int nSize = _msize(org) / sizeof(org[0]);
		dst = new AlignResult[nSize];
		
		for (int a = 0; a < nSize; a++)
			dst[a].nWindowID = -1;
;
		bool bFind = FindAlignResultData(nWndIndex, org, dst, nAlignCnt);
		if (bFind == false)
		{
			delete dst;
			dst = nullptr;
		}
	}
}

int InspManager::FindAlignResultData(int nWndIndex, AlignResult* arrAlignRes, AlignResult* curAlignResult, int* ptrnTotalCnt, int WindowId)
{
	int ret = e_NG;
	int nLine = __LINE__;
	try
	{
		int nWndID = m_pParamArray[nWndIndex].wndIndex;
		int nAlignID = m_pParamArray[nWndIndex].nAlignWndID;
		if (WindowId != 0)
			nAlignID = WindowId;

		if (arrAlignRes == NULL || nAlignID == 0)
			return ret;
		nLine = __LINE__;
		int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
		int* ptrnArrWndIndex = new int[nSize];
		AlignResult * arrAlignResBuf = new AlignResult[nSize];
		// int* ptrnArrWndIndex = g_pMManager->pem_new<int>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
		// AlignResult * arrAlignResBuf = g_pMManager->pem_new<AlignResult>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(ptrnArrWndIndex, 0, nSize * sizeof(int));
		memset(arrAlignResBuf, 0, nSize * sizeof(AlignResult));
		int nTotalWndCnt = m_inspItemCnts[eINSP_MOUNT] + m_inspItemCnts[eINSP_ALIGN] + m_inspItemCnts[eINSP_OCR]
			+ m_inspItemCnts[eINSP_LEADSOLDER] + m_inspItemCnts[eINSP_SOLDER] + m_inspItemCnts[eINSP_TAB]
			+ m_inspItemCnts[eINSP_S_BALL] + m_inspItemCnts[eINSP_PAD];
		int nIndex = 0;
		nLine = __LINE__;
		for (int a = 0; a < nSize; a++)
		{
			if (nAlignID == arrAlignRes[a].nWindowID)
			{
				arrAlignResBuf[nIndex].nWindowID = arrAlignRes[a].nWindowID;
				arrAlignResBuf[nIndex].centerX = arrAlignRes[a].centerX;
				arrAlignResBuf[nIndex].centerY = arrAlignRes[a].centerY;
				arrAlignResBuf[nIndex].offsetX = arrAlignRes[a].offsetX;
				arrAlignResBuf[nIndex].offsetY = arrAlignRes[a].offsetY;
				arrAlignResBuf[nIndex].TeachCenterX = arrAlignRes[a].TeachCenterX;
				arrAlignResBuf[nIndex].TeachCenterY = arrAlignRes[a].TeachCenterY;
				if (ABS(arrAlignRes[a].theta) > 90)	// theta�� 90���� ũ�� Align���� �ǹ� ���� ������ ������ (Pattern�˰���?� ���׶� ����� 180���� ������ ��찡 �־ �̷��� ��ġ��)
					arrAlignResBuf[nIndex].theta = 0;
				else
					arrAlignResBuf[nIndex].theta = arrAlignRes[a].theta;
				arrAlignResBuf[nIndex].rcBodyRect = arrAlignRes[a].rcBodyRect;
				ptrnArrWndIndex[nIndex] = a;
				nIndex++;
				break;
			}
		}
		nLine = __LINE__;
		if (nIndex == 0)
		{
			nLine = __LINE__;
			Delete_1DArray(&arrAlignResBuf);
			Delete_1DArray(&ptrnArrWndIndex);
			return ret;
		}
		nLine = __LINE__;
		// ���븦 ã�Ƽ�
		int nWndIdx = 0;
		for (int a = 0; a < nTotalWndCnt; a++)
		{
			if (m_pParamArray[a].wndIndex == nAlignID)
			{
				nWndIdx = a;
				break;
			}
		}
		nLine = __LINE__;
		for (int a = 0; a < nTotalWndCnt; a++)
		{
			if (m_pParamArray[a].wndIndex == nWndID)
				continue;
			bool bContinue = false;
			for (int b = 0; b < nIndex; b++)
			{
				int nIdx = ptrnArrWndIndex[b];
				if (m_pParamArray[a].wndIndex == m_pParamArray[nIdx].wndIndex)
				{
					bContinue = true;
					break;
				}
			}
			if (bContinue == true)
				continue;

			int nFindWndID = m_pParamArray[nWndIdx].wndIndex;
			int nFindAlignID = m_pParamArray[nWndIdx].nAlignWndID;
			int nAlgoCnt = m_pParamArray[nWndIdx].nAlgorithmCnt;
			if (nFindAlignID == 0 || nFindAlignID == nFindWndID)
				break;

			if (m_pInspBoardInfo != nullptr/* && !m_pInspBoardInfo->SemiConductor*/)
			{
				bool bInputData = true;
				for (int b = 0; b < nAlgoCnt; b++)
				{
					if (m_pParamArray[nWndIdx].vArrAlgoParam[b].m_eAlgoType == eAlgoBody_Blob)
					{
						bInputData = false;
						break;
					}
				}
				if (bInputData == false)
					break;
			}

			for (int b = 0; b < nSize; b++)
			{
				if (nFindAlignID == arrAlignRes[b].nWindowID)
				{
					bool bInput = true;
					for (int c = 0; c < nIndex; c++)
					{
						if (arrAlignResBuf[c].nWindowID == arrAlignRes[b].nWindowID)
						{
							bInput = false;
							break;
						}
					}
					if (bInput == true)
					{
						arrAlignResBuf[nIndex].nWindowID = arrAlignRes[b].nWindowID;
						arrAlignResBuf[nIndex].centerX = arrAlignRes[b].centerX;
						arrAlignResBuf[nIndex].centerY = arrAlignRes[b].centerY;
						arrAlignResBuf[nIndex].offsetX = arrAlignRes[b].offsetX;
						arrAlignResBuf[nIndex].offsetY = arrAlignRes[b].offsetY;
						arrAlignResBuf[nIndex].theta = arrAlignRes[b].theta;
						arrAlignResBuf[nIndex].rcBodyRect = arrAlignRes[b].rcBodyRect;
						arrAlignResBuf[nIndex].TeachCenterX = arrAlignRes[a].TeachCenterX;
						arrAlignResBuf[nIndex].TeachCenterY = arrAlignRes[a].TeachCenterY;
						ptrnArrWndIndex[nIndex] = a;
						nIndex++;
						break;
					}
				}
			}

			nWndIdx = a;
		}
		nLine = __LINE__;
		//
		*ptrnTotalCnt = nIndex;
		for (int a = 0; a < nIndex; a++)
		{
			int a_Buf = nIndex - 1 - a;
			curAlignResult[a].nWindowID = arrAlignResBuf[a_Buf].nWindowID;
			curAlignResult[a].centerX = arrAlignResBuf[a_Buf].centerX;
			curAlignResult[a].centerY = arrAlignResBuf[a_Buf].centerY;
			curAlignResult[a].offsetX = arrAlignResBuf[a_Buf].offsetX;
			curAlignResult[a].offsetY = arrAlignResBuf[a_Buf].offsetY;
			curAlignResult[a].theta = arrAlignResBuf[a_Buf].theta;
			curAlignResult[a].rcBodyRect = arrAlignResBuf[a_Buf].rcBodyRect;
			curAlignResult[a].TeachCenterX = arrAlignResBuf[a].TeachCenterX;
			curAlignResult[a].TeachCenterY = arrAlignResBuf[a].TeachCenterY;
		}

		nLine = __LINE__;
		Delete_1DArray(&arrAlignResBuf);
		Delete_1DArray(&ptrnArrWndIndex);
		ret = e_OK;
	}
	catch (...)
	{
		CString sLog;
		sLog.Format(_T("InspManager::FindAlignResultData, Line : %d Pass!!!"), nLine);
		TRACE(sLog);
		throw nLine;
	}
	return ret;
}


void InspManager::PreviousInspWireShort()
{
	m_NumWireAlgo = 0;
	CPInsp_WireBonding* PInspWireBonding = g_pInspMng->GetWireBondingInsp();
	PInspWireBonding->ClearWirePoints();
	m_NumWireAlgo = 0;
	if (m_puImgWireInterval)
	{
	 delete[] m_puImgWireInterval;
		m_puImgWireInterval = nullptr;
	}
	if (m_pParamArray)
	{
		for (int n = 0; n < m_ParamArraySize; n++)
		{
			for (int i = 0; i < m_pParamArray[n].nAlgorithmCnt; i++)
			{
				if (m_pParamArray[n].vArrAlgoParam[i].m_eAlgoType == eAlgoWire)
					m_NumWireAlgo++;/*= m_pParamArray[n].InspWire*/;
			}


		}

	}
	if (m_NumWireAlgo > 1)
	{
		//Wire Short 검사관련 Interval 이미지 생성
		m_puImgWireInterval = new UCHAR[m_pInspBoardInfo->partImgBuf.nImageSizeX * m_pInspBoardInfo->partImgBuf.nImageSizeY];
		memset(m_puImgWireInterval, 0, m_pInspBoardInfo->partImgBuf.nImageSizeX * m_pInspBoardInfo->partImgBuf.nImageSizeY);
	}
}

void InspManager::InspWireShort()
{
	WireRst* arrWire = new WireRst[m_NumWireAlgo];
	int nWireindex = 0;

	int wndStartIndex = 0;
	int groupCnt = m_groupIndexCnts[eINSP_MOUNT];
	InspAlgo* sInspWireAlgo = nullptr;

	for (int i = 0; i < groupCnt; i++)
	{
		int groupIndex = m_groupIndex[eINSP_MOUNT][i];
		int wndId = 0;
		for (int j = 0; j < m_groupWndCnts[groupIndex]; j++)
		{
			wndId = m_groupWndID[groupIndex][j];
			InspWndResult* WndResult = &m_inspectionResult->mountResult[j + wndStartIndex];
			for (int nAlgo = 0; nAlgo < m_pParamArray[wndId].nAlgorithmCnt; nAlgo++)
			{
				InspAlgo sInspAlgo = m_pParamArray[wndId].vArrAlgoParam[nAlgo];

				if (sInspWireAlgo == nullptr)
					sInspWireAlgo = &sInspAlgo;

				if (sInspAlgo.m_eAlgoType == eAlgoWire)
				{
					AlgoWire *pAlgo = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
					RstAlgoWire *pRst = (RstAlgoWire *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;

					arrWire[nWireindex].m_Wire = pAlgo;
					arrWire[nWireindex].m_RstWire = pRst;
					nWireindex++;
				}

			}
		}
		wndStartIndex += m_groupWndCnts[groupIndex];
	}

	////Wire Shosrt 검사
	int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;

	/*if (m_puImgWireInterval != nullptr)
		m_PInspWrapper->InspWireShort(m_NumWireAlgo, arrWire, m_puImgWireInterval, width, height);*/


	//wndImg.Destroy();
	delete[] arrWire;


	if (m_puImgWireInterval)
	{
		int nWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
		int nLength = m_pInspBoardInfo->partImgBuf.nImageSizeY;
		PIAL::PI_Buff BuffWireInterval(m_puImgWireInterval, nWidth, nLength, nWidth);

		PIAL::PAlgo::SaveWorkImg(&BuffWireInterval, _T("D:\\PInspAlgo\\PAlgo_Wire\\Intervel.bmp"), true);
	}
}
//
//BOOL MPTI_DrawNGBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob) // LSJ
//{
//	BOOL bRet = FALSE;
//	if (g_pInspMng == NULL)
//		return bRet;
//	CPInsp_Algo &PInspAlgo = g_pInspMng->GetInspAlgo();
//
//	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL && g_pMPTI->m_bSideOriginalSize == false) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
//		return bRet;
//
//	int WndImgCV = eMR_FAIL;
//	if (sInspAlgo.m_nMixCount == 0)
//		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
//	else
//		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV_MixImg(&sWndAlgoImg);
//
//	if (WndImgCV == eMR_FAIL)
//		return bRet;
//
//	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
//	int nStartX, nStartY;
//	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
//	{
//		if (sWndClipAlgo)
//		{
//			delete  sWndClipAlgo;
//			sWndClipAlgo = NULL;
//		}
//		return eMR_FAIL;
//	}
//
//	AlgoCoordinate sAlgoCoodi;
//	if (PInspAlgo.GetCoordinate(sWndAlgoImg, sWndInfo, &sAlgoCoodi) == false)
//	{
//		if (sWndClipAlgo)
//		{
//			delete  sWndClipAlgo;
//			sWndClipAlgo = NULL;
//		}
//		return eMR_FAIL;
//	}
//
//	if (ptrAlgoColorOpt.m_sFovImg.nImageSizeX > 0 && ptrAlgoColorOpt.m_sFovImg.nImageSizeY > 0)
//	{
//		if (PInspAlgo.WndAlgoImg_Update_fromCV_Color(&ptrAlgoColorOpt, false) == eMR_FAIL)
//		{
//			ptrAlgoColorOpt.Destroy();
//			return bRet;
//		}
//	}
//
//	AlgoNGBlob *pAlgoNGBlob = (AlgoNGBlob *)ExtPrmPtr(sInspAlgo.m_ptrInspAlgoParam);
//
//	PIAL::Insp_Wnd* pWnd_buf = new PIAL::Insp_Wnd;
//	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
//	PIAL::_TotalInspExceptArea stTieArea2;
//	PIAL::_AlgoNGBlob algo2;
//	PInspAlgoWrapper* pInspWarpper = g_pInspMng->GePInspWrapper();
//	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pWnd_buf);
//	pInspWarpper->ConvertAlgo(pAlgoNGBlob, algo2);
//	PIAL::PI_Buff* puImgDst = NULL;
//	if (pUcImgBlob)
//		puImgDst = new PIAL::PI_Buff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);// sWndAlgoImg.m_nHeight
//	pWnd_buf->nStartX = nStartX;
//	pWnd_buf->nStartY = nStartY;
//	pInspWarpper->m_PInspAlgo->DrawNGBlob(algo2, *pWnd_buf, puImgDst);
//
//	UCHAR* pbuffer = new UCHAR[sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight];
//	memset(pbuffer, 0, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
//	memcpy(pbuffer, puImgDst->m_pData, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
//
//	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, pbuffer, pUcImgBlob);
//
//	delete pWnd_buf;
//	delete pwnd_buf_ROI;
//
//	ptrAlgoColorOpt.Destroy();
//
//	if (puImgDst)
//		delete puImgDst;
//
//	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
//	//LJH
//	for (int iLoopCount = sInspAlgo.m_nMixCount; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
//		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
//	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
//	if (sWndClipAlgo)
//	{
//		delete  sWndClipAlgo;
//		sWndClipAlgo = NULL;
//	}
//}

// 윈도우 알고리즘 검사 함수
int _nOCRAlgoCnt = 0;
int m_nLeadTipPosIndex = 0;
int InspManager::InspWindowAlgo2(int wndType, int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, InspAlgoTempResult * ptrInspAlgoTempResult, bool bLeadInsp)
{
	int ret = e_OK;

	int inspType = 0;

	int nLine = __LINE__;
	try
	{
		inspType = m_pParamArray[nWndIndex].inspType;

		nLine = __LINE__;

		BOOL bAlignResOK = FALSE;
		int nAlignCnt = 0;
		AlignResult * ptrAlignRes = NULL;
		try
		{
			if ((m_pParamArray[nWndIndex].nAlignWndID != 0) && arrAlignRes != NULL && nAlignWndIdx > 0)
			{
				int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
				//ptrAlignRes = new AlignResult[nSize];
				ptrAlignRes = g_pMManager->pem_new<AlignResult>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
				for (int a = 0; a < nSize; a++)
				{
					ptrAlignRes[a].nWindowID = -1;
				}

				bool bFind = FindAlignResultData(nWndIndex, arrAlignRes, ptrAlignRes, &nAlignCnt);
				if (bFind == false)
				{
					Delete_1DArray(&ptrAlignRes);
				}
			}
		}
		catch (...)
		{
			Delete_1DArray(&ptrAlignRes);
		}

		BOOL bTabAutoSearchROI = FALSE;
		CAtlArray<CRect> rcArrTab;
		CAtlArray<CRect> rcArrTabBridge;
		try
		{
			nLine = __LINE__;

			if (inspType == eINSP_TAB)
			{
				for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
				{
					if (bTabAutoSearchROI)
						break;
					if (!m_pParamArray[nWndIndex].vArrAlgoParam || !WndResult->m_vArrRstInspAlgo)
						continue;

					InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
					if (sInspAlgo.m_eAlgoType == eAlgoTab)
					{
						AlgoColor* pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
						if (pAlgoColor && pAlgoColor->m_bAutoSearchROI)
						{
							bTabAutoSearchROI = TRUE;
							break;
						}
					}
					else if (sInspAlgo.m_eAlgoType == eAlgoBridge)
					{
						AlgoBridge* pAlgoBridge = (AlgoBridge *)sInspAlgo.m_ptrInspAlgoParam;
						if (pAlgoBridge && pAlgoBridge->m_bAutoSearchROI)
						{
							bTabAutoSearchROI = TRUE;
							break;
						}
					}
				}
			}
		}
		catch (CMemoryException* e)
		{

		}
		catch (CFileException* e)
		{
		}
		catch (CException* e)
		{
		}
		nLine = __LINE__;

		AlgoCoordinate coordinateAlgo;
		WndInfo sWndInfo;
		WndAlgoImg wndImg;
		int wnd_w = 0;
		int wnd_h = 0;
		double dX = 0.0;
		double dY = 0.0;
		int nOffX_pix = 0;
		int nOffY_pix = 0;

		CString dOCRPath = _T("");
		CString dOCRCopyPath = _T("");

		int nLeadTipPos = -1;
		BOOL bUseLeadTipSideOption = FALSE;
		double dLeadTipShiftX = -1;
		double dLeadTipShiftY = -1;
		double dLeadTipSize = -1;
		int nLeadTipDir = -1;
		double OCRNGScore = 0.0;
		double OCROKScore = 0.0;
		BOOL USENGOCR = FALSE;
		int nWndDir = e_LEFT;
		BOOL bUSeLeadAlign = FALSE;
		int *ptrnArrLeadTipRst = NULL;
		int nLeadTipTotalCnt = 0;
		int nInspLeadTipCnt = 0;
		POINTF poBodyCenter;
		poBodyCenter.x = -1;
		poBodyCenter.y = -1;
		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
		{
			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
			if (sInspAlgo.m_eAlgoType == eAlgoLead_Tip)
				nLeadTipTotalCnt++;
		}
		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
		{
			DWORD start = GetTickCount();
			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
			if (sInspAlgo.m_eAlgoType == eAlgoPattern)
			{
				m_pParamArray[nWndIndex].InspWire = 0;
			}
			if (inspType == eINSP_LEADSOLDER && ptrInspAlgoTempResult == NULL)
			{
				if (sInspAlgo.m_eAlgoType != eAlgoAlign && sInspAlgo.m_eAlgoType != eAlgoAlignEdge)
					continue;
				else
					bUSeLeadAlign = TRUE;
			}

			nLine = __LINE__;

			if (bUSeLeadAlign == FALSE && (!m_pParamArray[nWndIndex].vArrAlgoParam || !WndResult->m_vArrRstInspAlgo))
				continue;

			nLine = __LINE__;

			if (bUSeLeadAlign == FALSE)
			{
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = sInspAlgo.m_eAlgoType;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;
			}

			nLine = __LINE__;

			wndImg.Destroy();

			nLine = __LINE__;
			//CQTimer qtm; 
			//qtm.StartTick();

			//WndAlgoImg 구조체 생성
 			WndSizeChange(sInspAlgo.m_eAlgoType, coordinateAlgo, wndImg,
 				nWndIndex, wnd_w, wnd_h, dX, dY, ptrAlignRes, inspType, sInspAlgo,
 				bUseLeadTipSideOption, dLeadTipShiftX, dLeadTipShiftY, dLeadTipSize, nLeadTipDir, nAlignCnt, nOffX_pix, nOffY_pix);


			//아래는 원래 구문
			sWndInfo.dCenterX = coordinateAlgo.dROICenterX;
			sWndInfo.dCenterY = coordinateAlgo.dROICenterY;
			sWndInfo.dWidth = RounD(coordinateAlgo.dROIWidth);
			sWndInfo.dLength = RounD(coordinateAlgo.dROILength);

			nLine = __LINE__;

			// Input 2D Image 합성
			int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
			int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;
			UCHAR * ImgR = NULL, *ImgG = NULL, *ImgB = NULL, *ImgW = NULL;
			LightTypeBuf sLightBuf;
			ImgW = m_pInspBoardInfo->partImgBuf.imgTop_W;
			int nInspCameraType = m_pParamArray[nWndIndex].nInspCameraType;
			wndImg.m_nLight_index = nInspCameraType;

			// 통합 검사/제외 영역 생성
			TotalInspExceptArea stTieArea;
			stTieArea.m_nUsedWndPolygon = m_pParamArray[nWndIndex].m_nUsedWndPolygon;
			for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
				stTieArea.m_ptArrWndPolygon[i] = m_pParamArray[nWndIndex].m_ptArrWndPolygon[i];

			if (width > height)
			{
				if (width / 2 > dX + (wnd_w / 2))
					nWndDir = e_LEFT;
				else
					nWndDir = e_RIGHT;
			}
			else
			{
				if (height / 2 > dY + (wnd_h / 2))
					nWndDir = e_TOP;
				else
					nWndDir = e_BOTTOM;
			}
			switch (sInspAlgo.m_eLightType)
			{
			case Top_Light:
				ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;
				ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;
				ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;
				break;
			case Middle_Light:
				ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
				ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
				break;
			case Bottom_Light:
// 				if (g_pMPTI->m_nCameraType == Basler_Color)
// 				{
// 					ImgR = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_R;
// 					ImgB = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_G;
// 				}
				/*else*/
				{
					ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
					ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
				}
				break;
			case Side1_Light:
			case Side2_Light:
			case Side3_Light:
			case Side4_Light:
				if (nInspCameraType == eMSCN_SIDECAM1)
				{
					ImgR = m_pInspBoardInfo->partImgBuf.imgSide1_R;
					ImgG = m_pInspBoardInfo->partImgBuf.imgSide1_G;
					ImgB = m_pInspBoardInfo->partImgBuf.imgSide1_B;
				}
				else if (nInspCameraType == eMSCN_SIDECAM2)
				{
					ImgR = m_pInspBoardInfo->partImgBuf.imgSide2_R;
					ImgG = m_pInspBoardInfo->partImgBuf.imgSide2_G;
					ImgB = m_pInspBoardInfo->partImgBuf.imgSide2_B;
				}
				else if (nInspCameraType == eMSCN_SIDECAM3)
				{
					ImgR = m_pInspBoardInfo->partImgBuf.imgSide3_R;
					ImgG = m_pInspBoardInfo->partImgBuf.imgSide3_G;
					ImgB = m_pInspBoardInfo->partImgBuf.imgSide3_B;
				}
				else if (nInspCameraType == eMSCN_SIDECAM4)
				{
					ImgR = m_pInspBoardInfo->partImgBuf.imgSide4_R;
					ImgG = m_pInspBoardInfo->partImgBuf.imgSide4_G;
					ImgB = m_pInspBoardInfo->partImgBuf.imgSide4_B;
				}
				else
				{
					ImgR = NULL;
					ImgG = NULL;
					ImgB = NULL;
				}
				break;
			case  User_Light:
// 				if (g_pMPTI->m_nCameraType == Basler_Color)
// 				{
// 					sLightBuf.m_pucTRed = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_W;
// 					sLightBuf.m_pucTGreen = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgMiddle_R;
// 					sLightBuf.m_pucTBlue = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgMiddle_B;
// 					sLightBuf.m_pucTWhite = NULL;
// 					sLightBuf.m_pucMRed = NULL;
// 					sLightBuf.m_pucMGreen = NULL;
// 					sLightBuf.m_pucMBlue = NULL;
// 					sLightBuf.m_pucMWhite = NULL;
// 					sLightBuf.m_pucBRed = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_R;
// 					sLightBuf.m_pucBGreen = NULL;
// 					sLightBuf.m_pucBBlue = ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_G;
// 					sLightBuf.m_pucBWhite = NULL;
// 				}
// 				else
				{
					sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
					sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
					sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
					sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;

					sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
					sLightBuf.m_pucMGreen = NULL;
					sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
					sLightBuf.m_pucMWhite = NULL;

					sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
					sLightBuf.m_pucBGreen = NULL;
					sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
					sLightBuf.m_pucBWhite = NULL;
				}

				sLightBuf.m_nImgWidth = width;
				sLightBuf.m_nImgHeight = height;
				sLightBuf.m_nROIImgWidth = wnd_w;
				sLightBuf.m_nROIImgHeight = wnd_h;
				sLightBuf.m_dROIX = dX;
				sLightBuf.m_dROIY = dY;
				sLightBuf.m_nImgCnt = sInspAlgo.m_nLightCnt;
				sLightBuf.m_pnRedValue = sInspAlgo.m_nArrRedValue;
				sLightBuf.m_pnGreenValue = sInspAlgo.m_nArrGreenValue;
				sLightBuf.m_pnBlueValue = sInspAlgo.m_nArrBlueValue;
				sLightBuf.m_pnWhiteValue = sInspAlgo.m_nArrWhiteValue;
				sLightBuf.m_pnPosition = sInspAlgo.m_nArrLightPosition;
				sLightBuf.m_pnCalculation = sInspAlgo.m_nArrCalculation;
			}

			if (m_pInspBoardInfo->byPCBPosInspLight == 1)	// 끝단찾기 격자 합성 조명은 Top Red 에 있고 해당 이미지만 100%로 윈도우 검사 이미지 wndImg.m_ucArr2D 를 사용하기 위해서
			{
				sInspAlgo.m_nRedValue = 100;
				sInspAlgo.m_nGreenValue = 0;
				sInspAlgo.m_nBlueValue = 0;
				sInspAlgo.m_nWhiteValue = 0;
			}

			bool bUserLight = false;
			double dAlignAngle = 0;

			if ((sInspAlgo.m_eLightType == Top_Light) || (sInspAlgo.m_eLightType == Middle_Light) || (sInspAlgo.m_eLightType == Bottom_Light))
			{
				if (WindowRotate() == false)
				{
					dAlignAngle = 0;
				}
				//m_InspAlgo.RoiImageCompose(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, cx, cy, wnd_w, wnd_h, sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, sInspAlgo.m_nWhiteValue, wndImg.m_ucArr2D);
				m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h,
					sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, sInspAlgo.m_nWhiteValue,
					wndImg.m_ucArr2D, dAlignAngle);

// 				m_procMil->SaveWorkImg(ImgR, width, height, _T("Single2D Red.bmp"));
// 				m_procMil->SaveWorkImg(ImgW, width, height, _T("Single2D White.bmp"));
// 				m_procMil->SaveWorkImg(wndImg.m_ucArr2D, wnd_w, wnd_h, _T("Compose.bmp"));
			}
			else if ((sInspAlgo.m_eLightType == Side1_Light) || (sInspAlgo.m_eLightType == Side2_Light) || (sInspAlgo.m_eLightType == Side3_Light) || (sInspAlgo.m_eLightType == Side4_Light))
			{
				if (WindowRotate() == false)
				{
					dAlignAngle = 0;
				}
				m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h,
					sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, 0,
					wndImg.m_ucArr2D, dAlignAngle);

// 				m_procMil->SaveWorkImg(ImgR, width, height, _T("Side Red.bmp"));
// 				m_procMil->SaveWorkImg(ImgG, width, height, _T("Side Green.bmp"));
// 				m_procMil->SaveWorkImg(ImgB, width, height, _T("Side Blue.bmp"));
// 				m_procMil->SaveWorkImg(wndImg.m_ucArr2D, wnd_w, wnd_h, _T("Side_Compose.bmp"));
			}
			else
			{
				bUserLight = true;
// 				m_procMil->SaveWorkImg(sLightBuf.m_pucTRed, width, height, _T("Top Red.bmp"));
// 				m_procMil->SaveWorkImg(sLightBuf.m_pucMRed, width, height, _T("Mid Red.bmp"));
// 				m_procMil->SaveWorkImg(sLightBuf.m_pucBRed, width, height, _T("Bot Red.bmp"));
				if (WindowRotate() == false)
				{
					dAlignAngle = 0;
				}
				m_InspAlgo.ROIImageClaculCompose(sLightBuf, wndImg.m_ucArr2D, dAlignAngle);
				//m_procMil->SaveWorkImg(wndImg.m_ucArr2D, wnd_w, wnd_h, _T("Compose_User.bmp"));
			}

			InspRoiImgBuf *sInspImgBuf = new InspRoiImgBuf;
			//InspRoiImgBuf *sInspImgBuf = g_pMManager->pem_new<InspRoiImgBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
			InspRoiImgBuf* sArrInspImgBuf = NULL;
			sInspImgBuf->imgTop_R = NULL;
			sInspImgBuf->imgTop_G = NULL;
			sInspImgBuf->imgTop_B = NULL;
			sInspImgBuf->imgTop_W = NULL;
			sInspImgBuf->imgMiddle_R = NULL;
			sInspImgBuf->imgMiddle_B = NULL;
			sInspImgBuf->imgBottom_R = NULL;
			sInspImgBuf->imgBottom_B = NULL;
			ColorDataInput(sInspAlgo, width, height, dX, dY, wnd_w, wnd_h, sInspImgBuf, 0, nInspCameraType);

			AlgoColorOpt ptrAlgoColorOpt;
			if (sInspAlgo.m_eAlgoType == eAlgoFoot || sInspAlgo.m_eAlgoType == eAlgoWire)
			{
				ptrAlgoColorOpt.m_sFovImg.imgTop_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_R;
				ptrAlgoColorOpt.m_sFovImg.imgTop_G = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_G;
				ptrAlgoColorOpt.m_sFovImg.imgTop_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_B;
				ptrAlgoColorOpt.m_sFovImg.imgTop_W = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_W;

				ptrAlgoColorOpt.m_sFovImg.imgMiddle_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgMiddle_R;
				ptrAlgoColorOpt.m_sFovImg.imgMiddle_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgMiddle_B;

				ptrAlgoColorOpt.m_sFovImg.imgBottom_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgBottom_R;
				ptrAlgoColorOpt.m_sFovImg.imgBottom_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgBottom_B;

				ptrAlgoColorOpt.m_sImgBuf.nImageSizeX = 0;
				ptrAlgoColorOpt.m_sImgBuf.nImageSizeY = 0;

				ptrAlgoColorOpt.m_rcImageRect.left = dX;
				ptrAlgoColorOpt.m_rcImageRect.top = dY;
				ptrAlgoColorOpt.m_rcImageRect.right = dX + wnd_w;
				ptrAlgoColorOpt.m_rcImageRect.bottom = dY + wnd_h;
				ptrAlgoColorOpt.m_sFovImg.nImageSizeX = width;
				ptrAlgoColorOpt.m_sFovImg.nImageSizeY = height;
			}
			InspAlgoType eInspAlgoType = sInspAlgo.m_eAlgoType;

			if ((eInspAlgoType == eAlgoPattern))
			{
				CPInsp_Pattern *pPatternInsp = (CPInsp_Pattern *)GetPatternInsp();
				
				UCHAR* ImgR_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_R;
				UCHAR* ImgG_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_G;
				UCHAR* ImgB_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_B;
				
				pPatternInsp->SetInspParam(sInspAlgo, wndImg, coordinateAlgo, width, height, stTieArea);
				//cv::Mat src(height,width, CV_MAKETYPE(CV_8U,3));
				if (dX < 0) dX = 0;
				if (dY < 0) dY = 0;

				for (int y = dY; y < dY + wnd_h; y++)
				{
					for (int x = dX; x < dX + wnd_w; x++)
					{
						int sizeY = y - dY;
						int sizeX = x - dX;
						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 0] = ImgB_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_blue < 255 ? ImgB_Pattern[y * width + x] *  pPatternInsp->m_pAlgoPattern->m_factor_blue : 255;
						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 1] = ImgG_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_green < 255 ? ImgG_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_green : 255;
						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 2] = ImgR_Pattern[y * width + x] * pPatternInsp->m_pAlgoPattern->m_factor_red < 255 ? ImgR_Pattern[y * width + x] *   pPatternInsp->m_pAlgoPattern->m_factor_red : 255;

					}
				}
				//cv::Mat cripsrc;

				//cv::Rect rect(dX, dY, wnd_w, wnd_h);
				/*cripsrc = src(rect);

				for(int y=0; y< wnd_h ; y++)
				{
					memcpy(&wndImg.m_ucArrCV[y*(wnd_w * 3)], &cripsrc.data[y*cripsrc.step], wnd_w * 3);
				}*/
			}

			nLine = __LINE__;
			//InspAlgoType eInspAlgoType = sInspAlgo.m_eAlgoType;
			BOOL bAlgoOK = FALSE;
			if (bUSeLeadAlign == FALSE)
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;

			switch (eInspAlgoType)
			{
			case eAlgoWire:
				nLine = __LINE__;
				{
					//qtm.StartTick();
					AlgoWire *pAlgoWire = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
					//int nWireCnt = pAlgoWire->m_byFootCnt;

					int nWireRefAreaOpt = pAlgoWire->m_nWireRefAreaOpt;

					int nFootRstIdx = 0;
					POINTF poArrFoot[WIRE_DRAW_CNT];
					memset(&poArrFoot, 0, sizeof(POINTF) * 2);

					float fWireRefAreaHgt = 0.0f;
					//Foot 결과 설정
					for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
					{
						if (nFootRstIdx > 1)
							break;

						if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp == FALSE)
							continue;
						if (m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo].m_eAlgoType != eAlgoFoot)
							continue;
						RstAlgoFoot *sRstAlgo = (RstAlgoFoot *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
						if (sRstAlgo == NULL) continue;

						//FootPad 바닥높이 값 wire 검사함수에 넘기기 
						InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
						AlgoFoot *pInspAlgoFoot = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;

						if (nAlgo == 0 && pAlgoWire->m_nWireRefAreaOpt == (int)m_eWire_RefAreaType::Foot1_pad)	//foot 1
						{
							fWireRefAreaHgt = sRstAlgo->m_fAreaHeight;
							pInspAlgoFoot->m_fPadAreaAvgHeight = fWireRefAreaHgt;
						}
						else if (nAlgo == 1 && pAlgoWire->m_nWireRefAreaOpt == (int)m_eWire_RefAreaType::Foot2_pad)	//foot 2
						{
							fWireRefAreaHgt = sRstAlgo->m_fAreaHeight;
							pInspAlgoFoot->m_fPadAreaAvgHeight = fWireRefAreaHgt;
						}

						if (sRstAlgo->m_sEnd.x == 0 && sRstAlgo->m_sEnd.y == 0)
							continue;
						if (dX >= sRstAlgo->m_sEnd.x || dX + wnd_w <= sRstAlgo->m_sEnd.x ||
							dY >= sRstAlgo->m_sEnd.y || dY + wnd_h <= sRstAlgo->m_sEnd.y)
							continue;

						/*poArrFoot[nFootRstIdx].x = sRstAlgo->m_sEnd.x - dX;
						poArrFoot[nFootRstIdx].y = sRstAlgo->m_sEnd.y - dY;*/
						poArrFoot[nFootRstIdx].x = (sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Wedge_L].x + sRstAlgo->m_poWire_Wnd.x) - dX;
						poArrFoot[nFootRstIdx].y = (sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Wedge_L].y + sRstAlgo->m_poWire_Wnd.y) - dY;

						nFootRstIdx++;
					}

					wndImg.m_nWidth3D = m_pInspBoardInfo->partImgBuf.nImageSizeX;
					wndImg.m_nHeight3D = m_pInspBoardInfo->partImgBuf.nImageSizeY;

					m_pWireBondingInsp = GetWireBondingInsp();

					WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoWire>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
					nLine = __LINE__;
					
					bAlgoOK = m_pWireBondingInsp->InspWire(sInspAlgo, wndImg, (RstAlgoWire *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, stTieArea, dX, dY, ptrAlgoColorOpt, nFootRstIdx == 2 ? poArrFoot : nullptr, -1, 0, m_puImgWireInterval, ptrAlignRes, fWireRefAreaHgt);
					nLine = __LINE__;
					//m_AlgoDt[_T("eAlgoHeight_Diff")] = (qtm.EndTick() * 1000.0) + m_AlgoDt[_T("eAlgoHeight_Diff")];
				}
				break;
			case eAlgoFoot:
			{
				nLine = __LINE__;
				//WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = new RstAlgoFoot;
				cv::Mat cvimg(0, 0, CV_32FC1);
				bool bUsePadAreaAutoteach = false;

				bUsePadAreaAutoteach = ((AlgoFoot*)sInspAlgo.m_ptrInspAlgoParam)->m_bUsePadAreaAutoTeach;
				
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoFoot>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
				nLine = __LINE__;
				std::vector<float> _3DLog; 
				cv::Mat _2D_Test(wndImg.m_nHeight, wndImg.m_nWidth , CV_8UC1, wndImg.m_ucArr2D);
				cv::Mat _3D_Test(wndImg.m_nHeight3D, wndImg.m_nWidth3D, CV_32FC1, wndImg.m_fArr3D);
// 				for (int nloop = 0; nloop < wndImg.m_nWidth3D* wndImg.m_nHeight3D; nloop++)
// 					_3DLog.push_back(wndImg.m_fArr3D[nloop]);
				CPInsp_WireBonding* PInspWireBonding = GetWireBondingInsp();
				cv::Mat cvDBCPolygonImg2(wndImg.m_nHeight, wndImg.m_nWidth, CV_8UC1);
				cvDBCPolygonImg2.setTo(0);

				CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, wndImg, dX, dY, ptrAlgoColorOpt, &cvimg, &cvDBCPolygonImg2, true, bUsePadAreaAutoteach);
				//CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, wndImg, dX, dY, ptrAlgoColorOpt, &cvimg,true, bUsePadAreaAutoteach);

				if (pFoot == NULL)
				{
					bAlgoOK = FALSE;
					break;
				}

#ifdef ALGORITHMTOOL_CPP_USE
				PInspWireBonding->nFootAlgoCnt++;
#endif
				CString sWndAlgoName = NULL;

#define CheckValidationImage	1
#ifdef CheckValidationImage
				/*#ifdef _DEBUG */
				{
					//window ROI 디버깅 이미지 저장 
					int nPartWidth = ptrAlgoColorOpt.m_sFovImg.nImageSizeX;
					int nPartHeight = ptrAlgoColorOpt.m_sFovImg.nImageSizeY;

					cv::Mat cvPartImage(nPartHeight, nPartWidth, CV_8UC1, ptrAlgoColorOpt.m_sFovImg.imgTop_R);
					cv::Mat cvPart_WindowROICheck(nPartHeight, nPartWidth, CV_8UC3);
					cvPart_WindowROICheck = cvPartImage.clone();

					int nWndStX = ptrAlgoColorOpt.m_rcImageRect.left;		//dx
					int nWndStY = ptrAlgoColorOpt.m_rcImageRect.top;			//dy
					int nWndWidth = ptrAlgoColorOpt.m_rcImageRect.right - ptrAlgoColorOpt.m_rcImageRect.left;
					int nWndHeight = ptrAlgoColorOpt.m_rcImageRect.bottom - ptrAlgoColorOpt.m_rcImageRect.top;

					cv::cvtColor(cvPart_WindowROICheck, cvPart_WindowROICheck, cv::COLOR_GRAY2RGB);
					cv::rectangle(cvPart_WindowROICheck, cv::Rect(nWndStX, nWndStY, nWndWidth, nWndHeight), cv::Scalar(0, 0, 255), 1, 8, 0);

					int nAlgoInspRectStX = pFoot->mImageRect.left;
					int nAlgoInspRectStY = pFoot->mImageRect.top;
					int nAlgoInspRectWidth = pFoot->mImageRect.right - pFoot->mImageRect.left;
					int nAlgoInspRectHeight = pFoot->mImageRect.bottom - pFoot->mImageRect.top;
					cv::rectangle(cvPart_WindowROICheck, cv::Rect(nAlgoInspRectStX, nAlgoInspRectStY, nAlgoInspRectWidth, nAlgoInspRectHeight), cv::Scalar(0, 255, 0), 1, 8, 0);

					int nAlgoSpecRectStX = pFoot->m_PadRect.left;
					int nAlgoSpecRectStY = pFoot->m_PadRect.top;
					int nAlgoSpecRectWidth = pFoot->m_PadRect.right - pFoot->m_PadRect.left;
					int nAlgoSpecRectHeight = pFoot->m_PadRect.bottom - pFoot->m_PadRect.top;
					cv::rectangle(cvPart_WindowROICheck, cv::Rect(nAlgoSpecRectStX, nAlgoSpecRectStY, nAlgoSpecRectWidth, nAlgoSpecRectHeight), cv::Scalar(255, 105, 180), 1, 8, 0);

					//create dir name
					time_t rawtime = time(nullptr);
					struct tm *curr_tm;
					curr_tm = localtime(&rawtime);

					int curr_year = curr_tm->tm_year + 1900;
					int curr_month = curr_tm->tm_mon + 1;
					int curr_day = curr_tm->tm_mday;
					int curr_hour = curr_tm->tm_hour, curr_minute = curr_tm->tm_min, curr_second = curr_tm->tm_sec;

					CString sValidateImgTime;
					sValidateImgTime.Format(_T("D:\\FootRst2"));
					if (PathFileExists(sValidateImgTime) == false)
						CreateDirectory(sValidateImgTime, NULL);

					CString sInspTime;
					sInspTime.Format(_T("D:\\FootRst2\\%d-%d-%d"), curr_year, curr_month, curr_day);
					if (PathFileExists(sInspTime) == false)
						CreateDirectory(sInspTime, NULL);

					CString sPartName = _T("");
					if (wcslen(m_pInspBoardInfo->PartNo) > 0)
					{
						sPartName.Format(_T("%s\\%s"), sInspTime, m_pInspBoardInfo->PartNo);
						if (PathFileExists(sPartName) == false)
							CreateDirectory(sPartName, NULL);
					}
					else
					{
						sPartName.Format(_T("%s\\%s"), sInspTime, L"null");
						if (PathFileExists(sPartName) == false)
							CreateDirectory(sPartName, NULL);
					}

					
					sWndAlgoName.Format(_T("%s\\%d_%d.bmp"), sPartName, nWndIndex, sInspAlgo.m_nAlgoId);

					CT2CA strWndAlgoName(sWndAlgoName);
					std::string strFootAlgoName(strWndAlgoName);

					cv::imwrite(strFootAlgoName, cvPart_WindowROICheck);
					////////////////////////////////
				}
				/*#endif*/
#endif
				bAlgoOK = PInspWireBonding->InspFoot(pFoot, wndImg, (RstAlgoFoot *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, stTieArea, ptrAlgoColorOpt, &cvimg, -1, NULL, ptrAlignRes, false, sWndAlgoName);


				if (pFoot)
				{
					//delete  pFoot;
					g_pMManager->pem_delete(pFoot, false);
					pFoot = NULL;
				}

				//bAlgoOK = m_InspAlgo.InspFoot(sInspAlgo, wndImg, (RstAlgoFoot *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, stTieArea, dX, dY, 
				//	ptrAlgoColorOpt, -1,NULL);
				{
					RstAlgoFoot * sRstAlgo = (RstAlgoFoot *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					//	for (int nIdx = 0; nIdx < sRstAlgo->m_nFootCnt; nIdx++)
					{
						if (sRstAlgo->m_sPoint.x > 0 || sRstAlgo->m_sPoint.y > 0)
						{
							sRstAlgo->m_sEnd.x = sRstAlgo->m_sPoint.x + dX;
							sRstAlgo->m_sEnd.y = sRstAlgo->m_sPoint.y + dY;
						}
					}
				}
				nLine = __LINE__;
			}
			break;
			
			case eAlgoPattern:
			{
				CPInsp_Pattern *pPatternInsp = (CPInsp_Pattern *)GetPatternInsp();
				if (pPatternInsp == nullptr)
					return bAlgoOK;

				//pPatternInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
				
				//임시
				//SetExtMachinePatternParam(sInspAlgo);

				pPatternInsp->SetInspParam(sInspAlgo, wndImg, coordinateAlgo, m_pInspBoardInfo->partImgBuf.nImageSizeX, m_pInspBoardInfo->partImgBuf.nImageSizeY, stTieArea);
				int a, b; 
				bAlgoOK = pPatternInsp->InspProc_Pattern(3, nOffX_pix, nOffY_pix);
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = g_pMManager->pem_new<RstAlgoPattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
				pPatternInsp->GetInspRst((RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);

				if (bAlgoOK && !bAlignResOK && (inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
				{
					RstAlgoPattern * rst = (RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
					arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
					arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
					arrAlignRes[nAlignWndIdx].offsetX = -rst->offsetX;
					arrAlignRes[nAlignWndIdx].offsetY = rst->offsetY;
					arrAlignRes[nAlignWndIdx].theta = 0;
					//ptrAlignRes[nCurAlignID] = arrAlignRes[nAlignWndIdx];
					bAlignResOK = TRUE;
				}

			}
			break;
			case eAlgoPOCR:
			{
				CPInsp_POCR *pPOCR = (CPInsp_POCR *)g_pInspMng->GetPocrTeach();
				InspPartInfo* pInspBoardInfo = m_pInspBoardInfo;

				pPOCR->SetInspParam(sInspAlgo, pInspBoardInfo->angle);
				bAlgoOK = pPOCR->InspProc_POCR(wndImg.m_ucArr2D, wndImg.m_nWidth, wndImg.m_nHeight,0);
				pPOCR->GetInspRst((RstAlgoPOCR *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
			}
			break;
			default:
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = FALSE;
				break;
			}

			if (sInspImgBuf)
			{
				if (sInspImgBuf->imgTop_R != NULL && sInspImgBuf->imgTop_R != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_R);
				if (sInspImgBuf->imgTop_G != NULL && sInspImgBuf->imgTop_G != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_G);
				if (sInspImgBuf->imgTop_B != NULL && sInspImgBuf->imgTop_B != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_B);
				if (sInspImgBuf->imgTop_W != NULL && sInspImgBuf->imgTop_W != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_W);
				if (sInspImgBuf->imgMiddle_R != NULL && sInspImgBuf->imgMiddle_R != nullptr)
					Delete_1DArray(&sInspImgBuf->imgMiddle_R);
				if (sInspImgBuf->imgMiddle_B != NULL && sInspImgBuf->imgMiddle_B != nullptr)
					Delete_1DArray(&sInspImgBuf->imgMiddle_B);
				if (sInspImgBuf->imgBottom_R != NULL && sInspImgBuf->imgBottom_R != nullptr)
					Delete_1DArray(&sInspImgBuf->imgBottom_R);
				if (sInspImgBuf->imgBottom_B != NULL && sInspImgBuf->imgBottom_B != nullptr)
					Delete_1DArray(&sInspImgBuf->imgBottom_B);

				// MemoryLeak Fix -wjlee
				//Delete_1DArray(&sInspImgBuf);
				g_pMManager->pem_delete(sInspImgBuf, false);
			}

			if (sArrInspImgBuf)
			{
				for (int a = 0; a < ALIGNEDGE_AREA_CNTS; a++)
				{
					if (sArrInspImgBuf[a].imgTop_R != NULL && sArrInspImgBuf[a].imgTop_R != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_R);
					if (sArrInspImgBuf[a].imgTop_G != NULL && sArrInspImgBuf[a].imgTop_G != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_G);
					if (sArrInspImgBuf[a].imgTop_B != NULL && sArrInspImgBuf[a].imgTop_B != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_B);
					if (sArrInspImgBuf[a].imgTop_W != NULL && sArrInspImgBuf[a].imgTop_W != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_W);
					if (sArrInspImgBuf[a].imgMiddle_R != NULL && sArrInspImgBuf[a].imgMiddle_R != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgMiddle_R);
					if (sArrInspImgBuf[a].imgMiddle_B != NULL && sArrInspImgBuf[a].imgMiddle_B != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgMiddle_B);
					if (sArrInspImgBuf[a].imgBottom_R != NULL && sArrInspImgBuf[a].imgBottom_R != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgBottom_R);
					if (sArrInspImgBuf[a].imgBottom_B != NULL && sArrInspImgBuf[a].imgBottom_B != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgBottom_B);
				}
				Delete_1DArray(&sArrInspImgBuf);
			}

			if (bUSeLeadAlign == FALSE)
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = bAlgoOK;

			if (sInspAlgo.m_eAlgoType != eAlgoLead_Tip)	//  Lead Tip 의 경우 위에서 체크함
			{
				if (sInspAlgo.m_bIsRequired && !sInspAlgo.m_bAlgoGroup && !bAlgoOK)	// 필수 검사 알고리즘이  NG이면 Window  결과  NG (단, Group이 체크되어있으면 일단 보류, 아래에서 판정)
				{
					ret = e_NG;
				}
				if (m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo].m_eAlgoType == eAlgoOCR && USENGOCR)
				{
					if (OCROKScore > OCRNGScore)
						ret = e_OK;
				}
			}

			// 교체용 디펙트 코드
			if (!bAlgoOK && sInspAlgo.m_bUsingManualDefectCode && bUSeLeadAlign == FALSE)
			{
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bUsingManualDefectCode = sInspAlgo.m_bUsingManualDefectCode;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualDefectCode = sInspAlgo.m_nManualDefectCode;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualSubDefectCode = sInspAlgo.m_nManualSubDefectCode;
			}

			//if (eAlgoNum > (int)sInspAlgo.m_eAlgoType)
			//	m_fInspAlgoTact[(int)sInspAlgo.m_eAlgoType] += ((GetTickCount() - start) / 1000.0f);
		}

		nLine = __LINE__;
		Delete_1DArray(&ptrnArrLeadTipRst);
		Delete_1DArray(&ptrAlignRes);
		BOOL bIsInsp = TRUE;
		if (ret == e_OK && bUSeLeadAlign == FALSE)
		{

			nLine = __LINE__;

			int nResGroup = e_NG;		// Group이 아닌 알고리즘들의 결과
			int nResNotGroup = e_NG;		// Group인 알고리즘들의 결과
			BOOL bIsEssentialInGroup = FALSE;	// Group에 포함된 알고리즘 중 Essential check된 것이 있는지 여부
			bIsInsp = FALSE;
			if (m_pParamArray[nWndIndex].nAlgorithmCnt > 0)
				ret = e_NG;		// 일단 NG로,
			for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
			{
				if (!WndResult->m_vArrRstInspAlgo)
					continue;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Search || WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Tip
					|| WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoTab_Search)
				{
					if (m_pParamArray[nWndIndex].nAlgorithmCnt == 1)
						ret = e_OK;	// LeadSearch
					continue;
				}
				// 알고리즘 간 기본 OR 처리 (여기 들어왔다는 것은 필수 검사는 모두 OK 인 경우이므로 고려하지 않아도 됨)
				// 2015/08/10 : 알고리즘 Group flag 추가되어서 필수 검사 NG 여부도 여기서 체크해야함

				nLine = __LINE__;

				InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
				if (sInspAlgo.m_bAlgoGroup && sInspAlgo.m_bIsRequired)
					bIsEssentialInGroup = TRUE;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk == TRUE)
				{
					if (!sInspAlgo.m_bAlgoGroup)
						nResNotGroup = e_OK;		// Group Enable = false인 알고리즘 결과가 하나라도 OK라면 OK
					else
						nResGroup = e_OK;		// Group Enable = true인 알고리즘 결과가 하나라도 OK라면 OK
				}

				bIsInsp = TRUE;
			}

			nLine = __LINE__;

			if (nResGroup == e_NG && bIsEssentialInGroup)		// Group의 결과가 NG이고 Group에 포함된 알고리즘 중 Essential check가 있으면 무조건 NG
				ret = e_NG;
			else if (nResNotGroup == e_OK || nResGroup == e_OK)	// 그렇지 않고, Group이 아닌 알고리즘 중 OK가 있거나, Group인 알고리즘 중 OK가 있으면 OK
				ret = e_OK;
		}
		if (bUSeLeadAlign == FALSE)
		{
			WndResult->m_bOk = ret;
			WndResult->m_bIsInsp = bIsInsp;
		}
		nLine = __LINE__;

		if (wndImg.m_ucArr2D)
			Delete_1DArray(&wndImg.m_ucArr2D);

		if (wndImg.m_ucArrCV)
			Delete_1DArray(&wndImg.m_ucArrCV);

		//LJH
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		{
			if (wndImg.m_ucArr2D_Mix[iLoopCount] != NULL)
			{
				//delete wndImg.m_ucArr2D_Mix[iLoopCount];
				//wndImg.m_ucArr2D_Mix[iLoopCount] = NULL;
				Delete_1DArray(&wndImg.m_ucArr2D_Mix[iLoopCount]);
			}
		}
		if (wndImg.m_fArr3D)
			Delete_1DArray(&wndImg.m_fArr3D);

		nLine = __LINE__;

		if (wndImg.m_fArr3D)
			Delete_1DArray(&wndImg.m_fArr3D);

		Delete_1DArray(&ptrAlignRes);
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CMInspManager::InspWindowAlgo(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return ret;
}
// typedef struct tagPOINT_64F {
// 	double x;
// 	double y;
// 
// 	// SHKang 2018/07/12
// 	tagPOINT_64F()
// 	{
// 		x = y = 0.;
// 	}
// 	tagPOINT_64F(double sx, double sy)
// 	{
// 		x = sx;
// 		y = sy;
// 	}
// 
// } POINT_64F;
BOOL InspManager::WndSizeChange(InspAlgoType nAlgoType, AlgoCoordinate &coordinateAlgo, WndAlgoImg &wndImg,
	int nCurWndIndex, int &nWidth, int &nHeight, double &dX, double &dY, AlignResult * sptrAlignRes, int nWndinspType, InspAlgo sInspAlgo,
	BOOL bUseLeadTipSideOption, double dLeadTipShiftX, double dLeadTipShiftY, double dLeadTipSize, int nLeadTipDir, int nAlignTotalCnt, int &nOffX_pix, int &nOffY_pix)
{
	BOOL bResult = FALSE;

	InspAlgoType eAlgoType = nAlgoType;

	double dResolX = m_resolX;
	double dResolY = m_resolY;
	nOffX_pix = nOffY_pix = 0;
	// Side Camera MultiProcess used X
	//if (g_pMPTI->m_bSideOriginalSize == true)
	//{
	//	dResolX = g_pMPTI->m_dBtmSideResX;
	//	dResolY = g_pMPTI->m_dBtmSideResY;
	//}

	double fStartX = 0.0;
	double fStartY = 0.0;
	dX = 0;
	dY = 0;

	double fPartWidth = m_pInspBoardInfo->partWidth / dResolX;//m_resolX;
	double fPartHeight = m_pInspBoardInfo->partHeight / dResolY;//m_resolY;
	double fWndWidth = (double)(m_pParamArray[nCurWndIndex].width / dResolX);//m_resolX);
	double fWndHeight = (double)(m_pParamArray[nCurWndIndex].length / dResolY);//m_resolY);
	double dCx = m_pParamArray[nCurWndIndex].cx;
	double dCy = m_pParamArray[nCurWndIndex].cy;

	double dWndW = m_pParamArray[nCurWndIndex].width;
	double dWndH = m_pParamArray[nCurWndIndex].length;

	int nWndWidth = RounD(fWndWidth);
	int nWndHeight = RounD(fWndHeight);

	if (eAlgoType == eAlgoBody_Blob)
	{
		coordinateAlgo.dROICenterX = (fPartWidth / 2.);
		coordinateAlgo.dROICenterY = (fPartHeight / 2.);
		coordinateAlgo.dROIWidth = (fPartWidth);
		coordinateAlgo.dROILength = (fPartHeight);
	}
	else
	{
		coordinateAlgo.dROICenterX = (double)((fPartWidth / 2.) + dCx / m_resolX);
		coordinateAlgo.dROICenterY = (double)((fPartHeight / 2.) - dCy / m_resolY);
		coordinateAlgo.dROIWidth = fWndWidth;
		coordinateAlgo.dROILength = fWndHeight;
	}
	coordinateAlgo.dROIAngle = m_pInspBoardInfo->angle;
	coordinateAlgo.dWndX = 0;
	coordinateAlgo.dWndY = 0;
	coordinateAlgo.dPartW = m_pInspBoardInfo->partWidth;
	coordinateAlgo.dPartH = m_pInspBoardInfo->partHeight;

	bool bArrArr = (nAlignTotalCnt >= 2);
	int nTotalCnt = nAlignTotalCnt;
	if (nTotalCnt <= 0) nTotalCnt = 1;
	float fPartErrX = m_pInspBoardInfo->fPartRoundingErrX;
	float fPartErrY = m_pInspBoardInfo->fPartRoundingErrY;
	//if (eAlgoType == eAlgoBlob)
	//{
	//	fPartErrX = 0.0f;
	//	fPartErrY = 0.0f;
	//}
	float fWNDErrX = 0.0f;
	float fWNDErrY = 0.0f;
	for (int n = 0; n < nTotalCnt; n++)
	{
		int nAlignIdx = (bArrArr) ? n : 0;

		double dAlignRes_centerX = 0;
		double dAlignRes_centerY = 0;
		double dAlignRes_theta = 0;
		double dAlignRes_offsetX = 0;
		double dAlignRes_offsetY = 0;
		int nAlignRes_Width = 0;
		int nAlignRes_Height = 0;
		LONG lAlignRes_left = 0;
		LONG lAlignRes_right = 0;
		LONG lAlignRes_top = 0;
		LONG lAlignRes_bottom = 0;
		if (nAlignTotalCnt > nAlignIdx && sptrAlignRes)
		{
			dAlignRes_centerX = sptrAlignRes[nAlignIdx].centerX;
			dAlignRes_centerY = sptrAlignRes[nAlignIdx].centerY;
			dAlignRes_theta = sptrAlignRes[nAlignIdx].theta;
			dAlignRes_offsetX = sptrAlignRes[nAlignIdx].offsetX;
			dAlignRes_offsetY = sptrAlignRes[nAlignIdx].offsetY;
			nAlignRes_Width = sptrAlignRes[nAlignIdx].rcBodyRect.Width();
			nAlignRes_Height = sptrAlignRes[nAlignIdx].rcBodyRect.Height();
			lAlignRes_left = sptrAlignRes[nAlignIdx].rcBodyRect.left;
			lAlignRes_right = sptrAlignRes[nAlignIdx].rcBodyRect.right;
			lAlignRes_top = sptrAlignRes[nAlignIdx].rcBodyRect.top;
			lAlignRes_bottom = sptrAlignRes[nAlignIdx].rcBodyRect.bottom;
			if (sptrAlignRes[nAlignIdx].m_bAlgoCenter == TRUE)
			{
				if (sptrAlignRes[nAlignIdx].m_dCenterX_R > 0 &&
					sptrAlignRes[nAlignIdx].m_dCenterY_R > 0 &&
					sptrAlignRes[nAlignIdx].m_dCenterX_R < m_pInspBoardInfo->partWidth &&
					sptrAlignRes[nAlignIdx].m_dCenterY_R < m_pInspBoardInfo->partHeight)
				{
					dAlignRes_centerX = sptrAlignRes[nAlignIdx].m_dCenterX_R - (m_pInspBoardInfo->partWidth / 2.0);
					dAlignRes_centerY = sptrAlignRes[nAlignIdx].m_dCenterY_R - (m_pInspBoardInfo->partHeight / 2.0);
				}
			}
		}

		//bool bInspWindowArea = true;
		//bool bInspLeadAlgo = false;
		//bool bInspChipTrackingAlgo = false;



		bool bInspWindowArea = m_dicInspAlgorithm[eAlgoType]->InspWindowArea();
		bool bInspLeadAlgo = m_dicInspAlgorithm[eAlgoType]->InspWindowArea(m_eInspWindowArea_Type_Lead);
		bool bInspChipTrackingAlgo = m_dicInspAlgorithm[eAlgoType]->InspWindowArea(m_eInspWindowArea_Type_ChipTracking);
		if (bInspWindowArea == false)
		{
			nWidth = RounD(fPartWidth);
			nHeight = RounD(fPartHeight);
			fStartX = 0;
			fStartY = 0;

			dX = fStartX;
			dY = fStartY;

			if (eAlgoType == eAlgoPadAlign)
			{
				nWndWidth = nWidth;
				nWndHeight = nHeight;
			}
		}
		else if (eAlgoType == eAlgoLead_Tip || eAlgoType == eAlgoLead_SideSolder)
		{
			nWidth = RounD(coordinateAlgo.dROIWidth);
			nHeight = RounD(coordinateAlgo.dROILength);
			double corr_x = 0, corr_y = 0;


			if (nAlignTotalCnt > nAlignIdx && sptrAlignRes)
			{
				// LMJ 2021/08/08 : GetWndClip_3() 함수에서와 좌표 계산 동일하게 맞춰줌
				m_proc3d.CorrectCoordinate(dCx, dCy, dAlignRes_centerX, dAlignRes_centerY, dAlignRes_theta, dAlignRes_offsetX, dAlignRes_offsetY, &corr_x, &corr_y);
				fStartX = (fPartWidth) / 2. + (corr_x / dResolX);
				fStartY = (fPartHeight) / 2. - (corr_y / dResolY);
				coordinateAlgo.dROICenterX = fStartX;
				coordinateAlgo.dROICenterY = fStartY;
				dX = RounD(fStartX - (coordinateAlgo.dROIWidth / 2.));
				dY = RounD(fStartY - (coordinateAlgo.dROILength / 2.));
				if (dX < 0)
					dX = 0;
				if (dY < 0)
					dY = 0;
				if (dX + nWidth > RounD(fPartWidth))
					dX = RounD(fPartWidth - coordinateAlgo.dROIWidth);
				if (dY + nHeight > RounD(fPartHeight))
					dY = RounD(fPartHeight - coordinateAlgo.dROILength);
			}
			else
			{
				fStartX = (fPartWidth) / 2. + (dCx / dResolX);
				fStartY = (fPartHeight) / 2. - (dCy / dResolY);
				coordinateAlgo.dROICenterX = fStartX;
				coordinateAlgo.dROICenterY = fStartY;
				dX = RounD(fStartX - coordinateAlgo.dROIWidth / 2.);
				dY = RounD(fStartY - coordinateAlgo.dROILength / 2.);
			}
			if (dX < 0) dX = 0;
			if (dY < 0) dY = 0;

			if (eAlgoType == eAlgoLead_Tip)
			{
				AlgoLeadTip *pInspAlgoLeadTip = (AlgoLeadTip *)sInspAlgo.m_ptrInspAlgoParam;
				if (pInspAlgoLeadTip->m_bUseSideTip)
				{
					int nSearchSize = 0;
					if (pInspAlgoLeadTip->nLeadDirection == 0 || pInspAlgoLeadTip->nLeadDirection == 1)	// 0 = left, 1 = right
					{
						nSearchSize = nWndHeight * (pInspAlgoLeadTip->m_dSideTipSearchArea / 100.0);
						nHeight = nSearchSize * 2 + nWndHeight;
						dY = dY - nSearchSize;
						if (dY < 0) dY = 0;
						if (dY + nHeight > fPartHeight)
							nHeight = fPartHeight - dY;
						if (nHeight < 0)
							nHeight = 0;
						if (nHeight > fPartHeight)
							nHeight = fPartHeight - 1;
					}
					else
					{
						nSearchSize = nWndWidth * (pInspAlgoLeadTip->m_dSideTipSearchArea / 100.0);
						nWidth = nSearchSize * 2 + nWndWidth;
						dX = dX - nSearchSize;
						if (dX < 0) dX = 0;
						if (dX + nWidth > fPartWidth)
							nWidth = fPartWidth - dX;
						if (nWidth < 0)
							nWidth = 0;
						if (nWidth > fPartWidth)
							nWidth = fPartWidth - 1;
					}
				}
			}
			else	// Lead Side Solder
			{
				AlgoLeadSideSolder *pInspAlgoLeadSideS = (AlgoLeadSideSolder *)sInspAlgo.m_ptrInspAlgoParam;

				if (pInspAlgoLeadSideS->m_nLeadTipDirection == 0 || pInspAlgoLeadSideS->m_nLeadTipDirection == 1)	// 0 = left, 1 = right
				{
					nHeight += pInspAlgoLeadSideS->m_nRectHeight[1] * 2;
					dY -= pInspAlgoLeadSideS->m_nRectHeight[1];
				}
				else	// 2 = Top, 3 = Bottom
				{
					nWidth += pInspAlgoLeadSideS->m_nRectWidth[1] * 2;
					dX -= pInspAlgoLeadSideS->m_nRectWidth[1];
				}
				if (dX < 0) dX = 0;
				if (dY < 0) dY = 0;
			}
		}
		else
		{
			nWidth = RounD(coordinateAlgo.dROIWidth);
			nHeight = RounD(coordinateAlgo.dROILength);

			if (bUseLeadTipSideOption == TRUE && nWndinspType == eINSP_LEADSOLDER && bInspLeadAlgo)
			{
				if (nLeadTipDir == 0 || nLeadTipDir == 1)
				{
					if (dLeadTipSize > 0 && (int)(dLeadTipSize / dResolY) > 0)//m_resolY) > 0)
						nHeight = dLeadTipSize / m_resolY;
				}
				else if (nLeadTipDir == 2 || nLeadTipDir == 3)
				{
					if (dLeadTipSize > 0 && (int)(dLeadTipSize / dResolX) > 0)//m_resolX) > 0)
						nWidth = dLeadTipSize / m_resolX;
				}
			}
			double corr_x = 0, corr_y = 0;
			bool bBridgeOffset = true;

			if (eAlgoType == eAlgoBridge)
			{
				AlgoBridge *pInspAlgoBridge = (AlgoBridge *)sInspAlgo.m_ptrInspAlgoParam;
				bBridgeOffset = pInspAlgoBridge->m_bOffset;
			}

			//NYJ 2019/12/18
			bool bFootOffset = true;
			if (eAlgoType == eAlgoFoot)
			{
				AlgoFoot *pInspAlgoFoot = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;
				bFootOffset = pInspAlgoFoot->m_bOffset;
			}

			if (nAlignTotalCnt > nAlignIdx && sptrAlignRes && (bBridgeOffset == true) && (eAlgoType != eAlgoFoot))
			{
				if (nWndinspType == eINSP_SOLDER && bInspChipTrackingAlgo || eAlgoType == eAlgoFillet)
				{
					AlgoBaseBW *pInspAlgoBW = (AlgoBaseBW *)sInspAlgo.m_ptrInspAlgoParam;
					AlgoBW *pInspAlgoBW_ORG = NULL;
					AlgoLength *pInspAlgoLength_ORG = NULL;
					AlgoVolume *pInspAlgoVolume_ORG = NULL;
					if (eAlgoType == eAlgoBW)
						pInspAlgoBW_ORG = (AlgoBW *)sInspAlgo.m_ptrInspAlgoParam;
					if (eAlgoType == eAlgoWidth)
						pInspAlgoLength_ORG = (AlgoLength *)sInspAlgo.m_ptrInspAlgoParam;
					if (eAlgoType == eAlgoVolume)
						pInspAlgoVolume_ORG = (AlgoVolume *)sInspAlgo.m_ptrInspAlgoParam;
					// 회전 중심점 변경 : Part Center -> Align Window Center
					m_proc3d.CorrectCoordinate(dCx, dCy, dAlignRes_centerX, dAlignRes_centerY, dAlignRes_theta, dAlignRes_offsetX, dAlignRes_offsetY, &corr_x, &corr_y);
					if (bArrArr)
					{
						dCx = corr_x;
						dCy = corr_y;
					}
					fStartX = (fPartWidth / 2. + corr_x / dResolX);//m_resolX);
					fStartY = (fPartHeight / 2. - corr_y / dResolY);//m_resolY);
					coordinateAlgo.dROICenterX = fStartX;
					coordinateAlgo.dROICenterY = fStartY;
					dX = RounD(fStartX - (coordinateAlgo.dROIWidth / 2.));
					dY = RounD(fStartY - (coordinateAlgo.dROILength / 2.));
					if (dX < 0) dX = 0;
					if (dY < 0) dY = 0;
					if (pInspAlgoBW->m_bChipTracking == TRUE)
					{
						// 시작 위치를 바디블랍의 1픽셀 안쪽으로 붙여 생성한다
						int nBWGap = -1;
						if (bInspChipTrackingAlgo)
						{
							if (eAlgoType == eAlgoBW)
								nBWGap = pInspAlgoBW_ORG->m_nChipTrackingGap;
							else if (eAlgoType == eAlgoWidth)
								nBWGap = pInspAlgoLength_ORG->m_nChipTrackingGap;
							else if (eAlgoType == eAlgoVolume)
								nBWGap = pInspAlgoVolume_ORG->m_nChipTrackingGap;
							if (nBWGap < -20)	nBWGap = -20;
							if (nBWGap > 20)		nBWGap = 20;
						}
						if (nAlignRes_Width > 0 && nAlignRes_Height > 0)
						{
							if (nAlignRes_Width >= nAlignRes_Height)
							{
								int nBodyWidth = nAlignRes_Width;
								int nCenterX = lAlignRes_left + (nBodyWidth / 2);
								// Right Solder
								if (nCenterX < dX)
									dX = (lAlignRes_right + nBWGap);
								// Left Solder
								else
									dX = (lAlignRes_left - nBWGap) - nWidth;
							}
							else
							{
								int nBodyLength = nAlignRes_Height;
								int nCenterY = lAlignRes_top + (nBodyLength / 2);
								// Bottom Solder
								if (nCenterY < dY)
									dY = (lAlignRes_bottom + nBWGap);
								// Top Solder
								else
									dY = (lAlignRes_top - nBWGap) - nHeight;
							}
							if (dX < 0) dX = 0;
							if (dY < 0) dY = 0;
						}
					}
				}
				else
				{
					// 회전 중심점 변경 : Part Center -> Align Window Center
					m_proc3d.CorrectCoordinate(dCx, dCy, dAlignRes_centerX, dAlignRes_centerY, dAlignRes_theta, dAlignRes_offsetX, dAlignRes_offsetY, &corr_x, &corr_y);
					if (bArrArr)
					{
						dCx = corr_x;
						dCy = corr_y;
					}
					fStartX = (fPartWidth) / 2. + corr_x / dResolX;//m_resolX;
					fStartY = (fPartHeight) / 2. - corr_y / dResolY;//m_resolY;
					coordinateAlgo.dROICenterX = fStartX;
					coordinateAlgo.dROICenterY = fStartY;

					//apply PartClip Rounding Err  NYJ 2020.12
					float fDX = fStartX - (coordinateAlgo.dROIWidth / 2.);
					float fDY = fStartY - (coordinateAlgo.dROILength / 2.);
					m_pInspBoardInfo->m_sBigInfo.BigPartRoundingErr(fDX, fDY, nWidth, nHeight, fPartErrX, fPartErrY);
					fDX -= fPartErrX;
					fDY -= fPartErrY;

			/*		fDX -= m_pInspBoardInfo->fPartRoundingErrX;
					fDY -= m_pInspBoardInfo->fPartRoundingErrY;*/

					dX = (float)RounD(fDX);
					dY = (float)RounD(fDY);
					if (dX < 0) dX = 0;
					if (dY < 0) dY = 0;

					//save WindowClip Round err
					m_pInspBoardInfo->fWndRoundingErrX = (float)dX - (float)fDX;
					m_pInspBoardInfo->fWndRoundingErrY = (float)dY - (float)fDY;
					fWNDErrX = m_pInspBoardInfo->fWndRoundingErrX;
					fWNDErrY = m_pInspBoardInfo->fWndRoundingErrY;
					//apply WindowClip Round Err 
					if (CheckWndShift(eAlgoType))
					{
						coordinateAlgo.dWndX = fDX;
						coordinateAlgo.dWndY = fDY;
					}
					coordinateAlgo.dROICenterX = coordinateAlgo.dROICenterX + m_pInspBoardInfo->fWndRoundingErrX;
					coordinateAlgo.dROICenterY = coordinateAlgo.dROICenterY + m_pInspBoardInfo->fWndRoundingErrY;
					////////////////////////////////////////////////////

					nOffX_pix = RounD(((fPartWidth) / 2. + corr_x / dResolX) - ((fPartWidth) / 2. + dCx / dResolX));	// Align결과에 의해 offset된 pixel수
					nOffY_pix = RounD(((fPartHeight) / 2. + dCy / dResolY) - ((fPartHeight) / 2. + corr_y / dResolY));
				}
			}
			else if (nAlignTotalCnt > nAlignIdx && sptrAlignRes && (bFootOffset == true) && (eAlgoType == eAlgoFoot))		//NYJ 2019/12/18
			{
				AlgoFoot *pAlgoFoot1 = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;
				// 회전 중심점 변경 : Part Center -> Align Window Center
				m_proc3d.CorrectCoordinate(dCx, dCy, dAlignRes_centerX, dAlignRes_centerY, dAlignRes_theta, dAlignRes_offsetX, dAlignRes_offsetY, &corr_x, &corr_y);
				fStartX = (fPartWidth) / 2. + corr_x / m_resolX;
				fStartY = (fPartHeight) / 2. - corr_y / m_resolY;
				coordinateAlgo.dROICenterX = fStartX;
				coordinateAlgo.dROICenterY = fStartY;
				dX = (fStartX - ((float)(coordinateAlgo.dROIWidth) / 2.));
				dY = (fStartY - ((float)(coordinateAlgo.dROILength) / 2.));

				//////////////////////////////////////////
				CPoint rst;
				double part_x = m_pInspBoardInfo->partCx - m_pInspBoardInfo->partWidth / 2.;
				double part_y = m_pInspBoardInfo->partCy + m_pInspBoardInfo->partHeight / 2.;
				rst = CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, part_x, part_y, m_pInspBoardInfo->partCx, m_pInspBoardInfo->partCy, m_pInspBoardInfo);

				//save part-Clip rounding error correction value X/Y		NYJ 2020/12/03
				double dRetPixelX, dRetPixelY;
				CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, part_x, part_y, m_pInspBoardInfo->partCx, m_pInspBoardInfo->partCy, dRetPixelX, dRetPixelY, m_pInspBoardInfo);

				POINT_64F ptOff((float)dRetPixelX, (float)dRetPixelY);

				int nRstX, nRstY;
				nRstX = RounD(ptOff.x);
				nRstY = RounD(ptOff.y);

				float fPartRoundErrX = (float)nRstX - ptOff.x;
				float fPartRoundErrY = (float)nRstY - ptOff.y;
				////////////////////////////////////////////////

				dX -= fPartRoundErrX;
				dY -= fPartRoundErrY;

				dX = RounD(dX);
				dY = RounD(dY);

				if (dX + nWidth > RounD(fPartWidth))
					dX = RounD(fPartWidth - nWidth);
				if (dY + nHeight > RounD(fPartHeight))
					dY = RounD(fPartHeight - nHeight);
				if (dX < 0) dX = 0;
				if (dY < 0) dY = 0;

				//save WindowClip Round err
				m_pInspBoardInfo->fWndRoundingErrX = dX - dX;
				m_pInspBoardInfo->fWndRoundingErrY = dY - dY;

				//apply WindowClip Round Err 
				coordinateAlgo.dROICenterX = coordinateAlgo.dROICenterX + m_pInspBoardInfo->fWndRoundingErrX;
				coordinateAlgo.dROICenterY = coordinateAlgo.dROICenterY + m_pInspBoardInfo->fWndRoundingErrY;
			}
			else
			{
				fStartX = (m_pInspBoardInfo->partWidth / 2.) + (dCx);
				fStartY = (m_pInspBoardInfo->partHeight / 2.) - (dCy);

				//apply PartClip Rounding Err  NYJ 2020.12
				double fDX = (fStartX - (dWndW / 2.)) / dResolX;
				double fDY = (fStartY - (dWndH / 2.)) / dResolY;

				if ((eAlgoType == eAlgoFoot))
				{
					//////////////////////////////////////////
					CPoint rst;
					double part_x = m_pInspBoardInfo->partCx - m_pInspBoardInfo->partWidth / 2.;
					double part_y = m_pInspBoardInfo->partCy + m_pInspBoardInfo->partHeight / 2.;
					rst = CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, part_x, part_y, m_pInspBoardInfo->partCx, m_pInspBoardInfo->partCy, m_pInspBoardInfo);

					//save part-Clip rounding error correction value X/Y		NYJ 2020/12/03
					double dRetPixelX, dRetPixelY;
					CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, part_x, part_y, m_pInspBoardInfo->partCx, m_pInspBoardInfo->partCy, dRetPixelX, dRetPixelY, m_pInspBoardInfo);

					POINT_64F ptOff((float)dRetPixelX, (float)dRetPixelY);

					int nRstX, nRstY;
					nRstX = RounD(ptOff.x);
					nRstY = RounD(ptOff.y);

					float fPartRoundErrX = (float)nRstX - ptOff.x;
					float fPartRoundErrY = (float)nRstY - ptOff.y;
					////////////////////////////////////////////////

					fDX -= fPartRoundErrX;
					fDY -= fPartRoundErrY;

				}
				else
				{
					
						m_pInspBoardInfo->m_sBigInfo.BigPartRoundingErr(fDX, fDY, nWidth, nHeight, fPartErrX, fPartErrY);
						if (CheckWndShift(eAlgoType) == false)
						{
							fDX -= fPartErrX;
							fDY -= fPartErrY;
						}
	
				}

				dX = RounD(fDX);
				dY = RounD(fDY);
				if (dX < 0) dX = 0;
				if (dY < 0) dY = 0;

				//save WindowClip Round err
				m_pInspBoardInfo->fWndRoundingErrX = dX - fDX;
				m_pInspBoardInfo->fWndRoundingErrY = dY - fDY;
				fWNDErrX = m_pInspBoardInfo->fWndRoundingErrX;
				fWNDErrY = m_pInspBoardInfo->fWndRoundingErrY;


				//apply WindowClip Round Err 
				double dROICX = coordinateAlgo.dROICenterX;
				double dROICY = coordinateAlgo.dROICenterY;
				coordinateAlgo.dWndX = fDX;
				coordinateAlgo.dWndY = fDY;
			

				coordinateAlgo.dROICenterX = coordinateAlgo.dROICenterX + m_pInspBoardInfo->fWndRoundingErrX;
				coordinateAlgo.dROICenterY = coordinateAlgo.dROICenterY + m_pInspBoardInfo->fWndRoundingErrY;

				if (g_pMPTI->m_LogLevel == m_eLogLv_Blob)
				{
					CString sLog = _T("");
					sLog.Format(_T("[BLOB][WndSize] [X]\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f"), m_pInspBoardInfo->partWidth, dCx, fStartX, fDX, dWndW, dResolX, fDX, dX, m_pInspBoardInfo->fWndRoundingErrX, dROICX, coordinateAlgo.dROICenterX, fPartErrX);
					g_pMPTI->AddLog_Dev(sLog);
					sLog.Format(_T("[BLOB][WndSize] [Y]\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f"), m_pInspBoardInfo->partHeight, dCy, fStartY, fDY, dWndH, dResolY, fDY, dY, m_pInspBoardInfo->fWndRoundingErrY, dROICY, coordinateAlgo.dROICenterY, fPartErrY);
					g_pMPTI->AddLog_Dev(sLog);
				}

			}
		}
		if (dX < 0) dX = 0;
		if (dY < 0) dY = 0;
		if (dX + nWidth > RounD(fPartWidth))	dX = RounD(fPartWidth - nWidth);
		if (dY + nHeight > RounD(fPartHeight))	dY = RounD(fPartHeight - nHeight);
		if (bUseLeadTipSideOption == TRUE && nWndinspType == eINSP_LEADSOLDER && bInspLeadAlgo)
		{
			if (nLeadTipDir == 0 || nLeadTipDir == 1)
			{
				double dShift = (dLeadTipShiftY) / dResolY;//m_resolY;
				dY = RounD(dY + dShift);
			}
			else if (nLeadTipDir == 2 || nLeadTipDir == 3)
			{
				double dShift = (dLeadTipShiftX) / dResolX;//m_resolX;
				dX = RounD(dX + dShift);
			}
		}
		if (dX < 0) dX = 0;
		if (dY < 0) dY = 0;
	}

	// Algorithm 함수에 넘길 Window Image 구조체
	wndImg.m_nWidth = nWidth;
	wndImg.m_nHeight = nHeight;
	wndImg.m_nWidth3D = nWndWidth;
	wndImg.m_nHeight3D = nWndHeight;
	wndImg.m_ucArr2D = g_pMManager->pem_new<UCHAR>(true, wndImg.m_nWidth * wndImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
	if (sInspAlgo.m_eAlgoType == eAlgoPattern)
		wndImg.m_ucArrCV = g_pMManager->pem_new<UCHAR>(true, wndImg.m_nWidth * wndImg.m_nHeight * 3, (PCHAR)__FUNCTION__, __LINE__);

	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		wndImg.m_ucArr2D_Mix[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, wndImg.m_nWidth * wndImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);

	wndImg.m_fArr3D = g_pMManager->pem_new<float>(true, wndImg.m_nWidth * wndImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
	wndImg.dAngle = coordinateAlgo.dROIAngle;
	wndImg.m_fPartRoundingErrX = fPartErrX;
	wndImg.m_fPartRoundingErrY = fPartErrY;
	wndImg.m_fWndRoundingErrX = fWNDErrX;
	wndImg.m_fWndRoundingErrY = fWNDErrY;


	double dAngle = 0;
	if (nAlignTotalCnt == 1 && sptrAlignRes && CheckWndShift(eAlgoType))
		dAngle = sptrAlignRes[0].theta;

	if (m_pInspBoardInfo->partZmapData.data != NULL)
	{
		m_proc3d.GetCropZmap_LT(m_pInspBoardInfo->partZmapData.data, wndImg.m_fArr3D, m_pInspBoardInfo->partZmapData.zmapSizeX,
			m_pInspBoardInfo->partZmapData.zmapSizeY, dX, dY, nWidth, nHeight, dAngle);
		m_procMil->SaveWorkImg_float(m_pInspBoardInfo->partZmapData.data, m_pInspBoardInfo->partZmapData.zmapSizeX, m_pInspBoardInfo->partZmapData.zmapSizeY, _T("SingleOrg3DImage.bmp"));

		m_procMil->SaveWorkImg_float(wndImg.m_fArr3D, nWidth, nHeight, _T("SingleClip3DImage.bmp"));
	}
	if (eAlgoType == eAlgoBridge)
	{
		wndImg.m_nWidth3D = m_pInspBoardInfo->partZmapData.zmapSizeX;
		wndImg.m_nHeight3D = m_pInspBoardInfo->partZmapData.zmapSizeY;
	}
	else if (eAlgoType == eAlgoBody_Blob)
	{
		// 파트 Coordeinate가 필요.
		coordinateAlgo.dROICenterX = (wndImg.m_nWidth / 2.);
		coordinateAlgo.dROICenterY = (wndImg.m_nHeight / 2.);
	}
	return bResult = true;
}

bool InspManager::WindowRotate()
{
	bool wRotate = false; // g_pMPTI->m_bCheckWindowRotate;

	return wRotate;
}

// CPInsp_WireBonding* InspManager::GetWireBondingInsp() {
// 	return g_pMPTI->GetWireBondingInsp();
// }
// void * InspManager::GetPatternInsp() 
// {
// 	return g_pMPTI->GetPatternInsp();
// }
// void * InspManager::GetPocrTeach()
// {
// 	return g_pMPTI->GetPocrTeach();
// }
// void * InspManager::GetBarcodeInsp()
// {
// 	return g_pMPTI->GetBarocdeInsp();
// }
void InspManager::ColorDataInput(InspAlgo sInspAlgo, int nWidth, int nHeight, double dX, double dY, int nClipW, int nClipH, InspRoiImgBuf * sInspImgBuf, byte byIDX, int nSideCameraIndex)
{
	int nData = COLOR_DATA_Non;
	if (sInspAlgo.m_eAlgoType != eAlgoNum && g_pInspMng->m_dicInspAlgorithm[sInspAlgo.m_eAlgoType])
		nData = g_pInspMng->m_dicInspAlgorithm[sInspAlgo.m_eAlgoType]->UseColorImage(sInspAlgo, byIDX);
	if (nData == COLOR_DATA_Non)
		return;

	bool bSaveR = g_pMPTI->GetLogLv() == m_eLogLv_Blob;
	BOOL bColor = ((nData & COLOR_DATA_USE) == COLOR_DATA_USE);
	BOOL bWND = ((nData & COLOR_DATA_WND) == COLOR_DATA_WND);
	BOOL bBinAC = bColor && ((nData & COLOR_DATA_BIN_AC) == COLOR_DATA_BIN_AC);
	BOOL bAIData = ((nData & COLOR_DATA_AI) == COLOR_DATA_AI);
	BOOL bAngleColor = bAIData == FALSE && ((nData & COLOR_DATA_AC) == COLOR_DATA_AC);
	BOOL bMID = bAIData == FALSE && ((nData & COLOR_DATA_MID) == COLOR_DATA_MID);
	if (bColor == FALSE && bAngleColor == FALSE)
		return;

	CString str = _T("");
	str.Format(_T("part_TR_%d.bmp"), nData);
	if (m_pInspBoardInfo->partImgBuf.imgTop_R)
		m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgTop_R, nWidth, nHeight, str, 1, false, bSaveR);
	if (m_pInspBoardInfo->partImgBuf.imgTop_G)
		m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgTop_G, nWidth, nHeight, _T("part_TG.bmp"), 1, false, bSaveR);
	if (m_pInspBoardInfo->partImgBuf.imgTop_B)
		m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgTop_B, nWidth, nHeight, _T("part_TB.bmp"), 1, false, bSaveR);

	if (g_pMPTI->m_bSideOriginalSize)
	{
		if (m_pInspBoardInfo->partImgBuf.imgSide1_R)
			m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgSide1_R, nWidth, nHeight, _T("part_TR_Side1.bmp"), 1, false, bSaveR);
		if (m_pInspBoardInfo->partImgBuf.imgSide1_G)
			m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgSide1_G, nWidth, nHeight, _T("part_TG_Side1.bmp"), 1, false, bSaveR);
		if (m_pInspBoardInfo->partImgBuf.imgSide1_B)
			m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgSide1_B, nWidth, nHeight, _T("part_TB_Side1.bmp"), 1, false, bSaveR);
	}

	UCHAR *ucImg_TR = NULL;
	UCHAR *ucImg_TG = NULL;
	UCHAR *ucImg_TB = NULL;
	UCHAR *ucImg_TW = NULL;
	UCHAR *ucImg_MR = NULL;
	UCHAR *ucImg_MB = NULL;
	UCHAR *ucImg_BR = NULL;
	UCHAR *ucImg_BB = NULL;
	if (bWND)
	{
		sInspImgBuf->nImageSizeX = nClipW;
		sInspImgBuf->nImageSizeY = nClipH;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TG, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TW, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TG, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TW, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

		if (g_pMPTI->m_bSideOriginalSize == true)	// ?섎?移대찓??
		{
			m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);

			m_procMil->SaveWorkImg(ucImg_TR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TR_Side.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(ucImg_TG, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TG_Side.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(ucImg_TB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TB_Side.bmp"), 1, false, bSaveR);
		}
		else
		{
			if (nSideCameraIndex == 0 && m_pInspBoardInfo->partImgBuf.imgSide1_G != NULL)	// ?ъ씠??移대찓??1
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);

				m_procMil->SaveWorkImg(ucImg_TR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TR_Side_1.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TG, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TG_Side_1.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TB_Side_1.bmp"), 1, false, bSaveR);
			}
			else if (nSideCameraIndex == 1 && m_pInspBoardInfo->partImgBuf.imgSide2_G != NULL)	// ?ъ씠??移대찓??2
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide2_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide2_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide2_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);

				m_procMil->SaveWorkImg(ucImg_TR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TR_Side_2.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TG, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TG_Side_2.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TB_Side_2.bmp"), 1, false, bSaveR);
			}
			else if (nSideCameraIndex == 2 && m_pInspBoardInfo->partImgBuf.imgSide3_G != NULL)	// ?ъ씠??移대찓??3
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide3_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide3_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide3_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);

				m_procMil->SaveWorkImg(ucImg_TR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TR_Side_3.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TG, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TG_Side_3.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TB_Side_3.bmp"), 1, false, bSaveR);
			}
			else if (nSideCameraIndex == 3 && m_pInspBoardInfo->partImgBuf.imgSide4_G != NULL)	// ?ъ씠??移대찓??4
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide4_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide4_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide4_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);

				m_procMil->SaveWorkImg(ucImg_TR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TR_Side_4.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TG, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TG_Side_4.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TB_Side_4.bmp"), 1, false, bSaveR);
			}
			else	// ?숈텞 移대찓??
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);

				m_procMil->SaveWorkImg(ucImg_TR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TR.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TG, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TG.bmp"), 1, false, bSaveR);
				m_procMil->SaveWorkImg(ucImg_TB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("wnd_TB.bmp"), 1, false, bSaveR);
			}
		}

		// 		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		// 		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		// 		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_W, nWidth, nHeight, ucImg_TW, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgMiddle_R, nWidth, nHeight, ucImg_MR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgMiddle_B, nWidth, nHeight, ucImg_MB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgBottom_R, nWidth, nHeight, ucImg_BR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgBottom_B, nWidth, nHeight, ucImg_BB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
	}
	else
	{
		sInspImgBuf->nImageSizeX = nWidth;
		sInspImgBuf->nImageSizeY = nHeight;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TG, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TW, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TG, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TW, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TR, m_pInspBoardInfo->partImgBuf.imgTop_R, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TG, m_pInspBoardInfo->partImgBuf.imgTop_G, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TB, m_pInspBoardInfo->partImgBuf.imgTop_B, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TW, m_pInspBoardInfo->partImgBuf.imgTop_W, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_MR, m_pInspBoardInfo->partImgBuf.imgMiddle_R, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_MB, m_pInspBoardInfo->partImgBuf.imgMiddle_B, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_BR, m_pInspBoardInfo->partImgBuf.imgBottom_R, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_BB, m_pInspBoardInfo->partImgBuf.imgBottom_B, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

		m_procMil->SaveWorkImg(ucImg_TR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("part_cpy_TR.bmp"), 1, false, bSaveR);
		m_procMil->SaveWorkImg(ucImg_TG, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("part_cpy_TG.bmp"), 1, false, bSaveR);
		m_procMil->SaveWorkImg(ucImg_TB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("part_cpy_TB.bmp"), 1, false, bSaveR);
	}

	sInspImgBuf->imgTop_R = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgTop_G = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgTop_B = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgTop_W = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgMiddle_R = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgMiddle_B = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgBottom_R = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgBottom_B = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(sInspImgBuf->imgTop_R, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgTop_G, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgTop_B, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgTop_W, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgMiddle_R, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgMiddle_B, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgBottom_R, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgBottom_B, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

	cv::Mat imgG;
	if (bAIData)
		memcpy(sInspImgBuf->imgTop_W, ucImg_TW, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	else if (bAngleColor == TRUE || bBinAC == TRUE)
	{
		LightTypeBuf sLightBuf;
		sLightBuf.m_pucTRed = ucImg_TR;
		sLightBuf.m_pucTGreen = ucImg_TG;
		sLightBuf.m_pucTBlue = ucImg_TB;
		sLightBuf.m_pucTWhite = ucImg_TW;
		sLightBuf.m_pucMRed = ucImg_MR;
		imgG = m_pColorInsp->GetGreenImage(ucImg_MR, ucImg_MB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		sLightBuf.m_pucMGreen = imgG.data;
		sLightBuf.m_pucMBlue = ucImg_MB;
		sLightBuf.m_pucMWhite = NULL;
		sLightBuf.m_pucBRed = ucImg_BR;
		sLightBuf.m_pucBGreen = NULL;
		sLightBuf.m_pucBBlue = ucImg_BB;
		sLightBuf.m_pucBWhite = NULL;
		sLightBuf.m_nImgWidth = sInspImgBuf->nImageSizeX;
		sLightBuf.m_nImgHeight = sInspImgBuf->nImageSizeY;
		sLightBuf.m_nROIImgWidth = sInspImgBuf->nImageSizeX;
		sLightBuf.m_nROIImgHeight = sInspImgBuf->nImageSizeY;
		sLightBuf.m_dROIX = 0;
		sLightBuf.m_dROIY = 0;
		/*sLightBuf.m_pnRedValue = new int[LIGHT_CNT];
		sLightBuf.m_pnGreenValue = new int[LIGHT_CNT];
		sLightBuf.m_pnBlueValue = new int[LIGHT_CNT];
		sLightBuf.m_pnWhiteValue = new int[LIGHT_CNT];
		sLightBuf.m_pnPosition = new int[LIGHT_CNT];
		sLightBuf.m_pnCalculation = new int[LIGHT_CNT];*/
		sLightBuf.m_pnRedValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnGreenValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnBlueValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnWhiteValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnPosition = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnCalculation = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		for (int a = 0; a < 3; a++)
		{
			sLightBuf.m_nImgCnt = m_sLightData[a].m_byLightCnt;
			memset(sLightBuf.m_pnRedValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnGreenValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnBlueValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnWhiteValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnPosition, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnCalculation, 0, LIGHT_CNT * sizeof(int));
			for (int b = 0; b < LIGHT_CNT; b++)
			{
				sLightBuf.m_pnRedValue[b] = m_sLightData[a].m_byArrLightData[eLightData_Red][b];
				sLightBuf.m_pnGreenValue[b] = m_sLightData[a].m_byArrLightData[eLightData_Green][b];
				sLightBuf.m_pnBlueValue[b] = m_sLightData[a].m_byArrLightData[eLightData_Blue][b];
				sLightBuf.m_pnWhiteValue[b] = m_sLightData[a].m_byArrLightData[eLightData_White][b];
				sLightBuf.m_pnPosition[b] = m_sLightData[a].m_byArrLightData[eLightData_Position][b];
				sLightBuf.m_pnCalculation[b] = m_sLightData[a].m_byArrLightData[eLightData_Calculation][b];
			}
			if (a == 0)
				m_InspAlgo.ROIImageClaculCompose(sLightBuf, sInspImgBuf->imgMiddle_R);
			else if (a == 1)
				m_InspAlgo.ROIImageClaculCompose(sLightBuf, sInspImgBuf->imgTop_W);
			else if (a == 2)
				m_InspAlgo.ROIImageClaculCompose(sLightBuf, sInspImgBuf->imgMiddle_B);
		}
		Delete_1DArray(&sLightBuf.m_pnRedValue);
		Delete_1DArray(&sLightBuf.m_pnGreenValue);
		Delete_1DArray(&sLightBuf.m_pnBlueValue);
		Delete_1DArray(&sLightBuf.m_pnWhiteValue);
		Delete_1DArray(&sLightBuf.m_pnPosition);
		Delete_1DArray(&sLightBuf.m_pnCalculation);

		m_procMil->SaveWorkImg(sInspImgBuf->imgMiddle_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("AC_TR.bmp"), 1, false, bSaveR);
		m_procMil->SaveWorkImg(sInspImgBuf->imgTop_W, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("AC_TG.bmp"), 1, false, bSaveR);
		m_procMil->SaveWorkImg(sInspImgBuf->imgMiddle_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("AC_TB.bmp"), 1, false, bSaveR);

		if (bBinAC)
		{
			memcpy(sInspImgBuf->imgTop_R, sInspImgBuf->imgMiddle_R, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_G, sInspImgBuf->imgTop_W, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_B, sInspImgBuf->imgMiddle_B, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("RST_TR_AC.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_G, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("RST_TG_AC.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("RST_TB_AC.bmp"), 1, false, bSaveR);
		}
	}

	if (bBinAC == FALSE)
	{
		if (bMID && ucImg_MR != NULL && ucImg_MB != NULL && m_pColorInsp != NULL)
		{
			memcpy(sInspImgBuf->imgTop_R, ucImg_MR, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_B, ucImg_MB, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			imgG = m_pColorInsp->GetGreenImage(ucImg_MR, ucImg_MB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_G, imgG.data, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("MID_TR.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_G, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("MID_TG.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("MID_TB.bmp"), 1, false, bSaveR);
		}
		else
		{
			memcpy(sInspImgBuf->imgTop_R, ucImg_TR, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_G, ucImg_TG, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_B, ucImg_TB, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("RST_TR.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_G, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("RST_TG.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(sInspImgBuf->imgTop_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("RST_TB.bmp"), 1, false, bSaveR);
		}
	}
	memcpy(sInspImgBuf->imgBottom_R, ucImg_BR, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memcpy(sInspImgBuf->imgBottom_B, ucImg_BB, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

	cv::Mat TR(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgTop_R);
	cv::Mat TG(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgTop_G);
	cv::Mat TB(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgTop_B);
	cv::Mat TW(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgTop_W);

	cv::Mat MR(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgMiddle_R);
	cv::Mat MB(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgMiddle_B);

	cv::Mat BR(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgBottom_R);
	cv::Mat BB(sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX, CV_8UC1, sInspImgBuf->imgBottom_B);

	m_procMil->SaveWorkImg(sInspImgBuf->imgTop_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_TR.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgTop_G, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_TG.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgTop_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_TB.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgBottom_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_BR.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgBottom_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_BB.bmp"));
	Delete_1DArray(&ucImg_TR);
	Delete_1DArray(&ucImg_TG);
	Delete_1DArray(&ucImg_TB);
	Delete_1DArray(&ucImg_TW);
	Delete_1DArray(&ucImg_MR);
	Delete_1DArray(&ucImg_MB);
	Delete_1DArray(&ucImg_BR);
	Delete_1DArray(&ucImg_BB);
}

void InspManager::SetColorData(BOOL bAngleColor, int nWidth, int nHeight, InspRoiImgBuf * sInspImgBuf, int nType, double dX, double dY, int nClipW, int nClipH, int nSideCameraIndex, byte byColorLightType)
{
	if (m_pInspBoardInfo->partImgBuf.imgTop_R)
		m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgTop_R, nWidth, nHeight, _T("part_TR.bmp"));
	if (m_pInspBoardInfo->partImgBuf.imgTop_G)
		m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgTop_G, nWidth, nHeight, _T("part_TG.bmp"));
	if (m_pInspBoardInfo->partImgBuf.imgTop_B)
		m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgTop_B, nWidth, nHeight, _T("part_TB.bmp"));

	if (g_pMPTI->m_bSideOriginalSize)
	{
		if (m_pInspBoardInfo->partImgBuf.imgSide1_R)
			m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgSide1_R, nWidth, nHeight, _T("part_TR_Side1.bmp"));
		if (m_pInspBoardInfo->partImgBuf.imgSide1_G)
			m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgSide1_G, nWidth, nHeight, _T("part_TG_Side1.bmp"));
		if (m_pInspBoardInfo->partImgBuf.imgSide1_B)
			m_procMil->SaveWorkImg(m_pInspBoardInfo->partImgBuf.imgSide1_B, nWidth, nHeight, _T("part_TB_Side1.bmp"));
	}

	UCHAR *ucImg_TR = NULL;
	UCHAR *ucImg_TG = NULL;
	UCHAR *ucImg_TB = NULL;
	UCHAR *ucImg_TW = NULL;
	UCHAR *ucImg_MR = NULL;
	UCHAR *ucImg_MB = NULL;
	UCHAR *ucImg_BR = NULL;
	UCHAR *ucImg_BB = NULL;
	if (nType == 1)
	{
		sInspImgBuf->nImageSizeX = nClipW;
		sInspImgBuf->nImageSizeY = nClipH;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TG, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TW, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TG, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TW, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

		if (g_pMPTI->m_bSideOriginalSize == true)	// ?섎?移대찓??
		{
			m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		}
		else
		{
			if (nSideCameraIndex == 0 && m_pInspBoardInfo->partImgBuf.imgSide1_G != NULL)	// ?ъ씠??移대찓??1
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			}
			else if (nSideCameraIndex == 1 && m_pInspBoardInfo->partImgBuf.imgSide2_G != NULL)	// ?ъ씠??移대찓??2
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide2_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide2_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide2_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			}
			else if (nSideCameraIndex == 2 && m_pInspBoardInfo->partImgBuf.imgSide3_G != NULL)	// ?ъ씠??移대찓??3
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide3_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide3_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide3_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			}
			else if (nSideCameraIndex == 3 && m_pInspBoardInfo->partImgBuf.imgSide4_G != NULL)	// ?ъ씠??移대찓??4
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide4_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide4_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide4_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			}
			else	// ?숈텞 移대찓??
			{
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
				m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
			}
		}

		// 		m_procMil->GetClipImage_LT(ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		// 		m_procMil->GetClipImage_LT(ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		// 		m_procMil->GetClipImage_LT(ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgTop_W, nWidth, nHeight, ucImg_TW, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgMiddle_R, nWidth, nHeight, ucImg_MR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgMiddle_B, nWidth, nHeight, ucImg_MB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgBottom_R, nWidth, nHeight, ucImg_BR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgBottom_B, nWidth, nHeight, ucImg_BB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
	}
	else if (nType == 2)
	{
		sInspImgBuf->nImageSizeX = nWidth;
		sInspImgBuf->nImageSizeY = nHeight;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TG, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_TW, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_MB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BR, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImg_BB, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TG, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_TW, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_MB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BR, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memset(ucImg_BB, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TR, m_pInspBoardInfo->partImgBuf.imgTop_R, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TG, m_pInspBoardInfo->partImgBuf.imgTop_G, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TB, m_pInspBoardInfo->partImgBuf.imgTop_B, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_TW, m_pInspBoardInfo->partImgBuf.imgTop_W, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_MR, m_pInspBoardInfo->partImgBuf.imgMiddle_R, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_MB, m_pInspBoardInfo->partImgBuf.imgMiddle_B, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_BR, m_pInspBoardInfo->partImgBuf.imgBottom_R, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(ucImg_BB, m_pInspBoardInfo->partImgBuf.imgBottom_B, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	}
	/*sInspImgBuf->imgTop_R = new UCHAR[sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY];
	sInspImgBuf->imgTop_G = new UCHAR[sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY];
	sInspImgBuf->imgTop_B = new UCHAR[sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY];
	sInspImgBuf->imgBottom_R = new UCHAR[sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY];
	sInspImgBuf->imgBottom_B = new UCHAR[sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY];*/
	sInspImgBuf->imgTop_R = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgTop_G = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgTop_B = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgTop_W = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgMiddle_R = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgMiddle_B = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgBottom_R = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	sInspImgBuf->imgBottom_B = g_pMManager->pem_new<UCHAR>(true, sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(sInspImgBuf->imgTop_R, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgTop_G, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgTop_B, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgTop_W, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgMiddle_R, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgMiddle_B, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgBottom_R, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memset(sInspImgBuf->imgBottom_B, 0, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

	cv::Mat imgG;
	if (bAngleColor == TRUE)
	{
		LightTypeBuf sLightBuf;
		sLightBuf.m_pucTRed = ucImg_TR;
		sLightBuf.m_pucTGreen = ucImg_TG;
		sLightBuf.m_pucTBlue = ucImg_TB;
		sLightBuf.m_pucTWhite = ucImg_TW;
		sLightBuf.m_pucMRed = ucImg_MR;
		sLightBuf.m_pucMGreen = NULL;
		sLightBuf.m_pucMBlue = ucImg_MB;
		sLightBuf.m_pucMWhite = NULL;
		sLightBuf.m_pucBRed = ucImg_BR;
		imgG = m_pColorInsp->GetGreenImage(ucImg_MR, ucImg_MB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		sLightBuf.m_pucBGreen = imgG.data;
		sLightBuf.m_pucBBlue = ucImg_BB;
		sLightBuf.m_pucBWhite = NULL;
		sLightBuf.m_nImgWidth = sInspImgBuf->nImageSizeX;
		sLightBuf.m_nImgHeight = sInspImgBuf->nImageSizeY;
		sLightBuf.m_nROIImgWidth = sInspImgBuf->nImageSizeX;
		sLightBuf.m_nROIImgHeight = sInspImgBuf->nImageSizeY;
		sLightBuf.m_dROIX = 0;
		sLightBuf.m_dROIY = 0;
		/*sLightBuf.m_pnRedValue = new int[LIGHT_CNT];
		sLightBuf.m_pnGreenValue = new int[LIGHT_CNT];
		sLightBuf.m_pnBlueValue = new int[LIGHT_CNT];
		sLightBuf.m_pnWhiteValue = new int[LIGHT_CNT];
		sLightBuf.m_pnPosition = new int[LIGHT_CNT];
		sLightBuf.m_pnCalculation = new int[LIGHT_CNT];*/
		sLightBuf.m_pnRedValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnGreenValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnBlueValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnWhiteValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnPosition = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		sLightBuf.m_pnCalculation = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
		for (int a = 0; a < 3; a++)
		{
			sLightBuf.m_nImgCnt = m_sLightData[a].m_byLightCnt;
			memset(sLightBuf.m_pnRedValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnGreenValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnBlueValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnWhiteValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnPosition, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnCalculation, 0, LIGHT_CNT * sizeof(int));
			for (int b = 0; b < LIGHT_CNT; b++)
			{
				sLightBuf.m_pnRedValue[b] = m_sLightData[a].m_byArrLightData[eLightData_Red][b];
				sLightBuf.m_pnGreenValue[b] = m_sLightData[a].m_byArrLightData[eLightData_Green][b];
				sLightBuf.m_pnBlueValue[b] = m_sLightData[a].m_byArrLightData[eLightData_Blue][b];
				sLightBuf.m_pnWhiteValue[b] = m_sLightData[a].m_byArrLightData[eLightData_White][b];
				sLightBuf.m_pnPosition[b] = m_sLightData[a].m_byArrLightData[eLightData_Position][b];
				sLightBuf.m_pnCalculation[b] = m_sLightData[a].m_byArrLightData[eLightData_Calculation][b];
			}

			if (a == 0)
				m_InspAlgo.ROIImageClaculCompose(sLightBuf, sInspImgBuf->imgMiddle_R);
			else if (a == 1)
				m_InspAlgo.ROIImageClaculCompose(sLightBuf, sInspImgBuf->imgTop_W);
			else if (a == 2)
				m_InspAlgo.ROIImageClaculCompose(sLightBuf, sInspImgBuf->imgMiddle_B);
		}

		memcpy(sInspImgBuf->imgTop_R, ucImg_TR, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(sInspImgBuf->imgTop_G, ucImg_TG, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		memcpy(sInspImgBuf->imgTop_B, ucImg_TB, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

		Delete_1DArray(&sLightBuf.m_pnRedValue);
		Delete_1DArray(&sLightBuf.m_pnGreenValue);
		Delete_1DArray(&sLightBuf.m_pnBlueValue);
		Delete_1DArray(&sLightBuf.m_pnWhiteValue);
		Delete_1DArray(&sLightBuf.m_pnPosition);
		Delete_1DArray(&sLightBuf.m_pnCalculation);
	}
	else
	{
		if (byColorLightType == 1 && ucImg_MR != NULL && ucImg_MB != NULL && m_PInspWrapper != NULL && m_PInspWrapper->m_PInspAlgo != NULL)
		{
			memcpy(sInspImgBuf->imgTop_R, ucImg_MR, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_B, ucImg_MB, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);

			PIAL::PI_Buff* buf_R = new PIAL::PI_Buff(ucImg_MR, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX);
			PIAL::PI_Buff* buf_B = new PIAL::PI_Buff(ucImg_MB, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, sInspImgBuf->nImageSizeX);
			imgG = m_PInspWrapper->m_PInspAlgo->GetGreenImage(buf_R, buf_B);
			memcpy(sInspImgBuf->imgTop_G, imgG.data, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			delete buf_R; delete buf_B;
		}
		else
		{
			memcpy(sInspImgBuf->imgTop_R, ucImg_TR, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_G, ucImg_TG, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
			memcpy(sInspImgBuf->imgTop_B, ucImg_TB, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
		}
	}
	memcpy(sInspImgBuf->imgBottom_R, ucImg_BR, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	memcpy(sInspImgBuf->imgBottom_B, ucImg_BB, sizeof(UCHAR) * sInspImgBuf->nImageSizeX * sInspImgBuf->nImageSizeY);
	m_procMil->SaveWorkImg(sInspImgBuf->imgTop_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_TR.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgTop_G, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_TG.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgTop_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_TB.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgBottom_R, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_BR.bmp"));
	m_procMil->SaveWorkImg(sInspImgBuf->imgBottom_B, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY, _T("Color_Rst_BB.bmp"));
	Delete_1DArray(&ucImg_TR);
	Delete_1DArray(&ucImg_TG);
	Delete_1DArray(&ucImg_TB);
	Delete_1DArray(&ucImg_TW);
	Delete_1DArray(&ucImg_MR);
	Delete_1DArray(&ucImg_MB);
	Delete_1DArray(&ucImg_BR);
	Delete_1DArray(&ucImg_BB);
}

CPoint InspManager::CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, InspPartInfo* boardInfo)
{
	double dRetPixelX, dRetPixelY;
	return CvtBoradToPixel(ctFovCx, ctFovCy, ctRoiCx, ctRoiCy, ctRoiRealCx, ctRoiRealCy, dRetPixelX, dRetPixelY, boardInfo);
}

CPoint InspManager::CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, double& dRetPixelX, double& dRetPixelY, InspPartInfo* boardInfo)
{
	CPoint ret;
	double rstXpixel = 0.0;
	double rstYpixel = 0.0;

	int nFovWidth = m_fovWidth;
	int nFoVHeight = m_fovLength;
	double dResolX = m_resolX;
	double dResolY = m_resolY;

	double dFovCx = ctFovCx;
	double dFovCy = ctFovCy;
	double dRoiCx = ctRoiCx;
	double dRoiCy = ctRoiCy;

	//if (boardInfo != NULL && boardInfo->BtmCameraUse == 1)
	//{
	//	nFovWidth = boardInfo->BtmCameraWidth;
	//	nFoVHeight = boardInfo->BtmCameraHeight;

	//	dResolX = g_pMPTI->m_dBtmSideResX;
	//	dResolY = g_pMPTI->m_dBtmSideResY;

	//	double fovWidth = 0;
	//	double fovLength = 0;
	//	double rstXmm = 0.0;
	//	double rstYmm = 0.0;

	//	fovWidth = (nFovWidth/*m_fovWidth*/ * dResolX);  //unit : pixel -> mm
	//	fovLength = (nFoVHeight/*m_fovLength*/ * dResolY);  //unit : pixel -> mm


	//	double fovCX = ctFovCx; //unit : mm
	//	double fovCY = ctFovCy; //unit : mm
	//	//20141010 SHW : TEST 
	//	double srcPointX = ctRoiCx;
	//	double srcPointY = ctRoiCy;

	//	rstXmm = -fovCX + (fovWidth / 2.0) + srcPointX;  //unit : mm
	//	rstYmm = fovCY + (fovLength / 2.0) - srcPointY;  //unit : mm

	//	double rstXpixel = rstXmm * (/*m_fovWidth*/nFovWidth / fovWidth);   //unit : pixel
	//	double rstYpixel = rstYmm * (/*m_fovLength*/nFoVHeight / fovLength);  //unit : pixel

	//	ret.SetPoint(RounD(rstXpixel), RounD(rstYpixel));
	//}
	//else
	{
		double dCamCenterX = ((double)nFovWidth - 1) / 2.0;
		double dCamCenterY = ((double)nFoVHeight - 1) / 2.0;

		double dOffsetX = (dFovCx - dRoiCx) / dResolX;
		double dOffsetY = (dFovCy - dRoiCy) / dResolY;

		rstXpixel = dCamCenterX - dOffsetX;
		rstYpixel = dCamCenterY + dOffsetY;

		/*if (g_pMPTI->m_bUseLensDistortion)
		{
			dOffsetX = dCamCenterX - (dFovCx - ctRoiRealCx) / dResolX;
			dOffsetY = dCamCenterY + (dFovCy - ctRoiRealCy) / dResolY;

			double dstX = dOffsetX, dstY = dOffsetY;
			MPTI_LensCalcRefToObj(dOffsetX, dOffsetY, &dstX, &dstY);

			dOffsetX = dstX - dOffsetX;
			dOffsetY = dstY - dOffsetY;

			dRetPixelX = rstXpixel + dOffsetX;
			dRetPixelY = rstYpixel + dOffsetY;
		}
		else*/
		{
			dRetPixelX = rstXpixel;
			dRetPixelY = rstYpixel;
		}
		ret.SetPoint(RounD(dRetPixelX), RounD(dRetPixelY));
	}

	return ret;
}
bool InspManager::CheckAlgoGroup(int wndType, int nWndIndex, int nAlgoIdx, InspWndResult* WndResult)
{
	bool ret = true;
	InspAlgo sCurInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgoIdx];

	//unsigned long long nAlgoData = m_pInspDecision->GetInspAlgoData(sCurInspAlgo.m_eAlgoType);

	unsigned long long nAlgoData = m_dicInspAlgorithm[sCurInspAlgo.m_eAlgoType]->GetInspAlgoData();

	if ((m_nInspSkipAlgo[wndType] & nAlgoData) == nAlgoData)
		return false;
	if (m_pParamArray[nWndIndex].nAlgorithmCnt == 1 || !WndResult->m_vArrRstInspAlgo || nAlgoIdx == 0 ||
		m_bInspSkipGroupAlgo[wndType] == FALSE || sCurInspAlgo.m_bAlgoEnable == FALSE || sCurInspAlgo.m_bAlgoGroup == FALSE ||
		sCurInspAlgo.m_bIsRequired)
		return ret;
	int nResGroup = e_NG;
	BOOL bIsEssentialInGroup = FALSE;
	for (int nAlgo = 0; nAlgo < nAlgoIdx; nAlgo++)
	{
		InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
		if (sInspAlgo.m_bAlgoGroup == FALSE || WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk == FALSE)
			continue;
		if (sInspAlgo.m_bAlgoGroup && sInspAlgo.m_bIsRequired)
			bIsEssentialInGroup = TRUE;
		nResGroup = e_OK;
	}
	if (nResGroup == e_OK) ret = false;
	return ret;
}
int InspManager::ShaerdInspection()
{
//	//1. Eagle 3D 검사시작. 공정정보 Setting
//	{
//		ext::Sync sync;
//		int temp = ext::irc::get()->_CtrlServer.First()->signals;
//		if (( (ext::irc::get()->_CtrlServer.First()->signals) & ext::eCtrlSignal::eAutoMode) != 0)
//			SetExtProductionInfo();
//	}
//	
//
//	for (int i = 0; i < m_vPartIndex.size(); i++) // 미리지정된 파트만큼 반복. Init 떄 결정
//	{
//		// 2. Algo Tool Ready	
//		{
//			ext::Sync sync;
//			ext::irc::get()->_Ctrl.First()->signals |= ext::eCtrlSignal::eReady;
//			ext::irc::get()->_Ctrl.UpdateAll();
//		}
//
//		// 3. Buffer Full 대기
//		int & nCurrIdx = ext::irc::get()->_Ctrl.First()->CurrCalcPartIdx;
//		{
//			
//			ext::InspStatus * ipn = ext::irc::get()->_InspStatus.First();
//			
//			
//
//			while (true)
//			{
//				if (ipn[nCurrIdx].flag == ext::InspFlag::IF_BufFull)
//				{
//					// 4. Algo Tool Inspection
//					{
//						if (((ext::irc::get()->_Ctrl.First()->signals) & ext::eCtrlSignal::eReady) != 0)
//						{
//							ext::Sync sync;
//							ext::irc::get()->_Ctrl.First()->signals &= ~ext::eCtrlSignal::eReady; // 아닐떄만 업데이트 
//							ipn[nCurrIdx].flag = ext::InspFlag::IF_BufCalc;
//							ext::irc::get()->_Ctrl.UpdateAll();
//						}
//							
//					}
//
//					SetExtInspParam();
//					InspNormal(true);
//					break;
//				}
//			}
//		}
//		
//		
//// 			m_pInspBoardInfo = boardInfo;
//// 			m_pParamArray = pParamArray;
//// 			m_ParamArraySize = nParamArraySize;
//			m_pInspBoardInfo->ImagePath;
//		
//		//4. Save Image 
//		//검사 후 제어부 관리 수정
//		//5. Result Eagle 3D 로 
//		{
//			ext::irc::get()->_Result.AddResult(m_inspectionResult);
//			ext::Sync sync;
//			ext::InspStatus * ipn = ext::irc::get()->_InspStatus.First();
//			ipn[nCurrIdx].flag = ext::InspFlag::IF_BufDone;
//
//			ext::irc::get()->_Ctrl.First()->CalcPartCount++;
//			ext::irc::get()->_InspStatus.Update(nCurrIdx);
//			ext::irc::get()->_Ctrl.UpdateAll();
//			DeleteResultVal(m_inspectionResult);
//		}
//	}
	return 0;
}

int InspManager::SetExtProductionInfo()
{
	// 기존 방식으로 일단 사용 추후 변경 예정

	int nTotalCnt = 0;
	CString strTemp;

	//ext::ExtProductionInfo * prodInfo;
	//memcpy (prodInfo, (ext::ProductionInfo * )&ext::irs::get()->_Ctrl.First()->prod,sizeof(ext::ProductionInfo));

	m_ExtProdInfo = &ext::irc::get()->_CtrlServer.First()->prod;
	g_pMPTI->SetSizeBoard(m_ExtProdInfo->nBoardSizeX, m_ExtProdInfo->nBoardSizeY);

	//로그 레벨 set
	g_pMPTI->SetLogLv(m_ExtProdInfo->nLogLv);

	strTemp.Format(_T("Calibration : %d"), (m_ExtProdInfo->bUseCalibration ? 1 : 0));
	ext::Log::add(strTemp);

	strTemp.Format(_T("Start Job : %s"), m_ExtProdInfo->sJobPath);
	ext::Log::add(strTemp);

	strTemp.Format(_T("Log Level : %d"), m_ExtProdInfo->nLogLv);
	ext::Log::add(strTemp);

	strTemp.Format(_T("Set Ext Product : Resolution,X,%0.17g,Y,%0.17g"), m_ExtProdInfo->resolX, m_ExtProdInfo->resolY);
	ext::Log::add(strTemp);

	
	//파트 검사 갯수
	nTotalCnt = m_ExtProdInfo->partTotalCount;
	//ext::irc::get()->_InspStatus.SetTotalPartCount(m_vPartIndex.size());
	if (m_ExtProdInfo->bUseCalibration)
	{
		strTemp.Format(_T("Calibration No Inspection ") );
		ext::Log::add(strTemp);
		return 2;

	}
		

	SetExtAngleColorParam();

	CPInspDecision* pDecision = (CPInspDecision*)m_pInspDecision;
	if (pDecision)
	{
		pDecision->SetPriorityCode(m_ExtProdInfo->nArrPriorityCode);
	}

	MPTI_Set2DChannel_LightPos(m_ExtProdInfo->nRedLgtPos_2DImg, m_ExtProdInfo->nGreenLgtPos_2DImg,
		m_ExtProdInfo->nBlueLgtPos_2DImg);

// 	//Image Save 관련
// 	prodInfo->nImgNameRule
// 	prodInfo->nImgPathRule
// 	prodInfo->cImageRootPath
// 	prodInfo->bImgSave2DCheck
// 	prodInfo->n2DSaveQuality
	// 공정정보로 대체 해서 넣을것,
	ext::ExtAlgoMachineInfo stMachine = ext::irc::get()->_CtrlServer.First()->machineAlgo;
	for (int i = 0; i < 13; i++)
	{
		m_bInspSkipGroupAlgo[i] = stMachine.stExtGroupSkipData.m_bArrSkipGroupAlgo[i];
		m_bInspSkipGroupWnd[i] = stMachine.stExtGroupSkipData.m_bArrSkipGroupWnd[i];
		m_nInspSkipAlgo[i] = stMachine.stExtGroupSkipData.m_bArrSkipAlgo[i];
	}
	SetExtMachineAlgoParam();
	CString sSetLog;


	m_bSharedInOutImageSave = m_ExtProdInfo->bSharedInOutImageSave;
	//if (ext::ClientCtrl::get()->m_nToolType == (int)ext::eToolType::eBIG)
		m_bMultiProcessThreadSave = m_ExtProdInfo->bMultiProcessThreadSave;
	//else
	//	m_bMultiProcessThreadSave = 0; // bigpart 만 지원함
	m_bForeignDebugSave = m_ExtProdInfo->bForeignDebugSave;

	sSetLog.Format(_T("Shared Inout Image Save %d , MultiProcess ThreadSave Option %d"), m_bSharedInOutImageSave,m_bMultiProcessThreadSave);
	ext::Log::add(sSetLog);
	//if(!(m_ExtProdInfo->nFovSizeX == m_fovWidth && m_ExtProdInfo->nFovSizeY == m_fovLength ))
	//{
	//	SetResolution(m_ExtProdInfo->nFovSizeX, m_ExtProdInfo->nFovSizeY, m_ExtProdInfo->resolX, m_ExtProdInfo->resolY);
	////Machine Algo Option
	//	SetExtMachineAlgoParam();
	//}
	//else
	//if(0)
	{
		ext::Log::add(_T("[CHECK]Start"));
		bool rst = GetPocrTeach()->m_procPil->ModelList_Check(_T(""));
		CString sOCRLog;
		sOCRLog.Format(_T("[CHECK]ModelListLoad %d"), rst);
		ext::Log::add(sOCRLog);

		bool rstPattern = GetPatternInsp()->GetProcMil()->ExtModelList_Check();

		//GetPocrTeach()->m_procPil->ModelList_Load(_T(""));
		CString sLog;
		sLog.Format(_T("[CHECK]ModelListLoad Pattern %d"), rstPattern);
		ext::Log::add(sLog);
		CPInsp_AlgoShapeX* inspShape = dynamic_cast<CPInsp_AlgoShapeX*>(g_pInspMng->m_dicInspAlgorithm[eAlgoShapeX]);
		inspShape->CheckModelList(nullptr, true);
		
	
		sLog.Format(_T("[Check]ModelListLoad ShapeX"));
		ext::Log::add(sLog);

		int nOwnToolType = ext::ClientCtrl::get()->m_nToolType;
		if (nOwnToolType == (int)ext::eToolType::eBIG)		//halcon 은 License Check 와 동시에 Load 진행 --> DLL
		{
			int check = m_PInspWrapper->m_PInspAlgo->PInspAlgo::CheckHalconLicense();
			if (check != 1)
			{
				if (g_pMPTI)
				{
					CString sLog = _T("");
					sLog.Format(_T("POCR - HALCON License Error. (error code: %d)"), check);
					ext::Log::add(sLog);
				}
			}
		}
	}

	g_pMPTI->POCR_UseAI_Set(0, FALSE, false, 0, m_ExtProdInfo->nSegThreshold);

 	return 1;
}
//#define IMAGEBUFFERTEST
int InspManager::SetExtInspParam(int nBufferIndex)
{
	int & nCurrIdx = ext::ClientCtrl::get()->_ctrl.First()->CurrCalcPartIdx;
	//InspPartInfo & boardInfo = ext::irc::get()->_Job.parts[nPart];
	//InspPartParam * partParam = ExtPrmPtr(boardInfo.pWindows);
	
	InspPartInfo * pDstPart = new InspPartInfo();
	InspPartParam * pDstWindow = new InspPartParam();

	//ext::ClientCtrl::get()->_PartData[0].GetPartData(*&pDstPart, *&pDstWindow);
	//ext::ClientCtrl::get()->_PartData[0].GetLight(*&pDstWindow, pDstPart->nWindowCount);
	ext::ClientCtrl::get()->GetSharedData(pDstPart, pDstWindow, nBufferIndex);
	int  nWindowCnt = pDstPart->nWindowCount;
	if (m_bSharedInOutImageSave)
	{
		CString str;
		str.Format(_T("C:\\Eagle3D_64x\\PROGRAM\\AlgoTool\\Test\\%d_B_AlgoTool.bmp"), pDstPart->nPartIDOrg);
		cv::Mat CR(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.GetPtrImg(0) /*_MasterBuffer[nBufferIndex]._ChannelBuffer[0]._stream->Byte()*/);
		CT2CA strWndAlgoName(str);
		ext::Log::add(str);
		std::string strAlgoName(strWndAlgoName);
		imwrite(strAlgoName, CR);
	}

// 	cv::Mat CR(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_R);
// 	cv::Mat CB(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_B);
// 	cv::Mat CG(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_G);
// 	cv::Mat CW(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_W);
// 	cv::Mat CMR(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgMiddle_R);
// 	cv::Mat CMB(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgMiddle_B);
// 	cv::Mat CBR(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgBottom_R);
// 	cv::Mat CBB(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgBottom_B);
// 	cv::Mat CB3D(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_32FC1, pDstPart->partZmapData.data);
#ifdef IMAGEBUFFERTEST
	cv::Mat CR(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[0]._stream->Byte());
	cv::Mat CB(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[1]._stream->Byte());
	cv::Mat CG(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[2]._stream->Byte());
	cv::Mat CW(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[3]._stream->Byte());
	cv::Mat CMR(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[4]._stream->Byte());
	cv::Mat CMB(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[5]._stream->Byte());
	cv::Mat CBR(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[6]._stream->Byte());
	cv::Mat CBB(1408, 1050, CV_8UC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[7]._stream->Byte());
	cv::Mat CB3D(1408, 1050, CV_32FC1, ext::ClientCtrl::get()->_MasterBuffer[0]._ChannelBuffer[8]._stream->Float());
#endif
	//for (int i = 0; i < (int)InspRoiImgBuf_no::eInspRoiImg_NUM; i++)
	//{
	//	if (i >= EXT_3D_BUFFER_INDEX)
	//		break;
	//	pDstPart->partImgBuf.GetPtrImg(i) = ext::ClientCtrl::get()->_MasterBuffer[nBufferIndex]._ChannelBuffer[i]._stream->Byte();
	//}
	//pDstPart->partZmapData.data = ext::ClientCtrl::get()->_MasterBuffer[nBufferIndex]._ChannelBuffer[EXT_3D_BUFFER_INDEX]._stream->Float();
// 	ExtPrmPtr(boardInfo.partImgBuf).imgTop_R = imgPtr;
// 	ExtPrmPtr(boardInfo.partImgBuf).imgTop_G = imgPtr;
// 	ExtPrmPtr(boardInfo.partImgBuf).imgTop_B = imgPtr;
// 	ExtPrmPtr(boardInfo.partImgBuf).imgTop_W = imgPtr;
// 	ExtPrmPtr(boardInfo.partImgBuf).imgMiddle_R = imgPtr;
// 	ExtPrmPtr(boardInfo.partImgBuf).imgMiddle_B = imgPtr;
// 	ExtPrmPtr(boardInfo.partImgBuf).imgBottom_R = imgPtr;
// 	ExtPrmPtr(boardInfo.partImgBuf).imgBottom_B = imgPtr;

	////이미지 잘넘어오는지 확인용도.
	//cv::Mat partimg(ExtPrmPtr(boardInfo.partImgBuf).nImageSizeY, ExtPrmPtr(boardInfo.partImgBuf).nImageSizeX, CV_8UC1, imgPtr);
	SetInspParam(pDstPart, pDstWindow, nWindowCnt);
	SortingParamater(pDstWindow, nWindowCnt);
	////cvResultImage = cv::Mat::zeros(ExtPrmPtr(boardInfo.partImgBuf).nImageSizeY, ExtPrmPtr(boardInfo.partImgBuf).nImageSizeX,CV_8UC1);
	return 1; 
}
int InspManager::SetExtInspParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nBufferIndex)
{
	int & nCurrIdx = ext::ClientCtrl::get()->_ctrl.First()->CurrCalcPartIdx;
	ext::ClientCtrl::get()->GetSharedData(boardInfo, pParamArray, nBufferIndex);
	int  nWindowCnt = boardInfo->nWindowCount;

	SetInspParam(boardInfo, pParamArray, nWindowCnt);
	SortingParamater(pParamArray, nWindowCnt);

	return 1;
}
int InspManager::GetExtForeignInspParam(InspPartInfo* _ForePartInfo, ForeignData* _foreinData, FR2DData* Fr2Data, InspForeignInfo* m_ForeignInfoData, ForeignParamROI ** _ForeParamROI, ForeignParamROI ** _ArrROI, int nSelectedIdx)
{
	ext::ClientCtrl::get()->GetShared_ForeignData(_ForePartInfo, _foreinData, Fr2Data, m_ForeignInfoData, _ForeParamROI, _ArrROI, nSelectedIdx);
	// 변수들이랑 포인터 연결 
	m_FR.SetPartParam_Foreign(_ForePartInfo);
	//m_pInspBoardInfo_Foreign = _ForePartInfo;
	m_ForeignData = *m_ForeignInfoData;
	//m_ForeignParamROI =  _ForeParamROI;
	m_FR.SetForeignParam(*m_ForeignInfoData, * _ForeParamROI);

	// Mutlti Member Pointer 들과 연결 

	return 1;
}
int InspManager::SaveInspPartImage(CString * strInspectStart, InspPartInfo* boardInfo, InspPartParam *pParamArray, int nBuffIdx)
{
	//2D 만 작업되어있음.
	int ret = 0;
	//CString str2dFullPath = _T("");
	//str2dFullPath = (m_inspectionResult->nDefectType == defectCode::dftCODE_OK) ? m_pInspBoardInfo->sOKImagePath : m_pInspBoardInfo->sNGImagePath;
// 	CString sTemp;
// 	sTemp.Format(_T("2: %s\n"), m_pInspBoardInfo->sNGImagePath);
	//ext::Log::add((sTemp));
	CString str2dFullPath = boardInfo->s2DImagePath;//_T("D:\\Eagle3D_data\\InspectResult\\Image\\mobis_Wire\\wire_1115\\20230308124734\\1@2");
	CString str3dFullPath = boardInfo->s3DImagePath;// _T("D:\\Eagle3D_data\\InspectResult\\Image\\mobis_Wire\\test\\wire_1115\\20230308124734\\1@3");//

	int nRIndex = str2dFullPath.ReverseFind('\\');
	int pos = 0;
	CString delimiter = _T("\\");
	CString token; 
	std::vector< CString> strPath; 
	while ((token = str2dFullPath.Tokenize(delimiter, pos)) != "") // Module 안넘겨줘
		strPath.emplace_back(token);
	if (strPath.size() == 0) return 0;  // 2D 가 빈값으로 들어오면 문제가 있으니 여기서 걸러줌.
	CString sModuleNo  = strPath[strPath.size()-1 ].Mid(0, 1); //Module 
	CString sInspctStart = strPath[strPath.size() - 2];


	CString s2DName = str2dFullPath.Mid(str2dFullPath.ReverseFind('\\')+1);
	CString s2DPath = str2dFullPath.Left(str2dFullPath.ReverseFind('\\'));

	CString s3DName = str3dFullPath.Mid(str3dFullPath.ReverseFind('\\')+1);
	CString s3DPath = str3dFullPath.Left(str3dFullPath.ReverseFind('\\'));
	int nSaveImage2D = 0;
	int nSaveImage3D = 0;
	bool b2DImageSave = false;
	
	if (m_ExtProdInfo->bRunSaveOK && (m_inspectionResult->nDefectType == 0))
	{
		if (m_ExtProdInfo->b3DImageOK)
		{
			nSaveImage3D = 1;
		}

		if (m_ExtProdInfo->b2DImageOK)
		{
			nSaveImage2D |= 0x00000001;
			b2DImageSave = true;
		}
			


	}
	if (m_ExtProdInfo->bRunSaveNG && (m_inspectionResult->nDefectType != 0))
	{
		if (m_ExtProdInfo->b3DImageNG)
		{

			nSaveImage3D = 1;
		}
		if (m_ExtProdInfo->b2DImageNG)
		{
			nSaveImage2D |= 0x00000001;
			b2DImageSave = true;
		}

	}

	
	if (m_pInspBoardInfo->partImgBuf.nImageSizeX *  m_pInspBoardInfo->partImgBuf.nImageSizeY > m_ExtProdInfo->nKeep3DImageSize)
		nSaveImage2D |= 0x00000002;

	//if (g_pMPTI->GetExtMachineType() == (int)ext::eMachineType::eWIR)
	{
		//nSaveImage3D = 0;
		if (m_inspectionResult->nDefectType != 0)
		{
			int n = s2DPath.Find(_T("OKImage")); // OK 이미지가 패스명에있으면 
			if(n != -1 )
				s2DPath.Delete(n, 7); // 잘라줌
		}
		else
		{
			int n = s2DPath.Find(_T("\\OKImage")); //OK 인대 패스명에없으면
			if (n == -1)
				s2DPath.AppendFormat(_T("\\OKImage")); // 붙혀줌.

		}

	}
	//bool b2DImageSave = ((m_inspectionResult->nDefectType == 100/* defectCode.dftCODE_OK*/) & nSaveImage2D) || ((m_inspectionResult->nDefectType != 100/*defectCode.dftCODE_OK*/) & !nSaveImage2D);

#define  SaveTest 1 
	SaveImgParamData tgparam;
#if SaveTest 
	//InspRstDtCtrl.sInspctStart, InspRstDtCtrl.Barcode
	CString strRawTemp;
	strRawTemp.Format(_T("D:\\Temp\\Eagle3d\\rawdata3d\\%s@"), sInspctStart,sModuleNo);
	 
	tgparam.unNGType = m_inspectionResult->nDefectType; // m_inspectionResult->nDefectType
	tgparam.nSave_2d = nSaveImage2D;// m_ExtProdInfo->nImgSave2DCheck;//m_ExtProdInfo->nImgSave2DCheck
	tgparam.nSave_3d = nSaveImage3D;
	tgparam.nMSPtImg = 0;
	tgparam.nSave_put = 0;
	tgparam.nSave_AOILink = 0;
	tgparam.nSave_NextMC = 0;
	tgparam.m_nSave_MES = 0;
	tgparam.m_nSave_APC = 0;
	tgparam.m_nSave_Color = m_ExtProdInfo->nAngleColor;
	// tgparam.m_nSave_Color = 1; // AngleColor 는 현재 사용 x 
	tgparam.m_nSave_3DRaw = m_ExtProdInfo->m_nSave_3DRaw;
	tgparam.m_nSave_ImgType = -1;
	//tgparam.m_nSave_ImgType = -1; // uv 
	tgparam.unSaveOption = m_ExtProdInfo->unSaveOption;
	tgparam.m_n2DSaveQuality = m_ExtProdInfo->n2DImageQuality;
	wstring temp = L"";
	wcscpy(tgparam.arrNGCaption, temp.c_str());
	temp = s2DName.operator LPCWSTR();//L"1@2";
	wcscpy(tgparam.arrJpgFileName, temp.c_str());

	CString s3DRawName;
	s3DRawName.Format(_T("%s@%s"), sModuleNo, m_pInspBoardInfo->s3DRawName);
	//wcscpy(tgparam.arr3dRawFileName, temp.c_str()); // 2D 꺼 가져감
	wcscpy(tgparam.arr3dRawFileName, s3DRawName);

	tgparam.m_nMachineCode = m_ExtProdInfo->m_nMachineCode;
	wcscpy(tgparam.arrJobPath, m_ExtProdInfo->sJobPath);
	tgparam.ColorMode = m_ExtProdInfo->ColorMode;
	temp = s3DName.operator LPCWSTR();
	wcscpy(tgparam.arr3dFileName, temp.c_str());
// 	temp = L"D:\\Eagle3D_data\\InspectResult\\Image\\mobis_Wire\\wire_1115\\20230307114235";
	temp = (m_inspectionResult->nDefectType == 0)? s2DPath.operator LPCWSTR() : s3DPath.operator LPCWSTR();
	wcscpy(tgparam.arrJpgFilePath		, temp.c_str());
	temp = (m_inspectionResult->nDefectType == 0) ? s2DPath.operator LPCWSTR() : s3DPath.operator LPCWSTR();
	wcscpy(tgparam.arr3dFilePath		, temp.c_str());

	wcscpy(tgparam.arr3dRawFilePath, temp.c_str());
// 	wcscpy(tgparam.arr3dFilePathTemp	, temp.c_str());
// 	wcscpy(tgparam.arrPutFilePath		, temp.c_str());
// 	wcscpy(tgparam.arrColorMap2FilePath, temp.c_str());
// 	temp = L"D:\\Temp\\Eagle3d\\rawdata3d\\20230307114235@";
	temp = strRawTemp.operator LPCWSTR();
	wcscpy(tgparam.arr3dRawFilePath		, temp.c_str());

	

// 	temp = L"D:\\Eagle3D_data\\AI_Sample\\LeadPartImage\\20230307";
// 	wcscpy(tgparam.arr3dLeadOKFilePath	, temp.c_str());

	//tgparam.pArrSideImg
	NgParam ngColorParam;
	if (g_pMPTI->GetExtMachineType() == (int)ext::eMachineType::eWIR)
	{
		ngColorParam.redFator = 1.0;
		ngColorParam.greenFator = 1.0;
		ngColorParam.blueFator = 1.0;
		ngColorParam.btmRedfactor = 1.0;
		ngColorParam.btmBluefactor = 1.0;
	}
	else
	{
	ngColorParam.redFator = m_ExtProdInfo->fFactor_TR;
	ngColorParam.greenFator = m_ExtProdInfo->fFactor_TG;
	ngColorParam.blueFator = m_ExtProdInfo->fFactor_TB;
	ngColorParam.btmRedfactor = m_ExtProdInfo->fFactor_BR;
	ngColorParam.btmBluefactor = m_ExtProdInfo->fFactor_BB;
	}
	m_NgManager->SetNgParam(ngColorParam);

#endif


	if (m_bMultiProcessThreadSave)
	{
		MultiStruct* multiSave = new MultiStruct();
		SaveImgParamData * tgSaveParam = new  SaveImgParamData();
		tgSaveParam->Copy(tgparam);
		multiSave->nRoiArea = m_NgManager->GetROISize(multiSave->nRoiSizeX, multiSave->nRoiSizeY, multiSave->nWidthStep);
		std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool , int > TupleSaveImages;
		//InspPartInfo * infoTemp = new ;
		//memcpy(infoTemp, boardInfo, InspPartInfo
		TupleSaveImages = (std::make_tuple(boardInfo, tgSaveParam, multiSave, b2DImageSave , nBuffIdx));
		// Copy 를 최소화 하기위한 Tuple
		m_ImgStack.push(TupleSaveImages);
	}
	else
		m_NgManager->SaveInspPartImage(tgparam, nullptr, b2DImageSave, nullptr, false ,false,0, m_pInspBoardInfo, m_inspectionResult);
	if (m_bSharedInOutImageSave && b2DImageSave)
	{
		CString sLog; 
		sLog.Format(_T("Name : %s Path: %s"), s2DName, s2DPath);
		ext::Log::add(sLog);
	}
	
// 	if (m_ExtProdInfo->nImgSave2DCheck == 1)
// 	{
// 		
// 		//CreateDir(str2dFullPath);
// 		str2dFullPath.Format(_T("%s.jpg"), str2dFullPath);
// 		cv::Mat cvSaveImg(ExtPrmPtr(m_pInspBoardInfo->partImgBuf).nImageSizeY, ExtPrmPtr(m_pInspBoardInfo->partImgBuf).nImageSizeX, CV_8UC1, ExtPrmPtr(m_pInspBoardInfo->partImgBuf).imgTop_R);		
// 		cv::imwrite(std::string(CT2CA(str2dFullPath)), cvSaveImg);
// // 		cv::Mat aTest[] = { cvSaveImg, cvSaveImg,cvSaveImg };
// // 		cv::Mat out;
// // 		cv::merge(aTest, 3, out);
// // 
// // 		m_Jpeg.PIL_Save_Jpeg(
// // 			out.data,
// // 			cvSaveImg.cols,
// // 			cvSaveImg.rows,
// // 			3, 
// // 			IC_BGR,
// // 			str2dFullPath,
// // 			m_ExtProdInfo->n2DSaveQuality
// // 			);//UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, CString  file_path, int nJpegQuality);
// 	}
	return ret;
}
BOOL InspManager::SetExtMachineAlgoParam()
{
	//차후 PInspAlgo쪽에 Virtual로 알고리즘마다 머신옵션 혹은 추가 변수가잇을시 받을 수 있도록 수정 예정
	BOOL bRet = TRUE;

	ext::ExtAlgoMachineInfo stMachine = ext::irc::get()->_CtrlServer.First()->machineAlgo;

	//BodyBlob
	m_PInspWrapper->SetBodyBlobValue(stMachine.stExtBodyBlob.dSearchPer_Default, stMachine.stExtBodyBlob.dSearchPer_BodyTip, stMachine.stExtBodyBlob.nBodyBlobPoint, stMachine.stExtBodyBlob.nBodyBlobOPT);
	g_pInspMng->GetPtrInspAlgo()->m_inspMount.m_dBodyBlobSearchPer_BodyTip = stMachine.stExtBodyBlob.dSearchPer_BodyTip;
	g_pInspMng->GetPtrInspAlgo()->m_inspMount.m_nAlgoPoint = stMachine.stExtBodyBlob.nBodyBlobPoint;
	g_pInspMng->GetPtrInspAlgo()->m_inspMount.m_nBodyBlobOPT = stMachine.stExtBodyBlob.nBodyBlobOPT;
	memcpy(g_pInspMng->GetPtrInspAlgo()->m_inspMount.m_dSearchPer_Default, &stMachine.stExtBodyBlob.dSearchPer_Default, sizeof(double) * BODYBLOB_DEFAULT);

	//POCRFontInfo
	SetExtPOCRFontInfo();

	CPInsp_WireBonding* PInspWireBonding = m_pWireBondingInsp;// g_pMPTI->GetWireBondingInsp();

	if (PInspWireBonding == NULL)
	{
		ext::Log::add(_T("Wirebonding Null"));
		return false;
	}
		
	PInspWireBonding->SetWireOption(stMachine.stExtFootWire.FootPadTopWGrayLevel, stMachine.stExtFootWire.FootPatternMatchScore,
		stMachine.stExtFootWire.FootMatch2D, stMachine.stExtFootWire.FootMatch3D,
		stMachine.stExtFootWire.FootSobmn, stMachine.stExtFootWire.FootSobmx,
		stMachine.stExtFootWire.FootPatternMatchStopScore);

	PInspWireBonding->m_fDiffHigh = stMachine.stExtFootWire.fWireDiffHigh;//15;//40;
	PInspWireBonding->m_fDiffLow = stMachine.stExtFootWire.fWireDiffLow;// 15;//40;
	PInspWireBonding->m_fDiameterWire = stMachine.stExtFootWire.fDiameterWire;



	return bRet;
}

BOOL InspManager::SetModelLoad()
{
	//ModelLoad
	bool rstOCR = GetPocrTeach()->m_procPil->ModelList_Load(_T(""));
	CString sLog;
	sLog.Format(_T("[LOAD]ModelListLoad POCR %d"), rstOCR);
	ext::Log::add(sLog);

	bool rstPattern = GetPatternInsp()->GetProcMil()->ExtModelList_Load();

	//GetPocrTeach()->m_procPil->ModelList_Load(_T(""));

	sLog.Format(_T("[LOAD]ModelListLoad Pattern %d"), rstPattern);
	ext::Log::add(sLog);
	
	/*if (g_pMPTI->GetExtMachineType() == (int)ext::eMachineType::eWIR)
	{*/
		CPInsp_AlgoPatternDiff* inspCOB = dynamic_cast<CPInsp_AlgoPatternDiff*>(g_pInspMng->m_dicInspAlgorithm[eAlgoPatternDiff]);
		inspCOB->MapClear(); // 스타트 시 클리어.

		int nTotalCnt = ext::InspRoot_Server::get()->_InspStatus.ModelTotalCount(ext::ModelBufferFlag::eExtModel_COB); // COB안에 Add 된 Pattern or Mat 갯수 
		for (int i = 0; i < nTotalCnt; i++)
		{
			ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_COBModelCtrl.First()[i];
			bool rstCOBLoad = inspCOB->LoadExtModel(pCtrl, ext::InspRoot_Server::get()->_COBModelBuffer.Lock(pCtrl.img),false); // 한개 의 모델 가지고있어서 True 경우 Load 될경우 더이상 진행하지않음.

			if (rstCOBLoad)
			{
				CString sLog;
				sLog.Format(_T("[LOAD]ModelListLoad COB %d - %s "), rstCOBLoad, CString(pCtrl.sFilePath));
				ext::Log::add(sLog);
		}
	}
	// Pattern diff 의 Chipping 예외 모델
	nTotalCnt = ext::InspRoot_Server::get()->_InspStatus.ModelTotalCount(ext::ModelBufferFlag::eExtModel_Exc);
	for (int i = 0; i < nTotalCnt; i++)
	{
		ext::ModelStatus pCtrlExc = ext::InspRoot_Server::get()->_ExcModelCtrl.First()[i];
		bool rstExcLoad = inspCOB->LoadExtModel(pCtrlExc, ext::InspRoot_Server::get()->_ExcModelBuffer.Lock(pCtrlExc.img), false);
		if (rstExcLoad)
		{
			CString sLog;
			sLog.Format(_T("[LOAD]ModelListLoad COB %d - %s "), rstExcLoad, CString(pCtrlExc.sFilePath));
			ext::Log::add(sLog);
			//break;
		}
	}

	nTotalCnt = ext::InspRoot_Server::get()->_InspStatus.ModelTotalCount(ext::ModelBufferFlag::eExtModel_Exc);
	for (int i = 0; i < nTotalCnt; i++)
	{
		ext::ModelStatus pCtrlExc = ext::InspRoot_Server::get()->_ExcModelCtrl.First()[i];
		bool rstExcLoad = inspCOB->LoadExtModel(pCtrlExc, ext::InspRoot_Server::get()->_ExcModelBuffer.Lock(pCtrlExc.img), false);
		if (rstExcLoad)
		{
			CString sLog;
			sLog.Format(_T("[LOAD]ModelListLoad COB %d - %s "), rstExcLoad, CString(pCtrlExc.sFilePath));
			ext::Log::add(sLog);
			//break;
		}
	}
	CPInsp_AlgoShapeX* inspShape = dynamic_cast<CPInsp_AlgoShapeX*>(g_pInspMng->m_dicInspAlgorithm[eAlgoShapeX]);
	inspShape ->LoadModelList(nullptr,true);


	sLog.Format(_T("[Load]ModelListLoad ShapeX"));
	ext::Log::add(sLog);

	
		//Clear Last OK Model index(Each Tool)
		inspCOB->ClearModelIdx(0);

		CPInsp_AlgoNGBlob* inspNgBlob = dynamic_cast<CPInsp_AlgoNGBlob*>(g_pInspMng->m_dicInspAlgorithm[eAlgoNGBlob]);
		inspNgBlob->MapClear(); // 스타트 시 클리어.

		nTotalCnt = ext::InspRoot_Server::get()->_InspStatus.ModelTotalCount(ext::ModelBufferFlag::eExtModel_NGBlobExc);
		for (int i = 0; i < nTotalCnt; i++)
		{
			ext::ModelStatus pCtrlExc = ext::InspRoot_Server::get()->_ExcNGBlobModelCtrl.First()[i];
			bool rstExcLoad = inspNgBlob->LoadExtModel(pCtrlExc, ext::InspRoot_Server::get()->_ExcNGBlobModelBuffer.Lock(pCtrlExc.img), false);
			if (rstExcLoad)
			{
				CString sLog;
				sLog.Format(_T("[LOAD]ModelListLoad_NGBlob_EXT %d - %s "), rstExcLoad, CString(pCtrlExc.sFilePath));
				ext::Log::add(sLog);
			}
		}

	//}
	return TRUE;
}
void InspManager::SetExtMachinePatternParam(InspAlgo sInspAlgo)
{
	AlgoPattern* m_pAlgo = (AlgoPattern*)sInspAlgo.m_ptrInspAlgoParam;
	m_pAlgo->m_factor_red = ext::irc::get()->_CtrlServer.First()->machineAlgo.stExtPattern.dFactor_R;
	m_pAlgo->m_factor_green = ext::irc::get()->_CtrlServer.First()->machineAlgo.stExtPattern.dFactor_G;
	m_pAlgo->m_factor_blue = ext::irc::get()->_CtrlServer.First()->machineAlgo.stExtPattern.dFactor_B;
}
void InspManager::SetExtPOCRFontInfo()
{
	CPInsp_POCR *pPOCR = /*(CPInsp_POCR *)g_pInspMng->*/GetPocrTeach();
	pPOCR->SetExtPOCRFontInfo();
}
void InspManager::SetExtAngleColorParam()
{
	//if (m_pColorTeach)
	//	m_pColorTeach->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	//if (m_pColorInsp)
	//	m_pColorInsp->InitDevice(m_milApp, m_milSys, m_fovWidth, m_fovLength, m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
	//lightTest
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	CPInsp_Color *pColorInsp = g_pInspMng->GetColorInsp();

	if (pColorTeach != NULL)
	{
		pColorTeach->m_fFatorR = m_ExtProdInfo->fFactor_TR;
		pColorTeach->m_fFatorG = m_ExtProdInfo->fFactor_TG;
		pColorTeach->m_fFatorB = m_ExtProdInfo->fFactor_TB;
		pColorTeach->m_fFatorACR = m_ExtProdInfo->fFactor_ACR;
		pColorTeach->m_fFatorACG = m_ExtProdInfo->fFactor_ACG;
		pColorTeach->m_fFatorACB = m_ExtProdInfo->fFactor_ACB;
		pColorTeach->m_fFatorBR = m_ExtProdInfo->fFactor_BR;
		pColorTeach->m_fFatorBB = m_ExtProdInfo->fFactor_BB;
		pColorTeach->m_fFatorMGR = m_ExtProdInfo->fFactor_MGR;
		pColorTeach->m_fFatorMGB = m_ExtProdInfo->fFactor_MGB;
		pColorTeach->m_fFatorBGR = m_ExtProdInfo->fFactor_BGR;
		pColorTeach->m_fFatorBGB = m_ExtProdInfo->fFactor_BGB;
		
		lightData extlight[3];
		extlight[0] = ext::irc::get()->_CtrlServer.First()->sAngleColorData[0];
		extlight[1] = ext::irc::get()->_CtrlServer.First()->sAngleColorData[1];
		extlight[2] = ext::irc::get()->_CtrlServer.First()->sAngleColorData[2];

		for (int n = 0; n < 3; n++)
		{
			pColorTeach->m_sLightData[n].m_byLightCnt = ext::irc::get()->_CtrlServer.First()->sAngleColorData[n].m_byLightCnt;
			m_sLightData[n].m_byLightCnt = ext::irc::get()->_CtrlServer.First()->sAngleColorData[n].m_byLightCnt;
			for (int a = 0; a < LIGHT_DATA_CNT; a++)
			{
				for (int b = 0; b < LIGHT_CNT; b++)
				{
					byte byData = ext::irc::get()->_CtrlServer.First()->sAngleColorData[n].m_byArrLightData[a][b];
					pColorTeach->m_sLightData[n].m_byArrLightData[a][b] = byData;
					m_sLightData[n].m_byArrLightData[a][b] = byData;
				}
			}
		}
		if (g_pInspMng->m_bNewInspection)
		{
			BOOL bRet = FALSE;
			for (int n = 0; n < 3; n++)
			{
				PIAL::PInspAlgo_Lib::m_sLightData[n].m_byLightCnt = pColorTeach->m_sLightData[n].m_byLightCnt;
				for (int a = 0; a < LIGHT_DATA_CNT; a++)
				{
					for (int b = 0; b < LIGHT_CNT; b++)
					{
						byte byData = pColorTeach->m_sLightData[n].m_byArrLightData[a][b];
						PIAL::PInspAlgo_Lib::m_sLightData[n].m_byArrLightData[a][b] = byData;
					}
				}
			}
			bRet = TRUE;
		}

		for (int i = 0; i < eDefaultAC_Total; i++)
			pColorTeach->m_byDefaultAC[i] = ext::irc::get()->_CtrlServer.First()->byDefaultAC[i];
	}
	if (pColorInsp != NULL)
	{
		pColorInsp->m_fFatorMGR = m_ExtProdInfo->fFactor_MGR;
		pColorInsp->m_fFatorMGB = m_ExtProdInfo->fFactor_MGB;
		pColorInsp->m_fFatorBGR = m_ExtProdInfo->fFactor_BGR;
		pColorInsp->m_fFatorBGB = m_ExtProdInfo->fFactor_BGB;
	}

}
void InspManager::CreateDir(CString Path)
{
	CString strPath = Path;
	CString strTemp[20];
	CString strFile;
	int nLength = strPath.GetLength();
	int j = 0;

	for (int i = 0; i < 20; i++)
		strTemp[i].Empty();


	for (int i = 0; i < nLength; i++)
	{
		if (strPath.Mid(i, 1) == '\\')
		{
			j++;
		}
		strTemp[j] = strTemp[j] + strPath.Mid(i, 1);

	}

	strFile = strTemp[0];
	int i = 1;

	while (strTemp[i] != "")
	{
		strFile = strFile + strTemp[i];
		CreateDirectory(strFile, NULL);
		i++;
	}
}

CRect InspManager::GetRectRoi()
{
	CRect ret;
	double rotAngle = m_pInspBoardInfo->angle;
	BOOL isAnyAngle = GetPtrInspAlgo()->IsAnyAngle(rotAngle);
	if (isAnyAngle)
		return GetRectBoundaryRoi();

	double TempWidh = 0;
	double TempHeight = 0;
	double tempCx = 0;
	double tempCy = 0;

	int index = 0;
	BOOL flag = FALSE;//IsInMount(&index);

	if (flag)
	{
		TempWidh = m_pParamArray[index].bdrWidth;
		TempHeight = m_pParamArray[index].bdrLength;
		tempCx = m_pParamArray[index].bdrCx;
		tempCy = m_pParamArray[index].bdrCy;
	}
	else
	{
		TempWidh = m_pInspBoardInfo->partWidth;
		TempHeight = m_pInspBoardInfo->partHeight;
		tempCx = m_pInspBoardInfo->partCx;
		tempCy = m_pInspBoardInfo->partCy;
	}


	int width = (int)(float)(TempWidh / m_resolX + 0.00001);		// 임시 (좌표 round로 통일 시키기 전에..)
	int height = (int)(float)(TempHeight / m_resolY + 0.00001);
	POINT ct = CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, tempCx, tempCy, tempCx, tempCy, m_pInspBoardInfo);

	ret.left = (LONG)(ct.x - (width / 2));
	ret.top = (LONG)(ct.y - (height / 2));
	ret.right = (LONG)(ret.left + width);
	ret.bottom = (LONG)(ret.top + height);

	return ret;
}
CRect InspManager::GetRectBoundaryRoi()
{
	CRect ret;
	double TempWidh = 0;
	double TempHeight = 0;
	double tempCx = 0;
	double tempCy = 0;

	int index = 0;
	for (int i = 0; i < m_ParamArraySize; i++)
	{
		if (m_pParamArray[i].inspType == eINSP_LEADSOLDER)
		{
			index = i;
			break;
		}
	}

	TempWidh = m_pInspBoardInfo->anyAngleWidth;
	TempHeight = m_pInspBoardInfo->anyAngleLength;
	tempCx = m_pInspBoardInfo->anyAngleCx;
	tempCy = m_pInspBoardInfo->anyAngleCy;

	int width = (int)((float)TempWidh / m_resolX);
	int height = (int)((float)TempHeight / m_resolY);
	POINT ct = CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, tempCx, tempCy, tempCx, tempCy, m_pInspBoardInfo);

	ret.left = (LONG)(ct.x - (width / 2));
	ret.right = (LONG)(ct.x + (width / 2));
	ret.top = (LONG)(ct.y - (height / 2));
	ret.bottom = (LONG)(ct.y + (height / 2));

	return ret;
}
int InspManager::InspWindowAlgo3(int wndType, int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, PIAL::PInspDataSet* InspDataSet, InspAlgoTempResult * ptrInspAlgoTempResult, bool bLeadInsp, vector<InspRstPolyAlgo>* vpInspRstPoly)
{
	BOOL bAutoInsp = TRUE;
	int ret = e_OK;

	int inspType = 0;
	int nAlgoType = 0, nAlgoID = 0;

	int nLine = __LINE__;

	PIAL::Insp_Image* pImg_buf = nullptr;
	PIAL::PInspData* InspData = nullptr;		// 삭제하지 말 것..(소멸자에서 삭제함)

	try
	{
		// 		std::shared_ptr<ImgProcessing_POCR> imgProc_Pocr_POCR	 = std::make_shared<ImgProcessing_POCR>();
		// 		std::shared_ptr<ImgProcessing_POCR> imgProc_Pocr_POCR_ro = std::make_shared<ImgProcessing_POCR>();
		// 		std::shared_ptr<ImgProcessing> imgProc_Pocr_pat = std::make_shared<ImgProcessing>();

		inspType = m_pParamArray[nWndIndex].inspType;
		if (m_bNewInspection)
		{
			InspData = new PIAL::PInspData; // PemNew하지 말것.. PInspDataSet만 PemNew함, Dll내부에서 PInspData 삭제 처리 함.
			//g_pMManager->pem_new_check(InspData, (PCHAR)__FUNCTION__, __LINE__);
			InspData->WindowID = m_pParamArray[nWndIndex].wndIndex;
			InspDataSet->SetInspData(InspData);
		}

		nLine = __LINE__;

		BOOL bAlignResOK = FALSE;
		int nAlignCnt = 0;
		AlignResult * ptrAlignRes = NULL;
		try
		{
			if ((m_pParamArray[nWndIndex].nAlignWndID != 0) && arrAlignRes != NULL && nAlignWndIdx > 0)
			{
				int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
				//ptrAlignRes = new AlignResult[nSize];
				ptrAlignRes = g_pMManager->pem_new<AlignResult>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
				for (int a = 0; a < nSize; a++)
				{
					ptrAlignRes[a].nWindowID = -1;
				}

				bool bFind = FindAlignResultData(nWndIndex, arrAlignRes, ptrAlignRes, &nAlignCnt);
				if (bFind == false)
				{
					Delete_1DArray(&ptrAlignRes);
				}
			}
		}
		catch (...)
		{
			Delete_1DArray(&ptrAlignRes);
		}

		std::vector<PIAL::_AlignResult> vecAlignResult;
		if (m_bNewInspection)
			m_PInspWrapper->ConvertAlignResult(ptrAlignRes, nAlignCnt, vecAlignResult);

		BOOL bTabAutoSearchROI = FALSE;
		CAtlArray<CRect> rcArrTab;
		CAtlArray<CRect> rcArrTabBridge;
		try
		{
			nLine = __LINE__;

			if (inspType == eINSP_TAB)
			{
				for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
				{
					if (bTabAutoSearchROI)
						break;
					if (!m_pParamArray[nWndIndex].vArrAlgoParam || !WndResult->m_vArrRstInspAlgo)
						continue;

					InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
					nAlgoID = nAlgo;
					nAlgoType = (int)sInspAlgo.m_eAlgoType;

					if (sInspAlgo.m_eAlgoType == eAlgoTab)
					{
						AlgoTab* pAlgoTab = (AlgoTab *)sInspAlgo.m_ptrInspAlgoParam;
						if (pAlgoTab && pAlgoTab->m_bAutoSearchROI)
						{
							bTabAutoSearchROI = TRUE;
							break;
						}
					}
					else if (sInspAlgo.m_eAlgoType == eAlgoBridge)
					{
						AlgoBridge* pAlgoBridge = (AlgoBridge *)sInspAlgo.m_ptrInspAlgoParam;
						if (pAlgoBridge && pAlgoBridge->m_bAutoSearchROI)
						{
							bTabAutoSearchROI = TRUE;
							break;
						}
					}
				}
			}
		}
		catch (CMemoryException* e)
		{

		}
		catch (CFileException* e)
		{
		}
		catch (CException* e)
		{
		}
		nLine = __LINE__;

		AlgoCoordinate coordinateAlgo;
		WndInfo sWndInfo;
		WndAlgoImg wndImg;
		int wnd_w = 0;
		int wnd_h = 0;
		double dX = 0.0;
		double dY = 0.0;
		int nOffX_pix = 0;
		int nOffY_pix = 0;
		int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
		int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;

		CString dOCRPath = _T("");
		CString dOCRCopyPath = _T("");

		int nLeadTipPos = -1;
		BOOL bUseLeadTipSideOption = FALSE;
		double dLeadTipShiftX = -1;
		double dLeadTipShiftY = -1;
		double dLeadTipSize = -1;
		int nLeadTipDir = -1;
		double OCRNGScore = 0.0;
		double OCROKScore = 0.0;
		BOOL USENGOCR = FALSE;
		int nWndDir = e_LEFT;
		BOOL bUSeLeadAlign = FALSE;
		int *ptrnArrLeadTipRst = NULL;
		int nLeadTipTotalCnt = 0;
		int nInspLeadTipCnt = 0;
		POINTF poBodyCenter;
		poBodyCenter.x = -1;
		poBodyCenter.y = -1;
		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
		{
			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
			if (sInspAlgo.m_eAlgoType == eAlgoLead_Tip)
				nLeadTipTotalCnt++;
		}
		if (nLeadTipTotalCnt > 0)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrnArrLeadTipRst, nLeadTipTotalCnt);
			memset(ptrnArrLeadTipRst, 0, sizeof(int) * nLeadTipTotalCnt);
		}
		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
		{
			DWORD start = GetTickCount();
			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
			InspAlgoType eInspAlgoType = sInspAlgo.m_eAlgoType;
			if (WndResult->m_nAlgorithmCnt <= nAlgo)
				break;

			nAlgoID = nAlgo;
			nAlgoType = (int)sInspAlgo.m_eAlgoType;

			if (inspType == eINSP_LEADSOLDER && ptrInspAlgoTempResult == NULL)
			{
				if (sInspAlgo.m_eAlgoType != eAlgoAlign && sInspAlgo.m_eAlgoType != eAlgoAlignEdge)
					continue;
				else
					bUSeLeadAlign = TRUE;
			}
			if (eInspAlgoType == eAlgoDistance)
			{
				//Distance 알고리즘 툴에서 지원하지않음
	/*			nLine = __LINE__;

				AlignResult * pTmpAlignRes = NULL;

				if (ptrAlignRes)
				{
					int nSize = _msize(ptrAlignRes) / sizeof(AlignResult);
					pTmpAlignRes = g_pMManager->pem_new<AlignResult>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
					memcpy(pTmpAlignRes, ptrAlignRes, sizeof(AlignResult) * nSize);
				}

				WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = eAlgoDistance;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = TRUE;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;
				AddDistanceAlgoList(nWndIndex, nAlgo, WndResult, nAlignCnt, pTmpAlignRes);

				if (pImg_buf)
				{
					g_pMManager->pem_delete(pImg_buf, false);
					pImg_buf = nullptr;
				}*/

			}

			nLine = __LINE__;

			if (bUSeLeadAlign == FALSE && (!m_pParamArray[nWndIndex].vArrAlgoParam || !WndResult->m_vArrRstInspAlgo))
				continue;
			if (sInspAlgo.m_eAlgoType == eAlgoOCR)
				continue;

			nLine = __LINE__;

			if (bUSeLeadAlign == FALSE)
			{
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType = sInspAlgo.m_eAlgoType;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo = NULL;
			}
			// Essential check되어있고 NG일때에도 이후 알고리즘 검사하게 해달라는 요청으로 주석처리함
			// 		if(ret == e_NG)
			// 		{
			// 			WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = FALSE;
			// 			WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk	= FALSE;
			// 			continue;
			// 		}

			nLine = __LINE__;

			wndImg.Destroy();
			/*if(IsSeqForceStop())		// Auto검사 중에 멈출때 계산부분도 막으려 했으나 하면 안됨.
			return e_SKIP;*/

			nLine = __LINE__;
			//CQTimer qtm; 
			//qtm.StartTick();
			bool bNewInspAlgo = false;
			if ((eInspAlgoType == eAlgoBody_Blob || eInspAlgoType == eAlgoAlign || eInspAlgoType == eAlgoNGBlob ||
				eInspAlgoType == eAlgoPackageThickness || eInspAlgoType == eAlgoBodyEdge || eInspAlgoType == eAlgoPatternDiff ||
				eInspAlgoType == eAlgoPadBW || eInspAlgoType == eAlgoPOCR || eInspAlgoType == eAlgoBarcode || eInspAlgoType == eAlgoShapeX || eInspAlgoType == eAlgoPadArray)
				&& m_bNewInspection)
			{
				bNewInspAlgo = true;

				if (eInspAlgoType == eAlgoBody_Blob)
				{
					AlgoBodyBlob* pAlgobodyblob = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
					if (pAlgobodyblob->m_bOnlyBodyTip)
						bNewInspAlgo = false;
				}
				if (eInspAlgoType == eAlgoPOCR)
				{
					AlgoPOCR* pAlgoPOCR = (AlgoPOCR *)sInspAlgo.m_ptrInspAlgoParam;
					if (pAlgoPOCR->m_bUseLib == FALSE)
						bNewInspAlgo = false;
				}
				if (eInspAlgoType == eAlgoBarcode)
				{	//Halcon 미사용 시(insptype3이 아닐 시) bNewInspAlgo = false;
					AlgoBarcode* pAlgoBarcode = (AlgoBarcode *)sInspAlgo.m_ptrInspAlgoParam;
					bool bHalconInsp = ((pAlgoBarcode->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_HALCON_Barcode) == (int)m_eAlgoBar_Data2_HALCON_Barcode);
					if (!bHalconInsp) {
						bNewInspAlgo = false;
					}
				}
			}

			if (bNewInspAlgo == true)
			{
				pImg_buf = g_pMManager->pem_new<PIAL::Insp_Image>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

				int nSideCameraIndex = m_pParamArray[nWndIndex].nInspCameraType;

				// 추후통합 예정.
				if (nAlignCnt < 2)
					m_PInspWrapper->WndSizeChange(pImg_buf, sInspAlgo, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY, ptrAlignRes, nOffX_pix, nOffY_pix);
				else
					m_PInspWrapper->WndSizeChange_ArrAlign(pImg_buf, eInspAlgoType, coordinateAlgo, m_pInspBoardInfo, m_pParamArray, nWndIndex, wnd_w, wnd_h, dX, dY, ptrAlignRes, inspType, sInspAlgo, nAlignCnt);
				RECT rtWnd;
				rtWnd.left = dX;				rtWnd.top = dY;
				rtWnd.right = dX + wnd_w - 1;	rtWnd.bottom = dY + wnd_h - 1;
				pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeX = wnd_w;
				pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeY = wnd_h;

				//AlgoNGBlob* pAlgoNGBlob = (AlgoNGBlob*)sInspAlgo.m_ptrInspAlgoParam;
				bool bUseAI = false;
				int nPartID = m_pInspBoardInfo->nPartIDOrg;

				if (eInspAlgoType == eAlgoNGBlob)
				{
					if ((((AlgoNGBlob*)sInspAlgo.m_ptrInspAlgoParam)->idata2 & (int)_NGBlob_UseAI) == (int)_NGBlob_UseAI)
					{
						bUseAI = true;

						CString sLog = _T("");
						sLog.Format(_T("[AISeg][09][NGBlob]_Part[%d] InspWindowAlgo() "), nPartID);
						g_pMPTI->AddLog_Dev(sLog);
					}
				}
				else if (eInspAlgoType == eAlgoPadBW)
				{
					if (((AlgoPadBW*)sInspAlgo.m_ptrInspAlgoParam)->bUseAI_Segmentation)
					{
						bUseAI = true;

						CString sLog = _T("");
						sLog.Format(_T("[AISeg][09][PadBW]_Part[%d] InspWindowAlgo() "), nPartID);
						g_pMPTI->AddLog_Dev(sLog);
					}
				}

				// AI 사용 시 조명합성이미지 대신 AI 이미지 사용 (25.11.27 nylee)
				//if (sInspAlgo.m_bUseAI && m_pInspBoardInfo->partImgBuf.imgAI && pImg_buf && pImg_buf->m_p2D)
				if (bUseAI && ext::irc::get()->_CtrlServer.First()->prod.bUseAI)
				{
					if (m_pInspBoardInfo->partImgBuf.imgAI && pImg_buf)
					{
						bool bSuccessCropAI​img = CopyAICroppedToWnd2D(pImg_buf, wnd_w, wnd_h, m_pInspBoardInfo->partImgBuf.imgAI, m_pInspBoardInfo->partImgBuf.nImageSizeX, m_pInspBoardInfo->partImgBuf.nImageSizeY);
						if (!bSuccessCropAI​img)
						{
							CString sLog = _T("");
							sLog.Format(_T("[AISeg][09]_Part[%d] InspWindowAlgo(), Failed to crop AIimg to wnd size. "), nPartID);
							g_pMPTI->AddLog_Dev(sLog);
						}
						else
						{
							CString sLog = _T("");
							sLog.Format(_T("[AISeg][09]_Part[%d] InspWindowAlgo(), Successfully cropped AIimg to wnd size. "), nPartID);
							g_pMPTI->AddLog_Dev(sLog);
						}
					}
					else
					{
						CString sLog = _T("");
						sLog.Format(_T("[AISeg][09]_Part[%d] InspWindowAlgo(), partImgBuf.imgAI  is  NULL"), nPartID);
						g_pMPTI->AddLog_Dev(sLog);
					}
				}

				m_PInspWrapper->NormalImageCompose(m_pInspBoardInfo, eInspAlgoType, sInspAlgo, pImg_buf, pImg_buf->m_p2D, width, height, wnd_w, wnd_h, dX, dY, nSideCameraIndex, ptrAlignRes, nAlignCnt);
				m_PInspWrapper->SetWindowImage(m_pInspBoardInfo, pImg_buf, rtWnd);
				m_PInspWrapper->ColorDataInput(sInspAlgo, pImg_buf, width, height, rtWnd, 0, nSideCameraIndex);
				m_PInspWrapper->GetAlgoImage(&sInspAlgo, m_pInspBoardInfo, pImg_buf, rtWnd, nSideCameraIndex);

				if (sInspAlgo.m_nMixCount > 0)
				{
					pImg_buf->m_p2D_Mix[0] = new PIAL::PI_Buff(wnd_w, wnd_h);
					// 해당구문은 pImg_buf 소멸자(PInsp_Algo.dll)에서 해제하기 때문에 pem_new, pem_new_check를 사용하면 안됌.
					// g_pMManager->pem_new_check(pWnd_buf->m_P2D_Mix[0], (PCHAR)__FUNCTION__, __LINE__);						
					m_PInspWrapper->AlgoImageMixCompose(m_pInspBoardInfo, sInspAlgo.InspAlgoLightsMix[0], pImg_buf->m_p2D_Mix[0]->m_pData, width, height, wnd_w, wnd_h, dX, dY);
				}
				nLine = __LINE__;
				if (sInspAlgo.m_nMixCount > 1)
				{
					pImg_buf->m_p2D_Mix[1] = new PIAL::PI_Buff(wnd_w, wnd_h);
					// 해당구문은 pImg_buf 소멸자(PInsp_Algo.dll)에서 해제하기 때문에 pem_new, pem_new_check를 사용하면 안됌.
					// g_pMManager->pem_new_check(pWnd_buf->m_P2D_Mix[1], (PCHAR)__FUNCTION__, __LINE__);
					m_PInspWrapper->AlgoImageMixCompose(m_pInspBoardInfo, sInspAlgo.InspAlgoLightsMix[1], pImg_buf->m_p2D_Mix[1]->m_pData, width, height, wnd_w, wnd_h, dX, dY);
				}
			}
			else
			{
				WndSizeChange(eInspAlgoType, coordinateAlgo, wndImg,
					nWndIndex, wnd_w, wnd_h, dX, dY, ptrAlignRes, inspType, sInspAlgo,
					bUseLeadTipSideOption, dLeadTipShiftX, dLeadTipShiftY, dLeadTipSize, nLeadTipDir, nAlignCnt, nOffX_pix, nOffY_pix);
			}

			if (wnd_w <= 0 || wnd_h <= 0)
				return e_NG;

			bool bBreakCurAlignID = false;
			bool bBreakBody = false;
			int nCurAlignID = 0;
			for (int n = 0; n < nAlignCnt; n++)
			{
				nLine = __LINE__;
				if (bBreakCurAlignID == false && (ptrAlignRes[n].nWindowID == m_pParamArray[nWndIndex].nAlignWndID ||
					ptrAlignRes[n].nWindowID == m_pParamArray[nWndIndex].wndIndex))
				{
					nCurAlignID = n;
					bBreakCurAlignID = true;
				}
				if (bBreakBody == false && ptrAlignRes[n].rcBodyRect.Width() > 0 && ptrAlignRes[n].rcBodyRect.Height() > 0)
				{
					nLine = __LINE__;
					int nBodyCenterX = (ptrAlignRes[n].rcBodyRect.left + ptrAlignRes[n].rcBodyRect.right) / 2;
					int nBodyCenterY = (ptrAlignRes[n].rcBodyRect.top + ptrAlignRes[n].rcBodyRect.bottom) / 2;
					if (dX <= nBodyCenterX && nBodyCenterX <= dX + wnd_w &&
						dY <= nBodyCenterY && nBodyCenterY <= dY + wnd_h)
					{
						poBodyCenter.x = nBodyCenterX - dX;
						poBodyCenter.y = nBodyCenterY - dY;
						bBreakBody = true;
					}
					else if (dX <= nBodyCenterX && nBodyCenterX <= dX + wnd_w)
					{
						bBreakBody = true;
						poBodyCenter.x = nBodyCenterX - dX;
						if (dY <= ptrAlignRes[n].rcBodyRect.top && ptrAlignRes[n].rcBodyRect.top <= dY + wnd_h)
							poBodyCenter.y = ptrAlignRes[n].rcBodyRect.top - dY;
						else if (dY <= ptrAlignRes[n].rcBodyRect.bottom && ptrAlignRes[n].rcBodyRect.bottom <= dY + wnd_h)
							poBodyCenter.y = ptrAlignRes[n].rcBodyRect.bottom - dY;
						else
							bBreakBody = false;
					}
					else if (dY <= nBodyCenterY && nBodyCenterY <= dY + wnd_h)
					{
						bBreakBody = true;
						poBodyCenter.y = nBodyCenterY - dY;
						if (dX <= ptrAlignRes[n].rcBodyRect.left && ptrAlignRes[n].rcBodyRect.left <= dX + wnd_w)
							poBodyCenter.x = ptrAlignRes[n].rcBodyRect.left - dX;
						else if (dX <= ptrAlignRes[n].rcBodyRect.right && ptrAlignRes[n].rcBodyRect.right <= dX + wnd_w)
							poBodyCenter.x = ptrAlignRes[n].rcBodyRect.right - dX;
						else
							bBreakBody = false;
					}
				}
			}
			if (bBreakBody == false)
			{
				poBodyCenter.x = -1;
				poBodyCenter.y = -1;
			}
			if (nAlignCnt < 1)
			{
				nAlignCnt = 1;
				if (ptrAlignRes == NULL)
					//ptrAlignRes = new AlignResult[nAlignCnt];
					ptrAlignRes = g_pMManager->pem_new<AlignResult>(true, nAlignCnt, (PCHAR)__FUNCTION__, __LINE__);
				// 				if (bLeadInsp) nAlignCnt = 0;	// LMJ 2021/07/11 : Bridge 알고리즘에서 Lead - Align 알고리즘 으로 Offset 보정 안되는 문제 때문에 주석 처리함
			}
			//m_AlgoDt[_T("WndSizeChange")] = (qtm.EndTick() * 1000.0) + m_AlgoDt[_T("WndSizeChange")];
			sWndInfo.dCenterX = coordinateAlgo.dROICenterX;
			sWndInfo.dCenterY = coordinateAlgo.dROICenterY;
			sWndInfo.dWidth = RounD(coordinateAlgo.dROIWidth);
			sWndInfo.dLength = RounD(coordinateAlgo.dROILength);

			nLine = __LINE__;

			// Input 2D Image 합성
			int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
			int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;
			if (width <= 0 || height <= 0)
				return e_NG;

			UCHAR * ImgR = NULL, *ImgG = NULL, *ImgB = NULL, *ImgW = NULL;
			LightTypeBuf sLightBuf;
			ImgW = m_pInspBoardInfo->partImgBuf.imgTop_W;
			int nInspCameraType = m_pParamArray[nWndIndex].nInspCameraType;
			wndImg.m_nLight_index = nInspCameraType;

			// 통합 검사/제외 영역 생성
			TotalInspExceptArea stTieArea;
			PIAL::_TotalInspExceptArea PIALTieArea;
			if (m_bNewInspection)
				m_PInspWrapper->ConvertExceptROI(&sInspAlgo, &PIALTieArea, 0);
			nLine = __LINE__;
			stTieArea.m_nUsedWndPolygon = m_pParamArray[nWndIndex].m_nUsedWndPolygon;
			for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
				stTieArea.m_ptArrWndPolygon[i] = m_pParamArray[nWndIndex].m_ptArrWndPolygon[i];

			if (width > height)
			{
				if (width / 2 > dX + (wnd_w / 2))
					nWndDir = e_LEFT;
				else
					nWndDir = e_RIGHT;
			}
			else
			{
				if (height / 2 > dY + (wnd_h / 2))
					nWndDir = e_TOP;
				else
					nWndDir = e_BOTTOM;
			}

			if (bNewInspAlgo == false)
			{
				switch (sInspAlgo.m_eLightType)
				{
				case Top_Light:
					ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;
					ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;
					ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;
					break;
				case Middle_Light:
					ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
					ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
					break;
				case Bottom_Light:
					if (FALSE)//g_pMPTI->m_nCameraType == Basler_Color)
					{
						ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
						ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
					}
					else
					{
						ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
						ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
					}
					break;
				case Side1_Light:
				case Side2_Light:
				case Side3_Light:
				case Side4_Light:
					if (nInspCameraType == eMSCN_SIDECAM1)
					{
						ImgR = m_pInspBoardInfo->partImgBuf.imgSide1_R;
						ImgG = m_pInspBoardInfo->partImgBuf.imgSide1_G;
						ImgB = m_pInspBoardInfo->partImgBuf.imgSide1_B;
					}
					else if (nInspCameraType == eMSCN_SIDECAM2)
					{
						ImgR = m_pInspBoardInfo->partImgBuf.imgSide2_R;
						ImgG = m_pInspBoardInfo->partImgBuf.imgSide2_G;
						ImgB = m_pInspBoardInfo->partImgBuf.imgSide2_B;
					}
					else if (nInspCameraType == eMSCN_SIDECAM3)
					{
						ImgR = m_pInspBoardInfo->partImgBuf.imgSide3_R;
						ImgG = m_pInspBoardInfo->partImgBuf.imgSide3_G;
						ImgB = m_pInspBoardInfo->partImgBuf.imgSide3_B;
					}
					else if (nInspCameraType == eMSCN_SIDECAM4)
					{
						ImgR = m_pInspBoardInfo->partImgBuf.imgSide4_R;
						ImgG = m_pInspBoardInfo->partImgBuf.imgSide4_G;
						ImgB = m_pInspBoardInfo->partImgBuf.imgSide4_B;
					}
					else
					{
						ImgR = NULL;
						ImgG = NULL;
						ImgB = NULL;
					}
					break;
				case  User_Light:
					if (FALSE)//g_pMPTI->m_nCameraType == Basler_Color)
					{
						sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_W;
						sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
						sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
						sLightBuf.m_pucTWhite = NULL;
						sLightBuf.m_pucMRed = NULL;
						sLightBuf.m_pucMGreen = NULL;
						sLightBuf.m_pucMBlue = NULL;
						sLightBuf.m_pucMWhite = NULL;
						sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
						sLightBuf.m_pucBGreen = NULL;
						sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
						sLightBuf.m_pucBWhite = NULL;
					}
					else
					{
						sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
						sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
						sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
						sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;

						sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
						sLightBuf.m_pucMGreen = NULL;
						sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
						sLightBuf.m_pucMWhite = NULL;

						sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
						sLightBuf.m_pucBGreen = NULL;
						sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
						sLightBuf.m_pucBWhite = NULL;
					}

					sLightBuf.m_nImgWidth = width;
					sLightBuf.m_nImgHeight = height;
					sLightBuf.m_nROIImgWidth = wnd_w;
					sLightBuf.m_nROIImgHeight = wnd_h;
					sLightBuf.m_dROIX = dX;
					sLightBuf.m_dROIY = dY;
					sLightBuf.m_nImgCnt = sInspAlgo.m_nLightCnt;
					sLightBuf.m_pnRedValue = sInspAlgo.m_nArrRedValue;
					sLightBuf.m_pnGreenValue = sInspAlgo.m_nArrGreenValue;
					sLightBuf.m_pnBlueValue = sInspAlgo.m_nArrBlueValue;
					sLightBuf.m_pnWhiteValue = sInspAlgo.m_nArrWhiteValue;
					sLightBuf.m_pnPosition = sInspAlgo.m_nArrLightPosition;
					sLightBuf.m_pnCalculation = sInspAlgo.m_nArrCalculation;
				}
			}

			if (m_pInspBoardInfo->byPCBPosInspLight == 1)	// 끝단찾기 격자 합성 조명은 Top Red 에 있고 해당 이미지만 100%로 윈도우 검사 이미지 wndImg.m_ucArr2D 를 사용하기 위해서
			{
				sInspAlgo.m_nRedValue = 100;
				sInspAlgo.m_nGreenValue = 0;
				sInspAlgo.m_nBlueValue = 0;
				sInspAlgo.m_nWhiteValue = 0;
			}

			bool bUserLight = false;
			double dAlignAngle = 0;
			if (nAlignCnt > 0 && ptrAlignRes && CheckWndShift(eInspAlgoType, true) && WindowRotate())
			{
				int nAlignIDX = nAlignCnt - 1;
				dAlignAngle = ptrAlignRes[nAlignIDX].theta;
			}

			if (bNewInspAlgo == false)
			{
				if ((sInspAlgo.m_eLightType == Top_Light) || (sInspAlgo.m_eLightType == Middle_Light) || (sInspAlgo.m_eLightType == Bottom_Light))
				{
					//m_InspAlgo.RoiImageCompose(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, cx, cy, wnd_w, wnd_h, sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, sInspAlgo.m_nWhiteValue, wndImg.m_ucArr2D);
					m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h,
						sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, sInspAlgo.m_nWhiteValue,
						wndImg.m_ucArr2D, dAlignAngle);

					m_procMil->SaveWorkImg(ImgR, width, height, _T("Single2D Red.bmp"));
					m_procMil->SaveWorkImg(ImgW, width, height, _T("Single2D White.bmp"));
					m_procMil->SaveWorkImg(wndImg.m_ucArr2D, wnd_w, wnd_h, _T("Compose.bmp"));
				}
				else if ((sInspAlgo.m_eLightType == Side1_Light) || (sInspAlgo.m_eLightType == Side2_Light) || (sInspAlgo.m_eLightType == Side3_Light) || (sInspAlgo.m_eLightType == Side4_Light))
				{
					m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h,
						sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, 0,
						wndImg.m_ucArr2D, dAlignAngle);
					m_procMil->SaveWorkImg(ImgR, width, height, _T("Side Red.bmp"));
					m_procMil->SaveWorkImg(ImgG, width, height, _T("Side Green.bmp"));
					m_procMil->SaveWorkImg(ImgB, width, height, _T("Side Blue.bmp"));
					// 				m_procMil->SaveWorkImg(wndImg.m_ucArr2D, wnd_w, wnd_h, _T("Side Compose.bmp"));
					//				wndImg.m_ucArr2D = ImgR;
					m_procMil->SaveWorkImg(wndImg.m_ucArr2D, wnd_w, wnd_h, _T("Side_Compose.bmp"));
				}
				else
				{
					bUserLight = true;
					m_procMil->SaveWorkImg(sLightBuf.m_pucTRed, width, height, _T("Top Red.bmp"));
					m_procMil->SaveWorkImg(sLightBuf.m_pucMRed, width, height, _T("Mid Red.bmp"));
					m_procMil->SaveWorkImg(sLightBuf.m_pucBRed, width, height, _T("Bot Red.bmp"));
					m_InspAlgo.ROIImageClaculCompose(sLightBuf, wndImg.m_ucArr2D, dAlignAngle);
					m_procMil->SaveWorkImg(wndImg.m_ucArr2D, wnd_w, wnd_h, _T("Compose_User.bmp"));
				}
			}
			//InspRoiImgBuf *sInspImgBuf = new InspRoiImgBuf;
			InspRoiImgBuf *sInspImgBuf = g_pMManager->pem_new<InspRoiImgBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
			InspRoiImgBuf* sArrInspImgBuf = NULL;
			sInspImgBuf->imgTop_R = NULL;
			sInspImgBuf->imgTop_G = NULL;
			sInspImgBuf->imgTop_B = NULL;
			sInspImgBuf->imgTop_W = NULL;
			sInspImgBuf->imgMiddle_R = NULL;
			sInspImgBuf->imgMiddle_B = NULL;
			sInspImgBuf->imgBottom_R = NULL;
			sInspImgBuf->imgBottom_B = NULL;

			//if (bNewInspAlgo == false)
				ColorDataInput(sInspAlgo, width, height, dX, dY, wnd_w, wnd_h, sInspImgBuf, 0, nInspCameraType);
			if (eInspAlgoType == eAlgoAlignEdge)
			{
				//sArrInspImgBuf = new InspRoiImgBuf[ALIGNEDGE_AREA_CNTS];
				sArrInspImgBuf = g_pMManager->pem_new<InspRoiImgBuf>(true, ALIGNEDGE_AREA_CNTS, (PCHAR)__FUNCTION__, __LINE__);
				for (int a = 0; a < ALIGNEDGE_AREA_CNTS; a++)
				{
					sArrInspImgBuf[a].imgTop_R = NULL;
					sArrInspImgBuf[a].imgTop_G = NULL;
					sArrInspImgBuf[a].imgTop_B = NULL;
					sArrInspImgBuf[a].imgTop_W = NULL;
					sArrInspImgBuf[a].imgMiddle_R = NULL;
					sArrInspImgBuf[a].imgMiddle_B = NULL;
					sArrInspImgBuf[a].imgBottom_R = NULL;
					sArrInspImgBuf[a].imgBottom_B = NULL;
					ColorDataInput(sInspAlgo, width, height, dX, dY, wnd_w, wnd_h, &sArrInspImgBuf[a], a);
				}
			}
			if ((eInspAlgoType == eAlgoPattern))
			{
				UCHAR* ImgR_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_R;
				UCHAR* ImgG_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_G;
				UCHAR* ImgB_Pattern = m_pInspBoardInfo->partImgBuf.imgTop_B;

				((CPInsp_Pattern *)m_pPatternInsp)->SetInspParam(sInspAlgo, wndImg, coordinateAlgo, width, height, stTieArea);
				//cv::Mat src(height,width, CV_MAKETYPE(CV_8U,3));
				if (dX < 0) dX = 0;
				if (dY < 0) dY = 0;

				for (int y = dY; y < dY + wnd_h; y++)
				{
					for (int x = dX; x < dX + wnd_w; x++)
					{
						int sizeY = y - dY;
						int sizeX = x - dX;
						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 0] = ImgB_Pattern[y * width + x] * ((CPInsp_Pattern *)m_pPatternInsp)->m_pAlgoPattern->m_factor_blue < 255 ? ImgB_Pattern[y * width + x] * ((CPInsp_Pattern *)m_pPatternInsp)->m_pAlgoPattern->m_factor_blue : 255;
						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 1] = ImgG_Pattern[y * width + x] * ((CPInsp_Pattern *)m_pPatternInsp)->m_pAlgoPattern->m_factor_green < 255 ? ImgG_Pattern[y * width + x] * ((CPInsp_Pattern *)m_pPatternInsp)->m_pAlgoPattern->m_factor_green : 255;
						wndImg.m_ucArrCV[(sizeY * wnd_w * 3) + (sizeX * 3) + 2] = ImgR_Pattern[y * width + x] * ((CPInsp_Pattern *)m_pPatternInsp)->m_pAlgoPattern->m_factor_red < 255 ? ImgR_Pattern[y * width + x] * ((CPInsp_Pattern *)m_pPatternInsp)->m_pAlgoPattern->m_factor_red : 255;

					}
				}
				//cv::Mat cripsrc;

				//cv::Rect rect(dX, dY, wnd_w, wnd_h);
				/*cripsrc = src(rect);

				for(int y=0; y< wnd_h ; y++)
				{
					memcpy(&wndImg.m_ucArrCV[y*(wnd_w * 3)], &cripsrc.data[y*cripsrc.step], wnd_w * 3);
				}*/
			}

			//LJH 2016.05.26 일단 ocr의 경우 최대 3장 (기존 한장 + 두장)의 이미지를 취합 할 수 있도록 설정
			if (eInspAlgoType == eAlgoOCR || eInspAlgoType == eAlgoPOCR || eInspAlgoType == eAlgoAlignEdge || eInspAlgoType == eAlgoTab || eInspAlgoType == eAlgoHeight_Diff)
			{
				//qtm.StartTick();
				for (int iLoopCount = 0; iLoopCount < sInspAlgo.m_nMixCount; ++iLoopCount)
				{
					switch (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType)
					{
					case Top_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;		ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;		ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;	ImgW = m_pInspBoardInfo->partImgBuf.imgTop_W;	break;	}
					case Middle_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;	break;	}
					case Bottom_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;	break;	}
					case Side1_Light:
					case Side2_Light:
					case Side3_Light:
					case Side4_Light:
						if (nInspCameraType == eMSCN_SIDECAM1)
						{
							ImgR = m_pInspBoardInfo->partImgBuf.imgSide1_R;
							ImgG = m_pInspBoardInfo->partImgBuf.imgSide1_G;
							ImgB = m_pInspBoardInfo->partImgBuf.imgSide1_B;
						}
						else if (nInspCameraType == eMSCN_SIDECAM2)
						{
							ImgR = m_pInspBoardInfo->partImgBuf.imgSide2_R;
							ImgG = m_pInspBoardInfo->partImgBuf.imgSide2_G;
							ImgB = m_pInspBoardInfo->partImgBuf.imgSide2_B;
						}
						else if (nInspCameraType == eMSCN_SIDECAM3)
						{
							ImgR = m_pInspBoardInfo->partImgBuf.imgSide3_R;
							ImgG = m_pInspBoardInfo->partImgBuf.imgSide3_G;
							ImgB = m_pInspBoardInfo->partImgBuf.imgSide3_B;
						}
						else if (nInspCameraType == eMSCN_SIDECAM4)
						{
							ImgR = m_pInspBoardInfo->partImgBuf.imgSide4_R;
							ImgG = m_pInspBoardInfo->partImgBuf.imgSide4_G;
							ImgB = m_pInspBoardInfo->partImgBuf.imgSide4_B;
						}

						break;
					case  User_Light:
						sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
						sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
						sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
						sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;

						sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
						sLightBuf.m_pucMGreen = NULL;
						sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
						sLightBuf.m_pucMWhite = NULL;

						sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
						sLightBuf.m_pucBGreen = NULL;
						sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
						sLightBuf.m_pucBWhite = NULL;

						sLightBuf.m_nImgWidth = width;
						sLightBuf.m_nImgHeight = height;
						sLightBuf.m_nROIImgWidth = wnd_w;
						sLightBuf.m_nROIImgHeight = wnd_h;
						sLightBuf.m_dROIX = dX;
						sLightBuf.m_dROIY = dY;
						sLightBuf.m_nImgCnt = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nLightCnt;
						sLightBuf.m_pnRedValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrRedValue;
						sLightBuf.m_pnGreenValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrGreenValue;
						sLightBuf.m_pnBlueValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrBlueValue;
						sLightBuf.m_pnWhiteValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrWhiteValue;
						sLightBuf.m_pnPosition = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrLightPosition;
						sLightBuf.m_pnCalculation = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrCalculation;
					}

					if ((sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Top_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Middle_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Bottom_Light))
					{
						m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nRedValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nGreenValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nBlueValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nWhiteValue, wndImg.m_ucArr2D_Mix[iLoopCount]);
						m_procMil->SaveWorkImg(ImgR, width, height, _T("Single2D Red_Mix.bmp"));
						m_procMil->SaveWorkImg(ImgW, width, height, _T("Single2D White_Mix.bmp"));
						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Compose_Mix.bmp"));
					}
					else if ((sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side1_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side2_Light) ||
						(sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side3_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Side4_Light))
					{
						m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nRedValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nGreenValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nBlueValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nWhiteValue, wndImg.m_ucArr2D_Mix[iLoopCount]);
						m_procMil->SaveWorkImg(ImgR, width, height, _T("Side_2D Red_Mix.bmp"));
						m_procMil->SaveWorkImg(ImgW, width, height, _T("Side_2D White_Mix.bmp"));
						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Side_Compose_Mix.bmp"));
					}
					else
					{
						m_InspAlgo.ROIImageClaculCompose(sLightBuf, wndImg.m_ucArr2D_Mix[iLoopCount]);
						m_procMil->SaveWorkImg(sLightBuf.m_pucTRed, width, height, _T("Top Red_Mix.bmp"));
						m_procMil->SaveWorkImg(sLightBuf.m_pucMRed, width, height, _T("Mid Red_Mix.bmp"));
						m_procMil->SaveWorkImg(sLightBuf.m_pucBRed, width, height, _T("Bot Red_Mix.bmp"));
						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Compose_User_Mix.bmp"));
					}
				}

				//m_AlgoDt[_T("OCR_Compose")] = (qtm.EndTick() * 1000.0) + m_AlgoDt[_T("OCR_Compose")];
			}

			if (eInspAlgoType == eAlgoGrid || eInspAlgoType == eAlgoBlob)
			{
				for (int iLoopCount = 0; iLoopCount < sInspAlgo.m_nMixCount; iLoopCount++)
				{
					switch (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType)
					{
					case Top_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;		ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;		ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;	ImgW = m_pInspBoardInfo->partImgBuf.imgTop_W;	break;	}
					case Middle_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;	break;	}
					case Bottom_Light: {	ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;	ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;	break;	}
					case  User_Light:
						sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
						sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
						sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
						sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;

						sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
						sLightBuf.m_pucMGreen = NULL;
						sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
						sLightBuf.m_pucMWhite = NULL;

						sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
						sLightBuf.m_pucBGreen = NULL;
						sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
						sLightBuf.m_pucBWhite = NULL;

						sLightBuf.m_nImgWidth = width;
						sLightBuf.m_nImgHeight = height;
						sLightBuf.m_nROIImgWidth = wnd_w;
						sLightBuf.m_nROIImgHeight = wnd_h;
						sLightBuf.m_dROIX = dX;
						sLightBuf.m_dROIY = dY;
						sLightBuf.m_nImgCnt = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nLightCnt;
						sLightBuf.m_pnRedValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrRedValue;
						sLightBuf.m_pnGreenValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrGreenValue;
						sLightBuf.m_pnBlueValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrBlueValue;
						sLightBuf.m_pnWhiteValue = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrWhiteValue;
						sLightBuf.m_pnPosition = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrLightPosition;
						sLightBuf.m_pnCalculation = sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nArrCalculation;
					}

					if ((sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Top_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Middle_Light) || (sInspAlgo.InspAlgoLightsMix[iLoopCount].m_eLightType == Bottom_Light))
					{
						m_InspAlgo.RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nRedValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nGreenValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nBlueValue, sInspAlgo.InspAlgoLightsMix[iLoopCount].m_nWhiteValue, wndImg.m_ucArr2D_Mix[iLoopCount], dAlignAngle);
						m_procMil->SaveWorkImg(ImgR, width, height, _T("Grid Single2D Red_Mix.bmp"));
						m_procMil->SaveWorkImg(ImgW, width, height, _T("Grid Single2D White_Mix.bmp"));
						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Grid Compose_Mix.bmp"));
					}
					else
					{
						m_InspAlgo.ROIImageClaculCompose(sLightBuf, wndImg.m_ucArr2D_Mix[iLoopCount], dAlignAngle);
						m_procMil->SaveWorkImg(sLightBuf.m_pucTRed, width, height, _T("Grid Top Red_Mix.bmp"));
						m_procMil->SaveWorkImg(sLightBuf.m_pucMRed, width, height, _T("Grid Mid Red_Mix.bmp"));
						m_procMil->SaveWorkImg(sLightBuf.m_pucBRed, width, height, _T("Bot Red_Mix.bmp"));
						m_procMil->SaveWorkImg(wndImg.m_ucArr2D_Mix[iLoopCount], wnd_w, wnd_h, _T("Grid Compose_User_Mix.bmp"));
					}
				}
			}
			AlgoColorOpt ptrAlgoColorOpt;
			if (eInspAlgoType == eAlgoFoot || eInspAlgoType == eAlgoWire || eInspAlgoType == eAlgoSolderCone)
			{
				ptrAlgoColorOpt.m_sFovImg.imgTop_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_R;
				ptrAlgoColorOpt.m_sFovImg.imgTop_G = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_G;
				ptrAlgoColorOpt.m_sFovImg.imgTop_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_B;
				ptrAlgoColorOpt.m_sFovImg.imgTop_W = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgTop_W;

				ptrAlgoColorOpt.m_sFovImg.imgMiddle_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgMiddle_R;
				ptrAlgoColorOpt.m_sFovImg.imgMiddle_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgMiddle_B;

				ptrAlgoColorOpt.m_sFovImg.imgBottom_R = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgBottom_R;
				ptrAlgoColorOpt.m_sFovImg.imgBottom_B = (UCHAR*)m_pInspBoardInfo->partImgBuf.imgBottom_B;

				ptrAlgoColorOpt.m_sImgBuf.nImageSizeX = 0;
				ptrAlgoColorOpt.m_sImgBuf.nImageSizeY = 0;

				ptrAlgoColorOpt.m_rcImageRect.left = dX;
				ptrAlgoColorOpt.m_rcImageRect.top = dY;
				ptrAlgoColorOpt.m_rcImageRect.right = dX + wnd_w;
				ptrAlgoColorOpt.m_rcImageRect.bottom = dY + wnd_h;
				ptrAlgoColorOpt.m_sFovImg.nImageSizeX = width;
				ptrAlgoColorOpt.m_sFovImg.nImageSizeY = height;
			}

			nLine = __LINE__;
			BOOL bAlgoOK = FALSE;
			if (bUSeLeadAlign == FALSE)
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = TRUE;


			//if (CheckAlgoGroup(wndType, nWndIndex, nAlgo, WndResult) == false)
			//	eInspAlgoType = eAlgoNum;

			nLine = __LINE__;
			bool IsUseMultiArea = false;
			if (eInspAlgoType == eAlgoTab || eInspAlgoType == eAlgoColor || eInspAlgoType == eAlgoLead_Color)
			{
				AlgoColor* pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
				if (pAlgoColor && pAlgoColor->IsUseMultiArea())
				{
					IsUseMultiArea = true;
					int nCntTabROI = rcArrTab.GetCount();
					if (pAlgoColor->m_bAutoSearchROI && nCntTabROI > 0)
					{
						for (int i = 0; i < pAlgoColor->m_nCntRect; ++i)
						{
							CRect rcTab(pAlgoColor->m_rcArrTabRect[i]);
							CPoint xyCenOrg = rcTab.CenterPoint();
							int nIdxNear = -1;
							int nDistanceMin = 0;
							for (int nROI = 0; nROI < nCntTabROI; ++nROI)
							{
								CPoint xyOffset = rcArrTab[nROI].CenterPoint();
								xyOffset -= xyCenOrg;
								int nDistance = pow((double)xyOffset.y, 2);
								if ((pAlgoColor->m_nLeadTipDirection == e_TOP) || (pAlgoColor->m_nLeadTipDirection == e_BOTTOM))
									nDistance = pow((double)xyOffset.x, 2);
								if ((nIdxNear == -1) || (nDistanceMin > nDistance))
								{
									nIdxNear = nROI;
									nDistanceMin = nDistance;
								}
							}
							if (nIdxNear < 0) continue;
							CPoint xyOffset = rcArrTab[nIdxNear].CenterPoint() - xyCenOrg;
							rcTab.OffsetRect(xyOffset);
							pAlgoColor->m_rcArrTabRect[i] = rcTab;
						}
					}
				}
			}

			nLine = __LINE__;
			CAlgoFactory cloneFactory;
			void * vRstInspAlgoTemp = nullptr;
			if (bUSeLeadAlign == TRUE && (eInspAlgoType == eAlgoAlign || eInspAlgoType == eAlgoAlignEdge))
				cloneFactory.CreateRstAlgoClone(eInspAlgoType, vRstInspAlgoTemp, IsUseMultiArea);
			else
			{
				cloneFactory.CreateRstAlgoClone(eInspAlgoType, WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo, IsUseMultiArea);
				vRstInspAlgoTemp = WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
			}
			if (CheckAlgoGroup(wndType, nWndIndex, nAlgo, WndResult) == false) /// 기존 Eagle3D 랑 위치바꿈. 
				eInspAlgoType = eAlgoNum;

			nLine = __LINE__;
			InspAlgoParam stAlgoParam;
			//
			stAlgoParam.m_nAlgo = nAlgo;
			stAlgoParam.m_nWndIndex = nWndIndex;
			stAlgoParam.m_nAlignCnt = nAlignCnt;
			stAlgoParam.m_nCurAlignID = nCurAlignID;
			stAlgoParam.m_nWndDir = nWndDir;
			stAlgoParam.m_nLeadTipPos = nLeadTipPos;
			stAlgoParam.m_nInspType = inspType;
			stAlgoParam.m_nOffX_pix = nOffX_pix;
			stAlgoParam.m_nOffY_pix = nOffY_pix;
			stAlgoParam.m_nInspCameraType = nInspCameraType;
			stAlgoParam.m_nLeadTipPosIndex = m_nLeadTipPosIndex;
			stAlgoParam.m_nInspectionMode = 0;//m_nInspectionMode;
			// 			stAlgoParam.m_fRstHeightOffset = 0;
			// 			if (InspDataSet)
			// 				stAlgoParam.m_fRstHeightOffset = InspDataSet->GetHeightOffset();
			nLine = __LINE__;
			//
			stAlgoParam.m_dx = dX;
			stAlgoParam.m_dy = dY;
			stAlgoParam.m_dWndW = wnd_w;
			stAlgoParam.m_dWndH = wnd_h;
			stAlgoParam.m_dInspW = width;
			stAlgoParam.m_dInspH = height;
			stAlgoParam.m_nWndID = m_pParamArray[nWndIndex].wndIndex;
			nLine = __LINE__;
			//
			stAlgoParam.m_byWndInspType = m_pParamArray[nWndIndex].WndInspType;
			//
			stAlgoParam.m_bInspection = TRUE;
			if (eInspAlgoType == eAlgoTab_Search)
				stAlgoParam.m_bTeach = bTabAutoSearchROI;
			stAlgoParam.m_bUSeLeadAlign = bUSeLeadAlign;
			nLine = __LINE__;
			//
			stAlgoParam.m_poCenter.x = (poBodyCenter.x >= 0) ? poBodyCenter.x : wndImg.m_nWidth / 2;
			stAlgoParam.m_poCenter.y = (poBodyCenter.y >= 0) ? poBodyCenter.y : wndImg.m_nHeight / 2;
			stAlgoParam.m_ptWndCenter.x = m_pParamArray[nWndIndex].cx;
			stAlgoParam.m_ptWndCenter.y = m_pParamArray[nWndIndex].cy;
			nLine = __LINE__;
			//
			stAlgoParam.m_pnRet = &ret;
			nLine = __LINE__;
			//
			stAlgoParam.m_rcBlobBody = g_pMManager->pem_new<CRect>(false, 0, (PCHAR)__FUNCTION__, __LINE__, false);
			stAlgoParam.m_rcBlobBody->left = 0;
			stAlgoParam.m_rcBlobBody->right = 0;
			stAlgoParam.m_rcBlobBody->top = 0;
			stAlgoParam.m_rcBlobBody->bottom = 0;
			stAlgoParam.m_rcArrTab = &rcArrTab;
			stAlgoParam.m_rcArrTabBridge = &rcArrTabBridge;
			nLine = __LINE__;
			//
			stAlgoParam.m_sWndInfo = &sWndInfo;
			stAlgoParam.m_sAlignRes = ptrAlignRes;
			stAlgoParam.m_arrAlignRes = arrAlignRes;
			stAlgoParam.m_sInspAlgoTempResult = ptrInspAlgoTempResult;
			stAlgoParam.m_coordinateAlgo = &coordinateAlgo;
			stAlgoParam.m_WndResult = WndResult;
			stAlgoParam.m_sPartAlgoImg = &wndImg;
			stAlgoParam.m_ptrAlgoColorOpt = &ptrAlgoColorOpt;

			stAlgoParam.m_pvInspRstPoly = vpInspRstPoly;
			stAlgoParam.m_rcWND = GetWndROI(nWndIndex, nAlignCnt, ptrAlignRes);
			//
			nLine = __LINE__;


			//
// 			if (eInspAlgoType == eAlgoBarcode)
// 				stAlgoParam.m_arrAlignRes->nAlignWndID = m_pParamArray[nWndIndex].nAlignWndID;

			if (eInspAlgoType != eAlgoNum && m_dicInspAlgorithm[eInspAlgoType])
			{
				nLine = __LINE__;
				UCHAR* ucArrDstImg = (eInspAlgoType == eAlgoWire) ? m_puImgWireInterval : NULL;
				InspRoiImgBuf* sInspImageData = NULL;
				if (eInspAlgoType == eAlgoAlignEdge)
					sInspImageData = sArrInspImgBuf;
				else
					sInspImageData = sInspImgBuf;
				if (eInspAlgoType == eAlgoVolume)
				{
					if (m_pInspBoardInfo->m_bPadPattern == TRUE)
					{
						stAlgoParam.m_bPadPattern = TRUE;
						stAlgoParam.m_sPadPoly = m_pInspBoardInfo->m_sPadPoly;
						stAlgoParam.m_nOverlapPadPolyCnt = m_pInspBoardInfo->m_nOverlapPadPolyCnt;
						stAlgoParam.m_sArrOverlapPadPoly = m_pInspBoardInfo->m_sArrOverlapPadPoly;
						stAlgoParam.m_nPOLY_W = m_pInspBoardInfo->m_nPOLY_W;
						stAlgoParam.m_nPOLY_H = m_pInspBoardInfo->m_nPOLY_H;
						stAlgoParam.m_ucPOLY = m_pInspBoardInfo->m_ucPOLY;
					}
					stAlgoParam.m_sRstAlgo_P = m_pInspBoardInfo->m_sRstAlgo_P;
					stAlgoParam.m_nTYPE = m_pInspBoardInfo->m_nTYPE;
				}
				nLine = __LINE__;
				/*			CString sLogSt;
							sLogSt.Format(_T("Start Algo = %d"), (int)eInspAlgoType);
							g_pMPTI->AddLog(sLogSt);*/

				if (eInspAlgoType == eAlgoTab)
				{
					m_nTabAlgoIdx += 1;
					if (m_nTabAlgoIdx == 0)
					{
						m_pucTabAreaTmp = g_pMManager->pem_new<UCHAR>(true, sWndInfo.dWidth * sWndInfo.dLength, (PCHAR)__FUNCTION__, __LINE__);
					}
				}

				if (bNewInspAlgo == true)
				{
					if (sInspAlgo.m_eAlgoType == eAlgoPatternDiff || sInspAlgo.m_eAlgoType == eAlgoPadBW || sInspAlgo.m_eAlgoType == eAlgoShapeX)
					{
						/*		if (bAutoInsp == FALSE)
									stAlgoParam.m_nInspType = 1;
								else*/
						stAlgoParam.m_nInspType = 0;

						//Target AlignWindowID
						stAlgoParam.m_nCurAlignID = m_pParamArray[nWndIndex].nAlignWndID;
					}
					if (eInspAlgoType == eAlgoPOCR)
					{
						AlgoPOCR* pAlgoPOCR = (AlgoPOCR *)sInspAlgo.m_ptrInspAlgoParam;
						if (pAlgoPOCR->m_bUseLib == TRUE) // Halcon 사용시 InspTarget 변경
						{

							CPInsp_AlgoBarcode* insp = dynamic_cast<CPInsp_AlgoBarcode *>(g_pInspMng->m_dicInspAlgorithm[eAlgoBarcode]);
							if (((CString)pAlgoPOCR->m_sTargetFont).IsEmpty() && pAlgoPOCR->m_sTargetFont > 0) // 사용자가 입력한 Target이 없을때.
							{
								if (insp->m_MapBarcodeInspTarget.find(pAlgoPOCR->m_sWndName) != insp->m_MapBarcodeInspTarget.end()) // 있는지 없는지
								{
									memset(pAlgoPOCR->m_sTargetFont, 0, MAX_STRLEN); // Target 이 길경우 예외
									CString sTarget = insp->m_MapBarcodeInspTarget.at(pAlgoPOCR->m_sWndName).GetString();
									wcsncpy_s(pAlgoPOCR->m_sTargetFont, sTarget, _TRUNCATE);
									pAlgoPOCR->m_nCharMaxCount = sTarget.GetLength(); // Count 를 넣어야지 정상동작
								}

							}
						}
					}
					PIAL::BodyInfo* pBody = nullptr;
					PIAL::PI_Buff* pMaskImg = new PIAL::PI_Buff(pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY);
					g_pMManager->pem_new_check(pMaskImg, (PCHAR)__FUNCTION__, __LINE__);


					if (InspData)
						pBody = new PIAL::BodyInfo; // InspData에서 삭제하므로 pemNew 하지 않음..					
					std::vector<PIAL::MaskInfo*> maskList;
					for (auto & iter : InspDataSet->GetInspDataSet())
					{
						std::vector<PIAL::MaskInfo*> vMarskinFo = iter->GetMaskData();
						maskList.insert(maskList.end(), vMarskinFo.begin(), vMarskinFo.end());
					}
					InspDataSet->SetMaskToImage(pMaskImg, vecAlignResult, maskList, false, dX, dY);
					if (!pImg_buf->m_AngleColorImageBuffer)
					{
						pImg_buf->m_AngleColorImageBuffer = new PIAL::Insp_ROIImg();
					}
					g_pInspMng->GePInspWrapper()->MakeROIImg(sInspImageData, pImg_buf->m_AngleColorImageBuffer);
					m_PInspWrapper->m_PInspAlgo->SetShapeXMatchingPoint(InspData->m_nArrPointsCnt, InspData->m_poArrPoints, InspData->m_nArrMatchingIdx, InspData->m_fArrMatchingSc);
					//PIAL::PInspDataSet::SetMaskToImage(pImg_buf, InspDataSet, maskList, false, dX, dY).copyTo(pMaskImg->Mat());
					if (InspData && pBody)
						InspData->SetBodyData(pBody);
#if _DEBUG
					cv::Mat pi_mat = pImg_buf->m_p2D->Mat();

					//CString debugPath;
					//debugPath.Format(_T("D:\\%d_%d_%d.bmp"), m_pInspBoardInfo->nPartID, nWndIndex, sInspAlgo.m_nAlgoId);
					//cv::imwrite(string(CT2A(debugPath)), pi_mat);


					cv::Mat pi_wnd_mat = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_R)->Mat();

					CString debugPath_2;
					debugPath_2.Format(_T("D:\\A%d_%d_%d.bmp"), m_pInspBoardInfo->nPartID, nWndIndex, sInspAlgo.m_nAlgoId);
					cv::imwrite(string(CT2A(debugPath_2)), pi_wnd_mat);


					//cv::Mat pi_prt_mat = pImg_buf->inspPartImage->m_ImageBuffer->GetBuff(PIAL::Top_W)->Mat();

					//CString debugPath_3;
					//debugPath_3.Format(_T("D:\\B%d_%d_%d.bmp"), m_pInspBoardInfo->nPartID, nWndIndex, sInspAlgo.m_nAlgoId);
					//cv::imwrite(string(CT2A(debugPath_3)), pi_prt_mat);


					cv::Mat done = pi_mat.clone();
#endif

					if (eInspAlgoType == eAlgoPadArray)
						pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(m_pInspBoardInfo->partZmapData.data, m_pInspBoardInfo->partZmapData.zmapSizeX, m_pInspBoardInfo->partZmapData.zmapSizeY);
					bAlgoOK = m_dicInspAlgorithm[eInspAlgoType]->InspAlgorithm_Dll(sInspAlgo, vecAlignResult, pImg_buf, stAlgoParam, vRstInspAlgoTemp, InspDataSet, PIALTieArea, pBody, pMaskImg);

					g_pMManager->pem_delete(pMaskImg, false);
				}
				else
					bAlgoOK = m_dicInspAlgorithm[eInspAlgoType]->InspAlgorithm(sInspAlgo, wndImg, sInspImageData, vRstInspAlgoTemp, stTieArea, stAlgoParam, ucArrDstImg);
				/*if (eInspAlgoType == eAlgoHeight_Mean && m_pInspBoardInfo->zmapData.fResultPartOffset > -900 && WndResult->m_nAlgorithmCnt > nAlgo)
				{
					RstAlgoHeightMean * rst = (RstAlgoHeightMean *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					rst->m_dRstHeightMean = m_pInspBoardInfo->zmapData.fResultPartOffset;
					rst->m_bOKHeightMean = TRUE;
					bAlgoOK = TRUE;
				}*/
				//CString sLogEd;
				//sLogEd.Format(_T("End Algo = %d"), (int)eInspAlgoType);
				//g_pMPTI->AddLog(sLogEd);
				nLine = __LINE__;
				bool bAlgoAlign = false;
				if (eInspAlgoType == eAlgoBody_Blob)
				{
					AlgoBodyBlob *pAlgoBodyBlob = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
					if (((bAlgoOK && pAlgoBodyBlob->m_bNGOffset == FALSE) || pAlgoBodyBlob->m_bNGOffset == TRUE) && !bAlignResOK && inspType == eINSP_MOUNT)
						bAlgoAlign = true;
				}
				else if (eInspAlgoType == eAlgoPattern)
				{
					if (bAlgoOK && (inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
					{
						RstAlgoPattern * rst = (RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
						bAlgoAlign = true;
						if (nAlgo > 0)
						{
							for (int n = 0; n < nAlgo; n++)
							{
								if (WndResult->m_vArrRstInspAlgo[n].m_nAlgoType == eAlgoPattern)
								{
									RstAlgoPattern * rst_tmp = (RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[n].m_vRstInspAlgo;
									if (rst_tmp->score > rst->score)
									{
										bAlgoAlign = false;
										break;
									}
								}
								else if (bAlignResOK)	// 이전 알고리즘 중 패턴외 다른 알고리즘이 있었다면 적용하지 않음
								{
									bAlgoAlign = false;
									break;
								}
							}
						}
					}
					else if (bAlgoOK && (inspType == eINSP_OCR || inspType == eINSP_MOUNT) && InspDataSet != NULL)
					{
						RstAlgoPattern * rst = (RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
						{
							//cv::Mat PatternMat = cv::Mat(wndImg.m_nHeight, wndImg.m_nWidth, CV_8UC1, wndImg.m_ucArr2D);
							AlgoPattern *pAlgoPattern = (AlgoPattern *)sInspAlgo.m_ptrInspAlgoParam;
							//if (bAutoInsp == 0)
							//{
							//	POINTF windowCenterPoint;// = new POINTF();
							//	windowCenterPoint.x = sWndInfo.dCenterX;
							//	windowCenterPoint.y = sWndInfo.dCenterY;
							//	MPTI_SaveMaskWindow(windowCenterPoint, sWndInfo.dWidth, sWndInfo.dLength, pAlgoPattern->m_sWndName, bAlgoOK, m_MaskRstCount, true);
							//	m_MaskRstCount++;
							//}
							PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
							PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();// g_pMManager->pem_new<PIAL::MaskInfo>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

							maskInfo->StartPos.x = rst->m_ptModelPos.x - (rst->ModelWidth / 2);
							maskInfo->StartPos.y = rst->m_ptModelPos.y - (rst->ModelHeight / 2);
							maskInfo->Width = rst->ModelWidth;
							maskInfo->Length = rst->ModelHeight;
							maskInfo->MaskImage = nullptr;

							maskInfo->StartPos.x += (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));// pImg_buf->inspWndImage->nStartX;
							maskInfo->StartPos.y += (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));// pImg_buf->inspWndImage->nStartY;
							//maskInfo->StartPos.y += (nWndCy - (stAlgoParam.m_sWndInfo->dLength / 2));// pImg_buf->inspWndImage->nStartY;
							//PAlgo::RotateImg_ipp2020(pImg_buf.m_p2D->m_pData, &ptrBuff2D, -pAlgoPOCR.m_dFontAngle, nImgWidth, nImgHeight, &nImgWidth, &nImgHeight);

							int nMargin = 9;
							maskInfo->StartPos.x -= nMargin;
							maskInfo->StartPos.y -= nMargin;
							maskInfo->Width += (nMargin * 2);
							maskInfo->Length += (nMargin * 2);


							pData->SetMaskData(maskInfo);
						}

					}
					else if ((bAlgoOK == false) && (inspType == eINSP_OCR) && InspDataSet != NULL)
					{
						//if (bAutoInsp == 0)
						//{
						//	AlgoPattern *pAlgoPattern = (AlgoPattern *)sInspAlgo.m_ptrInspAlgoParam;
							POINTF windowCenterPoint;// = new POINTF();
							windowCenterPoint.x = sWndInfo.dCenterX;
							windowCenterPoint.y = sWndInfo.dCenterY;
						//	MPTI_SaveMaskWindow(windowCenterPoint, sWndInfo.dWidth, sWndInfo.dLength, pAlgoPattern->m_sWndName, bAlgoOK, m_MaskRstCount, true);
						//	m_MaskRstCount++;

							PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
							PIAL::MaskInfo* NGmaskInfo = new PIAL::MaskInfo();

							NGmaskInfo->StartPos.x = stAlgoParam.m_sWndInfo->dCenterX - (sWndInfo.dWidth / 2);
							NGmaskInfo->StartPos.y = stAlgoParam.m_sWndInfo->dCenterY - (sWndInfo.dLength / 2);

							//NGmaskInfo->StartPos.x = windowCenterPoint.x - (sWndInfo.dWidth / 2);
							//NGmaskInfo->StartPos.y = windowCenterPoint.y - (sWndInfo.dLength / 2);
							NGmaskInfo->Width = sWndInfo.dWidth;
							NGmaskInfo->Length = sWndInfo.dLength;
							pData->SetMaskData(NGmaskInfo);
						//}
					}

					/*
#pragma region CalcMaskArea
					RstAlgoPattern * rst_tmp = (RstAlgoPattern *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
					PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();// g_pMManager->pem_new<PIAL::MaskInfo>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
					maskInfo->StartPos.x = rst_tmp->m_ptModelPos.x - rst_tmp->ModelWidth / 2;
					maskInfo->StartPos.y = rst_tmp->m_ptModelPos.y - rst_tmp->ModelHeight / 2;
					maskInfo->Width = rst_tmp->ModelWidth;
					maskInfo->Length = rst_tmp->ModelHeight;
					maskInfo->MaskImage = nullptr;
					if (dAlignAngle != 0)
					{
						cv::Point Ce;
						Ce.x = wndImg.m_nWidth / 2; Ce.y = wndImg.m_nHeight / 2;
						cv::Point pLT = RotatePoint(cv::Point(maskInfo->StartPos.x, maskInfo->StartPos.y), Ce, dAlignAngle);
						cv::Point pRT = RotatePoint(cv::Point(maskInfo->StartPos.x + maskInfo->Width, maskInfo->StartPos.y), Ce, dAlignAngle);
						cv::Point pLB = RotatePoint(cv::Point(maskInfo->StartPos.x, maskInfo->StartPos.y + maskInfo->Length), Ce, dAlignAngle);
						cv::Point pRB = RotatePoint(cv::Point(maskInfo->StartPos.x + maskInfo->Width, maskInfo->StartPos.y + maskInfo->Length), Ce, dAlignAngle);
						int nL = min(min(pLT.x, pLB.x), min(pRT.x, pRB.x));
						int nR = max(max(pLT.x, pLB.x), max(pRT.x, pRB.x));
						int nT = min(min(pLT.y, pLB.y), min(pRT.y, pRB.y));
						int nB = max(max(pLT.y, pLB.y), max(pRT.y, pRB.y));
						maskInfo->StartPos.x = nL;
						maskInfo->StartPos.y = nT;
						maskInfo->Width = nR - nL;
						maskInfo->Length = nB - nT;
					}
					maskInfo->StartPos.x += stAlgoParam.m_dx;
					maskInfo->StartPos.y += stAlgoParam.m_dy;
					//PAlgo::RotateImg_ipp2020(pImg_buf.m_p2D->m_pData, &ptrBuff2D, -pAlgoPOCR.m_dFontAngle, nImgWidth, nImgHeight, &nImgWidth, &nImgHeight);
					pData->SetMaskData(maskInfo);
#pragma endregion
*/
				}
				else if (eInspAlgoType == eAlgoOCR)
				{
					USENGOCR = *stAlgoParam.m_dUSENGOCR;
					OCRNGScore = *stAlgoParam.m_dOCRNGScore;
					OCROKScore = *stAlgoParam.m_dOCROKScore;
				}
				else if (eInspAlgoType == eAlgoLead_Tip)
				{
					RstAlgoLeadTip * rst = (RstAlgoLeadTip *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					AlgoLeadTip *pInspAlgoLeadTip = (AlgoLeadTip *)sInspAlgo.m_ptrInspAlgoParam;
					nLeadTipPos = rst->m_nRstLeadTipPos;
					bUseLeadTipSideOption = pInspAlgoLeadTip->m_bUseSideTip;
					nLeadTipDir = pInspAlgoLeadTip->nLeadDirection;
					dLeadTipShiftX = rst->m_dRstSideTipShiftX * -1;
					dLeadTipShiftY = rst->m_dRstSideTipShiftY * -1;
					dLeadTipSize = rst->m_dRstSideTipSize;
					//m_AlgoDt[_T("eAlgoLead_Tip")] = (qtm.EndTick() * 1000.0) + m_AlgoDt[_T("eAlgoLead_Tip")];
					nLine = __LINE__;
					ptrnArrLeadTipRst[nInspLeadTipCnt] = rst->m_nRstLeadTipPos;
					nInspLeadTipCnt++;
					if (nInspLeadTipCnt == nLeadTipTotalCnt)
					{
						if (g_pInspMng->m_dicInspAlgorithm[eInspAlgoType])
						{
							CPInsp_AlgoLeadTip* inspLeadTip = dynamic_cast<CPInsp_AlgoLeadTip*>(g_pInspMng->m_dicInspAlgorithm[eInspAlgoType]);
							if (inspLeadTip && WndResult->m_nAlgorithmCnt > nAlgo)
							{
								int nTipPosBuf = inspLeadTip->TipMaster(sInspAlgo, ptrnArrLeadTipRst, nInspLeadTipCnt, (RstAlgoLeadTip *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo);
								if (nTipPosBuf >= 0)
									nLeadTipPos = nTipPosBuf;
							}
						}
					}
				}
				else if (eInspAlgoType == eAlgoBodyEdge && bAlgoOK == false && WndResult->m_nAlgorithmCnt > nAlgo)
				{
					RstAlgoBodyEdge * rst = (RstAlgoBodyEdge *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					//if (rst->m_bOKShiftX && rst->m_bOKShiftY && rst->m_bOKAngle)
					if (!bAlignResOK && inspType == eINSP_ALIGN)
						bAlgoAlign = true;
				}
				else if (eInspAlgoType == eAlgoBarcode && InspDataSet != NULL)
				{
					AlgoBarcode *pAlgoBarcode = (AlgoBarcode *)sInspAlgo.m_ptrInspAlgoParam;
					RstAlgoBarcode * rst = (RstAlgoBarcode *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					//if (bAutoInsp == 0)
					//{
					//	POINTF windowCenterPoint;
					//	windowCenterPoint.x = sWndInfo.dCenterX;
					//	windowCenterPoint.y = sWndInfo.dCenterY;
					//	MPTI_SaveMaskWindow(windowCenterPoint, sWndInfo.dWidth, sWndInfo.dLength, pAlgoBarcode->m_sWndName, bAlgoOK, m_MaskRstCount, true);
					//	m_MaskRstCount++;
					//}
					if (pAlgoBarcode->m_bUseBarcodeAlign == true)
						bAlgoAlign = true;
					//else
					{
						bool bHalconInsp = ((pAlgoBarcode->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_HALCON_Barcode) == (int)m_eAlgoBar_Data2_HALCON_Barcode);
						bool bQualityInsp = ((pAlgoBarcode->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseBarcodeQuality) == (int)m_eAlgoBar_Data2_UseBarcodeQuality);
						PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
						PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();// g_pMManager->pem_new<PIAL::MaskInfo>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

						if (((pAlgoBarcode->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_DATAMATRIX) == (int)m_eAlgoBar_Data_DATAMATRIX))
						{	// 2D Barcode(DataMatrix) Result Save for OCR Text
							CPInsp_AlgoBarcode* insp = dynamic_cast<CPInsp_AlgoBarcode *>(g_pInspMng->m_dicInspAlgorithm[eInspAlgoType]);

							if (insp->m_MapBarcodeInspTarget.size() == 0)
							{
								insp->BarcodeSplitInspTarget(rst); // 2D DataMatrix Data Save
							}
						}

						if (bHalconInsp == true && rst->m_nBarType == 1)
						{
							std::vector<cv::Point2f> points;
							for (int i = 0; i < 4; i++)// 영역 계산된 Corner Point 필요
								points.emplace_back(cv::Point2f(rst->m_ptRst_Coner[i].x, rst->m_ptRst_Coner[i].y));
							cv::RotatedRect rotatedRect = cv::minAreaRect(points); // 가장 작은 회전 사각형 생성
							//int nResCnt = rst->nInspRectCnt;
							std::sort(std::begin(rst->InspBarRect), std::end(rst->InspBarRect), [](const CRect& a, const CRect& b) {
								return std::tie(a.TopLeft().y, a.TopLeft().x) < std::tie(a.TopLeft().y, a.TopLeft().x);
							});
							PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
							int nMargin = 3;
							if (bQualityInsp && rst->nDetectRectCnt > 0)
							{	// 1D Barcode 품질검사 옵션 사용
								int nSt = 0, nEd = rst->nInspRectCnt - 1;
								int nMarginX = pAlgoBarcode->m_nArrData[(int)m_eAlgoBarQuality1DOffsetX];
								int nMarginY = pAlgoBarcode->m_nArrData[(int)m_eAlgoBarQuality1DOffsetY];

								for (int ocrmask = nSt; ocrmask <= nEd; ocrmask++)
								{
									PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();

									//maskInfo->StartPos.x = ((CRect)rst->InspBarRect[ocrmask]).TopLeft().x - nMarginX;//sRstAlgo->charX[ocrmask] - sRstAlgo->charWidth[ocrmask] / 2;
									//maskInfo->StartPos.y = ((CRect)rst->InspBarRect[ocrmask]).TopLeft().y - nMarginY; //sRstAlgo->charY[ocrmask] - sRstAlgo->charHeight[ocrmask] / 2;
									//maskInfo->Width = ((CRect)rst->InspBarRect[ocrmask]).Width() + nMarginX * 2;
									//maskInfo->Length = ((CRect)rst->InspBarRect[ocrmask]).Height() + nMarginY * 2;
									//maskInfo->MaskImage = nullptr;
									////dAlignAngle -= rotatedRect.angle;// pAlgoPOCR->m_dFontAngle;
									//maskInfo->StartPos.x += (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));// pImg_buf->inspWndImage->nStartX;
									//maskInfo->StartPos.y += (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));// pImg_buf->inspWndImage->nStartY;

									// KHS 2506 New QualityInsp
									maskInfo->StartPos.x = ((CRect)rst->DetectBarRect[ocrmask]).TopLeft().x;
									maskInfo->StartPos.y = ((CRect)rst->DetectBarRect[ocrmask]).TopLeft().y;
									maskInfo->StartPos.x += (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));
									maskInfo->StartPos.y += (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));
									maskInfo->Width = ((CRect)rst->DetectBarRect[ocrmask]).Width();
									maskInfo->Length = ((CRect)rst->DetectBarRect[ocrmask]).Height();
									maskInfo->MaskImage = nullptr;

									maskInfo->StartPos.x -= nMargin;
									maskInfo->StartPos.y -= nMargin;
									maskInfo->Width += (nMargin * 2);
									maskInfo->Length += (nMargin * 2);

									pData->SetMaskData(maskInfo);
								}
							}
							else
							{
								PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();// g_pMManager->pem_new<PIAL::MaskInfo>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

								if (((pAlgoBarcode->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_DATAMATRIX) == (int)m_eAlgoBar_Data_DATAMATRIX))
								{
									CPInsp_AlgoBarcode* insp = dynamic_cast<CPInsp_AlgoBarcode *>(g_pInspMng->m_dicInspAlgorithm[eInspAlgoType]);

									if (insp->m_MapBarcodeInspTarget.size() == 0)
									{
										insp->BarcodeSplitInspTarget(rst); // 2D DataMatrix Data Save
									}


								}
								maskInfo->StartPos.x = rst->m_rcROI.left;
								maskInfo->StartPos.y = rst->m_rcROI.top;
								maskInfo->Width = rst->m_rcROI.right - rst->m_rcROI.left;
								maskInfo->Length = rst->m_rcROI.bottom - rst->m_rcROI.top;


								maskInfo->StartPos.x += (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));// pImg_buf->inspWndImage->nStartX;
								maskInfo->StartPos.y += (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));// pImg_buf->inspWndImage->nStartY;
								//PAlgo::RotateImg_ipp2020(pImg_buf.m_p2D->m_pData, &ptrBuff2D, -pAlgoPOCR.m_dFontAngle, nImgWidth, nImgHeight, &nImgWidth, &nImgHeight);
								int ckNull = 0;
								for (int i = 0; i < 4; i++)
								{
									if (rst->m_ptRst_Coner[i].x == 0 && rst->m_ptRst_Coner[i].y == 0)
										ckNull++;
								}

								if (bHalconInsp && (ckNull < 2))
								{



									cv::Point MarginPoint[4] = { 0, };

									int totalMargin = 15;
									double dMargin = totalMargin * std::sqrt(2);
									double radian = 45 * CV_PI / 180.0f;

									cv::Point2f direction(std::cos(radian), std::sin(radian));

									double cenX = 0, cenY = 0;

									for (int i = 0; i < 4; i++)
									{
										cenX += rst->m_ptRst_Coner[i].x;
										cenY += rst->m_ptRst_Coner[i].y;
									}

									cenX = cenX / 4;
									cenY = cenY / 4;

									double startXMin = rst->m_ptRst_Coner[0].x, startYMin = rst->m_ptRst_Coner[0].y;

									for (int i = 0; i < 4; i++)
									{
										startXMin = MIN(startXMin, rst->m_ptRst_Coner[i].x);
										startYMin = MIN(startYMin, rst->m_ptRst_Coner[i].y);
									}


									maskInfo->StartPos.x = startXMin + (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));
									maskInfo->StartPos.y = startYMin + (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));

									maskInfo->StartPos.x -= direction.x * dMargin;
									maskInfo->StartPos.y -= direction.y * dMargin;
									maskInfo->Width += ((direction.x * dMargin) * 2);
									maskInfo->Length += ((direction.y * dMargin) * 2);



									for (int i = 0; i < 4; i++)
									{
										if (cenX < rst->m_ptRst_Coner[i].x)
											MarginPoint[i].x = rst->m_ptRst_Coner[i].x + direction.x * dMargin;
										else
											MarginPoint[i].x = rst->m_ptRst_Coner[i].x - direction.x * dMargin > 0 ? rst->m_ptRst_Coner[i].x - direction.x * dMargin : 0;

										if (cenY < rst->m_ptRst_Coner[i].y)
											MarginPoint[i].y = rst->m_ptRst_Coner[i].y + direction.y * dMargin;
										else
											MarginPoint[i].y = rst->m_ptRst_Coner[i].y - direction.y * dMargin > 0 ? rst->m_ptRst_Coner[i].y - direction.y * dMargin : 0;


									}

									double rstXmin = MarginPoint[0].x, rstXmax = MarginPoint[0].x, rstYmin = MarginPoint[0].y, rstYmax = MarginPoint[0].y;
									for (int i = 0; i < 4; i++)
									{
										rstXmax = MAX(rstXmax, MarginPoint[i].x);
										rstXmin = MIN(rstXmin, MarginPoint[i].x);
										rstYmax = MAX(rstYmax, MarginPoint[i].y);
										rstYmin = MIN(rstYmin, MarginPoint[i].y);
									}

									if (coordinateAlgo.dROIAngle == 90 || coordinateAlgo.dROIAngle == 270)
										maskInfo->MaskImage = new PIAL::PI_Buff(rstXmax - rstXmin, rstYmax - rstYmin);
									else
										maskInfo->MaskImage = new PIAL::PI_Buff(rstYmax - rstYmin, rstXmax - rstXmin);

									cv::Mat MaskImg;
									if (coordinateAlgo.dROIAngle == 90 || coordinateAlgo.dROIAngle == 270)
										MaskImg = cv::Mat::zeros(rstYmax - rstYmin, rstXmax - rstXmin, CV_8UC1);
									else
										MaskImg = cv::Mat::zeros(rstXmax - rstXmin, rstYmax - rstYmin, CV_8UC1);
									MaskImg.setTo(cv::Scalar(255, 255, 255));
									std::vector <cv::Point> ROIpoints;
									ROIpoints.clear();
									for (int i = 0; i < 4; i++)
									{
										ROIpoints.push_back(cv::Point(MarginPoint[i].x - rstXmin, MarginPoint[i].y - rstYmin));
									}
									cv::Scalar fillcolor(0, 0, 0);
									std::vector<std::vector<cv::Point>> pts = { ROIpoints };
									cv::fillPoly(MaskImg, pts, fillcolor);
									MaskImg.copyTo(maskInfo->MaskImage->Mat());
								}
								else
									maskInfo->MaskImage = nullptr;

								int nMargin = 0;
								if (bHalconInsp)
									nMargin = 0;
								else
									nMargin = 21;

								maskInfo->StartPos.x -= nMargin;
								maskInfo->StartPos.y -= nMargin;
								maskInfo->Width += (nMargin * 2);
								maskInfo->Length += (nMargin * 2);

								pData->SetMaskData(maskInfo);
							}
						}
						else if (bHalconInsp == true && rst->m_nBarType == 2)
						{
							maskInfo->StartPos.x = rst->m_rcROI.left;
							maskInfo->StartPos.y = rst->m_rcROI.top;
							maskInfo->Width = rst->m_rcROI.right - rst->m_rcROI.left;
							maskInfo->Length = rst->m_rcROI.bottom - rst->m_rcROI.top;


							maskInfo->StartPos.x += (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));// pImg_buf->inspWndImage->nStartX;
							maskInfo->StartPos.y += (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));// pImg_buf->inspWndImage->nStartY;
							//PAlgo::RotateImg_ipp2020(pImg_buf.m_p2D->m_pData, &ptrBuff2D, -pAlgoPOCR.m_dFontAngle, nImgWidth, nImgHeight, &nImgWidth, &nImgHeight);
							int ckNull = 0;
							for (int i = 0; i < 4; i++)
							{
								if (rst->m_ptRst_Coner[i].x == 0 && rst->m_ptRst_Coner[i].y == 0)
									ckNull++;
							}
							if (bHalconInsp && (ckNull < 2))
							{

								cv::Point MarginPoint[4] = { 0, };

								int totalMargin = 15;
								double dMargin = totalMargin * std::sqrt(2);
								double radian = 45 * CV_PI / 180.0f;

								cv::Point2f direction(std::cos(radian), std::sin(radian));

								double cenX = 0, cenY = 0;

								for (int i = 0; i < 4; i++)
								{
									cenX += rst->m_ptRst_Coner[i].x;
									cenY += rst->m_ptRst_Coner[i].y;
								}

								cenX = cenX / 4;
								cenY = cenY / 4;
								double startXMin = rst->m_ptRst_Coner[0].x, startYMin = rst->m_ptRst_Coner[0].y;

								for (int i = 0; i < 4; i++)
								{
									startXMin = MIN(startXMin, rst->m_ptRst_Coner[i].x);
									startYMin = MIN(startYMin, rst->m_ptRst_Coner[i].y);
								}


								maskInfo->StartPos.x = startXMin + (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));
								maskInfo->StartPos.y = startYMin + (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));

								maskInfo->StartPos.x -= direction.x * dMargin;
								maskInfo->StartPos.y -= direction.y * dMargin;
								maskInfo->Width += ((direction.x * dMargin) * 2);
								maskInfo->Length += ((direction.y * dMargin) * 2);

								for (int i = 0; i < 4; i++)
								{
									if (cenX < rst->m_ptRst_Coner[i].x)
										MarginPoint[i].x = rst->m_ptRst_Coner[i].x + direction.x * dMargin;
									else
										MarginPoint[i].x = rst->m_ptRst_Coner[i].x - direction.x * dMargin > 0 ? rst->m_ptRst_Coner[i].x - direction.x * dMargin : 0;

									if (cenY < rst->m_ptRst_Coner[i].y)
										MarginPoint[i].y = rst->m_ptRst_Coner[i].y + direction.y * dMargin;
									else
										MarginPoint[i].y = rst->m_ptRst_Coner[i].y - direction.y * dMargin > 0 ? rst->m_ptRst_Coner[i].y - direction.y * dMargin : 0;
								}

								double rstXmin = MarginPoint[0].x, rstXmax = MarginPoint[0].x, rstYmin = MarginPoint[0].y, rstYmax = MarginPoint[0].y;
								for (int i = 0; i < 4; i++)
								{
									rstXmax = MAX(rstXmax, MarginPoint[i].x);
									rstXmin = MIN(rstXmin, MarginPoint[i].x);
									rstYmax = MAX(rstYmax, MarginPoint[i].y);
									rstYmin = MIN(rstYmin, MarginPoint[i].y);
								}
								if (coordinateAlgo.dROIAngle == 90 || coordinateAlgo.dROIAngle == 270)
									maskInfo->MaskImage = new PIAL::PI_Buff(rstXmax - rstXmin, rstYmax - rstYmin);
								else
									maskInfo->MaskImage = new PIAL::PI_Buff(rstYmax - rstYmin, rstXmax - rstXmin);

								cv::Mat MaskImg;
								if (coordinateAlgo.dROIAngle == 90 || coordinateAlgo.dROIAngle == 270)
									MaskImg = cv::Mat::zeros(rstYmax - rstYmin, rstXmax - rstXmin, CV_8UC1);
								else
									MaskImg = cv::Mat::zeros(rstXmax - rstXmin, rstYmax - rstYmin, CV_8UC1);
								MaskImg.setTo(cv::Scalar(255, 255, 255));
								std::vector <cv::Point> ROIpoints;
								ROIpoints.clear();
								for (int i = 0; i < 4; i++)
								{
									ROIpoints.push_back(cv::Point(MarginPoint[i].x - rstXmin, MarginPoint[i].y - rstYmin));
								}
								cv::Scalar fillcolor(0, 0, 0);
								std::vector<std::vector<cv::Point>> pts = { ROIpoints };
								cv::fillPoly(MaskImg, pts, fillcolor);
								MaskImg.copyTo(maskInfo->MaskImage->Mat());
							}
							else
								maskInfo->MaskImage = nullptr;


							int nMargin = 0;
							if (bHalconInsp)
								nMargin = 0;
							else
								nMargin = 21;

							maskInfo->StartPos.x -= nMargin;
							maskInfo->StartPos.y -= nMargin;
							maskInfo->Width += (nMargin * 2);
							maskInfo->Length += (nMargin * 2);

							pData->SetMaskData(maskInfo);
						}
						else
							maskInfo->MaskImage = nullptr;
						if (false)
						{
#pragma region CalcMaskArea
							cv::Mat BCodeMat(wndImg.m_nHeight, wndImg.m_nWidth, CV_8UC1, wndImg.m_ucArr2D), BCodeMatDst;
							//AlgoBarcode *pAlgoBarcode = (AlgoBarcode *)sInspAlgo.m_ptrInspAlgoParam;
							double dCX = 0;
							double dCY = 0;
							double dArea = 0;
							int nMinBlobArea = 1;
							CRect rcBlob(0, 0, 0, 0);
							cv::threshold(BCodeMat, BCodeMatDst, 0, 255, cv::THRESH_OTSU);
							if ((pAlgoBarcode->m_nArrData[(int)m_eAlgoBar::m_eAlgoBarData] & (int)m_eAlgoBar_Data_ForeColor) != (int)m_eAlgoBar_Data_ForeColor)
								cv::threshold(BCodeMatDst, BCodeMatDst, 0, 255, cv::THRESH_BINARY_INV);
							int nCntBlob = m_procMil->CalcBlob_Select(BCodeMatDst.ptr(), NULL, BCodeMatDst.cols, BCodeMatDst.rows, nMinBlobArea, false, false, 0, eSelectCenter);

							m_procMil->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
							PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
							PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();// g_pMManager->pem_new<PIAL::MaskInfo>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
							maskInfo->StartPos.x = rcBlob.left;
							maskInfo->StartPos.y = rcBlob.top;
							maskInfo->Width = rcBlob.right - rcBlob.left;
							maskInfo->Length = rcBlob.bottom - rcBlob.top;
							maskInfo->MaskImage = nullptr;
							if (dAlignAngle != 0)
							{
								cv::Point Ce;
								Ce.x = wndImg.m_nWidth / 2; Ce.y = wndImg.m_nHeight / 2;
								cv::Point pLT = RotatePoint(cv::Point(maskInfo->StartPos.x, maskInfo->StartPos.y), Ce, dAlignAngle);
								cv::Point pRT = RotatePoint(cv::Point(maskInfo->StartPos.x + maskInfo->Width, maskInfo->StartPos.y), Ce, dAlignAngle);
								cv::Point pLB = RotatePoint(cv::Point(maskInfo->StartPos.x, maskInfo->StartPos.y + maskInfo->Length), Ce, dAlignAngle);
								cv::Point pRB = RotatePoint(cv::Point(maskInfo->StartPos.x + maskInfo->Width, maskInfo->StartPos.y + maskInfo->Length), Ce, dAlignAngle);
								int nL = min(min(pLT.x, pLB.x), min(pRT.x, pRB.x));
								int nR = max(max(pLT.x, pLB.x), max(pRT.x, pRB.x));
								int nT = min(min(pLT.y, pLB.y), min(pRT.y, pRB.y));
								int nB = max(max(pLT.y, pLB.y), max(pRT.y, pRB.y));
								maskInfo->StartPos.x = nL;
								maskInfo->StartPos.y = nT;
								maskInfo->Width = nR - nL;
								maskInfo->Length = nB - nT;
							}
							maskInfo->StartPos.x += stAlgoParam.m_dx;
							maskInfo->StartPos.y += stAlgoParam.m_dy;
							//PAlgo::RotateImg_ipp2020(pImg_buf.m_p2D->m_pData, &ptrBuff2D, -pAlgoPOCR.m_dFontAngle, nImgWidth, nImgHeight, &nImgWidth, &nImgHeight);
							pData->SetMaskData(maskInfo);
#pragma endregion
						}

					}
				}
				else if (eInspAlgoType == eAlgoPOCR)
				{
					AlgoPOCR *pAlgoPOCR = (AlgoPOCR *)sInspAlgo.m_ptrInspAlgoParam;
					RstAlgoPOCR * sRstAlgo = (RstAlgoPOCR *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					POINTF windowCenterPoint;
					windowCenterPoint.x = 0;
					windowCenterPoint.y = 0;
					/*		if (bAutoInsp == 0)
							{
								windowCenterPoint.x = sWndInfo.dCenterX;
								windowCenterPoint.y = sWndInfo.dCenterY;
								MPTI_SaveMaskWindow(windowCenterPoint, sWndInfo.dWidth, sWndInfo.dLength, pAlgoPOCR->m_sWndName, bAlgoOK, m_MaskRstCount, true);
								m_MaskRstCount++;
							}*/
					if (bAlgoOK)
					{
						int POCRImgrows = 0;
						int POCRImgcows = 0;
						if (bNewInspAlgo == true)
						{
							POCRImgrows = pImg_buf->m_p2D->Width();
							POCRImgcows = pImg_buf->m_p2D->Length();
						}
						else
						{
							POCRImgrows = wndImg.m_nWidth;
							POCRImgcows = wndImg.m_nHeight;
						}
						double dAlignAngle = 0.0f;
						if (g_pInspMng->WindowRotate())
						{
							for (int i = 0; i < vecAlignResult.size(); i++)
							{
								dAlignAngle -= vecAlignResult[i].theta;
							}
						}
						dAlignAngle -= m_pInspBoardInfo->angle;


						if (pAlgoPOCR->m_bUseLib == true && InspDataSet != NULL)
						{
							int nResCnt = sRstAlgo->m_nCharCount;
							PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
							//PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();// g_pMManager->pem_new<PIAL::MaskInfo>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
							int nSt = 0, nEd = nResCnt - 1;
							for (int nCharIdx = 0; nCharIdx < nResCnt; nCharIdx++)
							{
								if (sRstAlgo->charX[nCharIdx] != 0 && sRstAlgo->charX[nCharIdx] != 0)
								{
									nSt = nCharIdx;
									break;
								}
							}
							for (int nCharIdx = nEd; nCharIdx > nSt; nCharIdx--)
							{
								if (sRstAlgo->charX[nCharIdx] != 0 && sRstAlgo->charX[nCharIdx] != 0)
								{
									nEd = nCharIdx;
									break;
								}
							}
							for (int ocrcnt = nSt; ocrcnt <= nEd; ocrcnt++)
							{
								PIAL::MaskInfo* maskInfo = new PIAL::MaskInfo();

								maskInfo->StartPos.x = sRstAlgo->charX[ocrcnt] - sRstAlgo->charWidth[ocrcnt] / 2;
								maskInfo->StartPos.y = sRstAlgo->charY[ocrcnt] - sRstAlgo->charHeight[ocrcnt] / 2;
								maskInfo->Width = (sRstAlgo->charX[ocrcnt] + sRstAlgo->charWidth[ocrcnt] / 2) - maskInfo->StartPos.x;
								maskInfo->Length = max(max(sRstAlgo->charHeight[ocrcnt], sRstAlgo->charHeight[ocrcnt]), ((sRstAlgo->charY[ocrcnt] + sRstAlgo->charHeight[ocrcnt] / 2) - maskInfo->StartPos.y));
								maskInfo->MaskImage = nullptr;
								dAlignAngle -= pAlgoPOCR->m_dFontAngle;
								if (dAlignAngle != 0)
								{
									cv::Point Ce;
									if (pAlgoPOCR->m_dFontAngle == 90 || pAlgoPOCR->m_dFontAngle == 270)
									{
										Ce.x = POCRImgrows / 2; Ce.y = POCRImgcows / 2;
									}
									else
									{
										Ce.x = POCRImgcows / 2; Ce.y = POCRImgrows / 2;
									}
									cv::Point pLT = g_pInspMng->RotatePoint(cv::Point(maskInfo->StartPos.x, maskInfo->StartPos.y), Ce, dAlignAngle);
									cv::Point pRT = g_pInspMng->RotatePoint(cv::Point(maskInfo->StartPos.x + maskInfo->Width, maskInfo->StartPos.y), Ce, dAlignAngle);
									cv::Point pLB = g_pInspMng->RotatePoint(cv::Point(maskInfo->StartPos.x, maskInfo->StartPos.y + maskInfo->Length), Ce, dAlignAngle);
									cv::Point pRB = g_pInspMng->RotatePoint(cv::Point(maskInfo->StartPos.x + maskInfo->Width, maskInfo->StartPos.y + maskInfo->Length), Ce, dAlignAngle);
									int nL = min(min(pLT.x, pLB.x), min(pRT.x, pRB.x));
									int nR = max(max(pLT.x, pLB.x), max(pRT.x, pRB.x));
									int nT = min(min(pLT.y, pLB.y), min(pRT.y, pRB.y));
									int nB = max(max(pLT.y, pLB.y), max(pRT.y, pRB.y));
									maskInfo->StartPos.x = nL;
									maskInfo->StartPos.y = nT;
									maskInfo->Width = nR - nL;
									maskInfo->Length = nB - nT;
								}

								maskInfo->StartPos.x += (stAlgoParam.m_sWndInfo->dCenterX - (stAlgoParam.m_sWndInfo->dWidth / 2));// pImg_buf->inspWndImage->nStartX;
								maskInfo->StartPos.y += (stAlgoParam.m_sWndInfo->dCenterY - (stAlgoParam.m_sWndInfo->dLength / 2));// pImg_buf->inspWndImage->nStartY;
								//PAlgo::RotateImg_ipp2020(pImg_buf.m_p2D->m_pData, &ptrBuff2D, -pAlgoPOCR.m_dFontAngle, nImgWidth, nImgHeight, &nImgWidth, &nImgHeight);

								int nMargin = 15;
								maskInfo->StartPos.x -= nMargin;
								maskInfo->StartPos.y -= nMargin;
								maskInfo->Width += (nMargin * 2);
								maskInfo->Length += (nMargin * 2);

								pData->SetMaskData(maskInfo);
							}
						}
					}
					else
					{
						if (InspDataSet != NULL)
						{
							PIAL::PInspData* pData = InspDataSet->GetInspData(stAlgoParam.m_nWndID);
							PIAL::MaskInfo* NGmaskInfo = new PIAL::MaskInfo();
							NGmaskInfo->StartPos.x = stAlgoParam.m_sWndInfo->dCenterX - (sWndInfo.dWidth / 2);
							NGmaskInfo->StartPos.y = stAlgoParam.m_sWndInfo->dCenterY - (sWndInfo.dLength / 2);
							NGmaskInfo->Width = sWndInfo.dWidth;
							NGmaskInfo->Length = sWndInfo.dLength;
							pData->SetMaskData(NGmaskInfo);
						}

					}
				}
				else if (eInspAlgoType == eAlgoPatternDiff)
				{
					AlgoPatternDiff *pAlgoPatternDiff = (AlgoPatternDiff *)sInspAlgo.m_ptrInspAlgoParam;
					if (pAlgoPatternDiff->m_bUseArrAlignArea && (inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
					{
						RstAlgoPatternDiff * rst = (RstAlgoPatternDiff *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
						if (bAlgoOK && rst->m_dAlignScore >= pAlgoPatternDiff->m_nAcceptAlignScore)
						{
							bAlgoAlign = true;
						}
					}

				}
				else if (eInspAlgoType == eAlgoNGBlob)
				{
					RstAlgoNGBlob * rst = (RstAlgoNGBlob *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
					if (rst->OutlineChipping[7] != 0)
					{
						rst->OutlineChipping[0] += dX;
						rst->OutlineChipping[2] += dX;
						rst->OutlineChipping[4] += dX;
						rst->OutlineChipping[6] += dX;

						rst->OutlineChipping[1] += dY;
						rst->OutlineChipping[3] += dY;
						rst->OutlineChipping[5] += dY;
						rst->OutlineChipping[7] += dY;
					}
				}
				else if (eInspAlgoType == eAlgoShapeX)
				{
					if (InspData)
						InspData->SetArrPoints(((RstAlgoShapeX*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo)->nROICnt, ((RstAlgoShapeX*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo)->poArrCenter, ((RstAlgoShapeX*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo)->nMatchingIdx, ((RstAlgoShapeX*)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo)->nMatchingSc);
				}
				else
					bAlgoAlign = bAlgoOK && !bAlignResOK;

				//if (bAutoInsp == 0 && InspDataSet != NULL)
				//{
				//	std::vector<PIAL::MaskInfo*> maskWList;
				//	for (auto & iter : InspDataSet->GetInspDataSet())
				//	{
				//		std::vector<PIAL::MaskInfo*> vMarskinFo = iter->GetMaskData();
				//		maskWList.insert(maskWList.end(), vMarskinFo.begin(), vMarskinFo.end());
				//	}
				//	if (g_pInspMng != NULL)
				//		g_pInspMng->m_SaveMaskRect = NULL;
				//	RECT* SaveRect = new RECT[maskWList.size()];
				//	int ckMaskSize = 0;
				//	for (auto & iter : maskWList)
				//	{
				//		cv::Rect rectM;
				//		rectM.x = iter->StartPos.x;
				//		rectM.y = iter->StartPos.y;

				//		rectM.width = iter->Width;
				//		rectM.height = iter->Length;
				//		SaveRect[ckMaskSize].left = rectM.x;
				//		SaveRect[ckMaskSize].top = rectM.y;
				//		SaveRect[ckMaskSize].right = rectM.x + rectM.width;
				//		SaveRect[ckMaskSize].bottom = rectM.y + rectM.height;
				//		ckMaskSize++;
				//	}
				//	if (maskWList.size() != 0)
				//		ckMaskSize = MPTI_SaveMaskRect(SaveRect, ckMaskSize, true);
				//	delete[] SaveRect;
				//}
				//if (bLabelPartDebugImgSave && InspDataSet != NULL)
				//{
				//	//shkim 2023.06.15
				//	//mask Area
				//	///////////////////////////////////////////////////////////////
				//	std::vector<PIAL::MaskInfo*> maskWList;
				//	for (auto & iter : InspDataSet->GetInspDataSet())
				//	{
				//		std::vector<PIAL::MaskInfo*> vMarskinFo = iter->GetMaskData();
				//		maskWList.insert(maskWList.end(), vMarskinFo.begin(), vMarskinFo.end());
				//	}
				//	cv::Mat pPartMaskImg(m_pInspBoardInfo->partImgBuf.nImageSizeY, m_pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1);
				//	pPartMaskImg.setTo(0);

				//	cv::Mat pPartMaskRectImg(m_pInspBoardInfo->partImgBuf.nImageSizeY, m_pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, m_pInspBoardInfo->partImgBuf.imgTop_R);
				//	cv::cvtColor(pPartMaskRectImg, pPartMaskRectImg, cv::COLOR_GRAY2BGR);

				//	for (auto & iter : maskWList)
				//	{
				//		cv::Rect rectM;
				//		rectM.x = iter->StartPos.x;
				//		rectM.y = iter->StartPos.y;

				//		rectM.width = iter->Width;
				//		rectM.height = iter->Length;
				//		cv::rectangle(pPartMaskImg, rectM, cv::Scalar(255), -1);
				//		cv::rectangle(pPartMaskRectImg, rectM, cv::Scalar(0, 0, 255), 2);
				//	}
				//	//cv::imwrite("D:\\testimage\\_2Mask.bmp", pPartMaskImg);
				//	cv::Mat PImg(m_pInspBoardInfo->partImgBuf.nImageSizeY, m_pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, m_pInspBoardInfo->partImgBuf.imgTop_R);
				//	//cv::imwrite("D:\\testimage\\_1Img.bmp", PImg);
				//	cv::Mat pDst;
				//	cv::Mat pMerImg[3] = { cv::Mat::zeros(cv::Size(PImg.cols, PImg.rows), PImg.type()), PImg,  pPartMaskImg };
				//	cv::merge(pMerImg, 3, pDst);
				//	pPartMaskImg = pPartMaskImg + PImg;
				//	cv::imwrite("D:\\testimage\\InspWindowAlgoMaskImg.bmp", pPartMaskImg);
				//	cv::imwrite("D:\\testimage\\InspWindowAlgoMaskRectImg.bmp", pPartMaskRectImg);
				//	cv::imwrite("D:\\testimage\\InspWindowAlgoPartImg.bmp", PImg);
				//	///////////////////////////////////////////////////////////////
				//}


#if _DEBUG
				//shkim 2023.06.15
				//mask Area
				///////////////////////////////////////////////////////////////
				std::vector<PIAL::MaskInfo*> maskWList;
				for (auto & iter : InspDataSet->GetInspDataSet())
				{
					std::vector<PIAL::MaskInfo*> vMarskinFo = iter->GetMaskData();
					maskWList.insert(maskWList.end(), vMarskinFo.begin(), vMarskinFo.end());
				}
				cv::Mat pPartMaskImg(m_pInspBoardInfo->partImgBuf.nImageSizeY, m_pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1);
				pPartMaskImg.setTo(0);
				for (auto & iter : maskWList)
				{
					cv::Rect rectM;
					rectM.x = iter->StartPos.x;
					rectM.y = iter->StartPos.y;

					rectM.width = iter->Width;
					rectM.height = iter->Length;
					cv::rectangle(pPartMaskImg, rectM, cv::Scalar(255), -1);
				}
				//cv::imwrite("D:\\testimage\\_2Mask.bmp", pPartMaskImg);
				cv::Mat PImg(m_pInspBoardInfo->partImgBuf.nImageSizeY, m_pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, m_pInspBoardInfo->partImgBuf.imgTop_R);
				//cv::imwrite("D:\\testimage\\_1Img.bmp", PImg);
				cv::Mat pDst;
				cv::Mat pMerImg[3] = { cv::Mat::zeros(cv::Size(PImg.cols, PImg.rows), PImg.type()), PImg,  pPartMaskImg };
				cv::merge(pMerImg, 3, pDst);
				pPartMaskImg = pPartMaskImg + PImg;
				cv::imwrite("D:\\testimage\\_3Dst.bmp", pPartMaskImg);
				///////////////////////////////////////////////////////////////
#endif
				// AI Part Image
				//SetAIData(inspType, sInspAlgo, bAlgoOK, sLightBuf, ImgR, ImgG, ImgB, ImgW, width, height, wnd_w, wnd_h, dX, dY);

				nLine = __LINE__;
				if (bAlgoAlign)
				{
					bool bStdAngle = true;
					if (eInspAlgoType == eAlgoAlignEdge && bUSeLeadAlign == FALSE)
						bStdAngle = false;

					if (arrAlignRes != NULL && arrAlignRes != nullptr && m_dicInspAlgorithm[eInspAlgoType]->SetAlignRes(sInspAlgo, inspType, sInspAlgo.m_ptrInspAlgoParam, vRstInspAlgoTemp, &arrAlignRes[nAlignWndIdx], bStdAngle) == TRUE)
					{
						nLine = __LINE__;
						bool bCurAlignID = (bUSeLeadAlign == FALSE && eInspAlgoType == eAlgoAlign);
						if (bCurAlignID == true)
						{
							if (arrAlignRes[nAlignWndIdx].nWindowID != -1)
							{
								int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
								for (int a = 0; a < nSize; a++)
								{
									if (ptrAlignRes[a].nWindowID == -1);
									{
										nAlignWndIdx = a;
										break;
									}
								}
				}
			}
						nLine = __LINE__;
						arrAlignRes[nAlignWndIdx].nAlignWndID = m_pParamArray[nWndIndex].nAlignWndID;
						arrAlignRes[nAlignWndIdx].nWindowID = m_pParamArray[nWndIndex].wndIndex;
						arrAlignRes[nAlignWndIdx].centerX = m_pParamArray[nWndIndex].cx;
						arrAlignRes[nAlignWndIdx].centerY = m_pParamArray[nWndIndex].cy;
						arrAlignRes[nAlignWndIdx].rcBodyRect = stAlgoParam.m_rcBlobBody;

						if (bCurAlignID == true)
							ptrAlignRes[nCurAlignID] = arrAlignRes[nAlignWndIdx];
						bAlignResOK = TRUE;
						nLine = __LINE__;
		}
	}
}
			else
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp = FALSE;

			nLine = __LINE__;
			if (bUSeLeadAlign == TRUE && (eInspAlgoType == eAlgoAlign || eInspAlgoType == eAlgoAlignEdge))
			{
				if (vRstInspAlgoTemp)
					g_pMManager->pem_delete(vRstInspAlgoTemp, false);
			}
			nLine = __LINE__;
			if (stAlgoParam.m_rcBlobBody)
				g_pMManager->pem_delete(stAlgoParam.m_rcBlobBody, false);

			nLine = __LINE__;
			if (sInspImgBuf)
			{
				if (sInspImgBuf->imgTop_R != NULL && sInspImgBuf->imgTop_R != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_R);
				if (sInspImgBuf->imgTop_G != NULL && sInspImgBuf->imgTop_G != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_G);
				if (sInspImgBuf->imgTop_B != NULL && sInspImgBuf->imgTop_B != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_B);
				if (sInspImgBuf->imgTop_W != NULL && sInspImgBuf->imgTop_W != nullptr)
					Delete_1DArray(&sInspImgBuf->imgTop_W);
				if (sInspImgBuf->imgMiddle_R != NULL && sInspImgBuf->imgMiddle_R != nullptr)
					Delete_1DArray(&sInspImgBuf->imgMiddle_R);
				if (sInspImgBuf->imgMiddle_B != NULL && sInspImgBuf->imgMiddle_B != nullptr)
					Delete_1DArray(&sInspImgBuf->imgMiddle_B);
				if (sInspImgBuf->imgBottom_R != NULL && sInspImgBuf->imgBottom_R != nullptr)
					Delete_1DArray(&sInspImgBuf->imgBottom_R);
				if (sInspImgBuf->imgBottom_B != NULL && sInspImgBuf->imgBottom_B != nullptr)
					Delete_1DArray(&sInspImgBuf->imgBottom_B);
				if (sInspImgBuf->imgAI != NULL && sInspImgBuf->imgAI != nullptr)
					Delete_1DArray(&sInspImgBuf->imgAI);

				// MemoryLeak Fix -wjlee
				//Delete_1DArray(&sInspImgBuf);
				g_pMManager->pem_delete(sInspImgBuf, false);
			}
			if (sArrInspImgBuf)
			{
				for (int a = 0; a < ALIGNEDGE_AREA_CNTS; a++)
				{
					if (sArrInspImgBuf[a].imgTop_R != NULL && sArrInspImgBuf[a].imgTop_R != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_R);
					if (sArrInspImgBuf[a].imgTop_G != NULL && sArrInspImgBuf[a].imgTop_G != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_G);
					if (sArrInspImgBuf[a].imgTop_B != NULL && sArrInspImgBuf[a].imgTop_B != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_B);
					if (sArrInspImgBuf[a].imgTop_W != NULL && sArrInspImgBuf[a].imgTop_W != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgTop_W);
					if (sArrInspImgBuf[a].imgMiddle_R != NULL && sArrInspImgBuf[a].imgMiddle_R != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgMiddle_R);
					if (sArrInspImgBuf[a].imgMiddle_B != NULL && sArrInspImgBuf[a].imgMiddle_B != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgMiddle_B);
					if (sArrInspImgBuf[a].imgBottom_R != NULL && sArrInspImgBuf[a].imgBottom_R != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgBottom_R);
					if (sArrInspImgBuf[a].imgBottom_B != NULL && sArrInspImgBuf[a].imgBottom_B != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgBottom_B);
					if (sArrInspImgBuf[a].imgAI != NULL && sArrInspImgBuf[a].imgAI != nullptr)
						Delete_1DArray(&sArrInspImgBuf[a].imgAI);
				}
				Delete_1DArray(&sArrInspImgBuf);
			}
			if (WndResult->m_vArrRstInspAlgo)
			{
				if (bUSeLeadAlign == FALSE)
					WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = bAlgoOK;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsRequired = sInspAlgo.m_bIsRequired;
			}

			bool bCheckNGEss = eInspAlgoType != eAlgoLead_Tip;
			if (eInspAlgoType == eAlgoLead_Tip)
			{
				AlgoLeadTip *pInspAlgoLeadTip = (AlgoLeadTip *)sInspAlgo.m_ptrInspAlgoParam;
				if (pInspAlgoLeadTip)
					bCheckNGEss = pInspAlgoLeadTip->bTipFaultNG;
			}

			if (bCheckNGEss)	//  Lead Tip 의 경우 위에서 체크함
			{
				if (sInspAlgo.m_bIsRequired && !sInspAlgo.m_bAlgoGroup && !bAlgoOK)	// 필수 검사 알고리즘이  NG이면 Window  결과  NG (단, Group이 체크되어있으면 일단 보류, 아래에서 판정)
				{
					ret = e_NG;
				}
				if (m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo].m_eAlgoType == eAlgoOCR && USENGOCR)
				{
					if (OCROKScore > OCRNGScore)
						ret = e_OK;
				}
			}

			// 교체용 디펙트 코드
			if (!bAlgoOK && sInspAlgo.m_bUsingManualDefectCode && bUSeLeadAlign == FALSE)
			{
				WndResult->m_vArrRstInspAlgo[nAlgo].m_bUsingManualDefectCode = sInspAlgo.m_bUsingManualDefectCode;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualDefectCode = sInspAlgo.m_nManualDefectCode;
				WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualSubDefectCode = sInspAlgo.m_nManualSubDefectCode;
			}
			// 			if (eAlgoNum > (int)eInspAlgoType)
			// 				m_fInspAlgoTact[(int)eInspAlgoType] += ((GetTickCount() - start) / 1000.0f);
			if (eAlgoNum > (int)eInspAlgoType)
			{
				m_dAlgoListTack[(int)eInspAlgoType] += ((GetTickCount() - start) / 1000.0f);
				m_nAlgoCnt[(int)eInspAlgoType]++;
			}

			if (pImg_buf)
				g_pMManager->pem_delete(pImg_buf, false);
			pImg_buf = nullptr;

		}

		nLine = __LINE__;
		Delete_1DArray(&ptrnArrLeadTipRst);
		Delete_1DArray(&ptrAlignRes);
		BOOL bIsInsp = TRUE;
		if (ret == e_OK && bUSeLeadAlign == FALSE)
		{

			nLine = __LINE__;

			int nResGroup = e_NG;		// Group이 아닌 알고리즘들의 결과
			int nResNotGroup = e_NG;		// Group인 알고리즘들의 결과
			BOOL bIsEssentialInGroup = FALSE;	// Group에 포함된 알고리즘 중 Essential check된 것이 있는지 여부
			bIsInsp = FALSE;
			if (m_pParamArray[nWndIndex].nAlgorithmCnt > 0)
				ret = e_NG;		// 일단 NG로,
			for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
			{
				if (!WndResult->m_vArrRstInspAlgo || WndResult->m_nAlgorithmCnt <= nAlgo)
					continue;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Search || WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Tip
					|| WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoTab_Search)
				{
					if (m_pParamArray[nWndIndex].nAlgorithmCnt == 1)
						ret = e_OK;	// LeadSearch
					continue;
				}
				// 알고리즘 간 기본 OR 처리 (여기 들어왔다는 것은 필수 검사는 모두 OK 인 경우이므로 고려하지 않아도 됨)
				// 2015/08/10 : 알고리즘 Group flag 추가되어서 필수 검사 NG 여부도 여기서 체크해야함

				nLine = __LINE__;

				InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
				if (sInspAlgo.m_bAlgoGroup && sInspAlgo.m_bIsRequired)
					bIsEssentialInGroup = TRUE;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk == TRUE)
				{
					if (!sInspAlgo.m_bAlgoGroup)
						nResNotGroup = e_OK;		// Group Enable = false인 알고리즘 결과가 하나라도 OK라면 OK
					else
						nResGroup = e_OK;		// Group Enable = true인 알고리즘 결과가 하나라도 OK라면 OK
				}

				bIsInsp = TRUE;
			}

			nLine = __LINE__;

			if (nResGroup == e_NG && bIsEssentialInGroup)		// Group의 결과가 NG이고 Group에 포함된 알고리즘 중 Essential check가 있으면 무조건 NG
				ret = e_NG;
			else if (nResNotGroup == e_OK || nResGroup == e_OK)	// 그렇지 않고, Group이 아닌 알고리즘 중 OK가 있거나, Group인 알고리즘 중 OK가 있으면 OK
				ret = e_OK;
		}
		if (bUSeLeadAlign == FALSE)
		{
			WndResult->m_bOk = ret;
			WndResult->m_bIsInsp = bIsInsp;
		}
		nLine = __LINE__;

		if (wndImg.m_ucArr2D)
			Delete_1DArray(&wndImg.m_ucArr2D);

		if (wndImg.m_ucArrCV)
			Delete_1DArray(&wndImg.m_ucArrCV);

		//LJH
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		{
			if (wndImg.m_ucArr2D_Mix[iLoopCount] != NULL)
			{
				//delete wndImg.m_ucArr2D_Mix[iLoopCount];
				//wndImg.m_ucArr2D_Mix[iLoopCount] = NULL;
				Delete_1DArray(&wndImg.m_ucArr2D_Mix[iLoopCount]);
			}
		}
		if (wndImg.m_fArr3D)
			Delete_1DArray(&wndImg.m_fArr3D);

		nLine = __LINE__;

		if (wndImg.m_fArr3D)
			Delete_1DArray(&wndImg.m_fArr3D);

		Delete_1DArray(&ptrAlignRes);

		//Initialize TabArea Img
		if (m_pucTabAreaTmp)
			g_pMManager->pem_delete(m_pucTabAreaTmp, true);
		m_pucTabAreaTmp = nullptr;
		m_nTabAlgoIdx = -1;
	}
	catch (...)
	{
		if (pImg_buf)
		{
			g_pMManager->pem_delete(pImg_buf, false);
			pImg_buf = nullptr;
		}

		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CMInspManager::InspWindowAlgo(), Line : %d Pass!!! WndType : %d, WinID : %d, AlgoType : %d, AlgoID : %d"), nLine, wndType, nWndIndex, nAlgoType, nAlgoID);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}

		//Initialize TabArea Img
		if (m_pucTabAreaTmp)
			g_pMManager->pem_delete(m_pucTabAreaTmp, true);
		m_pucTabAreaTmp = nullptr;
		m_nTabAlgoIdx = -1;
	}
	return ret;
}
// 이미지: AI 세그멘트 이미지에서 윈도우 ROI를 잘라 pImg_buf->m_pAI 버퍼로 복사
// 입력: pImg_buf (대상 버퍼와 ROI 좌표 포함), wnd_w/wnd_h (ROI 크기), partImgAI (원본 AI 이미지 버퍼), aiImgSizeX/Y (원본 크기)
// 반환: true(성공) / false(실패)
bool InspManager::CopyAICroppedToWnd2D(PIAL::Insp_Image* pImg_buf, int wnd_w, int wnd_h, const UCHAR* partImgAI, int aiImgSizeX, int aiImgSizeY)
{
	if (!pImg_buf || !partImgAI || wnd_w <= 0 || wnd_h <= 0)
		return false;
	// 기존 메모리 해제 (메모리 릭 방지)
	if (pImg_buf->m_pAI)
	{
		delete pImg_buf->m_pAI;
		pImg_buf->m_pAI = nullptr;
	}
	pImg_buf->m_pAI = new PIAL::PI_Buff(wnd_w, wnd_h);

	cv::Mat aiSegImg(aiImgSizeY, aiImgSizeX, CV_8UC1, const_cast<UCHAR*>(partImgAI));

	int startX = pImg_buf->inspWndImage->nStartX;
	int startY = pImg_buf->inspWndImage->nStartY;
	if (startX < 0 || startY < 0 || startX + wnd_w > aiSegImg.cols || startY + wnd_h > aiSegImg.rows)
	{
		delete pImg_buf->m_pAI;
		pImg_buf->m_pAI = nullptr;
		return false;
	}


	cv::Rect roi(startX, startY, wnd_w, wnd_h);
	cv::Mat cropped = aiSegImg(roi).clone();
	cv::threshold(cropped, cropped, g_pMPTI->m_SegThreshold, 255, cv::THRESH_BINARY); // 이후 150(threshold값)을 machine options에서 설정 가능하도록 변경 예정

	size_t destStride = (size_t)pImg_buf->m_pAI->Width();
	size_t srcStride = (size_t)cropped.step[0];
	size_t rowCopySize = (size_t)cropped.cols * cropped.elemSize();
	size_t destCapacity = (size_t)pImg_buf->m_pAI->Width() * (size_t)pImg_buf->m_pAI->Length();
	size_t needSize = (size_t)cropped.rows * rowCopySize;
	if (needSize > destCapacity)
	{
		delete pImg_buf->m_pAI;
		pImg_buf->m_pAI = nullptr;
		return false;
	}
	UCHAR* dstBase = pImg_buf->m_pAI->m_pData;
	UCHAR* srcBase = cropped.data;

	for (int y = 0; y < cropped.rows; ++y)
	{
		UCHAR* dst = dstBase + y * destStride;
		UCHAR* src = srcBase + y * srcStride;
		memcpy(dst, src, rowCopySize);
	}

	return true;
}
int InspManager::GetWindowID(int nWinIdx)
{
	return m_pParamArray[nWinIdx].wndIndex;
}
void InspManager::GetPartImageSize(int& nWidth, int& nHeight)
{
	nWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	nHeight = m_pInspBoardInfo->partImgBuf.nImageSizeY;
}
bool InspManager::LicenseCheck(MIL_INT lic)
{
	if (g_pMPTI->isUseImagePilLib())
		return false;
	MIL_INT nLisenceModules = MsysInquire(M_DEFAULT_HOST, M_LICENSE_MODULES, M_NULL);

	if ((nLisenceModules & lic))
		return true;
	else
		return false;
}
int InspManager::ForeignPatternSet(const InspAlgo &sInspAlgo, UCHAR* ucArrBlobDst, int nImgWidth, int nImgHeight, double dWndAngle, double dAngle, POINTF* pFPMatchPos)
{
	int ret = 0;
	m_pForeignPattern->SetInspAlgo(sInspAlgo, ucArrBlobDst, nImgWidth, nImgHeight, 0, dWndAngle); // KIY 2019/10/11 : Body표면위의 임의위치 이물검사 적용 건
	ret = m_pForeignPattern->InspForeign(false);
	m_pForeignPattern->GetForeignPatternMatchPos(pFPMatchPos);
	return ret;
}
void InspManager::ForeingLineImgSet(UCHAR* LineImg, int width, int height, cv::Point pCenter)
{
	cv::Mat LineMask(height, width, CV_8UC1, LineImg, width);
	m_pForeignPattern->SetLineImg(LineMask, pCenter);
}
int InspManager::GetInspectionResultRef(InspectionResult* &retResult)
{
	if (m_inspectionResult == NULL /*|| m_isSingleInsp == TRUE*/)
		return eIMSG_FAIL;

	retResult = m_inspectionResult;

	return eIMSG_SUCCESE;
}
void InspManager::InvertDisplay(UCHAR* ucDst, int nwidth, int nheight, int nStartROIX, int nStartROIY, int nROIWidth, int nROIHeight, POINTF* pForeignPos)
{
	if (ucDst == NULL)
		return;

	cv::Mat src(nheight, nwidth, CV_8UC1, ucDst);

	cv::Mat mMask(src.size(), CV_8UC1, cv::Scalar(0));
	std::vector<cv::Point> vShapePtr;

	for (int i = 0; i < 4; i++)
	{
		float tmpX = pForeignPos[i].x + nStartROIX;
		float tmpY = pForeignPos[i].y + nStartROIY;
		vShapePtr.emplace_back(cv::Point2f(tmpX, tmpY));
	}

	cv::fillConvexPoly(mMask, vShapePtr, cv::Scalar(255, 255, 255));

	cv::Mat tmp = src ^ mMask;
	src.setTo(0);
	tmp.copyTo(src);
}
void InspManager::ReleaseImgSave(CString FullPath, bool OKNg, WndAlgoImg* wndImg, bool bhist)
{
	int split = 0;
	CString imgpath_tmp = FullPath.Tokenize(_T("@"), split);
	CString tmp1 = FullPath.Tokenize(_T("@"), split);
	CString tmp2 = FullPath.Tokenize(_T("@"), split);
	CString Filename;
	CTime t = CTime::GetCurrentTime();
	Filename.Format(_T("%s@%s_%s.bmp"), tmp1, tmp2, t.Format(_T("%y.%m.%d_%Hh.%Mm.%Ss")));
	if (bhist != false)Filename.Format(_T("%s@%s_%s_1.bmp"), tmp1, tmp2, t.Format(_T("%y.%m.%d_%Hh.%Mm.%Ss")));
	split = 0;
	int max = imgpath_tmp.GetLength();
	while (split < max)
	{
		tmp1 = imgpath_tmp.Tokenize(_T("\\"), split);
		if (split == -1)break;
	}
	imgpath_tmp.Format(_T("D:\\Eagle3D_data\\InspectResult\\OCRSaveImg\\%s"), tmp1);

	CString imgpath;
	if (OKNg == 0)
		imgpath.Format(_T("%s_NG"), imgpath_tmp);
	if (OKNg == 1)
		imgpath.Format(_T("%s_OK"), imgpath_tmp);
	m_procMil->SaveReleaseWorkImg(wndImg->m_ucArr2D, wndImg->m_nWidth, wndImg->m_nHeight, Filename, imgpath);
}
void InspManager::TabNGRectChangePosition(InspWndResult* WndResult, int nAlgo, double dX, double dY)
{
	RstAlgoTab * sRstAlgo = (RstAlgoTab *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
	for (int n = 0; n < sRstAlgo->m_nCntRect; n++)
	{
		if (sRstAlgo->m_bTabRst[n] == false)
		{
			sRstAlgo->m_pofNGRectCenter[n].x += dX;
			sRstAlgo->m_pofNGRectCenter[n].y += dY;
		}
	}
	for (int n = 0; n < sRstAlgo->m_nNGAreaRoiCnt; n++)
	{
		sRstAlgo->m_rcArrNGArrRect[n].left += dX;
		sRstAlgo->m_rcArrNGArrRect[n].right += dX;
		sRstAlgo->m_rcArrNGArrRect[n].top += dY;
		sRstAlgo->m_rcArrNGArrRect[n].bottom += dY;
	}
}
void InspManager::TabNGRectChangePosition(RstAlgoTab * sRstAlgo, int nAlgo, double dX, double dY)
{
	//RstAlgoTab * sRstAlgo = (RstAlgoTab *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
	for (int n = 0; n < sRstAlgo->m_nCntRect; n++)
	{
		if (sRstAlgo->m_bTabRst[n] == false)
		{
			sRstAlgo->m_pofNGRectCenter[n].x += dX;
			sRstAlgo->m_pofNGRectCenter[n].y += dY;
		}
		sRstAlgo->m_rcArrCalcRect[n].left += dX;
		sRstAlgo->m_rcArrCalcRect[n].right += dX;
		sRstAlgo->m_rcArrCalcRect[n].top += dY;
		sRstAlgo->m_rcArrCalcRect[n].bottom += dY;
	}
	for (int n = 0; n < sRstAlgo->m_nNGAreaRoiCnt; n++)
	{
		sRstAlgo->m_rcArrNGArrRect[n].left += dX;
		sRstAlgo->m_rcArrNGArrRect[n].right += dX;
		sRstAlgo->m_rcArrNGArrRect[n].top += dY;
		sRstAlgo->m_rcArrNGArrRect[n].bottom += dY;
	}
}
void InspManager::TabNGRectChangePosition(RstAlgoShapeX * sRstAlgo, double dX, double dY)
{
	//RstAlgoTab * sRstAlgo = (RstAlgoTab *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
	for (int n = 0; n < sRstAlgo->nROICnt; n++)
	{
		sRstAlgo->rcArrRect[n].left += dX;
		sRstAlgo->rcArrRect[n].right += dX;
		sRstAlgo->rcArrRect[n].top += dY;
		sRstAlgo->rcArrRect[n].bottom += dY;
	}
	for (int n = 0; n < sRstAlgo->nNGAreaRoiCnt; n++)
	{
		sRstAlgo->rcArrNGRect[n].left += dX;
		sRstAlgo->rcArrNGRect[n].right += dX;
		sRstAlgo->rcArrNGRect[n].top += dY;
		sRstAlgo->rcArrNGRect[n].bottom += dY;
	}
	for (int n = 0; n < sRstAlgo->nShapeNGCnt; n++)
	{
		sRstAlgo->rcArrShapeRect[n].left += dX;
		sRstAlgo->rcArrShapeRect[n].right += dX;
		sRstAlgo->rcArrShapeRect[n].top += dY;
		sRstAlgo->rcArrShapeRect[n].bottom += dY;
	}
	for (int n = 0; n < sRstAlgo->StripeRectCnt; n++)
	{
		sRstAlgo->rcArrStripeRect[n].left += dX;
		sRstAlgo->rcArrStripeRect[n].right += dX;
		sRstAlgo->rcArrStripeRect[n].top += dY;
		sRstAlgo->rcArrStripeRect[n].bottom += dY;
	}
}
bool InspManager::PtInPolygon(POINTF pt, POINTF *pts, int ptNum)
{
	bool oddNodes = false;		// false: even(out), true: odd(in)
	for (int i = 0; i < ptNum; i++)
	{
		POINTF spt = pts[i];
		POINTF ept = pts[(i + 1) % ptNum];		// % prevent over index
		if (((spt.y <= pt.y) && (ept.y > pt.y)) ||	// an upward crossing
			((spt.y > pt.y) && (ept.y <= pt.y)))	// a downward crossing
		{
			oddNodes ^= (pt.x < (spt.x + ((pt.y - spt.y) / (ept.y - spt.y) * (ept.x - spt.x)))); // pt.x is in left of edge
		}
	}
	return oddNodes;
}
int InspManager::Inspection_Foreign_new(ForeignData sInspData, FR2DData sFR2DData, ForeignParamROI* arrROI)
{
	return g_pInspMng->m_FR.Inspection_Foreign_new(sInspData, sFR2DData, arrROI);;
}
int InspManager::GetInspForeignResult(InspFovForeignResult* retResult)
{
	if (m_inspForeignResult == NULL/* || m_isSingleInsp == TRUE*/)
		return eIMSG_FAIL;

	*retResult = *m_inspForeignResult;

	return eIMSG_SUCCESE;
}
void InspManager::CalcForeignResult(InspFovForeignResult *arrTempRst, InspFovForeignResult* ForeignResult)
{
	int nLine = __LINE__;
	try
	{
		ForeignResult->m_bOK = arrTempRst->m_bOK;
		if (ForeignResult->m_nCountDefect < 0)
			ForeignResult->m_nCountDefect = 0;
		if (arrTempRst->m_nCountDefect <= 0)
			arrTempRst->m_nCountDefect = 0;
		else if (arrTempRst->m_nCountDefect > 0)
		{
			int nStart = 0;
			if (ForeignResult->m_nCountDefect > 0)
			{
				nStart = ForeignResult->m_nCountDefect;
				AForeignResult *arr = g_pMManager->pem_new<AForeignResult>(true, nStart, (PCHAR)__FUNCTION__, __LINE__);
				memcpy(arr, ForeignResult->m_stForeign, sizeof(AForeignResult) * nStart);

				if (ForeignResult->m_stForeign)
				{
					g_pMManager->pem_delete(ForeignResult->m_stForeign, true);
					ForeignResult->m_stForeign = NULL;
				}

				ForeignResult->m_nCountDefect += arrTempRst->m_nCountDefect;
				ForeignResult->m_stForeign = g_pMManager->pem_new<AForeignResult>(true, ForeignResult->m_nCountDefect, (PCHAR)__FUNCTION__, __LINE__);
				for (int i = 0; i < nStart; i++)
				{
					arr[i].Clone(ForeignResult->m_stForeign[i]);
				}
				Delete_1DArray(&arr);
			}
			else
			{
				ForeignResult->m_nCountDefect += arrTempRst->m_nCountDefect;
				if (ForeignResult->m_stForeign)
				{
					g_pMManager->pem_delete(ForeignResult->m_stForeign, true);
					ForeignResult->m_stForeign = NULL;
				}
				ForeignResult->m_stForeign = g_pMManager->pem_new<AForeignResult>(true, ForeignResult->m_nCountDefect, (PCHAR)__FUNCTION__, __LINE__);
			}

			for (int i = nStart; i < nStart + arrTempRst->m_nCountDefect; i++)
			{
				arrTempRst->m_stForeign[i - nStart].Clone(ForeignResult->m_stForeign[i]);
			}
		}
		nLine = __LINE__;

		if (ForeignResult->m_nCountDefectWP < 0)
			ForeignResult->m_nCountDefectWP = 0;
		if (arrTempRst->m_nCountDefectWP <= 0)
			arrTempRst->m_nCountDefectWP = 0;
		else if (arrTempRst->m_nCountDefectWP > 0)
		{
			for (int i = 0; i < arrTempRst->m_nCountDefectWP; i++)
			{
				bool bInput = false;
				for (int j = 0; j < ForeignResult->m_nCountDefectWP; j++)
				{
					if (ForeignResult->m_stForeignWP[j].nID != arrTempRst->m_stForeignWP[i].nID)
						continue;
					if (ForeignResult->m_stForeignWP[j].m_byWP == 1 && arrTempRst->m_stForeignWP[i].m_byWP == 1)
					{
						if (ForeignResult->m_stForeignWP[j].m_fHeightMin > arrTempRst->m_stForeignWP[i].m_fHeightMin)
						{
							ForeignResult->m_stForeignWP[j].m_fHeightMin = arrTempRst->m_stForeignWP[i].m_fHeightMin;
							ForeignResult->m_stForeignWP[j].m_fCXMin = arrTempRst->m_stForeignWP[i].m_fCXMin;
							ForeignResult->m_stForeignWP[j].m_fCYMin = arrTempRst->m_stForeignWP[i].m_fCYMin;
						}
						if (ForeignResult->m_stForeignWP[j].m_fHeightMax < arrTempRst->m_stForeignWP[i].m_fHeightMax)
						{
							ForeignResult->m_stForeignWP[j].m_fHeightMax = arrTempRst->m_stForeignWP[i].m_fHeightMax;
							ForeignResult->m_stForeignWP[j].m_fCXMin = arrTempRst->m_stForeignWP[i].m_fCXMax;
							ForeignResult->m_stForeignWP[j].m_fCYMin = arrTempRst->m_stForeignWP[i].m_fCYMax;
						}
					}
					else if (arrTempRst->m_stForeignWP[i].m_byWP == 1)
					{
						ForeignResult->m_stForeignWP[j].m_fHeightMin = arrTempRst->m_stForeignWP[i].m_fHeightMin;
						ForeignResult->m_stForeignWP[j].m_fCXMin = arrTempRst->m_stForeignWP[i].m_fCXMin;
						ForeignResult->m_stForeignWP[j].m_fCYMin = arrTempRst->m_stForeignWP[i].m_fCYMin;
						ForeignResult->m_stForeignWP[j].m_fHeightMax = arrTempRst->m_stForeignWP[i].m_fHeightMax;
						ForeignResult->m_stForeignWP[j].m_fCXMin = arrTempRst->m_stForeignWP[i].m_fCXMax;
						ForeignResult->m_stForeignWP[j].m_fCYMin = arrTempRst->m_stForeignWP[i].m_fCYMax;
					}
					if (ForeignResult->m_stForeignWP[j].m_byAM == 1 && arrTempRst->m_stForeignWP[i].m_byAM == 1)
					{
						if (ForeignResult->m_stForeignWP[j].m_fHeightMinAM > arrTempRst->m_stForeignWP[i].m_fHeightMinAM)
						{
							ForeignResult->m_stForeignWP[j].m_fHeightMinAM = arrTempRst->m_stForeignWP[i].m_fHeightMinAM;
							ForeignResult->m_stForeignWP[j].m_fCXMinAM = arrTempRst->m_stForeignWP[i].m_fCXMinAM;
							ForeignResult->m_stForeignWP[j].m_fCYMinAM = arrTempRst->m_stForeignWP[i].m_fCYMinAM;
						}
						if (ForeignResult->m_stForeignWP[j].m_fHeightMaxAM < arrTempRst->m_stForeignWP[i].m_fHeightMaxAM)
						{
							ForeignResult->m_stForeignWP[j].m_fHeightMaxAM = arrTempRst->m_stForeignWP[i].m_fHeightMaxAM;
							ForeignResult->m_stForeignWP[j].m_fCXMaxAM = arrTempRst->m_stForeignWP[i].m_fCXMaxAM;
							ForeignResult->m_stForeignWP[j].m_fCYMaxAM = arrTempRst->m_stForeignWP[i].m_fCYMaxAM;
						}
					}
					else if (arrTempRst->m_stForeignWP[i].m_byAM == 1)
					{
						ForeignResult->m_stForeignWP[j].m_fHeightMinAM = arrTempRst->m_stForeignWP[i].m_fHeightMinAM;
						ForeignResult->m_stForeignWP[j].m_fCXMinAM = arrTempRst->m_stForeignWP[i].m_fCXMinAM;
						ForeignResult->m_stForeignWP[j].m_fCYMinAM = arrTempRst->m_stForeignWP[i].m_fCYMinAM;
						ForeignResult->m_stForeignWP[j].m_fHeightMaxAM = arrTempRst->m_stForeignWP[i].m_fHeightMaxAM;
						ForeignResult->m_stForeignWP[j].m_fCXMaxAM = arrTempRst->m_stForeignWP[i].m_fCXMaxAM;
						ForeignResult->m_stForeignWP[j].m_fCYMaxAM = arrTempRst->m_stForeignWP[i].m_fCYMaxAM;
					}
					bInput = true;
				}
				nLine = __LINE__;
				if (bInput == false)
				{
					int nStart = ForeignResult->m_nCountDefectWP;
					if (nStart > 0)
					{
						AForeignResultWP *arr = g_pMManager->pem_new<AForeignResultWP>(true, ForeignResult->m_nCountDefectWP, (PCHAR)__FUNCTION__, __LINE__);
						memcpy(arr, ForeignResult->m_stForeignWP, sizeof(AForeignResultWP) * ForeignResult->m_nCountDefectWP);
						if (ForeignResult->m_stForeignWP)
						{
							g_pMManager->pem_delete(ForeignResult->m_stForeignWP, true);
							ForeignResult->m_stForeignWP = NULL;
						}

						ForeignResult->m_nCountDefectWP++;
						ForeignResult->m_stForeignWP = g_pMManager->pem_new<AForeignResultWP>(true, ForeignResult->m_nCountDefectWP, (PCHAR)__FUNCTION__, __LINE__);
						for (int j = 0; j < nStart; j++)
						{
							ForeignResult->m_stForeignWP[j].nID = arr[j].nID;
							ForeignResult->m_stForeignWP[j].m_byWP = arr[j].m_byWP;
							ForeignResult->m_stForeignWP[j].m_fHeightMin = arr[j].m_fHeightMin;
							ForeignResult->m_stForeignWP[j].m_fHeightMax = arr[j].m_fHeightMax;
							ForeignResult->m_stForeignWP[j].m_fCXMin = arr[j].m_fCXMin;
							ForeignResult->m_stForeignWP[j].m_fCYMin = arr[j].m_fCYMin;
							ForeignResult->m_stForeignWP[j].m_fCXMax = arr[j].m_fCXMax;
							ForeignResult->m_stForeignWP[j].m_fCYMax = arr[j].m_fCYMax;

							ForeignResult->m_stForeignWP[j].m_byAM = arr[j].m_byAM;
							ForeignResult->m_stForeignWP[j].m_fHeightMinAM = arr[j].m_fHeightMinAM;
							ForeignResult->m_stForeignWP[j].m_fHeightMaxAM = arr[j].m_fHeightMaxAM;
							ForeignResult->m_stForeignWP[j].m_fCXMinAM = arr[j].m_fCXMinAM;
							ForeignResult->m_stForeignWP[j].m_fCYMinAM = arr[j].m_fCYMinAM;
							ForeignResult->m_stForeignWP[j].m_fCXMaxAM = arr[j].m_fCXMaxAM;
							ForeignResult->m_stForeignWP[j].m_fCYMaxAM = arr[j].m_fCYMaxAM;
						}
						Delete_1DArray(&arr);
					}
					else
					{
						ForeignResult->m_nCountDefectWP++;
						if (ForeignResult->m_stForeignWP)
						{
							g_pMManager->pem_delete(ForeignResult->m_stForeignWP, true);
							ForeignResult->m_stForeignWP = NULL;
						}
						ForeignResult->m_stForeignWP = g_pMManager->pem_new<AForeignResultWP>(true, ForeignResult->m_nCountDefectWP, (PCHAR)__FUNCTION__, __LINE__);
					}
					nLine = __LINE__;
					ForeignResult->m_stForeignWP[nStart].nID = arrTempRst->m_stForeignWP[i].nID;
					ForeignResult->m_stForeignWP[nStart].m_byWP = arrTempRst->m_stForeignWP[i].m_byWP;
					ForeignResult->m_stForeignWP[nStart].m_fHeightMin = arrTempRst->m_stForeignWP[i].m_fHeightMin;
					ForeignResult->m_stForeignWP[nStart].m_fHeightMax = arrTempRst->m_stForeignWP[i].m_fHeightMax;
					ForeignResult->m_stForeignWP[nStart].m_fCXMin = arrTempRst->m_stForeignWP[i].m_fCXMin;
					ForeignResult->m_stForeignWP[nStart].m_fCYMin = arrTempRst->m_stForeignWP[i].m_fCYMin;
					ForeignResult->m_stForeignWP[nStart].m_fCXMax = arrTempRst->m_stForeignWP[i].m_fCXMax;
					ForeignResult->m_stForeignWP[nStart].m_fCYMax = arrTempRst->m_stForeignWP[i].m_fCYMax;

					ForeignResult->m_stForeignWP[nStart].m_byAM = arrTempRst->m_stForeignWP[i].m_byAM;
					ForeignResult->m_stForeignWP[nStart].m_fHeightMinAM = arrTempRst->m_stForeignWP[i].m_fHeightMinAM;
					ForeignResult->m_stForeignWP[nStart].m_fHeightMaxAM = arrTempRst->m_stForeignWP[i].m_fHeightMaxAM;
					ForeignResult->m_stForeignWP[nStart].m_fCXMinAM = arrTempRst->m_stForeignWP[i].m_fCXMinAM;
					ForeignResult->m_stForeignWP[nStart].m_fCYMinAM = arrTempRst->m_stForeignWP[i].m_fCYMinAM;
					ForeignResult->m_stForeignWP[nStart].m_fCXMaxAM = arrTempRst->m_stForeignWP[i].m_fCXMaxAM;
					ForeignResult->m_stForeignWP[nStart].m_fCYMaxAM = arrTempRst->m_stForeignWP[i].m_fCYMaxAM;
				}
			}
		}
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CMInspManager::CalcForeignResult(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
}

byte InspManager::ExceptForeignImage(int nMode, cv::Mat imgExcept, ForeignData sData_ROI)
{
	int nW = imgExcept.cols;
	int nH = imgExcept.rows;
	byte byRet = 0;
	int nROITotal = m_ForeignData.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= 0 || nW <= 0 || nH <= 0)
		return 0;

	int nDivide = m_ForeignData.GetDivide();
	bool bExtraPer = m_ForeignData.UseData2(FR_DATA2_ExtraPixelE_Per);
	float fExtraPixelCnt = m_ForeignData.m_nArrData[FR_N_ExtraPixelETC2] / nDivide;
	if (bExtraPer)
		fExtraPixelCnt = m_ForeignData.m_fArrData[FR_F_ExtraPixelETC2_Per];
	CRect rtFOV;
	if (sData_ROI.m_vInspectionMode == eMI_Inspection)
	{
		int nCX = RounD(m_pInspBoardInfo_Foreign->fovCx / m_resolX / nDivide);
		int nCY = RounD(m_pInspBoardInfo_Foreign->fovCy / m_resolY / nDivide);
		rtFOV.left = nCX - (nW / 2);
		rtFOV.right = rtFOV.left + nW;
		rtFOV.top = nCY - (nH / 2);
		rtFOV.bottom = rtFOV.top + nH;
	}
	else
	{
		POINTF poCur = g_pMPTI->GetForeignCurFovCenter();
		int nCX = RounD(poCur.x / m_resolX / nDivide);
		int nCY = RounD(poCur.y / m_resolY / nDivide);
		rtFOV.left = nCX - (nW / 2);
		rtFOV.right = rtFOV.left + nW;
		rtFOV.top = nCY - (nH / 2);
		rtFOV.bottom = rtFOV.top + nH;
	}

	bool bInput = false;
	long long nTotal = nW * nH;
	cv::Mat imgZero(nH, nW, CV_8UC1, cv::Scalar(0));
	int nFrIndex = 0;
	std::vector<int> vecDelROI;
	for (int a = 0; a < nROITotal; a++)
	{
		int nType = m_ForeignParamROI[a].m_nType;
		int nROIModule = m_ForeignParamROI[a].m_nModuleID;
		if (nType == ForeignParamROI_Data_Foreign)
		{
			nFrIndex = a;
			break;
		}
		if (m_ForeignParamROI[a].m_bSKIP)
		{
			vecDelROI.push_back(a);
			continue;
		}
		if (nROIModule > 0)
		{
			bool bNotInsp = false;
			if (sData_ROI.m_nContainModuleID > 0)
				bNotInsp = (nROIModule != sData_ROI.m_nContainModuleID);

			if (bNotInsp == false && sData_ROI.m_pInspModule && sData_ROI.m_nModule > nROIModule)
				bNotInsp = (sData_ROI.m_pInspModule[nROIModule] != 1);

			if (bNotInsp)
			{
				vecDelROI.push_back(a);
				continue;
			}
		}

		if (nMode == m_eForeignInsp_Gray)
		{
			if (nType != ForeignParamROI_Data_Solder)
				continue;
		}
		else if (nMode == m_eForeignInsp_SkipPart)
		{
			if (nType != ForeignParamROI_Data_SkipPart)
				continue;
		}
		else
		{
			if (nType != ForeignParamROI_Data_InspA && nType != ForeignParamROI_Data_Module)
				continue;
		}

		if (SetExceptForeignImage(a, nType, rtFOV, imgZero.data, nW, nH, 255) == false)
			continue;

		bInput = true;
	}

	if (vecDelROI.size() > 0)
	{
		for (int nIndex : vecDelROI)
		{
			int nType = m_ForeignParamROI[nIndex].m_nType;
			SetExceptForeignImage(nIndex, nType, rtFOV, imgZero.data, nW, nH);
		}
	}

	int nPCBL = m_ForeignData.m_nArrData[FR_N_PCBL] / nDivide;
	int nPCBR = m_ForeignData.m_nArrData[FR_N_PCBR] / nDivide;
	int nPCBT = m_ForeignData.m_nArrData[FR_N_PCBT] / nDivide;
	int nPCBB = m_ForeignData.m_nArrData[FR_N_PCBB] / nDivide;

	double dBoardW = m_ForeignData.m_fArrData[FR_F_BoardW];
	double dBoardH = m_ForeignData.m_fArrData[FR_F_BoardH];
	int nBoardW_pix = RounD(dBoardW / m_resolX / nDivide);
	int nBoardH_pix = RounD(dBoardH / m_resolY / nDivide);
	if (rtFOV.left < nPCBL)
	{
		int nPCBW = nPCBL - rtFOV.left;
		if (nPCBW > 0 && nW >= nPCBW)
			imgZero(cv::Rect(0, 0, nPCBW, nH)).setTo(0);
	}
	if (rtFOV.right > nBoardW_pix - nPCBR)
	{
		int nPCBS = (nBoardW_pix - nPCBR) - rtFOV.left;
		int nPCBW = nW - nPCBS;
		if (nPCBS >= 0 && nPCBW > 0 && nW >= nPCBS + nPCBW)
			imgZero(cv::Rect(nPCBS, 0, nPCBW, nH)).setTo(0);
	}
	if (rtFOV.top < nPCBT)
	{
		int nPCBH = nPCBT - rtFOV.top;
		int nPCBS = nH - nPCBH;
		if (nPCBS >= 0 && nPCBH > 0 && nH >= nPCBS + nPCBH)
			imgZero(cv::Rect(0, nPCBS, nW, nPCBH)).setTo(0);
	}
	if (rtFOV.bottom > nBoardH_pix - nPCBB)
	{
		int nPCBH = rtFOV.bottom - (nBoardH_pix - nPCBB);
		if (nPCBH > 0 && nH >= nPCBH)
			imgZero(cv::Rect(0, 0, nW, nPCBH)).setTo(0);
	}

	if (bInput)
	{
		if (nMode == ForeignParamROI_Data_Bubble)
		{
			for (long long a = 0; a < nTotal; a++)
			{
				if (imgZero.data[a] == 0 || imgZero.data[a] == 200)
					imgExcept.data[a] = imgZero.data[a];
			}
		}
		else
			memcpy(imgExcept.data, imgZero.data, sizeof(UCHAR)* nTotal);
	}
	if (bInput == false)
	{
		memset(imgExcept.data, 0, sizeof(UCHAR) * nW * nH);
		return 0;
	}
	for (long long a = nFrIndex; a < nROITotal; a++)
	{
		if (nMode != m_eForeignInsp_GrayBub)
			break;

		int nType = m_ForeignParamROI[a].m_nType;
		if (nType != ForeignParamROI_Data_Bubble)
			continue;

		if (SetExceptForeignImage(a, nType, rtFOV, imgExcept.data, nW, nH, 0, bExtraPer, fExtraPixelCnt) == false)
			continue;

		long long nTotalCnt = std::count(imgExcept.data, imgExcept.data + nTotal, 0);
		if (nTotalCnt == nTotal) break;
	}
	if (bInput) byRet = 1;
	return byRet;
}
void InspManager::ExceptForeign(int nType, cv::Mat imgExcept, ForeignData sData_ROI)
{
	int nW = imgExcept.cols;
	int nH = imgExcept.rows;
	int nROITotal = m_ForeignData.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= 0 || nW <= 0 || nH <= 0)
		return;

	int nDivide = m_ForeignData.GetDivide();
	CRect rtFOV;
	if (sData_ROI.m_vInspectionMode == eMI_Inspection)
	{
		int nCX = RounD(m_pInspBoardInfo_Foreign->fovCx / m_resolX / nDivide);
		int nCY = RounD(m_pInspBoardInfo_Foreign->fovCy / m_resolY / nDivide);
		rtFOV.left = nCX - (nW / 2);
		rtFOV.right = rtFOV.left + nW;
		rtFOV.top = nCY - (nH / 2);
		rtFOV.bottom = rtFOV.top + nH;
	}
	else
	{
		POINTF poCur = g_pMPTI->GetForeignCurFovCenter();
		int nCX = RounD(poCur.x / m_resolX / nDivide);
		int nCY = RounD(poCur.y / m_resolY / nDivide);
		rtFOV.left = nCX - (nW / 2);
		rtFOV.right = rtFOV.left + nW;
		rtFOV.top = nCY - (nH / 2);
		rtFOV.bottom = rtFOV.top + nH;
	}

	for (int a = 0; a < nROITotal; a++)
	{
		if (m_ForeignParamROI[a].m_nType != nType)
			continue;

		if (m_ForeignParamROI[a].m_bSKIP)
			continue;

		int nROIModule = m_ForeignParamROI[a].m_nModuleID;
		if (nROIModule > 0)
		{
			bool bNotInsp = false;
			if (sData_ROI.m_nContainModuleID > 0)
				bNotInsp = (nROIModule != sData_ROI.m_nContainModuleID);

			if (bNotInsp == false && sData_ROI.m_pInspModule && sData_ROI.m_nModule > nROIModule)
				bNotInsp = (sData_ROI.m_pInspModule[nROIModule] != 1);

			if (bNotInsp)
				continue;
		}

		if (SetExceptForeignImage(a, nType, rtFOV, imgExcept.data, nW, nH, 255) == false)
			continue;
	}
}
bool InspManager::SetExceptForeignImage(int nIndex, int nType, CRect rtFOV, UCHAR* pucDst, int nW, int nH, byte byValue, bool bExtraPer, float fExtraPixelCnt)
{
	int nROITotal = m_ForeignData.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= nIndex)
		return false;

	int nDivide = m_ForeignData.GetDivide();
	int nROIX = RounD(m_ForeignParamROI[nIndex].m_pPos.x / m_resolX / nDivide);
	int nROIY = RounD(m_ForeignParamROI[nIndex].m_pPos.y / m_resolY / nDivide);
	int nROIW = RounD(m_ForeignParamROI[nIndex].m_pSize.x / m_resolX / nDivide);
	int nROIH = RounD(m_ForeignParamROI[nIndex].m_pSize.y / m_resolY / nDivide);
	CRect rtROI;
	rtROI.left = nROIX;
	rtROI.right = nROIX + nROIW;
	rtROI.top = nROIY;
	rtROI.bottom = nROIY + nROIH;
	CRect rtDst;
	if (rtDst.IntersectRect(rtFOV, rtROI) == FALSE)
		return false;

	if (nType == ForeignParamROI_Data_Module)
	{
		float fExceptModuleAreaX = m_ForeignData.m_fArrData[FR_F_ExceptModuleAreaX];
		int nExceptModuleAreaX = fExceptModuleAreaX / m_resolX / nDivide;
		if (nExceptModuleAreaX > rtDst.Width())
			return false;

		float fExceptModuleAreaY = m_ForeignData.m_fArrData[FR_F_ExceptModuleAreaY];
		int nExceptModuleAreaY = fExceptModuleAreaY / m_resolY / nDivide;
		if (nExceptModuleAreaY > rtDst.Height())
			return false;
	}

	RECT rcROI;
	rcROI.left = rtROI.left;
	rcROI.top = rtROI.top;
	rcROI.right = rtROI.right;
	rcROI.bottom = rtROI.bottom;
	if (bExtraPer && fExtraPixelCnt > 0)
	{
		float fFrRoiW = rtROI.right - rtROI.left;
		float fFrRoiH = rtROI.bottom - rtROI.top;
		if (fFrRoiW > 0)
		{
			float fMar = fFrRoiW * fExtraPixelCnt / 100.0f;
			float fMarGap = fMar - fFrRoiW;
			float fMarGapH = fMarGap / 2.0f;
			rcROI.left = rtROI.left - (int)fMarGapH;
			rcROI.right = rtROI.right + (int)fMarGapH;
		}

		if (fFrRoiH > 0)
		{
			float fMar = fFrRoiH * fExtraPixelCnt / 100.0f;
			float fMarGap = fMar - fFrRoiH;
			float fMarGapH = fMarGap / 2.0f;
			rcROI.top = rtROI.top - (int)fMarGapH;
			rcROI.bottom = rtROI.bottom + (int)fMarGapH;
		}
	}
	else if (bExtraPer == false)
	{
		rcROI.left = rtROI.left - (int)fExtraPixelCnt;
		rcROI.top = rtROI.top - (int)fExtraPixelCnt;
		rcROI.right = rtROI.right + (int)fExtraPixelCnt;
		rcROI.bottom = rtROI.bottom + (int)fExtraPixelCnt;
	}

	if (rtFOV.left > rcROI.left) rcROI.left = rtFOV.left;
	if (rtFOV.top > rcROI.top) rcROI.top = rtFOV.top;
	if (rtFOV.right < rcROI.right) rcROI.right = rtFOV.right;
	if (rtFOV.bottom < rcROI.bottom) rcROI.bottom = rtFOV.bottom;
	rcROI.left -= rtFOV.left;
	rcROI.right -= rtFOV.left;
	int nTop = rcROI.top;
	rcROI.top = nH - (rcROI.bottom - rtFOV.top);
	rcROI.bottom = nH - (nTop - rtFOV.top);
	if (rcROI.left < 0 || rcROI.top < 0 || rcROI.right <= 0 || rcROI.bottom <= 0 ||
		rcROI.left > nW || rcROI.top > nH || rcROI.right > nW || rcROI.bottom > nH)
		return false;

	cv::Mat Dstimg(nH, nW, CV_8UC1, pucDst);
	if (m_ForeignParamROI[nIndex].m_bPolygon)
	{
		if (m_ForeignParamROI[nIndex].m_nType == ForeignParamROI_Data_Module)
		{
			POINTF ptPolygon[ZMPOLYPOINT_ULTRA_CNTS];
			for (int b = 0; b < ZMPOLYPOINT_ULTRA_CNTS; b++)
			{
				ptPolygon[b].x = RounD(m_ForeignParamROI[nIndex].m_pROIHit[b].x / m_resolX / nDivide);
				ptPolygon[b].y = RounD(m_ForeignParamROI[nIndex].m_pROIHit[b].y / m_resolY / nDivide);
				ptPolygon[b].x -= rtFOV.left;
				ptPolygon[b].y -= rtFOV.top;
				ptPolygon[b].y = nH - ptPolygon[b].y;
			}
			if (FillOutOfInspArea(rcROI, nW, nH, 0, Dstimg.data, NULL, ZMPOLYPOINT_ULTRA_CNTS, ptPolygon, byValue, TRUE, bExtraPer, fExtraPixelCnt) == 0)
				return false;
		}
		else if (m_ForeignParamROI[nIndex].m_nType == ForeignParamROI_Data_PAD)
		{
			int nTotalPoly = m_ForeignParamROI[nIndex].m_nPolyCNT;
			if (nTotalPoly == ZMPOLYPOINT_CNTS || nTotalPoly == ZMPOLYPOINT_ULTRA_CNTS)
			{
				std::vector<cv::Point> polyPoints;
				polyPoints.reserve(nTotalPoly); // 각 폴리곤 포인트 공간 예약
				bool isUltraCnts = (nTotalPoly == ZMPOLYPOINT_ULTRA_CNTS);
				for (int b = 0; b < nTotalPoly; b++)
				{
					float fX = isUltraCnts ? m_ForeignParamROI[nIndex].m_pROIHit[b].x : m_ForeignParamROI[nIndex].m_pROI[b].x;
					float fY = isUltraCnts ? m_ForeignParamROI[nIndex].m_pROIHit[b].y : m_ForeignParamROI[nIndex].m_pROI[b].y;

					int nPolyX = RounD(fX / (m_resolX * nDivide)) - rtFOV.left;
					int nPolyY = nH - (RounD(fY / (m_resolY * nDivide)) - rtFOV.top);

					polyPoints.push_back(cv::Point(nPolyX, nPolyY));
				}
				cv::fillPoly(Dstimg, std::vector<std::vector<cv::Point>>{polyPoints}, cv::Scalar(byValue));
			}
			else
				Dstimg(cv::Rect(rcROI.left, rcROI.top, rcROI.right - rcROI.left, rcROI.bottom - rcROI.top)).setTo(byValue);
		}
		else
		{
			POINTF ptPolygon[ZMPOLYPOINT_CNTS];
			for (int b = 0; b < ZMPOLYPOINT_CNTS; b++)
			{
				ptPolygon[b].x = RounD(m_ForeignParamROI[nIndex].m_pROI[b].x / m_resolX / nDivide);
				ptPolygon[b].y = RounD(m_ForeignParamROI[nIndex].m_pROI[b].y / m_resolY / nDivide);
				ptPolygon[b].x -= rtFOV.left;
				ptPolygon[b].y -= rtFOV.top;
				ptPolygon[b].y = nH - ptPolygon[b].y;
			}
			if (FillOutOfInspArea(rcROI, nW, nH, 0, Dstimg.data, NULL, ZMPOLYPOINT_CNTS, ptPolygon, byValue, TRUE, bExtraPer, fExtraPixelCnt) == 0)
				return false;
		}
	}
	else
		Dstimg(cv::Rect(rcROI.left, rcROI.top, rcROI.right - rcROI.left, rcROI.bottom - rcROI.top)).setTo(byValue);
	return true;
}
void InspManager::FillPolygon(cv::Mat Dst, POINTF *pts, int nMaxCount, cv::Scalar color)
{
	int width = Dst.cols;
	int height = Dst.rows;

	std::vector<cv::Point> contour;
	for (size_t j = 0; j < nMaxCount; j++)
		contour.push_back(cv::Point(pts[j].x, pts[j].y));

	const cv::Point *pts3 = (cv::Point*) cv::Mat(contour).data;
	fillPoly(Dst, &pts3, &nMaxCount, 1, color);
}
void InspManager::InspFRArea(ForeignData sInspData, cv::Mat imgArea)
{
	int nW = imgArea.cols;
	int nH = imgArea.rows;
	int nROITotal = m_ForeignData.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= 0 || nW <= 0 || nH <= 0)
		return;

	double dFactorX = 1.0;
	double dFactorY = 1.0;
	if (sInspData.m_bDEBUG)
	{
		dFactorX = (double)sInspData.m_rcFOV.right / (double)sInspData.m_nWP_W;
		dFactorY = (double)sInspData.m_rcFOV.bottom / (double)sInspData.m_nWP_H;

		int nPCBL = (int)((double)m_ForeignData.m_nArrData[FR_N_PCBL] / dFactorX + 0.5);
		int nPCBR = (int)((double)m_ForeignData.m_nArrData[FR_N_PCBR] / dFactorX + 0.5);
		int nPCBT = (int)((double)m_ForeignData.m_nArrData[FR_N_PCBT] / dFactorY + 0.5);
		int nPCBB = (int)((double)m_ForeignData.m_nArrData[FR_N_PCBB] / dFactorY + 0.5);

		imgArea(cv::Rect(0, 0, nPCBL, nH)).setTo(0);
		imgArea(cv::Rect(0, 0, nW, nPCBT)).setTo(0);
		imgArea(cv::Rect(nW - nPCBR, 0, nPCBR, nH)).setTo(0);
		imgArea(cv::Rect(0, nH - nPCBB, nW, nPCBB)).setTo(0);
	}

	for (int a = 0; a < nROITotal; a++)
	{
		int nType = m_ForeignParamROI[a].m_nType;
		int nROIModule = m_ForeignParamROI[a].m_nModuleID;
		if (nType == ForeignParamROI_Data_InspA)
			continue;

		if (nType != ForeignParamROI_Data_Module)
		{
			SetROI(sInspData, a, imgArea, nW, nH, dFactorX, dFactorY, 0);
			continue;
		}
		if (m_ForeignParamROI[a].m_bSKIP)
		{
			SetROI(sInspData, a, imgArea, nW, nH, dFactorX, dFactorY, 0);
			continue;
		}
		if (m_ForeignParamROI[a].m_nModuleID != sInspData.m_nContainModuleID)
		{
			SetROI(sInspData, a, imgArea, nW, nH, dFactorX, dFactorY, 0);
			continue;
		}
	}

	for (int a = 0; a < sInspData.m_nTotalCnt; a++)
	{
		int nROIX = sInspData.m_pXList[a];
		int nROIY = sInspData.m_pYList[a];
		int nROIW = sInspData.m_pWList[a] + 1;
		int nROIH = sInspData.m_pHList[a] + 1;
		if (nROIX < 0) nROIX = 0;
		if (nROIY < 0) nROIY = 0;
		if (nROIX + nROIW > nW) nROIW = nW - nROIX;
		if (nROIY + nROIH > nH) nROIH = nH - nROIY;
		if (nROIW <= 0 || nROIH <= 0)
			continue;

		imgArea(cv::Rect(nROIX, nROIY, nROIW, nROIH)).setTo(0);
	}
}
bool InspManager::SetROI(ForeignData sInspData, int nIndex, cv::Mat img, int nW, int nH, double dFactorX, double dFactorY, byte byValue)
{
	int nROITotal = m_ForeignData.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= nIndex)
		return false;

	long long nTotal = nW * nH;
	int nROIX = RounD(m_ForeignParamROI[nIndex].m_pPos.x / m_resolX / dFactorX);
	int nROIY = RounD(m_ForeignParamROI[nIndex].m_pPos.y / m_resolY / dFactorY);
	int nROIW = RounD(m_ForeignParamROI[nIndex].m_pSize.x / m_resolX / dFactorX);
	int nROIH = RounD(m_ForeignParamROI[nIndex].m_pSize.y / m_resolY / dFactorY);
	if (sInspData.m_bDEBUG == FALSE)
	{
		POINTF poCur = g_pMPTI->GetForeignCurFovCenter();
		int nCX = RounD(poCur.x / m_resolX);
		int nCY = RounD(poCur.y / m_resolY);
		CRect rtFOV;
		rtFOV.left = nCX - (nW / 2);
		rtFOV.right = rtFOV.left + nW;
		rtFOV.top = nCY - (nH / 2);
		rtFOV.bottom = rtFOV.top + nH;
		nROIX -= rtFOV.left;
		nROIY -= rtFOV.top;
	}

	if (nROIX < 0) nROIX = 0;
	if (nROIY < 0) nROIY = 0;
	if (nROIX + nROIW > nW) nROIW = nW - nROIX;
	if (nROIY + nROIH > nH) nROIH = nH - nROIY;
	if (nROIW <= 0 || nROIH <= 0)
		return false;

	RECT rcROI;
	rcROI.left = nROIX;
	rcROI.top = nROIY;
	rcROI.right = nROIX + nROIW;
	rcROI.bottom = nROIY + nROIH;

	if (m_ForeignParamROI[nIndex].m_bPolygon)
	{
		if (m_ForeignParamROI[nIndex].m_nType == ForeignParamROI_Data_Module)
		{
			POINTF ptPolygon[ZMPOLYPOINT_ULTRA_CNTS];
			for (int b = 0; b < ZMPOLYPOINT_ULTRA_CNTS; b++)
			{
				ptPolygon[b].x = RounD(m_ForeignParamROI[nIndex].m_pROIHit[b].x / m_resolX / dFactorX);
				ptPolygon[b].y = RounD(m_ForeignParamROI[nIndex].m_pROIHit[b].y / m_resolY / dFactorY);
				ptPolygon[b].y = nH - ptPolygon[b].y;
			}
			if (FillOutOfInspArea(rcROI, nW, nH, 0, img.data, NULL, ZMPOLYPOINT_ULTRA_CNTS, ptPolygon, byValue, TRUE) == 0)
				return false;
		}
		else
		{
			POINTF ptPolygon[ZMPOLYPOINT_CNTS];
			for (int b = 0; b < ZMPOLYPOINT_CNTS; b++)
			{
				ptPolygon[b].x = RounD(m_ForeignParamROI[nIndex].m_pROI[b].x / m_resolX / dFactorX);
				ptPolygon[b].y = RounD(m_ForeignParamROI[nIndex].m_pROI[b].y / m_resolY / dFactorY);
				ptPolygon[b].y = nH - ptPolygon[b].y;
			}
			if (FillOutOfInspArea(rcROI, nW, nH, 0, img.data, NULL, ZMPOLYPOINT_CNTS, ptPolygon, byValue, TRUE) == 0)
				return false;
		}
	}
	else
		img(cv::Rect(nROIX, nROIY, nROIW, nROIH)).setTo(byValue);
	return true;
}
int InspManager::FillOutOfInspArea(RECT rcROI, int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue, BOOL bConvertExceptROI, bool bExtraPer, float fExtraPixelCnt)
{
	int cnt = 0;
	if (nUsedInspPolygon < 1 || ptArrInspPolygon == NULL || ucArrSrcImg == NULL)
		return cnt;

	POINTF* ptDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
	memcpy(ptDstPoint, ptArrInspPolygon, sizeof(POINTF) * nUsedInspPolygon);

	//후보 영역
	cv::Mat ImgPoly(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat ImgPolyPerpect(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	if (PolygonAreaCalc())
	{
		cv::Scalar white(255, 255, 255);
		FillPolygon(ImgPoly, ptDstPoint, nUsedInspPolygon, white);
		MorErode(ImgPoly, ImgPolyPerpect);
		if (bExtraPer && fExtraPixelCnt > 0)
		{
			int nMorX = 1 + (fExtraPixelCnt * 2);
			int nMorY = 1 + (fExtraPixelCnt * 2);
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nMorX, nMorY));
			cv::Mat ImgPolyD(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
			memcpy(ImgPolyD.data, ImgPoly.data, nWidth * nHeight * sizeof(UCHAR));
			cv::dilate(ImgPolyD, ImgPoly, kernel);
		}
	}

	for (int i = rcROI.top; i < rcROI.bottom; i++)
	{
		for (int j = rcROI.left; j < rcROI.right; j++)
		{
			POINTF cur;
			cur.x = j;
			cur.y = i;
			long long nIndex = (nWidth * i) + j;
			if (nIndex < 0 || nIndex >= nWidth * nHeight)
				continue;

			bool bColor = false;
			if (PolygonAreaCalc())
			{
				if (ImgPoly.data[nIndex] == 255)
				{
					if (ImgPolyPerpect.data[nIndex] == 255)
						bColor = true;
					else
						bColor = PtInPolygon(cur, ptDstPoint, nUsedInspPolygon);
				}
			}
			else
				bColor = PtInPolygon(cur, ptDstPoint, nUsedInspPolygon);

			if (bColor == bConvertExceptROI)
			{
				ucArrSrcImg[nIndex] = nFillValue;
				if (ucArrOverlapImg != NULL)
					ucArrOverlapImg[nIndex] = 1;
				cnt++;
			}
		}
	}
	Delete_1DArray(&ptDstPoint);
	return cnt;
}
void InspManager::MorErode(cv::Mat Src, cv::Mat Dst, int nShape, int ksize)
{
 	if (nShape > cv::MORPH_ELLIPSE) nShape = cv::MORPH_ELLIPSE;
 	if (ksize < 3) ksize = 3;
 	int nW = Src.cols;
 	int nH = Src.rows;
 	IppiSize roiSize = { nW, nH };
 
 	/*Ipp8u pMask[3 * 3] =
 	{ 1, 1, 1,
 	  1, 0, 1,
 	  1, 1, 1 };
 	IppiSize maskSize = { 3, 3 };*/
 	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));
 	Ipp8u* pMask = kernel.data;
 	IppiSize maskSize = { ksize, ksize };
 
 	int specSize = 0, bufferSize = 0;
 	IppStatus status = ippiMorphologyBorderGetSize_8u_C1R(roiSize, maskSize, &specSize, &bufferSize);
 	if (status != ippStsNoErr)
 		return;
 
 	IppiMorphState* pSpec = (IppiMorphState*)ippsMalloc_8u(specSize);
 	Ipp8u* pBuffer = (Ipp8u*)ippsMalloc_8u(bufferSize);
 	status = ippiMorphologyBorderInit_8u_C1R(roiSize, pMask, maskSize, pSpec, pBuffer);
 	if (status != ippStsNoErr)
 	{
 		ippsFree(pBuffer);
 		ippsFree(pSpec);
 		return;
 	}
 
 	IppiBorderType borderType = ippBorderRepl;
 	Ipp16u borderValue = 0;
 	status = ippiErodeBorder_8u_C1R(Src.data, nW, Dst.data, nW, roiSize, borderType, borderValue, pSpec, pBuffer);
 
 	ippsFree(pBuffer);
 	ippsFree(pSpec);
}
int InspManager::ChangeInspAlgoResult(int nWndIndex, InspWndResult* WndResult, InspAlgoTempResult * ptrInspAlgoTempResult)
{
	int ret = e_OK;
	int inspType = 0;
	int nLine = __LINE__;
	try
	{
		bool bChangeResult = false;
		int nWndID = m_pParamArray[nWndIndex].wndIndex;
		inspType = m_pParamArray[nWndIndex].inspType;
		nLine = __LINE__;
		InspAlgoType eAlgoType = eAlgoBW;
		for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
		{
			if (!m_pParamArray[nWndIndex].vArrAlgoParam || !WndResult->m_vArrRstInspAlgo)
				continue;
			nLine = __LINE__;
			InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
			nLine = __LINE__;
			switch (sInspAlgo.m_eAlgoType)
			{
			case  eAlgoLead_Lift:
			{
				eAlgoType = sInspAlgo.m_eAlgoType;
				RstAlgoLeadLift * sRstAlgo = (RstAlgoLeadLift *)WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
				if (ptrInspAlgoTempResult->m_bParentWnd == true)
				{
					if (sRstAlgo->m_bOKCoplanarity == FALSE)
					{
						ptrInspAlgoTempResult->m_nLeadLiftGroupCnt = sRstAlgo->m_nLeadCoplanarityCnt;
						for (int n = 0; n < sRstAlgo->m_nLeadCoplanarityCnt; n++)
							ptrInspAlgoTempResult->m_ptrnLeadWndID[n] = sRstAlgo->m_nArrLeadCoplanarityRst[n];
					}
					else
						ptrInspAlgoTempResult->m_nLeadLiftGroupCnt = 0;
				}
				else
				{
					if (ptrInspAlgoTempResult->m_nLeadLiftGroupCnt > 0)
					{
						BOOL bOKCoplanarity = TRUE;
						for (int n = 0; n < ptrInspAlgoTempResult->m_nLeadLiftGroupCnt; n++)
						{
							if (ptrInspAlgoTempResult->m_ptrnLeadWndID[n] == nWndID)
							{
								bOKCoplanarity = FALSE;
								break;
							}
						}
						sRstAlgo->m_bOKCoplanarity = bOKCoplanarity;
						if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk != bOKCoplanarity)
							bChangeResult = true;
						WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk = bOKCoplanarity;

						if (sInspAlgo.m_bIsRequired && !sInspAlgo.m_bAlgoGroup && !bOKCoplanarity)
							ret = e_NG;
						// 교체용 디펙트 코드
						if (!bOKCoplanarity && sInspAlgo.m_bUsingManualDefectCode)
						{
							WndResult->m_vArrRstInspAlgo[nAlgo].m_bUsingManualDefectCode = sInspAlgo.m_bUsingManualDefectCode;
							WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualDefectCode = sInspAlgo.m_nManualDefectCode;
							WndResult->m_vArrRstInspAlgo[nAlgo].m_nManualSubDefectCode = sInspAlgo.m_nManualSubDefectCode;
						}
					}
				}
			}
			break;
			}
		}

		BOOL bIsInsp = TRUE;
		if (bChangeResult == true && ret == e_OK)
		{
			nLine = __LINE__;
			int nResGroup = e_NG;		// Group이 아닌 알고리즘들의 결과
			int nResNotGroup = e_NG;		// Group인 알고리즘들의 결과
			BOOL bIsEssentialInGroup = FALSE;	// Group에 포함된 알고리즘 중 Essential check된 것이 있는지 여부
			bIsInsp = FALSE;
			if (m_pParamArray[nWndIndex].nAlgorithmCnt > 0)
				ret = e_NG;		// 일단 NG로,
			for (int nAlgo = 0; nAlgo < m_pParamArray[nWndIndex].nAlgorithmCnt; nAlgo++)
			{
				if (!WndResult->m_vArrRstInspAlgo)
					continue;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Search || WndResult->m_vArrRstInspAlgo[nAlgo].m_nAlgoType == eAlgoLead_Tip)
				{
					if (m_pParamArray[nWndIndex].nAlgorithmCnt == 1)
						ret = e_OK;	// LeadSearch
					continue;
				}
				// 알고리즘 간 기본 OR 처리 (여기 들어왔다는 것은 필수 검사는 모두 OK 인 경우이므로 고려하지 않아도 됨)
				// 2015/08/10 : 알고리즘 Group flag 추가되어서 필수 검사 NG 여부도 여기서 체크해야함

				nLine = __LINE__;

				InspAlgo sInspAlgo = m_pParamArray[nWndIndex].vArrAlgoParam[nAlgo];
				if (sInspAlgo.m_bAlgoGroup && sInspAlgo.m_bIsRequired)
					bIsEssentialInGroup = TRUE;

				nLine = __LINE__;

				if (WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk == TRUE)
				{
					if (!sInspAlgo.m_bAlgoGroup)
						nResNotGroup = e_OK;		// Group Enable = false인 알고리즘 결과가 하나라도 OK라면 OK
					else
						nResGroup = e_OK;		// Group Enable = true인 알고리즘 결과가 하나라도 OK라면 OK
				}

				bIsInsp = TRUE;
			}

			nLine = __LINE__;

			if (nResGroup == e_NG && bIsEssentialInGroup)		// Group의 결과가 NG이고 Group에 포함된 알고리즘 중 Essential check가 있으면 무조건 NG
				ret = e_NG;
			else if (nResNotGroup == e_OK || nResGroup == e_OK)	// 그렇지 않고, Group이 아닌 알고리즘 중 OK가 있거나, Group인 알고리즘 중 OK가 있으면 OK
				ret = e_OK;
		}
		if (bChangeResult == true)
		{
			WndResult->m_bOk = ret;
			WndResult->m_bIsInsp = bIsInsp;
		}
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CMInspManager::ChangeInspAlgoResult(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return ret;
}
BOOL InspManager::FillExceptWndArea(UCHAR* MaskImg, RECT rtCurrentFOVWnd, int nImgWidth, int nImgHeight, float nAngle)
{
	if (IsAnyAngle(nAngle))
		// 일반각
	{
		float sin_theta = sin(nAngle *PI / 180.f);
		float cos_theta = cos(nAngle *PI / 180.f);

		int r_left_m = -rtCurrentFOVWnd.right / 2;
		int r_top_m = -rtCurrentFOVWnd.bottom / 2;

		for (int y = r_top_m; y <= -r_top_m; y++)
		{
			for (int x = r_left_m; x <= -r_left_m; x++)
			{

				int i = (int)(-x * sin_theta + y * cos_theta + rtCurrentFOVWnd.top) * nImgWidth + (int)(x*cos_theta + y * sin_theta) + rtCurrentFOVWnd.left;
				if (i >= 0 && i < nImgWidth*nImgHeight)
					MaskImg[i] = 0;
			}
		}

	}
	else
	{
		for (int i = rtCurrentFOVWnd.top; i < rtCurrentFOVWnd.bottom; i++)
		{
			for (int j = rtCurrentFOVWnd.left; j < rtCurrentFOVWnd.right; j++)
			{
				MaskImg[i*nImgWidth + j] = 0;
			}
		}
	}


	return TRUE;
}
cv::Point2f InspManager::RotatePoint(cv::Point Po, cv::Point Ce, double dAngle)
{
	double x = (Po.x - Ce.x) * std::cos((dAngle) / 180 * PI) - (Po.y - Ce.y)*std::sin((dAngle) / 180 * PI) + Ce.y;
	double y = (Po.x - Ce.x) * std::sin((dAngle) / 180 * PI) + (Po.y - Ce.y)*std::cos((dAngle) / 180 * PI) + Ce.x;
	return cv::Point2f(x, y);
}
void InspManager::SavePTR(CString str, cv::Mat img3D, int nBit)
{
	SaveZmapPTR(img3D.ptr<float>(), str, img3D.cols, img3D.rows, nBit);
}

void InspManager::SaveZmapPTR(float *pZmap, CString filePath, int nW, int nH, int nBit)
{
	if (filePath.GetLength() == 0 || pZmap == NULL)
		return;
	if (nW <= 0 || nH <= 0 || nBit <= 0)
		return;

	HEADER_PTR hd;
	hd.uiNumRow = nW;
	hd.uiNumCol = nH;
	hd.zResolX = m_resolX * 1000.0;
	hd.zResolY = m_resolY * 1000.0;
	hd.sizeBit = nBit;
	alpf_save_ptr(filePath, &hd, pZmap);
}
void InspManager::DeleteInspItem()
{
	int cnt = 0;

	if (m_pMountInsp != NULL)
	{
		//delete [] m_pMountInsp;
		g_pMManager->pem_delete(m_pMountInsp, true);
		m_pMountInsp = NULL;
	}

	if (m_pOcrInsp != NULL)
	{
		// 		if(!m_inspItemCnts || (m_inspItemCnts[eINSP_OCR] < 2))
		// 			delete m_pOcrInsp;
		// 		else
					//delete [] m_pOcrInsp;
		g_pMManager->pem_delete(m_pOcrInsp, true);
		m_pOcrInsp = NULL;
	}

	if (m_pColorInsp != NULL)
	{
		// 		if(!m_inspItemCnts || (m_inspItemCnts[eINSP_COLOR] < 2))
		// 			delete m_pColorInsp;
		// 		else
					//delete [] m_pColorInsp;
		g_pMManager->pem_delete(m_pColorInsp, true);
		m_pColorInsp = NULL;
	}

	if (m_pGridInsp != NULL)  // YJS 2016/11/04
	{
		//delete [] m_pGridInsp;
		g_pMManager->pem_delete(m_pGridInsp, true);

		m_pGridInsp = NULL;
	}


	if (m_pPatternInsp != NULL)
	{
		// 		if(!m_inspItemCnts || (m_inspItemCnts[eINSP_PATTERN] < 2))
		// 			delete m_pPatternInsp;
		// 		else
					//delete [] m_pPatternInsp;
		g_pMManager->pem_delete(m_pPatternInsp, true);
		m_pPatternInsp = NULL;
	}

	if (m_pLeadSolderInsp != NULL)
	{
		//delete [] m_pLeadSolderInsp;
		g_pMManager->pem_delete(m_pLeadSolderInsp, true);
		m_pLeadSolderInsp = NULL;
	}

	//if(m_pBGAInsp != NULL)
	//{
	//	delete [] m_pBGAInsp;
	//	m_pBGAInsp = NULL;
	//}

	if (m_pForeignPattern != NULL)
	{
		//delete m_pForeignPattern;
		g_pMManager->pem_delete(m_pForeignPattern, false);
		m_pForeignPattern = NULL;
	}
}
void InspManager::Delete_Algorithms()
{
	for (int i = 0; i < InspAlgoType::eAlgoNum; i++)
	{
		if (m_dicInspAlgorithm[(InspAlgoType)i])
		{
			g_pMManager->pem_delete(m_dicInspAlgorithm[(InspAlgoType)i], false);
			m_dicInspAlgorithm[(InspAlgoType)i] = nullptr;
		}
	}
	m_dicInspAlgorithm.erase(m_dicInspAlgorithm.begin(), m_dicInspAlgorithm.end());
}
void InspManager::DeleteTeachItem()
{
	if (m_pMountTeach != NULL)
	{
		//delete m_pMountTeach;
		g_pMManager->pem_delete(m_pMountTeach, false);
		m_pMountTeach = NULL;
	}

	if (m_pOcrTeach != NULL)
	{
		//delete m_pOcrTeach;
		g_pMManager->pem_delete(m_pOcrTeach, false);
		m_pOcrTeach = NULL;
	}

	if (m_pColorTeach != NULL)
	{
		//delete m_pColorTeach;
		g_pMManager->pem_delete(m_pColorTeach, false);
		m_pColorTeach = NULL;
	}

	if (m_pPatternTeach != NULL)
	{
		//delete m_pPatternTeach;
		g_pMManager->pem_delete(m_pPatternTeach, false);
		m_pPatternTeach = NULL;
	}

	if (m_pLeadSolderTeach != NULL)
	{
		//delete m_pLeadSolderTeach;
		g_pMManager->pem_delete(m_pLeadSolderTeach, false);
		m_pLeadSolderTeach = NULL;
	}

	/*if(m_pBGATeach != NULL)
	{
		//delete m_pBGATeach;
		g_pMManager->pem_delete(m_pBGATeach, false);
		m_pBGATeach = NULL;
	}*/

	if (m_pPOCR != NULL)
	{
		//delete m_pPOCR;
		g_pMManager->pem_delete(m_pPOCR, false);
		m_pPOCR = NULL;
	}


	if (m_pWireBondingInsp != NULL)
	{
		//delete m_pWireBondingInsp;
		g_pMManager->pem_delete(m_pWireBondingInsp, false);
		m_pWireBondingInsp = NULL;
	}

	if (m_PInspWrapper != NULL)
	{
		g_pMManager->pem_delete(m_PInspWrapper, false);
		m_PInspWrapper = NULL;
	}

	if (m_pTab != NULL)
	{
		g_pMManager->pem_delete(m_pTab, false);
		m_pTab = NULL;
	}
}
RECT InspManager::GetWndROI(int nCurWndIndex, int nAlignTotalCnt, AlignResult * sptrAlignRes)
{
	RECT rcWnd;
	rcWnd.left = 0;
	rcWnd.right = 0;
	rcWnd.top = 0;
	rcWnd.bottom = 0;
	double dResolX = m_resolX;
	double dResolY = m_resolY;
	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}

	double dPartW = m_pInspBoardInfo->partWidth;
	double dPartH = m_pInspBoardInfo->partHeight;
	double dWndW = m_pParamArray[nCurWndIndex].width;
	double dWndH = m_pParamArray[nCurWndIndex].length;
	double dWndCX = m_pParamArray[nCurWndIndex].cx;
	double dWndCY = m_pParamArray[nCurWndIndex].cy;

	int nPartW = dPartW / dResolX;
	int nPartH = dPartH / dResolY;
	int nWndW = RounD(dWndW / dResolX);
	int nWndH = RounD(dWndH / dResolY);
	double dX = 0;
	double dY = 0;
	int nTotalCnt = nAlignTotalCnt;
	if (nTotalCnt <= 0) nTotalCnt = 1;
	bool bArrArr = (nAlignTotalCnt >= 2);
	float fPartErrX = m_pInspBoardInfo->fPartRoundingErrX;
	float fPartErrY = m_pInspBoardInfo->fPartRoundingErrY;
	for (int n = 0; n < nTotalCnt; n++)
	{
		int nAlignIdx = (bArrArr) ? n : 0;
		double dAddX = dWndCX;
		double dAddY = dWndCY;
		if (nAlignTotalCnt > nAlignIdx && sptrAlignRes)
		{
			double dAlignRes_centerX = sptrAlignRes[nAlignIdx].centerX;
			double dAlignRes_centerY = sptrAlignRes[nAlignIdx].centerY;
			double dAlignRes_theta = sptrAlignRes[nAlignIdx].theta;
			double dAlignRes_offsetX = sptrAlignRes[nAlignIdx].offsetX;
			double dAlignRes_offsetY = sptrAlignRes[nAlignIdx].offsetY;
			if (sptrAlignRes[nAlignIdx].m_bAlgoCenter == TRUE)
			{
				if (sptrAlignRes[nAlignIdx].m_dCenterX_R > 0 &&
					sptrAlignRes[nAlignIdx].m_dCenterY_R > 0 &&
					sptrAlignRes[nAlignIdx].m_dCenterX_R < m_pInspBoardInfo->partWidth &&
					sptrAlignRes[nAlignIdx].m_dCenterY_R < m_pInspBoardInfo->partHeight)
				{
					dAlignRes_centerX = sptrAlignRes[nAlignIdx].m_dCenterX_R - (m_pInspBoardInfo->partWidth / 2.0);
					dAlignRes_centerY = sptrAlignRes[nAlignIdx].m_dCenterY_R - (m_pInspBoardInfo->partHeight / 2.0);
				}
			}
			double corr_x = 0, corr_y = 0;
			m_proc3d.CorrectCoordinate(dWndCX, dWndCY, dAlignRes_centerX, dAlignRes_centerY, dAlignRes_theta, dAlignRes_offsetX, dAlignRes_offsetY, &corr_x, &corr_y);
			if (bArrArr)
			{
				dWndCX = corr_x;
				dWndCY = corr_y;
			}
			dAddX = corr_x;
			dAddY = corr_y;
		}

		double dStartX = dPartW / 2. + dWndCX;
		double dStartY = dPartH / 2. - dWndCY;
		double dDX = (dStartX - (dWndW / 2.)) / dResolX;
		double dDY = (dStartY - (dWndH / 2.)) / dResolY;
		m_pInspBoardInfo->m_sBigInfo.BigPartRoundingErr(dDX, dDY, nWndW, nWndH, fPartErrX, fPartErrY);
		double fDXR = dDX - fPartErrX;
		double fDYR = dDY - fPartErrY;
		dX = RounD(fDXR);
		dY = RounD(fDYR);

		if (dX < 0) dX = 0;
		if (dY < 0) dY = 0;
		if (dX + nWndW > nPartW)	dX = nPartW - nWndW;
		if (dY + nWndH > nPartH)	dY = nPartH - nWndH;
		if (dX < 0) dX = 0;
		if (dY < 0) dY = 0;
	}

	rcWnd.left = dX;
	rcWnd.right = dX + nWndW;
	rcWnd.top = dY;
	rcWnd.bottom = dY + nWndH;
	return rcWnd;
}

void InspManager::Exception_SaveImage()
{
	int ret = 0;
	//CString str2dFullPath = _T("");
	//str2dFullPath = (m_inspectionResult->nDefectType == defectCode::dftCODE_OK) ? m_pInspBoardInfo->sOKImagePath : m_pInspBoardInfo->sNGImagePath;
// 	CString sTemp;
// 	sTemp.Format(_T("2: %s\n"), m_pInspBoardInfo->sNGImagePath);
	//ext::Log::add((sTemp));
	CString str2dFullPath = m_pInspBoardInfo->s2DImagePath;//_T("D:\\Eagle3D_data\\InspectResult\\Image\\mobis_Wire\\wire_1115\\20230308124734\\1@2");
	CString str3dFullPath = m_pInspBoardInfo->s3DImagePath;// _T("D:\\Eagle3D_data\\InspectResult\\Image\\mobis_Wire\\test\\wire_1115\\20230308124734\\1@3");//

	int nRIndex = str2dFullPath.ReverseFind('\\');
	int pos = 0;
	CString delimiter = _T("\\");
	CString token;
	std::vector< CString> strPath;
	while ((token = str2dFullPath.Tokenize(delimiter, pos)) != "") // Module 안넘겨줘
		strPath.emplace_back(token);
	if (strPath.size() == 0) return ;  // 2D 가 빈값으로 들어오면 문제가 있으니 여기서 걸러줌.
	CString sModuleNo = strPath[strPath.size() - 1].Mid(0, 1); //Module 
	CString sInspctStart = strPath[strPath.size() - 2];


	CString s2DName = str2dFullPath.Mid(str2dFullPath.ReverseFind('\\') + 1);
	CString s2DPath;
	s2DPath.Format(_T("C:\\Eagle3D_64x\\PROGRAM\\AlgoTool\\DumpMessage\\"));
	s2DPath.AppendFormat(_T("%s"), sInspctStart);
	CString s3DName = str3dFullPath.Mid(str3dFullPath.ReverseFind('\\') + 1);
	CString s3DPath = s2DPath;
	int nSaveImage2D = 0;
	int nSaveImage3D = 0;
	bool b2DImageSave = true;


	if (m_pInspBoardInfo->partImgBuf.nImageSizeX *  m_pInspBoardInfo->partImgBuf.nImageSizeY > m_ExtProdInfo->nKeep3DImageSize)
		nSaveImage2D |= 0x00000002;

	if (g_pMPTI->GetExtMachineType() == (int)ext::eMachineType::eWIR)
	{
		nSaveImage3D = 0;
		if (m_inspectionResult->nDefectType != 0)
		{
			int n = s2DPath.Find(_T("OKImage")); // OK 이미지가 패스명에있으면 
			if (n != -1)
				s2DPath.Delete(n, 7); // 잘라줌
		}
		else
		{
			int n = s2DPath.Find(_T("OKImage")); //OK 인대 패스명에없으면
			if (n == -1)
				s2DPath.AppendFormat(_T("OKImage")); // 붙혀줌.

		}

	}
	//bool b2DImageSave = ((m_inspectionResult->nDefectType == 100/* defectCode.dftCODE_OK*/) & nSaveImage2D) || ((m_inspectionResult->nDefectType != 100/*defectCode.dftCODE_OK*/) & !nSaveImage2D);

#define  SaveTest 1 
	SaveImgParamData tgparam;
#if SaveTest 
	//InspRstDtCtrl.sInspctStart, InspRstDtCtrl.Barcode
	CString strRawTemp;
	strRawTemp.Format(_T("D:\\Temp\\Eagle3d\\rawdata3d\\%s@"), sInspctStart, sModuleNo);

	tgparam.unNGType = m_inspectionResult->nDefectType; // m_inspectionResult->nDefectType
	tgparam.nSave_2d = 1;// m_ExtProdInfo->nImgSave2DCheck;//m_ExtProdInfo->nImgSave2DCheck
	tgparam.nSave_3d = 1;
	tgparam.nMSPtImg = 0;
	tgparam.nSave_put = 0;
	tgparam.nSave_AOILink = 0;
	tgparam.nSave_NextMC = 0;
	tgparam.m_nSave_MES = 0;
	tgparam.m_nSave_APC = 0;
	tgparam.m_nSave_Color = 0;
	// tgparam.m_nSave_Color = 1; // AngleColor 는 현재 사용 x 
	tgparam.m_nSave_3DRaw = m_ExtProdInfo->m_nSave_3DRaw;
	tgparam.m_nSave_ImgType = -1;
	//tgparam.m_nSave_ImgType = -1; // uv 
	tgparam.unSaveOption = m_ExtProdInfo->unSaveOption;
	tgparam.m_n2DSaveQuality = 75;
	wstring temp = L"";
	wcscpy(tgparam.arrNGCaption, temp.c_str());
	temp = s2DName.operator LPCWSTR();//L"1@2";
	wcscpy(tgparam.arrJpgFileName, temp.c_str());

	CString s3DRawName;
	s3DRawName.Format(_T("%s@%s"), sModuleNo, m_pInspBoardInfo->s3DRawName);
	//wcscpy(tgparam.arr3dRawFileName, temp.c_str()); // 2D 꺼 가져감
	wcscpy(tgparam.arr3dRawFileName, s3DRawName);


	temp = s3DName.operator LPCWSTR();
	wcscpy(tgparam.arr3dFileName, temp.c_str());
	temp = (m_inspectionResult->nDefectType == 0) ? s2DPath.operator LPCWSTR() : s3DPath.operator LPCWSTR();
	wcscpy(tgparam.arrJpgFilePath, temp.c_str());
	temp = (m_inspectionResult->nDefectType == 0) ? s2DPath.operator LPCWSTR() : s3DPath.operator LPCWSTR();
	wcscpy(tgparam.arr3dFilePath, temp.c_str());

	wcscpy(tgparam.arr3dRawFilePath, temp.c_str());
	temp = strRawTemp.operator LPCWSTR();
	wcscpy(tgparam.arr3dRawFilePath, temp.c_str());



	// 	temp = L"D:\\Eagle3D_data\\AI_Sample\\LeadPartImage\\20230307";
	// 	wcscpy(tgparam.arr3dLeadOKFilePath	, temp.c_str());

		//tgparam.pArrSideImg
	NgParam ngColorParam;
	if (g_pMPTI->GetExtMachineType() == (int)ext::eMachineType::eWIR)
	{
		ngColorParam.redFator = 1.0;
		ngColorParam.greenFator = 1.0;
		ngColorParam.blueFator = 1.0;
		ngColorParam.btmRedfactor = 1.0;
		ngColorParam.btmBluefactor = 1.0;
	}
	else
	{
		ngColorParam.redFator = m_ExtProdInfo->fFactor_TR;
		ngColorParam.greenFator = m_ExtProdInfo->fFactor_TG;
		ngColorParam.blueFator = m_ExtProdInfo->fFactor_TB;
		ngColorParam.btmRedfactor = m_ExtProdInfo->fFactor_BR;
		ngColorParam.btmBluefactor = m_ExtProdInfo->fFactor_BB;
	}
	m_NgManager->SetNgParam(ngColorParam);

#endif


	//if (m_bMultiProcessThreadSave)
	{
		SaveImgParamData * tgSaveParam = new  SaveImgParamData();
		MultiStruct* multiSave = new MultiStruct();
		multiSave->nRoiArea = m_NgManager->GetROISize(multiSave->nRoiSizeX, multiSave->nRoiSizeY, multiSave->nWidthStep);
		tgSaveParam->Copy(tgparam);
		std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool , int  > TupleSaveImages;
		TupleSaveImages = (std::make_tuple(m_pInspBoardInfo, tgSaveParam, multiSave, b2DImageSave , 0 ));
		m_NgManager->SaveMultiInspPartImage(TupleSaveImages);
		// Copy 를 최소화 하기위한 Tuple
		
	}


}
bool InspManager::CheckWndShift(InspAlgoType eAlgo, bool bROT)
{
	if (eAlgo == eAlgoBlob || eAlgo == eAlgoVolume)
		return true;
	if (bROT)
	{
		if (eAlgo == eAlgoPOCR || eAlgo == eAlgoPattern)
			return true;
	}
	return false;
}
bool InspManager::ProcessAIDataByPartID(int nPartID, bool bUseAI, PIAL::Insp_Image* pImg_buf, double dWndW, double dWndH)
{
	//if (!bUseAI)
	//	return false;

	// 중복 체크 - 이미 처리된 PartID인지 확인
	{
		//std::lock_guard<std::mutex> lock(m_mtxAIPartID);// 혹여 쓰레드화할떄	
		if (m_setProcessedAIPartID.find(nPartID) != m_setProcessedAIPartID.end())
		{
			CString sLog = _T("");
			sLog.Format(_T("[AISeg] ProcessAIDataByPartID(), PartID %d already processed. Skipping."), nPartID);
			g_pMPTI->AddLog_Dev(sLog);
			//return false;
		}
		m_setProcessedAIPartID.insert(nPartID);
	}

	cv::Mat matAIData;

	// 무한 대기 (데이터가 들어올 때까지) PadBW 가 언제끝날지모름....
	const int nSleepMs = 10;
	int nWaitCount = 0;

	while (!ext::MatDatas::get()->GetDataByPartID(nPartID, matAIData))
	{
		Sleep(nSleepMs);
		nWaitCount++;

		// 1초마다 대기 중 로그 출력 (디버깅용)
		if (nWaitCount % 100 == 0)
		{
			int nFlag = -1;
			if (ext::irc::get()->_CtrlServer.GetSignal(ext::eMstSignal::eMstStop) == true)
				nFlag = 0;
			else if (ext::irc::get()->_CtrlServer.GetSignal(ext::eMstSignal::eMstAlive) == false)
				nFlag = 1;
			else if (ext::irc::get()->_CtrlServer.GetSignal(ext::eMstSignal::eMstAutoMode) == false)
				nFlag = 2;

			if (nFlag != -1)
				return false; 

			CString sLog = _T("");
			sLog.Format(_T("[AISeg] ProcessAIDataByPartID(), Waiting for MatData... PartID: %d, WaitTime: %dms nFlag : %d"), nPartID, nWaitCount * nSleepMs, nFlag);
			g_pMPTI->AddLog_Dev(sLog);
		}
	}

	// 데이터 수신 완료
	if (!matAIData.empty())
	{
		CString sLog = _T("");
		sLog.Format(_T("[AISeg] ProcessAIDataByPartID(), MatData received. PartID: %d, Size: %dx%d"), nPartID, matAIData.cols, matAIData.rows);
		g_pMPTI->AddLog_Dev(sLog);

		if (pImg_buf)
		{


			m_pInspBoardInfo->partImgBuf.imgAI = matAIData.data;
			//memcpy(m_pInspBoardInfo->partImgBuf.imgAI, matAIData.data, m_pInspBoardInfo->partImgBuf.nImageSizeX * m_pInspBoardInfo->partImgBuf.nImageSizeY);
		
			// AI 데이터를 BoardInfo에 저장 (필요 시)= 
			bool bSuccess = CopyAICroppedToWnd2D(pImg_buf, (int)dWndW, (int)dWndH, matAIData.data, m_pInspBoardInfo->partImgBuf.nImageSizeX, m_pInspBoardInfo->partImgBuf.nImageSizeY);
			if (!bSuccess)
			{
				CString sLog = _T("");
				sLog.Format(_T("[AISeg] ProcessAIDataByPartID(), Failed to crop AIimg to wnd size."));
				g_pMPTI->AddLog_Dev(sLog);
				return false;
			}
			return true;
		}
		else
		{
			CString sLog = _T("");
			sLog.Format(_T("[AISeg] ProcessAIDataByPartID(), pImg_buf is NULL"));
			g_pMPTI->AddLog_Dev(sLog);
		}
	}

	return false;
}

void InspManager::ClearProcessedAIPartID()
{
	//std::lock_guard<std::mutex> lock(m_mtxAIPartID); // 혹여 쓰레드화할떄	
	m_setProcessedAIPartID.clear();
}