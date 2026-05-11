#pragma once
#include "pinsp.h"
#include "Proc3D.h"
#include "ProcMil_Color.h"


#ifdef _DEBUG
#define SAVE_TAB_IMG 1
#endif

#define     HISTO_SIZE      256
#define		COLORXY_WIDTH	256
#define		COLORXY_LENGTH	256

#define    DEF_INSP		0
#define    DEF_TEACH	1

#define SIZE_CLRX	(1000)	// (CIE)	// 256 (RB)
#define SIZE_CLRY	(1000)

#define  CIE_OFFSETX		235
#define  CIE_OFFSETY		205


// CIE model - RGB to XYZ	// YJS 2016/11/04 : Çì´õ·Î ¿Å±è
#define CIE_XR	(0.490f)
#define CIE_XG	(0.310f)
#define CIE_XB	(0.200f)
#define CIE_YR	(0.177f)
#define CIE_YG	(0.813f)
#define CIE_YB	(0.011f)
#define CIE_ZR	(0.000f)
#define CIE_ZG	(0.010f)
#define CIE_ZB	(0.990f)

#define FOREIGN2D_RSTCNT 4
class CPInsp_Tab : public CPInsp
{
public:
	CPInsp_Tab(void);
	virtual ~CPInsp_Tab(void);

protected:

	Coordinate m_inspCoordinate;
	Coordinate m_teachCoordinate;

	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;

	BOOL m_bIsBigPart;	// Teach
protected:

	RstInspTab m_resultTab;

	//3D inspection Class
	CProc3D m_proc3d;

	CProcMil_Color* m_procMil;
	const InspAlgo *m_pInspAlgo;
	AlgoTab *m_pAlgoTab;
	WndAlgoImg m_wndAlgoImg;
	InspAlgoType m_eAlgoType;
	TotalInspExceptArea m_stTieArea;

	InspRoiImgBuf m_sInspImgBuf;
	int m_nTabIndex;
	int medBl;
	CPInsp_Algo* m_pCPInsp_Algo;
protected:
	int GetWidthImg(Coordinate coordinate) { return coordinate.width; }
	int GetHeightImg(Coordinate coordinate) { return coordinate.length; }

public:
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	int CloseDevice();
	int SetInspParam(const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo, int nLeadTipPos, RECT* rcLeadInsp = NULL, int nInspCameratype = eMSCN_COAXIAL, InspRoiImgBuf *sInspImageData = NULL, TotalInspExceptArea *stTieArea = NULL, int nTabIndex = 0 );
	int SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, const WndInfo &wndInfoAnyAngle, TotalInspExceptArea stTieArea, int nLeadTipPos=-1, bool bIsBigPart = false, bool bIsLoadFovRawDataImage = false, int projectionmode = 0 , int nInspCameraType = -1, int nFovID = -1);

	void InitResult(RstInspTab* ret);
	int GetInspRst(tagRstAlgoTab * pRstAlgoTab/*, UCHAR* ucArrDstImg*/);
	//void GetInspRst(InspAlgoResult &inspAlgoResult, RECT rcLeadInsp);
	//int GetInspRst(RstInspTab * pRstAlgoTab);
	
	int InspProc(UCHAR * ucArrDstImg, bool bTeach, bool bUseTabAreaOther);
public:
	//tabInsp
	int CalTabArea(cv::Mat InputImg, double* A1, double* A2, double* A3, double* A2RecrArea, int* nSt, int* nEd, bool bIsTeach = false); 
	int CalTabArea(cv::Mat InputImg, double* A1, double* A2RecrArea, RECT* rcTabRect);
	int CalTabArea2(cv::Mat InputImg, double* A1, double* A2RecrArea, RECT* rcTabRect);
	BOOL InspLine(cv::Mat mTabArea, float fErr, int nSt, int nEd, float* fMxErr, float fFactormm);
	BOOL InspLine(cv::Mat mTabArea, float fErr, RECT rcTabRect, float* fMxErr, std::vector<RECT>& rcErrRect, float fFactormm2, int nMaxLength);
	BOOL InspLine_center(cv::Mat mTabArea, float fErr, RECT rcTabRect, float* fMxErr, std::vector<RECT>& rcErrRect, float fFactormm2);
	BOOL InspWidth(int nSt, int nEd, float fStdWidth, float fFactormm);
	BOOL InspArea(cv::Mat TabImage, cv::Mat InspImg, float fErr, float* dDiff, float fFactormm2);
	BOOL InspArea(RECT rcTabRect, cv::Mat InspImg, float fErr, float* dDiff, std::vector<RECT>& rcErrRect, float fFactormm2, RECT& rcInspRect, BOOL bUseArea);
	BOOL InspArea(RECT rcTabRect, cv::Mat InspImg, cv::Mat GrayImg, float fErr, float* dDiff, float fFactormm2);
	BOOL InspArea_circle(RECT rcTabRect, cv::Mat InspImg, float fErr, float* dDiff, float fFactormm2);
	BOOL InspTail(bool bTailTab, cv::Mat InspImg, RECT rcTabRect, float fStdArea, double* dDiff, std::vector<RECT>& rcErrRect, float fFactormm2);
	bool PCA_1D(std::vector< cv::Point> pint, float* a, float* b);
	bool PCA_1D(std::vector< cv::Point2f> pint, float* a, float* b);
	void CircleFitting(std::vector<cv::Point2f> vPoList, double *centX, double *centY, double *radius, double *err);
	void CircleFitting(std::vector<cv::Point> vPoList, double *centX, double *centY, double *radius, double *err);
	BOOL CircleFitting2(cv::Mat BinImage, int nCntBlob, float *fResult, double *dMaxWidth, std::vector<double> m_fArrRstL);
	BOOL CircleFitting_KJS(cv::Mat BinImage, int nCntBlob, float *fResult, double *dMaxWidth, std::vector<double> m_fArrRstL);
	cv::Mat ImageThining(cv::Mat Img);
	BOOL verification_TabArea(cv::Rect InspRect, RECT rcRect, cv::Mat GrayImg, bool bHor);
	BOOL verification_TabArea_bin(cv::Rect InspRect, RECT rcRect, cv::Mat BinImage, bool bHor);

	//tab histogram BW function
	bool CalcHistoBW(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstInspTab* sRstAlgo, UCHAR* ucDstImg, UCHAR* ucDsAreatImg, int m_nTabIdx = 0, bool bTeach = false, bool bUseTabAreaOther = false);
	int Histogram(UCHAR* ucSrc, UCHAR* ucMask, int nWidth, int nHeight, CRect rtRoi = (0, 0, 0, 0));
};
