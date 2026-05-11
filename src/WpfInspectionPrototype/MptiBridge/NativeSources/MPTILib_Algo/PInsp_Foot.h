#pragma once
#include "PInsp.h"
#include "ProcPil_Pattern.h"
#include "ModelFile_Foot.h"
#include "LeastSquare.h"

#define FOOT_RST_WRITE 0
typedef struct tagFootRect
{
public:
	RECT  m_PadRect;
	RECT  m_FootRect;
	POINT  m_Point;

	bool operator<(tagFootRect b) const {
		return m_Point.x < b.m_Point.x;
	}

}FootRect; //찾은 풋 위치
typedef struct tagFootPoins
{
public:
	POINT  m_pCenter;
	POINT  m_pFootEnd;
	POINT  m_pWedgeSt;
	double m_dSeta;
	RECT rcArea;

	bool bMatchSuccess;
	cv::Mat cvMatchingMask;
	tagFootPoins()
	{
		m_pCenter.x = 0;
		m_pCenter.y = 0;

		m_pFootEnd.x = 0;
		m_pFootEnd.y = 0;

		m_pWedgeSt.x = 0;
		m_pWedgeSt.y = 0;

		m_dSeta = 0.0f;

		bMatchSuccess = false;
	}
}FootPoins; //좌표

class CFoot_Model
{
public:
	CFoot_Model();
	~CFoot_Model();

	//Clip 검사 Raw Image 
	InspRoiImgBuf* m_ImageBuffer;
	float* pf3D_OrgROI; // Original Foot ROI
	float* pf3D; // ROI
	float* pf3DOrg; // Window

	int m_nFoottype;

	AlgoBin m_sArrBin[m_eFootBin_Total]; //Bin
	POINTF  m_FootCenterPoint; // FootCenter WindowLT 기준
	POINTF  m_FootCenterPoint_ImageRect; // FootCenter Image Rect 기준

	float m_fArrOptionValue[m_eFoot_Total][eMMD_Total];//검사 옵션

	//검사 ROI
	RECT mImageRect;

	//Window 영역
	RECT mWindowImageRect;
	double dAngle;
	int m_nWindowStartX; //Raw 기준 Window Start
	int m_nWindowStartY;

	RECT m_RefRect;  // 참조 바닥 영역
	RECT m_PadRect;	//NYJ 2020/02/05
	RECT m_BondingRect;	//shkim 2020/04/17
	cv::RotatedRect* m_RoRect; // shkim 2020/04/28

	//Fov/Part 영역
	int _WidthOrg;
	int _HeightOrg;

	//검사 옵션
	int m_nUseOption;
	int m_nUseOption2;
	int m_nFindOption;
	int m_nFindOption2;
	std::vector<int> m_nFindOptionOrder;
	std::vector<int> m_nFindOptionOrder2;
	bool m_bUsePatternAngle;
	int m_nFootAngle;
	double m_dTeachFootAngle;
	bool m_bUse2Foot;
	bool m_bUsePadAreaAutoTeach;
	bool m_rst_Color_Image;

	FootColor m_fcBackground;
	FootColor m_fcFoot;
	FootColor m_fcWing;
	FootColor m_fcWedge;
	FootColor m_fcFindDBC;

	UCHAR* m_pPad;
	float m_fPadArea3DAvgHeight;
	UCHAR* m_pDBCPolygonImg;	//2023.03.14 DBC 폴리곤 티칭된 영상 저장

	bool bUseTeachRect;
	int nPadWidth;
	int nPadHeight;
	int nRadius;

	bool bTeachBin;

	bool bUseRemovedPadArea;
	bool bUse2DImageForInspWidth;

	int nRemoveWireHeight;

	int nFootSobmin;
	int nFootSobmax;

	int nFootPadSizeX;
	int nFootPadSizeY;

	int nPadEdgeMinRatio;
	bool bUseDBCShapeTeaching;

	int nInspCrackHeight;
public:
	void GetImageMatrix(cv::Mat* Image, int footKind);
	void GetImageMatrix(cv::Mat* Image, int footKind, cv::Rect ImageRect);

