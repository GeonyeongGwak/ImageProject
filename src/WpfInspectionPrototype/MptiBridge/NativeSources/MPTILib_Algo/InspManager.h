#pragma once

#include "Manager.h"

#include "InspParamDef.h"
//#include "MBarcodeManager.h"
#include "PInspAlgoWrapper.h"

#include "PInsp_Algo.h"
#include "PInsp_WireBonding.h"
#include "FOREIGN/FOREIGN_INSP.h"
//#include "PInsp_Pattern.h"
#include "ProcMil.h"
//#include "PInspDecision.h"
#include "ExtInspSub.h"
#include "PAD_PAT_VOL.h"
#include "MNgManager.h"
#include "ipp.h"
#include "ippcv.h"
#pragma comment(lib, "ippcv.lib")

#define dfUser_LightCnt	10		// User light 합성 개수

enum m_eInspTact
{
	m_eInspTact_Cal3D = 0,
	m_eInspTact_Cal3DFr,
	m_eInspTact_3DSetParam,
	m_eInspTact_PartCalc3D,
	m_eInspTact_PartCalc3DFr,
	m_eInspTact_MultiProSetInsp,
	m_eInspTact_MultiProSetwait,
	m_eInspTact_WndM,
	m_eInspTact_WndA,
	m_eInspTact_WndO,
	m_eInspTact_WndL,
	m_eInspTact_WndS,
	m_eInspTact_WndT,
	m_eInspTact_WndSB,
	m_eInspTact_WndP,
	m_eInspTact_WndUser,
	m_eInspTact_WndBGA,
	m_eInspTact_WndInspInit,
	m_eInspTact_WndInspEnd,
	m_eInspTact_WndAlignPart,
	m_eInspTact_WndInspDecision,
	m_eInspTact_WndDelete1,
	m_eInspTact_WndDelete2,
	m_eInspTact_Total,
};

enum insp_msg
{
	eIMSG_SUCCESE = 0,
	eIMSG_FAIL
};
class IPINSP_ALGO;
class InspManager : public CManager
{
public:
	InspManager();
	~InspManager();

	//////////////////
	CProcMil* m_procMil;
	float m_fInspTact[m_eInspTact_Total];
	float m_fInspAlgoTact[eAlgoNum];
	float m_fFrTact[m_eFrTact_Total];
	float m_fVolTact[m_eVOL_TACT_Total];
	BOOL m_bInspSkipGroupAlgo[eINSP_Total];
	BOOL m_bInspSkipGroupWnd[eINSP_Total];
	unsigned long long m_nInspSkipAlgo[eINSP_Total];


	PartImgBuf m_vPartImgBuf;
	///////////////////

	CPInsp_Mount* m_pMountInsp;
	CPInsp_Pattern* m_pPatternInsp;
	CPInsp_OCR* m_pOcrInsp;
	CPInsp_Color* m_pColorInsp;
	CPInsp_LeadSolder* m_pLeadSolderInsp;
	//CPInsp_BGA* m_pBGAInsp;
	CPInsp_Grid* m_pGridInsp;	// YJS 2016/11/04

