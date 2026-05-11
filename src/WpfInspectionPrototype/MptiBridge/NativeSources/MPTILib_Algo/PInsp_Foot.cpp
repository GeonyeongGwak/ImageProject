#pragma once
#include "stdafx.h"
#include "PInsp_Foot.h"
#include "MPTI.h"

#include <vector>

#include "LeastSquare.h"
#include <array>
#include "HistogramAnalysis_New.h"

#include <algorithm>
#include <iostream>
#include <string>

//#include "ippm.h"
#include "ipps.h"
#include "ippi.h"
#pragma comment(lib, "ippi.lib")
#pragma comment(lib, "ipps.lib")
using namespace std;
#define MRad_deg90(a) std::abs(1.5707-a) < 0.01

double g_LTCos_foot[41];	// -20 to 20 degrees
double g_LTSin_foot[41];	// -20 to 20 degrees

void Log_WireModel(CString msg)
{

}
CFoot_Model::CFoot_Model()
{
	m_nWindowStartX = 0;
	m_nWindowStartY = 0;

	//m_ImageBuffer = new InspRoiImgBuf;
	m_ImageBuffer = g_pMManager->pem_new<InspRoiImgBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__);


	memset(m_ImageBuffer, 0, sizeof(InspRoiImgBuf));
	pf3D_OrgROI = NULL;
	pf3D = NULL;
	pf3DOrg = NULL;

	memset(&mImageRect, 0, sizeof(RECT));
	memset(&mWindowImageRect, 0, sizeof(RECT));

	memset(&m_RefRect, 0, sizeof(RECT));
	memset(&m_PadRect, 0, sizeof(RECT));

	m_nUseOption = 0;
	m_nUseOption2 = 0;

	dAngle = 0.;

	m_rst_Color_Image = false;

	m_fcBackground.nRed = 90;
	m_fcBackground.nGreen = 90;
	m_fcBackground.nBlue = 90;

	m_fcFoot.nRed = 125;
	m_fcFoot.nGreen = 0;
	m_fcFoot.nBlue = 0;

	m_fcWing.nRed = 0;
	m_fcWing.nGreen = 125;
	m_fcWing.nBlue = 0;

	m_fcWedge.nRed = 0;
	m_fcWedge.nGreen = 0;
	m_fcWedge.nBlue = 125;

	m_fcFindDBC.nRed = 0;
	m_fcFindDBC.nGreen = 0;
	m_fcFindDBC.nBlue = 125;

	m_pPad = NULL;
	m_fPadArea3DAvgHeight = 0.0f;

	m_pDBCPolygonImg = NULL;

	m_RoRect = nullptr;

	bUseTeachRect = false;
	nPadWidth = 147;
	nPadHeight = 170;
	nRadius = 16;
	bTeachBin = false;
}

CFoot_Model::~CFoot_Model()
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

	if (pf3D_OrgROI != NULL) {
		//delete[] pf3D_OrgROI; 
		g_pMManager->pem_delete(pf3D_OrgROI, true);
		pf3D_OrgROI = nullptr;
	}

	if (pf3D != NULL) {
		//delete[] pf3D; 
		g_pMManager->pem_delete(pf3D, true);
		pf3D = nullptr;
	}

	if (m_pPad != NULL) {
		//delete[] m_pPad;
		g_pMManager->pem_delete(m_pPad, true);
		m_pPad = nullptr;
	}

	if (m_pDBCPolygonImg != NULL) {
		//delete[] m_pDBCPolygonImg;
		g_pMManager->pem_delete(m_pDBCPolygonImg, true);
		m_pDBCPolygonImg = nullptr;
	}

	if (m_RoRect != nullptr) {
		//delete[] m_RoRect; 
		g_pMManager->pem_delete(m_RoRect, true);
		m_RoRect = nullptr;
	}
}

void CFoot_Model::GetImageMatrix(cv::Mat* Image, int footKind)
{
	cv::Rect ImageRect;
	ImageRect.x = 0;
	ImageRect.y = 0;
	ImageRect.width = m_ImageBuffer->nImageSizeX;
	ImageRect.height = m_ImageBuffer->nImageSizeY;
	AlgoBin bin = m_sArrBin[footKind];
	if (bin.m_bIsSet == false)
		bin = m_sArrBin[0];

	CPInsp_Foot::GetImageMatrix(Image, bin, m_ImageBuffer, ImageRect);
}

void CFoot_Model::GetImageMatrix(cv::Mat* Image, int footKind, cv::Rect ImageRect)
{
	AlgoBin bin = m_sArrBin[footKind];
	if (bin.m_bIsSet == false)
		bin = m_sArrBin[0];

	CPInsp_Foot::GetImageMatrix(Image, bin, m_ImageBuffer, ImageRect);
}

void CFoot_Model::GetPadBinImage(cv::Mat* Image)
{
	if (m_pPad == NULL) return;
	cv::Mat pad(GetImageLength(), GetImageWidth(), CV_8UC1, m_pPad);
	pad.copyTo(*Image);
}

void CFoot_Model::GetDBCPolygonBinImage(cv::Mat* Image)
{
	if (m_pDBCPolygonImg == NULL) return;
	cv::Mat DBCPolygonImg(GetImageLength(), GetImageWidth(), CV_8UC1, m_pDBCPolygonImg);
	DBCPolygonImg.copyTo(*Image);
}

CPInsp_Foot::CPInsp_Foot(void)
{
	m_pInspParam = &m_paramInspect;
	m_pTeachParam = &m_paramTeach;
	
	m_procPatternMil = NULL;

	int num_angles = 41; // -20 to 20 degrees
	for (int i = 0; i < num_angles; i++) {
		double angle_deg = (i - 20) * 1.0;				// convert index to angle
		double angle_rad = angle_deg * CV_PI / 180.0;	// convert to radians
		g_LTCos_foot[i] = cos(angle_rad);				// compute cosine and store in lookup table
		g_LTSin_foot[i] = sin(angle_rad);
	}

	CPInsp::CPInsp();
}

CPInsp_Foot::~CPInsp_Foot(void)
{
	if (m_procPatternMil != NULL)
	{
		m_procPatternMil->FreeMil();

		//delete m_procPatternMil;
		g_pMManager->pem_delete(m_procPatternMil, false);
		m_procPatternMil = NULL;
	}

	//CPInsp::~CPInsp();
}

int CPInsp_Foot::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	// Allocate a Application.
	//m_milApp = milApp;
	// Allocate a System.
	//m_milSys = milSys;

	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;


	if (m_procPatternMil == NULL)
		//m_procPatternMil = new CProcPil_Pattern();
		m_procPatternMil = g_pMManager->pem_new<CProcPil_Pattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	m_procPatternMil->InitMil(milApp, milSys, bUseImagePilLib);
	m_procPatternMil->SetResol(resolX, resolY, m_fovWidth);

	m_ModelMng2D = std::shared_ptr<CMModelManager<ModelFile_Foot>>(new CMModelManager<ModelFile_Foot>(eAlgoFoot, _T(".ppd"), Log_WireModel));
	m_ModelMng3D = std::shared_ptr<CMModelManager<ModelFile_Foot>>(new CMModelManager<ModelFile_Foot>(eAlgoFoot, _T(".ppd"), Log_WireModel));

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, bUseImagePilLib);

	if (m_pProcMilAlgo != NULL)
		m_pProcMilAlgo->InitMilAlgoBlob();

	return ePART_SUCCESS;
}

/*
CFoot_Model* CPInsp_Foot::MakeFoot(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D, bool bPadAreaAutoTeach)
{
	//CFoot_Model* model = new CFoot_Model();
	CFoot_Model* model = g_pMManager->pem_new<CFoot_Model>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	model->m_nWindowStartX = nStartX;
	model->m_nWindowStartY = nStartY;

	model->_WidthOrg = sWndAlgoImg.m_nWidth3D;
	model->_HeightOrg = sWndAlgoImg.m_nHeight3D;

	//Inspect Option
	model->m_nFoottype = sInspFoot->m_nFootType;
	model->m_nUseOption = sInspFoot->m_nUseOption;
	model->m_nUseOption2 = sInspFoot->m_nUseOption2;
	model->m_nFindOption = sInspFoot->m_nFindOption;
	model->m_nFindOption2 = sInspFoot->m_nFindOption2;
	model->m_bUsePatternAngle = sInspFoot->m_bUsePatternAngle;
	model->m_nFootAngle = sInspFoot->m_nFootAngle;
	model->m_dTeachFootAngle = sInspFoot->m_dTeachFootAngle;
	model->m_bUse2Foot = sInspFoot->m_bUse2Foot;
	model->m_bUsePadAreaAutoTeach = sInspFoot->m_bUsePadAreaAutoTeach;
	model->bUse2DImageForInspWidth = sInspFoot->bUse2DImageForInspWidth;

	model->nRemoveWireHeight = sInspFoot->nRemoveWireHeight;

	model->nFootPadSizeX = sInspFoot->nFootPadSizeX;
	model->nFootPadSizeY = sInspFoot->nFootPadSizeY;

	model->nPadEdgeMinRatio = sInspFoot->nPadEdgeMinRatio;

	model->bUseDBCShapeTeaching = sInspFoot->bUseDBCShapeTeaching;

	model->nInspCrackHeight = sInspFoot->nInspCrackHeight;

	CPInsp_WireBonding* PInspWireBonding = g_pInspMng->GetWireBondingInsp();
	if (sInspFoot->nFootSobmin == 0)//설정되어 있지 않으면 머신데이터로가져오기
		model->nFootSobmin = PInspWireBonding->GetFootSobelMin();
	else
		model->nFootSobmin = sInspFoot->nFootSobmin;
	if (sInspFoot->nFootSobmax == 0)
		model->nFootSobmax = PInspWireBonding->GetFootSobelMax();
	else
		model->nFootSobmax = sInspFoot->nFootSobmax;

	model->m_nFindOptionOrder.clear();
	model->m_nFindOptionOrder2.clear();
	for (int i = 0; i < m_eFoot_FindOption::MeanSlope; i++)
		if (sInspFoot->m_nFindOptionOrder[i] != 0)
			model->m_nFindOptionOrder.push_back(sInspFoot->m_nFindOptionOrder[i]);
	for (int i = 0; i < m_eFoot_FindOption::total; i++)
		if (sInspFoot->m_nFindOptionOrder2[i] != 0)
			model->m_nFindOptionOrder2.push_back(sInspFoot->m_nFindOptionOrder2[i]);
	model->dAngle = sWndAlgoImg.dAngle;

	//윈도우 시작좌표 홀수일 경우 예외처리 NYJ 2021/02/16
  	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeX % 2) != 0)		//odd
  	{
  		//model->mImageRect.left += 1;
  		//model->mImageRect.right += 1;
  		ptrAlgoColorOpt.m_rcImageRect.left += 1;
  		ptrAlgoColorOpt.m_rcImageRect.right += 1;
  	}
  	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeY % 2) != 0)		//odd
  	{
  		//model->mImageRect.top += 1;
  		//model->mImageRect.bottom += 1;
  		ptrAlgoColorOpt.m_rcImageRect.top += 1;
  		ptrAlgoColorOpt.m_rcImageRect.bottom += 1;
  	}

	//Clip
	model->mWindowImageRect = ptrAlgoColorOpt.m_rcImageRect;
	int wndWidth = model->mWindowImageRect.right - model->mWindowImageRect.left;
	int wndHeight = model->mWindowImageRect.bottom - model->mWindowImageRect.top;

	if (model->mWindowImageRect.left + wndWidth > ptrAlgoColorOpt.m_sFovImg.nImageSizeX || model->mWindowImageRect.top + wndHeight > ptrAlgoColorOpt.m_sFovImg.nImageSizeY)
	{
		if (model->mWindowImageRect.left + wndWidth > ptrAlgoColorOpt.m_sFovImg.nImageSizeX)
		{
			int nShiftX = ((model->mWindowImageRect.left + wndWidth) - ptrAlgoColorOpt.m_sFovImg.nImageSizeX);
			model->mWindowImageRect.left = model->mWindowImageRect.left - nShiftX;
			model->mWindowImageRect.right = model->mWindowImageRect.right - nShiftX;
		}
		if (model->mWindowImageRect.top + wndHeight > ptrAlgoColorOpt.m_sFovImg.nImageSizeY)
		{
			int nShiftY = ((model->mWindowImageRect.top + wndHeight) - ptrAlgoColorOpt.m_sFovImg.nImageSizeY);
			model->mWindowImageRect.top = model->mWindowImageRect.top - nShiftY;
			model->mWindowImageRect.bottom = model->mWindowImageRect.bottom - nShiftY;
		}
	}

	//Ref Bottom Rect
	model->m_RefRect = sInspFoot->m_sRefArea;

	//Pad Rect
	//model->m_PadRect = sInspFoot->m_rSpecRect;	//NYJ 2020/02/05
	if (!((CRect)sInspFoot->m_rSpecRect).IsRectNull())
	{
		model->m_PadRect = sInspFoot->m_rSpecRect;	//NYJ 2020/02/05

		model->m_PadRect.left += model->mWindowImageRect.left;
		model->m_PadRect.top += model->mWindowImageRect.top;
		model->m_PadRect.right += model->mWindowImageRect.left;
		model->m_PadRect.bottom += model->mWindowImageRect.top;

		if (model->m_PadRect.left < model->mWindowImageRect.left)
			model->m_PadRect.left = model->mWindowImageRect.left;

		if (model->m_PadRect.top < model->mWindowImageRect.top)
			model->m_PadRect.top = model->mWindowImageRect.top;

		if (model->m_PadRect.right > model->mWindowImageRect.right)
			model->m_PadRect.right = model->mWindowImageRect.right;

		if (model->m_PadRect.bottom > model->mWindowImageRect.bottom)
			model->m_PadRect.bottom = model->mWindowImageRect.bottom;
	}

	//Search Rect
	if (!((CRect)sInspFoot->m_rInspRect).IsRectNull())
	{
		model->mImageRect = sInspFoot->m_rInspRect;
		model->mImageRect.left += model->mWindowImageRect.left;
		model->mImageRect.top += model->mWindowImageRect.top;
		model->mImageRect.right += model->mWindowImageRect.left;
		model->mImageRect.bottom += model->mWindowImageRect.top;

		if (model->mImageRect.left < model->mWindowImageRect.left)
			model->mImageRect.left = model->mWindowImageRect.left;

		if (model->mImageRect.top < model->mWindowImageRect.top)
			model->mImageRect.top = model->mWindowImageRect.top;

		if (model->mImageRect.right > model->mWindowImageRect.right)
			model->mImageRect.right = model->mWindowImageRect.right;

		if (model->mImageRect.bottom > model->mWindowImageRect.bottom)
			model->mImageRect.bottom = model->mWindowImageRect.bottom;
	}

	//검사 Rect가 없으면 Window Rect
	if (((CRect)model->mImageRect).IsRectNull())
		model->mImageRect = model->mWindowImageRect;


	//윈도우 시작좌표 홀수일 경우 예외처리 NYJ 2021/02/16
//  	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeX % 2) != 0)		//odd
//  	{
//  		model->mImageRect.left += 1;
//  		model->mImageRect.right += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.left += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.right += 1;
//  	}
//  	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeY % 2) != 0)		//odd
//  	{
//  		model->mImageRect.top += 1;
//  		model->mImageRect.bottom += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.top += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.bottom += 1;
//  	}

	//Clip 
	cv::Rect ImageRect(model->mImageRect.left, model->mImageRect.top,
		model->mImageRect.right - model->mImageRect.left, model->mImageRect.bottom - model->mImageRect.top);

	int nWidth = ImageRect.width;;
	int nHeight = ImageRect.height;

	model->m_BondingRect.left = 0;
	model->m_BondingRect.top = 0;
	model->m_BondingRect.right = nWidth;
	model->m_BondingRect.bottom = nHeight;

	int _3DCX = ImageRect.x + (nWidth / 2.) - ptrAlgoColorOpt.m_rcImageRect.left;;
	int _3DCY = ImageRect.y + (nHeight / 2.) - ptrAlgoColorOpt.m_rcImageRect.top;

	model->m_ImageBuffer->nImageSizeX = nWidth;
	model->m_ImageBuffer->nImageSizeY = nHeight;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->pf3D, sizeof(float) * nWidth * nHeight);
	if (0 != m_pProcMilAlgo->GetCropZmap(sWndAlgoImg.m_fArr3D, model->pf3D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _3DCX, _3DCY, nWidth, nHeight))
	{
		//delete model;
		g_pMManager->pem_delete(model, false);
		return nullptr;
	}
	for (int r = 0; r < nHeight; r++)
	{
		for (int c = 0; c < nWidth; c++)
			if (isnan(model->pf3D[r*nWidth + c]) == true)
				model->pf3D[r*nWidth + c] = 0;
	}

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->pf3D_OrgROI, sizeof(float) * nWidth * nHeight);

	for (int r = 0; r < nHeight; r++)
	{
		for (int c = 0; c < nWidth; c++)
			model->pf3D_OrgROI[r*nWidth + c] = model->pf3D[r*nWidth + c];
	}

	model->pf3DOrg = sWndAlgoImg.m_fArr3D;

	//TempBuf
	UCHAR* imgTop_R = NULL;
	UCHAR* imgTop_B = NULL;
	UCHAR* imgTop_W = NULL;
	UCHAR* imgTop_G = NULL;
	UCHAR* imgMiddle_R = NULL;
	UCHAR* imgMiddle_B = NULL;
	UCHAR* imgBottom_R = NULL;
	UCHAR* imgBottom_B = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_R, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_B, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_W, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_G, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgMiddle_R, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgMiddle_B, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgBottom_R, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgBottom_B, sizeof(UCHAR) * wndWidth * wndHeight);

	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_R, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_B, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_W, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_W, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_G, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_G, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgMiddle_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgMiddle_R, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgMiddle_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgMiddle_B, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgBottom_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgBottom_R, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgBottom_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgBottom_B, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);

	//InspBuf
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgTop_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgTop_B, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgTop_W, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgTop_G, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgMiddle_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgMiddle_B, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgBottom_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model->m_ImageBuffer->imgBottom_B, sizeof(UCHAR) * nWidth * nHeight);

	m_pProcMilAlgo->GetClipImage_LT(imgTop_R, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_R, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgTop_B, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_B, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgTop_W, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_W, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgTop_G, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_G, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgMiddle_R, wndWidth, wndHeight, model->m_ImageBuffer->imgMiddle_R, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgMiddle_B, wndWidth, wndHeight, model->m_ImageBuffer->imgMiddle_B, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgBottom_R, wndWidth, wndHeight, model->m_ImageBuffer->imgBottom_R, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgBottom_B, wndWidth, wndHeight, model->m_ImageBuffer->imgBottom_B, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);


	cv::Mat cvFootTRImg;
	cv::Mat TRImg(nHeight, nWidth, CV_8UC1, model->m_ImageBuffer->imgTop_R);
	cvFootTRImg = TRImg.clone();

	cv::Mat cvFootTGImg(nHeight, nWidth, CV_8UC1, model->m_ImageBuffer->imgTop_G);

	#if _DEBUG
	cv::imwrite("D:\\FootRst\\0_Foot_TopR.bmp", cvFootTRImg);
	cv::imwrite("D:\\FootRst\\0_Foot_TopG.bmp", cvFootTGImg);
	#endif

	cv::Mat cvTopWhiteImg;
	cv::Mat TWImg(nHeight, nWidth, CV_8UC1, model->m_ImageBuffer->imgTop_W);
	cvTopWhiteImg = TWImg.clone();

	Delete_1DArray(&imgTop_R);
	Delete_1DArray(&imgTop_B);
	Delete_1DArray(&imgTop_W);
	Delete_1DArray(&imgTop_G);
	Delete_1DArray(&imgMiddle_R);
	Delete_1DArray(&imgMiddle_B);
	Delete_1DArray(&imgBottom_R);
	Delete_1DArray(&imgBottom_B);

	memcpy(&model->m_sArrBin, &sInspFoot->m_sArrBin, sizeof(AlgoBin) * m_eFootBin_Total); //Binary
	model->m_FootCenterPoint = sInspFoot->m_sPoint; //Foot Center


	model->m_FootCenterPoint_ImageRect.x = 0;
	model->m_FootCenterPoint_ImageRect.y = 0;
	if (model->m_FootCenterPoint.x > 0 || model->m_FootCenterPoint.y > 0)
	{
		model->m_FootCenterPoint_ImageRect.x = model->m_FootCenterPoint.x - (model->mImageRect.left - model->mWindowImageRect.left);
		model->m_FootCenterPoint_ImageRect.y = model->m_FootCenterPoint.y - (model->mImageRect.top - model->mWindowImageRect.top);
	}

	int n = m_eFoot_Total * eMMD_Total;
	memcpy(&model->m_fArrOptionValue, &sInspFoot->m_fArrOptionValue, sizeof(float) *m_eFoot_Total* eMMD_Total); //검사 옵션

	if (cvDBCPolygonImg->rows != 0 && cvDBCPolygonImg->cols != 0)
	{
		//dbc 이미지 저장
		model->m_pDBCPolygonImg = g_pMManager->pem_new<uchar>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
		memset(model->m_pDBCPolygonImg, 0, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

		//검출된 Pad 2D 정보 저장
		for (int y = 0; y < sWndAlgoImg.m_nHeight; y++)
		{
			UCHAR* pRaw = (*cvDBCPolygonImg).data;

			for (int x = 0; x < sWndAlgoImg.m_nWidth; x++)
			{
				if (pRaw[y * sWndAlgoImg.m_nWidth + x] == 255)
				{
					model->m_pDBCPolygonImg[y* sWndAlgoImg.m_nWidth + x] = 255;
				}
				else
				{
					model->m_pDBCPolygonImg[y* sWndAlgoImg.m_nWidth + x] = 0;
				}
			}
		}

	}
	
	if (bCorrect3D)
	{
		cv::Mat PadMask(nHeight, nWidth, CV_8UC1);
		model->GetImageMatrix(&PadMask, (int)m_eFootBin_Pad);

		//이진화 Blob
		int nPadBlobCnt = GetBinImage(model, &PadMask, (int)m_eFootBin::m_eFootBin_Pad, NULL, false);

		//model->m_pPad = new UCHAR[nWidth* nHeight];
		model->m_pPad = g_pMManager->pem_new<uchar>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);


		memset(model->m_pPad, 0, nWidth* nHeight);

		bool bUseAutoTeaching = bPadAreaAutoTeach;

		//if ((model->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		bool bIsDBCFoot = false;

		if (sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
			bIsDBCFoot = true;

		float fPad3DAvgHeigt = 0.0f;

		//Pad 자동티칭 시작
		if (bUseAutoTeaching)		//Pad 영역 BW - Blob Count 와는 상관없이 자동티칭 진행
		{
			int wid = nWidth;
			int len = nHeight;

			//1. Foot 3D 가우시안 필터링
			cv::Mat cvOrgFoot3D(len, wid, CV_32FC1, model->pf3D);
			cv::Mat cvTmpFoot3D = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

			
			if (bIsDBCFoot)
				cv::GaussianBlur(cvOrgFoot3D, cvTmpFoot3D, cv::Size(11, 11), 0);
			else
				cvTmpFoot3D = cvOrgFoot3D.clone();

			//2. Foot 3D 기울기 X,Y 계산
			cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
			cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

			size_t floatstep_3DCorrect = cvTmpFoot3D.step / sizeof(float);
			for (int y = 1; y < len - 1; y++)
			{
				float * pXgrad = (float *)Xgrad.data;
				float * pYgrad = (float *)Ygrad.data;
				float *rawData = (float*)cvTmpFoot3D.data;

				for (int x = 1; x < wid - 1; x++)
				{
					pXgrad[y*floatstep_3DCorrect + x] = rawData[y * floatstep_3DCorrect + (x + 1)] - rawData[ y * floatstep_3DCorrect + (x - 1)];
					pYgrad[y*floatstep_3DCorrect + x] = rawData[(y + 1) * floatstep_3DCorrect + x] - rawData[(y - 1) * floatstep_3DCorrect + x];
				}
			}

			//3. 2D - Top White 조명에서 grayLevel이 180 이상인 영역 & 3D - 가우시안 필터 적용한 3D 중 기울기 10 이하인 영역만 추출
			cv::Mat tempImage(len, wid, CV_8UC1);
			tempImage.setTo(0);

			cv::Mat Rst3DImage = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
			float fMaxHeight = 0;
			float fMinHeight = 10000;

			fPad3DAvgHeigt = 0.0f;
			float fRefAreaHgt = 0.0f;
			int nRefAreaCnt = 0;

			cv::Mat DBCBinImage(len, wid, CV_8UC1);
			DBCBinImage.setTo(0);
			if (bIsDBCFoot)
			{
				bool bIsFindDBCOutLine = FindDBCOutLines(model, &DBCBinImage, 80);

				///////////////////////
				//if (bIsFindDBCOutLine)
				//	PadMaskCalc = PadMaskCalc + DBCBinImage;
			}

			for (int i = 0; i < len; i++)
			{
				float * pXgrad = (float*)Xgrad.data;
				float * pYgrad = (float*)Ygrad.data;

				UCHAR* pTopWImg = cvTopWhiteImg.data;	//Top White 밝기 180 cvTopWhiteImg
				UCHAR* pTopRImg = cvFootTRImg.data;		//Top Red 밝기
				UCHAR* pResult2D = tempImage.data;
				float* rowptr = (float*)cvTmpFoot3D.data;
				float* ptrRst3D = (float*)Rst3DImage.data;

				for (int j = 0; j < wid; j++)
				{
					float val = rowptr[i * floatstep_3DCorrect + j];

					if (bIsDBCFoot)
					{
						if (pTopWImg[i * cvTopWhiteImg.step + j] > 220 &&
							( (int)(fabs(pYgrad[i * floatstep_3DCorrect + j]))< 10 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 10) )
						{
							if (isnan(val) == true)
								continue;

							if (rowptr[i * floatstep_3DCorrect + j] > 100 || rowptr[i * floatstep_3DCorrect + j] < -100)
								continue;

							ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
							pResult2D[i * tempImage.step + j] = 255;		//2D

							if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
								fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
							if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
								fMinHeight = rowptr[i * floatstep_3DCorrect + j];

						}
					}
					else
					{
						if (pTopRImg[i * cvTopWhiteImg.step + j] > 100 &&
							((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 5 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 5))
						{
							if (isnan(val) == true)
								continue;

							ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
							pResult2D[i * tempImage.step + j] = 255;		//2D

							if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
								fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
							if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
								fMinHeight = rowptr[i * floatstep_3DCorrect + j];

						}
					}
				}
					
			}
			cv::Mat grayImg = Rst3DImage.clone();

			//Blob Filtering 수행
			double dArea = 0, dCX = 0, dCY = 0;
			CRect rcBlob{ 0,0,0,0 };
			int nMinBlob = 1000;
			cv::Mat BinImageTmp(tempImage.rows, tempImage.cols, CV_8UC1);
			int nCntFBlob = m_pProcMilAlgo->CalcBlob_Select(tempImage.data, BinImageTmp.data, tempImage.cols, tempImage.rows, nMinBlob, FALSE, FALSE, 0, eSelectMix);
			if (nCntFBlob > 0)
			{
				fMaxHeight = 0;
				fMinHeight = 10000;

				for (int i = 0; i < len; i++)
				{
					UCHAR* pResult2D = BinImageTmp.data;
					float* ptrRst3D = (float*)Rst3DImage.data;

					for (int j = 0; j < wid; j++)
					{
						float val = ptrRst3D[i * floatstep_3DCorrect + j];

						if (pResult2D[i * tempImage.step + j] == 0)
						{
							ptrRst3D[i * floatstep_3DCorrect + j] = 0;	//3D
							//pResult2D[i * tempImage.step + j] = 255;		//2D

						}
						else
						{
							if (fMaxHeight < ptrRst3D[i * floatstep_3DCorrect + j])
								fMaxHeight = ptrRst3D[i * floatstep_3DCorrect + j];
							if (fMinHeight > ptrRst3D[i * floatstep_3DCorrect + j])
								fMinHeight = ptrRst3D[i * floatstep_3DCorrect + j];
						}

					}
				}
				grayImg = Rst3DImage.clone();
			}

			//4. 위의 두 조건을 만족하는 영역 중에서도, 최빈도 높이 값의 ±20 범위에 드는 부분을 최종 Background 로 추출
			int nTolerance = 50;
			float fMaxFeqHeight = GetHeightMaxHist(grayImg.ptr<float>(), wid, len, fMaxHeight, fMinHeight, bIsDBCFoot);

			if ((fMaxHeight - (model->nRemoveWireHeight - 30) < fMaxFeqHeight) && !(fMaxHeight - fMaxFeqHeight < 30) && !bIsDBCFoot)
			{
				tempImage.setTo(0);

				cv::Mat Rst3DImage_ReCalc = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

				//Pad 영역의 최빈도높이 > 3D최고높이-와이어제한높이 일경우, 최빈도 높이 재계산
				for (int i = 0; i < len; i++)
				{
					float * pXgrad = (float*)Xgrad.data;
					float * pYgrad = (float*)Ygrad.data;

					UCHAR* pTopWImg = cvTopWhiteImg.data;	//Top White 밝기 180 cvTopWhiteImg
					UCHAR* pTopRImg = cvFootTRImg.data;		//Top Red 밝기
					UCHAR* pResult2D = tempImage.data;
					float* rowptr = (float*)cvTmpFoot3D.data;
					float* ptrRst3D = (float*)Rst3DImage_ReCalc.data;

					for (int j = 0; j < wid; j++)
					{
						float val = rowptr[i * floatstep_3DCorrect + j];

						if (bIsDBCFoot)
						{
							if (pTopWImg[i * cvTopWhiteImg.step + j] > 220 &&
								((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 10 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 10))
							{
								if (isnan(val) == true)
									continue;

								if (rowptr[i * floatstep_3DCorrect + j] > 100 || rowptr[i * floatstep_3DCorrect + j] < -100)
									continue;

								ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
								pResult2D[i * tempImage.step + j] = 255;		//2D

								if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
									fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
								if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
									fMinHeight = rowptr[i * floatstep_3DCorrect + j];

							}
						}
						else
						{
							if (pTopRImg[i * cvTopWhiteImg.step + j] > 100 &&
								((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 5 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 5) &&
								rowptr[i * floatstep_3DCorrect + j] < fMaxFeqHeight-10)
							{
								if (isnan(val) == true)
									continue;

								ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
								pResult2D[i * tempImage.step + j] = 255;		//2D

								if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
									fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
								if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
									fMinHeight = rowptr[i * floatstep_3DCorrect + j];

							}
						}
					}

				}
				////

				cv::Mat grayImg = Rst3DImage_ReCalc.clone();

				float fMaxFeqH = 0;

				fMaxFeqH = GetHeightMaxHist(grayImg.ptr<float>(), wid, len, fMaxHeight, fMinHeight, bIsDBCFoot);

				if (fMaxFeqH != 0 && fMaxFeqH > 0 && fMaxFeqHeight - fMaxFeqH > 120)
				{
					fMaxFeqHeight = fMaxFeqH;
				}


			}

			

			cv::Mat cvAutoInsp_pad2D(cvTmpFoot3D.rows, cvTmpFoot3D.cols, CV_8UC1);
			cvAutoInsp_pad2D.setTo(0);

			std::vector< jsl::Point3d<double, double, double> > dataList;
			dataList.reserve(wid*len);
			jsl::LeastSquare lsCalc;

			for (int i = 0; i < len; i++)
			{
				UCHAR* pRstPad2DImg = cvAutoInsp_pad2D.data;
				float* rowptr = (float*)cvOrgFoot3D.data;

				for (int j = 0; j < wid; j++)
				{
					int val = rowptr[i * floatstep_3DCorrect + j];

					if (fMaxFeqHeight - nTolerance < val && val < fMaxFeqHeight + nTolerance)
					{
						pRstPad2DImg[i * cvAutoInsp_pad2D.step + j] = 255;
						jsl::Point3d<double, double, double> val2(j, i, rowptr[i * floatstep_3DCorrect + j] * 0.001);
						dataList.emplace_back(val2);
					}
				}
			}
// 
			cv::Mat cvRst_pad2D(cvTmpFoot3D.rows, cvTmpFoot3D.cols, CV_8UC1);
			cvRst_pad2D.setTo(0);
			//검출된 Pad 2D 정보 저장
			for (int y = 0; y < len; y++)
			{
				UCHAR* pRaw = cvRst_pad2D.data;
				float* rowptr = (float*)cvOrgFoot3D.data;

				float * pXgrad = (float*)Xgrad.data;
				float * pYgrad = (float*)Ygrad.data;
				UCHAR* pTopWImg = cvTopWhiteImg.data;	//Top White 밝기 
				UCHAR* pTopRImg = cvFootTRImg.data;	//Top Red 밝기
				UCHAR* pDBCSilkLines = DBCBinImage.data;

				for (int x = 0; x < wid; x++)
				{
					float fHgtVal = rowptr[y * floatstep_3DCorrect + x];

					if (!bIsDBCFoot)
					{
						if (pTopRImg[y * cvTopWhiteImg.step + x] > 160 &&
							((int)(fabs(pYgrad[y * floatstep_3DCorrect + x])) < 5 && (int)(fabs(pXgrad[y * floatstep_3DCorrect + x])) < 5 && (fMaxFeqHeight + 100 > fHgtVal)))		//2D랑 3D 기울기를 같이 봐야함
						{
							model->m_pPad[y*nWidth + x] = 255;
							pRaw[y*nWidth + x] = 255;
						}
						else
						{
							pRaw[y*nWidth + x] = 0;
						}
					}
					else
					{
						if (pTopWImg[y * cvTopWhiteImg.step + x] > 220 &&
							((int)(fabs(pYgrad[y * floatstep_3DCorrect + x])) < 5 && (int)(fabs(pXgrad[y * floatstep_3DCorrect + x])) < 5) && (fMaxFeqHeight + 100 > fHgtVal) && fHgtVal > -100)		//2D랑 3D 기울기를 같이 봐야함
						{
							model->m_pPad[y*nWidth + x] = 255;
							pRaw[y*nWidth + x] = 255;
						}
						else
						{
							pRaw[y*nWidth + x] = 0;
						}
					}

					if (pRaw[y*nWidth + x] == 255)
					{
						nRefAreaCnt++;
						fRefAreaHgt = fRefAreaHgt + fHgtVal;
					}

				}
			}

#if _DEBUG
			cv::imwrite("D:\\FootRst\\Pad_Correct3DArea_auto.bmp", cvRst_pad2D);
#endif

			if (nRefAreaCnt != 0)
				fPad3DAvgHeigt = fRefAreaHgt / nRefAreaCnt;	//Pad 영역 평균 3D 높이
			else
				fPad3DAvgHeigt = 0;

			if (_isnan(fPad3DAvgHeigt) == true)
				fPad3DAvgHeigt = 0;
			
 			model->m_fPadArea3DAvgHeight = fPad3DAvgHeigt;

			CString msg;
			msg.Format(_T("[FootAlgo]::MakeFoot_Pad3DAvgHeight : %f"), fPad3DAvgHeigt);
			g_pMPTI->AddLog(msg);

			std::vector< jsl::Point3d<double, double, double> > dataList2;

			int nStep(1);
			if (dataList.size() > 60000)
			{
				nStep = dataList.size() / 60000;
			}

			//if (dataList.size() < 20)
			//	return;

			dataList2.reserve(dataList.size());
			for (int n = 0; n < dataList.size(); n += nStep)
			{
				dataList2.emplace_back(dataList[n]);
			}

			//1차 평면 방정식으로 a,b,c 계수 3개(계수 a,b,c는 이 평면의 방향을 나타내는 법선 벡터)를 뽑아 원래3D 높이에서 빼줌

			std::vector<double> coeff;
			lsCalc.Run2d(1, dataList2, coeff);

			if (coeff.size() != 0)
			{

				double * pCoeff = &coeff[0];
				for (int y = 0; y < len; y++)
				{
					for (int x = 0; x < wid; x++)
					{
						//	height[y*wid + x] -= lsCalc.Apply2d(x, y, coeff) * 1000.0;
						model->pf3D[y*wid + x] -= (pCoeff[0] * (double)+(x)+pCoeff[1] * (double)(y)+pCoeff[2]) * 1000.0;
					}
				}
			}
			//검출된 Pad 3D 정보 저장
// 			for (int y = 0; y < len; y++)
// 			{
// 				for (int x = 0; x < wid; x++)
// 				{
// 					model->pf3D[y*wid + x] -= fPad3DAvgHeigt;
// 				}
// 			}

			//dbc실크라인 부분 3D 를 평균 높이로 변경하기
			if (bIsDBCFoot) {
				for (int y = 0; y < len; y++)
				{
					UCHAR* pDBCSilkLines = DBCBinImage.data;

					for (int x = 0; x < wid; x++)
					{
						if (pDBCSilkLines[y * DBCBinImage.step + x] == 255)
							model->pf3D[y*wid + x] = -50;

					}
				}
			}
			//background - Auto teaching 관련 구문 종료

			//calc Foot 3D Filter Image
			cv::Mat cvimg(0, 0, CV_32FC1);
			bool bCalcFoot3DFilterImg = CalcFoot3DAvgFilterImage(model, nWidth, nHeight, cvAutoInsp_pad2D, &cvimg);
			cvimg.copyTo(*cv3DAvgFilter);
		}
		////////////////////////////////////////////
		else
		{
			if (nPadBlobCnt > 0)
			{
				//Pad 이진화로 보정하자.
				Correct3d_Binary(PadMask, (sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), model->pf3D, nWidth, nHeight, fPad3DAvgHeigt);

				model->m_fPadArea3DAvgHeight = fPad3DAvgHeigt;

				CString msg;
				msg.Format(_T("[FootAlgo]::MakeFoot_Pad3DAvgHeight(manual): %f"), fPad3DAvgHeigt);
				g_pMPTI->AddLog(msg);

				//Correct3d((sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), model->pf3D, nWidth, nHeight, model->m_ImageBuffer->imgTop_R, model->m_ImageBuffer->imgTop_B, model->m_ImageBuffer->imgTop_W);
				m_pProcMilAlgo->SaveWorkImg_float(model->pf3D, nWidth, nHeight, _T("Foot_3D_R2.bmp"));

				//검출된 Pad 정보를 저장..
				for (size_t y = 0; y < nHeight; y++)
				{
					UCHAR* pRaw = PadMask.ptr<UCHAR>(y);
					for (size_t x = 0; x < nWidth; x++)
						model->m_pPad[y*nWidth + x] = pRaw[x];
				}

				//Pad 영역만 검출
				cv::Mat cvOnlyPadBW = CalcOnlyPadArea(model, nWidth, nHeight, (model->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC);
				
				//calc Foot 3D Filter Image
				cv::Mat cvimg(0, 0, CV_32FC1);
				bool bCalcFoot3DFilterImg = CalcFoot3DAvgFilterImage(model, nWidth, nHeight, cvOnlyPadBW, &cvimg);
				cvimg.copyTo(*cv3DAvgFilter);

			}
		}

	}

	model->bUseTeachRect = sInspFoot->bUseTeachRect;
	model->nPadWidth = sInspFoot->nPadWidth;
	model->nPadHeight = sInspFoot->nPadHeight;
	model->nRadius = sInspFoot->nRadius;

	model->bUseRemovedPadArea = sInspFoot->bUseRemovedPadArea;

	model->bUse2DImageForInspWidth = sInspFoot->bUse2DImageForInspWidth;

	model->nRemoveWireHeight = sInspFoot->nRemoveWireHeight;
	model->nFootPadSizeX = sInspFoot->nFootPadSizeX;
	model->nFootPadSizeY = sInspFoot->nFootPadSizeY;

	model->nPadEdgeMinRatio = sInspFoot->nPadEdgeMinRatio;
	model->bUseDBCShapeTeaching = sInspFoot->bUseDBCShapeTeaching;

	model->nInspCrackHeight = sInspFoot->nInspCrackHeight;

	return model;
}
*/


CFoot_Model* CPInsp_Foot::MakeFoot(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D, bool bPadAreaAutoTeach)
{
	//CFoot_Model* model = new CFoot_Model();
	CFoot_Model* model = g_pMManager->pem_new<CFoot_Model>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	model->m_nWindowStartX = nStartX;
	model->m_nWindowStartY = nStartY;

	model->_WidthOrg = sWndAlgoImg.m_nWidth3D;
	model->_HeightOrg = sWndAlgoImg.m_nHeight3D;

	//Inspect Option
	model->m_nFoottype = sInspFoot->m_nFootType;
	model->m_nUseOption = sInspFoot->m_nUseOption;
	model->m_nUseOption2 = sInspFoot->m_nUseOption2;
	model->m_nFindOption = sInspFoot->m_nFindOption;
	model->m_nFindOption2 = sInspFoot->m_nFindOption2;
	model->m_bUsePatternAngle = sInspFoot->m_bUsePatternAngle;
	model->m_nFootAngle = sInspFoot->m_nFootAngle;
	model->m_dTeachFootAngle = sInspFoot->m_dTeachFootAngle;
	model->m_bUse2Foot = sInspFoot->m_bUse2Foot;
	model->m_bUsePadAreaAutoTeach = sInspFoot->m_bUsePadAreaAutoTeach;
	model->bUse2DImageForInspWidth = sInspFoot->bUse2DImageForInspWidth;

	model->nRemoveWireHeight = sInspFoot->nRemoveWireHeight;

	model->nFootPadSizeX = sInspFoot->nFootPadSizeX;
	model->nFootPadSizeY = sInspFoot->nFootPadSizeY;

	model->nPadEdgeMinRatio = sInspFoot->nPadEdgeMinRatio;

	model->bUseDBCShapeTeaching = sInspFoot->bUseDBCShapeTeaching;

	model->nInspCrackHeight = sInspFoot->nInspCrackHeight;

	CPInsp_WireBonding* PInspWireBonding = g_pInspMng->GetWireBondingInsp();
	if (sInspFoot->nFootSobmin == 0)//설정되어 있지 않으면 머신데이터로가져오기
		model->nFootSobmin = PInspWireBonding->GetFootSobelMin();
	else
		model->nFootSobmin = sInspFoot->nFootSobmin;
	if (sInspFoot->nFootSobmax == 0)
		model->nFootSobmax = PInspWireBonding->GetFootSobelMax();
	else
		model->nFootSobmax = sInspFoot->nFootSobmax;

	model->m_nFindOptionOrder.clear();
	model->m_nFindOptionOrder2.clear();
	for (int i = 0; i < m_eFoot_FindOption::MeanSlope; i++)
		if (sInspFoot->m_nFindOptionOrder[i] != 0)
			model->m_nFindOptionOrder.push_back(sInspFoot->m_nFindOptionOrder[i]);
	for (int i = 0; i < m_eFoot_FindOption::total; i++)
		if (sInspFoot->m_nFindOptionOrder2[i] != 0)
			model->m_nFindOptionOrder2.push_back(sInspFoot->m_nFindOptionOrder2[i]);
	model->dAngle = sWndAlgoImg.dAngle;

	//윈도우 시작좌표 홀수일 경우 예외처리 NYJ 2021/02/16
	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeX % 2) != 0)		//odd
	{
		//model->mImageRect.left += 1;
		//model->mImageRect.right += 1;
		ptrAlgoColorOpt.m_rcImageRect.left += 1;
		ptrAlgoColorOpt.m_rcImageRect.right += 1;
	}
	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeY % 2) != 0)		//odd
	{
		//model->mImageRect.top += 1;
		//model->mImageRect.bottom += 1;
		ptrAlgoColorOpt.m_rcImageRect.top += 1;
		ptrAlgoColorOpt.m_rcImageRect.bottom += 1;
	}

	//Clip
	model->mWindowImageRect = ptrAlgoColorOpt.m_rcImageRect;
	int wndWidth = model->mWindowImageRect.right - model->mWindowImageRect.left;
	int wndHeight = model->mWindowImageRect.bottom - model->mWindowImageRect.top;

	if (model->mWindowImageRect.left + wndWidth > ptrAlgoColorOpt.m_sFovImg.nImageSizeX || model->mWindowImageRect.top + wndHeight > ptrAlgoColorOpt.m_sFovImg.nImageSizeY)
	{
		if (model->mWindowImageRect.left + wndWidth > ptrAlgoColorOpt.m_sFovImg.nImageSizeX)
		{
			int nShiftX = ((model->mWindowImageRect.left + wndWidth) - ptrAlgoColorOpt.m_sFovImg.nImageSizeX);
			model->mWindowImageRect.left = model->mWindowImageRect.left - nShiftX;
			model->mWindowImageRect.right = model->mWindowImageRect.right - nShiftX;
		}
		if (model->mWindowImageRect.top + wndHeight > ptrAlgoColorOpt.m_sFovImg.nImageSizeY)
		{
			int nShiftY = ((model->mWindowImageRect.top + wndHeight) - ptrAlgoColorOpt.m_sFovImg.nImageSizeY);
			model->mWindowImageRect.top = model->mWindowImageRect.top - nShiftY;
			model->mWindowImageRect.bottom = model->mWindowImageRect.bottom - nShiftY;
		}
	}

	//Ref Bottom Rect
	model->m_RefRect = sInspFoot->m_sRefArea;

	//Pad Rect
	//model->m_PadRect = sInspFoot->m_rSpecRect;	//NYJ 2020/02/05
	if (!((CRect)sInspFoot->m_rSpecRect).IsRectNull())
	{
		model->m_PadRect = sInspFoot->m_rSpecRect;	//NYJ 2020/02/05

		model->m_PadRect.left += model->mWindowImageRect.left;
		model->m_PadRect.top += model->mWindowImageRect.top;
		model->m_PadRect.right += model->mWindowImageRect.left;
		model->m_PadRect.bottom += model->mWindowImageRect.top;

		if (model->m_PadRect.left < model->mWindowImageRect.left)
			model->m_PadRect.left = model->mWindowImageRect.left;

		if (model->m_PadRect.top < model->mWindowImageRect.top)
			model->m_PadRect.top = model->mWindowImageRect.top;

		if (model->m_PadRect.right > model->mWindowImageRect.right)
			model->m_PadRect.right = model->mWindowImageRect.right;

		if (model->m_PadRect.bottom > model->mWindowImageRect.bottom)
			model->m_PadRect.bottom = model->mWindowImageRect.bottom;
	}

	//Search Rect
	if (!((CRect)sInspFoot->m_rInspRect).IsRectNull())
	{
		model->mImageRect = sInspFoot->m_rInspRect;
		model->mImageRect.left += model->mWindowImageRect.left;
		model->mImageRect.top += model->mWindowImageRect.top;
		model->mImageRect.right += model->mWindowImageRect.left;
		model->mImageRect.bottom += model->mWindowImageRect.top;

		if (model->mImageRect.left < model->mWindowImageRect.left)
			model->mImageRect.left = model->mWindowImageRect.left;

		if (model->mImageRect.top < model->mWindowImageRect.top)
			model->mImageRect.top = model->mWindowImageRect.top;

		if (model->mImageRect.right > model->mWindowImageRect.right)
			model->mImageRect.right = model->mWindowImageRect.right;

		if (model->mImageRect.bottom > model->mWindowImageRect.bottom)
			model->mImageRect.bottom = model->mWindowImageRect.bottom;
	}

	//검사 Rect가 없으면 Window Rect
	if (((CRect)model->mImageRect).IsRectNull())
		model->mImageRect = model->mWindowImageRect;


	//윈도우 시작좌표 홀수일 경우 예외처리 NYJ 2021/02/16
//  	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeX % 2) != 0)		//odd
//  	{
//  		model->mImageRect.left += 1;
//  		model->mImageRect.right += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.left += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.right += 1;
//  	}
//  	if ((ptrAlgoColorOpt.m_sFovImg.nImageSizeY % 2) != 0)		//odd
//  	{
//  		model->mImageRect.top += 1;
//  		model->mImageRect.bottom += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.top += 1;
//  		ptrAlgoColorOpt.m_rcImageRect.bottom += 1;
//  	}

	//Clip 
	cv::Rect ImageRect(model->mImageRect.left, model->mImageRect.top,
		model->mImageRect.right - model->mImageRect.left, model->mImageRect.bottom - model->mImageRect.top);

	int nWidth = ImageRect.width;;
	int nHeight = ImageRect.height;

	model->m_BondingRect.left = 0;
	model->m_BondingRect.top = 0;
	model->m_BondingRect.right = nWidth;
	model->m_BondingRect.bottom = nHeight;

	int _3DCX = ImageRect.x + (nWidth / 2.) - ptrAlgoColorOpt.m_rcImageRect.left;;
	int _3DCY = ImageRect.y + (nHeight / 2.) - ptrAlgoColorOpt.m_rcImageRect.top;

	model->m_ImageBuffer->nImageSizeX = nWidth;
	model->m_ImageBuffer->nImageSizeY = nHeight;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->pf3D, sizeof(float) * nWidth * nHeight);
	if (0 != m_pProcMilAlgo->GetCropZmap(sWndAlgoImg.m_fArr3D, model->pf3D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _3DCX, _3DCY, nWidth, nHeight))
	{
		//delete model;
		g_pMManager->pem_delete(model, false);
		return nullptr;
	}
	for (int r = 0; r < nHeight; r++)
	{
		for (int c = 0; c < nWidth; c++)
			if (isnan(model->pf3D[r*nWidth + c]) == true)
				model->pf3D[r*nWidth + c] = 0;
	}

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->pf3D_OrgROI, sizeof(float) * nWidth * nHeight);

	for (int r = 0; r < nHeight; r++)
	{
		for (int c = 0; c < nWidth; c++)
			model->pf3D_OrgROI[r*nWidth + c] = model->pf3D[r*nWidth + c];
	}

	model->pf3DOrg = sWndAlgoImg.m_fArr3D;



	//TempBuf
	UCHAR* imgTop_R = NULL;
	UCHAR* imgTop_B = NULL;
	UCHAR* imgTop_W = NULL;
	UCHAR* imgTop_G = NULL;
	UCHAR* imgMiddle_R = NULL;
	UCHAR* imgMiddle_B = NULL;
	UCHAR* imgBottom_R = NULL;
	UCHAR* imgBottom_B = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_R, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_B, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_W, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgTop_G, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgMiddle_R, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgMiddle_B, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgBottom_R, sizeof(UCHAR) * wndWidth * wndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &imgBottom_B, sizeof(UCHAR) * wndWidth * wndHeight);

	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_R, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_B, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_W, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_W, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_G, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgTop_G, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgMiddle_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgMiddle_R, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgMiddle_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgMiddle_B, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgBottom_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgBottom_R, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgBottom_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, imgBottom_B, model->mWindowImageRect.left, model->mWindowImageRect.top, wndWidth, wndHeight);

	//InspBuf
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_B, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_W, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_G, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgMiddle_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgMiddle_B, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgBottom_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgBottom_B, sizeof(UCHAR) * nWidth * nHeight);

	m_pProcMilAlgo->GetClipImage_LT(imgTop_R, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_R, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgTop_B, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_B, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgTop_W, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_W, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgTop_G, wndWidth, wndHeight, model->m_ImageBuffer->imgTop_G, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgMiddle_R, wndWidth, wndHeight, model->m_ImageBuffer->imgMiddle_R, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgMiddle_B, wndWidth, wndHeight, model->m_ImageBuffer->imgMiddle_B, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgBottom_R, wndWidth, wndHeight, model->m_ImageBuffer->imgBottom_R, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(imgBottom_B, wndWidth, wndHeight, model->m_ImageBuffer->imgBottom_B, sInspFoot->m_rInspRect.left, sInspFoot->m_rInspRect.top, nWidth, nHeight);


	cv::Mat cvFootTRImg;
	cv::Mat TRImg(nHeight, nWidth, CV_8UC1, model->m_ImageBuffer->imgTop_R);
	cvFootTRImg = TRImg.clone();

	cv::Mat cvFootTGImg(nHeight, nWidth, CV_8UC1, model->m_ImageBuffer->imgTop_G);

#if _DEBUG
	cv::imwrite("D:\\FootRst\\0_Foot_TopR.bmp", cvFootTRImg);
	cv::imwrite("D:\\FootRst\\0_Foot_TopG.bmp", cvFootTGImg);
#endif

	cv::Mat cvTopWhiteImg;
	cv::Mat TWImg(nHeight, nWidth, CV_8UC1, model->m_ImageBuffer->imgTop_W);
	cvTopWhiteImg = TWImg.clone();

	Delete_1DArray(&imgTop_R);
	Delete_1DArray(&imgTop_B);
	Delete_1DArray(&imgTop_W);
	Delete_1DArray(&imgTop_G);
	Delete_1DArray(&imgMiddle_R);
	Delete_1DArray(&imgMiddle_B);
	Delete_1DArray(&imgBottom_R);
	Delete_1DArray(&imgBottom_B);

	memcpy(&model->m_sArrBin, &sInspFoot->m_sArrBin, sizeof(AlgoBin) * m_eFootBin_Total); //Binary
	model->m_FootCenterPoint = sInspFoot->m_sPoint; //Foot Center


	model->m_FootCenterPoint_ImageRect.x = 0;
	model->m_FootCenterPoint_ImageRect.y = 0;
	if (model->m_FootCenterPoint.x > 0 || model->m_FootCenterPoint.y > 0)
	{
		model->m_FootCenterPoint_ImageRect.x = model->m_FootCenterPoint.x - (model->mImageRect.left - model->mWindowImageRect.left);
		model->m_FootCenterPoint_ImageRect.y = model->m_FootCenterPoint.y - (model->mImageRect.top - model->mWindowImageRect.top);
	}



	int n = m_eFoot_Total * eMMD_Total;
	memcpy(&model->m_fArrOptionValue, &sInspFoot->m_fArrOptionValue, sizeof(float) *m_eFoot_Total* eMMD_Total); //검사 옵션

	if (cvDBCPolygonImg->rows != 0 && cvDBCPolygonImg->cols != 0)
	{
		//dbc 이미지 저장
		model->m_pDBCPolygonImg = g_pMManager->pem_new<uchar>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
		memset(model->m_pDBCPolygonImg, 0, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

		//검출된 Pad 2D 정보 저장
		for (int y = 0; y < sWndAlgoImg.m_nHeight; y++)
		{
			UCHAR* pRaw = (*cvDBCPolygonImg).data;

			for (int x = 0; x < sWndAlgoImg.m_nWidth; x++)
			{
				if (pRaw[y * sWndAlgoImg.m_nWidth + x] == 255)
				{
					model->m_pDBCPolygonImg[y* sWndAlgoImg.m_nWidth + x] = 255;
				}
				else
				{
					model->m_pDBCPolygonImg[y* sWndAlgoImg.m_nWidth + x] = 0;
				}
			}
		}

	}

	if (bCorrect3D)
	{
		cv::Mat PadMask(nHeight, nWidth, CV_8UC1);

		model->GetImageMatrix(&PadMask, (int)m_eFootBin_Pad);
		//cv::Mat inputPadImage = PadMask.clone();  //Pad org 2D 영상 확인

		//cv::imwrite("D:\\FootRst\\bCorrect3D.bmp", PadMask);
		//이진화 Blob
		int nPadBlobCnt = GetBinImage(model, &PadMask, (int)m_eFootBin::m_eFootBin_Pad, NULL, false);

		//model->m_pPad = new UCHAR[nWidth* nHeight];
		model->m_pPad = g_pMManager->pem_new<uchar>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);


		memset(model->m_pPad, 0, nWidth* nHeight);

		bool bUseAutoTeaching = bPadAreaAutoTeach;

		//if ((model->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		bool bIsDBCFoot = false;

		if (sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
			bIsDBCFoot = true;

		float fPad3DAvgHeigt = 0.0f;

		//Pad 자동티칭 시작
		if (bUseAutoTeaching)		//Pad 영역 BW - Blob Count 와는 상관없이 자동티칭 진행
		{
			int wid = nWidth;
			int len = nHeight;

			//1. Foot 3D 가우시안 필터링
			cv::Mat cvOrgFoot3D(len, wid, CV_32FC1, model->pf3D);
			cv::Mat cvTmpFoot3D = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

			if (bIsDBCFoot)
				cv::GaussianBlur(cvOrgFoot3D, cvTmpFoot3D, cv::Size(11, 11), 0);
			else
				cvTmpFoot3D = cvOrgFoot3D.clone();

			//2. Foot 3D 기울기 X,Y 계산
			cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
			cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

			size_t floatstep_3DCorrect = cvTmpFoot3D.step / sizeof(float);
			for (int y = 1; y < len - 1; y++)
			{
				float * pXgrad = (float *)Xgrad.data;
				float * pYgrad = (float *)Ygrad.data;
				float *rawData = (float*)cvTmpFoot3D.data;

				for (int x = 1; x < wid - 1; x++)
				{
					pXgrad[y*floatstep_3DCorrect + x] = rawData[y * floatstep_3DCorrect + (x + 1)] - rawData[y * floatstep_3DCorrect + (x - 1)];
					pYgrad[y*floatstep_3DCorrect + x] = rawData[(y + 1) * floatstep_3DCorrect + x] - rawData[(y - 1) * floatstep_3DCorrect + x];
				}
			}

			//3. 2D - Top White 조명에서 grayLevel이 180 이상인 영역 & 3D - 가우시안 필터 적용한 3D 중 기울기 10 이하인 영역만 추출
			cv::Mat tempImage(len, wid, CV_8UC1);
			tempImage.setTo(0);

			cv::Mat Rst3DImage = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
			float fMaxHeight = 0;
			float fMinHeight = 10000;
			fPad3DAvgHeigt = 0.0f;
			float fRefAreaHgt = 0.0f;
			int nRefAreaCnt = 0;

			cv::Mat DBCBinImage(len, wid, CV_8UC1);
			DBCBinImage.setTo(0);
			if (bIsDBCFoot)
			{
				bool bIsFindDBCOutLine = FindDBCOutLines(model, &DBCBinImage, 80);
			}

			for (int i = 0; i < len; i++)
			{
				float * pXgrad = (float*)Xgrad.data;
				float * pYgrad = (float*)Ygrad.data;

				UCHAR* pTopWImg = cvTopWhiteImg.data;	//Top White 밝기 cvTopWhiteImg
				UCHAR* pTopRImg = cvFootTRImg.data;		//Top Red 밝기
				UCHAR* pResult2D = tempImage.data;
				float* rowptr = (float*)cvTmpFoot3D.data;
				float* ptrRst3D = (float*)Rst3DImage.data;

				for (int j = 0; j < wid; j++)
				{
					float val = rowptr[i * floatstep_3DCorrect + j];

					if (bIsDBCFoot)
					{
						if (pTopWImg[i * cvTopWhiteImg.step + j] > 220 &&
							((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 10 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 10))
						{
							if (isnan(val) == true)
								continue;

							if (rowptr[i * floatstep_3DCorrect + j] > 100 || rowptr[i * floatstep_3DCorrect + j] < -100)
								continue;

							ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
							pResult2D[i * tempImage.step + j] = 255;		//2D

							if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
								fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
							if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
								fMinHeight = rowptr[i * floatstep_3DCorrect + j];

						}
					}
					else
					{
						if (pTopRImg[i * cvTopWhiteImg.step + j] > 100 &&
							((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 5 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 5))
						{
							if (isnan(val) == true)
								continue;

							ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
							pResult2D[i * tempImage.step + j] = 255;		//2D

							if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
								fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
							if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
								fMinHeight = rowptr[i * floatstep_3DCorrect + j];

						}
					}
				}

			}
			cv::Mat grayImg = Rst3DImage.clone();

			//Blob Filtering 수행
			double dArea = 0, dCX = 0, dCY = 0;
			CRect rcBlob{ 0,0,0,0 };
			int nMinBlob = 1000;
			cv::Mat BinImageTmp(tempImage.rows, tempImage.cols, CV_8UC1);

			int nCntFBlob = m_pProcMilAlgo->CalcBlob_Select(tempImage.data, BinImageTmp.data, tempImage.cols, tempImage.rows, nMinBlob, FALSE, FALSE, 0, eSelectMix);

			//pad 바닥 후보군 데이터 쌓기


			if (nCntFBlob > 0)
			{
				fMaxHeight = 0;
				fMinHeight = 10000;

				for (int i = 0; i < len; i++)
				{
					UCHAR* pResult2D = BinImageTmp.data;
					float* ptrRst3D = (float*)Rst3DImage.data;


					for (int j = 0; j < wid; j++)
					{
						float val = ptrRst3D[i * floatstep_3DCorrect + j];

						if (pResult2D[i * tempImage.step + j] == 0)
						{
							ptrRst3D[i * floatstep_3DCorrect + j] = 0;	//3D
							//pResult2D[i * tempImage.step + j] = 255;		//2D

						}
						else
						{



							if (fMaxHeight < ptrRst3D[i * floatstep_3DCorrect + j])
								fMaxHeight = ptrRst3D[i * floatstep_3DCorrect + j];
							if (fMinHeight > ptrRst3D[i * floatstep_3DCorrect + j])
								fMinHeight = ptrRst3D[i * floatstep_3DCorrect + j];
						}

					}
				}
				grayImg = Rst3DImage.clone();
			}


			//4. 위의 두 조건을 만족하는 영역 중에서도, 최빈도 높이 값의 ±20 범위에 드는 부분을 최종 Background 로 추출
			int nTolerance = 50;
			float fMaxFeqHeight = GetHeightMaxHist(grayImg.ptr<float>(), wid, len, fMaxHeight, fMinHeight, bIsDBCFoot);

				if ((fMaxHeight - (model->nRemoveWireHeight - 30) < fMaxFeqHeight) && !(fMaxHeight - fMaxFeqHeight < 30) && !bIsDBCFoot)
				{
					tempImage.setTo(0);

					cv::Mat Rst3DImage_ReCalc = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

					//Pad 영역의 최빈도높이 > 3D최고높이-와이어제한높이 일경우, 최빈도 높이 재계산
					for (int i = 0; i < len; i++)
					{
					float * pXgrad = (float*)Xgrad.data;
					float * pYgrad = (float*)Ygrad.data;

						UCHAR* pTopWImg = cvTopWhiteImg.data;	//Top White 밝기 180 cvTopWhiteImg
						UCHAR* pTopRImg = cvFootTRImg.data;		//Top Red 밝기
						UCHAR* pResult2D = tempImage.data;
						float* rowptr = (float*)cvTmpFoot3D.data;
						float* ptrRst3D = (float*)Rst3DImage_ReCalc.data;

						for (int j = 0; j < wid; j++)
						{
						float val = rowptr[i * floatstep_3DCorrect + j];

							if (bIsDBCFoot)
							{
								if (pTopWImg[i * cvTopWhiteImg.step + j] > 220 &&
								((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 10 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 10))
								{
									if (isnan(val) == true)
										continue;

								if (rowptr[i * floatstep_3DCorrect + j] > 100 || rowptr[i * floatstep_3DCorrect + j] < -100)
										continue;

								ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
									pResult2D[i * tempImage.step + j] = 255;		//2D

								if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
									fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
								if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
									fMinHeight = rowptr[i * floatstep_3DCorrect + j];

								}
							}
							else
							{
								if (pTopRImg[i * cvTopWhiteImg.step + j] > 100 &&
								((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 5 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 5) &&
								rowptr[i * floatstep_3DCorrect + j] < fMaxFeqHeight-10)
								{
									if (isnan(val) == true)
										continue;

								ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
									pResult2D[i * tempImage.step + j] = 255;		//2D

								if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
									fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
								if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
									fMinHeight = rowptr[i * floatstep_3DCorrect + j];

								}
							}
						}

					}
				////

				cv::Mat grayImg = Rst3DImage_ReCalc.clone();

				float fMaxFeqH = 0;

				fMaxFeqH = GetHeightMaxHist(grayImg.ptr<float>(), wid, len, fMaxHeight, fMinHeight, bIsDBCFoot);

				if (fMaxFeqH != 0 && fMaxFeqH > 0 && fMaxFeqHeight - fMaxFeqH > 120)
				{
						fMaxFeqHeight = fMaxFeqH;
					}


				}


			cv::Mat cvAutoInsp_pad2D(cvTmpFoot3D.rows, cvTmpFoot3D.cols, CV_8UC1);
			cvAutoInsp_pad2D.setTo(0);

			std::vector< jsl::Point3d<double, double, double> > dataList;
			dataList.reserve(wid*len);
			jsl::LeastSquare lsCalc;

			for (int i = 0; i < len; i++)
			{
				UCHAR* pRstPad2DImg = cvAutoInsp_pad2D.data;
				float* rowptr = (float*)cvOrgFoot3D.data;

				for (int j = 0; j < wid; j++)
				{
					int val = rowptr[i * floatstep_3DCorrect + j];

					if (fMaxFeqHeight - nTolerance < val && val < fMaxFeqHeight + nTolerance)
					{
						pRstPad2DImg[i * cvAutoInsp_pad2D.step + j] = 255;

						jsl::Point3d<double, double, double> val2(j, i, rowptr[i * floatstep_3DCorrect + j] * 0.001);
						dataList.emplace_back(val2);

					}

				}
			}
// 
			cv::Mat cvRst_pad2D(cvTmpFoot3D.rows, cvTmpFoot3D.cols, CV_8UC1);
			cvRst_pad2D.setTo(0);
			//검출된 Pad 2D 정보 저장
			for (int y = 0; y < len; y++)
			{
				UCHAR* pRaw = cvRst_pad2D.data;
				float* rowptr = (float*)cvOrgFoot3D.data;

				float * pXgrad = (float*)Xgrad.data;
				float * pYgrad = (float*)Ygrad.data;
				UCHAR* pTopWImg = cvTopWhiteImg.data;	//Top White 밝기 
					UCHAR* pTopRImg = cvFootTRImg.data;	//Top Red 밝기
					UCHAR* pDBCSilkLines = DBCBinImage.data;

					for (int x = 0; x < wid; x++)
					{
					float fHgtVal = rowptr[y * floatstep_3DCorrect + x];

						if (!bIsDBCFoot)
						{
							if (pTopRImg[y * cvTopWhiteImg.step + x] > 160 &&
							((int)(fabs(pYgrad[y * floatstep_3DCorrect + x])) < 5 && (int)(fabs(pXgrad[y * floatstep_3DCorrect + x])) < 5 && (fMaxFeqHeight + 100 > fHgtVal)) /*&& !bIsDBCFoot*/)		//2D랑 3D 기울기를 같이 봐야함
							{
							model->m_pPad[y*nWidth + x] = 255;
							pRaw[y*nWidth + x] = 255;
							}
							else
							{
								pRaw[y*nWidth + x] = 0;
						}
					}
					else
						{
							if (pTopWImg[y * cvTopWhiteImg.step + x] > 220 &&
							((int)(fabs(pYgrad[y * floatstep_3DCorrect + x])) < 5 && (int)(fabs(pXgrad[y * floatstep_3DCorrect + x])) < 5) && (fMaxFeqHeight + 100 > fHgtVal) && fHgtVal > -100)		//2D랑 3D 기울기를 같이 봐야함
							{
								model->m_pPad[y*nWidth + x] = 255;
								pRaw[y*nWidth + x] = 255;
							}
							else
							{
								pRaw[y*nWidth + x] = 0;
							}
						}

						if (pRaw[y*nWidth + x] == 255)
						{
							nRefAreaCnt++;
						fRefAreaHgt = fRefAreaHgt + fHgtVal;
						}

					}
				}

#if _DEBUG
			cv::imwrite("D:\\FootRst\\Pad_Correct3DArea_auto.bmp", cvRst_pad2D);
#endif

				if (nRefAreaCnt != 0)
					fPad3DAvgHeigt = fRefAreaHgt / nRefAreaCnt;	//Pad 영역 평균 3D 높이
				else
					fPad3DAvgHeigt = 0;

				if (_isnan(fPad3DAvgHeigt) == true)
					fPad3DAvgHeigt = 0;

				model->m_fPadArea3DAvgHeight = fPad3DAvgHeigt;

				CString msg;
				msg.Format(_T("[FootAlgo]::MakeFoot_Pad3DAvgHeight : %f"), fPad3DAvgHeigt);
				g_pMPTI->AddLog(msg);

				std::vector< jsl::Point3d<double, double, double> > dataList2;
				int nStep(1);
			if (dataList.size() > 60000)
				{
				nStep = dataList.size() / 60000;
				}

			dataList2.reserve(dataList.size());
			for (int n = 0; n < dataList.size(); n += nStep)
				{
				dataList2.emplace_back(dataList[n]);
				}

				//1차 평면 방정식으로 a,b,c 계수 3개(계수 a,b,c는 이 평면의 방향을 나타내는 법선 벡터)를 뽑아 원래3D 높이에서 빼줌
				std::vector<double> coeff;
				if(dataList2.size() != 0 )
					lsCalc.Run2d(1, dataList2, coeff);
				if (coeff.size() != 0)
				{

					double * pCoeff = &coeff[0];
					for (int y = 0; y < len; y++)
					{
						for (int x = 0; x < wid; x++)
						{
							model->pf3D[y*wid + x] -= (pCoeff[0] * (double)+(x)+pCoeff[1] * (double)(y)+pCoeff[2]) * 1000.0;
						}
					}
				}

				//dbc실크라인 부분 3D 를 평균 높이로 변경하기
				if (bIsDBCFoot) {
					for (int y = 0; y < len; y++)
					{
						UCHAR* pDBCSilkLines = DBCBinImage.data;

						for (int x = 0; x < wid; x++)
						{
							if (pDBCSilkLines[y * DBCBinImage.step + x] == 255)
								model->pf3D[y*wid + x] = -50;

						}
					}
				}
			//background - Auto teaching 관련 구문 종료
			
			//calc Foot 3D Filter Image
			cv::Mat cvimg(0, 0, CV_32FC1);
			bool bCalcFoot3DFilterImg = CalcFoot3DAvgFilterImage(model, nWidth, nHeight, cvAutoInsp_pad2D, &cvimg);
			cvimg.copyTo(*cv3DAvgFilter);
		}
		////////////////////////////////////////////
		else
		{
			if (nPadBlobCnt > 0)
			{
				//Pad 이진화로 보정하자.
				Correct3d_Binary(PadMask, (sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), model->pf3D, nWidth, nHeight, fPad3DAvgHeigt);

				model->m_fPadArea3DAvgHeight = fPad3DAvgHeigt;

				CString msg;
				msg.Format(_T("[FootAlgo]::MakeFoot_Pad3DAvgHeight(manual): %f"), fPad3DAvgHeigt);
				g_pMPTI->AddLog(msg);

				//Correct3d((sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), model->pf3D, nWidth, nHeight, model->m_ImageBuffer->imgTop_R, model->m_ImageBuffer->imgTop_B, model->m_ImageBuffer->imgTop_W);
				m_pProcMilAlgo->SaveWorkImg_float(model->pf3D, nWidth, nHeight, _T("Foot_3D_R2.bmp"));

				//검출된 Pad 정보를 저장..
				for (size_t y = 0; y < nHeight; y++)
				{
					UCHAR* pRaw = PadMask.ptr<UCHAR>(y);
					for (size_t x = 0; x < nWidth; x++)
						model->m_pPad[y*nWidth + x] = pRaw[x];
				}

				//Pad 영역만 검출
				cv::Mat cvOnlyPadBW = CalcOnlyPadArea(model, nWidth, nHeight, (model->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC);

				//calc Foot 3D Filter Image
				cv::Mat cvimg(0, 0, CV_32FC1);
				bool bCalcFoot3DFilterImg = CalcFoot3DAvgFilterImage(model, nWidth, nHeight, cvOnlyPadBW, &cvimg);
				cvimg.copyTo(*cv3DAvgFilter);

			}
		}

	}

	model->bUseTeachRect = sInspFoot->bUseTeachRect;
	model->nPadWidth = sInspFoot->nPadWidth;
	model->nPadHeight = sInspFoot->nPadHeight;
	model->nRadius = sInspFoot->nRadius;

	model->bUseRemovedPadArea = sInspFoot->bUseRemovedPadArea;

	model->bUse2DImageForInspWidth = sInspFoot->bUse2DImageForInspWidth;

	model->nRemoveWireHeight = sInspFoot->nRemoveWireHeight;
	model->nFootPadSizeX = sInspFoot->nFootPadSizeX;
	model->nFootPadSizeY = sInspFoot->nFootPadSizeY;

	model->nPadEdgeMinRatio = sInspFoot->nPadEdgeMinRatio;
	model->bUseDBCShapeTeaching = sInspFoot->bUseDBCShapeTeaching;

	model->nInspCrackHeight = sInspFoot->nInspCrackHeight;

	return model;
}


float CPInsp_Foot::GetHeightMaxHist(float *pfArrImgData, int nImgWidth, int nImgHeight, float fMaxHgt, float fMinHgt, bool bIsDBCFoot)
{

	float fReturnRate = 0.0F;

	if (pfArrImgData == NULL)
		return fReturnRate;

	float fMaxH = 0.0F;
	float fMinH = 0.0F;
	IppiSize roiSize = { nImgWidth, nImgHeight };
	Ipp32f rst;
	//ippiMin_32f_C1R(pfArrImgData, nImgWidth * sizeof(float), roiSize, &rst);
	//fMinH = rst;
	fMinH = fMinHgt;

	//ippiMax_32f_C1R(pfArrImgData, nImgWidth * sizeof(float), roiSize, &rst);
	//fMaxH = rst;
	fMaxH = fMaxHgt;

	int diff = fMaxH - fMinH;
	if (diff < 0)
		return fReturnRate;

	std::vector<int> Hist;
	Hist.assign(diff + 1, 0.0f);

	cv::Mat cv3D(nImgHeight, nImgWidth, CV_32FC1, pfArrImgData);

	size_t floatstep_3DCorrect = cv3D.step / sizeof(float);
	for (int r = 0; r < nImgHeight; r++)
	{
		for (int c = 0; c < nImgWidth; c++)
		{
			float fHeight;

			if (isnan(pfArrImgData[r*floatstep_3DCorrect + c]) == true)
			{
				continue;
			}
			else if (/*!bIsDBCFoot && */pfArrImgData[r*floatstep_3DCorrect + c] == 0)
			{
				continue;
			}
			else
			{
				fHeight = pfArrImgData[r*floatstep_3DCorrect + c];
				if (fHeight < -5000)
					continue;

				if (fHeight - fMinH >= 0 && fHeight - fMinH < Hist.size())
					Hist[fHeight - fMinH]++;
			}
		}
	}

	int MaxVal = 0;
	float MaxHight = 0.0f;

#ifdef DEBUG
	FILE* fp_3DHistogram = fopen("D:\\FootInspImg\\_insp_3D_Histo.csv", "w+");
#endif // DEBUG
	for (int i = 0; i < Hist.size(); i++)
	{
#ifdef DEBUG
		fprintf(fp_3DHistogram, "%d,", Hist[i]);
#endif // DEBUG
		if (MaxVal < Hist[i])
		{
			MaxVal = Hist[i];
			MaxHight = i + fMinH;
		}

	}
#ifdef DEBUG
	fclose(fp_3DHistogram);
#endif // DEBUG

	fReturnRate = MaxHight;

	double dMIn = fReturnRate - 10;
	double dMax = fReturnRate + 10;

	double dSum = 0.0f;
	int nCnt(0);

	for (int r = 0; r < nImgHeight; r++)
	{
		for (int c = 0; c < nImgWidth; c++)
		{
			if (pfArrImgData[r*nImgWidth + c] > dMIn && pfArrImgData[r*nImgWidth + c] < dMax)
			{
				dSum += pfArrImgData[r*nImgWidth + c];
				nCnt++;
			}
		}
	}
	fReturnRate = dSum / nCnt;
	// 	CHistogramAnalysis_New ha;
	// 	cv::Mat hData(1, Hist.size(), CV_32SC1, &Hist[0]);
	// 	ha.Run(hData);


	return fReturnRate;
}


CFoot_Model* CPInsp_Foot::MakeRotateFoot(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, bool bCorrect3D)
{
	//CFoot_Model* model = new CFoot_Model();
	CFoot_Model* model = g_pMManager->pem_new<CFoot_Model>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	cv::RotatedRect SelectedRect;
	bool ret = AllImageClip(sInspFoot, sWndAlgoImg, ptrAlgoColorOpt, SelectedRect);
	model->m_RoRect = new cv::RotatedRect(SelectedRect);
	g_pMManager->pem_new_check(model->m_RoRect, (PCHAR)__FUNCTION__, __LINE__);


	if (!ret)
	{
		if (model)
		{
			//delete  model;
			g_pMManager->pem_delete(model, false);
			model = NULL;
		}

		cv::Mat cvimg(0, 0, CV_32FC1);
		cv::Mat cvimg_dbc(0, 0, CV_8UC1);
		return MakeFoot(sInspFoot, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, &cvimg, &cvimg_dbc, bCorrect3D);
	}

	model->m_nWindowStartX = nStartX;
	model->m_nWindowStartY = nStartY;

	model->_WidthOrg = sWndAlgoImg.m_nWidth3D;
	model->_HeightOrg = sWndAlgoImg.m_nHeight3D;

	//Inspect Option
	model->m_nFoottype = sInspFoot->m_nFootType;
	model->m_nUseOption = sInspFoot->m_nUseOption;
	model->m_nUseOption2 = sInspFoot->m_nUseOption2;
	model->m_nFindOption = sInspFoot->m_nFindOption;
	model->m_nFindOption2 = sInspFoot->m_nFindOption2;
	model->m_bUsePatternAngle = sInspFoot->m_bUsePatternAngle;
	model->m_nFootAngle = sInspFoot->m_nFootAngle;
	model->m_dTeachFootAngle = sInspFoot->m_dTeachFootAngle;
	model->m_bUse2Foot = sInspFoot->m_bUse2Foot;
	model->m_bUsePadAreaAutoTeach = sInspFoot->m_bUsePadAreaAutoTeach;
	model->bUse2DImageForInspWidth = sInspFoot->bUse2DImageForInspWidth;

	model->nRemoveWireHeight = sInspFoot->nRemoveWireHeight;
	model->nFootPadSizeX = sInspFoot->nFootPadSizeX;
	model->nFootPadSizeY = sInspFoot->nFootPadSizeY;

	model->nPadEdgeMinRatio = sInspFoot->nPadEdgeMinRatio;
	model->bUseDBCShapeTeaching = sInspFoot->bUseDBCShapeTeaching;

	model->nInspCrackHeight = sInspFoot->nInspCrackHeight;

	model->m_nFindOptionOrder.clear();
	model->m_nFindOptionOrder2.clear();
	for (int i = 0; i < m_eFoot_FindOption::MeanSlope; i++)
		if (sInspFoot->m_nFindOptionOrder[i] != 0)
			model->m_nFindOptionOrder.push_back(sInspFoot->m_nFindOptionOrder[i]);
	for (int i = 0; i < m_eFoot_FindOption::total; i++)
		if (sInspFoot->m_nFindOptionOrder2[i] != 0)
			model->m_nFindOptionOrder2.push_back(sInspFoot->m_nFindOptionOrder2[i]);
	model->dAngle = sWndAlgoImg.dAngle;

	//Clip
	model->mWindowImageRect = ptrAlgoColorOpt.m_rcImageRect;

	//Ref Bottom Rect
	model->m_RefRect = sInspFoot->m_sRefArea;

	//Pad Rect
	if (!((CRect)sInspFoot->m_rSpecRect).IsRectNull())
	{
		int wh = std::roundl(SelectedRect.size.width / 2);
		int hh = std::roundl(SelectedRect.size.height / 2);
		model->m_PadRect.left = SelectedRect.center.x - wh;
		model->m_PadRect.top = SelectedRect.center.y - hh;
		model->m_PadRect.right = SelectedRect.center.x + wh;
		model->m_PadRect.bottom = SelectedRect.center.y + hh;

		model->m_PadRect.left += model->mWindowImageRect.left;
		model->m_PadRect.top += model->mWindowImageRect.top;
		model->m_PadRect.right += model->mWindowImageRect.left;
		model->m_PadRect.bottom += model->mWindowImageRect.top;

		if (model->m_PadRect.left < model->mWindowImageRect.left)
			model->m_PadRect.left = model->mWindowImageRect.left;

		if (model->m_PadRect.top < model->mWindowImageRect.top)
			model->m_PadRect.top = model->mWindowImageRect.top;

		if (model->m_PadRect.right > model->mWindowImageRect.right)
			model->m_PadRect.right = model->mWindowImageRect.right;

		if (model->m_PadRect.bottom > model->mWindowImageRect.bottom)
			model->m_PadRect.bottom = model->mWindowImageRect.bottom;
	}

	//Search Rect
	if (!((CRect)sInspFoot->m_rInspRect).IsRectNull())
	{
		int modW = sInspFoot->m_rInspRect.right - sInspFoot->m_rInspRect.left;
		int modH = sInspFoot->m_rInspRect.bottom - sInspFoot->m_rInspRect.top;
		int padW = sInspFoot->m_rSpecRect.right - sInspFoot->m_rSpecRect.left;
		int padH = sInspFoot->m_rSpecRect.bottom - sInspFoot->m_rSpecRect.top;

		int margX = modW - padW;
		int margY = modH - padH;
		cv::RotatedRect RoImgRect;
		RoImgRect = SelectedRect;

		RoImgRect.size.width += margX;
		RoImgRect.size.height += margY;

		cv::Point2f ArrRe2[4];
		RoImgRect.points(ArrRe2);
		std::vector<cv::Point> vRectPo;
		vRectPo.push_back(cv::Point(ArrRe2[0].x, ArrRe2[0].y));
		vRectPo.push_back(cv::Point(ArrRe2[1].x, ArrRe2[1].y));
		vRectPo.push_back(cv::Point(ArrRe2[2].x, ArrRe2[2].y));
		vRectPo.push_back(cv::Point(ArrRe2[3].x, ArrRe2[3].y));
		cv::Rect roImageBoundingRect = cv::boundingRect(vRectPo);

		model->mImageRect.left = roImageBoundingRect.x;
		model->mImageRect.top = roImageBoundingRect.y;
		model->mImageRect.right = roImageBoundingRect.x + roImageBoundingRect.width;
		model->mImageRect.bottom = roImageBoundingRect.y + roImageBoundingRect.height;

		model->mImageRect.left += model->mWindowImageRect.left;
		model->mImageRect.top += model->mWindowImageRect.top;
		model->mImageRect.right += model->mWindowImageRect.left;
		model->mImageRect.bottom += model->mWindowImageRect.top;

		if (model->mImageRect.left < model->mWindowImageRect.left)
			model->mImageRect.left = model->mWindowImageRect.left;

		if (model->mImageRect.top < model->mWindowImageRect.top)
			model->mImageRect.top = model->mWindowImageRect.top;

		if (model->mImageRect.right > model->mWindowImageRect.right)
			model->mImageRect.right = model->mWindowImageRect.right;

		if (model->mImageRect.bottom > model->mWindowImageRect.bottom)
			model->mImageRect.bottom = model->mWindowImageRect.bottom;
	}

	//검사 Rect가 없으면 Window Rect
	if (((CRect)model->mImageRect).IsRectNull())
		model->mImageRect = model->mWindowImageRect;

	//Clip 
	cv::Rect ImageRect(model->mImageRect.left, model->mImageRect.top,
		model->mImageRect.right - model->mImageRect.left, model->mImageRect.bottom - model->mImageRect.top);

	int nWidth = ImageRect.width;;
	int nHeight = ImageRect.height;

	int _3DCX = ImageRect.x + (nWidth / 2.) - ptrAlgoColorOpt.m_rcImageRect.left;;
	int _3DCY = ImageRect.y + (nHeight / 2.) - ptrAlgoColorOpt.m_rcImageRect.top;

	model->m_ImageBuffer->nImageSizeX = nWidth;
	model->m_ImageBuffer->nImageSizeY = nHeight;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->pf3D, sizeof(float) * nWidth * nHeight);
	if (0 != m_pProcMilAlgo->GetCropZmap(sWndAlgoImg.m_fArr3D, model->pf3D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _3DCX, _3DCY, nWidth, nHeight))
	{
		//delete model;
		g_pMManager->pem_delete(model, false);
		return nullptr;
	}

	model->pf3DOrg = sWndAlgoImg.m_fArr3D;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_B, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_W, sizeof(UCHAR) * nWidth * nHeight);

	//Get Matrix에서 사용
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgTop_G, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgMiddle_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgMiddle_B, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgBottom_R, sizeof(UCHAR) * nWidth * nHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &model->m_ImageBuffer->imgBottom_B, sizeof(UCHAR) * nWidth * nHeight);

	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_B, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_W, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_W, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgTop_R, nWidth, nHeight, _T("Foot_Clip_TR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgTop_B, nWidth, nHeight, _T("Foot_Clip_TB.bmp"));
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgTop_W, nWidth, nHeight, _T("Foot_Clip_TW.bmp"));

	CString path = g_pMPTI->GetWireDebugDataFullPath((int)m_eFootBin::m_eFootBin_Foot, _T("FootRst"), _T("Foot_Clip_TR"), 0);

#if _DEBUG
	m_pProcMilAlgo->SaveDebugImg(model->m_ImageBuffer->imgTop_R, nWidth, nHeight, path, 1, FALSE);
#endif

	m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt.m_sFovImg.imgTop_W, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, _T("TW.bmp"));
	m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt.m_sFovImg.imgBottom_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, _T("BR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt.m_sFovImg.imgMiddle_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, _T("MR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt.m_sFovImg.imgTop_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, _T("TR.bmp"));

	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_G, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgTop_G, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgMiddle_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgMiddle_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgMiddle_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgMiddle_B, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgBottom_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgBottom_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgBottom_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, model->m_ImageBuffer->imgBottom_B, ImageRect.x, ImageRect.y, nWidth, nHeight);
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgTop_G, nWidth, nHeight, _T("Foot_Clip_TG.bmp"));
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgMiddle_R, nWidth, nHeight, _T("Foot_Clip_MR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgMiddle_B, nWidth, nHeight, _T("Foot_Clip_MB.bmp"));
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgBottom_R, nWidth, nHeight, _T("Foot_Clip_BR.bmp"));
	m_pProcMilAlgo->SaveWorkImg(model->m_ImageBuffer->imgBottom_B, nWidth, nHeight, _T("Foot_Clip_BB.bmp"));

	memcpy(&model->m_sArrBin, &sInspFoot->m_sArrBin, sizeof(AlgoBin) * m_eFootBin_Total); //Binary
	model->m_FootCenterPoint = sInspFoot->m_sPoint; //Foot Center

	model->m_FootCenterPoint_ImageRect.x = 0;
	model->m_FootCenterPoint_ImageRect.y = 0;
	if (model->m_FootCenterPoint.x > 0 || model->m_FootCenterPoint.y > 0)
	{
		model->m_FootCenterPoint_ImageRect.x = model->m_FootCenterPoint.x - (model->mImageRect.left - model->mWindowImageRect.left);
		model->m_FootCenterPoint_ImageRect.y = model->m_FootCenterPoint.y - (model->mImageRect.top - model->mWindowImageRect.top);
	}

	int n = m_eFoot_Total * eMMD_Total;
	memcpy(&model->m_fArrOptionValue, &sInspFoot->m_fArrOptionValue, sizeof(float) *m_eFoot_Total* eMMD_Total); //검사 옵션


	if (bCorrect3D)
	{
		cv::Mat PadMask(nHeight, nWidth, CV_8UC1);

		//Pad 이진화
		model->GetImageMatrix(&PadMask, (int)m_eFootBin_Pad);

		//이진화 Blob
		int nPadBlobCnt = GetBinImage(model, &PadMask, (int)m_eFootBin::m_eFootBin_Pad, NULL, false);

		//model->m_pPad = new UCHAR[nWidth* nHeight];
		model->m_pPad = g_pMManager->pem_new<uchar>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);

		memset(model->m_pPad, 0, nWidth* nHeight);

		if (nPadBlobCnt > 0)
		{
			float fPad3DAvgHeigt = 0.0f;
			//Pad 이진화로 보정하자.
			Correct3d_Binary(PadMask, (sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), model->pf3D, nWidth, nHeight, fPad3DAvgHeigt);

			model->m_fPadArea3DAvgHeight = fPad3DAvgHeigt;

			//Correct3d((sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC), model->pf3D, nWidth, nHeight, model->m_ImageBuffer->imgTop_R, model->m_ImageBuffer->imgTop_B, model->m_ImageBuffer->imgTop_W);
			m_pProcMilAlgo->SaveWorkImg_float(model->pf3D, nWidth, nHeight, _T("Foot_3D_R2.bmp"));

			//검출된 Pad 정보를 저장..
			for (size_t y = 0; y < nHeight; y++)
			{
				UCHAR* pRaw = PadMask.ptr<UCHAR>(y);
				for (size_t x = 0; x < nWidth; x++)
					model->m_pPad[y*nWidth + x] = pRaw[x];
			}
		}

	}
	model->bUseTeachRect = sInspFoot->bUseTeachRect;
	model->nPadWidth = sInspFoot->nPadWidth;
	model->nPadHeight = sInspFoot->nPadHeight;
	model->nRadius = sInspFoot->nRadius;

	model->bUseRemovedPadArea = sInspFoot->bUseRemovedPadArea;

	return model;
}


CFoot_Model* CPInsp_Foot::MakeFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D, bool bPadAreaAutoTeach)
{
	//Foot 부분 초기화 
	//m_pAlgoPattern = (AlgoPattern *)sInspAlgo.m_ptrInspAlgoParam;
	//m_pAlgoPattern->GetParam(m_pInspParam);

	AlgoFoot* sInspFoot = (AlgoFoot*)sInspAlgo.m_ptrInspAlgoParam;
	//if ((sInspFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_Align) == m_eFOOT_Data2::m_eFOOT_Data2_Align)
		//return MakeRotateFoot(sInspFoot, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, bCorrect3D);
	//else
	return MakeFoot(sInspFoot, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, cv3DAvgFilter, cvDBCPolygonImg, bCorrect3D, bPadAreaAutoTeach);
}


bool CPInsp_Foot::TeachFoot(CFoot_Model* pFoot, int nfootKind, UCHAR *ucArrDstImg)
{
	BOOL ret = FALSE;
	if (ucArrDstImg == NULL) return ret;
	int nLine = __LINE__;

	try
	{
		int nWidth = pFoot->GetImageWidth();
		int nHeight = pFoot->GetImageLength();
		//memset(ucArrDstImg, 0, sizeof(UCHAR) * pFoot->GetWindowImageSize());

		nLine = __LINE__;
		cv::Mat Img(nHeight, nWidth, CV_8UC1), ImgWedge(nHeight, nWidth, CV_8UC1), ImgWing(nHeight, nWidth, CV_8UC1), ImgPatTarg(nHeight, nWidth, CV_8UC1);
		cv::Mat* reMat;

		int nBlobCnt = 0;
		if (nfootKind == (int)m_eFootBin::m_eFootBin_Wedge || nfootKind == (int)m_eFootBin::m_eFootBin_Wing)
		{
			RstAlgoFoot sRstAlgo;
			nBlobCnt = FindFoot(pFoot, &sRstAlgo, &Img, &ImgWedge, &ImgWing, &ImgPatTarg, -2);
			if (nBlobCnt > 0)
			{
				if (nfootKind == (int)m_eFootBin::m_eFootBin_Wedge)
					reMat = &ImgWedge;
				else if (nfootKind == (int)m_eFootBin::m_eFootBin_Wing)
					reMat = &ImgWing;
				else
					reMat = &Img;
			}
		}

		if (nBlobCnt == 0)
		{
			nLine = __LINE__;
			pFoot->GetImageMatrix(&Img, nfootKind);
			nBlobCnt = GetBinImage(pFoot, &Img, nfootKind, NULL, false);
			reMat = &Img;
		}

		if (nBlobCnt > 0) //결과 저장
		{
			for (int y = 0; y < nHeight; y++)
			{
				for (int x = 0; x < nWidth; x++)
				{
					int nIndex = y * nWidth + x;
					int nIndex2 = (y + pFoot->mImageRect.top) * pFoot->_WidthOrg + (x + pFoot->mImageRect.left);
					ucArrDstImg[nIndex2] = reMat->data[nIndex];
				}
			}
			m_pProcMilAlgo->SaveWorkImg(reMat->data, nWidth, nHeight, _T("Foot_Teach.bmp"));
			nLine = __LINE__;
		}

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::TeachFoot error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return ret;
	}

	return ret;


}

bool CPInsp_Foot::TeachFootColor(CFoot_Model* pFoot, int nfootKind, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, FootColor fcFindDBC, UCHAR *ucArrDstImg, int nStartX, int nStartY, cv::Mat *cv3DAvgFilter)
{
	BOOL ret = FALSE;
	if (ucArrDstImg == NULL) return ret;
	int nLine = __LINE__;

	try
	{
		int nWidth = pFoot->GetImageWidth();
		int nHeight = pFoot->GetImageLength();
		memset(ucArrDstImg, 0, sizeof(UCHAR) * pFoot->GetWindowImageSize());

		nLine = __LINE__;
		cv::Mat ImgPad(nHeight, nWidth, CV_8UC1), ImgFoot(nHeight, nWidth, CV_8UC1), ImgWedge(nHeight, nWidth, CV_8UC1), ImgWing(nHeight, nWidth, CV_8UC1), ImgPatTarg(nHeight, nWidth, CV_8UC1);

		int nBlobCnt = 0;

		RstAlgoFoot sRstAlgo;

		bool bUseFootOnly2D = pFoot->bUse2DImageForInspWidth;

		if (bUseFootOnly2D)	//폭 검출용 BW 옵션 켜면
		{
			pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max] = 0;
		}

		nBlobCnt = FindFoot(pFoot, &sRstAlgo, &ImgFoot, &ImgWedge, &ImgWing, &ImgPatTarg, -2);
		{

#ifdef _AlgoTool_1
			nfootKind = 3;
#endif

			if (nfootKind == 0)//Pad면
			{
				pFoot->GetImageMatrix(&ImgPad, m_eFootBin_Pad);
				GetBinImage(pFoot, &ImgPad, m_eFootBin_Pad, NULL, false);
			}
			else
			{
				pFoot->GetPadBinImage(&ImgPad);
			}


			/*pFoot->GetImageMatrix(&ImgFoot, m_eFootBin_Foot);
			GetBinImage(pFoot, &ImgFoot, m_eFootBin_Foot, NULL, false);*/
			m_eFootBin footbinType = (m_eFootBin)nfootKind;

			if (bUseFootOnly2D && footbinType == m_eFootBin_Foot)
			{
				cv::Rect ImageRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);
				cv::Mat Img(nHeight, nWidth, CV_8UC1), ImgWedge(nHeight, nWidth, CV_8UC1), ImgWing(nHeight, nWidth, CV_8UC1), Img3DArea(nHeight, nWidth, CV_32FC1);
				cv::Mat ImgTemp(nHeight, nWidth, CV_32FC1, pFoot->pf3D);
				ImgTemp.copyTo(Img3DArea);

				cv::Mat PadImg;
				RstAlgoFoot sRstAlgo;
				Calc_padBinImage4(pFoot, &sRstAlgo, ImageRect, &PadImg, Img3DArea, pFoot->bTeachBin);
				int nImgWidth = ImageRect.width;
				int nImgHeight = ImageRect.height;
				int imgSize = nImgWidth * nImgHeight;

				FootPoins fPoints;
				cv::Mat cvRoFootRctImg(nHeight, nWidth, CV_8UC1);
				bool bIsFoundFoot = FindFoot_3DBlob(pFoot, &sRstAlgo, &Img, &ImgWedge, &ImgWing, &PadImg, fPoints, nfootKind, cv3DAvgFilter, cvRoFootRctImg, NULL, bUseFootOnly2D, false);

				if (bIsFoundFoot)
				{

					ImgFoot = ImgFoot & fPoints.cvMatchingMask;

					cv::Mat cvFootOnly2D;
					cvFootOnly2D = ImgFoot.clone();

					bool bIsDBCFoot = false;

					if (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
						bIsDBCFoot = true;

					if (bIsDBCFoot)
					{
						cv::Mat DBCBinImage(cvFootOnly2D.rows, cvFootOnly2D.cols, CV_8UC1);
						DBCBinImage.setTo(0);

						bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

						///////////////////////
						if (bIsFindDBCOutLine)
							cvFootOnly2D = cvFootOnly2D - DBCBinImage;
					}

					double dArea = 0, dCX = 0, dCY = 0;
					CRect rcBlob{ 0,0,0,0 };
					int nMinBlob = 100;
					cv::Mat BinImageTmp(cvFootOnly2D.rows, cvFootOnly2D.cols, CV_8UC1);
					int nCntFBlob = m_pProcMilAlgo->CalcBlob_Select(cvFootOnly2D.data, BinImageTmp.data, cvFootOnly2D.cols, cvFootOnly2D.rows, nMinBlob, FALSE, TRUE, 0, eSelectBigger);
					if (nCntFBlob > 0)
					{
						dCX = 0, dCY = 0, dArea = 0;
						m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
					}

					nMinBlob = dArea * 0.1;
					if (100 > nMinBlob) nMinBlob = 100;
					nCntFBlob = m_pProcMilAlgo->CalcBlob_Select(cvFootOnly2D.data, cvFootOnly2D.data, cvFootOnly2D.cols, cvFootOnly2D.rows, nMinBlob, FALSE, TRUE, 0, eSelectMix);

					ImgFoot = cvFootOnly2D.clone();

#if _DEBUG
					cv::imwrite("D:\\FootRst\\CalcWidth_FootImg_Teach.bmp", ImgFoot);
#endif
				}

			}

			UCHAR* ptrPad = ImgPad.data;
			UCHAR* ptrFoot = ImgFoot.data;
			UCHAR* ptrWing = ImgWing.data;
			UCHAR* ptrWedge = ImgWedge.data;


			m_eFootBin footbin = (m_eFootBin)nfootKind;

			for (int y = 0; y < nHeight; y++)
			{
				for (int x = 0; x < nWidth; x++)
				{
					int nIndex = y * nWidth + x;
					//int nIndex2 = (y + pFoot->m_nWindowStartY) * pFoot->_WidthOrg + (x + pFoot->m_nWindowStartX);
					//int nIndex2 = (y + pFoot->mImageRect.top) * pFoot->_WidthOrg + (x + pFoot->mImageRect.left);
					int nIndex2 = y * pFoot->_WidthOrg + x;
					nIndex2 *= 3;

					if (footbin == m_eFootBin_Wing && ptrWing[nIndex])
						memcpy(&ucArrDstImg[nIndex2], &fcWing, 3);
					else if (footbin == m_eFootBin_Wedge && ptrWedge[nIndex])
						memcpy(&ucArrDstImg[nIndex2], &fcWedge, 3);
					else if (footbin == m_eFootBin_Foot && ptrFoot[nIndex])
						memcpy(&ucArrDstImg[nIndex2], &fcFoot, 3);
					else if (footbin == m_eFootBin_Pad && ptrPad[nIndex])
						memcpy(&ucArrDstImg[nIndex2], &fcBackground, 3);
					else
					{
						if (footbin == m_eFootBin_Pad) continue;

						//Wing
						if (ptrWing[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &fcWing, 3);

						//Wedge
						else if (ptrWedge[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &fcWedge, 3);

						//Foot
						else if (ptrFoot[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &fcFoot, 3);

						//Pad
						else if (ptrPad[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &fcBackground, 3);

					}

				}
			}
			nLine = __LINE__;
			ret = TRUE;


		}
		return ret;
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::TeachFootColor error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return ret;
	}

	return ret;
}

void CPInsp_Foot::InitPatternResultStruct(RstInspPattern* rst)
{
	//RstInspPattern* temp = new RstInspPattern;
	RstInspPattern* temp = g_pMManager->pem_new<RstInspPattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	memset(temp, 0, sizeof(RstInspPattern));

	temp->ok = -1;
	temp->isInsp = FALSE;
	temp->list.findOK = TRUE;
	temp->list.scoreOk = TRUE;
	temp->list.angleOk = TRUE;
	temp->list.offsetXOk = TRUE;
	temp->list.offsetYOk = TRUE;
	temp->list.polarityOK = TRUE;

	*rst = *temp;

	//delete temp;
	if (temp != nullptr)
		g_pMManager->pem_delete(temp, false);
	temp = nullptr;
}

int CPInsp_Foot::GetPart3DGradientImg(cv::Mat &xGradImgRst, cv::Mat &yGradImgRst, cv::Mat &SumXYGradImgRst, CSearchWire_Model* pModel)
{
	int ret = e_NG;

	//Get Part 3D Image
	cv::Mat ImgFootModel3DCorrect(pModel->m_pPartImageBuffer->nImageSizeY, pModel->m_pPartImageBuffer->nImageSizeX, CV_32FC1, pModel->m_pPart3D);

	//Remove values ​​with 3D height of 0 or negative
	size_t floatstep_3DCorrect = ImgFootModel3DCorrect.step / sizeof(float);
	for (int i = 0; i < ImgFootModel3DCorrect.rows; i++)
	{
		float* rowptr = (float*)ImgFootModel3DCorrect.data;
		for (int j = 0; j < ImgFootModel3DCorrect.cols; j++)
		{
			int val = rowptr[i * floatstep_3DCorrect + j];
			if (val <= 0)
			{
				val = 0;
				rowptr[i * floatstep_3DCorrect + j] = 0;
			}
		}
	}

	//1. Remove values ​​with 3D height of 0 or negative
	int wid, len;
	wid = pModel->m_nWidth;
	len = pModel->m_nHeight;
	float *fPart3D = pModel->m_pPart3D;

	for (int i = 1; i < len - 1; i++)
	{
		for (int j = 1; j < wid - 1; j++)
		{
			if (fPart3D[i*wid + j + 1] <= 0)
				fPart3D[i*wid + j + 1] = 0;
		}
	}

	//2. Calc Part 3D gradient (X, Y축 높이 변화량 측정)
	cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	for (int y = 1; y < (len)-1; y++)
	{
		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);
		for (int x = 1; x < wid - 1; x++)
		{

			//if (fPart3D[y*wid + x + 1] < 5)
			//{
			//	fPart3D[y*wid + x + 1] = 0;
			//}

			pXgrad[x] = fPart3D[y*wid + x + 1] - fPart3D[y*wid + x - 1];
			pYgrad[x] = fPart3D[(y + 1)*wid + x] - fPart3D[(y - 1)*wid + x];
		}
	}

	cv::Mat X3Dgrad_abs, Y3Dgrad_abs;

	//3-1. Calc Part 3D Sobel gradient
// 	int masksize = 3;
// 	cv::Mat part3DSobeldx, part3DSobeldy;
// 	cv::Sobel(ImgFootModel3DCorrect, part3DSobeldx, CV_32FC1, 1, 0, masksize);
// 	cv::Sobel(ImgFootModel3DCorrect, part3DSobeldy, CV_32FC1, 0, 1, masksize);
// 	Xgrad = part3DSobeldx;
// 	Ygrad = part3DSobeldy;

	//3. Calculate an absolute value - X, Y 기울기 영상에 절대값 취함
	X3Dgrad_abs = cv::abs(Xgrad);
	Y3Dgrad_abs = cv::abs(Ygrad);


	//4. Remove small gradients value -높이 변화량(=기울기 크기)이 작은 부분(5 이하) 제거
	size_t floatstep_3DX = X3Dgrad_abs.step / sizeof(float);
	size_t floatstep_3DY = Y3Dgrad_abs.step / sizeof(float);
	for (int i = 0; i < X3Dgrad_abs.rows; i++)
	{
		float* rowptrX = (float*)X3Dgrad_abs.data;
		float* rowptrY = (float*)Y3Dgrad_abs.data;
		for (int j = 0; j < X3Dgrad_abs.cols; j++)
		{
			int valX = rowptrX[i * floatstep_3DX + j];
			int valY = rowptrY[i * floatstep_3DY + j];
			if (valX < 5)
			{
				//valX = 0;
				//rowptrX[i * floatstep_3DX + j] = 0;
			}
			if (valY < 5)
			{
				//valY = 0;
				//rowptrY[i * floatstep_3DY + j] = 0;
			}
		}
	}

	//5. Sum Gradient images of X,Y axis  - X, Y축 기울기 영상 합산 
	cv::Mat XY3Dgrad;
	XY3Dgrad = X3Dgrad_abs + Y3Dgrad_abs;

	//6. x,y축 기울기 합산 영상 32 bit 에서 8bit(0-255)로 변환
	cv::Mat imSumXY3Dgrad_8bit;
	XY3Dgrad.convertTo(imSumXY3Dgrad_8bit, CV_8UC1);

	//255값 제거 및 normalize하기
	for (int i = 0; i < imSumXY3Dgrad_8bit.rows; i++)
	{
		uchar* rowptr = (uchar*)imSumXY3Dgrad_8bit.data;
		for (int j = 0; j < imSumXY3Dgrad_8bit.cols; j++)
		{
			int val = rowptr[i * imSumXY3Dgrad_8bit.step + j];

			if (val > 100)
			{
				imSumXY3Dgrad_8bit.data[i * imSumXY3Dgrad_8bit.step + j] = 0;
			}
		}
	}

	//normalize 하기
	cv::Mat imSumXY3Dgrad_8bit_norm;
	cv::normalize(imSumXY3Dgrad_8bit, imSumXY3Dgrad_8bit_norm, 0, 255, cv::NORM_MINMAX);
	//ImgFootModel3DCorrect.convertTo(ImgFootModel3DCorrect, CV_8UC1);

	//7. Binarize XY3Dgrad image
	cv::Mat imBinSumXY3Dgrad;
	cv::threshold(imSumXY3Dgrad_8bit_norm, imBinSumXY3Dgrad, 0, 255, cv::THRESH_OTSU);

	//8. Blob filtering
	int nImgWidth = imBinSumXY3Dgrad.rows;
	int nImgHeight = imBinSumXY3Dgrad.cols;
	int nCntBlob = 0;
	UCHAR* ptrSrcPart3DBlob = imSumXY3Dgrad_8bit.data;
	UCHAR *ptrDstPart3DBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrDstPart3DBlob, nImgHeight * nImgWidth);
	memset(ptrDstPart3DBlob, 0, sizeof(UCHAR) * nImgHeight * nImgWidth);

	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imBinSumXY3Dgrad.data, ptrDstPart3DBlob, nImgHeight, nImgWidth, 20, FALSE, FALSE, 1, 1);
	cv::Mat DstMatBlob(nImgWidth, nImgHeight, CV_8UC1, ptrDstPart3DBlob);

	if (ptrDstPart3DBlob && nImgWidth > 0 && nImgHeight > 0)
	{
		CString path = g_pMPTI->GetWireDebugDataFullPath(1, _T("FootPatternMatch"), _T("3DGradientBlobResult"), 0);
#if _DEBUG
		m_pProcMilAlgo->SaveDebugImg(ptrSrcPart3DBlob, nImgHeight, nImgWidth, path, 1, FALSE);
#endif
	}

	//9. get blob area
// 	double *dCX = 0;
// 	double *dCY = 0;
// 	double *dArea = 0;
// 	CRect *pRtFootSearchArea = NULL;
// 	if (nCntBlob > 0)
// 	{
// 		pRtFootSearchArea = new CRect[nCntBlob];
// 		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&dArea, nCntBlob);
// 		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&dCX, nCntBlob);
// 		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&dCY, nCntBlob);
// 
// 		// get parameter
// 		m_pProcMilAlgo->GetBlobResult(dArea, dCX, dCY, pRtFootSearchArea, FALSE);
// 
// 		//blob filtering
// 
// 		delete[] pRtFootSearchArea;
// 		Delete_1DArray(&dArea);
// 		Delete_1DArray(&dCX);
// 		Delete_1DArray(&dCY);
// 	}
// 
// 	delete[] ptrDstPart3DBlob;
// 	ptrDstPart3DBlob = NULL;


	xGradImgRst = X3Dgrad_abs;		//32FC1형 반환
	yGradImgRst = Y3Dgrad_abs;		//32FC1형 반환

	SumXYGradImgRst = imSumXY3Dgrad_8bit_norm;		//uchar로 반환

	//CString sSrc_image_path;
	//sSrc_image_path.Format(_T("d:\\testimage\\workimage\\Part3DGrad_SumXY.bmp"));
	//cv::imwrite(std::string(CT2A(sSrc_image_path)), imSumXY3Dgrad_8bit_norm);

	if (xGradImgRst.empty() || yGradImgRst.empty() || SumXYGradImgRst.empty())
		ret = e_NG;
	else
		ret = e_OK;

	return ret;

}

int CPInsp_Foot::GetBinImage(CFoot_Model* pFoot, cv::Mat* Image, int footKind, cv::Mat* Mmask, bool bMaxblob, int nMinblobArea, bool bUseFootOnly2D)
{
	cv::Mat BinImage(Image->rows, Image->cols, CV_8UC1);
	cv::Mat srcImg = *Image;
	cv::Mat fHRectImg(Image->rows, Image->cols, CV_32FC1, pFoot->pf3D);

#if _DEBUG
	cv::imwrite("D:\\FootRst\\0_Input3D_Foot.bmp", fHRectImg);
#endif

	// 	PIAL::PI_Buff src2D(srcImg, true);
	// 	PIAL::PI_Buff src3D(fHRectImg, true);
	// 	PIAL::PI_Buff dst(BinImage, true);

	if (pFoot->m_sArrBin[footKind].m_bIsSet == false)
		footKind = 0;

	double dArea = 0, dCX = 0, dCY = 0;
	CRect rcBlob{ 0,0,0,0 };
	AlgoBlob sAlgoBlob = pFoot->GetAlgoBlob(footKind);

	if(bUseFootOnly2D)
		sAlgoBlob.m_dHeightRateMax = 0;

	// 	PIAL::_AlgoBlob sAlgoBlob;
	// 	sAlgoBlob.m_bUseIPC = sAlgoBlob1.m_bUseIPC;
	// 	sAlgoBlob.m_bInvertCheck = sAlgoBlob1.m_bInvertCheck;
	// 	sAlgoBlob.m_bFilterIsUse = sAlgoBlob1.m_bFilterIsUse;
	// 	sAlgoBlob.m_nTypeSelectBlob = sAlgoBlob1.m_nTypeSelectBlob;
	// 
	// 	sAlgoBlob.m_bInsp3D = sAlgoBlob1.m_bInsp3D;	//false;
	// 	sAlgoBlob.m_nTypeRange3D = sAlgoBlob1.m_nTypeRange3D;
	// 	sAlgoBlob.m_dHeightRateMin = sAlgoBlob1.m_dHeightRateMin;	//0;
	// 	sAlgoBlob.m_dHeightRateMax = sAlgoBlob1.m_dHeightRateMax;	//0;
	// 
	// 	sAlgoBlob.m_bInsp2D = sAlgoBlob1.m_bInsp2D;
	// 	sAlgoBlob.m_nTypeRange2D = sAlgoBlob1.m_nTypeRange2D; //2;
	// 	sAlgoBlob.m_nMinBinary = sAlgoBlob1.m_nMinBinary;	//0;
	// 	sAlgoBlob.m_nMaxBinary = sAlgoBlob1.m_nMaxBinary;	//100;
	// 	sAlgoBlob.m_bFillHole = sAlgoBlob1.m_bFillHole;	//true;

	if (bMaxblob)
		sAlgoBlob.m_nTypeSelectBlob = _ETypeBlob::_eSelectBigger;

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;

	int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, srcImg.data, (float*)fHRectImg.data, NULL, srcImg.cols, srcImg.rows, nMinblobArea, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, false);
	//int nCntBlob = PIAL::PAlgo::BlobImageStruct(sAlgoBlob, &src2D, &src3D, NULL, &dst, srcImg.cols, srcImg.rows, nMinblobArea, &dArea, &dCX, &dCY, &rcBlob, stTieArea, true);
	BinImage.copyTo(*Image);

	/////////////////////////////////////////////////////
	if (pFoot->m_sArrBin[footKind].Moph)
	{
#if _DEBUG
		cv::imwrite("D:\\FootRst\\1_BW_Foot_Blob.bmp", BinImage);
#endif
		if (bMaxblob)
			sAlgoBlob.m_nTypeSelectBlob = _ETypeBlob::_eSelectBigger;

		if (Mmask != NULL)
		{
			BinImage &= (*Mmask);
		}
#if _DEBUG
		cv::imwrite("D:\\FootRst\\1_BW_Foot_BlobM.bmp", BinImage);
#endif
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(BinImage, BinImage, kernel);
		cv::dilate(BinImage, BinImage, kernel);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Moph.bmp", BinImage);
#endif

		sAlgoBlob.m_nTypeRange2D = _eTypeRangeUpper;
		sAlgoBlob.m_bInsp3D = false;
		sAlgoBlob.m_bInsp2D = true;
		sAlgoBlob.m_nMinBinary = 100;

		nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, BinImage.data, fHRectImg.ptr<float>(0), NULL, BinImage.cols, BinImage.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
		//nCntBlob = PIAL::PAlgo::BlobImageStruct(sAlgoBlob, &dst, &src3D, NULL, &dst, BinImage.cols, BinImage.rows, 4, &dArea, &dCX, &dCY, &rcBlob, stTieArea, true);
		BinImage.copyTo(*Image);
	}

#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage_2D.bmp", *Image);
#endif

	return nCntBlob;

}

int CPInsp_Foot::GetBinImage_Pad(CFoot_Model* pFoot, cv::Mat* Image, int footKind, cv::Mat* Mmask, bool bMaxblob, int nMinblobArea, bool bUseFootOnly2D, bool bUseOrg3DROI)
{
	cv::Mat BinImage(Image->rows, Image->cols, CV_8UC1);
	cv::Mat srcImg = *Image;
	//cv::Mat fHRectImg(Image->rows, Image->cols, CV_32FC1, pFoot->pf3D);
	cv::Mat fHRectImg(Image->rows, Image->cols, CV_32FC1, pFoot->pf3D_OrgROI);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\0_Input3D_Foot.bmp", fHRectImg);
#endif

	// 	PIAL::PI_Buff src2D(srcImg, true);
	// 	PIAL::PI_Buff src3D(fHRectImg, true);
	// 	PIAL::PI_Buff dst(BinImage, true);

	if (pFoot->m_sArrBin[footKind].m_bIsSet == false)
		footKind = 0;

	double dArea = 0, dCX = 0, dCY = 0;
	CRect rcBlob{ 0,0,0,0 };
	AlgoBlob sAlgoBlob = pFoot->GetAlgoBlob(footKind);

	if (bUseFootOnly2D)
		sAlgoBlob.m_dHeightRateMax = 0;

	// 	PIAL::_AlgoBlob sAlgoBlob;
	// 	sAlgoBlob.m_bUseIPC = sAlgoBlob1.m_bUseIPC;
	// 	sAlgoBlob.m_bInvertCheck = sAlgoBlob1.m_bInvertCheck;
	// 	sAlgoBlob.m_bFilterIsUse = sAlgoBlob1.m_bFilterIsUse;
	// 	sAlgoBlob.m_nTypeSelectBlob = sAlgoBlob1.m_nTypeSelectBlob;
	// 
	// 	sAlgoBlob.m_bInsp3D = sAlgoBlob1.m_bInsp3D;	//false;
	// 	sAlgoBlob.m_nTypeRange3D = sAlgoBlob1.m_nTypeRange3D;
	// 	sAlgoBlob.m_dHeightRateMin = sAlgoBlob1.m_dHeightRateMin;	//0;
	// 	sAlgoBlob.m_dHeightRateMax = sAlgoBlob1.m_dHeightRateMax;	//0;
	// 
	// 	sAlgoBlob.m_bInsp2D = sAlgoBlob1.m_bInsp2D;
	// 	sAlgoBlob.m_nTypeRange2D = sAlgoBlob1.m_nTypeRange2D; //2;
	// 	sAlgoBlob.m_nMinBinary = sAlgoBlob1.m_nMinBinary;	//0;
	// 	sAlgoBlob.m_nMaxBinary = sAlgoBlob1.m_nMaxBinary;	//100;
	// 	sAlgoBlob.m_bFillHole = sAlgoBlob1.m_bFillHole;	//true;

	if (bMaxblob)
		sAlgoBlob.m_nTypeSelectBlob = _ETypeBlob::_eSelectBigger;

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;

	int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, srcImg.data, (float*)fHRectImg.data, NULL, srcImg.cols, srcImg.rows, nMinblobArea, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, false);
	//int nCntBlob = PIAL::PAlgo::BlobImageStruct(sAlgoBlob, &src2D, &src3D, NULL, &dst, srcImg.cols, srcImg.rows, nMinblobArea, &dArea, &dCX, &dCY, &rcBlob, stTieArea, true);
	BinImage.copyTo(*Image);

	/////////////////////////////////////////////////////
	if (pFoot->m_sArrBin[footKind].Moph)
	{
#if _DEBUG
		cv::imwrite("D:\\FootRst\\1_BW_Foot_Blob.bmp", BinImage);
#endif
		if (bMaxblob)
			sAlgoBlob.m_nTypeSelectBlob = _ETypeBlob::_eSelectBigger;

		if (Mmask != NULL)
		{
			BinImage &= (*Mmask);
		}
#if _DEBUG
		cv::imwrite("D:\\FootRst\\1_BW_Foot_BlobM.bmp", BinImage);
#endif
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(BinImage, BinImage, kernel);
		cv::dilate(BinImage, BinImage, kernel);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Moph.bmp", BinImage);
#endif

		sAlgoBlob.m_nTypeRange2D = _eTypeRangeUpper;
		sAlgoBlob.m_bInsp3D = false;
		sAlgoBlob.m_bInsp2D = true;
		sAlgoBlob.m_nMinBinary = 100;

		nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, BinImage.data, fHRectImg.ptr<float>(0), NULL, BinImage.cols, BinImage.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
		//nCntBlob = PIAL::PAlgo::BlobImageStruct(sAlgoBlob, &dst, &src3D, NULL, &dst, BinImage.cols, BinImage.rows, 4, &dArea, &dCX, &dCY, &rcBlob, stTieArea, true);
		BinImage.copyTo(*Image);
	}

#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage_2D.bmp", *Image);
#endif

	return nCntBlob;

}

bool CPInsp_Foot::HasBlobRectLine(long *pLebel, USHORT* LabelImage, int nCntBlob, int nW, int nH, int nL, int nR, int nT, int nB)
{
	for (int a = 0; a < nCntBlob; a++)
	{

		for (int w = nL; w < nR; w++)
		{
			//Top Line
			if (pLebel[a] == LabelImage[nT * nW + w])
				return true;

			//Bottom
			if (pLebel[a] == LabelImage[nB * nW + w])
				return true;
		}

		for (int h = nT; h < nB; h++)
		{
			//Left Line
			if (pLebel[a] == LabelImage[h * nW + nL])
				return true;

			//Right Line
			if (pLebel[a] == LabelImage[h * nW + nR])
				return true;
		}

		return false;

	}
}

int CPInsp_Foot::FindFoot(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * ImgPatTarg, int nfootKind)
{
	int nImageHeight = pFoot->GetImageLength();
	int nImageWidth = pFoot->GetImageWidth();

	cv::Mat cvFind(nImageHeight, nImageWidth, CV_8UC1);

	pFoot->GetImageMatrix(&cvFind, (int)m_eFootBin::m_eFootBin_Foot);
	pFoot->GetImageMatrix(cvFootImg, (int)m_eFootBin::m_eFootBin_Foot);  //m_eFootBin_Pad
	pFoot->GetImageMatrix(cvWedgeImg, (int)m_eFootBin::m_eFootBin_Wedge);
	pFoot->GetImageMatrix(cvWing, (int)m_eFootBin::m_eFootBin_Wing);

	for (int r = 0; r < nImageHeight; r++)
	{
		UCHAR* TarPtr = ImgPatTarg->ptr(r);
		UCHAR* imgSrc = &pFoot->m_ImageBuffer->imgTop_W[r*nImageWidth];
		memcpy(TarPtr, imgSrc, sizeof(UCHAR)*nImageWidth);
	}
	cv::Mat BinImage(cvFootImg->rows, cvFootImg->cols, CV_8UC1);
	cv::Mat srcImg = *cvFootImg;
	cv::Mat srcImg1 = *cvWedgeImg;
	cv::Mat srcImg2 = *cvWing;

#if _DEBUG
	cv::imwrite("D:\\FootInspImg\\FindFoot_Org_Foot.bmp", srcImg);
	cv::imwrite("D:\\FootRst\\FindFoot_Org_Wedge.bmp", srcImg1);
	cv::imwrite("D:\\FootRst\\FindFoot_Org_Wing.bmp", srcImg2);
	//cv::imwrite("D:\\FootRst\\FindFoot_Org_Foot.bmp", cvFind);
#endif
	int nSearchSizeX = pFoot->m_fArrOptionValue[(int)m_eFoot::m_eFoot_DeviationX][m_eMMD::eMMD_Default] / m_resolX;
	int nSearchSizeY = pFoot->m_fArrOptionValue[(int)m_eFoot::m_eFoot_DeviationY][m_eMMD::eMMD_Default] / m_resolY;
	if (nSearchSizeX < 10) nSearchSizeX = 10;
	if (nSearchSizeX > nImageWidth) nSearchSizeX = nImageWidth - 1;
	if (nSearchSizeY < 10) nSearchSizeY = 10;
	if (nSearchSizeY > nImageHeight) nSearchSizeY = nImageHeight - 1;
	double dArea = 0, dCX = 0, dCY = 0;
	CRect rcBlob{ 0,0,0,0 };
	float fHIMin = 0.0f;
	float fHIMax = 0.0f;

	CPInsp::GetHeightMinMax(pFoot->pf3D, nImageWidth, nImageHeight, &fHIMin, &fHIMax);

	RECT BottomArea = pFoot->m_RefRect;
	if (((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_RefArea) == m_eFOOT_Data2::m_eFOOT_Data2_RefArea) &&
		(CPInsp::AngleRectChange2(pFoot->dAngle, nImageWidth, nImageHeight, pFoot->m_RefRect, &BottomArea)))
	{
		if (BottomArea.left < 0) BottomArea.left = 0;
		if (BottomArea.left > nImageWidth) BottomArea.left = nImageWidth - 1;
		if (BottomArea.top < 0) BottomArea.top = 0;
		if (BottomArea.top > nImageHeight) BottomArea.top = nImageHeight - 1;
		int nDstRectW = BottomArea.right - BottomArea.left;
		int nDstRectH = BottomArea.bottom - BottomArea.top;
		if (nDstRectW <= 0) nDstRectW = 1;
		if (nDstRectH <= 0) nDstRectH = 1;
		if (BottomArea.left + nDstRectW > nImageWidth) nDstRectW = nImageWidth - BottomArea.left - 1;
		if (BottomArea.top + nDstRectH > nImageHeight) nDstRectH = nImageHeight - BottomArea.top - 1;
		float BotAvgH = 0.0f;
		if (nDstRectW > 0 && nDstRectH > 0)
		{
			float BotSumH = 0;
			for (int r = BottomArea.top; r < BottomArea.top + nDstRectH; r++)
			{
				for (int c = BottomArea.left; c < BottomArea.left + nDstRectW; c++)
					BotSumH += pFoot->pf3DOrg[r * pFoot->GetWindowImageWidth() + c];
			}
			BotAvgH = BotSumH / (nDstRectH * nDstRectW);
		}
		sRstAlgo->m_rcRefArea.left = BottomArea.left;
		sRstAlgo->m_rcRefArea.top = BottomArea.top;
		sRstAlgo->m_rcRefArea.right = BottomArea.left + nDstRectW;
		sRstAlgo->m_rcRefArea.bottom = BottomArea.top + nDstRectH;
		sRstAlgo->m_fAreaHeight = BotAvgH;
	}
	else
	{
		sRstAlgo->m_rcRefArea.left = 0;
		sRstAlgo->m_rcRefArea.top = 0;
		sRstAlgo->m_rcRefArea.right = 0;
		sRstAlgo->m_rcRefArea.bottom = 0;
		sRstAlgo->m_fAreaHeight = 0.0f;
	}
	AlgoBlob sAlgoBlob = pFoot->GetAlgoBlob((int)m_eFootBin::m_eFootBin_Foot);    //m_eFootBin_Pad);
	if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_Find_RefA) == m_eFOOT_Data2::m_eFOOT_Data2_Find_RefA &&
		(pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_RefArea) == m_eFOOT_Data2::m_eFOOT_Data2_RefArea)
	{
		sAlgoBlob.m_bInsp3D = TRUE;
		sAlgoBlob.m_nTypeRange3D = eTypeRangeIn;
		float fAreaHeight = sRstAlgo->m_fAreaHeight;
		if (fAreaHeight > 0) fAreaHeight *= 0.8f;
		if (fAreaHeight < 0) fAreaHeight *= 1.1f;
		sAlgoBlob.m_dHeightRateMin = fAreaHeight;
		sAlgoBlob.m_dHeightRateMax = fAreaHeight + 350.0f;
		if (sAlgoBlob.m_dHeightRateMax > fHIMax / 2.0f && fHIMax / 2.0f > sAlgoBlob.m_dHeightRateMin + 200.0f)
			sAlgoBlob.m_dHeightRateMax = fHIMax / 2.0f;
	}

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;

	sAlgoBlob.m_bFillHole = 0;
	sAlgoBlob.m_nTypeSelectBlob = 1;
	int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, srcImg.data, pFoot->pf3D, NULL, srcImg.cols, srcImg.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, false);
	if (nCntBlob == 0)
	{
		return 0;
	}
#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage_Blob_Pad.bmp", BinImage);
#endif
	UCHAR* pucImg = NULL;
	UCHAR* pucImg1 = NULL;
	UCHAR* pucImg2 = NULL;
	float* pfImg = NULL;
	USHORT* LabelImage = NULL;
	int nImgWidth = nImageWidth;
	int nImgHeight = nImageHeight;
	int imgSize = nImgWidth * nImgHeight;
	long pLebel[PTR_BLOB_MAX];
	double pLebel_Area[PTR_BLOB_MAX];
	int pLebel_MaxIDX[PTR_BLOB_MAX];
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&LabelImage, imgSize);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg, imgSize);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg1, imgSize);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg2, imgSize);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pfImg, imgSize);
	memset(pLebel, 0, sizeof(long) * PTR_BLOB_MAX);
	memset(pLebel_Area, 0, sizeof(double) * PTR_BLOB_MAX);
	memset(pLebel_MaxIDX, 0, sizeof(int) * PTR_BLOB_MAX);
	memset(LabelImage, 0, sizeof(USHORT) * imgSize);
	memset(pucImg, 0, sizeof(UCHAR) * imgSize);
	memset(pucImg1, 0, sizeof(UCHAR) * imgSize);
	memset(pucImg2, 0, sizeof(UCHAR) * imgSize);
	memset(pfImg, 0, sizeof(float) * imgSize);
	m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);
	for (int a = 0; a < nCntBlob; a++)
	{
		for (int b = 0; b < nImgWidth * nImgHeight; b++)
		{
			if (pLebel[a] == LabelImage[b])
				pLebel_Area[a]++;
		}
	}
	if (GetCorrectIDX(nCntBlob, pLebel_Area, pLebel_MaxIDX) == 0)
	{
		Delete_1DArray(&pucImg);
		Delete_1DArray(&pucImg1);
		Delete_1DArray(&pucImg2);
		Delete_1DArray(&pfImg);
		Delete_1DArray(&LabelImage);
		return 0;
	}

	//Pad Area
	int nstartX = pFoot->mImageRect.left - pFoot->mWindowImageRect.left; //Window 내의 Image ROI 시작좌표
	int nstartY = pFoot->mImageRect.top - pFoot->mWindowImageRect.top;

	sRstAlgo->m_rcSearchArea.left = rcBlob.left + nstartX;
	sRstAlgo->m_rcSearchArea.top = rcBlob.top + nstartY;
	sRstAlgo->m_rcSearchArea.right = rcBlob.right + nstartX;
	sRstAlgo->m_rcSearchArea.bottom = rcBlob.bottom + nstartY;

	std::vector<long> vecLabelOrder;
	int ntempCntBlob = nCntBlob;
	if (/*nfootKind == -2 &&*/ nCntBlob > 1)
	{
		int nFootTeach = 1;
		vecLabelOrder.push_back(pLebel[pLebel_MaxIDX[0]]);

		for (int a = 1; a < nCntBlob; a++)
		{
			if (pLebel_Area[pLebel_MaxIDX[a]] > pLebel_Area[pLebel_MaxIDX[0]] * 0.5)
			{
				nFootTeach++;
				vecLabelOrder.push_back(pLebel[pLebel_MaxIDX[a]]);
			}
		}
		nCntBlob = nFootTeach;
	}
	bool bFind2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
	int nFindRange = 0;
	if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
		nFindRange = 1;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
		nFindRange = 2;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		nFindRange = 3;
	int nFindMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[m_eBin::m_eBin_Min2D];
	int nFindMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[m_eBin::m_eBin_Max2D];
	bool bFind3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
	int nFindRange3D = 0;
	if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
		nFindRange3D = 1;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
		nFindRange3D = 2;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		nFindRange3D = 3;
	float fFindMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
	float fFindMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];

	bool bWedge2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
	int nWedgeRange = 0;
	if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
		nWedgeRange = 1;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
		nWedgeRange = 2;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		nWedgeRange = 3;
	int nWedgeMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[m_eBin::m_eBin_Min2D];
	int nWedgeMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[m_eBin::m_eBin_Max2D];
	bool bWedge3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
	int nWedgeRange3D = 0;
	if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
		nWedgeRange3D = 1;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
		nWedgeRange3D = 2;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		nWedgeRange3D = 3;
	float fWedgeMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_fArrH[m_eMMD::eMMD_Min];
	float fWedgeMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_fArrH[m_eMMD::eMMD_Max];

	bool bWing2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
	int nWingRange = 0;
	if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
		nWingRange = 1;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
		nWingRange = 2;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		nWingRange = 3;
	int nWingMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[m_eBin::m_eBin_Min2D];
	int nWingMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[m_eBin::m_eBin_Max2D];
	bool bWing3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
	int nWingRange3D = 0;
	if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
		nWingRange3D = 1;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
		nWingRange3D = 2;
	else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
		nWingRange3D = 3;
	float fWingMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_fArrH[m_eMMD::eMMD_Min];
	float fWingMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_fArrH[m_eMMD::eMMD_Max];

	//if(false)
	if (nfootKind != -2 && ntempCntBlob/* nCntBlob*/ > 1) // 등록된 Foot 만
	{
		int nFootL = pFoot->m_FootCenterPoint_ImageRect.x - (nSearchSizeX / 2);
		int nFootT = pFoot->m_FootCenterPoint_ImageRect.y - (nSearchSizeY / 2);
		int nFootR = nFootL + nSearchSizeX;
		int nFootB = nFootT + nSearchSizeY;
		if (nFootL < 0) nFootL = 0;
		if (nFootT < 0) nFootT = 0;
		if (nFootR > nImgWidth) nFootR = nImgWidth;
		if (nFootB > nImgHeight) nFootB = nImgHeight;

		if (nFootL >= nImgWidth || nFootT >= nImgHeight)
		{ //예외처리 검사 영역을 벗어나면 Fail.

			Delete_1DArray(&pucImg);
			Delete_1DArray(&pucImg1);
			Delete_1DArray(&pucImg2);
			Delete_1DArray(&pfImg);
			Delete_1DArray(&LabelImage);
			return 0;
		}

		if (nFootR <= 0 || nFootB <= 0)
		{ //예외처리 검사 영역을 벗어나면 Fail.

			Delete_1DArray(&pucImg);
			Delete_1DArray(&pucImg1);
			Delete_1DArray(&pucImg2);
			Delete_1DArray(&pfImg);
			Delete_1DArray(&LabelImage);
			return 0;
		}

		RECT rcArea = CPInsp::GetBlobRect(&vecLabelOrder[0], LabelImage, nCntBlob, nImgWidth, nImgHeight, nFootL, nFootR, nFootT, nFootB);
		if (rcArea.left == -1 || rcArea.right == -1 || rcArea.top == -1 || rcArea.bottom == -1)
		{
			// Ofttset 범위해서 모샂으면 제일 가까운 Foot 찾음
			bool bSearch = false;
			while (nFootL > 1 || nFootT > 1 || nFootR < nImgWidth - 1 || nFootB < nImgHeight - 1)
			{
				--nFootL;
				--nFootT;
				++nFootR;
				++nFootB;

				if (nFootL < 1) nFootL = 1;
				if (nFootT < 1) nFootT = 1;
				if (nFootR >= nImgWidth) nFootR = nImgWidth - 1;
				if (nFootB >= nImgHeight) nFootB = nImgHeight - 1;

				//확장 검색
				if (HasBlobRectLine(&vecLabelOrder[0], LabelImage, nCntBlob, nImgWidth, nImgHeight, nFootL, nFootR, nFootT, nFootB))
				{
					rcArea = CPInsp::GetBlobRect(&vecLabelOrder[0], LabelImage, nCntBlob, nImgWidth, nImgHeight, nFootL, nFootR, nFootT, nFootB);

					if (rcArea.left != -1 && rcArea.right != -1 && rcArea.top != -1 && rcArea.bottom != -1)
					{
						bSearch = true;
						break;
					}
				}

			}

			if (!bSearch)
			{
				Delete_1DArray(&pucImg);
				Delete_1DArray(&pucImg1);
				Delete_1DArray(&pucImg2);
				Delete_1DArray(&pfImg);
				Delete_1DArray(&LabelImage);
				return 0;
			}
		}

		int nAreaGapX = -5.;
		int nAreaGapY = -5.;
		int nSX = rcArea.left + nAreaGapX;
		int nEX = rcArea.right - nAreaGapX;
		int nSY = rcArea.top + nAreaGapY;
		int nEY = rcArea.bottom - nAreaGapY;

		bool bIsDBCBondFoot = false;

		if (pFoot->m_nUseOption2 & m_eFOOT_Data2_DBC == m_eFOOT_Data2_DBC)
			bIsDBCBondFoot = true;

		int nFootDir = 0;
		if (45 >= pFoot->m_dTeachFootAngle || pFoot->m_dTeachFootAngle > 315)
			//prev_Up;
			nFootDir = 1;
		else if (135 >= pFoot->m_dTeachFootAngle && pFoot->m_dTeachFootAngle > 45)
			//idx_prev;
			nFootDir = 2;
		else if (225 >= pFoot->m_dTeachFootAngle && pFoot->m_dTeachFootAngle > 135)
			//next_Down;
			nFootDir = 3;
		else if (315 >= pFoot->m_dTeachFootAngle && pFoot->m_dTeachFootAngle > 225)
			//idx_next;
			nFootDir = 4;

		if (nFootDir == 1 || nFootDir == 3)
		{
			if ((pFoot->m_nUseOption2 & m_eFOOT_Data2_Dir_Up) == m_eFOOT_Data2_Dir_Up)
				nSY = 0;
			else
				nEY = nImgHeight;
		}

		if (nSX < 0) nSX = 0;
		if (nSY < 0) nSY = 0;
		if (nEX < 0) nEX = 0;
		if (nEY < 0) nEY = 0;
		if (nSX > nImgWidth) nSX = nImgWidth;
		if (nSY > nImgHeight) nSY = nImgHeight;
		if (nEX > nImgWidth) nEX = nImgWidth;
		if (nEY > nImgHeight) nEY = nImgHeight;
		for (int y = nSY; y < nEY; y++)
		{
			for (int x = nSX; x < nEX; x++)
			{
				int nIdx = y * nImgWidth + x;
				float fH = pFoot->pf3D[y * nImgWidth + x];
				bool b2DIn = false;
				//foot binarization
				if (bFind2D)
				{

					UCHAR ucG = cvFind.data[y * nImgWidth + x];
					if ((nFindRange == 0 && (nFindMin2D <= ucG && ucG <= nFindMax2D)) ||
						(nFindRange == 1 && (nFindMin2D > ucG || ucG < nFindMax2D)) ||
						(nFindRange == 2 && ucG >= nFindMax2D) ||
						(nFindRange == 3 && nFindMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bFind3D)
					{
						if ((nFindRange3D == 0 && (fFindMin3D <= fH && fH <= fFindMax3D)) ||
							(nFindRange3D == 1 && (fFindMin3D > fH || fH < fFindMax3D)) ||
							(nFindRange3D == 2 && fH >= fFindMax3D) ||
							(nFindRange3D == 3 && fFindMin3D >= fH))
							b2DIn = true;
					}
				}
				if (b2DIn)
					pucImg[nIdx] = 255;

				//wedge binarization
				b2DIn = false;
				if (bWedge2D)
				{
					UCHAR ucG = srcImg1.data[y * nImgWidth + x];
					if ((nWedgeRange == 0 && (nWedgeMin2D <= ucG && ucG <= nWedgeMax2D)) ||
						(nWedgeRange == 1 && (nWedgeMin2D > ucG || ucG < nWedgeMax2D)) ||
						(nWedgeRange == 2 && ucG >= nWedgeMax2D) ||
						(nWedgeRange == 3 && nWedgeMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bWedge3D)
					{
						if ((nWedgeRange3D == 0 && (fWedgeMin3D <= fH && fH <= fWedgeMax3D)) ||
							(nWedgeRange3D == 1 && (fWedgeMin3D > fH || fH < fWedgeMax3D)) ||
							(nWedgeRange3D == 2 && fH >= fWedgeMax3D) ||
							(nWedgeRange3D == 3 && fWedgeMin3D >= fH))
							b2DIn = true;
					}
					else
						b2DIn = true;
				}
				if (b2DIn) pucImg1[nIdx] = 255;

				//wing binarization
				b2DIn = false;
				if (bWing2D)
				{
					UCHAR ucG = srcImg2.data[y * nImgWidth + x];
					if ((nWingRange == 0 && (nWingMin2D <= ucG && ucG <= nWingMax2D)) ||
						(nWingRange == 1 && (nWingMin2D > ucG || ucG < nWingMax2D)) ||
						(nWingRange == 2 && ucG >= nWingMax2D) ||
						(nWingRange == 3 && nWingMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bWing3D)
					{
						if ((nWingRange3D == 0 && (fWingMin3D <= fH && fH <= fWingMax3D)) ||
							(nWingRange3D == 1 && (fWingMin3D > fH || fH < fWingMax3D)) ||
							(nWingRange3D == 2 && fH >= fWingMax3D) ||
							(nWingRange3D == 3 && fWingMin3D >= fH))
							b2DIn = true;
					}
					else
						b2DIn = true;
				}
				if (b2DIn) pucImg2[nIdx] = 255;
			}
		}

	}
	else //전체 검색
	{
		for (int y = 0; y < nImageHeight; y++)
		{
			for (int x = 0; x < nImageWidth; x++)
			{
				int nIdx = y * nImgWidth + x;
				float fH = pFoot->pf3D[y * nImgWidth + x];
				bool b2DIn = false;

				//foot binarization
				if (bFind2D)
				{
					UCHAR ucG = cvFind.data[y * nImgWidth + x];
					if ((nFindRange == 0 && (nFindMin2D <= ucG && ucG <= nFindMax2D)) ||
						(nFindRange == 1 && (nFindMin2D > ucG || ucG < nFindMax2D)) ||
						(nFindRange == 2 && ucG >= nFindMax2D) ||
						(nFindRange == 3 && nFindMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bFind3D)
					{
						if ((nFindRange3D == 0 && (fFindMin3D <= fH && fH <= fFindMax3D)) ||
							(nFindRange3D == 1 && (fFindMin3D > fH || fH < fFindMax3D)) ||
							(nFindRange3D == 2 && fH >= fFindMax3D) ||
							(nFindRange3D == 3 && fFindMin3D >= fH))
							b2DIn = true;
					}
				}
				if (b2DIn)
					pucImg[nIdx] = 255;

				//wedge binarization
				b2DIn = false;
				if (bWedge2D)
				{
					UCHAR ucG = srcImg1.data[y * nImgWidth + x];
					if ((nWedgeRange == 0 && (nWedgeMin2D <= ucG && ucG <= nWedgeMax2D)) ||
						(nWedgeRange == 1 && (nWedgeMin2D > ucG || ucG < nWedgeMax2D)) ||
						(nWedgeRange == 2 && ucG >= nWedgeMax2D) ||
						(nWedgeRange == 3 && nWedgeMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bWedge3D)
					{
						if ((nWedgeRange3D == 0 && (fWedgeMin3D <= fH && fH <= fWedgeMax3D)) ||
							(nWedgeRange3D == 1 && (fWedgeMin3D > fH || fH < fWedgeMax3D)) ||
							(nWedgeRange3D == 2 && fH >= fWedgeMax3D) ||
							(nWedgeRange3D == 3 && fWedgeMin3D >= fH))
							b2DIn = true;
					}
					else
						b2DIn = true;
				}
				if (b2DIn) pucImg1[nIdx] = 255;

				//wing binarization
				b2DIn = false;
				if (bWing2D)
				{
					UCHAR ucG = srcImg2.data[y * nImgWidth + x];
					if ((nWingRange == 0 && (nWingMin2D <= ucG && ucG <= nWingMax2D)) ||
						(nWingRange == 1 && (nWingMin2D > ucG || ucG < nWingMax2D)) ||
						(nWingRange == 2 && ucG >= nWingMax2D) ||
						(nWingRange == 3 && nWingMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bWing3D)
					{
						if ((nWingRange3D == 0 && (fWingMin3D <= fH && fH <= fWingMax3D)) ||
							(nWingRange3D == 1 && (fWingMin3D > fH || fH < fWingMax3D)) ||
							(nWingRange3D == 2 && fH >= fWingMax3D) ||
							(nWingRange3D == 3 && fWingMin3D >= fH))
							b2DIn = true;
					}
					else
						b2DIn = true;
				}
				if (b2DIn) pucImg2[nIdx] = 255;
			}
		}
	}

	/*m_pProcMilAlgo->SaveWorkImg(pucImg, nImgWidth, nImgHeight, _T("Foot_1.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucImg1, nImgWidth, nImgHeight, _T("Foot_2.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucImg2, nImgWidth, nImgHeight, _T("Foot_3.bmp"));*/

	cv::Mat BinImage1(nImgHeight, nImgWidth, CV_8UC1, pucImg);
	cv::Mat BinImage4(nImgHeight, nImgWidth, CV_8UC1);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage1.bmp", BinImage1);
	cv::imwrite("D:\\FootRst\\GetBinImage4.bmp", BinImage4);
#endif
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
	cv::erode(BinImage1, BinImage1, kernel);
	cv::dilate(BinImage1, BinImage1, kernel);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage_Mop1.bmp", BinImage1);
#endif
	int nMinBlob = 100;
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage1.data, BinImage4.data, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, eSelectBigger);
	if (nCntBlob > 0)
	{
		dCX = 0, dCY = 0, dArea = 0;
		m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
	}
#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage4.bmp", BinImage4);
#endif
	nMinBlob = dArea * 0.1;
	if (100 > nMinBlob) nMinBlob = 100;
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage1.data, cvFootImg->data, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, eSelectMix);
	m_pProcMilAlgo->SaveWorkImg_float(pFoot->pf3D, nImgWidth, nImgHeight, _T("Find_3D_ORG.bmp"));
	for (int y = 0; y < nImgHeight; y++)
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			int nIdx = y * nImgWidth + x;
			pfImg[nIdx] = (cvFootImg->data[nIdx] == 255) ? pFoot->pf3D[nIdx] : 0.0f;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pucImg1, nImgWidth, nImgHeight, _T("Foot_2.bmp"));
	m_pProcMilAlgo->SaveWorkImg(pucImg2, nImgWidth, nImgHeight, _T("Foot_3.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(pfImg, nImgWidth, nImgHeight, _T("Find_3DArea.bmp"));
	cv::Mat BinImage2(nImgHeight, nImgWidth, CV_8UC1, pucImg1);
	cv::Mat BinImage3(nImgHeight, nImgWidth, CV_8UC1, pucImg2);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage2.bmp", BinImage2);
	cv::imwrite("D:\\FootRst\\GetBinImage3.bmp", BinImage3);
#endif
	cv::erode(BinImage2, BinImage2, kernel);
	cv::dilate(BinImage2, BinImage2, kernel);
	cv::erode(BinImage3, BinImage3, kernel);
	cv::dilate(BinImage3, BinImage3, kernel);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage_Mop2.bmp", BinImage2);
	cv::imwrite("D:\\FootRst\\GetBinImage_Mop3.bmp", BinImage3);
#endif
	for (int y = 0; y < nImgHeight; y++)
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			int nIdx = y * nImgWidth + x;
			cvWedgeImg->data[nIdx] = (cvFootImg->data[nIdx] == 255) ? pucImg1[nIdx] : 0;
			cvWing->data[nIdx] = (cvFootImg->data[nIdx] == 255) ? BinImage3.data[nIdx] : 0;

			//Wedge BW 티칭된 영상에서 Wing BW 티칭 영역 제외
// 			if (cvWing->data[nIdx] == 255)
// 			{
// 				cvWedgeImg->data[nIdx] = 0;
// 			}
		}
	}
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Find_Foot.bmp", *cvFootImg);
	cv::imwrite("D:\\FootRst\\Find_Wedge.bmp", *cvWedgeImg);
	cv::imwrite("D:\\FootRst\\Find_Wing.bmp", *cvWing);
#endif

	Delete_1DArray(&pucImg);
	Delete_1DArray(&pucImg1);
	Delete_1DArray(&pucImg2);
	Delete_1DArray(&pfImg);
	Delete_1DArray(&LabelImage);
	return nCntBlob;
}

bool CPInsp_Foot::TeachFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, UCHAR *ucArrDstImg)
{
	cv::Mat cvimg(0, 0, CV_32FC1);
	cv::Mat cvimg_dbc(0, 0, CV_8UC1);

	CFoot_Model* pFoot = MakeFoot(sInspAlgo, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, &cvimg, &cvimg_dbc, nfootKind != m_eFootBin_Pad);

	bool ret2 = TeachFoot(pFoot, nfootKind, ucArrDstImg);

	//delete pFoot;
	g_pMManager->pem_delete(pFoot, false);
	return ret2;

}
bool CPInsp_Foot::TeachFootColor(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, FootColor fcFindDBC, UCHAR *ucArrDstImg, cv::Mat *cv3DAvgFilter)
{
	cv::Mat cvimg(0, 0, CV_32FC1);
	cv::Mat cvimg_dbc(0, 0, CV_8UC1);
	CFoot_Model* pFoot = MakeFoot(sInspAlgo, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, &cvimg, &cvimg_dbc, nfootKind != m_eFootBin_Pad);

	nStartX = pFoot->mImageRect.left - (ptrAlgoColorOpt.m_sFovImg.nImageSizeX / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeX);
	nStartY = pFoot->mImageRect.top - (ptrAlgoColorOpt.m_sFovImg.nImageSizeY / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeY);

	if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeX <= 0) nStartX = pFoot->mImageRect.left;
	if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeY <= 0)nStartY = pFoot->mImageRect.top;

	bool ret2 = TeachFootColor(pFoot, nfootKind, fcBackground, fcFoot, fcWing, fcWedge, fcFindDBC, ucArrDstImg, nStartX, nStartY, cv3DAvgFilter);

	//delete pFoot;
	g_pMManager->pem_delete(pFoot, false);
	return ret2;

}

void CPInsp_Foot::Correct3d(int OptionKind, float * height, int wid, int len, BYTE * TopR, BYTE * TopB, BYTE * TopW, bool bGrouping)
{
#ifdef _DEBUG
	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = wid;
		hPtr.uiNumCol = len;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\FootRst\\Height_Bottom_pre.ptr"), &hPtr, height);
	}
#endif


	cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	for (int y = 1; y < len - 1; y++)
	{
		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);
		for (int x = 1; x < wid - 1; x++)
		{
			pXgrad[x] = height[y*wid + x + 1] - height[y*wid + x - 1];
			pYgrad[x] = height[(y + 1)*wid + x] - height[(y - 1)*wid + x];
		}
	}

	float * pXgrad = Xgrad.ptr<float>(0);
	float * pYgrad = Ygrad.ptr<float>(0);
	if (OptionKind > 0)
	{
		Correct3dDBC(CRect(0, 0, wid, len), height, pXgrad, pYgrad, TopR, TopB, TopW, wid, len, 0);
	}
	else
	{
		std::vector<std::array<int, 1200>> HistWid(wid);
		std::vector<std::array<int, 5>> HistMax(wid);
#ifdef _DEBUG
		cv::Mat tempImage = cv::Mat::zeros(cv::Size(wid, len), CV_8U);
		UCHAR * ptempimage = tempImage.ptr<UCHAR>(0);
#endif
		const int nHistOffset(600);
		for (int y = 0; y < len; y++)
		{
			float * pXgrad = Xgrad.ptr<float>(y);
			float * pYgrad = Ygrad.ptr<float>(y);

			for (int x = 0; x < wid; x++)
			{
				if ((/*(TopR[y*wid + x] > 220 || TopB[y*wid + x] > 200) &&*/ TopW[y*wid + x] > m_nFootPadTopWGrayLevel) && TopB[y*wid + x] > 5 &&
					(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
					(fabs(pYgrad[x]) < 5 && fabs(pXgrad[x]) < 5))
				{
					if (isnan(height[y*wid + x]) == true)
						continue;
					int nVal = (int)(height[y*wid + x] * 0.1f);
					if (1200 >= nVal)
						continue;

					nVal += nHistOffset;

					HistWid[x][nVal]++;

#ifdef _DEBUG
					ptempimage[y*wid + x] = 255;
#endif
				}
			}
		}
#ifdef _DEBUG
		cv::imwrite("D:\\FootRst\\Correct3DArea.bmp", tempImage);
#endif
		for (int x = 0; x < wid; x++)
		{
			int nMaxHistIdx(-6000), nMaxHistVal(INT_MIN);
			HistMax[x][0] = -6000;
			HistMax[x][1] = 0;
			for (int n = 0; n < 1200; n++)
			{
				if (HistWid[x][n] > nMaxHistVal)
				{
					nMaxHistVal = HistWid[x][n];
					nMaxHistIdx = n;
				}
			}

			HistMax[x][0] = nMaxHistIdx;
			HistMax[x][1] = nMaxHistVal;
		}

		int nSearchStep(100);
		std::vector<CRect> rectList;
		for (int x = 2; x < wid - 2; x++)
		{
			if (HistMax[x][0] > 0 && HistMax[x + 2][0] - HistMax[x - 2][0] > 50 && nSearchStep == 100)
			{
				HistMax[x][2] = 50;

				nSearchStep = 200;

				rectList.emplace_back(CRect(x, 0, wid, len));
			}
			else if (HistMax[x - 2][0] - HistMax[x + 2][0] > 50 && nSearchStep == 200)
			{
				if (rectList.empty() == true)
					rectList.emplace_back(CRect(0, 0, 0, len));
				if ((x - 20) > rectList[rectList.size() - 1].left)
				{
					HistMax[x][2] = 100;
					nSearchStep = 100;
					rectList[rectList.size() - 1].right = x;
				}
			}
		}

		if (rectList.empty() == true || bGrouping == false)
			Correct3dFoot(CRect(0, 0, wid, len), height, pXgrad, pYgrad, TopR, TopB, TopW, wid, len, 0);
		else
		{
			for (int n = 0; n < rectList.size(); n++)
			{
				if (n == 0)
				{
					rectList[n].left = 0;
				}
				else if (n > 0)
				{
					rectList[n].left = (rectList[n].left + rectList[n - 1].right) / 2;
				}

				if (n < (rectList.size() - 1))
				{
					rectList[n].right = (rectList[n].right + rectList[n + 1].left) / 2;
				}
				else if (n == (rectList.size() - 1))
				{
					rectList[n].right = wid;
				}

				Correct3dFoot(rectList[n], height, pXgrad, pYgrad, TopR, TopB, TopW, wid, len, 0);
			}
		}
	}

#ifdef _DEBUG
	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = wid;
		hPtr.uiNumCol = len;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\FootRst\\Height_Bottom_post.ptr"), &hPtr, height);
	}
#endif
}

void CPInsp_Foot::Correct3dFoot(CRect roi, float * height, float * Xgrad, float * Ygrad, BYTE * TopR, BYTE * TopB, BYTE * TopW, int wid, int len, int HistHeightLowLimit)
{
	int Hist[1200] = { 0, };

	const int nHistOffset(600);
	for (int y = roi.top; y < roi.bottom; y++)
	{
		for (int x = roi.left; x < roi.right; x++)
		{
			if ((/*(TopR[y*wid + x] > 220 || TopB[y*wid + x] > 220) &&*/ TopW[y*wid + x] > m_nFootPadTopWGrayLevel) && TopB[y*wid + x] > 5 &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				if (isnan(height[y*wid + x]))
					continue;
				int nVal = (int)(height[y*wid + x] * 0.1f);
				nVal += nHistOffset;
				Hist[nVal]++;
			}
		}

	}
	int nMaxHistIdx(-1), nMaxHistVal(INT_MIN);
	for (int n = 0; n < 1200; n++)
	{
		if (Hist[n] > nMaxHistVal)
		{
			nMaxHistVal = Hist[n];
			nMaxHistIdx = n;
		}
	}

	std::vector< jsl::Point3d<double, double, double> > dataList;
	dataList.reserve(wid*len);
	jsl::LeastSquare lsCalc;
	float stdHeight = (nMaxHistIdx - nHistOffset) * 10;
	for (int y = roi.top; y < roi.bottom; y++)
	{
		for (int x = roi.left; x < roi.right; x++)
		{
			if ((/*(TopR[y*wid + x] > 220 || TopB[y*wid + x] > 220) &&*/ TopW[y*wid + x] > m_nFootPadTopWGrayLevel) && TopB[y*wid + x] > 5 &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(height[y*wid + x] > (stdHeight - 50) && height[y*wid + x] < (stdHeight + 50)) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				jsl::Point3d<double, double, double> val(x, y, height[y*wid + x] * 0.001);
				dataList.emplace_back(val);
			}
		}
	}
	std::vector< jsl::Point3d<double, double, double> > dataList2;

	int nStep(1);
	if (dataList.size() > 60000)
	{
		nStep = dataList.size() / 60000;
	}

	for (int n = 0; n < dataList.size(); n += nStep)
	{
		dataList2.emplace_back(dataList[n]);
	}

	if (dataList.size() < 20)
		return;
	{
		std::vector<double> coeff;
		lsCalc.Run2d(1, dataList2, coeff);

		if (coeff.size() < 3)
			return;

		double * pCoeff = &coeff[0];
		for (int y = roi.top; y < roi.bottom; y++)
		{
			for (int x = roi.left; x < roi.right; x++)
			{
				//	height[y*wid + x] -= lsCalc.Apply2d(x, y, coeff) * 1000.0;
				height[y*wid + x] -= (pCoeff[0] * (double)+(x)+pCoeff[1] * (double)(y)+pCoeff[2]) * 1000.0;
			}
		}
	}
}

void CPInsp_Foot::Correct3dDBC(CRect roi, float * height, float * Xgrad, float * Ygrad, BYTE * TopR, BYTE * TopB, BYTE * TopW, int wid, int len, int HistHeightLowLimit)
{
	cv::Mat HistMat = cv::Mat::zeros(1, 1200, CV_32SC1);
	//	int Hist[1200] = { 0, };
	int * Hist = HistMat.ptr<int>(0);

	const int nHistOffset(600);
	for (int y = 0; y < len; y++)
	{
		for (int x = 0; x < wid; x++)
		{
			if ((/*TopR[y*wid + x] > 64 && TopB[y*wid + x] > 64 &&*/ TopW[y*wid + x] > m_nFootPadTopWGrayLevel) &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				int nVal = (int)(height[y*wid + x] * 0.1f);
				nVal += nHistOffset;
				Hist[nVal]++;
			}
		}

	}

	int nMaxHistIdx(-1);
	double nMaxHistVal(FLT_MIN);
	// 	for (int n = 0; n < 1200; n++)
	// 	{
	// 		if (Hist[n] > nMaxHistVal)
	// 		{
	// 			nMaxHistVal = Hist[n];
	// 			nMaxHistIdx = n;
	// 		}
	// 	}

	CHistogramAnalysis_New ha;
	ha.Run(HistMat);

	int nPeakCount = ha.m_PeakList.size();

	for (int n = 0; n < nPeakCount; n++)
	{
		if (ha.m_PeakList[n].dValue > nMaxHistVal)
		{
			nMaxHistVal = ha.m_PeakList[n].dValue;
			nMaxHistIdx = n;
		}
	}

	if (nMaxHistIdx < 0)
		return;
	float hSt = (ha.m_PeakList[nMaxHistIdx].nStRmsIdx - nHistOffset) * 10.0f;
	float hEd = (ha.m_PeakList[nMaxHistIdx].nEdRmsIdx - nHistOffset) * 10.0f;

	std::vector< jsl::Point3d<double, double, double> > dataList;
	dataList.reserve(wid*len);
	jsl::LeastSquare lsCalc;
	float stdHeight = (nMaxHistIdx - nHistOffset) * 10;
	for (int y = 0; y < len; y++)
	{
		for (int x = 0; x < wid; x++)
		{
			if ((/*TopR[y*wid + x] > 64 && TopB[y*wid + x] > 64 &&*/ TopW[y*wid + x] > m_nFootPadTopWGrayLevel) &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(height[y*wid + x] > hSt && height[y*wid + x] < hEd) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				jsl::Point3d<double, double, double> val(x, y, height[y*wid + x] * 0.001);
				dataList.emplace_back(val);
			}
		}
	}

	if (dataList.size() < 20)
		return;

	{
		std::vector<double> coeff;
		lsCalc.Run2d(1, dataList, coeff);

		if (coeff.size() < 3)
			return;

		double * pCoeff = &coeff[0];
		for (int y = 0; y < len; y++)
		{
			for (int x = 0; x < wid; x++)
			{
				//	height[y*wid + x] -= lsCalc.Apply2d(x, y, coeff) * 1000.0;
				height[y*wid + x] -= (pCoeff[0] * (double)(x)+pCoeff[1] * (double)(y)+pCoeff[2]) * 1000.0;
			}
		}
	}
}

void CPInsp_Foot::Correct3d_Binary_DBC(CFoot_Model* pFoot, cv::Mat Mask, int OptionKind, float * height, int wid, int len, float &fPadAvgHeight)
{
#ifdef _DEBUG
	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = wid;
		hPtr.uiNumCol = len;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\FootRst\\Height_Bottom_pre.ptr"), &hPtr, height);
	}
#endif


	cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	for (int y = 1; y < len - 1; y++)
	{
		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);
		for (int x = 1; x < wid - 1; x++)
		{
			pXgrad[x] = height[y*wid + x + 1] - height[y*wid + x - 1];
			pYgrad[x] = height[(y + 1)*wid + x] - height[(y - 1)*wid + x];
		}
	}



	cv::Mat tempImage = cv::Mat::zeros(cv::Size(wid, len), CV_8U);
	UCHAR * ptempimage = tempImage.ptr<UCHAR>(0);

	cv::Mat DBCBinImage(len, wid, CV_8UC1);
	DBCBinImage.setTo(0);
	if (OptionKind > 0)
	{
		bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

		///////////////////////
		//if (bIsFindDBCOutLine)
		//	PadMaskCalc = PadMaskCalc + DBCBinImage;
	}

	const int nHistOffset(600);

	float fRefAreaHgt = 0.0f;
	int nRefAreaCnt = 0;

	for (int y = 0; y < len; y++)
	{
		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);

		UCHAR* pMaskRow = Mask.ptr<UCHAR>(y);
		UCHAR* pMaskSilkLines = DBCBinImage.ptr<UCHAR>(y);

		for (int x = 0; x < wid; x++)
		{
			if (pMaskRow[x] &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(fabs(pYgrad[x]) < 5 && fabs(pXgrad[x]) < 5) && pMaskSilkLines[x] != 0)
			{
				if (isnan(height[y*wid + x]) == true)
					continue;

				ptempimage[y*wid + x] = 255;
				nRefAreaCnt++;
				fRefAreaHgt = fRefAreaHgt + height[y*wid + x];

			}
		}
	}

	fPadAvgHeight = fRefAreaHgt / nRefAreaCnt;	//Pad 영역 평균 3D 높이

#ifdef _DEBUG
	cv::imwrite("D:\\FootRst\\Correct3DArea.bmp", tempImage);
#endif

	float * pXgrad = Xgrad.ptr<float>(0);
	float * pYgrad = Ygrad.ptr<float>(0);
	if (OptionKind > 0)
		Correct3dDBC_Binary(Mask, height, pXgrad, pYgrad, wid, len, 0);
	else
		Correct3dFoot_Binary(Mask, height, pXgrad, pYgrad, wid, len, 0);

#ifdef _DEBUG
	cv::Mat cvAfterBackgroundHgt0(Mask.rows, Mask.cols, CV_32FC1, height);
#endif

#ifdef _AlgoTool_1
	DeleteFile(_T("D:\\FootRst\\Height_Bottom_post.ptr"));
#endif
//
#if defined(_DEBUG) || defined(_AlgoTool_1)
	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = wid;
		hPtr.uiNumCol = len;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\FootRst\\Height_Bottom_post.ptr"), &hPtr, height);
	}
#endif


}


void CPInsp_Foot::Correct3d_Binary(cv::Mat Mask, int OptionKind, float * height, int wid, int len, float &fPadAvgHeight)
{
#ifdef _DEBUG
	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = wid;
		hPtr.uiNumCol = len;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\FootRst\\Height_Bottom_pre.ptr"), &hPtr, height);
	}
#endif


	cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	for (int y = 1; y < len - 1; y++)
	{
		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);
		for (int x = 1; x < wid - 1; x++)
		{
			pXgrad[x] = height[y*wid + x + 1] - height[y*wid + x - 1];
			pYgrad[x] = height[(y + 1)*wid + x] - height[(y - 1)*wid + x];
		}
	}

	cv::Mat tempImage = cv::Mat::zeros(cv::Size(wid, len), CV_8U);
	UCHAR * ptempimage = tempImage.ptr<UCHAR>(0);

	const int nHistOffset(600);

	float fRefAreaHgt = 0.0f;
	int nRefAreaCnt = 0;

	for (int y = 0; y < len; y++)
	{
		float * pXgrad = Xgrad.ptr<float>(y);
		float * pYgrad = Ygrad.ptr<float>(y);

		UCHAR* pMaskRow = Mask.ptr<UCHAR>(y);

		for (int x = 0; x < wid; x++)
		{
			if (pMaskRow[x] &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(fabs(pYgrad[x]) < 5 && fabs(pXgrad[x]) < 5))
			{
				if (isnan(height[y*wid + x]) == true)
					continue;

				ptempimage[y*wid + x] = 255;
				nRefAreaCnt++;
				fRefAreaHgt = fRefAreaHgt + height[y*wid + x];

			}
		}
	}

	fPadAvgHeight = fRefAreaHgt / nRefAreaCnt;	//Pad 영역 평균 3D 높이

#ifdef _DEBUG
	cv::imwrite("D:\\FootRst\\Correct3DArea.bmp", tempImage);
#endif

	float * pXgrad = Xgrad.ptr<float>(0);
	float * pYgrad = Ygrad.ptr<float>(0);
	if (OptionKind > 0)
		Correct3dDBC_Binary(Mask, height, pXgrad, pYgrad, wid, len, 0);
	else
		Correct3dFoot_Binary(Mask, height, pXgrad, pYgrad, wid, len, 0);

#ifdef _DEBUG
	cv::Mat cvAfterBackgroundHgt0(Mask.rows, Mask.cols, CV_32FC1, height);
#endif

#ifdef _AlgoTool_1
	DeleteFile(_T("D:\\FootRst\\Height_Bottom_post.ptr"));
#endif
//
#if defined(_DEBUG) || defined(_AlgoTool_1)
	{
		HEADER_PTR hPtr;
		hPtr.uiNumRow = wid;
		hPtr.uiNumCol = len;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\FootRst\\Height_Bottom_post.ptr"), &hPtr, height);
	}
#endif


}

void CPInsp_Foot::Correct3dFoot_Binary(cv::Mat Mask, float * height, float * Xgrad, float * Ygrad, int wid, int len, int HistHeightLowLimit)
{
	int Hist[1200] = { 0, };

	const int nHistOffset(600);
	for (int y = 0; y < len; y++)
	{
		UCHAR* pMaskRow = Mask.ptr<UCHAR>(y);

		for (int x = 0; x < wid; x++)
		{
			if (pMaskRow[x] &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				if (isnan(height[y*wid + x]))
					continue;
				int nVal = (int)(height[y*wid + x] * 0.1f);
				nVal += nHistOffset;
				Hist[nVal]++;
			}
		}

	}
	int nMaxHistIdx(-1), nMaxHistVal(INT_MIN);
	for (int n = 0; n < 1200; n++)
	{
		if (Hist[n] > nMaxHistVal)
		{
			nMaxHistVal = Hist[n];
			nMaxHistIdx = n;
		}
	}

	std::vector< jsl::Point3d<double, double, double> > dataList;
	dataList.reserve(wid*len);
	jsl::LeastSquare lsCalc;
	float stdHeight = (nMaxHistIdx - nHistOffset) * 10;
	for (int y = 0; y < len; y++)
	{
		UCHAR* pMaskRow = Mask.ptr<UCHAR>(y);

		for (int x = 0; x < wid; x++)
		{
			if (pMaskRow[x] &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(height[y*wid + x] > (stdHeight - 50) && height[y*wid + x] < (stdHeight + 50)) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				jsl::Point3d<double, double, double> val(x, y, height[y*wid + x] * 0.001);
				dataList.emplace_back(val);
			}
		}
	}
	std::vector< jsl::Point3d<double, double, double> > dataList2;

	int nStep(1);
	if (dataList.size() > 60000)
	{
		nStep = dataList.size() / 60000;
	}

	if (dataList.size() < 20)
		return;

	dataList2.reserve(dataList.size());
	for (int n = 0; n < dataList.size(); n += nStep)
	{
		dataList2.emplace_back(dataList[n]);
	}

	{
		std::vector<double> coeff;
		lsCalc.Run2d(1, dataList2, coeff);

		if (coeff.size() < 3)
			return;

		double * pCoeff = &coeff[0];
		for (int y = 0; y < len; y++)
		{
			for (int x = 0; x < wid; x++)
			{
				//	height[y*wid + x] -= lsCalc.Apply2d(x, y, coeff) * 1000.0;
				height[y*wid + x] -= (pCoeff[0] * (double)+(x)+pCoeff[1] * (double)(y)+pCoeff[2]) * 1000.0;
			}
		}
	}
}

void CPInsp_Foot::Correct3dDBC_Binary(cv::Mat Mask, float * height, float * Xgrad, float * Ygrad, int wid, int len, int HistHeightLowLimit)
{
	cv::Mat HistMat = cv::Mat::zeros(1, 1200, CV_32SC1);
	//	int Hist[1200] = { 0, };
	int * Hist = HistMat.ptr<int>(0);

	const int nHistOffset(600);
	for (int y = 0; y < len; y++)
	{
		UCHAR* pMaskRow = Mask.ptr<UCHAR>(y);

		for (int x = 0; x < wid; x++)
		{
			if (pMaskRow[x] &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				if (isnan(height[y*wid + x]))
					continue;
				int nVal = (int)(height[y*wid + x] * 0.1f);
				nVal += nHistOffset;
				Hist[nVal]++;
			}
		}

	}

	int nMaxHistIdx(-1);
	double nMaxHistVal(FLT_MIN);

	CHistogramAnalysis_New ha;
	ha.Run(HistMat);

	int nPeakCount = ha.m_PeakList.size();

	for (int n = 0; n < nPeakCount; n++)
	{
		if (ha.m_PeakList[n].dValue > nMaxHistVal)
		{
			nMaxHistVal = ha.m_PeakList[n].dValue;
			nMaxHistIdx = n;
		}
	}

	if (nMaxHistIdx < 0)
		return;
	float hSt = (ha.m_PeakList[nMaxHistIdx].nStRmsIdx - nHistOffset) * 10.0f;
	float hEd = (ha.m_PeakList[nMaxHistIdx].nEdRmsIdx - nHistOffset) * 10.0f;

	std::vector< jsl::Point3d<double, double, double> > dataList;
	dataList.reserve(wid*len);
	jsl::LeastSquare lsCalc;
	float stdHeight = (nMaxHistIdx - nHistOffset) * 10;
	for (int y = 0; y < len; y++)
	{
		UCHAR* pMaskRow = Mask.ptr<UCHAR>(y);

		for (int x = 0; x < wid; x++)
		{
			if (pMaskRow[x] &&
				(height[y*wid + x] != 0 && height[y*wid + x] != -200) &&
				(height[y*wid + x] > hSt && height[y*wid + x] < hEd) &&
				(fabs(Ygrad[y*wid + x]) < 5 && fabs(Xgrad[y*wid + x]) < 5))
			{
				jsl::Point3d<double, double, double> val(x, y, height[y*wid + x] * 0.001);
				dataList.emplace_back(val);
			}
		}
	}

	if (dataList.size() < 20)
		return;

	{
		std::vector<double> coeff;
		lsCalc.Run2d(1, dataList, coeff);

		if (coeff.size() < 3)
			return;

		double * pCoeff = &coeff[0];
		for (int y = 0; y < len; y++)
		{
			for (int x = 0; x < wid; x++)
			{
				height[y*wid + x] -= (pCoeff[0] * (double)(x)+pCoeff[1] * (double)(y)+pCoeff[2]) * 1000.0;
			}
		}
	}
}

int CPInsp_Foot::GetBinImage(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, cv::Mat* Image, cv::Rect ImageRect, int footKind, int left, int right, cv::Mat* Mmask, bool bMaxblob, cv::Mat* ReHImg, cv::Mat* cv3DOrg, int nMinblobArea, bool bUseFootOnly2D)
{
	AlgoFoot* sInspFoot = (AlgoFoot*)sInspAlgo.m_ptrInspAlgoParam;

	cv::Mat BinImage(Image->rows, right - left, CV_8UC1);
	cv::Mat srcImg = *Image;
	cv::Mat fHImg(Image->rows, Image->cols, CV_32FC1, sWndAlgoImg.m_fArr3D);
	if (cv3DOrg != NULL) fHImg = cv3DOrg->clone();
	cv::Mat fHRectImg = fHImg;
	cv::Mat RstImage(Image->rows, right - left, CV_8UC1);
	if (right - left < Image->cols)
	{
		srcImg = (*Image)(cv::Rect(left, 0, right - left, Image->rows)).clone();
		fHRectImg = fHImg(cv::Rect(left, 0, right - left, Image->rows)).clone();
		if (ReHImg != NULL) *ReHImg = fHRectImg.clone();
	}

#if _DEBUG
	cv::imwrite("D:\\FootRst\\GetBinImage_2D.bmp", srcImg);
	cv::imwrite("D:\\FootRst\\GetBinImage_3D.bmp", fHImg);
#endif

	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	UCHAR *pUcImgBinary = BinImage.data;
	if (sInspFoot->m_sArrBin[footKind].m_bIsSet == false)
		footKind = 0;

	double dArea, dCX, dCY;
	CRect rcBlob;
	AlgoBlob sAlgoBlob = SetAlgoBlob(sInspAlgo, footKind);

	if (bMaxblob)
		sAlgoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;
	dCX = 0, dCY = 0, dArea = 0;
	rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
	int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, srcImg.data, fHRectImg.ptr<float>(0), NULL, srcImg.cols, srcImg.rows, nMinblobArea, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
	BinImage.copyTo(*Image);
	if (sInspFoot->m_sArrBin[footKind].Moph)
	{
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Blob.bmp", BinImage);
#endif

		if (bMaxblob)
			sAlgoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

		if (Mmask != NULL)
		{
			BinImage &= (*Mmask);
		}
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_BlobM.bmp", BinImage);
#endif
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(BinImage, BinImage, kernel);
		cv::dilate(BinImage, BinImage, kernel);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Moph.bmp", BinImage);
#endif

		//return

		sAlgoBlob.m_nTypeRange2D = eTypeRangeUpper;
		sAlgoBlob.m_bInsp3D = false;
		sAlgoBlob.m_bInsp2D = true;
		sAlgoBlob.m_nMinBinary = 100;
		nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, BinImage.data, fHRectImg.ptr<float>(0), NULL, BinImage.cols, BinImage.rows, 4, &dArea, &dCX, &dCY, &rcBlob, RstImage.data, stTieAreaNULL, true);
		RstImage.copyTo(*Image);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Result.bmp", RstImage);
#endif
		//BinImage.copyTo(*Image);
	}
	if (pUcImgDst2D != NULL)
	{
		//delete pUcImgDst2D;
		g_pMManager->pem_delete(pUcImgDst2D, false);
		pUcImgDst2D = NULL;
	}
	if (pUcImgDst3D != NULL)
	{
		//delete pUcImgDst3D;
		g_pMManager->pem_delete(pUcImgDst3D, false);
		pUcImgDst3D = NULL;
	}
	return nCntBlob;
}
void CPInsp_Foot::DefectRectSet(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo)
{
	if (pFoot->m_RoRect != nullptr)
	{
		cv::Point2f padRect[4];
		pFoot->m_RoRect->center.x += pFoot->mWindowImageRect.left;
		pFoot->m_RoRect->center.y += pFoot->mWindowImageRect.top;
		pFoot->m_RoRect->points(padRect);

		sRstAlgo->m_nTeachRectLB.x = (int)std::roundf(padRect[0].x);
		sRstAlgo->m_nTeachRectLT.x = (int)std::roundf(padRect[1].x);
		sRstAlgo->m_nTeachRectRT.x = (int)std::roundf(padRect[2].x);
		sRstAlgo->m_nTeachRectRB.x = (int)std::roundf(padRect[3].x);

		sRstAlgo->m_nTeachRectLB.y = (int)std::roundf(padRect[0].y);
		sRstAlgo->m_nTeachRectLT.y = (int)std::roundf(padRect[1].y);
		sRstAlgo->m_nTeachRectRT.y = (int)std::roundf(padRect[2].y);
		sRstAlgo->m_nTeachRectRB.y = (int)std::roundf(padRect[3].y);
	}
	else
	{
		RECT FootShiftSpecArea = pFoot->m_PadRect;

		sRstAlgo->m_nTeachRectLT.x = (int)std::roundf(FootShiftSpecArea.left);
		sRstAlgo->m_nTeachRectLB.x = (int)std::roundf(FootShiftSpecArea.left);
		sRstAlgo->m_nTeachRectRB.x = (int)std::roundf(FootShiftSpecArea.right);
		sRstAlgo->m_nTeachRectRT.x = (int)std::roundf(FootShiftSpecArea.right);

		sRstAlgo->m_nTeachRectLT.y = (int)std::roundf(FootShiftSpecArea.top);
		sRstAlgo->m_nTeachRectLB.y = (int)std::roundf(FootShiftSpecArea.top);
		sRstAlgo->m_nTeachRectRB.y = (int)std::roundf(FootShiftSpecArea.bottom);
		sRstAlgo->m_nTeachRectRT.y = (int)std::roundf(FootShiftSpecArea.bottom);
	}
}
BOOL CPInsp_Foot::InspFoot(CFoot_Model* pFoot, WndAlgoImg& /*sWndAlgoImg*/, RstAlgoFoot * sRstAlgo, TotalInspExceptArea stTieArea, AlgoColorOpt &ptrAlgoColorOpt,
	int nfootKind, UCHAR *ucArrDstImg, AlignResult * arrAlignRes, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, CString sSaveDebugPath)
{
	BOOL ret = FALSE;
	int nLine = __LINE__;
	
	CString msg;
	msg.Format(_T("[FootInspAlgo]::InspFoot()_Start"));
	g_pMPTI->AddLog(msg);

	nLine = __LINE__;

	int nWndW = pFoot->mWindowImageRect.right - pFoot->mWindowImageRect.left;
	int nWndH = pFoot->mWindowImageRect.bottom - pFoot->mWindowImageRect.top;

	cv::Mat cvDBCPolygonImage(nWndH, nWndW, CV_8UC1, pFoot->m_pDBCPolygonImg);

	//USHORT* LabelImage = NULL;
	memset(&sRstAlgo->m_sEnd, 0, sizeof(POINTF));
	try
	{
#pragma region 1.Set Algo Values

		if (sSaveDebugPath.IsEmpty() != TRUE)
		{
			cstDebugFolderPath = sSaveDebugPath;
		}

		sRstAlgo->m_poWire_Wnd.x = pFoot->m_nWindowStartX;
		sRstAlgo->m_poWire_Wnd.y = pFoot->m_nWindowStartY;
		//Get Foot Search ROI Size
		int nWidth = pFoot->GetImageWidth();		//Foot Search Area 
		int nHeight = pFoot->GetImageLength();

		int nFootDirection = 0;
		nFootDirection = pFoot->m_nFootAngle;

		int nBlobCnt = 0;
		bool bIsFoundFoot = false;
		std::vector<cv::Rect> rcFootROIList;

		int nStartX = 0, nStartY = 0;
		nStartX = pFoot->mImageRect.left - (ptrAlgoColorOpt.m_sFovImg.nImageSizeX / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeX);
		nStartY = pFoot->mImageRect.top - (ptrAlgoColorOpt.m_sFovImg.nImageSizeY / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeY);
		if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeX <= 0) nStartX = pFoot->m_nWindowStartX + (pFoot->mImageRect.left - pFoot->mWindowImageRect.left);// pFoot->mImageRect.left;
		if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeY <= 0)nStartY = pFoot->m_nWindowStartY + (pFoot->mImageRect.top - pFoot->mWindowImageRect.top);// pFoot->mImageRect.top;

		if (nStartY < 0)
			nStartY = 0;

		cv::Rect ImageRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);
		cv::Mat Img(nHeight, nWidth, CV_8UC1), ImgWedge(nHeight, nWidth, CV_8UC1), ImgWing(nHeight, nWidth, CV_8UC1), Img3DArea(nHeight, nWidth, CV_32FC1);
		cv::Mat ImgTemp(nHeight, nWidth, CV_32FC1, pFoot->pf3D);
		ImgTemp.copyTo(Img3DArea);


		CString msg1;
		msg1.Format(_T("[FootInspAlgo]::InspFoot()_CalcPadBWImg"));
		g_pMPTI->AddLog(msg1);

		cv::Mat PadImg;
		Calc_padBinImage4(pFoot, sRstAlgo, ImageRect, &PadImg, Img3DArea, pFoot->bTeachBin);
		int nImgWidth = ImageRect.width;
		int nImgHeight = ImageRect.height;
		int imgSize = nImgWidth * nImgHeight;
		long pLebel[PTR_BLOB_MAX];
		//Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&LabelImage, imgSize);
		memset(pLebel, 0, sizeof(long) * PTR_BLOB_MAX);
		//memset(LabelImage, 0, sizeof(USHORT) * imgSize);

		cv::Mat cvWingCandi_SecondImg;
		FootPoins fPoints;
		std::vector<cv::Mat> InspImages;
		InspImages.clear();

		DefectRectSet(pFoot, sRstAlgo);
#pragma endregion
		nLine = __LINE__;
#pragma region 2. validity check

		CString msg2;
		msg2.Format(_T("[FootInspAlgo]::InspFoot()_FindFoot3DBlob"));
		g_pMPTI->AddLog(msg2);

		bool bUseFootOnly2D = true;
		cv::Mat cvRoFootRctImg(nHeight, nWidth, CV_8UC1);
		bIsFoundFoot = FindFoot_3DBlob(pFoot, sRstAlgo, &Img, &ImgWedge, &ImgWing, &PadImg, fPoints, nfootKind, cv3DAvgFilter, cvRoFootRctImg, cstDebugFolderPath, bUseFootOnly2D, false, true);

		cvWingCandi_SecondImg = cvRoFootRctImg.clone();

		/*
		nBlobCnt = FindFoot_Pat(pFoot, sRstAlgo, &Img, &ImgWedge, &ImgWing, &PadImg, fPoints, nfootKind);
		if (nBlobCnt > 0) bIsFoundFoot = true;

		if (nBlobCnt == 0)
		{
			sRstAlgo->m_bFind = FALSE;
			sRstAlgo->m_bArrOK[m_eFoot_Deviation] == FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);

				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;

			}
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("CPInsp_Foot::InspFoot BlobCount = 0 "));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return false;
		}
		*/

		if (!bIsFoundFoot)
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;
			sRstAlgo->m_bArrOK[m_eFoot_Deviation] == FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);

				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;

			}
			//Delete_1DArray(&LabelImage);
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("CPInsp_Foot::InspFoot 2. validity check !bIsFoundFoot"));
			g_pMPTI->AddLog_OCR(sLog);
#endif

#ifdef _AlgoTool_1
			cv::Mat cvFindFootRctImg(nHeight, nWidth, CV_8UC1);
			cvFindFootRctImg.setTo(255);
			g_pMPTI->m_InspMng->footDebugImg_CropData.push_back(std::make_pair(cvFindFootRctImg.clone(), "Deviation"));
#endif
//
			return false;
		}
		sRstAlgo->m_nFindBin = 0;
		//sRstAlgo->m_bOK = TRUE;

		float fPadAreaAverageHgt = 0.0f;
		fPadAreaAverageHgt = pFoot->m_fPadArea3DAvgHeight;
		sRstAlgo->m_fAreaHeight = fPadAreaAverageHgt;

#pragma endregion
		nLine = __LINE__;
#pragma region 3. foot Type Inspection

		CString msg3;
		msg3.Format(_T("[FootInspAlgo]::InspFoot()_FootPointByType"));
		g_pMPTI->AddLog(msg3);

		int nRet = FootPointByType(pFoot, sRstAlgo, fPoints, &Img, &ImgWedge, &ImgWing, &PadImg, ucArrDstImg, &cvWingCandi_SecondImg, InspImages, nStartX, nStartY, nfootKind, bWingRstVisible, cv3DAvgFilter);		//Detect Points From Candidates images 
		if (nRet < 0)
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;
			sRstAlgo->m_bArrOK[m_eFoot_Deviation] == FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);

				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;

			}
			//Delete_1DArray(&LabelImage);
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("CPInsp_Foot::InspFoot 3. foot Type Inspection nRet < 0"));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return false;
		}
		cv::Point CenterPos(0,0);
		CenterPos.x = fPoints.m_pCenter.x;
		CenterPos.y = fPoints.m_pCenter.y;
#pragma endregion
		nLine = __LINE__;

#if _DEBUG
		//디버깅 이미지 저장
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_BackGroundBWImg.bmp"), PadImg);
			cv::imwrite(rawname + std::string("_FootBWImg.bmp"), Img);
		}
#endif

		//디버깅 이미지 저장(sSaveDebugPath)  part 넘버,Win 순서, Algo 넘버 순으로
		if (sSaveDebugPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(sSaveDebugPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_FootOrgImg.bmp"), Img);
		}

#pragma region 4. create foot Images

		CString msg4;
		msg4.Format(_T("[FootInspAlgo]::InspFoot()_FootImageByPoint"));
		g_pMPTI->AddLog(msg4);

		cv::Point Margin = FootImageByPoint(pFoot, fPoints, InspImages);		//Clip image using points (output images are real sub area)

		//GetBlob_BondingFloor(pFoot, sRstAlgo, nBlobCnt, pLebel, LabelImage, &Img, &ImgWedge, &ImgWing, &Img3DArea, rcFootROIList, nWidth, nHeight);

		//debug code
#if _DEBUG
		CString path = g_pMPTI->GetWireDebugDataFullPath(nfootKind, _T("FootRst"), _T("WedgeBin"), 0);
		m_pProcMilAlgo->SaveDebugImg(ImgWedge.ptr(), nWidth, nHeight, path, 1, FALSE);

		path = g_pMPTI->GetWireDebugDataFullPath(nfootKind, _T("FootRst"), _T("WingBin"), 0);
		m_pProcMilAlgo->SaveDebugImg(ImgWing.ptr(), nWidth, nHeight, path, 1, FALSE);

		path = g_pMPTI->GetWireDebugDataFullPath(nfootKind, _T("FootRst"), _T("FootBin"), 0);
		m_pProcMilAlgo->SaveDebugImg(Img.ptr(), nWidth, nHeight, path, 1, FALSE);
#endif

#pragma endregion
		nLine = __LINE__;
#pragma region 5. calculate items

		//InspImages.push_back(Img);//Deviation image
		//5 to 8

		CString msg5;
		msg5.Format(_T("[FootInspAlgo]::InspFoot()_FootMeasure"));
		g_pMPTI->AddLog(msg5);

		sRstAlgo->m_nFindBin = 1;
		cv::Point Center = FootMeasure(pFoot, sRstAlgo, InspImages, ucArrDstImg, &cvWingCandi_SecondImg, CenterPos, fPoints.m_dSeta, Margin, nStartX, nStartY, nfootKind, bWingRstVisible, cv3DAvgFilter, &PadImg, &Img);

#pragma endregion
		nLine = __LINE__;
#pragma region 6. end Function


		if (sRstAlgo->m_bFind) // Pad 영역 설정
		{

			POINT pt;
			pt.x = Center.x;
			pt.y = Center.y;

			CRect rect;
			rect.left = pFoot->m_PadRect.left;
			rect.top = pFoot->m_PadRect.top;
			rect.right = pFoot->m_PadRect.right;
			rect.bottom = pFoot->m_PadRect.bottom;
			if (rect.PtInRect(pt))
			{
				int nstX = pFoot->mImageRect.left - pFoot->mWindowImageRect.left; //Window 내의 Image ROI 시작 좌표
				int nstY = pFoot->mImageRect.top - pFoot->mWindowImageRect.top;

				sRstAlgo->m_rcSearchArea.left = rect.left + nstX;
				sRstAlgo->m_rcSearchArea.top = rect.top + nstY;
				sRstAlgo->m_rcSearchArea.right = rect.right + nstX;
				sRstAlgo->m_rcSearchArea.bottom = rect.bottom + nstY;
			}
		}


		nLine = __LINE__;
		//WIndow LT 기준으로 Position 변경
		int nImageStartXInWindowLT = pFoot->GetImageStartXInWindowLT();
		int nImageStartYInWindowLT = pFoot->GetImageStartYInWindowLT();

		for (int poIdx = 0; poIdx < m_eFoot::m_eFoot_Total; poIdx++)
		{

			if (sRstAlgo->rePoinSt[poIdx].x != 0 || sRstAlgo->rePoinSt[poIdx].y != 0
				|| sRstAlgo->rePoinEd[poIdx].x != 0 || sRstAlgo->rePoinEd[poIdx].y != 0)
			{
				//WindowLT 기준으로 변경
				sRstAlgo->rePoinSt[poIdx].x = sRstAlgo->rePoinSt[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->rePoinSt[poIdx].y = sRstAlgo->rePoinSt[poIdx].y + nImageStartYInWindowLT;

				sRstAlgo->rePoinEd[poIdx].x = sRstAlgo->rePoinEd[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->rePoinEd[poIdx].y = sRstAlgo->rePoinEd[poIdx].y + nImageStartYInWindowLT;


				sRstAlgo->DispPoinSt[poIdx].x = sRstAlgo->DispPoinSt[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->DispPoinSt[poIdx].y = sRstAlgo->DispPoinSt[poIdx].y + nImageStartYInWindowLT;

				sRstAlgo->DispPoinEd[poIdx].x = sRstAlgo->DispPoinEd[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->DispPoinEd[poIdx].y = sRstAlgo->DispPoinEd[poIdx].y + nImageStartYInWindowLT;
			}
		}

		if (sRstAlgo->m_sPoint.x != 0 || sRstAlgo->m_sPoint.y != 0)
		{
			sRstAlgo->m_sPoint.x = sRstAlgo->m_sPoint.x + nImageStartXInWindowLT;
			sRstAlgo->m_sPoint.y = sRstAlgo->m_sPoint.y + nImageStartYInWindowLT;
		}
		DefectRectSet(pFoot, sRstAlgo);

		nLine = __LINE__;
		for (int algoCnt = 0; algoCnt < (int)m_eFoot::m_eFoot_Bonding1; algoCnt++)
		{
			//if (sRstAlgo->m_bOK == FALSE)
				//break;
			int footDataType = convertToFootData(algoCnt);
			if ((pFoot->m_nUseOption & footDataType) != footDataType)
				continue;
			if (sRstAlgo->m_bArrOK[algoCnt] == FALSE)
				sRstAlgo->m_bOK = FALSE;
		}
		ret = sRstAlgo->m_bOK;

		//Delete_1DArray(&LabelImage);
#pragma endregion


	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::InspFoot error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		//if(LabelImage != NULL)
		//	Delete_1DArray(&LabelImage);
		return ret;
	}

	return ret;
}

bool CPInsp_Foot::Insp_FootShift_removedWire(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFoot, cv::Point *pWingBondingEd)
{
	int nLine = __LINE__;
	try
	{
		float shift, fshiftX, fshiftY;

		//Foot 이미지 회전(Tail 이 위로 오게)
		cv::Point LengRSt, LengREd;
		cv::Point CenterPos = NULL;
		CenterPos.x = (int)sRstAlgo->m_sPoint.x;
		CenterPos.y = (int)sRstAlgo->m_sPoint.y;
		float setaFoot = sRstAlgo->m_fAngle;

		if (CenterPos.x == 0 && CenterPos.y == 0)
		{
			return false;
		}

		cv::Mat FootClipImg = (*cvFoot);
		cv::Mat FootClipRotImg;
		//cv::Point FootMargin = WarpAffine(FootClipImg, FootClipRotImg, setaFoot, FootClipImg.size(), CenterPos);		//Foot Img 회전
		cv::Point FootMargin = WarpAffine_IPPRotate(FootClipImg, FootClipRotImg, setaFoot, FootClipImg.size(), CenterPos);		//Foot Img 회전

		//cv::Mat WingClipImg = ImgWing;
		//cv::Mat WingClipRotImg;
		//cv::Point WingMargin = WarpAffine(WingClipImg, WingClipRotImg, setaFoot, Img.size(), CenterPos);		//Wing Img 회전

		//Wing 의 Bonding End 지점(LWingEd) 찾아서 Foot 영역 아래의 Wire부분 삭제
		for (int y = pWingBondingEd->y; y < FootClipImg.rows; y++)
		{
			uchar *data = FootClipImg.ptr<uchar>(y);

			for (int x = 0; x < FootClipImg.cols; x++)
			{
				if (data[x] == 255)
				{
					data[x] = 0;
				}
			}
		}

		cv::Mat FootRstRoImg;
		cv::Mat FootOrg = cv::Mat(FootClipImg.rows, FootClipImg.cols, CV_8UC1);

		//Wire부분 자른 Foot 이미지 다시 원위치로 회전
		cv::Point CenterPos_Revert = NULL;
		CenterPos_Revert.x = CenterPos.x -FootMargin.x;
		CenterPos_Revert.y = CenterPos.y -FootMargin.y;
		//WarpAffine(FootClipImg, FootRstRoImg, -setaFoot, FootClipImg.size(), CenterPos_Revert);	//최종 이미지 원상복귀를 위한 회전
		WarpAffine_IPPRotate(FootClipImg, FootRstRoImg, -setaFoot, FootClipImg.size(), CenterPos_Revert);	//최종 이미지 원상복귀를 위한 회전

		//max Blob 처리 - foot부분 이외 이물이 함께 이진화 된 경우를 대비
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(FootRstRoImg, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
		//

		std::vector<std::vector<cv::Point> >hull(contours.size());
		for (size_t i = 0; i < contours.size(); i++)
		{
			convexHull(cv::Mat(contours[i]), hull[i], false);
		}

		int idx = 0;

		cv::Mat FootRstRoImg_maxBlob(FootRstRoImg.size(), CV_8UC1, cv::Scalar(0));
		std::vector<int> bolbCandidate;
		for (contours.begin(); idx < contours.size(); idx++)
		{
			drawContours(FootRstRoImg_maxBlob, contours, idx, cv::Scalar(255), cv::FILLED, 8, hierarchy);
		}


		cv::Mat FootRstRoImg_maxBlob2(FootRstRoImg_maxBlob.size(), CV_8UC1, cv::Scalar(0));
		int maxIdx = 0;
		int NearestCenterPosIdx = 0;
		int NearestMinDistance = FootRstRoImg_maxBlob.cols * FootRstRoImg_maxBlob.rows;
		for (contours.begin(); maxIdx < contours.size(); maxIdx++)
		{
			cv::Rect bbox = cv::boundingRect(contours[maxIdx]);
			double area = cv::contourArea(contours[maxIdx]);	////객체 사이즈(픽셀개수)

			if (area > 100)
			{
				//NearestCenterPosIdx = maxIdx;

				int blobCenterX = bbox.x + (bbox.width / 2);
				int blobCenterY = bbox.y + (bbox.height / 2);

				int dist = sqrt(pow(blobCenterX - CenterPos_Revert.x, 2) + pow(blobCenterY - CenterPos_Revert.y, 2));

				if (NearestMinDistance > dist)
				{
					NearestMinDistance = dist;
					NearestCenterPosIdx = maxIdx;
				}


			}

		}

		drawContours(FootRstRoImg_maxBlob2, contours, NearestCenterPosIdx, cv::Scalar(255), cv::FILLED, 8, hierarchy);


		//Wire 부분 자른 Foot의 Rect 영역 계산
		int ncvImgWidth = FootRstRoImg_maxBlob2.cols;
		int ncvImgHeight = FootRstRoImg_maxBlob2.rows;
		int nFootTmpMin_x = ncvImgWidth * ncvImgHeight, nFootTmpMax_x = 0;
		int nFootTmpMin_y = ncvImgWidth * ncvImgHeight, nFootTmpMax_y = 0;
		for (int r = 0; r < FootRstRoImg_maxBlob2.rows; r++)
		{
			UCHAR* Ptr = FootRstRoImg_maxBlob2.data;
			for (int c = 0; c < FootRstRoImg_maxBlob2.cols; c++)
			{
				if (Ptr[r*FootRstRoImg_maxBlob2.step + c] == 255)
				{
					if (c < nFootTmpMin_x) nFootTmpMin_x = c;
					if (c > nFootTmpMax_x) nFootTmpMax_x = c;
					if (r < nFootTmpMin_y) nFootTmpMin_y = r;
					if (r > nFootTmpMax_y) nFootTmpMax_y = r;
				}
			}
		}

		RECT rtFootRectRemovedWire;

		// Wire부분 자른 Foot의 정보가 없을 시 함수 리턴
		if (nFootTmpMin_x == ncvImgWidth * ncvImgHeight && nFootTmpMax_x == 0 &&
			nFootTmpMin_y == ncvImgWidth * ncvImgHeight && nFootTmpMax_y == 0)
		{
			return false;
		}
		else
		{
			rtFootRectRemovedWire.left = nFootTmpMin_x;
			rtFootRectRemovedWire.right = nFootTmpMax_x;
			rtFootRectRemovedWire.top = nFootTmpMin_y;
			rtFootRectRemovedWire.bottom = nFootTmpMax_y;
		}


		//회전 영상의 중심점과 Rect 시작위치의 차이 값 계산
		int diffFootCX = CenterPos_Revert.x - nFootTmpMin_x;
		int diffFootCY = CenterPos_Revert.y - nFootTmpMin_y;

		//Wire부분 자른 Foot의 Rect 영역 자른 이미지
		cv::Mat FootRectRemovedWire = FootRstRoImg_maxBlob2(cv::Rect(nFootTmpMin_x, nFootTmpMin_y, nFootTmpMax_x - nFootTmpMin_x, nFootTmpMax_y - nFootTmpMin_y));

		int FootRectStX = 0, FootRectStY = 0;

		if (CenterPos.x - diffFootCX < 0)
			FootRectStX = 0;
		else
			FootRectStX = CenterPos.x - diffFootCX;

		if (CenterPos.y - diffFootCY < 0)
			FootRectStY = 0;
		else
			FootRectStY = CenterPos.y - diffFootCY;

		if (FootRectStX + FootRectRemovedWire.cols > FootOrg.cols)
			FootRectStX = FootOrg.cols - FootRectRemovedWire.cols;

		if (FootRectStY + FootRectRemovedWire.rows > FootOrg.rows)
			FootRectStY = FootOrg.rows - FootRectRemovedWire.rows;

		//InspRect에 맞게 Wire부분 자른 Foot 영상 생성
		FootOrg.setTo(0);
		cv::Mat A1 = FootOrg(cv::Rect(FootRectStX, FootRectStY, FootRectRemovedWire.cols, FootRectRemovedWire.rows));
		FootRectRemovedWire.copyTo(A1);

		//Wire부분 자른 Foot의 Rect 영역 계산
		ncvImgWidth = FootOrg.cols;
		ncvImgHeight = FootOrg.rows;
		int nFootMin_x = ncvImgWidth * ncvImgHeight, nFootMax_x = 0;
		int nFootMin_y = ncvImgWidth * ncvImgHeight, nFootMax_y = 0;
		int nDeviationPoint[4][2];// 4방향에 대한 x,y 값 
		for (int r = 0; r < FootOrg.rows; r++)
		{
			UCHAR* Ptr = FootOrg.data;
			for (int c = 0; c < FootOrg.cols; c++)
			{
				if (Ptr[r*FootOrg.step + c] == 255)
				{
					
					if (c < nFootMin_x) { nDeviationPoint[0][0] = nFootMin_x = c;  nDeviationPoint[0][1] = r; } //left
					if (c > nFootMax_x) { nDeviationPoint[1][0] = nFootMax_x = c; nDeviationPoint[1][1] = r;	} // right
					if (r < nFootMin_y) { nDeviationPoint[2][0] = c; nDeviationPoint[2][1] = nFootMin_y = r;	} // bottom
					if (r > nFootMax_y) { nDeviationPoint[3][0] = c; nDeviationPoint[3][1] = nFootMax_y = r;	} // top
				}
			}
		}

		// Wire부분 자른 Foot의 Rect 정보가 없을 시 함수 리턴
		if (nFootMin_x == ncvImgWidth * ncvImgHeight && nFootMax_x == 0 &&
			nFootMin_y == ncvImgWidth * ncvImgHeight && nFootMax_y == 0)
		{
			return false;
		}
		
		RECT FootShiftInspArea;
		FootShiftInspArea.left = nFootMin_x + pFoot->mImageRect.left;
		FootShiftInspArea.right = nFootMax_x + pFoot->mImageRect.left;
		FootShiftInspArea.top = nFootMin_y + pFoot->mImageRect.top;
		FootShiftInspArea.bottom = nFootMax_y + pFoot->mImageRect.top;
		
		//SpecRect 영역과 (Wire부분 자른)Foot의 Rect 영역 사이의 Shift값 계산
		//Rect 영역(FootShiftInspArea)이 SpecRect 벗어나는 지 확인 및 벗어나는 정도(shift) 계산
		double dShiftFootx = 0;
		double dShiftFooty = 0;
		RECT FootSpecRect; 

		FootSpecRect.left = pFoot->m_PadRect.left - pFoot->mWindowImageRect.left;
		FootSpecRect.top = pFoot->m_PadRect.top - pFoot->mWindowImageRect.top;
		FootSpecRect.right = pFoot->m_PadRect.right - pFoot->mWindowImageRect.left;
		FootSpecRect.bottom = pFoot->m_PadRect.bottom - pFoot->mWindowImageRect.top;

#if _DEBUG
		//디버깅용 이미지 저장
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::Mat cvFootAlgoImageRect(pFoot->mImageRect.bottom - pFoot->mImageRect.top, pFoot->mImageRect.right - pFoot->mImageRect.left, CV_8UC1);
			cvFootAlgoImageRect.setTo(0);

			cvFootAlgoImageRect |= FootOrg;

			cv::rectangle(cvFootAlgoImageRect,
				cv::Rect(pFoot->m_PadRect.left - pFoot->mImageRect.left,
					pFoot->m_PadRect.top - pFoot->mImageRect.top,
					pFoot->m_PadRect.right - pFoot->m_PadRect.left,
					pFoot->m_PadRect.bottom - pFoot->m_PadRect.top)
				, cv::Scalar(125, 125, 125), 1, 4, 0);

			cv::imwrite(rawname + std::string("_Foot_deviation.bmp"), cvFootAlgoImageRect);

		}
#endif

		RECT FootShiftSpecArea = pFoot->m_PadRect;

		if (FootShiftSpecArea.left > FootShiftInspArea.left || FootShiftSpecArea.right < FootShiftInspArea.right ||
			FootShiftSpecArea.top > FootShiftInspArea.top || FootShiftSpecArea.bottom < FootShiftInspArea.bottom)
		{
			if (FootShiftSpecArea.left > FootShiftInspArea.left)
			{
				dShiftFootx = -(FootShiftSpecArea.left - FootShiftInspArea.left);
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].x = nDeviationPoint[0][0] + abs((int)dShiftFootx); //방향별로 NgViewer Display 를위한 Shift Point 
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].y = nDeviationPoint[0][1];
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].x = nDeviationPoint[0][0];
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].y = nDeviationPoint[0][1];
			}
			else if (FootShiftSpecArea.right < FootShiftInspArea.right)
			{
				dShiftFootx = FootShiftInspArea.right - FootShiftSpecArea.right;
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].x = nDeviationPoint[1][0] - abs((int)dShiftFootx);
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].y = nDeviationPoint[1][1];
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].x = nDeviationPoint[1][0];
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].y = nDeviationPoint[1][1];
			}

			if (FootShiftSpecArea.top > FootShiftInspArea.top)
			{
				dShiftFooty = (FootShiftSpecArea.top - FootShiftInspArea.top);
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].x = nDeviationPoint[2][0];
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].y = nDeviationPoint[2][1] + abs((int)dShiftFooty);
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].x = nDeviationPoint[2][0];
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].y = nDeviationPoint[2][1];
			}
			else if (FootShiftSpecArea.bottom < FootShiftInspArea.bottom)
			{
				dShiftFooty = -(FootShiftInspArea.bottom - FootShiftSpecArea.bottom);
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].x = nDeviationPoint[3][0];
				sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].y = nDeviationPoint[3][1] - abs((int)dShiftFooty);
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].x = nDeviationPoint[3][0];
				sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].y = nDeviationPoint[3][1];
			}
		}

		fshiftX = abs((float)(m_resolX * dShiftFootx));
		fshiftY = abs((float)(m_resolY * dShiftFooty));
		shift = std::sqrtf(std::powf(fshiftX, 2) + std::powf(fshiftY, 2));

		if (fshiftX >= fshiftY)
		{
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].x;
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].y;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].x;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].y;
		}
		else
		{
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].x;
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].y;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].x;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].y;
		}
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationX, fshiftX);
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationY, fshiftY);
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_Deviation, shift);
		

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::Insp_FootShift_removedWire() Line %d Pass"), nLine);
		g_pMPTI->AddLog(msg);
	}

	return true;
}

bool CPInsp_Foot::Insp_FootShift_ImgAnd(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFoot, cv::Mat * cvPad, cv::Mat * cvBondingRect, cv::Point cvPoNewCenter, float &shift, float &fshiftX, float &fshiftY)
{
	int nLine = __LINE__;
	try
	{
		//float shift, fshiftX, fshiftY;
		cv::Point CenterPos = NULL;
		CenterPos.x = (int)sRstAlgo->m_sPoint.x;
		CenterPos.y = (int)sRstAlgo->m_sPoint.y;
		if ((CenterPos.x == 0 && CenterPos.y == 0)
			|| cvFoot == nullptr || cvPad == nullptr || cvFoot->data == NULL || cvPad == NULL)
		{
			return false;
		}

		cv::Mat cvRotate3DArea;
		//cv::Mat Shift_Img_Ro = (*cvBondingRect) - (*cvPad), Shift_Img;
		cv::Mat Shift_Img_Ro, Shift_Img;

		int nWingRectMinX = cvFoot->cols;
		int nWingRectMaxX = 0;

		int nWingRectMinY = cvFoot->rows;
		int nWingRectMaxY = 0;

		for (int r = 0; r < cvFoot->rows; r++)
		{
			UCHAR* Ptr = (*cvBondingRect).data;
			for (int c = 0; c < cvFoot->cols; c++)
			{
				if (Ptr[r * cvFoot->step + c] == 255)
				{
					if (r < nWingRectMinY)	nWingRectMinY = r;
					if (r > nWingRectMaxY)	nWingRectMaxY = r;

					if (c < nWingRectMinX) nWingRectMinX = c;
					if (c > nWingRectMaxX) nWingRectMaxX = c;

				}
			}
		}

		cv::Mat cvInspShiftImg(cvFoot->rows, cvFoot->cols, CV_8UC1);
		cvInspShiftImg.setTo(0);

		if (nWingRectMinY != cvFoot->rows)
		{
			for (int r = 0; r < cvFoot->rows; r++)
			{
				UCHAR* PtrBonding = (*cvBondingRect).data;
				UCHAR* PtrFootTail = cvFoot->data;
				UCHAR* PtrShiftImg = cvInspShiftImg.data;

				for (int c = 0; c < cvFoot->cols; c++)
				{
					if (PtrBonding[r * cvFoot->step + c] == 255)
					{
						PtrShiftImg[r * cvFoot->step + c] = 255;
					}
					if (PtrFootTail[r * cvFoot->step + c] == 255 && r < nWingRectMinY)
					{
						PtrShiftImg[r * cvFoot->step + c] = 255;
					}
				}
			}

			Shift_Img_Ro = cvInspShiftImg.clone();
			Shift_Img_Ro = Shift_Img_Ro - (*cvPad);
		}
		else
		{
			Shift_Img_Ro = (*cvBondingRect) - (*cvPad);
		}

		cv::Mat cv3DArea(cvFoot->rows, cvFoot->cols, CV_32FC1, pFoot->pf3D);
		cv::Point Margin = WarpAffine_IPPRotate_3D(cv3DArea, cvRotate3DArea, sRstAlgo->m_fAngle, Shift_Img_Ro.size(), cvPoNewCenter, false, true);		////Foot Img 회전
		
		cv::Point pMargin = WarpAffine_IPPRotate(Shift_Img_Ro, Shift_Img, -sRstAlgo->m_fAngle, Shift_Img_Ro.size(), cvPoNewCenter, false, true);	//최종 이미지 원상복귀를 위한 회전

		CenterPos.x = (int)round((Shift_Img_Ro.cols - 1) / 2);
		CenterPos.y = (int)round((Shift_Img_Ro.rows - 1) / 2);

		double dArea, dCX, dCY;
		CRect rcBlob;

		AlgoBlob algoBlob;
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

		int Range3D = 0;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_bInsp3D = false;
		algoBlob.m_dHeightRateMin = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
		algoBlob.m_dHeightRateMax = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];

		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 100;
		algoBlob.m_nMaxBinary = 100;

		algoBlob.m_bFillHole = false;
		algoBlob.m_bFilterIsUse = false;

		TotalInspExceptArea stTieAreaNULL;
		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(Shift_Img.data, NULL, Shift_Img.cols, Shift_Img.rows, 1, 0, true, 0, algoBlob.m_nTypeSelectBlob);

#if _DEBUG
		CString ImagePath;
		ImagePath.Format(_T("D:\\FootInspImg\\PadDeviationImg.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), *cvPad);   
#endif

		fshiftX = 0.0f;
		fshiftY = 0.0f;
		shift = 0.0f;
		if (nCntBlob > 0)
		{
			std::vector<CRect> rcRect;
			CRect MaxRect(0,0,0,0);
			int nMaxAreaRect(0);
			m_pProcMilAlgo->GetBlobResult_RectsV(rcRect);

			for (int i = 0; i < rcRect.size(); i++)
			{
				int nAreaRect = rcRect[i].Width()*rcRect[i].Height();
				if (nMaxAreaRect < nAreaRect)
				{
					nMaxAreaRect = nAreaRect;
					MaxRect = rcRect[i];
				}
			}

			//int nWidthR = pFoot->GetImageWidth();
			//int nHeightR = pFoot->GetImageLength();
			//cv::Mat cvPad2DImg(nHeightR, nWidthR, CV_8UC1);
			//pFoot->GetImageMatrix(&cvPad2DImg, (int)m_eFootBin::m_eFootBin_Pad);

			cv::Mat cvRotatePadRstMask = cv::Mat(cvFoot->rows, cvFoot->cols, CV_8UC1);
			cvRotatePadRstMask.setTo(0);
			double dAngle = (double)sRstAlgo->m_fAngle;
			//bool bRoMaskImgOrg = RotateBackToOriginalPosition(pFoot, *cvPad, CenterPos, pMargin, dAngle, &cvRotatePadRstMask);
			cv::Point pMargin = WarpAffine_IPPRotate(*cvPad, cvRotatePadRstMask, -sRstAlgo->m_fAngle, Shift_Img_Ro.size(), cvPoNewCenter, false, true);	//최종 이미지 원상복귀를 위한 회전

			int nPadRectMinX = cvRotatePadRstMask.cols;
			int nPadRectMaxX = 0;
			int nPadRectMinY = cvRotatePadRstMask.rows;
			int nPadRectMaxY = 0;

			for (int r = 0; r < cvRotatePadRstMask.rows; r++)
			{
				UCHAR* Ptr = cvRotatePadRstMask.data;
				for (int c = 0; c < cvRotatePadRstMask.cols; c++)
				{
					if (Ptr[r * cvRotatePadRstMask.step + c] == 255)
					{
						if (r < nPadRectMinY)	nPadRectMinY = r;
						if (r > nPadRectMaxY)	nPadRectMaxY = r;
						if (c < nPadRectMinX)	nPadRectMinX = c;
						if (c > nPadRectMaxX)	nPadRectMaxX = c;

					}
				}
			}

			bool bIsDBCFoot = false;

			if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
				bIsDBCFoot = true;

			if(bIsDBCFoot)
			{
				if (MaxRect.top <= nPadRectMinY - 5)
				{

					{
						fshiftY = MaxRect.Height() * m_resolY;

						float fWidth = MaxRect.Width() * m_resolX;
						shift = std::sqrtf(std::powf(fWidth, 2) + std::powf(fshiftY, 2));

						fshiftY = -fshiftY;
					}
				}
				else if (MaxRect.top >= nPadRectMaxY - 5)
				{

					{
						fshiftY = MaxRect.Height() * m_resolY;

						float fWidth = MaxRect.Width() * m_resolX;
						shift = std::sqrtf(std::powf(fWidth, 2) + std::powf(fshiftY, 2));						
					}
				}
				else if(MaxRect.bottom >= nPadRectMaxY - 5)
				{
					{
						fshiftY = MaxRect.Height() * m_resolY;

						float fWidth = MaxRect.Width() * m_resolX;
						shift = std::sqrtf(std::powf(fWidth, 2) + std::powf(fshiftY, 2));
					}
				}


				if (MaxRect.left <= nPadRectMinX - 5)
				{
					{
						fshiftX = MaxRect.Width()* m_resolX;

						float fHeight = MaxRect.Height() * m_resolY;
						shift = std::sqrtf(std::powf(fshiftX, 2) + std::powf(fHeight, 2));

						fshiftX = -fshiftX;
					}
				}
				else if (MaxRect.left >= nPadRectMaxX - 5)
				{
					fshiftX = MaxRect.Width()* m_resolX;

					float fHeight = MaxRect.Height() * m_resolY;
					shift = std::sqrtf(std::powf(fshiftX, 2) + std::powf(fHeight, 2));

				}
				else if (MaxRect.right >= nPadRectMaxX - 5)
				{
					fshiftX = MaxRect.Width()* m_resolX;

					float fHeight = MaxRect.Height() * m_resolY;
					shift = std::sqrtf(std::powf(fshiftX, 2) + std::powf(fHeight, 2));

				}

			}
			else
			{
				if (std::abs(MaxRect.CenterPoint().x - CenterPos.x) < std::abs(MaxRect.CenterPoint().y - CenterPos.y))
				{
					float fWidth = MaxRect.Width() * m_resolX;
					fshiftY = MaxRect.Height() * m_resolY;
					shift = std::sqrtf(std::powf(fWidth, 2) + std::powf(fshiftY, 2));
				}
				else
				{
					fshiftX = MaxRect.Width()* m_resolX;
					float fHeight = MaxRect.Height() * m_resolY;
					shift = std::sqrtf(std::powf(fshiftX, 2) + std::powf(fHeight, 2));
				}

				if (Shift_Img.cols / 2 > MaxRect.left && fshiftX != 0)
				{
					fshiftX = -fshiftX;
				}

				if (Shift_Img.rows / 2 > MaxRect.top && fshiftY != 0)
				{
					fshiftY = -fshiftY;

				}
			}

		}

		if (abs(fshiftX) >= abs(fshiftY))
		{
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].x;
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationX].y;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].x;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationX].y;
		}
		else
		{
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].x;
			sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_DeviationY].y;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].x = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].x;
			sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Deviation].y = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_DeviationY].y;
		}
// 		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationX, fshiftX);
// 		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationY, fshiftY);
// 		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_Deviation, shift);
// 
// 		//Foot이 이미지를 넘겼을때 예외처리
// 		if (sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Tail_L].x == 0 || sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Tail_L].x >= Shift_Img.cols - 1
// 			|| sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Tail_L].y == 0 || sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Tail_L].y >= Shift_Img.rows - 1)
// 		{
// 			sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
// 		}

		//Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_Deviation, shift);

		//if (shift != 0.0 || abs(fshiftX) !=0.0 || abs(fshiftY) != 0.0)
		//{
		//	CTime t = CTime::GetCurrentTime();
		//	CString cstrCurTime;
		//	cstrCurTime.Format(_T("%s_NG.bmp"), t.Format(_T("%y.%m.%d_%Hh.%Mm.%Ss")));

		//	CString ImagePath2;
		//	ImagePath2.Format(_T("D:\\FootRst\\PadDeviationImg_%s.bmp"), cstrCurTime);
		//	cv::imwrite(std::string(CT2A(ImagePath2)), *cvPad);

		//	ImagePath2.Format(_T("D:\\FootRst\\BondingRect_%s.bmp"), cstrCurTime);
		//	cv::imwrite(std::string(CT2A(ImagePath2)), *cvBondingRect);

		//	ImagePath2.Format(_T("D:\\FootRst\\Shift_RstImg_%.3f_%s.bmp"), shift, cstrCurTime);
		//	cv::imwrite(std::string(CT2A(ImagePath2)), Shift_Img_Ro);

		//	int nWidthR = pFoot->GetImageWidth();
		//	int nHeightR = pFoot->GetImageLength();
		//	cv::Mat cvPad2DImg(nHeightR, nWidthR, CV_8UC1);
		//	pFoot->GetImageMatrix(&cvPad2DImg, (int)m_eFootBin::m_eFootBin_Pad);
		//	cv::Point cvCenter(sRstAlgo->m_sPoint.x, sRstAlgo->m_sPoint.y);
		//	cv::Point pMarginImg = WarpAffine_IPPRotate(cvPad2DImg, cvPad2DImg, sRstAlgo->m_fAngle, cvPad2DImg.size(), cvCenter);

		//	cv::Mat RstPadUIimage, testPadimage1, RstPadUIimage2;
		//	cv::cvtColor(cvPad2DImg, testPadimage1, cv::COLOR_GRAY2RGB);

		//	cv::Mat testPadEdgeimage1;
		//	cv::Mat scale1;
		//	cv::Canny(*cvPad, scale1, 100, 200, 3, false);
		//	cv::cvtColor(scale1, testPadEdgeimage1, cv::COLOR_GRAY2RGB);
		//	cv::Mat cvRstShift;
		//	cv::cvtColor(Shift_Img_Ro, cvRstShift, cv::COLOR_GRAY2RGB);

		//	double alpha = 0.5;
		//	cv::addWeighted(testPadimage1, alpha, testPadEdgeimage1, (1 - alpha), 0, RstPadUIimage);
		//	cv::addWeighted(cvRstShift, alpha, RstPadUIimage, (1 - alpha), 0, RstPadUIimage2);

		//	//cv::imwrite(rawname + std::string("_cvFootPadEdge.bmp"), RstPadUIimage);
		//	ImagePath2.Format(_T("D:\\FootRst\\Shift_PadEdgeImg_%s.bmp"), cstrCurTime);
		//	cv::imwrite(std::string(CT2A(ImagePath2)), RstPadUIimage2);
		//}

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::Insp_FootShift_ImgAnd() Line %d Pass"), nLine);
		g_pMPTI->AddLog(msg);
	}

	return true;
}
bool CPInsp_Foot::Insp_FootShift(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<int> startPad, std::vector<int> endPad, std::vector<int> startPadY, std::vector<int> endPadY
	, std::vector<int> Left, std::vector<int> Top, std::vector<int> Right, std::vector<int> Bot, std::vector<int> LeftIdx)
{
	int nLine = __LINE__;
	try
	{
		for (int footIdx = 0; footIdx < 1 && footIdx < (int)LeftIdx.size(); footIdx++)
		{
			if (sRstAlgo->m_sPoint.x == 0 && sRstAlgo->m_sPoint.y == 0)
				continue;
			int teachpad(0), findpad(0);
			for (int vp = 0; vp < endPad.size(); vp++)
			{
				if (startPad[vp]< sRstAlgo->m_sPoint.x && endPad[vp] > sRstAlgo->m_sPoint.x)
				{
					findpad = vp;
					break;
				}
			}
			if (footIdx >= LeftIdx.size())
			{
				continue;
			}
			if (LeftIdx[footIdx] >= Left.size())
			{
				continue;
			}
			//나가지 않았는가?

			cv::Point L1St;
			cv::Point L2St;
			cv::Point L1Ed;
			cv::Point L2Ed;
			L1St.x = Left[LeftIdx[footIdx]];
			L1St.y = Top[LeftIdx[footIdx]];
			L2St.x = Right[LeftIdx[footIdx]];
			L2St.y = Top[LeftIdx[footIdx]];
			L1Ed.x = Left[LeftIdx[footIdx]];
			L1Ed.y = Bot[LeftIdx[footIdx]];
			L2Ed.x = Right[LeftIdx[footIdx]];
			L2Ed.y = Bot[LeftIdx[footIdx]];

			cv::Point DBCSt, DBCEd;
			DBCSt.x = startPad[findpad];
			DBCSt.y = startPadY[0];
			DBCEd.x = endPad[findpad];
			DBCEd.y = endPadY[0];

			bool bIsShift = false;
			int footStartType(0);
			float fAngle = sRstAlgo->m_fAngle;
			if (fAngle > 360)
				fAngle -= 360;

			if (fAngle >= 80 && fAngle <= 100)
			{
				footStartType = 1;
				L1St.x = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x < sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x;//left

			}
			else if (fAngle > 10 && fAngle < 80)
			{
				footStartType = 2;
				L1St.x = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x < sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x;//left
				L2Ed.y = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y > sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y;//bottom

			}
			else if (fAngle <= 10 || fAngle >= 350)
			{
				footStartType = 3;
				L2Ed.y = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y > sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y;//bottom
			}
			else if (fAngle > 280 && fAngle < 350)
			{
				footStartType = 4;
				L2Ed.x = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x > sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x;//right
				L2Ed.y = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y > sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y;//bottom
			}
			else if (fAngle >= 260 && fAngle <= 280)
			{
				footStartType = 5;
				L2Ed.x = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x > sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x;//right
			}
			else if (fAngle > 190 && fAngle < 260)
			{
				footStartType = 6;
				L2Ed.x = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x > sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x;//right
				L1St.y = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y < sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y;//top
			}
			else if (fAngle >= 170 && fAngle <= 190)
			{
				footStartType = 7;
				L1St.y = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y < sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y;//top
			}
			else if (fAngle > 100 && fAngle < 170)
			{
				footStartType = 8;
				L1St.x = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x < sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].x : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].x;//left
				L1St.y = sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y < sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y ?
					sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding1].y : sRstAlgo->rePoinEd[m_eFoot::m_eFoot_Bonding2].y;//top
			}
			float shift, fshiftX, fshiftY;
			int shiftX, shiftY;


			footStartType = 0;

			CalcShiftPointLeft(L1St, L2Ed, DBCSt, DBCEd, footStartType, &shiftX, &shiftY);
			fshiftX = m_resolX * shiftX;
			fshiftY = m_resolY * shiftY;
			shift = std::sqrtf(std::powf(fshiftX, 2) + std::powf(fshiftY, 2));

			Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationX, fshiftX);
			Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationY, fshiftY);
			Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_Deviation, shift);
		}

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::Insp_FootShift() Line %d Pass"), nLine);
		g_pMPTI->AddLog(msg);
	}

	return true;
}

void CPInsp_Foot::CalcShiftPointLeft(cv::Point InspSt, cv::Point InspEd, cv::Point StdSt, cv::Point StdEd, int type, int* shX, int* shY)
{
	//←↑→↓
	//*shX = InspSt.x - StdEd.x;//→
	//*shX = StdEd.x - InspSt.x;//←
	//*shY = StdSt.y - InspSt.y;//↑
	//*shY = StdEd.y - InspSt.y;//↓
	//-값이면 정상 / +만큼 shift
	switch (type)
	{
	case 1: //↑→↓
		*shX = InspEd.x - StdEd.x;//→
		*shY = StdSt.y - InspSt.y;//↑
		if (*shY < 0)
		{
			*shY = InspEd.y - StdEd.y;//↓
		}
		break;
	case 2:  //↑→
		*shY = StdSt.y - InspSt.y;//↑
		*shX = InspEd.x - StdEd.x;//→
		break;
	case 3:    //←↑→
		*shX = InspEd.x - StdEd.x;//→
		*shY = StdSt.y - InspSt.y;//↑
		if (*shX < 0)
		{
			*shX = StdSt.x - InspSt.x;//←
		}
		break;
	case 4:  //↑→
		*shY = StdSt.y - InspSt.y;//↑
		*shY = InspEd.y - StdEd.y;//↓
		break;
	case 5:   //↑←↓
		*shX = InspEd.x - StdEd.x;//→
		*shY = StdSt.y - InspSt.y;//↑
		if (*shY < 0)
		{
			*shY = InspEd.y - StdEd.y;//↓
		}
		break;
	case 6:   //←↓
		*shX = StdSt.x - InspSt.x;//←
		*shY = InspEd.y - StdEd.y;//↓
		break;
	case 7:   //←→↓
		*shX = InspEd.x - StdEd.x;//→
		*shY = InspEd.y - StdEd.y;//↓
		if (*shX < 0)
		{
			*shX = StdSt.x - InspSt.x;//←
		}
		break;
	case 8:   //→↓
		*shX = InspEd.x - StdEd.x;//→
		*shY = InspEd.y - StdEd.y;//↓
		break;
	default:

		*shX = InspEd.x - StdEd.x;//→
		*shY = StdSt.y - InspSt.y;//↑
		if (*shX < 0)
		{
			*shX = StdSt.x - InspSt.x;//←
		}
		if (*shY < 0)
		{
			*shY = InspEd.y - StdEd.y;//↓
		}
		break;

	}


	if (*shX < 0)
	{
		*shX = 0;
	}
	if (*shY < 0)
	{
		*shY = 0;
	}

	return;
}

int CPInsp_Foot::GetBlob_BondingFloor(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, int nBlobCnt, int InspFootCnt, long *pLebel, USHORT* LabelImage,
	cv::Mat * cvAllFoot, cv::Mat * cvAllWedgeImg, cv::Mat * cvAllWing, cv::Mat * cvAll3DAreaImg,
	std::vector<int>& nMatLabel, std::vector<int>& nMatLeft, std::vector<int>& nMatRight, std::vector<int>& nMatTop, std::vector<int>& nMatBottom,
	std::vector<int> &LeftIdx, std::vector<cv::Rect>& rcFootROI,
	int nWidth, int nHeight)
{
	int nImgWidth = pFoot->GetImageWidth();
	int nImgHeight = pFoot->GetImageLength();
	int imgSize = pFoot->GetImageSize();

	//double pLebel_Area[PTR_BLOB_MAX];
	//int pLebel_MaxIDX[PTR_BLOB_MAX];

	UCHAR * srcPtr = NULL;
	srcPtr = (cvAllFoot->ptr<UCHAR>());
	UCHAR *pucBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucBlob, nWidth * nHeight);
	memset(pucBlob, 0, sizeof(UCHAR) * nWidth * nHeight);
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(srcPtr, pucBlob, nWidth, nHeight, 4, FALSE, FALSE, 0, eSelectMix);
	Delete_1DArray(&pucBlob);

	m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);

	cv::Mat ImgFoot(cvAllFoot->rows, cvAllFoot->cols, CV_8UC1, cvAllFoot->data);
	cv::Mat ImgWedge(cvAllWedgeImg->rows, cvAllWedgeImg->cols, CV_8UC1, cvAllWedgeImg->data);
	cv::Mat ImgWing(cvAllWing->rows, cvAllWing->cols, CV_8UC1, cvAllWing->data);
	cv::Mat Img3DArea(cvAll3DAreaImg->rows, cvAll3DAreaImg->cols, CV_32FC1, cvAll3DAreaImg->data);

	if (nBlobCnt > InspFootCnt)
		nBlobCnt = RemoveThin(pFoot, nBlobCnt, pLebel, LabelImage, &ImgFoot, &ImgWedge, &ImgWing, &Img3DArea);

	m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);


	//NYJ 2019/10/11 
	//Calc Foot Blob ROI Rect (한 Pad에 두 개 이상의 Foot 존재 시, Blob된 Foot 중심점이 실제 검사 영역과 가까운 것 선택 
	std::vector<cv::Rect> rcListFootROI(nBlobCnt);
	int min_x = nImgWidth * nImgHeight, max_x = 0;
	int min_y = nImgWidth * nImgHeight, max_y = 0;
	CString cstrFindedLabel;

	for (int a = 0; a < nBlobCnt; a++)
	{
		cstrFindedLabel.Format(_T("Find_Label_%d.bmp"), a);
		CPInsp::GetLabelImage(pLebel[a], LabelImage, nImgWidth, nImgHeight, cstrFindedLabel);
		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIndex = y * nImgWidth + x;
				if (pLebel[a] != LabelImage[nIndex])
					continue;

				if (pLebel[a] == LabelImage[nIndex])
				{
					if (x < min_x) min_x = x;
					if (x > max_x) max_x = x;
					if (y < min_y) min_y = y;
					if (y > max_y) max_y = y;
				}
			}
		}

		//Foot ROI 리스트 세팅
		rcListFootROI[a].x = min_x;
		rcListFootROI[a].y = min_y;
		rcListFootROI[a].width = max_x - min_x;
		rcListFootROI[a].height = max_y - min_y;

		//initialize Rect param
		min_x = nImgWidth * nImgHeight;
		max_x = 0;
		min_y = nImgWidth * nImgHeight;
		max_y = 0;
	}
	rcFootROI = rcListFootROI;


	if (InspFootCnt > nBlobCnt)		//footCnt==InspFootCnt
	{
		sRstAlgo->m_bOK = FALSE;		//sRstAlgo->m_bOK = (footCnt >= sInspFoot->m_byFoot);
		sRstAlgo->m_bFind = FALSE;

		if (nBlobCnt == 0) return FALSE;
		InspFootCnt = nBlobCnt;
	}

	int mar = pLebel[nBlobCnt - 1];


	if (mar == 0)
		return FALSE;

	//start accuracy
	std::vector<int> AreaCnt(mar);
	std::vector<int> BlobRank(mar);

	for (int i = 0; i < mar; i++)
	{
		BlobRank[i] = i + 1;
		AreaCnt[i] = 0;
	}

	for (int i = 0; i < nBlobCnt; i++)
	{
		for (int r = 0; r < nHeight; r++)
		{
			for (int c = 0; c < nWidth; c++)
			{
				if (pLebel[i] == LabelImage[r*nWidth + c])
					AreaCnt[pLebel[i] - 1]++;

			}
		}
	}

	for (int i = 0; i < mar; i++)
	{
		for (int j = mar - 1; j > i; j--)
		{
			if (AreaCnt[j - 1] < AreaCnt[j])
			{
				int ntemp = BlobRank[j];
				BlobRank[j] = BlobRank[j - 1];
				BlobRank[j - 1] = ntemp;

				ntemp = AreaCnt[j];
				AreaCnt[j] = AreaCnt[j - 1];
				AreaCnt[j - 1] = ntemp;
			}
		}
	}

	//foot inspection start
	std::vector<cv::Mat> vImgMat(InspFootCnt);

	nMatLeft.assign(InspFootCnt, nWidth);
	nMatRight.assign(InspFootCnt, 0);
	nMatLabel.assign(InspFootCnt, 0);

	for (int i = 0; i < InspFootCnt; i++)
	{
		cv::Mat temp = cv::Mat(nHeight, nWidth, CV_8UC1);
		temp.setTo(0);
		for (int r = 0; r < nHeight; r++)
		{
			UCHAR* imgPtr = temp.ptr(r);
			for (int c = 0; c < nWidth; c++)
			{
				if ((BlobRank[i]) == LabelImage[r*nWidth + c])
				{
					nMatLabel[i] = BlobRank[i];
					imgPtr[c] = 255;
					if (nMatLeft[i] > c)
						nMatLeft[i] = c;
					if (nMatRight[i] < c)
						nMatRight[i] = c;
				}
			}
		}
		int nw = nMatRight[i] - nMatLeft[i];
		if (nw < 1)
		{
			nMatLeft[i] = 0;
			nw = nMatRight[i] = nWidth;
		}
		int nL = nMatLeft[i];

		cv::Mat tempRect = temp(cv::Rect(nL, 0, nw, nHeight));
		tempRect.copyTo(vImgMat[i]);
	}

	nMatTop.assign(vImgMat.size(), 0);
	nMatBottom.assign(vImgMat.size(), 0);

	for (int i = 0; i < vImgMat.size(); i++)
	{
		cv::Mat integ, Devi;
		cv::integral(vImgMat[i], integ, Devi);
		for (int r = 0; r < vImgMat[i].rows; r++)
		{
			UINT* UNPtr = integ.ptr<UINT>(r);
			if (UNPtr[integ.cols - 1] > 0)
			{
				nMatTop[i] = r;
				break;
			}
		}
		for (int r = vImgMat[i].rows - 1; r > 0; r--)
		{

			UINT* UNPtr_1 = integ.ptr<UINT>(r - 1);
			UINT* UNPtr = integ.ptr<UINT>(r);
			if (UNPtr[integ.cols - 1] - UNPtr_1[integ.cols - 1] > 0)
			{
				nMatBottom[i] = r;
				break;
			}
		}
	}

	float fTopWhCnt(0), fBotWhCnt(0);

	for (int r = 0; r < 10; r++)
	{
		for (int c = 0; c < nWidth; c++)
		{
			if (LabelImage[r*nWidth + c] > 0)
				fTopWhCnt++;
		}
	}
	for (int r = nHeight - 1; r > nHeight - 11; r--)
	{
		for (int c = 0; c < nWidth; c++)
		{
			if (LabelImage[r*nWidth + c] > 0)
				fBotWhCnt++;
		}
	}

#if _DEBUG
	cv::Mat vLabelMat = cv::Mat(nHeight, nWidth, CV_8UC1);
	vLabelMat.setTo(0);
	for (int r = 0; r < nHeight; r++)
	{
		UCHAR* imgPtr = vLabelMat.ptr(r);
		for (int c = 0; c < nWidth; c++)
		{
			imgPtr[c] = LabelImage[r*nWidth + c] * 20;
		}
	}

	CString sLbName;
	sLbName.Format(_T("D:\\FootRst\\0_InspFoot_Label.bmp"));
	cv::imwrite(std::string(CT2A(sLbName)), vLabelMat);

#endif
#if _DEBUG

	cv::Mat vBlobMat = cv::Mat(nHeight, nWidth, CV_8UC1);
	vBlobMat.setTo(0);
	for (int r = 0; r < nHeight; r++)
	{
		UCHAR* imgBlobPtr = vBlobMat.ptr(r);
		for (int c = 0; c < nWidth; c++)
		{
			if (LabelImage[r*nWidth + c] > 0)
				imgBlobPtr[c] = 255;
		}
	}
	CString sLbName1;
	sLbName1.Format(_T("D:\\FootRst\\0_InspFoot_vBlobMat.bmp"));
	cv::imwrite(std::string(CT2A(sLbName1)), vBlobMat);
	cv::Mat Intg, Dev;
	cv::integral(vBlobMat, Intg, Dev);

	UINT* IntgPtr = Intg.ptr<UINT>(2);
	UINT unTopCnt = IntgPtr[Intg.cols - 1];
	IntgPtr = Intg.ptr<UINT>(Intg.rows - 3);
	UINT unBotCnt_3 = IntgPtr[Intg.cols - 1];
	IntgPtr = Intg.ptr<UINT>(Intg.rows - 1);
	UINT unBotCnt_1 = IntgPtr[Intg.cols - 1];
	UINT unBotCnt = unBotCnt_1 - unBotCnt_3;
	bool isbotWhite2D = unBotCnt > unTopCnt;
#endif


	std::vector<int> LeftIdx_Temp(nMatLeft.size());
	for (int Li = 0; Li < LeftIdx_Temp.size(); Li++)
		LeftIdx_Temp[Li] = Li;

	for (int i = 0; i < LeftIdx_Temp.size(); i++)
	{
		for (int j = LeftIdx_Temp.size() - 1; j > i; j--)
		{
			if (nMatLeft[LeftIdx_Temp[j]] < nMatLeft[LeftIdx_Temp[j - 1]])
			{
				int nt = LeftIdx_Temp[j];
				LeftIdx_Temp[j] = LeftIdx_Temp[j - 1];
				LeftIdx_Temp[j - 1] = nt;
			}
		}
	}

	LeftIdx = LeftIdx_Temp;

	cv::Mat vIdxMat = cv::Mat(nHeight, nWidth, CV_8UC1);

	return 0;

}

int CPInsp_Foot::RemoveThin(CFoot_Model *pFoot, int nCntBlob, long *pLebel, USHORT* LabelImage, cv::Mat * cvAllFoot, cv::Mat * cvAllWedgeImg, cv::Mat * cvAllWing, cv::Mat * cvAll3DAreaImg)
{
	cv::Mat ImgTemp(cvAllFoot->rows, cvAllFoot->cols, CV_8UC1, cvAllFoot->data);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\RemoveThin_AllFoot.bmp", *cvAllFoot);
	cv::imwrite("D:\\FootRst\\RemoveThin_AllWedgeImg.bmp", *cvAllWedgeImg);
	cv::imwrite("D:\\FootRst\\RemoveThin_AllWing.bmp", *cvAllWing);
	cv::imwrite("D:\\FootRst\\RemoveThin_All3DAreaImg.bmp", *cvAll3DAreaImg);
#endif
	int nImgWidth = pFoot->GetImageWidth();
	int nImgHeight = pFoot->GetImageLength();
	const int nMaxCnt = 100000;
	double pLebel_Area[nMaxCnt];
	int pLebel_MaxIDX[nMaxCnt];
	memset(pLebel_Area, 0, sizeof(double) * nMaxCnt);
	memset(pLebel_MaxIDX, 0, sizeof(int) * nMaxCnt);
	for (int a = 0; a < nCntBlob; a++)
	{
		for (int b = 0; b < nImgWidth * nImgHeight; b++)
		{
			if (pLebel[a] == LabelImage[b])
				pLebel_Area[a]++;
		}
	}
	if (CPInsp::GetCorrectIDX(nCntBlob, pLebel_Area, pLebel_MaxIDX) == 0)
		return nCntBlob;
	int nTotal = (nCntBlob > 3) ? 3 : nCntBlob;
	int nW = 0, nH = 0, nCnt = 0;
	for (int a = 0; a < nTotal; a++)
	{
		RECT rcLabel;
		rcLabel.left = -1;
		rcLabel.right = -1;
		rcLabel.top = -1;
		rcLabel.bottom = -1;
		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				if (pLebel[pLebel_MaxIDX[a]] != LabelImage[y * nImgWidth + x])
					continue;
				if (rcLabel.left == -1 || x < rcLabel.left) rcLabel.left = x;
				if (x > rcLabel.right) rcLabel.right = x;
			}
		}
		for (int x = 0; x < nImgWidth; x++)
		{
			for (int y = 0; y < nImgHeight; y++)
			{
				if (pLebel[pLebel_MaxIDX[a]] != LabelImage[y * nImgWidth + x])
					continue;
				if (rcLabel.top == -1 || y < rcLabel.top) rcLabel.top = y;
				if (y > rcLabel.bottom) rcLabel.bottom = y;
			}
		}
		if (rcLabel.left == -1 || rcLabel.right == -1 || rcLabel.top == -1 || rcLabel.bottom == -1)
			continue;
		nW += (rcLabel.right - rcLabel.left);
		nH += (rcLabel.bottom - rcLabel.top);
		nCnt++;
	}
	int nThinW = (nW / nCnt) * 0.3;
	int nThinH = (nH / nCnt) * 0.3;
	for (int a = 0; a < nCntBlob; a++)
	{
		int nRstW = 0, nRstH = 0;
		for (int y = 0; y < nImgHeight; y++)
		{
			int nS = -1;
			int nE = 0;
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIdx = y * nImgWidth + x;
				if (pLebel[a] != LabelImage[nIdx])
					continue;
				if (nS == -1) nS = x;
				nE = x;
			}
			if (nS >= 0 && nE - nS > nRstW) nRstW = nE - nS;
		}
		for (int x = 0; x < nImgWidth; x++)
		{
			int nS = -1;
			int nE = 0;
			for (int y = 0; y < nImgHeight; y++)
			{
				int nIdx = y * nImgWidth + x;
				if (pLebel[a] != LabelImage[nIdx])
					continue;
				if (nS == -1) nS = y;
				nE = y;
			}
			if (nS >= 0 && nE - nS > nRstH) nRstH = nE - nS;
		}
		if (nRstW > nThinW && nRstH > nThinH)
			continue;
		for (int y = 0; y < nImgHeight; y++)
		{
			int nS = -1;
			int nE = 0;
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIdx = y * nImgWidth + x;
				if (pLebel[a] != LabelImage[nIdx])
					continue;
				if (cvAllFoot->cols == nImgWidth && cvAllFoot->rows == nImgHeight) cvAllFoot->data[nIdx] = 0;
				if (cvAllWedgeImg->cols == nImgWidth && cvAllWedgeImg->rows == nImgHeight) cvAllWedgeImg->data[nIdx] = 0;
				if (cvAllWing->cols == nImgWidth && cvAllWing->rows == nImgHeight) cvAllWing->data[nIdx] = 0;
				if (cvAll3DAreaImg->cols == nImgWidth && cvAll3DAreaImg->rows == nImgHeight) cvAll3DAreaImg->data[nIdx] = 0;
			}
		}
	}
	cv::Mat BinImage2(nImgHeight, nImgWidth, CV_8UC1, cvAllFoot->data);
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage2.data, cvAllFoot->data, nImgWidth, nImgHeight, 4, FALSE, TRUE, 0, eSelectMix);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\RemoveThin_Rst_AllFoot.bmp", *cvAllFoot);
	cv::imwrite("D:\\FootRst\\RemoveThin_Rst_AllWedgeImg.bmp", *cvAllWedgeImg);
	cv::imwrite("D:\\FootRst\\RemoveThin_Rst_AllWing.bmp", *cvAllWing);
	cv::imwrite("D:\\FootRst\\RemoveThin_Rst_All3DAreaImg.bmp", *cvAll3DAreaImg);
#endif
	return nCntBlob;
}

//shkim calc PadBin Intg...
bool CPInsp_Foot::Calc_padBinImage2(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Rect ImageRect, cv::Mat* RepadImg, cv::Mat Img3D)
{
	try
	{
		cv::Mat padImg(ImageRect.height, ImageRect.width, CV_8UC1);
		//PIAL::PI_Buff padImgBuf(padImg, true);
		//PIAL::PI_Buff Img3DBuf(Img3D, true);

		pFoot->GetPadBinImage(&padImg);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\Calc_padBinImage.bmp", padImg);
#endif
		UCHAR *pUcImgDst2D = NULL;
		UCHAR *pUcImgDst3D = NULL;
		UCHAR *pUcImgBinary = padImg.data;
		double dArea, dCX, dCY;
		CRect rcBlob;

		AlgoBlob algoBlob;
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

		int Range3D = 0;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_bInsp3D = false;
		algoBlob.m_dHeightRateMin = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
		algoBlob.m_dHeightRateMax = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];

		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 100;
		algoBlob.m_nMaxBinary = 100;

		algoBlob.m_bFillHole = true;
		algoBlob.m_bFilterIsUse = false;

		TotalInspExceptArea stTieAreaNULL;
		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
		cv::Mat BinImage(padImg.rows, padImg.cols, CV_8UC1);
 		int nCntBlob = CPInsp::BlobImageStruct(algoBlob, padImg.data, Img3D.ptr<float>(0), NULL, padImg.cols, padImg.rows, 1000 /*10000*/,
 			&dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);

		//PIAL::PI_Buff BinImageBuf(BinImage, true);
		//int nCntBlob = PIAL::PAlgo::BlobImageStruct(algoBlob, &padImgBuf, &Img3DBuf, NULL, &BinImageBuf, padImgBuf.Width(), padImgBuf.Length(), 1000, &dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);

		BinImage.copyTo(*RepadImg);
		return true;
	}
	catch (...)
	{

	}
	return true;
}
bool CPInsp_Foot::Calc_padBinImage3(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Rect ImageRect, cv::Mat* RepadImg, cv::Mat Img3D, bool bTeachBin)
{
	try
	{
		cv::Mat padImg(ImageRect.height, ImageRect.width, CV_8UC1);
		//PIAL::PI_Buff padImgBuf(padImg, true);
		//PIAL::PI_Buff Img3DBuf(Img3D, true);
		pFoot->GetImageMatrix(&padImg, (int)m_eFootBin::m_eFootBin_Pad);

		int nWidth = ImageRect.width, nHeight = ImageRect.height;
		//pFoot->GetPadBinImage(&padImg);
		int nPadBlobCnt = GetBinImage_Pad(pFoot, &padImg, (int)m_eFootBin::m_eFootBin_Pad, NULL, false);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\Calc_padBinImage.bmp", padImg);
#endif

		//1. pad open padImage를 => 실크라인으로 수정
		cv::Mat PadMaskCalc = padImg.clone();
		//DBC 실크 라인 관련 검출
		if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		{
			cv::Mat DBCBinImage(padImg.rows, padImg.cols, CV_8UC1);

			bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

			///////////////////////
			if (bIsFindDBCOutLine)
				PadMaskCalc = PadMaskCalc + DBCBinImage;
		}
		
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11), cv::Point(1, 1));
		cv::Mat kernel_3 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));

		if (pFoot->m_bUsePadAreaAutoTeach)
		{
			cv::erode(PadMaskCalc, PadMaskCalc, kernel_3);
			cv::dilate(PadMaskCalc, PadMaskCalc, kernel_3);
		}
		else
		{
			cv::dilate(PadMaskCalc, PadMaskCalc, kernel);
			cv::erode(PadMaskCalc, PadMaskCalc, kernel);
		}

		//2.padImg convexhull
		cv::Mat convexhull(nHeight, nWidth, CV_8UC1);
		std::vector<cv::Point> vPadPoints, vHulPoints;
		convexhull.setTo(0);
		vPadPoints.clear();
		for (size_t y = 0; y < nHeight; y++)
		{
			UCHAR* pRaw = PadMaskCalc.ptr<UCHAR>(y);
			for (size_t x = 0; x < nWidth; x++)
				if (pRaw[x] > 0)
					vPadPoints.push_back(cv::Point(x, y));
		}
		if (vPadPoints.size()>0)
		{
			cv::convexHull(vPadPoints, vHulPoints);
			cv::fillPoly(convexhull, vHulPoints, cv::Scalar(255));
		}
		else
		{
			cv::Rect TeachRect(pFoot->m_PadRect.left - pFoot->mImageRect.left, pFoot->m_PadRect.top - pFoot->mImageRect.top
				, pFoot->m_PadRect.right - pFoot->m_PadRect.left, pFoot->m_PadRect.bottom - pFoot->m_PadRect.top);
			cv::rectangle(convexhull, TeachRect,cv::Scalar(255), cv::FILLED);
		}

		//2-1. MaskRect&orgPadImg
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(convexhull, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		double dMxArea = 0;
		int idx = 0;
		cv::Rect bMaxbox(0, 0, 0, 0);
		for (contours.begin(); idx < contours.size(); idx++)
		{
			cv::Rect bbox = cv::boundingRect(contours[idx]);
			double area = cv::contourArea(contours[idx]);

			if (area > dMxArea)
			{
				dMxArea = area;
				bMaxbox = bbox;
			}
		}
		if (!pFoot->m_bUsePadAreaAutoTeach)
		{
			if (bMaxbox.width > 0 && bMaxbox.height > 0)
			{
				cv::Mat mPadMaskRectImg(padImg.rows, padImg.cols, CV_8UC1);
				mPadMaskRectImg.setTo(0);
				cv::rectangle(mPadMaskRectImg, bMaxbox, cv::Scalar(255), cv::FILLED);
				PadMaskCalc = mPadMaskRectImg & padImg;
			}
		}

		convexhull.setTo(0);
		vPadPoints.clear();
		for (size_t y = 0; y < nHeight; y++)
		{
			UCHAR* pRaw = PadMaskCalc.ptr<UCHAR>(y);
			for (size_t x = 0; x < nWidth; x++)
				if (pRaw[x] > 0)
					vPadPoints.push_back(cv::Point(x, y));
		}
		if (vPadPoints.size() > 0)
		{
			cv::convexHull(vPadPoints, vHulPoints);
			cv::fillPoly(convexhull, vHulPoints, cv::Scalar(255));
		}
		else
		{
			cv::Rect TeachRect(pFoot->m_PadRect.left - pFoot->mImageRect.left, pFoot->m_PadRect.top - pFoot->mImageRect.top
				, pFoot->m_PadRect.right - pFoot->m_PadRect.left, pFoot->m_PadRect.bottom - pFoot->m_PadRect.top);
			cv::rectangle(convexhull, TeachRect, cv::Scalar(255), cv::FILLED);
		}

		//3. create FootBin Image
		cv::Mat Foot_Bin(nHeight, nWidth, CV_8UC1), PadNFootImg;
		pFoot->GetImageMatrix(&Foot_Bin, (int)m_eFootBin::m_eFootBin_Foot);
		GetBinImage(pFoot, &Foot_Bin, (int)m_eFootBin::m_eFootBin_Foot, NULL, false);

		//4. FootImg blob
		UCHAR *pUcImgDst2D = NULL;
		UCHAR *pUcImgDst3D = NULL;
		UCHAR *pUcImgBinary = padImg.data;
		double dArea, dCX, dCY;
		CRect rcBlob;

		AlgoBlob algoBlob;
		if (pFoot->m_bUse2Foot)
			algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		else if((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
			algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		else
			algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

		int Range3D = 0;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_bInsp3D = false;
		algoBlob.m_dHeightRateMin = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
		algoBlob.m_dHeightRateMax = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];

		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 100;
		algoBlob.m_nMaxBinary = 100;

		algoBlob.m_bFillHole = true;
		algoBlob.m_bFilterIsUse = false;

		TotalInspExceptArea stTieAreaNULL;
		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
		cv::Mat FootBlobImage(padImg.rows, padImg.cols, CV_8UC1);

		//Foot BW영상에서 Pad 영역제거
		cv::Mat cvOnlyPadBW = CalcOnlyPadArea(pFoot, padImg.cols, padImg.rows, (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC);
		Foot_Bin -= cvOnlyPadBW;
		
		int nCntBlob = CPInsp::BlobImageStruct(algoBlob, Foot_Bin.data, Img3D.ptr<float>(0), NULL, Foot_Bin.cols, Foot_Bin.rows, 1000 /*10000*/,
			&dArea, &dCX, &dCY, &rcBlob, FootBlobImage.data, stTieAreaNULL, true);

		//PIAL::PI_Buff BinImageBuf(BinImage, true);
		//int nCntBlob = PIAL::PAlgo::BlobImageStruct(algoBlob, &padImgBuf, &Img3DBuf, NULL, &BinImageBuf, padImgBuf.Width(), padImgBuf.Length(), 1000, &dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);
		//5. footImg | padCalc
		PadNFootImg = PadMaskCalc + FootBlobImage;
		cv::dilate(PadNFootImg, PadNFootImg, kernel);
		cv::erode(PadNFootImg, PadNFootImg, kernel);
		int nCntPadNFoot = CPInsp::BlobImageStruct(algoBlob, PadNFootImg.data, Img3D.ptr<float>(0), NULL, Foot_Bin.cols, Foot_Bin.rows, 1000 /*10000*/,
			&dArea, &dCX, &dCY, &rcBlob, PadNFootImg.data, stTieAreaNULL, true);

		//6. convexhull & CalcImg
		cv::Mat DeviationImg = PadNFootImg & convexhull;
		cv::dilate(DeviationImg, DeviationImg, kernel);
		cv::dilate(DeviationImg, DeviationImg, kernel);
		cv::erode(DeviationImg, DeviationImg, kernel);
		cv::erode(DeviationImg, DeviationImg, kernel);

		DeviationImg.copyTo(*RepadImg);
		//m_pProcMilAlgo->CalcBlob_Select(DeviationImg.data, RepadImg->data, nWidth, nHeight, 1000, 0, true, 0, algoBlob.m_nTypeSelectBlob);
		nCntBlob = CPInsp::BlobImageStruct(algoBlob, DeviationImg.data, Img3D.ptr<float>(0), NULL, Foot_Bin.cols, Foot_Bin.rows, 1000 /*10000*/,
			&dArea, &dCX, &dCY, &rcBlob, RepadImg->data, stTieAreaNULL, true);

		//7. Teaching Rect
		cv::Mat TeachRectImg(nHeight, nWidth,CV_8UC1);
		TeachRectImg.setTo(0);
		cv::Rect TeachPadRect(pFoot->m_PadRect.left - pFoot->mImageRect.left, pFoot->m_PadRect.top - pFoot->mImageRect.top
		, pFoot->m_PadRect.right - pFoot->m_PadRect.left, pFoot->m_PadRect.bottom - pFoot->m_PadRect.top);
		cv::rectangle(TeachRectImg, TeachPadRect, cv::Scalar(255), cv::FILLED);
		cv::bitwise_and(*RepadImg, TeachRectImg, *RepadImg);

		cv::Mat RstPadImage = *RepadImg;
 		if (pFoot->bUseTeachRect && !bTeachBin)
 		{
 			cv::RotatedRect RoRe;
 
 			RoRe.size.width = pFoot->nPadWidth;//147;
 			RoRe.size.height = pFoot->nPadHeight;//166;
 			RoRe.center.x = RoRe.size.width / 2 + 0.5;
 			RoRe.center.y = RoRe.size.height / 2 + 0.5;
 			RoRe.angle = 0.0f;
 
 			cv::Mat ConvexHullImage = DrawRoundPad(RoRe.size.width + 2, RoRe.size.height + 2, RoRe, pFoot->nRadius);
 
 			sRstAlgo->m_nRadius = pFoot->nRadius;
 			sRstAlgo->m_nPadWidth = pFoot->nPadWidth;
 			sRstAlgo->m_nPadHeight = pFoot->nPadHeight;
 
 			RstPadImage = MatPolygon(*RepadImg, ConvexHullImage);
 		}
 		else if (pFoot->bUseTeachRect)
		{
			cv::Rect PadRect(0, 0, 0, 0);
			cv::Mat ConvexHullImage = calcRoundRect(*RepadImg, pFoot->nRadius, PadRect);
			//cv::Mat ConvexHullImage = imread("D:\\MatchRect.bmp", cv::IMREAD_GRAYSCALE);

			sRstAlgo->m_nRadius = pFoot->nRadius;
			sRstAlgo->m_nPadWidth = PadRect.width;
			sRstAlgo->m_nPadHeight = PadRect.height;

			RstPadImage = MatPolygon(*RepadImg, ConvexHullImage);

#if _DEBUG
			if (cstDebugFolderPath.IsEmpty() != TRUE)
			{
				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);

				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);

				cv::imwrite(rawname + std::string("_cvFindPadRst.bmp"), RstPadImage);
				cv::imwrite(rawname + std::string("_cvFindConvexHull.bmp"), ConvexHullImage);
				cv::imwrite(rawname + std::string("_cvFindPad_InputPad.bmp"), *RepadImg);
			}
#endif

		}
		else
		{
			sRstAlgo->m_nRadius = pFoot->nRadius;
			sRstAlgo->m_nPadWidth = pFoot->nPadWidth;
			sRstAlgo->m_nPadHeight = pFoot->nPadHeight;
		}
		
#if _DEBUG
		cv::imwrite("D:\\zRstPadImage.bmp", RstPadImage);
		cv::imwrite("D:\\zRepadImg.bmp", *RepadImg);
#endif
		RstPadImage.copyTo(*RepadImg);
		//DeviationImg.copyTo(*RepadImg);

		/*
		if(pFoot->bUseTeachRect)		//if use RoundRect option..IGBT
		{
			cv::Mat cvPad2DImg(nHeight, nWidth, CV_8UC1);
			pFoot->GetImageMatrix(&cvPad2DImg, (int)m_eFootBin::m_eFootBin_Pad);

			//1. make bw image And 
			//1-1. cvOnlyPadBWImg= pad2D_BwImg - FootBlobImage
			int nPad2DGrayValMin = 100;

			cv::Mat imgPad2DBWTemp(nHeight, nWidth, CV_8UC1);
			cv::threshold(cvPad2DImg, imgPad2DBWTemp, nPad2DGrayValMin, 255, cv::THRESH_BINARY);

			cv::Mat cvOnlyPadBWImg = imgPad2DBWTemp - FootBlobImage;

			//RstPadImage
			cv::Mat PadRangeImg(cvOnlyPadBWImg.rows, cvOnlyPadBWImg.cols, CV_8UC1);
			PadRangeImg.setTo(0);
			
			int nPadRectMinX = cvOnlyPadBWImg.cols;
			int nPadRectMaxX = 0;
			int nPadRectMinY = cvOnlyPadBWImg.rows;
			int nPadRectMaxY = 0;

			for (int r = 0; r < DeviationImg.rows; r++)
			{
				UCHAR* Ptr = DeviationImg.data;
				for (int c = 0; c < DeviationImg.cols; c++)
				{
					if (Ptr[r * DeviationImg.step + c] == 255)
					{
						if (r < nPadRectMinY)	nPadRectMinY = r;
						if (r > nPadRectMaxY)	nPadRectMaxY = r;
						if (c < nPadRectMinX)	nPadRectMinX = c;
						if (c > nPadRectMaxX)	nPadRectMaxX = c;

					}
				}
			}

			if (nPadRectMinX != cvOnlyPadBWImg.cols && nPadRectMaxX != 0 &&
				nPadRectMinY != cvOnlyPadBWImg.rows && nPadRectMaxY != 0)
			{
				int nMarginPixelCnt = 20;
				int nPadW = (nPadRectMaxX - nPadRectMinX) + 1;
				int nPadL = (nPadRectMaxY - nPadRectMinY) + 1;

				if (nPadRectMinX - nMarginPixelCnt < 0)
					nPadRectMinX = 0;
				else
					nPadRectMinX = nPadRectMinX - nMarginPixelCnt;

				if (nPadRectMinY - nMarginPixelCnt < 0)
					nPadRectMinY = 0;
				else
					nPadRectMinY = nPadRectMinY - nMarginPixelCnt;

				if (nPadRectMinX + (nPadW)+(nMarginPixelCnt*2) >= cvOnlyPadBWImg.cols)
					nPadW = cvOnlyPadBWImg.cols - nPadRectMinX;
				else
					nPadW = (nPadW)+(nMarginPixelCnt*2);
				
				if (nPadRectMinY + (nPadL)+(nMarginPixelCnt*2) >= cvOnlyPadBWImg.rows)
					nPadL = cvOnlyPadBWImg.rows - nPadRectMinY;
				else
					nPadL = (nPadL)+(nMarginPixelCnt*2);
				
				{
					cv::rectangle(PadRangeImg, cv::Rect(nPadRectMinX, nPadRectMinY, nPadW, nPadL), cv::Scalar(255), cv::FILLED);
					cvOnlyPadBWImg = cvOnlyPadBWImg & PadRangeImg;
				}
			}

			//2.convert padImg to convexhullimg
			cv::Mat convexhullimg(nHeight, nWidth, CV_8UC1);
			std::vector<cv::Point> vPadPoints_2DPad, vHulPoints_2DPad;
			convexhullimg.setTo(0);
			vPadPoints_2DPad.clear();
			for (size_t y = 0; y < nHeight; y++)
			{
				UCHAR* pRaw = cvOnlyPadBWImg.ptr<UCHAR>(y);
				for (size_t x = 0; x < nWidth; x++)
					if (pRaw[x] > 0)
						vPadPoints_2DPad.push_back(cv::Point(x, y));
			}
			if (vPadPoints_2DPad.size() > 0)
			{
				cv::convexHull(vPadPoints_2DPad, vHulPoints_2DPad);
				cv::fillPoly(convexhullimg, vHulPoints_2DPad, cv::Scalar(255));
			}

			convexhullimg.copyTo(*RepadImg);

			

			//3. make round rect
 			cv::Mat cvGet2DPadImgShape;
 			if (pFoot->bUseTeachRect && !bTeachBin)
 			{
 				cv::RotatedRect RoRe;
 
 				RoRe.size.width = pFoot->nPadWidth;//147;
 				RoRe.size.height = pFoot->nPadHeight;//166;
 				RoRe.center.x = RoRe.size.width / 2 + 0.5;
 				RoRe.center.y = RoRe.size.height / 2 + 0.5;
 				RoRe.angle = 0.0f;
 
 				cv::Mat ConvexHullImage = DrawRoundPad(RoRe.size.width + 2, RoRe.size.height + 2, RoRe, pFoot->nRadius);
 
 				sRstAlgo->m_nRadius = pFoot->nRadius;
 				sRstAlgo->m_nPadWidth = pFoot->nPadWidth;
 				sRstAlgo->m_nPadHeight = pFoot->nPadHeight;
 
 				cvGet2DPadImgShape = MatPolygon(convexhullimg, ConvexHullImage);
 			}
 			else if (pFoot->bUseTeachRect)
 			{
 				cv::Rect PadRect(0, 0, 0, 0);
 				cv::Mat ConvexHullImage = calcRoundRect(convexhullimg, pFoot->nRadius, PadRect);
 				//cv::Mat ConvexHullImage = imread("D:\\MatchRect.bmp", cv::IMREAD_GRAYSCALE);
 
 				sRstAlgo->m_nRadius = pFoot->nRadius;
 				sRstAlgo->m_nPadWidth = PadRect.width;
 				sRstAlgo->m_nPadHeight = PadRect.height;
 
 				cvGet2DPadImgShape = MatPolygon(convexhullimg, ConvexHullImage);
 			}
 			else
 			{
 				sRstAlgo->m_nRadius = pFoot->nRadius;
 				sRstAlgo->m_nPadWidth = pFoot->nPadWidth;
 				sRstAlgo->m_nPadHeight = pFoot->nPadHeight;
 			}
 
 			cvGet2DPadImgShape.copyTo(*RepadImg);

			if (cstDebugFolderPath.IsEmpty() != TRUE)
			{
				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);

				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);

				cv::imwrite(rawname + std::string("_cvPadconvexhull.bmp"), cvGet2DPadImgShape);
				cv::imwrite(rawname + std::string("_cvFindPadImg.bmp"), DeviationImg);
				cv::imwrite(rawname + std::string("_cvFindPadImg_MarginArea.bmp"), PadRangeImg);
				cv::imwrite(rawname + std::string("_cvFindPad_Use2D.bmp"), cvOnlyPadBWImg);
			}

		}
		*/

		return true;
	}
	catch (...)
	{

	}
	return true;
}

bool CPInsp_Foot::FindDBCOutLines(CFoot_Model* pFoot, cv::Mat *cvDstDBCLineImg, int n2DThresMin, int nFootDir)
{
	bool bIsDetectedDBCLines = false;

	///////////////////////
	int width = pFoot->m_ImageBuffer->nImageSizeX;
	int height = pFoot->m_ImageBuffer->nImageSizeY;

	cv::Mat ImgBGDBCoutLine(height, width, CV_8UC1);

	int FOVSize = width * height;

	cv::Mat TmpImg(ImgBGDBCoutLine.rows, ImgBGDBCoutLine.cols, CV_8UC1);

	cv::Rect ImageRectFoot;
	ImageRectFoot.x = pFoot->mImageRect.left;
	ImageRectFoot.y = pFoot->mImageRect.top;

	ImageRectFoot.x = 0;
	ImageRectFoot.y = 0;

	ImageRectFoot.width = pFoot->mImageRect.right - pFoot->mImageRect.left;
	ImageRectFoot.height = pFoot->mImageRect.bottom - pFoot->mImageRect.top;

	double redval(0.0), Bluval(0.0), BtmRedval(0.0), Whival(0.0);

	cv::Mat FOVRedImg(height, width, CV_8UC1);
	cv::Mat FOVBtmRedImg(height, width, CV_8UC1);
	cv::Mat FOVBlueImg(height, width, CV_8UC1);
	cv::Mat FOVWhiteImg(height, width, CV_8UC1);

	cv::Mat RedImg(ImgBGDBCoutLine.rows, ImgBGDBCoutLine.cols, CV_8UC1);
	cv::Mat BlueImg(ImgBGDBCoutLine.rows, ImgBGDBCoutLine.cols, CV_8UC1);
	cv::Mat BtmRedImg(ImgBGDBCoutLine.rows, ImgBGDBCoutLine.cols, CV_8UC1);

	TmpImg.setTo(0);

	{
		redval = (double)100 / 100;
		Bluval = (double)100 / 100;
		BtmRedval = (double)10 / 100;

		if (nFootDir == 4 || nFootDir==3 )		//1:up, 2:prev, 3:down, 4:next
		{
			//가로와이어 오른쪽에 DBC 본딩된 Foot일 경우
			BtmRedval = (double)2 / 100;
		}

		memcpy(FOVRedImg.data, pFoot->m_ImageBuffer->imgMiddle_R, FOVSize * sizeof(UCHAR));
		memcpy(FOVBlueImg.data, pFoot->m_ImageBuffer->imgMiddle_B, FOVSize * sizeof(UCHAR));
		memcpy(FOVBtmRedImg.data, pFoot->m_ImageBuffer->imgBottom_R, FOVSize * sizeof(UCHAR));

		RedImg = FOVRedImg(ImageRectFoot).mul(redval);
		BlueImg = FOVBlueImg(ImageRectFoot).mul(Bluval);
		BtmRedImg = FOVBtmRedImg(ImageRectFoot).mul(BtmRedval);

		TmpImg = RedImg - BlueImg + BtmRedImg;
	}


#if _DEBUG
	cv::imwrite("D:\\FootRst\\GrayImage.bmp", TmpImg);
#endif

	double dDBCArea = 0, dDBCCX = 0, dDBCCY = 0;
	CRect rcDBCBlob{ 0,0,0,0 };
	int nMinDBCBlob = 50;

	AlgoBlob algoBlob;
	algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

	int Range3D = 0;
	algoBlob.m_nTypeRange3D = 2;
	algoBlob.m_bInsp3D = false;
	algoBlob.m_dHeightRateMin = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
	algoBlob.m_dHeightRateMax = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];

	algoBlob.m_bInsp2D = true;
	algoBlob.m_nTypeRange2D = 2;
	if (n2DThresMin == 0)
	{
		algoBlob.m_nMinBinary = 70;
		algoBlob.m_nMaxBinary = 70;
	}
	else
	{
		algoBlob.m_nMinBinary = n2DThresMin;
		algoBlob.m_nMaxBinary = n2DThresMin;
	}
	

	algoBlob.m_bFillHole = true;
	algoBlob.m_bFilterIsUse = false;

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;

	cv::Mat DBCBinImage(TmpImg.rows, TmpImg.cols, CV_8UC1);

	cv::Mat Img3D(TmpImg.rows, TmpImg.cols, CV_32FC1);

	int nCntDBCBlob = CPInsp::BlobImageStruct(algoBlob, TmpImg.data, Img3D.ptr<float>(0), NULL, TmpImg.cols, TmpImg.rows, nMinDBCBlob /*10000*/,
		&dDBCArea, &dDBCCX, &dDBCCY, &rcDBCBlob, DBCBinImage.data, stTieAreaNULL, true);


	//////////////////////
	if (nCntDBCBlob > 0)
	{
		//3d 조건 추가로 Foot부분이 같이 검출되지 않도록 처리 
		cv::Mat cvTmpFoot3D(ImgBGDBCoutLine.rows, ImgBGDBCoutLine.cols, CV_32FC1, pFoot->pf3D);
		cv::Mat cvOnlyFootArea(ImgBGDBCoutLine.rows, ImgBGDBCoutLine.cols, CV_8UC1);
		cvOnlyFootArea.setTo(0);

		//Remove values ​​with 3D height over 5um
		size_t floatstep_3DCorrect = cvTmpFoot3D.step / sizeof(float);
		for (int y = 0; y < ImgBGDBCoutLine.rows; y++)
		{
			float *rawData = (float*)cvTmpFoot3D.data;
			UCHAR* pOnlyFootImg = cvOnlyFootArea.data;

			for (int x = 0; x < ImgBGDBCoutLine.cols; x++)
			{
				if (rawData[y * floatstep_3DCorrect + x] >= 30)
				{
					pOnlyFootImg[y * cvOnlyFootArea.step + x] = 255;
				}
			}
		}

		DBCBinImage = DBCBinImage - cvOnlyFootArea;

		bIsDetectedDBCLines = true;
		DBCBinImage.copyTo(*cvDstDBCLineImg);
	}

	return bIsDetectedDBCLines;
}

int CPInsp_Foot::SetLabelImage(USHORT* LabelImage, int nLabel, cv::Mat * cvAllWedgeImg, cv::Mat * cvAllWing, cv::Mat * cvAll3DAreaImg, cv::Rect ImageRect, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cv3DAreaImg)
{
	if (LabelImage == NULL || nLabel <= 0) return 0;
	int nImgWidth = ImageRect.width;
	int nImgHeight = ImageRect.height;
	float * srcPtr(cvAll3DAreaImg->ptr<float>());
	float * dstPtr(cv3DAreaImg->ptr<float>());
	for (int y = 0; y < nImgHeight; y++)
	{
		for (int x = 0; x < nImgWidth; x++)
		{
			int nIdx = y * nImgWidth + x;
			cvFootImg->data[nIdx] = (LabelImage[nIdx] == nLabel) ? 255 : 0;
			cvWedgeImg->data[nIdx] = (LabelImage[nIdx] == nLabel) ? cvAllWedgeImg->data[nIdx] : 0;
			cvWing->data[nIdx] = (LabelImage[nIdx] == nLabel) ? cvAllWing->data[nIdx] : 0;
			dstPtr[nIdx] = (LabelImage[nIdx] == nLabel) ? srcPtr[nIdx] : 0;
		}
	}
	cv::Mat ImgTemp(cv3DAreaImg->rows, cv3DAreaImg->cols, CV_32FC1, dstPtr);
	ImgTemp.copyTo(*cv3DAreaImg);
	CPInsp::GetLabelImage(nLabel, LabelImage, nImgWidth, nImgHeight, _T("Find_Label.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(srcPtr, nImgWidth, nImgHeight, _T("Find_Label_3D_Org.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(dstPtr, nImgWidth, nImgHeight, _T("Find_Label_3D_Dst.bmp"));
#if _DEBUG
	cv::imwrite("D:\\FootRst\\SetLabelImage_AllWedgeImg.bmp", *cvAllWedgeImg);
	cv::imwrite("D:\\FootRst\\SetLabelImage_All3DAreaImg.bmp", *cvAll3DAreaImg);
	cv::imwrite("D:\\FootRst\\SetLabelImage_AllWing.bmp", *cvAllWing);
	cv::imwrite("D:\\FootRst\\SetLabelImage_cvFootImg.bmp", *cvFootImg);
	cv::imwrite("D:\\FootRst\\SetLabelImage_cvWedgeImg.bmp", *cvWedgeImg);
	cv::imwrite("D:\\FootRst\\SetLabelImage_cv3DAreaImg.bmp", *cv3DAreaImg);
	cv::imwrite("D:\\FootRst\\SetLabelImage_Wing.bmp", *cvWing);
#endif
	return 1;
}

int CPInsp_Foot::convertToFootData(int footType)
{
	int DataType = -1;

	switch (footType)
	{
	case	(int)m_eFoot_DeviationX:	DataType = (int)m_eFOOT_Data_ShiftX; break;
	case	(int)m_eFoot_DeviationY:	DataType = (int)m_eFOOT_Data_ShiftY; break;
	case	(int)m_eFoot_Deviation:		DataType = (int)m_eFOOT_Data_Shift; break;
	case	(int)m_eFoot_Height:	DataType = (int)m_eFOOT_Data_Height; break;
	case	(int)m_eFoot_Damage:	DataType = (int)m_eFOOT_Data_Damage; break;
	case	(int)m_eFoot_Width_1:	DataType = (int)m_eFOOT_Data_Width_1; break;
	case	(int)m_eFoot_Width_1_H:	DataType = (int)m_eFOOT_Data_Width_1_H; break;
	case	(int)m_eFoot_Length_1:	DataType = (int)m_eFOOT_Data_Length_1; break;
	case	(int)m_eFoot_Tail_L:	DataType = (int)m_eFOOT_Data_Tail_L; break;
	case	(int)m_eFoot_Tail_A:	DataType = (int)m_eFOOT_Data_Tail_A; break;
	case	(int)m_eFoot_Wedge_W:	DataType = (int)m_eFOOT_Data_Wedge_W; break;
	case	(int)m_eFoot_Wedge_L:	DataType = (int)m_eFOOT_Data_Wedge_L; break;
	case	(int)m_eFoot_Foreign:	DataType = (int)m_eFOOT_Data_Foreign; break;
	case	(int)m_eFoot_HeelCrack:	DataType = (int)m_eFOOT_Data_HeelCrack; break;
	case	(int)m_eFoot_WingArea:	DataType = (int)m_eFOOT_Data_WingArea; break;
	}

	return DataType;
}

bool CPInsp_Foot::Confim_OneFootAlgo_byValue(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, int footType, float fArea)
{
	int nLine = __LINE__;
	try
	{
		int footDataType = convertToFootData(footType);

		float fDst = fArea;
		sRstAlgo->m_fArrRst[footType] = fDst;
		if ((pFoot->m_nUseOption & footDataType) == footDataType)
		{
			{
				if (pFoot->m_fArrOptionValue[footType][(int)m_eMMD::eMMD_Max] >= sRstAlgo->m_fArrRst[footType]
					&& pFoot->m_fArrOptionValue[footType][(int)m_eMMD::eMMD_Min] <= sRstAlgo->m_fArrRst[footType])
				{
					sRstAlgo->m_bArrOK[footType] = TRUE;
				}
				else
				{
					sRstAlgo->m_bArrOK[footType] = FALSE;
				}
			}
		}
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Foot::Confim_OneFoot() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}
	return true;
}

bool CPInsp_Foot::Confim_OneFootAlgo(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Point PoSt, cv::Point PoEd, int footType)
{
	int nLine = __LINE__;
	try
	{

		int footDataType = convertToFootData(footType);

		sRstAlgo->rePoinSt[footType].x = PoSt.x;
		sRstAlgo->rePoinSt[footType].y = PoSt.y;

		sRstAlgo->rePoinEd[footType].x = PoEd.x;
		sRstAlgo->rePoinEd[footType].y = PoEd.y;

		float delX = (PoSt.x - PoEd.x)*m_resolX;
		float delY = (PoSt.y - PoEd.y)*m_resolY;
		float fPow = std::powf(delX, 2) + std::powf(delY, 2);
		float fDst = std::sqrtf(fPow);
		sRstAlgo->m_fArrRst[footType] = fDst;

		if ((pFoot->m_nUseOption & footDataType) == footDataType)
		{
			if (pFoot->m_fArrOptionValue[footType][(int)m_eMMD::eMMD_Max] >= sRstAlgo->m_fArrRst[footType]
				&& pFoot->m_fArrOptionValue[footType][(int)m_eMMD::eMMD_Min] <= sRstAlgo->m_fArrRst[footType])
			{
				sRstAlgo->m_bArrOK[footType] = TRUE;
			}
			else
				sRstAlgo->m_bArrOK[footType] = FALSE;
		}
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Foot::Confim_OneFoot() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}
	return true;
}

void CPInsp_Foot::InspWingArea(CFoot_Model *pFoot, cv::Mat * cvImgL, cv::Mat * cvImgR, RstAlgoFoot * sRstAlgo)
{
	if (pFoot == NULL || sRstAlgo == NULL)
		return;
#if _DEBUG
	cv::imwrite("D:\\FootRst\\InspWingArea_L_Input.bmp", *cvImgL);
	cv::imwrite("D:\\FootRst\\InspWingArea_R_Input.bmp", *cvImgR);
#endif
	float fRstAreaA_1 = 0.0f;
	float fRstAreaL_1 = 0.0f;
	float fRstAreaA_2 = 0.0f;
	float fRstAreaL_2 = 0.0f;

	for (int a = 0; a < 2; a++)
	{
		UCHAR * srcPtr = NULL;
		int nWidth = 0;
		int nHeight = 0;
		if (a == 0)
		{
			srcPtr = (cvImgL->ptr<UCHAR>());
			nWidth = cvImgL->cols;
			nHeight = cvImgL->rows;
		}
		else
		{
			srcPtr = (cvImgR->ptr<UCHAR>());
			nWidth = cvImgR->cols;
			nHeight = cvImgR->rows;
		}
		if (srcPtr == NULL || nWidth <= 0 || nHeight <= 0)
			continue;

		UCHAR *pucBlob = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucBlob, nWidth * nHeight);
		memset(pucBlob, 0, sizeof(UCHAR) * nWidth * nHeight);

		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(srcPtr, pucBlob, nWidth, nHeight, 4, FALSE, FALSE, 0, eSelectMix);
		Delete_1DArray(&pucBlob);

		if (nCntBlob == 0) continue;

		double dArrArea[3], dArrCX[3], dArrCY[3];
		CRect rcArrBlob[3];
		memset(dArrArea, 0, sizeof(double) * 3);
		memset(dArrCX, 0, sizeof(double) * 3);
		memset(dArrCY, 0, sizeof(double) * 3);
		memset(rcArrBlob, 0, sizeof(CRect) * 3);

		m_pProcMilAlgo->GetBlobResult(dArrArea, dArrCX, dArrCY, rcArrBlob, true);

		if (a == 0)
		{
			fRstAreaA_1 = dArrArea[0] * m_resolX * m_resolY;
			fRstAreaL_1 = rcArrBlob[0].Height() * m_resolY;
		}
		else
		{
			fRstAreaA_2 = dArrArea[0] * m_resolX * m_resolY;
			fRstAreaL_2 = rcArrBlob[0].Height() * m_resolY;
		}

	}
	float fAreaMin = pFoot->m_fArrOptionValue[m_eFoot_WingArea][eMMD_Min];
	float fAreaMax = pFoot->m_fArrOptionValue[m_eFoot_WingArea][eMMD_Max];

	bool bOK1 = (fAreaMin <= fRstAreaA_1 && fRstAreaA_1 <= fAreaMax);
	bool bOK2 = (fAreaMin <= fRstAreaA_2 && fRstAreaA_2 <= fAreaMax);

	bool inspOpt = (pFoot->m_nUseOption &  (int)m_eFOOT_Data_WingArea) == (int)m_eFOOT_Data_WingArea;
	//RstAlgoFoot 구조체에 왼/오른쪽 wing 면적 정보 set
	sRstAlgo->m_bArrOK[m_eFoot_WingArea] = ((bOK1 && bOK2) || !inspOpt);
	sRstAlgo->m_fArrRst_WingArea[0] = fRstAreaA_1;
	sRstAlgo->m_fArrRst_WingArea[1] = fRstAreaL_1;
	sRstAlgo->m_fArrRst_WingArea[2] = fRstAreaA_2;
	sRstAlgo->m_fArrRst_WingArea[3] = fRstAreaL_2;

	if (bOK1 == false && bOK2 == true)
		sRstAlgo->m_fArrRst[m_eFoot_WingArea] = fRstAreaA_1;
	else if (bOK1 == true && bOK2 == false)
		sRstAlgo->m_fArrRst[m_eFoot_WingArea] = fRstAreaA_2;
	else
		sRstAlgo->m_fArrRst[m_eFoot_WingArea] = (fRstAreaA_1 > fRstAreaA_2) ? fRstAreaA_2 : fRstAreaA_1;
}

bool CPInsp_Foot::Insp_FootLift(cv::Mat* vImage, cv::Mat* fHImg, cv::Mat* PadImg, float* fHeightDiff)
{
	int baseCnt(0), footCnt(0);
	double baseSum(0), footSum(0);
	double baseAvg(0.0f), footAvg(0.0f);
	int tot(0);
	double totSum(0.0f);

	for (int r = 0; r < vImage->rows; r++)
	{
		UCHAR* ImgPtr = vImage->ptr(r);
		UCHAR* PadPtr = PadImg->ptr(r);
		float* HeightPtr = fHImg->ptr<float>(r);

		for (int c = 0; c < vImage->cols; c++)
		{
			if (ImgPtr[c] > 0)
			{
				footCnt++;
				footSum += HeightPtr[c];
			}
			else if (PadPtr[c] > 0)
			{
				baseCnt++;
				baseSum += HeightPtr[c];
			}
			tot++;
			totSum += HeightPtr[c];
		}
	}

	if (footCnt <= 0)
	{
		g_pMPTI->AddLog(_T("baseCnt / footCnt == 0!!"));
		return false;

	}

	baseAvg = 0;
	footAvg = footSum / footCnt;

	*fHeightDiff = (float)(footAvg - baseAvg);

	return true;
}

bool CPInsp_Foot::Insp_FootLift_v2(cv::Mat* vImage, cv::Mat* fHImg, float BotAvgH, float* fHeightDiff)
{
	int footCnt(0);
	double footSum(0);
	double footAvg(0.0f);
	int tot(0);
	double totSum(0.0f);

	cv::Mat img, intg, dev;
	img = *vImage;
	cv::integral(*vImage, intg, dev);
	int nT(0), nB = vImage->rows - 1;
	for (int r = 0; r < intg.rows; r++)
	{
		UINT* unPtr = intg.ptr<UINT>(r);
		if (unPtr[intg.cols - 1] > 0)
		{
			nT = r;
			break;
		}
	}
	for (int r = intg.rows - 1; r > 0; r--)
	{
		UINT* unPtr = intg.ptr<UINT>(r);
		UINT* unPtr_1 = intg.ptr<UINT>(r - 1);
		int nSub = unPtr[intg.cols - 1] - unPtr_1[intg.cols - 1];
		if (nSub > 0)
		{
			nB = r;
			break;
		}
	}

	int nLeng = nB - nT;
	int nCen = (nB + nT) / 2;

	nLeng *= 0.15;
	nT = nCen - nLeng;
	nB = nCen + nLeng;

	//if (nT < 0 || nB > vImage->rows - 1)
	{
		nT = 0;
		nB = vImage->rows;
	}

	for (int r = nT; r < nB; r++)
	{
		UCHAR* ImgPtr = vImage->ptr(r);
		float* HeightPtr = fHImg->ptr<float>(r);

		for (int c = 0; c < vImage->cols; c++)
		{
			if (ImgPtr[c] > 0)
			{
				footCnt++;
				footSum += HeightPtr[c];
			}
			tot++;
			totSum += HeightPtr[c];
		}
	}

	if (footCnt <= 0)
	{
		footCnt, footSum, tot, totSum = 0;

		for (int r = 0; r < vImage->rows; r++)
		{
			UCHAR* ImgPtr = vImage->ptr(r);
			float* HeightPtr = fHImg->ptr<float>(r);

			for (int c = 0; c < vImage->cols; c++)
			{
				if (ImgPtr[c] > 0)
				{
					footCnt++;
					footSum += HeightPtr[c];
				}
				tot++;
				totSum += HeightPtr[c];
			}
		}


		if (footCnt <= 0)
		{
			g_pMPTI->AddLog(_T("baseCnt / footCnt == 0!!"));
			return false;
		}
	}

	footAvg = footSum / footCnt;

	*fHeightDiff = (float)(footAvg - BotAvgH);

	return true;
}

bool CPInsp_Foot::Insp_FootTailLength(cv::Mat vImage, cv::Point pLeftSt, cv::Point pRightSt, cv::Point* pSt, cv::Point* pEd, float*fArea)
{
	cv::Mat intg, Dev;
	cv::Mat vImageTmp;
	vImage.copyTo(vImageTmp);

	vImageTmp &= 1;
	cv::integral(vImageTmp, intg, Dev);
	int MaxT = vImageTmp.rows;

	cv::Point pTmp;
	pTmp.x = vImageTmp.cols / 2;
	pTmp.y = 0;

	int endY = (pRightSt.y + pLeftSt.y) / 2;
	for (int r = 0; r < intg.rows; r++)
	{
		int ptrIntg = intg.ptr<int>(r)[intg.cols - 1];
		if (ptrIntg > 0)
		{
			pTmp.y = r;
			break;
		}
	}
	for (int c = 0; c < vImageTmp.cols; c++)
	{
		UCHAR ptr = vImageTmp.ptr(pTmp.y)[c];

		if (ptr > 0)
		{
			pTmp.x = c;
			break;
		}
	}
	/*
		for (int c = 0; c < vImage->cols; c++)
		{
			bool startWhiPix = false;
			UCHAR* zeroPtr = vImage->ptr(endY);
			if (zeroPtr[c] > 0)
			{
				startWhiPix = true;
			}
			for (int r = endY - 1; r > 0; r--)
			{
				UCHAR* ptr = vImage->ptr(r);
				if (startWhiPix && ptr[c] <= 0)
				{
					if (MaxT > r)
					{
						MaxT = r;
						pTmp.x = c;
						pTmp.y = r;
					}
					break;
				}
			}
		}
	*/
	pSt->x = pTmp.x;
	pSt->y = pTmp.y-1;

	pEd->x = pSt->x;
	pEd->y = endY;

	//float intg2 = intg.ptr(pEd->y)[intg.cols - 1];
	////float intg2 = intg.ptr(pTmp.y)[intg.cols - 1];
	//float intg1 = intg.ptr(pSt->y)[intg.cols - 1];

	int intg2 = intg.ptr<int>(pEd->y)[intg.cols - 1];
	int intg1 = intg.ptr<int>(pSt->y)[intg.cols - 1];
	float fAreaPixel = intg2 - intg1;
	*fArea = fAreaPixel * m_resolX*m_resolY;

	return true;
}

bool CPInsp_Foot::Insp_FootHillCrack(cv::Mat* vImage, cv::Mat* fHImg, cv::Point pEd, cv::Point* re_Start, cv::Point* re_End)
{
	if (vImage->rows - 1 <= pEd.y)
		return false;

	float fSum(0.0f), fAvg(0.0f);
	int nPixCnt(0);

	for (int r = 0; r < vImage->rows; r++)
	{
		UCHAR* BinPtr = vImage->ptr(r);
		float* fPtr = fHImg->ptr<float>(r);
		for (int c = 0; c < vImage->cols; c++)
		{
			if (BinPtr[c] > 0)
			{
				fSum += fPtr[c];
				nPixCnt++;
			}
		}
	}

	fAvg = fSum / nPixCnt;

	cv::Mat binImage = (*vImage)(cv::Rect(0, pEd.y, vImage->cols - 1, vImage->rows - 1 - pEd.y));
	cv::Mat rec3DImage = (*fHImg)(cv::Rect(0, pEd.y, fHImg->cols - 1, fHImg->rows - 1 - pEd.y));

	std::vector<std::vector<cv::Point>> vPo;
	std::vector<cv::Point> vPTemp;
	std::vector<float> vValue;
	vPo.clear();
	vPTemp.clear();
	vValue.clear();

	for (int r = 0; r < binImage.rows; r++)
	{
		UCHAR* BinPtr = binImage.ptr(r);
		float* fPtr = rec3DImage.ptr<float>(r);

		vPTemp.clear();
		for (int c = 0; c < binImage.cols; c++)
		{
			if (BinPtr[c] > 0 && fPtr[c] < fAvg)
			{
				cv::Point pTemp;
				pTemp.x = c;
				pTemp.y = r;

				float fTemp = fPtr[c];

				vPTemp.push_back(pTemp);
				vValue.push_back(fTemp);
			}
		}
		vPo.push_back(vPTemp);
	}
	int MaxSize(5), MaxIdx(-1);
	for (int i = 0; i < vPo.size(); i++)
	{
		if (vPo[i].size() > MaxSize)
		{
			MaxSize = vPo[i].size();
			MaxIdx = i;
		}
	}
	if (MaxIdx == -1)
	{
		re_Start->x = -1;
		re_Start->y = -1;
		re_End->x = -1;
		re_End->y = -1;
		return true;
	}

	re_Start->x = vPo[MaxIdx][0].x;
	re_Start->y = vPo[MaxIdx][0].y + pEd.y;
	re_End->x = vPo[MaxIdx][vPo[MaxIdx].size() - 1].x;
	re_End->y = vPo[MaxIdx][vPo[MaxIdx].size() - 1].y + pEd.y;

	return true;
}

/////////////////////
// 풋 찍힘검사
// 1. 전제조건: 풋 부분이 세워진상태를 기준으로함.
//   바닥면 기준 찍힌 깊이가 25%이하 NG
//   찍힌 면적의 가로길이가 너비의 50%이상이면 NG
// 2. input/output: 이진화 이미지, 너비값(convert to pixel)/ 최대 찍힘 구간의 너비 좌표.
/////////////////////
bool CPInsp_Foot::Insp_FootImprint2(cv::Mat* vImage, float fWidth, cv::Point pSt, cv::Point pEd, cv::Point* re_Start, cv::Point* re_End)
{
	cv::Mat Intg, Dev;

	float MinW = fWidth;
	int MinR = 0;
	int MaxRow;

	bool bIsDetected = false;

	int nLine = __LINE__;
	try
	{
		cv::Mat localImg = vImage->clone();

		localImg &= 1;
		cv::integral(localImg, Intg, Dev);

		int CntX = localImg.cols / 2;

		std::vector<int> vSt, vEd, vHist, vCnt;
		vCnt.assign(Intg.rows, 0);
		vSt.assign(Intg.rows, 0);
		vEd.assign(Intg.rows, 0);
		vHist.assign(Intg.cols, 0);
		for (int r = 1; r < Intg.rows; r++)
		{
			int* nPtr = Intg.ptr<int>(r);
			int* nPtr_1 = Intg.ptr<int>(r - 1);

			if (nPtr[Intg.cols - 1] - nPtr_1[Intg.cols - 1] < 5)
				continue;

			UCHAR* uPtr = localImg.ptr(r - 1);
			for (int c = CntX; c > 0; c--)
			{
				if (uPtr[c] > 0)
				{
					vSt[r - 1] = c;
					break;
				}
			}
			for (int c = CntX; c < localImg.cols - 1; c++)
			{
				if (uPtr[c] > 0)
				{
					vEd[r - 1] = c;
					break;
				}
			}
			int ndif = vEd[r - 1] - vSt[r - 1] > 0 ? vEd[r - 1] - vSt[r - 1] : 0;
			vHist[ndif]++;
			vCnt[r] = ndif;
		}

		int nMaxidx(0), nMaxCnt(0);
		for (int i = 0; i < vHist.size(); i++)
		{
			if (vHist[i] > nMaxCnt)
			{
				nMaxCnt = vHist[i];
				nMaxidx = i;
			}
		}

		int nStdMxL = 75 / (GetResolX() * 1000);

		int nMaxLength = nStdMxL;
		int nDamageSt(0), nDamageEd(0);
		int stCntIdx = 0, edCntIdx = 0;
		for (int i = 0; i < vCnt.size(); i++)
		{
			if (vCnt[i]< nMaxidx + 3 && vCnt[i] > nMaxidx - 3)
			{
				stCntIdx = i;
				break;
			}
		}
		for (int i = vCnt.size() - 1; i > 1; i--)
		{
			if (vCnt[i]< nMaxidx + 3 && vCnt[i] > nMaxidx - 3)
			{
				edCntIdx = i;
				break;
			}
		}

		for (int i = stCntIdx; i < edCntIdx; i++)
		{
			if (vCnt[i]< nMaxidx + 3 && vCnt[i] > nMaxidx - 3)
			{
				if (nMaxLength < 0)
				{
					nDamageEd = i;
					break;
				}
				nDamageSt = 0;
				nMaxLength = nStdMxL;
				continue;
			}
			if (nMaxLength >= nStdMxL)
				nDamageSt = i;
			nMaxLength--;
		}
		if (nDamageEd > 0)
		{
			re_Start->x = (vEd[nDamageSt] + vSt[nDamageSt]) / 2;
			re_Start->y = nDamageSt;
			re_End->x = (vEd[nDamageEd] + vSt[nDamageEd]) / 2;
			re_End->y = nDamageEd;
			bIsDetected = true;
		}


	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Algo::Insp_FootImprint() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return bIsDetected;
}

bool CPInsp_Foot::Insp_FootWidth(cv::Mat* vImage, cv::Point pSt, cv::Point pEd, bool isLeft, cv::Point* Re_start, cv::Point* Re_end)
{
	cv::Mat orgImg = *vImage;
	if (isLeft)
	{
		cv::Mat RectImage = orgImg(cv::Rect(0, pSt.y, orgImg.cols, pEd.y - pSt.y)).clone();

#if _DEBUG
		cv::imwrite("D:\\FootRst\\Insp_FootWidth_Left.bmp", RectImage);
#endif

		cv::Mat Intg, Dev;
		RectImage &= 1;
		cv::integral(RectImage, Intg, Dev);

		for (int c = 1; c < Intg.cols - 1; c++)
		{
			int* intPtr = Intg.ptr<int>(Intg.rows - 1);
			if (intPtr[c] > 0)
			{
				int harf = (RectImage.rows - 1) / 2;
				Re_start->x = c - 1;
				Re_start->y = harf + pSt.y;
				for (int r = 0; r < harf - 1; r++)
				{
					uchar* intRowPtr = RectImage.ptr(harf - r);
					if (intRowPtr[c - 1] > 0)
					{
						Re_start->x = c - 1;
						Re_start->y = harf - r + pSt.y;
						break;
					}
					uchar* intRowPtr2 = RectImage.ptr(harf + r);
					if (intRowPtr2[c - 1] > 0)
					{
						Re_start->x = c - 1;
						Re_start->y = harf + r + pSt.y;
						break;
					}
				}
				break;
			}
		}

	}
	else
	{
		cv::Mat RectImage = orgImg(cv::Rect(0, pSt.y, vImage->cols, pEd.y - pSt.y)).clone();

#if _DEBUG
		cv::imwrite("D:\\FootRst\\Insp_FootWidth_Right.bmp", RectImage);
#endif
		cv::Mat Intg, Dev;
		RectImage &= 1;
		cv::integral(RectImage, Intg, Dev);


		for (int c = Intg.cols - 1; c > 1; c--)
		{
			int* intPtr = Intg.ptr<int>(Intg.rows - 1);
			int ncol1 = intPtr[c - 1];
			int ncol = intPtr[c];
			int nCalc = ncol - ncol1;
			if (nCalc > 0)
			{
				int harf = (RectImage.rows - 1) / 2;
				Re_start->x = c - 2;
				Re_start->y = harf + pSt.y;
				for (int r = 0; r <= harf; r++)
				{
					uchar* intRowPtr = RectImage.ptr(harf - r);
					if (intRowPtr[c - 2] > 0)
					{
						Re_start->x = c - 2;
						Re_start->y = harf - r + pSt.y;
						break;
					}
					uchar* intRowPtr2 = RectImage.ptr(harf + r);
					if (intRowPtr2[c - 2] > 0)
					{
						Re_start->x = c - 2;
						Re_start->y = harf + r + pSt.y;
						break;
					}
				}
				break;
			}
		}
	}
	return true;
}

void CPInsp_Foot::Insp_Wedge_WidthLength(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat* Img, cv::Mat* Wing, cv::Mat Img3D, cv::Point& CenterPo, double& seta, cv::Mat &mMaskImage, cv::Mat &mWedgeMaxBlob, cv::Mat &PadRectImg, cv::Point pLSt, cv::Point pLEd, std::vector<cv::Point> &vPoSt, std::vector<cv::Point>& vPoEd, cv::Point* CtLineSt, cv::Point* CtLineEd, cv::Point pWingPosLeftX, cv::Point pWingPosRightX)
{
	if (pFoot == NULL || sRstAlgo == NULL)
		return;

	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	UCHAR *pUcImgBinary = Img->data;

	AlgoBlob sAlgoBlob = pFoot->GetAlgoBlob(m_eFootBin_Wedge);
	sAlgoBlob.m_bUseIPC = false;
	sAlgoBlob.m_bInvertCheck = false;
	sAlgoBlob.m_bFilterIsUse = false;
	sAlgoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;
	//sAlgoBlob.m_bInsp3D = true;
	sAlgoBlob.m_bInsp3D = false;
	sAlgoBlob.m_bInsp2D = true;
	sAlgoBlob.m_nTypeRange2D = 2;
	sAlgoBlob.m_nMinBinary = 80;
	sAlgoBlob.m_nMaxBinary = 100;
	sAlgoBlob.m_bFillHole = true;

	cv::Mat BinImage = Img->clone();

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;


	/////////////////////
	cv::Mat mMaskMoph;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
	cv::erode(mMaskImage, mMaskMoph, kernel);

	cv::Mat roImg;
	*Img &= mMaskMoph;
	roImg = *Img;
	//WarpAffine(*Img, roImg, seta, Img->size(), CenterPo);

	cv::Mat PadRangeImg(Img->rows, Img->cols, CV_8UC1);
	PadRangeImg.setTo(0);

// 	int pStartX = 0, pStartY=0;
// 	pStartX = 0;
// 	pStartY = roImg.cols;
// 
// 	if (pWingPosLeftX.x != 0 && pWingPosLeftX.x >0)
// 		pStartX = pWingPosLeftX.x;
// 
// 	if (pWingPosRightX.x != 0 && pWingPosRightX.x > 0)
// 		pStartY = pWingPosRightX.x;
// 
// 	int nRectWidth = 0;
// 
// 	nRectWidth = roImg.cols;
// 
// 	if( (pStartY - pStartX) !=0)
// 		nRectWidth = pStartY - pStartX;
// 
// 	cv::rectangle(PadRangeImg, cv::Rect(pStartX, pLSt.y, nRectWidth, pLEd.y - pLSt.y), cv::Scalar(255), cv::FILLED);
// #if _DEBUG
// 	cv::imwrite(("D:\\FootInspImg\\Wedge_PadRectImg.bmp"), PadRangeImg);
// #endif
// 
// 	PadRectImg = PadRangeImg;

	//Wing 이미지로 범위 넓히기 
	cv::Mat ImgWingBW(roImg.rows, roImg.cols, CV_8UC1);
	pFoot->GetImageMatrix(&ImgWingBW, (int)m_eFootBin::m_eFootBin_Wing);
	GetBinImage(pFoot, &ImgWingBW, m_eFootBin_Wing, NULL, false);

	cv::Mat roWingBWImg;
	WarpAffine(ImgWingBW, roWingBWImg, seta, Img->size(), CenterPo);

	//shkim 3번째 Wing blob은 제거
	sAlgoBlob.m_nTypeSelectBlob = _ETypeBlob::_eSelectBigger;
	cv::Mat fHRectImg(Wing->rows, Wing->cols, CV_32FC1, pFoot->pf3D);
	cv::Mat WingImgTemp = Wing->clone();
	cv::Point BSt1 = vPoSt[(int)m_eFoot::m_eFoot_Bonding1];
	cv::Point BEd1 = vPoEd[(int)m_eFoot::m_eFoot_Bonding1];
	cv::Point BSt2 = vPoSt[(int)m_eFoot::m_eFoot_Bonding2];
	cv::Point BEd2 = vPoEd[(int)m_eFoot::m_eFoot_Bonding2];
	cv::line(WingImgTemp, BSt1, BEd1, cv::Scalar(255));
	cv::line(WingImgTemp, BSt2, BEd2, cv::Scalar(255));
	PadRectImg = cv::Mat(WingImgTemp.rows, WingImgTemp.cols, CV_8UC1);
	PadRectImg.setTo(0);
	int nFilter = (sAlgoBlob.m_bFilterIsUse) ? sAlgoBlob.m_nFilterStepNarrow : 0;
	//int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, Wing->data, fHRectImg.ptr<float>(0), NULL, Wing->cols, Wing->rows, nMinblobArea, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, false);
	int nCntWingBlob = m_pProcMilAlgo->CalcBlob_Select(WingImgTemp.data, PadRectImg.data, WingImgTemp.cols, WingImgTemp.rows, 10, 0, false, nFilter, _ETypeBlob::_eSelectBigger, -1, -1, -1, -1, -1, -1,-1, true,2);


	int nWingRectMinX = PadRectImg.cols;
	int nWingRectMaxX = 0;

	int nWingRectMinY = PadRectImg.rows;
	int nWingRectMaxY = 0;

	for (int r = 0; r < PadRectImg.rows; r++)
	{
		UCHAR* Ptr = PadRectImg.data;
		for (int c = 0; c < (*Wing).cols; c++)
		{
			if (Ptr[r * (*Wing).step + c] == 255)
			{
				if (r < nWingRectMinY)	nWingRectMinY = r;
				if (r > nWingRectMaxY)	nWingRectMaxY = r;

				if (c < nWingRectMinX) nWingRectMinX = c;
				if (c > nWingRectMaxX) nWingRectMaxX = c;

			}
		}
	}
	cv::rectangle(PadRangeImg, cv::Rect(nWingRectMinX, nWingRectMinY, nWingRectMaxX- nWingRectMinX, nWingRectMaxY -nWingRectMinY), cv::Scalar(255), cv::FILLED);
#if _DEBUG
	cv::imwrite(("D:\\FootRst\\Wedge_PadRectImg.bmp"), PadRangeImg);
#endif
	PadRectImg = PadRangeImg;

	roImg &= PadRangeImg;
	bool bUseWingRet = false;

	if (Wing != NULL)
	{
		cv::Mat WingImg, retMatIntg, retDev;
		WingImg = *Wing;

		cv::integral(WingImg, retMatIntg, retDev);
		UINT* unREPtr = retMatIntg.ptr<UINT>(retMatIntg.rows - 1);
		if (unREPtr[retMatIntg.cols - 1] > 0)
			bUseWingRet = true;
// 
// 		cv::Point BSt1 = vPoSt[(int)m_eFoot::m_eFoot_Bonding1];
// 		cv::Point BEd1 = vPoEd[(int)m_eFoot::m_eFoot_Bonding1];
// 		cv::Point BSt2 = vPoSt[(int)m_eFoot::m_eFoot_Bonding2];
// 		cv::Point BEd2 = vPoEd[(int)m_eFoot::m_eFoot_Bonding2];

		int nCenterX = (BSt2.x + BSt1.x) / 2.;

		if (nCenterX == 0 || nCenterX >= roImg.cols)
			bUseWingRet = false;
	}
	if (bUseWingRet)
	{
		//Wing 진성에서 제외하자
#if _DEBUG
		cv::imwrite(("D:\\FootInspImg\\Wing_Result.bmp"), *Wing);
#endif
		//Bonding 영역에서 Wing Result Image를 빼자
		cv::Point BSt1 = vPoSt[(int)m_eFoot::m_eFoot_Bonding1];
		cv::Point BEd1 = vPoEd[(int)m_eFoot::m_eFoot_Bonding1];

		cv::Point BSt2 = vPoSt[(int)m_eFoot::m_eFoot_Bonding2];
		cv::Point BEd2 = vPoEd[(int)m_eFoot::m_eFoot_Bonding2];

		double ptrdLineX[2];
		double ptrdLineY[2];

		ptrdLineX[0] = (double)BSt1.x;
		ptrdLineY[0] = (double)BSt1.y;
		ptrdLineX[1] = (double)BEd1.x;
		ptrdLineY[1] = (double)BEd1.y;
		double LeftA = 0;
		double LeftB = 0;
		CPInsp::GetGradient(ptrdLineX, ptrdLineY, 2, LeftA, LeftB, false);

		ptrdLineX[0] = (double)BSt2.x;
		ptrdLineY[0] = (double)BSt2.y;
		ptrdLineX[1] = (double)BEd2.x;
		ptrdLineY[1] = (double)BEd2.y;
		double RightA = 0;
		double RightB = 0;
		CPInsp::GetGradient(ptrdLineX, ptrdLineY, 2, RightA, RightB, false);

		int nCenterX = (BSt2.x + BSt1.x) / 2.;

		roImg += ~*Wing;

#if _DEBUG
		//디버깅용 이미지 저장
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_Wedge_NotWing.bmp"), roImg);
		}
#endif

		roImg &= PadRangeImg;

		roImg &= BinImage;

#if _DEBUG
		//디버깅용 이미지 저장
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_Wedge_padRange.bmp"), PadRangeImg);
			cv::imwrite(rawname + std::string("_Wedge_padRangeAndNotWing.bmp"), roImg);
		}
#endif

		int nMinY = pLSt.y;
		int nMaxY = nMinY + (pLEd.y - pLSt.y);
		if (nMaxY >= roImg.rows)
			nMaxY = roImg.rows - 1;

		for (size_t y = nMinY; y <= nMaxY; y++)
		{
			UCHAR* ptr = roImg.ptr<UCHAR>(y);
			bool bFindWing = false;

			if (ptr[nCenterX] == 255)
			{
				bFindWing = false;
				int i = nCenterX;

				//Left
				for (; i >= 0; i--)
				{
					if (ptr[i] == 0)
					{
						bFindWing = true;
						break;
					}
				}

				if (bFindWing)
				{
					for (; i >= 0; i--)
						ptr[i] = 0;
				}
				else
				{
					i = nCenterX;

					int nLeftX = (int)(y*LeftA + LeftB);
					/*	for (; i >= nLeftX; i--)
							ptr[i] = 255;*/
					for (i = nLeftX; i >= 0; i--)
						ptr[i] = 0;
				}

				//Right
				bFindWing = false;
				i = nCenterX;
				for (; i < roImg.cols; i++)
				{
					if (ptr[i] == 0)
					{
						bFindWing = true;
						break;
					}
				}
				if (bFindWing)
				{
					for (; i < roImg.cols; i++)
						ptr[i] = 0;
				}
				else
				{
					i = nCenterX;
					int nRightX = (int)(y*RightA + RightB);
					for (i = nRightX; i < roImg.cols; i++)
						ptr[i] = 0;
					/*for (; i < nRightX; i++)
						ptr[i] = 255;*/
				}

			}
			else
			{
				if (ptr[0] == 255)
				{
					bool bFindBlack = false;
					int i = 0;
					for (; i < roImg.cols; i++)
					{
						if (ptr[i] == 0)
						{
							bFindBlack = true;
							break;
						}
					}

					for (int x = 0; x < i; x++)
					{
						ptr[x] = 0;
					}

				}

				if (ptr[roImg.cols - 1] == 255)
				{
					bool bFindBlack = false;
					int i = roImg.cols - 1;
					for (; i > 0; i--)
					{
						if (ptr[i] == 0)
						{
							bFindBlack = true;
							break;
						}
					}

					for (int x = roImg.cols - 1; x > i; x--)
					{
						ptr[x] = 0;
					}

				}
			}
		}
#if _DEBUG
		cv::imwrite(("D:\\FootInspImg\\Wing_Result2.bmp"), roImg);
#endif
	}

	cv::Point pWedgeLeftSt, pWedgeLeftEd, pWedgeRightSt, pWedgeRightEd;
#if _DEBUG
	cv::imwrite("D:\\FootInspImg\\Bin2D_WedgevRotHill.bmp", roImg);
#endif
	/////////////////////////////////


	double dArea, dCX, dCY;
	dCX = 0, dCY = 0, dArea = 0;
	CRect rcBlob;
	rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
	sAlgoBlob.m_nTypeSelectBlob = 1; //All Blob

	int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, roImg.data, Img3D.ptr<float>(0), NULL, Img->cols, Img->rows, 50, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);

	BinImage &= ~*Wing;
	BinImage.copyTo(*Img);

	if (pUcImgDst2D != NULL)
	{
		//delete[] pUcImgDst2D;
		g_pMManager->pem_delete(pUcImgDst2D, true);
		pUcImgDst2D = NULL;
	}
	if (pUcImgDst3D != NULL)
	{
		//delete[] pUcImgDst3D;
		g_pMManager->pem_delete(pUcImgDst3D, true);
		pUcImgDst3D = NULL;
	}
#if _DEBUG
	cv::imwrite(("D:\\FootInspImg\\Wedge_BinImage.bmp"), *Img);
#endif

	cv::Mat WedgeBlob(roImg.rows, roImg.cols, CV_8UC1);
	WedgeBlob = roImg.clone();
	mWedgeMaxBlob = WedgeBlob & PadRectImg;	//Wedge Bin Image Clone

#if _DEBUG
	cv::imwrite(("D:\\FootInspImg\\Wedge_Maxblob.bmp"), mWedgeMaxBlob);
#endif

#if _DEBUG
	//디버깅용 이미지 저장
	if (cstDebugFolderPath.IsEmpty() != TRUE)
	{
		CT2CA convertedString(cstDebugFolderPath);
		std::string strFootDebugImgPath = std::string(convertedString);

		size_t lastindex = strFootDebugImgPath.find_last_of(".");
		string rawname = strFootDebugImgPath.substr(0, lastindex);

		cv::imwrite(rawname + std::string("_Wedge_padRangeRect.bmp"), PadRectImg);
		cv::imwrite(rawname + std::string("_Wedge_MaxBlob.bmp"), mWedgeMaxBlob);
	}
#endif
	///////////////////////////////////////////////////////////////////////////////////////////////////////////


	//cv::imwrite(("D:\\FootInspImg\\wedge.bmp"), roImg);
	//SetInspImageData(&roImg, *left, (int)m_eFootBin::m_eFootBin_Wedge, sRstFootImage);

	bool bWedge = Insp_FootWedge(&mWedgeMaxBlob, pLSt, pLEd, &pWedgeLeftSt, &pWedgeLeftEd, &pWedgeRightSt, &pWedgeRightEd);

	//WedgeWidth 계산
	bool bWedgeWidth = true;
	cv::Mat WedgeRoImg = mWedgeMaxBlob.clone();

	//rect 구하기
	int ncvImgWidth = WedgeRoImg.cols;
	int ncvImgHeight = WedgeRoImg.rows;
	int nWedgeTmpMin_x = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_x = 0;
	int nWedgeTmpMin_y = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_y = 0;
	for (int r = 0; r < WedgeRoImg.rows; r++)
	{
		UCHAR* Ptr = WedgeRoImg.data;
		for (int c = 0; c < WedgeRoImg.cols; c++)
		{
			if (Ptr[r*WedgeRoImg.step + c] == 255)
			{
				if (c < nWedgeTmpMin_x) nWedgeTmpMin_x = c;
				if (c > nWedgeTmpMax_x) nWedgeTmpMax_x = c;
				if (r < nWedgeTmpMin_y) nWedgeTmpMin_y = r;
				if (r > nWedgeTmpMax_y) nWedgeTmpMax_y = r;
			}
		}
	}

	//wedge 결과 없을 시 함수 리턴
	if (nWedgeTmpMin_x == ncvImgWidth * ncvImgHeight && nWedgeTmpMin_y == ncvImgWidth * ncvImgHeight &&
		nWedgeTmpMax_x == 0 && nWedgeTmpMax_y == 0)
	{
#if FOOT_RST_WRITE
		CString sLog;
		sLog.Format(_T("CPInsp_Foot::Insp_Wedge_WidthLength nWedgeTmpMin_x == ncvImgWidth * ncvImgHeight && nWedgeTmpMin_y == ncvImgWidth * ncvImgHeight &&nWedgeTmpMax_x == 0 && nWedgeTmpMax_y == 0"));
		g_pMPTI->AddLog_OCR(sLog);
#endif

		sRstAlgo->m_bOK = FALSE;
		sRstAlgo->m_bFind = FALSE;

		for (int footType = 0; footType < m_eFoot_Total; footType++)
		{
			int footDataType = convertToFootData(footType);

			if ((pFoot->m_nUseOption & footDataType) == footDataType)
				sRstAlgo->m_bArrOK[footType] = FALSE;

		}

		return;
	}

	int wedgeRectWidth = nWedgeTmpMax_x - nWedgeTmpMin_x;
	int wedgeRectHeight = nWedgeTmpMax_y - nWedgeTmpMin_y;

	//WedgeRect 중심점
	int wedgeRectCenterX = nWedgeTmpMin_x + (int)round((float)wedgeRectWidth / 2.0f);
	int wedgeRectCenterY = nWedgeTmpMin_y + (int)round((float)wedgeRectHeight / 2.0f);

	int nFindCtLineX = 0;
	int nStX = 0;
	for (int y = 0; y < WedgeRoImg.rows; y++)
	{
		UCHAR* ptr = WedgeRoImg.ptr<UCHAR>(y);
		for (int x = nWedgeTmpMin_x; x < nWedgeTmpMax_x; x++)
		{
			if (y == wedgeRectCenterY)
			{
				if (ptr[x] == 255)
				{
					if (nStX == 0)
						nStX = x;

					nFindCtLineX++;
				}
			}
		}

	}

	int wedgeWidthStX = 0;
	int wedgeWidthStY = 0;
	int wedgeWidthEdX = 0;
	int wedgeWidthEdY = 0;

	if (nFindCtLineX != 0 && nStX != 0)
	{

		wedgeRectCenterX = nStX + (int)round((float)nFindCtLineX / 2.0f);

		//WedgeRect의 20% 지점 길이만 추출
		RECT rtWedgeRect20;
		rtWedgeRect20.left = nWedgeTmpMin_x;
		rtWedgeRect20.right = nWedgeTmpMax_x;
		rtWedgeRect20.top = nWedgeTmpMin_y + wedgeRectHeight * 0.4;
		rtWedgeRect20.bottom = nWedgeTmpMin_y + wedgeRectHeight * 0.6;

		if (nWedgeTmpMin_x == nWedgeTmpMax_x || (int)(wedgeRectHeight * 0.4) == (int)(wedgeRectHeight * 0.6))
		{
			if (nWedgeTmpMin_x == nWedgeTmpMax_x)
				rtWedgeRect20.right = nWedgeTmpMin_x + 1;
			if ((int)(wedgeRectHeight * 0.4) == (int)(wedgeRectHeight * 0.6))
			{
				rtWedgeRect20.top = nWedgeTmpMin_y + (wedgeRectHeight * 0.4);
				rtWedgeRect20.bottom = nWedgeTmpMin_y + wedgeRectHeight * 0.6 + 1;
			}
		}

		if (rtWedgeRect20.right - rtWedgeRect20.left == 0)
		{
			rtWedgeRect20.left = nWedgeTmpMin_x;
			rtWedgeRect20.right = nWedgeTmpMax_x;

		}
		if (rtWedgeRect20.bottom - rtWedgeRect20.top == 0)
		{
			rtWedgeRect20.top = nWedgeTmpMin_y;
			rtWedgeRect20.bottom = nWedgeTmpMax_y;
		}

		cv::Mat Wedge20percentRect = WedgeRoImg(cv::Rect(rtWedgeRect20.left, rtWedgeRect20.top, rtWedgeRect20.right - rtWedgeRect20.left, rtWedgeRect20.bottom - rtWedgeRect20.top));

		cv::Mat IntgWedge20per, Dev;
		cv::Mat inputWedge20per = Wedge20percentRect.clone();
		Wedge20percentRect &= 1;
		cv::integral(Wedge20percentRect, IntgWedge20per, Dev);

		int nWedgelength2 = 0;
		int sumWedgelen2 = 0;
		int lengthCnt = 0;
		int avgWedgeLength = 0;
		for (int r = IntgWedge20per.rows - 1; r > 1; r--)
		{
			int* intPtr_1 = IntgWedge20per.ptr<int>(r - 1);
			int* intPtr_0 = IntgWedge20per.ptr<int>(r);
			int nCols_1 = intPtr_1[IntgWedge20per.cols - 1];
			int nCols_0 = intPtr_0[IntgWedge20per.cols - 1];

			int nCalc = nCols_0 - nCols_1;
			if (nCalc > 0)
			{
				lengthCnt++;
				nWedgelength2 = nCalc;
				sumWedgelen2 = sumWedgelen2 + nWedgelength2;
			}
		}

// 		if (sumWedgelen2 == 0 || lengthCnt == 0)
// 		{
// #if FOOT_RST_WRITE
// 			CString sLog;
// 			sLog.Format(_T("CPInsp_Foot::Insp_Wedge_WidthLength sumWedgelen2 == 0 || lengthCnt == 0"));
// 			g_pMPTI->AddLog_OCR(sLog);
// #endif
// 
// 			sRstAlgo->m_bOK = FALSE;
// 			return;
// 		}

		if (sumWedgelen2 != 0 && lengthCnt != 0)
		{
			avgWedgeLength = (int)round((float)sumWedgelen2 / (float)lengthCnt);

			wedgeWidthStX = wedgeRectCenterX - (int)round((float)(avgWedgeLength) / 2.0f);
			wedgeWidthStY = wedgeRectCenterY;

			wedgeWidthEdX = wedgeRectCenterX + (int)round((float)(avgWedgeLength) / 2.0f);
			wedgeWidthEdY = wedgeRectCenterY;
		}
		else
		{
			wedgeWidthStX = nWedgeTmpMin_x;
			wedgeWidthStY = nWedgeTmpMin_y;

			wedgeWidthEdX = nWedgeTmpMax_x;
			wedgeWidthEdY = nWedgeTmpMax_y;
		}
	}
	else
	{
		wedgeWidthStX = nWedgeTmpMin_x;
		wedgeWidthStY = nWedgeTmpMin_y;

		wedgeWidthEdX = nWedgeTmpMax_x;
		wedgeWidthEdY = nWedgeTmpMax_y;
	}

	cv::Point pWedgeW_1_v2, pWedgeW_2_v2;
	pWedgeW_1_v2.x = wedgeWidthStX;
	pWedgeW_1_v2.y = wedgeWidthStY;

	pWedgeW_2_v2.x = wedgeWidthEdX;
	pWedgeW_2_v2.y = wedgeWidthEdY;
	//

	if (pWedgeLeftSt.x == 0 && pWedgeLeftSt.y == 0)
	{
		pWedgeLeftSt.x = pWedgeRightSt.x = CenterPo.x;
		pWedgeLeftSt.y = pWedgeRightSt.y = CenterPo.y;
	}
	if (pWedgeLeftEd.x == 0 && pWedgeLeftEd.y == 0)
	{
		pWedgeLeftEd.x = pWedgeRightEd.x = CenterPo.x;
		pWedgeLeftEd.y = pWedgeRightEd.y = CenterPo.y;
	}
	if (bWedge)
	{
		CtLineSt->x = pWedgeLeftSt.x + pWedgeRightSt.x;
		CtLineSt->y = pWedgeLeftSt.y + pWedgeRightSt.y;
		CtLineEd->x = pWedgeLeftEd.x + pWedgeRightEd.x;
		CtLineEd->y = pWedgeLeftEd.y + pWedgeRightEd.y;
	}
	cv::Point pWedgeW_1, pWedgeW_2, pWedgeH_1, pWedgeH_2;

	pWedgeW_1.x = (pWedgeLeftSt.x + pWedgeLeftEd.x) / 2;
	pWedgeW_1.y = (pWedgeLeftSt.y + pWedgeLeftEd.y) / 2;
	pWedgeW_2.x = (pWedgeRightSt.x + pWedgeRightEd.x) / 2;
	pWedgeW_2.y = (pWedgeRightSt.y + pWedgeRightEd.y) / 2;

	pWedgeH_1.x = (pWedgeLeftSt.x + pWedgeRightSt.x) / 2;
	pWedgeH_1.y = (pWedgeLeftSt.y + pWedgeRightSt.y) / 2;
	pWedgeH_2.x = (pWedgeLeftEd.x + pWedgeRightEd.x) / 2;
	pWedgeH_2.y = (pWedgeLeftEd.y + pWedgeRightEd.y) / 2;

	//vPoSt[(int)m_eFoot::m_eFoot_Wedge_W] = (pWedgeW_1);
	//vPoEd[(int)m_eFoot::m_eFoot_Wedge_W] = (pWedgeW_2);
	vPoSt[(int)m_eFoot::m_eFoot_Wedge_W] = (pWedgeW_1_v2);
	vPoEd[(int)m_eFoot::m_eFoot_Wedge_W] = (pWedgeW_2_v2);

	vPoSt[(int)m_eFoot::m_eFoot_Wedge_L] = (pWedgeH_1);
	vPoEd[(int)m_eFoot::m_eFoot_Wedge_L] = (pWedgeH_2);
#if _DEBUG
	ImagePointSave(_T("D:\\FootRst\\Bin2D_L_Wedge.bmp"), &roImg, vPoSt, vPoEd);
#endif
	///////////////////////////////////////////////////////////////////////////////
	int nMaxRow = pWedgeH_2.y < Img3D.rows ? pWedgeH_2.y : Img3D.rows;
	int nMinRow = (pWedgeH_1.y + nMaxRow) / 2;
	cv::Point pWireSt(pWedgeH_1.x, nMaxRow);
	bool bIsDetectedWireStPos = false;

	for (int r = nMinRow; r < nMaxRow; r++)
	{
		if (Img3D.ptr<float>(r)[pWireSt.x] > pFoot->m_fArrOptionValue[m_eFoot_Height][(int)m_eMMD::eMMD_Max])
		{
				pWireSt.y = r;
				bIsDetectedWireStPos = true;
			//break;
		}
		else
		{
			if (bIsDetectedWireStPos)
				pWireSt.y = nMaxRow;
		}
	}
	cv::rectangle(PadRectImg, cv::Rect(0, 0, PadRectImg.cols, pWedgeH_1.y), cv::Scalar(0), cv::FILLED);
	cv::rectangle(PadRectImg, cv::Rect(0, pWireSt.y, PadRectImg.cols, PadRectImg.rows - pWireSt.y), cv::Scalar(0), cv::FILLED);
	PadRectImg &= mMaskMoph;
#if _DEBUG
	cv::imwrite(("D:\\FootRst\\Wedge_PadRectImg.bmp"), PadRectImg);
#endif
}

bool CPInsp_Foot::Insp_FootWedge(cv::Mat* vImage, cv::Point pSt, cv::Point pEd, cv::Point* RE_LeftSt, cv::Point* RE_LeftEd, cv::Point* RE_RightSt, cv::Point* RE_RightEd)
{
	int nLeft(0), nRight(0), nTop(0), nBottom(0);
	cv::Mat orgImg = (*vImage)(cv::Rect(0, pSt.y, vImage->cols - 1, pEd.y - pSt.y)).clone();
#if _DEBUG
	cv::imwrite(("D:\\FootRst\\Insp_FootWedge_INPUT_Img.bmp"), orgImg);
#endif

	cv::Mat Intg, Dev;
	orgImg &= 1;
	cv::integral(orgImg, Intg, Dev);

	//left
	for (int c = 0; c < Intg.cols - 1; c++)
	{
		int* intPtr = Intg.ptr<int>(Intg.rows - 1);
		if (intPtr[c] > 0)
		{
			nLeft = c;
			break;
		}
	}
	//top
	for (int r = 0; r < Intg.rows - 1; r++)
	{
		int* intPtr = Intg.ptr<int>(r);
		if (intPtr[Intg.cols - 1] > 0)
		{
			nTop = r;
			break;
		}
	}

	//right
	for (int c = Intg.cols - 1; c > 1; c--)
	{
		int* intPtr = Intg.ptr<int>(Intg.rows - 1);
		int nCols_1 = intPtr[c - 1];
		int nCols_0 = intPtr[c];

		int nCalc = nCols_0 - nCols_1;
		if (nCalc > 0)
		{
			nRight = c - 1;
			break;
		}
	}
	//bottom
	for (int r = Intg.rows - 1; r > 1; r--)
	{
		int* intPtr_1 = Intg.ptr<int>(r - 1);
		int* intPtr_0 = Intg.ptr<int>(r);
		int nCols_1 = intPtr_1[Intg.cols - 1];
		int nCols_0 = intPtr_0[Intg.cols - 1];

		int nCalc = nCols_0 - nCols_1;
		if (nCalc > 0)
		{
			nBottom = r;
			break;
		}
	}

	RE_LeftSt->x = nLeft;
	RE_LeftSt->y = nTop + pSt.y;
	RE_LeftEd->x = nLeft;
	RE_LeftEd->y = nBottom + pSt.y;
	RE_RightSt->x = nRight;
	RE_RightSt->y = nTop + pSt.y;
	RE_RightEd->x = nRight;
	RE_RightEd->y = nBottom + pSt.y;

	std::vector<cv::Point> vPoSt;
	std::vector<cv::Point> vPoEd;
	vPoSt.push_back(*RE_LeftSt);
	vPoSt.push_back(*RE_RightSt);
	vPoEd.push_back(*RE_LeftEd);
	vPoEd.push_back(*RE_RightEd);

	//ImagePointSave(_T("D:\\Bin2D_Wedge_func.bmp"), vImage, vPoSt, vPoEd);
	return true;
}

bool CPInsp_Foot::Insp_FootLength(cv::Mat* vImage, cv::Point pLSt, cv::Point pLEd, cv::Point* pRSt, cv::Point* pREd)
{
	bool ret = true;

	std::vector<POINT> vst;
	std::vector<POINT> ved;
	vst.clear();
	ved.clear();

	cv::Mat rectImage = (*vImage)(cv::Rect(0, 0, vImage->cols, pLEd.y)).clone();
	rectImage &= 1;

#if _DEBUG
	cv::imwrite("D:\\FootRst\\Insp_FootLength_INPUT_Img.bmp", rectImage);
#endif

	cv::Mat Intg, Dev;
	cv::integral(rectImage, Intg, Dev);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Insp_FootLength_Integral_Img.bmp", rectImage);
#endif
	int r = 0;
	int rStand = (Intg.cols - 1) / 5;
	for (; r < Intg.rows; r++)
	{
		int* RPtr = Intg.ptr<int>(r);
		if (RPtr[Intg.cols - 1]/* > rStand*/)
			break;
	}

	if (r > Intg.rows - 1)
	{
		POINT po;
		po.x = -1;
		po.y = -1;
		vst.push_back(po);
		ved.push_back(po);

		pRSt->x = (vst[0].x + ved[0].x) / 2;
		pREd->x = (vst[0].x + ved[0].x) / 2;

		pRSt->y = vst[0].y;
		pREd->y = pLEd.y;
#if FOOT_RST_WRITE
		CString sLog;
		sLog.Format(_T("CPInsp_Foot::Insp_FootLength r > Intg.rows - 1"));
		g_pMPTI->AddLog_OCR(sLog);
#endif
		return false;
	}

	if (rectImage.rows <= r) 
	{
#if FOOT_RST_WRITE
		CString sLog;
		sLog.Format(_T("CPInsp_Foot::Insp_FootLength rectImage.rows <= r"));
		g_pMPTI->AddLog_OCR(sLog);
#endif
		return false;
	}

	UCHAR* CPtr = rectImage.ptr(r);
	for (int c = 0; c < rectImage.cols - 1; c++)
	{
		if (CPtr[c] > 0)
		{
			POINT po;
			po.x = c;
			po.y = r;
			vst.push_back(po);
			break;
		}
	}
	for (int c = rectImage.cols - 1; c > 0; c--)
	{
		if (CPtr[c] > 0)
		{
			POINT po;
			po.x = c;
			po.y = r;
			ved.push_back(po);
			break;
		}
	}

	if (vst.size() < 1)
	{
		POINT po;
		po.x = pLEd.x;
		po.y = r;
		vst.push_back(po);
		ret = false;
	}
	if (ved.size() < 1)
	{
		POINT po;
		po.x = pLEd.x;
		po.y = pLEd.y;
		ved.push_back(po);
		ret = false;
	}
	for (int rEd = Intg.rows - 1; rEd > r; rEd--)
	{
		int* RPtr = Intg.ptr<int>(rEd);
		int* RPtr_before = Intg.ptr<int>(rEd - 1);
		if (RPtr[Intg.cols - 1] - RPtr_before[Intg.cols - 1] > 0)
		{
			pLEd.y = rEd < pLEd.y ? rEd : pLEd.y;
			break;
		}
	}

	pRSt->x = (vst[0].x + ved[0].x) / 2;
	pREd->x = (vst[0].x + ved[0].x) / 2;

	pRSt->y = vst[0].y;
	pREd->y = pLEd.y;

	return ret;
}

cv::Point CPInsp_Foot::WarpAffine(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev)
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
	cv::Mat RotImage;
	cv::Mat rotation = getRotationMatrix2D(NCenter, Seta, 1);

	if (CV_MAT_TYPE(orgImg.type()) == CV_32FC1)
		cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_LINEAR);
	else
		cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_NEAREST);

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

cv::Mat CPInsp_Foot::Rot_FootAng(cv::Mat * vImage, double fResAng, bool bInterpolation)
{
	cv::Point2f point_center(vImage->cols / 2.0, vImage->rows / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(point_center, fResAng, 1.0);

	cv::Rect bbox = cv::RotatedRect(point_center, vImage->size(), fResAng).boundingRect();

	rot.at<double>(0, 2) += bbox.width / 2.0 - point_center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - point_center.y;

	cv::Mat m_img_dst = cv::getRotationMatrix2D(point_center, fResAng, 1.0);

	int flag = bInterpolation == true ? cv::INTER_LINEAR : cv::INTER_NEAREST;

	cv::warpAffine(*vImage, m_img_dst, rot, bbox.size(), flag);

	return m_img_dst;
}

bool CPInsp_Foot::Insp_FootPos(cv::Mat* vImage, cv::Point* CenterPo, double* seta)
{
	int nLine = __LINE__;
	cv::Point* WhitPos = NULL;
	try
	{
		int cnt(0), sumX(0), sumY(0);
		int MaxCnt = vImage->rows*vImage->cols;
		//WhitPos = new cv::Point[MaxCnt];
		//g_pMManager->pem_new_check(WhitPos, (PCHAR)__FUNCTION__, __LINE__);
		WhitPos = g_pMManager->pem_new<cv::Point>(true, MaxCnt, (PCHAR)__FUNCTION__, __LINE__);

		for (int r = 0; r < vImage->rows; r++)
		{
			uchar* ImgRow = vImage->ptr(r);
			for (int c = 0; c < vImage->cols; c++)
			{
				if (ImgRow[c] == 255)
				{
					WhitPos[cnt].x = c;
					WhitPos[cnt].y = r;
					sumX += c;
					sumY += r;
					cnt++;
				}
			}
		}
		if (cnt < 4)
		{
			if (WhitPos != NULL)
			{
				//delete[] WhitPos;
				g_pMManager->pem_delete(WhitPos, true);

				WhitPos = NULL;
			}
			return false;
		}
		float a(0), b(0);
		CenterPo->x = sumX / cnt;
		CenterPo->y = sumY / cnt;
		PCA_1D(cnt, WhitPos, *CenterPo, &a, &b);
		*seta = std::atan(a) * 180 / PI;
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Algo::Insp_FootPos() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
		if (WhitPos != NULL)
		{
			//delete[] WhitPos;
			g_pMManager->pem_delete(WhitPos, true);

			WhitPos = NULL;
		}
		throw;
	}
	if (WhitPos != NULL)
	{
		//delete[] WhitPos;
		g_pMManager->pem_delete(WhitPos, true);
		WhitPos = NULL;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//shkim 2019.02.18 // creat for foot Inspection
void CPInsp_Foot::PCA_1D(int cnt, cv::Point* pint, cv::Point Center, float* a, float* b)
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
	cv::eigen(covar, eigenVal, eigenMt);
	float* tmpV = eigenVal.ptr<float>(0);
	float* tmpM = eigenMt.ptr<float>(0);
	float tmpA = tmpM[2];
	float tmpB = tmpM[3];
	float tmpx1 = Center.x;
	float tmpy1 = Center.y;

	*a = tmpB == 0 ? tmpB : -tmpA / tmpB;
	*b = tmpB == 0 ? tmpy1 : tmpA / tmpB * tmpx1 + tmpy1;
}
void CPInsp_Foot::PCA_1D_OrgVal(int cnt, cv::Point* pint, cv::Point Center, float* a, float* b, float* c, float* d)
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
	cv::eigen(covar, eigenVal, eigenMt);
	float* tmpV = eigenVal.ptr<float>(0);
	float* tmpM = eigenMt.ptr<float>(0);
	float tmpA = tmpM[2];
	float tmpB = tmpM[3];
	float tmpx1 = Center.x;
	float tmpy1 = Center.y;

	*a = tmpA;
	*b = tmpB;
	*c = tmpV[0];
	*d = tmpV[1];
}
void CPInsp_Foot::OLS_1D_2(int cnt, cv::Point* pint, float* a, float* b, float* c)
{

	if (cnt <= 1)
		return;
	cv::Mat matrixX(cnt, 3, CV_32F), matrixXT, matrixY(cnt, 1, CV_32F);

	double dAvg(0.0);
	double zeroPy = pint[0].y;

	for (int i = 0; i < cnt; i++)
	{
		float* fptr = matrixX.ptr<float>(i);
		float* fptry = matrixY.ptr<float>(i);

		fptr[0] = std::pow((double)pint[i].x, 2);
		fptr[1] = pint[i].x;
		fptr[2] = 1;

		fptry[0] = pint[i].y;
		dAvg += pint[i].y;
	}

	cv::Mat Xt = matrixX.t();
	cv::Mat XX = Xt * matrixX;
	cv::Mat XY = Xt * matrixY;
	cv::Mat res = XX.inv() * XY;;

#if _DEBUG
	float* XXptr = XX.ptr<float>(0);
	float* XTptr = Xt.ptr<float>(0);
	float* XYptr = XY.ptr<float>(0);
#endif

	float* Reptr = res.ptr<float>(0);
	*a = Reptr[0];
	*b = Reptr[1];

	double baseAvg = dAvg / cnt;
	double Calc_y = Reptr[0] * pint[0].x*pint[0].x + Reptr[1] * pint[0].x + baseAvg;
	double marginY = zeroPy - Calc_y;

	*c = marginY + baseAvg;
}
void CPInsp_Foot::RANSAC_1D(std::vector<cv::Point> AvgC, std::vector<cv::Point>* RanC, float MaxDiff)
{
	int nRAN_SamplCnt = 3;
	int nInnerCnt = 0;
	int nInnerMax = 0;

	int nTotSamCnt = AvgC.size();
	int nDiv = nTotSamCnt / nRAN_SamplCnt;

	std::vector<cv::Point> RanVector, TmpVector;
	RanVector.clear();
	TmpVector.clear();
	TmpVector.resize(nRAN_SamplCnt);
	srand((unsigned)time(NULL));

	for (int i = 0; i < nTotSamCnt / nRAN_SamplCnt; i++)
	{
		RanVector.clear();
		int nRanIdx1_Tmp = std::rand();
		int nRanIdx2_Tmp = std::rand();
		int nRanIdx3_Tmp = std::rand();

		int nRanIdx1 = (int)(((float)(nRanIdx1_Tmp % 10) / 10)*(nTotSamCnt / nRAN_SamplCnt));
		int nRanIdx2 = (int)(((float)(nRanIdx2_Tmp % 10) / 10)*(nTotSamCnt / nRAN_SamplCnt) + nDiv);
		int nRanIdx3 = (int)(((float)(nRanIdx3_Tmp % 10) / 10)*(nTotSamCnt / nRAN_SamplCnt) + nDiv * 2);
		TmpVector[0] = AvgC[nRanIdx1];
		TmpVector[1] = AvgC[nRanIdx2];
		TmpVector[2] = AvgC[nRanIdx3];
		float a1(0), b1(0), c1(0);
		OLS_1D_2(TmpVector.size(), &TmpVector[0], &a1, &b1, &c1);

		for (int idx = 0; idx < nTotSamCnt; idx++)
		{
			int x = AvgC[idx].x;
			int y = a1 * std::pow((double)x, 2) + b1 * x + c1;

			if (std::abs(AvgC[idx].y - y) < MaxDiff)
			{
				RanVector.push_back(AvgC[idx]);
			}
		}

		if (RanC->size() < RanVector.size())
		{
			RanC->clear();
			RanC->resize(RanVector.size());
			for (int idx = 0; idx < RanC->size(); idx++)
			{
				(*RanC)[idx] = RanVector[idx];
			}
		}
	}
}
void CPInsp_Foot::RANSAC_1D_All(std::vector<cv::Point> AvgC, std::vector<cv::Point>* RanC, float MaxDiff)
{
	int nRAN_SamplCnt = 3;
	int nInnerCnt = 0;
	int nInnerMax = 0;

	int nTotSamCnt = AvgC.size();
	int nDiv = nTotSamCnt / nRAN_SamplCnt;

	std::vector<cv::Point> RanVector;
	RanVector.clear();
	srand((unsigned)time(NULL));

	for (int i = 0; i < nTotSamCnt; i++)
	{
		for (int j = nTotSamCnt - 1; j > i; j--)
		{

			RanVector.clear();
			float a1(0), b1(0);

			a1 = (AvgC[j].y - AvgC[i].y) / (AvgC[j].x - AvgC[i].x);
			b1 = AvgC[i].y - a1 * AvgC[i].x;

			for (int idx = 0; idx < nTotSamCnt; idx++)
			{
				int x = AvgC[idx].x;
				int y = a1 * x + b1;

				if (std::abs(AvgC[idx].y - y) < MaxDiff)
				{
					RanVector.push_back(AvgC[idx]);
				}
			}

			if (RanC->size() < RanVector.size())
			{
				RanC->clear();
				RanC->resize(RanVector.size());
				for (int idx = 0; idx < RanC->size(); idx++)
				{
					(*RanC)[idx] = RanVector[idx];
				}
			}
		}
	}



}


bool CPInsp_Foot::Calc_FootAng(cv::Mat* vImage, double & ang)
{
	cv::Mat GIn(vImage->rows, vImage->cols, CV_8UC1);
	cv::GaussianBlur(*vImage, GIn, cv::Size(11, 11), 0);

	cv::Mat EdgeBin = cv::Mat::zeros(vImage->rows, vImage->cols, CV_8UC1);
	cv::Mat gradX(vImage->rows, vImage->cols, CV_32FC1);
	cv::Mat gradY(vImage->rows, vImage->cols, CV_32FC1);
	cv::Sobel(GIn, gradX, CV_32F, 1, 0, 5);
	cv::Sobel(GIn, gradY, CV_32F, 0, 1, 5);

	cv::Mat mHist = cv::Mat::zeros(1, 360, CV_32SC1);
	int * nHist = mHist.ptr<int>(0);
	for (int y = 0; y < vImage->rows; y++)
	{
		float * xG = gradX.ptr<float>(y);
		float * yG = gradY.ptr<float>(y);
		for (int x = 0; x < vImage->cols; x++)
		{
			if (xG[x] == 0 && yG[x] == 0)
				continue;

			//float mag = sqrt(xG[x] * xG[x] + yG[x] * yG[x]);
			float mag = xG[x] * xG[x] + yG[x] * yG[x];

			float angle = cv::fastAtan2(xG[x], yG[x]);
			if (angle > 180.0f)
				angle -= 180.0f;
			int nAng = (int)(angle * 1.0f);
			if (mag > 5 * 5)
				//if (mag > 5)
				nHist[nAng]++;
		}
	}

	int nMaxHistIdx(-1);
	double nMaxHistVal(FLT_MIN);
	for (int n = 0; n < 360; n++)
	{
		if (nHist[n] > nMaxHistVal)
		{
			nMaxHistVal = nHist[n];
			nMaxHistIdx = n;
		}
	}


	for (int y = 0; y < vImage->rows; y++)
	{
		BYTE * resBin = EdgeBin.ptr(y);
		float * xG = gradX.ptr<float>(y);
		float * yG = gradY.ptr<float>(y);
		for (int x = 0; x < vImage->cols; x++)
		{
			float angle = 0.;
			if (xG[x] != 0 && yG[x] != 0)
				angle = cv::fastAtan2(xG[x], yG[x]);

			int nAng = (int)(angle * 1.0f);

			if (abs(nAng - nMaxHistIdx) < 10)
			{
				resBin[x] = 255;
			}
		}
	}

	CHistogramAnalysis_New ha_Angle;
	ha_Angle.Run(mHist);

	if (ha_Angle.m_PeakList.size() <= 0)
		return false;

	nMaxHistIdx = -1;
	nMaxHistVal = INT_MIN;
	for (int n = 0; n < ha_Angle.m_PeakList.size(); n++)
	{
		if (nMaxHistVal < ha_Angle.m_PeakList[n].dValue)
		{
			nMaxHistVal = ha_Angle.m_PeakList[n].dValue;
			nMaxHistIdx = ha_Angle.m_PeakList[n].nIndex;
		}
	}

	float fResAngOrg = (float)(nMaxHistIdx);
	float fResAng = 90.0f - fResAngOrg;

	ang = fResAng;

	return true;
}

void CPInsp_Foot::CalcRotPnt(cv::Point pCenter, double dAngle, cv::Point& Po)
{
	cv::Point vStart, vEnd;


	vStart.x = Po.x - pCenter.x;
	vStart.y = Po.y - pCenter.y;
	double dRadian = dAngle / 180 * PI;

	Po.x = vStart.x*std::cos(dRadian) + vStart.y*std::sin(dRadian) + pCenter.x;
	Po.y = (-vStart.x)*std::sin(dRadian) + vStart.y*std::cos(dRadian) + pCenter.y;
}
void CPInsp_Foot::CalcRotPnt(cv::Point pCenter, double dAngle, std::vector<cv::Point>& vPoSt, std::vector<cv::Point>& vPoEd, cv::Point Margin)
{
	std::vector<cv::Point> vStart, vEnd;
	vStart.resize(vPoSt.size());
	vEnd.resize(vPoEd.size());

	cv::Point NCenter = pCenter;
	NCenter.x = NCenter.x - Margin.x;
	NCenter.y = NCenter.y - Margin.y;

	for (int i = 0; i < vPoSt.size(); i++)
	{
		if (vPoSt[i].x == -1 && vPoSt[i].y == -1)
		{
			vStart[i].x = -1;
			vStart[i].y = -1;
		}
		vStart[i].x = vPoSt[i].x - NCenter.x;
		vStart[i].y = vPoSt[i].y - NCenter.y;
	}
	for (int i = 0; i < vPoEd.size(); i++)
	{
		if (vPoEd[i].x == -1 && vPoEd[i].y == -1)
		{
			vEnd[i].x = -1;
			vEnd[i].y = -1;
		}
		vEnd[i].x = vPoEd[i].x - NCenter.x;
		vEnd[i].y = vPoEd[i].y - NCenter.y;
	}

	double dRadian = dAngle / 180 * PI;

	for (int i = 0; i < vPoSt.size(); i++)
	{
		if (vStart[i].x == -1 && vStart[i].y == -1)
		{
			vPoSt[i].x = -1;
			vPoSt[i].y = -1;
		}
		vPoSt[i].x = std::roundf(vStart[i].x*std::cos(dRadian) + vStart[i].y*std::sin(dRadian) + pCenter.x);
		vPoSt[i].y = std::roundf((-vStart[i].x)*std::sin(dRadian) + vStart[i].y*std::cos(dRadian) + pCenter.y);
	}
	for (int i = 0; i < vPoEd.size(); i++)
	{
		if (vEnd[i].x == -1 && vEnd[i].y == -1)
		{
			vPoEd[i].x = -1;
			vPoEd[i].y = -1;
		}
		vPoEd[i].x = std::roundf(vEnd[i].x*std::cos(dRadian) + vEnd[i].y*std::sin(dRadian) + pCenter.x);
		vPoEd[i].y = std::roundf((-vEnd[i].x)*std::sin(dRadian) + vEnd[i].y*std::cos(dRadian) + pCenter.y);
	}
}

void CPInsp_Foot::CalcDisplayPnt(double dAngle, std::vector<cv::Point> vPoSt, std::vector<cv::Point> vPoEd, std::vector<cv::Point>& vPoDispSt, std::vector<cv::Point>& vPoDispEd)
{
	int nLine = __LINE__;
	try
	{
		vPoDispSt.resize(vPoSt.size());
		vPoDispEd.resize(vPoEd.size());
		//(int)m_eFoot::m_eFoot_Total
		int nRight = vPoSt[(int)m_eFoot::m_eFoot_Width_1].x + 20;
		int nLeft = vPoSt[(int)m_eFoot::m_eFoot_Width_1_H].x - 20;
		int nTop = vPoSt[(int)m_eFoot::m_eFoot_Length_1].y - 20;
		int nBot = vPoEd[(int)m_eFoot::m_eFoot_Length_1].y + 20;

		int selectPoX = nRight;
		int selectPoY = nTop;
		int wterm = -5;
		if (dAngle > 90 || dAngle > 270 || dAngle < -90)
		{
			selectPoX = nLeft;
			wterm = 5;
		}

		//	m_eFoot_Width_1,
		//	m_eFoot_Width_1_H,
		//	m_eFoot_Length_1,
		//	m_eFoot_Tail_L,
		//	m_eFoot_Tail_A,
		//	m_eFoot_Wedge_W,
		//	m_eFoot_Wedge_L,

		vPoDispSt[(int)m_eFoot::m_eFoot_Width_1].x = vPoSt[(int)m_eFoot::m_eFoot_Width_1].x;
		vPoDispSt[(int)m_eFoot::m_eFoot_Width_1].y = selectPoY;
		vPoDispEd[(int)m_eFoot::m_eFoot_Width_1].x = vPoSt[(int)m_eFoot::m_eFoot_Width_1_H].x;
		vPoDispEd[(int)m_eFoot::m_eFoot_Width_1].y = selectPoY;

		vPoDispSt[(int)m_eFoot::m_eFoot_Length_1].x = selectPoX;
		vPoDispSt[(int)m_eFoot::m_eFoot_Length_1].y = vPoSt[(int)m_eFoot::m_eFoot_Length_1].y;
		vPoDispEd[(int)m_eFoot::m_eFoot_Length_1].x = selectPoX;
		vPoDispEd[(int)m_eFoot::m_eFoot_Length_1].y = vPoEd[(int)m_eFoot::m_eFoot_Length_1].y;

		vPoDispSt[(int)m_eFoot::m_eFoot_Tail_L].x = selectPoX + wterm;
		vPoDispSt[(int)m_eFoot::m_eFoot_Tail_L].y = vPoSt[(int)m_eFoot::m_eFoot_Tail_L].y;
		vPoDispEd[(int)m_eFoot::m_eFoot_Tail_L].x = selectPoX + wterm;
		vPoDispEd[(int)m_eFoot::m_eFoot_Tail_L].y = vPoEd[(int)m_eFoot::m_eFoot_Tail_L].y;

		vPoDispSt[(int)m_eFoot::m_eFoot_Wedge_W].x = vPoSt[(int)m_eFoot::m_eFoot_Wedge_W].x;
		vPoDispSt[(int)m_eFoot::m_eFoot_Wedge_W].y = selectPoY + 5;
		vPoDispEd[(int)m_eFoot::m_eFoot_Wedge_W].x = vPoEd[(int)m_eFoot::m_eFoot_Wedge_W].x;
		vPoDispEd[(int)m_eFoot::m_eFoot_Wedge_W].y = selectPoY + 5;

		vPoDispSt[(int)m_eFoot::m_eFoot_Wedge_L].x = selectPoX + wterm;
		vPoDispSt[(int)m_eFoot::m_eFoot_Wedge_L].y = vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y;
		vPoDispEd[(int)m_eFoot::m_eFoot_Wedge_L].x = selectPoX + wterm;
		vPoDispEd[(int)m_eFoot::m_eFoot_Wedge_L].y = vPoEd[(int)m_eFoot::m_eFoot_Wedge_L].y;


		vPoDispSt[(int)m_eFoot::m_eFoot_Bonding1].x = selectPoX + wterm;
		vPoDispSt[(int)m_eFoot::m_eFoot_Bonding1].y = vPoSt[(int)m_eFoot::m_eFoot_Bonding1].y;
		vPoDispEd[(int)m_eFoot::m_eFoot_Bonding1].x = selectPoX + wterm;
		vPoDispEd[(int)m_eFoot::m_eFoot_Bonding1].y = vPoEd[(int)m_eFoot::m_eFoot_Bonding1].y;

		vPoDispSt[(int)m_eFoot::m_eFoot_Bonding2].x = selectPoX + wterm;
		vPoDispSt[(int)m_eFoot::m_eFoot_Bonding2].y = vPoSt[(int)m_eFoot::m_eFoot_Bonding2].y;
		vPoDispEd[(int)m_eFoot::m_eFoot_Bonding2].x = selectPoX + wterm;
		vPoDispEd[(int)m_eFoot::m_eFoot_Bonding2].y = vPoEd[(int)m_eFoot::m_eFoot_Bonding2].y;
	}
	catch (...)
	{
		CString err;
		err.Format(_T("CalcDisplayPnt Line: %d pass"), nLine);
		g_pMPTI->AddLog(err);
	}
}

void CPInsp_Foot::ImagePointSave(CString sPath, cv::Mat* vImage, std::vector<cv::Point> vPoSt, std::vector<cv::Point> vPoEd)
{
#if _DEBUG
	cv::Mat colorImg;
	cv::cvtColor(*vImage, colorImg, cv::COLOR_GRAY2RGB);
	for (int i = 0; i < vPoSt.size(); i++)
	{
		if ((int)m_eFoot_HeelCrack == i || (int)m_eFoot_Width_1 == i)
			continue;
		cv::line(colorImg, vPoSt[i], vPoEd[i], cv::Scalar(0, 0, 255));
	}
	cv::imwrite(std::string(CT2A(sPath)), colorImg);
#endif
}

void CPInsp_Foot::ImageWingPointSave_forDebug(CString sPath, cv::Mat* vImage, cv::Point vPoSt, cv::Point vPoEd, int nStAlgoNum, int nEndAlgoNum, bool bUseBothSideRst, int nStAlgoOpt, int nEndAlgoOpt)
{
#if _DEBUG
	cv::Mat colorImg;

	int nType = (vImage)->type();
	uchar chans = 1 + (nType >> CV_CN_SHIFT);

	if (bUseBothSideRst)
	{
		if (int(chans) == 1)
			cv::cvtColor(*vImage, *vImage, cv::COLOR_GRAY2BGR);

		cv::line(*vImage, vPoSt, vPoEd, cv::Scalar(0, 0, 255), 2);
	}
	else
	{
		cv::cvtColor(*vImage, colorImg, cv::COLOR_GRAY2BGR);
		cv::line(colorImg, vPoSt, vPoEd, cv::Scalar(0, 0, 255), 2);
	}
	/// Text
	int myFontFace = 1;	// Font Face
	double myFontScale = 1;	// Font Scale

	if (nStAlgoNum != -1)
	{
		CString cstrStAlgoIndex;
		if (bUseBothSideRst)
			cstrStAlgoIndex.Format(_T("S_%d"), nStAlgoNum);
		else
			cstrStAlgoIndex.Format(_T("LeftAlgo_%d"), nStAlgoNum);

		CT2CA cvtStringAlgoSt(cstrStAlgoIndex);

		std::string myText = std::string(cvtStringAlgoSt);

		/// Text Location
		cv::Point myPoint;
		if (bUseBothSideRst)
		{
			myPoint.x = vPoSt.x - 10;
			myPoint.y = vPoSt.y - 10;
		}
		else
		{
			myPoint.x = colorImg.cols / 5;
			myPoint.y = colorImg.rows - 32;
		}

		if (bUseBothSideRst)
			cv::putText(*vImage, myText, myPoint, myFontFace, myFontScale, cv::Scalar::all(255));
		else
			cv::putText(colorImg, myText, myPoint, myFontFace, myFontScale, cv::Scalar::all(255));
	}
	else
	{
		CString cstrStAlgoIndex;
		if (bUseBothSideRst)
			cstrStAlgoIndex.Format(_T("S_%dfail"), nStAlgoOpt);
		else
			cstrStAlgoIndex.Format(_T("LeftAlgo_%dfail"), nStAlgoOpt);

		CT2CA cvtStringAlgoSt(cstrStAlgoIndex);

		cv::Point myPoint;
		if (bUseBothSideRst)
		{
			myPoint.x = vPoSt.x - 10;
			myPoint.y = vPoSt.y - 10;
		}
		else
		{
			myPoint.x = colorImg.cols / 5;
			myPoint.y = colorImg.rows - 32;
		}

		std::string myText = std::string(cvtStringAlgoSt);
		if (bUseBothSideRst)
			cv::putText(*vImage, myText, myPoint, myFontFace, myFontScale, cv::Scalar::all(255));
		else
			cv::putText(colorImg, myText, myPoint, myFontFace, myFontScale, cv::Scalar::all(255));

	}

	if (nEndAlgoNum != -1)
	{
		CString cstrEndAlgoIndex;

		if (bUseBothSideRst)
			cstrEndAlgoIndex.Format(_T("E_%d"), nEndAlgoNum);
		else
			cstrEndAlgoIndex.Format(_T("RightAlgo_%d"), nEndAlgoNum);

		CT2CA cvtStringAlgoEnd(cstrEndAlgoIndex);

		std::string myTextEnd = std::string(cvtStringAlgoEnd);

		/// Text Location
		cv::Point myPointEnd;
		if (bUseBothSideRst)
		{
			myPointEnd.x = vPoEd.x - 10;
			myPointEnd.y = vPoEd.y + 10;
		}
		else
		{
			myPointEnd.x = colorImg.cols / 5;
			myPointEnd.y = colorImg.rows - 15;
		}

		if (bUseBothSideRst)
			cv::putText(*vImage, myTextEnd, myPointEnd, myFontFace, myFontScale, cv::Scalar::all(255));
		else
			cv::putText(colorImg, myTextEnd, myPointEnd, myFontFace, myFontScale, cv::Scalar::all(255));
	}
	else
	{
		CString cstrEndAlgoIndex;
		if (bUseBothSideRst)
			cstrEndAlgoIndex.Format(_T("E_%dfail"), nEndAlgoOpt);
		else
			cstrEndAlgoIndex.Format(_T("RightAlgo_%dfail"), nEndAlgoOpt);

		CT2CA cvtStringAlgoEnd(cstrEndAlgoIndex);

		cv::Point myPointEnd;
		if (bUseBothSideRst)
		{
			myPointEnd.x = vPoEd.x - 10;
			myPointEnd.y = vPoEd.y + 10;
		}
		else
		{
			myPointEnd.x = colorImg.cols / 5;
			myPointEnd.y = colorImg.rows - 32;
		}

		std::string myTextEnd = std::string(cvtStringAlgoEnd);
		if (bUseBothSideRst)
			cv::putText(*vImage, myTextEnd, myPointEnd, myFontFace, myFontScale, cv::Scalar::all(255));
		else
			cv::putText(colorImg, myTextEnd, myPointEnd, myFontFace, myFontScale, cv::Scalar::all(255));
	}
	//
	if (bUseBothSideRst)
		cv::imwrite(std::string(CT2A(sPath)), *vImage);
	else
		cv::imwrite(std::string(CT2A(sPath)), colorImg);
#endif
}


void CPInsp_Foot::GetImageMatrix(cv::Mat* Image, AlgoBin bin, InspRoiImgBuf* m_ImageBuffer, cv::Rect ImageRect)
{
	int width = m_ImageBuffer->nImageSizeX;
	int height = m_ImageBuffer->nImageSizeY;

	int ArrLightPosition[10] = { 0, };
	int	ArrCalculation[10] = { 0, };
	int	ArrRedValue[10] = { 0, };
	int	ArrBlueValue[10] = { 0, };
	int ArrGreenValue[10] = { 0, };
	int ArrWhiteValue[10] = { 0, };

	int LightCnt = bin.m_byArrValue[4];
	if (LightCnt > 0)
	{
		for (int c = 0; c < LightCnt; c++)
		{
			ArrLightPosition[c] = (int)bin.m_byArrLightData[0][c];
			ArrCalculation[c] = (int)bin.m_byArrLightData[1][c];
			ArrRedValue[c] = (int)bin.m_byArrLightData[2][c];
			ArrBlueValue[c] = (int)bin.m_byArrLightData[3][c];
			ArrGreenValue[c] = (int)bin.m_byArrLightData[4][c];
			ArrWhiteValue[c] = (int)bin.m_byArrLightData[5][c];
		}
	}
	else
	{
		LightCnt = 0;
		ArrLightPosition[0] = (int)bin.m_byArrLightData[0][0];
		ArrCalculation[0] = (int)bin.m_byArrLightData[1][0];
		ArrRedValue[0] = (int)bin.m_byArrLightData[2][0];
		ArrBlueValue[0] = (int)bin.m_byArrLightData[3][0];
		ArrGreenValue[0] = (int)bin.m_byArrLightData[4][0];
		ArrWhiteValue[0] = (int)bin.m_byArrLightData[5][0];
	}


	int FOVSize = width * height;

	cv::Mat TmpImg(Image->rows, Image->cols, CV_8UC1);

	double redval(0.0), Bluval(0.0), Greval(0.0), Whival(0.0);
	if (LightCnt <= 0)
	{
		cv::Mat FOVRedImg(height, width, CV_8UC1);
		cv::Mat FOVGreenImg(height, width, CV_8UC1);
		cv::Mat FOVBlueImg(height, width, CV_8UC1);
		cv::Mat FOVWhiteImg(height, width, CV_8UC1);

		cv::Mat RedImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat GreenImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat BlueImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat WhiteImg(Image->rows, Image->cols, CV_8UC1);

		TmpImg.setTo(0);

		redval = (double)ArrRedValue[0] / 100;
		Bluval = (double)ArrBlueValue[0] / 100;
		Greval = (double)ArrGreenValue[0] / 100;
		Whival = (double)ArrWhiteValue[0] / 100;

		switch (ArrLightPosition[0])
		{
		case 0://top
			memcpy(FOVRedImg.data, m_ImageBuffer->imgTop_R, FOVSize * sizeof(UCHAR));
			memcpy(FOVGreenImg.data, m_ImageBuffer->imgTop_G, FOVSize * sizeof(UCHAR));
			memcpy(FOVBlueImg.data, m_ImageBuffer->imgTop_B, FOVSize * sizeof(UCHAR));
			memcpy(FOVWhiteImg.data, m_ImageBuffer->imgTop_W, FOVSize * sizeof(UCHAR));

			RedImg = FOVRedImg(ImageRect).mul(redval);
			GreenImg = FOVGreenImg(ImageRect).mul(Greval);
			BlueImg = FOVBlueImg(ImageRect).mul(Bluval);
			WhiteImg = FOVWhiteImg(ImageRect).mul(Whival);

			TmpImg = RedImg + GreenImg + BlueImg + WhiteImg;
			break;
		case 1: //middle
			memcpy(FOVRedImg.data, m_ImageBuffer->imgMiddle_R, FOVSize * sizeof(UCHAR));
			memcpy(FOVBlueImg.data, m_ImageBuffer->imgMiddle_B, FOVSize * sizeof(UCHAR));

			RedImg = FOVRedImg(ImageRect).mul(redval);
			BlueImg = FOVBlueImg(ImageRect).mul(Bluval);

			TmpImg = RedImg + BlueImg;
			break;
		case 2://bottom
			memcpy(FOVRedImg.data, m_ImageBuffer->imgBottom_R, FOVSize * sizeof(UCHAR));
			memcpy(FOVBlueImg.data, m_ImageBuffer->imgBottom_B, FOVSize * sizeof(UCHAR));
			RedImg = FOVRedImg(ImageRect).mul(redval);
			BlueImg = FOVBlueImg(ImageRect).mul(Bluval);

			TmpImg = RedImg + BlueImg;
			break;
		}
	}
	else
	{
		cv::Mat calcImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat sumImg;

		cv::Mat FOVRedImg(height, width, CV_8UC1);
		cv::Mat FOVGreenImg(height, width, CV_8UC1);
		cv::Mat FOVBlueImg(height, width, CV_8UC1);
		cv::Mat FOVWhiteImg(height, width, CV_8UC1);

		cv::Mat FOVMiddleRedImg(height, width, CV_8UC1);
		cv::Mat FOVMiddleBlueImg(height, width, CV_8UC1);

		cv::Mat FOVBottomRedImg(height, width, CV_8UC1);
		cv::Mat FOVBottomBlueImg(height, width, CV_8UC1);

		cv::Mat RectRedImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat RectGreenImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat RectBlueImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat RectWhiteImg(Image->rows, Image->cols, CV_8UC1);

		cv::Mat RectMiddleRedImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat RectMiddleBlueImg(Image->rows, Image->cols, CV_8UC1);

		cv::Mat RectBottomRedImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat RectBottomBlueImg(Image->rows, Image->cols, CV_8UC1);

		cv::Mat RedImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat GreenImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat BlueImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat WhiteImg(Image->rows, Image->cols, CV_8UC1);

		cv::Mat MiddleRedImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat MiddleBlueImg(Image->rows, Image->cols, CV_8UC1);

		cv::Mat BottomRedImg(Image->rows, Image->cols, CV_8UC1);
		cv::Mat BottomBlueImg(Image->rows, Image->cols, CV_8UC1);

		TmpImg.setTo(0);
		calcImg.setTo(0);

		memcpy(FOVRedImg.data, m_ImageBuffer->imgTop_R, FOVSize * sizeof(UCHAR));
		memcpy(FOVGreenImg.data, m_ImageBuffer->imgTop_G, FOVSize * sizeof(UCHAR));
		memcpy(FOVBlueImg.data, m_ImageBuffer->imgTop_B, FOVSize * sizeof(UCHAR));
		memcpy(FOVWhiteImg.data, m_ImageBuffer->imgTop_W, FOVSize * sizeof(UCHAR));

		memcpy(FOVMiddleRedImg.data, m_ImageBuffer->imgMiddle_R, FOVSize * sizeof(UCHAR));
		memcpy(FOVMiddleBlueImg.data, m_ImageBuffer->imgMiddle_B, FOVSize * sizeof(UCHAR));

		memcpy(FOVBottomRedImg.data, m_ImageBuffer->imgBottom_R, FOVSize * sizeof(UCHAR));
		memcpy(FOVBottomBlueImg.data, m_ImageBuffer->imgBottom_B, FOVSize * sizeof(UCHAR));

		RectRedImg = FOVRedImg(ImageRect).clone();
		RectGreenImg = FOVGreenImg(ImageRect).clone();
		RectBlueImg = FOVBlueImg(ImageRect).clone();
		RectWhiteImg = FOVWhiteImg(ImageRect).clone();
		RectMiddleRedImg = FOVMiddleRedImg(ImageRect).clone();
		RectMiddleBlueImg = FOVMiddleBlueImg(ImageRect).clone();
		RectBottomRedImg = FOVBottomRedImg(ImageRect).clone();
		RectBottomBlueImg = FOVBottomBlueImg(ImageRect).clone();
		cv::Mat tempTop;
		cv::Mat tempTop2;
		CString msg;
		for (int i = 0; i < LightCnt; i++)
		{
			redval = (double)ArrRedValue[i] / 100;
			Bluval = (double)ArrBlueValue[i] / 100;
			Greval = (double)ArrGreenValue[i] / 100;
			Whival = (double)ArrWhiteValue[i] / 100;

			switch (ArrLightPosition[i])
			{
			case 0://top
				RedImg = RectRedImg.mul(redval);
				GreenImg = RectGreenImg.mul(Greval);
				BlueImg = RectBlueImg.mul(Bluval);
				WhiteImg = RectWhiteImg.mul(Whival);

				tempTop = RedImg + GreenImg;
				tempTop2 = BlueImg + WhiteImg;
				calcImg = tempTop + tempTop2;
				break;
			case 1: //middle

				RedImg = RectMiddleRedImg.mul(redval);
				BlueImg = RectMiddleBlueImg.mul(Bluval);

				calcImg = RedImg + BlueImg;
				break;
			case 2://bottom
				RedImg = RectBottomRedImg.mul(redval);
				BlueImg = RectBottomBlueImg.mul(Bluval);

				calcImg = RedImg + BlueImg;
				break;
			}
			if (ArrCalculation[i] == 1)//add
			{
				sumImg = TmpImg + calcImg;
				sumImg.copyTo(TmpImg);

			}
			else if (ArrCalculation[i] == 2)//sub
			{
				sumImg = TmpImg - calcImg;
				sumImg.copyTo(TmpImg);
			}
		}

	}
#if _DEBUG
	cv::imwrite("D:\\FootRst\\GrayImage.bmp", TmpImg);
#endif
	TmpImg.copyTo(*Image);

}


int CPInsp_Foot::Update_fromCV(WndAlgoImg * sWndAlgoImg, UCHAR* src, UCHAR* dsc)
{
	if (!src)
		return eMR_FAIL;

	int width = sWndAlgoImg->m_nWidth;
	int height = sWndAlgoImg->m_nHeight;
	if ((width <= 0) || (height <= 0))
		return eMR_FAIL;
	int widthStep = width;

	int nSizeX = width % 4;
	if (nSizeX != 0)
		widthStep += 4 - nSizeX;

	IppStatus sts;
	IppiSize iSize = { width, height };
	sts = ippiCopy_8u_C1R(src, widthStep, dsc, width, iSize);

	return eMR_SUCCESS;
}


float CPInsp_Foot::GetCorrect3d(WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrAlgoColorOpt, POINT poXY, int nDBC)
{
	float fRet = -99999.9f;
	int nLine = __LINE__;
	try
	{
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nWidth = sWndAlgoImg.m_nWidth;
		int nHeight = sWndAlgoImg.m_nHeight;
		if ((pfImgSrc == NULL) || (nWidth <= 0) || (nHeight <= 0))
			return fRet;
		if (poXY.x < 0) poXY.x = 0;
		if (poXY.y < 0) poXY.y = 0;
		if (poXY.x > nWidth) poXY.x = nWidth;
		if (poXY.y > nHeight) poXY.y = nHeight;
		UCHAR *pucTR = NULL;
		UCHAR *pucTB = NULL;
		UCHAR *pucTW = NULL;
		float* fp3DOrg = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&fp3DOrg, sizeof(float) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucTR, sizeof(float) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucTB, sizeof(float) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucTW, sizeof(float) * nWidth * nHeight);
		memcpy(fp3DOrg, pfImgSrc, sizeof(float)*nWidth * nHeight);
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Foot_3D.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(fp3DOrg, nWidth, nHeight, _T("Foot_3D_R.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt.m_sFovImg.imgTop_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, _T("Foot_Org_TR.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt.m_sFovImg.imgTop_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, _T("Foot_Org_TB.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt.m_sFovImg.imgTop_W, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, _T("Foot_Org_TW.bmp"));
		nLine = __LINE__;
		m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_R, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, pucTR, ptrAlgoColorOpt.m_rcImageRect.left, ptrAlgoColorOpt.m_rcImageRect.top, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_B, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, pucTB, ptrAlgoColorOpt.m_rcImageRect.left, ptrAlgoColorOpt.m_rcImageRect.top, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt.m_sFovImg.imgTop_W, ptrAlgoColorOpt.m_sFovImg.nImageSizeX, ptrAlgoColorOpt.m_sFovImg.nImageSizeY, pucTW, ptrAlgoColorOpt.m_rcImageRect.left, ptrAlgoColorOpt.m_rcImageRect.top, nWidth, nHeight);
		nLine = __LINE__;
		m_pProcMilAlgo->SaveWorkImg(pucTR, nWidth, nHeight, _T("Foot_Clip_TR.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucTB, nWidth, nHeight, _T("Foot_Clip_TB.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pucTW, nWidth, nHeight, _T("Foot_Clip_TW.bmp"));
		nLine = __LINE__;
		Correct3d(nDBC, fp3DOrg, nWidth, nHeight, pucTR, pucTB, pucTW);
		nLine = __LINE__;
		m_pProcMilAlgo->SaveWorkImg_float(fp3DOrg, nWidth, nHeight, _T("Foot_3D_R2.bmp"));
		fRet = fp3DOrg[poXY.y * nWidth + poXY.x];
		nLine = __LINE__;
		Delete_1DArray(&fp3DOrg);
		Delete_1DArray(&pucTR);
		Delete_1DArray(&pucTB);
		Delete_1DArray(&pucTW);
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::GetCorrect3d error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return fRet;
	}
	return fRet;
}

float CPInsp_Foot::GetCorrect3d_Binary(const InspAlgo &sInsp, WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrAlgoColorOpt, POINT poXY, int nDBC)
{
	float fRet = -99999.9f;
	int nLine = __LINE__;
	try
	{
		AlgoFoot* sInspFoot = (AlgoFoot*)sInsp.m_ptrInspAlgoParam;

		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nWidth = sWndAlgoImg.m_nWidth;
		int nHeight = sWndAlgoImg.m_nHeight;
		if ((pfImgSrc == NULL) || (nWidth <= 0) || (nHeight <= 0))
			return fRet;
		if (poXY.x < 0) poXY.x = 0;
		if (poXY.y < 0) poXY.y = 0;
		if (poXY.x > nWidth) poXY.x = nWidth;
		if (poXY.y > nHeight) poXY.y = nHeight;

		float* fp3DOrg = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&fp3DOrg, sizeof(float) * nWidth * nHeight);
		memcpy(fp3DOrg, pfImgSrc, sizeof(float)*nWidth * nHeight);
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Foot_3D.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(fp3DOrg, nWidth, nHeight, _T("Foot_3D_R.bmp"));
		//Pad 합성 이미지 생성
		cv::Rect ImageRect;
		ImageRect.x = ptrAlgoColorOpt.m_rcImageRect.left;
		ImageRect.y = ptrAlgoColorOpt.m_rcImageRect.top;
		ImageRect.width = nWidth;
		ImageRect.height = nHeight;

		cv::Mat Image(nHeight, nWidth, CV_8UC1);;
		GetImageMatrix(&Image, sInspFoot->m_sArrBin[0], &ptrAlgoColorOpt.m_sFovImg, ImageRect);

		//이진화
		GetBinImage(sInsp, sWndAlgoImg, &Image, ImageRect, 0, 0, nWidth, NULL, false);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Pad.bmp", Image);
#endif

		nLine = __LINE__;
		float fPad3DAvgHeigt = 0.0f;

		Correct3d_Binary(Image, nDBC, fp3DOrg, nWidth, nHeight, fPad3DAvgHeigt);
		//Correct3d(nDBC, fp3DOrg, nWidth, nHeight, pucTR, pucTB, pucTW);
		nLine = __LINE__;
		m_pProcMilAlgo->SaveWorkImg_float(fp3DOrg, nWidth, nHeight, _T("Foot_3D_R2.bmp"));
		fRet = fp3DOrg[poXY.y * nWidth + poXY.x];
		nLine = __LINE__;
		Delete_1DArray(&fp3DOrg);
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::GetCorrect3d_Binary error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return fRet;
	}
	return fRet;


}

cv::Mat CPInsp_Foot::Correct3DRemoveLow(cv::Mat Image3D)
{
	cv::Mat ret = cv::Mat(Image3D.rows, Image3D.cols, CV_32FC1);
	ret.setTo(0);
	int nLine = __LINE__;
	try
	{
		std::vector<int> HeightCnt;
		HeightCnt.resize(3000);
		for (int r = 0; r < Image3D.rows; r++)
		{
			float* fPtr = Image3D.ptr<float>(r);
			for (int c = 0; c < Image3D.cols; c++)
			{
				int HValue = (int)fPtr[c] + 2000;
				if (HValue < 3000 && HValue>0)
					HeightCnt[HValue]++;
			}
		}

		int nMaxCnt(0), nMaxIdx(0);
		for (int i = 0; i < HeightCnt.size(); i++)
		{
			if (HeightCnt[i] > nMaxCnt)
			{
				nMaxCnt = HeightCnt[i];
				nMaxIdx = i;
			}
		}
		if (nMaxIdx < 2000)
		{
			Image3D.copyTo(ret);
			return ret;
		}
		nMaxIdx -= 2000;

		float MinHeight = nMaxIdx - 20.f;

		for (int r = 0; r < Image3D.rows; r++)
		{
			float* fPtr = Image3D.ptr<float>(r);
			float* fDstPtr = ret.ptr<float>(r);
			for (int c = 0; c < Image3D.cols; c++)
			{
				float HValue = fPtr[c];
				fDstPtr[c] = HValue > MinHeight ? HValue : MinHeight;
			}
		}
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::Correct3DRemoveLow() Line %d Pass"), nLine);
		g_pMPTI->AddLog(msg);
	}
	return ret;
}
bool CPInsp_Foot::CreateModel(cv::Mat Image2D, CString FilePath, int FootType, RECT MRect, int Direction, int DBC)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		m_PatModel = std::make_shared<GeoModel_Foot>();
		m_PatModel->Alloc(Image2D);
		m_PatModel->setAngleRange(true, 90, 90, 5);

		nLine = __LINE__;
		m_PatModel->setAngle(0);
		m_PatModel->setRes(m_resolX, m_resolY);
		m_PatModel->FootType = FootType;
		cv::Rect WedgeRect(MRect.left, MRect.top, MRect.right - MRect.left, MRect.bottom - MRect.top);
		m_PatModel->WedgeArea = WedgeRect;
		m_PatModel->FootDirection = Direction;
		m_PatModel->FootDBC = DBC;
		m_PatModel->Preprocess();
		nLine = __LINE__;
		m_PatModel->SaveFile(FilePath, g_pInspMng->getModelSzOptMz());
		nLine = __LINE__;

		std::shared_ptr<GeoResult_Foot> m_PatResult = std::make_shared<GeoResult_Foot>();
		m_PatResult->Alloc(1);
		Img_Proc = std::make_shared<ImgProcessing>();
		Img_Proc->_ImgProcess(Image2D, *m_PatModel);
		nLine = __LINE__;

		gm.SetImgProcess(Img_Proc);
		gm.FindModel(*m_PatModel, *m_PatResult, false);
		nLine = __LINE__;

#if _DEBUG
		double sc = m_PatResult->_Equality[0];
#endif
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::CreateModel error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::CreateModel3D(cv::Mat Image3D, CString FilePath, int FootType, RECT MRect, int Direction, int DBC)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		cv::Mat Nomal, Image8U;
		//cv::normalize(Image3D, Nomal, 0, 255, cv::NORM_MINMAX);
		//Nomal.convertTo(Image8U, CV_8UC1);

		m_PatModel = std::make_shared<GeoModel_Foot>();
		m_PatModel->Alloc(Image3D);
		m_PatModel->setAngleRange(true, 90, 90, 5);

		nLine = __LINE__;
		m_PatModel->setAngle(0);
		m_PatModel->setRes(m_resolX, m_resolY);
		m_PatModel->FootType = FootType;
		cv::Rect WedgeRect(MRect.left, MRect.top, MRect.right - MRect.left, MRect.bottom - MRect.top);
		m_PatModel->WedgeArea = WedgeRect;
		m_PatModel->FootDirection = Direction;
		m_PatModel->FootDBC = DBC;
		m_PatModel->Preprocess();
		nLine = __LINE__;
		m_PatModel->SaveFile(FilePath, g_pInspMng->getModelSzOptMz());
		nLine = __LINE__;

		std::shared_ptr<GeoResult_Foot> m_PatResult = std::make_shared<GeoResult_Foot>();
		m_PatResult->Alloc(1);
		Img_Proc = std::make_shared<ImgProcessing>();
		Img_Proc->_ImgProcess(Image3D, *m_PatModel);
		nLine = __LINE__;

		gm.SetImgProcess(Img_Proc);
		gm.FindModel(*m_PatModel, *m_PatResult, false);
		nLine = __LINE__;

#if _DEBUG
		double sc = m_PatResult->_Equality[0];

		if (CV_MAT_TYPE(Image3D.type()) != CV_8UC1)
		{
			cv::normalize(Image3D, Image3D, 0, 255, cv::NORM_MINMAX);
			Image3D.convertTo(Image3D, CV_8UC1);
		}
		cv::imwrite("D:\\FootImage3D.bmp", Image3D);
#endif
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::CreateModel3D error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::MatchModel(cv::Mat Image2D, CString FilePath, int FootType)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		m_PatModel = std::make_shared<GeoModel_Foot>();
		m_PatModel->LoadFile(FilePath, m_resolX, m_resolY);
		nLine = __LINE__;

		std::shared_ptr<GeoResult_Foot> m_PatResult = std::make_shared<GeoResult_Foot>();
		m_PatResult->Alloc(1);
		Img_Proc = std::make_shared<ImgProcessing>();
		Img_Proc->_ImgProcess(Image2D, *m_PatModel);
		nLine = __LINE__;
		m_PatModel->setFindSubPixel(false);
		if (m_PatModel->FootType != FootType)
			return false;

		gm.SetImgProcess(Img_Proc);
		gm.FindModel(*m_PatModel, *m_PatResult, false);
		nLine = __LINE__;

		double sc = m_PatResult->_Equality[0];
		int nPoX = m_PatResult->_Center_y[0];
		int nPoY = m_PatResult->_Center_x[0];

#if _DEBUG
// 		cv::Mat draw;
// 		Image2D.copyTo(draw);
// 		nLine = __LINE__;
// 		gm.DrawContours(draw, *m_PatModel, *m_PatResult, cv::Scalar(255), 1);
// 		nLine = __LINE__;
// 		CString drawImgPath;
// 		drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image2D__%d_match.bmp"), i + 1);
// 		cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
		CString sScore;
		sScore.Format(_T("Position: (%d, %d)\nMatching Score: %.2f"), nPoX, nPoY, sc);
		AfxMessageBox(sScore);
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::MatchModel error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::MatchModel3D(cv::Mat Image3D, CString FilePath, int FootType)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		cv::Mat Nomal, Image8U;
		cv::normalize(Image3D, Nomal, 0, 255, cv::NORM_MINMAX);
		Nomal.convertTo(Image8U, CV_8UC1);
		int nLine = __LINE__;

#if _DEBUG
		cv::imwrite("D:\\testimage\\workimage\\Nomal.bmp", Image8U);
#endif
		m_PatModel = std::make_shared<GeoModel_Foot>();
		m_PatModel->LoadFile(FilePath, m_resolX, m_resolY);
		nLine = __LINE__;

		std::shared_ptr<GeoResult_Foot> m_PatResult = std::make_shared<GeoResult_Foot>();
		m_PatResult->Alloc(1);
		Img_Proc = std::make_shared<ImgProcessing>();
		Img_Proc->_ImgProcess(Image3D, *m_PatModel);
		m_PatModel->setFindSubPixel(false);
		nLine = __LINE__;
		if (m_PatModel->FootType != FootType)
			return false;

		gm.SetImgProcess(Img_Proc);
		gm.FindModel(*m_PatModel, *m_PatResult, false);
		nLine = __LINE__;

		double sc = m_PatResult->_Equality[0];
		int nPoX = m_PatResult->_Center_y[0];
		int nPoY = m_PatResult->_Center_x[0];

#if _DEBUG
// 		cv::Mat draw;
// 		draw = Image8U.clone();
// 		gm.DrawContours(draw, *m_PatModel, *m_PatResult, cv::Scalar(255), 1);
// 		CString drawImgPath;
// 		drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D__%d_match.bmp"), i + 1);
// 		cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
		CString sScore;
		sScore.Format(_T("Position 3D: (%d, %d)\nMatching Score: %.2f"), nPoX, nPoY, sc);
		AfxMessageBox(sScore);
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::MatchModel3D error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}


bool CPInsp_Foot::LoadFootModelList(CString FolderPath)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		m_ModelMng2D->LoadFileList(FolderPath, m_resolX, m_resolY, _T("2D"));
		m_ModelMng3D->LoadFileList(FolderPath, m_resolX, m_resolY, _T("3D"));
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::LoadFootModelList error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}
}
bool CPInsp_Foot::CheckModelList(CString FolderPath)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		ret = m_ModelMng2D->CheckFileList(FolderPath, m_resolX, m_resolY, _T("2D"));
		ret = m_ModelMng3D->CheckFileList(FolderPath, m_resolX, m_resolY, _T("3D"));
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::CheckModelList error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::GetFootModelParam(CString sFileName2D, int* FootType, RECT* MRect, double* resX, double* resY)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		m_PatModel = std::make_shared<GeoModel_Foot>();
		nLine = __LINE__;
		std::shared_ptr<ModelFile_Foot> sMoFile = std::make_shared<ModelFile_Foot>(eAlgoFoot);
		sMoFile->LoadFile(sFileName2D, 0, 0);

		m_PatModel = sMoFile->milModel;
		m_PatModel->Preprocess();
		m_PatModel->setFindSubPixel(false);
		*FootType = m_PatModel->FootType;
		MRect->left = m_PatModel->WedgeArea.x;
		MRect->top = m_PatModel->WedgeArea.y;
		MRect->right = m_PatModel->WedgeArea.x + m_PatModel->WedgeArea.width;
		MRect->bottom = m_PatModel->WedgeArea.y + m_PatModel->WedgeArea.height;
		m_PatModel->getRes(resX, resY);

#if _DEBUG
// 		nLine = __LINE__;
// 		cv::Mat draw;
// 		m_PatModel->Image().copyTo(draw);
// 		nLine = __LINE__;
// 		CString drawImgPath;
// 		drawImgPath.Format(_T("D:\\testimage\\workimage\\Image2D_%d_GetModel.bmp"), i + 1);
// 		cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::GetFootModelParam error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}

bool CPInsp_Foot::MatchModel(cv::Mat Image2D, int FootType)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		CString FilePath;
		m_PatModel = std::make_shared<GeoModel_Foot>();
		nLine = __LINE__;
		for (int i = 0, nNum = 1; i < m_ModelMng2D->GetModelCount(false);nNum++)
		{
			FilePath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image2D_%d.ppd"), nNum);
			std::shared_ptr<ModelFile_Foot> sMoFile = m_ModelMng2D->GetModel(FilePath);
			if (!sMoFile)
				continue;
			i++;
			m_PatModel = sMoFile->milModel;
			m_PatModel->Preprocess();
			m_PatModel->setFindSubPixel(false);
			if (m_PatModel->FootType != FootType)
				continue;

			std::shared_ptr<GeoResult_Foot> m_PatResult = std::make_shared<GeoResult_Foot>();
			m_PatResult->Alloc(1);
			Img_Proc = std::make_shared<ImgProcessing>();
			Img_Proc->_ImgProcess(Image2D, *m_PatModel);
			nLine = __LINE__;

			gm.SetImgProcess(Img_Proc);
			gm.FindModel(*m_PatModel, *m_PatResult, false);
			nLine = __LINE__;

			double sc = m_PatResult->_Equality[0];
			int nPoX = m_PatResult->_Center_y[0];
			int nPoY = m_PatResult->_Center_x[0];

#if _DEBUG
			cv::Mat draw;
			Image2D.copyTo(draw);
			nLine = __LINE__;
			gm.DrawContours(draw, *m_PatModel, *m_PatResult, cv::Scalar(255), 1);
			nLine = __LINE__;
			CString drawImgPath;
			drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image2D__%d_match.bmp"), i + 1);
			cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
			CString sScore;
			sScore.Format(_T("Position: (%d, %d)\nMatching Score: %.2f"), nPoX, nPoY, sc);
			AfxMessageBox(sScore);
		}
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("CPInsp_Foot::MatchModel error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::MatchModel3D(cv::Mat Image3D, int FootType)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		cv::Mat Nomal, Image8U;
		cv::normalize(Image3D, Nomal, 0, 255, cv::NORM_MINMAX);
		Nomal.convertTo(Image8U, CV_8UC1);
#if _DEBUG
		cv::imwrite("D:\\testimage\\workimage\\Nomal.bmp", Image8U);
#endif

		CString FilePath;
		m_PatModel = std::make_shared<GeoModel_Foot>();
		nLine = __LINE__;
		for (int i = 0, nNum = 1; i < m_ModelMng3D->GetModelCount(false);nNum++)
		{
			FilePath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_%d.ppd"), nNum);
			std::shared_ptr<ModelFile_Foot> sMoFile = m_ModelMng3D->GetModel(FilePath);
			if (!sMoFile)
				continue;
			i++;
			m_PatModel = sMoFile->milModel;
			m_PatModel->Preprocess();
			m_PatModel->setFindSubPixel(false);
			if (m_PatModel->FootType != FootType)
				continue;

			std::shared_ptr<GeoResult_Foot> m_PatResult = std::make_shared<GeoResult_Foot>();
			m_PatResult->Alloc(1);
			Img_Proc = std::make_shared<ImgProcessing>();
			Img_Proc->_ImgProcess(Image8U, *m_PatModel);
			nLine = __LINE__;

			gm.SetImgProcess(Img_Proc);
			gm.FindModel(*m_PatModel, *m_PatResult, false);
			nLine = __LINE__;

			double sc = m_PatResult->_Equality[0];
			int nPoX = m_PatResult->_Center_y[0];
			int nPoY = m_PatResult->_Center_x[0];

#if _DEBUG
			cv::Mat draw;
			draw = Image8U.clone();
			gm.DrawContours(draw, *m_PatModel, *m_PatResult, cv::Scalar(255), 1);
			CString drawImgPath;
			drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D__%d_match.bmp"), i + 1);
			cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
			CString sScore;
			sScore.Format(_T("Position 3D: (%d, %d)\nMatching Score: %.2f"), nPoX, nPoY, sc);
			AfxMessageBox(sScore);
		}
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::MatchModel3D error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::MatchModel(CFoot_Model *pFoot, cv::Mat Image2D, cv::Mat ImagePad, std::shared_ptr<GeoResult_Foot> PatResult, cv::Rect* RectModelImg, cv::Mat ImgFoot2DBW, cv::Mat ImgFoot3D, int* MatchModelNum, double dAngle)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		//3D전처리
		cv::Mat Image3DBin, Image3DBinD;
		cv::threshold(ImgFoot3D, Image3DBin, 0, 255, cv::THRESH_BINARY);
		cv::threshold(ImgFoot3D, Image3DBinD, pFoot->m_fArrOptionValue[(int)m_eFoot::m_eFoot_Height][m_eMMD::eMMD_Max], 255, cv::THRESH_BINARY_INV);
		Image3DBin &= Image3DBinD;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(Image3DBin, Image3DBin, kernel);
		Image3DBin.convertTo(Image3DBin, CV_8UC1);
		std::vector<std::vector<cv::Point>> vContMax;
		cv::findContours(Image3DBin, vContMax, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		double dMaxArea = 0;
		cv::Rect rMaxRe;
		for (int i = 0; i < vContMax.size(); i++)
		{
			cv::Rect rRe = cv::boundingRect(vContMax[i]);
			double dArea = cv::contourArea(vContMax[i]);
			if (dMaxArea < dArea)
			{
				rMaxRe = rRe;
				dMaxArea = dArea;
			}
		}
		cv::Mat matchImage2D;
		cv::Mat Foot2DBWTemp;
		cv::Mat ImgFoot3DTemp;
		cv::Mat ImgImagePadTemp;
		if (vContMax.size() > 0 && !pFoot->m_bUse2Foot)
		{
			matchImage2D = Image2D(rMaxRe).clone();
			Foot2DBWTemp = ImgFoot2DBW(rMaxRe).clone();
			ImgFoot3DTemp = ImgFoot3D(rMaxRe);
			ImgImagePadTemp = ImagePad(rMaxRe);
		}
		else
		{
			matchImage2D = Image2D.clone();
			Foot2DBWTemp = ImgFoot2DBW.clone();
			ImgFoot3DTemp = ImgFoot3D;
			ImgImagePadTemp = ImagePad;
			rMaxRe.x = 0;
			rMaxRe.y = 0;
			rMaxRe.width = 0;
			rMaxRe.height = 0;
		}
		//
		//cv::Mat matchImage2D = Image2D.clone();
		CString FilePath;
		m_PatModel = std::make_shared<GeoModel_Foot>();
		GeoModel_Foot PatModel;
		nLine = __LINE__;
		int nModelCnt = m_ModelMng2D->GetModelCount(false);
		std::vector< std::shared_ptr<GeoResult_Foot>> vPatRe;
		std::shared_ptr<GeoResult_Foot> PatRe;
		std::vector<cv::Rect> RectModelList;
		std::vector<cv::Rect> RectWedgeList;
		std::vector<int> MatchNumList;
		nLine = __LINE__;
		double nMaxSc[2] = { 0,0 };
		int nFootDirection = pFoot->m_nFootAngle;
		bool b2Foot = pFoot->m_bUse2Foot;
		int footType = pFoot->m_nFoottype;
		int nDBCFoot = 0;
		if(pFoot->m_nUseOption2 & m_eFOOT_Data2_DBC)	
			nDBCFoot = 1;
		cv::Rect TeachPadRect(pFoot->m_PadRect.left - pFoot->mImageRect.left, pFoot->m_PadRect.top - pFoot->mImageRect.top
			, pFoot->m_PadRect.right - pFoot->m_PadRect.left, pFoot->m_PadRect.bottom - pFoot->m_PadRect.top);
		if (dAngle != -INFINITY)
		{
			if (135 >= nFootDirection && nFootDirection > 45)
				dAngle -= 90;
			else if (225 >= nFootDirection && nFootDirection > 135)
				dAngle -= 180;
			else if (315 >= nFootDirection && nFootDirection > 225)
				dAngle -= 270;
		}
		int resultCnt = 1;
		if (b2Foot) 
			resultCnt = PatResult->Count();

		for (int re = 0; re < resultCnt; re++)
		{
			vPatRe.clear();
			RectModelList.clear();
			RectWedgeList.clear();
			for (int i = 0, nNum = 1; i < m_ModelMng2D->GetModelCount(false); nNum++)
			{
				FilePath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image2D_%d.ppd"), nNum );
				std::shared_ptr<ModelFile_Foot> sMoFile = m_ModelMng2D->GetModel(FilePath);
				if (!sMoFile)
					continue;
				i++;
				if (45 >= nFootDirection || nFootDirection > 315)
					m_PatModel = sMoFile->milModel;
				else if (135 >= nFootDirection && nFootDirection > 45)
					m_PatModel = sMoFile->GetAngleModel(90);
				else if (225 >= nFootDirection && nFootDirection > 135)
					m_PatModel = sMoFile->GetAngleModel(180);
				else if (315 >= nFootDirection && nFootDirection > 225)
					m_PatModel = sMoFile->GetAngleModel(270);

				PatModel = *m_PatModel;
				if (PatModel.FootType != footType || PatModel.FootDirection != nFootDirection || PatModel.FootDBC != nDBCFoot)
					continue;
				if (dAngle != -INFINITY)
				{
					PatModel.setAngleRange(true, -(dAngle - 5), dAngle + 5, 0.5);

					nLine = __LINE__;
					PatModel.setAngle(0);
				}
				PatModel.Preprocess();
				PatModel.setFindSubPixel(false);

				PatRe = std::make_shared<GeoResult_Foot>();
				PatRe->Alloc(1);
				Img_Proc = std::make_shared<ImgProcessing>();
				Img_Proc->_ImgProcess(matchImage2D, PatModel);
				nLine = __LINE__;

				gm.SetImgProcess(Img_Proc);
				gm.FindModel(PatModel, *PatRe, false);
				nLine = __LINE__;
				bool bInPadRect = false;
				uchar* ptr = ImgImagePadTemp.ptr(PatRe->_Center_x[0]);
				if (ptr[(int)PatRe->_Center_y[0]]>0)
					bInPadRect = true;
				if (PatRe->_Equality[0] >= m_nFootPatternMatchScore && bInPadRect)
				{
					vPatRe.push_back(PatRe);
					int RectC = sMoFile->milModel->Image().cols;
					int RectR = sMoFile->milModel->Image().rows;
					cv::Rect WRect = PatModel.WedgeArea;

// 					if ((135 >= nFootDirection && nFootDirection > 45 )|| (315 >= nFootDirection && nFootDirection > 225))
// 					{
// 						RectC = sMoFile->milModel->Image().rows;
// 						RectR = sMoFile->milModel->Image().cols;
// 
// 						int nTemp = WRect.width;
// 						WRect.width = WRect.height;
// 						WRect.height = nTemp;
// 					}
					RectModelList.push_back(cv::Rect(vPatRe[vPatRe.size() - 1]->_Center_y[0] - RectC / 2 -10, vPatRe[vPatRe.size() - 1]->_Center_x[0] - RectR / 2, RectC + 10, RectR));
					RectWedgeList.push_back(WRect);
					MatchNumList.push_back(i + 1);
				}

#if _DEBUG
				cv::Mat draw;
				draw = matchImage2D.clone();
				gm.DrawContours(draw, PatModel, *PatRe, cv::Scalar(255), 1);
				CString drawImgPath;
				drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image2D__%d_match.bmp"), i + 1);
				cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
				if (PatRe->_Equality[0] >= m_nFootPatternMatchStopScore && !b2Foot && bInPadRect)
					break;
			}

			//여기서 한번 더 검증(패턴매칭으로 검출된 Center Point 가 Foot BW 영상내부에 존재하는 지 확인)
			bool bInsideFootBW_PatMatCtrPo = false;

			for (int i = 0; i < vPatRe.size(); i++)
			{
				bInsideFootBW_PatMatCtrPo = false;

				if (Foot2DBWTemp.rows != 0 && Foot2DBWTemp.cols != 0 && ImgFoot3DTemp.rows != 0 && ImgFoot3DTemp.cols != 0)
				{
// 					for (int r = 0; r < ImgFoot2DBW.rows; r++)
// 					{
// 						UCHAR* ImgPtr = ImgFoot2DBW.ptr(r);
// 						float* Img3DPtr = ImgFoot3D.ptr<float>(r);
// 
// 						for (int c = 0; c < ImgFoot2DBW.cols; c++)
// 						{
// 							if ((ImgPtr[c] == 255|| ImgPtr[c] == 255|| ImgPtr[c] == 255|| ImgPtr[c] == 255|| ImgPtr[c] == 255) && Img3DPtr[c] > 10)
// 							{
// 								if (vPatRe[i]->_Center_y[0] == c && vPatRe[i]->_Center_x[0] == r)
// 									bInsideFootBW_PatMatCtrPo = true;
// 							}
// 						}
// 
// 					}
					int c = vPatRe[i]->_Center_y[0];
					int r = vPatRe[i]->_Center_x[0];
					if (c < 0) c = 0;
					else if (c >= Foot2DBWTemp.cols) c = Foot2DBWTemp.cols - 1;
					if (r < 0) r = 0;
					else if (r >= Foot2DBWTemp.rows) r = Foot2DBWTemp.rows - 1;

					UCHAR* ImgPtr = Foot2DBWTemp.ptr(r);
					float* Img3DPtr = ImgFoot3DTemp.ptr<float>(r);
					if (ImgPtr[c] == 255 && Img3DPtr[c] > 10)
					{
						bInsideFootBW_PatMatCtrPo = true;
					}
					else
					{
						double vMatchAngle = vPatRe[i]->_Angle[0];
						if (135 >= nFootDirection && nFootDirection > 45)//90
						{
							if (dAngle == -INFINITY)
								vMatchAngle -= 90;
							double a1 = std::tan((180 + vMatchAngle) * std::_Pi / 180); //-
							double a2 = std::tan((90 + vMatchAngle) * std::_Pi / 180);//|
							double b1 = vMatchAngle == 0 ? r : r - a1 * c;
							double b2 = vMatchAngle == 0 ? r : r - a2 * c;
							int nWRange = RectModelList[i].width / 2;
							int nHRange = RectModelList[i].height / 2;
							int nWmin = c - nWRange < 0 ? 0 : c - nWRange;
							int nHmin = r - nHRange < 0 ? 0 : r - nHRange;
							int nHmax = r + nHRange >= Foot2DBWTemp.rows ? Foot2DBWTemp.rows - 1 : r + nHRange;
							bool bL(false), bB(false), bT(false);
							for (int tmpC = nWmin; tmpC < c; tmpC++)
							{
								int nRow = a1 * tmpC + b1;
								if (nRow < 0 || nRow >= Foot2DBWTemp.rows)
									continue;
								ImgPtr = Foot2DBWTemp.ptr(nRow);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(nRow);

								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bL = true;
									break;
								}
							}
							for (int tmpR = nHmin; tmpR < r; tmpR++)
							{
								ImgPtr = Foot2DBWTemp.ptr(tmpR);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(tmpR);
								int tmpC = (tmpR - b2) / a2;
								if (tmpC < 0 || tmpC >= Foot2DBWTemp.cols)
									continue;
								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bT = true;
									break;
								}
							}
							for (int tmpR = nHmax; tmpR > r; tmpR--)
							{
								ImgPtr = Foot2DBWTemp.ptr(tmpR);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(tmpR);
								int tmpC = (tmpR - b2) / a2;
								if (tmpC < 0 || tmpC >= Foot2DBWTemp.cols)
									continue;
								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bB = true;
									break;
								}
							}
							if (bL&bB&bT)
							{
								bInsideFootBW_PatMatCtrPo = true;
							}
						}
						else if (225 >= nFootDirection && nFootDirection > 135) //180
						{
							if (dAngle == -INFINITY)
								vMatchAngle -= 180;
							double a1 = std::tan((180 + vMatchAngle) * std::_Pi / 180);//-
							double a2 = std::tan((270 + vMatchAngle) * std::_Pi / 180); //|
							double b1 = vMatchAngle == 0 ? r : r - a1 * c;
							double b2 = vMatchAngle == 0 ? r : r - a2 * c;
							int nWRange = RectModelList[i].width / 2;
							int nHRange = RectModelList[i].height / 2;
							int nWmin = c - nWRange < 0 ? 0 : c - nWRange;
							int nWmax = c + nWRange >= Foot2DBWTemp.cols ? Foot2DBWTemp.cols - 1 : c + nWRange;
							int nHmax = r + nHRange >= Foot2DBWTemp.rows ? Foot2DBWTemp.rows - 1 : r + nHRange;
							bool bL(false), bR(false), bB(false);
							for (int tmpC = nWmin; tmpC < c; tmpC++)
							{
								int nRow = a1 * tmpC + b1;
								if (nRow < 0 || nRow >= Foot2DBWTemp.rows)
									continue;
								ImgPtr = Foot2DBWTemp.ptr(nRow);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(nRow);

								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bL = true;
									break;
								}
							}
							for (int tmpC = nWmax; tmpC > c; tmpC--)
							{
								int nRow = a1 * tmpC + b1;
								if (nRow < 0 || nRow >= Foot2DBWTemp.rows)
									continue;
								ImgPtr = Foot2DBWTemp.ptr(nRow);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(nRow);

								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bR = true;
									break;
								}
							}
							for (int tmpR = nHmax; tmpR > r; tmpR--)
							{
								ImgPtr = Foot2DBWTemp.ptr(tmpR);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(tmpR);
								int tmpC = (tmpR - b2) / a2;
								if (tmpC < 0 || tmpC >= Foot2DBWTemp.cols)
									continue;
								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bB = true;
									break;
								}
							}
							if (bL&bR&bB)
							{
								bInsideFootBW_PatMatCtrPo = true;
							}
						}
						else if (315 >= nFootDirection && nFootDirection > 225)//270
						{
							if (dAngle == -INFINITY)
								vMatchAngle -= 270;
							double a1 = std::tan((vMatchAngle) * std::_Pi / 180); //-
							double a2 = std::tan((270 + vMatchAngle) * std::_Pi / 180);//|
							double b1 = vMatchAngle == 0 ? r : r - a1 * c;
							double b2 = vMatchAngle == 0 ? r : r - a2 * c;
							int nWRange = RectModelList[i].width / 2;
							int nHRange = RectModelList[i].height / 2;
							int nWmax = c + nWRange >= Foot2DBWTemp.cols ? Foot2DBWTemp.cols - 1 : c + nWRange;
							int nHmin = r - nHRange < 0 ? 0 : r - nHRange;
							int nHmax = r + nHRange >= Foot2DBWTemp.rows ? Foot2DBWTemp.rows - 1 : r + nHRange;
							bool bB(false), bR(false), bT(false);
							for (int tmpC = nWmax; tmpC > c; tmpC--)
							{
								int nRow = a1 * tmpC + b1;
								if (nRow < 0 || nRow >= Foot2DBWTemp.rows)
									continue;
								ImgPtr = Foot2DBWTemp.ptr(nRow);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(nRow);

								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bR = true;
									break;
								}
							}
							for (int tmpR = nHmin; tmpR < r; tmpR++)
							{
								ImgPtr = Foot2DBWTemp.ptr(tmpR);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(tmpR);
								int tmpC = (tmpR - b2) / a2;
								if (tmpC < 0 || tmpC >= Foot2DBWTemp.cols)
									continue;
								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bT = true;
									break;
								}
							}
							for (int tmpR = nHmax; tmpR > r; tmpR--)
							{
								ImgPtr = Foot2DBWTemp.ptr(tmpR);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(tmpR);
								int tmpC = (tmpR - b2) / a2;
								if (tmpC < 0 || tmpC >= Foot2DBWTemp.cols)
									continue;
								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bB = true;
									break;
								}
							}
							if (bB&bR&bT)
							{
								bInsideFootBW_PatMatCtrPo = true;
							}
						}
						else //0
						{
							double a1 = std::tan((vMatchAngle) * std::_Pi / 180);//-
							double a2 = std::tan((90 + vMatchAngle) * std::_Pi / 180); //|
							double b1 = vMatchAngle == 0 ? r : r - a1 * c;
							double b2 = vMatchAngle == 0 ? r : r - a2 * c;
							int nWRange = RectModelList[i].width / 2;
							int nHRange = RectModelList[i].height / 2;
							int nWmin = c - nWRange < 0 ? 0 : c - nWRange;
							int nWmax = c + nWRange >= Foot2DBWTemp.cols ? Foot2DBWTemp.cols - 1 : c + nWRange;
							int nHmin = r - nHRange < 0 ? 0 : r - nHRange;
							bool bL(false), bR(false), bT(false);
							for (int tmpC = nWmin; tmpC < c; tmpC++)
							{
								int nRow = a1 * tmpC + b1;
								if (nRow < 0 || nRow >= Foot2DBWTemp.rows)
									continue;
								ImgPtr = Foot2DBWTemp.ptr(nRow);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(nRow);

								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bL = true;
									break;
								}
							}
							for (int tmpC = nWmax; tmpC > c; tmpC--)
							{
								int nRow = a1 * tmpC + b1;
								if (nRow < 0 || nRow >= Foot2DBWTemp.rows)
									continue;
								ImgPtr = Foot2DBWTemp.ptr(nRow);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(nRow);

								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bR = true;
									break;
								}
							}
							for (int tmpR = nHmin; tmpR < r; tmpR++)
							{
								ImgPtr = Foot2DBWTemp.ptr(tmpR);
								Img3DPtr = ImgFoot3DTemp.ptr<float>(tmpR);
								int tmpC = (tmpR - b2) / a2;
								if (tmpC < 0 || tmpC >= Foot2DBWTemp.cols)
									continue;
								if (ImgPtr[tmpC] == 255 && Img3DPtr[tmpC] > 10)
								{
									bT = true;
									break;
								}
							}
							if (bL&bR&bT)
							{
								bInsideFootBW_PatMatCtrPo = true;
							}
						}
					}
				}

				if (vPatRe[i]->_Equality[0] > nMaxSc[re] && bInsideFootBW_PatMatCtrPo)
				{
					PatResult->_Equality[re] = vPatRe[i]->_Equality[0];
					PatResult->_Center_x[re] = vPatRe[i]->_Center_x[0];
					PatResult->_Center_y[re] = vPatRe[i]->_Center_y[0];

					if (dAngle != -INFINITY)
					{
						if (135 >= nFootDirection && nFootDirection > 45)
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0] - 270;
						else if (225 >= nFootDirection && nFootDirection > 135)
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0] - 180;
						else if (315 >= nFootDirection && nFootDirection > 225)
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0] - 90;
						else
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0];
					}
					else
						PatResult->_Angle[re] = vPatRe[i]->_Angle[0];
					nMaxSc[re] = vPatRe[i]->_Equality[0];
					RectModelImg[re].x = RectModelList[i].x;
					RectModelImg[re].y = RectModelList[i].y;
					RectModelImg[re].width = RectModelList[i].width;
					RectModelImg[re].height = RectModelList[i].height;

					float fx = RectModelList[i].x;
					float fy = RectModelList[i].y;
					int wedgeW = RectWedgeList[i].width < 1 ? RectModelList[i].width : RectWedgeList[i].width;
					int wedgeH = RectWedgeList[i].height < 1 ? RectModelList[i].height : RectWedgeList[i].height;
					RectModelImg[re + 2].x = RectWedgeList[i].x + (int)std::roundf(fx);
					RectModelImg[re + 2].y = RectWedgeList[i].y + (int)std::roundf(fy);
					RectModelImg[re + 2].width = wedgeW;
					RectModelImg[re + 2].height = wedgeH;
					MatchModelNum[re] = MatchNumList[i];
				}
			}
			if (re == 0)
			{
				cv::Point pts[1][4];
				int cx = PatResult->_Center_y[0];
				int cy = PatResult->_Center_x[0];

				double w = RectModelImg[re].width / 2;
				double h = RectModelImg[re].height / 2;

				double dLeng = std::sqrt(std::pow(w, 2) + std::pow(h, 2));
				double dRate = h / w;
				double dstdRadian = std::atan(dRate);
				double dstdDegree = dstdRadian / std::_Pi * 180;

				double dRadian1 = (-PatResult->_Angle[0] - (dstdDegree)) * std::_Pi / 180;
				double dRadian2 = (-PatResult->_Angle[0] - (180 - dstdDegree)) * std::_Pi / 180;
				double dRadian3 = (-PatResult->_Angle[0] - (180 + dstdDegree)) * std::_Pi / 180;
				double dRadian4 = (-PatResult->_Angle[0] - (360 - dstdDegree)) * std::_Pi / 180;
				double diffX1 = std::cos(dRadian1) * dLeng;
				double diffY1 = std::sin(dRadian1) * dLeng;
				double diffX2 = std::cos(dRadian2) * dLeng;
				double diffY2 = std::sin(dRadian2) * dLeng;
				double diffX3 = std::cos(dRadian3) * dLeng;
				double diffY3 = std::sin(dRadian3) * dLeng;
				double diffX4 = std::cos(dRadian4) * dLeng;
				double diffY4 = std::sin(dRadian4) * dLeng;

				int p1x = cx + (int)std::round(diffX1); int  p1y = cy + (int)std::round(diffY1);
				int p2x = cx + (int)std::round(diffX2); int  p2y = cy + (int)std::round(diffY2);
				int p3x = cx + (int)std::round(diffX3); int  p3y = cy + (int)std::round(diffY3);
				int p4x = cx + (int)std::round(diffX4); int  p4y = cy + (int)std::round(diffY4);
				pts[0][0] = cv::Point(p1x, p1y);
				pts[0][1] = cv::Point(p2x, p2y);
				pts[0][2] = cv::Point(p3x, p3y);
				pts[0][3] = cv::Point(p4x, p4y);
				int nps[] = { 4 };
				const cv::Point* ptr[1] = { pts[0] };
				cv::fillPoly(matchImage2D, ptr, nps, 1, cv::Scalar(255));

				cv::fillPoly(Foot2DBWTemp, ptr, nps, 1, cv::Scalar(0));
				cv::Rect WireBlobRect;
				//sort leftTop0, rightTop1, leftBot2, rightBot3
				for (int si = 0; si < 4; si++)
				{
					for (int sj = si+1; sj < 4; sj++)
					{
						if (pts[0][si].y > pts[0][sj].y)
							std::swap(pts[0][si] , pts[0][sj]);
					}
				}
				if (pts[0][0].x > pts[0][1].x)
					std::swap(pts[0][0], pts[0][1]);
				if (pts[0][2].x > pts[0][3].x)
					std::swap(pts[0][2], pts[0][3]);
				if (135 >= nFootDirection && nFootDirection > 45)//90
				{
					int stX = pts[0][1].x < pts[0][3].x ? pts[0][1].x : pts[0][3].x;
					int width = Foot2DBWTemp.cols - pts[0][1].x;
					int height = pts[0][3].y - pts[0][1].y;
					WireBlobRect = cv::Rect(stX, pts[0][1].y, width, height);
				}
				else if (225 >= nFootDirection && nFootDirection > 135)//180
				{
					int edY = pts[0][0].y > pts[0][1].y ? pts[0][0].y : pts[0][1].y;
					int width = pts[0][1].x - pts[0][0].x;
					WireBlobRect = cv::Rect(pts[0][0].x, 0, width, edY);
				}
				else if (315 >= nFootDirection && nFootDirection > 225)//270
				{
					int edX = pts[0][0].x > pts[0][2].x ? pts[0][0].x : pts[0][2].x;
					int height = pts[0][2].y - pts[0][0].y;
					WireBlobRect = cv::Rect(0, pts[0][0].y, edX, height);
				}
				else//0
				{
					int stY = pts[0][2].y < pts[0][3].y ? pts[0][2].y : pts[0][3].y;
					int width = pts[0][3].x - pts[0][2].x;
					int height = Foot2DBWTemp.rows - stY;
					WireBlobRect = cv::Rect(pts[0][2].x, stY, width, height);
				}
				cv::rectangle(Foot2DBWTemp, WireBlobRect, cv::Scalar(0), cv::FILLED);
#if _DEBUG
				cv::imwrite("D:\\testimage\\workimage\\fillpoly.bmp", matchImage2D);
#endif
			}

			PatResult->_Center_y[re] += rMaxRe.x;
			PatResult->_Center_x[re] += rMaxRe.y;
		}

		if (nMaxSc[0] < m_nFootPatternMatchScore && nMaxSc[1] < m_nFootPatternMatchScore)
			ret = false;

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::MatchModel Insp error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::MatchModel3D(CFoot_Model *pFoot, cv::Mat Image3D, cv::Mat ImagePad, std::shared_ptr<GeoResult_Foot> PatResult, cv::Rect* RectModelImg, cv::Mat ImgFoot2DBW, int* MatchModelNum, double dAngle)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		cv::Mat Nomal, Image8U;
		Image8U = Image3D.clone();
		cv::Mat Foot2DBWTemp = ImgFoot2DBW.clone();

		//Remove values ​​with 3D height of 0 or negative
		size_t floatstep_3DCorrect = Image8U.step / sizeof(float);
		for (int i = 0; i < Image8U.rows; i++)
		{
			float* rowptr = (float*)Image8U.data;
			for (int j = 0; j < Image8U.cols; j++)
			{
				int val = rowptr[i * floatstep_3DCorrect + j];
				if (val <= 0)
				{
					val = 0;
					rowptr[i * floatstep_3DCorrect + j] = 0;
				}
			}
		}

		cv::normalize(Image8U, Nomal, 0, 255, cv::NORM_MINMAX);
		Nomal.convertTo(Image8U, CV_32FC1);
#if _DEBUG
		cv::imwrite("D:\\testimage\\workimage\\Nomal.bmp", Image8U);
#endif

		CString FilePath;
		m_PatModel = std::make_shared<GeoModel_Foot>();
		GeoModel_Foot PatModel;
		nLine = __LINE__;
		int nModelCnt = m_ModelMng3D->GetModelCount(false);
		std::vector< std::shared_ptr<GeoResult_Foot>> vPatRe;
		std::shared_ptr<GeoResult_Foot> PatRe;
		std::vector<cv::Rect> RectModelList;
		std::vector<cv::Rect> RectWedgeList;
		std::vector<int> MatchNumList;
		nLine = __LINE__;
		double nMaxSc[2] = { 0,0 };
		int nFootDirection = pFoot->m_nFootAngle;
		bool b2Foot = pFoot->m_bUse2Foot;
		int footType = pFoot->m_nFoottype;
		int nDBCFoot = 0;
		if (pFoot->m_nUseOption2 & m_eFOOT_Data2_DBC)
			nDBCFoot = 1;
		if (dAngle != -INFINITY)
		{
			if (135 >= nFootDirection && nFootDirection > 45)
				dAngle -= 90;
			else if (225 >= nFootDirection && nFootDirection > 135)
				dAngle -= 180;
			else if (315 >= nFootDirection && nFootDirection > 225)
				dAngle -= 270;
		}
		int resultCnt = 1;
		if (b2Foot) resultCnt = PatResult->Count();
		for (int re = 0; re < resultCnt; re++)
		{
			vPatRe.clear();
			RectModelList.clear();
			for (int i = 0, nNum = 1; i < m_ModelMng3D->GetModelCount(false);nNum++)
			{
				FilePath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_%d.ppd"), nNum);
				std::shared_ptr<ModelFile_Foot> sMoFile = m_ModelMng3D->GetModel(FilePath);
				if (!sMoFile)
					continue;
				i++;
				if (45 >= nFootDirection || nFootDirection > 315)
					m_PatModel = sMoFile->milModel;
				else if (135 >= nFootDirection && nFootDirection > 45)
					m_PatModel = sMoFile->GetAngleModel(90);
				else if (225 >= nFootDirection && nFootDirection > 135)
					m_PatModel = sMoFile->GetAngleModel(180);
				else if (315 >= nFootDirection && nFootDirection > 225)
					m_PatModel = sMoFile->GetAngleModel(270);
				PatModel = *m_PatModel;
				if (PatModel.FootType != footType || PatModel.FootDirection != nFootDirection || PatModel.FootDBC != nDBCFoot)
					continue;
				if (dAngle != -INFINITY)
				{
					PatModel.setAngleRange(true, -(dAngle - 5), dAngle + 5, 0.5);

					nLine = __LINE__;
					PatModel.setAngle(0);
				}
				PatModel.Preprocess();
				PatModel.setFindSubPixel(false);

				PatRe = std::make_shared<GeoResult_Foot>();
				PatRe->Alloc(1);
				Img_Proc = std::make_shared<ImgProcessing>();
				Img_Proc->_ImgProcess(Image8U, PatModel);
				nLine = __LINE__;

				gm.SetImgProcess(Img_Proc);
				gm.FindModel(PatModel, *PatRe, false);
				nLine = __LINE__;
				bool bInPadRect = false;
				uchar* ptr = ImagePad.ptr(PatRe->_Center_y[0]);
				if (ptr[(int)PatRe->_Center_x[0]] > 0)
					bInPadRect = true;
				if (PatRe->_Equality[0] >= m_nFootPatternMatchScore && bInPadRect)
				{
					vPatRe.push_back(PatRe);
					int RectC = sMoFile->milModel->Image().cols;
					int RectR = sMoFile->milModel->Image().rows;
// 					if (135 >= nFootDirection && nFootDirection > 45 || 315 >= nFootDirection && nFootDirection > 225)
// 					{
// 						RectC = PatModel.Image().rows;
// 						RectR = PatModel.Image().cols;
// 					}
					RectModelList.push_back(cv::Rect(vPatRe[vPatRe.size() - 1]->_Center_y[0] - RectC / 2, vPatRe[vPatRe.size() - 1]->_Center_x[0] - RectR / 2, RectC, RectR));
					RectWedgeList.push_back(PatModel.WedgeArea);
					MatchNumList.push_back(i + 1);
				}

#if _DEBUG
				cv::Mat draw;
				draw = Image8U.clone();
				gm.DrawContours(draw, PatModel, *PatRe, cv::Scalar(255), 1);
				CString drawImgPath;
				drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D__%d_match.bmp"), i + 1);
				cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
				if (PatRe->_Equality[0] >= m_nFootPatternMatchStopScore && bInPadRect)
					break;
			}

			//여기서 한번 더 검증(패턴매칭으로 검출된 Center Point 가 Foot BW 영상내부에 존재하는 지 확인)
			bool bInsideFootBW_PatMatCtrPo = false;

			for (int i = 0; i < vPatRe.size(); i++)
			{
				bInsideFootBW_PatMatCtrPo = false;

				if (Foot2DBWTemp.rows != 0 && Foot2DBWTemp.cols != 0 && Image3D.rows != 0 && Image3D.cols != 0)
				{
					int c = vPatRe[i]->_Center_y[0];
					int r = vPatRe[i]->_Center_x[0];
					if (c < 0) c = 0;
					else if (c >= Foot2DBWTemp.cols) c = Foot2DBWTemp.cols - 1;
					if (r < 0) r = 0;
					else if (r >= Foot2DBWTemp.rows) r = Foot2DBWTemp.rows - 1;

					UCHAR* ImgPtr = Foot2DBWTemp.ptr(r);
					float* Img3DPtr = Image3D.ptr<float>(r);
					if (ImgPtr[c] == 255 && Img3DPtr[c] > 10)
					{
						bInsideFootBW_PatMatCtrPo = true;
					}
				}
				if (vPatRe[i]->_Equality[0] > nMaxSc[re] && bInsideFootBW_PatMatCtrPo)
				{
					PatResult->_Equality[re] = vPatRe[i]->_Equality[0];
					PatResult->_Center_x[re] = vPatRe[i]->_Center_x[0];
					PatResult->_Center_y[re] = vPatRe[i]->_Center_y[0];

					if (dAngle != -INFINITY)
					{
						if (135 >= nFootDirection && nFootDirection > 45)
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0] - 270;
						else if (225 >= nFootDirection && nFootDirection > 135)
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0] - 180;
						else if (315 >= nFootDirection && nFootDirection > 225)
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0] - 90;
						else
							PatResult->_Angle[re] = vPatRe[i]->_Angle[0];
					}
					else
						PatResult->_Angle[re] = vPatRe[i]->_Angle[0];

					nMaxSc[re] = vPatRe[i]->_Equality[0];
					RectModelImg[re].x = RectModelList[i].x;
					RectModelImg[re].y = RectModelList[i].y;
					RectModelImg[re].width = RectModelList[i].width;
					RectModelImg[re].height = RectModelList[i].height;

					float fx = RectModelList[i].x;
					float fy = RectModelList[i].y;
					int wedgeW = RectWedgeList[i].width < 1 ? RectModelList[i].width : RectWedgeList[i].width;
					int wedgeH = RectWedgeList[i].height < 1 ? RectModelList[i].height : RectWedgeList[i].height;
					RectModelImg[re + 2].x = RectWedgeList[i].x + (int)std::roundf(fx);
					RectModelImg[re + 2].y = RectWedgeList[i].y + (int)std::roundf(fy);
					RectModelImg[re + 2].width = wedgeW;
					RectModelImg[re + 2].height = wedgeH;
					MatchModelNum[re] = MatchNumList[i];
				}
			}
			if (re == 0)
			{
				cv::Point pts[1][4];
				int cx = PatResult->_Center_y[0];
				int cy = PatResult->_Center_x[0];

				double w = RectModelImg[re].width / 2;
				double h = RectModelImg[re].height / 2;

				double dLeng = std::sqrt(std::pow(w, 2) + std::pow(h, 2));
				double dRate = h / w;
				double dstdRadian = std::atan(dRate);
				double dstdDegree = dstdRadian / std::_Pi * 180;

				double dRadian1 = (-PatResult->_Angle[0] - (dstdDegree)) * std::_Pi / 180;
				double dRadian2 = (-PatResult->_Angle[0] - (180 - dstdDegree)) * std::_Pi / 180;
				double dRadian3 = (-PatResult->_Angle[0] - (180 + dstdDegree)) * std::_Pi / 180;
				double dRadian4 = (-PatResult->_Angle[0] - (360 - dstdDegree)) * std::_Pi / 180;
				double diffX1 = std::cos(dRadian1) * dLeng;
				double diffY1 = std::sin(dRadian1) * dLeng;
				double diffX2 = std::cos(dRadian2) * dLeng;
				double diffY2 = std::sin(dRadian2) * dLeng;
				double diffX3 = std::cos(dRadian3) * dLeng;
				double diffY3 = std::sin(dRadian3) * dLeng;
				double diffX4 = std::cos(dRadian4) * dLeng;
				double diffY4 = std::sin(dRadian4) * dLeng;

				int p1x = cx + (int)std::round(diffX1); int  p1y = cy + (int)std::round(diffY1);
				int p2x = cx + (int)std::round(diffX2); int  p2y = cy + (int)std::round(diffY2);
				int p3x = cx + (int)std::round(diffX3); int  p3y = cy + (int)std::round(diffY3);
				int p4x = cx + (int)std::round(diffX4); int  p4y = cy + (int)std::round(diffY4);
				pts[0][0] = cv::Point(p1x, p1y);
				pts[0][1] = cv::Point(p2x, p2y);
				pts[0][2] = cv::Point(p3x, p3y);
				pts[0][3] = cv::Point(p4x, p4y);
				int nps[] = { 4 };
				const cv::Point* ptr[1] = { pts[0] };
				cv::fillPoly(Image8U, ptr, nps, 1, cv::Scalar(0));

#if _DEBUG
				cv::imwrite("D:\\testimage\\workimage\\fillpoly3D.bmp", Image8U);
#endif
			}

		}

		if (nMaxSc[0] < m_nFootPatternMatchScore && nMaxSc[1] < m_nFootPatternMatchScore)
			ret = false;

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::MatchModel3D Insp error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}

bool CPInsp_Foot::MatchModel(cv::Mat Image2D, std::shared_ptr<GeoResult_Foot> PatResult, cv::Rect* szModelImg, int nFootDirection, int footType)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		cv::Mat matchImage2D = Image2D.clone();
		CString FilePath;
		m_PatModel = std::make_shared<GeoModel_Foot>();
		GeoModel_Foot PatModel;
		nLine = __LINE__;
		int nModelCnt = m_ModelMng2D->GetModelCount(false);
		std::vector< std::shared_ptr<GeoResult_Foot>> vPatRe;
		std::shared_ptr<GeoResult_Foot> PatRe;
		std::vector<cv::Rect> RectModelList;
		nLine = __LINE__;
		double nMaxSc = 0.0;
		vPatRe.clear();
		RectModelList.clear();
		for (int i = 0, nNum = 1; i < m_ModelMng2D->GetModelCount(false);nNum++)
		{
			FilePath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image2D_%d.ppd"), nNum);
			std::shared_ptr<ModelFile_Foot> sMoFile = m_ModelMng2D->GetModel(FilePath);
			if (!sMoFile)
				continue;
			i++;
			if (45 >= nFootDirection || nFootDirection > 315)
				m_PatModel = sMoFile->milModel;
			else if (135 >= nFootDirection && nFootDirection > 45)
				m_PatModel = sMoFile->GetAngleModel(90);
			else if (225 >= nFootDirection && nFootDirection > 135)
				m_PatModel = sMoFile->GetAngleModel(180);
			else if (315 >= nFootDirection && nFootDirection > 225)
				m_PatModel = sMoFile->GetAngleModel(270);

			PatModel = *m_PatModel;
			PatModel.Preprocess();
			PatModel.setFindSubPixel(false);
			if (PatModel.FootType != footType || PatModel.FootDirection != nFootDirection)
				continue;

			PatRe = std::make_shared<GeoResult_Foot>();
			PatRe->Alloc(1);
			Img_Proc = std::make_shared<ImgProcessing>();
			Img_Proc->_ImgProcess(matchImage2D, PatModel);
			nLine = __LINE__;

			gm.SetImgProcess(Img_Proc);
			gm.FindModel(PatModel, *PatRe, false);
			nLine = __LINE__;

			if (PatRe->_Equality[0] >= m_nFootPatternMatchScore)
			{
				vPatRe.push_back(PatRe);
				RectModelList.push_back(cv::Rect(vPatRe[vPatRe.size() - 1]->_Center_y[0] - PatModel.Image().cols / 2, vPatRe[vPatRe.size() - 1]->_Center_x[0] - PatModel.Image().rows / 2, PatModel.Image().cols, PatModel.Image().rows));
			}

#if _DEBUG
			cv::Mat draw;
			draw = matchImage2D.clone();
			gm.DrawContours(draw, PatModel, *PatRe, cv::Scalar(255), 1);
			CString drawImgPath;
			drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image2D__%d_match.bmp"), i + 1);
			cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
			if (PatRe->_Equality[0] >= m_nFootPatternMatchStopScore)
				break;
		}

		for (int i = 0; i < vPatRe.size(); i++)
		{
			if (vPatRe[i]->_Equality[0] > nMaxSc)
			{
				PatResult->_Equality[0] = vPatRe[i]->_Equality[0];
				PatResult->_Center_x[0] = vPatRe[i]->_Center_y[0];
				PatResult->_Center_y[0] = vPatRe[i]->_Center_x[0];
				PatResult->_Angle[0] = vPatRe[i]->_Angle[0];
				nMaxSc = vPatRe[i]->_Equality[0];
				szModelImg->x = RectModelList[i].x;
				szModelImg->y = RectModelList[i].y;
				szModelImg->width = RectModelList[i].width;
				szModelImg->height = RectModelList[i].height;
			}
		}


		if (nMaxSc < m_nFootPatternMatchScore)
			ret = false;

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::MatchModel Insp error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}
bool CPInsp_Foot::MatchModel3D(cv::Mat Image3D, std::shared_ptr<GeoResult_Foot> PatResult, cv::Rect* szModelImg, int nFootDirection, int footType)
{
	bool ret = true;
	int nLine = __LINE__;

	try
	{
		cv::Mat Nomal, Image8U;
		cv::normalize(Image3D, Nomal, 0, 255, cv::NORM_MINMAX);
		Nomal.convertTo(Image8U, CV_32FC1);
#if _DEBUG
		cv::imwrite("D:\\testimage\\workimage\\Nomal.bmp", Image8U);
#endif

		CString FilePath;
		m_PatModel = std::make_shared<GeoModel_Foot>();
		GeoModel_Foot PatModel;
		nLine = __LINE__;
		int nModelCnt = m_ModelMng3D->GetModelCount(false);
		std::vector< std::shared_ptr<GeoResult_Foot>> vPatRe;
		std::shared_ptr<GeoResult_Foot> PatRe;
		std::vector<cv::Rect> RectModelList;
		nLine = __LINE__;
		double nMaxSc = 0.0;
		vPatRe.clear();
		RectModelList.clear();
		for (int i = 0, nNum = 1; i < m_ModelMng3D->GetModelCount(false);nNum++)
		{
			FilePath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_%d.ppd"), nNum);
			std::shared_ptr<ModelFile_Foot> sMoFile = m_ModelMng3D->GetModel(FilePath);
			if (!sMoFile)
				continue;
			i++;
			if (45 >= nFootDirection || nFootDirection > 315)
				m_PatModel = sMoFile->milModel;
			else if (135 >= nFootDirection && nFootDirection > 45)
				m_PatModel = sMoFile->GetAngleModel(90);
			else if (225 >= nFootDirection && nFootDirection > 135)
				m_PatModel = sMoFile->GetAngleModel(180);
			else if (315 >= nFootDirection && nFootDirection > 225)
				m_PatModel = sMoFile->GetAngleModel(270);
			PatModel = *m_PatModel;
			PatModel.Preprocess();
			PatModel.setFindSubPixel(false);
			if (PatModel.FootType != footType || PatModel.FootDirection != nFootDirection)
				continue;


			PatRe = std::make_shared<GeoResult_Foot>();
			PatRe->Alloc(1);
			Img_Proc = std::make_shared<ImgProcessing>();
			Img_Proc->_ImgProcess(Image8U, PatModel);
			nLine = __LINE__;

			gm.SetImgProcess(Img_Proc);
			gm.FindModel(PatModel, *PatRe, false);
			nLine = __LINE__;

			if (PatRe->_Equality[0] >= m_nFootPatternMatchScore)
			{
				vPatRe.push_back(PatRe);
				RectModelList.push_back(cv::Rect(vPatRe[vPatRe.size() - 1]->_Center_y[0] - PatModel.Image().cols / 2, vPatRe[vPatRe.size() - 1]->_Center_x[0] - PatModel.Image().rows / 2, PatModel.Image().cols, PatModel.Image().rows));
			}

#if _DEBUG
			cv::Mat draw;
			draw = Image8U.clone();
			gm.DrawContours(draw, PatModel, *PatRe, cv::Scalar(255), 1);
			CString drawImgPath;
			drawImgPath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D__%d_match.bmp"), i + 1);
			cv::imwrite(std::string(CT2A(drawImgPath)), draw);
#endif
			if (PatRe->_Equality[0] >= m_nFootPatternMatchStopScore)
				break;
		}

		for (int i = 0; i < vPatRe.size(); i++)
		{
			if (vPatRe[i]->_Equality[0] > nMaxSc)
			{
				PatResult->_Equality[0] = vPatRe[i]->_Equality[0];
				PatResult->_Center_x[0] = vPatRe[i]->_Center_y[0];
				PatResult->_Center_y[0] = vPatRe[i]->_Center_x[0];
				PatResult->_Angle[0] = vPatRe[i]->_Angle[0];
				nMaxSc = vPatRe[i]->_Equality[0];
			}
		}

		if (PatResult->_Equality[0] < m_nFootPatternMatchScore)
			ret = false;

	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::MatchModel3D Insp error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		return false;
	}

	return ret;
}


double CPInsp_Foot::returnTime()
{
	double dSearchTime = m_dSearchTime;
	m_dSearchTime = 0.f;
	return dSearchTime;
}

int CPInsp_Foot::Pattern_Teach(CFoot_Model *pFoot, UCHAR* retImg, int sizeX, int sizeY, int use_algo, int nLight_Number, bool refPart)
{
	//NYJ 2019/11/21
	//우선 칼라이미지 매칭이 아닌 양각으로 매칭을 진행한다.

	//////////////////////////////////////////////////////////////////////////////////////
	void* m_fovImage_teach;// = (void*)m_procPatternMil->GetMilSrc(sWndAlgoImg);
	UCHAR *ucArrDstImg;
	int nWidth = pFoot->GetImageWidth();
	int nHeight = pFoot->GetImageLength();
	memset(ucArrDstImg, 0, sizeof(UCHAR) * pFoot->GetWindowImageSize());

	cv::Mat Img(nHeight, nWidth, CV_8UC1);
	cv::Mat* reMat;

	pFoot->GetImageMatrix(&Img, 1);			//footkind -> m_eFootBin_Foot(1)
	reMat = &Img;
	for (int y = 0; y < nHeight; y++)
	{
		for (int x = 0; x < nWidth; x++)
		{
			int nIndex = y * nWidth + x;
			int nIndex2 = (y + pFoot->mImageRect.top) * pFoot->_WidthOrg + (x + pFoot->mImageRect.left);
			ucArrDstImg[nIndex2] = reMat->data[nIndex];
		}
	}
	m_pProcMilAlgo->SaveWorkImg(reMat->data, nWidth, nHeight, _T("Foot_Teach.bmp"));

	Im::PIL_ID milSrc = m_procPatternMil->AllocBuff(pFoot->GetWindowImageWidth(), pFoot->GetWindowImageLength());
	Im::Buf::Put(milSrc, ucArrDstImg);

	m_fovImage_teach = (void*)milSrc;
	////////////////////////////////////////////////////////////////////////////////////////

	cv::Mat * cvFootImg;
	pFoot->GetImageMatrix(cvFootImg, (int)m_eFootBin::m_eFootBin_Foot);

	cv::Mat srcImg = *cvFootImg;

	int ret = eMPAT_SUCCESS;

	//int cx = m_teachCoordinate.cx;
	//int cy = m_teachCoordinate.cy;
	//int width = m_teachCoordinate.width;
	//int height = m_teachCoordinate.length;
	int cx = pFoot->m_FootCenterPoint.x;
	int cy = pFoot->m_FootCenterPoint.y;
	int width = pFoot->GetImageWidth();
	int height = pFoot->GetImageLength();

	int roiArea;		//모델 이미지의 면적 (가로*세로)
	roiArea = width * height;

	int searchCx = pFoot->GetImageStartXInWindowLT();
	int searchCy = pFoot->GetImageStartYInWindowLT();
	int searchWidth = pFoot->GetWindowImageWidth();
	int searchHeight = pFoot->GetWindowImageLength();

	int searchArea;		//검사 영역의 면적 (가로*세로)
	searchArea = searchWidth * searchHeight;

	double wndAngle = pFoot->dAngle;
	CString strPath = m_pTeachParam->modelPath;

	double posLimit = 5;//m_teachParam->searchAngleRange_Max;
	double negLimit = 5;//m_teachParam->searchAngleRange_Min;

// 	int bdCx = m_teachCoordinate.anyAngleCx;
// 	int bdCy = m_teachCoordinate.anyAngleCy;
// 	int bdSizeX = m_teachCoordinate.anyAngleWidth;
// 	int bdSizeY = m_teachCoordinate.anyAngleLength;
// 	int bdRoiArea = bdSizeX * bdSizeY;

	int nSX = 0;
	int nSY = 0;

	if (cx < 0 || cy < 0 || width < 0 || height < 0 || searchCx < 0 || searchCy < 0 || searchWidth < 0 || searchHeight < 0)
		return eMPAT_FAIL;

	int widthstep;

	int nChannel = 1;		//1 채널 이미지로 모델 생성

	BOOL isAnyAngle = IsAnyAngle(wndAngle);

	// 검사 영역 이미지 자르기
	//UCHAR* clipSearchImg = new UCHAR[searchArea];
	UCHAR* clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);


	// 티칭 모델 이미지 자르기
	//UCHAR* clipModelImg = new UCHAR[roiArea];
	UCHAR* clipModelImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);

	{
		nSX = RounD((double)searchCx - ((double)searchWidth / 2.0));
		nSY = RounD((double)searchCy - ((double)searchHeight / 2.0));
		if (nSX < 0)	nSX = 0;
		if (nSY < 0)	nSY = 0;

		m_procPatternMil->GetClipBuff_LT(m_fovImage_teach, clipSearchImg, nSX, nSY, searchWidth, searchHeight);

		nSX = RounD((double)cx - ((double)width / 2.0));
		nSY = RounD((double)cy - ((double)height / 2.0));
		if (nSX < 0)	nSX = 0;
		if (nSY < 0)	nSY = 0;

		m_procPatternMil->GetnStep(width);

		m_procPatternMil->GetClipBuff_LT(m_fovImage_teach, clipModelImg, nSX, nSY, width, height);

		m_procPatternMil->SaveWorkImg(&clipModelImg[0], width, height, _T("MPATTERN_clipModelImg.bmp"), nChannel);
	}

	//////////////////////////////////////////////////////////////////////////
	//UCHAR* imgTemp = new UCHAR[searchArea];
	UCHAR* imgTemp = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);


	if (wndAngle > 360)
		wndAngle -= 360;

	int w = 0;
	int h = 0;
	double rotateAngle = 0;
	//if (wndAngle > 0 && !isAnyAngle)		// ÀÏ¹Ý°¢ÀÏ °æ¿ì 0µµ·Î Ã³¸®
	//	rotateAngle = GetRotateAngle(width, height, wndAngle, &w, &h);

	//////////////////////////////////////////////////////////////////////////
	//alloc model 모델 생성 및 모델 트레이닝
	BOOL rotMode = (rotateAngle == 0.0) ? FALSE : TRUE;

	m_procPatternMil->SetnChannel(nChannel == 3 ? 3 : 1);

	int allocRst = m_procPatternMil->AllocPatModel(clipModelImg, width, height, rotateAngle, w, h);
	m_procPatternMil->SetAngleMode(posLimit, negLimit, 0.5, rotMode);
	m_procPatternMil->SetSearchStartAngle(0);
	m_procPatternMil->SetFindSubPixel(rotMode, refPart);
	m_procPatternMil->PreprocModel(m_fovImage_teach, rotMode, use_algo, nLight_Number);
	//////////////////////////////////////////////////////////////////////////

	//패턴 매칭
	//search
	int searchCnt = 0;
	double score = 0;
	double angle = 0;
	double posX = 0;
	double posY = 0;
	if (allocRst == eMPAT_SUCCESS)
	{
		Img_Proc = std::make_shared<ImgProcessing>();
		m_procPatternMil->ImgProcess(clipSearchImg, searchWidth, searchHeight, Img_Proc);
		searchCnt = m_procPatternMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);	//패턴 매칭

		if (searchCnt > 0)
		{
			m_procPatternMil->GetPatResult(&score, &angle, &posX, &posY);	//매칭 결과 얻기 (찾은 위치 및 찾은 점수)
			double dX = posX + (searchCx - (searchWidth / 2.0));
			double dY = posY + (searchCy - (searchHeight / 2.0));

			/*
			double dCogX_pixel = m_pInspPartInfo->dCenterX - m_pInspPartInfo->dWidth / 2 + dX;
			double dCogY_pixel = m_pInspPartInfo->dCenterY - m_pInspPartInfo->dLength / 2 + dY;

			double dshiftX = (dCogX_pixel * m_resolX) - ((m_pInspPartInfo->m_dPartWidth / 2) * m_resolX);
			double dshiftY = ((m_pInspPartInfo->m_dPartHeight / 2) * m_resolY) - (dCogY_pixel * m_resolY);

			if ((m_pInspPartInfo->dAngle == 90) || (m_pInspPartInfo->dAngle == 180) || (m_pInspPartInfo->dAngle == 270))
			{
				m_proc3d.CorrectCoordinate(dshiftX, dshiftY, 0, 0, -m_pInspPartInfo->dAngle,
					0, 0, &dshiftX, &dshiftY);
			}

			// 			double cogX_board = 0.0;
			// 			double cogY_board = 0.0;
						//CvtPixelToBoard(m_teachCoordinate.fovCx, m_teachCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);
			*/

			m_pTeachParam->retScore = score;
			m_pTeachParam->retAngle = angle;
			m_pTeachParam->retPosX = posX;// - (searchCx - (searchWidth / 2.0));
			m_pTeachParam->retPosY = posY;// - (searchCy - (searchHeight / 2.0));
// 			m_pAlgoPattern->m_dRetCogX = dshiftX;
// 			m_pAlgoPattern->m_dRetCogY = dshiftY;

			if (m_pAlgoPattern)
				m_pAlgoPattern->SetParam(m_pTeachParam);
			//////////////////////////////////////////////////////////////////////////  display image
			if (retImg)
			{
				m_procPatternMil->GetResultImg(clipSearchImg, searchWidth, searchHeight, imgTemp);

				double factor = m_procPatternMil->GetResizeFactor(sizeX, sizeY, searchWidth, searchHeight);
				m_procPatternMil->ResizeImg(imgTemp, retImg, searchWidth, searchHeight, sizeX, sizeY, 3, factor);
			}
			//////////////////////////////////////////////////////////////////////////
		}
		Img_Proc->Free();
	}

	if (clipSearchImg)
		//delete[] clipSearchImg;
		g_pMManager->pem_delete(clipSearchImg, true);


	if (clipModelImg)
		//delete[] clipModelImg;
		g_pMManager->pem_delete(clipModelImg, true);


	if (imgTemp)
		//delete[] imgTemp;
		g_pMManager->pem_delete(imgTemp, true);


	return ret;
}

bool CPInsp_Foot::FindFootROI_BW(CFoot_Model* pFoot, std::vector<FootRect>& rst)
{
	int nPrevious = rst.size();

	RstAlgoFoot sRstAlgo;
	int footCnt = 1;
	//Get Foot Search ROI Size
	int nWidth = pFoot->GetImageWidth();		//Foot Search Area 
	int nHeight = pFoot->GetImageLength();

	USHORT* LabelImage = NULL;
	memset(&sRstAlgo.m_sEnd, 0, sizeof(POINTF));

	cv::Mat Img(nHeight, nWidth, CV_8UC1), ImgWedge(nHeight, nWidth, CV_8UC1), ImgWing(nHeight, nWidth, CV_8UC1), Img3DArea(nHeight, nWidth, CV_32FC1), roImg, ImgPatTarg(nHeight, nWidth, CV_8UC1);
	cv::Rect ImageRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);

	//0: find
	int nBlobCnt = 0;
	bool bIsFoundFoot = false;

	nBlobCnt = FindFoot(pFoot, &sRstAlgo, &Img, &ImgWedge, &ImgWing, &ImgPatTarg, -2);

	if (nBlobCnt > 0) bIsFoundFoot = true;

	footCnt = nBlobCnt;
	cv::Mat ImgTemp(nHeight, nWidth, CV_32FC1, pFoot->pf3D);
	ImgTemp.copyTo(Img3DArea);

	//NYJ 2019/10/08
	std::vector<int> nMatLeft;
	std::vector<int> nMatRight;
	std::vector<int> nMatTop;
	std::vector<int> nMatBottom;

	std::vector<int> nMatLabel;
	std::vector<int> LeftIdx;
	std::vector<cv::Rect> rcFootROIList;

	int nImgWidth = ImageRect.width;
	int nImgHeight = ImageRect.height;
	int imgSize = nImgWidth * nImgHeight;
	long pLebel[PTR_BLOB_MAX];
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &LabelImage, imgSize);
	memset(pLebel, 0, sizeof(long) * PTR_BLOB_MAX);
	memset(LabelImage, 0, sizeof(USHORT) * imgSize);
	GetBlob_BondingFloor(pFoot, &sRstAlgo, nBlobCnt, footCnt, pLebel, LabelImage, &Img, &ImgWedge, &ImgWing, &Img3DArea, nMatLabel, nMatLeft, nMatRight, nMatTop, nMatBottom, LeftIdx, rcFootROIList, nWidth, nHeight);

	cv::Mat PadImg;
	Calc_padBinImage2(pFoot, &sRstAlgo, ImageRect, &PadImg, Img3DArea);

	//Pad 영역 Rect로 변
	RECT padRect;
	padRect.left = pFoot->mImageRect.left;
	padRect.right = pFoot->mImageRect.right;
	padRect.top = pFoot->mImageRect.top;
	padRect.bottom = pFoot->mImageRect.bottom;

	sRstAlgo.m_nFindBin = 0;
	sRstAlgo.m_bOK = (footCnt >= 1);

	int nFootDirection = pFoot->m_nFootAngle;
	
	for (int idx = 0; idx < footCnt&&idx < LeftIdx.size(); idx++)
	{
		int i = LeftIdx[idx];
		//5 to 8
		cv::Mat cvFootImg(nHeight, nWidth, CV_8UC1), cvWedge(nHeight, nWidth, CV_8UC1), cvWing(nHeight, nWidth, CV_8UC1), cv3DArea(nHeight, nWidth, CV_32FC1);
		if (bIsFoundFoot && SetLabelImage(LabelImage, nMatLabel[i], &ImgWedge, &ImgWing, &Img3DArea, ImageRect, &cvFootImg, &cvWedge, &cvWing, &cv3DArea) == 1)
		{
			cv::Point Center;
			if (Insp_Foot_BW(pFoot, Center, &nMatLeft[i], &nMatRight[i], &cvFootImg, &cvWedge, &cvWing, &cv3DArea, &PadImg, nFootDirection))
			{
				FootRect footrect;
				memset(&footrect, 0, sizeof(FootRect));

				//Foot 중심점
				footrect.m_Point.x = Center.x + nMatLeft[i] + pFoot->mImageRect.left;
				footrect.m_Point.y = Center.y + pFoot->mImageRect.top;

				//Foot Rect
				footrect.m_FootRect.left = nMatLeft[i] + pFoot->mImageRect.left;
				footrect.m_FootRect.top = nMatTop[i] + pFoot->mImageRect.top;
				footrect.m_FootRect.right = nMatRight[i] + pFoot->mImageRect.left;
				footrect.m_FootRect.bottom = nMatBottom[i] + pFoot->mImageRect.top;

				//Pad Rect
				bool bFind = false;

				POINT pt;
				pt.x = Center.x + nMatLeft[i];
				pt.y = Center.y;
				CRect rect = padRect;

				if (rect.PtInRect(pt))
				{
					footrect.m_PadRect.left = rect.left + pFoot->mImageRect.left;
					footrect.m_PadRect.top = rect.top + pFoot->mImageRect.top;
					footrect.m_PadRect.right = rect.right + pFoot->mImageRect.left;
					footrect.m_PadRect.bottom = rect.bottom + pFoot->mImageRect.top;

					bFind = true;
				}

				if (!bFind)
				{
					footrect.m_PadRect.left = footrect.m_FootRect.left;
					footrect.m_PadRect.top = footrect.m_FootRect.top;
					footrect.m_PadRect.right = footrect.m_FootRect.right;
					footrect.m_PadRect.bottom = footrect.m_FootRect.bottom;
				}

				rst.emplace_back(footrect);
			}

		}
	}

	Delete_1DArray(&LabelImage);


	return nPrevious < rst.size();
}

bool CPInsp_Foot::Insp_Foot_BW(CFoot_Model *pFoot, cv::Point& CenterPo, int* left, int* right, cv::Mat* cvFootImg, cv::Mat* /*cvWedgeImg*/, cv::Mat* cvWing, cv::Mat* /*cv3DImg*/, cv::Mat* /*PadImg*/, int nFootDirection)
{
	//initialize 
	int nWidth = pFoot->GetImageWidth();
	int nHeight = pFoot->GetImageLength();
	cv::Rect ImageRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);

	double seta(0.0);
	cv::Mat Img;
	cv::Mat mMask;

	//티칭된 Foot Bin Img 예외처리 (이진 이미지가 All Black 일 경우 함수 리턴시킴)
	cv::Mat FootImg, retMatIntg, retDev;
	FootImg = (*cvFootImg)(cv::Rect(*left, 0, *right - (*left), cvFootImg->rows));
	cv::integral(FootImg, retMatIntg, retDev);
	UINT* unREPtr = retMatIntg.ptr<UINT>(retMatIntg.rows - 1);
	if (unREPtr[retMatIntg.cols - 1] <= 0)
		return false;

	//Foot CenterPos, Seta, Lenght End 구하는 함수
	Img = (*cvWing)(cv::Rect(*left, 0, *right - (*left), cvFootImg->rows));
	mMask = (*cvFootImg)(cv::Rect(*left, 0, *right - (*left), cvFootImg->rows)).clone();
	cv::Point2d WingLen;
	bool flag = getFootCenterPosAndSeta(CenterPo, seta, Img, mMask, WingLen, nFootDirection);

	if (flag == false)	//missing
	{
		return false;
	}

	return true;
}


bool CPInsp_Foot::AllImageClip(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrSrcColorOpt, cv::RotatedRect& SelectedRect)
{
	bool Ret = false;
	int nLine = __LINE__;

	try
	{
#pragma region Calculate align
		//init Image
		CFoot_Model model;
		model.mImageRect = ptrSrcColorOpt.m_rcImageRect;
		cv::Rect ImageRect(model.mImageRect.left, model.mImageRect.top,
			model.mImageRect.right - model.mImageRect.left, model.mImageRect.bottom - model.mImageRect.top);

		int nWidth = ptrSrcColorOpt.m_rcImageRect.right - ptrSrcColorOpt.m_rcImageRect.left;
		int nHeight = ptrSrcColorOpt.m_rcImageRect.bottom - ptrSrcColorOpt.m_rcImageRect.top;

		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgTop_R, sizeof(UCHAR) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgTop_B, sizeof(UCHAR) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgTop_G, sizeof(UCHAR) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgTop_W, sizeof(UCHAR) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgMiddle_R, sizeof(UCHAR) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgMiddle_B, sizeof(UCHAR) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgBottom_R, sizeof(UCHAR) * nWidth * nHeight);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.m_ImageBuffer->imgBottom_B, sizeof(UCHAR) * nWidth * nHeight);
		//Set background Image
		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgTop_R, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgTop_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgTop_G, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgTop_G, ImageRect.x, ImageRect.y, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgTop_B, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgTop_B, ImageRect.x, ImageRect.y, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgTop_W, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgTop_W, ImageRect.x, ImageRect.y, nWidth, nHeight);

		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgMiddle_R, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgMiddle_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgMiddle_B, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgMiddle_B, ImageRect.x, ImageRect.y, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgBottom_R, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgBottom_R, ImageRect.x, ImageRect.y, nWidth, nHeight);
		m_pProcMilAlgo->GetClipImage_LT(ptrSrcColorOpt.m_sFovImg.imgBottom_B, ptrSrcColorOpt.m_sFovImg.nImageSizeX, ptrSrcColorOpt.m_sFovImg.nImageSizeY, model.m_ImageBuffer->imgBottom_B, ImageRect.x, ImageRect.y, nWidth, nHeight);

		int _3DCX = ImageRect.x + (nWidth / 2.) - ptrSrcColorOpt.m_rcImageRect.left;;
		int _3DCY = ImageRect.y + (nHeight / 2.) - ptrSrcColorOpt.m_rcImageRect.top;

		if (!((CRect)sInspFoot->m_rSpecRect).IsRectNull())
		{
			model.m_PadRect = sInspFoot->m_rSpecRect;

			if (model.m_PadRect.left < 0)
				model.m_PadRect.left = 0;

			if (model.m_PadRect.top < 0)
				model.m_PadRect.top = 0;

			if (model.m_PadRect.right > model.mImageRect.right)
				model.m_PadRect.right = model.mImageRect.right;

			if (model.m_PadRect.bottom > model.mImageRect.bottom)
				model.m_PadRect.bottom = model.mImageRect.bottom;
		}
		cv::Rect PadRect(model.m_PadRect.left, model.m_PadRect.top,
			model.m_PadRect.right - model.m_PadRect.left, model.m_PadRect.bottom - model.m_PadRect.top);

		model.m_ImageBuffer->nImageSizeX = nWidth;
		model.m_ImageBuffer->nImageSizeY = nHeight;
		double dArea = 0, dCX = 0, dCY = 0;
		AlgoBlob sAlgoBlob = model.GetAlgoBlob((int)m_eFootBin::m_eFootBin_Pad);
		memcpy(&model.m_sArrBin, &sInspFoot->m_sArrBin, sizeof(AlgoBin) * m_eFootBin_Total); //Binary
		memcpy(&model.m_fArrOptionValue, &sInspFoot->m_fArrOptionValue, sizeof(float) *m_eFoot_Total* eMMD_Total); //검사 옵션
		cv::Mat ImgBackgr(nHeight, nWidth, CV_8UC1);
		model.GetImageMatrix(&ImgBackgr, (int)m_eFootBin::m_eFootBin_Pad);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&model.pf3D, sizeof(float) * nWidth * nHeight);
		if (0 != m_pProcMilAlgo->GetCropZmap(sWndAlgoImg.m_fArr3D, model.pf3D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _3DCX, _3DCY, nWidth, nHeight))
		{
			return nullptr;
		}
		GetBinImage(&model, &ImgBackgr, m_eFootBin_Pad, NULL, false);
		//calculate Center & Angle
		Ret = InspFootAlign(ImgBackgr, PadRect, SelectedRect);
#pragma endregion
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_WireBonding::AllImageClip() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}
bool CPInsp_Foot::InspFootAlign(cv::Mat vImage, cv::Rect TeachRect, cv::RotatedRect& SelectedRect)
{
	bool ret = false;
	int nLine = __LINE__;

	try
	{
		nLine = __LINE__;

		std::vector< cv::RotatedRect> vRectCand;
		vRectCand.clear();
		GerAlignRect(vImage, vRectCand);
		nLine = __LINE__;

		cv::Point2f ct;

		SelectedRect.center.x = ct.x = TeachRect.x + (TeachRect.width / 2);
		SelectedRect.center.y = ct.y = TeachRect.y + (TeachRect.height / 2);
		SelectedRect.size.width = TeachRect.width;
		SelectedRect.size.height = TeachRect.height;
		double dMaxSz = TeachRect.width * TeachRect.height * 1.3;
		double dMinSz = TeachRect.width * TeachRect.height * 0.7;
		double dMinDiff = (std::powf(TeachRect.width, 2) + std::powf(TeachRect.height, 2));
		nLine = __LINE__;

		if (vRectCand.size() < 1)
		{
			SelectedRect.angle = 0.0f;
			return false;
		}
		nLine = __LINE__;

		for (int i = 0; i < vRectCand.size(); i++)
		{
			cv::RotatedRect roRect = vRectCand[i];
			double roSz = roRect.size.width * roRect.size.height;
			double diff = (std::powf(roRect.center.x - ct.x, 2) + std::powf(roRect.center.y - ct.y, 2));
			if (roSz <= dMaxSz && roSz >= dMinSz && diff < dMinDiff)
			{
				dMinDiff = diff;
				SelectedRect = roRect;
				ret = true;
			}
		}
		nLine = __LINE__;
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_WireBonding::InspFootAlign() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return ret;
}
bool CPInsp_Foot::GerAlignRect(cv::Mat vImage, std::vector< cv::RotatedRect>& vDstRect)
{
	bool ret = false;
	int nLine = __LINE__;

	try
	{
		cv::Mat vLocalImg;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(vImage, vLocalImg, kernel);
		cv::dilate(vLocalImg, vLocalImg, kernel);

		//delete silk
		DeleteSilk(&vLocalImg);
		std::vector<std::vector<cv::Point>> vContMax;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(vLocalImg, vContMax, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		nLine = __LINE__;
#if _DEBUG
		cv::Mat dstback = vLocalImg.clone();
		cv::Mat testimage1;
		cv::cvtColor(dstback, testimage1, cv::COLOR_GRAY2RGB);
#endif
		for (int i = 0; i < vContMax.size(); i++)
		{
			cv::RotatedRect RoRe = cv::minAreaRect(vContMax[i]);
			vDstRect.push_back(RoRe);

#if _DEBUG
			cv::Rect re = cv::boundingRect(vContMax[i]);
			cv::rectangle(testimage1, re, cv::Scalar(0, 0, 255), 2);
			//cv::drawContours(testimage1,vContMax,i,cv::Scalar(255));
			cv::Point2f ArrRe2[4];
			RoRe.points(ArrRe2);
			for (int i = 0; i < 4; i++)
				cv::line(testimage1, ArrRe2[i], ArrRe2[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
#endif
		}
#if _DEBUG
		cv::imwrite("D:\\GerAlignRect.bmp", testimage1);
#endif

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_WireBonding::GerAlignRect() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return ret;
}

int CPInsp_Foot::SearchPattern(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst)
{
	int ret = e_NG;
	int nFootDir = 0;	//0-Up, 1-Down

	if (!m_pInspParam)
		return ret;

	void* m_fovImage_insp = M_NULL; //= (void*)m_procPatternMil->GetMilSrc(sWndAlgoImg);
	void* m_fovImage_insp_color = M_NULL;

	double wndAngle = 0;
	if (IsAnyAngle(wndAngle))
		wndAngle = 0;

	if (wndAngle > 360)
		wndAngle -= 360;

	double stdCogX[CNT_PATTERN_PATH];
	double stdCogY[CNT_PATTERN_PATH];
	memcpy(stdCogX, m_pInspParam->stdCogX, sizeof(double) * CNT_PATTERN_PATH);
	memcpy(stdCogY, m_pInspParam->stdCogY, sizeof(double) * CNT_PATTERN_PATH);

	//Set Angle Limit - min/max Range
	double posLimit = 45;	//m_inspParam->searchAngleRange_Max;
	double negLimit = 45;	//m_inspParam->searchAngleRange_Min;

	double angleTemp = 0.;
	CString strFullPath = _T("");		//Get Foot Model for Pattern Match
	double dAngleRotate = 0.0;

	BOOL bUseAlgoPattern = TRUE;	//BOOL bUseAlgoPattern = (m_pAlgoPattern && (m_pAlgoPattern->m_nCntPatternPath > 0));

	//Init Pattern Matching Result 
	int nCntPatternPath = 1;	//int nCntPatternPath = (bUseAlgoPattern) ? m_pAlgoPattern->m_nCntPatternPath : 1;
	RstInspPattern OrgResult;
	InitPatternResultStruct(&OrgResult);
	bool bDecision(true);
	m_rstInspPattern.score = 0;
	m_rstInspPattern.angle = 0;
	m_rstInspPattern.offsetX = 0;
	m_rstInspPattern.offsetY = 0;

	double dTempAngle = 0.;
	double offsetX = 0.;
	double offsetY = 0.;

	//Foot Model Imageㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//
	int nModelWidth = pFoot->GetImageWidth();
	int nModelHeight = pFoot->GetImageLength();

	//2D Foot Model
	cv::Mat ImgFootModel(nModelHeight, nModelWidth, CV_8UC1, cv::Scalar(0));
	pFoot->GetImageMatrix(&ImgFootModel, 1);	//footkind -> m_eFootBin_Foot(1)

	//Get Only-Pad(pad BW - foot BW) 2D binary Image 
	int nImageHeight = pFoot->GetImageLength();
	int nImageWidth = pFoot->GetImageWidth();

	cv::Mat ImgPad(nImageHeight, nImageWidth, CV_8UC1);
	//pFoot->GetImageMatrix(&ImgPad, (int)m_eFootBin::m_eFootBin_Pad);
	//GetBinImage(pFoot, &ImgPad, m_eFootBin_Pad, NULL, false);
	pFoot->GetPadBinImage(&ImgPad);

	cv::Mat Imgfoot(nImageHeight, nImageWidth, CV_8UC1);
	pFoot->GetImageMatrix(&Imgfoot, (int)m_eFootBin::m_eFootBin_Foot);
	GetBinImage(pFoot, &Imgfoot, m_eFootBin_Foot, NULL, false);
	cv::Mat Imgfoot_inverse(nImageHeight, nImageWidth, CV_8UC1);
	cv::bitwise_not(Imgfoot, Imgfoot_inverse);

	cv::Mat ImgOnlyPadArea;
	cv::bitwise_and(ImgPad, Imgfoot_inverse, ImgOnlyPadArea);

	//3D Foot Model (Foot model ROI 3d)
	cv::Mat ImgFootModel_3D(nModelHeight, nModelWidth, CV_32FC1, pFoot->pf3D);
	//ImgFootModel_3D.convertTo(ImgFootModel_3D, CV_8U);

	//Get Part 3D  
	cv::Mat ImgPart3D(pModel->m_pPartImageBuffer->nImageSizeY, pModel->m_pPartImageBuffer->nImageSizeX, CV_32FC1, pModel->m_pPart3D);

	//Remove values ​​with Part 3D height of 0 or negative
	size_t floatstep_3DCorrect = ImgPart3D.step / sizeof(float);
	for (int i = 0; i < ImgPart3D.rows; i++)
	{
		float* rowptr = (float*)ImgPart3D.data;
		for (int j = 0; j < ImgPart3D.cols; j++)
		{
			int val = rowptr[i * floatstep_3DCorrect + j];
			if (val <= 0)
			{
				val = 0;
				rowptr[i * floatstep_3DCorrect + j] = 0;
			}
		}
	}

	//Clip Foot ROI(32FC1) from Part 3D  
	cv::Rect rtFootRoi(pFoot->mImageRect.left, pFoot->mImageRect.top, pFoot->mImageRect.right - pFoot->mImageRect.left, pFoot->mImageRect.bottom - pFoot->mImageRect.top);
	cv::Mat model3DRoi = ImgPart3D(rtFootRoi).clone();

	//cv::Mat Part3DNormalize;
	//cv::normalize(ImgFootModel3DCorrect, ImgFootModel3DCorrect, 0, 255, cv::NORM_MINMAX);
	//ImgFootModel3DCorrect.convertTo(ImgFootModel3DCorrect, CV_8UC1);

	cv::Mat part3DGradX, part3DGradY, part3DGradSumXY;
	GetPart3DGradientImg(part3DGradX, part3DGradY, part3DGradSumXY, pModel);	// 32FC1, 32FC1, 8UC1 

	//Make 3D Gradient Foot Model
	cv::Mat model3DRoiFoot = part3DGradSumXY(rtFootRoi).clone();


	/*	//과거에 테스트했던 알고리즘 주석처리
	//Calc Foot Model zero Point (Pad Average Height)
	int sum = 0, matchingCnt = 0;
	int nFoot3DmaxHeight = 0;
	size_t floatstep = model3DRoi.step / sizeof(float);
	for (int i = 0; i < model3DRoi.rows; i++)
	{
		float* rowptr = (float*)model3DRoi.data;
		for (int j = 0; j < model3DRoi.cols; j++)
		{
			int val = rowptr[i * floatstep + j];
			if (ImgOnlyPadArea.data[i * ImgOnlyPadArea.step + j] > 0)
			{
				matchingCnt++;
				sum += val;
			}

			if (Imgfoot.data[i * Imgfoot.step + j] == 255)
			{
				if (val > nFoot3DmaxHeight)
					nFoot3DmaxHeight = val;
			}
		}
	}
	if(sum!=0 && matchingCnt!=0)
		sum /= matchingCnt;

	nFoot3DmaxHeight = nFoot3DmaxHeight - sum;
	*/	//과거에 테스트했던 알고리즘 주석처리

	/////////////////////////////////////////////////////////////////3D 기울기 기반
// 	cv::Mat Sx, Sy, Sxy;
// 
// 	cv::Sobel(CannyFootModelSrc1, Sx, CV_16S, 1, 0, 3);
// 	cv::Sobel(CannyFootModelSrc1, Sy, CV_16S, 0, 1, 3);
// 	Sxy = Sx + Sy;
// 
// 	Sxy = cv::abs(Sxy);
// 	Sxy.convertTo(Sxy, CV_8U);
	/////////////////////////////////////////////////////////////// 3D 기울기 기반

	//UCHAR* ptrFoot = ImgFootModel.data;
	//UCHAR* ptrFoot = Sxy.data; //3D 기울기 기반
	//UCHAR* ptrFoot = model3DRoiFoot.data; //3D 바닥 평활화
	UCHAR* ptrFoot = model3DRoiFoot.data;
	UCHAR* ucArrModelImg = ptrFoot;
	m_pProcMilAlgo->SaveWorkImg(ucArrModelImg, nModelWidth, nModelHeight, _T("Foot_ModelImg.bmp"));

	Im::PIL_ID milSrcModelImg = M_NULL;
	milSrcModelImg = m_procPatternMil->AllocBuff(nModelWidth, nModelHeight);
	Im::Buf::Put(milSrcModelImg, ucArrModelImg);

	void* m_footImg_teachModel = M_NULL;
	m_footImg_teachModel = (void*)milSrcModelImg;		//Pattern Model
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//

	int searchWidth, searchHeight, searchArea;
	int nSearchAreaWidth, nSearchAreaHeight;
	POINTF pfFootCenterPoint = pFoot->m_FootCenterPoint;


	if (pModel->m_pFoot1->m_FootCenterPoint.x == pfFootCenterPoint.x && pModel->m_pFoot1->m_FootCenterPoint.y == pfFootCenterPoint.y)
	{
		nFootDir = 0;

		//Pattern Matching Insp Area 

		searchWidth = ROI.right - ROI.left;
		searchHeight = ROI.bottom - ROI.top;

		nSearchAreaWidth = ROI.right - ROI.left;
		nSearchAreaHeight = ROI.bottom - ROI.top;

		searchArea = searchWidth * searchHeight;

		cv::Rect cvImgRectInspArea;
		cvImgRectInspArea.x = ROI.left;
		cvImgRectInspArea.y = ROI.top;
		cvImgRectInspArea.width = ROI.right - ROI.left;
		cvImgRectInspArea.height = ROI.bottom - ROI.top;

		//2D Insp Area
		cv::Mat ImgInspArea(nSearchAreaHeight, nSearchAreaWidth, CV_8UC1);
		pModel->GetImageMatrix(&ImgInspArea, pFoot, 1, cvImgRectInspArea);

		//Clip Insp Area From Part 3D
		cv::Rect inspAreaRoi(cvImgRectInspArea.x, cvImgRectInspArea.y, cvImgRectInspArea.width, cvImgRectInspArea.height);

		cv::Mat ImgInspArea3DClone = ImgPart3D(inspAreaRoi).clone();
		cv::Mat ImgInspArea3DClone_convert8;
		ImgInspArea3DClone.convertTo(ImgInspArea3DClone_convert8, CV_8UC1);		//8UC1

		cv::Mat ImgInspArea3DClone_2 = ImgPart3D(inspAreaRoi).clone();		//32FC1

		cv::Mat ImgInspArea3DGradi = part3DGradSumXY(inspAreaRoi).clone();		//3D 기울기로 매칭

		cv::Mat ImgInspAreaDx = part3DGradX(inspAreaRoi).clone();
		cv::Mat ImgInspAreaDx_8bit;
		ImgInspAreaDx.convertTo(ImgInspAreaDx_8bit, CV_8UC1);

		cv::Mat ImgInspAreaDy = part3DGradY(inspAreaRoi).clone();
		cv::Mat ImgInspAreaDy_8bit;
		ImgInspAreaDy.convertTo(ImgInspAreaDy_8bit, CV_8UC1);
/*
		CString sSrc_image_path;
		sSrc_image_path.Format(_T("d:\\testimage\\workimage\\inputSdx_Up.bmp"));
		cv::imwrite(std::string(CT2A(sSrc_image_path)), ImgInspAreaDx_8bit);

		sSrc_image_path.Format(_T("d:\\testimage\\workimage\\inputSdy_Up.bmp"));
		cv::imwrite(std::string(CT2A(sSrc_image_path)), ImgInspAreaDy_8bit);*/

		/*	//과거에 테스트했던 알고리즘 주석처리
		//3D 바닥 평활화
		cv::Mat convert3D;
		//ImgInspArea3DClone -= sum;

		//Subtract the Avg height value of the PAD area (obtained from the Foot Model)
		int nCntNegative = 0, nSumNegative = 0;
		bool bNegativeFlag = false;
		size_t floatstep = ImgInspArea3DClone.step / sizeof(float);
		for (int i = 0; i < ImgInspArea3DClone.rows; i++)
		{
			float* rowptr = (float*)ImgInspArea3DClone.data;
			for (int j = 0; j < ImgInspArea3DClone.cols; j++)
			{
				int val = rowptr[i * floatstep + j];

				//calc Avg Negative area
				if (val >= 0 && (val - sum) < 0)
				{
					bNegativeFlag = true;
					nCntNegative++;
					nSumNegative += (val - sum);

				}

				rowptr[i * floatstep + j] -= sum;
			}
		}

		if (bNegativeFlag)		//If it has a negative value, the subtraction operation is performed once more.
		{
			nSumNegative /= nCntNegative;
			ImgInspArea3DClone -= nSumNegative;
		}

		ImgInspArea3DClone.convertTo(convert3D, CV_8UC1);

		size_t floatstep2 = ImgInspArea3DClone_2.step / sizeof(float);
		for (int i = 0; i < ImgInspArea3DClone_2.rows; i++)
		{
			float* rowptr = (float*)ImgInspArea3DClone_2.data;
			for (int j = 0; j < ImgInspArea3DClone_2.cols; j++)
			{
				int val = rowptr[i * floatstep2 + j];

				if (val > (sum + nFoot3DmaxHeight))
				{
					convert3D.data[i * convert3D.step + j] = 0;
				}
				if (val < 0)
				{
					convert3D.data[i * convert3D.step + j] = 0;
				}
			}
		}

		// 		cv::Mat CannyInspAreaSrc1, CannyInspAreaDst1;
		//
		// 		convert3D.convertTo(CannyInspAreaSrc1, CV_8UC1);
		// 		cv::Canny(CannyInspAreaSrc1, CannyInspAreaDst1, 50, 70, 3, false);

		//////////////////////////////////// 3D기울기 기반
		cv::Mat SxModel, SyModel, SumModel;
		cv::Sobel(convert3D, SxModel, CV_16S, 1, 0, 3);
		cv::Sobel(convert3D, SyModel, CV_16S, 0, 1, 3);
		SumModel = SxModel + SyModel;

		SumModel = cv::abs(SumModel);
		SumModel.convertTo(SumModel, CV_8U);
		///////////////////////////////////// 3D기울기 기반


// 		for (int i = 0; i < ImgInspArea.rows; i++)
// 		{
// 			for (int j = 0; j < ImgInspArea.cols; j++)
// 			{
// 				if (convert3D.data[i * ImgInspArea.step + j] <= 0)
// 					ImgInspArea.data[i * ImgInspArea.step + j] = 0;
// 			}
// 		}

		////3D 기울기 이미지
		/////////////////////////////////////////////////
		//int masksize = 3;
		//cv::Mat Sdx, Sdy, SobelDst;
		//cv::Sobel(ImgInspArea3D, Sdx, CV_16S, 1, 0, masksize);
		//cv::Sobel(ImgInspArea3D, Sdy, CV_16S, 0, 1, masksize);
		//SobelDst = Sdx + Sdy;

		//cv::vector<short> aa;
		//int testval = 40;
		//cv::Mat Map3D(SobelDst.rows, SobelDst.cols, CV_8UC1, cv::Scalar(0));	//기울기가 평평한 곳을 0으로 출력한 이미지
		//size_t shortStep = SobelDst.step / sizeof(short);

		//cv::Scalar mean, std;
		//cv::meanStdDev(SobelDst, mean, std);

		//for (int i = 0; i < SobelDst.rows - 1; i++)
		//{
		//	short* rowptr = (short*)SobelDst.data;
		//	for (int j = 0; j < SobelDst.cols - 1; j++)
		//	{
		//		short val = rowptr[i * shortStep + j];
		//		if (val > -(std[0]) && val < std[0])
		//		{
		//			aa.emplace_back(val);
		//			Map3D.data[i * Map3D.cols + j] = 255;
		//		}
		//	}
		//}


		for (int i = 0; i < ImgInspArea3DGradi.rows; i++)
		{
			uchar* rowptr = (uchar*)ImgInspArea3DGradi.data;
			for (int j = 0; j < ImgInspArea3DGradi.cols; j++)
			{
				int val = rowptr[i * ImgInspArea3DGradi.step + j];

				if (val > 200)
				{
					ImgInspArea3DGradi.data[i * ImgInspArea3DGradi.step + j] = 0;
				}
			}
		}

		/////////////////////////////////////////////////
		//UCHAR* ucFootAutoSearchArea = SumModel.data;	//ImgInspArea.data;
		cv::imwrite("D:\\InspWindowUp.bmp", convert3D);

		//cvtColor(ImgInspArea3DGradi, ImgInspArea3DGradi, COLOR_GRAY2BGR);
		//UCHAR* ucFootAutoSearchArea = ImgInspArea3DGradi.data;	//3D 바닥 평활화
		*/	//과거에 테스트했던 알고리즘 주석처리

		UCHAR* ucFootAutoSearchArea = ImgInspArea3DGradi.data;
		m_pProcMilAlgo->SaveWorkImg(ucFootAutoSearchArea, nSearchAreaWidth, nSearchAreaHeight, _T("Foot_AutoSearch_Area.bmp"));

		if (searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
			return ret;

		for (int i = 0; i < nCntPatternPath; ++i)		//Repeat pattern model cnt
		{
			double* GapLnC = m_pInspParam->GapLnC[i];
			double* GapLnR = m_pInspParam->GapLnR[i];
			double* DiviScore = m_pInspParam->DiviScore[i];
			BOOL DetailSearch = m_pInspParam->DetailSearch[i];

			//Pattern Model Image Load
			strFullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, _T("Foot_ModelImg.bmp"));
			m_procPatternMil->ModelFileLoad(strFullPath);

			dAngleRotate = 0.0;
			angleTemp = dAngleRotate + wndAngle;

			m_procPatternMil->ModelPreproc(angleTemp, 0);	//m_pAlgoPattern->m_SamplingAngle = 0;		//SamplingAngle 0으로 임시 처리
			m_procPatternMil->SetnChannel(1);

			//m_pAlgoPattern->GetParam(m_pTeachParam);

			//Set m_fovImage_insp, m_fovImage_insp_color   ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//
			int widthStep = nSearchAreaWidth * 1;// width * sWndAlgoImg.m_nChannel;
			cv::Mat src(nSearchAreaHeight, nSearchAreaWidth, CV_MAKETYPE(CV_8U, 1)/*CV_8UC3*/, ucFootAutoSearchArea, widthStep);
			cv::Mat m_Mat_fovImage_teach;
			m_Mat_fovImage_teach = src;

			Im::PIL_ID milSrcInspAreaImg = M_NULL;
			milSrcInspAreaImg = m_procPatternMil->AllocBuff(nSearchAreaWidth, nSearchAreaHeight);;
			Im::Buf::Put(milSrcInspAreaImg, ucFootAutoSearchArea);

			m_fovImage_insp = (void*)milSrcInspAreaImg;		//Insp Area

			Im::PIL_ID milSrc_color = m_procPatternMil->AllocBuffColor(src.cols, src.rows);
			int64 M_width = Im::Buf::Inquire(milSrc_color, M_SIZE_X, M_NULL);
			int64 M_height = Im::Buf::Inquire(milSrc_color, M_SIZE_Y, M_NULL);

			//alloc model 모델 생성 및 모델 트레이닝ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//
			int nSX = 0;
			int nSY = 0;
			int w = 0;
			int h = 0;
			double rotateAngle = 0;
			UCHAR* clipModelImg = ucArrModelImg;

			int allocRst = m_procPatternMil->AllocPatModel(clipModelImg, nModelWidth, nModelHeight, rotateAngle, w, h);
			m_procPatternMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
			m_procPatternMil->SetSearchStartAngle(0);
			m_procPatternMil->SetFindSubPixel(FALSE, FALSE);
			m_procPatternMil->PreprocModel(m_footImg_teachModel, FALSE, 1, 1);		//third param : use_algo(1:default)
			//typedef enum { agOriginal=0, agDefault=1, agLowContrast, agIntaglio, agRelief, agImage, agValue, agImageMin }enmMatchAlgo;
			//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//

			//UCHAR* clipSearchImg = new UCHAR[searchArea];
			UCHAR* clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);


			clipSearchImg = ucFootAutoSearchArea;

			int searchCnt = 0;
			double score = 0;
			double angle = 0;
			double posX = 0;
			double posY = 0;

			int nRepeatCnt = 10;
			int ndetectedFootCnt = 0;
			UCHAR* MaskImg;

			for (int nRepeatIdx = 0; nRepeatIdx < nRepeatCnt; nRepeatIdx++)
			{
				if (allocRst == eMPAT_SUCCESS)
				{
					Img_Proc = std::make_shared<ImgProcessing>();

					// Remove pre-process and Set input Image(dx, dy)
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdx = ImgInspAreaDx_8bit;
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdy = ImgInspAreaDy_8bit;

// 					cv::Mat ImgInspAreaDx_8bit_half, ImgInspAreaDy_8bit_half;
// 					cv::resize(ImgInspAreaDx_8bit, ImgInspAreaDx_8bit_half, cv::Size(ImgInspAreaDx_8bit.cols / 2, ImgInspAreaDx_8bit.rows / 2), 0, 0, CV_INTER_NN);
// 					cv::resize(ImgInspAreaDy_8bit, ImgInspAreaDy_8bit_half, cv::Size(ImgInspAreaDy_8bit.cols / 2, ImgInspAreaDy_8bit.rows / 2), 0, 0, CV_INTER_NN);
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spHalf].Sdx = ImgInspAreaDx_8bit_half;
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spHalf].Sdy = ImgInspAreaDy_8bit_half;

					m_procPatternMil->ImgProcess(clipSearchImg, searchWidth, searchHeight, Img_Proc);
					searchCnt = m_procPatternMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);	//pattern matching

					if (searchCnt > 0)
					{
						m_procPatternMil->GetPatResult(&score, &angle, &posX, &posY);	//get pattern match result

						double dFootCenterX, dFootCenterY;


						if (score > 30/*40*/)
						{
							ndetectedFootCnt++;

							dFootCenterX = posX + ROI.left;
							dFootCenterY = posY + ROI.top;

							//Draw Center Point On Part Image
							FootRect footrect;
							memset(&footrect, 0, sizeof(FootRect));
							footrect.m_Point.x = dFootCenterX;
							footrect.m_Point.y = dFootCenterY;

							//Foot Rect
							footrect.m_FootRect.left = dFootCenterX - 20;
							footrect.m_FootRect.top = dFootCenterY - 20;
							footrect.m_FootRect.right = footrect.m_FootRect.left + 40;
							footrect.m_FootRect.bottom = footrect.m_FootRect.top + 40;

							rst.emplace_back(footrect);		//Set result 

							//Except matching area (InspArea - ExceptMatchedArea)
							MaskImg = clipSearchImg;
							CRect rtExceptArea;

							int nExceptLeft = posX - (nModelWidth / 2);
							int nExceptRight = posX + (nModelWidth / 2);
							int nExceptTop = posY - (nModelHeight / 2);
							int nExceptBottom = posY + (nModelHeight / 2);

							rtExceptArea.left = nExceptLeft < 0 ? 0 : nExceptLeft;
							rtExceptArea.right = nExceptRight > searchWidth ? searchWidth : nExceptRight;
							rtExceptArea.top = nExceptTop < 0 ? 0 : nExceptTop;
							rtExceptArea.bottom = nExceptBottom > searchHeight ? searchHeight : nExceptBottom;

							for (int i = rtExceptArea.top; i < rtExceptArea.bottom; i++)
							{
								for (int j = rtExceptArea.left; j < rtExceptArea.right; j++)
								{
									MaskImg[i*searchWidth + j] = 0;
								}
							}

							cv::Mat maskImage(searchHeight, searchWidth, CV_8UC1, MaskImg);

							clipSearchImg = MaskImg;

#if _DEBUG
							CString cstrIndex;
							cstrIndex.Format(_T("ExceptFootArea_%d"), ndetectedFootCnt);

							CString path = g_pMPTI->GetWireDebugDataFullPath(1, _T("FootPatternMatch"), cstrIndex, 0);
							if (MaskImg && searchWidth > 0 && searchHeight > 0)
							{
								m_pProcMilAlgo->SaveDebugImg(MaskImg, searchWidth, searchHeight, path, 1, FALSE);

							}
#endif
						}
						else
						{
							break;
						}
					}
				}

				score = 0;

			}
		}

	}
	else
	{
		nFootDir = 1;

		//Pattern Matching Insp Area 
		searchWidth = ROI.right - ROI.left;
		searchHeight = ROI.bottom - ROI.top;

		nSearchAreaWidth = ROI.right - ROI.left;
		nSearchAreaHeight = ROI.bottom - ROI.top;

		searchArea = searchWidth * searchHeight;

		cv::Rect cvImgRectInspArea;
		cvImgRectInspArea.x = ROI.left;
		cvImgRectInspArea.y = ROI.top;
		cvImgRectInspArea.width = ROI.right - ROI.left;
		cvImgRectInspArea.height = ROI.bottom - ROI.top;

		//2D Insp Area
		cv::Mat ImgInspArea(nSearchAreaHeight, nSearchAreaWidth, CV_8UC1);
		pModel->GetImageMatrix(&ImgInspArea, pFoot, 1, cvImgRectInspArea);

		//Clip Insp Area From Part 3D
		cv::Rect inspAreaRoi(cvImgRectInspArea.x, cvImgRectInspArea.y, cvImgRectInspArea.width, cvImgRectInspArea.height);

		cv::Mat ImgInspArea3DClone = ImgPart3D(inspAreaRoi).clone();
		cv::Mat ImgInspArea3DClone_convert8;
		ImgInspArea3DClone.convertTo(ImgInspArea3DClone_convert8, CV_8UC1);		//8UC1

		cv::Mat ImgInspArea3DClone_2 = ImgPart3D(inspAreaRoi).clone();		//32FC1

		cv::Mat ImgInspArea3DGradi = part3DGradSumXY(inspAreaRoi).clone();		//3D 기울기로 매칭

		cv::Mat ImgInspAreaDx = part3DGradX(inspAreaRoi).clone();
		cv::Mat ImgInspAreaDx_8bit;
		ImgInspAreaDx.convertTo(ImgInspAreaDx_8bit, CV_8UC1);

		cv::Mat ImgInspAreaDy = part3DGradY(inspAreaRoi).clone();
		cv::Mat ImgInspAreaDy_8bit;
		ImgInspAreaDy.convertTo(ImgInspAreaDy_8bit, CV_8UC1);

		//CString sSrc_image_path;
		//sSrc_image_path.Format(_T("d:\\testimage\\workimage\\inputSdx_Down.bmp"));
		//cv::imwrite(std::string(CT2A(sSrc_image_path)), ImgInspAreaDx_8bit);

		//sSrc_image_path.Format(_T("d:\\testimage\\workimage\\inputSdy_Down.bmp"));
		//cv::imwrite(std::string(CT2A(sSrc_image_path)), ImgInspAreaDy_8bit);

		/*	//과거에 테스트했던 알고리즘 주석처리
		//3D 바닥 평활화
		cv::Mat convert3D;
		//ImgInspArea3DClone -= sum;

		//Subtract the Avg height value of the PAD area (obtained from the Foot Model)
		int nCntNegative = 0, nSumNegative = 0;
		bool bNegativeFlag = false;
		size_t floatstep = ImgInspArea3DClone.step / sizeof(float);
		for (int i = 0; i < ImgInspArea3DClone.rows; i++)
		{
			float* rowptr = (float*)ImgInspArea3DClone.data;
			for (int j = 0; j < ImgInspArea3DClone.cols; j++)
			{
				int val = rowptr[i * floatstep + j];

				//calc Avg Negative area
				if (val >= 0 && (val - sum) < 0)
				{
					bNegativeFlag = true;
					nCntNegative++;
					nSumNegative += (val - sum);

				}
				rowptr[i * floatstep + j] -= sum;
			}
		}

		if (bNegativeFlag)		//If it has a negative value, the subtraction operation is performed once more.
		{
			nSumNegative /= nCntNegative;
			ImgInspArea3DClone -= nSumNegative;
		}

		ImgInspArea3DClone.convertTo(convert3D, CV_8UC1);

		size_t floatstep2 = ImgInspArea3DClone_2.step / sizeof(float);
		for (int i = 0; i < ImgInspArea3DClone_2.rows; i++)
		{
			float* rowptr = (float*)ImgInspArea3DClone_2.data;
			for (int j = 0; j < ImgInspArea3DClone_2.cols; j++)
			{
				int val = rowptr[i * floatstep2 + j];

				if (val > (sum + nFoot3DmaxHeight))
				{
					convert3D.data[i * convert3D.step + j] = 0;
				}
				if (val < 0)
				{
					convert3D.data[i * convert3D.step + j] = 0;
				}
			}
		}


		// 		cv::Mat CannyInspAreaSrc1, CannyInspAreaDst1;
		//
		// 		convert3D.convertTo(CannyInspAreaSrc1, CV_8UC1);
		// 		cv::Canny(CannyInspAreaSrc1, CannyInspAreaDst1, 50, 70, 3, false);

				//////////////////////////////////// 3D기울기 기반
		cv::Mat SxModel, SyModel, SumModel;
		cv::Sobel(convert3D, SxModel, CV_16S, 1, 0, 3);
		cv::Sobel(convert3D, SyModel, CV_16S, 0, 1, 3);
		SumModel = SxModel + SyModel;

		SumModel = cv::abs(SumModel);
		SumModel.convertTo(SumModel, CV_8U);
		///////////////////////////////////// 3D기울기 기반


// 		for (int i = 0; i < ImgInspArea.rows; i++)
// 		{
// 			for (int j = 0; j < ImgInspArea.cols; j++)
// 			{
// 				if (convert3D.data[i * ImgInspArea.step + j] <= 0)
// 					ImgInspArea.data[i * ImgInspArea.step + j] = 0;
// 			}
// 		}

		////3D 기울기 이미지
		/////////////////////////////////////////////////
		//int masksize = 3;
		//cv::Mat Sdx, Sdy, SobelDst;
		//cv::Sobel(ImgInspArea3D, Sdx, CV_16S, 1, 0, masksize);
		//cv::Sobel(ImgInspArea3D, Sdy, CV_16S, 0, 1, masksize);
		//SobelDst = Sdx + Sdy;

		//cv::vector<short> aa;
		//int testval = 40;
		//cv::Mat Map3D(SobelDst.rows, SobelDst.cols, CV_8UC1, cv::Scalar(0));	//기울기가 평평한 곳을 0으로 출력한 이미지
		//size_t shortStep = SobelDst.step / sizeof(short);

		//cv::Scalar mean, std;
		//cv::meanStdDev(SobelDst, mean, std);

		//for (int i = 0; i < SobelDst.rows - 1; i++)
		//{
		//	short* rowptr = (short*)SobelDst.data;
		//	for (int j = 0; j < SobelDst.cols - 1; j++)
		//	{
		//		short val = rowptr[i * shortStep + j];
		//		if (val > -(std[0]) && val < std[0])
		//		{
		//			aa.emplace_back(val);
		//			Map3D.data[i * Map3D.cols + j] = 255;
		//		}
		//	}
		//}


		for (int i = 0; i < ImgInspArea3DGradi.rows; i++)
		{
			uchar* rowptr = (uchar*)ImgInspArea3DGradi.data;
			for (int j = 0; j < ImgInspArea3DGradi.cols; j++)
			{
				int val = rowptr[i * ImgInspArea3DGradi.step + j];

				if (val > 200)
				{
					ImgInspArea3DGradi.data[i * ImgInspArea3DGradi.step + j] = 0;
				}
			}
		}

		//UCHAR* ucFootAutoSearchArea = ImgInspArea.data;
		//UCHAR* ucFootAutoSearchArea = SumModel.data; //ImgInspArea.data
		cv::imwrite("D:\\InspWindowDown.bmp", convert3D);

		*/ //과거에 테스트했던 알고리즘 주석처리

		UCHAR* ucFootAutoSearchArea = ImgInspArea3DGradi.data;	//3D 바닥 평활화
		m_pProcMilAlgo->SaveWorkImg(ucFootAutoSearchArea, nSearchAreaWidth, nSearchAreaHeight, _T("Foot_AutoSearch_Area.bmp"));

		if (searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
			return ret;

		for (int i = 0; i < nCntPatternPath; ++i)		//Repeat pattern model cnt
		{
			double* GapLnC = m_pInspParam->GapLnC[i];
			double* GapLnR = m_pInspParam->GapLnR[i];
			double* DiviScore = m_pInspParam->DiviScore[i];
			BOOL DetailSearch = m_pInspParam->DetailSearch[i];

			//Pattern Model Image Load
			strFullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, _T("Foot_ModelImg.bmp"));
			m_procPatternMil->ModelFileLoad(strFullPath);

			dAngleRotate = 0.0;
			angleTemp = dAngleRotate + wndAngle;

			m_procPatternMil->ModelPreproc(angleTemp, 0);	//m_pAlgoPattern->m_SamplingAngle = 0;		//SamplingAngle 0으로 임시 처리
			m_procPatternMil->SetnChannel(1);

			//m_pAlgoPattern->GetParam(m_pTeachParam);

			//Set m_fovImage_insp, m_fovImage_insp_color   ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//
			int widthStep = nSearchAreaWidth * 1;// width * sWndAlgoImg.m_nChannel;
			cv::Mat src(nSearchAreaHeight, nSearchAreaWidth, CV_MAKETYPE(CV_8U, 1)/*CV_8UC3*/, ucFootAutoSearchArea, widthStep);
			cv::Mat m_Mat_fovImage_teach;
			m_Mat_fovImage_teach = src;

			Im::PIL_ID milSrcInspAreaImg = M_NULL;
			milSrcInspAreaImg = m_procPatternMil->AllocBuff(nSearchAreaWidth, nSearchAreaHeight);;
			Im::Buf::Put(milSrcInspAreaImg, ucFootAutoSearchArea);

			m_fovImage_insp = (void*)milSrcInspAreaImg;		//Insp Area

			Im::PIL_ID milSrc_color = m_procPatternMil->AllocBuffColor(src.cols, src.rows);
			int64 M_width = Im::Buf::Inquire(milSrc_color, M_SIZE_X, M_NULL);
			int64 M_height = Im::Buf::Inquire(milSrc_color, M_SIZE_Y, M_NULL);

			//alloc model 모델 생성 및 모델 트레이닝ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//
			int nSX = 0;
			int nSY = 0;
			int w = 0;
			int h = 0;
			double rotateAngle = 0;
			UCHAR* clipModelImg = ucArrModelImg;

			int allocRst = m_procPatternMil->AllocPatModel(clipModelImg, nModelWidth, nModelHeight, rotateAngle, w, h);
			m_procPatternMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
			m_procPatternMil->SetSearchStartAngle(0);
			m_procPatternMil->SetFindSubPixel(FALSE, FALSE);
			m_procPatternMil->PreprocModel(m_footImg_teachModel, FALSE, 1, 1);		//third param : use_algo(1:default)
			//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//

			//UCHAR* clipSearchImg = new UCHAR[searchArea];
			UCHAR* clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);

			clipSearchImg = ucFootAutoSearchArea;

			int searchCnt = 0;
			double score = 0;
			double angle = 0;
			double posX = 0;
			double posY = 0;

			int nRepeatCnt = 10;
			int ndetectedFootCnt = 0;
			UCHAR* MaskImg;

			for (int nRepeatIdx = 0; nRepeatIdx < nRepeatCnt; nRepeatIdx++)
			{
				if (allocRst == eMPAT_SUCCESS)
				{
					Img_Proc = std::make_shared<ImgProcessing>();

					// Remove pre-process and Set input Image(dx, dy)
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdx = ImgInspAreaDx_8bit;
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdy = ImgInspAreaDy_8bit;
// 
// 					cv::Mat ImgInspAreaDx_8bit_half, ImgInspAreaDy_8bit_half;
// 					cv::resize(ImgInspAreaDx_8bit, ImgInspAreaDx_8bit_half, cv::Size(ImgInspAreaDx_8bit.cols / 2, ImgInspAreaDx_8bit.rows / 2), 0, 0, CV_INTER_NN);
// 					cv::resize(ImgInspAreaDy_8bit, ImgInspAreaDy_8bit_half, cv::Size(ImgInspAreaDy_8bit.cols / 2, ImgInspAreaDy_8bit.rows / 2), 0, 0, CV_INTER_NN);
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spHalf].Sdx = ImgInspAreaDx_8bit_half;
// 					Img_Proc->_ProcBuf[GeoModel::enmDepth::spHalf].Sdy = ImgInspAreaDy_8bit_half;

					m_procPatternMil->ImgProcess(clipSearchImg, searchWidth, searchHeight, Img_Proc);
					searchCnt = m_procPatternMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);	//pattern matching

					if (searchCnt > 0)
					{
						m_procPatternMil->GetPatResult(&score, &angle, &posX, &posY);	//get pattern match result

						double dFootCenterX, dFootCenterY;

						if (score > 30/*40*/)
						{
							ndetectedFootCnt++;

							dFootCenterX = posX + ROI.left;
							dFootCenterY = posY + ROI.top;

							//Draw Center Point On Part Image
							FootRect footrect;
							memset(&footrect, 0, sizeof(FootRect));
							footrect.m_Point.x = dFootCenterX;
							footrect.m_Point.y = dFootCenterY;

							//Foot Rect
							footrect.m_FootRect.left = dFootCenterX - 20;
							footrect.m_FootRect.top = dFootCenterY - 20;
							footrect.m_FootRect.right = footrect.m_FootRect.left + 40;
							footrect.m_FootRect.bottom = footrect.m_FootRect.top + 40;

							rst.emplace_back(footrect);		//Set result 

							//Except matching area (InspArea - ExceptMatchedArea)
							MaskImg = clipSearchImg;
							CRect rtExceptArea;

							int nExceptLeft = posX - (nModelWidth / 2);
							int nExceptRight = posX + (nModelWidth / 2);
							int nExceptTop = posY - (nModelHeight / 2);
							int nExceptBottom = posY + (nModelHeight / 2);

							rtExceptArea.left = nExceptLeft < 0 ? 0 : nExceptLeft;
							rtExceptArea.right = nExceptRight > searchWidth ? searchWidth : nExceptRight;
							rtExceptArea.top = nExceptTop < 0 ? 0 : nExceptTop;
							rtExceptArea.bottom = nExceptBottom > searchHeight ? searchHeight : nExceptBottom;

							for (int i = rtExceptArea.top; i < rtExceptArea.bottom; i++)
							{
								for (int j = rtExceptArea.left; j < rtExceptArea.right; j++)
								{
									MaskImg[i*searchWidth + j] = 0;
								}
							}
							cv::Mat maskImage(searchHeight, searchWidth, CV_8UC1, MaskImg);

#if _DEBUG
							CString cstrIndex;
							cstrIndex.Format(_T("ExceptFootArea_%d"), ndetectedFootCnt);
							CString path = g_pMPTI->GetWireDebugDataFullPath(1, _T("FootPatternMatch"), cstrIndex, 0);
							if (MaskImg && searchWidth > 0 && searchHeight > 0)
								m_pProcMilAlgo->SaveDebugImg(MaskImg, searchWidth, searchHeight, path, 1, FALSE);
#endif
						}
						else
							break;
					}
				}

				score = 0;

			}
		}

	}


	/*
	//극성검사 (모델 90도 회전해서 패턴 매칭)

	double dPatSubScore[2][CNT_PATTERN_SCORE] = { {0,}, {0,} };
	if (nCntSearch > 0)
	{
		m_procPatternMil->GetPatResult(&score[0], &angle[0], &posX[0], &posY[0]);
		m_procPatternMil->GetPatSubScore(dPatSubScore[0], CNT_PATTERN_DIVISION_C + 1);
	}
	int nSearchCnt(1);
	//		if(score[0] < 85)	// YJS 2017/02/20 70%=>85%·Î º¯°æ
	{
		nSearchCnt = 2;
		//2차: 1차에서 실패하거나, score 70% 미만이면 180도 에서 다시 검사.
		dAngleRotate = 180.0;
		angleTemp = dAngleRotate + wndAngle;
		if (angleTemp >= 360)
			angleTemp = angleTemp - 360;

		m_procPatternMil->ModelPreproc(angleTemp, m_pAlgoPattern->m_SamplingAngle);

		int nCntSearch2 = m_procPatternMil->SearchPattern(m_fovImage_insp, m_fovImage_insp_color, searchWidth / 2, searchHeight / 2, searchWidth, searchHeight);
		if (nCntSearch == 0)
			nCntSearch = nCntSearch2;

		m_procPatternMil->GetPatResult(&score[1], &angle[1], &posX[1], &posY[1]);

		double maxscore = score[0] > score[1] ? score[0] : score[1];
		CString savepath;
		m_procPatternMil->GetPatSubScore(dPatSubScore[1], CNT_PATTERN_DIVISION_C + 1);
	}

	//원래 패턴 < 90도 회전한 패턴모델
	if (score[0] < score[1])
		nSelIndex = 1;

	double dCogX_pixel = ((searchCx - (searchWidth / 2.0)) + posX[nSelIndex]);
	double dCogY_pixel = ((searchCy - (searchHeight / 2.0)) + posY[nSelIndex]);
	/////
	double dCenTeachX = stdCogX[i];
	double dCenTeachY = stdCogY[i];

	if ((wndAngle == 90) || (wndAngle == 180) || (wndAngle == 270))
	{
		m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, wndAngle,
			0, 0, &dCenTeachX, &dCenTeachY);
	}

	//double dshiftX = (dCogX_pixel * m_resolX) - ((m_pInspPartInfo->m_dPartWidth / 2) * m_resolX);
	//double dshiftY = ((m_pInspPartInfo->m_dPartHeight / 2) * m_resolY) - (dCogY_pixel * m_resolY);

	//double cogX_board = 0.0;
	//double cogY_board = 0.0;
	//CvtPixelToBoard(m_inspCoordinate.fovCx, m_inspCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);
	//

	//double offsetX = stdCogX - cogX_board;
	//double offsetY = stdCogY - cogY_board;

// 			if(wndAngle == 90 || wndAngle == 270)
// 			{
// 				offsetX = (dCenTeachY - dshiftY) * -1;
// 				offsetY = dCenTeachX - dshiftX;
// 			}
// 			else
// 			{
// 				offsetX = dCenTeachX - dshiftX;
// 				offsetY = (dCenTeachY - dshiftY) * -1;
// 			}

			m_rstInspPattern.m_nModelNum = i + 1;// YJS 2017/03/11
			m_rstInspPattern.m_ptModelPos.x = posX[nSelIndex];
			m_rstInspPattern.m_ptModelPos.y = posY[nSelIndex];
			memcpy(m_rstInspPattern.m_dDiviScore, dPatSubScore[nSelIndex], sizeof(double)*CNT_PATTERN_SCORE);
			m_rstInspPattern.score = score[nSelIndex];
			dTempAngle = (angle[nSelIndex] - wndAngle);

			if (dTempAngle <= -360)
			{
				while (dTempAngle <= -360)	dTempAngle += 360;
			}
			else if (dTempAngle >= 360)
			{
				while (dTempAngle >= 360)	dTempAngle -= 360;
			}

			m_rstInspPattern.angle = dTempAngle;
			m_rstInspPattern.offsetX = offsetX;		/// 140207
			m_rstInspPattern.offsetY = offsetY;		/// 140207

			//영역 4분할 및 각 영역의 score 계산 -> 제일 낮은 값 검출
			if (m_procPatternMil->IsDivInspAvailable())
			{
				bool bDecisionList[2] = { false, };
				int nDivNumList[2] = { 2, };
				double dResultScoreList[2] = { 0, };

				for (int j = 0; j < nSearchCnt; j++)
				{
					if (m_pInspParam->DiviLnC[i] > 0 || m_pInspParam->DiviLnR[i] > 0)
					{
						int nDivNum(-1), nDivNgNum(-1);
						double DivScore(HUGE_VAL), DivNGScore(HUGE_VAL);

						int DiviArC = m_pInspParam->DiviLnC[i] + 1;
						int DiviArR = m_pInspParam->DiviLnR[i] + 1;
						int step = CNT_PATTERN_DIVISION_C + 1;
						// 					for (int y=0; y<DiviArR; y++)
						{
							for (int x = 0; x < DiviArC; x++)
							{
								if (DivScore > dPatSubScore[j][x])
								{
									nDivNum = x;
									DivScore = dPatSubScore[j][x];
								}
								if (dPatSubScore[j][x] < m_pInspParam->DiviScore[i][x])
								{
									if (DivNGScore > dPatSubScore[j][x])
									{
										nDivNgNum = x;
										DivNGScore = dPatSubScore[j][x];
									}
								}
							}
							for (int x = 0; x < DiviArR; x++)
							{
								if (DivScore > dPatSubScore[j][x])
								{
									nDivNum = x;
									DivScore = dPatSubScore[j][x];
								}
								if (dPatSubScore[j][/x] < m_pInspParam->DiviScore[i][x])
								{
									if (DivNGScore > dPatSubScore[j][x])
									{
										nDivNgNum = x;
										DivNGScore = dPatSubScore[j][x];
									}
								}
							}
						}

						if (DivNGScore != HUGE_VAL)
						{
							bDecisionList[j] = false;
							nDivNumList[j] = nDivNgNum + 1;
							dResultScoreList[j] = DivNGScore;
						}
						else if (DivScore != HUGE_VAL)
						{
							bDecisionList[j] = true;
							nDivNumList[j] = nDivNum + 1;
							dResultScoreList[j] = DivScore;
						}
						else
						{
							bDecisionList[j] = true;
							nDivNumList[j] = 0;
							dResultScoreList[j] = 100.0;
						}
					}
					else
					{
						if (score[j] < m_pInspParam->stdScore)
						{
							bDecisionList[j] = false;
						}
						else
							bDecisionList[j] = true;
						dResultScoreList[j] = score[j];
						nDivNumList[j] = 0;
					}
				}


				int nSelIdx(0);
				if (nSearchCnt >= 2 && dResultScoreList[0] < dResultScoreList[1])
				{
					nSelIdx = 1;
					if (m_pInspParam->isPolarity)
					{
						bDecisionList[nSelIdx] = false;
					}
				}

				memcpy(m_rstInspPattern.m_dDiviScore, dPatSubScore[nSelIdx], sizeof(double)*CNT_PATTERN_SCORE);
				bDecision = bDecisionList[nSelIdx];
				m_rstInspPattern.score = dResultScoreList[nSelIdx];
				m_rstInspPattern.m_nDivisionNum = nDivNumList[nSelIdx];

				dTempAngle = (angle[nSelIdx] - wndAngle);

				if (dTempAngle <= -360)
				{
					while (dTempAngle <= -360)	dTempAngle += 360;
				}
				else if (dTempAngle >= 360)
				{
					while (dTempAngle >= 360)	dTempAngle -= 360;
				}

				nSelIndex = nSelIdx;	// YJS 2017/06/09

				m_rstInspPattern.angle = dTempAngle;

			}
			else	//shkim 2017.07.26
			{
				if (m_pInspParam->stdScore > m_rstInspPattern.score)
					bDecision = false;
				else
					bDecision = true;
			}




// 			if (nCntSearch >= 1)
// 			{
// 				bFound = TRUE;
//
// 				if (bDecision == false)
// 				{
// 					continue;
// 				}
// 				else {
// 					//offset NG
// 					if (PATTERN_LOCK && m_pInspParam->bUseOffset && (abs(m_rstInspPattern.offsetX) > m_pInspParam->offsetRange_x || abs(m_rstInspPattern.offsetY) > m_pInspParam->offsetRange_y))
// 					{
// 						m_rstInspPattern.ok = FALSE;
// 						if (abs(m_rstInspPattern.offsetX) > m_pInspParam->offsetRange_x)
// 							m_rstInspPattern.list.offsetXOk = FALSE;
// 						if (abs(m_rstInspPattern.offsetX) > m_pInspParam->offsetRange_y)
// 							m_rstInspPattern.list.offsetYOk = FALSE;
//
// 						if (m_rstInspPattern.list.offsetXOk == FALSE || m_rstInspPattern.list.offsetYOk == FALSE)
// 							continue;
// 					}
// 					break;
// 				}
// 			}
// 			else
// 			{
// 				continue;
// 			}

*/

//}		//모델 로드 성공 시 타는 구문
//////////////////////////////////////////////////////////////////////////	

/*
if (bFound == TRUE)
{
	m_rstInspPattern.ok = TRUE;
	m_rstInspPattern.list.findOK = TRUE;

	if (nSelIndex == 1)
		m_rstInspPattern.isReverse = TRUE;
	else
		m_rstInspPattern.isReverse = FALSE;
}
else
{
	m_rstInspPattern.ok = FALSE;
	m_rstInspPattern.list.findOK = FALSE;
}


m_rstInspPattern.isInsp = TRUE;
*/

// 	ret = DecisionPatternInsp(bDecision);
// 
// 	if (m_pAlgoPattern->m_bUseNGOpt == TRUE)
// 	{
// 		if (ret == e_NG)
// 			ret = e_OK;
// 		else
// 			ret = e_NG;
// 	}

	//////////////////////////////////////////////////////////////////////////
	ret = true;

	return ret;
}

int CPInsp_Foot::SearchPattern2D(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst, int FootIndex)
{
	int ret = e_NG;
	bool nFootDir = FootIndex == 0;	//0-Up, 1-Down

	if (!m_pInspParam)
		return ret;

	double wndAngle = 0;
	if (IsAnyAngle(wndAngle))
		wndAngle = 0;

	if (wndAngle > 360)
		wndAngle -= 360;

	//Set Angle Limit - min/max Range
	double posLimit = 45;	//m_inspParam->searchAngleRange_Max;
	double negLimit = 45;	//m_inspParam->searchAngleRange_Min;

	double angleTemp = 0.;
	CString strFullPath = _T("");		//Get Foot Model for Pattern Match
	double dAngleRotate = 0.0;

	BOOL bUseAlgoPattern = TRUE;	//BOOL bUseAlgoPattern = (m_pAlgoPattern && (m_pAlgoPattern->m_nCntPatternPath > 0));

	//Init Pattern Matching Result 
	int nCntPatternPath = 0;
	m_ModelMng2D->GetModelCount(false);	//int nCntPatternPath = (bUseAlgoPattern) ? m_pAlgoPattern->m_nCntPatternPath : 1;
	RstInspPattern OrgResult;
	InitPatternResultStruct(&OrgResult);
	bool bDecision(true);
	m_rstInspPattern.score = 0;
	m_rstInspPattern.angle = 0;
	m_rstInspPattern.offsetX = 0;
	m_rstInspPattern.offsetY = 0;

	double dTempAngle = 0.;
	double offsetX = 0.;
	double offsetY = 0.;

	//Foot Model Imageㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//
	int nModelWidth = pFoot->GetImageWidth();
	int nModelHeight = pFoot->GetImageLength();
	int nModelArea = nModelWidth * nModelHeight;

	//2D Foot Model
	cv::Mat ImgFootModel(nModelHeight, nModelWidth, CV_8UC1, cv::Scalar(0));
	pFoot->GetImageMatrix(&ImgFootModel, 1);	//footkind -> m_eFootBin_Foot(1)
	POINTF pfFootCenterPoint = pFoot->m_FootCenterPoint;

	UCHAR* ptrFoot = ImgFootModel.data;
	UCHAR* ucArrModelImg = ptrFoot;
	m_pProcMilAlgo->SaveWorkImg(ucArrModelImg, nModelWidth, nModelHeight, _T("Foot_ModelImg.bmp"));

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//

	//Pattern Matching Insp Area (search area)
	int searchWidth, searchHeight, searchArea;
	searchWidth = ROI.right - ROI.left;
	searchHeight = ROI.bottom - ROI.top;
	searchArea = searchWidth * searchHeight;

	cv::Rect cvImgRectInspArea;
	cvImgRectInspArea.x = ROI.left;
	cvImgRectInspArea.y = ROI.top;
	cvImgRectInspArea.width = searchWidth;
	cvImgRectInspArea.height = searchHeight;

	if (searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
		return ret;

	//Get Insp Area _2D
	cv::Mat ImgInspAreaOrg(pModel->m_pPartImageBuffer->nImageSizeY, pModel->m_pPartImageBuffer->nImageSizeX, CV_8UC1, pModel->m_pPartImageBuffer->imgTop_W);
	cv::Mat ImgInspArea = ImgInspAreaOrg(cvImgRectInspArea).clone();
	//pModel->GetImageMatrix(&ImgInspArea, pFoot, 1, cvImgRectInspArea);

	//Get Insp Area _3D
	cv::Mat ImgInspArea3D(pModel->m_pPartImageBuffer->nImageSizeY, pModel->m_pPartImageBuffer->nImageSizeX, CV_32FC1, pModel->m_pPart3D);
	cv::Rect roi(cvImgRectInspArea.x, cvImgRectInspArea.y, cvImgRectInspArea.width, cvImgRectInspArea.height);
	cv::Mat ImgInspArea3DClone = ImgInspArea3D(roi).clone();

	cv::Mat convert3D;
	ImgInspArea3DClone.convertTo(convert3D, CV_8UC1);

	for (int i = 0; i < ImgInspArea.rows; i++)
	{
		for (int j = 0; j < ImgInspArea.cols; j++)
		{
			if (convert3D.data[i * ImgInspArea.step + j] <= 0)
				ImgInspArea.data[i * ImgInspArea.step + j] = 0;
		}
	}

	UCHAR* ucFootAutoSearchArea = NULL;
	ucFootAutoSearchArea = ImgInspArea.data;
	m_pProcMilAlgo->SaveWorkImg(ucFootAutoSearchArea, searchWidth, searchHeight, _T("Foot_AutoSearch_Area.bmp"));

	double score = 0, angle = 0, posX = 0, posY = 0;
	int nRepeatCnt = 10;
	int ndetectedFootCnt = 0;
	UCHAR* MaskImg;
	std::shared_ptr<GeoResult_Foot> PatResult = std::make_shared<GeoResult_Foot>();
	PatResult->Alloc(1);

	for (int nRepeatIdx = 0; nRepeatIdx < nRepeatCnt; nRepeatIdx++)
	{
		bool res = MatchModel(ImgInspArea, PatResult, &cvImgRectInspArea, nFootDir, pFoot->m_nFoottype);

		if (res)
		{
			score = PatResult->_Equality[0];
			angle = PatResult->_Angle[0];
			posX = PatResult->_Center_y[0];
			posY = PatResult->_Center_x[0];
			double dFootCenterX, dFootCenterY;

			if (score > m_nFootPatternMatchScore)
			{
				ndetectedFootCnt++;

				dFootCenterX = posX + ROI.left;
				dFootCenterY = posY + ROI.top;

				//Draw Center Point On Part Image
				FootRect footrect;
				memset(&footrect, 0, sizeof(FootRect));
				footrect.m_Point.x = dFootCenterX;
				footrect.m_Point.y = dFootCenterY;

				//Foot Center Rect
				footrect.m_FootRect.left = dFootCenterX - (cvImgRectInspArea.width / 2);
				footrect.m_FootRect.top = dFootCenterY - (cvImgRectInspArea.height / 2);
				footrect.m_FootRect.right = footrect.m_FootRect.left + cvImgRectInspArea.width;
				footrect.m_FootRect.bottom = footrect.m_FootRect.top + cvImgRectInspArea.height;

				//Get teached Foot ROI Size
				int footROIWidth = pFoot->mImageRect.right - pFoot->mImageRect.left;
				int footROIHeight = pFoot->mImageRect.bottom - pFoot->mImageRect.top;

				//Foot ROI
				footrect.m_PadRect.left = dFootCenterX - (footROIWidth / 2);
				footrect.m_PadRect.right = dFootCenterX + (footROIWidth / 2);
				footrect.m_PadRect.top = dFootCenterY - (footROIHeight / 2);
				footrect.m_PadRect.bottom = dFootCenterY + (footROIHeight / 2);

				rst.emplace_back(footrect);		//Set result 

				//Except matching area (InspArea - ExceptMatchedArea)
				MaskImg = ucFootAutoSearchArea;
				CRect rtExceptArea;

				int nExceptLeft = cvImgRectInspArea.x;
				int nExceptRight = cvImgRectInspArea.width + nExceptLeft;
				int nExceptTop = cvImgRectInspArea.y;
				int nExceptBottom = cvImgRectInspArea.height + nExceptTop;

				rtExceptArea.left = nExceptLeft < 0 ? 0 : nExceptLeft;
				rtExceptArea.right = nExceptRight > searchWidth ? searchWidth : nExceptRight;
				rtExceptArea.top = nExceptTop < 0 ? 0 : nExceptTop;
				rtExceptArea.bottom = nExceptBottom > searchHeight ? searchHeight : nExceptBottom;

				cv::Mat maskImage(searchHeight, searchWidth, CV_8UC1, MaskImg);

				ucFootAutoSearchArea = MaskImg;
				{
					cv::Point pts[1][4];
					int cx = PatResult->_Center_y[0];
					int cy = PatResult->_Center_x[0];

					double w = (rtExceptArea.right - rtExceptArea.left) / 2;
					double h = (rtExceptArea.bottom - rtExceptArea.top) / 2;

					double dLeng = std::sqrt(std::pow(w, 2) + std::pow(h, 2));
					double dRate = h / w;
					double dstdRadian = std::atan(dRate);
					double dstdDegree = dstdRadian / std::_Pi * 180;

					double dRadian1 = (-PatResult->_Angle[0] - (dstdDegree)) * std::_Pi / 180;
					double dRadian2 = (-PatResult->_Angle[0] - (180 - dstdDegree)) * std::_Pi / 180;
					double dRadian3 = (-PatResult->_Angle[0] - (180 + dstdDegree)) * std::_Pi / 180;
					double dRadian4 = (-PatResult->_Angle[0] - (360 - dstdDegree)) * std::_Pi / 180;
					double diffX1 = std::cos(dRadian1) * dLeng;
					double diffY1 = std::sin(dRadian1) * dLeng;
					double diffX2 = std::cos(dRadian2) * dLeng;
					double diffY2 = std::sin(dRadian2) * dLeng;
					double diffX3 = std::cos(dRadian3) * dLeng;
					double diffY3 = std::sin(dRadian3) * dLeng;
					double diffX4 = std::cos(dRadian4) * dLeng;
					double diffY4 = std::sin(dRadian4) * dLeng;

					int p1x = cx + (int)std::round(diffX1); int  p1y = cy + (int)std::round(diffY1);
					int p2x = cx + (int)std::round(diffX2); int  p2y = cy + (int)std::round(diffY2);
					int p3x = cx + (int)std::round(diffX3); int  p3y = cy + (int)std::round(diffY3);
					int p4x = cx + (int)std::round(diffX4); int  p4y = cy + (int)std::round(diffY4);
					pts[0][0] = cv::Point(p1x, p1y);
					pts[0][1] = cv::Point(p2x, p2y);
					pts[0][2] = cv::Point(p3x, p3y);
					pts[0][3] = cv::Point(p4x, p4y);
					int nps[] = { 4 };
					const cv::Point* ptr[1] = { pts[0] };
					cv::fillPoly(maskImage, ptr, nps, 1, cv::Scalar(0));

				}

#if _DEBUG
				CString cstrIndex;
				cstrIndex.Format(_T("ExceptFootArea_%d"), ndetectedFootCnt);
				CString path = g_pMPTI->GetWireDebugDataFullPath(1, _T("FootPatternMatch"), cstrIndex, 0);
				if (MaskImg && searchWidth > 0 && searchHeight > 0)
					m_pProcMilAlgo->SaveDebugImg(MaskImg, searchWidth, searchHeight, path, 1, FALSE);
#endif

			}
			else
			{
				break;
			}
		}
		else
			break;
		score = 0;

	}
	ret = true;

	return ret;
}

int CPInsp_Foot::SearchPattern3D(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst, int FootIndex)
{
	int ret = e_NG;
	int nFootDir = FootIndex == 0;	//0-Up, 1-Down

	if (!m_pInspParam)
		return ret;

	double wndAngle = 0;
	if (IsAnyAngle(wndAngle))
		wndAngle = 0;

	if (wndAngle > 360)
		wndAngle -= 360;

	//Set Angle Limit - min/max Range
	double posLimit = 45;	//m_inspParam->searchAngleRange_Max;
	double negLimit = 45;	//m_inspParam->searchAngleRange_Min;

	double angleTemp = 0.;
	CString strFullPath = _T("");		//Get Foot Model for Pattern Match
	double dAngleRotate = 0.0;

	BOOL bUseAlgoPattern = TRUE;	//BOOL bUseAlgoPattern = (m_pAlgoPattern && (m_pAlgoPattern->m_nCntPatternPath > 0));

	//Init Pattern Matching Result 
	int nCntPatternPath = m_ModelMng3D->GetModelCount(false);	//int nCntPatternPath = (bUseAlgoPattern) ? m_pAlgoPattern->m_nCntPatternPath : 1;
	RstInspPattern OrgResult;
	InitPatternResultStruct(&OrgResult);
	bool bDecision(true);
	m_rstInspPattern.score = 0;
	m_rstInspPattern.angle = 0;
	m_rstInspPattern.offsetX = 0;
	m_rstInspPattern.offsetY = 0;

	double dTempAngle = 0.;
	double offsetX = 0.;
	double offsetY = 0.;

	//Foot Model Imageㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//
	int nModelWidth = pFoot->GetImageWidth();
	int nModelHeight = pFoot->GetImageLength();
	int nModelArea = nModelWidth * nModelHeight;

	//3D Foot Model (Foot model ROI 3d)
	cv::Mat ImgFootModel_3D(nModelHeight, nModelWidth, CV_32FC1, pFoot->pf3D);

	//Get Part 3D  
	cv::Mat ImgPart3D(pModel->m_pPartImageBuffer->nImageSizeY, pModel->m_pPartImageBuffer->nImageSizeX, CV_32FC1, pModel->m_pPart3D);

	//Remove values ​​with Part 3D height of 0 or negative
	size_t floatstep_3DCorrect = ImgPart3D.step / sizeof(float);
	for (int i = 0; i < ImgPart3D.rows; i++)
	{
		float* rowptr = (float*)ImgPart3D.data;
		for (int j = 0; j < ImgPart3D.cols; j++)
		{
			int val = rowptr[i * floatstep_3DCorrect + j];
			if (val <= 0)
			{
				val = 0;
				rowptr[i * floatstep_3DCorrect + j] = 0;
			}
		}
	}

	//Clip Foot ROI(32FC1) from Part 3D  
	cv::Rect rtFootRoi(pFoot->mImageRect.left, pFoot->mImageRect.top, pFoot->mImageRect.right - pFoot->mImageRect.left, pFoot->mImageRect.bottom - pFoot->mImageRect.top);
	cv::Mat model3DRoi = ImgPart3D(rtFootRoi).clone();
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ//

	//Pattern Matching Insp Area (search area)
	int searchWidth, searchHeight, searchArea;
	searchWidth = ROI.right - ROI.left;
	searchHeight = ROI.bottom - ROI.top;
	searchArea = searchWidth * searchHeight;

	cv::Rect cvImgRectInspArea;
	cvImgRectInspArea.x = ROI.left;
	cvImgRectInspArea.y = ROI.top;
	cvImgRectInspArea.width = searchWidth;
	cvImgRectInspArea.height = searchHeight;

	if (searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
		return ret;
	//Get Insp Area _3D		- Clip Insp Area From Part 3D
	cv::Rect inspAreaRoi(cvImgRectInspArea.x, cvImgRectInspArea.y, cvImgRectInspArea.width, cvImgRectInspArea.height);

	cv::Mat ImgInspArea3DGradi = ImgPart3D(inspAreaRoi).clone();		//3D 기울기로 매칭


	UCHAR* ucFootAutoSearchArea = NULL;
	ucFootAutoSearchArea = ImgInspArea3DGradi.data;
	m_pProcMilAlgo->SaveWorkImg(ucFootAutoSearchArea, searchWidth, searchHeight, _T("Foot_AutoSearch_Area.bmp"));
	int searchCnt = 0;
	double score = 0, angle = 0, posX = 0, posY = 0;
	int nRepeatCnt = 10;
	int ndetectedFootCnt = 0;
	UCHAR* MaskImg;

	std::shared_ptr<GeoResult_Foot> PatResult = std::make_shared<GeoResult_Foot>();
	PatResult->Alloc(1);
	for (int nRepeatIdx = 0; nRepeatIdx < nRepeatCnt; nRepeatIdx++)
	{
		bool res = MatchModel3D(ImgInspArea3DGradi, PatResult, &cvImgRectInspArea, nFootDir, pFoot->m_nFoottype);

		if (res)
		{
			score = PatResult->_Equality[0];
			angle = PatResult->_Angle[0];
			posX = PatResult->_Center_y[0];
			posY = PatResult->_Center_x[0];

			double dFootCenterX, dFootCenterY;

			if (score > m_nFootPatternMatchScore)
			{
				ndetectedFootCnt++;

				dFootCenterX = posX + ROI.left;
				dFootCenterY = posY + ROI.top;

				//Draw Center Point On Part Image
				FootRect footrect;
				memset(&footrect, 0, sizeof(FootRect));
				footrect.m_Point.x = dFootCenterX;
				footrect.m_Point.y = dFootCenterY;

				//Foot Center Rect
				footrect.m_FootRect.left = dFootCenterX - (cvImgRectInspArea.width / 2);
				footrect.m_FootRect.top = dFootCenterY - (cvImgRectInspArea.height / 2);
				footrect.m_FootRect.right = footrect.m_FootRect.left + cvImgRectInspArea.width;
				footrect.m_FootRect.bottom = footrect.m_FootRect.top + cvImgRectInspArea.height;

				//Get teached Foot ROI Size
				int footROIWidth = pFoot->mImageRect.right - pFoot->mImageRect.left;
				int footROIHeight = pFoot->mImageRect.bottom - pFoot->mImageRect.top;

				//Foot ROI
				footrect.m_PadRect.left = dFootCenterX - (footROIWidth / 2);
				footrect.m_PadRect.right = dFootCenterX + (footROIWidth / 2);
				footrect.m_PadRect.top = dFootCenterY - (footROIHeight / 2);
				footrect.m_PadRect.bottom = dFootCenterY + (footROIHeight / 2);

				rst.emplace_back(footrect);		//Set result 

				//Except matching area (InspArea - ExceptMatchedArea)
				MaskImg = ucFootAutoSearchArea;
				CRect rtExceptArea;

				int nExceptLeft = cvImgRectInspArea.x;
				int nExceptRight = cvImgRectInspArea.width + nExceptLeft;
				int nExceptTop = cvImgRectInspArea.y;
				int nExceptBottom = cvImgRectInspArea.height + nExceptTop;

				rtExceptArea.left = nExceptLeft < 0 ? 0 : nExceptLeft;
				rtExceptArea.right = nExceptRight > searchWidth ? searchWidth : nExceptRight;
				rtExceptArea.top = nExceptTop < 0 ? 0 : nExceptTop;
				rtExceptArea.bottom = nExceptBottom > searchHeight ? searchHeight : nExceptBottom;

				cv::Mat maskImage(searchHeight, searchWidth, CV_32FC1, MaskImg);

				ucFootAutoSearchArea = MaskImg;
				{
					cv::Point pts[1][4];
					int cx = PatResult->_Center_y[0];
					int cy = PatResult->_Center_x[0];

					double w = (rtExceptArea.right - rtExceptArea.left) / 2;
					double h = (rtExceptArea.bottom - rtExceptArea.top) / 2;

					double dLeng = std::sqrt(std::pow(w, 2) + std::pow(h, 2));
					double dRate = h / w;
					double dstdRadian = std::atan(dRate);
					double dstdDegree = dstdRadian / std::_Pi * 180;

					double dRadian1 = (-PatResult->_Angle[0] - (dstdDegree)) * std::_Pi / 180;
					double dRadian2 = (-PatResult->_Angle[0] - (180 - dstdDegree)) * std::_Pi / 180;
					double dRadian3 = (-PatResult->_Angle[0] - (180 + dstdDegree)) * std::_Pi / 180;
					double dRadian4 = (-PatResult->_Angle[0] - (360 - dstdDegree)) * std::_Pi / 180;
					double diffX1 = std::cos(dRadian1) * dLeng;
					double diffY1 = std::sin(dRadian1) * dLeng;
					double diffX2 = std::cos(dRadian2) * dLeng;
					double diffY2 = std::sin(dRadian2) * dLeng;
					double diffX3 = std::cos(dRadian3) * dLeng;
					double diffY3 = std::sin(dRadian3) * dLeng;
					double diffX4 = std::cos(dRadian4) * dLeng;
					double diffY4 = std::sin(dRadian4) * dLeng;

					int p1x = cx + (int)std::round(diffX1); int  p1y = cy + (int)std::round(diffY1);
					int p2x = cx + (int)std::round(diffX2); int  p2y = cy + (int)std::round(diffY2);
					int p3x = cx + (int)std::round(diffX3); int  p3y = cy + (int)std::round(diffY3);
					int p4x = cx + (int)std::round(diffX4); int  p4y = cy + (int)std::round(diffY4);
					pts[0][0] = cv::Point(p1x, p1y);
					pts[0][1] = cv::Point(p2x, p2y);
					pts[0][2] = cv::Point(p3x, p3y);
					pts[0][3] = cv::Point(p4x, p4y);
					int nps[] = { 4 };
					const cv::Point* ptr[1] = { pts[0] };
					cv::fillPoly(maskImage, ptr, nps, 1, cv::Scalar(0));

				}

#if _DEBUG

				CString cstrIndex;
				cstrIndex.Format(_T("ExceptFootArea_%d"), ndetectedFootCnt);
				CString path = g_pMPTI->GetWireDebugDataFullPath(1, _T("FootPatternMatch"), cstrIndex, 0);
				if (MaskImg && searchWidth > 0 && searchHeight > 0)
					m_pProcMilAlgo->SaveDebugImg(MaskImg, searchWidth, searchHeight, path, 1, FALSE);
#endif

			}
			else
				break;
		}
		else
			break;

		score = 0;

	}
	ret = true;

	return ret;
}

//Calc only Pad BW Teach Area(2D)
//3D Foot Model img - Pad Area Height(AVG) >> Apply Pad Height Offset
int CPInsp_Foot::SearchPattern2D3D(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst, int FootIndex)
{
	int ret = e_NG;
	int nFootDir = 0;	//0-Up, 1-Down

	if (!m_pInspParam)
		return ret;

	double wndAngle = 0;
	if (IsAnyAngle(wndAngle))
		wndAngle = 0;

	if (wndAngle > 360)
		wndAngle -= 360;

	//Set Angle Limit - min/max Range
	double posLimit = 45;	//m_inspParam->searchAngleRange_Max;
	double negLimit = 45;	//m_inspParam->searchAngleRange_Min;

	double angleTemp = 0.;
	CString strFullPath = _T("");		//Get Foot Model for Pattern Match
	double dAngleRotate = 0.0;

	BOOL bUseAlgoPattern = TRUE;	//BOOL bUseAlgoPattern = (m_pAlgoPattern && (m_pAlgoPattern->m_nCntPatternPath > 0));

	//Init Pattern Matching Result 
	int nCntPatternPath = 1;	//int nCntPatternPath = (bUseAlgoPattern) ? m_pAlgoPattern->m_nCntPatternPath : 1;
	RstInspPattern OrgResult;
	InitPatternResultStruct(&OrgResult);
	bool bDecision(true);
	m_rstInspPattern.score = 0;
	m_rstInspPattern.angle = 0;
	m_rstInspPattern.offsetX = 0;
	m_rstInspPattern.offsetY = 0;

	double dTempAngle = 0.;
	double offsetX = 0.;
	double offsetY = 0.;

	//Foot Model Image
	int nModelWidth = pFoot->GetImageWidth();
	int nModelHeight = pFoot->GetImageLength();

	//2D Foot Model
	cv::Mat ImgFootModel(nModelHeight, nModelWidth, CV_8UC1, cv::Scalar(0));
	pFoot->GetImageMatrix(&ImgFootModel, 1);	//footkind -> m_eFootBin_Foot(1)

	//3D Foot Model
	cv::Mat ImgFootModel_3D(nModelHeight, nModelWidth, CV_32FC1, pFoot->pf3D);
	ImgFootModel_3D.convertTo(ImgFootModel_3D, CV_8U);

	//Foot, pad binary Image 
	int nImageHeight = pFoot->GetImageLength();
	int nImageWidth = pFoot->GetImageWidth();
	cv::Mat ImgPad(nImageHeight, nImageWidth, CV_8UC1);
	//pFoot->GetImageMatrix(&ImgPad, (int)m_eFootBin::m_eFootBin_Pad);
	//GetBinImage(pFoot, &ImgPad, m_eFootBin_Pad, NULL, false);
	pFoot->GetPadBinImage(&ImgPad);
	cv::Mat Imgfoot(nImageHeight, nImageWidth, CV_8UC1);
	pFoot->GetImageMatrix(&Imgfoot, (int)m_eFootBin::m_eFootBin_Foot);
	GetBinImage(pFoot, &Imgfoot, m_eFootBin_Foot, NULL, false);

	cv::Mat Imgfoot_inverse(nImageHeight, nImageWidth, CV_8UC1);
	cv::bitwise_not(Imgfoot, Imgfoot_inverse);
	cv::Mat ImgOnlyPadArea;
	cv::bitwise_and(ImgPad, Imgfoot_inverse, ImgOnlyPadArea);

	//3D
	cv::Rect rtRoi(pFoot->mImageRect.left, pFoot->mImageRect.top, pFoot->mImageRect.right - pFoot->mImageRect.left, pFoot->mImageRect.bottom - pFoot->mImageRect.top);
	cv::Mat ImgFootModel3DCorrect(pModel->m_pPartImageBuffer->nImageSizeY, pModel->m_pPartImageBuffer->nImageSizeX, CV_32FC1, pModel->m_pPart3D);
	cv::Mat model3DRoi = ImgFootModel3DCorrect(rtRoi).clone();

	cv::Mat convertFootModel3D;
	model3DRoi.convertTo(convertFootModel3D, CV_8UC1);

	//Calc Foot Model zero Point (Pad Average Height)
	int sum = 0, matchingCnt = 0;
	int nFoot3DmaxHeight = 0;
	size_t floatstep = model3DRoi.step / sizeof(float);
	for (int i = 0; i < model3DRoi.rows; i++)
	{
		float* rowptr = (float*)model3DRoi.data;
		for (int j = 0; j < model3DRoi.cols; j++)
		{
			int val = rowptr[i * floatstep + j];
			if (ImgOnlyPadArea.data[i * ImgOnlyPadArea.step + j] > 0)
			{
				matchingCnt++;
				sum += val;
			}

			if (Imgfoot.data[i * Imgfoot.step + j] == 255)
			{
				if (val > nFoot3DmaxHeight)
					nFoot3DmaxHeight = val;
			}
		}
	}
	sum /= matchingCnt;

	nFoot3DmaxHeight = nFoot3DmaxHeight - sum;

	UCHAR* ptrFoot = ImgFootModel_3D.data;
	UCHAR* ucArrModelImg = ptrFoot;
	m_pProcMilAlgo->SaveWorkImg(ucArrModelImg, nModelWidth, nModelHeight, _T("Foot_ModelImg.bmp"));

	Im::PIL_ID milSrcModelImg = M_NULL;
	milSrcModelImg = m_procPatternMil->AllocBuff(nModelWidth, nModelHeight);
	Im::Buf::Put(milSrcModelImg, ucArrModelImg);

	void* m_footImg_teachModel = M_NULL;
	m_footImg_teachModel = (void*)milSrcModelImg;		//Pattern Model

	//Pattern Matching Insp Area (search area)
	int searchWidth, searchHeight, searchArea;
	searchWidth = ROI.right - ROI.left;
	searchHeight = ROI.bottom - ROI.top;
	searchArea = searchWidth * searchHeight;

	cv::Rect cvImgRectInspArea;
	cvImgRectInspArea.x = ROI.left;
	cvImgRectInspArea.y = ROI.top;
	cvImgRectInspArea.width = searchWidth;
	cvImgRectInspArea.height = searchHeight;

	if (searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
		return ret;

	//Get Insp Area _2D
	cv::Mat ImgInspArea(searchHeight, searchWidth, CV_8UC1);
	pModel->GetImageMatrix(&ImgInspArea, pFoot, 1, cvImgRectInspArea);

	//Get Insp Area _3D
	cv::Mat ImgInspArea3D(pModel->m_pPartImageBuffer->nImageSizeY, pModel->m_pPartImageBuffer->nImageSizeX, CV_32FC1, pModel->m_pPart3D);
	cv::Rect roi(cvImgRectInspArea.x, cvImgRectInspArea.y, cvImgRectInspArea.width, cvImgRectInspArea.height);
	cv::Mat ImgInspArea3DClone = ImgInspArea3D(roi).clone();
	cv::Mat ImgInspArea3DClone_2 = ImgInspArea3D(roi).clone();		//32FC1

	//Subtract 3D pad AVG height (offset)
	cv::Mat convert3D;
	ImgInspArea3DClone -= sum;
	ImgInspArea3DClone.convertTo(convert3D, CV_8UC1);

	//remove wire Area (pad avg height + Foot MaxHeight)
	size_t floatstep2 = ImgInspArea3DClone_2.step / sizeof(float);
	for (int i = 0; i < ImgInspArea3DClone_2.rows; i++)
	{
		float* rowptr = (float*)ImgInspArea3DClone_2.data;
		for (int j = 0; j < ImgInspArea3DClone_2.cols; j++)
		{
			int val = rowptr[i * floatstep2 + j];

			if (val > (sum + nFoot3DmaxHeight))
			{
				convert3D.data[i * convert3D.step + j] = 0;
			}
			if (val < 0)
			{
				convert3D.data[i * convert3D.step + j] = 0;
			}
		}
	}

	//UCHAR* ucFootAutoSearchArea = new UCHAR[searchArea];
	UCHAR* ucFootAutoSearchArea = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);


	ucFootAutoSearchArea = convert3D.data;
	m_pProcMilAlgo->SaveWorkImg(ucFootAutoSearchArea, searchWidth, searchHeight, _T("Foot_AutoSearch_Area.bmp"));

	Im::PIL_ID milSrcInspAreaImg = M_NULL;
	milSrcInspAreaImg = m_procPatternMil->AllocBuff(searchWidth, searchHeight);;
	Im::Buf::Put(milSrcInspAreaImg, ucFootAutoSearchArea);
	/////////////////////////////////////

	//Repeat Pattern Matching
	for (int i = 0; i < nCntPatternPath; ++i)		//Repeat pattern model cnt
	{
		//Pattern Model Image Load
		strFullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, _T("Foot_ModelImg.bmp"));
		m_procPatternMil->ModelFileLoad(strFullPath);

		dAngleRotate = 0.0;
		angleTemp = dAngleRotate + wndAngle;

		m_procPatternMil->ModelPreproc(angleTemp, 0);	//m_pAlgoPattern->m_SamplingAngle = 0;		//SamplingAngle 0챙흹쩌챘징흹 챙탑?왗р뮴?챙짼?챘짝짭
		m_procPatternMil->SetnChannel(1);
		//m_pAlgoPattern->GetParam(m_pTeachParam);

		//Alloc model and model training
		int w = 0;
		int h = 0;
		double rotateAngle = 0;
		int allocRst = m_procPatternMil->AllocPatModel(ucArrModelImg, nModelWidth, nModelHeight, rotateAngle, w, h);

		m_procPatternMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
		m_procPatternMil->SetSearchStartAngle(0);
		m_procPatternMil->SetFindSubPixel(FALSE, FALSE);
		m_procPatternMil->PreprocModel(m_footImg_teachModel, FALSE, 1, 1);		//third param : use_algo(1:default)
		//typedef enum { agOriginal=0, agDefault=1, agLowContrast, agIntaglio, agRelief, agImage, agValue, agImageMin }enmMatchAlgo;

		int searchCnt = 0;
		double score = 0, angle = 0, posX = 0, posY = 0;
		int nRepeatCnt = 10;
		int ndetectedFootCnt = 0;
		UCHAR* MaskImg;

		for (int nRepeatIdx = 0; nRepeatIdx < nRepeatCnt; nRepeatIdx++)
		{
			if (allocRst == eMPAT_SUCCESS)
			{
				Img_Proc = std::make_shared<ImgProcessing>();
				m_procPatternMil->ImgProcess(ucFootAutoSearchArea, searchWidth, searchHeight, Img_Proc);
				searchCnt = m_procPatternMil->SearchPattern(ucFootAutoSearchArea, searchWidth, searchHeight);	//pattern matching

				if (searchCnt > 0)
				{
					m_procPatternMil->GetPatResult(&score, &angle, &posX, &posY);	//get pattern match result

					double dFootCenterX, dFootCenterY;

					if (score > m_nFootPatternMatchScore)
					{
						ndetectedFootCnt++;

						dFootCenterX = posX + ROI.left;
						dFootCenterY = posY + ROI.top;

						//Draw Center Point On Part Image
						FootRect footrect;
						memset(&footrect, 0, sizeof(FootRect));
						footrect.m_Point.x = dFootCenterX;
						footrect.m_Point.y = dFootCenterY;

						//Foot Center Rect
						footrect.m_FootRect.left = dFootCenterX - 20;
						footrect.m_FootRect.top = dFootCenterY - 20;
						footrect.m_FootRect.right = footrect.m_FootRect.left + 40;
						footrect.m_FootRect.bottom = footrect.m_FootRect.top + 40;

						//Get teached Foot ROI Size
						int footROIWidth = pFoot->mImageRect.right - pFoot->mImageRect.left;
						int footROIHeight = pFoot->mImageRect.bottom - pFoot->mImageRect.top;

						//Foot ROI
						footrect.m_PadRect.left = dFootCenterX - (footROIWidth / 2);
						footrect.m_PadRect.right = dFootCenterX + (footROIWidth / 2);
						footrect.m_PadRect.top = dFootCenterY - (footROIHeight / 2);
						footrect.m_PadRect.bottom = dFootCenterY + (footROIHeight / 2);

						rst.emplace_back(footrect);		//Set result 

						//Except matching area (InspArea - ExceptMatchedArea)
						MaskImg = ucFootAutoSearchArea;
						CRect rtExceptArea;

						int nExceptLeft = posX - (nModelWidth / 2);
						int nExceptRight = posX + (nModelWidth / 2);
						int nExceptTop = posY - (nModelHeight / 2);
						int nExceptBottom = posY + (nModelHeight / 2);

						rtExceptArea.left = nExceptLeft < 0 ? 0 : nExceptLeft;
						rtExceptArea.right = nExceptRight > searchWidth ? searchWidth : nExceptRight;
						rtExceptArea.top = nExceptTop < 0 ? 0 : nExceptTop;
						rtExceptArea.bottom = nExceptBottom > searchHeight ? searchHeight : nExceptBottom;

						for (int i = rtExceptArea.top; i < rtExceptArea.bottom; i++)
						{
							for (int j = rtExceptArea.left; j < rtExceptArea.right; j++)
							{
								MaskImg[i*searchWidth + j] = 0;
							}
						}

						cv::Mat maskImage(searchHeight, searchWidth, CV_8UC1, MaskImg);

						ucFootAutoSearchArea = MaskImg;
#if _DEBUG
						CString cstrIndex;
						cstrIndex.Format(_T("ExceptFootArea_%d"), ndetectedFootCnt);
						CString path = g_pMPTI->GetWireDebugDataFullPath(1, _T("FootPatternMatch"), cstrIndex, 0);
						if (MaskImg && searchWidth > 0 && searchHeight > 0)
							m_pProcMilAlgo->SaveDebugImg(MaskImg, searchWidth, searchHeight, path, 1, FALSE);
#endif
					}
					else
					{
						break;
					}
				}
				else
					break;
			}

			score = 0;

		}
	}

	ret = true;

	return ret;
}

bool CPInsp_Foot::getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection)
{
	bool ret = false;
	return ret;
}

bool CPInsp_Foot::FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	//If the Foot cannot be found by pattern matching, the 2D and 3D image is used.
	bool ret = false;
	return ret;
}

int CPInsp_Foot::FindFoot_Pat(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, FootPoins &fPoints, int nfootKind)
{
	int nLine = __LINE__;

	UCHAR* pucImg = NULL;
	float* pfImg = NULL;
	USHORT* LabelImage = NULL;
	try
	{
		int nImageHeight = pFoot->GetImageLength();
		int nImageWidth = pFoot->GetImageWidth();
		fPoints.rcArea.left = 0;
		fPoints.rcArea.top = 0;
		fPoints.rcArea.right = nImageWidth;
		fPoints.rcArea.bottom = nImageHeight;
		int nImgWidth = nImageWidth;
		int nImgHeight = nImageHeight;
		int imgSize = nImgWidth * nImgHeight;

		long pLebel[PTR_BLOB_MAX];
		double pLebel_Area[PTR_BLOB_MAX];
		int pLebel_MaxIDX[PTR_BLOB_MAX];

		cv::Mat cv3DArea(nImgHeight, nImgWidth, CV_32FC1, pFoot->pf3D);
		cv::Mat ImgPatTarg(cvFootImg->rows, cvFootImg->cols, CV_8UC1);

		for (int r = 0; r < nImageHeight; r++)
		{
			UCHAR* TarPtr = ImgPatTarg.ptr(r);
			UCHAR* imgSrc = &pFoot->m_ImageBuffer->imgTop_W[r*nImageWidth];
			memcpy(TarPtr, imgSrc, sizeof(UCHAR)*nImageWidth);
		}

		//Foot BW 티칭 범위로 object 잘 나오는지 검증
		pFoot->GetImageMatrix(cvFootImg, (int)m_eFootBin::m_eFootBin_Foot);  //m_eFootBin_Pad
		cv::Mat BinImage(cvFootImg->rows, cvFootImg->cols, CV_8UC1);
		cv::Mat srcImg = *cvFootImg;
#if _DEBUG
		cv::imwrite("D:\\FootInspImg\\FindFoot_Org_Foot2.bmp", srcImg);
#endif
		
		int nSearchSizeX = pFoot->m_fArrOptionValue[(int)m_eFoot::m_eFoot_DeviationX][m_eMMD::eMMD_Default] / m_resolX;
		int nSearchSizeY = pFoot->m_fArrOptionValue[(int)m_eFoot::m_eFoot_DeviationY][m_eMMD::eMMD_Default] / m_resolY;
		if (nSearchSizeX < 10) nSearchSizeX = 10;
		if (nSearchSizeX > nImageWidth) nSearchSizeX = nImageWidth - 1;
		if (nSearchSizeY < 10) nSearchSizeY = 10;
		if (nSearchSizeY > nImageHeight) nSearchSizeY = nImageHeight - 1;

		double dArea = 0, dCX = 0, dCY = 0;
		CRect rcBlob{ 0,0,0,0 };
		float fHIMin = 0.0f;
		float fHIMax = 0.0f;

		CPInsp::GetHeightMinMax(pFoot->pf3D, nImageWidth, nImageHeight, &fHIMin, &fHIMax);

		nLine = __LINE__;

		sRstAlgo->m_rcRefArea.left = 0;
		sRstAlgo->m_rcRefArea.top = 0;
		sRstAlgo->m_rcRefArea.right = 0;
		sRstAlgo->m_rcRefArea.bottom = 0;
		sRstAlgo->m_fAreaHeight = 0.0f;

		AlgoBlob algoBlob = pFoot->GetAlgoBlob((int)m_eFootBin::m_eFootBin_Foot);    //m_eFootBin_Pad);

		if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_Find_RefA) == m_eFOOT_Data2::m_eFOOT_Data2_Find_RefA &&
			(pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_RefArea) == m_eFOOT_Data2::m_eFOOT_Data2_RefArea)
		{
			algoBlob.m_bInsp3D = TRUE;
			algoBlob.m_nTypeRange3D = eTypeRangeIn;
			float fAreaHeight = sRstAlgo->m_fAreaHeight;
			if (fAreaHeight > 0) fAreaHeight *= 0.8f;
			if (fAreaHeight < 0) fAreaHeight *= 1.1f;
			algoBlob.m_dHeightRateMin = fAreaHeight;
			algoBlob.m_dHeightRateMax = fAreaHeight + 350.0f;
			if (algoBlob.m_dHeightRateMax > fHIMax / 2.0f && fHIMax / 2.0f > algoBlob.m_dHeightRateMin + 200.0f)
				algoBlob.m_dHeightRateMax = fHIMax / 2.0f;
		}

		TotalInspExceptArea stTieAreaNULL;
		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;

		int nCntBlob = CPInsp::BlobImageStruct(algoBlob, srcImg.data, pFoot->pf3D, NULL, srcImg.cols, srcImg.rows, 100, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
		//int nCntBlob = PIAL::PAlgo::BlobImageStruct(algoBlob, &srcImgBuff, &Img3DBuff, NULL, &BinImageBuff, srcImgBuff.Width(), srcImgBuff.Length(), 4, &dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);

		nLine = __LINE__;

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Blob_foot.bmp", BinImage);
#endif

		if (nCntBlob == 0)
		{
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("CPInsp_Foot::FindFoot_Pat blob Count == 0 "));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return 0;
		}

#pragma region patternMatching

		bool bMatchSuccess = false;
		fPoints.cvMatchingMask = cv::Mat(nImgHeight, nImgWidth, CV_8UC1);

		std::shared_ptr<GeoResult_Foot> pPatResult = std::make_shared<GeoResult_Foot>();
		cv::Rect RectModelImg[4];
		int MatchModelNum[2];
		pPatResult->Alloc(2);
		if (m_bMatch2D)
			bMatchSuccess = MatchModel(pFoot, ImgPatTarg, *cvPadImg, pPatResult, RectModelImg, BinImage, cv3DArea, MatchModelNum);
		if (m_bMatch3D && !bMatchSuccess)
			bMatchSuccess = MatchModel3D(pFoot, cv3DArea, *cvPadImg, pPatResult, RectModelImg, BinImage, MatchModelNum);
		int MatchingIndex = -1;
		int NMatchIndex = -1;
		if (bMatchSuccess)
		{

			nLine = __LINE__;
			double diff1 = std::pow(pPatResult->_Center_y[0] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[0] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
			double diff2 = std::pow(pPatResult->_Center_y[1] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[1] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
			MatchingIndex = 0;
			NMatchIndex = 1;
			if (diff1 > diff2)
			{
				MatchingIndex = 1;
				NMatchIndex = 0;
			}
			sRstAlgo->m_nMatchModelNum = MatchModelNum[MatchingIndex];
			sRstAlgo->MatchingScore = pPatResult->_Equality[MatchingIndex];

			double MatchingAngle = pPatResult->_Angle[MatchingIndex];
			if (pFoot->m_bUsePatternAngle)
			{
				pPatResult->_Equality[0] = pPatResult->_Equality[1] = 0;
				pPatResult->_Center_y[0] = pPatResult->_Center_y[1] = 0;
				pPatResult->_Center_x[0] = pPatResult->_Center_x[1] = 0;
				if (m_bMatch2D)
					bMatchSuccess = MatchModel(pFoot,ImgPatTarg, *cvPadImg, pPatResult, RectModelImg, BinImage, cv3DArea, MatchModelNum, MatchingAngle);
				if (m_bMatch3D && !bMatchSuccess)
					bMatchSuccess = MatchModel3D(pFoot, cv3DArea, *cvPadImg, pPatResult, RectModelImg, BinImage, MatchModelNum, MatchingAngle);
			}

			diff1 = std::pow(pPatResult->_Center_y[0] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[0] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
			diff2 = std::pow(pPatResult->_Center_y[1] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[1] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
			MatchingIndex = 0;
			NMatchIndex = 1;
			if (diff1 > diff2) 
			{
				MatchingIndex = 1;
				NMatchIndex = 0;
			}
			MatchingAngle = pPatResult->_Angle[MatchingIndex];
			while (MatchingAngle < 0) { MatchingAngle += 360; }
			if (pFoot->m_nFootAngle!= 0 && abs(pFoot->m_nFootAngle - MatchingAngle) > 90)MatchingAngle -= 180;
			else if (pFoot->m_nFootAngle == 0 && abs(pFoot->m_nFootAngle - MatchingAngle) > 90
				&& abs(pFoot->m_nFootAngle - MatchingAngle) < 270)MatchingAngle -= 180;
			sRstAlgo->m_nMatchModelNum = MatchModelNum[MatchingIndex];
			sRstAlgo->MatchingScore = pPatResult->_Equality[MatchingIndex];
			cv::Point pts[1][4];
			int cx = pPatResult->_Center_y[MatchingIndex];
			int cy = pPatResult->_Center_x[MatchingIndex];

			double w = RectModelImg[MatchingIndex].width / 2;
			double h = RectModelImg[MatchingIndex].height / 2;

			double dLeng = std::sqrt(std::pow(w, 2) + std::pow(h, 2));
			double dRate = h / w;
			double dstdRadian = std::atan(dRate);
			double dstdDegree = dstdRadian / std::_Pi * 180;

			double dRadian1 = (-MatchingAngle - (dstdDegree)) * std::_Pi / 180;
			double dRadian2 = (-MatchingAngle - (180 - dstdDegree)) * std::_Pi / 180;
			double dRadian3 = (-MatchingAngle - (180 + dstdDegree)) * std::_Pi / 180;
			double dRadian4 = (-MatchingAngle - (360 - dstdDegree)) * std::_Pi / 180;
			double diffX1 = std::cos(dRadian1) * dLeng;
			double diffY1 = std::sin(dRadian1) * dLeng;
			double diffX2 = std::cos(dRadian2) * dLeng;
			double diffY2 = std::sin(dRadian2) * dLeng;
			double diffX3 = std::cos(dRadian3) * dLeng;
			double diffY3 = std::sin(dRadian3) * dLeng;
			double diffX4 = std::cos(dRadian4) * dLeng;
			double diffY4 = std::sin(dRadian4) * dLeng;

			int p1x = cx + (int)std::round(diffX1); int  p1y = cy + (int)std::round(diffY1);
			int p2x = cx + (int)std::round(diffX2); int  p2y = cy + (int)std::round(diffY2);
			int p3x = cx + (int)std::round(diffX3); int  p3y = cy + (int)std::round(diffY3);
			int p4x = cx + (int)std::round(diffX4); int  p4y = cy + (int)std::round(diffY4);
			pts[0][0] = cv::Point(p1x, p1y);
			pts[0][1] = cv::Point(p2x, p2y);
			pts[0][2] = cv::Point(p3x, p3y);
			pts[0][3] = cv::Point(p4x, p4y);

			for (int i = 0; i < 4; i++)
			{
				for (int j = i; j < 4; j++)
				{
					if (pts[0][i].y > pts[0][j].y)
					{
						cv::Point pTemp = pts[0][i];
						pts[0][i] = pts[0][j];
						pts[0][j] = pTemp;
					}
				}
			}
			if (pts[0][1].x > pts[0][0].x)
			{
				cv::Point pTemp = pts[0][1];
				pts[0][1] = pts[0][0];
				pts[0][0] = pTemp;
			}
			if (pts[0][2].x > pts[0][3].x)
			{
				cv::Point pTemp = pts[0][2];
				pts[0][2] = pts[0][3];
				pts[0][3] = pTemp;
			}

			//1) end점 3,4를 잇는 선의 방정식을 계산
			//2) 각도에 따라서 선이 이미지 끝에 닿는 영역 계산
			//2-1) 굳이 계산할 필요없이 대각선의 길이로좌표값을 설정하면 편하게 그릴수있음.
			//2-2) 2Foot의 경우 두번째 매칭포인트에 절반으로 width값을 수정
			//if (dstdDegree >= 45 && dstdDegree < 135)//90
			int PolyLeft = 0;
			int PolyRight = fPoints.cvMatchingMask.cols - 1;
			int PolyTop = 0;
			int PolyBottom = fPoints.cvMatchingMask.rows - 1;
			double dMinX = 0, dMinY = 0;

			nLine = __LINE__;


			float _Center_x[2] = { pPatResult->_Center_y[0] ,pPatResult->_Center_y[1] };
			float _Center_y[2] = { pPatResult->_Center_x[0] ,pPatResult->_Center_x[1] };
			double dFootDegree = pFoot->m_nFootAngle;
			if ((dFootDegree >= 45 && dFootDegree < 135) || (dFootDegree >= 225 && dFootDegree < 315))//90, 270
			{
				_Center_x[0] = pPatResult->_Center_x[0];
				_Center_x[1] = pPatResult->_Center_x[1];
				_Center_y[0] = pPatResult->_Center_y[0];
				_Center_y[1] = pPatResult->_Center_y[1];

// 				p1y = cx + (int)std::round(diffX1); p1x = cy + (int)std::round(diffY1);
// 				p2y = cx + (int)std::round(diffX2); p2x = cy + (int)std::round(diffY2);
// 				p3y = cx + (int)std::round(diffX3); p3x = cy + (int)std::round(diffY3);
// 				p4y = cx + (int)std::round(diffX4); p4x = cy + (int)std::round(diffY4);
// 				pts[0][0] = cv::Point(p1x, p1y);
// 				pts[0][1] = cv::Point(p2x, p2y);
// 				pts[0][2] = cv::Point(p3x, p3y);
// 				pts[0][3] = cv::Point(p4x, p4y);

				p1y = pts[0][2].x; p1x = pts[0][2].y;
				p2y = pts[0][1].x; p2x = pts[0][1].y;
				p3y = pts[0][0].x; p3x = pts[0][0].y;
				p4y = pts[0][3].x; p4x = pts[0][3].y;
				pts[0][0] = cv::Point(p1x, p1y);
				pts[0][1] = cv::Point(p2x, p2y);
				pts[0][2] = cv::Point(p3x, p3y);
				pts[0][3] = cv::Point(p4x, p4y);

				PolyLeft = 0;
				PolyRight = fPoints.cvMatchingMask.rows - 1;
				PolyTop = 0;
				PolyBottom = fPoints.cvMatchingMask.cols - 1;
			}

			bool bUse2Foot = pFoot->m_bUse2Foot;
			if ((pPatResult->_Center_y[1] == pPatResult->_Center_y[0]
				&& pPatResult->_Center_x[1] == pPatResult->_Center_x[0])
				|| (pPatResult->_Center_y[NMatchIndex] == 0 && pPatResult->_Center_x[NMatchIndex] == 0))
				bUse2Foot = false;
			if (bUse2Foot)
			{
				//변곡점찾기
				//1 두 중심점을 기준으로 선을 추출

				nLine = __LINE__;
				double xDiff = (_Center_x[1] - _Center_x[0]);
				double a = xDiff == 0 ? 0 : (_Center_y[1] - _Center_y[0]) / xDiff;
				double b = _Center_y[1] - a * _Center_x[1];
				int pstX = _Center_x[0];
				int pedX = _Center_x[1];
				if (_Center_x[0] > _Center_x[1])
				{
					pstX = _Center_x[1];
					pedX = _Center_x[0];
				}
				if (pedX <= pstX)
				{
					pedX = pstX + 1;
				}

				nLine = __LINE__;

// 				CString errorLog;
// 				errorLog.Format(_T("CPInsp_Foot::FindFoot_Pat() Line: %d Pass"), nLine);
// 				g_pMPTI->AddLog(errorLog);

				//2선을따라서 높이 추출
				std::vector<cv::Point2f> pint;

				if ((dFootDegree >= 45 && dFootDegree < 135) || (dFootDegree >= 225 && dFootDegree < 315))//90, 270
				{
					for (int c = pstX; c < pedX; c++)
					{
						pint.push_back(cv::Point2f(c, cv3DArea.ptr<float>(c)[(int)std::roundl(a*c + b)]));
					}
				}
				else 
				{

					for (int c = pstX; c < pedX; c++)
					{
						pint.push_back(cv::Point2f(c, cv3DArea.ptr<float>(a*c + b)[c]));
					}
				}

				nLine = __LINE__;

// 				errorLog.Format(_T("CPInsp_Foot::FindFoot_Pat() Line: %d Pass"), nLine);
// 				g_pMPTI->AddLog(errorLog);

				double a1, b1, c1;
				int cnt = pint.size();
				//
				{

					cv::Mat matrixX(cnt, 3, CV_32F), matrixXT, matrixY(cnt, 1, CV_32F);

					double dAvg(0.0);
					double zeroPy = pint[0].y;

					for (int i = 0; i < cnt; i++)
					{
						float* fptr = matrixX.ptr<float>(i);
						float* fptry = matrixY.ptr<float>(i);

						fptr[0] = std::pow((double)pint[i].x, 2);
						fptr[1] = pint[i].x;
						fptr[2] = 1;

						fptry[0] = pint[i].y;
						dAvg += pint[i].y;
					}

					cv::Mat Xt = matrixX.t();
					cv::Mat XX = Xt * matrixX;
					cv::Mat XY = Xt * matrixY;
					cv::Mat res = XX.inv() * XY;;

#if _DEBUG
					float* XXptr = XX.ptr<float>(0);
					float* XTptr = Xt.ptr<float>(0);
					float* XYptr = XY.ptr<float>(0);
#endif

					float* Reptr = res.ptr<float>(0);
					a1 = Reptr[0];
					b1 = Reptr[1];

					double baseAvg = dAvg / cnt;
					double Calc_y = Reptr[0] * pint[0].x*pint[0].x + Reptr[1] * pint[0].x + baseAvg;
					double marginY = zeroPy - Calc_y;

					c1 = marginY + baseAvg;
				}

				dMinX = -b1 / (2 * a1);
				dMinY = a * dMinX + b;
			}

			nLine = __LINE__;

			while (dFootDegree < 0) { dFootDegree += 360; }
			if (dFootDegree >= 45 && dFootDegree < 135)//90
			{
				int nLt = 0;
				int nRt = 0;
				if (bUse2Foot)
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = dMinX;
						nLt = dMinY;
					}
					else
					{
						PolyRight = dMinX;
						nRt = dMinY;
					}
				}

				double a = ((double)p4y - p3y) / ((double)p4x - p3x);
				double a2 = ((double)p3y - p2y) / ((double)p3x - p2x);
				double b = p3y - a * p3x;
				double aInv = -1 / a;
				double b2 = nLt - aInv * PolyLeft;
				double b3 = nRt - aInv * PolyRight;

				int Np1x = fPoints.cvMatchingMask.rows - 1;
				int Np1y = 0;
				int Np2x = 0;
				int Np2y = 0;
				int Np3x = 0;
				int Np3y = fPoints.cvMatchingMask.cols - 1;
				int Np4x = fPoints.cvMatchingMask.rows - 1;
				int Np4y = fPoints.cvMatchingMask.cols - 1;
				if (a != 0 && aInv != 0)
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = -((b - b2) / (a - aInv));
						Np2y = PolyTop;
						Np2x = (Np2y - b2) / aInv;
						Np3x = PolyLeft;
						Np3y = a * (-(b - b2) / (a - aInv)) + b;
						Np4x = PolyRight;
						Np4y = a * Np4x + b;
					}
					else
					{
						PolyRight = -((b - b3) / (a - aInv));
						Np1y = PolyTop;
						Np1x = (Np1y - b3) / aInv;
						Np3x = PolyLeft;
						Np3y = a * (-(b - b2) / (a - aInv)) + b;
						Np4x = PolyRight;
						Np4y = a * Np4x + b;
					}
				}
				else
				{
					Np1x = PolyRight - 1;
					Np1y = 0;
					Np2x = PolyLeft;
					Np2y = 0;
					Np3x = PolyLeft;
					Np3y = pts[0][2].y;
					Np4x = PolyRight - 1;
					Np4y = pts[0][3].y;
				}
				pts[0][0] = cv::Point(Np1y, Np1x);
				pts[0][1] = cv::Point(Np2y, Np2x);
				pts[0][2] = cv::Point(Np3y, Np3x);
				pts[0][3] = cv::Point(Np4y, Np4x);
			}
			else if (dFootDegree >= 135 && dFootDegree < 225)//180
			{
				int nLt = 0;
				int nRt = 0;
				if (bUse2Foot)
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = dMinX;
						nLt = dMinY;
					}
					else
					{
						PolyRight = dMinX;
						nRt = dMinY;
					}
				}

// 				double a = ((double)p4y - p3y) / ((double)p4x - p3x);
// 				double a2 = ((double)p3y - p2y) / ((double)p3x - p2x);
				double a = ((double)pts[0][0].y - pts[0][1].y) / ((double)pts[0][0].x - pts[0][1].x);
				double a2 = ((double)pts[0][1].y - pts[0][2].y) / ((double)pts[0][1].x - pts[0][2].x);
				double b = pts[0][1].y - a * pts[0][1].x;
				double aInv = -1 / a;
				double b2 = nLt - aInv * PolyLeft;
				double b3 = nRt - aInv * PolyRight;

				int Np1x = fPoints.cvMatchingMask.cols - 1;
				int Np1y = 0;
				int Np2x = 0;
				int Np2y = 0;
				int Np3x = 0;
				int Np3y = fPoints.cvMatchingMask.rows - 1;
				int Np4x = fPoints.cvMatchingMask.cols - 1;
				int Np4y = fPoints.cvMatchingMask.rows - 1;

				if (a != 0 && aInv != 0) 
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = -((b - b2) / (a - aInv));
						Np1x = PolyRight;
						Np1y = a * Np1x + b;
						Np2x = PolyLeft;
						Np2y = a * Np2x + b;
						Np3y = PolyBottom;
						Np3x = (Np3y - b2) / aInv;
					}
					else
					{
						PolyRight = -((b - b3) / (a - aInv));
						Np1x = PolyRight;
						Np1y = a * Np1x + b;
						Np2x = PolyLeft;
						Np2y = a * Np2x + b;
							// 					Np4x = PolyRight;
							// 					Np4y = aInv * Np4x + b3;
						Np4y = PolyBottom;
						Np4x = (Np4y - b3) / aInv;
					}
				}
				else
				{
					Np1x = PolyRight - 1;
					Np1y = pts[0][0].y;
					Np2x = PolyLeft;
					Np2y = pts[0][1].y;
					Np3x = PolyLeft;
					Np3y = fPoints.cvMatchingMask.rows - 1;
					Np4x = PolyRight - 1;
					Np4y = fPoints.cvMatchingMask.rows - 1;
				}


				pts[0][0] = cv::Point(Np1x, Np1y);
				pts[0][1] = cv::Point(Np2x, Np2y);
				pts[0][2] = cv::Point(Np3x, Np3y);
				pts[0][3] = cv::Point(Np4x, Np4y);
			}
			else if (dFootDegree >= 225 && dFootDegree < 315)//270
			{
				int nLt = 0;
				int nRt = 0;
				if (bUse2Foot)
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = dMinX;
						nLt = dMinY;
					}
					else
					{
						PolyRight = dMinX;
						nRt = dMinY;
					}
				}
// 				double a = ((double)p4y - p3y) / ((double)p4x - p3x);
// 				double a2 = ((double)p3y - p2y) / ((double)p3x - p2x);
				double a = ((double)pts[0][0].y - pts[0][1].y) / ((double)pts[0][0].x - pts[0][1].x);
				double a2 = ((double)pts[0][1].y - pts[0][2].y) / ((double)pts[0][1].x - pts[0][2].x);
				double b = pts[0][1].y - a * pts[0][1].x;
				double aInv = -1 / a;
				double b2 = nLt - aInv * PolyLeft;
				double b3 = nRt - aInv * PolyRight;

				int Np1x = fPoints.cvMatchingMask.rows - 1;
				int Np1y = 0;
				int Np2x = 0;
				int Np2y = 0;
				int Np3x = 0;
				int Np3y = fPoints.cvMatchingMask.cols - 1;
				int Np4x = fPoints.cvMatchingMask.rows - 1;
				int Np4y = fPoints.cvMatchingMask.cols - 1;

				if (a != 0 && aInv != 0)
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = -((b - b2) / (a - aInv));
						Np1x = PolyRight;
						Np1y = a * Np1x + b;
						Np2x = PolyLeft;
						Np2y = a * Np2x + b;
						Np3y = PolyBottom;
						Np3x = (Np3y - b2) / aInv;
					}
					else
					{
						PolyRight = -((b - b3) / (a - aInv));
						Np1x = PolyRight;
						Np1y = a * Np1x + b;
						Np2x = PolyLeft;
						Np2y = a * Np2x + b;
						// 					Np4x = PolyRight;
						// 					Np4y = aInv * Np4x + b3;
						Np4y = PolyBottom;
						Np4x = (Np4y - b3) / aInv;
					}
				}
				else
				{
					Np1x = PolyRight - 1;
					Np1y = pts[0][0].y;
					Np2x = PolyLeft;
					Np2y = pts[0][1].y;
					Np3x = PolyLeft;
					Np3y = fPoints.cvMatchingMask.cols - 1;
					Np4x = PolyRight - 1;
					Np4y = fPoints.cvMatchingMask.cols - 1;
				}

				pts[0][0] = cv::Point(Np1y, Np1x);
				pts[0][1] = cv::Point(Np2y, Np2x);
				pts[0][2] = cv::Point(Np3y, Np3x);
				pts[0][3] = cv::Point(Np4y, Np4x);
			}
			else//0
			{
				int nLt = 0;
				int nRt = 0;
				if(bUse2Foot)
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = dMinX;
						nLt = dMinY;
					}
					else
					{
						PolyRight = dMinX;
						nRt = dMinY;
					}
				}

				double a = ((double)p4y - p3y) / ((double)p4x - p3x);
				double a2 = ((double)p3y - p2y) / ((double)p3x - p2x);
				double b = p3y - a * p3x;
				double aInv = -1 / a;
				double b2 = nLt - aInv * PolyLeft;
				double b3 = nRt - aInv * PolyRight;

				int Np1x = fPoints.cvMatchingMask.cols - 1;
				int Np1y = 0;
				int Np2x = 0;
				int Np2y = 0;
				int Np3x = 0;
				int Np3y = fPoints.cvMatchingMask.rows - 1;
				int Np4x = fPoints.cvMatchingMask.cols - 1;
				int Np4y = fPoints.cvMatchingMask.rows - 1;
				if (a != 0 && aInv != 0) 
				{
					if (_Center_x[MatchingIndex] > _Center_x[NMatchIndex])
					{
						PolyLeft = -((b - b2) / (a - aInv));
						Np2y = PolyTop;
						Np2x = (Np2y - b2) / aInv;
						Np3x = PolyLeft;
						Np3y = a * (-(b - b2) / (a - aInv)) + b;
						Np4x = PolyRight;
						Np4y = a * Np4x + b;
					}
					else
					{
						PolyRight = -((b - b3) / (a - aInv));
						Np1y = PolyTop;
						Np1x = (Np1y - b3) / aInv;
						Np3x = PolyLeft;
						Np3y = a * (-(b - b2) / (a - aInv)) + b;
						Np4x = PolyRight;
						Np4y = a * Np4x + b;
					}
				}
				else
				{
					Np1x = PolyRight - 1;
					Np1y = 0;
					Np2x = PolyLeft;
					Np2y = 0;
					Np3x = PolyLeft;
					Np3y = pts[0][2].y;
					Np4x = PolyRight - 1;
					Np4y = pts[0][3].y;
				}
				pts[0][0] = cv::Point(Np1x, Np1y);
				pts[0][1] = cv::Point(Np2x, Np2y);
				pts[0][2] = cv::Point(Np3x, Np3y);
				pts[0][3] = cv::Point(Np4x, Np4y);
			}


			//3) 마스크 이미지 생성
			int nps[] = { 4 };
			const cv::Point* ptr[1] = { pts[0] };

			fPoints.cvMatchingMask.setTo(0);
			cv::fillPoly(fPoints.cvMatchingMask, ptr, nps, 1, cv::Scalar(255));
			
			fPoints.m_dSeta = -pPatResult->_Angle[MatchingIndex];
			fPoints.m_pCenter.x = cx;
			fPoints.m_pCenter.y = cy;
			pFoot->m_BondingRect.left = RectModelImg[MatchingIndex + 2].x;
			pFoot->m_BondingRect.top = RectModelImg[MatchingIndex + 2].y;
			pFoot->m_BondingRect.right = RectModelImg[MatchingIndex + 2].x + RectModelImg[MatchingIndex + 2].width;
			pFoot->m_BondingRect.bottom = RectModelImg[MatchingIndex + 2].y + RectModelImg[MatchingIndex + 2].height;
#if _DEBUG
			cv::Mat cvPatternMatchMask;
			cvPatternMatchMask = fPoints.cvMatchingMask.clone();
			cv::imwrite("D:\\FootInspImg\\bMatchSuccess_cvMatchingMask.bmp", fPoints.cvMatchingMask);
			cv::imwrite("D:\\FootRst\\bMatchSuccess_ImgPatTarg.bmp", ImgPatTarg);
#endif

			int nLeft1 = p1x < p2x ? p1x : p2x;
			int nLeft2 = p3x < p4x ? p3x : p4x;
			fPoints.rcArea.left = nLeft1 < nLeft2 ? nLeft1 : nLeft2;

			int nRight1 = p1x > p2x ? p1x : p2x;
			int nRight2 = p3x > p4x ? p3x : p4x;
			fPoints.rcArea.right = nRight1 > nRight2 ? nRight1 : nRight2;

			int nTop1 = p1y < p2y ? p1y : p2y;
			int nTop2 = p3y < p4y ? p3y : p4y;
			fPoints.rcArea.top = nTop1 < nTop2 ? nTop1 : nTop2;

			int nBottom1 = p1y > p2y ? p1y : p2y;
			int nBottom2 = p3y > p4y ? p3y : p4y;
			fPoints.rcArea.bottom = nBottom1 > nBottom2 ? nBottom1 : nBottom2;

		}
		fPoints.bMatchSuccess = bMatchSuccess;

#pragma endregion
		
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&LabelImage, imgSize);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg, imgSize);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pfImg, imgSize);
		memset(pLebel, 0, sizeof(long) * PTR_BLOB_MAX);
		memset(pLebel_Area, 0, sizeof(double) * PTR_BLOB_MAX);
		memset(pLebel_MaxIDX, 0, sizeof(int) * PTR_BLOB_MAX);
		memset(LabelImage, 0, sizeof(USHORT) * imgSize);
		memset(pucImg, 0, sizeof(UCHAR) * imgSize);
		memset(pfImg, 0, sizeof(float) * imgSize);
		m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImage, nImgWidth, nImgHeight);
		for (int a = 0; a < nCntBlob; a++)
		{
			for (int b = 0; b < nImgWidth * nImgHeight; b++)
			{
				if (pLebel[a] == LabelImage[b])
					pLebel_Area[a]++;
			}
		}
		if (GetCorrectIDX(nCntBlob, pLebel_Area, pLebel_MaxIDX) == 0)
		{
			Delete_1DArray(&pucImg);
			Delete_1DArray(&pfImg);
			Delete_1DArray(&LabelImage);
			return 0;
		}

		//Pad Area
		int nstartX = pFoot->mImageRect.left - pFoot->mWindowImageRect.left; 
		int nstartY = pFoot->mImageRect.top - pFoot->mWindowImageRect.top;

		sRstAlgo->m_rcSearchArea.left = rcBlob.left + nstartX;
		sRstAlgo->m_rcSearchArea.top = rcBlob.top + nstartY;
		sRstAlgo->m_rcSearchArea.right = rcBlob.right + nstartX;
		sRstAlgo->m_rcSearchArea.bottom = rcBlob.bottom + nstartY;

		std::vector<long> vecLabelOrder;
		int ntempCntBlob = nCntBlob;
		if (/*nfootKind == -2 &&*/ nCntBlob > 1)
		{
			int nFootTeach = 1;
			vecLabelOrder.push_back(pLebel[pLebel_MaxIDX[0]]);

			for (int a = 1; a < nCntBlob; a++)
			{
				if (pLebel_Area[pLebel_MaxIDX[a]] > pLebel_Area[pLebel_MaxIDX[0]] * 0.5)
				{
					nFootTeach++;
					vecLabelOrder.push_back(pLebel[pLebel_MaxIDX[a]]);
				}
			}
			nCntBlob = nFootTeach;
		}

		cv::Mat BinImage1(nImgHeight, nImgWidth, CV_8UC1);
		if (nfootKind != -2 && ntempCntBlob > 1 && !bMatchSuccess)
		{

			//
			int nCntNearCtrBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage.data, BinImage1.data, nImgWidth, nImgHeight, 10000, FALSE, TRUE, 0, eSelectPosition, pFoot->m_FootCenterPoint_ImageRect.x, pFoot->m_FootCenterPoint_ImageRect.y);
			if (nCntNearCtrBlob > 0)
			{
				CRect rcBlob{ 0,0,0,0 };
				m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);

				fPoints.rcArea.left = rcBlob.left;
				fPoints.rcArea.top = rcBlob.top;
				fPoints.rcArea.right = rcBlob.right;
				fPoints.rcArea.bottom = rcBlob.bottom;
			}
			else
			{
			int nFootL = pFoot->m_FootCenterPoint_ImageRect.x - (nSearchSizeX / 2);
			int nFootT = pFoot->m_FootCenterPoint_ImageRect.y - (nSearchSizeY / 2);
			int nFootR = nFootL + nSearchSizeX;
			int nFootB = nFootT + nSearchSizeY;
			if (nFootL < 0) nFootL = 0;
			if (nFootT < 0) nFootT = 0;
			if (nFootR > nImgWidth) nFootR = nImgWidth;
			if (nFootB > nImgHeight) nFootB = nImgHeight;

			if (nFootL >= nImgWidth || nFootT >= nImgHeight)
				{
				Delete_1DArray(&pucImg);
				Delete_1DArray(&pfImg);
				Delete_1DArray(&LabelImage);
				return 0;
			}

			if (nFootR <= 0 || nFootB <= 0)
				{
				Delete_1DArray(&pucImg);
				Delete_1DArray(&pfImg);
				Delete_1DArray(&LabelImage);
				return 0;
			}

			fPoints.rcArea = CPInsp::GetBlobRect(&vecLabelOrder[0], LabelImage, nCntBlob, nImgWidth, nImgHeight, nFootL, nFootR, nFootT, nFootB);
			if (fPoints.rcArea.left == -1 || fPoints.rcArea.right == -1 || fPoints.rcArea.top == -1 || fPoints.rcArea.bottom == -1)
			{
				bool bSearch = false;
				while (nFootL > 1 || nFootT > 1 || nFootR < nImgWidth - 1 || nFootB < nImgHeight - 1)
				{
					--nFootL;
					--nFootT;
					++nFootR;
					++nFootB;

					if (nFootL < 1) nFootL = 1;
					if (nFootT < 1) nFootT = 1;
					if (nFootR >= nImgWidth) nFootR = nImgWidth - 1;
					if (nFootB >= nImgHeight) nFootB = nImgHeight - 1;

					if (HasBlobRectLine(&vecLabelOrder[0], LabelImage, nCntBlob, nImgWidth, nImgHeight, nFootL, nFootR, nFootT, nFootB))
					{
						fPoints.rcArea = CPInsp::GetBlobRect(&vecLabelOrder[0], LabelImage, nCntBlob, nImgWidth, nImgHeight, nFootL, nFootR, nFootT, nFootB);

						if (fPoints.rcArea.left != -1 && fPoints.rcArea.right != -1 && fPoints.rcArea.top != -1 && fPoints.rcArea.bottom != -1)
						{
							bSearch = true;
							break;
						}
					}

				}

			}
		}
		}

		Delete_1DArray(&pucImg);
		Delete_1DArray(&pfImg);
		Delete_1DArray(&LabelImage);
		return nCntBlob;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Foot::FindFoot_Pat() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
		Delete_1DArray(&pucImg);
		Delete_1DArray(&pfImg);
		Delete_1DArray(&LabelImage);
	}

	return 0;
}

cv::Point CPInsp_Foot::FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, cv::Mat *cvFindPadImg, cv::Mat *cvFindFootImg)
{
	cv::Point pt;
	return pt;
}


CSearchWire_Model::CSearchWire_Model()
{
	m_pFoot1 = nullptr;
	m_pFoot2 = nullptr;

	m_nWidth = 0;
	m_nHeight = 0;

	m_pPartImageBuffer = nullptr;
	//m_pPartImageBuffer = new InspRoiImgBuf;
	//memset(m_pPartImageBuffer, 0, sizeof(InspRoiImgBuf));

	m_pPart3D = nullptr;
}

CSearchWire_Model::~CSearchWire_Model()
{
	if (m_pFoot1)
	{
		//delete m_pFoot1;
		g_pMManager->pem_delete(m_pFoot1, false);
		m_pFoot1 = nullptr;
	}

	if (m_pFoot2)
	{
		//delete m_pFoot2;
		g_pMManager->pem_delete(m_pFoot2, false);
		m_pFoot2 = nullptr;
	}

	m_nWidth = 0;
	m_nHeight = 0;

	/*if (m_pPartImageBuffer->imgTop_R != NULL) { delete[] m_pPartImageBuffer->imgTop_R; m_pPartImageBuffer->imgTop_R = nullptr; }
	if (m_pPartImageBuffer->imgTop_G != NULL) { delete[] m_pPartImageBuffer->imgTop_G; m_pPartImageBuffer->imgTop_G = nullptr; }
	if (m_pPartImageBuffer->imgTop_B != NULL) { delete[] m_pPartImageBuffer->imgTop_B; m_pPartImageBuffer->imgTop_B = nullptr; }
	if (m_pPartImageBuffer->imgTop_W != NULL) { delete[] m_pPartImageBuffer->imgTop_W; m_pPartImageBuffer->imgTop_W = nullptr; }
	if (m_pPartImageBuffer->imgMiddle_R != NULL) { delete[] m_pPartImageBuffer->imgMiddle_R; m_pPartImageBuffer->imgMiddle_R = nullptr; }
	if (m_pPartImageBuffer->imgMiddle_B != NULL) { delete[] m_pPartImageBuffer->imgMiddle_B; m_pPartImageBuffer->imgMiddle_B = nullptr; }
	if (m_pPartImageBuffer->imgBottom_R != NULL) { delete[] m_pPartImageBuffer->imgBottom_R; m_pPartImageBuffer->imgBottom_R = nullptr; }
	if (m_pPartImageBuffer->imgBottom_B != NULL) { delete[] m_pPartImageBuffer->imgBottom_B; m_pPartImageBuffer->imgBottom_B = nullptr; }

	delete m_pPartImageBuffer;*/
	m_pPartImageBuffer = NULL;

	//삭제하지 말자
	//if (m_pPart3D != NULL) { delete[] m_pPart3D; m_pPart3D = nullptr; }  
}

void CSearchWire_Model::GetImageMatrix(cv::Mat* Image, CFoot_Model* m_pFoot, int footKind)
{
	cv::Rect ImageRect;
	ImageRect.x = 0;
	ImageRect.y = 0;
	ImageRect.width = m_pPartImageBuffer->nImageSizeX;
	ImageRect.height = m_pPartImageBuffer->nImageSizeY;

	GetImageMatrix(Image, m_pFoot, footKind, ImageRect);
}

void CSearchWire_Model::GetImageMatrix(cv::Mat* Image, CFoot_Model* m_pFoot, int footKind, cv::Rect ImageRect)
{
	AlgoBin bin = m_pFoot->m_sArrBin[footKind];
	if (bin.m_bIsSet == false)
		bin = m_pFoot->m_sArrBin[0];

	CPInsp_Foot::GetImageMatrix(Image, bin, m_pPartImageBuffer, ImageRect);
}

void CPInsp_Foot::SetBWForUIImage(CFoot_Model *pFoot, int nSX, int nSY, int nEX, int nEY, cv::Mat* cvFootBW, cv::Mat* cvWedgeBW, cv::Mat* cvWingBW, FootPoins* fPoints)	//UI Image
{
	int nImgHeight = pFoot->GetImageLength();
	int nImgWidth = pFoot->GetImageWidth();
	int imgSize = nImgWidth * nImgHeight;

	try
	{
		pFoot->GetImageMatrix(cvFootBW, (int)m_eFootBin::m_eFootBin_Foot);
		pFoot->GetImageMatrix(cvWedgeBW, (int)m_eFootBin::m_eFootBin_Wedge);
		pFoot->GetImageMatrix(cvWingBW, (int)m_eFootBin::m_eFootBin_Wing);

		UCHAR *pucImg = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg, imgSize);
		memset(pucImg, 0, sizeof(UCHAR) * imgSize);
		
		cv::Mat srcImg1 = *cvWedgeBW;
		cv::Mat srcImg2 = *cvWingBW;

		UCHAR* pucImg1 = NULL;
		UCHAR* pucImg2 = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg1, imgSize);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg2, imgSize);
		memset(pucImg1, 0, sizeof(UCHAR) * imgSize);
		memset(pucImg2, 0, sizeof(UCHAR) * imgSize);

		//set BW Option
		bool bFind2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
		int nFindRange = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nFindRange = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nFindRange = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nFindRange = 3;
		int nFindMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[m_eBin::m_eBin_Min2D];
		int nFindMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[m_eBin::m_eBin_Max2D];
		bool bFind3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
		int nFindRange3D = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nFindRange3D = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nFindRange3D = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nFindRange3D = 3;
		float fFindMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
		float fFindMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];

		//set BW Option - Wedge Type Foot
		bool bWedge2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
		int nWedgeRange = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nWedgeRange = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nWedgeRange = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nWedgeRange = 3;
		int nWedgeMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[m_eBin::m_eBin_Min2D];
		int nWedgeMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[m_eBin::m_eBin_Max2D];
		bool bWedge3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
		int nWedgeRange3D = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nWedgeRange3D = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nWedgeRange3D = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nWedgeRange3D = 3;
		float fWedgeMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_fArrH[m_eMMD::eMMD_Min];
		float fWedgeMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_fArrH[m_eMMD::eMMD_Max];

		bool bWing2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
		int nWingRange = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nWingRange = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nWingRange = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nWingRange = 3;
		int nWingMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[m_eBin::m_eBin_Min2D];
		int nWingMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[m_eBin::m_eBin_Max2D];
		bool bWing3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
		int nWingRange3D = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nWingRange3D = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nWingRange3D = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nWingRange3D = 3;
		float fWingMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_fArrH[m_eMMD::eMMD_Min];
		float fWingMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wing].m_fArrH[m_eMMD::eMMD_Max];
		///////////////////////////////

		for (int y =0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIdx = y * nImgWidth + x;
				float fH = pFoot->pf3D[y * nImgWidth + x];
				bool b2DIn = false;
				//foot binarization
				if (bFind2D)
				{
					UCHAR ucG = cvFootBW->data[y * nImgWidth + x];
					if ((nFindRange == 0 && (nFindMin2D <= ucG && ucG <= nFindMax2D)) ||
						(nFindRange == 1 && (nFindMin2D > ucG || ucG < nFindMax2D)) ||
						(nFindRange == 2 && ucG >= nFindMax2D) ||
						(nFindRange == 3 && nFindMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bFind3D)
					{
						if ((nFindRange3D == 0 && (fFindMin3D <= fH && fH <= fFindMax3D)) ||
							(nFindRange3D == 1 && (fFindMin3D > fH || fH < fFindMax3D)) ||
							(nFindRange3D == 2 && fH >= fFindMax3D) ||
							(nFindRange3D == 3 && fFindMin3D >= fH))
							b2DIn = true;
					}
				}
				if (b2DIn)
					pucImg[nIdx] = 255;

				//wedge binarization
				b2DIn = false;
				if (bWedge2D)
				{
					UCHAR ucG = srcImg1.data[y * nImgWidth + x];
					if ((nWedgeRange == 0 && (nWedgeMin2D <= ucG && ucG <= nWedgeMax2D)) ||
						(nWedgeRange == 1 && (nWedgeMin2D > ucG || ucG < nWedgeMax2D)) ||
						(nWedgeRange == 2 && ucG >= nWedgeMax2D) ||
						(nWedgeRange == 3 && nWedgeMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bWedge3D)
					{
						if ((nWedgeRange3D == 0 && (fWedgeMin3D <= fH && fH <= fWedgeMax3D)) ||
							(nWedgeRange3D == 1 && (fWedgeMin3D > fH || fH < fWedgeMax3D)) ||
							(nWedgeRange3D == 2 && fH >= fWedgeMax3D) ||
							(nWedgeRange3D == 3 && fWedgeMin3D >= fH))
							b2DIn = true;
					}
					else
						b2DIn = true;
				}
				if (b2DIn) pucImg1[nIdx] = 255;

				//wing binarization
				b2DIn = false;
				if (bWing2D)
				{
					UCHAR ucG = srcImg2.data[y * nImgWidth + x];
					if ((nWingRange == 0 && (nWingMin2D <= ucG && ucG <= nWingMax2D)) ||
						(nWingRange == 1 && (nWingMin2D > ucG || ucG < nWingMax2D)) ||
						(nWingRange == 2 && ucG >= nWingMax2D) ||
						(nWingRange == 3 && nWingMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bWing3D)
					{
						if ((nWingRange3D == 0 && (fWingMin3D <= fH && fH <= fWingMax3D)) ||
							(nWingRange3D == 1 && (fWingMin3D > fH || fH < fWingMax3D)) ||
							(nWingRange3D == 2 && fH >= fWingMax3D) ||
							(nWingRange3D == 3 && fWingMin3D >= fH))
							b2DIn = true;
					}
					else
						b2DIn = true;
				}
				if (b2DIn) pucImg2[nIdx] = 255;
			}
		}

		//Remove overlapping area between wing and wedge.
		cv::Mat BinImage1(nImgHeight, nImgWidth, CV_8UC1, pucImg);

		//Make BW Image What Foot 3D height is over 30um
		cv::Mat cvTmpFoot3D(nImgHeight, nImgWidth, CV_32FC1, pFoot->pf3D);

		size_t floatstep_3DCorrect = cvTmpFoot3D.step / sizeof(float);
		for (int y = 0; y < nImgHeight; y++)
		{
			float *rawData = (float*)cvTmpFoot3D.data;
			UCHAR* pFoot2DImg = BinImage1.data;

			for (int x = 0; x < nImgWidth; x++)
			{
				if (rawData[y * floatstep_3DCorrect + x] >= 30)
				{
					pFoot2DImg[y * BinImage1.step + x] = 255;
				}
			}
		}

		cv::Mat BinImage4(nImgHeight, nImgWidth, CV_8UC1);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage1.bmp", BinImage1);
		cv::imwrite("D:\\FootRst\\GetBinImage4.bmp", BinImage4);
#endif
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(BinImage1, BinImage1, kernel);
		cv::dilate(BinImage1, BinImage1, kernel);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Mop1.bmp", BinImage1);
#endif
// 		if (fPoints->bMatchSuccess)
// 		{
		if (pFoot->m_bUse2Foot)
 			cv::bitwise_and(BinImage1, fPoints->cvMatchingMask, BinImage1);
// 		}
		double dCX = 0, dCY = 0, dArea = 0;
		int nMinBlob = 100;

// 		PIAL::PI_Buff src(BinImage1);
// 		PIAL::PI_Buff dst(BinImage4);
// 
// 		int nCntBlob = PIAL::PI_Blob::CalcBlob_Select(&src, &dst, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, _eSelectMix);
// 		
//
// 		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage1.data, BinImage4.data, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, eSelectPosition, fPoints->m_pCenter.x, fPoints->m_pCenter.y);
// 		if (nCntBlob > 0)
// 		{
// 			CRect rcBlob{ 0,0,0,0 };
// 			m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
// 			//PIAL::PI_Blob::BlobGet_Result(&dArea, &dCX, &dCY, &rcBlob, false);
// 		}
// #if _DEBUG
// 		cv::imwrite("D:\\FootRst\\GetBinImage4.bmp", BinImage4);
// #endif
// 
// 		nMinBlob = dArea * 0.1;
// 		if (100 > nMinBlob) nMinBlob = 100;
// 		int selectblob = eSelectMix;
// 		if (!pFoot->m_bUsePatternAngle)
// 			selectblob = eSelectBigger;
//
// 		PIAL::PI_Buff src2(BinImage1);
// 		PIAL::PI_Buff dst2(*cvFootBW);
// 
// 		nCntBlob = PIAL::PI_Blob::CalcBlob_Select(&src2, &dst2, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, _eSelectMix);
		ETypeBlob blobType = eSelectBigger;
		if (/*fPoints->bMatchSuccess &&*/ pFoot->m_bUse2Foot)
			blobType = eSelectMix;

		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage1.data, cvFootBW->data, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, blobType, fPoints->m_pCenter.x, fPoints->m_pCenter.y);
		if (nCntBlob > 0)
		{
			CRect rcBlob{ 0,0,0,0 };
			m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
			//PIAL::PI_Blob::BlobGet_Result(&dArea, &dCX, &dCY, &rcBlob, false);
		}
		//if (fPoints->bMatchSuccess)
		{
			//cv::bitwise_and(*cvFootBW, fPoints->cvMatchingMask, *cvFootBW);
		}

		cv::Mat BinImage2(nImgHeight, nImgWidth, CV_8UC1, pucImg1);
		cv::Mat BinImage3(nImgHeight, nImgWidth, CV_8UC1, pucImg2);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage2.bmp", BinImage2);
		cv::imwrite("D:\\FootRst\\GetBinImage3.bmp", BinImage3);
#endif
		cv::erode(BinImage2, BinImage2, kernel);
		cv::dilate(BinImage2, BinImage2, kernel);
		cv::erode(BinImage3, BinImage3, kernel);
		cv::dilate(BinImage3, BinImage3, kernel);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Mop2.bmp", BinImage2);
		cv::imwrite("D:\\FootRst\\GetBinImage_Mop3.bmp", BinImage3);
#endif
		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIdx = y * nImgWidth + x;
				cvWedgeBW->data[nIdx] = (cvFootBW->data[nIdx] == 255) ? pucImg1[nIdx] : 0;
				cvWingBW->data[nIdx] = (cvFootBW->data[nIdx] == 255) ? BinImage3.data[nIdx] : 0;

				//Wedge BW 티칭된 영상에서 Wing BW 티칭 영역 제외
// 				if (cvWingBW->data[nIdx] == 255)
// 				{
// 					cvWedgeBW->data[nIdx] = 0;
// 				}
			}
		}
#if _DEBUG
		cv::imwrite("D:\\FootRst\\Find_Foot.bmp", *cvFootBW);
		cv::imwrite("D:\\FootRst\\Find_Wedge.bmp", *cvWedgeBW);
		cv::imwrite("D:\\FootRst\\Find_Wing.bmp", *cvWingBW);
#endif
		//cvWedgeBWImg.copyTo(*cvWedgeBW);
		//cvWingBWImg.copyTo(*cvWingBW);

		Delete_1DArray(&pucImg2);
		Delete_1DArray(&pucImg1);

/*		BinImageFoot.copyTo(*cvFootBW);*/
		
		Delete_1DArray(&pucImg);

	}
	catch (const std::exception&)
	{

	}
	

}

int CPInsp_Foot::FootPointByType(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, FootPoins &fPoints, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, std::vector<cv::Mat> &Images, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter)
{
	int ret = 0;
	int nLine = __LINE__;
	try
	{
	}
	catch (const std::exception&)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::FootPointByType error Line: %d"),nLine);
		g_pMPTI->AddLog(msg);
	}
	return ret;
}

cv::Point CPInsp_Foot::FootImageByPoint(CFoot_Model *pFoot, FootPoins fPoints, std::vector<cv::Mat> &Images)
{
	cv::Point Margin = cv::Point(0, 0);
	int nLine = __LINE__;

	if (Images.size() == 0) 
	{
#if FOOT_RST_WRITE
		CString sLog;
		sLog.Format(_T("CPInsp_Foot::FootImageByPoint Images.size() == 0"));
		g_pMPTI->AddLog_OCR(sLog);
#endif
		return 0;
	}

	try
	{
		std::vector< cv::Mat> temp;
		temp.clear();

		for (int i = 0; i < Images.size(); i++)
		{
			temp.push_back(Images[i].clone());
		}


		cv::Point CenterPo(fPoints.m_pCenter.x, fPoints.m_pCenter.y);

		for (int i = 0; i < temp.size(); i++)
		{
			//Margin = WarpAffine(temp[i], Images[i], fPoints.m_dSeta, temp[i].size(), CenterPo);		////Foot Img 회전
			if(i==5)
				continue;
			if (Images[i].type() == CV_8UC1)
			{
				Margin = WarpAffine_IPPRotate(temp[i], Images[i], fPoints.m_dSeta, temp[i].size(), CenterPo);		////Foot Img 회전
			}
			else
			{
				Margin = WarpAffine_IPPRotate_3D(temp[i], Images[i], fPoints.m_dSeta, temp[i].size(), CenterPo);		////Foot Img 회전
			}

		}
	}
	catch (const std::exception&)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::FootImageByPoint error Line: %d"), nLine);
		g_pMPTI->AddLog(msg);
	}
	return Margin;
}
void CPInsp_Foot::DeleteSilk(cv::Mat* Image)
{
	cv::Mat vImage = *Image;
	//delete silk
	cv::Mat Intg, Dev;
	cv::integral(vImage, Intg, Dev);
	int nMaxW(0), stR(0), edR = vImage.rows - 1;

	for (int r = 1; r < Intg.rows; r++)
	{
		int* nPtr_b = Intg.ptr<int>(r - 1);
		int* nPtr = Intg.ptr<int>(r);
		int nW = nPtr[Intg.cols - 1] - nPtr_b[Intg.cols - 1];
		if (nW > nMaxW)
			nMaxW = nW;
	}
	int nMaxHarf = nMaxW / 2;
	int nRCnt(0);
	int r = 1;
	for (; r < Intg.rows; r++)
	{
		int* nPtr_b = Intg.ptr<int>(r - 1);
		int* nPtr = Intg.ptr<int>(r);
		int nW = nPtr[Intg.cols - 1] - nPtr_b[Intg.cols - 1];
		if (nW > nMaxHarf)
		{
			if (nRCnt > 5)
				break;
			else if (nRCnt == 0)
				stR = r - 1;
			nRCnt++;
		}
		else
			nRCnt = 0;
	}
	nRCnt = 0;
	for (; r < Intg.rows; r++)
	{
		int* nPtr_b = Intg.ptr<int>(r - 1);
		int* nPtr = Intg.ptr<int>(r);
		int nW = nPtr[Intg.cols - 1] - nPtr_b[Intg.cols - 1];
		if (nW < nMaxHarf)
		{
			if (nRCnt > 1)
				break;
			else if (nRCnt == 0)
				edR = r - 1;
			nRCnt++;
			break;
		}
	}

	// 	for (int r = 1; r < Intg.rows; r++)
// 	{
// 		int* nPtr_b = Intg.ptr<int>(r - 1);
// 		int* nPtr = Intg.ptr<int>(r);
// 		int nW = nPtr[Intg.cols - 1] - nPtr_b[Intg.cols - 1];
// 		if (nW > nMaxHarf)
// 		{
// 			if (nRCnt > 5)
// 				break;
// 			else if (nRCnt == 0)
// 				stR = r - 1;
// 			nRCnt++;
// 		}
// 		else
// 			nRCnt = 0;
// 	}
// 	nRCnt = 0;
// 	for (int r = Intg.rows - 1; r > 0; r--)
// 	{
// 		int* nPtr_b = Intg.ptr<int>(r - 1);
// 		int* nPtr = Intg.ptr<int>(r);
// 		int nW = nPtr[Intg.cols - 1] - nPtr_b[Intg.cols - 1];
// 		if (nW > nMaxHarf)
// 		{
// 			if (nRCnt > 5)
// 				break;
// 			else if (nRCnt == 0)
// 				edR = r;
// 			nRCnt++;
// 		}
// 		else
// 			nRCnt = 0;
// 	}
	cv::rectangle(vImage, cv::Rect(0, 0, vImage.cols, stR), 0, cv::FILLED);
	cv::rectangle(vImage, cv::Rect(0, edR, vImage.cols, vImage.rows - 1), 0, cv::FILLED);

}

cv::Point CPInsp_Foot::WarpAffine_IPPRotate(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev)
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

cv::Point CPInsp_Foot::WarpAffine_IPPRotate_3D(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev)
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

	bool bLinear = true;
	bool bColor = false;

	bool bUseOrgSize = true;

	int retRotateDstSizeX, retRotateDstSizeY;
	//unsigned char* ucWing2DSrc;
	//m_procFoot->RotateImg_ipp2020_2(Img.data, &ucWing2DSrc, Seta, Img.cols, Img.rows, &retRotateDstSizeX, &retRotateDstSizeY, bColor, bLinear, bUseOrgSize, NCenter);
	//cv::Mat cvRoWing2DImg_ipp(retRotateDstSizeY, retRotateDstSizeX, CV_8UC1, ucWing2DSrc);

	float *fHRoFoot3DImg_ipp = NULL;

	m_pProcMilAlgo->RotateZmap_ipp2020_2(Img.ptr<float>(0), &fHRoFoot3DImg_ipp, Seta, Img.cols, Img.rows, &retRotateDstSizeX, &retRotateDstSizeY, bLinear, bUseOrgSize, NCenter);

	cv::Mat RotImage(retRotateDstSizeY, retRotateDstSizeX, Img.type());
	memcpy(RotImage.data, fHRoFoot3DImg_ipp, sizeof(float)*retRotateDstSizeX*retRotateDstSizeY);

	Delete_1DArray(&fHRoFoot3DImg_ipp);

	//cv::Mat RotImage2;
	//cv::Mat rotation = getRotationMatrix2D(NCenter, Seta, 1);

	//if (CV_MAT_TYPE(orgImg.type()) == CV_32FC1)
	//	cv::warpAffine(Img, RotImage2, rotation, Img.size(), cv::INTER_LINEAR);
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

cv::Mat CPInsp_Foot::calcRoundRect(cv::Mat insp, int nRadius, cv::Rect& bMaxbox)
{

	int nHeight = insp.rows;
	int nWidth = insp.cols;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	double dMxArea = 0;
	int idx = 0;

	cv::RotatedRect MaxRoRe;
	contours.clear();
	hierarchy.clear();
	cv::findContours(insp, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	dMxArea = 0;
	idx = 0;
	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);
		double area = cv::contourArea(contours[idx]);
		cv::RotatedRect RoRe = cv::minAreaRect(contours[idx]);
		if (area > dMxArea)
		{
			dMxArea = area;
			bMaxbox = bbox;
			MaxRoRe = RoRe;
		}
	}
	MaxRoRe.center.x -= (bMaxbox.x + 1);
	MaxRoRe.center.y -= (bMaxbox.y + 1);
	return DrawRoundPad(bMaxbox.width + 2, bMaxbox.height + 2, MaxRoRe, nRadius);
}

cv::Mat CPInsp_Foot::calcRoundRect_2(cv::Mat insp, int nRadius, cv::Rect& bMaxbox)
{

	int nHeight = insp.rows;
	int nWidth = insp.cols;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	double dMxArea = 0;
	int idx = 0;

	cv::RotatedRect MaxRoRe;
	contours.clear();
	hierarchy.clear();
	cv::findContours(insp, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	dMxArea = 0;
	idx = 0;
	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);
		double area = cv::contourArea(contours[idx]);
		cv::RotatedRect RoRe = cv::minAreaRect(contours[idx]);
		if (area > dMxArea)
		{
			dMxArea = area;
			bMaxbox = bbox;
			MaxRoRe = RoRe;
		}
	}
	MaxRoRe.center.x -= (bMaxbox.x);
	MaxRoRe.center.y -= (bMaxbox.y);
	return DrawRoundPad(bMaxbox.width, bMaxbox.height, MaxRoRe, nRadius);
}

void CPInsp_Foot::calcLine(cv::Mat insp, cv::Point pSt, cv::Point pEd, float* a, float* b, float fX, float fY)
{
	try
	{
		int sumX(0), sumY(0);
		std::vector<cv::Point> vecCenterPo;
		int nSty = std::min(pSt.y - 1, pEd.y - 1) > 0 ? std::min(pSt.y - 1, pEd.y - 1) : 0;
		int nEdy = std::max(pSt.y + 1, pEd.y + 1) > insp.rows - 1 ? insp.rows - 1 : std::max(pSt.y + 1, pEd.y + 1);
		int nStx = std::min(pSt.x - 1, pEd.x - 1) > 0 ? std::min(pSt.x - 1, pEd.x - 1) : 0;
		int nEdx = std::max(pSt.x + 1, pEd.x + 1) > insp.cols - 1 ? insp.cols - 1 : std::max(pSt.x + 1, pEd.x + 1);

		for (int y = nSty; y <= nEdy; y++)
		{
			uchar* ptr = insp.ptr(y);
			for (int x = nStx; x < nEdx; x++)
			{
				if (ptr[x] > 0)
				{
					vecCenterPo.push_back(cv::Point(x + fX,y + fY));
					sumX += x + fX;
					sumY += y + fY;
				}
			}
		}
		int cnt = vecCenterPo.size();
		cv::Point* WhitPos = new cv::Point[cnt];
		for (int i = 0; i < vecCenterPo.size(); i++)
		{
			WhitPos[i] = vecCenterPo[i];
		}
		cv::Point CenterPo;
		CenterPo.x = sumX / cnt;
		CenterPo.y = sumY / cnt;
		PCA_1D(cnt, WhitPos, CenterPo, a, b);
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
}
cv::Mat CPInsp_Foot::DrawRoundPad(int nImgWidth, int nImgHeight, cv::RotatedRect RoRe, int nRadius)
{
	std::vector<cv::Point> vRoRectPo;
	cv::Point2f P2f[4];
	RoRe.points(P2f);
	vRoRectPo.clear();
	vRoRectPo.push_back(P2f[0]);
	vRoRectPo.push_back(P2f[1]);
	vRoRectPo.push_back(P2f[2]);
	vRoRectPo.push_back(P2f[3]);

	std::vector<std::vector<cv::Point>> vvRoRectPo2;
	vvRoRectPo2.push_back(vRoRectPo);

	//sort
	for (int i = 0; i < 4; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			if (P2f[i].y > P2f[j].y)
			{
				cv::Point pTemp = P2f[i];
				P2f[i] = P2f[j];
				P2f[j] = pTemp;
			}
		}
	}
	if (P2f[0].x > P2f[1].x)
	{
		cv::Point pTemp = P2f[0];
		P2f[0] = P2f[1];
		P2f[1] = pTemp;
	}
	if (P2f[2].x > P2f[3].x)
	{
		cv::Point pTemp = P2f[2];
		P2f[2] = P2f[3];
		P2f[3] = pTemp;
	}

	float fTopA(0), fTopB(0), fBottomA(0), fBottomB(0), fLeftA(0), fLeftB(0), fRightA(0), fRightB(0);
	float fTTop(0), fTBot(0), fTLeft(0), fTRight(0);
	fTopA = (P2f[1].y - P2f[0].y) / (P2f[1].x - P2f[0].x);
	fTopB = P2f[0].y - fTopA * P2f[0].x;

	fBottomA = (P2f[3].y - P2f[2].y) / (P2f[3].x - P2f[2].x);
	fBottomB = P2f[2].y - fTopA * P2f[2].x;

	fLeftA = (P2f[2].x - P2f[0].x) == 0 ? 0 : (P2f[2].y - P2f[0].y) / (P2f[2].x - P2f[0].x);
	fLeftB = P2f[0].y - fTopA * P2f[0].x;

	fRightA = (P2f[3].y - P2f[1].y) / (P2f[3].x - P2f[1].x);
	fRightB = P2f[1].y - fTopA * P2f[1].x;

	fTTop = std::atan2f((P2f[1].y - P2f[0].y), (P2f[1].x - P2f[0].x));
	fTBot = std::atan2f((P2f[3].y - P2f[2].y), (P2f[3].x - P2f[2].x));
	fTLeft = std::atan2f((P2f[2].y - P2f[0].y), (P2f[2].x - P2f[0].x));
	fTRight = std::atan2f((P2f[3].y - P2f[1].y), (P2f[3].x - P2f[1].x));

	cv::Point pTopLeft, pLeftTop, pTopRight, pRightTop, pBottomLeft, pLeftBottom, pBottomRight, pRightBottom;

	float fTopDiffY = nRadius;//fTTop < 0.001 ? nRadius : std::tan(fTTop)*nRadius;
	float fTopDiffX = nRadius;//fTTop < 0.001 ? nRadius : (1 / std::tan(fTTop))*nRadius;

	pTopLeft.x = P2f[0].x + fTopDiffX;
	pTopLeft.y = P2f[0].y /*+ fTopDiffY*/;
	pTopRight.x = P2f[1].x - fTopDiffX;
	pTopRight.y = P2f[1].y /*- fTopDiffY*/;

	float fBotDiffY = nRadius;//fTBot < 0.001 ? nRadius : std::tan(fTBot)*nRadius;
	float fBotDiffX = nRadius;//fTBot < 0.001 ? nRadius : (1 / std::tan(fTBot))*nRadius;

	pBottomLeft.x = P2f[2].x + fBotDiffX;
	pBottomLeft.y = P2f[2].y /*+ fBotDiffY*/;
	pBottomRight.x = P2f[3].x - fBotDiffX;
	pBottomRight.y = P2f[3].y/* - fBotDiffY*/;

	float fLeftDiffY = nRadius;//MRad_deg90(fTLeft) ? nRadius : std::tan(fTLeft)*nRadius;
	float fLeftDiffX = nRadius;//MRad_deg90(fTLeft) ? nRadius : (1 / std::tan(fTLeft))*nRadius;

	pLeftTop.x = P2f[0].x/* + fLeftDiffX*/;
	pLeftTop.y = P2f[0].y + fLeftDiffY;
	pLeftBottom.x = P2f[2].x/* + fLeftDiffX*/;
	pLeftBottom.y = P2f[2].y - fLeftDiffY;

	float fRightDiffY = nRadius;//MRad_deg90(fTRight) ? nRadius : std::tan(fTRight)*nRadius;
	float fRightDiffX = nRadius;//MRad_deg90(fTRight) ? nRadius : (1 / std::tan(fTRight))*nRadius;

	pRightTop.x = P2f[1].x/* + fRightDiffX*/;
	pRightTop.y = P2f[1].y + fRightDiffY;
	pRightBottom.x = P2f[3].x/* + fRightDiffX*/;
	pRightBottom.y = P2f[3].y - fRightDiffY;

	cv::Mat mRstImage(nImgHeight, nImgWidth, CV_8UC1);
	mRstImage.setTo(0);

	std::vector<cv::Point> vRstRectPo;
	std::vector<std::vector<cv::Point>> vvRstRectPo;
	vRstRectPo.push_back(pTopLeft);
	vRstRectPo.push_back(pTopRight);
	vRstRectPo.push_back(pBottomRight);
	vRstRectPo.push_back(pBottomLeft);
	vvRstRectPo.push_back(vRstRectPo);
	cv::fillPoly(mRstImage, vvRstRectPo, cv::Scalar(255));
	vvRstRectPo.clear();
	vRstRectPo.clear();
	vRstRectPo.push_back(pLeftTop);
	vRstRectPo.push_back(pRightTop);
	vRstRectPo.push_back(pRightBottom);
	vRstRectPo.push_back(pLeftBottom);
	vvRstRectPo.push_back(vRstRectPo);
	cv::fillPoly(mRstImage, vvRstRectPo, cv::Scalar(255));

	cv::Point LtCent = cv::Point(pTopLeft.x, pLeftTop.y);
	cv::Point RtCent = cv::Point(pTopRight.x, pRightTop.y);
	cv::Point LbCent = cv::Point(pBottomLeft.x, pLeftBottom.y);
	cv::Point RbCent = cv::Point(pBottomRight.x, pRightBottom.y);
	if (nRadius > 0) 
	{
		cv::Size ElipseSz = cv::Size(nRadius - 1, nRadius - 1);
		float fAngle = fTTop * (180 / PI);
		cv::ellipse(mRstImage, LtCent, ElipseSz, fAngle, 180, 270, cv::Scalar(255), cv::FILLED);
		cv::ellipse(mRstImage, RtCent, ElipseSz, fAngle, 270, 360, cv::Scalar(255), cv::FILLED);
		cv::ellipse(mRstImage, LbCent, ElipseSz, fAngle, 90, 180, cv::Scalar(255), cv::FILLED);
		cv::ellipse(mRstImage, RbCent, ElipseSz, fAngle, 0, 90, cv::Scalar(255), cv::FILLED);
	}

	return mRstImage;
}
cv::Mat CPInsp_Foot::MatPolygon(cv::Mat mImage, cv::Mat PolyImg)
{
	cv::Mat MatchTempl;
	cv::matchTemplate(mImage, PolyImg, MatchTempl, cv::TM_CCOEFF_NORMED);
	double dMin, dMax;
	cv::Point pMin, pMax;
	cv::minMaxLoc(MatchTempl, &dMin, &dMax, &pMin, &pMax);

	std::vector<std::vector<cv::Point>> contours, Polycontours;
	std::vector<cv::Vec4i> hierarchy, Polyhierarchy;
	cv::findContours(mImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	cv::findContours(PolyImg, Polycontours, Polyhierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	std::vector<double> dScore;
	for (int i = 0; i < contours.size(); i++)
	{
		double sc = cv::matchShapes(contours[i], Polycontours[0], cv::CONTOURS_MATCH_I1, 0.0f);
		dScore.push_back(sc);
	}

	double dMatchsc(1);
	int nMatchIdx(0);

	for (int i = 0; i < dScore.size(); i++)
	{
		if (dScore[i] < dMatchsc)
		{
			dMatchsc = dScore[i];
			nMatchIdx = i;
		}
	}

	cv::Mat rstMat(mImage.rows, mImage.cols, CV_8UC1);
	rstMat.setTo(0);

	if (contours.size() == 0)
	{
		return rstMat;
	}

	if ( nMatchIdx+1 > contours.size())
	{
		return rstMat;
	}

	cv::RotatedRect RoRe = cv::minAreaRect(contours[nMatchIdx]);

	int nAngle = 0;
	nAngle = (int)RoRe.angle;

	double dAngleP180 = std::abs(nAngle) % 180;
	cv::RotatedRect RstRoRe;
	if (dAngleP180 > 75 && dAngleP180 < 105)
	{
		RstRoRe = RoRe;
		RstRoRe.angle = RoRe.angle - 90;
	}

	cv::Mat mRoPolyImg;
	Polycontours.clear();
	WarpAffine_IPPRotate(PolyImg, mRoPolyImg, -RstRoRe.angle, PolyImg.size(), cv::Point(PolyImg.cols / 2, PolyImg.rows / 2));
	cv::findContours(mRoPolyImg, Polycontours, Polyhierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	cv::Rect rPolyRect = boundingRect(Polycontours[0]);
	cv::Rect rTargetRect = boundingRect(contours[nMatchIdx]);
	rTargetRect.x = pMax.x;
	rTargetRect.y = pMax.y;
	rTargetRect.width = rPolyRect.width;
	rTargetRect.height = rPolyRect.height;
	
	cv::Mat rstClip = rstMat(rTargetRect);
	cv::Mat mMatchClip = mRoPolyImg(rPolyRect);

	for (int r = 0; r < mMatchClip.rows; r++)
	{
		if ((r + rTargetRect.y)*(rTargetRect.x + mMatchClip.cols) >= rstMat.rows * rstMat.cols)
			break;
		uchar* TarPtr = mMatchClip.ptr(r);
		uchar* rstPtr = &rstMat.ptr(r + rTargetRect.y)[rTargetRect.x];
		memcpy(rstPtr, TarPtr, sizeof(uchar) * mMatchClip.cols);
	}

	return rstMat;
}

cv::Mat CPInsp_Foot::CalcOnlyPadArea(CFoot_Model* pFoot, int nWid, int nLen, bool bIsDBCArea)
{
	cv::Mat cvOnlyPadBW(nLen, nWid, CV_8UC1);
	cvOnlyPadBW.setTo(0);

	int wid = nWid;
	int len = nLen;
	float fPadAvg3DHeight = 0;		//빈도수가 가장 많은 높이

	//1. Foot 3D 가우시안 필터링
	cv::Mat cvOrgFoot3D(len, wid, CV_32FC1, pFoot->pf3D);
	cv::Mat cvTmpFoot3D = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

	if (bIsDBCArea)
		cv::GaussianBlur(cvOrgFoot3D, cvTmpFoot3D, cv::Size(11, 11), 0);
	else
		cvTmpFoot3D = cvOrgFoot3D.clone();

	//2. Foot 3D 기울기 X,Y 계산
	cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

	size_t floatstep_3DCorrect = cvTmpFoot3D.step / sizeof(float);
	for (int y = 1; y < len - 1; y++)
	{
		float * pXgrad = (float *)Xgrad.data;
		float * pYgrad = (float *)Ygrad.data;
		float *rawData = (float*)cvTmpFoot3D.data;

		for (int x = 1; x < wid - 1; x++)
		{
			pXgrad[y*floatstep_3DCorrect + x] = rawData[y * floatstep_3DCorrect + (x + 1)] - rawData[y * floatstep_3DCorrect + (x - 1)];
			pYgrad[y*floatstep_3DCorrect + x] = rawData[(y + 1) * floatstep_3DCorrect + x] - rawData[(y - 1) * floatstep_3DCorrect + x];
		}
	}

	//3. 2D - Top White 조명에서 grayLevel이 180 이상인 영역 & 3D - 가우시안 필터 적용한 3D 중 기울기 10 이하인 영역만 추출
	cv::Mat tempImage(len, wid, CV_8UC1);
	tempImage.setTo(0);

	cv::Mat Rst3DImage = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	float fMaxHeight = 0;
	float fMinHeight = 10000;

	float fRefAreaHgt = 0.0f;
	int nRefAreaCnt = 0;

	cv::Mat DBCBinImage(len, wid, CV_8UC1);
	DBCBinImage.setTo(0);
	if (bIsDBCArea)
	{
		bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

		///////////////////////
		//if (bIsFindDBCOutLine)
		//	PadMaskCalc = PadMaskCalc + DBCBinImage;
	}

	for (int i = 0; i < len; i++)
	{
		float * pXgrad = (float*)Xgrad.data;
		float * pYgrad = (float*)Ygrad.data;

		UCHAR* pResult2D = tempImage.data;

		UCHAR* pDBCSilkLines = DBCBinImage.data;

		float* rowptr = (float*)cvTmpFoot3D.data;
		float* ptrRst3D = (float*)Rst3DImage.data;

		for (int j = 0; j < wid; j++)
		{
			float val = rowptr[i * floatstep_3DCorrect + j];

			if (bIsDBCArea)
			{
				if (/*pDBCSilkLines[i * DBCBinImage.step + j] == 0 &&*/
					((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 10 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 10) && val < 50)
				{
					if (isnan(val) == true)
						continue;

					if (rowptr[i * floatstep_3DCorrect + j] > 100 || rowptr[i * floatstep_3DCorrect + j] < -100)
						continue;

					ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
					pResult2D[i * tempImage.step + j] = 255;		//2D

					if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
						fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
					if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
						fMinHeight = rowptr[i * floatstep_3DCorrect + j];

				}
			}
			else
			{
				if ( ((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 5 && (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 5) && val < 50)
				{
					if (isnan(val) == true)
						continue;

					ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
					pResult2D[i * tempImage.step + j] = 255;		//2D

					if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
						fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
					if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
						fMinHeight = rowptr[i * floatstep_3DCorrect + j];

				}
			}
		}

	}

	cv::Mat grayImg = Rst3DImage.clone();

	//4. 위의 두 조건을 만족하는 영역 중에서도, 최빈도 높이 값의 ±20 범위에 드는 부분을 최종 Background 로 추출
	int nTolerance = 50;
	float fMaxFeqHeight = GetHeightMaxHist(grayImg.ptr<float>(), wid, len, fMaxHeight, fMinHeight, bIsDBCArea);

	//cv::Mat cvAutoInsp_pad2D(cvTmpFoot3D.rows, cvTmpFoot3D.cols, CV_8UC1);
	//cvAutoInsp_pad2D.setTo(0);

	std::vector< jsl::Point3d<double, double, double> > dataList;
	dataList.reserve(wid*len);
	jsl::LeastSquare lsCalc;

	if (bIsDBCArea)
	{
		tempImage += DBCBinImage;
	}

	cv::Mat cvTmpCalcFoot3D(len, wid, CV_32FC1, pFoot->pf3D);
	for (int i = 0; i < len; i++)
	{
		UCHAR* pPad2DImg = tempImage.data;
		UCHAR* pRstPad2DImg = cvOnlyPadBW.data;
		float* rowptr = (float*)cvTmpCalcFoot3D.data;

		for (int j = 0; j < wid; j++)
		{
			int val = rowptr[i * floatstep_3DCorrect + j];

			{
				if (fMaxFeqHeight - nTolerance < val && val < fMaxFeqHeight + nTolerance && pPad2DImg[i * tempImage.step + j] == 255)
				{
					pRstPad2DImg[i * cvOnlyPadBW.step + j] = 255;

				}
			}

		}
	}

	return cvOnlyPadBW;

}


bool CPInsp_Foot::FindFoot_3DBlob(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, FootPoins &fPoints, int nfootKind, cv::Mat *cv3DAvgFilter, cv::Mat &RoRectimage, CString sSaveDebugPath, bool bUseFootOnly2D, bool bRotateOrgImage, bool bFstChk)
{

	int nWidth = pFoot->GetImageWidth();		//Foot Search Area 
	int nHeight = pFoot->GetImageLength();

	int nNormalize3D_MaxH = 320;
	float fRemoveWireHeight = 0;
	fRemoveWireHeight = (float)nNormalize3D_MaxH / (float)255;

	int nInputRemoveWireH = 230;
	// 	if (pFoot->nRemoveWireHeight > 0)		//User Teaching
	// 	{
	// 		nInputRemoveWireH = pFoot->nRemoveWireHeight;
	// 	}

	fRemoveWireHeight = (float)nInputRemoveWireH / fRemoveWireHeight;

	bool bIsDBCFoot = false;
	if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		bIsDBCFoot = true;

	cv::Mat Nomal, Image8U;
	Image8U = (*cv3DAvgFilter).clone();

#ifdef _DEBUG
	{
		//3D data
		cv::Mat cv3DArea(nHeight, nWidth, CV_32FC1, pFoot->pf3D);

		float *fp2;
		fp2 = (float*)cv3DArea.data;

		HEADER_PTR hPtr;
		hPtr.uiNumRow = nWidth;
		hPtr.uiNumCol = nHeight;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\Eagle3D_data\\3D_Foot.ptr"), &hPtr, fp2);
	}
#endif

	if (pFoot->m_bUse2Foot)
		bUseFootOnly2D = true;

	//Remove values ​​with 3D height of 0 or negative
	float fMaxHgt = 0;
	size_t floatstep_3DCorrect = Image8U.step / sizeof(float);
	for (int i = 0; i < Image8U.rows; i++)
	{
		float* rowptr = (float*)Image8U.data;
		for (int j = 0; j < Image8U.cols; j++)
		{
			int val = rowptr[i * floatstep_3DCorrect + j];
			//if (val <= 0 && val < nMinHeight)
			if (val <= 0)
			{
				val = 0;
				rowptr[i * floatstep_3DCorrect + j] = 0;
			}
			if (fMaxHgt < val)
				fMaxHgt = val;
		}
	}

	//정규화 할 때, 모델을 최대높이 nNormalize3D_MaxH(320)이 255값이 되도록 해서 정규화
	for (int i = 0; i < Image8U.rows; i++)
	{
		float* Ptr = (float*)Image8U.data;

		for (int j = 0; j < Image8U.cols; j++)
		{
			int val = Ptr[i * floatstep_3DCorrect + j];
			if (fMaxHgt <= nNormalize3D_MaxH && i == 0 && j == 0)
			{
				Ptr[i * floatstep_3DCorrect + j] = nNormalize3D_MaxH;
				break;
			}
		}
	}

	for (int i = 0; i < Image8U.rows; i++)
	{
		float* rowptr = (float*)Image8U.data;
		for (int j = 0; j < Image8U.cols; j++)
		{
			int val = rowptr[i * floatstep_3DCorrect + j];
			if (val > nNormalize3D_MaxH && fMaxHgt != 0)
			{
				rowptr[i * floatstep_3DCorrect + j] = nNormalize3D_MaxH;
			}
		}
	}

	cv::normalize(Image8U, Nomal, 0, 255, cv::NORM_MINMAX);
	Nomal.convertTo(Image8U, CV_8UC1);		//정규화

	cv::Mat cvTmpImgForDebug(nHeight, nWidth, CV_8UC1);
	cvTmpImgForDebug = Image8U.clone();

	//정규화 된 Foot에서 Wire 영역(230이상) 제거
	for (int i = 0; i < Image8U.rows; i++)
	{
		UCHAR* Ptr = Image8U.data;

		for (int j = 0; j < Image8U.cols; j++)
		{
			int val = Ptr[i * Image8U.step + j];
			if (i == 0 && j == 0)
			{
				Ptr[i * Image8U.step + j] = 0;
			}
			if (val > fRemoveWireHeight)
				Ptr[i * Image8U.step + j] = 0;
		}
	}

	//Blob 필터링
	AlgoBlob algoBlob;
	algoBlob.m_bUseIPC = false;
	algoBlob.m_bInvertCheck = false;
	algoBlob.m_bFilterIsUse = false;
	algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

	algoBlob.m_bInsp3D = false;
	algoBlob.m_nTypeRange3D = 2;
	algoBlob.m_dHeightRateMin = 0;
	algoBlob.m_dHeightRateMax = 0;

	algoBlob.m_bInsp2D = true;
	algoBlob.m_nTypeRange2D = 2;	//lower
	algoBlob.m_nMinBinary = 0;
	algoBlob.m_nMaxBinary = 1;
	algoBlob.m_bFillHole = true;

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;

	double dCX = 0, dCY = 0, dArea = 0;
	CRect rcBlob;
	rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

	cv::Mat Img3D_Candidates(Image8U.rows, Image8U.cols, CV_32FC1);
	cv::Mat BinImage(Image8U.rows, Image8U.cols, CV_8UC1);

	int nCntBlob = CPInsp::BlobImageStruct(algoBlob, Image8U.data, Img3D_Candidates.ptr<float>(0), NULL, Image8U.cols, Image8U.rows, 3000, &dArea, &dCX, &dCY, &rcBlob,
		BinImage.data, stTieAreaNULL, true);

	if (nCntBlob <= 0)
		return false;

	if (pFoot->m_bUse2Foot)
	{
		if (nCntBlob < 2)
		{
			CString cstrInspLog;
			cstrInspLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_ChkFootBlobCnt_2Foot : Fail"));
			g_pMPTI->AddLog(cstrInspLog);

			return false;
		}

		int nTypeSelectBlobTmp = ETypeBlob::eSelectPosition;

		int nCntFootBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage.ptr(), BinImage.ptr(), BinImage.cols, BinImage.rows, 4, FALSE, FALSE, FALSE, nTypeSelectBlobTmp, pFoot->m_FootCenterPoint_ImageRect.x, pFoot->m_FootCenterPoint_ImageRect.y);

		if (nCntFootBlob == 0)
		{
			CString cstrInspLog;
			cstrInspLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_ChkFootBlobCnt_2Foot_Select1Foot : Fail"));
			g_pMPTI->AddLog(cstrInspLog);

			return false;
		}
	}
	else
	{
		// 1 Foot은 Blob 개수가 하나 이상 나올 경우 NG
		if (nCntBlob > 1)
		{
			CString cstrInspLog;
			cstrInspLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_ChkFootBlobCnt_1Foot : Fail"));
			g_pMPTI->AddLog(cstrInspLog);

			return false;
		}
	}

	cv::Mat vMorpImg;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
	int nMorpologyCnt = 3;
	{
		cv::erode(BinImage, vMorpImg, kernel);
		cv::erode(vMorpImg, vMorpImg, kernel);
		cv::erode(vMorpImg, vMorpImg, kernel);
	}

	int nTypeSelectBlob = ETypeBlob::eSelectBigger;
	int nMinFootBlobPx = 50;
	int nCntBlob_AfterFootMorp = m_pProcMilAlgo->CalcBlob_Select(vMorpImg.ptr(), vMorpImg.ptr(), vMorpImg.cols, vMorpImg.rows, nMinFootBlobPx, FALSE, FALSE, FALSE, nTypeSelectBlob, pFoot->m_FootCenterPoint_ImageRect.x, pFoot->m_FootCenterPoint_ImageRect.y);

	if (nCntBlob_AfterFootMorp <= 0)
	{
		CString cstrInspLog;
		cstrInspLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_ChkFootBlobCnt_AfterMorp : Fail"));
		g_pMPTI->AddLog(cstrInspLog);

		return false;

	}

	double setaFoot = 0;

	if (nCntBlob > 0)
	{

#pragma region 1.Find Foot and Calculate Foot Mask Area  (theta, Center)

		//calculate Foot theta
		cv::Mat result_LineImg(vMorpImg.size(), CV_8UC1, cv::Scalar(0));
		cv::Point BSt1, BEd1;
		cv::Point poFootCenter_UseBlob;
		bool bFindFootLines = FindFootAngle_UseBlobLine(pFoot, &result_LineImg, &setaFoot, &BSt1, &BEd1, &poFootCenter_UseBlob, vMorpImg, vMorpImg.cols, vMorpImg.rows);
		if (bFindFootLines == false)
		{
			CString sLog;
			sLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_not found Foot blob lines"));
			g_pMPTI->AddLog(sLog);

			return false;
		}

		//calculate Foot Center Position
		cv::Mat FootImg = BinImage.clone();
		int nFootCtrX = 0, nFootCtrY = 0;
		nFootCtrX = poFootCenter_UseBlob.x;
		nFootCtrY = poFootCenter_UseBlob.y;
		if (nFootCtrX == 0 || nFootCtrY == 0)
		{
			nFootCtrX = (int)(FootImg.cols / 2);
			nFootCtrY = (int)(FootImg.rows / 2);
		}
		fPoints.m_pCenter.x = nFootCtrX;
		fPoints.m_pCenter.y = nFootCtrY;

		cv::Point CenterPos(nFootCtrX, nFootCtrY);

		cv::Mat FootMorpRotImg;
		if (nMorpologyCnt == 3)
		{
			cv::dilate(vMorpImg, vMorpImg, kernel);
			cv::dilate(vMorpImg, vMorpImg, kernel);
			cv::dilate(vMorpImg, vMorpImg, kernel);
		}

		//Rotate Foot BW Image
		cv::Point FootMargin = WarpAffine_IPPRotate(vMorpImg, FootMorpRotImg, setaFoot, vMorpImg.size(), CenterPos);		//Foot Img 회전

		//Rotate Foot Image (Corrected 3D)
		cv::Mat cvCorrect3dFoot(FootMorpRotImg.rows, FootMorpRotImg.cols, CV_32FC1, pFoot->pf3D);
		cv::Mat cvRoCorrect3dFoot;
		cv::Point Foot3DMargin = WarpAffine_IPPRotate_3D(cvCorrect3dFoot, cvRoCorrect3dFoot, setaFoot, cvCorrect3dFoot.size(), CenterPos);		//Foot 3D Img 회전

		//Calc Rect Area _ Rotate Foot (차렷자세)
		int nRoFootBlobMinX = FootMorpRotImg.cols;
		int nRoFootBlobMaxX = 0;
		int nRoFootBlobMinY = FootMorpRotImg.rows;
		int nRoFootBlobMaxY = 0;

		for (int r = 0; r < FootMorpRotImg.rows; r++)
		{
			UCHAR* Ptr = FootMorpRotImg.data;
			for (int c = 0; c < FootMorpRotImg.cols; c++)
			{
				if (Ptr[r * FootMorpRotImg.step + c] == 255)
				{
					if (r < nRoFootBlobMinY)	nRoFootBlobMinY = r;
					if (r > nRoFootBlobMaxY)	nRoFootBlobMaxY = r;
					if (c < nRoFootBlobMinX)	nRoFootBlobMinX = c;
					if (c > nRoFootBlobMaxX)	nRoFootBlobMaxX = c;

				}
			}
		}

		//Foot Blob 의 X direction CenterLine 확인
		cv::Mat	 cvImgFootCtLine(vMorpImg.rows, vMorpImg.cols, CV_8UC1);
		cvImgFootCtLine.setTo(0);

		int nMediumStPo = 0, nXDirForegroundCnt = 0;
		int nCntLineLength = 0;
		int nLineStPosX = vMorpImg.cols, nLineEdPosX = 0;
		int nLineStPosY = vMorpImg.cols, nLineEdPosY = 0;

		std::vector<cv::Point> vecLinePosList;
		vecLinePosList.clear();

		for (int y = 0; y < vMorpImg.rows; y++)
		{
			UCHAR* pRstImg = cvImgFootCtLine.data;

			UCHAR* pRawInput = FootMorpRotImg.data;
			float* rowptr = (float*)cvRoCorrect3dFoot.data;

			nMediumStPo = 0;
			nXDirForegroundCnt = 0;

			for (int x = 0; x < vMorpImg.cols; x++)
			{
				float fWhiteVal = pRawInput[y * FootMorpRotImg.step + x];

				if (fWhiteVal == 255)
				{
					if (nMediumStPo == 0)
						nMediumStPo = x;

					nXDirForegroundCnt++;
				}
			}

			if (nXDirForegroundCnt > 5)
				nCntLineLength++;

			if (nXDirForegroundCnt > 5 && nCntLineLength > 20 && y < (nRoFootBlobMaxY - 20) && (nMediumStPo + (int)(nXDirForegroundCnt / 2)) < vMorpImg.cols)
			{
				pRstImg[y * cvImgFootCtLine.step + (nMediumStPo + (int)(nXDirForegroundCnt / 2))] = 255;

				vecLinePosList.push_back(cv::Point(nMediumStPo + (int)(nXDirForegroundCnt / 2), y));

				if (nLineStPosX >= (nMediumStPo + (int)(nXDirForegroundCnt / 2)))
				{
					nLineStPosX = (nMediumStPo + (int)(nXDirForegroundCnt / 2));
					nLineStPosY = y;
				}
				if (nLineEdPosX <= (nMediumStPo + (int)(nXDirForegroundCnt / 2)))
				{
					nLineEdPosX = (nMediumStPo + (int)(nXDirForegroundCnt / 2));
					nLineEdPosY = y;
				}
			}
		}

		//CenterLine이 휘어졌는지 판단하는 구문
		bool bIsWarpedWire = false;
		int dxSum = 0;

		if (vecLinePosList.size() > 0)
		{
			for (int i = 3; i < vecLinePosList.size(); i++)
			{
				dxSum = dxSum + abs((int)(vecLinePosList[i].x - vecLinePosList[i - 3].x));
			}
		}

		if (dxSum != 0 && dxSum > 100)
		{
			bIsWarpedWire = true;
		}

		//CenterLine 휘어진 경우, 허프 Line 검출로 각도 계산
		double dRstTheta_UseHoughline = 0;
		cv::Point CenterPo_WarpWire;
		bool bFoundHoughLine_inWarpedLine = false;
		if (bIsWarpedWire)
		{
			bool bIsFound = Find_LineAngle_houghline(pFoot, &cvImgFootCtLine, dRstTheta_UseHoughline);

			if (bIsFound && dRstTheta_UseHoughline != 0)
			{
				bFoundHoughLine_inWarpedLine = true;

				cv::Mat cvRoFoot_UseHough(FootMorpRotImg.rows, FootMorpRotImg.cols, CV_8UC1);
				cv::Point FootBWMarginss = WarpAffine_IPPRotate(FootMorpRotImg, cvRoFoot_UseHough, dRstTheta_UseHoughline, cvRoFoot_UseHough.size(), CenterPos);		//Foot Img 회전

				////////////////////////////////////////////////////////////////////////////////////
				if (abs(dRstTheta_UseHoughline) > 5 && abs(dRstTheta_UseHoughline) < 80)
				{
					//찾은 휘어진 와이어 부분 제거
					for (int i = 0; i < vMorpImg.rows; i++)
					{
						UCHAR* pRawInput = FootMorpRotImg.data;

						for (int j = 0; j < vMorpImg.cols; j++)
						{
							if (pRawInput[i * vMorpImg.step + j] == 255)
							{
								if (i > nLineEdPosY && bIsWarpedWire/*nLineEdPosX- nLineStPosX > 20*/)
								{
									/*bIsWarpedWire = true;*/
									pRawInput[i * vMorpImg.step + j] = 0;
								}
							}
						}
					}
				

					setaFoot = setaFoot + dRstTheta_UseHoughline;

					CenterPo_WarpWire.x = CenterPos.x - FootBWMarginss.x;
					CenterPo_WarpWire.y = CenterPos.y - FootBWMarginss.y;

					cv::Point cvMargin = WarpAffine_IPPRotate(FootMorpRotImg, FootMorpRotImg, dRstTheta_UseHoughline, FootMorpRotImg.size(), CenterPo_WarpWire);

					cv::Point Foot3DMargin = WarpAffine_IPPRotate_3D(cvRoCorrect3dFoot, cvRoCorrect3dFoot, dRstTheta_UseHoughline, cvRoCorrect3dFoot.size(), CenterPo_WarpWire);		//Foot 3D Img 회전

				}
			}
		}
		
		//Re Calc Rect Area _ Rotate Foot (차렷자세)
		nRoFootBlobMinX = FootMorpRotImg.cols;
		nRoFootBlobMaxX = 0;
		nRoFootBlobMinY = FootMorpRotImg.rows;
		nRoFootBlobMaxY = 0;

		for (int r = 0; r < FootMorpRotImg.rows; r++)
		{
			UCHAR* Ptr = FootMorpRotImg.data;
			for (int c = 0; c < FootMorpRotImg.cols; c++)
			{
				if (Ptr[r * FootMorpRotImg.step + c] == 255)
				{
					if (r < nRoFootBlobMinY)	nRoFootBlobMinY = r;
					if (r > nRoFootBlobMaxY)	nRoFootBlobMaxY = r;
					if (c < nRoFootBlobMinX)	nRoFootBlobMinX = c;
					if (c > nRoFootBlobMaxX)	nRoFootBlobMaxX = c;

				}
			}
		}

		//Blob 한 중심(1/3 위치,nFindFootBlob_CenterY)에서 아래 부분은 입력받은 높이로 한번 더 검증 (중심 아래 부분 입력받은 높이값 220으로 제거)
		int nFindFootBlob_CenterY = 0;
		nFindFootBlob_CenterY = nRoFootBlobMinY + (int)((float)(nRoFootBlobMaxY - nRoFootBlobMinY) / 3.0);
		
		pFoot->nRemoveWireHeight = 220;

		cv::Mat cvRstFootBlob(FootMorpRotImg.rows, FootMorpRotImg.cols, CV_8UC1);
		cvRstFootBlob.setTo(0);

		for (int i = 0; i < cvRstFootBlob.rows; i++)
		{
			UCHAR* PtrFindFootMask = cvRstFootBlob.data;
			UCHAR* PtrRstFoot = cvRstFootBlob.data;
			float* PtrCorrected3d = (float*)cvRoCorrect3dFoot.data;

			for (int j = 0; j < cvRstFootBlob.cols; j++)
			{
				float valCorrected3d = PtrCorrected3d[i * floatstep_3DCorrect + j];

				if (nFindFootBlob_CenterY < i && PtrFindFootMask[i * cvRstFootBlob.step + j] == 255)
				{
					if (valCorrected3d < pFoot->nRemoveWireHeight
						&& nRoFootBlobMinX - 5 < j && j < nRoFootBlobMaxX + 5)
					{
						PtrRstFoot[i * cvRstFootBlob.step + j] = 255;
					}
				}

				if (nFindFootBlob_CenterY - 10 > i)
				{
					if (100 < valCorrected3d && valCorrected3d < 250
						&& nRoFootBlobMinX - 5 < j && j < nRoFootBlobMaxX + 5)		//wedge 높이가 높은 부분 검출되도록 
					{
						PtrRstFoot[i * cvRstFootBlob.step + j] = 255;
					}
				}
			}
		}

		cvRstFootBlob = cvRstFootBlob + FootMorpRotImg;

		//합친영상 관련 제거
		for (int i = 0; i < cvRstFootBlob.rows; i++)
		{
			UCHAR* PtrFindFootMask = cvRstFootBlob.data;
			float* PtrCorrected3d = (float*)cvRoCorrect3dFoot.data;

			for (int j = 0; j < cvRstFootBlob.cols; j++)
			{
				float valCorrected3d = PtrCorrected3d[i * floatstep_3DCorrect + j];

				if (nFindFootBlob_CenterY < i && PtrFindFootMask[i * cvRstFootBlob.step + j] == 255)
				{
					if (valCorrected3d > pFoot->nRemoveWireHeight)
					{
						PtrFindFootMask[i * cvRstFootBlob.step + j] = 0;
					}
				}
			}
		}

		int nCntB = 0;

		if (pFoot->m_bUse2Foot && bIsWarpedWire)
		{
			nCntB = m_pProcMilAlgo->CalcBlob_Select(cvRstFootBlob.ptr(), cvRstFootBlob.ptr(), cvRstFootBlob.cols, cvRstFootBlob.rows, 4, FALSE, TRUE, FALSE, eSelectPosition, CenterPo_WarpWire.x, CenterPo_WarpWire.y);

		}
		else
		{
			nCntB = m_pProcMilAlgo->CalcBlob_Select(cvRstFootBlob.ptr(), cvRstFootBlob.ptr(), cvRstFootBlob.cols, cvRstFootBlob.rows, 4, FALSE, TRUE, FALSE, eSelectBigger);
		}

		cv::erode(cvRstFootBlob, cvRstFootBlob, kernel);
		cv::erode(cvRstFootBlob, cvRstFootBlob, kernel);
		cv::erode(cvRstFootBlob, cvRstFootBlob, kernel);
		cv::erode(cvRstFootBlob, cvRstFootBlob, kernel);

		cv::dilate(cvRstFootBlob, cvRstFootBlob, kernel);
		cv::dilate(cvRstFootBlob, cvRstFootBlob, kernel);
		cv::dilate(cvRstFootBlob, cvRstFootBlob, kernel);
		cv::dilate(cvRstFootBlob, cvRstFootBlob, kernel);

		int nCnt_second = m_pProcMilAlgo->CalcBlob_Select(cvRstFootBlob.ptr(), cvRstFootBlob.ptr(), cvRstFootBlob.cols, cvRstFootBlob.rows, 300, FALSE, TRUE, FALSE, eSelectBigger);
		cv::Mat result_Footimg(cvRstFootBlob.size(), CV_8UC1, cv::Scalar(0));
		m_pProcMilAlgo->GetDrawContours(result_Footimg.ptr(), cvRstFootBlob.cols, cvRstFootBlob.rows, cvRstFootBlob.step, 255);

		FootMorpRotImg = cvRstFootBlob.clone();
		//////////////////////////////////////////////////

		//Re Calc Rect Area _ Rotate Foot (차렷자세)
		int nRoFootRectMinX = result_Footimg.cols;
		int nRoFootRectMaxX = 0;
		int nRoFootRectMinY = result_Footimg.rows;
		int nRoFootRectMaxY = 0;

		for (int r = 0; r < result_Footimg.rows; r++)
		{
			UCHAR* Ptr = result_Footimg.data;
			for (int c = 0; c < result_Footimg.cols; c++)
			{
				if (Ptr[r * result_Footimg.step + c] == 255)
				{
					if (r < nRoFootRectMinY)	nRoFootRectMinY = r;
					if (r > nRoFootRectMaxY)	nRoFootRectMaxY = r;
					if (c < nRoFootRectMinX)	nRoFootRectMinX = c;
					if (c > nRoFootRectMaxX)	nRoFootRectMaxX = c;

				}
			}
		}

		//make Foot mask rect ROI
		cv::Rect re;
		re.x = 0; re.y = 0;
		re.width = result_Footimg.cols; re.height = result_Footimg.rows;
		if (nRoFootRectMinX != result_Footimg.cols && nRoFootRectMinY != result_Footimg.rows && nRoFootRectMaxX != 0 && nRoFootRectMaxY != 0)
		{
			re.x = nRoFootRectMinX;
			re.y = nRoFootRectMinY;
			re.width = nRoFootRectMaxX - nRoFootRectMinX;
			re.height = nRoFootRectMaxY - nRoFootRectMinY;
		}

		//Make Debug Image
		cv::Mat cvOrgFootImg(nHeight, nWidth, CV_8UC1);
		pFoot->GetImageMatrix(&cvOrgFootImg, (int)m_eFootBin::m_eFootBin_Foot);
		cv::Mat FootOrgRotImg;
		cv::Point FMargin = WarpAffine_IPPRotate(cvOrgFootImg, FootOrgRotImg, setaFoot, cvOrgFootImg.size(), CenterPos);		//Foot Img 회전

		cv::Mat dstback = FootOrgRotImg.clone();
		cv::Mat testimage1;
		cv::cvtColor(dstback, testimage1, cv::COLOR_GRAY2RGB);		//Draw Rect (Orgin Foot Image)
		cv::rectangle(testimage1, re, cv::Scalar(0, 0, 255), 2);

#ifdef DEBUG			   
		//테스트용 이미지 저장
		CString drawImgPath;
		if (sSaveDebugPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(sSaveDebugPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_FindFootRst.bmp"), testimage1);
		}
		else
		{
			CTime t = CTime::GetCurrentTime();
			drawImgPath.Format(_T("D:\\FootRst\\FindFootRst_%s.bmp"), t.Format(_T("%y%m%d_%H%M%S")));

			cv::imwrite(std::string(CT2A(drawImgPath)), testimage1);
		}
#endif // DEBUG

		fPoints.rcArea.left = 0;
		fPoints.rcArea.top = 0;
		fPoints.rcArea.right = nWidth;
		fPoints.rcArea.bottom = nHeight;
		fPoints.cvMatchingMask = cv::Mat(nHeight, nWidth, CV_8UC1);
		fPoints.cvMatchingMask.setTo(255);
		fPoints.bMatchSuccess = false;
		fPoints.m_dSeta = setaFoot;



		//Calc Foot Margin ROI
		int nExpandRectMinX = nRoFootRectMinX;
		int nExpandRectMaxX = nRoFootRectMaxX;
		int nExpandRectMaxY = nRoFootRectMaxY;
		int nExpandRectMinY = nRoFootRectMinY;

		int nMarginXPixel = 0;
		if (re.width < 60)
		{
			nMarginXPixel = 30;

			if (pFoot->m_bUse2Foot)
			{
				nMarginXPixel = 20;
			}
		}
		else
			nMarginXPixel = re.width / 2;

		if (nExpandRectMinX - nMarginXPixel > 0)
			nExpandRectMinX = nExpandRectMinX - nMarginXPixel;
		else
			nExpandRectMinX = 0;

		if (nExpandRectMaxX + nMarginXPixel <= nWidth)
			nExpandRectMaxX = nExpandRectMaxX + nMarginXPixel;
		else
			nExpandRectMaxX = nWidth - 1;

		int nMarginYPixel = 0;
		if (bUseFootOnly2D)		//Set y-axis value range 
		{
			nMarginYPixel = 0;

			if (nExpandRectMaxY + nMarginYPixel <= nHeight)
				nExpandRectMaxY = nExpandRectMaxY + nMarginYPixel;
			else
				nExpandRectMaxY = nHeight - 1;

			if (nExpandRectMinY - nMarginYPixel > 0)
				nExpandRectMinY = nExpandRectMinY - nMarginYPixel;
			else
				nExpandRectMinY = 0;
		}
		else
			nExpandRectMaxY = nHeight - 1;

		//make Foot Margin Mask ROI image
		cv::Mat cvMatchRstMask(nHeight, nWidth, CV_8UC1);
		cvMatchRstMask.setTo(0);
		for (int r = 0; r < cvMatchRstMask.rows; r++)
		{
			UCHAR* Ptr = cvMatchRstMask.data;

			for (int c = 0; c < cvMatchRstMask.cols; c++)
			{
				if (nExpandRectMinX <= c && c <= nExpandRectMaxX)
				{
					if (r <= nExpandRectMaxY)
						Ptr[r * cvMatchRstMask.step + c] = 255;

					if (bUseFootOnly2D && nRoFootRectMinY >= r)
						Ptr[r * cvMatchRstMask.step + c] = 0;
				}
			}
		}

		//Rotate FootMask to Original Algo Image 
		cv::Mat cvRotateMatchRstMask = cv::Mat(cvMatchRstMask.rows, cvMatchRstMask.cols, CV_8UC1);
		cvRotateMatchRstMask.setTo(0);
		bool bRoMaskImgOrg = RotateBackToOriginalPosition(pFoot, cvMatchRstMask, CenterPos, FMargin, setaFoot, &cvRotateMatchRstMask);

		cv::Mat cvInspFootRctOrg = cv::Mat(cvMatchRstMask.rows, cvMatchRstMask.cols, CV_8UC1);
		cvInspFootRctOrg.setTo(0);
		bool bRoWingImgBackToOrgPos = RotateBackToOriginalPosition(pFoot, FootOrgRotImg, CenterPos, FMargin, setaFoot, &cvInspFootRctOrg);

		fPoints.cvMatchingMask = cvRotateMatchRstMask.clone();

		if (bRotateOrgImage)
			fPoints.cvMatchingMask = cvMatchRstMask.clone();	// rotate Foot Mask image to 0 degree 

		RoRectimage = cvMatchRstMask.clone();

		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			if (bUseFootOnly2D && !bIsDBCFoot)
			{
				cv::Mat FindFootMaskAreaOn3d(Image8U.rows, Image8U.cols, CV_8UC1);
				FindFootMaskAreaOn3d = cvTmpImgForDebug.clone();

				cv::Mat scale1;
				cv::Canny(cvRotateMatchRstMask, scale1, 100, 200, 3, false);
				scale1 = scale1 | result_LineImg;

				cv::Mat cvRstWeigtedImg;
				cv::addWeighted(FindFootMaskAreaOn3d, 0.5, scale1, 0.5, 0, cvRstWeigtedImg);

				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);

				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);

				cv::imwrite(rawname + std::string("_6_MaskArea.bmp"), cvRstWeigtedImg);
			}
		}

		//Calc ROI Rect 원본 영상 위치에서 
		int nOrgRectMinX = cvRotateMatchRstMask.cols;
		int nOrgRectMaxX = 0;
		int nOrgRectMinY = cvRotateMatchRstMask.rows;
		int nOrgRectMaxY = 0;

		for (int r = 0; r < cvRotateMatchRstMask.rows; r++)
		{
			UCHAR* Ptr = cvRotateMatchRstMask.data;
			for (int c = 0; c < cvRotateMatchRstMask.cols; c++)
			{
				if (Ptr[r * cvRotateMatchRstMask.step + c] == 255)
				{
					if (r < nOrgRectMinY)	nOrgRectMinY = r;
					if (r > nOrgRectMaxY)	nOrgRectMaxY = r;

					if (c < nOrgRectMinX)	nOrgRectMinX = c;
					if (c > nOrgRectMaxX)	nOrgRectMaxX = c;

				}
			}
		}

		fPoints.rcArea.left = nOrgRectMinX;
		fPoints.rcArea.top = nOrgRectMinY;
		fPoints.rcArea.right = nOrgRectMaxX;
		fPoints.rcArea.bottom = nOrgRectMaxY;

#pragma endregion

#pragma region 2. Inspection imprint Foot and double bonding Foot (using wing area)
		//반반걸쳐진이중본딩_전량파트 검증 후 변경 예정
		//Check 3d(cv3DAvgFilter) height at Founded_FootRect(cvMatchRstMask). 
		//if 3dHeight > 200(nRemoveWireHeight) and cvMatchRstMask_upper200 > FootRect Area 33%(1/3), NG. 
		if (bUseFootOnly2D && !bIsDBCFoot)
		{
			//Find Wing 3d 
			cv::Mat cvRstInspWingMask(cvRotateMatchRstMask.rows, cvRotateMatchRstMask.cols, CV_8UC1);
			bool bIsFindWing = FindWing_UseOnly3D(pFoot, &cvRstInspWingMask, cvRotateMatchRstMask.cols, cvRotateMatchRstMask.rows);

			//Rotate Wing 
			cv::Mat cvRoWingImg;
			cv::Point MarginWingImg = WarpAffine_IPPRotate(cvRstInspWingMask, cvRoWingImg, setaFoot, cvRstInspWingMask.size(), CenterPos);

			//Morpology Wing Image
			cv::Mat kernel_wing = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
			cv::erode(cvRoWingImg, cvRoWingImg, kernel_wing);
			cv::erode(cvRoWingImg, cvRoWingImg, kernel_wing);
			cv::dilate(cvRoWingImg, cvRoWingImg, kernel_wing);
			cv::dilate(cvRoWingImg, cvRoWingImg, kernel_wing);

			//bit operate (Wing & MaskFoot)
			cvRstInspWingMask = cvRoWingImg & cvRstFootBlob;

			//Wing Area _ Blob Filtering
			int nMinWingBlobPx = 50;
			if (pFoot->m_bUse2Foot)
				nMinWingBlobPx = 200;

			int nBlobType = ETypeBlob::eSelectMix;
			int nCntWingBlob = m_pProcMilAlgo->CalcBlob_Select(cvRstInspWingMask.ptr(), cvRstInspWingMask.ptr(),
				cvRstInspWingMask.cols, cvRstInspWingMask.rows, nMinWingBlobPx, FALSE, FALSE, FALSE, nBlobType, pFoot->m_FootCenterPoint_ImageRect.x, pFoot->m_FootCenterPoint_ImageRect.y);


			int nWingMinX = cvRstInspWingMask.cols;
			int nWingMaxX = 0;
			int nWingMinY = cvRstInspWingMask.rows;
			int nWingMaxY = 0;

			bool bFoundWingEndPoint = false;
			double setaFoot_2 = 0;
			cv::Point CenterPos_2;
			cv::Mat result_LineImg_2(vMorpImg.size(), CV_8UC1, cv::Scalar(0));
			cv::Mat cvReCalcFootArea(vMorpImg.size(), CV_8UC1, cv::Scalar(0));

			double dLineDistance = sqrt(pow(BSt1.x - BEd1.x, 2) + pow(BSt1.y - BEd1.y, 2));

			if (nCntWingBlob > 0)
			{
				for (int r = 0; r < cvRstInspWingMask.rows; r++)
				{
					UCHAR* Ptr = cvRstInspWingMask.data;
					for (int c = 0; c < cvRstInspWingMask.cols; c++)
					{
						if (Ptr[r * cvRstInspWingMask.step + c] == 255)
						{
							if (r < nWingMinY)	nWingMinY = r;
							if (r > nWingMaxY)	nWingMaxY = r;
							if (c < nWingMinX)  nWingMinX = c;
							if (c > nWingMaxX)  nWingMaxX = c;

						}
					}
				}

				if (((pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min]) * 2.5) < (dLineDistance*m_resolX)
					|| (((pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min]) * 1.5) < (dLineDistance*m_resolX) && pFoot->m_bUse2Foot) || (bIsWarpedWire && !bFoundHoughLine_inWarpedLine))
				{
					//여기서 윙 아래부분을 자르고 다시 Foot 각도 및 중심 계산
					cv::Mat cvClipImg_WingEd(FootMorpRotImg.rows, FootMorpRotImg.cols, CV_8UC1);
					cvClipImg_WingEd = FootMorpRotImg.clone();

					for (int r = 0; r < FootMorpRotImg.rows; r++)
					{
						UCHAR* Ptr = FootMorpRotImg.data;
						for (int c = 0; c < FootMorpRotImg.cols; c++)
						{
							if (Ptr[r * FootMorpRotImg.step + c] == 255)
							{
								if (nWingMaxY < r)
									Ptr[r * FootMorpRotImg.step + c] = 0;

							}
						}
					}

					//길이검증
					float fFootLength = ((float)nWingMaxY - (float)nRoFootRectMinY) * m_resolY;

					if ((pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min])* 1.5 >= fFootLength)
					{
						FootMorpRotImg = cvClipImg_WingEd.clone();
					}

					cv::erode(FootMorpRotImg, FootMorpRotImg, kernel);
					cv::erode(FootMorpRotImg, FootMorpRotImg, kernel);
					cv::erode(FootMorpRotImg, FootMorpRotImg, kernel);
					cv::erode(FootMorpRotImg, FootMorpRotImg, kernel);

					cv::dilate(FootMorpRotImg, FootMorpRotImg, kernel);
					cv::dilate(FootMorpRotImg, FootMorpRotImg, kernel);
					cv::dilate(FootMorpRotImg, FootMorpRotImg, kernel);
					cv::dilate(FootMorpRotImg, FootMorpRotImg, kernel);

					bool bRoMaskImgOrg = RotateBackToOriginalPosition(pFoot, FootMorpRotImg, CenterPos, FootMargin, setaFoot, &cvReCalcFootArea);

					cv::Point BSt1_2, BEd1_2;
					cv::Point poFootCenter_UseBlob_2;

					FindFootAngle_UseBlobLine(pFoot, &result_LineImg_2, &setaFoot_2, &BSt1_2, &BEd1_2, &poFootCenter_UseBlob_2, cvReCalcFootArea, cvReCalcFootArea.cols, cvReCalcFootArea.rows);
					fPoints.m_dSeta = setaFoot_2;

					cv::Mat FootImg_2 = BinImage.clone();
					cv::Mat FootRotImg_2;

					int nFootCtrX_2 = 0, nFootCtrY_2 = 0;
					nFootCtrX_2 = poFootCenter_UseBlob_2.x;
					nFootCtrY_2 = poFootCenter_UseBlob_2.y;

					if (nFootCtrX_2 == 0 || nFootCtrY_2 == 0)
					{
						nFootCtrX_2 = (int)(FootImg_2.cols / 2);
						nFootCtrY_2 = (int)(FootImg_2.rows / 2);
					}
					//fPoints.m_pCenter.x = nFootCtrX_2;
					//fPoints.m_pCenter.y = nFootCtrY_2;

					CenterPos_2.x = nFootCtrX_2;
					CenterPos_2.y = nFootCtrY_2;

					//재계산한 Foot 영역으로 Mask 자르기
					cv::Mat cvRoLineImg_2;		//차렷자세로 회전
					cv::Point ReCalcMargin = WarpAffine_IPPRotate(result_LineImg_2, cvRoLineImg_2, setaFoot_2, result_LineImg_2.size(), CenterPos);

					int nReCalcLineMinX = cvRoLineImg_2.cols;
					int nReCalcLineMaxX = 0;
					int nReCalcLineMinY = cvRoLineImg_2.rows;
					int nReCalcLineMaxY = 0;

					for (int r = 0; r < cvRoLineImg_2.rows; r++)
					{
						UCHAR* Ptr = cvRoLineImg_2.data;
						for (int c = 0; c < cvRoLineImg_2.cols; c++)
						{
							if (Ptr[r * cvRoLineImg_2.step + c] == 255)
							{
								if (r < nReCalcLineMinY)	nReCalcLineMinY = r;
								if (r > nReCalcLineMaxY)	nReCalcLineMaxY = r;
								if (c < nReCalcLineMinX)	nReCalcLineMinX = c;
								if (c > nReCalcLineMaxX)	nReCalcLineMaxX = c;

							}
						}
					}

					nWingMaxY = nReCalcLineMaxY;

					for (int r = 0; r < cvMatchRstMask.rows; r++)
					{
						UCHAR* Ptr = cvMatchRstMask.data;
						for (int c = 0; c < cvMatchRstMask.cols; c++)
						{
							if (Ptr[r * cvMatchRstMask.step + c] == 255)
							{
								if (nReCalcLineMaxY < r)
									Ptr[r * cvMatchRstMask.step + c] = 0;

							}
						}
					}

					bool bRoBackOrg = RotateBackToOriginalPosition(pFoot, cvMatchRstMask, CenterPos, ReCalcMargin, setaFoot_2, &cvRotateMatchRstMask);
					fPoints.cvMatchingMask = cvRotateMatchRstMask.clone();

					if (bRotateOrgImage)
						fPoints.cvMatchingMask = cvMatchRstMask.clone();	// rotate Foot Mask image to 0 degree 

					RoRectimage = cvMatchRstMask.clone();

					cv::Point ReMarginI = WarpAffine_IPPRotate(cvOrgFootImg, cvRoLineImg_2, setaFoot_2, cvMatchRstMask.size(), CenterPos);		//Foot Img 회전

					fPoints.m_dSeta = setaFoot_2;

					//fPoints.m_pCenter.x = CenterPos_2.x;
					//fPoints.m_pCenter.y = CenterPos_2.y;

					bFoundWingEndPoint = true;
				}
			}
			else
			{
				nWingMinX = 0;
				nWingMaxX = cvRstInspWingMask.cols - 1;
				nWingMinY = 0;
				nWingMaxY = cvRstInspWingMask.rows - 1;
			}

			if ((nWingMaxY - nWingMinY)*m_resolY < (pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min])*0.5)
			{
				nWingMinX = 0;
				nWingMaxX = cvRstInspWingMask.cols - 1;
				nWingMinY = 0;
				nWingMaxY = cvRstInspWingMask.rows - 1;
			}

			//이중본딩 관련
			cv::Mat cvRoFoot3D;
			cv::Point Margin_foot;
			if (bFoundWingEndPoint)
				cv::Point Margin_foot = WarpAffine_IPPRotate_3D(*cv3DAvgFilter, cvRoFoot3D, setaFoot_2, (*cv3DAvgFilter).size(), CenterPos);
			else
				Margin_foot = WarpAffine_IPPRotate_3D(*cv3DAvgFilter, cvRoFoot3D, setaFoot, (*cv3DAvgFilter).size(), CenterPos);		//Foot 3d Img 회전

			cv::Mat cvRstRemoveWireH_InFootMask(cvRotateMatchRstMask.rows, cvRotateMatchRstMask.cols, CV_8UC1);
			cvRstRemoveWireH_InFootMask.setTo(0);
			int nCntFootMaskAllArea = 0;
			int nCntWireHeightArea = 0;
			for (int i = 0; i < cvMatchRstMask.rows; i++)
			{
				float* Ptr3d = (float*)cvRoFoot3D.data;
				UCHAR* PtrFindFootMask = cvMatchRstMask.data;
				UCHAR* PtrRstRemovedWire = cvRstRemoveWireH_InFootMask.data;

				for (int j = 0; j < cvMatchRstMask.cols; j++)
				{
					float val3d = Ptr3d[i * floatstep_3DCorrect + j];
					int valMask2d = PtrFindFootMask[i * cvMatchRstMask.step + j];

					if (valMask2d == 255 && val3d > 0 && nWingMinY <= i && nWingMaxY >= i)
					{
						nCntFootMaskAllArea++;
						if (val3d >= pFoot->nRemoveWireHeight)
						{
							PtrRstRemovedWire[i * cvRotateMatchRstMask.step + j] = 255;
							nCntWireHeightArea++;
						}
					}
				}
			}
			//if Foot_upper200 > FindFootRectArea 33 % (1 / 3), NG.
			if (nCntFootMaskAllArea / 3 < nCntWireHeightArea)
			{
				CString cstrInspLog;
				cstrInspLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_CheckFootHeightInMask_overWire : Fail"));
				g_pMPTI->AddLog(cstrInspLog);

				//Tail 높이가 높은 경우 발생하므로 3D 기울기 윙검출 시작위치 찾기
				return false;
			}


			//찍힘 검사
			cv::Mat cvRstFootBlobInMask(cvRotateMatchRstMask.rows, cvRotateMatchRstMask.cols, CV_8UC1);
			cvRstFootBlobInMask.setTo(0);

			cv::Mat cvCorrect3dFoot(cvRotateMatchRstMask.rows, cvRotateMatchRstMask.cols, CV_32FC1, pFoot->pf3D);
			cv::Mat cvRstFootDistanceBlob(cvRotateMatchRstMask.rows, cvRotateMatchRstMask.cols, CV_8UC1);
			cvRstFootDistanceBlob.setTo(0);

			for (int i = 0; i < cvRotateMatchRstMask.rows; i++)
			{
				float* Ptr3d = (float*)(*cv3DAvgFilter).data;

				UCHAR* PtrFindFootMask = cvRotateMatchRstMask.data;

				float* PtrCorrected3d = (float*)cvCorrect3dFoot.data;
				UCHAR* PtrRstFindFootBlobInMask = cvRstFootBlobInMask.data;		//1. 찍힘 검사를 위해 Blob
				UCHAR* PtrRstFootDistance = cvRstFootDistanceBlob.data;		//2. 찍힘 검사를 위해 Line과의 거리측정
				UCHAR* PtrReCalcFootArea = cvReCalcFootArea.data;

				for (int j = 0; j < cvRotateMatchRstMask.cols; j++)
				{
					float val3d = Ptr3d[i * floatstep_3DCorrect + j];
					float valCorrected3d = PtrCorrected3d[i * floatstep_3DCorrect + j];
					int valMask2d = PtrFindFootMask[i * cvRotateMatchRstMask.step + j];

					if (valMask2d == 255 && val3d > 0)
					{
						if (val3d < pFoot->nRemoveWireHeight)
						{
							//Foot에서 150이상되는 영역 찾아 Blob
							if (val3d > pFoot->nInspCrackHeight)//120)
							{
								PtrRstFindFootBlobInMask[i * cvRotateMatchRstMask.step + j] = 255;
							}

							if (valCorrected3d > 10)
							{
								if (bFoundWingEndPoint)
								{
									if (PtrReCalcFootArea[i * cvRotateMatchRstMask.step + j] == 255)
										PtrRstFootDistance[i * cvRotateMatchRstMask.step + j] = 255;
								}
								else
								{
									PtrRstFootDistance[i * cvRotateMatchRstMask.step + j] = 255;
								}
							}

						}
					}

				}
			}

			//1. 찍힘 검출 조건1 - Foot 3d 높이 120 이상으로 Blob 후 덩어리 1개 이상일 시 찍혔다고 판정
			bool bUseFillHole = true;
			int nCntFootBlobInMask = m_pProcMilAlgo->CalcBlob_Select(cvRstFootBlobInMask.ptr(), cvRstFootBlobInMask.ptr(), cvRstFootBlobInMask.cols, cvRstFootBlobInMask.rows, 300, FALSE, bUseFillHole, FALSE, eSelectMix);
			if (nCntFootBlobInMask > 1 || nCntFootBlobInMask == 0)
			{
				CString cstrInspLog;
				cstrInspLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_ChkFootBlobCnt_ForImprinted : Fail"));
				g_pMPTI->AddLog(cstrInspLog);

				return false;		// 찍힌 경우 150이상으로 Blob했을 때 1개 이상 덩어리 나옴
			}

			//2. 찍힘 검출 조건2 - Foot BW 영상의 Row Lines check (If the number of lines is two or more : NG)
			bool bIsCrackedFoot = false;

			bool bIsFootStPo = false;
			bool bIsFootEndPo = false;
			int nCntFootWidthPixel = 0;
			int nCntFootNonWidthPixel = 0;

			int nCntSplitFootLines = 0;

			for (int i = 0; i < cvMatchRstMask.rows; i++)
			{
				float* Ptr3d = (float*)cvRoFoot3D.data;
				UCHAR* PtrMaskFoot = cvRstFootBlob.data;
				nCntFootWidthPixel = 0;
				nCntFootNonWidthPixel = 0;
				bIsFootStPo = false;
				bIsFootEndPo = false;

				if (bIsCrackedFoot)
					break;

				for (int j = 0; j < cvMatchRstMask.cols; j++)
				{
					float val3d = Ptr3d[i * floatstep_3DCorrect + j];
					int valMask2d = PtrMaskFoot[i * cvMatchRstMask.step + j];

					if (valMask2d == 255 && val3d > 0)
					{
						if (bIsFootEndPo)
						{
							nCntSplitFootLines++;
							bIsFootEndPo = false;
						}

						if (nCntSplitFootLines > 20)
						{
							bIsCrackedFoot = true;
							break;
						}
						bIsFootStPo = true;
						nCntFootWidthPixel++;
					}

					if (bIsFootStPo && valMask2d == 0 && nCntFootWidthPixel > 5 )
					{
						nCntFootNonWidthPixel++;

						if (nCntFootNonWidthPixel > 10)
							bIsFootEndPo = true;

					}

				}
			}

			if (bIsCrackedFoot)
			{
				CString cstrInspLog;
				cstrInspLog.Format(_T("PInsp_Foot::FindFoot_3DBlob()_ChkFootRowLines_ForImprinted : Fail"));
				g_pMPTI->AddLog(cstrInspLog);

				return false;		// 찍힌 경우 가로방향 라인이 2개 이상 나옴
			}

			//3. 찍힘 검출 조건 3 - Foot 3d 높이 20 이상으로 Blob 후, 덩어리가 Line에서 많이 떨어진 경우 찍혔다고 판정
			int nCntFootBlobDistance = m_pProcMilAlgo->CalcBlob_Select(cvRstFootDistanceBlob.ptr(), cvRstFootDistanceBlob.ptr(), cvRstFootDistanceBlob.cols, cvRstFootDistanceBlob.rows, 300, FALSE, FALSE, FALSE, eSelectBigger);

			if (nCntFootBlobDistance > 0)
			{
				float fdistLimitX = 0.0f;
				fdistLimitX = 250.0 / (m_resolX * 1000);		//Line 기준에서 220um 떨어진 경우

				//차렷자세로 라인 영상 회전
				cv::Mat cvRoFootLineImg;
				cv::Point MarginRo;
				if (bFoundWingEndPoint)
					MarginRo = WarpAffine_IPPRotate(result_LineImg_2, cvRoFootLineImg, setaFoot_2, cvOrgFootImg.size(), CenterPos);		//Foot Img 회전
				else
					MarginRo = WarpAffine_IPPRotate(result_LineImg, cvRoFootLineImg, setaFoot, cvOrgFootImg.size(), CenterPos);		//Foot Img 회전

				int nFootLineMinX = cvRoFootLineImg.cols;
				int nFootLineMaxX = 0;
				int nFootLineMinY = cvRoFootLineImg.rows;
				int nFootLineMaxY = 0;

				for (int r = 0; r < cvRoFootLineImg.rows; r++)
				{
					UCHAR* Ptr = cvRoFootLineImg.data;
					for (int c = 0; c < cvRoFootLineImg.cols; c++)
					{
						if (Ptr[r * cvRoFootLineImg.step + c] == 255)
						{
							if (nWingMaxY < r)
								Ptr[r * cvRoFootLineImg.step + c] = 0;

						}
					}
				}

				for (int r = 0; r < cvRoFootLineImg.rows; r++)
				{
					UCHAR* Ptr = cvRoFootLineImg.data;
					for (int c = 0; c < cvRoFootLineImg.cols; c++)
					{
						if (Ptr[r * cvRoFootLineImg.step + c] == 255)
						{
							if (r < nFootLineMinY)	nFootLineMinY = r;
							if (r > nFootLineMaxY)	nFootLineMaxY = r;
							if (c < nFootLineMinX)  nFootLineMinX = c;
							if (c > nFootLineMaxX)  nFootLineMaxX = c;

						}
					}
				}

				//차렷자세로 Blob 영상 회전
				cv::Mat cvFootBlobDistanceInMaskImg;
				cv::Point MarginImg;
				if (bFoundWingEndPoint)
					MarginImg = WarpAffine_IPPRotate(cvRstFootDistanceBlob, cvFootBlobDistanceInMaskImg, setaFoot_2, cvOrgFootImg.size(), CenterPos);		//Foot Img 회전
				else
					MarginImg = WarpAffine_IPPRotate(cvRstFootDistanceBlob, cvFootBlobDistanceInMaskImg, setaFoot, cvOrgFootImg.size(), CenterPos);		//Foot Img 회전

				for (int r = 0; r < cvFootBlobDistanceInMaskImg.rows; r++)
				{
					UCHAR* Ptr = cvFootBlobDistanceInMaskImg.data;
					for (int c = 0; c < cvFootBlobDistanceInMaskImg.cols; c++)
					{
						if (Ptr[r * cvFootBlobDistanceInMaskImg.step + c] == 255)
						{
							if (nFootLineMaxY < r)
								Ptr[r * cvFootBlobDistanceInMaskImg.step + c] = 0;

							if (nWingMaxY < r)
								Ptr[r * cvFootBlobDistanceInMaskImg.step + c] = 0;
						}
					}
				}

				//검출한 Line과 Blob과의 최대 거리 계산 (정 자세로 돌린 Line과, Blob 영상에서 x축 방향 거리를 양쪽 각각 측정)
				//네모 영역 구하기
				int nFootBlobRectMinX = cvFootBlobDistanceInMaskImg.cols;
				int nFootBlobRectMaxX = 0;
				int nFootBlobRectMinY = cvFootBlobDistanceInMaskImg.rows;
				int nFootBlobRectMaxY = 0;
				for (int r = 0; r < cvFootBlobDistanceInMaskImg.rows; r++)
				{
					UCHAR* Ptr = cvFootBlobDistanceInMaskImg.data;
					for (int c = 0; c < cvFootBlobDistanceInMaskImg.cols; c++)
					{
						if (Ptr[r * cvFootBlobDistanceInMaskImg.step + c] == 255)
						{
							if (r < nFootBlobRectMinY)	nFootBlobRectMinY = r;
							if (r > nFootBlobRectMaxY)	nFootBlobRectMaxY = r;
							if (c < nFootBlobRectMinX)  nFootBlobRectMinX = c;
							if (c > nFootBlobRectMaxX)  nFootBlobRectMaxX = c;
						}
					}
				}

				int nMaxDistance = 0;
				int nFindLinePosX = 0;
				bool bIsFirstLinePosX = false;
				int nFirstLinePosX = 0;

				for (int r = 0; r < cvRoFootLineImg.rows; r++)
				{
					UCHAR* PtrFootCtLinePx = cvRoFootLineImg.data;
					for (int c = 0; c < cvRoFootLineImg.cols; c++)
					{
						if (PtrFootCtLinePx[r * cvRoFootLineImg.step + c] == 255)
						{
							nFindLinePosX = c;

							if (!bIsFirstLinePosX)
							{
								bIsFirstLinePosX = true;
								nFirstLinePosX = c;
							}

							int nMaxD = 0;
							if (abs(nFootBlobRectMaxX - nFindLinePosX) > abs(nFootBlobRectMinX - nFindLinePosX))
								nMaxD = abs(nFootBlobRectMaxX - nFindLinePosX);
							else
								nMaxD = abs(nFootBlobRectMinX - nFindLinePosX);

							if (nMaxD > nMaxDistance)
								nMaxDistance = nMaxD;

						}
					}
				}

				//계산된 최대거리가 일정거리 조건 이상일 경우, NG처리
				//if (((pFoot->m_fArrOptionValue[m_eFoot_Width_1][eMMD_Min])) < (nMaxDistance * m_resolX))
				//if (fdistLimitX < (nMaxDistance))
				//	return false;

			}
		}

		//debug 용 이미지
#ifdef _AlgoTool_1
		if (bFstChk)
		{
			cv::Mat cvFindRect(Image8U.rows, Image8U.cols, CV_8UC1);
			cvFindRect.setTo(0);
			cv::rectangle(cvFindRect, re, cv::Scalar(255), cv::FILLED);
//
//			//cv::Mat cvRoFindRectImg;
//			//cv::Point MarginPo = WarpAffine_IPPRotate(cvFindRect, cvRoFindRectImg, -setaFoot, cvFindRect.size(), CenterPos);		//Foot Img 회전
//
			cv::Mat cvRoFindRectImg = cv::Mat(Image8U.rows, Image8U.cols, CV_8UC1);
			cvRoFindRectImg.setTo(0);
			bool bRoMaskImgOrg = RotateBackToOriginalPosition(pFoot, cvFindRect, CenterPos, FMargin, setaFoot, &cvRoFindRectImg);
//
			cv::Mat scale1;
			cv::Canny(cvRoFindRectImg, scale1, 100, 200, 3, false);
//
//			//라인 확장
			cv::Mat kernel_LineMorp = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
//
			cv::dilate(scale1, scale1, kernel_LineMorp);
			cv::dilate(scale1, scale1, kernel_LineMorp);
//
			cv::cvtColor(scale1, scale1, cv::COLOR_GRAY2RGB);
//
			cv::Mat changeImg;
//			// 			cv::cvtColor(scale1, hsv, cv::COLOR_BGR2HSV); //convert BGR to HSV
			vector<cv::Mat>  channels;
			cv::split(scale1, channels); //split to h,s,v
//
//			//channels[0] 색상(H),  channels[1]  채도(S),  channels[2]  명도(V)
//			//각 채널 출력
			channels[0] = 0; //value to max
			channels[1] = 0; //value to max
			channels[2] = 255; //value to max
			cv::merge(channels, changeImg); //merge h,s,v
//
			changeImg &= scale1;
//
			cv::Mat cvFoot3dFilter(Image8U.rows, Image8U.cols, CV_8UC1);
			cvFoot3dFilter = cvTmpImgForDebug.clone();
			cv::cvtColor(cvFoot3dFilter, cvFoot3dFilter, cv::COLOR_GRAY2RGB);
//
			cv::Mat cvRstWeigtedImg;
			cv::addWeighted(cvFoot3dFilter, 0.95, changeImg, 0.9, 0, cvRstWeigtedImg);
//
			int nWidth = pFoot->GetImageWidth();
			int nHeight = pFoot->GetImageLength();
			cv::Rect CropRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);
//
			g_pMPTI->m_InspMng->footDebugImg_CropData.push_back(std::make_pair(cvRstWeigtedImg.clone(), "Pass"));
//
			if (cstDebugFolderPath.IsEmpty() != TRUE)
			{
				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);
//
				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);
//
				cv::imwrite(rawname + std::string("_FindFoot_3DRect.bmp"), cvRstWeigtedImg);
			}
		}
#endif // DEBUG

#pragma endregion

	}

	return true;

}


bool CPInsp_Foot::RotateBackToOriginalPosition(CFoot_Model* pFoot, cv::Mat inputImg, cv::Point& CenterPo, cv::Point& Margin, double& seta, cv::Mat* ResultImg)
{
	bool Ret = false;

	int nLine = __LINE__;

	try
	{
		cv::Mat rotatedInputImg = inputImg.clone();

		cv::Mat RstRoImg;
		cv::Mat WingOrg = cv::Mat(inputImg.rows, inputImg.cols, CV_8UC1);
		WingOrg.setTo(0);

		bool existWingRst = true;
		RstRoImg = rotatedInputImg;

		cv::Point Center;
		Center.x = CenterPo.x - Margin.x;
		Center.y = CenterPo.y - Margin.y;

		//cv::Point margin_New = WarpAffine(rotatedInputImg, RstRoImg, -(seta), WingOrg.size(), Center);	//Wing 3D Img È¸Àü
		cv::Point margin_New = WarpAffine_IPPRotate(rotatedInputImg, RstRoImg, -(seta), WingOrg.size(), Center);	//Wing 3D Img È¸Àü

		int ncvImgWidth = RstRoImg.cols;
		int ncvImgHeight = RstRoImg.rows;
		int nWingTmpMin_x = ncvImgWidth * ncvImgHeight, nWingTmpMax_x = 0;
		int nWingTmpMin_y = ncvImgWidth * ncvImgHeight, nWingTmpMax_y = 0;
		for (int r = 0; r < RstRoImg.rows; r++)
		{
			UCHAR* Ptr = RstRoImg.data;
			for (int c = 0; c < RstRoImg.cols; c++)
			{
				if (Ptr[r*RstRoImg.step + c] == 255)
				{
					if (c < nWingTmpMin_x) nWingTmpMin_x = c;
					if (c > nWingTmpMax_x) nWingTmpMax_x = c;
					if (r < nWingTmpMin_y) nWingTmpMin_y = r;
					if (r > nWingTmpMax_y) nWingTmpMax_y = r;
				}
			}
		}

		if (nWingTmpMin_x == ncvImgWidth * ncvImgHeight && nWingTmpMin_y == ncvImgWidth * ncvImgHeight &&
			nWingTmpMax_x == 0 && nWingTmpMax_y == 0)
		{
			existWingRst = false;
		}

		if (existWingRst)
		{
			RECT rtWingRectRst;
			rtWingRectRst.left = nWingTmpMin_x;
			rtWingRectRst.right = nWingTmpMax_x;
			rtWingRectRst.top = nWingTmpMin_y;
			rtWingRectRst.bottom = nWingTmpMax_y;

			int diffWingCX = (CenterPo.x - Margin.x) - nWingTmpMin_x;
			int diffWingCY = (CenterPo.y - Margin.y) - nWingTmpMin_y;

			cv::Mat WingRectRst = RstRoImg(cv::Rect(nWingTmpMin_x, nWingTmpMin_y, (nWingTmpMax_x - nWingTmpMin_x)+1, (nWingTmpMax_y - nWingTmpMin_y)+1));

			int WingRectStX = 0, WingRectStY = 0;

			if (CenterPo.x - diffWingCX < 0)
				WingRectStX = 0;
			else
				WingRectStX = CenterPo.x - diffWingCX;

			if (CenterPo.y - diffWingCY < 0)
				WingRectStY = 0;
			else
				WingRectStY = CenterPo.y - diffWingCY;

			if (WingRectStX + WingRectRst.cols > WingOrg.cols)
				WingRectStX = WingOrg.cols - WingRectRst.cols;

			if (WingRectStY + WingRectRst.rows > WingOrg.rows)
				WingRectStY = WingOrg.rows - WingRectRst.rows;

			WingOrg.setTo(0);
			cv::Mat A = WingOrg(cv::Rect(WingRectStX, WingRectStY, WingRectRst.cols, WingRectRst.rows));
			WingRectRst.copyTo(A);

			WingOrg.copyTo(*ResultImg);

		}

		Ret = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Wedge::RotateBackToOriginalPosition() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;

}


//Form  y = (-A/B)*x + (-C/B)
bool CPInsp_Foot::LSQLineFit(std::vector<POINT>& pts, double &pdA, double &pdB, bool* bVertical, bool* bStraight)
{
	if (pts.size() < 2) return false;

	double X = 0, Y = 0, XY = 0, X2 = 0, Y2 = 0;
	for (auto const& point : pts)
	{
		double dx = (double)point.x;
		double dy = (double)point.y;

		X += dx;
		Y += dy;
		XY += dx * dy;
		X2 += dx * dx;
		Y2 += dy * dy;
	}
	X /= pts.size();
	Y /= pts.size();
	XY /= pts.size();
	X2 /= pts.size();
	Y2 /= pts.size();

	double A = 0, B = 0, C = 0;

	A = -(XY - X * Y); //!< Common for both solution
	double Bx = X2 - X * X;
	double By = Y2 - Y * Y;

	if (fabs(Bx) < fabs(By)) //!< Test verticality/horizontality
	{ // Line is more Vertical.
		B = By;
		std::swap(A, B);

		if (bVertical)
			*bVertical = true;
	}
	else
	{   // Line is more Horizontal.
		// Classical solution, when we expect more horizontal-like line
		B = Bx;

		if (bVertical)
			*bVertical = false;
	}

	C = -(A * X + B * Y);
	//y = (-A/B)*x + (-C/B)

	if (std::fabs(B) < 1e-7) {
		// singular matrix. can't solve the problem.
		pdA = X;
		pdB = 0;

		if (bStraight)
			*bStraight = true;
		return false;
	}
	else
	{
		//Normalize
		/*double D = sqrt(A*A + B * B);
		A /= D;
		B /= D;
		C /= D;*/

		pdA = (-A / B);
		pdB = (-C / B);
	}

	if (pdA == 0)
	{
		if (bStraight)
			*bStraight = true;
		return false;
	}

	if (bStraight)
		*bStraight = false;

	return true;
}

bool cmp_X(const POINT& u, const POINT& v) {

	if (u.x < v.x)

		return true;

	else if (u.x == v.x)

		return u.y < v.y;

	else

		return false;

}

bool cmp_Y(const POINT& u, const POINT& v) {

	if (u.y < v.y)

		return true;

	else if (u.y == v.y)

		return u.x < v.x;

	else

		return false;

}

bool CPInsp_Foot::GetLinePoint(BOOL bStraight, BOOL bVertical, float a, float b, std::vector<POINT>& vecpt, POINTF& x1, POINTF& x2)
{
	if (bStraight&& bVertical)
	{
		std::sort(vecpt.begin(), vecpt.end(), cmp_Y);

		float minY = (float)std::min(vecpt[0].y, vecpt[vecpt.size() - 1].y);
		float maxY = (float)std::max(vecpt[0].y, vecpt[vecpt.size() - 1].y);

		x1.x = a;
		x1.y = minY;

		x2.x = a;
		x2.y = maxY;
	}
	else if (bStraight&& bVertical == FALSE)
	{
		std::sort(vecpt.begin(), vecpt.end(), cmp_X);

		float minX = (float)std::min(vecpt[0].x, vecpt[vecpt.size() - 1].x);
		float maxX = (float)std::max(vecpt[0].x, vecpt[vecpt.size() - 1].x);

		x1.x = minX;
		x1.y = b;

		x2.x = maxX;
		x2.y = b;
	}
	else if (bVertical)
	{
		std::sort(vecpt.begin(), vecpt.end(), cmp_Y);

		float minY = (float)std::min(vecpt[0].y, vecpt[vecpt.size() - 1].y);
		float maxY = (float)std::max(vecpt[0].y, vecpt[vecpt.size() - 1].y);

		x1.x = (minY - b) / a;
		x1.y = minY;

		x2.x = (maxY - b) / a;
		x2.y = maxY;
	}
	else
	{
		std::sort(vecpt.begin(), vecpt.end(), cmp_X);

		float minX = (float)std::min(vecpt[0].x, vecpt[vecpt.size() - 1].x);
		float maxX = (float)std::max(vecpt[0].x, vecpt[vecpt.size() - 1].x);

		x1.x = minX;
		x1.y = minX * a + b;

		x2.x = maxX;
		x2.y = maxX * a + b;
	}

	return true;
}

bool CPInsp_Foot::CalcFoot3DAvgFilterImage(CFoot_Model* model, int nSrcWidth, int nSrcHeight, cv::Mat cvPadBWImage, cv::Mat *cvRst3DAvgFilter)
{
	//높이 패턴 매칭용 구문

	int wid = nSrcWidth;
	int len = nSrcHeight;

	//1. Foot 3D 가우시안 필터링
	cv::Mat cvOrg3D(len, wid, CV_32FC1, model->pf3D);
	cv::Mat cvTmp3D = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

	size_t floatstep_3DCorrect = cvOrg3D.step / sizeof(float);
	for (int y = 0; y < len; y++)
	{
		//UCHAR* pRaw = cvAutoInsp_pad2D.data;
		float* rowptr = (float*)cvOrg3D.data;

		for (int x = 0; x < wid; x++)
		{
			float fHgtVal = rowptr[y * floatstep_3DCorrect + x];

			//if (pRaw[y * cvAutoInsp_pad2D.step + x] == 255)
			//	rowptr[y * floatstep_3DCorrect + x] = 0;

			if (fHgtVal < 30)
				rowptr[y * floatstep_3DCorrect + x] = 0;

		}
	}


	cv::Mat avgKernel = cv::Mat::ones(3, 3, CV_32FC1) / 9;

	cv::filter2D(cvOrg3D, cvTmp3D, -1, avgKernel, cv::Point(-1, -1), (0, 0), cv::BORDER_REFLECT101);

	//normalize 하기
	cv::Mat im3Dgrad_8bit_norm, im3DFilter_8bit;
	cv::normalize(cvTmp3D, im3Dgrad_8bit_norm, 0, 255, cv::NORM_MINMAX);
	im3Dgrad_8bit_norm.convertTo(im3DFilter_8bit, CV_8UC1);

	//Blob 필터링
	AlgoBlob algoBlob;
	algoBlob.m_bUseIPC = false;
	algoBlob.m_bInvertCheck = false;
	algoBlob.m_bFilterIsUse = false;
	algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
// 	algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;
// 
// 	if (model->m_bUse2Foot)
// 		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

	algoBlob.m_bInsp3D = false;
	algoBlob.m_nTypeRange3D = 2;
	algoBlob.m_dHeightRateMin = 0;
	algoBlob.m_dHeightRateMax = 0;

	algoBlob.m_bInsp2D = true;
	algoBlob.m_nTypeRange2D = 2;
	algoBlob.m_nMinBinary = 0;
	algoBlob.m_nMaxBinary = 40;
	algoBlob.m_bFillHole = true;

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;

	double dCX = 0, dCY = 0, dArea = 0;
	CRect rcBlob;
	rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

	//cv::Mat BinWingBlobRstimg(roFootBinImg.rows, roFootBinImg.cols, CV_8UC1);
	cv::Mat Img3D_Candidates(len, wid, CV_32FC1);
	cv::Mat BinImage(len, wid, CV_8UC1);
	int nCntBlob = CPInsp::BlobImageStruct(algoBlob, im3DFilter_8bit.data, Img3D_Candidates.ptr<float>(0), NULL, wid, len, 1000, &dArea, &dCX, &dCY, &rcBlob, BinImage.data,
		stTieAreaNULL, true, false, model->m_FootCenterPoint_ImageRect.x, model->m_FootCenterPoint_ImageRect.y);
	//int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(im3DFilter_8bit.data, BinImage.data, nWidth, nHeight, 100, FALSE, TRUE, 0, blobType, fPoints->m_pCenter.x, fPoints->m_pCenter.y);

	cv::Mat cvAutoInsp_pad2D;
	cvAutoInsp_pad2D = cvPadBWImage.clone();

	for (int y = 0; y < len; y++)
	{
		UCHAR* pRaw = BinImage.data;
		float* rowptr = (float*)cvTmp3D.data;

		for (int x = 0; x < wid; x++)
		{
			float fHgtVal = rowptr[y * floatstep_3DCorrect + x];

			if (pRaw[y * cvAutoInsp_pad2D.step + x] == 0)
				rowptr[y * floatstep_3DCorrect + x] = 0;

		}
	}

	// 
	cv::filter2D(cvTmp3D, cvTmp3D, -1, avgKernel, cv::Point(-1, -1), (0, 0), cv::BORDER_REFLECT101);

	// 	cv::Mat im3Dgrad_8bit_norm_1, im3DFilter_8bit_1;
	// 	cv::normalize(cvTmp3D, im3Dgrad_8bit_norm_1, 0, 255, cv::NORM_MINMAX);
	// 	im3Dgrad_8bit_norm_1.convertTo(im3DFilter_8bit_1, CV_8UC1);
	// 
	// 	cv::filter2D(cvTmp3D, cvTmp3D, -1, avgKernel, cv::Point(-1, -1), (0, 0), cv::BORDER_REFLECT101);

	cv::Mat im3Dgrad_8bit_norm_2, im3DFilter_8bit_2;
	cv::normalize(cvTmp3D, im3Dgrad_8bit_norm_2, 0, 255, cv::NORM_MINMAX);
	im3Dgrad_8bit_norm_2.convertTo(im3DFilter_8bit_2, CV_8UC1);

	for (int y = 0; y < len; y++)
	{
		float* rowptr = (float*)cvTmp3D.data;

		for (int x = 0; x < wid; x++)
		{
			float fHgtVal = rowptr[y * floatstep_3DCorrect + x];

			if (fHgtVal < 30)
				rowptr[y * floatstep_3DCorrect + x] = 0;

		}
	}

	int nInputRemoveWireH = 220;
	if (model->nRemoveWireHeight > 0)
	{
		nInputRemoveWireH = model->nRemoveWireHeight;
	}

	cv::Mat	 cvImgFootCtLine(len, wid, CV_8UC1);
	cvImgFootCtLine.setTo(0);
	int nMediumStPo = 0, nXDirForegroundCnt = 0;
	for (int y = 0; y < len; y++)
	{
		UCHAR* pRaw = cvImgFootCtLine.data;
		float* rowptr = (float*)im3Dgrad_8bit_norm_2.data;
		float* fRaw3DH = (float*)cvTmp3D.data;

		nMediumStPo = 0;
		nXDirForegroundCnt = 0;

		for (int x = 0; x < wid; x++)
		{
			float fHgtVal = rowptr[y * floatstep_3DCorrect + x];

			if (fHgtVal > 0 && fRaw3DH[y * floatstep_3DCorrect + x] < nInputRemoveWireH)
			{
				if (nMediumStPo == 0)
					nMediumStPo = x;

				nXDirForegroundCnt++;
				//pRaw[y * cvImgFootCtLine.step + x] = 255;
			}
		}

		if (nXDirForegroundCnt > 40 && (nMediumStPo + (int)(nXDirForegroundCnt / 2)) < wid)
		{
			pRaw[y * cvImgFootCtLine.step + (nMediumStPo + (int)(nXDirForegroundCnt / 2))] = 255;
		}
	}

	/*
	cv::Mat cvRo3D(len, wid, CV_32FC1);
	cvRo3D = cvTmp3D.clone();
	cv::Point cvPoCenter(152, 152);
	cv::Point Margin = WarpAffine_IPPRotate_3D(cvRo3D, cvRo3D, 19, cvRo3D.size(), cvPoCenter, false, true);		////Foot Img 회전

	cv::Mat cv3D8BitMorp(len, wid, CV_8UC1);

	int nWingRectMinX = cv3D8BitMorp.cols - 1;
	int nWingRectMaxX = 0;
	int nWingRectMinY = cv3D8BitMorp.rows - 1;
	int nWingRectMaxY = 0;

	for (int y = 0; y < len; y++)
	{
		UCHAR* pRaw = cv3D8BitMorp.data;
		float* fRaw3DH = (float*)cvRo3D.data;

		for (int x = 0; x < wid; x++)
		{
			float fHgtVal = fRaw3DH[y * floatstep_3DCorrect + x];

			if (fHgtVal > 0 && fHgtVal < nInputRemoveWireH)
			{
				pRaw[y * cvImgFootCtLine.step + x] = 255;
			}
		}
	}

	cv::Mat kernel22 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
	//cv::erode(cv3D8BitMorp, cv3D8BitMorp, kernel);
	cv::dilate(cv3D8BitMorp, cv3D8BitMorp, kernel22);

	for (int y = 0; y < len; y++)
	{
		UCHAR* pRaw = cv3D8BitMorp.data;

		for (int x = 0; x < wid; x++)
		{
			if (y < nWingRectMinY)	nWingRectMinY = y;
			if (y > nWingRectMaxY)	nWingRectMaxY = y;

			if (x < nWingRectMinX) nWingRectMinX = x;
			if (x > nWingRectMaxX) nWingRectMaxX = x;
		}
	}

	//cv::Mat cvRectArea;
	//cv::rectangle(cvRectArea, cv::Rect(nWingRectMinX, nWingRectMinY, nWingRectMaxX - nWingRectMinX, nWingRectMaxY - nWingRectMinY), cv::Scalar(255), cv::FILLED);


	{
		//int nModelWid = 63, nModelLen = 135;
		//cv::Mat sub3DModelImage = cvRo3D(cv::Rect(140, 65, nModelWid, nModelLen)).clone();

		int nModelWid = 35, nModelLen = 155;
		int StX = 0;
		StX = 127;
		cv::Mat sub3DModelImage = cvRo3D(cv::Rect(StX, 45, nModelWid, nModelLen)).clone();

		float *fp2;
		fp2 = (float*)sub3DModelImage.data;

		HEADER_PTR hPtr;
		hPtr.uiNumRow = nModelWid;
		hPtr.uiNumCol = nModelLen;
		hPtr.zResolX = 5.5f;
		hPtr.zResolY = 5.5f;
		hPtr.sizeBit = 32;
		alpf_save_ptr(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF0.ptr"), &hPtr, fp2);

		CString FilePath;
		FilePath.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF0.ppd"), 15);

		RECT rt;
		rt.left = 0; rt.right = nModelWid; rt.top = 0; rt.bottom = nModelLen;

		CreateModel3D(sub3DModelImage, FilePath, 0, rt, 0);	//위쪽 본딩 방향
		//CreateModel3D(sub3DModelImage, FilePath, 0, rt, 180);	//아래쪽 본딩 방향
		//CreateModel3D(sub3DModelImage, FilePath, 0, rt, 90);
		//CreateModel3D(sub3DModelImage, FilePath, 0, rt, 270);
	}


	//Foot 3D Filtering 된 영상으로 Master 3D모델 등록
	{
		CString pullpath;
		TCHAR fname[MAX_PATH];

		//////////////////
		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF0.ptr"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName;
			strBackupName.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF0.ptr"));
			CopyFile(pullpath, strBackupName, false);
		}

		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF0.ppd"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName2;
			strBackupName2.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF0.ppd"));
			CopyFile(pullpath, strBackupName2, false);
		}
		//////////////////
		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF180.ptr"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName;
			strBackupName.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF180.ptr"));
			CopyFile(pullpath, strBackupName, false);
		}

		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF180.ppd"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName2;
			strBackupName2.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF180.ppd"));
			CopyFile(pullpath, strBackupName2, false);
		}

		//////////////////
		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF90.ptr"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName;
			strBackupName.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF90.ptr"));
			CopyFile(pullpath, strBackupName, false);
		}

		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF90.ppd"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName2;
			strBackupName2.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF90.ppd"));
			CopyFile(pullpath, strBackupName2, false);
		}
		//////////////////
		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF270.ptr"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName;
			strBackupName.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF270.ptr"));
			CopyFile(pullpath, strBackupName, false);
		}

		pullpath.Format(_T("D:\\Eagle3D_data\\MasterModel\\Image3D_master_LF270.ppd"));
		wsprintf(fname, pullpath);
		if (_taccess(pullpath, 0) != -1)
		{
			CString strBackupName2;
			strBackupName2.Format(_T("D:\\Eagle3D_data\\FootModel\\Image3D_master_LF270.ppd"));
			CopyFile(pullpath, strBackupName2, false);
		}
	}
	*/

	if (!cvRst3DAvgFilter)
		cvRst3DAvgFilter = new cv::Mat(len, wid, CV_32FC1);

	cvRst3DAvgFilter->convertTo(*cvRst3DAvgFilter, CV_32FC1);

	cvTmp3D.copyTo(*cvRst3DAvgFilter);

	return true;
}

bool CPInsp_Foot::Calc_padBinImage4(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Rect ImageRect, cv::Mat* RepadImg, cv::Mat Img3D, bool bTeachBin)
{
	try
	{
		int nWidth = ImageRect.width, nHeight = ImageRect.height;

		if ((nWidth <= 0) || (nHeight <= 0))
			return false;

		if ((nWidth < 11) || (nHeight < 11))
			return false;

		if (pFoot == NULL || sRstAlgo == NULL)
			return false;

		if(pFoot->m_PadRect.left - pFoot->mImageRect.left <=0 || pFoot->m_PadRect.top - pFoot->mImageRect.top <= 0 ||
			pFoot->m_PadRect.right - pFoot->m_PadRect.left <= 0 || pFoot->m_PadRect.bottom - pFoot->m_PadRect.top <= 0)
			return false;

		bool bIsDBCFoot = false;
		if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
			bIsDBCFoot = true;

		cv::Mat padImg(nHeight, nWidth, CV_8UC1);

		//1. Get Pad BW Image 
		pFoot->GetImageMatrix(&padImg, (int)m_eFootBin::m_eFootBin_Pad);
		int nPadBlobCnt = 0;		
		nPadBlobCnt = GetBinImage_Pad(pFoot, &padImg, (int)m_eFootBin::m_eFootBin_Pad, NULL, false);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\Calc_padBinImage.bmp", padImg);
#endif
		
		cv::Mat PadMaskCalc = padImg.clone();
		if (bIsDBCFoot && pFoot->m_bUsePadAreaAutoTeach)
		{
			cv::Mat cvAutoTeachedPad(nHeight, nWidth, CV_8UC1, pFoot->m_pPad);
			
			PadMaskCalc = cvAutoTeachedPad.clone();
	
		}

		if (bIsDBCFoot)
		{
			cv::Mat DBCBinImage(nHeight, nWidth, CV_8UC1);

			bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

			if (bIsFindDBCOutLine)
				PadMaskCalc = PadMaskCalc + DBCBinImage;
		}

		//2. Get Foot BW Image (FootBW - PadCandidateArea)
		cv::Mat Foot_Bin(nHeight, nWidth, CV_8UC1);
		cv::Mat PadNFootImg(nHeight, nWidth, CV_8UC1);
		pFoot->GetImageMatrix(&Foot_Bin, (int)m_eFootBin::m_eFootBin_Foot);
		GetBinImage(pFoot, &Foot_Bin, (int)m_eFootBin::m_eFootBin_Foot, NULL, false);

		UCHAR *pUcImgDst2D = NULL;
		UCHAR *pUcImgDst3D = NULL;
		UCHAR *pUcImgBinary = padImg.data;

		double dArea, dCX, dCY;
		dCX = 0, dCY = 0, dArea = 0;

		int Range3D = 0;

		CRect rcBlob;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

		AlgoBlob algoBlob;
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;
		algoBlob.m_bInsp3D = false;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_dHeightRateMin = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
		algoBlob.m_dHeightRateMax = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];
		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 100;		algoBlob.m_nMaxBinary = 100;
		algoBlob.m_bFillHole = true;		algoBlob.m_bFilterIsUse = false;

		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		

		cv::Mat cvDBCCorrect3D(nHeight, nWidth, CV_32FC1, pFoot->pf3D);

		size_t floatstep = Img3D.step / sizeof(float);
		for (int y = 0; y < nHeight; y++)
		{
			float *correct3dData = (float*)cvDBCCorrect3D.data;
			UCHAR* pOnlyFoot = Foot_Bin.data;

			for (int x = 0; x < nWidth; x++)
			{
				if (correct3dData[y * floatstep + x] > 5)
				{
					pOnlyFoot[y * padImg.step + x] = 255;
				}
			}
		}

		cv::Mat cvOnlyPadBW(nHeight, nWidth, CV_8UC1);
		cvOnlyPadBW = CalcOnlyPadArea(pFoot, padImg.cols, padImg.rows, (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC);

		//Foot BW영상에서 PadCandidateArea 제거
		if(Foot_Bin.rows == cvOnlyPadBW.rows && Foot_Bin.cols == cvOnlyPadBW.cols)
			Foot_Bin -= cvOnlyPadBW;	

		//Blob filtering
		cv::Mat FootBlobImage(nHeight, nWidth, CV_8UC1);
		int nCntBlob = CPInsp::BlobImageStruct(algoBlob, Foot_Bin.data, Img3D.ptr<float>(0), NULL, Foot_Bin.cols, Foot_Bin.rows, 1000,
			&dArea, &dCX, &dCY, &rcBlob, FootBlobImage.data, stTieAreaNULL, true);

		if (PadMaskCalc.rows == Foot_Bin.rows && PadMaskCalc.cols == Foot_Bin.cols)
			PadMaskCalc = PadMaskCalc - Foot_Bin;

		//3. PadImg Morpology & Blob
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11), cv::Point(1, 1));
		cv::Mat kernel_3 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));
		if (pFoot->m_bUsePadAreaAutoTeach)
		{
			cv::erode(PadMaskCalc, PadMaskCalc, kernel_3);
			cv::dilate(PadMaskCalc, PadMaskCalc, kernel_3);
		}
		else
		{
			cv::dilate(PadMaskCalc, PadMaskCalc, kernel);
			cv::erode(PadMaskCalc, PadMaskCalc, kernel);
		}

		
		int nMinBlobArea = 3000;
		if (bIsDBCFoot)
			nMinBlobArea = 300;

		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		int nOnlyPad = CPInsp::BlobImageStruct(algoBlob, PadMaskCalc.data, Img3D.ptr<float>(0), NULL, PadMaskCalc.cols, PadMaskCalc.rows, nMinBlobArea,
			&dArea, &dCX, &dCY, &rcBlob, PadMaskCalc.data, stTieAreaNULL, true);

		if (nOnlyPad == 0)
		{
			//Pad BW teaching 이 잘 안되어 Blob 결과가 없을 경우
			cv::Mat cvimg(nHeight, nWidth, CV_8UC1);
			Calc_PadBW_UseOnly3D(pFoot, &cvimg, nWidth, nHeight);

			PadMaskCalc = cvimg.clone();

			if (PadMaskCalc.rows == Foot_Bin.rows && PadMaskCalc.cols == Foot_Bin.cols)
				PadMaskCalc = PadMaskCalc - Foot_Bin;
		}

		//4.PadImg convexhull
		cv::Mat convexhull(nHeight, nWidth, CV_8UC1);
		convexhull.setTo(0);

		std::vector<cv::Point> vPadPoints, vHulPoints;
		vPadPoints.clear(); vHulPoints.clear();
		for (size_t y = 0; y < nHeight; y++)
		{
			UCHAR* pRaw = PadMaskCalc.ptr<UCHAR>(y);
			for (size_t x = 0; x < nWidth; x++)
				if (pRaw[x] > 0)
					vPadPoints.push_back(cv::Point(x, y));
		}
		if (vPadPoints.size() > 0)
		{
			cv::convexHull(vPadPoints, vHulPoints);
			cv::fillPoly(convexhull, vHulPoints, cv::Scalar(255));
		}
		else
		{
			if (pFoot->m_PadRect.left - pFoot->mImageRect.left > 0 && pFoot->m_PadRect.top - pFoot->mImageRect.top > 0 &&
				pFoot->m_PadRect.right - pFoot->m_PadRect.left > 0 && pFoot->m_PadRect.bottom - pFoot->m_PadRect.top > 0)
			{
				cv::Rect TeachRect(pFoot->m_PadRect.left - pFoot->mImageRect.left, pFoot->m_PadRect.top - pFoot->mImageRect.top
					, pFoot->m_PadRect.right - pFoot->m_PadRect.left, pFoot->m_PadRect.bottom - pFoot->m_PadRect.top);

				cv::rectangle(convexhull, TeachRect, cv::Scalar(255), cv::FILLED);
			}
		}

		//4-1. MaskRect & orgPadImg
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(convexhull, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		double dMxArea = 0;
		int idx = 0;		cv::Rect bMaxbox(0, 0, 0, 0);
		for (contours.begin(); idx < contours.size(); idx++)
		{
			cv::Rect bbox = cv::boundingRect(contours[idx]);
			double area = cv::contourArea(contours[idx]);

			if (area > dMxArea)
			{
				dMxArea = area;
				bMaxbox = bbox;
			}
		}
		if (!pFoot->m_bUsePadAreaAutoTeach)
		{
			if (bMaxbox.width > 0 && bMaxbox.height > 0)
			{
				cv::Mat mPadMaskRectImg(nHeight, nWidth, CV_8UC1);				mPadMaskRectImg.setTo(0);
				cv::rectangle(mPadMaskRectImg, bMaxbox, cv::Scalar(255), cv::FILLED);

				if (PadMaskCalc.rows == mPadMaskRectImg.rows && PadMaskCalc.cols == mPadMaskRectImg.cols)
					PadMaskCalc = mPadMaskRectImg & PadMaskCalc;
			}
		}

		convexhull.setTo(0);
		vPadPoints.clear();
		for (size_t y = 0; y < nHeight; y++)
		{
			UCHAR* pRaw = PadMaskCalc.ptr<UCHAR>(y);
			for (size_t x = 0; x < nWidth; x++)
			{
				if (pRaw[x] > 0)
					vPadPoints.push_back(cv::Point(x, y));
			}
		}
		if (vPadPoints.size() > 0)
		{
			cv::convexHull(vPadPoints, vHulPoints);
			cv::fillPoly(convexhull, vHulPoints, cv::Scalar(255));
		}
		else
		{
			if (pFoot->m_PadRect.left - pFoot->mImageRect.left > 0 && pFoot->m_PadRect.top - pFoot->mImageRect.top > 0 &&
				pFoot->m_PadRect.right - pFoot->m_PadRect.left > 0 && pFoot->m_PadRect.bottom - pFoot->m_PadRect.top > 0)
			{
				cv::Rect TeachRect(pFoot->m_PadRect.left - pFoot->mImageRect.left, pFoot->m_PadRect.top - pFoot->mImageRect.top
					, pFoot->m_PadRect.right - pFoot->m_PadRect.left, pFoot->m_PadRect.bottom - pFoot->m_PadRect.top);
				cv::rectangle(convexhull, TeachRect, cv::Scalar(255), cv::FILLED);
			}
		}

		//5. footImg | padCalc
		if (PadMaskCalc.rows == FootBlobImage.rows && PadMaskCalc.cols == FootBlobImage.cols)
			PadNFootImg = PadMaskCalc + FootBlobImage;

		cv::dilate(PadNFootImg, PadNFootImg, kernel);
		cv::erode(PadNFootImg, PadNFootImg, kernel);

		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		int nCntPadNFoot = CPInsp::BlobImageStruct(algoBlob, PadNFootImg.data, Img3D.ptr<float>(0), NULL, Foot_Bin.cols, Foot_Bin.rows, 1000,
			&dArea, &dCX, &dCY, &rcBlob, PadNFootImg.data, stTieAreaNULL, true);

		//6. convexhull & CalcImg
		cv::Mat DeviationImg(nHeight, nWidth, CV_8UC1);
		if (PadNFootImg.rows == convexhull.rows && PadNFootImg.cols == convexhull.cols)
			DeviationImg = PadNFootImg & convexhull;

		if (bIsDBCFoot)
		{
			cv::erode(DeviationImg, DeviationImg, kernel);
			cv::dilate(DeviationImg, DeviationImg, kernel);

			if (DeviationImg.rows == PadNFootImg.rows && DeviationImg.cols == PadNFootImg.cols)
				DeviationImg = DeviationImg & PadNFootImg;
		}
		else
		{
			cv::dilate(DeviationImg, DeviationImg, kernel);
			cv::dilate(DeviationImg, DeviationImg, kernel);
			cv::erode(DeviationImg, DeviationImg, kernel);
			cv::erode(DeviationImg, DeviationImg, kernel);
		}

		DeviationImg.copyTo(*RepadImg);

		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		nCntBlob = CPInsp::BlobImageStruct(algoBlob, DeviationImg.data, Img3D.ptr<float>(0), NULL, Foot_Bin.cols, Foot_Bin.rows, 1000 /*10000*/,
			&dArea, &dCX, &dCY, &rcBlob, RepadImg->data, stTieAreaNULL, true);

		//7. Teaching Rect
		cv::Mat TeachRectImg(nHeight, nWidth, CV_8UC1);
		TeachRectImg.setTo(0);

		cv::Rect TeachPadRect(pFoot->m_PadRect.left - pFoot->mImageRect.left, pFoot->m_PadRect.top - pFoot->mImageRect.top
			, pFoot->m_PadRect.right - pFoot->m_PadRect.left, pFoot->m_PadRect.bottom - pFoot->m_PadRect.top);
		cv::rectangle(TeachRectImg, TeachPadRect, cv::Scalar(255), cv::FILLED);

		cv::bitwise_and(*RepadImg, TeachRectImg, *RepadImg);

		cv::Mat RstPadImage = *RepadImg;
		bool bChkTemplateImgSize = true;

		if (pFoot->bUseTeachRect && !bTeachBin)
		{
			cv::RotatedRect RoRe;

			RoRe.size.width = pFoot->nPadWidth;//147;
			RoRe.size.height = pFoot->nPadHeight;//166;
			RoRe.center.x = RoRe.size.width / 2 + 0.5;
			RoRe.center.y = RoRe.size.height / 2 + 0.5;
			RoRe.angle = 0.0f;

			cv::Mat ConvexHullImage = DrawRoundPad(RoRe.size.width + 2, RoRe.size.height + 2, RoRe, pFoot->nRadius);

			sRstAlgo->m_nRadius = pFoot->nRadius;
			sRstAlgo->m_nPadWidth = pFoot->nPadWidth;
			sRstAlgo->m_nPadHeight = pFoot->nPadHeight;

			if (ConvexHullImage.cols > RstPadImage.cols || ConvexHullImage.rows > RstPadImage.rows)
			{
				bChkTemplateImgSize = false;
			}
			else
			{
				RstPadImage = MatPolygon(*RepadImg, ConvexHullImage);
			}

		}
		else if (pFoot->bUseTeachRect)
		{
			cv::Rect PadRect(0, 0, 0, 0);
			cv::Mat ConvexHullImage = calcRoundRect(*RepadImg, pFoot->nRadius, PadRect);

			sRstAlgo->m_nRadius = pFoot->nRadius;
			sRstAlgo->m_nPadWidth = PadRect.width;
			sRstAlgo->m_nPadHeight = PadRect.height;

			if (ConvexHullImage.cols > RstPadImage.cols || ConvexHullImage.rows > RstPadImage.rows)
			{
				bChkTemplateImgSize = false;
			}
			else
			{
				RstPadImage = MatPolygon(*RepadImg, ConvexHullImage);
			}

#if _DEBUG
			if (cstDebugFolderPath.IsEmpty() != TRUE)
			{
				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);

				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);

				cv::imwrite(rawname + std::string("_cvFindPadRst.bmp"), RstPadImage);
				cv::imwrite(rawname + std::string("_cvFindConvexHull.bmp"), ConvexHullImage);
				cv::imwrite(rawname + std::string("_cvFindPad_InputPad.bmp"), *RepadImg);
			}
#endif

		}
		else
		{
			sRstAlgo->m_nRadius = pFoot->nRadius;
			sRstAlgo->m_nPadWidth = pFoot->nPadWidth;
			sRstAlgo->m_nPadHeight = pFoot->nPadHeight;
		}

#if _DEBUG
		cv::imwrite("D:\\zRstPadImage.bmp", RstPadImage);
		cv::imwrite("D:\\zRepadImg.bmp", *RepadImg);
#endif
		if (bChkTemplateImgSize)
			RstPadImage.copyTo(*RepadImg);

		if (bIsDBCFoot)
		{
			cv::Mat imgDBCPad2DBWTemp = DeviationImg.clone();
			imgDBCPad2DBWTemp = imgDBCPad2DBWTemp - FootBlobImage;

			//3d 패드높이 이하 영역 제거
			cv::Mat cvTmpDBCOrg3D(imgDBCPad2DBWTemp.rows, imgDBCPad2DBWTemp.cols, CV_32FC1, pFoot->pf3D_OrgROI);
			cv::Mat cvFootDBCCorrect3D(imgDBCPad2DBWTemp.rows, imgDBCPad2DBWTemp.cols, CV_32FC1, pFoot->pf3D);

			float fPadAvgHeight = pFoot->m_fPadArea3DAvgHeight;

			cv::Mat imgRstDBCPad(imgDBCPad2DBWTemp.rows, imgDBCPad2DBWTemp.cols, CV_8UC1);
			imgRstDBCPad.setTo(0);

			imgRstDBCPad = imgDBCPad2DBWTemp.clone();
			size_t floatstep_3DCorrect = cvTmpDBCOrg3D.step / sizeof(float);
			for (int y = 0; y < imgDBCPad2DBWTemp.rows; y++)
			{
				float *raw3dData = (float*)cvTmpDBCOrg3D.data;
				float *correct3dData = (float*)cvFootDBCCorrect3D.data;

				UCHAR* pOnlyPadImg = imgRstDBCPad.data;

				for (int x = 0; x < imgDBCPad2DBWTemp.cols; x++)
				{
					if (raw3dData[y * floatstep_3DCorrect + x] <= fPadAvgHeight + 50)
					{
						pOnlyPadImg[y * imgDBCPad2DBWTemp.step + x] = 255;
					}

					if (raw3dData[y * floatstep_3DCorrect + x] <= fPadAvgHeight - 100)
					{
						pOnlyPadImg[y * imgDBCPad2DBWTemp.step + x] = 0;
					}

					if (correct3dData[y * floatstep_3DCorrect + x] > 5)
					{
						pOnlyPadImg[y * imgDBCPad2DBWTemp.step + x] = 0;
					}
				}
			}


			{
				cv::Mat DBCBinImage(padImg.rows, padImg.cols, CV_8UC1);

				bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

				///////////////////////
				if (bIsFindDBCOutLine)
					imgRstDBCPad = imgRstDBCPad + DBCBinImage;
			}

			for (int y = 0; y < imgDBCPad2DBWTemp.rows; y++)
			{
				float *raw3dData = (float*)cvTmpDBCOrg3D.data;
				float *correct3dData = (float*)cvFootDBCCorrect3D.data;

				UCHAR* pOnlyPadImg = imgRstDBCPad.data;

				for (int x = 0; x < imgDBCPad2DBWTemp.cols; x++)
				{
					if (correct3dData[y * floatstep_3DCorrect + x] > 5)
					{
						pOnlyPadImg[y * imgDBCPad2DBWTemp.step + x] = 0;
					}

				}
			}

			cv::erode(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::erode(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::erode(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::erode(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::erode(imgRstDBCPad, imgRstDBCPad, kernel_3);

			cv::dilate(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::dilate(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::dilate(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::dilate(imgRstDBCPad, imgRstDBCPad, kernel_3);
			cv::dilate(imgRstDBCPad, imgRstDBCPad, kernel_3);

			cv::Mat cvOnlyDBCPadBWImg = imgRstDBCPad - FootBlobImage;

			//Blob Filtering 으로 Pad 부분 작은 입자 제거
			//최종 패드이미지 추출
			if (pFoot->m_nFootAngle == 270)
			{
				algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectPosition;
				nCntBlob = m_pProcMilAlgo->CalcBlob_Select(cvOnlyDBCPadBWImg.data, cvOnlyDBCPadBWImg.data, cvOnlyDBCPadBWImg.cols, cvOnlyDBCPadBWImg.rows, 1000, FALSE, TRUE, 0, eSelectPosition, pFoot->m_FootCenterPoint_ImageRect.x, pFoot->m_FootCenterPoint_ImageRect.y);
				
			}
			else
			{
				algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
				nCntBlob = CPInsp::BlobImageStruct(algoBlob, cvOnlyDBCPadBWImg.data, Img3D.ptr<float>(0), NULL, cvOnlyDBCPadBWImg.cols, cvOnlyDBCPadBWImg.rows, 300 /*10000*/,
					&dArea, &dCX, &dCY, &rcBlob, cvOnlyDBCPadBWImg.data, stTieAreaNULL, true);
			}

			{
				//주변 DBC바닥이 함께 검출되는 경우, 티칭 위치에서 거리 확인 후 제외
				int nDBCPadTeachW = pFoot->nFootPadSizeX / (m_resolX * 1000);
				int nDBCPadTeachH = pFoot->nFootPadSizeX / (m_resolY * 1000);

				int nTeachMarginRectX = 0, nTeachMarginRectY = 0;
				int nTeachMarginRectW = 0, nTeachMarginRectH = 0;

				nTeachMarginRectX = pFoot->m_FootCenterPoint_ImageRect.x - (nDBCPadTeachW / 2);
				nTeachMarginRectY = pFoot->m_FootCenterPoint_ImageRect.y - (nDBCPadTeachH / 2);

				nTeachMarginRectW = nDBCPadTeachW;
				nTeachMarginRectH = nDBCPadTeachH;

				cv::Mat cvDBCPadRectRange(nHeight, nWidth, CV_8UC1);
				cvDBCPadRectRange.setTo(0);

				if (nTeachMarginRectX + (nDBCPadTeachW) >= nWidth)
					nDBCPadTeachW = nWidth - nTeachMarginRectX;
				else
					nDBCPadTeachW = (nDBCPadTeachW);

				if (nTeachMarginRectY + (nDBCPadTeachH) >= nHeight)
					nDBCPadTeachH = nHeight - nTeachMarginRectY;
				else
					nDBCPadTeachH = (nDBCPadTeachH);

				cv::rectangle(cvDBCPadRectRange, cv::Rect(nTeachMarginRectX, nTeachMarginRectY, nDBCPadTeachW, nDBCPadTeachH), cv::Scalar(255), cv::FILLED);

				cv::Mat cvDBCPadBlobRst(nHeight, nWidth, CV_8UC1);
				cvDBCPadBlobRst.setTo(0);

				cv::Mat cvBlobInputTmp(nHeight, nWidth, CV_8UC1);
				cvBlobInputTmp.setTo(0);
				cvBlobInputTmp = cvOnlyDBCPadBWImg.clone();

				if (nCntBlob > 1)
				{
					for (int i = 0; i < nCntBlob; i++)
					{
						int nDBCPadBlob = 0;

						cv::Mat cvDBCPadBlobTemp(nHeight, nWidth, CV_8UC1);
						cvDBCPadBlobTemp.setTo(0);

						algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectPosition;
						nDBCPadBlob = m_pProcMilAlgo->CalcBlob_Select(cvBlobInputTmp.data, cvDBCPadBlobTemp.data, cvDBCPadBlobRst.cols, cvDBCPadBlobRst.rows, 1000, FALSE, TRUE, 0, eSelectPosition, pFoot->m_FootCenterPoint_ImageRect.x, pFoot->m_FootCenterPoint_ImageRect.y);

						bool bFindPadRect_InCandidatesRange = false;

						for (int y = 0; y < cvDBCPadBlobTemp.rows; y++)
						{
							UCHAR* pPadCandidates = cvDBCPadRectRange.data;
							UCHAR* pOnlyPad = cvDBCPadBlobTemp.data;

							for (int x = 0; x < cvDBCPadBlobTemp.cols; x++)
							{
								if (pPadCandidates[y * cvDBCPadBlobRst.step + x] == 255 && pOnlyPad[y * cvDBCPadBlobRst.step + x] == 255)
								{
									bFindPadRect_InCandidatesRange = true;
									break;
								}
							}
						}

						if (bFindPadRect_InCandidatesRange == true)
						{
							cvDBCPadBlobRst = cvDBCPadBlobRst | cvDBCPadBlobTemp;
							cvBlobInputTmp = cvBlobInputTmp - cvDBCPadBlobTemp;
						}

					}

					cvOnlyDBCPadBWImg = cvDBCPadBlobRst.clone();
				}

				cv::Mat convexhullDBCimg(nHeight, nWidth, CV_8UC1);
				std::vector<cv::Point> vPadPoints_2dDBCPad, vHulPoints_2dDBCPad;
				convexhullDBCimg.setTo(0);

				vPadPoints_2dDBCPad.clear();
				for (size_t y = 0; y < nHeight; y++)
				{
					UCHAR* pRaw = cvOnlyDBCPadBWImg.ptr<UCHAR>(y);
					for (size_t x = 0; x < nWidth; x++)
						if (pRaw[x] > 0)
							vPadPoints_2dDBCPad.push_back(cv::Point(x, y));
				}
				if (vPadPoints_2dDBCPad.size() > 0)
				{
					cv::convexHull(vPadPoints_2dDBCPad, vHulPoints_2dDBCPad);
					cv::fillPoly(convexhullDBCimg, vHulPoints_2dDBCPad, cv::Scalar(255));
				}

				cv::Mat cvDBCimg(nHeight, nWidth, CV_8UC1);
				cvDBCimg.setTo(0);

				convexhullDBCimg.copyTo(cvDBCimg);

				cvDBCimg = PadNFootImg & cvDBCimg;

				cvDBCimg.copyTo(*RepadImg);
			}

			if (pFoot->bUseDBCShapeTeaching)		//DBC 폴리곤 모양 티칭되었을 경우
			{
				//pad 영역 네모 라인 검출
				int nDBCPadTeachSizeX = 720 / (m_resolX * 1000);
				int nDBCPadTeachSizeY = 850 / (m_resolY * 1000);

				nDBCPadTeachSizeX = pFoot->nFootPadSizeX / (m_resolX * 1000);
				nDBCPadTeachSizeY = pFoot->nFootPadSizeY / (m_resolY * 1000);

				cv::Mat cvRstDBCPadImg(nHeight, nWidth, CV_8UC1);
				cvRstDBCPadImg.setTo(0);

				float fRatioPadLine = 0.7;
				if (pFoot->nPadEdgeMinRatio != 0)
					fRatioPadLine = (float)pFoot->nPadEdgeMinRatio / 100.0;

				pFoot->nRadius = 30;

				float fRatioBlackLine = 0.7;

				bool bIsFoundDBCFoot = Find_PadRect_UsePadBWAnd3D(pFoot, &cvRstDBCPadImg, cvOnlyDBCPadBWImg, nWidth, nHeight, nDBCPadTeachSizeX, nDBCPadTeachSizeY, fRatioPadLine, fRatioBlackLine, true, PadNFootImg);


				//테스트용 디버깅 이미지 저장
#if _DEBUG
				if (cstDebugFolderPath.IsEmpty() != TRUE)
				{
					CT2CA convertedString(cstDebugFolderPath);
					std::string strFootDebugImgPath = std::string(convertedString);

					size_t lastindex = strFootDebugImgPath.find_last_of(".");
					string rawname = strFootDebugImgPath.substr(0, lastindex);

					int nWidthR = pFoot->GetImageWidth();
					int nHeightR = pFoot->GetImageLength();
					cv::Mat cvPad2DImg(nHeightR, nWidthR, CV_8UC1);
					pFoot->GetImageMatrix(&cvPad2DImg, (int)m_eFootBin::m_eFootBin_Pad);

					cv::Mat RstPadUIimage, testPadimage1;
					cv::cvtColor(cvPad2DImg, testPadimage1, cv::COLOR_GRAY2RGB);

					cv::Mat cvTmp = (cvRstDBCPadImg).clone();
					cv::Mat testPadEdgeimage1;
					cv::Mat scale1;
					cv::Canny(cvTmp, scale1, 100, 200, 3, false);
					cv::cvtColor(scale1, testPadEdgeimage1, cv::COLOR_GRAY2RGB);

					double alpha = 0.5;
					cv::addWeighted(testPadimage1, alpha, testPadEdgeimage1, (1 - alpha), 0, RstPadUIimage);

					cv::imwrite(rawname + std::string("_0_PadImg_Rect.bmp"), RstPadUIimage);

				}
#endif

				if (bIsFoundDBCFoot)
				{
					//Calc Result DBC PadLine
					int nDBCRstPadRectMinX = nWidth;
					int nDBCRstPadRectMaxX = 0;
					int nDBCRstPadRectMinY = nHeight;
					int nDBCRstPadRectMaxY = 0;
					for (int r = 0; r < nHeight; r++)
					{
						UCHAR* Ptr = cvRstDBCPadImg.data;
						for (int c = 0; c < nWidth; c++)
						{
							if (Ptr[r * cvRstDBCPadImg.step + c] == 255)
							{
								if (r < nDBCRstPadRectMinY)	nDBCRstPadRectMinY = r;
								if (r > nDBCRstPadRectMaxY)	nDBCRstPadRectMaxY = r;
								if (c < nDBCRstPadRectMinX)	nDBCRstPadRectMinX = c;
								if (c > nDBCRstPadRectMaxX)	nDBCRstPadRectMaxX = c;

							}
						}
					}
					int nDBCPolygonPaddingW = 0, nDBCPolygonPaddingH = 0;
					nDBCPolygonPaddingW = cvRstDBCPadImg.cols - nDBCRstPadRectMinX;
					nDBCPolygonPaddingH = cvRstDBCPadImg.rows - nDBCRstPadRectMinY;

					int nWndW = pFoot->mWindowImageRect.right - pFoot->mWindowImageRect.left;
					int nWndH = pFoot->mWindowImageRect.bottom - pFoot->mWindowImageRect.top;

					cv::Mat cvDBCPolygonImage(nWndH, nWndW, CV_8UC1, pFoot->m_pDBCPolygonImg);

					//Calc DBC Teach Polygon Area
					int nDBCPolygonPadRectMinX = nWndW;
					int nDBCPolygonPadRectMaxX = 0;
					int nDBCPolygonPadRectMinY = nWndH;
					int nDBCPolygonPadRectMaxY = 0;

					for (int r = 0; r < nWndH; r++)
					{
						UCHAR* Ptr = cvDBCPolygonImage.data;
						for (int c = 0; c < nWndW; c++)
						{
							if (Ptr[r * cvDBCPolygonImage.step + c] == 255)
							{
								if (r < nDBCPolygonPadRectMinY)	nDBCPolygonPadRectMinY = r;
								if (r > nDBCPolygonPadRectMaxY)	nDBCPolygonPadRectMaxY = r;
								if (c < nDBCPolygonPadRectMinX)	nDBCPolygonPadRectMinX = c;
								if (c > nDBCPolygonPadRectMaxX)	nDBCPolygonPadRectMaxX = c;

							}
						}
					}


					if (nDBCPolygonPadRectMinX != nWndW && nDBCPolygonPadRectMaxX != 0 &&
						nDBCPolygonPadRectMinY != nWndH && nDBCPolygonPadRectMaxY != 0)
					{

						if (nDBCPolygonPadRectMinX + nDBCPolygonPaddingW >= cvDBCPolygonImage.cols)
							nDBCPolygonPaddingW = cvDBCPolygonImage.cols-1- nDBCPolygonPadRectMinX;

						if (nDBCPolygonPadRectMinY + nDBCPolygonPaddingH >= cvDBCPolygonImage.rows)
							nDBCPolygonPaddingH = cvDBCPolygonImage.rows-1 - nDBCPolygonPadRectMinY;

						cv::Mat cvDBCPolyImg = cvDBCPolygonImage(cv::Rect(nDBCPolygonPadRectMinX, nDBCPolygonPadRectMinY, nDBCPolygonPaddingW, nDBCPolygonPaddingH));

						cv::Mat cvRstDBCPadImg(nHeight, nWidth, CV_8UC1);
						cvRstDBCPadImg.setTo(0);
						cv::Mat A = cvRstDBCPadImg(cv::Rect(nDBCRstPadRectMinX, nDBCRstPadRectMinY, nDBCPolygonPaddingW, nDBCPolygonPaddingH));

						cvDBCPolyImg.copyTo(A);

						cvRstDBCPadImg.copyTo(*RepadImg);
					}
				}
			}

			//테스트용 디버깅 이미지 저장
#if _DEBUG
			if (cstDebugFolderPath.IsEmpty() != TRUE)
			{
				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);

				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);
				
				int nWidthR = pFoot->GetImageWidth();
				int nHeightR = pFoot->GetImageLength();
				cv::Mat cvPad2DImg(nHeightR, nWidthR, CV_8UC1);
				pFoot->GetImageMatrix(&cvPad2DImg, (int)m_eFootBin::m_eFootBin_Pad);

				cv::Mat RstPadUIimage, testPadimage1;
				cv::cvtColor(cvPad2DImg, testPadimage1, cv::COLOR_GRAY2RGB);

				cv::Mat cvTmp = (*RepadImg).clone();
				cv::Mat testPadEdgeimage1;
				cv::Mat scale1;
				cv::Canny(cvTmp, scale1, 100, 200, 3, false);
				cv::cvtColor(scale1, testPadEdgeimage1, cv::COLOR_GRAY2RGB);

				double alpha = 0.5;
				cv::addWeighted(testPadimage1, alpha, testPadEdgeimage1, (1 - alpha), 0, RstPadUIimage);

				cv::imwrite(rawname + std::string("_0_PadImg.bmp"), RstPadUIimage);

			}
#endif
			

		}

		if (!bIsDBCFoot)
		{
			cv::Mat imgPad2DBWTemp = PadMaskCalc.clone();

			cv::Mat cvPadandFootImg(nHeight, nWidth, CV_8UC1);
			cvPadandFootImg.setTo(0);
			algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

			if(pFoot->m_bUse2Foot)
				algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

			int nCntPadNFoot = CPInsp::BlobImageStruct(algoBlob, PadNFootImg.data, Img3D.ptr<float>(0), NULL, Foot_Bin.cols, Foot_Bin.rows, 10000 /*10000*/,
				&dArea, &dCX, &dCY, &rcBlob, cvPadandFootImg.data, stTieAreaNULL, true);

			imgPad2DBWTemp = cvPadandFootImg & imgPad2DBWTemp;

			//3d 패드높이 이상 영역은 제거
			cv::Mat cvTmpOrg3D(imgPad2DBWTemp.rows, imgPad2DBWTemp.cols, CV_32FC1, pFoot->pf3D_OrgROI);
			float fPadAvgHeight = pFoot->m_fPadArea3DAvgHeight;

			cv::Mat cvFootCorrect3D(imgPad2DBWTemp.rows, imgPad2DBWTemp.cols, CV_32FC1, pFoot->pf3D);

			size_t floatstep_3DCorrect = cvTmpOrg3D.step / sizeof(float);
			for (int y = 0; y < imgPad2DBWTemp.rows; y++)
			{
				float *rawData = (float*)cvTmpOrg3D.data;
				float *correct3dData = (float*)cvFootCorrect3D.data;

				UCHAR* pOnlyPadImg = imgPad2DBWTemp.data;

				for (int x = 0; x < imgPad2DBWTemp.cols; x++)
				{
					if (rawData[y * floatstep_3DCorrect + x] <= fPadAvgHeight + 50 && pOnlyPadImg[y * imgPad2DBWTemp.step + x] == 255)
					{
						pOnlyPadImg[y * imgPad2DBWTemp.step + x] = 255;
					}

					if (rawData[y * floatstep_3DCorrect + x] <= fPadAvgHeight - 50)
					{
						pOnlyPadImg[y * imgPad2DBWTemp.step + x] = 0;
					}

					if (correct3dData[y * floatstep_3DCorrect + x] > 5)
					{
						pOnlyPadImg[y * imgPad2DBWTemp.step + x] = 0;
					}
					if (correct3dData[y * floatstep_3DCorrect + x] < -100)
					{
						pOnlyPadImg[y * imgPad2DBWTemp.step + x] = 0;
					}

				}
			}

			cv::erode(imgPad2DBWTemp, imgPad2DBWTemp, kernel_3);
			cv::dilate(imgPad2DBWTemp, imgPad2DBWTemp, kernel_3);

			//Blob Filtering 으로 Pad 부분 작은 입자 제거
			algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
			nCntBlob = CPInsp::BlobImageStruct(algoBlob, imgPad2DBWTemp.data, Img3D.ptr<float>(0), NULL, imgPad2DBWTemp.cols, imgPad2DBWTemp.rows, 3000 /*10000*/,
				&dArea, &dCX, &dCY, &rcBlob, imgPad2DBWTemp.data, stTieAreaNULL, true);

			//최종 패드이미지 추출
			cv::Mat cvOnlyPadBWImg = imgPad2DBWTemp - FootBlobImage;

			//Find Pad Rect Image
			cv::Mat cvRstPadImg(nHeight, nWidth, CV_8UC1);
			cvRstPadImg.setTo(0);

			int nPadTeachSizeX = 900 / (m_resolX * 1000);	//mobis IGBT pad 크기 : 900*900um (모비스 박준규 책임이 알려준 2022.02.06일 패드 크기)
			int nPadTeachSizeY = 900 / (m_resolY * 1000);
			nPadTeachSizeX = pFoot->nFootPadSizeX / (m_resolX * 1000);
			nPadTeachSizeY = pFoot->nFootPadSizeY / (m_resolY * 1000);

			float fRatioPadLine = 0.5;
			if (pFoot->nPadEdgeMinRatio != 0)
				fRatioPadLine = (float)pFoot->nPadEdgeMinRatio / 100.0;

			bool bMakeRoundPad = false;

			if (pFoot->bUseTeachRect) bMakeRoundPad = true;
			float fRatioBlackLine = 0.7;

			bool bIsFoundFoot = Find_PadRect_UsePadBWAnd3D(pFoot, &cvRstPadImg, cvOnlyPadBWImg, nWidth, nHeight, nPadTeachSizeX, nPadTeachSizeY, fRatioPadLine, fRatioBlackLine, bMakeRoundPad, PadNFootImg);

			if (bIsFoundFoot)
				cvRstPadImg.copyTo(*RepadImg);

		}

		return true;
	}
	catch (...)
	{

	}
	return true;
}


bool CPInsp_Foot::Calc_PadBW_UseOnly3D(CFoot_Model *pFoot, cv::Mat* cvRstAutoInspPad, int nSrcWidth, int nSrcHeight)
{
	int wid = nSrcWidth;
	int len = nSrcHeight;

	//1. Foot 3D 가우시안 필터링
	cv::Mat cvOrgFoot3D(len, wid, CV_32FC1, pFoot->pf3D_OrgROI);
	cv::Mat cvTmpFoot3D = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

	bool bIsDBCFoot = false;

	if (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		bIsDBCFoot = true;

	if (bIsDBCFoot)
		cv::GaussianBlur(cvOrgFoot3D, cvTmpFoot3D, cv::Size(11, 11), 0);
	else
		cvTmpFoot3D = cvOrgFoot3D.clone();

	//2. Foot 3D 기울기 X,Y 계산
	cv::Mat Xgrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	cv::Mat Ygrad = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

	size_t floatstep_3DCorrect = cvTmpFoot3D.step / sizeof(float);
	for (int y = 1; y < len - 1; y++)
	{
		float * pXgrad = (float *)Xgrad.data;
		float * pYgrad = (float *)Ygrad.data;
		float *rawData = (float*)cvTmpFoot3D.data;

		for (int x = 1; x < wid - 1; x++)
		{
			pXgrad[y*floatstep_3DCorrect + x] = rawData[y * floatstep_3DCorrect + (x + 1)] - rawData[y * floatstep_3DCorrect + (x - 1)];
			pYgrad[y*floatstep_3DCorrect + x] = rawData[(y + 1) * floatstep_3DCorrect + x] - rawData[(y - 1) * floatstep_3DCorrect + x];
		}
	}

	//3. Foot 3D - 가우시안 필터 적용한 3D 중 기울기 10 이하인 영역만 추출
	cv::Mat tempImage(len, wid, CV_8UC1);
	tempImage.setTo(0);

	cv::Mat Rst3DImage = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);
	float fMaxHeight = 0;
	float fMinHeight = 10000;

	float fPad3DAvgHeigt = 0.0f;
	float fRefAreaHgt = 0.0f;
	int nRefAreaCnt = 0;

	cv::Mat DBCBinImage(len, wid, CV_8UC1);
	DBCBinImage.setTo(0);
	if (bIsDBCFoot)
	{
		bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

		///////////////////////
		//if (bIsFindDBCOutLine)
		//	PadMaskCalc = PadMaskCalc + DBCBinImage;
	}

	for (int i = 0; i < len; i++)
	{
		float * pXgrad = (float*)Xgrad.data;
		float * pYgrad = (float*)Ygrad.data;

		UCHAR* pResult2D = tempImage.data;
		float* rowptr = (float*)cvTmpFoot3D.data;
		float* ptrRst3D = (float*)Rst3DImage.data;

		for (int j = 0; j < wid; j++)
		{
			float val = rowptr[i * floatstep_3DCorrect + j];

			if (bIsDBCFoot)
			{
				if (((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 10 
					&& (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 10))
				{
					if (isnan(val) == true)
						continue;

					if (rowptr[i * floatstep_3DCorrect + j] > 100 || rowptr[i * floatstep_3DCorrect + j] < -100)
						continue;

					ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
					pResult2D[i * tempImage.step + j] = 255;		//2D

					if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
						fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
					if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
						fMinHeight = rowptr[i * floatstep_3DCorrect + j];

				}
			}
			else
			{
				if (((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 5 
					&& (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 5))
				{
					if (isnan(val) == true)
						continue;

					ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
					pResult2D[i * tempImage.step + j] = 255;		//2D

					if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
						fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
					if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
						fMinHeight = rowptr[i * floatstep_3DCorrect + j];

				}
			}
		}

	}
	cv::Mat grayImg = Rst3DImage.clone();

	//4. 위의 두 조건을 만족하는 영역 중에서도, 최빈도 높이 값의 ±20 범위에 드는 부분을 최종 Background 로 추출
	int nTolerance = 50;
	float fMaxFeqHeight = GetHeightMaxHist(grayImg.ptr<float>(), wid, len, fMaxHeight, fMinHeight, bIsDBCFoot);

	if (fMaxHeight - (pFoot->nRemoveWireHeight - 20) < fMaxFeqHeight)
	{
		tempImage.setTo(0);

		cv::Mat Rst3DImage_ReCalc = cv::Mat::zeros(cv::Size(wid, len), CV_32FC1);

		//Pad 영역의 최빈도높이 > 3D최고높이-와이어제한높이 일경우, 최빈도 높이 재계산
		for (int i = 0; i < len; i++)
		{
			float * pXgrad = (float*)Xgrad.data;
			float * pYgrad = (float*)Ygrad.data;

			UCHAR* pResult2D = tempImage.data;
			float* rowptr = (float*)cvTmpFoot3D.data;
			float* ptrRst3D = (float*)Rst3DImage_ReCalc.data;

			for (int j = 0; j < wid; j++)
			{
				float val = rowptr[i * floatstep_3DCorrect + j];

				if (bIsDBCFoot)
				{
					if (((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 10 
						&& (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 10))
					{
						if (isnan(val) == true)
							continue;

						if (rowptr[i * floatstep_3DCorrect + j] > 100 || rowptr[i * floatstep_3DCorrect + j] < -100)
							continue;

						ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
						pResult2D[i * tempImage.step + j] = 255;		//2D

						if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
							fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
						if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
							fMinHeight = rowptr[i * floatstep_3DCorrect + j];

					}
				}
				else
				{
					if (((int)(fabs(pYgrad[i * floatstep_3DCorrect + j])) < 5 
						&& (int)(fabs(pXgrad[i * floatstep_3DCorrect + j])) < 5) 
						&& rowptr[i * floatstep_3DCorrect + j] < fMaxFeqHeight - 10)
					{
						if (isnan(val) == true)
							continue;

						ptrRst3D[i * floatstep_3DCorrect + j] = val;	//3D
						pResult2D[i * tempImage.step + j] = 255;		//2D

						if (fMaxHeight < rowptr[i * floatstep_3DCorrect + j])
							fMaxHeight = rowptr[i * floatstep_3DCorrect + j];
						if (fMinHeight > rowptr[i * floatstep_3DCorrect + j])
							fMinHeight = rowptr[i * floatstep_3DCorrect + j];

					}
				}
			}

		}

		cv::Mat grayImg = Rst3DImage_ReCalc.clone();

		fMaxFeqHeight = GetHeightMaxHist(grayImg.ptr<float>(), wid, len, fMaxHeight, fMinHeight, bIsDBCFoot);

	}

	cv::Mat cvAutoInsp_pad2D(cvTmpFoot3D.rows, cvTmpFoot3D.cols, CV_8UC1);
	cvAutoInsp_pad2D.setTo(0);

	for (int i = 0; i < len; i++)
	{
		UCHAR* pRstPad2DImg = cvAutoInsp_pad2D.data;
		float* rowptr = (float*)cvOrgFoot3D.data;

		for (int j = 0; j < wid; j++)
		{
			int val = rowptr[i * floatstep_3DCorrect + j];

			if (fMaxFeqHeight - nTolerance < val && val < fMaxFeqHeight + nTolerance)
			{
				pRstPad2DImg[i * cvAutoInsp_pad2D.step + j] = 255;

			}

		}
	}
	
	cvAutoInsp_pad2D.copyTo(*cvRstAutoInspPad);

	return true;
}

bool CPInsp_Foot::FindWing_UseOnly3D(CFoot_Model *pFoot, cv::Mat* cvRstAutoInspWing, int nSrcWidth, int nSrcHeight)
{
	//Detect Melting Area Using 3D Height
	cv::Mat Img3D_Candidates(nSrcHeight, nSrcWidth, CV_32FC1, pFoot->pf3D);

	for (int y = 0; y < Img3D_Candidates.rows; y++)
	{
		float* ptrInputImg = Img3D_Candidates.ptr<float>(y);

		for (int x = 0; x < Img3D_Candidates.cols; x++)
		{
			if (ptrInputImg[x] < 40)
			{
				ptrInputImg[x] = 0;
			}

		}
	}

	//Sobel Edge Detect
	cv::Mat gradX, gradY;
	cv::Mat gradX_ConvertImg(nSrcHeight, nSrcWidth, CV_16SC1);
	cv::Mat gradY_ConvertImg(nSrcHeight, nSrcWidth, CV_16SC1);

	cv::Sobel(Img3D_Candidates, gradX, CV_32FC1, 1, 0, 3);
	cv::Sobel(Img3D_Candidates, gradY, CV_32FC1, 0, 1, 3);
	cv::convertScaleAbs(gradX, gradX_ConvertImg, 1, 0);
	cv::convertScaleAbs(gradY, gradY_ConvertImg, 1, 0);

	int nInputRemoveWireH = 220;
	if (pFoot->nRemoveWireHeight > 0)
	{
		nInputRemoveWireH = pFoot->nRemoveWireHeight;
	}

	//Detect Wing Area Using SobelX Gradient
	cv::Mat cvFootSobelXResultImg(nSrcHeight, nSrcWidth, CV_8UC1);
	cvFootSobelXResultImg.setTo(0);

	size_t floatstep_3DCorrect = Img3D_Candidates.step / sizeof(float);
	for (int y = 0; y < gradX_ConvertImg.rows; y++)
	{
		uchar * ptrSobelX = gradX_ConvertImg.data;
		uchar * ptrSobelXRes = cvFootSobelXResultImg.data;
		float* rowptr = (float*)Img3D_Candidates.data;

		for (int x = 0; x < gradX_ConvertImg.cols; x++)
		{
			int val = ptrSobelX[y * gradX_ConvertImg.step + x];
			if (val > pFoot->nFootSobmin && val < pFoot->nFootSobmax && rowptr[y * floatstep_3DCorrect + x] < nInputRemoveWireH)
			{
				ptrSobelXRes[y * gradX_ConvertImg.step + x] = 255;
			}
		}
	}

	cvFootSobelXResultImg.copyTo(*cvRstAutoInspWing);

	return true;
}

bool CPInsp_Foot::FindFootAngle_UseBlobLine(CFoot_Model *pFoot, cv::Mat* cvRstFootLine, double *dRstThetaFoot, cv::Point *poLineSt, cv::Point *poLineEd, cv::Point *poFootCenter, cv::Mat cvSrcFootImage, int nSrcWidth, int nSrcHeight)
{
	
	cv::Mat result_img(cvSrcFootImage.size(), CV_8UC1, cv::Scalar(0));
	cv::Mat cvSrcImg = cvSrcFootImage.clone();

	bool bIsDBCFoot = false;

	if (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		bIsDBCFoot = true;

	int nCntB = m_pProcMilAlgo->CalcBlob_Select(cvSrcImg.ptr(), result_img.ptr(), cvSrcFootImage.cols, cvSrcFootImage.rows, 4, FALSE, FALSE, FALSE, eSelectBigger);

	m_pProcMilAlgo->GetDrawContours(result_img.ptr(), result_img.cols, result_img.rows, result_img.step, 255);

	std::vector<POINT> vec;
	vec.reserve(100);

	for (int i = 0; i < result_img.rows; i++)
	{
		UCHAR* Ptr = result_img.data;

		for (int j = 0; j < result_img.cols; j++)
		{
			int val = Ptr[i * result_img.step + j];
			if (val == 255)
			{
				POINT poEdge;
				poEdge.x = j;
				poEdge.y = i;

				vec.push_back(poEdge);
			}
		}
	}

	int nFootDir = pFoot->m_nFootAngle;

	//LSQ Line Fitting 으로 angle 추출////////////////////////////
	double pdA = 0;
	double pdB = 0;
	bool bVertical = false;
	bool bStraight = false;
	LSQLineFit(vec, pdA, pdB, &bVertical, &bStraight);

	POINTF x1, x2;
	GetLinePoint(bStraight, bVertical, pdA, pdB, vec, x1, x2);

	cv::Point BSt1, BEd1;
	BSt1.x = x1.x;	BSt1.y = x1.y;
	BEd1.x = x2.x;	BEd1.y = x2.y;

	*poLineSt = BSt1;
	*poLineEd = BEd1;

	int nFootCtrX = 0, nFootCtrY = 0;

	if (BSt1.y < BEd1.y)
	{
		nFootCtrY = BSt1.y + (abs(BEd1.y - BSt1.y) / 2);
	}
	else
		nFootCtrY = BEd1.y + (abs(BSt1.y - BEd1.y) / 2);

	if (BSt1.x < BEd1.x)
	{
		nFootCtrX = BSt1.x + (abs(BEd1.x - BSt1.x) / 2);
	}
	else
		nFootCtrX = BEd1.x + (abs(BEd1.x - BSt1.x) / 2);

	poFootCenter->x = nFootCtrX;
	poFootCenter->y = nFootCtrY;

	cv::Mat cvRstLineFit(nSrcHeight, nSrcWidth, CV_8UC1, cv::Scalar(0));
	cv::line(cvRstLineFit, BSt1, BEd1, cv::Scalar(255));

	cvRstLineFit.copyTo(*cvRstFootLine);

	//Find Foot Lines and Check Line Length. if Line Length < Foot min Length * 2/3, 검출한 라인 신뢰 불가
	double dLineDistance = sqrt(pow(BSt1.x - BEd1.x, 2) + pow(BSt1.y - BEd1.y, 2));

	if (cstDebugFolderPath.IsEmpty() != TRUE)
	{
		CT2CA convertedString(cstDebugFolderPath);
		std::string strFootDebugImgPath = std::string(convertedString);

		size_t lastindex = strFootDebugImgPath.find_last_of(".");
		string rawname = strFootDebugImgPath.substr(0, lastindex);

		cv::imwrite(rawname + std::string("_FindFoot3DBlob_MorpImg.bmp"), cvSrcFootImage);
		cv::imwrite(rawname + std::string("_FindFoot3DBlob_DetectLine.bmp"), cvRstLineFit);
	}

	//Foot Line Length 전량파트 검증 후 주석 해제 예정
	if (((pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min] * 2) / 3) > (dLineDistance*m_resolX) && !bIsDBCFoot)
	{
		return false;
	}

	//Line 각도 추출
	int dx = BSt1.x - BEd1.x;
	int dy = BSt1.y - BEd1.y;

	if (BSt1.y < BEd1.y)
	{
		dx = BEd1.x - BSt1.x;
		dy = BEd1.y - BSt1.y;
	}

	double dstdRadian = atan2((double)dx, (double)dy);
	double dstdDegree = (dstdRadian * 180) / PI;

	*dRstThetaFoot = 180 - dstdDegree;

	if (135 >= nFootDir && nFootDir > 45)
		*dRstThetaFoot = -dstdDegree;
	else if (225 >= nFootDir && nFootDir > 135) {}
	else if (315 >= nFootDir && nFootDir > 225) {}
	else
		*dRstThetaFoot = -dstdDegree;

	if (45 >= pFoot->m_nFootAngle || pFoot->m_nFootAngle > 315)// ^
	{
		if (abs(*dRstThetaFoot) > 90)
		{
			*dRstThetaFoot += 180;
		}
	}
	else if (225 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 135)// V
	{
		if (abs(*dRstThetaFoot) < 90)
		{
			*dRstThetaFoot += 180;
		}
	}
	else if (135 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 45)// <
	{
		while (*dRstThetaFoot < 0) { *dRstThetaFoot += 360; }
		if (abs(*dRstThetaFoot) < 180)
		{
			*dRstThetaFoot += 180;
		}
	}
	else if (315 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 225)// >
	{
		while (*dRstThetaFoot < 0) { *dRstThetaFoot += 360; }
		if (abs(*dRstThetaFoot) > 180)
		{
			*dRstThetaFoot += 180;
		}
	}

	return true;
}

bool CPInsp_Foot::Calc3DEdge_SSEImg(cv::Mat cvSrc3DImg, cv::Mat &cvDst3DImg)
{

	bool bIsLoad = false;
	if (cvSrc3DImg.rows == 0 || cvSrc3DImg.cols == 0)
	{
		return bIsLoad;
	}

	int nImageW = cvSrc3DImg.cols, nImageH = cvSrc3DImg.rows;

	cv::Mat dst(cv::Size(nImageW, nImageH), CV_32FC1);

	float* pSrcData = (float*)cvSrc3DImg.data;
	float* pDstData = (float*)dst.data;

	size_t floatstep_3DCorrect = cvSrc3DImg.step / sizeof(float);
	__m128 p1, p2, p3, p4;
	__m128 gx, gy, G;
	__m128 sign_mask = _mm_set1_ps(-0.f); // -0.f = 1 << 31

	pSrcData += floatstep_3DCorrect;
	pDstData += floatstep_3DCorrect;

	int x;
	for (int y = 1; y < nImageH - 1; y++)
	{
		for (x = 1; x < nImageW - 1; x += 4)
		{
			/*
			operator input matrix
			+~~~~~~~~~~~~~~+
			|    | p3 |    |
			|~~~~+~~~~+~~~~+
			| p1 |    | p2 |
			|~~~~+~~~~+~~~~+
			|    | p4 |    |
			+~~~~+~~~~+~~~~+
			X Grad:  p2 - p1
			Y Grad:  p4 - p3
			*/
			p1 = _mm_loadu_ps(pSrcData + (x - 1));
			p2 = _mm_loadu_ps(pSrcData + (x + 1));
			p3 = _mm_loadu_ps(pSrcData - floatstep_3DCorrect + x);
			p4 = _mm_loadu_ps(pSrcData + floatstep_3DCorrect + x);

			//X Grad:  p2 - p1
			gx = _mm_sub_ps(p2, p1);

			// Y Grad:  p4 - p3
			gy = _mm_sub_ps(p4, p3);

			//G = [gx] + [gy]
			gx = _mm_andnot_ps(sign_mask, gx);
			gy = _mm_andnot_ps(sign_mask, gy);
			G = _mm_add_ps(gx, gy);

			// Stroe
			_mm_storeu_ps(pDstData + x, G);

		}

		//나머지

		for (; x < nImageW - 1; x++)
		{
			float fgx = *(pSrcData + (x + 1)) - *(pSrcData + (x - 1));
			float fgy = *(pSrcData + floatstep_3DCorrect + x) - *(pSrcData - floatstep_3DCorrect + x);

			*(pDstData + x) = std::abs(fgx) + std::abs(fgy);
		}

		pSrcData += floatstep_3DCorrect;
		pDstData += floatstep_3DCorrect;

	}

	cvDst3DImg = dst;

	bIsLoad = true;

	return bIsLoad;

}

bool CPInsp_Foot::Find_PadRect_UsePadBWAnd3D(CFoot_Model *pFoot, cv::Mat* cvRstPadRectImg, cv::Mat cvSrcPadImage, int nSrcImgW, int nSrcImgH, int nPadTeachSizeX, int nPadTeachSizeY, float fRatioEdgeLineCnt, float fRatioBlackLineCnt, bool bMakeRoundRect, cv::Mat cvPadAndFootImage)
{

	if (nSrcImgW == 0 && nSrcImgH == 0)
		return false;

	if (cvSrcPadImage.empty() == true)
		return false;

	if (nPadTeachSizeX == 0 || nPadTeachSizeY == 0)
	{
		nPadTeachSizeX = 900 / (m_resolX * 1000);
		nPadTeachSizeY = 900 / (m_resolY * 1000);
	}

	int nPadTopY = 0;
	int nPadBottomY = nSrcImgH - 1;
	int nPadLeftX = 0;
	int nPadRightX = nSrcImgW - 1;

	float fRatioPadLine = 0.5;
	if (fRatioEdgeLineCnt != 0.0)
		fRatioPadLine = fRatioEdgeLineCnt;

	float fRatioBlackLine = 0.7;
	if (fRatioBlackLineCnt != 0.0)
		fRatioBlackLine = fRatioBlackLineCnt;

	bool bFlagBlackPx = false;
	int nPrevLineBlackPixelCnt = 0;
	int nSpecBlackPixelCnt = 0;

	bool bFlagFindPadEdgeLinePx = false;
	int nCandidateLineCnt = 0;

	int nCountX = 0;		// X axis White pixel count
	int nCountY = 0;		// Y axis White pixel count

	int nMatchRatio_LineTopY = 0;
	int nMatchRatio_LineBottomY = 0;
	int nMatchRatio_LineLeftX = 0;
	int nMatchRatio_LineRightX = 0;

	bool bIsDBCFoot = false;

	if (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		bIsDBCFoot = true;

	cv::Mat cvSrcImg = cvSrcPadImage.clone();

	//0. convexhull
	if (bIsDBCFoot)
	{
		cv::Mat convexhullimg(nSrcImgH, nSrcImgW, CV_8UC1);
		std::vector<cv::Point> vPadPoints_2DPad, vHulPoints_2DPad;
		convexhullimg.setTo(0);

		vPadPoints_2DPad.clear();
		for (size_t y = 0; y < nSrcImgH; y++)
		{
			UCHAR* pRaw = cvSrcImg.ptr<UCHAR>(y);
			for (size_t x = 0; x < nSrcImgW; x++)
				if (pRaw[x] > 0)
					vPadPoints_2DPad.push_back(cv::Point(x, y));
		}
		if (vPadPoints_2DPad.size() > 0)
		{
			cv::convexHull(vPadPoints_2DPad, vHulPoints_2DPad);
			cv::fillPoly(convexhullimg, vHulPoints_2DPad, cv::Scalar(255));
		}

		convexhullimg.copyTo(cvSrcImg);
	}

	//1. Calc Source PadImg Rect Area (to find Rect Center)
	int nPadOrgRectMinX = nSrcImgW;
	int nPadOrgRectMaxX = 0;
	int nPadOrgRectMinY = nSrcImgH;
	int nPadOrgRectMaxY = 0;

	for (int r = 0; r < nSrcImgH; r++)
	{
		UCHAR* Ptr = cvSrcImg.data;
		for (int c = 0; c < nSrcImgW; c++)
		{
			if (Ptr[r * cvSrcImg.step + c] == 255)
			{
				if (r < nPadOrgRectMinY)	nPadOrgRectMinY = r;
				if (r > nPadOrgRectMaxY)	nPadOrgRectMaxY = r;
				if (c < nPadOrgRectMinX)	nPadOrgRectMinX = c;
				if (c > nPadOrgRectMaxX)	nPadOrgRectMaxX = c;

			}
		}
	}

	if (nPadOrgRectMinX == nSrcImgW && nPadOrgRectMaxX == 0 &&
		nPadOrgRectMinY == nSrcImgH && nPadOrgRectMaxY == 0)
	{
		return false;
	}

	if (pFoot->bUseTeachRect == false && !bIsDBCFoot)
	{
		//leadframe의 경우, 검사 ROI 안에서 찾은 Blob Size로 기준을 두기
		nPadTeachSizeX = (nPadOrgRectMaxX - nPadOrgRectMinX) + 1;
		nPadTeachSizeY = (nPadOrgRectMaxY - nPadOrgRectMinY) + 1;
	}

	int nPadStartX = nPadOrgRectMinX;
	int nPadStartY = nPadOrgRectMinY;
	int nFindPadW = (nPadOrgRectMaxX - nPadOrgRectMinX) + 1;
	int nFindPadL = (nPadOrgRectMaxY - nPadOrgRectMinY) + 1;

	//2. Find Rect Edge (4 direction Edge) in Pad input Image, padImg Rect의 중심기준 
	{
		int nPadCenterX = nPadStartX + (nFindPadW / 2.0);
		int nPadCenterY = nPadStartY + (nFindPadL / 2.0);

		//a. 중심좌표를 기준으로 가로방향 스캔 - Top
		bFlagBlackPx = false;
		bFlagFindPadEdgeLinePx = false;
		nCandidateLineCnt = 0;
		int nPadTopYTmp = 0;
		nSpecBlackPixelCnt = nSrcImgW * fRatioBlackLine; //nPadTeachSizeX * fRatioBlackLine;

		for (int r = 0; r < nSrcImgH; r++)
		{
			UCHAR* Ptr = cvSrcImg.data;
			nCountX = 0;
			nPrevLineBlackPixelCnt = 0;

			for (int c = 0; c < nSrcImgW; c++)
			{
				if (r < nPadCenterY)
				{
					//x축의 255 픽셀 개수가 n개 이상이면 stop
					if (Ptr[r * cvSrcImg.step + c] == 255)
						nCountX++;

					if (Ptr[r * cvSrcImg.step + c] == 0)
						nPrevLineBlackPixelCnt++;
				}
			}

			if (nPrevLineBlackPixelCnt > nSpecBlackPixelCnt)
			{
				nPadTopYTmp = r;
				bFlagBlackPx = true;
			}

			if (nCountX > nPadTeachSizeX * fRatioPadLine && bFlagBlackPx == true)
			{
				if (bFlagFindPadEdgeLinePx == false)
				{
					nPadTopY = r;
					nCandidateLineCnt = nCountX;
				}

				bFlagFindPadEdgeLinePx = true;
			}

			if (r == nPadTopY && bFlagFindPadEdgeLinePx) nCandidateLineCnt++;

		}

		if (bFlagBlackPx == false)
			nPadTopY = nPadTopYTmp;

		if (bFlagFindPadEdgeLinePx)
			nMatchRatio_LineTopY = (int)((float)((float)nCandidateLineCnt / (float)nPadTeachSizeX) * 100);

		if (pFoot->bUseTeachRect == false)
			nPadTopY = 0;

		//b. 중심좌표를 기준으로 가로 방향 스캔 - Bottom
		bFlagBlackPx = false;
		bFlagFindPadEdgeLinePx = false;
		nCandidateLineCnt = 0;

		int nPadBottomYTmp = nSrcImgH - 1;
		nSpecBlackPixelCnt = nSrcImgW * fRatioBlackLine; //nPadTeachSizeX * fRatioBlackLine;

		for (int r = nSrcImgH - 1; r >= 0; r--)
		{
			UCHAR* Ptr = cvSrcImg.data;
			nCountX = 0;
			nPrevLineBlackPixelCnt = 0;

			for (int c = 0; c < nSrcImgW; c++)
			{
				if (r > nPadCenterY)
				{
					//x축의 255 픽셀 개수가 n개 이상이면 stop
					if (Ptr[r * cvSrcImg.step + c] == 255)
						nCountX++;

					if (Ptr[r * cvSrcImg.step + c] == 0)
						nPrevLineBlackPixelCnt++;
				}
			}

			if (nPrevLineBlackPixelCnt > nSpecBlackPixelCnt)
			{
				nPadBottomYTmp = r;
				bFlagBlackPx = true;
			}

			if (nCountX > nPadTeachSizeX * fRatioPadLine && bFlagBlackPx == true)
			{
				if (bFlagFindPadEdgeLinePx == false)
				{
					nPadBottomY = r;
					nCandidateLineCnt = nCountX;
				}

				bFlagFindPadEdgeLinePx = true;
			}

			if (r == nPadBottomY && bFlagFindPadEdgeLinePx) nCandidateLineCnt++;

		}
		if (bFlagBlackPx == false)
			nPadBottomY = nPadBottomYTmp;

		if (bFlagFindPadEdgeLinePx)
			nMatchRatio_LineBottomY = (int)((float)((float)nCandidateLineCnt / (float)nPadTeachSizeX) * 100);

		//c. 중심좌표를 기준으로 세로방향 스캔 - Left 
		bFlagBlackPx = false;
		bFlagFindPadEdgeLinePx = false;
		nCandidateLineCnt = 0;
		int nPadLeftXTmp = 0;
		nSpecBlackPixelCnt = nSrcImgH * fRatioBlackLine; //nPadTeachSizeY * fRatioBlackLine;

		for (int r = 0; r < nSrcImgW; r++)
		{
			UCHAR* Ptr = cvSrcImg.data;
			nCountY = 0;
			nPrevLineBlackPixelCnt = 0;

			for (int c = 0; c < nSrcImgH; c++)
			{
				if (r < nPadCenterX)
				{
					//x축의 255 픽셀 개수가 n개 이상이면 stop
					if (Ptr[c * cvSrcImg.step + r] == 255)
						nCountY++;

					if (Ptr[c * cvSrcImg.step + r] == 0)
						nPrevLineBlackPixelCnt++;
				}
			}

			if (nPrevLineBlackPixelCnt > nSpecBlackPixelCnt)
			{
				nPadLeftXTmp = r;
				bFlagBlackPx = true;
			}

			if (nCountY > nPadTeachSizeY * fRatioPadLine && bFlagBlackPx == true)
			{
				if (bFlagFindPadEdgeLinePx == false)
				{
					nPadLeftX = r;
					nCandidateLineCnt = nCountY;
				}

				bFlagFindPadEdgeLinePx = true;
			}

			if (r == nPadLeftX && bFlagFindPadEdgeLinePx) nCandidateLineCnt++;

		}
		if (bFlagBlackPx == false)
			nPadLeftX = nPadLeftXTmp;

		if (bFlagFindPadEdgeLinePx)
			nMatchRatio_LineLeftX = (int)((float)((float)nCandidateLineCnt / (float)nPadTeachSizeY) * 100);


		//d. 중심좌표를 기준으로 4방향 스캔 - Right
		bFlagBlackPx = false;
		bFlagFindPadEdgeLinePx = false;
		nCandidateLineCnt = 0;
		int nPadRightXTmp = nSrcImgW - 1;
		nSpecBlackPixelCnt = nSrcImgH * fRatioBlackLine; //nPadTeachSizeY * fRatioBlackLine;

		for (int r = nSrcImgW - 1; r >= 0; r--)
		{
			UCHAR* Ptr = cvSrcImg.data;
			nCountY = 0;
			nPrevLineBlackPixelCnt = 0;

			for (int c = 0; c < nSrcImgH; c++)
			{
				if (r > nPadCenterX)
				{
					//x축의 255 픽셀 개수가 n개 이상이면 stop
					if (Ptr[c * cvSrcImg.step + r] == 255)
						nCountY++;

					if (Ptr[c * cvSrcImg.step + r] == 0)
						nPrevLineBlackPixelCnt++;

				}
			}

			if (nPrevLineBlackPixelCnt > nSpecBlackPixelCnt)
			{
				nPadRightXTmp = r;
				bFlagBlackPx = true;
			}

			if (nCountY > nPadTeachSizeY * fRatioPadLine && bFlagBlackPx == true)
			{
				if (bFlagFindPadEdgeLinePx == false)
				{
					nPadRightX = r;
					nCandidateLineCnt = nCountY;
				}

				bFlagFindPadEdgeLinePx = true;
			}

			if (r == nPadRightX && bFlagFindPadEdgeLinePx) nCandidateLineCnt++;

		}
		if (bFlagBlackPx == false)
			nPadRightX = nPadRightXTmp;

		if (bFlagFindPadEdgeLinePx)
			nMatchRatio_LineRightX = (int)((float)((float)nCandidateLineCnt / (float)nPadTeachSizeY) * 100);

	}

	//3. 찾은 Rect의 4방향 Edge Line 이외 바깥부분 지우기
	for (int r = 0; r < nSrcImgH; r++)
	{
		UCHAR* Ptr = cvSrcImg.data;
		for (int c = 0; c < nSrcImgW; c++)
		{
			if (c < nPadLeftX)
			{
				if (Ptr[r * cvSrcImg.step + c] == 255)
					Ptr[r * cvSrcImg.step + c] = 0;
			}
			if (c > nPadRightX)
			{
				if (Ptr[r * cvSrcImg.step + c] == 255)
					Ptr[r * cvSrcImg.step + c] = 0;
			}

			if (r < nPadTopY)
			{
				if (Ptr[r * cvSrcImg.step + c] == 255)
					Ptr[r * cvSrcImg.step + c] = 0;
			}
			if (r > nPadBottomY)
			{
				if (Ptr[r * cvSrcImg.step + c] == 255)
					Ptr[r * cvSrcImg.step + c] = 0;
			}
		}
	}

	//4.  Re-calc Result Find PadImg Rect Area 
	int nPadRectMinX = nSrcImgW;
	int nPadRectMaxX = 0;
	int nPadRectMinY = nSrcImgH;
	int nPadRectMaxY = 0;
	for (int r = 0; r < nSrcImgH; r++)
	{
		UCHAR* Ptr = cvSrcImg.data;
		for (int c = 0; c < nSrcImgW; c++)
		{
			if (Ptr[r * cvSrcImg.step + c] == 255)
			{
				if (r < nPadRectMinY)	nPadRectMinY = r;
				if (r > nPadRectMaxY)	nPadRectMaxY = r;
				if (c < nPadRectMinX)	nPadRectMinX = c;
				if (c > nPadRectMaxX)	nPadRectMaxX = c;

			}
		}
	}

	if (nPadRectMinX == nSrcImgW && nPadRectMaxX == 0 &&
		nPadRectMinY == nSrcImgH && nPadRectMaxY == 0)
	{
		return false;
	}

	//5. Finally Make Pad Result Rect
	int nPadW = (nPadRectMaxX - nPadRectMinX) + 1;
	int nPadL = (nPadRectMaxY - nPadRectMinY) + 1;

	cv::Mat PadRangeImg(nSrcImgH, nSrcImgW, CV_8UC1);
	PadRangeImg.setTo(0);

	if (pFoot->bUseTeachRect)
	{
		int nPadMinX = nSrcImgW;
		int nPadMaxX = 0;
		int nPadMinY = nSrcImgH;
		int nPadMaxY = 0;

		//5-1) Margin ROI Masking
		int nMarginPixelCnt = 20;

		if (nPadRectMinX - nMarginPixelCnt < 0)
			nPadRectMinX = 0;
		else
			nPadRectMinX = nPadRectMinX - nMarginPixelCnt;

		if (nPadRectMinY - nMarginPixelCnt < 0)
			nPadRectMinY = 0;
		else
			nPadRectMinY = nPadRectMinY - nMarginPixelCnt;

		if (nPadRectMinX + (nPadW)+(nMarginPixelCnt * 2) >= nSrcImgW)
			nPadW = nSrcImgW - nPadRectMinX;
		else
			nPadW = (nPadW)+(nMarginPixelCnt * 2);

		if (nPadRectMinY + (nPadL)+(nMarginPixelCnt * 2) >= nSrcImgH)
			nPadL = nSrcImgH - nPadRectMinY;
		else
			nPadL = (nPadL)+(nMarginPixelCnt * 2);

		cv::rectangle(PadRangeImg, cv::Rect(nPadRectMinX, nPadRectMinY, nPadW, nPadL), cv::Scalar(255), cv::FILLED);
		cvSrcImg = cvSrcImg & PadRangeImg;


		//5-2) convert padImg to convexhullimg
		cv::Mat convexhullimg(nSrcImgH, nSrcImgW, CV_8UC1);
		std::vector<cv::Point> vPadPoints_2DPad, vHulPoints_2DPad;
		convexhullimg.setTo(0);
		vPadPoints_2DPad.clear();
		for (size_t y = 0; y < nSrcImgH; y++)
		{
			UCHAR* pRaw = cvSrcImg.ptr<UCHAR>(y);
			for (size_t x = 0; x < nSrcImgW; x++)
				if (pRaw[x] > 0)
					vPadPoints_2DPad.push_back(cv::Point(x, y));
		}
		if (vPadPoints_2DPad.size() > 0)
		{
			cv::convexHull(vPadPoints_2DPad, vHulPoints_2DPad);
			cv::fillPoly(convexhullimg, vHulPoints_2DPad, cv::Scalar(255));
		}
		convexhullimg.copyTo(*cvRstPadRectImg);

		//5-3) 최종 convexhull Pad 영역 Rect 다시 구하기
		for (int r = 0; r < convexhullimg.rows; r++)
		{
			UCHAR* Ptr = convexhullimg.data;
			for (int c = 0; c < convexhullimg.cols; c++)
			{
				if (Ptr[r * convexhullimg.step + c] == 255)
				{
					if (r < nPadMinY)	nPadMinY = r;
					if (r > nPadMaxY)	nPadMaxY = r;
					if (c < nPadMinX)	nPadMinX = c;
					if (c > nPadMaxX)	nPadMaxX = c;

				}
			}
		}

		//5-4) 최종 Pad Rect 위치 검증 (및 재계산)
		int nPadWidth = convexhullimg.cols, nPadHeight = convexhullimg.rows;
		nPadWidth = nPadMaxX - nPadMinX + 1;
		nPadHeight = nPadMaxY - nPadMinY + 1;

		//5-4-1)  min-max 위치 결정_검출영역이 패드 크기보다 큰 경우
		if (nPadMinX + nPadWidth >= convexhullimg.cols || nPadWidth > nPadTeachSizeX + 5)
		{
			if (nMatchRatio_LineLeftX > 0 || nMatchRatio_LineRightX > 0)
			{
				if (nMatchRatio_LineLeftX > 0 && nMatchRatio_LineRightX == 0)
				{
					nPadMaxX = nPadMinX + nPadTeachSizeX;

					if (nPadMaxX >= convexhullimg.cols)
						nPadMaxX = convexhullimg.cols - 1;
				}
				else if (nMatchRatio_LineRightX > 0 && nMatchRatio_LineLeftX == 0)
				{
					nPadMinX = nPadMaxX - nPadTeachSizeX;

					if (nPadMinX < 0)
						nPadMinX = 0;
				}
				else
				{
					if (nMatchRatio_LineLeftX > nMatchRatio_LineRightX)
					{
						nPadMaxX = nPadMinX + nPadTeachSizeX;

						if (nPadMaxX >= convexhullimg.cols)
							nPadMaxX = convexhullimg.cols - 1;
					}
					else
					{
						nPadMinX = nPadMaxX - nPadTeachSizeX;

						if (nPadMinX < 0)
							nPadMinX = 0;
					}
				}
			}
		}

		if (nPadMinY + nPadHeight >= convexhullimg.rows || nPadHeight > nPadTeachSizeY + 5)
		{
			if (nMatchRatio_LineTopY > 0 || nMatchRatio_LineBottomY > 0)
			{
				if (nMatchRatio_LineTopY > 0 && nMatchRatio_LineBottomY == 0)
				{
					nPadMaxY = nPadMinY + nPadTeachSizeY;

					if (nPadMaxY >= convexhullimg.rows)
						nPadMaxY = convexhullimg.rows - 1;
				}
				else if (nMatchRatio_LineBottomY > 0 && nMatchRatio_LineTopY == 0)
				{
					nPadMinY = nPadMaxY - nPadTeachSizeY;

					if (nPadMinY < 0)
						nPadMinY = 0;
				}
				else
				{
					if (nMatchRatio_LineTopY > nMatchRatio_LineBottomY)
					{
						nPadMaxY = nPadMinY + nPadTeachSizeY;

						if (nPadMaxY >= convexhullimg.rows)
							nPadMaxY = convexhullimg.rows - 1;
					}
					else
					{
						nPadMinY = nPadMaxY - nPadTeachSizeY;

						if (nPadMinY < 0)
							nPadMinY = 0;
					}
				}
			}
		}

		//5-4-2)  min-max 위치 결정_검출영역이 크기보다 작은 경우
		if (nPadWidth < (int)((float)nPadTeachSizeX * 0.95))
		{
			//벽에 닿는 부분 확인
			if (nPadMinX == 0 || nPadMaxX == convexhullimg.cols - 1) 
			{
				if (nPadMinX == 0 && nPadMaxX != convexhullimg.cols - 1)
				{
					nPadMinX = 0;
				}
				else if (nPadMinX != 0 && nPadMaxX == nSrcImgW - 1)
				{
					nPadMaxX = nSrcImgW - 1;
				}
				else
				{
					nPadMinX = 0;
					nPadMaxX = nSrcImgW - 1;
				}
			}
			else if (nPadMinX != 0 && nPadMaxX != nSrcImgW - 1)
			{
				if (nMatchRatio_LineLeftX > 0 || nMatchRatio_LineRightX > 0)
				{
					if (nMatchRatio_LineLeftX > 0 && nMatchRatio_LineRightX == 0)
					{
						nPadMaxX = nPadMinX + nPadTeachSizeX;

						if (nPadMaxX >= nSrcImgW)
							nPadMaxX = nSrcImgW - 1;
					}
					else if (nMatchRatio_LineRightX > 0 && nMatchRatio_LineLeftX == 0)
					{
						nPadMinX = nPadMaxX - nPadTeachSizeX;

						if (nPadMinX < 0)
							nPadMinX = 0;
					}
					else
					{
						if (nMatchRatio_LineLeftX > nMatchRatio_LineRightX)
						{
							nPadMaxX = nPadMinX + nPadTeachSizeX;

							if (nPadMaxX >= nSrcImgW)
								nPadMaxX = nSrcImgW - 1;
						}
						else
						{
							nPadMinX = nPadMaxX - nPadTeachSizeX;

							if (nPadMinX < 0)
								nPadMinX = 0;
						}
					}
				}
			}
		}

		if (nPadHeight < (int)((float)nPadTeachSizeY * 0.95))
		{
			//벽에 닿는 부분 확인
			if (nPadMinY == 0 || nPadMaxY == nSrcImgH - 1)
			{
				if (nPadMinY == 0 && nPadMaxY != nSrcImgH - 1)
				{
					nPadMinY = 0;
				}
				else if (nPadMinY != 0 && nPadMaxY == nSrcImgH - 1)
				{
					nPadMaxY = nSrcImgH - 1;
				}
				else
				{
					nPadMinY = 0;
					nPadMaxY = nSrcImgH - 1;
				}
			}
			else if (nPadMinY != 0 && nPadMaxY != convexhullimg.rows - 1)
			{
				if (nMatchRatio_LineTopY > 0 || nMatchRatio_LineBottomY > 0)
				{
					if (nMatchRatio_LineTopY > 0 && nMatchRatio_LineBottomY == 0)
					{
						nPadMaxY = nPadMinY + nPadTeachSizeY;

						if (nPadMaxY >= convexhullimg.rows)
							nPadMaxY = convexhullimg.rows - 1;
					}
					else if (nMatchRatio_LineBottomY > 0 && nMatchRatio_LineTopY == 0)
					{
						nPadMinY = nPadMaxY - nPadTeachSizeY;

						if (nPadMinY < 0)
							nPadMinY = 0;
					}
					else
					{
						if (nMatchRatio_LineTopY > nMatchRatio_LineBottomY)
						{
							nPadMaxY = nPadMinY + nPadTeachSizeY;

							if (nPadMaxY >= convexhullimg.rows)
								nPadMaxY = convexhullimg.rows - 1;
						}
						else
						{
							nPadMinY = nPadMaxY - nPadTeachSizeY;

							if (nPadMinY < 0)
								nPadMinY = 0;
						}
					}
				}
			}
		}

		nPadWidth = nPadMaxX - nPadMinX + 1;
		nPadHeight = nPadMaxY - nPadMinY + 1;

		if (nPadWidth > nPadTeachSizeX)
			nPadWidth = nPadTeachSizeX;

		if (nPadHeight > nPadTeachSizeY)
			nPadHeight = nPadTeachSizeY;

		//5-5) 최종 pad 결정
		nPadRectMinX = nPadMinX;
		nPadRectMaxX = nPadMaxX;
		nPadRectMinY = nPadMinY;
		nPadRectMaxY = nPadMaxY;

		nPadW = nPadWidth;
		nPadL = nPadHeight;
	}
	else if (pFoot->bUseTeachRect == false && !bIsDBCFoot)
	{
		//leadFrame의 경우
		//5-1) 최종 Pad Rect 위치 검증(및 재계산)
		//min-max 위치 결정_넘어가는경우
		if (nPadRectMinX + nPadW >= nSrcImgW || nPadW > nPadTeachSizeX + 5)
		{
			if (nMatchRatio_LineLeftX > 0 || nMatchRatio_LineRightX > 0)
			{
				if (nMatchRatio_LineLeftX > 0 && nMatchRatio_LineRightX == 0)
				{
					nPadRectMaxX = nPadRectMinX + nPadTeachSizeX;

					if (nPadRectMaxX >= nSrcImgW)
						nPadRectMaxX = nSrcImgW - 1;
				}
				else if (nMatchRatio_LineRightX > 0 && nMatchRatio_LineLeftX == 0)
				{
					nPadRectMinX = nPadRectMaxX - nPadTeachSizeX;

					if (nPadRectMinX < 0)
						nPadRectMinX = 0;
				}
				else
				{
					if (nMatchRatio_LineLeftX > nMatchRatio_LineRightX)
					{
						nPadRectMaxX = nPadRectMinX + nPadTeachSizeX;

						if (nPadRectMaxX >= nSrcImgW)
							nPadRectMaxX = nSrcImgW - 1;
					}
					else
					{
						nPadRectMinX = nPadRectMaxX - nPadTeachSizeX;

						if (nPadRectMinX < 0)
							nPadRectMinX = 0;
					}
				}
			}
		}

		if (nPadRectMinY + nPadL >= nSrcImgH || nPadL > nPadTeachSizeY + 5)
		{
			if (nMatchRatio_LineTopY > 0 || nMatchRatio_LineBottomY > 0)
			{
				if (nMatchRatio_LineTopY > 0 && nMatchRatio_LineBottomY == 0)
				{
					nPadRectMaxY = nPadRectMinY + nPadTeachSizeY;

					if (nPadRectMaxY >= nSrcImgH)
						nPadRectMaxY = nSrcImgH - 1;
				}
				else if (nMatchRatio_LineBottomY > 0 && nMatchRatio_LineTopY == 0)
				{
					nPadRectMinY = nPadRectMaxY - nPadTeachSizeY;

					if (nPadRectMinY < 0)
						nPadRectMinY = 0;
				}
				else
				{
					if (nMatchRatio_LineTopY > nMatchRatio_LineBottomY)
					{
						nPadRectMaxY = nPadRectMinY + nPadTeachSizeY;

						if (nPadRectMaxY >= nSrcImgH)
							nPadRectMaxY = nSrcImgH - 1;
					}
					else
					{
						nPadRectMinY = nPadRectMaxY - nPadTeachSizeY;

						if (nPadRectMinY < 0)
							nPadRectMinY = 0;
					}
				}
			}
		}

		//min-max 위치 결정_검출영역이 크기보다 부족한 경우
		if (nPadW < (int)((float)nPadTeachSizeX * 0.95))
		{
			if (nPadOrgRectMinX == 0 || nPadOrgRectMaxX == nSrcImgW - 1) 
			{
				if (nPadOrgRectMinX == 0 && nPadOrgRectMaxX != nSrcImgW - 1)
				{
					nPadRectMinX = 0;
				}
				else if (nPadOrgRectMinX != 0 && nPadOrgRectMaxX == nSrcImgW - 1)
				{
					nPadRectMaxX = nSrcImgW - 1;
				}
				else
				{
					nPadRectMinX = 0;
					nPadRectMaxX = nSrcImgW - 1;
				}
			}
			else if (nPadOrgRectMinX != 0 || nPadOrgRectMaxX != nSrcImgW - 1)
			{
				if (nMatchRatio_LineLeftX != 0 && nMatchRatio_LineRightX != 0)
				{
					if (nMatchRatio_LineLeftX > nMatchRatio_LineRightX)
					{
						nPadRectMaxX = nPadRectMinX + nPadTeachSizeX;
						if (nPadRectMaxX >= nSrcImgW)
							nPadRectMaxX = nSrcImgW - 1;
					}
					else if (nMatchRatio_LineLeftX < nMatchRatio_LineRightX)
					{
						nPadRectMinX = nPadRectMaxX - nPadTeachSizeX;
						if (nPadRectMinX < 0)
							nPadRectMinX = 0;
					}
				}
				else
				{
					nPadRectMinX = nPadRectMaxX - nPadTeachSizeX;
					if (nPadRectMinX < 0)
					{
						nPadRectMinX = 0;
						nPadRectMaxX = nPadTeachSizeX;
					}
				}
			}
		}

		if (nPadL < (int)((float)nPadTeachSizeY * 0.95))
		{
			//check roi Edge Position
			if (nPadOrgRectMinY == 0 || nPadOrgRectMaxY == nSrcImgH - 1)
			{
				if (nPadOrgRectMinY == 0 && nPadOrgRectMaxY != nSrcImgH - 1)
				{
					nPadRectMinY = 0;

					if (nPadRectMaxY < (float)nPadTeachSizeY * 0.8)
						nPadRectMaxY = nPadOrgRectMaxY;
				}
				else if (nPadOrgRectMinY != 0 && nPadOrgRectMaxY == nSrcImgH - 1)
				{
					nPadRectMaxY = nSrcImgH - 1;
				}
				else
				{
					nPadRectMinY = 0;
					nPadRectMaxY = nSrcImgH - 1;
				}
			}
			else if (nPadOrgRectMinY != 0 || nPadOrgRectMaxY != nSrcImgH - 1)
			{
				if (nMatchRatio_LineTopY != 0 && nMatchRatio_LineBottomY != 0)
				{
					if (nMatchRatio_LineTopY > nMatchRatio_LineBottomY)
					{
						nPadRectMaxY = nPadRectMinY + nPadTeachSizeY;
						if (nPadRectMaxY >= nSrcImgH)
							nPadRectMaxY = nSrcImgH - 1;
					}
					else if (nMatchRatio_LineTopY < nMatchRatio_LineBottomY)
					{
						nPadRectMinY = nPadRectMaxY - nPadTeachSizeY;
						if (nPadRectMinY < 0)
							nPadRectMinY = 0;
					}
				}
				else
				{
					nPadRectMinY = nPadRectMaxY - nPadTeachSizeY;
					if (nPadRectMinY < 0)
					{
						nPadRectMinY = 0;
						nPadRectMaxY = nPadTeachSizeY;
					}
				}
			}
		}

		nPadW = nPadRectMaxX - nPadRectMinX + 1;
		nPadL = nPadRectMaxY - nPadRectMinY + 1;

		cv::rectangle(PadRangeImg, cv::Rect(nPadRectMinX, nPadRectMinY, nPadW, nPadL), cv::Scalar(255), cv::FILLED);

	}

	//6. make Pad Round Rect
	cv::Mat cvGet2DPadImgShape(nSrcImgH, nSrcImgW, CV_8UC1);
	cvGet2DPadImgShape.setTo(0);

	if (bMakeRoundRect)
	{
		cv::Rect PadRect(0, 0, 0, 0);
		cv::RotatedRect RoPadRect;

		RoPadRect.size.width = nPadW;
		RoPadRect.size.height = nPadL;

		RoPadRect.center.x = RoPadRect.size.width / 2 + 0.5;
		RoPadRect.center.y = RoPadRect.size.height / 2 + 0.5;
		RoPadRect.angle = 0.0f;

		cv::Mat cvMakeRoundRectImg = DrawRoundPad(RoPadRect.size.width, RoPadRect.size.height, RoPadRect, pFoot->nRadius);

		cv::Mat cvRstBuff = cvGet2DPadImgShape(cv::Rect(nPadRectMinX, nPadRectMinY, RoPadRect.size.width, RoPadRect.size.height));
		cvMakeRoundRectImg.copyTo(cvRstBuff);

		cvGet2DPadImgShape.copyTo(*cvRstPadRectImg);
	}
	else
	{
		PadRangeImg.copyTo(*cvRstPadRectImg);
	}


	return true;
}

bool CPInsp_Foot::Find_LineAngle_houghline(CFoot_Model *pFoot, cv::Mat* cvSrcLineImage, double &dRstLineTheta)
{
	//Mat Iin = imread("warpedWire_22.jpg");
	//Mat Iin = imread("NonwarpedWire_6.jpg");
	//cv::cvtColor(Iin, Iin, cv::COLOR_BGR2GRAY);
	//imshow("Original Image", Iin);

	cv::Mat Iin = (*cvSrcLineImage).clone();

	int sizey = Iin.rows;
	int sizex = Iin.cols;

	vector<int> xx, yy;
	int ind = 0;

	for (int i = 0; i < sizey; i++)
	{
		for (int j = 0; j < sizex; j++)
		{
			if (Iin.at<uchar>(i, j) > 100)
			{
				xx.push_back(j);
				yy.push_back(i);

				ind++;
			}
		}
	}

	if (xx.size() == 0 || yy.size() == 0)
		return false;

	int samp_ind = (int)(ind / 2);

	cv::Mat I_Hough = cv::Mat::zeros(cv::Size(2 * 20 + 1, 50), CV_64F);

	//삼각함수 연산 중복 사용으로 인해, 속도 저하.
	//삼각함수 계산을 룩업 테이블로 미리 저장하여 사용(전역변수)
	for (int theta = -20; theta <= 20; theta++)
	{
		double phi = (double)theta * CV_PI / 180.0;
		int ind_phi = (int)theta + 20; // compute index in lookup table

		for (int i = 0; i < ind; i++)
		{
			//double Roh = xx[i] * cos(phi) + yy[i] * sin(phi);
			double Roh = xx[i] * g_LTCos_foot[ind_phi] + yy[i] * g_LTSin_foot[ind_phi];

			if (Roh > 10 && Roh < 300)
			{
				int ind_Roh = (int)(Roh / 7.0);
				I_Hough.at<double>(ind_Roh, theta + 20) += 1;
			}
		}
	}

	cv::Mat I_out;
	I_Hough.convertTo(I_out, CV_8U);
	//imshow("Hough Space", I_out);

	int sizeyy = I_out.rows;
	int sizexx = I_out.cols;

	vector<int> I_b, ix_b, iy_b;
	ind = 0;

	for (int i = 0; i < sizeyy; i++)
	{
		for (int j = 0; j < sizexx; j++)
		{
			if (I_out.at<uchar>(i, j) > 100)
			{
				I_b.push_back(I_out.at<uchar>(i, j));
				ix_b.push_back(j);
				iy_b.push_back(i);

				ind++;
			}
		}
	}

	if (I_b.size() == 0 || ix_b.size() == 0 || iy_b.size() == 0)
		return false;

	double sum_ib = 0.0;
	double sum_ix = 0.0;
	double sum_iy = 0.0;

	for (int i = 0; i < ind; i++)
	{
		sum_ib += (double)I_b[i];
		sum_ix += (double)ix_b[i] * (double)I_b[i];
		sum_iy += (double)iy_b[i] * (double)I_b[i];
	}

	if (sum_ib == 0.0 || sum_ix == 0.0 || sum_iy == 0.0)
		return false;

	double Ix = sum_ix / sum_ib;
	double Iy = sum_iy / sum_ib;

	double theta_deg = Ix - 21;
	double theta = theta_deg * CV_PI / 180.0;

	double k = yy[samp_ind] + 1.0 / tan(theta) * xx[samp_ind];

	int y_init = 20;
	int x_init = (int)((k - y_init) * tan(theta));

	int y_end = sizey - 20;
	int x_end = (int)((k - y_end) * tan(theta));

	//cv::cvtColor(Iin, Iin, cv::COLOR_GRAY2RGB);
	//cv::line(Iin, cv::Point(x_init, y_init), cv::Point(x_end, y_end), cv::Scalar(0, 0, 255), 2);

	double dRstTheta = 0;
	cv::Point BSt1, BEd1;

	BSt1.x = x_init;
	BSt1.y = y_init;

	BEd1.x = x_end;
	BEd1.y = y_end;

	//Line 각도 추출
	int dx = BSt1.x - BEd1.x;
	int dy = BSt1.y - BEd1.y;
	if (BSt1.y < BEd1.y)
	{
		dx = BEd1.x - BSt1.x;
		dy = BEd1.y - BSt1.y;
	}

	double dstdRadian = atan2((double)dx, (double)dy);
	double dstdDegree = (dstdRadian * 180) / PI;

	dRstTheta = 180 - dstdDegree;

	//Foot 알고리즘 - 본딩 방향 별 각도 재계산
	{
		int nFootDir = pFoot->m_nFootAngle;

		if (135 >= nFootDir && nFootDir > 45)
			dRstTheta = -dstdDegree;
		else if (225 >= nFootDir && nFootDir > 135) {}
		else if (315 >= nFootDir && nFootDir > 225) {}
		else
			dRstTheta = -dstdDegree;

		if (45 >= pFoot->m_nFootAngle || pFoot->m_nFootAngle > 315)// ^
		{
			if (abs(dRstTheta) > 90)
			{
				dRstTheta += 180;
			}
		}
		else if (225 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 135)// V
		{
			if (abs(dRstTheta) < 90)
			{
				dRstTheta += 180;
			}
		}
		else if (135 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 45)// <
		{
			while (dRstTheta < 0) { dRstTheta += 360; }
			if (abs(dRstTheta) < 180)
			{
				dRstTheta += 180;
			}
		}
		else if (315 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 225)// >
		{
			while (dRstTheta < 0) { dRstTheta += 360; }
			if (abs(dRstTheta) > 180)
			{
				dRstTheta += 180;
			}
		}

		dRstLineTheta = dRstTheta;
	}

	return true;
}