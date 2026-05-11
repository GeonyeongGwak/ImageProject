#pragma once
#include "StdAfx.h"
#include "MPTI.h"
#include "PInsp_Tab.h"
#include "MemoryManager.h"

//#define HISTOIMG_SIZEX   562
//#define HISTOIMG_SIZEY   532

#define HISTOIMG_SIZEX   320
#define HISTOIMG_SIZEY   290

CPInsp_Tab::CPInsp_Tab(void)
{
	m_className = _T("CPInsp_Tab");

	m_procMil = NULL;

	m_pAlgoTab = NULL;
	m_eAlgoType = eAlgoTab;

	InitResult(&m_resultTab);
	medBl = 1;
}

CPInsp_Tab::~CPInsp_Tab(void)
{
	CloseDevice();
}

int CPInsp_Tab::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
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

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength ,resolX, resolY, g_pMPTI->isUseImagePilLib());

	m_procMil->InitMilAlgoBlob();

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());

	if (m_pProcMilAlgo)
		m_pProcMilAlgo->InitMilAlgoBlob();

	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();

	return ePART_SUCCESS;
}

int CPInsp_Tab::CloseDevice()
{
	if(m_procMil != NULL)
	{
		m_procMil->FreeMil();
		
		//delete m_procMil;
		g_pMManager->pem_delete(m_procMil, false);
		m_procMil = NULL;
	}

	if (m_pProcMilAlgo != NULL)
		m_pProcMilAlgo->FreeMilAlgoBlob();

	CPInsp::CloseDevice();

	return ePART_SUCCESS;	
}

int CPInsp_Tab::SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, 
	const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, int nLeadTipPos, RECT* rcLeadInsp, int nInspCameraType,
	InspRoiImgBuf *sInspImageData, TotalInspExceptArea *stTieArea,int nTabIndex )
{
	if(sInspAlgo.m_eAlgoType != eAlgoTab)
		return ePART_FAIL;
	m_pInspAlgo = &sInspAlgo;
	m_pAlgoTab = (AlgoTab *)sInspAlgo.m_ptrInspAlgoParam;
	m_nTabIndex = nTabIndex;
	m_eAlgoType = sInspAlgo.m_eAlgoType;

	m_inspCoordinate.cx = RounD(coordinateAlgo.dROICenterX - coordinateAlgo.dROIWidth / 2.);
	m_inspCoordinate.cy = RounD(coordinateAlgo.dROICenterY - coordinateAlgo.dROILength / 2.);
	m_inspCoordinate.width = RounD(coordinateAlgo.dROIWidth);
	m_inspCoordinate.length = RounD(coordinateAlgo.dROILength);
	m_inspCoordinate.angle = coordinateAlgo.dROIAngle;

	m_wndAlgoImg = sWndAlgoImg;
	if (stTieArea)
		m_stTieArea.SetData(*stTieArea);

	m_sInspImgBuf = *sInspImageData;

	return ePART_SUCCESS;
}

// 현재 사용 중인 함수
int CPInsp_Tab::SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const WndInfo &wndInfoAnyAngle, TotalInspExceptArea stTieArea, int nLeadTipPos, bool bIsBigPart, bool bIsLoadFovRawDataImage, int projectionmode, int nInspCameraType, int nFovID)
{
	if(sInspAlgo.m_eAlgoType != eAlgoTab)
		return ePART_FAIL;
	m_pInspAlgo = &sInspAlgo;
	m_pAlgoTab = (AlgoTab *)sInspAlgo.m_ptrInspAlgoParam;
	m_bIsBigPart = bIsBigPart;

	m_eAlgoType = sInspAlgo.m_eAlgoType;

	m_wndAlgoImg = sWndAlgoImg;

	int window_left = RounD(coordinateAlgo.dROICenterX - coordinateAlgo.dROIWidth / 2.);	// Window 의 좌상단 좌표 (Pixel, Part의 Center 0,0 기준)
	int window_top = RounD(coordinateAlgo.dROICenterY + coordinateAlgo.dROILength / 2.);
	m_teachCoordinate.SetParamROI(coordinateAlgo);
	m_teachCoordinate.cx = window_left;
	m_teachCoordinate.cy = window_top;
	m_teachCoordinate.anyAngleCx = RounD(wndInfoAnyAngle.dCenterX / m_resolX);
	m_teachCoordinate.anyAngleCy = RounD(wndInfoAnyAngle.dCenterY / m_resolY);
	m_teachCoordinate.anyAngleWidth = RounD(wndInfoAnyAngle.dWidth / m_resolX);
	m_teachCoordinate.anyAngleLength = RounD(wndInfoAnyAngle.dLength / m_resolY);
	
	m_stTieArea.SetData(stTieArea);

	return ePART_SUCCESS;
}

int CPInsp_Tab::InspProc(UCHAR * ucArrDstImg, bool bTeach, bool bUseTabAreaOther)
{
	int nResult = e_NG;
	if (!m_pAlgoTab)
		return nResult;

	try
	{
		bool bInsp = true;
		int cx = m_inspCoordinate.cx;
		int cy = m_inspCoordinate.cy;
		int nImgWidth = GetWidthImg(m_inspCoordinate);
		int nImgHeight = GetHeightImg(m_inspCoordinate);
		if (nImgWidth < 1 || nImgHeight < 1)
			return nResult;
		RstInspTab resultTabTemp;
		UCHAR *ucWindTabImg = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR *ucWindInspImg = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);

		CalcHistoBW(*m_pInspAlgo, m_wndAlgoImg, &resultTabTemp, ucWindInspImg, ucWindTabImg, m_nTabIndex, bTeach, bUseTabAreaOther);

		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
		if (ucArrDstImg != NULL)
		{
			memcpy(ucArrDstImg, ucWindInspImg, nImgWidth * nImgHeight);
			cv::Mat mTabImg(nImgHeight, nImgWidth, CV_8UC1, ucWindTabImg);
			cv::Mat DstImg(nImgHeight, nImgWidth, CV_8UC1, ucArrDstImg);
			if (m_nTabIndex > 0)
			{
				cv::Mat DstTabImg = mTabImg.clone();
				cv::dilate(DstTabImg, DstTabImg, kernel);
				cv::erode(DstTabImg, DstTabImg, kernel);
				cv::erode(DstTabImg, DstTabImg, kernel);
				DstImg = DstTabImg & DstImg;
			}
			if (!bTeach) 
			{
				m_resultTab = resultTabTemp;
				m_resultTab.m_nCntRect = m_pAlgoTab->m_nCntRect;
				m_resultTab.m_nNGAreaRoiCnt = 0;
				g_pMManager->pem_delete(ucWindTabImg, true);
				g_pMManager->pem_delete(ucWindInspImg, true);
				return nResult;
			}
			else
			{
				for (int i = 0; i < m_pAlgoTab->m_nCntRect; ++i)
					m_pAlgoTab->m_bArrUseTab[i] = m_pAlgoTab->m_bArrUseWidth[i] = m_pAlgoTab->m_bArrUseLine[i] = m_pAlgoTab->m_bArrUseTail[i] = bTeach;
			}
		}
		m_procMil->SaveWorkImg(ucWindTabImg, nImgWidth, nImgHeight, _T("Tab_Area_wnd.bmp"));
		m_procMil->SaveWorkImg(ucWindInspImg, nImgWidth, nImgHeight, _T("Tab_Inso_wnd.bmp"));

		cv::Mat MatTabArea(nImgHeight, nImgWidth, CV_8UC1, ucWindTabImg);
		cv::Mat MatInsp(nImgHeight, nImgWidth, CV_8UC1, ucWindInspImg);

// 		cv::Mat MatTabAreaTemp;
// 		cv::dilate(MatTabArea, MatTabAreaTemp, kernel);
// 		cv::erode(MatTabAreaTemp, MatTabAreaTemp, kernel);
// 		cv::erode(MatTabAreaTemp, MatTabAreaTemp, kernel);
#if SAVE_TAB_IMG
		cv::Mat MatInspSrc = MatTabArea & MatInsp;
#endif
		POINT posCen;
		posCen.x = nImgWidth / 2;
		posCen.y = nImgHeight / 2;

		int nWidthSum = 0;
		int nHeightSum = 0;
		double dRateSum = 0;
		nResult = e_OK;

		float fWidth = 0;
		float fLength = 0;
		float fCenterX = 0;
		float fCenterY = 0;

		double resolX = 0;
		double resolY = 0;
		resolX = m_resolX;
		resolY = m_resolY;
		//MPTI_GetFovPixelResolution(&resolX, &resolY);
		double dFactor = resolX * resolY * 1000000;
		double dFactor2 = resolX * resolY;
		double dResolWidth = resolX;
		if (m_pAlgoTab->m_nLeadTipDirection == 2 || m_pAlgoTab->m_nLeadTipDirection == 3)
			dResolWidth = resolY;
		resultTabTemp.m_nNGAreaRoiCnt = 0;
		//////////////////////////////////////////////////////////////////////////
		for (int i = 0; i < m_pAlgoTab->m_nCntRect; ++i)
		{
			CRect rcTab(m_pAlgoTab->m_rcArrTabRect[i]);
			rcTab += posCen;
			if (rcTab.left < 0)
				rcTab.left = 0;
			if (rcTab.top < 0)
				rcTab.top = 0;
			if (rcTab.right < 0)
				rcTab.right = 0;
			if (rcTab.bottom < 0)
				rcTab.bottom = 0;
			if (rcTab.right > nImgWidth - 1)
				rcTab.right = nImgWidth - 1;
			if (rcTab.bottom > nImgHeight - 1)
				rcTab.bottom = nImgHeight - 1;
			if (rcTab.left > rcTab.right || rcTab.top > rcTab.bottom)
				continue;
			int nWidth = rcTab.Width();
			int nHeight = rcTab.Height();
			nWidthSum += nWidth;
			nHeightSum += nHeight;
			int nArea = nWidth * nHeight;

			//Load Images
			UCHAR *ucImgTabBin = g_pMManager->pem_new<UCHAR>(true, nArea, (PCHAR)__FUNCTION__, __LINE__);
			UCHAR *ucImgInspBin = g_pMManager->pem_new<UCHAR>(true, nArea, (PCHAR)__FUNCTION__, __LINE__);
			m_procMil->GetClipImage_LT(ucWindTabImg, nImgWidth, nImgHeight, ucImgTabBin, (int)rcTab.left, (int)rcTab.top, nWidth, nHeight);
			m_procMil->GetClipImage_LT(ucWindInspImg, nImgWidth, nImgHeight, ucImgInspBin, (int)rcTab.left, (int)rcTab.top, nWidth, nHeight);
			m_procMil->SaveWorkImg(ucImgTabBin, nWidth, nHeight, _T("Histo_Clip_TabArea.bmp"));
			m_procMil->SaveWorkImg(ucImgInspBin, nWidth, nHeight, _T("Histo_Clip_TabInsp.bmp"));

			double dPixelA1 = 0, dPixelA2 = 0, dIPixelA3 = 0, dPixel2_RecrArea = 0,dTailArea = 0;
			//int nSt, nEd;
			RECT rcTabRect;

			cv::Mat BinTabImg(nHeight, nWidth, CV_8UC1, ucImgTabBin);
			cv::Mat BinInspImg(nHeight, nWidth, CV_8UC1, ucImgInspBin);
			cv::Mat GrayImage(nImgHeight, nImgWidth, CV_8UC1, m_wndAlgoImg.m_ucArr2D_Mix[0]);
			cv::Mat GrayImageClip = GrayImage(cv::Rect((int)rcTab.left, (int)rcTab.top, nWidth, nHeight));
			cv::Mat TabImage,InspImg;
			//cv::Mat TabImageTemp = MatTabAreaTemp(cv::Rect(rcTab.left, rcTab.top, rcTab.Width(),rcTab.Height()));

			//m_nLeadTipDirection : Left, Right, Top, Bottom
			if (m_pAlgoTab->m_nLeadTipDirection == 1) { cv::rotate(BinTabImg, TabImage, cv::ROTATE_180); cv::rotate(BinInspImg, InspImg, cv::ROTATE_180); }
			else if (m_pAlgoTab->m_nLeadTipDirection == 2) { cv::rotate(BinTabImg, TabImage, cv::ROTATE_90_COUNTERCLOCKWISE); cv::rotate(BinInspImg, InspImg, cv::ROTATE_90_COUNTERCLOCKWISE); }
			else if (m_pAlgoTab->m_nLeadTipDirection == 3) { cv::rotate(BinTabImg, TabImage, cv::ROTATE_90_CLOCKWISE); cv::rotate(BinInspImg, InspImg, cv::ROTATE_90_CLOCKWISE); }
			else { TabImage = BinTabImg; InspImg = BinInspImg; }
			//int nIw = CalTabArea(TabImage, &dPixelA1, &dPixelA2, &dIPixelA3, &dPixel2_RecrArea, &nSt, &nEd);
			int nIw = CalTabArea2(TabImage, &dPixelA1, &dPixel2_RecrArea, &rcTabRect);
			//if (rcTabRect.right - rcTabRect.left < 1 || rcTabRect.bottom - rcTabRect.top < 1)
			//{
			//	g_pMManager->pem_delete(ucImgTabBin, true);
			//	g_pMManager->pem_delete(ucImgInspBin, true);
			//	resultTabTemp.m_bRstWidth[i] = !m_pAlgoTab->m_bArrUseWidth[i];
			//	resultTabTemp.m_bRstErrArea[i] = !m_pAlgoTab->m_bArrUseTab[i];
			//	resultTabTemp.m_bRstLine[i] = !m_pAlgoTab->m_bArrUseLine[i];
			//	resultTabTemp.m_bRstTail[i] = !m_pAlgoTab->m_bArrUseTail[i];
			//	int nTmpOK = ((resultTabTemp.m_bRstWidth[i]) &(resultTabTemp.m_bRstErrArea[i])& (resultTabTemp.m_bRstLine[i]) &(resultTabTemp.m_bRstTail[i]));
			//	if(nTmpOK == 0)
			//		nResult = e_NG;
			//	continue;
			//}
			cv::Mat TabImageTemp;
			cv::dilate(TabImage, TabImageTemp, kernel);
			cv::erode(TabImageTemp, TabImageTemp, kernel);
			cv::erode(TabImageTemp, TabImageTemp, kernel);
			cv::Mat MatInspSrctemp = TabImageTemp & InspImg;


			BOOL bOK = TRUE;
			BOOL bUseLineOK = m_pAlgoTab->m_bArrUseLine[i];
			BOOL bUseWidth = m_pAlgoTab->m_bArrUseWidth[i];
			BOOL bUseArea = m_pAlgoTab->m_bArrUseTab[i];
			BOOL bUseTail = m_pAlgoTab->m_bArrUseTail[i];
			std::vector<RECT> rcErrRect;
			float fMxErr = 0;
			float fDiff2 = 0.0f;
			RECT rcInspRect;
			BOOL bTailOK = FALSE;
			BOOL bLineOK = FALSE;
			BOOL bWidth = FALSE;
			BOOL bAreaOK = FALSE;
			if (bUseTail) bTailOK = InspTail(m_pAlgoTab->m_bUseTieBarROpt, TabImage, rcTabRect, m_pAlgoTab->m_dArrTailArea, &dTailArea, rcErrRect, dFactor2);

			if (bUseLineOK) bLineOK = InspLine(TabImage, m_pAlgoTab->m_dLine, rcTabRect, &fMxErr, rcErrRect, dFactor2, m_pAlgoTab->m_nMaxCippingLeng);
			if (bUseWidth) bWidth = nIw * dResolWidth >= m_pAlgoTab->m_dWidth[i];//bWidth = InspWidth(rcTabRect.left, rcTabRect.right, m_pAlgoTab->m_dWidth[i], dResolWidth);
			//BOOL bAreaOK = InspArea(TabImageTemp, InspImg, m_pAlgoTab->m_dArrMarginErrArea[i], &fDiff2, dFactor2);

			if (bUseWidth && bWidth == false)
			{
				RECT rcTab;
				rcTab.left = 3;
				rcTab.top = 3;
				rcTab.right = TabImage.cols - 3;
				rcTab.bottom = TabImage.rows - 3;
				rcErrRect.push_back(rcTab);
			}
			
			bAreaOK = InspArea(rcTabRect, MatInspSrctemp, m_pAlgoTab->m_dArrMarginErrArea, &fDiff2, rcErrRect, dFactor2, rcInspRect, bUseArea);
			//bAreaOK = InspArea(rcTabRect, MatInspSrctemp, GrayImageClip, m_pAlgoTab->m_dArrMarginErrArea[i], &fDiff2, dFactor2);
			//BOOL bAreaOK = InspArea_circle(rcTabRect, MatInspSrctemp, m_pAlgoTab->m_dArrMarginErrArea[i], &fDiff2, dFactor2);
			
			bOK = ( bLineOK | !bUseLineOK) &( bAreaOK| !bUseArea)& (bTailOK| !bUseTail) &( bWidth| !bUseWidth);
			double dDiffPixel2 = dPixel2_RecrArea - dPixelA2;
			bInsp &= (bUseLineOK | bUseArea | bUseTail | bUseWidth);
#if _DEBUG
			cv::Mat color;
			cv::cvtColor(TabImage, color, cv::COLOR_GRAY2BGR);
			cv::line(color, cv::Point(rcTabRect.left, 0), cv::Point(rcTabRect.left, color.rows), cv::Scalar(0, 0, 255));
			cv::line(color, cv::Point(rcTabRect.right, 0), cv::Point(rcTabRect.right, color.rows), cv::Scalar(0, 255, 0));
			CString sImgLog;
			sImgLog.Format(_T("D:\\testimage\\workimage\\Histo_Clip_%d.bmp"), i);
			cv::imwrite(std::string(CT2A(sImgLog)), color);
#endif
			if (bOK == false) 
			{
				nResult = e_NG;
#if SAVE_TAB_IMG
				CString sImgLogROI;
				sImgLogROI.Format(_T("D:\\testimage\\%d_4NGROI%d_ImgArea_Line%d_Area%d.bmp"), m_pInspAlgo->m_nAlgoId, i, bLineOK, bAreaOK);
				cv::imwrite(std::string(CT2A(sImgLogROI)), TabImage);
				cv::Mat AreaImg = InspImg & TabImage;// TabImageTemp;
				sImgLogROI.Format(_T("D:\\testimage\\%d_4NGROI%d_ImgInsp_Line%d_Area%d.bmp"), m_pInspAlgo->m_nAlgoId, i, bLineOK, bAreaOK);
				cv::imwrite(std::string(CT2A(sImgLogROI)), AreaImg);
				if (!bAreaOK)
				{
					cv::Mat GrayImage(nImgHeight, nImgWidth, CV_8UC1, m_wndAlgoImg.m_ucArr2D_Mix[0]);
					cv::Mat GrayImageClip = GrayImage(cv::Rect((int)rcTab.left, (int)rcTab.top, nWidth, nHeight));
					sImgLogROI.Format(_T("D:\\testimage\\%d_4NGROI%d_ImgGray_Line%d_Area%d.bmp"), m_pInspAlgo->m_nAlgoId, i, bLineOK, bAreaOK);
					cv::imwrite(std::string(CT2A(sImgLogROI)), GrayImageClip);
				}

#endif
			}

			g_pMManager->pem_delete(ucImgTabBin, true);
			g_pMManager->pem_delete(ucImgInspBin, true);

			RECT rcTabImg = m_pAlgoTab->m_rcArrTabRect[i];
			CPInsp::AngleRectChange(m_wndAlgoImg.dAngle, nImgWidth, nImgHeight, rcTabImg, &rcTabImg);

			resultTabTemp.m_bTabRst[i] = bOK;
			int ImgStX = rcTabImg.left, ImgStY = rcTabImg.top;
			if ((m_pAlgoTab->m_nLeadTipDirection == 0) || (m_pAlgoTab->m_nLeadTipDirection == 1))
			{
				fWidth = abs(rcTabImg.right - rcTabImg.left);
				fLength = abs(rcTabImg.bottom - rcTabImg.top);
				fCenterX = rcTabImg.left + (fWidth / 2.0f);
				fCenterY = rcTabImg.top + (fLength / 2.0f);
			}
			else
			{
				fWidth = abs(rcTabImg.bottom - rcTabImg.top);
				fLength = abs(rcTabImg.right - rcTabImg.left);
				fCenterX = rcTabImg.top + (fWidth / 2.0f);
				fCenterY = rcTabImg.left + (fLength / 2.0f);
				ImgStX = rcTabImg.top, ImgStY = rcTabImg.left;
			}

			if ((m_pAlgoTab->m_nLeadTipDirection == 0) || (m_pAlgoTab->m_nLeadTipDirection == 1))
			{
				resultTabTemp.m_pofNGRectCenter[i].x = fCenterX;
				resultTabTemp.m_pofNGRectCenter[i].y = fCenterY;
				resultTabTemp.m_pofNGRectSize[i].x = fWidth;
				resultTabTemp.m_pofNGRectSize[i].y = fLength;
			}
			else
			{
				resultTabTemp.m_pofNGRectCenter[i].x = fCenterY;
				resultTabTemp.m_pofNGRectCenter[i].y = fCenterX;
				resultTabTemp.m_pofNGRectSize[i].x = fLength;
				resultTabTemp.m_pofNGRectSize[i].y = fWidth;
			}
			if (ucArrDstImg)//teaching
			{
				resultTabTemp.m_bUseWidth[i] = TRUE;
				resultTabTemp.m_bUseErrArea[i] = TRUE;
				resultTabTemp.m_bUseLine[i] = TRUE;
				resultTabTemp.m_bUseTail[i] = dTailArea > 50* dFactor2;

				resultTabTemp.m_dArrTailArea[i] = 50 * dFactor2; // dTailArea * 0.65;
				resultTabTemp.m_dArrMarginErrArea[i] = 5 * dFactor2 > (double)fDiff2*1.2 ? 5 * dFactor2 : (double)fDiff2*1.2;
				resultTabTemp.m_dWidth[i] = nIw * dResolWidth*0.95;
				resultTabTemp.m_dLine[i] = fMxErr < 4 * dFactor2 ? 4 * dFactor2 : fMxErr;
			}
			else//Inspection
			{
				resultTabTemp.m_bUseWidth[i] = m_pAlgoTab->m_bArrUseWidth[i];
				resultTabTemp.m_bUseErrArea[i] = m_pAlgoTab->m_bArrUseTab[i];
				resultTabTemp.m_bUseLine[i] = m_pAlgoTab->m_bArrUseLine[i];
				resultTabTemp.m_bUseTail[i] = m_pAlgoTab->m_bArrUseTail[i];

				resultTabTemp.m_dArrTailArea[i] = dTailArea;
				resultTabTemp.m_dArrMarginErrArea[i] = (double)fDiff2;
				resultTabTemp.m_dWidth[i] = nIw * dResolWidth;
				resultTabTemp.m_dLine[i] = fMxErr;
			}
			resultTabTemp.m_rcArrNGArrRectCnt[i] = resultTabTemp.m_nNGAreaRoiCnt;
			if (resultTabTemp.m_nNGAreaRoiCnt < BRIEDGE_CNT)
			{
				int nAddCnt = 10 < BRIEDGE_CNT - resultTabTemp.m_nNGAreaRoiCnt ? 10 : BRIEDGE_CNT - resultTabTemp.m_nNGAreaRoiCnt;
				int nNGRcCnt = rcErrRect.size() < nAddCnt ? rcErrRect.size() : nAddCnt;
				int margin = 3;
				if (m_pAlgoTab->m_nLeadTipDirection == 1)
				{
					for (int nNgrc = 0; nNgrc < nNGRcCnt; nNgrc++)
					{
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].left = fWidth - rcErrRect[nNgrc].right + ImgStX - margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].right = fWidth - rcErrRect[nNgrc].left + ImgStX + margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].top = fLength - rcErrRect[nNgrc].bottom + ImgStY - margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].bottom = fLength - rcErrRect[nNgrc].top + ImgStY + margin;
						resultTabTemp.m_nNGAreaRoiCnt++;
						resultTabTemp.m_rcArrNGArrRectCnt[i] = resultTabTemp.m_nNGAreaRoiCnt;
					}
					resultTabTemp.m_rcArrCalcRect[i].left = fWidth - rcInspRect.right + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].right = fWidth - rcInspRect.left + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].top = fLength - rcInspRect.bottom + ImgStY;
					resultTabTemp.m_rcArrCalcRect[i].bottom = fLength - rcInspRect.top + ImgStY;
				}
				else if (m_pAlgoTab->m_nLeadTipDirection == 2)
				{
					for (int nNgrc = 0; nNgrc < nNGRcCnt; nNgrc++)
					{
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].top = rcErrRect[nNgrc].left + ImgStX - margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].bottom = rcErrRect[nNgrc].right + ImgStX + margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].left = ImgStY + fLength - rcErrRect[nNgrc].bottom - margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].right = ImgStY + fLength - rcErrRect[nNgrc].top + margin;
						resultTabTemp.m_nNGAreaRoiCnt++;
						resultTabTemp.m_rcArrNGArrRectCnt[i] = resultTabTemp.m_nNGAreaRoiCnt;
					}
					resultTabTemp.m_rcArrCalcRect[i].top = rcInspRect.left + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].bottom = rcInspRect.right + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].left = rcInspRect.top + ImgStY;
					resultTabTemp.m_rcArrCalcRect[i].right = rcInspRect.bottom + ImgStY;
				}
				else if (m_pAlgoTab->m_nLeadTipDirection == 3)
				{
					for (int nNgrc = 0; nNgrc < nNGRcCnt; nNgrc++)
					{
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].top = fWidth - rcErrRect[nNgrc].right + ImgStX- margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].bottom = fWidth - rcErrRect[nNgrc].left + ImgStX + margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].left = rcErrRect[nNgrc].top + ImgStY - margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].right = rcErrRect[nNgrc].bottom + ImgStY + margin;
						resultTabTemp.m_nNGAreaRoiCnt++;
						resultTabTemp.m_rcArrNGArrRectCnt[i] = resultTabTemp.m_nNGAreaRoiCnt;
					}
					resultTabTemp.m_rcArrCalcRect[i].top = fLength - rcInspRect.right + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].bottom = fLength - rcInspRect.left + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].left = fWidth - rcInspRect.bottom + ImgStY;
					resultTabTemp.m_rcArrCalcRect[i].right = fWidth - rcInspRect.top + ImgStY;
				}
				else//0
				{
					for (int nNgrc = 0; nNgrc < nNGRcCnt; nNgrc++)
					{
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].left = rcErrRect[nNgrc].left + ImgStX - margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].right = rcErrRect[nNgrc].right + ImgStX + margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].top = rcErrRect[nNgrc].top + ImgStY - margin;
						resultTabTemp.m_rcArrNGArrRect[resultTabTemp.m_nNGAreaRoiCnt].bottom = rcErrRect[nNgrc].bottom + ImgStY + margin;
						resultTabTemp.m_nNGAreaRoiCnt++;
						resultTabTemp.m_rcArrNGArrRectCnt[i] = resultTabTemp.m_nNGAreaRoiCnt;
					}
					resultTabTemp.m_rcArrCalcRect[i].left = rcInspRect.left + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].right = rcInspRect.right + ImgStX;
					resultTabTemp.m_rcArrCalcRect[i].top = rcInspRect.top + ImgStY;
					resultTabTemp.m_rcArrCalcRect[i].bottom = rcInspRect.bottom + ImgStY;
				}
			}

			resultTabTemp.m_bRstWidth[i] = bWidth | !bUseWidth;
			resultTabTemp.m_bRstErrArea[i] = bAreaOK | !bUseArea;
			resultTabTemp.m_bRstLine[i] = bLineOK | !bUseLineOK;
			resultTabTemp.m_bRstTail[i] = bTailOK | !bUseTail;
		}//End of for (int i = 0; i < m_pAlgoTab->nCntRect; ++i)
		//////////////////////////////////////////////////////////////////////////

		BOOL bInspDecision = (nResult == e_OK && bInsp);
		m_resultTab = resultTabTemp;
		//m_resultTab.m_ucResultImg = ucWindInspImg;
		m_resultTab.ok = bInspDecision;
		m_resultTab.m_nCntRect = m_pAlgoTab->m_nCntRect;
		m_resultTab.isInsp = TRUE;

