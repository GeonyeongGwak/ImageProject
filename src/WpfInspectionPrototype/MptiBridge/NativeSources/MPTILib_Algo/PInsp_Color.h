#pragma once
#include "pinsp.h"
#include "Proc3D.h"
#include "ProcMil_Color.h"


#define     HISTO_SIZE      256
#define		COLORXY_WIDTH	256
#define		COLORXY_LENGTH	256

#define    DEF_INSP		0
#define    DEF_TEACH	1

#define SIZE_CLRX	(1000)	// (CIE)	// 256 (RB)
#define SIZE_CLRY	(1000)

#define  CIE_OFFSETX		235
#define  CIE_OFFSETY		205


// CIE model - RGB to XYZ	// YJS 2016/11/04 : 헤더로 옮김
#define CIE_XR	(0.490f)
#define CIE_XG	(0.310f)
#define CIE_XB	(0.200f)
#define CIE_YR	(0.177f)
#define CIE_YG	(0.813f)
#define CIE_YB	(0.011f)
#define CIE_ZR	(0.000f)
#define CIE_ZG	(0.010f)
#define CIE_ZB	(0.990f)

#define CIE_XYZ	(0.03f)
#define CIE_RGB	(35.0f)
enum colorxy_mode
{
	eCOLORXY_PROC = 0,
	eCOLORXY_VIEW
};

enum eForeign3DMaskInfo
{
	e3DMaskInspArea = 0,
	e3DMaskPartArea = 1,	
	e3DMaskHoleArea = 2,	
	e3DMaskOutOfPCB = 10,	
	e3DMaskGerberHole = 20,
};


enum color_stepID
{
	eCStepID_ANALYSIS_COLOR= 0,
	eCStepID_ANALYSIS_COLOR_ROTATE,
};

enum color_result
{
	eCOLORRST_COLORXY_NG = -1,
	eCOLORRST_HISTO_NG,
	eCOLORRST_OK 
};

enum color_HistoRangeMode
{
	eHISTO_IN_RANGE = 0,
	eHISTO_OUT_RANGE,
	eHISTO_UPPER,
	eHISTO_LOWER,
};

enum colorInspType
{
	e_COLORXY = 0,
	e_HISTOGRAM,
};
enum m_eDefaultAC
{
	eDefaultAC_Data = 0,
	eDefaultAC_BW,
	eDefaultAC_Total,
};
class CPInsp_Color : public CPInsp
{
public:
	CPInsp_Color(void);
	virtual ~CPInsp_Color(void);
	float m_fFatorR;
	float m_fFatorG;
	float m_fFatorB;
	float m_fFatorACR;
	float m_fFatorACG;
	float m_fFatorACB;
	float m_fFatorMGR;
	float m_fFatorMGB;
	float m_fFatorBR;
	float m_fFatorBB;
	float m_fFatorBGR;
	float m_fFatorBGB;
	lightData m_sLightData[3];
	byte m_byDefaultAC[2];
protected:

	int m_mainLightIndex;
	//////Im::PIL_ID
	void* m_imgBuf[eImg_BufCnt];

	void* m_pFovImage_insp_R;
	void* m_pFovImage_insp_B;

	void* m_pFovImage_teach_R;
	void* m_pFovImage_teach_B;

	void* m_pFovImage_insp_Main;
	void* m_pFovImage_teach_Main;
	///////////////////////////////

	Coordinate m_inspCoordinate;
	Coordinate m_teachCoordinate;

	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;

	// LMJ 2015/04/12
	UCHAR * m_pPartImgBuf[eImg_BufCnt];
	int m_nPartWidth;
	int m_nPartHeight;
	BOOL m_bIsBigPart;	// Teach

	UCHAR* m_dispCIE_org;

protected:
	InspParamColor* m_pInspParam;
	TeachParamColor* m_pTeachParam;

	InspParamColor m_paramInspect;
	TeachParamColor m_paramTeach;

	RstInspColor m_resultClr;

	//3D inspection Class
	CProc3D m_proc3d;

	CProcMil_Color* m_procMil;
	
	POINTF* m_pForeignExceptRoiPos;
	POINTF* m_pForeignExceptRoiSize;

