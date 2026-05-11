#include "StdAfx.h"
#include "MPTI.h"
#include "PInsp_Grid.h"
#include "QTimer.h"
#include "compute.h"

CPInsp_Grid::CPInsp_Grid(void)
{
	m_className = _T("CPInsp_Grid");
	m_eAlgoType = eAlgoGrid;

	m_procMil = nullptr;
	m_pInspBoardInfo = nullptr;

	m_nColumn = m_nRow = 0;
}

CPInsp_Grid::~CPInsp_Grid(void)
{
}


int CPInsp_Grid::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	CPInsp_Color::InitDevice(milApp, milSys, fovWidth, fovLength ,resolX, resolY,bUseImagePilLib);

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength ,resolX, resolY,bUseImagePilLib);
	m_pProcMilAlgo->InitMilAlgoBlob();

	return ePART_SUCCESS;
}



int CPInsp_Grid::SetInspParam(const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, 
	const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo)
{
	if(sInspAlgo.m_eAlgoType != eAlgoGrid)
		return ePART_FAIL;

	m_pAlgoGrid = (AlgoGrid *)sInspAlgo.m_ptrInspAlgoParam;

	if(m_pAlgoGrid)
		m_pAlgoGrid->sAlgoColor.GetParam(m_pInspParam);

	m_pInspBoardInfo = (InspPartInfo *)pInspBoardInfo;

	if(pInspBoardInfo)
	{
		m_pPartImgBuf[eImg_Top_R]		= pInspBoardInfo->partImgBuf.imgTop_R;
		m_pPartImgBuf[eImg_Top_G]		= pInspBoardInfo->partImgBuf.imgTop_G;
		m_pPartImgBuf[eImg_Top_B]		= pInspBoardInfo->partImgBuf.imgTop_B;
		m_pPartImgBuf[eImg_Top_W]		= pInspBoardInfo->partImgBuf.imgTop_W;
		m_pPartImgBuf[eImg_Middle_R]	= pInspBoardInfo->partImgBuf.imgMiddle_R;
		m_pPartImgBuf[eImg_Middle_B]	= pInspBoardInfo->partImgBuf.imgMiddle_B;
		m_pPartImgBuf[eImg_Bottom_R]	= pInspBoardInfo->partImgBuf.imgBottom_R;
		m_pPartImgBuf[eImg_Bottom_B]	= pInspBoardInfo->partImgBuf.imgBottom_B;

		m_nPartWidth = pInspBoardInfo->partImgBuf.nImageSizeX;
		m_nPartHeight = pInspBoardInfo->partImgBuf.nImageSizeY;
	}

	m_inspCoordinate.cx = RounD(coordinateAlgo.dROICenterX - coordinateAlgo.dROIWidth / 2.);
	m_inspCoordinate.cy = RounD(coordinateAlgo.dROICenterY - coordinateAlgo.dROILength / 2.);
	m_inspCoordinate.width = RounD(coordinateAlgo.dROIWidth);
	m_inspCoordinate.length = RounD(coordinateAlgo.dROILength);
	m_inspCoordinate.angle = coordinateAlgo.dROIAngle;

	m_wndAlgoImg = sWndAlgoImg;

	if(m_img_Mix->empty()==true)
		return ePART_FAIL;

	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_R][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iTRed.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_G][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iTGreen.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_B][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iTBlue.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_W][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iTWhite.bmp"));

	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Middle_R][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iMRed.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Middle_B][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iMBlue.bmp"));

	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Bottom_R][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iBRed.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Bottom_B][0], m_inspCoordinate.width, m_inspCoordinate.length, _T("Grid_iBBlue.bmp"));

	return ePART_SUCCESS;
}

