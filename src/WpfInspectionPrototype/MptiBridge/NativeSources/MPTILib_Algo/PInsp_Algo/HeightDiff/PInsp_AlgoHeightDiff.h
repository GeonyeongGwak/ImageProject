#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoHeightDiff : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoHeightDiff(void);
	virtual ~CPInsp_AlgoHeightDiff(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	BOOL InspHeightDiff(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImgBuf, RstAlgoHeightDiff *sRstAlgo, AlignResult * sAlignRes, int nAlignResCnt, UCHAR* puImgDst);

	void BlobSeparateRect(cv::Mat imgBlob,cv::Mat image3D, float * fArrMax, int nIdx, int * nArrMaxX, int * nArrMaxY, int * nArrType, float * fArrRst, int * nArrRstX, int * nArrRstY, float * fArrMin, int * nArrMinX, int * nArrMinY, int * nArrT , int * nArrL);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	void GetBlobImg(const InspAlgo &sInspAlgo, cv::Mat imgBlob, int nL, int nT, float* pfImgSrc, UCHAR *pucImgSrc, cv::Mat imgColor, int nImgWidth, int nImgHeight, tagAlgoBlobBase sBlobBase);
};

