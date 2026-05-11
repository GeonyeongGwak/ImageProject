#pragma once
//#include "mil.h"
#include "InspParamDef.h"
#include "Proc.h"
#include "opencv2/opencv.hpp"
#include "GeoMatch.h"
#include "PILibrary.h"

//#pragma comment(lib, "Mil.lib")
//#pragma comment(lib, "Milim.lib")
//#pragma comment(lib, "MilBlob.lib")
//#pragma comment(lib, "MilStr.lib")
//#pragma comment(lib, "MilPat.lib")


#define STRUCT_ELEM_WIDTH     3L
#define STRUCT_ELEM_HEIGHT    3L
enum m_eAlgoPoint
{
	m_eAlgoPoint_RoundUp = 0,
	m_eAlgoPoint_RoundDown,
	m_eAlgoPoint_Up,
	m_eAlgoPoint_Down,
	m_eAlgoPoint_Auto,
	m_eAlgoPoint_Total,
};
class CMilBlobResult;

class CProcMil : public CProc 
{
public:
	CProcMil(void);
	virtual ~CProcMil(void);

	//enum { eTypeRangeIn, eTypeRangeOut, eTypeRangeGrater, eTypeRangeLess	};		// InspParamDef.h  이동
protected:
	Im::PIL_ID* m_milApp;	// mil application
	Im::PIL_ID* m_milSys;	// mil system

	Im::PIL_ID m_structElement_H;
	Im::PIL_ID m_structElement_V;

	Im::PIL_ID m_milBlobFeature;
	Im::PIL_ID m_milBlobResult;

	unsigned char *m_InImg;       // bmp파일의 이미지 내용 저장

	BITMAPFILEHEADER dibHf;    // 비트맵 파일헤드 구조체

	BITMAPINFOHEADER dibHi;    // 비트맵 영상헤드 구조체

	RGBQUAD palRGB[256];        // 팔레트 정보 구조체 배열

