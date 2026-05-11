#pragma once
#include "Proc.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
//#include "..\..\cvvImage.h"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
// #ifdef _DEBUG
// #pragma comment( lib, "libjasperd.lib" )
// #pragma comment( lib, "libjpegd.lib" )
// #pragma comment( lib, "libpngd.lib" )
// #pragma comment( lib, "libtiffd.lib" )
// #pragma comment( lib, "zlibd.lib" )
// #pragma comment( lib, "opencv_core242d.lib" )
// #pragma comment( lib, "opencv_gpu242d.lib" )
// #pragma comment( lib, "opencv_highgui242d.lib" )
// #pragma comment( lib, "opencv_imgproc242d.lib" )
// #else
// #pragma comment( lib, "libjasper.lib" )
// #pragma comment( lib, "libjpeg.lib" )
// #pragma comment( lib, "libpng.lib" )
// #pragma comment( lib, "libtiff.lib" )
// #pragma comment( lib, "zlib.lib" )
// #pragma comment( lib, "opencv_core242.lib" )
// #pragma comment( lib, "opencv_gpu242.lib" )
// #pragma comment( lib, "opencv_highgui242.lib" )
// #pragma comment( lib, "opencv_imgproc242.lib" )
// #endif
//-----------------------------------


class CProc3D : public CProc
{
public:
	CProc3D(void);
	~CProc3D(void);
private:	
	int* m_pZmapHistoData;
	int m_histoSize; 

public:
	float ExtractBody(float* zmapData, float* bodyDst, float* boardDst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, int range);
	void DeleteHistoVal();

	void GetClipZmapData(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY);
	void GetHlimit(float* src, int sizeX, int sizeY, float* retHmax, float* retHmin);
	void GetHlimit(int* src, int sizeX, int sizeY, int* retHmax, int* retHmin, BOOL removeData, int rmvVal);
	void GetZmapObjectRange(int* histo, int st, int ed, double range, int* rangeMin, int* rangeMax);
	float GetZmapHavr(float* src, int sizeX, int sizeY, float hMin);
	void NormalizeData_Gray(float* src, UCHAR* dst, int sizeX, int sizeY); // zmap data normalize => min:0 , max:255
	void NormalizeData_Zmap(float* src, float* dst, int sizeX, int sizeY, float hMin); //zmap data normalize => min:zmapMin - zmapMin , max:zmapMax - zmapMin
	void MakeHistogram(float* src, int* dstHisto, int sizeX, int sizeY, float hMax, float hMin, int histoSize);
	void GetHistogramSize(int* retSize, int* retMax);
	void GetHistogramData(int* dst, int histoSize);
	void ExtractData(float* src, float* dst, int sizeX, int sizeY, int rangeMin, int rangeMax, float defaultValue);

	int GetThreshold(int* histo, int sizeX, int sizeY, int histoSize);
	void GetStdev(int* data, int st, int ed, double* retAvr, double* retStd); 
	void GetStdev(float* data, int st, int ed, double* retAvr, double* retStd);

	void HistogramSave(int* histo, int histoSize);   //test
	void DataSave(float* data, int dataSize, CString path);
	void DataSave(int* data, int dataSize, CString path);
	void DataSave(float* data1, float* data2, int dataSize, CString path);

	int GetMaxVal(int* histo, int st, int ed);


	void SaveBuffer(unsigned char* pImageSrc, int nWidth, int nHeight, int nChannel, char *psFileName);	
	//////////////////////////////////////////////////////////////////////////

	float ExtractBody2(float* pZmapRoiData, float* bodyDst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, double angle, double* retCogX, double* retCogY, CRect* retRect, UCHAR* img = NULL);
	float ExtractBody(float* pZmapRoiData, float* bodyDst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, double angle, double* retCogX, double* retCogY, CRect* retRect, UCHAR* img = NULL);
	void ExtractData(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY, float defaultValue);
	int CalcThreshold(float* zmapData, int bufSize);
	int CalcThreshold(float* zmapData, int sizeX, int sizeY);
	
	void MakeProfileData_H(float* zmapData, float* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size);	
	void MakeProfileData_V(float* zmapData, float* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size);

	void MakeProfileData_H(UCHAR* imgData, UCHAR* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size); // คั
	void MakeProfileData_V(UCHAR* imgData, UCHAR* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size); // คำ

