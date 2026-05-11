#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoHeightMean : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoHeightMean(void);
	virtual ~CPInsp_AlgoHeightMean(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	BOOL InspHeightMean(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, InspRoiImgBuf &sInspImgBuf, TotalInspExceptArea stTieArea, RstAlgoHeightMean *sRstAlgo, UCHAR* puImgDst = NULL, int nStartX = 0, int nStartY = 0, bool bTeach = false);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	double InspSelectAreaHeightMean(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RECT &rcSelectArea, int &nLeadDir);
	BOOL InspHeightMnMx(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, RECT &rcSelectArea, int nMnMx, bool bHighest = false);
	float XY_Check(float* pfImgSrc, int nImgWidth, int nImgHeight, int x, int y, int nHighestX, int nHighestY);
	float XY_Check(float* pfImgSrc, int nImgWidth, int nImgHeight, float fH, int nHighestX, int nHighestY);
};