	DWORD m_dwWidth;
	DWORD m_dwHeight;

public:
	Im::PIL_ID GetBlobResultId() { return m_milBlobResult; }
	virtual int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys,bool bUseImagePilLib = false);
	virtual int FreeMil();
	void FreeMilAlgoBlob();

	Im::PIL_ID AllocBuff(int width, int height, int initVal = -1);
	Im::PIL_ID AllocBuffColor(int width, int height, double initVal = -1);
	Im::PIL_ID AllocClipBuff(Im::PIL_ID milSrc, int cX, int cY, int width, int height);	
	
	void GetClipBuff(void* milSrc, UCHAR* userDst, int cx, int cy, int roiSizeX, int roiSizeY); 
	void GetClipBuff_LT(void* milSrc, UCHAR* userDst, int nSX, int nSY, int roiSizeX, int roiSizeY);
	void GetClipBuff_LT_LoadRawData(void* milSrc, UCHAR* userDst, int nSX, int nSY, int roiSizeX, int roiSizeY);
	void GetClipImage(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int cx, int cy, int roiSizeX, int roiSizeY, int band = 1);
	void GetClipImage_LT(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int Sx, int sy, int roiSizeX, int roiSizeY, int band = 1, double bAngle = 0);
	void SetBackClipImage_LT(UCHAR* userSrc, int srcSizeX, int srcSizeY, int srcPitch, UCHAR* userDst, int Sx, int sy, int roiSizeX, int roiSizeY, int band = 1); // YJS 2016/11/04
	Im::PIL_ID AllocClipBuffColor(Im::PIL_ID milSrc, int cX, int cY, int width, int height);
	Im::PIL_ID AllocChildImage(Im::PIL_ID milSrc, int roiX, int roiY, int roiWidth, int roiHeight);
	Im::PIL_ID AllocRotateBuff(Im::PIL_ID milSrc, double angle);
	Im::PIL_ID AllocRotateBuff_Color(Im::PIL_ID milSrc, double angle);
	Im::PIL_ID AllocNomalRotateBuff(Im::PIL_ID milSrc, double angle);

	UCHAR* GetPtrGrabBuf(void* milBuf);
	void GetPtrGrabBuf(void* milBuf, UCHAR* userDst);
	void FreeMilImageBuff(Im::PIL_ID* milImage);



	CString CreatePath(CString path, CString format);

	void GetRotateImg(Im::PIL_ID milSrc, Im::PIL_ID milDst, double angle);
	void RotateImg(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR* userDst, int* retDstSizeX = NULL, int* retDstSizeY = NULL);
	void RotateImg_Color(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR* userDst, int* retDstSizeX = NULL, int* retDstSizeY = NULL);
	CSize GetImageSize(Im::PIL_ID milImg);

	BOOL IsExistDir(CString path);
	void CreateDir(CString Path);
	void SaveWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize = 1, bool bSaveSide = FALSE, bool bSaveR = false);
	void SaveWorkImg2(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize = 1, bool bSaveSide = FALSE, bool bSaveR = false);
	void SaveWorkImg(Im::PIL_ID milBuff, CString fileName);
	void SaveWorkImg(cv::Mat milBuff, CString fileName);
	void SaveReleaseWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, CString path, int bandSize = 1);
	void SaveReleaseWorkImg(Im::PIL_ID milBuff, CString fileName, CString path);
	void SaveWorkImg_float(Im::PIL_ID milSrc, CString fileName);
	void SaveWorkImg_float(float* src, int sizeX, int sizeY, CString fileName, int band = 1, bool bSaveR = false);
	void SaveDebugImg(UCHAR* buff, int sizeX, int sizeY, CString FullPath, int bandSize = 1, BOOL bEnCrypt = FALSE);
	void EnCryptBMP(CString FullPath);
	BOOL OnOpenDocument(LPCTSTR lpszPathName);
	void SaveDebugImg(Im::PIL_ID milBuff, CString fileName);
	void SaveDebugImg_float(Im::PIL_ID milSrc, CString fileName);
	void SaveDebugImg_float(float* src, int sizeX, int sizeY, CString fileName, int band = 1);
	void SaveBuff(Im::PIL_ID milBuff, CString path);
	void SaveBuffTIF(Im::PIL_ID milBuff, CString path);
	void SaveImage(UCHAR* userBuff, int sizeX, int sizeY, int band, CString path);
	void SaveTIFImage(UCHAR* userBuff, int sizeX, int sizeY, int band, CString path);
	void SaveNGimg(UCHAR* userBuf, int orcWidth, int orcHeight, int cx, int cy, int width, int height, CString path);
	void SaveClipImg(Im::PIL_ID src, int band, int cx, int cy, int width, int height, CString strPath);
	void SaveClipImg(UCHAR* userSrc, int band, int orcWidth, int orcHeight, int cx, int cy, int width, int height, CString strPath);
	void SaveClipImg_void(void* milSrc, int band, int cx, int cy, int width, int height, CString strPath);
	void SaveOCRImg(Im::PIL_ID milBuff, CString fileName);
	void SaveWorkImg_dword(DWORD* src, int sizeX, int sizeY, CString fileName, int band = 1);
	void MakeColorImage(void* milSrcR, void* milSrcG, void* milSrcB, UCHAR* userDst);

	Im::PIL_ID GetMilSrc( const WndAlgoImg &sWndAlgoImg, BOOL b2D=TRUE ,double angle = 0.0 );
	Im::PIL_ID GetMilSrc( const WndAlgoImg &sWndAlgoImg, int nIndex, BOOL b2D,double angle = 0.0);	//KSH
	void BinarizeImagesCombine(UCHAR* srcImg[], UCHAR* &destImg, int nImgWidth, int nImgHeight, int nImageCount, int nFiltering = false, BOOL bTeach=false);
	int Binarize(UCHAR* src, int sizeX, int sizeY, int threshmin, int threshmax, int nRangeMode, int mode, UCHAR* dest, int nChannel, BOOL bFillHole);
	int Morphology(UCHAR* src, int sizeX, int sizeY, int nMorphologyMode, int nIteration, UCHAR* dest);
	int FillHoleMode(UCHAR* src, int sizeX, int sizeY, int nFillHoleMode, int nIteration, UCHAR* dest);

	uchar* MatToBytes(cv::Mat image, uchar * bytes);
	cv::Mat BytesToMat(UCHAR* src, int sizeX, int sizeY, int colortype = 0);
	double ChangePoint(int nAlgoPoint, double dData);
	int GetBlobResult_ALL(double *dArea, double *dCx, double *dCy, CRect *rcRect);
	int cvContour(UCHAR* src, int sizeX, int sizeY, POINTF* ptVector);

	int CProcMil::FOVImageCompose(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer, 
		int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
		int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage); // LYS 2020/05/26

