#include "StdAfx.h"
#include "OpenCV_Functions.h"
//#include "mpti.h"
#include "ipps.h"
#include "ippi.h"
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")

//CMPTI *g_pMPTI = NULL;

COpenCV_Functions::COpenCV_Functions(void)
{
	//cv::setNumThreads(1);
	//m_procMil = new CProcMil();
	m_procMil = g_pMManager->pem_new<CProcMil>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
}


COpenCV_Functions::~COpenCV_Functions(void)
{
	if(m_procMil != NULL)
		//delete m_procMil;
		g_pMManager->pem_delete(m_procMil, false);
}


void COpenCV_Functions::InitMil()
{
// 	CBase_ImgProcess::InitMil();
// 	m_Blob.Alloc(2048, 2048);
}

void COpenCV_Functions::FiniMil()
{
// 	m_Blob.Free();
// 	CBase_ImgProcess::FiniMil();
}


MIL_ID COpenCV_Functions::Histogram(UCHAR* img, int sizeX, int sizeY, int dark)
{
	cv::Mat Image = cv::Mat(sizeY,sizeX,CV_8UC1);
	cv::Mat DesImage;

	Image.data = img;

	cv::equalizeHist(Image, DesImage);
	for(int j=0; j < dark; j++)
	{
		memset(&(DesImage.data[j*DesImage.step]), 0, sizeof(UCHAR)*sizeX);
	}
	for(int j=sizeY-dark; j < sizeY; j++)
	{
		memset(&(DesImage.data[j*DesImage.step]), 0, sizeof(UCHAR)*sizeX);
	}

	for(int j=0; j < sizeY; j++)
	{
		for(int i=0; i<dark; i++)
			DesImage.data[j*DesImage.step+i] = 0;

		for(int i=sizeX-dark; i<sizeX; i++) DesImage.data[j*DesImage.step +i] = 0;
	}

//	UCHAR* saveimg = DesImage.data;
	size_t imgsize = sizeX*sizeY;
	memcpy(img,DesImage.data,imgsize);
	DesImage.copyTo(Histimg);
	MIL_ID rst;

	return (MIL_ID)1;
}

void COpenCV_Functions::Histogram(MIL_ID img)
{
	int src_sz_x = MbufInquire(img, M_SIZE_X, M_NULL);
	int src_sz_y = MbufInquire(img, M_SIZE_Y, M_NULL);
	MIL_INT src_type = MbufInquire(img, M_TYPE, M_NULL);
	int srcPitch = MbufInquire(img, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)MbufInquire(img, M_HOST_ADDRESS, M_NULL);

	cv::Mat Image(cv::Size(src_sz_x, src_sz_y), ConvertType(src_type), srcPtr, srcPitch);
	cv::Mat DesImage;

	cv::equalizeHist(Image, DesImage);
	UCHAR* saveimg = DesImage.data;
	MbufPut(img, saveimg);
}

int COpenCV_Functions::ConvertType(MIL_INT nMILType)
{
	MIL_INT TypeML	= nMILType & M_TYPE_MASK;
	MIL_INT DepthML	= nMILType & M_SIZE_BIT_MASK;
	int TypeCV(0);

	switch(TypeML)
	{
	case M_SIGNED:
		switch(DepthML)
		{
		case 8:
			TypeCV = CV_8SC1; break;
		case 16:
			TypeCV = CV_16SC1; break;
		case 32:
			TypeCV = CV_32SC1; break;
		}
		break;
	case M_UNSIGNED:
		switch(DepthML)
		{
		case 8:
			TypeCV = CV_8UC1; break;
		case 16:
			TypeCV = CV_16UC1; break;
		}
		break;
	case M_FLOAT:
		switch(DepthML)
		{
		case 32:
			TypeCV = CV_32FC1; break;
		case 64:
			TypeCV = CV_64FC1; break;
		}
		break;
	case M_DOUBLE:
		TypeCV = CV_64FC1; break;
	}

	return TypeCV;
}


void COpenCV_Functions::FilterDilation(UCHAR* Src,int width, int height, int filtersize,int filterType)
{
	cv::Mat* src_img;
	*src_img = cv::Mat(height, width, CV_8UC1, Src);

	cv::Mat *dst;
	//IplConvKernel *element;
	*dst = (*src_img).clone();

	if(filterType == 1)
	{
		//element = cvCreateStructuringElementEx (filtersize, 1, filtersize/2, 0, CV_SHAPE_RECT, NULL);//가로
		//cvDilate( src_img, dst, element, 1 );
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(filtersize, 1), cv::Point(filtersize / 2, 0));
		cv::dilate(*src_img, *dst, element);
	}
	else if(filterType == 2)
	{
		//element = cvCreateStructuringElementEx (1,filtersize, 0, filtersize/2, CV_SHAPE_RECT, NULL);//세로
		//cvDilate( src_img, dst, element, 1 );
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, filtersize), cv::Point(0, filtersize / 2));
		cv::dilate(*src_img, *dst, element);

	}
	else
	{
		//element = cvCreateStructuringElementEx (filtersize,filtersize, filtersize/2, filtersize/2, CV_SHAPE_RECT, NULL);
		//cvDilate( src_img, dst, element, 1 );
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(filtersize, filtersize), cv::Point(filtersize/2, filtersize/2));
		cv::dilate(*src_img, *dst, element);
	}
	memcpy(Src, dst, sizeof(UCHAR)*width*height);
	(*dst).copyTo(*Dilateimg);
}