	void CalcStartPoint(int sizeX, int sizeY, int rowCnts, int colCnts, int margin, CPoint* retLeft, CPoint* retRight, CPoint* retTop, CPoint* retBottom);
	void CalcStartPoint2(int cx, int cy, CRect rt, int margin, int rowCnts, int colCnts, CPoint* retLeft, CPoint* retRight, CPoint* retTop, CPoint* retBottom);
	float GetFilterHeight(float* src, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY);
	void SearchDataEdge_rev(float* pfData, int dataSize, double stdHeight, int stdPixel, int* retDeltaMax);
	void SearchDataEdge(float* pfData, int dataSize, double stdHeight, int stdPixel, int mode, int* retDeltaMax);
	int CalcLineEquation(int* data, int cnts, int mode, CPoint* startCoordi, float* retCoef);
	void CalcLineEquation(POINT* data, int cnts, float* retCoef);
	double CalcLineEquation(int* data, int cnts, int mode, int width, int height, CPoint* startCoordi, float* retCoef);
	double CalcLineEquation(POINT* data, int cnts, int mode, int width, int height, float* retCoef);

	bool LeastSquareMethod(float x[], float y[], UINT nOrder, UINT nDatNum, float Coef[]);
	int LeastSquareMethod_XY(float *arrX, float *arrY, float *arrZ, int len, float* coefX, float* coefY, float* coefC);
	void HoughLine(float* x, float* y, int dataCnts, int width, int height, int ang, int t, double* retRho, double* retAngle);
	void HoughLine2(float* x, float* y, int dataCnts, int width, int height, int ang, int t, double* retRho, double* retAngle);
	void MakeLUT_Poly(BYTE* dst, int sizeX, int sizeY, const POINT *pts, int ptNum);
	bool PtInPolygon(POINT pt, const POINT *pts, int ptNum);

	float MaskingData(float* src, BYTE* mask, float* dst, int sizeX, int sizeY, float defaultVal);
	float MaskingData_rev(float* src, BYTE* mask, float* dst, int sizeX, int sizeY, float defaultVal);

	void CalcNodalPoint(float* pVal1, double angle1, float* pVal2, double angle2, long* retX, long* retY);



	//////////////////////////////////////////////////////////////////////////lead
	void ExtractLeadData(float* zmapData, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, UCHAR* grayImg);
	void MakeRoughGrayImg(float* src, UCHAR* dst, int sizeX, int sizeY);
	void MakeRoughGrayImg(float* src, UCHAR* dst, int sizeX, int sizeY, int minRange, BOOL inverse = FALSE);
	float GetZmapDataAvr(float* src, int sizeX, int sizeY);
	float GetZmapDataAvr(float* src, int sizeX, int sizeY, float minRange, float maxRange);

	BOOL InspLeadArea(float* leadData, int sizeX, int sizeY, int index, UCHAR* grayImage, BOOL* retResult);
	BOOL InspLeadArea(float* zmapData, int sizeX, int sizeY, CRect* leadRect, int rectCnts, double angle, BOOL* retResult);
	float CalcLeadHeight(float* zmapData, int sizeX, int sizeY, CRect* leadRect, int rectCnts, double angle, float* retHeight);
	float CalcLeadHeight(float* zmapData, int orgSizeX, int orgSizeY, int clipSizeX, int clipSizeY, CRect* leadRect, int rectCnts, float* retHeight);

	void RotateRect(CRect* data, int cnts, double angle, int orgSizeX, int orgSizeY, CRect* retResult, int* retWidth, int* retHeight);


	void RemakeZmap(float* srcZmap, float* dstZmap, int sizeX, int sizeY, int minRange, int maxRange, float minData);


	//////////////////////////////////////////////////////////////////////////
	float GetStdDeltaH(float* roiZmap, int sizeX, int sizeY, double range, float* retFilterHighH, float* retFilterLowH = NULL);
	int SearchEdge(float** data, int dataCnt, int dataSize, float filterH, float stdH, int stdLen, CPoint* startCoordi, int position, CPoint** retEdge);
	CPoint MakePoint(int calcData, CPoint startCoordi, int position, int pfMargin = 0);
	BOOL IsBody(float* data, int dataSize, int searchLen, float filterH, int position);
	double CalcLineEquation(CPoint* data, int cnts, int mode, int width, int height, CPoint* startCoordi, float* retCoef);