public:
	void Binarize(Im::PIL_ID milSrc, Im::PIL_ID milDst, int thresh, BOOL invert);
	void Binarize(Im::PIL_ID milSrc, Im::PIL_ID milDst, int threshLow, int threshHigh, BOOL invert);
	void Binarize(Im::PIL_ID milSrc, Im::PIL_ID milDst, int nTypeRange, int threshLow, int threshHigh, BOOL invert);

	MIL_INT GetTypeRange( int nTypeRange, BOOL binvert);

	// 	int CalcBlob(UCHAR* userBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob = FALSE, BOOL fillHole = FALSE, UCHAR* userDst = NULL);
// 	int CalcBlob(Im::PIL_ID milBinSrc, int minArea, int maxArea , BOOL eraseBorderBlob = FALSE, BOOL fillHole = FALSE, Im::PIL_ID milDst = NULL);

	int GetThreshold(Im::PIL_ID milSrc);
	int GetThreshold(UCHAR* userSrc, int sizeX, int sizeY);
	int GetOtsuThreshold(Im::PIL_ID milSrc);
	int GetOtsuThreshold(UCHAR* userSrc, int sizeX, int sizeY);
	

	void MorOpen(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration);
	void MorClose(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration);
	void MorDilate(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration);
	void MorErode(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration);

	void MorUserErode_V(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale = FALSE);
	void MorUserErode_H(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale = FALSE);
	void MorUserDilate_V(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale = FALSE);
	void MorUserDilate_H(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale = FALSE);

	void MedianFilter(Im::PIL_ID milSrc, Im::PIL_ID milDst, BOOL isGrayScale = FALSE);

	void ArithImage(Im::PIL_ID milSrc1,Im::PIL_ID milSrc2, Im::PIL_ID milDst, int operation, BOOL isSaturation = FALSE);

	double GetResizeFactor(int viewX, int viewY, int imgX, int imgY, BOOL rotate = FALSE);
	void ResizeImg(Im::PIL_ID milSrc, Im::PIL_ID milDst, double factor, BOOL imgCenter);
	void ResizeImg(UCHAR* userSrc, UCHAR* userDst, CRect bodyRect, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double factor, BOOL imgCenter, BOOL bUseMASK);
	void ResizeImg(UCHAR* userSrc, UCHAR* userDst, int roiSizeX, int roiSizeY, int sizeX, int sizeY, int bandSize, double factor);

	void FillHoleBlob(Im::PIL_ID milBinSrc, Im::PIL_ID milDst, Im::PIL_ID BlobResId = NULL);
	void EraseBorderBlob(Im::PIL_ID milBinSrc, Im::PIL_ID milDst, Im::PIL_ID BlobResId = NULL);

	void EdgeDetect(Im::PIL_ID milSrc, Im::PIL_ID milDst);
	void EdgeDetect(UCHAR* userSrc, UCHAR* userDst, int width, int height);


	void MakeBlob(Im::PIL_ID milSrc, Im::PIL_ID milDst);
	int CalcBlob(UCHAR* userBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob = FALSE, BOOL fillHole = FALSE, UCHAR* userDst = NULL);
	int CalcBlob(Im::PIL_ID milBinSrc, int minArea, int maxArea , BOOL eraseBorderBlob = FALSE, BOOL fillHole = FALSE, Im::PIL_ID milDst = NULL, int nFilter = 0);
	int CalcBlob_New(Im::PIL_ID milBinSrc, int minArea, int maxArea, BOOL eraseBorderBlob = FALSE, BOOL fillHole = FALSE, Im::PIL_ID milDst = NULL, int nFilter = 0);
	void GetPolygon(UCHAR * uArrDst, int nWidth, int nHeight, int nStartX, int nStartY, std::vector<std::vector<POINTF>>* vPolygons, vector<POINTF>* vPolyCenter);
	int InitMilAlgoBlob();
	int GetBlobResult(CMilBlobResult* pBlobResult);
	int GetBlobResult(double *dArea, double *dCx, double *dCy, CRect *rcRect, bool bAllBlob = false);
	int GetBlobResult_Renewal(double *dArea, double *dCx, double *dCy, CRect *rcRect, bool bAllBlob = false);
	int GetBlobResult_Area(std::vector<double> &vArea);
	int GetBlobResult_Rect(double dTeachW, double dTeachH, CRect *rcRect);
	int GetForeignBlobResult(double *dArea, double *dCx, double *dCy, CRect *rcRect);
	int SelectBlob_MaxArea(int blobCnt, Im::PIL_ID milDst);
	int SelectBlob_MaxArea(int blobCnt, Im::PIL_ID milDst, int nMaxCnt);
	int SelectBlob_Center(int blobCnt, Im::PIL_ID milDst, double center_x, double center_y);
	int SelectBlob_HiddenArea(int blobCbt, Im::PIL_ID milDst, int nTeachX, int nTeachY);

	int CalcBlob_Select(UCHAR * uArrBinSrc, UCHAR * uArrDst, int nWidth, int nHeight, int minArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType, int nTeachX = -1, int nTeachY = -1, int nBlobType = -1, double dArea = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = -1, bool bUseMultiMaxBlob = false, int nMaxCnt = -1);
	int CalcBlob_Select_new(cv::Mat& SrcImg, cv::Mat& DstImg, int nWidth, int nHeight, int minArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType, int nTeachX = -1, int nTeachY = -1, int nBlobType = -1, double dArea = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = -1, bool bUseMultiMaxBlob = false, int nMaxCnt = -1);

	void CalcHeightMean(UCHAR * pUcImgBinary, float * pfImgSrc, CRect *rcBlob, double *dHeight, int nCntBlob, int nWidth);
	void CalcHeightMean_Polygon(int nWidth, int nHeight, UCHAR * pUcImgBinary, float * pfImgSrc, ForeignPolygonArea pForeignPolygonArea, double *dHeight);
	bool PtInPolygon(POINT pt, POINT *pts, int ptNum);
		
	void GetHistoValue(Im::PIL_ID milSrc, int* retValue);
	void GetHistoValue(Im::PIL_ID milSrc, long* retValue);
	void GetHistoValue(UCHAR* userSrc, int sizeX, int sizeY, int* retValue);
	void GetHistoValue(UCHAR* userSrc, int nSttX, int nSttY, int nSizeX, int nSizeY, int* retValue);
	void GetHistoValue(long* src, int srcCount, int retCount, long* retValue);
	//void GetHistoValue(void* milSrc, int* retValue);

	void MakeHistogramImg(int* histo, int histoSize, UCHAR* userDst);


	void FillBlob_Outside(Im::PIL_ID src, Im::PIL_ID dst);
	void FillBlob_Outside(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* userDst);

