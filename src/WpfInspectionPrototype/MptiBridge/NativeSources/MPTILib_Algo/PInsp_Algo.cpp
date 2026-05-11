#include "StdAfx.h"
#include "PInsp_Mount.h"
#include "PInsp_Algo.h"
#include "MPTI.h"
#include "ippi.h"

// 20140913 SHW : IPP 평균 구하기에 사용
#include "ipps.h"
#include "ippi.h"
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")

#include <limits>
#include <float.h>
#include "ProcMil.h"
#include <immintrin.h>
#include<cmath>
#include<array>
#include "Ransac.h"
#include <vector>

#define CCH_ORDER_ON 0
#define USE_SELECT_FOOT_BOT_AREA 1
int ImgSaveCnt;
#define CalcTiltHeight 1

CPInsp_Algo::CPInsp_Algo(void)
{
}

CPInsp_Algo::~CPInsp_Algo(void)
{
}

int CPInsp_Algo::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	if (m_pProcMilAlgo)
		m_pProcMilAlgo->InitMilAlgoBlob();

	m_inspMount.InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, bUseImagePilLib);
	m_pInspLeadSolder.InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, bUseImagePilLib);

	//CString sLbName;
	//sLbName.Format(_T("D:\\FootRst"));
	//if (PathFileExists(sLbName) == false)
	//	CreateDirectory(sLbName, NULL);
	//sLbName.Format(_T("D:\\3DNomalZData"));
	//if (PathFileExists(sLbName) == false)
	//	CreateDirectory(sLbName, NULL);
	//sLbName.Format(_T("D:\\FootInspImg"));
	//if (PathFileExists(sLbName) == false)
	//	CreateDirectory(sLbName, NULL);
	ImgSaveCnt = 0;

	bFastCompose = true;

	return 0;
}
int CPInsp_Algo::CloseDevice()
{
	m_inspMount.CloseDevice();
	m_pInspLeadSolder.CloseDevice();
	if (m_pProcMilAlgo != NULL)
		m_pProcMilAlgo->FreeMilAlgoBlob();
	CPInsp::CloseDevice();
	return ePART_SUCCESS;
}

#pragma region  Image Func

void CPInsp_Algo::ArrayCopy_float(const float * src, float ** dst, int width, int length)
{
	IppStatus sts;
	IppiSize iSize = { width, length };

	if (*dst == NULL)
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, dst, width * length);
	sts = ippiCopy_32f_C1R(src, width * 4, *dst, width * 4, iSize);
}
void CPInsp_Algo::ArrayCopy_float(WndAlgoImg * sWndAlgoImg)
{
	if (sWndAlgoImg == NULL || sWndAlgoImg->m_fArr3D_part == NULL || sWndAlgoImg->m_nWidth3D <= 0 || sWndAlgoImg->m_nHeight3D <= 0)
		return;

	int nSrcW = sWndAlgoImg->m_nWidth3D;
	int nSrcH = sWndAlgoImg->m_nHeight3D;

	int nDstW = sWndAlgoImg->m_nWidth;
	int nDstH = sWndAlgoImg->m_nHeight;

	if (sWndAlgoImg->m_fArr3D == NULL)
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sWndAlgoImg->m_fArr3D, nDstW * nDstH);
#if _DEBUG
	cv::Mat imgSrc(nSrcH, nSrcW, CV_32FC1, sWndAlgoImg->m_fArr3D_part);
	cv::Mat imgDst(nDstH, nDstW, CV_32FC1, sWndAlgoImg->m_fArr3D);
#endif
	if (nSrcW != nDstW || nSrcH != nDstH)
	{
		sWndAlgoImg->m_nWidth3D = nDstW;
		sWndAlgoImg->m_nHeight3D = nDstH;
		memset(sWndAlgoImg->m_fArr3D, 0, sizeof(float) * nDstW * nDstH);

		m_proc3d.GetCropZmap_LT(sWndAlgoImg->m_fArr3D_part, sWndAlgoImg->m_fArr3D, nSrcW, nSrcH, 0, 0, nDstW, nDstH);
	}
	else
		memcpy(sWndAlgoImg->m_fArr3D, sWndAlgoImg->m_fArr3D_part, sizeof(float) * nDstW * nDstH);

}

int CPInsp_Algo::WndAlgoImg_Update_fromCV(WndAlgoImg * sWndAlgoImg)
{
	if (!sWndAlgoImg->m_ucArrCV || sWndAlgoImg->m_nWidth <= 0 || sWndAlgoImg->m_nHeight <= 0)
		return eMR_FAIL;

	int width = sWndAlgoImg->m_nWidth;
	int height = sWndAlgoImg->m_nHeight;
	int widthStep = g_pMPTI->nCalcWidthStep(false, width);

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sWndAlgoImg->m_ucArr2D, width * height);
	if (width != widthStep)
	{
		IppiSize iSize = { width, height };
		IppStatus sts = ippiCopy_8u_C1R(sWndAlgoImg->m_ucArrCV, widthStep, sWndAlgoImg->m_ucArr2D, width, iSize);
	}
	else
		memcpy(sWndAlgoImg->m_ucArr2D, sWndAlgoImg->m_ucArrCV, sizeof(UCHAR)*width * height);

	return eMR_SUCCESS;
}
int CPInsp_Algo::WndAlgoImg_Update_fromCV_MixImg(WndAlgoImg * sWndAlgoImg, bool bIsGrid)
{
	if (!sWndAlgoImg->m_ucArrCV || sWndAlgoImg->m_nWidth <= 0 || sWndAlgoImg->m_nHeight <= 0)
		return eMR_FAIL;

	CString sLog;
	int width = sWndAlgoImg->m_nWidth;
	int height = sWndAlgoImg->m_nHeight;
	int widthStep = g_pMPTI->nCalcWidthStep(false, width);

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sWndAlgoImg->m_ucArr2D, width * height);
	if (width != widthStep)
	{
		IppiSize iSize = { width, height };
		IppStatus sts = ippiCopy_8u_C1R(sWndAlgoImg->m_ucArrCV, widthStep, sWndAlgoImg->m_ucArr2D, width, iSize);
	}
	else
		memcpy(sWndAlgoImg->m_ucArr2D, sWndAlgoImg->m_ucArrCV, sizeof(UCHAR)*width * height);

	if (widthStep != width || bIsGrid == true)
	{
		UCHAR* Arr2D[2] = { NULL,NULL };
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		{
			if (sWndAlgoImg->m_ucArr2D_Mix[iLoopCount] == NULL)
			{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &Arr2D[iLoopCount], widthStep * height);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sWndAlgoImg->m_ucArr2D_Mix[iLoopCount], widthStep * height);
			}
			else
			{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &Arr2D[iLoopCount], widthStep * height);
				memcpy(Arr2D[iLoopCount], sWndAlgoImg->m_ucArr2D_Mix[iLoopCount], sizeof(UCHAR)*width * height);
			}
		}

		IppStatus sts;
		IppiSize iSize = { width, height };
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
			if (sWndAlgoImg->m_ucArr2D_Mix[iLoopCount] != NULL)
				sts = ippiCopy_8u_C1R(Arr2D[iLoopCount], widthStep, sWndAlgoImg->m_ucArr2D_Mix[iLoopCount], width, iSize);

		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
			Delete_1DArray(&Arr2D[iLoopCount]);
		return 1;
	}
	return eMR_SUCCESS;
}
int CPInsp_Algo::WndAlgoImg_Update_fromCV_Color(AlgoColorOpt * ptrAlgoColorOpt, bool bIsLoadFovRawDataImage)
{
	UCHAR* img_R = NULL;
	UCHAR* img_G = NULL;
	UCHAR* img_B = NULL;
	UCHAR* img_W = NULL;
	UCHAR* img_MR = NULL;
	UCHAR* img_MB = NULL;
	UCHAR* img_BR = NULL;
	UCHAR* img_BB = NULL;
	int nFovW = ptrAlgoColorOpt->m_sFovImg.nImageSizeX;
	int nFovH = ptrAlgoColorOpt->m_sFovImg.nImageSizeY;
	// LYS 2018/01/16	BW Color 는 하기 버퍼를 사용하므로 조건 문 주석 해제 및 else 문 구문 추가
	if (ptrAlgoColorOpt->m_nInspType == eMSCN_COAXIAL)	//LYS 2018/04/10 GrabPartImage() 함수에서 사이드 카메라영상도 m_sFovImg.imgTop_ 버퍼에 넣고 있으므로 분기가 필요없음 // 검사 결과가 상이해 다시 분기
	{
		img_R = ptrAlgoColorOpt->m_sFovImg.imgTop_R;
		img_G = ptrAlgoColorOpt->m_sFovImg.imgTop_G;
		img_B = ptrAlgoColorOpt->m_sFovImg.imgTop_B;
		img_W = ptrAlgoColorOpt->m_sFovImg.imgTop_W;
		img_MR = ptrAlgoColorOpt->m_sFovImg.imgMiddle_R;
		img_MB = ptrAlgoColorOpt->m_sFovImg.imgMiddle_B;
		img_BR = ptrAlgoColorOpt->m_sFovImg.imgBottom_R;
		img_BB = ptrAlgoColorOpt->m_sFovImg.imgBottom_B;
	}
	else
	{
		if (ptrAlgoColorOpt->m_nInspType == eMSCN_SIDECAM1)
		{
			if (bIsLoadFovRawDataImage == true)	// bIsLoadFovRawDataImage 추가할것
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide1_R = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_RED - 1, ptrAlgoColorOpt->m_nInspType);
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide1_G = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_GREEN - 1, ptrAlgoColorOpt->m_nInspType);
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide1_B = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_BLUE - 1, ptrAlgoColorOpt->m_nInspType);
			}
			else
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide1_R;
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide1_G;
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide1_B;
			}
		}
		else if (ptrAlgoColorOpt->m_nInspType == eMSCN_SIDECAM2)
		{
			if (bIsLoadFovRawDataImage == true)
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide2_R = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_RED - 1, ptrAlgoColorOpt->m_nInspType);
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide2_G = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_GREEN - 1, ptrAlgoColorOpt->m_nInspType);
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide2_B = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_BLUE - 1, ptrAlgoColorOpt->m_nInspType);
			}
			else
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide2_R;
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide2_G;
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide2_B;
			}
		}
		else if (ptrAlgoColorOpt->m_nInspType == eMSCN_SIDECAM3)
		{
			if (bIsLoadFovRawDataImage == true)
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide3_R = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_RED - 1, ptrAlgoColorOpt->m_nInspType);
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide3_G = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_GREEN - 1, ptrAlgoColorOpt->m_nInspType);
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide3_B = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_BLUE - 1, ptrAlgoColorOpt->m_nInspType);
			}
			else
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide3_R;
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide3_G;
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide3_B;
			}
		}
		else if (ptrAlgoColorOpt->m_nInspType == eMSCN_SIDECAM4)
		{
			if (bIsLoadFovRawDataImage == true)
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide4_R = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_RED - 1, ptrAlgoColorOpt->m_nInspType);
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide4_G = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_GREEN - 1, ptrAlgoColorOpt->m_nInspType);
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide4_B = g_pMPTI->GetRawDataFovBuffer_Side(eMSC_BLUE - 1, ptrAlgoColorOpt->m_nInspType);
			}
			else
			{
				img_R = ptrAlgoColorOpt->m_sFovImg.imgSide4_R;
				img_G = ptrAlgoColorOpt->m_sFovImg.imgSide4_G;
				img_B = ptrAlgoColorOpt->m_sFovImg.imgSide4_B;
			}
		}

		if (bIsLoadFovRawDataImage == true)
		{
			nFovW = g_pMPTI->m_nSizeXRawData;
			nFovH = g_pMPTI->m_nSizeYRawData;
			int widthStep = g_pMPTI->nCalcWidthStep(false, nFovW);
		}
	}
	if (img_R == NULL || img_G == NULL || img_B == NULL)
		return eMR_FAIL;

#if _DEBUG
	cv::Mat imgR(nFovH, nFovW, CV_8UC1, img_R);
	cv::Mat imgG(nFovH, nFovW, CV_8UC1, img_G);
	cv::Mat imgB(nFovH, nFovW, CV_8UC1, img_B);
