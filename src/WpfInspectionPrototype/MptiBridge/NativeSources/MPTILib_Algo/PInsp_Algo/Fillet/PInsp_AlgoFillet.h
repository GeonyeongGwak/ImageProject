#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoFillet : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoFillet(void);
	virtual ~CPInsp_AlgoFillet(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	BOOL CreateFilletHeightLine(WndAlgoImg &sWndAlgoImg, std::vector<POINTF> &arrDstFilletLinePoint, int &nFilletLineCnt, int nTipDirection);
	BOOL CreateFilletHighestHeightLine(WndAlgoImg& sWndAlgoImg, std::vector<POINTF>& arrDstFilletLinePoint, int nTipDirection);
	BOOL MesurementSpec(AlgoFillet pInspAlgoFillet, WndAlgoImg &sWndAlgoImg, std::vector<POINTF> arrDstFilletLinePoint, std::vector<float> &arrRstFilletHeight, int nFilletLineCnt, int nTipDirection);
	BOOL InspFillet(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nWndDir, RstAlgoFillet *sRstAlgo);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
};