public: //zamp
	Im::PIL_ID AllocBuff_float(int width, int height, int initVal = -1);
	Im::PIL_ID AllocClipBuff_float(Im::PIL_ID milSrc, int cX, int cY, int width, int height);
	Im::PIL_ID AllocRotateBuff_float(Im::PIL_ID milSrc, double angle);

	void ClipZmap(float* zmapData, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int width, int height);
	float ClipZmap_ipp(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY);;
	void RotateZmap(float* zmapData, float* dst, int orgSizeX, int orgSizeY, double angle);

	//////////////////////////////////////////////////////////////////////////

public:
	void DrawSigleRect(Im::PIL_ID milDst, int sizeX, int sizeY, CRect rect, BOOL isSave = FALSE, CString strPath = _T(""));
	void DrawRectM(Im::PIL_ID milDst, int sizeX, int sizeY, CRect* rect, int cnt, BOOL isSave = FALSE, CString strPath = _T(""));
	void DrawRectSingleM(Im::PIL_ID milDst, int sizeX, int sizeY, CRect rect, BOOL isSave = FALSE, CString strPath = _T(""));
	void DrawRectU(UCHAR* userDst, int sizeX, int sizeY, CRect* rect, int cnt, BOOL isSave = FALSE, CString strPath = _T(""));
	void DrawRectSingleU(UCHAR* userDst, int sizeX, int sizeY, CRect rect, BOOL isSave = FALSE, CString strPath = _T(""));


	long CalcAutoThreshold(long* pHistList, double dataCnt, long minValue, long maxValue);

	void GetProjection_V(Im::PIL_ID milSrc, long* retList);
	void GetProjection_H(Im::PIL_ID milSrc, long* retList);
	//shk 2014-01-03
	void GetProjection_H(Im::PIL_ID milSrc, float* retList);
	void GetProjection_V(Im::PIL_ID milSrc, float* retList);

	void DrawBlob(Im::PIL_ID milBlobResultId, Im::PIL_ID milDst, int label, BOOL reverse = FALSE);
	void DrawBlobLabels(Im::PIL_ID milBlobResultId, Im::PIL_ID milDst, BOOL reverse = FALSE);

	// LMJ 2013/11/25
	void RotateImg_ipp(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR** userDst, int* retDstSizeX = NULL, int* retDstSizeY = NULL);
	
	void RotateImg_ipp_color(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR** userDst, int* retDstSizeX = NULL, int* retDstSizeY = NULL);
	void RotateImg_ipp2(UCHAR* userSrc, UCHAR** userDst, int orgSizeX, int orgSizeY, double angle, int* retDstSizeX, int* retDstSizeY, int band = 1, int nInterPolation = 0);
	
	bool RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor = false, bool bLinear = true, bool bUseOrgSize = false);

	void RotateZmap_ipp(float* zmapData, float* dst, int orgSizeX, int orgSizeY, double angle);
	void RotateZmap_ipp2(float* zmapData, float** dst, int orgSizeX, int orgSizeY, double angle, int* retDstSizeX, int* retDstSizeY);

	bool RotateZmap_ipp2020(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize);
	
	//Foot Image Rotate
	bool RotateImg_ipp2020_2(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize, cv::Point cvPoCenter);
	bool RotateZmap_ipp2020_2(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize, cv::Point cvPoCenter);

	//huj 2013/12/20
	int ProcAnyAngle_Img(void* milSrc, UCHAR** dst, int bdryCx, int bdryCy, int bdrySizeX, int bdrySizeY, double angle, int* retDstSizeX, int* retDstSizeY);
	int ProcAnyAngle_Img(UCHAR* userSrc, UCHAR** dst, int sizeX, int sizeY, double angle, int band, int* retDstSizeX, int* retDstSizeY);
	int ProcAnyAngle_Img(UCHAR* userSrc, UCHAR** dst, int OrgSizeX, int OrgSizeY, int bdryCx, int bdryCy, int bdrySizeX, int bdrySizeY, double angle, int band, int* retDstSizeX, int* retDstSizeY);

	//lead ng save
	void DrawLeadNgRect(UCHAR* userSrc, UCHAR* userDst, int sizeX, int sizeY, int srcColorBand, CRect wndRect, CRect* leadRect, int count, int marginX, int marginY, int color);

	//shk 2014/02/04
	void GetBinaryImg_ipp(UCHAR* src, int orgSizeX,int orgSizeY, UCHAR* dst, int thresLT = 0,int LTvalue = 0,int thresGT = 255,int GTvalue = 255);

	//huj 2014/01/12

	void ResizeImg_Left(UCHAR* userSrc, UCHAR* userDst, int roiSizeX, int roiSizeY, int sizeX, int sizeY, int bandSize, double factor);
	float GetCropZmap(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY);

