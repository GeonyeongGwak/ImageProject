#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoShapeX : public IPINSP_ALGO
{
private:
	InspRoiImgBuf m_sInspImgBuf;
	CProcMil_Color* m_procMil;
	int m_nTabIndex = 0;
	CPInsp pInsp;
	int m_nModelListCnt = 0;

public:
	CPInsp_AlgoShapeX(void);
	virtual ~CPInsp_AlgoShapeX(void);
	virtual int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	bool CalcHistoBW(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoShapeX* sRstAlgo, UCHAR* ucDstImg, UCHAR* ucDsAreatImg, int nTabIdx, bool bTeach, bool bUseTabAreaOther);
	int Histogram(UCHAR* ucSrc, UCHAR* ucMask, int nWidth, int nHeight, CRect rtRoi);
	BOOL ShapeX_LoadImageFile(CString sPath);
	int GetModelCnt(wchar_t* arrPath, double dResolX, double dResolY);
	BOOL ModelMaskChange(cv::Mat& newMask, wchar_t* arrPath, int nIdx);
	BOOL ModelDelete(wchar_t* arrPath, int nIdx);
	BOOL ModelAdd(wchar_t* arrPath, PIAL::PI_Buff* Src, PIAL::PI_Buff* p3D, PIAL::PI_Buff* pBin, PIAL::_Bin bin);
	BOOL ModelAddWithType(wchar_t* arrPath, PIAL::PI_Buff* Src, PIAL::PI_Buff* p3D, PIAL::PI_Buff* pBin, PIAL::_Bin bin, int targetType, bool searchByShape);
	BOOL AlgoCnt(int nCnt);
	BOOL LoadModelList(SShapeXfileAlgoPath * ShapeXPath, bool bMultiProcessLoad = false);
	BOOL CheckModelList(SShapeXfileAlgoPath * ShapeXPath, bool bMultiProcessLoad = false);
	BOOL clearModelList(int nLane);
	BOOL DrawShapeXModel(PIAL::PI_Buff* Src, PIAL::PI_Buff* dst, AlgoShapeX* palgoShapeX,int* ModelIdx, POINTF* poArrCenter);
	BOOL DrawShapeXModelMatching(PIAL::PI_Buff* Src, PIAL::PI_Buff* dst, AlgoShapeX* palgoShapeX, int ModelIdx,int SeleteROI);
	int ShapeXModelKey(int ModelIndex);
};

