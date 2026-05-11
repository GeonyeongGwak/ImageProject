#include "stdafx.h"
#include "Pinsp_Crescent.h"
#include "MPTI.h"

CPInsp_Crescent::CPInsp_Crescent()
{
}
CPInsp_Crescent::~CPInsp_Crescent()
{
}
bool CPInsp_Crescent::getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection)
{
	cv::Mat Img = image;		//Foot Image
	cv::Mat mMask = mMaskImage;		//Foot Image
	
	cv::Point FootCenterPo;
	double seta_Foot(0.0);
	bool flag = false;
	//flag = Insp_FootPosition(&Img, &FootCenterPo, &seta_Foot, mMask, nFootDirection, WingLength);
	
	///////////////////////////////
	//new algorithm 

	//////////////////////////////

	CenterPo = FootCenterPo;
	seta = seta_Foot;

#if _DEBUG
	cv::imwrite("D:\\FootRst\\getFootCenterPosAndSeta_OUTPUT_Img.bmp", Img);
#endif

	return flag;
}

int CPInsp_Crescent::FindFoot_Pat(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, cv::Point* pCenter, double* pAngle, int nfootKind, int nFootDirection, UCHAR *ucArrDstImg, cv::Mat *wingCandiSecond, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	
	int nLine = __LINE__;
	try
	{
		cv::Mat ImgPatTarg;
		int nImageHeight = pFoot->GetImageLength();
		int nImageWidth = pFoot->GetImageWidth();

		pFoot->GetImageMatrix(cvFootImg, (int)m_eFootBin::m_eFootBin_Foot);
		pFoot->GetImageMatrix(cvWedgeImg, (int)m_eFootBin::m_eFootBin_Wedge);
		pFoot->GetImageMatrix(cvWing, (int)m_eFootBin::m_eFootBin_Wing);

		for (int r = 0; r < nImageHeight; r++)
		{
			UCHAR* TarPtr = ImgPatTarg.ptr(r);
			UCHAR* imgSrc = &pFoot->m_ImageBuffer->imgTop_W[r*nImageWidth];
			memcpy(TarPtr, imgSrc, sizeof(UCHAR)*nImageWidth);
		}

		cv::Mat BinImage(cvFootImg->rows, cvFootImg->cols, CV_8UC1);
		cv::Mat srcImg = *cvFootImg;
		cv::Mat srcImg1 = *cvWedgeImg;
		cv::Mat srcImg2 = *cvWing;

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindFoot_Org_Foot.bmp", srcImg);
		cv::imwrite("D:\\FootRst\\FindFoot_Org_Wedge.bmp", srcImg1);
		cv::imwrite("D:\\FootRst\\FindFoot_Org_Wing.bmp", srcImg2);		//bonding area 
#endif
		int nSearchSizeX = pFoot->m_fArrOptionValue[(int)m_eFOOT_Data::m_eFOOT_Data_ShiftX][m_eMMD::eMMD_Default] / m_resolX;
		int nSearchSizeY = pFoot->m_fArrOptionValue[(int)m_eFOOT_Data::m_eFOOT_Data_ShiftY][m_eMMD::eMMD_Default] / m_resolX;
		
		if (nSearchSizeX < 10) nSearchSizeX = 10;
		if (nSearchSizeX > nImageWidth) nSearchSizeX = nImageWidth - 1;
		if (nSearchSizeY < 10) nSearchSizeY = 10;
		if (nSearchSizeY > nImageHeight) nSearchSizeY = nImageHeight - 1;

		double dArea = 0, dCX = 0, dCY = 0;
		CRect rcBlob{ 0,0,0,0 };
		float fHIMin = 0.0f;
		float fHIMax = 0.0f;

		CPInsp::GetHeightMinMax(pFoot->pf3D, nImageWidth, nImageHeight, &fHIMin, &fHIMax);

		sRstAlgo->m_rcRefArea.left = 0;
		sRstAlgo->m_rcRefArea.top = 0;
		sRstAlgo->m_rcRefArea.right = 0;
		sRstAlgo->m_rcRefArea.bottom = 0;
		sRstAlgo->m_fAreaHeight = 0.0f;

		AlgoBlob sAlgoBlob = pFoot->GetAlgoBlob((int)m_eFootBin::m_eFootBin_Foot); 

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

		int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, srcImg.data, pFoot->pf3D, NULL, srcImg.cols, srcImg.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
		
		if (nCntBlob == 0)
		{
			return 0;
		}

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Blob_Foot.bmp", BinImage);
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

		bool bMatchSuccess = false;
		cv::Mat cvMatchingMask(nImgHeight, nImgWidth, CV_8UC1);
		
		cv::Mat cv3DArea(nImgHeight, nImgWidth, CV_32FC1, pFoot->pf3D);
		if (nfootKind != -2 && ntempCntBlob/* nCntBlob*/ > 1)
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
				Delete_1DArray(&pucImg1);
				Delete_1DArray(&pucImg2);
				Delete_1DArray(&pfImg);
				Delete_1DArray(&LabelImage);
				return 0;
			}

			if (nFootR <= 0 || nFootB <= 0)
			{ 
				Delete_1DArray(&pucImg);
				Delete_1DArray(&pucImg1);
				Delete_1DArray(&pucImg2);
				Delete_1DArray(&pfImg);
				Delete_1DArray(&LabelImage);
				return 0;
			}

			std::shared_ptr<GeoResult_Foot> pPatResult = std::make_shared<GeoResult_Foot>();
			cv::Rect RectModelImg[4];
			int MatchModelNum[2];
			pPatResult->Alloc(2);
			if (m_bMatch2D)
				bMatchSuccess = MatchModel(pFoot, ImgPatTarg, *cvPadImg, pPatResult, RectModelImg, BinImage, cv3DArea, MatchModelNum);
			if (m_bMatch3D && !bMatchSuccess)
				bMatchSuccess = MatchModel3D(pFoot, cv3DArea, *cvPadImg, pPatResult, RectModelImg, BinImage, MatchModelNum);
			int MatchingIndex = -1;
			RECT rcArea;

			if (bMatchSuccess)
			{
				double diff1 = std::pow(pPatResult->_Center_y[0] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[0] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
				double diff2 = std::pow(pPatResult->_Center_y[1] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[1] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
				MatchingIndex = 0;
				if (diff1 > diff2)
					MatchingIndex = 1;
				sRstAlgo->m_nMatchModelNum = MatchModelNum[MatchingIndex];
				sRstAlgo->MatchingScore = pPatResult->_Equality[MatchingIndex];
				pPatResult->_Equality[0] = pPatResult->_Equality[1] = 0;

				double MatchingAngle = pPatResult->_Angle[MatchingIndex];
				if (pFoot->m_bUsePatternAngle)
				{
					if (m_bMatch2D)
						bMatchSuccess = MatchModel(pFoot, ImgPatTarg, *cvPadImg, pPatResult, RectModelImg, BinImage, cv3DArea, MatchModelNum, MatchingAngle);
					if (m_bMatch3D && !bMatchSuccess)
						bMatchSuccess = MatchModel3D(pFoot, cv3DArea, *cvPadImg, pPatResult, RectModelImg, BinImage, MatchModelNum, MatchingAngle);
				}

				diff1 = std::pow(pPatResult->_Center_y[0] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[0] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
				diff2 = std::pow(pPatResult->_Center_y[1] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[1] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
				MatchingIndex = 0;
				if (diff1 > diff2)
					MatchingIndex = 1;
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

				double dRadian1 = (-pPatResult->_Angle[MatchingIndex] - (dstdDegree)) * std::_Pi / 180;
				double dRadian2 = (-pPatResult->_Angle[MatchingIndex] - (180 - dstdDegree)) * std::_Pi / 180;
				double dRadian3 = (-pPatResult->_Angle[MatchingIndex] - (180 + dstdDegree)) * std::_Pi / 180;
				double dRadian4 = (-pPatResult->_Angle[MatchingIndex] - (360 - dstdDegree)) * std::_Pi / 180;
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
				cv::fillPoly(cvMatchingMask, ptr, nps, 1, cv::Scalar(0));

				*pAngle = -pPatResult->_Angle[MatchingIndex];
				*pCenter = cv::Point(cx, cy);
				pFoot->m_BondingRect.left = RectModelImg[MatchingIndex + 2].x;
				pFoot->m_BondingRect.top = RectModelImg[MatchingIndex + 2].y;
				pFoot->m_BondingRect.right = RectModelImg[MatchingIndex + 2].x + RectModelImg[MatchingIndex + 2].width;
				pFoot->m_BondingRect.bottom = RectModelImg[MatchingIndex + 2].y + RectModelImg[MatchingIndex + 2].height;
#if _DEBUG
				cv::imwrite("D:\\FootRst\\bMatchSuccess_cvMatchingMask.bmp", cvMatchingMask);
#endif

				int nLeft1 = p1x < p2x ? p1x : p2x;
				int nLeft2 = p3x < p4x ? p3x : p4x;
				rcArea.left = nLeft1 < nLeft2 ? nLeft1 : nLeft2;

				int nRight1 = p1x > p2x ? p1x : p2x;
				int nRight2 = p3x > p4x ? p3x : p4x;
				rcArea.right = nRight1 > nRight2 ? nRight1 : nRight2;

				int nTop1 = p1y < p2y ? p1y : p2y;
				int nTop2 = p3y < p4y ? p3y : p4y;
				rcArea.top = nTop1 < nTop2 ? nTop1 : nTop2;

				int nBottom1 = p1y > p2y ? p1y : p2y;
				int nBottom2 = p3y > p4y ? p3y : p4y;
				rcArea.bottom = nBottom1 > nBottom2 ? nBottom1 : nBottom2;
			}
			else
			{
				rcArea = CPInsp::GetBlobRect(&vecLabelOrder[0], LabelImage, nCntBlob, nImgWidth, nImgHeight, nFootL, nFootR, nFootT, nFootB);
				if (rcArea.left == -1 || rcArea.right == -1 || rcArea.top == -1 || rcArea.bottom == -1)
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
			}

			int nAreaGapX = -5.;
			int nAreaGapY = -5.;
			int nSX = rcArea.left + nAreaGapX;
			int nEX = rcArea.right - nAreaGapX;
			int nSY = rcArea.top + nAreaGapY;
			int nEY = rcArea.bottom - nAreaGapY;
			if ((pFoot->m_nUseOption2 & m_eFOOT_Data2_Dir_Up) == m_eFOOT_Data2_Dir_Up)
				nSY = 0;
			else
				nEY = nImgHeight;
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
						UCHAR ucG = cvFootImg->data[y * nImgWidth + x];
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
						if ((nFindRange3D == 0 && (fFindMin3D <= fH && fH <= fFindMax3D)) ||
							(nFindRange3D == 1 && (fFindMin3D > fH || fH < fFindMax3D)) ||
							(nFindRange3D == 2 && fH >= fFindMax3D) ||
							(nFindRange3D == 3 && fFindMin3D >= fH))
							b2DIn = true;
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
		else 
		{
			std::shared_ptr<GeoResult_Foot> pPatResult = std::make_shared<GeoResult_Foot>();
			cv::Rect RectModelImg[4];
			int MathcingModelNum[2];
			pPatResult->Alloc(2);
			if (m_bMatch2D)
				bMatchSuccess = MatchModel(pFoot, ImgPatTarg, *cvPadImg, pPatResult, RectModelImg, BinImage, cv3DArea, MathcingModelNum);
			if (m_bMatch3D && !bMatchSuccess)
				bMatchSuccess = MatchModel3D(pFoot, cv3DArea, *cvPadImg, pPatResult, RectModelImg, BinImage, MathcingModelNum);
			int MatchingIndex = -1;
			RECT rcArea;
			if (bMatchSuccess)
			{
				double diff1 = std::pow(pPatResult->_Center_y[0] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[0] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
				double diff2 = std::pow(pPatResult->_Center_y[1] - pFoot->m_FootCenterPoint_ImageRect.x, 2) + std::pow(pPatResult->_Center_x[1] - pFoot->m_FootCenterPoint_ImageRect.y, 2);
				MatchingIndex = 0;
				if (diff1 > diff2)
					MatchingIndex = 1;
				sRstAlgo->m_nMatchModelNum = MathcingModelNum[MatchingIndex];
				sRstAlgo->MatchingScore = pPatResult->_Equality[MatchingIndex];

				double MatchingAngle = pPatResult->_Angle[MatchingIndex];

				if (pFoot->m_bUsePatternAngle)
				{
					if (m_bMatch2D)
						bMatchSuccess = MatchModel(pFoot, ImgPatTarg, *cvPadImg, pPatResult, RectModelImg, BinImage, cv3DArea, MathcingModelNum, MatchingAngle);
					if (m_bMatch3D && !bMatchSuccess)
						bMatchSuccess = MatchModel3D(pFoot, cv3DArea, *cvPadImg, pPatResult, RectModelImg, BinImage, MathcingModelNum, MatchingAngle);
				}
				MatchingIndex = 0;
				if (diff1 > diff2)
					MatchingIndex = 1;
				sRstAlgo->m_nMatchModelNum = MathcingModelNum[MatchingIndex];
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

				double dRadian1 = (-pPatResult->_Angle[MatchingIndex] - (dstdDegree)) * std::_Pi / 180;
				double dRadian2 = (-pPatResult->_Angle[MatchingIndex] - (180 - dstdDegree)) * std::_Pi / 180;
				double dRadian3 = (-pPatResult->_Angle[MatchingIndex] - (180 + dstdDegree)) * std::_Pi / 180;
				double dRadian4 = (-pPatResult->_Angle[MatchingIndex] - (360 - dstdDegree)) * std::_Pi / 180;
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
				cv::fillPoly(cvMatchingMask, ptr, nps, 1, cv::Scalar(0));

				*pAngle = -pPatResult->_Angle[MatchingIndex];
				*pCenter = cv::Point(cx, cy);
				pFoot->m_BondingRect.left = RectModelImg[MatchingIndex + 2].x;
				pFoot->m_BondingRect.top = RectModelImg[MatchingIndex + 2].y;
				pFoot->m_BondingRect.right = RectModelImg[MatchingIndex + 2].x + RectModelImg[MatchingIndex + 2].width;
				pFoot->m_BondingRect.bottom = RectModelImg[MatchingIndex + 2].y + RectModelImg[MatchingIndex + 2].height;
#if _DEBUG
				cv::imwrite("D:\\FootRst\\bMatchSuccess_cvMatchingMask.bmp", cvMatchingMask);
#endif

				int nLeft1 = p1x < p2x ? p1x : p2x;
				int nLeft2 = p3x < p4x ? p3x : p4x;
				rcArea.left = nLeft1 < nLeft2 ? nLeft1 : nLeft2;

				int nRight1 = p1x > p2x ? p1x : p2x;
				int nRight2 = p3x > p4x ? p3x : p4x;
				rcArea.right = nRight1 > nRight2 ? nRight1 : nRight2;

				int nTop1 = p1y < p2y ? p1y : p2y;
				int nTop2 = p3y < p4y ? p3y : p4y;
				rcArea.top = nTop1 < nTop2 ? nTop1 : nTop2;

				int nBottom1 = p1y > p2y ? p1y : p2y;
				int nBottom2 = p3y > p4y ? p3y : p4y;
				rcArea.bottom = nBottom1 > nBottom2 ? nBottom1 : nBottom2;
			}
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
						UCHAR ucG = cvFootImg->data[y * nImgWidth + x];
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
						if ((nFindRange3D == 0 && (fFindMin3D <= fH && fH <= fFindMax3D)) ||
							(nFindRange3D == 1 && (fFindMin3D > fH || fH < fFindMax3D)) ||
							(nFindRange3D == 2 && fH >= fFindMax3D) ||
							(nFindRange3D == 3 && fFindMin3D >= fH))
							b2DIn = true;
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
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage1.data, BinImage4.data, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, eSelectMix);
		if (nCntBlob > 0)
		{
			dCX = 0, dCY = 0, dArea = 0;
			m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
		}
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage4.bmp", BinImage4);
#endif
		if (bMatchSuccess)
		{
#if _DEBUG
			cv::imwrite("D:\\FootRst\\bMatchSuccess_before.bmp", BinImage4);
#endif
			cv::bitwise_and((*cvFootImg), cvMatchingMask, BinImage4);
#if _DEBUG
			cv::imwrite("D:\\FootRst\\bMatchSuccess_After.bmp", BinImage4);
#endif
		}
		nMinBlob = dArea * 0.1;
		if (100 > nMinBlob) nMinBlob = 100;
		int selectblob = eSelectMix;
		if (!pFoot->m_bUsePatternAngle)
			selectblob = eSelectBigger;
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage1.data, cvFootImg->data, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, eSelectBigger);

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
				if (cvWing->data[nIdx] == 255)
				{
					cvWedgeImg->data[nIdx] = 0;
				}
			}
		}
#if _DEBUG
		cv::imwrite("D:\\FootRst\\Find_Foot.bmp", *cvFootImg);
		cv::imwrite("D:\\FootRst\\Find_Wedge.bmp", *cvWedgeImg);
		cv::imwrite("D:\\FootRst\\Find_Wing.bmp", *cvWing);
#endif
		if (!bMatchSuccess || !pFoot->m_bUsePatternAngle)
		{
			//Foot BW(2D+3D)image

			cv::Point CenterPo(-1, -1);
			cv::Mat Img = (*cvFootImg).clone();
			cv::Mat mMask = (*cvFootImg).clone();
			
			double seta(0.0f);
			cv::Point2d WingLen;
			bool flag = getFootCenterPosAndSeta(CenterPo, seta, Img, mMask, WingLen, nFootDirection);

			*pAngle = seta;
			*pCenter = cv::Point(CenterPo.x, CenterPo.y);
		}

		if (45 >= nFootDirection || nFootDirection > 315)// ^
		{
			if (abs(*pAngle) > 90)
			{
				*pAngle += 180;
			}
		}
		else if (225 >= nFootDirection && nFootDirection > 135)// V
		{
			if (abs(*pAngle) < 90)
			{
				*pAngle += 180;
			}
		}
		else if (135 >= nFootDirection && nFootDirection > 45)// <
		{
			if (abs(*pAngle) > 180)
			{
				*pAngle += 180;
			}
		}
		else if (315 >= nFootDirection && nFootDirection > 225)// >
		{
			if (abs(*pAngle) < 180)
			{
				*pAngle += 180;
			}
		}

		/*
		cv::Mat WingCandi_Second(cvWing->rows, cvWing->cols, CV_8UC1);
		WingCandi_Second.setTo(0);
		WingWedgeImage(pFoot, *pCenter, *pAngle, cvFootImg, cvWedgeImg, cvWing, &cv3DArea, cvPadImg, ucArrDstImg, &WingCandi_Second, nStartX, nStartY, nUIFootBinIDX, bWingRstVisible);
		WingCandi_Second.copyTo(*wingCandiSecond);
		*/

#if _DEBUG
		cv::imwrite("D:\\FootRst\\Cand_Find_Foot.bmp", *cvFootImg);
		cv::imwrite("D:\\FootRst\\Cand_Find_Wedge.bmp", *cvWedgeImg);
		cv::imwrite("D:\\FootRst\\Cand_Find_Wing.bmp", *cvWing);
		cv::imwrite("D:\\FootRst\\Cand_Find_PadImg.bmp", *cvPadImg);
#endif
		Delete_1DArray(&pucImg);
		Delete_1DArray(&pucImg1);
		Delete_1DArray(&pucImg2);
		Delete_1DArray(&pfImg);
		Delete_1DArray(&LabelImage);
		return nCntBlob;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Foot::FindFoot_Pat() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return 0;
}

bool CPInsp_Crescent::FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	//If the Foot cannot be found by pattern matching, the 2D and 3D image is used.
	bool ret = false;
	return ret;
}

cv::Point CPInsp_Crescent::FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin)
{
	cv::Point pt;
	return pt;
}
