#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "InspParamDef.h"
#include "ProcMil.h"

#pragma once
enum m_ePAD_PARALLEL
{
	m_ePAD_PARALLEL_None = 0,
	m_ePAD_PARALLEL_Type1,
	m_ePAD_PARALLEL_Type2,
	m_ePAD_PARALLEL_Type3,
	m_ePAD_PARALLEL_Type4,
};

class PAD_PAT
{
private:
	double D_MIN_PER = 0.75;
	double D_MAX_PER = 0.99;
	double D_MOR_PER = 0.01;
	double D_OUT_PER = 0.98;
	double D_OUT_LINE_PER = 3;
	double D_OUT_LINE_PER2 = 3.5;
	double D_WH_PER = 2;
	int N_POLY_DIFF = 1;
	int N_BIN_DIFF = 2;
	int N_BIN_MIN = 5;
	int N_GAP_ORG = 5;
	int N_ONE_CHK = 20;
	int N_INSP_LINE_OUT = 25;
	int N_INSP_OUT = 30;
	int N_SMALL_L = 50;
	int N_SMALL_AB = 55;
	double m_resolX;
	double m_resolY;

public:
	PAD_PAT();
	virtual ~PAD_PAT();

	void InitAlgo(double dResX, double dResY);

#pragma region PARALLEL
	int PAD_PARALLEL(INSP_PAD_PAT stAlgoParam, INSP_PAD_PAT_RST* pPAT_RST, UCHAR* pucPOLY);

	void RoiImageCompose_LT(CProcMil *m_pProcMilAlgo, UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
		int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
		int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
		int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle = 0);
	void FullImageClaculCompose(CProcMil *m_pProcMilAlgo, const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage);
	int Binarize(cv::Mat imgSrc, PAD_2D_DATA* pfRst);
#pragma endregion

