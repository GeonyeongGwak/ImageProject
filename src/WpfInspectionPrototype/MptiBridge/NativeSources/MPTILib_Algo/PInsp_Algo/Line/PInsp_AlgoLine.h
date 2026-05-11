#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoLine : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoLine(void);
	virtual ~CPInsp_AlgoLine(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	double InspectionCrossLine(UCHAR *ucArrSrcImg, int nWndW, int nWndH, int nMeasureDirection, bool bUseFixPos, int nCrossOpt, POINTF* poDrawLine, double* dRstAngle);
	void CuttingPlaneLine(AlgoLine algoLine, UCHAR *ucArrDst, int nImgWidth, int nImgHeight, bool bIsHorizon, int nDir, RstAlgoLine * sRstAlgo);
	double PerpendicularLine(UCHAR *ucImage, int nImgWidth, int nImgHeight, int nType, bool bHorizon, double dA, double dB, POINTF* poDrawLine);
	BOOL InspLine(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipImg, InspRoiImgBuf &sInspImageData, RstAlgoLine * sRstAlgo, TotalInspExceptArea stTieArea, UCHAR *ucArrDstImg = NULL, int nStartX = 0, int nStartY = 0);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
};

