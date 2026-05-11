#pragma once
#include "../IPInsp_Algo.h"

//★★ 코드 작성 필요
class IPINSP_ALGO;
class CPInsp_AlgoDistance : public IPINSP_ALGO
{
private:
	InspPartParam* m_pParamArray;
	int m_nParamArraySize;
	InspectionResult* m_pInspectionResult;			// 현재 Part 검사 결과

	InspAlgoResult* m_pTargetAlgoRst;
	InspAlgoResult* m_arrAnchorAlgoRst[DISTANCE_ANCHOR_LIMIT];

	InspAlgo *m_pTargetInspAlgo;
	InspAlgo *m_pArrAnchorInspAlgo[DISTANCE_ANCHOR_LIMIT];

	AlignResult m_pTargetAlignRst;
	AlignResult m_arrAnchorAlignRst[DISTANCE_ANCHOR_LIMIT];

	bool m_bUseX;
	bool m_bUseY;
	bool m_bUseDist;
	bool m_bUseAngle;
	bool m_bUseTargetAxes;
	bool m_bUseTargetPnt;

	int m_nAnchorMode;
	int m_nAnchorCnt;
	int m_nPartAngle;

	double m_dTeachX;
	double m_dTeachY;
	double m_dTeachDist;
	double m_dTeachAngle;

	double m_dTeachMinX;
	double m_dTeachMaxX;
	double m_dTeachMinY;
	double m_dTeachMaxY;
	double m_dTeachMinDist;
	double m_dTeachMaxDist;

	double m_dTeachOffsetX;
	double m_dTeachOffsetY;
	double m_dTeachOffsetDist;

	POINTF m_ptTargetPnt;

public:
	CPInsp_AlgoDistance(void);
	virtual ~CPInsp_AlgoDistance(void);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);
	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);

	void InitVariable();
	bool SetArrDataToParams(int* ArrDataN, float* ArrDataF, RstAlgoDistance* sRstAlgo);
	bool GetTargetAnchorInfo(AlgoDistance *algo, AlignResult* arrAlignRes, RstAlgoDistance* sRstAlgo);
	bool SetRstItems(AlgoDistance *algo, AlignResult* arrAlignRes, RstAlgoDistance* sRstAlgo);
	bool GetAlignResultData(AlignResult *arrAlignRes, AlignResult &curAlignRes, int curWndID, int nSize, int a);
	bool IsAppropriateResult(InspAlgoResult *algoRst, bool &bIsOKX, bool &bIsOKY, int AnchorIdx);
	void GetAngleAppliedCoord(double srcX, double srcY, double &dstX, double &dstY, int nAngle);
	void CorrectCoordinate(int nCurWndIndex, AlignResult * sptrAlignRes, InspAlgo sInspAlgo, int nAlignTotalCnt, double &dOffsetX, double &dOffsetY);
};