#endif

	int nImgL = ptrAlgoColorOpt->m_rcImageRect.left;
	int nImgT = ptrAlgoColorOpt->m_rcImageRect.top;
	int nImgW = ptrAlgoColorOpt->m_rcImageRect.right - ptrAlgoColorOpt->m_rcImageRect.left;
	int nImgH = ptrAlgoColorOpt->m_rcImageRect.bottom - ptrAlgoColorOpt->m_rcImageRect.top;
	if (nImgW <= 0) nImgW = 1;
	if (nImgH <= 0) nImgH = 1;
	if ((nImgW <= 0) || (nImgH <= 0) || (nFovW <= 0) || (nFovH <= 0))
		return eMR_FAIL;
	if (nImgW > nFovW) nImgW = nFovW;
	if (nImgH > nFovH) nImgH = nFovH;
	if (nImgL < 0) nImgL = 0;
	if (nImgT < 0) nImgT = 0;
	if (bIsLoadFovRawDataImage == false)
	{
		if (nImgL + nImgW > nFovW)
			nImgW = nFovW - nImgL;
		if (nImgT + nImgH > nFovH)
			nImgH = nFovH - nImgT;
	}
	else
	{
		int nPartCX = nFovW / 2;
		int nPartCY = nFovH / 2;
		int nFovCX = ptrAlgoColorOpt->m_sFovImg.nImageSizeX / 2;
		int nFovCY = ptrAlgoColorOpt->m_sFovImg.nImageSizeY / 2;
		int nPartL = nPartCX + (nFovCX - nImgL);
		int nPartT = nPartCY + (nFovCY - nImgT);
		if (nImgL + nImgW > nFovW)
		{
			if (nPartL + nImgW > nFovW)
				nImgL = nFovW - nImgW;
			else
				nImgL = nPartL;
		}
		if (nImgT + nImgH > nFovH)
		{
			if (nPartT + nImgH > nFovH)
				nImgT = nFovH - nImgH;
			else
				nImgT = nPartT;
		}
	}

	int nWidthStep = nImgW;

	// 	int nSizeX = nImgW % 4;
	// 	if (nSizeX!=0)
	// 		nWidthStep += 4 - nSizeX;
	nWidthStep = g_pMPTI->nCalcWidthStep(false, nImgW);
	if (ptrAlgoColorOpt->m_nInspType > -1 && bIsLoadFovRawDataImage == true)
		nWidthStep = g_pMPTI->nCalcWidthStep(true, nImgW);

	ptrAlgoColorOpt->m_sImgBuf.nImageSizeX = nWidthStep;
	ptrAlgoColorOpt->m_sImgBuf.nImageSizeY = nImgH;

	ptrAlgoColorOpt->m_sImgBuf.imgTop_R = NULL;
	ptrAlgoColorOpt->m_sImgBuf.imgTop_G = NULL;
	ptrAlgoColorOpt->m_sImgBuf.imgTop_B = NULL;
	ptrAlgoColorOpt->m_sImgBuf.imgTop_W = NULL;
	ptrAlgoColorOpt->m_sImgBuf.imgMiddle_R = NULL;
	ptrAlgoColorOpt->m_sImgBuf.imgMiddle_B = NULL;
	ptrAlgoColorOpt->m_sImgBuf.imgBottom_R = NULL;
	ptrAlgoColorOpt->m_sImgBuf.imgBottom_B = NULL;

	if (img_R != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgTop_R, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_R, nFovW, nFovH, _T("Fov_Color2D_R.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_R, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgTop_R, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgTop_R, nWidthStep, nImgH, _T("Clip_Color2D_R.bmp"));

		if (g_pMPTI->m_bSideOriginalSize == true)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgSide1_R, nWidthStep * nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sFovImg.imgSide1_R, nFovW, nFovH, _T("Fov_Color2D_Side1_R.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt->m_sFovImg.imgSide1_R, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgSide1_R, nImgL, nImgT, nWidthStep, nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgSide1_R, nWidthStep, nImgH, _T("Clip_Color2D_Side1_R.bmp"));
		}
		else if (g_pMPTI->m_bSideOriginalSize == true && ptrAlgoColorOpt->m_nInspType == 2)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgSide3_R, nWidthStep * nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sFovImg.imgSide3_R, nFovW, nFovH, _T("Fov_Color2D_Side3_R.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt->m_sFovImg.imgSide3_R, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgSide3_R, nImgL, nImgT, nWidthStep, nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgSide3_R, nWidthStep, nImgH, _T("Clip_Color2D_Side3_R.bmp"));
		}
	}
	if (img_G != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgTop_G, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_G, nFovW, nFovH, _T("Fov_Color2D_G.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_G, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgTop_G, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgTop_G, nWidthStep, nImgH, _T("Clip_Color2D_G.bmp"));

		if (g_pMPTI->m_bSideOriginalSize == true)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgSide1_G, nWidthStep * nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sFovImg.imgSide1_G, nFovW, nFovH, _T("Fov_Color2D_Side1_G.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt->m_sFovImg.imgSide1_G, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgSide1_G, nImgL, nImgT, nWidthStep, nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgSide1_G, nWidthStep, nImgH, _T("Clip_Color2D_Side1_G.bmp"));
		}
		else if (g_pMPTI->m_bSideOriginalSize == true && ptrAlgoColorOpt->m_nInspType == 2)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgSide3_G, nWidthStep * nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sFovImg.imgSide3_G, nFovW, nFovH, _T("Fov_Color2D_Side3_G.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt->m_sFovImg.imgSide3_G, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgSide3_G, nImgL, nImgT, nWidthStep, nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgSide3_G, nWidthStep, nImgH, _T("Clip_Color2D_Side3_G.bmp"));
		}
	}
	if (img_B != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgTop_B, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_B, nFovW, nFovH, _T("Fov_Color2D_B.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_B, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgTop_B, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgTop_B, nWidthStep, nImgH, _T("Clip_Color2D_B.bmp"));

		if (g_pMPTI->m_bSideOriginalSize == true)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgSide1_B, nWidthStep * nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sFovImg.imgSide1_B, nFovW, nFovH, _T("Fov_Color2D_Side1_B.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt->m_sFovImg.imgSide1_B, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgSide1_B, nImgL, nImgT, nWidthStep, nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgSide1_B, nWidthStep, nImgH, _T("Clip_Color2D_Side1_B.bmp"));
		}
		else if (g_pMPTI->m_bSideOriginalSize == true && ptrAlgoColorOpt->m_nInspType == 2)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgSide3_B, nWidthStep * nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sFovImg.imgSide3_B, nFovW, nFovH, _T("Fov_Color2D_Side2_B.bmp"));
			m_pProcMilAlgo->GetClipImage_LT(ptrAlgoColorOpt->m_sFovImg.imgSide3_B, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgSide3_B, nImgL, nImgT, nWidthStep, nImgH);
			m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgSide3_B, nWidthStep, nImgH, _T("Clip_Color2D_Side2_B.bmp"));
		}
	}
	if (img_W != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgTop_W, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_W, nFovW, nFovH, _T("Fov_Color2D_W.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_W, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgTop_W, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgTop_W, nWidthStep, nImgH, _T("Clip_Color2D_W.bmp"));
	}
	if (img_MR != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgMiddle_R, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_MR, nFovW, nFovH, _T("Fov_Color2D_MR.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_MR, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgMiddle_R, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgMiddle_R, nWidthStep, nImgH, _T("Clip_Color2D_MR.bmp"));
	}
	if (img_MB != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgMiddle_B, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_MB, nFovW, nFovH, _T("Fov_Color2D_MB.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_MB, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgMiddle_B, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgMiddle_B, nWidthStep, nImgH, _T("Clip_Color2D_MB.bmp"));
	}
	if (img_BR != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgBottom_R, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_BR, nFovW, nFovH, _T("Fov_Color2D_BR.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_BR, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgBottom_R, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgBottom_R, nWidthStep, nImgH, _T("Clip_Color2D_BR.bmp"));
	}
	if (img_BB != NULL)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrAlgoColorOpt->m_sImgBuf.imgBottom_B, nWidthStep * nImgH);
		m_pProcMilAlgo->SaveWorkImg(img_BB, nFovW, nFovH, _T("Fov_Color2D_BB.bmp"));
		m_pProcMilAlgo->GetClipImage_LT(img_BB, nFovW, nFovH, ptrAlgoColorOpt->m_sImgBuf.imgBottom_B, nImgL, nImgT, nWidthStep, nImgH);
		m_pProcMilAlgo->SaveWorkImg(ptrAlgoColorOpt->m_sImgBuf.imgBottom_B, nWidthStep, nImgH, _T("Clip_Color2D_BB.bmp"));
	}

	if (ptrAlgoColorOpt->m_sImgBuf.nImageSizeX <= 0 || ptrAlgoColorOpt->m_sImgBuf.nImageSizeY <= 0)
		return eMR_FAIL;

	return eMR_SUCCESS;
}

void CPInsp_Algo::Update_Image_TOCV(WndAlgoImg * sWndAlgoImg, UCHAR * srcImg, UCHAR * dstImg)
{
	if (srcImg == NULL || dstImg == NULL /*|| strlen((const char*)srcImg) == 0 && strlen((const char*)dstImg) == 0/*|| sWndAlgoImg->m_bIs2dCV == FALSE*/)
		return;
	int width = sWndAlgoImg->m_nWidth;
	int height = sWndAlgoImg->m_nHeight;
	int widthStep = width;

	/*int nSizeX = width % 4;
	if (nSizeX != 0)
	{
		widthStep += 4 - nSizeX;
	}*/

	widthStep = g_pMPTI->nCalcWidthStep(false, width);
	IppiSize iSize = { width, height };
	IppStatus sts = ippiCopy_8u_C1R(srcImg, width, dstImg, widthStep, iSize);
	if (srcImg)
		// Delete_1DArray(&srcImg);
		g_pMManager->pem_delete(srcImg, true);
}
void CPInsp_Algo::Update_Image_TOCV_Color(int width, int height, UCHAR * srcImg, UCHAR * dstImg)
{
	if (srcImg == NULL || dstImg == NULL)
		return;

	int widthStep = g_pMPTI->nCalcWidthStep(false, width);

	IppStatus sts;
	IppiSize iSize = { width, height };
	sts = ippiCopy_8u_C1R(srcImg, width, dstImg, widthStep, iSize);
	if (srcImg)
		Delete_1DArray(&srcImg);
}

void CPInsp_Algo::RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle)
{
	if (bFastCompose)
	{
		PInspAlgoWrapper* pInspWarpper = g_pInspMng->GePInspWrapper();
		pInspWarpper->RoiImageCompose_LT(ptrbyRedBuffer, ptrbyGreenBuffer, ptrbyBlueBuffer, ptrbyWhiteBuffer,
			nFullImageSize, nFullImageWidth, nFullImageHeight, nROIImageSize, dROIX, dROIY, nROIWidth, nROIHeight,
			nRedValue, nGreenValue, nBlueValue, nWhiteValue, ptrbyResultImage, dAngle);

		return;
	}

	float fRedValue = (float)nRedValue / 100.0;
	float fGreenValue = (float)nGreenValue / 100.0;
	float fBlueValue = (float)nBlueValue / 100.0;
	float fWhiteValue = (float)nWhiteValue / 100.0;

	int nValueBuf = 0;
	float fRedImageValue = 0;
	float fGreenImageValue = 0;
	float fBlueImageValue = 0;
	float fWhiteImageValue = 0;

	//shkim 예외처리
	int RotateWidth = nROIWidth;
	int RotateHeight = nROIHeight;
	int nROIGapX = 0;
	int nROIGapY = 0;
	if (dAngle != 0)
	{
		//shkim rotate Rect sz 예외처리
		float radian = (float)(dAngle / 180.0f * M_PI);
		int RotateWidth = std::abs(nROIWidth * cos(radian)) + std::abs(nROIHeight * sin(radian));
		int RotateHeight = std::abs(nROIHeight * cos(radian)) + std::abs(nROIWidth * sin(radian));
		nROIGapX = RotateWidth - nROIWidth;
		nROIGapY = RotateHeight - nROIHeight;
		nROIWidth = RotateWidth;
		nROIHeight = RotateHeight;
		dROIX -= nROIGapX / 2;
		dROIY -= nROIGapY / 2;
	}
	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nMaxValue = nFullImageSize / (nFullImageWidth*nFullImageHeight);
	if (nMaxValue <= 0)	nMaxValue = 1;
	UCHAR* ptrbyResultImageRound = g_pMManager->pem_new<UCHAR>(true, RotateWidth*RotateHeight * sizeof(UCHAR), (PCHAR)__FUNCTION__, __LINE__);

	nRedValue = (int)(fRedValue * 128.0);
	nGreenValue = (int)(fGreenValue * 128.0);
	nBlueValue = (int)(fBlueValue * 128.0);
	nWhiteValue = (int)(fWhiteValue * 128.0);

	__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
	__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
	__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
	__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);
	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);


	for (int i = 0; i < nMaxValue; i++)
	{
		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * iptrbyResultImageRound = (&ptrbyResultImageRound[(y - nStartY)*nROIWidth]);
			int x = 0;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue;
				register __m128i fGreenImageValue;
				register __m128i fBlueImageValue;
				register __m128i fWhiteImageValue;

				if (!ptrbyRedBuffer)
					fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
				else
					fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyRedBuffer[FImgIdx]);
				if (!ptrbyGreenBuffer)
					fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
				else
					fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyGreenBuffer[FImgIdx]);
				if (!ptrbyBlueBuffer)
					fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
				else
					fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBlueBuffer[FImgIdx]);
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
				else
					fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyWhiteBuffer[FImgIdx]);

				_mm_storeu_si128(
					(__m128i *)(&iptrbyResultImageRound[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (!ptrbyRedBuffer)
					fRedImageValue = 0;
				else
					fRedImageValue = ptrbyRedBuffer[FImgIdx];
				if (!ptrbyGreenBuffer)
					fGreenImageValue = 0;
				else
					fGreenImageValue = ptrbyGreenBuffer[FImgIdx];
				if (!ptrbyBlueBuffer)
					fBlueImageValue = 0;
				else
					fBlueImageValue = ptrbyBlueBuffer[FImgIdx];
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = 0;
				else
					fWhiteImageValue = ptrbyWhiteBuffer[FImgIdx];

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				iptrbyResultImageRound[(x - nStartX)] = nValueBuf;
			}
		}
	}
	if (dAngle == 0)
		return;

	UCHAR* ptrSrc = ptrbyResultImage;
	UCHAR* ptrbyBuffer_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;
	if (ptrbyResultImage != NULL)
	{
		m_pProcMilAlgo->RotateImg_ipp(ptrbyResultImageRound, nROIWidth, nROIHeight, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		nROIWidth -= nROIGapX;
		nROIHeight -= nROIGapY;
		GapX = RounD((retDstSizeX - nROIWidth) / 2);
		GapY = RounD((retDstSizeY - nROIHeight) / 2);
		if ((GapX + nROIWidth) > retDstSizeX)
			GapX = 0;
		if ((GapY + nROIHeight) > retDstSizeY)
			GapY = 0;
		for (int r = 0; r < nROIHeight; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];
			UCHAR* dstPtr = &ptrSrc[r*nROIWidth];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*nROIWidth);
		}
		m_pProcMilAlgo->SaveWorkImg(ptrbyBuffer_ro, retDstSizeX, retDstSizeY, _T("ptrbyBuffer_ro Red.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ptrSrc, nROIWidth, nROIHeight, _T("ptrbyRedBuffer.bmp"));

		if (ptrbyBuffer_ro != NULL)
			//delete [] ptrbyBuffer_ro;
			g_pMManager->pem_delete(ptrbyBuffer_ro, true);
	}
	g_pMManager->pem_delete(ptrbyResultImageRound, true);

}
void CPInsp_Algo::FullImageCompose(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
	int nFullImageSize, int dfullImageWidth, int dfullImageHeight,
	double dROICenterX, double dROICenterY, int nRoiWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage)
{
	if (bFastCompose)
	{
		PInspAlgoWrapper* pInspWarpper = g_pInspMng->GePInspWrapper();
		pInspWarpper->RoiImageCompose_LT(ptrbyRedBuffer, ptrbyGreenBuffer, ptrbyBlueBuffer, ptrbyWhiteBuffer,
			nFullImageSize, dfullImageWidth, dfullImageHeight, nFullImageSize, 0, 0, 0, 0,
			nRedValue, nGreenValue, nBlueValue, nWhiteValue, ptrbyResultImage);

		return;
	}
	
	float fRedValue = (float)nRedValue / 100.0;
	float fGreenValue = (float)nGreenValue / 100.0;
	float fBlueValue = (float)nBlueValue / 100.0;
	float fWhiteValue = (float)nWhiteValue / 100.0;

	int nValueBuf = 0;
	float fRedImageValue = 0;
	float fGreenImageValue = 0;
	float fBlueImageValue = 0;
	float fWhiteImageValue = 0;

	for (int a = 0; a < nFullImageSize; a++)
	{
		if (!ptrbyRedBuffer)
			fRedImageValue = 0;
		else
			fRedImageValue = ptrbyRedBuffer[a];
		if (!ptrbyGreenBuffer)
			fGreenImageValue = 0;
		else
			fGreenImageValue = ptrbyGreenBuffer[a];
		if (!ptrbyBlueBuffer)
			fBlueImageValue = 0;
		else
			fBlueImageValue = ptrbyBlueBuffer[a];
		if (!ptrbyWhiteBuffer)
			fWhiteImageValue = 0;
		else
			fWhiteImageValue = ptrbyWhiteBuffer[a];

		nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

		if (nValueBuf > 255)
			nValueBuf = 255;
		ptrbyResultImage[a] = nValueBuf;
	}

	m_pProcMilAlgo->SaveWorkImg(ptrbyResultImage, dfullImageWidth, dfullImageHeight, _T("FullImageCompose.bmp"));
}
void CPInsp_Algo::FullImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage)
{
	if (bFastCompose)
	{
		PInspAlgoWrapper* pInspWarpper = g_pInspMng->GePInspWrapper();
		LightTypeBuf _sLightImg = sLightImg;
		_sLightImg.m_nROIImgWidth = 0;
		_sLightImg.m_nROIImgHeight = 0;
		pInspWarpper->ROIImageClaculCompose(_sLightImg, ptrbyResultImage);

		return;
	}
	int nWidth = sLightImg.m_nImgWidth;
	int nHeight = sLightImg.m_nImgHeight;
	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nWidth <= 0) || (nHeight <= 0) || (nImgCnt <= 0))
		return;
	int nImageSize = nWidth * nHeight;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	int nValueBuf = 0;
	float fRedImageValue = 0.0;
	float fGreenImageValue = 0.0;
	float fBlueImageValue = 0.0;
	float fWhiteImageValue = 0.0;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	UCHAR** ppucComposeImageBuf = NULL;
	//ppucComposeImageBuf = new UCHAR*[nImgCnt];
	ppucComposeImageBuf = g_pMManager->pem_new<UCHAR*>(true, nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	for (int i = 0; i < nImgCnt; i++)
	{
		//ppucComposeImageBuf[i] = new UCHAR[nImageSize];
		ppucComposeImageBuf[i] = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	}


	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);

	for (int a = 0; a < nImgCnt; a++)
	{
		nValueBuf = 0;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		int nRedValue = (int)(fRedValue * 128.0);
		int nGreenValue = (int)(fGreenValue * 128.0);
		int nBlueValue = (int)(fBlueValue * 128.0);
		int nWhiteValue = (int)(fWhiteValue * 128.0);

		__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
		__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
		__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
		__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);

		for (int y = 0; y < nHeight; y++)
		{
			UCHAR * ippucComposeImageBuf = (UCHAR *)(&ppucComposeImageBuf[a][(y)*nWidth]);
			int x;
			for (x = 0; (x + 16) < nWidth; x += 16)
			{


				int FImgIdx = y * nWidth + x;

				register __m128i fRedImageValue = zeroBuf;
				register __m128i fGreenImageValue = zeroBuf;
				register __m128i fBlueImageValue = zeroBuf;
				register __m128i fWhiteImageValue = zeroBuf;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopRedBuffer[FImgIdx]);
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopGreenBuffer[FImgIdx]);
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopBlueBuffer[FImgIdx]);
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidRedBuffer[FImgIdx]);
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidGreenBuffer[FImgIdx]);
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidBlueBuffer[FImgIdx]);
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotRedBuffer[FImgIdx]);
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotGreenBuffer[FImgIdx]);
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotBlueBuffer[FImgIdx]);
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotWhiteBuffer[FImgIdx]);
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				_mm_storeu_si128(
					(__m128i *)(&ippucComposeImageBuf[(x)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < nWidth; x++)
			{
				int FImgIdx = y * nWidth + x;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyTopRedBuffer[FImgIdx];
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyTopGreenBuffer[FImgIdx];
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyTopBlueBuffer[FImgIdx];
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyTopWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyMidRedBuffer[FImgIdx];
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyMidGreenBuffer[FImgIdx];
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyMidBlueBuffer[FImgIdx];
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyMidWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyBotRedBuffer[FImgIdx];
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyBotGreenBuffer[FImgIdx];
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyBotBlueBuffer[FImgIdx];
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyBotWhiteBuffer[FImgIdx];
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				if (nValueBuf < 0)
					nValueBuf = 0;
				ippucComposeImageBuf[x] = nValueBuf;
			}

		}
		m_pProcMilAlgo->SaveWorkImg(ppucComposeImageBuf[a], nWidth, nHeight, _T("ComposeImageBuf.bmp"));
	}

	// 	for (int a = 0; a< nImgCnt; a++)
	// 	{
	// 		nValueBuf = 0;
	// 
	// 		fRedValue = (float)pnRedValue[a] / 100.0;
	// 		fGreenValue = (float)pnGreenValue[a] / 100.0;
	// 		fBlueValue = (float)pnBlueValue[a] / 100.0;
	// 		fWhiteValue = (float)pnWhiteValue[a] / 100.0;
	// 
	// 		for (int b = 0; b < nImageSize; b++)
	// 		{
	// 			if(ptrnPosition[a] == 0)
	// 			{
	// 				if (!ptrbyTopRedBuffer)
	// 					fRedImageValue = 0;
	// 				else
	// 					fRedImageValue = ptrbyTopRedBuffer[b];
	// 				if (!ptrbyTopGreenBuffer)
	// 					fGreenImageValue = 0;
	// 				else
	// 					fGreenImageValue = ptrbyTopGreenBuffer[b];
	// 				if (!ptrbyTopBlueBuffer)
	// 					fBlueImageValue = 0;
	// 				else
	// 					fBlueImageValue = ptrbyTopBlueBuffer[b];
	// 				if (!ptrbyTopWhiteBuffer)
	// 					fWhiteImageValue = 0;
	// 				else
	// 					fWhiteImageValue = ptrbyTopWhiteBuffer[b];
	// 			}
	// 			else if(ptrnPosition[a] == 1)
	// 			{
	// 				if (!ptrbyMidRedBuffer)
	// 					fRedImageValue = 0;
	// 				else
	// 					fRedImageValue = ptrbyMidRedBuffer[b];
	// 				if (!ptrbyMidGreenBuffer)
	// 					fGreenImageValue = 0;
	// 				else
	// 					fGreenImageValue = ptrbyMidGreenBuffer[b];
	// 				if (!ptrbyMidBlueBuffer)
	// 					fBlueImageValue = 0;
	// 				else
	// 					fBlueImageValue = ptrbyMidBlueBuffer[b];
	// 				if (!ptrbyMidWhiteBuffer)
	// 					fWhiteImageValue = 0;
	// 				else
	// 					fWhiteImageValue = ptrbyMidWhiteBuffer[b];
	// 			}
	// 			else if(ptrnPosition[a] == 2)
	// 			{
	// 				if (!ptrbyBotRedBuffer)
	// 					fRedImageValue = 0;
	// 				else
	// 					fRedImageValue = ptrbyBotRedBuffer[b];
	// 				if (!ptrbyBotGreenBuffer)
	// 					fGreenImageValue = 0;
	// 				else
	// 					fGreenImageValue = ptrbyBotGreenBuffer[b];
	// 				if (!ptrbyBotBlueBuffer)
	// 					fBlueImageValue = 0;
	// 				else
	// 					fBlueImageValue = ptrbyBotBlueBuffer[b];
	// 				if (!ptrbyBotWhiteBuffer)
	// 					fWhiteImageValue = 0;
	// 				else
	// 					fWhiteImageValue = ptrbyBotWhiteBuffer[b];
	// 			}
	// 
	// 			nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));
	// 
	// 			if (nValueBuf > 255) 
	// 				nValueBuf = 255;
	// 			ppucComposeImageBuf[a][b] = nValueBuf;
	// 		}
	// 		m_pProcMilAlgo->SaveWorkImg(ppucComposeImageBuf[a], nWidth, nHeight, _T("ComposeImageBuf.bmp"));
	// 	}

	//UCHAR *pnImageValue = new UCHAR[nImageSize];
	UCHAR *pnImageValue = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	int nImgValue = 0;
	for (int a = 0; a < nImgCnt; a++)
	{
		for (int b = 0; b < nImageSize; b++)
		{
			if (a == 0)
				nImgValue = ppucComposeImageBuf[a][b];
			else
			{
				if (nCalculation[a - 1] == 1)
					nImgValue = pnImageValue[b] + ppucComposeImageBuf[a][b];
				else if (nCalculation[a - 1] == 2)
					nImgValue = pnImageValue[b] - ppucComposeImageBuf[a][b];
			}

			if (nImgValue > 255)
				nImgValue = 255;
			if (nImgValue < 0)
				nImgValue = 0;

			pnImageValue[b] = nImgValue;
		}
		m_pProcMilAlgo->SaveWorkImg(pnImageValue, nWidth, nHeight, _T("ImageValue.bmp"));
	}
	for (int i = 0; i < nImageSize; i++)
	{
		ptrbyResultImage[i] = pnImageValue[i];
	}
	m_pProcMilAlgo->SaveWorkImg(ptrbyResultImage, nWidth, nHeight, _T("ResultImage.bmp"));
	if (pnImageValue)
	{
		//delete [] pnImageValue;
		g_pMManager->pem_delete(pnImageValue, true);
		pnImageValue = NULL;
	}
	if (ppucComposeImageBuf)
	{
		for (int i = 0; i < nImgCnt; i++)
		{
			//delete ppucComposeImageBuf[i];

			// MemoryLeak Fix -wjlee
			//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
			g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

			ppucComposeImageBuf[i] = NULL;
		}
		//delete [] ppucComposeImageBuf;
		g_pMManager->pem_delete(ppucComposeImageBuf, true);
		ppucComposeImageBuf = NULL;
	}
}
void CPInsp_Algo::ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage, double dAngle)
{
	if (bFastCompose)
	{
		PInspAlgoWrapper* pInspWarpper = g_pInspMng->GePInspWrapper();
		pInspWarpper->ROIImageClaculCompose(sLightImg, ptrbyResultImage, dAngle);

		return;
	}
	int nFullImageWidth = sLightImg.m_nImgWidth;
	int nFullImageHeight = sLightImg.m_nImgHeight;
	int nROIWidth = sLightImg.m_nROIImgWidth;
	int nROIHeight = sLightImg.m_nROIImgHeight;
	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nFullImageWidth <= 0) || (nFullImageHeight <= 0) || (nImgCnt <= 0) || (nROIWidth <= 0) || (nROIHeight <= 0))
		return;
	double dROIX = sLightImg.m_dROIX;
	double dROIY = sLightImg.m_dROIY;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	int nValueBuf = 0;
	float fRedImageValue = 0.0;
	float fGreenImageValue = 0.0;
	float fBlueImageValue = 0.0;
	float fWhiteImageValue = 0.0;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nImageSize = nROIWidth * nROIHeight;
	UCHAR** ppucComposeImageBuf = NULL;
	//ppucComposeImageBuf = new UCHAR*[nImgCnt];
	ppucComposeImageBuf = g_pMManager->pem_new<UCHAR*>(true, nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	for(int i = 0; i < nImgCnt; i++)
	{
		//ppucComposeImageBuf[i] = new UCHAR[nImageSize];
		ppucComposeImageBuf[i] = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	}

	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);

	for (int a = 0; a < nImgCnt; a++)
	{
		nROIIndex = 0;
		nValueBuf = 0;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		int nRedValue = (int)(fRedValue * 128.0);
		int nGreenValue = (int)(fGreenValue * 128.0);
		int nBlueValue = (int)(fBlueValue * 128.0);
		int nWhiteValue = (int)(fWhiteValue * 128.0);

		__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
		__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
		__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
		__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);

		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * ippucComposeImageBuf = (UCHAR *)(&ppucComposeImageBuf[a][(y - nStartY)*nROIWidth]);
			int x;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue = zeroBuf;
				register __m128i fGreenImageValue = zeroBuf;
				register __m128i fBlueImageValue = zeroBuf;
				register __m128i fWhiteImageValue = zeroBuf;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopRedBuffer[FImgIdx]);
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopGreenBuffer[FImgIdx]);
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopBlueBuffer[FImgIdx]);
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidRedBuffer[FImgIdx]);
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidGreenBuffer[FImgIdx]);
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidBlueBuffer[FImgIdx]);
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotRedBuffer[FImgIdx]);
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotGreenBuffer[FImgIdx]);
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotBlueBuffer[FImgIdx]);
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotWhiteBuffer[FImgIdx]);
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				_mm_storeu_si128(
					(__m128i *)(&ippucComposeImageBuf[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyTopRedBuffer[FImgIdx];
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyTopGreenBuffer[FImgIdx];
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyTopBlueBuffer[FImgIdx];
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyTopWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyMidRedBuffer[FImgIdx];
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyMidGreenBuffer[FImgIdx];
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyMidBlueBuffer[FImgIdx];
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyMidWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyBotRedBuffer[FImgIdx];
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyBotGreenBuffer[FImgIdx];
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyBotBlueBuffer[FImgIdx];
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyBotWhiteBuffer[FImgIdx];
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				if (nValueBuf < 0)
					nValueBuf = 0;
				ippucComposeImageBuf[x - nStartX] = nValueBuf;
			}

		}
		m_pProcMilAlgo->SaveWorkImg(ppucComposeImageBuf[a], nROIWidth, nROIHeight, _T("ComposeImageBuf.bmp"));
	}

	int nImgValue = 0;
	for (int b = 0; b < nImageSize; b++)
	{
		for (int a = 0; a < nImgCnt; a++)
		{
			if (a == 0)
				nImgValue = ppucComposeImageBuf[a][b];
			else
			{
				if (nCalculation[a - 1] == 1)
					nImgValue = ptrbyResultImage[b] + ppucComposeImageBuf[a][b];
				else if (nCalculation[a - 1] == 2)
					nImgValue = ptrbyResultImage[b] - ppucComposeImageBuf[a][b];
			}

			if (nImgValue > 255)
				nImgValue = 255;
			if (nImgValue < 0)
				nImgValue = 0;

			ptrbyResultImage[b] = nImgValue;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(ptrbyResultImage, nROIWidth, nROIHeight, _T("pnImageValue.bmp"));

	if (ppucComposeImageBuf)
	{
		for (int i = 0; i < nImgCnt; i++)
		{
			// MemoryLeak Fix -wjlee
			g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

			ppucComposeImageBuf[i] = NULL;
		}
		//delete [] ppucComposeImageBuf;
		g_pMManager->pem_delete(ppucComposeImageBuf, true);
		ppucComposeImageBuf = NULL;
	}
	if (dAngle == 0)
		return;

	//shkim rotate
	UCHAR* ptrSrc = ptrbyResultImage;
	UCHAR* ptrbyBuffer_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;
	if (ptrbyResultImage != NULL)
	{
		m_pProcMilAlgo->RotateImg_ipp(ptrbyResultImage, nROIWidth, nROIHeight, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		GapX = RounD((retDstSizeX - nROIWidth) / 2);
		GapY = RounD((retDstSizeY - nROIHeight) / 2);
		if ((GapX + nROIWidth) > retDstSizeX)
			GapX = 0;
		if ((GapY + nROIHeight) > retDstSizeY)
			GapY = 0;
		for (int r = 0; r < nROIHeight; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];
			UCHAR* dstPtr = &ptrSrc[r*nROIWidth];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*nROIWidth);
		}
		m_pProcMilAlgo->SaveWorkImg(ptrbyBuffer_ro, retDstSizeX, retDstSizeY, _T("ptrbyBuffer_ro Red.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ptrSrc, nROIWidth, nROIHeight, _T("ptrbyRedBuffer.bmp"));

		if (ptrbyBuffer_ro != NULL)
			//delete [] ptrbyBuffer_ro;
			g_pMManager->pem_delete(ptrbyBuffer_ro, true);
	}
}

BOOL CPInsp_Algo::GetCoordinate(const WndAlgoImg &sWndAlgoImg, const WndInfo &sWndInfo, AlgoCoordinate *pAlgoCoodi, BOOL bWndErr)
{
	BOOL bResult = FALSE;

	double dResolX = m_resolX;
	double dResolY = m_resolY;

	if(g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}

	double fStartX = (sWndInfo.m_dPartWidth / 2.) + (sWndInfo.dCenterX);
	double fStartY = (sWndInfo.m_dPartHeight / 2.) - (sWndInfo.dCenterY);
	double fDX = (fStartX - (sWndInfo.dWidth / 2.)) / dResolX;
	double fDY = (fStartY - (sWndInfo.dLength / 2.)) / dResolY;
	double fDXR = fDX;// - fPartErrX;
	double fDYR = fDY;// - fPartErrY;
	int dX = RounD(fDX);
	int dY = RounD(fDY);
	if (dX < 0) dX = 0;
	if (dY < 0) dY = 0;

	double dWndW = sWndInfo.dWidth / dResolX;
	double dWndH = sWndInfo.dLength / dResolY;
	int nWndW = RounD(dWndW);
	int nWndH = RounD(dWndH);

	float fPartWidth = sWndInfo.m_dPartWidth / dResolX;
	float fPartHeight = sWndInfo.m_dPartHeight / dResolY;
	float fWndRoundingErrX = bWndErr ? (float)dX - fDX : 0.0f;
	float fWndRoundingErrY = bWndErr ? (float)dY - fDY : 0.0f;

	pAlgoCoodi->dROIAngle = sWndInfo.dAngle;
	pAlgoCoodi->dROICenterX = (float)(fPartWidth / 2. + (sWndInfo.dCenterX) / dResolX) + fWndRoundingErrX;
	pAlgoCoodi->dROICenterY = (float)(fPartHeight / 2. - (sWndInfo.dCenterY) / dResolY) + fWndRoundingErrY;
	pAlgoCoodi->dROILength = (float)(sWndInfo.dLength / dResolX);
	pAlgoCoodi->dROIWidth = (float)(sWndInfo.dWidth / dResolX);
	pAlgoCoodi->dWndX = fDXR;
	pAlgoCoodi->dWndY = fDYR;
	pAlgoCoodi->dPartW = sWndInfo.m_dPartWidth;
	pAlgoCoodi->dPartH = sWndInfo.m_dPartHeight;

	return bResult = TRUE;
}
// sWndAlgoImg 는 Window Image, sWndInfo는 Window 정보를 받는 경우 사용
BOOL CPInsp_Algo::GetWndClip_2(WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, const WndInfo &sWndInfo, int *nStartX, int *nStartY)
{
	BOOL bResult = FALSE;

	if (sWndAlgoImg.m_nChannel == 3)
		return TRUE;
	double dClipWidth = (float)(sWndInfo.dWidth / m_resolX);
	double dClipHeight = (float)(sWndInfo.dLength / m_resolY);
	if (dClipWidth > sWndAlgoImg.m_nWidth)
		dClipWidth = sWndAlgoImg.m_nWidth;
	if (dClipHeight > sWndAlgoImg.m_nHeight)
		dClipHeight = sWndAlgoImg.m_nHeight;

	if (((int)dClipWidth <= 0) || ((int)dClipHeight <= 0))
		return bResult;

	sWndClipAlgo.dAngle = sWndAlgoImg.dAngle;

	double dClipX2D = (double)sWndAlgoImg.m_nWidth / 2. + (sWndInfo.dCenterX / m_resolX);
	double dClipY2D = (double)sWndAlgoImg.m_nHeight / 2. - (sWndInfo.dCenterY / m_resolY);
	double dClipX3D = (double)sWndAlgoImg.m_nWidth3D / 2. + (sWndInfo.dCenterX / m_resolX);
	double dClipY3D = (double)sWndAlgoImg.m_nHeight3D / 2. - (sWndInfo.dCenterY / m_resolY);

	if (dClipX2D <= dClipWidth / 2.)
		dClipX2D = dClipWidth / 2.;
	if (dClipY2D <= dClipHeight / 2.)
		dClipY2D = dClipHeight / 2.;

	if (dClipX3D <= dClipWidth / 2.)
		dClipX3D = dClipWidth / 2.;
	if (dClipY3D <= dClipHeight / 2.)
		dClipY3D = dClipHeight / 2.;

	// 20141008 SHW
// 	if(dClipWidth > sWndAlgoImg.m_nWidth)
// 		dClipX2D = sWndAlgoImg.m_nWidth / 2.;
// 	if(dClipHeight > sWndAlgoImg.m_nHeight)
// 		dClipY2D = sWndAlgoImg.m_nHeight / 2.;

	int nClipWidth = RounD(dClipWidth);
	int nClipHeight = RounD(dClipHeight);
	int nStartX2D = RounD(dClipX2D - (dClipWidth / 2.0));
	int nStartY2D = RounD(dClipY2D - (dClipHeight / 2.0));
	int nStartX3D = RounD(dClipX3D - (dClipWidth / 2.0));
	int nStartY3D = RounD(dClipY3D - (dClipHeight / 2.0));
	if (sWndInfo.m_nWndX > 0 && sWndInfo.m_nWndY > 0)
	{
		nStartX3D = sWndInfo.m_nWndX;
		nStartY3D = sWndInfo.m_nWndY;
	}

	if (sWndAlgoImg.m_nWidth3D != 0)
	{
		if (nStartX2D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX2D = sWndAlgoImg.m_nWidth - nClipWidth;
		if (nStartX3D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX3D = sWndAlgoImg.m_nWidth3D - nClipWidth;
	}
	else if (sWndAlgoImg.m_nWidth > 0)
	{
		if (nStartX2D + nClipWidth > sWndAlgoImg.m_nWidth)
			nStartX2D = sWndAlgoImg.m_nWidth - nClipWidth;
	}

	if (sWndAlgoImg.m_nHeight3D != 0)
	{
		if (nStartY2D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY2D = sWndAlgoImg.m_nHeight - dClipHeight;
		if (nStartY3D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY3D = sWndAlgoImg.m_nHeight3D - dClipHeight;
	}
	else if (sWndAlgoImg.m_nHeight > 0)
	{
		if (nStartY2D + dClipHeight > sWndAlgoImg.m_nHeight)
			nStartY2D = sWndAlgoImg.m_nHeight - dClipHeight;
	}

	if (sWndAlgoImg.m_fArr3D_part)
	{
		m_pProcMilAlgo->SaveWorkImg_float(sWndAlgoImg.m_fArr3D_part, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D, _T("Org3D.bmp"));

		//sWndClipAlgo.m_fArr3D = new float[nClipWidth * nClipHeight];
		sWndClipAlgo.m_fArr3D = g_pMManager->pem_new<float>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D_part, sWndClipAlgo.m_fArr3D, sWndAlgoImg.m_nWidth3D,
			sWndAlgoImg.m_nHeight3D, nStartX3D, nStartY3D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg_float(sWndClipAlgo.m_fArr3D, nClipWidth, nClipHeight, _T("After3D.bmp"));

		if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
		{
			ArrayCopy_float(sWndClipAlgo.m_fArr3D, &sWndAlgoImg.m_fArr3D, nClipWidth, nClipHeight);
			m_pProcMilAlgo->SaveWorkImg_float(sWndAlgoImg.m_fArr3D, nClipWidth, nClipHeight, _T("New3D.bmp"));
		}
		else
		{
			ArrayCopy_float(sWndAlgoImg.m_fArr3D_part, &sWndAlgoImg.m_fArr3D, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D);
		}
	}

	if (sWndAlgoImg.m_ucArrCV && sWndAlgoImg.m_nChannel == 3)
	{
		m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArrCV, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Org2D.bmp"), sWndAlgoImg.m_nChannel);

		//sWndClipAlgo.m_ucArr2D = new UCHAR[nClipWidth * nClipHeight/* *  sWndAlgoImg.m_nChannel*/];
		sWndClipAlgo.m_ucArr2D = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);

		if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
		{
			if (sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0)
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArrCV, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, 0, 0, nClipWidth, nClipHeight);
			}
			else
			{
				sWndClipAlgo.m_ucArr2D = sWndAlgoImg.m_ucArrCV;
			}
		}
		else
		{
			m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArrCV, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
				sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		}
		m_pProcMilAlgo->SaveWorkImg(sWndClipAlgo.m_ucArr2D, nClipWidth, nClipHeight, _T("After2D.bmp"), sWndAlgoImg.m_nChannel);
	}
	else
	{
		m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Org2D.bmp"));

		//sWndClipAlgo.m_ucArr2D = new UCHAR[nClipWidth * nClipHeight];
		sWndClipAlgo.m_ucArr2D = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);

		if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
		{
			if ((sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0 && sWndAlgoImg.m_nLight_index < 0) ||
				(sWndAlgoImg.m_nLight_index >= 0 && sWndAlgoImg.m_nWidth == nClipWidth && sWndAlgoImg.m_nHeight == nClipHeight))
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, 0, 0, nClipWidth, nClipHeight);
			}
			else
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			}
		}
		else
		{
			m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
				sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		}
		m_pProcMilAlgo->SaveWorkImg(sWndClipAlgo.m_ucArr2D, nClipWidth, nClipHeight, _T("After2D.bmp"));
	}

	//LJH 2016.06.07
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
	{
		if (sWndAlgoImg.m_ucArr2D_Mix[iLoopCount])
		{
			m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Org2D_Mix1.bmp"));

			//sWndClipAlgo.m_ucArr2D_Mix[iLoopCount] = new UCHAR[nClipWidth * nClipHeight];
			sWndClipAlgo.m_ucArr2D_Mix[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);

			if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
			{
				if (sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0)
				{
					m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
						sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], 0, 0, nClipWidth, nClipHeight);
				}
				else
				{
					m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
						sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nStartX2D, nStartY2D, nClipWidth, nClipHeight);
				}
			}
			else
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			}
			m_pProcMilAlgo->SaveWorkImg(sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nClipWidth, nClipHeight, _T("After2D_Mix1.bmp"));
		}
	}

	if (nStartX) *nStartX = nStartX2D;
	if (nStartY) *nStartY = nStartY2D;
	sWndClipAlgo.m_nWidth = nClipWidth;
	sWndClipAlgo.m_nHeight = nClipHeight;
	bResult = TRUE;

	return bResult;
}
// YJS 2016/11/04
void CPInsp_Algo::SetBackWndClip_2(WndAlgoImg &sWndAlgoImg, const WndInfo &sWndInfo, UCHAR * pClip, UCHAR * pOrg)
{
	double dClipWidth = (float)(sWndInfo.dWidth / m_resolX);
	double dClipHeight = (float)(sWndInfo.dLength / m_resolY);
	if (dClipWidth > sWndAlgoImg.m_nWidth)
		dClipWidth = sWndAlgoImg.m_nWidth;
	if (dClipHeight > sWndAlgoImg.m_nHeight)
		dClipHeight = sWndAlgoImg.m_nHeight;

	if ((dClipWidth <= 0) || (dClipHeight <= 0))
		return;

	double dClipX2D = (double)sWndAlgoImg.m_nWidth / 2. + (sWndInfo.dCenterX / m_resolX);
	double dClipY2D = (double)sWndAlgoImg.m_nHeight / 2. - (sWndInfo.dCenterY / m_resolY);
	double dClipX3D = (double)sWndAlgoImg.m_nWidth3D / 2. + (sWndInfo.dCenterX / m_resolX);
	double dClipY3D = (double)sWndAlgoImg.m_nHeight3D / 2. - (sWndInfo.dCenterY / m_resolY);

	if (dClipX2D <= dClipWidth / 2.)
		dClipX2D = dClipWidth / 2.;
	if (dClipY2D <= dClipHeight / 2.)
		dClipY2D = dClipHeight / 2.;

	if (dClipX3D <= dClipWidth / 2.)
		dClipX3D = dClipWidth / 2.;
	if (dClipY3D <= dClipHeight / 2.)
		dClipY3D = dClipHeight / 2.;

	int nClipWidth = RounD(dClipWidth);
	int nClipHeight = RounD(dClipHeight);
	int nStartX2D = RounD(dClipX2D - (dClipWidth / 2.0));
	int nStartY2D = RounD(dClipY2D - (dClipHeight / 2.0));
	int nStartX3D = RounD(dClipX3D - (dClipWidth / 2.0));
	int nStartY3D = RounD(dClipY3D - (dClipHeight / 2.0));

	if (sWndAlgoImg.m_nWidth3D != 0)
	{
		if (nStartX2D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX2D = sWndAlgoImg.m_nWidth - nClipWidth;
		if (nStartX3D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX3D = sWndAlgoImg.m_nWidth3D - nClipWidth;
	}
	if (sWndAlgoImg.m_nHeight3D != 0)
	{
		if (nStartY2D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY2D = sWndAlgoImg.m_nHeight - dClipHeight;
		if (nStartY3D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY3D = sWndAlgoImg.m_nHeight3D - dClipHeight;
	}

	if (sWndAlgoImg.m_ucArr2D && pClip && pOrg)
	{
		m_pProcMilAlgo->SaveWorkImg(pClip, nClipWidth, nClipHeight, _T("Org2D.bmp"));

		int srcPitch = sWndAlgoImg.m_nWidth;
		/*int align = (srcPitch % 4);
		if (align > 0)
			align = 4 - align;*/
		int align = 0;
		srcPitch += align;

		if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
		{
			if (sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0)
			{
				m_pProcMilAlgo->SetBackClipImage_LT(pOrg, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, srcPitch,
					pClip, 0, 0, nClipWidth, nClipHeight);
			}
			else
			{
				m_pProcMilAlgo->SetBackClipImage_LT(pOrg, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, srcPitch,
					pClip, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			}
		}
		else
		{
			m_pProcMilAlgo->SetBackClipImage_LT(pOrg, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, srcPitch,
				pClip, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		}
		m_pProcMilAlgo->SaveWorkImg(pOrg, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("After2D.bmp"));
	}
}
// sWndAlgoImg 는 Part 전체 Image, sWndInfo는 Window 정보를 받는 경우 사용
BOOL CPInsp_Algo::GetWndClip_3(WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, const WndInfo &sWndInfo, int *nStartX, int *nStartY, BOOL bIsLoadFovRawDataImage, bool bPartErr)
{
	BOOL bResult = FALSE;

	double dResolX = m_resolX;
	double dResolY = m_resolY;

	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}

	double dClipWidth = (float)(sWndInfo.dWidth / dResolX/*m_resolX*/);
	double dClipHeight = (float)(sWndInfo.dLength / dResolY/*m_resolY*/);
	if ((dClipWidth <= 0) || (dClipHeight <= 0))
		return bResult;

	sWndClipAlgo.dAngle = sWndAlgoImg.dAngle;

	float nClipWndWidth = (sWndInfo.dWidth / dResolX);
	float nClipWndHeight = (sWndInfo.dLength / dResolY);

	float fClipPartWidth = (sWndInfo.m_dPartWidth / dResolX);
	float fClipPartHeight =(sWndInfo.m_dPartHeight / dResolY);

	float dClipX2D = ((sWndInfo.m_dPartWidth / 2.) + sWndInfo.dCenterX - (sWndInfo.dWidth / 2.)) / dResolX;
	float dClipY2D = ((sWndInfo.m_dPartHeight / 2.) - sWndInfo.dCenterY - (sWndInfo.dLength / 2.)) / dResolY;
	float dClipX3D = ((sWndInfo.m_dPartWidth / 2.) + sWndInfo.dCenterX - (sWndInfo.dWidth / 2.)) / dResolX;
	float dClipY3D = ((sWndInfo.m_dPartHeight / 2.) - sWndInfo.dCenterY - (sWndInfo.dLength / 2.)) / dResolY;

	//Get Part clip Rounding-Err
	float fPartClipErrX = sWndAlgoImg.m_fPartRoundingErrX;
	float fPartClipErrY = sWndAlgoImg.m_fPartRoundingErrY;
	if (bPartErr)
	{
	dClipX2D -= fPartClipErrX;
	dClipY2D -= fPartClipErrY;
	dClipX3D -= fPartClipErrX;
	dClipY3D -= fPartClipErrY;
	}

	// 20141008 SHW
	if (dClipWidth > sWndAlgoImg.m_nWidth)
		dClipX2D = sWndAlgoImg.m_nWidth / 2;
	if (dClipHeight > sWndAlgoImg.m_nHeight)
		dClipY2D = sWndAlgoImg.m_nHeight / 2;

	int nClipWidth = RounD(dClipWidth);
	int nClipHeight = RounD(dClipHeight);
	int nStartX2D = RounD(dClipX2D);
	int nStartY2D = RounD(dClipY2D);
	int nStartX3D = RounD(dClipX3D);
	int nStartY3D = RounD(dClipY3D);
	if (nStartX2D < 0)
		nStartX2D = 0;
	if (nStartY2D < 0)
		nStartY2D = 0;

	if (nStartX3D <= 0)
		nStartX3D = 0;
	if (nStartY3D <= 0)
		nStartY3D = 0;

	//Save window start Pos Rounding-Err NYJ 2020/12/17
	sWndClipAlgo.m_fWndRoundingErrX = (float)nStartX2D - (float)dClipX2D;
	sWndClipAlgo.m_fWndRoundingErrY = (float)nStartY2D - (float)dClipY2D;

	if (sWndAlgoImg.m_nWidth3D != 0)
	{
		if (nStartX2D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX2D = sWndAlgoImg.m_nWidth - nClipWidth;
		if (nStartX3D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX3D = sWndAlgoImg.m_nWidth3D - nClipWidth;
	}
	if (sWndAlgoImg.m_nHeight3D != 0)
	{
		if (nStartY2D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY2D = sWndAlgoImg.m_nHeight - dClipHeight;
		if (nStartY3D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY3D = sWndAlgoImg.m_nHeight3D - dClipHeight;
	}

	if (sWndAlgoImg.m_fArr3D_part && g_pMPTI->m_bSideOriginalSize != true && sWndAlgoImg.m_nWidth3D > 0 && sWndAlgoImg.m_nHeight3D > 0)
	{
		m_pProcMilAlgo->SaveWorkImg_float(sWndAlgoImg.m_fArr3D_part, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D, _T("Org3D.bmp"));

		//sWndClipAlgo.m_fArr3D = new float[nClipWidth * nClipHeight];
		sWndClipAlgo.m_fArr3D = g_pMManager->pem_new<float>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D_part, sWndClipAlgo.m_fArr3D, sWndAlgoImg.m_nWidth3D,
			sWndAlgoImg.m_nHeight3D, nStartX3D, nStartY3D, nClipWidth, nClipHeight, sWndInfo.m_dAlignAngle);
		m_pProcMilAlgo->SaveWorkImg_float(sWndClipAlgo.m_fArr3D, nClipWidth, nClipHeight, _T("After3D.bmp"));

#if _DEBUG
		cv::Mat imgArr3D_part(sWndAlgoImg.m_nHeight3D, sWndAlgoImg.m_nWidth3D, CV_32FC1, sWndAlgoImg.m_fArr3D_part);
		cv::Mat imgArr3D(nClipHeight, nClipWidth, CV_32FC1, sWndClipAlgo.m_fArr3D);
#endif
		if (bIsLoadFovRawDataImage)
		{
			bool bW = sWndAlgoImg.m_nWidth3D == sWndAlgoImg.m_nWidth;
			bool bH = sWndAlgoImg.m_nHeight3D == sWndAlgoImg.m_nHeight;
			bool bW4 = false;
			bool bH4 = false;
			if (bW == false)
			{
				int nGap1 = (sWndAlgoImg.m_nWidth3D % 4);
				int nGap2 = (sWndAlgoImg.m_nWidth % 4);
				int nStep1 = 4 - nGap1;
				int nStep2 = 4 - nGap2;
				int nStep3 = nStep1 == 4 ? sWndAlgoImg.m_nWidth3D : sWndAlgoImg.m_nWidth3D + nStep1;
				int nStep4 = nStep2 == 4 ? sWndAlgoImg.m_nWidth : sWndAlgoImg.m_nWidth + nStep2;
				int nStep5 = sWndAlgoImg.m_nWidth3D - nGap1;
				int nStep6 = sWndAlgoImg.m_nWidth - nGap2;
				bW4 = (nStep3 == nStep4) || (nStep5 == nStep6);
			}
			if (bH == false)
			{
				int nGap1 = (sWndAlgoImg.m_nHeight3D % 4);
				int nGap2 = (sWndAlgoImg.m_nHeight % 4);
				int nStep1 = 4 - nGap1;
				int nStep2 = 4 - nGap2;
				int nStep3 = nStep1 == 4 ? sWndAlgoImg.m_nHeight3D : sWndAlgoImg.m_nHeight3D + nStep1;
				int nStep4 = nStep2 == 4 ? sWndAlgoImg.m_nHeight : sWndAlgoImg.m_nHeight + nStep2;
				int nStep5 = sWndAlgoImg.m_nHeight3D - nGap1;
				int nStep6 = sWndAlgoImg.m_nHeight - nGap2;
				bH4 = (nStep3 == nStep4) || (nStep5 == nStep6);
			}

			if (bW && bH)
				ArrayCopy_float(sWndAlgoImg.m_fArr3D_part, &sWndAlgoImg.m_fArr3D, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D);
			else if ((!bW && bW4) || (!bH && bH4))
				ArrayCopy_float(&sWndAlgoImg);
			else
				ArrayCopy_float(sWndClipAlgo.m_fArr3D, &sWndAlgoImg.m_fArr3D, nClipWidth, nClipHeight);
		}
		else
		{
			if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
			{
				ArrayCopy_float(sWndClipAlgo.m_fArr3D, &sWndAlgoImg.m_fArr3D, nClipWidth, nClipHeight);
				m_pProcMilAlgo->SaveWorkImg_float(sWndAlgoImg.m_fArr3D, nClipWidth, nClipHeight, _T("New3D.bmp"));
			}
			else
			{
				ArrayCopy_float(sWndAlgoImg.m_fArr3D_part, &sWndAlgoImg.m_fArr3D, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D);
			}
		}
	}
	if (sWndAlgoImg.m_ucArr2D)
	{
#if _DEBUG
		cv::Mat imgArr2D(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D);
#endif
		m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Org2D.bmp"));

		//sWndClipAlgo.m_ucArr2D = new UCHAR[nClipWidth * nClipHeight];
		sWndClipAlgo.m_ucArr2D = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);

		if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
		{
			if ((sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0 && sWndAlgoImg.m_nLight_index < 0) 
				&& nStartX2D + nClipWidth <= sWndAlgoImg.m_nWidth 
				&& nStartY2D + nClipHeight <= sWndAlgoImg.m_nHeight)
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
			}
			else if ((sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0 && sWndAlgoImg.m_nLight_index < 0) || 
				(sWndAlgoImg.m_nLight_index >= 0 && sWndAlgoImg.m_nWidth == nClipWidth && sWndAlgoImg.m_nHeight == nClipHeight))
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, 0, 0, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
			}
			else
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
			}
		}
		else
		{
			m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
				sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
		}
		m_pProcMilAlgo->SaveWorkImg(sWndClipAlgo.m_ucArr2D, nClipWidth, nClipHeight, _T("After2D.bmp"));
	}

	//LJH 2016.06.07 
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
	{
		if (sWndAlgoImg.m_ucArr2D_Mix[iLoopCount])
		{
			m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Org2D_Mix1.bmp"));

			//sWndClipAlgo.m_ucArr2D_Mix[iLoopCount] = new UCHAR[nClipWidth * nClipHeight];
			sWndClipAlgo.m_ucArr2D_Mix[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);

			if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
			{
				if (sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0)
				{
					m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
						sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], 0, 0, nClipWidth, nClipHeight);
				}
				else
				{
					m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
						sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nStartX2D, nStartY2D, nClipWidth, nClipHeight);
				}
			}
			else
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			}
			m_pProcMilAlgo->SaveWorkImg(sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nClipWidth, nClipHeight, _T("After2D_Mix1.bmp"));
		}
	}

	sWndClipAlgo.m_nWidth = nClipWidth;
	sWndClipAlgo.m_nHeight = nClipHeight;
	sWndClipAlgo.m_nWidth3D = sWndAlgoImg.m_nWidth3D;
	sWndClipAlgo.m_nHeight3D = sWndAlgoImg.m_nHeight3D;
	*nStartX = nStartX2D;
	*nStartY = nStartY2D;
	bResult = TRUE;

	return bResult;
}
BOOL CPInsp_Algo::GetWndClip_3_2(WndAlgoImg& sWndAlgoImg, WndAlgoImg& sWndClipAlgo, const WndInfo& sWndInfo, int* nStartX, int* nStartY, BOOL bIsLoadFovRawDataImage, bool bPartErr)
{
	BOOL bResult = FALSE;

	double dResolX = m_resolX;
	double dResolY = m_resolY;

	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}

	double dClipWidth = (float)(sWndInfo.dWidth / dResolX/*m_resolX*/);
	double dClipHeight = (float)(sWndInfo.dLength / dResolY/*m_resolY*/);
	if ((dClipWidth <= 0) || (dClipHeight <= 0))
		return bResult;

	sWndClipAlgo.dAngle = sWndAlgoImg.dAngle;

	float nClipWndWidth = (sWndInfo.dWidth / dResolX);
	float nClipWndHeight = (sWndInfo.dLength / dResolY);

	float fClipPartWidth = (sWndInfo.m_dPartWidth / dResolX);
	float fClipPartHeight = (sWndInfo.m_dPartHeight / dResolY);

	float dClipX2D = ((sWndInfo.m_dPartWidth / 2.) + sWndInfo.dCenterX - (sWndInfo.dWidth / 2.)) / dResolX;
	float dClipY2D = ((sWndInfo.m_dPartHeight / 2.) - sWndInfo.dCenterY - (sWndInfo.dLength / 2.)) / dResolY;
	float dClipX3D = ((sWndInfo.m_dPartWidth / 2.) + sWndInfo.dCenterX - (sWndInfo.dWidth / 2.)) / dResolX;
	float dClipY3D = ((sWndInfo.m_dPartHeight / 2.) - sWndInfo.dCenterY - (sWndInfo.dLength / 2.)) / dResolY;

	//Get Part clip Rounding-Err
	float fPartClipErrX = sWndAlgoImg.m_fPartRoundingErrX;
	float fPartClipErrY = sWndAlgoImg.m_fPartRoundingErrY;
	if (bPartErr)
	{
		dClipX2D -= fPartClipErrX;
		dClipY2D -= fPartClipErrY;
		dClipX3D -= fPartClipErrX;
		dClipY3D -= fPartClipErrY;
	}

	// 20141008 SHW
	if (dClipWidth > sWndAlgoImg.m_nWidth)
		dClipX2D = sWndAlgoImg.m_nWidth / 2;
	if (dClipHeight > sWndAlgoImg.m_nHeight)
		dClipY2D = sWndAlgoImg.m_nHeight / 2;

	int nClipWidth = RounD(dClipWidth);
	int nClipHeight = RounD(dClipHeight);
	int nStartX2D = RounD(dClipX2D);
	int nStartY2D = RounD(dClipY2D);
	int nStartX3D = RounD(dClipX3D);
	int nStartY3D = RounD(dClipY3D);
	if (nStartX2D < 0)
		nStartX2D = 0;
	if (nStartY2D < 0)
		nStartY2D = 0;

	if (nStartX3D <= 0)
		nStartX3D = 0;
	if (nStartY3D <= 0)
		nStartY3D = 0;

	//Save window start Pos Rounding-Err NYJ 2020/12/17
	sWndClipAlgo.m_fWndRoundingErrX = (float)nStartX2D - (float)dClipX2D;
	sWndClipAlgo.m_fWndRoundingErrY = (float)nStartY2D - (float)dClipY2D;

	if (sWndAlgoImg.m_nWidth3D != 0)
	{
		if (nStartX2D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX2D = sWndAlgoImg.m_nWidth - nClipWidth;
		if (nStartX3D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX3D = sWndAlgoImg.m_nWidth3D - nClipWidth;
	}
	if (sWndAlgoImg.m_nHeight3D != 0)
	{
		if (nStartY2D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY2D = sWndAlgoImg.m_nHeight - dClipHeight;
		if (nStartY3D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY3D = sWndAlgoImg.m_nHeight3D - dClipHeight;
	}

	if (sWndAlgoImg.m_fArr3D_part && g_pMPTI->m_bSideOriginalSize != true && sWndAlgoImg.m_nWidth3D > 0 && sWndAlgoImg.m_nHeight3D > 0)
	{
		m_pProcMilAlgo->SaveWorkImg_float(sWndAlgoImg.m_fArr3D_part, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D, _T("Org3D.bmp"));

		//sWndClipAlgo.m_fArr3D = new float[nClipWidth * nClipHeight];
		sWndClipAlgo.m_fArr3D = g_pMManager->pem_new<float>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D_part, sWndClipAlgo.m_fArr3D, sWndAlgoImg.m_nWidth3D,
			sWndAlgoImg.m_nHeight3D, nStartX3D, nStartY3D, nClipWidth, nClipHeight, sWndInfo.m_dAlignAngle);
		m_pProcMilAlgo->SaveWorkImg_float(sWndClipAlgo.m_fArr3D, nClipWidth, nClipHeight, _T("After3D.bmp"));

#if _DEBUG
		cv::Mat imgArr3D_part(sWndAlgoImg.m_nHeight3D, sWndAlgoImg.m_nWidth3D, CV_32FC1, sWndAlgoImg.m_fArr3D_part);
		cv::Mat imgArr3D(nClipHeight, nClipWidth, CV_32FC1, sWndClipAlgo.m_fArr3D);
#endif
	}
	if (sWndAlgoImg.m_ucArr2D)
	{
#if _DEBUG
		cv::Mat imgArr2D(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D);
#endif
		m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Org2D.bmp"));

		//sWndClipAlgo.m_ucArr2D = new UCHAR[nClipWidth * nClipHeight];
		sWndClipAlgo.m_ucArr2D = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);

		if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
		{
			if ((sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0 && sWndAlgoImg.m_nLight_index < 0)
				&& nStartX2D + nClipWidth <= sWndAlgoImg.m_nWidth
				&& nStartY2D + nClipHeight <= sWndAlgoImg.m_nHeight)
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
			}
			else if ((sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0 && sWndAlgoImg.m_nLight_index < 0) ||
				(sWndAlgoImg.m_nLight_index >= 0 && sWndAlgoImg.m_nWidth == nClipWidth && sWndAlgoImg.m_nHeight == nClipHeight))
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, 0, 0, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
			}
			else
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
			}
		}
		else
		{
			m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
				sWndClipAlgo.m_ucArr2D, nStartX2D, nStartY2D, nClipWidth, nClipHeight, 1, sWndInfo.m_dAlignAngle);
		}
		m_pProcMilAlgo->SaveWorkImg(sWndClipAlgo.m_ucArr2D, nClipWidth, nClipHeight, _T("After2D.bmp"));
	}

	//LJH 2016.06.07 
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
	{
		if (sWndAlgoImg.m_ucArr2D_Mix[iLoopCount])
		{
			m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Org2D_Mix1.bmp"));

			//sWndClipAlgo.m_ucArr2D_Mix[iLoopCount] = new UCHAR[nClipWidth * nClipHeight];
			sWndClipAlgo.m_ucArr2D_Mix[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);

			if (sWndAlgoImg.m_nWidth3D != sWndAlgoImg.m_nWidth && sWndAlgoImg.m_nHeight3D != sWndAlgoImg.m_nHeight)
			{
				if (sWndAlgoImg.m_nWidth3D != 0 && sWndAlgoImg.m_nHeight3D != 0)
				{
					m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
						sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], 0, 0, nClipWidth, nClipHeight);
				}
				else
				{
					m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
						sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nStartX2D, nStartY2D, nClipWidth, nClipHeight);
				}
			}
			else
			{
				m_pProcMilAlgo->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[iLoopCount], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,
					sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			}
			m_pProcMilAlgo->SaveWorkImg(sWndClipAlgo.m_ucArr2D_Mix[iLoopCount], nClipWidth, nClipHeight, _T("After2D_Mix1.bmp"));
		}
	}

	sWndClipAlgo.m_nWidth = nClipWidth;
	sWndClipAlgo.m_nHeight = nClipHeight;
	sWndClipAlgo.m_nWidth3D = sWndAlgoImg.m_nWidth3D;
	sWndClipAlgo.m_nHeight3D = sWndAlgoImg.m_nHeight3D;
	*nStartX = nStartX2D;
	*nStartY = nStartY2D;
	bResult = TRUE;

	return bResult;
}
BOOL CPInsp_Algo::GetWndClip_4(InspRoiImgBuf &sImgBuf, InspRoiImgBuf &sClipImgBuf, const WndInfo &sWndInfo, int nStartX, int nStartY)
{
	BOOL bResult = FALSE;

	if ((sImgBuf.nImageSizeX <= 0) || (sImgBuf.nImageSizeY <= 0))
		return bResult;

	double dResolX = m_resolX;
	double dResolY = m_resolY;

	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}

	double dClipWidth = (float)(sWndInfo.dWidth / dResolX/*m_resolX*/);
	double dClipHeight = (float)(sWndInfo.dLength / dResolY/*m_resolY*/);
	if ((dClipWidth <= 0) || (dClipHeight <= 0))
		return bResult;

	double dClipX2D = ((sWndInfo.m_dPartWidth / 2.) + (sWndInfo.dCenterX) - (sWndInfo.dWidth / 2.0)) / dResolX/*m_resolX*/;
	double dClipY2D = ((sWndInfo.m_dPartHeight / 2.) - (sWndInfo.dCenterY) - (sWndInfo.dLength / 2.0)) / dResolY/*m_resolY*/;
	double dClipX3D = ((sWndInfo.m_dPartWidth / 2.) + (sWndInfo.dCenterX) - (sWndInfo.dWidth / 2.0)) / dResolX/*m_resolX*/;
	double dClipY3D = ((sWndInfo.m_dPartHeight / 2.) - (sWndInfo.dCenterY) - (sWndInfo.dLength / 2.0)) / dResolY/*m_resolY*/;

	if (dClipX2D < 0)
		dClipX2D = 0;
	if (dClipY2D < 0)
		dClipY2D = 0;

	if (dClipX3D <= 0)
		dClipX3D = 0;
	if (dClipY3D <= 0)
		dClipY3D = 0;

	// 20141008 SHW
	if (dClipWidth > sImgBuf.nImageSizeX)
		dClipX2D = sImgBuf.nImageSizeX / 2;
	if (dClipHeight > sImgBuf.nImageSizeY)
		dClipY2D = sImgBuf.nImageSizeY / 2;

	int nClipWidth = RounD(dClipWidth);
	int nClipHeight = RounD(dClipHeight);
	int nStartX2D = RounD(dClipX2D);
	int nStartY2D = RounD(dClipY2D);
	int nStartX3D = RounD(dClipX3D);
	int nStartY3D = RounD(dClipY3D);
	if (nStartX > 0 && nStartY > 0)
	{
		nStartX2D = nStartX;
		nStartY2D = nStartY;
	}
	if (nClipWidth > sImgBuf.nImageSizeX)nClipWidth = sImgBuf.nImageSizeX;
	if (nClipHeight > sImgBuf.nImageSizeY)nClipHeight = sImgBuf.nImageSizeY;

	if (sImgBuf.nImageSizeX != 0)
	{
		if (nStartX2D + nClipWidth > sImgBuf.nImageSizeX)
			nStartX2D = sImgBuf.nImageSizeX - nClipWidth;
		if (nStartX3D + nClipWidth > sImgBuf.nImageSizeX)
			nStartX3D = sImgBuf.nImageSizeX - nClipWidth;
	}
	if (sImgBuf.nImageSizeY != 0)
	{
		if (nStartY2D + dClipHeight > sImgBuf.nImageSizeY)
			nStartY2D = sImgBuf.nImageSizeY - dClipHeight;
		if (nStartY3D + dClipHeight > sImgBuf.nImageSizeY)
			nStartY3D = sImgBuf.nImageSizeY - dClipHeight;
	}

	if (sImgBuf.imgTop_R)
	{
		/*m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgTop_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgImgTop_R.bmp"));
		sClipImgBuf.imgTop_R = new UCHAR[nClipWidth * nClipHeight];
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgTop_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgTop_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_R, nClipWidth, nClipHeight, _T("AfterImgTop_R.bmp"));*/

		if (g_pMPTI->m_bSideOriginalSize == true)	// 하부 카메라 사용시 칼라 R.G.B 이미지 교체
		{
			m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide1_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgImgTop_Side_R.bmp"));
			//sClipImgBuf.imgTop_R = new UCHAR[nClipWidth * nClipHeight];
			sClipImgBuf.imgTop_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide1_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
				sClipImgBuf.imgTop_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_R, nClipWidth, nClipHeight, _T("AfterImgTop_Side1_R.bmp"));
		}
		else
		{
			m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgTop_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgImgTop_R.bmp"));
			//sClipImgBuf.imgTop_R = new UCHAR[nClipWidth * nClipHeight];
			sClipImgBuf.imgTop_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgTop_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
				sClipImgBuf.imgTop_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_R, nClipWidth, nClipHeight, _T("AfterImgTop_R.bmp"));
		}
	}
	else
		sClipImgBuf.imgTop_R = NULL;
	if (sImgBuf.imgTop_G)
	{
		// 		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgTop_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgImgTop_G.bmp"));
		// 		sClipImgBuf.imgTop_G = new UCHAR[nClipWidth * nClipHeight];
		// 		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgTop_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, 
		// 			sClipImgBuf.imgTop_G, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		// 		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_G, nClipWidth, nClipHeight, _T("AfterImgTop_G.bmp"));
		if (g_pMPTI->m_bSideOriginalSize == true)	// 하부 카메라 사용시 칼라 R.G.B 이미지 교체
		{
			m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide1_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgImgTop_Side_G.bmp"));
			//sClipImgBuf.imgTop_G = new UCHAR[nClipWidth * nClipHeight];
			sClipImgBuf.imgTop_G = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide1_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
				sClipImgBuf.imgTop_G, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_G, nClipWidth, nClipHeight, _T("AfterImgTop_Side1_G.bmp"));
		}
		else
		{
			m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgTop_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgImgTop_G.bmp"));
			//sClipImgBuf.imgTop_G = new UCHAR[nClipWidth * nClipHeight];
			sClipImgBuf.imgTop_G = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgTop_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
				sClipImgBuf.imgTop_G, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_G, nClipWidth, nClipHeight, _T("AfterImgTop_G.bmp"));
		}

	}
	else
		sClipImgBuf.imgTop_G = NULL;
	if (sImgBuf.imgTop_B)
	{
		// 		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgTop_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgTop_B.bmp"));
		// 		sClipImgBuf.imgTop_B = new UCHAR[nClipWidth * nClipHeight];
		// 		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgTop_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, 
		// 			sClipImgBuf.imgTop_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		// 		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_B, nClipWidth, nClipHeight, _T("AfterimgTop_B.bmp"));
		if (g_pMPTI->m_bSideOriginalSize == true)	// 하부 카메라 사용시 칼라 R.G.B 이미지 교체
		{
			m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide1_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgImgTop_Side_B.bmp"));
			//sClipImgBuf.imgTop_B = new UCHAR[nClipWidth * nClipHeight];
			sClipImgBuf.imgTop_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide1_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
				sClipImgBuf.imgTop_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_B, nClipWidth, nClipHeight, _T("AfterImgTop_Side1_B.bmp"));
		}
		else
		{
			m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgTop_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgTop_B.bmp"));
			//sClipImgBuf.imgTop_B = new UCHAR[nClipWidth * nClipHeight];
			sClipImgBuf.imgTop_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgTop_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
				sClipImgBuf.imgTop_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
			m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_B, nClipWidth, nClipHeight, _T("AfterimgTop_B.bmp"));
		}

	}
	else
		sClipImgBuf.imgTop_B = NULL;
	if (sImgBuf.imgTop_W)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgTop_W, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgTop_W.bmp"));
		//sClipImgBuf.imgTop_W = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgTop_W = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgTop_W, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgTop_W, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgTop_W, nClipWidth, nClipHeight, _T("AfterimgTop_W.bmp"));
	}
	else
		sClipImgBuf.imgTop_W = NULL;
	if (sImgBuf.imgMiddle_R)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgMiddle_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgMiddle_R.bmp"));
		//sClipImgBuf.imgMiddle_R = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgMiddle_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgMiddle_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgMiddle_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgMiddle_R, nClipWidth, nClipHeight, _T("AfterimgMiddle_R.bmp"));
	}
	else
		sClipImgBuf.imgMiddle_R = NULL;
	if (sImgBuf.imgMiddle_B)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgMiddle_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgMiddle_B.bmp"));
		//sClipImgBuf.imgMiddle_B = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgMiddle_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgMiddle_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgMiddle_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgMiddle_B, nClipWidth, nClipHeight, _T("AfterimgMiddle_B.bmp"));
	}
	else
		sClipImgBuf.imgBottom_R = NULL;
	if (sImgBuf.imgBottom_R)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgBottom_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_R.bmp"));
		//sClipImgBuf.imgBottom_R = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgBottom_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgBottom_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgBottom_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgBottom_R, nClipWidth, nClipHeight, _T("AfterimgBottom_R.bmp"));
	}
	else
		sClipImgBuf.imgBottom_R = NULL;
	if (sImgBuf.imgBottom_B)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgBottom_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_B.bmp"));
		//sClipImgBuf.imgBottom_B = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgBottom_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgBottom_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgBottom_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgBottom_B, nClipWidth, nClipHeight, _T("AfterimgBottom_B.bmp"));
	}
	else
		sClipImgBuf.imgBottom_B = NULL;

	// Side Camera
	if (sImgBuf.imgSide1_R)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide1_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side1_R.bmp"));
		//sClipImgBuf.imgSide1_R = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide1_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide1_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide1_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide1_R, nClipWidth, nClipHeight, _T("AfterimgBottom_Side1_R.bmp"));
	}
	else
		sClipImgBuf.imgSide1_R = NULL;
	if (sImgBuf.imgSide1_G)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide1_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side1_G.bmp"));
		//sClipImgBuf.imgSide1_G = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide1_G = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide1_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide1_G, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide1_G, nClipWidth, nClipHeight, _T("AfterimgBottom_Side1_G.bmp"));
	}
	else
		sClipImgBuf.imgSide1_G = NULL;
	if (sImgBuf.imgSide1_B)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide1_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side1_B.bmp"));
		//sClipImgBuf.imgSide1_B = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide1_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide1_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide1_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide1_B, nClipWidth, nClipHeight, _T("AfterimgBottom_Side1_B.bmp"));
	}
	else
		sClipImgBuf.imgSide2_B = NULL;

	if (sImgBuf.imgSide2_R)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide2_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side2_R.bmp"));
		//sClipImgBuf.imgSide2_R = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide2_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide2_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide2_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide2_R, nClipWidth, nClipHeight, _T("AfterimgBottom_Side2_R.bmp"));
	}
	else
		sClipImgBuf.imgSide2_R = NULL;
	if (sImgBuf.imgSide2_G)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide2_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side2_G.bmp"));
		//sClipImgBuf.imgSide2_G = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide2_G = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide2_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide2_G, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide2_G, nClipWidth, nClipHeight, _T("AfterimgBottom_Side2_G.bmp"));
	}
	else
		sClipImgBuf.imgSide2_G = NULL;
	if (sImgBuf.imgSide2_B)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide2_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side2_B.bmp"));
		//sClipImgBuf.imgSide2_B = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide2_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide2_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide2_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide2_B, nClipWidth, nClipHeight, _T("AfterimgBottom_Side2_B.bmp"));
	}
	else
		sClipImgBuf.imgSide2_B = NULL;

	if (sImgBuf.imgSide3_R)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide3_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side3_R.bmp"));
		//sClipImgBuf.imgSide3_R = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide3_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide3_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide3_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide3_R, nClipWidth, nClipHeight, _T("AfterimgBottom_Side3_R.bmp"));
	}
	else
		sClipImgBuf.imgSide3_R = NULL;
	if (sImgBuf.imgSide3_G)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide3_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side3_G.bmp"));
		//sClipImgBuf.imgSide3_G = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide3_G = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide3_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide3_G, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide3_G, nClipWidth, nClipHeight, _T("AfterimgBottom_Side3_G.bmp"));
	}
	else
		sClipImgBuf.imgSide3_G = NULL;
	if (sImgBuf.imgSide3_B)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide3_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side3_B.bmp"));
		//sClipImgBuf.imgSide3_B = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide3_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide3_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide3_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide3_B, nClipWidth, nClipHeight, _T("AfterimgBottom_Side3_B.bmp"));
	}
	else
		sClipImgBuf.imgSide3_B = NULL;

	if (sImgBuf.imgSide4_R)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide4_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side4_R.bmp"));
		//sClipImgBuf.imgSide4_R = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide4_R = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide4_R, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide4_R, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide4_R, nClipWidth, nClipHeight, _T("AfterimgBottom_Side4_R.bmp"));
	}
	else
		sClipImgBuf.imgSide4_R = NULL;
	if (sImgBuf.imgSide4_G)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide4_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side4_G.bmp"));
		//sClipImgBuf.imgSide4_G = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide4_G = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide4_G, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide4_G, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide4_G, nClipWidth, nClipHeight, _T("AfterimgBottom_Side4_G.bmp"));
	}
	else
		sClipImgBuf.imgSide4_G = NULL;
	if (sImgBuf.imgSide4_B)
	{
		m_pProcMilAlgo->SaveWorkImg(sImgBuf.imgSide4_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY, _T("OrgimgBottom_Side4_B.bmp"));
		//sClipImgBuf.imgSide4_B = new UCHAR[nClipWidth * nClipHeight];
		sClipImgBuf.imgSide4_B = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetClipImage_LT(sImgBuf.imgSide4_B, sImgBuf.nImageSizeX, sImgBuf.nImageSizeY,
			sClipImgBuf.imgSide4_B, nStartX2D, nStartY2D, nClipWidth, nClipHeight);
		m_pProcMilAlgo->SaveWorkImg(sClipImgBuf.imgSide4_B, nClipWidth, nClipHeight, _T("AfterimgBottom_Side4_B.bmp"));
	}
	else
		sClipImgBuf.imgSide4_B = NULL;

	sClipImgBuf.nImageSizeX = nClipWidth;
	sClipImgBuf.nImageSizeY = nClipHeight;
	bResult = TRUE;
	return bResult;
}
BOOL CPInsp_Algo::GetWndClip_5(WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, const WndInfo &sWndInfo)
{
	BOOL bResult = FALSE;

	double dPartW = sWndAlgoImg.m_nWidth3D;
	double dPartH = sWndAlgoImg.m_nHeight3D;
	double dWndW = sWndAlgoImg.m_nWidth;
	double dWndH = sWndAlgoImg.m_nHeight;
	if ((dPartW <= 0) || (dPartH <= 0) || (dWndW <= 0) || (dWndH <= 0))
		return bResult;
	if (dWndW > dPartW)
		return bResult;
	if (dWndH > dPartH)
		return bResult;

	double dWndCX = sWndInfo.dCenterX / m_resolX;
	double dWndCY = sWndInfo.dCenterY / m_resolY;

	int nWndXStartX = RounD((dPartW / 2.) + (dWndCX)-(dWndW / 2.0));
	int nWndYStartY = RounD((dPartH / 2.) - (dWndCY)-(dWndH / 2.0));
	if (nWndXStartX < 0)
		nWndXStartX = 0;
	if (nWndYStartY < 0)
		nWndYStartY = 0;

	if (sWndAlgoImg.m_fArr3D_part)
	{
		m_pProcMilAlgo->SaveWorkImg_float(sWndAlgoImg.m_fArr3D_part, sWndAlgoImg.m_nWidth3D, sWndAlgoImg.m_nHeight3D, _T("Org3D.bmp"));

		//sWndClipAlgo.m_fArr3D = new float[dWndW * dWndH];
		sWndClipAlgo.m_fArr3D = g_pMManager->pem_new<float>(true, dWndW * dWndH, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D_part, sWndClipAlgo.m_fArr3D, sWndAlgoImg.m_nWidth3D,
			sWndAlgoImg.m_nHeight3D, nWndXStartX, nWndYStartY, dWndW, dWndH);
		m_pProcMilAlgo->SaveWorkImg_float(sWndClipAlgo.m_fArr3D, dWndW, dWndH, _T("After3D.bmp"));
	}

	sWndClipAlgo.m_nWidth = dWndW;
	sWndClipAlgo.m_nHeight = dWndH;
	sWndClipAlgo.m_nWidth3D = dWndW;
	sWndClipAlgo.m_nHeight3D = dWndH;
	bResult = TRUE;

	return bResult;
}
BOOL CPInsp_Algo::TeachAIData(TeachAIParam stParam, AlgoColorOpt &ptrAlgoColorOpt)
{
	BOOL bRet = FALSE;

	if (stParam.m_nWidth <= 5 || stParam.m_nHeight <= 5 || stParam.m_nRstW <= 5 || stParam.m_nRstH <= 5 ||
		stParam.m_nRstL < 0 || stParam.m_nRstT < 0 || stParam.m_fArr3D_part == NULL ||
		stParam.m_nRstL + stParam.m_nRstW > stParam.m_nWidth || stParam.m_nRstT + stParam.m_nRstH > stParam.m_nHeight ||
		ptrAlgoColorOpt.m_rcImageRect.left + stParam.m_nRstW > ptrAlgoColorOpt.m_sFovImg.nImageSizeX ||
		ptrAlgoColorOpt.m_rcImageRect.top + stParam.m_nRstH > ptrAlgoColorOpt.m_sFovImg.nImageSizeY)
		return bRet;

	UCHAR* img_R = ptrAlgoColorOpt.m_sFovImg.imgTop_R;
	UCHAR* img_G = ptrAlgoColorOpt.m_sFovImg.imgTop_G;
	UCHAR* img_B = ptrAlgoColorOpt.m_sFovImg.imgTop_B;
	int nFovW = ptrAlgoColorOpt.m_sFovImg.nImageSizeX;
	int nFovH = ptrAlgoColorOpt.m_sFovImg.nImageSizeY;
	int nRstL = ptrAlgoColorOpt.m_rcImageRect.left;
	int nRstT = ptrAlgoColorOpt.m_rcImageRect.top;
	int nRstW = stParam.m_nRstW;
	int nRstH = stParam.m_nRstH;

	if (img_R == NULL || img_G == NULL || img_B == NULL)
		return bRet;

	m_pProcMilAlgo->SaveWorkImg(img_R, nFovW, nFovH, _T("TeachAIData_ORG_R.bmp"));
	m_pProcMilAlgo->SaveWorkImg(img_G, nFovW, nFovH, _T("TeachAIData_ORG_G.bmp"));
	m_pProcMilAlgo->SaveWorkImg(img_B, nFovW, nFovH, _T("TeachAIData_ORG_B.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(stParam.m_fArr3D_part, stParam.m_nWidth, stParam.m_nHeight, _T("TeachAIData_ORG_3D.bmp"));

	m_pProcMilAlgo->GetClipImage_LT(img_R, nFovW, nFovH, stParam.m_ucImgR, nRstL, nRstT, nRstW, nRstH);
	m_pProcMilAlgo->GetClipImage_LT(img_G, nFovW, nFovH, stParam.m_ucImgG, nRstL, nRstT, nRstW, nRstH);
	m_pProcMilAlgo->GetClipImage_LT(img_B, nFovW, nFovH, stParam.m_ucImgB, nRstL, nRstT, nRstW, nRstH);

	nRstL = stParam.m_nRstL;
	nRstT = stParam.m_nRstT;
	m_proc3d.GetCropZmap_LT(stParam.m_fArr3D_part, stParam.m_fImg3D, stParam.m_nWidth, stParam.m_nHeight, nRstL, nRstT, nRstW, nRstH);

	m_pProcMilAlgo->SaveWorkImg(stParam.m_ucImgR, nRstW, nRstH, _T("TeachAIData_RST_R.bmp"));
	m_pProcMilAlgo->SaveWorkImg(stParam.m_ucImgG, nRstW, nRstH, _T("TeachAIData_RST_G.bmp"));
	m_pProcMilAlgo->SaveWorkImg(stParam.m_ucImgB, nRstW, nRstH, _T("TeachAIData_RST_B.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(stParam.m_fImg3D, nRstW, nRstH, _T("TeachAIData_RST_3D.bmp"));

	return TRUE;
}
RECT CPInsp_Algo::GetWndROI(WndAlgoImg &sWndAlgoImg, const WndInfo &sWndInfo, bool bPartErr)
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

	double dClipWidth = sWndInfo.dWidth / dResolX;
	double dClipHeight = sWndInfo.dLength / dResolY;
	if ((dClipWidth <= 0) || (dClipHeight <= 0))
		return rcWnd;

	float dClipX3D = ((sWndInfo.m_dPartWidth / 2.) + sWndInfo.dCenterX - (sWndInfo.dWidth / 2.)) / dResolX;
	float dClipY3D = ((sWndInfo.m_dPartHeight / 2.) - sWndInfo.dCenterY - (sWndInfo.dLength / 2.)) / dResolY;
	if (bPartErr)
	{
		dClipX3D -= sWndAlgoImg.m_fPartRoundingErrX;
		dClipY3D -= sWndAlgoImg.m_fPartRoundingErrY;
	}

	int nClipWidth = RounD(dClipWidth);
	int nClipHeight = RounD(dClipHeight);
	int nStartX3D = RounD(dClipX3D);
	int nStartY3D = RounD(dClipY3D);

	if (nStartX3D <= 0)
		nStartX3D = 0;
	if (nStartY3D <= 0)
		nStartY3D = 0;

	if (sWndAlgoImg.m_nWidth3D != 0)
	{
		if (nStartX3D + nClipWidth > sWndAlgoImg.m_nWidth3D)
			nStartX3D = sWndAlgoImg.m_nWidth3D - nClipWidth;
	}
	if (sWndAlgoImg.m_nHeight3D != 0)
	{
		if (nStartY3D + dClipHeight > sWndAlgoImg.m_nHeight3D)
			nStartY3D = sWndAlgoImg.m_nHeight3D - dClipHeight;
	}

	rcWnd.left = nStartX3D;
	rcWnd.right = nStartX3D + nClipWidth;
	rcWnd.top = nStartY3D;
	rcWnd.bottom = nStartY3D + nClipHeight;

	return rcWnd;
}
BOOL CPInsp_Algo::ClipBuffer_PTT(float*pZMap, UCHAR* pRED, UCHAR* pGREEN, UCHAR* pBLUE, int nWIdth, int nHeight, int nROILeft, int nROITop, int nROIWidth, int nROILength, UCHAR** pTarget, int& nTargetSize, float fBottom)
{

	//#define IPP_CLIPPBUFFER;
	if (pZMap == NULL || pRED == NULL || pGREEN == NULL || pBLUE == NULL) return FALSE;
	if (nROIWidth <= 0 || nROILength <= 0) return FALSE;
	if (nROILeft < 0 || nROITop < 0) return FALSE;

	if (nROILeft + nROIWidth > nWIdth) return FALSE;
	if (nROITop + nROILength > nHeight) return FALSE;
	if (isnan(fBottom)) return FALSE;

	//Buffer 생성
	int nSizePTR = 32 + (nROIWidth * nROILength * 4);
	int nSizePRI = 44 + (nROIWidth * nROILength * 4);
	int nBufferSize = nSizePTR + nSizePRI;
	*pTarget = new UCHAR[nBufferSize];

#ifdef IPP_CLIPPBUFFER
	IppiSize iSize = { nROIWidth, nROILength };
#endif 

	//PTR
	{
		struct PTR_Header
		{
			UINT NumRow;
			UINT NumColumn;
			float resol_x;
			float resol_y;
			UINT nBit;
			UINT nGap_x;
			UINT nGap_y;
			UINT nUnit;
		};

		PTR_Header* pPTRHeader = (PTR_Header *)*pTarget;
		pPTRHeader->NumRow = nROILength;
		pPTRHeader->NumColumn = nROIWidth;
		pPTRHeader->resol_x = m_resolX * 1000;
		pPTRHeader->resol_y = m_resolY * 1000;
		pPTRHeader->nBit = 32;
		pPTRHeader->nGap_x = 1;
		pPTRHeader->nGap_y = 1;
		pPTRHeader->nUnit = 1;

		//3D Clip
		float* pZmapTargerStart = ((float*)(*pTarget)) + 8;
		float* pZmapSourceStart = pZMap + (nWIdth* nROITop) + nROILeft;

#ifdef IPP_CLIPPBUFFER
		IppStatus sts;
		sts = ippiCopy_32f_C1R(pZmapSourceStart, nWIdth * 4, pZmapTargerStart, nROIWidth * 4, iSize);

		//바닥 적용
		if (fBottom)
			ippiSubC_32f_C1IR(fBottom, pZmapTargerStart, nROIWidth * 4, iSize);
#else

		//Clip
		float* pZmapTargerStartY = pZmapTargerStart;
		float* pZmapSourceStartY = pZmapSourceStart;

		for (int y = 0; y < nROILength; y++)
		{
			for (int x = 0; x < nROIWidth; x++)
			{
				pZmapTargerStartY[x] = pZmapSourceStartY[x] - fBottom;
			}

			pZmapSourceStartY += nWIdth;
			pZmapTargerStartY += nROIWidth;
		}


#endif 
	}

	//PRI
	{
		struct PRI_Header
		{
			FLOAT NumRow;
			FLOAT NumColumn;
			FLOAT st_Col;
			FLOAT st_Row;
			FLOAT shiftX;
			FLOAT shiftY;
			FLOAT width;
			FLOAT length;
			FLOAT resol_x;
			FLOAT resol_y;
			FLOAT DEPTH;
		};

		PRI_Header* pPRIHeader = (PRI_Header *)(*pTarget + nSizePTR);
		pPRIHeader->NumRow = nROILength;
		pPRIHeader->NumColumn = nROIWidth;
		pPRIHeader->st_Col = 0;
		pPRIHeader->st_Row = 0;
		pPRIHeader->shiftX = 0;
		pPRIHeader->shiftY = 0;
		pPRIHeader->width = 0;
		pPRIHeader->length = 0;
		pPRIHeader->resol_x = m_resolX * 1000;
		pPRIHeader->resol_y = m_resolY * 1000;
		pPRIHeader->DEPTH = 32;


#ifdef IPP_CLIPPBUFFER
		Ipp8u* pAlpha = ippsMalloc_8u(nWIdth*nHeight);
		ippsZero_8u(pAlpha, nWIdth*nHeight);

		Ipp8u* pSrc[4]; //4 Channel
		pSrc[0] = pBLUE + (nWIdth* nROITop) + nROILeft;
		pSrc[1] = pGREEN + (nWIdth* nROITop) + nROILeft;
		pSrc[2] = pRED + (nWIdth* nROITop) + nROILeft;
		pSrc[3] = pAlpha;
		UCHAR* pRGB = *pTarget + nSizePTR + 44;
		IppStatus is = ippiCopy_8u_P4C4R(pSrc, nWIdth, pRGB, nROIWidth * 4, iSize);

		ippsFree(pAlpha);
		pAlpha = nullptr;
#else

		UCHAR* pClipBlueStart = pBLUE + (nWIdth* nROITop) + nROILeft;
		UCHAR* pClipGreenStart = pGREEN + (nWIdth* nROITop) + nROILeft;
		UCHAR* pClipRedStart = pRED + (nWIdth* nROITop) + nROILeft;
		UCHAR* pRGB = *pTarget + nSizePTR + 44;

		int index = 0;
		for (int y = 0; y < nROILength; y += 1)
		{
			for (int x = 0; x < nROIWidth; x++)
			{
				pRGB[index++] = pClipBlueStart[x];
				pRGB[index++] = pClipGreenStart[x];
				pRGB[index++] = pClipRedStart[x];
				pRGB[index++] = 0;
			}

			pClipBlueStart += nWIdth;
			pClipGreenStart += nWIdth;
			pClipRedStart += nWIdth;
		}

#endif 


	}
	nTargetSize = nBufferSize;

	return TRUE;
}
BOOL CPInsp_Algo::Get_PseudoColorMap(float * fZmapSrc, UCHAR * ucMask, int width, int length, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	int widthStep = g_pMPTI->nCalcWidthStep(false, width);

	m_pProcMilAlgo->SaveWorkImg(ucMask, widthStep, length, _T("OriginBW.bmp"));
	m_pProcMilAlgo->SaveWorkImg_float(fZmapSrc, width, length, _T("Origin3D.bmp"));

	if (CPInsp::Get_PseudoColorMap(fZmapSrc, ucMask, width, widthStep, length, ucArrDstImg) == 1)
	{
		m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, widthStep, length, _T("PseudoColorMap.bmp"), 3);
		bResult = TRUE;
	}
	return bResult;
}
BOOL CPInsp_Algo::Set_PseudoColorMap(float fMin, float fMax)
{
	BOOL bResult = FALSE;

	if (fMin == fMax)
	{
		fMin = -50.f;
		fMax = 1230.f;
	}
	CPInsp::Set_PseudoResol(fMin, fMax);
	bResult = TRUE;

	return bResult;
}
#pragma endregion

