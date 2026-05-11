#include "StdAfx.h"
#include "PInsp_WireBonding.h"
#include "MPTI.h"
#include <vector>
#include <windows.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#define COLOR_TESTCODE 0

#include "LeastSquare.h"
#include <array>
#include "HistogramAnalysis_New.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring> 
//#include "ippm.h"
#include "ipps.h"
#include "ippi.h"
using namespace std;



AlgoBlob CFoot_Model::GetAlgoBlob(int footKind)
{
	AlgoBlob algoBlob;

	algoBlob.m_bUseIPC = false;
	algoBlob.m_bInvertCheck = false;
	algoBlob.m_bFilterIsUse = false;
	algoBlob.m_nTypeSelectBlob = eSelectMix;
	algoBlob.m_bInsp3D = (m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) > 0 ? true : false;

	algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

	int Range3D = 0;
	if ((m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
		algoBlob.m_nTypeRange3D = 1;
	else if ((m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
		algoBlob.m_nTypeRange3D = 2;
	else if ((m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		algoBlob.m_nTypeRange3D = 3;
	algoBlob.m_dHeightRateMin = m_sArrBin[footKind].m_fArrH[m_eMMD::eMMD_Min];
	algoBlob.m_dHeightRateMax = m_sArrBin[footKind].m_fArrH[m_eMMD::eMMD_Max];


	algoBlob.m_bInsp2D = (m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) > 0 ? true : false;
	if ((m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
		algoBlob.m_nTypeRange2D = 1;
	else if ((m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
		algoBlob.m_nTypeRange2D = 2;
	else if ((m_sArrBin[footKind].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		algoBlob.m_nTypeRange2D = 3;
	algoBlob.m_nMinBinary = m_sArrBin[footKind].m_byArrValue[m_eBin::m_eBin_Min2D];
	algoBlob.m_nMaxBinary = m_sArrBin[footKind].m_byArrValue[m_eBin::m_eBin_Max2D];

	algoBlob.m_bFillHole = true;

	return algoBlob;
}

CWire_Model::CWire_Model()
{
	//m_ImageBuffer = new InspRoiImgBuf;
	m_ImageBuffer = g_pMManager->pem_new<InspRoiImgBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	memset(m_ImageBuffer, 0, sizeof(InspRoiImgBuf));

	pf3DOrg = nullptr;

	m_nWindowStartX = 0;
	m_nWindowStartY = 0;
	m_Width = 0;
	m_Height = 0;

	memset(&mImageRect, 0, sizeof(RECT));
}

CWire_Model::~CWire_Model()
{
	if (m_ImageBuffer->imgTop_R != NULL) { 
		//delete[] m_ImageBuffer->imgTop_R; 
		g_pMManager->pem_delete(m_ImageBuffer->imgTop_R, true);
		m_ImageBuffer->imgTop_R = nullptr; 
	}

	if (m_ImageBuffer->imgTop_G != NULL) { 
		//delete[] m_ImageBuffer->imgTop_G;
		g_pMManager->pem_delete(m_ImageBuffer->imgTop_G, true);
		m_ImageBuffer->imgTop_G = nullptr;
	}

	if (m_ImageBuffer->imgTop_B != NULL) { 
		//delete[] m_ImageBuffer->imgTop_B; 
		g_pMManager->pem_delete(m_ImageBuffer->imgTop_B, true);
		m_ImageBuffer->imgTop_B = nullptr; 
	}

	if (m_ImageBuffer->imgTop_W != NULL) { 
		//delete[] m_ImageBuffer->imgTop_W; 
		g_pMManager->pem_delete(m_ImageBuffer->imgTop_W, true);
		m_ImageBuffer->imgTop_W = nullptr; 
	}

	if (m_ImageBuffer->imgMiddle_R != NULL) { 
		//delete[] m_ImageBuffer->imgMiddle_R; 
		g_pMManager->pem_delete(m_ImageBuffer->imgMiddle_R, true);
		m_ImageBuffer->imgMiddle_R = nullptr; 
	}

	if (m_ImageBuffer->imgMiddle_B != NULL) { 
		//delete[] m_ImageBuffer->imgMiddle_B; 
		g_pMManager->pem_delete(m_ImageBuffer->imgMiddle_B, true);
		m_ImageBuffer->imgMiddle_B = nullptr; 
	}

	if (m_ImageBuffer->imgBottom_R != NULL) { 
		//delete[] m_ImageBuffer->imgBottom_R; 
		g_pMManager->pem_delete(m_ImageBuffer->imgBottom_R, true);
		m_ImageBuffer->imgBottom_R = nullptr; 
	}

	if (m_ImageBuffer->imgBottom_B != NULL) { 
		//delete[] m_ImageBuffer->imgBottom_B; 
		g_pMManager->pem_delete(m_ImageBuffer->imgBottom_B, true);
		m_ImageBuffer->imgBottom_B = nullptr; 
	}

	//delete m_ImageBuffer;
	g_pMManager->pem_delete(m_ImageBuffer, false);
	m_ImageBuffer = NULL;

}


CPInsp_WireBonding::CPInsp_WireBonding()
{
	m_className = _T("CPInsp_WireBonding");


	m_procWire = nullptr;
	//m_pInspFoot = new CPInsp_Foot();

	//m_pInspWedge	= new CPInsp_Wedge();
	m_pInspWedge = g_pMManager->pem_new<CPInsp_Wedge>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	//m_pInspRibbon	= new CPInsp_Ribbon();
	m_pInspRibbon = g_pMManager->pem_new<CPInsp_Ribbon>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	//m_pInspBall		= new CPInsp_Ball();
	m_pInspBall = g_pMManager->pem_new<CPInsp_Ball>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	//m_pInspCrescent = new CPInsp_Crescent();
	m_pInspCrescent = g_pMManager->pem_new<CPInsp_Crescent>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	m_vPInspFoot.clear();
	m_vPInspFoot.push_back(m_pInspWedge);
	m_vPInspFoot.push_back(m_pInspRibbon);
	m_vPInspFoot.push_back(m_pInspBall);
	m_vPInspFoot.push_back(m_pInspCrescent);
	m_vTupleWirePoint.clear();
	
	m_nFootType = 0;

	m_pWirePoint = NULL;
	m_WirepH2 = NULL;
}


CPInsp_WireBonding::~CPInsp_WireBonding()
{
	if (m_pInspWedge != NULL)
	{
		//delete m_pInspWedge;
		g_pMManager->pem_delete(m_pInspWedge, false);
		m_pInspWedge = NULL;
	}
	if (m_pInspRibbon != NULL)
	{
		//delete m_pInspRibbon;
		g_pMManager->pem_delete(m_pInspRibbon, false);
		m_pInspRibbon = NULL;
	}
	if (m_pInspBall != NULL)
	{
		//delete m_pInspBall;
		g_pMManager->pem_delete(m_pInspBall, false);
		m_pInspBall = NULL;
	}
	if (m_pInspCrescent != NULL)
	{
		//delete m_pInspCrescent;
		g_pMManager->pem_delete(m_pInspCrescent, false);
		m_pInspCrescent = NULL;
	}

	m_vPInspFoot.clear();

	if (m_procWire != nullptr)
	{
		m_procWire->FreeMil();

		//delete m_procWire;
		g_pMManager->pem_delete(m_procWire, false);
		m_procWire = nullptr;
	}


	CPInsp::CloseDevice();


	m_nPointNum = 0;
	if (m_pWirePoint)
	{
		//delete m_pWirePoint;
		g_pMManager->pem_delete(m_pWirePoint, false);
		m_pWirePoint = nullptr;
	}

	if (m_WirepH2)
	{
		//delete m_WirepH2;
		g_pMManager->pem_delete(m_WirepH2, false);
		m_WirepH2 = nullptr;
	}
}


int CPInsp_WireBonding::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	m_bUseImagePilLib = bUseImagePilLib;

	//Wire
	if(m_procWire == nullptr)
		//m_procWire = new CProc_Wire();
		m_procWire = g_pMManager->pem_new<CProc_Wire>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	else
		m_procWire->FreeMil();

	m_procWire->InitMil(milApp, milSys, bUseImagePilLib);
	m_procWire->SetResol(resolX, resolY, m_fovWidth);

	//pattern
// 	m_bUseImagePilLib = bUseImagePilLib;
// 
// 	if (bUseImagePilLib == true)
	m_pInspWedge->InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	m_pInspRibbon->InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	m_pInspBall->InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	m_pInspCrescent->InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());

	if (m_pProcMilAlgo)
		m_pProcMilAlgo->InitMilAlgoBlob();

	return ePART_SUCCESS;
}
void CPInsp_WireBonding::SetFootType(int nFootType)
{
	m_nFootType = nFootType;
}

bool CPInsp_WireBonding::LoadFootModelList(CString FolderPath)
{
	bool bResult = false;
	bResult &= m_vPInspFoot[0]->LoadFootModelList(FolderPath);
	bResult &= m_vPInspFoot[1]->LoadFootModelList(FolderPath);
	bResult &= m_vPInspFoot[2]->LoadFootModelList(FolderPath);
	bResult &= m_vPInspFoot[3]->LoadFootModelList(FolderPath);
	return bResult;
}
bool CPInsp_WireBonding::CheckModelList(CString FolderPath)
{
	bool bResult = false;
	bResult &= m_vPInspFoot[0]->CheckModelList(FolderPath);
	bResult &= m_vPInspFoot[1]->CheckModelList(FolderPath);
	bResult &= m_vPInspFoot[2]->CheckModelList(FolderPath);
	bResult &= m_vPInspFoot[3]->CheckModelList(FolderPath);
	return bResult;
}

CWire_Model* CPInsp_WireBonding::MakeWire(AlgoWire* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt)
{
	//CWire_Model* model = new CWire_Model();
	CWire_Model* model = g_pMManager->pem_new<CWire_Model>(false, 0, (PCHAR)__FUNCTION__, __LINE__);


	model->m_nWindowStartX = nStartX;
	model->m_nWindowStartY = nStartY;

	model->m_Width = sWndAlgoImg.m_nWidth3D;
	model->m_Height = sWndAlgoImg.m_nHeight3D;

	model->pf3DOrg = sWndAlgoImg.m_fArr3D;
	model->mImageRect = ptrAlgoColorOpt.m_rcImageRect;

	if (model->pf3DOrg == NULL)
	{
		//delete model;
		g_pMManager->pem_delete(model, false);

		return NULL;
	}

	//Nan 제거
	int nTotalSize = sWndAlgoImg.m_nHeight * sWndAlgoImg.m_nWidth;
	for (int r = 0; r < nTotalSize; r++)
	{
		if (isnan(model->pf3DOrg[r]) == true)
			model->pf3DOrg[r] = 0;
	}

	//Clip 
	cv::Rect ImageRect(model->mImageRect.left, model->mImageRect.top,
		model->mImageRect.right - model->mImageRect.left, model->mImageRect.bottom - model->mImageRect.top);

	int nWidth = ImageRect.width;
	int nHeight = ImageRect.height;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_W, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_G, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_B, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgMiddle_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgBottom_R, sizeof(UCHAR) * nWidth * nHeight);
	if (nWidth * nHeight < 1)
	{
		//delete model;
		g_pMManager->pem_delete(model, false);

		return NULL;
	}

	//TOP White
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_W, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_W, ImageRect.x, ImageRect.y, nWidth, nHeight);

	//Top Green
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_G, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_G, ImageRect.x, ImageRect.y, nWidth, nHeight);
	
	//Top Blue
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_B, ImageRect.x, ImageRect.y, nWidth, nHeight);

	//Top Red
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_R, ImageRect.x, ImageRect.y, nWidth, nHeight);

	//Middle Red
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgMiddle_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgMiddle_R, ImageRect.x, ImageRect.y, nWidth, nHeight);

	//Bottom Red
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgBottom_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgBottom_R, ImageRect.x, ImageRect.y, nWidth, nHeight);

	return model;

}


BOOL CPInsp_WireBonding::InspWire(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoWire * sRstAlgo, TotalInspExceptArea stTieArea,
	int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, POINTF* poArrInspFoot, int nSelectFoot, float fTeachH, UCHAR *ucArrDstImg, AlignResult * arrAlignRes, float fWireRefAreaHgt , float fWireRefAreaHgt_Sub)
{
	// 와이어 검사 부분
	// foot 검사 시, 얻은 st점/Ed점/Angle로 wire 트래킹
	//cv::Mat Dst(sWndAlgoImg.m_nHeight3D, sWndAlgoImg.m_nWidth3D, CV_8UC1, ucArrDstImg);
	int nLine = __LINE__;
	BOOL  bResult = TRUE;
	try
	{
		// 검사 구조체 결과로 넘기는 메모리 세팅
		// 검사 구조체 결과로 넘기는 메모리 세팅
		sRstAlgo->Init();
		if (sRstAlgo)
		{
			memset(sRstAlgo, 0, sizeof(RstAlgoWire));

			sRstAlgo->m_poWire_Wnd.x = nStartX;
			sRstAlgo->m_poWire_Wnd.y = nStartY;
			sRstAlgo->m_nWireCnt = 0;
			sRstAlgo->m_fRefArea_H = 0.0f;
			sRstAlgo->m_rcWire.left = 0;
			sRstAlgo->m_rcWire.right = 0;
			sRstAlgo->m_rcWire.top = 0;
			sRstAlgo->m_rcWire.bottom = 0;

			sRstAlgo->m_nArrCnt = 0;
			sRstAlgo->m_bArrNGType = 0;
			sRstAlgo->m_bArrHo = 0;

		}
		else
			return FALSE;
		//윈도우 정보 받아오기
		double dWndAngle = sWndAlgoImg.dAngle;
		UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
			return FALSE;

		if (sInspAlgo.m_eAlgoType != eAlgoWire)
			return FALSE;
		if (!m_pProcMilAlgo)
			return FALSE;

		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("WIRE_ORG_3D.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("WIRE_ORG_2D.bmp"));


		AlgoWire *pAlgo = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
		if (!pAlgo) return bResult;

		CWire_Model* pModel = MakeWire(pAlgo, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt);

		if (pModel == nullptr)
			return bResult;

		//기본 모드
		pAlgo->m_nArrData[eWIRE2_UseOption2] |= m_eWIRE_Data2_FootPos;

		// 와이어 자동 개수 측정 (기준 높이 입력 값 대비)
		//sRstAlgo->m_nWireCnt = pAlgo->m_byFootCnt;

		//if(pAlgo->m_byFootCnt <= 0)
		//sRstAlgo->m_nWireCnt = nSelectFoot;
		sRstAlgo->m_nWireCnt = 0;
		if (nSelectFoot == -1 || nSelectFoot == -2)
			nSelectFoot = pAlgo->m_byFootCnt;
		nLine = __LINE__;

		//기준 높이 값 측정
		if ((pAlgo->m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_RefArea) == m_eWIRE_Data2_RefArea)
		{
			RECT rcDstRect;
			if (CPInsp::AngleRectChange2(dWndAngle, nImgWidth, nImgHeight, pAlgo->m_sWireRect, &rcDstRect))
			{
				if (rcDstRect.left < 0) rcDstRect.left = 0;
				if (rcDstRect.left > nImgWidth) rcDstRect.left = nImgWidth - 1;
				if (rcDstRect.top < 0) rcDstRect.top = 0;
				if (rcDstRect.top > nImgHeight) rcDstRect.top = nImgHeight - 1;
				int nDstRectW = rcDstRect.right - rcDstRect.left;
				int nDstRectH = rcDstRect.bottom - rcDstRect.top;
				if (nDstRectW <= 0) nDstRectW = 1;
				if (nDstRectH <= 0) nDstRectH = 1;
				if (rcDstRect.left + nDstRectW > nImgWidth) nDstRectW = nImgWidth - rcDstRect.left - 1;
				if (rcDstRect.top + nDstRectH > nImgHeight) nDstRectH = nImgHeight - rcDstRect.top - 1;
				if (nDstRectW > 0 && nDstRectH > 0)
				{
					float BotSumH = 0;
					for (int r = rcDstRect.top; r < rcDstRect.top + nDstRectH; r++)
					{
						for (int c = rcDstRect.left; c < rcDstRect.left + nDstRectW; c++)
							BotSumH += sWndAlgoImg.m_fArr3D[r * nImgWidth + c];
					}
					sRstAlgo->m_fRefArea_H = BotSumH / (nDstRectH * nDstRectW);		//3D 높이 평균
					sRstAlgo->m_rcWire.left = rcDstRect.left;
					sRstAlgo->m_rcWire.top = rcDstRect.top;
					sRstAlgo->m_rcWire.right = rcDstRect.left + nDstRectW;
					sRstAlgo->m_rcWire.bottom = rcDstRect.top + nDstRectH;
				}
			}

			if (pAlgo->m_nWireRefAreaOpt == (int)m_eWire_RefAreaType::Foot1_pad || pAlgo->m_nWireRefAreaOpt == (int)m_eWire_RefAreaType::Foot2_pad)
			{
				if (nSelectFoot == -2) // Teaching 일때 
				{
					// 검사가 완료된 Foot 일경우 
					sRstAlgo->m_fRefArea_H = pAlgo->m_fWireRefAreaH;
				}
				else
				{
					sRstAlgo->m_fRefArea_H = fWireRefAreaHgt;
				}

			}
		}
		nLine = __LINE__;
		// Wire Inspection
		UCHAR *ucImgDst = NULL;
		UCHAR *ucImgInterval = NULL;
		int nArrWire;	// -1 :Missing / 0 : Rechecking / Label : Find
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImgDst, nImgWidth * nImgHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImgInterval, nImgWidth * nImgHeight);
		//memset(ucImgInterval, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		ippsSet_8u(0, ucImgInterval, nImgWidth * nImgHeight);
		nArrWire = 0;
		//memset(ucImgDst, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		ippsSet_8u(0, ucImgDst, nImgWidth * nImgHeight);

		{
			bool bHorizon = true;
			double dA_T = 0, dB_T = 0;
			ippsSet_8u(0, ucImgDst, nImgWidth * nImgHeight);
			nLine = __LINE__;
			nArrWire = FindWire(*pAlgo, sWndAlgoImg, ucImgInterval, sRstAlgo, 0, &bHorizon, &dA_T, &dB_T, ucImgDst, poArrInspFoot, pModel);
			
			if (nArrWire <= 0)
			{
				nArrWire = -1;
				
			}
			else
			{
				nLine = __LINE__;
				m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Wire_InspectionWire.bmp"));
				byte bRetInsp = InspectionWire(*pAlgo, pfImgSrc, 0, ucImgDst, nImgWidth, nImgHeight, bHorizon, dA_T, dB_T, sRstAlgo);
				if (bRetInsp == 0)
					nArrWire = 0;
				else
				{
					sRstAlgo->m_nWireCnt = nSelectFoot;

					if (bRetInsp == 2) nArrWire = -1;
					else if (nArrWire == 2 || bRetInsp == 3) nArrWire = -2;
					m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Wire_Insp.bmp"));
					for (int y = 0; y < nImgHeight; y++)
					{
						for (int x = 0; x < nImgWidth; x++)
						{
							int nIndex = y * nImgWidth + x;
							if (ucImgDst[nIndex] == 0) continue;
							if (ucArrDstImg && nSelectFoot >= 0)// && nSelectFoot >= 0 /*&& (nSelectFoot < 0 || nSelectFoot == 0)*/)
							{
								nIndex = ((y + nStartY) * sWndAlgoImg.m_nWidth3D + (x + nStartX));
								ucArrDstImg[nIndex] = 200 + (nSelectFoot/* * 10*/);  /*255*/;
							}
							if (ucImgInterval)
							{
								nIndex = y * nImgWidth + x;
								ucImgInterval[nIndex] = 200 + (nSelectFoot /* 10*/); //10;
							}
						}
					}
				}
			}
			
		}
		nLine = __LINE__;
		CString strLogArr;
		strLogArr.Format(_T("nArrWire Index : %d"), nArrWire);
		g_pMPTI->AddLog(strLogArr);

		//Short 검사 에서 진행
		//sRstAlgo->m_fArrRst[eWIRE_Interval] = InspInterval(*pAlgo, ucImgInterval, nImgWidth, nImgHeight, sRstAlgo);
		if (((pAlgo->m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_RefArea) == m_eWIRE_Data2_RefArea) && (nArrWire > 0))
		{
			if (pAlgo->m_nWireRefAreaOpt != 0)
			{
				cv::Mat cvWireInspImg(nImgHeight, nImgWidth, CV_8UC1, ucImgDst);
				cv::Mat cvWireInsp3D(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);

				size_t floatstep_Wire3DCorrect = cvWireInsp3D.step / sizeof(float);

				float fSumWireHst_dbc[eWire_Select_Max] = { 0.0f ,};
				int nWireHstPixelCnt[eWire_Select_Max] = { 0, };
				int nWireHstPosMinX[eWire_Select_Max];
				std::fill_n(nWireHstPosMinX, eWire_Select_Max, cvWireInspImg.cols);
				int nWireHstPosMaxX[eWire_Select_Max] = { 0, };
				int nWireHstPosY[eWire_Select_Max] = { 0, };
				int nWireHstPosX[eWire_Select_Max] = { 0, };
				int nWireHstPosMinY[eWire_Select_Max];
				std::fill_n(nWireHstPosMinY, eWire_Select_Max, cvWireInspImg.rows);
				int nWireHstPosMaxY[eWire_Select_Max] = { 0, };
				for (int r = 0; r < cvWireInspImg.rows; r++)
				{
					UCHAR* Ptr2D = cvWireInspImg.data;
					float* Ptr_Foot3D = (float*)cvWireInsp3D.data;

					for (int c = 0; c < cvWireInspImg.cols; c++)
					{
						if (Ptr2D[r*cvWireInspImg.step + c] != 0)
						{
							for (int nSelectCnt = eWire_Height_Select_1; nSelectCnt < eWire_Select_Max; nSelectCnt++)
							{
								if (pAlgo->m_bDirectionH)
								{
									if ((int)(nImgWidth / 2 + (pAlgo->m_sArrPoint[nSelectCnt].x)) == c)
									{
										fSumWireHst_dbc[nSelectCnt] += Ptr_Foot3D[r * floatstep_Wire3DCorrect + c];
										nWireHstPixelCnt[nSelectCnt]++;

										nWireHstPosX[nSelectCnt] = c;

										if (nWireHstPosMinY[nSelectCnt] > r)
											nWireHstPosMinY[nSelectCnt] = r;
										if (nWireHstPosMaxY[nSelectCnt] < r)
											nWireHstPosMaxY[nSelectCnt] = r;
									}
								}
								else
								{
									if ((int)(nImgHeight / 2 + (pAlgo->m_sArrPoint[nSelectCnt].y)) == r)
									{
										fSumWireHst_dbc[nSelectCnt] += Ptr_Foot3D[r * floatstep_Wire3DCorrect + c];
										nWireHstPixelCnt[nSelectCnt]++;

										nWireHstPosY[nSelectCnt] = r;

										if (nWireHstPosMinX[nSelectCnt] > c)
											nWireHstPosMinX[nSelectCnt] = c;
										if (nWireHstPosMaxX[nSelectCnt] < c)
											nWireHstPosMaxX[nSelectCnt] = c;
									}
								}
							}
						}
					}
				}

				int nExceptPixelCnt[eWire_Select_Max] = { 0 ,};
				for (int nSelectCnt = eWire_Height_Select_1; nSelectCnt < eWire_Select_Max; nSelectCnt++)
				{
					if (nWireHstPixelCnt[nSelectCnt] > 0)
						nExceptPixelCnt[nSelectCnt] = nWireHstPixelCnt[nSelectCnt] * 0.25;	//추후 샘플 상태에 따라 조정 할 수 있음

					if (nExceptPixelCnt[nSelectCnt] > 0)
					{
						nWireHstPixelCnt[nSelectCnt] = 0;
						fSumWireHst_dbc[nSelectCnt] = 0;
						for (int r = 0; r < cvWireInspImg.rows; r++)
						{
							UCHAR* Ptr2D = cvWireInspImg.data;
							float* Ptr_Foot3D = (float*)cvWireInsp3D.data;

							for (int c = 0; c < cvWireInspImg.cols; c++)
							{
								if (Ptr2D[r*cvWireInspImg.step + c] != 0)
								{
									if (pAlgo->m_bDirectionH)
									{
										if ((int)(nImgWidth / 2 + pAlgo->m_sArrPoint[nSelectCnt].x) == c)
										{
											if (r > nWireHstPosMinY[nSelectCnt] + nExceptPixelCnt[nSelectCnt] && r < nWireHstPosMaxY[nSelectCnt] - nExceptPixelCnt[nSelectCnt])
											{
												fSumWireHst_dbc[nSelectCnt] += Ptr_Foot3D[r * floatstep_Wire3DCorrect + c];
												nWireHstPixelCnt[nSelectCnt]++;
											}
										}
									}
									else
									{
										if ((int)(nImgHeight / 2 + pAlgo->m_sArrPoint[nSelectCnt].y) == r)
										{
											if (c > nWireHstPosMinX[nSelectCnt] + nExceptPixelCnt[nSelectCnt] && c < nWireHstPosMaxX[nSelectCnt] - nExceptPixelCnt[nSelectCnt])
											{
												fSumWireHst_dbc[nSelectCnt] += Ptr_Foot3D[r * floatstep_Wire3DCorrect + c];
												nWireHstPixelCnt[nSelectCnt]++;
											}
										}
									}

								}
							}
						}

					}

				}
				

				

				float fWireHstAvg[eWire_Select_Max] = { 0.0f ,};
				CString strLogH; // Wire Nan Chack Log

				for (int nSelectCnt = eWire_Height_Select_1; nSelectCnt < eWire_Select_Max; nSelectCnt++)
				{
					fWireHstAvg[nSelectCnt] = fSumWireHst_dbc[nSelectCnt] / (float)nWireHstPixelCnt[nSelectCnt];
					if (isnan(fWireHstAvg[nSelectCnt]))
						fWireHstAvg[nSelectCnt] = 0;
					strLogH.Format(_T("Wire fSumWireHst_dbc: %f , nWireHstPixelCnt: %d,fWireHstAvg: %f  sRstAlgo->m_fRefArea_H : %f fWireRefAreaHgt_Sub: %f "),
						fSumWireHst_dbc[nSelectCnt], nWireHstPixelCnt[nSelectCnt], fWireHstAvg[nSelectCnt], sRstAlgo->m_fRefArea_H, fWireRefAreaHgt_Sub);
						g_pMPTI->AddLog(strLogH);
				}
				
				
			
				sRstAlgo->m_fArrRst[eWIRE_Highest] = fWireHstAvg[eWire_Height_Select_1];
				sRstAlgo->m_fArrRst[eWIRE_Highest] -= sRstAlgo->m_fRefArea_H;
				sRstAlgo->m_fArrRst[eWIRE_HeightSub] = fWireHstAvg[eWire_Height_Select_2];
				sRstAlgo->m_fArrRst[eWIRE_HeightSub] -= fWireRefAreaHgt_Sub;

				//ASE Korea 김백현 요청으로 UI Offset 설정시 Result 의 값 빼주도록 설정
				sRstAlgo->m_fArrRst[eWIRE_HeightSub] -= pAlgo->m_fHeightSubOffset;
				if (pAlgo->m_bDirectionH)
				{
					sRstAlgo->m_nArrX[eDot_Highest] = nWireHstPosX[eWire_Height_Select_1];
					sRstAlgo->m_nArrY[eDot_Highest] = (int)((nWireHstPosMinY[eWire_Height_Select_1] + nWireHstPosMaxY[eWire_Height_Select_1]) / 2);
					sRstAlgo->m_nArrX[eDot_Height_Sub] = nWireHstPosX[eWire_Height_Select_2];
					sRstAlgo->m_nArrY[eDot_Height_Sub] = (int)((nWireHstPosMinY[eWire_Height_Select_2] + nWireHstPosMaxY[eWire_Height_Select_2]) / 2);
				}
				else
				{
					sRstAlgo->m_nArrX[eDot_Highest] = (int)((nWireHstPosMinX[eWire_Height_Select_1] + nWireHstPosMaxX[eWire_Height_Select_1]) / 2);
					sRstAlgo->m_nArrY[eDot_Highest] = nWireHstPosY[eWire_Height_Select_1];
					sRstAlgo->m_nArrX[eDot_Height_Sub] = (int)((nWireHstPosMinX[eWire_Height_Select_2] + nWireHstPosMaxX[eWire_Height_Select_2]) / 2);
					sRstAlgo->m_nArrY[eDot_Height_Sub] = nWireHstPosY[eWire_Height_Select_2];
				}


			}
			else
			{
				sRstAlgo->m_fArrRst[eWIRE_Highest] -= sRstAlgo->m_fRefArea_H;
				sRstAlgo->m_fArrRst[eWIRE_HeightSub] -= fWireRefAreaHgt_Sub;
			}

			sRstAlgo->m_fArrRst[eWIRE_Height2] -= sRstAlgo->m_fRefArea_H;
			for (int a = 0; a < WIRE_DOT_CNT; a++)
			{
				if (sRstAlgo->m_fArrValue_H[a] > 0)
					sRstAlgo->m_fArrValue_H[a] -= sRstAlgo->m_fRefArea_H;
				if (sRstAlgo->m_fArrValue_H2[a] > 0)
					sRstAlgo->m_fArrValue_H2[a] -= sRstAlgo->m_fRefArea_H;
			}
		}
		CString strLogArr2;
		strLogArr2.Format(_T("Wire DistanceRst : %f"), sRstAlgo->m_fArrRst[eWIRE_Distance]);
		g_pMPTI->AddLog(strLogArr2);
		//Result
		bResult = TRUE;
		sRstAlgo->m_bOK = TRUE;

		for (int a = 0; a < eWIRE_Total; a++)
		{
			if (a == eWIRE_Warp && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Warp) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			else if (a == eWIRE_Thickness && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Thickness) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			else if (a == eWIRE_Distance && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Distance) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			else if (a == eWIRE_Height && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Height) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			else if (a == eWIRE_Highest && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Highest) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			else if (a == eWIRE_HeightSub && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Height_Sub) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			else if (a == eWIRE_SteepSlope && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_SteepSlope) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			//else if (a == eWIRE_Interval && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Interval) == 0))
			else if (a == eWIRE_Height2 && ((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Height2) == 0))
			{
				sRstAlgo->m_bArrOK[a] = TRUE; continue;
			}
			if (a == eWIRE_Interval)
			{
				sRstAlgo->m_bArrOK[a] = sRstAlgo->m_bArrOK[0];
				continue;
			}
			sRstAlgo->m_bArrOK[a] = FALSE;
			sRstAlgo->m_bArrNGType = 0;
			if (nArrWire < 0)
			{
				sRstAlgo->m_bArrNGType = nArrWire * -1;
				sRstAlgo->m_bOK = FALSE;
				if (sRstAlgo->m_bArrNGType == 2 || sRstAlgo->m_bArrNGType == 1) //끊어짐일 경우 Data 0 
				{
					sRstAlgo->m_bArrOK[a] = FALSE;
					sRstAlgo->m_bOK = FALSE;
					bResult = FALSE;
					if (a == eWIRE_Distance)
						continue;
					sRstAlgo->m_fArrRst[a] = 0;
					continue;
				}


			}
			if (a == eWIRE_Warp || a == eWIRE_SteepSlope || a == eWIRE_Height || a == eWIRE_Interval ||
				a == eWIRE_Height2)
			{
				float fValue = sRstAlgo->m_fArrRst[a];
				if (fValue < 0) fValue *= -1;
				if (a == eWIRE_Interval || a == eWIRE_Height2)
				{
					if (fValue >= pAlgo->m_fArrOptionValue[a][eMMD_Default])
						sRstAlgo->m_bArrOK[a] = TRUE;
				}
				else
				{
					if (fValue < pAlgo->m_fArrOptionValue[a][eMMD_Default])
						sRstAlgo->m_bArrOK[a] = TRUE;
				}
			}
			else
			{
				float fMinValue = pAlgo->m_fArrOptionValue[a][eMMD_Min];
				float fMaxValue = pAlgo->m_fArrOptionValue[a][eMMD_Max];
				if (a == eWIRE_Distance)
				{
					fMinValue = pAlgo->m_fArrOptionValue[a][eMMD_Min];
					fMaxValue = pAlgo->m_fArrOptionValue[a][eMMD_Max];
				}
				if (sRstAlgo->m_fArrRst[a] >= fMinValue &&
					sRstAlgo->m_fArrRst[a] <= fMaxValue)
					sRstAlgo->m_bArrOK[a] = TRUE;
			}
			if (sRstAlgo->m_bArrOK[a] == FALSE)
			{
				sRstAlgo->m_bOK = FALSE;
				bResult = FALSE;
			}
		}

		if (ucArrDstImg && sWndAlgoImg.m_nWidth3D > 0 && sWndAlgoImg.m_nHeight3D > 0)
			m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D, _T("Wire_Blob_rst.bmp"));

		Delete_1DArray(&ucImgDst);

		Delete_1DArray(&ucImgInterval);

		if (pModel)
		{
			//delete pModel;
			g_pMManager->pem_delete(pModel, false);
			pModel = nullptr;
		}
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("PInsp_WireBonding Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return bResult;
	}
	return bResult;
}

BOOL CPInsp_WireBonding::InspWireShort(int nWireNum, WireRst* Wirerst, WndAlgoImg &sWndAlgoImg, UCHAR *ucArrDstImg, std::vector<std::vector<cv::Rect>> vFootRects)
{

	InspInterval(nWireNum, Wirerst, ucArrDstImg, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,vFootRects); // 변경 Interval
	for (size_t i = 0; i < nWireNum; i++)
	{
		AlgoWire* pAlgo = Wirerst[i].m_Wire;
		RstAlgoWire *sRstAlgo = Wirerst[i].m_RstWire;

		//좌표 보정

		for (size_t j = 0; j < 2; j++)
		{
			if (sRstAlgo->m_nArrX_I[j] == 0 && sRstAlgo->m_nArrY_I[j] == 0)
				continue;

			sRstAlgo->m_nArrX_I[j] -= sRstAlgo->m_poWire_Wnd.x;
			sRstAlgo->m_nArrY_I[j] -= sRstAlgo->m_poWire_Wnd.y;
		}
	

		if (((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Interval) == 0))
		{
			sRstAlgo->m_fArrRst[eWIRE_Interval] = -1.;
			sRstAlgo->m_bArrOK[eWIRE_Interval] = TRUE;
			continue;
		}

		sRstAlgo->m_bArrOK[eWIRE_Interval] = FALSE;

		if (sRstAlgo->m_nWireCnt == 0)
		{
			sRstAlgo->m_fArrRst[eWIRE_Interval] = 0.;
			continue;
		}
			

		if (sRstAlgo->m_fArrRst[eWIRE_Interval] == sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight)
		{
			//맨 오른쪽 Wire 검사시 
			sRstAlgo->m_fArrRst[eWIRE_Interval] = -1.;
			sRstAlgo->m_bArrOK[eWIRE_Interval] = TRUE;
			continue;
		}

		float fValue = sRstAlgo->m_fArrRst[eWIRE_Interval];
		if (fValue < 0) fValue *= -1;

		if (fValue >= pAlgo->m_fArrOptionValue[eWIRE_Interval][eMMD_Default])
			sRstAlgo->m_bArrOK[eWIRE_Interval] = TRUE;
		else
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bArrOK[eWIRE_Interval] = FALSE;
		}
	}

	return TRUE;
}

bool CPInsp_WireBonding::SearchWire(CSearchWire_Model* pModel, WireSearch& rst, int nSearchType)
{
	if (pModel == nullptr) return false;

	std::vector<FootRect> FindFoot1;
	std::vector<FootRect> FindFoot2;

	bool ret1 = FindFootROIList(pModel, 0, FindFoot1,nSearchType);
	bool ret2 = FindFootROIList(pModel, 1, FindFoot2, nSearchType);
		
	

	//하나라도 실패하면 실패
	if (!ret1 || !ret2)
		return false;

	//수량이 틀리면 실패
	if (FindFoot1.size() != FindFoot2.size())
		return false;
	
	//Wire Tracking??
	rst.WireCnt = (int)FindFoot1.size();
    
	for (size_t i = 0; i < (int)FindFoot1.size(); i++)
	{
		FootRect foot1 = FindFoot1[i];
		FootRect foot2 = FindFoot2[i];
		rst.m_FootPos[i].m_sArrFootPoint[0].x = (FLOAT)foot1.m_Point.x;
		rst.m_FootPos[i].m_sArrFootPoint[0].y = (FLOAT)foot1.m_Point.y;
		rst.m_FootPos[i].m_sArrPadRect[0] = foot1.m_PadRect;

		rst.m_FootPos[i].m_sArrFootPoint[1].x = (FLOAT)foot2.m_Point.x;
		rst.m_FootPos[i].m_sArrFootPoint[1].y = (FLOAT)foot2.m_Point.y;
		rst.m_FootPos[i].m_sArrPadRect[1] = foot2.m_PadRect;
	}

	return true;
}

int CPInsp_WireBonding::TeachWire(AlgoWire pAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoWire *sRstAlgo, float fTeachH)
{
	//wire 를 검색하는 함수

	int nRet = 0;
	double dWndAngle = sWndAlgoImg.dAngle;
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return nRet;

	float fHMin = 0.0f, fHMax = 0.0f;
	CPInsp::GetHeightMinMax(pfImgSrc, nImgWidth, nImgHeight, &fHMin, &fHMax);

	AlgoBlob algoBlob;
	algoBlob.m_bUseIPC = false;
	algoBlob.m_bInvertCheck = false;
	algoBlob.m_bFilterIsUse = false;
	algoBlob.m_nTypeSelectBlob = eSelectMix;
	algoBlob.m_bInsp2D = false;
	algoBlob.m_bInsp3D = true;
	algoBlob.m_nTypeRange3D = eTypeRangeUpper;
	algoBlob.m_dHeightRateMin = 0;
	algoBlob.m_dHeightRateMax = fTeachH;
	algoBlob.m_bFillHole = true;
	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;


	int nMinBlobArea = 4;
	double dCX = 0, dCY = 0, dArea = 0;
	CRect rcBlob(0, 0, 0, 0);
	int nCntBlob = 0;
	UCHAR *ucImgDst = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImgDst, nImgWidth * nImgHeight);

	//Wire 기준 높이 값 입력 시
	if (fTeachH > 0.0f)
		nCntBlob = CPInsp::BlobImageStruct(algoBlob, pucImgSrc, pfImgSrc, NULL, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucImgDst, stTieAreaNULL, TRUE);
	else
	{		//기준 높이값 미 입력시 내부에서 자동으로 기준 높이값 설정
		for (int a = 0; a < 5; a++)
		{
			algoBlob.m_dHeightRateMax = fHMax * (0.8 - (a * 0.1));
			memset(ucImgDst, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
			nCntBlob = CPInsp::BlobImageStruct(algoBlob, pucImgSrc, pfImgSrc, NULL, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucImgDst, stTieAreaNULL, TRUE);
			if (nCntBlob > 0) break;
		}
	}

	if (nCntBlob <= 0)
	{
		Delete_1DArray(&ucImgDst);
		return nRet;
	}

	// Define
	const int nMaxCnt = 100000;
	long pLebel[nMaxCnt];
	USHORT* LabelImage = NULL;
	double pLebel_Area[nMaxCnt];
	int pLebel_MaxIDX[nMaxCnt];
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &LabelImage, nImgWidth * nImgHeight);
	memset(pLebel, 0, sizeof(long) * nMaxCnt);
	memset(LabelImage, 0, sizeof(USHORT) * nImgWidth * nImgHeight);
	memset(pLebel_Area, 0, sizeof(double) * nMaxCnt);
	memset(pLebel_MaxIDX, 0, sizeof(int) * nMaxCnt);

	// Blob Max Index
	m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);
	for (int a = 0; a < nCntBlob; a++)
	{
		for (int b = 0; b < nImgWidth * nImgHeight; b++)
		{
			if (pLebel[a] == LabelImage[b])
				pLebel_Area[a]++;
		}
	}
	if (CPInsp::GetCorrectIDX(nCntBlob, pLebel_Area, pLebel_MaxIDX) == 0)
	{
		Delete_1DArray(&ucImgDst);
		Delete_1DArray(&LabelImage);
		return FALSE;
	}
	for (int a = 0; a < nCntBlob; a++)
	{
		if (a > 0 && pLebel_Area[pLebel_MaxIDX[0]] * 0.3 > pLebel_Area[pLebel_MaxIDX[a]])
			continue;
		CRect rcArrArea;
		long lLabel = pLebel[pLebel_MaxIDX[a]];
		CPInsp::GetLabelImage(lLabel, LabelImage, nImgWidth, nImgHeight, _T("Find_Label_2.bmp"));
		bool bIn = false;
		for (int x = 0; x < nImgWidth; x++)
		{
			for (int y = 0; y < nImgHeight; y++)
			{
				int nIndex = y * nImgWidth + x;
				if (LabelImage[nIndex] != lLabel)
					continue;
				rcArrArea.left = x;
				bIn = true;
				break;
			}
			if (bIn) break;
		}
		if (bIn == false) continue;
		bIn = false;
		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIndex = y * nImgWidth + x;
				if (LabelImage[nIndex] != lLabel)
					continue;
				rcArrArea.top = y;
				bIn = true;
				break;
			}
			if (bIn) break;
		}
		if (bIn == false) continue;
		bIn = false;
		for (int x = nImgWidth - 1; x >= 0; x--)
		{
			bIn = false;
			for (int y = nImgHeight - 1; y >= 0; y--)
			{
				int nIndex = y * nImgWidth + x;
				if (LabelImage[nIndex] != lLabel)
					continue;
				rcArrArea.right = x;
				bIn = true;
				break;
			}
			if (bIn) break;
		}
		if (bIn == false) continue;
		bIn = false;
		for (int y = nImgHeight - 1; y >= 0; y--)
		{
			bIn = false;
			for (int x = nImgWidth - 1; x >= 0; x--)
			{
				int nIndex = y * nImgWidth + x;
				if (LabelImage[nIndex] != lLabel)
					continue;
				rcArrArea.bottom = y;
				bIn = true;
				break;
			}
			if (bIn) break;
		}
		if (bIn == false) continue;

		bool bHorizon = true;
		float fHorizonX = (float)(rcArrArea.right - rcArrArea.left);
		float fHorizonY = (float)(rcArrArea.bottom - rcArrArea.top);
		if (fHorizonX < 0) fHorizonX *= -1.0f;
		if (fHorizonY < 0) fHorizonY *= -1.0f;
		if (fHorizonY > fHorizonX) bHorizon = false;

		int nSX = (bHorizon) ? rcArrArea.left : rcArrArea.top;
		int nEX = (bHorizon) ? rcArrArea.right : rcArrArea.bottom;
		int nSY = (bHorizon) ? rcArrArea.top : rcArrArea.left;
		int nEY = (bHorizon) ? rcArrArea.bottom : rcArrArea.right;
		int nW = (bHorizon) ? nImgWidth : nImgHeight;
		int nH = (bHorizon) ? nImgHeight : nImgWidth;

		int nRstY1 = -1;
		int nRstY2 = -1;
		if (bHorizon)
		{
			for (int x = nSX; x <= nEX; x++)
			{
				for (int y = 0; y <= nImgHeight; y++)
				{
					int nIndex = y * nImgWidth + x;
					if (LabelImage[nIndex] != lLabel)
						continue;
					nRstY1 = y;
					break;
				}
				if (nRstY1 > -1) break;
			}
			for (int x = nEX; x >= nSX; x--)
			{
				for (int y = 0; y <= nImgHeight; y++)
				{
					int nIndex = y * nImgWidth + x;
					if (LabelImage[nIndex] != lLabel)
						continue;
					nRstY2 = y;
					break;
				}
				if (nRstY2 > -1) break;
			}
		}
		else
		{
			for (int y = nSX; y <= nEX; y++)
			{
				for (int x = 0; x <= nImgWidth; x++)
				{
					int nIndex = y * nImgWidth + x;
					if (LabelImage[nIndex] != lLabel)
						continue;
					nRstY1 = x;
					break;
				}
				if (nRstY1 > -1) break;
			}
			for (int y = nEX; y >= nSX; y--)
			{
				for (int x = 0; x <= nImgWidth; x++)
				{
					int nIndex = y * nImgWidth + x;
					if (LabelImage[nIndex] != lLabel)
						continue;
					nRstY2 = x;
					break;
				}
				if (nRstY2 > -1) break;
			}
		}
		if (nRstY1 == -1 || nRstY2 == -2)
			continue;

		sRstAlgo->m_poWire_TS.x = (bHorizon) ? (FLOAT)rcArrArea.left : (FLOAT)nRstY1;
		sRstAlgo->m_poWire_TS.y = (bHorizon) ? (FLOAT)nRstY1 : (FLOAT)rcArrArea.top;
		sRstAlgo->m_poWire_TE.x = (bHorizon) ? (FLOAT)rcArrArea.right : (FLOAT)nRstY2;
		sRstAlgo->m_poWire_TE.y = (bHorizon) ? (FLOAT)nRstY2 : (FLOAT)rcArrArea.bottom;
		nRet++;
	}
	Delete_1DArray(&LabelImage);
	Delete_1DArray(&ucImgDst);

	return nRet;
}