public:
	int GetnChannel();
	void SetnChannel(int nChannel);
	void GetnStep(int step);
	void GetClipModelImg(cv::Mat ClipModelImg);
	void SaveCVBuff(cv::Mat src, CString path);

	double SearchFocus(UCHAR* userSrc, int width, int height, int nAlgo, int nChannel);
	bool InspSolderBall(cv::Mat src, std::vector<CRect> & rcBlob, int* nMaxIndex = NULL, float* ptrResult = NULL, double* ptrArrRst = NULL);
	void GetSolderBallPos(std::vector<cv::Point>&  Points, double *dCX, double *dCY, double *dRadius, double *dErr);
	int GetBlobResult_RectsV(std::vector<CRect> & rcRect);
	int GetBlobResult_Rects(RECT *rcRect, int nRectCnt);
	void GetRotateBound(int nImgW, int nImgH, double dAngle, int* ptrDstSizeX, int* ptrDstSizeY);
	void RotateImg_IPP(UCHAR* ucSrcImg, int nSrcW, int nSrcH, double dAngle, UCHAR* ucDstImg, int nDstW, int nDstH);
	int GetBlobLabel(long *pLebel, USHORT* LabelImage, int nWidth, int nHeight);
	int SelectBlob_Area(int blobCnt, Im::PIL_ID milDst, double dArea, double dAreaWPix, double dAreaHPix, int nAreaCnt);
	int GetBlobResult_Center(double *dCenterX, double *dCenterY);
	int nCalcWidthStep(bool bUseWidthStep, int OrgImgWidth);
