#pragma once

#include "InspParamDef.h"
#include "Proc3D.h"
#include "ProcMil.h"


#define	_mm2micron(val)    (val * 1000.0)
#define	_micron2mm(val)    (val / 1000.0)
// #define _pixel2mm(val)	   (val * 0.02)
// #define _mm2pixel(val)	   (val / 0.02)
#define _radian2degree(val)			(val * (180.0 / 3.14))
#define _degree2radian(val)			(val * (3.14 / 180.0))
 
#define	_limit_value(val, _min, _max) (val < _min) ? _min : ((val > _max) ? _max : val)
#define NoSolderStandardValue 5

enum teach_type
{
	eTEACH_MOUNT = 0,
	eTEACH_OCR,
	eTEACH_COLOR,
	//eTEACH_SOLDER,
	//eTEACH_LEAD,
	eTEACH_PATTERN,
	eTEACH_LEADSOLDER,
	eTEACH_BGA,
	eTEACH_CNT,
};


enum part_result
{
	ePART_SUCCESS = 0,
	ePART_FAIL
};

enum insp_mode
{
	eMODE_AUTO = 1,
	eMODE_TEACH = 2
};

enum eForeign2DInspMode
{
	e2DInspColor = 0,
	e2DInspGray,	
	e2DInspCopper,	
	e2DInspGrayAdd3D,
	e2DInspColorAdd3D,
	e2DInspGrayBub,
};

enum eBlobInspMode
{
	eBlobInspMode_Origin = 0,
	eBlobInspMode_Solderball,
	eBlobInspMode_Coil,
};

class CPInsp
{
public:
	CPInsp(void);
	virtual ~CPInsp(void);


	enum EDirection
	{
		eLeft,
		eRight,
		eTop,
		eBottom
	};

protected:
	
	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;
	CString m_className;

	//3D inspection Class
	CProc3D m_proc3d;
	CProcMil *m_pProcMilAlgo;

// 	MIL_ID* m_milApp;	// mil application
// 	MIL_ID* m_milSys;	// mil system

public:
	virtual int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int foxWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	virtual int CloseDevice();
	
	double GetResolX() const { return m_resolX; }
	double GetResolY() const { return m_resolY; }
	CProc3D GetProc3D() const { return m_proc3d; }
	CProcMil* GetProcMil() const { return m_pProcMilAlgo; }

	double	_pixel2mm_x(double val);	 
	double	_mm2pixel_x(double val);	
	double	_pixel2mm_y(double val);	 
	double	_mm2pixel_y(double val);	

	void Binarize(const UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nTypeRange, int nThreshLow, int nThreshHigh, BOOL bInvert, UCHAR* &pByImgDst);
	void Binarize(const float *pfImgSrc, int nImgWidth, int nImgHeight, int nTypeRange, float fThreshLow, float fThreshHigh, BOOL bInvert, UCHAR* &pByImgDst, BOOL bUseExceptHeight = false, float fHeightMin = 0.f, float fHeightMax = 0.f, bool bExceptZero = false);
	void GetGrayROI2D(UCHAR *pucImgSrc, UCHAR *pucImgDst, int nOrgWidth, RECT rcROISize);
	void GetHeighROI3D(float *pfImgSrc, float *pfImgDst, int nOrgWidth, RECT rcROISize);
	void GetHeightMinMax( float *pfArrImgData, int nImgWidth, int nImgHeight, float *fHeightMin, float *fHeightMax);
	float GetHeightMean(float *pfArrImgData, int nImgWidth, int nImgHeight, int nExceptCount = 0, bool bOneMore = false);
	float GetHeightAvgMask(float *pfArrImgData, UCHAR * ucMask, int nImgWidth, int nImgHeight, int nStX, int nStY, int roiSizeX, int roiSizeY);
	float GetMinMaxImg( float *pfArrImgData, int nImgWidth, int nImgHeight,int Min);
	float GetHeightMaxHist( float *pfArrImgData, int nImgWidth, int nImgHeight );
	float GetHeightMaxHist2( float *pfArrImgData, int nImgWidth, int nImgHeight );
	float GetHeightAvgMask( float *pfArrImgData, int nImgWidth, int nImgHeight ,UCHAR* BinImg);
	void GetBlobArea(UCHAR *pucImgSrc, int nSrcWidth, int nSrcHeight, double dSrcCx, double dSrcCy, double dBlobW, double dBlobH, double dWndAngle, double *rstW, double *rstH, int nOkPercent = 50);
	double GetGradient(double *ptrnLineX, double *ptrnLineY, double nTotalCnt, double &pdA, double &pdB, bool bHorizon);
	POINTF GetIntersectionOfTwoStraightLines(double *ptrdLineDA, double *ptrdLineDB, BOOL *ptrbHorizon);