int CPInsp_Grid::SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, 
	const WndInfo &wndInfoAnyAngle, const InspPartInfo *pInspBoardInfo, bool bIsLoadFovRawDataImage, int projectionmode)
{
	if(sInspAlgo.m_eAlgoType != eAlgoGrid)
		return ePART_FAIL;

	m_pAlgoGrid = (AlgoGrid *)sInspAlgo.m_ptrInspAlgoParam;
	if(m_pAlgoGrid)
		m_pAlgoGrid->sAlgoColor.GetParam(m_pInspParam);

	m_pFovImage_insp_Main = NULL;

	m_eAlgoType = sInspAlgo.m_eAlgoType;
	if(m_pAlgoColor)
		m_pAlgoColor->GetParam(m_pInspParam);

	m_pInspBoardInfo = (InspPartInfo *)pInspBoardInfo;

	if(pInspBoardInfo)
	{
		m_imgBuf[eImg_Top_R]	= pInspBoardInfo->fovImgBuf.imgTop_R;
		m_imgBuf[eImg_Top_G]	= pInspBoardInfo->fovImgBuf.imgTop_G;
		m_imgBuf[eImg_Top_B]	= pInspBoardInfo->fovImgBuf.imgTop_B;
		m_imgBuf[eImg_Top_W]	= pInspBoardInfo->fovImgBuf.imgTop_W;
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
		SetParamChannelImg(sInspAlgo, coordinateAlgo, TRUE, bIsLoadFovRawDataImage, projectionmode);
	}

	m_nPartWidth = GetWidthImg(m_inspCoordinate);
	m_nPartHeight = GetHeightImg(m_inspCoordinate);

	m_inspCoordinate.SetParamROI(coordinateAlgo);
	m_inspCoordinate.angle = coordinateAlgo.dROIAngle;
	m_inspCoordinate.anyAngleCx = RounD(wndInfoAnyAngle.dCenterX / m_resolX);
	m_inspCoordinate.anyAngleCy = RounD(wndInfoAnyAngle.dCenterY / m_resolY);
	m_inspCoordinate.anyAngleWidth = RounD(wndInfoAnyAngle.dWidth / m_resolX);
	m_inspCoordinate.anyAngleLength = RounD(wndInfoAnyAngle.dLength / m_resolY);

//	m_wndAlgoImg = sWndAlgoImg;


	_MakeColorBuf_Mix_Teach(bIsLoadFovRawDataImage);

	return ePART_SUCCESS;
}

void CPInsp_Grid::_MakeColorBuf_Mix_Teach(bool bIsLoadFovRawDataImage)
{
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	for (int i=0; i<InspImg_Kind::eImg_BufCnt; i++)
		m_img_Mix[i].resize(width * length);
	
	MakeProcImg(m_imgBuf[InspImg_Kind::eImg_Top_R], m_imgBuf[InspImg_Kind::eImg_Top_G], m_imgBuf[InspImg_Kind::eImg_Top_B], 
				&m_img_Mix[InspImg_Kind::eImg_Top_R][0], &m_img_Mix[InspImg_Kind::eImg_Top_G][0], &m_img_Mix[InspImg_Kind::eImg_Top_B][0], m_inspCoordinate, bIsLoadFovRawDataImage);
	MakeProcImg(m_imgBuf[InspImg_Kind::eImg_Top_W], nullptr, nullptr, 
		&m_img_Mix[InspImg_Kind::eImg_Top_W][0], nullptr, nullptr, m_inspCoordinate, bIsLoadFovRawDataImage);

	MakeProcImg(m_imgBuf[InspImg_Kind::eImg_Middle_R], nullptr, m_imgBuf[InspImg_Kind::eImg_Middle_B], 
				&m_img_Mix[InspImg_Kind::eImg_Middle_R][0], nullptr, &m_img_Mix[InspImg_Kind::eImg_Middle_B][0], m_inspCoordinate, bIsLoadFovRawDataImage);
	MakeProcImg(m_imgBuf[InspImg_Kind::eImg_Bottom_R], nullptr, m_imgBuf[InspImg_Kind::eImg_Bottom_B], 
				&m_img_Mix[InspImg_Kind::eImg_Bottom_R][0], nullptr, &m_img_Mix[InspImg_Kind::eImg_Bottom_B][0], m_inspCoordinate, bIsLoadFovRawDataImage);

	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_R][0], width, length, _T("Grid_TRed.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_G][0], width, length, _T("Grid_TGreen.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_B][0], width, length, _T("Grid_TBlue.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Top_W][0], width, length, _T("Grid_TWhite.bmp"));

	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Middle_R][0], width, length, _T("Grid_MRed.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Middle_B][0], width, length, _T("Grid_MBlue.bmp"));

	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Bottom_R][0], width, length, _T("Grid_BRed.bmp"));
	m_procMil->SaveWorkImg(&m_img_Mix[InspImg_Kind::eImg_Bottom_B][0], width, length, _T("Grid_BBlue.bmp"));
}



int CPInsp_Grid::ProcAuto()
{
	int ret = e_OK;
	int stepID = 0;
	int maxStepCnts = 4;

	for(int i = 0; i <= maxStepCnts; i++)
	{
		ret = ProcStep(i);

		stepID++;

		if(ret != e_OK) 
			break;
	}

	return ret;
}

int CPInsp_Grid::ProcStep(int stepID)
{
	int ret = e_OK;

	switch(stepID)
	{
	case 0:
		_CheckPartAngle();
		m_Buffer.Init(m_nColumn, m_nRow);
		break;
	case 1:
		CalcAllCell();
		break;
	case 2:
		IgnoreSelectedArea();
		break;
	case 3:
		BlobAnalysis();
		break;
	case 4:
		Decision();
		break;
	}

	return ret;
}