protected:
	


	int MakeRoughGrayImg(float* src, UCHAR* dst, UCHAR* binDst, int sizeX, int sizeY, float countThreshold);
	void GetHlimit(float* src, int sizeX, int sizeY, float* retHmax, float* retHmin);
	float GetZmapDataAvr(float* src, int sizeX, int sizeY, float noData = -1);
	float GetZmapDataAvr(float* src, int sizeX, int sizeY, float minRange, float maxRange);

	//shk 2014/04/21
	void GetMinMaxIdx(int* src,int size,int* minIdx,int* maxIdx);
public:
	void GetDrawContours(UCHAR* dst, int nSizeX, int nSizeY, int nStep, int nValue = 255);
	int GetBlobPerimeter(UCHAR* ucDst, USHORT* usDst, int nSizeX, int nSizeY, int nStep);
	float GetPerimeterLength();
	float SolderBall_Test(cv::Mat img_1, float dia, float stdscore, std::vector<CRect>& rcBlob);
	bool PCA_1D(std::vector<cv::Point> pint, float* a, float* b);
	void OLS_1D(std::vector<cv::Point> vPoint, float* a, float* b);
};


 





#pragma region _BLOB RESULT CLASS_

class CMilBlobResult
{
public:
	double* cx;			     //gravity center X
	double* cy;			     //gravity center Y
	double* left;			 //rect.left
	double* right;			 //rect.right
	double* top;			 //rect.top
	double* bottom;			 //rect.bottom
	double* width;           //rect.width()
	double* length;          //rect.height()
	double* diameter;        //Determines the average Feret diameter at all the angles checked. 
	double* angle;
	double* area;			 //blob area
	int* blobLabel;

	int count;               //blob count

public:
	CMilBlobResult(int blobCount)
	{
		count = 0;
		cx = NULL;			     
		cy = NULL;			     
		left = NULL;			 
		right = NULL;			
		top = NULL;			
		bottom = NULL;			 
		width = NULL;          
		length = NULL;         
		diameter = NULL; 
		angle = NULL;
		area = NULL;
		blobLabel = NULL;

		if(blobCount <0)
			return;

		count = blobCount;
		cx = new double[blobCount];
		cy = new double[blobCount];
		left = new double[blobCount];
		right = new double[blobCount];
		top = new double[blobCount];
		bottom = new double[blobCount];
		width = new double[blobCount];
		length = new double[blobCount];
		diameter = new double[blobCount];
		angle = new double[blobCount];
		area = new double[blobCount];
		blobLabel = new int[blobCount];
// 		cx = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		cy = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		left = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		right = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		top = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		bottom = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		width = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		length = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		diameter = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		angle = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		area = g_pMManager->pem_new<double>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
// 		blobLabel = g_pMManager->pem_new<int>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
	}

