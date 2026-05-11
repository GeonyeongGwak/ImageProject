#pragma once
#include "PInsp_Mount.h"
#include "pinsp.h"
#include "Proc3D.h"
#include "ipps.h"
#include "ippi.h"
#include "BilinearInterpolation.h"	//NYJ 2021/02/26
#include "PInsp_LeadSolder.h"
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")

typedef struct tagBlobNGImg
{
	CString sGray;
	cv::Mat mGrayImg;
	CString sBinary;
	cv::Mat mBinImg;
	CString sMask;
	cv::Mat mMask;

}BlobNGImg;

enum m_eAlgoOPT
{
	m_eAlgoOPT_PadAlign_ROI = 0x01,
	m_eAlgoOPT_PadAlign_WND = 0x02,
};
class CPInsp_Mount;
class CPInsp_Algo : public CPInsp
{
public:
	CPInsp_Algo(void);
	~CPInsp_Algo(void);
	CPInsp_Mount m_inspMount;
	CPInsp_LeadSolder m_pInspLeadSolder;
	bool bFastCompose;
public:
	virtual int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	virtual int CloseDevice();
	int m_nAlgoOPT = 0;
	double m_dNO_SOLDER = 5;

#pragma region  Image Func

	void ArrayCopy_float(const float * src, float ** dst, int width, int length);
	void ArrayCopy_float(WndAlgoImg * sWndAlgoImg);
	int WndAlgoImg_Update_fromCV(WndAlgoImg * sWndAlgoImg);
	int WndAlgoImg_Update_fromCV_MixImg(WndAlgoImg * sWndAlgoImg, bool bIsGrid = false);
	int WndAlgoImg_Update_fromCV_Color(AlgoColorOpt * ptrAlgoColorOpt, bool bIsLoadFovRawDataImage);
	void Update_Image_TOCV(WndAlgoImg * sWndAlgoImg, UCHAR * srcImg, UCHAR * dstImg);
	void Update_Image_TOCV_Color(int width, int height, UCHAR * srcImg, UCHAR * dstImg);
	void RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer, 
							int nFullImageSize, int nFullImageWidth, int nFullImageHeight, int nROIImageSize, 
							double dROIStartX, double dROIStartY, int nROIWidth, int nROIHeight, int nRedValue, 
							int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle = 0);
	void FullImageCompose(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer, 
							int nFullImageSize, int dfullImageWidth, int dfullImageHeight,
							double dROICenterX, double dROICenterY, int nRoiWidth, int nROIHeight, 
							int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage);	
	void FullImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage);
	void ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage, double dAngle = 0);
	BOOL GetCoordinate(const WndAlgoImg &sWndAlgoImg, const WndInfo &sWndInfo, AlgoCoordinate *pAlgoCoodi, BOOL bWndErr = FALSE);
	BOOL GetWndClip_2(WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, const WndInfo &sWndInfo, int *nStartX = NULL, int *nStartY = NULL);
	void SetBackWndClip_2(WndAlgoImg &sWndAlgoImg, const WndInfo &sWndInfo, UCHAR * pClip, UCHAR * pOrg);	// YJS 2016/11/04
	BOOL GetWndClip_3(WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, const WndInfo &sWndInfo, int *nStartX, int *nStartY, BOOL bIsLoadFovRawDataImage = FALSE, bool bPartErr = true);
	BOOL GetWndClip_4(InspRoiImgBuf &sImgBuf, InspRoiImgBuf &sClipImgBuf, const WndInfo &sWndInfo, int nStartX = -1, int nStartY = -1);
	BOOL GetWndClip_5(WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, const WndInfo &sWndInfo);
	BOOL GetWndClip_3_2(WndAlgoImg& sWndAlgoImg, WndAlgoImg& sWndClipAlgo, const WndInfo& sWndInfo, int* nStartX, int* nStartY, BOOL bIsLoadFovRawDataImage = FALSE, bool bPartErr = true);
	BOOL TeachAIData(TeachAIParam stParam, AlgoColorOpt &ptrAlgoColorOpt);
	RECT GetWndROI(WndAlgoImg &sWndAlgoImg, const WndInfo &sWndInfo, bool bPartErr);
	BOOL ClipBuffer_PTT(float*pZMap, UCHAR* pRED, UCHAR* pGREEN, UCHAR* pBLUE, int nWIdth, int nHeight, int nROILeft, int nROITop, int nROIWidth, int nROILength, UCHAR** pTarget, int& nTargetSize, float fBottom = 0);

	BOOL Get_PseudoColorMap(float * fZmapSrc, UCHAR * ucMask, int width, int length, UCHAR* ucArrDstImg);
	BOOL Set_PseudoColorMap(float fMin, float fMax);

#pragma endregion
};