#if SAVE_TAB_IMG
		CString imgPath, imgPath2;
		if (bInspDecision) {

			imgPath.Format(_T("D:\\testimage\\%d_2TabArea_OK.bmp"), m_pInspAlgo->m_nAlgoId);
			imgPath2.Format(_T("D:\\testimage\\%d_3Insp_OK.bmp"), m_pInspAlgo->m_nAlgoId);
		}
		else {

			imgPath.Format(_T("D:\\testimage\\%d_2TabArea_NG.bmp"), m_pInspAlgo->m_nAlgoId);
			imgPath2.Format(_T("D:\\testimage\\%d_3Insp_NG.bmp"), m_pInspAlgo->m_nAlgoId);
		}
		cv::imwrite(std::string(CT2A(imgPath)), MatTabArea);
		cv::imwrite(std::string(CT2A(imgPath2)), MatInspSrc);
#endif
		//////////////////////////////////////////////////////////////////////////
		g_pMManager->pem_delete(ucWindTabImg, true);
		g_pMManager->pem_delete(ucWindInspImg, true);
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
	return nResult;
}

void CPInsp_Tab::InitResult(RstInspTab* ret)
{
	memset(ret, 0, sizeof(RstInspTab));

	ret->ok = -1;
	ret->isInsp = FALSE;
}
int CPInsp_Tab::GetInspRst(tagRstAlgoTab * pRstAlgoTab)
{

// 	inspAlgoResult.m_bIsInsp = m_resultTab.isInsp;

	
	pRstAlgoTab->m_nCntRect = m_resultTab.m_nCntRect;

	for (int i = 0; i < m_resultTab.m_nCntRect; ++i)
	{
		pRstAlgoTab->m_bTabRst[i] = m_resultTab.m_bTabRst[i];
		pRstAlgoTab->m_pofNGRectCenter[i].x = m_resultTab.m_pofNGRectCenter[i].x;
		pRstAlgoTab->m_pofNGRectCenter[i].y = m_resultTab.m_pofNGRectCenter[i].y;
		pRstAlgoTab->m_pofNGRectSize[i].x = m_resultTab.m_pofNGRectSize[i].x;
		pRstAlgoTab->m_pofNGRectSize[i].y = m_resultTab.m_pofNGRectSize[i].y;
		pRstAlgoTab->m_dArrTailArea[i] = m_resultTab.m_dArrTailArea[i];
		pRstAlgoTab->m_dArrMarginErrArea[i] = m_resultTab.m_dArrMarginErrArea[i];
		pRstAlgoTab->m_dWidth[i] = m_resultTab.m_dWidth[i];
		pRstAlgoTab->m_dLine[i] = m_resultTab.m_dLine[i];
		pRstAlgoTab->m_rcArrGapRect[i] = m_pAlgoTab->m_rcArrTabRect[i];


		pRstAlgoTab->m_bUseWidth[i] = m_resultTab.m_bUseWidth[i];
		pRstAlgoTab->m_bUseErrArea[i] = m_resultTab.m_bUseErrArea[i];
		pRstAlgoTab->m_bUseLine[i] = m_resultTab.m_bUseLine[i];
		pRstAlgoTab->m_bUseTail[i] = m_resultTab.m_bUseTail[i];

		pRstAlgoTab->m_bRstWidth[i] = m_resultTab.m_bRstWidth[i];
		pRstAlgoTab->m_bRstErrArea[i] = m_resultTab.m_bRstErrArea[i];
		pRstAlgoTab->m_bRstLine[i] = m_resultTab.m_bRstLine[i];
		pRstAlgoTab->m_bRstTail[i] = m_resultTab.m_bRstTail[i];

		pRstAlgoTab->m_nArrMaxFreqValue1[i] = m_resultTab.m_nArrMaxFreqValue1[i];
		pRstAlgoTab->m_nArrMaxFreqValue2[i] = m_resultTab.m_nArrMaxFreqValue2[i];

		pRstAlgoTab->m_rcArrNGArrRectCnt[i] = m_resultTab.m_rcArrNGArrRectCnt[i];
		pRstAlgoTab->m_rcArrCalcRect[i] = m_resultTab.m_rcArrCalcRect[i];
	}
	pRstAlgoTab->m_nNGAreaRoiCnt = m_resultTab.m_nNGAreaRoiCnt;
	for (int i = 0; i < m_resultTab.m_nNGAreaRoiCnt; ++i)
	{
		pRstAlgoTab->m_rcArrNGArrRect[i] = m_resultTab.m_rcArrNGArrRect[i];
	}

	return m_resultTab.ok;
}
// void CPInsp_Tab::GetInspRst( InspAlgoResult &inspAlgoResult, RECT rcLeadInsp )
// {
//  	if(//inspAlgoResult.m_nAlgoType != eAlgoColor 
// // 		&& inspAlgoResult.m_nAlgoType != eAlgoLead_Color &&
//  		inspAlgoResult.m_nAlgoType != eAlgoTab)
// 		return;
// 
// 	BOOL bIsUseMultiArea = FALSE;
// 	if(m_pAlgoTab && m_pAlgoTab->IsUseMultiArea())
// 		bIsUseMultiArea = TRUE;
// 
// 	RstAlgoTab *pRstAlgoTab = (RstAlgoTab*)inspAlgoResult.m_vRstInspAlgo;
// 	if (!pRstAlgoTab)
// 		return;
// 
// 	inspAlgoResult.m_bIsInsp = m_resultTab.isInsp;
// 	inspAlgoResult.m_bOk = m_resultTab.ok;
// 
// 	pRstAlgoTab->m_nCntRect = m_resultTab.m_nCntRect;
// 	 
// 	for (int i = 0; i < m_resultTab.m_nCntRect; ++i)
// 	{
// 		pRstAlgoTab->m_bTabRst[i] = m_resultTab.m_bTabRst[i];
// 		pRstAlgoTab->m_pofNGRectCenter[i].x = m_resultTab.m_pofNGRectCenter[i].x;
// 		pRstAlgoTab->m_pofNGRectCenter[i].y = m_resultTab.m_pofNGRectCenter[i].y;
// 		pRstAlgoTab->m_pofNGRectSize[i].x = m_resultTab.m_pofNGRectSize[i].x;
// 		pRstAlgoTab->m_pofNGRectSize[i].y = m_resultTab.m_pofNGRectSize[i].y;
// 		pRstAlgoTab->m_dArrTailArea[i] = m_resultTab.m_dArrTailArea[i];
// 		pRstAlgoTab->m_dArrMarginErrArea[i] = m_resultTab.m_dArrMarginErrArea[i];
// 		pRstAlgoTab->m_dWidth[i] = m_resultTab.m_dWidth[i];
// 		pRstAlgoTab->m_rcArrGapRect[i] = m_pAlgoTab->m_rcArrTabRect[i];
// 
// 	}
// }

int CPInsp_Tab::CalTabArea(cv::Mat InputImg, double* A1, double* A2, double* A3, double* A2RecrArea, int* nSt, int* nEd, bool bIsTeach)
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
	std::vector<int> vColCount;
	vColCount.resize(bMaxboxTeach.height);
	bMaxboxTeach.height -= 2; bMaxboxTeach.y += 1;
	cv::Mat TabImg = InputImg(bMaxboxTeach);
	for (int c = 0; c < TabImg.cols; c++)
	{
		int RCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			if (TabImg.ptr(r)[c] > 0)
				RCnt++;
		}
		vColCount[RCnt]++;
	}
	int nMaxCol = 0, nMaxColIdx = 0;
	for (int i = 0; i < vColCount.size(); i++)
	{
		if (vColCount[i] > nMaxCol)
		{
			nMaxCol = vColCount[i];
			nMaxColIdx = i;
		}
	}
	//int nSt(0), nEd = TabImg.cols - 1;
	*nSt = 0; *nEd = TabImg.cols - 1;
	//for (int c = 0; c < TabImg.cols; c++)
	for (int c = TabImg.cols/2; c > 0; c--)
	{
		int nColCnt = 0;
		int nStThick = 0;
		int nEdThick = TabImg.rows - 1;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0) 
			{
				nStThick = r;
				break; 
			}
		}
		for (int r = TabImg.rows-1; r > 0 ; r--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEdThick = r;
				break;
			}
		}
		nColCnt = nEdThick - nStThick + 1;

		if ((double)nColCnt / nMaxColIdx < 0.90)
		{
			*nSt = c;
			break;
		}
	}
	for (int c = TabImg.cols / 2; c < TabImg.cols; c++)
	//for (int c = TabImg.cols - 1; c > 0; c--)
	{
		int nColCnt = 0;
		int nStThick = 0;
		int nEdThick = TabImg.rows - 1;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStThick = r;
				break;
			}
		}
		for (int r = TabImg.rows - 1; r > 0; r--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEdThick = r;
				break;
			}
		}
		nColCnt = nEdThick - nStThick + 1;

		if ((double)nColCnt / nMaxColIdx < 0.90)
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

	return A2Img.cols;
}

