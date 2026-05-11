#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoLeadTip : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoLeadTip(void);
	virtual ~CPInsp_AlgoLeadTip(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	BOOL SearchSideTipPosition(AlgoLeadTip *pInspAlgoLeadTip, LeadTipSideTip sSideTip, RstLeadTipSideTip *sRst);
	BOOL SearchSideTipFind(AlgoLeadTip *pInspAlgoLeadTip, LeadTipSideTip sSideTip, RstLeadTipSideTip *sRst);
	BOOL SearchTipPosition(AlgoLeadTip *pInspAlgoLeadTip, int nSearchType, int nSearchRange, int nLeadTipPos, int nLeadDirection, int nPixelPercentValue, UCHAR *pucImgSrc, float *pfImgSrc, int nImgWidth, int nImgHeight, BOOL bUse2D, BOOL bUse3D, int &nResultPos, float &fHeigntMean, BOOL bDirInvert = FALSE);
	int SearchTipPosition2(UCHAR *pucImgSrc, float *pfImgSrc, int nImgWidth, int nImgHeight, AlgoLeadTip *pInspAlgoLeadTip);
	BOOL TeachTip(const InspAlgo &sInspAlgo, WndInfo &sWndInfo, WndAlgoImg &sWndAlgoImg, double dWndX, double dWndY, RstAlgoLeadTip * sRstAlgo, BOOL bTeach, float &fHeightMean);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	int TipMaster(const InspAlgo &sInspAlgo, int *ptrnArrLeadTipRst, int nLeadTipTotalCnt, RstAlgoLeadTip * sRstAlgo);
};