	void Set_PseudoResol(float fMin, float fMax);
	int Get_PseudoColorMap(float * fZmapSrc, UCHAR * ucMask, int width, int widthstep, int length, UCHAR* ucArrDstImg);
	AlgoBlob SetAlgoBlob(const InspAlgo &sInspAlgo, int nIdx = 0);
	int GetCorrectIDX(int nCnt, double* dArrSrc, int* nArrIDX, bool bMax = true);
	void GetLabelImage(long lLabel, USHORT *LabelImage, int nW, int nH, CString sFileName);
	RECT GetBlobRect(long *pLebel, USHORT* LabelImage, int nCntBlob, int nW, int nH, int nCX, int nCY, int nRoiW, int nRoiH);

	void ExceptPointed(cv::Mat* binImg,float* HSrc);
	void ROIAnglePointChange(double dAngle, const POINTF rcSrc, POINTF *rcDst);
	int BlobImageStruct_Base(AlgoBlobBase &algo, UCHAR *pucImgSrc, float *pfImgSrc, UCHAR *ucArrColorImg, int nWidth, int nHeight, TotalInspExceptArea stTieArea, UCHAR *pucImgDst);
	int BlobImageStruct_Base(AlgoBlobBase &algo, cv::Mat img2D, cv::Mat img3D, cv::Mat imgColor, int nWidth, int nHeight, TotalInspExceptArea stTieArea, cv::Mat imgRst, bool bBin);
	void GetBlobPolygon(UCHAR * uArrDst, int nWidth, int nHeight, int nStartX, int nStartY, vector<vector<POINTF>>* vPolygons, vector<POINTF>* vPolyCenter);
	void GetInspectArea(UCHAR *pucSrc, int nImgW, int nImgH, int nDir, int nInspectionAreaPer, int nBand = 1);
	cv::Mat CPInsp::FillPolygon(cv::Mat binImg, int width, int height, POINT* ptDstPoint, bool isExceptROI = false, int nUsedPolygon = 0, int* cnt = nullptr, UCHAR * ucArrOverlapImg = nullptr, int nChanel = 1);
	cv::Mat CPInsp::FillPolygonImg(cv::Mat ucMask, cv::Mat binImg, int width, int height, POINT* ptDstPoint, bool isExceptROI = false, int nUsedPolygon = 0, int* cnt = nullptr, UCHAR * ucArrOverlapImg = nullptr, int nChanel = 1, int nExcept = 1);