	CPInsp_Mount* m_pMountTeach;
	CPInsp_Pattern* m_pPatternTeach;
	CPInsp_OCR* m_pOcrTeach;
	CPInsp_Color* m_pColorTeach;
	CPInsp_LeadSolder* m_pLeadSolderTeach;
	//CPInsp_BGA* m_pBGATeach;
	CPInsp_POCR* m_pPOCR;
	CPInsp_ForeignPattern* m_pForeignPattern;
	CPInsp_WireBonding* m_pWireBondingInsp; //YBJI 2019/10/01
	CPInsp_Tab* m_pTab;

#pragma region _Foreign_Func_

private:
	CPInsp_Algo m_inspForeign;
	InspPartInfo *m_pInspBoardInfo_Foreign;
	InspForeignInfo m_ForeignData;
	ForeignParamROI *m_ForeignParamROI;
	//InspFovForeignResult* m_inspForeignResult;		// 현재 FOV의 이물 검사 결과

public:
	CPInsp_Algo& GetInspForeign() { return m_inspForeign; }
	InspPartInfo* GetInspPartInfoForeign() { return m_pInspBoardInfo_Foreign; }
	InspForeignInfo GetForeignData() { return m_ForeignData; }
	ForeignParamROI* GetForeignParamROI() { return m_ForeignParamROI; }
	DefaultColorXYStd GetDefaultColorXYStd(){ return m_DefaultColorXYStd; }
// 	void SetPartParam_Foreign(InspPartInfo* boardInfo);
// 	int ReleaseInspForeignResult();
// 	int GetInspForeignResult(InspFovForeignResult* retResult);
// 	void DeleteForeignImgBuffer();
// 	void SetForeignParam(InspForeignInfo ForeignParam, ForeignParamROI* vForeignParamROI);
// 	int Inspection_Foreign_new(ForeignData sData_ROI);
// 	int InspForeignAlgo(float* pfOrg3D, int nOrgW, int nOrgH, InspFovForeignResult* ForeignResult, ForeignData sInspData, int nFovIDX);
	byte ExceptForeignImage(int nMode, cv::Mat imgExcept, ForeignData sData_ROI);
	void ExceptForeign(int nType, cv::Mat imgExcept, ForeignData sData_ROI);
	void InspFRArea(ForeignData sInspData, cv::Mat imgArea);
	bool SetROI(ForeignData sInspData, int nIndex, cv::Mat img, int nW, int nH, double dFactorX, double dFactorY, byte byValue);
	bool SetExceptForeignImage(int nIndex, int nType, CRect rtFOV, UCHAR* pucDst, int nW, int nH, byte byValue = 0, bool bExtraPer = false, float fExtraPixelCnt = 0);
	int FillOutOfInspArea(RECT rcROI, int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue = 0, BOOL bConvertExceptROI = FALSE, bool bExtraPer = false, float fExtraPixelCnt = 0);
	bool PolygonAreaCalc() { return true; };
	void FillPolygon(cv::Mat Dst, POINTF *pts, int nMaxCount, cv::Scalar color);
	void MorErode(cv::Mat Src, cv::Mat Dst, int nShape = 0, int ksize = 3);
	bool PtInPolygon(POINTF pt, POINTF *pts, int ptNum);
	int Inspection_Foreign_new(ForeignData sData_ROI, FR2DData sFR2DData, ForeignParamROI* arrROI);
	int GetInspForeignResult(InspFovForeignResult * retResult);
	//int InspForeignAlgo(float* pfOrg3D, int nOrgW, int nOrgH, InspFovForeignResult* ForeignResult, ForeignData sInspData, int nFovIDX, FR2DData sFR2DData, ForeignParamROI* arrROI);
	void CalcForeignResult(InspFovForeignResult *arrTempRst, InspFovForeignResult* ForeignResult);

#pragma endregion _Foreign_Func_

// 	CProcMil* m_procMil;
// 	BOOL m_bInspSkipGroupAlgo[eINSP_Total];
// 	BOOL m_bInspSkipGroupWnd[eINSP_Total];
// 	unsigned long long m_nInspSkipAlgo[eINSP_Total];
public:

	lightData m_sLightData[3];
	int m_nUserMap;
	lightData m_sUserLightData[3];
	bool m_bPsrUpdateChecking;
	bool m_bPsrNeedUpDate;
	InspFovForeignResult* m_inspForeignResult;
	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;
	int m_nCompositeLightMode;
	double m_dAlgoListTack[eAlgoNum];
	int m_nAlgoCnt[eAlgoNum];
	DWORD m_dwSectorLogTemp[10];
	double m_dSectorLog[10];
	bool m_bSetAIData;
	UCHAR* m_pucWndBinImg;

