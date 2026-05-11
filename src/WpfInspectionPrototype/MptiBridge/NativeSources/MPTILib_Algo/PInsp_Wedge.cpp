#pragma once
#include "stdafx.h"
#include <Windows.h>
#include "Pinsp_Wedge.h"
#include "MPTI.h"


CPInsp_Wedge::CPInsp_Wedge()
{
}
CPInsp_Wedge::~CPInsp_Wedge()
{
}
bool CPInsp_Wedge::FindCandidate_Wedge(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* dst, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg)
{
	bool flag = false;

	CString ImagePath;

	int nLine = __LINE__;
	try
	{
		//Wedge EAº¸±º AßAa - 2D ¹× 3D A¤º¸ CO≫e

		//cv::Mat cvFootImg2(nHeight, nWidth, CV_8UC1);
		//pFoot->GetImageMatrix(&cvFootImg2, (int)m_eFootBin::m_eFootBin_Foot);  //m_eFootBin_Pad

		//1. Foot InspROI AC Top Red A¶¸i¿¡¼­(2D AI¹IAo) ¹aAº ¿μ¿ª 180 AI≫o¸¸ AßAa
		int nImageHeight = pFoot->GetImageLength();
		int nImageWidth = pFoot->GetImageWidth();

		cv::Mat WedgeSrcImg(nImageHeight, nImageWidth, CV_8UC1, pFoot->m_ImageBuffer->imgTop_R);
		cv::Mat srcTopRedImg = WedgeSrcImg;

		cv::Mat Wedge2DCandidates = cv::Mat(srcTopRedImg.rows, srcTopRedImg.cols, CV_8UC1);
		Wedge2DCandidates.setTo(0);

		cv::Mat Foot3DImg = *cv3DImg;
		size_t floatstep_Foot3DCorrect = Foot3DImg.step / sizeof(float);

		for (int r = 0; r < srcTopRedImg.rows; r++)
		{
			UCHAR* Ptr = srcTopRedImg.data;
			UCHAR* Ptr_WedgeCandidates = Wedge2DCandidates.data;
			float* Ptr_Foot3D = (float*)Foot3DImg.data;

			for (int c = 0; c < srcTopRedImg.cols; c++)
			{
				//º¸°￡μE Foot 3d AC ³oAI°¡ 30 AI≫o μC´A ºIºÐ¸¸ AßAa
				if (Ptr_Foot3D[r * floatstep_Foot3DCorrect + c] != 0 &&
					Ptr_Foot3D[r * floatstep_Foot3DCorrect + c] > 30 && Ptr_Foot3D[r * floatstep_Foot3DCorrect + c] < 220)
				{
					//2D Top_Red A¶¸i GrayLevelAI 180 AI≫oAI ºIºÐ¸¸ AßAa
					if (Ptr[r*srcTopRedImg.step + c] > 150)
					{
						Ptr_WedgeCandidates[r*srcTopRedImg.step + c] = 255;
					}
				}
			}

		}
		Wedge2DCandidates &= *cvFootImg;		//2D°a°u ¿μ≫o Foot_BWÆ¼Aª AI¹IAo¿I ¸¶½ºA·

		//2. Foot 3DAC X¹æCa ³oAI º?E­·®(±a¿i±a) 5 AICIAI ºIºÐ¸¸ AßAa
		cv::Mat Foot3D_derivativeX(Foot3DImg.rows, Foot3DImg.cols, CV_16SC1);
		cv::Mat Wedge3DCandidates;

		Foot3D_derivativeX.setTo(0);
		for (int r = 0; r < Foot3D_derivativeX.rows; r++)
		{
			float* fPtrX = Foot3DImg.ptr<float>(r);
			short* fPtrDerivativeX = Foot3D_derivativeX.ptr<short>(r);
			for (int c = 1; c < Foot3D_derivativeX.cols - 1; c++)
			{
				float magX_P1 = abs(fPtrX[c + 1]);
				float magX_M1 = abs(fPtrX[c - 1]);

				float Hdiff = std::abs(magX_P1 - magX_M1);

				if (fPtrX[c] != 0 && fPtrX[c] > 30)
				{
					//XAa ±a¿i±a 5 AICI¸¸
					if (Hdiff < 10)
						fPtrDerivativeX[c] = 255;
				}
			}
		}
		Foot3D_derivativeX.convertTo(Wedge3DCandidates, CV_8UC1);
		Wedge3DCandidates &= *cvFootImg;		//3D °a°u ¿μ≫o Foot_BW Æ¼Aª AI¹IAo¿I ¸¶½ºA·

		//3. 2D + 3D °a°u ºnÆ® AND ¿￢≫e ¼oCa
		cv::Mat Wedge_2D3D_Merge;
		Wedge_2D3D_Merge = Wedge2DCandidates & Wedge3DCandidates;

		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));
		cv::dilate(Wedge_2D3D_Merge, Wedge_2D3D_Merge, kernel);
		cv::erode(Wedge_2D3D_Merge, Wedge_2D3D_Merge, kernel);

		//4. Foot Aß½E¿¡ °¡±i¿i Blob ¼±AA - foot ºIºÐ AI¿U AI¹°AI CO²² AIAøE­ μE °æ¿i¸| ´eºn(μI °³AC FootAI °æ¿iμμ ´eºn)
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(Wedge_2D3D_Merge, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
		std::vector<std::vector<cv::Point> >hull(contours.size());
		for (size_t i = 0; i < contours.size(); i++)
		{
			convexHull(cv::Mat(contours[i]), hull[i], false);
		}

		int idx = 0;
		cv::Mat WedgeRst_Blob(Wedge_2D3D_Merge.size(), CV_8UC1, cv::Scalar(0));
		std::vector<int> bolbCandidate;
		for (contours.begin(); idx < contours.size(); idx++)
		{
			drawContours(WedgeRst_Blob, contours, idx, cv::Scalar(255), cv::FILLED, 8, hierarchy);
		}

		//Blob ¸eAuAI 100AI≫oAI¸e¼­ Foot Aß½E°u °¡Aa °¡±i¿i °A¸®¿¡ A§A¡CN Blob ¸¸ AßAa
		cv::Mat detectedWedgeRst(WedgeRst_Blob.size(), CV_8UC1, cv::Scalar(0));
		int maxIdx = 0;
		bool nIncludeFootCenterPoint = false;
		int nSelectBlobMode = 0; //0: AllBlob, 1:NearestBlob

		if (pFoot->m_bUse2Foot)	nSelectBlobMode = 1;

		int nIncludeFootCtrIndex = 0;
		int NearestCenterPosIdx = 0;
		int NearestMinDistance = WedgeRst_Blob.cols * WedgeRst_Blob.rows;
		for (contours.begin(); maxIdx < contours.size(); maxIdx++)
		{
			cv::Rect bbox = cv::boundingRect(contours[maxIdx]);
			double area = cv::contourArea(contours[maxIdx]);

			if (area > 80)
			{
				for (int r = bbox.y; r < bbox.y + bbox.height; r++)
				{
					UCHAR* Ptr = WedgeRst_Blob.data;
					for (int c = bbox.x; c < bbox.x + bbox.width; c++)
					{
						if (Ptr[r * WedgeRst_Blob.step + c] == 255)
						{
							if ((CenterPo.x == c) && CenterPo.y == r)
							{
								nIncludeFootCtrIndex = maxIdx;
								nIncludeFootCenterPoint = true;
								break;
							}

						}
					}
				}

				if (nSelectBlobMode == 1)
				{
					int blobCenterX = bbox.x + (bbox.width / 2);
					int blobCenterY = bbox.y + (bbox.height / 2);

					int dist = sqrt(pow(blobCenterX - (CenterPo.x), 2) + pow(blobCenterY - CenterPo.y, 2));

					if (NearestMinDistance > dist)
					{
						NearestMinDistance = dist;
						NearestCenterPosIdx = maxIdx;
					}

					drawContours(detectedWedgeRst, contours, maxIdx, cv::Scalar(255), cv::FILLED, 8, hierarchy);
				}
				else if (nSelectBlobMode == 0)
				{
					drawContours(detectedWedgeRst, contours, maxIdx, cv::Scalar(255), cv::FILLED, 8, hierarchy);
				}
			}
			else
			{
				//Wedge AßAa ¿μ¿ªAC ¸eAuAI 100 ¹I¸¸AI °æ¿i CØ´c ¿μ¿ª A|°A
				for (int r = bbox.y; r < bbox.y + bbox.height; r++)
				{
					UCHAR* Ptr = WedgeRst_Blob.data;
					for (int c = bbox.x; c < bbox.x + bbox.width; c++)
					{
						if (Ptr[r * WedgeRst_Blob.step + c] == 255)
						{
							Ptr[r * WedgeRst_Blob.step + c] == 0;
						}
					}
				}
			}
		}

		//if (nIncludeFootCenterPoint)
		//	drawContours(WedgeRst_NearestBlob, contours, nIncludeFootCtrIndex, cv::Scalar(255), cv::FILLED, 8, hierarchy);
		//else
		//	drawContours(WedgeRst_NearestBlob, contours, NearestCenterPosIdx, cv::Scalar(255), cv::FILLED, 8, hierarchy);


		flag = true;

#if _DEBUG
		ImagePath.Format(_T("D:\\FootInspImg\\WedgeCandidates.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), detectedWedgeRst);
#endif

		//Wedge EAº¸±º AßAaμC¾u´AAo Blob °³¼o·I °EAo
// 		double dArea = 0, dCX = 0, dCY = 0;
// 		CRect rcBlob{ 0,0,0,0 };
// 		AlgoBlob sAlgoBlob = pFoot->GetAlgoBlob((int)m_eFootBin_Wedge);
// 
// 		TotalInspExceptArea stTieAreaNULL;
// 		stTieAreaNULL.m_nUsedInspPolygon = 0;
// 		stTieAreaNULL.m_nUsedMaskingValue = 0;
// 		stTieAreaNULL.m_nUsedWndPolygon = 0;
// 		cv::Mat BinImage(WedgeRst_NearestBlob.rows, WedgeRst_NearestBlob.cols, CV_8UC1);
// 		cv::Mat fHRectImg(WedgeRst_NearestBlob.rows, WedgeRst_NearestBlob.cols, CV_32FC1, pFoot->pf3D);
// 		int nMinblobArea = 4;
// 
// 		int nCntBlob = CPInsp::BlobImageStruct(sAlgoBlob, WedgeRst_NearestBlob.data, fHRectImg.ptr<float>(0), NULL, WedgeRst_NearestBlob.cols, WedgeRst_NearestBlob.rows, nMinblobArea, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);

		detectedWedgeRst.copyTo(*dst);

		flag = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::FindCandidate_Wedge() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return flag;
}

bool CPInsp_Wedge::FindCandidate_TailEndPosY(cv::Mat fHRoImg, bool bFindWedgeCandidates, cv::Mat WedgeCandidateImg, cv::Point& CenterPo, cv::Point& FootRoImgMargin, int &TailEndPosY, int &WedgeEndPosY)
{
	bool Ret = false;
	TailEndPosY = 0;
	WedgeEndPosY = fHRoImg.rows;

	int nLine = __LINE__;
	try
	{

		//View 3D height in the center line of the y-axis of the Foot
		cv::Mat Foot3DClipRoImg = fHRoImg;
		cv::Mat FootCenterLineHeight_Y(Foot3DClipRoImg.rows, 1, CV_32FC1);

		size_t floatstep_Foot3DCorrect = Foot3DClipRoImg.step / sizeof(float);
		size_t floatstep_FootCenterLineY = FootCenterLineHeight_Y.step / sizeof(float);
		int Foot3DmaxHeight = 0;

		for (int r = 0; r < Foot3DClipRoImg.cols; r++)
		{
			float* Ptr_Foot3D = (float*)Foot3DClipRoImg.data;
			float* Ptr_FootCenterLineHeightY = (float*)FootCenterLineHeight_Y.data;

			for (int c = 0; c < Foot3DClipRoImg.rows; c++)
			{
				if (r == CenterPo.x - FootRoImgMargin.x)
				{
					Ptr_FootCenterLineHeightY[c * floatstep_FootCenterLineY] = Ptr_Foot3D[c * floatstep_Foot3DCorrect + r];

					if (c > CenterPo.y)
					{
						Ptr_FootCenterLineHeightY[c * floatstep_FootCenterLineY] = 0;
					}

					//Ptr_Foot3D[c * floatstep_Foot3DCorrect + r] = 0;
				}

				//Calc the highest value at the y-axis center line of the Foot
				if (Foot3DmaxHeight < Ptr_Foot3D[c * floatstep_Foot3DCorrect + r])
					Foot3DmaxHeight = Ptr_Foot3D[c * floatstep_Foot3DCorrect + r];
			}
		}


		//
		int ncvImgWidth = WedgeCandidateImg.cols;
		int ncvImgHeight = WedgeCandidateImg.rows;
		int nWedgeTmpMin_x = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_x = 0;
		int nWedgeTmpMin_y = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_y = 0;

		bool bFindWedgeStartPos = false;

		if (bFindWedgeCandidates)
		{
			for (int r = 0; r < WedgeCandidateImg.rows; r++)
			{
				UCHAR* Ptr = WedgeCandidateImg.data;
				for (int c = 0; c < WedgeCandidateImg.cols; c++)
				{
					if (Ptr[r * WedgeCandidateImg.step + c] == 255)
					{
						if (c < nWedgeTmpMin_x) nWedgeTmpMin_x = c;
						if (c > nWedgeTmpMax_x) nWedgeTmpMax_x = c;
						if (r < nWedgeTmpMin_y) nWedgeTmpMin_y = r;
						if (r > nWedgeTmpMax_y) nWedgeTmpMax_y = r;
					}
				}
			}

			//wing °a°u ¾øA≫ ½A A¶°C¹® ºuA®³ª°¨
			if (nWedgeTmpMin_x == ncvImgWidth * ncvImgHeight && nWedgeTmpMin_y == ncvImgWidth * ncvImgHeight &&
				nWedgeTmpMax_x == 0 && nWedgeTmpMax_y == 0)
			{
				bFindWedgeStartPos = false;
			}
			else
				bFindWedgeStartPos = true;
		}

		/// Establish the number of bins
		int histSize = Foot3DClipRoImg.rows;

		/// Set the ranges ( for B,G,R) )
		float range[] = { 0, Foot3DmaxHeight };
		const float* histRange = { range };
		bool uniform = true; bool accumulate = false;

		cv::Mat b_hist = FootCenterLineHeight_Y; // , g_hist, r_hist;

		cv::Size szHistImg(512 * 2 * 0.7, 300 * 2 * 0.5);
		double textScale = 1.0;
		cv::Size divNum(20, 10);
		int guideLineClr = 150;
		int backClr = 1;
		bool bFill = true;

		cv::Mat selectedHist;
		selectedHist = b_hist;

		double maxVal = 0;
		minMaxLoc(selectedHist, 0, &maxVal, 0, 0);	// Get min max value

		// presetting
		cv::Scalar bgClr = CV_RGB(255, 255, 255);
		cv::Scalar bkClr = CV_RGB(0, 0, 0);
		cv::Scalar grClr = CV_RGB(150, 150, 150);
		if (backClr == 1)
		{
			bkClr = CV_RGB(255, 255, 255);
			bgClr = CV_RGB(0, 0, 0);
		}

		cv::Size szTextHor = cv::getTextSize("000", 1, textScale, 1, 0);
		cv::Size szTextMaxVal = cv::getTextSize(to_string((int)maxVal), 1, textScale, 1, 0);

		int marginText = 15;
		int marginBtm = szTextHor.height + marginText * 2;
		int marginRig = 30;
		int marginLef = szTextMaxVal.width + marginText * 2;
		int marginTop = 30;

		int hist_w = szHistImg.width;
		int hist_h = szHistImg.height;

		float bin_w = (float)(hist_w - marginLef - marginRig) / (float)histSize;

		cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));
		cv::Mat histImage_tmp(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));

		// Draw frame
		cv::arrowedLine(histImage, cv::Point(marginLef, hist_h - marginBtm), cv::Point(hist_w - marginRig * 0.5, hist_h - marginBtm), cv::Scalar(0, 255, 0), 2, 8, 0, 0.008);	// bottom line
		cv::arrowedLine(histImage, cv::Point(marginLef, hist_h - marginBtm), cv::Point(marginLef, marginTop*0.5), cv::Scalar(0, 255, 0), 2, 8, 0, 0.015);	// left

		// Draw horizontal value
		int repValNumHori = divNum.width;
		int interHori = double(hist_w - marginLef - marginRig) / repValNumHori;
		float interValHori = (float)(histSize - 1) / (float)repValNumHori;
		for (int ii = 0; ii < repValNumHori + 1; ii++) {
			string textHori = to_string((int)(interValHori*ii));
			cv::Size szTextHori = cv::getTextSize(textHori, 1, textScale, 1, 0);
			putText(histImage, textHori, cv::Point(marginLef + interHori * ii - szTextHori.width*0.5, hist_h - marginBtm + szTextHor.height + marginText), 1, textScale, bkClr, 1, 8, false);
		}

		// Draw vertical value
		int repValNumVert = divNum.height;
		int interVert = double(hist_h - marginTop - marginBtm) / repValNumVert;
		float interValVert = (float)(maxVal) / (float)repValNumVert;
		for (int ii = 0; ii < repValNumVert; ii++) {
			string textVer = to_string((int)(interValVert*(repValNumVert - ii)));
			cv::Size szTextVer = cv::getTextSize(textVer, 1, textScale, 1, 0);
			int rightAlig = szTextMaxVal.width - szTextVer.width;
			putText(histImage, textVer, cv::Point(marginLef - szTextMaxVal.width + rightAlig - marginText, marginTop + interVert * ii + szTextVer.height*0.5), 1, textScale, bkClr, 1, 8, false);
		}

		// Draw horizontal/vertical guide line
		for (int ii = 1; ii <= repValNumHori; ii++) {
			cv::line(histImage, cv::Point(marginLef + interHori * ii, marginTop*0.5), cv::Point(marginLef + interHori * ii, hist_h - marginBtm), grClr, 1, 8, 0);
			cv::line(histImage, cv::Point(marginLef + interHori * ii, hist_h - marginBtm), cv::Point(marginLef + interHori * ii, hist_h - marginBtm + 10), bkClr, 1, 8, 0);
		}
		for (int ii = 0; ii < repValNumVert; ii++) {
			cv::line(histImage, cv::Point(marginLef - 10, marginTop + interVert * ii), cv::Point(hist_w - marginRig, marginTop + interVert * ii), grClr, 1, 8, 0);
			cv::line(histImage, cv::Point(marginLef - 10, marginTop + interVert * ii), cv::Point(marginLef, marginTop + interVert * ii), bkClr, 1, 8, 0);
		}

		/// Normalize the result to [ 0, histImage.rows-margin of top & bottom ]
		normalize(selectedHist, selectedHist, 0, histImage.rows - marginTop - marginBtm, cv::NORM_MINMAX, -1, cv::Mat());

		/// Draw for each channel
		if (!bFill) {
			for (int i = 1; i < histSize; i++) {
				cv::Point prePtR(marginLef + (bin_w*(i - 1)), hist_h - marginBtm - cvRound(selectedHist.at<float>(i - 1)));
				cv::Point postPtR(marginLef + (bin_w*(i)), hist_h - marginBtm - cvRound(selectedHist.at<float>(i)));
				cv::line(histImage, prePtR, postPtR, cv::Scalar(255, 0, 0), 2, 8, 0);
				//line(histImage_tmp, prePtR, postPtR, Scalar(255, 0, 0), 2, 8, 0);
			}
		}
		else if (bFill) {
			for (int i = 1; i < histSize; i++) {
				cv::Point prePtR(marginLef + (bin_w*(i - 1)), hist_h - marginBtm);
				cv::Point postPtR(marginLef + (bin_w*(i)), hist_h - marginBtm - cvRound(selectedHist.at<float>(i)));
				rectangle(histImage, prePtR, postPtR, cv::Scalar(255, 0, 0), -1, 8, 0);
				//rectangle(histImage_tmp, prePtR, postPtR, Scalar(255, 0, 0), -1, 8, 0);
			}
		}


		for (int i = 1; i < histSize; i++) {
			if (cvRound(selectedHist.at<float>(i)) != 0)
			{
				cv::Point prePtR(marginLef + (bin_w*(i - 1)), hist_h - marginBtm - cvRound(selectedHist.at<float>(i - 1)));
				cv::Point postPtR(marginLef + (bin_w*(i)), hist_h - marginBtm - cvRound(selectedHist.at<float>(i)));
				line(histImage_tmp, prePtR, postPtR, cv::Scalar(255, 0, 0), 2, 8, 0);
			}
		}

		//Hough Lines
		cv::Mat LineTransformImg = histImage_tmp;

		cv::Mat contours;
		cv::Canny(LineTransformImg, contours, 125, 350);

		std::vector<cv::Vec4i> lines;
		lines.clear();
		double deltaRho = 1.0;
		double deltaTheta = PI / 180;	//Accumulator Resolution Parameter
		int minVote = 10;				//Minimum number of votes
		double minLength = 0.;			//Minimum length
		double maxGap = 0.;

		//Parameter setting
		minVote = 10;
		minLength = 40;
		maxGap = 20;
		cv::HoughLinesP(contours, lines, deltaRho, deltaTheta, minVote, minLength, maxGap);

		std::vector<cv::Vec4i> li = lines;

		//Draw Detected Lines
		std::vector<cv::Vec4i>::const_iterator it2 = lines.begin();
		cv::Scalar color = cv::Scalar(255, 255, 255);

		int LineCnt = 0;
		int slope = 0;
		int maxSlopeVal = 0;
		int minX = histImage_tmp.cols - 1;
		while (it2 != lines.end())
		{
			cv::Point pt1((*it2)[0], (*it2)[1]);
			cv::Point pt2((*it2)[2], (*it2)[3]);

			//Calc Line Slope
			if (!(pt1.x == pt2.x) && !(pt1.y == pt2.y))
			{
				slope = (int)RounD((pt1.y - pt2.y) / (pt1.x - pt2.x));
			}
			else if (!(pt1.x == pt2.x) && (pt1.y == pt2.y))
			{
				slope = 0;
			}
			else
				slope = 100;

			if (maxSlopeVal < std::abs(slope))
				maxSlopeVal = std::abs(slope);

			//Set the value of the line slope condition of the Wedge to 3 or less
			if (std::abs(slope) < 3)
			{
				cv::line(LineTransformImg, pt1, pt2, color);
				if (minX > pt1.x)
					minX = pt1.x;
				if (minX > pt2.x)
					minX = pt2.x;

				LineCnt++;
			}

			++it2;
		}

		int nTailEndPosX = 0;
		if (minX != histImage_tmp.cols - 1)
			nTailEndPosX = (int)RounD((float)(minX - marginLef) / (float)(bin_w));

		TailEndPosY = nTailEndPosX;

		if (bFindWedgeStartPos)
		{
			//if (nWedgeTmpMin_y > TailEndPosY)
			//	TailEndPosY = nWedgeTmpMin_y;

			WedgeEndPosY = nWedgeTmpMax_y;
		}

		//Display Line Profile 
		//cv::namedWindow("Detected Lines with HoughP");
		//cv::imshow("Detected Lines with HoughP", LineTransformImg);
		//cv::waitKey(0);

		//namedWindow("Histogram Foot", CV_WINDOW_AUTOSIZE);
		//imshow("Histogram Foot", histImage);

		if (LineCnt > 0)
			Ret = true;
		else
			Ret = false;
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::FindCandidate_TailEndPosY() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}
bool CPInsp_Wedge::FindCandidate_WingStartAndWingEnd(cv::Mat* vImage, cv::Point& CenterPo, cv::Point& FootRoImgMargin, double dWingLengthMinimumMM, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, cv::Mat Img3D, bool bFindHill, bool bFindCandidateWedge, cv::Mat* cvWedgeCandidates)
{
	bool Ret = false;

	int nLine = __LINE__;
	try
	{
		//Wing LengthAC AO¼O °ª
		int dWingLenMinPixel = (int)(dWingLengthMinimumMM / m_resolX);
		int dWingHalfLenMinPixel = (int)(dWingLenMinPixel / 2);

		int AvgW(0);
		std::vector<POINT> vst;
		std::vector<POINT> ved;

		cv::Mat Intg, Dev;
		cv::Mat Img = (*vImage).clone();
		Img &= 1;
		cv::integral(Img, Intg, Dev);

		float fValue = Intg.cols / 5;
		for (int r = 1; r < Intg.rows - 1; r++)
		{
			int* intPtr_1 = Intg.ptr<int>(r - 1);
			int* intPtr = Intg.ptr<int>(r);
			UCHAR* ImgPtr = Img.ptr(r);
			if (intPtr[Intg.cols - 1] - intPtr_1[Intg.cols - 1] < 1)
				continue;

			POINT st, ed;
			st.x = 0;
			st.y = 0;
			ed.x = 0;
			ed.y = 0;
			for (int c = 0; c < Img.cols - 1; c++)
			{
				if (ImgPtr[c] > 0)
				{
					st.x = c - 2;
					st.y = r;
					break;
				}
			}
			for (int c = Img.cols - 1; c > 0; c--)
			{
				if (ImgPtr[c] > 0)
				{
					ed.x = c;
					ed.y = r;
					break;
				}
			}
			vst.push_back(st);
			ved.push_back(ed);
		}
		int nSum(0);

		std::vector<int> vdiff;
		int maxDiff = 0;

		for (int i = 0; i < vst.size(); i++)
		{
			nSum += (ved[i].x - vst[i].x);
			vdiff.push_back((ved[i].x - vst[i].x));

			if (maxDiff < (ved[i].x - vst[i].x))
				maxDiff = (ved[i].x - vst[i].x);
		}

		int histSize = vdiff.size();
		float range[] = { 0, 35 };
		const float* histRange = { range };
		bool uniform = true; bool accumulate = false;

		////
		cv::Mat FootWidthList_Y(vst.size(), 1, CV_8UC1);

		for (int r = 0; r < FootWidthList_Y.cols; r++)
		{
			UCHAR* Ptr_FootWidthList = FootWidthList_Y.data;

			for (int c = 0; c < FootWidthList_Y.rows; c++)
			{
				Ptr_FootWidthList[c * FootWidthList_Y.step + r] = vdiff[c];
			}

		}

		cv::Mat b_hist = FootWidthList_Y.clone();


		cv::Size szHistImg(512 * 2 * 0.7, 300 * 2 * 0.5);
		double textScale = 1.0;
		cv::Size divNum(20, 10);
		int guideLineClr = 150;
		int backClr = 1;
		bool bFill = true;

		cv::Mat selectedHist;
		selectedHist = b_hist.clone();

		double maxVal = 0;
		cv::minMaxLoc(selectedHist, 0, &maxVal, 0, 0);	// Get min max value

		// presetting
// 		Scalar bgClr = CV_RGB(255, 255, 255);
// 		Scalar bkClr = CV_RGB(0, 0, 0);
// 		Scalar grClr = CV_RGB(150, 150, 150);
// 		if (backClr == 1)
// 		{
// 			bkClr = CV_RGB(255, 255, 255);
// 			bgClr = CV_RGB(0, 0, 0);
// 		}

// 		cv::Size szTextHor = getTextSize("000", 1, textScale, 1, 0);
// 		cv::Size szTextMaxVal = getTextSize(to_string((int)maxVal), 1, textScale, 1, 0);
// 
// 		int marginText = 15;
// 		int marginBtm = szTextHor.height + marginText * 2;
// 		int marginRig = 30;
// 		int marginLef = szTextMaxVal.width + marginText * 2;
// 		int marginTop = 30;
// 
// 		int hist_w = szHistImg.width;
// 		int hist_h = szHistImg.height;
// 
// 		float bin_w = (float)(hist_w - marginLef - marginRig) / (float)histSize;

// 		Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
// 		Mat histImage_tmp(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
// 
// 		// Draw frame
// 		arrowedLine(histImage, Point(marginLef, hist_h - marginBtm), Point(hist_w - marginRig * 0.5, hist_h - marginBtm), Scalar(0, 255, 0), 2, 8, 0, 0.008);	// bottom line
// 		arrowedLine(histImage, Point(marginLef, hist_h - marginBtm), Point(marginLef, marginTop*0.5), Scalar(0, 255, 0), 2, 8, 0, 0.015);	// left
// 
// 		// Draw horizontal value
// 		int repValNumHori = divNum.width;
// 		int interHori = double(hist_w - marginLef - marginRig) / repValNumHori;
// 		float interValHori = (float)(histSize - 1) / (float)repValNumHori;
// 		for (int ii = 0; ii < repValNumHori + 1; ii++) {
// 			string textHori = to_string((int)(interValHori*ii));
// 			cv::Size szTextHori = getTextSize(textHori, 1, textScale, 1, 0);
// 			putText(histImage, textHori, Point(marginLef + interHori * ii - szTextHori.width*0.5, hist_h - marginBtm + szTextHor.height + marginText), 1, textScale, bkClr, 1, 8, false);
// 		}
// 
// 		// Draw vertical value
// 		int repValNumVert = divNum.height;
// 		int interVert = double(hist_h - marginTop - marginBtm) / repValNumVert;
// 		float interValVert = (float)(maxVal) / (float)repValNumVert;
// 		for (int ii = 0; ii < repValNumVert; ii++) {
// 			string textVer = to_string((int)(interValVert*(repValNumVert - ii)));
// 			cv::Size szTextVer = getTextSize(textVer, 1, textScale, 1, 0);
// 			int rightAlig = szTextMaxVal.width - szTextVer.width;
// 			putText(histImage, textVer, Point(marginLef - szTextMaxVal.width + rightAlig - marginText, marginTop + interVert * ii + szTextVer.height*0.5), 1, textScale, bkClr, 1, 8, false);
// 		}
// 
// 		// Draw horizontal/vertical guide line
// 		for (int ii = 1; ii <= repValNumHori; ii++) {
// 			line(histImage, Point(marginLef + interHori * ii, marginTop*0.5), Point(marginLef + interHori * ii, hist_h - marginBtm), grClr, 1, 8, 0);
// 			line(histImage, Point(marginLef + interHori * ii, hist_h - marginBtm), Point(marginLef + interHori * ii, hist_h - marginBtm + 10), bkClr, 1, 8, 0);
// 		}
// 		for (int ii = 0; ii < repValNumVert; ii++) {
// 			line(histImage, Point(marginLef - 10, marginTop + interVert * ii), Point(hist_w - marginRig, marginTop + interVert * ii), grClr, 1, 8, 0);
// 			line(histImage, Point(marginLef - 10, marginTop + interVert * ii), Point(marginLef, marginTop + interVert * ii), bkClr, 1, 8, 0);
// 		}


		// °A¸® °i¼±AC 7A÷ ¹æA¤½A Fitting ¼oCa
		cv::Mat selectedHist_LMS(selectedHist.rows, 1, CV_32FC1);
		selectedHist_LMS.setTo(0);

		jsl::LeastSquare lsCalc;
		std::vector< jsl::Point2d<double> > dataList;

		for (int x = 0; x < selectedHist.rows; x++)
		{
			dataList.emplace_back((double)x, (double)selectedHist.data[x]);
		}

		if (dataList.size() == 0)
		{
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("FindCandidate_WingStartAndWingEnd dataList.size() == 0"));
			g_pMPTI->AddLog_OCR(sLog);
			cv::imwrite("D:\\FootRst\\FindCandidate_WingStartAndWingEnd_vImage.bmp", *vImage);
#endif
			return false;
		}

		// X¿¡ ´eCN 7A÷ ¹æAa½A °e¼o
		bool b2DInterpol = false;
		std::vector<double> coeff; // Aß¼¼¼±

		if (dataList.size() > 30)					//C￥º≫AI 30°³ AI≫oAI ¶§
			lsCalc.RunSvd(7, dataList, coeff);		//7A÷ º¸°￡ ¼oCa

		//if (coeff.size() == 8)
		b2DInterpol = true;

		if (coeff.size() == 0)
		{
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("FindCandidate_WingStartAndWingEnd coeff.size() == 0"));
			g_pMPTI->AddLog_OCR(sLog);
			cv::imwrite("D:\\FootRst\\FindCandidate_WingStartAndWingEnd_vImage.bmp", *vImage);
#endif
			return false;
		}

		//μμCO¼o °e≫e differential coefficient
		std::vector<double> coeff_differential;
		if (b2DInterpol)
		{
			for (size_t i = 0; i < coeff.size() - 1; i++)
			{
				int nOrder = coeff.size() - 1 - i;
				double Coeff_diff = coeff[i] * nOrder;
				coeff_differential.push_back(Coeff_diff);
			}
		}

		std::vector<int> inflectionPointY;				//7A÷ º¸°￡ °i¼±AC º?°iA¡ AuAa 
		std::vector<float> curveFittingSlopePointY;		//º¸°￡μE 7A÷ °i¼±AC ±a¿i±a AuAa

		float pfy = 0;
		for (int x = 0; x < selectedHist.rows; x++)
		{
			float fy = 0.0;
			fy = lsCalc.ApplySvd(x, coeff);

			//if (fy < 0) continue;
			pfy = fy;

			if (true)
			{
				float* fPtr_data = selectedHist_LMS.ptr<float>(x);

				//±a¿i±a °e≫e
				float fslope = lsCalc.ApplySvd(x, coeff_differential);
				curveFittingSlopePointY.push_back(fslope);

				for (int y = 0; y < selectedHist.cols; y++)
				{
					fPtr_data[y] = fy;
				}
			}
		}

		std::vector<int> curveFittingSlope_Pos;
		std::vector<int> curveFittingSlope_Neg;

		cv::Mat Foot_CandidatePoint = (*vImage).clone();
		cv::Point pointInflection;				//7A÷ °i¼± º¸°￡ °a°u·I ¾oAº º?°iA¡ AAC￥
		cv::Point pointCurveFittingSlope;		//º¸°￡μE 7A÷ °i¼± ±a¿i±a A¶°C¿¡ ¸¸A·CI´A AAC￥

		bool bFlatSlope = false;
		bool bFlagSlopeUp = false;
		bool bFlagSlopeDown = false;
		int nSlope_Pos_Cnt = 0;
		int nSlope_Neg_Cnt = 0;

		for (int x = 1; x < selectedHist.rows - 1; x++)
		{
			float* fPtr_n = selectedHist_LMS.ptr<float>((x)-1);
			float* fPtr = selectedHist_LMS.ptr<float>((x));
			float* fPtr_p = selectedHist_LMS.ptr<float>((x)+1);

			for (int k = 0; k < selectedHist.cols; k++)
			{
				//CoAc·IºIAI ¾O, μUAC CE¼¿ °ª ºn±³ EA º?°iA¡ ÆC´U
				if ((fPtr[k] < fPtr_n[k] && fPtr[k] < fPtr_p[k]) ||
					(fPtr[k] > fPtr_n[k] && fPtr[k] > fPtr_p[k]))
				{
					inflectionPointY.push_back(vst[0].y + x);
					pointInflection.x = Img.cols / 2;
					pointInflection.y = vst[0].y + x;
					cv::line(Foot_CandidatePoint, pointInflection, pointInflection, cv::Scalar(128), 2, 5);
				}
			}
		}

		std::vector<int> slopeUpStartPoint;
		std::vector<int> slopeUpEndPoint;
		std::vector<int> slopeDownStartPoint;
		std::vector<int> slopeDownEndPoint;
		int cnt = 0;
		for (int x = 0; x < selectedHist.rows; x++)
		{
			for (int k = 0; k < selectedHist.cols; k++)
			{
				if (abs(curveFittingSlopePointY[x]) < 0.1)
				{
					pointCurveFittingSlope.x = Img.cols / 2 - 10;
					pointCurveFittingSlope.y = vst[0].y + x;
					cv::line(Foot_CandidatePoint, pointCurveFittingSlope, pointCurveFittingSlope, cv::Scalar(32), 2, 5);

					if (bFlagSlopeUp && nSlope_Neg_Cnt == 0)
					{
						if (cnt == 0)
						{
							if (nSlope_Pos_Cnt != 0)
								curveFittingSlope_Pos.push_back(nSlope_Pos_Cnt);
							slopeUpEndPoint.push_back(vst[0].y + x);
						}

						nSlope_Pos_Cnt = 0;
						bFlatSlope = true;
						bFlagSlopeUp = false;

					}

					if (bFlagSlopeDown && nSlope_Pos_Cnt == 0)
					{
						if (cnt == 0)
						{
							if (nSlope_Neg_Cnt != 0)
								curveFittingSlope_Neg.push_back(nSlope_Neg_Cnt);
							slopeDownEndPoint.push_back(vst[0].y + x);
						}

						nSlope_Neg_Cnt = 0;
						bFlatSlope = true;
						bFlagSlopeDown = false;

					}

					cnt++;

				}
				else if (curveFittingSlopePointY[x] < 0)	//°A¸®°i¼± ±a¿i±a°¡ A½¼oAI °æ¿i
				{
					bFlatSlope = false;
					pointCurveFittingSlope.x = Img.cols / 2 - 20;
					pointCurveFittingSlope.y = vst[0].y + x;
					cv::line(Foot_CandidatePoint, pointCurveFittingSlope, pointCurveFittingSlope, cv::Scalar(128), 2, 5);

					if (bFlagSlopeUp)
					{
						bFlagSlopeUp = false;
						curveFittingSlope_Pos.push_back(nSlope_Pos_Cnt);
						slopeUpEndPoint.push_back(vst[0].y + x - 1);
						nSlope_Pos_Cnt = 0;
						cnt = 0;
					}

					nSlope_Neg_Cnt++;
					bFlagSlopeDown = true;

					if (nSlope_Neg_Cnt == 1)
					{
						slopeDownStartPoint.push_back(vst[0].y + x);
						cnt = 0;
					}

					if (x == selectedHist.rows - 1)
					{
						curveFittingSlope_Neg.push_back(nSlope_Neg_Cnt);
						slopeDownEndPoint.push_back(vst[0].y + x);
					}
				}
				else if (curveFittingSlopePointY[x] > 0)	//°A¸® °i¼± ±a¿i±a°¡ ¾c¼oAI °æ¿i
				{
					bFlatSlope = false;
					pointCurveFittingSlope.x = Img.cols / 2 - 5;
					pointCurveFittingSlope.y = vst[0].y + x;
					cv::line(Foot_CandidatePoint, pointCurveFittingSlope, pointCurveFittingSlope, cv::Scalar(64), 2, 5);

					if (bFlagSlopeDown)
					{
						bFlagSlopeDown = false;
						curveFittingSlope_Neg.push_back(nSlope_Neg_Cnt);
						slopeDownEndPoint.push_back(vst[0].y + x - 1);
						nSlope_Neg_Cnt = 0;
						cnt = 0;
					}
					nSlope_Pos_Cnt++;
					bFlagSlopeUp = true;

					if (nSlope_Pos_Cnt == 1)
					{
						slopeUpStartPoint.push_back(vst[0].y + x);
						cnt = 0;
					}

					if (x == selectedHist.rows - 1)
					{
						curveFittingSlope_Pos.push_back(nSlope_Pos_Cnt);
						slopeUpEndPoint.push_back(vst[0].y + x);
					}
				}


			}
		}

		//°A¸® °i¼±AC ±a¿i±a°¡ ≫o½ACI´A ºoμμ¼o Aß AO´e ºoμμ¼o¸| °¡Aø AoA¡ AßAa
		int maxSlopeUpFrequency = 0;
		int maxSlopeUpIndex = 0;
		int maxSlopeDownFrequency = 0;
		int maxSlopeDownIndex = 0;
		for (int i = 0; i < curveFittingSlope_Pos.size(); i++)
		{
			if (maxSlopeUpFrequency < curveFittingSlope_Pos[i])
			{
				maxSlopeUpFrequency = curveFittingSlope_Pos[i];
			}
		}
		for (int i = 0; i < curveFittingSlope_Pos.size(); i++)
		{
			if (curveFittingSlope_Pos[i] == maxSlopeUpFrequency)
				maxSlopeUpIndex = i;
		}

		//°A¸® °i¼±AC ±a¿i±a°¡ CI°­CI´A ºoμμ¼o Aß AO´e ºoμμ¼o¸| °¡Aø AoA¡ AßAa
		for (int i = 0; i < curveFittingSlope_Neg.size(); i++)
		{
			if (maxSlopeDownFrequency < curveFittingSlope_Neg[i])
			{
				maxSlopeDownFrequency = curveFittingSlope_Neg[i];
			}
		}
		for (int i = 0; i < curveFittingSlope_Neg.size(); i++)
		{
			if (curveFittingSlope_Neg[i] == maxSlopeDownFrequency)
				maxSlopeDownIndex = i;
		}

		if (curveFittingSlope_Pos.size() != slopeUpStartPoint.size())
			return false;

		if (curveFittingSlope_Neg.size() != slopeDownEndPoint.size())
			return false;

		//≫o½A °i¼±°u CI°­ °i¼±AC ºoμμ°¡ AoA¤ A¶°C(Wing AO¼O ±æAIAC Ay¹Y)¿¡ ¸¸A·CIAo ¾EA≫ °æ¿i ¸®AI
		if (maxSlopeUpFrequency < dWingHalfLenMinPixel || maxSlopeDownFrequency < dWingHalfLenMinPixel)
		{
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("FindCandidate_WingStartAndWingEnd maxSlopeUpFrequency < dWingHalfLenMinPixel || maxSlopeDownFrequency < dWingHalfLenMinPixel"));
			g_pMPTI->AddLog_OCR(sLog);
			cv::imwrite("D:\\FootRst\\FindCandidate_WingStartAndWingEnd_vImage.bmp", *vImage);
#endif
			return false;
		}

		bool bFindSlopeUp = false;

		int WedgeCandiCenterY = Img.rows;
		if (bFindCandidateWedge)
		{
			cv::Mat ImgWedgeCandidates = cvWedgeCandidates->clone();

#if _DEBUG
			cv::imwrite("D:\\\FootInspImg\\FindCandidate_Wedge_OutPUT_Img.bmp", ImgWedgeCandidates);
#endif

			//min max ±¸CI°i wedge Aß½EA¡ AAC￥ ±¸CI±a
			int ncvImgWidth = ImgWedgeCandidates.cols;
			int ncvImgHeight = ImgWedgeCandidates.rows;

			int nWedgeTmpMin_x = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_x = 0;
			int nWedgeTmpMin_y = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_y = 0;

			for (int r = 0; r < ImgWedgeCandidates.rows; r++)
			{
				UCHAR* Ptr = ImgWedgeCandidates.data;
				for (int c = 0; c < ImgWedgeCandidates.cols; c++)
				{
					if (Ptr[r * ImgWedgeCandidates.step + c] == 255)
					{
						if (c < nWedgeTmpMin_x) nWedgeTmpMin_x = c;
						if (c > nWedgeTmpMax_x) nWedgeTmpMax_x = c;
						if (r < nWedgeTmpMin_y) nWedgeTmpMin_y = r;
						if (r > nWedgeTmpMax_y) nWedgeTmpMax_y = r;
					}
				}
			}

			// Wedge min max A¤º¸ AOA≫ °æ¿i¸¸ ¼oCa
			if (nWedgeTmpMin_x == ncvImgWidth * ncvImgHeight  &&  nWedgeTmpMax_x == 0 &&
				nWedgeTmpMin_y == ncvImgWidth * ncvImgHeight  &&  nWedgeTmpMax_y == 0)
			{
#if FOOT_RST_WRITE
				CString sLog;
				sLog.Format(_T("FindCandidate_WingStartAndWingEnd nWedgeTmpMin_x == ncvImgWidth * ncvImgHeight  &&  nWedgeTmpMax_x == 0 && nWedgeTmpMin_y == ncvImgWidth * ncvImgHeight  &&  nWedgeTmpMax_y == 0"));
				g_pMPTI->AddLog_OCR(sLog);
				cv::imwrite("D:\\FootRst\\FindCandidate_WingStartAndWingEnd_ImgWedgeCandidates.bmp", ImgWedgeCandidates);
#endif
				return false;
			}
			else
			{
				int WedgeCandiLength = 0;

				WedgeCandiLength = nWedgeTmpMax_y - nWedgeTmpMin_y;
				WedgeCandiCenterY = nWedgeTmpMin_y + (WedgeCandiLength / 3);
			}
		}

		//≫o½A°i¼± ½AAUA¡°u CI°­°i¼± ³¡A¡AI WingStart¿I WingEnd°¡ ¸A´AAo °EAo (Foot Aß½EA¡A¸·I AO½A °EAo)
		if (vst[0].y < slopeUpStartPoint[maxSlopeUpIndex] &&
			(CenterPo.y - FootRoImgMargin.y)> slopeUpStartPoint[maxSlopeUpIndex] &&
			WedgeCandiCenterY > slopeUpStartPoint[maxSlopeUpIndex]
			)
		{
			bool FindWingLeftStart = false;
			bool FindWingRightStart = false;
			int nDetectLeftFirstCnt = 0, nDetectRightFirstCnt = 0;

			//Wing Start °ª AuAa 
			for (int r = 0; r < (*vImage).rows; r++)
			{
				UCHAR* Ptr = (*vImage).data;
				for (int c = 0; c < (*vImage).cols; c++)
				{
					if (r == slopeUpStartPoint[maxSlopeUpIndex])
					{
						if (Ptr[r * (*vImage).step + c] == 255)	//Wing Left Start 
						{
							FindWingLeftStart = true;
							if (nDetectLeftFirstCnt == 0)
							{
								pLSt->x = c;
								pLSt->y = r;
							}
							nDetectLeftFirstCnt++;

						}
					}

				}

				for (int c = (*vImage).cols - 1; c >= 0; c--)
				{
					if (r == slopeUpStartPoint[maxSlopeUpIndex])
					{
						if (Ptr[r * (*vImage).step + c] == 255)	//Wing Right Start 
						{
							FindWingRightStart = true;
							if (nDetectRightFirstCnt == 0)
							{
								pRSt->x = c;
								pRSt->y = r;
								break;
							}
							nDetectRightFirstCnt++;

						}
					}

				}
			}

			bFindSlopeUp = true;
		}
		else
		{
			bFindSlopeUp = false;
		}

		bool bFindSlopeDown = false;

		if ((CenterPo.y - FootRoImgMargin.y) < slopeDownEndPoint[maxSlopeDownIndex] && (*vImage).rows > slopeDownEndPoint[maxSlopeDownIndex])
		{
			bool FindWingLeftEnd = false;
			bool FindWingRightEnd = false;

			int nDetectLeftEndCnt = 0, nDetectRightEndCnt = 0;

			//Wing End °ª AuAa 
			for (int r = 0; r < (*vImage).rows; r++)
			{
				UCHAR* Ptr = (*vImage).data;
				for (int c = 0; c < (*vImage).cols; c++)
				{
					if (r == slopeDownEndPoint[maxSlopeDownIndex])
					{
						if (Ptr[r * (*vImage).step + c] == 255)	//Wing Left End 
						{
							FindWingLeftEnd = true;
							if (nDetectLeftEndCnt == 0)
							{
								pLEd->x = c;
								pLEd->y = r;
							}
							nDetectLeftEndCnt++;

						}
					}

				}

				for (int c = (*vImage).cols; c > 0; c--)
				{
					if (r == slopeDownEndPoint[maxSlopeDownIndex])
					{
						if (Ptr[r * (*vImage).step + c] == 255)	//Wing Right End 
						{
							FindWingRightEnd = true;
							if (nDetectRightEndCnt == 0)
							{
								pREd->x = c;
								pREd->y = r;
								break;
							}
							nDetectRightEndCnt++;

						}
					}

				}
			}

			bFindSlopeDown = true;
		}
		else
		{
			bFindSlopeDown = false;
		}

		if (pLSt->y == 0 || pRSt->y == 0)
		{

#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("FindCandidate_WingStartAndWingEnd if (pLSt->y == 0 || pRSt->y == 0) "));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return false;
		}

		if (pLEd->y == 0 || pREd->y == 0)
		{
#if FOOT_RST_WRITE

			CString sLog;
			sLog.Format(_T("FindCandidate_WingStartAndWingEnd if (pLEd->y == 0 || pREd->y == 0) "));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return false;
		}

		/*cv::Mat selectedHist_LMS_BeforeInterpol = selectedHist_LMS.clone();*/

// 		if (!bFill)
// 		{
// 			for (int i = 1; i < histSize; i++)
// 			{
// 				Point prePtR(marginLef + (bin_w*(i - 1)), hist_h - marginBtm - cvRound(selectedHist.at<int>(i - 1)));
// 				Point postPtR(marginLef + (bin_w*(i)), hist_h - marginBtm - cvRound(selectedHist.at<int>(i)));
// 				line(histImage, prePtR, postPtR, Scalar(255, 0, 0), 2, 8, 0);
// 				//line(histImage_tmp, prePtR, postPtR, Scalar(255, 0, 0), 2, 8, 0);
// 			}
// 		}
// 		else if (bFill)
// 		{
// 			for (int i = 1; i < histSize; i++)
// 			{
// 				Point prePtR(marginLef + (bin_w*(i - 1)), hist_h - marginBtm);
// 				Point postPtR(marginLef + (bin_w*(i)), hist_h - marginBtm - selectedHist.data[i]);
// 				rectangle(histImage, prePtR, postPtR, Scalar(255, 0, 0), -1, 8, 0);
// 				//rectangle(histImage_tmp, prePtR, postPtR, Scalar(255, 0, 0), -1, 8, 0);
// 			}
// 		}

// 		for (int i = 1; i < histSize; i++)
// 		{
// 			float* fPtr = selectedHist_LMS.ptr<float>(i);
// 			for (int y = 0; y < selectedHist.cols; y++)
// 			{
// 				Point prePtR(marginLef + (bin_w*(i - 1)), hist_h - marginBtm - (int)fPtr[y - 1]);
// 				Point postPtR(marginLef + (bin_w*(i)), hist_h - marginBtm - (int)fPtr[y]);
// 				line(histImage_tmp, prePtR, postPtR, Scalar(255, 0, 0), 2, 8, 0);
// 
// 			}
// 		}

		//namedWindow("Foot diff Histogram", CV_WINDOW_AUTOSIZE);
		//imshow("Foot diff Histogram", histImage);

		//namedWindow("Histogram LeastMeanSquare", CV_WINDOW_AUTOSIZE);
		//imshow("Histogram LeastMeanSquare", histImage_tmp);

		if (bFindSlopeUp && bFindSlopeDown)
			Ret = true;
		else
			Ret = false;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::FindCandidate_WingStartAndWingEnd() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}

bool CPInsp_Wedge::FindCandidate_Wing(CFoot_Model* pFoot, cv::Mat Img3D, cv::Mat BinWing, cv::Mat* dst, cv::Point& CenterPo)
{
	bool flag = false;

	int nLine = __LINE__;
	try
	{

#if _DEBUG
		cv::imwrite("D:\\FootInspImg\\FindCandidate_Wing_INPUT_Img.bmp", BinWing);
#endif

		//Detect Melting Area Using 3D Height
		cv::Mat Foot2D_img = BinWing.clone();
		cv::Mat Img3D_Candidates = Img3D.clone();

		//Ee·?³≫¸° ¿μ¿ª A|°A
		for (int y = 0; y < Img3D_Candidates.rows; y++)
		{
			float* ptrInputImg = Img3D_Candidates.ptr<float>(y);
			UCHAR* ptrFootBW = Foot2D_img.ptr(y);
			UCHAR* ptrFootBW_Rst = Foot2D_img.ptr(y);

			for (int x = 0; x < Img3D_Candidates.cols; x++)
			{
				if (ptrFootBW[x] == 0)
				{
					ptrInputImg[x] = 0;
				}

				if (ptrInputImg[x] < 40)
				{
					ptrInputImg[x] = 0;
					ptrFootBW_Rst[x] = 0;
				}

			}
		}

		//Wing BW Blob 2°³ AI≫oAI¸e, Max Blob 2°³¸¸ ≫Iμμ·I Aß°¡ (Wing ¿·A¸·I Ee·?³≫¸° ¿μ¿ªAI BlobA¸·I AaE÷´A °æ¿i°¡ AO¾i °E≫c¿¡ AoAa)
		double dArea, dCX, dCY;
		CRect rcBlob;
		AlgoBlob algoBlob;

		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;

		cv::Mat BinImage2(Foot2D_img.rows, Foot2D_img.cols, CV_8UC1);

		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

		int nFilter = (algoBlob.m_bFilterIsUse) ? algoBlob.m_nFilterStepNarrow : 0;
		int nCntWingBlob = m_pProcMilAlgo->CalcBlob_Select(Foot2D_img.data, BinImage2.data, Foot2D_img.cols, Foot2D_img.rows, 10, 0, false, nFilter, _ETypeBlob::_eSelectBigger, -1, -1, -1, -1, -1, -1, -1, true, 2);

		Foot2D_img = BinImage2.clone();
		BinWing = Foot2D_img;

		for (int r = 0; r < Img3D_Candidates.rows; r++)
		{

			float* fHightPtr = Img3D_Candidates.ptr<float>(r);
			UCHAR* ptrFootBW = Foot2D_img.ptr(r);

			for (int c = 0; c < Img3D_Candidates.cols; c++)
			{
				if (ptrFootBW[c] == 0)
				{
					fHightPtr[c] = 0;
				}
			}
		}

		//Sobel Edge Detect
		cv::Mat gradX, gradY;
		cv::Mat gradX_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);
		cv::Mat gradY_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);

		cv::Sobel(Img3D_Candidates, gradX, CV_32FC1, 1, 0, 3);
		cv::Sobel(Img3D_Candidates, gradY, CV_32FC1, 0, 1, 3);
		cv::convertScaleAbs(gradX, gradX_ConvertImg, 1, 0);	//A½AC º?E­μμ¿¡ ´eCN A¤º¸ A?Ao¸| A§CØ μ￥AIAI A¸AOA≫ CV_16S·I AoA¤CN μU, ´U½A CV_8U·I º?E?
		cv::convertScaleAbs(gradY, gradY_ConvertImg, 1, 0);

		//Detect Wing Area Using SobelX Gradient
		cv::Mat cvFootSobelXResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelXResultImg.setTo(0);
		size_t floatstep_3DCorrect = Img3D_Candidates.step / sizeof(float);

		int nInputRemoveWireH = 220;
// 		if (pFoot->nRemoveWireHeight > 0)
// 		{
// 			nInputRemoveWireH = pFoot->nRemoveWireHeight;
// 		}

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


#if _DEBUG
		cv::imwrite("D:\\FootInspImg\\FindCandidate_Wing_SobelX_Img.bmp", gradX_ConvertImg);
#endif

		cv::Mat cvFootSobelYResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelYResultImg.setTo(0);
		for (int y = 0; y < gradY_ConvertImg.rows; y++)
		{
			uchar * ptrSobelY = gradY_ConvertImg.data;
			uchar * ptrSobelYRes = cvFootSobelYResultImg.data;

			for (int x = 0; x < gradY_ConvertImg.cols; x++)
			{
				int val = ptrSobelY[y * gradY_ConvertImg.step + x];
				if (val > 50 && val < 150)
				{
					ptrSobelYRes[y * gradY_ConvertImg.step + x] = 255;
				}
			}
		}

		// 	//Canny Edge Detect
		// 	cv::Mat Canny3DFoot_Img = Img3D.clone();
		// 	cv::normalize(Canny3DFoot_Img, Canny3DFoot_Img, 0, 255, cv::NORM_MINMAX);
		// 	Canny3DFoot_Img.convertTo(Canny3DFoot_Img, CV_8UC1);
		// 	cv::Canny(Canny3DFoot_Img, Canny3DFoot_Img, 40, 60, 3, false);
		// 
		// 	//Laplacian Edge Detect
		// 	cv::Mat Laplacian3DFoot_ConvertImg(Img3D.rows, Img3D.cols, CV_8UC1);
		// 	cv::Mat Laplacian3DFoot_Img(Img3D.rows, Img3D.cols, CV_32FC1, Img3D.data);
		// 	cv::normalize(Laplacian3DFoot_Img, Laplacian3DFoot_Img, 0, 255, cv::NORM_MINMAX);
		// 	Laplacian3DFoot_Img.convertTo(Laplacian3DFoot_Img, CV_16SC1);
		// 	cv::Laplacian(Laplacian3DFoot_Img, Laplacian3DFoot_Img, CV_16SC1, 3);
		// 	cv::convertScaleAbs(Laplacian3DFoot_Img, Laplacian3DFoot_ConvertImg, 1, 0);


#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_Wing_3D_sobelX_Img.bmp", cvFootSobelXResultImg);
		cv::imwrite("D:\\FootRst\\FindCandidate_Wing_SubFlowDownFromInput.bmp", BinWing);
#endif
		//Bitwise Operations - SobelX  AND  WingCandidatesImg
		cv::Mat re = cvFootSobelXResultImg & BinWing;
		cv::Mat BinImage(re.rows, re.cols, CV_8UC1);

		UCHAR *pUcImgDst2D = NULL;
		UCHAR *pUcImgDst3D = NULL;
		UCHAR *pUcImgBinary = BinImage.data;

		//double dArea, dCX, dCY;
		//CRect rcBlob;
		//PIAL::_AlgoBlob algoBlob;
		//AlgoBlob algoBlob;
		algoBlob.m_bUseIPC = false;
		algoBlob.m_bInvertCheck = false;
		algoBlob.m_bFilterIsUse = false;
		algoBlob.m_nTypeSelectBlob = _ETypeBlob::_eSelectMix;
		algoBlob.m_bInsp3D = false;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_dHeightRateMin = 0;
		algoBlob.m_dHeightRateMax = 0;
		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 0;
		algoBlob.m_nMaxBinary = 100;
		algoBlob.m_bFillHole = true;

		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		//TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

		// 		PIAL::PI_Buff org2DSrc(re);
		// 		PIAL::PI_Buff org3DSrc(Img3D_Candidates.ptr<float>(0), Img3D_Candidates.cols, Img3D_Candidates.rows, Img3D_Candidates.cols * 4);
		// 
		// 		PIAL::PI_Buff pTempImgBlob2(re.cols, re.rows);
				///////////////////////////
		int nCntBlobWing = CPInsp::BlobImageStruct(algoBlob, re.data, Img3D_Candidates.ptr<float>(0), NULL, re.cols, re.rows, 100, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
		//int nCntBlobWing = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2DSrc, &org3DSrc, NULL, &pTempImgBlob2, re.cols, re.rows, 100,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);

//		BinImage = pTempImgBlob2.Mat();

		if (nCntBlobWing < 1)
		{
			re.copyTo(*dst);
			return false;
		}

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_Wing_AfterSobelMask_Img.bmp", BinImage);
#endif

		cv::Mat moph = BinImage.clone();

		//morphology
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(moph, moph, kernel);
		cv::erode(moph, moph, kernel);
		cv::dilate(moph, moph, kernel);
		cv::dilate(moph, moph, kernel);
// 		cv::erode(moph, moph, kernel);
// 		cv::dilate(moph, moph, kernel);

		moph = moph & BinWing;

		cv::Mat BinImage22(re.rows, re.cols, CV_8UC1);
		int nCntBlobWing_Last = CPInsp::BlobImageStruct(algoBlob, moph.data, Img3D_Candidates.ptr<float>(0), NULL, moph.cols, moph.rows, 100, &dArea, &dCX, &dCY, &rcBlob, BinImage22.data, stTieAreaNULL, false);
		int nWing3DBlobRectMinX = moph.cols;
		int nWing3DBlobRectMaxX = 0;
		int nWing3DBlobRectMinY = moph.rows;
		int nWing3DBlobRectMaxY = 0;
		for (int r = 0; r < BinImage22.rows; r++)
		{
			UCHAR* Ptr = BinImage22.data;
			for (int c = 0; c < BinImage22.cols; c++)
			{
				if (Ptr[r * BinImage22.step + c] == 255)
				{
					if (r < nWing3DBlobRectMinY)	nWing3DBlobRectMinY = r;
					if (r > nWing3DBlobRectMaxY)	nWing3DBlobRectMaxY = r;
					if (c < nWing3DBlobRectMinX)	nWing3DBlobRectMinX = c;
					if (c > nWing3DBlobRectMaxX)	nWing3DBlobRectMaxX = c;

				}
			}
		}

		//?? ?? ??? ?? ???? ??
		CRect* rcArrBlob = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &rcArrBlob, nCntBlobWing_Last);
		memset(rcArrBlob, 0, sizeof(CRect) * nCntBlobWing_Last);
		
		m_pProcMilAlgo->GetBlobResult_ALL(NULL, NULL, NULL, rcArrBlob);
		int nMinHeight = BinImage22.rows -1;
		for (int a = 0; a < nCntBlobWing_Last; a++)
		{
			int nH = rcArrBlob[a].Height();

			if (nMinHeight > nH)
				nMinHeight = nH;
		}
		Delete_1DArray(&rcArrBlob);

		if ( (nWing3DBlobRectMaxY - nWing3DBlobRectMinY + 1) *m_resolY < pFoot->m_fArrOptionValue[m_eFoot_Length_1][(int)m_eMMD::eMMD_Min] * 1.1 ||
			 ((nWing3DBlobRectMaxY - nWing3DBlobRectMinY + 1) *m_resolY < pFoot->m_fArrOptionValue[m_eFoot_Length_1][(int)m_eMMD::eMMD_Min] * 1.3) && pFoot->m_bUse2Foot ||
			 nCntBlobWing_Last ==1 ||
			 (nMinHeight != BinImage22.rows - 1) && nMinHeight*m_resolY < pFoot->m_fArrOptionValue[m_eFoot_Length_1][(int)m_eMMD::eMMD_Min] 		)
		{
			moph = BinImage.clone();

			//morphology
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
			
			cv::dilate(moph, moph, kernel);
			cv::dilate(moph, moph, kernel);
			cv::erode(moph, moph, kernel);
			cv::erode(moph, moph, kernel);

			cv::Mat kernel_1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));

			cv::erode(moph, moph, kernel_1);
			cv::dilate(moph, moph, kernel_1);
			// 		cv::erode(moph, moph, kernel);
			// 		cv::dilate(moph, moph, kernel);

			moph = moph & BinWing;

			int nCntBlobWing_Last = CPInsp::BlobImageStruct(algoBlob, moph.data, Img3D_Candidates.ptr<float>(0), NULL, moph.cols, moph.rows, 100, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, false);
			moph = BinImage.clone();
		}

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_Wing_OUTPUT_Img.bmp", moph);
#endif
#if _DEBUG
		//μð¹o±e AI¹IAo AuAa
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_WingSobel.bmp"), gradX_ConvertImg);
			cv::imwrite(rawname + std::string("_WingSobelBW.bmp"), cvFootSobelXResultImg);

			cv::imwrite(rawname + std::string("_WingSobelBWAndTeachBW.bmp"), re);
			cv::imwrite(rawname + std::string("_WingSobelBWAndTeachBW_moph.bmp"), moph);
		}
#endif

#if _AlgoTool
#if _DEBUG
#ifdef ALGORITHMTOOL_CPP_USE
		CString eStrDeb;
		eStrDeb.Format(_T("D:\\FootRst\\Debug\\2_FindCandidate_Wing_OUTPUT_Img_%d.bmp"), nFootAlgoCnt);	//C² ¾E°i¸®Ao °³¼o¸¸A­ 
		std::string strDeb = std::string(CT2CA(eStrDeb));
		cv::imwrite(strDeb, moph);
#endif
#endif
#endif
		////////////////////////////////////////////////////////////////////////////
		//Foot 3D Height Y-Axis Gradient MaxBlob
		cv::Mat BinImage_Tail(re.rows, re.cols, CV_8UC1);
		BinImage_Tail.setTo(0);
		UCHAR *pUcImgBinary_Tail = cvFootSobelYResultImg.data;

		int tmp = algoBlob.m_nTypeSelectBlob;
		algoBlob.m_nTypeSelectBlob = _ETypeBlob::_eSelectBigger;		//maxBlob
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

		// 		PIAL::PI_Buff org2DSrc1(cvFootSobelYResultImg);
		// 		PIAL::PI_Buff org3DSrc1(Img3D_Candidates.ptr<float>(0), Img3D_Candidates.cols, Img3D_Candidates.rows, Img3D_Candidates.cols * 4);

				//PIAL::PI_Buff pTempImgBlob(cvFootSobelYResultImg.cols, cvFootSobelYResultImg.rows);
				///////////////////////////
		int nCntBlob_Tail = CPInsp::BlobImageStruct(algoBlob, cvFootSobelYResultImg.data, Img3D_Candidates.ptr<float>(0), NULL, cvFootSobelYResultImg.cols, cvFootSobelYResultImg.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage_Tail.data, stTieAreaNULL, true);
		//int nCntBlob_Tail = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2DSrc1, &org3DSrc1, NULL, &pTempImgBlob, cvFootSobelYResultImg.cols, cvFootSobelYResultImg.rows, 4,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);
		//BinImage_Tail = pTempImgBlob.Mat();

		algoBlob.m_nTypeSelectBlob = tmp;

		if (nCntBlob_Tail > 0)
		{
			cv::Mat moph_Tail = BinImage_Tail.clone();
			cv::Mat kernel_Tail = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
			cv::dilate(moph_Tail, moph_Tail, kernel_Tail);
			cv::dilate(moph_Tail, moph_Tail, kernel_Tail);
			cv::erode(moph_Tail, moph_Tail, kernel_Tail);
			cv::erode(moph_Tail, moph_Tail, kernel_Tail);

			cv::erode(moph_Tail, moph_Tail, kernel_Tail);
			cv::dilate(moph_Tail, moph_Tail, kernel_Tail);
			cv::dilate(moph_Tail, moph_Tail, kernel_Tail);
			//

			//Remove Tail Area
			cv::Mat subWingToTail;
			cv::Mat MatFootBW_rst;
			cv::threshold(Img3D_Candidates, MatFootBW_rst, 0, 255, cv::THRESH_BINARY);

			int min_x = moph.cols * moph.rows, max_x = 0;
			int min_y = moph.cols * moph.rows, max_y = 0;

			size_t floatstep_3DCorrect = Img3D_Candidates.step / sizeof(float);
			for (int i = 0; i < Img3D_Candidates.rows; i++)
			{
				float* rowptr = (float*)Img3D_Candidates.data;
				for (int j = 0; j < Img3D_Candidates.cols; j++)
				{
					int val = rowptr[i * floatstep_3DCorrect + j];
					if (val != 0)
					{
						if (j < min_x) min_x = j;
						if (j > max_x) max_x = j;
						if (i < min_y) min_y = i;
						if (i > max_y) max_y = i;
					}
				}
			}

			int min_x_Tail = moph.cols * moph.rows, max_x_Tail = 0;
			int min_y_Tail = moph.cols * moph.rows, max_y_Tail = 0;

			for (int y = 0; y < moph_Tail.rows; y++)
			{
				uchar * rowptr = moph_Tail.data;
				for (int x = 0; x < moph_Tail.cols; x++)
				{
					int val = rowptr[y * moph_Tail.cols + x];
					if (val == 255)
					{
						if (x < min_x_Tail) min_x_Tail = x;
						if (x > max_x_Tail) max_x_Tail = x;
						if (y < min_y_Tail) min_y_Tail = y;
						if (y > max_y_Tail) max_y_Tail = y;
					}
				}
			}

			if (min_x_Tail != moph.cols * moph.rows && max_x_Tail != 0 &&
				min_y_Tail != moph.cols * moph.rows && max_y_Tail != 0)
			{
				if (CenterPo.y > min_y_Tail || CenterPo.y > max_y_Tail)
				{
					if (min_y <= min_y_Tail)
					{
						subWingToTail = moph - moph_Tail;
					}
					else
						subWingToTail = moph;
				}
				else
					subWingToTail = moph;
			}
			else
				subWingToTail = moph;


			subWingToTail.copyTo(*dst);
		}
		else
		{
			moph.copyTo(*dst);
		}

		flag = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::FindCandidate_Wing() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return flag;
}

bool CPInsp_Wedge::FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	//If the Foot cannot be found by pattern matching, the 2D and 3D image is used.
	bool ret = false;

	ret = WingWedgeImage(pFoot, CenterPo, seta, cvFootImg, cvWedgeImg, cvWing, cv3DImg, PadImg, ucArrDstImg, WingImg_SecondCandi, nStartX, nStartY, nUIFootBinIDX, bWingRstVisible);

	return ret;
}

bool CPInsp_Wedge::WingWedgeImage(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	bool ret = false;
	int nLine = __LINE__;

	try
	{
		CString ImagePath;
		cv::Mat Img = (*cvWing).clone();

		cv::Mat fHRectImg;
		cv::Mat fHImg = (*cv3DImg).clone();
		cv::Mat fHRoImg;
		fHRectImg = fHImg.clone();
		//cv::Point FootRoImgMargin = WarpAffine(fHRectImg, fHRoImg, seta, fHImg.size(), CenterPo);	//Foot 3D Img E¸Au
		cv::Point FootRoImgMargin = WarpAffine_IPPRotate_3D(fHRectImg, fHRoImg, seta, fHImg.size(), CenterPo);	//Foot 3D Img E¸Au

		cv::Mat roImgFoot;
		cv::Mat ImgFoot = (*cvFootImg);
		//WarpAffine(ImgFoot, roImgFoot, seta, ImgFoot.size(), CenterPo);		////Foot Img E¸Au
		cv::Point margin_New = WarpAffine_IPPRotate(ImgFoot, roImgFoot, seta, ImgFoot.size(), CenterPo);

		//¿ⓒ±a¼­ Foot A¤¿oAO±a
		for (int r = 0; r < roImgFoot.rows; r++)
		{
			UCHAR* Ptr = roImgFoot.data;

			int nLeftFootX = 0;
			int nRightFootX = 0;

			for (int c = 0; c < roImgFoot.cols; c++)
			{
				if (Ptr[r * roImgFoot.step + c] == 255)
				{
					nLeftFootX = c;
					break;
				}
			}

			for (int c = roImgFoot.cols - 1; c >= 0; c--)
			{
				if (Ptr[r * roImgFoot.step + c] == 255)
				{
					nRightFootX = c;
					break;
				}
			}

			if (nLeftFootX != 0 && nRightFootX != 0)
			{
				for (int c = nLeftFootX; c < nRightFootX; c++)
				{
					Ptr[r * roImgFoot.step + c] = 255;
				}
			}

		}

		cv::Mat FootRstRoImg;

		cv::Point CenterPos_Revert = NULL;
		CenterPos_Revert.x = CenterPo.x - margin_New.x;
		CenterPos_Revert.y = CenterPo.y - margin_New.y;
		//cv::Point margin_Foot = WarpAffine_IPPRotate(roImgFoot, FootRstRoImg, -seta, FootRstRoImg.size(), NCenter);
		WarpAffine_IPPRotate(roImgFoot, FootRstRoImg, -(seta), roImgFoot.size(), CenterPos_Revert);	//AOA¾ AI¹IAo ¿ø≫oº¹±I¸| A§CN E¸Au

		cv::Mat WingOrg = cv::Mat(cvFootImg->rows, cvFootImg->cols, CV_8UC1);
		WingOrg.setTo(0);

		int ncvImgWidth = FootRstRoImg.cols;
		int ncvImgHeight = FootRstRoImg.rows;
		int nWingTmpMin_x = ncvImgWidth * ncvImgHeight, nWingTmpMax_x = 0;
		int nWingTmpMin_y = ncvImgWidth * ncvImgHeight, nWingTmpMax_y = 0;
		for (int r = 0; r < FootRstRoImg.rows; r++)
		{
			UCHAR* Ptr = FootRstRoImg.data;
			for (int c = 0; c < FootRstRoImg.cols; c++)
			{
				if (Ptr[r*FootRstRoImg.step + c] == 255)
				{
					if (c < nWingTmpMin_x) nWingTmpMin_x = c;
					if (c > nWingTmpMax_x) nWingTmpMax_x = c;
					if (r < nWingTmpMin_y) nWingTmpMin_y = r;
					if (r > nWingTmpMax_y) nWingTmpMax_y = r;
				}
			}
		}

		bool existWingRst = true;

		//wing °a°u ¾øA≫ ½A A¶°C¹® ºuA®³ª°¨
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

			//E¸Au ¿μ≫oAC Aß½EA¡°u Rect ½AAUA§A¡AC A÷AI °ª °e≫e
			int diffWingCX = (CenterPo.x - FootRoImgMargin.x) - nWingTmpMin_x;
			int diffWingCY = (CenterPo.y - FootRoImgMargin.y) - nWingTmpMin_y;

			//Wing °E≫c°a°u AU¸￥ AI¹IAo
			cv::Mat WingRectRst = FootRstRoImg(cv::Rect(nWingTmpMin_x, nWingTmpMin_y, nWingTmpMax_x - nWingTmpMin_x, nWingTmpMax_y - nWingTmpMin_y));

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

			//InspRect¿¡ ¸A°O Wing AOA¾ °a°u ¿μ≫o ≫y¼º
			WingOrg.setTo(0);
			cv::Mat A = WingOrg(cv::Rect(WingRectStX, WingRectStY, WingRectRst.cols, WingRectRst.rows));
			WingRectRst.copyTo(A);

			/////
		}

		WingOrg.copyTo(*cvFootImg);

		cv::Mat cvRoImgFoot = roImgFoot(cv::Rect(0, 0, ImgFoot.cols, ImgFoot.rows));

#if _DEBUG
		ImagePath.Format(_T("D:\\FootRst\\FindCandidate_Tail_Input.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), cvRoImgFoot);
#endif

		//Wedge EAº¸±º AßAa
		bool FindWedgeCandidates = false;
		cv::Mat WedgeCandidatesImg;
		WedgeCandidatesImg.setTo(0);
		ret = FindWedgeCandidates = FindCandidate_Wedge(pFoot, CenterPo, seta, &WedgeCandidatesImg, cvFootImg, cvWedgeImg, cvWing, &fHImg);

		cv::Mat WedgeCandClipImg = WedgeCandidatesImg;

		cv::Mat RemovedWedgeCandidates;
		if (FindWedgeCandidates)
		{
			RemovedWedgeCandidates = (*cvFootImg) - WedgeCandidatesImg;

#if _DEBUG
			ImagePath.Format(_T("D:\\FootInspImg\\RemovedWedgeCandidates.bmp"));
			cv::imwrite(std::string(CT2A(ImagePath)), RemovedWedgeCandidates);
#endif
		}

		//Tail End Point EAº¸±º AßAa
		int TailEndPosY = 0;
		//bool FindTailEndPosCandidates = false;
		//cv::Point WedgeCandRoImgMargin = WarpAffine(WedgeCandClipImg, WedgeCandClipImg, seta, WedgeCandClipImg.size(), CenterPo);	//Foot 3D Img E¸Au
		//cv::Point WedgeCandRoImgMargin = WarpAffine_IPPRotate(WedgeCandClipImg, WedgeCandClipImg, seta, WedgeCandClipImg.size(), CenterPo);	//Wing Bin Img E¸Au

#if _DEBUG
		ImagePath.Format(_T("D:\\FootRst\\FindCandidate_Wedge_Output.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), WedgeCandClipImg);
#endif

		//int WedgeEndPosY = 0;
		//FindTailEndPosCandidates = FindCandidate_TailEndPosY(fHRoImg, FindWedgeCandidates, WedgeCandClipImg, CenterPo, FootRoImgMargin, TailEndPosY, WedgeEndPosY);

		//WingAC ¸eAu°u WingAC ¿Þ/¿A¸￥AE ±æAI ±¸CI´A CO¼o
		if (FindWedgeCandidates)
			Img = RemovedWedgeCandidates;
		else
			Img = (*cvWing).clone();

		cv::Mat roImg;
		//cv::Point Margin = WarpAffine(Img, roImg, seta, Img.size(), CenterPo);		//Wing Bin Img E¸Au
		cv::Point FootMargin = WarpAffine_IPPRotate(Img, roImg, seta, Img.size(), CenterPo);	//Wing Bin Img E¸Au

// 		if (FindTailEndPosCandidates)
// 		{
// 			for (int r = 0; r < roImg.rows; r++)
// 			{
// 				UCHAR* Ptr = roImg.data;
// 				for (int c = 0; c < roImg.cols; c++)
// 				{
// 					if (Ptr[r * roImg.step + c] == 255)
// 					{
// 						if (r < TailEndPosY)
// 							Ptr[r * roImg.step + c] = 0;
// 						//if( r > WedgeEndPosY)
// 						//	Ptr[r * roImg.step + c] = 0;
// 					}
// 				}
// 			}
// 		}

// #if _DEBUG
// 		ImagePath.Format(_T("D:\\FootRst\\FindCandidate_TailEndPosY_Output.bmp"));
// 		cv::imwrite(std::string(CT2A(ImagePath)), roImg);
// #endif

		bool bFindWingStAndEndPosCandidates = false;
		cv::Point WingLStCandi, WingLEdCandi, WingRStCandi, WingREdCandi;



		cv::Mat RoImgWedgeCandi;
		cv::Mat ImgWedgeCandi = (WedgeCandidatesImg);
		//WarpAffine(ImgWedgeCandi, RoImgWedgeCandi, seta, ImgWedgeCandi.size(), CenterPo);		////Foot Img E¸Au
		WarpAffine_IPPRotate(ImgWedgeCandi, RoImgWedgeCandi, seta, ImgWedgeCandi.size(), CenterPo);	//Wing Bin Img E¸Au

		if (m_WingLenthMinimumMM == 0)
		{
			m_WingLenthMinimumMM = 60.0 * m_resolX;
		}

		bFindWingStAndEndPosCandidates = FindCandidate_WingStartAndWingEnd(&cvRoImgFoot, CenterPo, FootRoImgMargin, m_WingLenthMinimumMM, &WingLStCandi, &WingLEdCandi, &WingRStCandi, &WingREdCandi, fHRoImg, cvRoImgFoot.rows - ImgFoot.rows > 1, FindWedgeCandidates, &RoImgWedgeCandi);

		if (bFindWingStAndEndPosCandidates)
		{
			for (int r = 0; r < roImg.rows; r++)
			{
				UCHAR* Ptr = roImg.data;
				for (int c = 0; c < roImg.cols; c++)
				{
					if (Ptr[r * roImg.step + c] == 255)
					{
						if ((r < WingLStCandi.y) && (WingLStCandi.y < CenterPo.y))
							Ptr[r * roImg.step + c] = 0;
						if ((r > WingLEdCandi.y) && (WingLEdCandi.y != 0) && (WingLEdCandi.y > CenterPo.y))
							Ptr[r * roImg.step + c] = 0;
					}
				}
			}
		}

#if _DEBUG
		ImagePath.Format(_T("D:\\FootRst\\FindCandidate_WingStartAndWingEnd_OutPUT_Img.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), roImg);
#endif

		cv::Mat CutImg = roImg(cv::Rect(0, 0, Img.cols, Img.rows));

#if _DEBUG
		ImagePath.Format(_T("D:\\FootInspImg\\MaskFootDiffX_InputWingImg.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), CutImg);
#endif

		//cv::Mat roWingCandiSecond = RemovedWedgeCandidates.clone();
		//WarpAffine(RemovedWedgeCandidates, roWingCandiSecond, seta, roWingCandiSecond.size(), CenterPo);		////Foot Img E¸Au
		//cv::Mat CutImg_Second = roWingCandiSecond(cv::Rect(0, 0, Img.cols, Img.rows));
		//bool bFindCandidateWing_Second = FindCandidate_Wing(fHRoImg, roWingCandiSecond(cv::Rect(0, 0, Img.cols, Img.rows)).clone(), &CutImg_Second, CenterPo);

#if _DEBUG
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_WingInputImg.bmp"), roImg);
		}
#endif

		//MaskFootDiffX(fHRoImg, roImg(cv::Rect(0, 0, Img.cols, bottomPo.y)).clone(), &CutImg, m_nFootHDiffmin, m_nFootHDiffmax);	//회전된 Wing_BinImg에 회전된 Wing_3DImg_sobel 마스크로 Masking하는 함수
		bool bFindCandidateWing = FindCandidate_Wing(pFoot, fHRoImg, roImg(cv::Rect(0, 0, Img.cols, Img.rows)).clone(), &CutImg, CenterPo);

		cv::Mat Foot3DBlobRstImg = (*WingImg_SecondCandi).clone();

		cv::Mat WingImg = CutImg.clone();
		WingImg.copyTo(*WingImg_SecondCandi);

#if _DEBUG
		ImagePath.Format(_T("D:\\FootInspImg\\MaskFootRotate.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), Foot3DBlobRstImg);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    	cv::imwrite(std::string(CT2A(ImagePath)), CutImg);
#endif

		//Remove the area below the detected Foot Using 3D Blob
		int nFoot3DBlobRectMinX = CutImg.cols;
		int nFoot3DBlobRectMaxX = 0;
		int nFoot3DBlobRectMinY = CutImg.rows;
		int nFoot3DBlobRectMaxY = 0;
		for (int r = 0; r < Foot3DBlobRstImg.rows; r++)
		{
			UCHAR* Ptr = Foot3DBlobRstImg.data;
			for (int c = 0; c < Foot3DBlobRstImg.cols; c++)
			{
				if (Ptr[r * Foot3DBlobRstImg.step + c] == 255)
				{
					if (r < nFoot3DBlobRectMinY)	nFoot3DBlobRectMinY = r;
					if (r > nFoot3DBlobRectMaxY)	nFoot3DBlobRectMaxY = r;

					if (c < nFoot3DBlobRectMinX)	nFoot3DBlobRectMinX = c;
					if (c > nFoot3DBlobRectMaxX)	nFoot3DBlobRectMaxX = c;

				}
			}
		}
		if (nFoot3DBlobRectMaxY != 0 && nFoot3DBlobRectMaxY/* - 30*/ > 0)
		{
			for (int y = 0; y < CutImg.rows; y++)
			{
				UCHAR* ptrWingRst = CutImg.ptr(y);

				for (int x = 0; x < CutImg.cols; x++)
				{
					if (y > nFoot3DBlobRectMaxY /*- 30*/)
					{
						ptrWingRst[x] = 0;
					}
				}
			}
		}


		//If the Wing result contains Warped wire, remove those areas.
//  		int nWarpedWirePosY = 0;
//  		for (int x = 0; x < WingImg.cols; x++)
//  		{
//  			UCHAR* ptWing_RstImg = WingImg.data;
//   			if (nWarpedWirePosY != 0)
//   				break;
//  
//  			for (int y = CenterPo.y; y < WingImg.rows; y++)
//  			{
//  				int val_1 = ptWing_RstImg[x + WingImg.step * y];
//  
//  				if (val_1 == 255 && x == (CenterPo.x - margin_New.x))
//  				{
//  					nWarpedWirePosY = y;
//  					break;
//  				}
//  			}
//  		}
//  
//  		for (int y = 0; y < CutImg.rows; y++)
//  		{
//  			UCHAR* ptrWingRst_ClipWarpedWire = WingImg.ptr(y);
//  
//  			for (int x = 0; x < CutImg.cols; x++)
//  			{
//  				if (nWarpedWirePosY != 0 && y > nWarpedWirePosY)
//  				{
//  					//ptrWingRst_ClipWarpedWire[x] = 0;
//  				}
//  			}
//  		}

		CutImg = CutImg & cvRoImgFoot;

		CutImg.copyTo(*WingImg_SecondCandi);

#if _DEBUG
		ImagePath.Format(_T("D:\\FootInspImg\\MaskFootDiffX_OutputWingImg.bmp"));
		cv::imwrite(std::string(CT2A(ImagePath)), CutImg);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    	cv::imwrite(std::string(CT2A(ImagePath)), CutImg);
#endif

		cv::Mat WingRstRoImg;
		cv::Mat FootOrg = cv::Mat(cvFootImg->rows, cvFootImg->cols, CV_8UC1);
		FootOrg.setTo(0);
		nLine = __LINE__;
		//Wing Result 
		if (bFindCandidateWing)
		{
			bool existWingRst = true;
			WingRstRoImg = CutImg.clone();
			cv::Point CenterNew;
			CenterNew.x = CenterPo.x - FootRoImgMargin.x;
			CenterNew.y = CenterPo.y - FootRoImgMargin.y;
			//cv::Point margin_New = WarpAffine(CutImg, WingRstRoImg, -seta, Img.size(), CenterNew);	//Wing 3D Img E¸Au
			cv::Point margin_New = WarpAffine_IPPRotate(CutImg, WingRstRoImg, -seta, CutImg.size(), CenterNew);

			///
			int ncvImgWidth = WingRstRoImg.cols;
			int ncvImgHeight = WingRstRoImg.rows;
			int nWingTmpMin_x = ncvImgWidth * ncvImgHeight, nWingTmpMax_x = 0;
			int nWingTmpMin_y = ncvImgWidth * ncvImgHeight, nWingTmpMax_y = 0;
			for (int r = 0; r < WingRstRoImg.rows; r++)
			{
				UCHAR* Ptr = WingRstRoImg.data;
				for (int c = 0; c < WingRstRoImg.cols; c++)
				{
					if (Ptr[r*WingRstRoImg.step + c] == 255)
					{
						if (c < nWingTmpMin_x) nWingTmpMin_x = c;
						if (c > nWingTmpMax_x) nWingTmpMax_x = c;
						if (r < nWingTmpMin_y) nWingTmpMin_y = r;
						if (r > nWingTmpMax_y) nWingTmpMax_y = r;
					}
				}
			}

			//wing °a°u ¾øA≫ ½A A¶°C¹® ºuA®³ª°¨
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

				//E¸Au ¿μ≫oAC Aß½EA¡°u Rect ½AAUA§A¡AC A÷AI °ª °e≫e
				int diffWingCX = (CenterPo.x - FootRoImgMargin.x) - nWingTmpMin_x;
				int diffWingCY = (CenterPo.y - FootRoImgMargin.y) - nWingTmpMin_y;

				//Wing °E≫c°a°u AU¸￥ AI¹IAo
				cv::Mat WingRectRst = WingRstRoImg(cv::Rect(nWingTmpMin_x, nWingTmpMin_y, nWingTmpMax_x - nWingTmpMin_x, nWingTmpMax_y - nWingTmpMin_y));

				int WingRectStX = 0, WingRectStY = 0;

				if (CenterPo.x - diffWingCX < 0)
					WingRectStX = 0;
				else
					WingRectStX = CenterPo.x - diffWingCX;

				if (CenterPo.y - diffWingCY < 0)
					WingRectStY = 0;
				else
					WingRectStY = CenterPo.y - diffWingCY;

				if (WingRectStX + WingRectRst.cols > FootOrg.cols)
					WingRectStX = FootOrg.cols - WingRectRst.cols;

				if (WingRectStY + WingRectRst.rows > FootOrg.rows)
					WingRectStY = FootOrg.rows - WingRectRst.rows;

				//InspRect¿¡ ¸A°O Wing AOA¾ °a°u ¿μ≫o ≫y¼º
				FootOrg.setTo(0);
				cv::Mat A = FootOrg(cv::Rect(WingRectStX, WingRectStY, WingRectRst.cols, WingRectRst.rows));
				WingRectRst.copyTo(A);
				/////
			}

		}

#if _DEBUG
		cv::Mat cvPadDebug = PadImg->clone();

		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
		cv::Mat cvPadEdgeRstForDebug;
		cv::Canny(cvPadDebug, cvPadEdgeRstForDebug, 0, 0);
		cv::dilate(cvPadEdgeRstForDebug, cvPadEdgeRstForDebug, kernel);


		int nWidthR = pFoot->GetImageWidth();
		int nHeightR = pFoot->GetImageLength();
		cv::Mat cvPad2DImg(nHeightR, nWidthR, CV_8UC1);
		pFoot->GetImageMatrix(&cvPad2DImg, (int)m_eFootBin::m_eFootBin_Pad);

		//영역 표시한 이미지 저장
		cv::Mat RstPadUIimage;

		cv::Mat testPadimage1;
		cv::cvtColor(cvPad2DImg, testPadimage1, cv::COLOR_GRAY2RGB);

		cv::Mat testPadEdgeimage1;
		cv::cvtColor(cvPadEdgeRstForDebug, testPadEdgeimage1, cv::COLOR_GRAY2RGB);

		double alpha = 0.5;
		cv::addWeighted(testPadimage1, alpha, testPadEdgeimage1, (1 - alpha), 0, RstPadUIimage);

		/*cv::imwrite("D:\\FootRst\\cvPadEdge.bmp", RstPadUIimage);*/

		//디버깅 이미지 저장
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_cvPadEdge.bmp"), RstPadUIimage);
		}

#endif

#pragma region Write UI_Image

		nLine = __LINE__;
		//
		//UI¿¡ C￥½ACI±a A§CN AI¹IAo (ucArrDstImg)≫y¼º
		if (ucArrDstImg != NULL)
		{
			nLine = __LINE__;
			int nWidth = pFoot->GetImageWidth();
			int nHeight = pFoot->GetImageLength();
			nLine = __LINE__;

			cv::Mat mMask = *cvFootImg;

#ifdef _AlgoTool
			nUIFootBinIDX = 3;
#endif

			m_eFootBin footbin = (m_eFootBin)nUIFootBinIDX;

			//shkim edge UI work
			cv::Mat cvPadUI = PadImg->clone();
			cv::Mat cvFootUI = cvFootImg->clone();
			cvPadUI = cvPadUI - cvFootUI;

			cv::Mat cvWingUI;
			if (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing && bWingRstVisible)
			{
				cvWingUI = WingOrg.clone();
			}
			else
				cvWingUI = cvWing->clone();

			cv::Mat cvWedgeUI = cvWedgeImg->clone();

			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
			cv::Mat cvPadEdge, cvFootEdge, cvWingEdge, cvWedgeEdge;
			cv::Canny(cvPadUI, cvPadEdge, 0, 0);
			cv::Canny(cvFootUI, cvFootEdge, 0, 0);
			cv::erode(cvWingUI, cvWingEdge, kernel);
			cv::erode(cvWedgeUI, cvWedgeEdge, kernel);

			cv::dilate(cvPadEdge, cvPadEdge, kernel);
			cv::dilate(cvFootEdge, cvFootEdge, kernel);
			cv::bitwise_xor(cvWingUI, cvWingEdge, cvWingEdge);
			cv::bitwise_xor(cvWedgeUI, cvWedgeEdge, cvWedgeEdge);

#if _DEBUG
			cv::imwrite("D:\\cvWedgeEdge.bmp", cvWedgeEdge);
			cv::imwrite("D:\\cvFootEdge.bmp", cvFootEdge);
			cv::imwrite("D:\\cvWingEdge.bmp", cvWingEdge);
			cv::imwrite("D:\\cvWedgeEdge.bmp", cvWedgeEdge);
#endif

			UCHAR* ptrPad = cvPadEdge.data;
			UCHAR* ptrFoot = cvFootEdge.data;
			UCHAR* ptrWing = cvWingEdge.data;
			UCHAR* ptrWedge = cvWedgeEdge.data;

			if (pFoot->m_rst_Color_Image) //Color
			{

				for (int r = 0; r < cvFootImg->rows; r++)
				{
					for (int c = 0; c < cvFootImg->cols; c++)
					{
						int nIndex = r * nWidth + c;
						int nIndex2 = (nStartY + r) * pFoot->_WidthOrg + (nStartX + c);
						nIndex2 *= 3;

						if (footbin == m_eFootBin_Wing && ptrWing[nIndex] && !(bWingRstVisible&&nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing))
						{
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWing, 3);
						}
						else if (footbin == m_eFootBin_Wedge && ptrWedge[nIndex])
						{
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWedge, 3);
						}

						else if (footbin == m_eFootBin_Foot && ptrFoot[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcFoot, 3);
						else if (footbin == m_eFootBin_Pad && ptrPad[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcBackground, 3);
						else
						{
							if (!(nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing &&bWingRstVisible))
							{
								//Wing
								if (ptrWing[nIndex] && nUIFootBinIDX != (int)m_eFootBin::m_eFootBin_Wing && nUIFootBinIDX != (int)m_eFootBin::m_eFootBin_Wedge)
									memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWing, 3);

								//Wedge
								if (ptrWedge[nIndex])
									memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWedge, 3);

								//Foot
								else if (ptrFoot[nIndex])
									memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcFoot, 3);

								//Pad
								else if (ptrPad[nIndex])
									memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcBackground, 3);
							}
							else
							{
								//Foot
								if (ptrFoot[nIndex])
									memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcFoot, 3);

								//Pad
								else if (ptrPad[nIndex])
									memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcBackground, 3);
							}

						}
					}
				}


				/*for (int r = 0; r < PadImg->rows; r++)
				{
					for (int c = 0; c < PadImg->cols; c++)
					{
						int nIndex = r * nWidth + c;
						int nIndex2 = (nStartY + r) * pFoot->_WidthOrg + (nStartX + c);
						nIndex2 *= 3;

						if (ucArrDstImg[nIndex2] == 0 &&
							ucArrDstImg[nIndex2 + 1] == 0 &&
							ucArrDstImg[nIndex2 + 2] == 0 &&
							ptrPad[nIndex])
						{
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcBackground, 3);
						}
					}
				}*/
			}
			else   //Binary
			{
				for (int r = 0; r < mMask.rows; r++)
				{
					for (int c = 0; c < mMask.cols; c++)
					{
						int nIndex = r * nWidth + c;
						int nIndex2 = (nStartY + r) * pFoot->_WidthOrg + (nStartX + c);

						UCHAR ucData = ptrFoot[nIndex]; //Foot
						if (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing && !bWingRstVisible)
							ucData = ptrWing[nIndex]; //Wing
						else if (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wedge && !bWingRstVisible)
							ucData = ptrWedge[nIndex]; //Foot

						if (ucData > 0) ucArrDstImg[nIndex2] = ucData;
					}
				}
			}
		}

#pragma endregion
		if (FindWedgeCandidates)
			WedgeCandidatesImg.copyTo(*cvWedgeImg);

		if (bFindCandidateWing)
			WingOrg.copyTo(*cvWing);
	}
	catch (...)
	{
		CString eStr;
		eStr.Format(_T("[FootAlgo]WingWedgeImage error Line: %d pass"), nLine);
		g_pMPTI->AddLog(eStr);
	}
	return ret;
}

bool CPInsp_Wedge::getFootBondingStEndPoint(CFoot_Model* pFoot, cv::Mat* vImage, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, int CntX, float WedgeMinSpec)
{
	cv::Mat Intg, Dev;

	bool flag = false;
	bool bIsDetected_BondingSt = false, bIsDetected_BondingEnd = false;

	int nLine = __LINE__;
	try
	{
		cv::Mat localImg = vImage->clone();
#if _DEBUG
		cv::imwrite("D:\\FootRst\\getFootBondingStEndPoint_INPUT_Img.bmp", localImg);
#endif
		localImg &= 1;
		cv::integral(localImg, Intg, Dev);

		int LeftBondingPntScanStCnt = 0;
		int LeftBondingPntScanStIdx = 0;
		int LeftBondingPntScanEndIdx = 0;
		int LeftBondingPntScanStX = 0;
		int LeftBondingPntScanStY = 0;
		int LeftBondingPntScanEndX = 0;
		int LeftBondingPntScanEndY = 0;

		int RightBondingPntScanStCnt = 0;
		int RightBondingPntScanStIdx = 0;
		int RightBondingPntScanEndIdx = 0;
		int RightBondingPntScanStX = 0;
		int RightBondingPntScanStY = 0;
		int RightBondingPntScanEndX = 0;
		int RightBondingPntScanEndY = 0;

		std::vector<int> vSt, vEd;
		vSt.assign(Intg.rows, 0);
		vEd.assign(Intg.rows, 0);
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
					vSt[r - 1] = c;		//wing °¡·I ±æAI°¡ 0AI≫oAI °æ¿i St A§A¡·I AoA¤
					LeftBondingPntScanStCnt++;
					break;
				}
			}
			for (int c = CntX; c < localImg.cols - 1; c++)
			{
				if (uPtr[c] > 0)
				{
					vEd[r - 1] = c;		//wing ±æAI°¡ 0AI≫oAI °æ¿i Ed A§A¡·I AoA¤
					RightBondingPntScanStCnt++;
					break;
				}
			}
			if (LeftBondingPntScanStCnt == 1)
			{
				LeftBondingPntScanStX = vSt[r - 1];
				LeftBondingPntScanStIdx = r - 1;
			}
			if (RightBondingPntScanStCnt == 1)
			{
				RightBondingPntScanStX = vEd[r - 1];
				RightBondingPntScanStIdx = r - 1;
			}

			int ndif = vEd[r - 1] - vSt[r - 1] > 0 ? vEd[r - 1] - vSt[r - 1] : 0;
			if (vSt[r - 1]<1 || vEd[r - 1]>localImg.cols - 2)
				continue;

			if (ndif != 0)
			{
				pLSt->x = LeftBondingPntScanStX;
				pLSt->y = LeftBondingPntScanStIdx;

				pRSt->x = RightBondingPntScanStX;
				pRSt->y = RightBondingPntScanStIdx;

				bIsDetected_BondingSt = true;

				break;
			}
		}

		std::vector<int> vStB, vEdB;
		vStB.assign(Intg.rows, 0);
		vEdB.assign(Intg.rows, 0);

		int LeftBondingPntScanEndCnt = 0;
		int RightBondingPntScanEndCnt = 0;
		cv::Mat localImg_Org = vImage->clone();

		for (int r = vImage->rows; r > 1; r--)
		{
			int* nPtr = Intg.ptr<int>(r);
			int* nPtr_1 = Intg.ptr<int>(r - 1);

			if (nPtr[Intg.cols - 1] - nPtr_1[Intg.cols - 1] < 5)
				continue;

			UCHAR* uPtr = localImg.ptr(r - 1);
			UCHAR* uPtr_Img = localImg_Org.ptr(r - 1);

			if (uPtr_Img[CntX] == 0)
			{
				for (int c = CntX; c > 0; c--)
				{
					if (uPtr_Img[c] == 255)
					{
						vStB[r - 1] = c;		//wing °¡·I ±æAI°¡ 0AI≫oAI °æ¿i St A§A¡·I AoA¤
						LeftBondingPntScanEndCnt++;
						break;

						//LeftBondingPntScanEndX = c;		//wing °¡·I ±æAI°¡ 0AI≫oAI °æ¿i St A§A¡·I AoA¤
					}
				}

				for (int c = CntX; c < localImg_Org.cols - 1; c++)
				{
					if (uPtr_Img[c] == 255)
					{
						vEdB[r - 1] = c;		//wing ±æAI°¡ 0AI≫oAI °æ¿i Ed A§A¡·I AoA¤
						RightBondingPntScanEndCnt++;
						break;

						//RightBondingPntScanEndX = c;
					}
				}
			}

			if (LeftBondingPntScanEndCnt == 1)
			{
				LeftBondingPntScanEndY = vStB[r - 1];
				LeftBondingPntScanEndIdx = r - 1;
			}
			if (RightBondingPntScanEndCnt == 1)
			{
				RightBondingPntScanEndY = vEdB[r - 1];
				RightBondingPntScanEndIdx = r - 1;
			}

			int ndif = vEdB[r - 1] - vStB[r - 1] > 0 ? vEdB[r - 1] - vStB[r - 1] : 0;
			if (vStB[r - 1]<1 || vEdB[r - 1]>localImg.cols - 2)
				continue;

			if (ndif > 10 && vStB[r - 1] != 0 && vEdB[r - 1] != 0)
			{
				//½AAU AAC￥ AuAa
				//¿ⓒ±a¼­ ¹U·I AuAaCIAU
				pLEd->x = vStB[r - 1];
				pLEd->y = r;

				pREd->x = vEdB[r - 1];
				pREd->y = r;

				bIsDetected_BondingEnd = true;

				break;

			}
		}

		if (bIsDetected_BondingSt && bIsDetected_BondingEnd)
		{
			flag = true;

#if _DEBUG
			std::vector<cv::Point> vPoSt(2);
			std::vector<cv::Point> vPoEd(2);

			vPoSt[0] = *pLSt;
			vPoSt[1] = *pRSt;
			vPoEd[0] = *pLEd;
			vPoEd[1] = *pREd;

			ImagePointSave(_T("D:\\FootRst\\getFootBondingStEndPoint_WingLength.bmp"), &localImg, vPoSt, vPoEd);
#endif

		}

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::getFootBondingStEndPoint() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return flag;
}

bool CPInsp_Wedge::FindCandidate_TailAndWire_SobelXY(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat* cvFootImg, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, cv::Point& CenterPo, cv::Point& FootRoImgMargin, int &TailEndPosY, int &WireStartPosY)
{
	bool Ret = false;
	bool FindTailPos = false;
	bool FindWirePos = false;

	TailEndPosY = 0;

	int nLine = __LINE__;
	try
	{
		//Sobel Edge Detect
		cv::Mat Img3D = fHRoImg.clone();
		cv::Mat Img3D_Candidates = Img3D.clone();

		cv::Mat gradX, gradY;
		cv::Mat gradX_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);
		cv::Mat gradY_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);

		cv::Sobel(Img3D_Candidates, gradX, CV_32FC1, 1, 0, 3);
		cv::Sobel(Img3D_Candidates, gradY, CV_32FC1, 0, 1, 3);
		cv::convertScaleAbs(gradX, gradX_ConvertImg, 1, 0);	//A½AC º?E­μμ¿¡ ´eCN A¤º¸ A?Ao¸| A§CØ μ￥AIAI A¸AOA≫ CV_16S·I AoA¤CN μU, ´U½A CV_8U·I º?E?
		cv::convertScaleAbs(gradY, gradY_ConvertImg, 1, 0);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelX_Img.bmp", gradX_ConvertImg);
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelY_Img.bmp", gradY_ConvertImg);
#endif

		//SobelX Gradient Binarization Using min-max Range
		cv::Mat cvFootSobelXResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelXResultImg.setTo(0);
		cv::Mat cvFootSobelXResultImg_Wire(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelXResultImg_Wire.setTo(0);
		for (int y = 0; y < gradX_ConvertImg.rows; y++)
		{
			uchar * ptrSobelX = gradX_ConvertImg.data;
			uchar * ptrSobelXRes = cvFootSobelXResultImg.data;
			uchar * ptrSobelXRes_Wire = cvFootSobelXResultImg_Wire.data;
			uchar * ptrFootBW = cvFootImg->data;

			for (int x = 0; x < gradX_ConvertImg.cols; x++)
			{
				int val = ptrSobelX[y * gradX_ConvertImg.step + x];
				if (ptrFootBW[y * gradX_ConvertImg.step + x] != 0)
				{
					if (val > pFoot->nFootSobmin && val < pFoot->nFootSobmax)
					{
						ptrSobelXRes[y * gradX_ConvertImg.step + x] = 255;
					}
				}

			}
		}


#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelXBW_Img.bmp", cvFootSobelXResultImg);
#endif

		cv::Mat cvFootSobelYAllResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelYAllResultImg.setTo(0);
		cv::Mat cvFootSobelYResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelYResultImg.setTo(0);
		cv::Mat cvFootSobelYResultImg_Wire(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelYResultImg_Wire.setTo(0);
		for (int y = 0; y < cvFootSobelYResultImg.rows; y++)
		{
			UCHAR* ptrSobelY = gradY_ConvertImg.ptr(y);
			UCHAR* ptrSobelYRes = cvFootSobelYResultImg.ptr(y);
			UCHAR* ptrSobelYRes_Wire = cvFootSobelYResultImg_Wire.ptr(y);
			UCHAR* ptrSobelYResAll_Wire = cvFootSobelYAllResultImg.ptr(y);
			UCHAR* ptrFootBW = cvFootImg->ptr(y);

			for (int x = 0; x < cvFootSobelYResultImg.cols; x++)
			{
				int val = ptrSobelY[x];

				if (ptrFootBW[x] != 0 && y < CenterPo.y)
				{
					if (ptrSobelY[x] > 10 && ptrSobelY[x] < 150)
					{
						ptrSobelYRes[x] = 255;
					}
					ptrSobelYRes_Wire[x] = 0;
				}
				if (ptrFootBW[x] != 0 && y > CenterPo.y)
				{
					ptrSobelYRes[x] = 0;
					if (ptrSobelY[x] > 10 && ptrSobelY[x] < 150)
					{
						ptrSobelYRes_Wire[x] = 255;
					}
				}

				if (ptrFootBW[x] != 0 && ptrSobelY[x] > 10 && ptrSobelY[x] < 150)
					ptrSobelYResAll_Wire[x] = 255;
			}
		}


#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelYBW_Img.bmp", cvFootSobelYAllResultImg);
#endif

		cv::Mat SobelXAndY = cvFootSobelXResultImg & cvFootSobelYResultImg;
		cv::Mat SobelXAndY_Wire = cvFootSobelXResultImg & cvFootSobelYResultImg_Wire;

		//Sobel X + Sobel Y ¿μ≫o CO≫e EA, ¿U°u ¶oAI AßAa - Tail ºIºÐ°u Wire ºIºÐAC ¿¡Ao °­μμ°¡ A≪°O E®AIμE
		cv::Mat AddSobelXAndY = gradX_ConvertImg + gradY_ConvertImg + gradY_ConvertImg;
		cv::Mat canny3dfoot_img = Img3D.clone();
		for (int y = 0; y < canny3dfoot_img.rows; y++)
		{
			float* ptrInputImg = canny3dfoot_img.ptr<float>(y);
			UCHAR* ptrFootBW = cvFootImg->ptr(y);
			UCHAR* ptrAddSobelXAndY = AddSobelXAndY.ptr(y);

			for (int x = 0; x < canny3dfoot_img.cols; x++)
			{
				int val = ptrInputImg[x];

				if (ptrFootBW[x] == 0)
				{
					ptrInputImg[x] = 0;
					ptrAddSobelXAndY[x] = 0;
				}
			}
		}

		//¿U°u ¶oAI BW ¼oCa
		cv::Mat LineSobelXAndY(Img3D.rows, Img3D.cols, CV_8UC1);
		cv::Mat LineSobelXAndY_Wire(Img3D.rows, Img3D.cols, CV_8UC1);
		LineSobelXAndY.setTo(0);
		LineSobelXAndY_Wire.setTo(0);
		for (int y = 0; y < LineSobelXAndY.rows; y++)
		{
			UCHAR* ptrSobelXAndY = AddSobelXAndY.ptr(y);
			UCHAR* ptrLineSobelXAndYRes = LineSobelXAndY.ptr(y);
			UCHAR* ptrLineSobelXAndY_WireRes = LineSobelXAndY_Wire.ptr(y);
			UCHAR* ptrFootBW = cvFootImg->ptr(y);

			for (int x = 0; x < LineSobelXAndY.cols; x++)
			{
				int val = ptrSobelXAndY[x];

				if (ptrFootBW[x] != 0 && y < CenterPo.y)
				{
					if (ptrSobelXAndY[x] > 120)
					{
						ptrLineSobelXAndYRes[x] = 255;
					}
					ptrLineSobelXAndY_WireRes[x] = 0;
				}
				if (ptrFootBW[x] != 0 && y > CenterPo.y)
				{
					ptrLineSobelXAndYRes[x] = 0;

					if (ptrSobelXAndY[x] > 120)
					{
						ptrLineSobelXAndY_WireRes[x] = 255;
					}
				}
			}
		}
		//¿U°u¶oAI ¸ðÆu·IAo ¼oCa
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));
		cv::dilate(LineSobelXAndY, LineSobelXAndY, kernel);
		cv::dilate(LineSobelXAndY, LineSobelXAndY, kernel);
		cv::erode(LineSobelXAndY, LineSobelXAndY, kernel);
		cv::erode(LineSobelXAndY, LineSobelXAndY, kernel);

		//¿U°u¶oAI Blob ¼oCa
		AddSobelXAndY = AddSobelXAndY - gradX_ConvertImg;
		cv::Mat cvFootSobelXAndYRstImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelXAndYRstImg.setTo(0);
		for (int y = 0; y < cvFootSobelXAndYRstImg.rows; y++)
		{
			UCHAR* ptrSobelXAndY = AddSobelXAndY.ptr(y);
			UCHAR* ptrSobelXAndYRes = cvFootSobelXAndYRstImg.ptr(y);
			UCHAR* ptrFootBW = cvFootImg->ptr(y);

			for (int x = 0; x < cvFootSobelXAndYRstImg.cols; x++)
			{
				int val = ptrSobelXAndY[x];

				if (ptrFootBW[x] != 0 && y < CenterPo.y)
				{
					if (ptrSobelXAndY[x] > 50 && y < CenterPo.y)
					{
						ptrSobelXAndYRes[x] = 255;
					}
				}
				if (ptrFootBW[x] != 0 && y > CenterPo.y)
				{
					ptrSobelXAndYRes[x] = 0;
				}
			}
		}

		//Center ±aAØ A§AEAC Tail Blob, ¿A¸￥AEAC Wire Blob ¼oCa
		double dArea, dCX, dCY;
		CRect rcBlob;

		AlgoBlob algoBlob;
		//PIAL::_AlgoBlob algoBlob;

		algoBlob.m_bUseIPC = false;
		algoBlob.m_bInvertCheck = false;
		algoBlob.m_bFilterIsUse = false;
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		algoBlob.m_bInsp3D = false;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_dHeightRateMin = 0;
		algoBlob.m_dHeightRateMax = 0;
		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 0;
		algoBlob.m_nMaxBinary = 100;
		algoBlob.m_bFillHole = true;

		TotalInspExceptArea stTieAreaNULL;
		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

		//////////////////////////
// 		PIAL::PI_Buff org2DSrc(SobelXAndY);
// 		PIAL::PI_Buff org3DSrc(canny3dfoot_img.ptr<float>(0), canny3dfoot_img.cols, canny3dfoot_img.rows, canny3dfoot_img.cols * 4);
// 
// 		PIAL::PI_Buff pTempImgBlob(canny3dfoot_img.cols, canny3dfoot_img.rows);
		///////////////////////////

		cv::Mat BinImage(SobelXAndY.rows, SobelXAndY.cols, CV_8UC1);
		int nCntBlob = CPInsp::BlobImageStruct(algoBlob, SobelXAndY.data, canny3dfoot_img.ptr<float>(0), NULL, SobelXAndY.cols, SobelXAndY.rows, 10, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
		//int nCntBlob = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2DSrc, &org3DSrc, NULL, &pTempImgBlob, SobelXAndY.cols, SobelXAndY.rows, 10,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);
		//BinImage = pTempImgBlob.Mat();

		cv::dilate(BinImage, BinImage, kernel);
		cv::dilate(BinImage, BinImage, kernel);
		cv::erode(BinImage, BinImage, kernel);
		cv::erode(BinImage, BinImage, kernel);

		///////////////////////////
// 		PIAL::PI_Buff org2DSrc2(SobelXAndY_Wire);
// 		PIAL::PI_Buff org3DSrc2(canny3dfoot_img.ptr<float>(0), canny3dfoot_img.cols, canny3dfoot_img.rows, canny3dfoot_img.cols * 4);
// 
// 		PIAL::PI_Buff pTempImgBlob_Wire(SobelXAndY_Wire.cols, SobelXAndY_Wire.rows);
		///////////////////////////

		cv::Mat BinImage_Wire(SobelXAndY_Wire.rows, SobelXAndY_Wire.cols, CV_8UC1);
		int nCntBlob_Wire = CPInsp::BlobImageStruct(algoBlob, SobelXAndY_Wire.data, canny3dfoot_img.ptr<float>(0), NULL, SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, 10, &dArea, &dCX, &dCY, &rcBlob, BinImage_Wire.data, stTieAreaNULL, true);
		//int nCntBlob_Wire = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2DSrc2, &org3DSrc2, NULL, &pTempImgBlob_Wire, SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, 10,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);
		//BinImage_Wire = pTempImgBlob_Wire.Mat();

		cv::dilate(BinImage_Wire, BinImage_Wire, kernel);
		cv::dilate(BinImage_Wire, BinImage_Wire, kernel);
		cv::erode(BinImage_Wire, BinImage_Wire, kernel);
		cv::erode(BinImage_Wire, BinImage_Wire, kernel);

		//Foot Center ±aAØ ¾cAE Blob °￡ AO¼O °A¸® AßAa ¹× CØ´c AoA¡A≫ Tail ·I ¼±AA
		//¾cAE¿¡ ´eAAμC´A Blob AOA≫ °æ¿i¿¡¸¸ Blob °￡ °A¸® AßAa
		cv::Mat TailCandiBlobRst(BinImage.rows, BinImage.cols, CV_8UC1);
		cv::Mat LeftMaxBlob = BinImage.clone();
		cv::Mat RightMaxBlob = BinImage.clone();
		TailCandiBlobRst.setTo(0);

		cv::Mat WireCandiBlobRst(BinImage_Wire.rows, BinImage_Wire.cols, CV_8UC1);
		cv::Mat LeftMaxBlob_Wire = BinImage_Wire.clone();
		cv::Mat RightMaxBlob_Wire = BinImage_Wire.clone();
		WireCandiBlobRst.setTo(0);

		for (int y = 0; y < BinImage.rows; y++)
		{
			UCHAR* ptrLeftBlob = LeftMaxBlob.ptr(y);
			UCHAR* ptrRightBlob = RightMaxBlob.ptr(y);
			UCHAR* ptrLeftBlob_Wire = LeftMaxBlob_Wire.ptr(y);
			UCHAR* ptrRightBlob_Wire = RightMaxBlob_Wire.ptr(y);
			UCHAR* ptrTailCandiBlobRst = TailCandiBlobRst.ptr(y);

			for (int x = 0; x < BinImage.cols; x++)
			{
				if (x > CenterPo.x - FootRoImgMargin.x)
				{
					ptrLeftBlob[x] = 0;
					ptrLeftBlob_Wire[x] = 0;
				}
				if (x < CenterPo.x - FootRoImgMargin.x)
				{
					ptrRightBlob[x] = 0;
					ptrRightBlob_Wire[x] = 0;
				}

			}
		}

		//Tail - Max Blob ¼oCa
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;
		cv::Mat BinImageLeftBlob(SobelXAndY.rows, SobelXAndY.cols, CV_8UC1);

		///////////////////////////
// 		PIAL::PI_Buff org2D_L(LeftMaxBlob);
// 		PIAL::PI_Buff org3D_L(canny3dfoot_img.ptr<float>(0), SobelXAndY.cols, SobelXAndY.rows, SobelXAndY.cols * 4);
// 
// 		PIAL::PI_Buff pTempImgBlob_L(SobelXAndY.cols, SobelXAndY.rows);
// 		///////////////////////////

		int nCntLeftBlob = CPInsp::BlobImageStruct(algoBlob, LeftMaxBlob.data, canny3dfoot_img.ptr<float>(0), NULL, SobelXAndY.cols, SobelXAndY.rows, 10, &dArea, &dCX, &dCY, &rcBlob, BinImageLeftBlob.data, stTieAreaNULL, true);
		//int nCntLeftBlob = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2D_L, &org3D_L, NULL, &pTempImgBlob_L, SobelXAndY.cols, SobelXAndY.rows, 10,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);

		//BinImageLeftBlob = pTempImgBlob_L.Mat();

		///////////////////////////
// 		PIAL::PI_Buff org2D_R(RightMaxBlob);
// 		PIAL::PI_Buff org3D_R(canny3dfoot_img.ptr<float>(0), SobelXAndY.cols, SobelXAndY.rows, SobelXAndY.cols * 4);
// 
// 		PIAL::PI_Buff pTempImgBlob_R(SobelXAndY.cols, SobelXAndY.rows);
		///////////////////////////

		cv::Mat BinImageRightBlob(SobelXAndY.rows, SobelXAndY.cols, CV_8UC1);
		int nCntRightBlob = CPInsp::BlobImageStruct(algoBlob, RightMaxBlob.data, canny3dfoot_img.ptr<float>(0), NULL, SobelXAndY.cols, SobelXAndY.rows, 10, &dArea, &dCX, &dCY, &rcBlob, BinImageRightBlob.data, stTieAreaNULL, true);
		//int nCntRightBlob = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2D_R, &org3D_R, NULL, &pTempImgBlob_R, SobelXAndY.cols, SobelXAndY.rows, 10,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);

		//BinImageRightBlob = pTempImgBlob_R.Mat();

		if (nCntLeftBlob < 1)
			return false;
		if (nCntRightBlob < 1)
			return false;

		TailCandiBlobRst = BinImageLeftBlob + BinImageRightBlob;

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelXY_TailCandiImg.bmp", TailCandiBlobRst);
#endif

		//Wire - Max Blob ¼oCa
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

		///////////////////////////
// 		PIAL::PI_Buff org2D_WireL(LeftMaxBlob_Wire);
// 		PIAL::PI_Buff org3D_WireL(canny3dfoot_img.ptr<float>(0), SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, SobelXAndY_Wire.cols * 4);
// 
// 		PIAL::PI_Buff pTempImgBlob_WireL(SobelXAndY_Wire.cols, SobelXAndY_Wire.rows);
		///////////////////////////

		cv::Mat BinImageLeftBlob_Wire(SobelXAndY_Wire.rows, SobelXAndY_Wire.cols, CV_8UC1);

		int nCntLeftBlob_Wire = CPInsp::BlobImageStruct(algoBlob, LeftMaxBlob_Wire.data, canny3dfoot_img.ptr<float>(0), NULL, SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, 10, &dArea, &dCX, &dCY, &rcBlob, BinImageLeftBlob_Wire.data, stTieAreaNULL, true);
		//int nCntLeftBlob_Wire = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2D_WireL, &org3D_WireL, NULL, &pTempImgBlob_WireL, SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, 10,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);
		//BinImageLeftBlob_Wire = pTempImgBlob_WireL.Mat();

		///////////////////////////
// 		PIAL::PI_Buff org2D_WireR(RightMaxBlob_Wire);
// 		PIAL::PI_Buff org3D_WireR(canny3dfoot_img.ptr<float>(0), SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, SobelXAndY_Wire.cols * 4);
// 
// 		PIAL::PI_Buff pTempImgBlob_WireR(SobelXAndY_Wire.cols, SobelXAndY_Wire.rows);
// 		///////////////////////////

		cv::Mat BinImageRightBlob_Wire(SobelXAndY_Wire.rows, SobelXAndY_Wire.cols, CV_8UC1);
		int nCntRightBlob_Wire = CPInsp::BlobImageStruct(algoBlob, RightMaxBlob_Wire.data, canny3dfoot_img.ptr<float>(0), NULL, SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, 10, &dArea, &dCX, &dCY, &rcBlob, BinImageRightBlob_Wire.data, stTieAreaNULL, true);
		//int nCntRightBlob_Wire = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2D_WireR, &org3D_WireR, NULL, &pTempImgBlob_WireR, SobelXAndY_Wire.cols, SobelXAndY_Wire.rows, 10,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);

		//BinImageRightBlob_Wire = pTempImgBlob_WireR.Mat();

		if (nCntLeftBlob_Wire < 1)
			return false;
		if (nCntRightBlob_Wire < 1)
			return false;

		WireCandiBlobRst = BinImageLeftBlob_Wire + BinImageRightBlob_Wire;

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelXY_WireCandiImg.bmp", WireCandiBlobRst);
#endif

		//Tail ºIºÐ ¾c AE Blob °￡AC AO¼O °A¸® °e≫e
		int LeftDist = 0, RightDist = 0;
		std::vector<POINT> vSt;
		std::vector<POINT> vEd;
		POINT vTailLeftRst;
		POINT vTailRightRst;
		vSt.clear();
		vEd.clear();
		vTailLeftRst.x = 0;
		vTailLeftRst.y = 0;
		vTailRightRst.x = 0;
		vTailRightRst.y = 0;

		bool FindLeftBlobObj = false, FindRightBlobObj = false;
		int BlobMinDist = TailCandiBlobRst.cols - 1;

		POINT LeftPoint;
		POINT RightPoint;
		LeftPoint.x = 0;
		LeftPoint.y = 0;
		RightPoint.x = 0;
		RightPoint.y = 0;

		for (int y = 0; y < TailCandiBlobRst.rows; y++)
		{
			UCHAR* ptrTailCandiBlobRst = TailCandiBlobRst.ptr(y);
			UCHAR* ptrWireCandiBlobRst = WireCandiBlobRst.ptr(y);

			for (int x = CenterPo.x - FootRoImgMargin.x; x > 0; x--)
			{
				if (ptrTailCandiBlobRst[x] == 255)
				{
					LeftPoint.x = x;
					LeftPoint.y = y;
					FindLeftBlobObj = true;
					break;
				}
				else
					LeftDist++;
			}

			for (int x = CenterPo.x - FootRoImgMargin.x; x < TailCandiBlobRst.cols - 1; x++)
			{
				if (ptrTailCandiBlobRst[x] == 255)
				{
					RightPoint.x = x;
					RightPoint.y = y;
					FindRightBlobObj = true;
					break;
				}
				else
					RightDist++;

			}

			if (FindLeftBlobObj == true && FindRightBlobObj == true)
			{
				vSt.push_back(LeftPoint);
				vEd.push_back(RightPoint);

				if (LeftDist != 0 && RightDist != 0)
				{
					if (BlobMinDist > LeftDist + RightDist && (LeftDist + RightDist) <= TailCandiBlobRst.cols - 10)
					{
						BlobMinDist = LeftDist + RightDist;

						vTailLeftRst = LeftPoint;
						vTailRightRst = RightPoint;
					}
				}

			}
			else
			{


			}

			FindLeftBlobObj = false;
			FindRightBlobObj = false;
			LeftDist = 0;
			RightDist = 0;

		}

		TailEndPosY = vTailLeftRst.y;

		if (vTailLeftRst.y != 0)
		{
			FindTailPos = true;

			pLSt->x = vTailLeftRst.x;
			pLSt->y = vTailLeftRst.y;
			pRSt->x = vTailRightRst.x;
			pRSt->y = vTailRightRst.y;
		}
		else
			FindTailPos = false;


		//Wire ºIºÐ ¾c AE Blob °￡AC AO¼O °A¸® °e≫e
		int LeftDist_Wire = 0, RightDist_Wire = 0;
		POINT vWireLeftRst;
		POINT vWireRightRst;

		vWireLeftRst.x = 0;
		vWireLeftRst.y = 0;
		vWireRightRst.x = 0;
		vWireRightRst.y = 0;

		bool FindLeftBlobObj_Wire = false, FindRightBlobObj_Wire = false;
		int BlobMinDist_Wire = WireCandiBlobRst.cols - 1;

		POINT LeftPoint_Wire;
		POINT RightPoint_Wire;
		LeftPoint_Wire.x = 0;
		LeftPoint_Wire.y = 0;
		RightPoint_Wire.x = 0;
		RightPoint_Wire.y = 0;

		for (int y = 0; y < WireCandiBlobRst.rows; y++)
		{
			UCHAR* ptrWireCandiBlobRst = WireCandiBlobRst.ptr(y);

			for (int x = CenterPo.x - FootRoImgMargin.x; x > 0; x--)
			{
				if (ptrWireCandiBlobRst[x] == 255)
				{
					LeftPoint_Wire.x = x;
					LeftPoint_Wire.y = y;
					FindLeftBlobObj_Wire = true;
					break;
				}
				else
					LeftDist_Wire++;
			}

			for (int x = CenterPo.x - FootRoImgMargin.x; x < WireCandiBlobRst.cols - 1; x++)
			{
				if (ptrWireCandiBlobRst[x] == 255)
				{
					RightPoint_Wire.x = x;
					RightPoint_Wire.y = y;
					FindRightBlobObj_Wire = true;
					break;
				}
				else
					RightDist_Wire++;

			}

			if (FindLeftBlobObj_Wire == true && FindRightBlobObj_Wire == true)
			{
				if (LeftDist_Wire != 0 && RightDist_Wire != 0)
				{
					if (BlobMinDist_Wire > LeftDist_Wire + RightDist_Wire &&
						(LeftDist_Wire + RightDist_Wire) <= WireCandiBlobRst.cols - 10)
					{
						BlobMinDist_Wire = LeftDist_Wire + RightDist_Wire;

						vWireLeftRst = LeftPoint_Wire;
						vWireRightRst = RightPoint_Wire;
					}
				}

			}
			else
			{


			}

			FindLeftBlobObj_Wire = false;
			FindRightBlobObj_Wire = false;
			LeftDist_Wire = 0;
			RightDist_Wire = 0;

		}

		WireStartPosY = vWireLeftRst.y;

		if (vWireLeftRst.y != 0)
		{
			FindWirePos = true;

			pLEd->x = vWireLeftRst.x;
			pLEd->y = vWireLeftRst.y;
			pREd->x = vWireRightRst.x;
			pREd->y = vWireRightRst.y;
		}
		else
			FindWirePos = false;

		if (FindTailPos == true && FindWirePos == true)
			Ret = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::FindCandidate_TailAndWire_SobelXY() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}

bool CPInsp_Wedge::getFootBondingStEndPoint2(cv::Mat* vImage, cv::Mat* fHRectImg, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, int CntX, int CntY, int isSt)
{
	cv::Mat Intg, Dev;

	bool flag = false;
	bool bIsDetected_BondingSt = true, bIsDetected_BondingEnd = true;

	int nLine = __LINE__;
	try
	{
		cv::Mat localImg = vImage->clone();
#if _DEBUG
		cv::Mat localImgW = vImage->clone();
		cv::Mat localImgH = fHRectImg->clone();
		cv::imwrite("D:\\FootRst\\getFootBondingStEndPoint_INPUT_Img.bmp", localImgW);
		cv::imwrite("D:\\FootRst\\getFootBondingStEndPoint_INPUT_Img_H.bmp", localImgH);
#endif
		localImg &= 1;

		std::vector<int> stPoint(localImg.cols);
		std::vector<UCHAR*> stptr(localImg.rows);
		std::vector<int> edPoint(localImg.cols);
		//std::vector<UCHAR*> Wstptr(localImgW.rows);

		for (int r = 0; r < localImg.rows; r++)
		{
			stptr[r] = localImg.ptr(r);
			//Wstptr[r] = localImgW.ptr(r);
		}

		for (int c = 0; c < localImg.cols; c++)
		{
			stPoint[c] = -1;
			for (int r = 0; r < stptr.size(); r++)
			{
				if (stptr[r][c] > 0)
				{
					stPoint[c] = r - 10;
					break;
				}
			}
		}

		int calcSz = 10;
		std::vector<float> nMinIdx(localImg.cols);

		int nErrIdx = 160;

		for (int c = 0; c < stPoint.size(); c++)
		{
			nMinIdx[c] = -1;
			if (stPoint[c] < 0)
				continue;
			int nTmpMinIdx = -1;
			float fMinVal = 0;
			for (int r = stPoint[c]; r < CntY; r++)
			{
				float fTmpMinVal = 0;
				float* fStptr = fHRectImg->ptr<float>(r);
				float fMinH = fStptr[c];
				for (int i = 0; i < calcSz; i++)
				{
					//float* fStptr = fHRectImg->ptr<float>(r + i);
					float* fEdptr = fHRectImg->ptr<float>(r + i + 1);

					if (fEdptr[c] < fMinH)
					{
						fMinH = fEdptr[c];
					}
				}
				fTmpMinVal = fMinH - fStptr[c];

				if (fMinVal > fTmpMinVal)
				{
					fMinVal = fTmpMinVal;
					nTmpMinIdx = r;
				}
			}

			nMinIdx[c] = nTmpMinIdx;
		}

		int nSumLeftX(0), nSumRightX(0), nSumLeftY(0), nSumRightY(0), nCntLeft(0), nCntRight(0);
		int nLeftX(0), nRightX(0), nLeftY(0), nRightY(0);
		std::vector<cv::Point> AvgC;
		std::vector<cv::Point> RanC;
		AvgC.clear();
		RanC.clear();
		for (int c = 0; c < CntX; c++)
		{
			if (nMinIdx[c] != -1)
			{
				cv::Point WhitPos;
				WhitPos.x = c;
				WhitPos.y = nMinIdx[c];
				AvgC.push_back(WhitPos);
			}
		}
		RANSAC_1D_All(AvgC, &RanC, 3);
		double dRandScore = (double)RanC.size() / AvgC.size();
		if (dRandScore < 0.5)
			bIsDetected_BondingSt = false;
		if (RanC.size() > 0)
		{
			for (int i = 0; i < RanC.size(); i++)
			{
				nSumLeftX += RanC[i].x;
				nSumLeftY += RanC[i].y;
			}
			//nLeftX = nSumLeftX / RanC.size();
			//nLeftY = nSumLeftY / RanC.size();
			nLeftX = RanC[RanC.size() - 1].x;
			nLeftY = RanC[RanC.size() - 1].y;
			nCntLeft = RanC.size();
		}

		AvgC.clear();
		RanC.clear();
		for (int c = CntX; c < nMinIdx.size(); c++)
		{
			if (nMinIdx[c] != -1)
			{
				cv::Point WhitPos;
				WhitPos.x = c;
				WhitPos.y = nMinIdx[c];
				AvgC.push_back(WhitPos);
			}
		}
		RANSAC_1D_All(AvgC, &RanC, 3);
		dRandScore = (double)RanC.size() / AvgC.size();
		if (dRandScore < 0.5)
			bIsDetected_BondingSt = false;
		if (RanC.size() > 0)
		{
			for (int i = 0; i < RanC.size(); i++)
			{
				nSumRightX += RanC[i].x;
				nSumRightY += RanC[i].y;
			}
			//nRightX = nSumRightX / RanC.size();
			//nRightY = nSumRightY / RanC.size();
			nRightX = RanC[0].x;
			nRightY = RanC[0].y;
			nCntRight = RanC.size();
		}

		if (nCntLeft == 0)
		{
			nLeftX = nRightX;
			nLeftY = nRightY;
		}

		if (nCntRight == 0)
		{
			nRightX = nLeftX;
			nRightY = nLeftY;
		}

		UCHAR* Leftptr = localImg.ptr(nLeftY);
		UCHAR* Rightptr = localImg.ptr(nRightY);
		for (int c = CntX; c > 0; c--)
		{
			if (Leftptr[c] > 0)
			{
				nLeftX = c;
				break;
			}
		}
		for (int c = CntX; c < localImg.cols; c++)
		{
			if (Rightptr[c] > 0)
			{
				nRightX = c;
				break;
			}
		}

		pLEd->x = pLSt->x = nLeftX;
		pLEd->y = pLSt->y = nLeftY;
		pREd->x = pRSt->x = nRightX;
		pREd->y = pRSt->y = nRightY;

#pragma region Inspection endPoint

		for (int c = 0; c < localImg.cols; c++)
		{
			edPoint[c] = -1;
			for (int r = stptr.size() - 1; r > 0; r--)
			{
				if (stptr[r][c] > 0)
				{
					edPoint[c] = r - 10;
					break;
				}
			}
		}

		std::vector<float> nMaxIdx(localImg.cols);

		for (int c = 0; c < edPoint.size(); c++)
		{
			nMaxIdx[c] = -1;
			if (edPoint[c] < 0)
				continue;
			int nTmpMaxIdx = -1;
			float fMaxVal = 0;
			for (int r = edPoint[c]; r < fHRectImg->rows - 11; r++)
			{
				float fTmpMaxVal = 0;
				float* fStptr = fHRectImg->ptr<float>(r);
				float fMaxH = fStptr[c];
				for (int i = 0; i < calcSz; i++)
				{
					//float* fStptr = fHRectImg->ptr<float>(r + i);
					float* fEdptr = fHRectImg->ptr<float>(r + i + 1);

					if (fEdptr[c] > fMaxH)
					{
						fMaxH = fEdptr[c];
					}
				}
				fTmpMaxVal = fMaxH - fStptr[c];

				if (fMaxVal < fTmpMaxVal)
				{
					fMaxVal = fTmpMaxVal;
					nTmpMaxIdx = r;
				}
			}

			nMaxIdx[c] = nTmpMaxIdx;
		}
		/*
				int nSumLeftX(0), nSumRightX(0), nSumLeftY(0), nSumRightY(0), nCntLeft(0), nCntRight(0);
				int nLeftX(0), nRightX(0), nLeftY(0), nRightY(0);*/
		nCntLeft = nCntRight = 0;
		nSumLeftX = nSumRightX = 0;
		nSumLeftY = nSumRightY = 0;

		AvgC.clear();
		RanC.clear();
		for (int c = 0; c < CntX; c++)
		{
			if (nMaxIdx[c] != -1)
			{
				cv::Point WhitPos;
				WhitPos.x = c;
				WhitPos.y = nMaxIdx[c];
				AvgC.push_back(WhitPos);
			}
		}
		RANSAC_1D_All(AvgC, &RanC, 3);
		dRandScore = (double)RanC.size() / AvgC.size();
		if (dRandScore < 0.5)
			bIsDetected_BondingEnd = false;
		if (RanC.size() > 0)
		{
			for (int i = 0; i < RanC.size(); i++)
			{
				nSumLeftX += RanC[i].x;
				nSumLeftY += RanC[i].y;
			}
			//nLeftX = nSumLeftX / RanC.size();
			//nLeftY = nSumLeftY / RanC.size();
			nLeftX = RanC[RanC.size() - 1].x;
			nLeftY = RanC[RanC.size() - 1].y;
			nCntLeft = RanC.size();
		}

		AvgC.clear();
		RanC.clear();
		for (int c = CntX; c < nMaxIdx.size(); c++)
		{
			if (nMaxIdx[c] != -1)
			{
				cv::Point WhitPos;
				WhitPos.x = c;
				WhitPos.y = nMaxIdx[c];
				AvgC.push_back(WhitPos);
			}
		}
		RANSAC_1D_All(AvgC, &RanC, 3);
		dRandScore = (double)RanC.size() / AvgC.size();
		if (dRandScore < 0.5)
			bIsDetected_BondingEnd = false;
		if (RanC.size() > 0)
		{
			for (int i = 0; i < RanC.size(); i++)
			{
				nSumRightX += RanC[i].x;
				nSumRightY += RanC[i].y;
			}
			//nRightX = nSumRightX / RanC.size();
			//nRightY = nSumRightY / RanC.size();
			nRightX = RanC[0].x;
			nRightY = RanC[0].y;
			nCntRight = RanC.size();
		}

		if (nCntLeft == 0)
		{
			nLeftX = nRightX;
			nLeftY = nRightY;
		}

		if (nCntRight == 0)
		{
			nRightX = nLeftX;
			nRightY = nLeftY;
		}
		for (int c = CntX; c > 0; c--)
		{
			if (Leftptr[c] > 0)
			{
				nLeftX = c;
				break;
			}
		}
		for (int c = CntX; c < localImg.cols; c++)
		{
			if (Rightptr[c] > 0)
			{
				nRightX = c;
				break;
			}
		}
#pragma endregion

		pLEd->x = nLeftX;
		pLEd->y = nLeftY;
		pREd->x = nRightX;
		pREd->y = nRightY;

		if ((isSt == 0 && bIsDetected_BondingSt) //start ok ->ok
			|| (isSt == 1 && bIsDetected_BondingEnd) //end ok ->ok
			|| (isSt == 2 && bIsDetected_BondingSt && bIsDetected_BondingEnd))//start end ok
		{
			flag = true;

#if _DEBUG
			std::vector<cv::Point> vPoSt(2);
			std::vector<cv::Point> vPoEd(2);

			vPoSt[0] = *pLSt;
			vPoSt[1] = *pRSt;
			vPoEd[0] = *pLEd;
			vPoEd[1] = *pREd;

			ImagePointSave(_T("D:\\FootRst\\getFootBondingStEndPoint_WingLength.bmp"), &localImg, vPoSt, vPoEd);
#endif

		}

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::getFootBondingStEndPoint2() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return flag;
}

bool CPInsp_Wedge::FindCandidate_Tail_v2(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat* cvFootImg, bool bFindWedgeCandidates, cv::Mat WedgeCandidateImg, cv::Point& CenterPo, cv::Point& FootRoImgMargin, int &TailEndPosY, int &WireStartPosY, int &WedgeEndPosY)
{
	bool Ret = false;
	TailEndPosY = 0;

	int nLine = __LINE__;
	try
	{
		//Sobel Edge Detect
		cv::Mat Img3D = fHRoImg.clone();
		cv::Mat Img3D_Candidates = Img3D.clone();

		cv::Mat gradX, gradY;
		cv::Mat gradX_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);
		cv::Mat gradY_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);

		cv::Sobel(Img3D_Candidates, gradX, CV_32FC1, 1, 0, 3);
		cv::Sobel(Img3D_Candidates, gradY, CV_32FC1, 0, 1, 3);
		cv::convertScaleAbs(gradX, gradX_ConvertImg, 1, 0);	//A½AC º?E­μμ¿¡ ´eCN A¤º¸ A?Ao¸| A§CØ μ￥AIAI A¸AOA≫ CV_16S·I AoA¤CN μU, ´U½A CV_8U·I º?E?
		cv::convertScaleAbs(gradY, gradY_ConvertImg, 1, 0);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelX_Img.bmp", gradX_ConvertImg);
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelY_Img.bmp", gradY_ConvertImg);
#endif

		//SobelX Gradient Binarization Using min-max Range
		cv::Mat cvFootSobelXResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelXResultImg.setTo(0);
		cv::Mat cvFootSobelXResultImg_Wire(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelXResultImg_Wire.setTo(0);
		for (int y = 0; y < gradX_ConvertImg.rows; y++)
		{
			uchar * ptrSobelX = gradX_ConvertImg.data;
			uchar * ptrSobelXRes = cvFootSobelXResultImg.data;
			uchar * ptrSobelXRes_Wire = cvFootSobelXResultImg_Wire.data;
			uchar * ptrFootBW = cvFootImg->data;

			for (int x = 0; x < gradX_ConvertImg.cols; x++)
			{
				int val = ptrSobelX[y * gradX_ConvertImg.step + x];
				if (ptrFootBW[y * gradX_ConvertImg.step + x] != 0)
				{
					if (val > pFoot->nFootSobmin && val < pFoot->nFootSobmax)
					{
						ptrSobelXRes[y * gradX_ConvertImg.step + x] = 255;
					}
				}

			}
		}


#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelXBW_Img.bmp", cvFootSobelXResultImg);
#endif

		cv::Mat cvFootSobelYAllResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelYAllResultImg.setTo(0);
		cv::Mat cvFootSobelYResultImg(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelYResultImg.setTo(0);
		cv::Mat cvFootSobelYResultImg_Wire(Img3D.rows, Img3D.cols, CV_8UC1);
		cvFootSobelYResultImg_Wire.setTo(0);
		for (int y = 0; y < cvFootSobelYResultImg.rows; y++)
		{
			UCHAR* ptrSobelY = gradY_ConvertImg.ptr(y);
			UCHAR* ptrSobelYRes = cvFootSobelYResultImg.ptr(y);
			UCHAR* ptrSobelYRes_Wire = cvFootSobelYResultImg_Wire.ptr(y);
			UCHAR* ptrSobelYResAll_Wire = cvFootSobelYAllResultImg.ptr(y);
			UCHAR* ptrFootBW = cvFootImg->ptr(y);

			for (int x = 0; x < cvFootSobelYResultImg.cols; x++)
			{
				int val = ptrSobelY[x];

				if (ptrFootBW[x] != 0 && y < CenterPo.y)
				{
					if (ptrSobelY[x] > 10 && ptrSobelY[x] < 150)
					{
						ptrSobelYRes[x] = 255;
					}
					ptrSobelYRes_Wire[x] = 0;
				}
				if (ptrFootBW[x] != 0 && y > CenterPo.y)
				{
					ptrSobelYRes[x] = 0;
					if (ptrSobelY[x] > 10 && ptrSobelY[x] < 150)
					{
						ptrSobelYRes_Wire[x] = 255;
					}
				}

				if (ptrFootBW[x] != 0 && ptrSobelY[x] > 10 && ptrSobelY[x] < 150)
					ptrSobelYResAll_Wire[x] = 255;
			}
		}


#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailAndWire_SobelYBW_Img.bmp", cvFootSobelYAllResultImg);
#endif

		cv::Mat SobelXAndY = cvFootSobelXResultImg & cvFootSobelYResultImg;
		cv::Mat SobelXAndY_Wire = cvFootSobelXResultImg & cvFootSobelYResultImg_Wire;

		//Sobel X + Sobel Y ¿μ≫o CO≫e EA, ¿U°u ¶oAI AßAa - Tail ºIºÐ°u Wire ºIºÐAC ¿¡Ao °­μμ°¡ A≪°O E®AIμE
		cv::Mat AddSobelXAndY = gradX_ConvertImg + gradY_ConvertImg + gradY_ConvertImg;
		cv::Mat canny3dfoot_img = Img3D.clone();
		for (int y = 0; y < canny3dfoot_img.rows; y++)
		{
			float* ptrInputImg = canny3dfoot_img.ptr<float>(y);
			UCHAR* ptrFootBW = cvFootImg->ptr(y);
			UCHAR* ptrAddSobelXAndY = AddSobelXAndY.ptr(y);

			for (int x = 0; x < canny3dfoot_img.cols; x++)
			{
				int val = ptrInputImg[x];

				if (ptrFootBW[x] == 0)
				{
					ptrInputImg[x] = 0;
					ptrAddSobelXAndY[x] = 0;
				}
			}
		}

		//¿ⓒ±a¼­ ºIAI Foot ¼¼·IAaA¸·I ³×CE¼¿ ¾Æ·¡±iAoAC CI°­CI´A AO´e °­μμ A￡±a
		std::vector<int> DecreaseHeightPointY;
		cv::Mat Foot3DImg = canny3dfoot_img.clone();
		Foot3DImg.setTo(0);

		cv::Mat Foot3DMaxDiffImg = canny3dfoot_img.clone();
		Foot3DMaxDiffImg.setTo(0);

		cv::Mat Foot3DDiffImg_Top10 = canny3dfoot_img.clone();
		Foot3DDiffImg_Top10.setTo(0);

		cv::Mat Foot3DDiffImg_Top10_2D(canny3dfoot_img.rows, canny3dfoot_img.cols, CV_8UC1);
		Foot3DDiffImg_Top10_2D.setTo(0);

		int nDecreaseHgt_Max = 0;

		size_t floatstep_3D = canny3dfoot_img.step / sizeof(float);


		for (int x = 0; x < Foot3DImg.cols; x++)		//xAa
		{
			//float* ptFoot3DImg = canny3dfoot_img.data;
			float* ptFoot3DImg = (float*)canny3dfoot_img.data;
			float* ptFoot3D_RstImg = (float*)Foot3DImg.data;
			float* ptFoot3D_RstImg_MaxDiff = (float*)Foot3DMaxDiffImg.data;
			float* ptFoot3D_RstImg_TOP10 = (float*)Foot3DDiffImg_Top10.data;
			UCHAR* ptFoot3D_RstImg_TOP10_8bit = Foot3DDiffImg_Top10_2D.data;

			int maxDiff = 0;
			POINT MaxDiffPos;
			MaxDiffPos.x = 0;
			MaxDiffPos.y = 0;

			std::vector<POINT> vTailLinePoint;
			std::vector<int> vTailLine_DiffValue;

			// 			struct TailHgtDiffList 
			// 			{
			// 				POINT vTailLinePoint_1;
			// 				int vTailLine_DiffValue_1;
			// 			};
			std::vector<TailHgtDiffList> vTailLine_ListTop10;

			for (int y = 0; y < CenterPo.y; y++)		//y
			{
				float val_1 = ptFoot3DImg[x + floatstep_3D * y];
				float val_2 = ptFoot3DImg[x + floatstep_3D * (y + 1)];
				float val_3 = ptFoot3DImg[x + floatstep_3D * (y + 2)];
				float val_4 = ptFoot3DImg[x + floatstep_3D * (y + 3)];
				float val_5 = ptFoot3DImg[x + floatstep_3D * (y + 4)];

				int nDiffval = 0;
				POINT poDiffPos;
				poDiffPos.x = 0;
				poDiffPos.y = 0;

				if (ptFoot3DImg[x + floatstep_3D * y] < 150)
				{
					ptFoot3D_RstImg[x + floatstep_3D * y] = 0;
				}
				else
				{
					ptFoot3D_RstImg[x + floatstep_3D * y] = /*val_1+ */(val_1 - val_2)*2.f + (val_1 - val_3)*3.f +
						(val_1 - val_4)*4.f; //+ (val_1 - val_5);		//4CE¼¿±iAo CE¼¿ A÷AI CO≫e


					if (ptFoot3D_RstImg[x + floatstep_3D * y] < 0)
						ptFoot3D_RstImg[x + floatstep_3D * y] = 0;
					else
					{
						nDiffval = ptFoot3D_RstImg[x + floatstep_3D * y];
						poDiffPos.x = x;
						poDiffPos.y = y;

						vTailLinePoint.push_back(poDiffPos);
						vTailLine_DiffValue.push_back(nDiffval);

						TailHgtDiffList tailList;
						tailList.vTailLine_DiffValue_1 = nDiffval;
						tailList.vTailLinePoint_1 = poDiffPos;
						vTailLine_ListTop10.push_back(tailList);
					}

					if (maxDiff < ptFoot3D_RstImg[x + floatstep_3D * y])
					{
						maxDiff = ptFoot3D_RstImg[x + floatstep_3D * y];
						MaxDiffPos.x = x;
						MaxDiffPos.y = y;
					}


				}
			}

			ptFoot3D_RstImg_MaxDiff[MaxDiffPos.x + floatstep_3D * MaxDiffPos.y] = maxDiff;

			//≫oA§ 10°³ Point AßAa - A¤·A ¹× 10¹øA°±iAo AI¹IAo¿¡ Aa·A
			if (vTailLine_ListTop10.size() > 0)
			{
				sort(vTailLine_ListTop10.begin(), vTailLine_ListTop10.end());//, greater<int>());

				int count = 0;
				vector<TailHgtDiffList>::iterator ptr;
				for (ptr = vTailLine_ListTop10.begin(); ptr != vTailLine_ListTop10.end(); ++ptr)
				{
					count++;

					if (count > 5)
						break;

					ptFoot3D_RstImg_TOP10[ptr->vTailLinePoint_1.x + floatstep_3D * ptr->vTailLinePoint_1.y] = ptr->vTailLine_DiffValue_1;
					ptFoot3D_RstImg_TOP10_8bit[ptr->vTailLinePoint_1.x + Foot3DDiffImg_Top10_2D.step * ptr->vTailLinePoint_1.y] = 255;
				}
				vTailLine_ListTop10.clear();
			}

			if (vTailLine_DiffValue.size() > 0)
			{
				sort(vTailLine_DiffValue.begin(), vTailLine_DiffValue.end(), greater<int>());
				vTailLine_DiffValue.clear();
			}

		}

		cv::Mat dstImg;
		cv::Mat clrImg[3] = { WedgeCandidateImg, cv::Mat::zeros(cv::Size(Foot3DDiffImg_Top10_2D.cols, Foot3DDiffImg_Top10_2D.rows), Foot3DDiffImg_Top10_2D.type()),  Foot3DDiffImg_Top10_2D };
		cv::merge(clrImg, 3, dstImg);

#if _DEBUG
		cv::imwrite("D:\\FootRst\\FindCandidate_TailV2_Top5AndWedge_Img.bmp", dstImg);
#endif

		//cv::Mat Foot3DDiffImg_Top10_2D_removeWedge = Foot3DDiffImg_Top10_2D - WedgeCandidateImg;
		//Foot3DDiffImg_Top10_2D_removeWedge = Foot3DDiffImg_Top10_2D_removeWedge & cvFootSobelXResultImg;
		//Foot 3D AC ¼¼·I¹æCa ³oAI º?E­°¡ ¶³¾iAo´A ºIºÐ¸¸ AßAa
		cv::Mat FootHgtDecreasePoint(Foot3DImg.rows, Foot3DImg.cols, CV_32FC1);
		FootHgtDecreasePoint.setTo(0);

		cv::Mat RstTailImg(Foot3DImg.rows, Foot3DImg.cols, CV_32FC1);
		for (int x = 0; x < Foot3DImg.cols; x++)		//xAa
		{
			float* ptFoot3DImg = (float*)canny3dfoot_img.data;
			float* ptFoot3D_RstImg_TOP10 = (float*)Foot3DDiffImg_Top10.data;
			float* ptFoot3D_HgtDecreasePoint = (float*)FootHgtDecreasePoint.data;

			for (int y = 0; y < CenterPo.y; y++)		//y
			{
				float val_1 = ptFoot3DImg[x + floatstep_3D * y];
				float val_2 = ptFoot3DImg[x + floatstep_3D * (y + 1)];
				float val_3 = ptFoot3DImg[x + floatstep_3D * (y + 2)];

				if (ptFoot3DImg[x + floatstep_3D * y] == 0)
				{
					ptFoot3D_HgtDecreasePoint[x + floatstep_3D * y] = 0;
				}
				else
				{
					if (val_1 - val_2 > 0 && val_2 - val_3 > 0)
					{
						ptFoot3D_HgtDecreasePoint[x + floatstep_3D * y] = 255;
						//ptFoot3D_RstImg_TOP10[x + floatstep_3D * y] = 255;
					}
				}
			}
		}


		cv::Mat gradX_TailLine, gradY_TailLine;
		cv::Mat gradXTailLine_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);
		cv::Mat gradYTailLine_ConvertImg(Img3D.rows, Img3D.cols, CV_16SC1);
		// 
		cv::Sobel(Foot3DImg, gradX_TailLine, CV_32FC1, 1, 0, 3);
		cv::Sobel(Foot3DImg, gradY_TailLine, CV_32FC1, 0, 1, 3);
		cv::convertScaleAbs(gradX_TailLine, gradXTailLine_ConvertImg, 1, 0);	//A½AC º?E­μμ¿¡ ´eCN A¤º¸ A?Ao¸| A§CØ μ￥AIAI A¸AOA≫ CV_16S·I AoA¤CN μU, ´U½A CV_8U·I º?E?
		cv::convertScaleAbs(gradY_TailLine, gradYTailLine_ConvertImg, 1, 0);

		cv::Mat Foot3D_Diff_Norm;
		cv::normalize(Foot3DImg, Foot3D_Diff_Norm, 0, 255, cv::NORM_MINMAX);

		cv::Mat Foot3D_Diff_8Bit;
		Foot3D_Diff_8Bit.setTo(0);
		Foot3DImg.convertTo(Foot3D_Diff_8Bit, CV_8UC1);

		cv::Mat Foot3D_Diff_Norm_8Bit;
		Foot3D_Diff_Norm_8Bit.setTo(0);
		Foot3D_Diff_Norm.convertTo(Foot3D_Diff_Norm_8Bit, CV_8UC1);

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::FindCandidate_TailAndWire_SobelXY() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}


bool CPInsp_Wedge::ExtractLeftAndRightAreaOfFoot(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat roFootBWImg, cv::Mat WedgeCandidatesImg, cv::Mat WingCandidatesImg, cv::Mat *roFootBWRemovedFlowDown, cv::Mat* roFootLeftAreaBWImg, cv::Mat* roFootLeftAreafHImg, cv::Mat* roFootRightAreaBWImg, cv::Mat* roFootRightAreafHImg, CRect  *rcArrClipedLeftFoot, CRect *rcArrClipedRightFoot, cv::Point& CenterPo, cv::Point& Margin)
{
	bool Ret = false;

	int nLine = __LINE__;

	try
	{
		//Wing °a°u¿¡¼­ Wire ºIºÐ A|°ACI±a A§CN AU¾÷
		//1. Foot 3D AI¹IAo¿¡¼­ Ee·?³≫¸° ¿μ¿ª(40AICI) A|°A
		cv::Mat CutImg = WingCandidatesImg.clone();
		cv::Mat WedgeCandClipImg = WedgeCandidatesImg.clone();

		cv::Mat Foot2D_img = roFootBWImg.clone();
		cv::Mat foot3D_img = fHRoImg.clone();

		//Ee·?³≫¸° ¿μ¿ª A|°A
		for (int y = 0; y < foot3D_img.rows; y++)
		{
			float* ptrInputImg = foot3D_img.ptr<float>(y);
			UCHAR* ptrWedgeCandImg = WedgeCandClipImg.ptr(y);
			UCHAR* ptrFootBW = Foot2D_img.ptr(y);
			UCHAR* ptrFootBW_Rst = Foot2D_img.ptr(y);

			for (int x = 0; x < foot3D_img.cols; x++)
			{
				if (ptrFootBW[x] == 0)
				{
					ptrInputImg[x] = 0;
				}

				if (ptrInputImg[x] < 40)
				{
					ptrInputImg[x] = 0;
					ptrFootBW_Rst[x] = 0;
				}

			}
		}

		//Foot Blob 1°³ AI≫oAI¸e, Max Blob ¸¸ ≫Iμμ·I Aß°¡ (Wing ¿·A¸·I Ee·?³≫¸° ¿μ¿ªAI BlobA¸·I AaE÷´A °æ¿i°¡ AO¾i °E≫c¿¡ AoAa)
		double dArea, dCX, dCY;
		CRect rcBlob;
		AlgoBlob algoBlob;

		algoBlob.m_bUseIPC = false;
		algoBlob.m_bInvertCheck = false;
		algoBlob.m_bFilterIsUse = false;
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

		algoBlob.m_bInsp3D = false;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_dHeightRateMin = 0;
		algoBlob.m_dHeightRateMax = 0;

		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 0;
		algoBlob.m_nMaxBinary = 100;
		algoBlob.m_bFillHole = true;

		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;

		cv::Mat BinImage(Foot2D_img.rows, Foot2D_img.cols, CV_8UC1);

		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
		int nCntBlob = CPInsp::BlobImageStruct(algoBlob, Foot2D_img.data, foot3D_img.ptr<float>(0), NULL, Foot2D_img.cols, Foot2D_img.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);

		Foot2D_img = BinImage.clone();

		Foot2D_img.copyTo(*roFootBWRemovedFlowDown);

		for (int r = 0; r < foot3D_img.rows; r++)
		{

			float* fHightPtr = foot3D_img.ptr<float>(r);
			UCHAR* ptrFootBW = Foot2D_img.ptr(r);

			for (int c = 0; c < foot3D_img.cols; c++)
			{
				if (ptrFootBW[c] == 0)
				{
					fHightPtr[c] = 0;
				}
			}
		}

		//FootAC AO¿U°u CE¼¿·IºIAI ¾i´A A¤μμ ¹uA§(CoAc 15px) AßAaCO Ao °aA¤CI±a A§CN AU¾÷
		//Foot BW - Ee·?³≫¸°¿μ¿ª A|°A(40AICI) - Wedge EAº¸±º A|°ACN AOA¾ ¿μ≫o¿¡¼­ Aß½E±aAØ 1/3 ºIºÐ °¡·IÆo±O °e≫e °ªA¸·I °aA¤
		cv::Mat imgWingInput = Foot2D_img - WedgeCandClipImg;
		cv::Mat imgWingInput_Left = imgWingInput.clone();
		cv::Mat imgWingInput_Right = imgWingInput.clone();

		//Foot Aß½E xAa ±aAØA¸·I ¿Þ, ¿A¸￥AE ¿μ¿ª ±¸ºÐ
		for (int y = 0; y < imgWingInput.rows; y++)
		{
			UCHAR* ptrWingInputImg = imgWingInput.ptr(y);
			UCHAR* ptrWingInputImg_Left = imgWingInput_Left.ptr(y);
			UCHAR* ptrWingInputImg_Right = imgWingInput_Right.ptr(y);

			for (int x = 0; x < imgWingInput.cols; x++)
			{
				if (x > CenterPo.x - Margin.x)
				{
					ptrWingInputImg_Left[x] = 0;
				}

				if (x < CenterPo.x - Margin.x)
				{
					ptrWingInputImg_Right[x] = 0;
				}

			}
		}

		//¿Þ, ¿A¸￥AE Foot Blob AuA¼ ±æAIAC 1 / 3 ¸¸A­¸¸ Aß½EA¡A¸·I ºIAI °A¸® Æo±O °e≫e
		int ncvWingInputImgWidth_Left = imgWingInput_Left.cols;
		int ncvWingInputImgHeight_Left = imgWingInput_Left.rows;
		int nWingTmpMin_x_Left = ncvWingInputImgWidth_Left * ncvWingInputImgHeight_Left, nWingTmpMax_x_Left = 0;
		int nWingTmpMin_y_Left = ncvWingInputImgWidth_Left * ncvWingInputImgHeight_Left, nWingTmpMax_y_Left = 0;

		int ncvWingInputImgWidth_Right = imgWingInput_Right.cols;
		int ncvWingInputImgHeight_Right = imgWingInput_Right.rows;
		int nWingTmpMin_x_Right = ncvWingInputImgWidth_Right * ncvWingInputImgHeight_Right, nWingTmpMax_x_Right = 0;
		int nWingTmpMin_y_Right = ncvWingInputImgWidth_Right * ncvWingInputImgHeight_Right, nWingTmpMax_y_Right = 0;



		for (int r = 0; r < imgWingInput_Left.rows; r++)
		{
			UCHAR* Ptr_Left = imgWingInput_Left.data;
			UCHAR* Ptr_Right = imgWingInput_Right.data;

			for (int c = 0; c < imgWingInput_Left.cols; c++)
			{
				if (Ptr_Left[r*imgWingInput_Left.step + c] == 255)
				{
					if (c < nWingTmpMin_x_Left) nWingTmpMin_x_Left = c;
					if (c > nWingTmpMax_x_Left) nWingTmpMax_x_Left = c;
					if (r < nWingTmpMin_y_Left) nWingTmpMin_y_Left = r;
					if (r > nWingTmpMax_y_Left) nWingTmpMax_y_Left = r;
				}
			}

			for (int c = 0; c < imgWingInput_Right.cols; c++)
			{
				if (Ptr_Right[r*imgWingInput_Right.step + c] == 255)
				{
					if (c < nWingTmpMin_x_Right) nWingTmpMin_x_Right = c;
					if (c > nWingTmpMax_x_Right) nWingTmpMax_x_Right = c;
					if (r < nWingTmpMin_y_Right) nWingTmpMin_y_Right = r;
					if (r > nWingTmpMax_y_Right) nWingTmpMax_y_Right = r;
				}
			}
		}

		int min_y_calcRange_Left = nWingTmpMin_y_Left + 2 * (nWingTmpMax_y_Left - nWingTmpMin_y_Left) / 5;
		int max_y_calcRange_Left = min_y_calcRange_Left + (nWingTmpMax_y_Left - nWingTmpMin_y_Left) / 5;

		int min_y_calcRange_Right = nWingTmpMin_y_Right + 2 * (nWingTmpMax_y_Right - nWingTmpMin_y_Right) / 5;
		int max_y_calcRange_Right = min_y_calcRange_Right + (nWingTmpMax_y_Right - nWingTmpMin_y_Right) / 5;

		CRect rcLeftWingRefROI;
		rcLeftWingRefROI.left = nWingTmpMin_x_Left;
		rcLeftWingRefROI.top = min_y_calcRange_Left;
		rcLeftWingRefROI.right = nWingTmpMax_x_Left;
		rcLeftWingRefROI.bottom = max_y_calcRange_Left;

		CRect rcRightWingRefROI;
		rcRightWingRefROI.left = nWingTmpMin_x_Right;
		rcRightWingRefROI.top = min_y_calcRange_Right;
		rcRightWingRefROI.right = nWingTmpMax_x_Right;
		rcRightWingRefROI.bottom = max_y_calcRange_Right;

		*rcArrClipedLeftFoot = &rcLeftWingRefROI;
		*rcArrClipedRightFoot = &rcRightWingRefROI;

		int dist_Left = 0, dist_Right = 0;
		int sum_diff_Left = 0, sum_diff_Right = 0;
		for (int r = 0; r < imgWingInput_Left.rows; r++)
		{
			UCHAR* Ptr_Left = imgWingInput_Left.data;
			UCHAR* Ptr_Right = imgWingInput_Right.data;

			dist_Left = 0;
			dist_Right = 0;

			for (int c = 0; c < imgWingInput_Left.cols; c++)
			{
				if (min_y_calcRange_Left <= r && r <= max_y_calcRange_Left)
				{
					if (nWingTmpMin_x_Left <= c && c <= nWingTmpMax_x_Left)
					{
						if (Ptr_Left[r*imgWingInput_Left.step + c] == 255)
						{
							dist_Left++;
						}
					}
				}
			}
			sum_diff_Left += dist_Left;

			for (int c = 0; c < imgWingInput_Right.cols; c++)
			{
				if (min_y_calcRange_Right <= r && r <= max_y_calcRange_Right)
				{
					if (nWingTmpMin_x_Right <= c && c <= nWingTmpMax_x_Right)
					{
						if (Ptr_Right[r*imgWingInput_Right.step + c] == 255)
						{
							dist_Right++;
						}
					}
				}
			}
			sum_diff_Right += dist_Right;
		}

		double avg_WingWidth_Left = (double)sum_diff_Left / (double)((max_y_calcRange_Left - min_y_calcRange_Left) + 1);
		double avg_WingWidth_Right = (double)sum_diff_Right / (double)((max_y_calcRange_Right - min_y_calcRange_Right) + 1);

		int nRangeToExtractFootCountour_Left = (int)avg_WingWidth_Left;
		int nRangeToExtractFootCountour_Right = (int)avg_WingWidth_Right;


		cv::Mat imgFoot_LeftBW(CutImg.rows, CutImg.cols, CV_8UC1);
		cv::Mat imgFoot_RightBW(CutImg.rows, CutImg.cols, CV_8UC1);
		imgFoot_LeftBW.setTo(0);
		imgFoot_RightBW.setTo(0);

		//2. ¿ÞAE FootAC AO¿U°u CE¼¿·IºIAI AIA¤ ¹uA§(15px)¸¸A­¸¸ ≫eAa
		cv::Mat Left3DRange(CutImg.rows, CutImg.cols, CV_32FC1);
		Left3DRange.setTo(0);
		int LeftRangeCount = 0;

		size_t floatStep_Wing = foot3D_img.step / sizeof(float);
		bool bFlagScan = false;
		for (int r = 0; r < CutImg.rows; r++)
		{
			UCHAR* Ptr = Foot2D_img.data;
			UCHAR* ptr_LeftBW = imgFoot_LeftBW.data;
			float* ptrInputImg = (float*)foot3D_img.data;
			float* ptrLeftResultImg = (float*)Left3DRange.data;
			bFlagScan = false;
			LeftRangeCount = 0;
			for (int c = 0; c < CenterPo.x - Margin.x; c++)
			{
				if (LeftRangeCount == nRangeToExtractFootCountour_Left)
				{
					LeftRangeCount = 0;
					bFlagScan = false;
					break;
				}

				if (bFlagScan == false && Ptr[r*CutImg.step + c] == 255)
				{
					bFlagScan = true;
				}
				if (bFlagScan)
				{
					ptrLeftResultImg[r * floatStep_Wing + c] = ptrInputImg[r * floatStep_Wing + c];
					ptr_LeftBW[r * imgFoot_LeftBW.step + c] = 255;
					LeftRangeCount++;
				}
			}
		}

		//3. ¿A¸￥AE FootAC AO¿U°u CE¼¿·IºIAI AIA¤ ¹uA§(15px)¸¸A­¸¸ ≫eAa
		cv::Mat Right3DRange(CutImg.rows, CutImg.cols, CV_32FC1);
		Right3DRange.setTo(0);
		int RightRangeCount = 0;
		bool bFlagScan_right = false;
		for (int r = 0; r < CutImg.rows; r++)
		{
			UCHAR* Ptr = Foot2D_img.data;
			UCHAR* ptr_RightBW = imgFoot_RightBW.data;
			float* ptrInputImg = (float*)foot3D_img.data;
			float* ptrRightResultImg = (float*)Right3DRange.data;
			bFlagScan_right = false;
			RightRangeCount = 0;
			for (int c = CutImg.cols - 1; c >= CenterPo.x - Margin.x; c--)
			{
				if (RightRangeCount == nRangeToExtractFootCountour_Right)
				{
					RightRangeCount = 0;
					bFlagScan_right = false;
					break;
				}
				if (bFlagScan_right == false && Ptr[r*CutImg.step + c] == 255)
				{
					bFlagScan_right = true;
				}

				if (bFlagScan_right)
				{
					ptrRightResultImg[r * floatStep_Wing + c] = ptrInputImg[r * floatStep_Wing + c];
					ptr_RightBW[r * imgFoot_RightBW.step + c] = 255;
					RightRangeCount++;
				}
			}
		}

		Left3DRange.copyTo(*roFootLeftAreafHImg);
		Right3DRange.copyTo(*roFootRightAreafHImg);

		imgFoot_LeftBW.copyTo(*roFootLeftAreaBWImg);
		imgFoot_RightBW.copyTo(*roFootRightAreaBWImg);

		/*
		//3-2. ≫eAaCN ¿A¸￥AE/¿ÞAE foot ¿μ¿ª csv ÆAAI·I AuAa
		//¿ÞAE Foot AIA¤ ¿μ¿ª¸¸ 3d ³oAI μ￥AIAI AßAa
		{
			CRect rect;
			rect.left = Left3DRange.cols;
			rect.top = Foot2D_img.rows;
			rect.right = 0;
			rect.bottom = 0;

			for (size_t y = 0; y < Left3DRange.rows; y++)
			{
				float* ptrMat = Left3DRange.ptr<float>(y);
				uchar* ptr_Foot = Foot2D_img.ptr(y);

				for (size_t x = 0; x < Left3DRange.cols; x++)
				{
					if (ptrMat[x] != 0)
					{
						if (rect.left > x) rect.left = x;
						if (rect.right < x) rect.right = x;

						if (rect.bottom < y) rect.bottom = y;
					}
					if (ptr_Foot[x] != 0)
					{
						if (rect.top > y) rect.top = y;
					}
				}
			}

			bool isEmpty = false;
			if (rect.bottom == 0 || rect.right == 0 && rect.top == Left3DRange.rows || rect.left == Left3DRange.cols)
				isEmpty = true;

			if (isEmpty == false)
			{
				//3D ptr
				//float* Rot3D = new float[rect.Width() * rect.Height() * 4];
				float* Rot3D = g_pMManager->pem_new<float>(true, rect.Width() * rect.Height() * 4, (PCHAR)__FUNCTION__, __LINE__);
				for (size_t y = 0; y < rect.Height(); y++)
				{
					float* ptrMat = Left3DRange.ptr<float>(rect.top + y);
					float* ptr = &Rot3D[rect.Width()*y];

					for (size_t x = 0; x < rect.Width(); x++)
					{
						ptr[x] = ptrMat[rect.left + x];
					}

				}

				HEADER_PTR hPtr;
				hPtr.uiNumRow = rect.Width();
				hPtr.uiNumCol = rect.Height();
				hPtr.zResolX = 5.5f;
				hPtr.zResolY = 5.5f;
				hPtr.sizeBit = 32;
				alpf_save_ptr(_T("D:\\FootInspImg\\insp_Foot_3D_Left.ptr"), &hPtr, Rot3D);

#ifdef _AlgoTool
				//3D csv
				FILE* fp = fopen("D:\\FootInspImg\\insp_Foot_3D_Left.csv", "w+");

				for (size_t y = 0; y < rect.Height(); y++)
				{
					float* ptr = &Rot3D[rect.Width()*y];

					for (size_t x = 0; x < rect.Width(); x++)
					{
						float f = ptr[x];

						if (x < rect.Width() - 1)
						{
							fprintf(fp, "%f,", f);
						}
						else
						{
							fprintf(fp, "%f", f);
						}
					}

					if (y < rect.Height() - 1)
						fprintf(fp, "\n");
				}

				fclose(fp);
#endif

				//delete[] Rot3D;
				//Rot3D = nullptr;
				g_pMManager->pem_delete(Rot3D, true);
			}
		}

		//¿A¸￥AE Foot AIA¤ ¿μ¿ª¸¸ 3d ³oAI μ￥AIAI AßAa
		{
			CRect rect;
			rect.left = Right3DRange.cols;
			rect.top = Right3DRange.rows;
			rect.right = 0;
			rect.bottom = 0;

			for (size_t y = 0; y < Right3DRange.rows; y++)
			{
				float* ptrMat = Right3DRange.ptr<float>(y);
				uchar* ptr_Foot = Foot2D_img.ptr(y);

				for (size_t x = 0; x < Right3DRange.cols; x++)
				{
					if (ptrMat[x] != 0)
					{
						if (rect.left > x) rect.left = x;
						if (rect.right < x) rect.right = x;

						if (rect.bottom < y) rect.bottom = y;
					}
					if (ptr_Foot[x] != 0)
					{
						if (rect.top > y) rect.top = y;
					}
				}
			}

			bool isEmpty = false;
			if (rect.bottom == 0 || rect.right == 0 && rect.top == Right3DRange.rows || rect.left == Right3DRange.cols)
				isEmpty = true;

			if (isEmpty == false)
			{
				//3D ptr
				//float* Rot3D = new float[rect.Width() * rect.Height() * 4];
				float* Rot3D = g_pMManager->pem_new<float>(true, rect.Width() * rect.Height() * 4, (PCHAR)__FUNCTION__, __LINE__);
				for (size_t y = 0; y < rect.Height(); y++)
				{
					float* ptrMat = Right3DRange.ptr<float>(rect.top + y);
					float* ptr = &Rot3D[rect.Width()*y];

					for (size_t x = 0; x < rect.Width(); x++)
					{
						ptr[x] = ptrMat[rect.left + x];
					}

				}

				HEADER_PTR hPtr;
				hPtr.uiNumRow = rect.Width();
				hPtr.uiNumCol = rect.Height();
				hPtr.zResolX = 5.5f;
				hPtr.zResolY = 5.5f;
				hPtr.sizeBit = 32;
				alpf_save_ptr(_T("D:\\FootInspImg\\insp_Foot_3D_Right.ptr"), &hPtr, Rot3D);

				//3D csv
				FILE* fp = fopen("D:\\FootInspImg\\insp_Foot_3D_Right.csv", "w+");

				for (size_t y = 0; y < rect.Height(); y++)
				{
					float* ptr = &Rot3D[rect.Width()*y];

					for (size_t x = 0; x < rect.Width(); x++)
					{
						float f = ptr[x];

						if (x < rect.Width() - 1)
						{
							fprintf(fp, "%f,", f);
						}
						else
						{
							fprintf(fp, "%f", f);
						}
					}

					if (y < rect.Height() - 1)
						fprintf(fp, "\n");
				}

				fclose(fp);

// 				delete[] Rot3D;
// 				Rot3D = nullptr;
				g_pMManager->pem_delete(Rot3D, true);
			}
		}
		*/

		Ret = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::ExtractLeftAndRightAreaOfFoot() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}

bool CPInsp_Wedge::DetectWireStartPos_UseLeftAndRightAreasOfFootHeight(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat FootBWRemovedFlowDown, cv::Mat inputFootLeftfH, cv::Mat inputFootRightfH, CRect rcLeftFootRefROI, CRect rcRightFootRefROI, cv::Point& CenterPo, cv::Point& Margin, bool bUseMeanSlopeAccum, int *nDetectedWireStartPosY)
{
	bool Ret = false;

	int nLine = __LINE__;

	try
	{
#ifdef _AlgoTool
		FILE* fp_LeftFootMeanSlope_Accum = fopen("D:\\FootInspImg\\insp_LeftFootMeanSlope_Accum.csv", "w+");
		FILE* fp_RightFootMeanSlope_Accum = fopen("D:\\FootInspImg\\insp_RightFootMeanSlope_Accum.csv", "w+");

		FILE* fp_LeftFootStdDevSlope_Accum = fopen("D:\\FootInspImg\\insp_LeftFootStdDevSlope_Accum.csv", "w+");
		FILE* fp_RightFootStdDevSlope_Accum = fopen("D:\\FootInspImg\\insp_RightFootStdDevSlope_Accum.csv", "w+");
#endif
		//Foot Blob ¿μ¿ª °e≫e
		int ncvImgWidth = FootBWRemovedFlowDown.cols;
		int ncvImgHeight = FootBWRemovedFlowDown.rows;
		int nFootTmpMin_x = ncvImgWidth * ncvImgHeight, nFootTmpMax_x = 0;
		int nFootTmpMin_y = ncvImgWidth * ncvImgHeight, nFootTmpMax_y = 0;
		for (int r = 0; r < FootBWRemovedFlowDown.rows; r++)
		{
			UCHAR* Ptr = FootBWRemovedFlowDown.data;
			for (int c = 0; c < FootBWRemovedFlowDown.cols; c++)
			{
				if (Ptr[r*FootBWRemovedFlowDown.step + c] == 255)
				{
					if (c < nFootTmpMin_x) nFootTmpMin_x = c;
					if (c > nFootTmpMax_x) nFootTmpMax_x = c;
					if (r < nFootTmpMin_y) nFootTmpMin_y = r;
					if (r > nFootTmpMax_y) nFootTmpMax_y = r;
				}
			}
		}

		//foot BW °a°u ¾øA≫ ½A CO¼o ¸®AI
		if (nFootTmpMin_x == ncvImgWidth * ncvImgHeight && nFootTmpMin_y == ncvImgWidth * ncvImgHeight &&
			nFootTmpMax_x == 0 && nFootTmpMax_y == 0)
		{
			return false;
		}

		cv::Mat Left3DRange = inputFootLeftfH.clone();
		cv::Mat Right3DRange = inputFootRightfH.clone();

		//1. ³oAI data Line º°·I Æo±O, C￥AØÆiA÷ ±¸CI±a
		cv::Mat leftFootMeanVal_raw(Left3DRange.rows, 1, CV_32FC1);
		cv::Mat rightFootMeanVal_raw(Left3DRange.rows, 1, CV_32FC1);
		leftFootMeanVal_raw.setTo(0);
		rightFootMeanVal_raw.setTo(0);

		cv::Mat leftFootStdDevVal_raw(Left3DRange.rows, 1, CV_32FC1);
		cv::Mat rightFootStdDevVal_raw(Left3DRange.rows, 1, CV_32FC1);
		leftFootStdDevVal_raw.setTo(0);
		rightFootStdDevVal_raw.setTo(0);

		float meanVal_Left_raw = 0.0f, meanVal_Right_raw = 0.0f;
		float tmpMeanVal_Left_raw = 0.0f, tmpMeanVal_Right_raw = 0.0f;
		float fStdDevVal_Left_raw = 0.0f, fStdDevVal_Right_raw = 0.0f;
		float tmpfStdDevVal_Left_raw = 0.0f, tmpfStdDevVal_Right_raw = 0.0f;

		std::vector<int> vLeftFootPxCnt_raw;
		int nLeftFootPxCnt_raw = 0, nRightFootPxCnt_raw = 0;

		for (int y = nFootTmpMin_y; y < fHRoImg.rows; y++)
		{
			float* ptrLeftBlob = Left3DRange.ptr<float>(y);
			float* ptrRightBlob = Right3DRange.ptr<float>(y);

			float* ptrLeftMean_raw = leftFootMeanVal_raw.ptr<float>(y);
			float* ptrRightMean_raw = rightFootMeanVal_raw.ptr<float>(y);

			float* ptrLeftStdDev_raw = leftFootStdDevVal_raw.ptr<float>(y);
			float* ptrRightStdDev_raw = rightFootStdDevVal_raw.ptr<float>(y);

			for (int x = 1; x < fHRoImg.cols; x++)
			{
				if (x <= CenterPo.x - Margin.x && ptrLeftBlob[x] != 0.0f)
				{
					tmpMeanVal_Left_raw += ptrLeftBlob[x];
					nLeftFootPxCnt_raw++;

				}
				if (x >= CenterPo.x - Margin.x && ptrRightBlob[x] != 0.0f)
				{
					tmpMeanVal_Right_raw += ptrRightBlob[x];
					nRightFootPxCnt_raw++;
				}

			}

			//x¹æCa ³oAI Æo±O °e≫e
			vLeftFootPxCnt_raw.push_back(nLeftFootPxCnt_raw);
			ptrLeftMean_raw[0] = tmpMeanVal_Left_raw / (float)nLeftFootPxCnt_raw;
			ptrRightMean_raw[0] = tmpMeanVal_Right_raw / (float)nRightFootPxCnt_raw;
			if (isnan(ptrLeftMean_raw[0]))
				ptrLeftMean_raw[0] = 0;
			if (isnan(ptrRightMean_raw[0]))
				ptrRightMean_raw[0] = 0;

			for (int x = 1; x < fHRoImg.cols; x++)
			{
				if (x <= CenterPo.x - Margin.x && ptrLeftBlob[x] != 0.0f)
				{
					tmpfStdDevVal_Left_raw = ptrLeftMean_raw[0] - ptrLeftBlob[x];				//a' = Æo±O - μ￥AIAI°ª
					tmpfStdDevVal_Left_raw += tmpfStdDevVal_Left_raw * tmpfStdDevVal_Left_raw;		//a' A|°o

				}
				if (x >= CenterPo.x - Margin.x && ptrRightBlob[x] != 0.0f)
				{
					tmpfStdDevVal_Right_raw = ptrRightMean_raw[0] - ptrRightBlob[x];
					tmpfStdDevVal_Right_raw += tmpfStdDevVal_Right_raw * tmpfStdDevVal_Right_raw;
				}

			}

			//x¹æCa ³oAI C￥AØÆiA÷ °e≫e
			ptrLeftStdDev_raw[0] = (float)sqrtf(tmpfStdDevVal_Left_raw / (float)nLeftFootPxCnt_raw);
			ptrRightStdDev_raw[0] = (float)sqrtf(tmpfStdDevVal_Right_raw / (float)nRightFootPxCnt_raw);

			if (isnan(ptrLeftStdDev_raw[0]))
				ptrLeftStdDev_raw[0] = 0;
			if (isnan(ptrRightStdDev_raw[0]))
				ptrRightStdDev_raw[0] = 0;

			nLeftFootPxCnt_raw = 0;
			nRightFootPxCnt_raw = 0;
			tmpMeanVal_Left_raw = 0;
			tmpMeanVal_Right_raw = 0;
			tmpfStdDevVal_Left_raw = 0;
			tmpfStdDevVal_Right_raw = 0;

#ifdef _AlgoTool
			fprintf(fp_LeftFootMeanSlope_Accum, "%f,", ptrLeftMean_raw[0]);
			fprintf(fp_RightFootMeanSlope_Accum, "%f,", ptrRightMean_raw[0]);
			fprintf(fp_LeftFootStdDevSlope_Accum, "%f,", ptrLeftStdDev_raw[0]);
			fprintf(fp_RightFootStdDevSlope_Accum, "%f,", ptrRightStdDev_raw[0]);
#endif
		}

#ifdef _AlgoTool
		fprintf(fp_LeftFootMeanSlope_Accum, "\n");
		fprintf(fp_RightFootMeanSlope_Accum, "\n");
		fprintf(fp_LeftFootStdDevSlope_Accum, "\n");
		fprintf(fp_RightFootStdDevSlope_Accum, "\n");
#endif
		//2. Æo±O CEAI Au¿e - 5°³ ¾¿ Æo±O °e≫e EA Au¿e

		cv::Mat leftFootMeanVal_filtering(Left3DRange.rows, 1, CV_32FC1);
		cv::Mat rightFootMeanVal_filtering(Right3DRange.rows, 1, CV_32FC1);

		cv::Mat leftFootStdDevVal_filtering(Left3DRange.rows, 1, CV_32FC1);
		cv::Mat rightFootStdDevVal_filtering(Right3DRange.rows, 1, CV_32FC1);

		leftFootMeanVal_filtering.setTo(0); rightFootMeanVal_filtering.setTo(0);
		leftFootStdDevVal_filtering.setTo(0); rightFootStdDevVal_filtering.setTo(0);

		size_t fstep = leftFootMeanVal_raw.step / sizeof(float);
		for (int y = nFootTmpMin_y; y < fHRoImg.rows; y++)
		{
			float* ptrLeftMeanVal_raw = (float*)leftFootMeanVal_raw.data;
			float* ptrRightMeanVal_raw = (float*)rightFootMeanVal_raw.data;

			float* ptrLeftStdDevVal_raw = (float*)leftFootStdDevVal_raw.data;
			float* ptrRightStdDevVal_raw = (float*)rightFootStdDevVal_raw.data;

			float* ptrleftMeanVal_filteringRaw = (float*)leftFootMeanVal_filtering.data;
			float* ptrRightMeanVal_filteringRaw = (float*)rightFootMeanVal_filtering.data;

			float* ptrleftStdDevVal_filteringRaw = (float*)leftFootStdDevVal_filtering.data;
			float* ptrRightStdDevVal_filteringRaw = (float*)rightFootStdDevVal_filtering.data;

			if (y == nFootTmpMin_y || y == nFootTmpMin_y + 1)
			{
				ptrleftMeanVal_filteringRaw[fstep * y] = 0.0f;

				ptrRightMeanVal_filteringRaw[fstep * y] = 0.0f;

				ptrleftStdDevVal_filteringRaw[fstep * y] = 0.0f;

				ptrRightStdDevVal_filteringRaw[fstep * y] = 0.0f;
			}
			else if (y == fHRoImg.rows - 1 || y == fHRoImg.rows - 2)
			{
				ptrleftMeanVal_filteringRaw[fstep * y] = (ptrLeftMeanVal_raw[fstep * (y - 2)] + ptrLeftMeanVal_raw[fstep * (y - 1)]
					+ ptrLeftMeanVal_raw[fstep * y]) / (float)5.0f;

				ptrRightMeanVal_filteringRaw[fstep * y] = (ptrRightMeanVal_raw[fstep * (y - 2)] + ptrRightMeanVal_raw[fstep * (y - 1)]
					+ ptrRightMeanVal_raw[fstep * y]) / (float)5.0f;

				ptrleftStdDevVal_filteringRaw[fstep * y] = (ptrLeftStdDevVal_raw[fstep * (y - 2)] + ptrLeftStdDevVal_raw[fstep * (y - 1)]
					+ ptrLeftStdDevVal_raw[fstep * y]) / (float)5.0f;

				ptrRightStdDevVal_filteringRaw[fstep * y] = (ptrRightStdDevVal_raw[fstep * (y - 2)] + ptrRightStdDevVal_raw[fstep * (y - 1)]
					+ ptrRightStdDevVal_raw[fstep * y]) / (float)5.0f;
			}
			else
			{
				if (ptrLeftMeanVal_raw[fstep * (y - 2)] != 0.0f  &&  ptrLeftMeanVal_raw[fstep * (y - 1)] != 0.0f)
				{
					ptrleftMeanVal_filteringRaw[fstep * y] = (ptrLeftMeanVal_raw[fstep * (y - 2)] + ptrLeftMeanVal_raw[fstep * (y - 1)]
						+ ptrLeftMeanVal_raw[fstep * y] + ptrLeftMeanVal_raw[fstep * (y + 1)] + ptrLeftMeanVal_raw[fstep * (y + 2)]) / (float)5.0f;
				}
				if (ptrRightMeanVal_raw[fstep * (y - 2)] != 0.0f  &&  ptrRightMeanVal_raw[fstep * (y - 1)] != 0.0f)
				{
					ptrRightMeanVal_filteringRaw[fstep * y] = (ptrRightMeanVal_raw[fstep * (y - 2)] + ptrRightMeanVal_raw[fstep * (y - 1)]
						+ ptrRightMeanVal_raw[fstep * y] + ptrRightMeanVal_raw[fstep * (y + 1)] + ptrRightMeanVal_raw[fstep * (y + 2)]) / (float)5.0f;
				}

				if (ptrLeftStdDevVal_raw[fstep * (y - 2)] != 0.0f  &&  ptrLeftStdDevVal_raw[fstep * (y - 1)] != 0.0f)
				{
					ptrleftStdDevVal_filteringRaw[fstep * y] = (ptrLeftStdDevVal_raw[fstep * (y - 2)] + ptrLeftStdDevVal_raw[fstep * (y - 1)]
						+ ptrLeftStdDevVal_raw[fstep * y] + ptrLeftStdDevVal_raw[fstep * (y + 1)] + ptrLeftStdDevVal_raw[fstep * (y + 2)]) / (float)5.0f;

				}

				if (ptrRightStdDevVal_raw[fstep * (y - 2)] != 0.0f  &&  ptrRightStdDevVal_raw[fstep * (y - 1)] != 0.0f)
				{
					ptrRightStdDevVal_filteringRaw[fstep * y] = (ptrRightStdDevVal_raw[fstep * (y - 2)] + ptrRightStdDevVal_raw[fstep * (y - 1)]
						+ ptrRightStdDevVal_raw[fstep * y] + ptrRightStdDevVal_raw[fstep * (y + 1)] + ptrRightStdDevVal_raw[fstep * (y + 2)]) / (float)5.0f;
				}
			}

#ifdef _AlgoTool
			fprintf(fp_LeftFootMeanSlope_Accum, "%f,", ptrleftMeanVal_filteringRaw[fstep * y]);
			fprintf(fp_RightFootMeanSlope_Accum, "%f,", ptrRightMeanVal_filteringRaw[fstep * y]);
			fprintf(fp_LeftFootStdDevSlope_Accum, "%f,", ptrleftStdDevVal_filteringRaw[fstep * y]);
			fprintf(fp_RightFootStdDevSlope_Accum, "%f,", ptrRightStdDevVal_filteringRaw[fstep * y]);
#endif
		}

#ifdef _AlgoTool
		fprintf(fp_LeftFootMeanSlope_Accum, "\n");
		fprintf(fp_RightFootMeanSlope_Accum, "\n");
		fprintf(fp_LeftFootStdDevSlope_Accum, "\n");
		fprintf(fp_RightFootStdDevSlope_Accum, "\n");
#endif
		//3. Æo±OCEAI μ￥AIAIAC ±a¿i±a(n = (n+1) - (n-1)) °e≫e
		cv::Mat leftFootMeanSlope(fHRoImg.rows, 1, CV_32FC1);
		cv::Mat rightFootMeanSlope(fHRoImg.rows, 1, CV_32FC1);

		cv::Mat leftFootStdDevSlope(fHRoImg.rows, 1, CV_32FC1);
		cv::Mat rightFootStdDevSlope(fHRoImg.rows, 1, CV_32FC1);

		leftFootMeanSlope.setTo(0);		rightFootMeanSlope.setTo(0);
		leftFootStdDevSlope.setTo(0);		rightFootStdDevSlope.setTo(0);

		for (int y = nFootTmpMin_y; y < fHRoImg.rows; y++)
		{
			float* ptrLeftMeanFilterVal = (float*)leftFootMeanVal_filtering.data;
			float* ptrRightMeanFilterVal = (float*)rightFootMeanVal_filtering.data;

			float* ptrLeftMeanSlope = (float*)leftFootMeanSlope.data;
			float* ptrRightMeanSlope = (float*)rightFootMeanSlope.data;

			float* ptrLeftStdDevFilterVal = (float*)leftFootStdDevVal_filtering.data;
			float* ptrRightStdDevFilterVal = (float*)rightFootStdDevVal_filtering.data;

			float* ptrLeftStdDevSlope = (float*)leftFootStdDevSlope.data;
			float* ptrRightStdDevSlope = (float*)rightFootStdDevSlope.data;

			if (y == nFootTmpMin_y)
			{
				ptrLeftMeanSlope[fstep * y] = 0.0f;
				ptrRightMeanSlope[fstep * y] = 0.0f;
			}
			else if (y == fHRoImg.rows - 1)
			{
				ptrLeftMeanSlope[fstep * y] = ptrLeftMeanSlope[fstep * (y - 1)];
				ptrRightMeanSlope[fstep * y] = ptrLeftMeanSlope[fstep * (y - 1)];
			}
			else
			{
				if (ptrLeftMeanFilterVal[fstep * (y + 1)] != 0.0f)
					ptrLeftMeanSlope[fstep * y] = ptrLeftMeanFilterVal[fstep * (y + 1)] - ptrLeftMeanFilterVal[fstep * (y - 1)];

				if (ptrRightMeanFilterVal[fstep * (y + 1)] != 0.0f)
					ptrRightMeanSlope[fstep * y] = ptrRightMeanFilterVal[fstep * (y + 1)] - ptrRightMeanFilterVal[fstep * (y - 1)];
			}



			if (y == nFootTmpMin_y)
			{
				ptrLeftStdDevSlope[fstep * y] = 0.0f;
				ptrRightStdDevSlope[fstep * y] = 0.0f;
			}
			else if (y == fHRoImg.rows - 1)
			{
				ptrLeftStdDevSlope[fstep * y] = ptrLeftStdDevFilterVal[fstep * (y - 1)];
				ptrRightStdDevSlope[fstep * y] = ptrRightStdDevFilterVal[fstep * (y - 1)];
			}
			else
			{
				if (ptrLeftStdDevFilterVal[fstep * (y + 1)] != 0.0f)
					ptrLeftStdDevSlope[fstep * y] = ptrLeftStdDevFilterVal[fstep * (y + 1)] - ptrLeftStdDevFilterVal[fstep * (y - 1)];

				if (ptrRightStdDevFilterVal[fstep * (y + 1)] != 0.0f)
					ptrRightStdDevSlope[fstep * y] = ptrRightStdDevFilterVal[fstep * (y + 1)] - ptrRightStdDevFilterVal[fstep * (y - 1)];
			}

#ifdef _AlgoTool
			fprintf(fp_LeftFootMeanSlope_Accum, "%f,", ptrLeftMeanSlope[fstep * y]);
			fprintf(fp_RightFootMeanSlope_Accum, "%f,", ptrRightMeanSlope[fstep * y]);
			fprintf(fp_LeftFootStdDevSlope_Accum, "%f,", ptrLeftStdDevSlope[fstep * y]);
			fprintf(fp_RightFootStdDevSlope_Accum, "%f,", ptrRightStdDevSlope[fstep * y]);
#endif

		}

#ifdef _AlgoTool
		fprintf(fp_LeftFootMeanSlope_Accum, "\n");
		fprintf(fp_RightFootMeanSlope_Accum, "\n");
		fprintf(fp_LeftFootStdDevSlope_Accum, "\n");
		fprintf(fp_RightFootStdDevSlope_Accum, "\n");
#endif

		//4. ±a¿i±a °e≫e °a°u¸| ´ⓒAu -	Accumulate results
		cv::Mat leftFootMeanSlope_Accum(fHRoImg.rows, 1, CV_32FC1);
		cv::Mat rightFootMeanSlope_Accum(fHRoImg.rows, 1, CV_32FC1);

		cv::Mat leftFootStdDevSlope_Accum(fHRoImg.rows, 1, CV_32FC1);
		cv::Mat rightFootStdDevSlope_Accum(fHRoImg.rows, 1, CV_32FC1);

		leftFootMeanSlope_Accum.setTo(0);		rightFootMeanSlope_Accum.setTo(0);
		leftFootStdDevSlope_Accum.setTo(0);		rightFootStdDevSlope_Accum.setTo(0);

		std::vector<float> vecLeftMeanSlopeAccum, vecRightMeanSlopeAccum;
		std::vector<float> vecLeftStdDevSlopeAccum, vecRightStdDevSlopeAccum;
		vecLeftMeanSlopeAccum.clear();	vecRightMeanSlopeAccum.clear();
		vecLeftStdDevSlopeAccum.clear();	vecRightStdDevSlopeAccum.clear();

		float fSum_Left = 0.0f, fSum_Right = 0.0f;

		for (int y = nFootTmpMin_y; y < nFootTmpMax_y; y++)
		{
			float* ptrLeftMeanSlopeVal = (float*)leftFootMeanSlope.data;
			float* ptrRightMeanSlopeVal = (float*)rightFootMeanSlope.data;
			float* ptrLeftMeanSlopeAccum = (float*)leftFootMeanSlope_Accum.data;
			float* ptrRightMeanSlopeAccum = (float*)rightFootMeanSlope_Accum.data;

			float* ptrLeftStdDevSlopeVal = (float*)leftFootStdDevSlope.data;
			float* ptrRightStdDevSlopeVal = (float*)rightFootStdDevSlope.data;
			float* ptrLeftStdDevSlopeAccum = (float*)leftFootStdDevSlope_Accum.data;
			float* ptrRightStdDevSlopeAccum = (float*)rightFootStdDevSlope_Accum.data;

			if (y == nFootTmpMin_y)
			{
				ptrLeftMeanSlopeAccum[fstep * y] = 0.0f;
				ptrRightMeanSlopeAccum[fstep * y] = 0.0f;
			}
			else
			{
				fSum_Left = 0.0f, fSum_Right = 0.0f;

				for (int x = nFootTmpMin_y; x <= y; x++)
				{
					if (ptrLeftMeanSlopeVal[fstep * x] != 0.0f)
					{
						fSum_Left = fSum_Left + ptrLeftMeanSlopeVal[fstep * x];

					}
					if (ptrRightMeanSlopeVal[fstep * x] != 0.0f)
					{
						fSum_Right = fSum_Right + ptrRightMeanSlopeVal[fstep * x];
					}
				}

				ptrLeftMeanSlopeAccum[fstep * y] = fSum_Left;
				ptrRightMeanSlopeAccum[fstep * y] = fSum_Right;
			}

#ifdef _AlgoTool
			fprintf(fp_LeftFootMeanSlope_Accum, "%f,", ptrLeftMeanSlopeAccum[fstep * y]);
			fprintf(fp_RightFootMeanSlope_Accum, "%f,", ptrRightMeanSlopeAccum[fstep * y]);
#endif

			vecLeftMeanSlopeAccum.push_back(ptrLeftMeanSlopeAccum[fstep * y]);
			vecRightMeanSlopeAccum.push_back(ptrRightMeanSlopeAccum[fstep * y]);

			if (y == nFootTmpMin_y)
			{
				ptrLeftStdDevSlopeAccum[fstep * y] = 0.0f;
				ptrRightStdDevSlopeAccum[fstep * y] = 0.0f;
			}
			else
			{
				fSum_Left = 0.0f, fSum_Right = 0.0f;

				for (int x = nFootTmpMin_y; x <= y; x++)
				{
					if (ptrLeftStdDevSlopeVal[fstep * x] != 0.0f)
					{
						fSum_Left = fSum_Left + ptrLeftStdDevSlopeVal[fstep * x];

					}
					if (ptrRightStdDevSlopeVal[fstep * x] != 0.0f)
					{
						fSum_Right = fSum_Right + ptrRightStdDevSlopeVal[fstep * x];
					}
				}

				ptrLeftStdDevSlopeAccum[fstep * y] = fSum_Left;
				ptrRightStdDevSlopeAccum[fstep * y] = fSum_Right;
			}

#ifdef _AlgoTool
			fprintf(fp_LeftFootStdDevSlope_Accum, "%f,", ptrLeftStdDevSlopeAccum[fstep * y]);
			fprintf(fp_RightFootStdDevSlope_Accum, "%f,", ptrRightStdDevSlopeAccum[fstep * y]);
#endif

			vecLeftStdDevSlopeAccum.push_back(ptrLeftStdDevSlopeAccum[fstep * y]);
			vecRightStdDevSlopeAccum.push_back(ptrRightStdDevSlopeAccum[fstep * y]);
		}

#ifdef _AlgoTool
		fclose(fp_LeftFootMeanSlope_Accum); fclose(fp_RightFootMeanSlope_Accum);
		fclose(fp_LeftFootStdDevSlope_Accum); fclose(fp_RightFootStdDevSlope_Accum);
#endif

		//5. Wing AC A¤≫o ¹uA§ °e≫e - ¾cAE footAC 1/3 AoA¡¸¸ ≫eAa
		float fAvgLeftWingRefMean = 0.0f;
		float fAvgRightWingRefMean = 0.0f;
		float fAvgLeftWingRefStdDev = 0.0f;
		float fAvgRightWingRefStdDev = 0.0f;

		for (int y = nFootTmpMin_y; y < nFootTmpMax_y; y++)
		{
			float* ptrLeftMeanSlopeAccum = (float*)leftFootMeanSlope_Accum.data;
			float* ptrRightMeanSlopeAccum = (float*)rightFootMeanSlope_Accum.data;
			float* ptrLeftStdDevSlopeAccum = (float*)leftFootStdDevSlope_Accum.data;
			float* ptrRightStdDevSlopeAccum = (float*)rightFootStdDevSlope_Accum.data;

			if (rcLeftFootRefROI.top <= y && y <= rcLeftFootRefROI.bottom)
			{
				fAvgLeftWingRefMean = fAvgLeftWingRefMean + ptrLeftMeanSlopeAccum[fstep * y];
			}

			if (rcRightFootRefROI.top <= y && y <= rcRightFootRefROI.bottom)
			{
				fAvgRightWingRefMean = fAvgRightWingRefMean + ptrRightMeanSlopeAccum[fstep * y];
			}

			if (rcLeftFootRefROI.top <= y && y <= rcLeftFootRefROI.bottom)
			{
				fAvgLeftWingRefStdDev = fAvgLeftWingRefStdDev + ptrLeftStdDevSlopeAccum[fstep * y];
			}

			if (rcRightFootRefROI.top <= y && y <= rcRightFootRefROI.bottom)
			{
				fAvgRightWingRefStdDev = fAvgRightWingRefStdDev + ptrRightStdDevSlopeAccum[fstep * y];
			}

		}
		fAvgLeftWingRefMean = fAvgLeftWingRefMean / ((rcLeftFootRefROI.bottom - rcLeftFootRefROI.top) + 1);
		fAvgRightWingRefMean = fAvgRightWingRefMean / ((rcRightFootRefROI.bottom - rcRightFootRefROI.top) + 1);

		fAvgLeftWingRefStdDev = fAvgLeftWingRefStdDev / ((rcLeftFootRefROI.bottom - rcLeftFootRefROI.top) + 1);
		fAvgRightWingRefStdDev = fAvgRightWingRefStdDev / ((rcRightFootRefROI.bottom - rcRightFootRefROI.top) + 1);

		float fStdDevLeftWingRefMean = 0.0f;
		float fStdDevRightWingRefMean = 0.0f;
		float fStdDevLeftWingRefStdDev = 0.0f;
		float fStdDevRightWingRefStdDev = 0.0f;

		for (int y = nFootTmpMin_y; y < nFootTmpMax_y; y++)
		{
			float* ptrLeftMeanSlopeAccum = (float*)leftFootMeanSlope_Accum.data;
			float* ptrRightMeanSlopeAccum = (float*)rightFootMeanSlope_Accum.data;
			float* ptrLeftStdDevSlopeAccum = (float*)leftFootStdDevSlope_Accum.data;
			float* ptrRightStdDevSlopeAccum = (float*)rightFootStdDevSlope_Accum.data;

			if (rcLeftFootRefROI.top <= y && y <= rcLeftFootRefROI.bottom)
			{
				fStdDevLeftWingRefMean = fAvgLeftWingRefMean - ptrLeftMeanSlopeAccum[fstep *y];				//a' = Æo±O - μ￥AIAI°ª
				fStdDevLeftWingRefMean += fStdDevLeftWingRefMean * fStdDevLeftWingRefMean;		//a' A|°o
			}

			if (rcRightFootRefROI.top <= y && y <= rcRightFootRefROI.bottom)
			{
				fStdDevRightWingRefMean = fAvgRightWingRefMean - ptrRightMeanSlopeAccum[fstep *y];				//a' = Æo±O - μ￥AIAI°ª
				fStdDevRightWingRefMean += fStdDevRightWingRefMean * fStdDevRightWingRefMean;		//a' A|°o
			}

			if (rcLeftFootRefROI.top <= y && y <= rcLeftFootRefROI.bottom)
			{
				fStdDevLeftWingRefStdDev = fAvgLeftWingRefStdDev - ptrLeftStdDevSlopeAccum[fstep *y];				//a' = Æo±O - μ￥AIAI°ª
				fStdDevLeftWingRefStdDev += fStdDevLeftWingRefStdDev * fStdDevLeftWingRefStdDev;		//a' A|°o
			}

			if (rcRightFootRefROI.top <= y && y <= rcRightFootRefROI.bottom)
			{
				fStdDevRightWingRefStdDev = fAvgRightWingRefStdDev - ptrRightStdDevSlopeAccum[fstep *y];				//a' = Æo±O - μ￥AIAI°ª
				fStdDevRightWingRefStdDev += fStdDevRightWingRefStdDev * fStdDevRightWingRefStdDev;		//a' A|°o
			}

		}
		fStdDevLeftWingRefMean = fStdDevLeftWingRefMean / ((rcLeftFootRefROI.bottom - rcLeftFootRefROI.top) + 1);
		fStdDevRightWingRefMean = fStdDevRightWingRefMean / ((rcRightFootRefROI.bottom - rcRightFootRefROI.top) + 1);

		fStdDevLeftWingRefStdDev = fStdDevLeftWingRefStdDev / ((rcLeftFootRefROI.bottom - rcLeftFootRefROI.top) + 1);
		fStdDevRightWingRefStdDev = fStdDevRightWingRefStdDev / ((rcRightFootRefROI.bottom - rcRightFootRefROI.top) + 1);


		//6. Wing A¤≫o±ºAC (Æo±O+C￥AØÆiA÷)*coef ¹uA§ ¹U¿¡ A¸Ac½A Wire Start EAº¸±ºA¸·I ¼±A¤ (Foot Aß½E A§A¡ ´eºn ¾Æ·¡AEA¸·I Scan)
		float fCoefToFindWire = 1.0f;

		if (bUseMeanSlopeAccum)
		{
			fCoefToFindWire = 1.3f;
		}
		else
		{
			fCoefToFindWire = 1.5f;
		}

		cv::Mat LeftWireMeanCandidatesImg(fHRoImg.rows, 1, CV_8UC1);
		cv::Mat RightWireMeanCandidatesImg(fHRoImg.rows, 1, CV_8UC1);
		cv::Mat LeftWireStdDevCandidatesImg(fHRoImg.rows, 1, CV_8UC1);
		cv::Mat RightWireStdDevCandidatesImg(fHRoImg.rows, 1, CV_8UC1);

		LeftWireMeanCandidatesImg.setTo(0);
		RightWireMeanCandidatesImg.setTo(0);
		LeftWireStdDevCandidatesImg.setTo(0);
		RightWireStdDevCandidatesImg.setTo(0);

		for (int y = nFootTmpMin_y; y < nFootTmpMax_y; y++)
		{
			float* ptrLeftMeanSlopeAccum = (float*)leftFootMeanSlope_Accum.data;
			float* ptrRightMeanSlopeAccum = (float*)rightFootMeanSlope_Accum.data;
			float* ptrLeftStdDevSlopeAccum = (float*)leftFootStdDevSlope_Accum.data;
			float* ptrRightStdDevSlopeAccum = (float*)rightFootStdDevSlope_Accum.data;

			uchar* ptrLeftMeanWireCandi = LeftWireMeanCandidatesImg.data;
			uchar* ptrRightMeanWireCandi = RightWireMeanCandidatesImg.data;
			uchar* ptrLeftStdDevWireCandi = LeftWireStdDevCandidatesImg.data;
			uchar* ptrRightStdDevWireCandi = RightWireStdDevCandidatesImg.data;

			if (y > CenterPo.y - Margin.y)
			{
				if (ptrLeftMeanSlopeAccum[fstep * y] < (fAvgLeftWingRefMean + fStdDevLeftWingRefMean)*fCoefToFindWire)	//Wing A¤≫o¹uA§
				{

				}
				else
				{
					ptrLeftMeanWireCandi[LeftWireMeanCandidatesImg.step * y] = 255;		//Wing A¤≫o¹uA§°¡ ¾Æ´O ½A wire EAº¸±ºA¸·I ¼±A¤
				}

				if (ptrRightMeanSlopeAccum[fstep * y] < (fAvgRightWingRefMean + fStdDevRightWingRefMean)*fCoefToFindWire)
				{

				}
				else
				{
					ptrRightMeanWireCandi[RightWireMeanCandidatesImg.step * y] = 255;
				}

				if (ptrLeftStdDevSlopeAccum[fstep * y] < (fAvgLeftWingRefStdDev + fStdDevLeftWingRefStdDev)*fCoefToFindWire)
				{

				}
				else
				{
					ptrLeftStdDevWireCandi[LeftWireStdDevCandidatesImg.step * y] = 255;
				}

				if (ptrRightStdDevSlopeAccum[fstep * y] < (fAvgRightWingRefStdDev + fStdDevRightWingRefStdDev)*fCoefToFindWire)
				{

				}
				else
				{
					ptrRightStdDevWireCandi[RightWireStdDevCandidatesImg.step * y] = 255;
				}
			}
		}

		//7. Wire EAº¸±º Aß, A÷·C AU¼¼AC Foot A|AI ¾Æ·¡ ³¡ AE¿¡¼­ 3E¸ ¿￢¼OμC¸e CØ´c °ªA≫ Wire Start·I °aA¤
		std::vector<int> vecStartPosY_LeftMean, vecEndPosY_LeftMean;
		std::vector<int> vecStartPosY_RightMean, vecEndPosY_RightMean;
		std::vector<int> vecStartPosY_LeftStdDev, vecEndPosY_LeftStdDev;
		std::vector<int> vecStartPosY_RightStdDev, vecEndPosY_RightStdDev;

		int nLeftMeanFootDetectCnt = 0, nRightMeanFootDetectCnt = 0;
		int nLeftStdDevFootDetectCnt = 0, nRightStdDevFootDetectCnt = 0;

		int StartPosY_LeftMean = 0, StartPosY_RightMean = 0;
		int StartPosY_LeftStdDev = 0, StartPosY_RightStdDev = 0;

		bool bStartFlag_LeftMean = false, bEndFlag_LeftMean = false, bStartFlag_RightMean = false, bEndFlag_RightMean = false;
		bool bStartFlag_LeftStdDev = false, bEndFlag_LeftStdDev = false, bStartFlag_RightStdDev = false, bEndFlag_RightStdDev = false;

		for (int y = nFootTmpMin_y; y < fHRoImg.rows; y++)
		{
			uchar* ptrLeftMeanWireCandi = LeftWireMeanCandidatesImg.data;
			uchar* ptrRightMeanWireCandi = RightWireMeanCandidatesImg.data;
			uchar* ptrLeftStdDevWireCandi = LeftWireStdDevCandidatesImg.data;
			uchar* ptrRightStdDevWireCandi = RightWireStdDevCandidatesImg.data;

			if (bUseMeanSlopeAccum)
			{
				if (bStartFlag_LeftMean == false && ptrLeftMeanWireCandi[LeftWireMeanCandidatesImg.step * y] == 255)
				{
					bStartFlag_LeftMean = true;
					StartPosY_LeftMean = y;
					nLeftMeanFootDetectCnt++;
				}
				if (bStartFlag_LeftMean == true && ptrLeftMeanWireCandi[LeftWireMeanCandidatesImg.step * y] == 0)
				{
					if (nLeftMeanFootDetectCnt >= 3)
					{
						vecStartPosY_LeftMean.push_back(StartPosY_LeftMean);
						vecEndPosY_LeftMean.push_back(y);

						bStartFlag_LeftMean = false;
						StartPosY_LeftMean = 0;
						nLeftMeanFootDetectCnt = 0;
					}
				}
				if (bStartFlag_LeftMean)
					nLeftMeanFootDetectCnt++;

				if (bStartFlag_RightMean == false && ptrRightMeanWireCandi[RightWireMeanCandidatesImg.step * y] == 255)
				{
					bStartFlag_RightMean = true;
					StartPosY_RightMean = y;
					nRightMeanFootDetectCnt++;
				}
				if (bStartFlag_RightMean == true && ptrRightMeanWireCandi[RightWireMeanCandidatesImg.step * y] == 0)
				{
					if (nRightMeanFootDetectCnt >= 3)
					{
						vecStartPosY_RightMean.push_back(StartPosY_RightMean);
						vecEndPosY_RightMean.push_back(y);

						bStartFlag_RightMean = false;
						StartPosY_RightMean = 0;
						nRightMeanFootDetectCnt = 0;
					}
				}
				if (bStartFlag_RightMean)
					nRightMeanFootDetectCnt++;
			}
			else
			{
				if (bStartFlag_LeftStdDev == false && ptrLeftStdDevWireCandi[LeftWireStdDevCandidatesImg.step * y] == 255)
				{
					bStartFlag_LeftStdDev = true;
					StartPosY_LeftStdDev = y;
					nLeftStdDevFootDetectCnt++;
				}
				if (bStartFlag_LeftStdDev == true && ptrLeftStdDevWireCandi[LeftWireStdDevCandidatesImg.step * y] == 0)
				{
					if (nLeftStdDevFootDetectCnt >= 3)
					{
						vecStartPosY_LeftStdDev.push_back(StartPosY_LeftStdDev);
						vecEndPosY_LeftStdDev.push_back(y);

						bStartFlag_LeftStdDev = false;
						StartPosY_LeftStdDev = 0;
						nLeftStdDevFootDetectCnt = 0;
					}
				}
				if (bStartFlag_LeftStdDev)
					nLeftStdDevFootDetectCnt++;

				if (bStartFlag_RightStdDev == false && ptrRightStdDevWireCandi[RightWireStdDevCandidatesImg.step * y] == 255)
				{
					bStartFlag_RightStdDev = true;
					StartPosY_RightStdDev = y;
					nRightStdDevFootDetectCnt++;
				}
				if (bStartFlag_RightStdDev == true && ptrRightStdDevWireCandi[RightWireStdDevCandidatesImg.step * y] == 0)
				{
					if (nRightStdDevFootDetectCnt >= 3)
					{
						vecStartPosY_RightStdDev.push_back(StartPosY_RightStdDev);
						vecEndPosY_RightStdDev.push_back(y);

						bStartFlag_RightStdDev = false;
						StartPosY_RightStdDev = 0;
						nRightStdDevFootDetectCnt = 0;
					}
				}
				if (bStartFlag_RightStdDev)
					nRightStdDevFootDetectCnt++;
			}


		}

		int nLeftMeanFoot_WireCandiPosY = fHRoImg.rows - 1;
		int nRightMeanFoot_WireCandiPosY = fHRoImg.rows - 1;

		int nLeftStdDevFoot_WireCandiPosY = fHRoImg.rows - 1;
		int nRightStdDevFoot_WireCandiPosY = fHRoImg.rows - 1;

		int nwireStartPosY = fHRoImg.rows - 1;
		if (bUseMeanSlopeAccum)
		{
			if (vecStartPosY_LeftMean.size() > 0)
			{

				nLeftMeanFoot_WireCandiPosY = vecStartPosY_LeftMean.back();
			}
			if (vecStartPosY_RightMean.size() > 0)
			{

				nRightMeanFoot_WireCandiPosY = vecStartPosY_RightMean.back();
			}

			if (nLeftMeanFoot_WireCandiPosY == fHRoImg.rows - 1)
				return false;
			if (nRightMeanFoot_WireCandiPosY == fHRoImg.rows - 1)
				return false;

			nwireStartPosY = (nLeftMeanFoot_WireCandiPosY + nRightMeanFoot_WireCandiPosY) / 2;
		}
		else
		{
			if (vecStartPosY_LeftStdDev.size() > 0)
			{

				nLeftStdDevFoot_WireCandiPosY = vecStartPosY_LeftStdDev.back();
			}
			if (vecStartPosY_RightStdDev.size() > 0)
			{

				nRightStdDevFoot_WireCandiPosY = vecStartPosY_RightStdDev.back();
			}

			if (nLeftStdDevFoot_WireCandiPosY == fHRoImg.rows - 1)
				return false;
			if (nRightStdDevFoot_WireCandiPosY == fHRoImg.rows - 1)
				return false;

			nwireStartPosY = (nLeftStdDevFoot_WireCandiPosY + nRightStdDevFoot_WireCandiPosY) / 2;
		}

		(*nDetectedWireStartPosY) = nwireStartPosY;

		Ret = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::DetectWireStartPos_UseLeftAndRightAreasOfFootHeight() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}


bool CPInsp_Wedge::DetectWireStartPos_UseFootHeightProfile(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat FootBWRemovedFlowDown, cv::Mat WingCandiImg, cv::Point& CenterPo, cv::Point& Margin, int *nDetectedWireStartPosY)
{
	bool Ret = false;

	int nLine = __LINE__;

	try
	{
		//°a°u AE±aE­
		(*nDetectedWireStartPosY) = fHRoImg.rows - 1;
		bool bFoundWireStart = false;

		//AO·A ¿μ≫o AE±aE­
		cv::Mat Foot_RemovedFlowDown = FootBWRemovedFlowDown.clone();
		cv::Mat Wing_CandidateImg = WingCandiImg.clone();

		cv::Mat cvFindWireStImg = FootBWRemovedFlowDown.clone();

		////1. Foot Blob ¿μ¿ª Rect °e≫e, Wing Blob ¿μ¿ª Rect °e≫e
		int ncvImgWidth = Foot_RemovedFlowDown.cols;
		int ncvImgHeight = Foot_RemovedFlowDown.rows;

		int nFootTmpMin_x = ncvImgWidth * ncvImgHeight, nFootTmpMax_x = 0;
		int nFootTmpMin_y = ncvImgWidth * ncvImgHeight, nFootTmpMax_y = 0;

		int nWingCandiTmpMin_x = ncvImgWidth * ncvImgHeight, nWingCandiTmpMax_x = 0;
		int nWingCandiTmpMin_y = ncvImgWidth * ncvImgHeight, nWingCandiTmpMax_y = 0;

		for (int r = 0; r < Foot_RemovedFlowDown.rows; r++)
		{
			UCHAR* Ptr_foot = Foot_RemovedFlowDown.data;
			UCHAR* Ptr_wing = Wing_CandidateImg.data;

			for (int c = 0; c < Foot_RemovedFlowDown.cols; c++)
			{
				if (Ptr_foot[r*Foot_RemovedFlowDown.step + c] == 255)
				{
					if (c < nFootTmpMin_x) nFootTmpMin_x = c;
					if (c > nFootTmpMax_x) nFootTmpMax_x = c;
					if (r < nFootTmpMin_y) nFootTmpMin_y = r;
					if (r > nFootTmpMax_y) nFootTmpMax_y = r;
				}

				if (Ptr_wing[r*Foot_RemovedFlowDown.step + c] == 255)
				{
					if (c < nWingCandiTmpMin_x) nWingCandiTmpMin_x = c;
					if (c > nWingCandiTmpMax_x) nWingCandiTmpMax_x = c;
					if (r < nWingCandiTmpMin_y) nWingCandiTmpMin_y = r;
					if (r > nWingCandiTmpMax_y) nWingCandiTmpMax_y = r;
				}
			}

		}

		//foot BW ¿μ≫o ºn¾iAOA≫ ½A CO¼o A¾·a
		if (nFootTmpMin_x == ncvImgWidth * ncvImgHeight && nFootTmpMin_y == ncvImgWidth * ncvImgHeight &&
			nFootTmpMax_x == 0 && nFootTmpMax_y == 0)
		{
			return false;
		}

		//Wing EAº¸±º ¿μ≫o ºn¾iAOA≫ ½A CO¼o A¾·a
		if (nWingCandiTmpMin_x == ncvImgWidth * ncvImgHeight && nWingCandiTmpMin_y == ncvImgWidth * ncvImgHeight &&
			nWingCandiTmpMax_x == 0 && nWingCandiTmpMax_y == 0)
		{
			return false;
		}

		////2. ³oAI ÆACu AßAaA≫ A§CN Foot 3D ¿μ≫o ≫y¼º (Ee·?³≫¸°¿μ¿ªAº A|°A), ³oAI AO¼OAO´e°ª AßAa
		cv::Mat cv3DImg_OnlyFootBlob(fHRoImg.rows, fHRoImg.cols, CV_32FC1);
		cv3DImg_OnlyFootBlob.setTo(0);

		size_t floatstep_Foot3DCorrect = fHRoImg.step / sizeof(float);
		float fFootHgtMaxVal = 0;
		float fFootHgtMinVal = 1000;

		int nScanStartPoY = nFootTmpMin_y;

		if (nFootTmpMin_x > nFootTmpMax_x)	return false;


		for (int r = nScanStartPoY; r < nFootTmpMax_y; r++)
		{
			UCHAR* Ptr = Foot_RemovedFlowDown.data;

			float* Ptr_Foot3D = (float*)fHRoImg.data;
			float* ptr3DHgtRstData = (float*)cv3DImg_OnlyFootBlob.data;

			for (int c = nFootTmpMin_x; c < nFootTmpMax_x; c++)
			{
				if (Ptr[r * Foot_RemovedFlowDown.step + c] == 0)
				{
					ptr3DHgtRstData[r * floatstep_Foot3DCorrect + c] = 0;
				}
				else
				{
					ptr3DHgtRstData[r * floatstep_Foot3DCorrect + c] = Ptr_Foot3D[r * floatstep_Foot3DCorrect + c];

					if (Ptr_Foot3D[r * floatstep_Foot3DCorrect + c] > fFootHgtMaxVal)
						fFootHgtMaxVal = Ptr_Foot3D[r * floatstep_Foot3DCorrect + c];
					if (Ptr_Foot3D[r * floatstep_Foot3DCorrect + c] < fFootHgtMinVal)
						fFootHgtMinVal = Ptr_Foot3D[r * floatstep_Foot3DCorrect + c];
				}
			}

		}

		//½ºAμ A§A¡º¸´U ¾OA¸·I 2CE¼¿, μU·I 2CE¼¿ ¶³¾iAø °¡·I ³oAI ÆACu μ￥AIAI 
		std::vector<float> HightVector;		HightVector.clear();
		std::vector<float> HightVector_post;		HightVector_post.clear();

		//μI °³AC ³oAI ÆACu μ￥AIAI 
		std::vector < int> vecGraphYVal_ErrDiff;		vecGraphYVal_ErrDiff.clear();
		std::vector < float> vecGraph_RearMaxHgt;		vecGraph_RearMaxHgt.clear();

		int nWingCnt = 0;
		int nContinuesCnt = 0;
		int nTmpLastFoundIdx = -1;
		int nGraphWingMaxVal = 0;

		//FILE* fp_FootWingSlope = fopen("D:\\FootInspImg\\insp_Slope_1Linediff.csv", "w+");
		//FILE* fp_FootWingSlope_2 = fopen("D:\\FootInspImg\\insp_Slope_1Linediff_2.csv", "w+");

		////4. Foot ½ºAμ ½AAU (¹I¸® ≫y¼ºCN 3D ¿¡¼­) - (°¡·I·I CN AU¾¿ ½ºAμCI¸e¼­ ³oAI ÆACu ±×·¡CA ≫y¼º)

		int nWingCandiLength = nWingCandiTmpMax_y - nWingCandiTmpMin_y;
		nScanStartPoY = (int)round(nWingCandiTmpMax_y - (nWingCandiLength * 0.6));

#if _DEBUG
		CString csDebugLog;
		csDebugLog.Format(_T("CPInsp_Wedge::DetectWireStartPos_UseFootHeightProfile() nScanStartPoY: [%d]"), nScanStartPoY);
		g_pMPTI->AddLog(csDebugLog);
#endif

		int nCurIdx = 0;
		float fExceptDataRatio = 0.6;
		int FirstScanLineDataCnt = 0;
		int ExptLineDataCnt = 0;
		int nCntLinePost = 4;

		int nIndexCheck = 0;
		float fOneLineHgt_Max_post = 0;
		int nFstLineDataCnt = 0;

		for (int r = 0; r < Foot_RemovedFlowDown.rows; r++)
		{
			HightVector.clear();
			HightVector_post.clear();

			bFoundWireStart = false;

			float* ptr3DHgtRstData = (float*)cv3DImg_OnlyFootBlob.data;

			if (nScanStartPoY > r)
				continue;

			//////4-1. Foot_BW X ¹uA§ ³≫¿¡¼­¸¸ ³oAI AuAa
			float fOneLineHgt_Max = 0;
			float fOneLineHgt_Min = fFootHgtMaxVal;
			fOneLineHgt_Max_post = 0;
			float fOneLineHgt_Min_post = fFootHgtMaxVal;

			float fTwoLineHgt_MaxValue = 0;

			for (int c = nFootTmpMin_x; c <= nFootTmpMax_x; c++)
			{
				float fHgtVal = 0;

				if (((r - 3) > 0) && (r - 3 < Foot_RemovedFlowDown.rows))
					fHgtVal = ptr3DHgtRstData[(r - 3) * floatstep_Foot3DCorrect + c];
				else
					return false;

				HightVector.push_back(fHgtVal);

				if (((r - 4) > 0) && (r - 4 < Foot_RemovedFlowDown.rows))
				{
					float fHgtVal_1 = ptr3DHgtRstData[(r - 4) * floatstep_Foot3DCorrect + c];
					HightVector[HightVector.size() - 1] = (HightVector[HightVector.size() - 1] + fHgtVal_1) / 2;
				}

				if (((r + nCntLinePost - 1) > 0) && ((r + nCntLinePost - 1) < Foot_RemovedFlowDown.rows))
				{
					float fHgtVal_4 = ptr3DHgtRstData[(r + nCntLinePost - 1) * floatstep_Foot3DCorrect + c];
					HightVector_post.push_back(fHgtVal_4);

				}

				if ((r + nCntLinePost > 0) && (r + nCntLinePost < Foot_RemovedFlowDown.rows))
				{
					float fHgtVal_post = ptr3DHgtRstData[(r + nCntLinePost) * floatstep_Foot3DCorrect + c];

					//μI ¶oAIμ￥AIAIAC °￠ Æ÷AIÆ®¸| Æo±OE­CN °ª
					float fValue = (HightVector_post[HightVector_post.size() - 1] + fHgtVal_post) / 2;
					HightVector_post[HightVector_post.size() - 1] = fValue;
				}
			}

			for (int x = 0; x < HightVector.size(); x++)
			{
				float fCurVal = HightVector[x];
				if (fOneLineHgt_Max < fCurVal)
					fOneLineHgt_Max = fCurVal;

				if (fCurVal != 0 && fOneLineHgt_Min > fCurVal)
					fOneLineHgt_Min = fCurVal;
			}

			for (int x = 0; x < HightVector_post.size(); x++)
			{
				float fCurVal_post = HightVector_post[x];
				if (fOneLineHgt_Max_post < fCurVal_post)
					fOneLineHgt_Max_post = fCurVal_post;

				if (fCurVal_post != 0 && fOneLineHgt_Min_post > fCurVal_post)
					fOneLineHgt_Min_post = fCurVal_post;

				if (fCurVal_post != 0)
					nFstLineDataCnt++;
			}

			if (r + nCntLinePost + 1 >= Foot_RemovedFlowDown.rows || r > Foot_RemovedFlowDown.rows - (nCntLinePost + 2))
			{
				continue;
			}

			if (r == nScanStartPoY)
			{
				FirstScanLineDataCnt = nFstLineDataCnt; // HightVector_post.size();
				ExptLineDataCnt = (int)round(nFstLineDataCnt * fExceptDataRatio);
			}

			//μI ¶oAIAC ³oAI A÷ °e≫eA≫ A§CN ¶oAI A¤±OE­ ¼oCa (1.0 : A¤±OE­ ¼oCa ¾ECO °æ¿i)
			float fLineGraphScale = 0;
			float fMok_post = 0;
			//μI ¶oAI Aß μUAE ¶oAIA¸·I Scale ¸AAa
			/*if ((int)fOneLineHgt_Max > (int)fOneLineHgt_Max_post)*/
			{
				fTwoLineHgt_MaxValue = fOneLineHgt_Max;
				fLineGraphScale = fTwoLineHgt_MaxValue;		//AO´e ³oAI·I AI¹IAo ¸AAa

				fMok_post = (fOneLineHgt_Max / fOneLineHgt_Max_post);

				//if (fMok_post == 0)
				fMok_post = 1.0;

				for (int x = 0; x < HightVector_post.size(); x++)
				{
					if (HightVector_post[x] != 0 && fMok_post != 0)
					{
						HightVector_post[x] = fMok_post * HightVector_post[x];
					}
				}

			}

#if _DEBUG
			//A¤±OE­CN μI ¶oAI E®AI¿e ±¸¹®
			if (r == nIndexCheck)
			{
				cv::Mat cvLineGraph((int)fLineGraphScale, HightVector_post.size(), CV_8UC1);
				cvLineGraph.setTo(0);
				int nHgtIdx = 0;
				int nHgtIdx_post = 0;

				for (int a = 0; a < cvLineGraph.rows; a++)
				{
					UCHAR* Ptr_LineRstImg = cvLineGraph.data;
					for (int b = 0; b < cvLineGraph.cols; b++)
					{
						int nIdx = 0;
						nIdx = (int)HightVector[nHgtIdx];

						int nIdx_post = 0;
						nIdx_post = (int)HightVector_post[nHgtIdx_post];

						if (nIdx <= cvLineGraph.rows)
						{
							if (nIdx == cvLineGraph.rows)
								nIdx = cvLineGraph.rows - 1;

							Ptr_LineRstImg[nIdx * cvLineGraph.cols + b] = 255;
							if (nHgtIdx < HightVector.size() - 1)
								nHgtIdx++;
						}
						if (nIdx_post <= cvLineGraph.rows)
						{
							if (nIdx_post == cvLineGraph.rows)
								nIdx_post = cvLineGraph.rows - 1;

							Ptr_LineRstImg[nIdx_post * cvLineGraph.cols + b] = 125;

							if (nHgtIdx_post < HightVector_post.size() - 1)
								nHgtIdx_post++;
						}



					}
				}
			}
			/////////////////////////////////
#endif

			bool bFlag = false;
			int nLineDataCnt = 0;
			for (int x = 0; x < HightVector.size(); x++)
			{
				if (HightVector[x] != 0)
				{
					bFlag = true;
					nLineDataCnt++;
				}
			}

			bool bFlag_post = false;
			int nLineDataCnt_post = 0;
			for (int x = 0; x < HightVector_post.size(); x++)
			{
				if (HightVector_post[x] != 0)
				{
					bFlag_post = true;
					nLineDataCnt_post++;
				}
			}

			if (nLineDataCnt > ExptLineDataCnt && nLineDataCnt_post > ExptLineDataCnt && bFlag_post && bFlag)
			{
				int nRepeatCnt = 0;
				if (HightVector_post.size() > HightVector.size())
					nRepeatCnt = HightVector.size();
				else
					nRepeatCnt = HightVector_post.size();

				float fSumDiff = 0;
				for (int a = 0; a < nRepeatCnt; a++)
				{
					//float fHgtDiffVal = (round(HightVector_post[a] - HightVector[a]));
					float fHgtDiffVal = abs(round(HightVector_post[a] - HightVector[a]));

					fSumDiff += fHgtDiffVal;
				}

				if (nGraphWingMaxVal < fSumDiff)
				{
					nGraphWingMaxVal = fSumDiff;
					nTmpLastFoundIdx = r;
				}

				vecGraphYVal_ErrDiff.push_back(fSumDiff);
				//fprintf(fp_FootWingSlope, "%f,", fSumDiff);

				vecGraph_RearMaxHgt.push_back(fOneLineHgt_Max_post);
				//fprintf(fp_FootWingSlope_2, "%f,", fOneLineHgt_Max_post);

			}

			nCurIdx++;

		}

		//fclose(fp_FootWingSlope);
		//fclose(fp_FootWingSlope_2);

		//³oAI A÷ ´ⓒAu ±×·¡CAAC ±a¿i±a °e≫e
		//FILE* fp_FootWingSlope_Angle = fopen("D:\\FootInspImg\\insp_Slope_diffAngle.csv", "w+");

		double dDeltaX1 = 0.0, dDeltaY1 = 0.0;
		float fAngle = 0.0;

		int fDiffMax = 0;
		int nIdx = 0;
		if (vecGraphYVal_ErrDiff.size() > 10 && vecGraph_RearMaxHgt.size() > 10 && vecGraphYVal_ErrDiff.size() == vecGraph_RearMaxHgt.size())
		{
			for (int a = 2; a < vecGraphYVal_ErrDiff.size() - 1; a++)
			{
				if (vecGraph_RearMaxHgt[a] > 250)
					continue;

				int nCurVal = vecGraphYVal_ErrDiff[a + 1] - vecGraphYVal_ErrDiff[a - 1];

				//fprintf(fp_FootWingSlope_Angle, "%d,", nCurVal);

				if (fDiffMax < nCurVal)
				{
					fDiffMax = nCurVal;
					nIdx = a;
				}
			}

			if (nIdx != 0)
				nTmpLastFoundIdx = nIdx + (nScanStartPoY + 1);
		}

		//fclose(fp_FootWingSlope_Angle);

#if _DEBUG
		//Wire start A￡Aº A§A¡ E®AI ±¸¹®
		CString csImgNum;
		csImgNum.Format(_T("D:\\FootInspImg\\00_FindWireStart.bmp"));
		std::string str = std::string(CT2CA(csImgNum));

		cv::String cvstr = (cv::String)str;

		if (nTmpLastFoundIdx != -1)
		{
			for (int a = 0; a < Foot_RemovedFlowDown.rows; a++)
			{
				UCHAR* Ptr_RstImg = cvFindWireStImg.data;
				for (int b = 0; b < Foot_RemovedFlowDown.cols; b++)
				{
					if (a == nTmpLastFoundIdx)
					{
						Ptr_RstImg[a * Foot_RemovedFlowDown.cols + b] = 255;
					}
				}
			}

			cv::imwrite(cvstr, cvFindWireStImg);
		}
#endif

		//°a°u°ª ¸®AI
		if (nTmpLastFoundIdx != -1)
		{
			(*nDetectedWireStartPosY) = nTmpLastFoundIdx;
			Ret = true;
		}
		else
			Ret = false;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::DetectWireStartPos_UseFootHeightProfile() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}

bool CPInsp_Wedge::DetectWireStartPos_UseLeftAndRightAreasOfFootHeight_2(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat FootBWRemovedFlowDown, cv::Mat inputFootLeftfH, cv::Mat inputFootRightfH, CRect rcLeftFootRefROI, CRect rcRightFootRefROI, cv::Point& CenterPo, cv::Point& Margin, bool bUseMeanSlopeAccum, int *nDetectedWireStartPosY)
{
	bool Ret = false;

	int nLine = __LINE__;

	try
	{
		//Foot Blob ¿μ¿ª °e≫e
		int ncvImgWidth = FootBWRemovedFlowDown.cols;
		int ncvImgHeight = FootBWRemovedFlowDown.rows;
		int nFootTmpMin_x = ncvImgWidth * ncvImgHeight, nFootTmpMax_x = 0;
		int nFootTmpMin_y = ncvImgWidth * ncvImgHeight, nFootTmpMax_y = 0;
		for (int r = 0; r < FootBWRemovedFlowDown.rows; r++)
		{
			UCHAR* Ptr = FootBWRemovedFlowDown.data;
			for (int c = 0; c < FootBWRemovedFlowDown.cols; c++)
			{
				if (Ptr[r*FootBWRemovedFlowDown.step + c] == 255)
				{
					if (c < nFootTmpMin_x) nFootTmpMin_x = c;
					if (c > nFootTmpMax_x) nFootTmpMax_x = c;
					if (r < nFootTmpMin_y) nFootTmpMin_y = r;
					if (r > nFootTmpMax_y) nFootTmpMax_y = r;
				}
			}
		}

		//foot BW °a°u ¾øA≫ ½A CO¼o ¸®AI
		if (nFootTmpMin_x == ncvImgWidth * ncvImgHeight && nFootTmpMin_y == ncvImgWidth * ncvImgHeight &&
			nFootTmpMax_x == 0 && nFootTmpMax_y == 0)
		{
			return false;
		}

		cv::Mat Left3DRange = inputFootLeftfH.clone();
		cv::Mat Right3DRange = inputFootRightfH.clone();

		//1. ³oAI data Line º°·I Æo±O, C￥AØÆiA÷ ±¸CI±a
		cv::Mat leftFootMeanVal_raw(Left3DRange.rows, 1, CV_32FC1);
		cv::Mat rightFootMeanVal_raw(Left3DRange.rows, 1, CV_32FC1);
		leftFootMeanVal_raw.setTo(0);
		rightFootMeanVal_raw.setTo(0);

		cv::Mat leftFootStdDevVal_raw(Left3DRange.rows, 1, CV_32FC1);
		cv::Mat rightFootStdDevVal_raw(Left3DRange.rows, 1, CV_32FC1);
		leftFootStdDevVal_raw.setTo(0);
		rightFootStdDevVal_raw.setTo(0);

		float meanVal_Left_raw = 0.0f, meanVal_Right_raw = 0.0f;
		float tmpMeanVal_Left_raw = 0.0f, tmpMeanVal_Right_raw = 0.0f;
		float fStdDevVal_Left_raw = 0.0f, fStdDevVal_Right_raw = 0.0f;
		float tmpfStdDevVal_Left_raw = 0.0f, tmpfStdDevVal_Right_raw = 0.0f;

		std::vector<int> vLeftFootPxCnt_raw;
		int nLeftFootPxCnt_raw = 0, nRightFootPxCnt_raw = 0;

		for (int y = nFootTmpMin_y; y < fHRoImg.rows; y++)
		{
			float* ptrLeftBlob = Left3DRange.ptr<float>(y);
			float* ptrRightBlob = Right3DRange.ptr<float>(y);

			float* ptrLeftMean_raw = leftFootMeanVal_raw.ptr<float>(y);
			float* ptrRightMean_raw = rightFootMeanVal_raw.ptr<float>(y);

			float* ptrLeftStdDev_raw = leftFootStdDevVal_raw.ptr<float>(y);
			float* ptrRightStdDev_raw = rightFootStdDevVal_raw.ptr<float>(y);

			for (int x = 1; x < fHRoImg.cols; x++)
			{
				if (x <= CenterPo.x - Margin.x && ptrLeftBlob[x] != 0.0f)
				{
					tmpMeanVal_Left_raw += ptrLeftBlob[x];
					nLeftFootPxCnt_raw++;

				}
				if (x >= CenterPo.x - Margin.x && ptrRightBlob[x] != 0.0f)
				{
					tmpMeanVal_Right_raw += ptrRightBlob[x];
					nRightFootPxCnt_raw++;
				}

			}

			//x¹æCa ³oAI Æo±O °e≫e
			vLeftFootPxCnt_raw.push_back(nLeftFootPxCnt_raw);
			ptrLeftMean_raw[0] = tmpMeanVal_Left_raw / (float)nLeftFootPxCnt_raw;
			ptrRightMean_raw[0] = tmpMeanVal_Right_raw / (float)nRightFootPxCnt_raw;
			if (isnan(ptrLeftMean_raw[0]))
				ptrLeftMean_raw[0] = 0;
			if (isnan(ptrRightMean_raw[0]))
				ptrRightMean_raw[0] = 0;

			for (int x = 1; x < fHRoImg.cols; x++)
			{
				if (x <= CenterPo.x - Margin.x && ptrLeftBlob[x] != 0.0f)
				{
					tmpfStdDevVal_Left_raw = ptrLeftMean_raw[0] - ptrLeftBlob[x];				//a' = Æo±O - μ￥AIAI°ª
					tmpfStdDevVal_Left_raw += tmpfStdDevVal_Left_raw * tmpfStdDevVal_Left_raw;		//a' A|°o

				}
				if (x >= CenterPo.x - Margin.x && ptrRightBlob[x] != 0.0f)
				{
					tmpfStdDevVal_Right_raw = ptrRightMean_raw[0] - ptrRightBlob[x];
					tmpfStdDevVal_Right_raw += tmpfStdDevVal_Right_raw * tmpfStdDevVal_Right_raw;
				}

			}

			//x¹æCa ³oAI C￥AØÆiA÷ °e≫e
			ptrLeftStdDev_raw[0] = (float)sqrtf(tmpfStdDevVal_Left_raw / (float)nLeftFootPxCnt_raw);
			ptrRightStdDev_raw[0] = (float)sqrtf(tmpfStdDevVal_Right_raw / (float)nRightFootPxCnt_raw);

			if (isnan(ptrLeftStdDev_raw[0]))
				ptrLeftStdDev_raw[0] = 0;
			if (isnan(ptrRightStdDev_raw[0]))
				ptrRightStdDev_raw[0] = 0;

			nLeftFootPxCnt_raw = 0;
			nRightFootPxCnt_raw = 0;
			tmpMeanVal_Left_raw = 0;
			tmpMeanVal_Right_raw = 0;
			tmpfStdDevVal_Left_raw = 0;
			tmpfStdDevVal_Right_raw = 0;
		}

		//2. Wing AC A¤≫o ¹uA§ °e≫e - ¾cAE footAC 1/3 AoA¡¸¸ ≫eAa
		int nWingAVGHeight = (fHRoImg.rows - nFootTmpMin_y) / 3;

		float fSumLeftWingHeight = 0, fSumRightWingHeight = 0;
		int nCntLeftWingCnt = 0, nCntRightWingCnt = 0;
		float fAVGLeftWingHeight = 0, fAVGRightWingHeight = 0;

		for (int y = nFootTmpMin_y; y < fHRoImg.rows; y++)
		{
			float* ptrLeftMean = (float*)leftFootMeanVal_raw.data;
			float* ptrRightMean = (float*)rightFootMeanVal_raw.data;

			if (y > CenterPo.y - Margin.y && y > nFootTmpMin_y + nWingAVGHeight && y < nFootTmpMin_y + nWingAVGHeight * 2)
			{
				fSumLeftWingHeight = fSumLeftWingHeight + ptrLeftMean[y];
				nCntLeftWingCnt++;

				fSumRightWingHeight = fSumRightWingHeight + ptrRightMean[y];
				nCntRightWingCnt++;
			}
		}

		fAVGLeftWingHeight = fSumLeftWingHeight / nCntLeftWingCnt;
		fAVGRightWingHeight = fSumRightWingHeight / nCntRightWingCnt;

		//. Wing A¤≫o±ºAC (Æo±O+C￥AØÆiA÷)*coef ¹uA§ ¹U¿¡ A¸Ac½A Wire Start EAº¸±ºA¸·I ¼±A¤ (Foot Aß½E A§A¡ ´eºn ¾Æ·¡AEA¸·I Scan)
		float fCoefToFindWire = 1.0f;

		if (bUseMeanSlopeAccum)
		{
			fCoefToFindWire = 1.1f;
		}
		else
		{
			fCoefToFindWire = 1.3f;
		}

		int nLeftMeanFoot_WireCandiPosY = fHRoImg.rows - 1;
		int nRightMeanFoot_WireCandiPosY = fHRoImg.rows - 1;

		bool bFindLeftWireStPo = false, bFindRightWireStPo = false;

		int nwireStartPosY = fHRoImg.rows - 1;
		int nCntLeftContinue = 0;
		int nCntRightContinue = 0;

		for (int y = nFootTmpMin_y; y < fHRoImg.rows; y++)
		{
			float* ptrLeftMeanImg = (float*)leftFootMeanVal_raw.data;
			float* ptrRightMeanImg = (float*)rightFootMeanVal_raw.data;

			if (y > CenterPo.y - Margin.y)
			{
				if (!bFindLeftWireStPo && ptrLeftMeanImg[y] > fAVGLeftWingHeight * fCoefToFindWire)
				{
					nLeftMeanFoot_WireCandiPosY = y;
					nCntLeftContinue++;
					if (nCntLeftContinue > 3)
						bFindLeftWireStPo = true;
				}

				if (!bFindRightWireStPo && ptrRightMeanImg[y] > fAVGRightWingHeight * fCoefToFindWire)
				{
					nRightMeanFoot_WireCandiPosY = y;
					nCntRightContinue++;
					if (nCntRightContinue > 3)
						bFindRightWireStPo = true;
				}

			}
		}

		if (bFindLeftWireStPo || bFindRightWireStPo)
		{
			if (bFindLeftWireStPo && bFindRightWireStPo)
			{
				if (nLeftMeanFoot_WireCandiPosY < nRightMeanFoot_WireCandiPosY)
					nwireStartPosY = nLeftMeanFoot_WireCandiPosY;
				else
					nwireStartPosY = nRightMeanFoot_WireCandiPosY;
			}
			else
			{
				if (bFindLeftWireStPo)
					nwireStartPosY = nLeftMeanFoot_WireCandiPosY;
				else
					nwireStartPosY = nRightMeanFoot_WireCandiPosY;
			}
		}

		(*nDetectedWireStartPosY) = nwireStartPosY;

		Ret = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::DetectWireStartPos_UseLeftAndRightAreasOfFootHeight() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;
}


bool CPInsp_Wedge::CalcFootHeightGradient_LeftAndRightAreas(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat inputFootLeftfH, cv::Mat inputFootRightfH, cv::Point& CenterPo, cv::Point& Margin, cv::Mat* LeftFootGradImg, cv::Mat* RightFootGradImg)
{
	bool Ret = false;

	int nLine = __LINE__;

	try
	{
		//Left/Right Foot º°·I x¹æCa AIAu CE¼¿°uAC ³oAI A÷(ÆiA÷)AI ±¸CI±a
		cv::Mat FootLeftAreafH = inputFootLeftfH;
		cv::Mat FootRightAreafH = inputFootRightfH;

		cv::Mat leftFootDiffVal(fHRoImg.rows, fHRoImg.cols, CV_32FC1);		//FootAC Left ºIºÐ x ¹æCa ³oAI A÷ °e≫eCN Image
		cv::Mat rightFootDiffVal(fHRoImg.rows, fHRoImg.cols, CV_32FC1);		//FootAC Right ºIºÐ x ¹æCa ³oAI A÷ °e≫eCN Image
		leftFootDiffVal.setTo(0);
		rightFootDiffVal.setTo(0);

		int nStartFlag_Left = false;
		int nStartPosX_Left = 0, nEndPosX_Left = 0;

		int nStartFlag_Right = false;
		int nStartPosX_Right = 0, nEndPosX_Right = 0;

		for (int y = 0; y < fHRoImg.rows; y++)
		{
			float* ptrLeftBlob = FootLeftAreafH.ptr<float>(y);
			float* ptrRightBlob = FootRightAreafH.ptr<float>(y);

			float* ptrLeftDiff = leftFootDiffVal.ptr<float>(y);
			float* ptrRightDiff = rightFootDiffVal.ptr<float>(y);

			for (int x = 1; x < fHRoImg.cols - 1; x++)
			{
				if (nStartFlag_Left == false && ptrLeftBlob[x] != 0.0f)
				{
					nStartFlag_Left = true;
					nStartPosX_Left = x;
				}
				if (nStartFlag_Left == true && ptrLeftBlob[x] == 0.0f)
				{
					nStartFlag_Left = false;
					nEndPosX_Left = x - 1;
				}

				if (nStartFlag_Right == false && ptrRightBlob[x] != 0.0f)
				{
					nStartFlag_Right = true;
					nStartPosX_Right = x;
				}
				if (nStartFlag_Right == true && ptrRightBlob[x] == 0.0f)
				{
					nStartFlag_Right = false;
					nEndPosX_Right = x - 1;
				}

				if (x <= CenterPo.x - Margin.x && nStartFlag_Left == true && x >= nStartPosX_Left)
				{
					ptrLeftDiff[x] = ptrLeftBlob[x - 1] - ptrLeftBlob[x + 1];
				}
				if (x >= CenterPo.x - Margin.x && nStartFlag_Right == true && x >= nStartPosX_Right)
				{
					ptrRightDiff[x] = ptrRightBlob[x - 1] - ptrRightBlob[x + 1];
				}

			}
			ptrLeftDiff[0] = ptrLeftDiff[1];
			ptrLeftDiff[fHRoImg.cols - 1] = ptrLeftDiff[fHRoImg.cols - 2];
			ptrLeftDiff[nStartPosX_Left] = ptrLeftDiff[nStartPosX_Left + 1];
			ptrLeftDiff[nEndPosX_Left] = ptrLeftDiff[nEndPosX_Left - 1];

			ptrRightDiff[0] = ptrRightDiff[1];
			ptrRightDiff[fHRoImg.cols - 1] = ptrRightDiff[fHRoImg.cols - 2];
			ptrRightDiff[nStartPosX_Right] = ptrRightDiff[nStartPosX_Right + 1];
			ptrRightDiff[nEndPosX_Right] = ptrRightDiff[nEndPosX_Right - 1];

			nStartFlag_Left = false;	nStartFlag_Right = false;

			nStartPosX_Left = 0, nEndPosX_Left = 0;
			nStartPosX_Right = 0, nEndPosX_Right = 0;
		}

		leftFootDiffVal.copyTo(*LeftFootGradImg);
		rightFootDiffVal.copyTo(*RightFootGradImg);

		Ret = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::CalcFootHeightGradient_LeftAndRightAreas() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;


}

bool CPInsp_Wedge::RotateBackToOriginalPosition(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat cvFootImg, cv::Mat inputImg, cv::Point& CenterPo, cv::Point& Margin, double& seta, cv::Mat* ResultImg)
{
	bool Ret = false;

	int nLine = __LINE__;

	try
	{

		cv::Mat rotatedInputImg = inputImg.clone();

		//UI¿¡ C￥½ACO AI¹IAo
		cv::Mat RstRoImg;
		cv::Mat WingOrg = cv::Mat(cvFootImg.rows, cvFootImg.cols, CV_8UC1);
		WingOrg.setTo(0);

		bool existWingRst = true;
		RstRoImg = rotatedInputImg;

		cv::Point Center;
		Center.x = CenterPo.x - Margin.x;
		Center.y = CenterPo.y - Margin.y;

		//cv::Point margin_New = WarpAffine(rotatedInputImg, RstRoImg, -(seta), WingOrg.size(), Center);	//Wing 3D Img E¸Au
		cv::Point margin_New = WarpAffine_IPPRotate(rotatedInputImg, RstRoImg, -(seta), WingOrg.size(), Center);	//Wing 3D Img E¸Au

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

		//ROI ¿μ¿ª ºn¾iAOA¸¸e A¶°C¹® ºuA®³ª°¨
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

			//E¸Au ¿μ≫oAC Aß½EA¡°u Rect ½AAUA§A¡ A÷AI °ª °e≫e
			int diffWingCX = (CenterPo.x - Margin.x) - nWingTmpMin_x;
			int diffWingCY = (CenterPo.y - Margin.y) - nWingTmpMin_y;

			//°E≫c°a°u AU¸￥ AI¹IAo
			cv::Mat WingRectRst = RstRoImg(cv::Rect(nWingTmpMin_x, nWingTmpMin_y, nWingTmpMax_x - nWingTmpMin_x, nWingTmpMax_y - nWingTmpMin_y));

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

			//InspRect¿¡ ¸A°O AOA¾ °a°u ¿μ≫o ≫y¼º
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
		errorLog.Format(_T("[FootAlgo]::RotateBackToOriginalPosition() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return Ret;

}

//************************************
// Method:    GetFootCenterPos / GetFootSeta / GetFootLength
// Qualifier:
// Æ¼AªμE Wing AI¹IAo ≫c¿eCI¿ⓒ CoAc FootAC Aß½EA¡, °￠μμ ¹× Foot ±æAI ±¸CI´A CO¼o
//************************************
bool CPInsp_Wedge::getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection)
{
	//NYJ Wing AI¹IAo ≫c¿eCI¿ⓒ CoAc FootAC Aß½EA¡, °￠μμ ¹× Foot ±æAI ±¸CI´A CO¼o

	cv::Mat Img = image;		//Wing Image
	cv::Mat mMask = mMaskImage;		//Foot Image
	/*cv::imwrite("D:\\FootRst\\getFootCenterPosAndSeta_Input_Img.bmp", Img);
	cv::imwrite("D:\\FootRst\\getFootCenterPosAndSeta_Mask_Img.bmp", mMask);*/

	//Wing ºIºÐ Foot ¿μ¿ª°u ¸¶½ºA· (CI³ªAC pad¿¡ μI °³AC Foot A¸AcCO ½A, ½CA| °E≫c AßAI Wing AßAaA≫ A§CØ ¸¶½ºA·)
	//Img &= mMask;		//Wing°u Foot AI¹IAo ¸¶½ºA·


	cv::Point2d WingLength;
	WingLength = WingLen;
	cv::Point FootCenterPo;

	double seta_Foot(0.0);
	bool flag = Insp_FootPosition(&Img, &FootCenterPo, &seta_Foot, mMask, nFootDirection, WingLength);

	//WingBW ¿μ≫oA¸·I Aß½E A|´e·I ¸øA￡Aº °æ¿i
	if (FootCenterPo.x <0 || FootCenterPo.x > Img.cols || FootCenterPo.y <0 || FootCenterPo.y > Img.rows)
	{
		flag = false;
		return flag;
	}

	CenterPo = FootCenterPo;
	seta = seta_Foot;
	WingLen = WingLength;

#if _DEBUG
	cv::imwrite("D:\\FootRst\\getFootCenterPosAndSeta_OUTPUT_Img.bmp", Img);
#endif

	return flag;
}

bool CPInsp_Wedge::Insp_FootPosition(cv::Mat* vImage, cv::Point* CenterPo, double* seta, cv::Mat FindImage, int nFootDirection, cv::Point2d & length)
{
	int nLine = __LINE__;
	try
	{
		//	if(bBotWhite)
		{
			cv::Mat LargeImg = cv::Mat::zeros(vImage->rows + 40, vImage->cols + 40, CV_8UC1);
			cv::Mat roiImg = LargeImg(cv::Rect(20, 20, vImage->cols, vImage->rows));

			vImage->copyTo(roiImg);
#if _DEBUG
			cv::imwrite("D:\\FootRst\\Insp_FootPosition_INPUT_Img.bmp", roiImg);
#endif
			cv::Mat tmpMorp1, tmpMorp2;
			cv::morphologyEx(LargeImg, tmpMorp1, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)), cv::Point(-1, -1), 1);
			cv::morphologyEx(tmpMorp1, tmpMorp2, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)), cv::Point(-1, -1), 1);

			// YJS(190625): Foot Angle Rotate
			double dFirstAng(0);
			if (Calc_FootAng(&tmpMorp2, dFirstAng) == false)
				return false;

			cv::Mat rImg = Rot_FootAng(&tmpMorp2, dFirstAng);

			double dSecondAng(0);
			if (Calc_FootAng(&rImg, dSecondAng) == false)
				return false;

			double fResAng = dFirstAng + dSecondAng;
			double fResAngOrg = 90.0 - fResAng;

			cv::Mat m_img_dst = Rot_FootAng(&tmpMorp2, fResAng, false);

			cv::Mat mLenHist = cv::Mat::zeros(1, m_img_dst.cols, CV_32SC1);
			int * pLenHist = mLenHist.ptr<int>(0);
			for (int y = 0; y < m_img_dst.rows; y++)
			{
				BYTE * dstPtr = m_img_dst.ptr(y);
				for (int x = 0; x < m_img_dst.cols; x++)
				{
					if (dstPtr[x] > 250)
					{
						pLenHist[x]++;
					}
				}
			}

			CHistogramAnalysis_New ha_Pos;
			ha_Pos.Run(mLenHist);

			if (ha_Pos.m_PeakList.size() < 2)
				return false;

			int nPeakIdx[2] = { -1, };
			int nPeakVal[2] = { -1, };
			int nPeakStVal[2] = { -1, };
			int nPeakPosX[2] = { -1, };
			int nPeakPosY[2] = { -1, };
			double dPeakVal[2] = { FLT_MIN, };
			for (int m = 0; m < 2; m++)
			{
				for (int n = 0; n < ha_Pos.m_PeakList.size(); n++)
				{
					if (m == 0)
					{
						if (dPeakVal[m] < ha_Pos.m_PeakList[n].dValue)
						{
							dPeakVal[m] = ha_Pos.m_PeakList[n].dValue;
							nPeakPosX[m] = ha_Pos.m_PeakList[n].nIndex;
							nPeakStVal[m] = std::max(ha_Pos.m_PeakList[n].dStRmsVal, ha_Pos.m_PeakList[n].dEdRmsVal);
							nPeakIdx[m] = n;
						}
					}
					else
					{
						if (nPeakPosX[0] != ha_Pos.m_PeakList[n].nIndex &&
							dPeakVal[m] < ha_Pos.m_PeakList[n].dValue)
						{
							dPeakVal[m] = ha_Pos.m_PeakList[n].dValue;
							nPeakPosX[m] = ha_Pos.m_PeakList[n].nIndex;
							nPeakStVal[m] = std::max(ha_Pos.m_PeakList[n].dStRmsVal, ha_Pos.m_PeakList[n].dEdRmsVal);
							nPeakIdx[m] = n;
						}
					}
				}

				nPeakVal[m] = pLenHist[nPeakPosX[m]] - nPeakStVal[m];
			}

			for (int m = 0; m < 2; m++)
			{
				int nMaxHistIdx = -1;
				double nMaxHistVal = INT_MIN;
				for (int y = m_img_dst.rows - nPeakVal[m]; y >= 0; y--)
				{
					int nSumGrad(0);
					for (int j = 0; j < nPeakVal[m]; j++)
					{
						nSumGrad += m_img_dst.ptr(y + j)[nPeakPosX[m]];
					}

					if (nSumGrad > nMaxHistVal)
					{
						nMaxHistVal = nSumGrad;
						nMaxHistIdx = y;
					}
				}

				nPeakPosY[m] = nMaxHistIdx + nPeakVal[m] / 2;
			}

			cv::Point2f dst_center(m_img_dst.cols / 2.0, m_img_dst.rows / 2.0);

			double fInverseAngle = fResAngOrg - 90.0f;
			cv::Point2f point_center(LargeImg.cols / 2.0, LargeImg.rows / 2.0);
			cv::Mat rotInv = cv::getRotationMatrix2D(point_center, fInverseAngle, 1.0);
			cv::Mat rotPos = cv::Mat(3, 1, CV_64FC1);

			*seta = nFootDirection == true ? (90 - fResAngOrg) : (90 - fResAngOrg) + 180;
			double ctX = (nPeakPosX[0] + nPeakPosX[1]) *0.5;
			double ctY = nPeakVal[0] >= nPeakVal[1] ? nPeakPosY[0] : nPeakPosY[1];

			rotPos.ptr<double>(0)[0] = ctX - dst_center.x;
			rotPos.ptr<double>(1)[0] = ctY - dst_center.y;
			rotPos.ptr<double>(2)[0] = 0;
			cv::Mat rotPos_ = rotInv * rotPos;

			CenterPo->x = point_center.x + rotPos_.ptr<double>(0)[0] - 20;
			CenterPo->y = point_center.y + rotPos_.ptr<double>(1)[0] - 20;

			length.x = nPeakVal[0];
			length.y = nPeakVal[1];
			//
		}

		return true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::Insp_FootPosition() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
		throw;
	}
	return true;
}

bool CPInsp_Wedge::Insp_FootPadLength(cv::Mat* vImage, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, cv::Mat Img3D, bool bFindHill)
{
	int AvgW(0);
	std::vector<POINT> vst;
	std::vector<POINT> ved;

	cv::Mat Intg, Dev;
	cv::Mat Img = (*vImage).clone();
	Img &= 1;
	cv::integral(Img, Intg, Dev);

	float fValue = Intg.cols / 5;
	for (int r = 1; r < Intg.rows - 1; r++)
	{
		int* intPtr_1 = Intg.ptr<int>(r - 1);
		int* intPtr = Intg.ptr<int>(r);
		UCHAR* ImgPtr = Img.ptr(r);
		if (intPtr[Intg.cols - 1] - intPtr_1[Intg.cols - 1] < 1)
			continue;

		POINT st, ed;
		st.x = 0;
		st.y = 0;
		ed.x = 0;
		ed.y = 0;
		for (int c = 0; c < Img.cols - 1; c++)
		{
			if (ImgPtr[c] > 0)
			{
				st.x = c - 2;
				st.y = r;
				break;
			}
		}
		for (int c = Img.cols - 1; c > 0; c--)
		{
			if (ImgPtr[c] > 0)
			{
				ed.x = c;
				ed.y = r;
				break;
			}
		}
		vst.push_back(st);
		ved.push_back(ed);
	}
	int nSum(0);
#if _DEBUG
	std::vector<int> vdiff;
#endif
	for (int i = 0; i < vst.size(); i++)
	{
		nSum += (ved[i].x - vst[i].x);
#if _DEBUG
		vdiff.push_back((ved[i].x - vst[i].x));
#endif
	}
	if (vst.size() < 1)
	{
#if _DEBUG
		cv::imwrite("D:\\FootRst\\Bin2D_BondingImg.bmp", Img);
#endif
		g_pMPTI->AddLog(_T("PadLength Find error"));
		//AfxMessageBox(_T("PadLength Find error"));
		return false;
	}
	AvgW = nSum / vst.size();
	//for mobis wire
	float stdWireW = 260 / (m_resolX * 1000);
	if (AvgW > stdWireW)
		AvgW = stdWireW;
	std::vector<int> LengthSt, LengthEd;
	std::vector<int> LStIndex, LEdIndex;
	bool bSt = false;
	int startCnt = 150 / (m_resolY * 1000);
	for (int i = 0; i < vst.size(); i++)
	{
		if (ved[i].x - vst[i].x < 1)
			continue;
		startCnt--;
		if (startCnt > 10)
			continue;
		int tempW = ved[i].x - vst[i].x;
		if (!bSt && tempW > AvgW)
		{
			LengthSt.push_back(vst[i].y);
			LStIndex.push_back(i);
			bSt = true;
		}


		if (bSt && tempW < AvgW)
		{
			LengthEd.push_back(ved[i].y);
			LEdIndex.push_back(i);
			bSt = false;
		}
	}
	if (LStIndex.size() < 1 || LengthSt.size() < 1)
	{
		LengthSt.push_back(0);
		LStIndex.push_back(0);
		bSt = false;
	}

	if (LEdIndex.size() < 1 || LengthEd.size() < 1)
	{
		LengthEd.push_back(Img.rows - 1);
		LEdIndex.push_back(0);
		bSt = false;
	}

	//shkim 20pixel
	int permit = 20 / (m_resolY * 1000);
	if (LengthSt.size() > 1 && LengthEd.size() > 1)
	{
		for (int i = 1; i < LengthSt.size(); i++)
		{
			int diff = LengthSt[i] - LengthEd[i - 1];
			if (diff < permit)
			{
				LengthSt.erase(LengthSt.begin() + i);
				LStIndex.erase(LStIndex.begin() + i);
				LengthEd.erase(LengthEd.begin() + i - 1);
				LEdIndex.erase(LEdIndex.begin() + i - 1);
			}
		}
	}
	int nMaxLength(0), LIndex(0);

	for (int i = 0; i < LengthEd.size(); i++)
	{
		int tempLen = LengthEd[i] - LengthSt[i];
		if (nMaxLength < tempLen)
		{
			LIndex = i;
			nMaxLength = tempLen;
		}
	}
	if (LEdIndex.size() < 1 || LengthEd.size() < 1)
		return false;

	cv::Point pLStTmp, pLEdTmp, pRStTmp, pREdTmp;

	if (bFindHill)
	{
		pLStTmp.x = vst[LStIndex[LIndex]].x;
		pLEdTmp.x = vst[ved.size() - 1].x;

		pLStTmp.y = LengthSt[LIndex];
		pLEdTmp.y = Img.rows - 1;

		pRStTmp.x = ved[LStIndex[LIndex]].x;
		pREdTmp.x = ved[ved.size() - 1].x;

		pRStTmp.y = LengthSt[LIndex];
		pREdTmp.y = Img.rows - 1;
	}
	else
	{
		pLStTmp.x = vst[LStIndex[LIndex]].x;
		pLEdTmp.x = vst[LEdIndex[LIndex]].x;

		pLStTmp.y = LengthSt[LIndex];
		pLEdTmp.y = LengthEd[LIndex];

		pRStTmp.x = ved[LStIndex[LIndex]].x;
		pREdTmp.x = ved[LEdIndex[LIndex]].x;

		pRStTmp.y = LengthSt[LIndex];
		pREdTmp.y = LengthEd[LIndex];
	}
	//if (bFindHill && topPo > 0)
	//{
	//	pLStTmp.x = vst[LStIndex[LIndex]].x;
	//	pLStTmp.y = LengthSt[LIndex];
	//	pRStTmp.x = ved[LStIndex[LIndex]].x;
	//	pRStTmp.y = LengthSt[LIndex];
	//}

	pLEd->x = pLEdTmp.x;
	pLEd->y = pLEdTmp.y;
	pREd->x = pREdTmp.x;
	pREd->y = pREdTmp.y;
	cv::Point nNeck;
	nNeck.x = 0;
	nNeck.y = pLStTmp.y;
	int HarfR = (pLEdTmp.y + pLStTmp.y) / 2;

	for (int r = nNeck.y; r < HarfR; r++)
	{
		UCHAR* ImgPtr = Img.ptr(r);

		for (int c = 0; c < Img.cols - 1; c++)
		{
			if (ImgPtr[c] < 1)
				continue;
			if (nNeck.x < c)
			{
				nNeck.y = r;
				nNeck.x = c;
			}
			break;
		}
	}

	pLSt->x = pLEdTmp.x;//nNeck.x;
	pLSt->y = nNeck.y;


	nNeck.x = pRStTmp.x;
	for (int r = nNeck.y; r < HarfR; r++)
	{
		UCHAR* ImgPtr = Img.ptr(r);

		for (int c = Img.cols - 1; c > 0; c--)
		{
			if (ImgPtr[c] < 1)
				continue;
			if (nNeck.x > c)
			{
				nNeck.y = r;
				nNeck.x = c;
			}
			break;
		}
	}
	pRSt->x = pREdTmp.x; //nNeck.x;
	pRSt->y = nNeck.y;

	pLEd->x = pLEdTmp.x;
	pLEd->y = pLEdTmp.y;
	pREd->x = pREdTmp.x;
	pREd->y = pREdTmp.y;

	//return true;
	//3d INSP//------------------------------------------------------------------------------------------
	cv::Mat roImg = *vImage;
	std::vector<cv::Point> Cen;
	std::vector<float> HightVector;
	Cen.clear();
	Cen.assign(roImg.rows, cv::Point(0, 0));
	HightVector.assign(roImg.rows, 0.0f);

	int nEndY = pREd->y > pLEd->y ? pREd->y : pLEd->y;
	for (int r = 0; r < nEndY; r++)
	{
		int stX(0), edX(0);
		UCHAR* uPtr = roImg.ptr(r);
		for (int c = 0; c < roImg.cols; c++)
		{
			if (uPtr[c] > 0)
			{
				stX = c;
				break;
			}
		}

		for (int c = roImg.cols - 1; c > stX; c--)
		{
			if (uPtr[c] > 0)
			{
				edX = c;
				break;
			}
		}

		Cen[r].x = (edX + stX) / 2;
		Cen[r].y = r;
		if (edX > stX)
		{
			float* fPtr = Img3D.ptr<float>(r);
			float fSum = 0.0f;
			for (int c = stX; c <= edX; c++)
				fSum += fPtr[c];
			HightVector[r] = fSum / (edX - stX);
		}
	}

	cv::Point Min(0, 0), max(0, 0);
	int MaxX = 0;
	int MinX = 0;

	for (int r = pLStTmp.y + 30; r < Cen.size() - 1; r++)
	{
		if (HightVector[r] > MaxX)
		{
			max = Cen[r];
			MaxX = (int)HightVector[r];
		}
	}
	MinX = MaxX;
	for (int r = pLStTmp.y + 30; r < max.y; r++)
	{
		if (HightVector[r] < MinX)
		{
			Min = Cen[r];
			MinX = (int)HightVector[r];
		}
	}

	if (Min.y < vst[0].y || max.y - Min.y == 0)
		return true;
	float a = (MaxX - MinX) / (max.y - Min.y);
	float b = MinX;
	float fMaxDiff(0.0f), fMinDiff(0.0f);
	cv::Point MaxDiff(0, 0);
	MaxDiff.x = pLEdTmp.x;
	MaxDiff.y = pLEdTmp.y;

	for (int r = Min.y + 1; r < HightVector.size(); r++)
	{
		float value = a * (Cen[r].y - Min.y) + b;

		if (value - HightVector[r] > fMaxDiff)
		{
			MaxDiff = Cen[r];
			fMaxDiff = value - HightVector[r];
		}
		//		if (value - HightVector[r] < fMinDiff)
		//		{
		//			pLStTmp.x = Cen[r].x;
		//			pLStTmp.y = Cen[r].y;
		//			fMinDiff = value - HightVector[r];
		//		}
	}
	if (MaxDiff.y < max.y)
		return true;
	if (MaxDiff.y - vst[0].y >= vst.size())
		return true;
	if (vst.size() < 1)
		return true;

	pLEdTmp.x = MaxDiff.x;
	pLEdTmp.y = MaxDiff.y;


	pLEd->x = vst[MaxDiff.y - vst[0].y].x;
	pLEd->y = MaxDiff.y;
	pREd->x = ved[MaxDiff.y - vst[0].y].x;
	pREd->y = MaxDiff.y;
	//	pLSt->x = vst[pLStTmp.y - vst[0].y].x;
	//	pLSt->y = pLStTmp.y;
	//	pRSt->x = ved[pRStTmp.y - vst[0].y].x;
	//	pRSt->y = pRStTmp.y;

	return true;
}

bool CPInsp_Wedge::getFootLengthEndPoint(CFoot_Model* pFoot, cv::Mat* vImage, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, int CntX, float WedgeMinSpec)
{
	int stdM = 210 / (m_resolX * 1000);
	int StdK = 49 / (m_resolX * 1000);  //defalut 50

	cv::Mat Intg, Dev;

	bool bIsDetected = false;

	int nLine = __LINE__;
	try
	{
		cv::Mat localImg = vImage->clone();
#if _DEBUG
		cv::imwrite("D:\\FootRst\\getFootLengthEndPoint_INPUT_Img.bmp", localImg);
#endif
		localImg &= 1;
		cv::integral(localImg, Intg, Dev);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\getFootLengthEndPoint_Integral_Img.bmp", Intg);
#endif
		//int CntX = localImg.cols / 2;

		int LeftBondingPntScanStCnt = 0;
		int LeftBondingPntScanStIdx = 0;
		int LeftBondingPntScanEndIdx = 0;
		int LeftBondingPntScanStX = 0;
		int LeftBondingPntScanStY = 0;
		int LeftBondingPntScanEndX = 0;
		int LeftBondingPntScanEndY = 0;

		int RightBondingPntScanStCnt = 0;
		int RightBondingPntScanStIdx = 0;
		int RightBondingPntScanEndIdx = 0;
		int RightBondingPntScanStX = 0;
		int RightBondingPntScanStY = 0;
		int RightBondingPntScanEndX = 0;
		int RightBondingPntScanEndY = 0;

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
					vSt[r - 1] = c;		//wing °¡·I ±æAI°¡ 0AI≫oAI °æ¿i St A§A¡·I AoA¤
					LeftBondingPntScanStCnt++;
					break;
				}
			}
			for (int c = CntX; c < localImg.cols - 1; c++)
			{
				if (uPtr[c] > 0)
				{
					vEd[r - 1] = c;		//wing ±æAI°¡ 0AI≫oAI °æ¿i Ed A§A¡·I AoA¤
					RightBondingPntScanStCnt++;
					break;
				}
			}
			if (LeftBondingPntScanStCnt == 1)
			{
				LeftBondingPntScanStX = vSt[r - 1];
				LeftBondingPntScanStIdx = r - 1;
			}
			if (RightBondingPntScanStCnt == 1)
			{
				RightBondingPntScanStX = vEd[r - 1];
				RightBondingPntScanStIdx = r - 1;
			}

			int ndif = vEd[r - 1] - vSt[r - 1] > 0 ? vEd[r - 1] - vSt[r - 1] : 0;
			if (vSt[r - 1]<1 || vEd[r - 1]>localImg.cols - 2)
				continue;
			vHist[ndif]++;
			vCnt[r] = ndif;
		}

		int nMaxidx(0), nMaxCnt(0);
		for (int i = 1; i < vHist.size(); i++)
		{
			if (vHist[i] > nMaxCnt)
			{
				nMaxCnt = vHist[i];
				nMaxidx = i;
			}
		}
		int nGap = 6;
		int nStdMxL = 75 / (m_resolX * 1000);
		int nMaxLength = nStdMxL;
		int nDamageSt(0), nDamageEd(0);
		int stCntIdx = 0, edCntIdx = 0;
		for (int i = 0; i < vCnt.size(); i++)
		{
			if (vCnt[i]< nMaxidx + nGap && vCnt[i] > nMaxidx - nGap)
			{
				stCntIdx = i;
				break;
			}
		}
		for (int i = vCnt.size() - 1; i > 1; i--)
		{
			if (vCnt[i]< nMaxidx + nGap && vCnt[i] > nMaxidx - nGap)
			{
				edCntIdx = i;
				break;
			}
		}

		if (stCntIdx == 0 || edCntIdx == vImage->rows)		//Wing Point ¸øA￡A≫ °æ¿i ¿¹¿UA³¸®
		{
			bIsDetected = false;
			return false;
			// 			if (LeftBondingPntScanStX != 0)
			// 			{
			// 				pLSt->x = LeftBondingPntScanStX;
			// 				pLSt->y = LeftBondingPntScanStIdx;
			// 				pLEd->x = LeftBondingPntScanStX - 1;
			// 				pLEd->y = LeftBondingPntScanStIdx - 1;
			// 			}
			// 			if (RightBondingPntScanStX != 0)
			// 			{
			// 				pRSt->x = RightBondingPntScanStX;
			// 				pRSt->y = RightBondingPntScanStIdx;
			// 				pREd->x = RightBondingPntScanStX - 1;
			// 				pREd->y = RightBondingPntScanStIdx - 1;
			// 			}
			// 
			// 			if (LeftBondingPntScanStX == 0 && RightBondingPntScanStX != 0)
			// 			{
			// 				pLSt->x = pRSt->x -1;
			// 				pLSt->y = pRSt->y -1;
			// 				pLEd->x = pREd->x -1;
			// 				pLEd->y = pREd->y -1;
			// 			}
			// 			else if (LeftBondingPntScanStX != 0 && RightBondingPntScanStX == 0)
			// 			{
			// 				pRSt->x = pLSt->x -1;
			// 				pRSt->y = pLSt->y -1;
			// 				pREd->x = pLEd->x -1;
			// 				pREd->y = pLEd->y -1;
			// 			}
		}
		else
		{
			pLSt->x = vSt[stCntIdx];
			pLSt->y = stCntIdx;
			pLEd->x = vSt[edCntIdx - 1];
			pLEd->y = edCntIdx - 1;

			pRSt->x = vEd[stCntIdx];
			pRSt->y = stCntIdx;
			pREd->x = vEd[edCntIdx - 1];
			pREd->y = edCntIdx - 1;
		}

		for (int r = vImage->rows - 1; r > 1; r--)
		{
			UINT* UnPtr = Intg.ptr<UINT>(r);
			UINT* UnPtr_1 = Intg.ptr<UINT>(r - 1);
			if (UnPtr[Intg.cols - 1] - UnPtr_1[Intg.cols - 1] < StdK * 2)
				continue;
			//if (vthickness[r] > stdth+5 || vthickness[r] < stdth - 5)
			//	continue;

			UCHAR* uPtr = vImage->ptr(r);
			int st(0), ed = vImage->cols;
			for (int c = 0; c < vImage->cols; c++)
			{
				if (uPtr[c] > 0)
				{
					st = c;
					break;
				}
			}
			for (int c = vImage->cols; c > 0; c--)
			{
				if (uPtr[c] > 0)
				{
					ed = c;
					break;
				}
			}
			if (ed - st < stdM)
				continue;

			int CentX = (ed + st) / 2;

			int Lst = st, Led = CentX;
			int Rst = CentX, Red = ed;

			int LCnt(0), RCnt(0);

			for (int c = st; c < CentX; c++)
			{
				if (uPtr[c] > 0)
				{
					LCnt++;
					Led = c;
				}
			}
			for (int c = ed; c > CentX; c--)
			{
				if (uPtr[c] > 0)
				{
					RCnt++;
					Rst = c;
				}
			}

			if (LCnt < StdK || RCnt < StdK)
				continue;

			pLEd->x = Led;
			pLEd->y = r;
			pREd->x = Rst;
			pREd->y = r;

			break;

		}


		bIsDetected = true;

	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::getFootLengthEndPoint() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return bIsDetected;
}

void CPInsp_Wedge::MaskFootDiffX(cv::Mat Img3D, cv::Mat BinWing, cv::Mat* dst, int nMin, int nMag)
{
#if _DEBUG
	cv::imwrite("D:\\FootRst\\MaskFootDiffX_INPUT_Img.bmp", BinWing);
#endif
	cv::Mat sobelX(Img3D.rows, Img3D.cols, CV_16SC1);
	cv::Mat NoX, NoY, No3D, NoMag;
	cv::Mat ConvX, ConvY, Conv3D, ConvMag;

	sobelX.setTo(0);
	for (int r = 0; r < sobelX.rows; r++)
	{
		float* fPtrX = Img3D.ptr<float>(r);
		short* fPtrSobel = sobelX.ptr<short>(r);
		for (int c = 1; c < sobelX.cols - 1; c++)
		{
			float magX_P1 = abs(fPtrX[c + 1]);
			float magX_M1 = abs(fPtrX[c - 1]);

			float Hdiff = std::abs(magX_P1 - magX_M1);
			if (Hdiff > nMin && Hdiff < nMag)
				fPtrSobel[c] = 255;
		}
	}

	cv::normalize(Img3D, No3D, 0, 255, cv::NORM_MINMAX);

	No3D.convertTo(Conv3D, CV_8UC1);
	sobelX.convertTo(ConvX, CV_8UC1);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\MaskFootDiffX_3D_sobelX_Img.bmp", sobelX);
#endif
	cv::Mat re = ConvX & BinWing;
	cv::Mat BinImage(re.rows, re.cols, CV_8UC1);

	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	UCHAR *pUcImgBinary = BinImage.data;
	double dArea, dCX, dCY;
	CRect rcBlob;
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
	algoBlob.m_nTypeRange2D = 2;
	algoBlob.m_nMinBinary = 0;
	algoBlob.m_nMaxBinary = 100;
	algoBlob.m_bFillHole = true;

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;
	dCX = 0, dCY = 0, dArea = 0;
	rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
	int nCntBlob = CPInsp::BlobImageStruct(algoBlob, re.data, Img3D.ptr<float>(0), NULL, re.cols, re.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);

	cv::Mat moph = BinImage.clone();
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
	cv::dilate(moph, moph, kernel);
	cv::dilate(moph, moph, kernel);
	cv::erode(moph, moph, kernel);
	cv::erode(moph, moph, kernel);

	cv::erode(moph, moph, kernel);
	cv::dilate(moph, moph, kernel);
#if _DEBUG
	cv::imwrite("D:\\FootRst\\MaskFootDiffX_OUTPUT_Img.bmp", moph);
#endif
	moph.copyTo(*dst);
}

cv::Point CPInsp_Wedge::FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, cv::Mat *cvFindPadImg, cv::Mat *cvFindFootImg)
{

	int nLine = __LINE__;
	cv::Point CenterPo;
	try
	{
		if (Images.size() == 0)
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;

			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);
				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;
			}
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("[FootAlgo]::FootMeasure Images.size() == 0"));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			sRstAlgo->m_bOK = FALSE;
			return CenterPo;
		}

		cv::Mat cvFootImg = Images[0].clone();
		cv::Mat cvWedgeImg = Images[1].clone();
		cv::Mat cvWing = Images[2].clone();
		cv::Mat cv3DImg = Images[3].clone();
		cv::Mat PadImg = Images[4].clone();
//#if _DEBUG
//		cv::imwrite("D:\\FootRst\\Find_Foot.bmp", *cvFootImg);
//		cv::imwrite("D:\\FootRst\\Find_Wedge.bmp", *cvWedgeImg);
//		cv::imwrite("D:\\FootRst\\Find_Wing.bmp", *cvWing);
//		cv::imwrite("D:\\FootRst\\Find_PadImg.bmp", *PadImg);
//#endif

		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_Find_Foot.bmp"), cvFootImg);
			cv::imwrite(rawname + std::string("_Find_Wedge.bmp"), cvWedgeImg);
			cv::imwrite(rawname + std::string("_Find_PadImg.bmp"), PadImg);

		}

		////////1. Foot, Wing 영상 유무 확인/////
		double dArea, dCX, dCY;
		CRect rcBlob;

		AlgoBlob algoBlob;
		//PIAL::_AlgoBlob algoBlob;

		algoBlob.m_bUseIPC = false;
		algoBlob.m_bInvertCheck = false;
		algoBlob.m_bFilterIsUse = false;
		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		algoBlob.m_bInsp3D = false;
		algoBlob.m_nTypeRange3D = 2;
		algoBlob.m_dHeightRateMin = 0;
		algoBlob.m_dHeightRateMax = 0;
		algoBlob.m_bInsp2D = true;
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 0;
		algoBlob.m_nMaxBinary = 100;
		algoBlob.m_bFillHole = false;

		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

		cv::Mat canny3dfoot_img(wingCandiImgSec->rows, wingCandiImgSec->cols, CV_32FC1);
		// 		PIAL::PI_Buff org2DSrc(*wingCandiImgSec);
		// 		PIAL::PI_Buff org3DSrc(canny3dfoot_img.ptr<float>(0), canny3dfoot_img.cols, canny3dfoot_img.rows, canny3dfoot_img.cols * 4);
		// 
		// 		PIAL::PI_Buff pTempImgBlob(canny3dfoot_img.cols, canny3dfoot_img.rows);

		cv::Mat SobelXAndY;
		SobelXAndY = (*wingCandiImgSec).clone();
		cv::Mat BinImage(wingCandiImgSec->rows, wingCandiImgSec->cols, CV_8UC1);
		int nCntBlob = CPInsp::BlobImageStruct(algoBlob, SobelXAndY.data, canny3dfoot_img.ptr<float>(0), NULL, SobelXAndY.cols, SobelXAndY.rows, 10, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);
		//int nWingCntBlob = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2DSrc, &org3DSrc, NULL, &pTempImgBlob, wingCandiImgSec->cols, wingCandiImgSec->rows, 10,
		//	&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);

		//BinImage = pTempImgBlob.Mat();

		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_Find_Wing_Rotate.bmp"), SobelXAndY);
		}

		if (nCntBlob == 0)
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;

			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);

				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;

			}

#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("[FootAlgo]::FootMeasure nCntBlob == 0"));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			sRstAlgo->m_bOK = FALSE;
			return CenterPo;
		}
		////////

		cv::Mat FootImg, retMatIntg, retDev;
		FootImg = (cvFootImg).clone();
		cv::integral(FootImg, retMatIntg, retDev);
		UINT* unREPtr = retMatIntg.ptr<UINT>(retMatIntg.rows - 1);
		if (unREPtr[retMatIntg.cols - 1] <= 0)
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);
				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;
			}
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("[FootAlgo]::FootMeasure unREPtr[retMatIntg.cols - 1] <= 0"));
			g_pMPTI->AddLog_OCR(sLog);
			cv::imwrite("D:\\FootRst\\FootMeasure_FootImg.bmp", FootImg);
#endif
			sRstAlgo->m_bOK = FALSE;
			return CenterPo;
		}

		//initialize 
		int nWidth = pFoot->GetImageWidth();
		int nHeight = pFoot->GetImageLength();
		cv::Rect ImageRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);

		sRstAlgo->m_bFind = TRUE;

		//2. Foot CenterPos, Seta E®AI ¹× Foot AøA¤A§A¡ º¤AI ¼³A¤
		cv::Mat Img;
		cv::Mat mMask;
		mMask = cvFootImg.clone();
		Img = cvWing.clone();

		cv::Mat fHRoImg;
		fHRoImg = cv3DImg.clone();

		double seta(0.0), seta1(0.0);
		bool flag = matchAngle == -INFINITY ? false : true;
		seta = matchAngle;

		CenterPo.x = CenterPos.x;
		CenterPo.y = CenterPos.y;

		float a = std::tan(seta*PI / 180);
		float b = -a * CenterPo.x + CenterPo.y;

		cv::Point CentLineSt(0, 0), CentLineEd(0.0);
		CentLineSt.x = 0;
		CentLineSt.y = b;
		CentLineEd.x = ImageRect.width;
		CentLineEd.y = a * ImageRect.width + b;

		nLine = __LINE__;

		seta1 = (seta);
		sRstAlgo->m_fAngle = seta1;

		nLine = __LINE__;

		if (flag == false)	//missing
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);
				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;
			}
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("[FootAlgo]::FootMeasure flag == false //missing"));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			sRstAlgo->m_bOK = FALSE;

			return CenterPo;
		}
		//

		std::vector<cv::Point> vPoSt((int)m_eFoot::m_eFoot_Total);
		std::vector<cv::Point> vPoEd((int)m_eFoot::m_eFoot_Total);

		for (int poIdx = 0; poIdx < m_eFoot::m_eFoot_Total; poIdx++)
		{
			vPoSt[poIdx].x = sRstAlgo->rePoinSt[poIdx].x;
			vPoSt[poIdx].y = sRstAlgo->rePoinSt[poIdx].y;

			vPoEd[poIdx].x = sRstAlgo->rePoinEd[poIdx].x;
			vPoEd[poIdx].y = sRstAlgo->rePoinEd[poIdx].y;
		}

		cv::Mat imgWingOutput = (*wingCandiImgSec).clone();
		cv::Mat imgWingOutput_Left = imgWingOutput.clone();
		cv::Mat imgWingOutput_Right = imgWingOutput.clone();

		//Foot Aß½E xAa ±aAØA¸·I ¿Þ, ¿A¸￥AE ¿μ¿ª ±¸ºÐ
		for (int y = 0; y < imgWingOutput.rows; y++)
		{
			UCHAR* ptrWingOutputImg = imgWingOutput.ptr<UCHAR>(y);
			UCHAR* ptrWingOutputImg_Left = imgWingOutput_Left.ptr<UCHAR>(y);
			UCHAR* ptrWingOutputImg_Right = imgWingOutput_Right.ptr<UCHAR>(y);

			for (int x = 0; x < imgWingOutput.cols; x++)
			{
				if (x > CenterPo.x - Margin.x)
				{
					ptrWingOutputImg_Left[x] = 0;
				}

				if (x < CenterPo.x - Margin.x)
				{
					ptrWingOutputImg_Right[x] = 0;
				}

			}
		}

		InspWingArea(pFoot, &imgWingOutput_Left, &imgWingOutput_Right, sRstAlgo);		//Wing ¸eAu A¤º¸ RstAlgoFoot ±¸A¶A¼¿¡ setting



#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_WingLength.bmp"), &(*wingCandiImgSec), vPoSt, vPoEd);
#endif
#if _DEBUG
		//μð¹o±e¿e AI¹IAo AuAa
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::Mat colorImg;
			(*wingCandiImgSec).copyTo(colorImg);

			cv::cvtColor(colorImg, colorImg, cv::COLOR_GRAY2RGB);
			for (int i = 0; i < vPoSt.size(); i++)
			{
				if ((int)m_eFoot_HeelCrack == i || (int)m_eFoot_Width_1 == i)
					continue;
				cv::line(colorImg, vPoSt[i], vPoEd[i], cv::Scalar(0, 0, 255));

			}

			cv::imwrite(rawname + std::string("_WingMeasurePoint.bmp"), colorImg);
		}
#endif
		cv::Point LSt, LEd;
		LSt.x = (int)sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Bonding1].x;
		LSt.y = (int)sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Bonding1].y;
		LEd.x = (int)sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Bonding1].x;
		LEd.y = (int)sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Bonding1].y;

		cv::Point RSt, REd;
		RSt.x = (int)sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Bonding2].x;
		RSt.y = (int)sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Bonding2].y;
		REd.x = (int)sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Bonding2].x;
		REd.y = (int)sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Bonding2].y;

		cv::Point pSt = LSt;		// WingAC ¿Þ/¿A¸￥ ±æAI Aß, ´o ±a AEAC PositionA≫ (pSt, pEd)¿¡ AuAa
		cv::Point pEd = LEd;

		if (LSt.y > RSt.y)
			pSt.y = RSt.y;
		if (LEd.y < REd.y)
			pEd.y = REd.y;

		if (pEd.y - pSt.y < 1)
		{
			pEd.y = Img.rows - 1;
		}

		//3. Foot Length inspection
		cv::Point LengRSt, LengREd;
		cv::Mat roImg;
		roImg = (cvFootImg).clone();
		//Img &= mMask;
		//WarpAffine(Img, roImg, seta1, Img.size(), CenterPo);		////Foot Img E¸Au

		bool bLength = Insp_FootLength(&roImg, pSt, pEd, &LengRSt, &LengREd);		//E¸AuμE Foot Img·IºIAI Foot ±æAI ±¸CI±a

		//AøA¤CN Foot ±æAI°¡ spec ¹þ¾i³ª´A Ao E®AI
		float delX = (pSt.x - pEd.x)*m_resolX;
		float delY = (pSt.y - pEd.y)*m_resolY;
		float fPow = std::powf(delX, 2) + std::powf(delY, 2);
		float fDstLength = std::sqrtf(fPow);
		//sRstAlgo->m_fArrRst[m_eFoot_Length_1] = fDst;

		if (pFoot->m_fArrOptionValue[m_eFoot_Length_1][(int)m_eMMD::eMMD_Max] * 0.8 < fDstLength
			|| pFoot->m_fArrOptionValue[m_eFoot_Length_1][(int)m_eMMD::eMMD_Min] > fDstLength)
		{
			//½ºÆa ¹þ¾i³? °æ¿i, wing ¿μ≫o¿¡¼­ Wire ºIºÐ A|°ACI´A CO¼o E￡Aa ¹× AøA¤ Æ÷AIÆ® Ac AuAa
			//1. Foot 3D AI¹IAo¿¡¼­ Ee·?³≫¸° ¿μ¿ª(40AICI) A|°A
			cv::Mat CutImg = SobelXAndY.clone();

			cv::Mat Foot2D_img = roImg.clone();
			cv::Mat foot3D_img = fHRoImg.clone();

			cv::Mat FootBWRmvFlowDown(foot3D_img.rows, foot3D_img.cols, CV_32FC1);

			//Ee·?³≫¸° ¿μ¿ª A|°A
			for (int y = 0; y < foot3D_img.rows; y++)
			{
				float* ptrInputImg = foot3D_img.ptr<float>(y);
				UCHAR* ptrFootBW = Foot2D_img.ptr(y);
				UCHAR* ptrFootBW_Rst = Foot2D_img.ptr(y);

				for (int x = 0; x < foot3D_img.cols; x++)
				{
					if (ptrFootBW[x] == 0)
					{
						ptrInputImg[x] = 0;
					}

					if (ptrInputImg[x] < 40)
					{
						ptrInputImg[x] = 0;
						ptrFootBW_Rst[x] = 0;
					}

				}
			}

			//Foot Blob 1°³ AI≫oAI¸e, Max Blob ¸¸ ≫Iμμ·I Aß°¡ (Wing ¿·A¸·I Ee·?³≫¸° ¿μ¿ªAI BlobA¸·I AaE÷´A °æ¿i°¡ AO¾i °E≫c¿¡ AoAa)
			double dArea, dCX, dCY;
			CRect rcBlob;
			AlgoBlob algoBlob;

			algoBlob.m_bUseIPC = false;
			algoBlob.m_bInvertCheck = false;
			algoBlob.m_bFilterIsUse = false;
			algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

			algoBlob.m_bInsp3D = false;
			algoBlob.m_nTypeRange3D = 2;
			algoBlob.m_dHeightRateMin = 0;
			algoBlob.m_dHeightRateMax = 0;

			algoBlob.m_bInsp2D = true;
			algoBlob.m_nTypeRange2D = 2;
			algoBlob.m_nMinBinary = 0;
			algoBlob.m_nMaxBinary = 100;
			algoBlob.m_bFillHole = true;

			TotalInspExceptArea stTieAreaNULL;
			stTieAreaNULL.m_nUsedInspPolygon = 0;
			stTieAreaNULL.m_nUsedMaskingValue = 0;
			stTieAreaNULL.m_nUsedWndPolygon = 0;

			cv::Mat BinImage(Foot2D_img.rows, Foot2D_img.cols, CV_8UC1);

			dCX = 0, dCY = 0, dArea = 0;
			rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
			int nCntBlob = CPInsp::BlobImageStruct(algoBlob, Foot2D_img.data, foot3D_img.ptr<float>(0), NULL, Foot2D_img.cols, Foot2D_img.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);

			Foot2D_img = BinImage.clone();

			for (int r = 0; r < foot3D_img.rows; r++)
			{

				float* fHightPtr = foot3D_img.ptr<float>(r);
				UCHAR* ptrFootBW = Foot2D_img.ptr(r);

				for (int c = 0; c < foot3D_img.cols; c++)
				{
					if (ptrFootBW[c] == 0)
					{
						fHightPtr[c] = 0;
					}
				}
			}

			int nDetectedWireStY = 0;

			bool bCalc_FootHeightProfile;

			bCalc_FootHeightProfile = DetectWireStartPos_UseFootHeightProfile(pFoot, fHRoImg, Foot2D_img, CutImg, CenterPo, Margin, &nDetectedWireStY);

			if (nDetectedWireStY != 0 && pEd.y > nDetectedWireStY && bCalc_FootHeightProfile)
			{
				for (int r = 0; r < (*wingCandiImgSec).rows; r++)
				{
					UCHAR* Ptr = (*wingCandiImgSec).data;
					for (int c = 0; c < (*wingCandiImgSec).cols; c++)
					{
						if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
						{
							if (r > nDetectedWireStY)
								Ptr[r * (*wingCandiImgSec).step + c] = 0;
						}
					}
				}

				LEd.y = nDetectedWireStY;
				REd.y = nDetectedWireStY;

				pSt = LSt;		// WingAC ¿Þ/¿A¸￥ ±æAI Aß, ´o ±a AEAC PositionA≫ (pSt, pEd)¿¡ AuAa
				pEd = LEd;

				vPoSt[(int)m_eFoot::m_eFoot_Bonding1] = LSt;
				vPoEd[(int)m_eFoot::m_eFoot_Bonding1] = LEd;
				vPoSt[(int)m_eFoot::m_eFoot_Bonding2] = RSt;
				vPoEd[(int)m_eFoot::m_eFoot_Bonding2] = REd;

				if (LSt.y > RSt.y)
					pSt.y = RSt.y;
				if (LEd.y < REd.y)
					pEd.y = REd.y;

				if (pEd.y - pSt.y < 1)
				{
					pEd.y = Img.rows - 1;
				}

				bLength = Insp_FootLength(&roImg, pSt, pEd, &LengRSt, &LengREd);

			}
		}

		//vPoSt[(int)m_eFoot::m_eFoot_Length_1] = LSt;
		//vPoEd[(int)m_eFoot::m_eFoot_Length_1] = LEd;
		vPoSt[(int)m_eFoot::m_eFoot_Length_1] = LengRSt;
		vPoEd[(int)m_eFoot::m_eFoot_Length_1] = LengREd;

		if (pEd.y >= roImg.rows)
			pEd.y = roImg.rows - 1;

		cv::Point WingBondingEd;
		WingBondingEd.x = pEd.x;
		WingBondingEd.y = pEd.y;
		cv::Point rotateWingEdPnt = pEd;

		cv::Point pLeftWingStX = LSt;		// ¾cAE WingAC XAa AuAa
		cv::Point pRightWingStX = RSt;

		if (LSt.x > LEd.x)
			pLeftWingStX = LEd;

		if (RSt.x < REd.x)
			pRightWingStX = REd;

#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_FootLength.bmp"), &roImg, vPoSt, vPoEd);
#endif

		//4. Wedge inspection
		Img = (cvWedgeImg).clone();

#if _DEBUG
		cv::imwrite(("D:\\FootRst\\Wedge_ALLblob.bmp"), Img);		//wedge max blob
#endif
		cv::Mat wedgeMaxBlob;
		cv::Mat PadRangeImg;
		//Insp_Wedge_WidthLength(pFoot, &Img, &(*wingCandiImgSec), fHRoImg, CenterPo, seta1, mMask, wedgeMaxBlob, PadRangeImg, pSt, pEd, vPoSt, vPoEd, &CentLineSt, &CentLineEd);
		Insp_Wedge_WidthLength(pFoot, sRstAlgo, &Img, &(*wingCandiImgSec), fHRoImg, CenterPo, seta1, mMask, wedgeMaxBlob, PadRangeImg, pSt, pEd, vPoSt, vPoEd, &CentLineSt, &CentLineEd, pLeftWingStX, pRightWingStX);

#if _DEBUG
		cv::imwrite(("D:\\FootRst\\Wedge_OutPut.bmp"), wedgeMaxBlob);		//wedge max blob
#endif

		//5. Foot width inspection
		//roImg = (*cvFootImg).clone();
		cv::Mat vLeft, vRight;

		// Change Foot Image(roImg) For inspect width
		if (pFoot->bUseRemovedPadArea)
		{
			cv::Mat cvOnlyPadBW = CalcOnlyPadArea(pFoot, Img.cols, Img.rows, (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC);
			cv::Mat cvRoOnlyPadBW;
			cv::Point Mar;
			Mar = WarpAffine_IPPRotate(cvOnlyPadBW, cvRoOnlyPadBW, seta1, cvOnlyPadBW.size(), CenterPo);
			cv::Mat kernelMorp = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
			cv::erode(cvRoOnlyPadBW, cvRoOnlyPadBW, kernelMorp);
			cv::erode(cvRoOnlyPadBW, cvRoOnlyPadBW, kernelMorp);
			cv::erode(cvRoOnlyPadBW, cvRoOnlyPadBW, kernelMorp);
			cv::erode(cvRoOnlyPadBW, cvRoOnlyPadBW, kernelMorp);
			roImg -= cvRoOnlyPadBW;
		}
		bool bUseFootWidthOnly2D = pFoot->bUse2DImageForInspWidth;	//Width inspection 시에는 Foot 부분만 취하도록 (true)

		if (bUseFootWidthOnly2D)
		{
			FootPoins fPoints;
			cv::Mat cvRofootRctBW(roImg.rows, roImg.cols, CV_8UC1);
			bool bIsFoundFoot = FindFoot_3DBlob(pFoot, sRstAlgo, &Img, &Img, &Img, &PadImg, fPoints, nUIFootBinIDX, cv3DAvgFilter, cvRofootRctBW, NULL, bUseFootWidthOnly2D, true, false);

			cv::Mat cvFootImgForInspWidth = fPoints.cvMatchingMask.clone();

			//Get Foot BW Image
			cv::Mat ImgfootBW(cvFootImgForInspWidth.rows, cvFootImgForInspWidth.cols, CV_8UC1);
			pFoot->GetImageMatrix(&ImgfootBW, (int)m_eFootBin::m_eFootBin_Foot);
			GetBinImage(pFoot, &ImgfootBW, m_eFootBin_Foot, NULL, false, 4, true);

			bool bIsDBCFoot = false;

			if (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
				bIsDBCFoot = true;

			if (bIsDBCFoot)
			{
				cv::Mat DBCBinImage(cvFootImgForInspWidth.rows, cvFootImgForInspWidth.cols, CV_8UC1);
				DBCBinImage.setTo(0);

				bool bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 80);

				///////////////////////
				if (bIsFindDBCOutLine)
					ImgfootBW = ImgfootBW - DBCBinImage;
			}

			//blob filtering///////////////////////
			cv::Mat ImgfootBWBlob(cvFootImgForInspWidth.rows, cvFootImgForInspWidth.cols, CV_8UC1);
			ImgfootBWBlob.setTo(0);
			cv::Mat kernelMorp = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
			cv::erode(ImgfootBW, ImgfootBW, kernelMorp);
			cv::dilate(ImgfootBW, ImgfootBW, kernelMorp);

			int nMinBlob = 100;
			cv::Mat BinImageTmp(cvFootImgForInspWidth.rows, cvFootImgForInspWidth.cols, CV_8UC1);
			int nCntFBlob = m_pProcMilAlgo->CalcBlob_Select(ImgfootBW.data, BinImageTmp.data, cvFootImgForInspWidth.cols, cvFootImgForInspWidth.rows, nMinBlob, FALSE, TRUE, 0, eSelectBigger);
			if (nCntFBlob > 0)
			{
				dCX = 0, dCY = 0, dArea = 0;
				m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
			}

			nMinBlob = dArea * 0.1;
			if (100 > nMinBlob) nMinBlob = 100;
			nCntFBlob = m_pProcMilAlgo->CalcBlob_Select(ImgfootBW.data, ImgfootBWBlob.data, cvFootImgForInspWidth.cols, cvFootImgForInspWidth.rows, nMinBlob, FALSE, TRUE, 0, eSelectMix);

			cv::Mat cvRoImgfootBW;
			cv::Point Margint;
			Margint = WarpAffine_IPPRotate(ImgfootBWBlob, cvRoImgfootBW, seta1, ImgfootBWBlob.size(), CenterPo);

			//Remove the area below the detected Foot Using 3D Blob
			int nWingRstRectMinX = imgWingOutput.cols;
			int nWingRstRectMaxX = 0;
			int nWingRstRectMinY = imgWingOutput.rows;
			int nWingRstRectMaxY = 0;
			for (int r = 0; r < imgWingOutput.rows; r++)
			{
				UCHAR* Ptr = imgWingOutput.data;
				for (int c = 0; c < imgWingOutput.cols; c++)
				{
					if (Ptr[r * imgWingOutput.step + c] == 255)
					{
						if (r < nWingRstRectMinY)	nWingRstRectMinY = r;
						if (r > nWingRstRectMaxY)	nWingRstRectMaxY = r;

						if (c < nWingRstRectMinX)	nWingRstRectMinX = c;
						if (c > nWingRstRectMaxX)	nWingRstRectMaxX = c;

					}
				}
			}
			if (nWingRstRectMaxY != 0)
			{
				for (int r = 0; r < cvRoImgfootBW.rows; r++)
				{
					UCHAR* Ptr = cvRoImgfootBW.data;
					for (int c = 0; c < cvRoImgfootBW.cols; c++)
					{
						if (Ptr[r * cvRoImgfootBW.step + c] == 255)
						{
							if (r > nWingRstRectMaxY)
								Ptr[r * cvRoImgfootBW.step + c] = 0;
						}
					}
				}
			}

			//Remove the area below the detected Foot Using 3D Blob
			cv::Mat Foot3DBlobRstImg = fPoints.cvMatchingMask.clone();

			//cv::Mat RoImgWingCandi;
			//WarpAffine_IPPRotate(Foot3DBlobRstImg, RoImgWingCandi, seta, Foot3DBlobRstImg.size(), CenterPo);	//Wing Bin Img E¸Au

			int nFoot3DBlobRectMinX = Foot3DBlobRstImg.cols;
			int nFoot3DBlobRectMaxX = 0;
			int nFoot3DBlobRectMinY = Foot3DBlobRstImg.rows;
			int nFoot3DBlobRectMaxY = 0;
			for (int r = 0; r < Foot3DBlobRstImg.rows; r++)
			{
				UCHAR* Ptr = Foot3DBlobRstImg.data;
				for (int c = 0; c < Foot3DBlobRstImg.cols; c++)
				{
					if (Ptr[r * Foot3DBlobRstImg.step + c] == 255)
					{
						if (r < nFoot3DBlobRectMinY)	nFoot3DBlobRectMinY = r;
						if (r > nFoot3DBlobRectMaxY)	nFoot3DBlobRectMaxY = r;

						if (c < nFoot3DBlobRectMinX)	nFoot3DBlobRectMinX = c;
						if (c > nFoot3DBlobRectMaxX)	nFoot3DBlobRectMaxX = c;

					}
				}
			}
			if (nFoot3DBlobRectMaxY != 0 && nFoot3DBlobRectMaxY /*- 30*/ > 0)
			{
				for (int y = 0; y < cvRoImgfootBW.rows; y++)
				{
					UCHAR* ptrFootRctRst = cvRoImgfootBW.ptr(y);

					for (int x = 0; x < cvRoImgfootBW.cols; x++)
					{
						if (y > nFoot3DBlobRectMaxY /*- 30*/)
						{
							ptrFootRctRst[x] = 0;
						}
					}
				}
			}

			cvFootImgForInspWidth = cvFootImgForInspWidth & cvRoImgfootBW;

			nMinBlob = 100;
			nCntFBlob = m_pProcMilAlgo->CalcBlob_Select(cvFootImgForInspWidth.data, cvFootImgForInspWidth.data, cvFootImgForInspWidth.cols, cvFootImgForInspWidth.rows, nMinBlob, FALSE, TRUE, 0, eSelectBigger);

#if _DEBUG
			if (cstDebugFolderPath.IsEmpty() != TRUE)
			{
				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);

				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);
				cv::imwrite(rawname + std::string("_CalcWidth_FootImg.bmp"), cvFootImgForInspWidth);
				cv::imwrite(rawname + std::string("_FootImg.bmp"), cvFootImg);
				cv::imwrite(rawname + std::string("_PadImg.bmp"), PadImg);
				cv::imwrite(rawname + std::string("_BondingAreaImg.bmp"), PadRangeImg);
				cv::imwrite(rawname + std::string("_WingImg.bmp"), imgWingOutput);
				cv::imwrite(rawname + std::string("_WedgeImg.bmp"), Img);
			}
#endif

			if (cstDebugFolderPath.IsEmpty() != TRUE)
			{
				CT2CA convertedString(cstDebugFolderPath);
				std::string strFootDebugImgPath = std::string(convertedString);

				size_t lastindex = strFootDebugImgPath.find_last_of(".");
				string rawname = strFootDebugImgPath.substr(0, lastindex);

				cv::imwrite(rawname + std::string("_CalcWidth_FootImg.bmp"), cvFootImgForInspWidth);
				cv::imwrite(rawname + std::string("_BondingAreaImg.bmp"), PadRangeImg);
				cv::imwrite(rawname + std::string("_ShiftPadImg.bmp"), PadImg);
			}

			vLeft = cvFootImgForInspWidth(cv::Rect(0, 0, CenterPo.x - Margin.x, roImg.rows)).clone();
			vRight = cvFootImgForInspWidth(cv::Rect(CenterPo.x - Margin.x, 0, roImg.cols - (CenterPo.x - Margin.x), roImg.rows)).clone();
		}
		else
		{
			vLeft = roImg(cv::Rect(0, 0, CenterPo.x - Margin.x, roImg.rows)).clone();
			vRight = roImg(cv::Rect(CenterPo.x - Margin.x, 0, roImg.cols - (CenterPo.x - Margin.x), roImg.rows)).clone();
		}

		cv::Point Re_LeftWidthSt(0, 0), Re_RightWidthSt(0, 0), Re_LeftWidthEd(0, 0), Re_RightWidthEd(0, 0);

		bool bWidthLeft = Insp_FootWidth(&vLeft, pSt, pEd, true, &Re_LeftWidthSt, &Re_LeftWidthEd);
		bool bWidthRight = Insp_FootWidth(&vRight, pSt, pEd, false, &Re_RightWidthSt, &Re_RightWidthEd);

		/*	Re_LeftWidthSt.y -= 2;
			Re_RightWidthSt.y -= 2;*/
		Re_LeftWidthSt.x = Re_LeftWidthSt.x;// -2;
		Re_RightWidthSt.x = Re_RightWidthSt.x + CenterPo.x - Margin.x;
		Re_LeftWidthEd.x = CenterPo.x - Margin.x;
		Re_LeftWidthEd.y = Re_LeftWidthSt.y;
		Re_RightWidthEd.x = CenterPo.x - Margin.x;
		Re_RightWidthEd.y = Re_RightWidthSt.y;
		vPoSt[(int)m_eFoot::m_eFoot_Width_1_H] = (Re_LeftWidthSt);
		vPoEd[(int)m_eFoot::m_eFoot_Width_1_H] = (Re_LeftWidthEd);

		vPoSt[(int)m_eFoot::m_eFoot_Width_1] = (Re_RightWidthSt);
		vPoEd[(int)m_eFoot::m_eFoot_Width_1] = (Re_RightWidthEd);
#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_Width.bmp"), &roImg, vPoSt, vPoEd);
#endif

		//6. tail Length inspection
		roImg = (cvFootImg).clone();
		cv::Point pTailSt, pTailEd;
		float fArea(0.0f);
		bool bTailLength = Insp_FootTailLength(roImg, pSt, pSt, &pTailSt, &pTailEd, &fArea); // Tail End bonding ±aAØA¸·I º?°æ

		vPoSt[(int)m_eFoot::m_eFoot_Tail_L] = (pTailSt);
		vPoEd[(int)m_eFoot::m_eFoot_Tail_L] = (pTailEd);
#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_W_Tail.bmp"), &roImg, vPoSt, vPoEd);
#endif
		nLine = __LINE__;

		/// ¹þ¾i³²°E≫c ¹× NG½A CN¹ø ´o °E≫c //////////////////////////////////////
		cv::Point cvPoNewCenter;
		cvPoNewCenter.x = CenterPo.x - Margin.x;
		cvPoNewCenter.y = CenterPo.y - Margin.y;

		//for teaching
		POINTF CenterPoint;
		CenterPoint.x = CenterPo.x;
		CenterPoint.y = CenterPo.y;
		sRstAlgo->m_sPoint = CenterPoint;

		cv::Mat ImgFoot = (cvFootImg).clone();
		float fshiftX = 0, fshiftY = 0, fshift = 0;
		bool shiftRst = Insp_FootShift_ImgAnd(pFoot, sRstAlgo, &ImgFoot, &PadImg, &PadRangeImg, cvPoNewCenter, fshift, fshiftX, fshiftY);

		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationX, fshiftX);
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationY, fshiftY);
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_Deviation, fshift);

		if (sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] == false ||
			sRstAlgo->m_bArrOK[(int)m_eFoot_DeviationX] == false ||
			sRstAlgo->m_bArrOK[(int)m_eFoot_DeviationY] == false)
		{
			{
				//½ºÆa ¹þ¾i³? °æ¿i¿¡´A A®¿¡¼­ Wire A|°ACI´A CO¼o E￡Aa ¹× Æ÷AIÆ® Ac AuAa
				//*wingCandiImgSec
				//Wing °a°u¿¡¼­ Wire ºIºÐ A|°ACI±a A§CN AU¾÷
			//1. Foot 3D AI¹IAo¿¡¼­ Ee·?³≫¸° ¿μ¿ª(40AICI) A|°A
				cv::Mat CutImg = SobelXAndY.clone();

				cv::Mat Foot2D_img = roImg.clone();
				cv::Mat foot3D_img = fHRoImg.clone();

				cv::Mat FootBWRmvFlowDown(foot3D_img.rows, foot3D_img.cols, CV_32FC1);

				//Ee·?³≫¸° ¿μ¿ª A|°A
				for (int y = 0; y < foot3D_img.rows; y++)
				{
					float* ptrInputImg = foot3D_img.ptr<float>(y);
					UCHAR* ptrFootBW = Foot2D_img.ptr(y);
					UCHAR* ptrFootBW_Rst = Foot2D_img.ptr(y);

					for (int x = 0; x < foot3D_img.cols; x++)
					{
						if (ptrFootBW[x] == 0)
						{
							ptrInputImg[x] = 0;
						}

						if (ptrInputImg[x] < 40)
						{
							ptrInputImg[x] = 0;
							ptrFootBW_Rst[x] = 0;
						}

					}
				}

				//Foot Blob 1°³ AI≫oAI¸e, Max Blob ¸¸ ≫Iμμ·I Aß°¡ (Wing ¿·A¸·I Ee·?³≫¸° ¿μ¿ªAI BlobA¸·I AaE÷´A °æ¿i°¡ AO¾i °E≫c¿¡ AoAa)
				double dArea, dCX, dCY;
				CRect rcBlob;
				AlgoBlob algoBlob;

				algoBlob.m_bUseIPC = false;
				algoBlob.m_bInvertCheck = false;
				algoBlob.m_bFilterIsUse = false;
				algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

				algoBlob.m_bInsp3D = false;
				algoBlob.m_nTypeRange3D = 2;
				algoBlob.m_dHeightRateMin = 0;
				algoBlob.m_dHeightRateMax = 0;

				algoBlob.m_bInsp2D = true;
				algoBlob.m_nTypeRange2D = 2;
				algoBlob.m_nMinBinary = 0;
				algoBlob.m_nMaxBinary = 100;
				algoBlob.m_bFillHole = true;

				TotalInspExceptArea stTieAreaNULL;
				stTieAreaNULL.m_nUsedInspPolygon = 0;
				stTieAreaNULL.m_nUsedMaskingValue = 0;
				stTieAreaNULL.m_nUsedWndPolygon = 0;

				cv::Mat BinImage(Foot2D_img.rows, Foot2D_img.cols, CV_8UC1);

				dCX = 0, dCY = 0, dArea = 0;
				rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
				int nCntBlob = CPInsp::BlobImageStruct(algoBlob, Foot2D_img.data, foot3D_img.ptr<float>(0), NULL, Foot2D_img.cols, Foot2D_img.rows, 4, &dArea, &dCX, &dCY, &rcBlob, BinImage.data, stTieAreaNULL, true);

				Foot2D_img = BinImage.clone();

				for (int r = 0; r < foot3D_img.rows; r++)
				{

					float* fHightPtr = foot3D_img.ptr<float>(r);
					UCHAR* ptrFootBW = Foot2D_img.ptr(r);

					for (int c = 0; c < foot3D_img.cols; c++)
					{
						if (ptrFootBW[c] == 0)
						{
							fHightPtr[c] = 0;
						}
					}
				}

				int nDetectedWireStY = 0;

				bool bCalc_FootHeightProfile;

				bCalc_FootHeightProfile = DetectWireStartPos_UseFootHeightProfile(pFoot, fHRoImg, Foot2D_img, CutImg, CenterPo, Margin, &nDetectedWireStY);

				if (nDetectedWireStY != 0 && pEd.y > nDetectedWireStY && bCalc_FootHeightProfile)
				{
					for (int r = 0; r < (*wingCandiImgSec).rows; r++)
					{
						UCHAR* Ptr = (*wingCandiImgSec).data;
						for (int c = 0; c < (*wingCandiImgSec).cols; c++)
						{
							if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
							{
								if (r > nDetectedWireStY)
									Ptr[r * (*wingCandiImgSec).step + c] = 0;
							}
						}
					}

					LEd.y = nDetectedWireStY;
					REd.y = nDetectedWireStY;

					pSt = LSt;		// WingAC ¿Þ/¿A¸￥ ±æAI Aß, ´o ±a AEAC PositionA≫ (pSt, pEd)¿¡ AuAa
					pEd = LEd;

					vPoSt[(int)m_eFoot::m_eFoot_Bonding1] = LSt;
					vPoEd[(int)m_eFoot::m_eFoot_Bonding1] = LEd;
					vPoSt[(int)m_eFoot::m_eFoot_Bonding2] = RSt;
					vPoEd[(int)m_eFoot::m_eFoot_Bonding2] = REd;

					if (LSt.y > RSt.y)
						pSt.y = RSt.y;
					if (LEd.y < REd.y)
						pEd.y = REd.y;

					if (pEd.y - pSt.y < 1)
					{
						pEd.y = Img.rows - 1;
					}

					if (bCalc_FootHeightProfile)
					{
						for (int r = 0; r < PadRangeImg.rows; r++)
						{
							UCHAR* Ptr = PadRangeImg.data;
							for (int c = 0; c < PadRangeImg.cols; c++)
							{
								if (Ptr[r * PadRangeImg.step + c] == 255)
								{
									if (r > nDetectedWireStY)
										Ptr[r * CutImg.step + c] = 0;
								}
							}
						}
					}

					CenterPoint.x = CenterPo.x;
					CenterPoint.y = CenterPo.y;
					sRstAlgo->m_sPoint = CenterPoint;

					fshiftX = 0, fshiftY = 0, fshift = 0;
					shiftRst = Insp_FootShift_ImgAnd(pFoot, sRstAlgo, &ImgFoot, &PadImg, &PadRangeImg, cvPoNewCenter, fshift, fshiftX, fshiftY);

					Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationX, fshiftX);
					Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_DeviationY, fshiftY);
					Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, (int)m_eFoot::m_eFoot_Deviation, fshift);

				}

			}
		}

		//////////////////////////////////////////////////////////////////////

		//UI¿¡ C￥½ACO Wedge AI¹IAo
		cv::Mat WedgeOrg = cv::Mat(cvFootImg.rows, cvFootImg.cols, CV_8UC1);
		WedgeOrg.setTo(0);
		bool bRoWedgeImgBackToOrgPos = RotateBackToOriginalPosition(pFoot, fHRoImg, roImg, wedgeMaxBlob, CenterPo, Margin, seta, &WedgeOrg);

		nLine = __LINE__;

		//UI ¿¡ C￥½ACO Wing AI¹IAo 
		cv::Mat WingOrg = cv::Mat(cvFootImg.rows, cvFootImg.cols, CV_8UC1);
		WingOrg.setTo(0);
		bool bRoWingImgBackToOrgPos = RotateBackToOriginalPosition(pFoot, fHRoImg, roImg, (*wingCandiImgSec), CenterPo, Margin, seta, &WingOrg);

		nLine = __LINE__;

#pragma region Write UI_Image
		nLine = __LINE__;
		//
		//UI¿¡ C￥½ACI±a A§CN Pointº?E?
		if (ucArrDstImg != NULL)
		{
			std::vector<cv::Point> vPoDispSt;
			std::vector<cv::Point> vPoDispEd;
			CalcDisplayPnt(seta1, vPoSt, vPoEd, vPoDispSt, vPoDispEd);
			CalcRotPnt(CenterPo, -seta1, vPoDispSt, vPoDispEd, Margin);

			for (int i = 0; i < m_eFoot::m_eFoot_Total; i++)
			{
				sRstAlgo->DispPoinSt[i].x = vPoDispSt[i].x;
				sRstAlgo->DispPoinSt[i].y = vPoDispSt[i].y;

				sRstAlgo->DispPoinEd[i].x = vPoDispEd[i].x;
				sRstAlgo->DispPoinEd[i].y = vPoDispEd[i].y;
			}
#if _DEBUG
			ImagePointSave(_T("D:\\FootRst\\Bin2D_End_Disp.bmp"), &roImg, vPoDispSt, vPoDispEd);
#endif
		}
		nLine = __LINE__;

#pragma endregion

		/*
		//Imprint
		Img = (*cvFootImg)(cv::Rect(*left, 0, *right - (*left), cvFootImg->rows));
		Img &= mMask;
		WarpAffine(Img, roImg, seta1, Img.size(), CenterPo);

		cv::Point pImprintSt, pImprintEd;
		bool bImprint = Insp_FootImprint2(&CutImg, sInspFoot->m_fArrOptionValue[(int)m_eFOOT_Data::m_eFOOT_Data_Damage][m_eMMD::eMMD_Min], pSt, pEd, &pImprintSt, &pImprintEd);
		if (bImprint)
		{
			vPoSt[(int)m_eFoot::m_eFoot_Damage] = (pImprintSt);
			vPoEd[(int)m_eFoot::m_eFoot_Damage] = (pImprintEd);
		}
	#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_W_T_Imprint.bmp"), &roImg, vPoSt, vPoEd);
	#endif

		//HillCrack
		cv::Mat Img3D = (*cv3DImg)(cv::Rect(*left, 0, *right - (*left), cvFootImg->rows));
		Img = (*cvFootImg)(cv::Rect(*left, 0, *right - (*left), cvFootImg->rows));
		WarpAffine(Img, roImg, seta1, Img.size(), CenterPo);
		fHRectImg = Img3D;

		if ((*right) - (*left) < Img3D.cols)
		{
			fHRectImg = Img3D(cv::Rect(*left, 0, (*right) - (*left), Img3D.rows)).clone();
		}
		cv::Mat ro3D;
		WarpAffine(fHRectImg, ro3D, seta1, fHRectImg.size(), CenterPo);
		cv::Point pHillCrackSt, pHillCrackEd;
		SetInspImageData(&roImg, *left, (int)m_eFootBin::m_eFootBin_Foot, sRstFootImage);

		bool bHillCrack = Insp_FootHillCrack(&roImg, &ro3D, pEd, &pHillCrackSt, &pHillCrackEd);

		if (bHillCrack && pHillCrackSt.x > 0 && pHillCrackSt.y > 0 && pHillCrackEd.x > 0 && pHillCrackEd.y > 0)
		{
			vPoSt[(int)m_eFoot::m_eFoot_HillCrack] = (pHillCrackSt);
			vPoEd[(int)m_eFoot::m_eFoot_HillCrack] = (pHillCrackEd);
		}
	#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_W_T_I_HillCrack.bmp"), &roImg, vPoSt, vPoEd);
	#endif

		*/
		nLine = __LINE__;
		//Lift
		roImg = (cvFootImg).clone();
		cv::Mat ro3D = (cv3DImg).clone();
		cv::Mat  padRectImg, padImage = (PadImg).clone();
		cv::Mat LiftWRect, RoLiftWRect;
		cv::Mat kernel;

		pFoot->GetPadBinImage(&padImage);


		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));
		cv::erode(padImage, padImage, kernel);

		Img = cvFootImg.clone();
		RoLiftWRect = wedgeMaxBlob.clone();

		padRectImg = padImage.clone();
		ro3D = (cv3DImg).clone();

		RoLiftWRect &= PadRangeImg;
#if _DEBUG
		cv::imwrite("D:\\FootRst\\LIft_RoLiftWRect.bmp", RoLiftWRect);
		cv::imwrite("D:\\FootInspImg\\PadRangeImg.bmp", PadRangeImg);
#endif
		nLine = __LINE__;
		cv::Mat InspImg, InspFindImg, InspRoWImg, Insp3D, InspPad;
		int nTop = vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y;	//pWedgeH_1.y;	
		nTop = vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y < vPoEd[(int)m_eFoot::m_eFoot_Wedge_L].y ? vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y : vPoEd[(int)m_eFoot::m_eFoot_Wedge_L].y;
		int nBot = LEd.y;
		nBot = REd.y > LEd.y ? REd.y : LEd.y;

		InspImg = RoLiftWRect.clone();
		//
		//Foot ³oAI °EAa ½A, Wedge ¿μ¿ªAC Aß¾O 20% ¿μ¿ª¿¡¼­¸¸ °EAaAa·I º?°æ
		//rect ±¸CI±a
		int ncvImgWidth = RoLiftWRect.cols;
		int ncvImgHeight = RoLiftWRect.rows;
		int nWedgeTmpMin_x = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_x = 0;
		int nWedgeTmpMin_y = ncvImgWidth * ncvImgHeight, nWedgeTmpMax_y = 0;
		for (int r = 0; r < RoLiftWRect.rows; r++)
		{
			UCHAR* Ptr = RoLiftWRect.data;
			for (int c = 0; c < RoLiftWRect.cols; c++)
			{
				if (Ptr[r*RoLiftWRect.step + c] == 255)
				{
					if (c < nWedgeTmpMin_x) nWedgeTmpMin_x = c;
					if (c > nWedgeTmpMax_x) nWedgeTmpMax_x = c;
					if (r < nWedgeTmpMin_y) nWedgeTmpMin_y = r;
					if (r > nWedgeTmpMax_y) nWedgeTmpMax_y = r;
				}
			}
		}

		//wedge °a°u ¾øA≫ ½A CO¼o ¸®AI
		if (nWedgeTmpMin_x == ncvImgWidth * ncvImgHeight && nWedgeTmpMin_y == ncvImgWidth * ncvImgHeight &&
			nWedgeTmpMax_x == 0 && nWedgeTmpMax_y == 0)
		{
			CString cstrInspLog;
			cstrInspLog.Format(_T("[FootAlgo]::FootMeasure()_Not Exist Wedge Blob"));
			g_pMPTI->AddLog(cstrInspLog);

			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);
				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;
			}
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("[FootAlgo]::Nothing in the Wedge Image (RoLiftWRect)"));
			g_pMPTI->AddLog_OCR(sLog);
#endif

			return CenterPo;
		}

		int wedgeRectWidth = nWedgeTmpMax_x - nWedgeTmpMin_x;
		int wedgeRectHeight = nWedgeTmpMax_y - nWedgeTmpMin_y;

		//WedgeRect Aß½EA¡
		int wedgeRectCenterX = nWedgeTmpMin_x + (int)round((float)wedgeRectWidth / 2.0f);
		int wedgeRectCenterY = nWedgeTmpMin_y + (int)round((float)wedgeRectHeight / 2.0f);

		int nFindCtLineX = 0;
		int nStX = 0;
		for (int y = 0; y < RoLiftWRect.rows; y++)
		{
			UCHAR* ptr = RoLiftWRect.ptr<UCHAR>(y);
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

		wedgeRectCenterX = nStX + (int)round((float)nFindCtLineX / 2.0f);

		//WedgeRectAC 20% AoA¡¸¸ AßAa
		float fExtractSomeArea_stRatio = 0.3;
		float fExtractSomeArea_EdRatio = 0.7;

		RECT rtWedgeRect20;
		rtWedgeRect20.left = nWedgeTmpMin_x;
		rtWedgeRect20.right = nWedgeTmpMax_x;
		rtWedgeRect20.top = nWedgeTmpMin_y + wedgeRectHeight * fExtractSomeArea_stRatio;
		rtWedgeRect20.bottom = nWedgeTmpMin_y + wedgeRectHeight * fExtractSomeArea_EdRatio;

		if (nWedgeTmpMin_x == nWedgeTmpMax_x || (int)(wedgeRectHeight * fExtractSomeArea_stRatio) == (int)(wedgeRectHeight * fExtractSomeArea_EdRatio))
		{
			if (nWedgeTmpMin_x == nWedgeTmpMax_x)
				rtWedgeRect20.right = nWedgeTmpMin_x + 1;
			if ((int)(wedgeRectHeight * fExtractSomeArea_stRatio) == (int)(wedgeRectHeight * fExtractSomeArea_EdRatio))
			{
				rtWedgeRect20.top = nWedgeTmpMin_y + (wedgeRectHeight * fExtractSomeArea_stRatio);
				rtWedgeRect20.bottom = nWedgeTmpMin_y + wedgeRectHeight * fExtractSomeArea_EdRatio + 1;
			}
		}

		cv::Mat Wedge20percentRect(RoLiftWRect.rows, RoLiftWRect.cols, CV_8UC1);
		Wedge20percentRect.setTo(0);

		for (int y = 0; y < RoLiftWRect.rows; y++)
		{
			UCHAR* ptr = RoLiftWRect.ptr<UCHAR>(y);
			UCHAR* ptrRstWedge20 = Wedge20percentRect.ptr<UCHAR>(y);

			for (int x = nWedgeTmpMin_x; x < nWedgeTmpMax_x; x++)
			{
				if (y > rtWedgeRect20.top && y < rtWedgeRect20.bottom)
				{
					if (ptr[x] == 255)
					{
						ptrRstWedge20[x] = 255;
					}
				}
			}

		}

		//Wedge ¿μ¿ª 20% AßAaμC¾u´AAo E®AI
		double dAreaTmp, dCXTmp, dCYTmp;
		CRect rcBlobTmp;
		AlgoBlob algoBlobTmp;

		algoBlobTmp.m_bUseIPC = false;	algoBlobTmp.m_bInvertCheck = false;	algoBlobTmp.m_bFilterIsUse = false;
		algoBlobTmp.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

		algoBlobTmp.m_bInsp3D = false;
		algoBlobTmp.m_nTypeRange3D = 2;	algoBlobTmp.m_dHeightRateMin = 0;	algoBlobTmp.m_dHeightRateMax = 0;

		algoBlobTmp.m_bInsp2D = true;
		algoBlobTmp.m_nTypeRange2D = 2;	algoBlobTmp.m_nMinBinary = 0;	algoBlobTmp.m_nMaxBinary = 100;
		algoBlobTmp.m_bFillHole = true;

		TotalInspExceptArea stTieAreaNULLTmp;
		stTieAreaNULLTmp.m_nUsedInspPolygon = 0;
		stTieAreaNULLTmp.m_nUsedMaskingValue = 0;
		stTieAreaNULLTmp.m_nUsedWndPolygon = 0;

		cv::Mat cvWedgeBinImage(Wedge20percentRect.rows, Wedge20percentRect.cols, CV_8UC1);

		dCXTmp = 0, dCYTmp = 0, dAreaTmp = 0;
		rcBlobTmp.left = 0; rcBlobTmp.right = 0; rcBlobTmp.top = 0; rcBlobTmp.bottom = 0;
		int nWedgeCntBlob = CPInsp::BlobImageStruct(algoBlob, Wedge20percentRect.data, fHRoImg.ptr<float>(0), NULL, Wedge20percentRect.cols, Wedge20percentRect.rows, 4, &dAreaTmp, &dCXTmp, &dCYTmp, &rcBlobTmp, cvWedgeBinImage.data, stTieAreaNULLTmp, true);

		if (nWedgeCntBlob == 0)
		{
			Wedge20percentRect = RoLiftWRect.clone();

			cv::Mat cvRealWedgeBinImage(RoLiftWRect.rows, RoLiftWRect.cols, CV_8UC1);
			cvRealWedgeBinImage.setTo(0);
			int nBlobCount = CPInsp::BlobImageStruct(algoBlob, RoLiftWRect.data, fHRoImg.ptr<float>(0), NULL, RoLiftWRect.cols, RoLiftWRect.rows, 10, &dAreaTmp, &dCXTmp, &dCYTmp, &rcBlobTmp, cvRealWedgeBinImage.data, stTieAreaNULLTmp, true);

			if (nBlobCount >= 2)
			{
				//¿μ¿ªAC Æo±O 3D ³oAI°¡ ´o AUAº BlobA≫ Wedge·I ¼±AA
				std::vector<std::vector<cv::Point>> contoursWedge;
				std::vector<cv::Vec4i> hierarchy;
				cv::findContours(RoLiftWRect, contoursWedge, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
				std::vector<std::vector<cv::Point> >hull(contoursWedge.size());
				for (size_t i = 0; i < contoursWedge.size(); i++)
				{
					convexHull(cv::Mat(contoursWedge[i]), hull[i], false);
				}

				int idx = 0;
				cv::Mat WedgeRst_Blob(RoLiftWRect.size(), CV_8UC1, cv::Scalar(0));
				std::vector<int> bolbCandidate;
				for (contoursWedge.begin(); idx < contoursWedge.size(); idx++)
				{
					drawContours(WedgeRst_Blob, contoursWedge, idx, cv::Scalar(255), cv::FILLED, 8, hierarchy);
				}

				//Blob ¸eAuAI 100AI≫oAI¸e¼­ Foot Aß½E°u °¡Aa °¡±i¿i °A¸®¿¡ A§A¡CN Blob ¸¸ AßAa
				cv::Mat detectedWedgeRst(WedgeRst_Blob.size(), CV_8UC1, cv::Scalar(0));
				int nIdx = 0;
				int nSelectIdx = 0;
				float fminHeight = 100000;

				cv::Mat Foot3DImg = fHRoImg.clone();
				size_t floatstep_Foot3DCorrect = Foot3DImg.step / sizeof(float);

				for (contoursWedge.begin(); nIdx < contoursWedge.size(); nIdx++)
				{
					cv::Rect bbox = cv::boundingRect(contoursWedge[nSelectIdx]);
					double area = cv::contourArea(contoursWedge[nSelectIdx]);
					float fBlobAreaAvgHeight = 0;
					int nBlobWhitePxCnt = 0;

					if (area > 80)
					{
						for (int r = bbox.y; r < bbox.y + bbox.height; r++)
						{
							UCHAR* Ptr = WedgeRst_Blob.data;
							float* Ptr_Foot3D = (float*)Foot3DImg.data;

							for (int c = bbox.x; c < bbox.x + bbox.width; c++)
							{
								if (Ptr[r * WedgeRst_Blob.step + c] == 255)
								{
									fBlobAreaAvgHeight = fBlobAreaAvgHeight + Ptr_Foot3D[r * floatstep_Foot3DCorrect + c];
									nBlobWhitePxCnt++;

									// 									if ((CenterPo.x == c) && CenterPo.y == r)
									// 									{
									// 										nIncludeFootCtrIndex = maxIdx;
									// 										nIncludeFootCenterPoint = true;
									// 										break;
									// 									}

								}
							}
						}

						if (fBlobAreaAvgHeight > 0 && fBlobAreaAvgHeight != 0 && nBlobWhitePxCnt != 0)
							fBlobAreaAvgHeight = (fBlobAreaAvgHeight / (float)nBlobWhitePxCnt);

					}

					if (fBlobAreaAvgHeight > 0 && fBlobAreaAvgHeight != 0)
					{
						if (fminHeight > fBlobAreaAvgHeight)
						{
							nSelectIdx = nIdx;
							fminHeight = fBlobAreaAvgHeight;
						}
					}

				}

				drawContours(detectedWedgeRst, contoursWedge, nSelectIdx, cv::Scalar(255), cv::FILLED, 8, hierarchy);

				Wedge20percentRect = detectedWedgeRst;
			}

			CString cstrInspWedgeBlob;
			cstrInspWedgeBlob.Format(_T("[FootAlgo]::FootMeasure() Wedge Blob Select by 3D Height"));
			g_pMPTI->AddLog(cstrInspWedgeBlob);

		}

		InspImg = Wedge20percentRect.clone();

		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_InspWedgeImg.bmp"), Wedge20percentRect);
		}

		//////////////////////

		Insp3D = ro3D.clone();
		nTop = 0;
		if (nBot - nTop < roImg.rows)
		{
			InspRoWImg = Wedge20percentRect(cv::Rect(0, nTop, Wedge20percentRect.cols, nBot - nTop)).clone();
			InspFindImg = roImg(cv::Rect(0, nTop, roImg.cols, nBot - nTop)).clone();

			InspImg = InspRoWImg.clone();
			Insp3D = ro3D(cv::Rect(0, nTop, ro3D.cols, nBot - nTop)).clone();
		}
		sRstAlgo->m_rcRefArea.left = 0;
		sRstAlgo->m_rcRefArea.right = 0;
		sRstAlgo->m_rcRefArea.top = 0;
		sRstAlgo->m_rcRefArea.bottom = 0;
		float fHightDiff(0.0f);

		if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_RefArea) == m_eFOOT_Data2::m_eFOOT_Data2_RefArea)
			Insp_FootLift_v2(&InspImg, &Insp3D, 0, &fHightDiff);
		else
		{
			cv::Mat PadRotImg;
			//WarpAffine(padRectImg, PadRotImg, seta1, padRectImg.size(), CenterPo);
			WarpAffine_IPPRotate(padRectImg, PadRotImg, seta1, padRectImg.size(), CenterPo);		//Wing Bin Img E¸Au

			int nTop = vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y;
			nTop = vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y < vPoEd[(int)m_eFoot::m_eFoot_Wedge_L].y ? vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y : vPoEd[(int)m_eFoot::m_eFoot_Wedge_L].y;
			int nBot = LEd.y;
			nBot = REd.y > LEd.y ? REd.y : LEd.y;

			InspPad = (PadRotImg).clone();
			nTop = 0;
			if (nBot - nTop < PadRotImg.rows)
			{
				InspPad = (PadRotImg)(cv::Rect(0, nTop, PadRotImg.cols, nBot - nTop)).clone();
				InspPad -= InspFindImg;
			}

#if _DEBUG
			cv::Mat cvTmpFoot3D;
			cvTmpFoot3D = Insp3D.clone();
			cvTmpFoot3D.convertTo(cvTmpFoot3D, CV_8UC1);
#endif

			Insp_FootLift(&InspImg, &Insp3D, &InspPad, &fHightDiff);
#if _DEBUG
			cv::imwrite("D:\\FootRst\\LIft_padRectImg.bmp", PadRotImg);
			cv::imwrite("D:\\FootRst\\LIft_InspPadImg.bmp", InspPad);
#endif
		}

#if _DEBUG
		//μð¹o±e¿e AI¹IAo AuAa
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_WedgeRstImage.bmp"), wedgeMaxBlob);
			cv::imwrite(rawname + std::string("_Wedge20PerImg.bmp"), Wedge20percentRect);
		}
#endif


		if (fHightDiff == 0)
		{

			int nCx = (vPoSt[(int)m_eFoot::m_eFoot_Bonding1].x + vPoSt[(int)m_eFoot::m_eFoot_Bonding2].x) / 2;
			int nCy = (vPoSt[(int)m_eFoot::m_eFoot_Bonding1].y + vPoEd[(int)m_eFoot::m_eFoot_Bonding1].y) / 2;

			int nMinY = vPoSt[(int)m_eFoot::m_eFoot_Bonding1].y;
			int nMaxY = vPoEd[(int)m_eFoot::m_eFoot_Bonding1].y;

			if (vPoSt[(int)m_eFoot::m_eFoot_Bonding1].y > vPoSt[(int)m_eFoot::m_eFoot_Bonding2].y)
			{
				nMinY = vPoSt[(int)m_eFoot::m_eFoot_Bonding2].y;
			}

			if (vPoEd[(int)m_eFoot::m_eFoot_Bonding1].y < vPoEd[(int)m_eFoot::m_eFoot_Bonding2].y)
			{
				nMaxY = vPoEd[(int)m_eFoot::m_eFoot_Bonding2].y;
			}

			nCy = (nMinY + nMaxY) / 2;

			cv::Mat Img3DRo(ro3D.rows, ro3D.cols, CV_8UC1);
			for (int r = 0; r < Insp3D.rows; r++)
			{
				UCHAR* ImgPtr = Img3DRo.ptr(r);
				float* Img3DPtr = Insp3D.ptr<float>(r);
				for (int c = 0; c < Insp3D.cols; c++)
				{
					//if (CenterPo.x == c && CenterPo.y == r)
					if (nCx == c && nCy == r)
						fHightDiff = (Img3DPtr[c]);
				}
			}
		}

		CString cstrInspFootHeightLog;
		cstrInspFootHeightLog.Format(_T("[FootAlgo]::Insp_FootHeight : %.2f"), fHightDiff);
		g_pMPTI->AddLog(cstrInspFootHeightLog);


#if _DEBUG
		cv::Mat Img3DRo(ro3D.rows, ro3D.cols, CV_8UC1);
		for (int r = 0; r < Insp3D.rows; r++)
		{
			UCHAR* ImgPtr = Img3DRo.ptr(r);
			float* Img3DPtr = Insp3D.ptr<float>(r);
			for (int c = 0; c < Insp3D.cols; c++)
			{
				ImgPtr[c] = (UCHAR)(Img3DPtr[c] / 10);
			}
		}
		//CString msg;
		//msg.Format(_T("D:\\FootRst\\LIft_InspImg.bmp"));
		//cv::imwrite(std::string(CT2A(msg)), InspImg);
		//msg.Format(_T("D:\\FootRst\\LIft_InspRoWImg.bmp"));
		//cv::imwrite(std::string(CT2A(msg)), InspRoWImg);
		//msg.Format(_T("D:\\FootRst\\LIft_InspFindImg.bmp"));
		//cv::imwrite(std::string(CT2A(msg)), InspFindImg);
#endif
#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_W_T_I_H_Lift.bmp"), &roImg, vPoSt, vPoEd);
#endif

		CalcRotPnt(CenterPo, -seta1, vPoSt, vPoEd, Margin);

#if _DEBUG
		cv::Mat rotateFootImg;
		cv::Point cvPoNew;
		cvPoNew.x = CenterPo.x - Margin.x;
		cvPoNew.y = CenterPo.y - Margin.y;

		cv::Point margin_New = WarpAffine(Img, rotateFootImg, -seta, Img.size(), cvPoNew);	//Wing 3D Img E¸Au

		ImagePointSave(_T("D:\\FootRst\\Bin2D_End.bmp"), &rotateFootImg, vPoSt, vPoEd);
#endif
		nLine = __LINE__;

		// 		//for teaching
		// 		POINTF CenterPoint;
		// 		CenterPoint.x = CenterPo.x;
		// 		CenterPoint.y = CenterPo.y;
		// 		sRstAlgo->m_sPoint = CenterPoint;

				//result ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				//PadLength
				//result /Left
		int footInspTypeIdx = (int)m_eFoot::m_eFoot_Bonding1;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		//result /Right
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Bonding2;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);

		//Length
		//result /Left
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Length_1;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);


		//wedge
		//result / length
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Wedge_L;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		//wedge result / width
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Wedge_W;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);

		//width
		//result / Left / Harf
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_1_H;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		float widthLeft = sRstAlgo->m_fArrRst[footInspTypeIdx];

		//result / Right / Harf
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_1;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		float widthright = sRstAlgo->m_fArrRst[footInspTypeIdx];
		float widthValue = widthLeft + widthright;
		//result /width
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_1;
		vPoEd[footInspTypeIdx] = vPoSt[(int)m_eFoot::m_eFoot_Width_1_H];
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, widthValue);
		//result / Right
		//footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_2;
		//if (vPoSt[footInspTypeIdx].x != -1 || vPoSt[footInspTypeIdx].y != -1)
		//	vPoSt[footInspTypeIdx].x += (*left);
		//if (vPoSt[footInspTypeIdx].x != -1 || vPoSt[footInspTypeIdx].y != -1)
		//	vPoEd[footInspTypeIdx].x += (*left);
		//Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, widthValue);


// 		if (!bWingRstVisible && g_pMPTI->m_bUseAIModule && ((g_pMPTI->m_nAIOption & m_eAIOPT_Foot_AutoInsp) == m_eAIOPT_Foot_AutoInsp))
// 		{
// 			//Width NG 
// 			//DWORD start = GetTickCount();
// 			//float fTacTime = 0;
// 			//CString sLog = _T("");
// 
// 			/////////////////////////////////////////////
// 			AUTO_INSP_AI_DATA stTmpbuf;
// 			stTmpbuf.rcWndROI.left = 0;
// 			stTmpbuf.rcWndROI.right = 0;
// 			stTmpbuf.rcWndROI.top = 0;
// 			stTmpbuf.rcWndROI.bottom = 0;
// 			stTmpbuf.m_nImgW = 0;
// 			stTmpbuf.m_nImgH = 0;
// 			stTmpbuf.m_nImgWndW = 0;
// 			stTmpbuf.m_nImgWndH = 0;
// 
// 			stTmpbuf.nAlgoType = eAlgoFoot;
// 			stTmpbuf.m_nImgW = pFoot->m_ImageBuffer->nImageSizeX;
// 			stTmpbuf.m_nImgH = pFoot->m_ImageBuffer->nImageSizeY;
// 			stTmpbuf.rcWndROI = pFoot->mWindowImageRect;
// 			stTmpbuf.m_nImgWndW = pFoot->mWindowImageRect.right - pFoot->mWindowImageRect.left;
// 			stTmpbuf.m_nImgWndH = pFoot->mWindowImageRect.bottom - pFoot->mWindowImageRect.top;
// 
// 			stTmpbuf.pucImage = NULL;
// 			stTmpbuf.pucImageBin = NULL;
// 			stTmpbuf.pucImageWnd = NULL;
// 
// 			//속도 Test 위한 log 추가
// 			//fTacTime = ((GetTickCount() - start) / 1000.0f);
// 			//sLog.Format(_T("PInsp_Wedge_FootMeasure(), 0_init AI Structure!!, TacTime:[%f]ms"), fTacTime);
// 			//g_pMPTI->AddLog_Dev(sLog);
// 
// 			//AI input Image 
// 			cv::Mat cvFootTRImg;
// 			cv::Mat TRImg(nHeight, nWidth, CV_8UC1, pFoot->m_ImageBuffer->imgTop_R);
// 			cvFootTRImg = TRImg.clone();
// 			//cv::imwrite("D:\\FootRst\\0_Foot_TopR.bmp", cvFootTRImg);
// 
// 			cv::Mat roFootTRImg;
// 			cv::Point MarginTest; MarginTest.x = 0; MarginTest.y = 0;
// 			MarginTest = WarpAffine_IPPRotate(cvFootTRImg, roFootTRImg, seta1, cvFootTRImg.size(), CenterPo);	//Foot TR Img E¸Au
// 			//cvFootTRImg.copyTo(roFootTRImg);
// 
// 			//속도 Test 위한 log 추가
// 			//fTacTime = ((GetTickCount() - start) / 1000.0f);
// 			//sLog.Format(_T("PInsp_Wedge_FootMeasure(), 0_Foot Rotate_Image!!, TacTime:[%f]ms"), fTacTime);
// 			//g_pMPTI->AddLog_Dev(sLog);
// 
// 			//cv::Mat cvAIInput_FootImg;
// 			//cv::Mat cvAIInput_WingBWImg;
// 			//cvAIInput_FootImg = roFootTRImg.clone();
// 			//cvAIInput_WingBWImg = BinImage.clone();
// 
// 			/////////////////////////////////////////////////
// 			//UCHAR* pUcImg_Foot = g_pMManager->pem_new<UCHAR>(true, stTmpbuf.m_nImgW * stTmpbuf.m_nImgH, (PCHAR)__FUNCTION__, __LINE__);
// 			//UCHAR* pUcImg_WingBW = g_pMManager->pem_new<UCHAR>(true, stTmpbuf.m_nImgW * stTmpbuf.m_nImgH, (PCHAR)__FUNCTION__, __LINE__);
// 			//memset(pUcImg_Foot, 0, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 			//memset(pUcImg_WingBW, 0, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 
// 			//pUcImg_Foot = cvAIInput_FootImg.data;
// 			//pUcImg_WingBW = cvAIInput_WingBWImg.data;
// 			////////////////////////////////////////////////////////////////////////////////////
// 
// 			//속도 Test 위한 log 추가
// 			//fTacTime = ((GetTickCount() - start) / 1000.0f);
// 			//sLog.Format(_T("PInsp_Wedge_FootMeasure(), 1_Make AI_Structure!!, TacTime:[%f]ms"), fTacTime);
// 			//g_pMPTI->AddLog_Dev(sLog);
// 
// 			float fFootTeachMax = pFoot->m_fArrOptionValue[m_eFoot_Width_1][(int)m_eMMD::eMMD_Max];
// 			float fFootTeachMin = pFoot->m_fArrOptionValue[m_eFoot_Width_1][(int)m_eMMD::eMMD_Min];
// 			float fFootResult = sRstAlgo->m_fArrRst[(int)m_eFoot::m_eFoot_Width_1];
// 
// 			if (fFootResult > fFootTeachMax || fFootResult < fFootTeachMin)
// 			{
// 				//If Foot Width Result NG
// 				//영상 버퍼 할당
// 
// 				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &stTmpbuf.pucImage, stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &stTmpbuf.pucImageBin, stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 
// 
// 				//속도 Test 위한 log 추가
// 				//fTacTime = ((GetTickCount() - start) / 1000.0f);
// 				//sLog.Format(_T("PInsp_Wedge_FootMeasure(), 2_Make_1DArray AI_Structure_Image!!, TacTime:[%f]ms"), fTacTime);
// 				//g_pMPTI->AddLog_Dev(sLog);
// 
// 				memset(stTmpbuf.pucImage, 0, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 				memset(stTmpbuf.pucImageBin, 0, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 
// 				//속도 Test 위한 log 추가
// 				//fTacTime = ((GetTickCount() - start) / 1000.0f);
// 				//sLog.Format(_T("PInsp_Wedge_FootMeasure(), 3_memset0 AI_Structure_Image!!, TacTime:[%f]"), fTacTime);
// 				//g_pMPTI->AddLog_Dev(sLog);
// 
// 				memcpy(stTmpbuf.pucImage, roFootTRImg.data, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 				memcpy(stTmpbuf.pucImageBin, BinImage.data, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 				//memcpy(stTmpbuf.pucImage, pUcImg_Foot, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 				//memcpy(stTmpbuf.pucImageBin, pUcImg_WingBW, sizeof(UCHAR) * stTmpbuf.m_nImgW * stTmpbuf.m_nImgH);
// 
// 				//속도 Test 위한 log 추가
// 				//fTacTime = ((GetTickCount() - start) / 1000.0f);
// 				//sLog.Format(_T("PInsp_Wedge_FootMeasure(),  4_memcpy AI_Structure_Image!!, TacTime:[%f]"), fTacTime);
// 				//g_pMPTI->AddLog_Dev(sLog);
// 
// 				//g_pInspMng->vInspAIData[eAlgoFoot].emplace_back(stTmpbuf);
// 
// 				//속도 Test 위한 log 추가
// 				//fTacTime = ((GetTickCount() - start) / 1000.0f);
// 				//sLog.Format(_T("PInsp_Wedge_FootMeasure(), 5_Complete vector add AI_Foot Buffer!!, StructureSize:[%d]_TacTime:[%f]"), g_pInspMng->vInspAIData[eAlgoFoot].size(), fTacTime);
// 				//g_pMPTI->AddLog_Dev(sLog);
// 
// 				//g_pMManager->pem_delete(pUcImg_Foot, true);
// 				//g_pMManager->pem_delete(pUcImg_WingBW, true);
// 				//Delete_1DArray(&pUcImg_WingBW);
// 				//Delete_1DArray(&pUcImg_Foot);
// 			}
// 			else
// 			{
// 				stTmpbuf.m_nImgW = 0;
// 				stTmpbuf.m_nImgH = 0;
// 			}
// 
// 		}
// 

		//tailLength result / Length
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Tail_L;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);

		//tailLength result / Area
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Tail_A;
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, fArea);

		//Foot Damage(Imprint) result
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Damage;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);


		//Lift
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Height;
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, fHightDiff);


		//shift
		//Insp_FootShift(pFoot, sRstAlgo, startPad, endPad, startPadY, endPadY, nMatLeft, nMatTop, nMatRight, nMatBottom, LeftIdx);

// 		cv::Point cvPoNewCenter;
// 		cvPoNewCenter.x = CenterPo.x - Margin.x;
// 		cvPoNewCenter.y = CenterPo.y - Margin.y;
// 
// 		bool shiftRst = Insp_FootShift_ImgAnd(pFoot, sRstAlgo, &Img, PadImg, &PadRangeImg, cvPoNewCenter);


		//FootAI AI¹IAo¸| ³N°aA≫¶§ ¿¹¿UA³¸®(Foot ¹þ¾i³² A?Aa³ª´A ºIºÐ A³¸®) 
// 		if (vPoSt[(int)m_eFoot::m_eFoot_Tail_L].x == 0 || vPoSt[(int)m_eFoot::m_eFoot_Tail_L].x >= PadRangeImg.cols - 1
// 			|| vPoSt[(int)m_eFoot::m_eFoot_Tail_L].y == 0 || vPoSt[(int)m_eFoot::m_eFoot_Tail_L].y >= PadRangeImg.rows - 1)
// 		{
// 			sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
// 		}

		//FootAI AI¹IAo¸| ³N°aA≫¶§ ¿¹¿UA³¸®(Foot ¹þ¾i³² A?Aa³ª´A ºIºÐ A³¸®) 
		cv::Mat ImgOrg;
		if (Images.size() < 6)
		{
			ImgOrg.setTo(0);
			bool bRoWingImgBackToOrgPos = RotateBackToOriginalPosition(pFoot, fHRoImg, Img, Img, CenterPo, Margin, seta, &ImgOrg);

			//cv::Point pMargin = WarpAffine_IPPRotate(Img, ImgOrg, -sRstAlgo->m_fAngle, Img.size(), cvPoNewCenter, false, true);
		}
		else
			ImgOrg = Images[5].clone();


		int nPadROIRectMinX = ImgOrg.cols;
		int nPadROIRectMaxX = 0;
		int nPadROIRectMinY = ImgOrg.rows;
		int nPadROIRectMaxY = 0;
		for (int r = 0; r < ImgOrg.rows; r++)
		{
			UCHAR* Ptr = ImgOrg.data;
			for (int c = 0; c < ImgOrg.cols; c++)
			{
				if (Ptr[r * ImgOrg.step + c] == 255)
				{
					if (r < nPadROIRectMinY)	nPadROIRectMinY = r;
					if (r > nPadROIRectMaxY)	nPadROIRectMaxY = r;
					if (c < nPadROIRectMinX)	nPadROIRectMinX = c;
					if (c > nPadROIRectMaxX)	nPadROIRectMaxX = c;
				}
			}
		}
		bool bInspShift_OnEdge = false;

		if (135 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 45)//90
		{
			for (int i = 0; i < ImgOrg.rows; i++)
			{
				if (ImgOrg.ptr(i)[0] > 0)
				{
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
					bInspShift_OnEdge = true;
					break;
				}
			}
		}
		else if (225 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 135)//180
		{
			for (int i = 0; i < ImgOrg.cols; i++)
			{
				if (ImgOrg.ptr(ImgOrg.rows - 1)[i] > 0)
				{
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
					bInspShift_OnEdge = true;
					break;
				}
			}
		}
		else if (315 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 225)//270
		{
			for (int i = 0; i < ImgOrg.rows; i++)
			{
				if (ImgOrg.ptr(i)[ImgOrg.cols - 1] > 0)
				{
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
					bInspShift_OnEdge = true;
					break;
				}
			}
		}
		else //0
		{
			for (int i = 0; i < ImgOrg.rows; i++)
			{
				if (ImgOrg.ptr(0)[i] > 0)
				{
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
					sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
					bInspShift_OnEdge = true;
					break;
				}
			}
		}

		if (bInspShift_OnEdge && pFoot->bUseRemovedPadArea)
		{
			//Foot BW 영상(ImgOrg)에서 Foot이 아닌 바닥 영역이 객체로 함께 잡힌경우 해당 부분 제거를 위한 작업
			//3D 기울기와 높이를 함께 확인 (입력:3D 영상)
			cv::Mat cvOnlyPadBW = CalcOnlyPadArea(pFoot, Img.cols, Img.rows, (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC);
			ImgOrg -= cvOnlyPadBW;

			sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = true;
			sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = true;
			sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = true;

			if (135 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 45)//90
			{
				for (int i = 0; i < ImgOrg.rows; i++)
				{
					if (ImgOrg.ptr(i)[0] > 0)
					{
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
						break;
					}
				}
			}
			else if (225 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 135)//180
			{
				for (int i = 0; i < ImgOrg.cols; i++)
				{
					if (ImgOrg.ptr(ImgOrg.rows - 1)[i] > 0)
					{
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
						break;
					}
				}
			}
			else if (315 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 225)//270
			{
				for (int i = 0; i < ImgOrg.rows; i++)
				{
					if (ImgOrg.ptr(i)[ImgOrg.cols - 1] > 0)
					{
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
						break;
					}
				}
			}
			else //0
			{
				for (int i = 0; i < ImgOrg.rows; i++)
				{
					if (ImgOrg.ptr(0)[i] > 0)
					{
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
						break;
					}
				}
			}

		}
		//°E≫c ÆCA¤AI ³¡³ª°i ³ª¼­¾ß ¿μ≫o Display
		if (pFoot->bUseTeachRect)
		{
			int nImageHeight = pFoot->GetImageLength();
			int nImageWidth = pFoot->GetImageWidth();
			cv::Mat cv3Ddata(nImageHeight, nImageWidth, CV_32FC1, pFoot->pf3D_OrgROI);
			cv::Mat cv2Ddata = PadImg.clone();
			cv::Point FfMargin; FfMargin.x = 0, FfMargin.y = 0;
			cv::Mat cvRotate3Ddata;
			FfMargin = WarpAffine_IPPRotate_3D(cv3Ddata, cvRotate3Ddata, seta, cv3Ddata.size(), CenterPo);
			cv::Mat cvRstPadDamaged(nImageHeight, nImageWidth, CV_8UC1);
			cvRstPadDamaged.setTo(0);
			size_t floatstep_3d = cvRotate3Ddata.step / sizeof(float);
			for (int r = 0; r < cv2Ddata.rows; r++)
			{
				UCHAR* Ptr2d = cv2Ddata.data;
				float* Ptr_Foot3D = (float*)cvRotate3Ddata.data;
				UCHAR* RstPtr = cvRstPadDamaged.data;
				for (int c = 0; c < cv2Ddata.cols; c++)
				{
					if (Ptr_Foot3D[r * floatstep_3d + c] < pFoot->m_fPadArea3DAvgHeight + 20 &&
						Ptr_Foot3D[r * floatstep_3d + c] < pFoot->m_fPadArea3DAvgHeight -100 )
					{
						if (Ptr2d[r * cv2Ddata.step + c] == 255)
						{
							RstPtr[r * cv2Ddata.step + c] = 255;
						}
					}
				}
			}
			cv::Mat kernel_mor = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
			cv::erode(cvRstPadDamaged, cvRstPadDamaged, kernel_mor);
			cv::erode(cvRstPadDamaged, cvRstPadDamaged, kernel_mor);
			cv::dilate(cvRstPadDamaged, cvRstPadDamaged, kernel_mor);
			cv::dilate(cvRstPadDamaged, cvRstPadDamaged, kernel_mor);
			int nMinBlob = 300;
			int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(cvRstPadDamaged.data, cvRstPadDamaged.data, nImageWidth, nImageHeight, nMinBlob, FALSE, TRUE, 0, eSelectMix);
			if (nCntBlob > 0)
			{
				int nPadDamagedRectMinX = nImageWidth;
				int nPadDamagedRectMaxX = 0;
				int nPadDamagedRectMinY = nImageHeight;
				int nPadDamagedRectMaxY = 0;
				for (int r = 0; r < nImageHeight; r++)
				{
					UCHAR* Ptr = cvRstPadDamaged.data;
					for (int c = 0; c < nImageWidth; c++)
					{
						if (Ptr[r * cvRstPadDamaged.step + c] == 255)
						{
							if (r < nPadDamagedRectMinY)	nPadDamagedRectMinY = r;
							if (r > nPadDamagedRectMaxY)	nPadDamagedRectMaxY = r;
							if (c < nPadDamagedRectMinX)	nPadDamagedRectMinX = c;
							if (c > nPadDamagedRectMaxX)	nPadDamagedRectMaxX = c;
						}
					}
				}
				int nFootROIRectMinX = roImg.cols;
				int nFootROIRectMaxX = 0;
				int nFootROIRectMinY = roImg.rows;
				int nFootROIRectMaxY = 0;
				for (int r = 0; r < roImg.rows; r++)
				{
					UCHAR* Ptr = roImg.data;
					for (int c = 0; c < roImg.cols; c++)
					{
						if (Ptr[r * roImg.step + c] == 255)
						{
							if (r < nFootROIRectMinY)	nFootROIRectMinY = r;
							if (r > nFootROIRectMaxY)	nFootROIRectMaxY = r;
							if (c < nFootROIRectMinX)	nFootROIRectMinX = c;
							if (c > nFootROIRectMaxX)	nFootROIRectMaxX = c;
						}
					}
				}
				if (nPadDamagedRectMinX != nImageWidth && nPadDamagedRectMaxX!=0 && nPadDamagedRectMinY!= nImageHeight && nPadDamagedRectMaxY!=0)
				{
					if (nPadDamagedRectMinY < nFootROIRectMinY)
					{
						CString cstrInspLog;
						cstrInspLog.Format(_T("[FootAlgo]::FootMeasure()_Deviation_CheckPadCrack : Fail"));
						g_pMPTI->AddLog(cstrInspLog);
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_Deviation] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationX] = false;
						sRstAlgo->m_bArrOK[(int)m_eFoot::m_eFoot_DeviationY] = false;
					}
				}
			}
		}
#pragma region Write UI_Image
		nLine = __LINE__;
		//
		//UI¿¡ C￥½ACI±a A§CN AI¹IAo (ucArrDstImg)≫y¼º
		if (ucArrDstImg != NULL)
		{
			nLine = __LINE__;
			int nWidth = pFoot->GetImageWidth();
			int nHeight = pFoot->GetImageLength();
			nLine = __LINE__;

			cv::Mat mMask = cvFootImg.clone();
#ifdef _AlgoTool
			nUIFootBinIDX = 3;
#endif
			m_eFootBin footbin = (m_eFootBin)nUIFootBinIDX;

			//shkim edge UI work
			cv::Mat cvPadUI = (PadImg).clone();
			cv::Mat cvFootUI = cvFootImg.clone();
			cvPadUI = cvPadUI - cvFootUI;


			cv::Mat WedgeOrg = cv::Mat(cvFootImg.rows, cvFootImg.cols, CV_8UC1);
			WedgeOrg.setTo(0);

			cv::Mat cvWingUI;
			if (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing && bWingRstVisible)
			{
				//UI ¿¡ C￥½ACO Wing AI¹IAo 
				cv::Mat WingOrg = cv::Mat(cvFootImg.rows, cvFootImg.cols, CV_8UC1);
				WingOrg.setTo(0);
				bool bRoWingImgBackToOrgPos = RotateBackToOriginalPosition(pFoot, fHRoImg, cvFootUI, (*wingCandiImgSec), CenterPo, Margin, seta, &WingOrg);
				cvWingUI = WingOrg.clone();
			}
			else
				cvWingUI = cvWing.clone();

			cv::Mat cvWedgeUI;
			if (bWingRstVisible)
			{
				bool bRoWingImgBackToOrgPos = RotateBackToOriginalPosition(pFoot, fHRoImg, cvFootUI, wedgeMaxBlob, CenterPo, Margin, seta, &cvWedgeUI);
			}
			else
			{
				bool bRoWingImgBackToOrgPos = RotateBackToOriginalPosition(pFoot, fHRoImg, cvFootUI, cvWedgeImg, CenterPo, Margin, seta, &cvWedgeUI);
			}
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));

			cv::Mat cvPadEdge, cvFootEdge, cvWingEdge, cvWedgeEdge;
			cv::Canny(cvPadUI, cvPadEdge, 0, 0);
			cv::Canny(cvFootUI, cvFootEdge, 0, 0);
			cv::erode(cvWingUI, cvWingEdge, kernel);
			cv::erode(cvWedgeUI, cvWedgeEdge, kernel);

			cv::dilate(cvPadEdge, cvPadEdge, kernel);
			//cv::dilate(cvFootEdge, cvFootEdge, kernel);
			cv::bitwise_xor(cvWingUI, cvWingEdge, cvWingEdge);
			cv::bitwise_xor(cvWedgeUI, cvWedgeEdge, cvWedgeEdge);

#if _DEBUG
			cv::imwrite("D:\\cvWedgeEdge.bmp", cvWedgeEdge);
			cv::imwrite("D:\\cvFootEdge.bmp", cvFootEdge);
			cv::imwrite("D:\\cvWingEdge.bmp", cvWingEdge);
			cv::imwrite("D:\\cvWedgeEdge.bmp", cvWedgeEdge);
#endif

			UCHAR* ptrPad = cvPadEdge.data;
			UCHAR* ptrFoot = cvFootEdge.data;
			UCHAR* ptrWing = cvWingEdge.data;
			UCHAR* ptrWedge = cvWedgeEdge.data;

			if (pFoot->m_rst_Color_Image) //Color
			{

				for (int r = 0; r < cvFootImg.rows; r++)
				{
					for (int c = 0; c < cvFootImg.cols; c++)
					{
						int nIndex = r * nWidth + c;
						int nIndex2 = (nStartY + r) * pFoot->_WidthOrg + (nStartX + c);
						nIndex2 *= 3;

						// 						if (footbin == m_eFootBin_Wing && ptrWing[nIndex] && bWingRstVisible)
						// 							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWing, 3);
						// 						else if (footbin == m_eFootBin_Wedge && ptrWedge[nIndex] && bWingRstVisible)
						// 							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWedge, 3);
						// 						else
						{
							//Wing
							if (ptrWing[nIndex] && (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing && bWingRstVisible))
								memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWing, 3);

							//Wedge
							else if (ptrWedge[nIndex] && (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing && bWingRstVisible))
								memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWedge, 3);
						}
					}
				}
			}
			else   //Binary
			{
				for (int r = 0; r < mMask.rows; r++)
				{
					for (int c = 0; c < mMask.cols; c++)
					{
						int nIndex = r * nWidth + c;
						int nIndex2 = (nStartY + r) * pFoot->_WidthOrg + (nStartX + c);

						UCHAR ucData = ptrFoot[nIndex]; //Foot
						if (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wing && bWingRstVisible)
							ucData = ptrWing[nIndex]; //Wing
						else if (nUIFootBinIDX == (int)m_eFootBin::m_eFootBin_Wedge && bWingRstVisible)
							ucData = ptrWedge[nIndex]; //Foot

						if (ucData > 0) ucArrDstImg[nIndex2] = ucData;
					}
				}
			}

			std::vector<cv::Point> vPoDispSt;
			std::vector<cv::Point> vPoDispEd;
			CalcDisplayPnt(seta, vPoSt, vPoEd, vPoDispSt, vPoDispEd);
			CalcRotPnt(CenterPo, -seta, vPoDispSt, vPoDispEd, Margin);

			for (int i = 0; i < m_eFoot::m_eFoot_Total; i++)
			{
				sRstAlgo->DispPoinSt[i].x = vPoDispSt[i].x;
				sRstAlgo->DispPoinSt[i].y = vPoDispSt[i].y;

				sRstAlgo->DispPoinEd[i].x = vPoDispEd[i].x;
				sRstAlgo->DispPoinEd[i].y = vPoDispEd[i].y;
			}
#if _DEBUG
			ImagePointSave(_T("D:\\FootRst\\Bin2D_End_Disp.bmp"), &Img, vPoDispSt, vPoDispEd);
#endif
		}
		//////////////////////

		return CenterPo;
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("[FootAlgo]::FootMeasure() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);
	}

	return CenterPo;
}

int CPInsp_Wedge::FootPointByType(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, FootPoins &fPoints, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, std::vector<cv::Mat> &Images, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter)
{
	int ret = 0;
	int nLine = __LINE__;
	try
	{
		int nImageHeight = pFoot->GetImageLength();
		int nImageWidth = pFoot->GetImageWidth();
		cv::Mat cv3DArea(nImageHeight, nImageWidth, CV_32FC1, pFoot->pf3D);

		int nAreaGapX = -5.;
		int nAreaGapY = -5.;
		int nSX = fPoints.rcArea.left + nAreaGapX;
		int nEX = fPoints.rcArea.right - nAreaGapX;
		int nSY = fPoints.rcArea.top + nAreaGapY;
		int nEY = fPoints.rcArea.bottom - nAreaGapY;

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
				nEY = nImageHeight;
		}

		if (nSX < 0) nSX = 0;
		if (nSY < 0) nSY = 0;
		if (nEX < 0) nEX = 0;
		if (nEY < 0) nEY = 0;
		if (nSX > nImageWidth) nSX = nImageWidth;
		if (nSY > nImageHeight) nSY = nImageHeight;
		if (nEX > nImageWidth) nEX = nImageWidth;
		if (nEY > nImageHeight) nEY = nImageHeight;

		FootPoins fPoints_Foot;
		cv::Mat ImgF;
		cv::Mat cvRofootRctBW(cvFootImg->rows, cvFootImg->cols, CV_8UC1);
		bool bIsFoundFoot = FindFoot_3DBlob(pFoot, sRstAlgo, &ImgF, &ImgF, &ImgF, cvPadImg, fPoints_Foot, nUIFootBinIDX, cv3DAvgFilter, cvRofootRctBW, NULL, false, false, false);

		cv::Mat cvFootImgForInspRct = fPoints_Foot.cvMatchingMask.clone();

		SetBWForUIImage(pFoot, nSX, nSY, nEX, nEY, cvFootImg, cvWedgeImg, cvWing, &fPoints_Foot);		//NYJ 2020/07/01
				
		if (pFoot->m_bUse2Foot)
		{
		cv::Mat cvFootTmpImg = (*cvFootImg).clone();
		cvFootTmpImg = cvFootTmpImg & cvFootImgForInspRct;

		cvFootTmpImg.copyTo(*cvFootImg);
		}

		cv::Mat BinImage1 = *cvFootImg;
		cv::Mat cvFootBinImg = cvFootImg->clone();
		//DBC ½CAⓒ ¶oAI °u·A 2D·I °EAa
		if ((pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC) == m_eFOOT_Data2::m_eFOOT_Data2_DBC)
		{
			cv::Mat DBCBinImage(BinImage1.rows, BinImage1.cols, CV_8UC1);

			bool bIsFindDBCOutLine;
			if (nFootDir == 1 || nFootDir == 3)
				bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage);
			else
				bIsFindDBCOutLine = FindDBCOutLines(pFoot, &DBCBinImage, 0, nFootDir);

			///////////////////////
			if (bIsFindDBCOutLine)
			{
				BinImage1 = BinImage1 - DBCBinImage;

				cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
				cv::Mat cvWingTmpImage(BinImage1.rows, BinImage1.cols, CV_8UC1);
				cvWingTmpImage.setTo(0);

				cv::dilate(*cvWing, cvWingTmpImage, kernel);

				BinImage1 = BinImage1 + cvWingTmpImage;
			}
		}

		cv::Mat BinImage4(nImageHeight, nImageWidth, CV_8UC1);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage1.bmp", BinImage1);
#endif
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(BinImage1, BinImage1, kernel);
		cv::dilate(BinImage1, BinImage1, kernel);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Mop1.bmp", BinImage1);
#endif
		double dArea = 0, dCX = 0, dCY = 0;
		CRect rcBlob{ 0,0,0,0 };
		int nMinBlob = 100;

		// 		PIAL::PI_Buff src(BinImage1);
		// 		PIAL::PI_Buff dst(nImageWidth, nImageHeight);
		// 		dst.Mat().setTo(0);
		// 
		// 		int nCntBlob = 0;
				//nCntBlob = PIAL::PI_Blob::CalcBlob_Select(&src, &dst, nImageWidth, nImageHeight, nMinBlob, FALSE, TRUE, 0, _eSelectMix);

		//		BinImage4 = dst.Mat();

		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage1.data, BinImage4.data, nImageWidth, nImageHeight, nMinBlob, FALSE, TRUE, 0, eSelectMix);

		if (nCntBlob > 0)
		{
			dCX = 0, dCY = 0, dArea = 0;
			m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);

			//m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
			//PIAL::PI_Blob::BlobGet_Result(&dArea, &dCX, &dCY, &rcBlob, false);
		}

#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage4.bmp", BinImage4);
#endif
		//  		if (fPoints.bMatchSuccess)
		//  		{
		//  #if _DEBUG
		//  			cv::imwrite("D:\\FootRst\\bMatchSuccess_before.bmp", BinImage4);
		//  #endif
		//  			cv::bitwise_and((*cvFootImg), fPoints.cvMatchingMask, BinImage4);
		//  #if _DEBUG
		//  			cv::imwrite("D:\\FootRst\\bMatchSuccess_After.bmp", BinImage4);
		//  #endif
		//  		}

		if (pFoot->m_bUse2Foot && fPoints.bMatchSuccess)
		{
			//º≫μu Rect ·I ¾Æ·¡AE A|°A
			for (int r = 0; r < BinImage4.rows; r++)
			{
				UCHAR* Ptr = (*cvFootImg).data;
				UCHAR* Ptr2 = BinImage4.data;

				for (int c = 0; c < BinImage4.cols; c++)
				{
					if (Ptr[r * BinImage4.step + c] == 255)
					{
						if (r > pFoot->m_BondingRect.bottom)
						{
							Ptr[r * BinImage4.step + c] = 0;
							Ptr2[r * BinImage4.step + c] = 0;
						}
					}
				}
			}
		}

		nMinBlob = dArea * 0.1;
		if (100 > nMinBlob) nMinBlob = 100;
		int selectblob = _eSelectMix;
		if (!pFoot->m_bUsePatternAngle)
			selectblob = eSelectBigger;

		// 		PIAL::PI_Buff src_bin(BinImage4);
		// 		PIAL::PI_Buff dst_bin(*cvFootImg);

		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage4.data, cvFootImg->data, nImageWidth, nImageHeight, nMinBlob, FALSE, TRUE, 0, eSelectBigger);
		//nCntBlob = PIAL::PI_Blob::CalcBlob_Select(&src_bin, &dst_bin, nImageWidth, nImageHeight, nMinBlob, FALSE, TRUE, 0, eSelectBigger);

		int nBlobCX = 0, nBlobCY = 0;
		if (nCntBlob > 0)
		{
			double dCXFoot = 0, dCYFoot = 0, dAreaFoot = 0;
			CRect rcBlob_Foot{ 0,0,0,0 };
			m_pProcMilAlgo->GetBlobResult(&dAreaFoot, &dCXFoot, &dCYFoot, &rcBlob_Foot, FALSE);

			nBlobCX = dCXFoot;
			nBlobCY = dCYFoot;

			//m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
			//PIAL::PI_Blob::BlobGet_Result(&dArea, &dCX, &dCY, &rcBlob, false);
		}

#if _DEBUG
		cv::imwrite("D:\\FootRst\\Find_Foot.bmp", *cvFootImg);
#endif

		if (false/*!fPoints.bMatchSuccess || !pFoot->m_bUsePatternAngle*/)
		{
			//Foot CenterPos, Seta
			cv::Point CenterPo(-1, -1);
			double seta(0.0f);

			cv::Mat cvWingBin(nImageHeight, nImageWidth, CV_8UC1);
			cvWingBin.setTo(0);

			cv::Mat Img = (cvWingBin).clone();
			cv::Mat mMask = (*cvFootImg).clone();

			cv::Point2d WingLen;

			if (pFoot->m_nFoottype == 0)	//Wedge Type
			{
				Img = (*cvWing).clone();
			}

			bool flag = getFootCenterPosAndSeta(CenterPo, seta, Img, mMask, WingLen, pFoot->dAngle);

			if (abs(pFoot->m_nFootAngle - seta) > 135)seta -= 180;
			fPoints.m_dSeta = seta;

			if (flag)
			{
				fPoints.m_pCenter.x = CenterPo.x;
				fPoints.m_pCenter.y = CenterPo.y;

				//if (CenterPo.y > Img.rows * 0.8)
				{
					///////////////////////////////////
					std::vector<std::vector<cv::Point>> vContMax;
					std::vector<cv::Vec4i> hierarchy;
					cv::findContours(Img, vContMax, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
					nLine = __LINE__;
#if _DEBUG
					cv::Mat dstback = Img.clone();
					cv::Mat testimage1;
					cv::cvtColor(dstback, testimage1, cv::COLOR_GRAY2RGB);
#endif
					double dWingAngle = 0;
					int nIdxMaxRectSize = 0;
					cv::Point cvRectCenter;

					for (int i = 0; i < vContMax.size(); i++)
					{
						cv::RotatedRect RoRe = cv::minAreaRect(vContMax[i]);
						//vDstRect.push_back(RoRe);

						if (nIdxMaxRectSize < vContMax[i].size())
							nIdxMaxRectSize = vContMax[i].size();
						else
							continue;

						cv::Rect re = cv::boundingRect(vContMax[i]);
#if _DEBUG
						cv::rectangle(testimage1, re, cv::Scalar(0, 0, 255), 2);
#endif

						cv::Point2f ArrRe2[4];

						RoRe.points(ArrRe2);
						dWingAngle = RoRe.angle;
						cvRectCenter.x = RoRe.center.x;
						cvRectCenter.y = RoRe.center.y;
#if _DEBUG
						for (int i = 0; i < 4; i++)
							cv::line(testimage1, ArrRe2[i], ArrRe2[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
#endif
					}
					double dFootTheta = 0;

					if (dWingAngle != 0)
					{
						if (pFoot->m_nFootAngle == 0)
							dFootTheta = dWingAngle + 90;
						else
							dFootTheta = dWingAngle + 270;
					}
					if (abs(fPoints.m_dSeta - dFootTheta) > 50)
						dFootTheta -= 90;

					cv::Mat roTestImg;
					cv::Point MarginTest; MarginTest.x = 0; MarginTest.y = 0;
					MarginTest = WarpAffine_IPPRotate(Img, roTestImg, dFootTheta, Img.size(), cvRectCenter);	//Wing Bin Img E¸Au

					////////////////
					if (CenterPo.y > Img.rows * 0.8)
					{
						fPoints.m_pCenter.x = cvRectCenter.x;
						fPoints.m_pCenter.y = cvRectCenter.y;
					}
				}
			}
			else
			{
				///////////////////////////////////
				std::vector<std::vector<cv::Point>> vContMax;
				std::vector<cv::Vec4i> hierarchy;
				cv::findContours(Img, vContMax, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
				nLine = __LINE__;
#if _DEBUG
				cv::Mat dstback = Img.clone();
				cv::Mat testimage1;
				cv::cvtColor(dstback, testimage1, cv::COLOR_GRAY2RGB);
#endif
				double dWingAngle = 0;
				int nIdxMaxRectSize = 0;
				cv::Point cvRectCenter;

				for (int i = 0; i < vContMax.size(); i++)
				{
					cv::RotatedRect RoRe = cv::minAreaRect(vContMax[i]);
					//vDstRect.push_back(RoRe);

					if (nIdxMaxRectSize < vContMax[i].size())
						nIdxMaxRectSize = vContMax[i].size();
					else
						continue;
					cv::Rect re = cv::boundingRect(vContMax[i]);
#if _DEBUG
					cv::rectangle(testimage1, re, cv::Scalar(0, 0, 255), 2);
#endif
					cv::Point2f ArrRe2[4];

					RoRe.points(ArrRe2);
					dWingAngle = RoRe.angle;
					cvRectCenter.x = RoRe.center.x;
					cvRectCenter.y = RoRe.center.y;

#if _DEBUG
					for (int i = 0; i < 4; i++)
						cv::line(testimage1, ArrRe2[i], ArrRe2[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
#endif
				}
				double dFootTheta = 0;

				if (dWingAngle != 0)
				{
					if (pFoot->m_nFootAngle == 0)
						dFootTheta = dWingAngle + 90;
					else
						dFootTheta = dWingAngle + 270;
				}

				cv::Mat roTestImg;
				cv::Point MarginTest; MarginTest.x = 0; MarginTest.y = 0;
				MarginTest = WarpAffine_IPPRotate(Img, roTestImg, dFootTheta, Img.size(), cvRectCenter);	//Wing Bin Img E¸Au

				////////////////
				fPoints.m_dSeta = dFootTheta;
				fPoints.m_pCenter.x = cvRectCenter.x;
				fPoints.m_pCenter.y = cvRectCenter.y;
			}
		}
// 		if (45 >= pFoot->m_nFootAngle || pFoot->m_nFootAngle > 315)// ^
// 		{
// 			if (abs(fPoints.m_dSeta) > 90)
// 			{
// 				fPoints.m_dSeta += 180;
// 			}
// 		}
// 		else if (225 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 135)// V
// 		{
// 			if (abs(fPoints.m_dSeta) < 90)
// 			{
// 				fPoints.m_dSeta += 180;
// 			}
// 		}
// 		else if (135 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 45)// <
// 		{
// 			while (fPoints.m_dSeta < 0) { fPoints.m_dSeta += 360; }
// 			if (abs(fPoints.m_dSeta) < 180)
// 			{
// 				fPoints.m_dSeta += 180;
// 			}
// 		}
// 		else if (315 >= pFoot->m_nFootAngle && pFoot->m_nFootAngle > 225)// >
// 		{
// 			while (fPoints.m_dSeta < 0) { fPoints.m_dSeta += 360; }
// 			if (abs(fPoints.m_dSeta) > 180)
// 			{
// 				fPoints.m_dSeta += 180;
// 			}
// 		}


		cv::Point CenterPo(0, 0);
		CenterPo.x = fPoints.m_pCenter.x;
		CenterPo.y = fPoints.m_pCenter.y;

		double seta(0.0);
		seta = fPoints.m_dSeta;

		cv::Mat WingCandi_Second(cvWing->rows, cvWing->cols, CV_8UC1);
		WingCandi_Second.setTo(0);

		WingCandi_Second = (*wingCandiImgSec).clone();

		FindFootSubArea(pFoot, CenterPo, seta, cvFootImg, cvWedgeImg, cvWing, &cv3DArea, cvPadImg, ucArrDstImg, &WingCandi_Second, nStartX, nStartY, nUIFootBinIDX, bWingRstVisible);

		WingCandi_Second.copyTo(*wingCandiImgSec);
		BinImage4.copyTo(*cvFootImg);
		int nFootMinBlob = 1000;
		int nCntFootBWBlob = m_pProcMilAlgo->CalcBlob_Select(BinImage4.data, cvFootImg->data, nImageWidth, nImageHeight, nFootMinBlob, FALSE, TRUE, 0, eSelectBigger);
		cv::Mat fHImg;
		cv::Mat fHRectImg;
		cv::Mat fHRoImg;

		fHImg = (cv3DArea).clone();
		fHRectImg = fHImg.clone();

		cv::Point FootRoImgMargin; FootRoImgMargin.x = 0, FootRoImgMargin.y = 0;
		//FootRoImgMargin = WarpAffine(fHRectImg, fHRoImg, seta, fHImg.size(), CenterPo);	//Foot 3D Img
		FootRoImgMargin = WarpAffine_IPPRotate_3D(fHRectImg, fHRoImg, seta, fHImg.size(), CenterPo);	//Wing Bin Img E¸Au

		cv::Mat Img = (*cvWing);

		cv::Point bottomPo;
		bottomPo.x = Img.cols;
		bottomPo.y = Img.rows;

		cv::Mat roImg;

		cv::Point Margin; Margin.x = 0; Margin.y = 0;
		//cv::Point Margin = WarpAffine(Img, roImg, seta, Img.size(), CenterPo);		//Wing Bin Img E¸Au
		Margin = WarpAffine_IPPRotate(Img, roImg, seta, Img.size(), CenterPo);	//Wing Bin Img E¸Au

		cv::Mat CutImg = roImg;

		/////////////////////Wing EAº¸±º¿¡¼­ Wire A|°A
		cv::Mat LeftFootAreaBW(roImg.rows, roImg.cols, CV_8UC1);
		cv::Mat LeftFootAreafH(fHRoImg.rows, fHRoImg.cols, CV_32FC1);
		cv::Mat RightFootAreaBW(roImg.rows, roImg.cols, CV_8UC1);
		cv::Mat RightFootAreafH(fHRoImg.rows, fHRoImg.cols, CV_32FC1);

		cv::Mat FootBWRmvFlowDown(roImg.rows, roImg.cols, CV_8UC1);
		CRect rcLeftFootRefROI, rcRightFootRefROI;

		cv::Mat FootBWTeach = (*cvFootImg).clone();
		cv::Mat roImgFootBW = FootBWTeach;
		//cv::Point RoFootImgMargin = WarpAffine(FootBWImg, roImgFootBW, seta, FootBWImg.size(), CenterPo);	//Foot 3D Img
		cv::Point RoFootImgMargin = WarpAffine_IPPRotate(FootBWTeach, roImgFootBW, seta, FootBWTeach.size(), CenterPo);	//Foot 2d Img

		cv::Mat WedgeBWTeach = (*cvWedgeImg).clone();
		cv::Mat roImgWedgeBW = WedgeBWTeach;
		//cv::Point RoWedgeImgMargin = WarpAffine(WedgeBWImg, roImgWedgeBW, seta, WedgeBWImg.size(), CenterPo);	//Foot 3D Img
		cv::Point RoWedgeImgMargin = WarpAffine_IPPRotate(WedgeBWTeach, roImgWedgeBW, seta, WedgeBWTeach.size(), CenterPo);	//Wedge 2D Img

		bool bExtractLeftAndRightAreaOfFoot = ExtractLeftAndRightAreaOfFoot(pFoot, fHRoImg, roImgFootBW, roImgWedgeBW, CutImg, &FootBWRmvFlowDown, &LeftFootAreaBW, &LeftFootAreafH, &RightFootAreaBW, &RightFootAreafH, &rcLeftFootRefROI, &rcRightFootRefROI, CenterPo, Margin);

		//Wing Wire
		int nDetectedWireStY = fHRoImg.rows - 1;
		bool bCalc_MeanStdDevGraph = DetectWireStartPos_UseLeftAndRightAreasOfFootHeight(pFoot, fHRoImg, FootBWRmvFlowDown, LeftFootAreafH, RightFootAreafH, rcLeftFootRefROI, rcRightFootRefROI, CenterPo, Margin, true, &nDetectedWireStY);


		if (bCalc_MeanStdDevGraph)
		{
			for (int r = 0; r < CutImg.rows; r++)
			{
				UCHAR* Ptr = CutImg.data;
				for (int c = 0; c < CutImg.cols; c++)
				{
					if (Ptr[r * CutImg.step + c] == 255)
					{
						//if (r > nDetectedWireStY)
							//Ptr[r * CutImg.step + c] = 0;
					}
				}
			}
		}

#if _DEBUG
		//wire A|°ACN Wing ¿μ≫o AuAa
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_Wing_RemovedWireImg.bmp"), CutImg);
		}
#endif

		// 		bool bCalc_FootHeightProfile;
		// 		bCalc_FootHeightProfile = DetectWireStartPos_UseFootHeightProfile(pFoot, fHRoImg, FootBWRmvFlowDown, CutImg, CenterPo, Margin, &nDetectedWireStY);
		// 
		cv::Mat RotateWingImg(cvWing->rows, cvWing->cols, CV_8UC1);
		RotateWingImg.setTo(0);
		// 			for (int r = 0; r < CutImg.rows; r++)
		// 			{
		// 				UCHAR* Ptr = CutImg.data;
		// 				for (int c = 0; c < CutImg.cols; c++)
		// 				{
		// 					if (Ptr[r * CutImg.step + c] == 255)
		// 					{
		// 						//if (r > nDetectedWireStY)
		// 							//Ptr[r * CutImg.step + c] = 0;
		// 					}
		// 				}
		// 			}
		// 		}

		RotateWingImg = WingCandi_Second.clone();
		// 		//wire A|°ACN Wing ¿μ≫o AuAa
		// 		if (cstDebugFolderPath.IsEmpty() != TRUE)
		// 		{
		// 			CT2CA convertedString(cstDebugFolderPath);
		// 			std::string strFootDebugImgPath = std::string(convertedString);
		// 
		// 			size_t lastindex = strFootDebugImgPath.find_last_of(".");
		// 			string rawname = strFootDebugImgPath.substr(0, lastindex);
		// 
		// 			cv::imwrite(rawname + std::string("_Wing_RemovedWireImg_UseFootHeightProfile.bmp"), CutImg);
		// 		}
		// #endif

				// #if _DEBUG
				// 		//wire A|°ACN Wing ¿μ≫o AuAa
				// 		if (cstDebugFolderPath.IsEmpty() != TRUE)
				// 		{
				// 			CT2CA convertedString(cstDebugFolderPath);
				// 			std::string strFootDebugImgPath = std::string(convertedString);
				// 
				// 			size_t lastindex = strFootDebugImgPath.find_last_of(".");
				// 			string rawname = strFootDebugImgPath.substr(0, lastindex);
				// 
				// 			cv::imwrite(rawname + std::string("_Wing_RemovedWireImg_UseFootHeightProfile.bmp"), CutImg);
				// 		}
				// #endif

						//CutImg.copyTo(*wingCandiImgSec);

						/////////////////////////////////////////////////////

						//wing Point
		std::vector<cv::Point> vPoSt((int)m_eFoot::m_eFoot_Total);
		std::vector<cv::Point> vPoEd((int)m_eFoot::m_eFoot_Total);

		cv::Point LSt, LEd, RSt, REd, LStTemp, RStTemp;
		bool bPadLength = false;

		//º≫μu A§A¡ AE±aE­(EAº¸±º AAC￥·I AE±a°ª ¼¼ÆA)
		cv::Point poInitLSt, poInitRSt;
		int nWingRectMinX = RotateWingImg.cols;
		int nWingRectMaxX = 0;

		int nWingRectMinY = RotateWingImg.rows;
		int nWingRectMaxY = 0;

		for (int r = 0; r < RotateWingImg.rows; r++)
		{
			UCHAR* Ptr = RotateWingImg.data;
			for (int c = 0; c < RotateWingImg.cols; c++)
			{
				if (Ptr[r * RotateWingImg.step + c] == 255)
				{
					if (r < nWingRectMinY)	nWingRectMinY = r;
					if (r > nWingRectMaxY)	nWingRectMaxY = r;

					if (c < nWingRectMinX) nWingRectMinX = c;
					if (c > nWingRectMaxX) nWingRectMaxX = c;

				}
			}
		}

		cv::Mat cvWingBondingArea(RotateWingImg.rows, RotateWingImg.cols, CV_8UC1);
		cvWingBondingArea.setTo(0);

		cv::rectangle(cvWingBondingArea, cv::Rect(nWingRectMinX, nWingRectMinY, nWingRectMaxX - nWingRectMinX, nWingRectMaxY - nWingRectMinY), cv::Scalar(255), cv::FILLED);
#if _DEBUG
		cv::imwrite(("D:\\FootRst\\Wing_PadRectImg.bmp"), cvWingBondingArea);
#endif

// 		if (fPoints.bMatchSuccess)
// 		{
// 			cv::bitwise_and(cvWingBondingArea, fPoints.cvMatchingMask, cvWingBondingArea);
// 		}

		cv::Point nPosLeftWingSt;
		cv::Point nPosRightWingSt;

		bool bCompleteInitPos_LeftWing = false, bCompleteInitPos_RightWing = false;

		for (int r = 0; r < CutImg.rows; r++)
		{
			UCHAR* Ptr = RotateWingImg.data;
			UCHAR* Ptr_BondingArea = cvWingBondingArea.data;

			for (int c = 0; c < CutImg.cols; c++)
			{
				if (Ptr[r * CutImg.step + c] == 255 && Ptr_BondingArea[r * CutImg.step + c] == 255)
				{
					if (!bCompleteInitPos_LeftWing && r < CenterPo.y - Margin.y && c < CenterPo.x - Margin.x)
					{
						nPosLeftWingSt.y = r;
						nPosLeftWingSt.x = c;
						bCompleteInitPos_LeftWing = true;
					}

					if (!bCompleteInitPos_RightWing && r < CenterPo.y - Margin.y && c > CenterPo.x - Margin.x)
					{
						nPosRightWingSt.y = r;
						nPosRightWingSt.x = c;
						bCompleteInitPos_RightWing = true;
					}

				}
			}
		}

		if (bCompleteInitPos_LeftWing && bCompleteInitPos_RightWing)
		{
			poInitLSt = LSt = nPosLeftWingSt;
			poInitRSt = RSt = nPosRightWingSt;
		}
		/////////////////////////

#pragma region wing Point

		//start Point

		std::vector<int> vecWingStartOption;

		if (pFoot->m_nFindOption == m_eFoot_FindOption::Wing) //Pad Length
		{
			vecWingStartOption.push_back((int)m_eFoot_FindOption::Wing);
		}
		else if (pFoot->m_nFindOption == m_eFoot_FindOption::WingGap)
		{
			vecWingStartOption.push_back((int)m_eFoot_FindOption::WingGap);
		}
		else if (pFoot->m_nFindOption == m_eFoot_FindOption::Bonding) //Pad Length   Foot
		{
			vecWingStartOption.push_back((int)m_eFoot_FindOption::Bonding);
		}
		else if (pFoot->m_nFindOption == m_eFoot_FindOption::Slope3D) //Pad Length   3D
		{
			vecWingStartOption.push_back((int)m_eFoot_FindOption::Slope3D);
		}
		else if (pFoot->m_nFindOption == m_eFoot_FindOption::Wing3D) //Pad Length
		{
			vecWingStartOption.push_back((int)m_eFoot_FindOption::Wing3D);
		}
		else
		{
			//Start Point °E≫c ¿i¼±¼øA§ ¼ø¼­·I ³ª¿­
			for (int i = 0; i < pFoot->m_nFindOptionOrder.size(); i++)
				vecWingStartOption.push_back((int)pFoot->m_nFindOptionOrder[i]);
			//vecWingStartOption.push_back((int)m_eFoot_FindOption::Wing3D);
			//vecWingStartOption.push_back((int)m_eFoot_FindOption::Slope3D);
			//vecWingStartOption.push_back((int)m_eFoot_FindOption::WingGap);
			//vecWingStartOption.push_back((int)m_eFoot_FindOption::Bonding);
			//vecWingStartOption.push_back((int)m_eFoot_FindOption::Wing);
		}

		CString cstrInspPadLengthLog;
		for (size_t i = 0; i < vecWingStartOption.size(); i++)
		{
			if (vecWingStartOption[i] == m_eFoot_FindOption::Wing3D)
			{
				bPadLength = getFootBondingStEndPoint2(&CutImg, &fHRoImg, &LSt, &LEd, &RSt, &REd, CenterPo.x - Margin.x, CenterPo.y - Margin.y, 0);
				cstrInspPadLengthLog.Format(_T("[FootAlgo]::Insp_PadLength_StartPoint : Wing3D"));
				sRstAlgo->m_stWingFunc = m_eFoot_FindOption::Wing3D;
			}
			else if (vecWingStartOption[i] == m_eFoot_FindOption::Slope3D)
			{
				//2. Slope3D
				cv::Mat roImgFootBWImage;
				cv::Mat ImgFootBWImage = (*cvFootImg);
				//WarpAffine(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);		////Foot Img
				WarpAffine_IPPRotate(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);	//Foot Img

				int TailEndPosY = 0, WireStartPosY = 0;
				bPadLength = FindCandidate_TailAndWire_SobelXY(pFoot, fHRoImg, &roImgFootBWImage, &LSt, &LEd, &RSt, &REd, CenterPo, FootRoImgMargin, TailEndPosY, WireStartPosY);
				cstrInspPadLengthLog.Format(_T("[FootAlgo]::Insp_PadLength_StartPoint : Slope3D"));
				sRstAlgo->m_stWingFunc = m_eFoot_FindOption::Slope3D;
			}
			else if (vecWingStartOption[i] == m_eFoot_FindOption::WingGap)
			{
				//3. wingGap
				bPadLength = getFootLengthEndPoint(pFoot, &CutImg, &LSt, &LEd, &RSt, &REd, CenterPo.x - Margin.x, pFoot->m_fArrOptionValue[m_eFoot_Wedge_L][(int)m_eMMD::eMMD_Min]);		//¾cAE wing °￡AC AOºoμμ °A¸® °ªA¸·I °aA¤
				cstrInspPadLengthLog.Format(_T("[FootAlgo]::Insp_PadLength_StartPoint : WingGap"));
				sRstAlgo->m_stWingFunc = m_eFoot_FindOption::WingGap;
			}
			else if (vecWingStartOption[i] == m_eFoot_FindOption::Bonding)
			{
				cv::Mat roImgFootBWImage;
				cv::Mat ImgFootBWImage = (*cvFootImg);	//Foot Img
				//WarpAffine(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);		////Foot Img
				WarpAffine_IPPRotate(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);	//Foot Img

				cv::Mat CutImg2 = roImgFootBWImage(cv::Rect(0, 0, ImgFootBWImage.cols, bottomPo.y));
				bPadLength = Insp_FootPadLength(&CutImg2, &LSt, &LEd, &RSt, &REd, fHRoImg, roImgFootBWImage.rows - bottomPo.y > 1);
				cstrInspPadLengthLog.Format(_T("[FootAlgo]::Insp_PadLength_StartPoint : Bonding"));
				sRstAlgo->m_stWingFunc = m_eFoot_FindOption::Bonding;

			}
			else if (vecWingStartOption[i] == m_eFoot_FindOption::Wing)
			{
				bPadLength = getFootBondingStEndPoint(pFoot, &CutImg, &LSt, &LEd, &RSt, &REd, CenterPo.x - Margin.x, pFoot->m_fArrOptionValue[m_eFoot_Wedge_L][(int)m_eMMD::eMMD_Min]);
				cstrInspPadLengthLog.Format(_T("[FootAlgo]::Insp_PadLength_StartPoint : wing"));
				sRstAlgo->m_stWingFunc = m_eFoot_FindOption::Wing;
			}

			if (bPadLength)
			{
				g_pMPTI->AddLog(cstrInspPadLengthLog);
				break;
			}
		}

		if (bPadLength == false)
		{
			sRstAlgo->m_stWingFunc = -1;
			cstrInspPadLengthLog.Format(_T("[FootAlgo]::Insp_PadLength_StartPoint : Fail"));
			g_pMPTI->AddLog(cstrInspPadLengthLog);
		}

		nLine = __LINE__;
		//Wing (Bonding) Start Point
		vPoSt[(int)m_eFoot::m_eFoot_Bonding1] = LSt;
		vPoSt[(int)m_eFoot::m_eFoot_Bonding2] = RSt;

		std::vector<int> vecWingEndOption;

		if (pFoot->m_nFindOption2 == m_eFoot_FindOption::Wing) //Pad Length   Wing
		{
			vecWingEndOption.push_back((int)m_eFoot_FindOption::Wing);
		}
		else if (pFoot->m_nFindOption2 == m_eFoot_FindOption::WingGap)
		{
			vecWingEndOption.push_back((int)m_eFoot_FindOption::WingGap);
		}
		else if (pFoot->m_nFindOption2 == m_eFoot_FindOption::Bonding) //Pad Length   Foot
		{
			vecWingEndOption.push_back((int)m_eFoot_FindOption::Bonding);
		}
		else if (pFoot->m_nFindOption2 == m_eFoot_FindOption::Slope3D) //Pad Length   3D SobelX, Y 
		{
			vecWingEndOption.push_back((int)m_eFoot_FindOption::Slope3D);
		}
		else if (pFoot->m_nFindOption2 == m_eFoot_FindOption::Wing3D) //Pad Length
		{
			vecWingEndOption.push_back((int)m_eFoot_FindOption::Wing3D);
		}
		else if (pFoot->m_nFindOption2 == m_eFoot_FindOption::MeanSlope) //Pad Length   MeanSlope
		{
			vecWingEndOption.push_back((int)m_eFoot_FindOption::MeanSlope);
		}
		else if (pFoot->m_nFindOption2 == m_eFoot_FindOption::StdDevSlope) //Pad Length   StdDevSlope
		{
			vecWingEndOption.push_back((int)m_eFoot_FindOption::StdDevSlope);
		}
		else
		{
			for (int i = 0; i < pFoot->m_nFindOptionOrder2.size(); i++)
				vecWingEndOption.push_back((int)pFoot->m_nFindOptionOrder2[i]);
			//end Point °E≫c ¿i¼±¼øA§ ¼ø¼­·I ³ª¿­
			//vecWingEndOption.push_back((int)m_eFoot_FindOption::MeanSlope);
			//vecWingEndOption.push_back((int)m_eFoot_FindOption::StdDevSlope);
			//vecWingEndOption.push_back((int)m_eFoot_FindOption::Slope3D);
			//vecWingEndOption.push_back((int)m_eFoot_FindOption::WingGap);
			//vecWingEndOption.push_back((int)m_eFoot_FindOption::Bonding);
			//vecWingEndOption.push_back((int)m_eFoot_FindOption::Wing);
		}

		CString cstrInspPadLength_EndPointLog;
		for (size_t i = 0; i < vecWingEndOption.size(); i++)
		{
			if (vecWingEndOption[i] == m_eFoot_FindOption::MeanSlope)
			{
#pragma region MeanSlope to Insp wing End Point
				cv::Mat FootLeftAreaBW(roImg.rows, roImg.cols, CV_8UC1);
				cv::Mat FootLeftAreafH(fHRoImg.rows, fHRoImg.cols, CV_32FC1);
				cv::Mat FootRightAreaBW(roImg.rows, roImg.cols, CV_8UC1);
				cv::Mat FootRightAreafH(fHRoImg.rows, fHRoImg.cols, CV_32FC1);

				cv::Mat FootBWRemovedFlowDown(roImg.rows, roImg.cols, CV_8UC1);
				CRect rcLeftFootRefROI, rcRightFootRefROI;

				cv::Mat FootBWImg = (*cvFootImg).clone();
				cv::Mat roImgFootBW = FootBWImg;
				//cv::Point RoFootImgMargin = WarpAffine(FootBWImg, roImgFootBW, seta, FootBWImg.size(), CenterPo);	//Foot 3D Img
				cv::Point RoFootImgMargin = WarpAffine_IPPRotate(FootBWImg, roImgFootBW, seta, FootBWImg.size(), CenterPo);	//Foot 2d Img

				cv::Mat WedgeBWImg = (*cvWedgeImg).clone();
				cv::Mat roImgWedgeBW = WedgeBWImg;
				//cv::Point RoWedgeImgMargin = WarpAffine(WedgeBWImg, roImgWedgeBW, seta, WedgeBWImg.size(), CenterPo);	//Foot 3D Img
				cv::Point RoWedgeImgMargin = WarpAffine_IPPRotate(WedgeBWImg, roImgWedgeBW, seta, WedgeBWImg.size(), CenterPo);	//Wedge 2D Img

				bool bExtractLeftAndRightAreaOfFoot = ExtractLeftAndRightAreaOfFoot(pFoot, fHRoImg, roImgFootBW, roImgWedgeBW, CutImg, &FootBWRemovedFlowDown, &FootLeftAreaBW, &FootLeftAreafH, &FootRightAreaBW, &FootRightAreafH, &rcLeftFootRefROI, &rcRightFootRefROI, CenterPo, Margin);

				//Wing Wire
				int nDetectedWireStPosY_v2 = fHRoImg.rows - 1;
				bool bCalc_MeanStdDevGraph_Raw = DetectWireStartPos_UseLeftAndRightAreasOfFootHeight(pFoot, fHRoImg, FootBWRemovedFlowDown, FootLeftAreafH, FootRightAreafH, rcLeftFootRefROI, rcRightFootRefROI, CenterPo, Margin, true, &nDetectedWireStPosY_v2);
				//bool bCalc_MeanStdDevGraph_Raw = DetectWireStartPos_UseLeftAndRightAreasOfFootHeight_2(pFoot, fHRoImg, FootBWRemovedFlowDown, FootLeftAreafH, FootRightAreafH, rcLeftFootRefROI, rcRightFootRefROI, CenterPo, Margin, true, &nDetectedWireStPosY_v2);

				if (bCalc_MeanStdDevGraph_Raw)
				{
					for (int r = 0; r < CutImg.rows; r++)
					{
						UCHAR* Ptr = CutImg.data;
						for (int c = 0; c < CutImg.cols; c++)
						{
							if (Ptr[r * CutImg.step + c] == 255)
							{
								//if (r > nDetectedWireStPosY_v2)
									//Ptr[r * CutImg.step + c] = 0;
							}
						}
					}
				}

				//Wire Blob 
				bool bFindbondingEndPoint = false;
				cv::Point vWireLeftRst;
				cv::Point vWireRightRst;
				vWireLeftRst.x = 0;
				vWireLeftRst.y = 0;
				vWireRightRst.x = 0;
				vWireRightRst.y = 0;

				bool FindLeftBlobObj_Wire = false, FindRightBlobObj_Wire = false;

				POINT LeftPoint_Wire;
				POINT RightPoint_Wire;
				LeftPoint_Wire.x = 0;
				LeftPoint_Wire.y = 0;
				RightPoint_Wire.x = 0;
				RightPoint_Wire.y = 0;

				for (int y = CutImg.rows - 1; y >= 0; y--)
				{
					UCHAR* ptrWireCandiBlobRst = CutImg.ptr(y);

					for (int x = CenterPo.x - RoFootImgMargin.x; x > 0; x--)
					{
						if (ptrWireCandiBlobRst[x] == 255)
						{
							LeftPoint_Wire.x = x;
							LeftPoint_Wire.y = y;
							FindLeftBlobObj_Wire = true;
							break;
						}
					}

					for (int x = CenterPo.x - RoFootImgMargin.x + 1; x < CutImg.cols - 1; x++)
					{
						if (ptrWireCandiBlobRst[x] == 255)
						{
							RightPoint_Wire.x = x;
							RightPoint_Wire.y = y;
							FindRightBlobObj_Wire = true;
							break;
						}
					}

					if (FindLeftBlobObj_Wire == true && FindRightBlobObj_Wire == true)
					{
						vWireLeftRst.x = LeftPoint_Wire.x;
						vWireLeftRst.y = LeftPoint_Wire.y;
						vWireRightRst.x = RightPoint_Wire.x;
						vWireRightRst.y = RightPoint_Wire.y;

						break;
					}
					else
					{


					}

					FindLeftBlobObj_Wire = false;
					FindRightBlobObj_Wire = false;

				}

				if (vWireLeftRst.y != 0 && vWireRightRst.y != 0 && vWireLeftRst.x != 0 && vWireRightRst.x != 0)
				{
					bFindbondingEndPoint = true;
					bPadLength = true;
				}
				else
				{
					bFindbondingEndPoint = false;
					bPadLength = false;
				}

				if (bFindbondingEndPoint)
				{
					LEd = vWireLeftRst;
					REd = vWireRightRst;
				}
#pragma endregion
				cstrInspPadLength_EndPointLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : MeanSlope"));
				sRstAlgo->m_edWingFunc = m_eFoot_FindOption::MeanSlope;
			}
			else if (vecWingEndOption[i] == m_eFoot_FindOption::StdDevSlope)
			{
#pragma region StdDevSlope to Insp wing End Point
				cv::Mat FootLeftAreaBW(roImg.rows, roImg.cols, CV_8UC1);
				cv::Mat FootLeftAreafH(fHRoImg.rows, fHRoImg.cols, CV_32FC1);
				cv::Mat FootRightAreaBW(roImg.rows, roImg.cols, CV_8UC1);
				cv::Mat FootRightAreafH(fHRoImg.rows, fHRoImg.cols, CV_32FC1);

				cv::Mat FootBWRemovedFlowDown(roImg.rows, roImg.cols, CV_8UC1);
				CRect rcLeftFootRefROI, rcRightFootRefROI;

				cv::Mat FootBWImg = (*cvFootImg).clone();
				cv::Mat roImgFootBW = FootBWImg;
				//cv::Point RoFootImgMargin = WarpAffine(FootBWImg, roImgFootBW, seta, FootBWImg.size(), CenterPo);	//Foot 2D Img E¸Au
				cv::Point RoFootImgMargin = WarpAffine_IPPRotate(FootBWImg, roImgFootBW, seta, FootBWImg.size(), CenterPo);	//Foot 2d Img

				cv::Mat WedgeBWImg = (*cvWedgeImg).clone();
				cv::Mat roImgWedgeBW = WedgeBWImg;
				//cv::Point RoWedgeImgMargin = WarpAffine(WedgeBWImg, roImgWedgeBW, seta, WedgeBWImg.size(), CenterPo);	//Wedge 2D Img
				cv::Point RoWedgeImgMargin = WarpAffine_IPPRotate(WedgeBWImg, roImgWedgeBW, seta, WedgeBWImg.size(), CenterPo);	//Wedge 2D Img

				bool bExtractLeftAndRightAreaOfFoot = ExtractLeftAndRightAreaOfFoot(pFoot, fHRoImg, roImgFootBW, roImgWedgeBW, CutImg, &FootBWRemovedFlowDown, &FootLeftAreaBW, &FootLeftAreafH, &FootRightAreaBW, &FootRightAreafH, &rcLeftFootRefROI, &rcRightFootRefROI, CenterPo, Margin);

				//Wing Wire
				bool bDetectedWireStartUseFootHeight_rawGrad = false;

				cv::Mat leftFootDiffVal(FootLeftAreafH.rows, FootLeftAreafH.cols, CV_32FC1);		//Foot Left Image
				cv::Mat rightFootDiffVal(FootLeftAreafH.rows, FootLeftAreafH.cols, CV_32FC1);		//Foot Right Image
				leftFootDiffVal.setTo(0);
				rightFootDiffVal.setTo(0);
				CalcFootHeightGradient_LeftAndRightAreas(pFoot, fHRoImg, FootLeftAreafH, FootRightAreafH, CenterPo, Margin, &leftFootDiffVal, &rightFootDiffVal);

				cv::Mat leftFootDiffVal2(FootLeftAreafH.rows, FootLeftAreafH.cols, CV_32FC1);
				cv::Mat rightFootDiffVal2(FootLeftAreafH.rows, FootLeftAreafH.cols, CV_32FC1);
				leftFootDiffVal2.setTo(0);
				rightFootDiffVal2.setTo(0);
				CalcFootHeightGradient_LeftAndRightAreas(pFoot, fHRoImg, leftFootDiffVal, rightFootDiffVal, CenterPo, Margin, &leftFootDiffVal2, &rightFootDiffVal2);

				int nDetectedWireStPosY = fHRoImg.rows - 1;
				bDetectedWireStartUseFootHeight_rawGrad = DetectWireStartPos_UseLeftAndRightAreasOfFootHeight(pFoot, fHRoImg, FootBWRemovedFlowDown, leftFootDiffVal2, rightFootDiffVal2, rcLeftFootRefROI, rcRightFootRefROI, CenterPo, Margin, false, &nDetectedWireStPosY);

				if (bDetectedWireStartUseFootHeight_rawGrad)
				{
					for (int r = 0; r < CutImg.rows; r++)
					{
						UCHAR* Ptr = CutImg.data;
						for (int c = 0; c < CutImg.cols; c++)
						{
							if (Ptr[r * CutImg.step + c] == 255)
							{
								if (r > nDetectedWireStPosY)
									Ptr[r * CutImg.step + c] = 0;
							}
						}
					}
				}

				//Wire
				bool bFindbondingEndPoint = false;
				cv::Point vWireLeftRst;
				cv::Point vWireRightRst;
				vWireLeftRst.x = 0;
				vWireLeftRst.y = 0;
				vWireRightRst.x = 0;
				vWireRightRst.y = 0;

				bool FindLeftBlobObj_Wire = false, FindRightBlobObj_Wire = false;

				POINT LeftPoint_Wire;
				POINT RightPoint_Wire;
				LeftPoint_Wire.x = 0;
				LeftPoint_Wire.y = 0;
				RightPoint_Wire.x = 0;
				RightPoint_Wire.y = 0;

				for (int y = CutImg.rows - 1; y >= 0; y--)
				{
					UCHAR* ptrWireCandiBlobRst = CutImg.ptr(y);

					for (int x = CenterPo.x - RoFootImgMargin.x; x > 0; x--)
					{
						if (ptrWireCandiBlobRst[x] == 255)
						{
							LeftPoint_Wire.x = x;
							LeftPoint_Wire.y = y;
							FindLeftBlobObj_Wire = true;
							break;
						}
					}

					for (int x = CenterPo.x - RoFootImgMargin.x + 1; x < CutImg.cols - 1; x++)
					{
						if (ptrWireCandiBlobRst[x] == 255)
						{
							RightPoint_Wire.x = x;
							RightPoint_Wire.y = y;
							FindRightBlobObj_Wire = true;
							break;
						}
					}

					if (FindLeftBlobObj_Wire == true && FindRightBlobObj_Wire == true)
					{
						vWireLeftRst.x = LeftPoint_Wire.x;
						vWireLeftRst.y = LeftPoint_Wire.y;
						vWireRightRst.x = RightPoint_Wire.x;
						vWireRightRst.y = RightPoint_Wire.y;

						break;
					}
					else
					{


					}

					FindLeftBlobObj_Wire = false;
					FindRightBlobObj_Wire = false;

				}

				if (vWireLeftRst.y != 0 && vWireRightRst.y != 0 && vWireLeftRst.x != 0 && vWireRightRst.x != 0)
				{
					bFindbondingEndPoint = true;
					bPadLength = true;
				}
				else
				{
					bFindbondingEndPoint = false;
					bPadLength = false;
				}

				if (bFindbondingEndPoint)
				{
					LEd = vWireLeftRst;
					REd = vWireRightRst;
				}

#pragma endregion
				cstrInspPadLength_EndPointLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : StdDevSlope"));
				sRstAlgo->m_edWingFunc = m_eFoot_FindOption::StdDevSlope;
			}
			else if (vecWingEndOption[i] == m_eFoot_FindOption::Slope3D)
			{
				//3. Slope3D
				cv::Mat roImgFootBWImage;
				cv::Mat ImgFootBWImage = (*cvFootImg);
				//WarpAffine(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);		////Foot Img E¸Au
				WarpAffine_IPPRotate(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);	//Wedge 2D Img

				int TailEndPosY = 0, WireStartPosY = 0;
				bPadLength = FindCandidate_TailAndWire_SobelXY(pFoot, fHRoImg, &roImgFootBWImage, &LStTemp, &LEd, &RStTemp, &REd, CenterPo, FootRoImgMargin, TailEndPosY, WireStartPosY);
				cstrInspPadLength_EndPointLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : Slope3D"));
				sRstAlgo->m_edWingFunc = m_eFoot_FindOption::Slope3D;
			}
			else if (vecWingEndOption[i] == m_eFoot_FindOption::WingGap)
			{
				//3. wingGap
				bPadLength = getFootLengthEndPoint(pFoot, &CutImg, &LStTemp, &LEd, &RStTemp, &REd, CenterPo.x - Margin.x, pFoot->m_fArrOptionValue[m_eFoot_Wedge_L][(int)m_eMMD::eMMD_Min]);		//¾cAE wing °￡AC AOºoμμ °A¸® °ªA¸·I °aA¤
				cstrInspPadLength_EndPointLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : WingGap"));
				sRstAlgo->m_edWingFunc = m_eFoot_FindOption::WingGap;
			}
			else if (vecWingEndOption[i] == m_eFoot_FindOption::Bonding)
			{
				cv::Mat roImgFootBWImage;
				cv::Mat ImgFootBWImage = (*cvFootImg);

				//WarpAffine(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);
				WarpAffine_IPPRotate(ImgFootBWImage, roImgFootBWImage, seta, ImgFootBWImage.size(), CenterPo);	//Wedge 2D Img

				cv::Mat CutImg2 = roImgFootBWImage(cv::Rect(0, 0, ImgFootBWImage.cols, bottomPo.y));
				bPadLength = Insp_FootPadLength(&CutImg2, &LStTemp, &LEd, &RStTemp, &REd, fHRoImg, roImgFootBWImage.rows - bottomPo.y > 1);
				cstrInspPadLength_EndPointLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : Bonding"));
				sRstAlgo->m_edWingFunc = m_eFoot_FindOption::Bonding;

			}
			else if (vecWingEndOption[i] == m_eFoot_FindOption::Wing)
			{
				bPadLength = getFootBondingStEndPoint(pFoot, &CutImg, &LStTemp, &LEd, &RStTemp, &REd, CenterPo.x - Margin.x, pFoot->m_fArrOptionValue[m_eFoot_Wedge_L][(int)m_eMMD::eMMD_Min]);
				cstrInspPadLength_EndPointLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : wing"));
				sRstAlgo->m_edWingFunc = m_eFoot_FindOption::Wing;
			}
			else if (vecWingEndOption[i] == m_eFoot_FindOption::Wing3D)
			{
				bPadLength = getFootBondingStEndPoint2(&CutImg, &fHRoImg, &LStTemp, &LEd, &RStTemp, &REd, CenterPo.x - Margin.x, CenterPo.y - Margin.y, 0);
				cstrInspPadLengthLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : Wing3D"));
				sRstAlgo->m_edWingFunc = m_eFoot_FindOption::Wing3D;
			}

			if (bPadLength)
			{
				g_pMPTI->AddLog(cstrInspPadLength_EndPointLog);
				break;
			}
		}

		if (bPadLength == false)
		{
			sRstAlgo->m_edWingFunc = -1;
			cstrInspPadLength_EndPointLog.Format(_T("[FootAlgo]::Insp_PadLength_EndPoint : Fail"));
			g_pMPTI->AddLog(cstrInspPadLength_EndPointLog);
		}

		fPoints.m_pCenter.x = CenterPo.x;
		fPoints.m_pCenter.y = CenterPo.y;

		fPoints.m_dSeta = seta;

#pragma endregion


		nLine = __LINE__;

		if ( LEd.y == (*wingCandiImgSec).rows - 1 || REd.y == (*wingCandiImgSec).rows - 1 ||
			 (LEd.y > (*wingCandiImgSec).rows - 20) || (REd.y > (*wingCandiImgSec).rows - 20) ||
			 (LEd.y > nWingRectMaxY + 10) || (REd.y > nWingRectMaxY + 10)
			)
		{
			int nWingChkRcMinX = CutImg.cols;
			int nWingChkRcMaxX = 0;
			int nWingChkRcMinY = CutImg.rows;
			int nWingChkRcMaxY = 0;
			for (int r = 0; r < (*wingCandiImgSec).rows; r++)
			{
				UCHAR* Ptr = (*wingCandiImgSec).data;
				for (int c = 0; c < (*wingCandiImgSec).cols; c++)
				{
					if (Ptr[r * CutImg.step + c] == 255)
					{
						if (r < nWingChkRcMinY)	nWingChkRcMinY = r;
						if (r > nWingChkRcMaxY)	nWingChkRcMaxY = r;
						if (c < nWingChkRcMinX) nWingChkRcMinX = c;
						if (c > nWingChkRcMaxX) nWingChkRcMaxX = c;
					}
				}
			}
			if(nWingChkRcMaxY != CutImg.rows)
			{
				LEd.y = nWingChkRcMaxY;
				REd.y = nWingChkRcMaxY;
			}
		}
		//Wing (Bonding) End Point
		vPoEd[(int)m_eFoot::m_eFoot_Bonding1] = LEd;
		vPoEd[(int)m_eFoot::m_eFoot_Bonding2] = REd;

		//result¿¹¿UA³¸®
		if (LEd.y <= LSt.y || REd.y < RSt.y)
		{
			cv::Mat Intg, Dev;
			cv::integral(*wingCandiImgSec, Intg, Dev);
			for (int r = 0; r < Intg.rows; r++)
			{
				if (Intg.ptr<int>(r)[Intg.cols - 1] > 0)
				{
					LSt.y = RSt.y = r;
					break;
				}
			}
			for (int r = Intg.rows - 1; r > 1; r--)
			{
				if (Intg.ptr<int>(r)[Intg.cols - 1] - Intg.ptr<int>(r - 1)[Intg.cols - 1] > 0)
				{
					LEd.y = REd.y = r;
					break;
				}
			}
		}

		if (abs(LEd.y - LSt.y) < 50 || abs(REd.y - RSt.y) < 50)
		{
			if (abs(LEd.y - LSt.y) < 50)
				LSt = poInitLSt;

			if (abs(REd.y - RSt.y) < 50)
				RSt = poInitRSt;
		}



		//Foot Center Point Aß¸ø ±¸CØA³A≫ °æ¿i,Wing St/Ed A§A¡ ¹®A| ≫y±aAo ¾Eμμ·I A³¸®
		int nFootCenterXPos = CenterPo.x - Margin.x;
		bool bNeedNewCtrPos = false;

		if (RSt.x - LSt.x < 5 || REd.x - LEd.x < 5)
		{
			int nWingStTmpMin_x = (*wingCandiImgSec).cols * (*wingCandiImgSec).rows, nWingStTmpMax_x = 0;
			int nWingEndTmpMin_x = (*wingCandiImgSec).cols * (*wingCandiImgSec).rows, nWingEndTmpMax_x = 0;

			for (int r = 0; r < (*wingCandiImgSec).rows; r++)
			{
				UCHAR* Ptr = (*wingCandiImgSec).data;
				for (int c = 0; c < (*wingCandiImgSec).cols; c++)
				{
					if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
					{
						if (r == LSt.y)
						{
							if (c < nWingStTmpMin_x) nWingStTmpMin_x = c;
							if (c > nWingStTmpMax_x) nWingStTmpMax_x = c;
						}
						if (r == LEd.y)
						{
							if (c < nWingEndTmpMin_x) nWingEndTmpMin_x = c;
							if (c > nWingEndTmpMax_x) nWingEndTmpMax_x = c;
						}

						if (CenterPo.x - Margin.x == c)
						{
							bNeedNewCtrPos = true;
						}
					}

				}
			}

			if (nWingStTmpMin_x == (*wingCandiImgSec).cols * (*wingCandiImgSec).rows  &&  nWingStTmpMax_x == 0 &&
				nWingEndTmpMin_x == (*wingCandiImgSec).cols * (*wingCandiImgSec).rows  &&  nWingEndTmpMax_x == 0)
			{

			}
			else
			{
				int nTmpCenterXPos = 0;
				if (nWingStTmpMin_x != (*wingCandiImgSec).cols * (*wingCandiImgSec).rows  &&  nWingStTmpMax_x != 0)
				{
					nTmpCenterXPos = (nWingStTmpMin_x + nWingEndTmpMax_x) / 2;
				}

				if (bNeedNewCtrPos && nTmpCenterXPos != 0)
				{
					bool bNotUseCtrXPos = false;

					for (int r = 0; r < (*wingCandiImgSec).rows; r++)
					{
						UCHAR* Ptr = (*wingCandiImgSec).data;
						for (int c = 0; c < (*wingCandiImgSec).cols; c++)
						{
							if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
							{
								if (nTmpCenterXPos == c)
								{
									bNotUseCtrXPos = true;
								}

							}

						}
					}

					if (!bNotUseCtrXPos)
					{
						nFootCenterXPos = nTmpCenterXPos;
						bNeedNewCtrPos = false;
					}
				}
			}
		}


#pragma region wing Point using second wing Candidates

		/*
		//double dArea, dCX, dCY;
		//CRect rcBlob;
		//PIAL::_AlgoBlob algoBlob;
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
		algoBlob.m_nTypeRange2D = 2;
		algoBlob.m_nMinBinary = 0;
		algoBlob.m_nMaxBinary = 100;
		algoBlob.m_bFillHole = true;

		//PIAL::_TotalInspExceptArea stTieAreaNULL;
		TotalInspExceptArea stTieAreaNULL;
		stTieAreaNULL.m_nUsedInspPolygon = 0;
		stTieAreaNULL.m_nUsedMaskingValue = 0;
		stTieAreaNULL.m_nUsedWndPolygon = 0;
		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;

		//cv::Mat BinWingBlobRstimg(roFootBinImg.rows, roFootBinImg.cols, CV_8UC1);
		cv::Mat Img3D_Candidates(roImg.rows, roImg.cols, CV_32FC1);

		int nCntWingBlob = CPInsp::BlobImageStruct(algoBlob, roImg.data, Img3D_Candidates.ptr<float>(0), NULL, roImg.cols, roImg.rows, 100, &dArea, &dCX, &dCY, &rcBlob, (*wingCandiImgSec).data, stTieAreaNULL, true);

// 		PIAL::PI_Buff org2DSrc(roImg);
// 		PIAL::PI_Buff org3DSrc(Img3D_Candidates.ptr<float>(0), Img3D_Candidates.cols, Img3D_Candidates.rows, Img3D_Candidates.cols * 4);
//
// 		PIAL::PI_Buff pTempImgBlob2(roImg.cols, roImg.rows);
// 		///////////////////////////
// 		int nCntBlobWing = PIAL::PAlgo::BlobImageStruct(algoBlob, &org2DSrc, &org3DSrc, NULL, &pTempImgBlob2, roImg.cols, roImg.rows, 100,
// 			&dArea, &dCX, &dCY, &rcBlob, stTieAreaNULL, true);
//
// 		cv::Mat WingCandiRstImg = pTempImgBlob2.Mat();

//		WingCandiRstImg.copyTo(*wingCandiImgSec);

*/
//Wing Start ºIºÐ ¾c AE Blob °￡ Wedge ºIºÐA¸·I Point º?°æ 
		bool bFindbondingStPoint = false;
		cv::Point vTailLeftRst;
		cv::Point vTailRightRst;
		vTailLeftRst.x = 0;
		vTailLeftRst.y = 0;
		vTailRightRst.x = 0;
		vTailRightRst.y = 0;

		bool FindLeftBlobObj_Tail = false, FindRightBlobObj_Tail = false;
		bool FindedLeft_Tail = false, FindedRight_Tail = false;
		POINT LeftPoint_Tail;
		POINT RightPoint_Tail;
		LeftPoint_Tail.x = 0;
		LeftPoint_Tail.y = 0;
		RightPoint_Tail.x = 0;
		RightPoint_Tail.y = 0;

		for (int y = 0; y < (*wingCandiImgSec).rows; y++)
		{
			UCHAR* ptrTailCandiBlobRst = (*wingCandiImgSec).ptr(y);

			for (int x = nFootCenterXPos; x > 0; x--)
			{
				if (ptrTailCandiBlobRst[x] == 255 && !FindedLeft_Tail)
				{
					LeftPoint_Tail.x = x;
					LeftPoint_Tail.y = y;
					FindLeftBlobObj_Tail = true;
					break;
				}
			}

			for (int x = nFootCenterXPos + 1; x < (*wingCandiImgSec).cols - 1; x++)
			{
				if (ptrTailCandiBlobRst[x] == 255 && !FindedRight_Tail)
				{
					RightPoint_Tail.x = x;
					RightPoint_Tail.y = y;
					FindRightBlobObj_Tail = true;
					break;
				}
			}

			if (FindLeftBlobObj_Tail == true || FindRightBlobObj_Tail == true)
			{
				if (FindLeftBlobObj_Tail)
				{
					vTailLeftRst.x = LeftPoint_Tail.x;
					vTailLeftRst.y = LeftPoint_Tail.y;
					FindedLeft_Tail = true;
				}
				if (FindRightBlobObj_Tail)
				{
					vTailRightRst.x = RightPoint_Tail.x;
					vTailRightRst.y = RightPoint_Tail.y;
					FindedRight_Tail = true;
				}
			}
			else
			{


			}

			FindLeftBlobObj_Tail = false;
			FindRightBlobObj_Tail = false;

		}

		if (vTailLeftRst.y != 0 && vTailRightRst.y != 0 && vTailLeftRst.x != 0 && vTailRightRst.x != 0)
		{
			bFindbondingStPoint = true;
		}
		else
			bFindbondingStPoint = false;

		if (bFindbondingStPoint && !bNeedNewCtrPos)
		{
			LSt = vTailLeftRst;
			RSt = vTailRightRst;
		}

		//Wing End ºIºÐ ¾c AE Blob °￡ Wedge ºIºÐA¸·I Point º?°æ 
		bool bFindbondingEndPoint = false;
		cv::Point vWireLeftRst;
		cv::Point vWireRightRst;
		vWireLeftRst.x = 0;
		vWireLeftRst.y = 0;
		vWireRightRst.x = 0;
		vWireRightRst.y = 0;

		bool FindLeftBlobObj_Wire = false, FindRightBlobObj_Wire = false;
		bool FindedLeft = false, FindedRight = false;
		POINT LeftPoint_Wire;
		POINT RightPoint_Wire;
		LeftPoint_Wire.x = 0;
		LeftPoint_Wire.y = 0;
		RightPoint_Wire.x = 0;
		RightPoint_Wire.y = 0;

		for (int y = (*wingCandiImgSec).rows - 1; y >= 0; y--)
		{
			UCHAR* ptrWireCandiBlobRst = (*wingCandiImgSec).ptr(y);

			for (int x = nFootCenterXPos; x > 0; x--)
			{
				if (ptrWireCandiBlobRst[x] == 255 && !FindedLeft)
				{
					LeftPoint_Wire.x = x;
					LeftPoint_Wire.y = y;
					FindLeftBlobObj_Wire = true;
					break;
				}
			}

			for (int x = nFootCenterXPos + 1; x < (*wingCandiImgSec).cols - 1; x++)
			{
				if (ptrWireCandiBlobRst[x] == 255 && !FindedRight)
				{
					RightPoint_Wire.x = x;
					RightPoint_Wire.y = y;
					FindRightBlobObj_Wire = true;
					break;
				}
			}

			if (FindLeftBlobObj_Wire == true || FindRightBlobObj_Wire == true)
			{
				if (FindLeftBlobObj_Wire)
				{
					vWireLeftRst.x = LeftPoint_Wire.x;
					vWireLeftRst.y = LeftPoint_Wire.y;
					FindedLeft = true;
				}
				if (FindRightBlobObj_Wire)
				{
					vWireRightRst.x = RightPoint_Wire.x;
					vWireRightRst.y = RightPoint_Wire.y;
					FindedRight = true;
				}
			}
			else
			{


			}

			FindLeftBlobObj_Wire = false;
			FindRightBlobObj_Wire = false;

		}

		if (vWireLeftRst.y != 0 && vWireRightRst.y != 0 && vWireLeftRst.x != 0 && vWireRightRst.x != 0)
		{
			bFindbondingEndPoint = true;
		}
		else
			bFindbondingEndPoint = false;

		if (bFindbondingEndPoint && !bNeedNewCtrPos)
		{
			LEd = vWireLeftRst;
			REd = vWireRightRst;
		}

		if (abs(RSt.x - LSt.x) < 5)
		{
			int nWingMinX = (*wingCandiImgSec).cols * (*wingCandiImgSec).rows, nWingMaxX = 0;

			for (int r = 0; r < (*wingCandiImgSec).rows; r++)
			{
				UCHAR* Ptr = (*wingCandiImgSec).data;
				for (int c = 0; c < (*wingCandiImgSec).cols; c++)
				{
					if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
					{
						if (r == RSt.y)
						{
							if (c < nWingMinX) nWingMinX = c;
							if (c > nWingMaxX) nWingMaxX = c;
						}

					}
				}
			}

			if (nWingMinX != (*wingCandiImgSec).cols * (*wingCandiImgSec).rows && nWingMaxX != 0)
			{
				if (abs(nWingMaxX - nWingMinX) > 5)
				{
					RSt.x = nWingMaxX;
					LSt.x = nWingMinX;
				}
			}
		}
		if (abs(REd.x - LEd.x) < 5)
		{
			int nWingMinX = (*wingCandiImgSec).cols * (*wingCandiImgSec).rows, nWingMaxX = 0;

			for (int r = 0; r < (*wingCandiImgSec).rows; r++)
			{
				UCHAR* Ptr = (*wingCandiImgSec).data;
				for (int c = 0; c < (*wingCandiImgSec).cols; c++)
				{
					if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
					{
						if (r == LEd.y)
						{
							if (c < nWingMinX) nWingMinX = c;
							if (c > nWingMaxX) nWingMaxX = c;
						}

					}
				}
			}

			if (nWingMinX != (*wingCandiImgSec).cols * (*wingCandiImgSec).rows && nWingMaxX != 0)
			{
				if (abs(nWingMaxX - nWingMinX) > 5)
				{
					REd.x = nWingMaxX;
					LEd.x = nWingMinX;
				}
			}
		}

		int nWingTmpRcMinX = CutImg.cols;
		int nWingTmpRcMaxX = 0;
		int nWingTmpRcMinY = CutImg.rows;
		int nWingTmpRcMaxY = 0;
		for (int r = 0; r < (*wingCandiImgSec).rows; r++)
		{
			UCHAR* Ptr = (*wingCandiImgSec).data;
			for (int c = 0; c < (*wingCandiImgSec).cols; c++)
			{
				if (Ptr[r * CutImg.step + c] == 255)
				{
					if (r < nWingTmpRcMinY)	nWingTmpRcMinY = r;
					if (r > nWingTmpRcMaxY)	nWingTmpRcMaxY = r;
					if (c < nWingTmpRcMinX) nWingTmpRcMinX = c;
					if (c > nWingTmpRcMaxX) nWingTmpRcMaxX = c;
				}
			}
		}
		if( (LEd.y - LSt.y)*m_resolY  < (pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min]) * 1.4 ||
			(REd.y - RSt.y)*m_resolY < (pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min]) * 1.4 )
		{
			LSt.y = nWingTmpRcMinY;
			RSt.y = nWingTmpRcMinY;
		}
		bool bIsDBCFoot = false;
		if (pFoot->m_nUseOption2 & m_eFOOT_Data2::m_eFOOT_Data2_DBC)
			bIsDBCFoot = true;
		if (((LEd.y - LSt.y)*m_resolY < (pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min]) * 1.4 && pFoot->bUseTeachRect) ||
			((REd.y - RSt.y)*m_resolY < (pFoot->m_fArrOptionValue[m_eFoot_Length_1][eMMD_Min]) * 1.4 && pFoot->bUseTeachRect) )
 		{
 			LEd.y = nWingTmpRcMaxY;
 			REd.y = nWingTmpRcMaxY;
 		}
		vPoSt[(int)m_eFoot::m_eFoot_Bonding1] = LSt;
		vPoSt[(int)m_eFoot::m_eFoot_Bonding2] = RSt;
		vPoEd[(int)m_eFoot::m_eFoot_Bonding1] = LEd;
		vPoEd[(int)m_eFoot::m_eFoot_Bonding2] = REd;

		//Wing bonding Point 
		for (int r = 0; r < (*wingCandiImgSec).rows; r++)
		{
			UCHAR* Ptr = (*wingCandiImgSec).data;
			for (int c = 0; c <= CenterPo.x - Margin.x; c++)		//wing
			{
				if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
				{
					if (r < LSt.y)
						Ptr[r * (*wingCandiImgSec).step + c] = 0;

					if (r > LEd.y)
						Ptr[r * (*wingCandiImgSec).step + c] = 0;
				}

			}

			for (int c = CenterPo.x - Margin.x + 1; c < (*wingCandiImgSec).cols; c++)
			{
				if (Ptr[r * (*wingCandiImgSec).step + c] == 255)
				{
					if (r < RSt.y)
						Ptr[r * (*wingCandiImgSec).step + c] = 0;

					if (r > REd.y)
						Ptr[r * (*wingCandiImgSec).step + c] = 0;
				}
			}

		}

#if _DEBUG
#ifdef ALGORITHMTOOL_CPP_USE
		cv::Mat ImgFootForDebug_StEnd(nImageHeight, nImageWidth, CV_8UC1);
		pFoot->GetImageMatrix(&ImgFootForDebug_StEnd, (int)m_eFootBin::m_eFootBin_Wing);

		cv::Mat ImgRoFootForDebug_StEnd(nImageHeight, nImageWidth, CV_8UC1);
		cv::Point MarginDebug_StEnd = WarpAffine(ImgFootForDebug_StEnd, ImgRoFootForDebug_StEnd, seta, ImgFootForDebug_StEnd.size(), CenterPo);		//Wing Bin Img E¸Au

		CString eStr1;
		eStr1.Format(_T("D:\\FootRst\\Debug\\3_wingRstPo_%d.bmp"), nFootAlgoCnt);	//C² ¾E°i¸®Ao °³¼o¸¸A­ 

		ImageWingPointSave_forDebug(eStr1, &ImgRoFootForDebug_StEnd, vPoSt[(int)m_eFoot::m_eFoot_Bonding1], vPoEd[(int)m_eFoot::m_eFoot_Bonding1], sRstAlgo->m_stWingFunc, sRstAlgo->m_edWingFunc, true, pFoot->m_nFindOption, pFoot->m_nFindOption2);

		eStr1.Format(_T("D:\\FootRst\\Debug\\3_wingRstPo_%d.bmp"), nFootAlgoCnt);	//C² ¾E°i¸®Ao °³¼o¸¸A­ 
		ImageWingPointSave_forDebug(eStr1, &ImgRoFootForDebug_StEnd, vPoSt[(int)m_eFoot::m_eFoot_Bonding2], vPoEd[(int)m_eFoot::m_eFoot_Bonding2], sRstAlgo->m_stWingFunc, sRstAlgo->m_edWingFunc, true, pFoot->m_nFindOption, pFoot->m_nFindOption2);
#endif
#endif

		//????? Point ?? ??
		fPoints.m_pWedgeSt.x = LSt.x;
		fPoints.m_pWedgeSt.y = LSt.y;
		fPoints.m_pFootEnd.x = LEd.x;
		fPoints.m_pFootEnd.y = LEd.y;

		cv::Point pSt = LSt;		// WingAC ¿Þ/¿A¸￥ ±æAI Aß, YAaA¸·I ´o ±a AEAC PositionA≫ (pSt, pEd)¿¡ AuAa
		cv::Point pEd = LEd;

		if (LSt.y > RSt.y)
			pSt.y = RSt.y;
		if (LEd.y < REd.y)
			pEd.y = REd.y;

		if (pEd.y - pSt.y < 1)
		{
			pEd.y = Img.rows - 1;
		}

		cv::Point pLeftWingStX = LSt;		// ¾cAE WingAC XAa AuAa
		cv::Point pRightWingStX = RSt;

		if (LSt.x > LEd.x)
			pLeftWingStX = LEd;

		if (RSt.x < REd.x)
			pRightWingStX = REd;

		//sRstAlgo->rePoinSt
		for (int poIdx = 0; poIdx < m_eFoot::m_eFoot_Total; poIdx++)
		{
			sRstAlgo->rePoinSt[poIdx].x = vPoSt[poIdx].x;
			sRstAlgo->rePoinSt[poIdx].y = vPoSt[poIdx].y;

			sRstAlgo->rePoinEd[poIdx].x = vPoEd[poIdx].x;
			sRstAlgo->rePoinEd[poIdx].y = vPoEd[poIdx].y;
		}

		//remove pixel over Wing Ed Point
		int nWingRstMinX = WingCandi_Second.cols;
		int nWingRstMaxX = 0;
		int nWingRstMinY = WingCandi_Second.rows;
		int nWingRstMaxY = 0;

		for (int r = 0; r < WingCandi_Second.rows; r++)
		{
			UCHAR* Ptr = WingCandi_Second.data;
			for (int c = 0; c < WingCandi_Second.cols; c++)
			{
				if (Ptr[r * WingCandi_Second.step + c] == 255)
				{
					if (r < nWingRstMinY)	nWingRstMinY = r;
					if (r > nWingRstMaxY)	nWingRstMaxY = r;

					if (c < nWingRstMinX) nWingRstMinX = c;
					if (c > nWingRstMaxX) nWingRstMaxX = c;

				}
			}
		}

		if (nWingRstMaxY != 0 && nWingRstMaxY > 0)
		{
			for (int y = 0; y < WingCandi_Second.rows; y++)
			{
				UCHAR* ptrFootRst = roImg.ptr(y);

				for (int x = 0; x < WingCandi_Second.cols; x++)
				{
					if (y > nWingRstMaxY)
					{
						ptrFootRst[x] = 0;
					}
				}
			}
		}

		Images.emplace_back(*cvFootImg);
		Images.emplace_back(*cvWedgeImg);
		Images.emplace_back(*cvWing);
		Images.emplace_back(cv3DArea);
		Images.emplace_back(*cvPadImg);
		Images.emplace_back(cvFootBinImg);

	}
	catch (const std::exception&)
	{
		CString msg;
		msg.Format(_T("[FootAlgo]::FootPointByType error Line: %d"), nLine);
		g_pMPTI->AddLog(msg);
	}
	return ret;
}