	AlgoColor *m_pAlgoColor;
	WndAlgoImg m_wndAlgoImg;
	InspAlgoType m_eAlgoType;
	TotalInspExceptArea m_stTieArea;

private:
	AlgoColorXY *m_pAlgoColorXY;
private:
	UCHAR* m_ucCIETab;
	UCHAR* m_ucMap;
protected:
	void SetParamChannelImg( const InspAlgo &sInspAlgo, const AlgoCoordinate &coordinateAlgo, BOOL bTeach=TRUE , bool bIsLoadFovRawDataImage = false, int projectionmode = 0, int nInspCameraType = -1, int nFovID = -1);

	int GetWidthImg(Coordinate coordinate);
	int GetHeightImg(Coordinate coordinate);

	void Color_CieViewLoad();
public:
	virtual int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	virtual int CloseDevice();
	int SetInspParam(void* itemParam, Coordinate cdn, InspImgBuf img, int mainImgIndex);
	int SetInspParam(void* itemParam, void* targetImg_R, void* targetImg_B, void* targetImg_Main, Coordinate cdn);
	int SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const WndInfo &wndInfoAnyAngle, const InspPartInfo *pInspBoardInfo, int nLeadTipPos=-1 );
	int SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, int nLeadTipPos, RECT* rcLeadInsp = NULL, int nInspCameratype = eMSCN_COAXIAL, InspRoiImgBuf *sInspImageData = NULL, TotalInspExceptArea *stTieArea = NULL);
	
	void ClipWndAlgoImgLead(int nLeadTipPos, RECT* rcLeadInsp = NULL);
	BOOL GetColorBaseCIE(InspAlgo &sInspAlgo, AlgoColorOpt &sAlgoColorOpt, UCHAR* retDstImg, RstAlgoColorOpt& pRstData);
	BOOL GetColorBaseBin(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg, bool bTeach = false, int nInspAC = 0, UCHAR* retDstImgAC = NULL, int nIDX = -1, int nSideCameraIndex = -1, TotalInspExceptArea *stTieArea = NULL, bool bSaveR = false);
	BOOL GetColorFromXYZ(POINTF ptfLocation, int* arrRGB);
	BOOL GetColorImg(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg);
	BOOL GetGrayMeanImg(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg);
	int GetSelectColorImg(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, UCHAR* retImg);
	int GetSelectGrayMeanImg(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, UCHAR* retImg);
	int GetSelectColorImg_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float fitR, float fitG, float fitB, float fitBR, float fitBB, 
		float compoBtmR, float compoBtmG, float compoBtmB, int sizeX, int sizeY, UCHAR* retImg);
	int GetSelectGrayMeanImg_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float fitR, float fitG, float fitB, float fitBR, float fitBB, 
		float compoBtmR, float compoBtmG, float compoBtmB, int sizeX, int sizeY, UCHAR* retImg);

	int GetColorBin(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, AlgoColorBase *pInspAlgoColor, UCHAR* retDstImg, double* pRetRate, double dWhiteA);
	int AutoTeachingColor(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, RstAlgoColorOpt &pRst);
	BOOL CheckThreadShapeArea(int nImgWidth, int nImgHeight, int nTargetWidth, int nTargetHeight, int nMinPixel);
	
	int SetTeachParam(void* itemParam, Coordinate cdn, InspImgBuf img, int mainImgIndex);
	int SetTeachParam(void* itemParam, void* targetImg_R, void* targetImg_B, void* targetImg_Main, Coordinate cdn, int teachType = -1);
	virtual int SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const WndInfo &wndInfoAnyAngle, TotalInspExceptArea stTieArea, int nLeadTipPos=-1, bool bIsBigPart = false, bool bIsLoadFovRawDataImage = false, int projectionmode = 0 , int nInspCameraType = -1, int nFovID = -1);

	virtual int InspProc();		// YJS 2016/11/04 : 이름 변경
	virtual int ProcAuto();
	virtual int ProcStep(int stepID);
	
	int Color_Teach(InspRoiImgBuf &sInspRoiImg, bool bIsLoadFovRawDataImage = false, int nSelectIdx = 0, BOOL bSideCamera = FALSE);
	
	int Color_GetColorCIEArea(int dstCieSizeX, int dstCieSizeY, RECT rcRoi, UCHAR* src_R, UCHAR* src_G, UCHAR* src_B, POINT* pt, float fFitR, float fFitG, float fFitB, int widthStep);
	int Color_GetColorCIEView(UCHAR* retCIEImg, int dstCieSizeX, int dstCieSizeY, UCHAR* retImg, int dstSizeX, int dstSizeY, POINT* pt, bool bIsLoadFovRawDataImage = false, InspRoiImgBuf *sInspRoiImg = NULL);
	int Color_GetColorxyView(UCHAR* retColorxy, UCHAR* retImg, int dstSizeX, int dstSizeY, POINT* pt);
	int Color_GetSelectColorView(UCHAR* retImg, int sizeX, int sizeY);
	int Color_GetColorHistogramView(UCHAR* retHistoImg, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg);
	int Color_GetSelectHistogramView(UCHAR* retImg, int sizeX, int sizeY);

	//int InspDecision(double dCurRatio, double dStdRatio);
	int InspDecision(int eInspType, int nRangeMode, double dCurRatio, double dStdRatio);