	PInspAlgoWrapper* m_PInspWrapper;
	//void * m_pPatternInsp;
	CPInsp_Algo  m_InspAlgo; //CPInsp_Algo mpInsp; //CPInsp_Algo* mpInsp;	//CPInsp_Algo m_InspAlgo;
	//CPInsp_WireBonding* m_pWireBondingInsp; //CPInsp_WireBonding* m_pWireBondingInsp;
	CMNgManager * m_NgManager;
	InspPartInfo* m_pInspBoardInfo = nullptr;
	InspPartParam* m_pParamArray = nullptr;
	InspectionResult* m_inspectionResult;			// 현재 Part 검사 결과
	InspImgBuf *m_pFovImgBufBig;
	PAD_PAT m_PAD_PAT_VOL;
	//Shaerd Mem 사용시 Part Index 
	int m_nToolIndex = 0; 
	std::vector<int> m_vPartIndex;
	int winid;
	UCHAR* m_ucPadCalcBuffer[dfUser_LightCnt];
	UCHAR* m_ucPadRstBuffer[dfUser_LightCnt];

	int m_ParamArraySize = 0;
	FOREIGN_INSP m_FR;

	int* m_inspItemCnts;//[eINSP_WND_CNT];
	int** m_inspItemID;//[eINSP_WND_CNT][MAX_WINDOW_CNTS];

	int* m_groupIndexCnts;//[eINSP_WND_CNT]; //검사 항목별 그룹 종류 개수.
	int** m_groupIndex;//[eINSP_WND_CNT][MAX_GROUP_CNTS]; //검사 항목별 가지고 있는 그룹 인덱스

	int* m_groupID;//[MAX_GROUP_CNTS];
	int* m_groupWndCnts;//[MAX_GROUP_CNTS]; // 그룹에 속해있는 윈도우 개수.
	int** m_groupWndID;//[MAX_GROUP_CNTS][MAX_WINDOW_CNTS];  //그룹 인덱스 별 윈도우 번호

	int** m_inspWndOrder; //실제 검사한 순서
	void AllocBuff();
	void DeleteBuff();

	PInspAlgoWrapper* GePInspWrapper() { return m_PInspWrapper; }
	PAD_PAT* GetPAD_PAT() { return &m_PAD_PAT_VOL; }

	void SetResolution(int fovWidth, int fovLength, double resolX, double resolY);
	void SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB);
	void SetFootOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax);
	void SetInspParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize);

	void SetPartParam(InspPartInfo * boardInfo, InspPartParam * pWindowArray, int nWindowCnt);

	void SetInspItemInfo(int * inspItemCnts, int ** inspItemID, int ** inspWndOrder);
	
	void SetInspGroupInfo(int * groupIndexCnts, int ** groupIndex, int * groupID, int * groupWndCnts, int ** groupWndID);

	//Parameter Sorting, And Result
	void SortingParamater(InspPartParam *pParamArray, int nParamArraySize);
	void InitResultVal();		// Insp Param Set, Result 생성 및 초기화
	void CreateInspWndResult(InspWndResult ** pArrWndResult, int wndType, InspParamTemp * wndParam);
	void SetInspParamTemp();
	void DeleteResultVal();
	void DeleteResultVal(InspectionResult*& pInspectionResult);


	int InspProc();
	int InspNormal(bool bUseDecision = false);
	int InspNormal_Ver2(bool bUseDecision = false);

	//int InspWindowAlgo(int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, PIAL::PInspDataSet* InspData, InspAlgoTempResult * ptrInspAlgoTempResult = NULL);
	//int InspBGAWindowAlgo(int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, PIAL::PInspDataSet* InspData);
	
	int InspBGAWindowAlgo(int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult * WndResult, PIAL::PInspDataSet * InspDataSet, std::vector<InspRstPolyAlgo>* vpInspRstPoly);

	int GetInspectionResult(InspectionResult* retResult);

	void FindAlignResult(AlignResult*& dst, AlignResult* org, int nWndIndex, int nAlignWndID, int nAlignWndIdx, int* nAlignCnt);
	int FindAlignResultData(int nWndIndex, AlignResult* arrAlignRes, AlignResult* curAlignResult, int* ptrnTotalCnt, int WindowId = 0);
    
	int m_NumWireAlgo;
	UCHAR* m_puImgWireInterval;
	UCHAR * m_puImgFootResult;

	void PreviousInspWireShort();
	void InspWireShort();

	
	//CPInsp_WireBonding* GetWireBondingInsp();