void COpenCV_Functions::FilterErosion(UCHAR* Src,int width, int height, int filtersize,int filterType)
{
	cv::Mat* src_img, *dst;
	//IplConvKernel *element;
	MIL_ID mil_img;

	*src_img = cv::Mat(height, width, CV_8UC1, Src);
	//cvSetData(src_img,Src,src_img->step);
	//dst = cvCloneMat(src_img);
	*dst = (*src_img).clone();

	if(filterType == 1)
	{
		//element = cvCreateStructuringElementEx (filtersize,1, filtersize/2, 0, CV_SHAPE_RECT, NULL);
		//cvErode( src_img, dst, element, 1 );
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(filtersize, 1), cv::Point(filtersize / 2, 0));
		cv::erode(*src_img, *dst, element);
	}
	else if(filterType == 2)
	{
		//element = cvCreateStructuringElementEx (1,filtersize, 0, filtersize/2, CV_SHAPE_RECT, NULL);
		//cvErode( src_img, dst, element, 1 );
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, filtersize), cv::Point(0, filtersize / 2));
		cv::erode(*src_img, *dst, element);
	}
	else
	{
		//element = cvCreateStructuringElementEx (filtersize,filtersize, filtersize/2, filtersize/2, CV_SHAPE_RECT, NULL);
		//cvErode( src_img, dst, element, 1 );
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(filtersize, filtersize), cv::Point(filtersize / 2, filtersize / 2));
		cv::erode(*src_img, *dst, element);
	}
	memcpy(Src, dst, sizeof(UCHAR)*width*height);
	//cvReleaseStructuringElement(&element);
	//cvReleaseMat(&src_img);
	//cvReleaseMat(&dst);
	(*dst).copyTo(*erodeimg);
}


void COpenCV_Functions::GetHistogram(UCHAR* img,int width, int height, UCHAR* &dst)
{
	//UCHAR* image  = new UCHAR[width * height];
	UCHAR* image = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
// 	if(dst != NULL)
// 	{
// 		delete dst;
// 		dst = NULL;
// 	}
// 	dst = new UCHAR[width * height];
	Img_fromCV(img,width,height,image);
	Histogram(image, width,height, 5);
	SaveWorkImg(image,width,height,_T("imgpro_Hist_image.bmp"));
	Img_TOCV(image,width,height,dst);
	if(image != NULL)
	{
		//delete image;
		g_pMManager->pem_delete(image, false);
		image = NULL;
	}
	SaveWorkImg(dst,width,height,_T("imgpro_Hist_dst.bmp"));
}
void COpenCV_Functions::GetErosion(UCHAR* img,int width, int height, int filtersize,int filterType,UCHAR* &dst)
{
	//UCHAR* image = new UCHAR[width * height];
	UCHAR* image = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
// 	if(dst != NULL)
// 	{
// 		delete dst;
// 		dst = NULL;
// 	}
// 	dst = new UCHAR[width * height];
	Img_fromCV(img,width,height,image);
	FilterErosion(image,width,height,filtersize,filterType);
	SaveWorkImg(image,width,height,_T("imgpro_Erosion_image.bmp"));
	Img_TOCV(image,width,height,dst);
	if(image != NULL)
	{
		//delete image;
		g_pMManager->pem_delete(image, false);
		image = NULL;
	}
}
void COpenCV_Functions::GetDilation(UCHAR* img,int width, int height, int filtersize,int filterType,UCHAR* &dst)
{
	//UCHAR* image = new UCHAR[width * height];
	UCHAR* image = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
// 	if(dst != NULL)
// 	{
// 		delete dst;
// 		dst = NULL;
// 	}
// 	dst = new UCHAR[width * height];
	Img_fromCV(img,width,height,image);
	FilterDilation(image,width,height,filtersize,filterType);
	SaveWorkImg(image,width,height,_T("imgpro_Dilation_image.bmp"));
	Img_TOCV(image,width,height,dst);
	if(image != NULL)
	{
		//delete image;
		g_pMManager->pem_delete(image, false);
		image = NULL;
	}
}

void COpenCV_Functions::DrawCharRect(UCHAR* img,int width, int height,double* x,double* y, double* w,double* h,int cnt, int forground)
{

	cv::Mat src_img, dst;
	MIL_ID mil_img;

	src_img = cv::Mat(height,width,CV_8UC1);
	src_img.data = img;
	src_img.copyTo(dst);

	for(int i=0;i<cnt;i++)
	{
		cv::Rect re;
		cv::Scalar color;
		if(forground == 1)color = 0;
		else color = 255;
		re.x = (int)x[i];
		re.y = (int)y[i];
		re.width = (int)w[i];
		re.height = (int)h[i];
		rectangle(dst,re,color,3);
	}

	memcpy(img,dst.data,sizeof(UCHAR)*width*height);
	src_img.release();
	dst.release();
}

