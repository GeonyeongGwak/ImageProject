#pragma once
#include "stdafx.h" 
#include "PsrFileManager.h"
#include "InspParamDef.h"
#include "MPTI.h"

// #include "stdafx.h" 
// #include "PsrFileManager.h"
//  #include "../PInspAlgo/PI_Blob.h"
// #include "PI_Void.h"
// #include "PI_Draw.h"
// #include "PI_Geo.h"
//#include "../MPTILib_Algo/MilWrapper.h"
//#include "MPTI.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#define MARGIN_PIXEL 2


	bool CharClipper::searchArea(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue)
	{
		cv::Mat tmp, img, _img, bilblur, medblur, blur, maskimg, BinAndImage;
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		//	int index(0);
		int cnt(1);
		img = src.clone();
		_img = img.clone();
		tmp = cv::Mat(_img.rows, _img.cols, CV_8UC1);
		MatList.clear();


		maskimg = _img.clone();
		tmp = _img.clone();
		//cv::erode(maskimg,maskimg,cv::Mat());
		//cv::dilate(maskimg,maskimg,cv::Mat());

		//for gray
		// 
		// 	// 	cv::bilateralFilter(img,bilblur,-1,45,7);
		// 	// 	cv::imwrite("D:\\testimage\\bilateralFilter.bmp",bilblur);
		// 	// 
		// 	// 	cv::medianBlur(img,medblur,7);
		// 	// 	cv::imwrite("D:\\testimage\\blur.bmp",medblur);
		// 	// 
		// 	//     img = bilblur.clone();
		// 
		// 	cv::equalizeHist(img,blur);
		// #if saveImage
		// 	cv::imwrite("D:\\testimage\\equalizeHist.bmp",blur);
		// #endif
		// 
		// 	cnt = 0;
		// 	tmp.zeros(img.rows,img.cols,CV_8SC1);
		// 	index=img.rows;
		// 
		// 	cv::Scalar mean_1, stddev_1; // LWW test
		// 
		// 	cv::meanStdDev(img, mean_1, stddev_1); // 평균과 표준편차를 계산해줌
		// 
		// 	//double thre_value = mean_1[0]+(0.5*stddev_1[0]);
		// 
		// 	cv::threshold(img,img,threshValue,255,CV_THRESH_BINARY); // 평균값을 이용한 이진화
		// 
		// #if saveImage
		// 	cv::imwrite("D:\\testimage\\threshold.bmp",img);
		// #endif
		// 
		// 	cv::findContours( img, contours, hierarchy, CV_RETR_LIST, cv::CHAIN_APPROX_SIMPLE  ); // 외곽선 추출
		// 	int i(contours.size()-1);
		// 
		// #if saveImage
		// 	cv::imwrite("D:\\testimage\\findContours.bmp",img);
		// #endif
		// 
		// 	for(contours.end();i>=0;i--)
		// 	{
		// 		cv::Rect Rbox = cv::boundingRect(contours[i]); //지정된 사각형을 계산해서 반환.
		// 		double area = cv::contourArea(contours[i]);
		// 
		// 		cv::Scalar color(0);	//black
		// 		cv::Scalar _color(255);	//while
		// 		cv::drawContours(tmp,contours,i,color,1,8,hierarchy);
		// 
		// 		cv::Point point(contours[i][0].x+1,contours[i][0].y+1);
		// 		if(tmp.data[(contours[i][0].y+1)*tmp.step[0]+contours[i][0].x+1] == 0)
		// 		{
		// 			point.y +=1;
		// 			cv::floodFill(tmp,point,_color);
		// 		}
		// 		else if(tmp.data[(contours[i][0].y+1)*tmp.step[0]+contours[i][0].x+1] == 255)
		// 		{
		// 			if(tmp.data[(contours[i][0].y+1)*tmp.step[0]+contours[i][0].x+1] == 0)
		// 			{
		// 				point.y +=1;
		// 				cv::floodFill(tmp,point,205);
		// 			}
		// 			else
		// 				cv::floodFill(tmp,point,205);
		// 		}
		// 		else
		// 		{
		// 			cv::floodFill(tmp,point,_color);
		// 		}
		// #if saveImage
		// 		cv::imwrite("D:\\testimage\\floodFill.bmp",tmp);
		// #endif
		// 	}
		// 	cv::Mat mbackground,mforground;
		// 	cv::Scalar forground,background;
		// 	int forg(0),backg(0),Pixcnt(0);

		// 	maskimg = tmp.clone();
		// 	img = _img.clone();
		// 
		// 	cv::threshold(tmp,maskimg,230,255,CV_THRESH_BINARY); // 평균값을 이용한 이진화
		// 	cv::bitwise_and(maskimg,img,mforground);
		// 
		// 	for(int y=0;y<mforground.rows;y++){
		// 		for(int x=0;x<mforground.cols;x++){
		// 			if((int)mforground.data[y*mforground.cols + x]>1)
		// 			{
		// 				Pixcnt++;
		// 				forg +=(int)mforground.data[y*mforground.cols + x];
		// 			}
		// 		}
		// 	}
		// 
		// 	forg = forg/Pixcnt;
		// 	Pixcnt = 0;
		// 
		// 	cv::bitwise_not(maskimg,maskimg);
		// 	cv::bitwise_and(maskimg,img,mbackground);
		// 
		// 	for(int y=0;y<mbackground.rows;y++){
		// 
		// 		for(int x=0;x<mbackground.cols;x++){
		// 
		// 			if((int)mbackground.data[y*mbackground.cols + x]>1){
		// 				Pixcnt++;
		// 				backg +=(int)mbackground.data[y*mbackground.cols + x];
		// 			}
		// 		}
		// 	}
		// 
		// 	backg = backg/Pixcnt;
		// 
		// 
		// 	cv::Scalar color(backg);	//black
		// 	cv::Scalar _color(forg);	//while
		// 
		// 	cv::bitwise_not(maskimg,maskimg);
		// 	for(int y=0;y<maskimg.rows;y++){
		// 		for(int x=0;x<maskimg.cols;x++){
		// 			if((int)maskimg.data[y*maskimg.cols + x]>1)	tmp.data[y*maskimg.cols + x] = forg;
		// 			else tmp.data[y*maskimg.cols + x] = backg;
		// 		}
		// 	}

#if ImageSave
//	cv::imwrite("D:\\testimage\\font.bmp",maskimg);
#endif

		cv::findContours(maskimg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 외곽선 추출

		int size = contours.size();

		std::vector<cv::Rect> rect;
	/*int* index = new int[size];
	double* indouble = new double[size];
	double* x = new double[size];*/
	int* index = g_pMManager->pem_new<int>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	double* indouble = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	double* x = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		for (int idx = 0; idx < size; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[idx]); //지정된 사각형을 계산해서 반환.
			double area = cv::contourArea(contours[idx]);

			int Recx = Rbox.x;
			int Recy = Rbox.y;
			Rbox.x = Rbox.x - 5 >= 0 ? Rbox.x - 5 : 0;
			Rbox.y = Rbox.y - 5 >= 0 ? Rbox.y - 5 : 0;
			Recx = Recx - Rbox.x;
			Recy = Recy - Rbox.y;
			Rbox.width = Rbox.width + Rbox.x + 10 >= tmp.cols ? tmp.cols - Rbox.x - 1 : Rbox.width + Recx + 5;
			Rbox.height = Rbox.height + Rbox.y + 10 >= tmp.rows ? tmp.rows - Rbox.y - 1 : Rbox.height + Recy + 5;

			index[idx] = idx;
			indouble[idx] = area;
			x[idx] = Rbox.x;
			for (int a = idx; 0 < a; a--)
			{
				if (indouble[a - 1] < indouble[a])
				{
					int ntmp = index[a - 1];
					double dtmp = indouble[a - 1];
					index[a - 1] = index[a];
					indouble[a - 1] = indouble[a];
					index[a] = ntmp;
					indouble[a] = dtmp;

					double xtmp = x[a - 1];
					x[a - 1] = x[a];
					x[a] = xtmp;
				}
			}

		}

		if (size < strLength || strLength == 0) {
		/*delete [] index;
		delete [] indouble;
		delete [] x;*/
		g_pMManager->pem_delete(index, true);
		g_pMManager->pem_delete(indouble, true);
		g_pMManager->pem_delete(x, true);
			return false;
		}

#if ImageSave
		cv::imwrite("D:\\testimage\\font.bmp", maskimg);
#endif
		//2017.06.30 shkim 최대 높이의 50% 미만은 문자에서 제외
		cv::Rect MaxRbox = cv::boundingRect(contours[index[0]]);
		for (int idx = 1; idx < strLength; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[idx]]);
			if (MaxRbox.height*0.5 > Rbox.height) {
			/*delete [] index;
			delete [] indouble;
			delete [] x;*/
			g_pMManager->pem_delete(index, true);
			g_pMManager->pem_delete(indouble, true);
			g_pMManager->pem_delete(x, true);
				return false;
			}
			if (Rbox.height > MaxRbox.height)
				MaxRbox.height = Rbox.height;
		}

		for (int idx = 0; idx < strLength; idx++)
		{
			for (int a = idx; 0 < a; a--)
			{
				if (x[a - 1] > x[a])
				{
					int ntmp = index[a - 1];
					index[a - 1] = index[a];
					index[a] = ntmp;

					double dtmp = indouble[a - 1];
					indouble[a - 1] = indouble[a];
					indouble[a] = dtmp;

					double xtmp = x[a - 1];
					x[a - 1] = x[a];
					x[a] = xtmp;
				}
			}

		}
		cv::bitwise_and(img, BinImage, BinAndImage);
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\maskimg_draw_bf.bmp", BinAndImage);
#endif
		for (int idx = strLength; idx < size; idx++)
		{
			cv::Scalar color(0);
			drawContours(BinAndImage, contours, index[idx], color, cv::FILLED, 1, hierarchy);
		}
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\maskimg_draw.bmp", BinAndImage);
#endif
		int rectL = BinAndImage.cols;
		int rectT = BinAndImage.rows;
		int rectR(0), rectB(0);
		for (int i = 0; i < strLength; i++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[i]]); //지정된 사각형을 계산해서 반환.

			if (Rbox.x < rectL)
				rectL = Rbox.x;
			if (Rbox.y < rectT)
				rectT = Rbox.y;
			if (rectR < Rbox.x + Rbox.width)
				rectR = Rbox.x + Rbox.width;
			if (rectB < Rbox.y + Rbox.height)
				rectB = Rbox.y + Rbox.height;
		}
		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
	// 	if(rectR-rectL>BinAndImage.cols-5 && rectB-rectT>BinAndImage.rows-5)
	// 		return false;
		rectL = rectL - 5 < 0 ? 0 : rectL - 5;
		rectT = rectT - 5 < 0 ? 0 : rectT - 5;
		rectR = rectR + 5 < BinAndImage.cols ? rectR + 5 : BinAndImage.cols;
		rectB = rectB + 5 < BinAndImage.rows ? rectB + 5 : BinAndImage.rows;
		cv::Mat ListImg;
		int backg(0);
		bool bCalc = CalcAvgForBack(BinAndImage, Grayimg, ListImg);
		if (!bCalc)
			return false;
		cv::Rect ListImgRect(rectL, rectT, rectR - rectL, rectB - rectT);
		sClipData->_img = ListImg(ListImgRect).clone();
		cv::Point NCent = cv::Point(ListImgRect.x + ListImgRect.width / 2, ListImgRect.y + ListImgRect.height / 2);
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\sClipData_img.bmp", sClipData->_img);
#endif
		//sClipData._img = CalcAvgForBack(BinAndImage,Grayimg);
	/*sClipData.center_x = new int[strLength];
	sClipData.center_y = new int[strLength];
	sClipData.width = new int[strLength];
	sClipData.height = new int[strLength];*/
		sClipData->nRectLength = strLength;
	sClipData->center_x = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->center_y = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->width = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->height = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
		for (int idx = 0; idx < strLength; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[idx]]); //지정된 사각형을 계산해서 반환.

			sClipData->width[idx] = Rbox.width;
			sClipData->height[idx] = Rbox.height;
			sClipData->center_x[idx] = Rbox.x + Rbox.width / 2 - NCent.x;
			sClipData->center_y[idx] = Rbox.y + Rbox.height / 2 - NCent.y;

			cv::Mat mask = BinAndImage(Rbox);


			cv::Mat temp = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			temp.setTo(0);
			mask.copyTo(temp(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, mask.cols, mask.rows)));
			//		cv::Mat temp = tmp(Rbox);

			cv::Mat graycrop = Grayimg(cv::Rect(Rbox.x, Rbox.y, Rbox.width, Rbox.height));

			cv::Mat gray = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			gray.setTo(cv::Scalar(sClipData->_backVal));
			graycrop.copyTo(gray(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, Rbox.width, Rbox.height)));
#if ImageSave
			cv::imwrite("D:\\testimage\\crop.bmp", temp);
			cv::imwrite("D:\\testimage\\crop_gray.bmp", gray);
#endif
			cnt++;
			MatList.push_back(temp);
			MatOrgImgList.push_back(gray);
		}

		//	std::vector<cv::Mat>::iterator	it	= MatList.begin();

	/*delete [] index;
	delete [] indouble;
	delete [] x;*/
	g_pMManager->pem_delete(index, true);
	g_pMManager->pem_delete(indouble, true);
	g_pMManager->pem_delete(x, true);
		return true;
	}