int CPInsp_WireBonding::FindWire(AlgoWire sAlgo, WndAlgoImg &sWndAlgoImg, UCHAR *ucALLImage, RstAlgoWire *sRstAlgo, int nRetry,
	bool *bHorizon, double *dA_T, double *dB_T, UCHAR *ucImgDst, POINTF* poArrInspFoot, CWire_Model* pModel)
{
	int nRet = 0;

	double dAngle = sWndAlgoImg.dAngle;
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return nRet;


	float *pfImgBuf = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pfImgBuf, nImgWidth * nImgHeight);
	memset(pfImgBuf, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
	for (int y = 0; y < nImgHeight; y++)
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			int nIndex = y * nImgWidth + x;
			if (ucALLImage[nIndex] == 0)
				pfImgBuf[nIndex] = pfImgSrc[nIndex];
			else
				pfImgBuf[nIndex] = -99999.0f;
		}
	}
	m_pProcMilAlgo->SaveWorkImg_float(pfImgBuf, nImgWidth, nImgHeight, _T("WIRE_ORG_3D.bmp"));

	int nArrFoot[WIRE_DRAW_CNT];
	POINT poArrFoot[WIRE_DRAW_CNT];
	cv::Rect rcArrROI[WIRE_DRAW_CNT];
	memset(poArrFoot, 0, sizeof(POINT) * WIRE_DRAW_CNT);
	memset(rcArrROI, 0, sizeof(cv::Rect) * WIRE_DRAW_CNT);

	for (int nDot = 0; nDot < WIRE_DRAW_CNT; nDot++)
	{
		nArrFoot[nDot] = -1;

		if ((nDot == 0 && CPInsp::AnglePosChange((int)dAngle, nImgWidth, nImgHeight, sAlgo.m_sArrPoint[nDot], &sRstAlgo->m_poWire_TS) == FALSE) ||
			(nDot == 1 && CPInsp::AnglePosChange((int)dAngle, nImgWidth, nImgHeight, sAlgo.m_sArrPoint[nDot], &sRstAlgo->m_poWire_TE) == FALSE))
		{
			Delete_1DArray(&pfImgBuf);
			return nRet;
		}

		int nStartTX = (nDot == 0) ? (int)sRstAlgo->m_poWire_TS.x : (int)sRstAlgo->m_poWire_TE.x;
		int nStartTY = (nDot == 0) ? (int)sRstAlgo->m_poWire_TS.y : (int)sRstAlgo->m_poWire_TE.y;
		int nFindFoot = 1;
		if (/*nRetry < 2 &&*/ poArrInspFoot != NULL &&
			((sAlgo.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_FootPos) == m_eWIRE_Data2_FootPos))
		{
			CString strLog1;
			strLog1.Format(_T("Foot[%d].x : %d,Foot[%d].y : %d Width : %d Height :%d "), nDot, (LONG)poArrInspFoot[nDot].x, nDot, (LONG)poArrInspFoot[nDot].y, nImgWidth, nImgHeight);
			g_pMPTI->AddLog(strLog1);

			if((((LONG)poArrInspFoot[nDot].x > 0 && nImgWidth > (LONG)poArrInspFoot[nDot].x)
				&& ((LONG)poArrInspFoot[nDot].y > 0 && nImgHeight > (LONG)poArrInspFoot[nDot].y)))
			{
				poArrFoot[nDot].x = (LONG)poArrInspFoot[nDot].x;
				poArrFoot[nDot].y = (LONG)poArrInspFoot[nDot].y;
				nStartTX = poArrFoot[nDot].x;
				nStartTY = poArrFoot[nDot].y;
				nArrFoot[nDot] = nFindFoot;
				rcArrROI[nDot] = cv::Rect(nStartTX - 50, nStartTY - 50, 100, 100);
			}
			else
			{
				poArrFoot[nDot].x = nStartTX;
				poArrFoot[nDot].y = nStartTY;

				nArrFoot[nDot] = nFindFoot;
				rcArrROI[nDot] = cv::Rect(nStartTX - 50, nStartTY - 50, 100, 100);
			}

			


		}
		else
		{
			poArrFoot[nDot].x = nStartTX;
			poArrFoot[nDot].y = nStartTY;

			nArrFoot[nDot] = nFindFoot;
			rcArrROI[nDot] = cv::Rect(nStartTX-50, nStartTY-50, 100, 100);
		}

	}

	for (int a = 0; a < WIRE_DRAW_CNT; a++)
	{
		if (rcArrROI[a].width == 0 || rcArrROI[a].height == 0)
		{
			if (nRetry < 2)
			{
				Delete_1DArray(&pfImgBuf);
				return nRet;
			}
			else
			{
				if (rcArrROI[a].width == 0) rcArrROI[a].width = 4;
				if (rcArrROI[a].height == 0) rcArrROI[a].height = 4;
			}
		}
		if (rcArrROI[a].x + rcArrROI[a].width >= nImgWidth || rcArrROI[a].y + rcArrROI[a].height >= nImgHeight ||
			rcArrROI[a].x <= 0 || rcArrROI[a].x <= 0 || rcArrROI[a].width <= 0 || rcArrROI[a].height <= 0)
		{
			Delete_1DArray(&pfImgBuf);
			return nRet;
		}
	}
	*bHorizon = true;
	float fHorizonX = sRstAlgo->m_poWire_TS.x - sRstAlgo->m_poWire_TE.x;
	float fHorizonY = sRstAlgo->m_poWire_TS.y - sRstAlgo->m_poWire_TE.y;
	if (fHorizonX < 0) fHorizonX *= -1.0f;
	if (fHorizonY < 0) fHorizonY *= -1.0f;
	if (fHorizonY > fHorizonX) *bHorizon = false;

	//상기 부분에서 Wire의 시작과 끝점을 찾은 뒤, 하기 부분에서 FloodFill 함수를 이용하여 1차 Wire 찾기
	cv::Mat src(nImgHeight, nImgWidth, CV_32FC1, pfImgBuf);

	cv::Mat dst;
	//float fDiffHigh = 40;//15;//40;
	//float fDiffLow = 40;// 15;//40;
	//float fDiameterWire = 120.0f;
	float fResol = (*bHorizon) ? (float)m_resolY : (float)m_resolX;
	bool bStartPos = ((sAlgo.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_StartPointHeightSearch) == m_eWIRE_Data2_StartPointHeightSearch);
	BinarizeByFloodFill(src, dst, rcArrROI[0], rcArrROI[1], m_fDiffHigh, m_fDiffLow, m_fDiameterWire, pModel, bStartPos);		//3D ¿µ»ó »ç¿ë


	double dArrLineX[WIRE_DRAW_CNT], dArrLineY[WIRE_DRAW_CNT];
	memset(dArrLineX, 0, sizeof(double) * WIRE_DRAW_CNT);
	memset(dArrLineY, 0, sizeof(double) * WIRE_DRAW_CNT);
//	bool bHorizon = true;
	double dA, dB;
	dArrLineX[0] = poArrFoot[0].x;
	dArrLineY[0] = poArrFoot[0].y;
	dArrLineX[1] = poArrFoot[1].x;
	dArrLineY[1] = poArrFoot[1].y;
	CPInsp::GetGradient(dArrLineX, dArrLineY, (double)WIRE_DRAW_CNT, dA, dB, bHorizon);
	double dMax;
	std::vector<cv::Point> ptCenterTh;
	InspectionDeleteWireTh(dst.data, dst.size().width, dst.size().height, bHorizon, dA, dB, &dMax , ptCenterTh);

	if (!bStartPos)
	{
		int nCnts = 0;
		if ((sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] / fResol) * 5 < dMax) // 최소 (Teaching)두께가 5배 이상일경우에만.
		{
			for (int j = 0; j < 2; j++)
			{
				for (int i = 0; i < ptCenterTh.size(); i++)
				{
					int nIndex = ptCenterTh[i].y * dst.size().width + ptCenterTh[i].x;
					if (dst.data[nIndex] == 0 && j == 0)
						nCnts++;
					if (j == 1 && (double)((double)nCnts / (double)ptCenterTh.size()) > 0.5) // 절반이상 Black Pixel 일 경우 
					{
						if (i > (ptCenterTh.size() / 10) && i < (ptCenterTh.size() - ptCenterTh.size() / 10)) // 앞뒤 10% Foot 관련 제외.
							cv::line(dst, ptCenterTh[i - 1], ptCenterTh[i], cv::Scalar(0), 3);
					}

				}
			}
			std::vector<CRect> rcRect;
			UCHAR *pucBlob = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucBlob, nImgWidth * nImgHeight);
			memset(pucBlob, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
			int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(dst.data, pucBlob, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix);
			Delete_1DArray(&pucBlob);
			CheckWire2(dst.data, sWndAlgoImg, nArrFoot, poArrFoot);

		}


	}

	memcpy(ucImgDst, dst.data, sizeof(UCHAR)*nImgWidth*nImgHeight);
	int nTrackWireCnt = -1 ;
	if (!bStartPos)
		nTrackWireCnt = m_pProcMilAlgo->CalcBlob_Select(ucImgDst, ucImgDst, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix);

	cv::Mat srcTest(nImgHeight, nImgWidth, CV_8UC1, ucImgDst);
	
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_3D_Dst.bmp"));


	InterpolateWire3D(sAlgo, sWndAlgoImg, ucImgDst, *bHorizon, poArrFoot, pModel);
	//1차 Wire에서 끊어짐이 발생 될 경우 2D를 이용하여 연결
	//양 끝 단을 확인 한 후 진행방향에 따라 이어질 수 있는지 확인 

	//BOOL bWireC = FALSE;
	BOOL bWireC = BinarizeByFloodFill(sAlgo, sWndAlgoImg, ucImgDst, sRstAlgo, 0, *bHorizon, poArrFoot);
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst1.bmp"));

	RemoveThin(ucImgDst, nImgWidth, nImgHeight, *bHorizon, (sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] * 0.5 / fResol));	//µÎ²² ¾ãÀº ºÎºÐ Á¦°Å
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst2.bmp"));

	//if (bWireC == TRUE && (nArrFoot[0] > -1 || nArrFoot[1] > -1) && nRetry < 1)
	//{
	//	Delete_1DArray(&pfImgBuf);
	//	return nRet;
	//}

	//Wire가 붙어 있을 경우 Wire를 분리 한 후 기준 점과 가까운 Wire를 선택 
	if (FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo, *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == TRUE &&
		CheckWire(sAlgo, sWndAlgoImg, ucImgDst, sRstAlgo, *bHorizon, *dA_T, *dB_T, (nRetry >= 2)) == TRUE &&
		FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo, *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == TRUE)
	{
		m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst3.bmp"));
		int nW = (*bHorizon) ? nImgWidth : nImgHeight;
		int nH = (*bHorizon) ? nImgHeight : nImgWidth;
		double dMaxTh = 0.0;
		double dAvgTh = InspectionWireTh(ucImgDst, nImgWidth, nImgHeight, *bHorizon, *dA_T, *dB_T, &dMaxTh);
		float fTh = sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Max] / fResol;
		if (sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Max] > sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Default] * 3.0)
			fTh = sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Default] / fResol;
		double dMax = (nRetry > 0) ? fTh * (2.0 + (nRetry * 0.3)) : fTh * 2.0;

		if (dAvgTh > 0 && dMaxTh > 0 && dMax >= dMaxTh)
			nRet = 1;
		else if (dAvgTh > 0 && dMaxTh > 0 && dMaxTh >= dMax)
		{
			if (CheckWire(sAlgo, sWndAlgoImg, ucImgDst, sRstAlgo, *bHorizon, *dA_T, *dB_T, true) == TRUE &&
				FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo, *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == TRUE)
			{
				m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst4.bmp"));
				dMaxTh = 0.0;
				CheckWire2(ucImgDst, sWndAlgoImg, nArrFoot, poArrFoot);
				dAvgTh = InspectionWireTh(ucImgDst, nImgWidth, nImgHeight, *bHorizon, *dA_T, *dB_T, &dMaxTh);
				dMax = (nRetry > 0) ? fTh * (2.0 + (nRetry * 0.4)) : fTh * 2.0;
				if (dAvgTh > 0 && dMaxTh > 0 && dMax >= dMaxTh)
					nRet = 1;
			}
		}

		if (nRet == 1)
		{
			//Wire 두께가 실제 두께보다 얇게 찾아진 경우 보완
			//BinarizeAddData(pfImgBuf, pucImgSrc, ucImgDst, nImgWidth, nImgHeight, *bHorizon);
			m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst5.bmp"));
			if (bWireC == FALSE && (nArrFoot[0] > -1 || nArrFoot[1] > -1)/* && nRetry < 1*/)
			{
				//Wire가 실제로는 끊어졌으나 찾은 결과가 서로 붙어있는 경우 보완
				BinarizeDelData(ucImgDst, nImgWidth, nImgHeight, poArrFoot, *bHorizon, *dA_T, *dB_T, 0/*50*/);
				m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst6.bmp"));
			}
			
			bWireC = m_pProcMilAlgo->CalcBlob_Select(ucImgDst, ucImgDst, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix) > 1;
			
			if (bStartPos)
			{
				if (bWireC)
					CheckWire2(ucImgDst, sWndAlgoImg, nArrFoot, poArrFoot);
				bWireC = m_pProcMilAlgo->CalcBlob_Select(ucImgDst, ucImgDst, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix) > 1;
			}
			else
			{
				if (bWireC == 1 && nTrackWireCnt == 1)// Tracking 시 Wire가 한개였으나, 자르거나 3D로인한 복원으로인해 갈라질경우예외처리
				{
					std::vector<double> vArea;
					m_pProcMilAlgo->GetBlobResult_Area(vArea);
					int nMaxBlobIndex = max_element(vArea.begin(), vArea.end()) - vArea.begin();
					memset(ucImgDst, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
					const int nMaxCnt = 100000;
					long pLebel[nMaxCnt];
					USHORT* LabelImage = NULL;
					Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &LabelImage, nImgWidth * nImgHeight);
					memset(pLebel, 0, sizeof(long) * nMaxCnt);
					memset(LabelImage, 0, sizeof(USHORT) * nImgWidth * nImgHeight);
					m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);

					for (int y = 0; y < nImgHeight; y++)
					{
						for (int x = 0; x < nImgWidth; x++)
						{
							int nIndex = y * nImgWidth + x;
							if (pLebel[nMaxBlobIndex] != LabelImage[nIndex])
								continue;

							if (pLebel[nMaxBlobIndex] == LabelImage[nIndex])
								ucImgDst[nIndex] = 255;
						}
					}

					Delete_1DArray(&LabelImage);

					bWireC = 0;
				}
			}

			nRet = (bWireC) ? 2 : 1;
			if (FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo, *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == FALSE)
				nRet = 0;
			m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst7.bmp"));
		}
	}
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst_Rst.bmp"));
	Delete_1DArray(&pfImgBuf);
	return nRet;
}
/*
int CPInsp_WireBonding::FindWire(AlgoWire sAlgo, WndAlgoImg &sWndAlgoImg, UCHAR *ucALLImage, RstAlgoWire *sRstAlgo, int nSelectFoot, int nWire,
	bool *bHorizon, double *dA_T, double *dB_T, UCHAR *ucImgDst, POINTF* poArrInspFoot)
{
	int nRet = 0;

	double dAngle = sWndAlgoImg.dAngle;
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return nRet;

	float *pfImgBuf = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pfImgBuf, nImgWidth * nImgHeight);
	memset(pfImgBuf, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
	for (int y = 0; y < nImgHeight; y++)
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			int nIndex = y * nImgWidth + x;
			if (ucALLImage[nIndex] == 0)
				pfImgBuf[nIndex] = pfImgSrc[nIndex];
			else
				pfImgBuf[nIndex] = -99999.0f;
		}
	}
	m_pProcMilAlgo->SaveWorkImg_float(pfImgBuf, nImgWidth, nImgHeight, _T("WIRE_ORG_3D.bmp"));

	int nArrFoot[WIRE_DRAW_CNT];
	POINT poArrFoot[WIRE_DRAW_CNT];
	cv::Rect rcArrROI[WIRE_DRAW_CNT];
	memset(poArrFoot, 0, sizeof(POINT) * WIRE_DRAW_CNT);
	memset(rcArrROI, 0, sizeof(cv::Rect) * WIRE_DRAW_CNT);

	//Wire 기준 점과 Shift 영역을 이용하여 시작 / 끝점을 찾음
	int nSearchSizeX = sAlgo.m_fArrOptionValue[eWIRE_ShiftX][eMMD_Default] / m_resolX;
	int nSearchSizeY = sAlgo.m_fArrOptionValue[eWIRE_ShiftY][eMMD_Default] / m_resolY;
	if (dAngle == 90 || dAngle == 270)
	{
		nSearchSizeY = sAlgo.m_fArrOptionValue[eWIRE_ShiftX][eMMD_Default] / m_resolX;
		nSearchSizeX = sAlgo.m_fArrOptionValue[eWIRE_ShiftY][eMMD_Default] / m_resolY;
	}
	if (nSelectFoot > 0)
	{
		nSearchSizeX += (nSearchSizeX + (nSelectFoot * 0.5));
		nSearchSizeY += (nSearchSizeY + (nSelectFoot * 0.5));
	}
	if (nSearchSizeX <= 10) nSearchSizeX = 10;
	if (nSearchSizeY <= 10) nSearchSizeY = 10;
	if (nSearchSizeX > nImgWidth) nSearchSizeX = nImgWidth;
	if (nSearchSizeY > nImgHeight) nSearchSizeY = nImgHeight;

	for (int nDot = 0; nDot < WIRE_DRAW_CNT; nDot++)
	{
		nArrFoot[nDot] = -1;
		if (nSelectFoot != -2)
		{
			if ((nDot == 0 && CPInsp::AnglePosChange(dAngle, nImgWidth, nImgHeight, sAlgo.m_sArrPoint[nDot], &sRstAlgo->m_poWire_TS) == FALSE) ||
				(nDot == 1 && CPInsp::AnglePosChange(dAngle, nImgWidth, nImgHeight, sAlgo.m_sArrPoint[nDot], &sRstAlgo->m_poWire_TE) == FALSE))
			{
				Delete_1DArray(&pfImgBuf);
				return nRet;
			}
		}
		int nStartTX = (nDot == 0) ? sRstAlgo->m_poWire_TS.x : sRstAlgo->m_poWire_TE.x;
		int nStartTY = (nDot == 0) ? sRstAlgo->m_poWire_TS.y : sRstAlgo->m_poWire_TE.y;
		double dDistMin = -111111;
		int nFindFoot = sAlgo.m_byFootCnt - (nWire + 1);
		if (nSelectFoot != -2 && nFindFoot > 0 && nSelectFoot < 2 && poArrInspFoot != NULL &&
			((sAlgo.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_FootPos) == m_eWIRE_Data2_FootPos))
		{
			poArrFoot[nDot].x = poArrInspFoot[nDot].x;
			poArrFoot[nDot].y = poArrInspFoot[nDot].y;
			double dDist1 = sqrt(pow((nStartTX - poArrFoot[nDot].x), 2) + pow((nStartTY - poArrFoot[nDot].y), 2));
			if (dDist1 < nSearchSizeX * 2 && dDist1 < nSearchSizeY * 2)
				dDistMin = dDist1;
		}

		//10회 정도 Wire 끝단 찾기 시도
		for (int a = 0; a < 10; a++)
		{
			if (nSelectFoot != -2 && dDistMin > -111111 && nSelectFoot < 2)
			{
				nStartTX = poArrFoot[nDot].x;
				nStartTY = poArrFoot[nDot].y;
				nArrFoot[nDot] = nFindFoot;
				rcArrROI[nDot] = cv::Rect(nStartTX, nStartTY, 5, 5);
				break;
			}
			int nW_Dot = nSearchSizeX + (nSearchSizeX / 2 * a);
			int nH_Dot = nSearchSizeY + (nSearchSizeY / 2 * a);
			if (nW_Dot <= 10) nW_Dot = 10;
			if (nH_Dot <= 10) nH_Dot = 10;
			if (nW_Dot > nImgWidth) nW_Dot = nImgWidth;
			if (nH_Dot > nImgHeight) nH_Dot = nImgHeight;
			int nSX_Dot = nStartTX - (nW_Dot / 2);
			int nSY_Dot = nStartTY - (nH_Dot / 2);
			if (nSX_Dot < 0) nSX_Dot = 0;
			if (nSY_Dot < 0) nSY_Dot = 0;
			if (nSX_Dot > nImgWidth || nSY_Dot > nImgHeight)
			{
				Delete_1DArray(&pfImgBuf);
				return nRet;
			}
			if (nSX_Dot + nW_Dot >= nImgWidth) nW_Dot = nImgWidth - nSX_Dot - 1;
			if (nSY_Dot + nH_Dot >= nImgHeight) nH_Dot = nImgHeight - nSY_Dot - 1;
			if (nW_Dot <= 0 || nH_Dot <= 0)
			{
				Delete_1DArray(&pfImgBuf);
				return nRet;
			}

			float *pfClip = NULL;
			UCHAR *pucBin = NULL;
			UCHAR *pucBlob = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucBin, nW_Dot * nH_Dot);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucBlob, nW_Dot * nH_Dot);
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pfClip, nW_Dot * nH_Dot);
			memset(pfClip, 0, sizeof(float) * nW_Dot * nH_Dot);
			m_proc3d.GetCropZmap_LT(pfImgBuf, pfClip, nImgWidth, nImgHeight, nSX_Dot, nSY_Dot, nW_Dot, nH_Dot);
			m_pProcMilAlgo->SaveWorkImg_float(pfClip, nW_Dot, nH_Dot, _T("WIRE_Clip_3D.bmp"));

			//검색 범위의 최대 높이를 구함
			float fMin = 0.0f, fMax = 0.0f, fClipSum = 0.0f;
			int nHCnt = 0;
			CPInsp::GetHeightMinMax(pfClip, nW_Dot, nH_Dot, &fMin, &fMax);

			for (int b = 0; b < nW_Dot * nH_Dot; b++)
			{
				if (pfClip[b] <= -500.0f || pfClip[b] >= fMax) continue;
				nHCnt++;
				fClipSum += pfClip[b];
			}
			float fClipHAvg = fClipSum / nHCnt;
			fClipSum = 0.0f; nHCnt = 0;
			for (int b = 0; b < nW_Dot * nH_Dot; b++)
			{
				if (pfClip[b] < fClipHAvg) continue;
				nHCnt++;
				fClipSum += pfClip[b];
			}
			fClipHAvg = fClipSum / nHCnt;

			int nCntBlob = 0;
			for (int b = 0; b < 10; b++)
			{
				float fClipMaxH = fClipHAvg * (2.0f - (0.1f * (float)b));

				//Foot이 아닌 경우 3D 높이 값 줄여가며 재 판단
				if (fClipMaxH >= fMax)
				{
					if (fMax > 0)
						fClipMaxH = fMax * (0.9f - (0.1f * (float)b));
					else
						fClipMaxH = fMax * (1.1f - (0.1f * (float)b));
				}
				if (fClipHAvg == 0)
					fClipMaxH = fClipHAvg - (50.0f * (b + 1));

				if (fClipMaxH >= 0) fClipMaxH -= 40.0f;
				else fClipMaxH += 40.0f;

				//높이를 기준으로 Blob을 진행하여 Foot 여부를 판단
				memset(pucBin, 0, sizeof(UCHAR) * nW_Dot * nH_Dot);
				memset(pucBlob, 0, sizeof(UCHAR) * nW_Dot * nH_Dot);
				CPInsp::Binarize(pfClip, nW_Dot, nH_Dot, eTypeRangeUpper, 0, fClipMaxH, false, pucBin);
				nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pucBin, pucBlob, nW_Dot, nH_Dot, 4, FALSE, TRUE, 0, eSelectMix);
				m_pProcMilAlgo->SaveWorkImg(pucBin, nW_Dot, nH_Dot, _T("WIRE_Clip_BW.bmp"));
				m_pProcMilAlgo->SaveWorkImg(pucBlob, nW_Dot, nH_Dot, _T("WIRE_Clip_BLOB.bmp"));

				if (nCntBlob > 0)
				{
					int nTotalCnt = nCntBlob;
					if (nTotalCnt > 3) nTotalCnt = 3;
					nCntBlob = 0;
					int nLim = sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] / m_resolX;
					if (nLim < 20) nLim = 20;
					int nBlobArea = 0, nBlobW = 0, nBlobH = 0, nBlobW2 = 0, nBlobH2 = 0;
					for (int y = 0; y < nH_Dot; y++)
					{
						int nS = -1, nE = 0;
						int nES = 0;
						for (int x = 0; x < nW_Dot; x++)
						{
							int nIndex = y * nW_Dot + x;
							if (pucBlob[nIndex] == 0) continue;
							nBlobArea++;
							if (nS == -1) nS = x;
							nE = x;
							nES++;
						}
						if (nS == -1) continue;
						if (nE - nS > nBlobW) nBlobW = nE - nS;
						if (nES > nBlobW2) nBlobW2 = nES;
					}
					for (int x = 0; x < nW_Dot; x++)
					{
						int nS = -1, nE = 0;
						int nES = 0;
						for (int y = 0; y < nH_Dot; y++)
						{
							int nIndex = y * nW_Dot + x;
							if (pucBlob[nIndex] == 0) continue;
							if (nS == -1) nS = y;
							nE = y;
							nES++;
						}
						if (nS == -1) continue;
						if (nE - nS > nBlobH) nBlobH = nE - nS;
						if (nES > nBlobH2) nBlobH2 = nES;
					}

					//Foot 여부 판정은 Blob의 면적과 Size로 판정함
					if (nBlobArea == 0 || nBlobArea > (nW_Dot * nH_Dot) * 0.6 || nBlobW2 > nW_Dot * 0.8 || nBlobH2 > nH_Dot * 0.8)
					{
						nCntBlob = -1;
						break;
					}
					else if (nBlobW <= nLim || nBlobH <= nLim)
						continue;

					//Foot 판정 결과 해당 영역이 Foot이 맞을 경우 아래 부분 실행
					double dArrArea[3], dArrCX[3], dArrCY[3];
					CRect rcArrBlob[3];
					memset(dArrArea, 0, sizeof(double) * 3);
					memset(dArrCX, 0, sizeof(double) * 3);
					memset(dArrCY, 0, sizeof(double) * 3);
					memset(rcArrBlob, 0, sizeof(CRect) * 3);
					m_pProcMilAlgo->GetBlobResult(dArrArea, dArrCX, dArrCY, rcArrBlob, true);
					int nIdx = -1;
					dDistMin = -111111;
					
					for (int b = 0; b < nTotalCnt; b++)
					{
						if (rcArrBlob[b].Width() <= nLim || rcArrBlob[b].Height() <= nLim)
							continue;
						double dDist1 = sqrt(pow((nStartTX - (nSX_Dot + dArrCX[b])), 2) + pow((nStartTY - (nSY_Dot + dArrCY[b])), 2));
						if (dDistMin == -111111 || dDistMin > dDist1)
						{
							dDistMin = dDist1;
							nIdx = b;
						}
					}
					if (nIdx < 0) break;

					nSX_Dot += (dArrCX[nIdx] - 2);
					nSY_Dot += (dArrCY[nIdx] - 2);
					nW_Dot = 4;
					nH_Dot = 4;
					nCntBlob = 1;
					break;
				}
				if (nCntBlob > 0 || nCntBlob == -1) break;
			}
			Delete_1DArray(&pfClip);
			Delete_1DArray(&pucBin);
			Delete_1DArray(&pucBlob);

			if (nCntBlob > 0)
			{
				rcArrROI[nDot] = cv::Rect(nSX_Dot, nSY_Dot, nW_Dot, nH_Dot);
				poArrFoot[nDot].x = nSX_Dot;
				poArrFoot[nDot].y = nSY_Dot;
				break;
			}
			else
				rcArrROI[nDot] = cv::Rect(nStartTX, nStartTY, 0, 0);
		}
	}
	for (int a = 0; a < WIRE_DRAW_CNT; a++)
	{
		if (rcArrROI[a].width == 0 || rcArrROI[a].height == 0)
		{
			if (nSelectFoot != -2 && nSelectFoot < 2)
			{
				Delete_1DArray(&pfImgBuf);
				return nRet;
			}
			else
			{
				if (rcArrROI[a].width == 0) rcArrROI[a].width = 4;
				if (rcArrROI[a].height == 0) rcArrROI[a].height = 4;
			}
		}
		if (rcArrROI[a].x + rcArrROI[a].width >= nImgWidth || rcArrROI[a].y + rcArrROI[a].height >= nImgHeight ||
			rcArrROI[a].x <= 0 || rcArrROI[a].x <= 0 || rcArrROI[a].width <= 0 || rcArrROI[a].height <= 0)
		{
			Delete_1DArray(&pfImgBuf);
			return nRet;
		}
	}
	*bHorizon = true;
	float fHorizonX = sRstAlgo->m_poWire_TS.x - sRstAlgo->m_poWire_TE.x;
	float fHorizonY = sRstAlgo->m_poWire_TS.y - sRstAlgo->m_poWire_TE.y;
	if (fHorizonX < 0) fHorizonX *= -1.0f;
	if (fHorizonY < 0) fHorizonY *= -1.0f;
	if (fHorizonY > fHorizonX) *bHorizon = false;

	//상기 부분에서 Wire의 시작과 끝점을 찾은 뒤, 하기 부분에서 FloodFill 함수를 이용하여 1차 Wire 찾기
	cv::Mat src(nImgHeight, nImgWidth, CV_32FC1, pfImgBuf);
	cv::Mat dst;
	float fDiffHigh = 40;
	float fDiffLow = 40;
	float fDiameterWire = 120.0f;
	float fResol = (*bHorizon) ? m_resolY : m_resolX;

	BinarizeByFloodFill(src, dst, rcArrROI[0], rcArrROI[1], fDiffHigh, fDiffLow, fDiameterWire);		//3D ¿µ»ó »ç¿ë
	memcpy(ucImgDst, dst.data, sizeof(UCHAR)*nImgWidth*nImgHeight);
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_3D_Dst.bmp"));

	//1차 Wire에서 끊어짐이 발생 될 경우 2D를 이용하여 연결
	//양 끝 단을 확인 한 후 진행방향에 따라 이어질 수 있는지 확인 
	BOOL bWireC = BinarizeByFloodFill(sAlgo, sWndAlgoImg, ucImgDst, sRstAlgo, nWire, *bHorizon, poArrFoot);
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst1.bmp"));

	RemoveThin(ucImgDst, nImgWidth, nImgHeight, *bHorizon, (sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] * 0.5 / fResol));	//µÎ²² ¾ãÀº ºÎºÐ Á¦°Å
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst2.bmp"));

	if (nSelectFoot != -2 && bWireC == TRUE && (nArrFoot[0] > -1 || nArrFoot[1] > -1) && nSelectFoot < 1)
	{
		Delete_1DArray(&pfImgBuf);
		return nRet;
	}

	//Wire가 붙어 있을 경우 Wire를 분리 한 후 기준 점과 가까운 Wire를 선택 
	if (FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo,  *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == TRUE &&
		CheckWire(sAlgo, sWndAlgoImg, ucImgDst, sRstAlgo, *bHorizon, *dA_T, *dB_T, (nSelectFoot >= 2)) == TRUE &&
		FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo,  *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == TRUE)
	{
		m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst3.bmp"));
		int nW = (*bHorizon) ? nImgWidth : nImgHeight;
		int nH = (*bHorizon) ? nImgHeight : nImgWidth;
		double dMaxTh = 0.0;
		double dAvgTh = InspectionWireTh(ucImgDst, nImgWidth, nImgHeight, *bHorizon, *dA_T, *dB_T, &dMaxTh);
		float fTh = sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Max] / fResol;
		if (sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Max] > sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Default] * 3.0)
			fTh = sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Default] / fResol;
		double dMax = (nSelectFoot > 0) ? fTh * (2.0 + (nSelectFoot * 0.3)) : fTh * 2.0;

		if (dAvgTh > 0 && dMaxTh > 0 && dMax >= dMaxTh)
			nRet = 1;
		else if (dAvgTh > 0 && dMaxTh > 0 && dMaxTh >= dMax)
		{
			if (CheckWire(sAlgo, sWndAlgoImg, ucImgDst, sRstAlgo, *bHorizon, *dA_T, *dB_T, true) == TRUE &&
				FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo, *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == TRUE)
			{
				m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst4.bmp"));
				dMaxTh = 0.0;
				dAvgTh = InspectionWireTh(ucImgDst, nImgWidth, nImgHeight, *bHorizon, *dA_T, *dB_T, &dMaxTh);
				dMax = (nSelectFoot > 0) ? fTh * (2.0 + (nSelectFoot * 0.4)) : fTh * 2.0;
				if (dAvgTh > 0 && dMaxTh > 0 && dMax >= dMaxTh)
					nRet = 1;
			}
		}

		if (nRet == 1)
		{
			//Wire 두께가 실제 두께보다 얇게 찾아진 경우 보완
			BinarizeAddData(pfImgBuf, pucImgSrc, ucImgDst, nImgWidth, nImgHeight, *bHorizon);
			m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst5.bmp"));
			if (nSelectFoot != -2 && bWireC == FALSE && (nArrFoot[0] > -1 || nArrFoot[1] > -1) && nSelectFoot < 1)
			{
				//Wire가 실제로는 끊어졌으나 찾은 결과가 서로 붙어있는 경우 보완
				BinarizeDelData(ucImgDst, nImgWidth, nImgHeight, poArrFoot, *bHorizon, *dA_T, *dB_T, 50);
				m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst6.bmp"));
			}
			bWireC = m_pProcMilAlgo->CalcBlob_Select(ucImgDst, ucImgDst, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix) > 1;
			nRet = (bWireC) ? 2 : 1;
			if (FindPoint(ucImgDst, nImgWidth, nImgHeight, sRstAlgo, *bHorizon, dA_T, dB_T, nArrFoot, poArrFoot) == FALSE)
				nRet = 0;
			m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst7.bmp"));
		}
	}
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Wire_Dst_Rst.bmp"));
	Delete_1DArray(&pfImgBuf);
	return nRet;
}
*/
byte CPInsp_WireBonding::InspectionWire(AlgoWire algoWire, float *pfImgSrc, int nWire, UCHAR* pucImage, int nImgWidth, int nImgHeight, bool bHorizon, double dA, double dB, RstAlgoWire *sRstAlgo)
{
	byte byRet = 1;
	sRstAlgo->m_fArrRst[eWIRE_Height2] = 9999999;
	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	double dResol = (bHorizon) ? m_resolX : m_resolY;
	m_vTupleWirePoint.push_back({ 0,0,0 }); 
	int nSteepSlopMargin = algoWire.m_nArrData[eWIRE2_SteepSlopeMargin];
	if (nSteepSlopMargin <= 0)
		nSteepSlopMargin = 1;

	//이런 경우가 발생... 일단 예외처리
	if (nW < 2 || nH < 2) return 0;

	sRstAlgo->m_nArrCnt = 0;

	int nArrCnt = 0;
	int nMissingCnt = 0;
	int nStart = -1;
	int nEnd = -1;

	float *fArrH = NULL;
	float *fArrHI = NULL;
	float *fArrH2 = NULL;
	float *fArrW = NULL;
	float *fArrX = NULL;
	float *fArrY = NULL;
	double *dArrT = NULL;
	POINTF *poArrT_Min = NULL;
	POINTF *poArrT_Max = NULL;
	int nMaxArea = max(nW , nH);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrH, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrHI, nMaxArea + nH);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrH2, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrW, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrX, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrY, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrT, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &poArrT_Min, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &poArrT_Max, nMaxArea);
	memset(fArrH, 0, nMaxArea * sizeof(float));
	memset(fArrHI, 0, (nMaxArea + nH) * sizeof(float));
	memset(fArrH2, 0, nMaxArea * sizeof(float));
	memset(fArrW, 0, nMaxArea * sizeof(float));
	memset(fArrX, 0, nMaxArea * sizeof(float));
	memset(fArrY, 0, nMaxArea * sizeof(float));
	memset(dArrT, 0, nMaxArea * sizeof(double));
	memset(poArrT_Min, 0, nMaxArea * sizeof(POINTF));
	memset(poArrT_Max, 0, nMaxArea * sizeof(POINTF));