int CPInsp_Tab::CalTabArea(cv::Mat InputImg, double* A1, double* A2RecrArea, RECT* rcTabRect)
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
	std::vector<int> vVerCount,vHorCount, vHorPixelCnt;
	vVerCount.resize(bMaxboxTeach.height+1);
	vHorCount.resize(bMaxboxTeach.width+1);
	vHorPixelCnt.resize(bMaxboxTeach.height +1);
	cv::Mat TabImg = InputImg(bMaxboxTeach);
	for (int c = 0; c < TabImg.cols; c++)
	{
		int RCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			if (TabImg.ptr(r)[c] > 0)
			{
				RCnt++;
			}
		}
		vVerCount[RCnt]++;
	}
	int nMaxCol = 0, nMaxColIdx = 0;
	for (int i = 0; i < vVerCount.size(); i++)
	{
		if (vVerCount[i] > nMaxCol)
		{
			nMaxCol = vVerCount[i];
			nMaxColIdx = i;
		}
	}
	//int nSt(0), nEd = TabImg.cols - 1;
	rcTabRect->left = 0; rcTabRect->right = TabImg.cols - 1;
	rcTabRect->top = 0; rcTabRect->bottom = TabImg.rows - 1;
	//col
	//for (int c = 0; c < TabImg.cols; c++)
	bool curved = false;
	int curvedPoint = TabImg.cols / 2, curveEndPnt = TabImg.cols;
	for (int c = curvedPoint; c >= 0; c--)
	{
		int nColCnt = 0;
		int nStThick = 0;
		int nEdThick = TabImg.rows - 1;
		int nColWhiteCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStThick = r;
				break;
			}
		}
		for (int r = TabImg.rows - 1; r > 0; r--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEdThick = r;
				break;
			}
		}
		nColCnt = nEdThick - nStThick + 1;
		
		for (int r = 0; r <= nEdThick; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
				nColWhiteCnt++;
		}

		if ((double)nColCnt / nMaxColIdx < 0.50)
		{
			if (curved || curveEndPnt - c < 10)
				rcTabRect->left = curvedPoint;
			else
				rcTabRect->left = c;

			curved = false;
			break;
		}
		else if (((double)nColCnt / nMaxColIdx < 0.90 )|| ((double)nColWhiteCnt / nMaxColIdx < 0.80))
		{
			if (!curved)
			{
				curvedPoint = c;
				curved = true;
			}
		}
		else
		{
			if (curved)
			{
				curveEndPnt = c;
				curved = false;
				if (curveEndPnt < 10)
				{
					rcTabRect->left = curvedPoint;
					break;
				}
			}
		}
	}
	if (curved && curveEndPnt >= 10)
	{
		rcTabRect->left = curvedPoint;
		curved = false;
	}

	curvedPoint = TabImg.cols / 2;
	curveEndPnt = 0;
	for (int c = TabImg.cols / 2; c < TabImg.cols; c++)
		//for (int c = TabImg.cols - 1; c > 0; c--)
	{
		int nColCnt = 0;
		int nStThick = 0;
		int nEdThick = TabImg.rows - 1;
		int nColWhiteCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStThick = r;
				break;
			}
		}
		for (int r = TabImg.rows - 1; r > 0; r--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEdThick = r;
				break;
			}
		}
		nColCnt = nEdThick - nStThick + 1;

		for (int r = 0; r <= nEdThick; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
				nColWhiteCnt++;
		}
		if ((double)nColCnt / nMaxColIdx < 0.50)
		{
			if (curved || c - curveEndPnt < 10)
				rcTabRect->right = curvedPoint;
			else
				rcTabRect->right = c;

			curved = false;
			break;
		}
		else if (((double)nColCnt / nMaxColIdx < 0.90 )|| ((double)nColWhiteCnt / nMaxColIdx < 0.80))
		{
			if (!curved)
			{
				curvedPoint = c;
				curved = true;
			}
		}
		else
		{
			if (curved)
			{
				curveEndPnt = c;
				curved = false;
				if (TabImg.cols - curveEndPnt < 10)
				{
					rcTabRect->right = curvedPoint;
					break;
				}
			}
		}
	}
	if (curved && TabImg.cols - curveEndPnt >= 10)
	{
		rcTabRect->right = curvedPoint;
		curved = false;
	}

	double dArea1(0.0f);
	if (rcTabRect->left > 5)
	{
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			for (int c = 0; c < rcTabRect->left; c++)
			{
				dArea1 += (double)ptr[c];
			}
		}
		*A1 = dArea1 / 255;
	}

	//row

	for (int c = rcTabRect->left; c < rcTabRect->right; c++)
	{
		int RCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			if (TabImg.ptr(r)[c] > 0)
			{
				RCnt++;
				vHorPixelCnt[r]++;
			}
		}
	}
	for (int i = 0; i < vHorPixelCnt.size(); i++)
	{
		vHorCount[vHorPixelCnt[i]]++;
	}
	int nMaxRow = 0, nMaxRowIdx = 0;
	for (int i = vHorCount.size() / 2; i < vHorCount.size(); i++)
	{
		if (vHorCount[i] > nMaxRow)
		{
			nMaxRow = vHorCount[i];
			nMaxRowIdx = i;
		}
	}
	for (int r = 0; r < TabImg.rows; r++)
	{
		int nRowCnt = 0;
		int nStartX = 0;
		int nEndX = TabImg.cols - 1;
		for (int c = 0; c < TabImg.cols; c++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStartX = c; break;
			}
		}
		for (int c = TabImg.cols - 1; c > 0; c--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEndX = c; break;
			}
		}
		nRowCnt = nEndX - nStartX;

		if ((double)nRowCnt / nMaxRowIdx > 0.90)
		{
			rcTabRect->top = r;
			break;
		}
	}
	//for (int r = TabImg.rows / 2; r < TabImg.rows; r++)
	for (int r = TabImg.rows - 1; r > 0; r--)
	{
		int nRowCnt = 0;
		int nStartX = 0;
		int nEndX = TabImg.cols - 1;
		for (int c = 0; c < TabImg.cols; c++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0) 
			{
				nStartX = c; break; 
			}
		}
		for (int c = TabImg.cols - 1; c > 0; c--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0) 
			{
				nEndX = c; break;
			}
		}
		nRowCnt = nEndX - nStartX;

		if ((double)nRowCnt / nMaxRowIdx > 0.90)
		{
			rcTabRect->bottom = r+1;
			break;
		}
	}

	rcTabRect->left += bMaxboxTeach.x;
	rcTabRect->right += bMaxboxTeach.x;
	rcTabRect->top += bMaxboxTeach.y;
	rcTabRect->bottom += bMaxboxTeach.y;

	return rcTabRect->right - rcTabRect->left;
}
int CPInsp_Tab::CalTabArea2(cv::Mat InputImg, double* A1, double* A2RecrArea, RECT* rcTabRect)
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
	/*std::vector<int> vVerCount, vHorCount, vHorPixelCnt;
	vVerCount.resize(bMaxboxTeach.height + 1);
	vHorCount.resize(bMaxboxTeach.width + 1);
	vHorPixelCnt.resize(bMaxboxTeach.height + 1);*/
	cv::Mat MxRectTabImg = InputImg(bMaxboxTeach);


	int nBotCnt = 0;
	int nTopCnt = 0;
	int nLeft = 0;

	for (int r = 0; r < MxRectTabImg.rows; r++)
	{
		UCHAR* uPtr = MxRectTabImg.ptr(r);
		for (int c = 0; c < MxRectTabImg.cols; c++)
		{
			if (uPtr[c] > 0)
			{
				if (c > nLeft) // count the most side of letter
					nLeft = c;
				break;
			}
		}
	}

	for (int c = 0; c < nLeft; c++)
	{
		for (int r = 0; r < MxRectTabImg.rows / 2; r++)
		{
			UCHAR* uPtr = MxRectTabImg.ptr(r);
			if (uPtr[c] > 0)
			{
				nTopCnt++;
			}
		}
		for (int r = MxRectTabImg.rows / 2; r < MxRectTabImg.rows; r++)
		{
			UCHAR* uPtr = MxRectTabImg.ptr(r);
			if (uPtr[c] > 0)
			{
				nBotCnt++;
			}
		}
	}
	bool isBot = false;
	if (nBotCnt < nTopCnt) isBot = true;

	cv::Rect temp;
	if (isBot) // from half of box to bottom 
	{
		temp = bMaxboxTeach;
		temp.y += (temp.height / 2);
		temp.height /= 2;
	}
	else
	{
		temp = bMaxboxTeach;
		temp.height /= 2;
	}
	cv::Mat TabImg = InputImg(temp);

	//Area
	std::vector<int> vVerCount, vHorCount, vHorPixelCnt;
	vVerCount.resize(temp.height + 1);
	vHorCount.resize(temp.width + 1);
	vHorPixelCnt.resize(temp.height + 1);


	for (int c = 0; c < TabImg.cols; c++)
	{
		int RCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			if (TabImg.ptr(r)[c] > 0)
			{
				RCnt++;
			}
		}
		vVerCount[RCnt]++;
	}


	int nMaxCol = 0, nMaxColIdx = 0;
	for (int i = 0; i < vVerCount.size(); i++)
	{
		if (vVerCount[i] > nMaxCol)
		{
			nMaxCol = vVerCount[i];
			nMaxColIdx = i;
		}
	}

	//int nSt(0), nEd = TabImg.cols - 1;
	rcTabRect->left = 0; rcTabRect->right = TabImg.cols - 1;
	rcTabRect->top = 0; rcTabRect->bottom = TabImg.rows - 1;
	//col
	//for (int c = 0; c < TabImg.cols; c++)
	bool curved = false;
	int curvedPoint = TabImg.cols / 2, curveEndPnt = TabImg.cols;
	for (int c = curvedPoint; c >= 0; c--)
	{
		int nColCnt = 0;
		int nStThick = 0;
		int nEdThick = TabImg.rows - 1;
		int nColWhiteCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStThick = r;
				break;
			}
		}
		for (int r = TabImg.rows - 1; r > 0; r--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEdThick = r;
				break;
			}
		}
		nColCnt = nEdThick - nStThick + 1;

		for (int r = 0; r <= nEdThick; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
				nColWhiteCnt++;
		}

		if ((double)nColCnt / nMaxColIdx < 0.50)
		{
			if (curved || curveEndPnt - c < 10)
				rcTabRect->left = curvedPoint;
			else
				rcTabRect->left = c;

			curved = false;
			break;
		}
		else if (((double)nColCnt / nMaxColIdx < m_pAlgoTab->m_fstdWidth) || ((double)nColWhiteCnt / nMaxColIdx < m_pAlgoTab->m_fstdWidth))
		{
			if (!curved)
			{
				curvedPoint = c;
				curved = true;
			}
		}
		else
		{
			if (curved)
			{
				curveEndPnt = c;
				curved = false;
				if (curveEndPnt < 10)
				{
					rcTabRect->left = curvedPoint;
					break;
				}
			}
		}
	}
	if (curved && curveEndPnt >= 10)
	{
		rcTabRect->left = curvedPoint;
		curved = false;
	}

	curvedPoint = TabImg.cols / 2;
	curveEndPnt = 0;
	for (int c = TabImg.cols / 2; c < TabImg.cols; c++)
		//for (int c = TabImg.cols - 1; c > 0; c--)
	{
		int nColCnt = 0;
		int nStThick = 0;
		int nEdThick = TabImg.rows - 1;
		int nColWhiteCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStThick = r;
				break;
			}
		}
		for (int r = TabImg.rows - 1; r > 0; r--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEdThick = r;
				break;
			}
		}
		nColCnt = nEdThick - nStThick + 1;

		for (int r = 0; r <= nEdThick; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
				nColWhiteCnt++;
		}
		if ((double)nColCnt / nMaxColIdx < 0.50)
		{
			if (curved || c - curveEndPnt < 10)
				rcTabRect->right = curvedPoint;
			else
				rcTabRect->right = c;

			curved = false;
			break;
		}
		else if (((double)nColCnt / nMaxColIdx < m_pAlgoTab->m_fstdWidth) || ((double)nColWhiteCnt / nMaxColIdx < m_pAlgoTab->m_fstdWidth))
		{
			if (!curved)
			{
				curvedPoint = c;
				curved = true;
			}
		}
		else
		{
			if (curved)
			{
				curveEndPnt = c;
				curved = false;
				if (TabImg.cols - curveEndPnt < 10)
				{
					rcTabRect->right = curvedPoint;
					break;
				}
			}
		}
	}
	if (curved && TabImg.cols - curveEndPnt >= 10)
	{
		rcTabRect->right = curvedPoint;
		curved = false;
	}

	double dArea1(0.0f);
	if (rcTabRect->left > 5)
	{
		for (int r = 0; r < TabImg.rows; r++)
		{
			uchar* ptr = TabImg.ptr(r);
			for (int c = 0; c < rcTabRect->left; c++)
			{
				dArea1 += (double)ptr[c];
			}
		}
		*A1 = dArea1 / 255;
	}

	//row

	for (int c = rcTabRect->left; c < rcTabRect->right; c++)
	{
		int RCnt = 0;
		for (int r = 0; r < TabImg.rows; r++)
		{
			if (TabImg.ptr(r)[c] > 0)
			{
				RCnt++;
				vHorPixelCnt[r]++;
			}
		}
	}
	for (int i = 0; i < vHorPixelCnt.size(); i++)
	{
		vHorCount[vHorPixelCnt[i]]++;
	}
	int nMaxRow = 0, nMaxRowIdx = 0;
	for (int i = vHorCount.size() / 2; i < vHorCount.size(); i++)
	{
		if (vHorCount[i] > nMaxRow)
		{
			nMaxRow = vHorCount[i];
			nMaxRowIdx = i;
		}
	}
	for (int r = 0; r < TabImg.rows; r++)
	{
		int nRowCnt = 0;
		int nStartX = 0;
		int nEndX = TabImg.cols - 1;
		for (int c = 0; c < TabImg.cols; c++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStartX = c; break;
			}
		}
		for (int c = TabImg.cols - 1; c > 0; c--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEndX = c; break;
			}
		}
		nRowCnt = nEndX - nStartX;

		if ((double)nRowCnt / nMaxRowIdx > 0.90)
		{
			rcTabRect->top = r;
			break;
		}
	}
	//for (int r = TabImg.rows / 2; r < TabImg.rows; r++)
	for (int r = TabImg.rows - 1; r > 0; r--)
	{
		int nRowCnt = 0;
		int nStartX = 0;
		int nEndX = TabImg.cols - 1;
		for (int c = 0; c < TabImg.cols; c++)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nStartX = c; break;
			}
		}
		for (int c = TabImg.cols - 1; c > 0; c--)
		{
			uchar* ptr = TabImg.ptr(r);
			if (ptr[c] > 0)
			{
				nEndX = c; break;
			}
		}
		nRowCnt = nEndX - nStartX;

		if ((double)nRowCnt / nMaxRowIdx > 0.90)
		{
			rcTabRect->bottom = r + 1;
			break;
		}
	}

	rcTabRect->left += bMaxboxTeach.x;
	rcTabRect->right += bMaxboxTeach.x;
	rcTabRect->top += bMaxboxTeach.y;
	rcTabRect->bottom += bMaxboxTeach.y + temp.height;

	return rcTabRect->right - rcTabRect->left;
}

