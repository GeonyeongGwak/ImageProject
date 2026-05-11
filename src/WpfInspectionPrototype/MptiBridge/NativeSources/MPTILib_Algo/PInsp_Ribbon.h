#pragma once
#include "PInsp_Foot.h"


class CPInsp_Ribbon:
	public CPInsp_Foot
{
public:
	CPInsp_Ribbon();
	~CPInsp_Ribbon();

#pragma region AutoSearch
	bool getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection);
#pragma endregion
	void SetBWForUIImage(CFoot_Model *pFoot, int nSX, int nSY, int nEX, int nEY, cv::Mat* cvFootBW, cv::Mat* cvWedgeBW, cv::Mat* cvWingBW);

	bool FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible);

	cv::Point FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible);

	//2 유형 검사
	int FootPointByType(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, FootPoins &fPoints, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, std::vector<cv::Mat> &Images, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible);
	bool Insp_Ribbon(CFoot_Model *pFoot, cv::Mat FootImg, cv::Mat WedgeImg, cv::Mat HImg, cv::Point& st, cv::Point& ed, double& seta, bool bMatchSuccess);

	double calcSeta(CFoot_Model *pFoot, cv::Mat FootImage, cv::Mat wedgeImage, cv::Point* CenterPo);
	double calcSeta2(CFoot_Model *pFoot, cv::Mat FootImage, cv::Mat wedgeImage, cv::Mat Image3D, cv::Point* CenterPo);
	BOOL InspFoot(CFoot_Model* pFoot, WndAlgoImg &sWndAlgoImg, RstAlgoFoot * sRstAlgo, TotalInspExceptArea stTieArea, AlgoColorOpt &ptrAlgoColorOpt, int nSelectFoot, UCHAR *ucArrDstImg, AlignResult * arrAlignRes, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, CString sSaveDebugPath = _T(""));
private:
};