bool CharClipper::searchArea(int strLength,cv::Mat src,cv::Mat Grayimg,cv::Mat BinImage,int color,int threshValue, RECT* FontRect,Im::PIL_ID* milApp, Im::PIL_ID* milSys)
 	{
 		cv::Mat tmp, img, _img, bilblur, medblur, blur, maskimg;
 		std::vector<std::vector<cv::Point>> contours;
 		std::vector<cv::Vec4i> hierarchy;
 
 		//	int index(0);
 		img = src.clone();
 		_img = img.clone();
 		tmp = cv::Mat(_img.rows, _img.cols, CV_8UC1);
 		MatList.clear();
 
 		//cv::bitwise_and(img,BinImage,BinAndImage);
 		maskimg = _img.clone();
 		tmp = _img.clone();
 
 		int rectL = src.cols;
 		int rectT = src.rows;
 		int rectR(0), rectB(0);
 		for (int i = 0; i < strLength; i++)
 		{
 			if (FontRect[i].left < rectL)
 				rectL = FontRect[i].left;
 			if (FontRect[i].top < rectT)
 				rectT = FontRect[i].top;
 			if (rectR < FontRect[i].right)
 				rectR = FontRect[i].right;
 			if (rectB < FontRect[i].bottom)
 				rectB = FontRect[i].bottom;
 		}
 		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
 	// 	if(rectR-rectL>src.cols-5 && rectB-rectT>src.rows-5)
 	// 		return false;
 		rectL = rectL - 5 < 0 ? 0 : rectL - 5;
 		rectT = rectT - 5 < 0 ? 0 : rectT - 5;
 		rectR = rectR + 5 < src.cols ? rectR + 5 : src.cols;
 		rectB = rectB + 5 < src.rows ? rectB + 5 : src.rows;
 		cv::Mat ListImg;
 		int backg(0);
 		bool bCalc = CalcAvgForBack(src, Grayimg, ListImg, threshValue, backg);
 		if (!bCalc)
 			return false;
 		cv::Rect ListImgRect(rectL, rectT, rectR - rectL, rectB - rectT);
 		sClipData->_img = ListImg(ListImgRect).clone();
 		cv::Point NCent = cv::Point(ListImgRect.x + ListImgRect.width / 2, ListImgRect.y + ListImgRect.height / 2);
 
 #if ImageSave
 		cv::imwrite("D:\\testimage\\POCRFont\\sClipData_img.bmp", sClipData->_img);
 #endif
 		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
 		sClipData->nRectLength = strLength;
	sClipData->center_x = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->center_y = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->width = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->height = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
 
	Im::PIL_ID m_milBlobFeature= M_NULL;
	Im::PIL_ID m_milBlobResult = M_NULL;
	Im::Blob::blobAllocFeatureList(*milSys, &m_milBlobFeature);   //Allocate blob feature list.
	Im::Blob::blobAllocResult(*milSys, &m_milBlobResult);         //Allocate blob result.
 
 		if (backg == 0)
 		{
 			for (int idx = 0; idx < strLength; idx++)
 			{
 				int x = FontRect[idx].left - MARGIN_PIXEL;
 				int y = FontRect[idx].top - MARGIN_PIXEL;
 				if (x < 0)
 					x = 0;
 				if (y < 0)
 					y = 0;
 				int w = FontRect[idx].right - x + MARGIN_PIXEL;
 				int h = FontRect[idx].bottom - y + MARGIN_PIXEL;
 				if (w + x > _img.cols)
 					w = _img.cols - x - 1;
 				if (h + y > _img.rows)
 					h = _img.rows - y - 1;
 				cv::Rect Rbox = cv::Rect(x, y, w, h); //지정된 사각형을 계산해서 반환.
 
 				sClipData->width[idx] = Rbox.width;
 				sClipData->height[idx] = Rbox.height;
 				sClipData->center_x[idx] = Rbox.x + Rbox.width / 2 - NCent.x;
 				sClipData->center_y[idx] = Rbox.y + Rbox.height / 2 - NCent.y;
 
 				cv::Mat mask = img(Rbox);
 				cv::Mat BinClip = _img(Rbox).clone();
 
			cv::Mat* imgPtr;
			imgPtr = &BinClip;
			Im::PIL_ID milBinSrc = (Im::PIL_ID)imgPtr;
			Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
			Im::Blob::blobCalculate(milBinSrc, M_NULL, m_milBlobFeature, m_milBlobResult);
 				//PI_Blob::CalcBlob(milBinSrc, M_NULL, m_milBlobFeature, m_milBlobResult);
			MIL_INT nCntBlob = 0;
			Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
 				double * area = NULL;
 				if (nCntBlob > 0)
 				{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &area, nCntBlob);
				Im::Blob::blobGetResult(m_milBlobResult,M_AREA,area);
 					double dMinArea = area[0] * 0.2;
 					if (dMinArea > 0)
					Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS , dMinArea, M_NULL);
 
				Im::Blob::blobFill(m_milBlobResult, milBinSrc, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
				Im::Blob::blobFill(m_milBlobResult, milBinSrc, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw
 
				g_pMManager->pem_delete(area, true);
 				}
 				cv::bitwise_and(mask, BinClip, mask);
 
 				cv::Mat graycrop = Grayimg(cv::Rect(Rbox.x, Rbox.y, Rbox.width, Rbox.height));
 #if ImageSave
 				cv::imwrite("D:\\testimage\\crop.bmp", mask);
 				cv::imwrite("D:\\testimage\\crop_gray.bmp", graycrop);
 #endif
 				MatList.push_back(mask);
 				MatOrgImgList.push_back(graycrop);
 			}
 		}
 		else
 		{
 			for (int idx = 0; idx < strLength; idx++)
 			{
 				int x = FontRect[idx].left - MARGIN_PIXEL;
 				int y = FontRect[idx].top - MARGIN_PIXEL;
 				if (x < 0)
 					x = 0;
 				if (y < 0)
 					y = 0;
 				int w = FontRect[idx].right - x + MARGIN_PIXEL;
 				int h = FontRect[idx].bottom - y + MARGIN_PIXEL;
 				if (w + x > _img.cols)
 					w = _img.cols - x - 1;
 				if (h + y > _img.rows)
 					h = _img.rows - y - 1;
 				cv::Rect Rbox = cv::Rect(x, y, w, h); //지정된 사각형을 계산해서 반환.
 
 				sClipData->width[idx] = Rbox.width;
 				sClipData->height[idx] = Rbox.height;
 				sClipData->center_x[idx] = Rbox.x + Rbox.width / 2 - NCent.x;
 				sClipData->center_y[idx] = Rbox.y + Rbox.height / 2 - NCent.y;
 
 				cv::Mat mask = img(Rbox);
 				cv::Mat BinClip = _img(Rbox).clone();
 
			cv::Mat* imgPtr;
			imgPtr = &BinClip;
			Im::PIL_ID milBinSrc = (Im::PIL_ID)imgPtr;
			Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
			Im::Blob::blobCalculate(milBinSrc, M_NULL, m_milBlobFeature, m_milBlobResult);
			MIL_INT nCntBlob = 0;
			Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
 				double * area = NULL;
 				if (nCntBlob > 0)
 				{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &area, nCntBlob);
				Im::Blob::blobGetResult(m_milBlobResult,M_AREA,area);
				double dMinArea = area[0]*0.4;
 					if (dMinArea > 0)
					Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS , dMinArea, M_NULL);
 
				Im::Blob::blobFill(m_milBlobResult, milBinSrc, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
				Im::Blob::blobFill(m_milBlobResult, milBinSrc, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw

				g_pMManager->pem_delete(area, true);
 				}
 				cv::bitwise_and(mask, BinClip, mask);
 
 
 				cv::Mat temp = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
 				temp.setTo(0);
 				mask.copyTo(temp(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, mask.cols, mask.rows)));
 				//		cv::Mat temp = tmp(Rbox);
 
 				cv::Mat graycrop = Grayimg(cv::Rect(Rbox.x, Rbox.y, Rbox.width, Rbox.height));
 
 				cv::Mat gray = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
 				gray.setTo(cv::Scalar(sClipData->_backVal));
 				graycrop.copyTo(gray(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, Rbox.width, Rbox.height)));
 #if _DEBUG
 
 				cv::imwrite("D:\\testimage\\crop.bmp", temp);
 				cv::imwrite("D:\\testimage\\crop_gray.bmp", gray);
 #endif
 				MatList.push_back(temp);
 				MatOrgImgList.push_back(gray);
 			}
 		}
 
 		//	std::vector<cv::Mat>::iterator	it	= MatList.begin();
	if (m_milBlobFeature != M_NULL)
	{
		Im::Blob::blobFree(m_milBlobFeature);
		m_milBlobFeature = M_NULL;
	}
	if (m_milBlobResult != M_NULL)
 		{
		Im::Blob::blobFree(m_milBlobResult);
		m_milBlobResult = M_NULL;
 		}
 		return true;
 	}
 
 	bool CharClipper::searchArea_NG(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, cv::Mat Model, RstAlgoPOCR result)
 	{
 		cv::Mat tmp, img, _img, bilblur, medblur, blur, maskimg, BinAndImage, Model2;
 		std::vector<std::vector<cv::Point>> contours, contours_Model;
 		std::vector<cv::Vec4i> hierarchy, hierarchy_Model;
 
 		//	int index(0);
 		int cnt(1);
 		int _top = src.rows, _left = src.cols, _bot(0), _rig(0);
 		int blobCnt(0);
 		img = src.clone();
 		_img = img.clone();
 		tmp = cv::Mat(_img.rows, _img.cols, CV_8UC1);
 		MatList.clear();
 
 
 		maskimg = _img.clone();
 		tmp = _img.clone();
 
 		cv::threshold(Model, Model2, 0, 255, cv::THRESH_OTSU);
 
 		cv::findContours(maskimg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 외곽선 추출
 		cv::findContours(Model2, contours_Model, hierarchy_Model, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 외곽선 추출
 
 		int size = contours.size();
 
 		std::vector<cv::Rect> rect, rstRect;
	/*int* index = new int[size];
	double* indouble = new double[size];
	double* x = new double[size];*/
	int* index = g_pMManager->pem_new<int>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	double* indouble = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	double* x = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
 
 		for (int i = 0; i < result.m_nCharCount; i++)
 		{
 			cv::Rect re((result.charX[i] - result.charWidth[i] / 2 - (result.ModelX - result.ModelWidth / 2)), (result.charY[i] - result.charHeight[i] / 2 - (result.ModelY - result.ModelHeight / 2)), result.charWidth[i], result.charHeight[i]);
 			rstRect.push_back(re);
 		}
	/*sClipData.center_x = new int[strLength];
	sClipData.center_y = new int[strLength];
	sClipData.width = new int[strLength];
	sClipData.height = new int[strLength];*/
 		sClipData->nRectLength = strLength;
	sClipData->center_x = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->center_y = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->width = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->height = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
 		for (int idx = 0; idx < result.m_nCharCount; idx++)
 		{
 			//cv::Rect Rbox = cv::boundingRect(contours[index[idx]]); //지정된 사각형을 계산해서 반환.
 			cv::Rect Rbox = rstRect[idx]; //지정된 사각형을 계산해서 반환.
 
 			sClipData->width[idx] = Rbox.width;
 			sClipData->height[idx] = Rbox.height;
 			sClipData->center_x[idx] = Rbox.x + Rbox.width / 2 - src.cols / 2;
 			sClipData->center_y[idx] = Rbox.y + Rbox.height / 2 - src.rows / 2;
 
 			cv::Mat mask = BinAndImage(Rbox);
 
 			cv::Mat temp = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
 			temp.setTo(0);
 			mask.copyTo(temp(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, mask.cols, mask.rows)));
 			//		cv::Mat temp = tmp(Rbox);
 
 			cv::Mat graycrop = Grayimg(cv::Rect(Rbox.x, Rbox.y, Rbox.width, Rbox.height));
 
 			cv::Mat gray = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
 			gray.setTo(cv::Scalar(sClipData->_backVal));
 			graycrop.copyTo(gray(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, Rbox.width, Rbox.height)));
 #if ImageSave
 			cv::imwrite("D:\\testimage\\crop.bmp", temp);
 			cv::imwrite("D:\\testimage\\crop_gray.bmp", gray);
 #endif
 			cnt++;
 			MatList.push_back(temp);
 			MatOrgImgList.push_back(gray);
 
 			if (Rbox.y < _top)
 				_top = Rbox.y;
 			if (Rbox.x < _left)
 				_left = Rbox.x;
 
 			if (_bot < Rbox.height + Rbox.y)
 				_bot = Rbox.height + Rbox.y;
 			if (_rig < Rbox.width + Rbox.x)
 				_rig = Rbox.width + Rbox.x;
 		}
 		cv::Rect re(_left, _top, _rig - _left, _bot - _top);
 		_img = img(re);
 		//	std::vector<cv::Mat>::iterator	it	= MatList.begin();
 
	/*delete [] index;
	delete [] indouble;
	delete [] x;*/
	g_pMManager->pem_delete(index, true);
	g_pMManager->pem_delete(indouble, true);
	g_pMManager->pem_delete(x, true);
 		return true;
 	}

	bool CharClipper::searchArea_OrgImage(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue, std::vector<std::vector<cv::Point2f>> vPoint)
	{
		cv::Mat tmp, img, _img, bilblur, medblur, blur, maskimg, BinAndImage;
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		bool bNotFound = false;
		//	int index(0);
		int cnt(1);
		img = src.clone();
		_img = img.clone();
		tmp = cv::Mat(_img.rows, _img.cols, CV_8UC1);
		MatList.clear();

		std::vector<int> vDist;
		std::vector<int> vFontWidth;
		std::vector<cv::Rect> FontRect;
		std::vector<cv::Rect> FontSearchRect;
		vDist.clear();
		vFontWidth.clear();
		FontRect.clear();
		FontSearchRect.clear();
		vDist.push_back(_img.cols);
		if (vPoint.size() > 0)
		{

			for (int i = 0; i < vPoint[0].size(); i++)
			{
				for (int j = i + 1; j < vPoint[0].size(); j++)
				{
					if (vPoint[0][i].x > vPoint[0][j].x)
					{
						cv::Point2f tmpPo = vPoint[0][i];
						vPoint[0][i] = vPoint[0][j];
						vPoint[0][j] = tmpPo;
					}
				}
			}
		}

		for (int i = 0; i < vPoint.size(); i++)
		{
			int nvPointSize = vPoint[i].size() - 1;
			for (int j = 0; j < nvPointSize; j++)
			{
				int jPl = j + 1;
				int Width = vPoint[i][jPl].x - vPoint[i][j].x;
				vDist.push_back(Width);

			}
		}
		vDist.push_back(_img.cols);

		for (int i = 0; i < vDist.size() - 1; i++)
		{
			int nMin = vDist[i] < vDist[i + 1] ? vDist[i] : vDist[i + 1];
			vFontWidth.push_back(nMin);
		}

		if (vPoint.size() < 1)
		{
			vFontWidth.clear();
			vFontWidth.push_back(_img.cols - 2);
			std::vector<cv::Point2f> vPo;
			vPo.push_back(cv::Point2f(_img.cols / 2 - 0.5, _img.rows / 2 - 0.5));
			vPoint.push_back(vPo);
			bNotFound = true;
		}
		else if (vPoint[0].size() < 1)
		{
			vFontWidth.clear();
			vFontWidth.push_back(_img.cols - 2);
			vPoint[0].push_back(cv::Point2f(_img.cols / 2 - 0.5, _img.rows / 2 - 0.5));
			bNotFound = true;
		}

		for (int i = 0; i < vFontWidth.size(); i++)
		{
			cv::Rect tmpRect(vPoint[0][i].x - (vFontWidth[i] / 2), 2, vFontWidth[i], _img.rows - 2);

			if (tmpRect.x < 0)
			{
				int nSubW = tmpRect.width - tmpRect.x;
				tmpRect.width = nSubW;
				tmpRect.x = 0;
			}
			if ((tmpRect.width + tmpRect.x) > _img.cols - 1)
			{
				int nSubW = (tmpRect.width + tmpRect.x) - (_img.cols - 1);
				tmpRect.width = tmpRect.width - nSubW;
			}

			if (tmpRect.y < 0)
			{
				int nSubH = tmpRect.height - tmpRect.y;
				tmpRect.height = nSubH;
				tmpRect.y = 0;
			}
			if ((tmpRect.height + tmpRect.y) > _img.rows - 1)
			{
				int nSubH = (tmpRect.height + tmpRect.y) - (_img.rows - 1);
				tmpRect.height = tmpRect.height - nSubH;
			}

			FontRect.push_back(tmpRect);
			int x = i < 1 ? 0 : vPoint[0][i - 1].x;
			int w = i + 1 < vPoint[0].size() ? vPoint[0][i + 1].x - x : _img.cols - 1;
			cv::Rect tmpSearchRect(x, 2, w, _img.rows - 2);

			if (tmpSearchRect.x < 0)
			{
				int nSubW = tmpSearchRect.width - tmpSearchRect.x;
				tmpSearchRect.width = nSubW;
				tmpSearchRect.x = 0;
			}
			if ((tmpSearchRect.width + tmpSearchRect.x) > _img.cols - 1)
			{
				int nSubW = (tmpSearchRect.width + tmpSearchRect.x) - (_img.cols - 1);
				tmpSearchRect.width = tmpSearchRect.width - nSubW;
			}

			if (tmpSearchRect.y < 0)
			{
				int nSubH = tmpSearchRect.height - tmpSearchRect.y;
				tmpSearchRect.height = nSubH;
				tmpSearchRect.y = 0;
			}
			if ((tmpSearchRect.height + tmpSearchRect.y) > _img.rows - 1)
			{
				int nSubH = (tmpSearchRect.height + tmpSearchRect.y) - (_img.rows - 1);
				tmpSearchRect.height = tmpSearchRect.height - nSubH;
			}
			FontSearchRect.push_back(tmpSearchRect);
			//if (FontRect.size() >= strLength)
			//	break;
		}

		std::vector<cv::Rect> rect;
		if (bNotFound)
		{
			rect.push_back(FontRect[0]);
		}
		else
		{
			for (int i = 0; i < FontSearchRect.size(); i++)
			{
				maskimg = _img(FontSearchRect[i]).clone();

				cv::findContours(maskimg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 외곽선 추출
				int size = contours.size();
				if (size < 1)
				{
					rect.push_back(FontRect[i]);
					continue;
				}
				int CenterX(0), CenterY(0);
				CenterX = vPoint[0][i].x - FontSearchRect[i].x;// (FontRect[i].width / 2);
				CenterY = vPoint[0][i].y - FontSearchRect[i].y;// (FontRect[i].height / 2);
				cv::Rect MaxRbox(0, 0, FontRect[i].width, FontRect[i].height);
				double MaxA = 0;
				for (int idx = 0; idx < size; idx++)
				{
					cv::Rect Rbox = cv::boundingRect(contours[idx]); //지정된 사각형을 계산해서 반환.
					double area = cv::contourArea(contours[idx]);
					if (Rbox.x >= CenterX || Rbox.x + Rbox.width <= CenterX
						|| Rbox.y >= CenterY || Rbox.y + Rbox.height <= CenterY)
						continue;
					if (MaxA < area)
					{
						MaxRbox = Rbox;
						MaxA = area;
					}
				}
				MaxRbox.x += FontSearchRect[i].x;
				MaxRbox.y += FontSearchRect[i].y;
				rect.push_back(MaxRbox);
			}
		}

		cv::bitwise_and(img, BinImage, BinAndImage);

		int rectL = BinAndImage.cols;
		int rectT = BinAndImage.rows;
		int rectR(0), rectB(0);
		cv::Rect MaxRbox(0, 0, 0, 0);
		for (int i = 0; i < rect.size(); i++)
		{
			cv::Rect Rbox = rect[i]; //지정된 사각형을 계산해서 반환.

			if (Rbox.x < rectL)
				rectL = Rbox.x;
			if (Rbox.y < rectT)
				rectT = Rbox.y;
			if (rectR < Rbox.x + Rbox.width)
				rectR = Rbox.x + Rbox.width;
			if (rectB < Rbox.y + Rbox.height)
				rectB = Rbox.y + Rbox.height;
			if (MaxRbox.height < Rbox.height)
			{
				MaxRbox.y = Rbox.y;
				MaxRbox.height = Rbox.height;
			}
		}
		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
		int nOffSet = 5;
		if (rectR - rectL > BinAndImage.cols - nOffSet && rectB - rectT > BinAndImage.rows - nOffSet)
			nOffSet = (BinAndImage.cols - (rectR - rectL)) / 2;
		if (nOffSet < 0)
			return false;
		if (rectR - rectL > BinAndImage.cols - nOffSet && rectB - rectT > BinAndImage.rows - nOffSet)
			return false;
		rectL = rectL - nOffSet < 0 ? 0 : rectL - nOffSet;
		rectT = rectT - nOffSet < 0 ? 0 : rectT - nOffSet;
		rectR = rectR + nOffSet < BinAndImage.cols ? rectR + nOffSet : BinAndImage.cols;
		rectB = rectB + nOffSet < BinAndImage.rows ? rectB + nOffSet : BinAndImage.rows;
		cv::Mat ListImg;
		int backg(0);
		bool bCalc = CalcAvgForBack(BinAndImage, Grayimg, ListImg, threshValue, backg);
		if (!bCalc)
			return false;
		cv::Rect ListImgRect(rectL, rectT, rectR - rectL, rectB - rectT);
		sClipData->_img = ListImg(ListImgRect).clone();
		cv::Point NCent = cv::Point(src.cols / 2, src.rows / 2);
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\sClipData_img.bmp", sClipData->_img);
#endif
		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
		sClipData->nRectLength = rect.size();
	sClipData->center_x = g_pMManager->pem_new<int>(true, rect.size(), (PCHAR)__FUNCTION__, __LINE__);
	sClipData->center_y = g_pMManager->pem_new<int>(true, rect.size(), (PCHAR)__FUNCTION__, __LINE__);
	sClipData->width = g_pMManager->pem_new<int>(true, rect.size(), (PCHAR)__FUNCTION__, __LINE__);
	sClipData->height = g_pMManager->pem_new<int>(true, rect.size(), (PCHAR)__FUNCTION__, __LINE__);
		for (int idx = 0; idx < rect.size(); idx++)
		{
			cv::Rect Rbox = rect[idx]; //지정된 사각형을 계산해서 반환.
			if (Rbox.height < MaxRbox.height*0.8)
			{
				Rbox.y = MaxRbox.y;
				Rbox.height = MaxRbox.height;
			}
			sClipData->width[idx] = Rbox.width;
			sClipData->height[idx] = Rbox.height;
			sClipData->center_x[idx] = Rbox.x + Rbox.width / 2 - NCent.x;
			sClipData->center_y[idx] = Rbox.y + Rbox.height / 2 - NCent.y;

			cv::Mat mask = BinAndImage(Rbox);


			cv::Mat temp = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			temp.setTo(0);
			mask.copyTo(temp(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, mask.cols, mask.rows)));
			//		cv::Mat temp = tmp(Rbox);

			cv::Mat graycrop = Grayimg(cv::Rect(Rbox.x, Rbox.y, Rbox.width, Rbox.height));

			cv::Mat gray = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			gray.setTo(cv::Scalar(sClipData->_backVal));
			graycrop.copyTo(gray(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, Rbox.width, Rbox.height)));
#if ImageSave
			cv::imwrite("D:\\testimage\\crop.bmp", temp);
			cv::imwrite("D:\\testimage\\crop_gray.bmp", gray);
#endif
			cnt++;
			MatList.push_back(temp);
			MatOrgImgList.push_back(gray);
		}

		return true;
	}
	bool CharClipper::searchArea_OrgImage_Old(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue, std::vector<std::vector<cv::Point2f>> vPoint)
	{
		cv::Mat tmp, img, _img, bilblur, medblur, blur, maskimg, BinAndImage;
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		//	int index(0);
		int cnt(1);
		img = src.clone();
		_img = img.clone();
		tmp = cv::Mat(_img.rows, _img.cols, CV_8UC1);
		MatList.clear();


		maskimg = _img.clone();
		tmp = _img.clone();

		cv::findContours(maskimg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 외곽선 추출

		int size = contours.size();
		if (size < 1)
			return false;

		std::vector<cv::Rect> rect;
	int* index = g_pMManager->pem_new<int>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	double* indouble = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	double* x = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		for (int idx = 0; idx < size; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[idx]); //지정된 사각형을 계산해서 반환.
			double area = cv::contourArea(contours[idx]);

			int Recx = Rbox.x;
			int Recy = Rbox.y;
			Rbox.x = Rbox.x - 5 >= 0 ? Rbox.x - 5 : 0;
			Rbox.y = Rbox.y - 5 >= 0 ? Rbox.y - 5 : 0;
			Recx = Recx - Rbox.x;
			Recy = Recy - Rbox.y;
			Rbox.width = Rbox.width + Rbox.x + 10 >= tmp.cols ? tmp.cols - Rbox.x - 1 : Rbox.width + Recx + 5;
			Rbox.height = Rbox.height + Rbox.y + 10 >= tmp.rows ? tmp.rows - Rbox.y - 1 : Rbox.height + Recy + 5;

			bool in = vPoint.size() > 0 ? false : true;

			for (int i1 = 0; i1 < vPoint.size(); i1++)
			{
				for (int i2 = 0; i2 < vPoint[i1].size(); i2++)
				{
					if (vPoint[i1][i2].x >= Rbox.x && vPoint[i1][i2].x <= Rbox.x + Rbox.width
						&& vPoint[i1][i2].y >= Rbox.y && vPoint[i1][i2].y <= Rbox.y + Rbox.height)
					{
						in = true;
						break;
					}
				}
				if (in)
					break;
			}
			if (in)
			{
				index[idx] = idx;
				indouble[idx] = area;
				x[idx] = Rbox.x;
			}
			else if (strLength == 0)
			{
				index[idx] = idx;
				indouble[idx] = area > (Grayimg.rows*1.7) ? area : (Grayimg.rows*1.7);
				x[idx] = Rbox.x;
			}
			else
			{
				index[idx] = idx;
				indouble[idx] = 0;
				x[idx] = 0;
				continue;
			}
			for (int a = idx; 0 < a; a--)
			{
				if (indouble[a - 1] < indouble[a])
				{
					int ntmp = index[a - 1];
					double dtmp = indouble[a - 1];
					index[a - 1] = index[a];
					indouble[a - 1] = indouble[a];
					index[a] = ntmp;
					indouble[a] = dtmp;

					double xtmp = x[a - 1];
					x[a - 1] = x[a];
					x[a] = xtmp;
				}
			}

		}

		if (size < strLength) {
		g_pMManager->pem_delete(index, true);
		g_pMManager->pem_delete(indouble, true);
		g_pMManager->pem_delete(x,true);
			return false;
		}

#if ImageSave
		cv::imwrite("D:\\testimage\\font.bmp", maskimg);
#endif
		int nStrL = strLength;
		//2017.06.30 shkim 최대 높이의 50% 미만은 문자에서 제외
		cv::Rect MaxRbox = cv::boundingRect(contours[index[0]]);
		for (int idx = 1; idx < size; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[idx]]);
			if (MaxRbox.height*0.5 > Rbox.height)
			{
				// 			delete [] index;
				// 			delete [] indouble;
				// 			delete [] x;
				// 			return false;
				nStrL = idx;
				break;
			}
			if (Rbox.height > MaxRbox.height)
				MaxRbox.height = Rbox.height;
		}
		if (strLength > nStrL || strLength == 0)
			strLength = nStrL;
		if (strLength == 0)
		{
		g_pMManager->pem_delete(index, true);
		g_pMManager->pem_delete(indouble, true);
		g_pMManager->pem_delete(x, true);
			return false;
		}

		for (int idx = 0; idx < strLength; idx++)
		{
			for (int a = idx; 0 < a; a--)
			{
				if (x[a - 1] > x[a])
				{
					int ntmp = index[a - 1];
					index[a - 1] = index[a];
					index[a] = ntmp;

					double dtmp = indouble[a - 1];
					indouble[a - 1] = indouble[a];
					indouble[a] = dtmp;

					double xtmp = x[a - 1];
					x[a - 1] = x[a];
					x[a] = xtmp;
				}
			}

		}
		cv::bitwise_and(img, BinImage, BinAndImage);
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\maskimg_draw_bf.bmp", BinAndImage);
#endif
		for (int idx = strLength; idx < size; idx++)
		{
			cv::Scalar color(0);
			drawContours(BinAndImage, contours, index[idx], color, cv::FILLED, 1, hierarchy);
		}
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\maskimg_draw.bmp", BinAndImage);
#endif
		int rectL = BinAndImage.cols;
		int rectT = BinAndImage.rows;
		int rectR(0), rectB(0);
		for (int i = 0; i < strLength; i++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[i]]); //지정된 사각형을 계산해서 반환.

			if (Rbox.x < rectL)
				rectL = Rbox.x;
			if (Rbox.y < rectT)
				rectT = Rbox.y;
			if (rectR < Rbox.x + Rbox.width)
				rectR = Rbox.x + Rbox.width;
			if (rectB < Rbox.y + Rbox.height)
				rectB = Rbox.y + Rbox.height;
		}
		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
		if (rectR - rectL > BinAndImage.cols - 5 && rectB - rectT > BinAndImage.rows - 5) {
		g_pMManager->pem_delete(index, true);
		g_pMManager->pem_delete(indouble, true);
		g_pMManager->pem_delete(x, true);
			return false;
		}
		rectL = rectL - 5 < 0 ? 0 : rectL - 5;
		rectT = rectT - 5 < 0 ? 0 : rectT - 5;
		rectR = rectR + 5 < BinAndImage.cols ? rectR + 5 : BinAndImage.cols;
		rectB = rectB + 5 < BinAndImage.rows ? rectB + 5 : BinAndImage.rows;
		cv::Mat ListImg;
		int backg(0);
		bool bCalc = CalcAvgForBack(BinAndImage, Grayimg, ListImg, threshValue, backg);
		if (!bCalc) {
		g_pMManager->pem_delete(index, true);
		g_pMManager->pem_delete(indouble, true);
		g_pMManager->pem_delete(x, true);
			return false;
		}
		cv::Rect ListImgRect(rectL, rectT, rectR - rectL, rectB - rectT);
		sClipData->_img = ListImg(ListImgRect).clone();
		cv::Point NCent = cv::Point(src.cols / 2, src.rows / 2);
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\sClipData_img.bmp", sClipData->_img);
#endif
		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
		sClipData->nRectLength = strLength;
	sClipData->center_x = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->center_y = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->width = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
	sClipData->height = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);

		for (int idx = 0; idx < strLength; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[idx]]); //지정된 사각형을 계산해서 반환.

			sClipData->width[idx] = Rbox.width;
			sClipData->height[idx] = Rbox.height;
			sClipData->center_x[idx] = Rbox.x + Rbox.width / 2 - NCent.x;
			sClipData->center_y[idx] = Rbox.y + Rbox.height / 2 - NCent.y;

			cv::Mat mask = BinAndImage(Rbox);


			cv::Mat temp = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			temp.setTo(0);
			mask.copyTo(temp(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, mask.cols, mask.rows)));
			//		cv::Mat temp = tmp(Rbox);

			cv::Mat graycrop = Grayimg(cv::Rect(Rbox.x, Rbox.y, Rbox.width, Rbox.height));

			cv::Mat gray = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			gray.setTo(cv::Scalar(sClipData->_backVal));
			graycrop.copyTo(gray(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, Rbox.width, Rbox.height)));
#if ImageSave
			cv::imwrite("D:\\testimage\\crop.bmp", temp);
			cv::imwrite("D:\\testimage\\crop_gray.bmp", gray);
