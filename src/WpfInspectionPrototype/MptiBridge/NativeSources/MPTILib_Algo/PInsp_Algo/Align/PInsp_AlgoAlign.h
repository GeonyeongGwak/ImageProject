#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoAlign : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoAlign(void);
	virtual ~CPInsp_AlgoAlign(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr);
	BOOL InspAlign(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, CRect &rcBlobBody, InspRoiImgBuf &sInspImageData, RstAlgoAlign *sRstAlgo, UCHAR* ucArrDstImg = NULL, AlignResult * sAlignRes = NULL, int nAlignCnt = 0);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	BOOL TeachAlign(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, UCHAR* ucArrDstImg = NULL);
	double Calc_Theta(double x1, double y1, double x2, double y2, double insp_x1, double insp_y1, double insp_x2, double insp_y2);
	BOOL TeachLeadAlign(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoAlign *sRstAlgo, InspAlgoParam stAlgoParam);
};