	BOOL InspectionLine(int nWndWidth, int nWndHeight, UCHAR *ucArrDstWnd, bool bIsHorizon, int nMeasureDirection, double* dAngle, POINTF* poDrawLine, int* nLineTotalLength, double* dAValue, double* dBValue, int nLineFindType = 0, double dLineFindRate = 100, int nInspOption = 0);
	BOOL InspectionLine2(int nWndWidth, int nWndHeight, UCHAR *ucArrDstWnd, bool bIsHorizon, int nMeasureDirection, double* dAngle, POINTF* poDrawLine, int* nLineTotalLength, double* dAValue, double* dBValue, int nLineFindType = 0, double dLineFindRate = 100);
	bool BlobCircleCenter_Fit(cv::Mat src, CRect *rcBlob, double& dCircle_x, double& dCircle_y, double* dRadius, double* dErrorRate, double *dArea, int nMinBlobArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType,
		int nTeachX = -1, int nTeachY = -1, int nBlobType = -1, double dAreaPix = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = -1);
	//int nTeachX = -1, int nTeachY = -1, int nBlobType = -1, double dArea = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = -1, bool bUseMultiMaxBlob = false, int nMaxCnt = -1
	BOOL AnglePosChange(const int nAngle, const int nWidth, const int nHeight, const POINTF poSrcPoint, POINTF *poDstPoint);
	double GetBWImageStruct(AlgoBaseBW &pInspAlgoBW, UCHAR *pucImgSrc, float *pfImgSrc, int nWidth, int nHeight, UCHAR*& ucArrDstImg, BOOL bUseColor = FALSE, UCHAR* ucArrColorImg = NULL, int nDir = 10, int nInspAreaPer = 100);
	double GetBWImageStruct(AlgoBlackWhite &pInspAlgoBW, WndAlgoImg &sWndAlgo, UCHAR *ucArrDstImg, TotalInspExceptArea stTieArea, UCHAR* ucArrColorImg = NULL, int nDir = 10, int nInspAreaPer = 100, UCHAR *ucArrDstImgAI = NULL);
	int BlobImageStruct(AlgoBlob &algoBlob, UCHAR *pucImgSrc, float *pfImgSrc, UCHAR *ucArrColorImg, int nWidth, int nHeight, int nMinBlobArea,
		double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, TotalInspExceptArea stTieArea,
		BOOL bApplyFillHole, BOOL bAlignAlgo = FALSE, int nTeachX = -1, int nTeachY = -1, BOOL eraseBorderBlob = FALSE, double dAreaPix = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = 0);
	int BlobImageStruct_BW(AlgoBaseBW &pInspAlgoBW, UCHAR *pucImgSrc, float *pfImgSrc, int nWidth, int nHeight, int nMinBlobArea,
		double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, int nTypeSelectBlob, bool bFilterIsUse, int nFilterStepNarrow, TotalInspExceptArea stTieArea,
		BOOL bApplyFillHole, BOOL bAlignAlgo = FALSE, UCHAR* ucArrColorImg = NULL);
	int BlobImageStruct_BW2(AlgoBaseBW &pInspAlgoBW, UCHAR *pucImgSrc, float *pfImgSrc, int nWidth, int nHeight, int nMinBlobArea,
		double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, int nTypeSelectBlob, bool bFilterIsUse, int nFilterStepNarrow, TotalInspExceptArea stTieArea,
		BOOL bApplyFillHole, int nDir = -1, int nInspAreaPer = 100, UCHAR* ucArrColorImg = NULL);
	int CalcBlob(UCHAR* uArrBinSrc, UCHAR* uArrDst, int nWidth, int nHeight, int nMinArea, BOOL bFillHole, int nFilter, int nSelectType, double* dArea, double* dCx, double* dCy, CRect* rcRect, int nTeachX = -1, int nTeachY = -1, double dSArea = -1);
	int GetBlobResult(cv::Mat imgBlob, double* dArea, double* dCx, double* dCy, CRect* rcRect);
	int GetBlobResult_Rects(cv::Mat imgBlob, std::vector<CRect>& rcRect);
	int BlobFillOutImage(AlgoBlob &algoBlob, UCHAR *pucImgSrc, float *pfImgSrc, UCHAR *ucArrColorImg, int nWidth, int nHeight, int nMinBlobArea,
		double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, TotalInspExceptArea stTieArea,
		BOOL bApplyFillHole, BOOL bAlignAlgo = FALSE, int nTeachX = -1, int nTeachY = -1, BOOL eraseBorderBlob = FALSE, double dAreaPix = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = 0, UCHAR* ucArrFillOutImg = NULL, UCHAR* ucBinImg = NULL, UCHAR* ucMask = NULL);
	BOOL AngleRectChange(const int nAngle, const int nWidth, const int nHeight, const RECT rcSrcRect, RECT *rcDstRect);
	BOOL AngleRectChange2(const double dAngle, const int nWidth, const int nHeight, const RECT rcSrcRect, RECT *rcDstRect);
	BOOL AnglePointChange(const int nWidth, const int nHeight, const POINTF poSrcPoint, POINTF *poDstPoint);
	BOOL AnglePointChange_mm(const float nWidth, const float nHeight, const POINTF poSrcPoint, POINTF *poDstPoint);
	BOOL IsAnyAngle(double angle);
	// for masking(except area)
	int FillMaskingROI(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL);
	int FillMaskingROI(int nWidth, int nHeight, double dAngle, float *fArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL);
	int FillMaskingROIUsingOpenCV(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL, int nChanel = 1);
	void FillMaskingROIUsingOpenCVImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL, int nChanel = 1, UCHAR *ucArrFillOutImg = NULL, UCHAR *ucMask = NULL);
	int FillMaskingROI(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const std::vector<RECT> rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL);
	int FillMaskingROI(int nWidth, int nHeight, double dAngle, float *fArrSrcImg, int nUsedMaskingValue, const std::vector<RECT>rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL);
	int FillMaskingROIUsingOpenCV(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const std::vector<RECT>rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL, int nChanel = 1);
	void FillMaskingROIUsingOpenCVImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const std::vector<RECT>rcArrMaskingROI, int nFillValue = 0, UCHAR *ucArrOverlapImg = NULL, int nChanel = 1, UCHAR *ucArrFillOutImg = NULL, UCHAR *ucMask = NULL);
	int FillOutOfInspAreaImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue = 0, BOOL bConvertExceptROI = FALSE, int nChanel = 1, UCHAR *ucArrFillOutImg = NULL, UCHAR *ucBinImg = NULL, UCHAR *ucMask = NULL, int nExcept = 0);
	int FillOutOfInspArea(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue = 0, BOOL bConvertExceptROI = FALSE, int nChanel = 1);
	int FillOutOfInspArea(int nWidth, int nHeight, double dAngle, float *fArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue = 0, BOOL bConvertExceptROI = FALSE, int nChanel = 1);
	int FillOutOfInspAreaCombine_Color(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, TotalInspExceptArea stTieArea);
	int FillOutOfInspAreaCombine(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, TotalInspExceptArea stTieArea);
	int FillOutOfInspAreaCombineImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, TotalInspExceptArea stTieArea, UCHAR *ucArrFillOutImg, UCHAR *ucBinImg, UCHAR *ucMask);
	float CalcTiltAngle(float *pfArrImgData, int nImgWidth, int nImgHeight, double* A, double* B, double* C);
	float LeastSquarePlane(int cnt, float *height, POINTF* pint, double* A, double* B, double* C);
	int GetRGB(float fHei, UCHAR * ucArrDstImg, int nIndex);
	int SetRGB(UCHAR * ucArrDstImg, int nIndex, float fH, float fMin, float fMax);
	int SetRGB(UCHAR * ucArrDstImg, long long nIndex, float fH, float fMin, float fMax);
	void CircleIntersection(UCHAR* pucBin, int nW, int nH, double *centX, double *centY, double *radius, double *err, double ini_radius, int nFindPer, float fMin, float fMax);
	bool GetROICenter(int nSelectBlob, cv::Mat img, POINTF* poArrDstPoint);
	BOOL ImageThining(int nWndWidth, int nWndHeight, UCHAR *ucArrSrcWnd, UCHAR *ucArrDstWnd, bool bnot = false);
	bool BinCheck(float fVal, int nRange, double dL, double dH);
	void Binarize(cv::Mat imgSrc, cv::Mat imgRst, int nRange, double dL, double dH, BOOL b3D = FALSE);
	void Binarize(cv::Mat img2D, cv::Mat img3D, cv::Mat imgRst, int n2DRange, double n2DL, double n2DH, int n3DRange, double n3DL, double n3DH);
	POINTF GetIntersection(POINTF A1, POINTF A2, POINTF B1, POINTF B2);
	double GetAngleBetweenLines(POINTF poA, POINTF poB, POINTF poC);
	double Mod(double a, double n);