#endif
			cnt++;
			MatList.push_back(temp);
			MatOrgImgList.push_back(gray);
		}

		//	std::vector<cv::Mat>::iterator	it	= MatList.begin();

	g_pMManager->pem_delete(index, true);
	g_pMManager->pem_delete(indouble, true);
	g_pMManager->pem_delete(x, true);

		return true;
	}

	bool CharClipper::CalcAvgForBack(cv::Mat bin, cv::Mat Gray, cv::Mat& tmp)
	{
		cv::Mat maskimg, img;
		cv::Mat mbackground, mforground;
		cv::Scalar forground, background;
		int forg(0), backg(0), Pixcnt(0);

		maskimg = bin.clone();
		img = Gray.clone();
		tmp = Gray.clone();

		cv::bitwise_and(maskimg, img, mforground);

		for (int y = 0; y < mforground.rows; y++) {
			for (int x = 0; x < mforground.cols; x++) {
				if ((int)mforground.data[y*mforground.cols + x] > 1)
				{
					Pixcnt++;
					forg += (int)mforground.data[y*mforground.cols + x];
				}
			}
		}

		if (Pixcnt < 1)
			return false;
		forg = forg / Pixcnt;
		Pixcnt = 0;

		cv::bitwise_not(maskimg, maskimg);
		cv::bitwise_and(maskimg, img, mbackground);

		for (int y = 0; y < mbackground.rows; y++) {

			for (int x = 0; x < mbackground.cols; x++) {

				if ((int)mbackground.data[y*mbackground.cols + x] > 1) {
					Pixcnt++;
					backg += (int)mbackground.data[y*mbackground.cols + x];
				}
			}
		}
		if (Pixcnt < 1)
			return false;

		backg = backg / Pixcnt;


		cv::Scalar color(backg);	//black
		cv::Scalar _color(forg);	//while

		cv::bitwise_not(maskimg, maskimg);
		for (int y = 0; y < maskimg.rows; y++) {
			for (int x = 0; x < maskimg.cols; x++) {
				if ((int)maskimg.data[y*maskimg.cols + x] > 1)	tmp.data[y*maskimg.cols + x] = forg;
				else tmp.data[y*maskimg.cols + x] = backg;
			}
		}

		sClipData->_ForVal = forg;
		sClipData->_backVal = backg;

		return true;
	}
	bool CharClipper::CalcAvgForBack(cv::Mat bin, cv::Mat Gray, cv::Mat& tmp, int threshold, int& backg)
	{
		cv::Mat maskimg, img;
		cv::Mat mbackground, mforground;
		cv::Scalar forground, background;
		int forg(0), Pixcnt(0);

		maskimg = bin.clone();
		img = Gray.clone();
		tmp = Gray.clone();

		cv::bitwise_and(maskimg, img, mforground);

		for (int y = 0; y < mforground.rows; y++) {
			for (int x = 0; x < mforground.cols; x++) {
				if ((int)mforground.data[y*mforground.cols + x] > 1)
				{
					Pixcnt++;
					forg += (int)mforground.data[y*mforground.cols + x];
				}
			}
		}

		if (Pixcnt < 1)
			return false;
		forg = forg / Pixcnt;
		Pixcnt = 0;

		cv::bitwise_not(maskimg, maskimg);
		cv::bitwise_and(maskimg, img, mbackground);

		for (int y = 0; y < mbackground.rows; y++) {

			for (int x = 0; x < mbackground.cols; x++) {

				if ((int)mbackground.data[y*mbackground.cols + x] > 1) {
					Pixcnt++;
					backg += (int)mbackground.data[y*mbackground.cols + x];
				}
			}
		}
		if (Pixcnt < 1)
			return false;

		backg = backg / Pixcnt;

		//shkim 검증
		int InnerPixel(0);
		for (int y = 0; y < mbackground.rows; y++) {

			for (int x = 0; x < mbackground.cols; x++) {

				if (abs((int)mbackground.data[y*mbackground.cols + x] - backg) <= 5)
				{
					InnerPixel++;
				}
			}
		}

		cv::Scalar color(backg);	//black
		cv::Scalar _color(forg);	//while

		sClipData->_ForVal = forg;
		sClipData->_backVal = backg;

		cv::bitwise_not(maskimg, maskimg);
		if (InnerPixel > (Pixcnt*0.4))
		{
			for (int y = 0; y < maskimg.rows; y++) {
				for (int x = 0; x < maskimg.cols; x++) {
					if ((int)maskimg.data[y*maskimg.cols + x] > 1)	tmp.data[y*maskimg.cols + x] = forg;
					else tmp.data[y*maskimg.cols + x] = backg;
				}
			}
		}
		else
		{
			for (int y = 0; y < maskimg.rows; y++) {
				for (int x = 0; x < maskimg.cols; x++) {
					if ((int)maskimg.data[y*maskimg.cols + x] > 1)	tmp.data[y*maskimg.cols + x] = forg;
					else if (mbackground.data[y*mbackground.cols + x] > threshold)
						tmp.data[y*maskimg.cols + x] = backg;
				}
			}
			backg = 0;
		}

		return true;
	}

	int CharClipper::GetRectArray(int strLength, RECT* ReArr)
	{
		int nLenth = strLength;

		if (nLenth > 255)
			nLenth = 255;
		if (sClipData->nRectLength == strLength)
		{
			for (int i = 0; i < sClipData->nRectLength; i++)
			{
				ReArr[i].left = sClipData->center_x[i] - (sClipData->width[i] / 2);
				ReArr[i].top = sClipData->center_y[i] - (sClipData->height[i] / 2);
				ReArr[i].right = sClipData->center_x[i] + (sClipData->width[i] / 2);
				ReArr[i].bottom = sClipData->center_y[i] + (sClipData->height[i] / 2);
			}
		}
		else if (strLength == 0)
		{
			//int nLeft = sClipData->center_x[0] - (sClipData->width[0] / 2);
			//int nRight = sClipData->center_x[sClipData->nRectLength - 1] + (sClipData->width[sClipData->nRectLength - 1] / 2);
			//int nTop = sClipData->center_y[0] - (sClipData->height[0] / 2);
			//int nBottom = sClipData->center_y[0] + (sClipData->height[0] / 2);

			//int nWidth = (nRight - nLeft) / sClipData->nRectLength;
			//nRight = nLeft + nWidth;
			//for (int i = 0; i < sClipData->nRectLength; i++)
			//{
			//	ReArr[i].left = nLeft + 3;
			//	ReArr[i].top = nTop;
			//	ReArr[i].right = nRight - 3;
			//	ReArr[i].bottom = nBottom;

			//	nLeft += nWidth;
			//	nRight += nWidth;
			//}
			for (int i = 0; i < sClipData->nRectLength; i++)
			{
				ReArr[i].left = sClipData->center_x[i] - (sClipData->width[i] / 2);
				ReArr[i].top = sClipData->center_y[i] - (sClipData->height[i] / 2);
				ReArr[i].right = sClipData->center_x[i] + (sClipData->width[i] / 2);
				ReArr[i].bottom = sClipData->center_y[i] + (sClipData->height[i] / 2);
			}

			nLenth = sClipData->nRectLength;
		}
		else if (sClipData->nRectLength > strLength)
		{
			for (int i = 0; i < strLength; i++)
			{
				ReArr[i].left = sClipData->center_x[i] - (sClipData->width[i] / 2);
				ReArr[i].top = sClipData->center_y[i] - (sClipData->height[i] / 2);
				ReArr[i].right = sClipData->center_x[i] + (sClipData->width[i] / 2);
				ReArr[i].bottom = sClipData->center_y[i] + (sClipData->height[i] / 2);
			}
			nLenth = strLength;
		}
		else if (sClipData->nRectLength > 0)
		{
			int nLeft = sClipData->center_x[0] - (sClipData->width[0] / 2);
			int nRight = sClipData->center_x[sClipData->nRectLength - 1] + (sClipData->width[sClipData->nRectLength - 1] / 2);
			int nTop = sClipData->center_y[0] - (sClipData->height[0] / 2);
			int nBottom = sClipData->center_y[0] + (sClipData->height[0] / 2);

			int nWidth = (nRight - nLeft) / strLength;
			nRight = nLeft + nWidth;
			for (int i = 0; i < strLength; i++)
			{
				ReArr[i].left = nLeft + 3;
				ReArr[i].top = nTop;
				ReArr[i].right = nRight - 3;
				ReArr[i].bottom = nBottom;

				nLeft += nWidth;
				nRight += nWidth;
			}
			nLenth = strLength;
		}
		else
		{
			return 0;
		}
		return nLenth;
	}

	// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
	sFont::sFont(sFontList * pParent)
	{
		_pParent = pParent;
		_color = enmGrayType::GtForeBright;

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);

	}

	sFont::~sFont()
	{
		dataClear();

	g_pMManager->pem_delete_check(this);
	}

	bool sFont::Alloc(TCHAR ch, cv::Mat img, cv::Mat Grayimg, int color)
	{
		cv::Mat CalcImg;

		CalcImg = CalcAvgForBack(Grayimg, img);
		_ch = ch;
	_Img = img.clone();
	_ModelEdge = std::make_shared<GeoModel_POCR>();

	_ModelEdge->Alloc(CalcImg.clone());
	_ModelEdge->setAngleRange(true, 6, 6, 2);
	_ModelEdge->Preprocess();

	_ModelDiff = std::make_shared<GeoModel_POCR>();
	_color = color;

	_ModelDiff->Alloc(CalcImg.clone());
	_ModelDiff->setMatchAlgo(3+_color);	// fore가 어두우면 3, 밝으면 4
	_ModelDiff->setAngleRange(true, 6, 6, 2);
	_ModelDiff->Preprocess();

	_ModelPnts = std::make_shared<GeoModel_PntsPOCR>();

	_ModelPnts->_modifiedAlloc(CalcImg.clone(), img.clone(), _ForVal, _backVal);
	_ModelPnts->setAngleRange(true, 6, 6, 2);
	_ModelPnts->Preprocess();
	_ModelPnts->Detect(_ch);
	_ModelPnts->setColorThres(_Threshold);
	int len = CalcImg.cols * CalcImg.rows;
	_area = 0;
	uchar *pCalcImgData = CalcImg.data;
	for (int i = 0; i < len; i++)
	{
		if (pCalcImgData[i] == _ForVal)
			_area++;
	}
#if ImageSave
		// 	cv::Mat image = _ModelEdge->Image().clone();
		cv::imwrite("D:\\testimage\\img.bmp", img.clone());
		cv::imwrite("D:\\testimage\\Grayimg.bmp", Grayimg.clone());
		cv::imwrite("D:\\testimage\\Diff.bmp", CalcImg.clone());
		// 	cv::imwrite("D:\\testimage\\_Img.bmp", _Img);
#endif
//	CalcAvgForBack(_ModelEdge->Image(),Grayimg);
		return true;
}
bool sFont::ConvertFileVersion()
{
	if (_ModelPnts == nullptr)
	{
		if (_ModelEdge == nullptr)
			return false;
		try
		{
			_ModelPnts = std::make_shared<GeoModel_PntsPOCR>();
			cv::Mat Img, gray;
			cv::resize(_ModelEdge->Image(), Img, cv::Size(_Img.cols, _Img.rows));
			if (_Img.empty())
				gray = _ModelEdge->Image().clone();
			else
				gray = _Img.clone();
			_ModelPnts->_modifiedAlloc(Img, gray, _ForVal, _backVal);
			_ModelPnts->setAngleRange(true, 6, 6, 2);
			_ModelPnts->Preprocess();
			_ModelPnts->Detect(_ch);
			_ModelPnts->setColorThres(_Threshold);
			if (_area <= 0 || _area >= _Img.cols * _Img.rows)
			{
				_area = 0;
				uchar *pImageData = Img.data;
				int len = Img.rows * Img.cols;
				if (_Threshold == 0)
					_Threshold = (_ForVal + _backVal) / 2;
				if (_ForVal > _backVal)
					for (int i = 0; i < len; i++)
					{
						if (pImageData[i] >= _Threshold)
							_area++;
					}
				else
					for (int i = 0; i < len; i++)
					{
						if (pImageData[i] <= _Threshold)
							_area++;
					}
			}
			return true;
		}
		catch (cv::Exception & e)
		{
			_ModelPnts = nullptr;
		}
		catch (std::exception & e)
		{
			_ModelPnts = nullptr;
		}
	}

	return false;
}
	bool sFont::SaveToFile(CArchive & ar, double resX, double resY)
	{
	SaveHeader(ar, resX, resY);


		SaveModelEdge(ar, resX, resY);

		SaveModelDiff(ar, resX, resY);

		return true;
	}

	bool sFont::LoadFromFile(CArchive & ar, double resX, double resY, CString path, int fontColor)
	{
	LoadHeader(ar, resX, resY);

		LoadModelEdge(ar, resX, resY);
		LoadModelDiff(ar, resX, resY);

	if (_ModelPnts != nullptr)
	{
		if (_ModelPnts->GrayEmpty())
		{
			_ModelPnts = nullptr;
			g_pInspMng->m_bPsrNeedUpDate = true;
		}
		else
			_ModelPnts->reSizeModelPnt(_ModelEdge);
	}
	else
		g_pInspMng->m_bPsrNeedUpDate = true;
		return true;
	}

void sFont::SaveHeader(CArchive & ar, double resX, double resY)
	{
		CMemFile CMFile;
		CArchive m_ar(&CMFile, CArchive::store);
		int ByteSz(0);

		//2017.05.02 shkim	써야할 변수 저장.
		//이곳에 쓸것////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cv::Mat image = _ModelEdge->Image();
		m_ar << _ch;
		m_ar << _color;
		// 기본 정보 
		int Wid(0), Len(0);
		Wid = _Img.cols;
		Len = _Img.rows;
		m_ar << Wid;
		m_ar << Len;
		if (Wid > 0 && Len > 0)
		{
			for (int y = 0; y < _Img.rows; y++)
			{
				uchar * ptr = _Img.ptr(y);
				for (int x = 0; x < _Img.cols; x++)
					m_ar << ptr[x];
			}
		}
		m_ar << _backVal;
		m_ar << _ForVal;
		m_ar << _center_x;
		m_ar << _center_y;
		m_ar << _width;
		m_ar << _height;
		m_ar << _Threshold;
	if (_ModelPnts == nullptr)
	{
		_ModelPnts = std::make_shared<GeoModel_PntsPOCR>();
		_ModelPnts->SaveFile_Pnts(m_ar, resX, resY);
		_ModelPnts.reset();
		_ModelPnts = nullptr;
	}
	else
		_ModelPnts->SaveFile_Pnts(m_ar, resX, resY);
	m_ar << _area;
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		m_ar.Close();

		ByteSz = CMFile.GetLength();
		ar << ByteSz;
		BYTE * Buf = CMFile.Detach();
		ar.Write(Buf, ByteSz);
	//delete [] Buf;
	g_pMManager->pem_delete(Buf, true);
		CMFile.Close();
	}

	void sFont::SaveModelEdge(CArchive & ar, double resX, double resY)
	{
		CMemFile CMFile;
		CArchive m_ar(&CMFile, CArchive::store);
		int ByteSz(0);

		_ModelEdge->setRes(resX, resY);
	_ModelEdge->SaveFile(m_ar, false);
		m_ar.Close();

		ByteSz = CMFile.GetLength();
		ar << ByteSz;
		BYTE * Buf = CMFile.Detach();
		ar.Write(Buf, ByteSz);
	//delete [] Buf;
	g_pMManager->pem_delete(Buf, true);
		CMFile.Close();


	}

	void sFont::SaveModelDiff(CArchive & ar, double resX, double resY)
	{
		CMemFile CMFile;
		int ByteSz(0);
		{
			CArchive m_ar(&CMFile, CArchive::store);

			_ModelDiff->setRes(resX, resY);
		_ModelDiff->SaveFile(m_ar, false);
			m_ar.Close();
		}
		ByteSz = CMFile.GetLength();
		ar << ByteSz;
		BYTE * Buf = CMFile.Detach();
		ar.Write(Buf, ByteSz);
	//delete [] Buf;
	g_pMManager->pem_delete(Buf, true);
		CMFile.Close();

	}


	void sFont::ThrowFontImg(int fListIndex, int fontIndex, CString destPath)//2017.05.29 shkim for FontEdit
	{
		bool flag = true;
		CString path;
		int idx = 0;
		while (flag)
		{

			CString tmp;
			AfxExtractSubString(tmp, destPath, idx, '\\');

			if (tmp.GetLength() == 0)
			{
				flag = false;
			}
			idx++;
		}

		CString Pathclone;
		CString tmp;

		AfxExtractSubString(tmp, destPath, 0, '\\');

		path.Format(_T("%s"), tmp);

		for (int i = 1; i < idx - 2; i++)
		{

			AfxExtractSubString(tmp, destPath, i, '\\');

			Pathclone.Format(_T("%s"), path);
			path.Format(_T("%s\\%s"), Pathclone, tmp);
		}
		CString spath;
		CFileFind Finder;

		CString FolderPath;
		FolderPath.Format(_T("%s\\ORGFont"), path);

		int fileExist = Finder.FindFile(FolderPath);
		if (PathFileExists(FolderPath) == false)
		{
			CreateDirectory(FolderPath, NULL);
		}

		spath.Format(_T("%s\\%d_%d@%c.bmp"), FolderPath, fListIndex, fontIndex, _ch);
		//cv::Mat fontImage = CalcAvgForBack(_Img.clone(),_Img);

		cv::imwrite(std::string(CT2A(spath)), _Img.clone());

#if ImageSave
		// 	cv::imwrite("D:\\testimage\\POCRFont\\ThrowFontImg.bmp",_Img.clone());
		// 	cv::imwrite("D:\\testimage\\POCRFont\\ThrowFontEdgeImg.bmp",_Img.clone());
		// 	if(PathFileExists(_T("D:\\testimage\\POCRFont"))==false)
		// 	{
		// 		CreateDirectory(_T("D:\\testimage\\POCRFont"),NULL);
		// 	}
		// 	spath.Format(_T("D:\\testimage\\POCRFont\\%d_%d@%c.bmp"),fListIndex,fontIndex,_ch);
		// 	cv::imwrite(std::string(CT2A(spath)),_ModelEdge->Image().clone());
#endif

	}

