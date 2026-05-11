#pragma once
#include "PInsp_Wedge.h"
#include "PInsp_Ribbon.h"
#include "PInsp_Ball.h"
#include "PInsp_Crescent.h"
#include "Proc_Wire.h"

#if _DEBUG
#define FootTimecheck 1
#else
#define FootTimecheck 0
#endif

// Wire Window 정보
// 1. 보정하기 위해 필요
class CWire_Model
{
public:
	CWire_Model();
	~CWire_Model();

	InspRoiImgBuf* m_ImageBuffer;  //channel별 이미지
	float* pf3DOrg; // Window

	int m_nWindowStartX; //Raw 기준 Window Start
	int m_nWindowStartY;

	int m_Width;
	int m_Height;

	//검사 ROI
	RECT mImageRect;

	double m_dFootHeight[WIRE_DRAW_CNT];
	


};
typedef struct tagTailHgtDiffList		//NYJ 2020-03-11
{
public:
	POINT vTailLinePoint_1;
	int vTailLine_DiffValue_1;

	bool operator<(tagTailHgtDiffList b) const {
		return vTailLine_DiffValue_1 > b.vTailLine_DiffValue_1;
	}
}TailHgtDiffList;

class CPInsp_WireBonding :
	public CPInsp
{
public:
	CPInsp_WireBonding();
	~CPInsp_WireBonding();

	bool m_bUseImagePilLib;

	float m_fDiffHigh = 40;//15;//40;
	float m_fDiffLow = 40;// 15;//40;
	float m_fDiameterWire = 120.0f;
private:
	CProc_Wire* m_procWire;

	std::vector<CPInsp_Foot*> m_vPInspFoot;

	CPInsp_Wedge *m_pInspWedge;
	CPInsp_Ribbon *m_pInspRibbon;
	CPInsp_Ball *m_pInspBall;
	CPInsp_Crescent* m_pInspCrescent;


	std::shared_ptr<ImgProcessing> Img_Proc;		//NYJ 2019/11/21

private:
	int m_nFootType;

	bool m_bPadAutoTeach;

	//UI Teaching 용 변수
	int  m_nPointNum;
	POINTF* m_pWirePoint;
	float* m_WirepH2;
	std::vector<tuple<long long, long long, int>> m_vTupleWirePoint; // Wire 양옆 Point 및 Index 값이 들어있는 구조 
public:
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);

	void SetWireOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax, int nFootPatternMatchStopScore);
	int GetFootSobelMin();
	int GetFootSobelMax();
	//Foot
	void SetFootType(int nFootType);
	bool GetFootModelParam(CString sFileName2D, int* FootType, RECT* MRect, double* resX, double* resY);
	cv::Mat Correct3DRemoveLow(cv::Mat Image3D);
	float GetCorrect3d_Binary(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrAlgoColorOpt, POINT poXY, int nDBC = 0);
	bool PCA_1D(int cnt, cv::Point* pint, float* a, float* b);
	//void PCA_1D(int cnt, cv::Point * pint, cv::Point Center, float * a, float * b);

	CFoot_Model* MakeFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D = true, bool bUsePadAreaAutoTeach = true);
	CFoot_Model* MakeFoot(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D = true, bool bUsePadAreaAutoTeach = true);
	bool TeachFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, UCHAR *ucArrDstImg);
	bool TeachFootColor(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, FootColor fcFindDBC, UCHAR *ucArrDstImg, cv::Mat *cv3DAvgFilter);
	
	BOOL InspFoot(CFoot_Model* pFoot, WndAlgoImg &sWndAlgoImg, RstAlgoFoot * sRstAlgo, TotalInspExceptArea stTieArea, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, int nSelectFoot = -1, UCHAR *ucArrDstImg = NULL, AlignResult * arrAlignRes = NULL, bool bWingRstVisible = false, CString csDebugFilePath = NULL);

	//Foot Pattern
	bool LoadFootModelList(CString FolderPath);
	bool CheckModelList(CString FolderPath);

	bool CreateModel(cv::Mat Image2D, CString FilePath, int FootType, RECT MRect, int FootDirection, int DBC);
	bool CreateModel3D(cv::Mat Image3D, CString FilePath, int FootType, RECT MRect, int FootDirection, int DBC);
	bool MatchModel(cv::Mat Image2D, CString FilePath, int FootType);
	bool MatchModel3D(cv::Mat Image3D, CString FilePath, int FootType);
	bool MatchModel(cv::Mat Image2D, int FootType);
	bool MatchModel3D(cv::Mat Image3D, int FootType);
	bool MatchModel(CFoot_Model* pFoot, cv::Mat Image2D, cv::Mat cvPadImg, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, cv::Mat ImgFoot2DBW, cv::Mat ImgFoot3D, int* MatchModelNum, double dAngle = -INFINITY);
	bool MatchModel3D(CFoot_Model* pFoot, cv::Mat Image3D, cv::Mat cvPadImg, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, cv::Mat ImgFoot2DBW, int* MatchModelNum, double dAngle = -INFINITY);
	bool MatchModel(cv::Mat Image2D, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, int nFootDirection, int footType);
	bool MatchModel3D(cv::Mat Image3D, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, int nFootDirection, int footType);


	  //Wire
	CWire_Model* MakeWire(AlgoWire* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt);

	bool SearchWire(CSearchWire_Model* pModel, WireSearch& rst, int nSearchType = 0);

	int TeachWire(AlgoWire pAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoWire *sRstAlgo, float fTeachH);

	BOOL InspWire(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoWire * sRstAlgo, TotalInspExceptArea stTieArea, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, POINTF* poArrInspFoot = NULL, int nSelectFoot = -1, float fTeachH = 0.0f, UCHAR *ucArrDstImg = NULL, AlignResult * arrAlignRes = NULL, float fWireRefAreaHgt=0.0f , float fWireRefAreaHgt_Sub = 0.0f);

	BOOL InspWireShort(int nWireNum, WireRst* Wirerst, WndAlgoImg &sWndAlgoImg, UCHAR *ucArrDstImg, std::vector<std::vector<cv::Rect>> vFootRects);

	void RemoveThin(UCHAR *ucImage, int nImgWidth, int nImgHeight, bool bHorizon, int nRemove);

	bool GetCenterPoints(std::vector<long long>& m_vWireCenterPts);
	bool ClearWirePoints();

	//Foot Shift 영역 계산 제거
	int FindWire(AlgoWire sAlgo, WndAlgoImg &sWndAlgoImg, UCHAR *ucALLImage, RstAlgoWire *sRstAlgo, int retry, bool *bHorizon, double *dA_T, double *dB_T, UCHAR *ucImgDst, POINTF* poArrInspFoot, CWire_Model* pModel);
	//int FindWire(AlgoWire sAlgo, WndAlgoImg &sWndAlgoImg, UCHAR *ucALLImage, RstAlgoWire *sRstAlgo, int nSelectFoot, int nWire, bool *bHorizon, double *dA_T, double *dB_T, UCHAR *ucImgDst, POINTF* poArrInspFoot);
	BOOL CheckWire(AlgoWire sAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucImgDst, RstAlgoWire *sRstAlgo, bool bHorizon, double dA, double dB, bool bCheck);

	BOOL CheckWire2(UCHAR * ucImgDst, WndAlgoImg &sWndAlgoImg, int * nArrFoot, POINT * poArrFoot);

	byte InspectionWire(AlgoWire algoWire, float *pfImgSrc, int nWire, UCHAR* pucImage, int nImgWidth, int nImgHeight, bool bHorizon, double dA, double dB, RstAlgoWire *sRstAlgo);
	bool HoleRemoveCheck(CWire_Model * pModel, int nIndex ,bool bOptionUse = true );

	double InspectionWireTh(UCHAR * pucImage, int nImgWidth, int nImgHeight, bool bHorizon, double dA, double dB, double * dMaxTh);
	double InspectionDeleteWireTh(UCHAR * pucImage, int nImgWidth, int nImgHeight, bool bHorizon, double dA, double dB, double * dMaxTh , std::vector<cv::Point>& ThCenterPt);
	//double InspectionWireTh(UCHAR* pucImage, int nImgWidth, int nImgHeight, bool bHorizon, double dA, double dB, double *dMaxTh,double dThMin = 0 );

	BOOL BinarizeByFloodFill(AlgoWire algoWire, WndAlgoImg &sWndAlgoImg, UCHAR *ucImage, RstAlgoWire *sRstAlgo, int nWire, bool bHorizon, POINT *poArrFoot);
	void BinarizeByFloodFill(cv::Mat src, cv::Mat & dst, cv::Rect st, cv::Rect ed, float nDiffHigh, float nDiffLow, float nDiameterWire , CWire_Model* pModel, bool bRectUse = false);

	BOOL FindPoint(UCHAR* ucImgDst, int nImgWidth, int nImgHeight, RstAlgoWire *sRstAlgo, bool bHorizon, double *dA, double *dB, int* nArrFoot, POINT* poArrFoot);

	void BinarizeAddData(float *pfImgSrc, UCHAR *ucImgSrc, UCHAR *ucImage, int nImgWidth, int nImgHeight, bool bHorizon);
	void BinarizeDelData(UCHAR *ucImage, int nImgWidth, int nImgHeight, POINT* poArrFoot, bool bHorizon, double dA, double dB, int nRemove);

	cv::Point vect2d(cv::Point p1, cv::Point p2);

	double InspInterval(AlgoWire algoWire, UCHAR *ucImage, int nImgWidth, int nImgHeight, RstAlgoWire *sRstAlgo);
	double InspInterval(int nWireNum, WireRst* Wirerst,  UCHAR *ucImage, int nImgWidth, int nImgHeight);

	double IntervalSort(WireRst * Wirerst, std::vector<std::pair<int, int>>& vpairIndex);


	double InspInterval(int nWireNum, WireRst * Wirerst, UCHAR * ucImage, int nImgWidth, int nImgHeight, std::vector<std::vector<cv::Rect>> vFootRects);

	bool pointInRectangle(cv::Rect cvRectValue, cv::Point m);

	//bool pointInRectangle(cv::Point A, cv::Point B, cv::Point C, cv::Point D, cv::Point m);
	

	int GetWireBinPoint(int nNumPoints, POINTF* pts, float* ph2);


	//Search Wire
	bool FindFootROIList(CSearchWire_Model* pModel, int FootIndex, std::vector<FootRect>& rst, int nSearchType = 0 );
	bool FindFootROI(CSearchWire_Model* pModel, CFoot_Model* pfoot, RECT ROI, std::vector<FootRect>& rst, int nSearchType = 0,int FootIndex = 0);

	//BW 검사용 CFoot 생성(외부에서 삭제 하여야함)
	CFoot_Model* MakeSearchFoot(CSearchWire_Model* pModel, CFoot_Model* baseModel, RECT ROI);


	//Wire 폭 복원
	bool InterpolateWire3D(AlgoWire algoWire, WndAlgoImg &sWndAlgoImg, UCHAR *ucImage, bool bHorizon, POINT *poArrFoot, CWire_Model* pModel);
	bool CurveFitting(UCHAR* ucImage, int nWidth, int nHeight, bool bHorizon, int nW, int nH, int nStartX, int nEndX, int nStartY, int nEndY, int nThMin, std::vector<double>& coeff);
	bool WireFitting(int* pWarpMap, int nWarp2DWidth, int nWidth, int nHeight, bool bHorizon, int nStartX, int nEndX, int nW, int nH, int nDistanceHalf, std::vector<double>& coeff);
	bool WireWarpImage(UCHAR* ucImage, UCHAR* ucWarp, int* pWarpMap, int nWarp2DWidth, int nHeight);
	bool WireWarp3D(float* ucImage, float* ucWarp, int* pWarpMap, int nWarp2DWidth, int nHeight);
	void FindWireEdge(UCHAR* Edges, float* pFHeight, UCHAR* ucTeached, UCHAR* ucWarpSearched, CWire_Model* pModel, int* pWarpMap, int nWarp2DWidth, int nHeight, int nCenterX, int nth);
	void FindWireEdgeV2(UCHAR* Edges, float* pFHeight, UCHAR* ucTeached, UCHAR* ucWarpSearched, CWire_Model* pModel, int* pWarpMap, int nWarp2DWidth, int nHeight, int nCenterX, int nth);

	bool CurveFitting2(UCHAR* ucImage, int nWidth, int nHeight, bool bHorizon, POINT *poArrFoot, int nThMin, int nTh, int nthMax, std::vector<double>& coeff, float* pfImgSrc);
	double InspectThByXY(UCHAR* pucImage, int nImgWidth, int nImgHeight, int nX, bool bHorizon, double dA, double dB, RECT* rect, std::vector<POINTF>& vecCenter, std::vector<POINTF>& vecEdges);


	//debug
	double returnTime();
	cv::Point WarpAffine_IPPRotate(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent = false, bool Rev = false);

};