#if _DEBUG
 	std::vector<double> loga;
#endif
// 	std::vector<double> logb;
	//End Point 구하기 위해 필요
	for (int a = 0; a < nW; a++)
	{
		double dY_Buf = dA * a + dB;
		int nY_Buf = (int)RounD(dY_Buf);
		if (nY_Buf < 0 || nY_Buf > nH) continue;
		double dA2 = 0;
		if (dA != 0) dA2 = -1.0 / dA;
		double dB2 = dY_Buf - (dA2 * a);
		int nIdx = nY_Buf * nImgWidth + a;
		if (!bHorizon) nIdx = a * nImgWidth + nY_Buf;
		if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
		int nMinBX = -1, nMinBY = -1, nMaxBX = -1, nMaxBY = -1;
		for (int a2 = 0; a2 < nH; a2++)
		{
			int nYTotal = 1;
			int nM = 1;
			if (dA2 == 0)
			{
				//수평일 경우에는 한줄만 검사.
				//nYTotal = nH;
				nY_Buf = a;
			}
			else
			{
				dY_Buf = (a2 - dB2) / dA2;
				nY_Buf = (int)RounD(dY_Buf);
				if (nY_Buf < 0 || nY_Buf > nW) continue;
				double dY_Buf2 = ((a2 + 1) - dB2) / dA2;
				int nY_Buf2 = (int)RounD(dY_Buf2);
				if (nY_Buf2 >= 0 && nY_Buf2 < nW)
				{
					nYTotal = nY_Buf2 - nY_Buf;
					if (nYTotal < 0)
					{
						nM = -1;
						nYTotal *= nM;
					}
				}
			}
			if (nYTotal == 0) nYTotal = 1;
			for (int a3 = 0; a3 < nYTotal; a3++)
			{
				nIdx = (a2 + (a3 * nM)) * nImgWidth + nY_Buf;
				if (!bHorizon) nIdx = nY_Buf * nImgWidth + (a2 + (a3 * nM));

				if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
				if (pucImage[nIdx] == 0) continue;
				if (nMinBY == -1)
				{
					nMinBX = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
					nMinBY = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
				}
				nMaxBX = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
				nMaxBY = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
			}
		}
		if (nMinBX < 0 || nMaxBX < 0 || nMinBY < 0 || nMaxBY < 0)
			continue;
		double dThickness = sqrt(pow((nMaxBX - nMinBX) * m_resolX, 2) + pow((nMaxBY - nMinBY) * m_resolY, 2));
		if (dThickness <= 0)
		{
			continue;
		}
		nEnd = a;
	}

	nEnd -= 5;

	if (nEnd <= 0)
		nEnd = nW;

	std::vector<float> vecCenter(nEnd); //Distance 구하기 위해 필요

	for (int a = 0; a < nEnd; a++)
	{
		vecCenter[a] = -1;
		//memset(fArrHI, 0, nMaxArea + nH * sizeof(float));
		double dY_Buf = dA * a + dB;
		int nY_Buf = (int)RounD(dY_Buf);
		if (nY_Buf < 0 || nY_Buf > nH) continue;
		double dA2 = 0;
		if (dA != 0) dA2 = -1.0 / dA;
		double dB2 = dY_Buf - (dA2 * a);
		int nIdx = nY_Buf * nImgWidth + a;
		if (!bHorizon) nIdx = a * nImgWidth + nY_Buf;
		if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
		int nMinBX = -1, nMinBY = -1, nMaxBX = -1, nMaxBY = -1;
		float fSumH = 0.0f;
		int nSumHCnt = 0;
		for (int a2 = 0; a2 < nH; a2++)
		{
			int nYTotal = 1;
			int nM = 1;
			if (dA2 == 0)
			{
				//수평일 경우 한줄만
				//nYTotal = nH;
				nY_Buf = a;
			}
			else
			{
				dY_Buf = (a2 - dB2) / dA2;
				nY_Buf = (int)RounD(dY_Buf);
				if (nY_Buf < 0 || nY_Buf > nW) continue;
				double dY_Buf2 = ((a2 + 1) - dB2) / dA2;
				int nY_Buf2 = (int)RounD(dY_Buf2);
				if (nY_Buf2 >= 0 && nY_Buf2 < nW)
				{
					nYTotal = nY_Buf2 - nY_Buf;
					if (nYTotal < 0)
					{
						nM = -1;
						nYTotal *= nM;
					}
				}
			}
			if (nYTotal == 0) nYTotal = 1;
			for (int a3 = 0; a3 < nYTotal; a3++)
			{
				nIdx = (a2 + (a3 * nM)) * nImgWidth + nY_Buf;
				if (!bHorizon) nIdx = nY_Buf * nImgWidth + (a2 + (a3 * nM));

				if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
				if (pucImage[nIdx] == 0) continue;
				if (nMinBY == -1)
				{
					nMinBX = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
					nMinBY = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
				}
				int nMaxBXTemp = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
				int nMaxBYTemp = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
				if (nMaxBXTemp == nImgWidth || nMaxBYTemp == nImgHeight)
					continue;
				nMaxBX = nMaxBXTemp;
				nMaxBY = nMaxBYTemp;

				fArrHI[nSumHCnt] = pfImgSrc[nIdx];
				fSumH += pfImgSrc[nIdx];
				nSumHCnt++;
			}
		}
		if (nMinBX < 0 || nMaxBX < 0 || nMinBY < 0 || nMaxBY < 0)
			continue;
		double dThickness = sqrt(pow((nMaxBX - nMinBX) * m_resolX, 2) + pow((nMaxBY - nMinBY) * m_resolY, 2));
		if (dThickness <= 0)
		{
			vecCenter[a] = -1;

			if (nArrCnt > 0) nMissingCnt++;
			continue;
		}
		else
		{
			if (bHorizon)
				vecCenter[a] = (nMinBY + nMaxBY) / 2.;
			else
				vecCenter[a] = (nMinBX + nMaxBX) / 2.;
		}

		dArrT[nArrCnt] = dThickness;
		poArrT_Min[nArrCnt].x = (FLOAT)nMinBX;
		poArrT_Max[nArrCnt].x = (FLOAT)nMaxBX;
		poArrT_Min[nArrCnt].y = (FLOAT)nMinBY;
		poArrT_Max[nArrCnt].y = (FLOAT)nMaxBY;
		fArrX[nArrCnt] = bHorizon ? a : (int)RounD(dA * a + dB);
		fArrY[nArrCnt] = bHorizon ? (int)RounD(dA * a + dB) : a;
		if (nStart < 0) nStart = (bHorizon) ? fArrX[nArrCnt] : fArrY[nArrCnt];

		if (nArrCnt == 0)
		{
			sRstAlgo->m_poWire_RS.x = (int)RounD(fArrX[nArrCnt]);
			sRstAlgo->m_poWire_RS.y = (int)RounD(fArrY[nArrCnt]);
		}
		sRstAlgo->m_poWire_RE.x = (int)RounD(fArrX[nArrCnt]);
		sRstAlgo->m_poWire_RE.y = (int)RounD(fArrY[nArrCnt]);
		fArrH[nArrCnt] = fSumH / (float)nSumHCnt;
		fSumH = 0;
		int nHICnt = 0;
		for (int a2 = (nSumHCnt*0.25); a2 < nSumHCnt- (nSumHCnt*0.25); a2++) // 두께 전부 쓰는부분 -> 일정부분 사용으로 변경 
		{
			//if (fArrHI[a2] >= fArrH[nArrCnt] * 0.9 && fArrHI[a2] <= fArrH[nArrCnt] * 1.3)
			{
				fSumH += fArrHI[a2];
				nHICnt++;
			}
		}

		//반복문 안에서 와이어 최대 높이 구하는 구문 
		//여기서 옵션(특정지점의높이를Wire높이로 or 전체영역에서의 최대높이를Wire높이로) 사용 유무에따른 높이계산 구문 추가
		//넘겨져 받아오는 값은 y=ax+b 직선 공식에서 a,b값 받아오면 됨.
		//티칭창 UI에는 Window 장축의 절반지점에 Line하나 출력해서 내릴 수 있도록 구성 
		if (fSumH / nHICnt > sRstAlgo->m_fArrRst[eWIRE_Highest])
		{
			sRstAlgo->m_fArrRst[eWIRE_Highest] = fSumH / nHICnt;
			sRstAlgo->m_nArrX[eDot_Highest] = (nMinBX + nMaxBX) / 2; // (int)RounD(fArrX[nArrCnt]);
			sRstAlgo->m_nArrY[eDot_Highest] = (nMinBY + nMaxBY) / 2; //(int)RounD(fArrY[nArrCnt]);
		}
		if (sRstAlgo->m_fArrRst[eWIRE_Height2] > fArrH[nArrCnt])
		{
			sRstAlgo->m_fArrRst[eWIRE_Height2] = fArrH[nArrCnt];
			sRstAlgo->m_nArrX[eDot_LowH] = (nMinBX + nMaxBX) / 2; //(int)RounD(fArrX[nArrCnt]);
			sRstAlgo->m_nArrY[eDot_LowH] = (nMinBY + nMaxBY) / 2;// (int)RounD(fArrY[nArrCnt]);
		}
// 		if (nArrCnt > 0)
// 		{
// 			if (nArrCnt - nSteepSlopMargin >= 0)
// 			{
#if _DEBUG
 				loga.push_back(fArrH[nArrCnt]);
#endif
//  				logb.push_back(fArrH[nArrCnt - nSteepSlopMargin]);
// 				float fGap = fArrH[nArrCnt] - fArrH[nArrCnt - nSteepSlopMargin];
// 				if (fGap < 0) fGap *= -1;
// 				if (fGap > sRstAlgo->m_fArrRst[eWIRE_SteepSlope])
// 					sRstAlgo->m_fArrRst[eWIRE_SteepSlope] = fGap;
// 
// 			}
// 		}

		if (bHorizon)
			fArrW[nArrCnt] = ((dA * a + dB) - ((nMinBY + nMaxBY) / 2)) * m_resolY;
		else
			fArrW[nArrCnt] = ((dA * a + dB) - ((nMinBX + nMaxBX) / 2)) * m_resolX;
		float fWarp = fArrW[nArrCnt];
		if (fWarp < 0) fWarp *= -1;
		if (fWarp > sRstAlgo->m_fArrRst[eWIRE_Warp])
		{
			sRstAlgo->m_fArrRst[eWIRE_Warp] = fWarp;
			sRstAlgo->m_nArrX[eDot_Warp] = (nMinBX + nMaxBX) / 2;
			sRstAlgo->m_nArrY[eDot_Warp] = (nMinBY + nMaxBY) / 2;
		}
		nArrCnt++;
	}
	int nDivCnt = algoWire.m_nArrData[eWIRE2_DivCnt];
	if (nDivCnt <= 0) nDivCnt = 1;
	if (nDivCnt > WIRE_DOT3_CNT) nDivCnt = WIRE_DOT3_CNT;
	if (nDivCnt > nArrCnt) nDivCnt = nArrCnt;

	// 두께 관련 앞뒤 Wire 10% 잘라 내고 사용.
	int  nCuttingWire = 0;
	if (!((nArrCnt == nDivCnt) || (nCuttingWire < 0)))
	{
		nCuttingWire = (nArrCnt / 10);
		nArrCnt -= nCuttingWire;
	}

	//모비스 기준 
	std::vector<double> dCuttingWireHight;
	double dStartWirePoint = fArrH[0]; //첫스타트점 기준.
	bool bStartPos = (algoWire.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_StartPointHeightSearch) == m_eWIRE_Data2_StartPointHeightSearch;
	if (!bStartPos)
	{
		std::vector<double> dStartWireHight;
		dStartWirePoint = fArrH[nArrCnt + nCuttingWire - 1]; //마지막점 기준.  Mobis 는 start 와 End 가 일정하지않아 높이값으로 변경해야함,
		for (int nSteepSlopStart = 0; nSteepSlopStart < nCuttingWire; nSteepSlopStart++)
			dStartWireHight.push_back(fArrH[nSteepSlopStart]);
		for (int nSteepSlopEnd = nArrCnt - nCuttingWire; nSteepSlopEnd < nArrCnt; nSteepSlopEnd++)
			dStartWireHight.push_back(fArrH[nSteepSlopEnd]);
		// Cutting 된 와이어중 재일 낮은곳을 스타트점으로 변경 
		dStartWirePoint = *min_element(dStartWireHight.begin(), dStartWireHight.end());
	}
 	for (int nSteepSlopCnt = nCuttingWire; nSteepSlopCnt < nArrCnt - nCuttingWire; nSteepSlopCnt++)
	{
		dCuttingWireHight.push_back(fArrH[nSteepSlopCnt] - dStartWirePoint);
	}
	int nMinIndex = min_element(dCuttingWireHight.begin(), dCuttingWireHight.end()) - dCuttingWireHight.begin();
 	if (nMinIndex == 0 || nMinIndex == dCuttingWireHight.size() -1) 
 	{
 		sRstAlgo->m_fArrRst[eWIRE_SteepSlope] = 0; //Pass
 	}
 	else if (dCuttingWireHight[nMinIndex] < 0 )
		sRstAlgo->m_fArrRst[eWIRE_SteepSlope] = abs(dCuttingWireHight[nMinIndex]); //음수일경우
	else
		sRstAlgo->m_fArrRst[eWIRE_SteepSlope] = 0; //양수일경우는 Pass 이기때문에. 

	CString strLog1;
	strLog1.Format(_T("SteepSlop : %f CheckIndex : %d ,TotalCount :  %d  "), dCuttingWireHight[nMinIndex], nMinIndex, nArrCnt);
	g_pMPTI->AddLog(strLog1);

	int nDivGap = 1;
	int nDivGap2 = 0;
	if (nDivCnt > 1 && nArrCnt > nDivCnt)
	{
		nDivGap = (nArrCnt- nCuttingWire) / nDivCnt;
		nDivGap2 = (nArrCnt- nCuttingWire) % nDivCnt;
	}
	else
	{
		nDivCnt = 1;
		nDivGap = nArrCnt;
		nDivGap2 = 0;
	}
	if (nDivGap <= 0) nDivGap = 1;
	if (nDivGap2 <= 0) nDivGap2 = 0;
	if (nDivGap > nArrCnt) nDivGap = nArrCnt;
	if (nDivGap2 > nDivCnt) nDivGap2 = 0;
	int nDivIdx = nCuttingWire;
	for (int a = 0; a < nDivCnt; a++)
	{
		int nTotal = nDivGap;
		if (nDivGap2 > 0 && a < nDivGap2)
			nTotal++;
		if (nTotal > nArrCnt) nTotal = nArrCnt;
		double dRstThickness = 0.0;
		POINTF poArrT[WIRE_DRAW_CNT];
		memset(poArrT, 0, sizeof(POINTF) * WIRE_DRAW_CNT);
		for (int b = 0; b < nTotal; b++)
		{
			if ((algoWire.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_Max) == m_eWIRE_Data2_Max)
			{
				if (dArrT[nDivIdx] > dRstThickness)
				{
					dRstThickness = dArrT[nDivIdx];
					poArrT[0].x = poArrT_Min[nDivIdx].x;
					poArrT[0].y = poArrT_Min[nDivIdx].y;
					poArrT[1].x = poArrT_Max[nDivIdx].x;
					poArrT[1].y = poArrT_Max[nDivIdx].y;
				}
			}
			else
			{
				dRstThickness += dArrT[nDivIdx];
				poArrT[0].x += poArrT_Min[nDivIdx].x;
				poArrT[0].y += poArrT_Min[nDivIdx].y;
				poArrT[1].x += poArrT_Max[nDivIdx].x;
				poArrT[1].y += poArrT_Max[nDivIdx].y;
			}
			nDivIdx++;
			if (nDivIdx > nArrCnt)
				break;
		}
		if ((algoWire.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_Max) == m_eWIRE_Data2_Max)
			sRstAlgo->m_fArrValue_T[a] = dRstThickness;
		else
			sRstAlgo->m_fArrValue_T[a] = dRstThickness / nTotal;
		if (sRstAlgo->m_fArrValue_T[a] > sRstAlgo->m_fArrRst[eWIRE_Thickness])
		{
			sRstAlgo->m_fArrRst[eWIRE_Thickness] = sRstAlgo->m_fArrValue_T[a];
			if ((algoWire.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_Max) != m_eWIRE_Data2_Max)
			{
				sRstAlgo->m_nArrX_T[0] = poArrT[0].x / nTotal;
				sRstAlgo->m_nArrY_T[0] = poArrT[0].y / nTotal;
				sRstAlgo->m_nArrX_T[1] = poArrT[1].x / nTotal;
				sRstAlgo->m_nArrY_T[1] = poArrT[1].y / nTotal;
			}
			else
			{
				sRstAlgo->m_nArrX_T[0] = poArrT[0].x;
				sRstAlgo->m_nArrY_T[0] = poArrT[0].y;
				sRstAlgo->m_nArrX_T[1] = poArrT[1].x;
				sRstAlgo->m_nArrY_T[1] = poArrT[1].y;
			}
		}
		if (nDivIdx > nArrCnt)
			break;
	}
	nArrCnt += nCuttingWire;
	float fX_D = (sRstAlgo->m_poWire_RE.x - sRstAlgo->m_poWire_RS.x) * m_resolX;
	float fY_D = (sRstAlgo->m_poWire_RE.y - sRstAlgo->m_poWire_RS.y) * m_resolY;
	sRstAlgo->m_fArrRst[eWIRE_Distance] = (double)sqrt(pow(fX_D, 2) + pow(fY_D, 2));
	CString strLogArr;
	strLogArr.Format(_T("Wire Distance : %f"), sRstAlgo->m_fArrRst[eWIRE_Distance]);
	g_pMPTI->AddLog(strLogArr);

	fX_D = (sRstAlgo->m_poWire_TE.x - sRstAlgo->m_poWire_TS.x) * m_resolX;
	fY_D = (sRstAlgo->m_poWire_TE.y - sRstAlgo->m_poWire_TS.y) * m_resolY;
	double dTDist = (double)sqrt(pow(fX_D, 2) + pow(fY_D, 2));
	if (dTDist > sRstAlgo->m_fArrRst[eWIRE_Distance] * 3)
		byRet = 2;
	fX_D = (sRstAlgo->m_poWire_TE.x - sRstAlgo->m_poWire_TS.x);
	fY_D = (sRstAlgo->m_poWire_TE.y - sRstAlgo->m_poWire_TS.y);
	dTDist = (double)sqrt(pow(fX_D, 2) + pow(fY_D, 2));
	if (dTDist > nArrCnt * 3)
		byRet = 2;
	if (byRet != 2 && nMissingCnt > dTDist * 0.2)
		byRet = 3;

	//실제 와이어의 Distance ,끊어지면 긴 와이어로 측정하자
	if(bStartPos)
 	{
 		float maxDistance = 0;
 		float localDistance = 0;
 		int nStart = sRstAlgo->m_poWire_RS.y;
 		int nEnd = sRstAlgo->m_poWire_RE.y;
 		if (bHorizon)
 		{
 			nStart = sRstAlgo->m_poWire_RS.x;
 			nEnd = sRstAlgo->m_poWire_RE.x;
 		}
 		nStart = (nStart == 0) ? 1 : nStart;
 		nEnd = (nEnd == 0) ? 1 : nEnd;
 		for (size_t i = nStart; i < nEnd; i++)
 		{
 			if (vecCenter[i] == -1 && localDistance > 0)
 			{
 				if (localDistance > maxDistance)
 					maxDistance = localDistance;
 
 				localDistance = 0;
 			}
 
 			if (vecCenter[i - 1] != -1)
 			{
 				float fX_D = (vecCenter[i - 1] - vecCenter[i]) * m_resolX;
 				float fY_D = m_resolY;
 
 				localDistance += (float)sqrt(pow(fX_D, 2) + pow(fY_D, 2));
 			}
 
 			if (i == nEnd - 1)
 			{
 				if (localDistance > maxDistance)
 					maxDistance = localDistance;
 
 				localDistance = 0;
 			}
 		}
 
 		if (maxDistance > 0)
 			sRstAlgo->m_fArrRst[eWIRE_Distance] = maxDistance;
 	}

	int nHeightCnt = (bHorizon) ? sRstAlgo->m_nArrX[eDot_Highest] : sRstAlgo->m_nArrY[eDot_Highest];
	float fGapH_1 = 0.0f, fGapH_2 = 0.0f;
	double *ptrdLineX = NULL;
	double *ptrdLineY = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrdLineX, nW);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrdLineY, nW);
	if (nHeightCnt > nStart)
	{
		int nDot = algoWire.m_nArrData[eWIRE2_Dot] + 1;
		if (nDot < 1) nDot = 1;
		int nDotW = nHeightCnt - nStart;
		int nDotGap = nDotW / nDot;
		for (int a = 0; a < nDot; a++)
		{
			memset(ptrdLineX, 0, sizeof(double) * nW);
			memset(ptrdLineY, 0, sizeof(double) * nW);
			int nS = a * nDotGap;
			int nE = nS + nDotGap;
			if (nS > nW) nS = nW;
			if (nE > nW) nE = nW;
			if (nDot - 1 == a) nE = nDotW;
			if (nS < 0) nS = 0;
			for (int b = nS; b < nE; b++)
			{
				ptrdLineX[b - nS] = b - nS;
				ptrdLineY[b - nS] = fArrH[b];
			}
			double dA_H = 0, dB_H = 0;
			CPInsp::GetGradient(ptrdLineX, ptrdLineY, nE - nS, dA_H, dB_H, true);
			for (int b = nS; b < nE; b++)
			{
				fArrH2[b] = dA_H * (b - nS) + dB_H;
				float fRstGapH = fArrH2[b] - fArrH[b];
				if (fRstGapH < 0) fRstGapH *= -1;
				if (fRstGapH > fGapH_1)
				{
					sRstAlgo->m_fArrRst[eWIRE_Height] = fArrH2[b] - fArrH[b];
					fGapH_1 = fRstGapH;
					sRstAlgo->m_nArrX[eDot_TH] = (poArrT_Min[b].x + poArrT_Max[b].x) / 2;//(int)RounD(fArrX[b]);
					sRstAlgo->m_nArrY[eDot_TH] = (poArrT_Min[b].y + poArrT_Max[b].y) / 2;//(int)RounD(fArrY[b]);
					sRstAlgo->m_nArrX[eDot_TH1] = (poArrT_Min[b].x + poArrT_Max[b].x) / 2;//(int)RounD(fArrX[b]);
					sRstAlgo->m_nArrY[eDot_TH1] = (poArrT_Min[b].y + poArrT_Max[b].y) / 2; //(int)RounD(fArrY[b]);
				}
			}
		}
	}
	if (nArrCnt > nHeightCnt - nStart)
	{
		int nDot = algoWire.m_nArrData[eWIRE2_Dot] + 1;
		if (nDot < 1) nDot = 1;
		int nDotW = nArrCnt - (nHeightCnt - nStart);
		int nDotGap = nDotW / nDot;
		for (int a = 0; a < nDot; a++)
		{
			memset(ptrdLineX, 0, sizeof(double) * nW);
			memset(ptrdLineY, 0, sizeof(double) * nW);
			int nS = a * nDotGap + (nHeightCnt - nStart);
			int nE = nS + nDotGap;
			if (nS > nW) nS = nW;
			if (nE > nW) nE = nW;
			if (nDot - 1 == a) nE = nArrCnt;
			if (nS < 0) nS = 0;
			for (int b = nS; b < nE; b++)
			{
				ptrdLineX[b - nS] = b - nS;
				ptrdLineY[b - nS] = fArrH[b];
			}
			double dA_H = 0, dB_H = 0;
			CPInsp::GetGradient(ptrdLineX, ptrdLineY, nE - nS, dA_H, dB_H, true);
			for (int b = nS; b < nE; b++)
			{
				fArrH2[b] = dA_H * (b - nS) + dB_H;
				float fRstGapH = fArrH2[b] - fArrH[b];
				if (fRstGapH < 0) fRstGapH *= -1;
				if (fRstGapH > fGapH_2)
				{
					if (fGapH_2 > fGapH_1)
					{
						sRstAlgo->m_fArrRst[eWIRE_Height] = fArrH2[b] - fArrH[b];
						sRstAlgo->m_nArrX[eDot_TH] = (poArrT_Min[b].x + poArrT_Max[b].x) / 2; //(int)RounD(fArrX[b]);
						sRstAlgo->m_nArrY[eDot_TH] = (poArrT_Min[b].y + poArrT_Max[b].y) / 2; //(int)RounD(fArrY[b]);
					}
					fGapH_2 = fRstGapH;
					sRstAlgo->m_nArrX[eDot_TH2] = (poArrT_Min[b].x + poArrT_Max[b].x) / 2; //(int)RounD(fArrX[b]);
					sRstAlgo->m_nArrY[eDot_TH2] = (poArrT_Min[b].y + poArrT_Max[b].y) / 2; //(int)RounD(fArrY[b]);
				}
			}
		}
	}
	memset(ptrdLineX, 0, sizeof(double) * nW);
	memset(ptrdLineY, 0, sizeof(double) * nW);
	ptrdLineX[0] = sRstAlgo->m_poWire_RS.x;
	ptrdLineY[0] = sRstAlgo->m_poWire_RS.y;
	ptrdLineX[1] = sRstAlgo->m_poWire_RE.x;
	ptrdLineY[1] = sRstAlgo->m_poWire_RE.y;
	double dA_R = 0, dB_R = 0;
	CPInsp::GetGradient(ptrdLineX, ptrdLineY, 2, dA_R, dB_R, bHorizon);
	sRstAlgo->m_bArrHo = bHorizon;
	sRstAlgo->m_poArrAB.x = (float)dA_R;
	sRstAlgo->m_poArrAB.y = (float)dB_R;
	Delete_1DArray(&ptrdLineX);
	Delete_1DArray(&ptrdLineY);

	//UI Teahing용 데이타
	{
		if (m_pWirePoint)
		{
			delete[] m_pWirePoint;
			m_pWirePoint = nullptr;
		}

		if (m_WirepH2)
		{
			delete[] m_WirepH2;
			m_WirepH2 = nullptr;
		}

		int nMargin = 10;
		if (nArrCnt / nMargin < 10)
			nMargin = 0;

		m_nPointNum = nArrCnt - (nMargin * 2);
		sRstAlgo->m_nBinCenterCnt = m_nPointNum;

		//m_pWirePoint = new POINTF[m_nPointNum];
		m_pWirePoint = g_pMManager->pem_new<POINTF>(true, m_nPointNum, (PCHAR)__FUNCTION__, __LINE__);


		for (size_t i = nMargin; i < nArrCnt - nMargin; i++)
		{
			m_pWirePoint[i - nMargin].x = (poArrT_Min[i].x + poArrT_Max[i].x) / 2.;
			m_pWirePoint[i - nMargin].y = (poArrT_Min[i].y + poArrT_Max[i].y) / 2.;
		}

		// 와이어의 Min Max Copy 를 위함.( Max - Min = 두께 )
		//

		POINTF * pWirePointMin = g_pMManager->pem_new<POINTF>(true, m_nPointNum, (PCHAR)__FUNCTION__, __LINE__);
		POINTF * pWirePointMax = g_pMManager->pem_new<POINTF>(true, m_nPointNum, (PCHAR)__FUNCTION__, __LINE__);
		memcpy(pWirePointMin, poArrT_Min, sizeof(POINTF) *m_nPointNum);
		memcpy(pWirePointMax, poArrT_Max, sizeof(POINTF) *m_nPointNum);
		
		m_vTupleWirePoint[m_vTupleWirePoint.size() - 1] = std::make_tuple((long long)(void *)pWirePointMin , (long long)(void *)pWirePointMax ,m_nPointNum );
		// WireCenter Copy 를 위함

		//m_WirepH2 = new float[m_nPointNum];
		m_WirepH2 = g_pMManager->pem_new<float>(true, m_nPointNum, (PCHAR)__FUNCTION__, __LINE__);

		memcpy(m_WirepH2, fArrH + nMargin, m_nPointNum * sizeof(float));

	}

	if (nArrCnt > WIRE_DOT_CNT)
	{
		float *fArrBufW = NULL;
		float *fArrBufH = NULL;
		float *fArrBufH_T = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrBufW, nArrCnt);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrBufH, nArrCnt);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrBufH_T, nArrCnt);
		memset(fArrBufW, 0, nArrCnt * sizeof(float));
		memset(fArrBufH, 0, nArrCnt * sizeof(float));
		memset(fArrBufH_T, 0, nArrCnt * sizeof(float));
		int nGap = nArrCnt % WIRE_DOT_CNT;
		int nGap2 = nGap / 2;
		if (nGap % 2 != 0) nGap2++;
		int nIdx = 0;
		for (int a = 0; a < nArrCnt - nGap; a++)
		{
			fArrBufW[a] = fArrW[nIdx];
			fArrBufH[a] = fArrH[nIdx];
			fArrBufH_T[a] = fArrH2[nIdx];
			if (a < nGap2 || a >= nArrCnt - nGap2)
				nIdx++;
			nIdx++;
		}
		nGap = (nArrCnt - nGap) / WIRE_DOT_CNT;
		nIdx = 0;
		for (int a = 0; a < WIRE_DOT_CNT; a++)
		{
			fArrW[a] = fArrBufW[nIdx];
			fArrH[a] = fArrBufH[nIdx];
			fArrH2[a] = fArrBufH_T[nIdx];
			nIdx += nGap;
		}
		nArrCnt = WIRE_DOT_CNT - 1;
		Delete_1DArray(&fArrBufW);
		Delete_1DArray(&fArrBufH);
		Delete_1DArray(&fArrBufH_T);
	}
	
	if (nArrCnt >= WIRE_DOT_CNT)
		nArrCnt = WIRE_DOT_CNT - 1;
	sRstAlgo->m_nArrCnt = nArrCnt;
	for (int a = 0; a < nArrCnt; a++)
	{
		sRstAlgo->m_fArrValue_H[a] = fArrH[a];
		sRstAlgo->m_fArrValue_H2[a] = fArrH2[a];
		sRstAlgo->m_fArrValue_W[a] = fArrW[a];

		/*sRstAlgo->m_nArrBinCentX[a] = (int)fArrX[a];
		sRstAlgo->m_nArrBinCentY[a] = (int)fArrY[a];*/
	}
	Delete_1DArray(&fArrH);
	Delete_1DArray(&fArrHI);
	Delete_1DArray(&fArrH2);
	Delete_1DArray(&fArrW);
	Delete_1DArray(&fArrX);
	Delete_1DArray(&fArrY);
	Delete_1DArray(&dArrT);
	Delete_1DArray(&poArrT_Min);
	Delete_1DArray(&poArrT_Max);

	return byRet;
}
bool CPInsp_WireBonding::HoleRemoveCheck(CWire_Model* pModel, int nIndex , bool bOptionUse)
{
	if (!bOptionUse) //사용 여부 받기. 
		return false;
	//찾는 영역의 Index 를 받고 빨간색 Circle을 제외시키기 위함 // Mobis 
	bool bResult = false;
	int nRedLow = 100, nGreenLow = 0, nBlueLow = 0;
	int nRedHi = 255, nGreenHi = 100, nBlueHi = 100;
	int nRValue, nGValue, nBValue;
	//cv::inRange(ColorImage, cv::Scalar(0, 0, 100), cv::Scalar(100, 100, 255), red1Thresholded);
	nRValue = pModel->m_ImageBuffer->imgTop_R[nIndex];
	nGValue = pModel->m_ImageBuffer->imgTop_G[nIndex];
	nBValue = pModel->m_ImageBuffer->imgTop_B[nIndex];

	if (
		(nRedLow <= nRValue) && (nRValue <= nRedHi) &&
		(nGreenLow <= nGValue) && (nGValue <= nGreenHi) &&
		(nBlueLow <= nBValue) && (nBValue <= nBlueHi)
		)
		bResult = true;

	return bResult;
}