// 	void * GetPatternInsp();
// 	void * GetPocrTeach();
// 	void * GetBarcodeInsp();

	CPInsp_Algo& GetInspAlgo() { return m_InspAlgo; }
	CPInsp_Algo* GetPtrInspAlgo() { return &m_InspAlgo; }

	CPInsp_OCR* GetOcrInsp() const { return m_pOcrInsp; }
	CPInsp_OCR* GetOcrTeach() const { return m_pOcrTeach; }
	CPInsp_POCR* GetPocrTeach() const { return m_pPOCR; }
	CPInsp_Color* GetColorInsp() const { return m_pColorInsp; }
	CPInsp_Grid* GetGridInsp() const { return m_pGridInsp; }	// YJS 2016/11/04
	CPInsp_Color* GetColorTeach() const { return m_pColorTeach; }
	CPInsp_Pattern* GetPatternInsp() const { return m_pPatternInsp; }
	CPInsp_Pattern* GetPatternTeach() const { return m_pPatternTeach; }
	CPInsp_ForeignPattern* GetForeignPattern() { return m_pForeignPattern; }
	CPInsp_WireBonding* GetWireBondingInsp() { return m_pWireBondingInsp; } //YBJI 2019/10/01
	CPInsp_Tab* GetTabInsp() const { return m_pTab; }
	InspPartInfo* GetInspPartInfo() { return m_pInspBoardInfo; }
	InspPartParam* GetInspPartParam() { return m_pParamArray; }
	int GetInspPartParamSize() { return m_ParamArraySize; }
	InspImgBuf* GetPtrFovImgBufBig() const { return m_pFovImgBufBig; }
	
#pragma region _MINSPMANAGER FUNCTION_
	bool m_bSetResolution = false;
	bool m_OCRNGimgSave;
	bool m_OCROKimgSave;
	bool m_OCRNGParamSave;
	int m_nOCRSimilarityScore;
	int m_nHomogeneityScore;
	bool m_bstringMulti;
	bool m_bImproveOCRVer;
	int m_nrotateInspScore;
	bool m_bNewInspection = true;
	DefaultGrayMeanStd m_DefaultGrayMeanStd;
	DefaultColorXYStd m_DefaultColorXYStd;

	AlgoDebuggingMode m_AlgoDebuggingMode;
	AlgoDebuggingMode m_AlgoDebuggingMode_Rear;
	BilinearInterpolation* m_CamInterpolation;
	int GetWindowID(int nWinIdx);
	void GetPartImageSize(int & nWidth, int & nHeight);
	void SegmentationAI_PartImg_Setting(UCHAR* pSrc, int width, int height, int nStride);
	
	bool LicenseCheck(MIL_INT lic = M_LICENSE_OCR);

	
	int ForeignPatternSet(const InspAlgo &sInspAlgo, UCHAR* ucArrBlobDst, int nImgWidth, int nImgHeight, double dWndAngle, double dAngle, POINTF* pFPMatchPos);
	void ForeingLineImgSet(UCHAR* img, int width, int height, cv::Point pCenter);
	int GetInspectionResultRef(InspectionResult* &retResult);
	void InvertDisplay(UCHAR* ucDst, int nwidth, int nheight, int nStartROIX, int nStartROIY, int nROIWidth, int nROIHeight, POINTF* pForignPos);
	void ReleaseImgSave(CString FullPath, bool OKNg, WndAlgoImg* wndImg, bool bhist = false);
	void TabNGRectChangePosition(InspWndResult* WndResult, int nAlgo, double dX, double dY);
	void TabNGRectChangePosition(RstAlgoTab * sRstAlgo, int nAlgo, double dX, double dY);
	void TabNGRectChangePosition(RstAlgoShapeX * sRstAlgo, double dX, double dY);


