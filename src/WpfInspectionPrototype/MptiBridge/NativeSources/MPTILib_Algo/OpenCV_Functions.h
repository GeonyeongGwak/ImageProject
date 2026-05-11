#pragma once

#include "stdafx.h"
#include "ProcMil.h"
//#include "mil.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "PILibrary.h"
//#pragma  comment(lib, "Mil.lib")
//#pragma  comment(lib, "Milim.lib")
//#pragma  comment(lib, "MilBlob.lib")
//#pragma  comment(lib, "MilStr.lib")
//#pragma  comment(lib, "MilPat.lib")

class COpenCV_Functions
{
public:
	COpenCV_Functions(void);
	virtual ~COpenCV_Functions(void);

	CProcMil* m_procMil;
	cv::Mat Histimg;
	cv::Mat* erodeimg;
	cv::Mat* Dilateimg;
public:
	virtual void FiniMil();
	virtual void InitMil();

	MIL_ID Histogram(UCHAR* img, int sizex, int sizeY, int dark);
	void Histogram(MIL_INT img);
	void FilterErosion(UCHAR* Src,int width, int height, int filtersize,int filterType);
	void FilterDilation(UCHAR* Src,int width, int height, int filtersize,int filterType);
	void GetHistogram(UCHAR* img,int width, int height, UCHAR* &dst);
	void GetErosion(UCHAR* Src,int width, int height, int filtersize,int filterType,UCHAR* &dst);
	void GetDilation(UCHAR* Src,int width, int height, int filtersize,int filterType,UCHAR* &dst);
	void DrawCharRect(UCHAR* img,int width, int height,double* x,double* y, double* w,double* h,int cnt,int forground);
	void ImgCrop(UCHAR* img,int width, int height,double x,double y,double dstWidth, double dstHeight,UCHAR* dstImg);
public:
	int ConvertType(MIL_INT nMILType);

public://MIL
	void SaveWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize = 1);
	MIL_ID AllocBuff(int width, int height, int initVal = -1);
	MIL_ID AllocBuffColor(int width, int height, double initVal = -1);
	void FreeMilImageBuff(MIL_ID* milImage);

public://convert
	int Img_fromCV(UCHAR* Img,int width, int height, UCHAR * dstImg);
	void Img_TOCV(UCHAR * srcImg, int width, int height, UCHAR * dstImg);
};