bool CPInsp_Tab::CalcHistoBW(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstInspTab* sRstAlgo, UCHAR* ucDstImg, UCHAR* ucDsAreatImg, int nTabIdx, bool bTeach, bool bUseTabAreaOther)
{
	if (sWndAlgoImg.m_ucArr2D == NULL || sWndAlgoImg.m_nWidth <= 0 || sWndAlgoImg.m_nHeight <= 0 
		|| sWndAlgoImg.m_ucArr2D_Mix[0] == NULL || ucDstImg == NULL)
		return false;

	//티칭 화면 초기화
	if (ucDstImg != NULL)
		memset(ucDstImg, 0, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

	AlgoTab* pAlgo = (AlgoTab*)sInspAlgo.m_ptrInspAlgoParam;
	int* nArrValue = g_pMManager->pem_new<int>(true, 255, (PCHAR)__FUNCTION__, __LINE__, true);

	UCHAR *pUcImgBinaryMask = g_pMManager->pem_new<UCHAR>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR *ucColorBin = g_pMManager->pem_new<UCHAR>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(ucColorBin, 255, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

	int nMixCnt = 1;
	bool bEnableNG2 = pAlgo->m_sBlobBase_NG2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eUse;
	bool bUse2D_NG2 = (pAlgo->m_sBlobBase_NG2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2D) == m_enBlobBase_Data::m_enBlobBase_Data_e2D ? true : false;
	bool bUse3D_NG2 = (pAlgo->m_sBlobBase_NG2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3D) == m_enBlobBase_Data::m_enBlobBase_Data_e3D ? true : false;
	bool bUseColor_NG2 = pAlgo->m_sBlobBase_NG2.m_sAlgoColorBase.m_bUseColor == 1 ? true : false;
	if (bEnableNG2 == true &&( bUse2D_NG2 == true || bUse3D_NG2 == true || bUseColor_NG2 == true))
		nMixCnt = 2;

	//Teaching UI 에서 4번 탭 선택했는데 해당탭의 이진화 티징데이터가 없을때 예외처리
	if (nTabIdx == 2 && nMixCnt == 1)
	{
		g_pMManager->pem_delete(nArrValue, true);
		g_pMManager->pem_delete(pUcImgBinaryMask, true);
		g_pMManager->pem_delete(ucColorBin, true);
		return false;
	}

	UCHAR* ucMix[2];
	for (int n = 0; n < nMixCnt; n++)
	{
		ucMix[n] = g_pMManager->pem_new<UCHAR>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
		memset(ucMix[n], 0, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	}

	POINT posCen;
	posCen.x = sWndAlgoImg.m_nWidth / 2;
	posCen.y = sWndAlgoImg.m_nHeight / 2;

	AlgoBlob algoBlob = CPInsp::SetAlgoBlob(sInspAlgo);
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;
	double dCX = 0;
	double dCY = 0;
	double dArea = 0;
	CRect rcBlob(0, 0, 0, 0);
#if 0
	//형상이미지 ColorBW

	//형상 이미지 BW
	if (pAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase.m_bUseColor)
	{
		pColorTeach->GetColorBaseBinTab(&pAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase, m_sInspImgBuf, ucColorBin);

		cv::Mat binDstTab(m_sInspImgBuf.nImageSizeY, m_sInspImgBuf.nImageSizeX, CV_8UC1, ucColorBin);
		m_pProcMilAlgo->SaveWorkImg(ucColorBin, m_sInspImgBuf.nImageSizeX, m_sInspImgBuf.nImageSizeY, _T("OrgColor_Clip.bmp"));
	}
	int nCntBlob = CPInsp::BlobImageStruct(algoBlob, sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_fArr3D, ucColorBin,
		sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, 100/*pAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_MinArea]*/,
		&dArea, &dCX, &dCY, &rcBlob, pUcImgBinaryMask, stTieAreaNULL, algoBlob.m_bFillHole);
#else

	//형상이미지 ColorBW
	//형상 이미지 BW
	if (pAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase.m_bUseColor)
	{
		pColorTeach->GetColorBaseBinTab(&pAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase, m_sInspImgBuf, ucColorBin, CRect(0, 0, 0, 0), m_nTabIndex > -1, pAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase.m_bUseAngleColor);

		cv::Mat binDstTab(m_sInspImgBuf.nImageSizeY, m_sInspImgBuf.nImageSizeX, CV_8UC1, ucColorBin);
		m_pProcMilAlgo->SaveWorkImg(ucColorBin, m_sInspImgBuf.nImageSizeX, m_sInspImgBuf.nImageSizeY, _T("OrgColor_Clip.bmp"));
	}
	cv::Mat TabArea(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1/*, pUcImgBinaryMask*/);
	
	if (bUseTabAreaOther && g_pInspMng->m_pucTabAreaTmp != nullptr)
	{ 
		TabArea.data = g_pInspMng->m_pucTabAreaTmp;
		memcpy(pUcImgBinaryMask, g_pInspMng->m_pucTabAreaTmp, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
	}
	else
	{
		TabArea.data = pUcImgBinaryMask;
		TabArea.setTo(0);
		for (int cnt = 0; cnt < pAlgo->m_nCntRect; cnt++)
		{

			CRect rt = pAlgo->m_rcArrTabRect[cnt];
			rt += posCen;

			if (rt.left < 0)rt.left = 0;
			if (rt.top < 0)rt.top = 0;
			if (rt.right > sWndAlgoImg.m_nWidth - 1)rt.right = sWndAlgoImg.m_nWidth - 1;
			if (rt.bottom > sWndAlgoImg.m_nHeight - 1)rt.bottom = sWndAlgoImg.m_nHeight - 1;

			int nWidth = rt.Width();
			int nHeight = rt.Height();
			if (nWidth < 1)
				continue;
			if (nHeight < 1)
				continue;
			UCHAR *pUcDst = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			UCHAR *pUcSrc = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_procMil->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, pUcSrc, rt.left, rt.top, nWidth, nHeight);
			FLOAT *pfSrc = NULL;
			if (algoBlob.m_bInsp3D)
			{
				pfSrc = g_pMManager->pem_new<float>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
				m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D, pfSrc, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, rt.left, rt.top, nWidth, nHeight);
			}
			UCHAR *pUcColorSrc = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_procMil->GetClipImage_LT(ucColorBin, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, pUcColorSrc, rt.left, rt.top, nWidth, nHeight);
			int nCntBlob = CPInsp::BlobImageStruct(algoBlob, pUcSrc, pfSrc, pUcColorSrc,
				nWidth, nHeight, 100/*pAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_MinArea]*/,
				&dArea, &dCX, &dCY, &rcBlob, pUcDst, stTieAreaNULL, algoBlob.m_bFillHole);

			cv::Mat Rst2D(nHeight, nWidth, CV_8UC1, pUcDst);
			Rst2D.copyTo(TabArea(cv::Rect(rt.left, rt.top, nWidth, nHeight)));

			g_pMManager->pem_delete(pUcDst, true);
			g_pMManager->pem_delete(pUcSrc, true);
			if (algoBlob.m_bInsp3D)
				g_pMManager->pem_delete(pfSrc, true);
			g_pMManager->pem_delete(pUcColorSrc, true);
		}
	}
#endif
#if SAVE_TAB_IMG
	CString imgPath;
	cv::Mat GrayImage(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[0]);
	imgPath.Format(_T("D:\\testimage\\%d_1GrayImg.bmp"), m_pInspAlgo->m_nAlgoId);
	cv::imwrite(std::string(CT2A(imgPath)), GrayImage);
	cv::Mat TabImage(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, pUcImgBinaryMask);
	imgPath.Format(_T("D:\\testimage\\%d_1TabImage.bmp"), m_pInspAlgo->m_nAlgoId);
	cv::imwrite(std::string(CT2A(imgPath)), TabImage);
	if (nMixCnt > 1)
	{
		cv::Mat GrayImage2(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[1]);
		imgPath.Format(_T("D:\\testimage\\%d_1GrayImg2.bmp"), m_pInspAlgo->m_nAlgoId);
		cv::imwrite(std::string(CT2A(imgPath)), GrayImage2);
	}
#endif
	
	for (int i = 0; i < nMixCnt; i++)
	{
		cv::Mat MixImg(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[i]);
		tagAlgoBlobBase stHistoBW;
		int nTeachHist = 0;
		if(sWndAlgoImg.m_ucArr2D_Mix[i] == NULL)
			continue;
		if (i == 0)
		{
			stHistoBW = pAlgo->m_sBlobBase_NG;
			nTeachHist = pAlgo->m_nHist1;
		}
		else
		{
			stHistoBW = pAlgo->m_sBlobBase_NG2;
			nTeachHist = pAlgo->m_nHist2;
		}
		UCHAR *ucColorImgDst = NULL;

		bool bUseColor = (pColorTeach != NULL && stHistoBW.m_sAlgoColorBase.m_bUseColor == TRUE) ? true : false;
		bool bUse2D = (stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2D) == m_enBlobBase_Data::m_enBlobBase_Data_e2D ? true : false;
		bool bUse3D = (stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3D) == m_enBlobBase_Data::m_enBlobBase_Data_e3D ? true : false;

		for (int cnt = 0; cnt < pAlgo->m_nCntRect; cnt++)
		{
			//버퍼 초기화
			CRect rt = pAlgo->m_rcArrTabRect[cnt];
			rt += posCen;

			if (rt.left < 0)rt.left = 0;
			if (rt.top < 0)rt.top = 0;
			if (rt.right > sWndAlgoImg.m_nWidth - 1)rt.right = sWndAlgoImg.m_nWidth - 1;
			if (rt.bottom > sWndAlgoImg.m_nHeight - 1)rt.bottom = sWndAlgoImg.m_nHeight - 1;

			int nWidth = rt.Width();
			int nHeight = rt.Height();

			if (nWidth < 1)
				continue;
			if (nHeight < 1)
				continue;

			UCHAR *pUcDst2D = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			UCHAR *pUcDst3D = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			
			if (bUse2D)
			{
				//histo
				int nHistoValue = Histogram(sWndAlgoImg.m_ucArr2D_Mix[i], pUcImgBinaryMask, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, rt);

				UCHAR *pUcSrc = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
				m_procMil->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[i], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, pUcSrc, rt.left, rt.top, nWidth, nHeight);

				int nMix2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D]- nTeachHist + nHistoValue > 255 ? 255 : stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D]- nTeachHist + nHistoValue;
				int nMax2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D]- nTeachHist + nHistoValue > 255 ? 255 : stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D]- nTeachHist + nHistoValue;
				int nRange = eTypeRangeOut;
				if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeIN) != 0)
					nRange = eTypeRangeIn;
				else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeUP) != 0)
					nRange = eTypeRangeUpper;
				else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeLo) != 0)
					nRange = eTypeRangeLower;
				if (nTeachHist == -1) 
				{
					nMix2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D];
					nMax2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D];
				}
				if (nMix2D < 0) nMix2D = 0;
				if (nMax2D < 0) nMax2D = 0;
				CPInsp::Binarize(pUcSrc, nWidth, nHeight, nRange, nMix2D, nMax2D, false, pUcDst2D);
				
				if (bUse3D == false)
				{
					cv::Mat Rst2D(nHeight, nWidth, CV_8UC1, pUcDst2D);
					Rst2D.copyTo(MixImg(cv::Rect(rt.left, rt.top, nWidth, nHeight)));
				}

				if (i == 0)
					sRstAlgo->m_nArrMaxFreqValue1[cnt] = nHistoValue;
				else
					sRstAlgo->m_nArrMaxFreqValue2[cnt] = nHistoValue;

				g_pMManager->pem_delete(pUcSrc, true);
			}
			if (bUse3D)
			{
				FLOAT *pfSrc = g_pMManager->pem_new<float>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
				m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D, pfSrc, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, rt.left, rt.top, nWidth, nHeight);
				
				float fMin3D = stHistoBW.m_fArrValue[m_efBlobBase::m_efBlobBase_Min3D];
				float nMax3D = stHistoBW.m_fArrValue[m_efBlobBase::m_efBlobBase_Max3D];
				int nRange = eTypeRangeOut;
				if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeIN) != 0)
					nRange = eTypeRangeIn;
				else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeUP) != 0)
					nRange = eTypeRangeUpper;
				else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeLo) != 0)
					nRange = eTypeRangeLower;

				CPInsp::Binarize(pfSrc, nWidth, nHeight, nRange, fMin3D, nMax3D, false, pUcDst3D);

				if (bUse2D == false)
				{
					cv::Mat Rst3D(nHeight, nWidth, CV_8UC1, pUcDst3D);
					Rst3D.copyTo(MixImg(cv::Rect(rt.left, rt.top, nWidth, nHeight)));
				}

				g_pMManager->pem_delete(pfSrc, true);
			}

			if (bUse2D == true && bUse3D == true)
			{
				cv::Mat Rst2D(nHeight, nWidth, CV_8UC1, pUcDst2D);
				cv::Mat Rst3D(nHeight, nWidth, CV_8UC1, pUcDst3D);

				cv::bitwise_and(Rst2D, Rst3D, Rst2D);
				Rst2D.copyTo(MixImg(cv::Rect(rt.left, rt.top, nWidth, nHeight)));
			}


			g_pMManager->pem_delete(pUcDst2D, true);
			g_pMManager->pem_delete(pUcDst3D, true);
		}
		memset(ucColorBin, 0, sWndAlgoImg.m_nWidth* sWndAlgoImg.m_nHeight);
		cv::Mat ColorImg(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucColorBin);

		if (bUseColor)
		{
			pColorTeach->GetColorBaseBinTab(&stHistoBW.m_sAlgoColorBase, m_sInspImgBuf, ucColorBin, CRect(0, 0, 0, 0), m_nTabIndex > -1, stHistoBW.m_sAlgoColorBase.m_bUseAngleColor);

			if (bUse2D == true || bUse3D == true)	//2D, 3D 하나라도 사용하면 위에 이진화 결과에 컬러를 합쳐야함
				cv::bitwise_and(MixImg, ColorImg, MixImg);
			else                                    //컬러 단독으로 이진화 하는경우
				ColorImg.copyTo(MixImg);
		}

		/*if (pColorTeach != NULL && stHistoBW.m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			cv::Mat ColorBin(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucColorImgDst);
			cv::Mat GrayBin(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[i]);
			if (bUse2D | bUse3D)
				cv::bitwise_and(GrayBin, ColorBin, GrayBin);
			else
				ColorBin.copyTo(GrayBin);
			Delete_1DArray(&ucColorImgDst);
		}*/
	}


	//1번 2번 mix 이미지 AND
	//TabIdx : 0 = 모두합친결과, 1 = Tab영역, 2 = 이물 조명1, 3 = 이물 조명2
	switch (nTabIdx)
	{
	case 3:
	case -1:
		if (nMixCnt > 1)
		{
			//1번 버퍼에 결과 몰아줌
			cv::Mat Rst1(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[0]);
			cv::Mat Rst2(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[1]);
			cv::bitwise_and(Rst1, Rst2, Rst1);
		}
		//최종 BW 영상 return 
		memcpy(ucDstImg, ucMix[0], sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	case 0:
		memcpy(ucDstImg, pUcImgBinaryMask, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	case 1:
		memcpy(ucDstImg, ucMix[0], sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	case 2:
		memcpy(ucDstImg, ucMix[1], sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	}

	//Area 영상 return 
	memcpy(ucDsAreatImg, pUcImgBinaryMask, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);

	if (g_pInspMng->m_nTabAlgoIdx == 0 && g_pInspMng->m_pucTabAreaTmp != nullptr)
		memcpy(g_pInspMng->m_pucTabAreaTmp, pUcImgBinaryMask, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);

	//해제
	g_pMManager->pem_delete(nArrValue, true);
	g_pMManager->pem_delete(pUcImgBinaryMask, true);
	g_pMManager->pem_delete(ucColorBin, true);
	for (int n = 0; n < nMixCnt; n++)
		g_pMManager->pem_delete(ucMix[n], true);

	return true;
}

int CPInsp_Tab::Histogram(UCHAR* ucSrc, UCHAR* ucMask, int nWidth, int nHeight, CRect rtRoi)
{
	//roi 안넣으면 전체이미지 히스토그램 계산
	//roi 넣으면 부분만 히스토그램 계산
	int nMaxLoc = 0;

	if (ucSrc == NULL || ucMask == NULL
		|| nWidth <= 0 || nHeight <= 0)
		return nMaxLoc;

	int nStX = 0;
	int nStY = 0;
	int nRoiWidth = nWidth;
	int nRoiHeight = nHeight;

	std::vector<int> vArrValue;
	vArrValue.resize(256, 0);

	if (rtRoi.Width() >= 0 || rtRoi.Height() >= 0)
	{
		nStX = rtRoi.left, nStY = rtRoi.top;
		nRoiWidth = rtRoi.Width(), nRoiHeight = rtRoi.Height();
	}
	int nWhitePixel = 0;
	int nSum = 0;
	for (int y = nStY; y < nStY + nRoiHeight; y++)
	{
		for (int x = nStX; x < nStX + nRoiWidth; x++)
		{
			if (ucMask[y * nWidth + x] == 255)
			{
				int nIdx = ucSrc[y * nWidth + x];
				vArrValue[nIdx] += 1;
				nWhitePixel++;
				nSum += nIdx;
			}
		}
	}
	float fDiv;
	float avg = nWhitePixel == 0 ? 0 : nSum / nWhitePixel;
	for (int nidx = 0; nidx < vArrValue.size(); nidx++)
		fDiv = std::abs(avg - vArrValue[nidx]);
	if (nWhitePixel != 0)fDiv /= nWhitePixel;
	if(fDiv>0.005)
		nMaxLoc = std::max_element(vArrValue.begin()+ (int)avg, vArrValue.end()) - vArrValue.begin();
	else
		nMaxLoc = std::max_element(vArrValue.begin(), vArrValue.end()) - vArrValue.begin();

	return nMaxLoc;
}

BOOL CPInsp_Tab::InspLine(cv::Mat mTabArea, float fErr, int nSt, int nEd, float* fMxErr, float fFactormm)
{
	float fErrPixel = (fErr / fFactormm + 0.5f);
	std::vector<cv::Point> vStPo, vEdPo;
	if (nEd - nSt > 20)
	{
		nEd -= 5;
		nSt += 5;
	}
	vStPo.resize(nEd - nSt);
	vEdPo.resize(nEd - nSt);
	int nStYCnt = 0,nEdYCnt = mTabArea.rows;
	std::vector<int> vCnt;
	vCnt.resize(mTabArea.rows);
	for (int c = nSt; c < nEd; c++)
	{
		bool bAllW = true;
		for (int r = mTabArea.rows/2; r > 0; r--)
		{
			if (mTabArea.ptr(r)[c] == 0)
			{
				vStPo[c - nSt].x = c;
				vStPo[c - nSt].y = r + 1;
				vCnt[r + 1]++;
				if (nStYCnt < r + 1)nStYCnt = r + 1;
				bAllW = false;
				break;
			}
		}
		if (bAllW)
		{
			vEdPo[c - nSt].x = c;
			vEdPo[c - nSt].y = 0;
			vCnt[r - 1]++;
			if (nEdYCnt > 0)nEdYCnt = 0;
		}
	}
	for (int c = nSt; c < nEd; c++)
	{
		bool bAllW = true;
		for (int r = mTabArea.rows / 2; r < mTabArea.rows; r++)
		{
			if (mTabArea.ptr(r)[c] == 0)
			{
				vEdPo[c - nSt].x = c;
				vEdPo[c - nSt].y = r - 1;
				vCnt[r - 1]++;
				if (nEdYCnt > r - 1)nEdYCnt = r - 1;
				bAllW = false;
				break;
			}
		}
		if (bAllW)
		{
			vEdPo[c - nSt].x = c;
			vEdPo[c - nSt].y = mTabArea.rows - 1;
			vCnt[mTabArea.rows - 1]++;
			if (nEdYCnt > mTabArea.rows - 1)nEdYCnt = mTabArea.rows - 1;
		}
	}
	int nStMax = 0, nStMaxCnt = 0;
	for (int i = nStYCnt; i >= 0; i--)
	{
		if (vCnt[i] > nStMaxCnt)
		{
			nStMaxCnt = vCnt[i];
			nStMax = i;
		}
	}
	int nEdMax = 0, nEdMaxCnt = 0;
	for (int i = nEdYCnt; i < vCnt.size(); i++)
	{
		if (vCnt[i] > nEdMaxCnt)
		{
			nEdMaxCnt = vCnt[i];
			nEdMax = i;
		}
	}
	int nStIdx = 0;
	while (vStPo.size() > 0)
	{
		if (abs(nStMax - vStPo[nStIdx].y) > 2)
		{
			vStPo.erase(vStPo.begin());
		}
		else 
			break;
	}
	while (vStPo.size() > 0)
	{
		nStIdx = vStPo.size()-1;
		if (abs(nStMax - vStPo[nStIdx].y) > 2)
		{
			//vStPo.erase(vStPo.end());
			vStPo.pop_back();
		}
		else
			break;
	}
	int nEdIdx = 0;
	while (vEdPo.size() > 0)
	{
		if (abs(nEdMax - vEdPo[nEdIdx].y) > 2)
		{
			vEdPo.erase(vEdPo.begin());
		}
		else
			break;
	}
	while (vEdPo.size() > 0)
	{
		nEdIdx = vEdPo.size() - 1;
		if (abs(nEdMax - vEdPo[nEdIdx].y) > 2)
		{
			//vEdPo.erase(vEdPo.end());
			vEdPo.pop_back();
		}
		else
			break;
	}
// 	float a1, a2, b1, b2;
// 	PCA_1D(vStPo, &a1, &b1);
// 	PCA_1D(vEdPo, &a2, &b2);

	for (int i = 0; i < vStPo.size(); i++)
	{
		float fErrValue = vStPo[i].y - nStMax;// std::abs(nStMax - vStPo[i].y);
		if (fErrValue > *fMxErr)
			*fMxErr = fErrValue;
		if (fErrValue > fErrPixel)
			return FALSE;
	}
	for (int i = 0; i < vEdPo.size(); i++)
	{
		float fErrValue = nEdMax - vEdPo[i].y;// std::abs(nEdMax - vEdPo[i].y);
		if (fErrValue > *fMxErr)
			*fMxErr = fErrValue;
		if (fErrValue > fErrPixel)
			return FALSE;
	}

	return TRUE;
}
BOOL CPInsp_Tab::InspLine(cv::Mat mTabArea, float fErr, RECT rcTabRect, float* fMxErr, std::vector<RECT>& rcErrRect, float fFactormm2, int nMaxLength)
{
	if (m_pAlgoTab->m_bUseChippingCriticalLine && m_pAlgoTab->m_nCriticalArea != 100)
	{
		float frate = (rcTabRect.bottom - rcTabRect.top) * ((float)m_pAlgoTab->m_nCriticalArea / 100);
		float fCriMargin = (rcTabRect.bottom - rcTabRect.top - frate) / 2;
		rcTabRect.left = rcTabRect.left + 4;
		rcTabRect.top = rcTabRect.top + fCriMargin;
		rcTabRect.right = rcTabRect.right - fCriMargin - 4;
		rcTabRect.bottom = rcTabRect.bottom - (fCriMargin);
	}
	int nEd = rcTabRect.right;
	int nSt = rcTabRect.left;
	std::vector<cv::Point> vStPo, vEdPo;
	if (nEd - nSt > 20)
	{
		nEd -= 5;
		nSt += 5;
	}
	if (nEd - nSt < 1)
	{
		RECT rcTab;
		rcTab.left = 3;
		rcTab.top = 3;
		rcTab.right = mTabArea.cols - 3;
		rcTab.bottom = mTabArea.rows - 3;
		rcErrRect.push_back(rcTab);
		return FALSE;
	}
	vStPo.resize(nEd - nSt);
	vEdPo.resize(nEd - nSt);

	cv::Mat InputImgCl = mTabArea.clone();
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(InputImgCl, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	double dMxArea = 0;
	int idx = 0;
	cv::Rect rMaxboxTeach(0, 0, 0, 0);
	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);
		double area = cv::contourArea(contours[idx]);

		if (area > dMxArea)
		{
			dMxArea = area;
			rMaxboxTeach = bbox;
		}
	}

	int nStYCnt = mTabArea.rows / 2, nEdYCnt = mTabArea.rows / 2;
	std::vector<int> vCnt;
	vCnt.resize(mTabArea.rows);
	for (int c = nSt; c < nEd; c++)
	{
		for (int r = rMaxboxTeach.y; r < mTabArea.rows; r++)
		{
			if (mTabArea.ptr(r)[c] > 0)
			{
				vStPo[c - nSt].x = c;
				vStPo[c - nSt].y = r;
				vCnt[r]++;
				if (nStYCnt > r)nStYCnt = r;
				break;
			}
		}
	}
	int stBot = (rMaxboxTeach.y + rMaxboxTeach.height) == mTabArea.rows ? mTabArea.rows - 1 : (rMaxboxTeach.y + rMaxboxTeach.height);
	for (int c = nSt; c < nEd; c++)
	{
		for (int r = stBot; r >0; r--)
		{
			if (mTabArea.ptr(r)[c] > 0)
			{
				vEdPo[c - nSt].x = c;
				vEdPo[c - nSt].y = r;
				vCnt[r]++;
				if (nEdYCnt < r)nEdYCnt = r;
				break;
			}
		}
	}
	int nStMax = 0, nStMaxCnt = 0;
	for (int i = nStYCnt; i < vCnt.size() / 2; i++)
	{
		if (vCnt[i] > nStMaxCnt)
		{
			nStMaxCnt = vCnt[i];
			nStMax = i;
		}
	}
	int nEdMax = 0, nEdMaxCnt = 0;
	for (int i = vCnt.size()/2; i <= nEdYCnt; i++)
	{
		if (vCnt[i] > nEdMaxCnt)
		{
			nEdMaxCnt = vCnt[i];
			nEdMax = i;
		}
	}
	int nStIdx = 0, nStErased = 0;
	while (vStPo.size() > 0)
	{
		if (abs(nStMax - vStPo[nStIdx].y) > 2)
		{
			vStPo.erase(vStPo.begin());
			nStErased++;
		}
		else
			break;
	}
	if (vStPo.size() > 0)
	{
		int PopStd = vStPo[vStPo.size() - 1].y;
		while (vStPo.size() > 0)
		{
			nStIdx = vStPo.size() - 1;
			//if (abs(PopStd - vStPo[nStIdx].y) > 2)
			if (abs(PopStd - vStPo[nStIdx].y) < 2)
			{
				//vStPo.erase(vStPo.end());
				vStPo.pop_back();
			}
			else
				break;
		}
	}
	int nEdIdx = 0, nEdErased = 0;
	while (vEdPo.size() > 0)
	{
		if (abs(nEdMax - vEdPo[nEdIdx].y) > 2)
		{
			vEdPo.erase(vEdPo.begin());
			nEdErased++;
		}
		else
			break;
	}
	if (vEdPo.size() > 0)
	{
		int PopStd = vEdPo[vEdPo.size() - 1].y;
		while (vEdPo.size() > 0)
		{
			nEdIdx = vEdPo.size() - 1;
			//if (abs(nEdMax - vEdPo[nEdIdx].y) > 2)
			if (abs(PopStd - vEdPo[nEdIdx].y) < 2)
			{
				//vEdPo.erase(vEdPo.end());
				vEdPo.pop_back();
			}
			else
				break;
		}
	}
	// 	float a1, a2, b1, b2;
	// 	PCA_1D(vStPo, &a1, &b1);
	// 	PCA_1D(vEdPo, &a2, &b2);
	int nErrPixel = 0, nErrPixeltemp = 0;
	std::vector<int> vErrArea;
	vErrArea.resize(vStPo.size());
	for (int i = 0; i < vStPo.size(); i++)
	{
		float fErrValue = std::abs(vStPo[i].y - nStMax);// std::abs(nStMax - vStPo[i].y);
		if (fErrValue > 1) 
		{
			vErrArea[i] += (fErrValue - 1);
		}
	}
	bool bDeep = false;
	std::vector<int> vErrPixels;
	std::vector<RECT> vRcRect;
	RECT rcRect = {0, 0, 0, 0};
	int nErrArea = 0;
	for (int i = 0; i < vErrArea.size(); i++)
	{
		if (vErrArea[i] > 0)
		{
			if (nErrArea==0)
			{
				rcRect.left = nSt + i + nStErased;
				rcRect.top = nStMax;
			}
			if (vErrArea[i] > 1)//3pixel이상 들어가야 NG처리 / 위에서 넣은때 -1을하고 넣어서 1이상이면 3픽셀이 들어감
				bDeep = true;
			nErrArea += vErrArea[i];
			if (nErrArea > fErr && nErrPixeltemp < nErrArea)
			{
				nErrPixeltemp = nErrArea;
			}
			if (bDeep)
			{
				nErrPixel = nErrPixeltemp;
				rcRect.right = nSt + i + nStErased + 1;

				int tmpBottom = nStMax + vErrArea[i] + 1;
				if(tmpBottom > rcRect.bottom)
					rcRect.bottom = tmpBottom;
			}
		}
		else 
		{
			if (bDeep)
			{
				vErrPixels.push_back(nErrPixel);
				vRcRect.push_back(rcRect);
			}
			bDeep = false;
			nErrArea = 0;
		}
	}
	if (bDeep)
	{
		if (vRcRect.size() > 0 && (rcRect.left - vRcRect[vRcRect.size() - 1].right) < 5)
		{
			vErrPixels[vErrPixels.size() - 1] += nErrPixel;
			vRcRect[vRcRect.size() - 1].right = rcRect.right;
			vRcRect[vRcRect.size() - 1].top = vRcRect[vRcRect.size() - 1].top < rcRect.top? vRcRect[vRcRect.size() - 1].top : rcRect.top;
			vRcRect[vRcRect.size() - 1].bottom = vRcRect[vRcRect.size() - 1].bottom < rcRect.bottom? vRcRect[vRcRect.size() - 1].bottom : rcRect.bottom;
		}
		else
		{
			vErrPixels.push_back(nErrPixel);
			vRcRect.push_back(rcRect);
		}
	}
	bDeep = false;

	BOOL bReturn = TRUE;
	for (int i = 0; i < vErrPixels.size(); i++)
	{
		float fErrTmp = vErrPixels[i] * fFactormm2;
		float fPer = (float)(vRcRect[i].right - vRcRect[i].left) / (rcTabRect.right - rcTabRect.left) * 100;
		if (fErrTmp > fErr && (float)nMaxLength > fPer)
		//if (fErrTmp > fErr)
		{
			if (*fMxErr < fErrTmp)
				*fMxErr = fErrTmp;

			rcErrRect.push_back(vRcRect[i]);
			bReturn = FALSE;
		}
	}

	nErrPixel = 0;
	nErrPixeltemp = 0;
	vErrArea.clear();
	vErrArea.resize(vEdPo.size());
	vErrPixels.clear();
	vRcRect.clear();
	rcRect = { 0, nEdMax, 0, 0 };
	for (int i = 0; i < vEdPo.size(); i++)
	{
		float fErrValue = std::abs(nEdMax - vEdPo[i].y);// std::abs(nEdMax - vEdPo[i].y);
		if (fErrValue > 1)
		{
			vErrArea[i] += (fErrValue - 1);
		}
	}
	nErrArea = 0;
	for (int i = 0; i < vErrArea.size(); i++)
	{
		if (vErrArea[i] > 0)
		{
			if (nErrArea == 0)
			{
				rcRect.left = nSt + i + nEdErased;
				rcRect.bottom = nEdMax;
			}
			if (vErrArea[i] > 1)//3pixel이상 들어가야 NG처리 / 위에서 넣은때 -1을하고 넣어서 1이상이면 3픽셀이 들어감
				bDeep = true;
			nErrArea += vErrArea[i];
			if (nErrArea > fErr && nErrPixeltemp < nErrArea)
			{
				nErrPixeltemp = nErrArea;
			}
			if (bDeep)
			{
				nErrPixel = nErrPixeltemp;
				rcRect.right = nSt + i + nEdErased + 1;

				int tmpTop = nEdMax - (vErrArea[i] + 1);
				if (tmpTop < rcRect.top)
					rcRect.top = tmpTop;
			}
		}
		else
		{
			if (bDeep)
			{
				vErrPixels.push_back(nErrPixel);
				vRcRect.push_back(rcRect);
			}
			bDeep = false;
			nErrArea = 0;
		}
	}
	if (bDeep)
	{
		if (vRcRect.size() > 0 && (rcRect.left - vRcRect[vRcRect.size() - 1].right) < 5)
		{
			vErrPixels[vErrPixels.size() - 1] += nErrPixel;
			vRcRect[vRcRect.size() - 1].right = rcRect.right;
			vRcRect[vRcRect.size() - 1].top = vRcRect[vRcRect.size() - 1].top < rcRect.top ? vRcRect[vRcRect.size() - 1].top : rcRect.top;
			vRcRect[vRcRect.size() - 1].bottom = vRcRect[vRcRect.size() - 1].bottom < rcRect.bottom ? vRcRect[vRcRect.size() - 1].bottom : rcRect.bottom;
		}
		else
		{
			vErrPixels.push_back(nErrPixel);
			vRcRect.push_back(rcRect);
		}
	}

	for (int i = 0; i < vErrPixels.size(); i++)
	{
		float fErrTmp = vErrPixels[i] * fFactormm2;
		float fPer = (float)(vRcRect[i].right - vRcRect[i].left) / (rcTabRect.right - rcTabRect.left) * 100;
		if (fErrTmp > fErr && (float)nMaxLength > fPer)
		//if (fErrTmp > fErr)
		{
			if (*fMxErr < fErrTmp)
				*fMxErr = fErrTmp;

			rcErrRect.push_back(vRcRect[i]);
			bReturn = FALSE;
		}
	}

	return bReturn;
}
BOOL CPInsp_Tab::InspLine_center(cv::Mat mTabArea, float fErr, RECT rcTabRect, float* fMxErr, std::vector<RECT>& rcErrRect, float fFactormm2)
{
	int nEd = rcTabRect.right;
	int nSt = rcTabRect.left;
	std::vector<cv::Point> vStPo, vEdPo;
	if (nEd - nSt > 20)
	{
		nEd -= 5;
		nSt += 5;
	}
	vStPo.resize(nEd - nSt);
	vEdPo.resize(nEd - nSt);
	int nStYCnt = mTabArea.rows / 2, nEdYCnt = mTabArea.rows / 2;
	std::vector<int> vCnt;
	vCnt.resize(mTabArea.rows);
	for (int c = nSt; c < nEd; c++)
	{
		bool bAllW = true;
		for (int r = mTabArea.rows / 2; r >= 0; r--)
		{
			if (mTabArea.ptr(r)[c] == 0)
			{
				vStPo[c - nSt].x = c;
				vStPo[c - nSt].y = r + 1;
				vCnt[r + 1]++;
				if (nStYCnt > r + 1)nStYCnt = r + 1;
				bAllW = false;
				break;
			}
		}
		if (bAllW)
		{
			vEdPo[c - nSt].x = c;
			vEdPo[c - nSt].y = 0;
			vCnt[0]++;
			if (nStYCnt > 0)nStYCnt = 0;
		}
	}
	for (int c = nSt; c < nEd; c++)
	{
		bool bAllW = true;
		for (int r = mTabArea.rows / 2; r < mTabArea.rows; r++)
		{
			if (mTabArea.ptr(r)[c] == 0)
			{
				vEdPo[c - nSt].x = c;
				vEdPo[c - nSt].y = r - 1;
				vCnt[r - 1]++;
				if (nEdYCnt < r - 1)nEdYCnt = r - 1;
				bAllW = false;
				break;
			}
		}
		if (bAllW)
		{
			vEdPo[c - nSt].x = c;
			vEdPo[c - nSt].y = mTabArea.rows - 1;
			vCnt[mTabArea.rows - 1]++;
			if (nEdYCnt < mTabArea.rows - 1)nEdYCnt = mTabArea.rows - 1;
		}
	}
	int nStMax = 0, nStMaxCnt = 0;
	for (int i = nStYCnt; i < vCnt.size() / 2; i++)
	{
		if (vCnt[i] > nStMaxCnt)
		{
			nStMaxCnt = vCnt[i];
			nStMax = i;
		}
	}
	int nEdMax = 0, nEdMaxCnt = 0;
	for (int i = vCnt.size() / 2; i <= nEdYCnt; i++)
	{
		if (vCnt[i] > nEdMaxCnt)
		{
			nEdMaxCnt = vCnt[i];
			nEdMax = i;
		}
	}
	int nStIdx = 0, nStErased = 0;
	while (vStPo.size() > 0)
	{
		if (abs(nStMax - vStPo[nStIdx].y) > 2)
		{
			vStPo.erase(vStPo.begin());
			nStErased++;
		}
		else
			break;
	}
	int PopStd = vStPo[vStPo.size() - 1].y;
	while (vStPo.size() > 0)
	{
		nStIdx = vStPo.size() - 1;
		//if (abs(PopStd - vStPo[nStIdx].y) > 2)
		if (abs(PopStd - vStPo[nStIdx].y) < 2)
		{
			//vStPo.erase(vStPo.end());
			vStPo.pop_back();
		}
		else
			break;
	}
	int nEdIdx = 0, nEdErased = 0;
	while (vEdPo.size() > 0)
	{
		if (abs(nEdMax - vEdPo[nEdIdx].y) > 2)
		{
			vEdPo.erase(vEdPo.begin());
			nEdErased++;
		}
		else
			break;
	}
	PopStd = vEdPo[vEdPo.size() - 1].y;
	while (vEdPo.size() > 0)
	{
		nEdIdx = vEdPo.size() - 1;
		//if (abs(nEdMax - vEdPo[nEdIdx].y) > 2)
		if (abs(PopStd - vEdPo[nEdIdx].y) < 2)
		{
			//vEdPo.erase(vEdPo.end());
			vEdPo.pop_back();
		}
		else
			break;
	}
	// 	float a1, a2, b1, b2;
	// 	PCA_1D(vStPo, &a1, &b1);
	// 	PCA_1D(vEdPo, &a2, &b2);
	int nErrPixel = 0, nErrPixeltemp = 0;
	std::vector<int> vErrArea;
	vErrArea.resize(vStPo.size());
	for (int i = 0; i < vStPo.size(); i++)
	{
		float fErrValue = std::abs(vStPo[i].y - nStMax);// std::abs(nStMax - vStPo[i].y);
		if (fErrValue > 1)
		{
			vErrArea[i] += (fErrValue - 1);
		}
	}
	bool bDeep = false;
	std::vector<int> vErrPixels;
	std::vector<RECT> vRcRect;
	RECT rcRect = { 0, 0, 0, 0 };
	int nErrArea = 0;
	for (int i = 0; i < vErrArea.size(); i++)
	{
		if (vErrArea[i] > 0)
		{
			if (nErrArea == 0)
			{
				rcRect.left = nSt + i + nStErased;
				rcRect.top = nStMax;
			}
			if (vErrArea[i] > 1)//3pixel이상 들어가야 NG처리 / 위에서 넣은때 -1을하고 넣어서 1이상이면 3픽셀이 들어감
				bDeep = true;
			nErrArea += vErrArea[i];
			if (nErrArea > fErr && nErrPixeltemp < nErrArea)
			{
				nErrPixeltemp = nErrArea;
			}
			if (bDeep)
			{
				nErrPixel = nErrPixeltemp;
				rcRect.right = nSt + i + nStErased + 1;

				int tmpBottom = nStMax + vErrArea[i] + 1;
				if (tmpBottom > rcRect.bottom)
					rcRect.bottom = tmpBottom;
			}
		}
		else
		{
			if (bDeep)
			{
				vErrPixels.push_back(nErrPixel);
				vRcRect.push_back(rcRect);
			}
			bDeep = false;
			nErrArea = 0;
		}
	}
	if (bDeep)
	{
		vErrPixels.push_back(nErrPixel);
		vRcRect.push_back(rcRect);
	}
	bDeep = false;

	BOOL bReturn = TRUE;
	for (int i = 0; i < vErrPixels.size(); i++)
	{
		float fErrTmp = vErrPixels[i] * fFactormm2;
		if (fErrTmp > fErr)
		{
			if (*fMxErr < fErrTmp)
				*fMxErr = fErrTmp;

			rcErrRect.push_back(vRcRect[i]);
			bReturn = FALSE;
		}
	}

	nErrPixel = 0;
	nErrPixeltemp = 0;
	vErrArea.clear();
	vErrArea.resize(vEdPo.size());
	vErrPixels.clear();
	vRcRect.clear();
	rcRect = { 0, nEdMax, 0, 0 };
	for (int i = 0; i < vEdPo.size(); i++)
	{
		float fErrValue = std::abs(nEdMax - vEdPo[i].y);// std::abs(nEdMax - vEdPo[i].y);
		if (fErrValue > 1)
		{
			vErrArea[i] += (fErrValue - 1);
		}
	}
	nErrArea = 0;
	for (int i = 0; i < vErrArea.size(); i++)
	{
		if (vErrArea[i] > 0)
		{
			if (nErrArea == 0)
			{
				rcRect.left = nSt + i + nEdErased;
				rcRect.bottom = nEdMax;
			}
			if (vErrArea[i] > 1)//3pixel이상 들어가야 NG처리 / 위에서 넣은때 -1을하고 넣어서 1이상이면 3픽셀이 들어감
				bDeep = true;
			nErrArea += vErrArea[i];
			if (nErrArea > fErr && nErrPixeltemp < nErrArea)
			{
				nErrPixeltemp = nErrArea;
			}
			if (bDeep)
			{
				nErrPixel = nErrPixeltemp;
				rcRect.right = nSt + i + nEdErased + 1;

				int tmpTop = nEdMax - (vErrArea[i] + 1);
				if (tmpTop < rcRect.top)
					rcRect.top = tmpTop;
			}
		}
		else
		{
			if (bDeep)
			{
				vErrPixels.push_back(nErrPixel);
				vRcRect.push_back(rcRect);
			}
			bDeep = false;
			nErrArea = 0;
		}
	}
	if (bDeep)
	{
		vErrPixels.push_back(nErrPixel);
		vRcRect.push_back(rcRect);
	}

	for (int i = 0; i < vErrPixels.size(); i++)
	{
		float fErrTmp = vErrPixels[i] * fFactormm2;
		if (fErrTmp > fErr)
		{
			if (*fMxErr < fErrTmp)
				*fMxErr = fErrTmp;

			rcErrRect.push_back(vRcRect[i]);
			bReturn = FALSE;
		}
	}

	return bReturn;
}
BOOL CPInsp_Tab::InspWidth(int nSt, int nEd, float fStdWidth, float fFactormm)
{
	float fWidth = (nEd - nSt + 1) * fFactormm;
	if (fWidth < fStdWidth)
		return FALSE;

	return TRUE;
}
BOOL CPInsp_Tab::InspArea(cv::Mat TabImage, cv::Mat InspImg, float fErr,float* dDiff, float fFactormm2)
{
	cv::Mat ForeignImg = TabImage & InspImg;
	float fMinPixel = 4;// fErr / fFactormm2;
	cv::Mat DstImg(ForeignImg.rows, ForeignImg.cols,CV_8UC1);
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ForeignImg.ptr(), DstImg.ptr(), ForeignImg.cols, ForeignImg.rows, fMinPixel, false, false, 0, eSelectBigger);
	if (nCntBlob == 0)
		return TRUE;
	BOOL bIsOK = TRUE;
	RECT *rcRect = g_pMManager->pem_new<RECT>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__, true);
	std::vector<double> vdArea;
	m_pProcMilAlgo->GetBlobResult_Rects(rcRect, nCntBlob);
	m_pProcMilAlgo->GetBlobResult_Area(vdArea);
	for (int i = 0; i < nCntBlob; i++)
	{
		int nW = (rcRect->right - rcRect->left);
		int nH = (rcRect->bottom - rcRect->top);
		float nRate = nW > nH ? (float)nH / nW : (float)nW / nH;
		float nAreaRate = nW > nH ? vdArea[i] / (float)nW : vdArea[i] / (float)nH;
		if (nRate > 0.5 && nAreaRate >= 2)
			bIsOK = FALSE;
	}
	g_pMManager->pem_delete(rcRect, true);

	return bIsOK;
}
BOOL CPInsp_Tab::InspArea(RECT rcTabRect, cv::Mat InspImg, float fErr, float* dDiff, std::vector<RECT>& rcErrRect, float fFactormm2, RECT& rcInspRect, BOOL bUseArea)
{
	if (rcTabRect.right - rcTabRect.left < 1 || rcTabRect.bottom - rcTabRect.top < 1)
	{
		RECT rcTab;
		rcTab.left = 3;
		rcTab.top = 3;
		rcTab.right = InspImg.cols - 3;
		rcTab.bottom = InspImg.rows - 3;
		rcErrRect.push_back(rcTab);
		return FALSE;
	}
	cv::Rect InspRect;

	// Erase Scar Area 옵션에 적용될 scar 영역 설정
	cv::Rect eraseRect;
	std::vector<RECT> rcErrRectTemp;
	if (m_pAlgoTab->m_bUseDisableTabArea && m_pAlgoTab->m_nCriticalArea == 100)
	{
		InspRect.x = 0;
		InspRect.y = 0;
		InspRect.width = rcTabRect.right;
		InspRect.height = InspImg.rows;

		if (m_pAlgoTab->m_nLeadTipDirection == 1)	// left
			eraseRect = cv::Rect(rcTabRect.left, rcTabRect.top + (rcTabRect.bottom - rcTabRect.top) * 0.1, (rcTabRect.right - rcTabRect.left) * 0.6, (rcTabRect.bottom - rcTabRect.top) * 0.6);
		else
			eraseRect = cv::Rect(rcTabRect.left, rcTabRect.top + (rcTabRect.bottom - rcTabRect.top) * 0.3, (rcTabRect.right - rcTabRect.left) * 0.6, (rcTabRect.bottom - rcTabRect.top) * 0.6);

	}
	else if(m_pAlgoTab->m_nCriticalArea == 100)
	{
		InspRect.x = rcTabRect.left + 4;
		InspRect.y = rcTabRect.top + 2;
		InspRect.width = rcTabRect.right - rcTabRect.left - 8;
		InspRect.height = rcTabRect.bottom - rcTabRect.top - 4;

		if (m_pAlgoTab->m_nLeadTipDirection == 1)	// left
			eraseRect = cv::Rect(0, InspRect.height * 0.1, InspRect.width * 0.6, InspRect.height * 0.6);
		else
			eraseRect = cv::Rect(0, InspRect.height * 0.3, InspRect.width * 0.6, InspRect.height * 0.6);

	}
	else
	{
		//shkim 치명영역 추가 -> 현재 tab폭의 N%만 검사
		float frate = (rcTabRect.bottom - rcTabRect.top) * ((float)m_pAlgoTab->m_nCriticalArea / 100);
		float fCriMargin = (rcTabRect.bottom - rcTabRect.top - frate)/2;
		InspRect.x = rcTabRect.left + 4;
		InspRect.y = rcTabRect.top + fCriMargin;
		InspRect.width = rcTabRect.right - rcTabRect.left - fCriMargin - 4;
		InspRect.height = rcTabRect.bottom - rcTabRect.top - (fCriMargin*2);

		if (m_pAlgoTab->m_nLeadTipDirection == 1)	// left
			eraseRect = cv::Rect(0, InspRect.height * 0.1, InspRect.width * 0.6, InspRect.height * 0.6);
		else
			eraseRect = cv::Rect(0, InspRect.height * 0.3, InspRect.width * 0.6, InspRect.height * 0.6);

	}
	rcInspRect.left = InspRect.x;
	rcInspRect.top = InspRect.y;
	rcInspRect.right = InspRect.x + InspRect.width;
	rcInspRect.bottom = InspRect.y + InspRect.height;
	if (bUseArea == FALSE)
	{
		RECT rcTab;
		rcTab.left = 3;
		rcTab.top = 3;
		rcTab.right = InspImg.cols - 3;
		rcTab.bottom = InspImg.rows - 3;
		rcErrRect.push_back(rcTab);
		return FALSE;
	}
	if (InspRect.width < 1 || InspRect.height < 1)
	{
		RECT rcTab;
		rcTab.left = 3;
		rcTab.top = 3;
		rcTab.right = InspImg.cols - 3;
		rcTab.bottom = InspImg.rows - 3;
		rcErrRect.push_back(rcTab);
		return FALSE;
	}

	cv::Mat ForeignImg = InspImg(InspRect).clone();

	// 모폴로지 열림 연산 우선 진행하는 옵션 (Constraction)
	if(m_pAlgoTab->m_bContraction)
	{
		cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
		cv::erode(ForeignImg, ForeignImg, kernel2);
		cv::dilate(ForeignImg, ForeignImg, kernel2);
	}
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	cv::dilate(ForeignImg, ForeignImg, kernel);
	cv::erode(ForeignImg, ForeignImg, kernel);
	float fMinPixel = 0;//fErr / fFactormm2;
	cv::Mat DstImg(ForeignImg.rows, ForeignImg.cols, CV_8UC1);
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ForeignImg.ptr(), DstImg.ptr(), ForeignImg.cols, ForeignImg.rows, fMinPixel, false, false, 0, eSelectMix);
	if (nCntBlob == 0)
		return TRUE;

	BOOL bIsOK = TRUE;
	ForeignImg = InspImg(InspRect);
	RECT *rcRect = g_pMManager->pem_new<RECT>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__, true);
	std::vector<double> vdArea;
	double *dCenterX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__, true);
	double *dCenterY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pProcMilAlgo->GetBlobResult_Rects(rcRect, nCntBlob);
	m_pProcMilAlgo->GetBlobResult_Center(dCenterX, dCenterY);
	m_pProcMilAlgo->GetBlobResult_Area(vdArea);

	////// 수직, 수평 방향 직선 성분으로 묶을 때, Blob의 라벨링을 위한 변수들
	
	// 다음에 세팅할 라벨 번호
	int curLabel = 0;

	// 수직, 수평 방향 라벨 저장하는 벡터. Index는 Blob의 Index. 값은 라벨이 없는 Blob은 -1, 있으면 라벨 번호.
	// Vert와 Horz에 중복되는 라벨 번호는 없음. (curLabel 변수 하나로 라벨 번호를 매기기 때문)
	std::vector<int> vLabelVert(nCntBlob);
	std::vector<int> vLabelHorz(nCntBlob);

	// 각 라벨 번호에 할당된 Blob의 개수. Index는 라벨 번호 (vLabelVert 또는 vLabelHorz)
	// 라벨 카운트가 3개 이상이어야 직선으로 판단한다.
	std::vector<int> vLabelVertCnt(nCntBlob);
	std::vector<int> vLabelHorzCnt(nCntBlob);

	// 각 라벨 번호에 할당된 Blob 전체의 Rect 범위. Index는 라벨 번호 (vLabelVert 또는 vLabelHorz)
	std::vector<cv::Rect> vLabelRect(nCntBlob);

	// 각 라벨 번호에 할당된 Blob 전체의 면적. Index는 라벨 번호 (vLabelVert 또는 vLabelHorz)
	std::vector<double> vLabelArea(nCntBlob);

	////// 수직, 수평 방향 직선 성분으로 묶을 때, Blob의 라벨링을 위한 변수들 --- 끝


	// CrossLine Detect 옵션 관련 변수.
	// Blob이 수직, 수평 방향이 교차되는 지점의 모양일 때 ('+', 'L', 'T', 'ㄱ' 등등),
	// 수직방향 성분과 수평방향 성분으로 분리하여 각각의 중심좌표를 따로 저장하는 벡터
	std::vector<std::vector<double>> crossedBlobCenter(nCntBlob);

	std::vector<double> vCircleErr(nCntBlob);

	int thickness = m_pAlgoTab->m_bUseMinScarThickness ? m_pAlgoTab->m_dMinScarThickness / m_resolX : InspRect.height;
	double dRatio = m_pAlgoTab->m_bUseScarAspectRatio ? m_pAlgoTab->m_dScarAspectRatio : 0.3;

	for (int i = 0; i < nCntBlob; i++)
	{
		vLabelVert[i] = vLabelHorz[i] = -1;

		// 수직, 수평방향으로 교차되는 Blob인지 파악하고 중심을 분리하는 과정
		// Blob Rect의 면적과 실제 Blob의 면적의 비율이 1/2 보다 작으면 + 모양일 가능성이 있다고 본다.
		if (m_pAlgoTab->m_bUseCrossLineDetect && vdArea[i] / double((rcRect[i].right - rcRect[i].left)* (rcRect[i].bottom - rcRect[i].top)) < 1. / 2.)
		{
			RECT cRect;

			// 수직방향으로 뻗은 성분 제거 (수평방향으로 긴 마스크로 모폴로지 열림 연산)
			cv::Mat ROIDstImg = DstImg(cv::Rect(rcRect[i].left, rcRect[i].top, (rcRect[i].right - rcRect[i].left), (rcRect[i].bottom - rcRect[i].top))).clone();
			cv::Mat kernel3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size((rcRect[i].right - rcRect[i].left) / 2, 1));
			cv::erode(ROIDstImg, ROIDstImg, kernel3);
			cv::dilate(ROIDstImg, ROIDstImg, kernel3);

			int nCntBlob2 = m_pProcMilAlgo->CalcBlob_Select(ROIDstImg.ptr(), ROIDstImg.ptr(), ROIDstImg.cols, ROIDstImg.rows, 1, false, false, 0, eSelectMix);
			if(nCntBlob2 != 1)	// 정상적으로 + 모양 또는 T 등의 모양이라면 수직방향으로 뻗은 걸 제거하면 수평방향 blob 한 개만 남아야 한다고 판단
				continue;

			m_pProcMilAlgo->GetBlobResult_Rects(&cRect, 1);
			int tW = cRect.right - cRect.left;
			int tH = cRect.bottom - cRect.top;
			double tRatio = tW < tH ? (double)tW / tH : (double)tH / tW;
			if(tRatio >= m_pAlgoTab->m_dScarAspectRatio)	// 수평방향의 마스크로 모폴로지 실시해서 수평 성분만 남으면 길고 얇게 남아야 한다고 판단
				continue;

			double t1CenterX, t1CenterY;
			m_pProcMilAlgo->GetBlobResult_Center(&t1CenterX, &t1CenterY);	// 중심좌표 임시 저장. 수직 성분에 대해서도 똑같이 OK 인 경우에만 중심좌표 저장

			// 수평방향으로 뻗은 성분 제거 (수직방향으로 긴 마스크로 모폴로지 열림 연산)
			cv::Mat ROIDstImg2 = DstImg(cv::Rect(rcRect[i].left, rcRect[i].top, (rcRect[i].right - rcRect[i].left), (rcRect[i].bottom - rcRect[i].top))).clone();
			cv::Mat kernel4 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, (rcRect[i].bottom - rcRect[i].top) / 2));
			cv::erode(ROIDstImg2, ROIDstImg2, kernel4);
			cv::dilate(ROIDstImg2, ROIDstImg2, kernel4);

			nCntBlob2 = m_pProcMilAlgo->CalcBlob_Select(ROIDstImg2.ptr(), ROIDstImg2.ptr(), ROIDstImg2.cols, ROIDstImg2.rows, 1, false, false, 0, eSelectMix);
			if (nCntBlob2 != 1)	// 수평방향으로 뻗은 걸 제거하면 수직방향 blob 한 개만 남아야 한다고 판단
				continue;

			m_pProcMilAlgo->GetBlobResult_Rects(&cRect, 1);
			tW = cRect.right - cRect.left;
			tH = cRect.bottom - cRect.top;
			tRatio = tW < tH ? (double)tW / tH : (double)tH / tW;
			if (tRatio >= m_pAlgoTab->m_dScarAspectRatio)	// 수직방향의 마스크로 모폴로지 실시해서 수직 성분만 남으면 길고 얇게 남아야 한다고 판단
				continue;

			double t2CenterX, t2CenterY;
			m_pProcMilAlgo->GetBlobResult_Center(&t2CenterX, &t2CenterY);	// 중심좌표 임시 저장

			dCenterX[i] = dCenterY[i] = -1;
			crossedBlobCenter[i].resize(4);
			crossedBlobCenter[i][0] = t1CenterX;
			crossedBlobCenter[i][1] = t1CenterY;
			crossedBlobCenter[i][2] = t2CenterX;
			crossedBlobCenter[i][3] = t2CenterY;
		}

		// Erase Scar Area 옵션에서 해당 영역에 들어오는 Blob이 충분히 크고 원형을 이루면 이물로 검출하고 아니면 scar인지 아닌지 판별하기 위한 과정
		if (m_pAlgoTab->m_bUseEarseScarArea && rcRect[i].bottom - rcRect[i].top > 7 && rcRect[i].right - rcRect[i].left > 7)
		{
			cv::Rect rt(rcRect[i].left, rcRect[i].top, rcRect[i].right - rcRect[i].left, rcRect[i].bottom - rcRect[i].top);
			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::findContours(DstImg(rt), contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

			std::vector<cv::Point> targetContour;
			for (int a = 0; a < contours.size(); a++)
			{
				cv::Rect contourRect = cv::boundingRect(contours[a]);
				if (contourRect.width == rt.width && contourRect.height == rt.height)	// Blob Rect와 동일한 크기인 Contour 선택
				{
					targetContour = contours[a];
					break;
				}
			}

			double err = 0.0, radius = 0;
			if (targetContour.size() > 0)
				CircleFitting(targetContour, NULL, NULL, &radius, &err);	// 기존에 있던 CircleFitting을 조금 수정해서 따로 함수 만들었음.

			vCircleErr[i] = 1.0 - err; // 원형율 score 저장. score가 높으면 이물일 가능성이 높으니까 다른 Blob들과 직선성분으로 묶지 않음.
		}
	}

	// 수직, 수평 방향으로 묶는 과정
	if (m_pAlgoTab->m_bUseCrossLineDetect)
	{
		for (int i = 0; i < nCntBlob - 1; i++)
		{
			if (m_pAlgoTab->m_bUseEarseScarArea && vCircleErr[i] > 0.7)	// Erase Scar Area 옵션 ON일 때, 원형율 체크
				continue;

			double cX_I = dCenterX[i], cY_I = dCenterY[i];
			bool isCross_I = false;
			if (cX_I < 0 && cY_I < 0)	// + 모양이라 중심 검출 다시 했는지 여부 파악
				isCross_I = true;

			for (int j = i + 1; j < nCntBlob; j++)
			{
				if (m_pAlgoTab->m_bUseEarseScarArea && vCircleErr[j] > 0.7)	// Erase Scar Area 옵션 ON일 때, 원형율 체크
					continue;

				double cX_J = dCenterX[j], cY_J = dCenterY[j];
				bool isCross_J = false;
				if (cX_J < 0 && cY_J < 0)	// + 모양이라 중심 검출 다시 했는지 여부 파악
					isCross_J = true;

				if (isCross_I)
				{
					cX_I = crossedBlobCenter[i][2];
					cY_I = crossedBlobCenter[i][3];
				}
				if (isCross_J)
				{
					cX_J = crossedBlobCenter[j][2];
					cY_J = crossedBlobCenter[j][3];
				}

				// Blob Rect의 수직방향 거리
				double dist = cY_I > cY_J ? (rcRect[i].top - rcRect[j].bottom) : (rcRect[j].top - rcRect[i].bottom);

				// 중심의 X 좌표가 거의 동일하고 Rect 간의 거리가 가까울 때, 또는 Blob Rect의 left 나 right가 거의 동일하고 거리가 매우 가까울 때 수직 성분으로 묶는다
				if ((abs(cX_I - cX_J) <= 1.5 && dist < InspRect.height * 0.2) || ((abs(rcRect[i].left - rcRect[j].left) <= 1 || abs(rcRect[i].right - rcRect[j].right) <= 1) && dist < 7))
				{
					float width_I = rcRect[i].right - rcRect[i].left;
					float width_J = rcRect[j].right - rcRect[j].left;
					float widthRatio = width_I > width_J ? width_J / width_I : width_I / width_J;
					
					// 두께 차이가 너무 많이 나면 묶지 않는다.
					// + 모양은 두께를 측정하기 어려워서 그냥 묶고, 거리가 매우 가까우면 두께 차이가 조금 심해도 묶는다.
					if (!isCross_I && !isCross_J && (dist >= 5 && widthRatio <= 0.4 || widthRatio <= 0.15))
						continue;

					// 수직방향 라벨링 작업

					if (vLabelVert[i] < 0)
					{
						vLabelVert[i] = curLabel++;
						vLabelVertCnt[curLabel - 1]++;

						// Blob이 충분히 길면 2개로 카운트한다.
						if (double(rcRect[i].right - rcRect[i].left) / double(rcRect[i].bottom - rcRect[i].top) < dRatio)
							vLabelVertCnt[curLabel - 1]++;
					}

					int label_I = vLabelVert[i];
					int label_J = vLabelVert[j];

					if (label_I == label_J)
						continue;

					if (label_J >= 0)
					{
						vLabelVertCnt[label_I] += vLabelVertCnt[label_J];
						vLabelVertCnt[label_J] = 0;

						for (int a = 0; a < nCntBlob; a++)
						{
							if (vLabelVert[a] == label_J)
								vLabelVert[a] = label_I;
						}
					}
					else
					{
						vLabelVert[j] = label_I;
						vLabelVertCnt[label_I]++;

						if (double(rcRect[j].right - rcRect[j].left) / double(rcRect[j].bottom - rcRect[j].top) < dRatio)
							vLabelVertCnt[label_I]++;
					}

					// 수직방향 라벨링 끝

					continue;
				}

				if (isCross_I)
				{
					cX_I = crossedBlobCenter[i][0];
					cY_I = crossedBlobCenter[i][1];
				}
				if (isCross_J)
				{
					cX_J = crossedBlobCenter[j][0];
					cY_J = crossedBlobCenter[j][1];
				}

				// Erase Scar Area 옵션 ON 일때, 해당 영역에 들어가지 않으면 수평방향 직선 성분으로 묶지 않는다.
				bool isInScarArea_I = eraseRect.contains(cv::Point(int(cX_I + 0.5), int(cY_I + 0.5)));
				bool isInScarArea_J = eraseRect.contains(cv::Point(int(cX_J + 0.5), int(cY_J + 0.5)));

				if (m_pAlgoTab->m_bUseEarseScarArea && !isInScarArea_I && !isInScarArea_J)
					continue;

				// 여기부터 수평방향 직선 성분 묶는 시퀀스. 수직방향과 동일함

				dist = cX_I < cX_J ? (rcRect[j].left - rcRect[i].right) : (rcRect[i].left - rcRect[j].right);

				if ((abs(cY_I - cY_J) <= 1.5 && dist < InspRect.width * 0.15) || ((abs(rcRect[i].bottom - rcRect[j].bottom) <= 1 || abs(rcRect[i].top - rcRect[j].top) <= 1) && dist < 10))
				{
					float height_I = rcRect[i].bottom - rcRect[i].top;
					float height_J = rcRect[j].bottom - rcRect[j].top;
					float heightRatio = height_I > height_J ? height_J / height_I : height_I / height_J;

					if (!isCross_I && !isCross_J && (dist >= 7 && heightRatio <= 0.4 || heightRatio <= 0.15))
						continue;

					if (vLabelHorz[i] < 0)
					{
						vLabelHorz[i] = curLabel++;
						vLabelHorzCnt[curLabel - 1]++;

						// Blob이 충분히 길면 2개로 카운트한다.
						if (double(rcRect[i].bottom - rcRect[i].top) / double(rcRect[i].right - rcRect[i].left) < dRatio)
							vLabelHorzCnt[curLabel - 1]++;
					}

					int label_I = vLabelHorz[i];
					int label_J = vLabelHorz[j];

					if (label_I == label_J)
						continue;

					if (label_J >= 0)
					{
						vLabelHorzCnt[label_I] += vLabelHorzCnt[label_J];
						vLabelHorzCnt[label_J] = 0;

						for (int a = 0; a < nCntBlob; a++)
						{
							if (vLabelHorz[a] == label_J)
								vLabelHorz[a] = label_I;
						}
					}
					else
					{
						vLabelHorz[j] = label_I;
						vLabelHorzCnt[label_I]++;

						// Blob이 충분히 길면 2개로 카운트한다.
						if (double(rcRect[j].bottom - rcRect[j].top) / double(rcRect[j].right - rcRect[j].left) < dRatio)
							vLabelHorzCnt[label_I]++;
					}
				}
			}
		}

		// 아래 For문은 각 라벨마다 직선성분인 Blob들의 전체 Rect와 Area를 계산하는 과정
		for (int i = 0; i < nCntBlob; i++)
		{
			// 같은 라벨로 묶인 blob 개수가 3개 이상이어야 직선 성분으로 판단한다.
			bool bHorz = vLabelHorz[i] >= 0 && vLabelHorzCnt[vLabelHorz[i]] > 2;
			bool bVert = vLabelVert[i] >= 0 && vLabelVertCnt[vLabelVert[i]] > 2;

			if (bHorz)
			{
				int label = vLabelHorz[i];
				vLabelArea[label] += vdArea[i];

				if (vLabelRect[label].width == 0 && vLabelRect[label].height == 0)
				{
					vLabelRect[label].x = rcRect[i].left;
					vLabelRect[label].y = rcRect[i].top;
					vLabelRect[label].width = rcRect[i].right - rcRect[i].left;
					vLabelRect[label].height = rcRect[i].bottom - rcRect[i].top;
				}
				else
				{
					if (vLabelRect[label].x > rcRect[i].left)
						vLabelRect[label].x = rcRect[i].left;
					if (vLabelRect[label].y > rcRect[i].top)
						vLabelRect[label].y = rcRect[i].top;
					if (vLabelRect[label].br().x < rcRect[i].right)
						vLabelRect[label].width = rcRect[i].right - vLabelRect[label].x;
					if (vLabelRect[label].br().y < rcRect[i].bottom)
						vLabelRect[label].height = rcRect[i].bottom - vLabelRect[label].y;
				}
			}

			if (bVert)
			{
				int label = vLabelVert[i];
				vLabelArea[label] += vdArea[i];

				if (vLabelRect[label].width == 0 && vLabelRect[label].height == 0)
				{
					vLabelRect[label].x = rcRect[i].left;
					vLabelRect[label].y = rcRect[i].top;
					vLabelRect[label].width = rcRect[i].right - rcRect[i].left;
					vLabelRect[label].height = rcRect[i].bottom - rcRect[i].top;
				}
				else
				{
					if (vLabelRect[label].x > rcRect[i].left)
						vLabelRect[label].x = rcRect[i].left;
					if (vLabelRect[label].y > rcRect[i].top)
						vLabelRect[label].y = rcRect[i].top;
					if (vLabelRect[label].br().x < rcRect[i].right)
						vLabelRect[label].width = rcRect[i].right - vLabelRect[label].x;
					if (vLabelRect[label].br().y < rcRect[i].bottom)
						vLabelRect[label].height = rcRect[i].bottom - vLabelRect[label].y;
				}
			}
		}
	}

	// 최종적으로 이물 여부 판단 및 결과 검출 과정
	int nErrIdx = 0;
	for (int i = 0; i < nCntBlob; i++)
	{
		int nW = (rcRect[i].right - rcRect[i].left);
		int nH = (rcRect[i].bottom - rcRect[i].top);

		if (m_pAlgoTab->m_bUseCrossLineDetect)
		{
			int labelVert = vLabelVert[i];
			int labelHorz = vLabelHorz[i];

			bool bHorz = labelHorz >= 0 && vLabelHorzCnt[labelHorz] > 2;
			bool bVert = labelVert >= 0 && vLabelVertCnt[labelVert] > 2;

			if (bHorz || bVert)
			{
				// rate : 종횡비(aspect ratio), thickness : 두께
				double rate = 1, rateV = 1, rateH = 1, thickness = 0, thicknessV = 0, thicknessH = 0;
				if (bHorz)
				{
					rate = rateH = (double)vLabelRect[labelHorz].height / (double)vLabelRect[labelHorz].width;
					thickness = thicknessH = (double)vLabelRect[labelHorz].height * m_resolY;
				}
				if (bVert)
				{
					rate = rateV = (double)vLabelRect[labelVert].width / (double)vLabelRect[labelVert].height;
					thickness = thicknessV = (double)vLabelRect[labelVert].width * m_resolX;
				}

				if (bHorz && bVert)
				{
					if (rateH > rateV)
					{
						rate = rateV;
						thickness = thicknessV;
					}
					else
					{
						rate = rateH;
						thickness = thicknessH;
					}
				}

				// 가성 제거 옵션들 계산
				if ((m_pAlgoTab->m_bUseVerticalMaxLen && bVert && vLabelRect[labelVert].height * m_resolY > m_pAlgoTab->m_dVerticalMaxLen)
					|| (m_pAlgoTab->m_bUseHorizontalMaxLen && bHorz && vLabelRect[labelHorz].width * m_resolX > m_pAlgoTab->m_dHorizontalMaxLen)
					|| (m_pAlgoTab->m_bUseMaxNGArea && ((bVert && vLabelArea[labelVert] * fFactormm2 >= m_pAlgoTab->m_dMaxNGArea)
						|| (bHorz && vLabelArea[labelHorz] * fFactormm2 >= m_pAlgoTab->m_dMaxNGArea)))
					|| (m_pAlgoTab->m_bUseScarAspectRatio && rate < m_pAlgoTab->m_dScarAspectRatio)
					|| (m_pAlgoTab->m_bUseMinScarThickness && thickness < m_pAlgoTab->m_dMinScarThickness))
					continue;
			}
		}

		// Max NG Area 옵션
		if (m_pAlgoTab->m_bUseMaxNGArea && vdArea[i] * fFactormm2 >= m_pAlgoTab->m_dMaxNGArea)
			continue;

		std::vector<cv::Point> vStPo;
		float a1, b1;
		if (nW > nH)
		{
			// 수평방향 Blob. 너무 얇거나 Horizontal Max Length 보다 길면 패스
			if ((nH < 4 && rcRect[i].right>(InspRect.width * 2 / 3 + InspRect.x)) || (m_pAlgoTab->m_bUseHorizontalMaxLen && nW * m_resolX > m_pAlgoTab->m_dHorizontalMaxLen))
				continue;
			for (int r = rcRect[i].top; r < rcRect[i].bottom; r++)
			{
				for (int c = rcRect[i].left; c < rcRect[i].right; c++)
				{
					if (ForeignImg.ptr(r)[c] > 0)
						vStPo.push_back(cv::Point(c, r));
				}
			}
		}
		else
		{
			// 수직방향 Blob. 너무 얇거나 Vertical Max Length 보다 길면 패스
			if ((nW < 4 && rcRect[i].right>(InspRect.width * 2 / 3 + InspRect.x)) || (m_pAlgoTab->m_bUseVerticalMaxLen && nH * m_resolY > m_pAlgoTab->m_dVerticalMaxLen))
				continue;
			for (int r = rcRect[i].top; r < rcRect[i].bottom; r++)
			{
				for (int c = rcRect[i].left; c < rcRect[i].right; c++)
				{
					if (ForeignImg.ptr(r)[c] > 0)
						vStPo.push_back(cv::Point(r, c));
				}
			}
		}
		if(vStPo.size()< fMinPixel+1)
			continue;

		float fRate = nW > nH ? (float)nH / nW : (float)nW / nH;
		float fThickness = nW > nH ? (float)nH * m_resolY : (float)nW * m_resolX;
		PCA_1D(vStPo, &a1, &b1);
		if(a1 == 0 && fRate < 0.5)
			continue;
		int nLinePoCnt = 0;
		for (int idx = 0; idx < vStPo.size(); idx++)
		{
			int x = vStPo[idx].x;
			int y = a1 * x + b1;

			if (std::abs(vStPo[idx].y - y) < 2)
			{
				nLinePoCnt++;
			}
		}

		// Aspect Ratio 옵션 체크
		if (!m_pAlgoTab->m_bUseScarAspectRatio|| fRate >= m_pAlgoTab->m_dScarAspectRatio)
		{
			double dArea = vdArea[i] * fFactormm2;

			// 면적 체크
			if (dArea > fErr/2)
			{
				RECT rt;
				rt.left = rcRect[i].left + InspRect.x;
				rt.right = rcRect[i].right + InspRect.x;
				rt.top = rcRect[i].top + InspRect.y;
				rt.bottom = rcRect[i].bottom + InspRect.y;
				rcErrRectTemp.push_back(rt);
				*dDiff += dArea;

			}
		}
	}
	if (*dDiff > fErr) 
	{
		bIsOK = FALSE;
		rcErrRect.insert(rcErrRect.end(), rcErrRectTemp.begin(), rcErrRectTemp.end());
	}
	g_pMManager->pem_delete(rcRect, true);
	g_pMManager->pem_delete(dCenterX, true);
	g_pMManager->pem_delete(dCenterY, true);

	return bIsOK;
}
BOOL CPInsp_Tab::InspArea(RECT rcTabRect, cv::Mat InspImg, cv::Mat GrayImg, float fErr, float* dDiff, float fFactormm2)
{
	cv::Rect InspRect;
	InspRect.x = rcTabRect.left + 7;
	InspRect.y = rcTabRect.top + 1;
	InspRect.width = rcTabRect.right - rcTabRect.left - 14;
	InspRect.height = rcTabRect.bottom - rcTabRect.top - 2;
	if (InspRect.width < 1 || InspRect.height < 1)
		return FALSE;

	cv::Mat ForeignGray = GrayImg(InspRect);
	cv::Mat ForeignImg = InspImg(InspRect).clone();
	//cv::medianBlur(ForeignImg, ForeignImg, medBl);

	float fMinPixel = 5;// fErr / fFactormm2;
	cv::Mat DstImg(ForeignImg.rows, ForeignImg.cols, CV_8UC1);
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ForeignImg.ptr(), DstImg.ptr(), ForeignImg.cols, ForeignImg.rows, fMinPixel, false, false, 0, eSelectMix);
	if (nCntBlob == 0)
		return TRUE;
	BOOL bIsOK = TRUE;
	RECT *rcRect = g_pMManager->pem_new<RECT>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__, true);
	std::vector<double> vdArea;
	m_pProcMilAlgo->GetBlobResult_Rects(rcRect, nCntBlob);
	m_pProcMilAlgo->GetBlobResult_Area(vdArea);
	for (int i = 0; i < nCntBlob; i++)
	{
		if(vdArea[i]<5)
			continue;
		BOOL bHor = FALSE;
		BOOL bVer = FALSE;
 		bHor = verification_TabArea_bin(InspRect, rcRect[i], InspImg, true);
 		bVer = verification_TabArea_bin(InspRect, rcRect[i], InspImg, false);
// 		BOOL bHor = verification_TabArea(InspRect, rcRect[i], GrayImg, true);
// 		BOOL bVer = verification_TabArea(InspRect, rcRect[i], GrayImg, false);
		bIsOK &= (bHor | bVer);
		if (bHor | bVer)
		{
			*dDiff = (vdArea[i] * fFactormm2);
		}
	}
	g_pMManager->pem_delete(rcRect, true);

	return bIsOK;
}
BOOL CPInsp_Tab::InspArea_circle(RECT rcTabRect, cv::Mat InspImg, float fErr, float* dDiff, float fFactormm2)
{
	cv::Rect InspRect;
	InspRect.x = rcTabRect.left + 7;
	InspRect.y = rcTabRect.top + 1;
	InspRect.width = rcTabRect.right - rcTabRect.left - 14;
	InspRect.height = rcTabRect.bottom - rcTabRect.top - 2;
	if (InspRect.width < 1 || InspRect.height < 1)
		return FALSE;

	cv::Mat ForeignImg = InspImg(InspRect).clone();

	float fMinPixel = 5;// fErr / fFactormm2;
	cv::Mat DstImg(ForeignImg.rows, ForeignImg.cols, CV_8UC1);
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ForeignImg.ptr(), DstImg.ptr(), ForeignImg.cols, ForeignImg.rows, fMinPixel, false, false, 0, eSelectMix);
	if (nCntBlob == 0)
		return TRUE;

	BOOL bIsOK = TRUE;
	std::vector<double> vdArea;
	m_pProcMilAlgo->GetBlobResult_Area(vdArea);
	float fResult = 0.0f, fMaxR = 0.0f;;
	std::vector<double> m_fArrRstL;
	for (int i = 0; i < nCntBlob; i++)
	{
		BOOL btmp = TRUE;

		float fRst = 0.0f;
		double fRad = 0.0f;;
		//CircleFitting2(cv::Mat BinImage, int nCntBlob, float *fResult, double *dMaxWidth, std::vector<double> m_fArrRstL)
		//btmp = CircleFitting2(DstImg, nCntBlob, &fRst, &fRad, m_fArrRstL);

		double centX, centY, radius, err;
		std::vector<cv::Point2f> vStPo;

		for (int c = 0; c < DstImg.cols; c++)
		{
			//bool bAdd = false;
			for (int r = 0; r < DstImg.rows; r++)
			{
				if (DstImg.ptr(r)[c] > 0)
				{
					vStPo.push_back(cv::Point2f(c, r));
				}
			}
		}

		CircleFitting(vStPo, &centX, &centY, &radius, &err);
		cv::Mat sCircle(DstImg.rows,DstImg.cols,CV_8UC1);
		sCircle.setTo(0);
		cv::circle(sCircle, cv::Point(centX, centY),radius,cv::Scalar(255), -1);
		sCircle &= DstImg;
		int nCircleCnt = 0;
		for (int c = 0; c < sCircle.cols; c++)
		{
			//bool bAdd = false;
			for (int r = 0; r < sCircle.rows; r++)
			{
				if (sCircle.ptr(r)[c] > 0)
				{
					nCircleCnt++;
				}
			}
		}
		double dAreErr = (vdArea[i] - (double)nCircleCnt) / vdArea[i];
		double dCirArea = std::pow(radius, 2)*PI;
		double dCirErr = (dCirArea - nCircleCnt)/ dCirArea;
		if (err < 0.5f && dCirErr<0.3)
			btmp = FALSE;
		//CircleFitting_KJS(DstImg, nCntBlob, &fRst, &fRad, m_fArrRstL);
		bIsOK &= btmp;
		if (bIsOK)
		{
			*dDiff = (vdArea[i] * fFactormm2);
		}
		if (fRst > fResult)
			fResult = fRst;
		if (fRad > fMaxR)
			fMaxR = fRad;
	}

	return bIsOK;
}
BOOL CPInsp_Tab::InspTail(bool bTailTab, cv::Mat InspImg, RECT rcTabRect, float fStdArea, double* dDiff, std::vector<RECT>& rcErrRect, float fFactormm2)
{
	if (rcTabRect.right - rcTabRect.left < 1 || rcTabRect.bottom - rcTabRect.top < 1)
	{
		RECT rcTab;
		rcTab.left = 3;
		rcTab.top = 3;
		rcTab.right = InspImg.cols - 3;
		rcTab.bottom = InspImg.rows - 3;
		rcErrRect.push_back(rcTab);
		return FALSE;
	}
	int nTailAreaTop = 0, nTailAreaBot = 0;
	int nMax = 0;
	int nStdR = (rcTabRect.bottom - rcTabRect.top)*0.5;
	std::vector<int> vCnt;
	vCnt.resize(rcTabRect.bottom - rcTabRect.top + 1);
	for (int r = rcTabRect.top; r < rcTabRect.bottom; r++)
	{
		UCHAR* ptr = InspImg.ptr(r);

		for (int c = rcTabRect.left - 3; c >= 0; c--)
		{
			if (ptr[c] > 0)
				vCnt[r - rcTabRect.top]++;
			else
				break;
		}
	}
	for (int i = 0; i < vCnt.size() / 2; i++)
	{
		//if (vCnt[i] < nStdR)
			nTailAreaTop += vCnt[i];
	}
	for (int i = vCnt.size() / 2; i < vCnt.size(); i++)
	{
		//if (vCnt[i] < nStdR)
			nTailAreaBot += vCnt[i];
	}
	int Top_Bot_Diff = nTailAreaTop - nTailAreaBot;
	if(Top_Bot_Diff > 50)
		*dDiff = nTailAreaTop * fFactormm2;
	else if (Top_Bot_Diff < -50)
		*dDiff = nTailAreaBot * fFactormm2;
	else
		*dDiff = Top_Bot_Diff > 0 ? Top_Bot_Diff * fFactormm2 : -Top_Bot_Diff * fFactormm2;

	if (*dDiff < fStdArea)
	{
		RECT re;
		re.right = rcTabRect.left;
		if (nTailAreaTop > nTailAreaBot)
		{
			for (int i = 0; i < vCnt.size() / 2; i++)
			{
				if (nMax < vCnt[i])
					nMax = vCnt[i];
			}
			re.left = rcTabRect.left - nMax - 1;
			re.bottom = (rcTabRect.bottom + rcTabRect.top) / 2;
			re.top = rcTabRect.top;
		}
		else
		{
			for (int i = vCnt.size() / 2; i < vCnt.size(); i++)
			{
				if (nMax < vCnt[i])
					nMax = vCnt[i];
			}
			re.left = rcTabRect.left - nMax - 1;
			re.top = (rcTabRect.bottom + rcTabRect.top) / 2;
			re.bottom = rcTabRect.bottom;
		}
		rcErrRect.push_back(re);
		return FALSE;
	}

	for (int i = 0; i < vCnt.size(); i++)
	{
		if (nMax < vCnt[i])
			nMax = vCnt[i];
	}
	while (nMax > 0)
	{
		int nMaxCnt = 0;
		for (int i = 0; i < vCnt.size(); i++)
		{
			if (nMax <= vCnt[i])
				nMaxCnt++;
		}
		if (nMaxCnt > 3)
			break;
		else
			nMax--;
	}
	int nEndCnt = 0;
	int ntotCnt = 0;
	for (int i = 0; i < vCnt.size(); i++)
	{
		if (2 < vCnt[i])
			ntotCnt++;
		if (vCnt[i] >= nMax - 3)
			nEndCnt++;
	}
	float endCntRate = ((float)nEndCnt) / ntotCnt;
	float TotCntRate = ((float)nEndCnt) / vCnt.size();
	if (bTailTab && endCntRate < 0.5 && TotCntRate < 0.33)
	{
		RECT re;
		re.right = rcTabRect.left;
		re.left = rcTabRect.left - nMax - 1;
		if (nTailAreaTop > nTailAreaBot)
		{
			re.bottom = (rcTabRect.bottom + rcTabRect.top) / 2;
			re.top = rcTabRect.top;
		}
		else
		{

			re.top = (rcTabRect.bottom + rcTabRect.top) / 2;
			re.bottom = rcTabRect.bottom;
		}
		rcErrRect.push_back(re);
		return FALSE;
	}

	return TRUE;
}
bool CPInsp_Tab::PCA_1D(std::vector< cv::Point> pint, float* a, float* b)
{
	cv::Mat matrixInsp(pint.size(), 2, CV_32F);
	//cv::Mat MatrixStd(4,2,CV_32F);
	for (int i = 0; i < pint.size(); i++)
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
bool CPInsp_Tab::PCA_1D(std::vector< cv::Point2f> pint, float* a, float* b)
{
	cv::Mat matrixInsp(pint.size(), 2, CV_32F);
	//cv::Mat MatrixStd(4,2,CV_32F);
	for (int i = 0; i < pint.size(); i++)
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
void CPInsp_Tab::CircleFitting(std::vector<cv::Point2f> vPoList,double *centX, double *centY, double *radius, double *err)
{
	double theta;
	double s_cos = 0, s_sin = 0;
	double xy = 0, x = 0, y = 0;

	double err2 = 0;

	double ds_Xn2 = 0, ds_Yn2 = 0, ds_XnYn = 0, ds_Xn = 0, ds_Yn = 0;
	double dsub1 = 0, dsub2 = 0, dsub3 = 0;
	for (int i = 0; i < vPoList.size(); i++)
	{
		double x, y;
		x = vPoList[i].x;
		y = vPoList[i].y;

		ds_Xn2 += x * x;
		ds_Yn2 += y * y;
		ds_XnYn += x * y;
		ds_Xn += x;
		ds_Yn += y;

		dsub1 += x * (x*x + y * y);
		dsub2 += y * (x*x + y * y);
		dsub3 += x * x + y * y;
	}


	int matrixSize = 3;
	int matrixSize2 = matrixSize * matrixSize;

	double org_m[9];
	double sub_m[9];

	org_m[0] = ds_Xn2; org_m[1] = ds_XnYn; org_m[2] = ds_Xn;
	org_m[3] = ds_XnYn; org_m[4] = ds_Yn2; org_m[5] = ds_Yn;
	org_m[6] = ds_Xn; org_m[7] = ds_Yn; org_m[8] = vPoList.size();

	sub_m[0] = dsub1; sub_m[1] = dsub2; sub_m[2] = dsub3;

	//IppStatus ans=ippmInvert_ma_64f(org_m, (int)36*2, (int)12*2, (int)4*2, pBuff, inv_m,
		//(int)36*2, (int)12*2, (int)4*2,(int)3, (int)1);

	//ans=ippmMul_mav_64f(inv_m, 36*2, 12*2, 4*2, 3,3, sub_m, 4*2, 3, org_m, 12*2,4*2,1);

	/**centX = (float)org_m[0]/2;
	*centY = (float)org_m[1]/2;
	*radius = (float)((*centX**centX) + (*centY**centY) + org_m[2]);
	*radius = sqrt(*radius);*/

	cv::Mat org(3, 3, CV_64FC1, org_m);
	cv::Mat sub(3, 1, CV_64FC1, sub_m);
	cv::Mat dst = org.inv() * sub;

	*centX = (float)dst.at<double>(0, 0) / 2;
	*centY = (float)dst.at<double>(1, 0) / 2;
	*radius = (float)((*centX**centX) + (*centY**centY) + dst.at<double>(2, 0));
	*radius = sqrt(*radius);

	if (*radius < 0) *radius = -*radius;

	for (int i = 0; i < vPoList.size(); i++)
	{
		CPoint pt(vPoList[i].x, vPoList[i].y);

		float fDistance = sqrtf(std::powf(pt.y - *centY,2) + std::powf(pt.x - *centX,2));
		float fDiff = *radius - fDistance;
		float fErr = fDiff / (*radius);

		if(fErr<0)
			err2 += 1;

	}

	*err = err2 / vPoList.size();
}
void CPInsp_Tab::CircleFitting(std::vector<cv::Point> vPoList, double *centX, double *centY, double *radius, double *err)
{
	double theta;
	double s_cos = 0, s_sin = 0;
	double xy = 0, x = 0, y = 0;

	double err2 = 0;

	double ds_Xn2 = 0, ds_Yn2 = 0, ds_XnYn = 0, ds_Xn = 0, ds_Yn = 0;
	double dsub1 = 0, dsub2 = 0, dsub3 = 0;
	for (int i = 0; i < vPoList.size(); i++)
	{
		double x, y;
		x = vPoList[i].x;
		y = vPoList[i].y;

		ds_Xn2 += x * x;
		ds_Yn2 += y * y;
		ds_XnYn += x * y;
		ds_Xn += x;
		ds_Yn += y;

		dsub1 += x * (x*x + y * y);
		dsub2 += y * (x*x + y * y);
		dsub3 += x * x + y * y;
	}


	int matrixSize = 3;
	int matrixSize2 = matrixSize * matrixSize;

	double org_m[9];
	double sub_m[9];

	org_m[0] = ds_Xn2; org_m[1] = ds_XnYn; org_m[2] = ds_Xn;
	org_m[3] = ds_XnYn; org_m[4] = ds_Yn2; org_m[5] = ds_Yn;
	org_m[6] = ds_Xn; org_m[7] = ds_Yn; org_m[8] = vPoList.size();

	sub_m[0] = dsub1; sub_m[1] = dsub2; sub_m[2] = dsub3;

	//IppStatus ans=ippmInvert_ma_64f(org_m, (int)36*2, (int)12*2, (int)4*2, pBuff, inv_m,
		//(int)36*2, (int)12*2, (int)4*2,(int)3, (int)1);

	//ans=ippmMul_mav_64f(inv_m, 36*2, 12*2, 4*2, 3,3, sub_m, 4*2, 3, org_m, 12*2,4*2,1);

	/**centX = (float)org_m[0]/2;
	*centY = (float)org_m[1]/2;
	*radius = (float)((*centX**centX) + (*centY**centY) + org_m[2]);
	*radius = sqrt(*radius);*/

	cv::Mat org(3, 3, CV_64FC1, org_m);
	cv::Mat sub(3, 1, CV_64FC1, sub_m);
	cv::Mat dst = org.inv() * sub;
	
	double cX = (float)dst.at<double>(0, 0) / 2;
	double cY = (float)dst.at<double>(1, 0) / 2;
	double rad = sqrt(float((cX*cX) + (cY*cY) + dst.at<double>(2, 0)));
	if (rad < 0) rad = -rad;

	for (int i = 0; i < vPoList.size(); i++)
	{
		CPoint pt(vPoList[i].x, vPoList[i].y);

		float fDistance = sqrtf(std::powf(pt.y - cY, 2) + std::powf(pt.x - cX, 2));
		float fDiff = abs(rad - fDistance);
		float fErr = fDiff / rad;

		if (fErr > 0.15)
			err2 += 1;
	}

	if (centX)	*centX = cX;
	if (centY)	*centY = cY;
	if (radius)	*radius = rad;
	if (err)	*err = err2 / vPoList.size();
}
BOOL CPInsp_Tab::CircleFitting2(cv::Mat BinImage, int nCntBlob, float *fResult, double *dMaxWidth, std::vector<double> m_fArrRstL)
{
	std::vector<CRect> vBlobRects;
	m_pProcMilAlgo->GetBlobResult_RectsV(vBlobRects);
	cv::Mat m_Integ, m_Dev;
	int nMaxIndex(0);
	double *dArrScore = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrScore, nCntBlob);
	memset(dArrScore, 0, sizeof(double)*nCntBlob);
	m_pProcMilAlgo->InspSolderBall(BinImage, vBlobRects, &nMaxIndex, fResult, dArrScore);
	cv::integral(BinImage, m_Integ, m_Dev);
	int nCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
	if (vBlobRects.size() > nMaxIndex)
		*dMaxWidth = vBlobRects[nMaxIndex].Width() * m_resolX;
	for (int a = 0; a < nCnt; a++)
		m_fArrRstL.push_back(dArrScore[a] * 100.0);
	Delete_1DArray(&dArrScore);
	if ((*fResult) > 0.6)
		return TRUE;
	return FALSE;
}
BOOL CPInsp_Tab::CircleFitting_KJS(cv::Mat BinImage, int nCntBlob, float *fResult, double *dMaxWidth, std::vector<double> m_fArrRstL)
{
	CRect rcBlob;
	double dCircle_x, dCircle_y, dRadius, dErrorRate, dArea;
	std::vector<CRect> vBlobRects;
	m_pProcMilAlgo->GetBlobResult_RectsV(vBlobRects);
	cv::Mat m_Integ, m_Dev;
	int nMaxIndex(0);
	//cv::Mat src, CRect *rcBlob, double& dCircle_x, double& dCircle_y, double* dRadius, double* dErrorRate, double *dArea, int nMinBlobArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType,
	//false, false, 0, eSelectBigger
	m_pCPInsp_Algo->BlobCircleCenter_Fit(BinImage, &rcBlob, dCircle_x, dCircle_y, &dRadius, &dErrorRate, &dArea, 4, false, false,0, eSelectMix);

	*fResult = dErrorRate;
	*dMaxWidth = dRadius;

	if ((*fResult) > 0.6)
		return TRUE;
	return FALSE;
}

cv::Mat CPInsp_Tab::ImageThining(cv::Mat Img)
{
	cv::Mat tmp, temp1;

	//	cv::Canny(Img,Img,125,225);

	Img.copyTo(tmp);
	Img.copyTo(temp1);
	//	Img.copySize(tmp);
	//	tmp.setTo(0);
	bool flag = true;

	int cnt(0);
	int nStep(0);
	int nPoint[8] = { 0,0,0,0,0,0,0,0 };
	int ntempPoint[8] = { 0,0,0,0,0,0,0,0 };

	while (flag)
	{
		flag = false;

		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];

				ntempPoint[0] = temp1.data[(r - 1)*Img.cols + c - 1];
				ntempPoint[1] = temp1.data[(r - 1)*Img.cols + c];
				ntempPoint[2] = temp1.data[(r - 1)*Img.cols + c + 1];
				ntempPoint[3] = temp1.data[(r)*Img.cols + c + 1];
				ntempPoint[4] = temp1.data[(r + 1)*Img.cols + c + 1];
				ntempPoint[5] = temp1.data[(r + 1)*Img.cols + c];
				ntempPoint[6] = temp1.data[(r + 1)*Img.cols + c - 1];
				ntempPoint[7] = temp1.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				int nPixCnt(0);
				int nCndi1(0), nCndi2(0);

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;
					if (nPoint[i] == 255)
						nPixCnt++;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;
				if (nPoint[0] == 255)
					nPixCnt++;

				nCndi1 = nPoint[1] & nPoint[7] & nPoint[5];
				nCndi2 = nPoint[3] & nPoint[7] & nPoint[5];

				if (nConnect == 1 && (nPixCnt > 1 && nPixCnt < 7) && nCndi1 == 0 && nCndi2 == 0)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;

					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				int nPixCnt(0);
				int nCndi1(0), nCndi2(0);

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;
					if (nPoint[i] == 255)
						nPixCnt++;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;
				if (nPoint[0] == 255)
					nPixCnt++;

				nCndi1 = nPoint[1] & nPoint[3] & nPoint[5];
				nCndi2 = nPoint[1] & nPoint[3] & nPoint[7];

				if (nConnect == 1 && (nPixCnt > 1 && nPixCnt < 7) && nCndi1 == 0 && nCndi2 == 0)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;
					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		cnt++;
	}

	temp1.copyTo(Img);
	return Img;
}