	//////////////////////////////////////////////////////////////////////////
	BOOL IsHeightOk(float* pZmapData, int roiSizeX, int roiSizeY, float stdH);
	BOOL GetRoughGrayImage(float* pZmapRoiData, int roiSizeX, int roiSizeY, double range, float* retThreshold, UCHAR* grayImg_High, UCHAR* grayImg_Low = NULL);
	double ExtractBody3(float* pZmapRoiData, int cx, int cy, int roiSizeX, int roiSizeY, double angle, UCHAR* blobImg, UCHAR* blobImg_board, int blobCx, int blobCy, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, BYTE* lutData = NULL, UCHAR* img = NULL, POINT* retNodalPoint = NULL);
	double ExtractBody4(UCHAR* blobImg, int cx, int cy, int roiSizeX, int roiSizeY, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, POINT* retNodalPoint, UCHAR** retTest1, UCHAR** retTest2, UCHAR** retTest3, UCHAR** retTest4, UCHAR** retTest5, int* retTestSizeX, int* retTestSizeY);
	int SearchEdge(UCHAR** srcProfile, int dataCnt, int dataSize, int pfMargin, CPoint* startCoordi, int position, CPoint** retEdge);
	int SearchEdge2(UCHAR** srcProfile, int dataCnt, int dataSize, int pfMargin, CPoint* startCoordi, int position, CPoint** retEdg);


	//////////////////////////////////////////////////////////////////////////
	int RemakeSolderRect(float* pZmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, CRect** retResult);
	int RemakeSolderRect_Chip(float* pZmapData, int roiSizeX, int roiSizeY, CRect solderRect, CRect* retResult);
	int CalcAreaVolume(float* pZmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, double* retAreaVolume, float* retHlimit);


	



	//////////////////////////////////////////////////////////////////////////
	void MakeZmap2BinImg(float* zmap, int sizeX, int sizeY, float threshold, UCHAR* dstImg);
	BOOL MakeZmap2BinImg2(float* zmap, int sizeX, int sizeY, float threshold, UCHAR* dstImg);
	float GetBinImage(float* roiZmap, int roiSizeX, int roiSizeY, float Threshold, UCHAR* BinImg);
	float GetBinImage2(float* roiZmap, int roiSizeX, int roiSizeY, float Threshold_Low, float Threshold_Heigh, UCHAR* BinImg);
	//SHK 2013/1107
	float GetCropZmap(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY);
	float * GetCropZmap(float* src, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY);		// LMJ 2014/01/16

	//SHK 2013/11/11
	double ExtractBodyCv(UCHAR* blobImg, double cx, double cy, int roiSizeX, int roiSizeY, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, POINT* retNodalPoint);
	double ExtractBodyCv(UCHAR* blobImg, double cx, double cy, int roiSizeX, int roiSizeY, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, POINT* retNodalPoint, int nOkPercent, bool bSTD = false, bool bUnitFind = false, bool bHO = false, int nTeachX = -1, int nTeachY = -1, double* retBodyA = NULL);
	double GetAngle(int x1,int y1, int x2,int y2);
	double GetBodyAngle(UCHAR* ucpBlobImg, int nSizeX, int nSizeY, int nBlobCenterX, int nBlobCenterY, int nBlobWidth, int nBlobHeight);

	//huj 2013/12/20
	void RotateZmap_ipp2(float* zmapData, float** dst, int orgSizeX, int orgSizeY, double angle, int* retDstSizeX = NULL, int* retDstSizeY = NULL);
	cv::Point WarpAffine(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev);
	
	bool RotateZmap_ipp2020(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize);

	int ProcAnyAngle_Zmap(float* src, float** dst, int orgSizeX, int orgSizeY, int bdryCx, int bdryCy, int bdrySizeX, int bdrySizeY, double angle, int* retDstSizeX, int* retDstSizeY);
	int ProcAnyAngle_Zmap_LT(float* src, float** dst, int orgSizeX, int orgSizeY, int bdry_stX, int bdry_stY, int bdrySizeX, int bdrySizeY, double angle, int* retDstSizeX, int* retDstSizeY);

	//shk 2014/04/10
	float CalcLeadHeight2(float* zmapData, int orgSizeX, int orgSizeY, int clipSizeX, int clipSizeY, CRect* leadRect, int rectCnts, float* retHeight);
	float GetFilterHeight2(float* src, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY);

	//////////////////////////////////////////////////////////////////////////

	// SHW 2014/11/03
	float GetCropZmap_LT(float* src, float* dst, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY);
	float GetCropZmap_LT(float* src, float* dst, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY, double dAngle);
	float * GetCropZmap_LT(float* src, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY);

	bool CheckBody(UCHAR* ucImg, int nW, int nH, CRect rcROI, double dBodyW, double dBodyH, double dStdW, double dStdH, double dPerOK);
private:

	void EraseBorderLine(UCHAR* userSrc, UCHAR* userDst, int sizeX, int sizeY, int pixelCount, int position);

	void GetAB(float * fArrPosX, float * fArrPosY, int nCount, float * a, float * b);
};