#pragma endregion _MINSPMANAGER FUNCTION_
#pragma region _MOUNT FUNCTION_
	int Mount_Teach(UCHAR* retTrueImg, int width, int height);

	int Mount_GetBodyRect(RECT* retRect);
	//void Mount_GetBodyRect(RECT* retRect);
	POINTF FP_CenterPtr;

#pragma endregion _MOUNT FUNCTION_
#pragma region _ALIGN FUNCTION_
	BOOL m_bChangeRotateCenter;

#pragma endregion _ALIGN FUNCTION_
	//CPInsp_Algo& GetInspAlgo() { return mpInsp; }
	//CPInsp_Algo* GetPtrInspAlgo() { return &mpInsp; }
// 	CPInsp_Algo& GetInspAlgo() { return m_InspAlgo; }
// 	CPInsp_Algo* GetPtrInspAlgo() { return &m_InspAlgo; }


	// 윈도우 알고리즘 검사 함수
	//int InspWindowAlgo2(int wndType, int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, InspAlgoTempResult * ptrInspAlgoTempResult = NULL, bool bLeadInsp = false);
	int InspWindowAlgo2(int wndType, int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, InspAlgoTempResult * ptrInspAlgoTempResult, bool bLeadInsp);
	//Multi 전용
	int InspWindowAlgo3(int wndType, int nWndIndex, AlignResult * arrAlignRes, int nAlignWndIdx, InspWndResult* WndResult, PIAL::PInspDataSet* InspDataSet, InspAlgoTempResult * ptrInspAlgoTempResult = NULL, bool bLeadInsp = false, vector<InspRstPolyAlgo>* vpInspRstPoly = NULL);

	bool CopyAICroppedToWnd2D(PIAL::Insp_Image* pImg_buf, int wnd_w, int wnd_h, const UCHAR* partImgAI, int aiImgSizeX, int aiImgSizeY);
	BOOL WndSizeChange(InspAlgoType nAlgoType, AlgoCoordinate &coordinateAlgo, WndAlgoImg &wndImg,
		int nCurWndIndex, int &nWidth, int &nHeight, double &dX, double &dY, AlignResult * sptrAlignRes, int nWndinspType, InspAlgo sInspAlgo,
		BOOL bUseLeadTipSideOption, double dLeadTipShiftX, double dLeadTipShiftY, double dLeadTipSize, int nLeadTipDir, int nAlignTotalCnt, int &nOffX_pix, int &nOffY_pix);

	void DeleteProcMil();

	void ColorDataInput(InspAlgo sInspAlgo, int nWidth, int nHeight, double dX, double dY, int nClipW, int nClipH, InspRoiImgBuf * sInspImgBuf, byte byIDX = 0, int nInspCameraType = -1);
	void SetColorData(BOOL bAngleColor, int nWidth, int nHeight, InspRoiImgBuf * sInspImgBuf, int nType, double dX, double dY, int nClipW, int nClipH, int nSideCameraIndex, byte byColorLightType);


	CPoint CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, InspPartInfo* boardInfo = NULL);
	CPoint CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, double& dRetPixelX, double& dRetPixelY, InspPartInfo* boardInfo = NULL);
	
	void Algorithms();
	bool CheckAlgoGroup(int wndType, int nWndIndex, int nAlgoIdx, InspWndResult* WndResult);

	//Ext MultiProcess