//NG: FALSE, OK: TRUE
BOOL CPInsp_Tab::verification_TabArea(cv::Rect InspRect, RECT rcRect, cv::Mat GrayImg, bool bHor)
{
	BOOL bIsOK = TRUE;

	cv::Mat gTemp = GrayImg(InspRect);
	int nW = (rcRect.right - rcRect.left);
	int nH = (rcRect.bottom - rcRect.top);
	std::vector<cv::Point2f> vStPo;
	double centX, centY, radius, err;
	int nLinePoCnt = 0;
	float fTheta = 0;

	if (bHor)
	{
		float nRate = (float)nH / nW;
// 		if (nRate < 0.5)
// 			return bIsOK;

		//soble Y
		int x = InspRect.x;
		int y = InspRect.y + rcRect.top - 2 > 0 ? InspRect.y + rcRect.top - 2 : 0;
		int w = gTemp.cols - 1;
		int h = rcRect.bottom - rcRect.top + y + 4 < GrayImg.rows ? rcRect.bottom - rcRect.top + 4 : GrayImg.rows - y - 1;
		if (InspRect.y >= y)
			y = InspRect.y + 1;
		if (h + y >= InspRect.y + InspRect.height)
			h = InspRect.y + InspRect.height - y - 1;
		cv::Mat BlobGray = GrayImg(cv::Rect(x, y, w, h));
		cv::Mat sob(BlobGray.rows, BlobGray.cols, CV_16S);
		sob.setTo(0);
		//calc soble
		for (int r = 1; r < BlobGray.rows; r++)
		{
			for (int c = 0; c < BlobGray.cols; c++)
			{
				sob.ptr<short>(r)[c] = BlobGray.ptr(r)[c] - BlobGray.ptr(r - 1)[c];
			}
		}

		float a1, b1;
		for (int c = 0; c < sob.cols; c++)
		{
			//bool bAdd = false;
			for (int r = 1; r < sob.rows; r++)
			{
				if (sob.ptr<short>(r - 1)[c] > 0 && sob.ptr<short>(r)[c] < 0)
				{
					float fTemp = sob.ptr<short>(r)[c] - sob.ptr<short>(r - 1)[c];
					if (fTemp < 10) continue;
					fTemp = sob.ptr<short>(r - 1)[c] / (fTemp);
					vStPo.push_back(cv::Point2f(c, r - fTemp));
					//bAdd = true;
				}
			}
// 			if (!bAdd)
// 				vStPo.push_back(cv::Point2f(c, sob.rows));
		}
		if (vStPo.size() < 2)
		{

			for (int c = 0; c < sob.cols; c++)
			{
				//bool bAdd = false;
				for (int r = 1; r < sob.rows; r++)
				{
					if (//(sob.ptr<short>(r - 1)[c] <= 0 && sob.ptr<short>(r)[c] > 0) ||
						(sob.ptr<short>(r - 1)[c] < sob.ptr<short>(r)[c]))
					{
						float fTemp = sob.ptr<short>(r - 1)[c] - sob.ptr<short>(r)[c];
						if (fTemp > -10) continue;
						fTemp = sob.ptr<short>(r)[c] / (fTemp);
						vStPo.push_back(cv::Point2f(c, r + fTemp));
						//bAdd = true;
					}
				}
// 			if (!bAdd)
// 				vStPo.push_back(cv::Point2f(c, sob.rows));
			}
		}
		if (vStPo.size() == 0) return bIsOK;
		PCA_1D(vStPo, &a1, &b1);
		CircleFitting(vStPo, &centX, &centY, &radius, &err);
		// 			if (a1 == 0 || a1 == INFINITY || a1 == -INFINITY)
		// 				continue;
		if (radius < nW) bIsOK = FALSE;
		for (int idx = 0; idx < vStPo.size(); idx++)
		{
			//if (vStPo[idx].y == sob.rows) continue;
			int x = vStPo[idx].x;
			int y = a1 * x + b1;

			if (std::abs(vStPo[idx].y - y) < 1)
			{
				nLinePoCnt++;
			}
		}
		int nAvgX = 0;
		float dRadi = std::atanf(a1);
		float dDeg = dRadi * 180 / PI; 
		fTheta = std::abs(dDeg);
		if ((nLinePoCnt == 0 && fTheta > 75))
		{
			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				nAvgX += vStPo[idx].x;
			}
			nAvgX = nAvgX / vStPo.size();

			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				if (std::abs(vStPo[idx].x - nAvgX) < 2)
				{
					nLinePoCnt++;
				}
			}
		}
	}
	else
	{
		float nRate = (float)nW / nH;
// 		if (nRate < 0.5)
// 			return bIsOK;

		//soble X
// 			int x = InspRect.x + rcRect[i].left - 2 > 0 ? InspRect.x + rcRect[i].left - 2 : 0;
// 			int y = InspRect.y + rcRect[i].top - 2 > 0 ? InspRect.y + rcRect[i].top - 2 : 0;
// 			int w = rcRect[i].right - rcRect[i].left + x + 4 < GrayImg.cols ? rcRect[i].right - rcRect[i].left + 4 : GrayImg.cols - x - 1;
// 			int h = rcRect[i].bottom - rcRect[i].top + y + 4 < GrayImg.rows ? rcRect[i].bottom - rcRect[i].top + 4 : GrayImg.rows - y - 1;
		int x = InspRect.x + rcRect.left - 2 > 0 ? InspRect.x + rcRect.left - 2 : 0;
		int y = InspRect.y;
		int w = rcRect.right - rcRect.left + x + 4 < GrayImg.cols ? rcRect.right - rcRect.left + 4 : GrayImg.cols - x - 1;
		int h = gTemp.rows - 1;
		if (InspRect.x >= x)
			x = InspRect.x + 1;
		if (w + x >= InspRect.x + InspRect.width)
			w = InspRect.x + InspRect.width - x - 1;
		cv::Mat BlobGray = GrayImg(cv::Rect(x, y, w, h));
		cv::Mat sob(BlobGray.rows, BlobGray.cols, CV_16S);
		sob.setTo(0);
		//calc soble
		for (int r = 0; r < BlobGray.rows; r++)
		{
			for (int c = 1; c < BlobGray.cols; c++)
			{
				sob.ptr<short>(r)[c] = BlobGray.ptr(r)[c] - BlobGray.ptr(r)[c - 1];
			}
		}

		float a1, b1;
		for (int r = 0; r < sob.rows; r++)
		{
			bool bAdd = false;
			for (int c = 1; c < sob.cols; c++)
			{
				if (sob.ptr<short>(r)[c - 1] > 0 && sob.ptr<short>(r)[c] < 0)
				{
					float fTemp = sob.ptr<short>(r)[c] - sob.ptr<short>(r)[c - 1];
					if(fTemp > -10) continue;
					fTemp = sob.ptr<short>(r)[c - 1] / (fTemp);
					vStPo.push_back(cv::Point2f(c - fTemp, r));
					bAdd = true;
				}
			}
// 			if (!bAdd)
// 				vStPo.push_back(cv::Point2f(sob.cols, r));
		}
		if (vStPo.size() < 2)
		{

			for (int c = 0; c < sob.cols; c++)
			{
				//bool bAdd = false;
				for (int r = 1; r < sob.rows; r++)
				{
					if (//(sob.ptr<short>(r)[c - 1] <= 0 && sob.ptr<short>(r)[c] > 0) ||
						(sob.ptr<short>(r)[c - 1]< sob.ptr<short>(r)[c]))
					{
						float fTemp = sob.ptr<short>(r)[c - 1] - sob.ptr<short>(r)[c];
						if (fTemp > -10) continue;
						fTemp = sob.ptr<short>(r)[c] / (fTemp);
						vStPo.push_back(cv::Point2f(c + fTemp, r));
						//bAdd = true;
					}
				}
// 			if (!bAdd)
// 				vStPo.push_back(cv::Point2f(c, sob.rows));
			}
		}
		if (vStPo.size() == 0) return bIsOK;
		PCA_1D(vStPo, &a1, &b1);
		CircleFitting(vStPo, &centX, &centY, &radius, &err);

		if (radius < nW) bIsOK = FALSE;
		
		for (int idx = 0; idx < vStPo.size(); idx++)
		{
			//if (vStPo[idx].x == sob.cols) continue;
			int x = vStPo[idx].x;
			int y = a1 * x + b1;

			if (std::abs(vStPo[idx].y - y) < 2)
			{
				nLinePoCnt++;
			}
		}
		int nAvgX = 0;
		float dRadi = std::atanf(a1);
		float dDeg = dRadi * 180 / PI;
		fTheta = std::abs(dDeg);
		if ((nLinePoCnt == 0 && fTheta > 75))
		{
			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				nAvgX += vStPo[idx].x;
			}
			nAvgX = nAvgX / vStPo.size();

			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				if (std::abs(vStPo[idx].x - nAvgX) < 2)
				{
					nLinePoCnt++;
				}
			}
		}
	}
	if (err > 0.50 //(rcRect.left <= centX && centX <= rcRect.right) || (rcRect.top <= centY && centY <= rcRect.bottom) 
		|| (((float)nLinePoCnt / vStPo.size()) > 0.7 && (fTheta<15 || fTheta>75)))
		return bIsOK;

	return FALSE;
}