protected:
	
	double _GetColor(int color);

	void CvtPixelToBoard(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double* retX, double* retY, int mode = DOWN_LEFT) ;
	int GetLightIndex(int channelIndex);
	double CalcRotateAngle(double orgAngle);
	
	//bool BlobCircleCenter(UCHAR* src, double& dCirle_x, double& dCirle_y, double* dRadius, double* dErrorRate, bool bDetail);
	bool BlobCircleCenter(cv::Mat src, double& dCirle_x, double& dCirle_y, double* dRadius, double* dErrorRate, bool bDetail);
	void GetFiduPos_LSQ(std::vector<cv::Point>&  Points, double *centX, double *centY, double *radius, double *err);
	void MappingExceptSolderArea(UCHAR *ucArrDstImg, int nWidth, int nHeight, int *pnExceptMatchingMaps,CRect *rcBlob, CRect *rtSolderFillRect, double *dSolderHeight,
		double dUserSolderMin, int nCntBlob, int nSolderCnt, BOOL bUseInit);
	int CalcLeadSolderViewAngle(int nPosition, double dAngle);
	BOOL AngleStartChange(const int nAngle, const int nWidth, const int nHeight, const int nDir, const RECT rcSrcRect, int *ptrnOutStartX, int *ptrnOutStartY);
	bool ExtractBody(UCHAR *pucImgSrc, int nWidth, int nHeight, CRect rcBlobSrc, CRect &rcBlobDst, int nOkPercent);
	float GetImageROIHeightAvg(float *pfImgSrc, int nWidth, int nHeight, RECT rcROI, float fDefaultHeight);
	
	float GetHeightMean_rate( float *pfArrImgData, int nImgWidth, int nImgHeight, float stdMeanHeight, double rate);
	void SetForeignResultRect(CRect *rcRectSource, CRect *rcRectDes, int nSrcCnt, int nDesCnt);
	void SetForeignResultValue(double *dAreaSrc, double *dCxSrc,double *dCySrc, double *dAreaDes, double *dCxDes,double *dCyDes, int nSrcCnt, int nDesCnt);
	void SetForeignResultRect(CRect rcRectSource, CRect *rcRectDes, int nDesCnt);
	void SetForeignResultValue(double dAreaSrc, double dCxSrc,double dCySrc, double *dAreaDes, double *dCxDes,double *dCyDes, int nDesCnt);

	bool PtInPolygon(POINT pt, POINT *pts, int ptNum);
	bool PtInPolygon(POINT pt, POINTF *pts, int ptNum);

	void InitExceptMatchingMaps(int *pnExceptMatchingMaps, int nCntBlob);
private:
	#define CR_MAP_SIZE 1280
	struct PseudoColor
	{
		int r;
		int g;
		int b;
	};
	PseudoColor m_crPseudo[CR_MAP_SIZE];
	float m_fMinZ;
	float m_fMaxZ;
	float m_fDivZ;
	void Init_PseudoMap();
	double CalcBWInspectArea(UCHAR *pucBWSrc, int &nImgW, int &nImgH, int nDir, int nInspectionAreaPer, int &pnROIValue);

public:
	bool CrossLineDetect(uchar *src, int nWidth, int nHeight, int nCntBlob, double *dArea, double *dCenterX, double *dCenterY, RECT *rcRect, double dMaxThickness, double dMaxRatio /* ~ INPUT*/
		, /*OUTPUT ~ */ int *arrLabelVert, int *arrLabelHorz, int *arrLabelVertCnt, int *arrLabelHorzCnt, double *arrLabelArea, RECT *arrLabelRect);
};