void CPInsp_Grid::MakeMask3d()
{
	if(m_pAlgoGrid->m_b3dCheck==false)
		return ;

	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	float * Data3d = m_wndAlgoImg.m_fArr3D;
	UCHAR * pMaskBuf = &m_Buffer.m_vMaskBuf[0];

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = RounDF((float)(width) / fCols);
	int nLenCell = RounDF((float)(length) / fRows);

	int nRangeMode = m_pAlgoGrid->m_n3dRange;
	float nRangeMin = m_pAlgoGrid->m_d3dHeightMin;
	float nRangeMax = m_pAlgoGrid->m_d3dHeightMax;

	for (int y=0 ; y<length ; y++)
	{
		int j = y * width;
		for (int x=0 ; x<width ; x++)
		{
			int iy = y / nLenCell;
			int ix = x / nWidCell;
			if(iy<nRows && ix<nCols)
			{
				int i = j+x;
			}
		}
	}

	m_procMil->SaveWorkImg(pMaskBuf, nCols, nRows, _T("Grid_Mask3d.bmp"));
}

inline bool CPInsp_Grid::_Threshold3d(int nRangeMode, float nRangeMin, float nRangeMax, float nGray)
{
	if(m_pAlgoGrid->m_b3dCheck==false)
		return true;

	if(nRangeMode == eTypeRangeIn)
	{
		if(nGray >= nRangeMin && nGray<=nRangeMax)
		{
			return true;
		}
	}
	else if(nRangeMode == eTypeRangeOut)
	{
		if(nGray < nRangeMin || nGray>nRangeMax)
		{
			return true;
		}
	}
	else if(nRangeMode == eTypeRangeUpper)
	{
		if(nGray > nRangeMax)
		{
			return true;
		}
	}
	else if(nRangeMode == eTypeRangeLower)
	{
		if(nGray < nRangeMin)
		{
			return true;
		}
	}

	return false;
}


void CPInsp_Grid::CalcAllCell()
{
	if(m_pAlgoGrid->m_bIgnoreUse == false)
		return;

	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = (fCols > 0) ? RounDF((float)(width) / fCols) : 0;
	int nLenCell = (fRows > 0) ? RounDF((float)(length) / fRows) : 0;
	if (nWidCell < 1) nWidCell = 1;
	if (nLenCell < 1) nLenCell = 1;
	if (nWidCell < 1) nWidCell = 1;
	if (nLenCell < 1) nLenCell = 1;

	//
	UCHAR * pImg = m_wndAlgoImg.m_ucArr2D_Mix[0];

	m_procMil->SaveWorkImg(pImg, m_wndAlgoImg.m_nWidth, m_wndAlgoImg.m_nHeight, _T("Grid_MixGray.bmp"));

	float * pAvg = &m_Buffer.m_vImg_Avg[0];
	int * pCntAvg = &m_Buffer.m_vImg_CntAvg[0];

	float * pStdev = &m_Buffer.m_vImg_Stdev[0];
	int * pCntStdev = &m_Buffer.m_vImg_CntStdev[0];

	UCHAR * pImgRes = &m_Buffer.m_vImg_Res[0];

	for (int y=0; y<length; y++)
	{
		for (int x=0; x<width; x++)
		{
			int iy = y / nLenCell;
			int ix = x / nWidCell;
			if(iy<nRows && ix<nCols)
			{
				pAvg[iy*nCols+ix] += pImg[y*width+x];
				pCntAvg[iy*nCols+ix]++;
			}
		}
	}

	for (int y=0; y<nRows; y++)
	{
		for (int x=0; x<nCols; x++)
		{
			pAvg[y*nCols+x] /= pCntAvg[y*nCols+x];
		}
	}

	// 평균값 검사 시
	if(m_pAlgoGrid->m_bStdUse == false)
	{
		for (int y=0; y<nRows; y++)
		{
			for (int x=0; x<nCols; x++)
			{
				pImgRes[y*nCols+x] = pAvg[y*nCols+x];
			}
		}
		m_procMil->SaveWorkImg(pImgRes, nCols, nRows, _T("Grid_CalcedGray.bmp"));
		return;
	}
	else
	{
		for (int y=0; y<length; y++)
		{
			for (int x=0; x<width; x++)
			{
				int iy = y / nLenCell;
				int ix = x / nWidCell;

				if(iy<nRows && ix<nCols)
				{
					pStdev[iy*nCols+ix] += pow(pImg[y*width+x] - pAvg[iy*nCols+ix], 2);
					pCntStdev[iy*nCols+ix]++;
				}
			}
		}

		for (int y=0; y<nRows; y++)
		{
			for (int x=0; x<nCols; x++)
			{
				pStdev[y*nCols+x] = sqrt(pStdev[y*nCols+x]/(pCntStdev[y*nCols+x]-1));
				pImgRes[y*nCols+x] = RounDF(pStdev[y*nCols+x]);
			}
		}
	}

	m_procMil->SaveWorkImg(pImgRes, nCols, nRows, _T("Grid_CalcedGray.bmp"));
}