void CPInsp_WireBonding::BinarizeByFloodFill(cv::Mat src, cv::Mat & dst, cv::Rect st, cv::Rect ed, float nDiffHigh, float nDiffLow, float nDiameterWire, CWire_Model* pModel, bool bRectUse)
{
	float nFindValueSt(0), nFindVlaueEd(0);
	// nFindValue	: 찾을 피크 밝기 값
	// nSubFindValue: 피크 밝기 주변으로 피크에 포함 될 밝기 값

	if (dst.empty() == true)
		dst = cv::Mat::zeros(src.size(), CV_8UC1);

	BYTE nLabelValue(255);	// foreground를 255로 한다.
	int iFindCount(8);
	int nRefArray[8][2] =   // 8pixel을 비교한다.
	{
		{ -1, +1 } ,     // 좌, 하, ↙
		{ -1,  0 } ,     // 좌, 정, ←
		{ -1, -1 } ,     // 좌, 상, ↖
		{ 0, -1 } ,     // 정, 상, ↑
		{ +1, -1 } ,     // 우, 상, ↗
		{ +1,  0 } ,     // 우, 정, →
		{ +1, +1 } ,     // 우, 하, ↘
		{ 0, +1 }       // 정, 하, ↓
	};

	jsl::CircularQueue<std::array<float, 4>>      ptque(src.total());

	float * srcPtr(src.ptr<float>());
	BYTE * binPtr(dst.ptr());
	cv::Size srcSz(src.size()), binSz(dst.size());
	int srcP(src.cols), binP(dst.step);
	cv::Rect RectList[2] = { st, ed };
	int nThresList[2] = { nFindValueSt, nFindVlaueEd };
	//float nMaxList[2] = { 1.0e-35, 1.0e-35 };
	float nMaxList[2] = { -1000, -1000 };
	float nMinList[2] = { 9999, 9999 };
	float fLow = (nDiameterWire / 3.);
	float fHi = (nDiameterWire * 5);

	int nX, nY;
	for (int n = 0; n < 2; n++)
	{

		if (bRectUse)
		{
			for (int j = RectList[n].y; j <= (RectList[n].y + RectList[n].height); j++)
			{
				for (int i = RectList[n].x; i <= (RectList[n].x + RectList[n].width); i++)
				{
					int nIndex = j * srcP + i;
					if (nIndex < 0 || nIndex >= src.cols * src.rows)
						break;
					if (nMaxList[n] < srcPtr[nIndex])
						nMaxList[n] = srcPtr[nIndex];
					if (nMinList[n] > srcPtr[nIndex])
						nMinList[n] = srcPtr[nIndex];
				}
			}
		}
		else // End - Wedge Point 기준으로 Tracking 진행시에 사용하는 옵션.
		{
			nX = (RectList[n].x + RectList[n].width) - 50;
			nY = (RectList[n].y + RectList[n].height) - 50;
			int nIndex = nY * srcP + nX;
			nMaxList[n] = srcPtr[nIndex];
		}
		

	}
// 	std::vector<float> logF;
// 
// 	for (int sz = 0; sz < srcP; sz++)
// 		logF.push_back(srcPtr[ (RectList[0].x + RectList[0].width)  * srcP + sz]);

	for (int n = 0; n < 2; n++)
	{
		pModel->m_dFootHeight[n] = nThresList[n] = nMaxList[n] - nDiameterWire;

// 		if (!((fLow <= abs(nMinList[n] - nMaxList[n])) && (abs(nMinList[n] - nMaxList[n]) <= fHi)))
// 		{
			CString strLog1;
			strLog1.Format(_T("Index [%d] /  hi : %f , Low %f, nDimater : %f  "),n , nDiffHigh, nDiffLow, nDiameterWire);
			g_pMPTI->AddLog(strLog1);
			//return;
		//}
	}
	int nCenterArea = abs(nThresList[0] - nThresList[1]);
	for (int n = 0; n < 2; n++)
	{
		for (int j = RectList[n].y; j <= (RectList[n].y + RectList[n].height); j++)
		{
			for (int i = RectList[n].x; i <= (RectList[n].x + RectList[n].width); i++)
			{
				int nIndex = j * srcP + i;
				int nIndex2 = j * binP + i;
				if (nIndex < 0 || nIndex >= src.cols * src.rows ||
					nIndex2 < 0 || nIndex2 >= src.cols * src.rows)
					continue;
				if (srcPtr[nIndex] > nThresList[n] && binPtr[nIndex2] == 0)	// 피크점을 찾으면...
				{
					binPtr[nIndex2] = nLabelValue;

					for (int nhDef = 0; nhDef < iFindCount; nhDef++)	// glassfire 초기 검색
					{
						int refy = j + nRefArray[nhDef][0];
						int refx = i + nRefArray[nhDef][1];

						if (refy >= 0 && refy < srcSz.height	&&
							refx >= 0 && refx < srcSz.width)
						{
							if (binPtr[refy*binP + refx] == 0 && srcPtr[refy*srcP + refx] > nThresList[n])
							{
								binPtr[refy*binP + refx] = nLabelValue;
								ptque.Push({ (float)refx, (float)refy, srcPtr[refy*srcP + refx], srcPtr[refy*srcP + refx] });

							}
						}
					}

					// glassfire 확장
					while (ptque.Size() > 0)
					{
						float * point = ptque.Front().data();
						ptque.Pop();

						for (int nhDef = 0; nhDef < iFindCount; nhDef++)
						{
							int refy = point[1] + nRefArray[nhDef][0];
							int refx = point[0] + nRefArray[nhDef][1];

							if (refy >= 0 && refy < srcSz.height	&&
								refx >= 0 && refx < srcSz.width)
							{
								if (binPtr[refy*binP + refx]) continue;

								//1. 해당 좌표의 높이값을 두 시작점간 가장 가까운 지점의 높이(-diameter)와 비교
								//2. Difflow, DiffHigh 비교
								float stDstX = (st.x + st.width * 0.5f) - refx;
								float stDstY = (st.y + st.height * 0.5f) - refy;

								float edDstX = (ed.x + ed.width * 0.5f) - refx;
								float edDstY = (ed.y + ed.height * 0.5f) - refy;
// 								float fCenterPtX = ((st.x + st.width * 0.5f) + (ed.x + ed.width * 0.5f)) / 2;
// 								float fCenterPtY = ((st.y + st.height * 0.5f) + (ed.y + ed.height * 0.5f)) / 2;
// 
								float distSt = sqrt((stDstY*stDstY) + (stDstX*stDstX));
								float distEd = sqrt((edDstY*edDstY) + (edDstX*edDstX));
								double dPadDist = abs(sqrt(pow((ed.x - st.x), 2) + pow((ed.y - st.y), 2)));
								double dPadCutting = dPadDist/5; 

								 
								//float distCenter =  (float)sqrt(pow(fCenterPtX - refx, 2) + pow(edDstY - refy, 2));
								float val = srcPtr[refy*srcP + refx];
								if (
									((distSt < distEd && val > nThresList[0]) || ((distSt > distEd && val > nThresList[1]))
//										|| distSt < distEd && distSt>dPadCutting && val > nThresList[1] || distSt > distEd && distEd > dPadCutting &&  val > nThresList[1]) // Pad 위치보단 높아야함.  
									//&&
// 									(!HoleRemoveCheck(pModel, refy*srcP + refx)) //||((distSt >= distCenter && distCenter <= distEd) && ((val > nCenterArea) &&  (val < nThresList[0])))
 									&&
									(point[2] - nDiffLow) < val && val < (point[2] + nDiffHigh)
									//(point[3] - nDiffLow) < val && val < (point[3] + nDiffHigh)
									))
								{

									
									binPtr[refy*binP + refx] = nLabelValue;
									ptque.Push({ (float)refx, (float)refy, val, point[2] });
								}
							}
						}
					}//while
				}//if main
			}//for x
		}// for y
	}// for rect
}

BOOL CPInsp_WireBonding::BinarizeByFloodFill(AlgoWire algoWire, WndAlgoImg &sWndAlgoImg, UCHAR *ucImage, RstAlgoWire *sRstAlgo, int /*nWire*/, bool bHorizon, POINT *poArrFoot)
{
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (ucImage == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return FALSE;
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("BinarizeByFloodFill.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nImgWidth, nImgHeight, _T("BinarizeByFloodFill_2D.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("BinarizeByFloodFill_3D.bmp"));

	UCHAR *pucBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucBlob, nImgWidth * nImgHeight);
	memset(pucBlob, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);

	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucImage, pucBlob, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix);
	//cv::Mat calcRst(nImgHeight, nImgWidth, CV_8UC1, pucBlob);


	m_pProcMilAlgo->SaveWorkImg(pucBlob, nImgWidth, nImgHeight, _T("BinarizeByFloodFill_BLOB.bmp"));
	Delete_1DArray(&pucBlob);
	if (nCntBlob <= 1) return FALSE;
	return TRUE;
	// 끊어짐이 발생한 경우(nCntBlob이 2개 이상) 2D 이미지로 Wire 재 탐색
	// Wire 양 끝 단 확인 후, 진행방향에 따라 이어질 수 있는지 확인
	double dArrArea[3], dArrCX[3], dArrCY[3];
	CRect rcArrBlob[3];
	memset(dArrArea, 0, sizeof(double) * 3);
	memset(dArrCX, 0, sizeof(double) * 3);
	memset(dArrCY, 0, sizeof(double) * 3);
	memset(rcArrBlob, 0, sizeof(CRect) * 3);
	m_pProcMilAlgo->GetBlobResult(dArrArea, dArrCX, dArrCY, rcArrBlob, true);

	if (nImgWidth * nImgHeight < dArrArea[0] * 4)  //화면 영역에 비례하여 클 경우 (잘못된 검출)
		return TRUE;

	float fX_D = (rcArrBlob[0].left - rcArrBlob[1].right);
	float fY_D = (rcArrBlob[0].top - rcArrBlob[1].top);

	//교차 영역이 클경우 서로 평행하는 Wire
	CRect rectIntersect;

	if (::IntersectRect(&rectIntersect, &rcArrBlob[0], &rcArrBlob[1]))
	{
		float fIntersectArea = rectIntersect.Width() * rectIntersect.Height();

		float Area1 = rcArrBlob[0].Width() * rcArrBlob[0].Height();
		float Area2 = rcArrBlob[1].Width() * rcArrBlob[1].Height();

		if (fIntersectArea > Area1 * 0.7 || fIntersectArea > Area2 * 0.7)
		{
			vector<std::vector<cv::Point>> contours;
			vector<std::vector<cv::Point>> contoursOut;
 			// 	//Wire 2개 이상일때 끊어짐과 서로 평행한 와이어 인지 찾기위함.
 			cv::Mat cvWireSrc(nImgHeight, nImgWidth, CV_8UC1, ucImage);
			cv::line(cvWireSrc, cv::Point(poArrFoot[0].x, poArrFoot[0].y), cv::Point(poArrFoot[1].x, poArrFoot[1].y), cv::Scalar(255, 255, 255), 1, 8);
 			std::vector<cv::Vec4i> hierarchy;
 			cv::findContours(cvWireSrc, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
 			vector<cv::Point> vBlobCenter;
			for (int i = 0; i < contours.size(); i++)
			{
				if (contours[i].size() < 100) // 100 보다 작은건 와이어로 취급하지않음. 
					continue;
				contoursOut.push_back(contours[i]);
			}
			
 			for (int i = 0; i < contoursOut.size(); i++)
			{
 				cv::Rect rectcv = cv::boundingRect(contoursOut[i]);
 				cv::RotatedRect rectRotate = cv::minAreaRect(contoursOut[i]);
 				vBlobCenter.push_back(rectRotate.center);
 			}
 			float fA = 0.f;
 			float fB = 0.f;
 			fA = (float)(poArrFoot[1].y - poArrFoot[0].y) / (float)(poArrFoot[1].x - poArrFoot[0].x);
 			fB = poArrFoot[0].y - fA * poArrFoot[0].x;
 			vector<double> vDistance;
 			for (int i = 0; i < vBlobCenter.size(); i++)
 				vDistance.push_back(fabs(((double)fA * vBlobCenter[i].x - vBlobCenter[i].y + fB)) / sqrtf(fA * fA + 1));
 			int nLableIndex = min_element(vDistance.begin(), vDistance.end()) - vDistance.begin();
			if (nLableIndex >= 0)
			{
				std::vector<cv::Point> vRstPt = contoursOut[nLableIndex];
				cvWireSrc = cv::Mat::zeros(cv::Size(nImgWidth, nImgHeight), CV_8UC1);
				cv::fillPoly(cvWireSrc, vRstPt, cv::Scalar(255, 255, 255), 8);
				cv::morphologyEx(cvWireSrc, cvWireSrc, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), 1);
			}
			return FALSE;
	}
	}
	return TRUE;

	if (bHorizon)
	{
		if (rcArrBlob[0].left > rcArrBlob[1].left)
			fX_D = (rcArrBlob[0].right - rcArrBlob[1].left);
	}
	else
	{
		fX_D = (rcArrBlob[0].left - rcArrBlob[1].left);
		fY_D = (rcArrBlob[0].top - rcArrBlob[1].bottom);
		if (rcArrBlob[0].top > rcArrBlob[1].top)
			fY_D = (rcArrBlob[0].bottom - rcArrBlob[1].top);
	}

	double dDist = (double)sqrt(pow(fX_D, 2) + pow(fY_D, 2));

	fX_D = (poArrFoot[1].x - poArrFoot[0].x);
	fY_D = (poArrFoot[1].y - poArrFoot[0].y);

	double dTDist = (double)sqrt(pow(fX_D, 2) + pow(fY_D, 2));
	if (dTDist * 0.5 > dDist) return FALSE;

	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	int nThMin = (int)((algoWire.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] / m_resolY) * 0.5);
	if (bHorizon == false)
		nThMin = (int)((algoWire.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] / m_resolX) * 0.5);
	int nStartX = (bHorizon) ? poArrFoot[0].x : poArrFoot[0].y;
	int nEndX = (bHorizon) ? poArrFoot[1].x : poArrFoot[1].y;
	int nStartY = (bHorizon) ? poArrFoot[0].y : poArrFoot[0].x;
	int nEndY = (bHorizon) ? poArrFoot[1].y : poArrFoot[1].x;
	if (nStartX > nEndX)
	{
		nEndX = (bHorizon) ? poArrFoot[0].x : poArrFoot[0].y;
		nStartX = (bHorizon) ? poArrFoot[1].x : poArrFoot[1].y;
	}
	if (nStartY > nEndY)
	{
		nEndY = (bHorizon) ? poArrFoot[0].y : poArrFoot[0].x;
		nStartY = (bHorizon) ? poArrFoot[1].y : poArrFoot[1].x;
	}
	if (nStartX < 0) nStartX = 0;
	if (nStartY < 0) nStartY = 0;
	if (nEndX < 0) nEndX = 0;
	if (nEndY < 0) nEndY = 0;

	if (bHorizon)
	{
		if (nStartX > nImgWidth) nStartX = nImgWidth;
		if (nStartY > nImgHeight) nStartY = nImgHeight;
		if (nEndX > nImgWidth) nEndX = nImgWidth;
		if (nEndY > nImgHeight) nEndY = nImgHeight;
	}
	else
	{
		if (nStartX > nImgHeight) nStartX = nImgHeight;
		if (nStartY > nImgWidth) nStartY = nImgWidth;
		if (nEndX > nImgHeight) nEndX = nImgHeight;
		if (nEndY > nImgWidth) nEndY = nImgWidth;
	}

	int nCX[WIRE_DRAW_CNT] = { -1, -1 };
	int nCY[WIRE_DRAW_CNT] = { -1, -1 };
	float fArrHMin[WIRE_DRAW_CNT] = { -1.0f, -1.0f };
	float fGap = 0.8f;
	int nStart = 0;
	int nFail = (nThMin / 4) + 1;
	for (int a = 0; a < nW; a++)
	{
		if (nStart == 1) nStart = 0;
		else nStart = 1;
		if (nCY[0] > 0 && nCY[1] > 0 &&
			(nCX[0] == nCX[1] || nCX[0] >= nEndX || nCX[1] <= nStartX))
			break;
		if (nStart == 0)
		{
			if (nCY[nStart] == -1)
			{
				for (int x = 0; x < nW; x++)
				{
					int nSY = 0, nEY = 0, nHCnt = 0;
					float fHSum = 0.0f;
					for (int y = 0; y < nH; y++)
					{
						int nIndex = y * nImgWidth + x;
						if (!bHorizon) nIndex = x * nImgWidth + y;
						if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
						if (ucImage[nIndex] == 0) continue;
						if (nSY == 0 && nEY == 0)
							nSY = y;
						nEY = y;
						fHSum += pfImgSrc[nIndex];		//3D 값 합계 구하기
						nHCnt++;
					}
					if (nSY == 0 || nEY == 0) continue;
					if (nEY - nSY > nThMin) continue;
					nCX[nStart] = x;
					nCY[nStart] = (nSY + nEY) / 2;
					if (nHCnt > 0) fArrHMin[nStart] = fHSum / nHCnt;		//fArrHMin 은 3D 높이 배열
					if (fArrHMin[nStart] < 50.0f) fArrHMin[nStart] = 50.0f;
					break;
				}
			}
			for (int x = nCX[nStart]; x < nEndX; x++)
			{
				int nSY = 0, nEY = 0, nHCnt = 0;
				float fHSum = 0.0f;
				for (int y = 0; y < nH; y++)
				{
					int nIndex = y * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					if (ucImage[nIndex] == 0) continue;
					if (nSY == 0 && nEY == 0)
						nSY = y;
					nEY = y;
					fHSum += pfImgSrc[nIndex];		//3D 높이 값 합계
					nHCnt++;
				}
				if (nEY - nSY > nThMin)
				{
					nCY[nStart] = (nSY + nEY) / 2;
					continue;
				}
				if (nSY > 0 || nEY > 0)
					nCY[nStart] = (nSY + nEY) / 2;
				nSY = 0, nEY = 0;
				int nIndex = nCY[nStart] * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + nCY[nStart];
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				float fH = 0;
				if (nHCnt > 0) fH = fHSum / nHCnt;		//높이 값 저장
				if (fH < fArrHMin[nStart]) fH = fArrHMin[nStart];		//더 높은 높이 값으로 저장
				float fHMin = fH * 0.5f;
				if (fH < 0) fHMin = fH;
				int nGMin = (int)(pucImgSrc[nIndex] * fGap);		//2D 이미지 Intensity 의 0.8배 
				if (nGMin > 255) nGMin = 255;

				int nLim = nCY[nStart] + nThMin;
				if (nLim >= nH) nLim = nH;
				int nFailCnt = 0;
				for (int y = nCY[nStart]; y < nLim; y++)
				{
					int nIndex = y * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					if (pfImgSrc[nIndex] <= -500.0f) continue;
					if (pucImgSrc[nIndex] >= nGMin && pfImgSrc[nIndex] >= fHMin)
					{
						ucImage[nIndex] = 255;		//Image 값 저장
						nEY = y;
					}
					else
					{
						nFailCnt++;
						if (pucImgSrc[nIndex] >= nGMin)
							ucImage[nIndex] = 255;
						if (nFailCnt < nFail) continue;
						break;
					}
				}
				nFailCnt = 0;
				nLim = nCY[nStart] - nThMin - (nEY - nCY[nStart]);
				if (nLim < 0) nLim = 0;
				for (int y = nCY[nStart]; y >= nLim; y--)
				{
					int nIndex = y * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					if (pfImgSrc[nIndex] <= -500.0f) continue;
					if (pucImgSrc[nIndex] >= nGMin && pfImgSrc[nIndex] >= fHMin)
					{
						ucImage[nIndex] = 255;
						nSY = y;
					}
					else
					{
						nFailCnt++;
						if (pucImgSrc[nIndex] >= nGMin)
							ucImage[nIndex] = 255;
						if (nFailCnt < nFail) continue;
						break;
					}
				}
				if (nSY == 0 || nEY == 0) continue;
				nCX[nStart] = x + 1;
				nCY[nStart] = (nSY + nEY) / 2;
				break;
			}
		}
		else
		{
			if (nCY[nStart] == -1)
			{
				for (int x = nW - 1; x >= 0; x--)
				{
					int nSY = 0, nEY = 0, nHCnt = 0;
					float fHSum = 0.0f;
					for (int y = 0; y < nH; y++)
					{
						int nIndex = y * nImgWidth + x;
						if (!bHorizon) nIndex = x * nImgWidth + y;
						if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
						if (ucImage[nIndex] == 0) continue;
						if (nSY == 0 && nEY == 0)
							nSY = y;
						nEY = y;
						fHSum += pfImgSrc[nIndex];
						nHCnt++;
					}
					if (nSY == 0 || nEY == 0) continue;
					if (nEY - nSY > nThMin) continue;
					nCX[nStart] = x;
					nCY[nStart] = (nSY + nEY) / 2;
					if (nHCnt > 0) fArrHMin[nStart] = fHSum / nHCnt;
					if (fArrHMin[nStart] < 50.0f) fArrHMin[nStart] = 50.0f;
					break;
				}
			}
			for (int x = nCX[nStart]; x >= nStartX; x--)
			{
				int nSY = 0, nEY = 0, nHCnt = 0;
				float fHSum = 0.0f;
				for (int y = 0; y < nH; y++)
				{
					int nIndex = y * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					if (ucImage[nIndex] == 0) continue;
					if (nSY == 0 && nEY == 0)
						nSY = y;
					nEY = y;
					fHSum += pfImgSrc[nIndex];
					nHCnt++;
				}

				if (nEY - nSY > nThMin)
				{
					nCY[nStart] = (nSY + nEY) / 2;
					continue;
				}
				if (nSY > 0 || nEY > 0)
					nCY[nStart] = (nSY + nEY) / 2;
				nSY = 0, nEY = 0;
				int nIndex = nCY[nStart] * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + nCY[nStart];
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				float fH = 0;
				if (nHCnt > 0) fH = fHSum / nHCnt;
				if (fH < fArrHMin[nStart]) fH = fArrHMin[nStart];
				float fHMin = fH * 0.5f;
				if (fH < 0) fHMin = fH;
				int nGMin = (int)(pucImgSrc[nIndex] * fGap);
				if (nGMin > 255) nGMin = 255;
				int nLim = nCY[nStart] + nThMin;
				if (nLim >= nH) nLim = nH;
				int nFailCnt = 0;
				for (int y = nCY[nStart]; y < nLim; y++)
				{
					int nIndex = y * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					if (pfImgSrc[nIndex] <= -500.0f) continue;
					if (pucImgSrc[nIndex] >= nGMin && pfImgSrc[nIndex] >= fHMin)
					{
						ucImage[nIndex] = 255;
						nEY = y;
					}
					else
					{
						nFailCnt++;
						if (pucImgSrc[nIndex] >= nGMin)
							ucImage[nIndex] = 255;
						if (nFailCnt < nFail) continue;
						break;
					}
				}
				nFailCnt = 0;
				nLim = nCY[nStart] - nThMin - (nEY - nCY[nStart]);
				if (nLim < 0) nLim = 0;
				for (int y = nCY[nStart]; y >= nLim; y--)
				{
					int nIndex = y * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					if (pfImgSrc[nIndex] <= -500.0f) continue;
					if (pucImgSrc[nIndex] >= nGMin && pfImgSrc[nIndex] >= fHMin)
					{
						ucImage[nIndex] = 255;
						nSY = y;
					}
					else
					{
						nFailCnt++;
						if (pucImgSrc[nIndex] >= nGMin)
							ucImage[nIndex] = 255;
						if (nFailCnt < nFail) continue;
						break;
					}
				}
				if (nSY == 0 || nEY == 0) continue;
				nCX[nStart] = x - 1;
				nCY[nStart] = (nSY + nEY) / 2;
				break;
			}
		}
	}
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("BinarizeByFloodFill_Dst.bmp"));
	return TRUE;
}

BOOL CPInsp_WireBonding::FindPoint(UCHAR* ucImgDst, int nImgWidth, int nImgHeight, RstAlgoWire *sRstAlgo, bool bHorizon, double *dA, double *dB, int* nArrFoot, POINT* poArrFoot)
{
	double dArrLineX[WIRE_DRAW_CNT], dArrLineY[WIRE_DRAW_CNT];
	memset(dArrLineX, 0, sizeof(double) * WIRE_DRAW_CNT);
	memset(dArrLineY, 0, sizeof(double) * WIRE_DRAW_CNT);
	dArrLineX[0] = (nArrFoot[0] > -1 && poArrFoot[0].x > 0) ? poArrFoot[0].x : sRstAlgo->m_poWire_TS.x;
	dArrLineY[0] = (nArrFoot[0] > -1 && poArrFoot[0].y > 0) ? poArrFoot[0].y : sRstAlgo->m_poWire_TS.y;
	dArrLineX[1] = (nArrFoot[1] > -1 && poArrFoot[1].x > 0) ? poArrFoot[1].x : sRstAlgo->m_poWire_TE.x;
	dArrLineY[1] = (nArrFoot[1] > -1 && poArrFoot[1].y > 0) ? poArrFoot[1].y : sRstAlgo->m_poWire_TE.y;
	CPInsp::GetGradient(dArrLineX, dArrLineY, (double)WIRE_DRAW_CNT, *dA, *dB, bHorizon);
	if (nArrFoot[0] > -1 && nArrFoot[1] > -1 &&
		poArrFoot[0].x > 0 && poArrFoot[0].y > 0 &&
		poArrFoot[1].x > 0 && poArrFoot[1].y > 0)
		return TRUE;
	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	bool bL = (dArrLineX[1] > dArrLineX[0]) ? true : false;
	bool bT = (dArrLineY[1] > dArrLineY[0]) ? true : false;
	if (bHorizon == false)
	{
		bL = (dArrLineX[1] > dArrLineX[0]) ? true : false;
		bT = (dArrLineY[1] > dArrLineY[0]) ? true : false;
	}
	for (int nDot = 0; nDot < WIRE_DRAW_CNT; nDot++)
	{
		if (nArrFoot[nDot] > -1 && poArrFoot[nDot].x > 0 && poArrFoot[nDot].y > 0)
			continue;
		if (nDot > 0)
		{
			bL = !bL;
			bT = !bT;
		}
		int nCX = -1;
		int nCYSum = 0;
		int nCnt = 0;
		int nH_Buf = 30;
		if (nH_Buf > nH) nH = nH_Buf;
		if (bL)
		{
			for (int x = 0; x < nW; x++)
			{
				int nT = -1, nB = -1;
				if (bT)
				{
					for (int y = 0; y < nH; y++)
					{
						int nIndex = (bHorizon) ? y * nImgWidth + x : x * nImgWidth + y;
						if (ucImgDst[nIndex] == 0) continue;
						if (nT == -1) nT = y;
						nB = y;
						continue;
					}
				}
				else
				{
					for (int y = nH - 1; y >= 0; y--)
					{
						int nIndex = (bHorizon) ? y * nImgWidth + x : x * nImgWidth + y;
						if (ucImgDst[nIndex] == 0) continue;
						if (nT == -1) nT = y;
						nB = y;
						continue;
					}
				}
				if (nT == -1 || nB == -1) continue;
				if (nCX == -1) nCX = x;
				if (nCnt > 0 && (nCYSum / nCnt * 0.9) > ((nB + nT) / 2))
					break;
				nCYSum += ((nB + nT) / 2);
				nCnt++;
				if (nCnt > nH_Buf) break;
			}
		}
		else
		{
			for (int x = nW - 1; x >= 0; x--)
			{
				int nT = -1, nB = -1;
				if (bT)
				{
					for (int y = 0; y < nH; y++)
					{
						int nIndex = (bHorizon) ? y * nImgWidth + x : x * nImgWidth + y;
						if (ucImgDst[nIndex] == 0) continue;
						if (nT == -1) nT = y;
						nB = y;
						continue;
					}
				}
				else
				{
					for (int y = nH - 1; y >= 0; y--)
					{
						int nIndex = (bHorizon) ? y * nImgWidth + x : x * nImgWidth + y;
						if (ucImgDst[nIndex] == 0) continue;
						if (nT == -1) nT = y;
						nB = y;
						continue;
					}
				}
				if (nT == -1 || nB == -1) continue;
				if (nCX == -1) nCX = x;
				if (nCnt > 0 && (nCYSum / nCnt * 0.9) > ((nB + nT) / 2))
					break;
				nCYSum += ((nB + nT) / 2);
				nCnt++;
				if (nCnt > nH_Buf) break;
			}
		}
		if (nCnt == 0) return FALSE;
		dArrLineX[nDot] = (bHorizon) ? nCX : nCYSum / nCnt;
		dArrLineY[nDot] = (bHorizon) ? nCYSum / nCnt : nCX;
	}
	CPInsp::GetGradient(dArrLineX, dArrLineY, (double)WIRE_DRAW_CNT, *dA, *dB, bHorizon);
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("Find_Point.bmp"));
	return TRUE;
}

