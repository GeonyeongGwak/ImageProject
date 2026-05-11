#include "FOREIGN_INSP.h"
#include "../MPTI.h"

FOREIGN_INSP::FOREIGN_INSP(void)
{
	m_resolX = 0;
	m_resolY = 0;
	m_fovWidth = 0;
	m_fovLength = 0;
	m_ForeignParamROI = NULL;
	m_pInspBoardInfo_Foreign = NULL;
	m_inspForeignResult = NULL;
	for (int i = 0; i < eM2C_NUM; i++)
	{
		m_ucForeignHalfBuffer[i] = NULL;
		m_ucForeignQuaterBuffer[i] = NULL;
		m_ucForeignHalfOrgBuffer[i] = NULL;
		m_ucForeignQuaterOrgBuffer[i] = NULL;
	}
	for (int i = 0; i < FOREIGN2D_RSTCNT; i++)
	{
		m_ucForeignHalfRstBuffer[i] = NULL;
		m_ucForeignQuaterRstBuffer[i] = NULL;
	}
}
FOREIGN_INSP::~FOREIGN_INSP(void)
{
	CloseDevice();
}

#pragma region FR
void FOREIGN_INSP::InitAlgo(int fovWidth, int fovLength, double resolX, double resolY)
{
	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	int nHalfArea = ((fovWidth / 2) - 1) * ((fovLength / 2) - 1);
	int nQuaterArea = ((fovWidth / 4) - 1) * ((fovLength / 4) - 1);
	for (int i = 0; i < eM2C_NUM; i++)
	{
		m_ucForeignHalfBuffer[i] = g_pMManager->pem_new<UCHAR>(true, nHalfArea, (PCHAR)__FUNCTION__, __LINE__, true);
		memset(m_ucForeignHalfBuffer[i], 0, nHalfArea);

		m_ucForeignQuaterBuffer[i] = g_pMManager->pem_new<UCHAR>(true, nQuaterArea, (PCHAR)__FUNCTION__, __LINE__, true);
		memset(m_ucForeignQuaterBuffer[i], 0, nQuaterArea);

		m_ucForeignHalfOrgBuffer[i] = g_pMManager->pem_new<UCHAR>(true, nHalfArea, (PCHAR)__FUNCTION__, __LINE__, true);
		memset(m_ucForeignHalfOrgBuffer[i], 0, nHalfArea);

		m_ucForeignQuaterOrgBuffer[i] = g_pMManager->pem_new<UCHAR>(true, nQuaterArea, (PCHAR)__FUNCTION__, __LINE__, true);
		memset(m_ucForeignQuaterOrgBuffer[i], 0, nQuaterArea);
	}
	for (int i = 0; i < FOREIGN2D_RSTCNT; i++)
	{
		m_ucForeignHalfRstBuffer[i] = g_pMManager->pem_new<UCHAR>(true, nHalfArea, (PCHAR)__FUNCTION__, __LINE__, true);
		memset(m_ucForeignHalfRstBuffer[i], 0, nHalfArea);

		m_ucForeignQuaterRstBuffer[i] = g_pMManager->pem_new<UCHAR>(true, nQuaterArea, (PCHAR)__FUNCTION__, __LINE__, true);
		memset(m_ucForeignQuaterRstBuffer[i], 0, nQuaterArea);
	}
	Init_PseudoMap();
}
void FOREIGN_INSP::Init_PseudoMap()
{
	int nTempB = 128;
	int nTempG = 0;
	int nTempR = 0;

	for (int i = 0; i < CR_MAP_SIZE; i++)
	{
		if (nTempB > 255) nTempB = 255; else if (nTempB < 0) nTempB = 0;
		if (nTempG > 255) nTempG = 255; else if (nTempG < 0) nTempG = 0;
		if (nTempR > 255) nTempR = 255; else if (nTempR < 0) nTempR = 0;

		m_crPseudo[i].r = nTempR;
		m_crPseudo[i].g = nTempG;
		m_crPseudo[i].b = nTempB;

		if (i < 382) nTempB++;
		else nTempB--;

		if (i < 127) nTempG--;
		else if (i < 892) nTempG++;
		else nTempG--;

		if (i < 637) nTempR--;
		else if (i < 1147) nTempR++;
		else nTempR--;
	}
}
void FOREIGN_INSP::CloseDevice()
{
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

	if (m_ForeignParamROI != NULL)
		g_pMManager->pem_delete(m_ForeignParamROI, true);
	m_ForeignParamROI = NULL;

	for (int i = 0; i < eM2C_NUM; i++)
	{
		if (m_ucForeignHalfBuffer[i] != NULL)
		{
			g_pMManager->pem_delete(m_ucForeignHalfBuffer[i], true);
			m_ucForeignHalfBuffer[i] = NULL;
		}

		if (m_ucForeignQuaterBuffer[i] != NULL)
		{
			g_pMManager->pem_delete(m_ucForeignQuaterBuffer[i], true);
			m_ucForeignQuaterBuffer[i] = NULL;
		}

		if (m_ucForeignHalfOrgBuffer[i] != NULL)
		{
			g_pMManager->pem_delete(m_ucForeignHalfOrgBuffer[i], true);
			m_ucForeignHalfOrgBuffer[i] = NULL;
		}

		if (m_ucForeignQuaterOrgBuffer[i] != NULL)
		{
			g_pMManager->pem_delete(m_ucForeignQuaterOrgBuffer[i], true);
			m_ucForeignQuaterOrgBuffer[i] = NULL;
		}
	}

	for (int i = 0; i < FOREIGN2D_RSTCNT; i++)
	{
		if (m_ucForeignHalfRstBuffer[i] != NULL)
		{
			g_pMManager->pem_delete(m_ucForeignHalfRstBuffer[i], true);
			m_ucForeignHalfRstBuffer[i] = NULL;
		}

		if (m_ucForeignQuaterRstBuffer[i] != NULL)
		{
			g_pMManager->pem_delete(m_ucForeignQuaterRstBuffer[i], true);
			m_ucForeignQuaterRstBuffer[i] = NULL;
		}
	}
}
#pragma endregion FR

#pragma region MINSPM
void FOREIGN_INSP::SetForeignParam(InspForeignInfo ForeignParam, ForeignParamROI* vForeignParamROI)
{
	ForeignParam.Clone(m_ForeignData);

	if (m_ForeignParamROI != NULL)
	{
		g_pMManager->pem_delete(m_ForeignParamROI, true);
		m_ForeignParamROI = NULL;
	}

	if (vForeignParamROI)
	{
		int nTotal = m_ForeignData.m_nArrData[FR_N_ForeignParamROI_Total];
		int nTotalCnt = (nTotal > 0) ? nTotal : 1;
		m_ForeignParamROI = g_pMManager->pem_new<ForeignParamROI>(true, nTotalCnt, (PCHAR)__FUNCTION__, __LINE__, true);
		for (int a = 0; a < nTotal; a++)
		{
			vForeignParamROI[a].Clone(m_ForeignParamROI[a]);
		}
	}
}
bool FOREIGN_INSP::PolygonAreaCalc()
{
	return true;
}
void FOREIGN_INSP::FillPolygon(cv::Mat Dst, POINTF *pts, int nMaxCount, cv::Scalar color)
{
	int width = Dst.cols;
	int height = Dst.rows;

	std::vector<cv::Point> contour;
	for (size_t j = 0; j < nMaxCount; j++)
		contour.push_back(cv::Point(pts[j].x, pts[j].y));

	const cv::Point *pts3 = (cv::Point*) cv::Mat(contour).data;
	fillPoly(Dst, &pts3, &nMaxCount, 1, color);
}
void FOREIGN_INSP::MorErode(cv::Mat Src, cv::Mat Dst, int nShape, int ksize)
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
int FOREIGN_INSP::FR_EXCEPT(FR_Bin sData, FR_EXT_SRC sInspData, FR_EXT_RST* vROI)
{
	int ret = 0;
	if (sInspData.m_p3D == NULL || vROI == NULL)
		return -2;

	InspForeignInfo sForeign = m_ForeignData;
	bool bSaveImg = sForeign.UseData(FR_DATA_DebugData);
	bool bSaveImg2 = sForeign.UseData(FR_DATA_DebugData2);
	int nFovID = 0;
	int projectionmode = sInspData.m_nProjectionmode;
	int nOrgW = m_fovWidth;
	int nOrgH = m_fovLength;
	int nDivide = m_ForeignData.GetDivide();
	int nW = nOrgW / nDivide - 1;
	int nH = nOrgH / nDivide - 1;
	int nHalfS = nOrgW / 2;
	int nHalfE = nOrgH / 2;

	cv::Mat img3D_Bin(nH, nW, CV_8UC1, cv::Scalar(255));
	cv::Mat img2D_Bin(nH, nW, CV_8UC1, cv::Scalar(255));
	if (sData.UseData(FR_BIN_DT_3D))
	{
		float *pfOrg3D = sInspData.m_p3D;
		cv::Mat imgORG(nOrgH, nOrgW, CV_32FC1, pfOrg3D);
		cv::Mat img3D(nH, nW, CV_32FC1, cv::Scalar(0));
		imgORG(cv::Rect(0, 0, nW, nH)).copyTo(img3D);
		int nRange3D = sData.m_nArr[FR_BIN_N_Range3D];
		float f3DMin = sData.m_fArr[FR_BIN_F_Min3D];
		float f3DMax = sData.m_fArr[FR_BIN_F_Max3D];
		if (f3DMin > f3DMax) f3DMax = f3DMin;
		if (nRange3D == ETypeInspRange::eTypeRangeOut)
		{
			cv::Mat img3DMin(nH, nW, CV_8UC1, cv::Scalar(0));
			cv::Mat img3DMax(nH, nW, CV_8UC1, cv::Scalar(0));

			ippiCompareC_32f_C1R(img3D.ptr<float>(), img3D.step, f3DMax, img3DMin.data, img3DMin.cols, { nW, nH }, ippCmpGreaterEq);
			ippiCompareC_32f_C1R(img3D.ptr<float>(), img3D.step, f3DMin, img3DMax.data, img3DMax.cols, { nW, nH }, ippCmpLessEq);

			cv::bitwise_or(img3DMin, img3DMax, img3D_Bin);
		}
		else
		{
			if (nRange3D != ETypeInspRange::eTypeRangeLower)
			{
				ippiCompareC_32f_C1R(img3D.ptr<float>(), img3D.step, f3DMin, img3D_Bin.data, img3D_Bin.cols, { nW, nH }, ippCmpGreaterEq);
			}
			if (nRange3D != ETypeInspRange::eTypeRangeUpper)
			{
				ippiCompareC_32f_C1R(img3D.ptr<float>(), img3D.step, f3DMax, img3D_Bin.data, img3D_Bin.cols, { nW, nH }, ippCmpLessEq);
			}
		}
	}
	if (sData.UseData(FR_BIN_DT_Color) || sData.UseData(FR_BIN_DT_Gray))
	{
		cv::Mat imgORG2D[eM2C_NUM];
		Get2DFOV(projectionmode, NULL, eMI_WindowTeaching, imgORG2D, nDivide, nFovID, bSaveImg);
		Get2DForeign(sForeign, sData, 0, NULL, img2D_Bin.data, eMI_WindowTeaching, imgORG2D, nDivide, nFovID, bSaveImg);
	}

	cv::Mat imgBin(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBlob(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::bitwise_and(img3D_Bin, img2D_Bin, imgBin);

	double dArea = sData.m_arrINSP[m_eFR_Bin_INSP_A].m_fArr[m_eFR_Bin_INSP_F_Min];
	double dAreaPix = dArea / m_resolX / m_resolY / (nDivide * nDivide);
	int nMinPix = 16 / nDivide;
	if (dAreaPix <= nMinPix) dAreaPix = nMinPix;

	std::vector<std::vector<cv::Point>> allRst = CalcBlob(imgBin, imgBlob, (int)dAreaPix);
	int nCntBlob = allRst.size();

	if (sInspData.m_pDesImgBin != NULL && sInspData.m_nFovW > 0 && sInspData.m_nFovH > 0)
	{
		cv::Mat imgDesImgBin(sInspData.m_nFovH, sInspData.m_nFovW, CV_8UC1, sInspData.m_pDesImgBin);
		if (sInspData.m_nFovW == nW && sInspData.m_nFovH == nH)
			memcpy(sInspData.m_pDesImgBin, imgBlob.ptr<UCHAR>(), nW * nH * sizeof(UCHAR));
		else
		{
			cv::Mat imgRst(sInspData.m_nFovH, sInspData.m_nFovW, CV_8UC1, cv::Scalar(0));
			cv::resize(imgBlob, imgRst, cv::Size(sInspData.m_nFovW, sInspData.m_nFovH));
			memcpy(sInspData.m_pDesImgBin, imgRst.ptr<UCHAR>(), sInspData.m_nFovW * sInspData.m_nFovH * sizeof(UCHAR));
		}
	}

	if (nCntBlob <= 0)
		return ret;

	int nMorErode = sForeign.m_nArrData[FR_N_ExtraPixelETC2];
	for (int a = 0; a < nCntBlob && a < FR_EXT_RST_MAX; a++)
	{
		cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::drawContours(imgTemp, allRst, a, cv::Scalar(255), cv::FILLED);
		cv::Mat imgTempA(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::bitwise_and(imgBlob, imgTemp, imgTempA);
		cv::Rect roiTemp = cv::boundingRect(imgTempA);
		CRect rect = CRect(roiTemp.x, roiTemp.y, roiTemp.br().x, roiTemp.br().y);

		if (sInspData.m_nPAD > 0)
		{
			int nX = rect.CenterPoint().x * nDivide;
			int nY = rect.CenterPoint().y * nDivide;
			cv::Rect roi(rect.left, rect.top, rect.Width(), rect.Height());
			cv::Mat imgPATBin = imgTempA(roi);
			double dWmm = imgPATBin.cols * nDivide * m_resolX;
			double dHmm = imgPATBin.rows * nDivide * m_resolY;
			int nWpat = dWmm / 0.01;
			int nHpat = dHmm / 0.01;
			cv::Mat imgPAT;
			cv::resize(imgPATBin, imgPAT, cv::Size(nWpat, nHpat));
			CString str;
			str.Format(_T("D:\\Eagle3D_data\\PAT_IMG\\PAD\\%d_%d_.bmp"), nX, nY);
			cv::imwrite(std::string(CT2A(str)), imgPAT);
		}

		cv::Mat imgDst;
		if (nMorErode > 0)
		{
			int nMor = 1 + (nMorErode * 2);
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nMor, nMor));
			cv::dilate(imgTempA, imgDst, kernel);
		}
		else
			imgDst = imgTempA.clone();

		int nArea = cv::countNonZero(imgDst);
		std::vector<std::vector<cv::Point>> allcontour;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(imgDst, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		if (allcontour.size() == 0 || allcontour.size() > 1)
			continue;

		int nSize = allcontour[0].size();
		if (nSize < 3)
			continue;

		int nGAP = nMorErode * 2;
		if (nMorErode <= 0) nGAP = INSP_MIN_SIZE;
		int nL = rect.left - nGAP;
		int nR = rect.right + nGAP;
		int nT = rect.top - nGAP;
		int nB = rect.bottom + nGAP;
		if (nL < 0) nL = 0;
		if (nL >= nW) nL = nW - 1;
		if (nR < 0) nR = 0;
		if (nR >= nW) nR = nW;
		if (nT < 0) nT = 0;
		if (nT >= nH) nT = nH - 1;
		if (nB < 0) nB = 0;
		if (nB >= nH) nB = nH;
		if (nL >= nR || nR - nL <= nGAP ||
			nT >= nB || nB - nT <= nGAP)
			continue;

		vROI->m_pROI[ret].m_nPoly = 0;
		vROI->m_pROI[ret].m_rcROI.left = nL * nDivide;
		vROI->m_pROI[ret].m_rcROI.right = nR * nDivide;
		vROI->m_pROI[ret].m_rcROI.top = nT * nDivide;
		vROI->m_pROI[ret].m_rcROI.bottom = nB * nDivide;
		if (nSize <= 4)
		{
			ret++;
			if (ret >= FR_EXT_RST_MAX) break;
			continue;
		}

		std::vector<std::vector<cv::Point>> arrVec;
		if (nSize > ZMPOLYPOINT_CNTS)
		{
			int nTry = nSize / 2;
			for (int b = 1; b < nTry; b++)
			{
				std::vector<cv::Point> arr;
				cv::approxPolyDP(allcontour[0], arr, b, true);
				int nCnt = arr.size();
				if (nCnt < 3 || nCnt > ZMPOLYPOINT_CNTS)
					continue;

				arrVec.push_back(arr);
				break;
			}
		}
		else
			arrVec.push_back(allcontour[0]);

		if (arrVec.size() != 1)
			continue;

		cv::Mat imgMAX(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::drawContours(imgMAX, arrVec, 0, cv::Scalar(255), cv::FILLED);

		int nTotalCnt = arrVec[0].size();
		if (nTotalCnt < 3 || nTotalCnt > ZMPOLYPOINT_CNTS)
			continue;

		int nQUO = ZMPOLYPOINT_CNTS / nTotalCnt;
		int nREM = ZMPOLYPOINT_CNTS % nTotalCnt;
		if (nQUO == 0)
			continue;

		int nIdx = 0;
		for (int b = 0; b < nTotalCnt; b++)
		{
			for (int c = 0; c < nQUO; c++)
			{
				vROI->m_pROI[ret].m_pROI[nIdx].x = arrVec[0][b].x * nDivide;
				vROI->m_pROI[ret].m_pROI[nIdx].y = arrVec[0][b].y * nDivide;
				nIdx++;
				if (nIdx >= ZMPOLYPOINT_CNTS)
					break;
			}
			if (nIdx >= ZMPOLYPOINT_CNTS)
				break;
			if (nREM > b)
			{
				vROI->m_pROI[ret].m_pROI[nIdx].x = arrVec[0][b].x * nDivide;
				vROI->m_pROI[ret].m_pROI[nIdx].y = arrVec[0][b].y * nDivide;
				nIdx++;
			}
			if (nIdx >= ZMPOLYPOINT_CNTS)
				break;
		}
		if (nIdx != ZMPOLYPOINT_CNTS)
			continue;

		vROI->m_pROI[ret].m_nPoly = 1;
		ret++;
		if (ret >= FR_EXT_RST_MAX) break;
	}
	return ret;
}
int FOREIGN_INSP::FR_HIT_AREA(FR_FIND_HIT sInspData, POINT* poPoly)
{
	int ret = 0;
	if (sInspData.m_pDesImg == NULL || poPoly == NULL)
		return -2;

	UCHAR* puc = sInspData.m_pDesImg;
	POINTF* arrPo = sInspData.m_pPo;
	ColorXYInfoForeign vCol = sInspData.m_vCol;
	int nWidth = sInspData.m_nW;
	int nHeight = sInspData.m_nH;
	int nFovL = sInspData.m_rcFOV.left;
	int nFovT = sInspData.m_rcFOV.top;
	int nFovW = sInspData.m_rcFOV.right - sInspData.m_rcFOV.left;
	int nFovH = sInspData.m_rcFOV.bottom - sInspData.m_rcFOV.top;
	int nGrayMin = 0;
	int nGray = sInspData.m_nGray;
	if (nGray < 0) nGray = 0;
	int nGrayRange = ETypeInspRange::eTypeRangeLower;
	int nAREA_CNT_MIN = 3;
	double dMinPer = (double)sInspData.m_nPer / 100;
	int nA_T = 0;
	if (nWidth <= INSP_MIN_SIZE || nHeight <= INSP_MIN_SIZE)
		return -3;
	else if (nFovL < 0 || nFovT < 0 || nFovL + nFovW >= nWidth || nFovT + nFovH >= nHeight)
		return -4;
	if (dMinPer <= 0.5) dMinPer = 0.5;
	if (dMinPer >= 1) dMinPer = 0.99;

	cv::Mat imgFull(nHeight, nWidth, CV_8UC3, puc);
	cv::Mat imgRGB_ORG[FR_Find_RGB_Total];
	cv::split(imgFull, imgRGB_ORG);

	cv::Mat imgTemp(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBin(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBTM(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBTM_BLOB(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Rect roiM1(nFovL, nFovT, nFovW, nFovH);

	cv::Mat imgRGB[FR_Find_RGB_Total];
	imgTemp(roiM1).setTo(255);
	for (int a = 0; a < FR_Find_RGB_Total; a++)
		cv::bitwise_and(imgRGB_ORG[a], imgTemp, imgRGB[a]);

	for (int nType = 0; nType < 2; nType++)
	{
		bool bBTM = (nType == 1);
		if (nType == 0)
		{
			if (sInspData.UseData(FR_HIT_AREA_DATA_GRAY) == false)
				continue;
		}
		else if (bBTM)
		{
			if (sInspData.UseData(FR_HIT_AREA_DATA_BTM_GRAY) == false)
				continue;
			CompareCIE_BTM(imgRGB, imgBTM, vCol, arrPo);
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			cv::erode(imgBTM, imgBTM_BLOB, kernel);  // 침식 적용 (반복 횟수 조절 가능)
		}

		for (int nBinRGB = -1; nBinRGB < FR_Find_RGB_Total; nBinRGB++)
		{
			int nBinS = 0;
			int nBinE = 5;
			for (int nBin = nBinS; nBin <= nBinE; nBin++)
			{
				int nGrayMax = nGray + (nBin * 5);
				imgBin.setTo(0);
				imgBin(roiM1).setTo(255);
				for (int a = 0; a < FR_Find_RGB_Total; a++)
				{
					if (nBinRGB >= 0 && nBinRGB < FR_Find_RGB_Total && a != nBinRGB)
						continue;
					if (a == FR_Find_RGB_TopB)
					{
						if (sInspData.UseData(FR_HIT_AREA_DATA_BTM_GRAY_B) == false)
							continue;
					}
					else if (a == FR_Find_RGB_TopG)
					{
						if (sInspData.UseData(FR_HIT_AREA_DATA_BTM_GRAY_G) == false)
							continue;
					}
					else if (a == FR_Find_RGB_TopR)
					{
						if (sInspData.UseData(FR_HIT_AREA_DATA_BTM_GRAY_R) == false)
							continue;
					}

					imgTemp.setTo(0);
					GRAY_BIN(nGrayMin, nGrayMax, nGrayRange, imgRGB[a], imgTemp);
					cv::bitwise_and(imgTemp, imgBin, imgBin);
				}

				if (bBTM)
				{
					imgTemp.setTo(0);
					imgTemp(roiM1).setTo(255);
					cv::bitwise_xor(imgTemp, imgBin, imgBin);
					cv::bitwise_and(imgBTM_BLOB, imgBin, imgBin);
				}

				nA_T = SET_MODULE_AREA(imgBin, nFovW, nFovH, dMinPer, poPoly, bBTM);
				if (nA_T >= nAREA_CNT_MIN && nA_T <= ZMPOLYPOINT_ULTRA_CNTS)
					return nA_T;
			}
		}
	}

	if (sInspData.UseData(FR_HIT_AREA_DATA_BTM))
	{
		imgBin.setTo(0);
		imgBin(roiM1).setTo(255);
		cv::bitwise_and(imgBTM_BLOB, imgBin, imgBin);
		nA_T = SET_MODULE_AREA(imgBin, nFovW, nFovH, dMinPer, poPoly, true);
		if (nA_T >= nAREA_CNT_MIN && nA_T <= ZMPOLYPOINT_ULTRA_CNTS)
			return nA_T;
	}

	return -1;
}
int FOREIGN_INSP::SET_MODULE_AREA(cv::Mat imgBin, int nFovW, int nFovH, double dMinPer, POINT* poPoly, bool bBTM)
{
	int nAREA_CNT_MIN = 3;
	int nWidth = imgBin.cols;
	int nHeight = imgBin.rows;
	std::vector<cv::Point> vec;
	cv::Mat imgRst(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	if (bBTM)
		vec = FIND_MODULE_AREA_R(imgBin, nFovW, nFovH, dMinPer);
	else
		vec = FIND_MODULE_AREA(imgBin, imgRst, nFovW, nFovH, dMinPer);

	int nA_T = vec.size();
	if (nA_T < nAREA_CNT_MIN)
		return nA_T;

	cv::Mat imgTemp(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	std::vector<std::vector<cv::Point>> allcontour;
	allcontour.push_back(vec);
	cv::drawContours(imgTemp, allcontour, 0, cv::Scalar(255), cv::FILLED);

	if (nA_T < ZMPOLYPOINT_ULTRA_CNTS)
	{
		int nGAP = ZMPOLYPOINT_ULTRA_CNTS / nA_T;
		int nDIV = ZMPOLYPOINT_ULTRA_CNTS % nA_T;
		int nIDX = 0;
		bool bOUT = false;
		for (int a = 0; a < nA_T; a++)
		{
			float fX = vec[a].x;
			float fY = vec[a].y;
			for (int b = 0; b < nGAP; b++)
			{
				if (nIDX >= ZMPOLYPOINT_ULTRA_CNTS)
				{
					bOUT = true;
					break;
				}

				poPoly[nIDX].x = fX;
				poPoly[nIDX].y = fY;
				nIDX++;
			}
			if (a < nDIV)
			{
				if (nIDX >= ZMPOLYPOINT_ULTRA_CNTS)
				{
					bOUT = true;
					break;
				}

				poPoly[nIDX].x = fX;
				poPoly[nIDX].y = fY;
				nIDX++;
			}
		}
		if (bOUT == false && nIDX == ZMPOLYPOINT_ULTRA_CNTS)
			return nA_T;
	}
	else if (nA_T == ZMPOLYPOINT_ULTRA_CNTS)
	{
		for (int a = 0; a < ZMPOLYPOINT_ULTRA_CNTS; a++)
		{
			poPoly[a].x = vec[a].x;
			poPoly[a].y = vec[a].y;
		}
		return nA_T;
	}
	return 0;
}
std::vector<cv::Point> FOREIGN_INSP::FIND_MODULE_AREA(cv::Mat imgBin, cv::Mat imgRst, int nFovW, int nFovH, double dMinPer)
{
	std::vector<cv::Point> vec;
	int nCnt = cv::countNonZero(imgBin);
	if (nCnt <= 0)
		return vec;

	int nWidth = imgBin.cols;
	int nHeight = imgBin.rows;
	cv::Rect roi = cv::boundingRect(imgBin);
	int nAREA_CNT_MIN = 3;
	cv::Mat imgMAX(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	std::vector<cv::Point> vecMAX = BIN_MAX_AREA(imgBin, imgMAX, nFovW, nFovH, dMinPer);
	if (vecMAX.size() < nAREA_CNT_MIN)
		return vec;

	cv::Mat imgXOR(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgXOR2(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTemp(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	imgTemp(roi).setTo(255);
	cv::bitwise_and(imgMAX, imgBin, imgRst);
	cv::bitwise_xor(imgTemp, imgRst, imgXOR);
	cv::bitwise_and(imgMAX, imgXOR, imgXOR2);

	cv::Mat imgXOR_BLOB(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgXOR_BLOB2(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4));
	cv::erode(imgXOR, imgXOR_BLOB, kernel);
	cv::erode(imgXOR2, imgXOR_BLOB2, kernel);

	for (int a = 0; a < 4; a++)
	{
		if (a == 0)
			vec = FIND_MODULE_AREA_R(imgXOR_BLOB2, nFovW, nFovH, dMinPer);
		else if (a == 1)
			vec = FIND_MODULE_AREA_R(imgXOR_BLOB, nFovW, nFovH, dMinPer);
		else if (a == 2)
			vec = FIND_MODULE_AREA_R(imgXOR2, nFovW, nFovH, dMinPer);
		else if (a == 3)
			vec = FIND_MODULE_AREA_R(imgXOR, nFovW, nFovH, dMinPer);

		if (vec.size() > 0)
			return vec;
	}
	return vec;
}
std::vector<cv::Point> FOREIGN_INSP::FIND_MODULE_AREA_R(cv::Mat imgORG, int nFovW, int nFovH, double dMinPer)
{
	std::vector<cv::Point> vec;
	int nCnt = cv::countNonZero(imgORG);
	if (nCnt <= 0)
		return vec;

	int nWidth = imgORG.cols;
	int nHeight = imgORG.rows;
	int nAREA_CNT_MIN = 3;
	cv::Mat imgMAX(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTemp(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	std::vector<cv::Point> vecMAX = BIN_MAX_AREA(imgORG, imgMAX, nFovW, nFovH, dMinPer);
	if (vecMAX.size() < nAREA_CNT_MIN)
		return vec;

	double dFovMinW = nFovW * dMinPer;
	double dFovMinH = nFovH * dMinPer;
	cv::Rect roiMax = cv::boundingRect(imgMAX);
	if (roiMax.width == nFovW || roiMax.height == nFovH)
		return vec;
	else if (roiMax.width < dFovMinW || roiMax.height < dFovMinH)
		return vec;

	nCnt = cv::countNonZero(imgMAX);
	double dPer = (double)nCnt / (double)(nFovW * nFovH);
	if (dPer < 0.6)
		return vec;

	int nORG_T = vecMAX.size();
	if (nORG_T < nAREA_CNT_MIN)
		return vec;
	else if (nORG_T <= ZMPOLYPOINT_ULTRA_CNTS)
		return vecMAX;
	else if (vecMAX.size() > ZMPOLYPOINT_ULTRA_CNTS)
	{
		std::vector<cv::Point> vecTemp = BIN_MAX_AREA(imgMAX, imgTemp, nFovW, nFovH, dMinPer, false, true);
		if (vecTemp.size() >= nAREA_CNT_MIN && vecTemp.size() <= ZMPOLYPOINT_ULTRA_CNTS)
			return vecTemp;
		else
		{
			for (int a = 1; a < nORG_T; a++)
			{
				cv::approxPolyDP(vecMAX, vec, a, true);
				if (vec.size() > 0 && vec.size() <= ZMPOLYPOINT_ULTRA_CNTS)
					return vec;
			}
		}
	}
	return vec;
}
void FOREIGN_INSP::GRAY_BIN(int nMin, int nMax, int nRange, cv::Mat imgGray, cv::Mat imgBin)
{
	if (nMin < 0) nMin = 0;
	if (nMax < 0) nMax = 0;
	if (nMin > 255) nMin = 255;
	if (nMax > 255) nMax = 255;
	if (nMin > nMax) nMax = nMin;
	cv::Mat imgMin;
	cv::Mat imgMax;
	switch (nRange)
	{
	case ETypeInspRange::eTypeRangeIn:
		cv::threshold(imgGray, imgMin, nMin, 255, cv::THRESH_BINARY);
		cv::threshold(imgGray, imgMax, nMax, 255, cv::THRESH_BINARY_INV);
		cv::bitwise_and(imgMin, imgMax, imgBin);
		break;
	case ETypeInspRange::eTypeRangeOut:
		cv::threshold(imgGray, imgMin, nMax, 255, cv::THRESH_BINARY);
		cv::threshold(imgGray, imgMax, nMin, 255, cv::THRESH_BINARY_INV);
		cv::bitwise_or(imgMin, imgMax, imgBin);
		break;
	case ETypeInspRange::eTypeRangeLower:
		cv::threshold(imgGray, imgBin, nMax, 255, cv::THRESH_BINARY_INV);
		break;
	default:
		cv::threshold(imgGray, imgBin, nMin, 255, cv::THRESH_BINARY);
		break;
	}
}
std::vector<cv::Point> FOREIGN_INSP::BIN_MAX_AREA(cv::Mat imgBin, cv::Mat imgMAX, int nFovW, int nFovH, double dMinPer, bool bArea, bool bONE)
{
	int nWidth = imgBin.cols;
	int nHeight = imgBin.rows;
	std::vector<cv::Point> vecRst;
	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imgBin, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nTotal = allcontour.size();
	if (nTotal == 0)
		return vecRst;

	int nAREA_CNT_MIN = 3;
	if (bONE)
	{
		if (nTotal != 1)
			return vecRst;
		else if (allcontour[0].size() < nAREA_CNT_MIN)
			return vecRst;

		if (allcontour[0].size() <= ZMPOLYPOINT_ULTRA_CNTS)
		{
			return allcontour[0];
		}
		else
		{
			nTotal = allcontour[0].size();
			for (int a = 1; a < nTotal; a++)
			{
				std::vector<cv::Point> arr;
				cv::approxPolyDP(allcontour[0], arr, a, true);
				if (arr.size() > nAREA_CNT_MIN && arr.size() <= ZMPOLYPOINT_ULTRA_CNTS)
					return arr;
			}
		}
		return vecRst;
	}

	int nMaxAID = -1;
	int nMaxA = 0;
	int nMaxA_W = 0;
	int nMaxA_H = 0;

	int nMaxWID = -1;
	int nMaxW = 0;
	int nMaxW_H = 0;
	int nMaxW_A = 0;

	int nMaxHID = -1;
	int nMaxH = 0;
	int nMaxH_W = 0;
	int nMaxH_A = 0;
	cv::Mat imgTemp(nHeight, nWidth, CV_8UC1, cv::Scalar(0));
	double dFovMinW = nFovW * dMinPer;
	double dFovMinH = nFovH * dMinPer;
	for (int a = 0; a < nTotal; a++)
	{
		cv::Rect roi = cv::boundingRect(allcontour[a]);
		int nArea = cv::contourArea(allcontour[a]);
#if _DEBUG
		imgTemp.setTo(0);
		cv::drawContours(imgTemp, allcontour, a, cv::Scalar(255), cv::FILLED);
#endif
		if (roi.width < dFovMinW || roi.height < dFovMinH)
			continue;

		if (nArea > nMaxA)
		{
			nMaxAID = a;
			nMaxA = nArea;
			nMaxA_W = roi.width;
			nMaxA_H = roi.height;
		}

		if (roi.width > nMaxW)
		{
			nMaxWID = a;
			nMaxW = roi.width;
			nMaxW_H = roi.height;
			nMaxW_A = nArea;
		}

		if (roi.height > nMaxH)
		{
			nMaxHID = a;
			nMaxH = roi.height;
			nMaxH_W = roi.width;
			nMaxH_A = nArea;
		}
	}

	int nMAX = -1;
	if (bArea)
	{
		if (nMaxAID < 0 || nMaxAID >= nTotal || nMaxA <= 0)
			return vecRst;
		else if (allcontour[nMaxAID].size() < nAREA_CNT_MIN)
			return vecRst;
		nMAX = nMaxAID;
	}
	else
	{
		if (nMaxWID < 0 || nMaxWID >= nTotal || nMaxW <= 0 || nMaxW_H <= 0)
			return vecRst;
		else if (nMaxHID < 0 || nMaxHID >= nTotal || nMaxH <= 0 || nMaxH_W <= 0)
			return vecRst;
		else if (nMaxWID != nMaxHID)
			return vecRst;
		nMAX = nMaxWID;
	}
	if (nMAX < 0 || nMAX >= nTotal)
		return vecRst;

	cv::drawContours(imgMAX, allcontour, nMAX, cv::Scalar(255), cv::FILLED);
	return allcontour[nMAX];
}
void FOREIGN_INSP::CompareCIE_BTM(cv::Mat* imgRGB, cv::Mat imgBin, ColorXYInfoForeign vCol, POINTF* arrPo)
{
	if (imgRGB == NULL || arrPo == NULL)
		return;

	int ptPolyNum = POLYGON_BTM_POINT_CNTS;
	int	nPixelCount = 1000;
	int nWidth = imgBin.cols;
	int nHeight = imgBin.rows;
	UCHAR* srcR = imgRGB[FR_Find_RGB_TopR].data;
	UCHAR* srcG = imgRGB[FR_Find_RGB_TopG].data;
	UCHAR* srcB = imgRGB[FR_Find_RGB_TopB].data;

	cv::Mat ImgPoly(nPixelCount, nPixelCount, CV_8UC1, cv::Scalar(0));
	cv::Mat ImgPolyPerpect(nPixelCount, nPixelCount, CV_8UC1, cv::Scalar(0));
	cv::Scalar white(255, 255, 255);
	bool bPolygonAreaCalc = PolygonAreaCalc();
	if (bPolygonAreaCalc)
	{
		FillPolygon(ImgPoly, arrPo, ptPolyNum, white);
		MorErode(ImgPoly, ImgPolyPerpect);
	}

	long long size = nWidth * nHeight;
	cv::Mat	pConvertMat = cv::Mat(3, 3, CV_32FC1);
	pConvertMat.at<float>(0, 0) = 0.490f;     pConvertMat.at<float>(0, 1) = 0.310f;     pConvertMat.at<float>(0, 2) = 0.200f;
	pConvertMat.at<float>(1, 0) = 0.177f;     pConvertMat.at<float>(1, 1) = 0.813f;     pConvertMat.at<float>(1, 2) = 0.011f;
	pConvertMat.at<float>(2, 0) = 0.000f;     pConvertMat.at<float>(2, 1) = 0.010f;     pConvertMat.at<float>(2, 2) = 0.990f;
	for (long long j = 0; j < size; j++)
	{
		imgBin.data[j] = 0;
		if (srcR[j] == 0 && srcG[j] == 0 && srcB[j] == 0)
			continue;

		float r = srcR[j];		r = _limit_value(r, 0.f, 255.f);
		float g = srcG[j];		g = _limit_value(g, 0.f, 255.f);
		float b = srcB[j];		b = _limit_value(b, 0.f, 255.f);
		POINTF pt = GetCIEPoint(&pConvertMat, r, g, b, nPixelCount);
		int nindex = (nPixelCount * pt.y) + pt.x;
		bool bIsWhite = false;
		bool bProc = false;
		if (bPolygonAreaCalc)
		{
			if (ImgPoly.data[nindex] == 255)
			{
				if (ImgPolyPerpect.data[nindex] == 255)
					bIsWhite = true;
				else
					bProc = true;
			}
		}
		else
			bProc = true;

		if (bProc)
			bIsWhite = pvProcPointInPolygon(arrPo, ptPolyNum, pt);

		if (bIsWhite && vCol.m_bUseThreshold)
			bIsWhite = IsBlack(r, g, b, vCol.m_vThresholdType, vCol.m_nMin, vCol.m_nMax);

		imgBin.data[j] = bIsWhite ? 255 : 0;
	}
}
int FOREIGN_INSP::FR_AutoPseudo(float* pfSrc, UCHAR* pucWP, int nW, int nH, double dMin, double dMax, BOOL bSET)
{
	if (pfSrc == NULL || pucWP == NULL || nW <= 100 || nH <= 100 ||
		dMin >= dMax)
		return eMR_FAIL;

#if _DEBUG
	cv::Mat imgWP(nH, nW, CV_8UC3, pucWP);
	cv::Mat img3D(nH, nW, CV_32FC1, pfSrc);
#endif

	for (int y = 0; y < nH; y++)
	{
		for (int x = 0; x < nW; x++)
		{
			long long lf = ((y * nW) + x);
			long long luc = lf * 3;
			if (bSET == FALSE)
			{
				if (pucWP[luc] == 0 && pucWP[luc + 1] == 0 && pucWP[luc + 2] == 0)
					continue;;
			}
			SetRGB(pucWP, luc, pfSrc[lf], dMin, dMax);
		}
	}
	return eMR_SUCCESS;
}
int FOREIGN_INSP::CalcCorrWarpage(ForeignData sInspData, AForeignResultWP &retResult, float* pfArrTact)
{
	int nLine = __LINE__;
	int nRet = eMR_FAIL;
	try
	{
		DWORD st = GetTickCount();
		DWORD stE = st;
		if (sInspData.m_nWP_W <= 0 || sInspData.m_nWP_H <= 0)
			return nRet;
		if (sInspData.m_pDesImg == NULL || sInspData.m_p3D == NULL || sInspData.m_pimgSUB == NULL ||
			pfArrTact == NULL || sInspData.m_pimgWPInspA == NULL)
			return nRet;

		bool bSET_RGB = true;
		bool bCorrectWP_B = m_ForeignData.sWP.UseData(FR_WP_DT_CorrectWP_B);
		if (bCorrectWP_B)
		{
			if (sInspData.m_nContainModuleID != 0)
				bSET_RGB = false;
		}
		else
		{
			if (sInspData.m_nContainModuleID <= 0)
				return nRet;
		}
		if (sInspData.m_nModule <= 0 || sInspData.m_nModule < sInspData.m_nContainModuleID)
			return nRet;
		nLine = __LINE__;

		cv::Mat imgFull_D(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_8UC3, sInspData.m_pDesImg);
		cv::Mat img3D_D(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_32FC1, sInspData.m_p3D);
		cv::Mat imgWPInspA(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_8UC1, sInspData.m_pimgWPInspA);

		int nDivide = m_ForeignData.GetDivide();
		int nL = sInspData.m_rcFOV.left;
		int nR = sInspData.m_rcFOV.right;
		int nT = sInspData.m_rcFOV.top;
		int nB = sInspData.m_rcFOV.bottom;
		if (nL < 0) nL = 0;
		if (nR < 0) nR = 0;
		if (nT < 0) nT = 0;
		if (nB < 0) nB = 0;
		int nW = nR - nL;
		int nH = nB - nT;
		if (nW <= 0 || nW > sInspData.m_nWP_W ||
			nH <= 0 || nH > sInspData.m_nWP_H)
			return nRet;

		int nInspL = sInspData.m_rcFOVMargin.left;
		int nInspR = sInspData.m_rcFOVMargin.right;
		int nInspT = sInspData.m_rcFOVMargin.top;
		int nInspB = sInspData.m_rcFOVMargin.bottom;
		if (nL > nInspL) nInspL = nL;
		if (nR < nInspR) nInspR = nR;
		if (nT > nInspT) nInspT = nT;
		if (nB < nInspB) nInspB = nB;
		int nInspW = nInspR - nInspL;
		int nInspH = nInspB - nInspT;
		if (nInspW <= 0 || nInspW > sInspData.m_nWP_W ||
			nInspH <= 0 || nInspH > sInspData.m_nWP_H)
			return nRet;
		nLine = __LINE__;

		bool bSave = m_ForeignData.UseData(FR_DATA_DebugData) || m_ForeignData.sWP.UseData(FR_WP_DT_CorrectWPSave);
		bool bNGSave = m_ForeignData.UseData(FR_DATA_DebugData2);
		bool bHole = m_ForeignData.sWP.UseData(FR_WP_DT_Not_Hole);
		bool bBTC = m_ForeignData.sWP.UseData(FR_WP_DT_BTC);
		bool bAuto = m_ForeignData.sWP.UseData(FR_WP_DT_CorrectWPAuto);
		int nModuleCnt = 0;
		float fPseudoColorMin = m_ForeignData.sWP.m_fArr[FR_WP_F_PseudoColorMin];
		float fPseudoColorMax = m_ForeignData.sWP.m_fArr[FR_WP_F_PseudoColorMax];
		cv::Mat imgInspArea(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgInsp3D(nH, nW, CV_32FC1, cv::Scalar(0));
		nLine = __LINE__;

		cv::Mat imgInspM;
		if (bSave || bNGSave || bAuto)
			imgInspM = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));

		cv::Mat imgInspSub;
		cv::Mat imgInspEXCP;
		cv::Mat imgInspSticker;
		cv::Mat imgInspHOLE;
		cv::Mat imgInspBOTM;
		if (bSave || bNGSave)
		{
			imgInspSub = cv::Mat(nH, nW, CV_32FC1, cv::Scalar(0));
			imgInspEXCP = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
			imgInspSticker = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
			imgInspHOLE = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
			imgInspBOTM = cv::Mat(nH, nW, CV_32FC1, cv::Scalar(0));
		}
		nLine = __LINE__;
		pfArrTact[WP_TACT_SET] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();

		cv::Rect roi(nL, nT, nW, nH);
		img3D_D(roi).copyTo(imgInsp3D);
		pfArrTact[WP_TACT_CLIP] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();

		nInspL = nW;
		nInspR = 0;
		nInspT = nH;
		nInspB = 0;
		for (int y = nT; y < nB && y < sInspData.m_nWP_H; y++)
		{
			if (y < 0) continue;
			for (int x = nL; x < nR && x < sInspData.m_nWP_W; x++)
			{
				if (x < 0) continue;
				int nY = (y - nT);
				int nX = (x - nL);
				if (nX < 0 || nX >= nW || nY < 0 || nY >= nH)
					continue;

				long long nIndex = (y * sInspData.m_nWP_W) + x;
				int nSub = sInspData.m_pimgSUB[nIndex];
				if (nSub <= 0)
					continue;

				for (int nModuleID = sInspData.m_nModule - 1; nModuleID > 0; nModuleID--)
				{
					int nType = FR_SUB_MODU * (nModuleID + 1);
					if ((nSub & nType) != nType)
						continue;

					if (bCorrectWP_B)
					{
						if (sInspData.m_nContainModuleID != 0 &&
							nModuleID != sInspData.m_nContainModuleID)
							break;
					}
					else
					{
						if (nModuleID != sInspData.m_nContainModuleID)
							break;
					}

					if (bAuto == false && bSET_RGB)
					{
						long long nDstId = nIndex * 3;
						SetRGB(sInspData.m_pDesImg, nDstId, sInspData.m_p3D[nIndex], fPseudoColorMin, fPseudoColorMax);
					}

					long long nIDX = (nY * nW) + nX;
					if (bSave || bNGSave || bAuto)
						imgInspM.data[nIDX] = 255;
					if (sInspData.m_pimgWPInspA[nIndex] == 255)
					{
						imgInspArea.data[nIDX] = 255;
						nModuleCnt++;
						if (nInspL > nX) nInspL = nX;
						if (nInspR < nX) nInspR = nX;
						if (nInspT > nY) nInspT = nY;
						if (nInspB < nY) nInspB = nY;
					}
					if (bSave || bNGSave)
					{
						imgInspSub.ptr<float>(nY)[nX] = nSub;
						if ((nSub & FR_SUB_EXCP) == FR_SUB_EXCP) imgInspEXCP.data[nIDX] = 255;
						if ((nSub & FR_SUB_Sticker) == FR_SUB_Sticker) imgInspSticker.data[nIDX] = 255;
						if ((nSub & FR_SUB_HOLE) == FR_SUB_HOLE) imgInspHOLE.data[nIDX] = 255;
						if ((nSub & FR_SUB_BOTM) == FR_SUB_BOTM) imgInspBOTM.data[nIDX] = 255;
					}
					break;
				}
			}
		}
		nLine = __LINE__;
		pfArrTact[WP_TACT_RGB] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();
		if (nModuleCnt <= INSP_MIN_SIZE)
			return nRet;

		if (nInspL < 0) nInspL = 0;
		if (nInspR < 0) nInspR = 0;
		if (nInspT < 0) nInspT = 0;
		if (nInspB < 0) nInspB = 0;
		if (nInspL >= nW) nInspL = nW - 1;
		if (nInspR > nW) nInspR = nW;
		if (nInspT >= nH) nInspT = nH - 1;
		if (nInspB > nH) nInspB = nH;
		nInspW = nInspR - nInspL;
		nInspH = nInspB - nInspT;
		if (nInspL >= nInspR + INSP_MIN_SIZE || nInspT >= nInspB + INSP_MIN_SIZE ||
			nInspW <= INSP_MIN_SIZE || nInspH <= INSP_MIN_SIZE)
			return nRet;

		cv::Rect rcROI(nInspL, nInspT, nInspW, nInspH);
		InspFovForeignResult *ForeignResult = g_pMManager->pem_new<InspFovForeignResult>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		nLine = __LINE__;

		int nMaxRst = 2000;
		ForeignResult->m_bOK = TRUE;
		ForeignResult->m_stForeignWP = g_pMManager->pem_new<AForeignResultWP>(true, nMaxRst, (PCHAR)__FUNCTION__, __LINE__);
		memset(ForeignResult->m_stForeignWP, 0, sizeof(AForeignResultWP) * nMaxRst);
		ForeignResult->m_nCountDefect = 0;
		ForeignResult->m_nCountDefectWP = 0;
		int nTotal = (nMaxRst > sInspData.m_nModule * 2) ? sInspData.m_nModule * 2 : nMaxRst;
		for (int a = 0; a < nTotal; a++)
		{
			ForeignResult->m_stForeignWP[a].m_fHeightMin = 9999.9;
			ForeignResult->m_stForeignWP[a].m_fHeightMax = -9999.9;
			ForeignResult->m_stForeignWP[a].m_fHeightMinAM = 9999.9;
			ForeignResult->m_stForeignWP[a].m_fHeightMaxAM = -9999.9;
		}
		nLine = __LINE__;
		pfArrTact[WP_TACT_RST_SET] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();

		int nAM = 0;
		bool bRet = InspWarpage(m_ForeignData, imgInsp3D.ptr<float>(), imgInspArea, nW, nH, sInspData.m_nContainModuleID, nAM, sInspData, ForeignResult, rcROI);
		nLine = __LINE__;
		pfArrTact[WP_TACT_INSP_WP] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();

		nLine = __LINE__;
		if (ForeignResult)
		{
			bool bCorrectWPAbsolute = m_ForeignData.sWP.UseData(FR_WP_DT_CorrectWPAbsolute);
			for (int a = 0; a < ForeignResult->m_nCountDefectWP; a++)
			{
				if (ForeignResult->m_stForeignWP[a].nID != sInspData.m_nContainModuleID)
					continue;
				if (ForeignResult->m_stForeignWP[a].m_byWP != 1 ||
					ForeignResult->m_stForeignWP[a].m_byAM != 0)
					continue;
				ForeignResult->m_stForeignWP[a].Clone(retResult);

				if (bCorrectWPAbsolute)
				{
					float fCX = nW / 2.0f;
					float fCY = nH / 2.0f;
					float fCXMin = (retResult.m_byWP == 1) ? retResult.m_fCXMin : retResult.m_fCXMinAM;
					float fCYMin = (retResult.m_byWP == 1) ? retResult.m_fCYMin : retResult.m_fCYMinAM;
					float fCXMax = (retResult.m_byWP == 1) ? retResult.m_fCXMax : retResult.m_fCXMaxAM;
					float fCYMax = (retResult.m_byWP == 1) ? retResult.m_fCYMax : retResult.m_fCYMaxAM;
					float fMin = sqrt(pow(fCX - fCXMin, 2) + pow(fCY - fCYMin, 2));
					float fMax = sqrt(pow(fCX - fCXMax, 2) + pow(fCY - fCYMax, 2));
					if (fMax > fMin)
					{
						retResult.m_fHeightMax = 0;
						retResult.m_fHeightMaxAM = 0;
						retResult.m_fCXMax = retResult.m_fCXMin;
						retResult.m_fCYMax = retResult.m_fCYMin;
						retResult.m_fCXMaxAM = retResult.m_fCXMinAM;
						retResult.m_fCYMaxAM = retResult.m_fCYMinAM;
					}
					else
					{
						retResult.m_fHeightMin = 0;
						retResult.m_fHeightMinAM = 0;
						retResult.m_fCXMin = retResult.m_fCXMax;
						retResult.m_fCYMin = retResult.m_fCYMax;
						retResult.m_fCXMinAM = retResult.m_fCXMaxAM;
						retResult.m_fCYMinAM = retResult.m_fCYMaxAM;
					}
				}

				if (retResult.m_byWP == 1)
				{
					retResult.m_fCXMin += nL;
					retResult.m_fCYMin += nT;
					retResult.m_fCXMax += nL;
					retResult.m_fCYMax += nT;

					retResult.m_fCXMin *= nDivide;
					retResult.m_fCYMin *= nDivide;
					retResult.m_fCXMax *= nDivide;
					retResult.m_fCYMax *= nDivide;

					retResult.m_fCXMin *= m_resolX;
					retResult.m_fCYMin *= m_resolY;
					retResult.m_fCXMax *= m_resolX;
					retResult.m_fCYMax *= m_resolY;
				}
				else if (retResult.m_byAM == 1)
				{
					retResult.m_fCXMinAM += nL;
					retResult.m_fCYMinAM += nT;
					retResult.m_fCXMaxAM += nL;
					retResult.m_fCYMaxAM += nT;

					retResult.m_fCXMinAM *= nDivide;
					retResult.m_fCYMinAM *= nDivide;
					retResult.m_fCXMaxAM *= nDivide;
					retResult.m_fCYMaxAM *= nDivide;

					retResult.m_fCXMinAM *= m_resolX;
					retResult.m_fCYMinAM *= m_resolY;
					retResult.m_fCXMaxAM *= m_resolX;
					retResult.m_fCYMaxAM *= m_resolY;
				}

				DWORD stRst = GetTickCount();
				if (bAuto && retResult.m_byWP == 1 && bSET_RGB)
				{
					fPseudoColorMin = retResult.m_fHeightMin;
					fPseudoColorMax = retResult.m_fHeightMax;
					for (int y = nT; y < nB && y < sInspData.m_nWP_H; y++)
					{
						if (y < 0) continue;
						for (int x = nL; x < nR && x < sInspData.m_nWP_W; x++)
						{
							if (x < 0) continue;
							int nY = (y - nT);
							int nX = (x - nL);
							if (nX < 0 || nX >= nW || nY < 0 || nY >= nH)
								continue;

							long long nIDX = (nY * nW) + nX;
							if (imgInspM.data[nIDX] == 0)
								continue;

							long long nIndex = (y * sInspData.m_nWP_W) + x;
							long long nDstId = nIndex * 3;
							SetRGB(sInspData.m_pDesImg, nDstId, sInspData.m_p3D[nIndex], fPseudoColorMin, fPseudoColorMax);
						}
					}
				}
				pfArrTact[WP_TACT_AUTO] += ((GetTickCount() - stRst) / 1000.0f);
				nRet = eMR_SUCCESS;
				break;
			}

			if (ForeignResult->m_stForeign)
				g_pMManager->pem_delete(ForeignResult->m_stForeign, true);
			ForeignResult->m_stForeign = NULL;
			if (ForeignResult->m_stForeignWP)
				g_pMManager->pem_delete(ForeignResult->m_stForeignWP, true);
			ForeignResult->m_stForeignWP = NULL;
			g_pMManager->pem_delete(ForeignResult, false);
			ForeignResult = NULL;
		}
		nLine = __LINE__;
		pfArrTact[WP_TACT_RST] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();
		nLine = __LINE__;

		if (bSave || (bNGSave && bRet == false))
		{
			CString dir = _T("");
			dir.Format(_T("D:\\Eagle3D_data\\ForeignDebugData\\Module%d"), sInspData.m_nContainModuleID);
			SaveReleaseWorkImg(sInspData.m_pDesImg, sInspData.m_nWP_W, sInspData.m_nWP_H, _T("WP_FULL_RST.bmp"), dir, 3);
			SaveReleaseWorkImg(imgInspArea.data, nW, nH, _T("WP_FULL_RST_INSP.bmp"), dir);
			SaveReleaseWorkImg(imgInspM.data, nW, nH, _T("WP_FULL_RST_M.bmp"), dir);
			SaveReleaseWorkImg(imgInspEXCP.data, nW, nH, _T("WP_FULL_RST_EXCP.bmp"), dir);
			SaveReleaseWorkImg(imgInspSticker.data, nW, nH, _T("WP_FULL_RST_Sticker.bmp"), dir);
			SaveReleaseWorkImg(imgInspHOLE.data, nW, nH, _T("WP_FULL_RST_HOLE.bmp"), dir);
			SaveReleaseWorkImg(imgInspBOTM.data, nW, nH, _T("WP_FULL_RST_BOTM.bmp"), dir);

			CString sPath = _T("");
			sPath.Format(_T("%s\\WP_FULL_RST_3D.ptr"), dir);
			SavePTR(sPath, imgInsp3D);

			sPath.Format(_T("%s\\WP_FULL_SUB_RST_3D.ptr"), dir);
			SavePTR(sPath, imgInspSub);
		}
		DWORD ed = GetTickCount();
		pfArrTact[WP_TACT_END] += ((ed - st) / 1000.0f);
		pfArrTact[WP_TACT_TOTAL] += ((ed - stE) / 1000.0f);
	}
	catch (...)
	{
		CString sLog = _T("");
		sLog.Format(_T("Line : %d Pass!!!"), nLine);
		AddLOG(_T("[ERROR]"), _T("[FOREIGN_INSP]"), _T("[CalcCorrWarpage]"), sLog);
		throw nLine;
	}
	return nRet;
}
void FOREIGN_INSP::SaveWarpageSUB(ForeignData sInspData, CString dir)
{
	int nLine = __LINE__;
	try
	{
		if (sInspData.m_nWP_W <= 0 || sInspData.m_nWP_H <= 0)
			return;
		else if (sInspData.m_pimgSUB == NULL || sInspData.m_pimgWP == NULL)
			return;
		else if (sInspData.m_pimgTopR == NULL || sInspData.m_pimgTopG == NULL || sInspData.m_pimgTopB == NULL)
			return;
		nLine = __LINE__;

		int nL = sInspData.m_rcFOV.left;
		int nR = sInspData.m_rcFOV.right;
		int nT = sInspData.m_rcFOV.top;
		int nB = sInspData.m_rcFOV.bottom;
		int nW = nR - nL;
		int nH = nB - nT;
		int nOrgW = sInspData.m_nWP_W;
		int nOrgH = sInspData.m_nWP_H;
		if (nL < 0) nL = 0;
		if (nT < 0) nT = 0;
		if (nL + nW > nOrgW) nW = nOrgW - nL;
		if (nT + nH > nOrgH) nH = nOrgH - nT;
		if (nW <= 0 || nH <= 0)
			return;
		else if (nL >= nOrgW || nT >= nOrgH)
			return;
		else if (nW > nOrgW || nH > nOrgH)
			return;

		cv::Rect roi(nL, nT, nW, nH);
		cv::Mat imgSUB_O(nOrgH, nOrgW, CV_32FC1, sInspData.m_pimgSUB);
		cv::Mat imgWP3_O(nOrgH, nOrgW, CV_32FC1, sInspData.m_pimgWP);
		cv::Mat imgTOR_O(nOrgH, nOrgW, CV_8UC1, sInspData.m_pimgTopR);
		cv::Mat imgTOG_O(nOrgH, nOrgW, CV_8UC1, sInspData.m_pimgTopG);
		cv::Mat imgTOB_O(nOrgH, nOrgW, CV_8UC1, sInspData.m_pimgTopB);

		cv::Mat imgSUB(nH, nW, CV_32FC1, cv::Scalar(0));
		cv::Mat imgWP3(nH, nW, CV_32FC1, cv::Scalar(0));
		cv::Mat imgTOR(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgTOG(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgTOB(nH, nW, CV_8UC1, cv::Scalar(0));
		nLine = __LINE__;

		imgSUB_O(roi).copyTo(imgSUB);
		imgWP3_O(roi).copyTo(imgWP3);
		imgTOR_O(roi).copyTo(imgTOR);
		imgTOG_O(roi).copyTo(imgTOG);
		imgTOB_O(roi).copyTo(imgTOB);
		nLine = __LINE__;

		CString sPath = _T("");
		sPath.Format(_T("%s\\WP_FULL_SUB_RST_3D.ptr"), dir);
		SavePTR(sPath, imgSUB);

		sPath.Format(_T("%s\\WP_FULL_RST_3D.ptr"), dir);
		SavePTR(sPath, imgWP3);

		sPath.Format(_T("%s\\WP_FULL_RST_TR.bmp"), dir);
		cv::imwrite(std::string(CT2A(sPath)), imgTOR);

		sPath.Format(_T("%s\\WP_FULL_RST_TG.bmp"), dir);
		cv::imwrite(std::string(CT2A(sPath)), imgTOG);

		sPath.Format(_T("%s\\WP_FULL_RST_TB.bmp"), dir);
		cv::imwrite(std::string(CT2A(sPath)), imgTOB);
	}
	catch (...)
	{
		CString sLog = _T("");
		sLog.Format(_T("Line : %d Pass!!!"), nLine);
		AddLOG(_T("[ERROR]"), _T("[FOREIGN_INSP]"), _T("[SaveWarpageSUB]"), sLog);
		throw nLine;
	}
}
void FOREIGN_INSP::GetWP3D(CString sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer)
{
	int nMinW = 100;
	int nMinH = 100;
	if (retFullImg == NULL || nWidth <= nMinW * 2 || nHeight <= nMinH * 2 )
		return;

	SaveWorkImg(retFullImg, nWidth, nHeight, _T("FullImage.bmp"), 3);

	if (rcClip.left < 0) rcClip.left = 0;
	if (rcClip.right < 0) rcClip.right = 0;
	if (rcClip.top < 0) rcClip.top = 0;
	if (rcClip.bottom < 0) rcClip.bottom = 0;
	if (rcClip.left > nWidth) rcClip.left = nWidth;
	if (rcClip.right > nWidth) rcClip.right = nWidth;
	if (rcClip.top > nHeight) rcClip.top = nHeight;
	if (rcClip.bottom > nHeight) rcClip.bottom = nHeight;

	int nImgW = nWidth;
	int nImgH = nHeight;
	int nL = rcClip.left;
	int nR = rcClip.right;
	int nT = rcClip.top;
	int nB = rcClip.bottom;
	int nW = nR - nL;
	int nH = nB - nT;
	if (nL < 0 || nR <= nL || nT < 0 || nB <= nT || nR <= 0 || nB <= 0)
		return;
	if (nL >= nImgW - 1 || nR > nImgW || nT >= nImgH - 1 || nB > nImgH)
		return;
	if (nW < nMinW || nH < nMinH || nW > nImgW || nH > nImgH)
		return;

	cv::Mat imgORG(nHeight, nWidth, CV_8UC3, retFullImg);
	cv::Mat imgSrcRGB[3];
	cv::Mat img3D(nH, nW, CV_32FC1, cv::Scalar(0));
	cv::split(imgORG, imgSrcRGB);

	double dDivZ = ((dMax * 1.15) - dMin) / (double)CR_MAP_SIZE;
	for (int y = nT; y < nB; y++)
	{
		for (int x = nL; x < nR; x++)
		{
			UCHAR ucB = imgSrcRGB[0].ptr<UCHAR>(y)[x];
			UCHAR ucG = imgSrcRGB[1].ptr<UCHAR>(y)[x];
			UCHAR ucR = imgSrcRGB[2].ptr<UCHAR>(y)[x];

			float fH = GetRGBH(ucR, ucG, ucB, dMin, dMax);
			int nX = x - nL;
			int nY = y - nT;
			if (fH == 0 && (ucR > 0 || ucG > 0 || ucB > 0))
			{
				for (int y2 = -1; y2 < 1; y2++)
				{
					for (int x2 = -1; x2 < 1; x2++)
					{
						if (x2 == 0 && y2 == 0)
							continue;

						int nX2 = (nX + x2);
						int nY2 = (nY + y2);
						if (nX2 < 0 || nX2 >= nW || nY2 < 0 || nY2 >= nH)
							continue;

						fH = img3D.ptr<float>(nY2)[nX2];
						if (fH != 0)
							break;
					}
					if (fH != 0)
						break;
				}
			}

			img3D.ptr<float>(nY)[nX] = fH;
		}
	}

	cv::Mat img3DTemp;
	if (100 > dPer)
	{
		if (dPer < 10) dPer = 10;
		nMinW = 500;
		nMinH = 500;
		nImgW = nW;
		nImgH = nH;
		for (int nDiv = 0; nDiv < 10; nDiv++)
		{
			if (nDiv > 0)
				dPer += (5.0 * nDiv);
			if (dPer > 100)
				break;

			int nImgW_Temp = nW * dPer / 100.0;
			int nImgH_Temp = nH * dPer / 100.0;
			if (nImgW_Temp < nMinW || nImgH_Temp < nMinH)
				continue;

			nImgW = nImgW_Temp;
			nImgH = nImgH_Temp;
			break;
		}
		cv::resize(img3D, img3DTemp, cv::Size(nImgW, nImgH));
	}
	else
		img3DTemp = img3D;

	if (img3DTemp.empty())
		return;

	SavePTR(sPath, img3DTemp);
}
void FOREIGN_INSP::GetWP3DM(CString sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, IN UCHAR* retFullImgM, IN int nWidthM, IN int nHeightM, RECT rcClip, double dMin, double dMax, double dPer, double dFacX, double dFacY)
{
	int nMinW = 100;
	int nMinH = 100;
	if (retFullImg == NULL || nWidth <= nMinW * 2 || nHeight <= nMinH * 2)
		return;
	if (retFullImgM == NULL || nWidthM <= nMinW * 2 || nHeightM <= nMinH * 2)
		return;

	if (rcClip.left < 0) rcClip.left = 0;
	if (rcClip.right < 0) rcClip.right = 0;
	if (rcClip.top < 0) rcClip.top = 0;
	if (rcClip.bottom < 0) rcClip.bottom = 0;
	if (rcClip.left > nWidth) rcClip.left = nWidth;
	if (rcClip.right > nWidth) rcClip.right = nWidth;
	if (rcClip.top > nHeight) rcClip.top = nHeight;
	if (rcClip.bottom > nHeight) rcClip.bottom = nHeight;

	int nImgW = nWidth;
	int nImgH = nHeight;
	int nL = rcClip.left;
	int nR = rcClip.right;
	int nT = rcClip.top;
	int nB = rcClip.bottom;
	int nW = nR - nL;
	int nH = nB - nT;
	if (nL < 0 || nR <= nL || nT < 0 || nB <= nT || nR <= 0 || nB <= 0)
		return;
	if (nL >= nImgW - 1 || nR > nImgW || nT >= nImgH - 1 || nB > nImgH)
		return;
	if (nW < nMinW || nH < nMinH || nW > nImgW || nH > nImgH)
		return;

	cv::Mat imgORG(nHeight, nWidth, CV_8UC3, retFullImg);
	cv::Mat imgSrcRGB[3];
	cv::split(imgORG, imgSrcRGB);

	cv::Mat imgFullSrc(nHeightM, nWidthM, CV_8UC3, retFullImgM);
	cv::Mat imgFullRst;
	if (nWidth == nWidthM && nHeight == nHeightM)
		imgFullRst = imgFullSrc;
	else
		cv::resize(imgFullSrc, imgFullRst, cv::Size(nWidth, nHeight));
	cv::Mat imgFullRstRGB[3];
	cv::split(imgFullRst, imgFullRstRGB);
	if (imgFullRstRGB[0].empty() || imgFullRstRGB[1].empty() || imgFullRstRGB[2].empty())
		return;

	cv::Mat img3D(nH, nW, CV_32FC1, cv::Scalar(0));
	cv::Mat imgTR(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTG(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTB(nH, nW, CV_8UC1, cv::Scalar(0));
	double dDivZ = ((dMax * 1.15) - dMin) / (double)CR_MAP_SIZE;
	for (int y = nT; y < nB; y++)
	{
		for (int x = nL; x < nR; x++)
		{
			UCHAR ucB = imgSrcRGB[0].ptr<UCHAR>(y)[x];
			UCHAR ucG = imgSrcRGB[1].ptr<UCHAR>(y)[x];
			UCHAR ucR = imgSrcRGB[2].ptr<UCHAR>(y)[x];

			float fH = GetRGBH(ucR, ucG, ucB, dMin, dMax);
			int nX = x - nL;
			int nY = y - nT;
			if (fH == 0 && (ucR > 0 || ucG > 0 || ucB > 0))
			{
				for (int y2 = -1; y2 < 1; y2++)
				{
					for (int x2 = -1; x2 < 1; x2++)
					{
						if (x2 == 0 && y2 == 0)
							continue;

						int nX2 = (nX + x2);
						int nY2 = (nY + y2);
						if (nX2 < 0 || nX2 >= nW || nY2 < 0 || nY2 >= nH)
							continue;

						fH = img3D.ptr<float>(nY2)[nX2];
						if (fH != 0)
							break;
					}
					if (fH != 0)
						break;
				}
			}

			img3D.ptr<float>(nY)[nX] = fH;
			imgTB.ptr<UCHAR>(nY)[nX] = imgFullRstRGB[0].ptr<UCHAR>(y)[x];
			imgTG.ptr<UCHAR>(nY)[nX] = imgFullRstRGB[1].ptr<UCHAR>(y)[x];
			imgTR.ptr<UCHAR>(nY)[nX] = imgFullRstRGB[2].ptr<UCHAR>(y)[x];
		}
	}

	cv::Mat img3DTemp;
	cv::Mat imgTRTemp;
	cv::Mat imgTGTemp;
	cv::Mat imgTBTemp;
	if (100 > dPer)
	{
		if (dPer < 10) dPer = 10;
		nMinW = 500;
		nMinH = 500;
		nImgW = nW;
		nImgH = nH;
		for (int nDiv = 0; nDiv < 10; nDiv++)
		{
			if (nDiv > 0)
				dPer += (5.0 * nDiv);
			if (dPer > 100)
				break;

			int nImgW_Temp = nW * dPer / 100.0;
			int nImgH_Temp = nH * dPer / 100.0;
			if (nImgW_Temp < nMinW || nImgH_Temp < nMinH)
				continue;

			nImgW = nImgW_Temp;
			nImgH = nImgH_Temp;
			break;
		}
		cv::resize(img3D, img3DTemp, cv::Size(nImgW, nImgH));
		cv::resize(imgTR, imgTRTemp, cv::Size(nImgW, nImgH));
		cv::resize(imgTG, imgTGTemp, cv::Size(nImgW, nImgH));
		cv::resize(imgTB, imgTBTemp, cv::Size(nImgW, nImgH));
	}
	else
	{
		img3DTemp = img3D;
		imgTRTemp = imgTR;
		imgTGTemp = imgTG;
		imgTBTemp = imgTB;
	}

	if (img3DTemp.empty() || imgTRTemp.empty() || imgTGTemp.empty() || imgTBTemp.empty())
		return;

	cv::Mat img32(img3DTemp.rows, img3DTemp.cols, CV_32FC1, cv::Scalar(0));
	UCHAR* pDest24 = NULL;
	RECT roi;
	roi.left = 0;
	roi.top = 0;
	roi.right = img3DTemp.cols;
	roi.bottom = img3DTemp.rows;
	alpf_make_img24n32f(roi, img3DTemp.cols, imgTRTemp.data, imgTGTemp.data, imgTBTemp.data, 1, 1, 1, img3DTemp.cols, img32.ptr<float>(), pDest24);

	HEADER_PTT hd;
	hd.uiNumRow = img3DTemp.cols;
	hd.uiNumCol = img3DTemp.rows;
	hd.fNumRow = (float)img3DTemp.cols;
	hd.fNumCol = (float)img3DTemp.rows;
	hd.pixelResX = (float)_mm2micron(m_resolX) * dFacX;
	hd.pixelResY = (float)_mm2micron(m_resolY) * dFacY;
	hd.zResolX = (float)_mm2micron(m_resolX) * dFacX;
	hd.zResolY = (float)_mm2micron(m_resolY) * dFacY;
	alpf_save_ptt(sPath, &hd, img3DTemp.ptr<float>(), img32.ptr<float>());
}
void FOREIGN_INSP::GetWP3DPTT(wchar_t* sPath, IN float* retFullImg, IN UCHAR* ptrTR, IN UCHAR* ptrTG, IN UCHAR* ptrTB, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer, double dFacW, double dFacH)
{
	int nMinW = 150;
	int nMinH = 150;
	if (retFullImg == NULL || ptrTR == NULL || ptrTG == NULL || ptrTB == NULL)
		return;
	if (nWidth <= nMinW * 2 || nHeight <= nMinH * 2)
		return;

	if (rcClip.left < 0) rcClip.left = 0;
	if (rcClip.right < 0) rcClip.right = 0;
	if (rcClip.top < 0) rcClip.top = 0;
	if (rcClip.bottom < 0) rcClip.bottom = 0;
	if (rcClip.left > nWidth) rcClip.left = nWidth;
	if (rcClip.right > nWidth) rcClip.right = nWidth;
	if (rcClip.top > nHeight) rcClip.top = nHeight;
	if (rcClip.bottom > nHeight) rcClip.bottom = nHeight;

	int nImgW = nWidth;
	int nImgH = nHeight;
	int nL = rcClip.left;
	int nR = rcClip.right;
	int nT = rcClip.top;
	int nB = rcClip.bottom;
	int nW = nR - nL;
	int nH = nB - nT;
	if (nL < 0 || nR <= nL || nT < 0 || nB <= nT || nR <= 0 || nB <= 0)
		return;
	if (nL >= nImgW - 1 || nR > nImgW || nT >= nImgH - 1 || nB > nImgH)
		return;
	if (nW < nMinW || nH < nMinH || nW > nImgW || nH > nImgH)
		return;

	cv::Mat imgORG(nHeight, nWidth, CV_32FC1, retFullImg);
	cv::Mat imgORGTR(nHeight, nWidth, CV_8UC1, ptrTR);
	cv::Mat imgORGTG(nHeight, nWidth, CV_8UC1, ptrTG);
	cv::Mat imgORGTB(nHeight, nWidth, CV_8UC1, ptrTB);
	cv::Mat img3D(nH, nW, CV_32FC1, cv::Scalar(0));
	cv::Mat imgTR(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTG(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTB(nH, nW, CV_8UC1, cv::Scalar(0));

	GetCropZmap_LT(imgORG.ptr<float>(), img3D.ptr<float>(), nWidth, nHeight, nL, nT, nW, nH);
	GetClipImage_LT(imgORGTR.data, nWidth, nHeight, imgTR.data, nL, nT, nW, nH);
	GetClipImage_LT(imgORGTG.data, nWidth, nHeight, imgTG.data, nL, nT, nW, nH);
	GetClipImage_LT(imgORGTB.data, nWidth, nHeight, imgTB.data, nL, nT, nW, nH);

	cv::Mat img3DTemp;
	cv::Mat imgTRTemp;
	cv::Mat imgTGTemp;
	cv::Mat imgTBTemp;
	if (100 > dPer)
	{
		if (dPer < 10) dPer = 10;
		nMinW = 500;
		nMinH = 500;
		nImgW = img3D.cols;
		nImgH = img3D.rows;
		for (int nDiv = 0; nDiv < 10; nDiv++)
		{
			if (nDiv > 0)
				dPer += (5.0 * nDiv);
			if (dPer > 100)
				break;

			int nImgW_Temp = nImgW * dPer / 100.0;
			int nImgH_Temp = nImgH * dPer / 100.0;
			if (nImgW_Temp < nMinW || nImgH_Temp < nMinH)
				continue;

			nImgW = nImgW_Temp;
			nImgH = nImgH_Temp;
			break;
		}
		cv::resize(img3D, img3DTemp, cv::Size(nImgW, nImgH));
		cv::resize(imgTR, imgTRTemp, cv::Size(nImgW, nImgH));
		cv::resize(imgTG, imgTGTemp, cv::Size(nImgW, nImgH));
		cv::resize(imgTB, imgTBTemp, cv::Size(nImgW, nImgH));
	}
	else
	{
		img3DTemp = img3D;
		imgTRTemp = imgTR;
		imgTGTemp = imgTG;
		imgTBTemp = imgTB;
	}

	if (img3DTemp.empty() || imgTRTemp.empty() || imgTGTemp.empty() || imgTBTemp.empty())
		return;

	cv::Mat img32(img3DTemp.rows, img3DTemp.cols, CV_32FC1, cv::Scalar(0));
	UCHAR* pDest24 = NULL;
	RECT roi;
	roi.left = 0;
	roi.top = 0;
	roi.right = img3DTemp.cols;
	roi.bottom = img3DTemp.rows;
	alpf_make_img24n32f(roi, img3DTemp.cols, imgTRTemp.data, imgTGTemp.data, imgTBTemp.data, 1, 1, 1, img3DTemp.cols, img32.ptr<float>(), pDest24);

	HEADER_PTT hd;
	hd.uiNumRow = img3DTemp.cols;
	hd.uiNumCol = img3DTemp.rows;
	hd.fNumRow = (float)img3DTemp.cols;
	hd.fNumCol = (float)img3DTemp.rows;
	hd.pixelResX = (float)_mm2micron(m_resolX) * dFacW;
	hd.pixelResY = (float)_mm2micron(m_resolY) * dFacH;
	hd.zResolX = (float)_mm2micron(m_resolX) * dFacW;
	hd.zResolY = (float)_mm2micron(m_resolY) * dFacH;
	alpf_save_ptt(sPath, &hd, img3DTemp.ptr<float>(), img32.ptr<float>());
}
void FOREIGN_INSP::GetWP3D(float* pf3D, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, int nModuleID, CString sPath)
{
	if (pf3D == NULL || retFullImg == NULL || nWidth <= 0 || nHeight <= 0)
		return;

	InspForeignInfo sForeign = m_ForeignData;
	bool bSave = sForeign.UseData(FR_DATA_DebugData) || sForeign.sWP.UseData(FR_WP_DT_CorrectWPSave);
	bool bSimpleSave = sForeign.sWP.UseData(FR_WP_DT_CorrectWPSimpleSave);

	if (rcClip.left < 0) rcClip.left = 0;
	if (rcClip.right < 0) rcClip.right = 0;
	if (rcClip.top < 0) rcClip.top = 0;
	if (rcClip.bottom < 0) rcClip.bottom = 0;
	if (rcClip.left > nWidth) rcClip.left = nWidth;
	if (rcClip.right > nWidth) rcClip.right = nWidth;
	if (rcClip.top > nHeight) rcClip.top = nHeight;
	if (rcClip.bottom > nHeight) rcClip.bottom = nHeight;

	int nL = rcClip.left;
	int nR = rcClip.right;
	int nT = rcClip.top;
	int nB = rcClip.bottom;
	int nW = nR - nL;
	int nH = nB - nT;
	if (nL < 0 || nR <= nL || nT < 0 || nB <= nT || nR <= 0 || nB <= 0)
		return;
	if (nL >= nWidth - 1 || nR > nWidth || nT >= nHeight - 1 || nB > nHeight)
		return;
	if (nW > nWidth || nH > nHeight)
		return;

	double dDivZ = ((dMax * 1.15) - dMin) / (double)CR_MAP_SIZE;
	cv::Mat imgInsp3D(nH, nW, CV_32FC1, cv::Scalar(0));
	for (int y = nT; y < nB && y < nHeight; y++)
	{
		for (int x = nL; x < nR && x < nWidth; x++)
		{
			long long nIndex = (y * nWidth) + x;
			long long nIndex2 = nIndex * 3;
			UCHAR ucB = retFullImg[nIndex2];
			UCHAR ucG = retFullImg[nIndex2 + 1];
			UCHAR ucR = retFullImg[nIndex2 + 2];

			float fH = GetRGBH(ucR, ucG, ucB, dMin, dMax);
			if (fH == 0 && (ucR > 0 || ucG > 0 || ucB > 0))
			{
				for (int y2 = -1; y2 < 1; y2++)
				{
					for (int x2 = -1; x2 < 1; x2++)
					{
						if (x2 == 0 && y2 == 0)
							continue;

						int nX2 = (x + x2);
						int nY2 = (y + y2);
						if (nX2 < 0 || nX2 >= nW || nY2 < 0 || nY2 >= nH)
							continue;

						fH = pf3D[nIndex];
						if (fH != 0)
							break;
					}
					if (fH != 0)
						break;
				}
			}

			pf3D[nIndex] = fH;
			int nX = x - nL;
			int nY = y - nT;
			if (nX < 0 || nX >= nW || nY < 0 || nY >= nH)
				continue;
			imgInsp3D.ptr<float>(nY)[nX] = fH;
		}
	}

	if ((bSave || bSimpleSave) && nModuleID > 0)
	{
		CString dir = _T("");
		dir.Format(_T("D:\\Eagle3D_data\\ForeignDebugData\\Module%d"), nModuleID);
		if (sPath.GetLength() > 0)
			dir.Format(_T("%s"), sPath);

		if (bSave)
			SaveReleaseWorkImg(retFullImg, nWidth, nHeight, _T("WP_FULL_SRC.bmp"), dir, 3);

		CString sPath = _T("");
		sPath.Format(_T("%s\\WP_FULL_SRC_3D.ptr"), dir);
		SavePTR(sPath, imgInsp3D);
	}
}
#pragma endregion MINSPM

#pragma region SAVE_NG
void FOREIGN_INSP::ForeignNG_Save(InspFR_Save sData, UCHAR * ucDst)
{
	int nLine = __LINE__;
	try
	{
		if (!g_pMPTI)
			return;

		InspPartInfo* pBoard = m_pInspBoardInfo_Foreign;
		if (!pBoard)
			return;

		nLine = __LINE__;
		CString fullPath = _T("");
		fullPath.Format(_T("%s\\%s"), sData.m_s3DPath, sData.m_sName);
		CreateDir(sData.m_s3DPath);
		nLine = __LINE__;

		int nDivide = sData.m_nArrData[m_eInspFR_Save_Divide];
		int nGrabMode = sData.m_nArrData[m_eInspFR_Save_GrabMode];
		float* pf3D = pBoard->zmapForeignData.data;
		int nFOVW = pBoard->zmapForeignData.zmapSizeX;
		int nFOVH = pBoard->zmapForeignData.zmapSizeY;
		int nImgW = nFOVW / nDivide - 1;
		int nImgH = nFOVH / nDivide - 1;
		nLine = __LINE__;
		if (nImgW <= 0 || nImgH <= 0)
			return;

		InspForeignInfo sFR = m_ForeignData;
		float redFator = sFR.m_fArrData[FR_F_FactorR];
		float greenFator = sFR.m_fArrData[FR_F_FactorG];
		float blueFator = sFR.m_fArrData[FR_F_FactorB];
		float btmRedfactor = sFR.m_fArrData[FR_F_FactorBR];
		float btmBluefactor = sFR.m_fArrData[FR_F_FactorBB];
		UCHAR* pucTR = NULL;
		UCHAR* pucTG = NULL;
		UCHAR* pucTB = NULL;
		UCHAR* pucBR = NULL;
		UCHAR* pucBB = NULL;
		int nFov2DW = 0;
		int nFov2DH = 0;

		if (sData.UseData(m_eInspFR_Save_Data_FOV))
		{
			pucTR = sData.m_pucTR;
			pucTG = sData.m_pucTG;
			pucTB = sData.m_pucTB;
			pucBR = sData.m_pucBR;
			pucBB = sData.m_pucBB;
			nFov2DW = sData.m_nArrData[m_eInspFR_Save_Fov2DW];
			nFov2DH = sData.m_nArrData[m_eInspFR_Save_Fov2DH];
#if FR_MULTI_FUNC == 1
			if (g_pMPTI->GetUseMultiProcess() && g_pMPTI->GetUseMultiProcess_Foreign())
			{
				
				int nTool = ext::irs::get()->m_vToolForeign[ext::irs::get()->_nFov % ext::env::nForeignTypeToolCnt];
				int nBufferIndex = ext::irs::get()->_Tools[nTool].InspFovGetIndex;
				//CString sLogs;
				//sLogs.Format(_T("[Ext][Set][Foreign][Result][X] nTool %d, ForePartInfo->fovIndex %d ,  nFOV %d "), nTool, nBufferIndex, ext::irs::get()->_nFov);
				//g_pMPTI->AddLog_Dev(sLogs);
				//CString sLogs;
				int nLastTool = nTool;
				//sLogs.Format(_T("m_eInspFR_Save_Data_FOV %d"), nBufferIndex);
				//g_pMPTI->AddLog_Dev(sLogs);
				// 이쪽 탈경우에 죽을수도있음 , 현재 2D Data 빼오는방법에대해 변경이 필요
				ZmapData  MultiTempZmapData = ext::irs::get()->_Tools[nTool].m_MultiTempZmapData[nBufferIndex];
				tagRoiImgBuf  MultiRoiImgBuf = ext::irs::get()->_Tools[nTool].m_MultiRoiImgBuf[nBufferIndex];
				InspPartInfo* pBoardTemp = new InspPartInfo();
				ext::irs::get()->_Tools[nLastTool]._ForeignShared[nBufferIndex].GetForeignPartInfoData(pBoardTemp, true);
				MultiTempZmapData.CopyData(pBoardTemp->zmapForeignData); // 가져온 데이터를 Temp 데이터 옮김 
				ext::irs::get()->_Tools[nLastTool]._ForeignShared[nBufferIndex].GetNGTempData(pucTR, pucTG, pucTB, pucBR, pucBB);
				pf3D = MultiTempZmapData.data; // Temp 구조체에서 3D Data 사용
				nFOVW = MultiTempZmapData.zmapSizeX;
				nFOVH = MultiTempZmapData.zmapSizeY;
				nImgW = nFOVW / nDivide - 1;
				nImgH = nFOVH / nDivide - 1;
				delete pBoardTemp;
			}
#endif
		}
		else
		{
			pucTR = pBoard->partImgBuf.imgTop_R;
			pucTG = pBoard->partImgBuf.imgTop_G;
			pucTB = pBoard->partImgBuf.imgTop_B;
			pucBR = pBoard->partImgBuf.imgBottom_R;
			pucBB = pBoard->partImgBuf.imgBottom_B;
			nFov2DW = pBoard->partImgBuf.nImageSizeX;
			nFov2DH = pBoard->partImgBuf.nImageSizeY;
#if FR_MULTI_FUNC == 1
			if (g_pMPTI->GetUseMultiProcess() && g_pMPTI->GetUseMultiProcess_Foreign())
			{

		
				int nTool = ext::irs::get()->m_vToolForeign[ext::irs::get()->_nFov % ext::env::nForeignTypeToolCnt];
				int nBufferIndex = ext::irs::get()->_Tools[nTool].InspFovGetIndex;


				ZmapData  MultiTempZmapData = ext::irs::get()->_Tools[nTool].m_MultiTempZmapData[nBufferIndex];
				tagRoiImgBuf  MultiRoiImgBuf = ext::irs::get()->_Tools[nTool].m_MultiRoiImgBuf[nBufferIndex];
			/*	CString sLogs;
				sLogs.Format(_T("[Ext][Set][Foreign][Result] nTool %d, ForePartInfo->fovIndex %d ,  nFOV %d "), nTool, nBufferIndex, ext::irs::get()->_nFov);
				g_pMPTI->AddLog_Dev(sLogs);*/
				int nLastTool = nTool;
				//InspPartInfo* pBoardTemp = new InspPartInfo();
				//ext::irs::get()->_Tools[nLastTool]._ForeignShared[nBufferIndex].GetForeignPartInfoData(pBoardTemp, false);
				//MultiTempZmapData.CopyData(pBoardTemp->zmapForeignData); // 가져온 데이터를 Temp 데이터 옮김 
				//MultiRoiImgBuf.CopyData(pBoardTemp->partImgBuf);
				pucTR = MultiRoiImgBuf.imgTop_R;
				pucTG = MultiRoiImgBuf.imgTop_G;
				pucTB = MultiRoiImgBuf.imgTop_B;
				pucBR = MultiRoiImgBuf.imgBottom_R;
				pucBB = MultiRoiImgBuf.imgBottom_B;
				nFov2DW = MultiRoiImgBuf.nImageSizeX;
				nFov2DH = MultiRoiImgBuf.nImageSizeY;
				nFOVW = MultiTempZmapData.zmapSizeX;
				nFOVH = MultiTempZmapData.zmapSizeY;
				pf3D = MultiTempZmapData.data; // Temp 구조체에서 3D Data 사용
				nImgW = nFOVW / nDivide - 1;
				nImgH = nFOVH / nDivide - 1;
				//delete pBoardTemp;
			}
#endif
		}
		if (pucTR == NULL || pucTG == NULL || pucTB == NULL ||
			pucBR == NULL || pucBB == NULL)
			return;
		if (nFov2DW <= 0 || nFov2DH <= 0)
			return;

		nLine = __LINE__;
		int nGapX = sData.m_nArrData[m_eInspFR_Save_GapX];
		int nGapY = sData.m_nArrData[m_eInspFR_Save_GapY];
		if (nGapX < 0) nGapX = 0;
		if (nGapY < 0) nGapY = 0;
		if (nGapX > 100) nGapX = 100;
		if (nGapY > 100) nGapY = 100;
		nLine = __LINE__;

		tagAForeignResult vforeign = sData.m_sforeign;
		CString partName = _T("");
		partName.Format(_T("%s"), sData.m_sName);
		nLine = __LINE__;
		if (partName.MakeUpper().Find(_T("BUBBLE")) >= 0)
		{
			int nRectW = vforeign.m_rcRect.right - vforeign.m_rcRect.left;
			int nRectH = vforeign.m_rcRect.bottom - vforeign.m_rcRect.top;
			int nLength = (nRectW + nRectH) / 2;
			nGapX = nLength * 8;
			nGapY = nLength * 8;
		}
		nLine = __LINE__;

		int nL = vforeign.m_rcRect.left - nGapX;
		int nR = vforeign.m_rcRect.right + nGapX;
		int nT = vforeign.m_rcRect.top - nGapY;
		int nB = vforeign.m_rcRect.bottom + nGapY;
		if (nL < 0) nL = 0;
		if (nR < 0) nR = 0;
		if (nT < 0) nT = 0;
		if (nB < 0) nB = 0;
		nLine = __LINE__;
		if (nL > nImgW) nL = nImgW;
		if (nR > nImgW) nR = nImgW;
		if (nT > nImgH) nT = nImgH;
		if (nB > nImgH) nB = nImgH;
		nLine = __LINE__;
		if (nR <= nL || nB <= nT) return;
		int nW = nR - nL;
		int nH = nB - nT;
		int nWTemp = nW % 4;
		nW -= nWTemp;
		nLine = __LINE__;
		if (nW < 4 || nH < 4)
			return;
		nLine = __LINE__;
		UCHAR* pUcColorImg = g_pMManager->pem_new<UCHAR>(true, nW * nH * 3, (PCHAR)__FUNCTION__, __LINE__);
		memset(pUcColorImg, 0, sizeof(UCHAR) * nW * nH);
		nLine = __LINE__;
		if (g_pMPTI->m_nCameraType == Basler_Color || nImgH != nFov2DH || nImgW != nFov2DW)
		{
			SaveDispImage_Foreign(fullPath, FALSE, pUcColorImg, nGapX, nGapY, vforeign, nDivide);
			nLine = __LINE__;
			Delete_1DArray(&pUcColorImg);
			nLine = __LINE__;
			return;
		}
		nLine = __LINE__;

		float* img32 = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
		float* clipzmap = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* clipImgR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* clipImgG = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* clipImgB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* clipImgBR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* clipImgBB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
		nLine = __LINE__;
		memset(img32, 0, sizeof(float) * nW * nH);
		memset(clipzmap, 0, sizeof(float) * nW * nH);
		memset(clipImgR, 0, sizeof(UCHAR) * nW * nH);
		memset(clipImgG, 0, sizeof(UCHAR) * nW * nH);
		memset(clipImgB, 0, sizeof(UCHAR) * nW * nH);
		memset(clipImgBR, 0, sizeof(UCHAR) * nW * nH);
		memset(clipImgBB, 0, sizeof(UCHAR) * nW * nH);
		nLine = __LINE__;
		if (pucTR) GetClipImage_LT(pucTR, nImgW, nImgH, clipImgR, nL, nT, nW, nH);
		if (pucTG) GetClipImage_LT(pucTG, nImgW, nImgH, clipImgG, nL, nT, nW, nH);
		if (pucTB) GetClipImage_LT(pucTB, nImgW, nImgH, clipImgB, nL, nT, nW, nH);
		if (pucBR) GetClipImage_LT(pucBR, nImgW, nImgH, clipImgBR, nL, nT, nW, nH);
		if (pucBB) GetClipImage_LT(pucBB, nImgW, nImgH, clipImgBB, nL, nT, nW, nH);
		nLine = __LINE__;

		cv::Mat imgSrc(nH, nW, CV_8UC3, pUcColorImg);
		cv::Mat imgSrcR(nH, nW, CV_8UC1, clipImgR);
		cv::Mat imgSrcG(nH, nW, CV_8UC1, clipImgG);
		cv::Mat imgSrcB(nH, nW, CV_8UC1, clipImgB);
		cv::Mat imgSrcBR(nH, nW, CV_8UC1, clipImgBR);
		cv::Mat imgSrcBB(nH, nW, CV_8UC1, clipImgBB);

		RECT rectZerotofull = { 0,0, nW, nH };
		nLine = __LINE__;
		if (g_pMPTI->GetCompositeLightMode() == 0)
		{
			nLine = __LINE__;
			if (vforeign.GetN(m_eFR_RST_N_Defect) == Foreign_NG_Type_Bubble)
			{
				nLine = __LINE__;
				alpf_make_img24n32f(rectZerotofull, nW, clipImgBB, clipImgBB, clipImgBB, 1, 1, 1, nW, img32, pUcColorImg);
				nLine = __LINE__;
				HEADER_POT hd;
				hd.fNumCol = (float)nW;
				hd.fNumRow = (float)nH;
				hd.fPixelResX = (float)m_resolX;
				hd.fPixelResY = (float)m_resolY;
				hd.fDepthBit = 8;
				alpf_save_put(fullPath, &hd, clipImgR, clipImgG, clipImgB, clipImgBB, clipImgBR);
				nLine = __LINE__;
			}
			else
			{
				nLine = __LINE__;
				alpf_make_img24n32f(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, redFator, greenFator, blueFator, nW, img32, pUcColorImg);
				nLine = __LINE__;
			}
			nLine = __LINE__;
		}
		else
		{
			nLine = __LINE__;
			alpf_make_img24n32f_CompositeBtm(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, clipImgBR, clipImgBB,
				redFator, greenFator, blueFator, btmRedfactor, btmBluefactor, g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB,
				nW, img32, pUcColorImg);
			nLine = __LINE__;
		}

		nLine = __LINE__;
		if (g_pMPTI->IsForeignEnable())
		{
			nLine = __LINE__;
			int nCX3D = nL + (nW / 2);
			int nCY3D = nT + (nH / 2);
			GetCropZmap(pf3D, clipzmap, nFOVW, nFOVH, nCX3D, nCY3D, nW, nH);
			nLine = __LINE__;
			HEADER_PTT hd;
			hd.uiNumRow = nW;
			hd.uiNumCol = nH;
			hd.fNumRow = (float)nW;
			hd.fNumCol = (float)nH;
			hd.pixelResX = (float)_mm2micron(m_resolX) * (float)nDivide;
			hd.pixelResY = (float)_mm2micron(m_resolY) * (float)nDivide;
			hd.zResolX = (float)_mm2micron(m_resolX) * (float)nDivide;
			hd.zResolY = (float)_mm2micron(m_resolY) * (float)nDivide;
			nLine = __LINE__;
			alpf_save_ptt(fullPath, &hd, clipzmap, img32);
			nLine = __LINE__;
		}

		nLine = __LINE__;
		Delete_1DArray(&img32);
		Delete_1DArray(&clipzmap);
		Delete_1DArray(&clipImgR);
		Delete_1DArray(&clipImgG);
		Delete_1DArray(&clipImgB);
		Delete_1DArray(&clipImgBB);
		Delete_1DArray(&clipImgBR);
		nLine = __LINE__;

		if (sData.UseData(m_eInspFR_Save_Data_e2D))
		{
			CString str2D = _T("");
			str2D.Format(_T("%s\\%s.jpg"), sData.m_s2DPath, sData.m_sName);
			CreateDir(sData.m_s2DPath);
			Save_JpegImage(pUcColorImg, nW, nH, 3, IC_BGR, str2D, sData.m_nArrData[m_eInspFR_Save_Quality]);
		}
		nLine = __LINE__;
		if (sData.UseData(m_eInspFR_Save_Data_MES))
		{
			CString fileName = _T("");
			CString path = _T("");
			path.Format(_T("%s"), sData.m_sMESPath);
			CreateDir(sData.m_sMESPath);
			if (sData.UseData(m_eInspFR_Save_Data_PNG))
				fileName.Format(_T("%s.png"), sData.m_sMESName);
			else if (sData.UseData(m_eInspFR_Save_Data_TIF))
				fileName.Format(_T("%s.tif"), sData.m_sMESName);
			else
				fileName.Format(_T("%s.jpg"), sData.m_sMESName);

			SaveReleaseWorkImg(pUcColorImg, nW, nH, fileName, path, 3);
		}
		nLine = __LINE__;

		if (sData.UseData(m_eInspFR_Save_Data_RUN) && ucDst)
		{
			int nColorImgW = sData.m_nArrData[m_eInspFR_Save_ColorImgW];
			int nColorImgH = sData.m_nArrData[m_eInspFR_Save_ColorImgH];
			if (nColorImgW >= 4 && nColorImgH >= 4 && nW >= nColorImgW && nH >= nColorImgH)
			{
#if _DEBUG
				cv::Mat imgDst(sData.m_nArrData[m_eInspFR_Save_ColorImgH], sData.m_nArrData[m_eInspFR_Save_ColorImgW], CV_8UC3, ucDst);
#endif
				int nSX = (nW - nColorImgW) / 2;
				int nSY = (nH - nColorImgH) / 2;
				int nEX = nSX + nColorImgW;
				int nEY = nSY + nColorImgH;
				for (int y = nSY; y < nEY; y++)
				{
					if (y < 0) continue;
					if (y >= nH) break;
					for (int x = nSX; x < nEX; x++)
					{
						if (x < 0) continue;
						if (x >= nW) break;

						int nX = x - nSX;
						int nY = y - nSY;
						if (nX < 0) continue;
						if (nY < 0) continue;
						if (nX >= nColorImgW) break;
						if (nY >= nColorImgH) break;

						int nIndex = ((y * nW) + x) * 3;
						int nIndexRst = ((nY * nColorImgW) + nX) * 3;
						ucDst[nIndexRst] = pUcColorImg[nIndex];
						ucDst[nIndexRst + 1] = pUcColorImg[nIndex + 1];
						ucDst[nIndexRst + 2] = pUcColorImg[nIndex + 2];
					}
				}
			}
		}
		nLine = __LINE__;

		if (sData.m_nSave_F_MES != 0)  // 0이 아닐 때에만 실행 
		{
			CString sSrc;
			sSrc.Format(_T("%s\\%s.jpg"), sData.m_s2DPath, sData.m_sName);

			CString sDst = sData.m_sMES_F_Path;
			sDst += _T(".jpg"); //확장자

			int nPos = sDst.ReverseFind('\\');
			if (nPos >= 0)
			{
				CString dstDir = sDst.Left(nPos);
				CreateDir(dstDir);
				CopyFile(sSrc, sDst, false);
			}
		}
		Delete_1DArray(&pUcColorImg);
	}
	catch (...)
	{
		CString sLog = _T("");
		sLog.Format(_T("Line : %d Pass!!!"), nLine);
		AddLOG(_T("[ERROR]"), _T("[FOREIGN_INSP]"), _T("[ForeignNG_Save]"), sLog);
		throw nLine;
	}
}
void FOREIGN_INSP::ForeignNG_Save_OFF(InspFR_Save sData, UCHAR * ucDst)
{
#ifdef MultiProcessNone 
	if (!g_pMPTI)
		return;

	if (!sData.m_pfOff3D)
		return;

	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), sData.m_s3DPath, sData.m_sName);
	CreateDir(sData.m_s3DPath);

	int nDivide = sData.m_nArrData[m_eInspFR_Save_Divide];
	int nGrabMode = sData.m_nArrData[m_eInspFR_Save_GrabMode];
	float* pf3D = sData.m_pfOff3D;
	int nFOVW = m_fovWidth;
	int nFOVH = m_fovLength;
	int nImgW = nFOVW / nDivide - 1;
	int nImgH = nFOVH / nDivide - 1;
	if (nImgW <= 0 || nImgH <= 0)
		return;

	InspPartInfo* pBoard = m_pInspBoardInfo_Foreign;
	int nFovID = 0;
#if _OFFLINE
	if (pBoard)
		nFovID = pBoard->fovIndex;
	if (nFovID < 0) nFovID = 0;
#endif

	UCHAR* pucTR_Org = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_TR, nGrabMode);
	UCHAR* pucTG_Org = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_TG, nGrabMode);
	UCHAR* pucTB_Org = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_TB, nGrabMode);
	UCHAR* pucBR_Org = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_BR, nGrabMode);
	UCHAR* pucBB_Org = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_BB, nGrabMode);
	if (pucTR_Org == NULL || pucTG_Org == NULL || pucTB_Org == NULL || pucBR_Org == NULL || pucBB_Org == NULL)
		return;

	cv::Mat imgTROrg(nFOVH, nFOVW, CV_8UC1, pucTR_Org);
	cv::Mat imgTGOrg(nFOVH, nFOVW, CV_8UC1, pucTG_Org);
	cv::Mat imgTBOrg(nFOVH, nFOVW, CV_8UC1, pucTB_Org);
	cv::Mat imgBROrg(nFOVH, nFOVW, CV_8UC1, pucBR_Org);
	cv::Mat imgBBOrg(nFOVH, nFOVW, CV_8UC1, pucBB_Org);
	if (imgTROrg.empty() || imgTGOrg.empty() || imgTBOrg.empty() || imgBROrg.empty() || imgBBOrg.empty())
		return;

	cv::Mat imgTR, imgTG, imgTB, imgBR, imgBB;
	cv::resize(imgTROrg, imgTR, cv::Size(nImgW, nImgH));
	cv::resize(imgTGOrg, imgTG, cv::Size(nImgW, nImgH));
	cv::resize(imgTBOrg, imgTB, cv::Size(nImgW, nImgH));
	cv::resize(imgBROrg, imgBR, cv::Size(nImgW, nImgH));
	cv::resize(imgBBOrg, imgBB, cv::Size(nImgW, nImgH));

	int nGapX = sData.m_nArrData[m_eInspFR_Save_GapX];
	int nGapY = sData.m_nArrData[m_eInspFR_Save_GapY];
	if (nGapX < 0) nGapX = 0;
	if (nGapY < 0) nGapY = 0;
	if (nGapX > 100) nGapX = 100;
	if (nGapY > 100) nGapY = 100;

	tagAForeignResult vforeign = sData.m_sforeign;
	CString partName = _T("");
	partName.Format(_T("%s"), sData.m_sName);
	if (partName.MakeUpper().Find(_T("BUBBLE")) >= 0)
	{
		int nRectW = vforeign.m_rcRect.right - vforeign.m_rcRect.left;
		int nRectH = vforeign.m_rcRect.bottom - vforeign.m_rcRect.top;
		int nLength = (nRectW + nRectH) / 2;
		nGapX = nLength * 8;
		nGapY = nLength * 8;
	}

	int nL = vforeign.m_rcRect.left - nGapX;
	int nR = vforeign.m_rcRect.right + nGapX;
	int nT = vforeign.m_rcRect.top - nGapY;
	int nB = vforeign.m_rcRect.bottom + nGapY;
	if (nL < 0) nL = 0;
	if (nR < 0) nR = 0;
	if (nT < 0) nT = 0;
	if (nB < 0) nB = 0;
	if (nL > nImgW) nL = nImgW;
	if (nR > nImgW) nR = nImgW;
	if (nT > nImgH) nT = nImgH;
	if (nB > nImgH) nB = nImgH;
	if (nR <= nL || nB <= nT) return;
	int nW = nR - nL;
	int nH = nB - nT;
	int nWTemp = nW % 4;
	nW -= nWTemp;
	if (nW < 4 || nH < 4)
		return;

	UCHAR* pUcColorImg = g_pMManager->pem_new<UCHAR>(true, nW * nH * 3, (PCHAR)__FUNCTION__, __LINE__);
	memset(pUcColorImg, 0, sizeof(UCHAR) * nW * nH);
	if (g_pMPTI->m_nCameraType == Basler_Color)
	{
		SaveDispImage_Foreign(fullPath, FALSE, pUcColorImg, nGapX, nGapY, vforeign, nDivide);
		Delete_1DArray(&pUcColorImg);
		return;
	}

	float* img32 = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	float* clipzmap = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgG = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgBR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgBB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	memset(img32, 0, sizeof(float) * nW * nH);
	memset(clipzmap, 0, sizeof(float) * nW * nH);
	memset(clipImgR, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgG, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgB, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgBR, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgBB, 0, sizeof(UCHAR) * nW * nH);
	GetClipImage_LT(imgTR.data, nImgW, nImgH, clipImgR, nL, nT, nW, nH);
	GetClipImage_LT(imgTG.data, nImgW, nImgH, clipImgG, nL, nT, nW, nH);
	GetClipImage_LT(imgTB.data, nImgW, nImgH, clipImgB, nL, nT, nW, nH);
	GetClipImage_LT(imgBR.data, nImgW, nImgH, clipImgBR, nL, nT, nW, nH);
	GetClipImage_LT(imgBB.data, nImgW, nImgH, clipImgBB, nL, nT, nW, nH);

#if _DEBUG
	cv::Mat imgSrc(nH, nW, CV_8UC3, pUcColorImg);
	cv::Mat imgSrcR(nH, nW, CV_8UC1, clipImgR);
	cv::Mat imgSrcG(nH, nW, CV_8UC1, clipImgG);
	cv::Mat imgSrcB(nH, nW, CV_8UC1, clipImgB);
	cv::Mat imgSrcBR(nH, nW, CV_8UC1, clipImgBR);
	cv::Mat imgSrcBB(nH, nW, CV_8UC1, clipImgBB);
#endif

	InspForeignInfo sFR = m_ForeignData;
	float redFator = sFR.m_fArrData[FR_F_FactorR];
	float greenFator = sFR.m_fArrData[FR_F_FactorG];
	float blueFator = sFR.m_fArrData[FR_F_FactorB];
	float btmRedfactor = sFR.m_fArrData[FR_F_FactorBR];
	float btmBluefactor = sFR.m_fArrData[FR_F_FactorBB];
	RECT rectZerotofull = { 0,0, nW, nH };
	if (g_pMPTI->GetCompositeLightMode() == 0)
	{
		if (vforeign.GetN(m_eFR_RST_N_Defect) == Foreign_NG_Type_Bubble)
		{
			alpf_make_img24n32f(rectZerotofull, nW, clipImgBB, clipImgBB, clipImgBB, 1, 1, 1, nW, img32, pUcColorImg);
			HEADER_POT hd;
			hd.fNumCol = (float)nW;
			hd.fNumRow = (float)nH;
			hd.fPixelResX = (float)m_resolX;
			hd.fPixelResY = (float)m_resolY;
			hd.fDepthBit = 8;
			alpf_save_put(fullPath, &hd, clipImgR, clipImgG, clipImgB, clipImgBB, clipImgBR);
		}
		else
			alpf_make_img24n32f(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, redFator, greenFator, blueFator, nW, img32, pUcColorImg);
	}
	else
	{
		alpf_make_img24n32f_CompositeBtm(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, clipImgBR, clipImgBB,
			redFator, greenFator, blueFator, btmRedfactor, btmBluefactor, g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB,
			nW, img32, pUcColorImg);
	}

	if (g_pMPTI->IsForeignEnable())
	{
		int nCX3D = nL + (nW / 2);
		int nCY3D = nT + (nH / 2);
		GetCropZmap(pf3D, clipzmap, nFOVW, nFOVH, nCX3D, nCY3D, nW, nH);
		HEADER_PTT hd;
		hd.uiNumRow = nW;
		hd.uiNumCol = nH;
		hd.fNumRow = (float)nW;
		hd.fNumCol = (float)nH;
		hd.pixelResX = (float)_mm2micron(m_resolX) * (float)nDivide;
		hd.pixelResY = (float)_mm2micron(m_resolY) * (float)nDivide;
		hd.zResolX = (float)_mm2micron(m_resolX) * (float)nDivide;
		hd.zResolY = (float)_mm2micron(m_resolY) * (float)nDivide;
		alpf_save_ptt(fullPath, &hd, clipzmap, img32);
	}

	Delete_1DArray(&img32);
	Delete_1DArray(&clipzmap);
	Delete_1DArray(&clipImgR);
	Delete_1DArray(&clipImgG);
	Delete_1DArray(&clipImgB);
	Delete_1DArray(&clipImgBB);
	Delete_1DArray(&clipImgBR);

	if (sData.UseData(m_eInspFR_Save_Data_e2D))
	{
		CString str2D = _T("");
		str2D.Format(_T("%s\\%s.jpg"), sData.m_s2DPath, sData.m_sName);
		CreateDir(sData.m_s2DPath);
		Save_JpegImage(pUcColorImg, nW, nH, 3, IC_BGR, str2D, sData.m_nArrData[m_eInspFR_Save_Quality]);
	}
	if (sData.UseData(m_eInspFR_Save_Data_MES))
	{
		CString fileName = _T("");
		CString path = _T("");
		path.Format(_T("%s"), sData.m_sMESPath);
		CreateDir(sData.m_sMESPath);
		if (sData.UseData(m_eInspFR_Save_Data_PNG))
			fileName.Format(_T("%s.png"), sData.m_sMESName);
		else if (sData.UseData(m_eInspFR_Save_Data_TIF))
			fileName.Format(_T("%s.tif"), sData.m_sMESName);
		else
			fileName.Format(_T("%s.jpg"), sData.m_sMESName);

		SaveReleaseWorkImg(pUcColorImg, nW, nH, fileName, path, 3);
	}

	if (sData.UseData(m_eInspFR_Save_Data_RUN) && ucDst)
	{
		int nColorImgW = sData.m_nArrData[m_eInspFR_Save_ColorImgW];
		int nColorImgH = sData.m_nArrData[m_eInspFR_Save_ColorImgH];
		if (nColorImgW >= 4 && nColorImgH >= 4 && nW >= nColorImgW && nH >= nColorImgH)
		{
#if _DEBUG
			cv::Mat imgDst(sData.m_nArrData[m_eInspFR_Save_ColorImgH], sData.m_nArrData[m_eInspFR_Save_ColorImgW], CV_8UC3, ucDst);
#endif
			int nSX = (nW - nColorImgW) / 2;
			int nSY = (nH - nColorImgH) / 2;
			int nEX = nSX + nColorImgW;
			int nEY = nSY + nColorImgH;
			for (int y = nSY; y < nEY; y++)
			{
				if (y < 0) continue;
				if (y >= nH) break;
				for (int x = nSX; x < nEX; x++)
				{
					if (x < 0) continue;
					if (x >= nW) break;

					int nX = x - nSX;
					int nY = y - nSY;
					if (nX < 0) continue;
					if (nY < 0) continue;
					if (nX >= nColorImgW) break;
					if (nY >= nColorImgH) break;

					int nIndex = ((y * nW) + x) * 3;
					int nIndexRst = ((nY * nColorImgW) + nX) * 3;
					ucDst[nIndexRst] = pUcColorImg[nIndex];
					ucDst[nIndexRst + 1] = pUcColorImg[nIndex + 1];
					ucDst[nIndexRst + 2] = pUcColorImg[nIndex + 2];
				}
			}
		}
	}

	Delete_1DArray(&pUcColorImg);
#endif

}
void FOREIGN_INSP::SaveDispImage_Foreign(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide)
{
#ifdef MultiProcessNone 
	CProcMil *procMil = g_pMPTI->GetProcMil();
	InspPartInfo* pBoard = m_pInspBoardInfo_Foreign;
	if (!pBoard)
		return;
	int nFOVW = pBoard->zmapData.zmapSizeX;
	int nFOVH = pBoard->zmapData.zmapSizeY;
	int nImgW = nFOVW / nDivide - 1;
	int nImgH = nFOVH / nDivide - 1;
	int nL = vforeign.m_rcRect.left - nGapX;
	int nR = vforeign.m_rcRect.right + nGapX;
	int nT = vforeign.m_rcRect.top - nGapY;
	int nB = vforeign.m_rcRect.bottom + nGapY;
	if (nL < 0) nL = 0;
	if (nR < 0) nR = 0;
	if (nT < 0) nT = 0;
	if (nB < 0) nB = 0;
	if (nL > nImgW) nL = nImgW;
	if (nR > nImgW) nR = nImgW;
	if (nT > nImgH) nT = nImgH;
	if (nB > nImgH) nB = nImgH;
	if (nR <= nL || nB <= nT) return;
	int nW = nR - nL;
	int nH = nB - nT;
	int nWTemp = nW % 4;
	nW -= nWTemp;
	if (nW < 4 || nH < 4)
		return;

	InspForeignInfo sFR = m_ForeignData;
	float redFator = sFR.m_fArrData[FR_F_FactorR];
	float greenFator = sFR.m_fArrData[FR_F_FactorG];
	float blueFator = sFR.m_fArrData[FR_F_FactorB];
	float btmRedfactor = sFR.m_fArrData[FR_F_FactorBR];
	float btmBluefactor = sFR.m_fArrData[FR_F_FactorBB];

	int nCX = nL + (nW / 2);
	int nCY = nT + (nH / 2);
	/*UCHAR* ConvertImgR = new UCHAR[nFOVW * nFOVH];
	UCHAR* ConvertImgG = new UCHAR[nFOVW * nFOVH];
	UCHAR* ConvertImgB = new UCHAR[nFOVW * nFOVH];
	UCHAR* ConvertImgBB = new UCHAR[nFOVW * nFOVH];
	UCHAR* ConvertImgBR = new UCHAR[nFOVW * nFOVH];*/
	UCHAR* ConvertImgR = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ConvertImgG = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ConvertImgB = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ConvertImgBB = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ConvertImgBR = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
	memset(ConvertImgR, 0, sizeof(UCHAR) * nFOVW * nFOVH);
	memset(ConvertImgG, 0, sizeof(UCHAR) * nFOVW * nFOVH);
	memset(ConvertImgB, 0, sizeof(UCHAR) * nFOVW * nFOVH);
	memset(ConvertImgBB, 0, sizeof(UCHAR) * nFOVW * nFOVH);
	memset(ConvertImgBR, 0, sizeof(UCHAR) * nFOVW * nFOVH);

	if (g_pMPTI->m_nCameraType == Basler_Color)
	{
		int nFOVSizeX = 2040;
		int nFOVSizeY = 2040;

		GetClipImage((UCHAR*)pBoard->fovImgBuf.imgTop_W, nFOVSizeX, nFOVSizeY, ConvertImgR, nImgW, nImgH, nFOVW, nFOVH);
		GetClipImage((UCHAR*)pBoard->fovImgBuf.imgMiddle_R, nFOVSizeX, nFOVSizeY, ConvertImgG, nImgW, nImgH, nFOVW, nFOVH);
		GetClipImage((UCHAR*)pBoard->fovImgBuf.imgMiddle_B, nFOVSizeX, nFOVSizeY, ConvertImgB, nImgW, nImgH, nFOVW, nFOVH);
		GetClipImage((UCHAR*)pBoard->fovImgBuf.imgBottom_B, nFOVSizeX, nFOVSizeY, ConvertImgBB, nImgW, nImgH, nFOVW, nFOVH);
		if (vforeign.GetN(m_eFR_RST_N_Defect) == Foreign_NG_Type_Bubble && vforeign.GetN(m_eFR_RST_N_UVColorSelect) > 2) // For Bubble Inspection
		{
			g_pMPTI->ConvertSubBB = ((UCHAR*)g_pMPTI->GetGrabOutputBuf_Color(pBoard->fovIndex, 2, 3));
			GetClipImage((UCHAR*)g_pMPTI->ConvertSubBB, nFOVSizeX, nFOVSizeY, ConvertImgBB, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
		}
		else
			GetClipImage((UCHAR*)pBoard->fovImgBuf.imgBottom_B, nFOVSizeX, nFOVSizeY, ConvertImgBB, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
	}
	else
	{
		if (procMil)
		{
			if (pBoard->fovImgBuf.imgTop_R != NULL)
				procMil->GetClipBuff(pBoard->fovImgBuf.imgTop_R, ConvertImgR, nImgW, nImgH, nFOVW, nFOVH);
			if (pBoard->fovImgBuf.imgTop_G != NULL)
				procMil->GetClipBuff(pBoard->fovImgBuf.imgTop_G, ConvertImgG, nImgW, nImgH, nFOVW, nFOVH);
			if (pBoard->fovImgBuf.imgTop_B != NULL)
				procMil->GetClipBuff(pBoard->fovImgBuf.imgTop_B, ConvertImgB, nImgW, nImgH, nFOVW, nFOVH);
			if (pBoard->fovImgBuf.imgBottom_B != NULL)
				procMil->GetClipBuff(pBoard->fovImgBuf.imgBottom_B, ConvertImgBB, nImgW, nImgH, nFOVW, nFOVH);
			if (pBoard->fovImgBuf.imgBottom_R != NULL)
				procMil->GetClipBuff(pBoard->fovImgBuf.imgBottom_R, ConvertImgBR, nImgW, nImgH, nFOVW, nFOVH);
		}
		SaveWorkImg(ConvertImgR, nFOVW, nFOVH, _T("Foreign_test_milTochar.bmp"));
		SaveWorkImg(ConvertImgBB, nFOVW, nFOVH, _T("Foreign_test_milTochar_BB.bmp"));
	}

	// uchar* -> resizing
	/*UCHAR* ResizingImgR = new UCHAR[nImgW * nImgH];
	UCHAR* ResizingImgG = new UCHAR[nImgW * nImgH];
	UCHAR* ResizingImgB = new UCHAR[nImgW * nImgH];
	UCHAR* ResizingImgBB = new UCHAR[nImgW * nImgH];
	UCHAR* ResizingImgBR = new UCHAR[nImgW * nImgH];*/
	UCHAR* ResizingImgR = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ResizingImgG = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ResizingImgB = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ResizingImgBB = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* ResizingImgBR = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
	memset(ResizingImgR, 0, sizeof(UCHAR) * nImgW * nImgH);
	memset(ResizingImgG, 0, sizeof(UCHAR) * nImgW * nImgH);
	memset(ResizingImgB, 0, sizeof(UCHAR) * nImgW * nImgH);
	memset(ResizingImgBB, 0, sizeof(UCHAR) * nImgW * nImgH);
	memset(ResizingImgBR, 0, sizeof(UCHAR) * nImgW * nImgH);
	double factor = GetResizeFactor(nImgW, nImgH, nFOVW, nFOVH);
	ResizeImg(ConvertImgR, ResizingImgR, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
	ResizeImg(ConvertImgG, ResizingImgG, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
	ResizeImg(ConvertImgB, ResizingImgB, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
	ResizeImg(ConvertImgBB, ResizingImgBB, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
	ResizeImg(ConvertImgBR, ResizingImgBR, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
	SaveWorkImg(ResizingImgR, nImgW, nImgH, _T("Foreign_test_resizing.bmp"));
	SaveWorkImg(ResizingImgBB, nImgW, nImgH, _T("Foreign_test_resizing_BB.bmp"));
	/*float* img32 = new float[nW * nH];
	float* clipzmap = new float[nW * nH];
	UCHAR* clipImgR = new UCHAR[nW * nH];
	UCHAR* clipImgG = new UCHAR[nW * nH];
	UCHAR* clipImgB = new UCHAR[nW * nH];
	UCHAR* clipImgBB = new UCHAR[nW * nH];
	UCHAR* clipImgBR = new UCHAR[nW * nH];*/
	float* img32 = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	float* clipzmap = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgG = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgBB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgBR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	memset(img32, 0, sizeof(float) * nW * nH);
	memset(clipzmap, 0, sizeof(float) * nW * nH);
	memset(clipImgR, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgG, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgB, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgBB, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgBR, 0, sizeof(UCHAR) * nW * nH);
	GetClipImage(ResizingImgR, nImgW, nImgH, clipImgR, nCX, nCY, nW, nH);
	GetClipImage(ResizingImgG, nImgW, nImgH, clipImgG, nCX, nCY, nW, nH);
	GetClipImage(ResizingImgB, nImgW, nImgH, clipImgB, nCX, nCY, nW, nH);
	GetClipImage(ResizingImgBB, nImgW, nImgH, clipImgBB, nCX, nCY, nW, nH);
	GetClipImage(ResizingImgBR, nImgW, nImgH, clipImgBR, nCX, nCY, nW, nH);
	SaveWorkImg(clipImgR, nW, nH, _T("Foreign_test_ClipImg.bmp"));
	SaveWorkImg(clipImgBB, nW, nH, _T("Foreign_test_ClipImg_BB.bmp"));
	RECT rectZerotofull = { 0,0, nW, nH };
	if (g_pMPTI->GetCompositeLightMode() == 0)
	{
		if (vforeign.GetN(m_eFR_RST_N_Defect) == Foreign_NG_Type_Bubble)
		{
			alpf_make_img24n32f(rectZerotofull, nW, clipImgBB, clipImgBB, clipImgBB, 1, 1, 1, nW, img32, retColorImg);
			HEADER_POT hd;
			hd.fNumCol = (float)nW;
			hd.fNumRow = (float)nH;
			hd.fPixelResX = (float)m_resolX;
			hd.fPixelResY = (float)m_resolY;
			hd.fDepthBit = 8;
			alpf_save_put(fullPath, &hd, clipImgR, clipImgG, clipImgB, clipImgBB, clipImgBR);
		}
		else
			alpf_make_img24n32f(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, redFator, greenFator, blueFator, nW, img32, retColorImg);
	}
	else
	{
		alpf_make_img24n32f_CompositeBtm(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, clipImgBR, clipImgBB,
			redFator, greenFator, blueFator, btmRedfactor, btmBluefactor, g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB,
			nW, img32, retColorImg);
	}
	if (g_pMPTI->IsForeignEnable() && zmapUse == TRUE)
	{
		int nCX3D = nCX;
		int nCY3D = nCY;
		GetCropZmap(pBoard->zmapForeignData.data, clipzmap, nFOVW, nFOVH, nCX3D, nCY3D, nW, nH);
		HEADER_PTT hd;
		hd.uiNumRow = nW;
		hd.uiNumCol = nH;
		hd.fNumRow = (float)nW;
		hd.fNumCol = (float)nH;
		hd.pixelResX = (float)_mm2micron(m_resolX);
		hd.pixelResY = (float)_mm2micron(m_resolY);
		hd.zResolX = (float)_mm2micron(m_resolX);
		hd.zResolY = (float)_mm2micron(m_resolY);
		alpf_save_ptt(fullPath, &hd, clipzmap, img32);
	}
	Delete_1DArray(&ConvertImgR);
	Delete_1DArray(&ConvertImgG);
	Delete_1DArray(&ConvertImgB);
	Delete_1DArray(&ConvertImgBB);
	Delete_1DArray(&ConvertImgBR);

	Delete_1DArray(&ResizingImgR);
	Delete_1DArray(&ResizingImgG);
	Delete_1DArray(&ResizingImgB);
	Delete_1DArray(&ResizingImgBR);
	Delete_1DArray(&ResizingImgBB);

	Delete_1DArray(&img32);
	Delete_1DArray(&clipzmap);
	Delete_1DArray(&clipImgR);
	Delete_1DArray(&clipImgG);
	Delete_1DArray(&clipImgB);
	Delete_1DArray(&clipImgBB);
	Delete_1DArray(&clipImgBR);
#endif

}
void FOREIGN_INSP::SaveDispImage_Foreign_new(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide)
{
	InspPartInfo* pBoard = m_pInspBoardInfo_Foreign;
	if (!pBoard)
		return;

	float* pf3D = pBoard->zmapForeignData.data;
	UCHAR* pucTR = pBoard->partImgBuf.imgTop_R;
	UCHAR* pucTG = pBoard->partImgBuf.imgTop_G;
	UCHAR* pucTB = pBoard->partImgBuf.imgTop_B;
	UCHAR* pucBR = pBoard->partImgBuf.imgBottom_R;
	UCHAR* pucBB = pBoard->partImgBuf.imgBottom_B;
	int nFov2DW = pBoard->partImgBuf.nImageSizeX;
	int nFov2DH = pBoard->partImgBuf.nImageSizeY;
	int nFOVW = pBoard->zmapData.zmapSizeX;
	int nFOVH = pBoard->zmapData.zmapSizeY;
	int nImgW = nFOVW / nDivide - 1;
	int nImgH = nFOVH / nDivide - 1;

	SaveDispImage_Foreign(fullPath, zmapUse, retColorImg, nGapX, nGapY, vforeign, nDivide, pf3D, pucTR, pucTG, pucTB, pucBR, pucBB, nFov2DW, nFov2DH, nFOVW, nFOVH);
}
void FOREIGN_INSP::SaveDispImage_Foreign_new(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide, float* pfOff3D, int nGrabMode, int nRunSave)
{
#ifdef MultiProcessNone 
	InspPartInfo* pBoard = m_pInspBoardInfo_Foreign;
	if (pfOff3D == NULL)
		return;

	float* pf3D = pfOff3D;
	int nFov2DW = 0;
	int nFov2DH = 0;
	int nFOVW = 0;
	int nFOVH = 0;
	if (g_pMPTI)
	{
		nFov2DW = m_fovWidth;
		nFov2DH = m_fovLength;
		nFOVW = nFov2DW;
		nFOVH = nFov2DH;
	}
	else
		return;

	if (nFov2DW <= 0 || nFov2DH <= 0 || nFOVW <= 0 || nFOVH <= 0)
		return;

	int nFovID = 0;
	if (pBoard && nRunSave == 2)
		nFovID = pBoard->fovIndex;
	if (nFovID < 0) nFovID = 0;

	UCHAR* pucR = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_TR, nGrabMode);
	UCHAR* pucG = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_TG, nGrabMode);
	UCHAR* pucB = g_pMPTI->GetSeqGrabBuf(eMGS_FG, nFovID, eM2C_TB, nGrabMode);
	if (pucR == NULL || pucG == NULL || pucB == NULL)
		return;

	cv::Mat imgROrg(nFOVH, nFOVW, CV_8UC1, pucR);
	cv::Mat imgGOrg(nFOVH, nFOVW, CV_8UC1, pucG);
	cv::Mat imgBOrg(nFOVH, nFOVW, CV_8UC1, pucB);
	InspForeignInfo sFR = m_ForeignData;
	float redFator = sFR.m_fArrData[FR_F_FactorR];
	float greenFator = sFR.m_fArrData[FR_F_FactorG];
	float blueFator = sFR.m_fArrData[FR_F_FactorB];
	float btmRedfactor = sFR.m_fArrData[FR_F_FactorBR];
	float btmBluefactor = sFR.m_fArrData[FR_F_FactorBB];

	if (imgROrg.empty() || imgGOrg.empty() || imgBOrg.empty())
		return;

	int nImgW = nFOVW / nDivide - 1;
	int nImgH = nFOVH / nDivide - 1;
	cv::Mat imgR, imgG, imgB;
	cv::resize(imgROrg, imgR, cv::Size(nImgW, nImgH));
	cv::resize(imgGOrg, imgG, cv::Size(nImgW, nImgH));
	cv::resize(imgBOrg, imgB, cv::Size(nImgW, nImgH));
	UCHAR* pucTR = imgR.ptr<UCHAR>();
	UCHAR* pucTG = imgG.ptr<UCHAR>();
	UCHAR* pucTB = imgB.ptr<UCHAR>();
	UCHAR* pucBR = NULL;
	UCHAR* pucBB = NULL;

	SaveDispImage_Foreign(fullPath, zmapUse, retColorImg, nGapX, nGapY, vforeign, nDivide, pf3D, pucTR, pucTG, pucTB, pucBR, pucBB, nImgW, nImgH, nFOVW, nFOVH);
#endif
}
void FOREIGN_INSP::SaveDispImage_Foreign(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide,
	float* pf3D, UCHAR* pucTR, UCHAR* pucTG, UCHAR* pucTB, UCHAR* pucBR, UCHAR* pucBB, int nFov2DW, int nFov2DH, int nFOVW, int nFOVH)
{
	int nImgW = nFOVW / nDivide - 1;
	int nImgH = nFOVH / nDivide - 1;

	//사이즈 다를시, 바슬러 컬러카메라(CI)일때 기존 시퀀스 타도록 수정
	if (nImgH != nFov2DH || nImgW != nFov2DW || g_pMPTI->m_nCameraType == Basler_Color)
		SaveDispImage_Foreign(fullPath, zmapUse, retColorImg, nGapX, nGapY, vforeign, nDivide);

	int nL = vforeign.m_rcRect.left - nGapX;
	int nR = vforeign.m_rcRect.right + nGapX;
	int nT = vforeign.m_rcRect.top - nGapY;
	int nB = vforeign.m_rcRect.bottom + nGapY;
	if (nL < 0) nL = 0;
	if (nR < 0) nR = 0;
	if (nT < 0) nT = 0;
	if (nB < 0) nB = 0;
	if (nL > nImgW) nL = nImgW;
	if (nR > nImgW) nR = nImgW;
	if (nT > nImgH) nT = nImgH;
	if (nB > nImgH) nB = nImgH;
	if (nR <= nL || nB <= nT) return;
	int nW = nR - nL;
	int nH = nB - nT;
	int nWTemp = nW % 4;
	nW -= nWTemp;
	if (nW < 4 || nH < 4)
		return;

	int nCX = nL + (nW / 2);
	int nCY = nT + (nH / 2);

#if _DEBUG
	cv::Mat imgSrc(nH, nW, CV_8UC3, retColorImg);
#endif

	double factor = GetResizeFactor(nImgW, nImgH, nFOVW, nFOVH);
	InspForeignInfo sFR = m_ForeignData;
	float redFator = sFR.m_fArrData[FR_F_FactorR];
	float greenFator = sFR.m_fArrData[FR_F_FactorG];
	float blueFator = sFR.m_fArrData[FR_F_FactorB];
	float btmRedfactor = sFR.m_fArrData[FR_F_FactorBR];
	float btmBluefactor = sFR.m_fArrData[FR_F_FactorBB];

	float* img32 = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	float* clipzmap = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgG = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgBB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipImgBR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	memset(img32, 0, sizeof(float) * nW * nH);
	memset(clipzmap, 0, sizeof(float) * nW * nH);
	memset(clipImgR, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgG, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgB, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgBB, 0, sizeof(UCHAR) * nW * nH);
	memset(clipImgBR, 0, sizeof(UCHAR) * nW * nH);
	if (pucTR) GetClipImage(pucTR, nImgW, nImgH, clipImgR, nCX, nCY, nW, nH);
	if (pucTG) GetClipImage(pucTG, nImgW, nImgH, clipImgG, nCX, nCY, nW, nH);
	if (pucTB) GetClipImage(pucTB, nImgW, nImgH, clipImgB, nCX, nCY, nW, nH);
	if (pucBB) GetClipImage(pucBB, nImgW, nImgH, clipImgBB, nCX, nCY, nW, nH);
	if (pucBR) GetClipImage(pucBR, nImgW, nImgH, clipImgBR, nCX, nCY, nW, nH);
	SaveWorkImg(clipImgR, nW, nH, _T("Foreign_test_ClipImg.bmp"));
	SaveWorkImg(clipImgBB, nW, nH, _T("Foreign_test_ClipImg_BB.bmp"));
	RECT rectZerotofull = { 0,0, nW, nH };
	if (g_pMPTI->GetCompositeLightMode() == 0)
	{
		if (vforeign.GetN(m_eFR_RST_N_Defect) == Foreign_NG_Type_Bubble)
		{
			alpf_make_img24n32f(rectZerotofull, nW, clipImgBB, clipImgBB, clipImgBB, 1, 1, 1, nW, img32, retColorImg);
			HEADER_POT hd;
			hd.fNumCol = (float)nW;
			hd.fNumRow = (float)nH;
			hd.fPixelResX = (float)m_resolX;
			hd.fPixelResY = (float)m_resolY;
			hd.fDepthBit = 8;
			alpf_save_put(fullPath, &hd, clipImgR, clipImgG, clipImgB, clipImgBB, clipImgBR);
		}
		else
			alpf_make_img24n32f(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, redFator, greenFator, blueFator, nW, img32, retColorImg);
	}
	else
	{
		alpf_make_img24n32f_CompositeBtm(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, clipImgBR, clipImgBB,
			redFator, greenFator, blueFator, btmRedfactor, btmBluefactor, g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB,
			nW, img32, retColorImg);
	}
	if (g_pMPTI->IsForeignEnable() && zmapUse == TRUE)
	{
		int nCX3D = nCX;
		int nCY3D = nCY;
		GetCropZmap(pf3D, clipzmap, nFOVW, nFOVH, nCX3D, nCY3D, nW, nH);
		HEADER_PTT hd;
		hd.uiNumRow = nW;
		hd.uiNumCol = nH;
		hd.fNumRow = (float)nW;
		hd.fNumCol = (float)nH;
		hd.pixelResX = (float)_mm2micron(m_resolX) * (float)nDivide;
		hd.pixelResY = (float)_mm2micron(m_resolY) * (float)nDivide;
		hd.zResolX = (float)_mm2micron(m_resolX) * (float)nDivide;
		hd.zResolY = (float)_mm2micron(m_resolY) * (float)nDivide;
		alpf_save_ptt(fullPath, &hd, clipzmap, img32);
	}

	Delete_1DArray(&img32);
	Delete_1DArray(&clipzmap);
	Delete_1DArray(&clipImgR);
	Delete_1DArray(&clipImgG);
	Delete_1DArray(&clipImgB);
	Delete_1DArray(&clipImgBB);
	Delete_1DArray(&clipImgBR);
}
int FOREIGN_INSP::Save_JpegImage(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, CString  file_path, int nJpegQuality)
{
	return m_JP.PIL_Save_Jpeg(ucSrc, width, height, nChannel, imColor, file_path, nJpegQuality);
}
double FOREIGN_INSP::GetResizeFactor(int viewX, int viewY, int imgX, int imgY, BOOL rotate)
{
	double factor = std::min((viewX * 1.0) / (imgX * 1.0), (viewY * 1.0) / (imgY * 1.0));
	return factor;
}
void FOREIGN_INSP::ResizeImg(UCHAR* userSrc, UCHAR* userDst, int roiSizeX, int roiSizeY, int sizeX, int sizeY, int bandSize, double factor)
{
	int nTYPE = CV_8UC1;
	if (bandSize == 3)
		nTYPE = CV_8UC3;
	else if (bandSize == 1)
		nTYPE = CV_8UC1;
	else
		return;

	cv::Mat imgSrc(roiSizeY, roiSizeX, CV_8UC1, userSrc);
	cv::Mat imgDst(sizeY, sizeX, CV_8UC1, userDst);
	cv::resize(imgSrc, imgDst, cv::Size(sizeX, sizeY));
}
CString FOREIGN_INSP::GetForeignDebugDataFullPath(int nFovCnt, CString csAddName, int nMode, int nFormat, bool bDEBUG)
{
	CString csForeignDebugDataFullPath = _T("");
	CString csRootPath = _T("");
	csRootPath.Format(_T("D:\\Eagle3D_data\\ForeignDebugData_M\\%d"), nFovCnt);
#if _DEBUG
	if (bDEBUG)
		csRootPath.Format(_T("D:\\Eagle3D_data\\ForeignDebugData\\DEBUG\\%d"), nFovCnt);
#endif
	CreateDirectory(csRootPath, NULL);
	CString csMode = _T("");
	if (nMode == m_eForeignInsp_Foreign)
		csMode = _T("Fre");
	else if (nMode == m_eForeignInsp_Foreign2D)
		csMode = _T("Fre2D");
	else if (nMode == m_eForeignInsp_Gray)
		csMode = _T("Sol");
	else if (nMode == m_eForeignInsp_GrayBub)
		csMode = _T("Bub");
	else if (nMode == m_eForeignInsp_Copper)
		csMode = _T("Cop");
	else if (nMode == m_eForeignInsp_Warpage)
		csMode = _T("War");
	else if (nMode == m_eForeignInsp_Total)
		csMode = _T("BTM");
	CString csFormat = _T("bmp");
	if (nFormat == 1) csFormat = _T("ptt");
	else if (nFormat == 2) csFormat = _T("ptr");
	CString csFullName = _T("");
	csFullName.Format(_T("%03d_%s_%s"), nFovCnt, csAddName, csMode);
	csForeignDebugDataFullPath.Format(_T("%s\\%s.%s"), csRootPath, csFullName, csFormat);
	return csForeignDebugDataFullPath;
}
#pragma endregion SAVE_NG

#pragma region COLOR
void FOREIGN_INSP::Get2DFOV(int projectionmode, const InspPartInfo *pInspBoardInfo, MPTI_InspectionMode vInspectionMod, cv::Mat *imgORG2D, int nDivide, int nFovID, bool bSave)
{
	CProcMil *procMil = g_pMPTI->GetProcMil();
	int sequence = g_pMPTI->GetSequence();
	int nFOVW = m_fovWidth;
	int nFOVH = m_fovLength;
	int nForeignW = nFOVW / nDivide - 1;
	int nForeignH = nFOVH / nDivide - 1;

	bool bHalf = nDivide == 2 ? true : false;
	cv::Mat imgFovORG(nFOVH, nFOVW, CV_8UC1);
	for (int nIDX = 0; nIDX < eM2C_NUM; nIDX++)
	{
		if (bHalf)
			imgORG2D[nIDX] = cv::Mat(nForeignH, nForeignW, CV_8UC1, m_ucForeignHalfBuffer[nIDX]);
		else
			imgORG2D[nIDX] = cv::Mat(nForeignH, nForeignW, CV_8UC1, m_ucForeignQuaterBuffer[nIDX]);

		bool bLoadDEBUG = false;
		bool bLoadDEBUG2D = false;
		void* pORG = NULL;
		CString csAddNameORG = _T("_Get2DForeign_FOV_TR");
		CString csAddName = _T("_Get2DForeign_TR");
		if (vInspectionMod == eMI_Inspection)
		{
			switch (nIDX)
			{
			case eM2C_TR:
				if (pInspBoardInfo->fovImgBuf.imgTop_R)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_R;
				break;
			case eM2C_TG:
				if (pInspBoardInfo->fovImgBuf.imgTop_G)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_G;
				break;
			case eM2C_TB:
				if (pInspBoardInfo->fovImgBuf.imgTop_B)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_B;
				break;
			case eM2C_TW:
				if (pInspBoardInfo->fovImgBuf.imgTop_W)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_W;
				break;
			case eM2C_MR:
				if (pInspBoardInfo->fovImgBuf.imgMiddle_R)
					pORG = pInspBoardInfo->fovImgBuf.imgMiddle_R;
				break;
			case eM2C_MB:
				if (pInspBoardInfo->fovImgBuf.imgMiddle_B)
					pORG = pInspBoardInfo->fovImgBuf.imgMiddle_B;
				break;
			case eM2C_BR:
				if (pInspBoardInfo->fovImgBuf.imgBottom_R)
					pORG = pInspBoardInfo->fovImgBuf.imgBottom_R;
				break;
			case eM2C_BB:
				if (pInspBoardInfo->fovImgBuf.imgBottom_B)
					pORG = pInspBoardInfo->fovImgBuf.imgBottom_B;
				break;
			}
		}
		else
		{
#if _DEBUG
			CString sDir;
			sDir.Format(_T("D:\\Eagle3D_data\\ForeignDebugData\\DEBUG\\%d"), nFovID);
			if (GetFileAttributes(sDir) != -1)
			{
				switch (nIDX)
				{
				case eM2C_TG:
					csAddNameORG = _T("_Get2DForeign_FOV_TG");
					csAddName = _T("_Get2DForeign_TG");
					break;
				case eM2C_TB:
					csAddNameORG = _T("_Get2DForeign_FOV_TB");
					csAddName = _T("_Get2DForeign_TB");
					break;
				case eM2C_TW:
					csAddNameORG = _T("_Get2DForeign_FOV_TW");
					csAddName = _T("_Get2DForeign_TW");
					break;
				case eM2C_MR:
					csAddNameORG = _T("_Get2DForeign_FOV_MR");
					csAddName = _T("_Get2DForeign_MR");
					break;
				case eM2C_MB:
					csAddNameORG = _T("_Get2DForeign_FOV_MB");
					csAddName = _T("_Get2DForeign_MB");
					break;
				case eM2C_BR:
					csAddNameORG = _T("_Get2DForeign_FOV_BR");
					csAddName = _T("_Get2DForeign_BR");
					break;
				case eM2C_BB:
					csAddNameORG = _T("_Get2DForeign_FOV_BB");
					csAddName = _T("_Get2DForeign_BB");
					break;
				}

				CString str = GetForeignDebugDataFullPath(nFovID, csAddNameORG, -1, 0, true);
				imgFovORG = cv::imread(std::string(CT2A(str)), cv::IMREAD_GRAYSCALE);
				bLoadDEBUG = !imgFovORG.empty();
				if (bLoadDEBUG == false)
					imgFovORG = cv::Mat(nFOVH, nFOVW, CV_8UC1);

				str = GetForeignDebugDataFullPath(nFovID, csAddName, -1, 0, true);
				cv::Mat imgTemp = cv::imread(std::string(CT2A(str)), cv::IMREAD_GRAYSCALE);
				if (imgTemp.empty() == false)
				{
					imgORG2D[nIDX] = imgTemp.clone();
					bLoadDEBUG2D = true;
				}
			}
			else
				pORG = (void*)g_pMPTI->GetSeqGrabBufID(sequence, 0, nIDX, projectionmode);
#else
			pORG = (void*)g_pMPTI->GetSeqGrabBufID(sequence, 0, nIDX, projectionmode);
#endif
		}

#if _DEBUG
		if (bLoadDEBUG2D)
			continue;
		if (bLoadDEBUG == false)
		{
			if (pORG == NULL)
				continue;

			if (procMil)
				procMil->GetClipBuff(pORG, imgFovORG.data, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
		}
#else
		if (pORG == NULL)
			continue;

		if (procMil)
			procMil->GetClipBuff(pORG, imgFovORG.data, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
#endif
		cv::resize(imgFovORG, imgORG2D[nIDX], cv::Size(nForeignW, nForeignH));

		if (bSave)
		{
			switch (nIDX)
			{
			case eM2C_TG:
				csAddNameORG = _T("_Get2DForeign_FOV_TG");
				csAddName = _T("_Get2DForeign_TG");
				break;
			case eM2C_TB:
				csAddNameORG = _T("_Get2DForeign_FOV_TB");
				csAddName = _T("_Get2DForeign_TB");
				break;
			case eM2C_TW:
				csAddNameORG = _T("_Get2DForeign_FOV_TW");
				csAddName = _T("_Get2DForeign_TW");
				break;
			case eM2C_MR:
				csAddNameORG = _T("_Get2DForeign_FOV_MR");
				csAddName = _T("_Get2DForeign_MR");
				break;
			case eM2C_MB:
				csAddNameORG = _T("_Get2DForeign_FOV_MB");
				csAddName = _T("_Get2DForeign_MB");
				break;
			case eM2C_BR:
				csAddNameORG = _T("_Get2DForeign_FOV_BR");
				csAddName = _T("_Get2DForeign_BR");
				break;
			case eM2C_BB:
				csAddNameORG = _T("_Get2DForeign_FOV_BB");
				csAddName = _T("_Get2DForeign_BB");
				break;
			}

			CString str = GetForeignDebugDataFullPath(nFovID, csAddNameORG);
			cv::imwrite(std::string(CT2A(str)), imgFovORG);
			str = GetForeignDebugDataFullPath(nFovID, csAddName);
			cv::imwrite(std::string(CT2A(str)), imgORG2D[nIDX]);
		}
	}
}
void FOREIGN_INSP::Get2DForeign(InspForeignInfo sData, FR_Bin sFRBin, int nMode, const InspPartInfo *pInspBoardInfo, UCHAR* pucColor, MPTI_InspectionMode vInspectionMod, cv::Mat *imgORG2D, int nDivide, int nFovID, BOOL bSave, UCHAR* pucGray)
{
	int nFOVW = m_fovWidth;
	int nFOVH = m_fovLength;
	int nForeignW = nFOVW / nDivide - 1;
	int nForeignH = nFOVH / nDivide - 1;

	LightTypeBuf sLightImg;
	if (vInspectionMod == eMI_Inspection)
	{
		sLightImg.m_pucTRed = pInspBoardInfo->partImgBuf.imgTop_R;	//imgORG2D[eM2C_TR].data;
		sLightImg.m_pucTGreen = pInspBoardInfo->partImgBuf.imgTop_G;	// imgORG2D[eM2C_TG].data;
		sLightImg.m_pucTBlue = pInspBoardInfo->partImgBuf.imgTop_B;	// imgORG2D[eM2C_TB].data;
		sLightImg.m_pucTWhite = pInspBoardInfo->partImgBuf.imgTop_W;	// imgORG2D[eM2C_TW].data;
		sLightImg.m_pucMRed = pInspBoardInfo->partImgBuf.imgMiddle_R;	// imgORG2D[eM2C_MR].data;
		sLightImg.m_pucMGreen = NULL;
		sLightImg.m_pucMBlue = pInspBoardInfo->partImgBuf.imgMiddle_B;	// imgORG2D[eM2C_MB].data;
		sLightImg.m_pucMWhite = NULL;
		sLightImg.m_pucBRed = pInspBoardInfo->partImgBuf.imgBottom_R;	// imgORG2D[eM2C_BR].data;
		sLightImg.m_pucBGreen = NULL;
		sLightImg.m_pucBBlue = pInspBoardInfo->partImgBuf.imgBottom_B;	// imgORG2D[eM2C_BB].data;
		sLightImg.m_pucBWhite = NULL;
		sLightImg.m_nImgWidth = nForeignW;
		sLightImg.m_nImgHeight = nForeignH;
	}
	else
	{
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
		sLightImg.m_nImgWidth = nForeignW;
		sLightImg.m_nImgHeight = nForeignH;
	}

	cv::Mat imgC(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgG(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
	bool bC = sFRBin.UseData(FR_BIN_DT_Color);
	bool bG = sFRBin.UseData(FR_BIN_DT_Gray);
	if (bC)
		Get2DForeignColor(sData, sFRBin, nMode, sLightImg, imgC, nFovID, bSave);
	if (bG)
		Get2DForeignGray(sFRBin, nMode, sLightImg, imgG, nFovID, bSave, pucGray);

#if _DEBUG
	if (pucGray)
	{
		cv::Mat imgGray(nForeignH, nForeignW, CV_8UC1, pucGray);
	}
#endif

	if (bC && bG)
	{
		cv::Mat imgColorTemp(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
		cv::bitwise_and(imgC, imgG, imgColorTemp);
		memcpy(pucColor, imgColorTemp.data, sizeof(UCHAR)* nForeignW * nForeignH);
	}
	else if (bC)
		memcpy(pucColor, imgC.data, sizeof(UCHAR)* nForeignW * nForeignH);
	else if (bG)
		memcpy(pucColor, imgG.data, sizeof(UCHAR)* nForeignW * nForeignH);

	if (bSave)
	{
		cv::Mat imgTemp(nForeignH, nForeignW, CV_8UC1, pucColor);
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Get2DForeign"), nMode);
		cv::imwrite(std::string(CT2A(str)), imgTemp);
	}
}
POINTF FOREIGN_INSP::GetCIEPoint(cv::Mat*	pConvertMat, double dRed, double dGreen, double dBlue, int nPixelCount)
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
bool FOREIGN_INSP::pvProcPointInPolygon(POINTF* ptArray, int nArrayCts, POINTF point)
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
bool FOREIGN_INSP::IsBlack(double dR, double dG, double dB, int nThresholdType, double dMin, double dMax)
{
	bool	bDraw = true;
	double	dIntensity = (dR + dG + dB) / 3.0;
	switch (nThresholdType)
	{
	case ThresholdType::eUpper:
	{
		if (dIntensity < dMin)
			bDraw = false;
	}
	break;
	case ThresholdType::eLower:
	{
		if (dIntensity > dMax)
			bDraw = false;
	}
	break;
	case ThresholdType::eInner:
	{
		if (dIntensity < dMin || dIntensity > dMax)
			bDraw = false;
	}
	break;
	case ThresholdType::eOuter:
	{
		if (dIntensity > dMin && dIntensity < dMax)
			bDraw = false;
	}
	break;
	}
	return	bDraw;
}
void FOREIGN_INSP::Get2DForeignGray(FR_Bin sData, int nMode, LightTypeBuf sLightImg, cv::Mat imgColorTemp, int nFovID, BOOL bSave, UCHAR* pucGray)
{
	ForignLight sLight = sData.m_MixLight;
	InspLightType eType = sLight.m_eLightType;

	sLightImg.m_nImgCnt = (eType == User_Light) ? sLight.m_nLightCnt : 1;
	sLightImg.m_pnRedValue = g_pMManager->pem_new<int>(true, sLightImg.m_nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	sLightImg.m_pnGreenValue = g_pMManager->pem_new<int>(true, sLightImg.m_nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	sLightImg.m_pnBlueValue = g_pMManager->pem_new<int>(true, sLightImg.m_nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	sLightImg.m_pnWhiteValue = g_pMManager->pem_new<int>(true, sLightImg.m_nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	sLightImg.m_pnPosition = g_pMManager->pem_new<int>(true, sLightImg.m_nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	sLightImg.m_pnCalculation = g_pMManager->pem_new<int>(true, sLightImg.m_nImgCnt, (PCHAR)__FUNCTION__, __LINE__);

	for (int i = 0; i < sLightImg.m_nImgCnt; i++)
	{
		sLightImg.m_pnRedValue[i] = (eType == User_Light) ? sLight.m_nArrRedValue[i] : sLight.m_nRedValue;
		sLightImg.m_pnGreenValue[i] = (eType == User_Light) ? sLight.m_nArrGreenValue[i] : sLight.m_nGreenValue;
		sLightImg.m_pnBlueValue[i] = (eType == User_Light) ? sLight.m_nArrBlueValue[i] : sLight.m_nBlueValue;
		sLightImg.m_pnWhiteValue[i] = (eType == User_Light) ? sLight.m_nArrWhiteValue[i] : sLight.m_nWhiteValue;
		sLightImg.m_pnPosition[i] = (eType == User_Light) ? sLight.m_nArrLightPosition[i] : 0;
		sLightImg.m_pnCalculation[i] = (eType == User_Light) ? sLight.m_nArrCalculation[i] : 0;
	}

	int nForeignW = sLightImg.m_nImgWidth;
	int nForeignH = sLightImg.m_nImgHeight;
	cv::Mat imgSrc(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
	if (eType == Top_Light)	// TOP
	{
		g_pMPTI->RoiImageCompose_LT(sLightImg.m_pucTRed, sLightImg.m_pucTGreen, sLightImg.m_pucTBlue, sLightImg.m_pucTWhite,
			nForeignW * nForeignH, nForeignW, nForeignH,
			nForeignW * nForeignH, 0, 0, nForeignW, nForeignH,
			sLightImg.m_pnRedValue[0], sLightImg.m_pnGreenValue[0], sLightImg.m_pnBlueValue[0], sLightImg.m_pnWhiteValue[0], imgSrc.data);
	}
	else if (eType == Middle_Light)	// MIDDLE
	{
		g_pMPTI->RoiImageCompose_LT(sLightImg.m_pucMRed, NULL, sLightImg.m_pucMBlue, NULL,
			nForeignW * nForeignH, nForeignW, nForeignH,
			nForeignW * nForeignH, 0, 0, nForeignW, nForeignH,
			sLightImg.m_pnRedValue[0], sLightImg.m_pnGreenValue[0], sLightImg.m_pnBlueValue[0], sLightImg.m_pnWhiteValue[0], imgSrc.data);
	}
	else if (eType == Bottom_Light)	// maybe Bottom
	{
		g_pMPTI->RoiImageCompose_LT(sLightImg.m_pucBRed, NULL, sLightImg.m_pucBBlue, NULL,
			nForeignW * nForeignH, nForeignW, nForeignH,
			nForeignW * nForeignH, 0, 0, nForeignW, nForeignH,
			sLightImg.m_pnRedValue[0], sLightImg.m_pnGreenValue[0], sLightImg.m_pnBlueValue[0], sLightImg.m_pnWhiteValue[0], imgSrc.data);
	}
	else if (eType = User_Light)	// mix light
		g_pMPTI->FullImageClaculCompose(sLightImg, imgSrc.data);

	if (bSave)
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Get2DForeign_Gray"), nMode);
		cv::imwrite(std::string(CT2A(str)), imgSrc);
	}
#if _DEBUG
	CString sDir;
	sDir.Format(_T("D:\\Eagle3D_data\\ForeignDebugData\\DEBUG\\%d"), nFovID);
	if (GetFileAttributes(sDir) != -1)
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Get2DForeign_Gray"), nMode, 0, true);
		imgSrc = cv::imread(std::string(CT2A(str)), cv::IMREAD_GRAYSCALE);
	}
#endif

	if (pucGray)
		memcpy(pucGray, imgSrc.data, sizeof(UCHAR) * nForeignH * nForeignW);

	int nGrayMin = sData.m_nArr[FR_BIN_N_Min2D];
	int nGrayMax = sData.m_nArr[FR_BIN_N_Max2D];
	int nRange = sData.m_nArr[FR_BIN_N_RangeGray];

	if (nGrayMin < 0) nGrayMin = 0;
	if (nGrayMax < 0) nGrayMax = 0;
	if (nGrayMin > 255) nGrayMin = 255;
	if (nGrayMax > 255) nGrayMax = 255;
	if (nGrayMin > nGrayMax) nGrayMax = nGrayMin;

	switch (nRange)
	{
	case ETypeInspRange::eTypeRangeIn:
	{
		cv::Mat imgMin;
		cv::Mat imgMax;
		cv::threshold(imgSrc, imgMin, nGrayMin, 255, cv::THRESH_BINARY);
		cv::threshold(imgSrc, imgMax, nGrayMax, 255, cv::THRESH_BINARY_INV);
		cv::bitwise_and(imgMin, imgMax, imgColorTemp);
		break;
	}
	case ETypeInspRange::eTypeRangeOut:
	{
		cv::Mat imgMin;
		cv::Mat imgMax;
		cv::threshold(imgSrc, imgMin, nGrayMax, 255, cv::THRESH_BINARY);
		cv::threshold(imgSrc, imgMax, nGrayMin, 255, cv::THRESH_BINARY_INV);
		cv::bitwise_or(imgMin, imgMax, imgColorTemp);
		break;
	}
	case ETypeInspRange::eTypeRangeLower:
	{
		cv::threshold(imgSrc, imgColorTemp, nGrayMax, 255, cv::THRESH_BINARY_INV);
		break;
	}
	default:
	{
		cv::threshold(imgSrc, imgColorTemp, nGrayMin, 255, cv::THRESH_BINARY);
		break;
	}
	}
	if (bSave)
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Get2DForeign_Gray_Bin"), nMode);
		cv::imwrite(std::string(CT2A(str)), imgColorTemp);
	}

	sLightImg.Distory();
}
void FOREIGN_INSP::Get2DForeignColor(InspForeignInfo sData, FR_Bin sFRBin, int nMode, LightTypeBuf sLightImg, cv::Mat imgColorTemp, int nFovID, BOOL bSave)
{
	POINT ptPt[10][POLYGON_POINT_CNTS];
	int nOffsetX = CIE_OFFSETX;
	int nOffsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
	POINT ptSrcPt[POLYGON_POINT_CNTS];
	POINT ptDesPt[POLYGON_POINT_CNTS];
	ColorXYInfoForeign *sColorXY = sFRBin.m_ColorXYInfo;
	int nRoiCount = sFRBin.m_nArr[FR_BIN_N_RoiCount];
	int nForeignW = sLightImg.m_nImgWidth;
	int nForeignH = sLightImg.m_nImgHeight;
	double dFactorR = sData.m_fArrData[FR_F_FactorR];
	double dFactorG = sData.m_fArrData[FR_F_FactorG];
	double dFactorB = sData.m_fArrData[FR_F_FactorB];
	bool bInvert = sFRBin.UseData(FR_BIN_DT_ColorInvert);
	for (int i = 0; i < nRoiCount; i++)
	{
		for (int j = 0; j < POLYGON_POINT_CNTS; j++)
			ptSrcPt[j] = sFRBin.m_ptArrPolygon[i][j];

		RemakePoly(ptSrcPt, POLYGON_POINT_CNTS, nOffsetX, nOffsetY, ptDesPt);
		ptPt[i][0] = ptDesPt[0];
		ptPt[i][1] = ptDesPt[1];
		ptPt[i][2] = ptDesPt[2];
		ptPt[i][3] = ptDesPt[3];
		ptPt[i][4] = ptDesPt[4];
	}

	cv::Mat imgR(nForeignH, nForeignW, CV_8UC1, sLightImg.m_pucTRed);
	cv::Mat imgG(nForeignH, nForeignW, CV_8UC1, sLightImg.m_pucTGreen);
	cv::Mat imgB(nForeignH, nForeignW, CV_8UC1, sLightImg.m_pucTBlue);
	if (sFRBin.UseData(FR_BIN_DT_AngleColor))
	{
		InspRoiImgBuf sFOV;
		sFOV.imgTop_R = sLightImg.m_pucTRed;
		sFOV.imgTop_G = sLightImg.m_pucTGreen;
		sFOV.imgTop_B = sLightImg.m_pucTBlue;
		sFOV.imgTop_W = sLightImg.m_pucTWhite;
		sFOV.imgMiddle_R = sLightImg.m_pucMRed;
		sFOV.imgMiddle_B = sLightImg.m_pucMBlue;
		sFOV.imgBottom_R = sLightImg.m_pucBRed;
		sFOV.imgBottom_B = sLightImg.m_pucBBlue;
		sFOV.nImageSizeX = sLightImg.m_nImgWidth;
		sFOV.nImageSizeY = sLightImg.m_nImgHeight;
		cv::Mat imgRGB[FR_Find_RGB_Total];
		imgRGB[FR_Find_RGB_TopR] = imgR;
		imgRGB[FR_Find_RGB_TopG] = imgG;
		imgRGB[FR_Find_RGB_TopB] = imgB;
		GetAngleColorFR(sFOV, sFRBin.m_sLightData, imgRGB);
		dFactorR = 1.0;
		dFactorG = 1.0;
		dFactorB = 1.0;
	}

	CompareCIE_Foreign(imgR.data, imgG.data, imgB.data, nForeignW, nForeignH,
		dFactorR, dFactorG, dFactorB, nRoiCount, ptPt, sColorXY, imgColorTemp.data, bInvert);

	if (bSave)
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Get2DForeign_Color"), nMode);
		cv::imwrite(std::string(CT2A(str)), imgColorTemp);
	}
}
void FOREIGN_INSP::RemakePoly(POINT* srcPoint, int count, int offsetX, int offsetY, POINT* retPoint)
{
	if (retPoint == NULL)
		return;

	for (int i = 0; i < count; i++)
	{
		retPoint[i].x = srcPoint[i].x + offsetX;
		retPoint[i].y = srcPoint[i].y + offsetY;

		if (retPoint[i].x < 0)
			retPoint[i].x = 0;

		if (retPoint[i].y < 0)
			retPoint[i].y = 0;


		if (retPoint[i].x > SIZE_CLRX - 1)
			retPoint[i].x = SIZE_CLRX - 1;

		if (retPoint[i].y > SIZE_CLRY - 1)
			retPoint[i].y = SIZE_CLRY - 1;
	}
}
void FOREIGN_INSP::GetAngleColorFR(InspRoiImgBuf sLightImg, lightData* sLightData, cv::Mat* img)
{
	LightTypeBuf sLightTypeBuf;
	sLightTypeBuf.m_pucTRed = sLightImg.imgTop_R;
	sLightTypeBuf.m_pucTGreen = sLightImg.imgTop_G;
	sLightTypeBuf.m_pucTBlue = sLightImg.imgTop_B;
	sLightTypeBuf.m_pucTWhite = sLightImg.imgTop_W;

	sLightTypeBuf.m_pucMRed = sLightImg.imgMiddle_R;
	sLightTypeBuf.m_pucMGreen = NULL;
	sLightTypeBuf.m_pucMBlue = sLightImg.imgMiddle_B;
	sLightTypeBuf.m_pucMWhite = NULL;

	sLightTypeBuf.m_pucBRed = sLightImg.imgBottom_R;
	sLightTypeBuf.m_pucBGreen = NULL;
	sLightTypeBuf.m_pucBBlue = sLightImg.imgBottom_B;
	sLightTypeBuf.m_pucBWhite = NULL;

	sLightTypeBuf.m_nImgWidth = sLightImg.nImageSizeX;
	sLightTypeBuf.m_nImgHeight = sLightImg.nImageSizeY;
	sLightTypeBuf.m_dROIX = 0;
	sLightTypeBuf.m_dROIY = 0;
	sLightTypeBuf.m_nROIImgWidth = sLightTypeBuf.m_nImgWidth;
	sLightTypeBuf.m_nROIImgHeight = sLightTypeBuf.m_nImgHeight;

	sLightTypeBuf.m_pnRedValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnGreenValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnBlueValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnWhiteValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnPosition = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnCalculation = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);

	for (int a = 0; a < FR_Find_RGB_Total; a++)
		GetAngleColorFR(sLightTypeBuf, sLightData[a], img[a]);

	Delete_1DArray(&sLightTypeBuf.m_pnRedValue);
	Delete_1DArray(&sLightTypeBuf.m_pnGreenValue);
	Delete_1DArray(&sLightTypeBuf.m_pnBlueValue);
	Delete_1DArray(&sLightTypeBuf.m_pnWhiteValue);
	Delete_1DArray(&sLightTypeBuf.m_pnPosition);
	Delete_1DArray(&sLightTypeBuf.m_pnCalculation);

#if _DEBUG
	cv::Mat imgRGB;
	cv::merge(img, 3, imgRGB);
#endif
}
void FOREIGN_INSP::GetAngleColorFR(LightTypeBuf sLightTypeBuf, lightData sLightData, cv::Mat img)
{
	sLightTypeBuf.m_nImgCnt = sLightData.m_byLightCnt;
	memset(sLightTypeBuf.m_pnRedValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightTypeBuf.m_pnGreenValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightTypeBuf.m_pnBlueValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightTypeBuf.m_pnWhiteValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightTypeBuf.m_pnPosition, 0, LIGHT_CNT * sizeof(int));
	memset(sLightTypeBuf.m_pnCalculation, 0, LIGHT_CNT * sizeof(int));
	for (int b = 0; b < LIGHT_CNT; b++)
	{
		sLightTypeBuf.m_pnRedValue[b] = sLightData.m_byArrLightData[eLightData_Red][b];
		sLightTypeBuf.m_pnGreenValue[b] = sLightData.m_byArrLightData[eLightData_Green][b];
		sLightTypeBuf.m_pnBlueValue[b] = sLightData.m_byArrLightData[eLightData_Blue][b];
		sLightTypeBuf.m_pnWhiteValue[b] = sLightData.m_byArrLightData[eLightData_White][b];
		sLightTypeBuf.m_pnPosition[b] = sLightData.m_byArrLightData[eLightData_Position][b];
		sLightTypeBuf.m_pnCalculation[b] = sLightData.m_byArrLightData[eLightData_Calculation][b];
	}
	g_pMPTI->ROIImageClaculCompose(sLightTypeBuf, img.data);
}
double FOREIGN_INSP::CompareCIE_Foreign(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, int width, int length,
	float factorR, float factorG, float factorB, int ptPolyCnt, POINT pts[10][POLYGON_POINT_CNTS], ColorXYInfoForeign* vColorXYInfoForeign, UCHAR* MaskImg, bool bInvert)
{
	double ret = 0.0;
	if (srcR == NULL || srcG == NULL || srcB == NULL || width <= 0 || length <= 0)
		return ret;

	bool bCalcFactor = true;
	if (factorR == 1.f && factorG == 1.f && factorB == 1.f)
		bCalcFactor = false;

	//후보 영역
	cv::Mat arrImgPoly[10];
	cv::Mat arrImgPolyPerpect[10];
	if (PolygonAreaCalc())
	{
		cv::Scalar white(255, 255, 255);
		for (int i = 0; i < ptPolyCnt; i++)
		{
			arrImgPoly[i] = cv::Mat(SIZE_CLRY, SIZE_CLRX, CV_8UC1, cv::Scalar(0));
			arrImgPolyPerpect[i] = cv::Mat(SIZE_CLRY, SIZE_CLRX, CV_8UC1, cv::Scalar(0));

#if _DEBUG
			cv::Mat imgPoly = arrImgPoly[i];
			cv::Mat imgPolyTemp = arrImgPolyPerpect[i];
#endif
			POINTF arrPo[POLYGON_POINT_CNTS];
			for (int a = 0; a < POLYGON_POINT_CNTS; a++)
			{
				arrPo[a].x = pts[i][a].x;
				arrPo[a].y = pts[i][a].y;
			}
			FillPolygon(arrImgPoly[i], arrPo, POLYGON_POINT_CNTS, white);
			MorErode(arrImgPoly[i], arrImgPolyPerpect[i]);
		}
	}

	cv::Mat imgR(length, width, CV_8UC1, srcR);
	cv::Mat imgG(length, width, CV_8UC1, srcG);
	cv::Mat imgB(length, width, CV_8UC1, srcB);

	POINT tempPt;
	int ptPolyNum = POLYGON_POINT_CNTS;
	long long size = width * length;
	int nLimitX = SIZE_CLRX - 1;
	int nLimitY = SIZE_CLRY - 1;
	float fMax = 255.f;
	for (long long j = 0; j < size; j++)
	{
		float r = (float)srcR[j], g = (float)srcG[j], b = (float)srcB[j];

		if (bCalcFactor)
		{
			r = _limit_Max(r * factorR, fMax);
			g = _limit_Max(g * factorG, fMax);
			b = _limit_Max(b * factorB, fMax);
		}

		float cx = ((CIE_XR * r) + (CIE_XG * g) + (CIE_XB * b));
		float cy = ((CIE_YR * r) + (CIE_YG * g) + (CIE_YB * b));
		float cz = ((CIE_ZR * r) + (CIE_ZG * g) + (CIE_ZB * b));
		float luminance = r + g + b;
		float sumXYZ = cx + cy + cz;

		int nx = SIZE_CLRX / 3;  // 0.333 * SIZE
		int ny = SIZE_CLRY / 3;
		if (sumXYZ >= CIE_XYZ && luminance >= CIE_RGB)
		{
			nx = _limit_value((int)(cx / sumXYZ * (float)SIZE_CLRX), 0.f, nLimitX);
			ny = _limit_value((int)(cy / sumXYZ * (float)SIZE_CLRY), 0.f, nLimitY);
		}
		tempPt.x = nx;
		tempPt.y = SIZE_CLRY - ny;
		int nindex = (SIZE_CLRX * tempPt.y) + tempPt.x;

		bool bIsWhite = false;
		for (int i = 0; i < ptPolyCnt; i++)
		{
			bool bGray = true;
			if (vColorXYInfoForeign[i].m_bUseThreshold)
				bGray = IsBlack(r, g, b, vColorXYInfoForeign[i].m_vThresholdType, vColorXYInfoForeign[i].m_nMin, vColorXYInfoForeign[i].m_nMax);

			bool bColor = false;
			if (PolygonAreaCalc())
			{
#if _DEBUG
				cv::Mat imgPoly = arrImgPoly[i];
				cv::Mat imgPolyTemp = arrImgPolyPerpect[i];
#endif
				if (arrImgPoly[i].data[nindex] == 255)
				{
					if (arrImgPolyPerpect[i].data[nindex] == 255)
						bColor = true;
					else
						bColor = PtInPolygonForeign(tempPt, pts[i], ptPolyNum);
				}
			}
			else
				bColor = PtInPolygonForeign(tempPt, pts[i], ptPolyNum);

			if (bGray && bColor)
			{
				bIsWhite = true;
				break;
			}
		}

		if (bInvert)
			MaskImg[j] = (bIsWhite) ? 0 : 255;
		else
			MaskImg[j] = (bIsWhite) ? 255 : 0;
	}
	return ret;
}
bool FOREIGN_INSP::PtInPolygonForeign(POINT pt, POINT *pts, int ptNum)
{
	POINT spt, ept;				// start, end point
	bool oddNodes = false;		// false: even(out), true: odd(in)

	for (int i = 0; i < ptNum; i++)
	{
		spt = pts[i];
		ept = pts[(i + 1) % ptNum];		// % prevent over index

		if (((spt.y <= pt.y) && (ept.y > pt.y))			// an upward crossing
			|| ((spt.y > pt.y) && (ept.y <= pt.y)))	// a downward crossing
		{
			// pt.x is in left of edge
			oddNodes ^= ((float)pt.x < ((float)spt.x + ((float)(pt.y - spt.y) / (float)(ept.y - spt.y) * (float)(ept.x - spt.x))));
		}
	}

	return oddNodes;
}
void FOREIGN_INSP::Get2DFOV_Resize(InspRoiImgBuf sFov, FR2DData pfRst)
{
	if (pfRst.m_nW <= 0 || pfRst.m_nH <= 0 ||
		pfRst.m_pFr1 == NULL || pfRst.m_pFr2 == NULL ||
		pfRst.m_pFr3 == NULL || pfRst.m_pFr4 == NULL)
		return;
	if (pfRst.m_pucTR == NULL || pfRst.m_pucTG == NULL || pfRst.m_pucTB == NULL ||
		pfRst.m_pucBR == NULL || pfRst.m_pucBB == NULL)
		return;

	int nFovID = pfRst.m_nFov;
	int nForeignW = pfRst.m_nW;
	int nForeignH = pfRst.m_nH;

	InspForeignInfo sForeign = m_ForeignData;
	int nFOVW = sFov.nImageSizeX;
	int nFOVH = sFov.nImageSizeY;
	int roiArea = nForeignW * nForeignH;
	BOOL bSave = sForeign.UseData(FR_DATA_DebugData);
	cv::Mat imgORG2D[eM2C_NUM];
#if _DEBUG
	cv::Mat imgFr1(nForeignH, nForeignW, CV_8UC1, pfRst.m_pFr1);
	cv::Mat imgFr2(nForeignH, nForeignW, CV_8UC1, pfRst.m_pFr2);
	cv::Mat imgFr3(nForeignH, nForeignW, CV_8UC1, pfRst.m_pFr3);
	cv::Mat imgFr4(nForeignH, nForeignW, CV_8UC1, pfRst.m_pFr4);

	cv::Mat imgFrTR(nForeignH, nForeignW, CV_8UC1, pfRst.m_pucTR);
	cv::Mat imgFrTG(nForeignH, nForeignW, CV_8UC1, pfRst.m_pucTG);
	cv::Mat imgFrTB(nForeignH, nForeignW, CV_8UC1, pfRst.m_pucTB);
	cv::Mat imgFrBR(nForeignH, nForeignW, CV_8UC1, pfRst.m_pucBR);
	cv::Mat imgFrBB(nForeignH, nForeignW, CV_8UC1, pfRst.m_pucBB);
#endif
	for (int nIDX = 0; nIDX < eM2C_NUM; nIDX++)
	{
		UCHAR* pORG = NULL;
		switch (nIDX)
		{
		case eM2C_TR:
			if (sFov.imgTop_R)
				pORG = sFov.imgTop_R;
			break;
		case eM2C_TG:
			if (sFov.imgTop_G)
				pORG = sFov.imgTop_G;
			break;
		case eM2C_TB:
			if (sFov.imgTop_B)
				pORG = sFov.imgTop_B;
			break;
		case eM2C_TW:
			if (sFov.imgTop_W)
				pORG = sFov.imgTop_W;
			break;
		case eM2C_MR:
			if (sFov.imgMiddle_R)
				pORG = sFov.imgMiddle_R;
			break;
		case eM2C_MB:
			if (sFov.imgMiddle_B)
				pORG = sFov.imgMiddle_B;
			break;
		case eM2C_BR:
			if (sFov.imgBottom_R)
				pORG = sFov.imgBottom_R;
			break;
		case eM2C_BB:
			if (sFov.imgBottom_B)
				pORG = sFov.imgBottom_B;
			break;
		}

		if (pORG == NULL)
			continue;

		cv::Mat imgFovORG(nFOVH, nFOVW, CV_8UC1, pORG);
		imgORG2D[nIDX] = cv::Mat(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
		cv::resize(imgFovORG, imgORG2D[nIDX], cv::Size(nForeignW, nForeignH));

		if (nIDX == eM2C_TR) memcpy(pfRst.m_pucTR, imgORG2D[nIDX].data, sizeof(UCHAR)* roiArea);
		else if (nIDX == eM2C_TG) memcpy(pfRst.m_pucTG, imgORG2D[nIDX].data, sizeof(UCHAR)* roiArea);
		else if (nIDX == eM2C_TB) memcpy(pfRst.m_pucTB, imgORG2D[nIDX].data, sizeof(UCHAR)* roiArea);
		else if (nIDX == eM2C_BR) memcpy(pfRst.m_pucBR, imgORG2D[nIDX].data, sizeof(UCHAR)* roiArea);
		else if (nIDX == eM2C_BB) memcpy(pfRst.m_pucBB, imgORG2D[nIDX].data, sizeof(UCHAR)* roiArea);

		if (bSave)
		{
			CString csAddNameORG = _T("_Get2DForeign_FOV_TR");
			CString csAddName = _T("_Get2DForeign_TR");
			switch (nIDX)
			{
			case eM2C_TG:
				csAddNameORG = _T("_Get2DForeign_FOV_TG");
				csAddName = _T("_Get2DForeign_TG");
				break;
			case eM2C_TB:
				csAddNameORG = _T("_Get2DForeign_FOV_TB");
				csAddName = _T("_Get2DForeign_TB");
				break;
			case eM2C_TW:
				csAddNameORG = _T("_Get2DForeign_FOV_TW");
				csAddName = _T("_Get2DForeign_TW");
				break;
			case eM2C_MR:
				csAddNameORG = _T("_Get2DForeign_FOV_MR");
				csAddName = _T("_Get2DForeign_MR");
				break;
			case eM2C_MB:
				csAddNameORG = _T("_Get2DForeign_FOV_MB");
				csAddName = _T("_Get2DForeign_MB");
				break;
			case eM2C_BR:
				csAddNameORG = _T("_Get2DForeign_FOV_BR");
				csAddName = _T("_Get2DForeign_BR");
				break;
			case eM2C_BB:
				csAddNameORG = _T("_Get2DForeign_FOV_BB");
				csAddName = _T("_Get2DForeign_BB");
				break;
			}
			CString str = GetForeignDebugDataFullPath(nFovID, csAddNameORG);
			cv::imwrite(std::string(CT2A(str)), imgFovORG);
			str = GetForeignDebugDataFullPath(nFovID, csAddName);
			cv::imwrite(std::string(CT2A(str)), imgORG2D[nIDX]);
		}
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
	sLightImg.m_nImgWidth = nForeignW;
	sLightImg.m_nImgHeight = nForeignH;

	for (int nMode = m_eForeignInsp_Foreign; nMode < m_eForeignInsp_Total; nMode++)
	{
		if (nMode == m_eForeignInsp_Foreign && pfRst.m_pFr1)
			memset(pfRst.m_pFr1, 0, sizeof(UCHAR) * roiArea);
		else if (nMode == m_eForeignInsp_Foreign2D && pfRst.m_pFr2)
			memset(pfRst.m_pFr2, 0, sizeof(UCHAR) * roiArea);
		else if (nMode == m_eForeignInsp_Gray && pfRst.m_pFr3)
			memset(pfRst.m_pFr3, 0, sizeof(UCHAR) * roiArea);
		else if (nMode == m_eForeignInsp_Copper && pfRst.m_pFr4)
			memset(pfRst.m_pFr4, 0, sizeof(UCHAR) * roiArea);

		if (nMode == m_eForeignInsp_GrayBub || nMode == m_eForeignInsp_Warpage)
			continue;

		if (sForeign.IsForeignEnable(nMode) == false)
			continue;

		FR_Bin sFRBin = sForeign.sBin[nMode];
		BOOL bUse2D = sFRBin.UseData(FR_BIN_DT_Color) || sFRBin.UseData(FR_BIN_DT_Gray);
		if (bUse2D == FALSE)
			continue;

		cv::Mat imgColor(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgC(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgG(nForeignH, nForeignW, CV_8UC1, cv::Scalar(0));
		bool bC = sFRBin.UseData(FR_BIN_DT_Color);
		bool bG = sFRBin.UseData(FR_BIN_DT_Gray);
		if (bC)
			Get2DForeignColor(sForeign, sFRBin, nMode, sLightImg, imgC, nFovID, bSave);
		if (bG)
			Get2DForeignGray(sFRBin, nMode, sLightImg, imgG, nFovID, bSave);

		if (bC && bG)
			cv::bitwise_and(imgC, imgG, imgColor);
		else if (bC)
			memcpy(imgColor.data, imgC.data, sizeof(UCHAR)* roiArea);
		else if (bG)
			memcpy(imgColor.data, imgG.data, sizeof(UCHAR)* roiArea);

		if (nMode == m_eForeignInsp_Foreign && pfRst.m_pFr1)
			memcpy(pfRst.m_pFr1, imgColor.data, roiArea);
		else if (nMode == m_eForeignInsp_Foreign2D && pfRst.m_pFr2)
			memcpy(pfRst.m_pFr2, imgColor.data, roiArea);
		else if (nMode == m_eForeignInsp_Gray && pfRst.m_pFr3)
			memcpy(pfRst.m_pFr3, imgColor.data, roiArea);
		else if (nMode == m_eForeignInsp_Copper && pfRst.m_pFr4)
			memcpy(pfRst.m_pFr4, imgColor.data, roiArea);

		if (bSave)
		{
			CString str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_Color_Blob"), nMode);
			cv::imwrite(std::string(CT2A(str)), imgColor);
		}
	}
}
int FOREIGN_INSP::SetRGB(UCHAR * ucArrDstImg, long long nIndex, float fH, float fMin, float fMax)
{
	float fMaxTemp = fMax * 1.15f;
	float fDivTemp = CR_MAP_SIZE;
	float fDivZ = (fMaxTemp - fMin) / fDivTemp;
	if (fDivZ <= 0)
		return 0;

	int crId = (fH - fMin) / fDivZ;
	if (crId < 0) crId = 0;
	if (crId >= CR_MAP_SIZE) crId = CR_MAP_SIZE - 1;

	ucArrDstImg[nIndex] = m_crPseudo[crId].b;
	ucArrDstImg[nIndex + 1] = m_crPseudo[crId].g;
	ucArrDstImg[nIndex + 2] = m_crPseudo[crId].r;

	return 1;
}
float FOREIGN_INSP::GetRGBH(UCHAR ucR, UCHAR ucG, UCHAR ucB, double dMin, double dMax)
{
	double dMaxTemp = dMax * 1.15;
	double dDivTemp = CR_MAP_SIZE;
	double dDivZ = (dMaxTemp - dMin) / dDivTemp;
	if (dDivZ <= 0)
		return 0.0f;

	if (ucR <= 50 && ucG <= 50 && ucB <= 50)
		return 0.0f;

	int nIndex = GetRGB_Index(ucR, ucG, ucB);
	if (nIndex < 0 || nIndex >= CR_MAP_SIZE)
		nIndex = GetRGB_Index_Find(ucR, ucG, ucB);
	if (nIndex < 0 || nIndex >= CR_MAP_SIZE)
		return 0.0f;

	float fH = (nIndex * dDivZ) + dMin;
	return fH;
}
int FOREIGN_INSP::GetRGB_Index(UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	int nIndex = -1;
	if (ucR == 0 && ucG == 0) // 0 ~ 127
	{
		if (ucB < 128)
			nIndex = 0;
		else
			nIndex = ucB - 128;
	}
	else if (ucR == 0 && ucB == 255) // 128 ~ 382
	{
		nIndex = 127 + ucG;
	}
	else if (ucR == 0 && ucG == 255) // 383 ~ 637
	{
		nIndex = 383 + (254 - ucB);
	}
	else if (ucG == 255 && ucB == 0) // 638 ~ 892
	{
		nIndex = 637 + ucR;
	}
	else if (ucR == 255 && ucB == 0) // 893 ~ 1147
	{
		nIndex = 893 + (254 - ucG);
	}
	else if (ucG == 0 && ucB == 0) // 1147 ~ 1279
	{
		if (ucR < 123)
			nIndex = CR_MAP_SIZE - 1;
		else
			nIndex = 1147 + (255 - ucR);
	}

	return nIndex;
}
int FOREIGN_INSP::GetRGB_Index_Find(UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	if (ucR > 0 && ucG > 0 && ucB > 0)
	{
		if (ucR > ucG)
		{
			if (ucG > ucB)
				ucB = 0;
			else
				ucG = 0;
		}
		else
		{
			if (ucR > ucB)
				ucB = 0;
			else
				ucR = 0;
		}
	}

	if (ucR > 0 && ucG > 0)
	{
		if (ucR != 255 && ucG != 255)
		{
			if (ucR > ucG)
				ucR = 255;
			else
				ucG = 255;
		}
	}
	else if (ucB > 0 && ucG > 0)
	{
		if (ucB != 255 && ucG != 255)
		{
			if (ucB > ucG)
				ucB = 255;
			else
				ucG = 255;
		}
	}
	else if (ucR > 0 && ucB > 0)
	{
		if (ucB > ucR)
			ucR = 0;
		else
			ucR = 0;
	}

	return GetRGB_Index(ucR, ucG, ucB);
}
int FOREIGN_INSP::GetColorRGB(IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT* rcRGB)
{
	if (retFullImg == NULL || nWidth <= 0 || nHeight <= 0 || rcRGB == NULL)
		return eMR_FAIL;

	cv::Mat imgFull(nHeight, nWidth, CV_8UC3, retFullImg);
	cv::Mat imgRGB[FR_Find_RGB_Total];
	cv::split(imgFull, imgRGB);
	GetColorRGB(imgRGB[FR_Find_RGB_TopR].data, imgRGB[FR_Find_RGB_TopG].data, imgRGB[FR_Find_RGB_TopB].data, nWidth, nHeight, rcRGB);

	return eMR_SUCCESS;
}
int FOREIGN_INSP::GetColorRGB(UCHAR* imgR, UCHAR* imgG, UCHAR* imgB, int nW, int nH, RECT* rcRGB)
{
	if (rcRGB == NULL || imgR == NULL || imgG == NULL || imgB == NULL)
		return eMR_FAIL;

	if (nW <= 0 || nH <= 0)
		return eMR_FAIL;

	const int nRGB = 3;
	int nArrMin[nRGB] = { 255, 255, 255 };
	int nArrMax[nRGB] = { 0, 0, 0 };
	int nArrSum[nRGB] = { 0, 0, 0 };
	int nCnt = 0;
	for (int y = rcRGB[nRGB].top; y < rcRGB[nRGB].bottom; y++)
	{
		if (y < 0) continue;
		if (y >= nH) break;
		for (int x = rcRGB[nRGB].left; x < rcRGB[nRGB].right; x++)
		{
			if (x < 0) continue;
			if (x >= nW) break;
			long long nIndex = (y * nW) + x;
			int nArr[nRGB] = { imgR[nIndex], imgG[nIndex] , imgB[nIndex] };
			for (int a = 0; a < nRGB; a++)
			{
				if (nArrMin[a] > nArr[a])
					nArrMin[a] = nArr[a];
				if (nArrMax[a] < nArr[a])
					nArrMax[a] = nArr[a];
				nArrSum[a] += nArr[a];
			}
			nCnt++;
		}
	}

	for (int a = 0; a < nRGB; a++)
	{
		if (nCnt > 0)
		{
			rcRGB[a].left = nArrMin[a];
			rcRGB[a].right = nArrMax[a];
			if (nArrSum[a] > 0)
				rcRGB[a].top = (nArrSum[a] / nCnt);
		}
	}

	return eMR_SUCCESS;
}
int FOREIGN_INSP::FrFindArea(ForeignData sInspData, InspRoiImgBuf sFov, FR_Find sFind, RECT* rcROI)
{
	int nTotalCnt = 0;
	try
	{
		if (sInspData.m_pDesImg == NULL || sInspData.m_nWP_W <= 0 || sInspData.m_nWP_H <= 0 || rcROI == NULL || sInspData.m_pDesImgBin == NULL)
			return nTotalCnt;
		if (sFov.imgTop_R == NULL || sFov.imgTop_G == NULL || sFov.imgTop_B == NULL)
			return eMR_FAIL;
		if (sFov.nImageSizeX <= 0 || sFov.nImageSizeY <= 0)
			return eMR_FAIL;
		if (sInspData.m_rcFOV.right <= 0 || sInspData.m_rcFOV.bottom <= 0)
			return eMR_FAIL;

		InspForeignInfo sForeign = m_ForeignData;
		BOOL bFullMap = sFind.UseData(FR_Find_DT_FullMap);
		BOOL bSave = sFind.UseData(FR_Find_DT_Save);
		if (sForeign.UseData(FR_DATA_DebugData) || sForeign.UseData(FR_DATA_DebugData2))
			bSave = true;

		int nFovID = sInspData.m_nFovID;
		int nWidth = sInspData.m_nWP_W;
		int nHeight = sInspData.m_nWP_H;
		int nW = bFullMap ? nWidth : sFov.nImageSizeX;
		int nH = bFullMap ? nHeight : sFov.nImageSizeY;
		memset(sInspData.m_pDesImgBin, 0, sizeof(UCHAR) * nW * nH);
		cv::Mat imgFull(nHeight, nWidth, CV_8UC3, sInspData.m_pDesImg);

		cv::Mat imgBin(nH, nW, CV_8UC1, cv::Scalar(255));
		InspFRArea(sInspData, imgBin);
		if (bSave) SaveFind(_T("InspFRArea_0"), imgBin, nFovID);

		POINT ptPt[10][POLYGON_POINT_CNTS];
		int nOffsetX = CIE_OFFSETX;
		int nOffsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
		POINT ptSrcPt[POLYGON_POINT_CNTS];
		POINT ptDesPt[POLYGON_POINT_CNTS];
		FR_Bin sData = sFind.sBin;
		int nRoiCount = sData.m_nArr[FR_BIN_N_RoiCount];
		for (int i = 0; i < nRoiCount; i++)
		{
			for (int j = 0; j < POLYGON_POINT_CNTS; j++)
				ptSrcPt[j] = sData.m_ptArrPolygon[i][j];
			RemakePoly(ptSrcPt, POLYGON_POINT_CNTS, nOffsetX, nOffsetY, ptDesPt);
			ptPt[i][0] = ptDesPt[0];
			ptPt[i][1] = ptDesPt[1];
			ptPt[i][2] = ptDesPt[2];
			ptPt[i][3] = ptDesPt[3];
			ptPt[i][4] = ptDesPt[4];
		}
		bool bInvert = sData.UseData(FR_BIN_DT_ColorInvert);
		double dFactorR = sForeign.m_fArrData[FR_F_FactorR];
		double dFactorG = sForeign.m_fArrData[FR_F_FactorG];
		double dFactorB = sForeign.m_fArrData[FR_F_FactorB];
		ColorXYInfoForeign* vColorXYInfoForeign = sData.m_ColorXYInfo;

		cv::Mat imgBlob(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgBinC(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgRGB[FR_Find_RGB_Total];
		cv::Mat imgRGBBin[FR_Find_RGB_Total];

		if (bFullMap)
		{
			cv::split(imgFull, imgRGB);
		}
		else
		{
			imgRGB[FR_Find_RGB_TopR] = cv::Mat(sFov.nImageSizeY, sFov.nImageSizeX, CV_8UC1, sFov.imgTop_R);
			imgRGB[FR_Find_RGB_TopG] = cv::Mat(sFov.nImageSizeY, sFov.nImageSizeX, CV_8UC1, sFov.imgTop_G);
			imgRGB[FR_Find_RGB_TopB] = cv::Mat(sFov.nImageSizeY, sFov.nImageSizeX, CV_8UC1, sFov.imgTop_B);
			if (sData.UseData(FR_BIN_DT_AngleColor))
				GetAngleColorFR(sFov, sData.m_sLightData, imgRGB);
		}
		if (bSave) SaveFind(_T("Find_R_Gray"), imgRGB[FR_Find_RGB_TopR], nFovID);
		if (bSave) SaveFind(_T("Find_G_Gray"), imgRGB[FR_Find_RGB_TopG], nFovID);
		if (bSave) SaveFind(_T("Find_B_Gray"), imgRGB[FR_Find_RGB_TopB], nFovID);

		CompareCIE_Foreign(imgRGB[FR_Find_RGB_TopR].data, imgRGB[FR_Find_RGB_TopG].data, imgRGB[FR_Find_RGB_TopB].data, nW, nH, dFactorR, dFactorG, dFactorB, nRoiCount, ptPt, vColorXYInfoForeign, imgBinC.data, bInvert);
		cv::bitwise_and(imgBinC, imgBin, imgBin);
		if (bSave) SaveFind(_T("Find_Color_Bin"), imgBinC, nFovID);
		if (bSave) SaveFind(_T("InspFRArea_1"), imgBin, nFovID);

		for (int a = 0; a < FR_Find_RGB_Total; a++)
		{
			imgRGBBin[a] = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(255));
			GetGrayBin(imgRGB[a], sFind.m_ColorXYInfo[a], imgRGBBin[a]);
			cv::bitwise_and(imgRGBBin[a], imgBin, imgBin);
		}
		if (bSave) SaveFind(_T("Find_R_Bin"), imgRGBBin[FR_Find_RGB_TopR], nFovID);
		if (bSave) SaveFind(_T("Find_G_Bin"), imgRGBBin[FR_Find_RGB_TopG], nFovID);
		if (bSave) SaveFind(_T("Find_B_Bin"), imgRGBBin[FR_Find_RGB_TopB], nFovID);
		if (bSave) SaveFind(_T("InspFRArea_2"), imgBin, nFovID);

		int nMinArea = 10;
		BOOL bFillHole = TRUE;
		BOOL bCircle = sFind.UseData(FR_Find_DT_Circle) && (bFullMap == FALSE);
		float fCircle = sFind.m_fArr[FR_Find_F_Circle];

		BOOL bInspW = sFind.UseData(FR_Find_DT_Width);
		int nWMin = sInspData.vStickerArea[0].x;
		int nWMax = sInspData.vStickerArea[0].y;

		BOOL bInspL = sFind.UseData(FR_Find_DT_Length);
		int nLMin = sInspData.vStickerArea[1].x;
		int nLMax = sInspData.vStickerArea[1].y;
		if (bCircle)
		{
			bInspL = bInspW;
			nLMin = nWMin;
			nLMax = nWMax;
		}

		std::vector<std::vector<cv::Point>> allRst = CalcBlob(imgBin, imgBlob, nMinArea);
		int nCntBlob = allRst.size();
		if (bSave) SaveFind(_T("InspFRArea_Blob"), imgBlob, nFovID);
		if (nCntBlob <= 0)
		{
			UCHAR* pucBin = NULL;
			for (int a = 0; a < FR_Find_RGB_Total; a++)
			{
				if (sInspData.m_nMode == a)
				{
					if (sInspData.m_nBin == 1)
						pucBin = imgRGBBin[a].ptr<UCHAR>();
					else
						pucBin = imgRGB[a].ptr<UCHAR>();
					break;
				}
			}
			if (pucBin == NULL)
			{
				if (sInspData.m_nBin == 1)
					pucBin = imgBin.ptr<UCHAR>();
				else
					pucBin = imgBlob.ptr<UCHAR>();
			}

			if (pucBin == NULL)
				memset(sInspData.m_pDesImgBin, 0, sizeof(UCHAR) * nW * nH);
			else
				memcpy(sInspData.m_pDesImgBin, pucBin, sizeof(UCHAR) * nW * nH);

			return nTotalCnt;
		}

		cv::Mat imgBlobRst(nH, nW, CV_8UC1, cv::Scalar(0));
		std::vector<cv::Mat> vArrImg;
		for (int a = 0; a < nCntBlob; a++)
		{
			cv::Mat imgTemp = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
			cv::drawContours(imgTemp, allRst, a, cv::Scalar(255), cv::FILLED);
			cv::Rect blobRes = cv::boundingRect(imgTemp);
			CRect rcBlob(blobRes.x, blobRes.y, blobRes.br().x, blobRes.br().y);

			if (bInspW)
			{
				if (nWMin > blobRes.width || nWMax < blobRes.width)
					continue;
			}
			if (bInspL)
			{
				if (nLMin > blobRes.height || nLMax < blobRes.height)
					continue;
			}

			if (bCircle)
			{
#if _DEBUG
				cv::Rect rtClip(rcBlob.left, rcBlob.top, rcBlob.Width(), rcBlob.Height());
				cv::Mat imgClip(rtClip.height, rtClip.width, CV_8UC1, cv::Scalar(0));
				imgTemp(rtClip).copyTo(imgClip);
#endif

				float fResult(0.0);
				int nMaxIndex(0);
				std::vector<CRect> vBlobRects;
				vBlobRects.push_back(rcBlob);
				InspSolderBall(imgTemp, vBlobRects, &nMaxIndex, &fResult);
				vBlobRects.clear();

				if (fCircle > fResult * 100.0f)
				{
					vArrImg.push_back(imgTemp);
					continue;
				}
			}

			cv::bitwise_or(imgTemp, imgBlobRst, imgBlobRst);
			rcROI[nTotalCnt].left = rcBlob.left;
			rcROI[nTotalCnt].right = rcBlob.right;
			rcROI[nTotalCnt].top = rcBlob.top;
			rcROI[nTotalCnt].bottom = rcBlob.bottom;
			nTotalCnt++;

			if (nTotalCnt >= FR_Find_MAX)
				break;
		}

		if (bCircle && FR_Find_MAX > nTotalCnt)
		{
			cv::Mat imgBlobTemp(nH, nW, CV_8UC1, cv::Scalar(0));
			cv::Mat imgDilate(nH, nW, CV_8UC1, cv::Scalar(0));
			cv::Mat imgMorErode(nH, nW, CV_8UC1, cv::Scalar(0));
			for (cv::Mat imgTemp : vArrImg)
			{
				int nTemp = 20;
				memset(imgDilate.data, 0, sizeof(UCHAR) * nW * nH);
				cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nTemp, nTemp));
				cv::dilate(imgTemp, imgDilate, kernel);

				memset(imgMorErode.data, 0, sizeof(UCHAR) * nW * nH);
				MorErode(imgDilate, imgMorErode, 0, nTemp);

				std::vector<std::vector<cv::Point>> allRst2 = CalcBlob(imgMorErode, imgBlobTemp, nMinArea, true, true);
				if (allRst2.size() != 1)
					continue;

				cv::Rect roi2 = cv::boundingRect(allRst2[0]);
				CRect rcBlob(roi2.x, roi2.y, roi2.br().x, roi2.br().y);

				float fResult(0.0);
				int nMaxIndex(0);
				std::vector<CRect> vBlobRects;
				vBlobRects.push_back(rcBlob);
				InspSolderBall(imgBlobTemp, vBlobRects, &nMaxIndex, &fResult);
				vBlobRects.clear();

				if (fCircle > fResult * 100.0f)
					continue;

				cv::bitwise_or(imgTemp, imgBlobRst, imgBlobRst);
				rcROI[nTotalCnt].left = rcBlob.left;
				rcROI[nTotalCnt].right = rcBlob.right;
				rcROI[nTotalCnt].top = rcBlob.top;
				rcROI[nTotalCnt].bottom = rcBlob.bottom;
				nTotalCnt++;

				if (nTotalCnt >= FR_Find_MAX)
					break;
			}
		}

		UCHAR* pucBin = NULL;
		for (int a = 0; a < FR_Find_RGB_Total; a++)
		{
			if (sInspData.m_nMode == a)
			{
				if (sInspData.m_nBin == 1)
					pucBin = imgRGBBin[a].ptr<UCHAR>();
				else
					pucBin = imgRGB[a].ptr<UCHAR>();
				break;
			}
		}
		if (pucBin == NULL)
		{
			if (sInspData.m_nBin == 1)
				pucBin = imgBlobRst.ptr<UCHAR>();
			else
				pucBin = imgBlob.ptr<UCHAR>();
		}

		if (pucBin == NULL)
			memset(sInspData.m_pDesImgBin, 0, sizeof(UCHAR) * nW * nH);
		else
			memcpy(sInspData.m_pDesImgBin, pucBin, sizeof(UCHAR) * nW * nH);

		return nTotalCnt;
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
	return nTotalCnt;
}
void FOREIGN_INSP::SaveFind(CString cName, cv::Mat img, int nFovID, bool b3D)
{
	CString csFormat = b3D ? _T("ptr") : _T("bmp");
	CString csRootPath = _T("");
	csRootPath.Format(_T("D:\\Eagle3D_data\\ForeignDebugData_M\\%d"), nFovID);

	CString csFullName = _T("");
	csFullName.Format(_T("%s\\%s.%s"), csRootPath, cName, csFormat);

	CreateDirectory(csRootPath, NULL);

	if (b3D)
	{
		HEADER_PTR hd;
		hd.uiNumRow = img.cols;
		hd.uiNumCol = img.rows;
		hd.zResolX = m_resolX * 1000.0;
		hd.zResolY = m_resolY * 1000.0;
		hd.sizeBit = 32;
		alpf_save_ptr(csFullName, &hd, img.ptr<float>());
	}
	else
	{
		cv::imwrite(std::string(CT2A(csFullName)), img);
	}
}
void FOREIGN_INSP::GetGrayBin(cv::Mat imgGray, ColorXYInfoForeign vInfo, cv::Mat imgBin)
{
	if (vInfo.m_bUseThreshold == FALSE)
		return;

	int nGrayMin = vInfo.m_nMin;
	int nGrayMax = vInfo.m_nMax;
	if (nGrayMin < 0) nGrayMin = 0;
	if (nGrayMax < 0) nGrayMax = 0;
	if (nGrayMin > 255) nGrayMin = 255;
	if (nGrayMax > 255) nGrayMax = 255;
	if (nGrayMin > nGrayMax) nGrayMax = nGrayMin;

	switch (vInfo.m_vThresholdType)
	{
	case ThresholdType::eInner:
	{
		cv::Mat imgMin;
		cv::Mat imgMax;
		cv::threshold(imgGray, imgMin, nGrayMin, 255, cv::THRESH_BINARY);
		cv::threshold(imgGray, imgMax, nGrayMax, 255, cv::THRESH_BINARY_INV);
		cv::bitwise_and(imgMin, imgMax, imgBin);
		break;
	}
	case ThresholdType::eOuter:
	{
		cv::Mat imgMin;
		cv::Mat imgMax;
		cv::threshold(imgGray, imgMin, nGrayMax, 255, cv::THRESH_BINARY);
		cv::threshold(imgGray, imgMax, nGrayMin, 255, cv::THRESH_BINARY_INV);
		cv::bitwise_or(imgMin, imgMax, imgBin);
		break;
	}
	case ThresholdType::eLower:
	{
		cv::threshold(imgGray, imgBin, nGrayMax, 255, cv::THRESH_BINARY_INV);
		break;
	}
	default:
	{
		cv::threshold(imgGray, imgBin, nGrayMin, 255, cv::THRESH_BINARY);
		break;
	}
	}
}
double FOREIGN_INSP::GetColorArea(InspRoiImgBuf sFov, FR_Bin sData, RECT rcROI)
{
	double dArea = 0;
	try
	{
		if (sFov.imgTop_R == NULL || sFov.imgTop_G == NULL || sFov.imgTop_B == NULL)
			return dArea;
		if (sFov.nImageSizeX <= 0 || sFov.nImageSizeY <= 0)
			return dArea;

		int nClipL = rcROI.left;
		int nClipT = rcROI.top;
		int nClipW = rcROI.right - rcROI.left;
		int nClipH = rcROI.bottom - rcROI.top;
		if (nClipL < 0) nClipL = 0;
		if (nClipT < 0) nClipT = 0;
		if (nClipL + nClipW > sFov.nImageSizeX) nClipW = sFov.nImageSizeX - nClipL;
		if (nClipT + nClipH > sFov.nImageSizeY) nClipH = sFov.nImageSizeY - nClipT;
		if (nClipL >= sFov.nImageSizeX || nClipT >= sFov.nImageSizeY)
			return dArea;
		if (nClipW < 5 || nClipH < 5)
			return dArea;

		cv::Rect rtClip(nClipL, nClipT, nClipW, nClipH);

		cv::Mat imgOrgR(sFov.nImageSizeY, sFov.nImageSizeX, CV_8UC1, sFov.imgTop_R);
		cv::Mat imgOrgG(sFov.nImageSizeY, sFov.nImageSizeX, CV_8UC1, sFov.imgTop_G);
		cv::Mat imgOrgB(sFov.nImageSizeY, sFov.nImageSizeX, CV_8UC1, sFov.imgTop_B);
		if (sData.UseData(FR_BIN_DT_AngleColor))
		{
			cv::Mat imgRGB[FR_Find_RGB_Total];
			imgRGB[FR_Find_RGB_TopR] = imgOrgR;
			imgRGB[FR_Find_RGB_TopG] = imgOrgG;
			imgRGB[FR_Find_RGB_TopB] = imgOrgB;
			GetAngleColorFR(sFov, sData.m_sLightData, imgRGB);
		}
		cv::Mat imgR(nClipH, nClipW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgG(nClipH, nClipW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgB(nClipH, nClipW, CV_8UC1, cv::Scalar(0));
		imgOrgR(rtClip).copyTo(imgR);
		imgOrgG(rtClip).copyTo(imgG);
		imgOrgB(rtClip).copyTo(imgB);

		cv::Mat imgBin(nClipH, nClipW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgBlob(nClipH, nClipW, CV_8UC1, cv::Scalar(0));

		POINT ptPt[10][POLYGON_POINT_CNTS];
		int nOffsetX = CIE_OFFSETX;
		int nOffsetY = SIZE_CLRY - (CIE_OFFSETY + 256);
		POINT ptSrcPt[POLYGON_POINT_CNTS];
		POINT ptDesPt[POLYGON_POINT_CNTS];
		InspForeignInfo sForeign = m_ForeignData;
		int nRoiCount = sData.m_nArr[FR_BIN_N_RoiCount];
		for (int i = 0; i < nRoiCount; i++)
		{
			for (int j = 0; j < POLYGON_POINT_CNTS; j++)
				ptSrcPt[j] = sData.m_ptArrPolygon[i][j];
			RemakePoly(ptSrcPt, POLYGON_POINT_CNTS, nOffsetX, nOffsetY, ptDesPt);
			ptPt[i][0] = ptDesPt[0];
			ptPt[i][1] = ptDesPt[1];
			ptPt[i][2] = ptDesPt[2];
			ptPt[i][3] = ptDesPt[3];
			ptPt[i][4] = ptDesPt[4];
		}
		bool bInvert = sData.UseData(FR_BIN_DT_ColorInvert);
		double dFactorR = sForeign.m_fArrData[FR_F_FactorR];
		double dFactorG = sForeign.m_fArrData[FR_F_FactorG];
		double dFactorB = sForeign.m_fArrData[FR_F_FactorB];
		ColorXYInfoForeign* vColorXYInfoForeign = sData.m_ColorXYInfo;
		CompareCIE_Foreign(imgR.ptr<UCHAR>(), imgG.ptr<UCHAR>(), imgB.ptr<UCHAR>(), nClipW, nClipH, dFactorR, dFactorG, dFactorB, nRoiCount, ptPt, vColorXYInfoForeign, imgBin.data, bInvert);

		int nTotalCnt = std::count(imgBin.data, imgBin.data + (nClipW * nClipH), 255);
		if (nTotalCnt > 0)
			dArea = ((double)nTotalCnt / (double)(nClipW * nClipH)) * 100;
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
	return dArea;
}
void FOREIGN_INSP::Get2DFOV_Resize(int projectionmode, InspPartInfo *pInspBoardInfo, MPTI_InspectionMode vInspectionMod, int nDivide, int nFovID, BOOL bSave)
{
	CProcMil *procMil = g_pMPTI->GetProcMil();
	int sequence = g_pMPTI->GetSequence();
	int nFOVW = m_fovWidth;
	int nFOVH = m_fovLength;
	int nForeignW = nFOVW / nDivide - 1;
	int nForeignH = nFOVH / nDivide - 1;
	if (vInspectionMod != eMI_Inspection)
		nFovID = 0;

	InspForeignInfo sForeign = g_pInspMng->m_FR.GetForeignData();

	int roiArea = nForeignW * nForeignH;
	pInspBoardInfo->partImgBuf.nImageSizeX = nForeignW;
	pInspBoardInfo->partImgBuf.nImageSizeY = nForeignH;

	bool bHalf = nDivide == 2 ? true : false;

	cv::Mat imgFovORG(nFOVH, nFOVW, CV_8UC1);
	for (int nIDX = 0; nIDX < eM2C_NUM; nIDX++)
	{

		void* pORG = NULL;
		void* pResize = NULL;
		CString csAddNameORG = _T("_Get2DForeign_FOV_TR");
		CString csAddName = _T("_Get2DForeign_TR");
		if (vInspectionMod == eMI_Inspection)
		{
			switch (nIDX)
			{
			case eM2C_TR:
				if (pInspBoardInfo->fovImgBuf.imgTop_R)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_R;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_TR], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_R = m_ucForeignHalfOrgBuffer[eM2C_TR];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_TR], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_R = m_ucForeignQuaterOrgBuffer[eM2C_TR];
				}
				pResize = pInspBoardInfo->partImgBuf.imgTop_R;
				break;
			case eM2C_TG:
				if (pInspBoardInfo->fovImgBuf.imgTop_G)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_G;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_TG], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_G = m_ucForeignHalfOrgBuffer[eM2C_TG];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_TG], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_G = m_ucForeignQuaterOrgBuffer[eM2C_TG];
				}
				pResize = pInspBoardInfo->partImgBuf.imgTop_G;
				break;
			case eM2C_TB:
				if (pInspBoardInfo->fovImgBuf.imgTop_B)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_B;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_TB], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_B = m_ucForeignHalfOrgBuffer[eM2C_TB];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_TB], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_B = m_ucForeignQuaterOrgBuffer[eM2C_TB];
				}
				pResize = pInspBoardInfo->partImgBuf.imgTop_B;
				break;
			case eM2C_TW:
				if (pInspBoardInfo->fovImgBuf.imgTop_W)
					pORG = pInspBoardInfo->fovImgBuf.imgTop_W;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_TW], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_W = m_ucForeignHalfOrgBuffer[eM2C_TW];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_TW], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgTop_W = m_ucForeignQuaterOrgBuffer[eM2C_TW];
				}
				pResize = pInspBoardInfo->partImgBuf.imgTop_W;
				break;
			case eM2C_MR:
				if (pInspBoardInfo->fovImgBuf.imgMiddle_R)
					pORG = pInspBoardInfo->fovImgBuf.imgMiddle_R;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_MR], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgMiddle_R = m_ucForeignHalfOrgBuffer[eM2C_MR];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_MR], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgMiddle_R = m_ucForeignQuaterOrgBuffer[eM2C_MR];
				}
				pResize = pInspBoardInfo->partImgBuf.imgMiddle_R;
				break;
			case eM2C_MB:
				if (pInspBoardInfo->fovImgBuf.imgMiddle_B)
					pORG = pInspBoardInfo->fovImgBuf.imgMiddle_B;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_MB], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgMiddle_B = m_ucForeignHalfOrgBuffer[eM2C_MB];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_MB], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgMiddle_B = m_ucForeignQuaterOrgBuffer[eM2C_MB];
				}
				pResize = pInspBoardInfo->partImgBuf.imgMiddle_B;
				break;
			case eM2C_BR:
				if (pInspBoardInfo->fovImgBuf.imgBottom_R)
					pORG = pInspBoardInfo->fovImgBuf.imgBottom_R;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_BR], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgBottom_R = m_ucForeignHalfOrgBuffer[eM2C_BR];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_BR], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgBottom_R = m_ucForeignQuaterOrgBuffer[eM2C_BR];
				}
				pResize = pInspBoardInfo->partImgBuf.imgBottom_R;
				break;
			case eM2C_BB:
				if (pInspBoardInfo->fovImgBuf.imgBottom_B)
					pORG = pInspBoardInfo->fovImgBuf.imgBottom_B;
				if (bHalf)
				{
					memset(m_ucForeignHalfOrgBuffer[eM2C_BB], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgBottom_B = m_ucForeignHalfOrgBuffer[eM2C_BB];
				}
				else
				{
					memset(m_ucForeignQuaterOrgBuffer[eM2C_BB], 0, roiArea);
					pInspBoardInfo->partImgBuf.imgBottom_B = m_ucForeignQuaterOrgBuffer[eM2C_BB];
				}
				pResize = pInspBoardInfo->partImgBuf.imgBottom_B;
				break;
			}
		}
		else
			pORG = (void*)g_pMPTI->GetSeqGrabBufID(sequence, nFovID, nIDX, projectionmode);

		if (pORG == NULL)
			continue;
		if (procMil)
			procMil->GetClipBuff(pORG, imgFovORG.data, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
		cv::Mat imgFovResize = cv::Mat(nForeignH, nForeignW, CV_8UC1, pResize);

		cv::resize(imgFovORG, imgFovResize, cv::Size(nForeignW, nForeignH));

		if (bSave)
		{
			switch (nIDX)
			{
			case eM2C_TG:
				csAddNameORG = _T("_Get2DForeign_FOV_TG");
				csAddName = _T("_Get2DForeign_TG");
				break;
			case eM2C_TB:
				csAddNameORG = _T("_Get2DForeign_FOV_TB");
				csAddName = _T("_Get2DForeign_TB");
				break;
			case eM2C_TW:
				csAddNameORG = _T("_Get2DForeign_FOV_TW");
				csAddName = _T("_Get2DForeign_TW");
				break;
			case eM2C_MR:
				csAddNameORG = _T("_Get2DForeign_FOV_MR");
				csAddName = _T("_Get2DForeign_MR");
				break;
			case eM2C_MB:
				csAddNameORG = _T("_Get2DForeign_FOV_MB");
				csAddName = _T("_Get2DForeign_MB");
				break;
			case eM2C_BR:
				csAddNameORG = _T("_Get2DForeign_FOV_BR");
				csAddName = _T("_Get2DForeign_BR");
				break;
			case eM2C_BB:
				csAddNameORG = _T("_Get2DForeign_FOV_BB");
				csAddName = _T("_Get2DForeign_BB");
				break;
			}
			CString str = g_pInspMng->m_FR.GetForeignDebugDataFullPath(nFovID, csAddNameORG);
			cv::imwrite(std::string(CT2A(str)), imgFovORG);
			str = g_pInspMng->m_FR.GetForeignDebugDataFullPath(nFovID, csAddName);
			cv::imwrite(std::string(CT2A(str)), imgFovResize);
		}
	}

	cv::Mat imgColor(nForeignH, nForeignW, CV_8UC1);
	cv::Mat imgORG2D[eM2C_NUM];

	for (int i = 0; i < eM2C_NUM; i++)
	{
		if (bHalf)
			imgORG2D[i] = cv::Mat(nForeignH, nForeignW, CV_8UC1, m_ucForeignHalfBuffer[i]);
		else
			imgORG2D[i] = cv::Mat(nForeignH, nForeignW, CV_8UC1, m_ucForeignQuaterBuffer[i]);
	}

	for (int nMode = m_eForeignInsp_Foreign; nMode < m_eForeignInsp_Total; nMode++)
	{
		if (nMode == m_eForeignInsp_GrayBub || nMode == m_eForeignInsp_Warpage)
			continue;

		if (sForeign.IsForeignEnable(nMode) == false)
			continue;

		FR_Bin sData = sForeign.sBin[nMode];
		BOOL bUse2D = sData.UseData(FR_BIN_DT_Color) || sData.UseData(FR_BIN_DT_Gray);
		if (bUse2D == FALSE)
			continue;

		memset(imgColor.data, 0, sizeof(UCHAR) * nForeignH * nForeignW);
		g_pInspMng->m_FR.Get2DForeign(sForeign, sData, nMode, pInspBoardInfo, imgColor.data, vInspectionMod, imgORG2D, nDivide, nFovID, bSave);

		switch (nMode)
		{
		case m_eForeignInsp_Foreign:		//m_eForeignInsp_Foreign,
			if (bHalf)
			{
				memset(m_ucForeignHalfRstBuffer[0], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide1_R = m_ucForeignHalfRstBuffer[0];
			}
			else
			{
				memset(m_ucForeignQuaterRstBuffer[0], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide1_R = m_ucForeignQuaterRstBuffer[0];
			}
			memcpy(pInspBoardInfo->partImgBuf.imgSide1_R, imgColor.data, roiArea);
			break;
		case m_eForeignInsp_Foreign2D:		//m_eForeignInsp_Foreign2D,
			if (bHalf)
			{
				memset(m_ucForeignHalfRstBuffer[1], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide1_G = m_ucForeignHalfRstBuffer[1];
			}
			else
			{
				memset(m_ucForeignQuaterRstBuffer[1], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide1_G = m_ucForeignQuaterRstBuffer[1];
			}
			memcpy(pInspBoardInfo->partImgBuf.imgSide1_G, imgColor.data, roiArea);
			break;
		case m_eForeignInsp_Gray:		//m_eForeignInsp_Gray,
			if (bHalf)
			{
				memset(m_ucForeignHalfRstBuffer[2], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide1_B = m_ucForeignHalfRstBuffer[2];
			}
			else
			{
				memset(m_ucForeignQuaterRstBuffer[2], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide1_B = m_ucForeignQuaterRstBuffer[2];
			}
			memcpy(pInspBoardInfo->partImgBuf.imgSide1_B, imgColor.data, roiArea);
			break;
		case m_eForeignInsp_Copper:		//m_eForeignInsp_Copper
			if (bHalf)
			{
				memset(m_ucForeignHalfRstBuffer[3], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide2_R = m_ucForeignHalfRstBuffer[3];
			}
			else
			{
				memset(m_ucForeignQuaterRstBuffer[3], 0, roiArea);
				pInspBoardInfo->partImgBuf.imgSide2_R = m_ucForeignQuaterRstBuffer[3];
			}
			memcpy(pInspBoardInfo->partImgBuf.imgSide2_R, imgColor.data, roiArea);
			break;
		}

		if (bSave)
		{
			CString str = g_pInspMng->m_FR.GetForeignDebugDataFullPath(nFovID, _T("_Insp_Color_Blob"), nMode);
			cv::imwrite(std::string(CT2A(str)), imgColor);
		}
	}

	imgColor.release();
}
#pragma endregion COLOR

#pragma region COMM
std::vector<std::vector<cv::Point>> FOREIGN_INSP::CalcBlob(cv::Mat imgSrc, cv::Mat imgDst, int nMinArea, bool bMaxBlob, bool bFillHole)
{
	std::vector<std::vector<cv::Point>> allRst;
	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imgSrc, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nCntBlob = allcontour.size();
	cv::Mat imgTemp(imgSrc.rows, imgSrc.cols, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTempA(imgSrc.rows, imgSrc.cols, CV_8UC1, cv::Scalar(0));
	imgDst.setTo(0);
	int nMAX_A = -1;
	int nMAX = -1;
	for (int a = 0; a < nCntBlob; a++)
	{
		int nArea = cv::contourArea(allcontour[a]);
		if (nMinArea >= nArea)
			continue;

		imgTemp.setTo(0);
		cv::drawContours(imgTemp, allcontour, a, cv::Scalar(255), cv::FILLED);
		if (bFillHole == false)
		{
			imgTempA.setTo(0);
			cv::bitwise_and(imgTemp, imgSrc, imgTempA);
			nArea = cv::countNonZero(imgTempA);
			if (nMinArea >= nArea)
				continue;
		}

		if (bMaxBlob)
		{
			if (nMAX_A >= nArea)
				continue;
			nMAX_A = nArea;
			nMAX = a;

			imgDst.setTo(0);
			allRst.clear();
			allRst.push_back(allcontour[nMAX]);
			if (bFillHole)
				imgTemp.copyTo(imgDst);
			else
				imgTempA.copyTo(imgDst);
		}
		else
		{
			if (bFillHole)
				cv::bitwise_or(imgTemp, imgDst, imgDst);
			else
				cv::bitwise_or(imgTempA, imgDst, imgDst);
			allRst.push_back(allcontour[a]);
		}
	}

	return allRst;
}
void FOREIGN_INSP::SaveReleaseWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, CString path, int bandSize)
{
	if ((buff == NULL) || (sizeX <= 0) || (sizeY <= 0) || (bandSize <= 0))
		return;

	int nTYPE = CV_8UC1;
	if (bandSize == 1)
		nTYPE = CV_8UC1;
	else if (bandSize == 3)
		nTYPE = CV_8UC3;
	else
		return;

	cv::Mat img(sizeY, sizeX, nTYPE, buff);
	SaveReleaseWorkImg(img, fileName, path);
}
void FOREIGN_INSP::SaveReleaseWorkImg(cv::Mat img, CString fileName, CString path)
{
	CreateDir(path);
	if (img.empty())
		return;

	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), path, fileName);
	cv::imwrite(std::string(CT2A(fullPath)), img);
}
void FOREIGN_INSP::CreateDir(CString Path)
{
	if (IsExistDir(Path))
		return;

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
BOOL FOREIGN_INSP::IsExistDir(CString path)
{
	BOOL ret = FALSE;
	CFileFind fn;

	BOOL bWorking = fn.FindFile(path);
	if (bWorking)
	{
		bWorking = fn.FindNextFileW();
		if (fn.IsDirectory())
			ret = TRUE;
	}
	return ret;
}
void FOREIGN_INSP::SavePTR(CString str, cv::Mat img3D, int nBit)
{
	SaveZmapPTR(img3D.ptr<float>(), str, img3D.cols, img3D.rows, nBit);
}
void FOREIGN_INSP::SaveZmapPTR(float *pZmap, CString filePath, int nW, int nH, int nBit)
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
void FOREIGN_INSP::SaveWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize, bool bSaveR)
{
	bool bReturn = false;
	if (bSaveR == true)
		bReturn = true;

#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
	bReturn = true;
#endif
#endif

	if (bReturn == false)
		return;

	SaveReleaseWorkImg(buff, sizeX, sizeY, WORK_IMAGE_PATH, fileName, bandSize);
}
void FOREIGN_INSP::GetClipImage_LT(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int Sx, int Sy, int roiSizeX, int roiSizeY, int band, double dAngle)
{
	if (userSrc == NULL || userDst == NULL)
		return;
	if (srcSizeX <= 0 || srcSizeY <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return;
	int stX = Sx;  // start point x
	int stY = Sy;  // start point Y
	if (stX < 0) stX = 0;
	if (stY < 0) stY = 0;
	if (roiSizeX > srcSizeX || stX + roiSizeX > srcSizeX ||
		roiSizeY > srcSizeY || stY + roiSizeY > srcSizeY)
		return;

	IppStatus sts;
	IppiSize iSize = { roiSizeX, roiSizeY };
	if (srcSizeX == roiSizeX && srcSizeY == roiSizeY)
	{
		if (band == 1)
			sts = ippiCopy_8u_C1R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);
		else if (band == 3)
			sts = ippiCopy_8u_C3R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);
	}
	else if (dAngle != 0 && band == 1)
	{
		UCHAR* ptrSrc = userSrc;
		UCHAR* ptrbyBuffer_ro = NULL;
		int retDstSizeX(0), retDstSizeY(0);

		for (int r = 0; r < srcSizeY; r++)
		{
			UCHAR* srcPtr = &userSrc[r*srcSizeX];
			UCHAR* dstPtr = &ptrSrc[r*srcSizeX];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*srcSizeX);
		}

		RotateImg_ipp(ptrSrc, srcSizeX, srcSizeY, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		int GapX = RounD((retDstSizeX - srcSizeX) / 2);
		int GapY = RounD((retDstSizeY - srcSizeY) / 2);
		if ((GapX + srcSizeX) > retDstSizeX)
			GapX = 0;
		if ((GapY + srcSizeY) > retDstSizeY)
			GapY = 0;

		for (int r = 0; r < srcSizeY; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];

			UCHAR* dstPtr = &userSrc[r*srcSizeX];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*srcSizeX);

		}
		SaveWorkImg(ptrbyBuffer_ro, retDstSizeX, retDstSizeY, _T("RotateTestbuffer.bmp"));
		SaveWorkImg(userSrc, srcSizeX, srcSizeY, _T("RotateTestPtr.bmp"));

		if (ptrbyBuffer_ro != NULL)
			g_pMManager->pem_delete(ptrbyBuffer_ro, true);
		sts = ippiCopy_8u_C1R(userSrc + stX + stY * srcSizeX, srcSizeX * band, userDst, roiSizeX * band, iSize);
	}
	else
	{
		if (band == 1)
			sts = ippiCopy_8u_C1R(userSrc + stX + stY * srcSizeX, srcSizeX * band, userDst, roiSizeX * band, iSize);
		else if (band == 3)
			sts = ippiCopy_8u_C3R(userSrc + ((stX* band) + (stY * (srcSizeX * band))), srcSizeX * band, userDst, roiSizeX * band, iSize);
	}
}
float FOREIGN_INSP::GetCropZmap_LT(float* src, float* dst, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY)
{
	if (src == NULL || dst == NULL)
		return -1;
	if (orgSizeX <= 0 || orgSizeY <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return -1;

	int stX = nStX;  // start point x
	int stY = nStY;  // start point Y
	if (stX < 0) stX = 0;
	if (stY < 0) stY = 0;
	if (stX >= orgSizeX || stY >= orgSizeY)
		return -1;

	if (roiSizeX > orgSizeX || roiSizeY > orgSizeY ||
		stX + roiSizeX > orgSizeX || stY + roiSizeY > orgSizeY)
	{
		int nETX = stX + roiSizeX;
		int nETY = stY + roiSizeY;
		if (nETX < 0) nETX = 0;
		if (nETY < 0) nETY = 0;
		if (nETX > orgSizeX) nETX = orgSizeX;
		if (nETY > orgSizeY) nETY = orgSizeY;
		for (int y = stY; y < nETY; y++)
		{
			if (y < 0) continue;
			if (y >= orgSizeY) break;
			for (int x = stX; x < nETX; x++)
			{
				if (x < 0) continue;
				if (x >= orgSizeX) break;
				int nIndex = (y * orgSizeX) + x;

				int nX = x - stX;
				int nY = y - stY;
				if (nX < 0 || nX >= roiSizeX)
					continue;
				if (nY < 0 || nY >= roiSizeY)
					continue;
				int nIndex2 = (nY * roiSizeX) + nX;
				dst[nIndex2] = src[nIndex];
			}
		}
		return 0;
	}

	IppiSize iSize = { roiSizeX, roiSizeY };
	IppStatus sts = ippiCopy_32f_C1R(src + stX + stY * orgSizeX, orgSizeX * 4, dst, roiSizeX * 4, iSize);

	return (float)sts;
}
void FOREIGN_INSP::RotateImg_ipp(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR** userDst, int* retDstSizeX, int* retDstSizeY)
{
	if (*userDst != NULL)
	{
		delete *userDst;
		*userDst = NULL;
	}

	bool bLinear = true;
	bool bColor = false;

	RotateImg_ipp2020(userSrc, userDst, angle, orgSizeX, orgSizeY, retDstSizeX, retDstSizeY, bColor, bLinear, false);
}
bool FOREIGN_INSP::RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (bColor)
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * 3 * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C3R(userSrc, orgSizeX * 3, *userDst, orgSizeX * 3, srcSize);
		}
		else
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C1R(userSrc, orgSizeX, *userDst, orgSizeX, srcSize);
		}

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	if (bColor)
		numChannels = 3;


	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp8u* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		//dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		//dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}


	//Memory allocation for the intermediate images
	//*userDst = new uchar[dstSize.width * numChannels * dstSize.height];
	*userDst = g_pMManager->pem_new<uchar>(true, dstSize.width * numChannels * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);
	Ipp8u* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;


	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing

		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}
void FOREIGN_INSP::GetClipImage(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int cx, int cy, int roiSizeX, int roiSizeY, int band)
{
	if (srcSizeX == roiSizeX && srcSizeY == roiSizeY)
	{
		IppStatus sts;
		IppiSize iSize = { roiSizeX, roiSizeY };

		if (band == 1)
			sts = ippiCopy_8u_C1R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);
		else if (band == 3)
			sts = ippiCopy_8u_C3R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);

		return;
	}

	if (roiSizeX > srcSizeX)
		roiSizeX = srcSizeX;
	if (roiSizeY > srcSizeY)
		roiSizeY = srcSizeY;

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int orgSizeX = srcSizeX;
	int orgSizeY = srcSizeY;
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if (stX < 0)
		stX = 0;

	if (stY < 0)
		stY = 0;

	if (stX + width >= orgSizeX)
		stX = orgSizeX - width;

	if (stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = { width, height };

	int offX = stX;
	int offY = stY;

	if (band == 1)
		sts = ippiCopy_8u_C1R(userSrc + offX + offY * srcSizeX, srcSizeX * band, userDst, width * band, iSize);
	else if (band == 3)
		sts = ippiCopy_8u_C3R(userSrc + ((offX* band) + (offY * (srcSizeX * band))), srcSizeX * band, userDst, width * band, iSize);
}
float FOREIGN_INSP::GetCropZmap(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY)
{
	if (src == NULL || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	if (roiSizeX > orgSizeX)
		roiSizeX = orgSizeX;
	if (roiSizeY > orgSizeY)
		roiSizeY = orgSizeY;

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if (stX < 0)
		stX = 0;

	if (stY < 0)
		stY = 0;

	if (stX + width >= orgSizeX)
		stX = orgSizeX - width;

	if (stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = { width, height };

	int offX = stX;
	int offY = stY;

	sts = ippiCopy_32f_C1R(src + offX + offY * orgSizeX, orgSizeX * 4, dst, width * 4, iSize);

	return (float)sts;
}
void FOREIGN_INSP::SaveDebugImg(cv::Mat img, CString FullPath)
{
	if (img.empty())
		return;

	cv::imwrite(std::string(CT2A(FullPath)), img);
}
bool FOREIGN_INSP::SizeChk(cv::Mat imgBin, double dArea, int nDivide, int nMode, int nNotSize)
{
	int nW = imgBin.cols;
	int nH = imgBin.rows;
	double dMinPer = 0.7;
	double dAreaPix = dArea / m_resolX / m_resolY / (nDivide * nDivide);
	double dAreaPixMin = dAreaPix * dMinPer;
	int nAreaMin = (int)(dAreaPixMin / 2.0) + 1;
	if (nNotSize > 0) nAreaMin = nNotSize + 2;
	int nMinMor = 12 / nDivide;

	int nMorErode = std::min(nAreaMin, nMinMor);
	if (nMorErode < 3) nMorErode = 3;

	std::vector<std::vector<cv::Point>> allSrc;
	std::vector<std::vector<cv::Point>> allRst;
	std::vector<cv::Vec4i> hierarchyS;
	std::vector<cv::Vec4i> hierarchyR;
	cv::findContours(imgBin, allSrc, hierarchyS, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nSrc = allSrc.size();
	if (nSrc == 0)
		return true;

	cv::Mat imgRst = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	MorErode(imgBin, imgRst, 0, nMorErode);
	cv::findContours(imgRst, allRst, hierarchyR, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nRst = allRst.size();
	if (nRst == 0)
		return true;

	cv::Mat imgDst(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgSrcTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgRstTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	for (int a = 0; a < nSrc; a++)
	{
		cv::Rect roiSrc = cv::boundingRect(allSrc[a]);
		if (roiSrc.width < 3 || roiSrc.height < 3)
			continue;

		imgSrcTemp.setTo(0);
		cv::drawContours(imgSrcTemp, allSrc, a, cv::Scalar(255), cv::FILLED);
		cv::Mat imgSrcTempA;
		cv::bitwise_and(imgSrcTemp, imgBin, imgSrcTempA);
		int nAreaSrc = cv::countNonZero(imgSrcTempA);
		if (nAreaSrc <= dAreaPixMin)
			continue;

		bool bContain = false;
		for (int b = 0; b < nRst; b++)
		{
			cv::Rect roiRst = cv::boundingRect(allRst[b]);
			cv::Rect roiA = roiSrc & roiRst;
			if (roiA.area() <= 0)
				continue;

			imgRstTemp.setTo(0);
			cv::drawContours(imgRstTemp, allRst, b, cv::Scalar(255), cv::FILLED);
			cv::Mat imgRstTempA;
			cv::bitwise_and(imgRstTemp, imgRst, imgRstTempA);

			cv::Mat imgA;
			cv::bitwise_and(imgSrcTempA, imgRstTempA, imgA);
			int nArea = cv::countNonZero(imgA);
			if (nArea > 0)
			{
				bContain = true;
				break;
			}
		}

		if (bContain == false)
			continue;
		cv::bitwise_or(imgSrcTempA, imgDst, imgDst);
	}
	imgDst.copyTo(imgBin);
	return false;
}
#pragma endregion COMM

#pragma region INSP

void FOREIGN_INSP::SetPartParam_Foreign(InspPartInfo* boardInfo)
{
	m_pInspBoardInfo_Foreign = boardInfo;
}
int FOREIGN_INSP::ReleaseInspForeignResult()
{
	if (m_inspForeignResult == NULL)
		return eIMSG_FAIL;

	if (m_inspForeignResult != NULL)
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
}
int FOREIGN_INSP::GetInspForeignResult(InspFovForeignResult* retResult)
{
	if (m_inspForeignResult == NULL)
		return eIMSG_FAIL;

	*retResult = *m_inspForeignResult;

	CString sLog = _T("");
	sLog.Format(_T("%d"), m_inspForeignResult->m_nCountDefect);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspForeignResult]"), _T("[ForeignResult]"), sLog);

	return eIMSG_SUCCESE;
}
void FOREIGN_INSP::DeleteForeignImgBuffer()
{
	//각 컬러 버퍼들은 동적으로 Alloc, Free 하지 않고 Pinsp_Color에 선언되어있는 전역버퍼를 참조하여 사용한다.
	//해당 전역버퍼는 프로그램 실행시 선언하고 종료할때 해재한다.
	if (m_pInspBoardInfo_Foreign->partZmapData.data)
		g_pMManager->pem_delete(m_pInspBoardInfo_Foreign->partZmapData.data, true);
	m_pInspBoardInfo_Foreign->partZmapData.data = NULL;
}
int FOREIGN_INSP::Inspection_Foreign_new(ForeignData sInspData, FR2DData sFR2DData, ForeignParamROI* arrROI)
{
	int nLine = __LINE__;
	try
	{
		DWORD st = GetTickCount();
		bool bDebugVerifyRun = false;
#if FR_MULTI_FUNC == 1
		if (g_pMPTI->GetUseMultiProcess_Foreign() && g_pMPTI->GetUseMultiProcess())
			bDebugVerifyRun = ext::irs::get()->_Ctrl.First()->prod.bForeignDebugSave; // 옵션키면 Eagle3d, Multi 둘다 검사

		if (g_pMPTI->GetUseMultiProcess_Foreign() && g_pMPTI->GetUseMultiProcess() && sInspData.m_vInspectionMode == eMI_Inspection && !bDebugVerifyRun)
		{
			SetExtForeignParam(m_pInspBoardInfo_Foreign, sInspData, sFR2DData, m_ForeignData, arrROI, 0);
			m_fFrTact[m_eFrTact_Inspection_Foreign_Set] += ((GetTickCount() - st) / 1000.0f);				st = GetTickCount();
			return dftCODE_OK;
		}
		else
#endif
		{
			if (bDebugVerifyRun)
			{
				SetExtForeignParam(m_pInspBoardInfo_Foreign, sInspData, sFR2DData, m_ForeignData, arrROI, 0);
			}
			// 이물검사
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
			nLine = __LINE__;

			m_inspForeignResult = g_pMManager->pem_new<InspFovForeignResult>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			float *pfOrg3D = (sInspData.m_vInspectionMode == eMI_Inspection) ? m_pInspBoardInfo_Foreign->zmapForeignData.data : sInspData.m_p3D;
			int nOrgW = (sInspData.m_vInspectionMode == eMI_Inspection) ? m_pInspBoardInfo_Foreign->zmapForeignData.zmapSizeX : m_fovWidth;
			int nOrgH = (sInspData.m_vInspectionMode == eMI_Inspection) ? m_pInspBoardInfo_Foreign->zmapForeignData.zmapSizeY : m_fovLength;
			int nFov = (sInspData.m_vInspectionMode == eMI_Inspection) ? m_pInspBoardInfo_Foreign->fovIndex : sInspData.m_nFovID;
			nLine = __LINE__;
			m_fFrTact[m_eFrTact_Inspection_Foreign_Set] += ((GetTickCount() - st) / 1000.0f);
			st = GetTickCount();

			int nRet = InspForeignAlgo(pfOrg3D, nOrgW, nOrgH, m_inspForeignResult, sInspData, nFov, sFR2DData, arrROI);
			m_fFrTact[m_eFrTact_Inspection_Foreign] += ((GetTickCount() - st) / 1000.0f);

			if (nRet == e_OK)
				return dftCODE_OK;
		}
	}
	catch (...)
	{
		CString sLog = _T("");
		sLog.Format(_T("Line : %d Pass!!!"), nLine);
		AddLOG(_T("[ERROR]"), _T("[FOREIGN_INSP]"), _T("[Inspection_Foreign]"), sLog);
		throw nLine;
	}

	return defCODE_FOREIGN;
}
int FOREIGN_INSP::InspForeignAlgo(float* pfOrg3D, int nOrgW, int nOrgH, InspFovForeignResult* ForeignResult, ForeignData sInspData, int nFovIDX, FR2DData sFR2DData, ForeignParamROI* arrROI)
{
	int nLine = __LINE__;

	int ret = e_OK;
	try
	{
		DWORD st = GetTickCount();
		if (g_pMPTI->IsForeignEnable() == false)
			return ret;

		if (m_ForeignParamROI == NULL)
		{
			m_ForeignData.m_nArrData[FR_N_ForeignParamROI_Total] = 0;
			m_ForeignParamROI = g_pMManager->pem_new<ForeignParamROI>(true, 1, (PCHAR)__FUNCTION__, __LINE__, true);
		}

		bool bSaveDebugImage = m_ForeignData.UseData(FR_DATA_DebugData);
		bool bSaveDebugImage2 = m_ForeignData.UseData(FR_DATA_DebugData2);
		int nDivide = m_ForeignData.GetDivide();
		int nW = nOrgW / nDivide - 1;
		int nH = nOrgH / nDivide - 1;
		int nHalfS = nOrgW / 2;
		int nHalfE = nOrgH / 2;
		nLine = __LINE__;

		float* pf3D = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__, true);
		float* pf3D2 = NULL;
		float* pfImg3DSub = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__, true);
		float* pfImgWP = NULL;
		nLine = __LINE__;

		GetCropZmap_LT(pfOrg3D, pf3D, nOrgW, nOrgH, 0, 0, nW, nH);
		GetCropZmap_LT(pfOrg3D, pfImg3DSub, nOrgW, nOrgH, nHalfS, nHalfE, nW, nH);
		int* pnImg3DSub = NULL;
		if (sInspData.m_bDEBUG)
		{
			pnImg3DSub = g_pMManager->pem_new<int>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__, true);
			for (long long nIndex = 0; nIndex < nW * nH; nIndex++)
				pnImg3DSub[nIndex] = pfImg3DSub[nIndex];
		}
		else
			pnImg3DSub = reinterpret_cast<int*>(pfImg3DSub);
		nLine = __LINE__;

		if (m_ForeignData.UseData(ZMAPFOV_TYPE_FOREIGN_PART, true))
		{
			pf3D2 = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__, true);
			GetCropZmap_LT(pfOrg3D, pf3D2, nOrgW, nOrgH, nHalfS, 0, nW, nH);
		}
		if (m_ForeignData.IsForeignEnable(m_eForeignInsp_Warpage) == true)
		{
			pfImgWP = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__, true);
			GetCropZmap_LT(pfOrg3D, pfImgWP, nOrgW, nOrgH, 0, nHalfE, nW, nH);
		}
		m_fFrTact[m_eFrTact_InspForeign_3D] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();
		nLine = __LINE__;

#if _DEBUG
		cv::Mat imgORG(nOrgH, nOrgW, CV_32FC1, pfOrg3D);
		cv::Mat img3D(nH, nW, CV_32FC1, pf3D);
		cv::Mat imgSub(nH, nW, CV_32FC1, pfImg3DSub);
#endif
		InspFovForeignResult *arrTempRst = g_pMManager->pem_new<InspFovForeignResult>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		if (m_ForeignData.IsForeignEnable(m_eForeignInsp_GrayBub))
		{
			CPInsp_Color *pColorInsp = g_pInspMng->GetColorInsp();
			if (pColorInsp)
				pColorInsp->InspColorBubble(m_ForeignData, m_pInspBoardInfo_Foreign, arrTempRst, sInspData.m_vInspectionMode, sInspData);
		}
		else
		{
			InspBlobForForeign(m_ForeignData, pf3D, pf3D2, pfImgWP, pnImg3DSub, nW, nH, arrTempRst, nFovIDX, sInspData, sFR2DData, arrROI);
			if (bSaveDebugImage || (bSaveDebugImage2 && arrTempRst && arrTempRst->m_bOK == FALSE))
			{
				float* pfSub = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__, true);
				for (int y = nHalfE; y < nOrgH; y++)
				{
					int nY = y - nHalfE;
					if (nY < 0 || nY >= nH)
						continue;
					for (int x = nHalfS; x < nOrgW; x++)
					{
						int nX = x - nHalfS;
						if (nX < 0 || nX >= nW)
							continue;
						long long nIndex = (y * nOrgW) + x;
						long long nIndex2 = (nY * nW) + nX;
						pfOrg3D[nIndex] = pnImg3DSub[nIndex2];
						pfSub[nIndex2] = pnImg3DSub[nIndex2];
					}
				}

				CString str = GetForeignDebugDataFullPath(nFovIDX, _T("OriginalImg"), -1, 2);
				SaveZmapPTR(pfOrg3D, str, nOrgW, nOrgH);

				str = GetForeignDebugDataFullPath(nFovIDX, _T("3DFr"), -1, 2);
				SaveZmapPTR(pf3D, str, nW, nH);

				str = GetForeignDebugDataFullPath(nFovIDX, _T("3D_2"), -1, 2);
				if (pf3D2) SaveZmapPTR(pf3D2, str, nW, nH);

				str = GetForeignDebugDataFullPath(nFovIDX, _T("3DWP"), -1, 2);
				if (pfImgWP) SaveZmapPTR(pfImgWP, str, nW, nH);

				str = GetForeignDebugDataFullPath(nFovIDX, _T("3DSub"), -1, 2);
				if (pfSub) SaveZmapPTR(pfSub, str, nW, nH);
				if (pfSub) { g_pMManager->pem_delete(pfSub, true); pfSub = NULL; }
			}
		}
		m_fFrTact[m_eFrTact_InspForeign_InspEnd] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();
		nLine = __LINE__;

		CalcForeignResult(arrTempRst, ForeignResult);
		if (arrTempRst)
		{
			nLine = __LINE__;
			if (arrTempRst->m_stForeign)
				g_pMManager->pem_delete(arrTempRst->m_stForeign, true);
			arrTempRst->m_stForeign = NULL;
			if (arrTempRst->m_stForeignWP)
				g_pMManager->pem_delete(arrTempRst->m_stForeignWP, true);
			arrTempRst->m_stForeignWP = NULL;
			g_pMManager->pem_delete(arrTempRst, false);
			arrTempRst = NULL;
			nLine = __LINE__;
		}
		m_fFrTact[m_eFrTact_InspForeign_CalcResult] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();
		nLine = __LINE__;

		if (pf3D) { g_pMManager->pem_delete(pf3D, true); pf3D = NULL; }
		if (pfImg3DSub) { g_pMManager->pem_delete(pfImg3DSub, true); pfImg3DSub = NULL; }
		if (pf3D2) { g_pMManager->pem_delete(pf3D2, true); pf3D2 = NULL; }
		if (pfImgWP) { g_pMManager->pem_delete(pfImgWP, true); pfImgWP = NULL; }
		if (sInspData.m_bDEBUG)
		{
			if (pnImg3DSub)
			{
				g_pMManager->pem_delete(pnImg3DSub, true);
				pnImg3DSub = NULL;
			}
		}

		ret = (ForeignResult->m_bOK) ? e_OK : e_NG;
		m_fFrTact[m_eFrTact_InspForeign_END] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;
	}
	catch (...)
	{
		CString sLog = _T("");
		sLog.Format(_T("Line : %d Pass!!!"), nLine);
		AddLOG(_T("[ERROR]"), _T("[FOREIGN_INSP]"), _T("[InspForeignAlgo]"), sLog);
		throw nLine;
	}
	return ret;
}
byte FOREIGN_INSP::ExceptForeignImage(int nMode, cv::Mat imgExcept, ForeignData sData_ROI)
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
void FOREIGN_INSP::ExceptForeign(int nType, cv::Mat imgExcept, ForeignData sData_ROI)
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
void FOREIGN_INSP::InspFRArea(ForeignData sInspData, cv::Mat imgArea)
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
bool FOREIGN_INSP::SetROI(ForeignData sInspData, int nIndex, cv::Mat img, int nW, int nH, double dFactorX, double dFactorY, byte byValue)
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
bool FOREIGN_INSP::SetExceptForeignImage(int nIndex, int nType, CRect rtFOV, UCHAR* pucDst, int nW, int nH, byte byValue, bool bExtraPer, float fExtraPixelCnt)
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
int FOREIGN_INSP::FillOutOfInspArea(RECT rcROI, int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue, BOOL bConvertExceptROI, bool bExtraPer, float fExtraPixelCnt)
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
bool FOREIGN_INSP::PtInPolygon(POINTF pt, POINTF *pts, int ptNum)
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
void FOREIGN_INSP::CalcForeignResult(InspFovForeignResult *arrTempRst, InspFovForeignResult* ForeignResult)
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
		CString sLog = _T("");
		sLog.Format(_T("Line : %d Pass!!!"), nLine);
		AddLOG(_T("[ERROR]"), _T("[FOREIGN_INSP]"), _T("[CalcForeignResult]"), sLog);
		throw nLine;
	}
}
void FOREIGN_INSP::SetExtForeignParam(InspPartInfo* _ForePartInfo, ForeignData _ForeinData, FR2DData _Fr2Data, InspForeignInfo _ForeignInfo, ForeignParamROI* arrROI, int nSelectedIdx)
{
	if (_ForePartInfo == NULL || g_pMPTI == nullptr)
		return;
#if FR_MULTI_FUNC == 1
	// Multi Exception 
	// Eagle3D Alive State Update
	if (ext::irs::get()->_Ctrl.GetSignal(ext::eMstSignal::eMstAlive) == false)
		ext::irs::get()->_Ctrl.SetSignal(ext::eMstSignal::eMstAlive, true, ExtNoUpdate);

	//Foreign Exception Add
	//


	int nSelectedToolid = ext::irs::get()->m_vToolForeign[_ForePartInfo->fovIndex % ext::env::nForeignTypeToolCnt]; // BigPartTool 에서 이물검사 진행
	// FOV Index 별로 공유메모리 저장 
	int nFov = _ForePartInfo->fovIndex;

	while (g_pMPTI->IsSeqForceStop() == false)
	{

		bool bBreak = ext::irs::get()->_Tools[nSelectedToolid]._SndForeignCtrl[nFov].GetFlag(ext::BufferFlag::IF_OFF);


		//	(!(nBufferIndex < 0 || nBufferIndex > EXT_BUFFER_CNT || ext::irs::get()->_Tools[nSelectTool]._SndCtrl[nBufferIndex].GetFlag(ext::BufferFlag::IF_CALC) || g_pMPTI->IsSeqForceStop())); // true used , false not use

			//서브 프로세스 시그널 모두 살아있는지 확인 필요
		if (g_pMPTI->CheckAllSubProcess2(ext::eProcessCheck::eExistCheck) == false)
		{
			if (g_pMPTI->IsSeqForceStop() == false)
			{
				g_pMPTI->SetExtAlarm(true); // 이중으로 Alaram Set 방지
				break;
			}

		}
		if (bBreak)
			break;
	}

	if (g_pMPTI->IsSeqForceStop() == false) // 알람 발생시 데이터 전달 방지
	{
		ext::irs::get()->_Tools[nSelectedToolid].AddShared_ForeignData(_ForePartInfo, _ForeinData, _Fr2Data, _ForeignInfo, m_ForeignParamROI, arrROI, nFov);
		ext::irs::get()->_Tools[nSelectedToolid]._SndForeignCtrl[nFov].SetFlag(ext::BufferFlag::IF_ON, true);
	}
	//
#endif
}
BOOL FOREIGN_INSP::InspBlobForForeign(InspForeignInfo sForeign, float* p3DFr, float* pf3D2, float* p3DWP, int* pnImg3DSub, int nW, int nH, InspFovForeignResult* ForeignResult, int nFovID, ForeignData sInspData, FR2DData sFR2DData, ForeignParamROI* arrROI)
{
	CString sLog = _T("");
	DWORD st = GetTickCount();
	DWORD stT = GetTickCount();
	if (nW <= 0 || nH <= 0)
		return FALSE;

	ForeignParamROI *pROI = m_ForeignParamROI;
	InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;
	MPTI_InspectionMode vInspectionMod = sInspData.m_vInspectionMode;

	if (sInspData.m_pDesImgBin)
		memset(sInspData.m_pDesImgBin, 0, sizeof(UCHAR) * nW * nH);
	int projectionmode = sInspData.m_nProjectionmode;
	int nDivide = sForeign.GetDivide();
	int nModuleTotal = sInspData.m_nModule;
	if (nModuleTotal <= 0)
		return FALSE;

	int nROITotal = sForeign.m_nArrData[FR_N_ForeignParamROI_Total];
	int* nArrModuleID = g_pMManager->pem_new<int>(true, nModuleTotal, (PCHAR)__FUNCTION__, __LINE__);
	int* nArrModuleCnt = g_pMManager->pem_new<int>(true, nModuleTotal, (PCHAR)__FUNCTION__, __LINE__);
	cv::Mat *imgInspArea = g_pMManager->pem_new<cv::Mat>(true, nModuleTotal, (PCHAR)__FUNCTION__, __LINE__);
	memset(nArrModuleID, 0, sizeof(int) * nModuleTotal);
	memset(nArrModuleCnt, 0, sizeof(int) * nModuleTotal);
	for (int a = 0; a < nModuleTotal; a++)
		imgInspArea[a] = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	int nMCnt = 0;
	for (int a = 0; a < nROITotal; a++)
	{
		if (pROI == NULL) break;
		int nModuleID = pROI[a].m_nModuleID;
		if (nModuleID > 0 && nModuleTotal > nModuleID && nArrModuleID[nModuleID] != nModuleID)
		{
			nArrModuleID[nModuleID] = nModuleID;
			nMCnt++;
		}
		if (nMCnt == nModuleTotal)
			break;
	}
	m_fFrTact[m_eFrTact_Insp_Module_Data] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	bool bSaveImg = sForeign.UseData(FR_DATA_DebugData);
	bool bGND = sInspData.UseData(m_eForeignData_DATA_GND);
	int nMaxRst = 2000;
	cv::Mat *imgFR = g_pMManager->pem_new<cv::Mat>(true, FR_IMG_Total, (PCHAR)__FUNCTION__, __LINE__);
	for (int a = 0; a < FR_IMG_Total; a++)
	{
		if (a == FR_IMG_FIND || a == FR_IMG_HEXT || a == FR_IMG_TRACE_NONE)
			imgFR[a] = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(255));
		else
			imgFR[a] = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	}
	if (bGND)
	{
		imgFR[FR_IMG_TRACE].setTo(255);
		imgFR[FR_IMG_TRACE_NONE].setTo(0);
	}
	m_fFrTact[m_eFrTact_Insp_Data] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	ForeignResult->m_bOK = TRUE;
	ForeignResult->m_stForeign = g_pMManager->pem_new<AForeignResult>(true, nMaxRst, (PCHAR)__FUNCTION__, __LINE__);
	ForeignResult->m_stForeignWP = g_pMManager->pem_new<AForeignResultWP>(true, nMaxRst, (PCHAR)__FUNCTION__, __LINE__);
	memset(ForeignResult->m_stForeign, 0, sizeof(AForeignResult) * nMaxRst);
	memset(ForeignResult->m_stForeignWP, 0, sizeof(AForeignResultWP) * nMaxRst);
	ForeignResult->m_nCountDefect = 0;
	ForeignResult->m_nCountDefectWP = 0;
	for (int a = 0; a < nMaxRst; a++)
	{
		ForeignResult->m_stForeignWP[a].m_fHeightMin = 9999.9;
		ForeignResult->m_stForeignWP[a].m_fHeightMax = -9999.9;
		ForeignResult->m_stForeignWP[a].m_fHeightMinAM = 9999.9;
		ForeignResult->m_stForeignWP[a].m_fHeightMaxAM = -9999.9;
	}

	if (sInspData.m_nFRROI > 0 && arrROI)
		GetInspAreaROI(sForeign, sInspData, arrROI, imgInspArea, imgFR, nW, nH, nModuleTotal, nFovID, nArrModuleID, nArrModuleCnt);
	sLog.Format(_T("%d OUT"), sInspData.m_nFRROI);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[GetInspAreaROI]"), sLog, st);

	if (GetInspArea(sForeign, sInspData, sFR2DData, p3DFr, p3DWP, pnImg3DSub, imgInspArea, imgFR, nW, nH, nModuleTotal, nFovID, nArrModuleID, nArrModuleCnt) == false)
	{
		AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[GetInspArea]"), _T("OUT"), st);
		m_fFrTact[m_eFrTact_Insp_GetInspArea] += ((GetTickCount() - st) / 1000.0f);
		st = GetTickCount();
		Delete_1DArray(&imgInspArea);
		Delete_1DArray(&imgFR);
		Delete_1DArray(&nArrModuleID);
		Delete_1DArray(&nArrModuleCnt);
		return FALSE;
	}
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[GetInspArea]"), _T("END"), st);

	m_fFrTact[m_eFrTact_Insp_GetInspArea] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	if (pf3D2 && sInspData.m_nTotalCnt > 0 && sForeign.UseData(ZMAPFOV_TYPE_FOREIGN_PART, true))
	{
		GetFindBody(p3DFr, pf3D2, imgInspArea[0], imgFR, nW, nH, sInspData, nDivide, nFovID, bSaveImg);
		for (int nModuleIDX = 0; nModuleIDX < nModuleTotal; nModuleIDX++)
		{
			int nModuleID = nArrModuleID[nModuleIDX];
			if (nModuleIDX > 0 && nModuleID <= 0)
				continue;

			if (nArrModuleCnt[nModuleID] <= 10)
				continue;

			cv::bitwise_and(imgFR[FR_IMG_FIND], imgInspArea[nModuleID], imgInspArea[nModuleID]);
		}
	}
	m_fFrTact[m_eFrTact_Insp_FindBody] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[GetFindBody]"), _T("END"), st);

	m_fFrTact[m_eFrTact_Insp_Set] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	cv::Mat imgColor;
	cv::Mat imgORG2D[eM2C_NUM];
	cv::Mat imgIM;
	if (g_pMPTI->IsForeignEnable(m_eForeignInsp_2D) == true)
	{
		imgColor = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
		if (vInspectionMod != eMI_Inspection)
			Get2DFOV(projectionmode, pInspBoardInfo, vInspectionMod, imgORG2D, nDivide, nFovID, bSaveImg);
	}

	m_fFrTact[m_eFrTact_Insp_Get2DFOV] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[Get2DFOV]"), _T("END"), st);

	bool bTrace = sForeign.IsForeignEnable(m_eForeignInsp_Foreign, m_eForeignInsp_Trace);
	if (bTrace)
	{
		bool bTraceModeNONE = sForeign.IsForeignEnable(m_eForeignInsp_Foreign, m_eForeignInsp_TraceModeNONE);
		if (m_nFR_TraceID > nModuleTotal)
		{
			int nDilateO = sForeign.m_nArrData[FR_N_ExtraPixelETC2];
			int nDilateG = sForeign.m_nArrData[FR_N_ExtraPixelPAD];
			int nDilateW = sForeign.m_nArrData[FR_N_ExtraPixelPAD_W];
			FR_INSP_TRACE(sForeign, sInspData, imgFR, nW, nH);
			AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[FR_INSP_TRACE]"), _T("END"), st);

			if (nDilateO > 0)
			{
				int nDilate = nDilateO + 2;
				if (nDilate < 3) nDilate = 3;
				cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nDilate, nDilate));
				imgFR[FR_IMG_BIN].setTo(0);
				imgFR[FR_IMG_FLASH].copyTo(imgFR[FR_IMG_BIN]);
				cv::dilate(imgFR[FR_IMG_BIN], imgFR[FR_IMG_FLASH], kernel);
			}

			cv::bitwise_and(imgFR[FR_IMG_TRACE], imgInspArea[0], imgFR[FR_IMG_TRACE]);
			imgFR[FR_IMG_TRACE] = imgFR[FR_IMG_TRACE] - imgFR[FR_IMG_TRACE_EXT];
			imgFR[FR_IMG_TRACE] = imgFR[FR_IMG_TRACE] - imgFR[FR_IMG_FLASH];
			if (nDilateG > 0)
			{
				int nDilate = nDilateG * 2 + 1;
				if (nDilate < 3) nDilate = 3;
				imgFR[FR_IMG_BIN].setTo(0);
				imgFR[FR_IMG_TRACE].copyTo(imgFR[FR_IMG_BIN]);
				MorErode(imgFR[FR_IMG_BIN], imgFR[FR_IMG_TRACE], 0, nDilate);
			}

			if (bTraceModeNONE)
			{
				cv::Mat imgTemp;
				cv::bitwise_and(imgFR[FR_IMG_TRACE_NONE], imgInspArea[0], imgTemp);
				cv::bitwise_or(imgTemp, imgFR[FR_IMG_TRACE_EXT], imgTemp);
				imgFR[FR_IMG_TRACE_NONE] = imgTemp - imgFR[FR_IMG_FLASH];
				if (nDilateW > 0)
				{
					int nDilate = nDilateW * 2 + 1;
					if (nDilate < 3) nDilate = 3;
					imgFR[FR_IMG_BIN].setTo(0);
					imgFR[FR_IMG_TRACE_NONE].copyTo(imgFR[FR_IMG_BIN]);
					MorErode(imgFR[FR_IMG_BIN], imgFR[FR_IMG_TRACE_NONE], 0, nDilate);
				}
			}
		}
	}
	m_fFrTact[m_eFrTact_Insp_TRACE] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[TRACE]"), _T("END"), st);

	for (int nMode = m_eForeignInsp_Foreign; nMode < m_eForeignInsp_Total; nMode++)
	{
		DWORD st1 = GetTickCount();
		if (nMode == m_eForeignInsp_GrayBub)
			continue;

		if (sForeign.IsForeignEnable(nMode) == false)
			continue;

		if (vInspectionMod == eMI_WindowTeaching)
		{
			if (sInspData.m_nMode >= 0 && m_eForeignInsp_Total > sInspData.m_nMode)
			{
				if (sInspData.m_nMode != nMode)
					continue;
			}
		}

		if (nMode == m_eForeignInsp_Warpage)
		{
			bool bNot_Borad = sForeign.sWP.UseData(FR_WP_DT_Not_Borad);
			for (int nModuleIDX = 0; nModuleIDX < nModuleTotal; nModuleIDX++)
			{

				int nModuleID = nArrModuleID[nModuleIDX];
				if (nModuleIDX > 0 && nModuleID <= 0)
					continue;

				if (nArrModuleCnt[nModuleID] == 0)
					continue;

				if (bNot_Borad)
				{
					if (nModuleID == 0)
						continue;
				}

				for (int nAM = 0; nAM < 2; nAM++)
				{
					if (InspWarpage(sForeign, p3DWP, imgInspArea[nModuleID], imgFR, nW, nH, nModuleID, nFovID, nAM, sInspData, ForeignResult) == false)
						ForeignResult->m_bOK = false;
				}
			}
			sLog.Format(_T("%d END"), nMode);
			AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[InspWarpage]"), sLog, stT);
		}
		else
		{
			if (ForeignResult->m_nCountDefect >= nMaxRst)
				continue;

			DWORD st2 = GetTickCount();
			FR_Bin sData = sForeign.sBin[nMode];
			bool bWP = sForeign.IsForeignEnable(m_eForeignInsp_Warpage);
			bool bIM = sForeign.sWP.UseData(FR_WP_DT_IM) && bWP;
			bool bBTC = sData.UseData(FR_BIN_DT_ExceptBTC);
			imgFR[FR_IMG_BIN].setTo(0);
			memcpy(imgFR[FR_IMG_BIN].data, imgInspArea[0].data, sizeof(UCHAR) * nW * nH);
			bool bExceptFR = true;
			if (sData.UseData(FR_BIN_DT_Insp_Skip_Part))
				memcpy(imgFR[FR_IMG_BIN].data, imgFR[FR_IMG_SKIP_PART].data, sizeof(UCHAR) * nW * nH);
			else if (sData.UseData(FR_BIN_DT_INSP_TRACE))
			{
				memcpy(imgFR[FR_IMG_BIN].data, imgFR[FR_IMG_TRACE].data, sizeof(UCHAR) * nW * nH);
				bExceptFR = false;
			}
			else if (sData.UseData(FR_BIN_DT_INSP_TRACE_NONE))
			{
				memcpy(imgFR[FR_IMG_BIN].data, imgFR[FR_IMG_TRACE_NONE].data, sizeof(UCHAR) * nW * nH);
				bExceptFR = false;
			}
			else
			{
				if (sData.UseData(FR_BIN_DT_InspSolderArea))
					memcpy(imgFR[FR_IMG_BIN].data, imgFR[FR_IMG_SOLDER].data, sizeof(UCHAR) * nW * nH);
				else
					cv::bitwise_and(imgFR[FR_IMG_SOLDER_INV], imgInspArea[0], imgFR[FR_IMG_BIN]);
			}
			if (bIM) cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_IM], imgFR[FR_IMG_BIN]);
			if (bBTC)
			{
				imgFR[FR_IMG_BLOB] = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(255));
				cv::bitwise_xor(imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BOTM], imgFR[FR_IMG_BLOB]);
				cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BIN]);
			}
			if (bExceptFR)
				GetExceptFR(sForeign, sInspData, imgFR[FR_IMG_BIN], nW, nH, nMode);
			m_fFrTact[m_eFrTact_Insp_AREA + nMode] += ((GetTickCount() - st2) / 1000.0f);
			sLog.Format(_T("%d END"), nMode);
			AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[FR_IMG_BIN]"), sLog, stT);

			if (InspForeign(sForeign, p3DFr, imgInspArea[0], imgFR, imgORG2D, nW, nH, nMode, nFovID, sInspData, ForeignResult, sFR2DData) == false)
				ForeignResult->m_bOK = false;
			sLog.Format(_T("%d END"), nMode);
			AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[InspForeign]"), sLog, stT);

			if (InspForeign_HA(sForeign, p3DFr, imgInspArea[0], imgFR, nW, nH, nMode, nFovID, sInspData, ForeignResult, sFR2DData) == false)
				ForeignResult->m_bOK = false;
			sLog.Format(_T("%d END"), nMode);
			AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[InspForeign_HA]"), sLog, stT);
		}

		m_fFrTact[m_eFrTact_Insp_Foreign + nMode] += ((GetTickCount() - st1) / 1000.0f);
	}

	Delete_1DArray(&imgInspArea);
	Delete_1DArray(&imgFR);
	Delete_1DArray(&nArrModuleID);
	Delete_1DArray(&nArrModuleCnt);
	m_fFrTact[m_eFrTact_Insp_END] += ((GetTickCount() - st) / 1000.0f);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[END]"), _T(""), stT);
	return TRUE;
}
bool FOREIGN_INSP::GetInspArea(InspForeignInfo sForeign, ForeignData sInspData, FR2DData sFR2DData, float* p3DFr, float* p3DWP, int* pnImg3DSub, cv::Mat *imgInspArea, cv::Mat *imgFR, int nW, int nH, int nModuleTotal, int nFovID, int* nArrModuleID, int* nArrModuleCnt)
{
	CString sLog;
	CString sAMIM;
	DWORD st = GetTickCount();
	DWORD st1 = GetTickCount();
	int nWP_W = sInspData.m_nWP_W;
	int nWP_H = sInspData.m_nWP_H;
	bool bRemoveHole = sForeign.UseData(FR_DATA_RemoveArea_Hole);
	bool bSave = sForeign.UseData(FR_DATA_DebugData);
	bool bForeignExceptHeight = sForeign.UseData(FR_DATA_ForeignExceptHeight);
	bool b3DTALL = sForeign.UseData(ZMAPFOV_TYPE_FOREIGN_TALL, true);
	bool bWP = sForeign.IsForeignEnable(m_eForeignInsp_Warpage);
	bool bRemoveIM = sForeign.sWP.UseData(FR_WP_DT_RemoveArea_IM);
	bool bIM = sForeign.sWP.UseData(FR_WP_DT_IM);
	bool bAM = sForeign.sWP.UseData(FR_WP_DT_AM);
	bool b3D = g_pMPTI->IsForeignEnable(m_eForeignInsp_3D);
	bool bBTC = g_pMPTI->IsForeignEnable(m_eForeignInsp_BTM);
	bool bSolder = g_pMPTI->IsForeignEnable(m_eForeignInsp_SolderArea);
	bool bSkipPart = g_pMPTI->IsForeignEnable(m_eForeignInsp_SkipPart);
	bool bManual = sForeign.UseData(FR_DATA_Manual_EXT);
	bool bCorrectWP = sForeign.sWP.UseData(FR_WP_DT_CorrectWP);
	bool bCorrectWP_B = sForeign.sWP.UseData(FR_WP_DT_CorrectWP_B);
	bool bNot_Borad = sForeign.sWP.UseData(FR_WP_DT_Not_Borad);
	bool bPseudoZero = sForeign.sWP.UseData(FR_WP_DT_PseudoZero);
	bool bSaveWP = bWP && sInspData.m_pDesImg;
	bool bTraceInsp = sForeign.IsForeignEnable(m_eForeignInsp_Foreign, m_eForeignInsp_Trace);
	bool bGND = sInspData.UseData(m_eForeignData_DATA_GND);
	bool bArrROI = sInspData.m_nFRROI > 0;
	if (bIM || bAM) bCorrectWP = false;
	if (bIM)
		sAMIM = _T("[IM]");
	else if (bAM)
		sAMIM = _T("[AM]");

	if (imgInspArea[0].empty())
		imgInspArea[0] = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgHol = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgWP(nH, nW, CV_8UC3, cv::Scalar(0));
	cv::Mat imgSticker(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgPart = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgExt = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgInspBoard = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));

	float fExceptModuleAreaX = sForeign.m_fArrData[FR_F_ExceptModuleAreaX];
	float fExceptModuleAreaY = sForeign.m_fArrData[FR_F_ExceptModuleAreaY];
	float fExceptHeightMin = sForeign.m_fArrData[FR_F_ForeignExceptHeightMin];
	float fExceptHeightMax = sForeign.m_fArrData[FR_F_ForeignExceptHeightMax];
	float fPseudoColorMin = sForeign.sWP.m_fArr[FR_WP_F_PseudoColorMin];
	float fPseudoColorMax = sForeign.sWP.m_fArr[FR_WP_F_PseudoColorMax];
	float fAMIMH = sForeign.m_fArrData[FR_F_BoardOffset];
	bool bExceptModule = fExceptModuleAreaX > 0 || fExceptModuleAreaY > 0;
	int nHoleCnt = 0;
	int nAMIMCnt = 0;
	int nDivide = sForeign.GetDivide();
	int nExceptArea = sForeign.GetRemove(nW, nH);
	int nExceptAreaWP = sForeign.sWP.GetRemove(nW, nH, nDivide);
	int nExceptAreaIM = sForeign.sWP.GetRemove(nW, nH, nDivide, true);
	int nExceptModuleAreaX = fExceptModuleAreaX / m_resolX / nDivide;
	int nExceptModuleAreaY = fExceptModuleAreaY / m_resolY / nDivide;
	int nExceptModuleAreaXMin = RounD((double)nW / 6.0);
	int nExceptModuleAreaYMin = RounD((double)nH / 6.0);
	if (nExceptModuleAreaX > 0 && nExceptModuleAreaXMin > nExceptModuleAreaX) nExceptModuleAreaXMin = nExceptModuleAreaX;
	if (nExceptModuleAreaY > 0 && nExceptModuleAreaYMin > nExceptModuleAreaY) nExceptModuleAreaYMin = nExceptModuleAreaY;
	int nExceptModuleArea = nExceptModuleAreaXMin * nExceptModuleAreaYMin;
	int nFOVW = sInspData.m_rcFOV.right - sInspData.m_rcFOV.left;
	int nFOVH = sInspData.m_rcFOV.bottom - sInspData.m_rcFOV.top;
	if (fExceptHeightMin < -2500) fExceptHeightMin = -2500;
	if (fExceptHeightMax > 2500) fExceptHeightMax = 2500;
	if (b3DTALL)
	{
		fExceptHeightMin = 0;
		fExceptHeightMax = 5000;
	}
	if (bCorrectWP)
	{
		fPseudoColorMin = -2500.0f;
		fPseudoColorMax = 2500.0f;
	}
	bool bSaveFR = nFOVW > 0 && nFOVH > 0;
	int nABS_W = abs(nW - nFOVW);
	int nABS_H = abs(nH - nFOVH);
	bool bSIZE = ((nABS_W == 0 || nABS_W == 1) || (nABS_H == 0 || nABS_H == 1));
	InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;
	m_fFrTact[m_eFrTact_GetInspArea_WP] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	for (int b = 0; b < StickerCnt; b++)
	{
		std::vector<cv::Point> vStickerRectPo;
		int nOUTCnt = 0;
		for (int a = 0; a < StickerPo; a++)
		{
			int nIDX = (b * StickerPo) + a;
			POINT po = sInspData.vStickerArea[nIDX];
			if (po.x <= 0 && po.y <= 0)
				nOUTCnt++;
			if (po.x < 0 && po.y < 0)
				continue;

			cv::Point pPo(0, 0);
			if (po.x > 0) pPo.x = po.x / nDivide - 1;
			if (po.y > 0) pPo.y = po.y / nDivide - 1;
			if (pPo.x < 0) pPo.x = 0;
			if (pPo.y < 0) pPo.y = 0;
			vStickerRectPo.push_back(pPo);
		}
		if (StickerPo > nOUTCnt && vStickerRectPo.size() > 3)
		{
			cv::fillPoly(imgSticker, vStickerRectPo, cv::Scalar(255));
			sLog.Format(_T("foreign: %d, %d / %d, %d / %d, %d / %d, %d")
				, vStickerRectPo[0].x, vStickerRectPo[0].y, vStickerRectPo[1].x, vStickerRectPo[1].y
				, vStickerRectPo[2].x, vStickerRectPo[2].y, vStickerRectPo[3].x, vStickerRectPo[3].y);
			AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[vStickerRectPo]"), sLog, st1, m_eLogLv::m_eLogLv_Sticker);
		}
	}
	m_fFrTact[m_eFrTact_GetInspArea_Sticker] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	if (bArrROI == false && bManual)
	{
		ExceptForeign(ForeignParamROI_Data_Foreign, imgFR[FR_IMG_EXT], sInspData);
		AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[ExceptForeign]"), _T("MANUAL"));
	}
	m_fFrTact[m_eFrTact_GetInspArea_Sticker] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();
#if _DEBUG
	//cv::Mat imgFovRD(pInspBoardInfo->partImgBuf.nImageSizeY, pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgTop_R);
	//cv::Mat imgFovGD(pInspBoardInfo->partImgBuf.nImageSizeY, pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgTop_G);
	//cv::Mat imgFovBD(pInspBoardInfo->partImgBuf.nImageSizeY, pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgTop_B);
	//cv::Mat imgRstRD(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_8UC1, sInspData.m_pimgTopR);
	//cv::Mat imgRstGD(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_8UC1, sInspData.m_pimgTopG);
	//cv::Mat imgRstBD(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_8UC1, sInspData.m_pimgTopB);
#endif
	for (long long nIndex = 0; nIndex < nW * nH; nIndex++)
	{
		DWORD st1 = bSave ? GetTickCount() : 0;
		int nSub = pnImg3DSub[nIndex];
		if (bBTC && (nSub & FR_SUB_BOTM) == FR_SUB_BOTM)
			imgFR[FR_IMG_BOTM].data[nIndex] = 255;
		if ((bAM || bIM) && (nSub & FR_SUB_AMIM) == FR_SUB_AMIM)
		{
			bool bInAMIM = true;
			if (b3D)
			{
				if (bIM)
				{
					float f3D = p3DFr[nIndex];
					if (fAMIMH > f3D)
						bInAMIM = false;
				}
			}
			if (bInAMIM)
			{
				imgFR[FR_IMG_AM].data[nIndex] = 255;
				nAMIMCnt++;
			}
		}
		if (b3D && (nSub & FR_SUB_HOLE) == FR_SUB_HOLE)
		{
			imgHol.data[nIndex] = 255;
			nHoleCnt++;
		}
		if (b3D && bForeignExceptHeight)
		{
			float f3D = p3DFr[nIndex];
			if (f3D < fExceptHeightMin || f3D > fExceptHeightMax)
				imgFR[FR_IMG_HEXT].data[nIndex] = 0;
		}

		if (bSave)
		{
			m_fFrTact[m_eFrTact_GetInspArea_Bin_Sub] += ((GetTickCount() - st1) / 1000.0f);
			st1 = GetTickCount();
		}

		if (bArrROI == false && (bTraceInsp && m_nFR_TraceID > nModuleTotal))
		{
			int nType = FR_SUB_MODU * (m_nFR_TraceID + 1);
			if ((nSub & nType) == nType)
			{
				imgFR[FR_IMG_TRACE].data[nIndex] = bGND ? 0 : 255;
				imgFR[FR_IMG_TRACE_NONE].data[nIndex] = bGND ? 255 : 0;
			}

			nType = FR_SUB_MODU * (m_nFR_TraceID + 1 + 1);
			if ((nSub & nType) == nType)
				imgFR[FR_IMG_TRACE_EXT].data[nIndex] = 255;
		}

		for (int nModuleIDX = nModuleTotal - 1; nModuleIDX >= 0; nModuleIDX--)
		{
			if (bArrROI)
				break;
			if ((nSub & FR_SUB_EXCP) != FR_SUB_EXCP)
				break;
			if (imgSticker.data[nIndex] == 255)
				break;
			if (bManual)
			{
				if (imgFR[FR_IMG_EXT].data[nIndex] == 255)
					break;
			}

			int nModuleID = nArrModuleID[nModuleIDX];
			if (nModuleIDX > 0 && nModuleID <= 0)
				continue;
			int nType = FR_SUB_MODU * (nModuleID + 1);
			if ((nSub & nType) == nType)
			{
				if (nModuleID > 0)
				{
					if (sInspData.m_nContainModuleID > 0 && sInspData.m_nContainModuleID != nModuleID)
						break;

					if (sInspData.m_nModule > nModuleID)
					{
						if (sInspData.m_pInspModule[nModuleID] == 0)
							break;
					}
				}

				imgInspArea[nModuleID].data[nIndex] = 255;
				nArrModuleCnt[nModuleID]++;
				if (nModuleID > 0)
				{
					imgInspArea[0].data[nIndex] = 255;
					nArrModuleCnt[0]++;
				}
				break;
			}
		}

		if ((bIM && bRemoveIM) || bSave)
		{
			if ((nSub & FR_SUB_PART) == FR_SUB_PART)
				imgPart.data[nIndex] = 255;
			if ((nSub & FR_SUB_EXCP) == FR_SUB_EXCP)
				imgExt.data[nIndex] = 255;
		}

		if (bSave)
			m_fFrTact[m_eFrTact_GetInspArea_Bin_M] += ((GetTickCount() - st1) / 1000.0f);

		if (bSaveFR)
		{
			bool bInput = true;
			if (bSaveWP && bPseudoZero)
			{
				bInput = false;
				for (int nModuleIDX = nModuleTotal - 1; nModuleIDX >= 0; nModuleIDX--)
				{
					int nModuleID = nArrModuleID[nModuleIDX];
					if (nModuleIDX > 0 && nModuleID <= 0)
						continue;

					int nType = FR_SUB_MODU * (nModuleID + 1);
					if ((nSub & nType) != nType)
						continue;

					if (bNot_Borad && nModuleID == 0)
						continue;

					imgInspBoard.data[nIndex] = 255;
					bInput = true;
					break;
				}
			}

			long long lX = (nIndex % nW);
			long long lY = (nIndex / nW);
			long long lFullX = lX + sInspData.m_rcFOV.left;
			long long lFullY = lY + sInspData.m_rcFOV.top;
			if (lFullX >= 0 && lFullX < nWP_W && lFullY >= 0 && lFullY < nWP_H)
			{
				long long nDst = ((lFullY * nWP_W) + lFullX);
				if (bSaveWP && bInput)
				{
					long long nDstId = bSIZE ? nDst * 3 : nIndex * 3;
					if (bSIZE)
						SetRGB(sInspData.m_pDesImg, nDstId, p3DWP[nIndex], fPseudoColorMin, fPseudoColorMax);
					else
						SetRGB(imgWP.data, nDstId, p3DWP[nIndex], fPseudoColorMin, fPseudoColorMax);
				}
				if (bSIZE)
				{
					if (sFR2DData.m_nCalc == 2 && sFR2DData.m_nFov == nFovID)
					{
						nIndex = (lY * sFR2DData.m_nW) + lX;
						if (bInput && sInspData.m_pimgFR)
							sInspData.m_pimgFR[nDst] = p3DFr[nIndex];
						if (bInput && bSaveWP)
						{
							if (sInspData.m_pimgWP)
								sInspData.m_pimgWP[nDst] = p3DWP[nIndex];
						}
						if (sInspData.m_pimgTopR)
							sInspData.m_pimgTopR[nDst] = sFR2DData.m_pucTR[nIndex];
						if (sInspData.m_pimgTopG)
							sInspData.m_pimgTopG[nDst] = sFR2DData.m_pucTG[nIndex];
						if (sInspData.m_pimgTopB)
							sInspData.m_pimgTopB[nDst] = sFR2DData.m_pucTB[nIndex];
					}
					else if (pInspBoardInfo && lX < pInspBoardInfo->partImgBuf.nImageSizeX && lY < pInspBoardInfo->partImgBuf.nImageSizeY)
					{
						nIndex = (lY * pInspBoardInfo->partImgBuf.nImageSizeX) + lX;
						if (bInput && sInspData.m_pimgFR)
							sInspData.m_pimgFR[nDst] = p3DFr[nIndex];
						if (bInput && bSaveWP)
						{
							if (sInspData.m_pimgWP)
								sInspData.m_pimgWP[nDst] = p3DWP[nIndex];
						}
						if (sInspData.m_pimgTopR)
							sInspData.m_pimgTopR[nDst] = pInspBoardInfo->partImgBuf.imgTop_R[nIndex];
						if (sInspData.m_pimgTopG)
							sInspData.m_pimgTopG[nDst] = pInspBoardInfo->partImgBuf.imgTop_G[nIndex];
						if (sInspData.m_pimgTopB)
							sInspData.m_pimgTopB[nDst] = pInspBoardInfo->partImgBuf.imgTop_B[nIndex];
					}

					if (bSaveWP && bCorrectWP && sInspData.m_pimgSUB && lX < nW && lY < nH)
					{
						nIndex = (lY * nW) + lX;
						bool bSticker = (imgSticker.data[nIndex] == 255);
						sInspData.SUB_DATA(nDst, pnImg3DSub[nIndex], nModuleTotal, nArrModuleID, bSticker);
					}
				}
			}
		}

		if (bSave)
		{
			m_fFrTact[m_eFrTact_GetInspArea_Bin_WP] += ((GetTickCount() - st1) / 1000.0f);
			st1 = GetTickCount();
		}
	}
	sLog.Format(_T("SIZE: %d / %d"), nW, nH);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[SET_FALG]"), sLog, st1);
	m_fFrTact[m_eFrTact_GetInspArea_Bin] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	if (bAM || bIM)
	{
		imgFR[FR_IMG_AM].copyTo(imgFR[FR_IMG_BIN]);
		CalcBlob(imgFR[FR_IMG_BIN], imgFR[FR_IMG_AM], 10, false, true);
		AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), sAMIM, _T("[CalcBlob]"), st1);
	}
	m_fFrTact[m_eFrTact_GetInspArea_AMIM] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	ExceptForeignImage(m_eForeignInsp_Gray, imgFR[FR_IMG_SOLDER], sInspData);
	cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(255));
	cv::bitwise_xor(imgFR[FR_IMG_SOLDER], imgTemp, imgFR[FR_IMG_SOLDER_INV]);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[ExceptForeignImage]"), _T("SOLDER"), st1);
	m_fFrTact[m_eFrTact_GetInspArea_SOLDER] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	if (bSkipPart)
	{
		ExceptForeignImage(m_eForeignInsp_SkipPart, imgFR[FR_IMG_SKIP_PART], sInspData);
		AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[ExceptForeignImage]"), _T("SkipPart"), st1);
	}
	m_fFrTact[m_eFrTact_GetInspArea_WP_SkipPart] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	if (bSaveFR && bSIZE == false)
	{
		cv::Mat imgBoard;
		cv::Mat imgFR3D;
		cv::Mat imgWP3D;
		cv::Mat imgFR3D_Src(nH, nW, CV_32FC1, p3DFr);
		cv::resize(imgFR3D_Src, imgFR3D, cv::Size(nFOVW, nFOVH));
		cv::Mat imgTR(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgTG(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgTB(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
		if (bSaveWP)
		{
			cv::Mat imgWP3D_Src(nH, nW, CV_32FC1, p3DWP);
			cv::resize(imgWP3D_Src, imgWP3D, cv::Size(nFOVW, nFOVH));
			cv::resize(imgInspBoard, imgBoard, cv::Size(nFOVW, nFOVH));
		}

		if (sInspData.m_pimgTopR && sInspData.m_pimgTopG && sInspData.m_pimgTopB)
		{
			if (sFR2DData.m_nCalc == 2 && sFR2DData.m_nFov == nFovID)
			{
				cv::Mat imgR(sFR2DData.m_nH, sFR2DData.m_nW, CV_8UC1, sFR2DData.m_pucTR);
				cv::Mat imgG(sFR2DData.m_nH, sFR2DData.m_nW, CV_8UC1, sFR2DData.m_pucTG);
				cv::Mat imgB(sFR2DData.m_nH, sFR2DData.m_nW, CV_8UC1, sFR2DData.m_pucTB);
				cv::resize(imgR, imgTR, cv::Size(nFOVW, nFOVH));
				cv::resize(imgG, imgTG, cv::Size(nFOVW, nFOVH));
				cv::resize(imgB, imgTB, cv::Size(nFOVW, nFOVH));
			}
			else if (pInspBoardInfo && pInspBoardInfo->partImgBuf.nImageSizeX > 0 && pInspBoardInfo->partImgBuf.nImageSizeY > 0)
			{
				cv::Mat imgR(pInspBoardInfo->partImgBuf.nImageSizeY, pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgTop_R);
				cv::Mat imgG(pInspBoardInfo->partImgBuf.nImageSizeY, pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgTop_G);
				cv::Mat imgB(pInspBoardInfo->partImgBuf.nImageSizeY, pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgTop_B);
				cv::resize(imgR, imgTR, cv::Size(nFOVW, nFOVH));
				cv::resize(imgG, imgTG, cv::Size(nFOVW, nFOVH));
				cv::resize(imgB, imgTB, cv::Size(nFOVW, nFOVH));
			}
		}
		for (int y = sInspData.m_rcFOV.top; y < sInspData.m_rcFOV.bottom && y < sInspData.m_nWP_H; y++)
		{
			if (y < 0) continue;

			for (int x = sInspData.m_rcFOV.left; x < sInspData.m_rcFOV.right && x < sInspData.m_nWP_W; x++)
			{
				if (x < 0) continue;

				long long nDst = ((y * sInspData.m_nWP_W) + x);
				long long nDstIndex = nDst * 3;

				int nX = x - sInspData.m_rcFOV.left;
				int nY = y - sInspData.m_rcFOV.top;
				if (nX < 0 || nX >= nFOVW)
					continue;
				if (nY < 0 || nY >= nFOVH)
					continue;

				long long nIndex = (nY * nFOVW) + nX;
				long long nIndex3 = (long long)nIndex * 3;
				if (bSaveWP)
				{
					if (imgBoard.ptr<UCHAR>(nY)[nX] != 255)
						continue;

					float fWPH = imgWP3D.ptr<float>(nY)[nX];
					SetRGB(sInspData.m_pDesImg, nDstIndex, fWPH, fPseudoColorMin, fPseudoColorMax);
					if (sInspData.m_pimgWP) sInspData.m_pimgWP[nDst] = fWPH;

					if (bCorrectWP && sInspData.m_pimgSUB && nX < nW && nY < nH)
					{
						nIndex = (nY * nW) + nX;
						sInspData.m_pimgSUB[nDst] = pnImg3DSub[nIndex];
						if (imgSticker.data[nIndex] == 255)
							sInspData.m_pimgSUB[nDst] |= FR_SUB_Sticker;
					}
				}

				if (sInspData.m_pimgFR) sInspData.m_pimgFR[nDst] = imgFR3D.ptr<float>(nY)[nX];
				if (sInspData.m_pimgTopR) sInspData.m_pimgTopR[nDst] = imgTR.data[nIndex];
				if (sInspData.m_pimgTopG) sInspData.m_pimgTopG[nDst] = imgTG.data[nIndex];
				if (sInspData.m_pimgTopB) sInspData.m_pimgTopB[nDst] = imgTB.data[nIndex];
			}
		}

#if _DEBUG
		if (sInspData.m_pimgSUB)
		{
			cv::Mat imgSUB(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_32FC1, cv::Scalar(0));
			for (int y = 0; y < sInspData.m_nWP_H; y++)
			{
				for (int x = 0; x < sInspData.m_nWP_W; x++)
				{
					long long nIndex = (y * sInspData.m_nWP_W) + x;
					imgSUB.ptr<float>(y)[x] = sInspData.m_pimgSUB[nIndex];
				}
			}
		}
#endif
		AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[Warpage]"), _T("Image Set"), st1);
	}
	m_fFrTact[m_eFrTact_GetInspArea_WP_IMG] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	if (nArrModuleCnt[0] == 0)
		return false;

	if (nExceptArea > 0)
	{
		nExceptArea += 2;

		if (b3D && nHoleCnt > 10)
		{
			imgHol.copyTo(imgFR[FR_IMG_BLOB]);
			ExceptArea(nExceptArea, imgFR[FR_IMG_BLOB], imgHol, false);
			sLog.Format(_T("ExceptArea %d HoleCnt %d"), nExceptArea, nHoleCnt);
			AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[ExceptArea]"), sLog, st1);
		}
		m_fFrTact[m_eFrTact_GetInspArea_Hole] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

		if (nAMIMCnt > 10)
		{
			if ((bIM || bAM) && bRemoveIM)
			{
				int nMinAMIM = (nW * 3) + (nH * 3);
				if (bIM)
				{
					cv::bitwise_and(imgPart, imgFR[FR_IMG_AM], imgFR[FR_IMG_BIN]);
					cv::bitwise_and(imgExt, imgFR[FR_IMG_BIN], imgFR[FR_IMG_BIN]);
				}
				else
					memcpy(imgFR[FR_IMG_BIN].data, imgFR[FR_IMG_AM].data, sizeof(UCHAR) * nW * nH);

				memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
				std::vector<std::vector<cv::Point>> allRst = CalcBlob(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], nMinAMIM);
				int nAMIMBlob = allRst.size();
				if (nAMIMBlob > 0)
				{
					if (bIM)
						ExceptArea(nExceptAreaIM, imgFR[FR_IMG_BLOB], imgFR[FR_IMG_AM], false);
					else if (bAM)
						ExceptArea(nExceptAreaIM, imgFR[FR_IMG_BLOB], imgFR[FR_IMG_AM], true);
				}
				else
					imgFR[FR_IMG_AM].setTo(0);

				sLog.Format(_T("AMIMCnt %d AMIM_Blob %d"), nAMIMCnt, nAMIMBlob);
				AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[ExceptArea]"), sLog, st1);
			}
		}
		m_fFrTact[m_eFrTact_GetInspArea_AMIM_EX] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();
	}
	if (bIM)
	{
		cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(255));
		cv::bitwise_xor(imgFR[FR_IMG_AM], imgTemp, imgFR[FR_IMG_IM]);
		AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[IM]"), _T("XOR"), st1);
	}
	m_fFrTact[m_eFrTact_GetInspArea_IM_XOR] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	if (b3D)
	{
		cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(255));
		cv::bitwise_xor(imgHol, imgTemp, imgFR[FR_IMG_HOLE]);
		AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[HOLE]"), _T("XOR"), st1);
	}
	m_fFrTact[m_eFrTact_GetInspArea_HOLE_XOR] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	bool bInspArea = false;
	for (int nModuleIDX = nModuleTotal - 1; nModuleIDX > 0; nModuleIDX--)
	{
		int nModuleID = nArrModuleID[nModuleIDX];
		if (nModuleIDX > 0 && nModuleID <= 0)
			continue;

		if (nModuleIDX > 0)
		{
			if (bWP == false)
			{
				if (nArrModuleCnt[nModuleID] > 10)
					bInspArea = true;
			}
		}

		if (nArrModuleCnt[nModuleID] <= 10)
		{
			nArrModuleCnt[nModuleID] = 0;
			cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(255));
			cv::bitwise_xor(imgInspArea[nModuleID], imgTemp, imgTemp);
			cv::bitwise_and(imgInspArea[0], imgTemp, imgInspArea[0]);
			memset(imgInspArea[nModuleID].data, 0, sizeof(UCHAR) * nW * nH);
			continue;
		}

		if (bExceptModule && (nExceptModuleAreaX > 0 || nExceptModuleAreaY > 0 || nExceptModuleArea >= nArrModuleCnt[nModuleID]))
		{
			memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
			std::vector<std::vector<cv::Point>> allRst = CalcBlob(imgInspArea[nModuleID], imgFR[FR_IMG_BLOB], 10);
			if (allRst.size() > 0)
			{
				cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
				cv::Mat imgTempA(nH, nW, CV_8UC1, cv::Scalar(0));
				cv::Mat imgTempOUT(nH, nW, CV_8UC1, cv::Scalar(0));
				memset(imgFR[FR_IMG_BIN].data, 0, sizeof(UCHAR) * nW * nH);
				for (int a = 0; a < allRst.size(); a++)
				{
					cv::Rect roi = cv::boundingRect(allRst[a]);
					imgTemp.setTo(0);
					imgTempA.setTo(0);
					cv::drawContours(imgTemp, allRst, a, cv::Scalar(255), cv::FILLED);
					cv::bitwise_and(imgTemp, imgFR[FR_IMG_BLOB], imgTempA);
					bool bInput = false;
					if ((nExceptModuleAreaX < roi.width && nExceptModuleAreaY < roi.height))
					{
						int nTotalCnt = cv::countNonZero(imgTemp);

						double dPer = ((double)nTotalCnt / (double)(roi.width * roi.height)) * 100.0;
						if (dPer >= 60)
						{
							cv::bitwise_or(imgTempA, imgFR[FR_IMG_BIN], imgFR[FR_IMG_BIN]);
							bInput = true;
						}
					}
					if (bInput == false)
						cv::bitwise_or(imgTempA, imgTempOUT, imgTempOUT);
				}

				cv::Mat imgREMOVE(nH, nW, CV_8UC1, cv::Scalar(255));
				cv::bitwise_xor(imgREMOVE, imgTempOUT, imgREMOVE);
				cv::bitwise_and(imgInspArea[0], imgREMOVE, imgInspArea[0]);
				memcpy(imgInspArea[nModuleID].data, imgFR[FR_IMG_BIN].data, sizeof(UCHAR) * nW * nH);
			}
			else
				imgInspArea[nModuleID].setTo(0);
			nArrModuleCnt[nModuleID] = cv::countNonZero(imgInspArea[nModuleID]);
		}
		if (nArrModuleCnt[nModuleID] > 10)
			bInspArea = true;
	}
	sLog.Format(_T("ModuleTotal %d Board %d"), nModuleTotal, nArrModuleCnt[0]);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[Module]"), sLog, st1);

	if (bInspArea == false)
	{
		if (nArrModuleCnt[0] > 10)
			bInspArea = true;
	}
	m_fFrTact[m_eFrTact_GetInspArea_Module] += ((GetTickCount() - st) / 1000.0f); st = GetTickCount();

	if (bSave)
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Except_1"));
		cv::imwrite(std::string(CT2A(str)), imgExt);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Part_2"));
		cv::imwrite(std::string(CT2A(str)), imgPart);

		cv::Mat imgFrEXT = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
		ExceptForeign(ForeignParamROI_Data_Foreign, imgFrEXT, sInspData);
		cv::Mat imgFrEXTA;
		cv::bitwise_and(imgFrEXT, imgExt, imgFrEXTA);

		str = GetForeignDebugDataFullPath(nFovID, _T("_BTM_4"));
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_BOTM]);

		str = GetForeignDebugDataFullPath(nFovID, _T("_IM_8"));
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_IM]);

		str = GetForeignDebugDataFullPath(nFovID, _T("_AM_8"));
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_AM]);

		str = GetForeignDebugDataFullPath(nFovID, _T("_HOLE_10"));
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_HOLE]);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Sticker"));
		cv::imwrite(std::string(CT2A(str)), imgSticker);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Manual_EXT"));
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_EXT]);

		for (int nModuleIDX = 0; nModuleIDX < nModuleTotal; nModuleIDX++)
		{
			int nModuleID = nArrModuleID[nModuleIDX];
			if (nModuleIDX > 0 && nModuleID <= 0)
				break;

			if (nExceptModuleArea > nArrModuleCnt[nModuleID])
				continue;

			CString cName = _T("");
			cName.Format(_T("_MODULE_%d"), nModuleID);
			str = GetForeignDebugDataFullPath(nFovID, cName);
			cv::imwrite(std::string(CT2A(str)), imgInspArea[nModuleID]);
		}
	}

	AddLOG(_T("[FOREIGN_INSP]"), _T("[GetInspArea]"), _T("[Module]"), _T("END"), st1);
	return bInspArea;
}
void FOREIGN_INSP::ExceptArea(int nExceptArea, cv::Mat imgSrc, cv::Mat imgDst, bool bMorErode)
{
	DWORD st = GetTickCount();
	if (nExceptArea <= 0)
		return;
	imgDst.setTo(0);
	if (bMorErode)
		MorErode(imgSrc, imgDst, 0, nExceptArea);
	else
	{
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nExceptArea, nExceptArea));
		cv::dilate(imgSrc, imgDst, kernel);
	}
}
bool FOREIGN_INSP::InspWarpage(InspForeignInfo sForeign, float* p3DWP, cv::Mat imgInspArea, cv::Mat *imgFR, int nW, int nH, int nModuleID, int nFovID, int nAM, ForeignData sInspData, InspFovForeignResult* ForeignResult)
{
#if _DEBUG
	cv::Mat img3D(nH, nW, CV_32FC1, p3DWP);
#endif
	DWORD st = GetTickCount();
	FR_WP sData = sForeign.sWP;
	int nDivide = sForeign.GetDivide();
	int nExceptArea = sForeign.sWP.GetRemove(nW, nH, nDivide);
	int nXInterval = (nAM == 0) ? 20 / nDivide : 8 / nDivide;
	int nYInterval = (nAM == 0) ? 20 / nDivide : 8 / nDivide;
	bool bSaveImg = sForeign.UseData(FR_DATA_DebugData);
	bool bSaveImg2 = sForeign.UseData(FR_DATA_DebugData2);
	bool bBTC = sData.UseData(FR_WP_DT_BTC);
	bool bIM = sData.UseData(FR_WP_DT_IM);
	bool bAM = sData.UseData(FR_WP_DT_AM);
	bool bRemoveWP = sData.UseData(FR_WP_DT_RemoveArea_WP);
	bool bHole = sData.UseData(FR_WP_DT_Not_Hole);
	bool bNot_FR = sForeign.sWP.UseData(FR_WP_DT_Not_FR);
	bool bCorrectWP = sForeign.sWP.UseData(FR_WP_DT_CorrectWP);
	bool bCorrectWP_B = sForeign.sWP.UseData(FR_WP_DT_CorrectWP_B);
	if (bIM || bAM) bCorrectWP = false;
	if (nAM > 0)
	{
		if (nModuleID == 0 || bAM == false)
			return true;
	}

	ForeignResult->m_stForeignWP[nModuleID].m_fHeightMin = HUGE_VAL;
	ForeignResult->m_stForeignWP[nModuleID].m_fHeightMax = -HUGE_VAL;
	ForeignResult->m_stForeignWP[nModuleID].m_fHeightMinAM = HUGE_VAL;
	ForeignResult->m_stForeignWP[nModuleID].m_fHeightMaxAM = -HUGE_VAL;

	memset(imgFR[FR_IMG_BIN].data, 0, sizeof(UCHAR) * nW * nH);
	memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
	if (nAM == 1)
		cv::bitwise_or(imgFR[FR_IMG_BIN], imgFR[FR_IMG_AM], imgFR[FR_IMG_BIN]);
	else
		cv::bitwise_or(imgFR[FR_IMG_BIN], imgInspArea, imgFR[FR_IMG_BIN]);
	if (bHole == false)
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_HOLE], imgFR[FR_IMG_BIN]);
	if (bBTC)
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BOTM], imgFR[FR_IMG_BIN]);
	cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_HEXT], imgFR[FR_IMG_BIN]);
	if (bIM)
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_IM], imgFR[FR_IMG_BIN]);
	if (bNot_FR == false)
		GetExceptFR(sForeign, sInspData, ForeignResult, imgFR[FR_IMG_BIN], nW, nH);

	m_fFrTact[m_eFrTact_InspWP_Set] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	POINTF poMin; POINTF poMax;
	poMin.x = 0; poMin.y = 0;
	poMax.x = 0; poMax.y = 0;

	int nWPMaxCnet = (bRemoveWP) ? 20 : 50;
	int nMinArea = (nW / 50) * (nH / 50);
	if (nMinArea < 100) nMinArea = 100;
	std::vector<std::vector<cv::Point>> allRst = CalcBlob(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], nMinArea);
	if (allRst.size() > 0)
	{
		if (bSaveImg)
		{
			CString strName = _T("");
			strName.Format(_T("_InspWarpage_BLOB_%d_%d"), nModuleID, nAM);
			CString str = GetForeignDebugDataFullPath(nFovID, strName);
			SaveDebugImg(imgFR[FR_IMG_BLOB], str);
		}
		imgFR[FR_IMG_BLOB].copySize(imgFR[FR_IMG_BIN]);
		ExceptArea(nExceptArea, imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], true);
	}
	else
	{
		InspWarpageGood(nModuleID, nAM, ForeignResult);
		return true;
	}

	if (bCorrectWP)
	{
		if (sInspData.m_pimgWPInspA)
		{
#if _DEBUG
			cv::Mat imgWPInspA(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_8UC1, sInspData.m_pimgWPInspA);
#endif
		for (int y = sInspData.m_rcFOV.top; y < sInspData.m_rcFOV.bottom && y < sInspData.m_nWP_H; y++)
		{
			if (y < 0) continue;
			for (int x = sInspData.m_rcFOV.left; x < sInspData.m_rcFOV.right && x < sInspData.m_nWP_W; x++)
			{
				if (x < 0) continue;
				long long nDst = ((y * sInspData.m_nWP_W) + x);
				int nX = x - sInspData.m_rcFOV.left;
				int nY = y - sInspData.m_rcFOV.top;
				if (nX < 0 || nX >= nW)
					continue;
				if (nY < 0 || nY >= nH)
					continue;

					long long nIndex = (nY * nW) + nX;
					if (imgInspArea.data[nIndex] != 255)
						continue;

					sInspData.m_pimgWPInspA[nDst] = imgFR[FR_IMG_BLOB].data[nIndex];
				}
			}
		}
		if (bCorrectWP_B)
		{
			InspWarpageGood(nModuleID, nAM, ForeignResult);
			return true;
		}
	}
	cv::Rect roiA = cv::boundingRect(imgFR[FR_IMG_BLOB]);
	m_fFrTact[m_eFrTact_InspWP_Area] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	bool bRet = InspWarpage(sForeign, p3DWP, imgFR[FR_IMG_BLOB], nW, nH, nModuleID, nAM, sInspData, ForeignResult, roiA);
	if (sInspData.m_vInspectionMode == eMI_WindowTeaching && sInspData.m_pDesImgBin)
	{
		if (m_eForeignInsp_Total > sInspData.m_nMode)
		{
			if (sInspData.m_nMode == m_eForeignInsp_Warpage)
				memcpy(sInspData.m_pDesImgBin, imgFR[FR_IMG_BLOB].data, sizeof(UCHAR) * nW * nH);
		}
		else
		{
			int nTempMode = sInspData.m_nMode - m_eForeignInsp_Total;
			if (nTempMode == m_eForeignInsp_Warpage)
				memcpy(sInspData.m_pDesImgBin, imgFR[FR_IMG_BLOB].data, sizeof(UCHAR) * nW * nH);
		}
	}

	if (bSaveImg || (bSaveImg2 && bRet == false))
	{
		CString strName = _T("");
		strName.Format(_T("_INSP_WP_%d_%d"), nModuleID, nAM);
		CString str = GetForeignDebugDataFullPath(nFovID, strName);
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_BLOB]);

		if (bAM)
		{
			strName.Format(_T("_INSP_WP_AM_%d_%d"), nModuleID, nAM);
			str = GetForeignDebugDataFullPath(nFovID, strName);
			cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_AM]);
		}
		else
		{
			strName.Format(_T("_INSP_WP_AREA_%d_%d"), nModuleID, nAM);
			str = GetForeignDebugDataFullPath(nFovID, strName);
			cv::imwrite(std::string(CT2A(str)), imgInspArea);
		}

		strName.Format(_T("_INSP_WP_HOLE_%d_%d"), nModuleID, nAM);
		str = GetForeignDebugDataFullPath(nFovID, strName);
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_HOLE]);

		strName.Format(_T("_INSP_WP_HEXT_%d_%d"), nModuleID, nAM);
		str = GetForeignDebugDataFullPath(nFovID, strName);
		cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_HEXT]);

		if (bBTC)
		{
			strName.Format(_T("_INSP_WP_BTC_%d_%d"), nModuleID, nAM);
			str = GetForeignDebugDataFullPath(nFovID, strName);
			cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_BOTM]);
		}

		if (bIM)
		{
			strName.Format(_T("_INSP_WP_IM_%d_%d"), nModuleID, nAM);
			str = GetForeignDebugDataFullPath(nFovID, strName);
			cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_IM]);
		}
	}

	m_fFrTact[m_eFrTact_InspWP_END] += ((GetTickCount() - st) / 1000.0f);
	return bRet;
}
void FOREIGN_INSP::InspWarpageGood(int nModuleID, int nAM, InspFovForeignResult* ForeignResult)
{
	int nIndexWP2 = -1;
	POINTF poMin; POINTF poMax;
	poMin.x = 0; poMin.y = 0;
	poMax.x = 0; poMax.y = 0;
	for (int b = 0; b < ForeignResult->m_nCountDefectWP; b++)
	{
		if (ForeignResult->m_stForeignWP[b].nID == nModuleID)
		{
			nIndexWP2 = b;
			break;
		}
	}
	if (nIndexWP2 == -1 || (nIndexWP2 >= 0 && nIndexWP2 >= ForeignResult->m_nCountDefectWP))
	{
		ForeignResult->m_nCountDefectWP++;
		nIndexWP2 = ForeignResult->m_nCountDefectWP - 1;
	}
	ForeignResult->m_stForeignWP[nIndexWP2].nID = nModuleID;
	if (nAM == 1)
	{
		if (ForeignResult->m_stForeignWP[nIndexWP2].m_byAM != 1)
		{
			ForeignResult->m_stForeignWP[nIndexWP2].m_byAM = 2;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMinAM = 0;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMaxAM = 0;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMinAM = poMin.x;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMinAM = poMin.y;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMaxAM = poMax.x;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMaxAM = poMax.y;
		}
	}
	else
	{
		if (ForeignResult->m_stForeignWP[nIndexWP2].m_byWP != 1)
		{
			ForeignResult->m_stForeignWP[nIndexWP2].m_byWP = 2;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMin = 0;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMax = 0;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMin = poMin.x;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMin = poMin.y;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMax = poMax.x;
			ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMax = poMax.y;
		}
	}
}
bool FOREIGN_INSP::InspWarpage(InspForeignInfo sForeign, float* p3DWP, cv::Mat imgInspArea, int nW, int nH, int nModuleID, int nAM, ForeignData sInspData, InspFovForeignResult* ForeignResult, cv::Rect rcROI)
{
	DWORD st = GetTickCount();
	FR_WP sData = sForeign.sWP;
	int nDivide = sForeign.GetDivide();
	int nXInterval = (nAM == 0) ? 20 / nDivide : 8 / nDivide;
	int nYInterval = (nAM == 0) ? 20 / nDivide : 8 / nDivide;
	CheckWPMargin(sForeign, sInspData, nW, nH, nModuleID, rcROI);

	POINTF poMin; POINTF poMax;
	poMin.x = 0; poMin.y = 0;
	poMax.x = 0; poMax.y = 0;
	float fMin = HUGE_VAL, fMax = -HUGE_VAL;
	for (int nRecheck = 0; nRecheck < 2; nRecheck++)
	{
		bool bCheckMin = (fMin == HUGE_VAL || fMin == -HUGE_VAL);
		bool bCheckMax = (fMax == HUGE_VAL || fMax == -HUGE_VAL);
		for (int y = rcROI.y; y <= rcROI.br().y; y += nYInterval)
		{
			if (y < 0 || y >= nH)
				continue;
			for (int x = rcROI.x; x <= rcROI.br().x; x += nXInterval)
			{
				if (x < 0 || x >= nW || y < 0 || y >= nH)
					continue;
				long long nIndex = (y * nW) + x;
				if (imgInspArea.data[nIndex] == 0) continue;
				bool bFind = true;
				int nXGap = 4 - (nRecheck * 2);
				int nYGap = 4 - (nRecheck * 2);
				float fSumH = 0;
				int nGapCnt = 0;
				for (int y2 = 0; y2 < nYGap + 1; y2++)
				{
					for (int x2 = 0; x2 < nXGap + 1; x2++)
					{
						int nX = x;
						int nY = y;
						if (x2 < (nXGap / 2)) nX -= (1 + x2);
						if (x2 > (nXGap / 2)) nX += (x2 - (nXGap / 2));
						if (y2 < (nYGap / 2)) nY -= (1 + y2);
						if (y2 > (nYGap / 2)) nY += (y2 - (nYGap / 2));
						long long nIndex2 = (nY * nW) + nX;
						if (nX < 0 || nY < 0 || nX >= nW || nY >= nH ||
							nIndex2 < 0 || nIndex2 >= nW * nH || imgInspArea.data[nIndex2] == 0)
						{
							bFind = false;
							break;
						}
						fSumH += p3DWP[nIndex2];
						nGapCnt++;
					}
					if (bFind == false) break;
				}
				if (bFind == false) continue;
				float fAVG = fSumH / (float)nGapCnt;
				if (bCheckMin && fMin > fAVG) { fMin = fAVG; poMin.x = x; poMin.y = y; }
				if (bCheckMax && fMax < fAVG) { fMax = fAVG; poMax.x = x; poMax.y = y; }
			}
		}
		if (fMin != HUGE_VAL && fMin != -HUGE_VAL && fMax != HUGE_VAL && fMax != -HUGE_VAL)
			break;
	}

	m_fFrTact[m_eFrTact_InspWP_Check] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	if (fMin == HUGE_VAL || fMin == -HUGE_VAL || fMax == HUGE_VAL || fMax == -HUGE_VAL)
	{
		int nIndexWP2 = -1;
		for (int b = 0; b < ForeignResult->m_nCountDefectWP; b++)
		{
			if (ForeignResult->m_stForeignWP[b].nID == nModuleID)
			{
				nIndexWP2 = b;
				break;
			}
		}
		if (nIndexWP2 == -1 || (nIndexWP2 >= 0 && nIndexWP2 >= ForeignResult->m_nCountDefectWP))
		{
			ForeignResult->m_nCountDefectWP++;
			nIndexWP2 = ForeignResult->m_nCountDefectWP - 1;
		}
		ForeignResult->m_stForeignWP[nIndexWP2].nID = nModuleID;
		if (nAM == 1)
		{
			if (ForeignResult->m_stForeignWP[nIndexWP2].m_byAM != 1)
			{
				ForeignResult->m_stForeignWP[nIndexWP2].m_byAM = 2;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMinAM = 0;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMaxAM = 0;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMinAM = poMin.x;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMinAM = poMin.y;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMaxAM = poMax.x;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMaxAM = poMax.y;
			}
		}
		else
		{
			if (ForeignResult->m_stForeignWP[nIndexWP2].m_byWP != 1)
			{
				ForeignResult->m_stForeignWP[nIndexWP2].m_byWP = 2;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMin = 0;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fHeightMax = 0;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMin = poMin.x;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMin = poMin.y;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCXMax = poMax.x;
				ForeignResult->m_stForeignWP[nIndexWP2].m_fCYMax = poMax.y;
			}
		}
		return true;
	}

	int nIndexWP = -1;
	for (int b = 0; b < ForeignResult->m_nCountDefectWP; b++)
	{
		if (ForeignResult->m_stForeignWP[b].nID == nModuleID)
		{
			nIndexWP = b;
			break;
		}
	}

	if (nIndexWP == -1 || (nIndexWP >= 0 && nIndexWP >= ForeignResult->m_nCountDefectWP))
	{
		ForeignResult->m_nCountDefectWP++;
		nIndexWP = ForeignResult->m_nCountDefectWP - 1;
	}

	m_fFrTact[m_eFrTact_InspWP_SetWP] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	float fGap = 0.0f;
	ForeignResult->m_stForeignWP[nIndexWP].nID = nModuleID;
	if (nAM == 1)
	{
		ForeignResult->m_stForeignWP[nIndexWP].m_byAM = 1;
		if (ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMinAM > fMin)
		{
			ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMinAM = fMin;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCXMinAM = poMin.x;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCYMinAM = poMin.y;
		}
		if (ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMaxAM < fMax)
		{
			ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMaxAM = fMax;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCXMaxAM = poMax.x;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCYMaxAM = poMax.y;
		}

		fGap = ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMaxAM - ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMinAM;
	}
	else
	{
		ForeignResult->m_stForeignWP[nIndexWP].m_byWP = 1;
		if (ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMin > fMin)
		{
			ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMin = fMin;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCXMin = poMin.x;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCYMin = poMin.y;
		}
		if (ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMax < fMax)
		{
			ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMax = fMax;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCXMax = poMax.x;
			ForeignResult->m_stForeignWP[nIndexWP].m_fCYMax = poMax.y;
		}

		fGap = ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMax - ForeignResult->m_stForeignWP[nIndexWP].m_fHeightMin;
	}

	m_fFrTact[m_eFrTact_InspWP_Insp] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	float fT = sData.m_fArr[FR_WP_F_HeightMaxMin_WP];
	float fTM = sData.m_fArr[FR_WP_F_HeightMaxMin_WPM];
	float fTeach = (ForeignResult->m_stForeignWP[nIndexWP].nID > 0) ? fTM : fT;
	bool bRet = (fTeach >= fGap);
	m_fFrTact[m_eFrTact_InspWP_END] += ((GetTickCount() - st) / 1000.0f);
	return bRet;
}
bool FOREIGN_INSP::InspForeign(InspForeignInfo sForeign, float* p3D, cv::Mat imgInspArea, cv::Mat *imgFR, cv::Mat *imgORG2D, int nW, int nH, int nMode, int nFovID, ForeignData sInspData, InspFovForeignResult* ForeignResult, FR2DData sFR2DData)
{
	CString sLog = _T("");
	DWORD st = GetTickCount();
	DWORD stT = GetTickCount();

	int nContainModuleID = sInspData.m_nContainModuleID;
	MPTI_InspectionMode vInspectionMode = sInspData.m_vInspectionMode;

	InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;
	int nDivide = sForeign.GetDivide();
	bool bSaveImg = sForeign.UseData(FR_DATA_DebugData);
	bool bSaveImg2 = sForeign.UseData(FR_DATA_DebugData2);
	bool bWP = sForeign.IsForeignEnable(m_eForeignInsp_Warpage);
	bool bIM = sForeign.sWP.UseData(FR_WP_DT_IM) && bWP;
	bool bAM = sForeign.sWP.UseData(FR_WP_DT_AM) && bWP;

	FR_Bin sData = sForeign.sBin[nMode];
	bool bBTC = sData.UseData(FR_BIN_DT_ExceptBTC);

	cv::Mat imgColor(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgInspAreaTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	if (bSaveImg || bSaveImg2)
	{
		imgFR[FR_IMG_BIN].copyTo(imgInspAreaTemp);
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_INSP_AREA"), nMode);
		if (bSaveImg)
			SaveDebugImg(imgInspAreaTemp, str);
	}
	bool bColor = sData.UseData(FR_BIN_DT_Color);
	bool bGray = sData.UseData(FR_BIN_DT_Gray);
	bool bUse2D = bColor || bGray;
	bool bUse3D = sData.UseData(FR_BIN_DT_3D);
	bool bNotSizeChk = sData.UseData(FR_BIN_DT_NotSize);
	bool bSIZE_AREA = sData.UseData(FR_BIN_DT_SIZE_AREA);

	bool bCircleCheck = sData.UseData(FR_BIN_DT_CircleCheck);
	bool bCircleD = sData.UseData(FR_BIN_DT_CircleDiameter);
	bool bCircleP = sData.UseData(FR_BIN_DT_CirclePercent);
	double dCircleD_Min = sData.m_fArr[FR_BIN_F_CircleDiameterMin];
	double dCircleD_Max = sData.m_fArr[FR_BIN_F_CircleDiameterMax];
	double dCircleP = sData.m_fArr[FR_BIN_F_CirclePercent];
	double dCircleD_Min_Pix = dCircleD_Min / m_resolX / nDivide;
	double dCircleD_Max_Pix = dCircleD_Max / m_resolX / nDivide;

	double dArea = sData.m_arrINSP[m_eFR_Bin_INSP_A].m_fArr[m_eFR_Bin_INSP_F_Min];
	double dAreaPix = dArea / m_resolX / m_resolY / (nDivide * nDivide);
	int nMinPix = 16 / nDivide;
	if (dAreaPix <= nMinPix) dAreaPix = nMinPix;

	double dAreaRoot = sqrt(dArea);
	int nAreaRootPixX = (int)(dAreaRoot / m_resolX / nDivide * 0.7);
	int nAreaRootPixY = (int)(dAreaRoot / m_resolY / nDivide * 0.7);
	if (nAreaRootPixX <= nDivide) nAreaRootPixX = nDivide;
	if (nAreaRootPixY <= nDivide) nAreaRootPixY = nDivide;

	int nRange3D = sData.m_nArr[FR_BIN_N_Range3D];
	float f3DMin = sData.m_fArr[FR_BIN_F_Min3D];
	float f3DMax = sData.m_fArr[FR_BIN_F_Max3D];
	if (f3DMin > f3DMax) f3DMax = f3DMin;

	int nCluster = sForeign.m_nArrData[FR_N_PixelClusteringCnt];
	if (nCluster < 0) nCluster = 0;
	bool bCluster = (sForeign.UseData(FR_DATA_UseClustering) && nCluster > 0);
	if (bCluster == false) nCluster = 0;

	m_fFrTact[m_eFrTact_InspFR_SET] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	if (bUse2D)
	{
		if (sInspData.m_vInspectionMode == eMI_Inspection)
		{
			switch (nMode)
			{
			case m_eForeignInsp_Foreign:
				if (sFR2DData.m_nCalc == 2 && sFR2DData.m_nFov == nFovID)
					memcpy(imgColor.data, sFR2DData.m_pFr1, imgColor.cols * imgColor.rows);
				else
					memcpy(imgColor.data, pInspBoardInfo->partImgBuf.imgSide1_R, imgColor.cols * imgColor.rows);
				break;
			case m_eForeignInsp_Foreign2D:
				if (sFR2DData.m_nCalc == 2 && sFR2DData.m_nFov == nFovID)
					memcpy(imgColor.data, sFR2DData.m_pFr2, imgColor.cols * imgColor.rows);
				else
					memcpy(imgColor.data, pInspBoardInfo->partImgBuf.imgSide1_G, imgColor.cols * imgColor.rows);
				break;
			case m_eForeignInsp_Gray:
				if (sFR2DData.m_nCalc == 2 && sFR2DData.m_nFov == nFovID)
					memcpy(imgColor.data, sFR2DData.m_pFr3, imgColor.cols * imgColor.rows);
				else
					memcpy(imgColor.data, pInspBoardInfo->partImgBuf.imgSide1_B, imgColor.cols * imgColor.rows);
				break;
			case m_eForeignInsp_Copper:
				if (sFR2DData.m_nCalc == 2 && sFR2DData.m_nFov == nFovID)
					memcpy(imgColor.data, sFR2DData.m_pFr4, imgColor.cols * imgColor.rows);
				else
					memcpy(imgColor.data, pInspBoardInfo->partImgBuf.imgSide2_R, imgColor.cols * imgColor.rows);
				break;
			}
		}
		else
		{
			UCHAR* pucGray = NULL;
			if (sInspData.m_vInspectionMode == eMI_WindowTeaching && sInspData.m_pDesImgBin)
			{
				if (sInspData.m_nBin == 1)
					pucGray = sInspData.m_pDesImgBin;
			}
			Get2DForeign(sForeign, sData, nMode, pInspBoardInfo, imgColor.data, sInspData.m_vInspectionMode, imgORG2D, nDivide, nFovID, bSaveImg, pucGray);
#if _DEBUG
			CString sDir;
			sDir.Format(_T("D:\\Eagle3D_data\\ForeignDebugData\\DEBUG\\%d"), nFovID);
			if (GetFileAttributes(sDir) != -1)
			{
				CString csAddNameORG = _T("_Insp_FR_Color");
				CString str = GetForeignDebugDataFullPath(nFovID, csAddNameORG, nMode, 0, true);
				cv::Mat imgColorTemp = cv::imread(std::string(CT2A(str)), cv::IMREAD_GRAYSCALE);
				if (imgColorTemp.empty() == false && imgColorTemp.cols == imgColor.cols && imgColorTemp.rows == imgColor.rows)
					imgColorTemp.copyTo(imgColor);
			}
#endif
		}
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgColor, imgFR[FR_IMG_BIN]);
	}
	if (bUse3D)
	{
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_HOLE], imgFR[FR_IMG_BIN]);
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_HEXT], imgFR[FR_IMG_BIN]);
		cv::Mat img3D(nH, nW, CV_32FC1, p3D);
		if (nRange3D == ETypeInspRange::eTypeRangeOut)
		{
			cv::Mat img3DMin(nH, nW, CV_8UC1, cv::Scalar(0));
			cv::Mat img3DMax(nH, nW, CV_8UC1, cv::Scalar(0));

			ippiCompareC_32f_C1R(p3D, img3D.step, f3DMax, img3DMin.data, img3DMin.cols, { nW, nH }, ippCmpGreaterEq);
			ippiCompareC_32f_C1R(p3D, img3D.step, f3DMin, img3DMax.data, img3DMax.cols, { nW, nH }, ippCmpLessEq);

			memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
			cv::bitwise_or(img3DMin, img3DMax, imgFR[FR_IMG_BLOB]);
			cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BIN]);
		}
		else
		{
			if (nRange3D != ETypeInspRange::eTypeRangeLower)
			{
				memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
				ippiCompareC_32f_C1R(p3D, img3D.step, f3DMin, imgFR[FR_IMG_BLOB].data, imgFR[FR_IMG_BLOB].cols, { nW, nH }, ippCmpGreaterEq);
				cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BIN]);
			}
			if (nRange3D != ETypeInspRange::eTypeRangeUpper)
			{
				memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
				ippiCompareC_32f_C1R(p3D, img3D.step, f3DMax, imgFR[FR_IMG_BLOB].data, imgFR[FR_IMG_BLOB].cols, { nW, nH }, ippCmpLessEq);
				cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BIN]);
			}
		}
	}
	int nBinCnt = cv::countNonZero(imgFR[FR_IMG_BIN]);
	sLog.Format(_T("[%d] : %d | %.3f"), nMode, nBinCnt, dAreaPix);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspForeign]"), _T("[BIN Area]"), sLog, stT);
	if (nBinCnt < dAreaPix)
		return true;

	m_fFrTact[m_eFrTact_InspFR_Bin] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	if (bNotSizeChk == false)
	{
		int nNotSize = sData.m_nArr[FR_BIN_N_NOT_SIZE];
		bool bSizeChk = SizeChk(imgFR[FR_IMG_BIN], dArea, nDivide, nMode, nNotSize);
		sLog.Format(_T("[%d] : %.3f"), nMode, dArea);
		AddLOG(_T("[FOREIGN_INSP]"), _T("[InspForeign]"), _T("[NotSizeChk]"), sLog, stT);
		if (bSizeChk)
			return true;
	}

	m_fFrTact[m_eFrTact_InspFR_SizeChk] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

#if _DEBUG
	// 	CString strD = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Blob"), nMode, 0, true);
	// 	imgFR[FR_IMG_BIN] = cv::imread(std::string(CT2A(strD)), cv::IMREAD_GRAYSCALE);
#endif

	memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
	std::vector<std::vector<cv::Point>> allBlob = CalcBlob(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], (int)dAreaPix);

	if (sInspData.m_vInspectionMode == eMI_WindowTeaching && sInspData.m_pDesImgBin)
	{
		bool bSave = true;
		if (sInspData.m_nBin > 0)
		{
			if (bGray == true)
				bSave = false;
		}
		if (bSave)
		{
			if (m_eForeignInsp_Total > sInspData.m_nMode)
			{
				if (sInspData.m_nMode == nMode)
					memcpy(sInspData.m_pDesImgBin, imgFR[FR_IMG_BLOB].data, sizeof(UCHAR) * nW * nH);
			}
			else
			{
				int nModeTemp = sInspData.m_nMode - m_eForeignInsp_Total;
				if (nModeTemp == nMode)
					memcpy(sInspData.m_pDesImgBin, imgFR[FR_IMG_BLOB].data, sizeof(UCHAR) * nW * nH);
			}
		}
	}

	m_fFrTact[m_eFrTact_InspFR_Blob] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();
	int nCntBlob = allBlob.size();
	sLog.Format(_T("[%d] : %d"), nMode, nCntBlob);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspForeign]"), _T("[CalcBlob]"), sLog, stT);
	if (nCntBlob <= 0)
		return true;

	double *dArrArea = NULL;
	CRect *rcArrBlob = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrArea, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &rcArrBlob, nCntBlob);
	memset(dArrArea, 0, sizeof(double) * nCntBlob);
	memset(rcArrBlob, 0, sizeof(CRect) * nCntBlob);

	byte *bArrIN = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bArrIN, nCntBlob);
	memset(bArrIN, 0, sizeof(byte) * nCntBlob);

	cv::Mat imgTemp = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTempA = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
	for (int a = 0; a < nCntBlob; a++)
	{
		imgTemp.setTo(0);
		imgTempA.setTo(0);
		cv::drawContours(imgTemp, allBlob, a, cv::Scalar(255), cv::FILLED);
		cv::bitwise_and(imgTemp, imgFR[FR_IMG_BLOB], imgTempA);
		cv::Rect roi = cv::boundingRect(allBlob[a]);

		dArrArea[a] = cv::countNonZero(imgTempA);
		rcArrBlob[a] = CRect(roi.x, roi.y, roi.br().x, roi.br().y);
		if (bCircleCheck)
		{
			if (bCircleD)
			{
				int nROI_FR_W = rcArrBlob[a].Width();
				int nROI_FR_H = rcArrBlob[a].Height();
				if (dCircleD_Min_Pix > nROI_FR_W || dCircleD_Max_Pix < nROI_FR_W)
				{
					bArrIN[a] = 2;
					continue;
				}
				if (dCircleD_Min_Pix > nROI_FR_H || dCircleD_Max_Pix < nROI_FR_H)
				{
					bArrIN[a] = 2;
					continue;
				}
			}
			std::vector<CRect> vBlobRects;
			vBlobRects.push_back(CRect(rcArrBlob[a].left, rcArrBlob[a].top, rcArrBlob[a].right, rcArrBlob[a].bottom));
			float fResult(0.0);
			int nMaxIndex(0);
			InspSolderBall(imgTempA, vBlobRects, &nMaxIndex, &fResult);
			if (bCircleP)
			{
				float fResultPer = fResult * 100.0f;
				if (fResultPer < dCircleP)
					bArrIN[a] = 2;
			}
		}
	}
	sLog.Format(_T("[%d] : %d"), nMode, nCntBlob);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspForeign]"), _T("[Merge Data_1]"), sLog, stT);

	int nDefect = sData.GetDEFECT();
	int nMaxRst = 2000;
	int nRealCnt = ForeignResult->m_nCountDefect;
	for (int a = 0; a < nCntBlob; a++)
	{
		if (bArrIN[a] == 2) continue;
		if (ChekcForignROI(sData, rcArrBlob[a], dArrArea[a], nAreaRootPixX, nAreaRootPixY, nDivide, bNotSizeChk) == FALSE)
		{
			bArrIN[a] = 2;
			continue;
		}

		for (int c = 0; c < nCntBlob; c++)
		{
			if (bCluster == false)
				break;
			int nClu = 0;
			for (int b = 0; b < nCntBlob; b++)
			{
				double dAreaB = dArrArea[b];
				if (a == b || bArrIN[b] == 2)
					continue;

				CRect rtTemp;
				rtTemp.left = rcArrBlob[a].left - nCluster;
				rtTemp.right = rcArrBlob[a].right + nCluster;
				rtTemp.top = rcArrBlob[a].top - nCluster;
				rtTemp.bottom = rcArrBlob[a].bottom + nCluster;

				CRect rtDst;
				if (rtDst.IntersectRect(rtTemp, rcArrBlob[b]) == FALSE)
					continue;

				if (bArrIN[b] == 1)
				{
					if (rcArrBlob[b].left > rcArrBlob[a].left) rcArrBlob[b].left = rcArrBlob[a].left;
					if (rcArrBlob[b].top > rcArrBlob[a].top) rcArrBlob[b].top = rcArrBlob[a].top;
					if (rcArrBlob[b].right < rcArrBlob[a].right) rcArrBlob[b].right = rcArrBlob[a].right;
					if (rcArrBlob[b].bottom < rcArrBlob[a].bottom) rcArrBlob[b].bottom = rcArrBlob[a].bottom;
					dArrArea[b] += dArrArea[a];
					bArrIN[a] = 2;
					break;
				}
				if (rcArrBlob[a].left > rcArrBlob[b].left) rcArrBlob[a].left = rcArrBlob[b].left;
				if (rcArrBlob[a].top > rcArrBlob[b].top) rcArrBlob[a].top = rcArrBlob[b].top;
				if (rcArrBlob[a].right < rcArrBlob[b].right) rcArrBlob[a].right = rcArrBlob[b].right;
				if (rcArrBlob[a].bottom < rcArrBlob[b].bottom) rcArrBlob[a].bottom = rcArrBlob[b].bottom;
				dArrArea[a] += dAreaB;
				bArrIN[b] = 2;
				nClu++;
			}
			if (nClu == 0)
				break;
		}
		if (bArrIN[a] == 2)
			continue;

		for (int b = 0; b < nCntBlob; b++)
		{
			double dAreaB = dArrArea[b];
			if (a == b || bArrIN[b] == 2)
				continue;

			if (ChekcForignROI(sData, rcArrBlob[b], dAreaB, nAreaRootPixX, nAreaRootPixY, nDivide, bNotSizeChk) == FALSE)
			{
				if (bArrIN[b] != 1)
					bArrIN[b] = 2;
				continue;
			}

			CRect rtDst;
			if (rtDst.IntersectRect(rcArrBlob[a], rcArrBlob[b]) == FALSE)
				continue;

			if (bArrIN[b] == 1)
			{
				if (rcArrBlob[b].left > rcArrBlob[a].left) rcArrBlob[b].left = rcArrBlob[a].left;
				if (rcArrBlob[b].top > rcArrBlob[a].top) rcArrBlob[b].top = rcArrBlob[a].top;
				if (rcArrBlob[b].right < rcArrBlob[a].right) rcArrBlob[b].right = rcArrBlob[a].right;
				if (rcArrBlob[b].bottom < rcArrBlob[a].bottom) rcArrBlob[b].bottom = rcArrBlob[a].bottom;
				dArrArea[b] += dArrArea[a];
				bArrIN[a] = 2;
				break;
			}

			if (rcArrBlob[a].left > rcArrBlob[b].left) rcArrBlob[a].left = rcArrBlob[b].left;
			if (rcArrBlob[a].top > rcArrBlob[b].top) rcArrBlob[a].top = rcArrBlob[b].top;
			if (rcArrBlob[a].right < rcArrBlob[b].right) rcArrBlob[a].right = rcArrBlob[b].right;
			if (rcArrBlob[a].bottom < rcArrBlob[b].bottom) rcArrBlob[a].bottom = rcArrBlob[b].bottom;
			dArrArea[a] += dAreaB;
			bArrIN[b] = 2;

			for (int c = 0; c < b; c++)
			{
				double dAreaC = dArrArea[c];
				if (a == b || a == c || b == c || bArrIN[c] == 2)
					continue;

				if (ChekcForignROI(sData, rcArrBlob[c], dAreaC, nAreaRootPixX, nAreaRootPixY, nDivide, bNotSizeChk) == FALSE)
				{
					if (bArrIN[c] != 1)
						bArrIN[c] = 2;
					continue;
				}

				CRect rtDst2;
				if (rtDst2.IntersectRect(rcArrBlob[a], rcArrBlob[c]) == FALSE)
					continue;

				if (bArrIN[c] == 1)
				{
					if (rcArrBlob[c].left > rcArrBlob[a].left) rcArrBlob[c].left = rcArrBlob[a].left;
					if (rcArrBlob[c].top > rcArrBlob[a].top) rcArrBlob[c].top = rcArrBlob[a].top;
					if (rcArrBlob[c].right < rcArrBlob[a].right) rcArrBlob[c].right = rcArrBlob[a].right;
					if (rcArrBlob[c].bottom < rcArrBlob[a].bottom) rcArrBlob[c].bottom = rcArrBlob[a].bottom;
					dArrArea[c] += dArrArea[a];
					bArrIN[a] = 2;
					break;
				}

				if (rcArrBlob[a].left > rcArrBlob[c].left) rcArrBlob[a].left = rcArrBlob[c].left;
				if (rcArrBlob[a].top > rcArrBlob[c].top) rcArrBlob[a].top = rcArrBlob[c].top;
				if (rcArrBlob[a].right < rcArrBlob[c].right) rcArrBlob[a].right = rcArrBlob[c].right;
				if (rcArrBlob[a].bottom < rcArrBlob[c].bottom) rcArrBlob[a].bottom = rcArrBlob[c].bottom;
				dArrArea[a] += dAreaC;
				bArrIN[c] = 2;
			}
		}
		if (bArrIN[a] == 0)
			bArrIN[a] = 1;
	}
	sLog.Format(_T("[%d] : %d | %d/%d"), nMode, nCntBlob, nRealCnt, ForeignResult->m_nCountDefect);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspForeign]"), _T("[Merge Data_2]"), sLog, stT);
	for (int a = 0; a < nCntBlob; a++)
	{
		if (bArrIN[a] != 1) continue;
		if (rcArrBlob[a].left < 0) rcArrBlob[a].left = 0;
		if (rcArrBlob[a].top < 0) rcArrBlob[a].top = 0;
		if (rcArrBlob[a].right > nW) rcArrBlob[a].right = nW;
		if (rcArrBlob[a].bottom > nH) rcArrBlob[a].bottom = nH;
		if (rcArrBlob[a].left >= nW || rcArrBlob[a].top >= nH ||
			rcArrBlob[a].right <= 0 || rcArrBlob[a].bottom <= 0)
			continue;

		if (ChekcForignROI(sData, rcArrBlob[a], dArrArea[a], nAreaRootPixX, nAreaRootPixY, nDivide, bNotSizeChk) == FALSE)
			continue;

		int nRstCnt = 0;
		float fRstHeight = 0.0f;
		if (bUse3D)
		{
			int nHCnt = 0;
			float fHeightSum = 0.0f;
			for (int y = rcArrBlob[a].top; y < rcArrBlob[a].bottom; y++)
			{
				for (int x = rcArrBlob[a].left; x < rcArrBlob[a].right; x++)
				{
					long long nIndex = (y * nW) + x;
					if (imgFR[FR_IMG_BLOB].data[nIndex] != 255) continue;
					float fH = p3D[nIndex];
					fHeightSum += fH;
					nHCnt++;
				}
			}
			if (nHCnt == 0) continue;
			fRstHeight = fHeightSum / nHCnt;
			nRstCnt = nHCnt;
		}
		RECT rcRect = rcArrBlob[a];
		int nWTemp = rcArrBlob[a].Width();
		int nHTemp = rcArrBlob[a].Height();
		if (rcArrBlob[a].left >= rcArrBlob[a].right)
			rcArrBlob[a].right = rcArrBlob[a].left + 1;
		if (rcArrBlob[a].top >= rcArrBlob[a].bottom)
			rcArrBlob[a].bottom = rcArrBlob[a].top + 1;
		int nSize = 0;
		if ((nWTemp % 4) > 0)
			nSize = 4 - (nWTemp % 4);
		int nSZCnt = 0;
		for (int nSZ = 1; nSZ <= nSize; nSZ++)
		{
			if (rcRect.left - 1 >= 0)
			{
				rcRect.left -= 1;
				nSZCnt++;
			}
			if (nSZCnt == nSize)
				break;
			if (rcRect.right + 1 < nW)
			{
				rcRect.right += 1;
				nSZCnt++;
			}
			if (nSZCnt == nSize)
				break;
		}
		int nModuleID = 0;
		int nRows = imgFR[FR_IMG_BLOB].rows;
		POINTF ptCur[] = { rcRect.left, nRows - rcRect.top
							, rcRect.right, nRows - rcRect.top
							, rcRect.left, nRows - rcRect.bottom
							, rcRect.right, nRows - rcRect.bottom };
		for (int n = 0; n < 4; n++)
		{
			nModuleID = GetForeignModuleNumber(sForeign, sInspData, ptCur[n], nDivide, vInspectionMode, nContainModuleID);
			if (nModuleID > 0)
				break;
		}

		double dRssX = m_resolX * nDivide;
		double dRssY = m_resolY * nDivide;
		double dRstW = rcArrBlob[a].Width() * dRssX;
		double dRstL = rcArrBlob[a].Height() * dRssY;
		double dResVal = dRssX * dRssY;
		double dRstA = dArrArea[a] * dResVal;
		if (nRstCnt > 0)
			dRstA = nRstCnt * dResVal;
		if (bSIZE_AREA)
			dRstA = dRstW * dRstL;

		ForeignResult->m_stForeign[nRealCnt].SetData(m_eFR_RST_D_HA, false);

		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Mode, true, nMode);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_ModuleID, true, nModuleID);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Range3D, true, nRange3D);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Defect, true, nDefect);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_UVColorSelect, true);

		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_X, true, (rcRect.right + rcRect.left) / 2.0);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_Y, true, (rcRect.bottom + rcRect.top) / 2.0);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_W, true, dRstW);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_L, true, dRstL);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_A, true, dRstA);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_H, true, fRstHeight);

		ForeignResult->m_stForeign[nRealCnt].m_rcRect = rcRect;
		nRealCnt++;
		if (nRealCnt >= nMaxRst)
			break;
	}
	sLog.Format(_T("[%d] : %d | %d/%d"), nMode, nCntBlob, nRealCnt, ForeignResult->m_nCountDefect);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspForeign]"), _T("[Merge Data_3]"), sLog, stT);

	if (bSaveImg || (bSaveImg2 && nRealCnt > ForeignResult->m_nCountDefect))
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Find"), nMode);
		SaveDebugImg(imgFR[FR_IMG_FIND], str);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_INSP_AREA"), nMode);
		SaveDebugImg(imgInspAreaTemp, str);

		if (sData.UseData(FR_BIN_DT_InspSolderArea))
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_SolderA"), nMode);
			SaveDebugImg(imgFR[FR_IMG_SOLDER], str);
		}
		else if (sData.UseData(FR_BIN_DT_Insp_Skip_Part))
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_SkipPart"), nMode);
			SaveDebugImg(imgFR[FR_IMG_SKIP_PART], str);
		}
		else
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Area"), nMode);
			SaveDebugImg(imgInspArea, str);
		}

		if (bUse2D)
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Color"), nMode);
			SaveDebugImg(imgColor, str);
		}
		if (bUse3D)
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_HEXT"), nMode);
			SaveDebugImg(imgFR[FR_IMG_HEXT], str);
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_HOLE"), nMode);
			SaveDebugImg(imgFR[FR_IMG_HOLE], str);
		}
		if (bIM)
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_IM"), nMode);
			SaveDebugImg(imgFR[FR_IMG_IM], str);
		}
		if (bBTC)
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_BTC"), nMode);
			SaveDebugImg(imgFR[FR_IMG_BOTM], str);
		}

		if (sForeign.UseData(FR_DATA_Manual_EXT))
		{
			str = GetForeignDebugDataFullPath(nFovID, _T("_Manual_EXT"), nMode);
			cv::imwrite(std::string(CT2A(str)), imgFR[FR_IMG_EXT]);
		}

		str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Blob"), nMode);
		SaveDebugImg(imgFR[FR_IMG_BLOB], str);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Trace_ext"), nMode);
		SaveDebugImg(imgFR[FR_IMG_TRACE_EXT], str);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Trace"), nMode);
		SaveDebugImg(imgFR[FR_IMG_TRACE], str);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Trace_None"), nMode);
		SaveDebugImg(imgFR[FR_IMG_TRACE_NONE], str);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_Flash"), nMode);
		SaveDebugImg(imgFR[FR_IMG_FLASH], str);
	}
	ForeignResult->m_nCountDefect = nRealCnt;

	Delete_1DArray(&bArrIN);
	Delete_1DArray(&dArrArea);
	Delete_1DArray(&rcArrBlob);
	m_fFrTact[m_eFrTact_InspFR_END] += ((GetTickCount() - st) / 1000.0f);
	sLog.Format(_T("[%d] : %d | %d/%d"), nMode, nCntBlob, nRealCnt, ForeignResult->m_nCountDefect);
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspForeign]"), _T("[END]"), sLog, stT);
	return ForeignResult->m_nCountDefect == 0;
}
BOOL FOREIGN_INSP::ChekcForignROI(FR_Bin sData, CRect rcBlob, double dRstArea, int nAreaRootPixX, int nAreaRootPixY, int nDivide, bool bNotSizeChk)
{
	int nRstW = (rcBlob.right - rcBlob.left) + 1;
	int nRstH = (rcBlob.bottom - rcBlob.top) + 1;

	if (bNotSizeChk == false)
	{
		if (nRstW < nAreaRootPixX || nRstH < nAreaRootPixY)
			return FALSE;

		if (nRstW * nRstH * 0.1 >= dRstArea)
			return FALSE;
	}

	bool bSIZE_AREA = sData.UseData(FR_BIN_DT_SIZE_AREA);
	float fArrR[m_eFR_Bin_INSP_Total];
	fArrR[m_eFR_Bin_INSP_W] = (float)(nRstW * nDivide) * m_resolX;
	fArrR[m_eFR_Bin_INSP_L] = (float)(nRstH * nDivide) * m_resolY;
	fArrR[m_eFR_Bin_INSP_A] = (float)(dRstArea * nDivide * nDivide * m_resolX * m_resolY);
	if (bSIZE_AREA)
		fArrR[m_eFR_Bin_INSP_A] = fArrR[m_eFR_Bin_INSP_W] * fArrR[m_eFR_Bin_INSP_L];

	int nEss = 0;
	int nNGCnt = 0;
	int nEssNG = 0;
	for (int a = 0; a < m_eFR_Bin_INSP_Total; a++)
	{
		FR_Bin_INSP sInsp = sData.m_arrINSP[a];
		bool bMin = sInsp.UseData(m_eFR_Bin_INSP_D_Min);
		bool bMax = sInsp.UseData(m_eFR_Bin_INSP_D_Max);
		bool bEss = sInsp.UseData(m_eFR_Bin_INSP_D_Ess);
		float fMin = sInsp.m_fArr[m_eFR_Bin_INSP_F_Min];
		float fMax = sInsp.m_fArr[m_eFR_Bin_INSP_F_Max];

		if (bEss)
			nEss++;
		bool bNG = false;
		if (bMin && bMax)
		{
			if (fMin < fArrR[a] && fArrR[a] < fMax)
				bNG = true;
		}
		else if (bMin)
		{
			if (fMin < fArrR[a])
				bNG = true;
		}
		else if (bMax)
		{
			if (fArrR[a] < fMax)
				bNG = true;
		}

		if (bNG)
		{
			nNGCnt++;
			if (bEss)
				nEssNG++;
		}
	}
	if (nNGCnt == 0)
		return FALSE;
	else if (nEss > 0 && nEss != nEssNG)
		return FALSE;

	return TRUE;
}
int FOREIGN_INSP::GetForeignModuleNumber(InspForeignInfo sForeign, ForeignData sInspData, POINTF ptRst, int nDivide, MPTI_InspectionMode vInspectionMod, int nContainModuleNum)
{
	int nRet = 0;
	int nTotalModuleCnt = sForeign.m_nArrData[FR_N_ForeignParamROI_Total];
	ForeignParamROI *pROI = m_ForeignParamROI;
	InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;

	bool bFind = false;

	float fFovStX = 0;
	float fFovStY = 0;
	CRect rtFOV;
	if (vInspectionMod == eMI_Inspection)
	{
		fFovStX = pInspBoardInfo->fovCx - ((m_fovWidth / 2) * m_resolX);
		fFovStY = pInspBoardInfo->fovCy - ((m_fovLength / 2) * m_resolY);
	}
	else
	{
		POINTF poCur = g_pMPTI->GetForeignCurFovCenter();
		fFovStX = poCur.x - ((m_fovWidth / 2) * m_resolX);
		fFovStY = poCur.y - ((m_fovLength / 2) * m_resolY);
	}

	POINTF ptBoardRst;
	ptBoardRst.x = ((ptRst.x * m_resolX) * nDivide) + fFovStX;
	ptBoardRst.y = ((ptRst.y * m_resolY) * nDivide) + fFovStY;

	for (int i = 0; i < nTotalModuleCnt; i++)
	{
		int nMID = pROI[i].m_nModuleID;
		//사용하지 않거나 검사 에어리어 타입이 아닌 모듈 예외처리
		if (pROI[i].m_bSKIP || pROI[i].m_nType != ForeignParamROI_Data_Module)
			continue;

		//contain module 조건이있을때 예외처리
		if (nContainModuleNum > 0 && nContainModuleNum != nMID)
			continue;

		if (nMID > 0 && sInspData.m_nModule > nMID)
		{
			if (sInspData.m_pInspModule[nMID] == 0)
				continue;
		}

		if (pROI[i].m_bPolygon)	//Polygon Module
			bFind = PtInPolygon(ptBoardRst, pROI[i].m_pROIHit, ZMPOLYPOINT_ULTRA_CNTS);
		else  // Rectangle Module
		{
			double dLeft = pROI[i].m_pPos.x;
			double dRight = pROI[i].m_pPos.x + pROI[i].m_pSize.x;
			double dTop = pROI[i].m_pPos.y;
			double dBottom = pROI[i].m_pPos.y + (pROI[i].m_pSize.y);
			if (dLeft <= ptBoardRst.x && ptBoardRst.x <= dRight
				&& dTop <= ptBoardRst.y && ptBoardRst.y <= dBottom)
				bFind = true;
		}

		if (bFind)
		{
			nRet = nMID;
			break;
		}
	}

	return nRet;
}
int FOREIGN_INSP::GetForeignModuleNumber(InspForeignInfo sForeign, ForeignData sInspData, RECT rcROI, int nDivide, MPTI_InspectionMode vInspectionMod, int nContainModuleNum)
{
	int nRet = 0;
	int nTotalModuleCnt = sForeign.m_nArrData[FR_N_ForeignParamROI_Total];
	ForeignParamROI *pROI = m_ForeignParamROI;
	InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;

	bool bFind = false;

	float fFovStX = 0;
	float fFovStY = 0;
	CRect rtFOV;
	if (vInspectionMod == eMI_Inspection)
	{
		fFovStX = pInspBoardInfo->fovCx - ((m_fovWidth / 2) * m_resolX);
		fFovStY = pInspBoardInfo->fovCy - ((m_fovLength / 2) * m_resolY);
	}
	else
	{
		POINTF poCur = g_pMPTI->GetForeignCurFovCenter();
		fFovStX = poCur.x - ((m_fovWidth / 2) * m_resolX);
		fFovStY = poCur.y - ((m_fovLength / 2) * m_resolY);
	}

	float fL = ((rcROI.left * m_resolX) * nDivide) + fFovStX;
	float fR = ((rcROI.right * m_resolX) * nDivide) + fFovStX;
	float fT = ((rcROI.bottom * m_resolY) * nDivide) + fFovStY;
	float fB = (((rcROI.bottom + (rcROI.bottom - rcROI.top)) * m_resolY) * nDivide) + fFovStY;
	int nMax = 2;
	float fMax = 0;
	for (int i = 0; i < nTotalModuleCnt; i++)
	{
		int nMID = pROI[i].m_nModuleID;
		//사용하지 않거나 검사 에어리어 타입이 아닌 모듈 예외처리
		if (pROI[i].m_bSKIP || pROI[i].m_nType != ForeignParamROI_Data_Module)
			continue;

		//contain module 조건이있을때 예외처리
		if (nContainModuleNum > 0 && nContainModuleNum != nMID)
			continue;

		if (nMID > 0 && sInspData.m_nModule > nMID)
		{
			if (sInspData.m_pInspModule[nMID] == 0)
				continue;
		}

		float fFrL = pROI[i].m_pPos.x;
		float fFrR = pROI[i].m_pPos.x + pROI[i].m_pSize.x;
		float fFrT = pROI[i].m_pPos.y;
		float fFrB = pROI[i].m_pPos.y + (pROI[i].m_pSize.y);

		if (fFrR <= fL) continue;
		if (fFrL >= fR) continue;
		if (fFrB <= fT) continue;
		if (fFrT >= fB) continue;

		float fROIL = (fFrL < fL) ? fL : fFrL;
		float fROIR = (fFrR > fR) ? fR : fFrR;
		float fROIT = (fFrT < fT) ? fT : fFrT;
		float fROIB = (fFrB > fB) ? fB : fFrB;
		float fROIW = fROIR - fROIL;
		float fROIH = fROIB - fROIT;
		if (fROIW <= 0 || fROIH <= 0)
			continue;

		int nCnt = 0;
		if (fFrL <= fL && fL <= fFrR) nCnt++;
		if (fFrL <= fR && fR <= fFrR) nCnt++;
		if (fFrT <= fT && fT <= fFrB) nCnt++;
		if (fFrT <= fB && fB <= fFrB) nCnt++;
		if (fROIW * fROIH > fMax && nCnt >= nMax)
		{
			nMax = nCnt;
			fMax = fROIW * fROIH;
			nRet = nMID;
		}
	}

	return nRet;
}
void FOREIGN_INSP::GetFindBody(float * pf3DOrg, float * pf3D2Org, cv::Mat imgInspArea, cv::Mat *imgFR, int nW_Org, int nH_Org, ForeignData sData, int nDivideOrg, int nFovID, bool bSaveImg)
{
	if (sData.m_nTotalCnt <= 0 || sData.m_pXList == NULL || sData.m_pYList == NULL || sData.m_pWList == NULL ||
		sData.m_pHList == NULL || nW_Org <= 0 || nH_Org <= 0 || pf3DOrg == NULL || pf3D2Org == NULL || imgFR == NULL)
		return;

	InspForeignInfo sForeignInfo = m_ForeignData;
	bool bQ = false;
	if (sForeignInfo.GetDivide() == 2)
		bQ = sForeignInfo.UseData(FR_DATA_FindBody_Q);
	bool bNotSize = sForeignInfo.UseData(FR_DATA_FindBody_NotSize);

	int nWidth = bQ ? nW_Org / 2 : nW_Org;
	int nHeight = bQ ? nH_Org / 2 : nH_Org;
	int nDivide = bQ ? 4 : nDivideOrg;
	cv::Mat img3D;
	cv::Mat img3D2;
	cv::Mat imgArea(nHeight, nWidth, CV_8UC1, cv::Scalar(255));
	if (bQ)
	{
		cv::Mat img3DOrg(nH_Org, nW_Org, CV_32FC1, pf3DOrg);
		cv::Mat img3D2Org(nH_Org, nW_Org, CV_32FC1, pf3D2Org);
		cv::resize(img3DOrg, img3D, cv::Size(nWidth, nHeight));
		cv::resize(img3D2Org, img3D2, cv::Size(nWidth, nHeight));
	}
	else
	{
		img3D = cv::Mat(nHeight, nWidth, CV_32FC1, pf3DOrg);
		img3D2 = cv::Mat(nHeight, nWidth, CV_32FC1, pf3D2Org);
	}

	int nDiv = nDivide / 2;
	if (nDiv < 0) nDiv = 1;
	bool bExtPer = sForeignInfo.UseData2(FR_DATA2_ExtraPixelM_Per);
	float fExtPix = RounD(sForeignInfo.m_nArrData[FR_N_ExtraPixelM] / (float)nDiv);
	if (fExtPix > 100) fExtPix = 100;
	if (bExtPer)
		fExtPix = sForeignInfo.m_fArrData[FR_F_ExtraPixelM_Per];
	if (fExtPix < 0) fExtPix = 0;

	float fExtZ = 0;

	int nMinW = 5;
	int nMinH = 5;
	float f3DT = sForeignInfo.sBin[m_eForeignInsp_Foreign].m_fArr[FR_BIN_F_Min3D];
	double dFindPer = sForeignInfo.m_nArrData[FR_N_FR_Remove_ROI] / 100.0;
	if (dFindPer <= 0) dFindPer = 1;
	if (dFindPer > m_fFr_FINDBODY_FindX) dFindPer = m_fFr_FINDBODY_FindX;
	if (dFindPer > m_fFr_FINDBODY_FindY) dFindPer = m_fFr_FINDBODY_FindY;
	for (int a = 0; a < sData.m_nTotalCnt; a++)
	{
		CRect rcPart;
		rcPart.left = (sData.m_pXList[a] / nDivide);
		rcPart.top = (sData.m_pYList[a] / nDivide);
		rcPart.right = ((sData.m_pXList[a] + sData.m_pWList[a]) / nDivide);
		rcPart.bottom = ((sData.m_pYList[a] + sData.m_pHList[a]) / nDivide);

		int nClipL = rcPart.left - (rcPart.Width() * m_fFr_FINDBODY_FindX);
		int nClipT = rcPart.top - (rcPart.Height() * m_fFr_FINDBODY_FindY);
		int nClipW = rcPart.Width() + (rcPart.Width() * m_fFr_FINDBODY_FindX * 2);
		int nClipH = rcPart.Height() + (rcPart.Height() * m_fFr_FINDBODY_FindY * 2);
		if (nClipL < 0) nClipL = 0;
		if (nClipT < 0) nClipT = 0;
		if (nClipL + nClipW > nWidth) nClipW = nWidth - nClipL;
		if (nClipT + nClipH > nHeight) nClipH = nHeight - nClipT;

		cv::Rect rtClip(nClipL, nClipT, nClipW, nClipH);

		int nRmvL = rcPart.left - (rcPart.Width() * dFindPer);
		int nRmvT = rcPart.top - (rcPart.Height() * dFindPer);
		int nRmvW = rcPart.Width() + (rcPart.Width() * dFindPer * 2);
		int nRmvH = rcPart.Height() + (rcPart.Height() * dFindPer * 2);
		if (nRmvL < 0) nRmvL = 0;
		if (nRmvT < 0) nRmvT = 0;
		if (nRmvL + nRmvW > nWidth) nRmvW = nWidth - nRmvL;
		if (nRmvT + nRmvH > nHeight) nRmvH = nHeight - nRmvT;

		cv::Rect rtRmv(nRmvL, nRmvT, nRmvW, nRmvH);

		if (nClipW <= nMinW || nClipH <= nMinH)
		{
			Except3DArea(imgArea, nWidth, nHeight, rcPart, bExtPer, fExtZ, bQ, false);
			Except3DArea(imgFR[FR_IMG_FIND], nW_Org, nH_Org, rcPart, bExtPer, fExtPix, bQ, true);
			continue;
		}

		CRect rcFind(-1, -1, -1, -1);
		CRect rcFind2(-1, -1, -1, -1);
		bool bFind = GetFindROI_Derivative(img3D2, imgArea, imgFR, rtClip, rcPart, nDivide, &rcFind);
		bool bFindX = false;
		bool bFindY = false;
		bool bFindX_2 = false;
		bool bFindY_2 = false;
		for (int b = 0; b < 2; b++)
		{
			if (bFind)
			{
				FindROI(nWidth, nHeight, rcPart, &rcFind);
				if (bNotSize) NotSizeChange(rtRmv, &rcFind);
				Except3DArea(imgArea, nWidth, nHeight, rcFind, bExtPer, fExtZ, bQ, false);
				Except3DArea(imgFR[FR_IMG_FIND], nW_Org, nH_Org, rcFind, bExtPer, fExtPix, bQ, true);
				break;
			}

			cv::Mat img3DTemp = (b == 0) ? img3D : img3D2;
			cv::Mat imgBin(nClipH, nClipW, CV_8UC1, cv::Scalar(0));
			cv::Mat imgBlob(nClipH, nClipW, CV_8UC1, cv::Scalar(0));

			float fMin = GetBodyH(img3DTemp, f3DT, rcPart, b, nDivide);
			cv::Mat clip3D = img3DTemp(rtClip);
			cv::Mat clipArea = imgArea(rtClip);
			int ret2 = ippiCompareC_32f_C1R((float*)clip3D.data, img3DTemp.step, fMin, imgBin.data, imgBin.cols, { nClipW, nClipH }, ippCmpGreater);
			cv::bitwise_and(imgBin, clipArea, imgBin);
			std::vector<std::vector<cv::Point>> allRst = CalcBlob(imgBin, imgBlob, 10, false, true);
			int nCntBlob = allRst.size();
			if (nCntBlob <= 0)
				continue;

			CRect rcFindXY = GetFindCenter(imgBlob, rcPart, allRst, nClipL, nClipT, nDivide, &rcFind, &rcFind2);

			CRect rcFind_Temp(-1, -1, -1, -1);
			if ((rcFindXY.left & 0x01) == 0x01) rcFind_Temp.left = rcFind.left;
			if ((rcFindXY.right & 0x01) == 0x01) rcFind_Temp.right = rcFind.right;
			if ((rcFindXY.top & 0x01) == 0x01) rcFind_Temp.top = rcFind.top;
			if ((rcFindXY.bottom & 0x01) == 0x01) rcFind_Temp.bottom = rcFind.bottom;

			if (rcFind_Temp.left < 0 || (rcFindXY.left & 0x02) == 0x02) rcFind_Temp.left = rcFind2.left;
			if (rcFind_Temp.right < 0 || (rcFindXY.right & 0x02) == 0x02) rcFind_Temp.right = rcFind2.right;
			if (rcFind_Temp.top < 0 || (rcFindXY.top & 0x02) == 0x02) rcFind_Temp.top = rcFind2.top;
			if (rcFind_Temp.bottom < 0 || (rcFindXY.bottom & 0x02) == 0x02) rcFind_Temp.bottom = rcFind2.bottom;
			bFindX = rcFind_Temp.left >= 0 && rcFind_Temp.right >= 0;
			bFindY = rcFind_Temp.top >= 0 && rcFind_Temp.bottom >= 0;

			if (bFindX && bFindY)
			{
				FindROI(nWidth, nHeight, rcPart, &rcFind_Temp);
				if (bNotSize) NotSizeChange(rtRmv, &rcFind_Temp);
				Except3DArea(imgArea, nWidth, nHeight, rcFind_Temp, bExtPer, fExtZ, bQ, false);
				Except3DArea(imgFR[FR_IMG_FIND], nW_Org, nH_Org, rcFind_Temp, bExtPer, fExtPix, bQ, true);
				bFind = true;
				break;
			}
		}

		if (bFind == false)
		{
			if (rcFind.left < 0 && rcFind2.left >= 0)
				rcFind.left = rcFind2.left;
			if (rcFind.right < 0 && rcFind2.right >= 0)
				rcFind.right = rcFind2.right;
			if (rcFind.top < 0 && rcFind2.top >= 0)
				rcFind.top = rcFind2.top;
			if (rcFind.bottom < 0 && rcFind2.bottom >= 0)
				rcFind.bottom = rcFind2.bottom;
			if (rcFind.left < 0) rcFind.left = rcPart.left;
			if (rcFind.right <= 0) rcFind.right = rcPart.right;
			if (rcFind.top < 0) rcFind.top = rcPart.top;
			if (rcFind.bottom <= 0) rcFind.bottom = rcPart.bottom;
			if (rcFind.left >= rcFind.right || rcFind.Width() <= nMinW)
			{
				rcFind.left = rcPart.left;
				rcFind.right = rcPart.right;
			}
			if (rcFind.top >= rcFind.bottom || rcFind.Height() <= nMinH)
			{
				rcFind.top = rcPart.top;
				rcFind.bottom = rcPart.bottom;
			}

			FindROI(nWidth, nHeight, rcPart, &rcFind);
			if (bNotSize) NotSizeChange(rtRmv, &rcFind);
			Except3DArea(imgArea, nWidth, nHeight, rcFind, bExtPer, fExtZ, bQ, false);
			Except3DArea(imgFR[FR_IMG_FIND], nW_Org, nH_Org, rcFind, bExtPer, fExtPix, bQ, true);
		}
	}

	if (bSaveImg)
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_GetFindBody_Blob"));
		SaveDebugImg(imgFR[FR_IMG_FIND], str);
	}
}
void FOREIGN_INSP::FindROI(int nW, int nH, CRect rcPart, CRect* rcFind)
{
	for (int a = 0; a < 2; a++)
	{
		int nSize = (a == 0) ? nW : nH;
		int nFind = (a == 0) ? rcFind->Width() : rcFind->Height();
		int nFindS = (a == 0) ? rcFind->left : rcFind->top;
		int nFindE = (a == 0) ? rcFind->right : rcFind->bottom;
		int nPart = (a == 0) ? rcPart.Width() : rcPart.Height();
		int nPartS = (a == 0) ? rcPart.left : rcPart.top;
		int nPartE = (a == 0) ? rcPart.right : rcPart.bottom;

		double dGap = nPart - nFind;
		if (nFind >= nPart || dGap <= 0)
			continue;

		if (nFindS <= 0)
		{
			if (a == 0)
				rcFind->right += dGap;
			else
				rcFind->bottom += dGap;
			continue;
		}
		if (nFindE >= nSize - 1)
		{
			if (a == 0)
				rcFind->left -= dGap;
			else
				rcFind->top -= dGap;
			continue;
		}

		double dGapS = nPartS - nFindS;
		double dGapE = nPartE - nFindE;
		if (dGapS < 0) dGapS *= -1.0;
		if (dGapE < 0) dGapE *= -1.0;

		double dTo = dGapS + dGapE;
		double dPerS = dGapS / dTo;
		double dPerE = 1.0 - dPerS;
		double dMarS = (dGap * dPerS) + 1.55;
		double dMarE = (dGap * dPerE) + 1.55;
		if (a == 0)
		{
			rcFind->left -= dMarS;
			rcFind->right += dMarE;
		}
		else
		{
			rcFind->top -= dMarS;
			rcFind->bottom += dMarE;
		}
	}
}
void FOREIGN_INSP::NotSizeChange(cv::Rect rtClip, CRect* rcFind)
{
	if (rtClip.width != rcFind->Width())
	{
		int nCenter = (rcFind->left + rcFind->right) / 2;
		rcFind->left = nCenter - (rtClip.width / 2);
		rcFind->right = rcFind->left + rtClip.width;
	}
	if (rtClip.height != rcFind->Height())
	{
		int nCenter = (rcFind->top + rcFind->bottom) / 2;
		rcFind->top = nCenter - (rtClip.height / 2);
		rcFind->bottom = rcFind->top + rtClip.height;
	}
}
float FOREIGN_INSP::GetBodyH(cv::Mat img3D, float f3DT, CRect rcROI, int nType, int nDivide)
{
	float fSum = 0.0f;
	int nCnt = 0;
	float f3D = f3DT;
	int nW = img3D.cols;
	int nH = img3D.rows;

	int nL = rcROI.left;
	int nR = rcROI.right;
	int nT = rcROI.top;
	int nB = rcROI.bottom;
	int nROIW = rcROI.right - rcROI.left;
	int nROIH = rcROI.bottom - rcROI.top;

	int nMinGap = nDivide == 2 ? 8 : 4;
	double dGap = (nType == 0) ? 0.8 : 1.2;
	for (int a = 0; a < 2; a++)
	{
		int nMin = (a == 0) ? rcROI.left : rcROI.top;
		int nMax = (a == 0) ? rcROI.right : rcROI.bottom;
		int nSize = (a == 0) ? nROIW : nROIH;
		int nTemp = (int)(nSize * dGap);
		int nDiv = (nType == 0) ? nSize - nTemp : nTemp - nSize;
		if (nDiv <= nMinGap)
			continue;

		int nGap = nDiv / 2;
		int nTempMin = nMin + nGap;
		int nTempMax = nMax - nGap;
		if (nTempMax <= nTempMin || nTempMax - nTempMin <= nMinGap)
			continue;

		if (a == 0)
		{
			nL = nTempMin;
			nR = nTempMax;
		}
		else
		{
			nT = nTempMin;
			nB = nTempMax;
		}
	}

	for (int y = nT; y < nB; y++)
	{
		if (y < 0 || y >= nH)
			continue;

		for (int x = nL; x < nR; x++)
		{
			if (x < 0 || x >= nW)
				continue;

			float f3DVal = img3D.ptr<float>(y)[x];
			if (f3DVal >= f3D)
			{
				fSum += f3DVal;
				nCnt++;
			}
		}
	}

	if (nCnt > 0)
	{
		float fAVG = fSum / (float)nCnt;
		float fRst = fAVG * 0.7f;
		if (fRst > f3D)
			return fRst;
	}

	return f3D;
}
void FOREIGN_INSP::Except3DArea(cv::Mat img, int nW, int nH, CRect rcROI, bool bExtPer, float fExtPix, bool bQ, bool bOrg)
{
	int nDiv = 1;
	if (bOrg) nDiv = bQ ? 2 : 1;
	int nBlobL = rcROI.left;
	int nBlobR = rcROI.right;
	int nBlobT = rcROI.top;
	int nBlobB = rcROI.bottom;
	if (bExtPer && fExtPix > 0)
	{
		float fFrRoiW = rcROI.right - rcROI.left;
		float fFrRoiH = rcROI.bottom - rcROI.top;
		if (fFrRoiW > 0)
		{
			float fMar = fFrRoiW * fExtPix / 100.0f;
			float fMarGap = fMar - fFrRoiW;
			float fMarGapH = fMarGap / 2.0f;
			nBlobL = rcROI.left - (int)fMarGapH;
			nBlobR = rcROI.right + (int)fMarGapH;
		}

		if (fFrRoiH > 0)
		{
			float fMar = fFrRoiH * fExtPix / 100.0f;
			float fMarGap = fMar - fFrRoiH;
			float fMarGapH = fMarGap / 2.0f;
			nBlobT = rcROI.top - (int)fMarGapH;
			nBlobB = rcROI.bottom + (int)fMarGapH;
		}
	}
	else if (bExtPer == false)
	{
		nBlobL = (rcROI.left - fExtPix);
		nBlobR = (rcROI.right + fExtPix);
		nBlobT = (rcROI.top - fExtPix);
		nBlobB = (rcROI.bottom + fExtPix);
	}
	nBlobL *= nDiv;
	nBlobR *= nDiv;
	nBlobT *= nDiv;
	nBlobB *= nDiv;

	if (nBlobL < 0) nBlobL = 0;
	if (nBlobR < 0) nBlobR = 0;
	if (nBlobT < 0) nBlobT = 0;
	if (nBlobB < 0) nBlobB = 0;
	if (nBlobL >= nW) nBlobL = nW - 1;
	if (nBlobR >= nW) nBlobR = nW - 1;
	if (nBlobT >= nH) nBlobT = nH - 1;
	if (nBlobB >= nH) nBlobB = nH - 1;
	if (nBlobL >= nBlobR || nBlobT >= nBlobB)
		return;

	int nBlobW = nBlobR - nBlobL + 1;
	int nBlobH = nBlobB - nBlobT + 1;
	if (nBlobL + nBlobW > nW) nBlobW = nW - nBlobL;
	if (nBlobT + nBlobH > nH) nBlobH = nH - nBlobT;
	if (nBlobW <= 0 || nBlobH <= 0)
		return;

	img(cv::Rect(nBlobL, nBlobT, nBlobW, nBlobH)).setTo(0);
}
bool FOREIGN_INSP::UseData(int nData, int nType)
{
	bool bRet = ((nData & nType) == nType);
	return bRet;
}
bool FOREIGN_INSP::GetFindROI_Derivative(cv::Mat img3D2, cv::Mat imgArea, cv::Mat *imgFR, cv::Rect rtClip, CRect rcPart, int nDivide, CRect *rcFind_Temp)
{
	bool bFind = false;

	int nClipL = rtClip.tl().x;
	int nClipT = rtClip.tl().y;
	int nW = rtClip.width;
	int nH = rtClip.height;
	int nBin = 0;
	cv::Mat clip3D = img3D2(rtClip);
	cv::Mat clipArea = imgArea(rtClip);
	cv::Mat imgBin(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgGap(nH, nW, CV_32FC1, cv::Scalar(0));
	for (int y = 0; y < nH; y++)
	{
		float fSum = 0.0f;
		int nSum = 0;
		for (int x = 0; x < nW - 1; x++)
		{
			float f3DVal_1 = clip3D.ptr<float>(y)[x];
			float f3DVal_2 = clip3D.ptr<float>(y)[x + 1];
			float f3DVal = f3DVal_2 - f3DVal_1;
			if (f3DVal < 0) f3DVal *= -1.0f;
			imgGap.ptr<float>(y)[x + 1] = f3DVal;
			if (f3DVal > 10.0f)
			{
				fSum += f3DVal;
				nSum++;
			}

			long long nIndex = (y * nW) + x;
			if (f3DVal_1 >= 50.0f && imgBin.data[nIndex] != 255)
			{
				imgBin.data[nIndex] = 255;
				nBin++;
			}

			nIndex = (y * nW) + (x + 1);
			if (f3DVal_2 >= 50.0f && imgBin.data[nIndex] != 255)
			{
				imgBin.data[nIndex] = 255;
				nBin++;
			}
		}

		if (nSum <= 0)
			continue;

		float fAVG = fSum / nSum;
		for (int x = 0; x < nW; x++)
		{
			long long nIndex = (y * nW) + x;
			if (imgBin.data[nIndex] == 255)
				continue;

			float f3DVal = imgGap.ptr<float>(y)[x];
			if (f3DVal >= fAVG)
			{
				imgBin.data[nIndex] = 255;
				nBin++;
			}
		}
	}

	int nMinBlob = 10;
	if (nBin < ((nW * nH) / (nMinBlob * 3)))
		return bFind;

	cv::Mat imgBlob(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::bitwise_and(imgBin, clipArea, imgBin);
	std::vector<std::vector<cv::Point>> allRst = CalcBlob(imgBin, imgBlob, nMinBlob, false, true);
	if (allRst.size() <= 0)
		return bFind;

	CRect rcFind(-1, -1, -1, -1);
	CRect rcFind2(-1, -1, -1, -1);
	CRect rcFindXY = GetFindCenter(imgBlob, rcPart, allRst, nClipL, nClipT, nDivide, &rcFind, &rcFind2);

	if ((rcFindXY.left & 0x01) == 0x01) rcFind_Temp->left = rcFind.left;
	if ((rcFindXY.right & 0x01) == 0x01) rcFind_Temp->right = rcFind.right;
	if ((rcFindXY.top & 0x01) == 0x01) rcFind_Temp->top = rcFind.top;
	if ((rcFindXY.bottom & 0x01) == 0x01) rcFind_Temp->bottom = rcFind.bottom;

	if (rcFind_Temp->left >= 0 && rcFind_Temp->right >= 0 &&
		rcFind_Temp->top >= 0 && rcFind_Temp->bottom >= 0)
		bFind = true;

	return bFind;
}
CRect FOREIGN_INSP::GetFindCenter(cv::Mat imgBlob, CRect rcPart, std::vector<std::vector<cv::Point>> allRst, int nClipL, int nClipT, int nDivide, CRect* rcFind, CRect* rcFind2)
{
	CRect rcFindXY(-1, -1, -1, -1);
	int nFindX = (int)RounD(m_fFr_FINDBODY_FindX / m_resolX / nDivide);
	int nFindY = (int)RounD(m_fFr_FINDBODY_FindY / m_resolY / nDivide);
	int nShiftX = (int)RounD(m_fFr_FINDBODY_ShiftX / m_resolX / nDivide);
	int nShiftY = (int)RounD(m_fFr_FINDBODY_ShiftY / m_resolY / nDivide);
	int nPartW = rcPart.Width();
	int nPartH = rcPart.Height();

	float fShiftMinX = 1.0f - m_fFr_FINDBODY_ShiftX;
	float fShiftMaxX = 1.0f + m_fFr_FINDBODY_ShiftX;
	float fShiftMinY = 1.0f - m_fFr_FINDBODY_ShiftY;
	float fShiftMaxY = 1.0f + m_fFr_FINDBODY_ShiftY;
	float fFindMinX = 1.0f - m_fFr_FINDBODY_FindX;
	float fFindMaxX = 1.0f + m_fFr_FINDBODY_FindX;
	float fFindMinY = 1.0f - m_fFr_FINDBODY_FindY;
	float fFindMaxY = 1.0f + m_fFr_FINDBODY_FindY;

	int nTemp[4] = { nShiftX * 2, nShiftX * 2, nShiftY * 2, nShiftY * 2 };
	int nTemp2[4] = { nFindX * 2, nFindX * 2, nFindY * 2, nFindY * 2 };

	for (int c = 0; c < allRst.size(); c++)
	{
		cv::Rect roi = cv::boundingRect(allRst[c]);
		double dCX = roi.x + (roi.width / 2.0);
		double dCY = roi.y + (roi.height / 2.0);
		int nBlobW = roi.width;
		int nBlobH = roi.height;
		if (nPartW * 0.2 >= nBlobW || nPartH * 0.2 >= nBlobH)
			continue;

		CRect rcBlob;
		rcBlob.left = roi.x + nClipL;
		rcBlob.right = roi.br().x + nClipL;
		rcBlob.top = roi.y + nClipT;
		rcBlob.bottom = roi.br().y + nClipT;

		CRect rcDst;
		if (rcDst.IntersectRect(rcPart, rcBlob) == FALSE)
			continue;

		CPoint poPart = rcPart.CenterPoint();
		CPoint poBlob = rcBlob.CenterPoint();
		double dBlobX = dCX + nClipL;
		double dBlobY = dCY + nClipT;

		int nGapX = (poPart.x > poBlob.x) ? poPart.x - poBlob.x : poBlob.x - poPart.x;
		int nGapY = (poPart.y > poBlob.y) ? poPart.y - poBlob.y : poBlob.y - poPart.y;
		int nGapX2 = (poPart.x > dBlobX) ? poPart.x - dBlobX : dBlobX - poPart.x;
		int nGapY2 = (poPart.y > dBlobY) ? poPart.y - dBlobY : dBlobY - poPart.y;

		int nGapL = rcPart.left - rcBlob.left;
		int nGapR = rcPart.right - rcBlob.right;
		int nGapT = rcPart.top - rcBlob.top;
		int nGapB = rcPart.bottom - rcBlob.bottom;
		if (nGapL < 0) nGapL *= -1;
		if (nGapR < 0) nGapR *= -1;
		if (nGapT < 0) nGapT *= -1;
		if (nGapB < 0) nGapB *= -1;
		if (nShiftX >= nGapX || nShiftX >= nGapX2)
		{
			if (rcBlob.Width() >= rcPart.Width() * fShiftMinX && rcBlob.Width() <= rcPart.Width() * fShiftMaxX)
			{
				if (nShiftX >= nGapL && nTemp[0] > nGapL)
				{
					rcFind->left = rcBlob.left;
					nTemp[0] = nGapL;
					rcFindXY.left |= 1;
				}
				if (nShiftX >= nGapR && nTemp[1] > nGapR)
				{
					rcFind->right = rcBlob.right;
					nTemp[1] = nGapR;
					rcFindXY.right |= 1;
				}
			}
		}
		if (nFindX >= nGapX || nFindX >= nGapX2)
		{
			if (rcBlob.Width() >= rcPart.Width() * fFindMinX && rcBlob.Width() <= rcPart.Width() * fFindMaxX)
			{
				if (nFindX >= nGapL && nTemp2[0] > nGapL)
				{
					rcFind2->left = rcBlob.left;
					nTemp2[0] = nGapL;
					rcFindXY.left |= 2;
				}
				if (nFindX >= nGapR && nTemp2[1] > nGapR)
				{
					rcFind2->right = rcBlob.right;
					nTemp2[1] = nGapR;
					rcFindXY.right |= 2;
				}
			}
		}

		if (nShiftY >= nGapY || nShiftY >= nGapY2)
		{
			if (rcBlob.Height() >= rcPart.Height() * fShiftMinY && rcBlob.Height() <= rcPart.Height() * fShiftMaxX)
			{
				if (nShiftY >= nGapT && nTemp[2] > nGapT)
				{
					rcFind->top = rcBlob.top;
					nTemp[2] = nGapT;
					rcFindXY.top |= 1;
				}
				if (nShiftY >= nGapB && nTemp[3] > nGapB)
				{
					rcFind->bottom = rcBlob.bottom;
					nTemp[2] = nGapB;
					rcFindXY.bottom |= 1;
				}
			}
		}
		if (nFindY >= nGapY || nFindY >= nGapY2)
		{
			if (rcBlob.Height() >= rcPart.Height() * fFindMinY && rcBlob.Height() <= rcPart.Height() * fFindMaxY)
			{
				if (nFindY >= nGapT && nTemp2[2] > nGapT)
				{
					rcFind2->top = rcBlob.top;
					nTemp2[2] = nGapT;
					rcFindXY.top |= 2;
				}
				if (nFindY >= nGapB && nTemp2[3] > nGapB)
				{
					rcFind2->bottom = rcBlob.bottom;
					nTemp2[2] = nGapB;
					rcFindXY.bottom |= 2;
				}
			}
		}
	}
	return rcFindXY;
}
void FOREIGN_INSP::GetExceptFR(InspForeignInfo sForeign, ForeignData sInspData, cv::Mat imgFREXT, int nW, int nH, int nMode)
{
	ForeignParamROI *pROI = m_ForeignParamROI;
	if (pROI == NULL) return;

	int nROITotal = sForeign.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= 0) return;

	int nFR = 0;
	if (nMode >= m_eForeignInsp_Foreign && nMode <= m_eForeignInsp_Copper)
		nFR = pow(2, nMode);
	if (nFR <= 0) return;

	CRect rtFOV;
	int nDivide = sForeign.GetDivide();
	double dFactorX = 1.0;
	double dFactorY = 1.0;
	if (sInspData.m_bDEBUG)
	{
		dFactorX = (double)sInspData.m_rcFOV.right / (double)sInspData.m_nWP_W;
		dFactorY = (double)sInspData.m_rcFOV.bottom / (double)sInspData.m_nWP_H;
	}
	if (sInspData.m_vInspectionMode == eMI_Inspection)
	{
		InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;
		if (pInspBoardInfo == NULL)
			return;

		int nCX = RounD(pInspBoardInfo->fovCx / m_resolX / nDivide);
		int nCY = RounD(pInspBoardInfo->fovCy / m_resolY / nDivide);
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
		if (pROI[a].m_nInspFR <= 0)
			continue;
		if ((pROI[a].m_nInspFR & nFR) != nFR)
			continue;

		int nType = pROI[a].m_nType;
		SetExceptForeignImage(a, nType, rtFOV, imgFREXT.data, nW, nH, 0);
	}
}
void FOREIGN_INSP::FR_INSP_TRACE(InspForeignInfo sForeign, ForeignData sInspData, cv::Mat *imgFR, int nW, int nH)
{
	ForeignParamROI *pROI = m_ForeignParamROI;
	if (pROI == NULL) return;

	int nROITotal = sForeign.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= 0) return;

	CRect rtFOV;
	int nDivide = sForeign.GetDivide();
	double dFactorX = 1.0;
	double dFactorY = 1.0;
	if (sInspData.m_bDEBUG)
	{
		dFactorX = (double)sInspData.m_rcFOV.right / (double)sInspData.m_nWP_W;
		dFactorY = (double)sInspData.m_rcFOV.bottom / (double)sInspData.m_nWP_H;
	}
	if (sInspData.m_vInspectionMode == eMI_Inspection)
	{
		InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;
		if (pInspBoardInfo == NULL)
			return;

		int nCX = RounD(pInspBoardInfo->fovCx / m_resolX / nDivide);
		int nCY = RounD(pInspBoardInfo->fovCy / m_resolY / nDivide);
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

	//cv::Mat imgTEMP(nH, nW, CV_8UC1, cv::Scalar(0));
	bool bGND = sInspData.UseData(m_eForeignData_DATA_GND);
	for (int a = 0; a < nROITotal; a++)
	{
		int nType = pROI[a].m_nType;
		if (nType != ForeignParamROI_Data_PAD)
			continue;

		if (pROI[a].m_nInspFR == pad_format_Flash)
			SetExceptForeignImage(a, nType, rtFOV, imgFR[FR_IMG_FLASH].data, nW, nH, 255);
		else if (pROI[a].m_nInspFR == pad_format_Trace)
		{
			if (bGND)
				continue;
			else if (pROI[a].m_nModuleID == m_nFR_TraceID + 1)
				SetExceptForeignImage(a, nType, rtFOV, imgFR[FR_IMG_TRACE_EXT].data, nW, nH, 255);
			//else if (pROI[a].m_nModuleID == m_nFR_TraceID)
			//	SetExceptForeignImage(a, nType, rtFOV, imgTEMP.data, nW, nH, 255);
		}
	}
}
void FOREIGN_INSP::GetRGB(float fHei, UCHAR * ucArrDstImg, long long nIndex, float fPseudoColorMin, float fPseudoColorMax)
{
	SetRGB(ucArrDstImg, nIndex, fHei, fPseudoColorMin, fPseudoColorMax);
}
void FOREIGN_INSP::GetExceptFR(InspForeignInfo sForeign, ForeignData sInspData, InspFovForeignResult* ForeignResult, cv::Mat imgFREXT, int nW, int nH)
{
	if (ForeignResult == NULL || ForeignResult->m_stForeign == NULL)
		return;

	int nROITotal = ForeignResult->m_nCountDefect;
	if (nROITotal <= 0)
		return;

	int nFOVW = sInspData.m_rcFOV.right - sInspData.m_rcFOV.left;
	int nFOVH = sInspData.m_rcFOV.bottom - sInspData.m_rcFOV.top;
	bool bIM = sForeign.sWP.UseData(FR_WP_DT_IM);
	bool bAM = sForeign.sWP.UseData(FR_WP_DT_AM);
	bool bCorrectWP = sForeign.sWP.UseData(FR_WP_DT_CorrectWP);
	if (bIM || bAM) bCorrectWP = false;
#if _DEBUG
	cv::Mat imgFUL(sInspData.m_nWP_H, sInspData.m_nWP_W, CV_8UC1, cv::Scalar(0));
	cv::Mat imgFOV(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
#endif
	for (int a = 0; a < nROITotal; a++)
	{
		RECT roi = ForeignResult->m_stForeign[a].m_rcRect;
		int nL = roi.left;
		int nR = roi.right;
		int nT = roi.top;
		int nB = roi.bottom;
		if (nL < 0) nL = 0;
		if (nR < 0) nR = 0;
		if (nT < 0) nT = 0;
		if (nB < 0) nB = 0;
		if (nL >= nW) nL = nW - 1;
		if (nR > nW) nR = nW;
		if (nT >= nH) nT = nH - 1;
		if (nB > nH) nB = nH;

		int nImgW = nR - nL + 1;
		int nImgH = nB - nT + 1;
		if (nL + nImgW > nW) nImgW = nW - nL;
		if (nT + nImgH > nH) nImgH = nH - nT;
		if (nImgW <= 0 || nImgH <= 0)
			continue;

		imgFREXT(cv::Rect(nL, nT, nImgW, nImgH)).setTo(0);

		if (bCorrectWP == false)
			continue;
		if (nFOVW != nW || nFOVH != nH)
			continue;
		if (sInspData.m_pDesImg == NULL)
			continue;

		for (int y = nT; y < nB; y++)
		{
			if (y < 0) continue;
			if (y >= nFOVH) break;

			for (int x = nL; x < nR; x++)
			{
				if (x < 0) continue;
				if (x >= nFOVW) break;

				int nX = x + sInspData.m_rcFOV.left;
				int nY = y + sInspData.m_rcFOV.top;
				if (nX < 0 || nY < 0)
					continue;
				if (nX >= sInspData.m_nWP_W || nY >= sInspData.m_nWP_H)
					break;

				long long nDst = ((nY * sInspData.m_nWP_W) + nX);
				if ((sInspData.m_pimgSUB[nDst] & FR_SUB_EXCP) == FR_SUB_EXCP)
					sInspData.m_pimgSUB[nDst] -= FR_SUB_EXCP;
#if _DEBUG
				imgFUL.data[nDst] = 255;
				int nIDX = (y * nFOVW) + x;
				imgFOV.data[nIDX] = 255;
#endif
			}
		}
	}
}
void FOREIGN_INSP::CheckWPMargin(InspForeignInfo sForeign, ForeignData sInspData, int nW, int nH, int nModuleID, cv::Rect &rcROI)
{
	FR_WP sWP = sForeign.sWP;
	float fBoardW = sForeign.m_fArrData[FR_F_BoardW];
	float fBoardH = sForeign.m_fArrData[FR_F_BoardH];
	if (fBoardW <= 0 || fBoardH <= 0)
		return;

	float fMarginT = sWP.m_fArr[FR_WP_F_Margin_T];
	float fMarginB = sWP.m_fArr[FR_WP_F_Margin_B];
	if (fMarginT <= 0) fMarginT = 0;
	if (fMarginB <= 0) fMarginB = 0;
	if (fMarginT <= 0 && fMarginB <= 0)
		return;

	int nDivide = sForeign.GetDivide();
	int nMarginT = fMarginT / m_resolX / nDivide;
	int nMarginB = fMarginB / m_resolY / nDivide;
	if (nMarginT <= 0) nMarginT = 0;
	if (nMarginB <= 0) nMarginB = 0;
	if (nMarginT <= 0 && nMarginB <= 0)
		return;

	ForeignParamROI *pROI = m_ForeignParamROI;
	if (pROI == NULL) return;

	int nROITotal = sForeign.m_nArrData[FR_N_ForeignParamROI_Total];
	if (nROITotal <= 0) return;

	CRect rtFOV;
	if (sInspData.m_vInspectionMode == eMI_Inspection)
	{
		InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;
		if (pInspBoardInfo == NULL)
			return;

		int nCX = RounD(pInspBoardInfo->fovCx / m_resolX / nDivide);
		int nCY = RounD((fBoardH - pInspBoardInfo->fovCy) / m_resolY / nDivide);
		rtFOV.left = nCX - (nW / 2);
		rtFOV.right = rtFOV.left + nW;
		rtFOV.top = nCY - (nH / 2);
		rtFOV.bottom = rtFOV.top + nH;
	}
	else
	{
		POINTF poCur = g_pMPTI->GetForeignCurFovCenter();
		int nCX = RounD(poCur.x / m_resolX / nDivide);
		int nCY = RounD((fBoardH - poCur.y) / m_resolY / nDivide);
		rtFOV.left = nCX - (nW / 2);
		rtFOV.right = rtFOV.left + nW;
		rtFOV.top = nCY - (nH / 2);
		rtFOV.bottom = rtFOV.top + nH;
	}

	float fTOP = HUGE_VAL;
	float fBTM = HUGE_VAL;
	for (int a = 0; a < nROITotal; a++)
	{
		if (pROI[a].m_nType != ForeignParamROI_Data_Module)
			continue;
		if (pROI[a].m_nModuleID != nModuleID)
			continue;

		float fB = fBoardH - pROI[a].m_pPos.y;
		float fT = fB - pROI[a].m_pSize.y;
		if (fTOP == HUGE_VAL || fTOP > fT) fTOP = fT;
		if (fBTM == HUGE_VAL || fBTM < fB) fBTM = fB;
	}

	if (fTOP == HUGE_VAL || fBTM == HUGE_VAL)
		return;
	if (fTOP >= fBTM || fBTM <= fMarginT)
		return;

	int nTOP = RounD(fTOP / m_resolX / nDivide);
	int nBTM = RounD(fBTM / m_resolY / nDivide);
	if (nBTM <= nTOP || nBTM <= nMarginB)
		return;

	int nMarginTop = nTOP + nMarginT;
	int nMarginBtm = nBTM - nMarginB;
	if (nMarginBtm <= nMarginTop || nMarginBtm <= 0)
		return;

	int nROIT = rcROI.y + rtFOV.top;
	int nROIB = rcROI.br().y + rtFOV.top;
	if (nMarginTop > nROIT)
		nROIT = nMarginTop;
	if (nMarginBtm < nROIT)
		nROIT += nMarginBtm;

	if (nMarginTop > nROIB)
		nROIB = nMarginTop;
	if (nMarginBtm < nROIB)
		nROIB = nMarginBtm;
	if (nROIB <= nROIT || nROIB <= 0)
		return;

	int nRstT = nROIT - rtFOV.top;
	int nRstB = nROIB - rtFOV.top;
	if (nRstB <= nRstT || nRstB <= 0)
		return;

	if (nRstT >= nH - 1) { nRstT = nH - 2; nRstB = nH; }
	if (nRstT >= nRstB) nRstB = nH;
	if (nRstB > nH) nRstB = nH;

	int nrcROIX = rcROI.x;
	int nrcROIY = nRstT;
	int nrcROIW = rcROI.width;
	int nrcROIH = nRstB - nRstT;
	rcROI = cv::Rect(nrcROIX, nrcROIY, nrcROIW, nrcROIH);
}
bool FOREIGN_INSP::InspForeign_HA(InspForeignInfo sForeign, float* p3D, cv::Mat imgInspArea, cv::Mat *imgFR, int nW, int nH, int nMode, int nFovID, ForeignData sInspData, InspFovForeignResult* ForeignResult, FR2DData sFR2DData)
{
	DWORD st = GetTickCount();
	if (p3D == NULL || p3D == nullptr)
		return true;

	int nMaxRst = 2000;
	int nRealCnt = ForeignResult->m_nCountDefect;
	if (nRealCnt >= nMaxRst)
		return false;

	MPTI_InspectionMode vInspectionMode = sInspData.m_vInspectionMode;
	int nContainModuleID = sInspData.m_nContainModuleID;

	InspPartInfo* pInspBoardInfo = m_pInspBoardInfo_Foreign;
	int nDivide = sForeign.GetDivide();
	bool bSaveImg = sForeign.UseData(FR_DATA_DebugData);
	bool bSaveImg2 = sForeign.UseData(FR_DATA_DebugData2);
	bool bWP = sForeign.IsForeignEnable(m_eForeignInsp_Warpage);
	bool bIM = sForeign.sWP.UseData(FR_WP_DT_IM) && bWP;
	bool bAM = sForeign.sWP.UseData(FR_WP_DT_AM) && bWP;

	FR_Bin sData = sForeign.sBin[nMode];
	bool bHA_Under = sData.UseData(FR_BIN_DT_HA_Under);
	bool bHA_Upper = sData.UseData(FR_BIN_DT_HA_Upper);
	bool bDEFECT = sData.UseData(FR_BIN_DT_HA_DEFECT);
	if (bHA_Under == false && bHA_Upper == false)
		return true;

	cv::Mat img3D(nH, nW, CV_32FC1, p3D);
	cv::Mat imgInspAreaTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	if (bSaveImg || bSaveImg2)
	{
		imgFR[FR_IMG_BIN].copyTo(imgInspAreaTemp);
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_HA_INSP_AREA"), nMode);
		if (bSaveImg)
			SaveDebugImg(imgInspAreaTemp, str);
	}
	bool bBTC = sData.UseData(FR_BIN_DT_ExceptBTC);

	cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_HOLE], imgFR[FR_IMG_BIN]);
	cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_HEXT], imgFR[FR_IMG_BIN]);
	m_fFrTact[m_eFrTact_InspForeign_HA_SET] += ((GetTickCount() - st) / 1000.0f);
	st = GetTickCount();

	float f3DMin = sData.m_fArr[FR_BIN_F_HA_Under];
	float f3DMax = sData.m_fArr[FR_BIN_F_HA_Upper];
	int nRange3D = ETypeInspRange::eTypeRangeOut;
	if (bHA_Under && bHA_Upper)
	{
		cv::Mat img3DMin(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat img3DMax(nH, nW, CV_8UC1, cv::Scalar(0));

		ippiCompareC_32f_C1R(p3D, img3D.step, f3DMax, img3DMin.data, img3DMin.cols, { nW, nH }, ippCmpGreaterEq);
		ippiCompareC_32f_C1R(p3D, img3D.step, f3DMin, img3DMax.data, img3DMax.cols, { nW, nH }, ippCmpLessEq);

		memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
		cv::bitwise_or(img3DMin, img3DMax, imgFR[FR_IMG_BLOB]);
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BIN]);
	}
	else if (bHA_Upper)
	{
		memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
		ippiCompareC_32f_C1R(p3D, img3D.step, f3DMax, imgFR[FR_IMG_BLOB].data, imgFR[FR_IMG_BLOB].cols, { nW, nH }, ippCmpLessEq);
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BIN]);
		nRange3D = ETypeInspRange::eTypeRangeUpper;
	}
	else if (bHA_Under)
	{
		memset(imgFR[FR_IMG_BLOB].data, 0, sizeof(UCHAR) * nW * nH);
		ippiCompareC_32f_C1R(p3D, img3D.step, f3DMin, imgFR[FR_IMG_BLOB].data, imgFR[FR_IMG_BLOB].cols, { nW, nH }, ippCmpGreaterEq);
		cv::bitwise_and(imgFR[FR_IMG_BIN], imgFR[FR_IMG_BLOB], imgFR[FR_IMG_BIN]);
		nRange3D = ETypeInspRange::eTypeRangeLower;
	}

	float fHA_W = sData.m_fArr[FR_BIN_F_HA_W];
	float fHA_L = sData.m_fArr[FR_BIN_F_HA_L];
	float fHA_A = sData.m_fArr[FR_BIN_F_HA_A];
	double dHA_WPix = fHA_W / m_resolX / nDivide;
	double dHA_LPix = fHA_L / m_resolY / nDivide;
	double dHA_APix = fHA_A / m_resolX / m_resolY / (nDivide * nDivide);
	if (dHA_WPix < 0) dHA_WPix = 0;
	if (dHA_LPix < 0) dHA_LPix = 0;
	if (dHA_APix < 0) dHA_APix = 0;
	int nBinCnt = cv::countNonZero(imgFR[FR_IMG_BIN]);
	m_fFrTact[m_eFrTact_InspForeign_HA_Bin] += ((GetTickCount() - st) / 1000.0f);
	if (nBinCnt <= dHA_APix)
		return true;

	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imgFR[FR_IMG_BIN], allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	if (allcontour.size() == 0)
		return true;

	int nDefect = sData.GetDEFECT();
	if (bDEFECT)
		nDefect = Foreign_NG_Type_HeightArea;
	bool bSIZE_AREA = sData.UseData(FR_BIN_DT_SIZE_AREA);
	for (int a = 0; a < allcontour.size(); a++)
	{
		if (allcontour[a].size() <= 1)
			continue;

		cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
		std::vector<std::vector<cv::Point>> allcontourT;
		allcontourT.push_back(allcontour[a]);
		cv::drawContours(imgTemp, allcontourT, 0, cv::Scalar(255), cv::FILLED);
		cv::RotatedRect roi = cv::minAreaRect(allcontour[a]);
		int nArea = cv::countNonZero(imgTemp);
		if (nArea <= dHA_APix)
			continue;
		if (roi.size.width < dHA_WPix)
			continue;
		if (roi.size.height < dHA_LPix)
			continue;

		int nL = allcontour[a][0].x;
		int nR = allcontour[a][0].x;
		int nT = allcontour[a][0].y;
		int nB = allcontour[a][0].y;
		for (int b = 1; b < allcontour[a].size(); b++)
		{
			if (nL > allcontour[a][b].x) nL = allcontour[a][b].x;
			if (nR < allcontour[a][b].x) nR = allcontour[a][b].x;
			if (nT > allcontour[a][b].y) nT = allcontour[a][b].y;
			if (nB < allcontour[a][b].y) nB = allcontour[a][b].y;
		}
		nL -= 1;
		nR += 1;
		nT -= 1;
		nB += 1;
		if (nL < 0) nL = 0;
		if (nR < 0) nR = 0;
		if (nT < 0) nT = 0;
		if (nB < 0) nB = 0;
		if (nL >= nW) nL = nW - 1;
		if (nR >= nW) nR = nW;
		if (nT >= nH) nT = nH - 1;
		if (nB >= nH) nB = nH;

		int nROIW = nR - nL;
		int nROIH = nB - nT;
		if (nROIW <= 1 || nROIH <= 1)
			continue;

		RECT rcRect;
		rcRect.left = nL;
		rcRect.top = nT;
		rcRect.right = nR;
		rcRect.bottom = nB;
		int nRstCnt = 0;
		float fRstHeight = 0.0f;
		int nHCnt = 0;
		float fHeightSum = 0.0f;
		for (int y = nT; y < nB; y++)
		{
			for (int x = nL; x < nR; x++)
			{
				long long nIndex = (y * nW) + x;
				if (imgFR[FR_IMG_BIN].data[nIndex] != 255) continue;
				float fH = p3D[nIndex];
				fHeightSum += fH;
				nHCnt++;
			}
		}
		if (nHCnt == 0) continue;
		fRstHeight = fHeightSum / nHCnt;
		nRstCnt = nHCnt;

		int nWTemp = nW;
		int nHTemp = nH;
		if (rcRect.left >= rcRect.right)
			rcRect.right = rcRect.left + 1;
		if (rcRect.top >= rcRect.bottom)
			rcRect.bottom = rcRect.top + 1;
		int nSize = 0;
		if ((nWTemp % 4) > 0)
			nSize = 4 - (nWTemp % 4);
		int nSZCnt = 0;
		for (int nSZ = 1; nSZ <= nSize; nSZ++)
		{
			if (rcRect.left - 1 >= 0)
			{
				rcRect.left -= 1;
				nSZCnt++;
			}
			if (nSZCnt == nSize)
				break;
			if (rcRect.right + 1 < nW)
			{
				rcRect.right += 1;
				nSZCnt++;
			}
			if (nSZCnt == nSize)
				break;
		}

		int nModuleID = 0;
		int nRows = imgFR[FR_IMG_BIN].rows;
		POINTF ptCur[] = { rcRect.left, nRows - rcRect.top
							, rcRect.right, nRows - rcRect.top
							, rcRect.left, nRows - rcRect.bottom
							, rcRect.right, nRows - rcRect.bottom };
		for (int n = 0; n < 4; n++)
		{
			nModuleID = GetForeignModuleNumber(sForeign, sInspData, ptCur[n], nDivide, vInspectionMode, nContainModuleID);
			if (nModuleID > 0)
				break;
		}

		double dRssX = m_resolX * nDivide;
		double dRssY = m_resolY * nDivide;
		double dRstW = nROIW * dRssX;
		double dRstL = nROIH * dRssY;
		double dResVal = dRssX * dRssY;
		double dRstA = nArea * dResVal;
		if (nRstCnt > 0)
			dRstA = nRstCnt * dResVal;
		if (bSIZE_AREA)
			dRstA = dRstW * dRstL;

		ForeignResult->m_stForeign[nRealCnt].SetData(m_eFR_RST_D_HA, true);

		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Mode, true, nMode);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_ModuleID, true, nModuleID);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Range3D, true, nRange3D);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_Defect, true, nDefect);
		ForeignResult->m_stForeign[nRealCnt].GetN(m_eFR_RST_N_UVColorSelect, true);

		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_X, true, (rcRect.right + rcRect.left) / 2.0);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_Y, true, (rcRect.bottom + rcRect.top) / 2.0);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_W, true, dRstW);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_L, true, dRstL);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_A, true, dRstA);
		ForeignResult->m_stForeign[nRealCnt].GetF(m_eFR_RST_F_H, true, fRstHeight);

		ForeignResult->m_stForeign[nRealCnt].m_rcRect = rcRect;
		nRealCnt++;
		if (nRealCnt >= nMaxRst)
			break;
	}
	m_fFrTact[m_eFrTact_InspForeign_HA_Rst] += ((GetTickCount() - st) / 1000.0f);
	if (bSaveImg || (bSaveImg2 && nRealCnt > ForeignResult->m_nCountDefect))
	{
		CString str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_HA_Bin"), nMode);
		SaveDebugImg(imgFR[FR_IMG_BIN], str);

		str = GetForeignDebugDataFullPath(nFovID, _T("_Insp_FR_HA_INSP_AREA"), nMode);
		SaveDebugImg(imgInspAreaTemp, str);
	}
	ForeignResult->m_nCountDefect = nRealCnt;
	return ForeignResult->m_nCountDefect == 0;
}
void FOREIGN_INSP::GetInspAreaROI(InspForeignInfo sForeign, ForeignData sInspData, ForeignParamROI* arrROI, cv::Mat *imgInspArea, cv::Mat *imgFR, int nW, int nH, int nModuleTotal, int nFovID, int* nArrModuleID, int* nArrModuleCnt)
{
	CString sLog = _T("");
	DWORD st = GetTickCount();
	if (sInspData.m_nFRROI <= 0 || arrROI == NULL)
		return;

	int nDivide = sForeign.GetDivide();
	bool bTraceInsp = sForeign.IsForeignEnable(m_eForeignInsp_Foreign, m_eForeignInsp_Trace);
	bool bGND = sInspData.UseData(m_eForeignData_DATA_GND);
	bool bManual = sForeign.UseData(FR_DATA_Manual_EXT);
	if (imgInspArea[0].empty())
		imgInspArea[0] = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));

	if (bManual)
	{
		ExceptForeign(ForeignParamROI_Data_Foreign, imgFR[FR_IMG_EXT], sInspData);
		AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[GetInspAreaROI]"), _T("[ExceptForeign]"), st);
	}

	for (int a = 0; a < sInspData.m_nFRROI; a++)
	{
		int nModuleID = arrROI[a].m_nModuleID;
		if (bTraceInsp && m_nFR_TraceID > nModuleTotal)
		{
			if (nModuleID == m_nFR_TraceID)
			{
				int nVal = bGND ? 0 : 255;
				SET_IMG_DATA(imgFR[FR_IMG_TRACE], arrROI[a], nDivide, nVal);

				nVal = bGND ? 0 : 255;
				SET_IMG_DATA(imgFR[FR_IMG_TRACE_NONE], arrROI[a], nDivide, nVal);

			}
			else if (arrROI[a].m_nModuleID == m_nFR_TraceID + 1)
			{
				SET_IMG_DATA(imgFR[FR_IMG_TRACE_EXT], arrROI[a], nDivide);
			}
			continue;
		}
		if (arrROI[a].m_nType == ForeignParamROI_Data_InspA ||
			arrROI[a].m_nType == ForeignParamROI_Data_Module)
		{
			if (nModuleID > 0)
			{
				if (sInspData.m_nContainModuleID > 0 && sInspData.m_nContainModuleID != nModuleID)
					continue;

				if (sInspData.m_nModule > nModuleID)
				{
					if (sInspData.m_pInspModule[nModuleID] == 0)
						continue;
				}
				SET_IMG_DATA(imgInspArea[0], arrROI[a], nDivide);
			}
			if (imgInspArea[nModuleID].empty())
				continue;
			SET_IMG_DATA(imgInspArea[nModuleID], arrROI[a], nDivide);
		}
		else
		{
			SET_IMG_DATA(imgFR[FR_IMG_EXT], arrROI[a], nDivide);
		}
	}
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[GetInspAreaROI]"), _T("SET_IMG_DATA OUT"), st);

	for (int a = 0; a < nModuleTotal; a++)
	{
		if (imgInspArea[a].empty())
			continue;
		imgInspArea[a] = imgInspArea[a] - imgFR[FR_IMG_EXT];
		nArrModuleCnt[a] = cv::countNonZero(imgInspArea[a]);
	}
	AddLOG(_T("[FOREIGN_INSP]"), _T("[InspBlobForForeign]"), _T("[GetInspAreaROI]"), _T("END"), st);
}
void FOREIGN_INSP::SET_IMG_DATA(cv::Mat img, ForeignParamROI roiFR, int nDivide, int nVal)
{
	int nImgW = img.cols;
	int nImgH = img.rows;
	if (roiFR.m_bPolygon && (roiFR.m_nPolyCNT == ZMPOLYPOINT_ULTRA_CNTS || roiFR.m_nPolyCNT == ZMPOLYPOINT_CNTS))
	{
		std::vector<cv::Point> polyPoints;
		polyPoints.reserve(roiFR.m_nPolyCNT);
		for (int b = 0; b < roiFR.m_nPolyCNT; b++)
		{
			float fX = (roiFR.m_nPolyCNT == ZMPOLYPOINT_ULTRA_CNTS) ? roiFR.m_pROIHit[b].x : roiFR.m_pROI[b].x;
			float fY = (roiFR.m_nPolyCNT == ZMPOLYPOINT_ULTRA_CNTS) ? roiFR.m_pROIHit[b].y : roiFR.m_pROI[b].y;
			int nX = RounD(fX / nDivide);
			int nY = RounD(fY / nDivide);
			polyPoints.push_back(cv::Point(nX, nY));
		}
		cv::fillPoly(img, std::vector<std::vector<cv::Point>>{polyPoints}, cv::Scalar(nVal));
	}
	else
	{
		int nL = roiFR.m_pPos.x / nDivide;
		int nT = roiFR.m_pPos.y / nDivide;
		int nW = roiFR.m_pSize.x / nDivide;
		int nH = roiFR.m_pSize.y / nDivide;
		int nR = nL + nW;
		int nB = nT + nH;
		if (nL >= nImgW || nT >= nImgH)
			return;
		if (nW <= 0 || nH <= 0)
			return;

		if (nL < 0) nL = 0;
		if (nT < 0) nT = 0;
		if (nR > nImgW) nR = nImgW;
		if (nB > nImgH) nB = nImgH;
		int nRoiW = nR - nL;
		int nRoiH = nB - nT;
		if (nRoiW <= 0 || nRoiH <= 0)
			return;

		cv::Rect roi = cv::Rect(nL, nT, nRoiW, nRoiH);
		img(roi).setTo(nVal);
	}
}
bool FOREIGN_INSP::InspSolderBall(cv::Mat src, std::vector<CRect> & rcBlob, int* nMaxIndex, float* ptrResult, double* ptrArrRst)
{
	bool bReturn = true;

	double MaxScore(0);
	int MaxIndex(0);
	int szImgW = src.cols;
	int szImgL = src.rows;
	int nBlobCnt = rcBlob.size();
	int nWidth = (szImgW >= szImgL) ? szImgW : szImgL;

	for (int i = 0; i < nBlobCnt; i++)
	{
		cv::Rect rc(rcBlob[i].left, rcBlob[i].top, rcBlob[i].Width(), rcBlob[i].Height());

		rc.x = rc.x < 0 ? 0 : rc.x;
		rc.y = rc.y < 0 ? 0 : rc.y;
		if ((rc.x + rc.width) > szImgW)
			rc.width -= (rc.x + rc.width) - szImgW;
		if ((rc.y + rc.height) > szImgL)
			rc.height -= (rc.y + rc.height) - szImgL;

		cv::Mat clipROI = src(rc).clone();
		cv::Mat FullROI = cv::Mat::zeros(nWidth, nWidth, CV_8UC1);
		UCHAR* pucClipROI = clipROI.data;
		UCHAR* pucFullROI = FullROI.data;

		int nClipWidth = clipROI.cols;
		int nClipHeight = clipROI.rows;
		CPoint ptClipCen(nClipWidth / 2, nClipHeight / 2);
		CPoint ptFullCen(nWidth / 2, nWidth / 2);

		int dX = ptFullCen.x - ptClipCen.x;
		int dY = ptFullCen.y - ptClipCen.y;
		if (dX < 0) dX = 0;
		if (dY < 0) dY = 0;
		int nR = dX + nClipWidth;
		int nB = dY + nClipHeight;
		if (nR >= nWidth) nR = nWidth - 1;
		if (nB >= nWidth) nB = nWidth - 1;

		int nWndIDX = 0;
		for (int m = dY; m < nB; m++)
		{
			for (int n = dX; n < nR; n++)
			{
				int nIndex = (m * nWidth) + n;
				pucFullROI[nIndex] = pucClipROI[nWndIDX];
				nWndIDX++;
			}
		}

		double dErr(1.);

		cv::Mat Ecircle = FullROI.clone();

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		cv::findContours(Ecircle, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
		if (contours.size() > 0)
		{
			std::vector<cv::Point> selectedCon;
			for (int cCnt = 0; cCnt < contours.size(); cCnt++)
			{
				if (contours[cCnt].size() < 6)
					continue;

				if (contours[cCnt].size() < selectedCon.size())
					continue;

				selectedCon = contours[cCnt];
			}

			if (selectedCon.size() > 6)
			{
				double radi(0.0), dCirle_x(0.0), dCirle_y(0.0);

				cv::Rect rcBounding = cv::boundingRect(selectedCon);
				double dCircleRatio = (double)rcBounding.width / rcBounding.height;
				double dCircleTol = 2.0;
				bool bCircleSimilar = 1.0 / dCircleTol < dCircleRatio && dCircleRatio < dCircleTol;

				if (bCircleSimilar == true)
					GetSolderBallPos(selectedCon, &dCirle_x, &dCirle_y, &radi, &dErr);
			}
		}
		else
			bReturn = false;

		dErr = 1. - dErr;
		if (dErr < 0)
			dErr = 0.0;
		if (MaxScore < dErr)
		{
			MaxIndex = i;
			MaxScore = dErr;
		}
		if (ptrArrRst != NULL)
			ptrArrRst[i] = dErr;
	}

	if (nMaxIndex != NULL)
		*nMaxIndex = MaxIndex;
	if (ptrResult != NULL)
		*ptrResult = MaxScore;

	return bReturn;
}
void FOREIGN_INSP::GetSolderBallPos(std::vector<cv::Point>&  Points, double *dCX, double *dCY, double *dRadius, double *dErr)
{
	int nBorderCnt = Points.size();

	double theta;
	double s_cos = 0, s_sin = 0;
	double xy = 0, x = 0, y = 0;
	int cnt = nBorderCnt;

	double err2 = 0;

	double ds_Xn2 = 0, ds_Yn2 = 0, ds_XnYn = 0, ds_Xn = 0, ds_Yn = 0;
	double dsub1 = 0, dsub2 = 0, dsub3 = 0;

	for (int i = 0; i < nBorderCnt; i++)
	{
		double x, y;
		x = Points[i].x;
		y = Points[i].y;

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

	double* org_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* inv_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* sub_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize, (PCHAR)__FUNCTION__, __LINE__);

	org_m[0] = ds_Xn2; org_m[1] = ds_XnYn; org_m[2] = ds_Xn;
	org_m[3] = ds_XnYn; org_m[4] = ds_Yn2; org_m[5] = ds_Yn;
	org_m[6] = ds_Xn; org_m[7] = ds_Yn; org_m[8] = cnt;

	sub_m[0] = dsub1; sub_m[1] = dsub2; sub_m[2] = dsub3;

	cv::Mat org(3, 3, CV_64FC1, org_m);
	cv::Mat sub(3, 1, CV_64FC1, sub_m);
	cv::Mat dst = org.inv() * sub;

	*dCX = (float)dst.at<double>(0, 0) / 2;
	*dCY = (float)dst.at<double>(1, 0) / 2;
	*dRadius = (float)((*dCX**dCX) + (*dCY**dCY) + dst.at<double>(2, 0));
	*dRadius = sqrt(*dRadius);

	if (*dRadius < 0) *dRadius = -*dRadius;


	int m_borderCnt = 0;
	for (int i = 0; i < nBorderCnt; i++)
	{
		m_borderCnt++;
		cv::Point pt = Points[i];

		double dist = sqrt(SQR(pt.x - *dCX) + SQR(pt.y - *dCY));
		double distdeviate = fabs(dist - *dRadius) / *dRadius;

		if (distdeviate > 0.07)
			err2++;
	}
	*dErr = err2 / nBorderCnt;

	if (org_m)
	{
		g_pMManager->pem_free(org_m);
		org_m = NULL;
	}

	if (inv_m)
	{
		g_pMManager->pem_free(inv_m);
		inv_m = NULL;
	}

	if (sub_m)
	{
		g_pMManager->pem_free(sub_m);
		sub_m = NULL;
	}

	return;
}
void FOREIGN_INSP::AddLOG(CString sType, CString sFunc, CString sFunc2, CString sMSG, DWORD st, m_eLogLv eLogLv)
{
	if (g_pMPTI == NULL || g_pMPTI == nullptr)
		return;

	CString sLog = _T("");
	if (st > 0)
	{
		double tact = (GetTickCount() - st) / 1000.0;  // 초 단위로 변환
		sLog.Format(_T("%s %s %s %s [%.3f]"), sType, sFunc, sFunc2, sMSG, tact);
	}
	else
		sLog.Format(_T("%s %s %s %s"), sType, sFunc, sFunc2, sMSG);
	g_pMPTI->AddLog_Dev(sLog, eLogLv);
}
#pragma endregion INSP

#pragma region EX
int  MPTI_FR_EXCEPT(FR_Bin sData, FR_EXT_SRC sInspData, FR_EXT_RST* vROI)
{
	if (g_pInspMng == NULL)
		return -1;

	return g_pInspMng->m_FR.FR_EXCEPT(sData, sInspData, vROI);
}
int  MPTI_FR_HIT_AREA(FR_FIND_HIT sInspData, POINT* poPoly)
{
	if (g_pInspMng == NULL)
		return -1;

	return g_pInspMng->m_FR.FR_HIT_AREA(sInspData, poPoly);
}
void MPTI_Get2DFOV_Resize(InspRoiImgBuf sFov, FR2DData pfRst)
{
	if (g_pInspMng == NULL)
		return;

	g_pInspMng->m_FR.Get2DFOV_Resize(sFov, pfRst);
}
int MPTI_FR_AutoPseudo(float* pfSrc, UCHAR* pucWP, int nW, int nH, double dMin, double dMax, BOOL bSET)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	return g_pInspMng->m_FR.FR_AutoPseudo(pfSrc, pucWP, nW, nH, dMin, dMax, bSET);
}
int MPTI_CalcCorrWarpage(ForeignData sInspData, AForeignResultWP &retResult, float* pfArrTact)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	return g_pInspMng->m_FR.CalcCorrWarpage(sInspData, retResult, pfArrTact);
}
void MPTI_FrACImage(InspRoiImgBuf sLightImg, lightData sLightData, UCHAR* pucImage)
{
	if (g_pInspMng == NULL)
		return;

	if (sLightImg.nImageSizeX <= 0 || sLightImg.nImageSizeY <= 0)
		return;
	if (sLightImg.imgTop_R == NULL || sLightImg.imgTop_G == NULL || sLightImg.imgTop_B == NULL || sLightImg.imgTop_W == NULL ||
		sLightImg.imgMiddle_R == NULL || sLightImg.imgMiddle_B == NULL || sLightImg.imgBottom_R == NULL || sLightImg.imgBottom_B == NULL)
		return;

	LightTypeBuf sLightTypeBuf;
	sLightTypeBuf.m_pucTRed = sLightImg.imgTop_R;
	sLightTypeBuf.m_pucTGreen = sLightImg.imgTop_G;
	sLightTypeBuf.m_pucTBlue = sLightImg.imgTop_B;
	sLightTypeBuf.m_pucTWhite = sLightImg.imgTop_W;

	sLightTypeBuf.m_pucMRed = sLightImg.imgMiddle_R;
	sLightTypeBuf.m_pucMGreen = NULL;
	sLightTypeBuf.m_pucMBlue = sLightImg.imgMiddle_B;
	sLightTypeBuf.m_pucMWhite = NULL;

	sLightTypeBuf.m_pucBRed = sLightImg.imgBottom_R;
	sLightTypeBuf.m_pucBGreen = NULL;
	sLightTypeBuf.m_pucBBlue = sLightImg.imgBottom_B;
	sLightTypeBuf.m_pucBWhite = NULL;

	sLightTypeBuf.m_nImgWidth = sLightImg.nImageSizeX;
	sLightTypeBuf.m_nImgHeight = sLightImg.nImageSizeY;
	sLightTypeBuf.m_dROIX = 0;
	sLightTypeBuf.m_dROIY = 0;
	sLightTypeBuf.m_nROIImgWidth = sLightTypeBuf.m_nImgWidth;
	sLightTypeBuf.m_nROIImgHeight = sLightTypeBuf.m_nImgHeight;

	sLightTypeBuf.m_pnRedValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnGreenValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnBlueValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnWhiteValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnPosition = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightTypeBuf.m_pnCalculation = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);

	cv::Mat img(sLightTypeBuf.m_nImgHeight, sLightTypeBuf.m_nImgWidth, CV_8UC1, pucImage);
	g_pInspMng->m_FR.GetAngleColorFR(sLightTypeBuf, sLightData, img);

	Delete_1DArray(&sLightTypeBuf.m_pnRedValue);
	Delete_1DArray(&sLightTypeBuf.m_pnGreenValue);
	Delete_1DArray(&sLightTypeBuf.m_pnBlueValue);
	Delete_1DArray(&sLightTypeBuf.m_pnWhiteValue);
	Delete_1DArray(&sLightTypeBuf.m_pnPosition);
	Delete_1DArray(&sLightTypeBuf.m_pnCalculation);
}
void MPTI_Foreign_Value_RGB(InspRoiImgBuf sFov, RECT* rcROI)
{
	if (rcROI == NULL)
		return;
	if (sFov.imgTop_R == NULL || sFov.imgTop_G == NULL || sFov.imgTop_B == NULL)
		return;
	if (sFov.nImageSizeX <= 0 || sFov.nImageSizeY <= 0)
		return;

	int nW = sFov.nImageSizeX;
	int nH = sFov.nImageSizeY;
	int nX = rcROI[2].left;
	int nY = rcROI[2].top;
	if (nX < 0 || nX >= nW || nY < 0 || nY >= nH)
		return;

	int nIndex = (nY * nW) + nX;
	rcROI[0].left = sFov.imgTop_R[nIndex];
	rcROI[0].right = sFov.imgTop_G[nIndex];
	rcROI[0].top = sFov.imgTop_B[nIndex];
}
void MPTI_Foreign_FullMap(IN UCHAR* retFullImg, IN int nFullW, IN int nFullH, UCHAR* byR, UCHAR* byG, UCHAR* byB)
{
	if (retFullImg == NULL || nFullW <= 0 || nFullH <= 0)
		return;
	if (byR == NULL || byG == NULL || byB == NULL)
		return;

	cv::Mat imgFull(nFullH, nFullW, CV_8UC3, retFullImg);
	cv::Mat imgRGB[FR_Find_RGB_Total];
	cv::split(imgFull, imgRGB);

	memcpy(byR, imgRGB[FR_Find_RGB_TopR].ptr<UCHAR>(), sizeof(UCHAR) * nFullW * nFullH);
	memcpy(byG, imgRGB[FR_Find_RGB_TopG].ptr<UCHAR>(), sizeof(UCHAR) * nFullW * nFullH);
	memcpy(byB, imgRGB[FR_Find_RGB_TopB].ptr<UCHAR>(), sizeof(UCHAR) * nFullW * nFullH);

#if _DEBUG

	cv::Mat imgR(nFullH, nFullW, CV_8UC1, byR);
	cv::Mat imgG(nFullH, nFullW, CV_8UC1, byG);
	cv::Mat imgB(nFullH, nFullW, CV_8UC1, byB);
#endif
}
void MPTI_SaveZmapPTR(float *pZmap, wchar_t* filePath, int nW, int nH)
{
	if (g_pInspMng == NULL) return;
	g_pInspMng->m_FR.SaveZmapPTR(pZmap, filePath, nW, nH);
}
POINTF MPTI_Foreign_Value(ForeignData sInspData)
{
	POINTF poVal;
	poVal.x = 0;
	poVal.y = 0;

	float *pfOrg3D = sInspData.m_p3D;
	UCHAR *puc2D = sInspData.m_pDesImgBin;
	int nFovW = sInspData.m_nWP_W;
	int nFovH = sInspData.m_nWP_H;
	int nOrgW = g_pInspMng->GetFovWidth();
	int nOrgH = g_pInspMng->GetFovLength();
	if (pfOrg3D == NULL || puc2D == NULL || nFovW <= 0 || nFovH <= 0)
		return poVal;
	if (nOrgW <= nFovW || nOrgH <= nFovH)
		return poVal;

#if _DEBUG
	cv::Mat img3D(nOrgH, nOrgW, CV_32FC1, pfOrg3D);
	cv::Mat img2D(nFovH, nFovW, CV_8UC1, puc2D);
#endif

	int nX = sInspData.m_rcFOV.left;
	int nY = sInspData.m_rcFOV.top;
	if (nX < 0 || nX >= nFovW)
		return poVal;

	if (nY < 0 || nY >= nFovH)
		return poVal;

	long long nIndex2 = (nY * nFovW) + nX;
	long long nIndex3 = (nY * nOrgW) + nX;
	poVal.x = pfOrg3D[nIndex3];
	poVal.y = puc2D[nIndex2];

	return poVal;
}
void MPTI_SaveWarpageSUB(ForeignData sInspData, wchar_t* sPath)
{
	if (g_pInspMng == NULL) return;
	g_pInspMng->m_FR.SaveWarpageSUB(sInspData, sPath);
}
void MPTI_GetWP3D(wchar_t* sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer)
{
	if (g_pInspMng == NULL) return;

	g_pInspMng->m_FR.GetWP3D(sPath, retFullImg, nWidth, nHeight, rcClip, dMin, dMax, dPer);
}
void MPTI_GetWP3DM(wchar_t* sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, IN UCHAR* retFullImgM, IN int nWidthM, IN int nHeightM, RECT rcClip, double dMin, double dMax, double dPer, double dFacX, double dFacY)
{
	if (g_pInspMng == NULL) return;

	g_pInspMng->m_FR.GetWP3DM(sPath, retFullImg, nWidth, nHeight, retFullImgM, nWidthM, nHeightM, rcClip, dMin, dMax, dPer, dFacX, dFacY);
}
void MPTI_GetWP3DPTT(wchar_t* sPath, IN float* retFullImg, IN UCHAR* ptrTR, IN UCHAR* ptrTG, IN UCHAR* ptrTB, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer, double dFacW, double dFacH)
{
	if (g_pInspMng == NULL) return;

	g_pInspMng->m_FR.GetWP3DPTT(sPath, retFullImg, ptrTR, ptrTG, ptrTB, nWidth, nHeight, rcClip, dMin, dMax, dPer, dFacW, dFacH);
}
void MPTI_GetWP3DIMG(float* pf3D, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, int nModuleID, wchar_t* sPath)
{
	if (g_pInspMng == NULL) return;

	g_pInspMng->m_FR.GetWP3D(pf3D, retFullImg, nWidth, nHeight, rcClip, dMin, dMax, nModuleID, sPath);
}
int MPTI_GetColorRGB(InspRoiImgBuf sFov, RECT* rcRGB)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	g_pInspMng->m_FR.GetColorRGB(sFov.imgTop_R, sFov.imgTop_G, sFov.imgTop_B, sFov.nImageSizeX, sFov.nImageSizeY, rcRGB);

	return eMR_SUCCESS;
}
int MPTI_GetFullColorRGB(IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT* rcRGB)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	g_pInspMng->m_FR.GetColorRGB(retFullImg, nWidth, nHeight, rcRGB);

	return eMR_SUCCESS;
}
int  MPTI_SaveDisplaynForeignNg(InspFR_Save sData, UCHAR * ucDst)
{
	if (g_pInspMng == NULL)
		return FALSE;

	if (sData.UseData(m_eInspFR_Save_Data_ONLINE))
		g_pInspMng->m_FR.ForeignNG_Save(sData, ucDst);
	else
		g_pInspMng->m_FR.ForeignNG_Save_OFF(sData, ucDst);

	return TRUE;
}
int MPTI_FrFindArea(ForeignData sInspData, InspRoiImgBuf sFov, FR_Find sFind, RECT* rcROI)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	return g_pInspMng->m_FR.FrFindArea(sInspData, sFov, sFind, rcROI);
}
double MPTI_GetColorArea(InspRoiImgBuf sFov, FR_Bin sBin, RECT rcROI)
{
	if (g_pInspMng == NULL)
		return 0;

	return g_pInspMng->m_FR.GetColorArea(sFov, sBin, rcROI);
}
#pragma endregion EX

#pragma region _Foreign_Func_
int MPTI_GetFullColorCIEView_Foreign(IN UCHAR* retFullImg, IN int nFullW, IN int nFullH, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (!pColorTeach) return eMR_FAIL;

	return pColorTeach->GetFullColorCIEView_Foreign(retFullImg, nWidth, nHeight, retCie, nCieW, nCieH, retImg, nWidth, nHeight, pt, rcROI);
}
int  MPTI_COLOR_GetColorHistogramView_Foreign(int nBin, int projectionmode, RECT rcROI, UCHAR* pucDstImg)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (!pColorTeach) return eMR_FAIL;

	pColorTeach->Color_GetColorHistogramView_Foreign(nBin, projectionmode, rcROI, pucDstImg);
	return eMR_SUCCESS;
}
int  MPTI_GetColorCIEView_Foreign(InspRoiImgBuf sFov, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (!pColorTeach) return eMR_FAIL;

	pColorTeach->GetColorCIEView_Foreign(sFov, retCie, nCieW, nCieH, retImg, nWidth, nHeight, pt, rcROI);

	return eMR_SUCCESS;
}
#pragma endregion _Foreign_Func_

#pragma region EX_INSP
void MPTI_SetForeignParam(InspForeignInfo ForeignParam, ForeignParamROI* vForeignParamROI)
{
	if (g_pInspMng == NULL)
		return;

	g_pInspMng->m_FR.SetForeignParam(ForeignParam, vForeignParamROI);
}
int  MPTI_InspProc_Foreign(ForeignData sInspData, FR2DData sFR2DData, ForeignParamROI* arrROI)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	return g_pInspMng->m_FR.Inspection_Foreign_new(sInspData, sFR2DData, arrROI);
}
int  MPTI_GetInspForeignResult(InspFovForeignResult* retResult)
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	return g_pInspMng->m_FR.GetInspForeignResult(retResult);
}
int  MPTI_ReleaseInspForeignResult()
{
	if (g_pInspMng == NULL)
		return eMR_FAIL;

	return g_pInspMng->m_FR.ReleaseInspForeignResult();
}
#pragma endregion EX_INSP