	void GetPadBinImage(cv::Mat* Image);
	void GetDBCPolygonBinImage(cv::Mat* Image);

	AlgoBlob GetAlgoBlob(int footKind);

	int GetImageWidth() { return m_ImageBuffer->nImageSizeX; }
	int GetImageLength() { return m_ImageBuffer->nImageSizeY; }
	int GetImageSize() { return m_ImageBuffer->nImageSizeX * m_ImageBuffer->nImageSizeY; }

	int GetWindowImageWidth() { return mWindowImageRect.right - mWindowImageRect.left; }
	int GetWindowImageLength() { return mWindowImageRect.bottom - mWindowImageRect.top; }
	int GetWindowImageSize() { return GetWindowImageWidth() * GetWindowImageLength(); }


	int GetImageStartXInWindowLT() { return mImageRect.left - mWindowImageRect.left; }
	int GetImageStartYInWindowLT() { return mImageRect.top - mWindowImageRect.top; }

};

//Wire Search를 위한 데이터 구조
class CSearchWire_Model
{
public:
	CSearchWire_Model();
	~CSearchWire_Model();

	CFoot_Model* m_pFoot1;  // Foot1 관련 정보
	std::vector<RECT> m_ROI_Foot1;  // Foot1이 대상인 ROI

	CFoot_Model* m_pFoot2;  // Foot2 관련 정보
	std::vector<RECT> m_ROI_Foot2;  // Foot2가 대상인 ROI

	int m_nWidth;  // Part Width
	int m_nHeight;  // Part Height

	InspRoiImgBuf* m_pPartImageBuffer;  // Part 관련 Raw 이미지
	float* m_pPart3D;  // Part 관련 3D 이미지

	void GetImageMatrix(cv::Mat* Image, CFoot_Model* m_pFoot, int footKind);
	void GetImageMatrix(cv::Mat* Image, CFoot_Model* m_pFoot, int footKind, cv::Rect ImageRect);


};


class CPInsp_Foot :
	public CPInsp
{
public:
	InspParamPattern* m_pInspParam;			//auto inspection parameter for pattern   NYJ 2019/11/20
	TeachParamPattern* m_pTeachParam;		//teach parameter for pattern

	InspParamPattern m_paramInspect;
	TeachParamPattern m_paramTeach;

	CProcMil_Pattern* m_procPatternMil;		//NYJ 2019/11/20
	RstInspPattern m_rstInspPattern;		//NYJ 2019/11/20

	int m_nFootPadTopWGrayLevel = 220;
	int m_nFootPatternMatchScore = 50;
	int m_nFootPatternMatchStopScore = 70;

	int m_nFootSobelmin = 30;
	int m_nFootSobelmax = 150;

	std::shared_ptr<ImgProcessing> Img_Proc;		//NYJ 2019/11/21
	std::shared_ptr<GeoModel_Foot> m_PatModel;
	AlgoPattern *m_pAlgoPattern;	//NYJ 2019/11/21
	GeoMatch_Foot gm;

	std::shared_ptr<CMModelManager<ModelFile_Foot>> m_ModelMng2D;
	std::shared_ptr<CMModelManager<ModelFile_Foot>> m_ModelMng3D;

	double m_WingLenthMinimumMM = 0.0;		//NYJ 2020/02/27

	bool m_bMatch2D = false;
	bool m_bMatch3D = true;

	CString cstDebugFolderPath = _T("");

#ifdef ALGORITHMTOOL_CPP_USE
	int nFootAlgoCnt = 0;
#endif

public:
#pragma region init

	CPInsp_Foot();
	~CPInsp_Foot();

	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);

#pragma endregion