double CPInsp_WireBonding::InspectionWireTh(UCHAR* pucImage, int nImgWidth, int nImgHeight, bool bHorizon, double dA, double dB, double *dMaxTh)
{
#if _DEBUG
	cv::Mat src(nImgHeight, nImgWidth, CV_8UC1, pucImage);
	cv::Mat OutPut3Ch[] = { src ,src ,src };
	cv::Mat c;
	cv::merge(OutPut3Ch, 3, c);
#endif
	double dAvgTh = 0;
	double dSumTh = 0;
	int nCnt = 0;
	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	int nX = 0, nY = 0, nXX = 0, nYY = 0;


	for (int a = 0; a < nW; a++)
	{
		double dY_Buf = dA * a + dB;
		int nY_Buf = (int)RounD(dY_Buf);
		if (nY_Buf < 0 || nY_Buf > nH) continue;
		double dA2 = 0;
		if (dA != 0) dA2 = -1.0 / dA;
		double dB2 = dY_Buf - (dA2 * a);
		int nIdx = nY_Buf * nImgWidth + a;
		if (!bHorizon) nIdx = a * nImgWidth + nY_Buf;
		if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
		int nMinBX = -1, nMinBY = -1, nMaxBX = -1, nMaxBY = -1;
		for (int a2 = 0; a2 < nH; a2++)
		{
			int nYTotal = 1;
			int nM = 1;
			if (dA2 == 0)
			{
					//수평일 경우에는 한줄만 검사.
					//nYTotal = nH;
				nY_Buf = a;
			}
			else
			{
				dY_Buf = (a2 - dB2) / dA2;
				nY_Buf = (int)RounD(dY_Buf);
				if (nY_Buf < 0 || nY_Buf > nW) continue;
				double dY_Buf2 = ((a2 + 1) - dB2) / dA2;
				int nY_Buf2 = (int)RounD(dY_Buf2);
				if (nY_Buf2 >= 0 && nY_Buf2 < nW)
				{
					nYTotal = nY_Buf2 - nY_Buf;
					if (nYTotal < 0)
					{
						nM = -1;
						nYTotal *= nM;
					}
				}
			}
			if (nYTotal == 0) nYTotal = 1;
			for (int a3 = 0; a3 < nYTotal; a3++)
			{
				nIdx = (a2 + (a3 * nM)) * nImgWidth + nY_Buf;
				if (!bHorizon) nIdx = nY_Buf * nImgWidth + (a2 + (a3 * nM));

				if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
				if (pucImage[nIdx] == 0) continue;
				if (nMinBY == -1)
				{
					nMinBX = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
					nMinBY = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
				}
				int nMaxBXTemp = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
				int nMaxBYTemp = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
				if (nMaxBXTemp == nImgWidth || nMaxBYTemp == nImgHeight)
					continue;
				nMaxBX = nMaxBXTemp;
				nMaxBY = nMaxBYTemp;
#if _DEBUG
 					cv::circle(c, cv::Point(nMinBX, nMinBY), 1, cv::Scalar(255, 0, 0), 1, 8);
 					cv::circle(c, cv::Point(nMaxBX, nMaxBY), 1, cv::Scalar(255, 0, 0), 1, 8);
					cv::line(c, cv::Point(nMinBX, nMinBY), cv::Point(nMaxBX, nMaxBY), cv::Scalar(255, 0, 0), 3, 8);
#endif
			}
		}
		if (nMinBX < 0 || nMaxBX < 0 || nMinBY < 0 || nMaxBY < 0)
			continue;
		if (nMaxBX == nImgWidth || nMaxBY == nImgHeight)
			continue;
		double dThickness = sqrt(pow((nMaxBX - nMinBX), 2) + pow((nMaxBY - nMinBY), 2));		//두 점 사이의 거리 구하기

		if (dThickness <= 0) continue;
			if (dThickness > *dMaxTh)
			{
#if _DEBUG
				nX = nMinBX;
				nY = nMinBY;
				nXX = nMaxBX;
 				nYY = nMaxBY;
#endif
				*dMaxTh = dThickness;
			}
		dSumTh += dThickness;
		nCnt++;
	}
	
#if _DEBUG
	cv::line(c, cv::Point(nX, nY), cv::Point(nXX, nYY), cv::Scalar(0, 0, 255), 3, 8);
#endif


	dAvgTh = 0;
	if (nCnt > 0) dAvgTh = dSumTh / nCnt;
	return dAvgTh;
}

	
double CPInsp_WireBonding::InspectionDeleteWireTh(UCHAR* pucImage, int nImgWidth, int nImgHeight, bool bHorizon, double dA, double dB, double *dMaxTh, std::vector<cv::Point> &ThCenterPt )
{
#if _DEBUG
	cv::Mat src(nImgHeight, nImgWidth, CV_8UC1, pucImage);
	cv::Mat OutPut3Ch[] = { src ,src ,src };
	cv::Mat c;
	cv::merge(OutPut3Ch, 3, c);
#endif
	//std::vector<cv::Point> ThCenterPt;
	double dAvgTh = 0;
	double dSumTh = 0;
	int nCnt = 0;
	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	int nX = 0, nY = 0, nXX = 0, nYY = 0;


	for (int a = 0; a < nW; a++)
	{
		double dY_Buf = dA * a + dB;
		int nY_Buf = (int)RounD(dY_Buf);
		if (nY_Buf < 0 || nY_Buf > nH) continue;
		double dA2 = 0;
		if (dA != 0) dA2 = -1.0 / dA;
		double dB2 = dY_Buf - (dA2 * a);
		int nIdx = nY_Buf * nImgWidth + a;
		if (!bHorizon) nIdx = a * nImgWidth + nY_Buf;
		if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
		int nMinBX = -1, nMinBY = -1, nMaxBX = -1, nMaxBY = -1;
		for (int a2 = 0; a2 < nH; a2++)
		{
			int nYTotal = 1;
			int nM = 1;
			if (dA2 == 0)
			{
				//수평일 경우에는 한줄만 검사.
				//nYTotal = nH;
				nY_Buf = a;
			}
			else
			{
				dY_Buf = (a2 - dB2) / dA2;
				nY_Buf = (int)RounD(dY_Buf);
				if (nY_Buf < 0 || nY_Buf > nW) continue;
				double dY_Buf2 = ((a2 + 1) - dB2) / dA2;
				int nY_Buf2 = (int)RounD(dY_Buf2);
				if (nY_Buf2 >= 0 && nY_Buf2 < nW)
				{
					nYTotal = nY_Buf2 - nY_Buf;
					if (nYTotal < 0)
					{
						nM = -1;
						nYTotal *= nM;
					}
				}
			}
			if (nYTotal == 0) nYTotal = 1;
			for (int a3 = 0; a3 < nYTotal; a3++)
			{
				nIdx = (a2 + (a3 * nM)) * nImgWidth + nY_Buf;
				if (!bHorizon) nIdx = nY_Buf * nImgWidth + (a2 + (a3 * nM));

				if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
				if (pucImage[nIdx] == 0) continue;
				if (nMinBY == -1)
				{
					nMinBX = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
					nMinBY = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
				}
				int nMaxBXTemp = (bHorizon) ? nY_Buf : (a2 + (a3 * nM));
				int nMaxBYTemp = (bHorizon) ? (a2 + (a3 * nM)) : nY_Buf;
				if (nMaxBXTemp == nImgWidth || nMaxBYTemp == nImgHeight)
					continue;
				nMaxBX = nMaxBXTemp;
				nMaxBY = nMaxBYTemp;
#if _DEBUG
				cv::circle(c, cv::Point(nMinBX, nMinBY), 1, cv::Scalar(255, 0, 0), 1, 8);
				cv::circle(c, cv::Point(nMaxBX, nMaxBY), 1, cv::Scalar(255, 0, 0), 1, 8);
				cv::line(c, cv::Point(nMinBX, nMinBY), cv::Point(nMaxBX, nMaxBY), cv::Scalar(255, 0, 0), 3, 8);
#endif
			}
		}
		if (nMinBX < 0 || nMaxBX < 0 || nMinBY < 0 || nMaxBY < 0)
			continue;
		if (nMaxBX == nImgWidth || nMaxBY == nImgHeight)
			continue;
		double dThickness = sqrt(pow((nMaxBX - nMinBX), 2) + pow((nMaxBY - nMinBY), 2));		//두 점 사이의 거리 구하기
		ThCenterPt.emplace_back(cv::Point((nMinBX + nMaxBX) / 2, (nMinBY + nMaxBY) / 2));
		if (dThickness <= 0) continue;
		if (dThickness > *dMaxTh)
		{
#if _DEBUG
			nX = nMinBX;
			nY = nMinBY;
			nXX = nMaxBX;
			nYY = nMaxBY;

#endif
			*dMaxTh = dThickness;
		}
		dSumTh += dThickness;
		nCnt++;
	}
#if _DEBUG
	cv::line(c, cv::Point(nX, nY), cv::Point(nXX, nYY), cv::Scalar(0, 0, 255), 3, 8);
	for (int i = 0; i < ThCenterPt.size(); i++)
	{
		cv::circle(c, ThCenterPt[i], 1, cv::Scalar(220, 170, 255), 1, 8);
	}
#endif
	dAvgTh = 0;
	if (nCnt > 0) dAvgTh = dSumTh / nCnt;
	return dAvgTh;
}

BOOL CPInsp_WireBonding::CheckWire(AlgoWire sAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucImgDst, RstAlgoWire *sRstAlgo, bool bHorizon, double dA, double dB, bool bCheck)
{
	//예외 처리
	double dAngle = sWndAlgoImg.dAngle;
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return FALSE;

	//윈도우 크기의 입력 이미지 Blob 진행 ( All Blob )
	UCHAR *pucBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucBlob, nImgWidth * nImgHeight);
	memset(pucBlob, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);

	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucImgDst, pucBlob, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix);	//All Blob ÁøÇà
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("CheckWire_Bin.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucBlob, nImgWidth, nImgHeight, _T("CheckWire_BLOB.bmp"));

	if (nCntBlob == 0 || (nCntBlob == 1 && bCheck == false))	//Blob결과 라벨이 0개일 경우
	{
		memcpy(ucImgDst, pucBlob, sizeof(UCHAR) * nImgWidth * nImgHeight);
		Delete_1DArray(&pucBlob);
		return TRUE;
	}
	else if (nCntBlob == 1 && bCheck)		//Blob결과 라벨이 1개일 경우
	{
		double dMaxTh = 0.0;

		//이진화된 Wire 이미지에서 와이어 두께 측정
		double dAvgTh = InspectionWireTh(ucImgDst, nImgWidth, nImgHeight, bHorizon, dA, dB, &dMaxTh);

		double dTh = sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Max] / m_resolX;
		if (sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Max] > sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Default] * 3.0)
			dTh = sAlgo.m_fArrOptionValue[eWIRE_Thickness][eMMD_Default] / m_resolX;

		//측정된 와이어 두께가 티칭된 Spec을(1.3배 이상) 만족하는 경우
		if (dAvgTh > dTh * 1.3)
		{
			int nW = (bHorizon) ? nImgWidth : nImgHeight;
			int nH = (bHorizon) ? nImgHeight : nImgWidth;
			int nWGap = 10;
			if (nWGap >= nW * nWGap) nWGap = 0;

			double dArea = 0, dCX = 0, dCY = 0;
			CRect rcBlob;
			m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);

			int nStartX = (bHorizon) ? rcBlob.left + nWGap : rcBlob.top + nWGap;
			int nEndX = (bHorizon) ? rcBlob.right - nWGap : rcBlob.bottom - nWGap;
			if (nStartX < nWGap) nStartX = nWGap;
			if (nEndX < nWGap) nEndX = nWGap;
			if (nStartX > nW - nWGap) nStartX = nW - nWGap;
			if (nStartX >= nEndX) nEndX = nStartX + 1;
			if (nEndX > nW - nWGap) nEndX = nW - nWGap;
			for (int x = nStartX; x < nEndX; x++)
			{
				int nS_1 = -1, nE_1 = -1;
				for (int y = 0; y < nH; y++)
				{
					int nIndex = y * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + y;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					if (ucImgDst[nIndex] == 0)
						continue;
					if (nS_1 == -1) nS_1 = y;
					nE_1 = y;
				}
				if (nS_1 == -1 || nE_1 == -1 || nE_1 - nS_1 <= dTh * 0.3) continue;
				int nRemove = (nE_1 - nS_1) * 0.3;
				if (nRemove == 0) nRemove = 1;
				for (int a = 0; a < nRemove; a++)
				{
					int nY = ((nS_1 + nE_1) / 2) - (nRemove / 2) + a;
					if (nY < 0 || nY > nH) break;
					int nIndex = nY * nImgWidth + x;
					if (!bHorizon) nIndex = x * nImgWidth + nY;
					if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
					ucImgDst[nIndex] = 0;
				}
			}
			m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("CheckWire_BLOB_1.bmp"));
			memset(pucBlob, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);

			//윈도우 크기의 입력 이미지 Blob 진행 ( Area 100이상 )
			nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucImgDst, pucBlob, nImgWidth, nImgHeight, 100, FALSE, TRUE, 0, eSelectMix);
			m_pProcMilAlgo->SaveWorkImg(pucBlob, nImgWidth, nImgHeight, _T("CheckWire_BLOB_2.bmp"));
			if (nCntBlob <= 1)
			{
				Delete_1DArray(&pucBlob);
				return TRUE;
			}
		}
	}

	/*int nSearchSizeX = sAlgo.m_fArrOptionValue[eWIRE_ShiftX][eMMD_Default] / m_resolX;
	int nSearchSizeY = sAlgo.m_fArrOptionValue[eWIRE_ShiftY][eMMD_Default] / m_resolY;
	if (dAngle == 90 || dAngle == 270)
	{
		nSearchSizeY = sAlgo.m_fArrOptionValue[eWIRE_ShiftX][eMMD_Default] / m_resolX;
		nSearchSizeX = sAlgo.m_fArrOptionValue[eWIRE_ShiftY][eMMD_Default] / m_resolY;
	}
	if (nSearchSizeX <= 10) nSearchSizeX = 10;
	if (nSearchSizeY <= 10) nSearchSizeY = 10;
	if (nSearchSizeX > nImgWidth) nSearchSizeX = nImgWidth;
	if (nSearchSizeY > nImgHeight) nSearchSizeY = nImgHeight;*/

	// Define
	const int nMaxCnt = 100000;
	long pLebel[nMaxCnt];
	USHORT* LabelImage = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &LabelImage, nImgWidth * nImgHeight);
	memset(pLebel, 0, sizeof(long) * nMaxCnt);
	memset(LabelImage, 0, sizeof(USHORT) * nImgWidth * nImgHeight);
	m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);


	int n1 = 0, n1_2 = 0, n2 = 0, n2_1 = 0;
	double dDist1 = 0, dDist1_2 = -44411144, dDist2 = 0, dDist2_1 = -44411144;

	memset(ucImgDst, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
	for (int a = 0; a < nCntBlob; a++)
	{
		CPInsp::GetLabelImage(pLebel[a], LabelImage, nImgWidth, nImgHeight, _T("Find_Label.bmp"));

		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIndex = y * nImgWidth + x;
				if (pLebel[a] != LabelImage[nIndex])
					continue;

				//double dDistBlobToWireStPoint = sqrt(pow((sRstAlgo->m_poWire_TS.x - x), 2) + pow((sRstAlgo->m_poWire_TS.y - y), 2));
				//double dDistBlobToWireEndPoint = sqrt(pow((sRstAlgo->m_poWire_TE.x - x), 2) + pow((sRstAlgo->m_poWire_TE.y - y), 2));
				
				if (pLebel[a] == LabelImage[nIndex])
					ucImgDst[nIndex] = 255;
			}
		}
	}
	
	m_pProcMilAlgo->SaveWorkImg(ucImgDst, nImgWidth, nImgHeight, _T("CheckWire_Rst.bmp"));
	Delete_1DArray(&pucBlob);
	Delete_1DArray(&LabelImage);
	return TRUE;
}

BOOL CPInsp_WireBonding::CheckWire2(UCHAR* ucImgDst, WndAlgoImg &sWndAlgoImg,int* nArrFoot, POINT* poArrFoot)
{
	if ((ucImgDst == NULL) || (nArrFoot == NULL) || (poArrFoot == NULL) )
		return false;
	std::vector<CRect> vRectBlob;
	m_pProcMilAlgo->GetBlobResult_RectsV(vRectBlob);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	cv::Mat srcTest(nImgHeight, nImgWidth, CV_8UC1, ucImgDst);
	for (int nRectCnt = 0; nRectCnt < vRectBlob.size(); nRectCnt++)
	{
		if (vRectBlob[nRectCnt].PtInRect(poArrFoot[0]) && nArrFoot[0] != -1 && vRectBlob[nRectCnt].PtInRect(poArrFoot[1]) && nArrFoot[1] != -1) // 두개를찾았을경우 Foot Point 기준 조건참일경우 해당이미지를사용
		{
			memset(ucImgDst, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
			const int nMaxCnt = 100000;
			long pLebel[nMaxCnt];
			USHORT* LabelImage = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &LabelImage, nImgWidth * nImgHeight);
			memset(pLebel, 0, sizeof(long) * nMaxCnt);
			memset(LabelImage, 0, sizeof(USHORT) * nImgWidth * nImgHeight);
			m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);

			for (int y = 0; y < nImgHeight; y++)
			{
				for (int x = 0; x < nImgWidth; x++)
				{
					int nIndex = y * nImgWidth + x;
					if (pLebel[nRectCnt] != LabelImage[nIndex])
						continue;

					if (pLebel[nRectCnt] == LabelImage[nIndex])
						ucImgDst[nIndex] = 255;
				}
			}

			Delete_1DArray(&LabelImage);
		}
	}

	return true;
}


void CPInsp_WireBonding::BinarizeAddData(float *pfImgSrc, UCHAR *ucImgSrc, UCHAR *ucImage, int nImgWidth, int nImgHeight, bool bHorizon)
{
	if ((pfImgSrc == NULL) || (ucImage == NULL) || (ucImage == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return;
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("BinarizeAddData.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("BinarizeAddData_3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg(ucImgSrc, nImgWidth, nImgHeight, _T("BinarizeAddData_2D.bmp"));
	float fSum = 0.0f, fCnt = 0.0f, fMax = 0.0f;
	for (int a = 0; a < nImgWidth * nImgHeight; a++)
	{
		if (ucImage[a] == 0 || pfImgSrc[a] < 0.0f) continue;
		fSum += pfImgSrc[a];
		fCnt++;
		if (pfImgSrc[a] > fMax) fMax = pfImgSrc[a];
	}
	float fGapH = 0.6f;
	float fGapG = 0.8f;
	float fAvg = fSum / fCnt;
	float fLim = fAvg * fGapH;
	if (fAvg > fLim) fLim = (fMax + fAvg) / 2.0f;
	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	for (int x = 0; x < nW; x++)
	{
		for (int y = 0; y < nH; y++)
		{
			int nIndex = y * nImgWidth + x;
			if (!bHorizon) nIndex = x * nImgWidth + y;
			if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
			if (ucImage[nIndex] == 0) continue;
			float fH = pfImgSrc[nIndex];
			UCHAR ucG = ucImgSrc[nIndex];
			if (fH < fAvg || ucG < 100) break;
			for (int y2 = y - 1; y2 > 0; y2--)
			{
				nIndex = y2 * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y2;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) break;
				float fH2 = pfImgSrc[nIndex];
				UCHAR ucG2 = ucImgSrc[nIndex];
				if (fH2 < 0 || ucG < 100) break;
				if (fH2 < fLim || ucG < 100) break;
				if (ucG2 < ucG * fGapG) break;
				ucImage[nIndex] = 255;
			}
			break;
		}
		for (int y = nH - 1; y > 0; y--)
		{
			int nIndex = y * nImgWidth + x;
			if (!bHorizon) nIndex = x * nImgWidth + y;
			if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
			if (ucImage[nIndex] == 0) continue;
			float fH = pfImgSrc[nIndex];
			UCHAR ucG = ucImgSrc[nIndex];
			if (fH < fAvg || ucG < 100) break;
			for (int y2 = y + 1; y2 < nH; y2++)
			{
				nIndex = y2 * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y2;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) break;
				float fH2 = pfImgSrc[nIndex];
				UCHAR ucG2 = ucImgSrc[nIndex];
				if (fH2 < 0 || ucG < 100) break;
				if (fH2 < fLim || ucG < 100) break;
				if (ucG2 < ucG * fGapG) break;
				ucImage[nIndex] = 255;
			}
			break;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("BinarizeAddData_Rst.bmp"));
}

void CPInsp_WireBonding::BinarizeDelData(UCHAR *ucImage, int nImgWidth, int nImgHeight, POINT* poArrFoot, bool bHorizon, double dA, double dB, int nRemove)
{
	if ((ucImage == NULL) || (poArrFoot == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return;
	if ((poArrFoot[0].x == 0 && poArrFoot[0].y == 0) ||
		(poArrFoot[1].x == 0 && poArrFoot[1].y == 0))
		return;
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("BinarizeDelData.bmp"));
	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	bool bUp = (bHorizon) ? poArrFoot[0].x < poArrFoot[1].x : poArrFoot[0].y < poArrFoot[1].y;
	if (nRemove < 0 || nRemove > nW * 0.5) return;
	for (int nDot = 0; nDot < WIRE_DRAW_CNT; nDot++)
	{
		if (nDot > 0) bUp = !bUp;
		if (poArrFoot[nDot].x == 0 && poArrFoot[nDot].y == 0)
			continue;
		int nLim = (bHorizon) ? poArrFoot[nDot].x : poArrFoot[nDot].y;
		if (bUp) nLim += nRemove;
		else nLim -= nRemove;
		if (nLim < 0) nLim = 0;
		if (nLim > nW) nLim = nW;
		int nS = 0, nE = nLim;
		if (bUp == false)
		{
			nS = nLim;
			nE = nW;
		}
		for (int x = nS; x < nE; x++)
		{
			for (int y = 0; y < nH; y++)
			{
				int nIdx = y * nImgWidth + x;
				if (!bHorizon) nIdx = x * nImgWidth + y;
				ucImage[nIdx] = 0;
			}
		}
	}
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("BinarizeDelData_Rst.bmp"));
}

double CPInsp_WireBonding::InspInterval(int nWireNum, WireRst* Wirerst, UCHAR *ucImage, int nImgWidth, int nImgHeight)
{

	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("InspInterval.bmp"));
	double *dArrT = NULL;
	POINTF *poArrT_Min = NULL;
	POINTF *poArrT_Max = NULL;
	int nMaxArea = max(nImgWidth, nImgHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrT, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &poArrT_Min, nMaxArea);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &poArrT_Max, nMaxArea);
	for (int nWire = 0; nWire < nWireNum; nWire++)
	{
		AlgoWire algoWire = *Wirerst[nWire].m_Wire;
		RstAlgoWire *sRstAlgo = Wirerst[nWire].m_RstWire;
		sRstAlgo->m_fArrRst[eWIRE_Interval] = nImgWidth * nImgHeight;
		if (sRstAlgo->m_nWireCnt == 0 || ((algoWire.m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Interval) != eWIRE_Data_Interval) ||
			ucImage == NULL || nImgWidth < 1 || nImgHeight < 1 || sRstAlgo == NULL)
			continue;

		if (sRstAlgo->m_nWireCnt == 0)
			continue;

		bool bHorizon = sRstAlgo->m_bArrHo;
		double dA = sRstAlgo->m_poArrAB.x;
		double dB = sRstAlgo->m_poArrAB.y;
		memset(dArrT, 0, nMaxArea * sizeof(double));
		memset(poArrT_Min, 0, nMaxArea * sizeof(POINTF));
		memset(poArrT_Max, 0, nMaxArea * sizeof(POINTF));
		double dRstInterval = nMaxArea;
		int nData = 200 + sRstAlgo->m_nWireCnt; // 150 + (sRstAlgo->m_nWireCnt * 10);
		int nW = (bHorizon) ? nImgWidth : nImgHeight;
		int nH = (bHorizon) ? nImgHeight : nImgWidth;
		int nArrCnt = 0;
		for (int x = 0; x < nW; x++)
		{
			double dY = dA * x + dB;
			int nY = (int)RounD(dY);
			if (nY < 0 || nY > nH) continue;
			double dA2 = 0;
			if (dA != 0) dA2 = -1.0 / dA;
			double dB2 = dY - (dA2 * x);
			int nIdx = nY * nImgWidth + x;
			if (!bHorizon) nIdx = x * nImgWidth + nY;
			if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
			bool bContinue = false;
			for (int y = 0; y < nH; y++)
			{
				int nIdx2 = y * nImgWidth + x;
				if (!bHorizon) nIdx2 = x * nImgWidth + y;
				if (ucImage[nIdx2] == nData)
				{
					bContinue = true;
					break;
				}
			}
			if (bContinue == false) continue;
			byte bWhite = 0;
			for (int y = 0; y < nH; y++)
			{
				int nYTotal = 1;
				int nM = 1;
				if (dA2 == 0)
				{
					//nYTotal = nH;
					nY = x;
				}
				else
				{
					dY = (y - dB2) / dA2;
					nY = (int)RounD(dY);
					if (nY < 0 || nY > nW) continue;
					double dY_Buf2 = ((y + 1) - dB2) / dA2;
					int nY_Buf2 = (int)RounD(dY_Buf2);
					if (nY_Buf2 >= 0 && nY_Buf2 < nW)
					{
						nYTotal = nY_Buf2 - nY;
						if (nYTotal < 0)
						{
							nM = -1;
							nYTotal *= nM;
						}
					}
				}
				if (nYTotal == 0) nYTotal = 1;
				for (int a3 = 0; a3 < nYTotal; a3++)
				{
					nIdx = (y + (a3 * nM)) * nImgWidth + nY;
					if (!bHorizon) nIdx = nY * nImgWidth + (y + (a3 * nM));

					if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
					if (bWhite == 0 && ucImage[nIdx] == nData)
					{
						bWhite = 1;
						continue;
					}
					else if (bWhite == 1 && (ucImage[nIdx] == 0 || ucImage[nIdx] == 255))
					{
						bWhite = 2;
						poArrT_Min[nArrCnt].x = (bHorizon) ? nY : (y + (a3 * nM));
						poArrT_Min[nArrCnt].y = (bHorizon) ? (y + (a3 * nM)) : nY;
						poArrT_Max[nArrCnt].x = (bHorizon) ? nY : (y + (a3 * nM));
						poArrT_Max[nArrCnt].y = (bHorizon) ? (y + (a3 * nM)) : nY;
					}
					else if (bWhite == 2 && (ucImage[nIdx] > 0 && ucImage[nIdx] != 255) && ucImage[nIdx] != nData)
					{
						poArrT_Max[nArrCnt].x = (bHorizon) ? nY : (y + (a3 * nM));
						poArrT_Max[nArrCnt].y = (bHorizon) ? (y + (a3 * nM)) : nY;
						dArrT[nArrCnt] = sqrt(pow((poArrT_Min[nArrCnt].x - poArrT_Max[nArrCnt].x) * m_resolX, 2) + pow((poArrT_Min[nArrCnt].y - poArrT_Max[nArrCnt].y) * m_resolY, 2));
						if (dRstInterval > dArrT[nArrCnt]) dRstInterval = dArrT[nArrCnt];
						bWhite = 0;
						nArrCnt++;
					}
				}
			}
		}
		if (nArrCnt == 0) continue;
		int nDivCnt = algoWire.m_nArrData[eWIRE2_DivCnt];
		if (nDivCnt <= 0) nDivCnt = 1;
		if (nDivCnt > WIRE_DOT3_CNT) nDivCnt = WIRE_DOT3_CNT;
		if (nDivCnt > nArrCnt) nDivCnt = nArrCnt;
		int nDivGap = 1;
		int nDivGap2 = 0;
		if (nDivCnt > 1 && nArrCnt > nDivCnt)
		{
			nDivGap = nArrCnt / nDivCnt;
			nDivGap2 = nArrCnt % nDivCnt;
		}
		else
		{
			nDivCnt = 1;
			nDivGap = nArrCnt;
			nDivGap2 = 0;
		}
		if (nDivGap <= 0) nDivGap = 1;
		if (nDivGap2 <= 0) nDivGap2 = 0;
		if (nDivGap > nArrCnt) nDivGap = nArrCnt;
		if (nDivGap2 > nDivCnt) nDivGap2 = 0;
		int nDivIdx = 0;
		bool bMax = (algoWire.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_Max) == m_eWIRE_Data2_Max;
		bMax = true;
		double dMaxDiv = -11111;
		for (int a = 0; a < nDivCnt; a++)
		{
			int nTotal = nDivGap;
			if (nDivGap2 > 0 && a < nDivGap2)
				nTotal++;
			if (nTotal > nArrCnt) nTotal = nArrCnt;
			double dRstBuf = nW * nH;
			POINTF poArrT[WIRE_DRAW_CNT];
			memset(poArrT, 0, sizeof(POINTF) * WIRE_DRAW_CNT);
			for (int b = 0; b < nTotal; b++)
			{
				if (bMax)
				{
					if (dRstBuf > dArrT[nDivIdx])
					{
						dRstBuf = dArrT[nDivIdx];
						poArrT[0].x = poArrT_Min[nDivIdx].x;
						poArrT[0].y = poArrT_Min[nDivIdx].y;
						poArrT[1].x = poArrT_Max[nDivIdx].x;
						poArrT[1].y = poArrT_Max[nDivIdx].y;
					}
				}
				else
				{
					dRstBuf += dArrT[nDivIdx];
					poArrT[0].x += poArrT_Min[nDivIdx].x;
					poArrT[0].y += poArrT_Min[nDivIdx].y;
					poArrT[1].x += poArrT_Max[nDivIdx].x;
					poArrT[1].y += poArrT_Max[nDivIdx].y;
				}
				nDivIdx++;
				if (nDivIdx > nArrCnt) break;
			}
			sRstAlgo->m_fArrValue_I[a] = (bMax) ? dRstBuf : dRstBuf / nTotal;
			if (dMaxDiv == -11111 || dMaxDiv > sRstAlgo->m_fArrValue_I[a])
			{
				dMaxDiv = sRstAlgo->m_fArrValue_I[a];
				sRstAlgo->m_nArrX_I[0] = (bMax) ? poArrT[0].x : poArrT[0].x / nTotal;
				sRstAlgo->m_nArrY_I[0] = (bMax) ? poArrT[0].y : poArrT[0].y / nTotal;
				sRstAlgo->m_nArrX_I[1] = (bMax) ? poArrT[1].x : poArrT[1].x / nTotal;
				sRstAlgo->m_nArrY_I[1] = (bMax) ? poArrT[1].y : poArrT[1].y / nTotal;
			}
			if (sRstAlgo->m_fArrRst[eWIRE_Interval] > sRstAlgo->m_fArrValue_I[a])
				sRstAlgo->m_fArrRst[eWIRE_Interval] = sRstAlgo->m_fArrValue_I[a];
			if (nDivIdx > nArrCnt) break;
		}
	}
	Delete_1DArray(&dArrT);
	Delete_1DArray(&poArrT_Min);
	Delete_1DArray(&poArrT_Max);
	return 0;

}
// Window 를 순서대로 그리지않을경우에를 대비한 부분
double CPInsp_WireBonding::IntervalSort( WireRst* Wirerst,  std::vector<std::pair<int, int>>& vpairIndex)
{
	bool bHorizon = true;
	std::vector<cv::Rect> vTempFootRect;
	

	for (int i = 0; i < m_vTupleWirePoint.size(); i++)// Wire 갯수대로 H V 알고리즘 데이터 확인 
	{
		bHorizon &= ((AlgoWire *)Wirerst[i].m_Wire)->m_bDirectionH;  // 하나라도 False 시엔 Wire 가 X 방향으로 InterVal 계산
	}
	
	if (!bHorizon)
	{
		for (int i = 0; i < m_vTupleWirePoint.size(); i++)
			vpairIndex.push_back({ ((RstAlgoWire *)Wirerst[i].m_RstWire)->m_poWire_Wnd.x,i });	  //  X  기준 Sort
	}
	else
	{
		for (int i = 0; i < m_vTupleWirePoint.size(); i++) //y 기준 sort
			vpairIndex.push_back({ ((RstAlgoWire *)Wirerst[i].m_RstWire)->m_poWire_Wnd.y,i });
	}
	std::sort(vpairIndex.begin(), vpairIndex.end());

	return 0.0;
}
double CPInsp_WireBonding::InspInterval(int nWireNum, WireRst* Wirerst, UCHAR *ucImage, int nImgWidth, int nImgHeight, std::vector<std::vector<cv::Rect>> vFootRects)
{
#ifdef _DEBUG
	cv::Mat inPutImg(nImgHeight, nImgWidth, CV_8UC1, ucImage);
	cv::Mat OutPut3Ch[] = { inPutImg ,inPutImg ,inPutImg };
	cv::Mat IntervalImage;
	cv::merge(OutPut3Ch, 3, IntervalImage);
	
#endif
	std::vector<std::pair<int, int>> vpairSortIndex;
	IntervalSort(Wirerst, vpairSortIndex);
	double dDistance; 
	long long InvalidWire;
	std::vector<long long> vInvalidWire;
	for (int i = 0; i < nWireNum; i++)
	{
		RstAlgoWire *rstCurWire = Wirerst[i].m_RstWire;
		InvalidWire = (rstCurWire->m_fArrRst[eWIRE_Distance] > 0) ? (long long)(void *)rstCurWire : 0;
		rstCurWire->m_fArrRst[eWIRE_Interval] = nImgWidth * nImgHeight;
		if (InvalidWire != 0)
			vInvalidWire.push_back(InvalidWire);
	}

	if ((int)m_vTupleWirePoint.size() > 1)
	{

		
		if (vInvalidWire.size() == (int)m_vTupleWirePoint.size())
		{
			for (int nCurWire = 0; nCurWire < (int)m_vTupleWirePoint.size() - 1; nCurWire++)
			{
				int nNextWire = vpairSortIndex[nCurWire + 1].second;
				RstAlgoWire *rstNextWire = (RstAlgoWire *)vInvalidWire[nNextWire]; // Wirerst[nNextWire].m_RstWire;
				POINTF * ptNextWireCenter = (POINTF *)(get<0>(m_vTupleWirePoint[nNextWire]));
				int nNextWireLoop = (get<2>(m_vTupleWirePoint[nNextWire])) - 1;
				RstAlgoWire *rstCurWire = (RstAlgoWire *)vInvalidWire[vpairSortIndex[nCurWire].second];//Wirerst[vpairSortIndex[nCurWire].second].m_RstWire;
				//rstCurWire->m_fArrRst[eWIRE_Interval] = 999;
				POINTF * ptWireCenter = (POINTF *)(get<1>(m_vTupleWirePoint[vpairSortIndex[nCurWire].second]));
				int nCurWireLoop = (get<2>(m_vTupleWirePoint[vpairSortIndex[nCurWire].second])) - 1;
#ifdef _DEBUG
				cv::rectangle(IntervalImage, vFootRects[vpairSortIndex[nCurWire].second][0], cv::Scalar(255, 255, 0), 3, 8); //Fir Foot
				cv::rectangle(IntervalImage, vFootRects[vpairSortIndex[nCurWire].second][1], cv::Scalar(0, 255, 255), 3, 8); // Sec Foot
#endif
				for (int i = 0; i < nNextWireLoop; i++) // 다음 Wire  와 현재 Wire 의 계산을위한 CenterPoint 거리값 계산.
				{
					int nNextX = ptNextWireCenter[i].x + rstNextWire->m_poWire_Wnd.x;
					int nNextY = ptNextWireCenter[i].y + rstNextWire->m_poWire_Wnd.y;
					for (int j = 0; j < nCurWireLoop; j++) //
					{
						int nX = ptWireCenter[j].x + rstCurWire->m_poWire_Wnd.x;
						int nY = ptWireCenter[j].y + rstCurWire->m_poWire_Wnd.y;

						dDistance = sqrt(pow((nX - nNextX) * m_resolX, 2) + pow((nY - nNextY) * m_resolY, 2));
						if (rstCurWire->m_fArrRst[eWIRE_Interval] > dDistance)
						{
							rstCurWire->m_fArrRst[eWIRE_Interval] = dDistance;
							rstCurWire->m_nArrX_I[0] = nX;
							rstCurWire->m_nArrY_I[0] = nY;
							rstCurWire->m_nArrX_I[1] = nNextX;
							rstCurWire->m_nArrY_I[1] = nNextY;
						}
					}
				}
#ifdef _DEBUG
				cv::line(IntervalImage, cv::Point(rstCurWire->m_nArrX_I[0], rstCurWire->m_nArrY_I[0]),
					cv::Point(rstCurWire->m_nArrX_I[1], rstCurWire->m_nArrY_I[1]), cv::Scalar(0, 0, 255), 3, 8); //최종적으로그려지는Line 
#endif
			}
		}
		

	}
	vpairSortIndex.clear();
	ClearWirePoints();
	return 0.;
}