public:
	int ColorCIETeach(InspRoiImgBuf &sInspRoiImg, bool bIsLoadFovRawDataImage = false, int nSelectIdx = 0, BOOL bSideCamera = FALSE);
	cv::Rect GetLeadColorROI(InspRoiImgBuf &sInspRoiImg);
	double GetColorRate();
	int ColorxyTeach();
	int HistogramTeach();
	int AnalysisColorCIE(BOOL bRotateImg=FALSE, int nSelectPolygon = 0);
	int AnalysisColorXY();
	int AnalysisHistogram(BOOL bRotateImg=FALSE);
	double CompareCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, BYTE* lut, int width, int length, int &nCntValid);
	double CompareCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, BYTE* lut, int width, int length, float *zmap, float limitHeight);
	double CompareCIE_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
		float compoBtmR, float compoBtmG, float compoBtmB, int width, int length, const POINT *pts, int ptNum, int &nCntValid, double dWhiteA);
	double CompareCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, int width, int length, const POINT *pts, int ptNum, int &nCntValid, double dWhiteA);
	POINTF GetCIEPoint(cv::Mat* pConvertMat, double dRed, double dGreen, double dBlue, int nPixelCount);
	int CompareCIE_Image(InspRoiImgBuf sInspImg, AlgoColorOpt &sAlgoColorOpt, const POINT *ptPolygon, UCHAR* retDstImg);
	int CompareCIE_CompoBtm_Image(InspRoiImgBuf sInspImg, AlgoColorOpt &sAlgoColorOpt, const POINT *ptPolygon, UCHAR* retDstImg);
	double CompareData(UCHAR* colorXY, BYTE* lutData, int xyWidth, int xyLength, int roiArea);
	int GetColorxy(void* srcR, void* srcB, UCHAR* dst, int cx, int cy, int sizeX, int sizeY);
	int GetColorCIE(void* srcR, void* srcG, void* srcB, float fitR, float fitG, float fitB, UCHAR* dst, int cx, int cy, int sizeX, int sizeY);
	int CPInsp_Color::GetSelectColorCIEImg(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float fitR, float fitG, float fitB, int sizeX, int sizeY, POINT* pt, int ptCnts, UCHAR* retImg);
	int CPInsp_Color::GetSelectColorFromXYZ(POINTF ptfLocation, int* arrRGB);
	int GetSelectColorCIEImg_CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float fitR, float fitG, float fitB, float fitBR, float fitBB, 
		float compoBtmR, float compoBtmG, float compoBtmB, int sizeX, int sizeY, POINT* pt, int ptCnts, UCHAR* retImg);
	int GetSelectColorImg(UCHAR* retImg, POINT* pt, int ptCnts);
	int GetSelectHistoImg(UCHAR* srcImg, int srcSizeX, int srcSizeY, UCHAR* retDstImg, int dstSizeX, int dstSizeY, int rangeMode, int maxRange, int minRange);
	int ImageToColorxy(UCHAR* img_R, UCHAR* img_B, UCHAR* dst, UCHAR* dst2, int width, int length, int mode = eCOLORXY_PROC);

	int ImageToColorCIE_Rect(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, RECT rcRoi, int widthStep);
	int ImageToColorCIE(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, int width, int length);
	int ImageToColorCIE_CompoBtm(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB, float factorR, float factorG, float factorB, 
		float factorBR, float factorBB, float compoBtmR, float compoBtmG, float compoBtmB, UCHAR* dst, int width, int length);
	int ImageToColorCIE_Color(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, int width, int length);
	int ImageToColorCIE_Color_CompoBtm(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
		float compoBtmR, float compoBtmG, float compoBtmB, UCHAR* dst, int width, int length);
	int ColorCIEToImage(UCHAR* colorXY, UCHAR* img_R, UCHAR* img_G , UCHAR* img_B, UCHAR* dst, int width, int length);
	int ColorxyToImage(UCHAR* colorXY, UCHAR* img_R, UCHAR* img_B, UCHAR* dst, int width, int length);
	int RemakeColorCIE(UCHAR* src, UCHAR* dst, const POINT *pts, int ptNum);
	int RemakeColorxy(UCHAR* src, UCHAR* dst, const POINT *pts, int ptNum);
	int MakeLUT_Poly(BYTE* dst, const POINT *pts, int ptNum);
	int MakeLUT_Poly(BYTE* dst, int sizeX, int sizeY, const POINT *pts, int ptNum);
	int MakeLUT_Blob(UCHAR* srcColorXY, BYTE* dst);
	inline bool PtInPolygon(POINT pt, const POINT *pts, int ptNum);
	bool pvProcPointInPolygon(POINTF* ptArray, int nArrayCts, POINTF point);
	void InitResult(RstInspColor* ret);
	//void GetInspRst(RstInspColor* ret);
	virtual void GetInspRst( InspAlgoResult &inspAlgoResult, RECT rcLeadInsp );
	BOOL DataAnalysis(UCHAR data, int maxData, int minData, int mode);

	virtual void RemakePoly(POINT* srcPoint, int count, int offsetX, int offsetY, POINT* retPoint);

	void MakeProcImg(UCHAR * src1, UCHAR* src2, UCHAR* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi, UCHAR * src4 = NULL, UCHAR * src5 = NULL, UCHAR * dst4 = NULL, UCHAR * dst5 = NULL);
	void MakeProcImg(void* src1, void* src2, void* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi, bool bIsLoadFovRawDataImage = false, bool bForeign = false, 
		void* src4 = NULL, void* src5 = NULL, UCHAR* dst4 = NULL, UCHAR* dst5 = NULL, void* src6 = NULL, void* src7 = NULL, void* src8 = NULL, UCHAR* dst6 = NULL, UCHAR* dst7 = NULL, UCHAR* dst8 = NULL);
	void MakeProcImg_Clip( UCHAR * src1, UCHAR* src2, UCHAR* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, int nSX, int nSY, int roiSizeX, int roiSizeY, 
		UCHAR* src4 = NULL, UCHAR* src5 = NULL, UCHAR* dst4 = NULL, UCHAR* dst5 = NULL, int nImgW = -1, int nImgH = -1);
	void MakeProcImg_Clip( void *pSrc1, void *pSrc2, void *pSrc3, UCHAR* pDst1, UCHAR* pDst2, UCHAR* pDst3, bool bIsLoadFovRawDataImage, int nSX, int nSY, double roiSizeX, double roiSizeY, 
		void* pSrc4 = NULL, void* pSrc5 = NULL, UCHAR* pDst4 = NULL, UCHAR* pDst5 = NULL, void* pSrc6 = NULL, void* pSrc7 = NULL, void* pSrc8 = NULL, UCHAR* pDst6 = NULL, UCHAR* pDst7 = NULL, UCHAR* pDst8 = NULL);
	void MakeProcImg_Big_Teach(UCHAR * src1, UCHAR* src2, UCHAR* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi, UCHAR* src4 = NULL, UCHAR* src5 = NULL, UCHAR* dst4 = NULL, UCHAR* dst5 = NULL, UCHAR * src6 = NULL, UCHAR* src7 = NULL, UCHAR* src8 = NULL, UCHAR * dst6 = NULL, UCHAR* dst7 = NULL, UCHAR* dst8 = NULL);
	int GetHistogramCount( int rangeMode, int maxRange, int *histo, int minRange );
	bool IsBlack(double dR, double dG, double dB, int nThresholdType, double dMin, double dMax);	

	BOOL InspColorXY(const InspAlgo &sInspAlgo, const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, RstAlgoColorXY *sRstAlgo, byte byInspType = 0);
	BOOL TeachColorXY(const InspAlgo &sInspAlgo, const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, RstAlgoColorXY *sRstAlgo, float* nGapXMin, float* nGapXMax, float* nGapYMin, float* nGapYMax, BOOL bIsLoadFovRawDataImage);
	POINTF CompareXYAvg(UCHAR* srcTR, UCHAR* srcTG, UCHAR* srcTB, UCHAR* srcBB, float factorTR, float factorTG, float factorTB, float factorBB, int width, int length);
	void MakeProcImg(void* src, UCHAR* dst, Coordinate coordi, bool bIsLoadFovRawDataImage);
	void MakeProcImg(UCHAR * src, UCHAR* dst, Coordinate coordi);
	void MakeProcImg_Clip( void *pSrc, UCHAR* pDst, bool bIsLoadFovRawDataImage, int nSX, int nSY, double roiSizeX, double roiSizeY );
	void MakeProcImg_Clip( UCHAR * src, UCHAR* dst, int nSX, int nSY, int roiSizeX, int roiSizeY );
	void MakeProcImg_Big(UCHAR * src, UCHAR* dst, Coordinate coordi);
	int GetAngleColor(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float fitR, float fitG, float fitB, int nImgWidth, int nImgHeight, UCHAR* retDstImg);
	BOOL GetAngleColor(InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg);
	cv::Mat GetGreenImage(UCHAR* pucR, UCHAR* pucB, int nW, int nH, byte byColorLightType = 1);

