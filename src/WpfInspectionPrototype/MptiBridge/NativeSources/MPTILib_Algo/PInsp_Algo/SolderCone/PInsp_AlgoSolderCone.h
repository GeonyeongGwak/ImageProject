#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoSolderCone : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoSolderCone(void);
	virtual ~CPInsp_AlgoSolderCone(void);

	enum ELevel
	{
		eLevel1 = 0,
		eLevel2,
		eLevel3,
		eLevelCnt
	};

	enum ELevel_VolumeDiff
	{
		eVolDifLevel_non = 0,
		eVolDifLevel_non2,
		eVolDifLevel_non3,
		eVolDifLevel1,
		eVolDifLevel2,
		eVolDifLevel3,
		eVolDifLevel4,
		eVolDifLevel5,
		eVolDifLevel6,
		eVolDifLevel7,
		eVolDifLevel8,
		eVolDifLevel9,
		eVolDifLevel10,
		eVolDifLevelCnt
	};

	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	BOOL InspSolderCone(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoSolderCone *sRstAlgo, TotalInspExceptArea stTieArea);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	BOOL TeachSolderCone(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, int nIdxSelectLv, RstAlgoSolderCone *pRstAlgo, UCHAR* puImgDst, int nStartX, int nStartY, TotalInspExceptArea stTieArea);
	BOOL CalcSolderCone(AlgoSolderCone * pInspAlgoSolderCone, int nIdxLevel, float * pfImgSrc, int nImgWidth, int nImgHeight, UCHAR* pUcArrTemp, RstAlgoSolderCone * sRstAlgo, TotalInspExceptArea stTieArea);
	int CalcSolderCone(float *pfImgSrc, int nImgWidth, int nImgHeight, double fMaxHeight, UCHAR* pUcArrTemp, double &dRateArea, TotalInspExceptArea stTieArea);
	void CircleFitting(std::vector<cv::Point> vPoList, double *centX, double *centY, double *radius, double *err);

	//VolumeDiff °Ë»ç¿ë
	BOOL CalcSolderVolumeDiff_Inspection(AlgoSolderCone * pInspAlgoSolderCone, int nIdxLevel, float * pfRstDiff, float * pfImgSrc, int nImgWidth, int nImgHeight, UCHAR* pUcArrTemp, RstAlgoSolderCone * sRstAlgo, TotalInspExceptArea stTieArea);
	BOOL CalcSolderVolumeDiff(AlgoSolderCone * pInspAlgoSolderCone, int nIdxLevel, float * pfRstDiff, float * pfImgSrc, int nImgWidth, int nImgHeight, UCHAR* pUcArrTemp, RstAlgoSolderCone * sRstAlgo, TotalInspExceptArea stTieArea);
	int CalcSolderVolumeDiff(float *pfImgSrc, cv::Point* CenterPo, int nImgWidth, int nImgHeight, double dHighestH, double dGapH, double fMaxHeight, UCHAR* pUcArrTemp,
		double &dMinDistance, double &dMaxDistance, POINTF *minPoints, POINTF *maxPoints, TotalInspExceptArea stTieArea, bool bUseSolderHighestVolumeDiff, bool bUseInspVolumePieHDiff, RECT *rcNGArea);


	void GetThetaAreaDist(cv::Mat cvInputImg, double gapDegree, double andRadi, double tarRadi, int ancSize, int tarSize, POINTF Acenter, POINTF Tcenter, std::vector<POINTF> AnchorCont, std::vector<POINTF> TargetCont, POINTF *maxPoints, POINTF *minPoints, double &maxDist, double &minDist);
	void GetFiduPos_LSQ(std::vector<cv::Point>&  Points, double *centX, double *centY, double *radius, double *err);
	double GetDist_ACenterToTCircle(POINTF vecBetweenCenters, double theta, double targetRadius);

};