void sFont::LoadHeader(CArchive & ar, double resX, double resY)
	{
		int ByteSz(0);
		ar >> ByteSz;
		if (ByteSz == 0)
			return;
		std::vector<BYTE> buf(ByteSz);
		ar.Read(&buf[0], ByteSz);

		CMemFile CMFile;
		CMFile.Attach(&buf[0], ByteSz);
		CArchive m_ar(&CMFile, CArchive::load, ByteSz);


		m_ar >> _ch;
		m_ar >> _color;

		// 기본 정보 
		int Wid(0), Len(0);
		m_ar >> Wid;
		m_ar >> Len;
		if (Wid > 0 && Len > 0)
		{
			_Img = cv::Mat(Len, Wid, CV_8UC1);
			for (int y = 0; y < _Img.rows; y++)
			{
				uchar * ptr = _Img.ptr(y);
				for (int x = 0; x < _Img.cols; x++)
					m_ar >> ptr[x];
			}
		}
		if (m_ar.IsBufferEmpty() == FALSE)
			m_ar >> _backVal;
		else
			_backVal = 0;

		if (m_ar.IsBufferEmpty() == FALSE)
			m_ar >> _ForVal;
		else
			_ForVal = 0;

		if (m_ar.IsBufferEmpty() == FALSE)
			m_ar >> _center_x;
		else
			_center_x = 0;

		if (m_ar.IsBufferEmpty() == FALSE)
			m_ar >> _center_y;
		else
			_center_y = 0;

		if (m_ar.IsBufferEmpty() == FALSE)
			m_ar >> _width;
		else
			_width = 0;
		if (m_ar.IsBufferEmpty() == FALSE)
			m_ar >> _height;
		else
			_height = 0;

		if (m_ar.IsBufferEmpty() == FALSE)
			m_ar >> _Threshold;
		else
			_Threshold = 0;
	if (m_ar.IsBufferEmpty() == FALSE)
	{
		_ModelPnts = std::make_shared<GeoModel_PntsPOCR>();
		if (!_ModelPnts->LoadFile_Pnts(m_ar, resX, resY))
		{
			_ModelPnts.reset();
			_ModelPnts = nullptr;
		}
	}
	else
		_ModelPnts = nullptr;
	if (m_ar.IsBufferEmpty() == FALSE)
		m_ar >> _area;
	else
		_area = 0;
	if ((_area <= 0 || _area >= Wid * Len) && _ModelPnts != nullptr && _ModelPnts->Image().empty() == false)
	{
		_area = 0;
		cv::Mat image = _ModelPnts->Image();
		uchar *pImageData = image.data;
		int len = image.rows * image.cols;
		if (_Threshold == 0)
			_Threshold = (_ForVal + _backVal) / 2;
		if(_ForVal > _backVal)
			for (int i = 0; i < len; i++)
			{
				if (pImageData[i] >= _Threshold)
					_area++;
			}
		else
			for (int i = 0; i < len; i++)
			{
				if (pImageData[i] <= _Threshold)
					_area++;
			}
	}

		m_ar.Close();
		CMFile.Close();
		buf.clear();
	}
	void sFont::LoadModelEdge(CArchive & ar, double resX, double resY)
	{
		int ByteSz(0);
		ar >> ByteSz;
		if (ByteSz == 0)
			return;
		std::vector<BYTE> buf(ByteSz);
		ar.Read(&buf[0], ByteSz);

		CMemFile CMFile;
		CMFile.Attach(&buf[0], ByteSz);
		CArchive m_ar(&CMFile, CArchive::load, ByteSz);

	_ModelEdge = std::make_shared<GeoModel_POCR>();
		_ModelEdge->LoadFile(m_ar, resX, resY);


		m_ar.Close();
		CMFile.Close();
		buf.clear();

		if (_width == 0)
		{
			_width = _ModelEdge->Image().cols - 5 > 0 ? _ModelEdge->Image().cols : 0;
		}
		if (_height == 0)
		{
			_height = _ModelEdge->Image().rows - 5 > 0 ? _ModelEdge->Image().rows : 0;
		}
	}
	void sFont::LoadModelDiff(CArchive & ar, double resX, double resY)
	{
		int ByteSz(0);
		ar >> ByteSz;
		if (ByteSz == 0)
			return;
		std::vector<BYTE> buf(ByteSz);
		ar.Read(&buf[0], ByteSz);

		CMemFile CMFile;
		CMFile.Attach(&buf[0], ByteSz);
		CArchive m_ar(&CMFile, CArchive::load, ByteSz);

	_ModelDiff = std::make_shared<GeoModel_POCR>();
		_ModelDiff->LoadFile(m_ar, resX, resY);


		m_ar.Close();
		CMFile.Close();
		buf.clear();
	}
	void sFont::SaveFontImg(CString Path, int index, CString destPath, bool bOrg)
	{
		cv::Mat image = _ModelEdge->Image().clone();

		CString cutString;
		//	AfxExtractSubString( cutString, Path, 0, '.');
		cutString = Path.Left(Path.ReverseFind('.'));

		if (destPath.IsEmpty() == false && destPath != "")
		{
			CString ModelName = cutString.Right(cutString.GetLength() - cutString.ReverseFind('\\') - 1);
			cutString.Format(_T("%s\\%s"), destPath, ModelName);
		}

		CString spath;
		spath.Format(_T("%s@%d@%c.bmp"), cutString, index, _ch);

		cv::Mat fontImage;

		if (bOrg)
			fontImage = _Img.clone();
		else
			fontImage = _ModelEdge->Image().clone();

		cv::imwrite(std::string(CT2A(spath)), fontImage);

	}
	bool CharClipper::searchArea_OrgImage_AutoTeach(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue)
	{
		cv::Mat tmp, img, _img, bilblur, medblur, blur, maskimg, BinAndImage;
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		//	int index(0);
		int cnt(1);
		img = src.clone();
		_img = img.clone();
		tmp = cv::Mat(_img.rows, _img.cols, CV_8UC1);
		MatList.clear();


		maskimg = _img.clone();
		tmp = _img.clone();

		cv::findContours(maskimg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 외곽선 추출

		int size = contours.size();
		if (size < 1)
			return false;

		std::vector<cv::Rect> rect;
		int* index = g_pMManager->pem_new<int>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		double* indouble = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		double* x = g_pMManager->pem_new<double>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		for (int idx = 0; idx < size; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[idx]); //지정된 사각형을 계산해서 반환.
			double area = cv::contourArea(contours[idx]);

			int Recx = Rbox.x;
			int Recy = Rbox.y;
			Rbox.x = Rbox.x - 5 >= 0 ? Rbox.x - 5 : 0;
			Rbox.y = Rbox.y - 5 >= 0 ? Rbox.y - 5 : 0;
			Recx = Recx - Rbox.x;
			Recy = Recy - Rbox.y;
			Rbox.width = Rbox.width + Rbox.x + 10 >= tmp.cols ? tmp.cols - Rbox.x - 1 : Rbox.width + Recx + 5;
			Rbox.height = Rbox.height + Rbox.y + 10 >= tmp.rows ? tmp.rows - Rbox.y - 1 : Rbox.height + Recy + 5;

			index[idx] = idx;
			indouble[idx] = area;
			x[idx] = Rbox.x;

			for (int a = idx; 0 < a; a--)
			{
				if (indouble[a - 1] < indouble[a])
				{
					int ntmp = index[a - 1];
					double dtmp = indouble[a - 1];
					index[a - 1] = index[a];
					indouble[a - 1] = indouble[a];
					index[a] = ntmp;
					indouble[a] = dtmp;

					double xtmp = x[a - 1];
					x[a - 1] = x[a];
					x[a] = xtmp;
				}
			}

		}

		if (size < strLength) {
			g_pMManager->pem_delete(index, true);
			g_pMManager->pem_delete(indouble, true);
			g_pMManager->pem_delete(x, true);
			return false;
		}

		if (contours.size() > 0)
		{
			for (int idx = 0; idx < size; idx++)
			{
				for (int a = idx; 0 < a; a--)
				{
					cv::Rect Rbox = cv::boundingRect(contours[index[a]]); //지정된 사각형을 계산해서 반환.
					cv::Rect Rtmp = cv::boundingRect(contours[index[a - 1]]);
					if (Rtmp.height < Rbox.height)
					{
						int ntmp = index[a - 1];
						double dtmp = indouble[a - 1];
						index[a - 1] = index[a];
						indouble[a - 1] = indouble[a];
						index[a] = ntmp;
						indouble[a] = dtmp;

						double xtmp = x[a - 1];
						x[a - 1] = x[a];
						x[a] = xtmp;
					}
				}
			}

		}
#if ImageSave
		cv::imwrite("D:\\testimage\\font.bmp", maskimg);
#endif
		int nStrL = strLength;
		//2017.06.30 shkim 최대 높이의 50% 미만은 문자에서 제외
		cv::Rect MaxRbox = cv::boundingRect(contours[index[0]]);
		for (int idx = 1; idx < size; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[idx]]);
			if (MaxRbox.height*0.5 > Rbox.height)
			{
				// 			delete [] index;
				// 			delete [] indouble;
				// 			delete [] x;
				// 			return false;
				nStrL = idx;
				break;;
			}
			if (Rbox.height > MaxRbox.height)
				MaxRbox.height = Rbox.height;
		}
		if (strLength > nStrL || strLength == 0)
			strLength = nStrL;
		if (strLength == 0)
		{
			g_pMManager->pem_delete(index, true);
			g_pMManager->pem_delete(indouble, true);
			g_pMManager->pem_delete(x, true);
			return false;
		}
		std::vector<std::vector<cv::Rect>> FontRectLine;
		std::vector<std::vector<int>> FontIndexLine;
		std::vector<std::vector<int>> FontXLine;
		for (int idx = 0; idx < nStrL; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[index[idx]]); //지정된 사각형을 계산해서 반환.
			int cy = Rbox.y + (Rbox.height / 2);
			bool bIn = false;
			for (int i = 0; i < FontRectLine.size(); i++)
			{
				for (int j = 0; j < FontRectLine[i].size(); j++)
				{
					if (cy > FontRectLine[i][j].y && cy < (FontRectLine[i][j].y + FontRectLine[i][j].height))
					{
						bIn = true;
						FontRectLine[i].push_back(Rbox);
						FontIndexLine[i].push_back(index[idx]);
						FontXLine[i].push_back(x[idx]);
						break;
					}
				}
				if (bIn)
					break;
			}
			if (!bIn)
			{
				std::vector<cv::Rect> LineTmp;
				LineTmp.push_back(Rbox);
				FontRectLine.push_back(LineTmp);
				std::vector<int > FontIndexLinetmp;
				FontIndexLinetmp.push_back(index[idx]);
				FontIndexLine.push_back(FontIndexLinetmp);
				std::vector<int > FontXLinetmp;
				FontXLinetmp.push_back(x[idx]);
				FontXLine.push_back(FontXLinetmp);
			}
		}
		std::vector<cv::Rect> SelectFontRectLine;
		std::vector<int > SelectFontIndexLine;
		std::vector<int > SelectFontXLine;
		int nSelectedLineI = 0;
		int selecteddiff = _img.rows;
		for (int i = 0; i < FontRectLine.size(); i++)
		{
			int nCy = 0;
			for (int j = 0; j < FontRectLine[i].size(); j++)
			{
				nCy += (FontRectLine[i][j].y + (FontRectLine[i][j].height / 2));
			}
			nCy /= (FontRectLine[i].size() > 0 ? FontRectLine[i].size() : 1);
			int diff = abs(nCy - (_img.rows / 2));
			if (selecteddiff > diff)
			{
				selecteddiff = diff;
				SelectFontRectLine = FontRectLine[i];
				SelectFontIndexLine = FontIndexLine[i];
				SelectFontXLine = FontXLine[i];
				nSelectedLineI = i;
			}
		}
		strLength = SelectFontRectLine.size();
		for (int idx = 0; idx < strLength; idx++)
		{
			for (int a = idx; 0 < a; a--)
			{
				if (SelectFontXLine[a - 1] > SelectFontXLine[a])
				{
					int ntmp = SelectFontIndexLine[a - 1];
					SelectFontIndexLine[a - 1] = SelectFontIndexLine[a];
					SelectFontIndexLine[a] = ntmp;

					double xtmp = SelectFontXLine[a - 1];
					SelectFontXLine[a - 1] = SelectFontXLine[a];
					SelectFontXLine[a] = xtmp;

					cv::Rect Rtmp = SelectFontRectLine[a - 1];
					SelectFontRectLine[a - 1] = SelectFontRectLine[a];
					SelectFontRectLine[a] = Rtmp;
				}
			}

		}
		cv::bitwise_and(img, BinImage, BinAndImage);
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\maskimg_draw_bf.bmp", BinAndImage);
#endif
		for (int idx = nStrL; idx < size; idx++)
		{
			cv::Scalar color(0);
			drawContours(BinAndImage, contours, index[idx], color, cv::FILLED, 1, hierarchy);
		}

		for (int i = 0; i < FontIndexLine.size(); i++)
		{
			if (nSelectedLineI == i)
				continue;
			for (int j = 0; j < FontIndexLine[i].size(); j++)
			{
				cv::Scalar color(0);
				drawContours(BinAndImage, contours, FontIndexLine[i][j], color, cv::FILLED, 1, hierarchy);
			}
		}
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\maskimg_draw.bmp", BinAndImage);
#endif
		int rectL = BinAndImage.cols;
		int rectT = BinAndImage.rows;
		int rectR(0), rectB(0);
		for (int i = 0; i < strLength; i++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[SelectFontIndexLine[i]]); //지정된 사각형을 계산해서 반환.

			if (Rbox.x < rectL)
				rectL = Rbox.x;
			if (Rbox.y < rectT)
				rectT = Rbox.y;
			if (rectR < Rbox.x + Rbox.width)
				rectR = Rbox.x + Rbox.width;
			if (rectB < Rbox.y + Rbox.height)
				rectB = Rbox.y + Rbox.height;
		}
		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
		if (rectR - rectL > BinAndImage.cols - 5 && rectB - rectT > BinAndImage.rows - 5) {
			g_pMManager->pem_delete(index, true);
			g_pMManager->pem_delete(indouble, true);
			g_pMManager->pem_delete(x, true);
			return false;
		}
		rectL = rectL - 5 < 0 ? 0 : rectL - 5;
		rectT = rectT - 5 < 0 ? 0 : rectT - 5;
		rectR = rectR + 5 < BinAndImage.cols ? rectR + 5 : BinAndImage.cols;
		rectB = rectB + 5 < BinAndImage.rows ? rectB + 5 : BinAndImage.rows;
		cv::Mat ListImg;
		int backg(0);
		bool bCalc = CalcAvgForBack(BinAndImage, Grayimg, ListImg, threshValue, backg);
		if (!bCalc) {
			g_pMManager->pem_delete(index, true);
			g_pMManager->pem_delete(indouble, true);
			g_pMManager->pem_delete(x, true);
			return false;
		}
		cv::Rect ListImgRect(rectL, rectT, rectR - rectL, rectB - rectT);
		sClipData->_img = ListImg(ListImgRect).clone();
		cv::Point NCent = cv::Point(src.cols / 2, src.rows / 2);
#if ImageSave
		cv::imwrite("D:\\testimage\\POCRFont\\sClipData_img.bmp", sClipData->_img);
#endif
		//sClipData->_img = CalcAvgForBack(BinAndImage,Grayimg);
		sClipData->nRectLength = strLength;
		sClipData->center_x = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
		sClipData->center_y = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
		sClipData->width = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);
		sClipData->height = g_pMManager->pem_new<int>(true, strLength, (PCHAR)__FUNCTION__, __LINE__);

		for (int idx = 0; idx < strLength; idx++)
		{
			cv::Rect Rbox = cv::boundingRect(contours[SelectFontIndexLine[idx]]); //지정된 사각형을 계산해서 반환.

			sClipData->width[idx] = Rbox.width;
			sClipData->height[idx] = Rbox.height;
			sClipData->center_x[idx] = Rbox.x + Rbox.width / 2 - NCent.x;
			sClipData->center_y[idx] = Rbox.y + Rbox.height / 2 - NCent.y;

			cv::Mat mask = BinAndImage(Rbox);


			cv::Mat temp = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			temp.setTo(0);
			mask.copyTo(temp(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, mask.cols, mask.rows)));
			//		cv::Mat temp = tmp(Rbox);

			cv::Mat graycrop = Grayimg(cv::Rect(Rbox.x, Rbox.y, Rbox.width, Rbox.height));

			cv::Mat gray = cv::Mat(Rbox.height + MARGIN_PIXEL * 2, Rbox.width + MARGIN_PIXEL * 2, CV_8UC1);
			gray.setTo(cv::Scalar(sClipData->_backVal));
			graycrop.copyTo(gray(cv::Rect(MARGIN_PIXEL, MARGIN_PIXEL, Rbox.width, Rbox.height)));
#if ImageSave
			cv::imwrite("D:\\testimage\\crop.bmp", temp);
			cv::imwrite("D:\\testimage\\crop_gray.bmp", gray);
#endif
			cnt++;
			MatList.push_back(temp);
			MatOrgImgList.push_back(gray);
		}

		//	std::vector<cv::Mat>::iterator	it	= MatList.begin();

		g_pMManager->pem_delete(index, true);
		g_pMManager->pem_delete(indouble, true);
		g_pMManager->pem_delete(x, true);
		return true;
	}
	cv::Mat sFont::CalcAvgForBack(cv::Mat bin, cv::Mat Gray)
	{
		cv::Mat maskimg, img, tmp;
		cv::Mat mbackground, mforground;
		cv::Scalar forground, background;
		int forg(0), backg(0), Pixcnt(0);

		maskimg = bin.clone();
		img = Gray.clone();
		tmp = Gray.clone();

		cv::bitwise_and(maskimg, img, mforground);

		//shkim case: foreGround value = 0(0=default value)
		if (_ForVal == 0)
		{
			for (int y = 0; y < mforground.rows; y++) {
				for (int x = 0; x < mforground.cols; x++) {
					if ((int)mforground.data[y*mforground.cols + x] > 1)
					{
						Pixcnt++;
						forg += (int)mforground.data[y*mforground.cols + x];
					}
				}
			}

			if (Pixcnt < 1)
				return tmp;
			forg = forg / Pixcnt;
			Pixcnt = 0;
			cv::Scalar _color(forg);	//while
			_ForVal = forg;
		}

		cv::bitwise_not(maskimg, maskimg);
		cv::bitwise_and(maskimg, img, mbackground);

		//shkim case: backGround value = 0(0=default value)
		if (_backVal == 0)
		{

			for (int y = 0; y < mbackground.rows; y++) {

				for (int x = 0; x < mbackground.cols; x++) {

					if ((int)mbackground.data[y*mbackground.cols + x] > 1) {
						Pixcnt++;
						backg += (int)mbackground.data[y*mbackground.cols + x];
					}
				}
			}

			if (Pixcnt < 1)
				return tmp;
			backg = backg / Pixcnt;


			cv::Scalar color(backg);	//black
			_backVal = backg;
		}

		cv::bitwise_not(maskimg, maskimg);
		for (int y = 0; y < maskimg.rows; y++) {
			for (int x = 0; x < maskimg.cols; x++) {
				if ((int)maskimg.data[y*maskimg.cols + x] > 1)	tmp.data[y*maskimg.cols + x] = _ForVal;
				else tmp.data[y*maskimg.cols + x] = _backVal;
			}
		}

		return tmp;
	}

	int sFont::getDataCount()
	{
		return 0;
	}
	bool sFont::dataClear()
	{
		_Img.release();
		_ModelEdge.reset();
		_ModelDiff.reset();
	_ModelPnts.reset();
		return true;
	}
std::vector<std::shared_ptr<rstPSRModel>> sFont::SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, cPOCRInfoParam* vPOCRInspPtr, int cnt, algoPOCR_ParamContainer* param)
	{
		std::vector<std::shared_ptr<rstPSRModel>> rst;

		bool isSmallFont = false;
		if ((_ModelEdge->Image().rows <= 20 || _ModelEdge->Image().cols <= 20) && !_CheckThinFont(_ch))
			isSmallFont = true;
		// 엣지 알고리즘으로 먼저 검사 한다.
	// 	GeoMatch_POCR gmEdge;
	// 	std::shared_ptr<GeoResult_POCR> m_milPatResult = std::shared_ptr<GeoResult_POCR>(new GeoResult_POCR(2+cnt));
	// 	gmEdge.FindModel_Char(_ch, srcImg, *_ModelEdge, vPOCRInspPtr, *m_milPatResult);
		GeoMatch_POCR gmEdge;
	std::shared_ptr<GeoResult_POCR> m_milPatResult = std::make_shared<GeoResult_POCR>(cnt);
		gmEdge._EngineParam = vPOCRInspPtr;
		gmEdge._Ch = _ch;
		//gmEdge.CalcDivScore(srcImg, *_ModelEdge, *m_milPatResult);
		gmEdge.SetImgProcess(ImgProc->edge);
		gmEdge.FindModel_Char(_ch, *_ModelEdge, vPOCRInspPtr, *m_milPatResult);

	if (g_pMPTI->m_nPOCRAlgoVersion == 2 && _ModelPnts != nullptr)		// 2021.06.23  특징점 검사 활용 여부 체크 분기점
	{
		GeoMatch_PntsPOCR gmPnts;
		gmPnts._EngineParam = vPOCRInspPtr;
		gmPnts._Ch = _ch;
		gmPnts.SetImgProcess(ImgProc->Img);
		gmPnts.CalcDivScore(*_ModelPnts, *m_milPatResult, cv::Point2i(_center_x, _center_y));
	}
	else
	{
		// 편차 알고리즘은 엣지 알고리즘 결과에 세부 검사만 더 한다.
		CheckPOCRModelSize(*_ModelDiff, m_milPatResult->_AngleStepId);
		GeoMatch_POCR gmDiff;
		gmDiff._EngineParam = vPOCRInspPtr;
		gmDiff._Ch = _ch;
		gmDiff.SetImgProcess(ImgProc->diff);
		gmDiff.CalcDivScore(*_ModelDiff, *m_milPatResult, _Threshold);


		// 이미지 알고리즘도 엣지 알고리즘 결과에 세부 검사만 더 한다.
		CheckPOCRModelSize(*_ModelEdge, m_milPatResult->_AngleStepId);
		std::shared_ptr<GeoModel_POCR> ModelImage = std::make_shared<GeoModel_POCR>();
		ModelImage->Alloc(_ModelEdge->Image());
		ModelImage->setMatchAlgo(GeoModel::enmMatchAlgo::agImage);
		ModelImage->Preprocess();


		GeoMatch_POCR gmImage;
		gmImage._EngineParam = vPOCRInspPtr;
		gmImage._Ch = _ch;
		gmImage.SetImgProcess(ImgProc->Img);
		gmImage.CalcDivScore(*ModelImage, *m_milPatResult, _Threshold);
		gmImage.CalcResScore(*m_milPatResult, isSmallFont);
	}
		// 	if(m_milPatResult->getResCnt() <= 0)
		// 	{
		// 		AfxMessageBox(_T("m_milPatResult->getResCnt() <= 0"));
		// 	}
	double con(0), up(0), op(0), blobsize(0);
	if (_area > 0 && param != NULL)
	{
		cv::Mat model = _ModelEdge->Image(), clipModel;
		int cols = model.cols, rows = model.rows;
		int stRoiX;
		int stRoiY;
		if (g_pMPTI->m_nPOCRAlgoVersion == 2 && _ModelPnts != nullptr)
		{
			stRoiX = (int)(m_milPatResult->_Center_y[0] - cols / 2);
			stRoiY = (int)(m_milPatResult->_Center_x[0] - rows / 2);
		}
		else
		{
			stRoiX = (int)(m_milPatResult->_Center_y[0] - (float)(cols) * 0.5);
			stRoiY = (int)(m_milPatResult->_Center_x[0] - (float)(rows) * 0.5);
		}
		clipModel = model;
		if (stRoiX < 0)
		{
			cols = clipModel.cols + stRoiX;
			clipModel = clipModel(cv::Rect(-stRoiX, 0, cols, clipModel.rows));
			stRoiX = 0;
		}
		if (stRoiY < 0)
		{
			rows = clipModel.rows + stRoiY;
			clipModel = clipModel(cv::Rect(0, -stRoiY, clipModel.cols, rows));
			stRoiY = 0;
		}
		if (stRoiX + cols > ImgProc->Img->_OrgImage->cols)
		{
			cols = ImgProc->Img->_OrgImage->cols - stRoiX;
			clipModel = clipModel(cv::Rect(0, 0, cols, clipModel.rows));
		}
		if (stRoiY + rows > ImgProc->Img->_OrgImage->rows)
		{
			rows = ImgProc->Img->_OrgImage->rows - stRoiY;
			clipModel = clipModel(cv::Rect(0, 0, clipModel.cols, rows));
		}
		cv::Mat src2 = (*ImgProc->Img->_OrgImage)(cv::Rect(stRoiX, stRoiY, cols, rows));
		param->fontArea = _area;
		CalcShapeDiff(param, clipModel, src2, _Threshold, false, con, up, op, blobsize);
		con = double(_ForVal - _backVal) / double(_ForVal + _backVal) * 100.0 - con;
		if (con < 0) con = -con;
	}

		for (int i = 0; i < m_milPatResult->getResCnt(); i++)//retrun 되는 결과값은 X,Y가 바꿔서 나옴.
		{
		std::shared_ptr<rstPSRModel> resultModel = std::make_shared<rstPSRModel>();
			resultModel->y = (double)m_milPatResult->_Center_x[i];
			resultModel->x = (double)m_milPatResult->_Center_y[i];
			resultModel->angle = (double)m_milPatResult->_Angle[i];
			resultModel->score = (double)m_milPatResult->_Equality[i];
			resultModel->ch = (char)_ch;
			resultModel->nCharWidth = _width;
			resultModel->nCharHeight = _height;
			resultModel->dContrastScore = con;
			resultModel->dUPScore = up;
			resultModel->dOPScore = op;
			resultModel->dBlobSizeValue = blobsize;

			for (int j = 0; j < MAX_POCR_DIVISITION_CNTS; j++)
			{
				int nSelIdx = m_milPatResult->_SelEngine[i];
				if (nSelIdx != INT_MAX)
					resultModel->div_score[j] = m_milPatResult->_Score[i][nSelIdx]._Score[j];
				else
					resultModel->div_score[j] = 0;
			}

			rst.push_back(resultModel);
		}

		return rst;
	}