#pragma region ImageProcess


	//image
	static void GetImageMatrix(cv::Mat* Image, AlgoBin bin, InspRoiImgBuf* m_ImageBuffer, cv::Rect ImageRect);
	int GetBinImage(CFoot_Model* pFoot, cv::Mat* Image, int footKind, cv::Mat* Mmask = NULL, bool bMaxblob = true, int nMinblobArea = 4, bool bUseFootOnly2D = false);
	int GetBinImage_Pad(CFoot_Model* pFoot, cv::Mat* Image, int footKind, cv::Mat* Mmask = NULL, bool bMaxblob = true, int nMinblobArea = 4, bool bUseFootOnly2D = false, bool bUseOrg3DROI = true);
	int GetBinImage(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, cv::Mat* Image, cv::Rect ImageRect, int footKind, int left, int right, cv::Mat* Mmask = NULL, bool bMaxblob = true, cv::Mat* ReHImg = NULL, cv::Mat* cv3DOrg = NULL, int nMinblobArea = 4, bool bUseFootOnly2D = false);
	cv::Point WarpAffine(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent = false, bool Rev = false);
	cv::Point WarpAffine_IPPRotate(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent = false, bool Rev = false);
	cv::Point WarpAffine_IPPRotate_3D(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent = false, bool Rev = false);


	int Update_fromCV(WndAlgoImg * sWndAlgoImg, UCHAR* src, UCHAR* dsc);
	void Correct3d(int OptionKind, float * height, int wid, int len, BYTE * TopR, BYTE * TopB, BYTE * TopW, bool bGrouping = false);
	void Correct3dFoot(CRect roi, float * height, float * Xgrad, float * Ygrad, BYTE * TopR, BYTE * TopB, BYTE * TopW, int wid, int len, int HistHeightLowLimit);
	void Correct3dDBC(CRect roi, float * height, float * Xgrad, float * Ygrad, BYTE * TopR, BYTE * TopB, BYTE * TopW, int wid, int len, int HistHeightLowLimit);
	void Correct3d_Binary(cv::Mat Mask, int OptionKind, float * height, int wid, int len, float &fPadAvgHeight);
	void Correct3d_Binary_DBC(CFoot_Model* pFoot, cv::Mat Mask, int OptionKind, float * height, int wid, int len, float &fPadAvgHeight);
	void Correct3dFoot_Binary(cv::Mat Mask, float * height, float * Xgrad, float * Ygrad, int wid, int len, int HistHeightLowLimit);
	void Correct3dDBC_Binary(cv::Mat Mask, float * height, float * Xgrad, float * Ygrad, int wid, int len, int HistHeightLowLimit);
	float GetCorrect3d(WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrAlgoColorOpt, POINT poXY, int nDBC = 0);
	float GetCorrect3d_Binary(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrAlgoColorOpt, POINT poXY, int nDBC = 0);

	//make Images
	CFoot_Model* MakeFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D, bool bPadAreaAutoTeach = true);
	CFoot_Model* MakeFoot(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, cv::Mat *cv3DAvgFilter, cv::Mat *cvDBCPolygonImg, bool bCorrect3D, bool bPadAreaAutoTeach = true);
	CFoot_Model* MakeRotateFoot(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, bool bCorrect3D = true);

	//int GetBlob_BondingFloor(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, int nBlobCnt, long *pLebel, USHORT* LabelImage, cv::Mat * cvAllFoot, cv::Mat * cvAllWedgeImg, cv::Mat * cvAllWing, cv::Mat * cvAll3DAreaImg, std::vector<cv::Rect>& rcFootROI, int nWidth, int nHeight);
	int GetBlob_BondingFloor(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, int nBlobCnt, int InspFootCnt, long *pLebel, USHORT* LabelImage, cv::Mat * cvAllFoot, cv::Mat * cvAllWedgeImg, cv::Mat * cvAllWing, cv::Mat * cvAll3DAreaImg, std::vector<int>& nMatLabel, std::vector<int>& nMatLeft, std::vector<int>& nMatRight, std::vector<int>& nMatTop, std::vector<int>& nMatBottom, std::vector<int>& LeftIdx, std::vector<cv::Rect>& rcFootROI, int nWidth, int nHeight);

	int RemoveThin(CFoot_Model *pFoot, int nCntBlob, long *pLebel, USHORT* LabelImage, cv::Mat * cvAllFoot, cv::Mat * cvAllWedgeImg, cv::Mat * cvAllWing, cv::Mat * cvAll3DAreaImg);
	bool Calc_padBinImage2(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Rect ImageRect, cv::Mat* RepadImg, cv::Mat Img3D);
	bool Calc_padBinImage3(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Rect ImageRect, cv::Mat* RepadImg, cv::Mat Img3D, bool bTeachBin);
	
	bool Calc_padBinImage4(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Rect ImageRect, cv::Mat* RepadImg, cv::Mat Img3D, bool bTeachBin);
	bool Calc_PadBW_UseOnly3D(CFoot_Model *pFoot, cv::Mat* cvRstAutoInspPad, int nSrcWidth, int nSrcHeight);
	bool FindWing_UseOnly3D(CFoot_Model *pFoot, cv::Mat* cvRstAutoInspWing, int nSrcWidth, int nSrcHeight);
	bool FindFootAngle_UseBlobLine(CFoot_Model *pFoot, cv::Mat* cvRstFootLine, double *dRstThetaFoot, cv::Point *poLineSt, cv::Point *poLineEd, cv::Point *poFootCenter, cv::Mat cvSrcFootImage, int nSrcWidth, int nSrcHeight);
	bool Calc3DEdge_SSEImg(cv::Mat cvSrc3DImg, cv::Mat &cvDst3DImg);
	bool Find_PadRect_UsePadBWAnd3D(CFoot_Model *pFoot, cv::Mat* cvRstPadRectImg, cv::Mat cvSrcPadImage, int nSrcImgW, int nSrcImgH, int nPadTeachSizeX, int nPadTeachSizeY, float fRatioEdgeLineCnt, float fRatioBlackLineCnt, bool bMakeRoundRect, cv::Mat cvPadAndFootImage);

	int SetLabelImage(USHORT* LabelImage, int nLabel, cv::Mat * cvAllWedgeImg, cv::Mat * cvAllWing, cv::Mat * cvAll3DAreaImg, cv::Rect ImageRect, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cv3DAreaImg);
	//bool FindDBCOutLines(CFoot_Model* pFoot, cv::Mat *cvDstDBCLineImg, int n2DThresMin=0 );
	bool FindDBCOutLines(CFoot_Model* pFoot, cv::Mat *cvDstDBCLineImg, int n2DThresMin = 0, int nFootDir = 3);

	cv::Mat CalcOnlyPadArea(CFoot_Model* pFoot, int nWid, int nLen, bool bIsDBCArea);
	void SetBWForUIImage(CFoot_Model *pFoot, int nSX, int nSY, int nEX, int nEY, cv::Mat* cvFootBW, cv::Mat* cvWedgeBW, cv::Mat* cvWingBW, FootPoins* fPoints = NULL);	//UI Image

	bool CalcFoot3DAvgFilterImage(CFoot_Model* model, int nSrcWidth, int nSrcHeight, cv::Mat cvPadBWImage, cv::Mat *cvRst3DAvgFilter);
	bool LSQLineFit(std::vector<POINT>& pts, double &pdA, double &pdB, bool* bVertical = nullptr, bool* bStraight = nullptr);
	bool GetLinePoint(BOOL bStraight, BOOL bVertical, float a, float b, std::vector<POINT>& vecpt, POINTF& x1, POINTF& x2);
	bool Find_LineAngle_houghline(CFoot_Model *pFoot, cv::Mat* cvSrcLineImage, double &dRstLineTheta);