#pragma region _Foreign_Func_

	public:
		int GetColorCIEView_Foreign(InspRoiImgBuf sFov, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI);
		int Color_GetColorHistogramView_Foreign(int nBin, int projectionmode, RECT rcROI, UCHAR* pucDstImg);
		double CompareCIE_ForeignBTM(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, int width, int length, POINTF pts[10][POLYGON_BTM_POINT_CNTS], int ptPolyCnt, int ptPolyNum, UCHAR* MaskImg, ColorXYInfoForeign* vColorXYInfoForeign, bool bGetExcept = false);
		int GetFullColorCIEView_Foreign(IN UCHAR* retFullImg, IN int nFullW, IN int nFullH, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI);

#pragma endregion _Foreign_Func_

#pragma region _BUBBLE_
		BOOL InspColorBubble(InspForeignInfo pForeignData, const InspPartInfo *pInspBoardInfo, InspFovForeignResult* ForeignResult, MPTI_InspectionMode vInspectionMod, ForeignData sData_ROI);
		void SaveBubbleDebugData();		//NYJ 2019/07/09
		BOOL CheckForeignBigArea(float fSizeX, float fSizeY, float fStandardSize);
		CString MakeFIlePath_BubbleResult(CString FolderName, CString FileName);		//NYJ 2019/07/15