std::vector<std::shared_ptr<rstPSRModel>> sFont::SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, cPOCRInfoParam* vPOCRInspPtr, cv::Point2f OfsChar, int cnt, algoPOCR_ParamContainer* param)
	{
		std::vector<std::shared_ptr<rstPSRModel>> rst;

		bool isSmallFont = false;
		if ((_ModelEdge->Image().rows <= 20 || _ModelEdge->Image().cols <= 20) && !_CheckThinFont(_ch))
			isSmallFont = true;
		// 엣지 알고리즘으로 먼저 검사 한다.
	// 	GeoMatch_POCR gmEdge;
	// 	std::shared_ptr<GeoResult_POCR> m_milPatResult = std::shared_ptr<GeoResult_POCR>(new GeoResult_POCR(2+cnt));
	// 	gmEdge.FindModel_Char(_ch, srcImg, *_ModelEdge, vPOCRInspPtr, *m_milPatResult);
		GeoMatch_POCR gmEdge;
		std::shared_ptr<GeoResult_POCR> m_milPatResult = std::shared_ptr<GeoResult_POCR>(new GeoResult_POCR(cnt));
		gmEdge._EngineParam = vPOCRInspPtr;
		gmEdge._Ch = _ch;
		//gmEdge.CalcDivScore(srcImg, *_ModelEdge, *m_milPatResult);
		gmEdge.SetImgProcess(ImgProc->edge);
		gmEdge.FindModel_Char(_ch, *_ModelEdge, vPOCRInspPtr, OfsChar, *m_milPatResult);

	if (g_pMPTI->m_nPOCRAlgoVersion == 2 && _ModelPnts != nullptr)		// 2021.06.23  특징점 검사 활용 여부 체크 분기점
	{
		GeoMatch_PntsPOCR gmPnts;
		gmPnts._EngineParam = vPOCRInspPtr;
		gmPnts._Ch = _ch;
		gmPnts.SetImgProcess(ImgProc->Img);
		gmPnts.CalcDivScore(*_ModelPnts, *m_milPatResult, cv::Point2i(_center_x, _center_y));
	}
	else
	{
		// 편차 알고리즘은 엣지 알고리즘 결과에 세부 검사만 더 한다.
		GeoMatch_POCR gmDiff;
		gmDiff._EngineParam = vPOCRInspPtr;
		gmDiff._Ch = _ch;
		gmDiff.SetImgProcess(ImgProc->diff);
		gmDiff.CalcDivScore(*_ModelDiff, *m_milPatResult, _Threshold);


		// 이미지 알고리즘도 엣지 알고리즘 결과에 세부 검사만 더 한다.
		std::shared_ptr<GeoModel_POCR> ModelImage(new GeoModel_POCR());
		ModelImage->Alloc(_ModelEdge->Image());
		ModelImage->setMatchAlgo(GeoModel::enmMatchAlgo::agImage);
		ModelImage->Preprocess();


		GeoMatch_POCR gmImage;
		gmImage._EngineParam = vPOCRInspPtr;
		gmImage._Ch = _ch;
		gmImage.SetImgProcess(ImgProc->Img);
		gmImage.CalcDivScore(*ModelImage, *m_milPatResult, _Threshold);
		gmImage.CalcResScore(*m_milPatResult, isSmallFont);
	}
		// 	if(m_milPatResult->getResCnt() <= 0)
		// 	{
		// 		AfxMessageBox(_T("m_milPatResult->getResCnt() <= 0"));
		// 	}
	double con(0), up(0), op(0), blobsize(0);
	if (_area > 0 && param != NULL)
	{
		cv::Mat model = _ModelEdge->Image(), clipModel;
		int cols = model.cols, rows = model.rows;
		int stRoiX;
		int stRoiY;
		if (g_pMPTI->m_nPOCRAlgoVersion == 2 && _ModelPnts != nullptr)
		{
			stRoiX = (int)(m_milPatResult->_Center_y[0] - cols / 2);
			stRoiY = (int)(m_milPatResult->_Center_x[0] - rows / 2);
		}
		else
		{
			stRoiX = (int)(m_milPatResult->_Center_y[0] - (float)(cols) * 0.5);
			stRoiY = (int)(m_milPatResult->_Center_x[0] - (float)(rows) * 0.5);
		}
		clipModel = model;
		if (stRoiX < 0)
		{
			cols = clipModel.cols + stRoiX;
			clipModel = clipModel(cv::Rect(-stRoiX, 0, cols, clipModel.rows));
			stRoiX = 0;
		}
		if (stRoiY < 0)
		{
			rows = clipModel.rows + stRoiY;
			clipModel = clipModel(cv::Rect(0, -stRoiY, clipModel.cols, rows));
			stRoiY = 0;
		}
		if (stRoiX + cols > ImgProc->Img->_OrgImage->cols)
		{
			cols = ImgProc->Img->_OrgImage->cols - stRoiX;
			clipModel = clipModel(cv::Rect(0, 0, cols, clipModel.rows));
		}
		if (stRoiY + rows > ImgProc->Img->_OrgImage->rows)
		{
			rows = ImgProc->Img->_OrgImage->rows - stRoiY;
			clipModel = clipModel(cv::Rect(0, 0, clipModel.cols, rows));
		}
		cv::Mat src2 = (*ImgProc->Img->_OrgImage)(cv::Rect(stRoiX, stRoiY, cols, rows));
		param->fontArea = _area;
		CalcShapeDiff(param, clipModel, src2, _Threshold, false, con, up, op, blobsize);
		con = double(_ForVal - _backVal) / double(_ForVal + _backVal) * 100.0 - con;
		if (con < 0) con = -con;
	}

		for (int i = 0; i < m_milPatResult->getResCnt(); i++)//retrun 되는 결과값은 X,Y가 바꿔서 나옴.
		{
			std::shared_ptr<rstPSRModel> resultModel(new rstPSRModel());
			resultModel->y = (double)m_milPatResult->_Center_x[i];
			resultModel->x = (double)m_milPatResult->_Center_y[i];
			resultModel->angle = (double)m_milPatResult->_Angle[i];
			resultModel->score = (double)m_milPatResult->_Equality[i];
			resultModel->ch = (char)_ch;
			resultModel->nCharWidth = _width;
			resultModel->nCharHeight = _height;
		resultModel->dContrastScore = con;
		resultModel->dUPScore = up;
		resultModel->dOPScore = op;
		resultModel->dBlobSizeValue = blobsize;

			for (int j = 0; j < MAX_POCR_DIVISITION_CNTS; j++)
			{
				int nSelIdx = m_milPatResult->_SelEngine[i];
				if (nSelIdx != INT_MAX)
					resultModel->div_score[j] = m_milPatResult->_Score[i][nSelIdx]._Score[j];
				else
					resultModel->div_score[j] = 0;
			}

			rst.push_back(resultModel);
		}

		return rst;
	}

std::vector<std::shared_ptr<rstPSRModel>> sFont::SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, cPOCRInfoParam* vPOCRInspPtr, std::shared_ptr<GeoModel_POCR> StrModel, GeoResult & StrRes,std::shared_ptr<GeoMatch_POCR> ptrEdge, algoPOCR_ParamContainer* param)
	{
		std::vector<std::shared_ptr<rstPSRModel>> rst;
		if (_ModelEdge == nullptr)
			return rst;
		// 엣지 알고리즘으로 먼저 검사 한다.
	//	GeoMatch_POCR gmEdge;
	std::shared_ptr<GeoResult_POCR> m_milPatResult = std::make_shared<GeoResult_POCR>(1);
		ptrEdge->SetImgProcess(ImgProc->edge);
		ptrEdge->FindModel_Char_AngleFunc(param->m_nUseFontAngle, _ch, *_ModelEdge, vPOCRInspPtr, StrModel.get(), &StrRes, cv::Point2f(_center_x, _center_y), *m_milPatResult);

		bool isSmallFont = false;
		if ((_ModelEdge->Image().rows <= 20 || _ModelEdge->Image().cols <= 20) && !_CheckThinFont(_ch))
			isSmallFont = true;

	if (g_pMPTI->m_nPOCRAlgoVersion == 2 && _ModelPnts != nullptr)		// 2021.06.23 특징점 검사 활용 여부 체크 분기점
	{
		GeoMatch_PntsPOCR gmPnts;
		gmPnts._EngineParam = vPOCRInspPtr;
		gmPnts._Ch = _ch;
		gmPnts.SetImgProcess(ImgProc->Img);
		gmPnts.CalcDivScore(*_ModelPnts, *m_milPatResult, cv::Point2i(_center_x, _center_y));
	}
	else
	{
		// 편차 알고리즘은 엣지 알고리즘 결과에 세부 검사만 더 한다.
		CheckPOCRModelSize(*_ModelDiff, m_milPatResult->_AngleStepId);
		GeoMatch_POCR gmDiff;
		gmDiff._EngineParam = vPOCRInspPtr;
		gmDiff._Ch = _ch;
		gmDiff.SetImgProcess(ImgProc->diff);
		gmDiff.CalcDivScore(*_ModelDiff, *m_milPatResult, _Threshold);


		// 이미지 알고리즘도 엣지 알고리즘 결과에 세부 검사만 더 한다.
		CheckPOCRModelSize(*_ModelEdge, m_milPatResult->_AngleStepId);
		std::shared_ptr<GeoModel_POCR> ModelImage = std::make_shared<GeoModel_POCR>();
		ModelImage->Alloc(_ModelEdge->Image());
		ModelImage->setMatchAlgo(GeoModel::enmMatchAlgo::agImage);
		ModelImage->Preprocess();


		GeoMatch_POCR gmImage;
		gmImage._EngineParam = vPOCRInspPtr;
		gmImage._Ch = _ch;
		gmImage.SetImgProcess(ImgProc->Img);
		gmImage.CalcDivScore(*ModelImage, *m_milPatResult, _Threshold);
		gmImage.CalcResScore(*m_milPatResult, isSmallFont);
	}
		// 	if(m_milPatResult->getResCnt() <= 0)
		// 	{
		// 		AfxMessageBox(_T("m_milPatResult->getResCnt() <= 0"));
		// 	}
	double con(0), up(0), op(0), blobsize(0);
	if (_area > 0 && param != NULL && (param->bUseBlobSize || param->bUseContrast || param->bUseOP || param->bUseUP))
	{
		cv::Mat model = _ModelEdge->Image(), clipModel;
		int cols = model.cols, rows = model.rows;
		int stRoiX;
		int stRoiY;
		if (g_pMPTI->m_nPOCRAlgoVersion == 2 && _ModelPnts != nullptr)
		{
			stRoiX = (int)(m_milPatResult->_Center_y[0] - cols / 2);
			stRoiY = (int)(m_milPatResult->_Center_x[0] - rows / 2);
		}
		else
		{
			stRoiX = (int)(m_milPatResult->_Center_y[0] - (float)(cols) * 0.5);
			stRoiY = (int)(m_milPatResult->_Center_x[0] - (float)(rows) * 0.5);
		}
		clipModel = model;
		if (stRoiX < 0)
		{
			cols = clipModel.cols + stRoiX;
			clipModel = clipModel(cv::Rect(-stRoiX, 0, cols, clipModel.rows));
			stRoiX = 0;
		}
		if (stRoiY < 0)
		{
			rows = clipModel.rows + stRoiY;
			clipModel = clipModel(cv::Rect(0, -stRoiY, clipModel.cols, rows));
			stRoiY = 0;
		}
		if (stRoiX + cols > ImgProc->Img->_OrgImage->cols)
		{
			cols = ImgProc->Img->_OrgImage->cols - stRoiX;
			clipModel = clipModel(cv::Rect(0, 0, cols, clipModel.rows));
		}
		if (stRoiY + rows > ImgProc->Img->_OrgImage->rows)
		{
			rows = ImgProc->Img->_OrgImage->rows - stRoiY;
			clipModel = clipModel(cv::Rect(0, 0, clipModel.cols, rows));
		}
		cv::Mat src2 = (*ImgProc->Img->_OrgImage)(cv::Rect(stRoiX, stRoiY, cols, rows));
		param->fontArea = _area;
		CalcShapeDiff(param, clipModel, src2, _Threshold, false, con, up, op, blobsize);
		con = double(_ForVal - _backVal) / double(_ForVal + _backVal) * 100.0 - con;
		if (con < 0) con = -con;
	}

		for (int i = 0; i < m_milPatResult->getResCnt(); i++)//retrun 되는 결과값은 X,Y가 바꿔서 나옴.
		{
		std::shared_ptr<rstPSRModel> resultModel = std::make_shared<rstPSRModel>();
			resultModel->y = (double)m_milPatResult->_Center_x[i];
			resultModel->x = (double)m_milPatResult->_Center_y[i];
			resultModel->angle = (double)m_milPatResult->_Angle[i];
			if (HUGE_VAL != m_milPatResult->_Equality[i])
				resultModel->score = (double)m_milPatResult->_Equality[i];
			else
				resultModel->score = 0;
			resultModel->ch = (char)_ch;
			resultModel->nCharWidth = _width;
			resultModel->nCharHeight = _height;
		resultModel->dContrastScore = con;
		resultModel->dUPScore = up;
		resultModel->dOPScore = op;
		resultModel->dBlobSizeValue = blobsize;

			for (int j = 0; j < MAX_POCR_DIVISITION_CNTS; j++)
			{
				int nSelIdx = m_milPatResult->_SelEngine[i];
				if (nSelIdx != INT_MAX)
					resultModel->div_score[j] = m_milPatResult->_Score[i][nSelIdx]._Score[j];
				else
					resultModel->div_score[j] = 0;
			}

			rst.push_back(resultModel);
		}

		return rst;
	}

std::vector<std::shared_ptr<rstPSRModel>> sFont::SearchFontSingle(cv::Mat srcimg, POCRInspParam& vPOCRInspPtr, algoPOCR_ParamContainer* paramContainer)
	{
	//cPOCRInfoParam * param = new cPOCRInfoParam[POCRFont::Num];
	cPOCRInfoParam * param = g_pMManager->pem_new<cPOCRInfoParam>(true, POCRFont::Num, (PCHAR)__FUNCTION__, __LINE__);
		for (int i = 0; i < POCRFont::Num; i++)
			param[i].eFontKey = (POCRFont)i;
		cv::Mat inspImg;


		POCRFont fnt = _GetFontEnum(_ch);
		if (_Img.cols <= 30 || _Img.cols <= 30)
			fnt = POCRFont::SMall;
		param[fnt].nPOCRInspCnt = 1;
	//param[fnt].vPOCRInspPtr = param[fnt]._vPOCRInspPtr = new cPOCRInspPtr[1];
	param[fnt].vPOCRInspPtr = param[fnt]._vPOCRInspPtr = g_pMManager->pem_new<cPOCRInspPtr>(true, 1, (PCHAR)__FUNCTION__, __LINE__);
		param[fnt]._vPOCRInspPtr->_vPOCRInspPtr = new cPOCRInspParam(vPOCRInspPtr);
	g_pMManager->pem_new_check(param[fnt]._vPOCRInspPtr->_vPOCRInspPtr, (PCHAR)__FUNCTION__, __LINE__);

		param[fnt]._vPOCRInspPtr->vPOCRInspPtr = param[fnt]._vPOCRInspPtr->_vPOCRInspPtr;

		cv::resize(srcimg, inspImg, cv::Size(_Img.cols, _Img.rows));

		std::shared_ptr<ImgProcessing_POCR> ImgProc;
		ImgProc = std::make_shared<ImgProcessing_POCR>();
		ImgProc->_ImgProcess(inspImg, _ModelEdge->_pyrDownStep);
	auto ret = SearchFont(ImgProc, param, 1, paramContainer);

	//delete [] param;
	g_pMManager->pem_delete(param, true);

		return ret;
	}

	POCRFont sFont::_GetFontEnum(TCHAR ch)
	{
		switch (ch)
		{
		case '0':	return POCRFont::_0;		break;
		case '1':	return POCRFont::_1;		break;
		case '2':	return POCRFont::_2;		break;
		case '3':	return POCRFont::_3;		break;
		case '4':	return POCRFont::_4;		break;
		case '5':	return POCRFont::_5;		break;
		case '6':	return POCRFont::_6;		break;
		case '7':	return POCRFont::_7;		break;
		case '8':	return POCRFont::_8;		break;
		case '9':	return POCRFont::_9;		break;

		case 'A':	return POCRFont::A;		break;
		case 'B':	return POCRFont::B;		break;
		case 'C':	return POCRFont::C;		break;
		case 'D':	return POCRFont::D;		break;
		case 'E':	return POCRFont::E;		break;
		case 'F':	return POCRFont::F;		break;
		case 'G':	return POCRFont::G;		break;
		case 'H':	return POCRFont::H;		break;
		case 'I':	return POCRFont::I;		break;
		case 'J':	return POCRFont::J;		break;

		case 'K':	return POCRFont::K;		break;
		case 'L':	return POCRFont::L;		break;
		case 'M':	return POCRFont::M;		break;
		case 'N':	return POCRFont::N;		break;
		case 'O':	return POCRFont::O;		break;
		case 'P':	return POCRFont::P;		break;
		case 'Q':	return POCRFont::Q;		break;
		case 'R':	return POCRFont::R;		break;
		case 'S':	return POCRFont::S;		break;
		case 'T':	return POCRFont::T;		break;

		case 'U':	return POCRFont::U;		break;
		case 'V':	return POCRFont::V;		break;
		case 'W':	return POCRFont::W;		break;
		case 'X':	return POCRFont::X;		break;
		case 'Y':	return POCRFont::Y;		break;
		case 'Z':	return POCRFont::Z;		break;


		case 'a':	return POCRFont::a;		break;
		case 'b':	return POCRFont::b;		break;
		case 'c':	return POCRFont::c;		break;
		case 'd':	return POCRFont::d;		break;
		case 'e':	return POCRFont::e;		break;
		case 'f':	return POCRFont::f;		break;
		case 'g':	return POCRFont::g;		break;
		case 'h':	return POCRFont::h;		break;
		case 'i':	return POCRFont::i;		break;
		case 'j':	return POCRFont::j;		break;

		case 'k':	return POCRFont::k;		break;
		case 'l':	return POCRFont::l;		break;
		case 'm':	return POCRFont::m;		break;
		case 'n':	return POCRFont::n;		break;
		case 'o':	return POCRFont::o;		break;
		case 'p':	return POCRFont::p;		break;
		case 'q':	return POCRFont::q;		break;
		case 'r':	return POCRFont::r;		break;
		case 's':	return POCRFont::s;		break;
		case 't':	return POCRFont::t;		break;

		case 'u':	return POCRFont::u;		break;
		case 'v':	return POCRFont::v;		break;
		case 'w':	return POCRFont::w;		break;
		case 'x':	return POCRFont::x;		break;
		case 'y':	return POCRFont::y;		break;
		case 'z':	return POCRFont::z;		break;
		}
	}

	bool sFont::Preprocess()
	{
		if (_ModelEdge->isProcessed() && _ModelDiff->isProcessed())
			return true;


		// 	cv::Mat img = CalcAvgForBack(_ModelEdge->Image(),_Img);

		// 	cv::imwrite("D:\\testimage\\Preprocess_Img.bmp",img);
		// 	cv::imwrite("D:\\testimage\\Preprocess_ModelEdge.bmp",img);
		// 	cv::imwrite("D:\\testimage\\PreprocessImg.bmp",img);

		// 	_ModelEdge->Alloc(img);
		// 	_ModelEdge->setAngleRange(true, 5, 5, 2);
		_ModelEdge->Preprocess();

		// 	_ModelDiff->Alloc(img);
		// 	_ModelDiff->setMatchAlgo(3+_color);	// fore가 어두우면 3, 밝으면 4
		// 	_ModelDiff->setAngleRange(true, 5, 5, 2);
		_ModelDiff->Preprocess();

	return true;
}
bool sFont::RePreprocess()
{
	_ModelEdge->RePreprocess();
	_ModelDiff->RePreprocess();
		return true;
	}

	bool sFont::_CheckThinFont(TCHAR ch)
	{
		switch (ch)
		{
		case '1':	return true;		break;

		case 'I':	return true;		break;
		case 'i':	return true;		break;
		case 'j':	return true;		break;

		case 'l':	return true;		break;

		default:	return false;		break;
		}
	}
