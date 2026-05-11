#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoBarcode : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoBarcode(void);
	virtual ~CPInsp_AlgoBarcode(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr);

	BOOL InspBarcode(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoBarcode *sRstAlgo, PIAL::Insp_Image* pImg_buf = nullptr);
	BOOL InspBarcode_Halcon(const InspAlgo & sInspAlgo, PIAL::Insp_Image* pImg_buf, std::vector<PIAL::_AlignResult> vecAlignResult, RstAlgoBarcode * sRstAlgo);

	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);

	BOOL CompareTarget(AlgoBarcode *pInspAlgo, RstAlgoBarcode *sRstAlgo);
	BOOL CompareTarget2(AlgoBarcode *pInspAlgo, RstAlgoBarcode *sRstAlgo);
	BOOL CrossCheck(CString sFilter, CString sRst, CString &sResultF);
	void SetTypeName(int Type, CString &TypeName);
	int SetDataType(int Type, bool bHALCON = false);
	int BarcodeSplitInspTarget(RstAlgoBarcode * rst);
	BOOL ValidateSplitData(CString sRstBarcode, int nType);
	void SetInspectWindowPosition(InspAlgoParam algoParam, RstAlgoBarcode * rst);

	std::map<CString, CString> m_MapBarcodeInspTarget; // Hotayi - 2DTarGet 문자 저장
};