//NG: FALSE, OK: TRUE
BOOL CPInsp_Tab::verification_TabArea_bin(cv::Rect InspRect, RECT rcRect, cv::Mat BinImage, bool bHor)
{
	BOOL bIsOK = TRUE;

	cv::Mat binTemp = BinImage(InspRect);
	int nW = (rcRect.right - rcRect.left);
	int nH = (rcRect.bottom - rcRect.top);
	std::vector<cv::Point2f> vStPo;
	double centX, centY, radius, err;
	int nLinePoCnt = 0;
	float fTheta = 0;

	if (bHor)
	{
		float nRate = (float)nH / nW;
		// 		if (nRate < 0.5)
		// 			return bIsOK;

				//soble Y
		int x = 0;
		int y = rcRect.top - 2 > 0 ? rcRect.top - 2 : 0;
		int w = binTemp.cols - 1;
		int h = rcRect.bottom - rcRect.top + y + 4 < binTemp.rows ? rcRect.bottom - rcRect.top + 4 : binTemp.rows - y - 1;

		cv::Mat BlobBin = binTemp(cv::Rect(x, y, w, h));
		//calc soble
		for (int r = 0; r < BlobBin.rows; r++)
		{
			for (int c = 0; c < BlobBin.cols; c++)
			{
				if (BlobBin.ptr(r)[c] > 0)
					vStPo.push_back(cv::Point2f(c, r));
			}
		}

		float a1, b1;
		if (vStPo.size() == 0) return bIsOK;
		PCA_1D(vStPo, &a1, &b1);
		CircleFitting(vStPo, &centX, &centY, &radius, &err);
		// 			if (a1 == 0 || a1 == INFINITY || a1 == -INFINITY)
		// 				continue;
		if (radius < nW) bIsOK = FALSE;
		for (int idx = 0; idx < vStPo.size(); idx++)
		{
			//if (vStPo[idx].y == sob.rows) continue;
			int x = vStPo[idx].x;
			int y = a1 * x + b1;

			if (std::abs(vStPo[idx].y - y) < 2)
			{
				nLinePoCnt++;
			}
		}
		int nAvgX = 0;
		float dRadi = std::atanf(a1);
		float dDeg = dRadi * 180 / PI;
		fTheta = std::abs(dDeg);
		if ((nLinePoCnt == 0 && fTheta > 75))
		{
			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				nAvgX += vStPo[idx].x;
			}
			nAvgX = nAvgX / vStPo.size();

			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				if (std::abs(vStPo[idx].x - nAvgX) < 2)
				{
					nLinePoCnt++;
				}
			}
		}
	}
	else
	{
		float nRate = (float)nW / nH;
		// 		if (nRate < 0.5)
		// 			return bIsOK;

				//soble X
		// 			int x = InspRect.x + rcRect[i].left - 2 > 0 ? InspRect.x + rcRect[i].left - 2 : 0;
		// 			int y = InspRect.y + rcRect[i].top - 2 > 0 ? InspRect.y + rcRect[i].top - 2 : 0;
		// 			int w = rcRect[i].right - rcRect[i].left + x + 4 < GrayImg.cols ? rcRect[i].right - rcRect[i].left + 4 : GrayImg.cols - x - 1;
		// 			int h = rcRect[i].bottom - rcRect[i].top + y + 4 < GrayImg.rows ? rcRect[i].bottom - rcRect[i].top + 4 : GrayImg.rows - y - 1;
		int x = rcRect.left - 2 > 0 ? rcRect.left - 2 : 0;
		int y = 0;
		int w = rcRect.right - rcRect.left + x + 4 < binTemp.cols ? rcRect.right - rcRect.left + 4 : binTemp.cols - x - 1;
		int h = binTemp.rows - 1;

		cv::Mat BlobBin = binTemp(cv::Rect(x, y, w, h));
		//calc soble
		for (int r = 0; r < BlobBin.rows; r++)
		{
			for (int c = 0; c < BlobBin.cols; c++)
			{
				if (BlobBin.ptr(r)[c] > 0)
					vStPo.push_back(cv::Point2f(r, c));
			}
		}

		float a1, b1;
		if (vStPo.size() == 0) return bIsOK;
		PCA_1D(vStPo, &a1, &b1);
		CircleFitting(vStPo, &centX, &centY, &radius, &err);

		if (radius < nW) bIsOK = FALSE;

		for (int idx = 0; idx < vStPo.size(); idx++)
		{
			//if (vStPo[idx].x == sob.cols) continue;
			int x = vStPo[idx].x;
			int y = a1 * x + b1;

			if (std::abs(vStPo[idx].y - y) < 2)
			{
				nLinePoCnt++;
			}
		}
		int nAvgX = 0;
		float dRadi = std::atanf(a1);
		float dDeg = dRadi * 180 / PI;
		fTheta = std::abs(dDeg);
		if ((nLinePoCnt == 0 && fTheta > 75))
		{
			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				nAvgX += vStPo[idx].x;
			}
			nAvgX = nAvgX / vStPo.size();

			for (int idx = 0; idx < vStPo.size(); idx++)
			{
				if (std::abs(vStPo[idx].x - nAvgX) < 2)
				{
					nLinePoCnt++;
				}
			}
		}
	}
	if (err > 0.50 //(rcRect.left <= centX && centX <= rcRect.right) || (rcRect.top <= centY && centY <= rcRect.bottom) 
		|| (((float)nLinePoCnt / vStPo.size()) > 0.7 && (fTheta < 15 || fTheta>75)))
		return bIsOK;

	return FALSE;
}