/* pointInRectangle
		A		D1		B
        *--------------*
	D2	|			   | D3
		|			   |
		*--------------*
		D		D4     C
*/
bool CPInsp_WireBonding::pointInRectangle(cv::Rect cvRectValue, cv::Point m) 
{
	//직사각형이 아니면 아래부분을바꿔야함
	cv::Point A = cv::Point(cvRectValue.x, cvRectValue.y);
	cv::Point B = cv::Point(cvRectValue.x + cvRectValue.width, cvRectValue.y);
	cv::Point D = cv::Point(cvRectValue.x , cvRectValue.y + cvRectValue.height);
	cv::Point C = cv::Point(cvRectValue.x + cvRectValue.width, cvRectValue.y + cvRectValue.height);
	//
	//A -> B 직선방향에서 오른쪽 양수 / 왼쪽 음수/ 선위 0
	cv::Point AB = vect2d(A, B);  float C1 = -1 * (AB.y*A.x + AB.x*A.y); float  D1 = (AB.y*m.x + AB.x*m.y) + C1;
	//A -> D 직선방향에서 오른쪽 양수 / 왼쪽 음수/ 선위 0
	cv::Point AD = vect2d(A, D);  float C2 = -1 * (AD.y*A.x + AD.x*A.y); float D2 = (AD.y*m.x + AD.x*m.y) + C2;
	cv::Point BC = vect2d(B, C);  float C3 = -1 * (BC.y*B.x + BC.x*B.y); float D3 = (BC.y*m.x + BC.x*m.y) + C3;
	cv::Point CD = vect2d(C, D);  float C4 = -1 * (CD.y*C.x + CD.x*C.y); float D4 = (CD.y*m.x + CD.x*m.y) + C4;
	return     0 <= D1 && 0 <= D4 && 0 >= D2 && 0 <= D3;
}
cv::Point CPInsp_WireBonding::vect2d(cv::Point p1, cv::Point p2) 
{
	cv::Point temp;
	temp.x = (p2.x - p1.x);
	temp.y = -1 * (p2.y - p1.y);
	return temp;
}
double CPInsp_WireBonding::InspInterval(AlgoWire algoWire, UCHAR *ucImage, int nImgWidth, int nImgHeight, RstAlgoWire *sRstAlgo)
{
	sRstAlgo->m_fArrRst[eWIRE_Interval] = nImgWidth * nImgHeight;
	if (sRstAlgo->m_nWireCnt == 1 || ((algoWire.m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Interval) != eWIRE_Data_Interval) ||
		ucImage == NULL || nImgWidth < 1 || nImgHeight < 1 || sRstAlgo == NULL)
		return sRstAlgo->m_fArrRst[eWIRE_Interval];

	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("InspInterval.bmp"));
	double *dArrT = NULL;
	POINTF *poArrT_Min = NULL;
	POINTF *poArrT_Max = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrT, nImgWidth * nImgHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &poArrT_Min, nImgWidth * nImgHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &poArrT_Max, nImgWidth * nImgHeight);
	for (int nWire = 0; nWire < sRstAlgo->m_nWireCnt; nWire++)
	{
		bool bHorizon = sRstAlgo->m_bArrHo;
		double dA = sRstAlgo->m_poArrAB.x;
		double dB = sRstAlgo->m_poArrAB.y;
		memset(dArrT, 0, nImgWidth * nImgHeight * sizeof(double));
		memset(poArrT_Min, 0, nImgWidth * nImgHeight * sizeof(POINTF));
		memset(poArrT_Max, 0, nImgWidth * nImgHeight * sizeof(POINTF));
		double dRstInterval = nImgWidth * nImgHeight;
		int nData = (nWire + 1) * 10;
		int nW = (bHorizon) ? nImgWidth : nImgHeight;
		int nH = (bHorizon) ? nImgHeight : nImgWidth;
		int nArrCnt = 0;
		for (int x = 0; x < nW; x++)
		{
			double dY = dA * x + dB;
			int nY = (int)RounD(dY);
			if (nY < 0 || nY > nH) continue;
			double dA2 = 0;
			if (dA != 0) dA2 = -1.0 / dA;
			double dB2 = dY - (dA2 * x);
			int nIdx = nY * nImgWidth + x;
			if (!bHorizon) nIdx = x * nImgWidth + nY;
			if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
			bool bContinue = false;
			for (int y = 0; y < nH; y++)
			{
				int nIdx2 = y * nImgWidth + x;
				if (!bHorizon) nIdx2 = x * nImgWidth + y;
				if (ucImage[nIdx2] == nData)
				{
					bContinue = true;
					break;
				}
			}
			if (bContinue == false) continue;
			byte bWhite = 0;
			for (int y = 0; y < nH; y++)
			{
				int nYTotal = 1;
				int nM = 1;
				if (dA2 == 0)
				{
					nYTotal = nH;
					nY = x;
				}
				else
				{
					dY = (y - dB2) / dA2;
					nY = (int)RounD(dY);
					if (nY < 0 || nY > nW) continue;
					double dY_Buf2 = ((y + 1) - dB2) / dA2;
					int nY_Buf2 = (int)RounD(dY_Buf2);
					if (nY_Buf2 >= 0 && nY_Buf2 < nW)
					{
						nYTotal = nY_Buf2 - nY;
						if (nYTotal < 0)
						{
							nM = -1;
							nYTotal *= nM;
						}
					}
				}
				if (nYTotal == 0) nYTotal = 1;
				for (int a3 = 0; a3 < nYTotal; a3++)
				{
					nIdx = (y + (a3 * nM)) * nImgWidth + nY;
					if (!bHorizon) nIdx = nY * nImgWidth + (y + (a3 * nM));

					if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;
					if (bWhite == 0 && ucImage[nIdx] == nData)
					{
						bWhite = 1;
						continue;
					}
					else if (bWhite == 1 && ucImage[nIdx] == 0)
					{
						bWhite = 2;
						poArrT_Min[nArrCnt].x = (bHorizon) ? nY : (y + (a3 * nM));
						poArrT_Min[nArrCnt].y = (bHorizon) ? (y + (a3 * nM)) : nY;
						poArrT_Max[nArrCnt].x = (bHorizon) ? nY : (y + (a3 * nM));
						poArrT_Max[nArrCnt].y = (bHorizon) ? (y + (a3 * nM)) : nY;
					}
					else if (bWhite == 2 && ucImage[nIdx] > 0 && ucImage[nIdx] != nData)
					{
						poArrT_Max[nArrCnt].x = (bHorizon) ? nY : (y + (a3 * nM));
						poArrT_Max[nArrCnt].y = (bHorizon) ? (y + (a3 * nM)) : nY;
						dArrT[nArrCnt] = sqrt(pow((poArrT_Min[nArrCnt].x - poArrT_Max[nArrCnt].x) * m_resolX, 2) + pow((poArrT_Min[nArrCnt].y - poArrT_Max[nArrCnt].y) * m_resolY, 2));
						if (dRstInterval > dArrT[nArrCnt]) dRstInterval = dArrT[nArrCnt];
						bWhite = 0;
						nArrCnt++;
					}
				}
			}
		}
		if (nArrCnt == 0) continue;
		int nDivCnt = algoWire.m_nArrData[eWIRE2_DivCnt];
		if (nDivCnt <= 0) nDivCnt = 1;
		if (nDivCnt > WIRE_DOT3_CNT) nDivCnt = WIRE_DOT3_CNT;
		if (nDivCnt > nArrCnt) nDivCnt = nArrCnt;
		int nDivGap = 1;
		int nDivGap2 = 0;
		if (nDivCnt > 1 && nArrCnt > nDivCnt)
		{
			nDivGap = nArrCnt / nDivCnt;
			nDivGap2 = nArrCnt % nDivCnt;
		}
		else
		{
			nDivCnt = 1;
			nDivGap = nArrCnt;
			nDivGap2 = 0;
		}
		if (nDivGap <= 0) nDivGap = 1;
		if (nDivGap2 <= 0) nDivGap2 = 0;
		if (nDivGap > nArrCnt) nDivGap = nArrCnt;
		if (nDivGap2 > nDivCnt) nDivGap2 = 0;
		int nDivIdx = 0;
		bool bMax = (algoWire.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_Max) == m_eWIRE_Data2_Max;
		bMax = true;
		double dMaxDiv = -11111;
		for (int a = 0; a < nDivCnt; a++)
		{
			int nTotal = nDivGap;
			if (nDivGap2 > 0 && a < nDivGap2)
				nTotal++;
			if (nTotal > nArrCnt) nTotal = nArrCnt;
			double dRstBuf = nW * nH;
			POINTF poArrT[WIRE_DRAW_CNT];
			memset(poArrT, 0, sizeof(POINTF) * WIRE_DRAW_CNT);
			for (int b = 0; b < nTotal; b++)
			{
				if (bMax)
				{
					if (dRstBuf > dArrT[nDivIdx])
					{
						dRstBuf = dArrT[nDivIdx];
						poArrT[0].x = poArrT_Min[nDivIdx].x;
						poArrT[0].y = poArrT_Min[nDivIdx].y;
						poArrT[1].x = poArrT_Max[nDivIdx].x;
						poArrT[1].y = poArrT_Max[nDivIdx].y;
					}
				}
				else
				{
					dRstBuf += dArrT[nDivIdx];
					poArrT[0].x += poArrT_Min[nDivIdx].x;
					poArrT[0].y += poArrT_Min[nDivIdx].y;
					poArrT[1].x += poArrT_Max[nDivIdx].x;
					poArrT[1].y += poArrT_Max[nDivIdx].y;
				}
				nDivIdx++;
				if (nDivIdx > nArrCnt) break;
			}
			sRstAlgo->m_fArrValue_I[a] = (bMax) ? dRstBuf : dRstBuf / nTotal;
			if (dMaxDiv == -11111 || dMaxDiv > sRstAlgo->m_fArrValue_I[a])
			{
				dMaxDiv = sRstAlgo->m_fArrValue_I[a];
				sRstAlgo->m_nArrX_I[0] = (bMax) ? poArrT[0].x : poArrT[0].x / nTotal;
				sRstAlgo->m_nArrY_I[0] = (bMax) ? poArrT[0].y : poArrT[0].y / nTotal;
				sRstAlgo->m_nArrX_I[1] = (bMax) ? poArrT[1].x : poArrT[1].x / nTotal;
				sRstAlgo->m_nArrY_I[1] = (bMax) ? poArrT[1].y : poArrT[1].y / nTotal;
			}
			if (sRstAlgo->m_fArrRst[eWIRE_Interval] > sRstAlgo->m_fArrValue_I[a])
				sRstAlgo->m_fArrRst[eWIRE_Interval] = sRstAlgo->m_fArrValue_I[a];
			if (nDivIdx > nArrCnt) break;
		}
	}
	Delete_1DArray(&dArrT);
	Delete_1DArray(&poArrT_Min);
	Delete_1DArray(&poArrT_Max);
	return sRstAlgo->m_fArrRst[eWIRE_Interval];
}

void CPInsp_WireBonding::RemoveThin(UCHAR *ucImage, int nImgWidth, int nImgHeight, bool bHorizon, int nRemove)
{
	if ((ucImage == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return;

	if (nRemove < 5) nRemove = 5;

	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("RemoveThin.bmp"));

	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	if (nRemove > nH * 0.1) return;

	for (int x = 0; x < nW; x++)
	{
		int nS = -1, nE = 0;
		for (int y = 0; y < nH; y++)
		{
			int nIndex = y * nImgWidth + x;
			if (!bHorizon) nIndex = x * nImgWidth + y;
			if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
			if (ucImage[nIndex] == 0) continue;
			if (nS == -1) nS = y;
			nE = y;
		}
		if (nS == -1) continue;
		else if (nE - nS + 1 > nRemove || nE + 1 >= nH) break;
		for (int y = nS; y < nE + 1; y++)
		{
			int nIndex = y * nImgWidth + x;
			if (!bHorizon) nIndex = x * nImgWidth + y;
			if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
			ucImage[nIndex] = 0;
		}
	}
	for (int x = nW - 1; x >= 0; x--)
	{
		int nS = -1, nE = 0;
		for (int y = 0; y < nH; y++)
		{
			int nIndex = y * nImgWidth + x;
			if (!bHorizon) nIndex = x * nImgWidth + y;
			if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
			if (ucImage[nIndex] == 0) continue;
			if (nS == -1) nS = y;
			nE = y;
		}
		if (nS == -1) continue;
		else if (nE - nS + 1 > nRemove || nE + 1 >= nH) break;
		for (int y = nS; y < nE + 1; y++)
		{
			int nIndex = y * nImgWidth + x;
			if (!bHorizon) nIndex = x * nImgWidth + y;
			if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
			ucImage[nIndex] = 0;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("RemoveThin_Rst.bmp"));
}


bool CPInsp_WireBonding::FindFootROIList(CSearchWire_Model* pModel, int FootIndex, std::vector<FootRect>& rst, int nSearchType)
{
	CFoot_Model* pfoot = nullptr;
	std::vector<RECT> ROI;

	if (FootIndex == 0)
	{
		pfoot = pModel->m_pFoot1;
		ROI.insert(ROI.begin(), pModel->m_ROI_Foot1.begin(), pModel->m_ROI_Foot1.end());
	}
	else
	{
		pfoot = pModel->m_pFoot2;
		ROI.insert(ROI.begin(), pModel->m_ROI_Foot2.begin(), pModel->m_ROI_Foot2.end());
	}

	for (vector<RECT>::iterator iter = ROI.begin(); iter != ROI.end(); iter++) {		
		FindFootROI(pModel, pfoot, *iter, rst, nSearchType, FootIndex);
	}

	if (pfoot->m_FootCenterPoint.x != 0 && pfoot->m_FootCenterPoint.y != 0)
	{
		//모델 foot과 겹치는 영역 제거	
		POINT ptFootC;
		ptFootC.x = pfoot->m_FootCenterPoint.x + pfoot->mWindowImageRect.left;
		ptFootC.y = pfoot->m_FootCenterPoint.y + pfoot->mWindowImageRect.top;

		for (vector<FootRect>::iterator it = rst.begin(); it != rst.end(); ) {

			CRect rect =(*it).m_FootRect;
			if (rect.PtInRect(ptFootC)) {
				it = rst.erase(it);
			}
			else {it++;}
		}
	}


	//X축 정렬
	if (rst.size() > 0)
		std::sort(rst.begin(), rst.end());
	
	if (rst.size() > 0) return true;
	else return false;
}

bool CPInsp_WireBonding::FindFootROI(CSearchWire_Model* pModel, CFoot_Model* pfoot, RECT ROI, std::vector<FootRect>& rst, int nSearchType, int FootIndex)
{
	bool ret = false;

	if (nSearchType == 0) // Foot B/W
	{
		CFoot_Model* searchModel = MakeSearchFoot(pModel, pfoot, ROI);
		ret = m_vPInspFoot[pfoot->m_nFoottype]->FindFootROI_BW(searchModel, rst);
		//delete searchModel;
		g_pMManager->pem_delete(searchModel, false);
	}
	else if(nSearchType == 1)	// 2D Pattern Matching
	{
		Img_Proc = std::make_shared<ImgProcessing>();
		int retPat = m_vPInspFoot[pfoot->m_nFoottype]->SearchPattern2D(pfoot, pModel, ROI, rst, FootIndex);
		Img_Proc->Free();

		ret = true;
	}
	else if (nSearchType == 2)	// 3D Pattern Matching
	{
		Img_Proc = std::make_shared<ImgProcessing>();
		int retPat = m_vPInspFoot[pfoot->m_nFoottype]->SearchPattern3D(pfoot, pModel, ROI, rst, FootIndex);
		Img_Proc->Free();

		ret = true;
	}
	else if (nSearchType == 3)	// 2D,3D Pattern Matching
	{
		Img_Proc = std::make_shared<ImgProcessing>();
		int retPat = m_vPInspFoot[pfoot->m_nFoottype]->SearchPattern2D3D(pfoot, pModel, ROI, rst, FootIndex);
		Img_Proc->Free();

		ret = true;
	}

	//


	return ret;
}



CFoot_Model* CPInsp_WireBonding::MakeSearchFoot(CSearchWire_Model* pModel, CFoot_Model* baseFoot, RECT ROI)
{
	//CFoot_Model* pSearchModel = new CFoot_Model;
	CFoot_Model* pSearchModel = g_pMManager->pem_new<CFoot_Model>(false, 0, (PCHAR)__FUNCTION__, __LINE__);


	//model->m_nWindowStartX = nStartX;
	//model->m_nWindowStartY = nStartY;

	pSearchModel->m_FootCenterPoint.x = 0;
	pSearchModel->m_FootCenterPoint.y = 0;

	pSearchModel->_WidthOrg = pModel->m_nWidth;
	pSearchModel->_HeightOrg = pModel->m_nHeight;

	////검사 Option
	pSearchModel->m_nUseOption = baseFoot->m_nUseOption;
	pSearchModel->m_nUseOption2 = baseFoot->m_nUseOption2;
	pSearchModel->m_nFindOption = baseFoot->m_nFindOption;
	pSearchModel->m_nFindOption2 = baseFoot->m_nFindOption2;

	pSearchModel->m_nUseOption2 = 0;

	////Clip
	pSearchModel->mWindowImageRect = ROI;

	////Ref Bottom Rect
	pSearchModel->m_RefRect = baseFoot->m_RefRect;

	////Search Rect
	pSearchModel->mImageRect = ROI;

	//Clip 영역
	cv::Rect ImageRect(pSearchModel->mImageRect.left, pSearchModel->mImageRect.top,
		pSearchModel->mImageRect.right - pSearchModel->mImageRect.left, pSearchModel->mImageRect.bottom - pSearchModel->mImageRect.top);

	int nWidth = ImageRect.width;;
	int nHeight = ImageRect.height;

	pSearchModel->m_ImageBuffer->nImageSizeX = nWidth;
	pSearchModel->m_ImageBuffer->nImageSizeY = nHeight;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->pf3D, sizeof(float) * nWidth * nHeight);

	int _3DCX = ImageRect.x + (nWidth / 2.);// -pModel->m_nWidth;
	int _3DCY = ImageRect.y + (nHeight / 2.);// -pModel->m_nHeight;


	if (0 != m_pProcMilAlgo->GetCropZmap(pModel->m_pPart3D, pSearchModel->pf3D, pModel->m_nWidth, pModel->m_nHeight, _3DCX, _3DCY, nWidth, nHeight))
	{
		//delete pSearchModel;
		g_pMManager->pem_delete(pSearchModel, false);
		return nullptr;
	}

	pSearchModel->pf3DOrg = pSearchModel->pf3D;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgTop_R, sizeof(float) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgTop_B, sizeof(float) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgTop_W, sizeof(float) * nWidth * nHeight);

	//Get Matrix에서 사용
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgTop_G, sizeof(float) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgMiddle_R, sizeof(float) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgMiddle_B, sizeof(float) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgBottom_R, sizeof(float) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pSearchModel->m_ImageBuffer->imgBottom_B, sizeof(float) * nWidth * nHeight);

	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgTop_R, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgTop_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgTop_B, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgTop_B, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgTop_W, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgTop_W, ImageRect.x, ImageRect.y, nWidth, nHeight);

	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgTop_G, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgTop_G, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgMiddle_R, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgMiddle_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgMiddle_B, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgMiddle_B, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgBottom_R, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgBottom_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(pModel->m_pPartImageBuffer->imgBottom_B, pModel->m_pPartImageBuffer->nImageSizeX, pModel->m_pPartImageBuffer->nImageSizeY, pSearchModel->m_ImageBuffer->imgBottom_B, ImageRect.x, ImageRect.y, nWidth, nHeight);


	memcpy(&pSearchModel->m_sArrBin, &baseFoot->m_sArrBin, sizeof(AlgoBin) * m_eFootBin_Total); //Binary
	memcpy(&pSearchModel->m_fArrOptionValue, &baseFoot->m_fArrOptionValue, sizeof(float) *m_eFoot_Total* eMMD_Total); //°Ë»ç ¿É¼Ç


	cv::Mat PadMask(nHeight, nWidth, CV_8UC1);

	//Pad 이진화
	pSearchModel->GetImageMatrix(&PadMask, (int)m_eFootBin_Pad);

	//이진화 Blob
	int nPadBlobCnt = m_vPInspFoot[m_nFootType]->GetBinImage(pSearchModel, &PadMask, (int)m_eFootBin::m_eFootBin_Pad, NULL, false);

	//pSearchModel->m_pPad = new UCHAR[nWidth* nHeight];
	pSearchModel->m_pPad = g_pMManager->pem_new<UCHAR>(true, nWidth* nHeight, (PCHAR)__FUNCTION__, __LINE__);

	memset(pSearchModel->m_pPad, 0, nWidth* nHeight);

	if (nPadBlobCnt > 0)
	{
		float fPad3DAvgHeigt = 0.0f;

		//Pad 이진화로 보정하자.
		m_vPInspFoot[m_nFootType]->Correct3d_Binary(PadMask, (baseFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), pSearchModel->pf3D, nWidth, nHeight, fPad3DAvgHeigt);
		
		pSearchModel->m_fPadArea3DAvgHeight = fPad3DAvgHeigt;

		//검출된 Pad 정보를 저장..
		for (size_t y = 0; y < nHeight; y++)
		{
			UCHAR* pRaw = PadMask.ptr<UCHAR>(y);
			for (size_t x = 0; x < nWidth; x++)
				pSearchModel->m_pPad[y*nWidth + x] = pRaw[x];
		}
	}


	//Correct3d((baseFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), pSearchModel->pf3D, nWidth, nHeight, pSearchModel->m_ImageBuffer->imgTop_R, pSearchModel->m_ImageBuffer->imgTop_B, pSearchModel->m_ImageBuffer->imgTop_W, true);


	return pSearchModel;
}




bool CPInsp_WireBonding::InterpolateWire3D(AlgoWire algoWire, WndAlgoImg& sWndAlgoImg, UCHAR* ucImage, bool bHorizon, POINT* poArrFoot, CWire_Model* pModel)
{
	UCHAR* pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float* pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || (ucImage == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return false;

	UCHAR* pucBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucBlob, nImgWidth * nImgHeight);
	memset(pucBlob, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);

	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucImage, pucBlob, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix);
	Delete_1DArray(&pucBlob);
	if (nCntBlob == 0) return false;

	// 끊어짐이 발생한 경우(nCntBlob이 2개 이상) 2D 이미지로 Wire 재 탐색
	// Wire 양 끝 단 확인 후, 진행방향에 따라 이어질 수 있는지 확인
	double dArrArea[3], dArrCX[3], dArrCY[3];
	CRect rcArrBlob[3];
	memset(dArrArea, 0, sizeof(double) * 3);
	memset(dArrCX, 0, sizeof(double) * 3);
	memset(dArrCY, 0, sizeof(double) * 3);
	memset(rcArrBlob, 0, sizeof(CRect) * 3);
	m_pProcMilAlgo->GetBlobResult(dArrArea, dArrCX, dArrCY, rcArrBlob, true);

	//if (nImgWidth * nImgHeight < dArrArea[0] * 4)  //화면 영역에 비례하여 클 경우 (잘못된 검출)
	//	return true;

	float fX_D = (rcArrBlob[0].left - rcArrBlob[1].right);
	float fY_D = (rcArrBlob[0].top - rcArrBlob[1].top);
	if (bHorizon)
	{
		if (rcArrBlob[0].left > rcArrBlob[1].left)
			fX_D = (rcArrBlob[0].right - rcArrBlob[1].left);
	}
	else
	{
		fX_D = (rcArrBlob[0].left - rcArrBlob[1].left);
		fY_D = (rcArrBlob[0].top - rcArrBlob[1].bottom);
		if (rcArrBlob[0].top > rcArrBlob[1].top)
			fY_D = (rcArrBlob[0].bottom - rcArrBlob[1].top);
	}

	double dDist = (double)sqrt(pow(fX_D, 2) + pow(fY_D, 2));

	fX_D = (poArrFoot[1].x - poArrFoot[0].x);
	fY_D = (poArrFoot[1].y - poArrFoot[0].y);

	//double dTDist = (double)sqrt(pow(fX_D, 2) + pow(fY_D, 2));
	//if (dTDist * 0.5 > dDist) return false;

	int nTh = (int)(algoWire.m_fArrOptionValue[eWIRE_Thickness][eMMD_Default] / m_resolY);

	if (nTh == 0) return false;

	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;
	int nThMin = (int)((algoWire.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] / m_resolY) * 0.5);
	if (bHorizon == false)
		nThMin = (int)((algoWire.m_fArrOptionValue[eWIRE_Thickness][eMMD_Min] / m_resolX) * 0.5);
	int nStartX = (bHorizon) ? poArrFoot[0].x : poArrFoot[0].y;
	int nEndX = (bHorizon) ? poArrFoot[1].x : poArrFoot[1].y;
	int nStartY = (bHorizon) ? poArrFoot[0].y : poArrFoot[0].x;
	int nEndY = (bHorizon) ? poArrFoot[1].y : poArrFoot[1].x;
	if (nStartX > nEndX)
	{
		nEndX = (bHorizon) ? poArrFoot[0].x : poArrFoot[0].y;
		nStartX = (bHorizon) ? poArrFoot[1].x : poArrFoot[1].y;
	}
	if (nStartY > nEndY)
	{
		nEndY = (bHorizon) ? poArrFoot[0].y : poArrFoot[0].x;
		nStartY = (bHorizon) ? poArrFoot[1].y : poArrFoot[1].x;
	}
	if (nStartX < 0) nStartX = 0;
	if (nStartY < 0) nStartY = 0;
	if (nEndX < 0) nEndX = 0;
	if (nEndY < 0) nEndY = 0;

	if (bHorizon)
	{
		if (nStartX > nImgWidth) nStartX = nImgWidth;
		if (nStartY > nImgHeight) nStartY = nImgHeight;
		if (nEndX > nImgWidth) nEndX = nImgWidth;
		if (nEndY > nImgHeight) nEndY = nImgHeight;
	}
	else
	{
		if (nStartX > nImgHeight) nStartX = nImgHeight;
		if (nStartY > nImgWidth) nStartY = nImgWidth;
		if (nEndX > nImgHeight) nEndX = nImgHeight;
		if (nEndY > nImgWidth) nEndY = nImgWidth;
	}

	//2dImage Warp
	int nWarpMargin = 20;// 5;
	int nWarp2DWidth = (nTh + nWarpMargin * 2);
	int nWarpCenterX = nTh / 2. + nWarpMargin;
	int nWarp2Dheigth = nW;


	//UCHAR* uc2DWarpSearchedImage = new UCHAR[nWarp2Dheigth * nWarp2DWidth];
	UCHAR* uc2DWarpSearchedImage = g_pMManager->pem_new<UCHAR>(true, nWarp2Dheigth * nWarp2DWidth, (PCHAR)__FUNCTION__, __LINE__);

	memset(uc2DWarpSearchedImage, 0, nWarp2Dheigth * nWarp2DWidth);

/*#if _DEBUG*/ // 복원시 필요
	//float* f2DWarp3D = new float[nWarp2Dheigth * nWarp2DWidth * 4];
	float* f2DWarp3D = g_pMManager->pem_new<float>(true, nWarp2Dheigth * nWarp2DWidth * 4, (PCHAR)__FUNCTION__, __LINE__);

	memset(f2DWarp3D, 0, nWarp2Dheigth * nWarp2DWidth * 4);
/*#endif*/

	// X에 대한 7차 방장식 계수
	std::vector<double> coeff; // 추세선

	bool b2DInterpol = false;

	b2DInterpol = CurveFitting2(ucImage, nImgWidth, nImgHeight, bHorizon, poArrFoot, nThMin == 0 ? nTh / 2. : nThMin, nTh, nTh*1.5f, coeff, pfImgSrc);
	// b2DInterpol = CurveFitting(ucImage, nImgWidth, nImgHeight, bHorizon, nW, nH, nStartX, nEndX,  nStartY,  nEndY, nThMin == 0 ? nTh / 2. : nThMin, coeff);


	if (b2DInterpol == false)
	{
		//delete[] uc2DWarpSearchedImage;
		g_pMManager->pem_delete(uc2DWarpSearchedImage, true);

/*#if _DEBUG*/
		//delete[] f2DWarp3D;
		g_pMManager->pem_delete(f2DWarp3D, true);
/*#endif*/

		return false;
	}

	double dDistance = nTh / 2. + nWarpMargin;;

	//Warp이미지에 대한 이미지 Map
	//int* pWarpMap = new int[nWarp2DWidth*nWarp2Dheigth * sizeof(int)];
	int* pWarpMap = g_pMManager->pem_new<int>(true, nWarp2DWidth*nWarp2Dheigth * sizeof(int), (PCHAR)__FUNCTION__, __LINE__);


	memset(pWarpMap, 0, nWarp2DWidth*nWarp2Dheigth * sizeof(int));

	WireFitting(pWarpMap, nWarp2DWidth, nImgWidth, nImgHeight, bHorizon, nStartX, nEndX, nW, nH, dDistance, coeff);

	//이미지에 대한 Warp이미지 Map
	//int* pWarpReverseMap = new  int[nImgWidth*nImgHeight * sizeof(int)];
	int* pWarpReverseMap = g_pMManager->pem_new<int>(true, nImgWidth*nImgHeight * sizeof(int), (PCHAR)__FUNCTION__, __LINE__);


	memset(pWarpReverseMap, 0, nImgWidth*nImgHeight * sizeof(int));
	for (size_t i = 0; i < nWarp2DWidth*nWarp2Dheigth; i++)
	{
		if (pWarpMap[i])
			pWarpReverseMap[pWarpMap[i]] = i;
	}


	WireWarpImage(ucImage, uc2DWarpSearchedImage, pWarpMap, nWarp2DWidth, nWarp2Dheigth); //Flood Fill로 찾은 Wire

	//UCHAR* puEdges = new UCHAR[nWarp2DWidth*nWarp2Dheigth];
	UCHAR* puEdges = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nWarp2Dheigth, (PCHAR)__FUNCTION__, __LINE__);

	memset(puEdges, 0, nWarp2DWidth*nWarp2Dheigth);
	FindWireEdgeV2(puEdges, pfImgSrc, pucImgSrc, uc2DWarpSearchedImage, pModel, pWarpMap, nWarp2DWidth, nWarp2Dheigth, nWarpCenterX, nTh);


	WireWarp3D(pfImgSrc, f2DWarp3D, pWarpMap, nWarp2DWidth, nWarp2Dheigth);
#if _DEBUG
	cv::Mat matWa_T(nWarp2Dheigth, nWarp2DWidth, CV_32FC1, f2DWarp3D);
	vector<float> vTestPoint; 
	vector<float> vTestPoint2;
	double dErrorSum = 0;
	double dErrorSum2 = 0;

	for (int i = 0; i < nWarp2Dheigth; i++)
	{
		if (f2DWarp3D[i*nWarp2DWidth + nWarpCenterX] != 0)
		{
			dErrorSum += f2DWarp3D[i*nWarp2DWidth + nWarpCenterX];
			vTestPoint.push_back((f2DWarp3D[i*nWarp2DWidth + nWarpCenterX]));
		}
	}
	
#endif
	

	//UCHAR* ucOrgImage = new UCHAR[nImgHeight* nImgWidth];
	UCHAR* ucOrgImage = g_pMManager->pem_new<UCHAR>(true, nImgHeight* nImgWidth, (PCHAR)__FUNCTION__, __LINE__);

	memcpy(ucOrgImage, ucImage, nImgHeight* nImgWidth);

	memset(ucImage, 0, nImgHeight* nImgWidth);

	//3D Fiiting
	std::vector< jsl::Point3d<double, double, double> > dataListH;
	for (int y = 0; y < nWarp2Dheigth; y++)
	{
		UCHAR* pEdgesY = &puEdges[y*nWarp2DWidth];
		int* pMapy = &pWarpMap[y*nWarp2DWidth];

		for (int x = 0; x < nWarp2DWidth; x++)
		{
			if (pEdgesY[x] == 255)
			{
				int nindex = pMapy[x];

				if (nindex)
					dataListH.emplace_back(x, y, pfImgSrc[nindex] * 0.001);
			}
		}
	}

	//2D 복원
	for (int y = 0; y < nWarp2Dheigth; y++)
	{
		UCHAR* pEdgesY = &puEdges[y*nWarp2DWidth];
		int* pMapy = &pWarpMap[y*nWarp2DWidth];

		for (int x = 0; x < nWarp2DWidth; x++)
		{
			if (pEdgesY[x] == 150)
			{
				int nindex = pMapy[x];
				ucImage[nindex] = 255;
			}
			else if (pEdgesY[x] == 255)
			{
				int nindex = pMapy[x];
				ucImage[nindex] = 255;
			}
		}
	}
	//3D 복원
	bool b3DInterpol = false;
	jsl::LeastSquare lsCalc;
	std::vector<double> coeffH; //높이
	
	if (dataListH.size() > 30)
		lsCalc.Run2d(3, dataListH, coeffH);

	if (coeffH.size() == 10)
		b3DInterpol = true;
// 	vector<float> vTestPoint_s;
// 	vector<float> vTestPoint_a;
	if (b3DInterpol)
	{
		for (int y = 0; y < nWarp2Dheigth; y++)
		{
			UCHAR* pEdgesY = &puEdges[y*nWarp2DWidth];
			int* pMapy = &pWarpMap[y*nWarp2DWidth];
#if _DEBUG
			cv::Mat matWa1(nWarp2Dheigth, nWarp2DWidth, CV_8UC1, pEdgesY);
			cv::Mat matWa2(nWarp2Dheigth, nWarp2DWidth, CV_32FC1, pMapy);
			cv::Mat matWa3(nWarp2Dheigth, nWarp2DWidth, CV_32FC1, f2DWarp3D);
#endif
			for (int x = 0; x < nWarp2DWidth; x++)
			{

				if (x == nWarpCenterX)
				{
					if (f2DWarp3D[y*nWarp2DWidth + nWarpCenterX] != 0)
					{
						if (y > nWarp2Dheigth / 10 && y < nWarp2Dheigth - nWarp2Dheigth / 10)
						{
							// 10 퍼씩 제외.
							if (f2DWarp3D[y*nWarp2DWidth + nWarpCenterX] < -1)
							{
								g_pMManager->pem_delete(uc2DWarpSearchedImage, true);
								/*#if _DEBUG*/
															//delete[] f2DWarp3D;
								g_pMManager->pem_delete(f2DWarp3D, true);
								/*#endif*/
								return false;
							}
						}

// 						vTestPoint_s.push_back(lsCalc.Apply2d(x, y, coeffH)*1000.0);
// 						vTestPoint_a.push_back(f2DWarp3D[y*nWarp2DWidth + nWarpCenterX]);

						if (f2DWarp3D[y*nWarp2DWidth + nWarpCenterX] < -1)
						{
							g_pMManager->pem_delete(uc2DWarpSearchedImage, true);
/*#if _DEBUG*/
							//delete[] f2DWarp3D;
							g_pMManager->pem_delete(f2DWarp3D, true);
/*#endif*/
							return false;
						}
// 						dErrorSum2 += lsCalc.Apply2d(x, y, coeffH)*1000.0;
// 						vTestPoint2.push_back((lsCalc.Apply2d(x, y, coeffH)*1000.0));
					}
				}
		

				if (pEdgesY[x] == 150)
				{
					int nindex = pMapy[x];

					if (nindex == 0) continue;
					if (ucOrgImage[nindex] > 0)
						continue;


#if _DEBUG
					f2DWarp3D[y*nWarp2DWidth + x] = lsCalc.Apply2d(x, y, coeffH)*1000.0;
#endif
					pfImgSrc[nindex] = lsCalc.Apply2d(x, y, coeffH)*1000.0;
				}

				/*	else
						f2DWarp3D[y*nWarp2DWidth + x] = 0;*/
				if (pEdgesY[x] == 255) // Tracking 된 와이어가 깨진와이어일때 
				{
					int nindex = pMapy[x];
					double temp = lsCalc.Apply2d(x, y, coeffH)*1000.0;
					double dFootHeight = pModel->m_dFootHeight[0] < pModel->m_dFootHeight[1] ? pModel->m_dFootHeight[0] : pModel->m_dFootHeight[1];
					if (pfImgSrc[nindex] + dFootHeight < temp) // +300 보다 
					{
#if _DEBUG
						f2DWarp3D[y*nWarp2DWidth + x] = temp ;
#endif
						pfImgSrc[nindex] = temp;
					}
					
				}

			}
#if _DEBUG
			double dErrorSum3 = 0;
			if (vTestPoint2.size() == vTestPoint.size())
			{
				for (int i = 0; i < vTestPoint.size(); i++)
				{
					dErrorSum3 += abs(pow(vTestPoint2[i], 2) - pow(vTestPoint[i], 2));
				}
				//					double rst  = dErrorSum - dErrorSum2;
			}
#endif
		}

		//빈 공간 채우기
		for (int x = nStartX; x < nEndX; x++)
		{
			int nSY = 0, nEY = 0;
			for (int y = 0; y < nH; y++)
			{
				int nIndex = y * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nIndex] == 0) continue;
				if (nSY == 0 && nEY == 0)
					nSY = y;
				nEY = y;
			}
			if (nSY == 0 || nEY == 0) continue;

			for (int y = nSY; y <= nEY; y++)
			{
				int nIndex = y * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nIndex] == 0)
				{
					if (ucOrgImage[nIndex])
					{
						ucImage[nIndex] = 255;
					}
					else if (pWarpReverseMap[nIndex + 1])
					{
						int IndexWarp = pWarpReverseMap[nIndex + 1];

						int y2 = IndexWarp / nWarp2DWidth;
						int x2 = IndexWarp % nWarp2DWidth;

						if (ucOrgImage[nIndex] == 0)
							pfImgSrc[nIndex] = lsCalc.Apply2d(x2, y2, coeffH)*1000.0;

						ucImage[nIndex] = 255;
					}
					else if (pWarpReverseMap[nIndex - 1])
					{
						int IndexWarp = pWarpReverseMap[nIndex - 1];

						int y2 = IndexWarp / nWarp2DWidth;
						int x2 = IndexWarp % nWarp2DWidth;

						if (ucOrgImage[nIndex] == 0)
							pfImgSrc[nIndex] = lsCalc.Apply2d(x2, y2, coeffH)*1000.0;

						ucImage[nIndex] = 255;

					}

				}

			}

			//세로
			//이미 찾은 위치에서만 보정.. 실제 끊어질 수 있음..
			for (int y = 0; y < nH; y++)
			{
				int nIndex = y * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nIndex]) continue;

				int nPreviosIndex = y * nImgWidth + (x - 1);
				if (!bHorizon) nPreviosIndex = (x - 1)* nImgWidth + y;
				if (nPreviosIndex < 0 || nPreviosIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nPreviosIndex] == 0) continue;

				int nAfterIndex = y * nImgWidth + (x + 1);
				if (!bHorizon) nAfterIndex = (x + 1)* nImgWidth + y;
				if (nAfterIndex < 0 || nAfterIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nAfterIndex] == 0) continue;

				if (ucOrgImage[nIndex])
					ucImage[nIndex] = 255;

			}
		}

	}


