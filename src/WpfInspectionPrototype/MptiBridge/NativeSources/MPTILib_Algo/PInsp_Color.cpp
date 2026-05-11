#include "StdAfx.h"
#include "MPTI.h"
#include "PInsp_Color.h"
#include "MemoryManager.h"

//#define HISTOIMG_SIZEX   562
//#define HISTOIMG_SIZEY   532

#define HISTOIMG_SIZEX   320
#define HISTOIMG_SIZEY   290


CPInsp_Color::CPInsp_Color(void)
{
	m_className = _T("CPInsp_Color");

	m_procMil = NULL;
	//m_milApp = M_NULL;
	//m_milSys = M_NULL;

	m_pFovImage_insp_R = NULL;
	m_pFovImage_insp_B = NULL;

	m_pFovImage_teach_R = NULL;
	m_pFovImage_teach_B = NULL;

	m_pFovImage_insp_Main = NULL;
	m_pFovImage_teach_Main = NULL;


	m_pTeachParam = &m_paramTeach;
	m_pInspParam = &m_paramInspect;

	m_pAlgoColor = NULL;
	m_eAlgoType = eAlgoColor;

	m_bIsBigPart = false;

	m_pForeignExceptRoiPos = NULL;
	m_pForeignExceptRoiSize = NULL;

	m_pAlgoColorXY = NULL;

	m_fFatorR = 0;
	m_fFatorG = 0;
	m_fFatorB = 0;
	m_fFatorACR = 0;
	m_fFatorACG = 0;
	m_fFatorACB = 0;
	m_fFatorMGR = 0.4f;
	m_fFatorMGB = 0.8f;
	m_fFatorBR = 0;
	m_fFatorBB = 0;
	m_fFatorBGR = 0.4f;
	m_fFatorBGB = 0.8f;

	InitResult(&m_resultClr);

	//이물검사용 버퍼

	m_ucCIETab = NULL;
	m_ucMap = NULL;

	Color_CieViewLoad();
}

CPInsp_Color::~CPInsp_Color(void)
{
	CloseDevice();
}

int CPInsp_Color::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	// Allocate a Application.
	//m_milApp = milApp;
	// Allocate a System.
	//m_milSys = milSys;

	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;	

	//m_procMil = new CProcMil_Color();
		m_procMil = g_pMManager->pem_new<CProcMil_Color>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	//m_procMil->InitMil(m_milApp, m_milSys);
	m_procMil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procMil->SetResol(resolX, resolY, m_fovWidth);
	

	//Tab
	m_ucCIETab = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY * 3, (PCHAR)__FUNCTION__, __LINE__, true);
	m_ucMap = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY, (PCHAR)__FUNCTION__, __LINE__, true);

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength ,resolX, resolY, g_pMPTI->isUseImagePilLib());

	m_procMil->InitMilAlgoBlob();
	Color_CieViewLoad();

	return ePART_SUCCESS;
}

int CPInsp_Color::CloseDevice()
{
	if(m_procMil != NULL)
	{
		m_procMil->FreeMil();
		
		//delete m_procMil;
		g_pMManager->pem_delete(m_procMil, false);
		m_procMil = NULL;
	}

	if(m_dispCIE_org)
	{
		//delete [] m_dispCIE_org;
		g_pMManager->pem_delete(m_dispCIE_org, true);
		m_dispCIE_org = NULL;
	}


	g_pMManager->pem_delete(m_ucCIETab, true);
	m_ucCIETab = NULL;

	g_pMManager->pem_delete(m_ucMap, true);
	m_ucMap = NULL;

	//m_milSys = M_NULL;
	//m_milApp = M_NULL;
	CPInsp::CloseDevice();

	return ePART_SUCCESS;	
}

// 사용 안함
int CPInsp_Color::SetInspParam(void* itemParam, Coordinate cdn, InspImgBuf img, int mainImgIndex)
{
	m_pInspParam = (InspParamColor*) itemParam;
	m_inspCoordinate = cdn;

	m_mainLightIndex = mainImgIndex;

	m_imgBuf[eImg_Top_R] = img.imgTop_R;
	m_imgBuf[eImg_Top_G] = img.imgTop_G;
	m_imgBuf[eImg_Top_B] = img.imgTop_B;

	m_imgBuf[eImg_Middle_R] = img.imgMiddle_R;
	m_imgBuf[eImg_Middle_B] = img.imgMiddle_B;

	m_imgBuf[eImg_Bottom_R] = img.imgBottom_R;
	m_imgBuf[eImg_Bottom_B] = img.imgBottom_B;


	m_pFovImage_insp_Main = m_imgBuf[GetLightIndex(m_mainLightIndex)];

	//임시  나중에 안씀... colorXY전용
	int lightIndex = GetLightIndex(m_mainLightIndex);
	if(lightIndex ==  eImg_Top_R || lightIndex ==  eImg_Top_G || lightIndex ==  eImg_Top_B)
	{
		m_pFovImage_insp_R = m_imgBuf[eImg_Top_R];
		m_pFovImage_insp_B = m_imgBuf[eImg_Top_B];
	}
	else if(lightIndex ==  eImg_Middle_R || lightIndex ==  eImg_Middle_B)
	{
		m_pFovImage_insp_R = m_imgBuf[eImg_Middle_R]; 
		m_pFovImage_insp_B = m_imgBuf[eImg_Middle_B]; 
	}
	else if(lightIndex ==  eImg_Bottom_R || lightIndex ==  eImg_Bottom_B)
	{
		m_pFovImage_insp_R = m_imgBuf[eImg_Bottom_R]; 
		m_pFovImage_insp_B = m_imgBuf[eImg_Bottom_B]; 
	}


	return ePART_SUCCESS;
}

// 사용 안함
int CPInsp_Color::SetInspParam(void* itemParam, void* targetImg_R, void* targetImg_B, void* targetImg_Main, Coordinate cdn)
{
	m_pInspParam = (InspParamColor*) itemParam;
	m_pFovImage_insp_R = targetImg_R;
	m_pFovImage_insp_B = targetImg_B;
	m_pFovImage_insp_Main = targetImg_Main;
	m_inspCoordinate = cdn;
	return ePART_SUCCESS;
}

// 사용 안함
int CPInsp_Color::SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, 
							const AlgoCoordinate &coordinateAlgo, const WndInfo &wndInfoAnyAngle, 
							const InspPartInfo *pInspBoardInfo, int nLeadTipPos )
{
	if(sInspAlgo.m_eAlgoType != eAlgoColor 
		&& sInspAlgo.m_eAlgoType != eAlgoLead_Color)
		return ePART_FAIL;
	if(!m_pInspParam)
		return ePART_FAIL;

	AlgoColor* pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
	if(pAlgoColor)
	{
		m_pAlgoColor = (!pAlgoColor->IsUseMultiArea()) ? (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam : NULL;
	}

	m_pFovImage_insp_Main = NULL;//m_imgBuf[sInspAlgo.GetKindInspImg()];

	m_eAlgoType = sInspAlgo.m_eAlgoType;
	if(m_pAlgoColor)
		m_pAlgoColor->GetParam(m_pInspParam);

	if(pInspBoardInfo)
	{
		m_imgBuf[eImg_Top_R]	= pInspBoardInfo->fovImgBuf.imgTop_R;
		m_imgBuf[eImg_Top_G]	= pInspBoardInfo->fovImgBuf.imgTop_G;
		m_imgBuf[eImg_Top_B]	= pInspBoardInfo->fovImgBuf.imgTop_B;
		m_imgBuf[eImg_Middle_R]	= pInspBoardInfo->fovImgBuf.imgMiddle_R;
		m_imgBuf[eImg_Middle_B]	= pInspBoardInfo->fovImgBuf.imgMiddle_B;
		m_imgBuf[eImg_Bottom_R]	= pInspBoardInfo->fovImgBuf.imgBottom_R;
		m_imgBuf[eImg_Bottom_B]	= pInspBoardInfo->fovImgBuf.imgBottom_B;

		void *pFovImage_R = NULL;
		void *pFovImage_B = NULL;

		if(sInspAlgo.m_eLightType == Top_Light)
		{
			pFovImage_R = m_imgBuf[eImg_Top_R];
			pFovImage_B = m_imgBuf[eImg_Top_G];
		}
		else if(sInspAlgo.m_eLightType == Middle_Light)
		{
			pFovImage_R = m_imgBuf[eImg_Middle_R];
			pFovImage_B = m_imgBuf[eImg_Middle_B];
		}
		else if(sInspAlgo.m_eLightType == Bottom_Light)
		{
			pFovImage_R = m_imgBuf[eImg_Bottom_R];
			pFovImage_B = m_imgBuf[eImg_Bottom_B];
		}

		{
			m_pFovImage_insp_R	= pFovImage_R;
			m_pFovImage_insp_B = pFovImage_B;
		}
	}
	else if(g_pMPTI)
	{
		SetParamChannelImg(sInspAlgo, coordinateAlgo, TRUE);
	}

	m_inspCoordinate.SetParamROI(coordinateAlgo);
	m_inspCoordinate.anyAngleCx = wndInfoAnyAngle.dCenterX;
	m_inspCoordinate.anyAngleCy = wndInfoAnyAngle.dCenterY;
	m_inspCoordinate.anyAngleWidth = wndInfoAnyAngle.dWidth;
	m_inspCoordinate.anyAngleLength = wndInfoAnyAngle.dLength;

	m_wndAlgoImg = sWndAlgoImg;

	if(sInspAlgo.m_eAlgoType == eAlgoLead_Color)
	{
		ClipWndAlgoImgLead(nLeadTipPos);
	}

	return ePART_SUCCESS;
}

// LMJ 2015/04/12	: FOV Image 에서 Clip하여 검사하던 방식 => Part Image 에서 Clip 하여 검사하는 방식으로 변경
// 현재 사용 중인 함수
int CPInsp_Color::SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, 
	const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, int nLeadTipPos, RECT* rcLeadInsp, int nInspCameraType,
	InspRoiImgBuf *sInspImageData, TotalInspExceptArea *stTieArea)
{
	if(sInspAlgo.m_eAlgoType != eAlgoColor 
		&& sInspAlgo.m_eAlgoType != eAlgoLead_Color
		&& sInspAlgo.m_eAlgoType != eAlgoColorXY)
		return ePART_FAIL;
	if(!m_pInspParam)
		return ePART_FAIL;

	AlgoColor* pAlgoColor = (sInspAlgo.m_eAlgoType != eAlgoColorXY) ? (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam : NULL;
	if(pAlgoColor)
	{
		m_pAlgoColor = (!pAlgoColor->IsUseMultiArea()) ? (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam : NULL;
	}
	m_pAlgoColorXY = (sInspAlgo.m_eAlgoType == eAlgoColorXY) ? (AlgoColorXY *)sInspAlgo.m_ptrInspAlgoParam : NULL;

	m_pFovImage_insp_Main = NULL;

	m_eAlgoType = sInspAlgo.m_eAlgoType;

	// ColorXY 안 쓰는 상황에서 m_pAlgoColor 에 쓰레기 값이 있어서 덤프나는 경우가 생김.
	if(sInspAlgo.m_eAlgoType != eAlgoColorXY)
	{
		if(m_pAlgoColor)
			m_pAlgoColor->GetParam(m_pInspParam);
	}
	if((sInspAlgo.m_eAlgoType == eAlgoColor || sInspAlgo.m_eAlgoType == eAlgoLead_Color) &&
		sInspImageData != NULL)
	{
		m_pPartImgBuf[eImg_Top_R] = sInspImageData->imgTop_R;
		m_pPartImgBuf[eImg_Top_G] = sInspImageData->imgTop_G;
		m_pPartImgBuf[eImg_Top_B] = sInspImageData->imgTop_B;
		m_pPartImgBuf[eImg_Top_W] = sInspImageData->imgTop_W;
		m_pPartImgBuf[eImg_Middle_R] = sInspImageData->imgMiddle_R;
		m_pPartImgBuf[eImg_Middle_B] = sInspImageData->imgMiddle_B;
		m_pPartImgBuf[eImg_Bottom_R] = sInspImageData->imgBottom_R;
		m_pPartImgBuf[eImg_Bottom_B] = sInspImageData->imgBottom_B;
		m_nPartWidth = sInspImageData->nImageSizeX;
		m_nPartHeight = sInspImageData->nImageSizeY;
	}
	else if(pInspBoardInfo)
	{
		if(nInspCameraType == eMSCN_COAXIAL)
		{
			m_pPartImgBuf[eImg_Top_R]		= pInspBoardInfo->partImgBuf.imgTop_R;
			m_pPartImgBuf[eImg_Top_G]		= pInspBoardInfo->partImgBuf.imgTop_G;
			m_pPartImgBuf[eImg_Top_B]		= pInspBoardInfo->partImgBuf.imgTop_B;
		}
		else if(nInspCameraType == eMSCN_SIDECAM1)
		{
			m_pPartImgBuf[eImg_Top_R]		= pInspBoardInfo->partImgBuf.imgSide1_R;
			m_pPartImgBuf[eImg_Top_G]		= pInspBoardInfo->partImgBuf.imgSide1_G;
			m_pPartImgBuf[eImg_Top_B]		= pInspBoardInfo->partImgBuf.imgSide1_B;
		}
		else if(nInspCameraType == eMSCN_SIDECAM2)
		{
			m_pPartImgBuf[eImg_Top_R]		= pInspBoardInfo->partImgBuf.imgSide2_R;
			m_pPartImgBuf[eImg_Top_G]		= pInspBoardInfo->partImgBuf.imgSide2_G;
			m_pPartImgBuf[eImg_Top_B]		= pInspBoardInfo->partImgBuf.imgSide2_B;
		}
		else if(nInspCameraType == eMSCN_SIDECAM3)
		{
			m_pPartImgBuf[eImg_Top_R]		= pInspBoardInfo->partImgBuf.imgSide3_R;
			m_pPartImgBuf[eImg_Top_G]		= pInspBoardInfo->partImgBuf.imgSide3_G;
			m_pPartImgBuf[eImg_Top_B]		= pInspBoardInfo->partImgBuf.imgSide3_B;
		}
		else if(nInspCameraType == eMSCN_SIDECAM4)
		{
			m_pPartImgBuf[eImg_Top_R]		= pInspBoardInfo->partImgBuf.imgSide4_R;
			m_pPartImgBuf[eImg_Top_G]		= pInspBoardInfo->partImgBuf.imgSide4_G;
			m_pPartImgBuf[eImg_Top_B]		= pInspBoardInfo->partImgBuf.imgSide4_B;
		}
// 		m_pPartImgBuf[eImg_Top_R]		= pInspBoardInfo->partImgBuf.imgTop_R;
// 		m_pPartImgBuf[eImg_Top_G]		= pInspBoardInfo->partImgBuf.imgTop_G;
// 		m_pPartImgBuf[eImg_Top_B]		= pInspBoardInfo->partImgBuf.imgTop_B;
		m_pPartImgBuf[eImg_Top_W]		= pInspBoardInfo->partImgBuf.imgTop_W;
		m_pPartImgBuf[eImg_Middle_R]	= pInspBoardInfo->partImgBuf.imgMiddle_R;
		m_pPartImgBuf[eImg_Middle_B]	= pInspBoardInfo->partImgBuf.imgMiddle_B;
		m_pPartImgBuf[eImg_Bottom_R]	= pInspBoardInfo->partImgBuf.imgBottom_R;
		m_pPartImgBuf[eImg_Bottom_B]	= pInspBoardInfo->partImgBuf.imgBottom_B;

		m_nPartWidth = pInspBoardInfo->partImgBuf.nImageSizeX;
		m_nPartHeight = pInspBoardInfo->partImgBuf.nImageSizeY;
	}
	if((sInspAlgo.m_eAlgoType == eAlgoColor || sInspAlgo.m_eAlgoType == eAlgoLead_Color) &&
		sInspImageData != NULL)
	{
		m_inspCoordinate.cx = 0;
		m_inspCoordinate.cy = 0;
		m_inspCoordinate.width = m_nPartWidth;
		m_inspCoordinate.length = m_nPartHeight;
		m_inspCoordinate.angle = 0;
	}
	else
	{
		m_inspCoordinate.cx = RounD(coordinateAlgo.dROICenterX - coordinateAlgo.dROIWidth / 2.);
		m_inspCoordinate.cy = RounD(coordinateAlgo.dROICenterY - coordinateAlgo.dROILength / 2.);
		m_inspCoordinate.width = RounD(coordinateAlgo.dROIWidth);
		m_inspCoordinate.length = RounD(coordinateAlgo.dROILength);
		m_inspCoordinate.angle = coordinateAlgo.dROIAngle;
	}

	m_wndAlgoImg = sWndAlgoImg;
	if (stTieArea)
		m_stTieArea.SetData(*stTieArea);

	if(sInspAlgo.m_eAlgoType == eAlgoLead_Color)
	{
		ClipWndAlgoImgLead(nLeadTipPos, rcLeadInsp);
	}

	return ePART_SUCCESS;
}

void CPInsp_Color::ClipWndAlgoImgLead( int nLeadTipPos, RECT* rcLeadInsp)
{
	if(m_pAlgoColor == NULL)
		return;

	if(nLeadTipPos < 0)
		nLeadTipPos = m_pAlgoColor->m_nLeadPosition;
	else
		m_pAlgoColor->m_nLeadPosition = nLeadTipPos;

	int nLeadDir = m_pAlgoColor->m_nLeadTipDirection;
	int nSolderLengthHeight = m_pAlgoColor->m_nSolderLength;
	int nSolderX = 0;
	int nSolderY = 0;
	int nSolderWidth =0;
	int nSolderHeight =0;
	int nSolderScanCount = 0;

	int nImgWidth = m_wndAlgoImg.m_nWidth;
	int nImgHeight = m_wndAlgoImg.m_nHeight;

	if((nLeadDir == 0) || (nLeadDir == 1))
	{
		if(nLeadDir == 0)
		{
			if (nLeadTipPos >= nImgWidth)
				nLeadTipPos = nImgWidth - 1;
			if (nLeadTipPos < 0) nLeadTipPos = 0;

			if(nSolderLengthHeight >= 0)
				nSolderX = (nImgWidth - nLeadTipPos) - nSolderLengthHeight;
			else
			{
				nSolderX = (nImgWidth - nLeadTipPos);
				nSolderLengthHeight *= -1;
			}
		}
		else
		{
			if (nLeadTipPos + nSolderLengthHeight >= nImgWidth)
				nLeadTipPos = nImgWidth - nSolderLengthHeight;
			if (nLeadTipPos < 0) nLeadTipPos = 0;

			nSolderX = nLeadTipPos;
			if(nSolderLengthHeight < 0)
			{
				nSolderX += nSolderLengthHeight;
				nSolderLengthHeight *= -1;
			}
		}
		if(nSolderX < 0)	nSolderX = 0;
		nSolderWidth = nSolderLengthHeight;
		nSolderHeight = nImgHeight;
		nSolderScanCount = nSolderWidth * nSolderHeight;
	}
	else
	{
		if(nLeadDir == 2)
		{
			if(nSolderLengthHeight >= 0)
				nSolderY = (nImgHeight - nLeadTipPos) - nSolderLengthHeight;
			else
			{
				nSolderY = (nImgHeight - nLeadTipPos);
				nSolderLengthHeight *= -1;
			}
		}
		else
		{
			nSolderY = nLeadTipPos;
			if(nSolderLengthHeight < 0)
			{
				nSolderY += nSolderLengthHeight;
				nSolderLengthHeight *= -1;
			}
		}
		if(nSolderY < 0)	nSolderY = 0;
		nSolderWidth = nImgWidth;
		nSolderHeight = nSolderLengthHeight;
		nSolderScanCount = nSolderWidth * nSolderHeight;
	}

	{
		if (nSolderX < 0) nSolderX = 0;
		if (nSolderY < 0) nSolderY = 0;

		if((nSolderX + nSolderWidth) >= nImgWidth)
			nSolderWidth = nImgWidth - nSolderX;

		if((nSolderY + nSolderHeight) >= nImgHeight)
			nSolderHeight = nImgHeight - nSolderY;
	}

	UCHAR *pucImgSrc	= m_wndAlgoImg.m_ucArr2D; 
// 	float *pfImgSrc		= m_wndAlgoImg.m_fArr3D; 
// 	float *pfSolderImg	= NULL;
	UCHAR *pucSolderImg	= NULL;

// 	if(pfImgSrc)
// 	{
// 		pfSolderImg	= new float[nSolderScanCount]; 
// 		m_proc3d.GetCropZmap_LT(pfImgSrc, pfSolderImg, nImgWidth, nImgHeight, nSolderX, nSolderY , nSolderWidth, nSolderHeight);
// 		m_procMil->SaveWorkImg_float(pfSolderImg, nSolderWidth, nSolderHeight, _T("LeadColor3D.bmp"));
// 	}
	if(pucImgSrc)
	{
		//pucSolderImg	= new UCHAR[nSolderScanCount]; 
		pucSolderImg = g_pMManager->pem_new<UCHAR>(true, nSolderScanCount, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipImage_LT(pucImgSrc, nImgWidth, nImgHeight, 
			pucSolderImg, nSolderX, nSolderY, nSolderWidth, nSolderHeight);
		m_procMil->SaveWorkImg(pucSolderImg, nSolderWidth, nSolderHeight, _T("LeadColor2D.bmp"));
		if(rcLeadInsp != NULL)
		{
			rcLeadInsp->left = nSolderX;
			rcLeadInsp->top = nSolderY;
			rcLeadInsp->right = nSolderX + nSolderWidth;
			rcLeadInsp->bottom = nSolderY + nSolderHeight;
		}
		m_inspCoordinate.cx = nSolderX;
		m_inspCoordinate.cy = nSolderY;
		m_inspCoordinate.width = nSolderWidth;
		m_inspCoordinate.length = nSolderHeight;
		m_inspCoordinate.angle = 0;
	}

	m_wndAlgoImg.Destroy();
	m_wndAlgoImg.m_nWidth = m_wndAlgoImg.m_nWidth3D = nSolderWidth;
	m_wndAlgoImg.m_nHeight = m_wndAlgoImg.m_nHeight3D = nSolderHeight;
// 	m_wndAlgoImg.m_fArr3D = pfSolderImg;
	m_wndAlgoImg.m_ucArr2D = pucSolderImg;
}

// 사용하지 않음
int CPInsp_Color::SetTeachParam(void* itemParam, Coordinate cdn, InspImgBuf img, int mainImgIndex)
{
	m_pTeachParam = (TeachParamColor*) itemParam;
	m_teachCoordinate = cdn;

	m_mainLightIndex = mainImgIndex;

	m_imgBuf[eImg_Top_R]	= img.imgTop_R;
	m_imgBuf[eImg_Top_G]	= img.imgTop_G;
	m_imgBuf[eImg_Top_B]	= img.imgTop_B;

	m_imgBuf[eImg_Middle_R] = img.imgMiddle_R;
	m_imgBuf[eImg_Middle_B] = img.imgMiddle_B;

	m_imgBuf[eImg_Bottom_R] = img.imgBottom_R;
	m_imgBuf[eImg_Bottom_B] = img.imgBottom_B;


	m_pFovImage_teach_Main = m_imgBuf[GetLightIndex(m_mainLightIndex)];

	//임시  나중에 안씀... colorXY전용
	int lightIndex = GetLightIndex(m_mainLightIndex);
	if(lightIndex ==  eImg_Top_R || lightIndex ==  eImg_Top_G || lightIndex ==  eImg_Top_B)
	{
		m_pFovImage_teach_R = m_imgBuf[eImg_Top_R]; 
		m_pFovImage_teach_B = m_imgBuf[eImg_Top_B]; 
	}
	else if(lightIndex ==  eImg_Middle_R || lightIndex ==  eImg_Middle_B)
	{
		m_pFovImage_teach_R = m_imgBuf[eImg_Middle_R]; 
		m_pFovImage_teach_B = m_imgBuf[eImg_Middle_B]; 
	}
	else if(lightIndex ==  eImg_Bottom_R || lightIndex ==  eImg_Bottom_B)
	{
		m_pFovImage_teach_R = m_imgBuf[eImg_Bottom_R]; 
		m_pFovImage_teach_B = m_imgBuf[eImg_Bottom_B]; 
	}


	return ePART_SUCCESS;
}

// 사용하지 않음
int CPInsp_Color::SetTeachParam(void* itemParam, void* targetImg_R, void* targetImg_B, void* targetImg_Main, Coordinate cdn, int teachType)
{
	m_pTeachParam = (TeachParamColor*) itemParam;
	m_pFovImage_teach_R = targetImg_R;
	m_pFovImage_teach_B = targetImg_B;
	m_pFovImage_teach_Main = targetImg_Main;
	m_teachCoordinate = cdn;
	return ePART_SUCCESS;
}

#if 0
int CPInsp_Color::SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const WndInfo &wndInfoAnyAngle, int nLeadTipPos )
{
	if(!m_pTeachParam)
		return ePART_FAIL;
	if(sInspAlgo.m_eAlgoType != eAlgoColor && sInspAlgo.m_eAlgoType != eAlgoLead_Color)
		return ePART_FAIL;
	m_pAlgoColor = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pAlgoColor)
		return ePART_FAIL;

	m_eAlgoType = sInspAlgo.m_eAlgoType;
	m_pFovImage_teach_Main = NULL;//m_imgBuf[sInspAlgo.GetKindInspImg()];

	m_wndAlgoImg = sWndAlgoImg;

	m_pAlgoColor->GetParam(m_pTeachParam);

	if(g_pMPTI)
	{
		SetParamChannelImg(sInspAlgo, coordinateAlgo, TRUE);
	}

	m_teachCoordinate.SetParamROI(coordinateAlgo);

	m_teachCoordinate.anyAngleCx = RounD(wndInfoAnyAngle.dCenterX / m_resolX);
	m_teachCoordinate.anyAngleCy = RounD(wndInfoAnyAngle.dCenterY / m_resolY);
	m_teachCoordinate.anyAngleWidth = RounD(wndInfoAnyAngle.dWidth / m_resolX);
	m_teachCoordinate.anyAngleLength = RounD(wndInfoAnyAngle.dLength / m_resolY);

	if(sInspAlgo.m_eAlgoType == eAlgoLead_Color)
	{
		ClipWndAlgoImgLead(nLeadTipPos);
	}

	return ePART_SUCCESS;
}
#endif

// 현재 사용 중인 함수
int CPInsp_Color::SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const WndInfo &wndInfoAnyAngle, TotalInspExceptArea stTieArea, int nLeadTipPos, bool bIsBigPart, bool bIsLoadFovRawDataImage, int projectionmode, int nInspCameraType, int nFovID)
{
	if(!m_pTeachParam)
		return ePART_FAIL;
	if(sInspAlgo.m_eAlgoType != eAlgoColor 
		&& sInspAlgo.m_eAlgoType != eAlgoLead_Color
		&& sInspAlgo.m_eAlgoType != eAlgoTab
		&& sInspAlgo.m_eAlgoType != eAlgoColorXY)
		return ePART_FAIL;

	AlgoColor* pAlgoColor = (sInspAlgo.m_eAlgoType != eAlgoColorXY) ? (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam : NULL;
	if(pAlgoColor)
	{
		m_pAlgoColor = (!pAlgoColor->IsUseMultiArea()) ? (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam : NULL;
	}
	m_pAlgoColorXY = (sInspAlgo.m_eAlgoType == eAlgoColorXY) ? (AlgoColorXY *)sInspAlgo.m_ptrInspAlgoParam : NULL;

	m_bIsBigPart = bIsBigPart;

	m_eAlgoType = sInspAlgo.m_eAlgoType;
	m_pFovImage_teach_Main = NULL;

	m_wndAlgoImg = sWndAlgoImg;

	// ColorXY 안 쓰는 상황에서 m_pAlgoColor 에 쓰레기 값이 있어서 덤프나는 경우가 생김.
	if(sInspAlgo.m_eAlgoType != eAlgoColorXY)
	{
		if(m_pAlgoColor)
			m_pAlgoColor->GetParam(m_pTeachParam);
	}

	if(g_pMPTI)
	{
		SetParamChannelImg(sInspAlgo, coordinateAlgo, TRUE, bIsLoadFovRawDataImage, projectionmode, nInspCameraType, nFovID);
	}

	int window_left = RounD(coordinateAlgo.dROICenterX - coordinateAlgo.dROIWidth / 2.);	// Window 의 좌상단 좌표 (Pixel, Part의 Center 0,0 기준)
	int window_top = RounD(coordinateAlgo.dROICenterY + coordinateAlgo.dROILength / 2.);
	m_teachCoordinate.SetParamROI(coordinateAlgo);
	m_teachCoordinate.cx = window_left;
	m_teachCoordinate.cy = window_top;
	m_teachCoordinate.anyAngleCx = RounD(wndInfoAnyAngle.dCenterX / m_resolX);
	m_teachCoordinate.anyAngleCy = RounD(wndInfoAnyAngle.dCenterY / m_resolY);
	m_teachCoordinate.anyAngleWidth = RounD(wndInfoAnyAngle.dWidth / m_resolX);
	m_teachCoordinate.anyAngleLength = RounD(wndInfoAnyAngle.dLength / m_resolY);
	
	/******************************************
	3. 변경 후
	AlgoCoodi.m_dROICenterX : Window 의 좌상단 좌표 (Pixel, Part의 Center 0,0 기준)
	=> 이 변수는 일반파트인 경우는 사용하지 않음. Big Part 티칭 시에 사용.
	********************************************/
	if(sInspAlgo.m_eAlgoType == eAlgoLead_Color)
	{
		ClipWndAlgoImgLead(nLeadTipPos);
	}

	m_stTieArea.SetData(stTieArea);

	return ePART_SUCCESS;
}

  void CPInsp_Color::SetParamChannelImg( const InspAlgo &sInspAlgo, const AlgoCoordinate &coordinateAlgo, BOOL bTeach/*=TRUE */ , bool bIsLoadFovRawDataImage/*=false*/, int projectionmode, int nInspCameraType, int nFovID)
  {
  	int fovNum = 0;  //single grab
  	int sequence = g_pMPTI->GetSequence(); 
  	if (nFovID >= 0)
  	{
  		fovNum = nFovID;
  		sequence = eMGS_FG;
  	}
  	//////////////////////////////////////////////////////////////////////////
  	CAtlArray<int> nArrTypeBuf;
  	nArrTypeBuf.Add(eImg_Top_R);
  	nArrTypeBuf.Add(eImg_Top_G);
  	nArrTypeBuf.Add(eImg_Top_B);
  	nArrTypeBuf.Add(eImg_Top_W);	// YJS 2016/11/17:그리드 검사
  	nArrTypeBuf.Add(eImg_Middle_R);
  	nArrTypeBuf.Add(eImg_Middle_B);
  	nArrTypeBuf.Add(eImg_Bottom_R);
  	nArrTypeBuf.Add(eImg_Bottom_B);
  	CAtlArray<int> nArrTypeChannel2D;
  	nArrTypeChannel2D.Add(eM2C_TR);
  	nArrTypeChannel2D.Add(eM2C_TG);
  	nArrTypeChannel2D.Add(eM2C_TB);
  	nArrTypeChannel2D.Add(eM2C_TW);	// YJS 2016/11/17:그리드 검사
  	nArrTypeChannel2D.Add(eM2C_MR);
  	nArrTypeChannel2D.Add(eM2C_MB);
  	nArrTypeChannel2D.Add(eM2C_BR);
  	nArrTypeChannel2D.Add(eM2C_BB);
  
  	int nCntImgChannel = MIN(nArrTypeBuf.GetCount(), nArrTypeChannel2D.GetCount());
  
  	int nFovSizeX = 0;
  	int nFovSizeY = 0;
  
  	g_pMPTI->SetSideFovMilAlloc(&nFovSizeX, &nFovSizeY);
  
  	for(int i=0; i < nCntImgChannel; ++i)
  	{
  		if(!m_bIsBigPart && bIsLoadFovRawDataImage == false)
  		{
  			if (nInspCameraType == (int)eMSCN_COAXIAL)
  			{
  				if (g_pMPTI->m_nCameraType == Basler_Color)
  				{
  					if (i == (int)eImg_Top_R)
  					{
  						m_imgBuf[eImg_Top_R] = (void*)g_pMPTI->GetGrabOutputBuf_Color(0, 0, (int)eMSC_RED);
  					}
  					else if (i == (int)eImg_Top_G)
  					{
  						m_imgBuf[eImg_Top_G] = (void*)g_pMPTI->GetGrabOutputBuf_Color(0, 0, (int)eMSC_GREEN);
  					}
  					else if (i == (int)eImg_Top_B)
  					{
  						m_imgBuf[eImg_Top_B] = (void*)g_pMPTI->GetGrabOutputBuf_Color(0, 0, (int)eMSC_BLUE);
  					}
  				}
  				else
  					m_imgBuf[nArrTypeBuf.GetAt(i)] = (void*)g_pMPTI->GetSeqGrabBufID(sequence, fovNum, nArrTypeChannel2D.GetAt(i), projectionmode);	//top red
  			}
			else
			{
// 				if (i == (int)eImg_Top_R && g_pMPTI->m_bUseSideCamera == true)
// 				{
// 					Im::Buf::Put2d(g_pMPTI->m_bufSideDataFov[0], 0, 0, nFovSizeX, nFovSizeY, (void*)g_pMPTI->GetSingleGrabWarpBuf_Side(0, nInspCameraType, (int)eMSC_RED));
// 					m_imgBuf[eImg_Top_R] = (void*)g_pMPTI->m_bufSideDataFov[0];
// 				}
// 				else if (i == (int)eImg_Top_G && g_pMPTI->m_bUseSideCamera == true)
// 				{
// 					Im::Buf::Put2d(g_pMPTI->m_bufSideDataFov[1], 0, 0, nFovSizeX, nFovSizeY, (void*)g_pMPTI->GetSingleGrabWarpBuf_Side(0, nInspCameraType, (int)eMSC_GREEN));
// 					m_imgBuf[eImg_Top_G] = (void*)g_pMPTI->m_bufSideDataFov[1];
// 				}
// 				else if (i == (int)eImg_Top_B && g_pMPTI->m_bUseSideCamera == true)
// 				{
// 					Im::Buf::Put2d(g_pMPTI->m_bufSideDataFov[2], 0, 0, nFovSizeX, nFovSizeY, (void*)g_pMPTI->GetSingleGrabWarpBuf_Side(0, nInspCameraType, (int)eMSC_BLUE));
// 					m_imgBuf[eImg_Top_B] = (void*)g_pMPTI->m_bufSideDataFov[2];
// 				}
// 				else
// 					m_imgBuf[nArrTypeBuf.GetAt(i)] = (void*)g_pMPTI->GetSeqGrabBufID(sequence, fovNum, nArrTypeChannel2D.GetAt(i), projectionmode);
			}
			//(void*)g_pMPTI->GetSingleGrabBuf_Side(0, nCam, false, )
  		}
  		else if(/*!m_bIsBigPart && */bIsLoadFovRawDataImage == true)
  		{
  			// RawDataInsp
  			if (g_pMPTI->m_bUseSideCamera == true &&
  				(nInspCameraType == (int)eMSCN_SIDECAM1 || nInspCameraType == (int)eMSCN_SIDECAM2 ||
  				 nInspCameraType == (int)eMSCN_SIDECAM3 || nInspCameraType == (int)eMSCN_SIDECAM4))
  			{
  				if (i == (int)eImg_Top_R)
  					m_imgBuf[eImg_Top_R] = (void*)g_pMPTI->m_bufRawDataFov_Side[nInspCameraType][0];
  				else if (i == (int)eImg_Top_G)
  					m_imgBuf[eImg_Top_G] = (void*)g_pMPTI->m_bufRawDataFov_Side[nInspCameraType][1];
  				else if (i == (int)eImg_Top_B)
  					m_imgBuf[eImg_Top_B] = (void*)g_pMPTI->m_bufRawDataFov_Side[nInspCameraType][2];
  			}
  			else
  			{
  				if (i == (int)eImg_Top_R)
  					m_imgBuf[eImg_Top_R] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_TR];
  				else if (i == (int)eImg_Top_G)
  					m_imgBuf[eImg_Top_G] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_TG];
  				else if (i == (int)eImg_Top_B)
  					m_imgBuf[eImg_Top_B] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_TB];
  			}
  
  			if (i == (int)eImg_Top_W)
  				m_imgBuf[eImg_Top_W] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_TW];
  			else if (i == (int)eImg_Middle_R)
  				m_imgBuf[eImg_Middle_R] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_MR];
  			else if (i == (int)eImg_Middle_B)
  				m_imgBuf[eImg_Middle_B] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_MB];
  			else if (i == (int)eImg_Bottom_R)
  				m_imgBuf[eImg_Bottom_R] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_BR];
  			else if (i == (int)eImg_Bottom_B)
  				m_imgBuf[eImg_Bottom_B] = (void*)g_pMPTI->m_bufRawDataFov[eM2C_BB];
  
  			if(m_bIsBigPart)
  			{
  				m_nPartWidth = g_pMPTI->GetDigitSizeX();
  				m_nPartHeight = g_pMPTI->GetDigitSizeY();
  				m_pPartImgBuf[eImg_Top_R]		= g_pMPTI->m_parrFovBuf[eM2C_TR];
  				m_pPartImgBuf[eImg_Top_G]		= g_pMPTI->m_parrFovBuf[eM2C_TG];
  				m_pPartImgBuf[eImg_Top_B]		= g_pMPTI->m_parrFovBuf[eM2C_TB];
  				m_pPartImgBuf[eImg_Top_W]		= g_pMPTI->m_parrFovBuf[eM2C_TW];
  				m_pPartImgBuf[eImg_Middle_R]	= g_pMPTI->m_parrFovBuf[eM2C_MR];
  				m_pPartImgBuf[eImg_Middle_B]	= g_pMPTI->m_parrFovBuf[eM2C_MB];
  				m_pPartImgBuf[eImg_Bottom_R]	= g_pMPTI->m_parrFovBuf[eM2C_BR];
  				m_pPartImgBuf[eImg_Bottom_B]	= g_pMPTI->m_parrFovBuf[eM2C_BB];
  			}
  		}
  
  		else
  		{
  			InspImgBuf *pFovImgBufBig = g_pInspMng->GetPtrFovImgBufBig();
  
  			int channelIndex = 0;
  			// FOV Image InspPartInfo 에 넣어놓는다.
  			for(int i=0; i < eM2C_NUM; ++i)
  			{
  				//int channelIndex = g_pMPTI->GetSeqGrabIndex(sequence, 0, i, projectionmode);
  				void*& pImgBuf = pFovImgBufBig->GetPtrImg(i);
  				if(channelIndex == -1)
  				{
  					pImgBuf = NULL;
  				}
//   				else
//   				{
//   					pImgBuf = g_pMPTI->GetMGrab()->GetPtrArrGrabBuf(channelIndex);
//   				}				
  
  				//m_pPartImgBuf[i] = (UCHAR *)pImgBuf;		
  
  				//g_pMPTI->GetMGrab()->GetSizePixelBig(m_nPartWidth, m_nPartHeight);
  			}
  			// Teaching Mode 에서는 FOV 합쳐진 영상으로 사용
  			m_pPartImgBuf[eImg_Top_R]		= (UCHAR *)pFovImgBufBig->imgTop_R;
  			m_pPartImgBuf[eImg_Top_G]		= (UCHAR *)pFovImgBufBig->imgTop_G;
  			m_pPartImgBuf[eImg_Top_B]		= (UCHAR *)pFovImgBufBig->imgTop_B;
  			m_pPartImgBuf[eImg_Top_W]		= (UCHAR *)pFovImgBufBig->imgTop_W;
  			m_pPartImgBuf[eImg_Middle_R]	= (UCHAR *)pFovImgBufBig->imgMiddle_R;
  			m_pPartImgBuf[eImg_Middle_B]	= (UCHAR *)pFovImgBufBig->imgMiddle_B;
  			m_pPartImgBuf[eImg_Bottom_R]	= (UCHAR *)pFovImgBufBig->imgBottom_R;
  			m_pPartImgBuf[eImg_Bottom_B]	= (UCHAR *)pFovImgBufBig->imgBottom_B;
  		}
  	}
  
  // 	m_procMil->SaveWorkImg(m_pPartImgBuf[eImg_Top_G], m_nPartWidth, m_nPartHeight, _T("Green.bmp"));
  // 	m_procMil->SaveWorkImg(m_pPartImgBuf[eImg_Top_B], m_nPartWidth, m_nPartHeight, _T("Blue.bmp"));
  	
  	//////////////////////////////////////////////////////////////////////////
  
  	int nTypeLightRed = eM2C_TR;
  	int nTypeLightBlue = eM2C_TB;
  	if(sInspAlgo.m_eLightType == Middle_Light)
  	{
  		nTypeLightRed = eM2C_MR;
  		nTypeLightBlue = eM2C_MB;
  	}
  	else if(sInspAlgo.m_eLightType == Bottom_Light)
  	{
  		nTypeLightRed = eM2C_BR;
  		nTypeLightBlue = eM2C_BB;
  	}
  
  	void *pFovImage_R = (void*) g_pMPTI->GetSeqGrabBufID(sequence, fovNum, nTypeLightRed, projectionmode);
  	void *pFovImage_B = (void*) g_pMPTI->GetSeqGrabBufID(sequence, fovNum, nTypeLightBlue, projectionmode);
  
  	if(bTeach)
  	{
  		m_pFovImage_teach_R	= pFovImage_R;
  		m_pFovImage_teach_B = pFovImage_B;
  	}
  	else
  	{
  		m_pFovImage_insp_R	= pFovImage_R;
  		m_pFovImage_insp_B = pFovImage_B;
  	}
  
  	return;
  }

int CPInsp_Color::InspProc()
{
	int ret = 0;

	ret = ProcAuto();

	return ret;
}

int CPInsp_Color::ProcAuto()
{
	int ret = e_OK;
	int stepID = 0;
	int maxStepCnts = 1;


	//step1(eCStepID_ANALYSIS_COLOR) 

	if(m_pAlgoColor)
		InitResult(&m_resultClr);
	InitResult(&m_pInspParam->retInspColorResult);
	
	for(int i = 0; i < maxStepCnts; i++)
	{
		ret = ProcStep(i);

		stepID++;

		if(ret != e_OK) 
			break;
	}

	return ret;
}


int CPInsp_Color::ProcStep(int stepID)
{
	int ret = e_OK;

	int colorInspType = m_pInspParam->colorInspType;

	switch(stepID)
	{
	case eCStepID_ANALYSIS_COLOR:
		{
			if(colorInspType == e_COLORXY)
			{
				int nPolygonCnt = m_pInspParam->m_nPolygonCnt;
				if(nPolygonCnt <= 0 || m_pInspParam->m_bUseRangeBar)
					nPolygonCnt = 1;
				for (int n = 0; n < nPolygonCnt; n++)
				{
					ret = AnalysisColorCIE(FALSE, n);
					if(ret == e_OK)
						break;
				}
				//ret = AnalysisColorXY();
			}	
			else if(colorInspType == e_HISTOGRAM)
			{
				ret = AnalysisHistogram();
			}			
		}
		break;
	case eCStepID_ANALYSIS_COLOR_ROTATE:
		{
			BOOL bRotateImg = TRUE;
			if(colorInspType == e_COLORXY)
				ret = AnalysisColorCIE(bRotateImg);
			else if(colorInspType == e_HISTOGRAM)
				ret = AnalysisHistogram(bRotateImg);
		}
		break;
	}

	return ret;
}

// LMJ 2014/01/14	: LUT 사용 안함 (속도 문제)
#if 0
int CPInsp_Color::AnalysisColorCIE()
{
	int ret = e_OK;

	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);
	int ptCnts = m_pInspParam->pointCnts;
	double stdRate = m_pInspParam->stdRate;	
	
	float fitR = m_pInspParam->redFator;
	float fitG = m_pInspParam->greenFator;
	float fitB = m_pInspParam->blueFator;

	int dstSizeX = m_pInspParam->sizeX_CIE;
	int dstSizeY = m_pInspParam->sizeY_CIE;

	void* temp_R = m_imgBuf[eImg_Top_R];
	void* temp_G = m_imgBuf[eImg_Top_G];
	void* temp_B = m_imgBuf[eImg_Top_B];

	//////////////////////////////////////////////////////////////////////////
	int offsetX = CIE_OFFSETX;
	int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);

	POINT pPt[POLYGON_POINT_CNTS];
	RemakePoly(m_pInspParam->polygonPt, POLYGON_POINT_CNTS, offsetX, offsetY, pPt);

	//BYTE* lutData = NULL;
	//lutData = new BYTE[SIZE_CLRX * SIZE_CLRY];
	//MakeLUT_Poly(lutData, SIZE_CLRX, SIZE_CLRY, pPt, ptCnts);

	//////////////////////////////////////////////////////////////////////////
	UCHAR* img_R = NULL;
	UCHAR* img_G = NULL;
	UCHAR* img_B = NULL;

	img_R = new UCHAR[width * length];
	img_G = new UCHAR[width * length];
	img_B = new UCHAR[width * length];
	//m_procMil->SaveWorkImg((Im::PIL_ID)temp_R, _T("imgRed.bmp"));
	//m_procMil->SaveWorkImg((Im::PIL_ID)temp_G, _T("imgGreen.bmp"));
	//m_procMil->SaveWorkImg((Im::PIL_ID)temp_B, _T("imgBlue.bmp"));

	MakeProcImg(temp_R, temp_G, temp_B, img_R, img_G, img_B, m_inspCoordinate);
	//m_procMil->SaveWorkImg(img_R, width, length, _T("imgRed.bmp"));
	//m_procMil->SaveWorkImg(img_G, width, length, _T("imgGreen.bmp"));
	//m_procMil->SaveWorkImg(img_B, width, length, _T("imgBlue.bmp"));

	double rate = 0.0;
	//rate = CompareCIE(img_R, img_G, img_B, fitR, fitG, fitB, lutData, width, length);
	rate = CompareCIE(img_R, img_G, img_B, fitR, fitG, fitB, width, length, pPt, ptCnts);


	//////////////////////////////////////////////////////////////////////////
	ret = ColorInspDecision(rate, stdRate);

	m_result.rstRate = rate;
	m_result.rstTotalRate = rate;
	m_result.isInsp = TRUE;

	//m_result.wndDefectCode = ret;

	//////////////////////////////////////////////////////////////////////////
	delete img_R;
	delete img_G;
	delete img_B;
	//delete lutData;

	return ret;
}
#endif

int CPInsp_Color::AnalysisColorCIE(BOOL bRotateImg, int nSelectPolygon)
{
	int ret = e_NG;
	try
	{
		int width = GetWidthImg(m_inspCoordinate);
		int length = GetHeightImg(m_inspCoordinate);
		if (width < 1 || length < 1)
			return ret;
		int ptCnts = m_pInspParam->pointCnts;
		double stdRate = m_pInspParam->stdRate;	

		float fitR = m_pInspParam->redFator;
		float fitG = m_pInspParam->greenFator;
		float fitB = m_pInspParam->blueFator;
		if(m_pInspParam->m_bUseColorMap2)
		{
			fitR = 1.0f;
			fitG = 1.0f;
			fitB = 1.0f;
		}
		int dstSizeX = m_pInspParam->sizeX_CIE;
		int dstSizeY = m_pInspParam->sizeY_CIE;

		//////////////////////////////////////////////////////////////////////////
		int offsetX = CIE_OFFSETX;
		int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);

		POINT pPt[POLYGON_POINT_CNTS];
		RemakePoly(m_pInspParam->polygonPt[nSelectPolygon], POLYGON_POINT_CNTS, offsetX, offsetY, pPt);

		//BYTE* lutData = NULL;
		//lutData = new BYTE[SIZE_CLRX * SIZE_CLRY];
		//MakeLUT_Poly(lutData, SIZE_CLRX, SIZE_CLRY, pPt, ptCnts);

		//////////////////////////////////////////////////////////////////////////
		/*UCHAR* img_R = new UCHAR[width * length];
		UCHAR* img_G = new UCHAR[width * length];
		UCHAR* img_B = new UCHAR[width * length];
		UCHAR* img_BR = new UCHAR[width * length];
		UCHAR* img_BB = new UCHAR[width * length];*/
		UCHAR* img_R = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_G = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_B = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_BR = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_BB = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);

		m_stTieArea.m_nUsedInspPolygon = 0;
		m_stTieArea.m_nUsedMaskingValue = 0;

		m_procMil->SaveWorkImg(m_pPartImgBuf[eImg_Top_R], m_nPartWidth, m_nPartHeight, _T("partRed.bmp"));
		m_procMil->SaveWorkImg(m_pPartImgBuf[eImg_Top_G], m_nPartWidth, m_nPartHeight, _T("partGreen.bmp"));
		m_procMil->SaveWorkImg(m_pPartImgBuf[eImg_Top_B], m_nPartWidth, m_nPartHeight, _T("partBlue.bmp"));

		int nCntValid = 0;
		double rate = 0;
		int nCompoLightMode = g_pMPTI->GetCompositeLightMode();
		cv::Mat img255(length, width, CV_8UC1, cv::Scalar(255));
		CPInsp::FillOutOfInspAreaCombine(width, length, 0, img255.data, NULL, m_stTieArea);
		double dWhiteA = std::count(img255.data, img255.data + width * length, 255);
		if(nCompoLightMode == 0)
		{
			MakeProcImg(m_pPartImgBuf[eImg_Top_R], m_pPartImgBuf[eImg_Top_G], m_pPartImgBuf[eImg_Top_B], img_R, img_G, img_B, m_inspCoordinate);
			m_procMil->SaveWorkImg(img_R, width, length, _T("windowRed.bmp"));
			m_procMil->SaveWorkImg(img_G, width, length, _T("windowGreen.bmp"));
			m_procMil->SaveWorkImg(img_B, width, length, _T("windowBlue.bmp"));

			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_R, NULL, m_stTieArea);
			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_G, NULL, m_stTieArea);
			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_B, NULL, m_stTieArea);

			if(m_pInspParam->m_bUseRangeBar == TRUE)
			{
				//UCHAR* retDstImg = new UCHAR[width * length];
				UCHAR* retDstImg = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
				memset(retDstImg, 0, width * length * sizeof(UCHAR));
				AlgoColorBase pInspAlgoColor;
				for (int a = 0 ; a < 3 ; a++)
				{
					pInspAlgoColor.m_bUseRGB[a] = m_pInspParam->m_bUseRGB[a];
					pInspAlgoColor.m_byRange[a] = m_pInspParam->m_byRange[a];
					pInspAlgoColor.m_byMin[a] = m_pInspParam->m_byMin[a];
					pInspAlgoColor.m_byMax[a] = m_pInspParam->m_byMax[a];
				}
				GetColorBin(img_R, img_G, img_B, fitR, fitG, fitB, width, length, &pInspAlgoColor, retDstImg, &rate, dWhiteA);
				m_procMil->SaveWorkImg(retDstImg, width, length, _T("RangeDst.bmp"));
				Delete_1DArray(&retDstImg);
			}
			else
				rate = CompareCIE(img_R, img_G, img_B, fitR, fitG, fitB, width, length, pPt, ptCnts, nCntValid, dWhiteA);
		}
		else
		{
			float fitBR = m_pInspParam->redFactorBtm;
			float fitBB = m_pInspParam->blueFactorBtm;
			float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmR;
			float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmG;
			float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmB;

			MakeProcImg(m_pPartImgBuf[eImg_Top_R], m_pPartImgBuf[eImg_Top_G], m_pPartImgBuf[eImg_Top_B], img_R, img_G, img_B, m_inspCoordinate, 
				m_pPartImgBuf[eImg_Bottom_R], m_pPartImgBuf[eImg_Bottom_B],img_BR, img_BB);
			m_procMil->SaveWorkImg(img_BR, width, length, _T("windowBtmRed.bmp"));
			m_procMil->SaveWorkImg(img_BB, width, length, _T("windowBtmBlue.bmp"));

			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_R, NULL, m_stTieArea);
			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_G, NULL, m_stTieArea);
			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_B, NULL, m_stTieArea);
			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_BR, NULL, m_stTieArea);
			CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_BB, NULL, m_stTieArea);

			if(m_pInspParam->m_bUseRangeBar == TRUE)
			{
				//UCHAR* retDstImg = new UCHAR[width * length];
				UCHAR* retDstImg = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
				memset(retDstImg, 0, width * length * sizeof(UCHAR));
				AlgoColorBase pInspAlgoColor;
				for (int a = 0 ; a < 3 ; a++)
				{
					pInspAlgoColor.m_bUseRGB[a] = m_pInspParam->m_bUseRGB[a];
					pInspAlgoColor.m_byRange[a] = m_pInspParam->m_byRange[a];
					pInspAlgoColor.m_byMin[a] = m_pInspParam->m_byMin[a];
					pInspAlgoColor.m_byMax[a] = m_pInspParam->m_byMax[a];
				}
				GetColorBin(img_R, img_G, img_B, fitR, fitG, fitB, width, length, &pInspAlgoColor, retDstImg, &rate, dWhiteA);
				m_procMil->SaveWorkImg(retDstImg, width, length, _T("RangeDst.bmp"));
				Delete_1DArray(&retDstImg);
			}
			else
				rate = CompareCIE_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, width, length, pPt, ptCnts, nCntValid, dWhiteA);
		}
		if(m_pInspParam->m_bInvert) rate = 100.0 - rate;
// 		int nCntValid = 0;
// 		double rate = CompareCIE(img_R, img_G, img_B, fitR, fitG, fitB, width, length, pPt, ptCnts, nCntValid);

		//////////////////////////////////////////////////////////////////////////
		ret = InspDecision(m_pAlgoColor->m_nInspTypeColor, m_pAlgoColor->m_nRangeMode, rate, stdRate);
		
		m_resultClr.rstRate = rate;
		m_resultClr.rstTotalRate = rate;
		m_resultClr.isInsp = TRUE;
		//m_result.wndDefectCode = ret;

		//////////////////////////////////////////////////////////////////////////
		Delete_1DArray(&img_R);
		Delete_1DArray(&img_G);
		Delete_1DArray(&img_B);
		Delete_1DArray(&img_BR);
		Delete_1DArray(&img_BB);
		//delete lutData;
	}
	catch (CMemoryException* e)
	{
		ret = e_NG;
	}
	catch (CFileException* e)
	{
		ret = e_NG;
	}
	catch (CException* e)
	{
		ret = e_NG;
	}

	return ret;
}

int CPInsp_Color::AnalysisColorXY()
{
	int ret = e_OK;

	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int sizeX = m_inspCoordinate.width;
	int sizeY = m_inspCoordinate.length;
	int ptCnts = m_pInspParam->pointCnts;
	POINT* pPt = m_pInspParam->polygonPt[0];
	double stdRate = m_pInspParam->stdRate;	
	void* temp_R = m_pFovImage_insp_R;
	void *temp_B = m_pFovImage_insp_B;

	BYTE* tempLut = NULL;
	UCHAR* colorXY = NULL;

	//tempLut = new BYTE[COLORXY_WIDTH * COLORXY_LENGTH];
	tempLut = g_pMManager->pem_new<BYTE>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(tempLut, 0, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH));

	//colorXY = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	colorXY = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(colorXY, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));


	GetColorxy(temp_R, temp_B, colorXY, cx, cy, sizeX, sizeY);
	MakeLUT_Poly(tempLut, pPt, ptCnts);

	double rate = 0.0;
	int roiArea = sizeX * sizeY ;
	rate = CompareData(colorXY, tempLut, COLORXY_WIDTH, COLORXY_LENGTH, roiArea);
	m_procMil->SaveWorkImg((UCHAR*)tempLut, 256, 256, _T("lut.bmp"));


	//////////////////////////////////////////////////////////////////////////
	ret = InspDecision(m_pAlgoColor->m_nInspTypeColor, m_pAlgoColor->m_nRangeMode, rate, stdRate);

	m_resultClr.rstRate = rate;
	m_resultClr.rstTotalRate = rate;
	m_resultClr.isInsp = TRUE;

	//m_result.wndDefectCode = ret;

	//////////////////////////////////////////////////////////////////////////


	/*delete tempLut;
	delete colorXY;*/
	g_pMManager->pem_delete(tempLut, false);
	g_pMManager->pem_delete(colorXY, false);

	return ret;
}

int CPInsp_Color::AnalysisHistogram(BOOL bRotateImg/*=FALSE*/)
{
	int ret = e_OK;
	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int width = GetWidthImg(m_inspCoordinate);
	int height = GetHeightImg(m_inspCoordinate);
	int rangeMode = m_pInspParam->rangeMode;
	int maxRange = m_pInspParam->maxRange;
	int minRange = m_pInspParam->minRange;
	double stdRate = m_pInspParam->stdRate;
	int stdHistoCnt = m_pInspParam->stdHisroCnt;


	int histo[HISTO_SIZE] = {0,};
	UCHAR* pUcClipBuf = NULL;
	if(m_pFovImage_insp_Main)
	{
		//pUcClipBuf = new UCHAR[width * height];
		pUcClipBuf = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
		MakeProcImg(m_pFovImage_insp_Main, NULL, NULL, pUcClipBuf, NULL, NULL, m_inspCoordinate);
		m_procMil->SaveWorkImg(pUcClipBuf, width, height, _T("HistogramOrg1.bmp"));
		m_procMil->GetHistoValue(pUcClipBuf, width, height, histo);
	}
	else
	{
#if _DEBUG
		cv::Mat img(height, width, CV_8UC1, m_wndAlgoImg.m_ucArr2D);
#endif
		CPInsp::FillOutOfInspAreaCombine(width, height, 0, m_wndAlgoImg.m_ucArr2D, NULL, m_stTieArea);

		m_procMil->SaveWorkImg(m_wndAlgoImg.m_ucArr2D, width, height, _T("HistogramOrg2.bmp"));
		m_procMil->GetHistoValue(m_wndAlgoImg.m_ucArr2D, width, height, histo);
	}

	int dataSum = GetHistogramCount(rangeMode, maxRange, histo, minRange);

	double area = width * height;
	double rate = 0.;	
	double totalRate = 0.;
	totalRate = (dataSum / area) * 100.0;

// 	if(stdHistoCnt > 0)
// 	{
// 		rate = ((double)dataSum / (double)stdHistoCnt) * 100.0;
// 
// 		if(rate > 100)
// 			rate = 100.0;
// 	}	
// 	else
// 	{
// 		rate = 0;
// 	}

	{
		rate = (dataSum / area) * 100.0;
		if( rate > 100 )
			rate = 100.0;
		// CJY - Color 는 Invert가 적영되야하지만 Gray는 적용되면 안되는 컨셉 
		//if (m_pInspParam->m_bInvert) rate = 100.0 - rate;
	}

//////////////////////////////////////////////////////////////////////////

	ret = InspDecision(m_pAlgoColor->m_nInspTypeColor, m_pAlgoColor->m_nRangeMode, rate, stdRate);
	
	m_resultClr.rstRate = rate;
	m_resultClr.rstTotalRate = totalRate;
	m_resultClr.isInsp = TRUE;

	//m_result.wndDefectCode = ret;

//////////////////////////////////////////////////////////////////////////

	if(pUcClipBuf)
	{
		//delete pUcClipBuf;
		g_pMManager->pem_delete(pUcClipBuf, false);
		pUcClipBuf = NULL;
	}

	return ret;
}

// int CPInsp_Color::InspDecision(double dCurRatio, double dStdRatio)
// {
// 	int ret = e_OK;
// 
// 	BOOL isPolarity = m_pInspParam->isPolarity;
// 
// 	BOOL bInspDecision = e_NG;
// 	if(m_pAlgoColor)
// 	{
// 		if(m_pAlgoColor->m_nInspTypeColor == e_COLORXY)
// 		{
// 			if(m_pAlgoColor->m_nRangeMode == eTypeRangeLower)
// 				bInspDecision = (dCurRatio < dStdRatio);
// 			else
// 				bInspDecision = (dCurRatio > dStdRatio);
// 		}
// 		else
// 		{
// 			bInspDecision = (dCurRatio > dStdRatio);
// 		}
// 	}
// 
// 	if(bInspDecision)
// 	{
// 		ret = e_OK;
// 		m_resultClr.ok = TRUE;
// 
// 		m_resultClr.list.colorRateOk = TRUE;
// 		m_resultClr.list.polarityOK = TRUE;
// 	}
// 	else
// 	{
// 		ret = e_NG;
// 		m_resultClr.ok = FALSE;
// 
// 		m_resultClr.list.colorRateOk = FALSE;
// 
// 		if(isPolarity == TRUE)
// 			m_resultClr.list.polarityOK = FALSE;
// 		else
// 			m_resultClr.list.polarityOK = TRUE;
// 	}
// 
// 	return ret;
// }

int CPInsp_Color::InspDecision(int eInspType, int nRangeMode, double dCurRatio, double dStdRatio)
{
	int ret = e_OK;

	BOOL bInspDecision = e_NG;
	if(eInspType == e_COLORXY)
	{
		if(nRangeMode == eTypeRangeLower)
			bInspDecision = (dCurRatio <= dStdRatio);
		else
			bInspDecision = (dCurRatio >= dStdRatio);
	}
	else
	{
		bInspDecision = (dCurRatio >= dStdRatio);
	}

	ret = (bInspDecision) ? e_OK : e_NG;
	
	if(m_pAlgoColor)
	{
		m_resultClr.ok = bInspDecision;

		m_resultClr.list.colorRateOk = bInspDecision;
		m_resultClr.list.polarityOK = bInspDecision;

		if(m_pInspParam)
		{
			BOOL isPolarity = m_pInspParam->isPolarity;
			if(isPolarity == FALSE)
				m_resultClr.list.polarityOK = TRUE;
		}
	}

	return ret;
}

int CPInsp_Color::Color_Teach(InspRoiImgBuf &sInspRoiImg, bool bIsLoadFovRawDataImage, int nSelectIdx,BOOL bSideCamera)
{
	int ret = e_OK;

	int colorInspType = m_pTeachParam->colorInspType;

	switch(colorInspType)
	{
	case e_COLORXY:
		{
			ret = ColorCIETeach(sInspRoiImg, bIsLoadFovRawDataImage, nSelectIdx, bSideCamera);
			//ret = ColorxyTeach();
		}
		break;
	case e_HISTOGRAM:
		{
			ret = HistogramTeach();
		}
		break;
	}

	return ret;
}

int CPInsp_Color::ColorCIETeach(InspRoiImgBuf &sInspRoiImg, bool bIsLoadFovRawDataImage, int nSelectIdx, BOOL bSideCamera)
{
	int ret = e_NG;
	if(!m_pTeachParam)
		return ret;
	if(nSelectIdx < 0 || nSelectIdx >= COLORALGO_POLYGON_CNTS)
		nSelectIdx = 0;

	int width = sInspRoiImg.nImageSizeX;
	int length = sInspRoiImg.nImageSizeY;
	int ptCnts = m_pTeachParam->pointCnts;
	float fitR = m_pTeachParam->redFator;
	float fitG = m_pTeachParam->greenFator;
	float fitB = m_pTeachParam->blueFator;
	float fitBR = m_pTeachParam->redFactorBtm == 0.0 ?    (float)1.0 : m_pTeachParam->redFactorBtm;
	float fitBB = m_pTeachParam->blueFactorBtm == 0.0 ?    (float)1.0 : m_pTeachParam->blueFactorBtm;
	float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmR;
	float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmG;
	float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmB;

	//////////////////////////////////////////////////////////////////////////
	int dstSizeX = m_pTeachParam->sizeX_CIE;
	int dstSizeY = m_pTeachParam->sizeY_CIE;

	int offsetX = CIE_OFFSETX;
	int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);

	bool bMid = (m_pTeachParam->m_byColorLightType == 1);

	POINT pPt[POLYGON_POINT_CNTS];
	RemakePoly(m_pTeachParam->polygonPt[nSelectIdx], POLYGON_POINT_CNTS, offsetX, offsetY, pPt);

	cv::Rect rcLead = GetLeadColorROI(sInspRoiImg);
	cv::Mat img_R;
	cv::Mat img_G;
	cv::Mat img_B;
	cv::Mat img_TR;
	cv::Mat img_TG;
	cv::Mat img_TB;
	cv::Mat img_TW;
	cv::Mat img_MR;
	cv::Mat img_MB;
	cv::Mat img_BR;
	cv::Mat img_BB;
	if (rcLead.width > 0 && rcLead.height > 0)
	{
		width = rcLead.width;
		length = rcLead.height;
		img_R = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_G = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_B = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_TR = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_TG = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_TB = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_TW = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_MR = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_MB = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_BR = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_BB = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		if (sInspRoiImg.imgTop_R) m_procMil->GetClipImage_LT(sInspRoiImg.imgTop_R, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_TR.data, rcLead.x, rcLead.y, width, length);
		if (sInspRoiImg.imgTop_G) m_procMil->GetClipImage_LT(sInspRoiImg.imgTop_G, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_TG.data, rcLead.x, rcLead.y, width, length);
		if (sInspRoiImg.imgTop_B) m_procMil->GetClipImage_LT(sInspRoiImg.imgTop_B, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_TB.data, rcLead.x, rcLead.y, width, length);
		if (sInspRoiImg.imgTop_W) m_procMil->GetClipImage_LT(sInspRoiImg.imgTop_W, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_TW.data, rcLead.x, rcLead.y, width, length);
		if (sInspRoiImg.imgMiddle_R) m_procMil->GetClipImage_LT(sInspRoiImg.imgMiddle_R, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_MR.data, rcLead.x, rcLead.y, width, length);
		if (sInspRoiImg.imgMiddle_B) m_procMil->GetClipImage_LT(sInspRoiImg.imgMiddle_B, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_MB.data, rcLead.x, rcLead.y, width, length);
		if (sInspRoiImg.imgBottom_R) m_procMil->GetClipImage_LT(sInspRoiImg.imgBottom_R, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_BR.data, rcLead.x, rcLead.y, width, length);
		if (sInspRoiImg.imgBottom_B) m_procMil->GetClipImage_LT(sInspRoiImg.imgBottom_B, sInspRoiImg.nImageSizeX, sInspRoiImg.nImageSizeY, img_BB.data, rcLead.x, rcLead.y, width, length);
	}
	else
	{
		img_R = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_G = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_B = cv::Mat(length, width, CV_8UC1, cv::Scalar(0));
		img_TR = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgTop_R);
		img_TG = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgTop_G);
		img_TB = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgTop_B);
		if (!bSideCamera)
		{
			img_TW = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgTop_W);
			img_MR = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgMiddle_R);
			img_MB = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgMiddle_B);
			img_BR = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgBottom_R);
			img_BB = cv::Mat(length, width, CV_8UC1, sInspRoiImg.imgBottom_B);
		}
		
	}

	int nCompoMode = g_pMPTI->GetCompositeLightMode();
	if(m_pTeachParam->m_bUseColorMap2)
	{
		fitR = 1.0f;
		fitG = 1.0f;
		fitB = 1.0f;

		CPInsp_Algo &InsAlgo = g_pInspMng->GetInspAlgo();
		LightTypeBuf sLightBuf;
		sLightBuf.m_pucTRed = img_TR.data;
		sLightBuf.m_pucTGreen = img_TG.data;
		sLightBuf.m_pucTBlue = img_TB.data;
		sLightBuf.m_pucTWhite = img_TW.data;
		sLightBuf.m_pucMRed = img_MR.data;
		sLightBuf.m_pucMBlue = img_MB.data;
		cv::Mat cvImgG = GetGreenImage(sLightBuf.m_pucMRed, sLightBuf.m_pucMBlue, width, length);
		sLightBuf.m_pucMGreen = cvImgG.data;
		sLightBuf.m_pucMWhite = NULL;
		sLightBuf.m_pucBRed = img_BR.data;
		sLightBuf.m_pucBGreen = NULL;
		sLightBuf.m_pucBBlue = img_BB.data;
		sLightBuf.m_pucBWhite = NULL;
		sLightBuf.m_nImgWidth = width;
		sLightBuf.m_nImgHeight = length;
		sLightBuf.m_nROIImgWidth = width;
		sLightBuf.m_nROIImgHeight = length;
		sLightBuf.m_dROIX = 0;
		sLightBuf.m_dROIY = 0;
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
			if(a == 0)
				InsAlgo.ROIImageClaculCompose(sLightBuf, img_R.data);
			else if(a == 1)
				InsAlgo.ROIImageClaculCompose(sLightBuf, img_G.data);
			else if(a == 2)
				InsAlgo.ROIImageClaculCompose(sLightBuf, img_B.data);
		}
		Delete_1DArray(&sLightBuf.m_pnRedValue);
		Delete_1DArray(&sLightBuf.m_pnGreenValue);
		Delete_1DArray(&sLightBuf.m_pnBlueValue);
		Delete_1DArray(&sLightBuf.m_pnWhiteValue);
		Delete_1DArray(&sLightBuf.m_pnPosition);
		Delete_1DArray(&sLightBuf.m_pnCalculation);
	}
	else
	{
		if (bMid)
		{
			if (!bSideCamera)
			{
				memcpy(img_R.data, img_MR.data, sizeof(UCHAR) * width * length);
				memcpy(img_B.data, img_MB.data, sizeof(UCHAR) * width * length);
				img_G = GetGreenImage(img_R.data, img_B.data, width, length);
			}			
		}
		else
		{
			memcpy(img_R.data, img_TR.data, sizeof(UCHAR) * width * length);
			memcpy(img_G.data, img_TG.data, sizeof(UCHAR) * width * length);
			memcpy(img_B.data, img_TB.data, sizeof(UCHAR) * width * length);
		}
	}

	m_procMil->SaveWorkImg(img_R.data, width, length, _T("imgRed2.bmp"));
	m_procMil->SaveWorkImg(img_G.data, width, length, _T("imgGreen2.bmp"));
	m_procMil->SaveWorkImg(img_B.data, width, length, _T("imgBlue2.bmp"));
	m_procMil->SaveWorkImg(img_BR.data, width, length, _T("imgBtmRed2.bmp"));
	m_procMil->SaveWorkImg(img_BB.data, width, length, _T("imgBtmBlue2.bmp"));

	m_stTieArea.m_nUsedInspPolygon = 0;
	m_stTieArea.m_nUsedMaskingValue = 0;
	CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_R.data, NULL, m_stTieArea);
	CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_G.data, NULL, m_stTieArea);
	CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_B.data, NULL, m_stTieArea);
	CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_BR.data, NULL, m_stTieArea);
	CPInsp::FillOutOfInspAreaCombine(width, length, 0, img_BB.data, NULL, m_stTieArea);

	cv::Mat img255(length, width, CV_8UC1, cv::Scalar(255));
	CPInsp::FillOutOfInspAreaCombine(width, length, 0, img255.data, NULL, m_stTieArea);
	double dWhiteA = std::count(img255.data, img255.data + width * length, 255);
	int nCntValid = 0;
	double rate = 0.0;
	if (m_pTeachParam->m_bUseRangeBar == TRUE)
	{
		UCHAR* retDstImg = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
		memset(retDstImg, 0, width * length * sizeof(UCHAR));
		AlgoColorBase pInspAlgoColor;
		for (int a = 0; a < 3; a++)
		{
			pInspAlgoColor.m_bUseRGB[a] = m_pTeachParam->m_bUseRGB[a];
			pInspAlgoColor.m_byRange[a] = m_pTeachParam->m_byRange[a];
			pInspAlgoColor.m_byMin[a] = m_pTeachParam->m_byMin[a];
			pInspAlgoColor.m_byMax[a] = m_pTeachParam->m_byMax[a];
		}
		GetColorBin(img_R.data, img_G.data, img_B.data, fitR, fitG, fitB, width, length, &pInspAlgoColor, retDstImg, &rate, dWhiteA);
		Delete_1DArray(&retDstImg);
	}
	else
	{
		if (nCompoMode == 0)
			rate = CompareCIE(img_R.data, img_G.data, img_B.data, fitR, fitG, fitB, width, length, pPt, ptCnts, nCntValid, dWhiteA);
		else
			rate = CompareCIE_CompoBtm(img_R.data, img_G.data, img_B.data, img_BR.data, img_BB.data, fitR, fitG, fitB, fitBR, fitBB,
				compoBtmR, compoBtmG, compoBtmB, width, length, pPt, ptCnts, nCntValid, dWhiteA);
	}
	if(m_pTeachParam->m_bInvert) rate = 100.0 - rate;
	m_pTeachParam->rstRate = rate;
	if(m_pAlgoColor)
		m_pAlgoColor->m_dRateResult = rate;

	return e_OK;
}
cv::Rect CPInsp_Color::GetLeadColorROI(InspRoiImgBuf &sInspRoiImg)
{
	cv::Rect rc(0, 0, 0, 0);
	if (m_pAlgoColor == NULL || m_eAlgoType != eAlgoLead_Color)
		return rc;

	int nLeadTipPos = m_pAlgoColor->m_nLeadPosition;
	int nLeadDir = m_pAlgoColor->m_nLeadTipDirection;
	int nSolderLengthHeight = m_pAlgoColor->m_nSolderLength;
	int nSolderX = 0;
	int nSolderY = 0;
	int nSolderWidth = 0;
	int nSolderHeight = 0;
	int nSolderScanCount = 0;

	int nImgWidth = sInspRoiImg.nImageSizeX;
	int nImgHeight = sInspRoiImg.nImageSizeY;

	if ((nLeadDir == 0) || (nLeadDir == 1))
	{
		if (nLeadDir == 0)
		{
			if (nSolderLengthHeight >= 0)
				nSolderX = (nImgWidth - nLeadTipPos) - nSolderLengthHeight;
			else
			{
				nSolderX = (nImgWidth - nLeadTipPos);
				nSolderLengthHeight *= -1;
			}
		}
		else
		{
			nSolderX = nLeadTipPos;
			if (nSolderLengthHeight < 0)
			{
				nSolderX += nSolderLengthHeight;
				nSolderLengthHeight *= -1;
			}
		}
		if (nSolderX < 0)	nSolderX = 0;
		nSolderY = 0;
		nSolderWidth = nSolderLengthHeight;
		nSolderHeight = nImgHeight;
		nSolderScanCount = nSolderWidth * nSolderHeight;
	}
	else
	{
		nSolderX = 0;

		if (nLeadDir == 2)
		{
			if (nSolderLengthHeight >= 0)
				nSolderY = (nImgHeight - nLeadTipPos) - nSolderLengthHeight;
			else
			{
				nSolderY = (nImgHeight - nLeadTipPos);
				nSolderLengthHeight *= -1;
			}
		}
		else
		{
			nSolderY = nLeadTipPos;
			if (nSolderLengthHeight < 0)
			{
				nSolderY += nSolderLengthHeight;
				nSolderLengthHeight *= -1;
			}
		}
		if (nSolderY < 0)	nSolderY = 0;
		nSolderWidth = nImgWidth;
		nSolderHeight = nSolderLengthHeight;
		nSolderScanCount = nSolderWidth * nSolderHeight;
	}

	if (nSolderX < 0) nSolderX = 0;
	if (nSolderY < 0) nSolderY = 0;

	if (nSolderX + nSolderWidth > nImgWidth)
		nSolderWidth = nImgWidth - nSolderX;

	if (nSolderY + nSolderHeight > nImgHeight)
		nSolderHeight = nImgHeight - nSolderY;

	if (nSolderWidth < 0) nSolderWidth = 0;
	if (nSolderHeight < 0) nSolderHeight = 0;

	rc.x = nSolderX;
	rc.y = nSolderY;
	rc.width = nSolderWidth;
	rc.height = nSolderHeight;
	return rc;
}
double CPInsp_Color::GetColorRate()
{
	return m_pTeachParam->rstRate;
}
int CPInsp_Color::ColorxyTeach()
{
	int ret = e_OK;

	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = m_teachCoordinate.width;
	int length = m_teachCoordinate.length;
	int ptCnts = m_pTeachParam->pointCnts;
	POINT* pPt = m_pTeachParam->polygonPt[0];

	void* temp_R = m_pFovImage_teach_R;
	void *temp_B = m_pFovImage_teach_B;

	BYTE* tempLut = NULL;
	UCHAR* colorXY = NULL;

	//tempLut = new BYTE[COLORXY_WIDTH * COLORXY_LENGTH];
	tempLut = g_pMManager->pem_new<BYTE>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(tempLut, 0, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH));

	//colorXY = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	colorXY = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(colorXY, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));


	GetColorxy(temp_R, temp_B, colorXY, cx, cy, width, length);
	MakeLUT_Poly(tempLut, pPt, ptCnts);
	m_procMil->SaveWorkImg((UCHAR*)tempLut, 256, 256, _T("lut.bmp"));

	double rate = 0.0;
	int roiArea = width * length ;
	rate = CompareData(colorXY, tempLut, COLORXY_WIDTH, COLORXY_LENGTH, roiArea);

	m_pTeachParam->rstRate = rate;

	/*delete tempLut;
	delete colorXY;*/
	g_pMManager->pem_delete(tempLut, false);
	g_pMManager->pem_delete(colorXY, false);

	return ret;
}

int CPInsp_Color::HistogramTeach()
{
	int ret = e_NG;

	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = GetWidthImg(m_teachCoordinate);
	int height = GetHeightImg(m_teachCoordinate);
	int rangeMode = m_pTeachParam->rangeMode;
	int maxRange = m_pTeachParam->maxRange;
	int minRange = m_pTeachParam->minRange;

	m_stTieArea.m_nUsedInspPolygon = 0;
	m_stTieArea.m_nUsedMaskingValue = 0;

	UCHAR* pUcClipBuf = NULL;
	int histo[HISTO_SIZE] = {0,};
	if(m_pFovImage_teach_Main)
	{
		//pUcClipBuf = new UCHAR[width * height];
		pUcClipBuf = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
		MakeProcImg(m_pFovImage_teach_Main, NULL, NULL, pUcClipBuf, NULL, NULL, m_teachCoordinate);

		m_procMil->SaveWorkImg(pUcClipBuf, width, height, _T("HistogramOrg1.bmp"));
		m_procMil->GetHistoValue(pUcClipBuf, width, height, histo);
	}
	else
	{
#if _DEBUG
		cv::Mat img(height, width, CV_8UC1, m_wndAlgoImg.m_ucArr2D);
#endif
		CPInsp::FillOutOfInspAreaCombine(width, height, 0, m_wndAlgoImg.m_ucArr2D, NULL, m_stTieArea);

		m_procMil->SaveWorkImg(m_wndAlgoImg.m_ucArr2D, width, height, _T("HistogramOrg2.bmp"));
		m_procMil->GetHistoValue(m_wndAlgoImg.m_ucArr2D, width, height, histo);
	}


	int dataSum = GetHistogramCount(rangeMode, maxRange, histo, minRange);

	double area = width * height;
	double rate = 0.;	
	rate = (dataSum / area) * 100.0;
	if (m_pTeachParam->m_bInvert) rate = 100.0 - rate;

	m_pTeachParam->rstRate = rate;
	if(m_pAlgoColor)
		m_pAlgoColor->m_dRateResult = rate;
	m_pTeachParam->rstHisroCnt = (int)dataSum;

	if(pUcClipBuf)
	{
		//delete pUcClipBuf;
		g_pMManager->pem_delete(pUcClipBuf, false);
		pUcClipBuf = NULL;
	}

	return e_OK;
}
int CPInsp_Color::GetHistogramCount( int rangeMode, int maxRange, int *histo, int minRange ) 
{
	int st = 0;
	int ed = 0;
	int dataSum = 0;

	switch(rangeMode)
	{
	case eHISTO_UPPER:
		{
			st = maxRange/*+1*/;
			ed = HISTO_SIZE - 1;
			for(int i = st; i <= ed; i++)
			{
				dataSum += histo[i];
			}
		}
		break;
	case eHISTO_LOWER:
		{
			st = 0;
			ed = maxRange;
			for(int i = st; i <= ed; i++)
			{
				dataSum += histo[i];
			}
		}
		break;
	case eHISTO_IN_RANGE:
		{
			st = minRange;
			ed = maxRange;
			for(int i = st; i <= ed; i++)
			{
				dataSum += histo[i];
			}
		}
		break;
	case eHISTO_OUT_RANGE:
		{
			st = 0;
			ed = minRange;
			for(int i = st; i < ed; i++)
			{
				dataSum += histo[i];
			}

			st = maxRange+1;
			ed = HISTO_SIZE - 1;
			for(int i = st; i <= ed; i++)
			{
				dataSum += histo[i];
			}
		}
		break;
	}
	return dataSum;
}

int CPInsp_Color::Color_GetColorHistogramView(UCHAR* retHistoImg, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg)
{
	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = GetWidthImg(m_teachCoordinate);
	int height = GetHeightImg(m_teachCoordinate);

	if((width <= 0) || (height <= 0))
		return ePART_FAIL;

	int ret = 0;
	int histo[256] = {0,};

	UCHAR* pUcClipBuf = NULL;
	IppiSize roiSize;
	roiSize.width = width;
	roiSize.height = height;
	Ipp64f isum;
	if(m_pFovImage_teach_Main)
	{
		//pUcClipBuf = new UCHAR[width * height];
		pUcClipBuf = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
		MakeProcImg(m_pFovImage_teach_Main, NULL, NULL, pUcClipBuf, NULL, NULL, m_teachCoordinate);
		ippiSum_8u_C1R(pUcClipBuf, width, roiSize, &isum);
		m_procMil->SaveWorkImg(pUcClipBuf, width, height, _T("HistogramOrg1.bmp"));
		m_procMil->GetHistoValue(pUcClipBuf, width, height, histo);
	}
	else
	{
		ippiSum_8u_C1R(m_wndAlgoImg.m_ucArr2D, width, roiSize, &isum);
		m_procMil->SaveWorkImg(m_wndAlgoImg.m_ucArr2D, width, height, _T("HistogramOrg2.bmp"));
		m_procMil->GetHistoValue(m_wndAlgoImg.m_ucArr2D, m_wndAlgoImg.m_nWidth, m_wndAlgoImg.m_nHeight, histo);
	}
	if (nRetRangeAvg != NULL)
		*nRetRangeAvg = (int)(isum / (width * height));

	//histo[0] = 0;

	m_procMil->MakeHistogramImg(histo, 256, retHistoImg);

	int n = 0, nMin = 0, nMax = 0;
	for(n = 0; n < 256; n++)
	{
		if(histo[n] > 0)
		{
			nMin = n;
			break;
		}
	}

	for(n = 255; n >= 0; n--)
	{
		if(histo[n] > 0)
		{
			nMax = n;
			break;
		}
	}
	if(nRetRangeMin)
		*nRetRangeMin = nMin;
	if(nRetRangeMax)
		*nRetRangeMax = nMax;

	if(pUcClipBuf)
	{
		//delete pUcClipBuf;
		g_pMManager->pem_delete(pUcClipBuf, false);
		pUcClipBuf = NULL;
	}

	return ePART_SUCCESS;
}
double CPInsp_Color::CompareCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, BYTE* lut, int width, int length, int &nCntValid)
{
	double ret = 0.0;

	if(srcR == NULL || srcG == NULL || srcB == NULL || lut == NULL || width <= 0 || length <= 0)
		return ret;

	nCntValid = 0;

	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float r = ((float)srcR[i] * factorR);		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)srcG[i] * factorG);		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)srcB[i] * factorB);		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}
			nCntValid += lut[(SIZE_CLRY - ny) * SIZE_CLRX + nx];
	}

	ret = ((nCntValid * 1.0) / ((width * length) * 1.0)) * 100.0 ;

	return ret;
}

// LMJ 2014/01/14	: LUT 사용 안하고 PtInPolygon
double CPInsp_Color::CompareCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, 
								float factorR, float factorG, float factorB, int width, int length, const POINT *pts, int ptNum, int &nCntValid, double dWhiteA)
{
	double ret = 0.0;

	if(srcR == NULL || srcG == NULL || srcB == NULL || /*lut == NULL || */width <= 0 || length <= 0)
		return ret;

	nCntValid = 0;
	POINT tempPt;
	bool rst;

	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float r = ((float)srcR[i] * factorR);		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)srcG[i] * factorG);		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)srcB[i] * factorB);		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}
			tempPt.x = nx;
			tempPt.y = SIZE_CLRY - ny;

		rst = PtInPolygon(tempPt, pts, ptNum);
		if (rst)
			nCntValid++;
	}

	if (dWhiteA > 0 && width * length > dWhiteA)
		ret = ((nCntValid * 1.0) / dWhiteA) * 100.0;
	else
	ret = ((nCntValid * 1.0) / ((width * length) * 1.0)) * 100.0 ;

	return ret;
}

double CPInsp_Color::CompareCIE_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
	float compoBtmR, float compoBtmG, float compoBtmB, int width, int length, const POINT *pts, int ptNum, int &nCntValid, double dWhiteA)
{
	double ret = 0.0;

	if(srcR == NULL || srcG == NULL || srcB == NULL || /*lut == NULL || */width <= 0 || length <= 0 || srcBR == NULL || srcBB == NULL)
		return ret;

	nCntValid = 0;
	POINT tempPt;
	bool rst;

	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float fBR = (float)(srcBR[i] * factorBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
		float fBB = (float)(srcBB[i] * factorBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

		float fBGR = 0.4f;
		float fBGB = 0.8f;

		float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

		float r = ((float)srcR[i] * factorR) + fBR;		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)srcG[i] * factorG) + fBG;		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)srcB[i] * factorB) + fBB;		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}
			tempPt.x = nx;
			tempPt.y = SIZE_CLRY - ny;

			rst = PtInPolygon(tempPt, pts, ptNum);
			if(rst)
				nCntValid++;

// 			TRACE("InspColor cnt:%d, x:%d, y:%d\n", i, tempPt.x, tempPt.y);

			//cnts += lut[(SIZE_CLRY - ny) * SIZE_CLRX + nx];
	}

	if (dWhiteA > 0 && width * length > dWhiteA)
		ret = ((nCntValid * 1.0) / dWhiteA) * 100.0;
	else
	ret = ((nCntValid * 1.0) / ((width * length) * 1.0)) * 100.0 ;

	return ret;
}

bool CPInsp_Color::IsBlack(double dR, double dG, double dB, int nThresholdType, double dMin, double dMax)
{
	bool	bDraw	= true;
	double	dIntensity	= (dR + dG + dB) / 3.0;
	switch(nThresholdType)
	{
	case ThresholdType::eUpper:
		{
			if(dIntensity < dMin)
				bDraw	= false;
		}
		break;
	case ThresholdType::eLower:
		{
			if(dIntensity > dMax)
				bDraw	= false;
		}
		break;
	case ThresholdType::eInner:
		{
			if(dIntensity < dMin || dIntensity > dMax)
				bDraw	= false;
		}
		break;
	case ThresholdType::eOuter:
		{
			if(dIntensity > dMin && dIntensity < dMax)
				bDraw	= false;
		}
		break;
	}
	return	bDraw;
}

POINTF CPInsp_Color::GetCIEPoint(cv::Mat*	pConvertMat, double dRed, double dGreen, double dBlue, int nPixelCount)
{
	POINTF pt;
	pt.x = 0;
	pt.y = 0;
	cv::Mat	pVal = cv::Mat(3, 1, CV_32FC1);
	pVal.at<float>(0, 0) = dRed;
	pVal.at<float>(1, 0) = dGreen;
	pVal.at<float>(2, 0) = dBlue;
	cv::Mat	pXYZ = cv::Mat(3, 1, CV_32FC1);
	cv::Mat dst = (*pConvertMat) * pVal;
	pXYZ = dst;
	cv::Scalar	scSum = cv::sum(pXYZ);
	double	dSum = scSum.val[0];
	if (dSum != 0.0)
	{
		pt.x = (int)((double)pXYZ.at<float>(0, 0) / dSum * (double)nPixelCount); //cvmGet(pXYZ, 0, 0)
		pt.y = (int)((double)pXYZ.at<float>(1, 0) / dSum * (double)nPixelCount);  //cvmGet(pXYZ, 1, 0)
	}
	pVal.release();
	pXYZ.release();
	return	pt;
}
// LMJ 2014/01/08	: 제외 높이 인자 추가
double CPInsp_Color::CompareCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, BYTE* lut, int width, int length, float *zmap, float limitHeight)
{
	double ret = 0.0;

	if(srcR == NULL || srcG == NULL || srcB == NULL || lut == NULL || width <= 0 || length <= 0)
		return ret;

	int cnts = 0, total_cnt = 0;

	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		if(zmap[i] >= limitHeight)
		{
			total_cnt++;
			continue;
		}

		float r = ((float)srcR[i] * factorR);		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)srcG[i] * factorG);		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)srcB[i] * factorB);		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}
		cnts += lut[(SIZE_CLRY - ny) * SIZE_CLRX + nx];
		total_cnt++;
	}

	if(total_cnt > 0)
		ret = ((cnts * 1.0) / (total_cnt * 1.0)) * 100.0 ;
	
	return ret;
}

double CPInsp_Color::CompareData(UCHAR* colorXY, BYTE* lutData, int xyWidth, int xyLength, int roiArea)
{
	int cnts = 0;
	UCHAR* colorTemp = NULL;
	double ret = 0.0;

	int data = 0;
	colorTemp = colorXY;
	for(int y = 0; y < xyLength; y++)
	{
		for(int x = 0; x < xyWidth; x++)
		{
			data = *colorTemp++;
			if(data > 0)
			{
				for(int i = 0; i < data; i++)
				{
					cnts += lutData[(y * xyWidth) + x];
				}
			}
		}
	}

	ret = ((cnts * 1.0) / (roiArea * 1.0)) * 100.0 ;

	return ret;
}

int CPInsp_Color::GetColorxy(void* srcR, void* srcB, UCHAR* dst, int cx, int cy, int sizeX, int sizeY)
{
	if(sizeX < 1 || sizeY < 1)
		return ePART_FAIL;

	void* temp_R = NULL;
	void* temp_B = NULL;
	UCHAR* img_R = NULL;
	UCHAR* img_B = NULL;

	//img_R = new UCHAR[sizeX * sizeY];
	img_R = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(img_R, 0, (sizeof(UCHAR) * sizeX * sizeY));

	//img_B = new UCHAR[sizeX * sizeY];
	img_B = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(img_B, 0, (sizeof(UCHAR) * sizeX * sizeY));

	m_procMil->GetPtrClipBuf(srcR, img_R, cx, cy, sizeX, sizeY);
	m_procMil->GetPtrClipBuf(srcB, img_B, cx, cy, sizeX, sizeY);

	m_procMil->SaveWorkImg(img_R, sizeX, sizeY, _T("color_clip_red.bmp"));
	m_procMil->SaveWorkImg(img_B, sizeX, sizeY, _T("color_clip_blue.bmp"));
	ImageToColorxy(img_R, img_B, dst, NULL, sizeX, sizeY, eCOLORXY_PROC);

	m_procMil->SaveWorkImg(dst, 256, 256, _T("color_xy.bmp"));

	/*delete img_R;
	delete img_B;*/
	g_pMManager->pem_delete(img_R, false);
	g_pMManager->pem_delete(img_B, false);
	return ePART_SUCCESS;
}

int CPInsp_Color::GetColorCIE(void* srcR, void* srcG, void* srcB, float fitR, float fitG, float fitB, UCHAR* dst, int cx, int cy, int sizeX, int sizeY)
{
	if(sizeX < 1 || sizeY < 1)
		return ePART_FAIL;

	UCHAR* img_R = NULL;
	UCHAR* img_G = NULL;
	UCHAR* img_B = NULL;

	/*img_R = new UCHAR[sizeX * sizeY];
	img_G = new UCHAR[sizeX * sizeY];
	img_B = new UCHAR[sizeX * sizeY];*/
	img_R = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	img_G = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	img_B = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);

	m_procMil->GetPtrClipBuf(srcR, img_R, cx, cy, sizeX, sizeY);
	m_procMil->GetPtrClipBuf(srcG, img_G, cx, cy, sizeX, sizeY);
	m_procMil->GetPtrClipBuf(srcB, img_B, cx, cy, sizeX, sizeY);

	m_procMil->SaveWorkImg(img_R, sizeX, sizeY, _T("color_clip_red.bmp"));
	m_procMil->SaveWorkImg(img_G, sizeX, sizeY, _T("color_clip_green.bmp"));
	m_procMil->SaveWorkImg(img_B, sizeX, sizeY, _T("color_clip_blue.bmp"));
	ImageToColorCIE(img_R, img_G, img_B, fitR, fitG, fitB, dst, sizeX, sizeY);

	m_procMil->SaveWorkImg(dst, SIZE_CLRX, SIZE_CLRY, _T("color_CIE.bmp"));

	/*delete img_R;
	delete img_G;
	delete img_B;*/
	g_pMManager->pem_delete(img_R, false);
	g_pMManager->pem_delete(img_G, false);
	g_pMManager->pem_delete(img_B, false);
	return ePART_SUCCESS;
}

void CPInsp_Color::Color_CieViewLoad()
{
	//display
	//m_dispCIE_org = new UCHAR[SIZE_CLRX * SIZE_CLRY * 3];
	m_dispCIE_org = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY * 3, (PCHAR)__FUNCTION__, __LINE__);

	// drawing background
	for(int i = 0; i < (SIZE_CLRX * SIZE_CLRY * 3); i++)
		m_dispCIE_org[i] = 255;
	float alpha_blend = 1.0f;
	for(int r=0 ; r<256 ; r++)
	{
		for(int g=0 ; g<256 ; g++)
		{
			for(int b=0 ; b<256 ; b++)
			{
				float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
				float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
				float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
				float sumXYZ = cx + cy + cz;

				if (sumXYZ)
				{
					int nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX + 0.5f), 0, (SIZE_CLRX - 1));
					int ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY + 0.5f), 0, (SIZE_CLRY - 1));

					//if ((nx >= 235) && (nx < (235+256)) && (ny >= 205) && (ny < (205+256)))
					{
						m_dispCIE_org[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 2)] = _limit_value((UCHAR)((float)r * alpha_blend), 0, 255);
						m_dispCIE_org[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 1)] = _limit_value((UCHAR)((float)g * alpha_blend), 0, 255);
						m_dispCIE_org[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 0)] = _limit_value((UCHAR)((float)b * alpha_blend), 0, 255);
					}
				}
			}		
		}
	}
}
int  CPInsp_Color::Color_GetColorCIEView(UCHAR* retCIEImg, int dstCieSizeX, int dstCieSizeY, UCHAR* retImg, int dstSizeX, int dstSizeY, POINT* pt, bool bIsLoadFovRawDataImage, InspRoiImgBuf *sInspRoiImg)
{
	part_result resInspClr = ePART_FAIL;
	try
	{
		if(sInspRoiImg->nImageSizeX < 1 || sInspRoiImg->nImageSizeY < 1)
			return ePART_FAIL;
		if(m_dispCIE_org == nullptr)
			return ePART_FAIL;
		int nImgWidth = sInspRoiImg->nImageSizeX;
		int nImgHeight = sInspRoiImg->nImageSizeY;
		float fitR = m_pTeachParam->redFator == 0.0 ?    (float)1.0 : m_pTeachParam->redFator;
		float fitG = m_pTeachParam->greenFator == 0.0 ?    (float)1.0 : m_pTeachParam->greenFator;
		float fitB = m_pTeachParam->blueFator == 0.0 ?    (float)1.0 : m_pTeachParam->blueFator;
		float fitBR = m_pTeachParam->redFactorBtm == 0.0 ?    (float)1.0 : m_pTeachParam->redFactorBtm;
		float fitBB = m_pTeachParam->blueFactorBtm == 0.0 ?    (float)1.0 : m_pTeachParam->blueFactorBtm;
		float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmB;
		if(sInspRoiImg->imgSide1_B!=NULL || sInspRoiImg->imgSide2_B!=NULL || sInspRoiImg->imgSide3_B!=NULL || sInspRoiImg->imgSide4_B!=NULL || m_pTeachParam->m_bUseColorMap2)
		{
			fitR = 1;
			fitG = 1;
			fitB = 1;
		}
		int nCompoMode = g_pMPTI->GetCompositeLightMode();
		int offsetX = CIE_OFFSETX;
		int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
		POINT pPt[POLYGON_POINT_CNTS];
		RemakePoly(pt, POLYGON_POINT_CNTS, offsetX, offsetY, pPt);
		
		/*UCHAR* dispCIE_org = new UCHAR[SIZE_CLRX * SIZE_CLRY * 3];
		UCHAR* colorCIE_org = new UCHAR[SIZE_CLRX * SIZE_CLRY];
		UCHAR* dstTemp = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* img_R = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* img_G = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* img_B = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* img_BR = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* img_BB = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* colorCIE_Clip = new UCHAR[dstCieSizeX * dstCieSizeY];
		UCHAR* blobImg = new UCHAR[dstCieSizeX * dstCieSizeY];
		UCHAR* dispCIE = new UCHAR[dstCieSizeX * dstCieSizeY * 3];*/
		UCHAR* dispCIE_org = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY * 3, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* colorCIE_org = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* dstTemp = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_R = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_G = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_B = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_BR = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_BB = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* colorCIE_Clip = g_pMManager->pem_new<UCHAR>(true, dstCieSizeX * dstCieSizeY, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* blobImg = g_pMManager->pem_new<UCHAR>(true, dstCieSizeX * dstCieSizeY, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* dispCIE = g_pMManager->pem_new<UCHAR>(true, dstCieSizeX * dstCieSizeY * 3, (PCHAR)__FUNCTION__, __LINE__);
		memset(colorCIE_Clip, 0, sizeof(UCHAR) * dstCieSizeX * dstCieSizeY);
		memset(blobImg, 0, sizeof(UCHAR) * dstCieSizeX * dstCieSizeY);
		memset(colorCIE_org, 0, sizeof(UCHAR) * SIZE_CLRX * SIZE_CLRY);
		memset(dstTemp, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		memset(img_R, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		memset(img_G, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		memset(img_B, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		memset(img_BR, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		memset(img_BB, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		memcpy(dispCIE_org, m_dispCIE_org, sizeof(UCHAR)* SIZE_CLRX * SIZE_CLRY * 3);
		if(sInspRoiImg->imgBottom_R != NULL)
			memcpy(img_BR, sInspRoiImg->imgBottom_R, sizeof(UCHAR)* nImgWidth * nImgHeight);
		if(sInspRoiImg->imgBottom_B != NULL)
			memcpy(img_BB, sInspRoiImg->imgBottom_B, sizeof(UCHAR)* nImgWidth * nImgHeight);
		cv::Mat cvImgG;
		if(m_pTeachParam->m_bUseColorMap2)
		{
			CPInsp_Algo &InsAlgo = g_pInspMng->GetInspAlgo();
			LightTypeBuf sLightBuf;
			sLightBuf.m_pucTRed = sInspRoiImg->imgTop_R;
			sLightBuf.m_pucTGreen = sInspRoiImg->imgTop_G;
			sLightBuf.m_pucTBlue = sInspRoiImg->imgTop_B;
			sLightBuf.m_pucTWhite = sInspRoiImg->imgTop_W;
			sLightBuf.m_pucMRed = sInspRoiImg->imgMiddle_R;
			sLightBuf.m_pucMBlue = sInspRoiImg->imgMiddle_B;
			cvImgG = GetGreenImage(sLightBuf.m_pucMRed, sLightBuf.m_pucMBlue, nImgWidth, nImgHeight);
			sLightBuf.m_pucMGreen = cvImgG.data;
			sLightBuf.m_pucMWhite = NULL;
			sLightBuf.m_pucBRed = sInspRoiImg->imgBottom_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = sInspRoiImg->imgBottom_B;
			sLightBuf.m_pucBWhite = NULL;
			sLightBuf.m_nImgWidth = nImgWidth;
			sLightBuf.m_nImgHeight = nImgHeight;
			sLightBuf.m_nROIImgWidth = nImgWidth;
			sLightBuf.m_nROIImgHeight = nImgHeight;
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
				if(a == 0)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_R);
				else if(a == 1)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_G);
				else if(a == 2)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_B);
			}
			Delete_1DArray(&sLightBuf.m_pnRedValue);
			Delete_1DArray(&sLightBuf.m_pnGreenValue);
			Delete_1DArray(&sLightBuf.m_pnBlueValue);
			Delete_1DArray(&sLightBuf.m_pnWhiteValue);
			Delete_1DArray(&sLightBuf.m_pnPosition);
			Delete_1DArray(&sLightBuf.m_pnCalculation);
		}
		else
		{
			if (m_pTeachParam->m_byColorLightType == 1 && sInspRoiImg->imgMiddle_R != NULL && sInspRoiImg->imgMiddle_B != NULL)
			{
				memcpy(img_R, sInspRoiImg->imgMiddle_R, sizeof(UCHAR)* nImgWidth * nImgHeight);
				memcpy(img_B, sInspRoiImg->imgMiddle_B, sizeof(UCHAR)* nImgWidth * nImgHeight);
				cvImgG = GetGreenImage(img_R, img_B, nImgWidth, nImgHeight);
				memcpy(img_G, cvImgG.data, sizeof(UCHAR)* nImgWidth * nImgHeight);
			}
			else
			{
				if (sInspRoiImg->imgTop_R != NULL && sInspRoiImg->imgTop_G != NULL && sInspRoiImg->imgTop_B != NULL)
				{
					memcpy(img_R, sInspRoiImg->imgTop_R, sizeof(UCHAR)* nImgWidth * nImgHeight);
					memcpy(img_G, sInspRoiImg->imgTop_G, sizeof(UCHAR)* nImgWidth * nImgHeight);
					memcpy(img_B, sInspRoiImg->imgTop_B, sizeof(UCHAR)* nImgWidth * nImgHeight);
				}
			}
		}
		m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("imgRed2.bmp"));
		m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("imgGreen2.bmp"));
		m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("imgBlue2.bmp"));
		m_procMil->SaveWorkImg(img_BR, nImgWidth, nImgHeight, _T("imgBtnRed2.bmp"));
		m_procMil->SaveWorkImg(img_BB, nImgWidth, nImgHeight, _T("imgBtnBlue2.bmp"));
		//////////////////////////////////////////////////////////////////////////
		if(nCompoMode == 0)
			ImageToColorCIE(img_R, img_G, img_B, fitR, fitG, fitB, colorCIE_org, nImgWidth, nImgHeight);
		else
			ImageToColorCIE_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, colorCIE_org, nImgWidth, nImgHeight);

		m_procMil->SaveWorkImg(colorCIE_org, SIZE_CLRX, SIZE_CLRY, _T("testCIE.bmp"));

		//display
		if(nCompoMode == 0)
			ImageToColorCIE_Color(img_R, img_G, img_B, fitR, fitG, fitB, dispCIE_org, nImgWidth, nImgHeight);
		else
			ImageToColorCIE_Color_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, dispCIE_org, nImgWidth, nImgHeight);

		m_procMil->SaveWorkImg(dispCIE_org, SIZE_CLRX, SIZE_CLRY, _T("testCIE3.bmp"), 3);

		int ptCnts = m_pTeachParam->pointCnts;

		if(nCompoMode == 0)
			GetSelectColorCIEImg(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, pPt, ptCnts, dstTemp);
		else
			GetSelectColorCIEImg_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB,
			nImgWidth, nImgHeight, pPt, ptCnts, dstTemp);

		//////////////////////////////////////////////////////////////////////////
		// Final Clip
		int cieCx = (int)(dstCieSizeX / 2.0);
		int cieCy = (int)(dstCieSizeY / 2.0);
		m_procMil->GetClipImage(colorCIE_org, SIZE_CLRX, SIZE_CLRY, colorCIE_Clip, cieCx, cieCy, dstCieSizeX, dstCieSizeY);
		m_procMil->SaveWorkImg(colorCIE_Clip, dstCieSizeX, dstCieSizeY, _T("testCIE2.bmp"));
		m_procMil->ImageClip_CIE(dispCIE_org, SIZE_CLRX, SIZE_CLRY, 3, dispCIE, cieCx, cieCy, dstCieSizeX, dstCieSizeY);
		if(retCIEImg != NULL)
			memcpy_s(retCIEImg, sizeof(UCHAR) * dstCieSizeX * dstCieSizeY * 3, dispCIE, sizeof(UCHAR) * dstCieSizeX * dstCieSizeY * 3);
		//auto polygon
		m_procMil->MakeBlobImg(colorCIE_Clip, blobImg, dstCieSizeX, dstCieSizeY, 1);
		m_procMil->SaveWorkImg(blobImg, dstCieSizeX, dstCieSizeY, _T("blobImg.bmp"));
		m_procMil->GetPolygonCoordi(blobImg, dstCieSizeX, dstCieSizeY, 12, pt); //auto polygon margin
		RemakePoly(pt, POLYGON_POINT_CNTS, -offsetX, -offsetY, pt); //Origin좌표로 돌린다.
		// DST
		double factor = m_procMil->GetResizeFactor(dstSizeX, dstSizeY, nImgWidth, nImgHeight);
		m_procMil->ResizeImg(dstTemp, retImg, nImgWidth, nImgHeight, dstSizeX, dstSizeY, 1, factor);
		//////////////////////////////////////////////////////////////////////////
		Delete_1DArray(&dispCIE_org);
		Delete_1DArray(&colorCIE_org);
		Delete_1DArray(&dstTemp);
		Delete_1DArray(&img_R);
		Delete_1DArray(&img_G);
		Delete_1DArray(&img_B);
		Delete_1DArray(&img_BR);
		Delete_1DArray(&img_BB);
		Delete_1DArray(&colorCIE_Clip);
		Delete_1DArray(&blobImg);
		Delete_1DArray(&dispCIE);
		resInspClr = ePART_SUCCESS;
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
	return resInspClr;
}

#define	LMTI(i)	(i < 0.f) ? 0.f : ((i > 255.f) ? 255.f : i)
#define MPTT(r,g,b,rf,gf,bf)	( ((int)(LMTI((float)r*rf)) << 16) | ((int)(LMTI((float)g*gf)) << 8) | ((int)(LMTI((float)b*bf))) )
int  CPInsp_Color::Color_GetColorxyView(UCHAR* retColorxy, UCHAR* retImg, int dstSizeX, int dstSizeY, POINT* pt)
{
	//BYTE* lutData = NULL;
	UCHAR* img_R = NULL;
	UCHAR* img_B = NULL;
	UCHAR* tempBuf = NULL;
	UCHAR* tempBuf2 = NULL;
	UCHAR* blobImg = NULL;
	//UCHAR* remakeColorXY = NULL;

	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int sizeX = m_teachCoordinate.width;
	int sizeY = m_teachCoordinate.length;
	int color = m_pTeachParam->viewColor;

	if(sizeX < 1 || sizeY < 1)
		return ePART_FAIL;


	//img_R = new UCHAR[sizeX * sizeY];
	img_R = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(img_R, 0, (sizeof(UCHAR) * sizeX * sizeY));

	//img_B = new UCHAR[sizeX * sizeY];
	img_B = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(img_B, 0, (sizeof(UCHAR) * sizeX * sizeY));

// 	lutData = new BYTE[COLORXY_WIDTH * COLORXY_LENGTH];
// 	memset(lutData, 0, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH));

	//tempBuf = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	tempBuf = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(tempBuf, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	//tempBuf2 = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	tempBuf2 = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(tempBuf2, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	//blobImg = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	blobImg = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(blobImg, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	//remakeColorXY = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	//memset(remakeColorXY, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	

	m_procMil->GetPtrClipBuf(m_pFovImage_teach_R, img_R, cx, cy, sizeX, sizeY);
	m_procMil->GetPtrClipBuf(m_pFovImage_teach_B, img_B, cx, cy, sizeX, sizeY);

	ImageToColorxy(img_R, img_B, tempBuf, tempBuf2, sizeX, sizeY, eCOLORXY_VIEW);

	double tempColor = _GetColor(color);
	m_procMil->CvtGrayToColor(tempBuf2, retColorxy, COLORXY_WIDTH, COLORXY_LENGTH, tempColor);

	m_procMil->MakeBlobImg(tempBuf, blobImg, COLORXY_WIDTH, COLORXY_LENGTH, 1);
	m_procMil->GetPolygonCoordi(blobImg, COLORXY_WIDTH, COLORXY_LENGTH, 0, pt);
	

	//////////////////////////////////////////////////////////////////////////
	//UCHAR* dstTemp = new UCHAR[sizeX * sizeY];
	UCHAR* dstTemp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(dstTemp, 0, (sizeof(UCHAR) * sizeX * sizeY));

	GetSelectColorImg(dstTemp, pt, POLYGON_POINT_CNTS);

	double factor = m_procMil->GetResizeFactor(dstSizeX, dstSizeY, sizeX, sizeY);
	m_procMil->ResizeImg(dstTemp, retImg, sizeX, sizeY, dstSizeX, dstSizeY, 1, factor);

	//delete dstTemp;
	g_pMManager->pem_delete(dstTemp, false);
	//////////////////////////////////////////////////////////////////////////

	/*delete img_R;
	delete img_B;*/
	g_pMManager->pem_delete(img_R, false);
	g_pMManager->pem_delete(img_B, false);
	//delete lutData;

	/*delete tempBuf;
	delete tempBuf2;
	delete blobImg;*/
	g_pMManager->pem_delete(tempBuf, false);
	g_pMManager->pem_delete(tempBuf2, false);
	g_pMManager->pem_delete(blobImg, false);
	//delete remakeColorXY;

	return ePART_SUCCESS;
}

int CPInsp_Color::Color_GetSelectHistogramView(UCHAR* retImg, int sizeX, int sizeY)
{
	int ret = ePART_SUCCESS;
	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = GetWidthImg(m_teachCoordinate);
	int height = GetHeightImg(m_teachCoordinate);
	int rangeMode = m_pTeachParam->rangeMode;
	int maxRange = m_pTeachParam->maxRange;
	int minRange = m_pTeachParam->minRange;

	UCHAR* pUcClipBuf = NULL;
	if(m_pFovImage_teach_Main)
	{
		//pUcClipBuf = new UCHAR[width * height];
		pUcClipBuf = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
		//memset(clipBuf, 0, sizeof(UCHAR) * width * height);
		MakeProcImg(m_pFovImage_teach_Main, NULL, NULL, pUcClipBuf, NULL, NULL, m_teachCoordinate);
		ret =  GetSelectHistoImg(pUcClipBuf, width, height, retImg, sizeX, sizeY, rangeMode, maxRange, minRange);
	}
	else
	{
		//m_procMil->SaveWorkImg(m_wndAlgoImg.m_ucArr2D, width, height, _T("HistogramOrg.bmp"));
		ret =  GetSelectHistoImg(m_wndAlgoImg.m_ucArr2D, width, height, retImg, sizeX, sizeY, rangeMode, maxRange, minRange);
		//m_procMil->SaveWorkImg(retImg, sizeX, sizeY, _T("HistogramRet.bmp"));
	}

	if(pUcClipBuf)
	{
		//delete pUcClipBuf;
		g_pMManager->pem_delete(pUcClipBuf, false);
		pUcClipBuf = NULL;
	}

	return ret;
}
int CPInsp_Color::GetSelectHistoImg(UCHAR* srcImg, int srcSizeX, int srcSizeY, UCHAR* retDstImg, int dstSizeX, int dstSizeY, int rangeMode, int maxRange, int minRange)
{
	UCHAR* dstTemp = NULL;
	//dstTemp = new UCHAR[srcSizeX * srcSizeY];
	dstTemp = g_pMManager->pem_new<UCHAR>(true, srcSizeX * srcSizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(dstTemp, 0, (sizeof(UCHAR) * srcSizeX * srcSizeY));

	UCHAR data = 0;
	BOOL flag = FALSE;
	int index = 0;
	for(int y = 0; y < srcSizeY; y++)
	{
		for(int x = 0; x < srcSizeX; x++)
		{
			index = (srcSizeX * y) + x;
			data = srcImg[index];
			flag = DataAnalysis(data, maxRange, minRange, rangeMode);

			if(flag)
				dstTemp[index] = data;
			else 
				dstTemp[index] = 0;
		}
	}

	double factor = m_procMil->GetResizeFactor(dstSizeX, dstSizeY, srcSizeX, srcSizeY);
	m_procMil->ResizeImg(dstTemp, retDstImg, srcSizeX, srcSizeY, dstSizeX, dstSizeY, 1, factor);


	//delete dstTemp;
	g_pMManager->pem_delete(dstTemp, false);

	return ePART_SUCCESS;
}

BOOL CPInsp_Color::DataAnalysis(UCHAR data, int maxData, int minData, int mode)
{
	BOOL ret = TRUE;
	int temp = (int)data;

	switch(mode)
	{
	case eHISTO_UPPER:
		{
			if(temp > maxData)
				ret = TRUE;
			else
				ret = FALSE;
		}
		break;
	case eHISTO_LOWER:
		{
			if(temp < maxData)
				ret = TRUE;
			else
				ret = FALSE;
		}
		break;
	case eHISTO_IN_RANGE:
		{
			if(temp < maxData && temp > minData)
				ret = TRUE;
			else
				ret = FALSE;
		}
		break;
	case eHISTO_OUT_RANGE:
		{
			if(temp > maxData || temp < minData)
				ret = TRUE;
			else
				ret = FALSE;
		}
		break;
	}

	return  ret;
}

int CPInsp_Color::Color_GetSelectColorView(UCHAR* retImg, int sizeX, int sizeY)
{
	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = GetWidthImg(m_teachCoordinate);
	int length = GetHeightImg(m_teachCoordinate);
	int ptCnts = m_pTeachParam->pointCnts;
	float fitR = m_pTeachParam->redFator;
	float fitG = m_pTeachParam->greenFator;
	float fitB = m_pTeachParam->blueFator;

	int dstSizeX = m_pTeachParam->sizeX_CIE;
	int dstSizeY = m_pTeachParam->sizeY_CIE;
	if(m_pTeachParam->m_bUseColorMap2)
	{
// 		nR = m_byLightR;
// 		nG = m_byLightG;
// 		nB = m_byLightB;
	}
	void* temp_R = m_imgBuf[eImg_Top_R];
	void* temp_G = m_imgBuf[eImg_Top_G];
	void* temp_B = m_imgBuf[eImg_Top_B];
	if(m_bIsBigPart)
	{
		temp_R = g_pInspMng->GetPtrFovImgBufBig()->imgTop_R;
		temp_G = g_pInspMng->GetPtrFovImgBufBig()->imgTop_G;
		temp_B = g_pInspMng->GetPtrFovImgBufBig()->imgTop_B;
	}

//////////////////////////////////////////////////////////////////////////
	//UCHAR* dstTemp = new UCHAR[width * length];
	UCHAR* dstTemp = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	memset(dstTemp, 0, (sizeof(UCHAR) * width * length));

	int offsetX = CIE_OFFSETX;
	int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);

	POINT pPt[POLYGON_POINT_CNTS];
	RemakePoly(m_pTeachParam->polygonPt[0], POLYGON_POINT_CNTS, offsetX, offsetY, pPt);
//////////////////////////////////////////////////////////////////////////

	UCHAR* img_R = NULL;
	UCHAR* img_G = NULL;
	UCHAR* img_B = NULL;

	/*img_R = new UCHAR[width * length];
	img_G = new UCHAR[width * length];
	img_B = new UCHAR[width * length];*/
	img_R = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	img_G = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	img_B = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);

	if(!m_bIsBigPart)
		MakeProcImg(temp_R, temp_G, temp_B, img_R, img_G, img_B, m_teachCoordinate);
	else
		MakeProcImg_Big_Teach((UCHAR*)temp_R, (UCHAR*)temp_G, (UCHAR*)temp_B, img_R, img_G, img_B, m_teachCoordinate);

	GetSelectColorCIEImg(img_R, img_G, img_B, fitR, fitG, fitB, width, length, pPt, ptCnts, dstTemp);

	m_procMil->SaveWorkImg(dstTemp, width, length, _T("CLR_Result.bmp"));


	double factor = m_procMil->GetResizeFactor(sizeX, sizeY, width, length);
	m_procMil->ResizeImg(dstTemp, retImg, width, length, sizeX, sizeY, 1, factor);

	/*delete img_R;
	delete img_G;
	delete img_B;
	delete dstTemp;*/
	g_pMManager->pem_delete(img_R, false);
	g_pMManager->pem_delete(img_G, false);
	g_pMManager->pem_delete(img_B, false);
	g_pMManager->pem_delete(dstTemp, false);

	return ePART_SUCCESS;
}


int CPInsp_Color::GetSelectColorCIEImg(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, POINT* pt, int ptCnts, UCHAR* retImg)
{
	if(srcR == NULL ||srcG == NULL ||srcB == NULL ||retImg == NULL || sizeX < 1 || sizeY < 1)
		return ePART_FAIL;
	CPoint tempPt;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{
			int index = (y * sizeX) + x;
			float r = ((float)srcR[index] * fitR);		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)srcG[index] * fitG);		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)srcB[index] * fitB);		b = _limit_value(b, 0.f, 255.f);
			float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
			float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
			float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
			float luminance = r + g + b;
			float sumXYZ = cx + cy + cz;
			// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
			int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
			int ny = SIZE_CLRY / 3;
			if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
			{
				nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
				ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
			}
			tempPt.x = nx;
			tempPt.y = (SIZE_CLRY - ny);
			retImg[index] = (PtInPolygon(tempPt, pt, ptCnts) == true) ? 255 : 0;
		}
	}

	return ePART_SUCCESS;
}
int CPInsp_Color::GetSelectColorFromXYZ(POINTF ptfLocation, int* arrRGB)
{
	int nx = (int)ptfLocation.x * 3;
	int ny = (int)ptfLocation.y * SIZE_CLRX * 3;
	int idxR = _limit_value(ny + nx + 2, 0, SIZE_CLRX * SIZE_CLRY * 3 - 1);
	int idxG = _limit_value(ny + nx + 1, 0, SIZE_CLRX * SIZE_CLRY * 3 - 1);
	int idxB = _limit_value(ny + nx + 0, 0, SIZE_CLRX * SIZE_CLRY * 3 - 1);
	arrRGB[0] = m_dispCIE_org[idxR];
	arrRGB[1] = m_dispCIE_org[idxG];
	arrRGB[2] = m_dispCIE_org[idxB];
	return ePART_SUCCESS;
}

int CPInsp_Color::GetSelectColorCIEImg_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float fitR, float fitG, float fitB, float fitBR, float fitBB, 
	float compoBtmR, float compoBtmG, float compoBtmB, int sizeX, int sizeY, POINT* pt, int ptCnts, UCHAR* retImg)
{
	if(srcR == NULL ||srcG == NULL ||srcB == NULL ||retImg == NULL || sizeX < 1 || sizeY < 1 || srcBR == NULL || srcBB == NULL)
		return ePART_FAIL;


	CPoint tempPt;
	int index = 0;
	BOOL rst = FALSE;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{
			index = (y * sizeX) + x;

			float fBR = (float)(srcBR[index] * fitBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
			float fBB = (float)(srcBB[index] * fitBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

			float fBGR = 0.4f;
			float fBGB = 0.8f;

			float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

			float r = ((float)srcR[index] * fitR) + fBR;		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)srcG[index] * fitG) + fBG;		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)srcB[index] * fitB) + fBB;		b = _limit_value(b, 0.f, 255.f);
			float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
			float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
			float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
			float luminance = r + g + b;
			float sumXYZ = cx + cy + cz;
			// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
			int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
			int ny = SIZE_CLRY / 3;
			if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
			{
				nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
				ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));		
			}
			tempPt.x = nx;
			tempPt.y = (SIZE_CLRY - ny);
			rst = PtInPolygon(tempPt, pt, ptCnts);
			if(rst)
			{
				retImg[index] = 255;
			}
		}
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::GetSelectColorImg(UCHAR* retImg, POINT* pt, int ptCnts)
{
	UCHAR* colorXY = NULL;
	UCHAR* remakeColorXY = NULL;
	UCHAR* img_R = NULL;
	UCHAR* img_B = NULL;

	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int sizeX = m_teachCoordinate.width;
	int sizeY = m_teachCoordinate.length;

	if(sizeX < 1 || sizeY < 1)
		return ePART_FAIL;

	//img_R = new UCHAR[sizeX * sizeY];
	img_R = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(img_R, 0, (sizeof(UCHAR) * sizeX * sizeY));

	//img_B = new UCHAR[sizeX * sizeY];
	img_B = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(img_B, 0, (sizeof(UCHAR) * sizeX * sizeY));

	//colorXY = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	colorXY = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(colorXY, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	//remakeColorXY = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	remakeColorXY = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(remakeColorXY, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	m_procMil->GetPtrClipBuf(m_pFovImage_teach_R, img_R, cx, cy, sizeX, sizeY);
	m_procMil->GetPtrClipBuf(m_pFovImage_teach_B, img_B, cx, cy, sizeX, sizeY);


	ImageToColorxy(img_R, img_B, colorXY, NULL, sizeX, sizeY, eCOLORXY_VIEW);
	RemakeColorxy(colorXY, remakeColorXY, pt, POLYGON_POINT_CNTS);
	ColorxyToImage(remakeColorXY, img_R, img_B, retImg, sizeX, sizeY);


	/*delete img_R;
	delete img_B;
	delete colorXY;
	delete remakeColorXY;*/
	g_pMManager->pem_delete(img_R, false);
	g_pMManager->pem_delete(img_B, false);
	g_pMManager->pem_delete(colorXY, false);
	g_pMManager->pem_delete(remakeColorXY, false);

	return ePART_SUCCESS;
}

int CPInsp_Color::ImageToColorxy(UCHAR* img_R, UCHAR* img_B, UCHAR* dst, UCHAR* dst2, int width, int length, int mode)
{
	UCHAR* pSrcTemp_R = NULL;
	UCHAR* pSrcTemp_B = NULL;
	UCHAR* tempBuf = NULL;
	UCHAR* tempBuf2 = NULL;
	int red = 0;
	int blue = 0;
	int blue2= 0;

	//tempBuf = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	tempBuf = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	if(dst2 != NULL)
	{
		//tempBuf2 = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
		tempBuf2 = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
		memset(tempBuf2, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));
	}


	pSrcTemp_R = img_R;
	pSrcTemp_B = img_B;

	for(int y = 0; y < length; y++) //Axis_Y = color blue
	{
		for(int x = 0; x < width; x++) //Axis_X = color red
		{
			red = *pSrcTemp_R++;
			blue = *pSrcTemp_B++;

			//원점이 좌하 일경우 (현재는 좌상 즉, 픽셀좌표...)
			// blue = 256 - blue;  //???

			if(mode == eCOLORXY_PROC)
			{
				tempBuf[(blue * COLORXY_WIDTH) + red]++;
				//tempBuf[(blue * COLORXY_WIDTH) + red] = 1;
			}
			else if(mode == eCOLORXY_VIEW)
			{
				
				tempBuf[(blue * COLORXY_WIDTH) + red] = 255;

				if(dst2 != NULL)
				{
					blue2 = COLORXY_LENGTH - blue;
					tempBuf2[(blue2 * COLORXY_WIDTH) + red] = 255;
				}
			}
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH),
		tempBuf, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	if(dst2 != NULL)
	{
		memcpy_s(dst2, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH),
			tempBuf2, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));
	}


	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);

	if(tempBuf2 != NULL)
		//delete tempBuf2;
		g_pMManager->pem_delete(tempBuf2, false);

	return ePART_SUCCESS;
}

int CPInsp_Color::ImageToColorCIE(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, int width, int length)
{
	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float r = ((float)img_R[i] * factorR);		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)img_G[i] * factorG);		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)img_B[i] * factorB);		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}
		// 1th - extract res area using color
		dst[(SIZE_CLRY - ny) * SIZE_CLRX + nx] = 255;
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::ImageToColorCIE_CompoBtm(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB, float factorR, float factorG, float factorB, 
	float factorBR, float factorBB, float compoBtmR, float compoBtmG, float compoBtmB, UCHAR* dst, int width, int length)
{
	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float fBR = (float)(img_BR[i] * factorBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
		float fBB = (float)(img_BB[i] * factorBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

		float fBGR = 0.4f;
		float fBGB = 0.8f;

		float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

		float r = ((float)img_R[i] * factorR) + fBR;		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)img_G[i] * factorG) + fBG;		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)img_B[i] * factorB) + fBB;		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}
			// 1th - extract res area using color
			dst[(SIZE_CLRY - ny) * SIZE_CLRX + nx] = 255;
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::ImageToColorCIE_Rect(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, RECT rcRoi, int widthStep)
{
	int nH = rcRoi.bottom;
	int nW = rcRoi.right;

	for(int i = rcRoi.top; i < nH; i = i+10)
	{
		for(int j = rcRoi.left; j < nW; j = j+10)
		{
			float r = ((float)img_R[i*widthStep+j] * factorR);		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)img_G[i*widthStep+j] * factorG);		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)img_B[i*widthStep+j] * factorB);		b = _limit_value(b, 0.f, 255.f);
			float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
			float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
			float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
			float luminance = r + g + b;
			float sumXYZ = cx + cy + cz;
			// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
			int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
			int ny = SIZE_CLRY / 3;
			if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
			{
				nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
				ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
			}
			// 1th - extract res area using color
			dst[(SIZE_CLRY - ny) * SIZE_CLRX + nx] = 255;
		}
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::ImageToColorCIE_Color(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, int width, int length)
{
	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float r = ((float)img_R[i] * factorR);		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)img_G[i] * factorG);		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)img_B[i] * factorB);		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}

		// fill black
		dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 2)] = 0;
		dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 1)] = 0;
		dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 0)] = 0;
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::ImageToColorCIE_Color_CompoBtm(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
	float compoBtmR, float compoBtmG, float compoBtmB, UCHAR* dst, int width, int length)
{
	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float fBR = (float)(img_BR[i] * factorBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
		float fBB = (float)(img_BB[i] * factorBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

		float fBGR = 0.4f;
		float fBGB = 0.8f;

		float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

		float r = ((float)img_R[i] * factorR) + fBR;		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)img_G[i] * factorG) + fBG;		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)img_B[i] * factorB) + fBB;		b = _limit_value(b, 0.f, 255.f);
		float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
		float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
		float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;
		// 검정색일 경우 중심 좌표로 강제 지정 (예: 흰색과 같은 좌표)
		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		}

		// fill black
		dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 2)] = 0;
		dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 1)] = 0;
		dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 0)] = 0;
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::ColorCIEToImage(UCHAR* colorXY, UCHAR* img_R, UCHAR* img_G , UCHAR* img_B, UCHAR* dst, int width, int length)
{
	if(width < 1 || length < 1)
		return ePART_FAIL;

	UCHAR* pSrcTemp_R = NULL;
	UCHAR* pSrcTemp_B = NULL;
	UCHAR* pDstTemp = NULL;
	UCHAR* tempBuf = NULL;
	int red = 0;
	int blue = 0;

	//tempBuf = new UCHAR[width * length];
	tempBuf = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(UCHAR) * width * length));

	pSrcTemp_R = img_R;
	pSrcTemp_B = img_B;
	pDstTemp = tempBuf;

	for(int y = 0; y < length; y++) 
	{
		for(int x = 0; x < width; x++) 
		{
			red = *pSrcTemp_R++;
			blue = *pSrcTemp_B++;

			if(colorXY[(blue * COLORXY_WIDTH) + red] > 0)
			{
				*pDstTemp++ = 255;
			}
			else
			{
				*pDstTemp++ = 0;
			}
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * width * length),
		tempBuf, (sizeof(UCHAR) * width * length));

	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);

	return ePART_SUCCESS;
}

int CPInsp_Color::ColorxyToImage(UCHAR* colorXY, UCHAR* img_R, UCHAR* img_B, UCHAR* dst, int width, int length)
{
	if(width < 1 || length < 1)
		return ePART_FAIL;

	UCHAR* pSrcTemp_R = NULL;
	UCHAR* pSrcTemp_B = NULL;
	UCHAR* pDstTemp = NULL;
	UCHAR* tempBuf = NULL;
	int red = 0;
	int blue = 0;

	//tempBuf = new UCHAR[width * length];
	tempBuf = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(UCHAR) * width * length));

	pSrcTemp_R = img_R;
	pSrcTemp_B = img_B;
	pDstTemp = tempBuf;

	for(int y = 0; y < length; y++) 
	{
		for(int x = 0; x < width; x++) 
		{
			red = *pSrcTemp_R++;
			blue = *pSrcTemp_B++;

			if(colorXY[(blue * COLORXY_WIDTH) + red] > 0)
			{
				*pDstTemp++ = 255;
			}
			else
			{
				*pDstTemp++ = 0;
			}
		}
	}
	
	memcpy_s(dst, (sizeof(UCHAR) * width * length),
		tempBuf, (sizeof(UCHAR) * width * length));

	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);

	return ePART_SUCCESS;
}

int CPInsp_Color::RemakeColorCIE(UCHAR* src, UCHAR* dst, const POINT *pts, int ptNum)
{
	UCHAR* pSrcTemp = NULL;
	UCHAR* pDstTemp = NULL;
	UCHAR* tempBuf = NULL;
	int red = 0;
	int blue = 0;

	//tempBuf = new UCHAR[SIZE_CLRX * SIZE_CLRY];
	tempBuf = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(UCHAR) * SIZE_CLRX * SIZE_CLRY));

	POINT tempPt;
	BOOL rst = FALSE;
	int data = 0 ;

	pSrcTemp = src;
	pDstTemp = tempBuf;
	for(int y = 0; y < SIZE_CLRY; y++)
	{
		for(int x = 0; x < SIZE_CLRX; x++)
		{
			data = *pSrcTemp++;
			if(data > 0)
			{
				tempPt.x = x;
				tempPt.y = y;
				rst = PtInPolygon(tempPt, pts, ptNum);

				if(rst)
					*pDstTemp++ = 255;
				else
					*pDstTemp++ = 0;

			}
			else
			{
				*pDstTemp++ = 0;
			}
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * SIZE_CLRX * SIZE_CLRY),
		tempBuf, (sizeof(UCHAR) * SIZE_CLRX * SIZE_CLRY));


	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);

	return ePART_SUCCESS;
}

int CPInsp_Color::RemakeColorxy(UCHAR* src, UCHAR* dst, const POINT *pts, int ptNum)
{
	UCHAR* pSrcTemp = NULL;
	UCHAR* pDstTemp = NULL;
	UCHAR* tempBuf = NULL;
	int red = 0;
	int blue = 0;

	//tempBuf = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	tempBuf = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	POINT tempPt;
	BOOL rst = FALSE;
	int data = 0 ;

	pSrcTemp = src;
	pDstTemp = tempBuf;
	for(int y = 0; y < COLORXY_LENGTH; y++)
	{
		for(int x = 0; x < COLORXY_WIDTH; x++)
		{
			data = *pSrcTemp++;
			if(data > 0)
			{
				tempPt.x = x;
				tempPt.y = y;
				rst = PtInPolygon(tempPt, pts, ptNum);

				if(rst)
					*pDstTemp++ = 255;
				else
					*pDstTemp++ = 0;
			
			}
			else
			{
				*pDstTemp++ = 0;
			}
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH),
		tempBuf, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));


	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);

	return ePART_SUCCESS;
}


int CPInsp_Color::MakeLUT_Poly(BYTE* dst, const POINT *pts, int ptNum)
{
	BYTE* dstTemp = NULL;
	BYTE* tempBuf = NULL;

	//tempBuf = new BYTE[COLORXY_WIDTH * COLORXY_LENGTH];
	tempBuf = g_pMManager->pem_new<BYTE>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH));

	POINT tempPt;
	BOOL rst = FALSE;

	dstTemp = tempBuf;
	for(int y = 0; y < COLORXY_LENGTH; y++)
	{
		for(int x = 0; x < COLORXY_WIDTH; x++)
		{
			tempPt.x = x;
			tempPt.y = y;
			rst = PtInPolygon(tempPt, pts, ptNum);

			if(rst)
				*dstTemp++ = 1;
			else
				*dstTemp++ = 0;

		}
	}

	memcpy_s(dst, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH),
		tempBuf, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH));

	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);

	return ePART_SUCCESS;
}


int CPInsp_Color::MakeLUT_Poly(BYTE* dst, int sizeX, int sizeY, const POINT *pts, int ptNum)
{
	BYTE* dstTemp = NULL;
	BYTE* tempBuf = NULL;

	//tempBuf = new BYTE[sizeX * sizeY];
	tempBuf = g_pMManager->pem_new<BYTE>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(BYTE) * sizeX * sizeY));

	POINT tempPt;
	BOOL rst = FALSE;

	dstTemp = tempBuf;

	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			tempPt.x = x;
			tempPt.y = y;
			rst = PtInPolygon(tempPt, pts, ptNum);

			if(rst)
				*dstTemp++ = 1;
			else
				*dstTemp++ = 0;

		}
	}

	memcpy_s(dst, (sizeof(BYTE) * sizeX * sizeY),
		tempBuf, (sizeof(BYTE) * sizeX * sizeY));

	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);

	return ePART_SUCCESS;
}


inline bool CPInsp_Color::PtInPolygon(POINT pt, const POINT *pts, int ptNum)
{
	POINT spt, ept;				// start, end point
	bool oddNodes = false;		// false: even(out), true: odd(in)

	for(int i=0 ; i<ptNum ; i++)
	{
		spt = pts[i];
		ept = pts[(i + 1) % ptNum];		// % prevent over index

		if(((spt.y <= pt.y) && (ept.y > pt.y))			// an upward crossing
			|| ((spt.y > pt.y) && (ept.y <=  pt.y)))	// a downward crossing
	{
			// pt.x is in left of edge
			oddNodes ^= ((float)pt.x < ((float)spt.x + ((float)(pt.y - spt.y) / (float)(ept.y - spt.y) * (float)(ept.x - spt.x))));
			}
		}

	return oddNodes;
}

bool CPInsp_Color::pvProcPointInPolygon(POINTF* ptArray, int nArrayCts, POINTF point)
{
	int nCount = nArrayCts;
	bool oddNodes = false;
	int i, j = nCount - 1;
	for (i = 0; i < nCount; i++)
	{
		if (ptArray[i].y < point.y && ptArray[j].y >= point.y || ptArray[j].y < point.y && ptArray[i].y >= point.y)
		{
			if (ptArray[i].x + (point.y - ptArray[i].y) / (ptArray[j].y - ptArray[i].y) * (ptArray[j].x - ptArray[i].x) < point.x)
				oddNodes = !oddNodes;
		}
		j = i;
	}
	return oddNodes;
}
int CPInsp_Color::MakeLUT_Blob(UCHAR* srcColorXY, BYTE* dst)
{
	UCHAR* blobImg = NULL;
	BYTE*  lutData= NULL;

	UCHAR* tempSrc = NULL;
	BYTE*  tempDst= NULL;

	//blobImg = new UCHAR[COLORXY_WIDTH * COLORXY_LENGTH];
	blobImg = g_pMManager->pem_new<UCHAR>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	//memset(blobImg, 0, (sizeof(UCHAR) * COLORXY_WIDTH * COLORXY_LENGTH));

	//lutData = new BYTE[COLORXY_WIDTH * COLORXY_LENGTH];
	lutData = g_pMManager->pem_new<BYTE>(true, COLORXY_WIDTH * COLORXY_LENGTH, (PCHAR)__FUNCTION__, __LINE__);
	memset(lutData, 0, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH));

	m_procMil->MakeBlobImg(srcColorXY, blobImg, COLORXY_WIDTH, COLORXY_LENGTH, 1);

	int data = 0;
	tempSrc = blobImg;
	tempDst = lutData;
	for(int y = 0; y < COLORXY_LENGTH; y++)
	{
		for(int x = 0; x < COLORXY_WIDTH; x++)
		{		
			data = *tempSrc++;
			if(data > 0)
			{
				*tempDst++ = 1;
			}
			else
			{
				*tempDst++ = 0;
			}
		}
	}

	memcpy_s(dst, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH), 
			lutData, (sizeof(BYTE) * COLORXY_WIDTH * COLORXY_LENGTH));

	/*delete blobImg;
	delete lutData;*/
	g_pMManager->pem_delete(blobImg, false);
	g_pMManager->pem_delete(lutData, false);

	return ePART_SUCCESS;
}


void CPInsp_Color::InitResult(RstInspColor* ret)
{
	//RstInspColor* temp = new RstInspColor;
	RstInspColor* temp = g_pMManager->pem_new<RstInspColor>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(RstInspColor));

	temp->ok = -1;
	temp->isInsp = FALSE;
	temp->list.colorRateOk = TRUE;

	*ret = *temp;
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

//void CPInsp_Color::GetInspRst(RstInspColor* ret)
//{
//	InitColorResult(ret);
//
//	*ret = m_result;
//}

void CPInsp_Color::GetInspRst( InspAlgoResult &inspAlgoResult, RECT rcLeadInsp )
{
	if(inspAlgoResult.m_nAlgoType != eAlgoColor 
		&& inspAlgoResult.m_nAlgoType != eAlgoLead_Color
		&& inspAlgoResult.m_nAlgoType != eAlgoTab)
		return;

	BOOL bIsUseMultiArea = FALSE;
	if(m_pAlgoColor && m_pAlgoColor->IsUseMultiArea())
		bIsUseMultiArea = TRUE;

	if(!bIsUseMultiArea)
	{
		RstAlgoColor *pRstAlgoColor = (RstAlgoColor*)inspAlgoResult.m_vRstInspAlgo;
		if(!pRstAlgoColor)
			return;

		if(m_pAlgoColor)
		{
			inspAlgoResult.m_bIsInsp	= m_resultClr.isInsp;
			inspAlgoResult.m_bOk		= m_resultClr.ok;

			pRstAlgoColor->m_bOKPolarity	= m_resultClr.list.polarityOK;
			pRstAlgoColor->m_bRateOK	= m_resultClr.list.colorRateOk;
			pRstAlgoColor->m_dRstRate	= m_resultClr.rstRate;
			pRstAlgoColor->m_rcRect_I = rcLeadInsp;
		}
	}
}

void CPInsp_Color::RemakePoly(POINT* srcPoint, int count, int offsetX, int offsetY, POINT* retPoint)
{
	if(retPoint == NULL)
		return;

	for(int i = 0; i < count; i++)
	{
		retPoint[i].x = srcPoint[i].x + offsetX;		
		retPoint[i].y = srcPoint[i].y + offsetY;

		if(retPoint[i].x < 0)
			retPoint[i].x = 0;

		if(retPoint[i].y < 0)
			retPoint[i].y = 0;


		if(retPoint[i].x > SIZE_CLRX - 1)
			retPoint[i].x = SIZE_CLRX - 1;

		if(retPoint[i].y > SIZE_CLRY - 1)
			retPoint[i].y = SIZE_CLRY - 1;
	}
}

// Teaching 중 일반파트인 경우 사용
// src1,2,3는 현재 FOV Image
// srt4,5 FOV Bottom Red, Blue
void CPInsp_Color::MakeProcImg(void* src1, void* src2, void* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi, bool bIsLoadFovRawDataImage, bool bForeign,
	void* src4, void* src5, UCHAR* dst4, UCHAR* dst5, void* src6, void* src7, void* src8, UCHAR* dst6, UCHAR* dst7, UCHAR* dst8)
{
	double cx = coordi.fovCx;
	double cy = coordi.fovCy;
	double roiSizeX = coordi.width;
	double roiSizeY = coordi.length;
	int roiArea = roiSizeX * roiSizeY;

	int bdCx = coordi.anyAngleCx;
	int bdCy = coordi.anyAngleCy;
	int bdSizeX = coordi.anyAngleWidth;
	int bdSizeY = coordi.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	double wndAngle = coordi.angle;

	if(src1 == NULL || dst1 == NULL)
		return;

	//////////////////////////////////////////////////////////////////////////
	BOOL bIsAnyAngle = IsAnyAngle(wndAngle);

	if(!bIsAnyAngle)
	{
		int nSX = RounD(cx - (roiSizeX / 2.0));
		int nSY = RounD(cy - (roiSizeY / 2.0));
		if(bForeign == true)
		{
			nSX = cx;
			nSY = cy;
		}

		if(m_pAlgoColor && (m_eAlgoType == eAlgoLead_Color))
		{
			int nLeadTipPos = m_pAlgoColor->m_nLeadPosition;

			int nLeadDir = m_pAlgoColor->m_nLeadTipDirection;
			int nSolderLengthHeight = m_pAlgoColor->m_nSolderLength;
			int nSolderX = 0;
			int nSolderY = 0;
			int nSolderWidth =0;
			int nSolderHeight =0;
			int nSolderScanCount = 0;

			int nImgWidth = m_wndAlgoImg.m_nWidth;
			int nImgHeight = m_wndAlgoImg.m_nHeight;

			if((nLeadDir == 0) || (nLeadDir == 1))
			{
				if(nLeadDir == 0)
				{
					if(nSolderLengthHeight >= 0)
						nSolderX = ((nSX + roiSizeX) - nLeadTipPos) - nSolderLengthHeight;
					else
					{
						nSolderX = ((nSX + roiSizeX) - nLeadTipPos);
						nSolderLengthHeight *= -1;
					}
				}
				else
				{
					nSolderX = nSX+ nLeadTipPos;
					if(nSolderLengthHeight < 0)
					{
						nSolderX += nSolderLengthHeight;
						nSolderLengthHeight *= -1;
					}
				}
				if(nSolderX < 0)	nSolderX = 0;
				nSolderY = nSY;
				nSolderWidth = nSolderLengthHeight;
				nSolderHeight = nImgHeight;
				nSolderScanCount = nSolderWidth * nSolderHeight;
			}
			else
			{
				nSolderX = nSX;

				if(nLeadDir == 2)
				{
					if(nSolderLengthHeight >= 0)
						nSolderY =((nSY + roiSizeY) - nLeadTipPos) - nSolderLengthHeight;
					else
					{
						nSolderY = ((nSY + roiSizeY) - nLeadTipPos);
						nSolderLengthHeight *= -1;
					}
				}
				else
				{
					nSolderY = nSY+ nLeadTipPos;
					if(nSolderLengthHeight < 0)
					{
						nSolderY += nSolderLengthHeight;
						nSolderLengthHeight *= -1;
					}
				}
				if(nSolderY < 0)	nSolderY = 0;
				nSolderWidth = nImgWidth;
				nSolderHeight = nSolderLengthHeight;
				nSolderScanCount = nSolderWidth * nSolderHeight;
			}

			{
				if((nSolderWidth) > nImgWidth)
					nSolderWidth = nImgWidth;

				if((nSolderHeight) > nImgHeight)
					nSolderHeight = nImgHeight;
			}

			nSX = nSolderX;
			nSY = nSolderY;
			roiSizeX = nSolderWidth;
			roiSizeY = nSolderHeight;
		}

		MakeProcImg_Clip(src1, src2, src3, dst1, dst2, dst3, bIsLoadFovRawDataImage, nSX, nSY, roiSizeX, roiSizeY, src4, src5, dst4, dst5, src6, src7, src8, dst6, dst7, dst8);
	}
	else
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;

		UCHAR* bdryImgTemp_src1 = NULL;
		m_procMil->ProcAnyAngle_Img(src1, &bdryImgTemp_src1, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_procMil->SaveWorkImg(bdryImgTemp_src1, w, l, _T("COLOR_boundaryImg1.bmp"));

		wndAngle = 0;

		int nSX = RounD(cx - (bdCx - (w / 2.0)) - roiSizeX / 2.0);
		int nSY = RounD(cy - (bdCy - (l / 2.0)) - roiSizeY / 2.0);

		if(m_pAlgoColor && (m_eAlgoType == eAlgoLead_Color))
		{
			int nLeadTipPos = m_pAlgoColor->m_nLeadPosition;

			int nLeadDir = m_pAlgoColor->m_nLeadTipDirection;
			int nSolderLengthHeight = m_pAlgoColor->m_nSolderLength;
			int nSolderX = 0;
			int nSolderY = 0;
			int nSolderWidth =0;
			int nSolderHeight =0;
			int nSolderScanCount = 0;

			int nImgWidth = m_wndAlgoImg.m_nWidth;
			int nImgHeight = m_wndAlgoImg.m_nHeight;

			if((nLeadDir == 0) || (nLeadDir == 1))
			{
				if(nLeadDir == 0)
				{
					if(nSolderLengthHeight >= 0)
						nSolderX = ((nSX + roiSizeX) - nLeadTipPos) - nSolderLengthHeight;
					else
					{
						nSolderX = ((nSX + roiSizeX) - nLeadTipPos);
						nSolderLengthHeight *= -1;
					}
				}
				else
				{
					nSolderX = nSX+ nLeadTipPos;
					if(nSolderLengthHeight < 0)
					{
						nSolderX += nSolderLengthHeight;
						nSolderLengthHeight *= -1;
					}
				}
				if(nSolderX < 0)	nSolderX = 0;
				nSolderY = nSY;
				nSolderWidth = nSolderLengthHeight;
				nSolderHeight = nImgHeight;
				nSolderScanCount = nSolderWidth * nSolderHeight;
			}
			else
			{
				nSolderX = nSX;

				if(nLeadDir == 2)
				{
					if(nSolderLengthHeight >= 0)
						nSolderY =((nSY + roiSizeY) - nLeadTipPos) - nSolderLengthHeight;
					else
					{
						nSolderY = ((nSY + roiSizeY) - nLeadTipPos);
						nSolderLengthHeight *= -1;
					}
				}
				else
				{
					nSolderY = nSY+ nLeadTipPos;
					if(nSolderLengthHeight < 0)
					{
						nSolderY += nSolderLengthHeight;
						nSolderLengthHeight *= -1;
					}
				}
				if(nSolderY < 0)	nSolderY = 0;
				nSolderWidth = nImgWidth;
				nSolderHeight = nSolderLengthHeight;
				nSolderScanCount = nSolderWidth * nSolderHeight;
			}

			{
				if((nSolderWidth) > nImgWidth)
					nSolderWidth = nImgWidth;

				if((nSolderHeight) > nImgHeight)
					nSolderHeight = nImgHeight;
			}

			nSX = nSolderX;
			nSY = nSolderY;
			roiSizeX = nSolderWidth;
			roiSizeY = nSolderHeight;
		}

		m_procMil->GetClipImage_LT(bdryImgTemp_src1, w, l, dst1, nSX, nSY, roiSizeX, roiSizeY);
		//delete bdryImgTemp_src1;
		g_pMManager->pem_delete(bdryImgTemp_src1, false);
		m_procMil->SaveWorkImg(dst1, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));


		if(src2 != NULL && dst2 != NULL)
		{
			UCHAR* bdryImgTemp_src2 = NULL;
			m_procMil->ProcAnyAngle_Img(src2, &bdryImgTemp_src2, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src2, w, l, _T("COLOR_boundaryImg2.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src2, w, l, dst2, nSX, nSY, roiSizeX, roiSizeY);
			//delete bdryImgTemp_src2;
			g_pMManager->pem_delete(bdryImgTemp_src2, false);
			m_procMil->SaveWorkImg(dst2, roiSizeX, roiSizeY, _T("COLOR_dst2.bmp"));
		}


		if(src3 != NULL && dst3 != NULL)
		{
			UCHAR* bdryImgTemp_src3 = NULL;
			m_procMil->ProcAnyAngle_Img(src3, &bdryImgTemp_src3, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src3, w, l, _T("COLOR_boundaryImg3.bmp"));
		
			m_procMil->GetClipImage_LT(bdryImgTemp_src3, w, l, dst3, nSX, nSY, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src3;
			g_pMManager->pem_delete(bdryImgTemp_src3, false);
			m_procMil->SaveWorkImg(dst3, roiSizeX, roiSizeY, _T("COLOR_dst3.bmp"));
		}

		if(src4 != NULL && dst4 != NULL)
		{
			UCHAR* bdryImgTemp_src4 = NULL;
			m_procMil->ProcAnyAngle_Img(src4, &bdryImgTemp_src4, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src4, w, l, _T("COLOR_boundaryImg4.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src4, w, l, dst4, nSX, nSY, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src4;
			g_pMManager->pem_delete(bdryImgTemp_src4, false);
			m_procMil->SaveWorkImg(dst4, roiSizeX, roiSizeY, _T("COLOR_dst4.bmp"));
		}

		if(src5 != NULL && dst5 != NULL)
		{
			UCHAR* bdryImgTemp_src5 = NULL;
			m_procMil->ProcAnyAngle_Img(src5, &bdryImgTemp_src5, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src5, w, l, _T("COLOR_boundaryImg5.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src5, w, l, dst5, nSX, nSY, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src5;
			g_pMManager->pem_delete(bdryImgTemp_src5, false);
			m_procMil->SaveWorkImg(dst5, roiSizeX, roiSizeY, _T("COLOR_dst5.bmp"));
		}

		if(src6 != NULL && dst6 != NULL)
		{
			UCHAR* bdryImgTemp_src6 = NULL;
			m_procMil->ProcAnyAngle_Img(src6, &bdryImgTemp_src6, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src6, w, l, _T("COLOR_boundaryImg6.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src6, w, l, dst6, nSX, nSY, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src6;
			g_pMManager->pem_delete(bdryImgTemp_src6, false);
			m_procMil->SaveWorkImg(dst6, roiSizeX, roiSizeY, _T("COLOR_dst6.bmp"));
		}

		if(src7 != NULL && dst7 != NULL)
		{
			UCHAR* bdryImgTemp_src7 = NULL;
			m_procMil->ProcAnyAngle_Img(src7, &bdryImgTemp_src7, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src7, w, l, _T("COLOR_boundaryImg7.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src7, w, l, dst7, nSX, nSY, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src7;
			g_pMManager->pem_delete(bdryImgTemp_src7, false);
			m_procMil->SaveWorkImg(dst7, roiSizeX, roiSizeY, _T("COLOR_dst7.bmp"));
		}

		if(src8 != NULL && dst8 != NULL)
		{
			UCHAR* bdryImgTemp_src8 = NULL;
			m_procMil->ProcAnyAngle_Img(src8, &bdryImgTemp_src8, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src8, w, l, _T("COLOR_boundaryImg8.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src8, w, l, dst8, nSX, nSY, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src8;
			g_pMManager->pem_delete(bdryImgTemp_src8, false);
			m_procMil->SaveWorkImg(dst8, roiSizeX, roiSizeY, _T("COLOR_dst8.bmp"));
		}
	}
}

void CPInsp_Color::MakeProcImg_Clip( void *pSrc1, void *pSrc2, void *pSrc3, UCHAR* pDst1, UCHAR* pDst2, UCHAR* pDst3, bool bIsLoadFovRawDataImage, int nSX, int nSY, double roiSizeX, double roiSizeY, 
	void* pSrc4, void* pSrc5, UCHAR* pDst4, UCHAR* pDst5, void* pSrc6, void* pSrc7, void* pSrc8, UCHAR* pDst6, UCHAR* pDst7, UCHAR* pDst8)
{
	if(bIsLoadFovRawDataImage == false)
	{
		if (g_pMPTI->m_nCameraType == Basler_Color)
		{
			int nFOVSizeX = 2040;
			int nFOVSizeY = 2040;
			//m_procMil->GetClipBuff_LT(pSrc1, pDst1, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->GetClipImage_LT((UCHAR*)pSrc1, nFOVSizeX, nFOVSizeY, pDst1, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst1, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));

			if (pSrc2 != NULL && pDst2 != NULL)
			{
				m_procMil->GetClipImage_LT((UCHAR*)pSrc2, nFOVSizeX, nFOVSizeY, pDst2, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst2, roiSizeX, roiSizeY, _T("COLOR_dst2.bmp"));
			}

			if (pSrc3 != NULL && pDst3 != NULL)
			{
				m_procMil->GetClipImage_LT((UCHAR*)pSrc3, nFOVSizeX, nFOVSizeY, pDst3, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst3, roiSizeX, roiSizeY, _T("COLOR_dst3.bmp"));
			}

			if (pSrc4 != NULL && pDst4 != NULL)
			{
				m_procMil->GetClipImage_LT((UCHAR*)pSrc4, nFOVSizeX, nFOVSizeY, pDst4, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst4, roiSizeX, roiSizeY, _T("COLOR_dst4.bmp"));
			}

			if (pSrc5 != NULL && pDst5 != NULL)
			{
				m_procMil->GetClipImage_LT((UCHAR*)pSrc5, nFOVSizeX, nFOVSizeY, pDst5, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst5, roiSizeX, roiSizeY, _T("COLOR_dst5.bmp"));
			}

			if (pSrc6 != NULL && pDst6 != NULL)
			{
				m_procMil->GetClipImage_LT((UCHAR*)pSrc6, nFOVSizeX, nFOVSizeY, pDst6, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst6, roiSizeX, roiSizeY, _T("COLOR_dst6.bmp"));
			}

			if (pSrc7 != NULL && pDst7 != NULL)
			{
				m_procMil->GetClipImage_LT((UCHAR*)pSrc7, nFOVSizeX, nFOVSizeY, pDst7, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst7, roiSizeX, roiSizeY, _T("COLOR_dst7.bmp"));
			}

			if (pSrc8 != NULL && pDst8 != NULL)
			{
				m_procMil->GetClipImage_LT((UCHAR*)pSrc8, nFOVSizeX, nFOVSizeY, pDst8, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst8, roiSizeX, roiSizeY, _T("COLOR_dst8.bmp"));
			}
		}
		else
		{
			m_procMil->GetClipBuff_LT(pSrc1, pDst1, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst1, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));

			if (pSrc2 != NULL && pDst2 != NULL)
			{
				m_procMil->GetClipBuff_LT(pSrc2, pDst2, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst2, roiSizeX, roiSizeY, _T("COLOR_dst2.bmp"));
			}

			if (pSrc3 != NULL && pDst3 != NULL)
			{
				m_procMil->GetClipBuff_LT(pSrc3, pDst3, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst3, roiSizeX, roiSizeY, _T("COLOR_dst3.bmp"));
			}

			if (pSrc4 != NULL && pDst4 != NULL)
			{
				m_procMil->GetClipBuff_LT(pSrc4, pDst4, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst4, roiSizeX, roiSizeY, _T("COLOR_dst4.bmp"));
			}

			if (pSrc5 != NULL && pDst5 != NULL)
			{
				m_procMil->GetClipBuff_LT(pSrc5, pDst5, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst5, roiSizeX, roiSizeY, _T("COLOR_dst5.bmp"));
			}

			if (pSrc6 != NULL && pDst6 != NULL)
			{
				m_procMil->GetClipBuff_LT(pSrc6, pDst6, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst6, roiSizeX, roiSizeY, _T("COLOR_dst6.bmp"));
			}

			if (pSrc7 != NULL && pDst7 != NULL)
			{
				m_procMil->GetClipBuff_LT(pSrc7, pDst7, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst7, roiSizeX, roiSizeY, _T("COLOR_dst7.bmp"));
			}

			if (pSrc8 != NULL && pDst8 != NULL)
			{
				m_procMil->GetClipBuff_LT(pSrc8, pDst8, nSX, nSY, roiSizeX, roiSizeY);
				m_procMil->SaveWorkImg(pDst8, roiSizeX, roiSizeY, _T("COLOR_dst8.bmp"));
			}
		}
	}
	else
	{
		m_procMil->GetClipBuff_LT_LoadRawData(pSrc1, pDst1, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(pDst1, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));

		if(pSrc2 != NULL && pDst2 != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc2, pDst2, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst2, roiSizeX, roiSizeY, _T("COLOR_dst2.bmp"));
		}

		if(pSrc3 != NULL && pDst3 != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc3, pDst3, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst3, roiSizeX, roiSizeY, _T("COLOR_dst3.bmp"));
		}

		if(pSrc4 != NULL && pDst4 != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc4, pDst4, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst4, roiSizeX, roiSizeY, _T("COLOR_dst4.bmp"));
		}

		if(pSrc5 != NULL && pDst5 != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc5, pDst5, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst5, roiSizeX, roiSizeY, _T("COLOR_dst5.bmp"));
		}

		if(pSrc6 != NULL && pDst6 != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc6, pDst6, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst6, roiSizeX, roiSizeY, _T("COLOR_dst6.bmp"));
		}

		if(pSrc7 != NULL && pDst7 != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc7, pDst7, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst7, roiSizeX, roiSizeY, _T("COLOR_dst7.bmp"));
		}

		if(pSrc8 != NULL && pDst8 != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc8, pDst8, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst8, roiSizeX, roiSizeY, _T("COLOR_dst8.bmp"));
		}
	}
}

// 검사 중 사용(Big part, 일반각 무관하게 사용)
// src1,2,3는 Part Image
void CPInsp_Color::MakeProcImg(UCHAR * src1, UCHAR* src2, UCHAR* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi, UCHAR * src4, UCHAR* src5, UCHAR* dst4, UCHAR* dst5)
{
	int roiSizeX = coordi.width;
	int roiSizeY = coordi.length;
	int roiArea = roiSizeX * roiSizeY;

	if(src1 == NULL || dst1 == NULL)
		return;
	int nSX = coordi.cx;
	int nSY = coordi.cy;
	MakeProcImg_Clip(src1, src2, src3, dst1, dst2, dst3, nSX, nSY, roiSizeX, roiSizeY, src4, src5, dst4, dst5);

// 	int nCompoLightMode = g_pMPTI->GetCompositeLightMode();
// 	if(nCompoLightMode == 1)
// 	{
// 		if(src4 != NULL && dst4 != NULL)
// 			m_procMil->GetClipImage_LT(src4, m_nPartWidth, m_nPartHeight, dst4, nSX, nSY, roiSizeX, roiSizeY);
// 
// 		if(src5 != NULL && dst5 != NULL)
// 			m_procMil->GetClipImage_LT(src5, m_nPartWidth, m_nPartHeight, dst5, nSX, nSY, roiSizeX, roiSizeY);
// 	}
}

void CPInsp_Color::MakeProcImg_Clip( UCHAR * src1, UCHAR* src2, UCHAR* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, int nSX, int nSY, int roiSizeX, int roiSizeY,
	UCHAR* src4, UCHAR* src5, UCHAR* dst4, UCHAR* dst5, int nImgW, int nImgH)
{
	int nW = (nImgW <= 0) ? m_nPartWidth : nImgW;
	int nH = (nImgH <= 0) ? m_nPartHeight : nImgH;
	if(nSX == 0 && nSY == 0 && nW == roiSizeX && nH == roiSizeY)
	{
		if(src1 != NULL && dst1 != NULL)
			memcpy(dst1, src1, sizeof(UCHAR) * nW * nH);
		if(src2 != NULL && dst2 != NULL)
			memcpy(dst2, src2, sizeof(UCHAR) * nW * nH);
		if(src3 != NULL && dst3 != NULL)
			memcpy(dst3, src3, sizeof(UCHAR) * nW * nH);
		if(src4 != NULL && dst4 != NULL)
			memcpy(dst4, src4, sizeof(UCHAR) * nW * nH);
		if(src5 != NULL && dst5 != NULL)
			memcpy(dst5, src5, sizeof(UCHAR) * nW * nH);
		return;
	}
	m_procMil->GetClipImage_LT(src1, nW, nH, dst1, nSX, nSY, roiSizeX, roiSizeY);
	m_procMil->SaveWorkImg(dst1, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));

	if(src2 != NULL && dst2 != NULL)
	{
		m_procMil->GetClipImage_LT(src2, nW, nH, dst2, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(dst2, roiSizeX, roiSizeY, _T("COLOR_dst2.bmp"));
	}

	if(src3 != NULL && dst3 != NULL)
	{
		m_procMil->GetClipImage_LT(src3, nW, nH, dst3, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(dst3, roiSizeX, roiSizeY, _T("COLOR_dst3.bmp"));
	}

	if(src4 != NULL && dst4 != NULL)
	{
		m_procMil->GetClipImage_LT(src4, nW, nH, dst4, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(dst4, roiSizeX, roiSizeY, _T("COLOR_dst4.bmp"));
	}

	if(src5 != NULL && dst5 != NULL)
	{
		m_procMil->GetClipImage_LT(src5, nW, nH, dst5, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(dst5, roiSizeX, roiSizeY, _T("COLOR_dst5.bmp"));
	}
}

// Teaching에서 big part 인 경우 사용함
// src1,2,3는 FOV 여러장 합성 이미지
void CPInsp_Color::MakeProcImg_Big_Teach(UCHAR * src1, UCHAR* src2, UCHAR* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi, UCHAR * src4, UCHAR* src5, UCHAR * dst4, UCHAR* dst5, UCHAR * src6, UCHAR* src7, UCHAR* src8, UCHAR * dst6, UCHAR* dst7, UCHAR* dst8)
{
	double roiSizeX = coordi.width;
	double roiSizeY = coordi.length;
	int roiArea = roiSizeX * roiSizeY;

	double wndAngle = coordi.angle;

	if(src1 == NULL || dst1 == NULL)
		return;

	BOOL bIsAnyAngle = IsAnyAngle(wndAngle);

	if(!bIsAnyAngle)
	{
		int nSX = coordi.cx + m_nPartWidth / 2.0;		// FOV 합성 영상의 좌상단이 0,0인  Window 좌상단 좌표
		int nSY = - coordi.cy + m_nPartHeight / 2.0;

		if(m_pAlgoColor && (m_eAlgoType == eAlgoLead_Color))
		{
			int nLeadTipPos = m_pAlgoColor->m_nLeadPosition;

			int nLeadDir = m_pAlgoColor->m_nLeadTipDirection;
			int nSolderLengthHeight = m_pAlgoColor->m_nSolderLength;
			int nSolderX = 0;
			int nSolderY = 0;
			int nSolderWidth =0;
			int nSolderHeight =0;
			int nSolderScanCount = 0;

			int nImgWidth = m_wndAlgoImg.m_nWidth;
			int nImgHeight = m_wndAlgoImg.m_nHeight;

			if((nLeadDir == 0) || (nLeadDir == 1))
			{
				if(nLeadDir == 0)
				{
					if(nSolderLengthHeight >= 0)
						nSolderX = ((nSX + roiSizeX) - nLeadTipPos) - nSolderLengthHeight;
					else
					{
						nSolderX = ((nSX + roiSizeX) - nLeadTipPos);
						nSolderLengthHeight *= -1;
					}
				}
				else
				{
					nSolderX = nSX+ nLeadTipPos;
					if(nSolderLengthHeight < 0)
					{
						nSolderX += nSolderLengthHeight;
						nSolderLengthHeight *= -1;
					}
				}
				if(nSolderX < 0)	nSolderX = 0;
				nSolderY = nSY;
				nSolderWidth = nSolderLengthHeight;
				nSolderHeight = nImgHeight;
				nSolderScanCount = nSolderWidth * nSolderHeight;
			}
			else
			{
				nSolderX = nSX;

				if(nLeadDir == 2)
				{
					if(nSolderLengthHeight >= 0)
						nSolderY =((nSY + roiSizeY) - nLeadTipPos) - nSolderLengthHeight;
					else
					{
						nSolderY = ((nSY + roiSizeY) - nLeadTipPos);
						nSolderLengthHeight *= -1;
					}
				}
				else
				{
					nSolderY = nSY+ nLeadTipPos;
					if(nSolderLengthHeight < 0)
					{
						nSolderY += nSolderLengthHeight;
						nSolderLengthHeight *= -1;
					}
				}
				if(nSolderY < 0)	nSolderY = 0;
				nSolderWidth = nImgWidth;
				nSolderHeight = nSolderLengthHeight;
				nSolderScanCount = nSolderWidth * nSolderHeight;
			}

			{
				if((nSolderWidth) > nImgWidth)
					nSolderWidth = nImgWidth;

				if((nSolderHeight) > nImgHeight)
					nSolderHeight = nImgHeight;
			}

			nSX = nSolderX;
			nSY = nSolderY;
			roiSizeX = nSolderWidth;
			roiSizeY = nSolderHeight;
		}

		m_procMil->GetClipImage_LT(src1, m_nPartWidth, m_nPartHeight, dst1, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(dst1, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));

		if(src2 != NULL && dst2 != NULL)
		{
			m_procMil->GetClipImage_LT(src2, m_nPartWidth, m_nPartHeight, dst2, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(dst2, roiSizeX, roiSizeY, _T("COLOR_dst2.bmp"));
		}

		if(src3 != NULL && dst3 != NULL)
		{
			m_procMil->GetClipImage_LT(src3, m_nPartWidth, m_nPartHeight, dst3, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(dst3, roiSizeX, roiSizeY, _T("COLOR_dst3.bmp"));
		}

		if(src4 != NULL && dst4 != NULL)
		{
			m_procMil->GetClipImage_LT(src4, m_nPartWidth, m_nPartHeight, dst4, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(dst4, roiSizeX, roiSizeY, _T("COLOR_dst4.bmp"));
		}

		if(src5 != NULL && dst5 != NULL)
		{
			m_procMil->GetClipImage_LT(src5, m_nPartWidth, m_nPartHeight, dst5, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(dst5, roiSizeX, roiSizeY, _T("COLOR_dst5.bmp"));
		}

		if(src6 != NULL && dst6 != NULL)
		{
			m_procMil->GetClipImage_LT(src6, m_nPartWidth, m_nPartHeight, dst6, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(dst6, roiSizeX, roiSizeY, _T("COLOR_dst6.bmp"));
		}

		if(src7 != NULL && dst7 != NULL)
		{
			m_procMil->GetClipImage_LT(src7, m_nPartWidth, m_nPartHeight, dst7, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(dst7, roiSizeX, roiSizeY, _T("COLOR_dst7.bmp"));
		}

		if(src8 != NULL && dst8 != NULL)
		{
			m_procMil->GetClipImage_LT(src8, m_nPartWidth, m_nPartHeight, dst8, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(dst8, roiSizeX, roiSizeY, _T("COLOR_dst8.bmp"));
		}
	}
	else
	{
		double cx = 0;
		double cy = 0;
		double roiSizeX = coordi.width;
		double roiSizeY = coordi.length;

		int bdSizeX = coordi.anyAngleWidth;
		int bdSizeY = coordi.anyAngleLength;
		int bdCx = m_nPartWidth / 2.;
		int bdCy = m_nPartHeight / 2.;

		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;
		UCHAR* bdryImgTemp_src1 = NULL;
		m_procMil->ProcAnyAngle_Img(src1, &bdryImgTemp_src1, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, &w, &l);
		m_procMil->SaveWorkImg(bdryImgTemp_src1, w, l, _T("COLOR_boundaryImg1.bmp"));

		wndAngle = 0;
		cx = (int)(coordi.cx + w / 2.0);
		cy = (int)(-coordi.cy + l / 2.0);

		m_procMil->GetClipImage_LT(bdryImgTemp_src1, w, l, dst1, cx, cy, roiSizeX, roiSizeY);
		//delete bdryImgTemp_src1;
		g_pMManager->pem_delete(bdryImgTemp_src1, false);
		m_procMil->SaveWorkImg(dst1, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));


		if(src2 != NULL && dst2 != NULL)
		{
			UCHAR* bdryImgTemp_src2 = NULL;
			m_procMil->ProcAnyAngle_Img(src2, &bdryImgTemp_src2, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src2, w, l, _T("COLOR_boundaryImg2.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src2, w, l, dst2, cx, cy, roiSizeX, roiSizeY);
			//delete bdryImgTemp_src2;
			g_pMManager->pem_delete(bdryImgTemp_src2, false);
			m_procMil->SaveWorkImg(dst2, roiSizeX, roiSizeY, _T("COLOR_dst2.bmp"));
		}


		if(src3 != NULL && dst3 != NULL)
		{
			UCHAR* bdryImgTemp_src3 = NULL;
			m_procMil->ProcAnyAngle_Img(src3, &bdryImgTemp_src3, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src3, w, l, _T("COLOR_boundaryImg3.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src3, w, l, dst3, cx, cy, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src3;
			g_pMManager->pem_delete(bdryImgTemp_src3, false);
			m_procMil->SaveWorkImg(dst3, roiSizeX, roiSizeY, _T("COLOR_dst3.bmp"));
		}

		if(src4 != NULL && dst4 != NULL)
		{
			UCHAR* bdryImgTemp_src4 = NULL;
			m_procMil->ProcAnyAngle_Img(src4, &bdryImgTemp_src4, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src4, w, l, _T("COLOR_boundaryImg4.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src4, w, l, dst4, cx, cy, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src4;
			g_pMManager->pem_delete(bdryImgTemp_src4, false);
			m_procMil->SaveWorkImg(dst4, roiSizeX, roiSizeY, _T("COLOR_dst4.bmp"));
		}

		if(src5 != NULL && dst5 != NULL)
		{
			UCHAR* bdryImgTemp_src5 = NULL;
			m_procMil->ProcAnyAngle_Img(src5, &bdryImgTemp_src5, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src5, w, l, _T("COLOR_boundaryImg5.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src5, w, l, dst5, cx, cy, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src5;
			g_pMManager->pem_delete(bdryImgTemp_src5, false);
			m_procMil->SaveWorkImg(dst5, roiSizeX, roiSizeY, _T("COLOR_dst5.bmp"));
		}

		if(src6 != NULL && dst6 != NULL)
		{
			UCHAR* bdryImgTemp_src6 = NULL;
			m_procMil->ProcAnyAngle_Img(src6, &bdryImgTemp_src6, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src6, w, l, _T("COLOR_boundaryImg6.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src6, w, l, dst6, cx, cy, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src6;
			g_pMManager->pem_delete(bdryImgTemp_src6, false);
			m_procMil->SaveWorkImg(dst6, roiSizeX, roiSizeY, _T("COLOR_dst6.bmp"));
		}

		if(src7 != NULL && dst7 != NULL)
		{
			UCHAR* bdryImgTemp_src7 = NULL;
			m_procMil->ProcAnyAngle_Img(src7, &bdryImgTemp_src7, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src7, w, l, _T("COLOR_boundaryImg7.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src7, w, l, dst7, cx, cy, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src7;
			g_pMManager->pem_delete(bdryImgTemp_src7, false);
			m_procMil->SaveWorkImg(dst7, roiSizeX, roiSizeY, _T("COLOR_dst7.bmp"));
		}

		if(src8 != NULL && dst8 != NULL)
		{
			UCHAR* bdryImgTemp_src8 = NULL;
			m_procMil->ProcAnyAngle_Img(src8, &bdryImgTemp_src8, m_nPartWidth, m_nPartHeight, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, 1, NULL, NULL);
			m_procMil->SaveWorkImg(bdryImgTemp_src8, w, l, _T("COLOR_boundaryImg8.bmp"));

			m_procMil->GetClipImage_LT(bdryImgTemp_src8, w, l, dst8, cx, cy, roiSizeX, roiSizeY);	
			//delete bdryImgTemp_src8;
			g_pMManager->pem_delete(bdryImgTemp_src8, false);
			m_procMil->SaveWorkImg(dst8, roiSizeX, roiSizeY, _T("COLOR_dst8.bmp"));
		}
	}
}

//************************************
// Method:    GetWidthImg / GetHeightImg
// Qualifier:
// eAlgoColor인 경우엔 윈도우 전체크기를 eAlgoLead_Color경우엔 Lead Gap크기를 사용
//************************************
int CPInsp_Color::GetWidthImg( Coordinate coordinate )
{
	return (m_eAlgoType != eAlgoLead_Color) ? coordinate.width : m_wndAlgoImg.m_nWidth;
}

int CPInsp_Color::GetHeightImg( Coordinate coordinate )
{
	return (m_eAlgoType != eAlgoLead_Color) ? coordinate.length : m_wndAlgoImg.m_nHeight;
}

BOOL CPInsp_Color::CheckThreadShapeArea(int nImgWidth, int nImgHeight, int nTargetWidth, int nTargetHeight, int nMinPixel)
{
	bool bIsRemove = false;

	int nQuarterWidth = nImgWidth / 4;
	int nQuarterHeight = nImgHeight / 4;


	if(nQuarterWidth*3 < nTargetWidth  && nTargetHeight <= nMinPixel)
		bIsRemove = true;

	if(nQuarterHeight*3 < nTargetHeight  && nTargetWidth <= nMinPixel)
		bIsRemove = true;


	return bIsRemove;
}

BOOL CPInsp_Color::TeachColorXY(const InspAlgo &sInspAlgo, const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, RstAlgoColorXY *sRstAlgo, float* nGapXMin, float* nGapXMax, float* nGapYMin, float* nGapYMax, BOOL bIsLoadFovRawDataImage)
{
	BOOL bResult = FALSE;

	if(sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoColorXY));
		sRstAlgo->Init();
	}	
	int width = GetWidthImg(m_teachCoordinate);
	int length = GetHeightImg(m_teachCoordinate);

	if(m_pAlgoColorXY == NULL)
		return FALSE;

	float fitR = m_pAlgoColorXY->m_fFatorRed;
	float fitG = m_pAlgoColorXY->m_fFatorGreen;
	float fitB = m_pAlgoColorXY->m_fFatorBlue;

	int offsetX = CIE_OFFSETX;
	int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);

	/*UCHAR* img_TR = new UCHAR[width * length];
	UCHAR* img_TG = new UCHAR[width * length];
	UCHAR* img_TB = new UCHAR[width * length];*/
	UCHAR* img_TR = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* img_TG = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* img_TB = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* img_BB = NULL;
	
	if(g_pMPTI->IsMachineTypeUV())
		//img_BB = new UCHAR[width * length];
		img_BB = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	
	if(!m_bIsBigPart)
	{
		if(img_TR != NULL)	MakeProcImg(m_imgBuf[eImg_Top_R], img_TR, m_teachCoordinate, bIsLoadFovRawDataImage);
		if(img_TG != NULL)	MakeProcImg(m_imgBuf[eImg_Top_G], img_TG, m_teachCoordinate, bIsLoadFovRawDataImage);
		if(img_TB != NULL)	MakeProcImg(m_imgBuf[eImg_Top_B], img_TB, m_teachCoordinate, bIsLoadFovRawDataImage);
		if(img_BB != NULL)	MakeProcImg(m_imgBuf[eImg_Bottom_B], img_BB, m_teachCoordinate, bIsLoadFovRawDataImage);
	}
	else
	{
		if(img_TR != NULL)	MakeProcImg_Big(m_pPartImgBuf[eImg_Top_R], img_TR, m_teachCoordinate);
		if(img_TG != NULL)	MakeProcImg_Big(m_pPartImgBuf[eImg_Top_G], img_TG, m_teachCoordinate);
		if(img_TB != NULL)	MakeProcImg_Big(m_pPartImgBuf[eImg_Top_B], img_TB, m_teachCoordinate);		
		if(img_BB != NULL)	MakeProcImg_Big(m_pPartImgBuf[eImg_Bottom_B], img_BB, m_teachCoordinate);
	}

	if(img_TR != NULL)	m_procMil->SaveWorkImg(img_TR, width, length, _T("windowRed.bmp"));
	if(img_TG != NULL)	m_procMil->SaveWorkImg(img_TG, width, length, _T("windowGreen.bmp"));
	if(img_TB != NULL)	m_procMil->SaveWorkImg(img_TB, width, length, _T("windowBlue.bmp"));
	if(img_BB != NULL)	m_procMil->SaveWorkImg(img_BB, width, length, _T("windowBBlue.bmp"));

	int nCntValid = 0;
	POINTF rate = CompareXYAvg(img_TR, img_TG, img_TB, img_BB, fitR, fitG, fitB, fitB, width, length);

	bResult = TRUE;
	sRstAlgo->m_bColorXYOK_X = TRUE;
	sRstAlgo->m_bColorXYOK_Y = TRUE;

	if(sRstAlgo)
	{
		sRstAlgo->m_dRstColorXY_X = rate.x;
		sRstAlgo->m_dRstColorXY_Y = rate.y;

		if(m_pAlgoColorXY->m_fColorAvgX > 0 && m_pAlgoColorXY->m_fColorAvgY > 0)
		{
			float MinX = m_pAlgoColorXY->m_fColorAvgX - m_pAlgoColorXY->m_fColorXYMinX;
			float MaxX = m_pAlgoColorXY->m_fColorAvgX + m_pAlgoColorXY->m_fColorXYMaxX;

			float MinY = m_pAlgoColorXY->m_fColorAvgY - m_pAlgoColorXY->m_fColorXYMinY;
			float MaxY = m_pAlgoColorXY->m_fColorAvgY + m_pAlgoColorXY->m_fColorXYMaxY;

			if(MinX > rate.x || MaxX < rate.x)
			{
				bResult = FALSE;

				sRstAlgo->m_bColorXYOK_X = FALSE;
			}

			if(MinY > rate.y || MaxY < rate.y)
			{
				bResult = FALSE;

				sRstAlgo->m_bColorXYOK_Y = FALSE;
			}
		}
		else
			bResult = FALSE;

		// all measure 안 한 경우
		if(m_pAlgoColorXY->m_fColorAvgX == 0 && m_pAlgoColorXY->m_fColorXYMinX == 0 && m_pAlgoColorXY->m_fColorXYMaxX == 0)
		{
			bResult = FALSE;
			sRstAlgo->m_bColorXYOK_X = FALSE;
		}
		if(m_pAlgoColorXY->m_fColorAvgY == 0 && m_pAlgoColorXY->m_fColorXYMinY == 0 && m_pAlgoColorXY->m_fColorXYMaxY == 0)
		{
			bResult = FALSE;
			sRstAlgo->m_bColorXYOK_Y = FALSE;
		}
	}

// 	if(g_pMPTI->IsMachineTypeUV())
// 	{
// 		int nInspMargin = 1;
// 
// 		AlgoDebuggingMode vAlgoDebuggingMode = g_pInspMng->GetAlgoDebuggingMode(g_pMPTI->GetLaneNum());
// 
// 		// WndTeaching에서 AutoDebug일때 만 사용.
// 		if(vAlgoDebuggingMode.bReviewMode)
// 		{
// 			DefaultColorXYStd vDefaultColorXYStd = g_pInspMng->GetDefaultColorXYStd();
// 
// 			if(m_pAlgoColorXY->m_fColorAvgX != 0 && m_pAlgoColorXY->m_fColorAvgY != 0)
// 			{
// 				bool bIsIncludeNG = vAlgoDebuggingMode.bIncludeAlgoNG == true ? true : bResult;	// NG도 포함하는 모드이면 무조건 하고 아니면 OK 일때만 한다.
// 
// 				float fCurrMaxX = rate.x + vDefaultColorXYStd.fStdColorXYMaxX;
// 				float fCurrMinX = rate.x - vDefaultColorXYStd.fStdColorXYMinX;
// 				float fCurrMaxY = rate.y + vDefaultColorXYStd.fStdColorXYMaxY;
// 				float fCurrMinY = rate.y - vDefaultColorXYStd.fStdColorXYMinY;
// 
// 				// is Result good?
// 				if(bIsIncludeNG)
// 				{
// 					if(fCurrMaxX > m_pAlgoColorXY->m_fColorAvgX + m_pAlgoColorXY->m_fColorXYMaxX)	// 기존 티칭값 + 기존 MAX값 
// 						*nGapXMax = fCurrMaxX - m_pAlgoColorXY->m_fColorAvgX;
// 
// 					if(fCurrMinX < m_pAlgoColorXY->m_fColorAvgX - m_pAlgoColorXY->m_fColorXYMinX)	// 기존 티칭값 - 기존 Min값 
// 						*nGapXMin = m_pAlgoColorXY->m_fColorAvgX - fCurrMinX;
// 
// 					if(fCurrMaxY > m_pAlgoColorXY->m_fColorAvgY + m_pAlgoColorXY->m_fColorXYMaxY)	// 기존 티칭값 + 기존 MAX값 
// 						*nGapYMax = fCurrMaxY - m_pAlgoColorXY->m_fColorAvgY;
// 
// 					if(fCurrMinY < m_pAlgoColorXY->m_fColorAvgY - m_pAlgoColorXY->m_fColorXYMinY)	// 기존 티칭값 - 기존 Min값 
// 						*nGapYMin = m_pAlgoColorXY->m_fColorAvgY - fCurrMinY;
// 				}				
// 			}		
// 		}
// 	}

	if(img_TR != NULL)
	{
		//delete[] img_TR;
		g_pMManager->pem_delete(img_TR, true);
		img_TR = NULL;
	}

	if(img_TG != NULL)
	{
		//delete[] img_TG;
		g_pMManager->pem_delete(img_TG, true);
		img_TG = NULL;
	}

	if(img_TB != NULL)
	{
		//delete[] img_TB;
		g_pMManager->pem_delete(img_TB, true);
		img_TB = NULL;
	}

	if(img_BB != NULL)
	{
		//delete[] img_BB;
		g_pMManager->pem_delete(img_BB, true);
		img_BB = NULL;
	}

	return bResult; 
}
BOOL CPInsp_Color::InspColorXY(const InspAlgo &sInspAlgo, const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, RstAlgoColorXY *sRstAlgo, byte byInspType)
{
	BOOL bResult = FALSE;

	if(sRstAlgo)
	{
		memset(sRstAlgo, 0, sizeof(RstAlgoColorXY));
		sRstAlgo->Init();
	}

	// 예외처리
	if(g_pMPTI->IsMachineTypeUV() == FALSE)
		byInspType = WndCoating;

	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	float fitR = m_pAlgoColorXY->m_fFatorRed;
	float fitG = m_pAlgoColorXY->m_fFatorGreen;
	float fitB = m_pAlgoColorXY->m_fFatorBlue;

	int offsetX = CIE_OFFSETX;
	int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);

	/*UCHAR* img_TR = new UCHAR[width * length];
	UCHAR* img_TG = new UCHAR[width * length];
	UCHAR* img_TB = new UCHAR[width * length];*/
	UCHAR* img_TR = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* img_TG = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* img_TB = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* img_BB = NULL;
	
	if(g_pMPTI->IsMachineTypeUV())
		//img_BB = new UCHAR[width * length];
		img_BB = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);

	if(img_TR != NULL)	MakeProcImg(m_pPartImgBuf[eImg_Top_R], img_TR, m_inspCoordinate);
	if(img_TG != NULL)	MakeProcImg(m_pPartImgBuf[eImg_Top_G], img_TG, m_inspCoordinate);
	if(img_TB != NULL)	MakeProcImg(m_pPartImgBuf[eImg_Top_B], img_TB, m_inspCoordinate);
	if(img_BB != NULL)	MakeProcImg(m_pPartImgBuf[eImg_Bottom_B], img_BB, m_inspCoordinate);	

	if(img_TR != NULL)	m_procMil->SaveWorkImg(img_TR, width, length, _T("windowRed.bmp"));
	if(img_TG != NULL)	m_procMil->SaveWorkImg(img_TG, width, length, _T("windowGreen.bmp"));
	if(img_TB != NULL)	m_procMil->SaveWorkImg(img_TB, width, length, _T("windowBlue.bmp"));
	if(img_BB != NULL )	m_procMil->SaveWorkImg(img_BB, width, length, _T("windowBBlue.bmp"));

	int nCntValid = 0;
	POINTF rate = CompareXYAvg(img_TR, img_TG, img_TB, img_BB, fitR, fitG, fitB, fitB, width, length);

	bResult = TRUE;
	sRstAlgo->m_bColorXYOK_X = TRUE;
	sRstAlgo->m_bColorXYOK_Y = TRUE;

	if(sRstAlgo)
	{
		sRstAlgo->m_dRstColorXY_X = rate.x;
		sRstAlgo->m_dRstColorXY_Y = rate.y;
		sRstAlgo->m_fStdX_Min = m_pAlgoColorXY->m_fColorAvgX - m_pAlgoColorXY->m_fColorXYMinX;
		sRstAlgo->m_fStdX_Max = m_pAlgoColorXY->m_fColorAvgX + m_pAlgoColorXY->m_fColorXYMaxX;
		sRstAlgo->m_fStdY_Min = m_pAlgoColorXY->m_fColorAvgY - m_pAlgoColorXY->m_fColorXYMinY;
		sRstAlgo->m_fStdY_Max = m_pAlgoColorXY->m_fColorAvgY + m_pAlgoColorXY->m_fColorXYMaxY;
		sRstAlgo->m_bUseUV = g_pMPTI->IsMachineTypeUV();

		if(m_pAlgoColorXY->m_fColorAvgX > 0 && m_pAlgoColorXY->m_fColorAvgY > 0)
		{
			float MinX = m_pAlgoColorXY->m_fColorAvgX - m_pAlgoColorXY->m_fColorXYMinX;
			float MaxX = m_pAlgoColorXY->m_fColorAvgX + m_pAlgoColorXY->m_fColorXYMaxX;

			float MinY = m_pAlgoColorXY->m_fColorAvgY - m_pAlgoColorXY->m_fColorXYMinY;
			float MaxY = m_pAlgoColorXY->m_fColorAvgY + m_pAlgoColorXY->m_fColorXYMaxY;

			if((byInspType == WndNonCoating ? false : MinX > rate.x) || MaxX < rate.x)
			{
				bResult = FALSE;

				sRstAlgo->m_bColorXYOK_X = FALSE;
			}

			if((byInspType == WndNonCoating ? false : MinY > rate.y) || MaxY < rate.y)
			{
				bResult = FALSE;

				sRstAlgo->m_bColorXYOK_Y = FALSE;
			}
		}
		else
		{
			bResult = FALSE;
			sRstAlgo->m_bColorXYOK_X = FALSE;
			sRstAlgo->m_bColorXYOK_Y = FALSE;
		}

		// all measure 안 한 경우
		if(m_pAlgoColorXY->m_fColorAvgX == 0 && m_pAlgoColorXY->m_fColorXYMinX == 0 && m_pAlgoColorXY->m_fColorXYMaxX == 0)
		{
			bResult = FALSE;
			sRstAlgo->m_bColorXYOK_X = FALSE;
		}
		if(m_pAlgoColorXY->m_fColorAvgY == 0 && m_pAlgoColorXY->m_fColorXYMinY == 0 && m_pAlgoColorXY->m_fColorXYMaxY == 0)
		{
			bResult = FALSE;
			sRstAlgo->m_bColorXYOK_Y = FALSE;
		}
	}

// 	if(g_pMPTI->IsMachineTypeUV())
// 	{
// 		int nInspMargin = 1;
// 
// 		AlgoDebuggingMode vAlgoDebuggingMode = g_pInspMng->GetAlgoDebuggingMode(g_pMPTI->GetLaneNum());
// 
// 		if(vAlgoDebuggingMode.bAlgoDebuggingReady && vAlgoDebuggingMode.nAlgoDebuggingRemainCnt > 0 || vAlgoDebuggingMode.bReviewMode)
// 		{
// 			DefaultColorXYStd vDefaultColorXYStd = g_pInspMng->GetDefaultColorXYStd();
// 
// 			if(m_pAlgoColorXY->m_fColorAvgX != 0 && m_pAlgoColorXY->m_fColorAvgY != 0)
// 			{
// 				bool bIsIncludeNG = vAlgoDebuggingMode.bIncludeAlgoNG == true ? true : bResult;	// NG도 포함하는 모드이면 무조건 하고 아니면 OK 일때만 한다.
// 
// 				float fCurrMaxX = rate.x + vDefaultColorXYStd.fStdColorXYMaxX;
// 				float fCurrMinX = rate.x - vDefaultColorXYStd.fStdColorXYMinX;
// 
// 				float fCurrMaxY = rate.y + vDefaultColorXYStd.fStdColorXYMaxY;
// 				float fCurrMinY = rate.y - vDefaultColorXYStd.fStdColorXYMinY;
// 
// 				// is Result good?
// 				if(bIsIncludeNG)
// 				{
// 					bool bChanged = false;
// 					if(fCurrMaxX > m_pAlgoColorXY->m_fColorAvgX + m_pAlgoColorXY->m_fColorXYMaxX)	// 기존 티칭값 + 기존 MAX값 
// 					{
// 						sRstAlgo->m_fRstColorXYMaxX = fCurrMaxX - m_pAlgoColorXY->m_fColorAvgX;
// 						bChanged = true;
// 					}
// 
// 					if(fCurrMinX < m_pAlgoColorXY->m_fColorAvgX - m_pAlgoColorXY->m_fColorXYMinX)	// 기존 티칭값 - 기존 Min값 
// 					{
// 						sRstAlgo->m_fRstColorXYMinX = m_pAlgoColorXY->m_fColorAvgX - fCurrMinX;
// 						bChanged = true;
// 					}
// 
// 					if(fCurrMaxY > m_pAlgoColorXY->m_fColorAvgY + m_pAlgoColorXY->m_fColorXYMaxY)	// 기존 티칭값 + 기존 MAX값 
// 					{
// 						sRstAlgo->m_fRstColorXYMaxY = fCurrMaxY - m_pAlgoColorXY->m_fColorAvgY;
// 						bChanged = true;
// 					}
// 
// 					if(fCurrMinY < m_pAlgoColorXY->m_fColorAvgY - m_pAlgoColorXY->m_fColorXYMinY)	// 기존 티칭값 - 기존 Min값 
// 					{
// 						sRstAlgo->m_fRstColorXYMinY = m_pAlgoColorXY->m_fColorAvgY - fCurrMinY;
// 						bChanged = true;
// 					}
// 
// 					sRstAlgo->m_bRstStdChanged = bChanged;
// 				}				
// 				else
// 					sRstAlgo->m_bRstStdChanged = false;
// 			}		
// 			else
// 				sRstAlgo->m_bRstStdChanged = false;
// 		}
// 		else
// 		{
// 			sRstAlgo->m_bRstStdChanged = false;
// 		}
// 	}

	if(img_TR != NULL)
	{
		//delete[] img_TR;
		g_pMManager->pem_delete(img_TR, true);
		img_TR = NULL;
	}

	if(img_TG != NULL)
	{
		//delete[] img_TG;
		g_pMManager->pem_delete(img_TG, true);
		img_TG = NULL;
	}

	if(img_TB != NULL)
	{
		//delete[] img_TB;
		g_pMManager->pem_delete(img_TB, true);
		img_TB = NULL;
	}

	if(img_BB != NULL)
	{
		//delete[] img_BB;
		g_pMManager->pem_delete(img_BB, true);
		img_BB = NULL;
	}

	return bResult;
}

POINTF CPInsp_Color::CompareXYAvg(UCHAR* srcTR, UCHAR* srcTG, UCHAR* srcTB, UCHAR* srcBB,
	float factorTR, float factorTG, float factorTB, float factorBB, int width, int length)
{
	double ret = 0.0;
	POINTF ptfRet;

	ptfRet.x = 0.f;
	ptfRet.y = 0.f;

	if(srcTR == NULL || srcTG == NULL || srcTB == NULL || width <= 0 || length <= 0)
		return ptfRet;

	POINT tempPt;
	bool rst;

	float fSumX = 0.f;
	float fSumY = 0.f;

	int size = width * length;
	for (int i=0 ; i<size ; i++)
	{
		float tr = ((float)srcTR[i] * factorTR);		tr = _limit_value(tr, 0.f, 255.f);
		float tg = ((float)srcTG[i] * factorTG);		tg = _limit_value(tg, 0.f, 255.f);
		float tb = ((float)srcTB[i] * factorTB);		tb = _limit_value(tb, 0.f, 255.f);
		
		float bb = 0;
		
		float mergeb = tb;

		if(srcBB != NULL)
		{
			bb = ((float)srcBB[i] * factorBB);
			bb = _limit_value(bb, 0.f, 255.f);

			mergeb = _limit_value((tb * 0.2) + (bb * 0.8), 0.f, 255.f);
		}

		float cx = ((CIE_XR * (float)tr) + (CIE_XG * (float)tg) + (CIE_XB * (float)mergeb));
		float cy = ((CIE_YR * (float)tr) + (CIE_YG * (float)tg) + (CIE_YB * (float)mergeb));
		float cz = ((CIE_ZR * (float)tr) + (CIE_ZG * (float)tg) + (CIE_ZB * (float)mergeb));
		float sumXYZ = cx + cy + cz;
		if (sumXYZ)
		{
			float nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
			float ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));

			fSumX += nx;
			fSumY += (SIZE_CLRY - ny);
		}
	}

	ptfRet.x = fSumX / size;
	ptfRet.y = fSumY / size;

	return ptfRet;
}

void CPInsp_Color::MakeProcImg(void* src, UCHAR* dst, Coordinate coordi, bool bIsLoadFovRawDataImage = false)
{
	double cx = coordi.fovCx;
	double cy = coordi.fovCy;
	double roiSizeX = coordi.width;
	double roiSizeY = coordi.length;
	int roiArea = roiSizeX * roiSizeY;

	int bdCx = coordi.anyAngleCx;
	int bdCy = coordi.anyAngleCy;
	int bdSizeX = coordi.anyAngleWidth;
	int bdSizeY = coordi.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	double wndAngle = coordi.angle;

	if(src == NULL || dst == NULL)
		return;
	BOOL bIsAnyAngle = IsAnyAngle(wndAngle);

	if(!bIsAnyAngle)
	{
		int nSX = RounD(cx - (roiSizeX / 2.0));
		int nSY = RounD(cy - (roiSizeY / 2.0));

		MakeProcImg_Clip(src, dst, bIsLoadFovRawDataImage, nSX, nSY, roiSizeX, roiSizeY);
	}
	else
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;

		UCHAR* bdryImgTemp_src = NULL;
		m_procMil->ProcAnyAngle_Img(src, &bdryImgTemp_src, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);

		wndAngle = 0;

		int nSX = RounD(cx - (bdCx - (w / 2.0)) - roiSizeX / 2.0);
		int nSY = RounD(cy - (bdCy - (l / 2.0)) - roiSizeY / 2.0);

		m_procMil->GetClipImage_LT(bdryImgTemp_src, w, l, dst, nSX, nSY, roiSizeX, roiSizeY);
		//delete bdryImgTemp_src;
		g_pMManager->pem_delete(bdryImgTemp_src, false);
	}
}

void CPInsp_Color::MakeProcImg_Clip( void *pSrc, UCHAR* pDst, bool bIsLoadFovRawDataImage, int nSX, int nSY, double roiSizeX, double roiSizeY )
{
	if(bIsLoadFovRawDataImage == false)
	{
		if(pSrc != NULL && pDst != NULL)
		{
			m_procMil->GetClipBuff_LT(pSrc, pDst, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));
		}
	}
	else
	{
		if(pSrc != NULL && pDst != NULL)
		{
			m_procMil->GetClipBuff_LT_LoadRawData(pSrc, pDst, nSX, nSY, roiSizeX, roiSizeY);
			m_procMil->SaveWorkImg(pDst, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));
		}		
	}
}

void CPInsp_Color::MakeProcImg(UCHAR * src, UCHAR* dst, Coordinate coordi)
{
	int roiSizeX = coordi.width;
	int roiSizeY = coordi.length;
	int roiArea = roiSizeX * roiSizeY;

	if(src == NULL || dst == NULL)
		return;

	int nSX = coordi.cx;
	int nSY = coordi.cy;

	MakeProcImg_Clip(src, dst, nSX, nSY, roiSizeX, roiSizeY);
}

void CPInsp_Color::MakeProcImg_Clip( UCHAR * src, UCHAR* dst, int nSX, int nSY, int roiSizeX, int roiSizeY )
{
	if(src != NULL && dst != NULL)
	{
		m_procMil->GetClipImage_LT(src, m_nPartWidth, m_nPartHeight, dst, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(dst, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));
	}	
}

void CPInsp_Color::MakeProcImg_Big(UCHAR * src, UCHAR* dst, Coordinate coordi)
{
	double roiSizeX = coordi.width;
	double roiSizeY = coordi.length;
	int roiArea = roiSizeX * roiSizeY;

	double wndAngle = coordi.angle;

	if(src == NULL || dst == NULL)
		return;

	BOOL bIsAnyAngle = IsAnyAngle(wndAngle);

	if(!bIsAnyAngle)
	{
		int nSX = coordi.cx + m_nPartWidth / 2.0;		// FOV 합성 영상의 좌상단이 0,0인  Window 좌상단 좌표
		int nSY = - coordi.cy + m_nPartHeight / 2.0;

		m_procMil->GetClipImage_LT(src, m_nPartWidth, m_nPartHeight, dst, nSX, nSY, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(dst, roiSizeX, roiSizeY, _T("COLOR_dst1.bmp"));

	}
	else
	{
		double cx = 0;
		double cy = 0;
		double roiSizeX = coordi.width;
		double roiSizeY = coordi.length;

		int bdSizeX = coordi.anyAngleWidth;
		int bdSizeY = coordi.anyAngleLength;
		int bdCx = m_nPartWidth / 2. - bdSizeX / 2.;
		int bdCy = m_nPartHeight / 2. - bdSizeY / 2.;

		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;

		UCHAR* bdryImgTemp_src1 = NULL;
		m_procMil->ProcAnyAngle_Img(src, &bdryImgTemp_src1, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		
		wndAngle = 0;
		cx = (int)(coordi.cx + w / 2.0);
		cy = (int)(-coordi.cy + l / 2.0);

		m_procMil->GetClipImage(bdryImgTemp_src1, w, l, dst, cx, cy, roiSizeX, roiSizeY);
		//delete bdryImgTemp_src1;
		g_pMManager->pem_delete(bdryImgTemp_src1, false);
	}
}


int  CPInsp_Color::Color_GetColorCIEArea(int dstCieSizeX, int dstCieSizeY, RECT rcRoi, UCHAR* src_R, UCHAR* src_G, UCHAR* src_B, POINT* pt,
	float fFitR, float fFitG, float fFitB, int widthStep)
{
	part_result resInspClr = ePART_FAIL;

	try
	{
		UCHAR* colorCIE_org = NULL;
		UCHAR* blobImg = NULL;

// 		int offsetX = CIE_OFFSETX;
// 		int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);

		//////////////////////////////////////////////////////////////////////////
		//colorCIE_org = new UCHAR[SIZE_CLRX * SIZE_CLRY];
		colorCIE_org = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY, (PCHAR)__FUNCTION__, __LINE__);
		memset(colorCIE_org, 0, sizeof(UCHAR) * SIZE_CLRX * SIZE_CLRY);

		ImageToColorCIE_Rect(src_R, src_G, src_B, fFitR, fFitG, fFitB, colorCIE_org, rcRoi, widthStep);
		m_procMil->SaveWorkImg(colorCIE_org, SIZE_CLRX, SIZE_CLRY, _T("testCIE.bmp"));

		//////////////////////////////////////////////////////////////////////////
		// Final Clip
		int cieCx = (int)(/*offsetX +*/ (dstCieSizeX / 2.0));
		int cieCy = (int)(/*offsetY +*/ (dstCieSizeY / 2.0));

		//auto polygon
		//blobImg = new UCHAR[dstCieSizeX * dstCieSizeY];
		blobImg = g_pMManager->pem_new<UCHAR>(true, dstCieSizeX * dstCieSizeY, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->MakeBlobImg(colorCIE_org, blobImg, dstCieSizeX, dstCieSizeY, 1);
		m_procMil->SaveWorkImg(blobImg, dstCieSizeX, dstCieSizeY, _T("blobImg.bmp"));
		if(colorCIE_org)
		{
			//delete [] colorCIE_org;
			g_pMManager->pem_delete(colorCIE_org, true);
			colorCIE_org = NULL;
		}

		m_procMil->GetPolygonCoordi_Pt7(blobImg, dstCieSizeX, dstCieSizeY, 0, pt); //auto polygon margin
		//RemakePoly(pt, 7, -offsetX, -offsetY, pt); //Origin좌표로 돌린다.

		if(blobImg)
		{
			//delete [] blobImg;
			g_pMManager->pem_delete(blobImg, true);
			blobImg = NULL;
		}
		//////////////////////////////////////////////////////////////////////////

		resInspClr = ePART_SUCCESS;
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
	return resInspClr;
}

BOOL CPInsp_Color::GetColorBaseCIE(InspAlgo &sInspAlgo, AlgoColorOpt &sAlgoColorOpt, UCHAR* retDstImg, RstAlgoColorOpt& pRstData)
{
	BOOL bResult = FALSE;
	try
	{
		if(sInspAlgo.m_eAlgoType != eAlgoColor)
			return bResult;
		if(!m_pProcMilAlgo)
			return bResult;
		if(retDstImg == NULL)
			return bResult;
		if(sAlgoColorOpt.m_sImgBuf.imgTop_R == NULL || sAlgoColorOpt.m_sImgBuf.imgTop_G == NULL || sAlgoColorOpt.m_sImgBuf.imgTop_B == NULL)
			return bResult;
		if(sAlgoColorOpt.m_sImgBuf.nImageSizeX <= 0 || sAlgoColorOpt.m_sImgBuf.nImageSizeY <= 0)
			return bResult;
		AlgoColorBase *pInspAlgoColor = (AlgoColorBase *)sInspAlgo.m_ptrInspAlgoParam;
		if(!pInspAlgoColor)
			return bResult;

		int nImgWidth = sAlgoColorOpt.m_sImgBuf.nImageSizeX;
		int nImgHeight = sAlgoColorOpt.m_sImgBuf.nImageSizeY;
		/*UCHAR* img_R = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* img_G = new UCHAR[nImgWidth * nImgHeight];
		UCHAR* img_B = new UCHAR[nImgWidth * nImgHeight];*/
		UCHAR* img_R = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_G = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_B = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_BR =sAlgoColorOpt.m_sImgBuf.imgBottom_R;
		UCHAR* img_BB =sAlgoColorOpt.m_sImgBuf.imgBottom_B;
		memset(img_R, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
		memset(img_G, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
		memset(img_B, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
		cv::Mat imgG;
		if(pInspAlgoColor->m_bUseAngleColor)
		{
			CPInsp_Algo &InsAlgo = g_pInspMng->GetInspAlgo();
			LightTypeBuf sLightBuf;
			sLightBuf.m_pucTRed = sAlgoColorOpt.m_sImgBuf.imgTop_R;
			sLightBuf.m_pucTGreen = sAlgoColorOpt.m_sImgBuf.imgTop_G;
			sLightBuf.m_pucTBlue = sAlgoColorOpt.m_sImgBuf.imgTop_B;
			sLightBuf.m_pucTWhite = sAlgoColorOpt.m_sImgBuf.imgTop_W;
			sLightBuf.m_pucMRed = sAlgoColorOpt.m_sImgBuf.imgMiddle_R;
			imgG = GetGreenImage(sAlgoColorOpt.m_sImgBuf.imgMiddle_R, sAlgoColorOpt.m_sImgBuf.imgMiddle_B, nImgWidth, nImgHeight);
			sLightBuf.m_pucMGreen = imgG.data;
			sLightBuf.m_pucMBlue = sAlgoColorOpt.m_sImgBuf.imgMiddle_B;
			sLightBuf.m_pucMWhite = NULL;
			sLightBuf.m_pucBRed = sAlgoColorOpt.m_sImgBuf.imgBottom_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = sAlgoColorOpt.m_sImgBuf.imgBottom_B;
			sLightBuf.m_pucBWhite = NULL;
			sLightBuf.m_nImgWidth = nImgWidth;
			sLightBuf.m_nImgHeight = nImgHeight;
			sLightBuf.m_nROIImgWidth = nImgWidth;
			sLightBuf.m_nROIImgHeight = nImgHeight;
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
				if(a == 0)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_R);
				else if(a == 1)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_G);
				else if(a == 2)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_B);
			}
			Delete_1DArray(&sLightBuf.m_pnRedValue);
			Delete_1DArray(&sLightBuf.m_pnGreenValue);
			Delete_1DArray(&sLightBuf.m_pnBlueValue);
			Delete_1DArray(&sLightBuf.m_pnWhiteValue);
			Delete_1DArray(&sLightBuf.m_pnPosition);
			Delete_1DArray(&sLightBuf.m_pnCalculation);
		}
		else
		{
			if (pInspAlgoColor->m_byColorLightType == 1 && sAlgoColorOpt.m_sImgBuf.imgMiddle_R != NULL && sAlgoColorOpt.m_sImgBuf.imgMiddle_B != NULL)
			{
				memcpy(img_R, sAlgoColorOpt.m_sImgBuf.imgMiddle_R, sizeof(UCHAR)* nImgWidth * nImgHeight);
				memcpy(img_B, sAlgoColorOpt.m_sImgBuf.imgMiddle_B, sizeof(UCHAR)* nImgWidth * nImgHeight);
				imgG = GetGreenImage(img_R, img_B, nImgWidth, nImgHeight);
				memcpy(img_G, imgG.data, sizeof(UCHAR)* nImgWidth * nImgHeight);
			}
			else
			{
				memcpy(img_R, sAlgoColorOpt.m_sImgBuf.imgTop_R, sizeof(UCHAR)* nImgWidth * nImgHeight);
				memcpy(img_G, sAlgoColorOpt.m_sImgBuf.imgTop_G, sizeof(UCHAR)* nImgWidth * nImgHeight);
				memcpy(img_B, sAlgoColorOpt.m_sImgBuf.imgTop_B, sizeof(UCHAR)* nImgWidth * nImgHeight);
			}
		}
		m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R.bmp"));
		m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G.bmp"));
		m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B.bmp"));
		if(img_BR != NULL)
			m_procMil->SaveWorkImg(img_BR, nImgWidth, nImgHeight, _T("Img_BR.bmp"));
		if(img_BB != NULL)
			m_procMil->SaveWorkImg(img_BB, nImgWidth, nImgHeight, _T("Img_BB.bmp"));
		
		float fitR = m_fFatorR == 0.0 ?    (float)1.0 : m_fFatorR;
		float fitG = m_fFatorG == 0.0 ?    (float)1.0 : m_fFatorG;
		float fitB = m_fFatorB == 0.0 ?    (float)1.0 : m_fFatorB;
		float fitBR = m_fFatorBR == 0.0 ?    (float)1.0 : m_fFatorBR;
		float fitBB = m_fFatorBB == 0.0 ?    (float)1.0 : m_fFatorBB;
		float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmB;
		int nCompoMode = g_pMPTI->GetCompositeLightMode();
		if(pInspAlgoColor->m_bUseAngleColor)
		{
			fitR = 1.0f;
			fitG = 1.0f;
			fitB = 1.0f;
		}
		if(sAlgoColorOpt.m_sImgBuf.imgSide1_B!=NULL || sAlgoColorOpt.m_sImgBuf.imgSide1_B!=NULL || sAlgoColorOpt.m_sImgBuf.imgSide3_B!=NULL || sAlgoColorOpt.m_sImgBuf.imgSide4_B!=NULL)
		{
			fitR = 1;
			fitG = 1;
			fitB = 1;
		}
		
		//UCHAR* dispCIE_org = new UCHAR[SIZE_CLRX * SIZE_CLRY * 3];
		UCHAR* dispCIE_org = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY * 3, (PCHAR)__FUNCTION__, __LINE__);
		int dstCieSizeX = sAlgoColorOpt.m_nCIEWidth;
		int dstCieSizeY = sAlgoColorOpt.m_nCIEHeight;
		int cieCx = (int)(dstCieSizeX / 2.0);
		int cieCy = (int)(dstCieSizeY / 2.0);
		memcpy(dispCIE_org, m_dispCIE_org, sizeof(UCHAR)* SIZE_CLRX * SIZE_CLRY * 3);
		int nCntValid = 0;
		double dWhiteA = 0;
		if(pRstData.m_dRate == 0)
		{
			if(nCompoMode == 0)
				ImageToColorCIE_Color(img_R, img_G, img_B, fitR, fitG, fitB, dispCIE_org, nImgWidth, nImgHeight);
			else
				ImageToColorCIE_Color_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, dispCIE_org, nImgWidth, nImgHeight);
		}
		else
		{
			int nIndex = pRstData.m_dRate - 1;
			POINT pPtPolygon[POLYGON_POINT_CNTS];
			int offsetX = CIE_OFFSETX;
			int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
			RemakePoly(pInspAlgoColor->m_ptArrPolygon[nIndex], POLYGON_POINT_CNTS, offsetX, offsetY, pPtPolygon);
			if(nCompoMode == 0)
				pRstData.m_dRate = CompareCIE(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, pPtPolygon, POLYGON_POINT_CNTS, nCntValid, dWhiteA);
			else
				pRstData.m_dRate = CompareCIE_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, nImgWidth, nImgHeight, pPtPolygon, POLYGON_POINT_CNTS, nCntValid, dWhiteA);
		}
		
		m_procMil->SaveWorkImg(dispCIE_org, SIZE_CLRX, SIZE_CLRY, _T("testCIE.bmp"), 3);

		//UCHAR* dispCIE = new UCHAR[dstCieSizeX * dstCieSizeY * 3];
		UCHAR* dispCIE = g_pMManager->pem_new<UCHAR>(true, dstCieSizeX * dstCieSizeY * 3, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->ImageClip_CIE(dispCIE_org, SIZE_CLRX, SIZE_CLRY, 3, dispCIE, cieCx, cieCy, dstCieSizeX, dstCieSizeY);
		m_procMil->SaveWorkImg(dispCIE, dstCieSizeX, dstCieSizeY, _T("testCIE2.bmp"), 3);
		if(dispCIE_org)
		{
			//delete [] dispCIE_org;
			g_pMManager->pem_delete(dispCIE_org, true);
			dispCIE_org = NULL;
		}
		if(retDstImg != NULL)
		{
			memset(retDstImg, 0, dstCieSizeX * dstCieSizeY * sizeof(UCHAR));
			memcpy_s(retDstImg, sizeof(UCHAR) * dstCieSizeX * dstCieSizeY * 3, dispCIE, sizeof(UCHAR) * dstCieSizeX * dstCieSizeY * 3);
		m_procMil->SaveWorkImg(retDstImg, dstCieSizeX, dstCieSizeY, _T("testCIE3.bmp"), 3);
		}
		if(dispCIE)
		{
			//delete [] dispCIE;
			g_pMManager->pem_delete(dispCIE, true);
			dispCIE = NULL;
		}

		if(sAlgoColorOpt.m_bUseRangeBar == TRUE)
			AutoTeachingColor(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, pRstData);
		Delete_1DArray(&img_R);
		Delete_1DArray(&img_G);
		Delete_1DArray(&img_B);
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
	return bResult;
}

BOOL CPInsp_Color::GetColorBaseBin(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg, bool bTeach, int nInspAC, UCHAR* retDstImgAC, int nIDX, int nSideCameraIndex, TotalInspExceptArea *stTieArea, bool bSaveR)
{
	BOOL bResult = FALSE;
	try
	{
		if(!m_pProcMilAlgo)
			return bResult;
		if(pInspAlgoColor == NULL)
			return bResult;
		if(sInspRoiImg.imgTop_R == NULL || sInspRoiImg.imgTop_G == NULL || sInspRoiImg.imgTop_B == NULL)
			return bResult;
		if(sInspRoiImg.nImageSizeX <= 0 || sInspRoiImg.nImageSizeY <= 0)
			return bResult;

		bool bInspAC = ((nInspAC & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
		int nImgWidth = sInspRoiImg.nImageSizeX;
		int nImgHeight = sInspRoiImg.nImageSizeY;
		UCHAR* img_R = sInspRoiImg.imgTop_R;
		UCHAR* img_G = sInspRoiImg.imgTop_G;
		UCHAR* img_B = sInspRoiImg.imgTop_B;
		UCHAR* img_BR =sInspRoiImg.imgBottom_R;
		UCHAR* img_BB = sInspRoiImg.imgBottom_B;
		cv::Mat imgG;
		if (bTeach && pInspAlgoColor->m_byColorLightType == 1 && pInspAlgoColor->m_bUseAngleColor == FALSE && sInspRoiImg.imgMiddle_R != NULL && sInspRoiImg.imgMiddle_B != NULL)
		{
			img_R = sInspRoiImg.imgMiddle_R;
			img_B = sInspRoiImg.imgMiddle_B;
			imgG = GetGreenImage(img_R, img_B, nImgWidth, nImgHeight);
			img_G = imgG.ptr<UCHAR>();
		}

		if(nSideCameraIndex == 0)	// 사이드 카메라 1
		{
			if (sInspRoiImg.imgSide1_R != NULL &&
				sInspRoiImg.imgSide1_G != NULL &&
				sInspRoiImg.imgSide1_B != NULL)
		{
			img_R = sInspRoiImg.imgSide1_R;
			img_G = sInspRoiImg.imgSide1_G;
				img_B = sInspRoiImg.imgSide1_B;
			}
		}
		else if(nSideCameraIndex == 1)	// 사이드 카메라 2
		{
			if (sInspRoiImg.imgSide2_R != NULL &&
				sInspRoiImg.imgSide2_G != NULL &&
				sInspRoiImg.imgSide2_B != NULL)
			{
				img_R = sInspRoiImg.imgSide2_R;
				img_G = sInspRoiImg.imgSide2_G;
				img_B = sInspRoiImg.imgSide2_B;
			}
		}
		else if(nSideCameraIndex == 2)	// 사이드 카메라 3
		{
			if (sInspRoiImg.imgSide3_R != NULL &&
				sInspRoiImg.imgSide3_G != NULL &&
				sInspRoiImg.imgSide3_B != NULL)
			{
				img_R = sInspRoiImg.imgSide3_R;
				img_G = sInspRoiImg.imgSide3_G;
				img_B = sInspRoiImg.imgSide3_B;
			}
		}
		else if(nSideCameraIndex == 3)	// 사이드 카메라 4
		{
			if (sInspRoiImg.imgSide4_R != NULL &&
				sInspRoiImg.imgSide4_G != NULL &&
				sInspRoiImg.imgSide4_B != NULL)
			{
				img_R = sInspRoiImg.imgSide4_R;
				img_G = sInspRoiImg.imgSide4_G;
				img_B = sInspRoiImg.imgSide4_B;
			}
		}
		if (bInspAC && retDstImgAC && bTeach == false)
		{
			img_R = sInspRoiImg.imgMiddle_R;
			img_G = sInspRoiImg.imgTop_W;
			img_B = sInspRoiImg.imgMiddle_B;
		}

#if _DEBUG
		cv::Mat imgRR(nImgHeight, nImgWidth, CV_8UC1, img_R);
		cv::Mat imgGG(nImgHeight, nImgWidth, CV_8UC1, img_G);
		cv::Mat imgBB(nImgHeight, nImgWidth, CV_8UC1, img_B);
#endif
		bool bAC = (pInspAlgoColor->m_bUseAngleColor || bInspAC);
		if (bTeach && bAC)
		{
			LightTypeBuf sLightBuf;
			sLightBuf.m_pucTRed = sInspRoiImg.imgTop_R;
			sLightBuf.m_pucTGreen = sInspRoiImg.imgTop_G;
			sLightBuf.m_pucTBlue = sInspRoiImg.imgTop_B;
			sLightBuf.m_pucTWhite = sInspRoiImg.imgTop_W;
			sLightBuf.m_pucMRed = sInspRoiImg.imgMiddle_R;
			imgG = GetGreenImage(sInspRoiImg.imgMiddle_R, sInspRoiImg.imgMiddle_B, nImgWidth, nImgHeight);
			sLightBuf.m_pucMGreen = imgG.data;
			sLightBuf.m_pucMBlue = sInspRoiImg.imgMiddle_B;
			sLightBuf.m_pucMWhite = NULL;
			sLightBuf.m_pucBRed = sInspRoiImg.imgBottom_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = sInspRoiImg.imgBottom_B;
			sLightBuf.m_pucBWhite = NULL;
			sLightBuf.m_nImgWidth = nImgWidth;
			sLightBuf.m_nImgHeight = nImgHeight;
			sLightBuf.m_nROIImgWidth = nImgWidth;
			sLightBuf.m_nROIImgHeight = nImgHeight;
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
			CPInsp_Algo &InsAlgo = g_pInspMng->GetInspAlgo();
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
				if(a == 0)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_R);
				else if(a == 1)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_G);
				else if(a == 2)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_B);
			}
			Delete_1DArray(&sLightBuf.m_pnRedValue);
			Delete_1DArray(&sLightBuf.m_pnGreenValue);
			Delete_1DArray(&sLightBuf.m_pnBlueValue);
			Delete_1DArray(&sLightBuf.m_pnWhiteValue);
			Delete_1DArray(&sLightBuf.m_pnPosition);
			Delete_1DArray(&sLightBuf.m_pnCalculation);

			m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R_AC.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G_AC.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B_AC.bmp"), 1, false, bSaveR);
		}
		else
		{
			m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B.bmp"), 1, false, bSaveR);
			if (img_BR != NULL && img_BR == nullptr)
				m_procMil->SaveWorkImg(img_BR, nImgWidth, nImgHeight, _T("Img_BR.bmp"), 1, false, bSaveR);
			if (img_BB != NULL && img_BB == nullptr)
				m_procMil->SaveWorkImg(img_BB, nImgWidth, nImgHeight, _T("Img_BB.bmp"), 1, false, bSaveR);
		}
		if(retDstImg != NULL)
			memset(retDstImg, 0, nImgWidth * nImgHeight * sizeof(UCHAR));

		float fitR = m_fFatorR == 0.0 ?    (float)1.0 : m_fFatorR;
		float fitG = m_fFatorG == 0.0 ?    (float)1.0 : m_fFatorG;
		float fitB = m_fFatorB == 0.0 ?    (float)1.0 : m_fFatorB;
		float fitBR = m_fFatorBR == 0.0 ?    (float)1.0 : m_fFatorBR;
		float fitBB = m_fFatorBB == 0.0 ?    (float)1.0 : m_fFatorBB;
		float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmB;
		int nCompoMode = g_pMPTI->GetCompositeLightMode();
		if(bAC)
		{
			fitR = 1.0f;
			fitG = 1.0f;
			fitB = 1.0f;
		}
		//if(sInspRoiImg.imgSide1_B!=NULL || sInspRoiImg.imgSide2_B!=NULL || sInspRoiImg.imgSide3_B!=NULL || sInspRoiImg.imgSide4_B!=NULL)
		if(nSideCameraIndex >= 0 && nSideCameraIndex <= 3)
		{
			fitR = 1;
			fitG = 1;
			fitB = 1;
		}
		int nRet = 0;
		if (bInspAC && retDstImgAC)
		{
			nRet = GetAngleColor(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, retDstImgAC);
			m_procMil->SaveWorkImg(retDstImgAC, nImgWidth, nImgHeight, _T("Rst_Color_Image_AC.bmp"), 3, false, bSaveR);
			if(nRet == eMR_FAIL)
				bResult = FALSE;
		}
		if (pInspAlgoColor->m_bUseColor == FALSE)
		{
			if(nRet == eMR_FAIL) bResult = FALSE;
			return bResult;
		}
		double dWhiteA = 0;
		if(pInspAlgoColor->m_byType == 1)
		{
			double dRate = 0;
			nRet = GetColorBin(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, pInspAlgoColor, retDstImg, &dRate, dWhiteA);
			if(nRet == eMR_FAIL)
				bResult = FALSE;
		}
		else
		{
			//UCHAR* ptrImgBuf = new UCHAR[nImgWidth * nImgHeight];
			UCHAR* ptrImgBuf = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
			int offsetX = CIE_OFFSETX;
			int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
			POINT pPtPolygon[POLYGON_POINT_CNTS];
			bResult = TRUE;
			for (int nCnt = 0; nCnt < pInspAlgoColor->m_nPolygonCnt; nCnt++)
			{
				if (nIDX > -1 && nIDX != nCnt)
					continue;
				memset(ptrImgBuf, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
				RemakePoly(pInspAlgoColor->m_ptArrPolygon[nCnt], POLYGON_POINT_CNTS, offsetX, offsetY, pPtPolygon);

				if (nCompoMode == 0)
				{
					nRet = GetSelectColorCIEImg(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, pPtPolygon, POLYGON_POINT_CNTS, ptrImgBuf);

					CString csFileName = _T("");
					csFileName.Format(_T("Color_Image_Buf_%d.bmp"), nCnt);
					m_procMil->SaveWorkImg(ptrImgBuf, nImgWidth, nImgHeight, csFileName, 1, false, bSaveR);
				}
				else
				{
					nRet = GetSelectColorCIEImg_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB,
						nImgWidth, nImgHeight, pPtPolygon, POLYGON_POINT_CNTS, ptrImgBuf);

					CString csFileName = _T("");
					csFileName.Format(_T("Color_Image_Buf_BTM_%d.bmp"), nCnt);
					m_procMil->SaveWorkImg(ptrImgBuf, nImgWidth, nImgHeight, _T("Color_Image_Buf_BTM.bmp"), 1, false, bSaveR);
				}
				if(nRet == eMR_FAIL)
				{
					bResult = FALSE;
					break;
				}

				for (int n = 0; n < nImgWidth * nImgHeight; n++)
				{
					if(retDstImg[n] == 255 || ptrImgBuf[n] == 255)
						retDstImg[n] = (pInspAlgoColor->m_bInvert == FALSE) ? 255 : 0;
					else
						retDstImg[n] = (pInspAlgoColor->m_bInvert == FALSE) ? 0 : 255;
				}
			}
			Delete_1DArray(&ptrImgBuf);
		}

		if (pInspAlgoColor->m_bUseIntensity)
		{
			UCHAR* ucIntensMask = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
			memset(ucIntensMask, 255, nImgWidth * nImgHeight);

			BOOL bUseSide = nCompoMode == 0 ? FALSE : TRUE;
			BOOL bCalcMask = CalcIntensityMask(img_R, img_G, img_B, img_BR, img_BB, nImgWidth, nImgHeight, ucIntensMask,
				pInspAlgoColor->m_nRangeType, pInspAlgoColor->m_nRangeMin, pInspAlgoColor->m_nRangeMax, bUseSide);

			cv::Mat ColorDst(nImgHeight, nImgWidth, CV_8UC1, retDstImg);
			cv::Mat ColorMask(nImgHeight, nImgWidth, CV_8UC1, ucIntensMask);

			cv::bitwise_and(ColorDst, ColorMask, ColorDst);

			g_pMManager->pem_delete(ucIntensMask, true);
		}

		m_procMil->SaveWorkImg(retDstImg, nImgWidth, nImgHeight, _T("Rst_Color_Image.bmp"), 1, false, bSaveR);

		if (stTieArea)
		{
			stTieArea->m_nUsedInspPolygon = 0;
			stTieArea->m_nUsedMaskingValue = 0;
			CPInsp::FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, retDstImg, NULL, *stTieArea);
			m_procMil->SaveWorkImg(retDstImg, nImgWidth, nImgHeight, _T("Rst_Color_Image_Poly.bmp"), 1, false, bSaveR);
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
	return bResult;
}
BOOL CPInsp_Color::GetColorFromXYZ(POINTF ptfLocation, int* arrRGB)
{
	try
	{
		return GetSelectColorFromXYZ(ptfLocation, arrRGB);
	}
	catch (CMemoryException* e)
	{
		return FALSE;
	}
	catch (CFileException* e)
	{
		return FALSE;
	}
	catch (CException* e)
	{
		return FALSE;
	}
}
BOOL CPInsp_Color::GetColorImg(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg)
{
	BOOL bResult = FALSE;
	try
	{
		if(!m_pProcMilAlgo)
			return bResult;
		if(pInspAlgoColor == NULL)
			return bResult;

		int nImgWidth = sInspRoiImg.nImageSizeX;
		int nImgHeight = sInspRoiImg.nImageSizeY;
		UCHAR* img_R = sInspRoiImg.imgTop_R;
		UCHAR* img_G = sInspRoiImg.imgTop_G;
		UCHAR* img_B = sInspRoiImg.imgTop_B;
		UCHAR* img_BR =sInspRoiImg.imgBottom_R;
		UCHAR* img_BB =sInspRoiImg.imgBottom_B;

		if(img_R == NULL || img_G == NULL || img_B == NULL)
			return bResult;
		if(nImgWidth <= 0 || nImgHeight <= 0)
			return bResult;

		m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R.bmp"));
		m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G.bmp"));
		m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B.bmp"));
		if(img_BR != NULL && img_BR == nullptr)
			m_procMil->SaveWorkImg(img_BR, nImgWidth, nImgHeight, _T("Img_BR.bmp"));
		if(img_BB != NULL&& img_BB == nullptr)
			m_procMil->SaveWorkImg(img_BB, nImgWidth, nImgHeight, _T("Img_BB.bmp"));

		float fitR = m_fFatorR == 0.0 ?    (float)1.0 : m_fFatorR;
		float fitG = m_fFatorG == 0.0 ?    (float)1.0 : m_fFatorG;
		float fitB = m_fFatorB == 0.0 ?    (float)1.0 : m_fFatorB;
		float fitBR = m_fFatorBR == 0.0 ?    (float)1.0 : m_fFatorBR;
		float fitBB = m_fFatorBB == 0.0 ?    (float)1.0 : m_fFatorBB;
		float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmB;
		int nCompoMode = g_pMPTI->GetCompositeLightMode();

		if(sInspRoiImg.imgSide1_B!=NULL || sInspRoiImg.imgSide2_B!=NULL || sInspRoiImg.imgSide3_B!=NULL || sInspRoiImg.imgSide4_B!=NULL)
		{
			fitR = 1;
			fitG = 1;
			fitB = 1;
		}
		int nRet = 0;

		bResult = TRUE;
		for (int nCnt = 0; nCnt < pInspAlgoColor->m_nPolygonCnt; nCnt++)
		{
			if(nCompoMode == 0)
				nRet = GetSelectColorImg(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, retDstImg);
			else
				nRet = GetSelectColorImg_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, nImgWidth, nImgHeight, retDstImg);
			if(nRet == eMR_FAIL)
			{
				bResult = FALSE;
				break;
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
	return bResult;
}

BOOL CPInsp_Color::GetGrayMeanImg(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg)
{
	BOOL bResult = FALSE;
	try
	{
		if(!m_pProcMilAlgo)
			return bResult;
		if(pInspAlgoColor == NULL)
			return bResult;

		int nImgWidth = sInspRoiImg.nImageSizeX;
		int nImgHeight = sInspRoiImg.nImageSizeY;
		UCHAR* img_R = sInspRoiImg.imgTop_R;
		UCHAR* img_G = sInspRoiImg.imgTop_G;
		UCHAR* img_B = sInspRoiImg.imgTop_B;
		UCHAR* img_BR =sInspRoiImg.imgBottom_R;
		UCHAR* img_BB =sInspRoiImg.imgBottom_B;

		if(img_R == NULL || img_G == NULL || img_B == NULL)
			return bResult;
		if(nImgWidth <= 0 || nImgHeight <= 0)
			return bResult;

		m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R.bmp"));
		m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G.bmp"));
		m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B.bmp"));
		if(img_BR != NULL && img_BR == nullptr)
			m_procMil->SaveWorkImg(img_BR, nImgWidth, nImgHeight, _T("Img_BR.bmp"));
		if(img_BB != NULL&& img_BB == nullptr)
			m_procMil->SaveWorkImg(img_BB, nImgWidth, nImgHeight, _T("Img_BB.bmp"));

		float fitR = m_fFatorR == 0.0 ?    (float)1.0 : m_fFatorR;
		float fitG = m_fFatorG == 0.0 ?    (float)1.0 : m_fFatorG;
		float fitB = m_fFatorB == 0.0 ?    (float)1.0 : m_fFatorB;
		float fitBR = m_fFatorBR == 0.0 ?    (float)1.0 : m_fFatorBR;
		float fitBB = m_fFatorBB == 0.0 ?    (float)1.0 : m_fFatorBB;
		float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ?    (float)1.0 : g_pMPTI->m_fCompoBtmB;
		int nCompoMode = g_pMPTI->GetCompositeLightMode();

		if(sInspRoiImg.imgSide1_B!=NULL || sInspRoiImg.imgSide2_B!=NULL || sInspRoiImg.imgSide3_B!=NULL || sInspRoiImg.imgSide4_B!=NULL)
		{
			fitR = 1;
			fitG = 1;
			fitB = 1;
		}
		int nRet = 0;

		bResult = TRUE;
		for (int nCnt = 0; nCnt < pInspAlgoColor->m_nPolygonCnt; nCnt++)
		{
			if(nCompoMode == 0)
				nRet = GetSelectGrayMeanImg(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, retDstImg);
			else
				nRet = GetSelectGrayMeanImg_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, nImgWidth, nImgHeight, retDstImg);
			if(nRet == eMR_FAIL)
			{
				bResult = FALSE;
				break;
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
	return bResult;
}

int CPInsp_Color::GetSelectColorImg(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, UCHAR* retImg)
{
	if(srcR == NULL ||srcG == NULL ||srcB == NULL ||retImg == NULL || sizeX < 1 || sizeY < 1)
		return ePART_FAIL;


	CPoint tempPt;
	int index = 0, idx2d=0;
	BOOL rst = FALSE;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{
			index = (y * sizeX) + x;
			idx2d = (y * (sizeX*3)) + (x*3);
			float r = ((float)srcR[index] * fitR);		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)srcG[index] * fitG);		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)srcB[index] * fitB);		b = _limit_value(b, 0.f, 255.f);

			retImg[idx2d+0] = b;
			retImg[idx2d+1] = g;
			retImg[idx2d+2] = r;
		}
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::GetSelectGrayMeanImg(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, UCHAR* retImg)
{
	if(srcR == NULL ||srcG == NULL ||srcB == NULL ||retImg == NULL || sizeX < 1 || sizeY < 1)
		return ePART_FAIL;


	CPoint tempPt;
	int index = 0, idx2d=0;
	BOOL rst = FALSE;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{
			index = (y * sizeX) + x;
			float r = ((float)srcR[index] * fitR);		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)srcG[index] * fitG);		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)srcB[index] * fitB);		b = _limit_value(b, 0.f, 255.f);

			retImg[index] = (UCHAR)((r+b+g) / 3.0);
		}
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::GetSelectColorImg_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float fitR, float fitG, float fitB, float fitBR, float fitBB, 
	float compoBtmR, float compoBtmG, float compoBtmB, int sizeX, int sizeY, UCHAR* retImg)
{
	if(srcR == NULL ||srcG == NULL ||srcB == NULL ||retImg == NULL || sizeX < 1 || sizeY < 1 || srcBR == NULL || srcBB == NULL)
		return ePART_FAIL;


	CPoint tempPt;
	int index = 0, idx2d=0;
	BOOL rst = FALSE;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{
			index = (y * sizeX) + x;
			idx2d = (y * (sizeX*3)) + (x*3);
			float fBR = (float)(srcBR[index] * fitBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
			float fBB = (float)(srcBB[index] * fitBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

			float fBGR = 0.4f;
			float fBGB = 0.8f;

			float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

			float r = ((float)srcR[index] * fitR) + fBR;		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)srcG[index] * fitG) + fBG;		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)srcB[index] * fitB) + fBB;		b = _limit_value(b, 0.f, 255.f);

			retImg[idx2d+0] = b;
			retImg[idx2d+1] = g;
			retImg[idx2d+2] = r;
		}
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::GetSelectGrayMeanImg_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float fitR, float fitG, float fitB, float fitBR, float fitBB, 
	float compoBtmR, float compoBtmG, float compoBtmB, int sizeX, int sizeY, UCHAR* retImg)
{
	if(srcR == NULL ||srcG == NULL ||srcB == NULL ||retImg == NULL || sizeX < 1 || sizeY < 1 || srcBR == NULL || srcBB == NULL)
		return ePART_FAIL;


	CPoint tempPt;
	int index = 0, idx2d=0;
	BOOL rst = FALSE;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{
			index = (y * sizeX) + x;

			float fBR = (float)(srcBR[index] * fitBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
			float fBB = (float)(srcBB[index] * fitBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

			float fBGR = 0.4f;
			float fBGB = 0.8f;

			float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

			float r = ((float)srcR[index] * fitR) + fBR;		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)srcG[index] * fitG) + fBG;		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)srcB[index] * fitB) + fBB;		b = _limit_value(b, 0.f, 255.f);

			retImg[index] = (UCHAR)((r+b+g) / 3.0);
		}
	}

	return ePART_SUCCESS;
}

int CPInsp_Color::GetColorBin(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, AlgoColorBase *pInspAlgoColor, UCHAR* retDstImg, double* pRetRate, double dWhiteA)
{
	*pRetRate = 0;
	if(srcR == NULL ||srcG == NULL ||srcB == NULL ||retDstImg == NULL || pInspAlgoColor == NULL || sizeX < 1 || sizeY < 1)
		return ePART_FAIL;
	if(pInspAlgoColor->m_bUseRGB[0] == false && pInspAlgoColor->m_bUseRGB[1] == false
		&& pInspAlgoColor->m_bUseRGB[2] == false)
		return ePART_FAIL;
#if false
	UCHAR* ucColorImage = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImage, sizeX * sizeY * 3);
	memset(ucColorImage, 0, sizeX * sizeY * 3 * sizeof(UCHAR));
	int nIdx = 0;
	for (int y = 0; y < sizeY; y++)
	{
		for (int x = 0; x < sizeX; x++)
		{
			int nIndex = (y * sizeX) + x;
			ucColorImage[nIdx] = srcB[nIndex];
			nIdx++;
			ucColorImage[nIdx] = srcG[nIndex];
			nIdx++;
			ucColorImage[nIdx] = srcR[nIndex];
			nIdx++;
		}
	}
 	m_procMil->SaveWorkImg(ucColorImage, sizeX, sizeY, _T("ColorImage.bmp"), 3);
	Delete_1DArray(&ucColorImage);
#endif
	double dDataBW = 0;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x = 0 ; x < sizeX ; x++)
		{
			int nIndex = (y * sizeX) + x;
			float fR = ((float)srcR[nIndex] * fitR);
			float fG = ((float)srcG[nIndex] * fitG);
			float fB = ((float)srcB[nIndex] * fitB);

			int nWhite = 0;
			for (int a = 0 ; a < 3 ; a++)
			{
				if (pInspAlgoColor->m_bUseRGB[a] == false)
				{
					nWhite++;
					continue;
				}

				byte byRange = pInspAlgoColor->m_byRange[a];
				float fMin = pInspAlgoColor->m_byMin[a];
				float fMax = pInspAlgoColor->m_byMax[a];
				fMin = _limit_value(fMin, 0, 255);
				fMax = _limit_value(fMax, 0, 255);
				if (fMin > fMax) fMax = fMin;

				float fValue = fR;
				if (a == 1)
					fValue = fG;
				else if (a == 2)
					fValue = fB;

				bool bIn = false;
				if ((byRange == 0 && fValue >= fMin && fValue <= fMax) ||
					(byRange == 1 && (fValue <= fMin || fValue >= fMax)) ||
					(byRange == 2 && fValue >= fMax) ||
					(byRange == 3 && fValue <= fMin))
					bIn = true;

				if (bIn == false)
					continue;

				nWhite++;
			}

			bool bWhite = (nWhite == 3);
			if ((pInspAlgoColor->m_bInvert == TRUE && bWhite == false) ||
				(pInspAlgoColor->m_bInvert == FALSE && bWhite == true))
			{
				retDstImg[nIndex] = 255;
				dDataBW++;
			}
		}
	}
	if (dWhiteA > 0 && sizeX * sizeY > dWhiteA)
		*pRetRate = (dDataBW / dWhiteA) * 100.0;
	else
	*pRetRate = (dDataBW / (double)(sizeX * sizeY)) * 100.0;

	return ePART_SUCCESS;
}

int CPInsp_Color::AutoTeachingColor(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, RstAlgoColorOpt &pRst)
{
	if(srcR == NULL ||srcG == NULL ||srcB == NULL || sizeX < 1 || sizeY < 1)
		return ePART_FAIL;
	for(int y = 0; y < sizeY; y++)
	{
		for (int x = 0 ; x < sizeX ; x++)
		{
			int nIndex = (y * sizeX) + x;
			float fR = ((float)srcR[nIndex] * fitR);
			float fG = ((float)srcG[nIndex] * fitG);
			float fB = ((float)srcB[nIndex] * fitB);
			for (int a = 0 ; a < 3 ; a++)
			{
				if(a == 0 && (fR < fG || fR < fB))
					continue;
				else if(a == 1 && (fG < fR || fG < fB))
					continue;
				else if(a == 2 && (fB < fR || fB < fG))
					continue;
				pRst.m_bUseRGB[a] = true;
				float fValue = 0.0f;
				if(a == 0)
					fValue = fR;
				else if(a == 1)
					fValue = fG;
				else if(a == 2)
					fValue = fB;
				fValue = _limit_value(fValue, 0.f, 255.f);
				if(pRst.m_byMax[a] < fValue)
					pRst.m_byMax[a] = fValue;
			}
		}
	}
	return ePART_SUCCESS;
}
int CPInsp_Color::GetAngleColor(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float fitR, float fitG, float fitB, int nImgWidth, int nImgHeight, UCHAR* retDstImg)
{
	int nRet = 0;
	if (img_R == NULL || img_R == NULL || img_R == NULL || nImgWidth < 0 || nImgWidth < 0 || retDstImg == NULL)
		return nRet;
	if (fitR <= 0) fitR = 1.0f;
	if (fitG <= 0) fitG = 1.0f;
	if (fitB <= 0) fitB = 1.0f;
	int nGap = m_byDefaultAC[eDefaultAC_BW];
	int nIndex = 0;
	for(int a = 0; a < nImgWidth * nImgHeight; a++)
	{
		byte byRGB = 0;
		float fR = ((float)img_R[a] * fitR);
		float fG = ((float)img_G[a] * fitG);
		float fB = ((float)img_B[a] * fitB);
		float fRG = fR - fG;
		float fRB = fR - fB;
		float fBG = fB - fG;

		if (fRG < 0) fRG *= -1;
		if (fRB < 0) fRB *= -1;
		if (fBG < 0) fBG *= -1;
		if ((fR == fG && fR == fB && fG == fB) ||
			(fRG <= nGap && fRB <= nGap && fBG <= nGap))
			byRGB = 0;
		else if (fR > fG && fR > fB)
			byRGB = 1;
		else if (fG > fR && fG > fB)
			byRGB = 2;
		else if (fB > fR && fB > fG)
			byRGB = 3;
		else
		{
			if ((fR > fG && fR == fB) || (fB > fG && fR == fB))
			{
				if ((m_byDefaultAC[eDefaultAC_Data] & m_eDefaultAC_Default_BR) == m_eDefaultAC_Default_BR)
					byRGB = 3;
				else
					byRGB = 1;
			}
			else if ((fR > fB && fR == fG) || (fG > fB && fR == fG))
			{
				if ((m_byDefaultAC[eDefaultAC_Data] & m_eDefaultAC_Default_GR) == m_eDefaultAC_Default_GR)
					byRGB = 2;
				else
					byRGB = 1;
			}
			else if ((fG > fR && fG == fB) || (fB > fR && fG == fB))
			{
				if ((m_byDefaultAC[eDefaultAC_Data] & m_eDefaultAC_Default_BG) == m_eDefaultAC_Default_BG)
					byRGB = 3;
				else
					byRGB = 2;
			}
		}
		if (byRGB == 1)
		{
			retDstImg[nIndex++] = 0;
			retDstImg[nIndex++] = 0;
			retDstImg[nIndex++] = 255;
		}
		else if (byRGB == 2)
		{
			retDstImg[nIndex++] = 0;
			retDstImg[nIndex++] = 255;
			retDstImg[nIndex++] = 0;
		}
		else if (byRGB == 3)
		{
			retDstImg[nIndex++] = 255;
			retDstImg[nIndex++] = 0;
			retDstImg[nIndex++] = 0;
		}
		else
		{
			retDstImg[nIndex++] = img_B[a];
			retDstImg[nIndex++] = img_G[a];
			retDstImg[nIndex++] = img_R[a];
		}
	}
	m_procMil->SaveWorkImg(retDstImg, nImgWidth, nImgHeight, _T("Rst_Color_Image.bmp"), 3);
	return ePART_SUCCESS;
}
BOOL CPInsp_Color::GetAngleColor(InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg)
{
	BOOL bResult = FALSE;
	try
	{
		if(!m_pProcMilAlgo || sInspRoiImg.imgTop_R == NULL || sInspRoiImg.imgTop_G == NULL ||
			sInspRoiImg.imgTop_B == NULL || sInspRoiImg.nImageSizeX <= 0 || sInspRoiImg.nImageSizeY <= 0 ||
			retDstImg == NULL)
			return bResult;
		memset(retDstImg, 0, sizeof(UCHAR) * sInspRoiImg.nImageSizeX * sInspRoiImg.nImageSizeY * 3);
		int nImgWidth = sInspRoiImg.nImageSizeX;
		int nImgHeight = sInspRoiImg.nImageSizeY;
		UCHAR* img_R = sInspRoiImg.imgTop_R;
		UCHAR* img_G = sInspRoiImg.imgTop_G;
		UCHAR* img_B = sInspRoiImg.imgTop_B;
		LightTypeBuf sLightBuf;
		sLightBuf.m_pucTRed = sInspRoiImg.imgTop_R;
		sLightBuf.m_pucTGreen = sInspRoiImg.imgTop_G;
		sLightBuf.m_pucTBlue = sInspRoiImg.imgTop_B;
		sLightBuf.m_pucTWhite = sInspRoiImg.imgTop_W;
		sLightBuf.m_pucMRed = sInspRoiImg.imgMiddle_R;
		sLightBuf.m_pucMBlue = sInspRoiImg.imgMiddle_B;
		cv::Mat cvImgG = GetGreenImage(sLightBuf.m_pucMRed, sLightBuf.m_pucMBlue, nImgWidth, nImgHeight);
		sLightBuf.m_pucMGreen = cvImgG.data;
		sLightBuf.m_pucMWhite = NULL;
		sLightBuf.m_pucBRed = sInspRoiImg.imgBottom_R;
		sLightBuf.m_pucBGreen = NULL;
		sLightBuf.m_pucBBlue = sInspRoiImg.imgBottom_B;
		sLightBuf.m_pucBWhite = NULL;
		sLightBuf.m_nImgWidth = nImgWidth;
		sLightBuf.m_nImgHeight = nImgHeight;
		sLightBuf.m_nROIImgWidth = nImgWidth;
		sLightBuf.m_nROIImgHeight = nImgHeight;
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
		CPInsp_Algo &InsAlgo = g_pInspMng->GetInspAlgo();
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
			if(a == 0)
				InsAlgo.ROIImageClaculCompose(sLightBuf, img_R);
			else if(a == 1)
				InsAlgo.ROIImageClaculCompose(sLightBuf, img_G);
			else if(a == 2)
				InsAlgo.ROIImageClaculCompose(sLightBuf, img_B);
		}
		Delete_1DArray(&sLightBuf.m_pnRedValue);
		Delete_1DArray(&sLightBuf.m_pnGreenValue);
		Delete_1DArray(&sLightBuf.m_pnBlueValue);
		Delete_1DArray(&sLightBuf.m_pnWhiteValue);
		Delete_1DArray(&sLightBuf.m_pnPosition);
		Delete_1DArray(&sLightBuf.m_pnCalculation);
		m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R.bmp"));
		m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G.bmp"));
		m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B.bmp"));
		int nIdx = 0;
		for (int a = 0; a < nImgWidth * nImgHeight; a++)
		{
			retDstImg[nIdx++] = img_B[a];
			retDstImg[nIdx++] = img_G[a];
			retDstImg[nIdx++] = img_R[a];
		}
		m_procMil->SaveWorkImg(retDstImg, nImgWidth, nImgHeight, _T("Rst_Color_Image.bmp"), 3);
		bResult = TRUE;
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
	return bResult;
}
cv::Mat CPInsp_Color::GetGreenImage(UCHAR* pucR, UCHAR* pucB, int nW, int nH, byte byColorLightType)
{
	cv::Mat imgG;
	if (nW <= 0 || nH <= 0 || pucR == NULL || pucB == NULL)
		return imgG;

	cv::Mat imgR(nH, nW, CV_8UC1, pucR);
	cv::Mat imgB(nH, nW, CV_8UC1, pucB);
	float fGR = m_fFatorMGR;
	float fGB = m_fFatorMGB;
	if (byColorLightType == 2)
	{
		fGR = m_fFatorBGR;
		fGB = m_fFatorBGB;
	}

	float fitMR = fGR <= 0.0 ? 0.4f : fGR;
	float fitMB = fGB <= 0.0 ? 0.8f : fGB;
	cv::addWeighted(imgR, fitMR, imgB, fitMB, 1, imgG);
	return imgG;
}

#pragma region _Foreign_Func_

int  CPInsp_Color::GetColorCIEView_Foreign(InspRoiImgBuf sFov, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI)
{
	try
	{
		if (g_pMPTI)
		{
			InspAlgo sInspAlgo;
			AlgoCoordinate coordinateAlgo;
			sInspAlgo.m_eLightType = Top_Light;
			SetParamChannelImg(sInspAlgo, coordinateAlgo);
		}
		else
			return ePART_FAIL;
		Coordinate vCoordinate;
		vCoordinate.angle = 0;
		vCoordinate.width = rcROI.right - rcROI.left;
		vCoordinate.length = rcROI.bottom - rcROI.top;
		vCoordinate.fovCx = vCoordinate.cx = rcROI.left + (vCoordinate.width / 2);
		vCoordinate.fovCy = vCoordinate.cy = rcROI.top + (vCoordinate.length / 2);
		if (vCoordinate.width < 1 || vCoordinate.length < 1)
			return ePART_FAIL;

		InspForeignInfo sForeign = g_pInspMng->m_FR.GetForeignData();
		float fitR = sForeign.m_fArrData[FR_F_FactorR];
		float fitG = sForeign.m_fArrData[FR_F_FactorG];
		float fitB = sForeign.m_fArrData[FR_F_FactorB];
		float fitBR = sForeign.m_fArrData[FR_F_FactorBR];
		float fitBB = sForeign.m_fArrData[FR_F_FactorBB];
		float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ? (float)1.0 : g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ? (float)1.0 : g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ? (float)1.0 : g_pMPTI->m_fCompoBtmB;
		if (fitR == 0.0) fitR = 1.0f;
		if (fitG == 0.0) fitG = 1.05f;
		if (fitB == 0.0) fitB = 1.0f;
		if (fitBR == 0.0) fitBR = 1.0f;
		if (fitBB == 0.0) fitBB = 1.0f;
		if (sFov.imgSide1_B != NULL || sFov.imgSide1_B != NULL || sFov.imgSide3_B != NULL || sFov.imgSide4_B != NULL)
		{
			fitR = 1;
			fitG = 1;
			fitB = 1;
		}
		//////////////////////////////////////////////////////////////////////////
		/*UCHAR* img_R = new UCHAR[vCoordinate.width * vCoordinate.length];
		UCHAR* img_G = new UCHAR[vCoordinate.width * vCoordinate.length];
		UCHAR* img_B = new UCHAR[vCoordinate.width * vCoordinate.length];
		UCHAR* img_BR = NULL;
		UCHAR* img_BB = NULL;
		UCHAR* colorCIE_org = new UCHAR[SIZE_CLRX * SIZE_CLRY];
		UCHAR* colorCIE = new UCHAR[nCieW * nCieH];
		UCHAR* dispCIE_org = new UCHAR[SIZE_CLRX * SIZE_CLRY * 3];
		UCHAR* dispCIE = new UCHAR[nCieW * nCieH * 3];
		UCHAR* blobImg = new UCHAR[nCieW * nCieH];
		UCHAR* dstTemp = new UCHAR[vCoordinate.width * vCoordinate.length];*/
		UCHAR* img_R = g_pMManager->pem_new<UCHAR>(true, vCoordinate.width * vCoordinate.length, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_G = g_pMManager->pem_new<UCHAR>(true, vCoordinate.width * vCoordinate.length, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_B = g_pMManager->pem_new<UCHAR>(true, vCoordinate.width * vCoordinate.length, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* img_BR = NULL;
		UCHAR* img_BB = NULL;
		UCHAR* colorCIE_org = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* colorCIE = g_pMManager->pem_new<UCHAR>(true, nCieW * nCieH, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* dispCIE_org = g_pMManager->pem_new<UCHAR>(true, SIZE_CLRX * SIZE_CLRY * 3, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* dispCIE = g_pMManager->pem_new<UCHAR>(true, nCieW * nCieH * 3, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* blobImg = g_pMManager->pem_new<UCHAR>(true, nCieW * nCieH, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* dstTemp = g_pMManager->pem_new<UCHAR>(true, vCoordinate.width * vCoordinate.length, (PCHAR)__FUNCTION__, __LINE__);
		memset(img_R, 0, sizeof(UCHAR) * vCoordinate.width * vCoordinate.length);
		memset(img_G, 0, sizeof(UCHAR) * vCoordinate.width * vCoordinate.length);
		memset(img_B, 0, sizeof(UCHAR) * vCoordinate.width * vCoordinate.length);
		memset(colorCIE_org, 0, sizeof(UCHAR) * SIZE_CLRX * SIZE_CLRY);
		memset(colorCIE, 0, sizeof(UCHAR) * nCieW * nCieH);
		memset(dispCIE_org, 0, sizeof(UCHAR) * SIZE_CLRX * SIZE_CLRY * 3);
		memset(dispCIE, 0, sizeof(UCHAR) * nCieW * nCieH * 3);
		memset(blobImg, 0, sizeof(UCHAR) * nCieW * nCieH);
		memset(dstTemp, 0, sizeof(UCHAR) * vCoordinate.width * vCoordinate.length);

		void* temp_BR = NULL;
		void* temp_BB = NULL;
		int nCompoMode = g_pMPTI->GetCompositeLightMode();
		if (nCompoMode == 1)
		{
			/*img_BR = new UCHAR[vCoordinate.width * vCoordinate.length];
			img_BB = new UCHAR[vCoordinate.width * vCoordinate.length];*/
			img_BR = g_pMManager->pem_new<UCHAR>(true, vCoordinate.width * vCoordinate.length, (PCHAR)__FUNCTION__, __LINE__);
			img_BB = g_pMManager->pem_new<UCHAR>(true, vCoordinate.width * vCoordinate.length, (PCHAR)__FUNCTION__, __LINE__);
			memset(img_BR, 0, sizeof(UCHAR) * vCoordinate.width * vCoordinate.length);
			memset(img_BB, 0, sizeof(UCHAR) * vCoordinate.width * vCoordinate.length);
		}
		if (sFov.imgTop_R && sFov.imgTop_G && sFov.imgTop_B && sFov.nImageSizeX > 0 && sFov.nImageSizeY > 0)
		{
			m_procMil->SaveWorkImg(sFov.imgTop_R, sFov.nImageSizeX, sFov.nImageSizeY, _T("GetColorCIEView_Foreign_imgTop_R.bmp"));
			m_procMil->SaveWorkImg(sFov.imgTop_G, sFov.nImageSizeX, sFov.nImageSizeY, _T("GetColorCIEView_Foreign_imgTop_G.bmp"));
			m_procMil->SaveWorkImg(sFov.imgTop_B, sFov.nImageSizeX, sFov.nImageSizeY, _T("GetColorCIEView_Foreign_imgTop_B.bmp"));
			m_procMil->GetClipImage(sFov.imgTop_R, sFov.nImageSizeX, sFov.nImageSizeY, img_R, vCoordinate.cx, vCoordinate.cy, vCoordinate.width, vCoordinate.length);
			m_procMil->GetClipImage(sFov.imgTop_G, sFov.nImageSizeX, sFov.nImageSizeY, img_G, vCoordinate.cx, vCoordinate.cy, vCoordinate.width, vCoordinate.length);
			m_procMil->GetClipImage(sFov.imgTop_B, sFov.nImageSizeX, sFov.nImageSizeY, img_B, vCoordinate.cx, vCoordinate.cy, vCoordinate.width, vCoordinate.length);
		}
		else
		{
			if (g_pMPTI->m_nCameraType == Basler_Color)
			{
				m_procMil->SaveWorkImg((UCHAR*)m_imgBuf[eImg_Top_R], 2040, 2040, _T("GetColorCIEView_Foreign_imgRed_Basler_Color.bmp"), 1);
				m_procMil->SaveWorkImg((UCHAR*)m_imgBuf[eImg_Top_G], 2040, 2040, _T("GetColorCIEView_Foreign_imgGreen_Basler_Color.bmp"), 1);
				m_procMil->SaveWorkImg((UCHAR*)m_imgBuf[eImg_Top_B], 2040, 2040, _T("GetColorCIEView_Foreign_imgBlue_Basler_Color.bmp"), 1);
			}
			else
			{
				m_procMil->SaveWorkImg((Im::PIL_ID)m_imgBuf[eImg_Top_R], _T("GetColorCIEView_Foreign_imgRed.bmp"));
				m_procMil->SaveWorkImg((Im::PIL_ID)m_imgBuf[eImg_Top_G], _T("GetColorCIEView_Foreign_imgGreen.bmp"));
				m_procMil->SaveWorkImg((Im::PIL_ID)m_imgBuf[eImg_Top_B], _T("GetColorCIEView_Foreign_imgBlue.bmp"));
			}
			MakeProcImg(m_imgBuf[eImg_Top_R], m_imgBuf[eImg_Top_G], m_imgBuf[eImg_Top_B], img_R, img_G, img_B, vCoordinate, FALSE, true, m_imgBuf[eImg_Bottom_R], m_imgBuf[eImg_Bottom_B], img_BR, img_BB);
		}
		m_procMil->SaveWorkImg(img_R, vCoordinate.width, vCoordinate.length, _T("GetColorCIEView_Foreign_imgRed2.bmp"));
		m_procMil->SaveWorkImg(img_G, vCoordinate.width, vCoordinate.length, _T("GetColorCIEView_Foreign_imgGreen2.bmp"));
		m_procMil->SaveWorkImg(img_B, vCoordinate.width, vCoordinate.length, _T("GetColorCIEView_Foreign_imgBlue2.bmp"));
		//////////////////////////////////////////////////////////////////////////
		if (nCompoMode == 0)
			ImageToColorCIE(img_R, img_G, img_B, fitR, fitG, fitB, colorCIE_org, vCoordinate.width, vCoordinate.length);
		else
			ImageToColorCIE_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, colorCIE_org, vCoordinate.width, vCoordinate.length);
		m_procMil->SaveWorkImg(colorCIE_org, SIZE_CLRX, SIZE_CLRY, _T("testCIE.bmp"));
		int offsetX = CIE_OFFSETX;
		int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
		int cieCx = (int)(nCieW / 2.0);
		int cieCy = (int)(nCieH / 2.0);
		m_procMil->GetClipImage(colorCIE_org, SIZE_CLRX, SIZE_CLRY, colorCIE, cieCx, cieCy, nCieW, nCieH);
		m_procMil->SaveWorkImg(colorCIE, nCieW, nCieH, _T("testCIE2.bmp"));
		//////////////////////////////////////////////////////////////////////////
		memcpy(dispCIE_org, m_dispCIE_org, sizeof(UCHAR)* SIZE_CLRX * SIZE_CLRY * 3);
		if (nCompoMode == 0)
			ImageToColorCIE_Color(img_R, img_G, img_B, fitR, fitG, fitB, dispCIE_org, vCoordinate.width, vCoordinate.length);
		else
			ImageToColorCIE_Color_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, dispCIE_org, vCoordinate.width, vCoordinate.length);
		m_procMil->SaveWorkImg(dispCIE_org, SIZE_CLRX, SIZE_CLRY, _T("testCIE3.bmp"), 3);
		m_procMil->ImageClip_CIE(dispCIE_org, SIZE_CLRX, SIZE_CLRY, 3, dispCIE, cieCx, cieCy, nCieW, nCieH);
		if (retCie)
		{
			memcpy_s(retCie, sizeof(UCHAR) * nCieW * nCieH * 3, dispCIE, sizeof(UCHAR) * nCieW * nCieH * 3);
			m_procMil->SaveWorkImg(retCie, nCieW, nCieH, _T("testCIE4.bmp"), 3);
		}
		//////////////////////////////////////////////////////////////////////////
		//auto polygon
		m_procMil->MakeBlobImg(colorCIE, blobImg, nCieW, nCieH, 1);
		m_procMil->SaveWorkImg(blobImg, nCieW, nCieH, _T("blobImg.bmp"));
		m_procMil->GetPolygonCoordi(blobImg, nCieW, nCieH, 12, pt);
		RemakePoly(pt, POLYGON_POINT_CNTS, -offsetX, -offsetY, pt);
		//////////////////////////////////////////////////////////////////////////
		POINT pPt[POLYGON_POINT_CNTS];
		RemakePoly(pt, POLYGON_POINT_CNTS, offsetX, offsetY, pPt);
		if (nCompoMode == 0)
			GetSelectColorCIEImg(img_R, img_G, img_B, fitR, fitG, fitB, vCoordinate.width, vCoordinate.length, pPt, 0, dstTemp);
		else
			GetSelectColorCIEImg_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, vCoordinate.width, vCoordinate.length, pPt, 0, dstTemp);
		double factor = m_procMil->GetResizeFactor(nWidth, nHeight, vCoordinate.width, vCoordinate.length);
		m_procMil->ResizeImg(dstTemp, retImg, vCoordinate.width, vCoordinate.length, nWidth, nHeight, 1, factor);
		Delete_1DArray(&img_R);
		Delete_1DArray(&img_G);
		Delete_1DArray(&img_B);
		Delete_1DArray(&img_BR);
		Delete_1DArray(&img_BB);
		Delete_1DArray(&colorCIE_org);
		Delete_1DArray(&colorCIE);
		Delete_1DArray(&dispCIE_org);
		Delete_1DArray(&dispCIE);
		Delete_1DArray(&blobImg);
		Delete_1DArray(&dstTemp);
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
	return ePART_SUCCESS;
}
int CPInsp_Color::Color_GetColorHistogramView_Foreign(int nBin, int projectionmode, RECT rcROI, UCHAR* pucDstImg)
{
	InspForeignInfo sForeign = g_pInspMng->m_FR.GetForeignData();
	int sequence = g_pMPTI->GetSequence();
	int nFOVW = m_fovWidth;
	int nFOVH = m_fovLength;
	int ret = 0;
	int histo[256] = { 0, };
	int nL = rcROI.left;
	int nR = rcROI.right;
	int nT = rcROI.top;
	int nB = rcROI.bottom;
	if (nL < 0) nL = 0;
	if (nR < 0) nR = 0;
	if (nT < 0) nT = 0;
	if (nB < 0) nB = 0;
	if (nL > nFOVW - 1) nL = nFOVW - 1;
	if (nT > nFOVH - 1) nT = nFOVH - 1;
	if (nR > nFOVW) nR = nFOVW;
	if (nT > nFOVH) nT = nFOVH;
	int nW = nR - nL;
	int nH = nB - nT;
	if (nW <= 0 || nH <= 0)
		return ePART_FAIL;

	if (nBin < 0) nBin = 0;
	if (nBin > m_eForeignInsp_GrayBub) nBin = m_eForeignInsp_GrayBub - 1;

	cv::Mat imgORG2D[eM2C_NUM];
	for (int nIDX = 0; nIDX < eM2C_NUM; nIDX++)
	{
		imgORG2D[nIDX] = cv::Mat(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
		void* pORG = (void*)g_pMPTI->GetSeqGrabBufID(sequence, 0, nIDX, projectionmode);
		if (pORG == NULL)
			continue;

		m_pProcMilAlgo->GetClipBuff(pORG, imgORG2D[nIDX].data, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
	}

	LightTypeBuf sLightImg;
	sLightImg.m_pucTRed = imgORG2D[eM2C_TR].data;
	sLightImg.m_pucTGreen = imgORG2D[eM2C_TG].data;
	sLightImg.m_pucTBlue = imgORG2D[eM2C_TB].data;
	sLightImg.m_pucTWhite = imgORG2D[eM2C_TW].data;
	sLightImg.m_pucMRed = imgORG2D[eM2C_MR].data;
	sLightImg.m_pucMGreen = NULL;
	sLightImg.m_pucMBlue = imgORG2D[eM2C_MB].data;
	sLightImg.m_pucMWhite = NULL;
	sLightImg.m_pucBRed = imgORG2D[eM2C_BR].data;
	sLightImg.m_pucBGreen = NULL;
	sLightImg.m_pucBBlue = imgORG2D[eM2C_BB].data;
	sLightImg.m_pucBWhite = NULL;
	sLightImg.m_nImgWidth = nFOVW;
	sLightImg.m_nImgHeight = nFOVH;

	cv::Mat imgSrc(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgG(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBin(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
	int nMode = 0;
	g_pInspMng->m_FR.Get2DForeignGray(sForeign.sBin[nBin], nMode, sLightImg, imgBin, 0, FALSE, imgG.data);

	UCHAR *ucImg = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	m_procMil->GetClipImage_LT(imgG.data, nFOVW, nFOVH, ucImg, nL, nT, nW, nH);
	m_procMil->SaveWorkImg(ucImg, nW, nH, _T("Clip_2D.bmp"));
	m_procMil->GetHistoValue(ucImg, nW, nH, histo);
	m_procMil->MakeHistogramImg(histo, 256, pucDstImg);
	if (ucImg)
	{
		//delete [] ucImg;
		g_pMManager->pem_delete(ucImg, true);
		ucImg = NULL;
	}
	return ePART_SUCCESS;
}
double CPInsp_Color::CompareCIE_ForeignBTM(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, int width,
	int length, POINTF pts[10][POLYGON_BTM_POINT_CNTS], int ptPolyCnt, int ptPolyNum, UCHAR* MaskImg, ColorXYInfoForeign* vColorXYInfoForeign, bool bGetExcept)
{
	double ret = 0.0;
	if (srcR == NULL || srcG == NULL || srcB == NULL || width <= 0 || length <= 0)
		return ret;

	int	nPixelCount = 1000;
	//후보 영역
	cv::Mat arrImgPoly[10];
	cv::Mat arrImgPolyPerpect[10];
	if (g_pInspMng->m_FR.PolygonAreaCalc())
	{
		cv::Scalar white(255, 255, 255);
		for (int i = 0; i < ptPolyCnt; i++)
		{
			arrImgPoly[i] = cv::Mat(nPixelCount, nPixelCount, CV_8UC1, cv::Scalar(0));
			arrImgPolyPerpect[i] = cv::Mat(nPixelCount, nPixelCount, CV_8UC1, cv::Scalar(0));

#if _DEBUG
			cv::Mat imgPoly = arrImgPoly[i];
			cv::Mat imgPolyTemp = arrImgPolyPerpect[i];
#endif
			POINTF arrPo[POLYGON_BTM_POINT_CNTS];
			for (int a = 0; a < ptPolyNum; a++)
			{
				arrPo[a].x = pts[i][a].x;
				arrPo[a].y = pts[i][a].y;
			}
			g_pInspMng->m_FR.FillPolygon(arrImgPoly[i], arrPo, ptPolyNum, white);
			g_pInspMng->m_FR.MorErode(arrImgPoly[i], arrImgPolyPerpect[i]);
		}
	}

#if _DEBUG
	cv::Mat imgR(length, width, CV_8UC1, srcR);
	cv::Mat imgG(length, width, CV_8UC1, srcG);
	cv::Mat imgB(length, width, CV_8UC1, srcB);
#endif

	long long size = width * length;
	cv::Mat	pConvertMat = cv::Mat(3, 3, CV_32FC1);
	pConvertMat.at<float>(0, 0) = 0.490f;     pConvertMat.at<float>(0, 1) = 0.310f;     pConvertMat.at<float>(0, 2) = 0.200f;
	pConvertMat.at<float>(1, 0) = 0.177f;     pConvertMat.at<float>(1, 1) = 0.813f;     pConvertMat.at<float>(1, 2) = 0.011f;
	pConvertMat.at<float>(2, 0) = 0.000f;     pConvertMat.at<float>(2, 1) = 0.010f;     pConvertMat.at<float>(2, 2) = 0.990f;
	for (long long j = 0; j < size; j++)
	{
		if (srcR[j] == 0 && srcG[j] == 0 && srcB[j] == 0)
			continue;
		if (bGetExcept)
		{
			if (srcR[j] <= 20 && srcG[j] <= 20 && srcB[j] <= 20)
				continue;
			else if (srcR[j] >= 200 && srcG[j] >= 200 && srcB[j] >= 200)
				continue;
		}
		float r = ((float)srcR[j] * factorR);		r = _limit_value(r, 0.f, 255.f);
		float g = ((float)srcG[j] * factorG);		g = _limit_value(g, 0.f, 255.f);
		float b = ((float)srcB[j] * factorB);		b = _limit_value(b, 0.f, 255.f);
		POINTF pt = GetCIEPoint(&pConvertMat, r, g, b, nPixelCount);
		int nindex = (nPixelCount * pt.y) + pt.x;
		bool bMergeRst = false;
		for (int a = 0; a < ptPolyCnt; a++)
		{
			bool bIsWhite = true;
			bool bColor = false;
			if (g_pInspMng->m_FR.PolygonAreaCalc())
			{
#if _DEBUG
				cv::Mat imgPoly = arrImgPoly[a];
				cv::Mat imgPolyTemp = arrImgPolyPerpect[a];
#endif
				if (arrImgPoly[a].data[nindex] == 255)
				{
					if (arrImgPolyPerpect[a].data[nindex] == 255)
						bColor = true;
					else
						bColor = pvProcPointInPolygon(pts[a], ptPolyNum, pt);
				}
			}
			else
				bColor = pvProcPointInPolygon(pts[a], ptPolyNum, pt);

			if (vColorXYInfoForeign[a].m_bUseThreshold)
				bIsWhite = IsBlack(r, g, b, vColorXYInfoForeign[a].m_vThresholdType, vColorXYInfoForeign[a].m_nMin, vColorXYInfoForeign[a].m_nMax);
			if (bColor == true && bIsWhite == true)
			{
				bMergeRst = true;
				break;
			}
		}
		MaskImg[j] = (bGetExcept == false) ? (bMergeRst) ? 255 : 0 : (bMergeRst) ? 0 : 255;
	}
	return ret;
}
int  CPInsp_Color::GetFullColorCIEView_Foreign(IN UCHAR* retFullImg, IN int nFullW, IN int nFullH, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI)
{
	try
	{
		if (retFullImg == NULL || nWidth <= 0 || nHeight <= 0)
			return eMR_FAIL;

		InspRoiImgBuf sFov;
		cv::Mat imgFull(nHeight, nWidth, CV_8UC3, retFullImg);
		cv::Mat imgRGB[FR_Find_RGB_Total];
		cv::split(imgFull, imgRGB);

		sFov.imgTop_R = imgRGB[FR_Find_RGB_TopR].data;
		sFov.imgTop_G = imgRGB[FR_Find_RGB_TopG].data;
		sFov.imgTop_B = imgRGB[FR_Find_RGB_TopB].data;
		sFov.nImageSizeX = nFullW;
		sFov.nImageSizeY = nFullH;

		GetColorCIEView_Foreign(sFov, retCie, nCieW, nCieH, retImg, nWidth, nHeight, pt, rcROI);
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
	return ePART_SUCCESS;
}

#pragma endregion _Foreign_Func_

#pragma region _BUBBLE_
// Bubble inspection
BOOL CPInsp_Color::InspColorBubble(InspForeignInfo pForeignData, const InspPartInfo *pInspBoardInfo, InspFovForeignResult* ForeignResult, MPTI_InspectionMode vInspectionMod, ForeignData sData_ROI)
{
	if (!ForeignResult || !m_procMil || !g_pMPTI || g_pMPTI->UseBubbleInsp() == 0)
		return ePART_FAIL;
	SaveBubbleDebugData();
	void* pBB = NULL;
	void* pBR = NULL;
	void* pTB = NULL;
	void* pSubBB = NULL; //Bottom blues for Bubble Inspection (UV Img)
	UCHAR* pUcConvertImgTB = g_pMManager->pem_new<UCHAR>(true, m_fovWidth * m_fovLength, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* pUcConvertImgBR = g_pMManager->pem_new<UCHAR>(true, m_fovWidth * m_fovLength, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* pUcConvertImgBB = g_pMManager->pem_new<UCHAR>(true, m_fovWidth * m_fovLength, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* pUcConvertImgSubBB = g_pMManager->pem_new<UCHAR>(true, m_fovWidth * m_fovLength, (PCHAR)__FUNCTION__, __LINE__);

	FR_Bubble sData = pForeignData.sBubble;
	if (vInspectionMod == eMI_WindowTeaching)
	{
		if (g_pMPTI->m_nCameraType == Basler_Color)
		{
			// 검사용
			pBB = (void*)g_pMPTI->GetGrabOutputBuf_Color(0, 1, 3);
			pBR = (void*)g_pMPTI->GetGrabOutputBuf_Color(0, 1, 1);
			pTB = (void*)g_pMPTI->GetGrabOutputBuf_Color(0, 0, 3);
			if (sData.m_nArr[FR_BUBBLE_N_ColorUVSelect] > 2) // For Bubble Inspection
			{
				pSubBB = (void*)g_pMPTI->GetGrabOutputBuf_Color(0, 1, 2);
				m_procMil->GetClipImage((UCHAR*)pSubBB, m_fovWidth, m_fovLength, pUcConvertImgBB, m_fovWidth, m_fovLength, m_fovWidth, m_fovLength);
			}
			m_procMil->GetClipImage((UCHAR*)pBB, m_fovWidth, m_fovLength, pUcConvertImgBB, m_fovWidth, m_fovLength, m_fovWidth, m_fovLength);
			m_procMil->GetClipImage((UCHAR*)pTB, m_fovWidth, m_fovLength, pUcConvertImgTB, m_fovWidth, m_fovLength, m_fovWidth, m_fovLength);
			m_procMil->GetClipImage((UCHAR*)pBR, m_fovWidth, m_fovLength, pUcConvertImgBR, m_fovWidth, m_fovLength, m_fovWidth, m_fovLength);
		}
		else
		{
			pTB = g_pMPTI->GetSeqGrabBufID(eMGS_FG, 0, eM2C_TB, 0);
			pBR = g_pMPTI->GetSeqGrabBufID(eMGS_FG, 0, eM2C_BR, 0);
			pBB = g_pMPTI->GetSeqGrabBufID(eMGS_FG, 0, eM2C_BB, 0);

			m_procMil->GetClipBuff(pBB, pUcConvertImgBB, m_fovWidth, m_fovLength, m_fovWidth, m_fovLength);
			m_procMil->GetClipBuff(pTB, pUcConvertImgTB, m_fovWidth, m_fovLength, m_fovWidth, m_fovLength);
			m_procMil->GetClipBuff(pBR, pUcConvertImgBR, m_fovWidth, m_fovLength, m_fovWidth, m_fovLength);
		}

		CString path = g_pInspMng->m_FR.GetForeignDebugDataFullPath(vInspectionMod == eMI_Inspection ? pInspBoardInfo->fovIndex : 0, _T("2DInspGrayBub_GetTeachingImage"), m_eForeignInsp_GrayBub);
		if (pUcConvertImgBB && m_fovWidth > 0 && m_fovLength > 0)
			m_procMil->SaveDebugImg(pUcConvertImgBB, m_fovWidth, m_fovLength, path, 1, FALSE);
	}
	int nFOVMargin = 1;	//mm
	int nFilter = 1;
	bool fillHole = TRUE;
	int nExtraPixelCnt = sData.m_nArr[FR_BUBBLE_N_ExtraPixelCnt_Bubble];
	double dArea = sData.m_fArr[FR_BUBBLE_F_Area2D_GrayBub];
	double dForeignArea2D = dArea / m_resolX / m_resolY / 4;
	//For CNN Inference Function 
	CRect *rtForeignArea_Infer = NULL;
	double *dArea_Infer = 0;
	double *dCx_Infer = 0;
	double *dCy_Infer = 0;
	int cntBubbleAfterCNN = 0;		//Bubble Result Count using CNN Function
	cv::Mat im;
	int nAfterMergeCnt = 0;
	if (sData.m_nArr[FR_BUBBLE_N_ColorUVSelect] > 2) // For Bubble Inspection
		im = cv::Mat(cv::Size(m_fovWidth, m_fovLength), CV_8UC1, pUcConvertImgSubBB);
	else
		im = cv::Mat(cv::Size(m_fovWidth, m_fovLength), CV_8UC1, pUcConvertImgBB);
	bool isCNNModelLoaded = true;
	g_pMPTI->m_pCSMLThreadMng->inference._block = 20;					// (AI) Set Bubble Test count
	g_pMPTI->m_pCSMLThreadMng->inference.binary_threshold = sData.m_nArr[FR_BUBBLE_N_BubbleThreshold];
	int cntBubbleAfterSeg = 0;
	std::vector<CRect> vecSegmentRstBubRect;
	std::vector<double> vecSegmentRstBubArea;
	std::vector<double> vecBubRstAreamm;
	std::vector<double> SegPX, SegPY;
	cv::Mat BWimg;
	UCHAR* pUcDesBuff = NULL;
	//UCHAR *ucArrDstImgMergeAllBW = new UCHAR[m_fovWidth * m_fovLength];
	UCHAR *ucArrDstImgMergeAllBW = g_pMManager->pem_new<UCHAR>(true, m_fovWidth * m_fovLength, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR *ucArrDstMergeAllBW_Blob = NULL;		//BW_Img All_Merge Blob Image
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrDstMergeAllBW_Blob, m_fovWidth * m_fovLength);
	memset(ucArrDstImgMergeAllBW, 0, m_fovWidth * m_fovLength * sizeof(UCHAR));

	int cnt_Infer = 0;
	if (vInspectionMod == eMI_Inspection)
		BWimg = g_pMPTI->m_pCSMLThreadMng->buffer_get(g_pMPTI->m_nCurInspFovIdx);
	else if (vInspectionMod == eMI_WindowTeaching)
	{
		BWimg = g_pMPTI->m_pCSMLThreadMng->inference.LabelMaker(im);
	}

	pUcDesBuff = (UCHAR*)BWimg.data;

	CString path = g_pInspMng->m_FR.GetForeignDebugDataFullPath(vInspectionMod == eMI_Inspection ? pInspBoardInfo->fovIndex : 0, _T("2DInspGrayBub_GetLabelImage"), m_eForeignInsp_GrayBub);
	if (pUcDesBuff && m_fovWidth > 0 && m_fovLength > 0)
		m_procMil->SaveDebugImg(pUcDesBuff, m_fovWidth, m_fovLength, path, 1, FALSE);

	// Except
	int widthStep = m_fovWidth;
	//if (m_fovWidth % 4 != 0) widthStep += (4 - (m_fovWidth % 4));
	widthStep = g_pMPTI->nCalcWidthStep(false, m_fovWidth);

	IppiSize iSize = { m_fovWidth, m_fovLength };
	IppStatus sts = ippiCopy_8u_C1R(pUcDesBuff, widthStep, ucArrDstImgMergeAllBW, m_fovWidth, iSize);
	if (g_pInspMng)
	{
		cv::Mat imgDstImgMergeAllBW(m_fovLength, m_fovWidth, CV_8UC1, ucArrDstImgMergeAllBW);
		g_pInspMng->m_FR.ExceptForeignImage(ForeignParamROI_Data_Bubble, imgDstImgMergeAllBW, sData_ROI);
		memcpy(ucArrDstImgMergeAllBW, imgDstImgMergeAllBW.data, sizeof(UCHAR)* m_fovWidth * m_fovLength);
	}

	path = g_pInspMng->m_FR.GetForeignDebugDataFullPath(vInspectionMod == eMI_Inspection ? pInspBoardInfo->fovIndex : 0, _T("2DInspGrayBub_Except2DImage"), m_eForeignInsp_GrayBub);
	if (ucArrDstImgMergeAllBW && m_fovWidth > 0 && m_fovLength > 0)
		m_procMil->SaveDebugImg(ucArrDstImgMergeAllBW, m_fovWidth, m_fovLength, path, 1, FALSE);
	int nCntBlobAfterSegment = 0;
	//Blob (input : bubble BW Merge image[m_fovWidth x m_fovLength])
	bool bUseMeasureBubSize = sData.UseData(FR_BUBBLE_DT_MeasureBubSize);
	if (bUseMeasureBubSize)		//Use Bubble Size Spec 
		nCntBlobAfterSegment = m_procMil->CalcBlob_Select(ucArrDstImgMergeAllBW, ucArrDstMergeAllBW_Blob, m_fovWidth, m_fovLength, 0, FALSE, fillHole, nFilter, 1);
	else
		nCntBlobAfterSegment = m_procMil->CalcBlob_Select(ucArrDstImgMergeAllBW, ucArrDstMergeAllBW_Blob, m_fovWidth, m_fovLength, dForeignArea2D * 4, FALSE, fillHole, nFilter, 1);

	path = g_pInspMng->m_FR.GetForeignDebugDataFullPath(vInspectionMod == eMI_Inspection ? pInspBoardInfo->fovIndex : 0, _T("2DInspGrayBub_AfterCalcBlob_Select"), m_eForeignInsp_GrayBub);
	if (ucArrDstMergeAllBW_Blob && m_fovWidth > 0 && m_fovLength > 0)
		m_procMil->SaveDebugImg(ucArrDstMergeAllBW_Blob, m_fovWidth, m_fovLength, path, 1, FALSE);

	// Release memery
	if (vInspectionMod == eMI_Inspection)
	{
		g_pMPTI->m_pCSMLThreadMng->buffer_init(g_pMPTI->m_nCurInspFovIdx);
	}
	else if (vInspectionMod == eMI_WindowTeaching)
	{
		BWimg.release();
	}

	CRect *rtForeignArea_AfterSegment = NULL;
	double *dCX_AfterSegment = 0;
	double *dCY_AfterSegment = 0;
	double *dArea_AfterSegment = 0;

	int cnt_Segmentation = 0;

	float nSegImgWidth = 0.0f;
	float nSegImgHeight = 0.0f;

	if (nCntBlobAfterSegment > 0)
	{
		//rtForeignArea_AfterSegment = new CRect[nCntBlobAfterSegment];
		rtForeignArea_AfterSegment = g_pMManager->pem_new<CRect>(true, nCntBlobAfterSegment, (PCHAR)__FUNCTION__, __LINE__);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArea_AfterSegment, nCntBlobAfterSegment);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dCX_AfterSegment, nCntBlobAfterSegment);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dCY_AfterSegment, nCntBlobAfterSegment);

		// get blob parameter
		m_procMil->GetForeignBlobResult(dArea_AfterSegment, dCX_AfterSegment, dCY_AfterSegment, rtForeignArea_AfterSegment);

		// 2019.03.05 MSH - ÃÖÁ¾ Bubble Blob °á°ú txt ÀúÀåºÎ Ãß°¡
		bool	InspResSave = false;

		if (g_pMPTI->GetSaveBubInspResult() == 1)
		{
			InspResSave = true;
		}
		FILE	*fp_insp;
		CString fpCNNTxtInspData;
		CString FovInspNum;

		if (InspResSave)
		{
			CString cstrFolderName = _T("CNNResultData");
			CString cstrFileName = _T("INSP.txt");
			fpCNNTxtInspData = MakeFIlePath_BubbleResult(cstrFolderName, cstrFileName);
			fp_insp = _wfopen(fpCNNTxtInspData, _T("at"));
		}

		cnt_Segmentation = 0;
		for (int j = 0; j < nCntBlobAfterSegment; j++)
		{
			//BlobµÈ ¶óº§ÀÇ Area ±¸ÇÏ±â À§ÇÑ ±¸¹®
			nSegImgWidth = abs(rtForeignArea_AfterSegment[cnt_Segmentation].right - rtForeignArea_AfterSegment[cnt_Segmentation].left);		//FArea[i].width 
			nSegImgHeight = abs(rtForeignArea_AfterSegment[cnt_Segmentation].bottom - rtForeignArea_AfterSegment[cnt_Segmentation].top);		//FArea[i].Bottom

			float fFovHalfWidth = (nSegImgWidth / 2.0) * (float)m_resolX;	// board ÁÂÇ¥ À§µå
			float fFovHalfHeight = (nSegImgHeight / 2.0) * (float)m_resolY;
			float fSegArea = (fFovHalfWidth * fFovHalfHeight);

			rtForeignArea_AfterSegment[cnt_Segmentation].bottom = (rtForeignArea_AfterSegment[cnt_Segmentation].bottom) / (LONG)2.0;
			rtForeignArea_AfterSegment[cnt_Segmentation].top = (rtForeignArea_AfterSegment[cnt_Segmentation].top) / (LONG)2.0;
			rtForeignArea_AfterSegment[cnt_Segmentation].left = (rtForeignArea_AfterSegment[cnt_Segmentation].left) / (LONG)2.0;
			rtForeignArea_AfterSegment[cnt_Segmentation].right = (rtForeignArea_AfterSegment[cnt_Segmentation].right) / (LONG)2.0;

			vecSegmentRstBubRect.emplace_back((rtForeignArea_AfterSegment[j]));
			vecSegmentRstBubArea.emplace_back((dArea_AfterSegment[j]));
			vecBubRstAreamm.emplace_back(fSegArea);

			SegPX.emplace_back(dCX_AfterSegment[j]);		//PX : ±âÁ¸ blob°á°ú ÁÂÇ¥ +Offset X°ª
			SegPY.emplace_back(dCY_AfterSegment[j]);		//PY : ±âÁ¸ blob°á°ú ÁÂÇ¥ +Offset Y°ª

			//최종 Bubble Blob 좌표 저장하는 옵션
			if (InspResSave && fp_insp)
			{
				fwprintf(fp_insp, _T("[index:%d]\t%f\t%f\t%f\t%f\n"), j, dCX_AfterSegment[j], dCY_AfterSegment[j], nSegImgWidth, nSegImgHeight);		//save Insp blob position(text file)
			}

			/////////////////////
			cntBubbleAfterSeg++;		//Segmentation BW_Merge  Blob
			cnt_Segmentation++;

			nSegImgWidth = 0.0;
			nSegImgHeight = 0.0;
			fSegArea = 0.0;
		}

		if (InspResSave)
			fclose(fp_insp);

		cntBubbleAfterCNN = cntBubbleAfterSeg;
	}
	else
	{
		//AfxMessageBox(_T("None Segmentation Result."));
	}

	if (cntBubbleAfterCNN > 0)		//ÃÖÁ¾ CNN ÇÔ¼ö °á°ú°¡ Á¸ÀçÇÒ ¶§ 
	{
		/*rtForeignArea_Infer = new CRect[cntBubbleAfterCNN];
		dArea_Infer = new double[cntBubbleAfterCNN];
		dCx_Infer = new double[cntBubbleAfterCNN];
		dCy_Infer = new double[cntBubbleAfterCNN];*/
		rtForeignArea_Infer = g_pMManager->pem_new<CRect>(true, cntBubbleAfterCNN, (PCHAR)__FUNCTION__, __LINE__);
		dArea_Infer = g_pMManager->pem_new<double>(true, cntBubbleAfterCNN, (PCHAR)__FUNCTION__, __LINE__);
		dCx_Infer = g_pMManager->pem_new<double>(true, cntBubbleAfterCNN, (PCHAR)__FUNCTION__, __LINE__);
		dCy_Infer = g_pMManager->pem_new<double>(true, cntBubbleAfterCNN, (PCHAR)__FUNCTION__, __LINE__);

		//get Bubble position After CNNInfer
		int cnt_Infer = 0;
		int nIdxCnt = 0;
		for (int i = 0; i < cntBubbleAfterCNN; i++)
		{
			if (SegPX.size() == cntBubbleAfterCNN)
			{
				dCx_Infer[cnt_Infer] = SegPX[i];
				dCy_Infer[cnt_Infer] = SegPY[i];

				dArea_Infer[cnt_Infer] = (vecSegmentRstBubArea[i]);
				rtForeignArea_Infer[cnt_Infer] = vecSegmentRstBubRect[i];

				cnt_Infer++;
			}
		}
	}

	if (cntBubbleAfterCNN > 0)		//Bubble Result  Using AI Function
	{
		//ForeignResult->m_stForeign = new AForeignResult[cntBubbleAfterCNN];	
		ForeignResult->m_stForeign = g_pMManager->pem_new<AForeignResult>(true, cntBubbleAfterCNN, (PCHAR)__FUNCTION__, __LINE__);
		ForeignResult->m_nCountDefect = cntBubbleAfterCNN;

		int nRealCnt = 0;
		int nExceededMaxSizeCnt = 0;		// Use Bubble Max Size Check
		double AreaBub = 0;

		for (int i = 0; i < cntBubbleAfterCNN; i++)
		{
			double nImgWidth = abs(rtForeignArea_Infer[i].right - rtForeignArea_Infer[i].left);		//FArea[0].width ±¸ÇÏ±â
			double nImgHeight = abs(rtForeignArea_Infer[i].bottom - rtForeignArea_Infer[i].top);	//FArea[0].Bottom ±¸ÇÏ±â

			if (CheckForeignBigArea((nImgWidth*(double)2.0) * m_resolX, (nImgHeight*(double)2.0) * m_resolY, sData.m_fArr[FR_BUBBLE_F_BubbleExceptSize]))		//Except Bubble Size
			{
				ForeignResult->m_nCountDefect--;
				continue;
			}

			double dImgWidthMM = (nImgWidth*(double)2.0) * m_resolX;
			double dImgHeightMM = (nImgHeight*(double)2.0) * m_resolY;

			//Use Measuring Size of Bubble   NYJ 2018/12/12 
			if (bUseMeasureBubSize)
			{
				if (g_pMPTI->GetUseBubMaxSizeCheckOption() == 1)	//Use Bubble Max Size Check 
				{
					AreaBub = dImgWidthMM * dImgHeightMM;
					if (g_pMPTI->GetBubMaxSizeSpec() < AreaBub)		//Exceeded Max Size 
					{
						nExceededMaxSizeCnt++;
					}
				}

				//Use Bubble Size Option 
				if ((sData.m_fArr[FR_BUBBLE_F_BubbleHeightMin] > dImgHeightMM || sData.m_fArr[FR_BUBBLE_F_BubbleHeightMax] < dImgHeightMM) ||
					(sData.m_fArr[FR_BUBBLE_F_BubbleWidthMin] > dImgWidthMM || sData.m_fArr[FR_BUBBLE_F_BubbleWidthMax] < dImgWidthMM))
				{
					ForeignResult->m_nCountDefect--;
					continue;
				}
			}
			else
			{
				//Use Bubble Area Spec && Use Bubble Max Size Check Option
				if (g_pMPTI->GetUseBubMaxSizeCheckOption() == 1)
				{
					/*AreaBub = ( ((dForeignArea2D*m_resolX)*m_resolY) * 4 );*/
					AreaBub = dImgWidthMM * dImgHeightMM;
					if (g_pMPTI->GetBubMaxSizeSpec() < AreaBub)
						nExceededMaxSizeCnt++;
				}

				//Use Bubble Area Spec
				if ((dImgWidthMM * dImgHeightMM) < (((dForeignArea2D*m_resolX)*m_resolY) * 4))	//Except Area
				{
					ForeignResult->m_nCountDefect--;
					continue;
				}
			}

			ForeignResult->m_stForeign[nRealCnt].SetData(m_eFR_RST_D_HA, false);

			ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Mode, true, m_eForeignInsp_GrayBub);
			ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_ModuleID, true);
			ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Range3D, true);
			ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Defect, true, Foreign_NG_Type_Bubble);
			ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_UVColorSelect, true, sData.m_nArr[FR_BUBBLE_N_ColorUVSelect]);

			ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_X, true, dCx_Infer[i] / 2.0f);
			ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_Y, true, dCy_Infer[i] / 2.0f);
			ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_W, true, (nImgWidth * 2)* m_resolX);
			ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_L, true, (nImgHeight * 2)* m_resolY);
			ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_A, true, dArea_Infer[i] * m_resolX * m_resolY);
			ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_H, true);

			ForeignResult->m_stForeign[nRealCnt].m_rcRect = rtForeignArea_Infer[i];
			nRealCnt++;
		}

		//Use Bubble NG Limit Option       NYJ 2019/04/29
		if (g_pMPTI->GetUseBubNGLimitOption() == 1) //&& vInspectionMod == eMI_Inspection )
		{
			if (g_pMPTI->GetUseBubMaxSizeCheckOption() == 1)		// 두 옵션 동시 사용시, Max 크기가 하나라도 벗어나면 NG
			{
				if (nExceededMaxSizeCnt == 0)
				{
					int LimitCount = g_pMPTI->GetBubNGLimitCount();
					if (LimitCount >= nRealCnt)
					{
						ForeignResult->m_nCountDefect = 0;
						ForeignResult->m_stForeign = NULL;
					}
				}
			}
			else
			{
				int LimitCount = g_pMPTI->GetBubNGLimitCount();
				if (LimitCount >= nRealCnt)
				{
					ForeignResult->m_nCountDefect = 0;
					ForeignResult->m_stForeign = NULL;
				}
			}
		}
	}

	// Release Memory
	if (rtForeignArea_AfterSegment != NULL)
		//delete[] rtForeignArea_AfterSegment;
		g_pMManager->pem_delete(rtForeignArea_AfterSegment, true);
	rtForeignArea_AfterSegment = NULL;

	Delete_1DArray(&dArea_AfterSegment);
	Delete_1DArray(&dCX_AfterSegment);
	Delete_1DArray(&dCY_AfterSegment);
	Delete_1DArray(&ucArrDstMergeAllBW_Blob);

	if (rtForeignArea_Infer != NULL)
		//delete [] rtForeignArea_Infer;
		g_pMManager->pem_delete(rtForeignArea_Infer, true);
	rtForeignArea_Infer = NULL;

	if (dArea_Infer != NULL)
		//delete [] dArea_Infer;
		g_pMManager->pem_delete(dArea_Infer, true);
	dArea_Infer = NULL;

	if (dCx_Infer != NULL)
		//delete [] dCx_Infer;
		g_pMManager->pem_delete(dCx_Infer, true);
	dCx_Infer = NULL;

	if (dCy_Infer != NULL)
		//delete [] dCy_Infer;
		g_pMManager->pem_delete(dCy_Infer, true);
	dCy_Infer = NULL;

	if (ucArrDstImgMergeAllBW != NULL)
		//delete[] ucArrDstImgMergeAllBW;
		g_pMManager->pem_delete(ucArrDstImgMergeAllBW, true);
	ucArrDstImgMergeAllBW = NULL;

	if (pUcConvertImgTB != NULL)
		//delete []		pUcConvertImgTB;
		g_pMManager->pem_delete(pUcConvertImgTB, true);
	pUcConvertImgTB = NULL;

	if (pUcConvertImgBR != NULL)
		//delete []		pUcConvertImgBR;
		g_pMManager->pem_delete(pUcConvertImgBR, true);
	pUcConvertImgBR = NULL;
// 
	if (pUcConvertImgBB != NULL)
// 		//delete []		pUcConvertImgBB;
		g_pMManager->pem_delete(pUcConvertImgBB, true);
	pUcConvertImgBB = NULL;
// 
	if (pUcConvertImgSubBB != NULL)
// 		//delete[]		pUcConvertImgSubBB;
		g_pMManager->pem_delete(pUcConvertImgSubBB, true);
	pUcConvertImgSubBB = NULL;
	return FALSE;
}
void CPInsp_Color::SaveBubbleDebugData()
{
	if (!g_pMPTI->UseBubbleInsp())
		return;
// 
// 	//Monitoring the Bubble NG Data
	if (g_pMPTI->GetSaveBubNGImg())
	{
		CString FilePath = _T("D:\\Eagle3D_Data\\ForeignDebugData\\Bubble\\");			// Bubble NG Image Path 
		if (GetFileAttributes(FilePath) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory((LPCWSTR)FilePath, NULL);
		}
// 
		CString currentTimeFilePath;
		CTime tmDir(CTime::GetCurrentTime());
		currentTimeFilePath.Format(_T("%s\\%s"), FilePath, tmDir.Format(_T("20%y%m%d_%H%M%S")));
		if (GetFileAttributes(currentTimeFilePath) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory((LPCWSTR)currentTimeFilePath, NULL);		//Creates Bubble NG file and file names with inspection time
		}
// 
		FilePath = currentTimeFilePath;
		g_pMPTI->m_pCSMLThreadMng->inference.BubbleNGImgPath = FilePath;		//AI CNN Func
// 
		g_pMPTI->m_pCSMLThreadMng->inference.save_chk = true;
// 
	}
	else
	{
		g_pMPTI->m_pCSMLThreadMng->inference.save_chk = false;
	}
}
BOOL CPInsp_Color::CheckForeignBigArea(float fSizeX, float fSizeY, float fStandardSize)
{
	bool bIsRemove = false;

	if (fSizeX >= fStandardSize || fSizeY >= fStandardSize)
		bIsRemove = true;

	return bIsRemove;
}
CString CPInsp_Color::MakeFIlePath_BubbleResult(CString FolderName, CString FileName)
{
	//CNN Data(Blob position) path :  [D:\\FolderName\\{InspTime}\\{fov_idx}.txt]
	CString strDesFolderPath;
	strDesFolderPath.Format(_T("%s%s"), _T("D:\\"), FolderName);

	CString strDesPathCNNData;
	strDesPathCNNData.Format(_T("%s%s%s%s"), _T("D:\\"), FolderName, _T("\\"), g_pMPTI->m_sCurJobPath);

	if (GetFileAttributes(strDesFolderPath) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectory((LPCWSTR)strDesFolderPath, NULL);
	}

	if (GetFileAttributes(strDesPathCNNData) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectory((LPCWSTR)strDesPathCNNData, NULL);
	}

	CString fpCurTimeCNNData;
	fpCurTimeCNNData.Format(_T("%s\\%s"), strDesPathCNNData, g_pMPTI->timeCNNDataDir);

	if (GetFileAttributes(fpCurTimeCNNData) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectory((LPCWSTR)fpCurTimeCNNData, NULL);
	}

	CString FovNum;
	CString fpCNNUVImg;

	FovNum.Format(_T("FOV_%d_%s"), g_pMPTI->m_nCurInspFovIdx, FileName);	////검사 중인 FOV Number 및 저장 대상체로 파일명 생성

	fpCNNUVImg.Format(_T("%s\\%s"), fpCurTimeCNNData, FovNum);

	return fpCNNUVImg;
}
#pragma endregion _BUBBLE_

int CPInsp_Color::CalTabArea(cv::Mat InputImg, double* A1, double* A2, double* A3, double* A2RecrArea, int* nSt, int* nEd, bool bIsTeach)
{
	//InputImg must Binary Image (0, 255)
	//contours box
	int nCntTeachBin(0), nCntInspBin(0);
	cv::Mat InputImgCl = InputImg.clone();
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(InputImgCl, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	double dMxArea = 0;
	int idx = 0;
	cv::Rect bMaxboxTeach(0, 0, 0, 0);
	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);
		double area = cv::contourArea(contours[idx]);

		if (area > dMxArea)
		{
			dMxArea = area;
			bMaxboxTeach = bbox;
		}
	}

	//Area
	bMaxboxTeach.height -= 2; bMaxboxTeach.y += 1;
	cv::Mat TabImg = InputImg(bMaxboxTeach);
	//int nSt(0), nEd = TabImg.cols - 1;
	*nSt = 0; *nEd = TabImg.cols - 1;
	for (int c = 0; c < TabImg.cols; c++)
	{
		int nColCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			nColCnt += ptr[c];
		}
		nColCnt /= 255;

		if ((double)nColCnt / TabImg.rows > 0.6)
		{
			*nSt = c;
			break;
		}
	}
	for (int c = TabImg.cols - 1; c > 0; c--)
	{
		int nColCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			nColCnt += ptr[c];
		}
		nColCnt /= 255;

		if ((double)nColCnt / TabImg.rows > 0.85)
		{
			*nEd = c;
			break;
		}
	}

	double dArea1(0.0f), dArea2(0.0f), dArea3(0.0f);
	if (*nSt > 5 || !bIsTeach)
	{
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			for (int c = 0; c < *nSt; c++)
			{
				dArea1 += (double)ptr[c];
			}
		}
		*A1 = dArea1 / 255;
	}
	for (int r = 0; r < TabImg.rows; r++)
	{
		uchar* ptr = TabImg.ptr(r);
		for (int c = *nEd; c < TabImg.cols; c++)
		{
			dArea3 += (double)ptr[c];
		}
	}
	*A3 = dArea3 / 255;

	cv::Mat A2Img = TabImg(cv::Rect(*nSt, 0, *nEd - *nSt, TabImg.rows)).clone();
	contours.clear(); hierarchy.clear();
	cv::findContours(A2Img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	dMxArea = 0;
	idx = 0;
	cv::Rect bMaxboxA2 = cv::Rect(0, 0, 0, 0);
	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);
		double area = cv::contourArea(contours[idx]);

		if (area > dMxArea)
		{
			dMxArea = area;
			bMaxboxA2 = bbox;
		}
	}
	if (bMaxboxA2.width > 18) 
	{
		bMaxboxA2.x += (4/*bMaxboxA2.width / 10*/);
		bMaxboxA2.width -= 8;
	}

	if (bMaxboxA2.height > 18)
	{
		bMaxboxA2.y += (4/*bMaxboxA2.height / 10*/);
		bMaxboxA2.height -= 8;
	}
	*A2RecrArea = (double)bMaxboxA2.area();

	for (int r = bMaxboxA2.y; r < bMaxboxA2.y + bMaxboxA2.height; r++)
	{
		uchar* ptr = TabImg.ptr(r);
		for (int c = bMaxboxA2.x; c < bMaxboxA2.x + bMaxboxA2.width; c++)
		{
			dArea2 += (double)ptr[c + *nSt];
		}
	}
	*A2 = dArea2 / 255;
	*nSt += bMaxboxTeach.x;
	*nEd += bMaxboxTeach.x;
#if _DEBUG
	CString sImgLog;
	sImgLog.Format(_T("D:\\testimage\\bMaxboxA2.bmp"), i);
	cv::imwrite(std::string(CT2A(sImgLog)), TabImg(bMaxboxA2));
#endif
	return A2Img.cols;
}
BOOL CPInsp_Color::GetColorBaseBinTab(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg, CRect rt, bool bTeach, int nInspAC,
	UCHAR* retDstImgAC, int nIDX, int nSideCameraIndex, TotalInspExceptArea *stTieArea, bool bSaveR)
{
	BOOL bResult = FALSE;
	try
	{
		if (!m_pProcMilAlgo)
			return bResult;
		if (pInspAlgoColor == NULL)
			return bResult;
		if (sInspRoiImg.imgTop_R == NULL || sInspRoiImg.imgTop_G == NULL || sInspRoiImg.imgTop_B == NULL)
			return bResult;
		if (sInspRoiImg.nImageSizeX <= 0 || sInspRoiImg.nImageSizeY <= 0)
			return bResult;

		bool bInspAC = ((nInspAC & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
		int nImgWidth = sInspRoiImg.nImageSizeX;
		int nImgHeight = sInspRoiImg.nImageSizeY;
		UCHAR* img_R = sInspRoiImg.imgTop_R;
		UCHAR* img_G = sInspRoiImg.imgTop_G;
		UCHAR* img_B = sInspRoiImg.imgTop_B;
		UCHAR* img_BR = sInspRoiImg.imgBottom_R;
		UCHAR* img_BB = sInspRoiImg.imgBottom_B;
		cv::Mat imgG;
		if (pInspAlgoColor->m_byColorLightType == 1 && pInspAlgoColor->m_bUseAngleColor == FALSE && sInspRoiImg.imgMiddle_R != NULL && sInspRoiImg.imgMiddle_B != NULL)
		{
			img_R = sInspRoiImg.imgMiddle_R;
			img_B = sInspRoiImg.imgMiddle_B;
			imgG = GetGreenImage(img_R, img_B, nImgWidth, nImgHeight);
			img_G = imgG.ptr<UCHAR>();
		}

		if (nSideCameraIndex == 0 && sInspRoiImg.imgSide1_G != NULL)	// 사이드 카메라 1
		{
			img_R = sInspRoiImg.imgSide1_R;
			img_G = sInspRoiImg.imgSide1_G;
			img_B = sInspRoiImg.imgSide1_B;
		}
		else if (nSideCameraIndex == 1 && sInspRoiImg.imgSide2_G != NULL)	// 사이드 카메라 2
		{
			img_R = sInspRoiImg.imgSide2_R;
			img_G = sInspRoiImg.imgSide2_G;
			img_B = sInspRoiImg.imgSide2_B;
		}
		else if (nSideCameraIndex == 2 && sInspRoiImg.imgSide3_G != NULL)	// 사이드 카메라 3
		{
			img_R = sInspRoiImg.imgSide3_R;
			img_G = sInspRoiImg.imgSide3_G;
			img_B = sInspRoiImg.imgSide3_B;
		}
		else if (nSideCameraIndex == 3 && sInspRoiImg.imgSide4_G != NULL)	// 사이드 카메라 4
		{
			img_R = sInspRoiImg.imgSide4_R;
			img_G = sInspRoiImg.imgSide4_G;
			img_B = sInspRoiImg.imgSide4_B;
		}
		if (bInspAC && retDstImgAC && bTeach == false)
		{
			img_R = sInspRoiImg.imgMiddle_R;
			img_G = sInspRoiImg.imgTop_W;
			img_B = sInspRoiImg.imgMiddle_B;
		}
		bool bAC = (pInspAlgoColor->m_bUseAngleColor || bInspAC);
		if (bTeach && bAC)
		{
			LightTypeBuf sLightBuf;
			sLightBuf.m_pucTRed = sInspRoiImg.imgTop_R;
			sLightBuf.m_pucTGreen = sInspRoiImg.imgTop_G;
			sLightBuf.m_pucTBlue = sInspRoiImg.imgTop_B;
			sLightBuf.m_pucTWhite = sInspRoiImg.imgTop_W;
			sLightBuf.m_pucMRed = sInspRoiImg.imgMiddle_R;
			imgG = GetGreenImage(sInspRoiImg.imgMiddle_R, sInspRoiImg.imgMiddle_B, nImgWidth, nImgHeight);
			sLightBuf.m_pucMGreen = imgG.data;
			sLightBuf.m_pucMBlue = sInspRoiImg.imgMiddle_B;
			sLightBuf.m_pucMWhite = NULL;
			sLightBuf.m_pucBRed = sInspRoiImg.imgBottom_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = sInspRoiImg.imgBottom_B;
			sLightBuf.m_pucBWhite = NULL;
			sLightBuf.m_nImgWidth = nImgWidth;
			sLightBuf.m_nImgHeight = nImgHeight;
			sLightBuf.m_nROIImgWidth = nImgWidth;
			sLightBuf.m_nROIImgHeight = nImgHeight;
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
			CPInsp_Algo &InsAlgo = g_pInspMng->GetInspAlgo();
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
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_R);
				else if (a == 1)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_G);
				else if (a == 2)
					InsAlgo.ROIImageClaculCompose(sLightBuf, img_B);
			}
			Delete_1DArray(&sLightBuf.m_pnRedValue);
			Delete_1DArray(&sLightBuf.m_pnGreenValue);
			Delete_1DArray(&sLightBuf.m_pnBlueValue);
			Delete_1DArray(&sLightBuf.m_pnWhiteValue);
			Delete_1DArray(&sLightBuf.m_pnPosition);
			Delete_1DArray(&sLightBuf.m_pnCalculation);

			m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R_AC.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G_AC.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B_AC.bmp"), 1, false, bSaveR);
		}
		else
		{
			m_procMil->SaveWorkImg(img_R, nImgWidth, nImgHeight, _T("Img_R.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_G, nImgWidth, nImgHeight, _T("Img_G.bmp"), 1, false, bSaveR);
			m_procMil->SaveWorkImg(img_B, nImgWidth, nImgHeight, _T("Img_B.bmp"), 1, false, bSaveR);
			if (img_BR != NULL && img_BR == nullptr)
				m_procMil->SaveWorkImg(img_BR, nImgWidth, nImgHeight, _T("Img_BR.bmp"), 1, false, bSaveR);
			if (img_BB != NULL && img_BB == nullptr)
				m_procMil->SaveWorkImg(img_BB, nImgWidth, nImgHeight, _T("Img_BB.bmp"), 1, false, bSaveR);
		}
		if (retDstImg != NULL)
			memset(retDstImg, 0, nImgWidth * nImgHeight * sizeof(UCHAR));

		float fitR = m_fFatorR == 0.0 ? (float)1.0 : m_fFatorR;
		float fitG = m_fFatorG == 0.0 ? (float)1.0 : m_fFatorG;
		float fitB = m_fFatorB == 0.0 ? (float)1.0 : m_fFatorB;
		float fitBR = m_fFatorBR == 0.0 ? (float)1.0 : m_fFatorBR;
		float fitBB = m_fFatorBB == 0.0 ? (float)1.0 : m_fFatorBB;
		float compoBtmR = g_pMPTI->m_fCompoBtmR == 0.0 ? (float)1.0 : g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG == 0.0 ? (float)1.0 : g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB == 0.0 ? (float)1.0 : g_pMPTI->m_fCompoBtmB;
		int nCompoMode = g_pMPTI->GetCompositeLightMode();
		if (bAC)
		{
			fitR = 1.0f;
			fitG = 1.0f;
			fitB = 1.0f;
		}
		//if(sInspRoiImg.imgSide1_B!=NULL || sInspRoiImg.imgSide2_B!=NULL || sInspRoiImg.imgSide3_B!=NULL || sInspRoiImg.imgSide4_B!=NULL)
		if (nSideCameraIndex >= 0 && nSideCameraIndex <= 3)
		{
			fitR = 1;
			fitG = 1;
			fitB = 1;
		}
		int nRet = 0;
		if (bInspAC && retDstImgAC)
		{
			nRet = GetAngleColor(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, retDstImgAC);
			m_procMil->SaveWorkImg(retDstImgAC, nImgWidth, nImgHeight, _T("Rst_Color_Image_AC.bmp"), 3, false, bSaveR);
			if (nRet == eMR_FAIL)
				bResult = FALSE;
		}
		if (pInspAlgoColor->m_bUseColor == FALSE)
		{
			if (nRet == eMR_FAIL) bResult = FALSE;
			return bResult;
		}
		double dWhiteA = 0;
		if (pInspAlgoColor->m_byType == 1)
		{
			double dRate = 0;
			nRet = GetColorBin(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, pInspAlgoColor, retDstImg, &dRate, dWhiteA);
			if (nRet == eMR_FAIL)
				bResult = FALSE;
		}
		else
		{
			//UCHAR* ptrImgBuf = new UCHAR[nImgWidth * nImgHeight];
			UCHAR* ptrImgBuf = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
			int offsetX = CIE_OFFSETX;
			int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
			POINT pPtPolygon[POLYGON_POINT_CNTS];
			bResult = TRUE;
			for (int nCnt = 0; nCnt < pInspAlgoColor->m_nPolygonCnt; nCnt++)
			{
				if (nIDX > -1 && nIDX != nCnt)
					continue;
				memset(ptrImgBuf, 0, nImgWidth * nImgHeight * sizeof(UCHAR));
				RemakePoly(pInspAlgoColor->m_ptArrPolygon[nCnt], POLYGON_POINT_CNTS, offsetX, offsetY, pPtPolygon);

				memset(m_ucCIETab, 255, SIZE_CLRY * SIZE_CLRX * 3);

				if (nCompoMode == 0)
				{
					//for Debug
					/*cv::Mat imgMap(SIZE_CLRY, SIZE_CLRX, CV_8UC3, m_ucCIETab);
					cv::Mat imgR(nImgHeight, nImgWidth, CV_8UC1, img_R);
					cv::Mat imgG(nImgHeight, nImgWidth, CV_8UC1, img_G);
					cv::Mat imgB(nImgHeight, nImgWidth, CV_8UC1, img_B);*/

					if (rt.Width() > 0 && rt.Height() > 0)
						ImageToColorCIE_Color_Rect(img_R, img_G, img_B, fitR, fitG, fitB, m_ucCIETab, rt, nImgWidth);
					else
						ImageToColorCIE_Color(img_R, img_G, img_B, fitR, fitG, fitB, m_ucCIETab, nImgHeight, nImgWidth);

					//for (int i = 0; i < 3; i++)
					//{
					//	BOOL bCountinue = ShiftTabPolygon(m_ucCIETab, pPtPolygon, POLYGON_POINT_CNTS);
					//	if (bCountinue == FALSE)
					//		break;
					//}

					nRet = GetSelectColorCIEImg(img_R, img_G, img_B, fitR, fitG, fitB, nImgWidth, nImgHeight, pPtPolygon, POLYGON_POINT_CNTS, ptrImgBuf);

					CString csFileName = _T("");
					csFileName.Format(_T("Color_Image_Buf_%d.bmp"), nCnt);
					m_procMil->SaveWorkImg(ptrImgBuf, nImgWidth, nImgHeight, csFileName, 1, false, bSaveR);
				}
				else
				{
					ImageToColorCIE_Color_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, m_ucCIETab, nImgWidth, nImgHeight);
					
					for (int i = 0; i < 3; i++)
					{
						BOOL bCountinue = ShiftTabPolygon(m_ucCIETab, pPtPolygon, POLYGON_POINT_CNTS);
						if (bCountinue == FALSE)
							break;
					}

					nRet = GetSelectColorCIEImg_CompoBtm(img_R, img_G, img_B, img_BR, img_BB, fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB,
						nImgWidth, nImgHeight, pPtPolygon, POLYGON_POINT_CNTS, ptrImgBuf);

					CString csFileName = _T("");
					csFileName.Format(_T("Color_Image_Buf_BTM_%d.bmp"), nCnt);
					m_procMil->SaveWorkImg(ptrImgBuf, nImgWidth, nImgHeight, _T("Color_Image_Buf_BTM.bmp"), 1, false, bSaveR);
				}
				if (nRet == eMR_FAIL)
				{
					bResult = FALSE;
					break;
				}

				for (int n = 0; n < nImgWidth * nImgHeight; n++)
				{
					if (retDstImg[n] == 255 || ptrImgBuf[n] == 255)
						retDstImg[n] = (pInspAlgoColor->m_bInvert == FALSE) ? 255 : 0;
					else
						retDstImg[n] = (pInspAlgoColor->m_bInvert == FALSE) ? 0 : 255;
				}
			}

			if (pInspAlgoColor->m_bUseIntensity)
			{
				UCHAR* ucIntensMask = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
				memset(ucIntensMask, 255, nImgWidth * nImgHeight);

				BOOL bUseSide = nCompoMode == 0 ? FALSE : TRUE;
				BOOL bCalcMask = CalcIntensityMask(img_R, img_G, img_B, img_BR, img_BB, nImgWidth, nImgHeight, ucIntensMask, 
					pInspAlgoColor->m_nRangeType, pInspAlgoColor->m_nRangeMin, pInspAlgoColor->m_nRangeMax, bUseSide);
			
				cv::Mat ColorDst(nImgHeight, nImgWidth, CV_8UC1, retDstImg);
				cv::Mat ColorMask(nImgHeight, nImgWidth, CV_8UC1, ucIntensMask);

				cv::bitwise_and(ColorDst, ColorMask, ColorDst);

				g_pMManager->pem_delete(ucIntensMask, true);
			}

			g_pMManager->pem_delete(ptrImgBuf, true);
			
		}
		m_procMil->SaveWorkImg(retDstImg, nImgWidth, nImgHeight, _T("Rst_Color_Image.bmp"), 1, false, bSaveR);

		if (stTieArea)
		{
			stTieArea->m_nUsedInspPolygon = 0;
			stTieArea->m_nUsedMaskingValue = 0;
			CPInsp::FillOutOfInspAreaCombine(nImgWidth, nImgHeight, 0, retDstImg, NULL, *stTieArea);
			m_procMil->SaveWorkImg(retDstImg, nImgWidth, nImgHeight, _T("Rst_Color_Image_Poly.bmp"), 1, false, bSaveR);
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
	return bResult;
}

BOOL CPInsp_Color::ShiftTabPolygon(UCHAR* ucCIEMap, POINT* ArrPt, int nPtCnt)
{
	int nWhite = 255;
	int nBlack = 0;
	memset(m_ucMap, nBlack, SIZE_CLRY * SIZE_CLRX);

	//for Debug
	//cv::Mat viewimg(SIZE_CLRY, SIZE_CLRX, CV_8UC3, ucCIEMap);
	//cv::Mat testMapImg(SIZE_CLRY, SIZE_CLRX, CV_8UC1, m_ucMap);

	cv::Mat BinPoly = cv::Mat::zeros(SIZE_CLRY, SIZE_CLRX, CV_8UC1);
	cv::Mat BinDst(SIZE_CLRY, SIZE_CLRX, CV_8UC1);
	for (int y = 0; y < SIZE_CLRY; y++)
	{
		for (int x = 0; x < SIZE_CLRX; x++)
		{
			POINT tempPt;
			tempPt.x = x;
			tempPt.y = y;
			BOOL rst = FALSE;
			//colormap
			if (ucCIEMap[(y * (SIZE_CLRX * 3)) + ((x * 3) + 2)] == nBlack &&
				ucCIEMap[(y * (SIZE_CLRX * 3)) + ((x * 3) + 1)] == nBlack &&
				ucCIEMap[(y * (SIZE_CLRX * 3)) + ((x * 3) + 0)] == nBlack)
			{
				rst = PtInPolygon(tempPt, ArrPt, nPtCnt);
				if (rst == TRUE)
					m_ucMap[(y * SIZE_CLRY) + x] = nWhite;
			}

			//polygon
			rst = PtInPolygon(tempPt, ArrPt, nPtCnt);
			if (rst == TRUE)
				BinPoly.data[(y * SIZE_CLRY) + x] = nWhite;
		}
	}

	//map
	int nCntBlob = m_procMil->CalcBlob_Select(m_ucMap, BinDst.ptr(), SIZE_CLRX, SIZE_CLRY, 4, false, false, 0, eSelectBigger);
	
	if (nCntBlob <= 0)
		return false;
	
	double* dCenterX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	double* dCenterY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	m_procMil->GetBlobResult_Center(dCenterX, dCenterY);

	cv::Point2d ptMapCenter;
	ptMapCenter.x = dCenterX[0];
	ptMapCenter.y = dCenterY[0];
	
	//polygon
	nCntBlob = m_procMil->CalcBlob_Select(BinPoly.ptr(), BinDst.ptr(), SIZE_CLRX, SIZE_CLRY, 4, false, false, 0, eSelectBigger);
	m_procMil->GetBlobResult_Center(dCenterX, dCenterY);

	cv::Point2d ptPolyCenter;
	ptPolyCenter.x = dCenterX[0];
	ptPolyCenter.y = dCenterY[0];

	//
	int nShiftX = ptMapCenter.x - ptPolyCenter.x;
	int nShiftY = ptMapCenter.y - ptPolyCenter.y;
	for (int i = 0; i < nPtCnt; i++)
	{
		ArrPt[i].x += nShiftX;
		ArrPt[i].y += nShiftY;
	}

	g_pMManager->pem_delete(dCenterX, true);
	g_pMManager->pem_delete(dCenterY, true);

	if (abs(nShiftX) < 3 && abs(nShiftY) < 3)
		return FALSE;
	else
		return TRUE;
}
int CPInsp_Color::ImageToColorCIE_Color_Rect(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, RECT rcRoi, int widthStep)
{
	int nH = rcRoi.bottom;
	int nW = rcRoi.right;

	for (int i = rcRoi.top; i < nH; i = i + 10)
	{
		for (int j = rcRoi.left; j < nW; j = j + 10)
		{
			float r = ((float)img_R[i*widthStep + j] * factorR);		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)img_G[i*widthStep + j] * factorG);		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)img_B[i*widthStep + j] * factorB);		b = _limit_value(b, 0.f, 255.f);
			float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
			float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
			float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
			float sumXYZ = cx + cy + cz;
			if (sumXYZ)
			{
				int nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
				int ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));

				dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 2)] = 0;
				dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 1)] = 0;
				dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 0)] = 0;
			}
		}
	}

	return ePART_SUCCESS;
}
int CPInsp_Color::ImageToColorCIE_Color_Rect_CompoBtm(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
	float compoBtmR, float compoBtmG, float compoBtmB, UCHAR* dst, RECT rcRoi, int width, int length)
{
	int nH = rcRoi.bottom;
	int nW = rcRoi.right;

	for (int i = rcRoi.top; i < nH; i = i + 10)
	{
		for (int j = rcRoi.left; j < nW; j = j + 10)
		{
			float fBR = (float)(img_BR[i] * factorBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
			float fBB = (float)(img_BB[i] * factorBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

			float fBGR = 0.4f;
			float fBGB = 0.8f;

			float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

			float r = ((float)img_R[i] * factorR) + fBR;		r = _limit_value(r, 0.f, 255.f);
			float g = ((float)img_G[i] * factorG) + fBG;		g = _limit_value(g, 0.f, 255.f);
			float b = ((float)img_B[i] * factorB) + fBB;		b = _limit_value(b, 0.f, 255.f);
			float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
			float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
			float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));
			float sumXYZ = cx + cy + cz;

			if (sumXYZ)
			{
				int nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
				int ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));

				// fill black
				dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 2)] = 0;
				dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 1)] = 0;
				dst[((SIZE_CLRY - ny) * (SIZE_CLRX * 3)) + ((nx * 3) + 0)] = 0;
			}
		}
	}

	return ePART_SUCCESS;
}
BOOL CPInsp_Color::CalcIntensityMask(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB,
	int width, int length, UCHAR* ucDst, int nRange, int nGrayMin, int nGrayMax, BOOL bUseSide)
{
	if (ucDst == NULL || width <= 0 || length <= 0)
		return FALSE;
	if ((bUseSide == true) && (img_BR == NULL || img_BB == NULL))
		return FALSE;

	UCHAR* ucMean = g_pMManager->pem_new<UCHAR>(true, length * width, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ucTmpDst = NULL;
	memset(ucMean, 0, length * width);
	BOOL bRet = FALSE;


	int nMeanDiv = bUseSide == FALSE ? 3 : 5;

	register int x = 0, y = 0;

	int nMean = 0;
	if (bUseSide == FALSE)
	{
		for (y = 0; y < length; y++)
		{
			for (x = 0; x < width; x++)
			{
				nMean = 0;
				nMean += img_R[y * width + x];
				nMean += img_G[y * width + x];
				nMean += img_B[y * width + x];
				nMean /= nMeanDiv;
				ucMean[y * width + x] = nMean;
			}
		}
	}
	else
	{
		for (y = 0; y < length; y++)
		{
			for (x = 0; x < width; x++)
			{
				nMean = 0;
				nMean += img_R[y * width + x];
				nMean += img_G[y * width + x];
				nMean += img_B[y * width + x];
				nMean += img_BR[y * width + x];
				nMean += img_BB[y * width + x];
				nMean /= nMeanDiv;
				ucMean[y * width + x] = nMean;
			}
		}
	}

	Binarize(ucMean, width, length, nRange, nGrayMin, nGrayMax, FALSE, ucTmpDst);

	memcpy(ucDst, ucTmpDst, width * length);

	g_pMManager->pem_delete(ucMean, true);
	g_pMManager->pem_delete(ucTmpDst, true);

	return TRUE;
}