void COpenCV_Functions::ImgCrop(UCHAR* img,int width, int height,double x,double y,double dstWidth, double dstHeight,UCHAR* dstImg)
{

	cv::Mat src_img, dst;
	//MIL_ID mil_img;

	src_img = cv::Mat(height,width,CV_8UC1);
	src_img.data = img;

	cv::Rect rect(x,y,dstWidth,dstHeight);
	dst = src_img(rect);

	memcpy(dstImg,dst.data,sizeof(UCHAR)*dstWidth*dstHeight);
	src_img.release();
	dst.release();
}

void COpenCV_Functions::SaveWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize)
{
#ifdef _DEBUG
	//#ifdef _OFFLINE_INSP
	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, fileName);

	if((buff == NULL) || (sizeX <= 0) || (sizeY <= 0) || (bandSize <= 0))
		return;

	MIL_ID temp = M_NULL;
	if(bandSize == 1)
	{
		temp = AllocBuff(sizeX, sizeY);
		MbufPut(temp, buff);
	}
	else if(bandSize == 3)
	{
		temp = AllocBuffColor(sizeX, sizeY);
		MbufPutColor(temp, M_PACKED + M_BGR24, M_ALL_BANDS, buff);
	}

	MbufExport(fullPath, M_BMP, temp);

	FreeMilImageBuff(&temp);	
	//#endif
#endif
}
MIL_ID COpenCV_Functions::AllocBuff(int width, int height, int initVal)
{
	MIL_ID milBuff = M_NULL;
	MbufAlloc2d(M_DEFAULT_HOST, (MIL_INT)width, (MIL_INT)height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);

	if(initVal >= 0 && initVal <= 255)
		MbufClear(milBuff, initVal);

	return milBuff;
}

MIL_ID COpenCV_Functions::AllocBuffColor(int width, int height, double initVal)
{
	MIL_ID milBuff = M_NULL;
	MbufAllocColor(M_DEFAULT_HOST ,3 ,(MIL_INT)width ,(MIL_INT)height ,8 + M_UNSIGNED, M_IMAGE + M_PROC  , &milBuff);

	if(initVal >= 0)
		MbufClear(milBuff, initVal);

	return milBuff;
}

void COpenCV_Functions::FreeMilImageBuff(MIL_ID* milImage)
{
	if(*milImage != M_NULL)
	{
		MbufFree(*milImage);
		*milImage = M_NULL;
	}
}


int COpenCV_Functions::Img_fromCV(UCHAR* Img,int width, int height, UCHAR * dstImg)
{
	// 	if(sWndAlgoImg->m_bIs2dCV == FALSE)
	// 		return;
	if(!Img)
		return eMR_FAIL;

	if((width <= 0) || (height <= 0))
		return eMR_FAIL;
	int widthStep = width;

	/*int nSizeX = width % 4;
	if (nSizeX!=0)
		widthStep += (4 - nSizeX);*/

	//Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dstImg, width * height);
	//UCHAR* Image = new UCHAR[width * height];
	UCHAR* Image = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);

	IppStatus sts;
	IppiSize iSize = {width, height};
	sts = ippiCopy_8u_C1R(Img, widthStep, Image, width, iSize);
	memcpy(dstImg,Image,sizeof(UCHAR)*width*height);

	if(Image != NULL)
	{
		//delete Image;
		g_pMManager->pem_delete(Image, false);
		Image = NULL;
	}
	//LJH 2016.06.07 일단 주석 처리
	//Release Test 시 이상이 없으면 삭제
	//for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
	//	sts = ippiCopy_8u_C1R(sWndAlgoImg->m_ucArrCV, widthStep, sWndAlgoImg->m_ucArr2D_Mix[iLoopCount], width, iSize);
	return eMR_SUCCESS;
}

void COpenCV_Functions::Img_TOCV(UCHAR * srcImg, int width, int height, UCHAR * dstImg)
{
	if(srcImg == NULL || dstImg == NULL/*|| sWndAlgoImg->m_bIs2dCV == FALSE*/)
		return;

	int widthStep = width;

	//int nSizeX = width % 4;
	//if (nSizeX != 0)
	//{
	//	widthStep += (4 - nSizeX);
	//}

	//UCHAR* Image = new UCHAR[width * height];
	UCHAR* Image = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);

	IppStatus sts;
	IppiSize iSize = {width, height};
	sts = ippiCopy_8u_C1R(srcImg, width, dstImg, widthStep, iSize);
// 	memcpy(dstImg,Image,sizeof(UCHAR)*width*height);

	if(Image != NULL)
	{
		//delete Image;
		g_pMManager->pem_delete(Image, false);
		Image = NULL;
	}

}