#pragma endregion


#pragma region Foot Pattern function

	//create foot model
	bool CreateModel(cv::Mat Image2D, CString FilePath, int FootType, RECT MRect, int FootDirection, int DBC);
	bool CreateModel3D(cv::Mat Image3D, CString FilePath, int FootType, RECT MRect, int FootDirection, int DBC);
	//Load foot model
	bool LoadFootModelList(CString FolderPath);
	bool CheckModelList(CString FolderPath);
	//Matching & test
	bool MatchModel(cv::Mat Image2D, CString FilePath, int FootType);
	bool MatchModel3D(cv::Mat Image3D, CString FilePath, int FootType);
	bool MatchModel(cv::Mat Image2D, int FootType);
	bool MatchModel3D(cv::Mat Image3D, int FootType);
	bool MatchModel(CFoot_Model *pFoot, cv::Mat Image2D, cv::Mat ImagePad, std::shared_ptr<GeoResult_Foot> m_PatResult,  cv::Rect* szModelImg, cv::Mat ImgFoot2DBW, cv::Mat ImgFoot3D, int* MatchModelNum, double dAngle = -INFINITY);

	bool MatchModel3D(CFoot_Model *pFoot, cv::Mat Image3D, cv::Mat ImagePad, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, cv::Mat ImgFoot2DBW, int* MatchModelNum, double dAngle = -INFINITY);
	bool MatchModel(cv::Mat Image2D, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, int nFootDirection, int footType);
	bool MatchModel3D(cv::Mat Image3D, std::shared_ptr<GeoResult_Foot> m_PatResult, cv::Rect* szModelImg, int nFootDirection, int footType);

	bool GetFootModelParam(CString sFileName2D, int* FootType, RECT* MRect, double* resX, double* resY);