	bool INSP_SHIFT_AREA(AlgoVolume *pAlgo, WndAlgoImg &sWndAlgoImg, cv::Mat ImgPoly, UCHAR* pUcArrDstImg, int nW, int nH);
	BOOL PADPatternCheck(WndAlgoImg &sWndAlgoImg, AlgoVolume* pInspAlgoVolume, InspAlgoParam stAlgoParam, RstAlgoVolume* sRstAlgo, UCHAR* pUcArrDstImg, UCHAR* ucArrDstImg, int nImgWidth, int nImgHeight, double dRstShiftX, double dRstShiftY, double dRstShiftXPix, double dRstShiftYPix, int nTYPE_P = m_ePAD_PARALLEL_None, INSP_PAD_PAT_RST* pPAT_RST = NULL, UCHAR* pucPOLY = NULL);
	BOOL PAT_OverlapArea(AlgoVolume* pInspAlgoVolume, InspAlgoParam stAlgoParam, RstAlgoVolume* sRstAlgo, cv::Mat ImgPoly, cv::Mat ImgBin, UCHAR* ucArrDstImg, int nImgWidth, int nImgHeight, cv::Mat imgMix);
	cv::Rect FIND_MAX_ROI(std::vector<std::vector<cv::Point>> arrBin, int nIDX, cv::Mat imgMor);
	int FIND_OFFSET(cv::Mat ImgBin, cv::Mat ImgPoly, RECT_F roi, cv::Mat ImgPolyArea, int nPolyAreaMargin, RECT_F* pRST, std::vector<RECT_F> vecROI, double& dBinCNT, double dRST_MAX_AREA, double dRST_MAX_MOR);
	bool POLY_IMG_CHECK(cv::Mat ImgPoly);

#pragma region OLD
	BOOL PAT_Thickness_Ver2(WndAlgoImg &sWndAlgoImg, AlgoVolume* pInspAlgoVolume, cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat ImgBin, RstAlgoVolume* sRstAlgo, cv::Mat imgMix, UCHAR* pUcArrDstImg, bool bX, int nTYPE_P);
	void DRAW_Contour(std::vector<std::vector<cv::Point>> arr, int nIDX, cv::Mat img, cv::Mat imgSRC);
	int FIND_TH_BIN(VOL_TH_RE sRE, cv::Rect rectP, cv::Rect rectB, cv::Mat img, cv::Mat imgSrc, bool bX, double dAValue, double dBValue, RECT_F* pMAX, RECT_F* pMIN, RECT_F* pMAX_F, RECT_F* pMIN_F, float fPol_ALL = -1, bool bCheck = true);
	float FIND_TH_POLY(cv::Rect rectP, cv::Mat img, bool bX, double* dAValue, double* dBValue, VOL_TH_DATA sData);
	int FIND_TH(VOL_TH_RE sRE, cv::Rect roi, cv::Mat img, cv::Mat imgSrc, bool bX, RECT_F* pMAX, RECT_F* pMIN);
	float CHECK_LINE(cv::Mat ImgPoly, RECT_F rcROI, bool bX, double dA_VAL, double dB_VAL, float fPAT_Min, float fPAT_Max, bool bROI = false);
	void PAT_TH_AB(int nIDX, cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat imgPOLY_BIN, cv::Mat imgBIN_BIN, cv::Mat imgBIN_ORG, cv::Mat kernel,
		std::vector<std::vector<cv::Point>> arrPoly, std::vector<std::vector<cv::Point>> arrBin, float fPAT_Min, float fPAT_Max,
		VOL_TH_DATA sDataP, VOL_TH_RE sRE, int &nMIN, int &nMAX, RstAlgoVolume* sRstAlgo, bool bFIND_LINE = false, bool bFIND_LINE_X = false, double dFIND_LINE_A = 0, double dFIND_LINE_B = 0);
	double PAT_TH_LINE(cv::Mat imgSrc, std::vector<std::vector<cv::Point>> arrPoly, int nIDX, VOL_TH_DATA sData);
	int FIND_TH_WA(VOL_TH_RE sRE, cv::Mat img, bool bX, double dAValue, double dBValue, RECT_F pMAX_ORG, RECT_F pMIN_ORG, RECT_F* pMAX, RECT_F* pMIN, RECT_F* pMAX_F, RECT_F* pMIN_F);
	bool FIND_POS(cv::Mat imgPOLY_BIN, cv::Mat imgBIN_BIN, cv::Mat ImgPoly, cv::Mat imgBIN_ORG, bool bX, float fPAT_Min, float fPAT_Max, VOL_TH_DATA* sDataP, VOL_TH_DATA* sDataB);
	bool FIND_POS_ROT(cv::Mat img, cv::Mat imgSRC, bool bX, float fPAT_Min, float fPAT_Max, VOL_TH_DATA* sData);
	double GetGradient(double *ptrnLineX, double *ptrnLineY, double nTotalCnt, double &pdA, double &pdB, bool bHorizon);
	void Check_End_Bin(cv::Rect rect, cv::Mat Img);
	void POLY_ROI_LINE(cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat imgPOLY_BIN,
		cv::Mat imgBIN_BIN, cv::Mat imgBIN_ORG, cv::Mat kernel,
		float fPAT_Min, float fPAT_Max, VOL_TH_DATA sDataP, VOL_TH_RE sRE, int &nMIN, int &nMAX, RstAlgoVolume* sRstAlgo);
	bool FIND_LINE(cv::Mat ImgSrc, cv::Mat ImgRst, VOL_TH_DATA sDataP, double* dAValue, double* dBValue, bool bBIN, RECT_F* roiPOLY, bool* pbX, bool bONE, bool bEdge);
	bool CHECK_POLY_LINE(cv::Mat Img, bool bX, cv::Rect roi, float fPol);
	bool FIND_LINE_XY(cv::Mat Img, int* pnPOL);
	void END_LINE_REMOVE(cv::Mat ImgPoly, cv::Mat imgPOLY_BIN, cv::Mat imgBIN_BIN);
	void CHANGE_ONE_IMG(cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat ImgBIN, cv::Mat ImgBINORG, int nPOL, bool bX);
	void REMOVE_POLY_NOT_XY(cv::Mat ImgPoly, std::vector<std::vector<cv::Point>> arrPoly, int nIDX, bool bX, VOL_TH_DATA sDataP, int nPOLY_DIFF);
	int PAT_TH_L(VOL_TH_RE sRE, float fPol, bool bX, cv::Mat imgBIN_BIN, cv::Mat imgBin_Temp, cv::Rect roiB, RECT_F* roiMAX, RECT_F* roiMIN);
#pragma endregion

#pragma region TH
	BOOL PAT_Thickness(WndAlgoImg &sWndAlgoImg, AlgoVolume* pInspAlgoVolume, cv::Mat ImgPoly, cv::Mat ImgBin, RstAlgoVolume* sRstAlgo, bool bX, int nTYPE_P);
	bool CHK_BIN_POLY_XOR(cv::Mat imgBIN, cv::Mat imgPOLY, cv::Mat kernel, cv::Rect roiB, cv::Rect roiP);
	float CHECK_LINE(cv::Mat ImgPoly, RECT_F rcROI, bool bX);
	int PAT_TH(VOL_TH_RE sRE, float fPol, bool bX, cv::Mat imgBin_Temp, cv::Rect roiB, RECT_F* roiMAX, RECT_F* roiMIN, bool bAB);
	int FIND_TH2(cv::Rect roi, cv::Mat img, bool bX, RECT_F* pMAX, RECT_F* pMIN, bool bAB);
	bool FIND_POS2(cv::Mat imgBIN_BIN, cv::Mat ImgPoly, bool bX, float fPAT_Min, float fPAT_Max, VOL_TH_DATA* sDataP, VOL_TH_DATA* sDataB);
	int CHECK_POLY_BIN(cv::Mat Img, cv::Rect roi, bool bX, int nPOLY_DIFF);
#pragma endregion

	void SaveWorkImg(UCHAR* puc, int nW, int nH, CString fileName);
	void CreateDir(CString Path);
	BOOL IsExistDir(CString path);
};