	virtual ~CMilBlobResult()
	{
		if (cx != NULL) { delete cx;	      cx = NULL; }
		if (cy != NULL) { delete cy;         cy = NULL; }
		if (left != NULL) { delete left;	      left = NULL; }
		if (right != NULL) { delete right;	  right = NULL; }
		if (top != NULL) { delete top;        top = NULL; }
		if (bottom != NULL) { delete bottom;	  bottom = NULL; }
		if (width != NULL) { delete width;      width = NULL; }
		if (length != NULL) { delete length;	  length = NULL; }
		if (diameter != NULL) { delete diameter;	  diameter = NULL; }
		if (angle != NULL) { delete angle;	  angle = NULL; }
		if (area != NULL) { delete area;	      area = NULL; }
		if (blobLabel != NULL) { delete blobLabel;  blobLabel = NULL; }
// 		if (cx != NULL) { g_pMManager->pem_delete(cx, true);	      cx = NULL; }
// 		if (cy != NULL) { g_pMManager->pem_delete(cy, true);         cy = NULL; }
// 		if (left != NULL) { g_pMManager->pem_delete(left, true);	      left = NULL; }
// 		if (right != NULL) { g_pMManager->pem_delete(right, true);	  right = NULL; }
// 		if (top != NULL) { g_pMManager->pem_delete(top, true);       top = NULL; }
// 		if (bottom != NULL) { g_pMManager->pem_delete(bottom, true);	  bottom = NULL; }
// 		if (width != NULL) { g_pMManager->pem_delete(width, true);     width = NULL; }
// 		if (length != NULL) { g_pMManager->pem_delete(length, true);	  length = NULL; }
// 		if (diameter != NULL) { g_pMManager->pem_delete(diameter, true);	  diameter = NULL; }
// 		if (angle != NULL) { g_pMManager->pem_delete(angle, true);	  angle = NULL; }
// 		if (area != NULL) { g_pMManager->pem_delete(area, true);	      area = NULL; }
// 		if (blobLabel != NULL) { g_pMManager->pem_delete(blobLabel, true);  blobLabel = NULL; }
	}



	void Copy(CMilBlobResult* refBlob, int nStartOffset)
	{
		int nIdx = 0;
		for(int i=0;i<refBlob->count;i++)
		{
			nIdx = nStartOffset + i;
			if(nIdx>=refBlob->count)
				break;
			cx[nIdx] = refBlob->cx[i];
			cy[nIdx] = refBlob->cy[i];
			left[nIdx] = refBlob->left[i];
			right[nIdx] = refBlob->right[i];
			top[nIdx] = refBlob->top[i];
			bottom[nIdx] = refBlob->bottom[i];
			width[i] = refBlob->width[i];          
			length[i] = refBlob->length[i]; 
			diameter[nIdx] = refBlob->diameter[i];  
			angle[nIdx] = refBlob->angle[i]; 
			area[nIdx] = refBlob->area[i];
			blobLabel[nIdx] = refBlob->blobLabel[i];
		}
	}

	CMilBlobResult& operator =(const CMilBlobResult& refBlob)
	{
		for(int i = 0; i < refBlob.count; i++)
		{
			if(i == count)
				break;		

			cx[i] = refBlob.cx[i];			     
			cy[i] = refBlob.cy[i];			     
			left[i] = refBlob.left[i];			 
			right[i] = refBlob.right[i];			
			top[i] = refBlob.top[i];			
			bottom[i] = refBlob.bottom[i];			 
			width[i] = refBlob.width[i];          
			length[i] = refBlob.length[i];         
			diameter[i] = refBlob.diameter[i];     
			angle[i] = refBlob.angle[i];  
			area[i] = refBlob.area[i];
			blobLabel[i] = refBlob.blobLabel[i];
		}	
		return *this;
	}
};

#pragma endregion _BLOB RESULT CLASS_

