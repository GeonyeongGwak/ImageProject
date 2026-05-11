#pragma once

#include "pinsp.h"
#include "ProcPil_ForeignPattern.h"
#include "OpenCV_Functions.h"

class CPInsp_ForeignPattern : public CPInsp
{
public:
	CPInsp_ForeignPattern(void);
	virtual ~CPInsp_ForeignPattern(void);
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);

	std::shared_ptr<CProcPil_ForeignPattern> m_procPil;
	int SetInspAlgo(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, double dAngle);
	int SetInspAlgo(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, double dAngle,cv::Rect re,bool bIsMin = false);
	int SetInspAlgo(const InspAlgo &sInspAlgo, UCHAR* ucImage, int nWidth, int nHeight, double dPartAngle, double dAngle);
	int InspForeign(bool bCheckBW = true);
	int TeachingModel(cv::Mat src, CString sPath,bool isNew,bool bIsMin,double dAngle, int nMargin);
	int resultImage(cv::Mat src, CString sPath,bool flag,double dangle);
	int ModelImage_Load(CString sPath,double dAngle);
	void PatternModelAndOption(bool flag, int erodeSize);
	void SetForeingPattern_MaskParam(bool bUseLineFilter, int dilateSize, bool bUseDisplay);
	void SetLineImg(cv::Mat LineImg, cv::Point pCenter);
	void GetForeignPatternMatchPos(POINTF* srcPtr);
private:
	AlgoBlob *m_pInspAlgo;

	cv::Mat m_mInspImg;
	int m_nImgAngle;
	UCHAR* InspImg;
	double _dAngle;
	POINTF m_pMatchPos[4];

#pragma endregion _BASE FUNCTION_

};