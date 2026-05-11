#include "stdafx.h"
#include "Pinsp_Ball.h"
#include "MPTI.h"

CPInsp_Ball::CPInsp_Ball()
{
}
CPInsp_Ball::~CPInsp_Ball()
{
}

bool CPInsp_Ball::getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection)
{
	bool ret = false;
	return ret;
}

int CPInsp_Ball::FindFoot_Pat(CFoot_Model* pFoot, RstAlgoFoot * sRstAlgo, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, cv::Point* pCenter, double* pAngle, int nfootKind, int nFootDirection, UCHAR *ucArrDstImg, cv::Mat *wingCandiSecond, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	int ret = 0;
	return ret;
}

bool CPInsp_Ball::FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	//If the Foot cannot be found by pattern matching, the 2D and 3D image is used.
	bool ret = false;
	return ret;
}

cv::Point CPInsp_Ball::FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin)
{
	cv::Point pt;
	return pt;
}
