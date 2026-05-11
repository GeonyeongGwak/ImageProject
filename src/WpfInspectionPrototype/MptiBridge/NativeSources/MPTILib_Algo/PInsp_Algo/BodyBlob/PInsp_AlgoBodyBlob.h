#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoBodyBlob : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoBodyBlob(void);
	virtual ~CPInsp_AlgoBodyBlob(void);

	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr);
	BOOL InspBodyBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg,
		BOOL bTeach, CRect &rcBlobBody, InspRoiImgBuf &sInspImageData,
		RstAlgoBodyBlob * sRstAlgo, POINTF ptWndCenter, TotalInspExceptArea stTieArea, UCHAR *pUcImgBlob);

	BOOL InspBodyBlob_Dll(PIAL::_AlgoBodyBlob & algoBodyBlob, PIAL::Insp_Image& pImg_buf, CRect &rcBlobBody, PIAL::_RstAlgoBodyBlob * sRstAlgo, POINTF ptWndCenter, PIAL::_TotalInspExceptArea stTieArea, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::PI_Buff* pMask, PIAL::BodyInfo* pBodyInfo, double nDx, double nDy);

	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	BOOL TeachBodyBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* puImgDst, int nMaskViewingMode, InspRoiImgBuf &sInspImageData, RstAlgoBodyBlob *pRstAlgo, TotalInspExceptArea stTieArea);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
};