#if _DEBUG
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("WireFitting_Result.bmp"));
	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = nWarp2DWidth;
		hPtr.uiNumCol = nImgHeight;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\testimage\\workimage\\WireFitting_3DWarp.ptr"), &hPtr, f2DWarp3D);
	}

	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = nImgWidth;
		hPtr.uiNumCol = nImgHeight;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\testimage\\workimage\\WireFitting_3D.ptr"), &hPtr, pfImgSrc);
	}
	
	

#endif
	//delete[] f2DWarp3D;
	g_pMManager->pem_delete(f2DWarp3D, true);

	//delete[] pWarpMap;
	g_pMManager->pem_delete(pWarpMap, true);

	//delete[] pWarpReverseMap;
	g_pMManager->pem_delete(pWarpReverseMap, true);

	//delete[] uc2DWarpSearchedImage;
	g_pMManager->pem_delete(uc2DWarpSearchedImage, true);

	//delete[] puEdges;
	g_pMManager->pem_delete(puEdges, true);

	//delete[] ucOrgImage;
	g_pMManager->pem_delete(ucOrgImage, true);

	return false;
}

bool CPInsp_WireBonding::CurveFitting(UCHAR* ucImage, int nImgWidth, int nImgHeight, bool bHorizon, int nW, int nH, int nStartX, int nEndX, int nStartY, int nEndY, int nThMin, std::vector<double>& coeff)
{

#if _DEBUG
	//UCHAR* ucImageCenterLine = new UCHAR[nImgHeight * nImgWidth];
	UCHAR* ucImageCenterLine = g_pMManager->pem_new<UCHAR>(true, nImgHeight * nImgWidth, (PCHAR)__FUNCTION__, __LINE__);


	memset(ucImageCenterLine, 0, nImgHeight * nImgWidth);
#endif

	jsl::LeastSquare lsCalc;
	std::vector< jsl::Point2d<double> > dataList;


	bool bStartCheck = false;

	//시작점 체크
	int nPreviousCenterY = nEndY;

	//end Y 체크
	{
		int nIndex = nPreviousCenterY * nImgWidth + nStartX;
		if (!bHorizon) nIndex = nStartX * nImgWidth + nPreviousCenterY;
		if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
			bStartCheck = false;
		else if (ucImage[nIndex])
			bStartCheck = true;
	}



	//start Y 체크

	if (!bStartCheck)
	{
		nPreviousCenterY = nStartY;

		int nIndex = nPreviousCenterY * nImgWidth + nStartX;
		if (!bHorizon) nIndex = nStartX * nImgWidth + nPreviousCenterY;
		if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight)
			bStartCheck = false;
		else if (ucImage[nIndex])
			bStartCheck = true;
	}


	if (!bStartCheck)
	{

#if _DEBUG
		//delete ucImageCenterLine;
		g_pMManager->pem_delete(ucImageCenterLine, false);
#endif

		return false;
	}


	std::vector<int> vecCenters;
	std::vector<int> vecWidths;

	int nPreviousWidth = 0;
	int nSeletedBlob = -1;
	for (int x = nStartX; x < nEndX; x++)
		//for (int x = 0; x < nW; x++)
	{
		vecCenters.clear();
		vecWidths.clear();
		{
			int nSY = -1, nEY = -1;
			//몇개의 Blob 인지 체크
			for (int y = 0; y < nH; y++)
			{
				int nIndex = y * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;

				if (ucImage[nIndex] && nSY == -1)
					nSY = y;
				else if (ucImage[nIndex] == 0 && nSY != -1)
					nEY = y;
				if (nSY != -1 && nEY != -1)
				{
					int gap = nEY - nSY;
					if (gap > 3)
					{
						vecCenters.push_back((nEY + nSY) / 2.);
						vecWidths.push_back(gap);
					}
					nSY = -1;
					nEY = -1;
				}
			}

		}



		int nSY = -1, nEY = -1;

		int nIndex = nPreviousCenterY * nImgWidth + x;
		if (!bHorizon) nIndex = x * nImgWidth + nPreviousCenterY;
		if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;

		if (vecCenters.size() == 2 && nSeletedBlob != -1)
		{
			nSY = vecCenters[nSeletedBlob] - vecWidths[nSeletedBlob] / 2.;
			nEY = vecCenters[nSeletedBlob] + vecWidths[nSeletedBlob] / 2.;
		}
		else if (ucImage[nIndex]) //이전 라인으로 중심점이 있을 경우
		{
			for (int y = nPreviousCenterY; y > 0; y--)
			{
				int nIndex = y * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nIndex] == 0)
				{
					nSY = y;
					break;
				}
			}

			for (int y = nPreviousCenterY; y < nH; y++)
			{
				int nIndex = y * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nIndex] == 0)
				{
					nEY = y;
					break;
				}

			}

			if (nSeletedBlob == -1 && vecCenters.size() == 2)
			{
				int y = (nEY + nSY) / 2.;

				if (abs(y - vecCenters[0]) < abs(y - vecCenters[1]))
					nSeletedBlob = 0;
				else
					nSeletedBlob = 1;
			}
		}

		//이전 중심점에서 못찾으면
		if (nSY == -1 || nEY == -1)
		{
			for (int y = 0; y < nH; y++)
			{
				int nIndex = y * nImgWidth + x;
				if (!bHorizon) nIndex = x * nImgWidth + y;
				if (nIndex < 0 || nIndex >= nImgWidth * nImgHeight) continue;
				if (ucImage[nIndex] == 0) continue;
				if (nSY == -1 && nEY == -1)
					nSY = y;
				nEY = y;
			}
		}

		if (nSY == -1 || nEY == -1) continue;
		if (nEY - nSY > nThMin)
		{
			/*	if (nEY - nSY > 70)
					continue;*/


			int y = (nSY + nEY) / 2;
			int nIndex = y * nImgWidth + x;
			if (!bHorizon) nIndex = x * nImgWidth + y;

			nPreviousCenterY = y;
			dataList.emplace_back((double)x, (double)y);

#if _DEBUG			
			ucImageCenterLine[nIndex] = 150;
#endif
		}

	}

	bool b2DInterpol = false;
	coeff.clear();

	// X에 대한 7차 방장식 계수
	if (dataList.size() > 30 && dataList.size() > (nEndX - nStartX) / 3.)
		lsCalc.RunSvd(7, dataList, coeff);

	if (coeff.size() == 8)
		b2DInterpol = true;

	if (b2DInterpol)
	{
		//원래 좌표보다 떨어져 있으면 실패
		jsl::Point2d<double> pt = dataList[dataList.size() / 2];

		float fy = lsCalc.ApplySvd(pt.x, coeff);
		float diff = std::abs(pt.y - fy);

		if (diff > nThMin / 2.)
			b2DInterpol = false;

	}

#if _DEBUG
	m_pProcMilAlgo->SaveWorkImg(ucImage, nImgWidth, nImgHeight, _T("WireFitting_FloodFill.bmp"));
	m_pProcMilAlgo->SaveWorkImg(ucImageCenterLine, nImgWidth, nImgHeight, _T("WireFitting_ImageCenterLine.bmp"));
	//delete ucImageCenterLine;
	g_pMManager->pem_delete(ucImageCenterLine, false);
#endif

	return b2DInterpol;
}


bool CPInsp_WireBonding::WireFitting(int* pWarpMap, int nWarp2DWidth, int nImgWidth, int nImgHeight, bool bHorizon, int nStartX, int nEndX, int nW, int nH, int nDistanceHalf, std::vector<double>& coeff)
{

#if _DEBUG
	//UCHAR* ucImageCenterLine = new UCHAR[nImgHeight * nImgWidth];
	UCHAR* ucImageCenterLine = g_pMManager->pem_new<UCHAR>(true, nImgHeight * nImgWidth, (PCHAR)__FUNCTION__, __LINE__);

	memset(ucImageCenterLine, 0, nImgHeight * nImgWidth);
#endif


	//추세선
	jsl::LeastSquare lsCalc;

	//도함수 구함
	// differential coefficient
	std::vector<double> coeff_differential;

	for (size_t i = 0; i < coeff.size() - 1; i++)
	{
		int nOrder = coeff.size() - 1 - i;
		double Coeff_diff = coeff[i] * nOrder;
		coeff_differential.push_back(Coeff_diff);
	}

	float pfy = 0;
	for (int x = nStartX; x < nEndX; x++)
	{
		float fy = lsCalc.ApplySvd(x, coeff);
		if (fy < 0) continue;

		pfy = fy;

		int y = (int)fy;
		int nIndex = y * nImgWidth + x;
		if (!bHorizon) nIndex = x * nImgWidth + y;
		if (nIndex >= nImgHeight * nImgWidth) continue;

		//기울기
		float fslope = lsCalc.ApplySvd(x, coeff_differential);

		//수직인 기울기
		float fslope_perpendicular = -1 / fslope;
		float yss = y - (fslope_perpendicular * x);

		{
			double dcos = sqrt(1 / (fslope_perpendicular * fslope_perpendicular + 1));
			double dsin = fslope_perpendicular * dcos;

			double dDistance = nDistanceHalf;   //10.;

			for (double d = -dDistance; d <= dDistance; d++)
			{
				int dx = (int)(x + (d * dcos));
				int dy = (int)(y + (d * dsin));

				int nIndex = dy * nImgWidth + dx;

				if (!bHorizon)
					nIndex = dx * nImgWidth + dy;

				if (dy >= nH) continue;
				if (dy < 0) continue;
				if (dx < 0) continue;
				//if (dx >= nW) continue;

				if (nIndex >= nImgHeight * nImgWidth) continue;
				if (nIndex < 0) continue;

				if ((int)(d + dDistance) > nWarp2DWidth - 1) continue;

				int nIndex2 = x * nWarp2DWidth + (int)(d + dDistance);

				//if ((int)(d + dDistance) >= nWarp2DWidth) continue;

				pWarpMap[nIndex2] = nIndex;

#if _DEBUG
				ucImageCenterLine[nIndex] = 255;
#endif
			}
		}
	}

#if _DEBUG
	m_pProcMilAlgo->SaveWorkImg(ucImageCenterLine, nImgWidth, nImgHeight, _T("WireFitting_Range.bmp"));
	//delete ucImageCenterLine;
	g_pMManager->pem_delete(ucImageCenterLine, false);
#endif

	return true;
}

bool CPInsp_WireBonding::WireWarpImage(UCHAR* ucImage, UCHAR* ucWarp, int* pWarpMap, int nWarp2DWidth, int nHeight)
{
	for (size_t y = 0; y < nHeight; y++)
	{
		UCHAR* pWarRow = &ucWarp[y *nWarp2DWidth];
		int* pWarpMapRow = &pWarpMap[y *nWarp2DWidth];

		for (size_t x = 0; x < nWarp2DWidth; x++)
		{
			int nIndexImage = pWarpMapRow[x];
			if (nIndexImage <= 0) continue;

			pWarRow[x] = ucImage[nIndexImage];
		}

	}

	return true;
}

bool CPInsp_WireBonding::WireWarp3D(float* ucImage, float* ucWarp, int* pWarpMap, int nWarp2DWidth, int nHeight)
{
	for (size_t y = 0; y < nHeight; y++)
	{
		float* pWarRow = &ucWarp[y *nWarp2DWidth];
		int* pWarpMapRow = &pWarpMap[y *nWarp2DWidth];

		for (size_t x = 0; x < nWarp2DWidth; x++)
		{
			int nIndexImage = pWarpMapRow[x];
			if (nIndexImage <= 0) continue;

			pWarRow[x] = ucImage[nIndexImage];
		}

	}

	return true;
}

