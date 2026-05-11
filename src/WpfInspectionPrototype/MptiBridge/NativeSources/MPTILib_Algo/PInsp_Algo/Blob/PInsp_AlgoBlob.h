#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoBlob : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoBlob(void);
	virtual ~CPInsp_AlgoBlob(void);

	std::vector<BlobNGImg> vBlobNGImg;
	bool BlobNgImageSave;
	CString m_sBoard;
	CString m_sPartCode;
	CString m_sRefID;
	CString m_sModuleID;
	CString m_sWindowID;
	CString BlobImgPath;
	bool AINgImageSave;

	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) { return TRUE; }
	void BlobDiagonalDistance(std::vector<CRect> blobRect, int cnt, std::vector<double>& rstD);
	BOOL InspBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, AlgoCoordinate* coordinateAlgo, double dPartW, double dPartH, RstAlgoBlob * sRstAlgo, TotalInspExceptArea stTieArea, int nInspType, int nOffX_pix, int nOffY_pix, POINTF poCenter, int nInspectionMode, UCHAR *pUcImgBlob = NULL);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	BOOL TeachingBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, InspRoiImgBuf &sInspImageData, const AlgoCoordinate &coordinateAlgo, WndInfo sWndInfo, UCHAR *puImgDst, RstAlgoBlob * sRstAlgo, TotalInspExceptArea stTieArea, int nInspType, int nBlobMode, UCHAR* ucArrDstImgAC, bool bTeach);
	POINTF CalcForiegnPatternCenter(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, double dWndCX, double dWndCY, double dPartW, double dPartH,
		TotalInspExceptArea stTieArea, int nInspType, cv::Rect& rtArea, UCHAR *pUcImgBlob = NULL);
	void BlobGaussianLength(int nImgWidth, int nImgLength, std::vector<CRect> blobRect, int * nLabel, int cnt, std::vector<double>& rstD);	// BJY 2019-07-03: Blob계산 후 호출 해야함.
	void BlobShiftDistance(int nImgWidth, int nImgLength, std::vector<CRect> blobRect, int * nLabel, int cnt, double& nRstD, int& nDir_Index, std::vector<cv::Point>& vLinePtr, double nStdLength);
	void _GaussianLength(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, std::vector<double>& rstD);	// BJY 2019-07-03: 이진화된 이미지를 입력해야 함.
	void _ShiftLength(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, double& nRstD, int& nDir_Index, std::vector<cv::Point>& vLinePtr);
	bool CheckBlobShiftImg(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch);
	int BlobShiftImgSet(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, UCHAR *dstByimgSrc, UCHAR *rectByimgSrc, int nPitch, std::vector<cv::Point2f>& vPoint, int nShiftCase);
	int SearchBlobShiftLine(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, UCHAR *rectByimgSrc, std::vector<cv::Point2f>& vPoint, int nShiftCase);
	void BlobTeachAC(int nImgWidth, int nImgHeight, RstAlgoBlob * sRstAlgo, AlgoBlob * pAlgoBlob, InspRoiImgBuf & sInspImageData, UCHAR * ucArrBlobDst, WndAlgoImg & sWndClipAlgo, UCHAR* ucArrDstImgAC, int nOrgWidth, int nOrgHeight, int nROIWidth, int nROILength, int nClipStartX, double dClipWidth, int nClipStartY, double dClipHeight, UCHAR * puImgDst);
	void WndArrayBlobImg(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, UCHAR *puImgDst, RstWndArr * sRstAlgo);
	int SetAIAlgoImageParam(CString sBoard, CString sPartCodeNWndName, CString sRefId, CString sModuleID);
	void BlobImageSavePath(CString sTimePath);
	void AIImageListSave(int nCurJob);
	std::vector<cv::Point> thickness(cv::Mat Image, int nWndDir, cv::Point Center, std::vector<std::vector<cv::Point>> contours, cv::Rect rcBlob, long lLebel);
	std::vector<cv::Point> thickness2(cv::Mat Image, int nWndDir, cv::Point Center, std::vector<std::vector<cv::Point>> contours, cv::Rect rcBlob, std::vector<cv::Point> vLinePo, long lLebel);
	std::vector<cv::Point> thickness3(cv::Mat Image, int nWndDir, cv::Point Center, std::vector<std::vector<cv::Point>> contours, cv::Rect rcBlob, std::vector<cv::Point> vLinePo, long lLebel);
	cv::Mat ImageThining(cv::Mat Img);


	bool getLineIntersection(cv::Point2f a1, cv::Point2f a2, cv::Point2f b1, cv::Point2f b2, cv::Point2f& intersection);
	int convertDirectionFlag(int dirFlag);
	void interpolHull(std::vector<cv::Point>& hull, std::vector<cv::Point2f>& interpolated, float spacing);
	int InspThickness(AlgoBlob *pAlgoBlob, UCHAR* pUcImgSrc, UCHAR* pUcImgBlob, UCHAR* ucArrBlobDst_except, int nImgWidth, int nImgHeight, int nCntBlobExcept, double dArea, double dArea_except, CRect rcBlob, float fWndCenterX, float fWndCenterY, float fWndWidth, float fWndLength, RECT& ThminRect, RECT& ThmaxRect, double& minVal, double& maxVal, RstAlgoBlob * sRstAlgo);
	int InspThickness2(AlgoBlob* pAlgoBlob, UCHAR* pUcImgSrc, UCHAR* pUcImgBlob, UCHAR* ucArrBlobDst_except, int nImgWidth, int nImgHeight, int nCntBlobExcept, double dArea, double dArea_except, CRect rcBlob, float fWndCenterX, float fWndCenterY, float fWndWidth, float fWndLength, RECT& ThminRect, RECT& ThmaxRect, double& minVal, double& maxVal, RstAlgoBlob* sRstAlgo,bool bTeach);

	void AiResultClear(int nCurJob);
};