#pragma endregion _BUBBLE_

	//tabInsp
	int CalTabArea(cv::Mat InputImg, double* A1, double* A2, double* A3, double* A2RecrArea, int* nSt, int* nEd, bool bIsTeach = false);

	BOOL ShiftTabPolygon(UCHAR* ucCIEMap, POINT* ArrPt, int nPtCnt);
	BOOL GetColorBaseBinTab(AlgoColorBase *pInspAlgoColor, InspRoiImgBuf &sInspRoiImg, UCHAR* retDstImg, CRect rt = CRect(0,0,0,0), bool bTeach = false, int nInspAC = 0, UCHAR* retDstImgAC = NULL, int nIDX = -1, int nSideCameraIndex = -1, TotalInspExceptArea *stTieArea = NULL, bool bSaveR = false);

	int ImageToColorCIE_Color_Rect(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, float factorR, float factorG, float factorB, UCHAR* dst, RECT rcRoi, int widthStep);
	int ImageToColorCIE_Color_Rect_CompoBtm(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
		float compoBtmR, float compoBtmG, float compoBtmB, UCHAR* dst, RECT rcRoi, int width, int length);
	BOOL CalcIntensityMask(UCHAR* img_R, UCHAR* img_G, UCHAR* img_B, UCHAR* img_BR, UCHAR* img_BB, int width, int length, UCHAR* ucDst, int nRange, int nGrayMin, int nGrayMax, BOOL bUseSide = false);

};