void CPInsp_Grid::IgnoreSelectedArea(bool bIsLoadFovRawDataImage)
{
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);
	int ptCnts = m_pInspParam->pointCnts;
	double stdRate = m_pInspParam->stdRate;
	int nCompoLightMode = g_pMPTI->GetCompositeLightMode();

	float fitR = m_pInspParam->redFator;
	float fitG = m_pInspParam->greenFator;
	float fitB = m_pInspParam->blueFator;

	int dstSizeX = m_pInspParam->sizeX_CIE;
	int dstSizeY = m_pInspParam->sizeY_CIE;

	int offsetX = CIE_OFFSETX;
	int offsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
	POINT pPt[POLYGON_POINT_CNTS];
	RemakePoly(m_pInspParam->polygonPt[0], POLYGON_POINT_CNTS, offsetX, offsetY, pPt);

	std::vector<UCHAR> vImg_R(width * length);
	std::vector<UCHAR> vImg_G(width * length);
	std::vector<UCHAR> vImg_B(width * length);
	std::vector<UCHAR> vImg_BR(width * length);
	std::vector<UCHAR> vImg_BB(width * length);

	_MakeProcImg(&vImg_R[0], &vImg_G[0], &vImg_B[0], &vImg_BR[0], &vImg_BB[0], bIsLoadFovRawDataImage);

	if(nCompoLightMode == 0)
		_CalcCIE(&vImg_R[0], &vImg_G[0], &vImg_B[0], fitR, fitG, fitB, width, length, pPt, ptCnts, &m_Buffer.m_vImg_Res[0]);
	else
	{
		float fitBR = m_pInspParam->redFactorBtm;
		float fitBB = m_pInspParam->blueFactorBtm;
		float compoBtmR = g_pMPTI->m_fCompoBtmR;
		float compoBtmG = g_pMPTI->m_fCompoBtmG;
		float compoBtmB = g_pMPTI->m_fCompoBtmB;

		_CalcCIE__CompoBtm(&vImg_R[0], &vImg_G[0], &vImg_B[0], &vImg_BR[0], &vImg_BB[0], fitR, fitG, fitB, fitBR, fitBB, compoBtmR, compoBtmG, compoBtmB, width, length, pPt, ptCnts, &m_Buffer.m_vImg_Res[0]);
	}
}