#pragma endregion


#pragma region AutoSearchFunction

	void InitPatternResultStruct(RstInspPattern* rst);

	cv::Mat Correct3DRemoveLow(cv::Mat Image3D);
	int SearchPattern(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst);
	int SearchPattern2D(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst, int FootIndex = 0);
	int SearchPattern3D(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst, int FootIndex = 0);
	int SearchPattern2D3D(CFoot_Model *pFoot, CSearchWire_Model* pModel, RECT ROI, std::vector<FootRect>& rst, int FootIndex = 0);

	int Pattern_Teach(CFoot_Model *pFoot, UCHAR* retImg, int sizeX, int sizeY, int use_algo, int nLight_Number, bool refPart);
	int GetPart3DGradientImg(cv::Mat &xGradImgRst, cv::Mat &yGradImgRst, cv::Mat &SumXYGradImgRst, CSearchWire_Model* pModel);

	//BW 검사용
	bool FindFootROI_BW(CFoot_Model* pfoot, std::vector<FootRect>& rst);
	bool Insp_Foot_BW(CFoot_Model *pFoot, cv::Point& CenterPt, int* left, int* right, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, int nFootDirection);
	virtual bool getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection);
	
#pragma endregion


	//InspFunction
	virtual BOOL InspFoot(CFoot_Model* pFoot, WndAlgoImg &sWndAlgoImg, RstAlgoFoot * sRstAlgo, TotalInspExceptArea stTieArea, AlgoColorOpt &ptrAlgoColorOpt, int nSelectFoot, UCHAR *ucArrDstImg, AlignResult * arrAlignRes, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, CString sSaveDebugPath = _T(""));

	//search Rect 확장
	bool HasBlobRectLine(long *pLebel, USHORT* LabelImage, int nCntBlob, int nW, int nH, int nL, int nR, int nT, int nB);

	//Inspection
	//1. foot 존재 여부
	int FindFoot(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * ImgPatTarg, int nfootKind);
	virtual int FindFoot_Pat(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, FootPoins &fPoints, int nfootKind);
	virtual bool FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible);
	bool FindFoot_3DBlob(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, FootPoins &fPoints, int nfootKind, cv::Mat *cv3DAvgFilter, cv::Mat &RoRectimage, CString sSaveDebugPath = _T(""), bool bUseFootOnly2D=false, bool bRotateOrgImage = false, bool bFstChk =false);

	//2,3,4
	virtual cv::Point FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, cv::Mat *cvFindPadImg, cv::Mat *cvFindFootImg);
	virtual int FootPointByType(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, FootPoins &fPoints, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, std::vector<cv::Mat> &Images, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter);
	virtual cv::Point FootImageByPoint(CFoot_Model *pFoot, FootPoins fPoints, std::vector<cv::Mat> &Images);

	//4. 측정
	void CalcShiftPointLeft(cv::Point InspSt, cv::Point InspEd, cv::Point StdSt, cv::Point StdEd, int type, int* shX, int* shY); // shift
	bool Insp_FootShift(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<int> startPad, std::vector<int> endPad, std::vector<int> startPadY, std::vector<int> endPadY, std::vector<int> Left, std::vector<int> Top, std::vector<int> right, std::vector<int> Bot, std::vector<int> LeftIdx);		//위치 틀어짐 // foot bonding Location
	
	bool Insp_FootLift(cv::Mat* vImage, cv::Mat* fHImg, cv::Mat* PadImg, float* fHeightDiff);		//풋떨어짐
	bool Insp_FootLift_v2(cv::Mat* vImage, cv::Mat* fHImg, float BotAvgH, float* fHeightDiff);		//풋떨어짐

	bool Insp_FootTailLength(cv::Mat vImage, cv::Point pLeftSt, cv::Point pRightSt, cv::Point* pSt, cv::Point* pEd, float*fArea);
	bool Insp_FootHillCrack(cv::Mat* vImage, cv::Mat* fHImg, cv::Point pEd, cv::Point* re_Start, cv::Point* re_End);	//힐 크랙
	bool Insp_FootImprint2(cv::Mat* vImage, float fWidth, cv::Point pSt, cv::Point pEd, cv::Point* re_Start, cv::Point* re_End);	//풋찍힘
	bool Insp_FootWidth(cv::Mat* vImage, cv::Point pSt, cv::Point pEd, bool isLeft, cv::Point* Re_start, cv::Point* Re_end);		//풋 너비
	void Insp_Wedge_WidthLength(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat* Img, cv::Mat* Wing, cv::Mat Img3D, cv::Point& CenterPo, double& seta, cv::Mat &mMaskImage, cv::Mat &mWedgeMaxBlob, cv::Mat &PadRectImg, cv::Point pLSt, cv::Point pLEd, std::vector<cv::Point> &vPoSt, std::vector<cv::Point>& vPoEd, cv::Point* CtLineSt, cv::Point* CtLineEd, cv::Point pWingPosLeftX, cv::Point pWingPosRightX);
	bool Insp_FootWedge(cv::Mat* vImage, cv::Point pSt, cv::Point pEd, cv::Point* RE_LeftSt, cv::Point* RE_LeftEd, cv::Point* RE_RightSt, cv::Point* RE_RightEd);		//웨지 상단
	bool Insp_FootLength(cv::Mat* vImage, cv::Point pLSt, cv::Point pLEd, cv::Point* pRSt, cv::Point* pREd);		//풋 길이
	bool Insp_FootShift_removedWire(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFoot, cv::Point *pWingBondingEd); // 
	//bool Insp_FootShift_ImgAnd(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFoot, cv::Mat * cvPad, cv::Mat * cvBondingRect, cv::Point cvPoNewCenter);
	bool Insp_FootShift_ImgAnd(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFoot, cv::Mat * cvPad, cv::Mat * cvBondingRect, cv::Point cvPoNewCenter, float &shift, float &fshiftX, float &fshiftY);

	void InspWingArea(CFoot_Model *pFoot, cv::Mat * cvImgL, cv::Mat * cvImgR, RstAlgoFoot * sRstAlgo);
	bool RotateBackToOriginalPosition(CFoot_Model* pFoot, cv::Mat inputImg, cv::Point& CenterPo, cv::Point& Margin, double& seta, cv::Mat* ResultImg);


	//Result
	bool Confim_OneFootAlgo(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, cv::Point PoSt, cv::Point PoEd, int footType);
	bool Confim_OneFootAlgo_byValue(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, int footType, float fArea);
	int convertToFootData(int footType);
	void CalcRotPnt(cv::Point pCenter, double dAngle, cv::Point& Po);
	void CalcRotPnt(cv::Point pCenter, double dAngle, std::vector<cv::Point>& vPoSt, std::vector<cv::Point>& vPoEd, cv::Point Margin = cv::Point(0, 0));

	//Foot Align
	bool AllImageClip(AlgoFoot* sInspFoot, WndAlgoImg &sWndAlgoImg, AlgoColorOpt &ptrSrcColorOpt, cv::RotatedRect& SelectedRect);
	bool InspFootAlign(cv::Mat vImage, cv::Rect TeachRect, cv::RotatedRect& SelectedRect);
	bool GerAlignRect(cv::Mat vImage, std::vector< cv::RotatedRect>& vDstRect);

	//UI Image
	bool TeachFoot(CFoot_Model* pFoot, int nfootKind, UCHAR *ucArrDstImg);
	bool TeachFootColor(CFoot_Model* pFoot, int nfootKind, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, FootColor fcFindDBC, UCHAR *ucArrDstImg, int nStartX, int nStartY, cv::Mat *cv3DAvgFilter);
	bool TeachFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, UCHAR *ucArrDstImg);
	bool TeachFootColor(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nStartX, int nStartY, AlgoColorOpt &ptrAlgoColorOpt, int nfootKind, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, FootColor fcFindDBC, UCHAR *ucArrDstImg, cv::Mat *cv3DAvgFilter);

	void DefectRectSet(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo);
	//testcode
	double m_dSearchTime;
	double returnTime();
	void ImagePointSave(CString sPath, cv::Mat* vImage, std::vector<cv::Point> vPoSt, std::vector<cv::Point> vPoEd);
	void ImageWingPointSave_forDebug(CString sPath, cv::Mat* vImage, cv::Point vPoSt, cv::Point vPoEd, int nStAlgoNum, int nEndAlgoNum, bool bUseBothSideRst = false, int nStAlgoOpt = 0, int nEndAlgoOpt = 0);

	int GetPart3DGradientImg(cv::Mat &xGradImgRst, cv::Mat &yGradImgRst, cv::Mat &SumXYGradImgRst, CSearchWire_Model* pModel, cv::Mat &DstBlobGradient);

	//float GetHeightMaxHist(float *pfArrImgData, int nImgWidth, int nImgHeight, float fMaxHgt, float fMinHgt);
	float GetHeightMaxHist(float *pfArrImgData, int nImgWidth, int nImgHeight, float fMaxHgt, float fMinHgt, bool bIsDBCFoot);
	cv::Mat calcRoundRect(cv::Mat insp, int nRadius, cv::Rect& bMaxbox);
	cv::Mat calcRoundRect_2(cv::Mat insp, int nRadius, cv::Rect& bMaxbox);
	
	void calcLine(cv::Mat insp, cv::Point pSt, cv::Point pEd, float* a, float* b, float fX = 0, float fY = 0);
	cv::Mat DrawRoundPad(int nImgWidth, int nImgHeight, cv::RotatedRect RoRe, int nRadius = 16);
	cv::Mat MatPolygon(cv::Mat mImage, cv::Mat PolyImg);

protected:
	void PCA_1D(int cnt, cv::Point* pint, cv::Point Center, float* a, float* b);			//1차 Line
	void PCA_1D_OrgVal(int cnt, cv::Point* pint, cv::Point Center, float* a, float* b, float* c, float* d);
	void OLS_1D_2(int cnt, cv::Point* pint, float* a, float* b, float* c);
	void RANSAC_1D(std::vector<cv::Point> AvgC, std::vector<cv::Point>* RanC, float MaxDiff);
	void RANSAC_1D_All(std::vector<cv::Point> AvgC, std::vector<cv::Point>* RanC, float MaxDiff);
	cv::Mat Rot_FootAng(cv::Mat * vImage, double fResAng, bool bInterpolation = true);
	bool Insp_FootPos(cv::Mat* vImage, cv::Point* CenterPo, double* seta);	//foot rotate
	bool Calc_FootAng(cv::Mat* vImage, double & ang);
	void CalcDisplayPnt(double dAngle, std::vector<cv::Point> vPoSt, std::vector<cv::Point> vPoEd, std::vector<cv::Point>& vPoDispSt, std::vector<cv::Point>& vPoDispEd);
	void DeleteSilk(cv::Mat* Image);

};