public:
	std::map<InspAlgoType, IPINSP_ALGO*> m_dicInspAlgorithm;
	ext::ExtProductionInfo * m_ExtProdInfo;
	cv::Mat cvResultImage;
	void SetExtPOCRFontInfo();
	int ShaerdInspection();
	int SetExtInspParam(int nPart);
	int SetExtInspParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nPart);
	int GetExtForeignInspParam(InspPartInfo * _ForePartInfo, ForeignData * _foreinData, FR2DData * Fr2Data, InspForeignInfo * m_ForeignInfoData, ForeignParamROI ** _ForeParamROI, ForeignParamROI ** _arrROI, int nSelectedIdx);
	int SaveInspPartImage(CString * strInspectStart , InspPartInfo* boardInfo, InspPartParam *pParamArray, int nBuffIdx);
	int SetExtProductionInfo();
	BOOL SetExtMachineAlgoParam();
	BOOL SetModelLoad();
	void SetExtMachinePatternParam(InspAlgo sInspAlgo);	//DLL로 접근시 제거 예정
	void SetExtAngleColorParam();

	int SaveInspPartImageExt();
	void CreateDir(CString Path);
	CRect GetRectRoi();
	CRect GetRectBoundaryRoi();
	void GetUserMapImage(InspRoiImgBuf partImgBuf, UCHAR * ptrbyDst, int nType);
	void GetUserMapImage(UCHAR * ptrbyTopRedBuffer, UCHAR * ptrbyTopGreenBuffer, UCHAR * ptrbyTopBlueBuffer, UCHAR * ptrbyTopWhiteBuffer, UCHAR * ptrbyMidRedBuffer, UCHAR * ptrbyMidGreenBuffer, UCHAR * ptrbyMidBlueBuffer, UCHAR * ptrbyBotRedBuffer, UCHAR * ptrbyBotBlueBuffer, UCHAR * ptrbyDst, int nType, int nWidth, int nHeight);
	int ChangeInspAlgoResult(int nWndIndex, InspWndResult* WndResult, InspAlgoTempResult * ptrInspAlgoTempResult);
	BOOL FillExceptWndArea(UCHAR * MaskImg, RECT rtCurrentFOVWnd, int nImgWidth, int nImgHeight, float nAngle);
	cv::Point2f RotatePoint(cv::Point Po, cv::Point Ce, double dAngle);
	void SavePTR(CString str, cv::Mat img3D, int nBit = 32);
	RECT GetWndROI(int nCurWndIndex, int nAlignTotalCnt, AlignResult * sptrAlignRes);
	void Exception_SaveImage();
	bool CheckWndShift(InspAlgoType eAlgo, bool bROT = false);
	bool ProcessAIDataByPartID(int nPartID, bool bUseAI, PIAL::Insp_Image* pImg_buf, double dWndW, double dWndH);
	void ClearProcessedAIPartID();
	void SaveZmapPTR(float * pZmap, CString filePath, int nW, int nH, int nBit = 32);
	void DeleteInspItem();
	void Delete_Algorithms();
	void DeleteTeachItem();
	//RECT GetWndROI(int nCurWndIndex, int nAlignTotalCnt, AlignResult * sptrAlignRes);
	//int Save_JpegImage(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, CString file_path, int nJpegQuality);
	//void setModelSzOptMz(int val) { _bOptimizeFileSize = val; }
	bool getModelSzOptMz() { return false; }

protected:
	Im::PIL_ID* m_milApp;	// mil application
	Im::PIL_ID* m_milSys;	// mil system
/*	CJPEG m_Jpeg;*/
private:
	UCHAR* CopyPVImage;
	void * m_pInspDecision;// 교차 Include 회피
	CProc3D m_proc3d;
	bool WindowRotate();
	std::set<int> m_setProcessedAIPartID;
//	std::mutex m_mtxAIPartID; // 혹여 쓰레드화 할때

#pragma region _TAB FUNCTION_
public:
	UCHAR* m_pucTabAreaTmp;
	int m_nTabAlgoIdx;
	bool m_bSharedInOutImageSave;
	bool m_bMultiProcessThreadSave;
	bool m_bForeignDebugSave;
	ext::ImageStack m_ImgStack;
#pragma endregion _TAB FUNCTION_

};
//InspManager* g_pInspMng = NULL;
extern InspManager* g_pInspMng;

	