void CPInsp_Grid::_MakeProcImg(UCHAR * img_R, UCHAR * img_G, UCHAR * img_B, UCHAR * imgBR, UCHAR * imgBB, bool bIsLoadFovRawDataImage)
{
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);
	int nCompoLightMode = g_pMPTI->GetCompositeLightMode();

	if(m_pInspBoardInfo == nullptr)
	{
		if(nCompoLightMode == 0)
		{
			void * temp_R = m_imgBuf[eImg_Top_R];
			void * temp_G = m_imgBuf[eImg_Top_G];
			void * temp_B = m_imgBuf[eImg_Top_B];

			m_procMil->SaveWorkImg((Im::PIL_ID)temp_R, _T("Grid_imgRed.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_G, _T("Grid_imgGreen.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_B, _T("Grid_imgBlue.bmp"));

			MakeProcImg(temp_R, temp_G, temp_B, img_R, img_G, img_B, m_inspCoordinate, bIsLoadFovRawDataImage);
		}
		else
		{
			void * temp_R = m_imgBuf[eImg_Top_R];
			void * temp_G = m_imgBuf[eImg_Top_G];
			void * temp_B = m_imgBuf[eImg_Top_B];
			void * temp_BR = m_imgBuf[eImg_Bottom_R];
			void * temp_BB = m_imgBuf[eImg_Bottom_B];

			m_procMil->SaveWorkImg((Im::PIL_ID)temp_R, _T("Grid_imgRed.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_G, _T("Grid_imgGreen.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_B, _T("Grid_imgBlue.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_BR, _T("Grid_imgBtmRed.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_BB, _T("Grid_imgBtmBlue.bmp"));

			MakeProcImg(temp_R, temp_G, temp_B, img_R, img_G, img_B, m_inspCoordinate, bIsLoadFovRawDataImage, false, temp_BR, temp_BB, imgBR, imgBB);
		}
		
	}
	else
	{
		if(nCompoLightMode == 0)
		{
			UCHAR * temp_R = m_pPartImgBuf[eImg_Top_R];
			UCHAR * temp_G = m_pPartImgBuf[eImg_Top_G];
			UCHAR * temp_B = m_pPartImgBuf[eImg_Top_B];

			m_procMil->SaveWorkImg(temp_R, m_nPartWidth, m_nPartHeight, _T("Grid_partRed.bmp"));
			m_procMil->SaveWorkImg(temp_G, m_nPartWidth, m_nPartHeight, _T("Grid_partGreen.bmp"));
			m_procMil->SaveWorkImg(temp_B, m_nPartWidth, m_nPartHeight, _T("Grid_partBlue.bmp"));

			MakeProcImg(temp_R, temp_G, temp_B, img_R, img_G, img_B, m_inspCoordinate);
		}
		else
		{
			void * temp_R = m_pPartImgBuf[eImg_Top_R];
			void * temp_G = m_pPartImgBuf[eImg_Top_G];
			void * temp_B = m_pPartImgBuf[eImg_Top_B];
			void * temp_BR = m_pPartImgBuf[eImg_Bottom_R];
			void * temp_BB = m_pPartImgBuf[eImg_Bottom_B];

			m_procMil->SaveWorkImg((Im::PIL_ID)temp_R, _T("Grid_imgRed.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_G, _T("Grid_imgGreen.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_B, _T("Grid_imgBlue.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_BR, _T("Grid_imgBtmRed.bmp"));
			m_procMil->SaveWorkImg((Im::PIL_ID)temp_BB, _T("Grid_imgBtmBlue.bmp"));

			MakeProcImg(temp_R, temp_G, temp_B, img_R, img_G, img_B, m_inspCoordinate, temp_BR, temp_BB, imgBR, imgBB);
		}
	}

	m_procMil->SaveWorkImg(img_R, width, length, _T("Grid_windowRed.bmp"));
	m_procMil->SaveWorkImg(img_G, width, length, _T("Grid_windowGreen.bmp"));
	m_procMil->SaveWorkImg(img_B, width, length, _T("Grid_windowBlue.bmp"));
	m_procMil->SaveWorkImg(imgBR, width, length, _T("Grid_windowBtmRed.bmp"));
	m_procMil->SaveWorkImg(img_B, width, length, _T("Grid_windowBtmBlue.bmp"));
}

void CPInsp_Grid::_CalcCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, 
	int width, int length, POINT * pts, int ptNum, UCHAR * mask)
{
	if(srcR == NULL || srcG == NULL || srcB == NULL || width <= 0 || length <= 0)
		return ;

	int nRangeMode = m_pAlgoGrid->sAlgoColor.m_nRangeMode;
	int nRangeMin = m_pAlgoGrid->sAlgoColor.m_nRangeMin;
	int nRangeMax = m_pAlgoGrid->sAlgoColor.m_nRangeMax;

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = RounDF((float)(width) / fCols);
	int nLenCell = RounDF((float)(length) / fRows);
	if (nWidCell < 1) nWidCell = 1;
	if (nLenCell < 1) nLenCell = 1;

	UCHAR * pMixGray = m_wndAlgoImg.m_ucArr2D;

	m_procMil->SaveWorkImg(pMixGray, width, length, _T("Grid_CalcCIEUcArrCV.bmp"));

	UCHAR * pData = &m_Buffer.m_vImg_Res[0];
	UCHAR * pMaskBuf = &m_Buffer.m_vMaskBuf[0];

	bool b3dCheck = m_pAlgoGrid->m_b3dCheck;

	float * Data3d = m_wndAlgoImg.m_fArr3D;
	int n3dRangeMode = m_pAlgoGrid->m_n3dRange;
	float f3dRangeMin = m_pAlgoGrid->m_d3dHeightMin;
	float f3dRangeMax = m_pAlgoGrid->m_d3dHeightMax;

	int size = width * length;
	for (int y=0 ; y<length ; y++)
	{
		int j = y * width;
		for (int x=0 ; x<width ; x++)
		{
			int iy = y / nLenCell;
			int ix = x / nWidCell;
			if(iy<nRows && ix<nCols)
			{
				int i = j+x;

				float r = ((float)srcR[i] * factorR);		r = _limit_value(r, 0.f, 255.f);
				float g = ((float)srcG[i] * factorG);		g = _limit_value(g, 0.f, 255.f);
				float b = ((float)srcB[i] * factorB);		b = _limit_value(b, 0.f, 255.f);

				float fData3D = 0.0f;
				if(m_wndAlgoImg.m_fArr3D != NULL && m_wndAlgoImg.m_fArr3D != nullptr)
					fData3D = Data3d[i];
				if(_Threshold3d(n3dRangeMode, f3dRangeMin, f3dRangeMax, fData3D)==true && _Color_GetCxy(r, g, b, pts, ptNum) == true && _ThresholdGray(nRangeMode, nRangeMin, nRangeMax, pMixGray[i])==true)
				{
					pMaskBuf[iy*nCols+ix] = 255;
					pData[iy*nCols+ix] = 0;
				}
			}
		}
	}

	m_procMil->SaveWorkImg(pMaskBuf, nCols, nRows, _T("Grid_MaskImg.bmp"));
}

void CPInsp_Grid::_CalcCIE__CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
	float compoBtmR, float compoBtmG, float compoBtmB, int width, int length, POINT * pts, int ptNum, UCHAR * mask)
{
	if(srcR == NULL || srcG == NULL || srcB == NULL || width <= 0 || length <= 0 || srcBR == NULL || srcBB == NULL)
		return ;

	int nRangeMode = m_pAlgoGrid->sAlgoColor.m_nRangeMode;
	int nRangeMin = m_pAlgoGrid->sAlgoColor.m_nRangeMin;
	int nRangeMax = m_pAlgoGrid->sAlgoColor.m_nRangeMax;

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = RounDF((float)(width) / fCols);
	int nLenCell = RounDF((float)(length) / fRows);
	UCHAR * pMixGray = m_wndAlgoImg.m_ucArr2D;

	m_procMil->SaveWorkImg(pMixGray, width, length, _T("Grid_CalcCIEUcArrCV.bmp"));

	UCHAR * pData = &m_Buffer.m_vImg_Res[0];
	UCHAR * pMaskBuf = &m_Buffer.m_vMaskBuf[0];

	bool b3dCheck = m_pAlgoGrid->m_b3dCheck;

	float * Data3d = m_wndAlgoImg.m_fArr3D;
	int n3dRangeMode = m_pAlgoGrid->m_n3dRange;
	float f3dRangeMin = m_pAlgoGrid->m_d3dHeightMin;
	float f3dRangeMax = m_pAlgoGrid->m_d3dHeightMax;

	int size = width * length;
	for (int y=0 ; y<length ; y++)
	{
		int j = y * width;
		for (int x=0 ; x<width ; x++)
		{
			int iy = y / nLenCell;
			int ix = x / nWidCell;
			if(iy<nRows && ix<nCols)
			{
				int i = j+x;

				float fBR = (float)(srcBR[i] * factorBR) * compoBtmR;		fBR = _limit_value(fBR, 0.f, 255.f);
				float fBB = (float)(srcBB[i] * factorBB) * compoBtmB;		fBB = _limit_value(fBB, 0.f, 255.f);

				float fBGR = 0.4f;
				float fBGB = 0.8f;

				float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;

				float r = ((float)srcR[i] * factorR) + fBR;		r = _limit_value(r, 0.f, 255.f);
				float g = ((float)srcG[i] * factorG) + fBG;		g = _limit_value(g, 0.f, 255.f);
				float b = ((float)srcB[i] * factorB) + fBB;		b = _limit_value(b, 0.f, 255.f);


				if(_Threshold3d(n3dRangeMode, f3dRangeMin, f3dRangeMax, Data3d[i])==true && _Color_GetCxy(r, g, b, pts, ptNum) == true && _ThresholdGray(nRangeMode, nRangeMin, nRangeMax, pMixGray[i])==true)
				{
					pMaskBuf[iy*nCols+ix] = 255;
					pData[iy*nCols+ix] = 0;
				}
			}
		}
	}

	m_procMil->SaveWorkImg(pMaskBuf, nCols, nRows, _T("Grid_MaskImg.bmp"));
}

inline bool CPInsp_Grid::_Color_GetCxy(float r, float g, float b, POINT * pts, int ptNum)
{
	if(m_pAlgoGrid->m_nIgnoreColor == grid_IgnoreMode::eGray)
		return true;
	else if(m_pAlgoGrid->m_nIgnoreColor == eNone)
		return false;

	float cx = ((CIE_XR * (float)r) + (CIE_XG * (float)g) + (CIE_XB * (float)b));
	float cy = ((CIE_YR * (float)r) + (CIE_YG * (float)g) + (CIE_YB * (float)b));
	float cz = ((CIE_ZR * (float)r) + (CIE_ZG * (float)g) + (CIE_ZB * (float)b));

	float sumXYZ = cx + cy + cz;
	if (sumXYZ)
	{
		POINT pt;

		pt.x = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0, (SIZE_CLRX - 1));
		int ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0, (SIZE_CLRY - 1));
		pt.y = SIZE_CLRY - ny;

		return PtInPolygon(pt, pts, ptNum);
	}

	return false;
}

inline bool CPInsp_Grid::_ThresholdGray(int nRangeMode, int nRangeMin, int nRangeMax, UCHAR nGray)
{
	if(m_pAlgoGrid->m_nIgnoreColor == grid_IgnoreMode::eColor)
		return true;
	else if(m_pAlgoGrid->m_nIgnoreColor == grid_IgnoreMode::eNone)
		return false;

	if(nRangeMode == eTypeRangeIn)
	{
		if(nGray >= nRangeMin && nGray<=nRangeMax)
		{
			return true;
		}
	}
	else if(nRangeMode == eTypeRangeOut)
	{
		if(nGray < nRangeMin || nGray>nRangeMax)
		{
			return true;
		}
	}
	else if(nRangeMode == eTypeRangeUpper)
	{
		if(nGray > nRangeMax)
		{
			return true;
		}
	}
	else if(nRangeMode == eTypeRangeLower)
	{
		if(nGray < nRangeMin)
		{
			return true;
		}
	}

	return false;
}

void CPInsp_Grid::BlobAnalysis()
{
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = RounDF((float)(width) / fCols);
	int nLenCell = RounDF((float)(length) / fRows);

	Im::PIL_ID bin = m_pProcMilAlgo->AllocBuff(nCols, nRows);

	// Y_TODO: exclude 모드가 아닐때는 mask영상을 검사한다.
	if(m_pAlgoGrid->m_bIgnoreUse == true)
	{
		m_procMil->SaveWorkImg(&m_Buffer.m_vImg_Res[0], nCols, nRows, _T("Grid_Threshold.bmp"));
		for (int a = 0; a < nRows * nCols; a++)
		{
			if(m_pAlgoGrid->m_bArrExceptArea[a] == true)
				m_Buffer.m_vImg_Res[a] = 0;
		}
		m_procMil->SaveWorkImg(&m_Buffer.m_vImg_Res[0], nCols, nRows, _T("Grid_Threshold1.bmp"));
		Im::Buf::Put2d(bin, 0, 0, nCols, nRows, &m_Buffer.m_vImg_Res[0]);
		m_pProcMilAlgo->Binarize(bin, bin, m_pAlgoGrid->dStdDev, FALSE);
	}
	else
	{
		m_procMil->SaveWorkImg(&m_Buffer.m_vMaskBuf[0], nCols, nRows, _T("Grid_Threshold2.bmp"));
		for (int a = 0; a < nRows * nCols; a++)
		{
			if(m_pAlgoGrid->m_bArrExceptArea[a] == true)
				m_Buffer.m_vMaskBuf[a] = 0;
		}
		m_procMil->SaveWorkImg(&m_Buffer.m_vMaskBuf[0], nCols, nRows, _T("Grid_Threshold3.bmp"));
		Im::Buf::Put2d(bin, 0, 0, nCols, nRows, &m_Buffer.m_vMaskBuf[0]);
	}
	m_procMil->SaveWorkImg(bin, _T("Grid_BlobAnalysis1.bmp"));
	int nCount = m_pProcMilAlgo->CalcBlob(bin, 4, 0, 0, 0, bin);
	m_procMil->SaveWorkImg(bin, _T("Grid_BlobAnalysis2.bmp"));

	double dArea = 0.0;
	CMilBlobResult BlobResult(nCount);
	m_pProcMilAlgo->GetBlobResult(&BlobResult);
	for (int i=0; i<BlobResult.count; i++)
		dArea += BlobResult.area[i];

	m_Buffer.dArea = dArea * nWidCell * nLenCell;

	Im::Buf::Get2d(bin, 0, 0, nCols, nRows, &m_Buffer.m_vImg_Res[0]);
	Im::Buf::Free(bin);
	for (int a = 0; a < nRows * nCols; a++)
	{
		if(m_pAlgoGrid->m_bArrExceptArea[a] == true)
			m_Buffer.m_vMaskBuf[a] = 1;
	}
}

void CPInsp_Grid::Decision()
{
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = RounDF((float)(width) / fCols);
	int nLenCell = RounDF((float)(length) / fRows);

	double LimitArea = m_pAlgoGrid->m_dDetectWidth * m_pAlgoGrid->m_dDetectLength;
	m_Buffer.dAreaPer = m_Buffer.dArea * GetResolX() * GetResolY();

	if(LimitArea < m_Buffer.dAreaPer)
		m_Buffer.bResultOK = FALSE;
	else
		m_Buffer.bResultOK = TRUE;
}


void CPInsp_Grid::GetBlobImage(std::vector<UCHAR> & vBin)
{
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = RounDF((float)(width) / fCols);
	int nLenCell = RounDF((float)(length) / fRows);
	if (nWidCell < 1) nWidCell = 1;
	if (nLenCell < 1) nLenCell = 1;

	int rsW = nCols*nWidCell;
	int rsL = nRows*nLenCell;

	int cpW = rsW>width?width:rsW;
	int cpL = rsL>length?length:rsL;

	vBin.resize(width * length);

	
	cv::Mat resImg(nRows, nCols, CV_8UC1, &m_Buffer.m_vImg_Res[0]);
	cv::Mat rsImg(rsL, rsW, CV_8UC1);
	cv::resize(resImg, rsImg, cv::Size(rsW,rsL), 0.0, 0.0, cv::INTER_NEAREST);
	UCHAR * rsPtr = rsImg.ptr();
	int rsStep = rsImg.step1();
	for (int y=0; y<cpL; y++)
		memcpy(&vBin[y*width], &rsPtr[y*rsStep], cpW);

	m_procMil->SaveWorkImg(&vBin[0], width, length, _T("Grid_GetBlobImage.bmp"));
}

void CPInsp_Grid::GetMaskImage(std::vector<UCHAR> & vMask)
{
	int width = GetWidthImg(m_inspCoordinate);
	int length = GetHeightImg(m_inspCoordinate);

	int nCols = m_nColumn;
	int nRows = m_nRow;
	float fCols = m_nColumn;
	float fRows = m_nRow;
	int nWidCell = RounDF((float)(width) / fCols);
	int nLenCell = RounDF((float)(length) / fRows);
	if (nWidCell < 1) nWidCell = 1;
	if (nLenCell < 1) nLenCell = 1;

	int rsW = nCols*nWidCell;
	int rsL = nRows*nLenCell;

	int cpW = rsW>width?width:rsW;
	int cpL = rsL>length?length:rsL;

	vMask.resize(width * length);

	cv::Mat resImg(nRows, nCols, CV_8UC1, &m_Buffer.m_vMaskBuf[0]);
	cv::Mat rsImg(rsL, rsW, CV_8UC1);
	cv::resize(resImg, rsImg, cv::Size(rsW,rsL), 0.0, 0.0, cv::INTER_NEAREST);
	UCHAR * rsPtr = rsImg.ptr();
	int rsStep = rsImg.step1();
	for (int y=0; y<cpL; y++)
		memcpy(&vMask[y*width], &rsPtr[y*rsStep], cpW);

	m_procMil->SaveWorkImg(&vMask[0], width, length, _T("Grid_GetMaskImage_2.bmp"));
}

BOOL CPInsp_Grid::GetInspRst( RstAlgoGrid *inspAlgoResult )
{
	inspAlgoResult->dRstDetectArea = m_Buffer.dAreaPer;

	return m_Buffer.bResultOK;
}

void CPInsp_Grid::_CheckPartAngle()
{
	if(m_wndAlgoImg.dAngle == 270.0 || m_wndAlgoImg.dAngle==90.0)
	{
		m_nRow = m_pAlgoGrid->nColumn;
		m_nColumn = m_pAlgoGrid->nRow;
	}
	else
	{
		m_nColumn = m_pAlgoGrid->nColumn;
		m_nRow = m_pAlgoGrid->nRow;
	}
}

void CPInsp_Grid::CalcInspParam(double * stdev, double * stdevThreshold, bool bIsLoadFovRawDataImage)
{
	_CheckPartAngle();
	m_Buffer.Init(m_nColumn, m_nRow);

	CalcAllCell();
	IgnoreSelectedArea(bIsLoadFovRawDataImage);
	_CalcParam(stdev, stdevThreshold);
	BlobAnalysis();
}

void CPInsp_Grid::_CalcParam(double * stdev, double * threshold)
{
	UCHAR * pMask = &m_Buffer.m_vMaskBuf[0];
	UCHAR * pData = &m_Buffer.m_vImg_Res[0];
	int nCount=0;
	double dAvg=0.0, dStdev=0.0;

	int width = m_nColumn;
	int length = m_nRow;

	for (int y=0; y<length; y++)
	{
		for (int x=0; x<width; x++)
		{
			if(pMask[y*width+x] == 0)
			{
				dAvg += pData[y*width+x];
				nCount++;
			}
		}
	}
	if(nCount>0)
		dAvg /= (double)(nCount);
	else
		dAvg = 0.0;
	nCount=0;
	for (int y=0; y<length; y++)
	{
		for (int x=0; x<width; x++)
		{
			if(pMask[y*width+x] == 0)
			{
				dStdev += pow(pData[y*width+x] - dAvg, 2);
				nCount++;
			}
		}
	}

	if(nCount>2)
		dStdev = sqrt(dStdev/(double)(nCount-1));
	else
		dStdev = 0;

	if(stdev)
		*stdev = dAvg;
	if(threshold)
		*threshold = dAvg + dStdev*2.0;
}