void CPInsp_WireBonding::FindWireEdge(UCHAR* Edges, float* pFHeight, UCHAR* ucTeached, UCHAR* ucWarpSearched, CWire_Model* pModel, int* pWarpMap, int nWarp2DWidth, int nHeight, int nCenterX, int nth)
{
	//UCHAR* puTeached = new UCHAR[nWarp2DWidth*nHeight];
	//memset(puTeached, 0, nWarp2DWidth*nHeight);

	//float* pf3D = new float[nWarp2DWidth*nHeight * sizeof(float)];
	float* pf3D = g_pMManager->pem_new<float>(true, nWarp2DWidth*nHeight * sizeof(float), (PCHAR)__FUNCTION__, __LINE__);

	memset(pf3D, 0, nWarp2DWidth*nHeight * sizeof(float));


	//UCHAR* puTW = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puTW = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);

	memset(puTW, 0, nWarp2DWidth*nHeight);

	//UCHAR* puTR = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puTR = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);

	memset(puTR, 0, nWarp2DWidth*nHeight);

	//UCHAR* puMR = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puMR = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);

	memset(puMR, 0, nWarp2DWidth*nHeight);

	//UCHAR* puBR = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puBR = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);

	memset(puBR, 0, nWarp2DWidth*nHeight);

	//UCHAR* puCombine = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puCombine = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);

	memset(puCombine, 0, nWarp2DWidth*nHeight);


	WireWarp3D(pFHeight, pf3D, pWarpMap, nWarp2DWidth, nHeight);
	//WireWarpImage(ucTeached, puTeached, pWarpMap, nWarp2DWidth, nHeight);
	WireWarpImage(pModel->m_ImageBuffer->imgTop_W, puTW, pWarpMap, nWarp2DWidth, nHeight);
	WireWarpImage(pModel->m_ImageBuffer->imgTop_R, puTR, pWarpMap, nWarp2DWidth, nHeight);
	WireWarpImage(pModel->m_ImageBuffer->imgMiddle_R, puMR, pWarpMap, nWarp2DWidth, nHeight);
	WireWarpImage(pModel->m_ImageBuffer->imgBottom_R, puBR, pWarpMap, nWarp2DWidth, nHeight);

	//3D Gradient
	cv::Mat Xgrad = cv::Mat::zeros(cv::Size(nWarp2DWidth, nHeight), CV_32FC1);
	cv::Mat Ygrad = cv::Mat::zeros(cv::Size(nWarp2DWidth, nHeight), CV_32FC1);
	for (int y = 1; y < nHeight - 1; y++)
	{
		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);
		for (int x = 1; x < nWarp2DWidth - 1; x++)
		{
			pXgrad[x] = pf3D[y*nWarp2DWidth + x + 1] - pf3D[y*nWarp2DWidth + x - 1];
			pYgrad[x] = pf3D[(y + 1)*nWarp2DWidth + x] - pf3D[(y - 1)*nWarp2DWidth + x];
		}
	}



	int leftX = nCenterX - (int)(nth / 2.);
	int rightX = nCenterX + (int)(nth / 2.);

	//Combine TR, MR, BR
	for (size_t y = 0; y < nHeight; y++)
	{
		UCHAR* pTR = &puTR[y*nWarp2DWidth];
		UCHAR* pMR = &puMR[y*nWarp2DWidth];
		UCHAR* pBR = &puBR[y*nWarp2DWidth];
		UCHAR* pTW = &puTW[y*nWarp2DWidth];

		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);

		for (size_t x = 0; x < nWarp2DWidth; x++)
		{
			int value = pTR[x] + pMR[x] + pBR[x];
			if (value > 255) value = 255;
			if (pTW[x] > 200) value = 0;

			if (ucWarpSearched[y* nWarp2DWidth + x])
				value = 255;

			//if (value > 100)
			//	value = 255;
			//else
			//	value = 0;
			puCombine[y* nWarp2DWidth + x] = value;

			/*	if(x == leftX || x == rightX)
					puCombine[y* nWarp2DWidth + x] = 0;*/

					/*	if(fabs(pYgrad[x]) < 5 && fabs(pXgrad[x]) < 5)
							puCombine[y* nWarp2DWidth + x] = 0;*/

		}
	}

	m_pProcMilAlgo->SaveWorkImg(ucWarpSearched, nWarp2DWidth, nHeight, _T("WireFitting_Warp_Searched.bmp"));
	//m_pProcMilAlgo->SaveWorkImg(puTeached, nWarp2DWidth, nHeight, _T("WireFitting_Warp_Teached.bmp"));
	//m_pProcMilAlgo->SaveWorkImg_float(pf3D, nWarp2DWidth, nHeight, _T("WireFitting_Warp_3D.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puTW, nWarp2DWidth, nHeight, _T("WireFitting_Warp_TW.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puTR, nWarp2DWidth, nHeight, _T("WireFitting_Warp_TR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puMR, nWarp2DWidth, nHeight, _T("WireFitting_Warp_MR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puBR, nWarp2DWidth, nHeight, _T("WireFitting_Warp_BR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puCombine, nWarp2DWidth, nHeight, _T("WireFitting_Warp_comb.bmp"));

	//combine
	cv::Mat matWarp(nHeight, nWarp2DWidth, CV_8UC1, puCombine);
	cv::Mat gradX;
	cv::Mat gradY;
	cv::GaussianBlur(matWarp, matWarp, cv::Size(5, 5), 0);
	cv::Sobel(matWarp, gradX, CV_16S, 1, 0, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	//cv::Sobel(matWarp, gradY, CV_16S, 0, 1, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	//for (int r = 0; r < gradX.rows; r++)
	//{
	//	short* pGX = gradX.ptr<short>(r);
	//	short* pGY = gradY.ptr<short>(r);
	//	for (int c = 0; c < gradX.cols; c++)
	//	{
	//		float angle = cv::fastAtan2(pGX[c], pGY[c]);
	//		if (  std::abs(angle /90.) >1)
	//		{
	//			pGX[c] = 0;
	//		}
	//	/*	if ((angle / 90.) > 10)
	//			pGX[c] = 0;*/

	//	}

	//}



	gradX = cv::abs(gradX);

	cv::Mat sobelImage;
	gradX.convertTo(sobelImage, CV_8UC1); // 8bit 영상으로 변환


	cv::Mat canny;
	cv::Canny(matWarp, canny, 100, 255);

#if _DEBUG

	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_comb_canny.bmp", canny);
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_comb_sobel.bmp", sobelImage);

	cv::Mat magnitue(nHeight, nWarp2DWidth, CV_8UC1, cv::Scalar(0));
	for (int r = 0; r < magnitue.rows; r++)
	{
		UCHAR* pcomp = matWarp.ptr<UCHAR>(r);
		UCHAR* fPtrX = magnitue.ptr<UCHAR>(r);
		for (int c = 1; c < magnitue.cols - 1; c++)
		{
			int magX_P1 = abs(pcomp[c + 1]);
			int magX_M1 = abs(pcomp[c - 1]);

			int Hdiff = std::abs(magX_P1 - magX_M1);

			if (Hdiff > 255)
				Hdiff = 0;


			fPtrX[c] = Hdiff;
		}
	}
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_comb_magnitude.bmp", magnitue);

	//prewitt
	/*cv::Mat prewitt(nHeight, nWarp2DWidth, CV_8UC1, cv::Scalar(0));
	for (int r = 1; r < magnitue.rows-1; r++)
	{
		UCHAR* pcomp = matWarp.ptr<UCHAR>(r);
		UCHAR* fPtrX = prewitt.ptr<UCHAR>(r);
		for (int c = 1; c < magnitue.cols - 1; c++)
		{
			int magX_P1 = abs(pcomp[c + 1]);
			int magX_M1 = abs(pcomp[c - 1]);

			int Hdiff = std::abs(magX_P1 - magX_M1);

			if (Hdiff > 255)
				Hdiff = 0;


			fPtrX[c] = Hdiff;
		}
	}*/


	vector<cv::Vec4i> lines;
	cv::HoughLinesP(canny, lines, 1, PI / 180, 10, 0, 0);

	cv::Mat hough(nHeight, nWarp2DWidth, CV_8UC1, cv::Scalar(0));
	for (size_t i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		cv::Point p1(l[0], l[1]);
		cv::Point p2(l[2], l[3]);
		line(hough, p1, p2, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
	}

	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_comb_hough.bmp", hough);

#endif
	//TR
	cv::Mat matWarp_TR(nHeight, nWarp2DWidth, CV_8UC1, puTR);
	cv::Mat gradX_TR;
	cv::GaussianBlur(matWarp_TR, matWarp_TR, cv::Size(5, 5), 0);
	cv::Sobel(matWarp_TR, gradX_TR, CV_16S, 1, 0, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	gradX_TR = cv::abs(gradX_TR);

	cv::Mat sobelImage_TR;
	gradX_TR.convertTo(sobelImage_TR, CV_8UC1); // 8bit 영상으로 변환

#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_TR_sobel.bmp", sobelImage_TR);

	cv::Mat canny_TR;
	cv::Canny(matWarp_TR, canny_TR, 100, 255);
	canny_TR = canny_TR & matWarp;
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_TR_canny.bmp", canny_TR);

	/*{
		/// Generate grad_x and grad_y
		cv::Mat grad_x, grad_y;
		cv::Mat abs_grad_x, abs_grad_y;

		/// Gradient X
		cv::Scharr(matWarp_TR, grad_x, CV_16S, 1, 0, 1, 0, BORDER_DEFAULT);

		cv::convertScaleAbs(grad_x, abs_grad_x);
		cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_comb_TR_Scharr.bmp", abs_grad_x);

	}*/
#endif

	//MR
	cv::Mat matWarp_MR(nHeight, nWarp2DWidth, CV_8UC1, puMR);
	cv::Mat gradX_MR;
	cv::GaussianBlur(matWarp_MR, matWarp_MR, cv::Size(5, 5), 0);
	cv::Sobel(matWarp_MR, gradX_MR, CV_16S, 1, 0, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	gradX_MR = cv::abs(gradX_MR);

	cv::Mat sobelImage_MR;
	gradX_MR.convertTo(sobelImage_MR, CV_8UC1); // 8bit 영상으로 변환

#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_MR_sobel.bmp", sobelImage_MR);

	cv::Mat canny_MR;
	cv::Canny(matWarp_MR, canny_MR, 100, 255);
	canny_MR = canny_MR & matWarp;
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_MR_canny.bmp", canny_MR);
#endif
	//BR
	cv::Mat matWarp_BR(nHeight, nWarp2DWidth, CV_8UC1, puBR);
	cv::Mat gradX_BR;
	cv::GaussianBlur(matWarp_BR, matWarp_BR, cv::Size(5, 5), 0);
	cv::Sobel(matWarp_BR, gradX_BR, CV_16S, 1, 0, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	gradX_BR = cv::abs(gradX_BR);

	cv::Mat sobelImage_BR;
	gradX_BR.convertTo(sobelImage_BR, CV_8UC1); // 8bit 영상으로 변환

#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_BR_sobel.bmp", sobelImage_BR);

	cv::Mat canny_BR;
	cv::Canny(matWarp_BR, canny_BR, 100, 255);

	canny_BR = canny_BR & matWarp;
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_BR_canny.bmp", canny_BR);
	cv::Mat canny_sum = canny_TR + canny_MR + canny_BR;
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_sum_canny.bmp", canny_sum);
#endif

	cv::Mat sobel_sum = sobelImage_TR + sobelImage_MR + sobelImage_BR;
	sobel_sum = sobel_sum & matWarp;

#if _DEBUG



	//cv::Mat canny_sobel_sum;
	//cv::Canny(sobel_sum, canny_sobel_sum, 100, 255);
	//cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_sum_sobel_canny.bmp", canny_sobel_sum);


	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_sum_sobel.bmp", sobel_sum);

	/*cv::Mat ret;
	cv::threshold(sobel_sum, ret, 200, 255, 0);
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_ret.bmp", ret);*/

	cv::Mat sobel_AND = sobelImage_TR & sobelImage_MR & sobelImage_BR & matWarp;
	cv::threshold(sobel_AND, sobel_AND, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_AND_sobel.bmp", sobel_AND);
#endif
	//Mat sobelImage_Master = sobelImage + sobelImage_MR;
	//cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_Master_sobel.bmp", sobelImage_Master);

	//sobelImage_Master = sobelImage_Master & matWarp;
	//cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_Master2_sobel.bmp", sobelImage_Master);


	for (size_t y = 0; y < nHeight; y++)
	{
		/*UCHAR* pTR = &puTR[y*nWarp2DWidth];
		UCHAR* pMR = &puMR[y*nWarp2DWidth];
		UCHAR* pBR = &puBR[y*nWarp2DWidth];
		UCHAR* pTW = &puTW[y*nWarp2DWidth];*/

		UCHAR* pcombo = matWarp.ptr<UCHAR>(y);
		UCHAR* pCanny = canny.ptr<UCHAR>(y);
		UCHAR* pSobel = sobel_sum.ptr<UCHAR>(y);

		UCHAR* pedge = &Edges[y*nWarp2DWidth];
		UCHAR* pSearched = &ucWarpSearched[y*nWarp2DWidth];

		bool bEdge = false;
		size_t x = 0;
		for (x = nCenterX; x > 0; x--)
		{
			if (pSearched[x])
				pedge[x] = 255;
			else if (pcombo[x] < 200)
				break;
			else if (x > leftX)
			{
				if (pSobel[x] < 30)
					break;

				pedge[x] = 150;

			} //경계선 외부
			else
			{
				if (x < leftX  && pCanny[x])
				{
					bEdge = true;
					break;
				}
			}
		}

		if (bEdge)
		{
			for (int i = leftX; i >= x; i--)
			{
				pedge[i] = 150;
			}
		}

		bEdge = false;
		for (x = nCenterX; x < nWarp2DWidth; x++)
		{
			if (pSearched[x])
				pedge[x] = 255;
			else if (pcombo[x] < 200)
				break;
			else if (x < rightX) //경계선 안쪽
			{
				if (pSobel[x] < 30)
					break;


				pedge[x] = 150;


			}
			else  // 경계선 외부
			{

				//if (pSobel[x] < 30)
				//{
				////	bEdge = true;
				//	break;
				//}

				//if (x == nWarp2DWidth - 1)
				//	bEdge = true;

				if (x >= rightX && pCanny[x])
				{
					bEdge = true;
					break;
				}

			}


		}

		if (bEdge)
		{
			for (int i = rightX; i <= x; i++)
			{
				pedge[i] = 150;
			}
		}

		/*pedge[leftX] = 0;
		pedge[nCenterX] = 0;
		pedge[rightX] = 0;*/

	}

	m_pProcMilAlgo->SaveWorkImg(Edges, nWarp2DWidth, nHeight, _T("WireFitting_Warp_FootArea.bmp"));

	//delete[] puTeached;
	//delete[] pf3D;
	g_pMManager->pem_delete(pf3D, true);

	//delete[] puTW;
	g_pMManager->pem_delete(puTW, true);

	//delete[] puTR;
	g_pMManager->pem_delete(puTR, true);

	//delete[] puMR;
	g_pMManager->pem_delete(puMR, true);

	//delete[] puBR;
	g_pMManager->pem_delete(puBR, true);

	//delete[] puCombine;
	g_pMManager->pem_delete(puCombine, true);


}


void CPInsp_WireBonding::FindWireEdgeV2(UCHAR* Edges, float* pFHeight, UCHAR* ucTeached, UCHAR* ucWarpSearched, CWire_Model* pModel, int* pWarpMap, int nWarp2DWidth, int nHeight, int nCenterX, int nth)
{

	//UCHAR* puTW = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puTW = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(puTW, 0, nWarp2DWidth*nHeight);

	//UCHAR* puTR = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puTR = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(puTR, 0, nWarp2DWidth*nHeight);

	//UCHAR* puMR = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puMR = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(puMR, 0, nWarp2DWidth*nHeight);

	//UCHAR* puBR = new UCHAR[nWarp2DWidth*nHeight];
	UCHAR* puBR = g_pMManager->pem_new<UCHAR>(true, nWarp2DWidth*nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(puBR, 0, nWarp2DWidth*nHeight);


	WireWarpImage(pModel->m_ImageBuffer->imgTop_W, puTW, pWarpMap, nWarp2DWidth, nHeight);
	WireWarpImage(pModel->m_ImageBuffer->imgTop_R, puTR, pWarpMap, nWarp2DWidth, nHeight);
	WireWarpImage(pModel->m_ImageBuffer->imgMiddle_R, puMR, pWarpMap, nWarp2DWidth, nHeight);
	WireWarpImage(pModel->m_ImageBuffer->imgBottom_R, puBR, pWarpMap, nWarp2DWidth, nHeight);

	int leftX = nCenterX - (int)(nth / 2.);
	int rightX = nCenterX + (int)(nth / 2.);


	m_pProcMilAlgo->SaveWorkImg(ucWarpSearched, nWarp2DWidth, nHeight, _T("WireFitting_Warp_Searched.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puTW, nWarp2DWidth, nHeight, _T("WireFitting_Warp_TW.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puTR, nWarp2DWidth, nHeight, _T("WireFitting_Warp_TR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puMR, nWarp2DWidth, nHeight, _T("WireFitting_Warp_MR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(puBR, nWarp2DWidth, nHeight, _T("WireFitting_Warp_BR.bmp"));

	//Shartp filter
	double maskSharp[3][3] = { { -1, -1, -1 }, { -1, 9, -1 },{ -1, -1, -1 } };
	cv::Mat kernelSharp = cv::Mat(3, 3, CV_64FC1, maskSharp);

	//prewitt filter
	double maskprewitt[3][3] = { { -1, 0 ,1 }, { -1, 0, 1 },{ -1, 0, 1 } };
	cv::Mat kernelprewitt = cv::Mat(3, 3, CV_64FC1, maskprewitt);

	//TR
	cv::Mat matWarp_TR(nHeight, nWarp2DWidth, CV_8UC1, puTR);
	cv::Mat gradX_TR;
	cv::GaussianBlur(matWarp_TR, matWarp_TR, cv::Size(5, 5), 0);
	cv::Sobel(matWarp_TR, gradX_TR, CV_16S, 1, 0, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	gradX_TR = cv::abs(gradX_TR);

	cv::Mat sobelImage_TR;
	gradX_TR.convertTo(sobelImage_TR, CV_8UC1); // 8bit 영상으로 변환

#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_TR_sobel.bmp", sobelImage_TR);
#endif

	//MR
	cv::Mat matWarp_MR(nHeight, nWarp2DWidth, CV_8UC1, puMR);
	cv::Mat gradX_MR;
	cv::GaussianBlur(matWarp_MR, matWarp_MR, cv::Size(5, 5), 0);
	cv::Sobel(matWarp_MR, gradX_MR, CV_16S, 1, 0, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	gradX_MR = cv::abs(gradX_MR);

	cv::Mat sobelImage_MR;
	gradX_MR.convertTo(sobelImage_MR, CV_8UC1); // 8bit 영상으로 변환

#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_MR_sobel.bmp", sobelImage_MR);
#endif
	//BR
	cv::Mat matWarp_BR(nHeight, nWarp2DWidth, CV_8UC1, puBR);
	cv::Mat gradX_BR;
	cv::GaussianBlur(matWarp_BR, matWarp_BR, cv::Size(5, 5), 0);
	cv::Sobel(matWarp_BR, gradX_BR, CV_16S, 1, 0, 3, 2.0, 0.0, cv::BORDER_DEFAULT);
	gradX_BR = cv::abs(gradX_BR);

	cv::Mat sobelImage_BR;
	gradX_BR.convertTo(sobelImage_BR, CV_8UC1); // 8bit 영상으로 변환

#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\WireFitting_Warp_BR_sobel.bmp", sobelImage_BR);
#endif
	std::vector<UCHAR> vecCenter;
	vecCenter.resize(nHeight);
	for (size_t y = 0; y < nHeight; y++)
	{
		UCHAR* pSearched = &ucWarpSearched[y*nWarp2DWidth];

		bool bFind = false;

		int nLeft = 0;
		int nRight = 0;

		for (int x = 0; x < nWarp2DWidth; x++)
		{
			if (pSearched[x])
			{
				nLeft = x;
				bFind = true;
				break;
			}
		}

		if (bFind)
			for (int x = nWarp2DWidth; x >= 0; x--)
			{
				if (pSearched[x])
				{
					nRight = x;
					break;
				}
			}

		if (bFind)
			vecCenter[y] = (int)((nLeft + nRight) / 2.);
		else
			vecCenter[y] = 0;
	}

	int nStartY = 0;
	int nEndY = 0;

	for (size_t y = 0; y < nHeight; y++)
	{
		if (vecCenter[y] > 0)
		{
			nStartY = y;
			break;
		}
	}
	for (size_t y = nHeight - 1; y >= 0; y--)
	{
		if (vecCenter[y] > 0)
		{
			nEndY = y;
			break;
		}
	}

	//찾은 영역에서 강도가 낮은 부분 제거
	for (size_t y = 0; y < nHeight; y++)
	{
		UCHAR* pedge = &Edges[y*nWarp2DWidth];
		UCHAR* pSearched = &ucWarpSearched[y*nWarp2DWidth];

		for (int x = 0; x < nCenterX; x++)
		{
			if (pSearched[x])
			{
				int index2 = y * sobelImage_MR.step + x;
				if (sobelImage_TR.data[index2] <= 30
					&& sobelImage_MR.data[index2] <= 30
					&& sobelImage_BR.data[index2] <= 30)
				{
					pSearched[x] = 0;

					if (x + 1 < nCenterX)
						pSearched[x + 1] = 255;
				}
				else
					break;
			}
		}

		for (int x = nWarp2DWidth - 1; x >= nCenterX; x--)
		{
			if (pSearched[x])
			{
				int index2 = y * sobelImage_MR.step + x;
				if (sobelImage_TR.data[index2] <= 30
					&& sobelImage_MR.data[index2] <= 30
					&& sobelImage_BR.data[index2] <= 30)
				{
					pSearched[x] = 0;

					if (x - 1 >= nCenterX)
						pSearched[x - 1] = 255;
				}
				else
					break;
			}
		}
	}


	for (size_t y = 0; y < nHeight; y++)
	{
		UCHAR* pedge = &Edges[y*nWarp2DWidth];
		UCHAR* pSearched = &ucWarpSearched[y*nWarp2DWidth];

		bool bEdge = false;
		size_t x = 0;
		for (x = 0; x < nCenterX; x++)
		{
			if (pSearched[x])
			{
				pedge[x] = 255;
				bEdge = true;
				break;
			}
		}

		//sobelImage_TR
		//sobelImage_BR
		//sobelImage_MR
		if (bEdge)
		{
			for (size_t y2 = y + 1; y2 < nHeight - 1; y2++)
			{
				int index = y2 * nWarp2DWidth + x;
				int index2 = y2 * sobelImage_MR.step + x;

				//if (vecCenter[y2] == 0)break;
				if (y2 > nEndY) break;

				//if (ucWarpSearched[y2 * nWarp2DWidth + nCenterX] == 0) break;
				if (ucWarpSearched[index]) break;
				else if (Edges[index]) break;
				else if (sobelImage_TR.data[index2] > 30)
				{
					Edges[index] = 150;
					continue;
				}
				else if (sobelImage_MR.data[index2] > 30)
				{
					Edges[index] = 150;
					continue;
				}
				else if (sobelImage_BR.data[index2] > 30)
				{
					Edges[index] = 150;
					continue;
				}
				else
					break;
			}
		}


		bEdge = false;
		for (x = nWarp2DWidth - 1; x >= nCenterX; x--)
		{
			if (pSearched[x])
			{
				pedge[x] = 255;
				bEdge = true;
				break;
			}
		}

		if (bEdge)
		{
			for (size_t y2 = y + 1; y2 < nHeight - 1; y2++)
			{
				int index = y2 * nWarp2DWidth + x;
				int index2 = y2 * sobelImage_MR.step + x;

				if (y2 > nEndY) break;
				//if (vecCenter[y2] == 0)break;
				//if (ucWarpSearched[y2 * nWarp2DWidth + nCenterX] == 0) break;
				if (ucWarpSearched[index]) break;
				else if (Edges[index]) break;
				else if (sobelImage_TR.data[index2] > 30)
				{
					Edges[index] = 150;
					continue;
				}
				else if (sobelImage_MR.data[index2] > 30)
				{
					Edges[index] = 150;
					continue;
				}
				else if (sobelImage_BR.data[index2] > 30)
				{
					Edges[index] = 150;
					continue;
				}
				else
					break;
			}


		}

	}

	std::vector<cv::Point> vRightEdgePt;
	std::vector<cv::Point> vLeftEdgePt;
	for (size_t y = 0; y < nHeight; y++)
	{
		UCHAR* pedge = &Edges[y*nWarp2DWidth];

		bool bEdge = false;
		for (size_t x = 0; x < nCenterX; x++)
		{
			if ((pedge[x]))
			{
				vLeftEdgePt.push_back(cv::Point(x, y));
				break;
			}
		}

		bEdge = false;
		for (size_t x = nWarp2DWidth - 1; x >= nCenterX; x--)
		{
			if (pedge[x])
			{
				vRightEdgePt.push_back(cv::Point(x, y));
				break;
			}
		}
	}
	cv::Vec4f line;
	cv::fitLine(vLeftEdgePt, line, cv::DIST_L2, 0, 0.01, 0.01);
	cv::Point ptLpt;
	ptLpt.x = line[2]; // 선에 놓은 한 점
	ptLpt.y = line[3];

	cv::Vec4f line2;
	cv::fitLine(vRightEdgePt, line2, cv::DIST_L2, 0, 0.01, 0.01);
	cv::Point ptRpt;
	ptRpt.x = line2[2]; // 선에 놓은 한 점
	ptRpt.y = line2[3];

	//빈공간 메꾸자
	for (size_t y = 0; y < nHeight; y++)
	{
		UCHAR* pedge = &Edges[y*nWarp2DWidth];
		UCHAR* pSearched = &ucWarpSearched[y*nWarp2DWidth];

		bool bEdge = false;
		for (size_t x = 0; x < nCenterX; x++)
		{
			if (x < ptLpt.x)
			{
				if (pedge[x])
					pedge[ptRpt.x ] = 150;

				if (pSearched[x])
					pSearched[x] = 0;
				pedge[x] = 0;
			}


			if (bEdge)
			{
				if (pSearched[x])
					pedge[x] = 255;
				else
					pedge[x] = 150;
			}
			else if (pedge[x] || pSearched[x])
				bEdge = true;

		}

		bEdge = false;
		for (size_t x = nWarp2DWidth - 1; x >= nCenterX; x--)
		{
			if (x > ptRpt.x)
			{
				if (pedge[x] )
					pedge[ptRpt.x - 1] = 150;

				pSearched[x] = 0;
				pedge[x] = 0;
			}
			if (bEdge)
			{
				if (pSearched[x])
					pedge[x] = 255;
				else
					pedge[x] = 150;
			}
			else if (pedge[x] || pSearched[x])
				bEdge = true;

		}


	}
	m_pProcMilAlgo->SaveWorkImg(Edges, nWarp2DWidth, nHeight, _T("WireFitting_Warp_FootArea.bmp"));

	//delete[] puTW;
	g_pMManager->pem_delete(puTW, true);

	//delete[] puTR;
	g_pMManager->pem_delete(puTR, true);

	//delete[] puMR;
	g_pMManager->pem_delete(puMR, true);

	//delete[] puBR;
	g_pMManager->pem_delete(puBR, true);


}

cv::Point CPInsp_WireBonding::WarpAffine_IPPRotate(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev)
{
	cv::Point Cent, NCenter, temp;
	cv::Point Margin(0, 0);
	int Maxsz = orgImg.cols > orgImg.rows ? orgImg.cols : orgImg.rows;
	Cent.x = (orgImg.cols - 1) / 2;// MaxI;
	Cent.y = (orgImg.rows - 1) / 2;// MaxI;
	cv::Mat Img = orgImg.clone();
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_orgImg.bmp", orgImg);
#endif
	Margin.x = Center.x - orgImg.cols / 2;
	Margin.y = Center.y - orgImg.rows / 2;

	NCenter = Center;
	temp.x = 0;
	temp.y = 0;

	if (abs(Center.x - Cent.x) > 2 || abs(Center.y - Cent.y) > 2)
	{
		cv::Point TmpM;
		if (Rev)
		{
			TmpM.x = Center.x - Cent.x;// +Margin.x * 2;
			TmpM.y = Center.y - Cent.y;// +Margin.y * 2;
		}
		else
		{
			TmpM.x = Center.x - Cent.x;
			TmpM.y = Center.y - Cent.y;
		}

		int w = TmpM.x <= 0 ? Img.cols + (Img.cols - Center.x - Center.x) : Img.cols + (Center.x * 2 - Img.cols);
		int h = TmpM.y <= 0 ? Img.rows + (Img.rows - Center.y - Center.y) : Img.rows + (Center.y * 2 - Img.rows);

		cv::Mat TempImage(h, w, orgImg.type());
		TempImage.setTo(0);

		temp.x = TempImage.cols / 2 - Center.x;// +TmpM.x;
		temp.y = TempImage.rows / 2 - Center.y;// +TmpM.y;

		cv::Mat roi = TempImage(cv::Rect(temp.x, temp.y, Img.cols, Img.rows));

		orgImg.copyTo(roi);

		Img = TempImage;
		NCenter.x = Img.cols / 2;
		NCenter.y = Img.rows / 2;

	}

	bool bLinear = false;
	bool bColor = false;

	bool bUseOrgSize = true;

	int retRotateDstSizeX, retRotateDstSizeY;
	//unsigned char* ucWing2DSrc;
	UCHAR* ucWing2DDst = NULL;

	m_pProcMilAlgo->RotateImg_ipp2020_2(Img.data, &ucWing2DDst, Seta, Img.cols, Img.rows, &retRotateDstSizeX, &retRotateDstSizeY, bColor, bLinear, bUseOrgSize, NCenter);

	cv::Mat RotImage(retRotateDstSizeY, retRotateDstSizeX, CV_MAKETYPE(CV_8U, Img.channels()));
	memcpy(RotImage.data, ucWing2DDst, sizeof(UCHAR)*retRotateDstSizeX*retRotateDstSizeY);

	Delete_1DArray(&ucWing2DDst);

	//cv::Mat RotImage;
	//cv::Mat rotation = getRotationMatrix2D(NCenter, Seta, 1);

	//if (CV_MAT_TYPE(orgImg.type()) == CV_32FC1)
	//	cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_LINEAR);
	//else
	//	cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_NEAREST);

#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_RotImage.bmp", RotImage);
#endif

	int MarX(0), MarY(0);
	if (NOtCent)
	{
		MarX = Margin.x;
		MarY = Margin.y;
		if (Rev)
		{
			MarX += Margin.x*0.5;
			MarY += Margin.y*0.5;
		}
	}

	temp.x = RotImage.cols / 2 - Cent.x - MarX;
	temp.y = RotImage.rows / 2 - Cent.y - MarY;

	//Center가 이미지 중심에서 오른쪽에 있을 경우
	if (Center.x > Cent.x)
	{
		temp.x = RotImage.cols / 2 - Center.x - MarX;
		Margin.x = 0;
	}

	//Center가 이미지 중심에서 아래에 있을 경우
	if (Center.y > Cent.y)
	{
		temp.y = RotImage.rows / 2 - Center.y - MarY;
		Margin.y = 0;
	}

	cv::Mat Crop = RotImage;
	int x(0), y(0), w = orgImg.cols, h = orgImg.rows;
	if (temp.x < 0)
	{
		x = -temp.x;
		w = orgImg.cols + temp.x;

		temp.x = 0;

		if (temp.y < 0)
		{
			y = -temp.y;
			h = orgImg.rows + temp.y;

			temp.y = 0;
			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else if (temp.y + orgImg.rows > RotImage.rows)
		{
			y = temp.y;
			y = 0;
			//int diffh = (orgImg.rows + temp.y) - RotImage.rows;
			//h = (orgImg.rows - diffh);
			h = RotImage.rows - temp.y;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else
		{
			y = temp.y;
			h = orgImg.rows;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, 0, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
	}
	else if (temp.x + orgImg.cols > RotImage.cols)
	{
		x = temp.x;
		x = 0;

		w = RotImage.cols - temp.x;

		if (temp.y < 0)
		{
			y = -temp.y;
			h = orgImg.rows + temp.y;

			temp.y = 0;
			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else if (temp.y + orgImg.rows > RotImage.rows)
		{
			y = temp.y;
			y = 0;

			h = RotImage.rows - temp.y;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else
		{
			y = temp.y;
			h = orgImg.rows;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, 0, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
	}
	if (temp.y < 0)
	{
		y = -temp.y;
		h = orgImg.rows + temp.y;

		temp.y = 0;
		cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

		roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
		roImg.setTo(0);
		cv::Mat roi = roImg(cv::Rect(0, y, w, h));

		cropImg.copyTo(roi);
		return Margin;
	}
	else if (temp.y + orgImg.rows > RotImage.rows)
	{
		y = temp.y;
		y = 0;

		h = RotImage.rows - temp.y;

		cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

		roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
		roImg.setTo(0);
		cv::Mat roi = roImg(cv::Rect(0, y, w, h));

		cropImg.copyTo(roi);
		return Margin;
	}

	roImg = RotImage(cv::Rect(temp.x, temp.y, orgImg.cols, orgImg.rows)).clone();
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_roImg.bmp", roImg);
#endif
	return Margin;
}


bool CPInsp_WireBonding::CurveFitting2(UCHAR* ucImage, int nWidth, int nHeight, bool bHorizon, POINT *poArrFoot, int nThMin, int nTh, int nthMax, std::vector<double>& coeff, float* pfImgSrc)
{
	//두점의 직선 방정식
	double dA, dB;
	double dArrLineX[2], dArrLineY[2];
	dArrLineX[0] = poArrFoot[0].x;
	dArrLineY[0] = poArrFoot[0].y;
	dArrLineX[1] = poArrFoot[1].x;
	dArrLineY[1] = poArrFoot[1].y;
	CPInsp::GetGradient(dArrLineX, dArrLineY, (double)2, dA, dB, bHorizon);
	cv::Mat cv3DData(nHeight, nWidth, CV_32FC1, pfImgSrc);

	// 두점간 검사 영역 계산
	int nW = (bHorizon) ? nWidth : nHeight;
	int nH = (bHorizon) ? nHeight : nWidth;
	int nStartX = (bHorizon) ? poArrFoot[0].x : poArrFoot[0].y;
	int nEndX = (bHorizon) ? poArrFoot[1].x : poArrFoot[1].y;
	int nStartY = (bHorizon) ? poArrFoot[0].y : poArrFoot[0].x;
	int nEndY = (bHorizon) ? poArrFoot[1].y : poArrFoot[1].x;
	if (nStartX > nEndX)
	{
		nEndX = (bHorizon) ? poArrFoot[0].x : poArrFoot[0].y;
		nStartX = (bHorizon) ? poArrFoot[1].x : poArrFoot[1].y;
	}
	if (nStartY > nEndY)
	{
		nEndY = (bHorizon) ? poArrFoot[0].y : poArrFoot[0].x;
		nStartY = (bHorizon) ? poArrFoot[1].y : poArrFoot[1].x;
	}
	if (nStartX < 0) nStartX = 0;
	if (nStartY < 0) nStartY = 0;
	if (nEndX < 0) nEndX = 0;
	if (nEndY < 0) nEndY = 0;

	if (bHorizon)
	{
		if (nStartX > nWidth) nStartX = nWidth;
		if (nStartY > nHeight) nStartY = nHeight;
		if (nEndX > nWidth) nEndX = nWidth;
		if (nEndY > nHeight) nEndY = nHeight;
	}
	else
	{
		if (nStartX > nHeight) nStartX = nHeight;
		if (nStartY > nWidth) nStartY = nWidth;
		if (nEndX > nHeight) nEndX = nHeight;
		if (nEndY > nWidth) nEndY = nWidth;
	}

	std::vector<POINTF> vecCenter;
	std::vector<POINTF> vecEdges;
	RECT rectTH;

#if _DEBUG
	//UCHAR* ucImageCenterLine = new UCHAR[nHeight * nWidth];
	UCHAR* ucImageCenterLine = g_pMManager->pem_new<UCHAR>(true, nHeight * nWidth, (PCHAR)__FUNCTION__, __LINE__);

	memset(ucImageCenterLine, 0, nHeight * nWidth);
#endif

	jsl::LeastSquare lsCalc;
	std::vector< jsl::Point2d<double> > dataList;

	//Curve 중심점 구함
	for (int x = nStartX; x < nEndX; x++)
	{
		//직선 방정식 상의 중심 좌표
		int x2 = x;
		int y2 = dB;
		if (dA != 0)
			y2 = dA * x + dB;

		int nIndex = y2 * nW + x2;
		if (!bHorizon) nIndex = x2 * nH + y2;
		if (nIndex < 0 || nIndex >= nWidth * nHeight) continue;

#if _DEBUG			
		ucImageCenterLine[nIndex] = 255;
#endif

		double th = InspectThByXY(ucImage, nWidth, nHeight, x, bHorizon, dA, dB, &rectTH, vecCenter, vecEdges);

		if (th < nThMin) continue;
		else if (th > nthMax)
		{
			if (vecCenter.size() > 2)
				continue;

			if (vecEdges.size() < 3)
				continue;

			int selectindex = 0;
			{
				double DistanceA = sqrt(pow(x2 - vecCenter[0].x, 2) + pow(y2 - vecCenter[0].y, 2));
				double DistanceB = sqrt(pow(x2 - vecCenter[1].x, 2) + pow(y2 - vecCenter[1].y, 2));

				if (!bHorizon)
				{
					DistanceA = sqrt(pow(x2 - vecCenter[0].y, 2) + pow(y2 - vecCenter[0].x, 2));
					DistanceB = sqrt(pow(x2 - vecCenter[1].y, 2) + pow(y2 - vecCenter[1].x, 2));
				}

				if (DistanceA > DistanceB)
					selectindex = 1;
			}

			POINTF ptA = vecEdges[selectindex * 2];
			POINTF ptB = vecEdges[selectindex * 2 + 1];

			double Thsub = sqrt(pow(ptB.x - ptA.x, 2) + pow(ptB.y - ptA.y, 2));

			if (nThMin < Thsub&& Thsub < nthMax)
			{
				int x3 = (ptB.x + ptA.x) / 2.;
				int y3 = (ptB.y + ptB.y) / 2.;

				if (!bHorizon)
				{
					int ntemp = x3;
					x3 = y3;
					y3 = ntemp;
				}

				dataList.emplace_back((double)x3, (double)y3);

				int nIndex = y3 * nW + x3;
				if (!bHorizon) nIndex = x3 * nH + y3;
				if (nIndex < 0 || nIndex >= nWidth * nHeight) continue;

#if _DEBUG	
				ucImageCenterLine[nIndex] = 150;
#endif
			}
		}
		else
		{
			int x3 = (rectTH.left + rectTH.right) / 2.;
			int y3 = (rectTH.top + rectTH.bottom) / 2.;

			if (!bHorizon)
			{
				int ntemp = x3;
				x3 = y3;
				y3 = ntemp;
			}

			int nIndex = y3 * nW + x3;
			if (!bHorizon) nIndex = x3 * nH + y3;
			if (nIndex < 0 || nIndex >= nWidth * nHeight) continue;

			dataList.emplace_back((double)x3, (double)y3);

#if _DEBUG			
			ucImageCenterLine[nIndex] = 150;
#endif

		}
	}

	bool b2DInterpol = false;
	coeff.clear();

	// X에 대한 7차 방장식 계수
	if (dataList.size() > 30 && dataList.size() > (nEndX - nStartX) / 3.)
		lsCalc.RunSvd(7, dataList, coeff);

	if (coeff.size() == 8)
		b2DInterpol = true;

	if (b2DInterpol)
	{
		//원래 좌표보다 떨어져 있으면 실패
		jsl::Point2d<double> pt = dataList[dataList.size() / 2];

		float fy = lsCalc.ApplySvd(pt.x, coeff);
		float diff = std::abs(pt.y - fy);

		if (diff > nThMin / 2.)
			b2DInterpol = false;
	}

#if _DEBUG
	cv::Mat src(nHeight, nWidth, CV_8UC1, ucImage);
	cv::Mat OutPut3Ch[] = { src ,src ,src };
	cv::Mat cvDrawDebug_1;
	cv::merge(OutPut3Ch, 3, cvDrawDebug_1);
#endif 
	std::vector< jsl::Point2d<double> >removeDataList = dataList;
	jsl::Point2d<double> ptZero;
	int nCnt = 0;
	
//  	if (b2DInterpol)
//  	{
		int nStData = (int)(dataList.size() /(100/5)) ; 
 		for (size_t i = nStData; i < dataList.size()- nStData; i ++)
 		{
 			jsl::Point2d<double> pt = dataList[i];
 
 			float fy = lsCalc.ApplySvd(pt.x, coeff);
 			float diff = std::abs(pt.y - fy);
#if _DEBUG
			cv::circle(cvDrawDebug_1, (bHorizon)? cv::Point(pt.x, pt.y): cv::Point(pt.y, pt.x), 1, cv::Scalar(255, 0, 0), 1, 8);
#endif 
			if (diff > nThMin / 2.)
			{
				
				if (i >= 3)
				{
					removeDataList[i] = ptZero;
#if _DEBUG
					cv::circle(cvDrawDebug_1, (bHorizon) ? cv::Point(removeDataList[i].x, removeDataList[i].y) : cv::Point(removeDataList[i].y, removeDataList[i].x), 1, cv::Scalar(0, 255, 0), 1, 8);
#endif 
				}
				
				b2DInterpol = false;
			}
				
 		}
 	//}
#if _DEBUG
	cv::Mat cvDrawDebug_2;
	cv::merge(OutPut3Ch, 3, cvDrawDebug_2);
#endif

	if (!b2DInterpol) // 문제되는 포인트 제거.
	{

		for (vector<jsl::Point2d<double>>::iterator it = removeDataList.begin(); it != removeDataList.end();) {
			if (*it == ptZero) 
			{
				it = removeDataList.erase(it);
			}
			else 
			{
#if _DEBUG
				cv::circle(cvDrawDebug_2, (bHorizon) ? cv::Point(it->x, it->y) : cv::Point(it->y, it->x), 1, cv::Scalar(255, 0, 0), 1, 8);
#endif
				++it;
			}
		}
		coeff.clear();

		// X에 대한 7차 방장식 계수
		if (removeDataList.size() > 30 && removeDataList.size() > (nEndX - nStartX) / 3.)
			lsCalc.RunSvd(7, removeDataList, coeff);

		if (coeff.size() == 8)
			b2DInterpol = true;

		if (b2DInterpol)
		{
			//원래 좌표보다 떨어져 있으면 실패
// 			int nOffset = removeDataList.size() / 10;
// 			for (int i = 1; i < 10; i++)
// 			{
// 				jsl::Point2d<double> pt = removeDataList[(nOffset * i)];
// 
// 				float fy = lsCalc.ApplySvd(pt.x, coeff);
// 				float diff = std::abs(pt.y - fy);
// 
// 				if (diff > nThMin)
// 					b2DInterpol = false;
// 			}
			int  binP(cv3DData.cols);
			for (int i = 1; i < removeDataList.size(); i++)
			{
				//복원 기능을 사용할때 Dist 가 100이상 넘으면 복원하지못하도록 변경 
				double dResult = sqrt(pow((bHorizon) ? (removeDataList[i - 1].x - removeDataList[i].x) : (removeDataList[i - 1].y - removeDataList[i].y), 2) + pow((bHorizon) ? (removeDataList[i - 1].y - removeDataList[i].y) : (removeDataList[i - 1].x - removeDataList[i].x), 2));
				if (100 < dResult)
				{
					//std::vector< cv::Point >AvgC; 
					std::vector< double >AvgD;
					int nCheckCnt = 10;
					double dAvg[2] = {0,0};
					float fPCA_A(0), fPCA_B(0);
					for (int n = 0; n < 2; n++) //끊기는 부분 전과 후 
					{
						if (i > nCheckCnt && i < removeDataList.size() - nCheckCnt) // 10 Pixel PCA
						{
							dAvg[n] = 0;
							if (bHorizon)
							{
								if (n == 0)
								{
									for (int j = i - nCheckCnt; j < i; j++)
										//AvgD.push_back(pfImgSrc[(int)removeDataList[j].y* binP + (int)removeDataList[j].x]);
										dAvg[0] += pfImgSrc[(int)removeDataList[j].y* binP + (int)removeDataList[j].x];
								}
								else
								{
									for (int j = i; j < i + nCheckCnt; j++)
										//AvgD.push_back(pfImgSrc[(int)removeDataList[j].y* binP + (int)removeDataList[j].x]);
										dAvg[1] += pfImgSrc[(int)removeDataList[j].y* binP + (int)removeDataList[j].x];
								}

								//AvgD.clear();
							}
							else
							{
								if (n == 0)
								{
									for (int j = i - nCheckCnt; j < i; j++)
										//AvgD.push_back(pfImgSrc[(int)removeDataList[j].x* binP + (int)removeDataList[j].y]);
										dAvg[0] += pfImgSrc[(int)removeDataList[j].x* binP + (int)removeDataList[j].y];
								}
								else
								{
									for (int j = i; j < i + nCheckCnt; j++)
										dAvg[1] += pfImgSrc[(int)removeDataList[j].x* binP + (int)removeDataList[j].y];
									//AvgD.push_back(pfImgSrc[(int)removeDataList[j].x* binP + (int)removeDataList[j].y]);
								}
								//AvgD.clear();
							}

						}
					}
				
					double dLow = (dAvg[0] / nCheckCnt >= dAvg[1] / nCheckCnt)? dAvg[1] / nCheckCnt : dAvg[0] / nCheckCnt;
					double dHi = (dAvg[0] / nCheckCnt >= dAvg[1] / nCheckCnt) ? dAvg[0] / nCheckCnt : dAvg[1] / nCheckCnt;
					
					if ((abs(dHi - dLow) / dResult) > 1)
						b2DInterpol = false;

					CString strLogArr;
					strLogArr.Format(_T("Low : %f / Hi : %f / m : %f  / result : %f"), dLow, dHi, abs(dHi - dLow), dResult);
					g_pMPTI->AddLog(strLogArr);

				}
			}
		}
	}
		


#if _DEBUG
	m_pProcMilAlgo->SaveWorkImg(ucImage, nWidth, nHeight, _T("WireFitting_FloodFill2.bmp"));
	m_pProcMilAlgo->SaveWorkImg(ucImageCenterLine, nWidth, nHeight, _T("WireFitting_ImageCenterLine2.bmp"));
	//delete[] ucImageCenterLine;
	g_pMManager->pem_delete(ucImageCenterLine, true);
#endif

	return b2DInterpol;

}

double CPInsp_WireBonding::InspectThByXY(UCHAR* pucImage, int nImgWidth, int nImgHeight, int nX, bool bHorizon, double dA, double dB, RECT* rect, std::vector<POINTF>& vecCenter, std::vector<POINTF>& vecEdges)
{
	vecEdges.clear();
	vecCenter.clear();

	memset(rect, 0, sizeof(RECT));

	int nW = (bHorizon) ? nImgWidth : nImgHeight;
	int nH = (bHorizon) ? nImgHeight : nImgWidth;

	double dY_Buf = dA * nX + dB;
	int nY_Buf = (int)RounD(dY_Buf);
	if (nY_Buf < 0 || nY_Buf > nH) return 0.;
	double dA2 = 0;
	if (dA != 0) dA2 = -1.0 / dA;
	double dB2 = dY_Buf - (dA2 * nX);
	int nIdx = nY_Buf * nW + nX;
	if (!bHorizon) nIdx = nX * nH + nY_Buf;
	if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) return 0.;
	int nMinBX = -1, nMinBY = -1, nMaxBX = -1, nMaxBY = -1;

	int nBlobNum = 0;
	bool bBlob = false;
	for (int a2 = 0; a2 < nH; a2++)
	{
		int nYTotal = 1;
		int nM = 1;
		if (dA2 == 0)
		{
		// 수평일 경우 한줄만.
		//	nYTotal = nH;
			nY_Buf = nX;
		}
		else
		{
			dY_Buf = (a2 - dB2) / dA2;
			nY_Buf = (int)RounD(dY_Buf);
			if (nY_Buf < 0 || nY_Buf > nW) continue;;
			double dY_Buf2 = ((a2 + 1) - dB2) / dA2;
			int nY_Buf2 = (int)RounD(dY_Buf2);
			if (nY_Buf2 >= 0 && nY_Buf2 < nW)
			{
				nYTotal = nY_Buf2 - nY_Buf;
				if (nYTotal < 0)
				{
					nM = -1;
					nYTotal *= nM;
				}
			}
		}
		if (nYTotal == 0) nYTotal = 1;
		for (int a3 = 0; a3 < nYTotal; a3++)
		{
			int nX2 = (a2 + (a3 * nM));

			nIdx = nX2 * nImgWidth + nY_Buf;
			if (!bHorizon) nIdx = nY_Buf * nImgWidth + nX2;
			if (nIdx < 0 || nIdx >= nImgWidth * nImgHeight) continue;

			int BX = (bHorizon) ? nY_Buf : nX2;
			int BY = (bHorizon) ? nX2 : nY_Buf;

			if (bHorizon)
			{
				if (BX < 0 || nW < BX)
					continue;

				if (BY < 0 || nH < BY)
					continue;
			}
			else
			{
				if (BX < 0 || nH < BX)
					continue;

				if (BY < 0 || nW < BY)
					continue;
			}

			if (pucImage[nIdx] == 0 && bBlob == false) continue;

			if (pucImage[nIdx])
			{
				if (nMinBY == -1)
				{
					nMinBX = BX;
					nMinBY = BY;
				}
				nMaxBX = BX;
				nMaxBY = BY;
			}

			if (pucImage[nIdx])
			{
				POINTF pt = { BX, BY };
				if (bBlob == false)
				{
					vecEdges.emplace_back(pt);
					vecEdges.emplace_back(pt);
					bBlob = true;
				}
				else
				{
					vecEdges[vecEdges.size() - 1] = pt;
				}
			}
			else
				bBlob = false;

		}
	}
	if (nMinBX < 0 || nMaxBX < 0 || nMinBY < 0 || nMaxBY < 0)
		return 0.;

	rect->left = nMinBX;
	rect->right = nMaxBX;
	rect->top = nMinBY;
	rect->bottom = nMaxBY;

	double dThickness = sqrt(pow((nMaxBX - nMinBX), 2) + pow((nMaxBY - nMinBY), 2));		//두 점 사이의 거리 구하기

	//Center
	for (size_t i = 0; i < vecEdges.size(); i += 2)
	{
		POINTF ptCenter;
		ptCenter.x = (vecEdges[i + 1].x + vecEdges[i].x) / 2;
		ptCenter.y = (vecEdges[i + 1].y + vecEdges[i].y) / 2;
		vecCenter.emplace_back(ptCenter);
	}

	return dThickness;

}

int CPInsp_WireBonding::GetWireBinPoint(int nNumPoints, POINTF* pts, float* ph2)
{
	if (m_nPointNum != nNumPoints || m_nPointNum == 0)
		return 0;

	memcpy(pts, m_pWirePoint, m_nPointNum * sizeof(POINTF));
	memcpy(ph2, m_WirepH2, m_nPointNum * sizeof(float));

	return nNumPoints;
}
double CPInsp_WireBonding::returnTime()
{
	return m_vPInspFoot[m_nFootType]->returnTime();
}

bool CPInsp_WireBonding::GetCenterPoints(std::vector<long long>& m_vWireCenterPts)
{
	bool bResult = true;
	if (m_vWireCenterPts.size() == 0)
		bResult = false;
	return bResult;
}
bool CPInsp_WireBonding::ClearWirePoints()
{
	bool bResult = true;
	if (m_vTupleWirePoint.size() != 0)
	{
		for (int i = 0; i < m_vTupleWirePoint.size(); i++)
		{
			g_pMManager->pem_delete((POINTF *)get<0>(m_vTupleWirePoint[i]), true); // Wire Min 제거
			g_pMManager->pem_delete((POINTF *)get<1>(m_vTupleWirePoint[i]), true); // Wire Max 제거. get <2> 은 Index 이므로 Clrea 로 조치.
		}
		m_vTupleWirePoint.clear();
	}
	else
		bResult = false;


	return bResult;
}

//////////////////foot
bool CPInsp_WireBonding::CreateModel(cv::Mat Image2D, CString FilePath, int FootType, RECT MRect, int FootDirection, int DBC)
{
	return m_vPInspFoot[m_nFootType]->CreateModel(Image2D, FilePath, FootType, MRect, FootDirection, DBC);
}
bool CPInsp_WireBonding::CreateModel3D(cv::Mat Image3D, CString FilePath, int FootType, RECT MRect, int FootDirection, int DBC)
{
	return m_vPInspFoot[m_nFootType]->CreateModel3D(Image3D, FilePath, FootType, MRect, FootDirection, DBC);
}
bool CPInsp_WireBonding::MatchModel(cv::Mat Image2D, CString FilePath, int FootType)
{
	return m_vPInspFoot[m_nFootType]->MatchModel(Image2D, FilePath, FootType);
}
bool CPInsp_WireBonding::MatchModel3D(cv::Mat Image3D, CString FilePath, int FootType)
{
	return m_vPInspFoot[m_nFootType]->MatchModel3D(Image3D, FilePath, FootType);
}

CFoot_Model* CPInsp_WireBonding::MakeFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D, bool bUsePadAreaAutoTeach)
{
	if (sInspAlgo.m_eAlgoType == eAlgoFoot)
	{
		AlgoFoot* alFoot = (AlgoFoot*)sInspAlgo.m_ptrInspAlgoParam;
		SetFootType(alFoot->m_nFootType);
	}
	return m_vPInspFoot[m_nFootType]->MakeFoot(sInspAlgo, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, cv3DAvgFilter, cvDBCPolygonImg, bCorrect3D, bUsePadAreaAutoTeach);
}
CFoot_Model* CPInsp_WireBonding::MakeFoot(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat* cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D, bool bUsePadAreaAutoTeach)
{
	return m_vPInspFoot[m_nFootType]->MakeFoot(sInspFoot, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, cv3DAvgFilter, cvDBCPolygonImg, bCorrect3D, bUsePadAreaAutoTeach);
}
void CPInsp_WireBonding::SetWireOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax, int nFootPatternMatchStopScore)
{
	for (int i = 0; i < m_vPInspFoot.size(); i++)
	{
		m_vPInspFoot[i]->m_nFootPadTopWGrayLevel = nFootPadTopWGrayLevel;
		m_vPInspFoot[i]->m_nFootPatternMatchScore = nFootPatternMatchScore;
		m_vPInspFoot[i]->m_bMatch2D = bMatch2D;
		m_vPInspFoot[i]->m_bMatch3D = bMatch3D;

		m_vPInspFoot[i]->m_nFootSobelmin = nSobelmin;
		m_vPInspFoot[i]->m_nFootSobelmax = nSobelmax;
		m_vPInspFoot[i]->m_nFootPatternMatchStopScore = nFootPatternMatchStopScore;
	}
}
int CPInsp_WireBonding::GetFootSobelMin()
{
	int rtn = 0;
	for (int i = 0; i < m_vPInspFoot.size(); i++)
	{
		rtn = m_vPInspFoot[i]->m_nFootSobelmin;
	}
	return rtn;
}
int CPInsp_WireBonding::GetFootSobelMax()
{
	int rtn = 0;
	for (int i = 0; i < m_vPInspFoot.size(); i++)
	{
		rtn = m_vPInspFoot[i]->m_nFootSobelmax;
	}
	return rtn;
}
bool CPInsp_WireBonding::TeachFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, UCHAR *ucArrDstImg)
{
	return m_vPInspFoot[m_nFootType]->TeachFoot(sInspAlgo, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, nfootKind, ucArrDstImg);
}
bool CPInsp_WireBonding::TeachFootColor(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, FootColor fcFindDBC, UCHAR *ucArrDstImg, cv::Mat *cv3DAvgFilter)
{
	return m_vPInspFoot[m_nFootType]->TeachFootColor(sInspAlgo, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, nfootKind, fcBackground, fcFoot, fcWing, fcWedge, fcFindDBC, ucArrDstImg, cv3DAvgFilter);
}
BOOL CPInsp_WireBonding::InspFoot(CFoot_Model* pFoot, WndAlgoImg &sWndAlgoImg, RstAlgoFoot * sRstAlgo, TotalInspExceptArea stTieArea, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, int nSelectFoot, UCHAR *ucArrDstImg, AlignResult * arrAlignRes, bool bWingRstVisible, CString csDebugFilePath)
{
#ifdef ALGORITHMTOOL_CPP_USE
	m_vPInspFoot[m_nFootType]->nFootAlgoCnt++;
#endif

	return m_vPInspFoot[m_nFootType]->InspFoot(pFoot, sWndAlgoImg, sRstAlgo, stTieArea, ptrAlgoColorOpt, nSelectFoot, ucArrDstImg, arrAlignRes, bWingRstVisible, cv3DAvgFilter, csDebugFilePath);
}
bool CPInsp_WireBonding::MatchModel(cv::Mat Image2D, int FootType)
{
	return m_vPInspFoot[m_nFootType]->MatchModel(Image2D, FootType);
}
bool CPInsp_WireBonding::MatchModel3D(cv::Mat Image3D, int FootType)
{
	return m_vPInspFoot[m_nFootType]->MatchModel3D(Image3D, FootType);
}
bool CPInsp_WireBonding::MatchModel(CFoot_Model* pFoot, cv::Mat Image2D, cv::Mat cvPadImg, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, cv::Mat ImgFoot2DBW, cv::Mat ImgFoot3D, int* MatchModelNum, double dAngle)
{
	return m_vPInspFoot[m_nFootType]->MatchModel(pFoot, Image2D, cvPadImg, m_PatResult, szModelImg, ImgFoot2DBW, ImgFoot3D, MatchModelNum);
}
bool CPInsp_WireBonding::MatchModel3D(CFoot_Model* pFoot, cv::Mat Image3D, cv::Mat cvPadImg, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, cv::Mat ImgFoot2DBW, int* MatchModelNum, double dAngle)
{
	return m_vPInspFoot[m_nFootType]->MatchModel3D(pFoot, Image3D, cvPadImg, m_PatResult, szModelImg, ImgFoot2DBW, MatchModelNum, dAngle);
}
bool CPInsp_WireBonding::MatchModel(cv::Mat Image2D, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, int nFootDirection, int footType)
{
	return m_vPInspFoot[m_nFootType]->MatchModel(Image2D, m_PatResult, szModelImg, nFootDirection, footType);
}
bool CPInsp_WireBonding::MatchModel3D(cv::Mat Image3D, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, int nFootDirection, int footType)
{
	return m_vPInspFoot[m_nFootType]->MatchModel3D(Image3D, m_PatResult, szModelImg, nFootDirection, footType);
}
cv::Mat CPInsp_WireBonding::Correct3DRemoveLow(cv::Mat Image3D)
{
	return m_vPInspFoot[m_nFootType]->Correct3DRemoveLow(Image3D);
}
bool CPInsp_WireBonding::GetFootModelParam(CString sFileName2D, int* FootType, RECT* MRect, double* resX, double* resY)
{
	return m_vPInspFoot[m_nFootType]->GetFootModelParam(sFileName2D, FootType, MRect, resX, resY);
}
float CPInsp_WireBonding::GetCorrect3d_Binary(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrAlgoColorOpt, POINT poXY, int nDBC)
{
	return m_vPInspFoot[m_nFootType]->GetCorrect3d_Binary(sInspAlgo, sWndAlgoImg, ptrAlgoColorOpt, poXY, nDBC);
}
bool CPInsp_WireBonding::PCA_1D(int cnt, cv::Point* pint, float* a, float* b)
{
	cv::Mat matrixInsp(cnt, 2, CV_32F);
	//cv::Mat MatrixStd(4,2,CV_32F);
	for (int i = 0; i < cnt; i++)
	{
		matrixInsp.ptr<float>(i)[0] = pint[i].x;		matrixInsp.ptr<float>(i)[1] = pint[i].y;
	}

	cv::Mat covar, comeam, eigenVal, eigenMt;
	cv::calcCovarMatrix(matrixInsp, covar, comeam, cv::COVAR_NORMAL | cv::COVAR_ROWS, CV_32F);
	float* tmp1 = covar.ptr<float>(0);
	covar /= (matrixInsp.rows - 1);
	cv::eigen(covar, eigenVal, eigenMt);	//주성분(그 방향으로 데이터들의 분산이 가장 큰 방향벡터) 분석
	float* tmpV = eigenVal.ptr<float>(0);
	float* tmpM = eigenMt.ptr<float>(0);
	float tmpA = tmpM[2];
	float tmpB = tmpM[3];
	float tmpx1 = matrixInsp.ptr<float>(0)[0];
	float tmpy1 = matrixInsp.ptr<float>(0)[1];

	*a = -tmpA / tmpB;
	*b = tmpA / tmpB * tmpx1 + tmpy1;
	if (tmpB == 0)
		return false;
	return true;
}