void sFont::CalcShapeDiff(algoPOCR_ParamContainer* param, cv::Mat &model, cv::Mat &src, double nThreshold, bool isBin, double &contrast, double &up, double &op, double &blobsize)
{
	if (model.rows != src.rows || model.cols != src.cols || param == NULL || _ModelPnts == NULL)
		return;
	cv::Mat binModel, binInvModel, binSrc, binInvSrc;
	cv::Mat mask = cv::Mat::ones(3, 3, CV_8UC1);
	std::vector<std::vector<cv::Point>> contours;

	if (param->bUseUP || param->bUseOP || param->bUseBlobSize)
		cv::threshold(src, binSrc, nThreshold, 255, cv::THRESH_BINARY);
	
	if (param->bUseUP || param->bUseContrast)
		cv::bitwise_not(binSrc, binInvSrc);
	if (isBin)
	{
		model.copyTo(binModel);
		if (param->bUseOP)
			cv::bitwise_not(binModel, binInvModel);
	}
	else
	{
		cv::threshold(model, binModel, nThreshold, 255, cv::THRESH_BINARY);
		if (param->bUseOP)
			cv::threshold(model, binInvModel, nThreshold, 255, cv::THRESH_BINARY_INV);
	}
	if (param->bUseContrast)
	{
		int fore = cv::mean(src, binSrc)[0];
		int back = cv::mean(src, binInvSrc)[0];
		contrast = double(fore - back) / double(fore + back) * 100.0;
	}
	if (param->bUseBlobSize)
	{
		int r = src.rows, c = src.cols;
		cv::Mat modelXORsrc = cv::Mat::zeros(r + 2, c + 2, CV_8UC1);
		cv::Rect roi(1, 1, c, r);
		cv::bitwise_xor(binModel, binSrc, modelXORsrc(roi));
		uchar *pXOR = modelXORsrc.data;
		cv::Mat badConnectedPixcels = cv::Mat::zeros(r, c, CV_8UC1);
		uchar *pBadImg = badConnectedPixcels.data;
		for (int i = 1; i <= r; i++)
		{
			int idx = (i - 1) * c;
			int idx_xor = i * (c + 2);
			for (int j = 1; j <= c; j++)
			{
				if (pXOR[idx_xor + j] > 0 && cv::countNonZero(modelXORsrc(cv::Rect(j - 1, i - 1, 3, 3))) > param->nPixelFilter)
					pBadImg[idx + j - 1] = 255;
			}
		}
		cv::morphologyEx(badConnectedPixcels, modelXORsrc, cv::MORPH_CLOSE, mask, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::Scalar::all(0));
		CPInsp_Algo *pinsp = g_pInspMng->GetPtrInspAlgo();
		CProcMil *procmil = pinsp->GetProcMil();
		int blobCnt = procmil->CalcBlob_Select(modelXORsrc.data, modelXORsrc.data, modelXORsrc.cols, modelXORsrc.rows, 4, FALSE, FALSE, 0, eSelectMix);
		double sumArea = 0;
		if (blobCnt > 0)
		{
			std::vector<double> vArea;
			procmil->GetBlobResult_Area(vArea);
			if (param->nBlobSizeTeachMaxCnt > 0 && blobCnt > param->nBlobSizeTeachMaxCnt)
			{
				sort(vArea.begin(), vArea.end(), greater<>());
				blobCnt = param->nBlobSizeTeachMaxCnt;
			}
			for (int i = 0; i < blobCnt; i++)
				sumArea += vArea[i];
		}
		blobsize = sumArea;

		if (_ModelPnts != nullptr && param->bImageLog && blobsize > param->dBlobSizeTeachValue)
		{
			wstring ws(g_pInspMng->GetInspPartInfo()->modelName);
			string partno(ws.begin(), ws.end());
			string str = "D:\\BL_Result\\" + partno + "_" + to_string(g_pInspMng->GetInspPartInfo()->nPartIDOrg) + "_";
			str += _ModelPnts->getModelChar();
			cv::imwrite(str + "_Model_Binary.png", binModel);
			cv::imwrite(str + "_Src_Binary.png", binSrc);
			cv::imwrite(str + "_Model.png", _ModelPnts->GrayImage());
			cv::imwrite(str + "_Src.png", src);
			cv::imwrite(str + "_DIFF.png", modelXORsrc);
		}
	}
	if (param->bUseOP)
	{
		cv::Mat invModelANDsrc;
		cv::bitwise_and(binInvModel, binSrc, invModelANDsrc);
		int cnt = cv::countNonZero(invModelANDsrc);
		int backArea = (model.rows * model.cols - param->fontArea);
		op = (1.0 - (double)cnt / backArea) * 100.0;
		if (op < 0) op = 0;
		else if (op > 100.0) op = 100.0;
	}
	if (param->bUseUP)
	{
		cv::Mat modelANDsrc;
		cv::bitwise_and(binModel, binInvSrc, modelANDsrc);
		int cnt = cv::countNonZero(modelANDsrc);
		up = (1.0 - (double)cnt / param->fontArea) * 100.0;

		if (up < 0) up = 0;
		else if (up > 100.0) up = 100.0;
	}
}
void sFont::CheckPOCRModelSize(GeoModel & model, std::vector<int> angleStepId)
{
	int MxNum = 0;
	for (int i = 0; i < angleStepId.size(); i++)
	{
		if (angleStepId[i] > MxNum)
		{
			MxNum = angleStepId[i];
		}
	}
	if (model._Models[GeoModel::enmDepth::spZoom].size() <= MxNum
	 || model._Models[GeoModel::enmDepth::spHalf].size() <= MxNum
	 || model._Models[GeoModel::enmDepth::spOrg ].size() <= MxNum)
	{
		model.RePreprocess();
	}
}
	// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =




	int sFontList::getDataCount()
	{
		return 0;
	}
	bool sFontList::dataClear()
	{
		int listSz = _sfont.size();

		for (int i = 0; i < listSz; i++)
			_sfont[i]->dataClear();
		_sfont.clear();
		return true;
	}

	sFontList::sFontList(PsrModel * pParent)
	{
		_pParent = pParent;

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	sFontList::~sFontList()
	{
		_sfont.clear();

	g_pMManager->pem_delete_check(this);
	}

	bool sFontList::AddFont(CString sStr, std::vector<cv::Mat> img, std::vector<cv::Mat> Grayimg, int color, clipData* sClipData, int nThreshold, RECT TeachRect)
	{

		int TFontsz = img.size();
		//	int* characterCnt=new int[TFontsz];	//2017.05.31 shkim 중복문자 저장 안하는 구문
		// 
		// 
		// 	for(int i=0;i<TFontsz;i++)
		// 	{
		// 		characterCnt[i]=i;
		// 		for(int j=0;j<TFontsz;j++)
		// 		{
		// 			if(sStr[i] == sStr[j])
		// 			{
		// 				characterCnt[i] = j;
		// 			}
		// 		}
		// 	}

		_chfont = sStr;
		_teachPo.x = (TeachRect.left + TeachRect.right) / 2;
		_teachPo.y = (TeachRect.top + TeachRect.bottom) / 2;
	_sfont.clear();
	_sfont.reserve(img.size());
		for (int i = 0; i < img.size(); i++)
		{
			// 		if(characterCnt[i] != i)
			// 			continue;
		std::shared_ptr<sFont> fnt = std::make_shared<sFont>(this);
			fnt->_ForVal = sClipData->_ForVal;
			fnt->_backVal = sClipData->_backVal;
			fnt->_center_x = sClipData->center_x[i];
			fnt->_center_y = sClipData->center_y[i];
			fnt->_width = sClipData->width[i];
			fnt->_height = sClipData->height[i];
			fnt->_Threshold = nThreshold;
			fnt->Alloc(sStr.GetAt(i), Grayimg[i], img[i], color);
			_sfont.push_back(fnt);
		}

		return true;
	}

bool sFontList::AddFont_ReTeaching(CString sStr, std::vector<cv::Mat> img, std::vector<cv::Mat> Grayimg, int color, clipData* sClipData, int nThreshold, RECT TeachRect, std::vector<std::shared_ptr<sFont>> vFontList)
{
	int TFontsz = img.size();
	_chfont = sStr;
	_teachPo.x = (TeachRect.left + TeachRect.right) / 2;
	_teachPo.y = (TeachRect.top + TeachRect.bottom) / 2;

	vFontList;
	_sfont.clear();
	_sfont.reserve(_chfont.GetLength());
	for (int i = 0; i < _chfont.GetLength(); i++)
	{
		std::shared_ptr<sFont> fnt(new sFont(this));
		std::shared_ptr<sFont> selectfnt(new sFont(this));
		bool bSetptr = false;
		for (int fIndex = 0; fIndex < vFontList.size(); fIndex++)
		{
			if (_chfont[i] != vFontList[fIndex]->_ch)
				continue;
			if ((vFontList[fIndex]->_Img.cols > img[i].cols*0.8 && vFontList[fIndex]->_Img.cols < img[i].cols * 1.2) &&
				(vFontList[fIndex]->_Img.rows > img[i].rows*0.8 && vFontList[fIndex]->_Img.rows < img[i].rows * 1.2))
			{
				selectfnt = vFontList[fIndex];
				bSetptr = true;
				break;
			}
		}
		if (bSetptr)
		{
			fnt->_ForVal = sClipData->_ForVal;
			fnt->_backVal = sClipData->_backVal;
			fnt->_center_x = sClipData->center_x[i];
			fnt->_center_y = sClipData->center_y[i];
			fnt->_width = sClipData->width[i];
			fnt->_height = sClipData->height[i];
			fnt->_ch = selectfnt->_ch;
			fnt->_Img = selectfnt->_Img.clone();
			fnt->_color = selectfnt->_color;
			fnt->_backVal = selectfnt->_backVal;
			fnt->_ForVal = selectfnt->_ForVal;
			fnt->_Threshold = selectfnt->_Threshold;
			fnt->_ModelEdge = std::make_shared<GeoModel_POCR>(*selectfnt->_ModelEdge);
			fnt->_ModelDiff = std::make_shared<GeoModel_POCR>(*selectfnt->_ModelDiff);
			_sfont.push_back(fnt);
		}
		else
		{
			selectfnt->_ForVal = sClipData->_ForVal;
			selectfnt->_backVal = sClipData->_backVal;
			selectfnt->_center_x = sClipData->center_x[i];
			selectfnt->_center_y = sClipData->center_y[i];
			selectfnt->_width = sClipData->width[i];
			selectfnt->_height = sClipData->height[i];
			selectfnt->_Threshold = nThreshold;
			selectfnt->Alloc(sStr.GetAt(i), Grayimg[i], img[i], color);
			_sfont.push_back(selectfnt);
		}
	}
	return true;
}
	bool sFontList::SaveFile(CArchive &ar, bool bAddMode, CString path, int index, double resX, double resY)
	{
		CMemFile CMFile;
		CArchive m_ar(&CMFile, CArchive::store);
		int ByteSz(0);
		int nFontSz(_sfont.size());

		//2017.05.02 shkim	써야할 변수 저장.
		//이곳에 쓸것////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		m_ar << _chfont;
		m_ar << nFontSz;
		_OrgImage->setRes(resX, resY);
		_OrgImage->SaveFile(m_ar, false);
		m_ar << _teachPo.x;
		m_ar << _teachPo.y;
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		m_ar.Close();

		ByteSz = CMFile.GetLength();
		ar << ByteSz;
		BYTE * Buf = CMFile.Detach();
		ar.Write(Buf, ByteSz);
	//delete [] Buf;
	g_pMManager->pem_delete(Buf, true);
		CMFile.Close();
		for (int i = 0; i < nFontSz; i++)
		{
			_sfont[i]->SaveToFile(ar, resX, resY);
			if (bAddMode)
				_sfont[i]->ThrowFontImg(index + 1, i, path);
		}

		return true;
	}

	bool sFontList::LoadFileReadFont(CArchive &ar, CString path, int index, CString readfonts, CString destPath, double resX, double resY, bool bOrg)
	{

		int ByteSz(0);
		ar >> ByteSz;
		if (ByteSz == 0)
			return false;
		std::vector<BYTE> buf(ByteSz);
		ar.Read(&buf[0], ByteSz);

		CMemFile CMFile;
		CMFile.Attach(&buf[0], ByteSz);
		CArchive m_ar(&CMFile, CArchive::load, ByteSz);

		m_ar >> _chfont;
		int nFontSz(0);
		m_ar >> nFontSz;

		m_ar.Close();
		CMFile.Close();
		buf.clear();

		CString msg;
		cv::Mat fontlistImg;
		for (int i = 0; i < nFontSz; i++)
		{
		std::shared_ptr<sFont> fnt = std::make_shared<sFont>(this);

			fnt->LoadFromFile(ar, resX, resY, path);

			if (readfonts.IsEmpty() == false)
			{
				CString strFont;
				for (int i = 0; i < readfonts.GetLength(); i++)
				{
					strFont = readfonts.GetAt(i);

					if (strFont == fnt->_ch)
					{
						fnt->SaveFontImg(path, index, destPath, bOrg);
						break;
					}
				}
			}

			_sfont.push_back(fnt);
		}

		return true;
	}

	bool sFontList::LoadFile(CArchive &ar, CString path, int index, double resX, double resY, int fontColor)
	{
		try
		{

			int ByteSz(0);
			ar >> ByteSz;
			if (ByteSz == 0)
				return false;
			std::vector<BYTE> buf(ByteSz);
			ar.Read(&buf[0], ByteSz);

			CMemFile CMFile;
			CMFile.Attach(&buf[0], ByteSz);
			CArchive m_ar(&CMFile, CArchive::load, ByteSz);


			m_ar >> _chfont;
			int nFontSz(0);
			m_ar >> nFontSz;
			bool flag = true;

			if (m_ar.IsBufferEmpty() == FALSE)
			{
			_OrgImage = std::make_shared<GeoModel_POCR>();
				_OrgImage->LoadFile(m_ar, resX, resY);
				// 
// 		CString cutString,strPsr;
// 		CString spath;
// 		AfxExtractSubString( cutString, path, 0, '.');
// 		AfxExtractSubString( strPsr, path, 1, '.');
// 		int i=0;
// 		while(0 != strPsr.CompareNoCase(_T("psr")))
// 		{
// 			CString temp;
// 			temp.Format(_T("%s.%s"),cutString,strPsr);
// 			cutString.Format(_T("%s"),temp);
// 			i++;
// 			AfxExtractSubString( strPsr, path, i+1, '.');
// 		}
// 
// 		spath.Format(_T("%s@%d.bmp"),cutString,index);
// 
// 		cv::imwrite(std::string(CT2A(spath)),_OrgImage->Image());
// 		flag = false;
// 	}
// 	else
// 	{
// 		flag = true;
			}

			if (m_ar.IsBufferEmpty() == FALSE)
			{
				m_ar >> _teachPo.x;
				m_ar >> _teachPo.y;
			}

			m_ar.Close();
			CMFile.Close();
			buf.clear();

			CString msg;
			cv::Mat fontlistImg;
			CString spath;
			for (int i = 0; i < nFontSz; i++)
			{
			std::shared_ptr<sFont> fnt = std::make_shared<sFont>(this);

				fnt->LoadFromFile(ar, resX, resY, path);

				if (flag)
				{
					//				cv::Mat FontImg =fnt->CalcAvgForBack( fnt->_Img.clone(),fnt->_ModelEdge->Image());
					if (fnt->_ModelEdge == nullptr)
						return false;

					cv::Mat FontImg = fnt->_ModelEdge->Image();
				if (FontImg.empty())
					return false;
					if (i == 0)
					{
						fontlistImg = FontImg.clone();

						CString cutString, strPsr;
						AfxExtractSubString(cutString, path, 0, '.');
						AfxExtractSubString(strPsr, path, 1, '.');
						int i = 0;
						while (0 != strPsr.CompareNoCase(_T("psr")))
						{
							CString temp;
							temp.Format(_T("%s.%s"), cutString, strPsr);
							cutString.Format(_T("%s"), temp);
							i++;
							AfxExtractSubString(strPsr, path, i + 1, '.');
						}
						spath.Format(_T("%s@%d.bmp"), cutString, index);

						// 					DeleteFile(spath);
						// 					cv::imwrite(std::string(CT2A(spath)),fontlistImg);
					}
					else
					{
						int w = FontImg.cols + fontlistImg.cols;
						int h = FontImg.rows > fontlistImg.rows ? FontImg.rows : fontlistImg.rows;

						cv::Mat tmp = cv::Mat(h, w, CV_8UC1);

						tmp.setTo(0);

						fontlistImg.copyTo(tmp(cv::Rect(0, 0, fontlistImg.cols, fontlistImg.rows)));
						FontImg.copyTo(tmp(cv::Rect(fontlistImg.cols, 0, FontImg.cols, FontImg.rows)));

						// 					CString cutString;
						// 					AfxExtractSubString( cutString, path, 0, '.');
						//
						// 					CString spath;
						// 					spath.Format(_T("%s@%d.bmp"),cutString,index);
						// 					DeleteFile(spath);
						// 					cv::imwrite(std::string(CT2A(spath)),tmp);

						fontlistImg = tmp.clone();
					}
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				_sfont.push_back(fnt);
			}

			DeleteFile(spath);
		if (fontlistImg.empty())
			return false;
			std::vector<uchar> vBuf;
			cv::imencode(".bmp", fontlistImg, vBuf);
			SaveImageList(spath, vBuf);

		}
		catch (CMemoryException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

			csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}
		catch (CFileException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

			csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}
		catch (CException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

			csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}
		catch (cv::Exception & e)
		{
			CString csLog;

			csLog.Format(_T("PsrModel::SearchFont_%s"), CA2T(e.what()).m_psz);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}

		return true;
	}
	bool sFontList::DeleteFont(CString ch)
	{
		int fontsize = _sfont.size();

		for (int i = 0; i < fontsize; i++)
		{
			if (_sfont[i]->_ch == ch)
			{
				_sfont[i].~shared_ptr();
			}
		}

		return true;
	}

std::vector<std::shared_ptr<rstPSRModel>> sFontList::SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc,CString TargetFont, cPOCRInfoParam* vPOCRInspPtr,std::shared_ptr<GeoMatch_POCR> ptrEdge,CString sSaveParam, int nForeGround, algoPOCR_ParamContainer* param)
	{
		std::vector<std::shared_ptr<rstPSRModel>> rst;
		int fontsize = _sfont.size();
		int TFontsz = TargetFont.GetLength();
	//int* characterCnt=new int[TFontsz];
	int* characterCnt = g_pMManager->pem_new<int>(true, TFontsz, (PCHAR)__FUNCTION__, __LINE__);
	
		for (int i = 0; i < TFontsz; i++)
		{
			characterCnt[i] = 0;
			for (int j = 0; j < TFontsz; j++)
			{
				if (TargetFont[i] == TargetFont[j])
				{
					characterCnt[i]++;
				}
			}
			}

		GeoMatch_POCR gMatch;
		GeoResult res(1);
		gMatch.SetImgProcess(ImgProc->edge);
		gMatch.setProcDataSaveFlag(sSaveParam);
		gMatch.FindModel(*_OrgImage, res, false);

	if(res._Equality[0] < 40)
	{
		if (_teachPo.x <= 0 || _teachPo.y <= 0)
		{
			CString msg;
			msg.Format(_T("_Equality: %.3f (%.3f, %.3f) / _teachPo:(%d, %d)"),res._Equality[0],res._Center_y[0],res._Center_x[0],_teachPo.x,_teachPo.y);
			g_pMPTI->AddLog_OCR(msg);

		}
		else
		{
			res._Center_y[0] = RounD(_teachPo.x);
			res._Center_x[0] = RounD(_teachPo.y);
			}
		}

		for (int i = 0; i < fontsize; i++)
		{
			std::vector<std::shared_ptr<rstPSRModel>> tmp;
			//	int indx = TargetFont.Find(_sfont[i]->_ch);
			//	tmp = _sfont[i]->SearchFont(roiImg, vPOCRInspPtr, characterCnt[indx]);
		tmp = _sfont[i]->SearchFont(ImgProc,vPOCRInspPtr, _OrgImage, res,ptrEdge, param);
			// 		if(UseRoiImg)
			// 		{
			// 			for(int tmpI = 0;tmpI<tmp.size();tmpI++)
			// 			{
			// 				tmp[tmpI]->x +=(double)left;
			// 				tmp[tmpI]->y +=(double)top;
			// 			}
			// 
			// 		}
			if (tmp.size() > 0)
			{
				tmp[0]->Parent_x = (double)res._Center_y[0];
				tmp[0]->Parent_y = (double)res._Center_x[0];
				tmp[0]->Parent_Width = _OrgImage->Image().cols;
				tmp[0]->Parent_Height = _OrgImage->Image().rows;
			tmp[0]->angle = res._Angle[0];
			}

			rst.insert(rst.end(), tmp.begin(), tmp.end());
		}
		int rstSize = (int)rst.size() - 1;
		if (rstSize > 0)
		{
			std::vector<bool> vbLocationOK;
			vbLocationOK.resize(rstSize);
			for (int i = 0; i + 1 < rst.size(); i++)
			{
				float fCurx = rst[i + 1]->x - rst[i]->x;
				float fStdx = _sfont[i + 1]->_center_x - _sfont[i]->_center_x;
				float fMaxOffSet = fStdx * 0.2;
				vbLocationOK[i] = fMaxOffSet < std::abs(fStdx - fCurx);
			}
			for (int i = 0; i <= vbLocationOK.size(); i++)
			{
				int nL = i - 1 < 0 ? 0 : i - 1;
				int nR = i == vbLocationOK.size() ? vbLocationOK.size() - 1 : i;
				if (vbLocationOK[nL] && vbLocationOK[nR])
					rst[i]->score = 0;
			}
		}
	//delete [] characterCnt;
	g_pMManager->pem_delete(characterCnt, true);

		return rst;
	}


	bool sFontList::Preprocess()
	{
		for (int i = 0; i < _sfont.size(); i++)
		{
			_sfont[i]->Preprocess();
	}
	return true;
}
bool sFontList::RePreprocess()
{
	for (int i = 0; i < _sfont.size(); i++)
	{
		_sfont[i]->RePreprocess();
		}
		return true;
	}

	bool sFontList::LoadFile(CArchive &ar, CString path, double resX, double resY)
	{

		int ByteSz(0);
		ar >> ByteSz;
		if (ByteSz == 0)
			return false;
		std::vector<BYTE> buf(ByteSz);
		ar.Read(&buf[0], ByteSz);

		CMemFile CMFile;
		CMFile.Attach(&buf[0], ByteSz);
		CArchive m_ar(&CMFile, CArchive::load, ByteSz);


		m_ar >> _chfont;
		int nFontSz(0);
		m_ar >> nFontSz;


		if (m_ar.IsBufferEmpty() == FALSE)
		{
		_OrgImage = std::make_shared<GeoModel_POCR>();
			_OrgImage->LoadFile(m_ar, resX, resY);
			// 
			// 		CString cutString,strPsr;
			// 		CString spath;
			// 		AfxExtractSubString( cutString, path, 0, '.');
			// 		AfxExtractSubString( strPsr, path, 1, '.');
			// 		int i=0;
			// 		while(0 != strPsr.CompareNoCase(_T("psr")))
			// 		{
			// 			CString temp;
			// 			temp.Format(_T("%s.%s"),cutString,strPsr);
			// 			cutString.Format(_T("%s"),temp);
			// 			i++;
			// 			AfxExtractSubString( strPsr, path, i+1, '.');
			// 		}
			// 
			// 		spath.Format(_T("%s@%d.bmp"),cutString,index);
			// 
			// 		cv::imwrite(std::string(CT2A(spath)),_OrgImage->Image());
			// 		flag = false;
			// 	}
			// 	else
			// 	{
			// 		flag = true;
		}

		if (m_ar.IsBufferEmpty() == FALSE)
		{
			m_ar >> _teachPo.x;
			m_ar >> _teachPo.y;
		}

		m_ar.Close();
		CMFile.Close();
		buf.clear();

		CString msg;
		cv::Mat fontlistImg;
		for (int i = 0; i < nFontSz; i++)
		{
		std::shared_ptr<sFont> fnt = std::make_shared<sFont>(this);
			fnt->LoadFromFile(ar, resX, resY);
			_sfont.push_back(fnt);
		}
		return true;
	}

	bool sFontList::SaveImageList(clipData* sClipData)
	{
	_OrgImage = std::make_shared<GeoModel_POCR>();
		_OrgImage->Alloc(sClipData->_img);
		_OrgImage->setAngleRange(true, 6, 6, 2);
		_OrgImage->Preprocess();
		//	fontList->_OrgImage->MakeModel(Grayimg, img, 0, 100, 20 , 2);

		return true;
	}


	void sFontList::SearchFontDraw(cv::Mat src, bool rot, UCHAR * ucArrDstImgMask)
	{
		std::vector<std::shared_ptr<rstPSRModel>> rst;
		//	int fontsize=_sfont.size();

		cv::Mat draw(src.rows, src.cols, CV_8UC1, ucArrDstImgMask, src.cols);

		GeoMatch_POCR gMatch;
		GeoResult res(1), rotRes(1);
		std::shared_ptr<GeoModel_POCR> rotModel = std::make_shared<GeoModel_POCR>();
		rotModel->CopyOf(*_OrgImage, 180);
		cv::Mat Edge_pyr, Edge_half, Edge_img = src.clone();
		int nPyrDownStep(0);
		do
		{
			Edge_pyr.release();
			cv::resize(Edge_img, Edge_pyr, cv::Size(Edge_img.cols*0.5, Edge_img.rows*0.5));
			Edge_img = Edge_pyr;
			nPyrDownStep++;
			if (nPyrDownStep == 1)
				Edge_half = Edge_pyr;

		} while (nPyrDownStep < _OrgImage->_pyrDownStep);

		std::shared_ptr<ImgProcessing> ImgProc;
		ImgProc = std::make_shared<ImgProcessing>();
		ImgProc->_ImgProcess(src, Edge_half, Edge_pyr, _OrgImage->_pyrDownStep, (GeoModel::enmMatchAlgo)_OrgImage->getMatchAlgo());
		gMatch.SetImgProcess(ImgProc);
		gMatch.FindModel(*_OrgImage, res, false);
		gMatch.FindModel(*rotModel, rotRes, false);

		if (res._Equality[0] >= rotRes._Equality[0] || rot == true)
			gMatch.DrawContours(draw, *_OrgImage, res, cv::Scalar(255), 1, _sfont[0]->_ForVal > _sfont[0]->_backVal);
		else
			gMatch.DrawContours(draw, *rotModel, rotRes, cv::Scalar(255), 1, _sfont[0]->_ForVal > _sfont[0]->_backVal);
#ifdef _DEBUG
		cv::imwrite("D:\\testimage\\MaskImg_draw.bmp", draw);
#endif
	}

	void sFontList::SearchFontDraw3Ch(cv::Mat src, bool rot, UCHAR * ucArrDstImgMask)
	{
		std::vector<std::shared_ptr<rstPSRModel>> rst;
		//	int fontsize=_sfont.size();

		cv::Mat draw(src.rows, src.cols, CV_8UC3, ucArrDstImgMask);
		for (int r = 0; r < src.rows; r++)
		{
			uchar* sPtr = src.ptr(r);
			uchar* dPtr = draw.ptr(r);
			for (int c = 0; c < src.cols; c++)
			{
				dPtr[c * 3] = sPtr[c];
				dPtr[c * 3 + 1] = sPtr[c];
				dPtr[c * 3 + 2] = sPtr[c];
			}
		}


		GeoMatch_POCR gMatch;
		GeoResult res(1), rotRes(1);
		std::shared_ptr<GeoModel_POCR> rotModel = std::make_shared<GeoModel_POCR>();
		rotModel->CopyOf(*_OrgImage, 180);
		cv::Mat Edge_pyr, Edge_half, Edge_img = src.clone();
		int nPyrDownStep(0);
		do
		{
			Edge_pyr.release();
			cv::resize(Edge_img, Edge_pyr, cv::Size(Edge_img.cols*0.5, Edge_img.rows*0.5));
			Edge_img = Edge_pyr;
			nPyrDownStep++;
			if (nPyrDownStep == 1)
				Edge_half = Edge_pyr;

		} while (nPyrDownStep < _OrgImage->_pyrDownStep);

		std::shared_ptr<ImgProcessing> ImgProc;
		ImgProc = std::make_shared<ImgProcessing>();
		ImgProc->_ImgProcess(src, Edge_half, Edge_pyr, _OrgImage->_pyrDownStep, (GeoModel::enmMatchAlgo)_OrgImage->getMatchAlgo());
		gMatch.SetImgProcess(ImgProc);
		gMatch.FindModel(*_OrgImage, res, false);
		gMatch.FindModel(*rotModel, rotRes, false);

		if (res._Equality[0] >= rotRes._Equality[0] || rot == true)
			gMatch.DrawContours(draw, *_OrgImage, res, cv::Scalar(0, 0, 255), 1, _sfont[0]->_ForVal > _sfont[0]->_backVal);
		else
			gMatch.DrawContours(draw, *rotModel, rotRes, cv::Scalar(0, 0, 255), 1, _sfont[0]->_ForVal > _sfont[0]->_backVal);
#ifdef _DEBUG
		cv::imwrite("D:\\testimage\\MaskImg_draw.bmp", draw);
#endif
	}


	void sFontList::SaveImageList(CString sPath, std::vector<uchar>& vBuf)
	{
	if (g_pInspMng->m_bPsrUpdateChecking)
		return;

		CFile file;
		CFileException fe;
		CMemFile CMFile;
		if (file.Open(sPath, CFile::modeCreate | CFile::modeWrite /*| CFile::shareDenyRead || CFile::shareDenyWrite*/, NULL, &fe) == FALSE)
			return;

		CArchive ar(&file, CArchive::store);

		try
		{
			for (int i = 0; i < vBuf.size(); i++)
				ar << vBuf[i];
		}
		catch (...)
		{
			AfxMessageBox(_T("PsrModel_Save fail"));
		}

		ar.Close();
		file.Close();
	}
void sFontList::CheckPOCRModelSize(GeoModel & model, std::vector<int> angleStepId)
{
	if (model._Models[GeoModel::enmDepth::spZoom].size() != angleStepId.size()
	 || model._Models[GeoModel::enmDepth::spHalf].size() != angleStepId.size()
	 || model._Models[GeoModel::enmDepth::spOrg ].size() != angleStepId.size())
	{
		model.RePreprocess();
	}
}
void sFontList::ConvertFileVersion()
{
	bool r = false;
	for (int i = 0; i < _sfont.size(); i++)
	{
		bool result = _sfont[i]->ConvertFileVersion();
	}
}
	// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

	PsrModel::PsrModel(void)
	{
		Preprocessed = false;
		for (int i = 0; i < 256; i++)
			m_dStdAddFontScore[i] = 50;

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}


	PsrModel::~PsrModel(void)
	{
		_sfList.clear();

	g_pMManager->pem_delete_check(this);
	}
	bool PsrModel::dataClear()
	{
		int listSz = _sfList.size();
		for (int i = 0; i < listSz; i++)
			_sfList[i]->dataClear();
		_sfList.clear();
		return true;
	}
bool PsrModel::SetPilParam(Im::PIL_ID* milApp, Im::PIL_ID* milSys)
{
	m_milSys = milSys;
	m_milApp = milApp;
	return true;
}
	bool PsrModel::AddFont(CString sStr, cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int thresh, int listIndex, cPOCRInfoParam* vPOCRInspPtr, RECT TeachRect, RECT* FontRect, HWND Handle)
	{
#if ImageSave
		cv::imwrite("D:\\testimage\\img.bmp", img);
		cv::imwrite("D:\\testimage\\Grayimg.bmp", Grayimg);
		cv::imwrite("D:\\testimage\\BinImage.bmp", BinImage);
#endif
		CharClipper clip;

		cv::Mat dstSavImg2;
		cv::Mat clrImg2[3] = { BinImage, cv::Mat::zeros(cv::Size(BinImage.cols, BinImage.rows), BinImage.type()),  Grayimg };
		cv::merge(clrImg2, 3, dstSavImg2);

#if USE_DOTFONT
		//////////////////////////////////////////////////////////////////////////
		//img->드로우 수정된 image
		//Grayimg->조명 image
		//BinImage->조명->threshold image

		//x축 projection
		//sStr 갯수 만큼 나누기.
		//큰 image에 붙여넣기
		//morphology 수행.(y축 프로젝션 해서 morph size 구하기...)
		//나눈 image 붙이기.



		//안된 사항.
		//sStr 갯수 만큼 나누기. - ProJectionImageSplit
		//morphology 수행.(y축 프로젝션 해서 morph size 구하기...) - ProJectionImageSplitMorPh
		//ProJection 양 쪽 2pixel 씩 보고 급감 한다면, 2Pixel의 대한 평균으로 보정.
		bCkCircle = false;
		if (bCkCircle)
		{
			std::vector<int> vProjection;
			std::vector<cv::Mat> vSrc;
			std::vector<cv::Mat> vResultImg;
			std::vector<CPoint> vROIPoint;
			cv::Mat Mergeimg;

			ProJection(img, vProjection);
			bool bCheck = ProJectionImageSplit(img, vProjection, sStr.GetLength(), vSrc, vROIPoint);

			if (bCheck)
			{
				ProJectionImageSplitMorPh(vSrc, vResultImg);
				ProJectionImageMerge(vResultImg, Mergeimg, 75, img, vROIPoint);

				if (img.empty() != true) {
					img.release();
					Mergeimg.copyTo(img);
				}
				if (BinImage.empty() != true) {
					BinImage.release();
					Mergeimg.copyTo(BinImage);
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////
#endif

		cv::Mat dstSavImg1;
		cv::Mat clrImg1[3] = { BinImage, cv::Mat::zeros(cv::Size(BinImage.cols, BinImage.rows), BinImage.type()),  Grayimg };
		cv::merge(clrImg1, 3, dstSavImg1);

		bool flag = false;
		if (FontRect)
		flag = clip.searchArea(sStr.GetLength(), img, Grayimg, BinImage, color, thresh, FontRect, m_milApp, m_milSys);
		else
			flag = clip.searchArea(sStr.GetLength(), img, Grayimg, BinImage, color, thresh);
		if (!flag)
			return false;
		std::vector<cv::Mat> clipImg = clip.MatList;
		std::vector<cv::Mat> clipGrayImg = clip.MatOrgImgList;

	std::shared_ptr<sFontList> fontList = std::make_shared<sFontList>(this);
		fontList->dataClear();
		fontList->AddFont(sStr, clipImg, clipGrayImg, color, clip.sClipData, thresh, TeachRect);

		fontList->SaveImageList(clip.sClipData);

#if ImageSave
		cv::imwrite("D:\\testimage\\_OrgImage.bmp", fontList->_OrgImage->Image());

#endif
		_sfList.push_back(fontList);
		nfontListCnt = _sfList.size();

		if (FontRect == NULL)
			clip.sClipData->reset();
		Preprocessed = false;

		//shkim 2022.05.02 검사삭제(AI로 검사대체 가능할것으로 판단)
		return true;
		//2017.06.30 shkim 검사해본다.
		std::shared_ptr<ImgProcessing_POCR> ImgProc;
		ImgProc = std::make_shared<ImgProcessing_POCR>();
		ImgProc->_ImgProcess(Grayimg, fontList->_OrgImage->_pyrDownStep);

	std::vector<std::shared_ptr<rstPSRModel>> rst = SearchFont(ImgProc, sStr, vPOCRInspPtr, _T(""), m_dStdAddFontScore, color == 1 ? 0 : 1);

		int rstsz = rst.size();
		if (rstsz == 0)
		{
			_sfList.pop_back();
			nfontListCnt = _sfList.size();
			return false;
		}
		// 	_sfList[_sfList.size() - 1]->_teachPo.x += rst[0]->Parent_x;
		// 	_sfList[_sfList.size() - 1]->_teachPo.y += rst[0]->Parent_y;

		std::vector<std::vector<std::shared_ptr<rstPSRModel>>> arr;
		arr.clear();
		int nStrLength = sStr.GetLength();
		arr.resize(nStrLength);
		for (int fontIdx = 0; fontIdx < nStrLength; fontIdx++)		//2017.06.30 shkim // 문자열로 분류
		{

			for (int i = 0; i < rstsz; i++)
			{
				if (rst[i]->ch == sStr[fontIdx])
				{
					arr[fontIdx].push_back(rst[i]);
				}
			}
		}

		for (int fontIdx = 0; fontIdx < nStrLength; fontIdx++)		//2017.06.30 shkim // 점수기준 내림차순 정렬
		{
			int cnt = arr[fontIdx].size();
			for (int i = 0; i < cnt; i++)
			{
				for (int j = 0; j < cnt; j++)
				{
					if (arr[fontIdx][i]->score > arr[fontIdx][j]->score)
					{
						std::shared_ptr<rstPSRModel> tmp = arr[fontIdx][i];
						arr[fontIdx][i] = arr[fontIdx][j];
						arr[fontIdx][j] = tmp;
					}
				}
			}
		}
		for (int i = 0; i < nStrLength; i++)
		{
			if (arr[i].size() > 0)
			{
				if (arr[i][0]->score < 50)
				{
					_sfList.pop_back();
					nfontListCnt = _sfList.size();
					return false;
				}
			}
		}

		return true;
	}
	bool PsrModel::AddFont_NG(cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int listIndex, cPOCRInfoParam* vPOCRInspPtr, CString ImgPath, int nThreshold, RECT TeachRect)
	{
		//	dataClear(); // for new font
		RstAlgoPOCR result;
		result.reset();
#if ImageSave
		cv::imwrite("D:\\testimage\\img.bmp", img);
		cv::imwrite("D:\\testimage\\Grayimg.bmp", Grayimg);
		cv::imwrite("D:\\testimage\\BinImage.bmp", BinImage);
#endif
		std::vector<std::shared_ptr<rstPSRModel>> rst;
	std::shared_ptr<GeoMatch_POCR> ptrEdge = std::make_shared<GeoMatch_POCR>();
		std::shared_ptr<ImgProcessing_POCR> ImgProc;
		ImgProc = std::make_shared<ImgProcessing_POCR>();
		ImgProc->_ImgProcess(Grayimg, _sfList[0]->_OrgImage->_pyrDownStep);
	rst= _sfList[0]->SearchFont(ImgProc,_sfList[0]->_chfont, vPOCRInspPtr,ptrEdge,_T(""), color);

		if (rst.size() > 0)
		{
			bool flag = false;
			for (int i = 0; i < rst.size(); i++)
			{
				if (rst[i]->score < 50)
				{
					if (flag)
						return false;
					//return false;
					float center_x = Grayimg.cols / 2 + 0.5;
					float center_y = Grayimg.rows / 2 + 0.5;

					cv::Point2f po(center_x, center_y);
					cv::Mat matro = cv::getRotationMatrix2D(po, 180, 1);
					cv::warpAffine(Grayimg, Grayimg, matro, Grayimg.size());
					cv::warpAffine(img, img, matro, img.size());
					cv::warpAffine(BinImage, BinImage, matro, BinImage.size());

#if ImageSave
					cv::imwrite("D:\\testimage\\img2.bmp", img);
					cv::imwrite("D:\\testimage\\Grayimg2.bmp", Grayimg);
					cv::imwrite("D:\\testimage\\BinImage2.bmp", BinImage);
#endif

					std::shared_ptr<ImgProcessing_POCR> ImgProc;
					ImgProc = std::make_shared<ImgProcessing_POCR>();
					ImgProc->_ImgProcess(Grayimg, _sfList[0]->_OrgImage->_pyrDownStep);
				rst= _sfList[0]->SearchFont(ImgProc,_sfList[0]->_chfont, vPOCRInspPtr,ptrEdge,_T(""), color);
					if (rst[i]->score < 50)
						return false;
					flag = true;
				}
			}

			result.m_nCharCount = _sfList[0]->_chfont.GetLength();

			result.ModelX = rst[0]->Parent_x;
			result.ModelX = rst[0]->Parent_x;
			result.ModelY = rst[0]->Parent_y;
			result.ModelWidth = rst[0]->Parent_Width;
			result.ModelHeight = rst[0]->Parent_Height;

			for (int i = 0; i < result.m_nCharCount; i++)
			{
				result.charX[i] = rst[i]->x;
				result.charY[i] = rst[i]->y;
				result.charWidth[i] = rst[i]->nCharWidth;
				result.charHeight[i] = rst[i]->nCharHeight;
			}
		}
		else return false;

		CharClipper clip;
		cv::Rect re(result.ModelX - result.ModelWidth / 2, result.ModelY - result.ModelHeight / 2, result.ModelWidth, result.ModelHeight);
		cv::Mat img_ = img(re);
		cv::Mat Grayimg_ = Grayimg(re);
		cv::Mat BinImage_ = BinImage(re);
		bool flag = clip.searchArea(_sfList[0]->_chfont.GetLength(), img_, Grayimg_, BinImage_, color, nThreshold);
		if (!flag)
			return false;
		std::vector<cv::Mat> clipImg = clip.MatList;
		std::vector<cv::Mat> clipGrayImg = clip.MatOrgImgList;

		TeachRect.left += re.x;
		TeachRect.top += re.y;
	std::shared_ptr<sFontList> fontList = std::make_shared<sFontList>(this);
		fontList->dataClear();
		fontList->AddFont(_sfList[0]->_chfont, clipImg, clipGrayImg, color, clip.sClipData, nThreshold, TeachRect);

		fontList->SaveImageList(clip.sClipData);

#if ImageSave
		cv::imwrite("D:\\testimage\\_OrgImage.bmp", fontList->_OrgImage->Image());

#endif
		_sfList.push_back(fontList);
		nfontListCnt = _sfList.size();

		clip.sClipData->reset();
		Preprocessed = false;

		// 	//2017.06.30 shkim 검사해본다.
	//	std::vector<std::shared_ptr<rstPSRModel>> rst = SearchFont(Grayimg.data,Grayimg.cols,Grayimg.rows,_sfList[0]->_chfont,vPOCRInspPtr);
	rst = fontList->SearchFont(ImgProc,_sfList[0]->_chfont, vPOCRInspPtr,ptrEdge,_T(""), color);

		int rstsz = rst.size();
		if (rstsz == 0)
		{
			_sfList.pop_back();
			nfontListCnt = _sfList.size();
			return false;
		}

		std::vector<std::vector<std::shared_ptr<rstPSRModel>>> arr;
		arr.clear();
		int nStrLength = _sfList[0]->_chfont.GetLength();
		arr.resize(nStrLength);
		for (int fontIdx = 0; fontIdx < nStrLength; fontIdx++)		//2017.06.30 shkim // 문자열로 분류
		{

			for (int i = 0; i < rstsz; i++)
			{
				if (rst[i]->ch == _sfList[0]->_chfont[fontIdx])
				{
					arr[fontIdx].push_back(rst[i]);
				}
			}
		}

		for (int fontIdx = 0; fontIdx < nStrLength; fontIdx++)		//2017.06.30 shkim // 점수기준 내림차순 정렬
		{
			int cnt = arr[fontIdx].size();
			for (int i = 0; i < cnt; i++)
			{
				for (int j = 0; j < cnt; j++)
				{
					if (arr[fontIdx][i]->score > arr[fontIdx][j]->score)
					{
						std::shared_ptr<rstPSRModel> tmp = arr[fontIdx][i];
						arr[fontIdx][i] = arr[fontIdx][j];
						arr[fontIdx][j] = tmp;
					}
				}
			}
		}
		for (int i = 0; i < nStrLength; i++)
		{
			if (arr[i].size() > 0)
			{
				if (arr[i][0]->score < 50)
				{
					_sfList.pop_back();
					nfontListCnt = _sfList.size();
					return false;
				}
			}
		}

		cv::imwrite(std::string(CT2A(ImgPath)), fontList->_OrgImage->Image());
		return true;
	}
bool PsrModel::AddFont_ReTeaching(CString sStr, cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int thresh, int listIndex, cPOCRInfoParam* vPOCRInspPtr, RECT TeachRect, RECT* FontRect)
{
#if ImageSave
	cv::imwrite("D:\\testimage\\img.bmp", img);
	cv::imwrite("D:\\testimage\\Grayimg.bmp", Grayimg);
	cv::imwrite("D:\\testimage\\BinImage.bmp", BinImage);
#endif
	CharClipper clip;

	cv::Mat dstSavImg2;
	cv::Mat clrImg2[3] = { BinImage, cv::Mat::zeros(cv::Size(BinImage.cols, BinImage.rows), BinImage.type()),  Grayimg };
	cv::merge(clrImg2, 3, dstSavImg2);
	cv::Mat dstSavImg1;
	cv::Mat clrImg1[3] = { BinImage, cv::Mat::zeros(cv::Size(BinImage.cols, BinImage.rows), BinImage.type()),  Grayimg };
	cv::merge(clrImg1, 3, dstSavImg1);
	bool flag = clip.searchArea(sStr.GetLength(), img, Grayimg, BinImage, color, thresh, FontRect, m_milApp, m_milSys);
	if (!flag)
		return false;
	std::vector<cv::Mat> clipImg = clip.MatList;
	std::vector<cv::Mat> clipGrayImg = clip.MatOrgImgList;
	std::vector<std::shared_ptr<sFont>> vFontList;
	vFontList.clear();
	CString sTargetStr;
	sTargetStr.Format(_T("%s"), sStr);
	for (int i = 0; i < _sfList.size(); i++)
	{
		for (int f = 0; f < _sfList[i]->_sfont.size(); f++)
		{
			int nFontidx = sTargetStr.Find(_sfList[i]->_sfont[f]->_ch);
			if (nFontidx != -1 && color == _sfList[i]->_sfont[f]->_color)
			{
				vFontList.push_back(_sfList[i]->_sfont[f]);
				sTargetStr.Delete(nFontidx, 1);
			}
		}
	}
	bool bFontEx = false;
	if (sTargetStr.IsEmpty())
		bFontEx = true;
	std::shared_ptr<sFontList> fontList(new sFontList(this));
	fontList->dataClear();
	if (bFontEx)
		fontList->AddFont_ReTeaching(sStr, clipImg, clipGrayImg, color, clip.sClipData, thresh, TeachRect, vFontList);
	else
		fontList->AddFont(sStr, clipImg, clipGrayImg, color, clip.sClipData, thresh, TeachRect);
	fontList->SaveImageList(clip.sClipData);
#if ImageSave
	cv::imwrite("D:\\testimage\\_OrgImage.bmp", fontList->_OrgImage->Image());
#endif
	_sfList.push_back(fontList);
	nfontListCnt = _sfList.size();
	Preprocessed = false;

	return true;
}
	bool PsrModel::Save(CString sPath, bool bAddMode, double resX, double resY)
	{
		CFile file;
		CFileException fe;
		CMemFile CMFile;
		if (file.Open(sPath, CFile::modeCreate | CFile::modeWrite /*| CFile::shareDenyRead || CFile::shareDenyWrite*/, NULL, &fe) == FALSE)
			return false;

		CArchive ar(&file, CArchive::store);
		CString header;

		nfontListCnt = _sfList.size();
		try
		{
			CString sName(_T("pemtronpsr"));
			int version(1);


			ar << sName;
			ar << version;

			SaveHeader(ar);

			for (int i = 0; i < nfontListCnt; i++)
			{
				_sfList[i]->SaveFile(ar, (bAddMode&&nfontListCnt - 1 == i), sPath, i, resX, resY);
			}

			// 		datacnt = dataCount();
			// 		ar<<datacnt;
			//
			// 		for(int i=0;i<spsr.nfontListCnt;i++)
			// 		{
			// 			sName.Format(_T("fontListCnt%d|"),i);
			// 			ar<<sName;
			// 			int cInt = sfList.chfont.GetLength();
			// 			ar<<cInt;
			// 			ar<<sfList.chfont;
			// 			for(int j=0;j<cInt;j++)
			// 			{
			// 				ar<<sfont.at(j)._ch;
			// 
			// 				int fontImgsize = sizeof(sfont.at(j).Img.data);
			// 				cv::Size imSize = sfont.at(j).Img.size();
			// 				sName.Format(_T("fontImgsizeX%d|"),j);
			// 				ar<<sName;
			// 				ar<<imSize.width;
			// 				sName.Format(_T("fontImgsizeY%d|"),j);
			// 				ar<<sName;
			// 				ar<<imSize.height;
			// 				sName.Format(_T("fontImgCh%d|"),j);
			// 				ar<<sName;
			// 				ar<<sfont.at(j).Img.channels();
			// 
			// 				sName.Format(_T("fontImgsize%d|"),j);
			// 				ar<<sName;
			// 				ar<<fontImgsize;
			// 
			// 
			// 				//Image save
			// 				sName.Format(_T("fontImgdata%d|"),j);
			// 				ar<<sName;
			// 
			// 				for (int y=0; y<imSize.height; y++)
			// 				{
			// 					uchar * ptr = sfont.at(j).Img.ptr(y);
			// 					for (int x=0; x<imSize.width; x++)
			// 						ar << ptr[x];
			// 				}
			// 			}
			// 		}

		}
		catch (...)
		{
			AfxMessageBox(_T("PsrModel_Save fail"));
		}

		ar.Close();
		file.Close();
		return true;
	}

	bool PsrModel::SaveHeader(CArchive &ar)
	{
		CMemFile CMFile;
		CArchive m_ar(&CMFile, CArchive::store);
		int ByteSz(0);

		//2017.05.02 shkim	써야할 변수 저장.
		//이곳에 쓸것////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		m_ar << nfontListCnt;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		m_ar.Close();

		ByteSz = CMFile.GetLength();
		ar << ByteSz;
		BYTE * Buf = CMFile.Detach();
		ar.Write(Buf, ByteSz);
	//delete [] Buf;
	g_pMManager->pem_delete(Buf, true);
		CMFile.Close();

		return true;
	}

	bool PsrModel::Load(CString sPath, int fontColor, double resX, double resY, CString readfonts, CString destPath, bool bOrg)
	{
		dataClear();
		int nLine = __LINE__;
		CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
		if (extension != _T(".psr"))
			return false;

	CFile file;
	CFileException fe;
	if(file.Open(sPath, CFile::modeRead, NULL, &fe)==FALSE)
		return false;

		CArchive ar(&file, CArchive::load);


		try
		{


			// 헤더
			CString sName(_T("pemtronpsr")), sReadName;
			ar >> sReadName;
			if (sReadName != sName)
				return false;
			int version(1);

			int totByte(0);

			ar >> version;

			LoadHeader(ar);


			for (int i = 0; i < nfontListCnt; i++)
			{
			std::shared_ptr<sFontList> tmpflist = std::make_shared<sFontList>(this);

				nLine = __LINE__;
				bool bLoadOK = false;
				if (readfonts.IsEmpty() == true)
					bLoadOK = tmpflist->LoadFile(ar, sPath, i, resX, resY, fontColor);
				else
					bLoadOK = tmpflist->LoadFileReadFont(ar, sPath, i, readfonts, destPath, resX, resY, bOrg);

				nLine = __LINE__;
				if (bLoadOK)
					_sfList.push_back(tmpflist);
			}
		if (nfontListCnt != _sfList.size())
			nfontListCnt = _sfList.size();
		}
		catch (CMemoryException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch(CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch(...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::LoadFile_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::LoadFile_UnknownError %d"), nLine);
		g_pMPTI->AddLog(csLog);
		}

		ar.Close();
		file.Close();

		return true;
	}
	bool PsrModel::Load(CString sPath, double resX, double resY)
	{
		dataClear();
		int nLine = __LINE__;

		CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
		if (extension != _T(".psr"))
			return false;


		CFile file;
		CFileException fe;
		if (file.Open(sPath, CFile::modeRead /*| CFile::shareDenyRead || CFile::shareDenyWrite*/, NULL, &fe) == FALSE)
			return false;
		if (file.GetLength() < 1)
			return false;
		CArchive ar(&file, CArchive::load);
	if( ext::InspRoot_Server::get()->_Tools.size() != 0)
		ext::InspRoot_Server::get()->ExtModelAdd(&file, sPath);


		try
		{

			nLine = __LINE__;
			// 헤더
			CString sName(_T("pemtronpsr")), sReadName;

			ar >> sReadName;
			if (sReadName != sName)
				return false;
			int version(1);

			int totByte(0);

			ar >> version;
			nLine = __LINE__;
			LoadHeader(ar);

			nLine = __LINE__;
			for (int i = 0; i < nfontListCnt; i++)
			{
			std::shared_ptr<sFontList> tmpflist = std::make_shared<sFontList>(this);
				bool bLoadOK = false;
				bLoadOK = tmpflist->LoadFile(ar, sPath, resX, resY);

				if (bLoadOK)
					_sfList.push_back(tmpflist);
		}
		//RePreprocess();
	}
	catch (CMemoryException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (cv::Exception& e)
	{
		CString csLog;
		csLog.Format(_T("PsrModel::LoadFile_%s"), CA2T(e.what()).m_psz);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch(...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::LoadFile_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::LoadFile_UnknownError %d"), nLine);
		g_pMPTI->AddLog(csLog);
	}

	ar.Close();
	file.Close();

	return true;
}
bool PsrModel::ExtLoad(int nIndex, double resX, double resY)
{
	dataClear();
	int nLine = __LINE__;
	ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_POCRModelCtrl.First()[nIndex];
	CString sPath(pCtrl.sFilePath);
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if (extension != _T(".psr"))
		return false;
	CMemFile file;
	CArchive arStore(&file, CArchive::store); // 공유메모리에서 가져와 셋팅.
	arStore.Write(ext::InspRoot_Server::get()->_POCRModelBuffer.Lock(pCtrl.img), pCtrl.img.imgSz/* static_cast<UINT>(fileSize)*/);
	arStore.Close();
	file.Flush();
	file.SeekToBegin();
	CArchive ar(&file, CArchive::load);
	try
	{
		nLine = __LINE__;
		CString sName(_T("pemtronpsr")), sReadName;
		ar >> sReadName;
		if (sReadName != sName)
			return false;
		int version(1);

		int totByte(0);

		ar >> version;
		nLine = __LINE__;
		LoadHeader(ar);

		nLine = __LINE__;
		for (int i = 0; i < nfontListCnt; i++)
		{
			std::shared_ptr<sFontList> tmpflist = std::make_shared<sFontList>(this);
			bool bLoadOK = false;
			bLoadOK = tmpflist->LoadFile(ar, sPath, resX, resY);

			if (bLoadOK)
				_sfList.push_back(tmpflist);
		}
	}
	catch (CMemoryException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		//g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		//g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		//g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::LoadFile_%s"), szMsg);
		//g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (cv::Exception& e)
	{
		CString csLog;
		csLog.Format(_T("PsrModel::LoadFile_%s"), CA2T(e.what()).m_psz);
		//g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::LoadFile_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::LoadFile_UnknownError %d"), nLine);
		//g_pMPTI->AddLog(csLog);
	}

	ar.Close();
	file.Close();

	return true;
}
	bool PsrModel::LoadHeader(CArchive &ar)
	{
		int ByteSz(0);
		ar >> ByteSz;
		std::vector<BYTE> buf(ByteSz);
		ar.Read(&buf[0], ByteSz);

		CMemFile CMFile;
		CMFile.Attach(&buf[0], ByteSz);
		CArchive m_ar(&CMFile, CArchive::load, ByteSz);


		m_ar >> nfontListCnt;

		m_ar.Close();
		CMFile.Close();
		buf.clear();

		return true;
	}


	int PsrModel::dataCount()
	{
		int cnt(0);

		int headerCnt(2);//name, ver
		int FontListCnt(4); //fontListCnt2,fontListToByte2

		int FontCnt(0);

		for (int i = 0; i < nfontListCnt; i++)
		{
			cnt += _sfList[i]->getDataCount();
		}
		cnt += headerCnt;
		cnt += FontListCnt;
		cnt += FontCnt;
		return cnt;
	}

	bool PsrModel::deleteFont(CString ch, int index)
	{
		try
		{
			_sfList[index]->DeleteFont(ch);
			Preprocessed = false;
		}
		catch (CMemoryException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::deleteFont_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::deleteFont_UnknownError"));
		g_pMPTI->AddLog(csLog);
		}

		return true;
	}

	bool PsrModel::deleteFontList(int index)
	{
		try
		{

			if (_sfList.size() == 0)
			{
				return false;
			}
			else if (_sfList.size() == 1)
			{
				_sfList.clear();
			}
			else
			{
				if (index < 0)
				{
					for (int i = 0; i < _sfList.size(); i++)
					{
						_sfList[i]->dataClear();
					}
					_sfList.clear();

				}
				else
				{
					_sfList[index]->dataClear();
					_sfList.erase(_sfList.begin() + index);
				}
			}
			Preprocessed = false;
	}
	catch (CMemoryException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::deleteFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::deleteFont_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::deleteFont_UnknownError"));
		g_pMPTI->AddLog(csLog);
	}

	return true;
	}


std::vector<std::shared_ptr<rstPSRModel>> PsrModel::SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc,CString Targetfont, cPOCRInfoParam* vPOCRInspPtr,CString sSaveParam, double* dStdScore, int nForeGround, algoPOCR_ParamContainer* param)
{
	try
	{
		rst.clear();
// 		cv::Mat srcImg(height, width, CV_8UC1);
// 		for (int y=0; y<height; y++)
// 			memcpy(srcImg.ptr(y), &userSrc[y*width], width);
// #if ImageSave
// 		cv::imwrite("D:\\testimage\\workimage\\POCRSrc.bmp",srcImg);
// #endif
// 		if(!isPreprocess())
// 			Preprocess();
		//
		std::shared_ptr<GeoMatch_POCR> ptrEdge = std::make_shared<GeoMatch_POCR>();
		for(int i=0;i<_sfList.size();i++)
		{
			bool flag = true;
			std::vector<std::shared_ptr<rstPSRModel>> tmp;
			tmp = _sfList[i]->SearchFont(ImgProc,Targetfont, vPOCRInspPtr,ptrEdge,sSaveParam, nForeGround, param);

			for(int fnt=0;fnt<tmp.size();fnt++)
			{
				if(tmp[fnt]->score < dStdScore[fnt])
				{
					flag = false;
					tmp[fnt]->x =tmp[fnt]->Parent_x + _sfList[i]->_sfont[fnt]->_center_x;
					tmp[fnt]->y =tmp[fnt]->Parent_y + _sfList[i]->_sfont[fnt]->_center_y;

					if(tmp[fnt]->x>ImgProc->edge->_OrgImage->cols-1)
					{
						tmp[fnt]->nCharWidth = 0;
						tmp[fnt]->x = ImgProc->edge->_OrgImage->cols-1;
					}
				}
			}
			if(tmp.size() != Targetfont.GetLength())
				flag = false;
			if(flag && tmp.size()>1)
			{
				if(i>0)
				{
					for(int idx = i;idx>0;idx--)
					{
						_sfList[i].swap(_sfList[i-1]);
					}
				}
				rst.clear();
				rst.insert(rst.end(), tmp.begin(), tmp.end());
				break;
			}
			rst.insert(rst.end(), tmp.begin(), tmp.end());
			if (g_pMPTI->bInspOne && g_pMPTI->m_nlnspIndexAI != 0)
				break;
		}
	}
	catch (CMemoryException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::SearchFont_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::SearchFont_UnknownError"));
		g_pMPTI->AddLog(csLog);
	}

	return rst;
}
std::vector<std::shared_ptr<rstPSRModel>> PsrModel::MatchingFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, CString Targetfont, cPOCRInfoParam* vPOCRInspPtr, CString sSaveParam, double* dStdScore, int nForeGround, algoPOCR_ParamContainer* param)
{
	try
		{
			rst.clear();
			//create Target font list
			int TFontsz = Targetfont.GetLength();
			std::vector < std::vector<std::shared_ptr<sFont>>> vvFonts;
			vvFonts.resize(TFontsz);
			int nLengthAvg = 0; int nLengthCnt = 0;
			for (int i = 0; i < _sfList.size(); i++)
			{
				for (int j = 0; j < _sfList[i]->_sfont.size(); j++)
				{
					for (int k = 0; k < TFontsz; k++)
					{
						if (_sfList[i]->_sfont[j]->_ch == Targetfont[k])
						{
							vvFonts[k].push_back(_sfList[i]->_sfont[j]);
							nLengthAvg += _sfList[i]->_sfont[j]->_height;
							nLengthCnt++;
						}
					}
				}
			}
			if (nLengthCnt > 0)
			{
				nLengthAvg = nLengthAvg / nLengthCnt;
			}
			else
				return rst;

			//create font rect
			std::shared_ptr<cv::Mat> procMat = ImgProc->edge->_fSrc;
			cv::Mat TempProcImg;
			procMat->copyTo(TempProcImg);
			cv::threshold(TempProcImg, TempProcImg, 0, 255, cv::THRESH_OTSU);
			if (nForeGround == 1)
				cv::bitwise_not(TempProcImg, TempProcImg);
			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::findContours(TempProcImg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 외곽선 추출
			int size = contours.size();
			if (size < 1)
			{
				return rst;
			}
			cv::Rect MaxRbox = cv::boundingRect(contours[0]);;
			double MaxA = cv::contourArea(contours[0]);
			for (int idx = 1; idx < size; idx++)
			{
				cv::Rect Rbox = cv::boundingRect(contours[idx]); //지정된 사각형을 계산해서 반환.
				double area = cv::contourArea(contours[idx]);
				if (MaxA < area)
				{
					MaxRbox = Rbox;
					MaxA = area;
				}
			}
			//std::vector<cv::Rect> vFontRectCand;
			std::vector < std::vector<cv::Rect>> vFontRectCandTemp;
			std::vector < std::vector<cv::Rect>> vvFontRect;
			if (MaxRbox.height < TempProcImg.rows - 2 && MaxA < (TempProcImg.cols*TempProcImg.rows / 2))
			{
				int CenterY(0);
				CenterY = MaxRbox.y + MaxRbox.height / 2;
				for (int idx = 0; idx < size; idx++)
				{
					cv::Rect Rbox = cv::boundingRect(contours[idx]);

					if (nLengthAvg*0.75 < Rbox.height && nLengthAvg*1.25 > Rbox.height)
					{
						//vFontRectCand.push_back(Rbox);
						bool bIsAdd = false;
						for (int vFontIdx = 0; vFontIdx < vFontRectCandTemp.size(); vFontIdx++)
						{
							int nTemY = vFontRectCandTemp[vFontIdx][0].y + vFontRectCandTemp[vFontIdx][0].height / 2;
							if (Rbox.y < nTemY && Rbox.y + Rbox.height > nTemY)
							{
								vFontRectCandTemp[vFontIdx].push_back(Rbox);
								bIsAdd = true;
								break;
					}
				}
						if (bIsAdd == false)
						{
							std::vector<cv::Rect> vFontRectTemp;
							vFontRectTemp.push_back(Rbox);
							vFontRectCandTemp.push_back(vFontRectTemp);
			}
					}
				}
				//vFontRectCandTemp.push_back(vFontRectCand);
				std::vector<int> vDiffCnt;
				for (int vFonttempIdx = 0; vFonttempIdx < vFontRectCandTemp.size(); vFonttempIdx++)
				{
					int nDiff = (int)std::labs(vFontRectCandTemp[vFonttempIdx].size() - TFontsz);
					vDiffCnt.push_back(nDiff);
				}
				int nDiffIdx = 0;
				int nDiffMax = TFontsz;
				for (int Diffi = 0; Diffi < vDiffCnt.size(); Diffi++)
				{
					if (nDiffMax > vDiffCnt[Diffi])
					{
						nDiffMax = vDiffCnt[Diffi];
						nDiffIdx = Diffi;
					}
				}
				for (int Diffi = 0; Diffi < vDiffCnt.size(); Diffi++)
				{
					if (nDiffMax == vDiffCnt[Diffi])
					{
						vvFontRect.push_back(vFontRectCandTemp[Diffi]);
					}
				}
			}
			int rectSz = vvFontRect.size();
			//if(vFontRect.size()< TFontsz)
			//	return rst;
			for (int fli = 0; fli < vvFontRect.size(); fli++)
			{
				for (int i = 0; i < vvFontRect[fli].size(); i++)
				{
					for (int j = i; j < vvFontRect[fli].size(); j++)
					{
						if (vvFontRect[fli][i].x > vvFontRect[fli][j].x)
						{
							cv::Rect tmpRect = vvFontRect[fli][i];
							vvFontRect[fli][i] = vvFontRect[fli][j];
							vvFontRect[fli][j] = tmpRect;
						}
					}
				}
			}

			//Matching
			bool flag = true;
			std::vector<std::vector<std::shared_ptr<rstPSRModel>>> rstFont;
			rstFont.resize(vvFonts.size());
			int nvrstMaxCnt = 0;
			for (int fli = 0; fli < rectSz; fli++)
			{
			for (int i = 0; i < vvFonts.size(); i++)
			{
				//std::vector<std::shared_ptr<rstPSRModel>> rstFont;
				for (int j = 0; j < vvFonts[i].size(); j++)
				{
					cv::Point2f OfsChar;
					OfsChar.x = vvFontRect[fli][i].x + vvFontRect[fli][i].width / 2;
					OfsChar.y = vvFontRect[fli][i].y + vvFontRect[fli][i].height / 2;
					std::vector<std::shared_ptr<rstPSRModel>> tmp;
					tmp = vvFonts[i][j]->SearchFont(ImgProc, vPOCRInspPtr, OfsChar, 1, param);
					if (tmp.size() > 0)
					{
						rstFont[i].insert(rstFont[i].end(), tmp.begin(), tmp.end());
					}
				}
				if (rstFont[i].size() > nvrstMaxCnt)
				{
					nvrstMaxCnt = rstFont[i].size();
				}
				//rst.push_back(rstFont);
			}
		}
			for (int nRstIdx = 0; nRstIdx < nvrstMaxCnt; nRstIdx++)
			{
				for (int i = 0; i < rstFont.size(); i++)
				{
					if (rstFont[i].size() > nRstIdx)
					{
						rst.push_back(rstFont[i][nRstIdx]);
					}
					else
					{
						std::shared_ptr<rstPSRModel> resultModel(new rstPSRModel());
						resultModel->y = 0;
						resultModel->x = 0;
						resultModel->angle = 0;
						resultModel->score = 0;
						if (vvFonts[i].size() > 0)
						resultModel->ch = (char)vvFonts[i][0]->_ch;
						else
							resultModel->ch = '?';
						resultModel->nCharWidth = 0;
						resultModel->nCharHeight = 0;
						rst.push_back(resultModel);
					}
			}
		}
	}
	catch (CMemoryException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
	}
	catch (...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::SearchFont_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::SearchFont_UnknownError"));
		g_pMPTI->AddLog(csLog);
	}

	return rst;
}

bool PsrModel::Preprocess()
{
	for(int i=0;i<_sfList.size();i++)
	{
			_sfList[i]->Preprocess();
		}
	Preprocessed = true;
	return true;
}
bool PsrModel::RePreprocess()
{
	for (int i = 0; i < _sfList.size(); i++)
	{
		_sfList[i]->RePreprocess();
	}
	Preprocessed = true;
	return true;
}

bool PsrModel::isPreprocess()
{
	return Preprocessed;
}


std::vector<std::shared_ptr<rstPSRModel>> PsrModel::SearchSingleFont(cv::Mat srcImg, CString ModelFont, int PsrListIndex, POCRInspParam & vPOCRInspPtr, algoPOCR_ParamContainer* param)
	{
		try
		{
			rst.clear();
#if ImageSave
			cv::imwrite("D:\\testimage\\workimage\\POCRSrc.bmp", srcImg);
#endif
			if (!isPreprocess())
				Preprocess();
			if (PsrListIndex >= _sfList.size())
				return rst;

			for (int i = 0; i < _sfList[PsrListIndex]->_sfont.size(); i++)
			{
				CString ch;
				ch.Format(_T("%c"), _sfList[PsrListIndex]->_sfont[i]->_ch);
				if (ModelFont.CompareNoCase(ch) == 0)
				{
					std::vector<std::shared_ptr<rstPSRModel>> tmp;
				tmp = _sfList[PsrListIndex]->_sfont[i]->SearchFontSingle(srcImg, vPOCRInspPtr, param);
					rst.insert(rst.end(), tmp.begin(), tmp.end());
				}
			}
		}
		catch (CMemoryException* e)
		{
			CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchSingleFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
		catch (CFileException* e)
		{
			CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchSingleFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(CArchiveException* e)
		{
			CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchSingleFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CException* e)
	{
		CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("PsrModel::SearchSingleFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::SearchSingleFont_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::SearchSingleFont_UnknownError"));
		g_pMPTI->AddLog(csLog);
	}

		return rst;
	}

	void PsrModel::SearchFontDraw(cv::Mat srcImg, bool rot, UCHAR * ucArrDstImgMask, int ModelIndex)
	{
		try
		{
#if ImageSave
			cv::imwrite("D:\\testimage\\workimage\\POCRDrawSrc.bmp", srcImg);
#endif
			if (!isPreprocess())
				Preprocess();
			if (ModelIndex < _sfList.size())
			{
				_sfList[ModelIndex]->SearchFontDraw(srcImg, rot, ucArrDstImgMask);
			}

#if ImageSave
			cv::Mat Mask(srcImg.rows, srcImg.cols, CV_8UC1);
			for (int y = 0; y < srcImg.rows; y++)
				memcpy(Mask.ptr(y), &ucArrDstImgMask[y*srcImg.cols], srcImg.cols);
			cv::imwrite("D:\\testimage\\MaskImg.bmp", Mask);
#endif
		}
	catch (CMemoryException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(CArchiveException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
		g_pMPTI->AddLog(csLog);
		//AfxMessageBox(csLog);
		e->Delete();
	}
	catch(...)
	{
		//AfxMessageBox(_T("Exception_GeoModel_POCR::SearchFont_UnknownError"));
		CString csLog;
		csLog.Format(_T("Exception_GeoModel_POCR::SearchFont_UnknownError"));
		g_pMPTI->AddLog(csLog);
	}
}

	void PsrModel::SearchFontDraw3Ch(cv::Mat srcImg, bool rot, UCHAR * ucArrDstImgMask, int ModelIndex)
	{
		try
		{
#if ImageSave
			cv::imwrite("D:\\testimage\\workimage\\POCRDrawSrc.bmp", srcImg);
#endif
			if (!isPreprocess())
				Preprocess();
			if (ModelIndex < _sfList.size())
			{
				_sfList[ModelIndex]->SearchFontDraw3Ch(srcImg, rot, ucArrDstImgMask);
			}

#if ImageSave
			cv::Mat Mask(srcImg.rows, srcImg.cols, CV_8UC3);
			for (int y = 0; y < srcImg.rows; y++)
				memcpy(Mask.ptr(y), &ucArrDstImgMask[y*srcImg.cols], srcImg.cols);
			cv::imwrite("D:\\testimage\\MaskImg.bmp", Mask);
#endif
		}
		catch (CMemoryException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

			csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}
		catch (CFileException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

			csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}
		catch (CArchiveException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

			csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}
		catch (CException* e)
		{
			CString csLog;

			TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

			csLog.Format(_T("PsrModel::SearchFont_%s"), szMsg);
			//g_pMPTI->AddLog(csLog);
			AfxMessageBox(csLog);
		}
		catch (...)
		{
			AfxMessageBox(_T("Exception_GeoModel_POCR::SearchFont_UnknownError"));
		}
	}
	CString PsrModel::GetModelStr(int idx)
	{
		return _sfList[idx]->_chfont;
	}
bool PsrModel::ConvertFileVersion()
{
	bool r = false;
	for (int i = 0; i < nfontListCnt; i++)
	{
		std::vector<std::shared_ptr<sFont>> fnt = _sfList[i]->_sfont;
		int num = fnt.size();
		for (int j = 0; j < num; j++)
		{
			bool _rst = fnt[j]->ConvertFileVersion();
			r = r || _rst;
		}
	}
	return r;
}