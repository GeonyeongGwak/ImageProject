#pragma once
#include "PInsp_Foot.h"


class CPInsp_Ball:
	public CPInsp_Foot
{
public:
	CPInsp_Ball();
	~CPInsp_Ball();

#pragma region AutoSearch
	bool getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection);
#pragma endregion

	bool FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible);

	int FindFoot_Pat(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, cv::Point* pCenter, double* pAngle, int nfootKind, int nFootDirection, UCHAR *ucArrDstImg, cv::Mat *wingCandiSecond, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible = false);
	cv::Point FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin);

private:
};